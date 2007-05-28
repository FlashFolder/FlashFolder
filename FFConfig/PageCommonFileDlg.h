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

#include "AutoPropertyPage.h"

//-----------------------------------------------------------------------------------------------

class CPageCommonFileDlg : public CAutoPropertyPage
{
public:
	typedef CAutoPropertyPage base;

	enum { IDD = IDD_PAGE_COMMON_FILEDLG };

	CPageCommonFileDlg();

	virtual void ReadProfile( const Profile& profile );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnBnClickedBtnExcludes();
	afx_msg void OnBnClickedBtnNonresizableExcludes();
	afx_msg void OnBnClickedChkEnable();
	DECLARE_MESSAGE_MAP()

private:
	CComboBox m_cbPos;
	std::vector<CString> m_excludes;
	std::vector<CString> m_nonResizableExcludes;
	bool m_bReadDefaults;
};