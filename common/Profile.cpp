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
#include "profile.h"

#include <shlwapi.h>

//-----------------------------------------------------------------------------------

bool Profile::IsShared() const
{
	bool res = false;
	tstring keyPath = tstring( _T("Software\\") ) + m_rootKey;
	HKEY hKey = NULL;
	if( ::RegOpenKeyEx( HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		DWORD type = 0;
		TCHAR buf[128] = _T("");
		DWORD size = sizeof(buf);
		if( ::RegQueryValueEx( hKey, _T("MultiUserProfileOption"), NULL, &type, 
				reinterpret_cast<LPBYTE>( &buf ), &size ) == ERROR_SUCCESS )
			if( type == REG_SZ )
				res = _tcscmp( buf, _T("shared") ) == 0;				
		::RegCloseKey( hKey );
	}
	return res;
}

//-----------------------------------------------------------------------------------

void Profile::SetShared( bool isShared )
{
	tstring keyPath = tstring( _T("Software\\") ) + m_rootKey;
	HKEY hKey = NULL;
	if( ::RegCreateKeyEx( hKey, keyPath.c_str(), 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL ) == ERROR_SUCCESS )
	{
		tstring value( isShared ? _T("shared") : _T("individual") );
		DWORD size = ( value.size() + 1 ) * sizeof(TCHAR);
		::RegSetValueEx( hKey, _T("MultiUserProfileOption"), 0, REG_SZ, 
			reinterpret_cast<const BYTE*>( value.c_str() ), size ); 
		::RegCloseKey( hKey );
	}
}

//-----------------------------------------------------------------------------------

void Profile::GetRootKey( HKEY* pKey, tstring* pPath, LPCTSTR pSectionName ) const
{
	*pKey = HKEY_CURRENT_USER;
	*pPath = tstring( _T("Software\\") ) + m_rootKey + tstring( _T("\\") );
	if( IsShared() )
	{
		*pPath += _T("Shared");
		*pKey = HKEY_LOCAL_MACHINE;
	}	
	if( pSectionName )
		*pPath += tstring( _T("\\") ) + tstring( pSectionName );
}

//-----------------------------------------------------------------------------------

bool Profile::ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	bool res = false;
	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath, pSectionName );
	HKEY hKey = NULL;
	if( ::RegOpenKeyEx( hRoot, keyPath.c_str(), 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		if( ::RegQueryValueEx( hKey, pValueName, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS )
			res = true;
		::RegCloseKey( hKey );
	}
	return res;
}

//-----------------------------------------------------------------------------------

bool Profile::SectionExists( LPCTSTR pSectionName ) const
{
	bool res = false;
	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath, pSectionName );
	HKEY hKey = NULL;
	if( ::RegOpenKeyEx( hRoot, keyPath.c_str(), 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		res = true;
		::RegCloseKey( hKey );
	}
	return res;
}

//-----------------------------------------------------------------------------------

tstring Profile::GetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pDefaultValue ) const
{
	tstring res( pDefaultValue );
	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath, pSectionName );
	HKEY hKey = NULL;
	if( ::RegOpenKeyEx( hRoot, keyPath.c_str(), 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		DWORD type = 0;
		TCHAR buf[1024] = _T("");
		DWORD size = sizeof(buf);
		if( ::RegQueryValueEx( hKey, pValueName, NULL, &type, 
				reinterpret_cast<LPBYTE>( &buf ), &size ) == ERROR_SUCCESS )
			if( type == REG_SZ )
				res = tstring( buf, size / sizeof(TCHAR) );				
		::RegCloseKey( hKey );
	}
	return res;
}

//-----------------------------------------------------------------------------------

int Profile::GetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int defaultValue ) const
{
	int res = defaultValue;
	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath, pSectionName );
	HKEY hKey = NULL;
	if( ::RegOpenKeyEx( hRoot, keyPath.c_str(), 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		DWORD type = 0;
		TCHAR buf[128] = _T("");
		DWORD size = sizeof(buf);
		if( ::RegQueryValueEx( hKey, pValueName, NULL, &type, 
				reinterpret_cast<LPBYTE>( &buf ), &size ) == ERROR_SUCCESS )
			if( type == REG_SZ )
			{
				tstring tmp( buf, size / sizeof(TCHAR) ); 
				res = _ttoi( tmp.c_str() );
			}
			else if( type == REG_DWORD )
				res = *reinterpret_cast<DWORD*>( buf );
		::RegCloseKey( hKey );
	}
	return res;
}

//-----------------------------------------------------------------------------------

void Profile::SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags )
{
	if( flags & DONT_OVERWRITE )
		if( ValueExists( pSectionName, pValueName ) )
			return;

	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath, pSectionName );
	HKEY hKey = NULL;
	if( ::RegCreateKeyEx( hRoot, keyPath.c_str(), 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL ) == ERROR_SUCCESS )
	{
		DWORD size = ( _tcslen( pValue ) + 1 ) * sizeof(TCHAR);
		::RegSetValueEx( hKey, pValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>( pValue ), size ); 
		::RegCloseKey( hKey );
	}
}

//-----------------------------------------------------------------------------------

void Profile::SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags )
{
	if( flags & DONT_OVERWRITE )
		if( ValueExists( pSectionName, pValueName ) )
			return;

	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath, pSectionName );
	HKEY hKey = NULL;
	if( ::RegCreateKeyEx( hRoot, keyPath.c_str(), 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL ) == ERROR_SUCCESS )
	{
		DWORD dwValue = static_cast<DWORD>( value );
		::RegSetValueEx( hKey, pValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>( &dwValue ), 4 ); 
		::RegCloseKey( hKey );
	}
}

//-----------------------------------------------------------------------------------

void Profile::DeleteSection( LPCTSTR pSectionName )
{
	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath );
	HKEY hKey = NULL;
	if( ::RegOpenKeyEx( hRoot, keyPath.c_str(), 0, DELETE, &hKey ) == ERROR_SUCCESS )
	{
		::SHDeleteKey( hKey, pSectionName );
		::RegCloseKey( hKey );
	}
}

//-----------------------------------------------------------------------------------

void Profile::DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName )
{
	tstring keyPath;
	HKEY hRoot;
	GetRootKey( &hRoot, &keyPath );
	HKEY hKey = NULL;
	if( ::RegOpenKeyEx( hRoot, keyPath.c_str(), 0, KEY_SET_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		::RegDeleteValue( hKey, pValueName );
		::RegCloseKey( hKey );
	}
}
