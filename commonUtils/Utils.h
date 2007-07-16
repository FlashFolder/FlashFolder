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
 
/// \file Various platform utilities.
 
#pragma once

#include <vector>

void GetAppDir( HINSTANCE hInstApp, LPTSTR szDir );

inline bool DirectoryExists( LPCTSTR szName )
{
    DWORD res = ::GetFileAttributes( szName );
    return (res != -1) && (res & FILE_ATTRIBUTE_DIRECTORY);
}
inline bool FileExists( LPCTSTR szName )
{
    DWORD res = ::GetFileAttributes( szName );
    return (res != -1) && (! (res & FILE_ATTRIBUTE_DIRECTORY));
}

bool IsFilePath( LPCTSTR path );

bool IsRelativePath( LPCTSTR path );

void GetTempFilePath( LPTSTR pResult, LPCTSTR pPrefix );

bool IsIniSectionNotEmpty( LPCTSTR filename, LPCTSTR sectionName );

void AddTextInput( std::vector<INPUT>* pInput, LPCTSTR pText );

inline void ScreenToClientRect( HWND hwnd, RECT* prc )
{
	POINT pt1 = { prc->left, prc->top };
	::ScreenToClient( hwnd, &pt1 );
	POINT pt2 = { prc->right, prc->bottom };
	::ScreenToClient( hwnd, &pt2 );
	prc->left = pt1.x; prc->top = pt1.y; prc->right = pt2.x; prc->bottom = pt2.y;
}
