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
#include "StdAfx.h"
#include "DlgExpander.h"

#pragma warning(disable:4800) //convert BOOL to bool

//-----------------------------------------------------------------------------------------------

void CDlgExpander::GetExpandChilds()
{
	// SetParent() or parametrized ctor must have be called before
	ASSERT( m_pParent );

	m_controls.clear();

	for( CWnd* pWnd = m_pParent->GetWindow( GW_CHILD ); pWnd; 
	     pWnd = pWnd->GetNextWindow() )
	{
		// ignore the helper control used to get the expand rect from
		if( m_expandRectId != -1 &&
			pWnd->GetDlgCtrlID() == m_expandRectId )
			continue;

		CRect rc; pWnd->GetWindowRect( rc );
		m_pParent->ScreenToClient( rc );
		if( rc.left >= m_rcExpand.left &&
			rc.right <= m_rcExpand.right &&
			rc.top >= m_rcExpand.top &&
			rc.bottom <= m_rcExpand.bottom )
		{
			Wnd wnd;
			wnd.hwnd = pWnd->GetSafeHwnd();
			wnd.isEnabled = pWnd->IsWindowEnabled();
			m_controls.push_back( wnd );
		}
	}
}

//-----------------------------------------------------------------------------------------------

void CDlgExpander::SetExpanded( bool expand )
{
	// SetParent() or parametrized ctor must have be called before
	ASSERT( m_pParent );

	if( expand == m_isExpanded )
		return;
	m_isExpanded = expand;

	if( expand )
	{
		//----- expand the dialog

		//--- resize dialog

		CRect rc;
		m_pParent->GetWindowRect( rc );
		rc.bottom += m_rcExpand.Height();
		
		// The expanded dialog could cross the border of the monitor work area. 
		// If so, move the dialog up so that it is fully visible.
		MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
		HMONITOR hMon = ::MonitorFromWindow( *m_pParent, MONITOR_DEFAULTTONEAREST );
		::GetMonitorInfo( hMon, &monitorInfo );
		if( rc.bottom > monitorInfo.rcWork.bottom )
		{
			int diff = rc.bottom - monitorInfo.rcWork.bottom;
			rc.OffsetRect( 0, -diff );
		}		

		m_pParent->SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER );

		//--- show child windows inside expand rect

		for( int i = 0; i != m_controls.size(); ++i )
		{
			Wnd& wnd = m_controls[ i ];

			// ignore the helper control used to get the expand rect from
			if( m_expandRectId != -1 &&
				::GetDlgCtrlID( wnd.hwnd ) == m_expandRectId )
				continue;

			if( wnd.isEnabled || m_isInitialExpand )
				::EnableWindow( wnd.hwnd, TRUE ); 
			if( wnd.isVisible || m_isInitialExpand )
				::ShowWindow( wnd.hwnd, SW_SHOWNA );
		}

		m_isInitialExpand = false;
	}
	else
	{
		//----- collapse the dialog

		//--- hide child windows inside of collapse rect

		for( int i = 0; i != m_controls.size(); ++i )
		{
			Wnd& wnd = m_controls[ i ];
			wnd.isVisible = ::IsWindowVisible( wnd.hwnd );
			wnd.isEnabled = ::IsWindowEnabled( wnd.hwnd );
			::EnableWindow( wnd.hwnd, FALSE );
			::ShowWindow( wnd.hwnd, SW_HIDE );
		}

		//--- resize dialog

		CRect rc;
		m_pParent->GetWindowRect( rc );
		rc.bottom -= m_rcExpand.Height();
		m_pParent->SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER );
	}
}