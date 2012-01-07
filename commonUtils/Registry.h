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
/**
 * \file Functions for easy registry access
 */

#pragma once

#include <boost\noncopyable.hpp>
#include "tstring.h"

/**
 *  \brief Encapsulates a registry key (HKEY) and provides methods to access it.
**/
class RegKey : boost::noncopyable
{
public:
	RegKey() : m_hKey( NULL ) {}

	/// \brief Constructor attaches this instance to an HKEY handle, taking ownership of it.
	RegKey( HKEY hKeyAttach ) : m_hKey( hKeyAttach ) {}

	/// \brief Constructor opens an existing regkey or creates a new one.
	RegKey( HKEY hRoot, LPCTSTR pSubKey, DWORD desiredAccess = KEY_QUERY_VALUE, bool bCreate = false ) :
		m_hKey( NULL )
	{ 
		if( bCreate )
			Create( hRoot, pSubKey, desiredAccess );
		else
			Open( hRoot, pSubKey, desiredAccess ); 
	}

	/// \brief Destructor automatically closes the regkey if it goes out of scope.
	~RegKey() { Close(); }

	/// \brief Opens an existing regkey for read access.
	bool Open( HKEY hRoot, LPCTSTR pSubKey, DWORD desiredAccess = KEY_QUERY_VALUE )
	{
		Close();
		if( ::RegOpenKeyEx( hRoot, pSubKey, 0, desiredAccess, &m_hKey ) == ERROR_SUCCESS )
			return true;
		m_hKey = NULL;
		return false;
	}

	/// \brief Creates a new regkey or opens it if it already exists. 
	bool Create( HKEY hRoot, LPCTSTR pSubKey, DWORD desiredAccess = KEY_ALL_ACCESS )
	{
		Close();
		if( ::RegCreateKeyEx( hRoot, pSubKey, 0, NULL, 0, desiredAccess, NULL, &m_hKey, NULL ) 
				== ERROR_SUCCESS )
			return true;
		m_hKey = NULL;
		return false;
	}

	/// Attach this instance to an HKEY handle, taking ownership of it.
	void Attach( HKEY hKey ) { Close(); m_hKey = hKey; }

	/// Detaches this instance from the HKEY handle.
	HKEY Detach() { HKEY h = m_hKey; m_hKey = NULL; return h; }


	/// \brief Closes the currently open regkey instance. 
	void Close()
	{
		if( m_hKey ) 
			::RegCloseKey( m_hKey );
		m_hKey = NULL;
	}

	/// \brief Converts a RegKey instance to an HKEY so it can be used in place of an HKEY. 
	operator HKEY() const { return m_hKey; }
	
	/// \brief Checks if a sub-key of the currently open regkey exists. 
	bool KeyExists( LPCTSTR pKeyPath )
	{
		if( ! m_hKey )
			return false;
		HKEY hKey = NULL;
		if( ::RegOpenKeyEx( m_hKey, pKeyPath, 0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
		{
			::RegCloseKey( hKey );
			return true;
		}
		return false;				
	}
	/// \brief Checks if a value of the currently open regkey exists. 
	bool ValueExists( LPCTSTR pValueName )
	{
		if( ! m_hKey )
			return false;
		return ::RegQueryValueEx( m_hKey, pValueName, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS;
	}

	/// \brief Retrieves a string value from the currently open regkey.
	/// \return true if the operation was successful.
	bool GetString( tstring* pValue, LPCTSTR pValueName );

	/// \brief Retrieves a string value from the currently open regkey. 
	/// \return The retrieved value or the given default value if the value could not be read.
	tstring GetString( LPCTSTR pValueName, LPCTSTR pDefaultValue = _T("") )
	{
		if( ! m_hKey )
			return pDefaultValue;
		tstring res;
		if( GetString( &res, pValueName ) )
			return res;
		return pDefaultValue;
	}

	/// \brief Sets a string value of the currently open regkey. 
	/// \return true if the operation was successful
	bool SetString( LPCTSTR pValueName, LPCTSTR pValue )
	{
		if( ! m_hKey )
			return false;
		DWORD size = DWORD( ( _tcslen( pValue ) + 1 ) * sizeof(TCHAR) );
		return ::RegSetValueEx( m_hKey, pValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>( pValue ), size )
			== ERROR_SUCCESS; 
	}

	/// \brief Retrieves an integer value from the currently open regkey.
	///
	/// The value can be of type REG_DWORD or REG_SZ. In the latter case the string is interpreted as integer.
	/// \return true if the operation was successful
	bool GetInt( int* pValue, LPCTSTR pValueName );

	/// \brief Retrieves an integer value from the currently open regkey.
	///
	/// The value can be of type REG_DWORD or REG_SZ. In the latter case the string is interpreted as integer.
	/// \return The retrieved value or the given default value if the value could not be read.
	int GetInt( LPCTSTR pValueName, int defaultValue = 0 )
	{
		if( ! m_hKey )
			return defaultValue;
		int res;
		if( GetInt( &res, pValueName ) )
			return res;
		return defaultValue;
	}

	/// \brief Retrieves an integer value from the currently open regkey.
	/// \return true if the operation was successful
	bool SetInt( LPCTSTR pValueName, int value )
	{
		if( ! m_hKey )
			return false;
		DWORD dwValue = static_cast<DWORD>( value );
		return ::RegSetValueEx( m_hKey, pValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>( &dwValue ), 4 )
			== ERROR_SUCCESS;
	}

	/// \brief Deletes a subkey and all its descendants. 
	/// \return true if the operation was successful
	bool DeleteKey( LPCTSTR pKeyPath = NULL )
	{
		if( ! m_hKey )
			return false;
		return ::SHDeleteKey( m_hKey, pKeyPath ) == ERROR_SUCCESS;
	}

	/// \brief Delete all values and subkeys of the current regkey; 
	/// \return true if the operation was successful
	bool Clear()
	{
		if( ! m_hKey )
			return false;
		return ClearKey( m_hKey );
	}

	/// \brief Delete a value of the current regkey.
	/// \retval true if the operation was successful
	bool DeleteValue( LPCTSTR pValueName )
	{
		if( ! m_hKey )
			return false;
		return ::RegDeleteValue( m_hKey, pValueName ) == ERROR_SUCCESS;
	}

private:
	static bool ClearKey( HKEY hKeyRoot );

	HKEY m_hKey;
};