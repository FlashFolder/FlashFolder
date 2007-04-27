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
#include "SizeGrip.h"

//---------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CSizeGrip, CScrollBar)

//---------------------------------------------------------------------------------------

BOOL CSizeGrip::Create( CWnd* pParent, DWORD dwStyle, UINT nID )
{
	RECT rcClient; pParent->GetClientRect( &rcClient );

	if( CScrollBar::Create( dwStyle | WS_CHILD | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN,
		rcClient, pParent, nID ) )
	{
		m_isVisible = ( dwStyle & WS_VISIBLE ) != 0;
		UpdateVisible();
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSizeGrip, CScrollBar)
	ON_WM_NCHITTEST()
	ON_WM_MOVE()
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

//---------------------------------------------------------------------------------------

UINT CSizeGrip::OnNcHitTest( CPoint pt )
{
	// this makes the size grip able to resize its parent window
	return HTBOTTOMRIGHT;
}

//---------------------------------------------------------------------------------------

void CSizeGrip::OnMove( int x, int y )
{
	CScrollBar::OnMove( x, y );
	UpdateVisible();
}

//---------------------------------------------------------------------------------------

// check if the size grip should be visible according to the state of its parent window

bool CSizeGrip::ShouldBeVisible()
{
	DWORD style = GetParent()->GetStyle();
	return ( style & WS_SIZEBOX ) != 0 && ( style & WS_MAXIMIZE ) == 0;
}

//---------------------------------------------------------------------------------------

void CSizeGrip::UpdateVisible()
{
	m_bInternalShow = true;

	if( ShouldBeVisible() )
		// show only if ShowWindow( SW_SHOW ) has been called externally before
		if( ! IsWindowVisible() && m_isVisible ) ShowWindow( SW_SHOW );
		else;
	else
		if( IsWindowVisible() ) ShowWindow( SW_HIDE );

	m_bInternalShow = false;
}

//---------------------------------------------------------------------------------------

void CSizeGrip::OnWindowPosChanging( WINDOWPOS* pw )
{
	CScrollBar::OnWindowPosChanging( pw );

	if( m_bInternalShow ) return;

	// we get here if ShowWindow() has been called externally

	if( pw->flags & SWP_SHOWWINDOW )
	{
		m_isVisible = true;
		// check whether the size grip can be visible right now
		if( ! ShouldBeVisible() )
			pw->flags = pw->flags & ~SWP_SHOWWINDOW;
	}
	else if( pw->flags & SWP_HIDEWINDOW )
		m_isVisible = false;
}
