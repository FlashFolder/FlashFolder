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

#include <windows.h>

namespace ffplug {

struct PluginInfo
{
	WCHAR description[ 2048 ];
	UINT hostMinVersion;
};

/// Possible bit flags for FileMgrProgram::capabilities
enum FileMgrCapabilitiesFlags
{
	FMC_EnumFavorites        = 0x0001,
	FMC_AddFavoriteFolder    = 0x0002,
	FMC_StartFavoritesEditor = 0x0004,
	FMC_EnumCurrentFolders   = 0x0008
};

struct FileMgrProgram
{
	WCHAR id[ 128 ];            ///< Unique identifier of this program
	WCHAR displayName[ 256 ];   ///< User-friendly name of this program (like "Total Commander")
	WCHAR shortName[ 32 ];      ///< User-friendly abbreviated name of this program (like "TC")
	WCHAR websiteUrl[ 1024 ];   ///< Homepage of the program ("http://....")
	BOOL isInstalled;           ///< non-zero if this program is currently installed

	/// Functions supported for this program, see FileMgrCapabilitiesFlags.
	DWORD capabilities;
};

struct FavMenuItem
{
	WCHAR displayName[ 256 ];
	PIDLIST_ABSOLUTE pidlFolder;
	
	enum Type { T_UNKNOWN, T_FOLDER_FAVORITE, T_SEPARATOR, T_SUBMENU, T_SUBMENU_END }
		type;
};

struct FileMgrFolders
{
	PIDLIST_ABSOLUTE pidlFolder;
	PIDLIST_ABSOLUTE pidlFolder2;   // optional "target" folder for two-pane file managers
};

/// Initialize the plugin. Mandatory function.
typedef BOOL ( CALLBACK *P_FlashFolderPluginInit )( PluginInfo* info );

/// Enumerate the file managers supported by the plugin.
typedef BOOL ( CALLBACK *P_EnumSupportedFileManagers )( UINT index, FileMgrProgram* info );

/// Enumerate the favorites menu of the given file manager.
typedef BOOL ( CALLBACK *P_EnumFavorites )( LPCWSTR fileManagerId, UINT index, FavMenuItem* item );

/// Add an item to the favorites menu of the given file manager.
typedef BOOL ( CALLBACK *P_AddFavoriteFolder )( 
	LPCWSTR fileManagerId, LPCWSTR displayName, PCIDLIST_ABSOLUTE folder, INT insertAfter );

/// Start the build-in favorites menu editor of the file manager.
typedef BOOL ( CALLBACK *P_StartFavoritesEditor )( LPCWSTR fileManagerId );

/// Get the currently open folders of the file manager.
typedef BOOL ( CALLBACK *P_EnumCurrentFolders )( LPCWSTR fileManagerId, UINT index, FileMgrFolders* folders );

}; //namespace