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
 */
#include "stdafx.h"
#include "ExplorerUtils.h"

using namespace std;
using namespace ffplug;

//----------------------------------------------------------------------------------------------------
/// Enumerate new Explorer favorites which are independent of IE favorites and available with 
/// Vista and newer OS.

BOOL Explorer_EnumFavorites( UINT index, ffplug::FavMenuItem* item )
{
	static wstring s_folderPath;
	static auto_ptr< FileFinder > s_finder;	

	if( index == 0 )
	{
		s_folderPath = GetKnownFolderPath( FOLDERID_Links );
		if( s_folderPath == L"" )
		{
			DebugOut( L"FOLDERID_Links not available." );
			return FALSE;
		}

		s_finder.reset( new FileFinder( s_folderPath.c_str(), L"*.lnk" ) );
	}

	if( ! s_finder.get() )
		return FALSE;

	if( ! s_finder->IsValid() )
	{
		s_finder.reset();
		return FALSE;
	}

	DebugOut( L"Link: %s\n", s_finder->GetFileName() );

	// Get target and description of link.
	CComPtr< IShellLink > link;
	if( SUCCEEDED( link.CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER ) ) )
	{
		CComQIPtr< IPersistFile > persistFile( link );
		if( persistFile )
		{
			if( SUCCEEDED( persistFile->Load( ( s_folderPath + s_finder->GetFileName() ).c_str(), STGM_READ ) ) )
			{
				link->GetIDList( &item->pidlFolder );

				LPCITEMIDLIST pidl = item->pidlFolder;
				SFGAOF flags = SFGAO_FOLDER;
				CComPtr< IShellFolder > desktopFolder;
				if( SUCCEEDED( ::SHGetDesktopFolder( &desktopFolder ) &&
					SUCCEEDED( desktopFolder->GetAttributesOf( 1, &pidl, &flags ) ) &&
					flags == SFGAO_FOLDER ) )
				{
					item->type = FavMenuItem::T_FOLDER_FAVORITE;

					WCHAR linkName[ MAX_PATH ] = L"";
					wcscpy_s( linkName, s_finder->GetFileName() );
					if( wchar_t* p = wcsrchr( linkName, '.' ) )
						*p = 0;						
					wcscpy_s( item->displayName, linkName );
				}
			}
		}
	}

	s_finder->FindNext();

	return TRUE;	
}

//----------------------------------------------------------------------------------------------------

BOOL Explorer_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter )
{
	
	return FALSE;
}

//----------------------------------------------------------------------------------------------------

BOOL IE_EnumFavorites( UINT index, FavMenuItem* item )
{
	
	return FALSE;
}

//----------------------------------------------------------------------------------------------------

BOOL IE_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter )
{

	return FALSE;
}
