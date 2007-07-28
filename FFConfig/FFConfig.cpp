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
#include "stdafx.h"
#include "FFConfig.h"
#include "FFConfigDlg.h"
#include "FolderFavoritesDlg.h"
#include "UpdateCheckDlg.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------------------------

// The one and only CFFConfigApp object
CFFConfigApp g_app;

// The one and only Profile object
RegistryProfile g_profile( _T("zett42\\FlashFolder") );
MemoryProfile g_profileDefaults;

//-----------------------------------------------------------------------------------------------

void ActivateWindow( LPCTSTR pWndClassName )
{
    if( HWND hwnd = ::FindWindow( pWndClassName, NULL ) )
    {
        ::SetForegroundWindow( hwnd );
        if( ::IsIconic( hwnd ) )
            ::ShowWindowAsync( hwnd, SW_RESTORE );
    }
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFFConfigApp, CWinApp)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

CFFConfigApp::CFFConfigApp() :
	CWinApp( _T("FlashFolder") )
{}

//-----------------------------------------------------------------------------------------------

BOOL CFFConfigApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxInitRichEdit2();

	HWND hwndParent = NULL;
	if( __argc > 1 )
		hwndParent = reinterpret_cast<HWND>( _ttoi64( __targv[ 1 ] ) );

	enum 
	{ 
		DLG_CONFIG, DLG_FAVS, DLG_UPDATECHECK, DLG_ABOUT 
	} 
	dlgType = DLG_CONFIG;
	
	if( __argc > 2 )
		if( _tcscmp( __targv[ 2 ], _T("--fav") ) == 0 )
			dlgType = DLG_FAVS;
		else if( _tcscmp( __targv[ 2 ], _T("--updatecheck") ) == 0 )
			dlgType = DLG_UPDATECHECK;
		else if( _tcscmp( __targv[ 2 ], _T("--about") ) == 0 )
			dlgType = DLG_ABOUT;

	//--- running application instance detection 
	// Allow one instance for each dialog.
	
    //Unique name to identify the mutex and the main window of the program.

	const TCHAR* pUniqueAppId = NULL;

	switch( dlgType )
	{
		case DLG_CONFIG:
			pUniqueAppId = _T("FFConfig.xmgn4ngertu4mnsf");
			break;
		case DLG_FAVS:
			pUniqueAppId = _T("FFFavorites.xmgn4ngertu4mnsf");
			break;
		case DLG_UPDATECHECK:
			pUniqueAppId = _T("FFUpdateCheck.xmgn4ngertu4mnsf");
			break;
		case DLG_ABOUT:
			pUniqueAppId = _T("FFAbout.xmgn4ngertu4mnsf");
			break;
	}
	// --> make sure that dialogs in .RC-file have correct CLASS name!

	CHandle hMutex( ::CreateMutex( NULL, TRUE, pUniqueAppId ) );
	if( ::GetLastError() == ERROR_ALREADY_EXISTS )
	{
		// another instance of this dialog is already running 
		ActivateWindow( pUniqueAppId  );
		return FALSE;
	}	    


	// Register the dialog class that is specified in the .rc-file
	WNDCLASS wc = { 0 };
	wc.hInstance = AfxGetInstanceHandle();
	wc.lpszClassName = pUniqueAppId;
	wc.cbWndExtra = DLGWINDOWEXTRA;
	wc.lpfnWndProc = ::DefDlgProc;
	wc.hIcon = LoadIcon( IDR_MAINFRAME );
	wc.hCursor = LoadStandardCursor( IDC_ARROW );
	::RegisterClass( &wc );

	// intialize default profile settings
	GetProfileDefaults( &g_profileDefaults );
	g_profile.SetDefaults( &g_profileDefaults );

	// By specifying the handle of the parent window, dialogs of FFConfig behave like modal dialogs to
	// the program where FlashFolder toolbar is currently attached to.
	switch( dlgType )
	{
		case DLG_CONFIG:
		{
			CFFConfigDlg dlg( hwndParent ? CWnd::FromHandle( hwndParent ) : NULL );
			m_pMainWnd = &dlg;
			dlg.DoModal();
			break;
		}
		case DLG_FAVS:
		{
			CFolderFavoritesDlg dlg( hwndParent ? CWnd::FromHandle( hwndParent ) : NULL );
			m_pMainWnd = &dlg;
			dlg.DoModal();
			break;
		}
		case DLG_UPDATECHECK:
		{
			// Do not specify the parent window here, so that a blocking network connection 
			// doesn't block the current program where FlashFolder is attached to.
			CUpdateCheckDlg dlg; 			
			m_pMainWnd = &dlg;
			dlg.DoModal();
			break;
		}
		case DLG_ABOUT:
		{
			CAboutDlg dlg( hwndParent ? CWnd::FromHandle( hwndParent ) : NULL );
			m_pMainWnd = &dlg;
			dlg.DoModal();
			break;
		}
	}

	return FALSE;
}
