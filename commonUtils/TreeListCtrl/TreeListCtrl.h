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

#include <map>
#include <hash_map>

#include "MultiSelTree.h"

// Use this as the classname when inserting the CTreeListCtrl control as a custom control
// in the MSVC++ dialog editor
const TCHAR TREELISTCTRL_CLASSNAME[] = _T("TreeListCtrl_zett42"); 


class CTreeListCtrl;
class CTreeListCtrl_tree;

//--------------------------------------------------------------------------------------------
// Header ctrl used by CTreeListCtrl to divide the tree ctrl into columns

class CTreeListCtrl_header : public CHeaderCtrl
{
	DECLARE_DYNCREATE(CTreeListCtrl_header)

// Konstruktion
public:
	CTreeListCtrl_header();

	void LinkToParentCtrl( CTreeListCtrl* pParent )     { m_pParent = pParent; }
    void LinkToTreeCtrl(CTreeListCtrl_tree *pTreeCtrl)  { m_pTreeCtrl = pTreeCtrl; }

	int GetItemWidth( int n ) const
	{
		RECT rc; GetItemRect( n, &rc );
		return rc.right - rc.left;
	}

	bool IsTracking() const { return m_isTracking; }

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTrack( NMHDR * pnm, LRESULT* result );
	afx_msg LRESULT OnItemChange( WPARAM wp, LPARAM lp );
	afx_msg void OnDividerDblClick( NMHDR* _pnm, LRESULT* pResult );
	DECLARE_MESSAGE_MAP()

private:
	CPen m_penForHeaderTrack;
	CTreeListCtrl_tree *m_pTreeCtrl;
	CTreeListCtrl* m_pParent;
    bool m_isTracking;
};

//--------------------------------------------------------------------------------------------
// Tooltip control used by CTreeListCtrl

class CTreeListCtrl_tooltip : public CWnd
{
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnNcHitTest( CPoint pt ) { return HTTRANSPARENT; }
};

//--------------------------------------------------------------------------------------------
// Tree control used by CTreeListCtrl

class CTreeListCtrl_tree : public CMultiSelTree
{
	typedef CMultiSelTree base;
	friend class CTreeListCtrl;

public:
	CTreeListCtrl_tree();

	void ExpandSubTree(HTREEITEM hSubTreeRoot, UINT nCode, bool bExpandRoot);

protected:
	// override
	BOOL OnWndMsg( UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult );

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnCustomDraw( NMHDR* pnm, LRESULT *pResult);
	afx_msg LRESULT OnSetRedraw( WPARAM wp, LPARAM lp );
	afx_msg BOOL OnDeleteItem( NMHDR* pnm, LRESULT* pResult);
	afx_msg void OnSize( UINT type, int cx, int cy );
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown( UINT ch, UINT repCnt, UINT flags );
	afx_msg LRESULT OnInsertItem( WPARAM wp, LPARAM lp );
	afx_msg void OnTimer( UINT_PTR id );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint pt );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint pt );
	DECLARE_MESSAGE_MAP()

private:
	void LinkToParentCtrl( CTreeListCtrl* pParent )          { m_pParent = pParent; }
    void LinkToHeaderCtrl(CTreeListCtrl_header *pHeaderCtrl) { m_pHeaderCtrl = pHeaderCtrl; }

	void DrawTreeGraphics( CDC& dc, LPNMTVCUSTOMDRAW pnm, const RECT &rcSubItem );
	void ExpandSubTreeRecursive( HTREEITEM hSubTreeRoot, UINT nCode );
	void HandleSysColorChange();
	void DrawInsertMark( CDC& dc );

private:
	CTreeListCtrl_header *m_pHeaderCtrl;
	CTreeListCtrl* m_pParent;
	int m_lastClientWidth;
	bool m_isRedrawEnabled;
	bool m_isOnPaint;
	CPen m_penForLines;
	CBrush m_brushForLines;
	COLORREF m_clrBtnShadow, m_clrWindow, m_clrWindowText, m_clrHighlight, m_clrHighlightText;
	bool m_isMouseOver;
	CTreeListCtrl_tooltip m_tooltip;
	TOOLINFO m_toolInfo;
	HTREEITEM m_tooltipItem;
	int m_tooltipColumn;
};


//--------------------------------------------------------------------------------------------
///	 A tree ctrl that can display multiple columns of informations (similar to the list
///        control).

class CTreeListCtrl : public CDialog
{
	DECLARE_DYNCREATE(CTreeListCtrl)
	friend class CTreeListCtrl_header;
	friend class CTreeListCtrl_tree;

public:
	/// Bit flags for "mask" attribute of ItemFormat
	enum 
	{ 
		FMT_COLOR         = 0x0001,   ///< bkColor is defined
		FMT_BKCOLOR       = 0x0002,   ///< textColor is defined
		FMT_HIGHLIGHTTEXT = 0x0004,   ///< highlight the item text only
		FMT_DIVIDER       = 0x0008,   ///< draw a horizontal divider line
		FMT_ITALIC        = 0x0010,   ///< format text with italic font
		FMT_BOLD          = 0x0020    ///< format text with bold font
	};

	/// Formatting attributes of an item
	struct ItemFormat
	{
		DWORD flags;             ///< Bit mask that defines used formatting attributes.
		COLORREF textColor; 
		COLORREF bkColor; 
		ItemFormat() : flags( 0 ), textColor( 0 ), bkColor( 0 ) {}
	};

	/// bit flags for SetOptions()
	enum Options
	{
		OPT_NO_GRAY_SELECTION = 0x0001  ///< no gray selection if unfocused (for TVS_SHOWSELALWAYS)
	};

	/// parameter type for SetColumnWidth(), closely resembles the functionality of
	/// LVM_SETCOLUMNWIDTH of listview controls
	enum AutoWidthType 
	{ 
		AW_FIT_ITEMS,          ///< Fits the column to the largest sub-item text width
		AW_FIT_HEADER,         ///< Fits the column to the header text width
		AW_FIT_ITEMS_OR_HEADER ///< Fits the column to the largest of subitem / header width
	};

	/// parameter type for SetMyInsertMark
	struct InsertMarkPos
	{
		HTREEITEM hParent, hItem;
		bool isBefore;
		InsertMarkPos() : hParent( NULL ), hItem( NULL ), isBefore( false ) {}
	};

	/// custom notification messages send in form of WM_NOTIFY
	enum
	{
		/// Send after an item was inserted. NMTREEVIEW::itemNew contains info about new item.
		TVN_INSERTITEM = TVN_LAST + 1
	};

public:
	CTreeListCtrl();

	///  create the control
	virtual BOOL Create( CWnd* pParentWnd, const RECT& rect, UINT nID, 
		DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		DWORD dwExStyle = WS_EX_CLIENTEDGE );

	//----- generic options

	/// Set options for the whole control
	void SetOptions( DWORD opt ) { m_options = opt; Invalidate(); }
	/// Get options for the whole control
	DWORD GetOptions() const     { return m_options; }

	//----- access to child controls

	CTreeListCtrl_tree& GetTree() { return m_tree; }
	const CTreeListCtrl_tree& GetTree() const { return m_tree; }
	CTreeListCtrl_header& GetHeaderCtrl() { return m_headerCtrl; }
	const CTreeListCtrl_header& GetHeaderCtrl() const { return m_headerCtrl; }

	//----- column methods (mostly wrappers to the underlying header control functionality)

	/// get number of columns
	int GetColumnCount() const { return m_headerCtrl.GetItemCount(); }
	/// insert a column
	int InsertColumn( int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1 );
	/// delete a column
	void DeleteColumn( int nCol )            { m_headerCtrl.DeleteItem( nCol ); } 
	/// set width of a column 
	BOOL SetColumnWidth( int nCol, int cx ); 
	/// automatically set the column width
	void SetColumnAutoWidth( int nCol, AutoWidthType type = AW_FIT_ITEMS_OR_HEADER, int minWidth = 10 );
	/// get width of a column
	int GetColumnWidth( int nCol ) const     { return m_headerCtrl.GetItemWidth( nCol ); }
	/// get the text of a column header
	CString GetColumnText( int nCol ) const;

	//----- item methods

	/// insert item
	HTREEITEM InsertItem( LPTVINSERTSTRUCT lpInsertStruct ) { return m_tree.InsertItem( lpInsertStruct ); }
	/// insert item
	HTREEITEM InsertItem( LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST )
		{ return m_tree.InsertItem( lpszItem, hParent, hInsertAfter ); }
	/// insert item
	HTREEITEM InsertItem( LPCTSTR lpszItem, int nImage, int nSelectedImage, 
	                      HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST )
		{ return m_tree.InsertItem( lpszItem, nImage, nSelectedImage, hParent, hInsertAfter ); }

	/// delete single item
	BOOL DeleteItem( HTREEITEM hItem ) { return m_tree.DeleteItem( hItem ); }
	/// delete all items
	BOOL DeleteAllItems() { return m_tree.DeleteAllItems(); }

	/// set the text of an item
	BOOL SetItemText( HTREEITEM hItem, int nSubItem, LPCTSTR lpszText );
	/// get the text of an item
	CString GetItemText( HTREEITEM hItem, int nSubItem ) const; 

	/// set image of an item
	BOOL SetItemImage( HTREEITEM hItem, int nImage, int nSelectedImage )
		{ return m_tree.SetItemImage( hItem, nImage, nSelectedImage ); }
	/// get image of an item
	BOOL GetItemImage( HTREEITEM hItem, int& nImage, int& nSelectedImage ) const
		{ return m_tree.GetItemImage( hItem, nImage, nSelectedImage ); }

	/// set format of an item
	BOOL SetItemFormat( HTREEITEM hItem, const ItemFormat& format );
	/// get format of an item
	BOOL GetItemFormat( HTREEITEM hItem, ItemFormat* pFormat ) const;

	/// set item format = divider
	void SetItemDivider( HTREEITEM hItem, bool isDivider = true )
	{
		ItemFormat fmt;
		GetItemFormat( hItem, &fmt );
		fmt.flags |= FMT_DIVIDER;
		SetItemFormat( hItem, fmt );
	}

	/// check if item is a divider
	bool IsItemDivider( HTREEITEM hItem ) const
	{
		ItemFormat fmt;
		GetItemFormat( hItem, &fmt );
		return ( fmt.flags & CTreeListCtrl::FMT_DIVIDER ) != 0;	
	}

	int GetSubItemWidth( HTREEITEM hItem, int nCol );
	void GetSubItemRect( RECT* pSubItemRect, RECT* pTextRect, RECT* pUnclippedTextRect, 
	                     HTREEITEM hItem, int nCol );


	//--- imagelist methods

	/// set image list
	CImageList* SetImageList( CImageList* pImgList, int type ) { return m_tree.SetImageList( pImgList, type ); }
	/// get image list
	CImageList* GetImageList( int type ) const { return m_tree.GetImageList( type ); }

	//--- custom insert marker

	/// Set position of custom insert marker. This method allows to specify the indentation level of the
	/// insert marker by specifying a "parent item".
	void SetMyInsertMark( const InsertMarkPos* pPos );

protected:
	// overrides CDialog
	virtual BOOL OnInitDialog();
	// overrides CDialog
	void OnSetFont( CFont* pFont );
	// overrides CDialog
	BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );

	afx_msg LRESULT OnSetRedraw( WPARAM wp, LPARAM lp );
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS* pnp );
	afx_msg void OnNcPaint();
	afx_msg LRESULT VerifyThemeState( WPARAM wp = 0, LPARAM lp = 0 );
	afx_msg void OnSysColorChange();
	void OnContextMenu(CWnd* pWnd, CPoint pt );
	DECLARE_MESSAGE_MAP()

	int GetSubItemWidth( HTREEITEM hItem, int nCol, CDC& dc, const CRect& subItemMargins );

private:
	void AdjustSizeAndPosition( bool bScroll );

	bool m_isThemeActive;
	HTHEME m_hTheme;
	RECT m_rcClientPos;

	// In-memory dialog template for dialog creation without resource script
	struct
	{
		DLGTEMPLATE dlg;
		WORD menuArray;
		WCHAR classArray[64];
		WORD titleArray;
		WORD fontSize;
		WCHAR fontFace[64];
	}
	m_templ;

	CTreeListCtrl_tree m_tree;
	CTreeListCtrl_header m_headerCtrl;
	CScrollBar m_horizScrollBar;

	DWORD m_treeStyle;

	int m_horizScrollPos;
	int m_horizScrollMax;

	// Data associated with each tree item. I could use lParam of TVITEM but this would prevent
	// usage of lParam by user-defined data (for users of this class).
	struct ItemData
	{
		// Map item position to its text. This way, not all sub-items need to be defined, making
		// it indpendent from the current number of list columns.
		typedef std::map<int, CString> TextMap;
		TextMap texts;
		ItemFormat format;
	};
	typedef stdext::hash_map<HTREEITEM, ItemData> ItemDataMap;
	ItemDataMap m_itemData;

	DWORD m_options;

	InsertMarkPos m_insertMark;

	CFont m_font;
};

