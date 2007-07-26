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
#include "Favorites.h"

using namespace std;

//-----------------------------------------------------------------------------------------
/// get favorites from profile

void GetDirFavorites( FavoritesList* pList )
{
	pList->clear();

	RegistryProfile profile( _T("zett42\\FlashFolder") );

	TCHAR key[32];
	for( int i = 0;; ++i )
	{
		FavoritesItem item;

		StringCbPrintf( key, sizeof(key), _T("%d_title"), i );			
		item.title = profile.GetString( _T("Favorites"), key );

		StringCbPrintf( key, sizeof(key), _T("%d"), i );
		item.command = profile.GetString( _T("Favorites"), key );

		if( item.title.empty() && item.command.empty() )
			break;

		if( item.title.empty() )
			item.title = item.command;

		StringCbPrintf( key, sizeof(key), _T("%d_targetPath"), i );			
		item.targetpath = profile.GetString( _T("Favorites"), key );

		pList->push_back( item );
	}
}

//-----------------------------------------------------------------------------------------
/// write favorites to profile

void SetDirFavorites( const FavoritesList& list )
{
	RegistryProfile profile( _T("zett42\\FlashFolder") );

	profile.DeleteSection( _T("Favorites") );
	TCHAR key[32];
	int nItem = 0;

	for( int i = 0; i != list.size(); ++i )
	{
		if( list[ i ].title.empty() )
			continue;

		StringCbPrintf( key, sizeof(key), _T("%d_title"), nItem );			
		profile.SetString( _T("Favorites"), key, list[i].title.c_str() );

		if( ! list[ i ].command.empty() )
		{
			StringCbPrintf( key, sizeof(key), _T("%d"), nItem );
			profile.SetString( _T("Favorites"), key, list[i].command.c_str() );
		}

		if( ! list[ i ].targetpath.empty() )
		{
			StringCbPrintf( key, sizeof(key), _T("%d_targetPath"), nItem );
			profile.SetString( _T("Favorites"), key, list[i].targetpath.c_str() );
		}

		++nItem;
	}
}

//-----------------------------------------------------------------------------------------------

int GetFavItemByPath( const FavoritesList& favs, LPCTSTR pPath )
{
	for( size_t i = 0; i < favs.size(); ++i )
	{
		const FavoritesItem& fav = favs[ i ];

		tstring itemPath;
		tstring token, args;
		SplitTcCommand( fav.command.c_str(), &token, &args );

		if( _tcsicmp( token.c_str(), _T("cd") ) == 0 )
			itemPath = args;
		else if( IsFilePath( fav.command.c_str() ) )
			itemPath = fav.command;

		if( _tcsicmp( itemPath.c_str(), pPath ) == 0 )
			return i;
	}
	return -1;
}
