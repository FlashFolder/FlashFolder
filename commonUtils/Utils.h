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
void GetAppFilename( HINSTANCE hInstApp, LPTSTR pName );

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

int ComparePath( LPCTSTR path1, LPCTSTR path2 );


bool IsIniSectionNotEmpty( LPCTSTR filename, LPCTSTR sectionName );

void AddTextInput( std::vector<INPUT>* pInput, LPCTSTR pText );

/// Get runtime OS version, high-byte = major version, low-byte = minor version.
/// e.g. 0x0500 = Win2k, 0x0501 = WinXP, 0x0600 = Vista
inline WORD GetOsVersion()
{
	OSVERSIONINFO ovi = { sizeof(ovi) };
	::GetVersionEx( &ovi );
	return static_cast<WORD>( ovi.dwMajorVersion << 8 | ovi.dwMinorVersion );
}

inline bool IsShiftKeyPressed() 
	{ return (GetKeyState(VK_SHIFT) & (1 << (sizeof(SHORT)*8-1))) != 0; }
inline bool IsCtrlKeyPressed()  
	{ return (GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8-1))) != 0; }
inline bool IsAltKeyPressed()  
	{ return (GetKeyState(VK_MENU) & (1 << (sizeof(SHORT)*8-1))) != 0; }


/// Set enabled state of dialog control but switch focus if it is on a disabled item.\n
/// Otherwise, if using EnableWindow() alone, focus can be lost and cannot be 
/// activated again by using keyboard.
void EnableDlgItem( HWND hDlg, UINT idCtrl, BOOL bEnable = TRUE );

/// Send a formatted message to the debugger (or external tool). 
/// Message size can be max. 1024 TCHARs.
void DebugOut( LPCTSTR pFormat, ... );
