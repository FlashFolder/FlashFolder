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
 */
#pragma once

class CUpdateCheckDlg : public CResizableDlg
{
public:
	CUpdateCheckDlg(CWnd* pParent = NULL);   // Standardkonstruktor

	enum { IDD = IDD_UPDATECHECK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK() {}
	virtual BOOL PreTranslateMessage( MSG* pMsg );

	afx_msg LRESULT OnThreadNotify( WPARAM wp, LPARAM lp );
	afx_msg void OnBnClickedBtnDownload();
	DECLARE_MESSAGE_MAP()

private:
	bool ProcessDownloadedFile();

private:
	CDlgExpander m_expander;
	CToolTipCtrl m_tooltip;

	CString m_tempPath;
	TiXmlDocument m_rssDoc;
	int m_instVer[ 4 ];
	
	CDownloadThread m_thread;
	bool m_isCanceled;
};

