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
// GetFileDlgType()
//
//   checks, whether given window handle is the handle of a new-style 
//   common file dialog or the MS Office file dialog
//-----------------------------------------------------------------------------------------
FileDlgType GetFileDlgType( HWND dlg )
{
	HWND hStat1, hCombFolder, hEditFileName, hShellView, hDriveListBox;

	TCHAR className[256] = _T("");
    ::GetClassName(dlg, className, sizeof(className) / sizeof(TCHAR) - 1 );
	if( _tcscmp( className, _T("#32770") ) != 0 )
	{
		// Detect variants of the MS Office file dialog.
		// In different versions of this dialog the ID / classname of the filename edit control
		// changes.

		if( _tcsncmp( className, _T("bosa_sdm_"), 9 ) == 0 )
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
		return FileDlgType( FDT_NONE );
	}

	// detect the common "Open With" dialog
	// TODO: do it only if Win2k is running
	if( HWND hWnd = ::GetDlgItem( dlg, 0x3009 ) )
	{
		className[0] = 0;
		::GetClassName( hWnd, className, sizeof(className) / sizeof(TCHAR) - 1 );
		if( _tcscmp( className, _T("Static") ) == 0 )
			if( hWnd = ::GetDlgItem( dlg, 0x3003 ) )
			{
				className[0] = 0;
				::GetClassName( hWnd, className, sizeof(className) / sizeof(TCHAR) - 1 );
				if( _tcscmp( className, _T("Static") ) == 0 )
					if( hWnd = ::GetDlgItem( dlg, 0x3605 ) )
					{
						className[0] = 0;
						::GetClassName( hWnd, className, sizeof(className) / sizeof(TCHAR) - 1 );
						if( _tcscmp( className, _T("SysListView32") ) == 0 )
							return FileDlgType( FDT_COMMON_OPENWITH );							
					}
			}
	}         

	// detect the common folder dialog
	if( HWND hWnd = ::GetDlgItem( dlg, 0 ) )
	{
		className[0] = 0;
		::GetClassName( hWnd, className, sizeof(className) / sizeof(TCHAR) - 1 );
		if( _tcscmp( className, _T("SHBrowseForFolder ShellNameSpace Control") ) == 0 )
			return FileDlgType( FDT_COMMON_FOLDER );
	}

	// to detect the common file dlg I check various control IDs and class names
	// that exist only in the common file dlg

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

	if ((hCombFolder = GetDlgItem(dlg, FILEDLG_CB_FOLDER)) == NULL)
		return FileDlgType( FDT_NONE );
	className[0] = 0;
    ::GetClassName( hCombFolder, className, sizeof(className) / sizeof(TCHAR) - 1 );
	if( _tcscmp( className, _T("ComboBox") ) != 0)
		return FileDlgType( FDT_NONE );

	if ((hEditFileName = GetDlgItem(dlg, FILEDLG_ED_FILENAME)) == NULL)
		return FileDlgType( FDT_NONE );
	className[0] = 0;
    ::GetClassName( hEditFileName, className, sizeof(className) / sizeof(TCHAR) - 1 );
	if( _tcscmp( className, _T("Edit") ) != 0)
		return FileDlgType( FDT_NONE );

	// filter out old-style (Win 3.1) open/save dialogs
	//   we detect them by checking for the "drive listbox" that doesn't
	//   exists in new style dialogs
	if( (hDriveListBox = GetDlgItem(dlg, FILEDLG_CB_OLD_DRIVES)) != NULL )
		return FileDlgType( FDT_NONE );

	return FileDlgType( FDT_COMMON );
}

//-----------------------------------------------------------------------------------------
// FileDlgBrowseToFolder()
//
//   browses in a common file dialog to a specific folder 
//   --> needs Win2k or newer version
//   --> should be called only in the address space of the process which owns the
//       file dialog
//   returns true if successful
//-----------------------------------------------------------------------------------------
bool FileDlgBrowseToFolder( HWND hwndFileDlg, LPCTSTR path )
{	
	if( ! DirectoryExists( path ) )
		return false;

	bool res = false;

    WCHAR wpath[MAX_PATH + 1] = L"";

    IShellBrowser *pShellBrowser = 
        (IShellBrowser *) SendMessage(hwndFileDlg, WM_GETISHELLBROWSER, 0,0);
    if (pShellBrowser != NULL)
    {
		//get the shell memory allocator object
		IMalloc *pShellAlloc = NULL;			
		if (SHGetMalloc(&pShellAlloc) == NOERROR)
		{
			IShellFolder *pDesktopFolder;
			if (SHGetDesktopFolder(&pDesktopFolder) == NOERROR)
			{                
#ifdef _UNICODE
                wcsncpy( wpath, path, MAX_PATH );
#else
                ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, path, -1, wpath, MAX_PATH);
                LPWSTR p_wpath = wpath;
#endif
				ULONG eaten;
				LPITEMIDLIST pidl;
				if (pDesktopFolder->ParseDisplayName(hwndFileDlg, NULL, wpath, &eaten, &pidl, NULL)
					== NOERROR)
				{
					pShellBrowser->BrowseObject(pidl, SBSP_SAMEBROWSER );
					pShellAlloc->Free(pidl);
					res = true;
				}
				pDesktopFolder->Release();
			}

			//release the shell memory allocator object
			pShellAlloc->Release();
		}
    }

	return res;
}

//-----------------------------------------------------------------------------------------
// FileDlgGetCurrentFolder()
//
//    Returns path to folder who's content is currently visible in a common file dialog
//    Caller must have allocated folderPath with size of MAX_PATH  characters
//-----------------------------------------------------------------------------------------
bool FileDlgGetCurrentFolder(HWND hwndFileDlg, LPTSTR folderPath )
{
	bool res = false;

	folderPath[0] = 0;

    LPITEMIDLIST pidl = NULL;
    int pidlSize = 0, cb;
    do 
    {
        pidlSize += 1024;
        pidl = (LPITEMIDLIST) realloc(pidl, pidlSize);
        cb = (int) SendMessage(hwndFileDlg, CDM_GETFOLDERIDLIST, pidlSize, (LPARAM) pidl);        
    }
    while (cb > pidlSize);

    if (cb > 0)
		res = (SHGetPathFromIDList( pidl, folderPath ) == TRUE);

    free(pidl);
	return res;
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
/**
 * \brief Returns path to folder who's content is currently visible in a shell folder view.
 *
 * Tested successfully with both common file dialog and MS Office file dialog on WinXP.\n
 * TODO: Win2k - find a way to work around IFolderView which isn't supported under this OS.
 * 
 * \param hwnd Window handle that contains the shell view. This should be the parent window
 *             of the control which has the "SHELLDLL_DefView" class. 
 * \param path Receives the path if the folder is from the file system. Otherwise an empty string
 *             returned. The buffer must be at least MAX_PATH characters in size.
 * \retval true success
 * \retval false failure
 */
bool ShellView_GetCurrentDir( HWND hwnd, LPTSTR path )
{
	path[ 0 ] = 0;
	bool res = false;

	IShellBrowser *pShBrowser = 
		reinterpret_cast<IShellBrowser*>( ::SendMessage( hwnd, WM_GETISHELLBROWSER, 0, 0 ) );
	if( pShBrowser )
	{
		IShellView* psv;
		if( SUCCEEDED( pShBrowser->QueryActiveShellView( &psv ) ) )
		{	
			IFolderView* pfv;
			if( SUCCEEDED( psv->QueryInterface( IID_IFolderView, (void**) &pfv ) ) ) 
			{
				IPersistFolder2* ppf2;
				if( SUCCEEDED( pfv->GetFolder( IID_IPersistFolder2, (void**) &ppf2 ) ) )
				{
					LPITEMIDLIST pidlFolder;
					if( SUCCEEDED( ppf2->GetCurFolder( &pidlFolder ) ) ) 
					{
						if( ::SHGetPathFromIDList( pidlFolder, path ) )
							res = true;
						::CoTaskMemFree( pidlFolder );
					}
					ppf2->Release();
				}
				pfv->Release();
			}

			psv->Release();
		}
	}
	return res;
}