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

class CFolderFavoritesDlg : public CResizableDlg
{
public:
	CFolderFavoritesDlg(CWnd* pParent = NULL, int selectItemId = -1 );   

	enum { IDD = IDD_FAVORITES };

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTree_SelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTree_InsertItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTree_DeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEdCommand();
	afx_msg void OnEnChangeEdTargetPath();
	afx_msg void OnEnChangeEdIconPath();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnAddDivider();
	afx_msg void OnBnClickedBtnAddSubmenu();
	afx_msg void OnBnClickedBtnRemove();
	afx_msg void OnBnClickedBtnImport();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnTargetbrowse();
	afx_msg void OnBnClickedBtnRevert();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point );
	afx_msg void OnCbnEditchangeCbTitle();	
	afx_msg void OnCbnSelendokCbTitle();
	DECLARE_MESSAGE_MAP()

	void LoadFavorites();
	void LoadFavorites_worker( HTREEITEM hParent, HTREEITEM hInsertAfter, 
		const FavoritesList& favs, size_t& iItem, bool bSelectInsertedItems = false );
	void SaveFavorites();
	void SaveFavorites_worker( FavoritesList& favs, HTREEITEM hParent );

	void UpdateSelItemEditControls();
	void SaveDialogSize();
	
	void UpdateTitleSuggestions();

	CEditableTreeListCtrl m_tree;

	CCacheImageList m_treeIcons;

	CEditEx m_edTitle;
	CComboBox m_cbTitle;
	CEditEx m_edPath;
	CEditEx m_edTargetPath;

	HTREEITEM m_hSelItem;

	stdext::hash_map< HTREEITEM, CString > m_iconPathes;
	CString GetItemIconPath(  HTREEITEM hItem ) const;

	stdext::hash_map< CString, int > m_iconIDs;

	bool m_isOwned;
	
	int m_selectItemId;
	HTREEITEM m_selectTreeItem;
};
