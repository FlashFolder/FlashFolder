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
#include "StdAfx.h"
#include "PluginManager.h"

using namespace std;

//-------------------------------------------------------------------------------------------------------------

PluginManager::PluginManager( HINSTANCE hInst, UINT flags ) :
	m_flags( flags )
{
	WCHAR pluginDir[ 1024 ] = L"";
	GetAppDir( hInst, pluginDir, _countof( pluginDir ) );

#ifdef _WIN64
	wcscat_s( pluginDir, L"plugins64" );
#else
	wcscat_s( pluginDir, L"plugins32" );
#endif
	::PathAddBackslash( pluginDir );
	DebugOut( L"Loading FlashFolder plugins from '%s'", pluginDir );

	// Enumerate sub folders, each folder may contain one ore more plugins.

	for( FileFinder finder( pluginDir ); finder; finder.FindNext() )
	{
		if( ! finder.IsDir() )
			continue;

		// Enumerate plugins. We use a special file extension so we will ignore
		// additional DLLs (like VC runtime) of the plugin.
	
		tstring subDir = tstring( pluginDir ) + finder.GetFileName();
		for( FileFinder pluginFinder( subDir.c_str(), L"*.ffplugin" ); pluginFinder; pluginFinder.FindNext() )
		{
			tstring path = tstring( subDir ) + L"\\" + tstring( pluginFinder.GetFileName() );
			DebugOut( L"Loading plugin '%s'", path.c_str() );

			Module module( path.c_str() );
			if( ! module )
			{
				DebugOut( L"Failed to load plugin, error %d", ::GetLastError() );
				continue;
			}

			Plugin plugin;
			plugin.name = pluginFinder.GetFileName();

			plugin.fnFlashFolderPluginInit = reinterpret_cast<ffplug::P_FlashFolderPluginInit>(
				::GetProcAddress( module, "FlashFolderPluginInit" ) );
			if( ! plugin.fnFlashFolderPluginInit )
			{
				DebugOut( L"FlashFolderPluginInit() function missing from plugin." );
				continue;
			}

			DebugOut( L"Calling FlashFolderPluginInit()" );
			memset( &plugin.info, 0, sizeof( plugin.info ) );
			if( ! plugin.fnFlashFolderPluginInit( &plugin.info ) )
			{
				DebugOut( L"Plugin returned FALSE" );
				continue;
			}

			plugin.fnEnumSupportedFileManagers = reinterpret_cast<ffplug::P_EnumSupportedFileManagers>(
				::GetProcAddress( module, "EnumSupportedFileManagers" ) );
			plugin.fnEnumFavorites = reinterpret_cast<ffplug::P_EnumFavorites>(
				::GetProcAddress( module, "EnumFavorites" ) );
			plugin.fnAddFavoriteFolder = reinterpret_cast<ffplug::P_AddFavoriteFolder>(
				::GetProcAddress( module, "AddFavoriteFolder" ) );
			plugin.fnStartFavoritesEditor = reinterpret_cast<ffplug::P_StartFavoritesEditor>(
				::GetProcAddress( module, "StartFavoritesEditor" ) );
			plugin.fnEnumCurrentFolders = reinterpret_cast<ffplug::P_EnumCurrentFolders>(
				::GetProcAddress( module, "EnumCurrentFolders" ) );

			plugin.handle = module.Detach();

			m_plugins.push_back( plugin );
		}
	}
}

//----------------------------------------------------------------------------------------------------

PluginManager::~PluginManager()
{
	if( ! ( m_flags & FL_NO_AUTOUNLOAD ) )
	{
		DebugOut( L"Unloading FlashFolder plugins" );

		foreach( const Plugin& plugin, m_plugins )
			::FreeLibrary( plugin.handle );
	}
}

//----------------------------------------------------------------------------------------------------

void PluginManager::UnloadPlugins()
{
	DebugOut( L"Unloading FlashFolder plugins" );

	foreach( const Plugin& plugin, m_plugins )
		::FreeLibrary( plugin.handle );

	m_plugins.clear();
}

//----------------------------------------------------------------------------------------------------

int PluginManager::FindPlugin( LPCWSTR pluginName ) const
{
	int i = 0;
	foreach( const Plugin& plugin, m_plugins )
	{
		if( _wcsicmp( plugin.name.c_str(), pluginName ) == 0 )
			return i;
		++i;
	}
	return -1;
}

//-------------------------------------------------------------------------------------------------------------

PluginManager::FileMgrs PluginManager::GetSupportedFileManagers() const
{
	FileMgrs result;

	int nPlugin = 0;
	foreach( const Plugin& plugin, m_plugins )
	{
		if( plugin.fnEnumSupportedFileManagers )
		{
			DebugOut( L"Enumerating file managers supported by '%s'", plugin.name.c_str() );

			for( UINT i = 0; i < 50; ++i )
			{
				FileMgr fm;
				fm.nPlugin = nPlugin;

				memset( &fm.program, 0, sizeof( fm.program ) );
				if( ! plugin.fnEnumSupportedFileManagers( i, &fm.program ) )
					break;

				DebugOut( L"  ID=%s, displayName=%s", fm.program.id, fm.program.displayName ); 

				result.push_back( fm );
			}
		}

		++nPlugin;
	}

	return result;
}

//----------------------------------------------------------------------------------------------------

bool PluginManager::GetFavMenu( LPCWSTR pluginName, LPCWSTR programId, FavoriteItems* result ) const
{
	::SetLastError( ERROR_SUCCESS );
	result->clear();

	int nPlugin = FindPlugin( pluginName );
	if( nPlugin == -1 )
	{
		::SetLastError( ERROR_INVALID_PARAMETER );
		return false;
	}

	const Plugin& plugin = m_plugins[ nPlugin ];

	if( ! plugin.fnEnumFavorites )
	{
		::SetLastError( ERROR_INVALID_FUNCTION );
		return false;
	}

	for( UINT i = 0; i < 500; ++i )
	{
		ffplug::FavMenuItem menuItem = { 0 };
		if( ! plugin.fnEnumFavorites( programId, i, &menuItem ) )
			break;

		FavoriteItem item;
		item.displayName = menuItem.displayName;
		item.type = menuItem.type;
		item.folder = MakeSharedPidl( menuItem.pidlFolder );

		result->push_back( item );
	}

	return true;
}

//----------------------------------------------------------------------------------------------------

bool PluginManager::AddFavMenuItem( 
	LPCWSTR pluginName, LPCWSTR programId, LPCWSTR displayName, PCIDLIST_ABSOLUTE folder ) const
{
	::SetLastError( ERROR_SUCCESS );

	int nPlugin = FindPlugin( pluginName );
	if( nPlugin == -1 )
	{
		::SetLastError( ERROR_INVALID_PARAMETER );
		return false;
	}

	const Plugin& plugin = m_plugins[ nPlugin ];

	if( ! plugin.fnAddFavoriteFolder )
	{
		::SetLastError( ERROR_INVALID_FUNCTION );
		return false;
	}

	plugin.fnAddFavoriteFolder( programId, displayName, folder, -1 );
	
	return true;
}

//----------------------------------------------------------------------------------------------------

PluginManager::CurrentFolders PluginManager::GetCurrentFolders() const
{
	CurrentFolders result;

	FileMgrs fileMgrs = GetSupportedFileManagers();

	foreach( const FileMgr& fm, fileMgrs )
	{
		if( ! fm.program.isInstalled )
			continue;

		const Plugin& plugin = m_plugins[ fm.nPlugin ];

		if( plugin.fnEnumCurrentFolders )
		{
			DebugOut( L"Enumerating current folders of plugin '%s', programId '%s'",
				plugin.name.c_str(), fm.program.id );

			CurrentFolder resultItem;
			resultItem.programShortName = wstring( fm.program.shortName );

			for( UINT i = 0; i < 50; ++i )
			{
				ffplug::FileMgrFolders folders = { 0 };
				if( ! plugin.fnEnumCurrentFolders( fm.program.id, i, &folders ) )
					break;

				resultItem.folder = MakeSharedPidl( folders.pidlFolder );
				result.push_back( resultItem );

				if( folders.pidlFolder2 )
				{
					resultItem.folder = MakeSharedPidl( folders.pidlFolder2 );
					result.push_back( resultItem );
				}
			}
		}
	}

	return result;
}
