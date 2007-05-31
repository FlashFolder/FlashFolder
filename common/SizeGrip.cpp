/*
Copyright (c) 2005 zett42 (zett42@users.sourceforge.net)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software, to deal in the Software without restriction,
including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the source code.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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