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

class CImportDlg : public CDialog
{
	DECLARE_DYNAMIC(CImportDlg)

public:
	CImportDlg(CWnd* pParent = NULL);   // Standardkonstruktor

	enum { IDD = IDD_IMPORT };

	CString GetTcIniPath() const { return m_enteredPath; }
	bool GetReplaceExisting() const { return m_replaceExisting; }

private:
	// overrides CDialog
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	// overrides CDialog
	BOOL OnInitDialog();
	// overrides CDialog
	void OnOK();

	afx_msg void OnBnClickedCheckInstalled();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnEnChangeEdit1();
	DECLARE_MESSAGE_MAP()

	CString m_installedIniPath, m_enteredPath;
	bool m_replaceExisting;
};

//----------------------------------------------------------------------------------------------------

void GetTcFavorites( FavoritesList* pFavs, CString tcIniPath );

