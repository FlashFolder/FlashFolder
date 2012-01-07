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
#pragma once

#include "PluginApi.h"
#include <commonUtils\ItemIdList.h>

//-------------------------------------------------------------------------------------------------------------
/// Manages FlashFolder plugins.

class PluginManager : boost::noncopyable
{
public:
	/// \name Generic plugin management
	//@{

	/// Plugin information
	struct Plugin
	{
		HMODULE handle;
		std::wstring name;
		ffplug::PluginInfo info;

		ffplug::P_FlashFolderPluginInit fnFlashFolderPluginInit;
		ffplug::P_EnumSupportedFileManagers fnEnumSupportedFileManagers;
		ffplug::P_EnumFavorites fnEnumFavorites;
		ffplug::P_AddFavoriteFolder fnAddFavoriteFolder;
		ffplug::P_StartFavoritesEditor fnStartFavoritesEditor;
		ffplug::P_EnumCurrentFolders fnEnumCurrentFolders;

		Plugin() :
			handle( NULL ),
			fnFlashFolderPluginInit( NULL ),
			fnEnumSupportedFileManagers( NULL ),
			fnEnumFavorites( NULL ),
			fnAddFavoriteFolder( NULL ),
			fnStartFavoritesEditor( NULL ),
			fnEnumCurrentFolders( NULL )
		{}
	};

	/// Constructor bit flags
	enum Flags
	{
		FL_DEFAULT       = 0x0000,
		FL_NO_AUTOUNLOAD = 0x0001
	};

	/// Loads all available plugins and gets plugin functions.
	PluginManager( HINSTANCE hInst = NULL, UINT flags = FL_DEFAULT );

	/// Automatically frees all loaded plugins, unless the constructor specified not to do so.
	~PluginManager();

	/// Manually unload all plugins.
	void UnloadPlugins();

	/// Get number of loaded plugins.
	size_t GetPluginCount() const { return m_plugins.size(); }

	/// Get information about a loaded plugin.
	const Plugin& GetPlugin( size_t i ) const { return m_plugins[ i ]; }

	/// Get the index of a loaded plugin by name. Returns -1 if not found.
	int FindPlugin( LPCWSTR pluginName ) const;

	//@}
	/// \name Helper functions for calling plugin API.
	//@{

	/// File manager supported by a plugin.
	struct FileMgr
	{
		int nPlugin;
		ffplug::FileMgrProgram program;
	};

	/// Container of file managers supported by a plugin.
	typedef std::vector< FileMgr > FileMgrs;

	/// Get supported file managers from all loaded plugins.
	FileMgrs GetSupportedFileManagers() const;

	/// Favorites menu item (encapsulates PIDL allocation compared to FavMenuItem ).
	struct FavoriteItem
	{
		std::wstring displayName;
		SpITEMIDLIST folder;
		ffplug::FavMenuItem::Type type;
	};

	/// Container of favorites item.
	typedef std::vector< FavoriteItem > FavoriteItems;

	/// Get favorites from a file manager.
	bool GetFavMenu( LPCWSTR pluginName, LPCWSTR programId, FavoriteItems* result ) const;

	/// Add a favorite folder to the favorites menu.
	bool AddFavMenuItem( LPCWSTR pluginName, LPCWSTR programId, LPCWSTR displayName, PCIDLIST_ABSOLUTE folder ) const;

	/// Current folder associated with a file manager.
	struct CurrentFolder
	{
		SpcITEMIDLIST folder;
		std::wstring programShortName;
	};

	/// Current folders
	typedef std::vector< CurrentFolder > CurrentFolders;
		
	/// Get the current folders of all supported file managers.
	CurrentFolders GetCurrentFolders() const; 

	//@}

private:
	std::vector< Plugin > m_plugins;
	UINT m_flags;
};
