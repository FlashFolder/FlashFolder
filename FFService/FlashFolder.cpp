/* This file is part of FlashFolder. 
 * Copyright (C) 2007 zett42 ( zett42 at users.sourceforge.net ) 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/* \file FlashFolder service.
  The service is used to wait for session logon events and start an admin process
  in the new users session which in turn installs the global FlashFolder hook function
  contained in "fflib.dll". This way FlashFolder is available in admin processes,
  even if the logged-on user has restricted rights.
*/
#include "stdafx.h"
#include "resource.h"
#include "logfile.h"
#include "../fflib/fflib_exports.h"

HINSTANCE g_hInstance = NULL;

const TCHAR INTERNAL_APPNAME[] = L"FlashFolder"; 

//---------------------------------------------------------------------------
// global state

SERVICE_STATUS          g_myServiceStatus = { 0 }; 
SERVICE_STATUS_HANDLE   g_myServiceStatusHandle = NULL; 

CHandle g_serviceTerminateEvent;
CHandle g_hookTerminateEvent;

LogFile g_logfile;

//---------------------------------------------------------------------------

void DebugOut( LPCTSTR str ) 
{ 
	g_logfile.Write( str );
}

void DebugOut( LPCTSTR str, DWORD status ) 
{ 
	TCHAR msg[ 1024 ]; swprintf_s( msg, L"%s (%d)", str, status );
	g_logfile.Write( msg );
}

//---------------------------------------------------------------------------
/// Notify the SCM about a changed service status.

void SetMyServiceStatus( DWORD status, DWORD err = 0 )
{
	g_myServiceStatus.dwWin32ExitCode = err; 
	g_myServiceStatus.dwCurrentState  = status; 
	g_myServiceStatus.dwCheckPoint    = 0; 
	g_myServiceStatus.dwWaitHint      = 0; 

	if( ! ::SetServiceStatus( g_myServiceStatusHandle, &g_myServiceStatus ) )
	{ 
		DWORD err = ::GetLastError(); 
		DebugOut( L"SetServiceStatus() error", err ); 
	} 
}

//---------------------------------------------------------------------------
/// Notify the SCM about unchanged service status.

void SetMyServiceStatusUnchanged()
{
	if( ! ::SetServiceStatus( g_myServiceStatusHandle, &g_myServiceStatus ) )
	{ 
		DWORD err = ::GetLastError(); 
		DebugOut( L"SetServiceStatus() error", err ); 
	} 
}

//---------------------------------------------------------------------------
/// Start an instance of the current process in a different session
/// but with the same access rights as the current process.
/// This is to support "fast user switching" available beginning with XP.

bool StartHookProcessInSession( DWORD dwSessionId )
{
	DebugOut( L"Starting hook process in sesssion", dwSessionId );

	CHandle hProcessToken;
	if( ! ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcessToken.m_h ) )
	{
		DebugOut( L"OpenProcessToken() failed", ::GetLastError() );
		return false;
	}	
	
	CHandle hDupToken;
	if( ! ::DuplicateTokenEx( hProcessToken, 0, NULL, SecurityImpersonation, 
	                          TokenPrimary, &hDupToken.m_h ) )
	{
		DebugOut( L"DuplicateToken() failed", ::GetLastError() );
		return false;
	}					

	if( ! ::SetTokenInformation( hDupToken, TokenSessionId, &dwSessionId, sizeof(DWORD) ) )
	{
		DebugOut( L"SetTokenInformation() failed", ::GetLastError() );
		return false;
	}
	
	STARTUPINFO si = { sizeof(si) };
	si.lpDesktop = L"WinSta0\\Default";
	PROCESS_INFORMATION pi = { 0 };
	WCHAR cmd[ 1024 ] = L"\"";
	WCHAR exePath[ 1024 ] = L"";
	::GetModuleFileName( NULL, exePath, _countof( exePath ) );
	wcscat_s( cmd, exePath );
	wcscat_s( cmd, L"\" /sethook" );
	 
	// If the FlashFolder hook process is already running in another session, 
	// CreateProcessAsUser() returns ERROR_PIPE_NOT_CONNECTED if we call it immediately
	// after we received SERVICE_CONTROL_SESSIONCHANGE notification.
	// I could not find no another way around this then to call CreateProcessAsUser() 
	// repeatedly in this case. 
	DWORD time0 = ::GetTickCount();
	do	
	{	 
		if( ::CreateProcessAsUser( hDupToken, NULL, cmd, NULL, NULL, FALSE, 0,
					  NULL, NULL, &si, &pi ) )
		{
			DebugOut( L"Hook process created" );
			::CloseHandle( pi.hThread );
			::CloseHandle( pi.hProcess );
			return true;
		}					  
					
		DWORD err = ::GetLastError();
		DebugOut( L"CreateProcessAsUser failed", err );
	
		if( err != ERROR_PIPE_NOT_CONNECTED )
			return false;
			
		::Sleep( 2000 );
		DebugOut( L"Retry..." );
	}
	while( ::GetTickCount() - time0 < 30000 );

	DebugOut( L"Starting hook process failed because of timeout" );
	
	return false;
}

//---------------------------------------------------------------------------
// handler that receives events from the service manager

DWORD WINAPI MyServiceCtrlHandler( DWORD control, DWORD eventType, 
	LPVOID pEventData, LPVOID pContext ) 
{ 
	switch( control ) 
	{ 
		case SERVICE_CONTROL_STOP: 
		{
			DebugOut( L"SERVICE_CONTROL_STOP event received" );
			
			SetMyServiceStatus( SERVICE_STOP_PENDING );
			::PulseEvent( g_serviceTerminateEvent );

			return NO_ERROR; 
		}

		case SERVICE_CONTROL_SHUTDOWN:
		{
			DebugOut( L"SERVICE_CONTROL_SHUTDOWN event received" );
			
			SetMyServiceStatus( SERVICE_STOP_PENDING );
			::PulseEvent( g_serviceTerminateEvent );
			
			return NO_ERROR;
		}
		
		case SERVICE_CONTROL_SESSIONCHANGE:
		{
			SetMyServiceStatusUnchanged();

			if( eventType == WTS_SESSION_LOGON )
			{
				WTSSESSION_NOTIFICATION* pSession = (WTSSESSION_NOTIFICATION*) pEventData;

				DebugOut( L"WTS_SESSION_LOGON event received", pSession->dwSessionId );				
				
				// Hint to self: wait for event "Global\\TermSrvReadyEvent" if you need
				// terminal services functions like WtsQuerySessionInformation() here. 
				
				StartHookProcessInSession( pSession->dwSessionId );
                  								
				return NO_ERROR;			
			}
		
			return ERROR_CALL_NOT_IMPLEMENTED;
		}

		case SERVICE_CONTROL_INTERROGATE:
		{
			SetMyServiceStatusUnchanged();
			
			return NO_ERROR;
		}
	} 
	
	DebugOut( L"Unrecognized control received", control ); 

	SetMyServiceStatusUnchanged();

	return ERROR_CALL_NOT_IMPLEMENTED;
}

//---------------------------------------------------------------------------

bool InitService()
{
	// Create event for service termination 
	g_serviceTerminateEvent.Attach( ::CreateEvent( NULL, FALSE, FALSE, NULL ) );
	
	// Create event for hook termination
	g_hookTerminateEvent.Attach( 
		::CreateEvent( NULL, TRUE, FALSE, L"Global\\FFHookTerminateEvent_du38hndkj4" ) );

	if( ! g_hookTerminateEvent )
	{
		DebugOut( L"Failed to create hook termination event.", ::GetLastError() );
		return false;
	}
		
	if( ::GetLastError() == ERROR_ALREADY_EXISTS )
	{
		// If the event exists, it could have been created by another process
		// with lower privileges, so we cannot trust it.		
	
		DebugOut( L"Cannot trust existing hook termination event." );
		return false;
	}
	
	return true;
}

//---------------------------------------------------------------------------

void WINAPI ServiceMain( DWORD argc, LPTSTR *argv ) 
{ 
    DebugOut( L"ServiceMain() entered" ); 

    g_myServiceStatus.dwServiceType      = SERVICE_WIN32; 
    g_myServiceStatus.dwCurrentState     = SERVICE_START_PENDING; 
    g_myServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | 
                                           SERVICE_ACCEPT_SESSIONCHANGE; 
    g_myServiceStatus.dwWin32ExitCode    = 0; 
    g_myServiceStatus.dwServiceSpecificExitCode = 0; 
    g_myServiceStatus.dwCheckPoint       = 0; 
    g_myServiceStatus.dwWaitHint         = 0; 
 
	g_myServiceStatusHandle = ::RegisterServiceCtrlHandlerEx( 
        INTERNAL_APPNAME, MyServiceCtrlHandler, NULL ); 
 
    if( g_myServiceStatusHandle == (SERVICE_STATUS_HANDLE)0 ) 
    { 
		DebugOut( L"RegisterServiceCtrlHandler() failed", ::GetLastError() ); 
        return;
    } 
 
	if( InitService() )
	{
		// Initialization complete - report running status. 
		SetMyServiceStatus( SERVICE_RUNNING );
		
		DebugOut( L"Service running, waiting for events..." );

		::WaitForSingleObject( g_serviceTerminateEvent, INFINITE );
		::CloseHandle( g_serviceTerminateEvent );

		// Terminate hook processes in all sessions.
		DebugOut( L"Setting hook termination event" );
		if( ! ::SetEvent( g_hookTerminateEvent ) )
			DebugOut( L"Failed to set hook termination event", ::GetLastError() );
	}
	
	SetMyServiceStatus( SERVICE_STOPPED );
	DebugOut( L"Service stopped." );

    return; 
} 

//---------------------------------------------------------------------------

int StartService()
{
	g_logfile.Open( L"_ffservice_log.txt" );

	DebugOut( L"===== Starting service" );

	SERVICE_TABLE_ENTRY dispatchTable[] = 
	{ 
		{ (LPWSTR) INTERNAL_APPNAME, ServiceMain }, 
		{ NULL, NULL } 
	}; 

	if( ! ::StartServiceCtrlDispatcher( dispatchTable ) ) 
	{ 
		DWORD err = ::GetLastError();
		if( err == ERROR_SERVICE_ALREADY_RUNNING )	
		{
			DebugOut( L"Service is already running." );
			return 0;
		}

		DebugOut( L"StartServiceCtrlDispatcher() error, exiting", err );
		
		return 1;
	} 
	return 0;
}

//---------------------------------------------------------------------------

LRESULT CALLBACK HookWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( msg == WM_DESTROY )
	{
		DebugOut( L"WM_DESTROY received - exiting" );
		::PostQuitMessage( 0 );
	}	
	else if( msg == WM_ENDSESSION && wParam == TRUE )
	{
		DebugOut( L"WM_ENDSESSION received - exiting" );
		::PostQuitMessage( 0 );	
	}

	return ::DefWindowProc( hWnd, msg, wParam, lParam );
}

//---------------------------------------------------------------------------

void OpenHookLogFile()
{
	DWORD sessionId = -1;
	::ProcessIdToSessionId( ::GetCurrentProcessId(), &sessionId );

	WCHAR logName[ 100 ]; swprintf_s( logName, L"_ffhook_log%d.txt", sessionId );
	g_logfile.Open( logName );
}

//---------------------------------------------------------------------------

int SetHook()
{
	OpenHookLogFile();
	
	DebugOut( L"===== Setting hook" );
	
	// Open/create event for hook termination
	CHandle hookTerminateEvent( 
		::CreateEvent( NULL, TRUE, FALSE, L"Global\\FFHookTerminateEvent_du38hndkj4" ) );
	if( ! hookTerminateEvent )
	{
		DebugOut( L"Failed to create hook termination event.", ::GetLastError() );
		return 1;
	}	

	DebugOut( L"Installing hook..." );
	if( ! InstallHook() )
	{
		DWORD err = ::GetLastError();
		DebugOut( L"InstallHook() failed, exiting", err );
		return 1;
	}	
	
	
	//--- Run message loop to keep hook alive.
	
	DebugOut( L"Running message loop..." );
	
	WNDCLASS wc = { 0, HookWndProc, 0, 0, g_hInstance, 0, 0, 0, 0, L"FlashFolder.jks2hd4fenjcnd3" };
	::RegisterClass( &wc );
	HWND hwnd = ::CreateWindow( wc.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, g_hInstance, 0 );
	if( ! hwnd )
	{
		DebugOut( L"Failed to create message window", ::GetLastError() );
		return 1;
	}
	
	for(;;)
	{
		DWORD waitRes =	::MsgWaitForMultipleObjects( 1, &hookTerminateEvent.m_h, FALSE, INFINITE, QS_ALLINPUT );
		if( waitRes == WAIT_OBJECT_0 )
		{
			DebugOut( L"Termination event received" );
			break;
		}
		else if( waitRes == WAIT_OBJECT_0 + 1 )
		{
			MSG msg;		
			if( ::GetMessage( &msg, NULL, 0, 0 ) <= 0 )
			{	
				DebugOut( L"GetMessage() return value <= 0." );
				break;
			}
		
			DebugOut( L"Message received", msg.message );
		
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );	
		}
		else
		{
			DebugOut( L"Unexpected return value of MsgWaitForMultipleObjects()", waitRes );
			break;
		}
	}
	 
	::UnregisterClass( wc.lpszClassName, g_hInstance );	

	DebugOut( L"Hook process exits" );

	return 0;
}

//---------------------------------------------------------------------------
/// Remove the FlashFolder hook for all sessions.

int RemoveHook()
{
	OpenHookLogFile();
	
	DebugOut( L"===== Removing hook" );

	// Open/create event for hook termination
	CHandle hookTerminateEvent( 
		::OpenEvent( EVENT_MODIFY_STATE, FALSE, L"Global\\FFHookTerminateEvent_du38hndkj4" ) );
	if( ! hookTerminateEvent )
	{
		DebugOut( L"Failed to open hook termination event.", ::GetLastError() );
		return 1;
	}	
	if( ! ::SetEvent( hookTerminateEvent ) )
	{
		DebugOut( L"Failed to set hook termination event", ::GetLastError() );
		return 1;
	}

	DebugOut( L"Termination event set" );
	
	return 0;
}

//---------------------------------------------------------------------------

void ShowHelp()
{
	::MessageBox( NULL,
		L"FlashFolder command line syntax:\n\n"
		L"FlashFolder.exe <Parameter>\n\n"
		L"<Parameter> can be one of:\n"
		L"/startservice Starts the FlashFolder service and activates the hook for any active user sessions.\n"
		L"/sethook Activates the FlashFolder hook.\n"
		L"/unhook Terminates the FlashFolder hook.\n",
		L"FlashFolder commandline help",
		MB_OK );
}

//---------------------------------------------------------------------------

int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )         
{ 
	g_hInstance = hInstance;

	if( wcsstr( lpCmdLine, L"/startservice" ) )
	{	
		return StartService();	
	}
	else if( wcsstr( lpCmdLine, L"/sethook" ) )
	{
		return SetHook();
	}
	else if( wcsstr( lpCmdLine, L"/unhook" ) )
	{
		return RemoveHook();
	}
	else
	{
		ShowHelp();
		return 1;
	}

	return 0;
} 
