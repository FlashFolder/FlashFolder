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
#pragma once

//-----------------------------------------------------------------------------------------
/// Hook for Total Commander window to receive hotkey messages for showing FlashFolder 
/// favorites menu.

class TotalCmdHook
{
public:
	TotalCmdHook( HWND hwndTC ); 

private:
	static LRESULT CALLBACK HookWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void RegisterMyHotkeys();
	void UnregisterMyHotkeys();
	void OnHotkey( HWND hwnd, WPARAM wp, LPARAM lp );

	HWND m_hwndTC;
	WNDPROC m_oldWndProc;
	ATOM m_favmenuHotkeyAtom;
};
