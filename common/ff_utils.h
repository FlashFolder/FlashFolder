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

enum FileDlgMainType { FDT_NONE, FDT_COMMON, FDT_MSOFFICE, FDT_COMMON_OPENWITH, FDT_COMMON_FOLDER };
enum FileDlgSubType { FDT_SUB_NONE, FDT_MSO2000, FDT_MSO2002, FDT_VS2005 };

struct FileDlgType
{
	FileDlgMainType mainType;
	FileDlgSubType subType;

	explicit FileDlgType( FileDlgMainType mt = FDT_NONE, FileDlgSubType st = FDT_SUB_NONE ) : 
		mainType( mt ), subType( st ) {}
};

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
const unsigned VS2005_FILEDLG_ED_FILENAME = 51;   ///< e.g. Visual Studio 2005

// reverse-engineered command codes for SHELLDLL_DefView
enum ShellListViewMode
{
	ODM_VIEW_ICONS  = 0x7029,
	ODM_VIEW_LIST   = 0x702B,
	ODM_VIEW_DETAIL = 0x702C,
	ODM_VIEW_THUMBS = 0x702D,
	ODM_VIEW_TILES  = 0x702E
};

// constants for storing listview modes
enum FFListViewMode
{
	FLM_VIEW_DEFAULT = -1,
	FLM_VIEW_ICONS   = 0,
	FLM_VIEW_LIST    = 1,
	FLM_VIEW_DETAIL  = 2,
	FLM_VIEW_THUMBS  = 3,
	FLM_VIEW_TILES   = 4
};