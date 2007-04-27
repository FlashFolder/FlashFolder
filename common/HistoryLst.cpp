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

#pragma warning(disable:4786)   // disable STL-template-related warnings
#pragma warning(disable:4018)   // conflict between signed and unsigned

#include "HistoryLst.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------------------

HistoryLst::HistoryLst() :
	m_maxEntries( 20 )
{
    m_list.reserve( 20 );
}

//---------------------------------------------------------------------------------------

void HistoryLst::Add( LPCTSTR s )
{
    HISTORYLIST::iterator it;
    for (it = m_list.begin(); it != m_list.end(); it++)
        if( _tcsicmp( it->c_str(), s ) == 0 ) 
        {
            m_list.erase( it );
            break;
        }

	m_list.insert( m_list.begin(), s );  
    
    if( m_list.size() > m_maxEntries )
		m_list.resize( m_maxEntries );
}

//---------------------------------------------------------------------------------------

void HistoryLst::AddFolder( LPCTSTR s )
{
	//add a file folder (directory) - remove the trailing '\' if exists
	if( s != NULL )
	{
		int len = _tcslen( s );
		if( s[len] != _T('\\') )
			Add( s );
		else
		{
			LPTSTR s1 = _tcsdup( s );
			s1[len] = 0;
			Add( s1 );
			free( s1 );
		}
	}
}

//---------------------------------------------------------------------------------------

void HistoryLst::SetMaxEntries(int count)
{
	if (count > 0)
	{
		m_maxEntries = count;
		if (count < m_list.size())
			m_list.resize(count);
	}
}

//---------------------------------------------------------------------------------------

bool HistoryLst::LoadFromProfile( const Profile& profile, LPCTSTR sectionName )
{
	::OutputDebugString( _T("[fflib] HistoryLst::LoadFromProfile\n") );

	m_list.clear();
	for( int n = 0;; ++n )
	{
		TCHAR key[8];
		_stprintf( key, _T("%d"), n );
		tstring path = profile.GetString( sectionName, key );
		if( path.empty() )
			break;
		m_list.push_back( path );
	}
    return m_list.size() > 0;    

	::OutputDebugString( _T("[fflib] HistoryLst::LoadFromProfile return\n") );
}

//---------------------------------------------------------------------------------------
    
void HistoryLst::SaveToProfile( Profile& profile, LPCTSTR sectionName )
{
	::OutputDebugString( _T("[fflib] HistoryLst::SaveToProfile\n") );

	// write new entries to profile
	TCHAR key[8];
	HISTORYLIST::const_iterator it;
	int n = 0;
	for( it = m_list.begin(); it != m_list.end(); it++ )
    {
		_stprintf( key, _T("%d"), n );	
		profile.SetString( sectionName, key, it->c_str() );
		n++;
	}

    // delete all unused entries from profile
	for( int n = m_list.size();; ++n ) 
	{
		_stprintf( key, _T("%d"), n );
		if( profile.GetString( sectionName, key ).empty() )
			break;
		profile.DeleteValue( sectionName, key );
	}
	::OutputDebugString( _T("[fflib] HistoryLst::SaveToProfile return\n") );
}
