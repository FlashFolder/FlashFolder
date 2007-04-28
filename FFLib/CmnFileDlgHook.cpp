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

bool CmnFileDlgHook::Init( HWND hwndFileDlg, FileDlgHookCallback_base* pCallbacks )
{
	if( m_hwndFileDlg ) return false;  // only init once!

	::OutputDebugString( _T("[fflib] CmnFileDlgHook::Init()\n") );

	g_pHook = this;

	m_hwndFileDlg = hwndFileDlg;
	m_pCallbacks = pCallbacks;

	// Subclass the window proc of the file dialog.
	m_oldWndProc = reinterpret_cast<WNDPROC>( 
		::SetWindowLongPtr( hwndFileDlg, GWL_WNDPROC, 
		                    reinterpret_cast<LONG_PTR>( HookWindowProc) ) );
	_ASSERTE( m_oldWndProc );

	// Set a window property for debugging purposes (makes identifying "hooked" windows easier).
	::SetProp( hwndFileDlg, FLASHFOLDER_HOOK_PROPERTY, NULL );

	//--- read settings from INI file ---
	m_minFileDialogWidth = g_profile.GetInt( _T("CommonFileDlg"), _T("MinWidth"), 650 );
	m_minFileDialogHeight = g_profile.GetInt( _T("CommonFileDlg"), _T("MinHeight"), 500 );
	m_centerFileDialog = g_profile.GetInt( _T("CommonFileDlg"), _T("Center"), 1 );
	m_folderComboHeight = g_profile.GetInt( _T("CommonFileDlg"), _T("FolderComboHeight"), 400 );
	m_filetypesComboHeight = g_profile.GetInt( _T("CommonFileDlg"), _T("FiletypesComboHeight"), 300 );
	m_bResizeNonResizableDlgs = g_profile.GetInt( _T("CommonFileDlg"), _T("ResizeNonResizableDialogs"), 1 ) != 0;

    //--- check exclusion list for resizing of non-resizable dialogs

    // get application EXE filename
	TCHAR procPath[MAX_PATH+1];
	::GetModuleFileName( NULL, procPath, MAX_PATH );
    TCHAR* pProcExe = _tcsrchr( procPath, _T('\\') );
    if( pProcExe ) pProcExe++;

	for( int i = 0;; ++i )
    {
		TCHAR key[10];
        _stprintf( key, _T("%d"), ++i );
		tstring path = g_profile.GetString( _T("CommonFileDlg.NonResizableExcludes"), key, _T("") );
		if( path.empty() )
			break;

        if( _tcsicmp( pProcExe, path.c_str() ) == 0 )
        {
            m_bResizeNonResizableDlgs = false;
            break;
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

				//customize file dialog's initial size + position
				g_pHook->ResizeFileDialog();

				// notify the tool window
				g_pHook->m_pCallbacks->OnInitDone();
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
				g_pHook->m_pCallbacks->OnFolderChange();
			break;

		case WM_ACTIVATE:
			g_pHook->m_isWindowActive = LOWORD(wParam) != 0;
			break;

        case WM_WINDOWPOSCHANGED:
			g_pHook->m_pCallbacks->OnResize();
			break;

        case WM_ENABLE:
			g_pHook->m_pCallbacks->OnEnable( wParam != 0 );
            break;

		case WM_SHOWWINDOW:
			g_pHook->m_pCallbacks->OnShow( wParam != 0 );
			break;

		case WM_DESTROY:
			g_pHook->m_pCallbacks->OnDestroy( ! g_pHook->m_fileDialogCanceled );
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

void CmnFileDlgHook::ResizeNonResizableFileDialog(
	int x, int y, int newWidth, int newHeight, bool bCenter)
{
	//--- EXPERIMENTAL ! ---
	//    set ResizeNonResizableDialogs=0 in "FlashFolderGlobal.ini" if this
	//    feature makes problems

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
	SetWindowPos(m_hwndFileDlg, NULL, x, y, newWidth, newHeight,
                 (bCenter ? 0 : SWP_NOMOVE) | SWP_NOZORDER | SWP_NOACTIVATE);

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

	RECT rcCenter = { 0 };
	HWND hwndParent = ::GetParent( m_hwndFileDlg );
	if( m_centerFileDialog == 1 )
	{
		if( hwndParent )
			::GetWindowRect( hwndParent, &rcCenter );
		else
			m_centerFileDialog = 2;
	}
	if( m_centerFileDialog == 2 )
	{
		if( ! hwndParent ) 
			hwndParent = m_hwndFileDlg;
		GetMaximizedRect( hwndParent, rcCenter );
	}

	//get original file dialog size
	RECT rc;
	GetClientRect(m_hwndFileDlg, &rc);

	//get new dimensions only if bigger than original size
	int newWidth = rc.right < m_minFileDialogWidth ? m_minFileDialogWidth : rc.right;
	int newHeight = rc.bottom < m_minFileDialogHeight ? m_minFileDialogHeight : rc.bottom;

	//check whether the file dialog is resizable
	LONG style = GetWindowLong(m_hwndFileDlg, GWL_STYLE);
	if (style & WS_SIZEBOX) 
	{
		//--- make the dialog bigger + center it ---
		if( m_centerFileDialog != 0 )
			SetWindowPos(m_hwndFileDlg, NULL, 
				rcCenter.left + (rcCenter.right - rcCenter.left - newWidth) / 2, 
				rcCenter.top  + (rcCenter.bottom - rcCenter.top - newHeight) / 2, 
				newWidth, newHeight, 
				SWP_NOZORDER | SWP_NOACTIVATE);
		else
			SetWindowPos(m_hwndFileDlg, NULL, 
				0, 0, newWidth, newHeight, 
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		if (m_bResizeNonResizableDlgs)
			//if dialog isn't resizable, we must manually resize it
			ResizeNonResizableFileDialog(
				rcCenter.left + (rcCenter.right - rcCenter.left - newWidth) / 2, 
				rcCenter.top  + (rcCenter.bottom - rcCenter.top - newHeight) / 2, 
				newWidth, newHeight, m_centerFileDialog != 0 );
		else
			if (m_centerFileDialog != 0 )
			{
				//--- just center the dialog on the screen
				SetWindowPos(m_hwndFileDlg, NULL, 
					rcCenter.left + (rcCenter.right - rcCenter.left - rc.right) / 2, 
					rcCenter.top  + (rcCenter.bottom - rcCenter.top - rc.bottom) / 2, 
					0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
	}

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

