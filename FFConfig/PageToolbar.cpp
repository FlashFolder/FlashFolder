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
#include "PageToolbar.h"

using namespace std;

//-----------------------------------------------------------------------------------------------

const CString PROFILE_GROUP = _T("Toolbar");

//-----------------------------------------------------------------------------------------------

CPageToolbar::CPageToolbar() :
	base(CPageToolbar::IDD)
{}

//-----------------------------------------------------------------------------------------------

void CPageToolbar::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageToolbar, CPageToolbar::base)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPageToolbar::OnInitDialog()
{
	base::OnInitDialog();

	//--- init controls

	GetDlgItem( IDC_SP_OFFSET_X )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( 500, -500 ) );
	GetDlgItem( IDC_SP_OFFSET_Y )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( 500, -500 ) );
	GetDlgItem( IDC_SP_OFFSET_W )->SendMessage( UDM_SETRANGE, NULL, MAKELONG( 500, -500 ) );

	//--- get profile data

	ReadProfile();

	GotoDlgCtrl( GetDlgItem( IDC_ED_OFFSET_X ) );
	return FALSE;
}

//-----------------------------------------------------------------------------------------------

void CPageToolbar::ReadProfile()
{
	const Profile& profile = CApp::GetReadProfile();

	CString offsetX, offsetY, offsetWidth;
	offsetX.Format( _T("%d"), profile.GetInt( PROFILE_GROUP, _T("OffsetX") ) );
	offsetY.Format( _T("%d"), profile.GetInt( PROFILE_GROUP, _T("OffsetY") ) );
	offsetWidth.Format( _T("%d"), profile.GetInt( PROFILE_GROUP, _T("OffsetWidth") ) );
	SetDlgItemText( IDC_ED_OFFSET_X, offsetX );
	SetDlgItemText( IDC_ED_OFFSET_Y, offsetY );
	SetDlgItemText( IDC_ED_OFFSET_W, offsetWidth );
}

//-----------------------------------------------------------------------------------------------

BOOL CPageToolbar::OnApply()
{
	Profile& profile = CApp::GetWriteProfile();

	// Multi-User option is currently read-only since it would require more work like
	// checking for admin rights, elevation on Vista, etc.

	CString offsetX, offsetY, offsetWidth;
	GetDlgItemText( IDC_ED_OFFSET_X, offsetX );
	GetDlgItemText( IDC_ED_OFFSET_Y, offsetY );
	GetDlgItemText( IDC_ED_OFFSET_W, offsetWidth );
	profile.SetInt( PROFILE_GROUP, _T("OffsetX"), _ttoi( offsetX ) );
	profile.SetInt( PROFILE_GROUP, _T("OffsetY"), _ttoi( offsetY ) );
	profile.SetInt( PROFILE_GROUP, _T("OffsetWidth"), _ttoi( offsetWidth ) );
	
	return base::OnApply();
}