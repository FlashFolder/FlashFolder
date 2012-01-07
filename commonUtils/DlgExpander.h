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
#pragma once

/** \brief Class to assist in creating expandable dialogs.
 *
 * Purpose is to hide extended information in which the user might not
 * be interested in, or not at this time.\n
 * Either programmatically or upon click of a button, the extended info
 * can be shown.
 */
class CDlgExpander
{
public:
	explicit CDlgExpander( CWnd* pParent = NULL ) :
		m_pParent( pParent ),
		m_expandRectId( -1 ),
		m_isExpanded( true ),
		m_isInitialExpand( true ) 
	{}

    /// \brief Set the parent window which should become expandable
	void SetParent( CWnd* pParent ) { m_pParent = pParent; }

    /// \brief Set a rectangle which defines the expandable area. Child controls in this
    /// area will be hidden/show if the parent window is collapsed/expanded. The height
    /// of the parent window will be adjusted according to the height of this rectangle.
	void SetExpandRect( const CRect& rc )
	{
		m_rcExpand = rc;
		GetExpandChilds();
	}
	/// \brief Variant of SetExpandRect which allows a control (e.g. an invisible Static) to
	/// be used for defining the expand rectangle.
	void SetExpandRect( UINT idCtrl )
	{
		m_expandRectId = idCtrl;
		CWnd* pWnd = m_pParent->GetDlgItem( idCtrl );
		pWnd->GetWindowRect( m_rcExpand );
		m_pParent->ScreenToClient( m_rcExpand );
		GetExpandChilds();
	}
    /// \brief Expand/collapse the parent window
	void SetExpanded( bool expand = true );
    /// \brief Return true if the parent window is currently expanded.
	bool IsExpanded() const { return m_isExpanded; }
    /// \brief Toggle expanded state.
    void Toggle() { SetExpanded( ! m_isExpanded ); }

private:
	void GetExpandChilds();

private:
	struct Wnd
	{
		bool isVisible;
		bool isEnabled;
		HWND hwnd;
		Wnd() : 
			isVisible( true ),
			isEnabled( true ),
			hwnd( NULL ) {}
	};
	typedef std::vector<Wnd> WndList;

	CWnd* m_pParent;
	UINT m_expandRectId;
	WndList m_controls;
	CRect m_rcExpand;
	bool m_isExpanded;
	bool m_isInitialExpand;
};