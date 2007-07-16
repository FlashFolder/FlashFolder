//*************************************************************************
// GroupCheck.h : header file
//
// By Ming Liu <ming.liu@utoronto.ca> on 17-Aug-2002
// Copyright (c) 2002. All Rights Reserved.
//
// You are free to use/modify this code but leave this header intact.
// Please let me know of any bugs/mods/improvements that you have 
// found/implemented and I will fix/incorporate them into this file. 
//
// This file is provided "as is" with no expressed or implied warranty.
// It's free, so don't hassle me about it.
//
// DESCRIPTION:
// This class makes a checkbox associated with a groupbox. The controls inside
// the groupbox can be enabled/disabled or hidden based on the checkbox state.
// - The checkbox is moved to the top left corner of the groupbox and resized 
//	 properly.
// - All controls within the groupbox are enabled when checkbox is checked, and
//   disabled when checkbox is unchecked.
// - You can optionally hide the controls inside the groupbox when unchecked.
//*************************************************************************
#if !defined(AFX_GROUPCHECK_H__E402446F_B22B_11D6_ACA4_0080C6EB5BBF__INCLUDED_)
#define AFX_GROUPCHECK_H__E402446F_B22B_11D6_ACA4_0080C6EB5BBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GroupCheck.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGroupCheck window

class CGroupCheck : public CButton
{
// Construction
public:
	CGroupCheck();

	void SetCheck(int nCheck);

	// Sets groupbox by Resource ID
	void SetGroupbox(UINT nGroupboxID, BOOL bHideDisabled = FALSE);
	// Sets groupbox by a pointer to that
	void SetGroupbox(CButton* pGroupbox, BOOL bHideDisabled = FALSE);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupCheck)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGroupCheck();

	// Generated message map functions
protected:
	void CheckGroupboxControls();

	CButton*	m_pGroupbox;
	BOOL		m_bHideDisabled;
	//{{AFX_MSG(CGroupCheck)
	afx_msg BOOL OnClicked();
	afx_msg LRESULT OnSetCheck( WPARAM wp, LPARAM lp );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPCHECK_H__E402446F_B22B_11D6_ACA4_0080C6EB5BBF__INCLUDED_)
