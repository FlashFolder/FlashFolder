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
#include "PageCommonFileDlg.h"
#include "ExcludesDlg.h"

using namespace std;

const CString PROFILE_GROUP = _T("CommonFileDlg");

//-----------------------------------------------------------------------------------------------

CPageCommonFileDlg::CPageCommonFileDlg()
	: base(CPageCommonFileDlg::IDD)
{}

//-----------------------------------------------------------------------------------------------

void CPageCommonFileDlg::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CB_POSITION, m_cbPos );
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP( CPageCommonFileDlg, CPageCommonFileDlg::base )
	ON_BN_CLICKED( IDC_BTN_EXCLUDES, OnBnClickedBtnExcludes )
	ON_BN_CLICKED( IDC_BTN_NONRESIZABLE_EXCLUDES, OnBnClickedBtnNonresizableExcludes )
	ON_BN_CLICKED( IDC_CHK_ENABLE, OnBnClickedChkEnable )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPageCommonFileDlg::OnInitDialog()
{
	base::OnInitDialog();

	//--- init controls

	int cxs = ::GetSystemMetrics( SM_CXSCREEN );
	int cys = ::GetSystemMetrics( SM_CYSCREEN );

	GetDlgItem( IDC_SP_MINWIDTH )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cxs, 0 ) );
	GetDlgItem( IDC_SP_MINHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );
	GetDlgItem( IDC_SP_FOLDERCOMBO_MAXHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );
	GetDlgItem( IDC_SP_FILETYPECOMBO_MAXHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );

	//--- get profile data

	CString s;
	CheckDlgButton( IDC_CHK_ENABLE, g_profile.GetInt( PROFILE_GROUP, _T("EnableHook") ) );
	s.Format( _T("%d"), g_profile.GetInt( PROFILE_GROUP, _T("MinWidth") ) );
	SetDlgItemText( IDC_ED_MINWIDTH, s );
	s.Format( _T("%d"), g_profile.GetInt( PROFILE_GROUP, _T("MinHeight") ) );
	SetDlgItemText( IDC_ED_MINHEIGHT, s );
	m_cbPos.SetCurSel( g_profile.GetInt( PROFILE_GROUP, _T("Center") ) );
	CheckDlgButton( IDC_CHK_RESIZENONRESIZABLE, g_profile.GetInt( PROFILE_GROUP, 
		_T("ResizeNonResizableDialogs") ) );
	s.Format( _T("%d"), g_profile.GetInt( PROFILE_GROUP, _T("FiletypesComboHeight") ) );
	SetDlgItemText( IDC_ED_FILETYPECOMBO_MAXHEIGHT, s );
	s.Format( _T("%d"), g_profile.GetInt( PROFILE_GROUP, _T("FolderComboHeight") ) );
	SetDlgItemText( IDC_ED_FOLDERCOMBO_MAXHEIGHT, s );

	for( int i = 0;; ++i )
	{
		CString key; key.Format( _T("%d"), i );
		CString name = g_profile.GetString( _T("CommonFileDlg.NonResizableExcludes"), key ).c_str();
		if( name.IsEmpty() )
			break;
		m_nonResizableExcludes.push_back( name );
	}
	for( int i = 0;; ++i )
	{
		CString key; key.Format( _T("%d"), i );
		CString name = g_profile.GetString( _T("CommonFileDlg.Excludes"), key ).c_str();
		if( name.IsEmpty() )
			break;
		m_excludes.push_back( name );
	}

	// Set initial enabled state for child controls
	OnBnClickedChkEnable();

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

BOOL CPageCommonFileDlg::OnApply()
{
	CString s;
	g_profile.SetInt( PROFILE_GROUP, _T("EnableHook"), IsDlgButtonChecked( IDC_CHK_ENABLE ) );
	GetDlgItemText( IDC_ED_MINWIDTH, s );
	g_profile.SetInt( PROFILE_GROUP, _T("MinWidth"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_MINHEIGHT, s );
	g_profile.SetInt( PROFILE_GROUP, _T("MinHeight"), _ttoi( s ) );
	g_profile.SetInt( PROFILE_GROUP, _T("Center"), m_cbPos.GetCurSel() );
	g_profile.SetInt( PROFILE_GROUP, _T("ResizeNonResizableDialogs"), 
		IsDlgButtonChecked( IDC_CHK_RESIZENONRESIZABLE ) );
	GetDlgItemText( IDC_ED_FILETYPECOMBO_MAXHEIGHT, s );
	g_profile.SetInt( PROFILE_GROUP, _T("FiletypesComboHeight"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_FOLDERCOMBO_MAXHEIGHT, s );
	g_profile.SetInt( PROFILE_GROUP, _T("FolderComboHeight"), _ttoi( s ) );

	g_profile.ClearSection( PROFILE_GROUP + _T(".NonResizableExcludes") );
	for( int i = 0; i != m_nonResizableExcludes.size(); ++i )
	{
		CString key; key.Format( _T("%d"), i );
		g_profile.SetString( PROFILE_GROUP + _T(".NonResizableExcludes"), key, 
			m_nonResizableExcludes[ i ] );
	}
	g_profile.ClearSection( PROFILE_GROUP + _T(".Excludes") );
	for( int i = 0; i != m_excludes.size(); ++i )
	{
		CString key; key.Format( _T("%d"), i );
		g_profile.SetString( PROFILE_GROUP + _T(".Excludes"), key, 
			m_excludes[ i ] );
	}
	
	return base::OnApply();
}

//-----------------------------------------------------------------------------------------------

void CPageCommonFileDlg::OnBnClickedChkEnable()
{
	bool isEnabled = IsDlgButtonChecked( IDC_CHK_ENABLE ) != 0;
	for( CWnd* pChild = GetWindow( GW_CHILD ); pChild; pChild = pChild->GetNextWindow() )
		if( pChild->GetDlgCtrlID() != IDC_CHK_ENABLE )
			pChild->EnableWindow( isEnabled );
}

//-----------------------------------------------------------------------------------------------

void CPageCommonFileDlg::OnBnClickedBtnExcludes()
{
	CExcludesDlg dlg( this );
	CString s; s.LoadString( IDS_EXCLUDES );
	dlg.SetDescr( s );
	dlg.SetStrings( m_excludes );
	if( dlg.DoModal() == IDOK )
	{
		dlg.GetStrings( &m_excludes );
		SetModified();
	}
}

//-----------------------------------------------------------------------------------------------

void CPageCommonFileDlg::OnBnClickedBtnNonresizableExcludes()
{
	CExcludesDlg dlg( this );
	CString s; s.LoadString( IDS_RESIZE_NONRESIZABLE_EXCL );
	dlg.SetDescr( s );
	dlg.SetStrings( m_nonResizableExcludes );
	if( dlg.DoModal() == IDOK )
	{
		dlg.GetStrings( &m_nonResizableExcludes );
		SetModified();
	}
}