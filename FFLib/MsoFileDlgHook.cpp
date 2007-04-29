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
#include "msofiledlghook.h"

using namespace std;

//-----------------------------------------------------------------------------------------

namespace {
	const TCHAR FLASHFOLDER_HOOK_PROPERTY[] = _T("FlashFolderHook.k5evf782uxt56a8zp29");
};

// for easy access from the subclassed WindowProc
MsoFileDlgHook* g_pHook = NULL;

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::Init( HWND hwndFileDlg, HWND hwndTool, FileDlgHookCallback_base* pCallbacks )
{
	if( m_hwndFileDlg ) return false;  // only init once!
	m_hwndTool = hwndTool; 

	::OutputDebugString( _T("[fflib] MsoFileDlgHook::Init()\n") );

	g_pHook = this;

	m_hwndFileDlg = hwndFileDlg;
	m_pCallbacks = pCallbacks;

	// Subclass the window proc of the file dialog.
	m_oldWndProc = reinterpret_cast<WNDPROC>( 
		::SetWindowLongPtr( hwndFileDlg, GWL_WNDPROC, 
		                    reinterpret_cast<LONG_PTR>( HookWindowProc) ) );
	_ASSERTE( m_oldWndProc );

	// Associate the this-pointer with the hooked window so we can access
	// "this" instance in the static HookWindowProc without the need of global variables.
	::SetProp( hwndFileDlg, FLASHFOLDER_HOOK_PROPERTY, 
	           reinterpret_cast<HANDLE>( this ) );

	//--- read settings from INI file ---
	m_minFileDialogWidth = g_profile.GetInt( _T("MSOfficeFileDlg"), _T("MinWidth"), 650 );
	m_minFileDialogHeight = g_profile.GetInt( _T("MSOfficeFileDlg"), _T("MinHeight"), 500 );
	m_centerFileDialog = g_profile.GetInt( _T("MSOfficeFileDlg"), _T("Center"), 1 );

	return true;
}

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::SetFolder( LPCTSTR path )
{
	// MSO seems to partly use its own windowless controls. E.g. calling 
	// SetFocus() seems to put the cursor into the edit control but the internal focus 
	// state of the windowless controls isn't changed.
	// The only reliable method to automate the MSO dialog seems to simulate mouse- and
	// keyboard input.

	TCHAR oldEditTxt[1024] = _T(""); 
	
	// append backslash to path if necessary
	TCHAR pathBuf[MAX_PATH + 2 ];
	_tcsncpy( pathBuf, path, MAX_PATH );
	pathBuf[MAX_PATH] = 0;
	size_t len = _tcslen( pathBuf );
	if( len > 0 )
	{
		TCHAR* p = _tcsninc( pathBuf, len - 1 );
        if( *p != _T('\\') ) _tcscat( pathBuf, _T("\\") );
	}

	HWND hEditFileName = ::GetDlgItem( m_hwndFileDlg, MSO2002_FILEDLG_ED_FILENAME );
	if( ! hEditFileName )
		hEditFileName = ::GetDlgItem( m_hwndFileDlg, MSO2000_FILEDLG_ED_FILENAME );

	if( hEditFileName )
	{
		// save current state
		::GetWindowText( hEditFileName, oldEditTxt, (sizeof(oldEditTxt) - 1) / sizeof(TCHAR) );
		POINT oldCursorPos;
		::GetCursorPos( &oldCursorPos );

		// clear the edit control for new keyboard input
		::SetWindowText( hEditFileName, _T("") );

		//--- prepare the mouse / keyboard input stream ---

		// simulate mouse click in the filename edit control to set the focus to it

		RECT rcEdit;
		::GetWindowRect( hEditFileName, &rcEdit );
		int mx = rcEdit.left + ( rcEdit.right - rcEdit.left ) / 2;
		int my = rcEdit.top + ( rcEdit.bottom - rcEdit.top ) / 2;

		vector<INPUT> vinp;
		INPUT inp;

		memset( &inp, 0, sizeof(inp) );
		inp.type = INPUT_MOUSE;
		inp.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
		inp.mi.dx = mx * 65535 / ::GetSystemMetrics( SM_CXSCREEN );
		inp.mi.dy = my * 65535 / ::GetSystemMetrics( SM_CYSCREEN );
		vinp.push_back( inp );

		memset( &inp, 0, sizeof(inp) );
		inp.type = INPUT_MOUSE;
		inp.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		vinp.push_back( inp );
		inp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		vinp.push_back( inp );

		// simulate Ctrl+A keystrokes in the filename edit control to select all text
		memset( &inp, 0, sizeof(inp) );
		inp.type = INPUT_KEYBOARD;
		inp.ki.wVk = VK_CONTROL;
		vinp.push_back( inp );
		inp.ki.wVk = 'A';
		vinp.push_back( inp );
		inp.ki.dwFlags = KEYEVENTF_KEYUP; 
		vinp.push_back( inp );
		inp.ki.wVk = VK_CONTROL;
		vinp.push_back( inp );

		// simulate keystrokes in the filename edit control to put the destination 
		// path into it
		AddTextInput( &vinp, pathBuf );

		// simulate return key
		memset( &inp, 0, sizeof(inp) );
		inp.type = INPUT_KEYBOARD;
		inp.ki.wVk = VK_RETURN;
		vinp.push_back( inp );
		inp.ki.dwFlags = KEYEVENTF_KEYUP; 
		vinp.push_back( inp );

		// simulate Ctrl+A keystrokes in the filename edit control to select all text
		memset( &inp, 0, sizeof(inp) );
		inp.type = INPUT_KEYBOARD;
		inp.ki.wVk = VK_CONTROL;
		vinp.push_back( inp );
		inp.ki.wVk = 'A';
		vinp.push_back( inp );
		inp.ki.dwFlags = KEYEVENTF_KEYUP; 
		vinp.push_back( inp );
		inp.ki.wVk = VK_CONTROL;
		vinp.push_back( inp );

		// simulate keystrokes in the filename edit control to put the original
		// text into it
		AddTextInput( &vinp, oldEditTxt );

		// simulate mouse move to restore old cursor pos
		memset( &inp, 0, sizeof(inp) );
		inp.type = INPUT_MOUSE;
		inp.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
		inp.mi.dx = oldCursorPos.x * 65535 / ::GetSystemMetrics( SM_CXSCREEN );
		inp.mi.dy = oldCursorPos.y * 65535 / ::GetSystemMetrics( SM_CYSCREEN );
		vinp.push_back( inp );

		//--- send the prepared input stream ---

		::SendInput( vinp.size(), &vinp[0], sizeof(INPUT) );

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::GetFolder( LPTSTR folderPath )
{
	::GetCurrentDirectory( MAX_PATH, folderPath );
	return true;
}

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::SetFilter( LPCTSTR filter )
{
	return true;
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK MsoFileDlgHook::HookWindowProc(
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
/*
		case WM_USER + 300:
			// I don't really know what this message means, but I need it to check
			// whether the user has changed the actual folder path.
            if( g_pHook->m_isWindowActive )
				g_pHook->m_pCallbacks->OnFolderChange();
			break;
*/          
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

void MsoFileDlgHook::ResizeFileDialog()
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
	if( style & WS_SIZEBOX ) 
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
	SetWindowPos( m_hwndFileDlg, NULL, 
		newX, newY, newWidth, newHeight, SWP_NOZORDER | SWP_NOACTIVATE );
}


