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

void GetDirFavorites( FavoritesList* pList, DirFavoritesSrc source )
{
	tstring tcIniPath;
	bool isTcInstalled = GetTotalCmdLocation( NULL, &tcIniPath );

	RegistryProfile profile( _T("zett42\\FlashFolder") );
	//MemoryProfile defProfile;
	//defProfile.SetInt( _T("main"), _T("UseTcFavorites"), isTcInstalled ? 1 : 0 );
	//profile.SetDefaults( &defProfile );

	if( source == DFS_DEFAULT )
	{
		source = DFS_FLASHFOLDER;
		if( isTcInstalled )
			if( profile.GetInt( _T("main"), _T("UseTcFavorites") ) != 0 )
				source = DFS_TOTALCMD;
	}

	pList->clear();

	if( source == DFS_TOTALCMD )
	{
		//--- Get directory menu from TotalCommander profile.
		// Get all available data for each menu item so that even if we don't need it,
		// proper roundtripping can be done when the menu is later saved by
		// SetDirFavorites().

		tstring tcIniPath;
		if( ! GetTotalCmdLocation( NULL, &tcIniPath ) )
			return;
		
		tstring dbg = tstring( _T("[fflib] Found wincmd.ini at ") ) + tcIniPath + 
			tstring( _T("\n") );
		::OutputDebugString( dbg.c_str() );

		for( int i = 1;; ++i )
		{
			TCHAR key[ 32 ];
			TCHAR cmd[ MAX_PATH + 1 + 3 ] = _T("");
			TCHAR title[ 256 ] = _T("");
			TCHAR targetPath[ MAX_PATH + 1 ] = _T("");

			StringCbPrintf( key, sizeof(key), _T("menu%d"), i );
			if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), title, 
					sizeof(title) / sizeof(TCHAR) - 1, tcIniPath.c_str() ) == 0 )
				break;

			FavoritesItem item;
			item.title = title;

			StringCbPrintf( key, sizeof(key), _T("cmd%d"), i );
			if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), cmd, 
					sizeof(cmd) / sizeof(TCHAR) - 1, tcIniPath.c_str() ) > 0 )
			{
				if( _tcsnicmp( cmd, _T("cd "), 3 ) == 0 )
					// menu item is directory, extract the path
					item.command = &cmd[3];
				else
					// menu item is Total Commander command, keep as is
					item.command = cmd;
			}

			StringCbPrintf( key, sizeof(key), _T("path%d"), i );
			if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), targetPath, 
					sizeof(targetPath) / sizeof(TCHAR) - 1, tcIniPath.c_str() ) > 0 )
				item.targetpath = targetPath;		

			pList->push_back( item );
		}
	}
	else 
	{
		//--- get favorites from FlashFolder profile

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
}

//-----------------------------------------------------------------------------------------

void SetDirFavorites( const FavoritesList& list, DirFavoritesSrc source )
{
	RegistryProfile profile( _T("zett42\\FlashFolder") );

	if( source == DFS_DEFAULT )
		if( profile.GetInt( _T("main"), _T("UseTcFavorites") ) != 0 )
			source = DFS_TOTALCMD;
		else
			source = DFS_FLASHFOLDER;

	if( source == DFS_TOTALCMD )
	{
		//--- write favorites to Total Commander profile

		tstring tcIniPath;
		if( ! GetTotalCmdLocation( NULL, &tcIniPath ) )
			return;

		// remove current items from file
		::WritePrivateProfileString( _T("DirMenu"), NULL, NULL, tcIniPath.c_str() ); 

		TCHAR key[ 32 ];
		int nItem = 1;
		for( int i = 0; i != list.size(); ++i )
		{
			if( list[ i ].title.empty() )
				continue;

			StringCbPrintf( key, sizeof(key), _T("menu%d"), nItem );
			::WritePrivateProfileString( _T("DirMenu"), key, list[i].title.c_str(), tcIniPath.c_str() );

			if( ! list[ i ].command.empty() )
			{
				tstring s;
				if( IsFilePath( list[ i ].command.c_str() ) )
					s = tstring( _T("cd ") ) + list[ i ].command;
				else
					s = list[ i ].command;
				StringCbPrintf( key, sizeof(key), _T("cmd%d"), nItem );
				::WritePrivateProfileString( _T("DirMenu"), key, s.c_str(), tcIniPath.c_str() );
			}
			
			if( ! list[ i ].targetpath.empty() )
			{
				StringCbPrintf( key, sizeof(key), _T("path%d"), nItem );
				::WritePrivateProfileString( _T("DirMenu"), key, list[i].targetpath.c_str(), tcIniPath.c_str() );
			}
			
			++nItem;
		}
	}
	else
	{
		//--- write favorites to FlashFolder profile

		profile.DeleteSection( _T("Favorites") );
		TCHAR key[32];
		int nItem = 0;
		for( int i = 0; i != list.size(); ++i )
		{
			if( list[ i ].title.empty() )
				continue;
		
			StringCbPrintf( key, sizeof(key), _T("%d_title"), i );			
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
}

//-----------------------------------------------------------------------------------------------

int GetFavItemByPath( const FavoritesList& favs, LPCTSTR pPath, LPCTSTR pTargetPath )
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
		{
			if( ! pTargetPath )  
				return i;
			if( _tcsicmp( fav.targetpath.c_str(), pTargetPath ) == 0 )
				return i;
		}		
	}
	return -1;
}