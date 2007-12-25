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

class CAddFavoriteDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddFavoriteDlg)

public:
	CAddFavoriteDlg( CWnd* pParent, const CString& path, const CString& targetPath = _T("") );

	enum { IDD = IDD_ADD_FAVORITE };

private:
	virtual void DoDataExchange(CDataExchange* pDX);    
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	
	bool Save();
	
private:
	CString m_path, m_targetPath;
	CComboBox m_cbTitle;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedEditMenu();
};
