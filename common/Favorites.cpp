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
	MemoryProfile defProfile;
	defProfile.SetInt( _T("main"), _T("UseTcFavorites"), isTcInstalled ? 1 : 0 );
	profile.SetDefaults( &defProfile );

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

			_stprintf( key, _T("menu%d"), i );
			if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), title, 
					sizeof(title) / sizeof(TCHAR) - 1, tcIniPath.c_str() ) == 0 )
				break;

			FavoritesItem item;
			item.title = title;

			_stprintf( key, _T("cmd%d"), i );
			if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), cmd, 
					sizeof(cmd) / sizeof(TCHAR) - 1, tcIniPath.c_str() ) > 0 )
			{
				if( _tcsnicmp( cmd, _T("cd "), 3 ) == 0 )
					// menu item is directory, extract the path
					item.path = &cmd[3];
				else
					// menu item is Total Commander command, keep as is
					item.path = cmd;
			}

			_stprintf( key, _T("path%d"), i );
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
			_stprintf( key, _T("%d"), i );
			item.path = profile.GetString( _T("Favorites"), key );
			if( item.path.empty() )
				break;

			_stprintf( key, _T("%d_title"), i );			
			item.title = profile.GetString( _T("Favorites"), key );
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
		for( int i = 0; i != list.size(); ++i )
		{
			_stprintf( key, _T("menu%d"), i + 1 );
			::WritePrivateProfileString( _T("DirMenu"), key, list[i].title.c_str(), tcIniPath.c_str() );

			tstring s;
			if( IsFilePath( list[ i ].path.c_str() ) )
				s = tstring( _T("cd ") ) + list[ i ].path;
			else
				s = list[ i ].path;
			_stprintf( key, _T("cmd%d"), i + 1 );
			if( ! s.empty() )
				::WritePrivateProfileString( _T("DirMenu"), key, s.c_str(), tcIniPath.c_str() );

			_stprintf( key, _T("path%d"), i + 1 );
			if( ! list[i].targetpath.empty() )
				::WritePrivateProfileString( _T("DirMenu"), key, list[i].targetpath.c_str(), tcIniPath.c_str() );
		}
	}
	else
	{
		//--- write favorites to FlashFolder profile

		profile.DeleteSection( _T("Favorites") );
		TCHAR key[32];
		for( int i = 0; i != list.size(); ++i )
		{
			_stprintf( key, _T("%d"), i );
			profile.SetString( _T("Favorites"), key, list[i].path.c_str() );
			_stprintf( key, _T("%d_title"), i );			
			profile.SetString( _T("Favorites"), key, list[i].title.c_str() );
		}
	}
}
