/* This file is part of FlashFolder.
 * Copyright (C) 2007-2012 zett42.de ( zett42 at users.sourceforge.net )
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
#include "AutoPropertyPage.h"

#pragma warning(disable:4244) //convert to smaller type

//-----------------------------------------------------------------------------------------------

CAutoPropertyPage::CAutoPropertyPage( UINT resId )
	: CPropertyPage( resId ),
	m_isInitialized( false )
{}

//-----------------------------------------------------------------------------------------------

const UINT WM_APP_AFTERINITDIALOG = WM_APP + 1;

BEGIN_MESSAGE_MAP(CAutoPropertyPage, CPropertyPage)
	ON_MESSAGE( WM_APP_AFTERINITDIALOG, OnAfterInitDialog )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CAutoPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	PostMessage( WM_APP_AFTERINITDIALOG );
	return TRUE;
}

//-----------------------------------------------------------------------------------------------

LRESULT CAutoPropertyPage::OnAfterInitDialog( WPARAM, LPARAM )
{
	// this will be called after the _derived_ class has returned from OnInitDialog()
	m_isInitialized = true;
	return 0;	
}

//-----------------------------------------------------------------------------------------------

BOOL CAutoPropertyPage::OnCommand( WPARAM wp, LPARAM lp )
{
	// avoid SetModified() during dialog initialization
	if( ! m_isInitialized )
		return CPropertyPage::OnCommand( wp, lp );

	//--- set the modified flag if a child control has changed

	UINT code = ( wp >> 16 ) & 0xFFFF;
	CWnd* pWnd = CWnd::FromHandle( (HWND) lp );
	DWORD style = pWnd->GetStyle();

	if( code == BN_CLICKED )
	{
		if( ! ( style & BS_PUSHBUTTON ) )
			SetModified();
	}
	else if( code == CBN_SELCHANGE || code == EN_CHANGE  )
	{
		SetModified();
	}

	return CPropertyPage::OnCommand( wp, lp );
}

