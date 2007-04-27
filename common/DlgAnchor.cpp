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
#include "DlgAnchor.h"

#include <utility>

//-------------------------------------------------------------------------------------------------

CDlgAnchor::CDlgAnchor() 
{
    m_hWnd = NULL;
	memset(&m_rect, 0, sizeof(m_rect));
	m_controlsCount = 0;
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Init(HWND hDlgWindow) 
{
	ASSERT( ::IsWindow( hDlgWindow ) );

	m_controls.clear();
	m_controlsCount = 0;

    if (IsWindow(hDlgWindow)) 
	{
		m_hWnd = hDlgWindow;
		if (GetClientRect(m_hWnd, &m_rect)) 
		{
			return TRUE;
		}
    }
    return FALSE;
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Init(const CWnd &wnd)
{
	ASSERT( wnd.GetSafeHwnd() );

    return Init(wnd.m_hWnd);
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Init(const CWnd *pWnd)
{
    ASSERT( pWnd->GetSafeHwnd() );
    
	return Init(pWnd->m_hWnd);
}
//-----------------------------------------------------------------

BOOL CDlgAnchor::Add(HWND hWnd, UINT uFlag) 
{
	ASSERT( ::IsWindow( hWnd ) );

    // add new item
    RECT rect;
    if (GetWindowRect(hWnd, &rect)) 
	{
		ScreenToClient(m_hWnd, &((LPPOINT)&rect)[0]);
		ScreenToClient(m_hWnd, &((LPPOINT)&rect)[1]);
		
		//associate a AnchorItem with the HWND
		std::pair<AnchorItemMap::iterator, bool> res =
			m_controls.insert( AnchorItemMap::value_type( 
		                          hWnd, AnchorItem(uFlag, &rect) ));
		
		//increment item count if insertion successful
		if (res.second)
			m_controlsCount++;  

		return TRUE;
	}

    return FALSE;
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Add(UINT uID, UINT uFlag) 
{
    return Add(GetDlgItem(m_hWnd, uID), uFlag);
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Add(const CWnd &wnd, UINT uFlag)
{
    return Add(wnd.m_hWnd, uFlag);
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Remove(HWND hWnd) 
{
	ASSERT( ::IsWindow( hWnd ) );

	AnchorItemMap::iterator it = m_controls.find( hWnd );
	if (it != m_controls.end())
	{
		m_controls.erase( it );
		m_controlsCount--;
		return TRUE;		
	}
	return FALSE;
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Remove(UINT uID) 
{
    return Remove(GetDlgItem(m_hWnd, uID));
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Remove(const CWnd &wnd) 
{
    return Remove(wnd.m_hWnd);
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Update(HWND hWnd) 
{
	ASSERT( ::IsWindow( hWnd ) );

	AnchorItemMap::iterator it = m_controls.find( hWnd );
	if (it != m_controls.end())
	{
		RECT rc;
        if (GetWindowRect(hWnd, &rc)) 
		{
			ScreenToClient(m_hWnd, &((LPPOINT)&rc)[0]);
			ScreenToClient(m_hWnd, &((LPPOINT)&rc)[1]);
			it->second.m_rect = rc;
			return TRUE;
        }
	}
	return FALSE;
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Update(UINT uID) 
{
    return Update(GetDlgItem(m_hWnd, uID));
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::Update(const CWnd &wnd)
{
    return Update(wnd.m_hWnd);
}

//-----------------------------------------------------------------

void CDlgAnchor::UpdateAll() 
{
	AnchorItemMap::iterator it;
	for (it = m_controls.begin(); it != m_controls.end(); it++)
	{
		RECT rc;
        if (GetWindowRect(it->first, &rc)) 
		{
			ScreenToClient(m_hWnd, &((LPPOINT)&rc)[0]);
			ScreenToClient(m_hWnd, &((LPPOINT)&rc)[1]);
			it->second.m_rect = rc;
        }
	}
}

//-----------------------------------------------------------------

void CDlgAnchor::RemoveAll() 
{
	m_controls.clear();
	m_controlsCount = 0;
}

//-----------------------------------------------------------------

BOOL CDlgAnchor::OnSize(BOOL bRepaint) 
{
	if (m_controlsCount == 0)
		return TRUE;

    RECT rect;
    if (! GetClientRect(m_hWnd, &rect)) 
		return FALSE;

	HDWP hdwp =	::BeginDeferWindowPos( m_controlsCount );
	if (hdwp == NULL)
		return FALSE;

	AnchorItemMap::iterator it;
	for (it = m_controls.begin(); it != m_controls.end(); it++)
	{
		AnchorItem *pItem = &it->second;

		// horizontal
		if ((pItem->m_uFlag & ANCHOR_LEFT) && (pItem->m_uFlag & ANCHOR_RIGHT)) 
		{
			pItem->m_rect.right += rect.right - m_rect.right;
		}
		else if (pItem->m_uFlag & ANCHOR_LEFT) 
		{
			// left is default
		}
		else if (pItem->m_uFlag & ANCHOR_RIGHT) 
		{
			pItem->m_rect.right += rect.right - m_rect.right;
			pItem->m_rect.left += rect.right - m_rect.right;
		}
		else 
		{
			// relative move
			LONG sx = ((rect.right-rect.left)/2)-((m_rect.right-m_rect.left)/2);
			pItem->m_rect.right += sx;
			pItem->m_rect.left += sx;
		}

		// vertical
		if ((pItem->m_uFlag & ANCHOR_TOP) && (pItem->m_uFlag & ANCHOR_BOTTOM)) 
		{
			pItem->m_rect.bottom += rect.bottom - m_rect.bottom;
		}
		else if (pItem->m_uFlag & ANCHOR_TOP) 
		{
			// top is default
		}
		else if (pItem->m_uFlag & ANCHOR_BOTTOM) 
		{
			pItem->m_rect.bottom += rect.bottom - m_rect.bottom;
			pItem->m_rect.top += rect.bottom - m_rect.bottom;
		}
		else 
		{
			// relative move
			LONG sy = ((rect.bottom-rect.top)/2) - ((m_rect.bottom-m_rect.top)/2);
			pItem->m_rect.bottom += sy;
			pItem->m_rect.top += sy;
		}
		if (pItem->m_uFlag & DOCK_TOP) pItem->m_rect.top = rect.top;
		if (pItem->m_uFlag & DOCK_LEFT) pItem->m_rect.left = rect.left;
		if (pItem->m_uFlag & DOCK_RIGHT) pItem->m_rect.right = rect.right;
		if (pItem->m_uFlag & DOCK_BOTTOM) pItem->m_rect.bottom = rect.bottom;

		//prepare repositioning / resizing of the control
		::DeferWindowPos( hdwp, it->first, NULL, 
			         pItem->m_rect.left, pItem->m_rect.top, 
					 pItem->m_rect.right - pItem->m_rect.left, 
					 pItem->m_rect.bottom - pItem->m_rect.top, 
					 (bRepaint ? 0 : SWP_NOREDRAW ) | 
					 (pItem->m_uFlag & REDRAW_NOCOPYBITS ? SWP_NOCOPYBITS : 0) |
   					 SWP_NOZORDER | SWP_NOACTIVATE);
	} //for

	m_rect = rect;

	//reposition / resize all controls at once for better performance
	//   and prevention of redraw problems
	BOOL res = ::EndDeferWindowPos( hdwp );

	return res;
}