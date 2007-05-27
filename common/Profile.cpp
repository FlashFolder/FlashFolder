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
#include "Registry.h"

//-----------------------------------------------------------------------------------

void Profile::SetRoot( LPCTSTR rootPath )
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

bool Profile::ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	return key.ValueExists( pValueName );
}

//-----------------------------------------------------------------------------------

bool Profile::SectionExists( LPCTSTR pSectionName ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	return key != NULL;
}

//-----------------------------------------------------------------------------------

tstring Profile::GetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pDefaultValue ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	return key.GetString( pValueName, pDefaultValue );
}

//-----------------------------------------------------------------------------------

int Profile::GetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int defaultValue ) const
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str() ); 
	return key.GetInt( pValueName, defaultValue );
}

//-----------------------------------------------------------------------------------

void Profile::SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags )
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

void Profile::SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags )
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

void Profile::DeleteSection( LPCTSTR pSectionName )
{
	RegKey key( m_hRootKey, m_regPath.c_str(), DELETE );
	key.DeleteKey( pSectionName );
}

//-----------------------------------------------------------------------------------

void Profile::ClearSection( LPCTSTR pSectionName )
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str(), KEY_ALL_ACCESS );
	key.Clear();
}

//-----------------------------------------------------------------------------------

void Profile::DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName )
{
	tstring path = m_regPath + tstring( _T("\\") ) + tstring( pSectionName );
	RegKey key( m_hRootKey, path.c_str(), KEY_SET_VALUE );
	key.DeleteValue( pValueName );
}

//-----------------------------------------------------------------------------------------------

void Profile::Clear()
{
	RegKey key( m_hRootKey, m_regPath.c_str(), KEY_ALL_ACCESS );
	key.Clear();
}
