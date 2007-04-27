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

class CFFConfigDlg : public CDialog
{
public:
	CFFConfigDlg(CWnd* pParent = NULL);	// standard constructor

	enum { IDD = IDD_FFCONFIG_DIALOG };

private:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnNonResizableExcl();
	DECLARE_MESSAGE_MAP()

	CGroupCheck m_chkCFO, m_chkCFD, m_chkMSO, m_chkCOW;
	CComboBox m_cbCFO_pos, m_cbCFD_pos, m_cbMSO_pos, m_cbCOW_pos;

	std::vector<CString> m_cfoNonResizableExcludes;
};
