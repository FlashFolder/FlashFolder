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
#include "TestGui.h"
#include "TestGuiDlg.h"
#include ".\testguidlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------------------------

CTestGuiDlg::CTestGuiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestGuiDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------------------------

void CTestGuiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CTestGuiDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_FILEOPEN, OnBnClickedBtnFileopen)
	ON_BN_CLICKED(IDC_BTN_BROWSEDIR, OnBnClickedBtnBrowsedir)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CTestGuiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Groﬂes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	CheckDlgButton( IDC_CHK_FO_EXPLORER, 1 );
	CheckDlgButton( IDC_CHK_FO_SIZING, 1 );
	CheckDlgButton( IDC_CHK_FO_MFC, 1 );
	CheckDlgButton( IDC_CHK_FO_CUST, 0 );

	return TRUE; 
}

//-----------------------------------------------------------------------------------------------

void CTestGuiDlg::OnBnClickedBtnFileopen()
{
	DWORD flags = 0;
	if( IsDlgButtonChecked( IDC_CHK_FO_EXPLORER ) )
		flags |= OFN_EXPLORER;
	if( IsDlgButtonChecked( IDC_CHK_FO_SIZING ) )
		flags |= OFN_ENABLESIZING;

	CString filter = _T("Text files (*.txt)|*.txt|All files (*.*)|*.*||");

	if( IsDlgButtonChecked( IDC_CHK_FO_CUST ) )
	{
	}
	if( IsDlgButtonChecked( IDC_CHK_FO_MFC ) )
	{
		CFileDialog dlg( TRUE, _T("txt"), NULL, flags | OFN_HIDEREADONLY, filter, this );
		dlg.DoModal();
	}
	else
	{
		OPENFILENAME ofn = { sizeof(ofn) };
		ofn.hwndOwner = GetSafeHwnd();
		ofn.hInstance = AfxGetInstanceHandle();
		filter.Replace( '|', '\0' );
		ofn.lpstrFilter = filter;
		TCHAR fileName[ MAX_PATH + 1 ] = _T("");
		ofn.lpstrFile = fileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = flags;
		::GetOpenFileName( &ofn );
	}
}

//-----------------------------------------------------------------------------------------------

void CTestGuiDlg::OnBnClickedBtnBrowsedir()
{
	CFolderDlg dlg( _T("Select directory"), NULL, this );
	dlg.DoModal();
}
