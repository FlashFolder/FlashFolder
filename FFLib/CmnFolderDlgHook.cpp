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
#include "CmnFolderDlgHook.h"

//-----------------------------------------------------------------------------------------

namespace {
	const TCHAR FLASHFOLDER_HOOK_PROPERTY[] = _T("FlashFolderHook.k5evf782uxt56a8zp29");
};

// for easy access from the subclassed WindowProc
CmnFolderDlgHook* g_pHook = NULL;

//-----------------------------------------------------------------------------------------

bool CmnFolderDlgHook::Init( HWND hwndFileDlg, FileDlgHookCallback_base* pCallbacks )
{
	if( m_hwndFileDlg ) return false;  // only init once!

	::OutputDebugString( _T("[fflib] CmnFolderDlgHook::Init()\n") );

	g_pHook = this;

	m_hwndFileDlg = hwndFileDlg;
	m_pCallbacks = pCallbacks;

	m_currentPath[ 0 ] = 0;

	// Subclass the window proc of the file dialog.
	m_oldWndProc = reinterpret_cast<WNDPROC>( 
		::SetWindowLongPtr( hwndFileDlg, GWL_WNDPROC, 
		                    reinterpret_cast<LONG_PTR>( HookWindowProc) ) );
	_ASSERTE( m_oldWndProc );

	// Set a window property for debugging purposes (makes identifying "hooked" windows easier).
	::SetProp( hwndFileDlg, FLASHFOLDER_HOOK_PROPERTY, NULL );

	//--- read settings from INI file ---
	m_minFileDialogWidth = g_profile.GetInt( _T("CommonFolderDlg"), _T("MinWidth"), 650 );
	m_minFileDialogHeight = g_profile.GetInt( _T("CommonFolderDlg"), _T("MinHeight"), 500 );
	m_centerFileDialog = g_profile.GetInt( _T("CommonFolderDlg"), _T("Center"), 1 ) != 0;

	::OutputDebugString( _T("[fflib] CmnFolderDlgHook::Init() return\n") );
	return true;
}

//-----------------------------------------------------------------------------------------

bool CmnFolderDlgHook::SetFolder( LPCTSTR path )
{
	::SendMessage( m_hwndFileDlg, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>( path ) );
	return true;
}

//-----------------------------------------------------------------------------------------

bool CmnFolderDlgHook::GetFolder( LPTSTR folderPath )
{
	StringCchCopy( folderPath, MAX_PATH, m_currentPath );
	return true;
}

//-----------------------------------------------------------------------------------------

bool CmnFolderDlgHook::SetFilter( LPCTSTR filter )
{
	return true;
}

//-----------------------------------------------------------------------------------------------
/// Get the currently selected path from the tree control and store it in m_currentPath

bool CmnFolderDlgHook::UpdateCurrentPath( HWND hwndTree, HTREEITEM hItem )
{
	m_currentPath[ 0 ] = 0;

	// Get absolute directory path by traversing the tree down 
	// until a root directory (drive path) is found.

	TCHAR tmp[ MAX_PATH + 1 ] = _T("");
	TCHAR textBuf[ MAX_PATH + 1 ] = _T("");

	while( hItem )
	{
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask =	TVIF_TEXT;
		item.pszText = textBuf;
		item.cchTextMax = MAX_PATH;

		TreeView_GetItem( hwndTree, &item );

		// check if this is the root m_currentPath
		TCHAR* pPathPart = item.pszText;
		TCHAR rootBuf[ 3 ] = _T("");					
		if( TCHAR* p = _tcschr( item.pszText, ':' ) )
		{
			if( p != item.pszText )
			{
				rootBuf[ 0 ] = *(--p);
				rootBuf[ 1 ] = ':';
				pPathPart = rootBuf;
			}
		}

		StringCbCopy( tmp, sizeof(tmp), m_currentPath );
		StringCchCopy( m_currentPath, MAX_PATH, pPathPart );
		StringCchCat( m_currentPath, MAX_PATH, _T("\\") );
		StringCchCat( m_currentPath, MAX_PATH, tmp );

		if( pPathPart == rootBuf )
			break;

		hItem = TreeView_GetParent( hwndTree, hItem );
	}

	::OutputDebugString( m_currentPath );

	return true;
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK CmnFolderDlgHook::TreeParentWindowProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_NOTIFY:
		{
			NMTREEVIEW* pnm = reinterpret_cast<NMTREEVIEW*>( lParam );
			if( pnm->hdr.code == TVN_SELCHANGED )
			{
				if( pnm->itemNew.hItem )
				{
					if( HWND hwndTree = ::GetDlgItem( hwnd, 0x0064 ) )
					{
						g_pHook->UpdateCurrentPath( hwndTree, pnm->itemNew.hItem );
						g_pHook->m_pCallbacks->OnFolderChange();
					}
				}
			}
		}
		break;
	}

    //call original message handler
    return CallWindowProc( g_pHook->m_oldParentWndProc, hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK CmnFolderDlgHook::HookWindowProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
		case WM_PAINT:
		{
			// hooking WM_PAINT may seem curious here but we need a message that is send
			// AFTER the file dialogs controls are initialised
			if( ! g_pHook->m_initDone )
            {
				g_pHook->m_initDone = true;

				// customize file dialog's initial size + position
				g_pHook->ResizeFileDialog();

				// Hook the parent window of the tree control to get notifications from 
				// the tree control.
				// This control seems to get initialized later, not during WM_INITDIALOG so
				// this check is done again until the window exists.
				if( HWND hwndTreeParent = ::GetDlgItem( hwnd, 0 ) )
				{
					g_pHook->m_oldParentWndProc = reinterpret_cast<WNDPROC>(
						::GetWindowLongPtr( hwndTreeParent, GWLP_WNDPROC ) );
					::SetWindowLongPtrA( hwndTreeParent, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(
						TreeParentWindowProc ) );

					// get initially selected directory
					if( HWND hwndTree = ::GetDlgItem( hwndTreeParent, 0x0064 ) )
					{
						if( HTREEITEM hItem = TreeView_GetSelection( hwndTree ) )
							g_pHook->UpdateCurrentPath( hwndTree, hItem );
					}
				}

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
							g_pHook->m_pCallbacks->OnFolderChange();

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
			// We stored it there in CmnFolderDlgHook::Init().
			::RemoveProp( hwnd, FLASHFOLDER_HOOK_PROPERTY );     
		}
		break;
    }

    //call original message handler
    return CallWindowProc( g_pHook->m_oldWndProc, hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------------------

void CmnFolderDlgHook::ResizeFileDialog()
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
		if (m_centerFileDialog)
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
		if (m_centerFileDialog)
		{
			//--- just center the dialog on the screen :-)
			SetWindowPos(m_hwndFileDlg, NULL, 
				rcCenter.left + (rcCenter.right - rcCenter.left - rc.right) / 2, 
				rcCenter.top  + (rcCenter.bottom - rcCenter.top - rc.bottom) / 2, 
				0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}
