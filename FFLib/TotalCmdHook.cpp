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
#include "TotalCmdHook.h"

//-----------------------------------------------------------------------------------------

namespace {
	const TCHAR FLASHFOLDER_HOOK_PROPERTY[] = _T("FlashFolderHook.k5evf782uxt56a8zp29");
};

// for easy access from the subclassed WindowProc
TotalCmdHook* g_pHook = NULL;

//-----------------------------------------------------------------------------------------------

void RegisterTcHotkeys();
void UnregisterTcHotkeys();

//-----------------------------------------------------------------------------------------

TotalCmdHook::TotalCmdHook( HWND hwndTC ) : 
	m_hwndTC( hwndTC ),
	m_favmenuHotkeyAtom( 0 )
{
	::OutputDebugString( _T("[fflib] TotalCmdHook::Init()\n") );

	g_pHook = this;

	// Subclass the window proc.
	m_oldWndProc = reinterpret_cast<WNDPROC>( 
		::SetWindowLongPtr( hwndTC, GWL_WNDPROC, 
		                    reinterpret_cast<LONG_PTR>( HookWindowProc) ) );
	_ASSERTE( m_oldWndProc );

	// Set a window property for debugging purposes (makes identifying "hooked" windows easier).
	::SetProp( hwndTC, FLASHFOLDER_HOOK_PROPERTY, NULL );
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK TotalCmdHook::HookWindowProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
		case WM_ACTIVATE:
		{
			bool isActive = LOWORD( wParam ) != WA_INACTIVE;
			if( isActive )
				g_pHook->RegisterMyHotkeys();
			else
				g_pHook->UnregisterMyHotkeys();		
		}
		break;

		case WM_DESTROY:
		{
			g_pHook->UnregisterMyHotkeys();
		}
		break;
		
		case WM_HOTKEY:
		{
			g_pHook->OnHotkey( hwnd, wParam, lParam );
		}
		break;

		case WM_NCDESTROY:
		{			
			// Remove our window property from the file dialog.
			// We stored it there in TotalCmdHook::Init().
			::RemoveProp( hwnd, FLASHFOLDER_HOOK_PROPERTY );     
		}
		break;
    }

    //call original message handler
    return CallWindowProc( g_pHook->m_oldWndProc, hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------------------------

void TotalCmdHook::RegisterMyHotkeys()
{
	if( m_favmenuHotkeyAtom != 0 )
		return;

	int hotkey = g_profile.GetInt( _T("Hotkeys"), _T("ff_MenuFavorites") );  
	if( hotkey == 0 )
		return;

	// use GlobalAddAtom() to avoid conflicts with hotkey IDs of other programs / DLLs
	tstring atomName = tstring( _T("ff_MenuFavorites.") ) + tstring( FF_GUID );
	m_favmenuHotkeyAtom = ::GlobalAddAtom( atomName.c_str() );

	UINT vk, mod;
	SplitHotKey( &vk, &mod, hotkey );
	if( ! ::RegisterHotKey( m_hwndTC, m_favmenuHotkeyAtom, mod, vk ) )
	{
		::GlobalDeleteAtom( m_favmenuHotkeyAtom );
		m_favmenuHotkeyAtom = 0;
	}
}

//-----------------------------------------------------------------------------------------------

void TotalCmdHook::UnregisterMyHotkeys()
{
	if( m_favmenuHotkeyAtom != 0 )
	{
		::UnregisterHotKey( m_hwndTC, m_favmenuHotkeyAtom );
		::GlobalDeleteAtom( m_favmenuHotkeyAtom );		
		m_favmenuHotkeyAtom = 0;
	}
}

//-----------------------------------------------------------------------------------------------

void TotalCmdHook::OnHotkey( HWND hwnd, WPARAM wp, LPARAM lp )
{
	if( hwnd != m_hwndTC || hwnd != ::GetForegroundWindow() )
		return;

	ATOM hotkeyAtom = static_cast<ATOM>( wp );
	TCHAR atomName[ 256 ] = _T("");
	::GlobalGetAtomName( hotkeyAtom, atomName, 255 );

	tstring favmenuAtomName = tstring( _T("ff_MenuFavorites.") ) + tstring( FF_GUID );
	if( favmenuAtomName != atomName )
		return;
		
	// Open favorites menu below the TC control which contains the path of the source directory.
	CTotalCmdUtils tcUtils( m_hwndTC );
	HWND activePathWnd = tcUtils.IsLeftDirActive() ? tcUtils.GetLeftPathWnd() : tcUtils.GetRightPathWnd();
	RECT rc; ::GetWindowRect( activePathWnd, &rc );
	FavMenu_DisplayForTotalCmd( m_hwndTC, rc.left, rc.bottom );
}
