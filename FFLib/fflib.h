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

// GUID for various purposes
const TCHAR FF_GUID[] = _T("{163F258C-65E0-483d-8B7A-5ABD9E3D4487}");

// Window class of FlashFolder toolbar
const TCHAR FF_WNDCLASSNAME[] = _T("FlashFolder_3832795"); 

extern RegistryProfile g_profile;
extern HINSTANCE g_hInstDll;

int FavMenu_Display( HWND hWndParent, int x, int y, const FavoritesList& favs );
void FavMenu_StartEditor( HWND hWndParent );
void FavMenu_AddDir( HWND hWndParent, FavoritesList& favs, LPCTSTR pPath, LPCTSTR pTargetPath );
