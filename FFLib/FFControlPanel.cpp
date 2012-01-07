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

#include "stdafx.h"
#include "resource.h"
#include "fflib.h"

//-------------------------------------------------------------------------------------------
/// Callback for control panel applet to start FFConfig.exe

LONG CALLBACK CPlApplet( HWND hwndCPl, UINT uMsg, LPARAM lParam1, LPARAM lParam2 )
{
	switch( uMsg )
	{
		case CPL_INIT: 
			return TRUE;
		case CPL_GETCOUNT: 
			return 1;
		case CPL_INQUIRE:
		{
			LPCPLINFO pInfo = (LPCPLINFO) lParam2;
			pInfo->idIcon = IDI_CPL_ICON;
			pInfo->idName = ID_FF_CONFIG;
			pInfo->idInfo = IDS_CPL_INFO;
			pInfo->lData = NULL;	
			return 0;
		}
		case CPL_DBLCLK:
		{
			// Create command for starting FFConfig in same directory as this DLL.
			TCHAR cmd[ 4096 ] = L"";
			TCHAR cplDir[ 4096 ] = L"";
			::GetModuleFileName( g_hInstDll, cplDir, _countof( cplDir ) );
			::PathRemoveFileSpec( cplDir );
			wcscpy_s( cmd, L"\"" );
			wcscat_s( cmd, cplDir );
			wcscat_s( cmd, L"\\" FFCONFIG_EXE );

			// Start FFConfig
			PROCESS_INFORMATION pi = { 0 };
			STARTUPINFO si = { sizeof( si ) };
			if( ::CreateProcess( NULL, cmd, NULL, NULL, FALSE, 0, NULL, cplDir, &si, &pi ) )
			{
				::CloseHandle( pi.hThread );
				::CloseHandle( pi.hProcess );
			}
			else
			{
				::MessageBox( hwndCPl, L"Could not start configuration program.", L"FlashFolder",
					MB_ICONERROR );
			}
			return 0;
		}
	};

	return 0;
}

