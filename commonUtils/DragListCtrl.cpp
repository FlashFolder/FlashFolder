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

#include "stdafx.h"
#include "DragListCtrl.h"

//---------------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP( CDragListCtrl, CListCtrl )
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnLvnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()	
END_MESSAGE_MAP()

//---------------------------------------------------------------------------------------------------------

void CDragListCtrl::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// Set dragging to ON
	m_Dragging = TRUE;

	// Store item where dragging begins
	m_DragItem = pNMLV->iItem;

	// Get mouse capture on the list control
	SetCapture();

	*pResult = 0;
}

//---------------------------------------------------------------------------------------------------------

void CDragListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// Were we dragging?
	if(m_Dragging) {	
		m_Dragging = FALSE;
		// Release mouse capture
		ReleaseCapture();
	}
	CListCtrl::OnLButtonUp(nFlags, point);
}

//---------------------------------------------------------------------------------------------------------

void CDragListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// Are we dragging?
	if(m_Dragging)
	{
		// Disable list drawing for less flickering
		SetRedraw(FALSE);

		// Now find the item where the mouse cursor is
		UINT flags=0;
		UINT index = HitTest( point, &flags );

		// No valid item found? Perhaps the mouse is outside the list
		if(index==-1)
		{
			int top = GetTopIndex();
			int	last = top + GetCountPerPage();
			// Mouse is under the listview, so pretend it's over the last item
			// in view
			if(flags & LVHT_BELOW) index=last;
			else 
				// Mouse is above the listview, so pretend it's over the top item in
				// view - 1
				if(flags & LVHT_ABOVE) index=top-1;
		}

		// Do we have a valid item now?
		if(index!=-1) {
			// calculate the offset between the two items
			int offset = index-m_DragItem;
			// Is it not the same item?
			if(offset != 0) {
				// Do we have a multiple selection?
				UINT selectedcount = GetSelectedCount();
				// Create an array of selected items (could use CArray here)
				UINT *selected = new UINT[selectedcount];
				UINT i = 0;
				// Add all selected items to this array
				POSITION pos = GetFirstSelectedItemPosition();
				while(pos) 
					selected[i++]=GetNextSelectedItem(pos);
				// Now we are going to move the selected items
				for(i=0;i<selectedcount;i++){
					// If we are moving the selection downward, we'll start
					// with the last one and iterate up. Else start with
					// the first one and iterate down.
					int iterator = (offset>0) ? selectedcount-i-1 : i;
					// Now get the position of the first selected item
					int oldpos = selected[iterator];
					// Calculate the new position
					int newpos = oldpos+offset;
					// Is the new position outsize the list's boundaries? break
					if(newpos<0 || newpos>=GetItemCount()) break;
					// Unselect the item
					SetItemState(oldpos, 0, LVIS_SELECTED);
					// No we keep swapping items until the selected
					// item reaches the new position
					if(offset>0) {
						// Going down
						for(int j=oldpos;j<newpos;j++) 
							SwapRows(j,j+1);
					}else {
						// Going up
						for(int j=oldpos;j>newpos;j--) 
							SwapRows(j,j-1);
					}
					// Make sure the newposition is in view
					EnsureVisible(newpos,TRUE);
					// Select it again
					SetItemState(newpos, LVIS_SELECTED, LVIS_SELECTED);
				}
				// Free the array
				delete [] selected;
				// Set the dragging item to the current index position,
				// so we can start over again
				m_DragItem=index;
			}
		}
		// Enable drawing in the listview again
		SetRedraw(TRUE);
	}
	CListCtrl::OnMouseMove(nFlags, point);
}

//---------------------------------------------------------------------------------------------------------

BOOL CDragListCtrl::SwapRows(UINT row1,UINT row2)
{
	// In this function we need to swap two rows,
	// Here it does some mangling with the listview's item texts/image/userdata
	// If you have a virtual list view you can swap it's items here

	CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem(0);
	int columncount = pHeader->GetItemCount();

	for(int i=0;i<columncount;i++) {
		CString string1 = GetItemText(row1,i);
		CString string2 = GetItemText(row2,i);
		SetItemText(row1,i,string2);
		SetItemText(row2,i,string1);
	}

	LVITEM item1;
	ZeroMemory(&item1,sizeof(LVITEM));
	item1.iItem = row1;
	item1.mask = LVIF_IMAGE | LVIF_PARAM;
	GetItem(&item1);
	item1.iItem = row2;

	LVITEM item2;
	ZeroMemory(&item2,sizeof(LVITEM));
	item2.iItem = row2;
	item2.mask = LVIF_IMAGE | LVIF_PARAM;
	GetItem(&item2);
	item2.iItem = row1;

	SetItem(&item1);
	SetItem(&item2);

	return 1;
}
