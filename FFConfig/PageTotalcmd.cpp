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
#include "PageTotalcmd.h"

using namespace std;

//-----------------------------------------------------------------------------------------------

const CString PROFILE_GROUP = _T("main");

//-----------------------------------------------------------------------------------------------

CPageTotalcmd::CPageTotalcmd()
	: base(CPageTotalcmd::IDD),
	m_bReadDefaults( false )
{}

//-----------------------------------------------------------------------------------------------

void CPageTotalcmd::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageTotalcmd, CPageTotalcmd::base)
	ON_BN_CLICKED(IDC_CHK_ENABLE, OnBnClickedChkEnable)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPageTotalcmd::OnInitDialog()
{
	base::OnInitDialog();

	//--- get profile data

	if( m_bReadDefaults )
		ReadProfile( g_profileDefaults );
	else
		ReadProfile( g_profile );
		
	tstring tcIniPath;
	if( ! GetTotalCmdLocation( NULL, &tcIniPath ) )
	{
		for( CWnd* pChild = GetWindow( GW_CHILD ); pChild; pChild = pChild->GetNextWindow() )
			pChild->EnableWindow( FALSE );
	}

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CPageTotalcmd::ReadProfile( const Profile& profile )
{
	if( ! GetSafeHwnd() )
	{
		m_bReadDefaults = true;
		return;
	}

	CString s;
	CheckDlgButton( IDC_CHK_ENABLE, profile.GetInt( PROFILE_GROUP, _T("UseTcFavorites") ) );

	// Set initial enabled state for child controls
	OnBnClickedChkEnable();
}

//-----------------------------------------------------------------------------------------------

BOOL CPageTotalcmd::OnApply()
{
	CString s;
	g_profile.SetInt( PROFILE_GROUP, _T("UseTcFavorites"), IsDlgButtonChecked( IDC_CHK_ENABLE ) );

	return base::OnApply();
}

//-----------------------------------------------------------------------------------------------

void CPageTotalcmd::OnBnClickedChkEnable()
{
}

