//*************************************************************************
// GroupCheck.cpp : implementation file
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

#include "stdafx.h"
#include "GroupCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupCheck

CGroupCheck::CGroupCheck()
{
	m_pGroupbox = NULL;
	m_bHideDisabled = FALSE;
}

CGroupCheck::~CGroupCheck()
{
}


BEGIN_MESSAGE_MAP(CGroupCheck, CButton)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	ON_MESSAGE( BM_SETCHECK, OnSetCheck )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupCheck message handlers

BOOL CGroupCheck::OnClicked() 
{
	CheckGroupboxControls();
	// Return FALSE to allow client to intercept the message
	return FALSE;
}

void CGroupCheck::SetCheck(int nCheck)
{
	CButton::SetCheck(nCheck);
	CheckGroupboxControls();
}

// Sets groupbox by Resource ID
void CGroupCheck::SetGroupbox(UINT nGroupboxID, BOOL bHideDisabled /*= FALSE*/)
{
	SetGroupbox((CButton*)GetParent()->GetDlgItem(nGroupboxID), bHideDisabled);
}

// Sets groupbox by a pointer to that
void CGroupCheck::SetGroupbox(CButton* pGroupbox, BOOL bHideDisabled /*= FALSE*/)
{
	m_bHideDisabled = bHideDisabled;
	m_pGroupbox = pGroupbox;
	
	// Clear the groupbox text
	pGroupbox->SetWindowText(_T(""));	

	// Sometimes the window size of the checkbox is much bigger than 
	// the text, let's trim it.
	CString strText;
	GetWindowText(strText);
	// Figure out how long the text really is
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CSize czText = dc.GetTextExtent(strText);
	dc.SelectObject(pOldFont);
	// Add some space for the checkbox and at the end
	czText.cx += 25;

	// Move the checkbox on top of the groupbox
	CRect rc;
	pGroupbox->GetWindowRect(rc);	
	GetParent()->ScreenToClient(rc);
	SetWindowPos(pGroupbox, rc.left+10, rc.top, czText.cx, czText.cy, 0);

	// Check controls within the groupbox based on the check state
	CheckGroupboxControls();
}

// Enable or disable controls inside the groupbox based on the checkbox state.
// If checkbox is unchecked and m_bHideDisabled is set TRUE, the groupbox 
// and all controls inside are hidden. For a tri-state checkbox in 
// immediate state, all controls inside are just disabled.
void CGroupCheck::CheckGroupboxControls()
{
	ASSERT(m_pGroupbox);

	int nCheck = GetCheck();
	CRect rcGroupbox;
	m_pGroupbox->GetWindowRect(rcGroupbox);

	// Get first child control
	CWnd* pWnd = GetParent()->GetWindow(GW_CHILD);
	
	CRect rcWnd, rcTest;

	while (pWnd)
	{
		pWnd->GetWindowRect(rcWnd);

		if (rcTest.IntersectRect(rcGroupbox, rcWnd) && pWnd != this && pWnd != m_pGroupbox)
		{
			pWnd->EnableWindow(nCheck == 1);
			if (m_bHideDisabled)
				pWnd->ShowWindow(nCheck ? SW_SHOW : SW_HIDE);
		}
		pWnd = pWnd->GetWindow(GW_HWNDNEXT);
	}
	if (m_bHideDisabled)
		m_pGroupbox->ShowWindow(nCheck ? SW_SHOW : SW_HIDE);
}

LRESULT CGroupCheck::OnSetCheck( WPARAM wp, LPARAM lp )
{
	LRESULT res = DefWindowProc( BM_SETCHECK, wp, lp );
	CheckGroupboxControls();
	return res;
}



