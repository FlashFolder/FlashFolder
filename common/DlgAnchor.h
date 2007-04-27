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
#pragma once

#include <map>

//-------------------------------------------------------------------------------------------------

class CDlgAnchor 
{
public:
    enum AlignFlagsEnum { 
        ANCHOR_TOP          = 0x0001,
        ANCHOR_LEFT         = 0x0002,
        ANCHOR_RIGHT        = 0x0004,
        ANCHOR_BOTTOM       = 0x0008,
        ANCHOR_TOPLEFT      = 0x0003,
        ANCHOR_TOPRIGHT     = 0x0005,
        ANCHOR_BOTTOMLEFT   = 0x000a,
        ANCHOR_BOTTOMRIGHT  = 0x000c,
        ANCHOR_ALL          = 0x000f,
        DOCK_TOP            = 0x0100,
        DOCK_LEFT           = 0x0200,
        DOCK_RIGHT          = 0x0400,
        DOCK_BOTTOM         = 0x0800,
        DOCK_FILL           = 0x0f00,
        
        REDRAW_NOCOPYBITS   = 0x010000 };   
	          //use this flag only if redraw problems occur

	// constructor
	CDlgAnchor();

	// initialize
	BOOL Init(HWND hDlgWindow);
	BOOL Init(const CWnd &wnd);
	BOOL Init(const CWnd *pWnd);

	BOOL Add(HWND hWnd, UINT uFlag);
	BOOL Add(UINT uID, UINT uFlag);
	BOOL Add(const CWnd &wnd, UINT uFlag);

	BOOL Remove(UINT uID);
	BOOL Remove(HWND hWnd);
	BOOL Remove(const CWnd &wnd);

	BOOL Update(UINT uID);
	BOOL Update(HWND hWnd);
	BOOL Update(const CWnd &wnd);

	void UpdateAll();
	void RemoveAll();

	BOOL OnSize(BOOL bRepaint=TRUE);

private:
	struct AnchorItem 
	{
		UINT m_uFlag;        // anchor flags
		RECT m_rect;                // window rect
        AnchorItem( UINT uFlag, LPRECT lpRect ) 
            : m_uFlag( uFlag ), m_rect( *lpRect ) {}
	};

	typedef std::map<HWND, AnchorItem> AnchorItemMap;

private:
	HWND m_hWnd;
	RECT m_rect;
	AnchorItemMap m_controls;
	int m_controlsCount;
};
