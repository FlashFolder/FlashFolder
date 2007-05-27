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

class CExcludesDlg : public CDialog
{
public:
	enum { IDD = IDD_EXCLUDE };

	CExcludesDlg(CWnd* pParent = NULL);   // Standardkonstruktor

	void SetTitle( const CString& s ) { m_title = s; }
	void SetDescr( const CString& s ) { m_descr = s; }

	void SetStrings( const std::vector<CString>& list )  { m_list = list; }
	void GetStrings( std::vector<CString>* pList ) const { *pList = m_list; }

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedBtnAdd();
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CString> m_list;
	CEdit m_edList;

	CString m_title, m_descr;
};
