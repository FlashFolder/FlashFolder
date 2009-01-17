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

const UINT WM_FF_INIT_DONE = ::RegisterWindowMessage( FF_GUID );

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::Init( HWND hwndFileDlg, HWND hwndTool )
{
	if( m_hwndFileDlg ) 
		return false;  // only init once
	
	DebugOut( _T("[fflib] CmnFileDlgHook::Init()\n") );
	
	m_hwndFileDlg = hwndFileDlg;
	m_hwndTool = hwndTool; 

	// Subclass the window proc of the file dialog.
	
	::SetWindowSubclass( hwndFileDlg, HookWindowProc, 0, reinterpret_cast<DWORD_PTR>( this ) );

	//--- read settings from INI file ---
	
	m_minFileDialogWidth = MapProfileX( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("MinWidth") ) );
	m_minFileDialogHeight = MapProfileY( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("MinHeight") ) );
	m_centerFileDialog = g_profile.GetInt( _T("CommonFileDlg"), _T("Center") );
	m_folderComboHeight = MapProfileY( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("FolderComboHeight") ) );
	m_filetypesComboHeight = MapProfileY( hwndTool, g_profile.GetInt( _T("CommonFileDlg"), _T("FiletypesComboHeight") ) );
	m_bResizeNonResizableDlgs = g_profile.GetInt( _T("CommonFileDlg"), _T("ResizeNonResizableDialogs") ) != 0;
	m_shellViewMode = (FOLDERVIEWMODE) g_profile.GetInt( L"Main", L"ListViewMode" );
	m_shellViewImageSize = g_profile.GetInt( L"Main", L"ListViewImageSize" );

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

void CmnFileDlgHook::Uninstall()
{
	::RemoveWindowSubclass( m_shellWnd, HookShellWndProc, 0 );
	::RemoveWindowSubclass( m_hwndFileDlg, HookWindowProc, 0 );
	
	Reset();
}

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::SetFolder( LPCTSTR path )
{
	return ShellViewBrowseToFolder( m_hwndFileDlg, path );
}

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::GetFolder( LPTSTR folderPath )
{
	return ShellViewGetCurrentFolder( m_hwndFileDlg, folderPath );
}

//-----------------------------------------------------------------------------------------

bool CmnFileDlgHook::SetFilter( LPCTSTR filter )
{
	return FileDlgSetFilter( m_hwndFileDlg, filter );
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK CmnFileDlgHook::HookWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
	UINT_PTR subclassId, DWORD_PTR refData )
{
	CmnFileDlgHook* pHook = reinterpret_cast<CmnFileDlgHook*>( refData );

	if( uMsg == WM_FF_INIT_DONE )
	{
		// At this time the file dialog has finally settled down.

		DebugOut( L"WM_FF_INIT_DONE" );		
	
		if( wParam == FALSE )
		{
			// File dialog has been initialized the first time.
		
			// Customize file dialog's initial size + position.
			// TODO: this should be done earlier so the dialog does not flicker during position change.
			// (WM_WINDOWPOSCHANGING could be a candidate)
			pHook->ResizeFileDialog();

			// Hook the shell window the first time.
			pHook->InitShellWnd();		
				
			// notify the tool window
			FileDlgHookCallbacks::OnInitDone();
		}
		else
		{
			// Shell window was destroyed and recreated after the user has changed the 
			// current folder.

			pHook->InitShellWnd();
			
			FileDlgHookCallbacks::OnFolderChange();
		}
	}

	switch( uMsg )
	{
        case WM_WINDOWPOSCHANGED:
        {
			LRESULT res = ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
        
			WINDOWPOS* wp = (WINDOWPOS*) lParam;
		
			FileDlgHookCallbacks::OnResize();
				
			if( ( wp->flags & SWP_SHOWWINDOW ) && ! pHook->m_fileDlgShown ) 
			{
				// This is the last message during dialog initialization after which
				// we know the dialog has finally settled down. 
				pHook->m_fileDlgShown = true;
				
				// We post a private message to do any processing after the current
				// operation, where WM_WINDOWPOSCHANGED is part of, has actually finished.
				// See http://blogs.msdn.com/oldnewthing/archive/2006/09/25/770536.aspx
				if( WM_FF_INIT_DONE != 0 )
					::PostMessage( hwnd, WM_FF_INIT_DONE, FALSE, 0 );
			}
			
			return res;
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
					pHook->m_fileDialogCanceled = true;
			}
		}
		break;

		case WM_ACTIVATE:
		{		
			pHook->m_isWindowActive = LOWORD(wParam) != 0;
			FileDlgHookCallbacks::OnActivate( wParam, lParam );
			break;
		}
        case WM_ENABLE:
			FileDlgHookCallbacks::OnEnable( wParam != 0 );
            break;

		case WM_SHOWWINDOW:
			FileDlgHookCallbacks::OnShow( wParam != 0 );
			break;

		case WM_DESTROY:
		{
			// Save view mode permanently.
		
			if( g_profile.GetInt( L"main", L"ListViewMode" ) != FVM_AUTO )
			{
				if( pHook->m_shellViewMode != FVM_AUTO )
					g_profile.SetInt( L"main", L"ListViewMode", (int) pHook->m_shellViewMode );
				g_profile.SetInt( L"main", L"ListViewImageSize", (int) pHook->m_shellViewImageSize );
			}
			FileDlgHookCallbacks::OnDestroy( ! pHook->m_fileDialogCanceled );
		}
		break;

		case WM_NCDESTROY:
		{			
			::RemoveWindowSubclass( hwnd, HookWindowProc, subclassId );
		}
		break;
    }

	// Call the next handler in the window's subclass chain.
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------------------------------------

LRESULT CALLBACK CmnFileDlgHook::HookShellWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR subclassId, DWORD_PTR refData )
{
	CmnFileDlgHook* pHook = reinterpret_cast<CmnFileDlgHook*>( refData );

	if( uMsg == WM_DESTROY )
	{
		// Save view mode in memory (to survive folder changes).
		
		ShellViewGetViewMode( pHook->m_hwndFileDlg, &pHook->m_shellViewMode, &pHook->m_shellViewImageSize );
		DebugOut( L"[fflib] Save viewMode = %d, imageSize = %d\n", pHook->m_shellViewMode, pHook->m_shellViewImageSize );
	}
	if( uMsg == WM_NCDESTROY )
	{
		// We first call DefSubclassProc so that the original window proc can clean up.
		LRESULT res = ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
	
		DebugOut( L"[fflib] Shell view destroyed\n" );
	
		// Make sure we get notified when the shell view is recreated (on folder change).
		PostMessage( pHook->m_hwndFileDlg, WM_FF_INIT_DONE, TRUE, 0 );
		::RemoveWindowSubclass( hwnd, HookShellWndProc, subclassId );

		return res;
	}
	
	// Call the next handler in the window's subclass chain.
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------------------------------------

void CmnFileDlgHook::InitShellWnd()
{
	if( m_shellWnd = FindChildWindowRecursively( m_hwndFileDlg, L"SHELLDLL_DefView" ) )
	{
		// Restore last view mode.		
		DebugOut( L"[fflib] Restore viewMode = %d, imageSize = %d\n", m_shellViewMode, m_shellViewImageSize );
		ShellViewSetViewMode( m_hwndFileDlg, m_shellViewMode, m_shellViewImageSize );

		// hook the shell window to get notified of view mode changes
		DebugOut( L"[fflib] Hooking shell view\n" );
		::SetWindowSubclass( m_shellWnd, HookShellWndProc, 0, reinterpret_cast<DWORD_PTR>( this ) );		
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
	ShellViewGetCurrentFolder(m_hwndFileDlg, folderPath);
	ShellViewBrowseToFolder(m_hwndFileDlg, folderPath);
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

