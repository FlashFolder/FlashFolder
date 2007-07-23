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

#include "treelistctrl.h"
#include <hash_set>

//-----------------------------------------------------------------------------------------------
/// \brief A tree-list-control where the items can be rearranged by using either drag-n-drop
/// or copy-n-paste.

class CEditableTreeListCtrl : public CTreeListCtrl
{
public:
	CEditableTreeListCtrl() :
		m_isDragging( false )
	{}

	/// Mark an item to be a "folder" (meaning it can contain other items)
	void SetItemIsFolder( HTREEITEM hItem );
	/// Check if an item is a "folder" (if it can contain other items, even if it is currently empty)
	bool IsItemFolder( HTREEITEM hItem ) const
		{ return m_folderItems.find( hItem ) != m_folderItems.end(); }

	/// Check if an item is a "dummy" (the only one in an empty folder)
	bool IsItemDummy( HTREEITEM hItem ) const
		{ return m_dummyItems.find( hItem ) != m_dummyItems.end(); }

	/// Check if an item has "real" children, not just the dummy item (the only one in an empty folder)
	bool HasRealChildren( HTREEITEM hItem ) const 
	{
		if( HTREEITEM hChild = GetTree().GetChildItem( hItem ) )
			if( ! IsItemDummy( hChild ) )
				return true;
		return false;
	}

	/// Delete items, taking care of duplicates and dummy-items.
	void DeleteItemList( const CTreeItemList& list );
	/// Delete a sub-tree taking care of dummy-items.
	void DeleteTree( std::set<HTREEITEM>* pDeletedItems, HTREEITEM hItem );

	/// Specifiy drag cursor resource.
	void SetDragCursor( HINSTANCE hInstance, LPCTSTR resourceId );

protected:
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnBeginDrag( NMHDR* pnm_, LRESULT* pRes );
	afx_msg void OnMouseMove( UINT flags, CPoint pt );
	afx_msg void OnLButtonUp( UINT flags, CPoint pt );
	afx_msg void OnCaptureChanged( CWnd* pWnd );
	afx_msg BOOL OnDeleteItem( NMHDR* pnm_, LRESULT* pResult );
	afx_msg BOOL OnKeyDown( NMHDR* pnm_, LRESULT* pResult );
	afx_msg void OnTimer( UINT_PTR id );
	DECLARE_MESSAGE_MAP()

private:
	// Tree-structure for copy-n-paste of tree items 
	struct TreeData
	{
		std::vector<CString> text;
		int iImage;
		LPARAM lParam;
		bool isFolder;
		bool isExpanded;
		bool isEmptyDummy;
		ItemFormat format;

		std::vector<TreeData> childs;

		TreeData() : 
			iImage( 0 ),
			lParam( 0 ),
			isFolder( false ),
			isExpanded( false ),
			isEmptyDummy( false) 
		{}
	};
	void CopyItemList( TreeData* pData, const CTreeItemList& list );
	void CopyChildren( TreeData* pData, std::set<HTREEITEM>* pCopiedItems, HTREEITEM hItemRoot );
	void CopyItem( TreeData* pData, HTREEITEM hItem );
	void InsertItems( HTREEITEM hParent, HTREEITEM hInsertAfter, const TreeData& data );
	void InsertItems_worker( HTREEITEM hParent, HTREEITEM hInsertAfter, const TreeData& data,
	                  bool isParentExpanded );
	bool IsChildOf( HTREEITEM hItem, const CTreeItemList& list );
	void InsertDummyItem( HTREEITEM hParent );

	// overrides CTreeListCtrl
	virtual HTREEITEM OnInsertItem( const TVINSERTSTRUCT& tvi );

private:
	bool m_isDragging;
	bool m_isAutoScrolling;
	HTREEITEM m_hLastInsertPos;
	HTREEITEM m_hLastInsertParent;
	HTREEITEM m_hLastHighlight;
	HTREEITEM m_hLastButton;
	DWORD m_lastButtonTime;
	CTreeItemList m_dragSelection;
	TreeData m_clipboard;
	stdext::hash_set<HTREEITEM> m_folderItems;
	stdext::hash_set<HTREEITEM> m_dummyItems;
	stdext::hash_set<HTREEITEM> m_deleteItems;
	HCURSOR m_hDragCursor, m_hNoDragCursor;
};
