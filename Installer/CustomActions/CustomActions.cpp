/* This file is part of the installer library "WiX_CptUI".
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
 */

#include "stdafx.h"

using namespace std;

//----------------------------------------------------------------------------------------------------
/// SC_HANDLE resource wrapper

class ServiceHandle
{
public:
	ServiceHandle( SC_HANDLE hsc = NULL ) :
		m_hsc( hsc ) {}
			
	~ServiceHandle() { Close(); }
	
	void Close()                 { if( m_hsc ) { ::CloseServiceHandle( m_hsc ); m_hsc = NULL; } }
	SC_HANDLE Detach()           { SC_HANDLE hsc = m_hsc; m_hsc = NULL; return hsc; }
	void Attach( SC_HANDLE hsc ) { Close(); m_hsc = hsc; }
	operator SC_HANDLE() const   { return m_hsc; }
	SC_HANDLE Get() const        { return m_hsc; }

private:
	// copy protection, since the service object is not copyable
	ServiceHandle( const ServiceHandle& ) {}
	ServiceHandle& operator=( const ServiceHandle& ) {}
	
	SC_HANDLE m_hsc;	
};

//----------------------------------------------------------------------------------------------

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD reason, LPVOID lpReserved )
{
	// Initialize / finalize WiX custom action library.
	if( reason == DLL_PROCESS_ATTACH )
		WcaGlobalInitialize( hModule );
	else if( reason == DLL_PROCESS_DETACH )
		WcaGlobalFinalize();

    return TRUE;
}

//----------------------------------------------------------------------------------------------

UINT __stdcall MsgBox( MSIHANDLE hInst )
{
	MessageBox(NULL, _T("CustomAction \"MsgBox\" running"), _T("Installer"), MB_ICONINFORMATION);
	return ERROR_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
/// Explicitly define required service privileges, so the service does not run with more 
/// privileges than absolutely necessary. This makes it harder for attackers to abuse the service.

UINT __stdcall FF_SetServicePrivileges( MSIHANDLE hInst )
{
	WcaInitialize( hInst, L"FF_SetServicePrivileges" );

	ServiceHandle scm( ::OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT ) );
	if( ! scm )
	{
		WcaLogError( HRESULT_FROM_WIN32( ::GetLastError() ), L"Failed to open SCM" );
		return ERROR_INSTALL_FAILURE;
	}
	
	ServiceHandle sc( ::OpenService( scm, L"FlashFolder", SERVICE_CHANGE_CONFIG ) );
	if( ! sc )
	{
		WcaLogError( HRESULT_FROM_WIN32( ::GetLastError() ), L"Failed to open service" );
		return ERROR_INSTALL_FAILURE;
	}
	
	SERVICE_REQUIRED_PRIVILEGES_INFO priv;
	priv.pmszRequiredPrivileges = 
		SE_CREATE_GLOBAL_NAME L"\0" 
		SE_TCB_NAME L"\0"
		SE_ASSIGNPRIMARYTOKEN_NAME L"\0"
		SE_INCREASE_QUOTA_NAME L"\0"
		L"\0";
		
	if( ! ::ChangeServiceConfig2( sc, SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO, &priv ) )
	{
		WcaLogError( HRESULT_FROM_WIN32( ::GetLastError() ), L"Failed to configure service" );
		return ERROR_INSTALL_FAILURE;
	}	
	
	return ERROR_SUCCESS;
}