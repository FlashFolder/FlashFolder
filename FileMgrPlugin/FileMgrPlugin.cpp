
#include "stdafx.h"

using namespace std;

BOOL TotalCmd_EnumFavorites( UINT index, FavMenuItem* item );
BOOL TotalCmd_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter );

//-----------------------------------------------------------------------------------------

FileMgrProgram g_fileMgrPrograms[] = 
{
	{ L"ID_FlashFolder", L"FlashFolder", L"FF", L"", TRUE, 
		FMC_EnumFavorites | FMC_AddFavoriteFolder | FMC_StartFavoritesEditor },

	{ L"ID_Explorer", L"Windows Explorer", L"Explorer", L"", TRUE, 
		FMC_EnumFavorites | FMC_AddFavoriteFolder | FMC_StartFavoritesEditor | FMC_EnumCurrentFolders },

	{ L"ID_InternetExplorer", L"Internet Explorer", L"IE", L"", TRUE, 
		FMC_EnumFavorites | FMC_AddFavoriteFolder | FMC_StartFavoritesEditor },

	{ L"ID_TotalCmd", L"Total Commander®", L"TC", L"http://www.ghisler.com", FALSE, 
		FMC_EnumFavorites | FMC_AddFavoriteFolder | FMC_EnumCurrentFolders },

	{ L"ID_SpeedCmd", L"Speed Commander®", L"SC", L"http://www.speedproject.com", FALSE, 
		FMC_EnumFavorites | FMC_AddFavoriteFolder | FMC_EnumCurrentFolders }
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
			break;

		case DLL_PROCESS_DETACH:
			break;
	}
    return TRUE;
}

//-----------------------------------------------------------------------------------------

/// Initialize the plugin.
BOOL CALLBACK FlashFolderPluginInit( PluginInfo* info )
{
	wcscpy_s( info->description, L"This plugin supports the favorites menu of popular file managers." );

	return TRUE;
}

//-----------------------------------------------------------------------------------------

/// Enumerate the file managers supported by the plugin.
BOOL CALLBACK EnumSupportedFileManagers( UINT index, FileMgrProgram* info )
{
	if( index < _countof( g_fileMgrPrograms ) )
	{
		*info = g_fileMgrPrograms[ index ];

		if( wcscmp( info->id, L"ID_TotalCmd" ) == 0 )
			info->isInstalled = GetTotalCmdLocation() ? TRUE : FALSE;
		else if( wcscmp( info->id, L"ID_SpeedCmd" ) == 0 )
			info->isInstalled = FALSE;

		return TRUE;
	}

	return FALSE;  // No more items.
}

//-----------------------------------------------------------------------------------------

/// Enumerate the favorites menu of the given file manager.
BOOL CALLBACK EnumFavorites( LPCWSTR fileManagerId, UINT index, FavMenuItem* item )
{
	if( wcscmp( fileManagerId, L"ID_TotalCmd" ) == 0 )
		return TotalCmd_EnumFavorites( index, item );

	return FALSE;
}

//-----------------------------------------------------------------------------------------

/// Add an item to the favorites menu of the given file manager.
BOOL CALLBACK AddFavoriteFolder( 
	LPCWSTR fileManagerId, LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter )
{
	if( wcscmp( fileManagerId, L"ID_TotalCmd" ) == 0 )
		return TotalCmd_AddFavoriteFolder( displayName, folderPath, insertAfter );	

	return FALSE;
}

//-----------------------------------------------------------------------------------------

/// Start the build-in favorites menu editor of the file manager.
BOOL CALLBACK StartFavoritesEditor( LPCWSTR fileManagerId )
{

	return FALSE;
}

//-----------------------------------------------------------------------------------------

BOOL CALLBACK EnumCurrentFolders( LPCWSTR fileManagerId, UINT index, FileMgrFolders* folders )
{
	if( wcscmp( fileManagerId, L"ID_TotalCmd" ) == 0 )
	{
		if( index == 0 )
		{
			CTotalCmdUtils utils( FindTopTcWnd() );
			if( utils.GetDirs( folders->folder, _countof( folders->folder ), 
							   folders->folder2, _countof( folders->folder2 ) ) )
				return TRUE;
		}
	}
	else if( wcscmp( fileManagerId, L"ID_Explorer" ) == 0 )
	{
		static std::vector< wstring > s_pathes;
		if( index == 0 )
		{
			if( GetAllExplorerPathes( &s_pathes ) > 0 )
			{
				wcscpy_s( folders->folder, s_pathes[ 0 ].c_str() );
				return TRUE;
			}
		}
		else if( index < s_pathes.size() )
		{
			wcscpy_s( folders->folder, s_pathes[ index ].c_str() );
			return TRUE;
		}
	}
	
	return FALSE;
}

//-----------------------------------------------------------------------------------------

// Get directory menu from TotalCommander profile.

BOOL TotalCmd_EnumFavorites( UINT index, FavMenuItem* item )
{
	static wstring s_iniPath;
	if( index == 0 )
	{
		if( ! GetTotalCmdLocation( NULL, &s_iniPath ) )
		{
			DebugOut( L"TotalCmd wincmd.ini not found." );
			return FALSE;
		}
		DebugOut( _T("Found TotalCmd wincmd.ini at %s\n"), s_iniPath.c_str() ); 
	}

	++index;   // TC uses 1-based index

	WCHAR key[ 32 ] = L"";
	WCHAR menu[ 256 ] = L"";
	WCHAR cmd[ 1024 ] = L"";

	swprintf_s( key, L"menu%d", index );
	if( ! ::GetPrivateProfileString( L"DirMenu", key, L"", menu, _countof( menu ), s_iniPath.c_str() ) )
		return FALSE;

	item->type = FavMenuItem::T_UNKNOWN;

	if( wcscmp( menu, L"-" ) == 0 )
	{
		item->type = FavMenuItem::T_SEPARATOR;
	}
	else if( wcscmp( menu, L"--" ) == 0 )
	{
		item->type = FavMenuItem::T_SUBMENU_END;
	}
	else if( menu[ 0 ] == L'-' )
	{
		item->type = FavMenuItem::T_SUBMENU;
		wcscpy_s( item->displayName, &menu[ 1 ] );
	}
	else
	{
		wcscpy_s( item->displayName, menu );

		swprintf_s( key, L"cmd%d", index );
		if( ::GetPrivateProfileString( L"DirMenu", key, L"", cmd, _countof( cmd ), s_iniPath.c_str() ) )
		{
			if( _wcsnicmp( cmd, L"cd ", 3 ) == 0 )
			{
				item->type = FavMenuItem::T_FOLDER_FAVORITE;
				wcscpy_s( item->folder, &cmd[ 3 ] );

				// try to get optional target path
				swprintf_s( key, L"path%d", index );
				::GetPrivateProfileString( L"DirMenu", key, L"", item->folder2, _countof( item->folder2 ), 
				                           s_iniPath.c_str() );
			}
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------------------

BOOL TotalCmd_AddFavoriteFolder( LPCWSTR displayName, LPCWSTR folderPath, INT insertAfter )
{
	wstring iniPath;
	if( ! GetTotalCmdLocation( NULL, &iniPath ) )
	{
		DebugOut( L"TotalCmd wincmd.ini not found." );
		return FALSE;
	}
	DebugOut( _T("Found TotalCmd wincmd.ini at %s\n"), iniPath.c_str() );

	// Read existing menu completely without interpreting it to minimize risk of damage.

	WCHAR key[ 32 ] = L"";
	WCHAR buf[ 1024 ] = L"";

	if( ::GetPrivateProfileString( L"DirMenu", L"RedirectSection", L"", buf, _countof( buf ), iniPath.c_str() ) )
	{
		DebugOut( L"wincmd.ini - RedirectSection is currently not supported." );
		return FALSE;
	}

	struct Item	{ wstring menu, cmd, path, param; };
	vector< Item > items;

	for( int i = 1;; ++i )
	{
		swprintf_s( key, L"menu%d", i );
		if( ! ::GetPrivateProfileString( L"DirMenu", key, L"", buf, _countof( buf ), iniPath.c_str() ) )
			break;

		Item item;
		item.menu = wstring( buf );

		swprintf_s( key, L"cmd%d", i );
		if( ::GetPrivateProfileString( L"DirMenu", key, L"", buf, _countof( buf ), iniPath.c_str() ) )
			item.cmd = wstring( buf );
	
		swprintf_s( key, L"path%d", i );
		if( ::GetPrivateProfileString( L"DirMenu", key, L"", buf, _countof( buf ), iniPath.c_str() ) )
			item.path = wstring( buf );

		swprintf_s( key, L"param%d", i );
		if( ::GetPrivateProfileString( L"DirMenu", key, L"", buf, _countof( buf ), iniPath.c_str() ) )
			item.param = wstring( buf );

		items.push_back( item );
	}

	// Insert new menu item.
	
	Item newItem;
	newItem.menu = wstring( displayName );
	newItem.cmd = wstring( L"cd " ) + folderPath;

	if( items.empty() )
		items.push_back( newItem );
	else if( insertAfter == -1 )
		items.insert( items.begin() + items.size(), newItem );
	else
		items.insert( items.begin() + insertAfter, newItem );

	// Wipe out old menu.

	::WritePrivateProfileSection( L"DirMenu", L"\0", iniPath.c_str() );

	// Write modified menu back.

	for( size_t i = 0; i < items.size(); ++i )
	{
		const Item& item = items[ i ];

		swprintf_s( key, L"menu%d", i +  1 );
		::WritePrivateProfileString( L"DirMenu", key, item.menu.c_str(), iniPath.c_str() );

		if( ! item.cmd.empty() )
		{
			swprintf_s( key, L"cmd%d", i + 1 );
			::WritePrivateProfileString( L"DirMenu", key, item.cmd.c_str(), iniPath.c_str() );
		}

		if( ! item.path.empty() )
		{
			swprintf_s( key, L"path%d", i + 1 );
			::WritePrivateProfileString( L"DirMenu", key, item.path.c_str(), iniPath.c_str() );
		}

		if( ! item.param.empty() )
		{
			swprintf_s( key, L"param%d", i + 1 );
			::WritePrivateProfileString( L"DirMenu", key, item.path.c_str(), iniPath.c_str() );
		}
	}	

	return TRUE;
}