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
#include "ResizableDlg.h"

using namespace std;

#pragma warning(disable:4267) // size_t to int

//---------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CResizeGrip, CScrollBar)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

void CResizableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CResizableDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CResizableDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetClientRect( m_rcClient ); 

	m_sizeGrip.Create( WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN,
		m_rcClient, this, -1 );	
	Anchor( m_sizeGrip, ANCHOR_BOTTOMRIGHT );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CResizableDlg::Anchor( HWND hWnd, UINT flags ) 
{
	ASSERT( ::IsWindow( hWnd ) );

    RECT rc;
	if( ::GetWindowRect( hWnd, &rc ) ) 
	{
		ScreenToClient( &rc );
		m_controls.insert( make_pair( hWnd, AnchorItem( flags, rc ) ) );
	}
}

//-----------------------------------------------------------------

void CResizableDlg::AnchorUpdate( HWND hWnd ) 
{
	ASSERT( ::IsWindow( hWnd ) );

	AnchorItemMap::iterator it = m_controls.find( hWnd );
	if( it != m_controls.end() )
	{
		RECT rc;
		if( ::GetWindowRect( hWnd, &rc ) ) 
		{
			ScreenToClient( &rc );
			it->second.pos = rc;
        }
	}
}

//-----------------------------------------------------------------

void CResizableDlg::AnchorUpdateAll() 
{
	AnchorItemMap::iterator it;
	for( it = m_controls.begin(); it != m_controls.end(); ++it )
	{
		RECT rc;
		if( ::GetWindowRect( it->first, &rc ) ) 
		{
			ScreenToClient( &rc );
			it->second.pos = rc;
        }
	}
}

//-----------------------------------------------------------------

void CResizableDlg::OnSize( UINT type, int cx, int cy ) 
{
	CDialog::OnSize( type, cx, cy );

	if( type != SIZE_MAXIMIZED && type != SIZE_RESTORED )
		return;

	if( m_sizeGrip.GetSafeHwnd() )
	{
		DWORD style = GetStyle();
		if( ( style & WS_SIZEBOX ) && type != SIZE_MAXIMIZED )
			m_sizeGrip.ShowWindow( SW_SHOW );
		else
			m_sizeGrip.ShowWindow( SW_HIDE );
	}

	if( m_controls.empty() )
		return;

    RECT rect;
    GetClientRect( &rect );

	// Prepare for reposition of anchored controls
	HDWP hdwp =	::BeginDeferWindowPos( m_controls.size() );
	if( ! hdwp )
		return;

	AnchorItemMap::iterator it;
	for( it = m_controls.begin(); it != m_controls.end(); ++it )
	{
		AnchorItem *pItem = &it->second;

		// horizontal
		if ((pItem->flags & ANCHOR_LEFT) && (pItem->flags & ANCHOR_RIGHT)) 
		{
			pItem->pos.right += rect.right - m_rcClient.right;
		}
		else if (pItem->flags & ANCHOR_LEFT) 
		{
			// left is default
		}
		else if (pItem->flags & ANCHOR_RIGHT) 
		{
			pItem->pos.right += rect.right - m_rcClient.right;
			pItem->pos.left += rect.right - m_rcClient.right;
		}
		else 
		{
			// relative move
			LONG sx = ((rect.right-rect.left)/2)-((m_rcClient.right-m_rcClient.left)/2);
			pItem->pos.right += sx;
			pItem->pos.left += sx;
		}

		// vertical
		if ((pItem->flags & ANCHOR_TOP) && (pItem->flags & ANCHOR_BOTTOM)) 
		{
			pItem->pos.bottom += rect.bottom - m_rcClient.bottom;
		}
		else if (pItem->flags & ANCHOR_TOP) 
		{
			// top is default
		}
		else if (pItem->flags & ANCHOR_BOTTOM) 
		{
			pItem->pos.bottom += rect.bottom - m_rcClient.bottom;
			pItem->pos.top += rect.bottom - m_rcClient.bottom;
		}
		else 
		{
			// relative move
			LONG sy = ((rect.bottom-rect.top)/2) - ((m_rcClient.bottom-m_rcClient.top)/2);
			pItem->pos.bottom += sy;
			pItem->pos.top += sy;
		}
		if (pItem->flags & DOCK_TOP) pItem->pos.top = rect.top;
		if (pItem->flags & DOCK_LEFT) pItem->pos.left = rect.left;
		if (pItem->flags & DOCK_RIGHT) pItem->pos.right = rect.right;
		if (pItem->flags & DOCK_BOTTOM) pItem->pos.bottom = rect.bottom;

		// store position information in hdwp
		::DeferWindowPos( hdwp, it->first, NULL, 
			         pItem->pos.left, pItem->pos.top, 
					 pItem->pos.right - pItem->pos.left, 
					 pItem->pos.bottom - pItem->pos.top, 			 
   					 SWP_NOZORDER | SWP_NOACTIVATE);
	} //for

	m_rcClient = rect;

	// Reposition all anchored childs at once
	::EndDeferWindowPos( hdwp );
}

//-----------------------------------------------------------------------------------------------

void CResizableDlg::OnGetMinMaxInfo( MINMAXINFO* pm )
{
	pm->ptMinTrackSize = m_ptMin;
}
