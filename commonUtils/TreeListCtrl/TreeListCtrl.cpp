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
#include "TreeListCtrl.h"
#include "memDC.h"
#include "gdiUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _MSC_VER
	#pragma warning(disable:4018)		// conflict between signed/unsigned
	#pragma warning(disable:4244)		// numeric conversion
#endif

//-----------------------------------------------------------------------------------------------
// Helper stuff

inline bool IsShiftPressed() 
	{ return (GetKeyState(VK_SHIFT) & (1 << (sizeof(SHORT)*8-1))) != 0; }
inline bool IsCtrlPressed()  
	{ return (GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8-1))) != 0; }
inline bool IsAltPressed()  
	{ return (GetKeyState(VK_MENU) & (1 << (sizeof(SHORT)*8-1))) != 0; }

//-----------------------------------------------------------------------------------------------

// child control IDs
const UINT IDC_TREE    = 0x1000;
const UINT IDC_HEADER  = 0x1001;
const UINT IDC_HSCROLL = 0x1002;
  
// margins for the text displayed in a sub-item of the tree (dialog units)
const RECT TREE_SUBITEM_MARGINS = { 0, 0, 3, 3 };
const RECT TREE_FIRSTCOLUMN_TEXT_MARGINS = { 0, 0, 2, 2 };
const RECT TREE_BUTTON_SIZE = { 0, 0, 2, 2 };

// "unique" ID for tooltip timer
const UINT ID_TOOLTIP_TIMER = 0xE89A347F;


//============================================================================================
//  CTreeListCtrl_header methods
//============================================================================================

IMPLEMENT_DYNCREATE(CTreeListCtrl_header, CHeaderCtrl)

BEGIN_MESSAGE_MAP(CTreeListCtrl_header, CHeaderCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT( HDN_BEGINTRACK, OnTrack )
	ON_NOTIFY_REFLECT( HDN_TRACK, OnTrack )
	ON_NOTIFY_REFLECT( HDN_ENDTRACK, OnTrack )
	ON_NOTIFY_REFLECT( HDN_DIVIDERDBLCLICK, OnDividerDblClick )
	ON_MESSAGE( HDM_INSERTITEM, OnItemChange )
	ON_MESSAGE( HDM_SETITEM, OnItemChange )
	ON_MESSAGE( HDM_DELETEITEM, OnItemChange )
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------

CTreeListCtrl_header::CTreeListCtrl_header() :
	m_pTreeCtrl( NULL ),
	m_isTracking( false )
{
	m_penForHeaderTrack.CreatePen( PS_SOLID, 2, 0x444444 );
}

//-----------------------------------------------------------------------------------------

int CTreeListCtrl_header::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CHeaderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

//-----------------------------------------------------------------------------------------

void CTreeListCtrl_header::OnTrack( NMHDR* _pnm, LRESULT* result)
{
	ASSERT(m_pTreeCtrl != NULL);

	LPNMHEADER pnm = reinterpret_cast<LPNMHEADER>( _pnm );

	static int s_lastSectionWidth = 0;
	static int s_lastMoverLineX = 0;
	static BOOL s_isFullDrag = FALSE;
	static HTREEITEM s_hFocusItem = NULL;

	int width = pnm->pitem->cxy;

	switch (pnm->hdr.code)
	{
		case HDN_BEGINTRACK:
		{
			m_isTracking = true;
			s_lastSectionWidth = width;
			s_lastMoverLineX = -1;

			// Respect the users system setting of "Drag full windows".
			::SystemParametersInfo( SPI_GETDRAGFULLWINDOWS, 0, &s_isFullDrag, 0 );

			s_hFocusItem = NULL;
			break;
		}
		case HDN_TRACK:
		{
			if( s_isFullDrag && ! s_hFocusItem )
			{
				// While tracking, focus rect is disabled to avoid drawing artefacts
				s_hFocusItem = m_pTreeCtrl->GetFocusedItem();
				if( s_hFocusItem )
				{
					CRect rcItem;
					m_pTreeCtrl->GetItemRect( s_hFocusItem, rcItem, FALSE );
					m_pTreeCtrl->InvalidateRect( rcItem );
				}
			}

			if( s_lastSectionWidth != width )
			{
				if (width < 0) width = 0; 

				HDITEM hdi = { 0 };
				hdi.mask = HDI_WIDTH;
				hdi.cxy = width;
				if( s_isFullDrag )
				{
					//update the header ctrl
					SetItem(pnm->iItem, &hdi);
				}

				//--- scroll the tree ctrl columns to the right of the 
				//    actual column
				RECT rcTree;
				m_pTreeCtrl->GetClientRect(&rcTree);
				RECT rcInv;
				GetItemRect(pnm->iItem, &rcInv);
				rcInv.top = 0;
				rcInv.bottom = rcTree.bottom;
				RECT rcScroll = rcTree;
				rcScroll.left = rcInv.right;
				RECT rcClip = rcScroll;
				rcClip.left = rcInv.left;

				if( s_isFullDrag )
				{
					//--- scroll the column ---
					m_pTreeCtrl->ScrollWindowEx(width - s_lastSectionWidth, 0, 
						&rcScroll, &rcClip, NULL, NULL, SW_INVALIDATE);

					//--- invalidate the resized tree ctrl column ---
					m_pTreeCtrl->InvalidateRect( &rcInv, FALSE );
				}
				else
				{
					//--- draw sizing line ---
					HDC hdc = ::GetDC(m_pTreeCtrl->m_hWnd);
					HPEN hLastPen = (HPEN) ::SelectObject(hdc, m_penForHeaderTrack);
					int lastRop = ::SetROP2(hdc, R2_NOT);

					if( s_lastMoverLineX != -1 )
					{
						::MoveToEx(hdc, s_lastMoverLineX, 0, NULL);
						::LineTo(hdc, s_lastMoverLineX, rcTree.bottom);
					}
					::MoveToEx(hdc, width + rcInv.left, 0, NULL);
					::LineTo(hdc, width + rcInv.left, rcTree.bottom);
					
					::SetROP2(hdc, lastRop);
					::SelectObject(hdc, hLastPen);
					::ReleaseDC(m_pTreeCtrl->m_hWnd, hdc);
				}

				//remember current header section width
				s_lastSectionWidth = width;
				s_lastMoverLineX = width + rcInv.left;
			}
			break;
		}

		case HDN_ENDTRACK:
		{
			m_isTracking = false;

			// nothing to do if the header item was not moved
			if( s_lastMoverLineX == -1 )
				break;

			if( s_isFullDrag )
			{
				if( s_hFocusItem )
				{
					// show focus rectangle again
					CRect rcItem;
					m_pTreeCtrl->GetItemRect( s_hFocusItem, rcItem, FALSE );
					m_pTreeCtrl->InvalidateRect( rcItem );
				}
			}
			else 
			{
				//--- erase sizing line
				HDC hdc = ::GetDC(m_pTreeCtrl->m_hWnd);
				HPEN hLastPen = (HPEN) ::SelectObject(hdc, m_penForHeaderTrack);
				int lastRop = ::SetROP2(hdc, R2_NOT);

				RECT rcTree;
				m_pTreeCtrl->GetClientRect(&rcTree);
				::MoveToEx(hdc, s_lastMoverLineX, 0, NULL);
				::LineTo(hdc, s_lastMoverLineX, rcTree.bottom);

				::SetROP2(hdc, lastRop);
				::SelectObject(hdc, hLastPen);
				::ReleaseDC(m_pTreeCtrl->m_hWnd, hdc);

				//--- update the header ctrl
				HDITEM hdi = { 0 };
				hdi.mask = HDI_WIDTH;
				hdi.cxy = width;
				SetItem(pnm->iItem, &hdi);

				//--- invalidate the tree columns
				RECT rcInv;
				GetItemRect(pnm->iItem, &rcInv);
				rcInv.top = 0;
				rcInv.bottom = rcTree.bottom;
				rcInv.right = rcTree.right;
				m_pTreeCtrl->InvalidateRect(&rcInv, FALSE);
			}

			break;
		}
	}

	*result = FALSE;  //continue tracking
}

//-----------------------------------------------------------------------------------------------

LRESULT CTreeListCtrl_header::OnItemChange( WPARAM wp, LPARAM lp )
{
	LRESULT res = Default();
    m_pParent->AdjustSizeAndPosition( false );	
	return res;
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_header::OnDividerDblClick( NMHDR* _pnm, LRESULT* pResult )
{
	*pResult = 0;	
	NMHEADER* pnm = reinterpret_cast<NMHEADER*>( _pnm );
	RECT rcMin = { 0, 0, 4, 1 };
	m_pParent->MapDialogRect( &rcMin );
	m_pParent->SetColumnAutoWidth( pnm->iItem, CTreeListCtrl::AW_FIT_ITEMS, rcMin.right );
}

//============================================================================================
//  CTreeListCtrl_tooltip methods
//============================================================================================

BEGIN_MESSAGE_MAP( CTreeListCtrl_tooltip, CWnd )
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


//============================================================================================
//  CTreeListCtrl_tree methods
//============================================================================================

BEGIN_MESSAGE_MAP(CTreeListCtrl_tree, CTreeListCtrl_tree::base)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_MESSAGE( WM_SETREDRAW, OnSetRedraw )
	ON_NOTIFY_REFLECT_EX(TVN_DELETEITEM, OnDeleteItem)
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_MESSAGE( TVM_INSERTITEM, OnInsertItem )
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------

CTreeListCtrl_tree::CTreeListCtrl_tree() :
	m_pParent( NULL ),
	m_lastClientWidth( -1 ),
	m_isRedrawEnabled( true ),
	m_isOnPaint( false ),
	m_isMouseOver( false ),
	m_tooltipItem( NULL ),
	m_tooltipColumn( -1 )
{
	// create initial sys-color dependent objects
	HandleSysColorChange();
}

//-----------------------------------------------------------------------------------------

int CTreeListCtrl_tree::OnCreate( LPCREATESTRUCT pcs) 
{
	if( base::OnCreate( pcs ) == -1 )
		return -1;

	//--- create tooltip control for clipped text (see OnMouseMove())

	m_tooltip.CreateEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, _T(""), TTS_NOPREFIX | TTS_ALWAYSTIP, 
		CRect(0,0,0,0),	this, 0 ); 

	memset( &m_toolInfo, 0, sizeof(m_toolInfo) );
	m_toolInfo.cbSize = sizeof(m_toolInfo);
	m_toolInfo.uFlags = TTF_TRACK | TTF_TRANSPARENT;
	m_toolInfo.hwnd = GetSafeHwnd();

	m_tooltip.SendMessage( TTM_SETMAXTIPWIDTH, 0, SHRT_MAX );
	m_tooltip.SendMessage( TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>( &m_toolInfo ) );

	return 0;
}

//-----------------------------------------------------------------------------------------------

LRESULT CTreeListCtrl_tree::OnSetRedraw( WPARAM wp, LPARAM lp )
{
	if( wp && ! m_isRedrawEnabled )
	{
		Invalidate();
	}
	m_isRedrawEnabled = wp != 0;

	return 0;
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnPaint()
{  
	CPaintDC dc( this );

	if( ! m_isRedrawEnabled )
		return;

	//--- use a memory DC for flicker-free drawing
	CMemDC memDC( &dc );
	m_isOnPaint = true;
	DefWindowProc( WM_PAINT, (WPARAM) memDC.GetSafeHdc(), 0 );
	m_isOnPaint = false;

	DrawInsertMark( memDC );

	return;
}

//-----------------------------------------------------------------------------------------------

BOOL CTreeListCtrl_tree::OnEraseBkgnd( CDC* pDC )
{
	// Avoid flickering by not erasing the background separately.
	return TRUE;
}

//-----------------------------------------------------------------------------------------
// draw the expand/collapse button of a tree view item and the connecting lines

void CTreeListCtrl_tree::DrawTreeGraphics( CDC& dc, LPNMTVCUSTOMDRAW pnm, const RECT &rcSubItem )
{
	COLORREF bkColor = GetBkColor();
	if( bkColor == -1 )
		bkColor = m_clrWindow;
	COLORREF textColor = GetTextColor();
	if( textColor == -1 )
		textColor = m_clrWindowText;

	int indentUnit = GetIndent();
	int indent = ( pnm->iLevel ) * indentUnit;

	const int halfIndentUnit = indentUnit / 2;

	int cx = rcSubItem.left + indent + halfIndentUnit;
	int cy = rcSubItem.top + ( rcSubItem.bottom - rcSubItem.top ) / 2;

	HTREEITEM hItem = (HTREEITEM) pnm->nmcd.dwItemSpec; 
	HTREEITEM hParent = GetParentItem( hItem );
	bool hasChildren = ItemHasChildren( hItem ) != FALSE;

	// draw horiz. line
	dc.FillRect( CRect( cx, cy, 
		rcSubItem.left + indent + indentUnit /*+ halfIndentUnit*/, cy + 1 ), &m_brushForLines );

	// draw top half of vert. line
	dc.FillRect( CRect( cx, cy, cx + 1, rcSubItem.top - 1 ), &m_brushForLines );

	//draw bottom half of vert. line
	if( HTREEITEM hNextSibling = GetNextSiblingItem( hItem ) )
		dc.FillRect( CRect( cx, cy, cx + 1, rcSubItem.bottom + 1 ), &m_brushForLines );
	
	//--- connect vertical lines of higher level nodes
	int x = cx;
	HTREEITEM hTemp = hItem;
	while( HTREEITEM hParent = GetParentItem( hTemp ) )
	{
		x -= indentUnit;
		if( GetNextSiblingItem( hParent ) )
			dc.FillRect( CRect( x, rcSubItem.top, x + 1, rcSubItem.bottom ), &m_brushForLines );
		hTemp = hParent;
	}

	if( hasChildren )
	{
		//--- draw the expand / collapse button

		if( m_pParent->m_hTheme )
		{
			//--- draw themed button

			int state = ( GetItemState( hItem, TVIS_STATEIMAGEMASK ) & TVIS_EXPANDED ) != 0 ? 
				GLPS_OPENED : GLPS_CLOSED;
			SIZE size = { 0 };
			::GetThemePartSize( m_pParent->m_hTheme, dc, TVP_GLYPH, state, NULL, TS_TRUE, &size );
			int x = cx - size.cx / 2;
			int y = cy - size.cy / 2;
			CRect rcBtn = CRect( x, y, x + size.cx, y + size.cy );		
			dc.FillSolidRect( rcBtn, bkColor );
			::DrawThemeBackground( m_pParent->m_hTheme, dc, TVP_GLYPH, state, rcBtn, NULL );
		}
		else
		{
			//--- draw unthemed button that mimics the default look for Win2k / XP

			CRect btnSize = TREE_BUTTON_SIZE;
			m_pParent->MapDialogRect( btnSize );
			CRect rcBtn = CRect( cx - btnSize.Width(), cy - btnSize.Height(), 
				cx + btnSize.Width() + 1, cy + btnSize.Height() + 1 );		

			AutoSelectObj sel_pen( dc, m_penForLines );
			dc.Rectangle( rcBtn.left, rcBtn.top, rcBtn.right, rcBtn.bottom );

			CPen pen( PS_SOLID, 1, textColor );
			AutoSelectObj sel_pen1( dc, pen );
			dc.MoveTo( rcBtn.left + 2, cy );
			dc.LineTo( rcBtn.right - 2, cy );
			if( ! ( GetItemState( hItem, TVIS_STATEIMAGEMASK ) & TVIS_EXPANDED ) )
			{
				dc.MoveTo( cx, rcBtn.top + 2 );
				dc.LineTo( cx, rcBtn.bottom - 2 );
			}
		}
	}
}

//-----------------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnCustomDraw( NMHDR* _pnm, LRESULT *pResult )
{
	NMTVCUSTOMDRAW* pnm = reinterpret_cast<NMTVCUSTOMDRAW*>( _pnm );

	//------- Prepare drawing ----------------------------------------

	//--- enable full owner-draw of the treeview
	
	if( pnm->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	}
	if( pnm->nmcd.dwDrawStage != CDDS_ITEMPREPAINT )
	{
		*pResult = 0;
		return;
	}
	// avoid default drawing since we draw everything by ourself
	*pResult = CDRF_SKIPDEFAULT;

    	// It seems that sometimes the CTreeCtrl sends custom-draw messages directly, not
	// only during WM_PAINT. So we must check the redraw flag here too.
	if( ! m_isRedrawEnabled )
		return;
	if( ! m_isOnPaint )
		return;

	CDC& dc = *CDC::FromHandle( pnm->nmcd.hdc );

	//--- get data associated with item 

	HTREEITEM hTreeItem = reinterpret_cast<HTREEITEM>( pnm->nmcd.dwItemSpec );
	CTreeListCtrl::ItemDataMap::const_iterator itData = m_pParent->m_itemData.find( hTreeItem );

	CTreeListCtrl::ItemFormat itemFormat;
	if( itData != m_pParent->m_itemData.end() )
		itemFormat = itData->second.format;

	//--- get various metrics

	int maxColCount = m_pHeaderCtrl->GetItemCount();
	int colCount = maxColCount;
	if( colCount > 1 && itData == m_pParent->m_itemData.end() )
		colCount = 1;

	RECT rcItem = pnm->nmcd.rc;
	DWORD style = GetStyle();

	int indentUnit = GetIndent();

	CRect rcText1st;
	GetItemRect( hTreeItem, rcText1st, TRUE );
	CRect rcSel = rcText1st;
	rcSel.left -= indentUnit + 1;
	rcSel.top = rcItem.top;
	rcSel.bottom = rcItem.bottom;
	rcSel.right = rcSel.left;
	if( maxColCount > 0 )
	{
		CRect rcFirstCol;
		m_pHeaderCtrl->GetItemRect( 0, rcFirstCol );
		if( rcFirstCol.right < rcSel.left )
			rcSel.left = rcFirstCol.right;

		CRect rcLastCol;
		m_pHeaderCtrl->GetItemRect( maxColCount - 1, rcLastCol );
		rcSel.right = rcLastCol.right;
	}

	//----- draw the item background -----------------------------------------

	//--- draw background left and right of item text

	COLORREF bkColor = GetBkColor();
	if( bkColor == -1 )
		bkColor = m_clrWindow; 
	{
		CBrush brush( bkColor );
		CRect rc = rcItem;
		rc.right = rcSel.left;
		dc.FillRect( rc, &brush );
		rc.left = rcSel.right;
		rc.right = rcItem.right;
		dc.FillRect( rc, &brush );
	}

	//--- draw background of item text (selection)

	COLORREF textColor = pnm->clrText;
	COLORREF textBkColor = pnm->clrTextBk;

	if( pnm->nmcd.uItemState & CDIS_SELECTED )
	{
		if( ( m_pParent->m_options & CTreeListCtrl::OPT_NO_GRAY_SELECTION ) &&
			( style & TVS_SHOWSELALWAYS ) )
		{
			textColor   = m_clrHighlightText;  
			textBkColor = m_clrHighlight;
		}
	}
	else
	{
		if( itemFormat.flags & CTreeListCtrl::FMT_BKCOLOR )
			textBkColor = itemFormat.bkColor;
		if( itemFormat.flags & CTreeListCtrl::FMT_COLOR )
			textColor = itemFormat.textColor;
	}
	{
		CBrush brush( textBkColor );
		dc.FillRect( rcSel, &brush );
	}

	//----- draw the sub items -----------------------------------------------

	if( pnm->nmcd.uItemState & CDIS_FOCUS && 
		! m_pParent->GetHeaderCtrl().IsTracking() )
		dc.DrawFocusRect( rcSel );

	// Getting the bounding box for painting enhances the performance by only processing these
	// columns that have actually changed.
	CRect rcClipBox;
	dc.GetClipBox( rcClipBox );

	CRect rcMargins = TREE_SUBITEM_MARGINS;
	m_pParent->MapDialogRect( rcMargins );

	// simple custom text formatting
	std::auto_ptr<CFont> spFont;
	AutoSelectObj selFont( dc );
	if( itemFormat.flags & CTreeListCtrl::FMT_ITALIC ||
		itemFormat.flags & CTreeListCtrl::FMT_BOLD )
	{
		spFont.reset( new CFont );
        LOGFONT lf;
		GetFont()->GetLogFont( &lf );
		
		if( itData->second.format.flags & CTreeListCtrl::FMT_ITALIC )
			lf.lfItalic = true;
		if(	itData->second.format.flags & CTreeListCtrl::FMT_BOLD )
			lf.lfWeight = FW_BOLD;

		spFont->CreateFontIndirect( &lf );
		selFont.Select( *spFont );
	}

	//--- iterate columns

	for( int nCol = 0; nCol < colCount; ++nCol )
	{
		CRect rcSubItem;
		m_pHeaderCtrl->GetItemRect( nCol, rcSubItem );
		rcSubItem.top = rcItem.top;
		rcSubItem.bottom = rcItem.bottom;

		CRect rcText; 
		CString text;

		if( nCol == 0 )
		{
			GetItemRect( hTreeItem, rcText, TRUE );

			if( rcSubItem.left < rcSubItem.right &&
				rcSubItem.left < rcClipBox.right && rcSubItem.right >= rcClipBox.left )
			{
				//-- draw tree lines, button and icon

				// save the current clipping region
				AutoSaveDC savedDC( dc );

				// set clipping for this column
				CRect rcClip = rcSubItem;
				rcClip.right -= rcMargins.right;
				dc.LPtoDP( rcClip );
				CRgn rgn; rgn.CreateRectRgnIndirect( rcClip );
				dc.SelectClipRgn( &rgn, RGN_AND );

				DrawTreeGraphics( dc, pnm, rcSubItem ); 

				if( ! ( itemFormat.flags & CTreeListCtrl::FMT_DIVIDER ) )
				{
					CImageList* pImgList = GetImageList( TVSIL_NORMAL );
					int nStdImage, nSelImage;
					if( pImgList && GetItemImage( hTreeItem, nStdImage, nSelImage ) )
					{
						IMAGEINFO img;
						pImgList->GetImageInfo( nStdImage, &img );
						int y = ( rcSubItem.Height() - img.rcImage.bottom + img.rcImage.top ) / 2;

						if( nSelImage == 0 )
							nSelImage = nStdImage;
						int nImage = pnm->nmcd.uItemState & CDIS_SELECTED ? nSelImage : nStdImage;

						if( nImage != -1 )
							ImageList_Draw( *pImgList, nImage, dc, 
								rcText.left - indentUnit, rcSubItem.top + y, ILD_NORMAL ); 
					}
				}
			}
			CRect rc = TREE_FIRSTCOLUMN_TEXT_MARGINS;
			m_pParent->MapDialogRect( rc );
			rcText.left += rc.right;
			rcText.right = rcSubItem.right - rcMargins.right;

			text = GetItemText( hTreeItem );

			if( itemFormat.flags & CTreeListCtrl::FMT_DIVIDER )
			{
				//--- draw horiz. divider item instead of item text

				int y = rcText.top + ( rcText.bottom - rcText.top ) / 2;
				CPen pen( PS_SOLID, 1, BlendColor( textColor, textBkColor, 200 ) );
				AutoSelectObj selPen( dc, pen );
				dc.MoveTo( rcSel.left, y );
				dc.LineTo( rcSel.right, y );
				dc.MoveTo( rcSel.left, y + 1 );
				dc.LineTo( rcSel.right, y + 1 );

				break;
			}
		}
		else
		{
			GetItemRect( hTreeItem, rcText, TRUE );
			rcText.right = rcSubItem.right - rcMargins.right;
			rcText.left = rcSubItem.left + rcMargins.right;

			if( itData != m_pParent->m_itemData.end() )
			{
				CTreeListCtrl::ItemData::TextMap::const_iterator itText = itData->second.texts.find( nCol );
				if( itText != itData->second.texts.end() )
					text = itText->second;
			}
		}

		if( rcText.left < rcText.right &&
			rcText.left < rcClipBox.right && rcText.right >= rcClipBox.left &&
			! text.IsEmpty() )
		{
			//--- draw text

			DWORD dtFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;
			HDITEM hditem;
			hditem.mask = HDI_FORMAT;
			m_pHeaderCtrl->GetItem( nCol, &hditem );
			if( hditem.fmt & HDF_CENTER )
				dtFlags |= DT_CENTER;
			else if( hditem.fmt & HDF_RIGHT )
				dtFlags |= DT_RIGHT;

			COLORREF aTextColor = textColor;
			COLORREF aTextBkColor = textBkColor;
			if( nCol == 0 && ( itemFormat.flags & CTreeListCtrl::FMT_HIGHLIGHTTEXT ) )
			{
				aTextColor = m_clrHighlightText;
				aTextBkColor = m_clrHighlight;			
			}
			dc.SetTextColor( aTextColor );
			dc.SetBkColor( aTextBkColor );

			dc.DrawText( text, rcText, dtFlags );
		}
	}
	
	return;
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl::SetMyInsertMark( const InsertMarkPos* pPos )
{
	InsertMarkPos newPos;
	if( pPos )
		newPos = *pPos;
	if( newPos.hItem != m_insertMark.hItem ||
		newPos.hParent != m_insertMark.hParent ||
		newPos.isBefore != m_insertMark.isBefore )
	{
		m_insertMark = newPos;
		// TODO: invalidate only required area
		m_tree.Invalidate();			
	}
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_tree::DrawInsertMark( CDC& dc )
{
	CTreeListCtrl::InsertMarkPos& insMark = m_pParent->m_insertMark;

	if( ! insMark.hItem )
		return;

	int colCount = m_pParent->GetHeaderCtrl().GetItemCount();
	int insertMarkerWidth = 0;
	for( int i = 0; i < colCount; ++i )
		insertMarkerWidth += m_pParent->GetHeaderCtrl().GetItemWidth( i );

	CRect rcMarker;
	GetItemRect( m_pParent->m_insertMark.hItem, &rcMarker, TRUE );
	if( ! m_pParent->m_insertMark.isBefore )
		rcMarker.top = rcMarker.bottom;

	CRect rcParent;
	if( insMark.hParent )
		GetItemRect( insMark.hParent, rcParent, TRUE );
	else
		rcParent.left = GetIndent();
	rcMarker.left = rcParent.left;


	COLORREF color = GetInsertMarkColor();
	CBrush brush( color );
	AutoSelectObj selBrush( dc, brush );
	CPen pen( PS_SOLID, 0, color );
	AutoSelectObj selPen( dc, pen );

	RECT rcMarkerSize = { 0, 0, 4, 2 };
	m_pParent->MapDialogRect( &rcMarkerSize );

	//--- marker line
	dc.MoveTo( rcMarker.left + rcMarkerSize.right, rcMarker.top - 1 );
	dc.LineTo( insertMarkerWidth, rcMarker.top - 1 );
	dc.MoveTo( rcMarker.left + rcMarkerSize.right, rcMarker.top );
	dc.LineTo( insertMarkerWidth, rcMarker.top );

	//--- marker triangle
	CPoint pts[ 4 ];
	pts[ 0 ].x = rcMarker.left;                      
	pts[ 0 ].y = rcMarker.top - rcMarkerSize.bottom - 1;
	pts[ 1 ].x = rcMarker.left + rcMarkerSize.right; 
	pts[ 1 ].y = rcMarker.top - 1;
	pts[ 2 ].x = rcMarker.left + rcMarkerSize.right; 
	pts[ 2 ].y = rcMarker.top;
	pts[ 3 ].x = rcMarker.left;                      
	pts[ 3 ].y = rcMarker.top + rcMarkerSize.bottom;
	dc.Polygon( pts, 4 );
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_tree::HandleSysColorChange()
{
	m_clrBtnShadow = ::GetSysColor( COLOR_BTNSHADOW );
	m_clrWindow = ::GetSysColor( COLOR_WINDOW );
	m_clrWindowText = ::GetSysColor( COLOR_WINDOWTEXT );
	m_clrHighlight = ::GetSysColor( COLOR_HIGHLIGHT );
	m_clrHighlightText = ::GetSysColor( COLOR_HIGHLIGHTTEXT );

    m_penForLines.CreatePen( PS_SOLID, 1, m_clrBtnShadow );

	// Create dotted brush for tree lines.
	struct 
	{
		BITMAPINFOHEADER bmi;
		DWORD pixels[ 4 * 4 ];
	}
	brush = { 0 };
	brush.bmi.biSize = sizeof(brush.bmi);
	brush.bmi.biWidth = 4;
	brush.bmi.biHeight = 4;
	brush.bmi.biBitCount = 32;
	brush.bmi.biCompression = BI_RGB;
	brush.bmi.biPlanes = 1;
	for( int x = 0; x < 4; ++x )
		for( int y = 0; y < 4; ++y )
			brush.pixels[ y * 4 + x ] = ( x & 1 ^ y & 1 ) == 1 ? m_clrBtnShadow : m_clrWindow;
	m_brushForLines.CreateDIBPatternBrush( (const void*) &brush, DIB_RGB_COLORS );
}

//-----------------------------------------------------------------------------------------

BOOL CTreeListCtrl_tree::OnDeleteItem( NMHDR* _pnm, LRESULT* pResult ) 
{
	NMTREEVIEW* pnm = reinterpret_cast<NMTREEVIEW*>( _pnm );

	// delete data associated with this tree item
	m_pParent->m_itemData.erase( pnm->itemOld.hItem );
	*pResult = 0;

	return FALSE; // make the parent receive this message too
}

//-----------------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// enable expand / collapse by double-clicking on the full row
	TVHITTESTINFO ht;
	ht.pt = point;
	HTREEITEM hItem = HitTest( &ht );
	if( hItem && ( ht.flags & TVHT_ONITEMRIGHT ) )
	{
		NMHDR nmh = { 0 };
		nmh.idFrom = GetDlgCtrlID();
		nmh.hwndFrom = GetSafeHwnd();
		nmh.code = NM_DBLCLK;
		if( GetParent()->SendMessage( WM_NOTIFY, nmh.idFrom, reinterpret_cast<LPARAM>( &nmh ) ) )
			return;

		// Expand() does not always send the standard notification messages, so do it ourselfs.
		NMTREEVIEW nm = { 0 };
		nm.hdr.idFrom = nmh.idFrom;
		nm.hdr.hwndFrom = nmh.hwndFrom; 
		nm.itemNew.hItem = hItem;
		nm.itemNew.state = GetItemState( hItem, UINT(-1) );
		nm.itemNew.lParam = GetItemData( hItem );
		nm.action = ( nm.itemNew.state & TVIS_EXPANDED ) ? TVE_COLLAPSE : TVE_EXPAND;
		
		nm.hdr.code = TVN_ITEMEXPANDING;
		if( ! GetParent()->SendMessage( WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>( &nm ) ) )
		{
			Expand( hItem, TVE_TOGGLE );

			nm.hdr.code = TVN_ITEMEXPANDED;
			GetParent()->SendMessage( WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>( &nm ) );
		}
	}

	base::OnLButtonDblClk(nFlags, point);
}

//---------------------------------------------------------------------------------

LRESULT CTreeListCtrl_tree::OnNcHitTest(CPoint point) 
{
	//OnNcHitTest is handled 'cause sometimes the header ctrl would not
	//  receive clicks since it lays OVER the tree ctrl

	UINT res = base::OnNcHitTest(point);
	if( res == HTNOWHERE )
		return HTTRANSPARENT;
	return res;
}

//---------------------------------------------------------------------------------

void CTreeListCtrl_tree::ExpandSubTreeRecursive(HTREEITEM hSubTreeRoot, UINT nCode)
{
	HTREEITEM hItem = GetChildItem(hSubTreeRoot);
	while (hItem != NULL)
	{
		if (ItemHasChildren(hItem))
			ExpandSubTreeRecursive(hItem, nCode);
		hItem = GetNextSiblingItem(hItem);
	}
	
	Expand(hSubTreeRoot, nCode);
}

void CTreeListCtrl_tree::ExpandSubTree(HTREEITEM hSubTreeRoot, UINT nCode, bool bExpandRoot)
{
	HTREEITEM hItem = GetChildItem(hSubTreeRoot);
	while (hItem != NULL)
	{
		if (ItemHasChildren(hItem))
			ExpandSubTreeRecursive(hItem, nCode);
		hItem = GetNextSiblingItem(hItem);
	}

	if (bExpandRoot)
		Expand(hSubTreeRoot, nCode);
}


//---------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* pnsc ) 
{
	base::OnNcCalcSize( bCalcValidRects, pnsc );

	if( bCalcValidRects )
	{
		// to switch off the default horizontal scroll bar, we simply set the 
		// height of the client area to the window height so there will be no 
		// "non-client-space" where the scrollbar can reside in.
		// We provide our own horiz. scrollbar through the parent 
		//   window (CTreeListCtrl).
		pnsc->rgrc[0].bottom = pnsc->rgrc[0].top + pnsc->lppos->cy;

		// if the width of the client area changes, the width of the horiz. scrollbar must be updated
		if( pnsc->rgrc[0].right != m_lastClientWidth )
		{
			m_lastClientWidth = pnsc->rgrc[0].right;

			//--- update horiz. scrollbar size 
			
			RECT rcParent; m_pParent->GetClientRect( &rcParent );
			int horizScrollbarHeight = ::GetSystemMetrics( SM_CYHSCROLL );

			m_pParent->m_horizScrollBar.SetWindowPos( NULL,
				0, 0, m_lastClientWidth, horizScrollbarHeight, 
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			m_pParent->m_horizScrollBar.RedrawWindow();
		}		
	}
}

//---------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnMouseMove( UINT nFlags, CPoint pt ) 
{
	//--- show tooltip if subitem text under mouse cursor is clipped

	CRect rcClient;
	GetClientRect( rcClient );

	CRect rcParent;
	m_pParent->GetClientRect( rcParent );
	m_pParent->MapWindowPoints( this, rcParent );  

	// determine mouse-over item and column, if it contains clipped text
	int nCol = -1;
	CRect rcCol;
	UINT htFlags = 0;
	HTREEITEM hItem = HitTest( pt, &htFlags );
	if( hItem && ! ( htFlags & TVHT_ONITEMINDENT ) )
	{
		int colCount = m_pParent->m_headerCtrl.GetItemCount();
		for( int i = 0; i < colCount; ++i )
		{
			m_pParent->m_headerCtrl.GetItemRect( i, &rcCol );

			int subWidth = m_pParent->GetSubItemWidth( hItem, i );

			rcCol.right = min( rcCol.right, rcCol.left + subWidth );

			if( pt.x >= rcCol.left && pt.x < rcCol.right )
			{
				if( subWidth > rcCol.Width() || rcCol.right > rcParent.right )
					nCol = i;
				break;
			}
		}
	}
	
	if( nCol != -1 )
	{
		if( hItem != m_tooltipItem || nCol != m_tooltipColumn )
		{
			m_tooltipItem = hItem;
			m_tooltipColumn = nCol;

			m_tooltip.SetFont( GetFont(), FALSE );

			CString s = m_pParent->GetItemText( hItem, nCol );
			m_toolInfo.lpszText = const_cast<LPTSTR>( s.GetString() );
			m_tooltip.SendMessage( TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>( &m_toolInfo ) );

			CRect rcTT;
			m_pParent->GetSubItemRect( NULL, &rcTT, NULL, hItem, nCol );
			ClientToScreen( rcTT );		
			m_tooltip.SendMessage( TTM_ADJUSTRECT, TRUE, reinterpret_cast<LPARAM>( &rcTT ) );
			m_tooltip.SendMessage( TTM_TRACKPOSITION, 0, MAKELONG( rcTT.left, rcTT.top ) );
						
			if( ! m_tooltip.IsWindowVisible() )
				m_tooltip.SendMessage( 
					TTM_TRACKACTIVATE, TRUE, reinterpret_cast<LPARAM>( &m_toolInfo ) );
		}
	}
	else
	{
		m_tooltip.SendMessage( 
			TTM_TRACKACTIVATE, FALSE, reinterpret_cast<LPARAM>( &m_toolInfo ) );
			
		m_tooltipItem = NULL;
		m_tooltipColumn = -1;			
	}

	if( ! m_isMouseOver )
	{
		m_isMouseOver = true;
		// Create a timer for checking when the mouse leaves the window so we should hide the tooltip.
		SetTimer( ID_TOOLTIP_TIMER, 250, NULL );
	}
	
	base::OnMouseMove(nFlags, pt );
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnTimer( UINT_PTR id )
{
	if( id != ID_TOOLTIP_TIMER )
		return;

	//--- hide tooltip if cursor is outside of window

	CRect rc, rcParent;
	GetClientRect( rc );
	ClientToScreen( rc );
	m_pParent->GetWindowRect( rcParent );
	rc.left = rcParent.left;
	rc.right = rcParent.right;

	POINT pt;
	GetCursorPos( &pt );

    if( ! rc.PtInRect( pt ) )
	{
		m_tooltip.SendMessage( 
			TTM_TRACKACTIVATE, FALSE, reinterpret_cast<LPARAM>( &m_toolInfo ) );

		KillTimer( ID_TOOLTIP_TIMER );

		m_isMouseOver = false;
		m_tooltipItem = NULL;
		m_tooltipColumn = -1;			
	}
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnKeyDown( UINT ch, UINT repCnt, UINT flags )
{
	bool handled = false;

	//--- handle horizontal scroll keys since treectrl has only vert. scrollbar

	if( IsCtrlPressed() )
	{
		handled = true;
		if( ch == VK_LEFT )
			m_pParent->SendMessage( WM_HSCROLL, SB_LINELEFT );
		else if( ch == VK_RIGHT )
			m_pParent->SendMessage( WM_HSCROLL, SB_LINERIGHT );
		else if( ch == VK_ADD )  // Num +
		{
			// resize all columns to fit (mimics listview behaviour)
			
			RECT rcMin = { 0, 0, 4, 1 };
			m_pParent->MapDialogRect( &rcMin );

			int colCount = m_pParent->GetColumnCount();
			for( int i = 0; i < colCount; ++i )
				m_pParent->SetColumnAutoWidth( i, CTreeListCtrl::AW_FIT_ITEMS, rcMin.right ); 
		}
		else 
			handled = false;
	}    
	if( IsShiftPressed() )
	{
		handled = true;
		if( ch == VK_LEFT )
			m_pParent->SendMessage( WM_HSCROLL, SB_PAGELEFT );
		else if( ch == VK_RIGHT )
			m_pParent->SendMessage( WM_HSCROLL, SB_PAGERIGHT );
		else 
			handled = false;
	}    

	if( ! handled )
		base::OnKeyDown( ch, repCnt, flags );
}

//-----------------------------------------------------------------------------------------------

BOOL CTreeListCtrl_tree::OnWndMsg( UINT message, WPARAM wp, LPARAM lp, LRESULT* pResult )
{
	BOOL res = base::OnWndMsg( message, wp, lp, pResult );

	if( message == WM_VSCROLL )
	{
		*pResult = GetParent()->SendMessage( message, wp, lp );
		return TRUE;
	}

	return res;
}

//-----------------------------------------------------------------------------------------------

LRESULT CTreeListCtrl_tree::OnInsertItem( WPARAM wp, LPARAM lp )
{
	LRESULT res = Default();

	TVINSERTSTRUCT* pIns = reinterpret_cast<TVINSERTSTRUCT*>( lp );

	// provide TVN_INSERT notification which is not done by original tree control
	NMTREEVIEW nm = { 0 };
	nm.itemNew = pIns->item;
	nm.hdr.code = CTreeListCtrl::TVN_INSERTITEM;
	nm.hdr.idFrom = m_pParent->GetDlgCtrlID();
	nm.hdr.hwndFrom = m_pParent->GetSafeHwnd();
	m_pParent->GetParent()->SendMessage( WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>( &nm ) );

	return res;
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnRButtonDown( UINT nFlags, CPoint pt )
{
	// Don't call default handler, since it	messes things up when we want to provide a context menu.

	// Select / focus item under mouse cursor as necessary.
	UINT flags = 0;
	HTREEITEM hItem = HitTest( pt, &flags );
	if( hItem && ! ( flags & TVHT_ONITEMINDENT ) )
	{
		UINT state = TVIS_FOCUSED;
		if( ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) == 0 )
		{
			SelectAll( FALSE );
			state |= TVIS_SELECTED;
		}
		SetItemState( hItem, state, state );
	}
	else
	{
		SetFocusedItem( NULL );
		SelectAll( FALSE );
	}
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl_tree::OnRButtonUp( UINT nFlags, CPoint pt )
{
	// Send context menu message that is hidden by default right-click handler of tree control.
	SendMessage( WM_CONTEXTMENU, 
		reinterpret_cast<WPARAM>( m_pParent->GetSafeHwnd() ), GetMessagePos() );
}


//============================================================================================
//  CTreeListCtrl methods
//============================================================================================

IMPLEMENT_DYNCREATE(CTreeListCtrl, CDialog)

BEGIN_MESSAGE_MAP(CTreeListCtrl, CDialog)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_MESSAGE( WM_SETREDRAW, OnSetRedraw )
	ON_MESSAGE( WM_THEMECHANGED, VerifyThemeState )
	ON_MESSAGE( WM_STYLECHANGED, VerifyThemeState )
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

//--------------------------------------------------------------------------------------------

CTreeListCtrl::CTreeListCtrl() :
	m_horizScrollPos( 0 ),
	m_horizScrollMax( 0 ),
	m_isThemeActive( false ),
	m_hTheme( NULL ),
	m_options( 0 ),
	m_treeStyle( 0 )
{
	memset( &m_rcClientPos, 0, sizeof(m_rcClientPos) );

	//--- register the window class for the window tree if not already done

    WNDCLASS wndcls = { 0 };
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInst, TREELISTCTRL_CLASSNAME, &wndcls)))
    {
        // otherwise we need to register a new class
        wndcls.lpszClassName    = TREELISTCTRL_CLASSNAME;
        wndcls.hInstance        = hInst;
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
		// following two params are important to create a dialog
		wndcls.lpfnWndProc      = ::DefDlgProc;
		wndcls.cbWndExtra       = DLGWINDOWEXTRA;

        if (!AfxRegisterClass(&wndcls))
            AfxThrowResourceException();
    }
}

//--------------------------------------------------------------------------------------------

BOOL CTreeListCtrl::Create( CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle, DWORD dwExStyle ) 
{
    ASSERT( pParentWnd->GetSafeHwnd() );
    
	m_treeStyle = dwStyle & 0xFFFF;

	memset( &m_templ, 0, sizeof(m_templ) );
	m_templ.dlg.style = ( dwStyle & 0xFFFF0000 & ~WS_VISIBLE ) | WS_CHILD | WS_CLIPCHILDREN | DS_CONTROL | DS_SETFONT;
	m_templ.dlg.dwExtendedStyle = dwExStyle;
	StringCbCopyW( m_templ.classArray, sizeof(m_templ.classArray), L"TreeListCtrl_zett42" );
	StringCbCopyW( m_templ.fontFace, sizeof(m_templ.fontFace), L"MS Shell Dlg" );
	m_templ.fontSize = 8;
	
	if( CreateIndirect( &m_templ, pParentWnd ) )
	{
		SetDlgCtrlID( nID );
		DWORD flags = SWP_NOZORDER;
		if( dwStyle & WS_VISIBLE )
			flags |= SWP_SHOWWINDOW;
		SetWindowPos( NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, flags );
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------

BOOL CTreeListCtrl::OnInitDialog()
{
	CDialog::OnInitDialog();

	VerifyThemeState();

	//--- Create header ctrl and link it to the tree ctrl
    m_headerCtrl.LinkToParentCtrl( this );
	m_headerCtrl.Create( WS_CHILD | WS_VISIBLE | CCS_TOP | HDS_BUTTONS | HDS_HORZ,
		CRect(0,0,0,0), this, IDC_HEADER );
    m_headerCtrl.LinkToTreeCtrl( &m_tree );

	//--- create tree ctrl ----
	m_tree.LinkToParentCtrl( this );
	m_tree.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_NOTOOLTIPS | m_treeStyle, 
		CRect(0,0,0,0), this, IDC_TREE );
    m_tree.LinkToHeaderCtrl( &m_headerCtrl );

	//--- create horiz. scrollbar
	m_horizScrollBar.Create( WS_CHILD | WS_VISIBLE | SBS_HORZ | SBS_BOTTOMALIGN,
		CRect(0,0,0,0), this, IDC_HSCROLL );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

LRESULT CTreeListCtrl::OnSetRedraw( WPARAM wp, LPARAM lp )
{
	Default();
	m_headerCtrl.SetRedraw( wp );
	m_tree.SetRedraw( wp );
	return 0;
}

//--------------------------------------------------------------------------------------------

void CTreeListCtrl::OnSize( UINT nType, int cx, int cy ) 
{
	CDialog::OnSize( nType, cx, cy );
	if( m_headerCtrl.GetSafeHwnd() )
	{
		AdjustSizeAndPosition( false );
	}
}

//--------------------------------------------------------------------------------------------

void CTreeListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//---- calculate new scroll position ----
	
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	m_horizScrollBar.GetScrollInfo(&si);	

	int newPos;

	switch (nSBCode)
	{
		case SB_LEFT: newPos = si.nMin; break;
		case SB_RIGHT: newPos = si.nMax - si.nPage; break;
		case SB_LINELEFT: newPos = si.nPos - 10; break;
		case SB_LINERIGHT: newPos = si.nPos + 10; break;
		case SB_PAGELEFT: newPos = si.nPos - si.nPage; break;
		case SB_PAGERIGHT: newPos = si.nPos + si.nPage; break;
		case SB_ENDSCROLL: newPos = m_horizScrollPos; break;
		default: newPos = nPos; break;
	}

	if (newPos < 0) newPos = 0;
	else if (newPos > si.nMax - si.nPage) newPos = si.nMax - si.nPage;

	m_horizScrollBar.SetScrollPos(newPos);

	m_horizScrollPos = newPos;

	AdjustSizeAndPosition( true );
	m_headerCtrl.RedrawWindow();
}

//--------------------------------------------------------------------------------------------

int CTreeListCtrl::InsertColumn( int nCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth )
{
	HDITEM item = { 0 };
	item.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	item.cxy = nWidth;
	item.pszText = const_cast<LPTSTR>( lpszColumnHeading );
	item.fmt = 0;
	if( nFormat == LVCFMT_CENTER )
		item.fmt = HDF_CENTER;
	else if( nFormat == LVCFMT_RIGHT )
		item.fmt = HDF_RIGHT;
	return m_headerCtrl.InsertItem( nCol, &item );
}

//--------------------------------------------------------------------------------------------

void CTreeListCtrl::AdjustSizeAndPosition( bool bScroll )
{
	//--- adjusts size and position of the contained controls 
	// Reasons:
	//    * horizontal scrolling 
	//    * window size change
	//    * vertical scrollbar visibility change

	if (m_horizScrollBar.m_hWnd == NULL || m_headerCtrl.m_hWnd == NULL ||
		m_tree.m_hWnd == NULL)
		return;

	RECT rc;
	GetClientRect(&rc);

	//--- update scrollbar info ---
	int bottomAdjust = 0;
	int horizScrollbarHeight = ::GetSystemMetrics(SM_CYHSCROLL);
	int vertScrollbarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);

	if (bScroll)
	{
		m_horizScrollBar.GetScrollInfo(&si, SIF_ALL);
	}
	else
	{
		//set scroll bar range + page size
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nPage = rc.right - vertScrollbarWidth;
		//si.nPage = m_tree.m_lastClientWidth; 
		
		si.nMax = 0;
		int headerItemCount = m_headerCtrl.GetItemCount();
		for( int i = 0; i < headerItemCount; ++i )
			si.nMax += m_headerCtrl.GetItemWidth( i );

		m_horizScrollBar.SetScrollInfo(&si);
	}
	
	//adjust scroll pos if there is empty space on the right side of the last column
	if (m_horizScrollPos > 0)
	{
		int sOffs = si.nPage - (si.nMax - m_horizScrollPos);
		if (sOffs > 0)
		{
			m_horizScrollPos -= sOffs;
			if (m_horizScrollPos < 0) m_horizScrollPos = 0;
			m_horizScrollBar.SetScrollPos(m_horizScrollPos);
		}
	}

	//check if horiz. scrollbar must be shown, set bottom adjustment for tree ctrl
	if (si.nPage < si.nMax)
	{
		bottomAdjust = horizScrollbarHeight;
		if (! m_horizScrollBar.IsWindowVisible())
			m_horizScrollBar.ShowWindow(TRUE);
	}
	else
		m_horizScrollBar.ShowWindow(FALSE);


	//--- adjust size and position of the header ctrl ---
	HDLAYOUT hdl;
	WINDOWPOS wp;	
	rc.left -= m_horizScrollPos;
	hdl.prc = &rc; 
	hdl.pwpos = &wp; 
	m_headerCtrl.SendMessage(HDM_LAYOUT, 0, (LPARAM) &hdl);
	m_headerCtrl.SetWindowPos(NULL, 
		wp.x, wp.y, wp.cx + m_horizScrollPos, wp.cy, 
		SWP_NOZORDER | SWP_NOACTIVATE); 

	//--- adjust the size and position of the tree ctrl ---
	m_tree.SetWindowPos(NULL,
		-m_horizScrollPos, wp.cy, 
		rc.right + m_horizScrollPos, rc.bottom - wp.cy - bottomAdjust, 
		SWP_NOZORDER | SWP_NOACTIVATE);

	//--- adjust position of horiz. scrollbar
	m_horizScrollBar.SetWindowPos( NULL,
		0, rc.bottom - horizScrollbarHeight, 0, 0, 
		SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	if( ! bScroll )
		m_horizScrollBar.RedrawWindow();
}

//-----------------------------------------------------------------------------------------------

BOOL CTreeListCtrl::SetItemText( HTREEITEM hItem, int nSubItem, LPCTSTR lpszText )
{
	if( nSubItem == 0 )
		return m_tree.SetItemText( hItem, lpszText );

	ItemDataMap::iterator it = m_itemData.find( hItem );

	if( it != m_itemData.end() )
	{
		if( ! lpszText )
		{
			it->second.texts.erase( nSubItem );
			if( it->second.texts.empty() )
				m_itemData.erase( it );
		}
	}
	else
	{
		if( lpszText )
			it = m_itemData.insert( m_itemData.end(), std::make_pair( hItem, ItemData() ) );
	}

	if( lpszText )
		it->second.texts[ nSubItem ] = lpszText;

	CRect rcItem; m_tree.GetItemRect( hItem, rcItem, FALSE );
	m_tree.InvalidateRect( rcItem );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

CString CTreeListCtrl::GetItemText( HTREEITEM hItem, int nSubItem ) const
{
	if( nSubItem == 0 )
		return m_tree.GetItemText( hItem );

	ItemDataMap::const_iterator it = m_itemData.find( hItem );
	if( it != m_itemData.end() )
	{
		ItemData::TextMap::const_iterator itText = it->second.texts.find( nSubItem );
		if( itText != it->second.texts.end() )
			return itText->second;
	}
	return _T("");
}

//-----------------------------------------------------------------------------------------------

CString CTreeListCtrl::GetColumnText( int nCol ) const
{
	TCHAR text[ 256 ];
	HDITEM item;
	item.mask = HDI_TEXT;
	item.pszText = text;
	item.cchTextMax = sizeof(text) / sizeof(TCHAR) - 1;
	if( ! m_headerCtrl.GetItem( nCol, &item ) )
		return _T("");
	return item.pszText;
}

//-----------------------------------------------------------------------------------------------

BOOL CTreeListCtrl::SetColumnWidth( int nCol, int width )
{
	m_tree.Invalidate();
	HDITEM item;
	item.mask = HDI_WIDTH;
	item.cxy = width;
	return m_headerCtrl.SetItem( nCol, &item );
}

//-----------------------------------------------------------------------------------------------
/// get unclipped width of subitem text

int CTreeListCtrl::GetSubItemWidth( HTREEITEM hItem, int nCol )
{
	CRect subItemMargins = TREE_SUBITEM_MARGINS;
	MapDialogRect( subItemMargins );

	CWindowDC dc( &m_tree );

	CFont* pFont = m_tree.GetFont();
	if( ! pFont )
		pFont = CFont::FromHandle( (HFONT) ::GetStockObject( DEFAULT_GUI_FONT ) );
	AutoSelectObj trSelFont( dc, *pFont );

	return GetSubItemWidth( hItem, nCol, dc, subItemMargins );
}

//-----------------------------------------------------------------------------------------------
/// get unclipped width of subitem text (optimized for batch calculation of multiple items)

int CTreeListCtrl::GetSubItemWidth( HTREEITEM hItem, int nCol, CDC& dc, const CRect& subItemMargins )
{
	CString text = GetItemText( hItem, nCol );
	CRect rc = CRect( 0, 0, 1, 1 );
	dc.DrawText( text, rc, DT_CALCRECT | DT_SINGLELINE | DT_HIDEPREFIX ); 
	rc.right += subItemMargins.right * 2;
    if( nCol == 0 )
	{
		// add tree indentation + icon size to text width
		CRect rcIndent;
		m_tree.GetItemRect( hItem, rcIndent, TRUE );
		rc.right += rcIndent.left;
	}

	return rc.right;
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl::GetSubItemRect( RECT* pSubItemRect, RECT* pTextRect, RECT* pUnclippedTextRect, 
                                    HTREEITEM hItem, int nCol )
{
	CRect rcSubItem;
	m_tree.GetItemRect( hItem, rcSubItem, FALSE );
	CRect rcCol;
	m_headerCtrl.GetItemRect( nCol, rcCol );
	rcSubItem.left = rcCol.left;
	rcSubItem.right = rcCol.right;

	if( pSubItemRect )
		*pSubItemRect = rcSubItem;

	if( ! pTextRect && ! pUnclippedTextRect )
		return;

	CWindowDC dc( &m_tree );

	CFont* pFont = m_tree.GetFont();
	if( ! pFont )
		pFont = CFont::FromHandle( (HFONT) ::GetStockObject( DEFAULT_GUI_FONT ) );
	AutoSelectObj trSelFont( dc, *pFont );

	CString text = GetItemText( hItem, nCol );
	CRect rcUnclippedText = CRect( 0, 0, 1, 1 );
	dc.DrawText( text, rcUnclippedText, DT_CALCRECT | DT_SINGLELINE | DT_HIDEPREFIX ); 

	CRect rcText;
	m_tree.GetItemRect( hItem, rcText, TRUE );
	rcText.OffsetRect( 0, ( rcSubItem.Height() - rcUnclippedText.Height() ) / 2 );

	CRect rcMargins = TREE_SUBITEM_MARGINS;
	MapDialogRect( rcMargins );

	if( nCol == 0 )
	{
		CRect rc = TREE_FIRSTCOLUMN_TEXT_MARGINS;
		MapDialogRect( rc );
		rcText.left += rc.right;
		rcText.right = rcSubItem.right - rcMargins.right;
	}
	else
	{
		rcText.left = rcSubItem.left + rcMargins.right;
		rcText.right = rcSubItem.right - rcMargins.right;
	}

	if( pTextRect )
		*pTextRect = rcText;
	if( pUnclippedTextRect )
	{
		rcUnclippedText.OffsetRect( rcText.left, rcText.top );
		*pUnclippedTextRect = rcUnclippedText;
	}
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl::SetColumnAutoWidth( int nCol, AutoWidthType type, int minWidth )
{
	//--- get text, format and width of header item

	TCHAR text[ 256 ] = _T("");
	HDITEM hditem;
	hditem.mask = HDI_FORMAT | HDI_TEXT | HDI_WIDTH;
	hditem.pszText = text;
	hditem.cchTextMax = sizeof(text) / sizeof(TCHAR) - 1;
	if( ! m_headerCtrl.GetItem( nCol, &hditem ) )
	{
		hditem.pszText = _T("");
		hditem.fmt = HDF_LEFT;
		hditem.cxy = 0;
	}

	//--- get text extent and margins of header item

	CWindowDC hddc( &m_headerCtrl );

	AutoSelectObj hdSelFont( hddc, *m_headerCtrl.GetFont() );

	CRect hdTextsize = CRect( 0, 0, 1, 1 );
	hddc.DrawText( hditem.pszText, hdTextsize, DT_SINGLELINE | DT_CALCRECT ); 
	// Guessed size of margins since I couldn't find an API for this
	CRect hdMargins = CRect( 0, 0, 8, 1 );
	MapDialogRect( hdMargins );
	hdTextsize.right += hdMargins.right;

	//--- resize the column

	if( type == AW_FIT_HEADER )
	{
		SetColumnWidth( nCol, max( hdTextsize.right, minWidth ) );
		return;
	}

	//--- enumerate all visible items and calculate the max. text width from them

	CWindowDC trdc( &m_tree );

	CFont* pFont = m_tree.GetFont();
	if( ! pFont )
		pFont = CFont::FromHandle( (HFONT) ::GetStockObject( DEFAULT_GUI_FONT ) );
	AutoSelectObj trSelFont( trdc, *pFont );

	CRect subItemMargins = TREE_SUBITEM_MARGINS;
	MapDialogRect( subItemMargins );

	int maxItemWidth = 0;
	for( HTREEITEM hItem = m_tree.GetFirstVisibleItem(); hItem;
		hItem = m_tree.GetNextVisibleItem( hItem ) )
	{
		int width = GetSubItemWidth( hItem, nCol, trdc, subItemMargins );
		if( width > maxItemWidth )
			maxItemWidth = width;
	}

	if( type == AW_FIT_ITEMS_OR_HEADER )
		maxItemWidth = max( maxItemWidth, hdTextsize.right );

	SetColumnWidth( nCol, max( maxItemWidth, minWidth ) );
}

//-----------------------------------------------------------------------------------------------

BOOL CTreeListCtrl::SetItemFormat( HTREEITEM hItem, const ItemFormat& format )
{
	ItemDataMap::iterator it = m_itemData.find( hItem );
	if( it == m_itemData.end() )
		it = m_itemData.insert( m_itemData.end(), std::make_pair( hItem, ItemData() ) );

	it->second.format = format;

	CRect rcItem; m_tree.GetItemRect( hItem, rcItem, FALSE );
	m_tree.InvalidateRect( rcItem );

	return TRUE;
}

BOOL CTreeListCtrl::GetItemFormat( HTREEITEM hItem, ItemFormat* pFormat ) const
{
	ItemDataMap::const_iterator it = m_itemData.find( hItem );
	if( it == m_itemData.end() )
		return FALSE;

	*pFormat = it->second.format;

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl::OnSetFont( CFont* pFont )
{
	CDialog::OnSetFont( pFont );
	if( m_tree.GetSafeHwnd() )
		m_tree.SetFont( pFont );
	if( m_headerCtrl.GetSafeHwnd() )
		m_headerCtrl.SetFont( pFont );
}

//-----------------------------------------------------------------------------------------------

BOOL CTreeListCtrl::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	if( CDialog::OnNotify( wParam, lParam, pResult ) )
		return TRUE;

	//--- route notifications from CTreeCtrl to the parent window

	if( wParam == IDC_TREE )
	{
		NMTREEVIEW nm = *reinterpret_cast<NMTREEVIEW*>( lParam );
		nm.hdr.idFrom = GetDlgCtrlID();
		nm.hdr.hwndFrom = GetSafeHwnd();
		GetParent()->SendMessage( WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>( &nm ) );
		return TRUE;
	}
	
	return FALSE;
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl::OnSysColorChange()
{
	// WM_SYSCOLORCHANGE is only send to top-level windows so it must be forwarded to childs
	CDialog::OnSysColorChange();
	m_tree.HandleSysColorChange();
	m_headerCtrl.SendMessage( WM_SYSCOLORCHANGE );
	m_horizScrollBar.SendMessage( WM_SYSCOLORCHANGE );
}

//-----------------------------------------------------------------------------------------------

void CTreeListCtrl::OnContextMenu(CWnd* pWnd, CPoint pt )
{
	// forward WM_CONTEXTMENU from CTreeCtrl to parent of CTreeListCtrl
	CDialog::OnContextMenu( pWnd, pt );
	
	const MSG* pMsg = GetCurrentMessage();
	GetParent()->SendMessage( WM_CONTEXTMENU, pMsg->wParam, pMsg->lParam );
}

//-----------------------------------------------------------------------------------------------
//=== BEGIN WinXP themed border implementation

// Calculate the client size according to the themed border.

void CTreeListCtrl::OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS* pnp )
{
	if( ! m_hTheme )
	{
		Default();
		return;
	}

	bool bSuccess = false;

	//Get the size required by the current theme to be displayed properly
	RECT rcClient = { 0 }; 

	CWindowDC dc( GetParent() );

	if( ::GetThemeBackgroundContentRect( m_hTheme, dc, TVP_TREEITEM, TREIS_NORMAL,
		&pnp->rgrc[0], &rcClient ) == S_OK )
	{
		// Add a pixel to every edge so that the client area is not too close to the border drawn by the theme 
		// (thus simulating a native treeview)
		InflateRect(&rcClient, -1, -1);

		m_rcClientPos.left = rcClient.left-pnp->rgrc[0].left;
		m_rcClientPos.top = rcClient.top-pnp->rgrc[0].top;
		m_rcClientPos.right = pnp->rgrc[0].right-rcClient.right;
		m_rcClientPos.bottom = pnp->rgrc[0].bottom-rcClient.bottom;

		memcpy(&pnp->rgrc[0], &rcClient, sizeof(RECT));

		bSuccess = true;
	}
	if( ! bSuccess )
		Default();
}
	
//-----------------------------------------------------------------------------------------------
// Draw the themed border of the window.

void CTreeListCtrl::OnNcPaint()
{
	if( ! m_hTheme )
	{
		Default();
		return;
	}

	CWindowDC dc( this );

	//Clip the DC so that we only draw on the non-client area
	RECT rcBorder;
	GetWindowRect( &rcBorder );
	rcBorder.right -= rcBorder.left; rcBorder.bottom -= rcBorder.top;
	rcBorder.left = rcBorder.top = 0;

	RECT rcClient; memcpy(&rcClient, &rcBorder, sizeof(RECT));
	rcClient.left += m_rcClientPos.left;
	rcClient.top += m_rcClientPos.top;
	rcClient.right -= m_rcClientPos.right;
	rcClient.bottom -= m_rcClientPos.bottom;
	dc.ExcludeClipRect( rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

	// Draw parent background to support translucent control.
	if( ::IsThemeBackgroundPartiallyTransparent( m_hTheme, TVP_TREEITEM, TREIS_NORMAL ) )
		::DrawThemeParentBackground( GetSafeHwnd(), dc, &rcBorder);
	// Draw the background itself.
	int nState = TREIS_NORMAL;
	if( ! IsWindowEnabled() )
		nState = TREIS_DISABLED;	
	::DrawThemeBackground( m_hTheme, dc, TVP_TREEITEM, nState, &rcBorder, NULL );
}

//-----------------------------------------------------------------------------------------------
// Check if visual styles are enabled for this window and open the theme data.
// The application should delay-load "uxtheme.dll" so it can be run on Win2k too.

LRESULT CTreeListCtrl::VerifyThemeState( WPARAM, LPARAM )
{
	m_isThemeActive = false;

	// return if OS version < WinXP
	OSVERSIONINFO osVer = { sizeof(osVer) };  ::GetVersionEx( &osVer );
	if( ( osVer.dwMajorVersion << 8 | osVer.dwMinorVersion ) < 0x0501 )
		return 0;
	m_isThemeActive = ::IsThemeActive() != FALSE;

	bool usingTheme = m_hTheme != NULL;
	if( m_hTheme )
		::CloseThemeData( m_hTheme );
	m_hTheme = NULL;

	//First, check if the control is supposed to have a border
	if( usingTheme ||
		( GetStyle() & WS_BORDER ) || ( GetExStyle() & WS_EX_CLIENTEDGE ) )
	{
		if( m_isThemeActive )
		{
			//Remove the border style, we don't want the control to draw its own border
			if( GetStyle() & WS_BORDER )
				ModifyStyle( WS_BORDER, 0 );
			if( GetExStyle() & WS_EX_CLIENTEDGE )
				ModifyStyleEx( WS_EX_CLIENTEDGE, 0 );
			
			m_hTheme = ::OpenThemeData( GetSafeHwnd(), L"TREEVIEW" );
		}
	}

	//Recalculate the NC area and repaint the window
	SetWindowPos(NULL, NULL, NULL, NULL, NULL, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);

	return 0;
}

//=== END WinXP themed border implementation


