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
#include "ExcludesDlg.h"

using namespace std;
using namespace TreePropSheet;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------------------------

CFFConfigDlg::CFFConfigDlg(CWnd* pParent /*=NULL*/)
	: CTreePropSheet( _T("FlashFolder Options"), pParent)
{
	SetTreeWidth( 125 );
	SetTreeViewMode( TRUE, TRUE, FALSE );
	SetEmptyPageText( _T("Please select a sub-page.") );

	AddPage( &m_pageGeneric );
	AddPage( &m_pageCommonFileDlg );
	AddPage( &m_pageCommonDirDlg );
	if( GetOsVersion() <= 0x0500 )
		AddPage( &m_pageCommonOpenWithDlg );
	AddPage( &m_pageMsoFileDlg );

	// See http://support.microsoft.com/default.aspx?scid=kb%3Ben-us%3BQ158552 for why 
	// the first-chance access-violation exception can savely be ignored. 
	// The Exception occurs in comctl32.dll during property sheet creation.
}

//-----------------------------------------------------------------------------------------------

void CFFConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CTreePropSheet::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFFConfigDlg, CTreePropSheet)
	ON_MESSAGE( PSM_CHANGED, OnPageChanged )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CFFConfigDlg::OnInitDialog()
{
	CTreePropSheet::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------------------------

LRESULT CFFConfigDlg::OnPageChanged( WPARAM wp, LPARAM lp )
{
	const MSG* pMsg = GetCurrentMessage();
	LRESULT res = DefWindowProc( pMsg->message, wp, lp );

	HWND hwndChanged = reinterpret_cast<HWND>( wp );

	/// notify all pages about the change
	for( int i = 0; i < GetPageCount(); ++i )
	{
		CPropertyPage* pPage = GetPage( i );
		if( pPage->GetSafeHwnd() )
			pPage->SendMessage( WM_APP_PAGE_CHANGED, wp, 0 );
	}
	return res;
}
