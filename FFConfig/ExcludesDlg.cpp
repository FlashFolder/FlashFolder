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
#include "ExcludesDlg.h"

using namespace std;

//-----------------------------------------------------------------------------------------------

CExcludesDlg::CExcludesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExcludesDlg::IDD, pParent)
{}

//-----------------------------------------------------------------------------------------------

void CExcludesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ED_EXCLUDES, m_edList);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CExcludesDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBnClickedBtnAdd)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CExcludesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if( ! m_title.IsEmpty() )
		SetWindowText( m_title );
	SetDlgItemText( IDC_ST_DESCR, m_descr );

	CString s;
	for( int i = 0; i != m_list.size(); ++i )
	{
		if( ! s.IsEmpty() )
			s += _T("\r\n");
		s += m_list[ i ];
	}
	if( ! s.IsEmpty() )
		s += _T("\r\n");
		
	m_edList.SetWindowText( s );

	m_edList.SetFocus();
	m_edList.SetSel( m_edList.GetWindowTextLength(), m_edList.GetWindowTextLength() );

	return FALSE;
}

//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------

namespace {
void GetProgramsList( vector<CString>* pList, CString s )
{
	s.Replace( _T("\r\n"), _T("\n") );
	vector<CString> tempList;
	SplitString( back_inserter( tempList ), s.GetString(), _T('\n') );
	pList->clear();
	for( int i = 0; i != tempList.size(); ++i )
	{
		tempList[ i ].Trim();
		if( ! tempList[ i ].IsEmpty() )
			pList->push_back( tempList[ i ] );
	}
}
} //unnamed namespace

//-----------------------------------------------------------------------------------------------

void CExcludesDlg::OnOK()
{
	CString s; m_edList.GetWindowText( s );
	GetProgramsList( &m_list, s );
	CDialog::OnOK();
}

//-----------------------------------------------------------------------------------------------

void CExcludesDlg::OnBnClickedBtnAdd()
{
	CFileDialog dlg( TRUE, _T("exe"), _T(""), OFN_HIDEREADONLY | OFN_ENABLESIZING, 
		_T("Applications (*.exe)|*.exe|All files (*.*)|*.*||"), this );
	if( dlg.DoModal() == IDOK )
	{
		CString fileName = dlg.GetFileName();
		CString s; m_edList.GetWindowText( s );

		// Check if program is already in list
		vector<CString> list;
		GetProgramsList( &list, s );
		for( int i = 0; i != list.size(); ++i )
			if( list[ i ].CompareNoCase( fileName ) == 0 )
				return;

		// Append filename to edit control
		CString sNew = fileName + _T("\r\n");
		m_edList.SetSel( m_edList.GetWindowTextLength(), m_edList.GetWindowTextLength() );
		if( ! s.IsEmpty() )
			if( s[ s.GetLength() - 1 ] != '\n' )
				sNew = _T("\r\n") + sNew;
		m_edList.ReplaceSel( sNew );
	}
}
