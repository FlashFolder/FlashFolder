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

//-----------------------------------------------------------------------------------------------

enum FileDlgType { FDT_NONE, FDT_COMMON, FDT_MSOFFICE, FDT_COMMON_OPENWITH, FDT_COMMON_FOLDER };
FileDlgType GetFileDlgType( HWND dlg );

bool FileDlgBrowseToFolder( HWND hwndFileDlg, LPCTSTR path );
bool FileDlgGetCurrentFolder( HWND hwndFileDlg, LPTSTR folderPath );
bool FileDlgSetFilter( HWND hwndFileDlg, LPCTSTR filter );

bool ShellView_GetCurrentDir( HWND hwnd, LPTSTR path );

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

