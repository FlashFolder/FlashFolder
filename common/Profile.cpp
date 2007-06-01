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
#include "Profile.h"
#include "Registry.h"


//===============================================================================================
// Profile methods

void Profile::GetStringList( std::vector<tstring>* pList, LPCTSTR pSectionName ) const
{
	pList->clear();
	if( ! SectionExists( pSectionName ) )
	{
		if( m_pDefaults )
			m_pDefaults->GetStringList( pList, pSectionName );
		return;
	}  
	TCHAR key[ 32 ];
	for( int i = 0;; ++i )
	{
		StringCbPrintf( key, sizeof(key), _T("%d"), i );
		if( ! ValueExists( pSectionName, key ) )
			break;
		tstring val = GetString( pSectionName, key );
		pList->push_back( val );
	}
}

//-----------------------------------------------------------------------------------------------

void Profile::SetStringList( LPCTSTR pSectionName, const std::vector<tstring>& list )
{
	ClearSection( pSectionName );

	TCHAR key[ 32 ];
	for( int i = 0; i != list.size(); ++i )
	{
		StringCbPrintf( key, sizeof(key), _T("%d"), i );
		SetString( pSectionName, key, list[ i ].c_str() );
	}
}

//===============================================================================================
// MemoryProfile methods

bool MemoryProfile::ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	SectionMap::const_iterator itSec = m_data.find( pSectionName );
	if( itSec == m_data.end() )
		return false;
	ValueMap::const_iterator it = itSec->second.find( pValueName );
	return it != itSec->second.end();
}

//-----------------------------------------------------------------------------------------------

bool MemoryProfile::SectionExists( LPCTSTR pSectionName ) const
{
	SectionMap::const_iterator itSec = m_data.find( pSectionName );
	return itSec != m_data.end();
}

//-----------------------------------------------------------------------------------------------

tstring MemoryProfile::GetString( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	SectionMap::const_iterator itSec = m_data.find( pSectionName );
	if( itSec != m_data.end() )
	{
		ValueMap::const_iterator it = itSec->second.find( pValueName );
		if( it != itSec->second.end() )
			return it->second;
	}
	const Profile* pDef = GetDefaults();
	return pDef ? pDef->GetString( pSectionName, pValueName ) : _T("");
}

//-----------------------------------------------------------------------------------------------

int MemoryProfile::GetInt( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	SectionMap::const_iterator itSec = m_data.find( pSectionName );
	if( itSec != m_data.end() )
	{
		ValueMap::const_iterator it = itSec->second.find( pValueName );
		if( it != itSec->second.end() )
			return _ttoi( it->second.c_str() );
	}
	const Profile* pDef = GetDefaults();
	return pDef ? pDef->GetInt( pSectionName, pValueName ) : 0;
}

//-----------------------------------------------------------------------------------------------

void MemoryProfile::SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags )
{
	if( flags & DONT_OVERWRITE )
		if( ValueExists( pSectionName, pValueName ) )
			return;
	m_data[ pSectionName ][ pValueName ] = pValue;
}

//-----------------------------------------------------------------------------------------------

void MemoryProfile::SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags )
{
	if( flags & DONT_OVERWRITE )
		if( ValueExists( pSectionName, pValueName ) )
			return;
	TCHAR sValue[ 64 ];
	StringCbPrintf( sValue, sizeof(sValue), _T("%d"), value );
	m_data[ pSectionName ][ pValueName ] = sValue;
}

//-----------------------------------------------------------------------------------------------

void MemoryProfile::DeleteSection( LPCTSTR pSectionName )
{
    m_data.erase( pSectionName );	
}

//-----------------------------------------------------------------------------------------------

void MemoryProfile::ClearSection( LPCTSTR pSectionName )
{
	SectionMap::iterator itSec = m_data.find( pSectionName );
	if( itSec == m_data.end() )
		return;
	itSec->second.clear();
}

//-----------------------------------------------------------------------------------------------

void MemoryProfile::DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName )
{
	SectionMap::iterator itSec = m_data.find( pSectionName );
	if( itSec == m_data.end() )
		return;
	itSec->second.erase( pValueName );	
}


//===============================================================================================
// RegistryProfile methods

void RegistryProfile::SetRoot( LPCTSTR rootPath )
{
	m_hRootKey = HKEY_CURRENT_USER;
	m_regPath = tstring( _T("Software\\") ) + tstring( rootPath ); 

	RegKey key( HKEY_LOCAL_MACHINE, m_regPath.c_str() );
	tstring multiUserOption = key.GetString( _T("MultiUserProfileOption") );
	m_isShared = ( multiUserOption == _T("shared") );
	if( m_isShared )
	{
		m_regPath += _T("\\Shared");
		m_hRootKey = HKEY_LOCAL_MACHINE;
	}	
}

//-----------------------------------------------------------------------------------

bool RegistryProfile::ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	return key.ValueExists( pValueName );
}

//-----------------------------------------------------------------------------------

bool RegistryProfile::SectionExists( LPCTSTR pSectionName ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	return key != NULL;
}

//-----------------------------------------------------------------------------------

tstring RegistryProfile::GetString( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	tstring res;
	if( key.GetString( &res, pValueName ) )
		return res;

	const Profile* pDef = GetDefaults();
	return pDef ? pDef->GetString( pSectionName, pValueName ) : _T("");
}

//-----------------------------------------------------------------------------------

int RegistryProfile::GetInt( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	int res;
	if( key.GetInt( &res, pValueName ) )
		return res;

	const Profile* pDef = GetDefaults();
	return pDef ? pDef->GetInt( pSectionName, pValueName ) : 0;
}

//-----------------------------------------------------------------------------------

void RegistryProfile::SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags )
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str(), KEY_QUERY_VALUE | KEY_SET_VALUE, true ); 
	if( ! key )
		return;
	if( flags & DONT_OVERWRITE )
		if( key.ValueExists( pValueName ) )
			return;
	key.SetString( pValueName, pValue );
}

//-----------------------------------------------------------------------------------

void RegistryProfile::SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags )
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str(), KEY_QUERY_VALUE | KEY_SET_VALUE, true ); 
	if( ! key )
		return;
	if( flags & DONT_OVERWRITE )
		if( key.ValueExists( pValueName ) )
			return;
	key.SetInt( pValueName, value );
}

//-----------------------------------------------------------------------------------

void RegistryProfile::DeleteSection( LPCTSTR pSectionName )
{
	RegKey key( m_hRootKey, m_regPath.c_str(), DELETE );
	key.DeleteKey( pSectionName );
}

//-----------------------------------------------------------------------------------

void RegistryProfile::ClearSection( LPCTSTR pSectionName )
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str(), KEY_ALL_ACCESS );
	key.Clear();
}

//-----------------------------------------------------------------------------------

void RegistryProfile::DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName )
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str(), KEY_SET_VALUE );
	key.DeleteValue( pValueName );
}

//-----------------------------------------------------------------------------------------------

void RegistryProfile::Clear()
{
	RegKey key( m_hRootKey, m_regPath.c_str(), KEY_ALL_ACCESS );
	key.Clear();
}
