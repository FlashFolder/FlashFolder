
#pragma once

#include "PluginApi.h"

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
		PluginInfo info;

		P_FlashFolderPluginInit fnFlashFolderPluginInit;
		P_EnumSupportedFileManagers fnEnumSupportedFileManagers;
		P_EnumFavorites fnEnumFavorites;
		P_AddFavoriteFolder fnAddFavoriteFolder;
		P_StartFavoritesEditor fnStartFavoritesEditor;
		P_EnumCurrentFolders fnEnumCurrentFolders;

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
		FileMgrProgram program;
	};

	/// Container of file managers supported by a plugin.
	typedef std::vector< FileMgr > FileMgrs;

	/// Get supported file managers from all loaded plugins.
	FileMgrs GetSupportedFileManagers() const;

	/// Container of favorites item.
	typedef std::vector< FavMenuItem > FavMenuItems;

	/// Get favorites from a file manager.
	bool GetFavMenu( LPCWSTR pluginName, LPCWSTR programId, PluginManager::FavMenuItems* result ) const;

	/// Add a favorite folder to the favorites menu.
	bool AddFavMenuItem( LPCWSTR pluginName, LPCWSTR programId, LPCWSTR displayName, LPCWSTR folder ) const;

	/// Current folder associated with a file manager.
	struct CurrentFolder
	{
		std::wstring folder;
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
