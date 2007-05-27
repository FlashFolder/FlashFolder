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
#include "PageGeneric.h"

const CString PROFILE_GROUP = _T("Main");

//-----------------------------------------------------------------------------------------------

CPageGeneric::CPageGeneric()
	: base(CPageGeneric::IDD)
{}

//-----------------------------------------------------------------------------------------------

void CPageGeneric::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageGeneric, CPageGeneric::base)
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
	
	s.Format( _T("%d"), g_profile.GetInt( PROFILE_GROUP, _T("MaxGlobalHistoryEntries") ) );
	SetDlgItemText( IDC_ED_MAX_DIRHISTORY, s );

	return TRUE;
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