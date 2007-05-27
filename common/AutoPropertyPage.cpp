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
#include "AutoPropertyPage.h"

//-----------------------------------------------------------------------------------------------

CAutoPropertyPage::CAutoPropertyPage( UINT resId )
	: CPropertyPage( resId )
{}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAutoPropertyPage, CPropertyPage)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CAutoPropertyPage::OnNotify( WPARAM wp, LPARAM lp, LRESULT* pRes )
{
	NMHDR* pnm = reinterpret_cast<NMHDR*>( lp );
	return CPropertyPage::OnNotify( wp, lp, pRes );
}

//-----------------------------------------------------------------------------------------------

BOOL CAutoPropertyPage::OnCommand( WPARAM wp, LPARAM lp )
{
	UINT code = ( wp >> 16 ) & 0xFFFF;
	CWnd* pWnd = CWnd::FromHandle( (HWND) lp );
	DWORD style = pWnd->GetStyle();

	if( code == BN_CLICKED )
	{
		if( ! ( style & BS_PUSHBUTTON ) )
			SetModified();
	}
	else if( code == CBN_SELCHANGE || code == EN_CHANGE  )
		SetModified();

	return CPropertyPage::OnCommand( wp, lp );
}
