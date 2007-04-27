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

//---------------------------------------------------------------------------------------

class CSizeGrip : public CScrollBar
{
	DECLARE_DYNAMIC(CSizeGrip)

public:
	CSizeGrip() : m_isVisible(true), m_bInternalShow(false) {}
	virtual ~CSizeGrip() {}

	// use WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS for dwStyle
	BOOL Create( CWnd* pParent, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, UINT nID = -1 );
	
	// call in rare circumstances when the visibility state doesn't update
	// automatically
	void UpdateVisible();

protected:
	afx_msg UINT OnNcHitTest( CPoint pt );
	afx_msg void OnMove( int x, int y );
	afx_msg void OnWindowPosChanging( WINDOWPOS* pw );
	DECLARE_MESSAGE_MAP()

private:
	bool ShouldBeVisible();

	bool m_isVisible;
	bool m_bInternalShow;
};
