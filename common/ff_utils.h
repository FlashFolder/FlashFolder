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
#if !defined(_FF_UTILS_H__INCLUDED_)
#define _FF_UTILS_H__INCLUDED_

#include <windows.h>
#include <tchar.h>
#include <map>
#include <vector>
#include <string>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//-------------------------------------------------------------------------------------------------

void GetAppDir( HINSTANCE hInstApp, LPTSTR szDir );
bool DirectoryExists( LPCTSTR szName );
bool FileExists( LPCTSTR szName ); 
bool IsFilePath( LPCTSTR path );
bool IsRelativePath( LPCTSTR path );

bool IsIniSectionNotEmpty( LPCTSTR filename, LPCTSTR sectionName );

void AddTextInput( std::vector<INPUT>* pInput, LPCTSTR pText );

enum FileDlgType { FDT_NONE, FDT_COMMON, FDT_MSOFFICE, FDT_COMMON_OPENWITH, FDT_COMMON_FOLDER };
FileDlgType GetFileDlgType( HWND dlg );

bool FileDlgBrowseToFolder( HWND hwndFileDlg, LPCTSTR path );
bool FileDlgGetCurrentFolder( HWND hwndFileDlg, LPTSTR folderPath );
bool FileDlgSetFilter( HWND hwndFileDlg, LPCTSTR filter );

// undocumented message for explorer / common file dialog
const UINT WM_GETISHELLBROWSER = WM_USER + 7;

// some control ID's of common file dialog
const unsigned FILEDLG_LB_SHELLVIEW = 1120;
const unsigned FILEDLG_ST_SEARCH    = 1091;
const unsigned FILEDLG_CB_FOLDER    = 1137;
const unsigned FILEDLG_CB_FILETYPES = 1136;
const unsigned FILEDLG_ED_FILENAME  = 1152;
const unsigned FILEDLG_CB_FILENAME  = 1148;      // sometimes in Win NT systems instead of 1152
const unsigned FILEDLG_CB_OLD_DRIVES = 1121;      // Win 3.1 style dialogs only 

// some control ID's of the MS Office file dialog
const unsigned MSO2000_FILEDLG_ED_FILENAME = 48;
const unsigned MSO2002_FILEDLG_ED_FILENAME = 54;

inline void ScreenToClientRect( HWND hwnd, RECT* prc )
{
	POINT pt1 = { prc->left, prc->top };
	::ScreenToClient( hwnd, &pt1 );
	POINT pt2 = { prc->right, prc->bottom };
	::ScreenToClient( hwnd, &pt2 );
	prc->left = pt1.x; prc->top = pt1.y; prc->right = pt2.x; prc->bottom = pt2.y;
}

//-------------------------------------------------------------------------------------------------

#endif // !defined(_FF_UTILS_H__INCLUDED_)
