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
#include "ExplorerUtils.h"

namespace tc = totalcmdutils;
using namespace std;

BOOL Explorer_EnumFavorites( UINT index, ffplug::FavMenuItem* item );
BOOL Explorer_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter );
BOOL IE_EnumFavorites( UINT index, ffplug::FavMenuItem* item );
BOOL IE_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter );
BOOL TotalCmd_EnumFavorites( UINT index, ffplug::FavMenuItem* item );
BOOL TotalCmd_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter );

//-----------------------------------------------------------------------------------------
/// Programs and capabilities supported by this plugin.

ffplug::FileMgrProgram g_fileMgrPrograms[] = 
{
	{ L"ID_FlashFolder", L"FlashFolder", L"FF", L"", TRUE, 
		ffplug::FMC_EnumFavorites | ffplug::FMC_AddFavoriteFolder | ffplug::FMC_StartFavoritesEditor },

	{ L"ID_Explorer", L"Windows Explorer", L"Explorer", L"", TRUE, 
		ffplug::FMC_EnumCurrentFolders },

	{ L"ID_InternetExplorer", L"Internet Explorer", L"IE", L"", TRUE, 
		ffplug::FMC_EnumFavorites | ffplug::FMC_AddFavoriteFolder | ffplug::FMC_StartFavoritesEditor },

	{ L"ID_TotalCmd", L"Total Commander®", L"TC", L"http://www.ghisler.com", FALSE, 
		ffplug::FMC_EnumFavorites | ffplug::FMC_AddFavoriteFolder | ffplug::FMC_EnumCurrentFolders },
};

//-----------------------------------------------------------------------------------------
//// DLL entry point.

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
			// Disable unneeded DLL-callbacks (DLL_THREAD_ATTACH).
			::DisableThreadLibraryCalls( hModule );

			DebugOut( L"[DefFileMgrPlugin] DLL_PROCESS_ATTACH (pid %d)", ::GetCurrentProcessId() );
			break;

		case DLL_PROCESS_DETACH:
			DebugOut( L"[DefFileMgrPlugin] DLL_PROCESS_DETACH (pid %d)", ::GetCurrentProcessId() );
			break;
	}
    return TRUE;
}

//-----------------------------------------------------------------------------------------
/// Initialize the plugin.

BOOL CALLBACK FlashFolderPluginInit( ffplug::PluginInfo* info )
{
	wcscpy_s( info->description, L"This plugin supports the favorites menu of popular file managers." );

	return TRUE;
}

//-----------------------------------------------------------------------------------------
/// Enumerate the file managers supported by the plugin.

BOOL CALLBACK EnumSupportedFileManagers( UINT index, ffplug::FileMgrProgram* info )
{
	if( index < _countof( g_fileMgrPrograms ) )
	{
		*info = g_fileMgrPrograms[ index ];

		if( wcscmp( info->id, L"ID_Explorer" ) == 0 )
		{	
			// Explorer favorites are only available on Vista and newer OS.
			if( GetKnownFolderPath( FOLDERID_Links ) != L"" )
				info->capabilities |= ffplug::FMC_EnumFavorites | 
				                      ffplug::FMC_AddFavoriteFolder | 
				                      ffplug::FMC_StartFavoritesEditor;
		}
		else if( wcscmp( info->id, L"ID_TotalCmd" ) == 0 )
		{
			info->isInstalled = tc::GetTotalCmdLocation() ? TRUE : FALSE;
		}

		return TRUE;
	}

	return FALSE;  // No more items.
}

//-----------------------------------------------------------------------------------------
/// Enumerate the favorites menu of the given file manager.

BOOL CALLBACK EnumFavorites( LPCWSTR fileManagerId, UINT index, ffplug::FavMenuItem* item )
{
	if( wcscmp( fileManagerId, L"ID_Explorer" ) == 0 )
		return Explorer_EnumFavorites( index, item );
	else if( wcscmp( fileManagerId, L"ID_InternetExplorer" ) == 0 )
		return IE_EnumFavorites( index, item );
	else if( wcscmp( fileManagerId, L"ID_TotalCmd" ) == 0 )
		return TotalCmd_EnumFavorites( index, item );

	return FALSE;
}

//-----------------------------------------------------------------------------------------
/// Add an item to the favorites menu of the given file manager.

BOOL CALLBACK AddFavoriteFolder( 
	LPCWSTR fileManagerId, LPCWSTR displayName, PCIDLIST_ABSOLUTE folder, INT insertAfter )
{
	WCHAR folderPath[ MAX_PATH ] = L"";
	if( ! ::SHGetPathFromIDList( folder, folderPath ) )
		return FALSE;

	if( wcscmp( fileManagerId, L"ID_Explorer" ) == 0 )
		return  Explorer_AddFavoriteFolder( displayName, folderPath, insertAfter );	
	else if( wcscmp( fileManagerId, L"ID_InternetExplorer" ) == 0 )
		return IE_AddFavoriteFolder( displayName, folderPath, insertAfter );	
	else if( wcscmp( fileManagerId, L"ID_TotalCmd" ) == 0 )
		return TotalCmd_AddFavoriteFolder( displayName, folderPath, insertAfter );	

	return FALSE;
}

//-----------------------------------------------------------------------------------------
/// Start the build-in favorites menu editor of the file manager.

BOOL CALLBACK StartFavoritesEditor( LPCWSTR fileManagerId )
{
	// TODO

	return FALSE;
}

//-----------------------------------------------------------------------------------------
/// Enumerate current folders of running file managers.

BOOL CALLBACK EnumCurrentFolders( LPCWSTR fileManagerId, UINT index, ffplug::FileMgrFolders* folders )
{
	if( wcscmp( fileManagerId, L"ID_TotalCmd" ) == 0 )
	{
		if( index == 0 )
		{
			tc::TcInstance tc( tc::FindTopTcWnd() );
			WCHAR path[ MAX_PATH ] = L"";
			WCHAR path2[ MAX_PATH ] = L"";
			if( tc.GetDirs( path, _countof( path ), path2, _countof( path2 ) ) )
			{
				GetPidlFromPath( &folders->pidlFolder, path );
				GetPidlFromPath( &folders->pidlFolder2, path2 );
				return TRUE;
			}
		}
	}
	else if( wcscmp( fileManagerId, L"ID_Explorer" ) == 0 )
	{
		static std::vector< PIDLIST_ABSOLUTE > s_folders;
		if( index == 0 )
		{
			GetCurrentExplorerFolders( &s_folders );
			if( ! s_folders.empty() )
			{
				folders->pidlFolder = s_folders[ 0 ];
				return TRUE;
			}
		}
		else if( index < s_folders.size() )
		{
			folders->pidlFolder2 = s_folders[ index ];
			return TRUE;
		}
	}
	
	return FALSE;
}

