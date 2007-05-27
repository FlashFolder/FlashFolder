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
#include "PageCommonOpenWithDlg.h"
#include "ExcludesDlg.h"

const CString PROFILE_GROUP = _T("CommonOpenWithDlg");

//-----------------------------------------------------------------------------------------------

CPageCommonOpenWithDlg::CPageCommonOpenWithDlg()
	: base(CPageCommonOpenWithDlg::IDD)
{}

//-----------------------------------------------------------------------------------------------

void CPageCommonOpenWithDlg::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_CB_POSITION, m_cbPos );
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageCommonOpenWithDlg, CPageCommonOpenWithDlg::base)
	ON_BN_CLICKED(IDC_BTN_EXCLUDES, OnBnClickedBtnExcludes)
	ON_BN_CLICKED(IDC_CHK_ENABLE, OnBnClickedChkEnable)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPageCommonOpenWithDlg::OnInitDialog()
{
	base::OnInitDialog();

	//--- init controls

	int cxs = ::GetSystemMetrics( SM_CXSCREEN );
	int cys = ::GetSystemMetrics( SM_CYSCREEN );

	GetDlgItem( IDC_SP_MINWIDTH )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cxs, 0 ) );
	GetDlgItem( IDC_SP_MINHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );

	//--- get profile data

	CString s;
	CheckDlgButton( IDC_CHK_ENABLE, g_profile.GetInt( PROFILE_GROUP, _T("EnableHook") ) );
	s.Format( _T("%d"), g_profile.GetInt( PROFILE_GROUP, _T("MinWidth") ) );
	SetDlgItemText( IDC_ED_MINWIDTH, s );
	s.Format( _T("%d"), g_profile.GetInt( PROFILE_GROUP, _T("MinHeight") ) );
	SetDlgItemText( IDC_ED_MINHEIGHT, s );
	m_cbPos.SetCurSel( g_profile.GetInt( PROFILE_GROUP, _T("Center") ) );

	for( int i = 0;; ++i )
	{
		CString key; key.Format( _T("%d"), i );
		CString name = g_profile.GetString( PROFILE_GROUP + _T(".Excludes"), key ).c_str();
		if( name.IsEmpty() )
			break;
		m_excludes.push_back( name );
	}

	// Set initial enabled state for child controls
	OnBnClickedChkEnable();

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

BOOL CPageCommonOpenWithDlg::OnApply()
{
	CString s;
	g_profile.SetInt( PROFILE_GROUP, _T("EnableHook"), IsDlgButtonChecked( IDC_CHK_ENABLE ) );
	GetDlgItemText( IDC_ED_MINWIDTH, s );
	g_profile.SetInt( PROFILE_GROUP, _T("MinWidth"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_MINHEIGHT, s );
	g_profile.SetInt( PROFILE_GROUP, _T("MinHeight"), _ttoi( s ) );
	g_profile.SetInt( PROFILE_GROUP, _T("Center"), m_cbPos.GetCurSel() );

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

void CPageCommonOpenWithDlg::OnBnClickedChkEnable()
{
	bool isEnabled = IsDlgButtonChecked( IDC_CHK_ENABLE ) != 0;
	for( CWnd* pChild = GetWindow( GW_CHILD ); pChild; pChild = pChild->GetNextWindow() )
		if( pChild->GetDlgCtrlID() != IDC_CHK_ENABLE )
			pChild->EnableWindow( isEnabled );
}

//-----------------------------------------------------------------------------------------------

void CPageCommonOpenWithDlg::OnBnClickedBtnExcludes()
{
	CExcludesDlg dlg( this );
	CString s; 
	s.LoadString( IDS_EXCLUDES );
	dlg.SetDescr( s );
	dlg.SetStrings( m_excludes );
	if( dlg.DoModal() == IDOK )
		dlg.GetStrings( &m_excludes );
}