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

#include "fflib.h"
#include "CmnFileDlgHook.h"

//-----------------------------------------------------------------------------------------

namespace {
	const TCHAR FLASHFOLDER_HOOK_PROPERTY[] = _T("FlashFolderHook.k5evf782uxt56a8zp29");
};

// for easy access from the subclassed WindowProc
CmnFileDlgHook* g_pHook = NULL;

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::Init( HWND hwndFileDlg, HWND hwndTool )
{
	if( m_hwndFileDlg ) return false;  // only init once!
	m_hwndTool = hwndTool; 

	::OutputDebugString( _T("[fflib] CmnFileDlgHook::Init()\n") );

	g_pHook = this;

	m_hwndFileDlg = hwndFileDlg;

	// Subclass the window proc of the file dialog.
	m_oldWndProc = reinterpret_cast<WNDPROC>( 
		::SetWindowLongPtr( hwndFileDlg, GWL_WNDPROC, 
		                    reinterpret_cast<LONG_PTR>( HookWindowProc) ) );
	_ASSERTE( m_oldWndProc );

	// Set a window property for debugging purposes (makes identifying "hooked" windows easier).
	::SetProp( hwndFileDlg, FLASHFOLDER_HOOK_PROPERTY, NULL );

	//--- read settings from INI file ---
	m_minFileDialogWidth = MapProfileX( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("MinWidth") ) );
	m_minFileDialogHeight = MapProfileY( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("MinHeight") ) );
	m_centerFileDialog = g_profile.GetInt( _T("CommonFileDlg"), _T("Center") );
	m_folderComboHeight = MapProfileY( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("FolderComboHeight") ) );
	m_filetypesComboHeight = MapProfileY( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("FiletypesComboHeight") ) );
	m_bResizeNonResizableDlgs = g_profile.GetInt( _T("CommonFileDlg"), _T("ResizeNonResizableDialogs") ) != 0;
	m_listViewMode = g_profile.GetInt( _T("Main"), _T("ListViewMode") );

    //--- check exclusion list for resizing of non-resizable dialogs

    // get application EXE filename
	TCHAR procPath[ MAX_PATH + 1 ] = _T("");
	::GetModuleFileName( NULL, procPath, MAX_PATH );
    if( TCHAR* pProcExe = _tcsrchr( procPath, _T('\\') ) )
	{
		++pProcExe;
		for( int i = 0;; ++i )
		{
			TCHAR key[10];
			StringCbPrintf( key, sizeof(key), _T("%d"), i );
			tstring path = g_profile.GetString( _T("CommonFileDlg.NonResizableExcludes"), key );
			if( path.empty() )
				break;

			if( _tcsicmp( pProcExe, path.c_str() ) == 0 )
			{
				m_bResizeNonResizableDlgs = false;
				break;
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::SetFolder( LPCTSTR path )
{
	return FileDlgBrowseToFolder( m_hwndFileDlg, path );
}

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::GetFolder( LPTSTR folderPath )
{
	return FileDlgGetCurrentFolder( m_hwndFileDlg, folderPath );
}

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::SetFilter( LPCTSTR filter )
{
	return FileDlgSetFilter( m_hwndFileDlg, filter );
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK CmnFileDlgHook::HookWindowProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
		case WM_PAINT:
		{
			//hooking WM_PAINT may seem curious here but we need a message that is send
			// AFTER the file dialogs controls are initialised
			if( ! g_pHook->m_initDone )
            {
				g_pHook->m_initDone = true;

				// For persistent list view mode.
				g_pHook->InitShellWnd();
				
				//customize file dialog's initial size + position
				g_pHook->ResizeFileDialog();
				
				// notify the tool window
				FileDlgHookCallbacks::OnInitDone();
            }
		}
		break;

        case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam); // notification code 
			WORD wID = LOWORD(wParam);         // item, control, or accelerator identifier 
			HWND hwndCtl = (HWND) lParam;      // handle of control 
 
			if( wNotifyCode == BN_CLICKED )
			{
				// To check whether the user has opened a file, we can't check for IDOK
				//  'cause this way we won't get files opened by double-clicks in
				//  the listview. Instead, we check if not IDCANCEL was received.
				//  IDCANCEL will be send for all ways of cancel: button and keyboard.
				if( wID == IDCANCEL )	      
					g_pHook->m_fileDialogCanceled = true;
			}
		}
		break;

		case WM_USER + 300:
			// I don't really know what this message means, but I need it to check
			// whether the user has changed the actual folder path.
            if( g_pHook->m_isWindowActive )
				FileDlgHookCallbacks::OnFolderChange();
			break;

		case WM_ACTIVATE:
			g_pHook->m_isWindowActive = LOWORD(wParam) != 0;
			FileDlgHookCallbacks::OnActivate( wParam, lParam );
			break;

        case WM_WINDOWPOSCHANGED:
			FileDlgHookCallbacks::OnResize();
			break;

        case WM_ENABLE:
			FileDlgHookCallbacks::OnEnable( wParam != 0 );
            break;

		case WM_SHOWWINDOW:
			FileDlgHookCallbacks::OnShow( wParam != 0 );
			break;

		case WM_DESTROY:
		{
			if( g_profile.GetInt( _T("main"), _T("ListViewMode") ) != FLM_VIEW_DEFAULT )
				g_profile.SetInt( _T("main"), _T("ListViewMode"), g_pHook->m_listViewMode );
		
			FileDlgHookCallbacks::OnDestroy( ! g_pHook->m_fileDialogCanceled );
		}
		break;

		case WM_NCDESTROY:
		{			
			// Remove our window property from the file dialog.
			// We stored it there in CmnFileDlgHook::Init().
			::RemoveProp( hwnd, FLASHFOLDER_HOOK_PROPERTY );     
		}
		break;
    }

    //call original message handler
    return CallWindowProc( g_pHook->m_oldWndProc, hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------------------------

LRESULT CALLBACK CmnFileDlgHook::HookShellWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( uMsg == WM_COMMAND )
	{
		switch( wParam )
		{
			case ODM_VIEW_ICONS:  g_pHook->m_listViewMode = FLM_VIEW_ICONS; break;
			case ODM_VIEW_LIST:   g_pHook->m_listViewMode = FLM_VIEW_LIST; break;
			case ODM_VIEW_DETAIL: g_pHook->m_listViewMode = FLM_VIEW_DETAIL; break;
			case ODM_VIEW_THUMBS: g_pHook->m_listViewMode = FLM_VIEW_THUMBS; break;
			case ODM_VIEW_TILES:  g_pHook->m_listViewMode = FLM_VIEW_TILES; break;
		}
	}	
	
    //call original message handler
    return CallWindowProc( g_pHook->m_oldShellWndProc, hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------------------------

void CmnFileDlgHook::InitShellWnd()
{
	if( HWND shellWnd = ::GetDlgItem( m_hwndFileDlg, lst2 ) )
	{
		// restore last view mode
		WPARAM wp = 0;
		switch( m_listViewMode )
		{
			case FLM_VIEW_ICONS:  wp = ODM_VIEW_ICONS; break; 
			case FLM_VIEW_LIST:   wp = ODM_VIEW_LIST; break; 
			case FLM_VIEW_DETAIL: wp = ODM_VIEW_DETAIL; break; 
			case FLM_VIEW_THUMBS: wp = ODM_VIEW_THUMBS; break; 
			case FLM_VIEW_TILES:  wp = ODM_VIEW_TILES; break; 
		}
		if( wp )
			::SendMessage( shellWnd, WM_COMMAND, wp, 0 );

		// hook the shell window to get notified of view mode changes
		g_pHook->m_oldShellWndProc = (WNDPROC)
			::SetWindowLongPtr(	shellWnd, GWLP_WNDPROC, (LONG_PTR) CmnFileDlgHook::HookShellWndProc );
	}
}

//-----------------------------------------------------------------------------------------

// unnnamed namespace avoids linker problems
namespace {

	struct FDMC_STRUCT
	{
		HWND hParent;
		HWND hwndFileDlg;
		RECT rcList;
		int dx, dy;
	};

	BOOL CALLBACK FileDlgMoveChilds(HWND hwnd, LPARAM lParam)
	{
		// Callback function used by ResizeNonResizableFileDialog()

		FDMC_STRUCT *fdm = (FDMC_STRUCT *) lParam;

		// only think about direct childs of the file dialog
		if (GetParent(hwnd) == fdm->hParent)
		{
			// get relative coordinates of file dialog child window
			RECT rc;
			GetWindowRect(hwnd, &rc);
			POINT pt = { rc.left, rc.top };
			ScreenToClient( fdm->hwndFileDlg, &pt );

			if (pt.x > fdm->rcList.right)
				SetWindowPos(hwnd, NULL, pt.x + fdm->dx, pt.y, 0, 0, 
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			else if (pt.y > fdm->rcList.bottom)
				SetWindowPos(hwnd, NULL, pt.x, pt.y + fdm->dy, 0, 0, 
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		return TRUE;
	}

} //unnamed namespace

//-----------------------------------------------------------------------------------------

void CmnFileDlgHook::ResizeNonResizableFileDialog( int x, int y, int newWidth, int newHeight )
{
	//--- EXPERIMENTAL ! ---

	FDMC_STRUCT fdm;
	fdm.hwndFileDlg = m_hwndFileDlg;

	RECT rc;

	// dcx, dcy = difference to original dialog size
	GetWindowRect(m_hwndFileDlg, &rc);
	fdm.dx = newWidth - (rc.right - rc.left);
	fdm.dy = newHeight - (rc.bottom - rc.top);

	// get relative coords of list view container
	HWND wndList = GetDlgItem(m_hwndFileDlg, FILEDLG_LB_SHELLVIEW);
	GetWindowRect(wndList, &fdm.rcList);
	POINT ptList = { fdm.rcList.left, fdm.rcList.top };
	ScreenToClient(m_hwndFileDlg, &ptList);
	fdm.rcList.right = ptList.x + fdm.rcList.right - fdm.rcList.left;
	fdm.rcList.bottom = ptList.y + fdm.rcList.bottom - fdm.rcList.top;
	fdm.rcList.left = ptList.x;  
	fdm.rcList.top = ptList.y; 

	//move the controls which are located to the right and/or bottom of the
	//  shell list view
	fdm.hParent = m_hwndFileDlg;
	EnumChildWindows(m_hwndFileDlg, FileDlgMoveChilds, (LPARAM) &fdm);

	//get the sub-dialog (exists on customized file dialogs only)
	// resize it + move its controls 
	fdm.hParent = FindWindowEx(m_hwndFileDlg, NULL, _T("#32770"), NULL);
	// check whether the sub-dialog exists and has child windows
	if (fdm.hParent != NULL && GetWindow(fdm.hParent, GW_CHILD) != NULL)
	{
		RECT rcDlg;
		GetClientRect(fdm.hParent, &rcDlg);
		SetWindowPos(fdm.hParent, NULL, 0, 0, rcDlg.right + fdm.dx, 
			rcDlg.bottom + fdm.dy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		EnumChildWindows(fdm.hParent, FileDlgMoveChilds, (LPARAM) &fdm);
	}

	// resize the file dialog (must be done after the sub-dialog is resized,
	//   else the file dialogs size will sometimes flip back to its original size)
	SetWindowPos( m_hwndFileDlg, NULL, x, y, newWidth, newHeight, SWP_NOZORDER | SWP_NOACTIVATE );

	//resize the list view container
	SetWindowPos(wndList, NULL, 0, 0, 
	   fdm.rcList.right - fdm.rcList.left + fdm.dx, 
	   fdm.rcList.bottom - fdm.rcList.top + fdm.dy, 
	   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	//resize the list view itself
	HWND hSysList = FindWindowEx(wndList, NULL, _T("SysListView32"), NULL);
	if (hSysList != NULL)
		SetWindowPos(hSysList, NULL, 0, 0, 
		   fdm.rcList.right - fdm.rcList.left + fdm.dx, 
		   fdm.rcList.bottom - fdm.rcList.top + fdm.dy, 
		   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	//force size update of the listview:
	TCHAR folderPath[MAX_PATH +1];
	FileDlgGetCurrentFolder(m_hwndFileDlg, folderPath);
	FileDlgBrowseToFolder(m_hwndFileDlg, folderPath);
}

//-----------------------------------------------------------------------------------------

void CmnFileDlgHook::ResizeFileDialog()
{
	//---- customize file dialog size + position -----
	//when centering the file dialog, take care that it is centered on the correct monitor

	HWND hwndParent = ::GetParent( m_hwndFileDlg );

	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	HMONITOR hMon = ::MonitorFromWindow( hwndParent ? hwndParent : m_hwndFileDlg, MONITOR_DEFAULTTONEAREST );
	::GetMonitorInfo( hMon, &monitorInfo );

	RECT rcCenter = { 0 };
	if( m_centerFileDialog == 1 )
	{
		// center relative to parent window
		if( hwndParent )
			::GetWindowRect( hwndParent, &rcCenter );
		else
			rcCenter = monitorInfo.rcWork;
	}
	else if( m_centerFileDialog == 2 )
	{
		// center relative to monitor work area
		rcCenter = monitorInfo.rcWork;
	}

	RECT rcTool = { 0 };
	::GetWindowRect( m_hwndTool, &rcTool );
	int toolHeight = rcTool.bottom - rcTool.top;

	//get original file dialog size
	RECT rcOld; ::GetWindowRect( m_hwndFileDlg, &rcOld );
	int oldWidth = rcOld.right - rcOld.left;
	int oldHeight = rcOld.bottom - rcOld.top;
	int newX = rcOld.left;
	int newY = rcOld.top;
	int newWidth = oldWidth;
	int newHeight = oldHeight;

	// check whether the file dialog is resizable
	LONG style = GetWindowLong( m_hwndFileDlg, GWL_STYLE );
	if( ( style & WS_SIZEBOX ) || m_bResizeNonResizableDlgs ) 
	{
		// use new size only if bigger than original size
		newWidth  = max( oldWidth,  m_minFileDialogWidth );
		newHeight = max( oldHeight, m_minFileDialogHeight );
	}
	if( m_centerFileDialog != 0 )
	{
		// center the dialog		
		newX = rcCenter.left + ( rcCenter.right - rcCenter.left - newWidth ) / 2;
		newY = rcCenter.top + ( rcCenter.bottom - rcCenter.top - newHeight + toolHeight ) / 2;
	}

	// clip new position against screen borders
	if( newX < monitorInfo.rcWork.left )
		newX = monitorInfo.rcWork.left;
	else if( newX + newWidth > monitorInfo.rcWork.right )
		newX = monitorInfo.rcWork.right - newWidth;
	if( newY < monitorInfo.rcWork.top + toolHeight )
		newY = monitorInfo.rcWork.top + toolHeight;
	else if( newY + newHeight > monitorInfo.rcWork.bottom )
		newY = monitorInfo.rcWork.bottom - newHeight;

	// set the new position / size
	if( ! ( style & WS_SIZEBOX ) && m_bResizeNonResizableDlgs )
		ResizeNonResizableFileDialog( 
			newX, newY, newWidth, newHeight );
	else
		SetWindowPos( m_hwndFileDlg, NULL, 
			newX, newY, newWidth, newHeight, SWP_NOZORDER | SWP_NOACTIVATE );


	//--- enlarge the combo boxes ---
	HWND hFolderCombo = GetDlgItem(m_hwndFileDlg, FILEDLG_CB_FOLDER);
	HWND hFiletypesCombo = GetDlgItem(m_hwndFileDlg, FILEDLG_CB_FILETYPES);
	RECT rcCombo;
	if (hFolderCombo != NULL)
	{
		GetClientRect(hFolderCombo, &rcCombo);
		if (m_folderComboHeight > rcCombo.bottom)
		{
			rcCombo.bottom = m_folderComboHeight;
			SetWindowPos(hFolderCombo, NULL, 0, 0, rcCombo.right, rcCombo.bottom,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	if (hFiletypesCombo != NULL)
	{
		GetClientRect(hFiletypesCombo, &rcCombo);
		if (m_filetypesComboHeight > rcCombo.bottom)
		{
			rcCombo.bottom = m_filetypesComboHeight;
			SetWindowPos(hFiletypesCombo, NULL, 0, 0, rcCombo.right, rcCombo.bottom,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}

