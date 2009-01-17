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
 *
 */

#include <stdafx.h>

#include "ff_utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------------------

BOOL CALLBACK DebugEnumChildProc( HWND hwnd, LPARAM lParam )
{
	HWND parent = ::GetParent( hwnd );
	DWORD id = ::GetDlgCtrlID( hwnd );
	TCHAR sclass[ 256 ] = L"";
	TCHAR stitle[ 256 ] = L"";
	::GetClassName( hwnd, sclass, _countof( sclass ) );
	::GetWindowText( hwnd, stitle, _countof( sclass ) );
	DebugOut( L"hwnd = %04x, parent = %04x, id = %d, class = '%s', title = '%s'\n", 
		hwnd, parent, id, sclass, stitle );  

	return TRUE;	
}

//-----------------------------------------------------------------------------------------
// GetFileDlgType()
//
//   checks, whether given window handle is the handle of a new-style 
//   common file dialog or the MS Office file dialog
//-----------------------------------------------------------------------------------------
FileDlgType GetFileDlgType( HWND dlg )
{
	HWND hStat1, hEditFileName, hShellView, hDriveListBox;

	TCHAR className[256] = _T("");
    ::GetClassName(dlg, className, sizeof(className) / sizeof(TCHAR) - 1 );
	if( _tcscmp( className, _T("#32770") ) != 0 )
	{
		// Detect variants of the MS Office file dialog.
		// In different versions of this dialog the ID / classname of the filename edit control
		// changes.

		if( _tcsncmp( className, _T("bosa_sdm_"), 9 ) == 0 )
		{
			if( ::FindWindowEx( dlg, NULL, _T("Snake List"), NULL ) )
			{		
				if( hEditFileName = GetDlgItem( dlg, VS2005_FILEDLG_ED_FILENAME ) )
				{
					className[0] = 0;
					::GetClassName( hEditFileName, className, sizeof(className) / sizeof(TCHAR) - 1 );
					if( _tcscmp( className, _T("Edit") ) == 0 )
						return FileDlgType( FDT_MSOFFICE, FDT_VS2005 );
				}

				FileDlgType res( FDT_MSOFFICE );

				if( hEditFileName = GetDlgItem( dlg, MSO2002_FILEDLG_ED_FILENAME ) )
					res.subType = FDT_MSO2002;
				else if( hEditFileName = GetDlgItem( dlg, MSO2000_FILEDLG_ED_FILENAME ) )
					res.subType = FDT_MSO2000;
				
				if( hEditFileName )
				{
					className[0] = 0;
					::GetClassName( hEditFileName, className, sizeof(className) / sizeof(TCHAR) - 1 );
					if( _tcsncmp( className, _T("RichEdit20"), 10 ) == 0 )
						return res;
				}
			}
		}
		return FileDlgType( FDT_NONE );
	}

	// Detect the common folder dialog.
	
	if( HWND hWnd = ::GetDlgItem( dlg, 0 ) )
	{
		className[0] = 0;
		::GetClassName( hWnd, className, sizeof(className) / sizeof(TCHAR) - 1 );
		if( _tcscmp( className, _T("SHBrowseForFolder ShellNameSpace Control") ) == 0 )
			return FileDlgType( FDT_COMMON_FOLDER );
	}
	
	//DebugOut( L"--- childs windows of %04x ---\n", dlg );
	//::EnumChildWindows( dlg, DebugEnumChildProc, 0 );

	// To detect the common file dlg we check various control IDs and class names
	// that should exist only in this dialog. Ideally we would want to check
	// for the control with "SHELLDLL_DefView" class but this is created only later by
	// the file dialog.

	if( (hShellView = GetDlgItem(dlg, FILEDLG_LB_SHELLVIEW)) == NULL )
		return FileDlgType( FDT_NONE );
	className[0] = 0;
    ::GetClassName(hShellView, className, sizeof(className) / sizeof(TCHAR) - 1 );
	if( _tcscmp( className, _T("ListBox") ) != 0)
		return FileDlgType( FDT_NONE );

	if( (hStat1 = GetDlgItem(dlg, FILEDLG_ST_SEARCH)) == NULL )
		return FileDlgType( FDT_NONE );	
	className[0] = 0;
    ::GetClassName( hStat1, className, sizeof(className) / sizeof(TCHAR) - 1 );
	if( _tcscmp( className, _T("Static") ) != 0)
		return FileDlgType( FDT_NONE );

	if ((hEditFileName = GetDlgItem(dlg, FILEDLG_ED_FILENAME)) == NULL)
		return FileDlgType( FDT_NONE );
	className[0] = 0;
    ::GetClassName( hEditFileName, className, sizeof(className) / sizeof(TCHAR) - 1 );
	if( _tcscmp( className, _T("Edit") ) != 0)
		return FileDlgType( FDT_NONE );

	// Filter out old-style (Win 3.1) open/save dialogs. They are detect by checking 
	// for the "drive listbox" that doesn't exists in new style dialogs (control ID is
	// the same as the shellview, which is created only later by new file dialogs).
	if( ( hDriveListBox = GetDlgItem( dlg, FILEDLG_SHELLVIEW ) ) != NULL )
		return FileDlgType( FDT_NONE );

	return FileDlgType( FDT_COMMON );
}

//-----------------------------------------------------------------------------------------
// FileDlgSetFilter()
//
//   sets a filter for the specified common file dialog
//   --> should be called only in the address space of the process which owns the
//       file dialog
//   returns true if successful
//-----------------------------------------------------------------------------------------
bool FileDlgSetFilter( HWND hwndFileDlg, LPCTSTR filter )
{
	if ( filter == NULL || filter[0] == 0 || 
		 (( _tcschr( filter, _T('*') ) == NULL ) && ( _tcschr( filter, _T('?') ) == NULL )) )
	   return false;

	TCHAR oldEditTxt[1024] = _T(""); 
    HWND hEditFileName = ::GetDlgItem( hwndFileDlg, FILEDLG_ED_FILENAME );

    // in NT systems, the filename edit control can actually be a combobox
    if( ! hEditFileName )
        hEditFileName = ::GetDlgItem( hwndFileDlg, FILEDLG_CB_FILENAME );

	if( hEditFileName )
	{
        HWND hBtnOk = ::GetDlgItem( hwndFileDlg, IDOK );
		if( hBtnOk )
		{
			// Make sure the listcontrol is not focused, otherwise it would react on BN_CLICKED.
			HWND hOldFocus = ::GetFocus();
			if( hOldFocus != hEditFileName )
				::SendMessage( hwndFileDlg, WM_NEXTDLGCTL, (WPARAM) hEditFileName, TRUE ); 

            ::GetWindowText( hEditFileName, oldEditTxt, (sizeof(oldEditTxt) - 1) * sizeof(TCHAR) );
            ::SetWindowText( hEditFileName, filter );
            ::SendMessage( hwndFileDlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM) hBtnOk );
			//only restore old text if it was not a filter
			if( ( _tcschr( oldEditTxt, _T('*') ) == NULL ) &&
				( _tcschr( oldEditTxt, _T('?') ) == NULL) )
                ::SetWindowText( hEditFileName, oldEditTxt );

			// restore focus
			if( hOldFocus && hOldFocus != hEditFileName )
				::PostMessage( hwndFileDlg, WM_NEXTDLGCTL, (WPARAM) hOldFocus, TRUE ); 

			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------------------
/// Browses in a shell view to a given folder.
///
/// Tested successfully with both common file dialog and MS Office file dialog on WinXP.
///
/// \return true if successful

bool ShellViewBrowseToFolder( HWND hwnd, LPCWSTR path )
{	
	if( ! DirectoryExists( path ) )
		return false;

	bool res = false;

    IShellBrowser *psb = (IShellBrowser *) SendMessage( hwnd, WM_GETISHELLBROWSER, 0, 0 );
    if( ! psb )
		return false;
		
	CComPtr<IShellFolder> pDesktopFolder;
	if( FAILED( ::SHGetDesktopFolder( &pDesktopFolder ) ) )
		return false;
		
	LPITEMIDLIST pidl;
	if( FAILED( pDesktopFolder->ParseDisplayName( hwnd, NULL, const_cast<LPWSTR>( path ), NULL, &pidl, NULL ) ) )
		return false;

	HRESULT hr = psb->BrowseObject( pidl, SBSP_SAMEBROWSER );
	::CoTaskMemFree( pidl );

	return SUCCEEDED( hr );
}

//-----------------------------------------------------------------------------------------
/**
 * \brief Returns path to folder who's content is currently visible in a shell folder view.
 *
 * Tested successfully with both common file dialog and MS Office file dialog on WinXP.
 * 
 * \param hwnd Window handle that contains the shell view. This should be the parent window
 *             of the control which has the "SHELLDLL_DefView" class. 
 * \param path Receives the path if the folder is from the file system. Otherwise an empty string
 *             returned. The buffer must be at least MAX_PATH characters in size.
 * \retval true success
 * \retval false failure
 */
bool ShellViewGetCurrentFolder( HWND hwnd, LPTSTR path )
{
	path[ 0 ] = 0;

	IShellBrowser *psb = (IShellBrowser*) ::SendMessage( hwnd, WM_GETISHELLBROWSER, 0, 0 );
	if( ! psb )
		return false;
		
	CComPtr<IShellView> psv;
	if( FAILED( psb->QueryActiveShellView( &psv ) ) )
		return false;
		
	CComPtr<IFolderView> pfv;
	if( FAILED( psv->QueryInterface( IID_IFolderView, (void**) &pfv ) ) )
		return false;
		 
	CComPtr<IPersistFolder2> ppf2;
	if( FAILED( pfv->GetFolder( IID_IPersistFolder2, (void**) &ppf2 ) ) )
		return false;
		
	LPITEMIDLIST pidlFolder;
	if( SUCCEEDED( ppf2->GetCurFolder( &pidlFolder ) ) ) 
	{
		bool res = false;
		if( ::SHGetPathFromIDList( pidlFolder, path ) )
			res = true;
		::CoTaskMemFree( pidlFolder );
		return res;
	}
	
	return false;
}

//-----------------------------------------------------------------------------------------

bool ShellViewGetViewMode( HWND hwnd, FOLDERVIEWMODE* pViewMode, int* pImageSize )
{
	IShellBrowser *psb = (IShellBrowser*) ::SendMessage( hwnd, WM_GETISHELLBROWSER, 0, 0 );
	if( ! psb )
		return false;

	CComPtr<IShellView> psv;
	if( FAILED( psb->QueryActiveShellView( &psv ) ) ) 
		return false;

	CComPtr<IFolderView2> pfv2;  
	if( SUCCEEDED( psv->QueryInterface( IID_IFolderView2, (void**) &pfv2 ) ) )
	{		
		// Vista and above
	
		return SUCCEEDED( pfv2->GetViewModeAndIconSize( pViewMode, pImageSize ) );
	}
	else
	{
		// XP and below
		
		CComPtr<IFolderView> pfv;
		if( FAILED( psv->QueryInterface( IID_IFolderView, (void**) &pfv ) ) ) 
			return false;

		UINT viewMode = FVM_AUTO;
		if( SUCCEEDED( pfv->GetCurrentViewMode( &viewMode ) ) )
		{
			*pViewMode = (FOLDERVIEWMODE) viewMode;
			*pImageSize = -1;
			return true;
		}
	}
		
	return false;
}

//-----------------------------------------------------------------------------------------

bool ShellViewSetViewMode( HWND hwnd, FOLDERVIEWMODE viewMode, int imageSize )
{
	IShellBrowser *psb = (IShellBrowser*) ::SendMessage( hwnd, WM_GETISHELLBROWSER, 0, 0 );
	if( ! psb )
		return false;

	CComPtr<IShellView> psv;
	if( FAILED( psb->QueryActiveShellView( &psv ) ) )
		return false;

	CComPtr<IFolderView2> pfv2;  
	if( SUCCEEDED( psv->QueryInterface( IID_IFolderView2, (void**) &pfv2 ) ) )
	{		
		// Vista and above
	
		return SUCCEEDED( pfv2->SetViewModeAndIconSize( viewMode, imageSize ) );
	}
	else
	{
		// XP and below
		
		CComPtr<IFolderView> pfv;
		if( FAILED( psv->QueryInterface( IID_IFolderView, (void**) &pfv ) ) )
			return false;

		return SUCCEEDED( pfv->SetCurrentViewMode( (UINT) viewMode ) );
	}
}