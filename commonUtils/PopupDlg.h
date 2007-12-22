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
 */
#pragma once
#include "afxcmn.h"

//-----------------------------------------------------------------------------------------------
/// Dialog that works just like a popup menu: if the user clicks outside the window, 
/// it is dismissed.

class CPopupDlg : public CResizableDlg
{
public:
	CPopupDlg() :
		m_cx( 0 ), m_cy( 0 ), m_rcButton( 0,0,0,0 ) {}
	explicit CPopupDlg( UINT idTempl, CWnd* pParent = NULL ) :
		CResizableDlg( idTempl, pParent ),
		m_cx( 0 ), m_cy( 0 ), m_rcButton( 0,0,0,0 ) {}

	/// Set dialog align rectangle, used by SetPopupPosition()
	void SetPopupButtonRect( const CRect& rcButton ) { m_rcButton = rcButton; }
	/// Set initial dialog size, used by SetPopupPosition()
	void SetPopupSize( int cx, int cy )         { m_cx = cx; m_cy = cy; }

	/// Calculate the dialog size and position and take monitor clipping into account.
	/// Should be called after the dialog window is created, unless you want to position 
	/// the dialog by yourself.
	void SetPopupPos();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnCaptureChanged( CWnd* pWnd );
	afx_msg void OnLButtonDown( UINT flags, CPoint pt );
	afx_msg void OnRButtonDown( UINT flags, CPoint pt );
	afx_msg void OnTimer( UINT id );
	afx_msg LRESULT OnEnterSizeMove( WPARAM, LPARAM );
	afx_msg LRESULT OnExitSizeMove( WPARAM, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	void CheckDismiss();

	CRect m_rcButton;
	int m_cx, m_cy;
	bool m_isCancelling;
	bool m_isSizing;
};
