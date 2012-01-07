
#pragma once

#include "FileDlg_base.h"

/// Load and show favorites menu.
void FavMenu_DisplayForFileDialog( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook,
	const PluginManager& pluginMgr, const Profile& profile );

/// Start favorites editor.
void FavMenu_StartEditor( HWND hWndParent );

/// Add folder to favorites menu.
void FavMenu_AddFolder( HWND hwndParent, const PluginManager& pluginMgr, const Profile& profile,
                        LPCWSTR displayName, PCIDLIST_ABSOLUTE folder );

/// Show a menu with the current folders of the application and other file managers.
void DisplayMenu_OpenDirs( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook, 
                           const PluginManager& pluginMgr );

HMENU CreateFolderMenu( const std::vector<tstring> &folderList, HMENU hMenu = NULL );

int DisplayFolderMenu( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook,
                       HMENU hMenu, int buttonID );

void AddCurrentFolderToHistory( FileDlgHook_base* fileDlgHook, Profile* profile );

void DisplayMenu_GlobalHist( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook, 
	const Profile& profile );

void DisplayMenu_Config( HINSTANCE hInstDll, HWND hFileDialog, HWND hToolWnd );

void GotoLastDir( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook, const Profile& profile );
