/* This file is part of FlashFolder. 
 * Copyright (C) 2007-2012 zett42.de ( zett42 at users.sourceforge.net ) 
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

// Undocumented message for explorer / common file dialog to get a pointer to the IShellBrowser instance.
const UINT WM_GETISHELLBROWSER = WM_USER + 7;

inline IShellBrowser* GetShellBrowser( HWND hwnd )
	{ return reinterpret_cast<IShellBrowser*>( ::SendMessage( hwnd, WM_GETISHELLBROWSER, 0, 0 ) ); }

bool FileDlgSetFilter( HWND hwndFileDlg, LPCWSTR filter );

HRESULT ShellViewGetCurrentFolder( IShellBrowser *psb, SpITEMIDLIST* pidlResult );
bool ShellViewGetViewMode( IShellBrowser *psb, FOLDERVIEWMODE* pViewMode, int* pImageSize );
bool ShellViewSetViewMode( IShellBrowser* psb, FOLDERVIEWMODE viewMode, int imageSize );

HRESULT ShellViewGetCurrentFolder( HWND hwnd, SpITEMIDLIST* pidlResult );
HRESULT ShellViewSetCurrentFolder( HWND hwnd, PCIDLIST_ABSOLUTE folder );
bool ShellViewGetViewMode( HWND hwnd, FOLDERVIEWMODE* pViewMode, int* pImageSize = NULL );
bool ShellViewSetViewMode( HWND hwnd, FOLDERVIEWMODE viewMode, int imageSize = -1 );

// some control ID's of common file dialog
const unsigned FILEDLG_SHELLVIEW    = 1121;
const unsigned FILEDLG_LB_SHELLVIEW = 1120;
const unsigned FILEDLG_ST_SEARCH    = 1091;
const unsigned FILEDLG_CB_FOLDER    = 1137;
const unsigned FILEDLG_CB_FILETYPES = 1136;
const unsigned FILEDLG_ED_FILENAME  = 1152;
const unsigned FILEDLG_CB_FILENAME  = 1148;      // sometimes in Win NT systems instead of 1152
const unsigned FILEDLG_SAVEAS_VISTA_ED_FILENAME = 1001;   // New "Save as" dialog beginning with Vista.

// some control ID's of the MS Office file dialog
const unsigned MSO2000_FILEDLG_ED_FILENAME = 48;
const unsigned MSO2002_FILEDLG_ED_FILENAME = 54;
const unsigned VS2005_FILEDLG_ED_FILENAME = 51;   ///< e.g. Visual Studio 2005
