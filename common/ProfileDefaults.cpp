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
#include "stdafx.h"
#include "ProfileDefaults.h"

//-----------------------------------------------------------------------------------------
void GetProfileDefaults( Profile* profile )
{
	LONG baseUnits = ::GetDialogBaseUnits();
	int baseUnitX = baseUnits & 0xFFFF;
	int baseUnitY = baseUnits >> 16;

	profile->Clear();

	//--- general

	profile->SetInt( L"main", L"MaxGlobalHistoryEntries", 15 );
	profile->SetInt( L"main", L"ListViewMode", -1 );
	profile->SetInt( L"main", L"ListViewImageSize", -1 );

	//--- common file dialog

	profile->SetInt( L"CommonFileDlg", L"EnableHook", 1 );
	profile->SetInt( L"CommonFileDlg", L"MinWidth", 433 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonFileDlg", L"MinHeight", 308 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonFileDlg", L"Center", 1 );
	profile->SetInt( L"CommonFileDlg", L"FolderComboHeight", 400 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonFileDlg", L"FiletypesComboHeight", 246 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonFileDlg", L"ResizeNonResizableDialogs", 1 );
	profile->SetString( L"CommonFileDlg.NonResizableExcludes", L"0", L"i_view32.exe" );
	profile->SetString( L"CommonFileDlg.Excludes", L"0", L"iTunes.exe" );

	//--- common folder dialog

	profile->SetInt( L"CommonFolderDlg", L"EnableHook", 1 );
	profile->SetInt( L"CommonFolderDlg", L"MinWidth", 267 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonFolderDlg", L"MinHeight", 308 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonFolderDlg", L"Center", 1 );
	profile->SetString( L"CommonFolderDlg.Excludes", L"0", L"iTunes.exe" );

	//--- MSO file dialog

	profile->SetInt( L"MSOfficeFileDlg", L"EnableHook", 1 );
	profile->SetInt( L"MSOfficeFileDlg", L"MinWidth", 433 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"MSOfficeFileDlg", L"MinHeight", 308 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"MSOfficeFileDlg", L"Center", 1 );

	//--- common "Open With" dialog

	profile->SetInt( L"CommonOpenWithDlg", L"EnableHook", 0 );
	profile->SetInt( L"CommonOpenWithDlg", L"MinWidth", 267 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonOpenWithDlg", L"MinHeight", 308 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"CommonOpenWithDlg", L"Center", 1 );

	//--- favorites editor

	profile->SetInt( L"main", L"FavoritesDlgWidth", 450 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"main", L"FavoritesDlgHeight", 330 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"main", L"UseTcFavorites", 0 );
	profile->SetInt( L"Favorites.Options", L"ColWidth_title", 130 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"Favorites.Options", L"ColWidth_command", 140 + DIALOG_UNITS_FLAG );
	profile->SetInt( L"Favorites.Options", L"ColWidth_targetPath", 140 + DIALOG_UNITS_FLAG );

	//--- toolbar
	
	profile->SetInt( L"Toolbar", L"OffsetX", 0 );
	profile->SetInt( L"Toolbar", L"OffsetY", 0 );
	profile->SetInt( L"Toolbar", L"OffsetWidth", 0 );

	//--- file manager integration

	profile->SetString( L"FileManager", L"FavoritesSrc.PluginName", L"FF_FileMgrPlugin.dll" );
	profile->SetString( L"FileManager", L"FavoritesSrc.ProgramId", L"ID_Explorer" );
}
