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
#include "ImportDlg.h"

//-----------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CImportDlg, CDialog)
CImportDlg::CImportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportDlg::IDD, pParent)
{
}

//-----------------------------------------------------------------------------------------------

void CImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CImportDlg, CDialog)
	ON_BN_CLICKED(IDC_CHK_INSTALLED, OnBnClickedCheckInstalled)
	ON_BN_CLICKED(IDC_BTN_BROWSE, OnBnClickedBtnBrowse)
	ON_EN_CHANGE(IDC_ED_PATH, OnEnChangeEdit1)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CImportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	tstring dummy, tcIniPath;
	if( GetTotalCmdLocation( &dummy, &tcIniPath ) )
	{
		m_installedIniPath = tcIniPath.c_str();
		m_installedIniPath.MakeLower();
		SetDlgItemText( IDC_ED_PATH, m_installedIniPath );
	}
	CheckDlgButton( IDC_RD_APPEND, 1 );

	return FALSE;
}

//-----------------------------------------------------------------------------------------------

void CImportDlg::OnBnClickedCheckInstalled()
{
	if( IsDlgButtonChecked( IDC_CHK_INSTALLED ) )
		SetDlgItemText( IDC_ED_PATH, m_installedIniPath );
}

//-----------------------------------------------------------------------------------------------

void CImportDlg::OnEnChangeEdit1()
{
	CString s; GetDlgItemText( IDC_ED_PATH, s );
	s.Trim();
	::PathCanonicalize( m_enteredPath.GetBuffer( MAX_PATH ), s );
	m_enteredPath.ReleaseBuffer();
	CheckDlgButton( IDC_CHK_INSTALLED, 
		m_enteredPath.CompareNoCase( m_installedIniPath ) == 0 ? 1 : 0 );

	GetDlgItem( IDOK )->EnableWindow( ! m_enteredPath.IsEmpty() );
}

//-----------------------------------------------------------------------------------------------

void CImportDlg::OnBnClickedBtnBrowse()
{
	CFileDialog dlg( TRUE, _T("ini"), _T("wincmd.ini"), 
		OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_PATHMUSTEXIST, 
		_T("INI files (*.ini)|*.ini|All files (*.*)|*.*||"), this );
	if( dlg.DoModal() == IDOK )
	{
		SetDlgItemText( IDC_ED_PATH, dlg.GetPathName() );
	}
}

//-----------------------------------------------------------------------------------------------

void CImportDlg::OnOK()
{
	if( ! FileExists( m_enteredPath ) )
	{
		CRect rc; GetDlgItem( IDC_ED_PATH )->GetWindowRect( rc );
		CBalloonHelp::LaunchBalloon( _T("Error"), _T("This file could not be found."), 
			rc.CenterPoint(), IDI_ERROR, CBalloonHelp::unCLOSE_ON_ANYTHING );
		return;
	}
	// check if TC directory menu is redirected to another .INI file
	CString redirSection;
	if( ::GetPrivateProfileString( _T("DirMenu"), _T("RedirectSection"), _T(""), 
			redirSection.GetBuffer( MAX_PATH ),	MAX_PATH, m_enteredPath ) )
	{
		redirSection.ReleaseBuffer();
		CString exRedirSection;
		::ExpandEnvironmentStrings( redirSection, exRedirSection.GetBuffer( MAX_PATH ), MAX_PATH );
		exRedirSection.ReleaseBuffer();
		if( ! FileExists( exRedirSection ) )
		{

		}
		m_enteredPath = exRedirSection;
	}

	m_replaceExisting = IsDlgButtonChecked( IDC_RD_REPLACE ) != 0;
	
	EndDialog( IDOK );
}

//-----------------------------------------------------------------------------------------------
// Get directory menu from Total Commander INI file.

void GetTcFavorites( FavoritesList* pFavs, CString tcIniPath )
{
	for( int i = 1;; ++i )
	{
		CString key, command, title, targetPath;

		key.Format( _T("menu%d"), i );
		if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), 
				title.GetBuffer( 256 ),	256, tcIniPath ) == 0 )
			break;
		title.ReleaseBuffer();

		FavoritesItem favItem;
		favItem.title = title;

		key.Format( _T("cmd%d"), i );
		if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), 
				command.GetBuffer( 4096 ), 4096, tcIniPath ) ) 
		{
			command.ReleaseBuffer();
			favItem.command = command;
		}
		key.Format( _T("path%d"), i );
		if( ::GetPrivateProfileString( _T("DirMenu"), key, _T(""), 
			targetPath.GetBuffer( MAX_PATH ), MAX_PATH, tcIniPath ) ) 
		{
			targetPath.ReleaseBuffer();
			favItem.targetpath = targetPath;
		}

		pFavs->push_back( favItem );
	}
}