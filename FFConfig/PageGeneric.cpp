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
#include "PageGeneric.h"

using namespace std;

//-----------------------------------------------------------------------------------------------

const CString PROFILE_GROUP = _T("Main");

//-----------------------------------------------------------------------------------------------

CPageGeneric::CPageGeneric() :
	base(CPageGeneric::IDD),
	m_bReadDefaults( false )
{}

//-----------------------------------------------------------------------------------------------

void CPageGeneric::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageGeneric, CPageGeneric::base)
	ON_BN_CLICKED(IDC_BTN_RESET, OnBnClickedBtnReset)
	ON_MESSAGE( CFFConfigDlg::WM_APP_PAGE_CHANGED, OnPageChanged )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPageGeneric::OnInitDialog()
{
	base::OnInitDialog();

	//--- init controls

	GetDlgItem( IDC_SP_MAX_DIRHISTORY )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( 50, 0 ) );

	//--- get profile data

	CString s;
	s.LoadString( g_profile.IsShared() ? IDS_MU_SHARED : IDS_MU_INDIVIDUAL );
	SetDlgItemText( IDC_ST_MULTIUSER, s );

	if( m_bReadDefaults )
		ReadProfile( g_profileDefaults );
	else
		ReadProfile( g_profile );

	SendMessage( WM_NEXTDLGCTL, (WPARAM) GetDlgItem( IDC_ED_MAX_DIRHISTORY )->GetSafeHwnd(), TRUE ); 

	return FALSE;
}

//-----------------------------------------------------------------------------------------------

void CPageGeneric::ReadProfile( const Profile& profile )
{
	if( ! GetSafeHwnd() )
	{
		m_bReadDefaults = true;
		return;
	}

	CString s;
	s.Format( _T("%d"), profile.GetInt( PROFILE_GROUP, _T("MaxGlobalHistoryEntries") ) );
	SetDlgItemText( IDC_ED_MAX_DIRHISTORY, s );
}

//-----------------------------------------------------------------------------------------------

BOOL CPageGeneric::OnApply()
{
	// Multi-User option is currently read-only since it would require more work like
	// checking for admin rights, elevation on Vista, etc.

	CString s;
	GetDlgItemText( IDC_ED_MAX_DIRHISTORY, s );
	g_profile.SetInt( PROFILE_GROUP, _T("MaxGlobalHistoryEntries"), _ttoi( s ) );
	
	return base::OnApply();
}

//-----------------------------------------------------------------------------------------------

void CPageGeneric::OnBnClickedBtnReset()
{
	CPropertySheet* pSheet = static_cast<CPropertySheet*>( GetParent() );
	ASSERT( pSheet );
	if( ! pSheet )
		return;

	int pageCount = pSheet->GetPageCount();
	for( int i = 0; i < pageCount; ++i )
	{
		CAutoPropertyPage* pPage = static_cast<CAutoPropertyPage*>( pSheet->GetPage( i ) );
		ASSERT( pPage );
		if( pPage )
		{
			pPage->ReadProfile( g_profileDefaults );
			pPage->SetModified();
		}
	}
	GetDlgItem( IDC_BTN_RESET )->EnableWindow( FALSE );
}

//-----------------------------------------------------------------------------------------------

LRESULT CPageGeneric::OnPageChanged( WPARAM, LPARAM lp )
{
	GetDlgItem( IDC_BTN_RESET )->EnableWindow( TRUE );
	return 0;
}
