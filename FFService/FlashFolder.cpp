/* This file is part of FlashFolder. 
 * Copyright (C) 2007-2012 zett42.de ( zett42 at users.sourceforge.net ) 
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
  The service is used to wait for session logon events and start a SYSTEM process
  in the new users session which in turn installs the global FlashFolder hook function
  contained in "fflib.dll". This way FlashFolder is available in elevated processes,
  even if the logged-on user has restricted rights.
*/
#include "stdafx.h"
#include "resource.h"
#include "../fflib/fflib_exports.h"
#include "../fflib/HookInstaller.h"
#include "../common/Defines.h"
#include "../commonUtils/logfile.h"

const WCHAR INTERNAL_APPNAME[] = L"FlashFolder"; 
const WCHAR SERVICE_LOG_NAME[] = L"_ffservice_log.txt";

//---------------------------------------------------------------------------
// global state

SERVICE_STATUS          g_myServiceStatus = { SERVICE_WIN32_OWN_PROCESS }; 
SERVICE_STATUS_HANDLE   g_myServiceStatusHandle = NULL; 

CHandle g_serviceTerminateEvent;
CHandle g_hookTerminateEvent;

LogFile g_logfile;

//---------------------------------------------------------------------------

void Log( LPCTSTR str ) 
{ 
	g_logfile.Write( str );
	DebugOut( L"%s\n", str );
}

void Log( LPCTSTR str, DWORD status ) 
{ 
	TCHAR msg[ 1024 ]; swprintf_s( msg, L"%s (%d)", str, status );
	g_logfile.Write( msg );
	DebugOut( L"%s\n", msg );
}

//---------------------------------------------------------------------------
/// Notify the SCM about a changed service status.
/// Algorithm taken from the MSDN example "Writing a service main function". 

void SetMyServiceStatus( DWORD status, DWORD err = 0, DWORD waitHint = 0 )
{
	static DWORD s_checkPoint = 1;

	g_myServiceStatus.dwWin32ExitCode = err; 
	g_myServiceStatus.dwCurrentState  = status; 
	g_myServiceStatus.dwWaitHint      = waitHint; 
	
	if( status == SERVICE_START_PENDING )
		g_myServiceStatus.dwControlsAccepted = 0;
	else
		g_myServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | 
											   SERVICE_ACCEPT_SESSIONCHANGE;
											   
	if( status == SERVICE_RUNNING || status == SERVICE_STOPPED )
		g_myServiceStatus.dwCheckPoint = 0;
	else 
		g_myServiceStatus.dwCheckPoint = s_checkPoint++;											   

	if( ! ::SetServiceStatus( g_myServiceStatusHandle, &g_myServiceStatus ) )
	{ 
		DWORD err = ::GetLastError(); 
		Log( L"SetServiceStatus() error", err ); 
	} 
}

//---------------------------------------------------------------------------
/// Notify the SCM about unchanged service status.

void SetMyServiceStatusUnchanged()
{
	if( ! ::SetServiceStatus( g_myServiceStatusHandle, &g_myServiceStatus ) )
	{ 
		DWORD err = ::GetLastError(); 
		Log( L"SetServiceStatus() error", err ); 
	} 
}

//----------------------------------------------------------------------------------------------------
/// Start a hook process as the user specified by the token.

bool TryCreateHookProcess( HANDLE hToken, LPCTSTR cmd )
{
	STARTUPINFO si = { sizeof(si) };
	si.lpDesktop = L"WinSta0\\Default";  // The desktop of the interactive WinStation.
	
	PROCESS_INFORMATION pi = { 0 };

	// If the FlashFolder hook process is already running in another session, 
	// CreateProcessAsUser() returns ERROR_PIPE_NOT_CONNECTED if we call it immediately
	// after we received SERVICE_CONTROL_SESSIONCHANGE notification.
	// I could not find no another way around this then to call CreateProcessAsUser() 
	// repeatedly in this case. 
	DWORD time0 = ::GetTickCount();
	do	
	{	 
		if( ::CreateProcessAsUser( hToken, NULL, const_cast<LPTSTR>( cmd ), NULL, NULL, FALSE, 0,
					  NULL, NULL, &si, &pi ) )
		{
			Log( L"Hook process created" );
			::CloseHandle( pi.hThread );
			::CloseHandle( pi.hProcess );
			return true;
		}					  
					
		DWORD err = ::GetLastError();
		Log( L"CreateProcessAsUser failed", err );
	
		if( err != ERROR_PIPE_NOT_CONNECTED )
			return false;
			
		::Sleep( 2000 );
		Log( L"Retry..." );
	}
	while( ::GetTickCount() - time0 < 30000 );

	Log( L"Starting hook process failed because of timeout" );
	
	return false;
}

//---------------------------------------------------------------------------
/// Start 32/64-bit instances of the hook process in a different session
/// but with the same access rights as the current process (i. e. SYSTEM privileges).
/// This is to support "fast user switching" available beginning with XP and also
/// beginning with Vista, services run in a separate session than the user.
/// We need to run both 32- and 64-bit instances in parallel, so that both 32- and 64-bit
/// processes can be hooked.

bool StartHookProcessInSession( DWORD dwSessionId )
{
	Log( L"Starting hook process in session", dwSessionId );

	CHandle hProcessToken;
	if( ! ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcessToken.m_h ) )
	{
		Log( L"OpenProcessToken() failed", ::GetLastError() );
		return false;
	}	
	
	CHandle hDupToken;
	if( ! ::DuplicateTokenEx( hProcessToken, 0, NULL, SecurityImpersonation, 
	                          TokenPrimary, &hDupToken.m_h ) )
	{
		Log( L"DuplicateToken() failed", ::GetLastError() );
		return false;
	}					

	if( ! ::SetTokenInformation( hDupToken, TokenSessionId, &dwSessionId, sizeof(DWORD) ) )
	{
		Log( L"SetTokenInformation() failed", ::GetLastError() );
		return false;
	}

	// The hook process doesn't need the service privileges, so remove them.
	CHandle hRestrictedToken;
	if( ! ::CreateRestrictedToken( hDupToken, DISABLE_MAX_PRIVILEGE, 0, NULL,
		0, NULL, 0, NULL, &hRestrictedToken.m_h ) )
	{
		Log( L"CreateRestrictedToken() failed", ::GetLastError() );
		return false;
	}

	WCHAR exePath[ 4096 ] = L"";
	::GetModuleFileName( NULL, exePath, _countof( exePath ) );
	
	WCHAR exeDir[ 4096 ];
	GetAppDir( NULL, exeDir, _countof( exeDir ) );

	WCHAR cmd[ 4096 ] = L"\"";
	wcscat_s( cmd, exeDir );
	wcscat_s( cmd, L"FlashFolder.exe\" /sethooksvc" );	 

	Log( L"Starting 32-bit hook process" );
	if( ! TryCreateHookProcess( hRestrictedToken, cmd ) )
		return false;

#ifdef WIN64
	WCHAR cmd64[ 1024 ] = L"\"";
	wcscat_s( cmd64, exeDir );
	wcscat_s( cmd64, L"FlashFolder64.exe\" /sethooksvc" );	 

	Log( L"Starting 64-bit hook process" );
	if( ! TryCreateHookProcess( hRestrictedToken, cmd64 ) )
		return false;
#endif

	return true;
}

//---------------------------------------------------------------------------
/// Handler that receives events from the service manager.

DWORD WINAPI MyServiceCtrlHandler( DWORD control, DWORD eventType, 
	LPVOID pEventData, LPVOID pContext ) 
{ 
	switch( control ) 
	{ 
		case SERVICE_CONTROL_STOP: 
		{
			Log( L"SERVICE_CONTROL_STOP event received" );
			
			SetMyServiceStatus( SERVICE_STOP_PENDING, 0, 6000 );
			::PulseEvent( g_serviceTerminateEvent );

			return NO_ERROR; 
		}

		case SERVICE_CONTROL_SHUTDOWN:
		{
			Log( L"SERVICE_CONTROL_SHUTDOWN event received" );
			
			SetMyServiceStatus( SERVICE_STOP_PENDING, 0, 6000 );
			::PulseEvent( g_serviceTerminateEvent );
			
			return NO_ERROR;
		}
		
		case SERVICE_CONTROL_SESSIONCHANGE:
		{
			SetMyServiceStatusUnchanged();

			if( eventType == WTS_SESSION_LOGON )
			{
				WTSSESSION_NOTIFICATION* pSession = (WTSSESSION_NOTIFICATION*) pEventData;

				Log( L"WTS_SESSION_LOGON event received", pSession->dwSessionId );				
				
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
	
	Log( L"Unrecognized control received", control ); 

	SetMyServiceStatusUnchanged();

	return ERROR_CALL_NOT_IMPLEMENTED;
}

//---------------------------------------------------------------------------
/// Start one hook process per user session (if any).

bool StartHookInCurrentUserSessions()
{
	Log( L"StartHookInCurrentUserSessions()" );

	PWTS_SESSION_INFO pSessions = NULL;
	DWORD sessionCount = 0;

	// This is expected to fail if there are no user sessions available at the time 
	// (e. g. at boot time).
	if( ! ::WTSEnumerateSessions( WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessions, &sessionCount ) )
	{
		Log( L"WTSEnumerateSessions() error", ::GetLastError() );
		return false;
	}
	
	// Starting with Vista, session 0 is reserved for the services, user session begin at 1.
	int firstUserSession = ( GetVersion() & 0xFF ) >= 6 ? 1 : 0;
	
	for( DWORD i = firstUserSession; i < sessionCount; ++i )
	{
		WCHAR s[ 256 ] = L"";
		swprintf_s( s, L"Session ID: %d, state: %d, WinStation: %s",
			pSessions[ i ].SessionId, pSessions[ i ].State, pSessions[ i ].pWinStationName );
		Log( s );
		
		if( pSessions[ i ].State != WTSActive && 
		    pSessions[ i ].State != WTSConnected && 
		    pSessions[ i ].State != WTSDisconnected &&
		    pSessions[ i ].State != WTSIdle )
		{
			Log( L"\tSkipping session because of its state" );
			continue;	
		}
		
		StartHookProcessInSession( pSessions[ i ].SessionId );

		// Notify the SCM so it doesn't think the service is hung.
		SetMyServiceStatusUnchanged();
	}
	
	::WTSFreeMemory( pSessions );
	
	return true;
}

//---------------------------------------------------------------------------

bool InitService()
{
	// Create event for service termination 
	g_serviceTerminateEvent.Attach( ::CreateEvent( NULL, FALSE, FALSE, NULL ) );
	
	// Create event for hook termination
	g_hookTerminateEvent.Attach( 
		::CreateEvent( NULL, TRUE, FALSE, L"Global\\FFHookTerminateEvent_983874652" ) );

	if( ! g_hookTerminateEvent )
	{
		Log( L"Failed to create hook termination event.", ::GetLastError() );
		return false;
	}
		
	if( ::GetLastError() == ERROR_ALREADY_EXISTS )
	{
		// If the event exists, it could have been created by another process
		// with lower privileges, so we cannot trust it.		
	
		Log( L"Cannot trust existing hook termination event." );
		return false;
	}
	
	// Start the hook so it is available immediately after installation,
	// if the installation is done in an active user session (not for group 
	// policy installation which is done "per-machine" without user context).
	StartHookInCurrentUserSessions();
	
	return true;
}

//---------------------------------------------------------------------------

void FinalizeService()
{
	// Terminate hook processes in all sessions.
	Log( L"Setting hook termination event" );
	if( ! ::SetEvent( g_hookTerminateEvent ) )
		Log( L"Failed to set hook termination event", ::GetLastError() );
	else
		// Give hook processes time to settle down and unload the hook DLL so
		// upgrade installs work smoothly without closing other programs first.
		// TODO: make this more reliable...
		::Sleep( 3000 );
}

//---------------------------------------------------------------------------

void WINAPI ServiceMain( DWORD argc, LPTSTR *argv ) 
{ 
    Log( L"ServiceMain()" ); 

	g_myServiceStatusHandle = ::RegisterServiceCtrlHandlerEx( 
        INTERNAL_APPNAME, MyServiceCtrlHandler, NULL ); 
 
    if( g_myServiceStatusHandle == (SERVICE_STATUS_HANDLE)0 ) 
    { 
		Log( L"RegisterServiceCtrlHandler() failed", ::GetLastError() ); 
        return;
    } 
    
    SetMyServiceStatus( SERVICE_START_PENDING, 0, 30000 );
 
	if( InitService() )
	{
		// Initialization complete - report running status. 
		SetMyServiceStatus( SERVICE_RUNNING );
		
		Log( L"Service running, waiting for events..." );

		::WaitForSingleObject( g_serviceTerminateEvent, INFINITE );
		::CloseHandle( g_serviceTerminateEvent );

		FinalizeService();
	}
	
	Log( L"Service stopped." );
	SetMyServiceStatus( SERVICE_STOPPED );

    return; 
} 

//---------------------------------------------------------------------------

int StartService()
{
	WCHAR logPath[ MAX_PATH ] = L"";
	LogFile::GetPath( logPath, SERVICE_LOG_NAME );	
	// Open log file only if it already exists. So normally we don't create the log, but it can optionally
	// be created to analyze problems. 
	if( ::GetFileAttributes( logPath ) != INVALID_FILE_ATTRIBUTES )
		g_logfile.Open( logPath );

	Log( L"Task: Starting service" );

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
			Log( L"Service is already running." );
			return 0;
		}

		Log( L"StartServiceCtrlDispatcher() error, exiting", err );
		
		return 1;
	} 
	return 0;
}

//---------------------------------------------------------------------------

void OpenHookLogFile()
{
	DWORD sessionId = -1;
	::ProcessIdToSessionId( ::GetCurrentProcessId(), &sessionId );

#ifdef WIN64
	WCHAR logName[ 100 ]; swprintf_s( logName, L"_ffhook64_log%d.txt", sessionId );
#else
	WCHAR logName[ 100 ]; swprintf_s( logName, L"_ffhook32_log%d.txt", sessionId );
#endif

	WCHAR serviceLogPath[ MAX_PATH ] = L"";
	LogFile::GetPath( serviceLogPath, SERVICE_LOG_NAME );	
	// Open log file only if main log file already exists. So normally we don't create the log, 
	// but it can optionally be created to analyze problems. 
	if( ::GetFileAttributes( serviceLogPath ) != INVALID_FILE_ATTRIBUTES )
	{
		WCHAR logPath[ MAX_PATH ] = L"";
		LogFile::GetPath( logPath, logName );	
		g_logfile.Open( logPath );
	}
}

//---------------------------------------------------------------------------

int RunHook( bool startFromService )
{
	OpenHookLogFile();
	Log( startFromService ? L"Task: Starting hook from service" : L"Task: Starting hook locally" );

	//--- Open/create mutex to enforce a single instance of this process 
	//    (per user logon session and 32-bit / 64-bit).
	
#ifdef WIN64
	LPCWSTR singleInstanceMutexName = L"Local\\FFHookSingleInstanceMutex64_0938746251";
#else
	LPCWSTR singleInstanceMutexName = L"Local\\FFHookSingleInstanceMutex32_0938746251";
#endif
	CHandle singleInstanceMutex( ::CreateMutex( NULL, TRUE, singleInstanceMutexName ) );
	DWORD err = ::GetLastError();
	if( err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED )
	{
		Log( L"Hook process is already running in this session" );
		return 0;
	}
	else if( err != ERROR_SUCCESS )
	{	
		Log( L"ERROR: Could not create single-instance mutex", err );
		return 1;
	}
	
	//--- Open/create event for hook termination.
	
	LPCWSTR hookTerminateEventName = startFromService ? 
		L"Global\\FFHookTerminateEvent_983874652" :
		L"Local\\FFHookTerminateEvent_983874652";		
	CHandle hookTerminateEvent(	::CreateEvent( NULL, TRUE, FALSE, hookTerminateEventName ) );
	if( ! hookTerminateEvent )
	{
		Log( L"Failed to create hook termination event.", ::GetLastError() );
		return 1;
	}	
	
	FFHookInstaller hookInstaller;
	
	if( ! hookInstaller.Install() )
	{
		Log( L"Failed to install hook, exiting", ::GetLastError() );
		return 1;
	}
	
	Log( L"Hook installed", (DWORD) hookInstaller.GetHHook() );		
	
	//--- Don't exit process yet to keep hook alive.

	Log( L"Waiting for termination event..." );
	
	for(;;)
	{
		::WaitForSingleObject( hookTerminateEvent, INFINITE );

		Log( L"Termination event received" );

		// Check if the FlashFolder toolbar window doesn't exist and it is
		// therefore safe to exit the hook process.
		// If the hook process would exit while the toolbar still exists in
		// any process it would crash the process(es) hosting the toolbar!

		if( ::FindWindow( FF_WNDCLASSNAME, NULL ) == NULL )
		{
			Log( L"Toolbar window not found - process can terminate", ::GetLastError() );
			break;
		}
		Log( L"Cannot terminate because the FlashFolder toolbar window exists" );
				
		if( ! ::ResetEvent( hookTerminateEvent ) )
		{
			Log( L"Could not reset termination event", ::GetLastError() );
			break;
		}			
	}
	
	Log( L"Removing hook..." );
	if( ! hookInstaller.Uninstall() )
	{
		Log( L"Failed to uninstall hook", ::GetLastError() );
	}
	
	// Post a broadcast message to finally unload the hook DLL from all processes
	// that receive the message. Windows seems to unload hook DLLs for a specific process
	// only the next time the process receives a message after UnhookWindowsHookEx() 
	// has been called.
	Log( L"Broadcasting message to unload hook DLL..." );
	DWORD recipients = BSM_APPLICATIONS | BSF_IGNORECURRENTTASK;	
	if( ::BroadcastSystemMessage( BSF_FORCEIFHUNG | BSF_POSTMESSAGE, &recipients, WM_NULL, 0, 0 ) < 0 )
	{
		Log( L"Failed to broadcast message", ::GetLastError() );
	}
	
	Log( L"Exiting process" );
	
	return 0;
}

//---------------------------------------------------------------------------
/// Remove the FlashFolder hook for the current sessions.

int RemoveHook()
{
	OpenHookLogFile();
	Log( L"Task: Removing hook" );
	
	// Open event for hook termination
	CHandle hookTerminateEvent( 
		::OpenEvent( EVENT_MODIFY_STATE, FALSE, L"Local\\FFHookTerminateEvent_983874652" ) );
	if( ! hookTerminateEvent )
	{
		Log( L"Failed to open hook termination event.", ::GetLastError() );
		return 1;
	}	
	if( ! ::SetEvent( hookTerminateEvent ) )
	{
		Log( L"Failed to set hook termination event", ::GetLastError() );
		return 1;
	}

	Log( L"Termination event set" );
	
	return 0;
}

//---------------------------------------------------------------------------

void ShowHelp()
{
	::MessageBox( NULL,
		L"FlashFolder commandline syntax:\n\n"
		FLASHFOLDER_EXE L" [/sethook | /unhook | /startservice]\n\n"
		L"/sethook - Activates the FlashFolder hook for the current session.\n"
		L"/unhook - Terminates the FlashFolder hook for the current session.\n"
		L"/startservice - Entry point for the Service Control Manager.\n",
		FLASHFOLDER_NAME, 
		MB_OK );
}

//---------------------------------------------------------------------------

int WINAPI _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )         
{ 
	if( __argc == 2 )
	{
		if( wcscmp( __wargv[ 1 ], L"/startservice" ) == 0 )
			return StartService();	
		else if( wcscmp( __wargv[ 1 ], L"/sethooksvc" ) == 0 )
			return RunHook( true );
		else if( wcscmp( __wargv[ 1 ], L"/sethook" ) == 0 )
			return RunHook( false );
		else if( wcscmp( __wargv[ 1 ], L"/unhook" ) == 0 )
			return RemoveHook();
	}

	ShowHelp();
	return 1;
} 
