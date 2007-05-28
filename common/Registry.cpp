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
#include "stdafx.h"
#include "Registry.h"

//-----------------------------------------------------------------------------------------------

bool RegKey::GetString( tstring* pValue, LPCTSTR pValueName )
{
	if( ! m_hKey )
		return false;
	*pValue = _T("");
	DWORD type = 0;
	TCHAR buf[ 1024 ] = _T("");
	DWORD size = sizeof(buf) - sizeof(TCHAR);
	if( ::RegQueryValueEx( m_hKey, pValueName, NULL, &type, 
			reinterpret_cast<LPBYTE>( &buf ), &size ) == ERROR_SUCCESS )
		if( type == REG_SZ )
		{
			buf[ size / sizeof(TCHAR) ] = 0;
			*pValue = buf;				
			return true;
		}
	return false;
}

//-----------------------------------------------------------------------------------------------

bool RegKey::GetInt( int* pValue, LPCTSTR pValueName )
{
	if( ! m_hKey )
		return false;
	DWORD type = 0;
	TCHAR buf[ 64 ] = _T("");
	DWORD size = sizeof(buf) - sizeof(TCHAR);
	if( ::RegQueryValueEx( m_hKey, pValueName, NULL, &type, 
			reinterpret_cast<LPBYTE>( &buf ), &size ) == ERROR_SUCCESS )
		if( type == REG_SZ )
		{
			buf[ size / sizeof(TCHAR) ] = 0;
			*pValue = _ttoi( buf );
			return true;
		}
		else if( type == REG_DWORD )
		{
			*pValue = *reinterpret_cast<DWORD*>( buf );
			return true;
		}
	return false;
}

//-----------------------------------------------------------------------------------------------

bool RegKey::ClearKey( HKEY hKeyRoot )
{
	DWORD keyCount = 0, maxKeyNameLen = 0, valueCount = 0, maxValueNameLen = 0;
	if( ::RegQueryInfoKey( hKeyRoot, NULL, NULL, NULL, &keyCount, &maxKeyNameLen, NULL, 
	                       &valueCount, &maxValueNameLen, NULL, NULL, NULL ) 
		!= ERROR_SUCCESS )
		return false;

	bool isSuccess = true;

	//--- delete values

	std::vector<TCHAR> name( maxValueNameLen + 1 );
	std::vector<tstring> nameList;
	for( int i = 0; i < valueCount; ++i )
	{
		DWORD nameLen = name.size();
		if( ::RegEnumValue( hKeyRoot, i, &name[ 0 ], &nameLen, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS )
			nameList.push_back( &name[ 0 ] );
	}
	if( nameList.size() < valueCount )
		isSuccess = false;
	for( int i = 0; i != nameList.size(); ++i )
		if( ::RegDeleteValue( hKeyRoot, nameList[ i ].c_str() ) != ERROR_SUCCESS )
			isSuccess = false;

	//--- delete keys

	name.resize( maxKeyNameLen + 1 );
	nameList.clear();
	for( int i = 0; i < keyCount; ++i )
	{
		DWORD nameLen = name.size();
		if( ::RegEnumKeyEx( hKeyRoot, i, &name[ 0 ], &nameLen, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS )
			nameList.push_back( &name[ 0 ] );
	}	
	if( nameList.size() < keyCount )
		isSuccess = false;
	for( int i = 0; i != nameList.size(); ++i )
		if( ::SHDeleteKey( hKeyRoot, nameList[ i ].c_str() ) != ERROR_SUCCESS )
			isSuccess = false;

	return isSuccess;
}
