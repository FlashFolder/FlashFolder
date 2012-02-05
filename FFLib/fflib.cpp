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

/** \file Main file to compile the "fflib.dll".
 *  This DLL is the work horse of FlashFolder - it contains a hook function
 *  that watches every window activation in the system. If a window is a common
 *  file dialog, FlashFolder subclasses the window procedure.
 */

#include "stdafx.h"

#pragma warning(disable:4786)  // disable STL-template-related warnings
#pragma warning(disable:4995)  // disable strsafe.h warnings

#include "fflib.h"
#include "fflib_exports.h"
#include "resource.h"
#include "HookManager.h"
#include "ToolWnd.h"
#include "HookBase.h"
#include "CmnFileDlgHook.h"
#include "MsoFileDlgHook.h"
#include "CmnFolderDlgHook.h"
#include "Utils.h"

using namespace std;

//-----------------------------------------------------------------------------------------
// global variables  

boost::scoped_ptr< FileDlgHookBase > g_spFileDlgHook;       // ptr to the hook instance
boost::scoped_ptr< ToolWnd > g_toolWnd;

RegistryProfile g_profile;   
MemoryProfile g_profileDefaults;

TCHAR g_currentExePath[ MAX_PATH + 1 ] = L"";
LPCTSTR g_currentExeName = L"";
TCHAR g_currentExeDir[ MAX_PATH + 1 ] = L"";

boost::scoped_ptr< PluginManager > g_pluginMgr;

//-----------------------------------------------------------------------------------------
// Prototypes

void GetSharedData();
HACCEL CreateMyAcceleratorTable();
LPCTSTR GetCommandName( int cmd );


//-----------------------------------------------------------------------------------------
// DLL entry point

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch( ul_reason_for_call )
    {
		case DLL_PROCESS_ATTACH:
		{
			// Disable unneeded DLL-callbacks (DLL_THREAD_ATTACH).
			::DisableThreadLibraryCalls( hModule );

			::GetModuleFileName( NULL, g_currentExePath, MAX_PATH );
			if( LPTSTR p = _tcsrchr( g_currentExePath, L'\\' ) )
			{
				g_currentExeName = p + 1;
				StringCbCopy( g_currentExeDir, sizeof(g_currentExeDir), g_currentExePath );
				g_currentExeDir[ p - g_currentExePath ] = 0;
			}
			
			DebugOut( L"[fflib] DLL_PROCESS_ATTACH (pid %08Xh, \"%s\"\n", 
				::GetCurrentProcessId(), g_currentExePath );

			// Register unique class name so FF can be identified easily by other tools.
			WNDCLASS wc = { 0 };
			wc.lpszClassName = FF_WNDCLASSNAME;
			wc.hInstance = hModule;
			wc.hCursor = ::LoadCursor( NULL, IDC_ARROW );
			wc.lpfnWndProc = DefDlgProc;
			wc.cbWndExtra = DLGWINDOWEXTRA;
			::RegisterClass( &wc );
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			DebugOut( L"[fflib] DLL_PROCESS_DETACH (pid %08Xh)\n", ::GetCurrentProcessId() );

			::UnregisterClass( FF_WNDCLASSNAME, hModule );
		}
		break;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------------------
// callbacks that will be called by the file dialog hook

namespace FileDlgHookCallbacks
{
	void OnInitDone()
	{
		g_toolWnd->OnFileDialogInitDone();
	}

	void OnFolderChange()
	{
		g_toolWnd->OnFileDialogFolderChange();
	}

	void OnResize()
	{
		g_toolWnd->OnFileDialogResize();
	}

	void OnEnable( bool bEnable )
	{
		g_toolWnd->OnFileDialogEnable( bEnable );
	}

	void OnShow( bool bShow )
	{
		g_toolWnd->OnFileDialogShow( bShow );
	}

	void OnActivate( WPARAM wParam, LPARAM lParam )
	{
		g_toolWnd->OnFileDialogActivate( wParam, lParam );
	}

	void SetTimer( DWORD interval )
	{
		g_toolWnd->OnFileDialogSetTimer( interval );
	}

	void OnDestroy( bool isOkBtnPressed )
	{
		// add folder to history if file dialog was closed with OK

		if( isOkBtnPressed )
			AddCurrentFolderToHistory( *g_spFileDlgHook );

		g_toolWnd->OnFileDialogDestroy( isOkBtnPressed );

		g_toolWnd.reset();
		g_spFileDlgHook.reset();
		
	}
}; //namespace FileDlgHookCallbacks

//-----------------------------------------------------------------------------------------------
// Check if hook for the current program and the given type of dialog is enabled.

bool IsCurrentProgramEnabledForDialog( FileDlgType fileDlgType )
{
	// Check if FlashFolder is globally disabled for given kind of dialog.
	TCHAR* pProfileGroup = L"";
	switch( fileDlgType.mainType )
	{
		case FDT_COMMON:
			pProfileGroup = L"CommonFileDlg";
		break;
		case FDT_MSOFFICE:
			pProfileGroup = L"MSOfficeFileDlg";
		break;
		case FDT_COMMON_FOLDER:
			pProfileGroup = L"CommonFolderDlg";
		break;	
	}
	if( g_profile.GetInt( pProfileGroup, L"EnableHook" ) == 0 )
		return false;

	// Check if EXE filename is in the excludes list for given dialog type.
	tstring excludesGroup = pProfileGroup;
	excludesGroup += L".Excludes";
	for( int i = 0;; ++i )
	{
		TCHAR key[10];
		StringCbPrintf( key, sizeof(key), L"%d", i );
		tstring path = g_profile.GetString( excludesGroup.c_str(), key );
		if( path.empty() )
			break;
		if( _tcsicmp( g_currentExeName, path.c_str() ) == 0 )
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------------------
// Initialise hook for given file dialog.

void InitHook( HWND hwnd, FileDlgType fileDlgType )
{
	// Load plugins if not already done for this process.
	// We specify FL_NO_AUTOUNLOAD so the plugin DLLs will not be unloaded automatically during 
	// process termination (see DllMain()).
	if( ! g_pluginMgr )
		g_pluginMgr.reset( new PluginManager( _AtlBaseModule.GetModuleInstance(), PluginManager::FL_NO_AUTOUNLOAD ) );

	g_spFileDlgHook.reset();

	// create an instance of a file dialog hook class depending on the
	// type of file dialog
	switch( fileDlgType.mainType )
	{
		case FDT_COMMON:
			g_spFileDlgHook.reset( new CmnFileDlgHook );
			break;
		case FDT_MSOFFICE:
			g_spFileDlgHook.reset( new MsoFileDlgHook( fileDlgType.subType ) );
			break;
		case FDT_COMMON_FOLDER:
			g_spFileDlgHook.reset( new CmnFolderDlgHook );
			break;
	}

	if( g_spFileDlgHook )
	{
		bool isFileDialog = fileDlgType.mainType == FDT_COMMON || fileDlgType.mainType == FDT_MSOFFICE;
		g_toolWnd.reset( new ToolWnd );
		g_toolWnd->Create( hwnd, isFileDialog, g_spFileDlgHook.get(), g_pluginMgr.get() );
	
		g_spFileDlgHook->Init( hwnd, g_toolWnd->m_hWnd );
	}
}

//-----------------------------------------------------------------------------------------
/// This function gets called in the context of the hooked process.
/// (DLL-Export)

LRESULT CALLBACK FFHook_CBT( int nCode, WPARAM wParam, LPARAM lParam )
{
	HWND hwnd = reinterpret_cast<HWND>( wParam );

	if( nCode == HCBT_ACTIVATE )
	{
		// Check whether a file dialog must be hooked.
		// For now, we can only handle one running file dialog per application, but
		// this should be enough in nearly all cases.
		if( ! g_spFileDlgHook )
		{
			FileDlgType fileDlgType = GetFileDlgType( hwnd );
			if( fileDlgType.mainType != FDT_NONE )
			{
				DebugOut( L"[fflib] file dialog detected, type %d.%d", 
					fileDlgType.mainType, fileDlgType.subType );
			
				if( g_profileDefaults.IsEmpty() )
				{
					GetProfileDefaults( &g_profileDefaults );
					g_profile.SetRoot( L"zett42\\FlashFolder" );
					g_profile.SetDefaults( &g_profileDefaults );
				}

				if( IsCurrentProgramEnabledForDialog( fileDlgType ) )
					InitHook( hwnd, fileDlgType );
			}
		} //if		
	} //if

	return CallNextHookEx( NULL, nCode, wParam, lParam );
}

//-----------------------------------------------------------------------------------------
/// This function can be used to force loading of this delay-loaded DLL immediately
/// so its module handle can be referenced.
/// (DLL-Export)

HINSTANCE GetFFLibHandle()
{
	return _AtlBaseModule.GetModuleInstance();
}
