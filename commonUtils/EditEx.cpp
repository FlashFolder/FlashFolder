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
#include "EditEx.h"

//----------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CEditEx, CEdit)

//----------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CEditEx, CEdit)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

//----------------------------------------------------------------------------------------

void CEditEx::OnPaint()
{
	CString s; GetWindowText( s );
	if( ! s.IsEmpty() || ! IsWindowEnabled() || GetFocus() == this )
	{
		Default();
	}
	else
	{
		CPaintDC dc( this );
		CRect rc; GetClientRect( rc );
		dc.FillSolidRect( rc, ::GetSysColor( COLOR_WINDOW ) );
		COLORREF color = m_hintColor;
		if( m_hintColor & 0xFF000000 )
			color = ::GetSysColor( m_hintColor & 0xFFFFFF );
		dc.SetTextColor( color );
		dc.SetBkMode( TRANSPARENT );

		CFont font;
		LOGFONT lf;
		GetFont()->GetLogFont( &lf );
		if( m_hintFontStyle & FF_ITALIC )
            lf.lfItalic = true;
		if( m_hintFontStyle & FF_BOLD )
            lf.lfWeight = FW_BOLD;
        font.CreateFontIndirect( &lf );

		CFont* oldFont = dc.SelectObject( &font );

		DWORD margins = GetMargins();
		rc.left += margins & 0xFFFF;
		rc.right -= margins >> 16;

		dc.DrawText( m_hintText, rc, DT_SINGLELINE | DT_VCENTER | DT_EDITCONTROL );
		dc.SelectObject( oldFont );
	}
}

//----------------------------------------------------------------------------------------

void CEditEx::OnSetFocus( CWnd* pWnd )
{
	CEdit::OnSetFocus( pWnd );
	Invalidate();
}

//----------------------------------------------------------------------------------------

void CEditEx::OnKillFocus( CWnd* pWnd )
{
	CEdit::OnKillFocus( pWnd );	
	Invalidate();
}
