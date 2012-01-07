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
 */
#include "stdafx.h"
#include "TotalCmdUtils.h"

namespace tc = totalcmdutils;
using namespace std;
using namespace ffplug;

//-----------------------------------------------------------------------------------------

BOOL TotalCmd_EnumFavorites( UINT index, FavMenuItem* item )
{
	static tc::FavMenu s_menu;

	if( index == 0 )
	{
		wstring iniPath;
		if( ! tc::GetTotalCmdLocation( NULL, &iniPath ) )
		{
			DebugOut( L"TotalCmd wincmd.ini not found." );
			return FALSE;
		}
		DebugOut( _T("Found TotalCmd wincmd.ini at %s\n"), iniPath.c_str() );

		if( ! tc::LoadFavoritesMenu( &s_menu, iniPath.c_str(), L"DirMenu" ) )
		{
			DebugOut( L"Failed to load TotalCmd DirMenu." );
			return FALSE;
		}
	}

	if( index >= s_menu.size() )
		return FALSE;

	const tc::FavMenuItem& m = s_menu[ index ];

	if( m.menu == L"" )
		return FALSE;

	item->type = FavMenuItem::T_UNKNOWN;

	if( m.menu == L"-" )
	{
		item->type = FavMenuItem::T_SEPARATOR;
	}
	else if( m.menu == L"--" )
	{
		item->type = FavMenuItem::T_SUBMENU_END;
	}
	else if( m.menu[ 0 ] == L'-' )
	{
		item->type = FavMenuItem::T_SUBMENU;
		wcscpy_s( item->displayName, &m.menu[ 1 ] );
	}
	else
	{
		wcscpy_s( item->displayName, m.menu.c_str() );

		if( m.cmd.substr( 0, 3 ) == L"cd " )
		{
			item->type = FavMenuItem::T_FOLDER_FAVORITE;

			CComPtr< IShellFolder > desktopFolder;
			if( SUCCEEDED( ::SHGetDesktopFolder( &desktopFolder ) ) )
				desktopFolder->ParseDisplayName( NULL, NULL, const_cast<LPWSTR>( &m.cmd[ 3 ] ), NULL, &item->pidlFolder, NULL );
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------------------

BOOL TotalCmd_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter )
{
	wstring iniPath;
	if( ! tc::GetTotalCmdLocation( NULL, &iniPath ) )
	{
		DebugOut( L"TotalCmd wincmd.ini not found." );
		return FALSE;
	}
	DebugOut( _T("Found TotalCmd wincmd.ini at %s\n"), iniPath.c_str() );

	// Read existing menu completely.

	tc::FavMenu items;
	if( ! tc::LoadFavoritesMenu( &items, iniPath.c_str(), L"DirMenu") )
	{
		DebugOut( L"Failed to load TotalCmd DirMenu." );
		return FALSE;
	}

	// Insert new menu item.
	
	tc::FavMenuItem newItem;
	newItem.menu = wstring( displayName );
	newItem.cmd = wstring( L"cd " ) + folderPath;

	if( items.empty() )
		items.push_back( newItem );
	else if( insertAfter == -1 )
		items.insert( items.begin() + items.size(), newItem );
	else
		items.insert( items.begin() + insertAfter, newItem );

	// Write modified menu back.

	if( tc::SaveFavoritesMenu( items, iniPath.c_str(), L"DirMenu" ) )
		return TRUE;

	DebugOut( L"Failed to save TotalCmd DirMenu." );

	return FALSE;
}