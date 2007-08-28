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

#include <stdafx.h>

#include "ExplorerUtils.h"

//-----------------------------------------------------------------------------------------------

// Undocumented message to retrieve current path from explorer window.
#ifndef CWM_GETPATH
#define CWM_GETPATH WM_USER + 12
#endif

// Prototypes misdeclared in shlobj.h
SHSTDAPI_(HANDLE) SHAllocShared(LPCVOID pvData, DWORD dwSize, DWORD dwDestinationProcessId); 
SHSTDAPI_(BOOL) SHFreeShared(HANDLE hData, DWORD dwSourceProcessId); 
SHSTDAPI_(void *) SHLockShared(HANDLE hData, DWORD dwSourceProcessId); 
SHSTDAPI_(BOOL) SHUnlockShared(void * pvData);

//-----------------------------------------------------------------------------------------------

bool GetExplorerPath( LPTSTR pPath, HWND hWnd )
{
    LPITEMIDLIST pidl = NULL;

	DWORD pid = ::GetCurrentProcessId();
	if( HANDLE hMem = (HANDLE) ::SendMessage( hWnd, CWM_GETPATH, pid, 0 ) )
	{
		if( void* pv = ::SHLockShared( hMem, pid ) )
		{
			pidl = ::ILClone( (LPCITEMIDLIST) pv );
			::SHUnlockShared( pv );
		}
		::SHFreeShared( hMem, pid );
    }

	if( pidl )
	{
		bool res = ::SHGetPathFromIDList( pidl, pPath ) != 0;
		::ILFree( pidl );
		return res;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------

BOOL CALLBACK FindExplorerWnd_Proc( HWND hwnd, LPARAM lParam )
{
	std::vector<tstring>* pPathes = reinterpret_cast<std::vector<tstring>*>( lParam );

    TCHAR classname[ 256 ];
    ::GetClassName( hwnd, classname, 255 );
    if( _tcscmp( classname, _T("ExploreWClass") ) == 0 ||
		_tcscmp( classname, _T("CabinetWClass") ) == 0 )
    {
		TCHAR path[ MAX_PATH + 1 ];
		if( GetExplorerPath( path, hwnd ) )
			pPathes->push_back( path );
    }

    return TRUE;
}

//-----------------------------------------------------------------------------------------------

int GetAllExplorerPathes( std::vector<tstring>* pPathes )
{
	pPathes->clear();
	::EnumWindows( FindExplorerWnd_Proc, reinterpret_cast<LPARAM>( pPathes ) );
    return pPathes->size();	
}
