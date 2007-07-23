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
#include "ProfileDefaults.h"

//-----------------------------------------------------------------------------------------
void GetProfileDefaults( Profile* pProfile )
{
	LONG baseUnits = ::GetDialogBaseUnits();
	int baseUnitX = baseUnits & 0xFFFF;
	int baseUnitY = baseUnits >> 16;

	pProfile->Clear();

	//--- general

	pProfile->SetInt( _T("main"), _T("MaxGlobalHistoryEntries"), 15 );

	tstring tcIniPath;
	bool isTcInstalled = GetTotalCmdLocation( NULL, &tcIniPath );
	pProfile->SetInt( _T("main"), _T("UseTcFavorites"), isTcInstalled ? 1 : 0 );

	//--- common file dialog

	pProfile->SetInt( _T("CommonFileDlg"), _T("EnableHook"), 1 );
	pProfile->SetInt( _T("CommonFileDlg"), _T("MinWidth"), 433 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonFileDlg"), _T("MinHeight"), 308 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonFileDlg"), _T("Center"), 1 );
	pProfile->SetInt( _T("CommonFileDlg"), _T("FolderComboHeight"), 400 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonFileDlg"), _T("FiletypesComboHeight"), 246 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonFileDlg"), _T("ResizeNonResizableDialogs"), 1 );
	pProfile->SetString( _T("CommonFileDlg.NonResizableExcludes"), _T("0"), _T("i_view32.exe") );
	pProfile->SetString( _T("CommonFileDlg.Excludes"), _T("0"), _T("iTunes.exe") );

	//--- common folder dialog

	pProfile->SetInt( _T("CommonFolderDlg"), _T("EnableHook"), 1 );
	pProfile->SetInt( _T("CommonFolderDlg"), _T("MinWidth"), 267 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonFolderDlg"), _T("MinHeight"), 308 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonFolderDlg"), _T("Center"), 1 );
	pProfile->SetString( _T("CommonFolderDlg.Excludes"), _T("0"), _T("iTunes.exe") );

	//--- MSO file dialog

	pProfile->SetInt( _T("MSOfficeFileDlg"), _T("EnableHook"), 0 );
	pProfile->SetInt( _T("MSOfficeFileDlg"), _T("MinWidth"), 433 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("MSOfficeFileDlg"), _T("MinHeight"), 308 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("MSOfficeFileDlg"), _T("Center"), 1 );

	//--- common "Open With" dialog

	pProfile->SetInt( _T("CommonOpenWithDlg"), _T("EnableHook"), 0 );
	pProfile->SetInt( _T("CommonOpenWithDlg"), _T("MinWidth"), 267 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonOpenWithDlg"), _T("MinHeight"), 308 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("CommonOpenWithDlg"), _T("Center"), 1 );

	//--- favorites editor

	pProfile->SetInt( _T("main"), _T("FavoritesDlgWidth"), 532 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("main"), _T("FavoritesDlgHeight"), 431 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("Favorites.Options"), _T("ColWidth_title"), 110 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("Favorites.Options"), _T("ColWidth_command"), 130 + DIALOG_UNITS_FLAG );
	pProfile->SetInt( _T("Favorites.Options"), _T("ColWidth_targetPath"), 130 + DIALOG_UNITS_FLAG );
}
