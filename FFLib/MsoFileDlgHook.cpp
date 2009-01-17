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

bool MsoFileDlgHook::Init( HWND hwndFileDlg, HWND hwndTool )
{
	if( m_hwndFileDlg ) return false;  // only init once!
	m_hwndFileDlg = hwndFileDlg;
	m_hwndTool = hwndTool; 

	DebugOut( _T("[fflib] MsoFileDlgHook::Init()\n") );

	m_currentDir[ 0 ] = 0;

	g_pHook = this;

	// Create a hook for watching ESC key to determine if file dialog has been canceled.
	// Just watching for ESC in WM_KEYDOWN of file dialog would not be enough, since ESC in
	// the file name edit control would not be catched this way.
	m_hKeyboardHook = ::SetWindowsHookEx( WH_KEYBOARD, KeyboardHookProc, NULL, ::GetCurrentThreadId() ); 

	// Registered message that is send by the MSO file dialog
	m_wmObjectSel = ::RegisterWindowMessage( _T("WM_OBJECTSEL") );

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
	m_minFileDialogWidth = MapProfileX( hwndTool, g_profile.GetInt( _T("MSOfficeFileDlg"), _T("MinWidth") ) );
	m_minFileDialogHeight = MapProfileY( hwndTool, g_profile.GetInt( _T("MSOfficeFileDlg"), _T("MinHeight") ) );
	m_centerFileDialog = g_profile.GetInt( _T("MSOfficeFileDlg"), _T("Center") );

	return true;
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK MsoFileDlgHook::KeyboardHookProc( int code, WPARAM wParam, LPARAM lParam )
{
	if( code == HC_ACTION && wParam == VK_ESCAPE )
		g_pHook->m_fileDialogCanceled = true;

	return ::CallNextHookEx( g_pHook->m_hKeyboardHook, code, wParam, lParam );
}

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::EnterFilenameEditText( LPCTSTR pText )
{
	// MSO seems to partly use its own windowless controls. E.g. calling 
	// SetFocus() seems to put the cursor into the edit control but the internal focus 
	// state of the windowless controls isn't changed.
	// The only reliable method to automate the MSO dialog seems to simulate mouse- and
	// keyboard input.

	TCHAR oldEditTxt[1024] = _T(""); 

	HWND hEditFileName = NULL;

	switch( m_subType )
	{
		case FDT_MSO2000:
			hEditFileName = ::GetDlgItem( m_hwndFileDlg, MSO2000_FILEDLG_ED_FILENAME );
			break;
		case FDT_MSO2002:
			hEditFileName = ::GetDlgItem( m_hwndFileDlg, MSO2002_FILEDLG_ED_FILENAME );
			break;
		case FDT_VS2005:
			hEditFileName = ::GetDlgItem( m_hwndFileDlg, VS2005_FILEDLG_ED_FILENAME );
			break;
	}

	if( ! hEditFileName )
		return false;

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

	// simulate keystrokes in the filename edit control to put the given text
	// into it
	AddTextInput( &vinp, pText );

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

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::SetFolder( LPCTSTR path )
{	
	// append backslash to path if necessary
	TCHAR pathBuf[MAX_PATH + 1 ];
	StringCbCopy( pathBuf, sizeof(pathBuf), path );
	size_t len = _tcslen( pathBuf );
	if( len > 0 )
	{
		TCHAR* p = _tcsninc( pathBuf, len - 1 );
        if( *p != _T('\\') ) 
			StringCbCat( pathBuf, sizeof(pathBuf), _T("\\") );
	}

	return EnterFilenameEditText( path );
}

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::GetFolder( LPTSTR folderPath )
{
	StringCchCopy( folderPath, MAX_PATH, m_currentDir );
	return true;
}

//-----------------------------------------------------------------------------------------

bool MsoFileDlgHook::SetFilter( LPCTSTR filter )
{
	return EnterFilenameEditText( filter );
}

//-----------------------------------------------------------------------------------------

void MsoFileDlgHook::OnTimer()
{
	if( ! g_pHook->m_isWindowActive )
		return;

	// Check if the current path has changed. If so, notify the tool window.
	// I do this since I didn't found a message that is send by the file dialog when the path
	// has changed. And who knows if this message would be the same for all versions of MSO?

	if( HWND hwnd = ::FindWindowEx( g_pHook->m_hwndFileDlg, NULL, _T("Snake List"), NULL ) ) 
	{
		TCHAR curDir[ MAX_PATH + 1 ] = _T("");
		ShellViewGetCurrentFolder( hwnd, curDir );
	
		if( _tcsicmp( curDir, m_currentDir ) != 0 )
		{
			StringCbCopy( m_currentDir, sizeof(m_currentDir), curDir );
			FileDlgHookCallbacks::OnFolderChange();
		}
	}
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

				// create timer for polling current directory
				FileDlgHookCallbacks::SetTimer( 75 );

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
				// Note: this doesn't work for office 2000, 2002 but keep it in since it might work for
				// other versions.

				if( wID == IDCANCEL )	      
					g_pHook->m_fileDialogCanceled = true;
			}
		}
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
			FileDlgHookCallbacks::OnDestroy( ! g_pHook->m_fileDialogCanceled );
			break;

		case WM_NCDESTROY:
		{
			// Remove our window property from the file dialog.
			// We stored it there in CmnFileDlgHook::Init().
			::RemoveProp( hwnd, FLASHFOLDER_HOOK_PROPERTY );     

			if( g_pHook->m_hKeyboardHook )
				::UnhookWindowsHookEx( g_pHook->m_hKeyboardHook );
		}
		break;

		case WM_CLOSE:
			// WM_CLOSE is only received when the dialog is closed via the window close button
			g_pHook->m_fileDialogCanceled = true;
		break;
    }

	if( uMsg == g_pHook->m_wmObjectSel )
	{
		// The registered window message "WM_OBJECTSEL" is send when the user sets the 
		// input focus to another control in the file dialog.

		g_pHook->m_fileDialogCanceled = lParam == 2;  // cancel button selected
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
