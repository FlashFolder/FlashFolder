/* This file is part of the installer library "WiX_CptUI".
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
 */

#include "stdafx.h"

#include <shlwapi.h>
#include <strsafe.h>

#pragma comment( lib, "shlwapi" )
#pragma comment( lib, "msi" )

//----------------------------------------------------------------------------------------------

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//----------------------------------------------------------------------------------------------

UINT __stdcall ClearRegistry( MSIHANDLE hInst )
{
	DWORD size;
	UINT msiRes;

	TCHAR rootReg[ 256 ] = _T("");
	size = sizeof(rootReg) / sizeof(TCHAR) - 1;
	msiRes = ::MsiGetProperty( hInst, _T("FF_RootReg"), rootReg, &size );
    if( msiRes != ERROR_SUCCESS )
        return ERROR_INSTALL_FAILURE;

	TCHAR productReg[ 256 ] = _T("");
	size = sizeof(productReg) / sizeof(TCHAR) - 1;
	msiRes = ::MsiGetProperty( hInst, _T("FF_ProductReg"), productReg, &size );
    if( msiRes != ERROR_SUCCESS )
        return ERROR_INSTALL_FAILURE;

	// fullRootReg = Software\[CptUI_RootReg]
	TCHAR fullRootReg[ 256 ];
	StringCbCopy( fullRootReg, sizeof(fullRootReg), _T("Software\\") );
	StringCbCat( fullRootReg, sizeof(fullRootReg), rootReg );

	// fullProductReg = Software\[CptUI_RootReg]\[CptUI_ProductReg]
	TCHAR fullProductReg[ 256 ];
	StringCbCopy( fullProductReg, sizeof(fullProductReg), fullRootReg );
	StringCbCat( fullProductReg, sizeof(fullProductReg), _T("\\") );
	StringCbCat( fullProductReg, sizeof(fullProductReg), productReg );

	//--- keys which will be deleted recursively

	const int numEntries = 1;
	LPCTSTR entries[numEntries] = 
	{	
		fullProductReg
	};

	//--- keys which will only be deleted only if they are empty

	const int numNonRecEntries = 1;
	LPCTSTR nonRecursiveEntries[numNonRecEntries] = 
	{	
		fullRootReg
	};

	//---------

	for( int i = 0; i < numEntries; ++i )
	{
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, entries[i], 0, KEY_WRITE, &hKey)==ERROR_SUCCESS)
		{
			::SHDeleteKey(HKEY_LOCAL_MACHINE, entries[i]);
			::RegCloseKey( hKey );
		}
	}
	for( int i = 0; i < numEntries; ++i )
	{
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, entries[i], 0, KEY_WRITE, &hKey)==ERROR_SUCCESS)
		{
			::SHDeleteKey(HKEY_CURRENT_USER, entries[i]);
			::RegCloseKey( hKey );
		}
	}
	for( int i = 0; i < numNonRecEntries; ++i )
	{
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, nonRecursiveEntries[i], 0, KEY_WRITE, &hKey)==ERROR_SUCCESS)
		{
			::RegDeleteKey( HKEY_LOCAL_MACHINE, nonRecursiveEntries[i] );
			::RegCloseKey( hKey );
		}
	}
	for( int i = 0; i < numNonRecEntries; ++i )
	{
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, nonRecursiveEntries[i], 0, KEY_WRITE, &hKey)==ERROR_SUCCESS)
		{
			::RegDeleteKey( HKEY_CURRENT_USER, nonRecursiveEntries[i] );
			::RegCloseKey( hKey );
		}
	}

	return ERROR_SUCCESS;
}

//----------------------------------------------------------------------------------------------

UINT __stdcall MsgBox(MSIHANDLE hModule)
{
	MessageBox(NULL, _T("CustomAction \"MsgBox\" running"), _T("Installer"), MB_ICONINFORMATION);
	return ERROR_SUCCESS;
}