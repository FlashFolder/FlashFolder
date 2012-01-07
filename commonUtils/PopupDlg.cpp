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
 */
#include "stdafx.h"
#include "PopupDlg.h"

// "unique" timer ID
const UINT POPUPDLG_TIMER_ID = 0xF39AE7D4;

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPopupDlg, CResizableDlg)
	ON_WM_CAPTURECHANGED()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	ON_MESSAGE( WM_ENTERSIZEMOVE, OnEnterSizeMove )
	ON_MESSAGE( WM_EXITSIZEMOVE, OnExitSizeMove )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPopupDlg::OnInitDialog()
{
	CResizableDlg::OnInitDialog();

	m_isCancelling = false;
	m_isSizing = false;

	SetTimer( POPUPDLG_TIMER_ID, 100, NULL );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CPopupDlg::SetPopupPos()
{
	CRect rc( m_rcButton.left, m_rcButton.bottom, 
		m_rcButton.left + m_cx, m_rcButton.bottom + m_cy );

	// Clip window pos to monitor work area.
	MONITORINFO mi = { sizeof(mi) };
	HMONITOR hMon = ::MonitorFromWindow( GetParent()->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST );
	::GetMonitorInfo( hMon, &mi );
	if( rc.left < mi.rcWork.left )
		rc.OffsetRect( mi.rcWork.left - rc.left, 0 );
	else if( rc.right > mi.rcWork.right )
		rc.OffsetRect( mi.rcWork.right - rc.right, 0 );
	if( rc.top < mi.rcWork.top )
		rc.OffsetRect( 0, mi.rcWork.top - rc.top );
	else if( rc.bottom > mi.rcWork.bottom )
	{
		int h = rc.Height();
		rc.top = m_rcButton.top - h;
		rc.bottom = m_rcButton.top;
	}

	SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER ); 
}

//-----------------------------------------------------------------------------------------------

void CPopupDlg::OnCaptureChanged( CWnd* pWnd )
{
	CResizableDlg::OnCaptureChanged( pWnd );
	CheckDismiss();
}

//-----------------------------------------------------------------------------------------------

void CPopupDlg::OnLButtonDown( UINT flags, CPoint pt )
{
	CResizableDlg::OnLButtonDown( flags, pt );
	CheckDismiss();
}

//-----------------------------------------------------------------------------------------------

void CPopupDlg::OnRButtonDown( UINT flags, CPoint pt )
{
	CResizableDlg::OnRButtonDown( flags, pt );
	CheckDismiss();
}

//-----------------------------------------------------------------------------------------------

void CPopupDlg::CheckDismiss()
{
	if( ! m_isSizing )
	{
		CPoint pt; ::GetCursorPos( &pt );
		CRect rc;
		GetWindowRect( rc );
		if( ! rc.PtInRect( pt ) )
		{
			m_isCancelling = true;
			KillTimer( POPUPDLG_TIMER_ID );
			EndDialog( IDCANCEL );
		}
	}
}

//-----------------------------------------------------------------------------------------------

void CPopupDlg::OnTimer( UINT id )
{
	if( id != POPUPDLG_TIMER_ID || m_isCancelling || m_isSizing )
		return;

	CPoint pt; ::GetCursorPos( &pt );
	CRect rc;
	GetWindowRect( rc );
	if( rc.PtInRect( pt ) )
	{
		if( GetCapture() == this )
			ReleaseCapture();
	}
	else
	{
		if( GetCapture() != this )
			SetCapture();
	}
}

//-----------------------------------------------------------------------------------------------

LRESULT CPopupDlg::OnEnterSizeMove( WPARAM, LPARAM )
{
	m_isSizing = true;
	return 0;
}

//-----------------------------------------------------------------------------------------------

LRESULT CPopupDlg::OnExitSizeMove( WPARAM, LPARAM )
{
	m_isSizing = false;
	return 0;
}
