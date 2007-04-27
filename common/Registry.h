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
/**
 * \file Functions for easy registry access
 */

#pragma once

class RegKey
{
public:
	RegKey() : m_hKey( NULL ) {}

	RegKey( HKEY hRoot, LPCTSTR pSubKey, DWORD desiredAccess = KEY_QUERY_VALUE )
		{ Open( hRoot, pSubKey, desiredAccess ); }

	~RegKey() { Close(); }

	bool Open( HKEY hRoot, LPCTSTR pSubKey, DWORD desiredAccess = KEY_QUERY_VALUE )
	{
		Close();
		if( ::RegOpenKeyEx( hRoot, pSubKey, 0, desiredAccess, &m_hKey ) == ERROR_SUCCESS )
			return true;
		m_hKey = NULL;
		return false;
	}

	bool Create( HKEY hRoot, LPCTSTR pSubKey, DWORD desiredAccess = KEY_ALL_ACCESS )
	{
		Close();
		if( ::RegCreateKeyEx( hRoot, pSubKey, 0, NULL, 0, desiredAccess, NULL, &m_hKey, NULL ) 
				== ERROR_SUCCESS )
			return true;
		m_hKey = NULL;
		return false;
	}
	
	void Close()
	{
		if( m_hKey ) 
			::RegCloseKey( m_hKey );
		m_hKey = NULL;
	}

	operator HKEY() const { return m_hKey; }
	
	bool ValueExists( LPCTSTR pValueName )
	{
		return ::RegQueryValueEx( m_hKey, pValueName, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS;
	}

	bool GetString( tstring* pValue, LPCTSTR pValueName )
	{
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

	tstring GetString( LPCTSTR pValueName, LPCTSTR pDefaultValue = _T("") )
	{
		tstring res;
		if( GetString( &res, pValueName ) )
			return res;
		return pDefaultValue;
	}

	void SetString( LPCTSTR pValueName, LPCTSTR pValue )
	{
		DWORD size = ( _tcslen( pValue ) + 1 ) * sizeof(TCHAR);
		::RegSetValueEx( m_hKey, pValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>( pValue ), size ); 
	}

	bool GetInt( int* pValue, LPCTSTR pValueName )
	{
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

	int GetInt( LPCTSTR pValueName, int pDefaultValue = 0 )
	{
		int res;
		if( GetInt( &res, pValueName ) )
			return res;
		return pDefaultValue;
	}

	void SetInt( LPCTSTR pValueName, int value )
	{
		DWORD dwValue = static_cast<DWORD>( value );
		::RegSetValueEx( m_hKey, pValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>( &dwValue ), 4 ); 
	}


private:
	HKEY m_hKey;
};