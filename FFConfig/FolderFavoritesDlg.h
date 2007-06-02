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
	CFolderFavoritesDlg(CWnd* pParent = NULL);   

	enum { IDD = IDD_FAVORITES };

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnLstFavs_ItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRdOwnFavorites();
	afx_msg void OnBnClickedRdTcFavorites();
	afx_msg void OnLstFavs_DeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEdTitle();
	afx_msg void OnEnChangeEdPath();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnRemove();
	afx_msg void OnLstFavs_KeyDown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnRevert();
	afx_msg void OnListFavs_DblClick(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

	void UpdateFavList( DirFavoritesSrc source = DFS_DEFAULT );
	void SelectItem( int sel );
	void UpdateSelItemEditControls();
	void SaveDialogSize();

	CDragListCtrl m_listFavs;
	CEditEx m_edTitle;
	CEditEx m_edPath;

	int m_sel;
};
