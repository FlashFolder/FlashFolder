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
#include "../fflib/fflib_exports.h"

HINSTANCE g_hInstance = NULL;

TCHAR INTERNAL_APPNAME[] = L"FlashFolder"; 

//---------------------------------------------------------------------------
// global state

SERVICE_STATUS          g_myServiceStatus = { 0 }; 
SERVICE_STATUS_HANDLE   g_myServiceStatusHandle = NULL; 

HANDLE g_hEventTerminate = NULL;

//---------------------------------------------------------------------------

void DebugOut( LPCTSTR str, DWORD status = 0 ) 
{ 
   TCHAR buf[1024]; 
   swprintf_s( buf, L"[%s] %s (%d)\n", INTERNAL_APPNAME, str, status ); 
   ::OutputDebugString( buf ); 
}

//---------------------------------------------------------------------------

void SetMyServiceStatus( DWORD status, DWORD err = 0 )
{
	if( status != 0xFFFFFFFF )
	{
		g_myServiceStatus.dwWin32ExitCode = err; 
		g_myServiceStatus.dwCurrentState  = status; 
		g_myServiceStatus.dwCheckPoint    = 0; 
		g_myServiceStatus.dwWaitHint      = 0; 
	}
	if( ! ::SetServiceStatus( g_myServiceStatusHandle, &g_myServiceStatus ) )
	{ 
		DWORD err = ::GetLastError(); 
		DebugOut( L"SetServiceStatus() error", err ); 
	} 
}

//---------------------------------------------------------------------------

DWORD MyServiceUninitialization( bool isShutDown )
{
	if( ! UninstallHook() )
	{
		DWORD err = ::GetLastError();
		DebugOut( L"UninstallHook() failed.", err );
		if( ! isShutDown )
			return err;
	}

	::PulseEvent( g_hEventTerminate );

	return NO_ERROR;
}

//---------------------------------------------------------------------------
/// Start an instance of the current process in a different session
/// but with the same access rights as the current process.

bool StartHookProcessInSession( DWORD dwSessionId )
{
	DebugOut( L"Starting self in sesssion", dwSessionId );

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
	 
	if( ! ::CreateProcessAsUser( hDupToken, NULL, cmd, NULL, NULL, FALSE, 0,
				  NULL, NULL, &si, &pi ) )
	{
		DebugOut( L"CreateProcessAsUser failed", ::GetLastError() );
		return false;
	}

	DebugOut( L"Process created" );
	::CloseHandle( pi.hThread );
	::CloseHandle( pi.hProcess );
	return true;
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

			if( MyServiceUninitialization( false ) != NO_ERROR )
				SetMyServiceStatus( SERVICE_RUNNING );
			else
				SetMyServiceStatus( SERVICE_STOPPED );
			
			return NO_ERROR; 
		}

		case SERVICE_CONTROL_SHUTDOWN:
		{
			DebugOut( L"SERVICE_CONTROL_SHUTDOWN event received" );
		
			// TODO: on error write to system log
			MyServiceUninitialization( true );
			
			return NO_ERROR;
		}
		
		case SERVICE_CONTROL_SESSIONCHANGE:
			if( eventType == WTS_SESSION_LOGON )
			{
				DebugOut( L"WTS_SESSION_LOGON event received" );				
				
				WTSSESSION_NOTIFICATION* pSession = (WTSSESSION_NOTIFICATION*) pEventData;

				LPTSTR pBuf = NULL; DWORD bufSize = 0;
				::WTSQuerySessionInformation( WTS_CURRENT_SERVER_HANDLE, pSession->dwSessionId,
					WTSUserName, &pBuf, &bufSize );
				WCHAR msg[ 256 ] = L"";
				wcscpy_s( msg, L"User: " ); wcscat_s( msg, pBuf );
				DebugOut( msg );
				::WTSFreeMemory( pBuf ); 
			
				StartHookProcessInSession( pSession->dwSessionId );
                  								
				return NO_ERROR;			
			}
			DebugOut( L"SERVICE_CONTROL_SESSIONCHANGE, unhandled eventType", eventType );
			return ERROR_CALL_NOT_IMPLEMENTED;

		case SERVICE_CONTROL_INTERROGATE: 
			return NO_ERROR;
	} 
	DebugOut( L"Unrecognized control received", control ); 
	return ERROR_CALL_NOT_IMPLEMENTED;
}

//---------------------------------------------------------------------------

void WINAPI MyServiceStart( DWORD argc, LPTSTR *argv ) 
{ 
    DebugOut( L"MyServiceStart()", 0 ); 

    DWORD status = 0; 
 
    g_myServiceStatus.dwServiceType        = SERVICE_WIN32; 
    g_myServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    g_myServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE; 
    g_myServiceStatus.dwWin32ExitCode      = 0; 
    g_myServiceStatus.dwServiceSpecificExitCode = 0; 
    g_myServiceStatus.dwCheckPoint         = 0; 
    g_myServiceStatus.dwWaitHint           = 0; 
 
	g_myServiceStatusHandle = ::RegisterServiceCtrlHandlerEx( 
        INTERNAL_APPNAME, MyServiceCtrlHandler, NULL ); 
 
    if( g_myServiceStatusHandle == (SERVICE_STATUS_HANDLE)0 ) 
    { 
		DebugOut( L"RegisterServiceCtrlHandler() failed", ::GetLastError() ); 
        return; 
    } 
 
	// Create event for termination 
	g_hEventTerminate = ::CreateEvent( NULL, FALSE, FALSE, NULL );

    // Initialization complete - report running status. 
	SetMyServiceStatus( SERVICE_RUNNING, status );

	DebugOut( L"Service running, waiting for termination event" );

	// SetWindowsHookEx() seems to attach to the caller thread. So keep this thread running.
	::WaitForSingleObject( g_hEventTerminate, INFINITE );

	::CloseHandle( g_hEventTerminate );

    return; 
} 

//---------------------------------------------------------------------------

int StartService()
{
	SERVICE_TABLE_ENTRY dispatchTable[] = 
	{ 
		{ INTERNAL_APPNAME, MyServiceStart }, 
		{ NULL, NULL } 
	}; 

	if( ! ::StartServiceCtrlDispatcher( dispatchTable ) ) 
	{ 
		DWORD err = ::GetLastError();
		DebugOut( L"StartServiceCtrlDispatcher() error, exiting", err );

		if( err == ERROR_SERVICE_ALREADY_RUNNING )	
		{
			DebugOut( L"(Service already running)", 0 );
			return 0;
		}
		return 1;
	} 
	return 0;
}

//---------------------------------------------------------------------------

int SetHook()
{
	if( ! InstallHook() )
	{
		DWORD err = ::GetLastError();
		DebugOut( L"InstallHook() failed, exiting.", err );
		return 1;
	}	
	
	//--- Run message loop to keep hook alive.
	
	DebugOut( L"Running message loop..." );
	
	WNDCLASS wc = { 0, DefWindowProc, 0, 0, g_hInstance, 0, 0, 0, 0, L"FlashFolder.jks2hd4fenjcnd3" };
	::RegisterClass( &wc );
	HWND hwnd = ::CreateWindow( wc.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, g_hInstance, 0 );
	if( ! hwnd )
	{
		DebugOut( L"Failed to create message window", ::GetLastError() );
		return 1;
	}
	
	MSG msg;
	while( ::GetMessage( &msg, NULL, 0, 0 )	)
	{
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}			

	DebugOut( L"Exiting from message loop." );

	return 0;
}

//---------------------------------------------------------------------------

int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )         
{ 
	g_hInstance = hInstance;

	DebugOut( L"Process started" );

	if( wcsstr( lpCmdLine, L"/startservice" ) )
	{
		DebugOut( L"Starting service due to command line argument" );
	
		return StartService();	
	}
	else if( wcsstr( lpCmdLine, L"/sethook" ) )
	{
		DebugOut( L"Setting hook due to command line argument" );
		
		return SetHook();
	}
	else
	{
		DebugOut( L"No command line parameters given, exiting" );
		return 1;
	}

	return 0;
} 
