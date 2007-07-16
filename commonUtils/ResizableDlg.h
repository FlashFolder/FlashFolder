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

//-----------------------------------------------------------------------------------------------

class CResizeGrip : public CScrollBar
{
protected:
	afx_msg UINT OnNcHitTest( CPoint pt ) { return HTBOTTOMRIGHT; }
	DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------------------------

class CResizableDlg : public CDialog
{
public:
	// Bit flags for anchoring child controls
	enum AlignFlags 
	{ 
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
	};  

public:
	CResizableDlg() { Construct(); }
	
	explicit CResizableDlg( UINT idTempl, CWnd* pParent = NULL ) :
		CDialog( idTempl, pParent ) 
	{ Construct(); }

	/// \brief Set minimum dialog size 
	void SetMinSize( int width, int height )       { m_ptMin = CPoint( width, height ); }
	/// \brief Set minimum dialog size to the current dialog size
	void SetMinSize()       
	{
		CRect rc; GetWindowRect( rc );
		m_ptMin = CPoint( rc.Width(), rc.Height() ); 
	}

	void Anchor( HWND hWnd, UINT flags );
	void Anchor( const CWnd &wnd, UINT flags )     { Anchor( wnd.GetSafeHwnd(), flags ); }
	void Anchor( UINT uID, UINT flags )            { Anchor( *GetDlgItem( uID ), flags ); }

	void AnchorRemove( HWND hWnd )                 { m_controls.erase( hWnd ); }
	void AnchorRemove( const CWnd &wnd )           { AnchorRemove( wnd.GetSafeHwnd() ); }
	void AnchorRemove( UINT uID )                  { AnchorRemove( *GetDlgItem( uID ) ); }

	void AnchorUpdate( HWND hWnd );   
	void AnchorUpdate( const CWnd &wnd )           { AnchorUpdate( wnd.GetSafeHwnd() ); }
	void AnchorUpdate( UINT uID )                  { AnchorUpdate( *GetDlgItem( uID ) ); }

	void AnchorUpdateAll();
	void AnchorRemoveAll()                         { m_controls.clear(); }

protected:
	void Construct()
		{ m_ptMin = CPoint( 0, 0 ); }

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnInitDialog();

	afx_msg void OnSize( UINT type, int cx, int cy );
	afx_msg void OnGetMinMaxInfo( MINMAXINFO* pm );
	DECLARE_MESSAGE_MAP()

private:
	struct AnchorItem 
	{
		UINT flags;        
		RECT pos;               
		AnchorItem( UINT flags_, const RECT& pos_ ) :
			flags( flags_ ), pos( pos_ ) {}
	};
	typedef std::map<HWND, AnchorItem> AnchorItemMap;

	CResizeGrip m_sizeGrip;
	CRect m_rcClient;
	AnchorItemMap m_controls;
	CPoint m_ptMin;
};
