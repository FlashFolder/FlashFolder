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
#include "StringListDlg.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------------------------

CFFConfigDlg::CFFConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFFConfigDlg::IDD, pParent)
{}

//-----------------------------------------------------------------------------------------------

void CFFConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_CHK_CFO_ENABLE, m_chkCFO );
	DDX_Control( pDX, IDC_CHK_CFD_ENABLE, m_chkCFD );
	DDX_Control( pDX, IDC_CHK_MSO_ENABLE, m_chkMSO );
	DDX_Control( pDX, IDC_CHK_COW_ENABLE, m_chkCOW );
	DDX_Control( pDX, IDC_CB_CFO_POSITION, m_cbCFO_pos );
	DDX_Control( pDX, IDC_CB_CFD_POSITION, m_cbCFD_pos );
	DDX_Control( pDX, IDC_CB_MSO_POSITION, m_cbMSO_pos );
	DDX_Control( pDX, IDC_CB_COW_POSITION, m_cbCOW_pos );
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFFConfigDlg, CDialog)
	ON_BN_CLICKED( IDC_BTN_NONRESIZABLE_EXCLUDES, OnBtnNonResizableExcl )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CFFConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//--- setup up/down controls range, if not done up/down is reverted

	int cxs = ::GetSystemMetrics( SM_CXSCREEN );
	int cys = ::GetSystemMetrics( SM_CYSCREEN );

	GetDlgItem( IDC_SP_MAX_DIRHISTORY )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( 50, 0 ) );

	GetDlgItem( IDC_SP_CFO_MINWIDTH )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cxs, 0 ) );
	GetDlgItem( IDC_SP_CFO_MINHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );
	GetDlgItem( IDC_SP_FOLDERCOMBO_MAXHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );
	GetDlgItem( IDC_SP_FILETYPECOMBO_MAXHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );

	GetDlgItem( IDC_SP_CFD_MINWIDTH )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cxs, 0 ) );
	GetDlgItem( IDC_SP_CFD_MINHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );

	GetDlgItem( IDC_SP_MSO_MINWIDTH )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cxs, 0 ) );
	GetDlgItem( IDC_SP_MSO_MINHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );

	GetDlgItem( IDC_SP_COW_MINWIDTH )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cxs, 0 ) );
	GetDlgItem( IDC_SP_COW_MINHEIGHT )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( cys, 0 ) );

	//--- associate checkboxes with groupboxes

	m_chkCFO.SetGroupbox( IDC_GP_CFO );
	m_chkCFD.SetGroupbox( IDC_GP_CFD );
	m_chkMSO.SetGroupbox( IDC_GP_MSO );
	m_chkCOW.SetGroupbox( IDC_GP_COW );

	//--- get profile data

	CString s;
	s.LoadString( g_profile.IsShared() ? IDS_MU_SHARED : IDS_MU_INDIVIDUAL );
	SetDlgItemText( IDC_ST_MULTIUSER, s );
	
	s.Format( _T("%d"), g_profile.GetInt( _T("main"), _T("MaxGlobalHistoryEntries") ) );
	SetDlgItemText( IDC_ED_MAX_DIRHISTORY, s );

	//--- common file dialog

	CheckDlgButton( IDC_CHK_CFO_ENABLE, g_profile.GetInt( _T("CommonFileDlg"), _T("EnableHook") ) );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonFileDlg"), _T("MinWidth") ) );
	SetDlgItemText( IDC_ED_CFO_MINWIDTH, s );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonFileDlg"), _T("MinHeight") ) );
	SetDlgItemText( IDC_ED_CFO_MINHEIGHT, s );
	m_cbCFO_pos.SetCurSel( g_profile.GetInt( _T("CommonFileDlg"), _T("Center") ) );
	CheckDlgButton( IDC_CHK_CFO_RESIZENONRESIZABLE, g_profile.GetInt( _T("CommonFileDlg"), 
		_T("ResizeNonResizableDialogs") ) );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonFileDlg"), _T("FiletypesComboHeight") ) );
	SetDlgItemText( IDC_ED_FILETYPECOMBO_MAXHEIGHT, s );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonFileDlg"), _T("FolderComboHeight") ) );
	SetDlgItemText( IDC_ED_FOLDERCOMBO_MAXHEIGHT, s );

	for( int i = 0;; ++i )
	{
		CString key; key.Format( _T("%d"), i );
		CString name = g_profile.GetString( _T("CommonFileDlg.NonResizableExcludes"), key ).c_str();
		if( name.IsEmpty() )
			break;
		m_cfoNonResizableExcludes.push_back( name );
	}

	//--- common folder dialog

	CheckDlgButton( IDC_CHK_CFD_ENABLE, g_profile.GetInt( _T("CommonFolderDlg"), _T("EnableHook") ) );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonFolderDlg"), _T("MinWidth") ) );
	SetDlgItemText( IDC_ED_CFD_MINWIDTH, s );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonFolderDlg"), _T("MinHeight") ) );
	SetDlgItemText( IDC_ED_CFD_MINHEIGHT, s );
	m_cbCFD_pos.SetCurSel( g_profile.GetInt( _T("CommonFolderDlg"), _T("Center") ) );

	//--- MSO file dialog

	CheckDlgButton( IDC_CHK_MSO_ENABLE, g_profile.GetInt( _T("MSOfficeFileDlg"), _T("EnableHook") ) );
	s.Format( _T("%d"), g_profile.GetInt( _T("MSOfficeFileDlg"), _T("MinWidth") ) );
	SetDlgItemText( IDC_ED_MSO_MINWIDTH, s );
	s.Format( _T("%d"), g_profile.GetInt( _T("MSOfficeFileDlg"), _T("MinHeight") ) );
	SetDlgItemText( IDC_ED_MSO_MINHEIGHT, s );
	m_cbMSO_pos.SetCurSel( g_profile.GetInt( _T("MSOfficeFileDlg"), _T("Center") ) );
	
	//--- common "Open With" dialog

	CheckDlgButton( IDC_CHK_COW_ENABLE, g_profile.GetInt( _T("CommonOpenWithDlg"), _T("EnableHook") ) );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonOpenWithDlg"), _T("MinWidth") ) );
	SetDlgItemText( IDC_ED_COW_MINWIDTH, s );
	s.Format( _T("%d"), g_profile.GetInt( _T("CommonOpenWithDlg"), _T("MinHeight") ) );
	SetDlgItemText( IDC_ED_COW_MINHEIGHT, s );
	m_cbCOW_pos.SetCurSel( g_profile.GetInt( _T("CommonOpenWithDlg"), _T("Center") ) );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------------------------

void CFFConfigDlg::OnOK()
{
	//--- save profile data

	// Multi-User option is currently read-only since it would require more work like
	// checking for admin rights, elevation on Vista, etc.

	CString s;

	GetDlgItemText( IDC_ED_MAX_DIRHISTORY, s );
	g_profile.SetInt( _T("main"), _T("MaxGlobalHistoryEntries"), _ttoi( s ) );

	//--- common file dialog

	g_profile.SetInt( _T("CommonFileDlg"), _T("EnableHook"), IsDlgButtonChecked( IDC_CHK_CFO_ENABLE ) );
	GetDlgItemText( IDC_ED_CFO_MINWIDTH, s );
	g_profile.SetInt( _T("CommonFileDlg"), _T("MinWidth"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_CFO_MINHEIGHT, s );
	g_profile.SetInt( _T("CommonFileDlg"), _T("MinHeight"), _ttoi( s ) );
	g_profile.SetInt( _T("CommonFileDlg"), _T("Center"), m_cbCFO_pos.GetCurSel() );
	g_profile.SetInt( _T("CommonFileDlg"), _T("ResizeNonResizableDialogs"), 
		IsDlgButtonChecked( IDC_CHK_CFO_RESIZENONRESIZABLE ) );
	GetDlgItemText( IDC_ED_FILETYPECOMBO_MAXHEIGHT, s );
	g_profile.SetInt( _T("CommonFileDlg"), _T("FiletypesComboHeight"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_FOLDERCOMBO_MAXHEIGHT, s );
	g_profile.SetInt( _T("CommonFileDlg"), _T("FolderComboHeight"), _ttoi( s ) );

	g_profile.DeleteSection( _T("CommonFileDlg.NonResizableExcludes") );
	for( int i = 0; i != m_cfoNonResizableExcludes.size(); ++i )
	{
		CString key; key.Format( _T("%d"), i );
		g_profile.SetString( _T("CommonFileDlg.NonResizableExcludes"), key, 
			m_cfoNonResizableExcludes[ i ] );
	}

	//--- common folder dialog

	g_profile.SetInt( _T("CommonFolderDlg"), _T("EnableHook"), IsDlgButtonChecked( IDC_CHK_CFD_ENABLE ) );
	GetDlgItemText( IDC_ED_CFD_MINWIDTH, s );
	g_profile.SetInt( _T("CommonFolderDlg"), _T("MinWidth"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_CFD_MINHEIGHT, s );
	g_profile.SetInt( _T("CommonFolderDlg"), _T("MinHeight"), _ttoi( s ) );
	g_profile.SetInt( _T("CommonFolderDlg"), _T("Center"), m_cbCFD_pos.GetCurSel() );

	//--- MSO file dialog

	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("EnableHook"), IsDlgButtonChecked( IDC_CHK_MSO_ENABLE ) );
	GetDlgItemText( IDC_ED_MSO_MINWIDTH, s );
	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("MinWidth"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_MSO_MINHEIGHT, s );
	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("MinHeight"), _ttoi( s ) );
	g_profile.SetInt( _T("MSOfficeFileDlg"), _T("Center"), m_cbMSO_pos.GetCurSel() );

	//--- common "Open With" dialog

	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("EnableHook"), IsDlgButtonChecked( IDC_CHK_COW_ENABLE ) );
	GetDlgItemText( IDC_ED_COW_MINWIDTH, s );
	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("MinWidth"), _ttoi( s ) );
	GetDlgItemText( IDC_ED_COW_MINHEIGHT, s );
	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("MinHeight"), _ttoi( s ) );
	g_profile.SetInt( _T("CommonOpenWithDlg"), _T("Center"), m_cbCOW_pos.GetCurSel() );

	CDialog::OnOK();
}

//-----------------------------------------------------------------------------------------------

void CFFConfigDlg::OnBtnNonResizableExcl()
{
	CStringListDlg dlg( this );
	CString s; 
	s.LoadString( IDS_EDIT_EXCLUDES );
	dlg.SetTitle( s );
	s.LoadString( IDS_RESIZE_NONRESIZABLE_EXCL );
	dlg.SetDescr( s );
	dlg.SetStrings( m_cfoNonResizableExcludes );

	if( dlg.DoModal() == IDOK )
	{
		m_cfoNonResizableExcludes.clear();
		vector<CString> list;
		dlg.GetStrings( &list );
		for( int i = 0; i != list.size(); ++i )
			if( ! list[ i ].IsEmpty() )
				m_cfoNonResizableExcludes.push_back( list[ i ] );		
	}
}
