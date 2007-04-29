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

/** \file Main file to compile the "fflib.dll".
 *  This DLL is the work horse of FlashFolder - it contains a hook function
 *  that watches every window activation in the system. If a window is a common
 *  file dialog, FlashFolder subclasses the window procedure.
 */

#include "stdafx.h"

#pragma warning(disable:4786)  // disable STL-template-related warnings
#pragma warning(disable:4995)  // disable strsafe.h warnings

#include "fflib.h"
#include "resource.h"
#include "FileDlg_base.h"
#include "CmnFileDlgHook.h"
#include "MsoFileDlgHook.h"
#include "CmnOpenWithDlgHook.h"
#include "CmnFolderDlgHook.h"

using namespace NT;
using namespace std;
using namespace boost;


//-----------------------------------------------------------------------------------------
// global variables that are shared by all Instances of the DLL  
//-----------------------------------------------------------------------------------------

#pragma data_seg( ".shared" )

HHOOK g_hHook = NULL;						// handle of the hook

#pragma data_seg()
#pragma comment( linker, "/SECTION:.shared,RWS" )


//-----------------------------------------------------------------------------------------
// global variables that are "local" to each instance of the DLL  
//-----------------------------------------------------------------------------------------
HMODULE g_hInstDll = NULL;
HWND g_hFileDialog = NULL;				// handle of file open/save dialog 

scoped_ptr<FileDlgHook_base> g_spFileDlgHook;   // ptr to the hook instance
scoped_ptr<FileDlgHook_base> g_spOpenWithDlgHook;   // ptr to the hook instance

HWND g_hToolWnd = NULL;					// handle of cool external tool window
WNDPROC g_wndProcToolWindowEditPath;

TCHAR g_favIniFilePath[MAX_PATH+1];		// Path to INI-File with favorite folders

Profile g_profile( _T("zett42\\FlashFolder") );   

//--- options, read from global INI file ---
int g_globalHistoryMaxEntries;

//--- constants ---
const HBRUSH hWindowBrush = GetSysColorBrush(COLOR_WINDOW);


//-----------------------------------------------------------------------------------------
// DLL entry point
//-----------------------------------------------------------------------------------------
BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch( ul_reason_for_call )
    {
		case DLL_PROCESS_ATTACH:
		{
			TCHAR exePath[ MAX_PATH + 1 ] = _T("");
			::GetModuleFileName( NULL, exePath, MAX_PATH );

			TCHAR s[512];
			_stprintf( s, _T("[fflib] DLL_PROCESS_ATTACH (pid %08Xh, \"%s\")\n"), 
				::GetCurrentProcessId(), exePath );
			::OutputDebugString( s ); 

			//save the HInstance of the DLL
			g_hInstDll = hModule;
			//optimize DLL loading
			::DisableThreadLibraryCalls( hModule );
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			TCHAR s[256];
			_stprintf( s, _T("[fflib] DLL_PROCESS_DETACH (pid %08Xh)\n"), ::GetCurrentProcessId() );
			::OutputDebugString( s ); 
		}
		break;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------------------

void AdjustToolWndRect(RECT &rc)
{
	//calculates the position + size of the tool window accordingly to the size
	// of the file dialog

	GetWindowRect(g_hFileDialog, &rc);
	rc.bottom = rc.top; 
	rc.top -= 27; 
}

//-----------------------------------------------------------------------------------------

void UpdatePathEdit()
{
    TCHAR folderPath[MAX_PATH + 1] = _T("");
	g_spFileDlgHook->GetFolder( folderPath );
    SetDlgItemText(g_hToolWnd, ID_FF_PATH, folderPath);
}

//-----------------------------------------------------------------------------------------

void AddCurrentFolderToHistory()
{
    TCHAR folderPath[MAX_PATH + 1];
	if( ! g_spFileDlgHook->GetFolder( folderPath ) )
		return;

	HistoryLst globalHistory;
	globalHistory.LoadFromProfile( g_profile, _T("GlobalFolderHistory") );
	globalHistory.SetMaxEntries( g_globalHistoryMaxEntries );
	globalHistory.AddFolder( folderPath );
	globalHistory.SaveToProfile( g_profile, _T("GlobalFolderHistory") );
}

//----------------------------------------------------------------------------------------

void GotoLastDir()
{
	SetForegroundWindow(g_hFileDialog);

	HistoryLst history;

	//load + display the history
	if( ! history.LoadFromProfile( g_profile, _T("GlobalFolderHistory") ) )
		return;

	SetDlgItemText( g_hToolWnd, ID_FF_PATH, history.GetList().front().c_str() );

	g_spFileDlgHook->SetFolder( history.GetList().front().c_str() );
}

//-----------------------------------------------------------------------------------------

HMENU CreateFolderMenu( const vector<tstring> &folderList, HMENU hMenu = NULL )
{
    if( ! hMenu ) hMenu = CreatePopupMenu();
	if( hMenu == NULL ) 
		return NULL;

	for( size_t i = 0; i < folderList.size(); i++ )
	{
		if( folderList[ i ] == _T("-") )
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		else
	        ::AppendMenu( hMenu, MF_STRING, i + 1, folderList[i].c_str() );
	}
	return hMenu;
}

//-----------------------------------------------------------------------------------------

int DisplayFolderMenu( HMENU hMenu, int buttonID )
{
	HWND hTb = GetDlgItem(g_hToolWnd, ID_FF_TOOLBAR);
	RECT rc;
	SendMessage(hTb, TB_GETRECT, buttonID, (LPARAM) &rc);
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc ) ); 
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc.right ) ); 

	int id = TrackPopupMenu(hMenu, 
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
		rc.left, rc.bottom, 0, g_hToolWnd, NULL);
	if( id > 0 )
	{
        // get directory path from menu item text
        TCHAR path[MAX_PATH+1] = _T("");
        MENUITEMINFO mi = { sizeof(mi) };
        mi.fMask = MIIM_STRING;
        mi.dwTypeData = path;
        mi.cch = MAX_PATH;
        ::GetMenuItemInfo( hMenu, id, FALSE, &mi );

		// strip additional text to get the path only
		LPCTSTR pPath = mi.dwTypeData;
		if( pPath[ 0 ] == '[' )
			pPath = _tcsstr( pPath, _T("] ") ) + 2;

		if( IsFilePath( pPath ) )
        {
		    SetDlgItemText( g_hToolWnd, ID_FF_PATH, pPath );
		
			SetForegroundWindow(g_hFileDialog);
			g_spFileDlgHook->SetFolder( pPath );
        }
	}

	return id;
}

//-----------------------------------------------------------------------------------------

void DisplayMenu_GlobalHist()
{
	HistoryLst history;

	//load + display the history
	if (history.LoadFromProfile( g_profile, _T("GlobalFolderHistory") ) )
	{
		HMENU hMenu = CreateFolderMenu(history.GetList());
		DisplayFolderMenu( hMenu, ID_FF_GLOBALHIST );
		DestroyMenu(hMenu);
	}
		
	SetForegroundWindow(g_hFileDialog);
}

//-----------------------------------------------------------------------------------------

// helper for DisplayMenu_OpenDirs()
struct CNoCaseCompare
{
    const bool operator()( const tstring& s1, const tstring& s2 ) const 
        { return _tcsicmp( s1.c_str(), s2.c_str() ) < 0; }
};

//-------------------------------------------------------------------------------------------------
/// show a menu with the folders of the currently open files

void DisplayMenu_OpenDirs()
{
	HMENU hMenu = ::CreatePopupMenu();

    //--- get current Total Commander folders and add them to the menu

	CTotalCmdUtils tcmdUtils( CTotalCmdUtils::FindTopTCmdWnd() );
    if( tcmdUtils.GetTCmdWnd() )
    {
        TCHAR leftDir[MAX_PATH+1];
        TCHAR rightDir[MAX_PATH+1];
        if( tcmdUtils.GetDirs( leftDir, MAX_PATH, rightDir, MAX_PATH ) )
        {
			TCHAR s1[ MAX_PATH + 20 ] = _T("[TC] ");
			StringCbCat( s1, sizeof(s1), leftDir );
			TCHAR s2[ MAX_PATH + 20 ] = _T("[TC] ");
			StringCbCat( s2, sizeof(s2), rightDir );
            ::AppendMenu( hMenu, MF_STRING, 2000, s1 );
            ::AppendMenu( hMenu, MF_STRING, 2001, s2 );
            ::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
        }
    }

    //--- allocate buffers for return data of NT kernel APIs

    CTypedBuf<SYSTEM_HANDLE_INFORMATION> handleBuf( 0x40000 );
    CTypedBuf<OBJECT_NAME_INFORMATION> nameBuf( 4096 );

    //--- get the global handle list 

    NTSTATUS res;
    while( ( res = D_NtQuerySystemInformation( SystemHandleInformation, handleBuf.Get(), 
                     handleBuf.GetSize(), NULL ) ) == STATUS_INFO_LENGTH_MISMATCH )
        handleBuf.Resize( handleBuf.GetSize() * 2 );
    if( res != STATUS_SUCCESS )
        return;

    //--- get application EXE dir - will be ignored by the handle-enumeration and added
    //    later on top of the menu
    TCHAR exeDir[MAX_PATH+1];
    ::GetModuleFileName( NULL, exeDir, MAX_PATH );
    LPTSTR pExeDirBs = _tcsrchr( exeDir, _T('\\') );
    if( pExeDirBs ) *pExeDirBs = 0;

    //--- for each file handle of the current process: get the dir path and add it to the set
    
    set<tstring, CNoCaseCompare> dirset;
    DWORD processId = ::GetCurrentProcessId();  
    BYTE objType_file = CNtObjTypeMap::GetTypeId( OT_File );
    SYSTEM_HANDLE *pHandleInfo = handleBuf.Get()->Handles;

    for( size_t n = 0; n < handleBuf.Get()->NumberOfHandles; n++, pHandleInfo++ )
    {
        if( pHandleInfo->ProcessId != processId || pHandleInfo->ObjectType != objType_file )
            continue;            

        //--- get file path (in NT namespace format) ---
        WCHAR filepath[MAX_PATH];
        if( D_NtQueryObject( (HANDLE) pHandleInfo->Value, ObjectNameInformation, 
                nameBuf.Get(), nameBuf.GetSize(), NULL ) != STATUS_SUCCESS )
            continue;
        if( ! nameBuf.Get()->ObjectName.Buffer )
            continue;

        //--- get file attributes ---
        bool bIsDir = false;
        OBJECT_ATTRIBUTES objAttr = { sizeof(objAttr), NULL, &nameBuf.Get()->ObjectName };
        FILE_BASIC_INFORMATION fileInfo; 
        if( D_NtQueryAttributesFile( &objAttr, &fileInfo ) == STATUS_SUCCESS )
        {
            // ignore temporary files
            if( (fileInfo.FileAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0 )
                continue;
            bIsDir = (fileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        }

        //--- convert filepath to user-level format ---
        if( ! MapNtFilePathToUserPath( filepath, MAX_PATH, nameBuf.Get()->ObjectName.Buffer ) )
            continue;

        //--- if its not a directory, extract dir ---
        if( ! bIsDir )
        {
            WCHAR* p = wcsrchr( filepath, _T('\\') );
            if( p ) *p = 0;
        }

        // add the directory path - the set eleminates duplicates
        if( _tcsicmp( filepath, exeDir ) != 0 )
            dirset.insert( filepath );
    } //for       

    vector<tstring> dirlist( dirset.size() + 1 );

    //--- add application dir to the folder list
    dirlist[0] = exeDir;

    //--- copy the in-use-folders to the list
    vector<tstring>::iterator it = dirlist.begin();  it++;
    copy( dirset.begin(), dirset.end(), it );
    
    //--- append menu items ---
	CreateFolderMenu( dirlist, hMenu );
    
	DisplayFolderMenu( hMenu, ID_FF_OPENDIRS );
    
	DestroyMenu( hMenu );
		
	SetForegroundWindow(g_hFileDialog);
}

//-----------------------------------------------------------------------------------------
// display favorite folders menu

void DisplayMenu_Favorites()
{

	// *** TODO: horiz dividers + sub menus 

    HMENU hMenu = ::CreatePopupMenu();

	FavoritesList dirList;
	GetDirFavorites( &dirList );
	if( dirList.size() > 0 )
	{
		vector<tstring> simpleDirList;
		for( int i = 0; i != dirList.size(); ++i )
		{
			if( IsFilePath( dirList[ i ].path.c_str() ) )
				simpleDirList.push_back( dirList[ i ].path );
			else if( dirList[ i ].title == _T("-") )
				simpleDirList.push_back( _T("-") );
		}
		CreateFolderMenu( simpleDirList, hMenu );
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	}

	AppendMenu(hMenu, MF_STRING, 1000, _T("&Add current folder") );
	AppendMenu(hMenu, MF_STRING, 1001, _T("&Configure...") );
	
	int id = DisplayFolderMenu( hMenu, ID_FF_FAVORITES );
	
	if( id == 1000 )
	{
		//--- add current folder to favorite folders 

		TCHAR path[ MAX_PATH + 1 ];
		if( g_spFileDlgHook->GetFolder( path ) )
		{
			//check if current folder already exists in the list
			bool bExists = false;
			for( int i = 0; i != dirList.size() && ! bExists; ++i )
				bExists = _tcsicmp( dirList[ i ].path.c_str(), path ) == 0;	

			if( ! bExists )
			{
				FavoritesItem item;
				item.title = path;
				item.path = path;
				dirList.push_back( item );
				SetDirFavorites( dirList );
			}
		}
	}
	else if( id == 1001 )
	{
		TCHAR path[ MAX_PATH + 1 ] = _T("");
		GetAppDir( g_hInstDll, path );
		StringCbCat( path, sizeof(path), _T("FFConfig.exe") );

		TCHAR params[ 256 ] = _T("");
		_sntprintf( params, sizeof(params) / sizeof(TCHAR) - 1, _T("%d --fav"), g_hFileDialog ); 

		::ShellExecute( g_hToolWnd, _T("open"), path, params, NULL, SW_SHOW );
	}

	DestroyMenu(hMenu);

	SetForegroundWindow( g_hFileDialog );
}

//-----------------------------------------------------------------------------------------------

void DisplayMenu_Config()
{
	const int buttonId = ID_FF_CONFIG;

	// get menu position
	HWND hTb = ::GetDlgItem( g_hToolWnd, ID_FF_TOOLBAR );
	RECT rc;
	::SendMessage( hTb, TB_GETRECT, buttonId, (LPARAM) &rc );
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc ) ); 
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc.right ) ); 

	HMENU hMenu = ::CreatePopupMenu();
	::AppendMenu( hMenu, MF_STRING, 1, _T("Options...") );
	::AppendMenu( hMenu, MF_STRING, 2, _T("About FlashFolder...") );

	int id = TrackPopupMenu(hMenu, 
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
		rc.left, rc.bottom, 0, g_hToolWnd, NULL);

	TCHAR path[ MAX_PATH + 1 ] = _T("");
	GetAppDir( g_hInstDll, path );
	StringCbCat( path, sizeof(path), _T("FFConfig.exe") );
	
	TCHAR params[ 256 ] = _T("");
	_sntprintf( params, sizeof(params) / sizeof(TCHAR) - 1, _T("%d"), g_hFileDialog ); 
	
	if( id == 1 )
	{
		::ShellExecute( g_hFileDialog, _T("open"), path, params, NULL, SW_SHOW );
	}
	else if( id == 2 )
	{
		StringCbCat( params, sizeof(params), _T(" --about") );
		::ShellExecute( g_hFileDialog, _T("open"), path, params, NULL, SW_SHOW );
	}

	::SetForegroundWindow( g_hFileDialog );
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK ToolWindowEditPathProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//window proc for the path edit ctrl of the tool window

	switch (uMsg)
	{
		case WM_KEYDOWN:
			if (wParam == VK_RETURN)
			{
				TCHAR path[MAX_PATH + 1] = _T("");
				GetWindowText(hwnd, path, sizeof(path)-1);

				SetForegroundWindow(g_hFileDialog);
				g_spFileDlgHook->SetFolder( path );
			}
			else if (wParam == VK_ESCAPE) 
				PostMessage(g_hFileDialog, WM_CLOSE, 0, 0);
			break;

		case WM_LBUTTONDBLCLK:
			//select all on dbl-click / select & copy on ctrl + dbl-click
			if (wParam == MK_LBUTTON)
			{
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			}
			else if (wParam & MK_CONTROL)
			{
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				SendMessage(hwnd, WM_COPY, 0, 0);		
				return 0;
			}
			break;
	}

	//call original message handler
    return CallWindowProc(g_wndProcToolWindowEditPath, hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK ToolWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam); // notification code 
			WORD wID = LOWORD(wParam);         // item, control, or accelerator identifier 
			HWND hwndCtl = (HWND) lParam;      // handle of control 
 
			if (wNotifyCode == BN_CLICKED)
				switch (wID)
				{
					case ID_FF_LASTDIR:
						GotoLastDir();
						break;
					case ID_FF_SHOWALL:
						g_spFileDlgHook->SetFilter( _T("*.*") );
						SetForegroundWindow(g_hFileDialog);
						break;
				}
		}
		break;

		case WM_NOTIFY:
		{
			NMHDR* pnm = reinterpret_cast<NMHDR*>( lParam );
			if( pnm->code == TBN_DROPDOWN )
			{
				// drop-down button pressed

				NMTOOLBAR* pnmt = reinterpret_cast<NMTOOLBAR*>( pnm );
				switch( pnmt->iItem )
				{
					case ID_FF_GLOBALHIST: 
						DisplayMenu_GlobalHist();
						break;
                    case ID_FF_OPENDIRS:
                        DisplayMenu_OpenDirs();
                        break;
					case ID_FF_FAVORITES:
						DisplayMenu_Favorites();
						break;
					case ID_FF_CONFIG:
						DisplayMenu_Config();
						break;
				}
				return TBDDRET_DEFAULT;
			}
			else if( pnm->code == TTN_NEEDTEXT )
			{
				// tooltip text requested
			
				NMTTDISPINFO* pTTT = reinterpret_cast<NMTTDISPINFO*>( pnm );

				pTTT->hinst = (HINSTANCE) g_hInstDll;
			
				if( pTTT->hdr.idFrom == ID_FF_LASTDIR )
				{
					// use last entry of global history as tooltip
					HistoryLst history;
					if( history.LoadFromProfile( g_profile, _T("GlobalFolderHistory") ) )
						StringCchCopyN( pTTT->szText, sizeof(pTTT->szText) / sizeof(TCHAR),
							history.GetList().front().c_str(), history.GetList().front().size() );
				}
				else
				{
					pTTT->lpszText = MAKEINTRESOURCE(pTTT->hdr.idFrom);
						//return ID of appropriate string resource (= ID of control)
				}
			}
		}
		break;

		case WM_WINDOWPOSCHANGED:
		{
			//resize path control
			WINDOWPOS *wp = (WINDOWPOS *) lParam;
			RECT rc;
			GetClientRect(GetDlgItem(hwnd, ID_FF_TOOLBAR), &rc);
			rc.right += 7;
			HWND hPath = GetDlgItem(hwnd, ID_FF_PATH);
			MoveWindow(hPath, rc.right, 2, wp->cx - rc.right - 8, 17, TRUE);
		}
		break;


		case WM_CTLCOLOREDIT:
		{
			//called when the path control is redrawn by windows
			//make it a different text color if the folder path is invalid

			static bool bLastValidDir = true;

			HDC hdcEdit = (HDC) wParam;   // handle to display context 
			HWND hwndEdit = (HWND) lParam; // handle to static control 
		
			TCHAR path[MAX_PATH+1];
			GetWindowText(hwndEdit, path, MAX_PATH);
			
			bool bValidDir = DirectoryExists(path);
			if (bValidDir)
				SetTextColor(hdcEdit, GetSysColor(COLOR_WINDOWTEXT));	
			else
				SetTextColor(hdcEdit, GetSysColor(COLOR_GRAYTEXT));
			if (bValidDir != bLastValidDir)
			{
				bLastValidDir = bValidDir;
				InvalidateRect(hwndEdit, NULL, FALSE);
			}

			SetBkColor(hdcEdit, GetSysColor(COLOR_WINDOW));
			return (LPARAM) hWindowBrush;
		}
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam); 
}

//-----------------------------------------------------------------------------------------

BOOL CALLBACK ToolWndSetFont(HWND hwnd, LPARAM lParam)
{
	SendMessage(hwnd, WM_SETFONT, lParam, 0);
	return TRUE;
}

//-----------------------------------------------------------------------------------------

void CreateToolWindow( bool isFileDialog )
{
	//--- create the external tool window ---

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = &ToolWindowProc;
	wc.hInstance = (HINSTANCE) g_hInstDll;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1); 
	wc.lpszClassName = _T("FlashFolderToolWnd_73614384");
	RegisterClassEx( &wc );

	RECT rc;
	AdjustToolWndRect(rc);
	g_hToolWnd = ::CreateWindow( wc.lpszClassName, NULL, WS_POPUP | WS_CLIPCHILDREN | WS_DLGFRAME, 
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
		g_hFileDialog, (HMENU) 0, (HINSTANCE) g_hInstDll, NULL); 
	if( g_hToolWnd == NULL )
		return;

	//--- create the toolbar ---

	vector<TBBUTTON> tbButtons;
	{
		TBBUTTON btn = { 5, ID_FF_LASTDIR,                 0, BTNS_BUTTON, 0, 0, 0, 0 };
		tbButtons.push_back( btn );
	}
	{
		TBBUTTON btn = { 0, ID_FF_GLOBALHIST,              0, BTNS_BUTTON | BTNS_WHOLEDROPDOWN, 0, 0, 0, 0 };
		tbButtons.push_back( btn );
	}
	{
		TBBUTTON btn = { 2, ID_FF_OPENDIRS,  TBSTATE_ENABLED, BTNS_BUTTON | BTNS_WHOLEDROPDOWN, 0, 0, 0, 0 };
		tbButtons.push_back( btn );
	}
	{
		TBBUTTON btn = { 3, ID_FF_FAVORITES, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_WHOLEDROPDOWN, 0, 0, 0, 0 };
		tbButtons.push_back( btn );
	}	  
	if( isFileDialog )
	{
		TBBUTTON btn = { 4, ID_FF_SHOWALL,   TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0, 0 };
		tbButtons.push_back( btn );
	}
	{
		TBBUTTON btn = { 6, ID_FF_CONFIG,    TBSTATE_ENABLED, BTNS_BUTTON | BTNS_WHOLEDROPDOWN, 0, 0, 0, 0 };
		tbButtons.push_back( btn );
	}
	
    HWND hTb = ::CreateToolbarEx( g_hToolWnd, WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | 
		CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS, 
		ID_FF_TOOLBAR, tbButtons.size(), (HINSTANCE) g_hInstDll, ID_FF_TOOLBAR, &tbButtons[ 0 ], 
		tbButtons.size(), 16,15, 16,15, sizeof(TBBUTTON) );
	::SendMessage( hTb, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );
	::SendMessage( hTb, TB_AUTOSIZE, 0, 0 );

	// calculate width of the toolbar from position of last button (TB_GETMAXSIZE has a bug under Win2k!)
	RECT tbRC = { 0 };
	::SendMessage( hTb, TB_GETRECT, tbButtons.back().idCommand, reinterpret_cast<LPARAM>( &tbRC ) );
	SIZE tbSize = { tbRC.right, tbRC.bottom };

	SetWindowPos( hTb, NULL, 2, 0, tbSize.cx, tbSize.cy, SWP_NOZORDER | SWP_NOACTIVATE );

	//--- create + sub-class the edit control 
	tbSize.cx += 7;
	HWND hEdit = CreateWindowEx(WS_EX_STATICEDGE, _T("Edit"), NULL, WS_VISIBLE | WS_CHILD, 
		tbSize.cx, 2, rc.right - rc.left - tbSize.cx - 6, 17, 
		g_hToolWnd, (HMENU) ID_FF_PATH, (HINSTANCE) g_hInstDll, NULL);
	//sub-class the edit control to handle key-stroke messages
	g_wndProcToolWindowEditPath = (WNDPROC)  
		SetWindowLong(hEdit, GWL_WNDPROC, (LONG) &ToolWindowEditPathProc);

    //--- set default font for all controls
	HFONT hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	EnumChildWindows(g_hToolWnd, ToolWndSetFont, (LPARAM) hFont);

	//--- read options from global configuration
	g_globalHistoryMaxEntries = g_profile.GetInt( _T("main"), _T("MaxGlobalHistoryEntries"), 20 );
 
	//--- enable toolbar buttons / leave disabled ---
	
	if( ! g_profile.GetString( _T("GlobalFolderHistory"), _T("0") ).empty() )
	{
		SendMessage( hTb, TB_SETSTATE, ID_FF_GLOBALHIST, MAKELONG( TBSTATE_ENABLED, 0 ) );
		SendMessage( hTb, TB_SETSTATE, ID_FF_LASTDIR, MAKELONG(TBSTATE_ENABLED, 0 ) );
	}
}

//-----------------------------------------------------------------------------------------

// callbacks that will be called by the file dialog hook

class FileDlgHookCallback : public FileDlgHookCallback_base
{
public:
	virtual void OnInitDone()
	{
        //--- initial show + update of the tool window ---
		UpdatePathEdit();
		::ShowWindow( g_hToolWnd, SW_SHOWNA );
	}
	
	virtual void OnFolderChange()
	{
		UpdatePathEdit();
	}

    virtual void OnResize()
	{
		//reposition the tool window to "dock" it onto the file dialog
		RECT rc;
		AdjustToolWndRect( rc );
		::MoveWindow( g_hToolWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE );
	}

	virtual void OnEnable( bool bEnable )
	{
		::EnableWindow( g_hToolWnd, bEnable );
	}

	virtual void OnShow( bool bShow )
	{
		::ShowWindow( g_hToolWnd, bShow ? SW_SHOW : SW_HIDE );
	}

	virtual void OnDestroy( bool isOkBtnPressed )
	{
        //--- add folder to history if file dialog was closed with OK
		if( isOkBtnPressed )
			AddCurrentFolderToHistory();			

        //--- destroy tool window + class ---
		::DestroyWindow( g_hToolWnd );
		::UnregisterClass( _T("FlashFolderToolWnd_73614384"), (HINSTANCE) g_hInstDll );
		g_hToolWnd = NULL;
		g_hFileDialog = NULL;
	}
}
g_fileDlgHookCallback;

//-----------------------------------------------------------------------------------------
/**
 * Set default program settings when the first file dialog is opened by the current user.
**/
void SetProfileDefaults()
{
	OutputDebugString( _T("[fflib] setting default profile data\n") );

	tstring tcIniPath;
	bool isTcInstalled = GetTotalCmdLocation( NULL, &tcIniPath );

	g_profile.SetInt( _T("main"), _T("MaxGlobalHistoryEntries"), 15, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("main"), _T("UseTcFavorites"), isTcInstalled ? 1 : 0, Profile::DONT_OVERWRITE );
	
	//--- common file dialog

	g_profile.SetInt( _T("CommonFileDlg"), _T("EnableHook"), 1, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFileDlg"), _T("MinWidth"), 650, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFileDlg"), _T("MinHeight"), 500, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFileDlg"), _T("Center"), 1, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFileDlg"), _T("FolderComboHeight"), 650, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFileDlg"), _T("FiletypesComboHeight"), 400, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFileDlg"), _T("ResizeNonResizableDialogs"), 1, Profile::DONT_OVERWRITE );
	if( ! g_profile.SectionExists( _T("CommonFileDlg.NonResizableExcludes") ) )
		g_profile.SetString( _T("CommonFileDlg.NonResizableExcludes"), _T("0"), _T("i_view32.exe") );

	//--- common folder dialog

	g_profile.SetInt( _T("CommonFolderDlg"), _T("EnableHook"), 1, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFolderDlg"), _T("MinWidth"), 400, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFolderDlg"), _T("MinHeight"), 500, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonFolderDlg"), _T("Center"), 1, Profile::DONT_OVERWRITE );

	//--- MSO file dialog

	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("EnableHook"), 0, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("MinWidth"), 650, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("MinHeight"), 500, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("Center"), 1, Profile::DONT_OVERWRITE );

	//--- common "Open With" dialog

	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("EnableHook"), 0, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("MinWidth"), 400, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("MinHeight"), 500, Profile::DONT_OVERWRITE );
	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("Center"), 1, Profile::DONT_OVERWRITE );
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// Check whether a file dialog is already hooked.
	// For now, we can only handle one running file dialog per application, but
	// this should be enough in nearly all cases.
    if( nCode == HCBT_ACTIVATE && g_hFileDialog == NULL )
	{ 
		// HCBT_ACTIVATE gets called in the context of the process belonging to the dialog

		HWND hwnd = reinterpret_cast<HWND>( wParam );

		FileDlgType fileDlgType = GetFileDlgType( hwnd );
		if( fileDlgType != FDT_NONE )
		{
			SetProfileDefaults();

			bool isFileDialog = false;

			//--- check if hook for this type of dialog is enabled
			bool isEnabled = true;
			if( fileDlgType == FDT_COMMON )
			{
				isFileDialog = true;
				if( g_profile.GetInt( _T("CommonFileDlg"), _T("EnableHook") ) == 0 )
					isEnabled = false;
			}
			else if( fileDlgType == FDT_MSOFFICE ) 
			{
				isFileDialog = true;
				if( g_profile.GetInt( _T("MSOfficeFileDlg"), _T("EnableHook") ) == 0 )
					isEnabled = false;
			}
			else if( fileDlgType == FDT_COMMON_OPENWITH ) 
			{
				if( g_profile.GetInt( _T("CommonOpenWithDlg"), _T("EnableHook") ) == 0 )
					isEnabled = false;
			}
			else if( fileDlgType == FDT_COMMON_FOLDER ) 
			{
				if( g_profile.GetInt( _T("CommonFolderDlg"), _T("EnableHook") ) == 0 )
					isEnabled = false;
			}
			if( ! isEnabled )
				return CallNextHookEx( g_hHook, nCode, wParam, lParam );


			//--- initialise hook for this dialog

			g_hFileDialog = hwnd;

			CreateToolWindow( isFileDialog );

			// create an instance of a file dialog hook class depending on the
			// type of file dialog
			if( fileDlgType == FDT_COMMON )
			{
				g_spFileDlgHook.reset( new CmnFileDlgHook );
				g_spFileDlgHook->Init( hwnd, g_hToolWnd, &g_fileDlgHookCallback );
			}
			else if( fileDlgType == FDT_MSOFFICE )
			{
				g_spFileDlgHook.reset( new MsoFileDlgHook );
				g_spFileDlgHook->Init( hwnd, g_hToolWnd, &g_fileDlgHookCallback );
			}
			else if( fileDlgType == FDT_COMMON_OPENWITH )
			{
				// init the "Open With" dialog hook
				g_spOpenWithDlgHook.reset( new CmnOpenWithDlgHook );
				g_spOpenWithDlgHook->Init( hwnd, g_hToolWnd, NULL );
			}
			else if( fileDlgType == FDT_COMMON_FOLDER )
			{
				// init the "Open With" dialog hook
				g_spFileDlgHook.reset( new CmnFolderDlgHook );
				g_spFileDlgHook->Init( hwnd, g_hToolWnd, &g_fileDlgHookCallback );
			}

			// Make sure the DLL gets not unloaded as long as the window is subclassed.
			// This can occur if the FlashFolder service stops while a file dialog is open.
			// Intentionally there is no balancing FreeLibrary() call since I couldn't find
			// a good place to call it. No problem since Windows Installer can "magically" 
			// delete the DLL file during uninstall, even if it is still loaded into some processes.
			TCHAR dllPath[ MAX_PATH + 1 ];
			::GetModuleFileName( (HMODULE) g_hInstDll, dllPath, MAX_PATH ); 
			::LoadLibrary( dllPath );  

			return 0;                
		}
	}

	// be a good Windoze citizen
   	return CallNextHookEx( g_hHook, nCode, wParam, lParam );
}


//=========================================================================================
//  Installation / Deinstallation Functions (DLL-Export)
//=========================================================================================

DLLFUNC bool IsHookInstalled()
{  
	return (g_hHook != NULL);
}

//-----------------------------------------------------------------------------------------

DLLFUNC bool InstallHook()
{  
	// TODO: make thread safe 
    if( g_hHook == NULL )
    {
		::OutputDebugString( _T("[fflib] creating hook...\n") );

        // Install the hook
		g_hHook = ::SetWindowsHookEx( WH_CBT, HookProc, (HINSTANCE) g_hInstDll, 0 );

		TCHAR s[256]; _stprintf( s, _T("[fflib] g_hHook = %08Xh\n"), g_hHook );
		::OutputDebugString( s );

		return g_hHook != NULL;
    }
	return false;
}

//-----------------------------------------------------------------------------------------

DLLFUNC bool UninstallHook()
{  
	// TODO: make thread safe 
    if( g_hHook != NULL )
    {
		TCHAR s[256]; _stprintf( s, _T("[fflib] removing hook %08Xh...\n"), g_hHook );
		::OutputDebugString( s );

		if( ::UnhookWindowsHookEx( g_hHook ) )
		{
			g_hHook = NULL;
			return true;
		}
    }
	return false;
}

