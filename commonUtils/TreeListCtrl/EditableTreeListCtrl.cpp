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
#include "EditableTreeListCtrl.h"
#include "gdiUtils.h"

using namespace std;

//-----------------------------------------------------------------------------------------------
// Helper stuff

inline bool IsShiftPressed() 
	{ return (GetKeyState(VK_SHIFT) & (1 << (sizeof(SHORT)*8-1))) != 0; }
inline bool IsCtrlPressed()  
	{ return (GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8-1))) != 0; }
inline bool IsAltPressed()  
	{ return (GetKeyState(VK_MENU) & (1 << (sizeof(SHORT)*8-1))) != 0; }

// "unique" timer ID to prevent conflicts with derived classes 
const UINT ID_DRAGTIMER = 0xF39A8473;

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP( CEditableTreeListCtrl, CTreeListCtrl )
	ON_NOTIFY_REFLECT_EX( TVN_BEGINDRAG, OnBeginDrag )
	ON_NOTIFY_REFLECT_EX( TVN_DELETEITEM, OnDeleteItem )
	ON_NOTIFY_REFLECT_EX( TVN_KEYDOWN, OnKeyDown )
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	ON_WM_TIMER()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CEditableTreeListCtrl::OnInitDialog()
{
	CTreeListCtrl::OnInitDialog();

	InsertDummyItem( TVI_ROOT );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

BOOL CEditableTreeListCtrl::OnBeginDrag( NMHDR* pnm_, LRESULT* pRes )
{
	LPNMTREEVIEW pnm = reinterpret_cast<LPNMTREEVIEW>( pnm_ );

	m_dragSelection.RemoveAll(); 
	GetTree().GetSelectedList( m_dragSelection );

	if( m_dragSelection.GetCount() == 1 )
	{
		// dont' allow the dummy item contained in an empty folder, to be dragged
		HTREEITEM hItem = m_dragSelection.GetAt( m_dragSelection.GetHeadPosition() );
		if( m_dummyItems.find( hItem ) != m_dummyItems.end() )
			return FALSE;
	}

	SetCapture(); 
	m_isDragging = true;
	m_isAutoScrolling = false;
	m_hLastInsertPos = NULL;
	m_hLastInsertParent = NULL;
	m_hLastHighlight = NULL;
	m_hLastButton = NULL;
	m_lastButtonTime = 0;

	if( ! m_hDragCursor )
		m_hDragCursor = LoadCursor( NULL, IDC_ARROW );
	m_hNoDragCursor = ::LoadCursor( NULL, IDC_NO );
	SetCursor( m_hDragCursor );

	SetTimer( ID_DRAGTIMER, 100, NULL );

	return FALSE; // enable parent to handle message
}

//-----------------------------------------------------------------------------------------------
// handle dragging

void CEditableTreeListCtrl::OnMouseMove( UINT flags, CPoint pt )
{
	CTreeListCtrl::OnMouseMove( flags, pt );

	if( ! m_isDragging )
		return;

	CTreeListCtrl_tree& treeCtrl = GetTree();

	ClientToScreen( &pt );
	treeCtrl.ScreenToClient( &pt );

	CRect rcClient; treeCtrl.GetClientRect( rcClient ); 

	HCURSOR hCursor = m_hDragCursor;

	//--- get insert position by enumerating visible items

	HTREEITEM hFirstVisibleItem = treeCtrl.GetFirstVisibleItem();
	HTREEITEM hItem = hFirstVisibleItem;
	HTREEITEM hItemPrev = NULL;
	if( hItem )
		hItemPrev = treeCtrl.GetNextItem( hItem, TVGN_PREVIOUS );

	HTREEITEM hItemInsertParent = NULL;
	HTREEITEM hItemInsert = NULL;
	HTREEITEM hItemInsertNext = NULL;
	bool isFolderInsert = false;

	while( hItem )
	{
		HTREEITEM hItemNext = treeCtrl.GetNextVisibleItem( hItem );

		CRect rcOver;
		treeCtrl.GetItemRect( hItem, rcOver, FALSE );

		if( pt.x > rcOver.left && pt.x <= rcOver.right &&
			pt.y > rcOver.top && pt.y <= rcOver.bottom )
		{
			int ycenter = rcOver.top + ( rcOver.bottom - rcOver.top ) / 2;

			//--- determine new insert marker position

			hItemInsertParent = NULL;
			hItemInsert = hItem;
			hItemInsertNext = hItemNext;

			isFolderInsert = false;

			if( GetTree().ItemHasChildren( hItem ) )
			{
				// mouse cursor is over a parent item --> check if user wants to append to sub-tree

				CRect rc = rcOver;
				rc.top += rcOver.Height() / 4;
				rc.bottom -= rcOver.Height() / 4;

				CRect rcText;
				treeCtrl.GetItemRect( hItem, rcText, TRUE );

				if( pt.y > rc.top && pt.y < rc.bottom &&
					pt.x > rcText.left && pt.x < rcText.right )
				{
					hItemInsertParent = hItem;
					hItemInsert = TVI_LAST;
					isFolderInsert = true;
				}
			}

			if( ! isFolderInsert )
			{
				if( pt.y < ycenter ) 
				{
					// insert pos is before the current mouse-over item
					hItemInsert = hItemPrev ? hItemPrev : TVI_FIRST;
					hItemInsertNext = hItem;
				}

				if( hItemInsert != TVI_FIRST ) 
				{
					HTREEITEM hItemChild = treeCtrl.GetChildItem( hItemInsert );
					if( hItemInsertNext && hItemInsertNext == hItemChild )
					{
						// insert pos is the first child item of a tree
						hItemInsertParent = hItemInsert;				
						hItemInsert = TVI_FIRST;
					}
					else
					{
						hItemInsertParent = treeCtrl.GetParentItem( hItemInsert );		

						// decide if insert pos is after the whole sub-tree
						if( ! hItemInsertNext ||
							treeCtrl.GetParentItem( hItemInsertNext ) != hItemInsertParent )
						{
							CRect rc;
							treeCtrl.GetItemRect( hItemInsert, &rc, TRUE );
							if( pt.y > rc.bottom )
								if( hItemInsertNext )
								{
									hItemInsertParent = treeCtrl.GetParentItem( hItemInsertNext );
									hItemInsert = treeCtrl.GetNextItem( hItemInsertNext, TVGN_PREVIOUS );
								}
						}
					}				
				}
			}

			// avoid dragging an item into a child item of itself (this would delete it)
			if( IsChildOf( hItemInsertParent, m_dragSelection ) )
			{
				hItemInsert = NULL;
				hItemInsertParent = NULL;
				hItemInsertNext = NULL;
			}

			break;
		}
		else if( ! hItemNext )
		{
			// mouse cursor is above first visible or below last visible item

			if( pt.y <= 0 )
			{
				hItemInsertParent = treeCtrl.GetParentItem( hFirstVisibleItem );
				hItemInsert = treeCtrl.GetNextItem( hFirstVisibleItem, TVGN_PREVIOUS );
				if( ! hItemInsert )
					hItemInsert = TVI_FIRST;
				hItemInsertNext = hFirstVisibleItem;
			}
			else
			{
				hItemInsert = hItem;
			}
		}
		
		hItemPrev = hItem;
		hItem = hItemNext;
	}

	//--- update insert mark

	// clear insert mark
	SetMyInsertMark( NULL );

	if( m_hLastHighlight )
	{
		// remove last folder insert marker
		CTreeListCtrl::ItemFormat fmt;
		fmt.flags = 0; 
		SetItemFormat( m_hLastHighlight, fmt );
	}

	if( isFolderInsert )
	{
		// highlight a folder name
		CTreeListCtrl::ItemFormat fmt;
		fmt.flags = CTreeListCtrl::FMT_HIGHLIGHTTEXT;
		SetItemFormat( hItem, fmt );
	}
	else
	{
		// set new insert mark
		CTreeListCtrl::InsertMarkPos insPos;
		insPos.hParent = hItemInsertParent;
		if( hItemInsertNext )
		{
			insPos.hItem = hItemInsertNext;
            insPos.isBefore = true;
		}
		else
		{
			insPos.hItem = hItemInsert;
		}
		SetMyInsertMark( &insPos );
	}
	treeCtrl.UpdateWindow();
	

	m_hLastInsertPos = hItemInsert;
	m_hLastInsertParent = hItemInsertParent;
	m_hLastHighlight = isFolderInsert ? hItem : NULL;

	SetCursor( hItemInsert ? m_hDragCursor : m_hNoDragCursor );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::OnLButtonUp( UINT flags, CPoint pt )
{
	CTreeListCtrl::OnLButtonUp( flags, pt );
	if( m_isDragging )
		ReleaseCapture();
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::OnCaptureChanged( CWnd* pWnd )
{
	CTreeListCtrl::OnCaptureChanged( pWnd );

	if( ! m_isDragging )
		return;
	m_isDragging = false;

	KillTimer( ID_DRAGTIMER );

	AutoRedraw redraw( GetTree() );

	//--- remove insert marker

	SetMyInsertMark( NULL );

	if( m_hLastHighlight )
	{
		CTreeListCtrl::ItemFormat fmt;
		fmt.flags = 0; 
		SetItemFormat( m_hLastHighlight, fmt );
	}

	//--- move selected items to new position

	if( m_hLastInsertPos )
	{
		//--- copy selected items
		TreeData treeData;
		CopyItemList( &treeData, m_dragSelection );

		//--- insert items
		GetTree().SelectAll( FALSE );
		HTREEITEM hInsertAfter = m_hLastInsertPos ? m_hLastInsertPos : TVI_FIRST;
		
		InsertItems( m_hLastInsertParent, hInsertAfter, treeData );

		//--- delete selected items
		DeleteItemList( m_dragSelection );

		m_hLastInsertPos = NULL;
	}
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::OnTimer( UINT_PTR id )
{
	if( id != ID_DRAGTIMER || ! m_isDragging )
		return;

	//--- auto-scroll the window during drag-n-drop

	CRect rcClient; GetTree().GetClientRect( rcClient );

	CPoint pt, ptClient;
	::GetCursorPos( &pt );
	ptClient = pt;
	GetTree().ScreenToClient( &ptClient );

	int h = rcClient.Height() / 7;
	if( ptClient.y < h )
	{
		GetTree().SendMessage( WM_VSCROLL, SB_LINEUP, NULL );
	}
	else if( ptClient.y > rcClient.Height() - h )
	{
		GetTree().SendMessage( WM_VSCROLL, SB_LINEDOWN, NULL );
	}


	//--- if cursor is above a parent item for some time during drag-n-drop, then expand the item

	UINT htFlags = 0;
	HTREEITEM hNewButton = GetTree().HitTest( ptClient, &htFlags );
	if( hNewButton && 
		( htFlags & ( TVHT_ONITEMBUTTON | TVHT_ONITEM ) ) &&
		GetTree().ItemHasChildren( hNewButton ) && 
		( GetTree().GetItemState( hNewButton, TVIS_EXPANDED ) & TVIS_EXPANDED ) == 0 )
	{
		if( hNewButton != m_hLastButton )
		{
			m_lastButtonTime = ::GetTickCount();
		}
		else
		{
			DWORD time = ::GetTickCount();
			if( time - m_lastButtonTime > 1500 )
			{
				GetTree().Expand( hNewButton, TVE_EXPAND );
				hNewButton = NULL;
			}
		}		
	}
	else
	{
		m_lastButtonTime = ::GetTickCount();
	}
	m_hLastButton = hNewButton;
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::Cut()
{
	m_clipboard.childs.clear();
	CTreeItemList selection;
	GetTree().GetSelectedList( selection );
	CopyItemList( &m_clipboard, selection );

	DeleteItemList( selection );	
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::Copy()
{
	m_clipboard.childs.clear();
	CTreeItemList selection;
	GetTree().GetSelectedList( selection );
	CopyItemList( &m_clipboard, selection );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::Paste()
{
	AutoRedraw redraw( GetTree() );

	HTREEITEM hParent = TVI_ROOT, hInsert = TVI_LAST;
	if( HTREEITEM hFocusedItem = GetTree().GetFocusedItem() )
	{
		hParent = GetTree().GetParentItem( hFocusedItem );
		hInsert = hFocusedItem;
	}
	GetTree().SelectAll( FALSE );
	InsertItems( hParent, hInsert, m_clipboard );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::Delete()
{
	CTreeItemList selection;
	GetTree().GetSelectedList( selection );
	DeleteItemList( selection );
}

//-----------------------------------------------------------------------------------------------

BOOL CEditableTreeListCtrl::OnKeyDown( NMHDR* pnm_, LRESULT* pResult )
{
	NMTVKEYDOWN* pnm = reinterpret_cast<NMTVKEYDOWN*>( pnm_ );

	bool isCtrl = IsCtrlPressed();
	bool isShift = IsShiftPressed();
	int ch = pnm->wVKey;
	int chl = tolower( pnm->wVKey );

	if( isCtrl && ( chl == 'c' || ch == VK_INSERT ) )
	{
		Copy();
	}
	else if( isCtrl && chl == 'x' || isShift && ch == VK_DELETE )
	{
		Cut();
	}
	else if( isCtrl && chl == 'v' || isShift && ch == VK_INSERT )
	{
		Paste();
	}
	else if( ch == VK_DELETE )
	{
		Delete();
	}

	return FALSE; // enable parent to handle message
}

//-----------------------------------------------------------------------------------------------

BOOL CEditableTreeListCtrl::OnDeleteItem( NMHDR* pnm_, LRESULT* pResult ) 
{
	*pResult = 0;
	NMTREEVIEW* pnm = reinterpret_cast<NMTREEVIEW*>( pnm_ );

	//--- delete data associated with this tree item
	m_folderItems.erase( pnm->itemOld.hItem );
	m_dummyItems.erase( pnm->itemOld.hItem );

	//--- if all child items are deleted, insert "-empty-" item, making it possible to 
	//    move items by keyboard into empty folders
	HTREEITEM hParent = GetTree().GetParentItem( pnm->itemOld.hItem );
	if( m_deleteItems.find( hParent ) == m_deleteItems.end() )
	{
		int childCount = 0;
		for( HTREEITEM hItem = GetTree().GetChildItem( hParent ); hItem && childCount < 2; 
			hItem = GetTree().GetNextItem( hItem, TVGN_NEXT ) )
			++childCount;

		if( childCount == 1 )
			InsertDummyItem( hParent );
	}

	return FALSE; // enable parent to handle message
}

//-----------------------------------------------------------------------------------------------

HTREEITEM CEditableTreeListCtrl::InsertItem( LPTVINSERTSTRUCT pti ) 
{ 
	// remove the dummy item if exists

	HTREEITEM hEmpty = GetTree().GetChildItem( pti->hParent );
	HTREEITEM hNewItem = GetTree().InsertItem( pti );

	if( m_dummyItems.find( hEmpty ) != m_dummyItems.end() )
		GetTree().DeleteItem( hEmpty );

	return hNewItem;
}

HTREEITEM CEditableTreeListCtrl::InsertItem( LPCTSTR lpszItem, int nImage, int nSelectedImage, 
					  HTREEITEM hParent, HTREEITEM hInsertAfter )
{ 
	TVINSERTSTRUCT tvi;
	tvi.hParent = hParent;
	tvi.hInsertAfter = hInsertAfter;
	tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.item.pszText = const_cast<LPTSTR>( lpszItem );
	tvi.item.iImage = nImage;
	tvi.item.iSelectedImage = nSelectedImage;
	return InsertItem( &tvi ); 
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::InsertDummyItem( HTREEITEM hParent )
{
	HTREEITEM hDummy = InsertItem( _T("-empty-"), -1, -1, hParent );
	CTreeListCtrl::ItemFormat fmt;
	fmt.flags = FMT_COLOR | FMT_ITALIC;
	fmt.textColor = ::GetSysColor( COLOR_GRAYTEXT );
	SetItemFormat( hDummy, fmt );

	m_dummyItems.insert( hDummy );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::SetItemIsFolder( HTREEITEM hItem )
{
	m_folderItems.insert( hItem );
	if( ! GetTree().ItemHasChildren( hItem ) )
		InsertDummyItem( hItem );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::CopyItemList( TreeData* pData, const CTreeItemList& items )
{
	CTreeListCtrl_tree& treeCtrl = GetTree();

	// Since we traverse the child items of the selected items, we must take care
	// to avoid redundant child items which are already contained in the selection.
	set< HTREEITEM > copiedItems;

	for( POSITION pos = items.GetHeadPosition(); pos; )
	{
		HTREEITEM hSelItem = items.GetNext( pos );
		if( copiedItems.find( hSelItem ) == copiedItems.end() && 
			m_dummyItems.find( hSelItem ) == m_dummyItems.end() )
		{
			copiedItems.insert( hSelItem );
			pData->childs.push_back( TreeData() );

			CopyItem( &pData->childs.back(), hSelItem );
			if( treeCtrl.ItemHasChildren( hSelItem ) )
				CopyChildren( &pData->childs.back(), &copiedItems, hSelItem ); 
		}
	}
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::CopyChildren( TreeData* pData, std::set<HTREEITEM>* pCopiedItems, HTREEITEM hItemRoot )
{
	CTreeListCtrl_tree& treeCtrl = GetTree();

	for( HTREEITEM hItem = treeCtrl.GetChildItem( hItemRoot ); hItem; 
	     hItem = treeCtrl.GetNextItem( hItem, TVGN_NEXT ) )
	{
		if( pCopiedItems->find( hItem ) == pCopiedItems->end() )
		{
			pCopiedItems->insert( hItem );

			pData->childs.push_back( TreeData() );

			CopyItem( &pData->childs.back(), hItem );

			// recursion
			if( treeCtrl.ItemHasChildren( hItem ) )
				CopyChildren( &pData->childs.back(), pCopiedItems, hItem );
		}
	}	
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::CopyItem( TreeData* pData, HTREEITEM hItem )
{
	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_IMAGE | TVIF_PARAM | TVIF_STATE;
	item.stateMask = TVIS_EXPANDED;
	GetTree().GetItem( &item );

	pData->iImage = item.iImage;
	pData->lParam = item.lParam;
	pData->isFolder = IsItemFolder( hItem );
	pData->isExpanded = ( item.state & TVIS_EXPANDED ) != 0;
	pData->isEmptyDummy = m_dummyItems.find( hItem ) != m_dummyItems.end();
	GetItemFormat( hItem, &pData->format );

	int colCount = GetColumnCount();
	pData->text.resize( colCount );
	for( int i = 0; i < colCount; ++i )
		pData->text[ i ] = GetItemText( hItem, i );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::InsertItems( 
	HTREEITEM hParent, HTREEITEM hInsertAfter, const TreeData& data )
{
	bool isExpanded = true;
	if( hParent && hParent != TVI_ROOT )
		isExpanded = ( GetTree().GetItemState( hParent, TVIS_EXPANDED ) & TVIS_EXPANDED ) != 0;
	
	InsertItems_worker( hParent, hInsertAfter, data, isExpanded );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::InsertItems_worker( 
	HTREEITEM hParent, HTREEITEM hInsertAfter, const TreeData& data, bool isParentExpanded )
{
	TVINSERTSTRUCT ins = { 0 };
	ins.hParent = hParent;
	ins.hInsertAfter = hInsertAfter;
	ins.item.mask = TVIF_IMAGE | TVIF_PARAM;

	for( int iChild = 0; iChild != data.childs.size(); ++iChild )
	{
		const TreeData& childData = data.childs[ iChild ];

		ins.item.iImage = childData.iImage;
		ins.item.lParam = childData.lParam;
		HTREEITEM hItem = InsertItem( &ins );

		if( childData.format.flags != 0 )
			SetItemFormat( hItem, childData.format );
					
		for( int i = 0; i != childData.text.size(); ++i )
			SetItemText( hItem, i, childData.text[ i ] );

		if( childData.isFolder )
			m_folderItems.insert( hItem );

		if( childData.isEmptyDummy )
			m_dummyItems.insert( hItem );

        if( ! childData.childs.empty() )
		{
			// recursion
			InsertItems_worker( hItem, TVI_FIRST, childData, childData.isExpanded && isParentExpanded );

			if( childData.isExpanded )
				GetTree().Expand( hItem, TVE_EXPAND );
		}

		// only select expanded child items
		if( isParentExpanded )
			GetTree().SetItemState( hItem, TVIS_SELECTED | TVIS_FOCUSED, TVIS_SELECTED | TVIS_FOCUSED );

		ins.hInsertAfter = hItem;
	}
}

//-----------------------------------------------------------------------------------------------
// Determine if hItem is a child of or equal to one of the items in list.

bool CEditableTreeListCtrl::IsChildOf( HTREEITEM hItem, const CTreeItemList& list )
{
	if( ! hItem )
		return false;
	do
	{
		for( POSITION pos = list.GetHeadPosition(); pos; )
		{
			HTREEITEM hListItem = list.GetNext( pos );
			if( hItem == hListItem )
				return true;
		}		
		hItem = GetTree().GetParentItem( hItem );	
	}
	while( hItem );

	return false;
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::DeleteItemList( const CTreeItemList& list )
{
	// Since deleting a tree item recursively deletes all childs, make sure that items are 
	// not delete more than once.
	set<HTREEITEM> deletedItems;

	// Save items to be deleted so we can temporary disable "dummy item" creation in OnDeleteItem() 
	for( POSITION pos = list.GetHeadPosition(); pos; )
		m_deleteItems.insert( list.GetNext( pos ) );

	// Delete the items
	for( POSITION pos = list.GetHeadPosition(); pos; )
		DeleteTree( &deletedItems, list.GetNext( pos ) );

	m_deleteItems.clear();
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::DeleteTree( set<HTREEITEM>* pDeletedItems, HTREEITEM hItemRoot )
{
	CTreeListCtrl_tree& treeCtrl = GetTree();

	if( pDeletedItems->find( hItemRoot ) != pDeletedItems->end() )
		return;

	vector<HTREEITEM> childs;

	for( HTREEITEM hItem = treeCtrl.GetChildItem( hItemRoot ); hItem; 
	     hItem = treeCtrl.GetNextItem( hItem, TVGN_NEXT ) )
	{
		// recursion - depth first
		if( treeCtrl.ItemHasChildren( hItem ) )
			DeleteTree( pDeletedItems, hItem );

		childs.push_back( hItem );
	}	

	for( int i = 0; i != childs.size(); ++i )
	{
		if( pDeletedItems->find( childs[ i ] ) == pDeletedItems->end() )
		{
			treeCtrl.DeleteItem( childs[ i ] );
			pDeletedItems->insert( childs[ i ] );
		}
	}
	treeCtrl.DeleteItem( hItemRoot );
	pDeletedItems->insert( hItemRoot );
}

//-----------------------------------------------------------------------------------------------

void CEditableTreeListCtrl::SetDragCursor( HINSTANCE hInstance, LPCTSTR resourceId )
{
	m_hDragCursor = ::LoadCursor( hInstance, resourceId );
}
