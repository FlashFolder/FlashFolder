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
#include "StringListDlg.h"

using namespace std;

//-----------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CStringListDlg, CDialog)
CStringListDlg::CStringListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStringListDlg::IDD, pParent)
{}

//-----------------------------------------------------------------------------------------------

void CStringListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ED_EXCLUDES, m_edList);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CStringListDlg, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CStringListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText( m_title );
	SetDlgItemText( IDC_ST_DESCR, m_descr );

	CString s;
	for( int i = 0; i != m_list.size(); ++i )
	{
		if( ! s.IsEmpty() )
			s += _T("\r\n");
		s += m_list[ i ];
	}
		
	m_edList.SetWindowText( s );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CStringListDlg::OnOK()
{
	CString s; m_edList.GetWindowText( s );
	s.Replace( _T("\r\n"), _T("\n") );

	m_list.clear();
	SplitString( back_inserter( m_list ), s.GetString(), _T('\n') );

	CDialog::OnOK();
}

//-----------------------------------------------------------------------------------------------
