/********************************************************************
*
* Copyright (c) 2002 Sven Wiegand <mail@sven-wiegand.de>
*
* You can use this and modify this in any way you want,
* BUT LEAVE THIS HEADER INTACT.
*
* Redistribution is appreciated.
*
* Changes by zett42 (zett42 at users.sourceforge.net):
*    FIX: CTreePropSheet did not use DPI-independent metrics
*    FIX: Prop-page caption colors did not work with some XP themes
*    ADD: Allow different captions for tree and prop-page.
*    ADD: Vista - draw page headline with task dialog "main instruction" style
*********************************************************************/


#if !defined(AFX_PROPPAGEFRAMEDEFAULT_H__5C5B7AC9_2DF5_4E8C_8F5E_DE2CC04BBED7__INCLUDED_)
#define AFX_PROPPAGEFRAMEDEFAULT_H__5C5B7AC9_2DF5_4E8C_8F5E_DE2CC04BBED7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPageFrame.h"


namespace TreePropSheet
{


/**
An implementation of CPropPageFrame, that works well for Windows XP
styled systems and older windows versions (without themes).

@author Sven Wiegand
*/
class /*AFX_EXT_CLASS*/ CPropPageFrameDefault : public CWnd,
                                            public CPropPageFrame
{
// construction/destruction
public:
	CPropPageFrameDefault();
	virtual ~CPropPageFrameDefault();

// operations
public:

// overridings
public:
	virtual BOOL Create(DWORD dwWindowStyle, const RECT &rect, CWnd *pwndParent, UINT nID);
	virtual CWnd* GetWnd();
	virtual void SetCaption(LPCTSTR lpszCaption, HICON hIcon = NULL);
	
protected:
	virtual CRect CalcMsgArea();
	virtual CRect CalcCaptionArea();
	virtual void DrawCaption(CDC *pDc, CRect rect, LPCTSTR lpszCaption, HICON hIcon);

// Implementation helpers
protected:
	/**
	Returns TRUE if Windows XP theme support is available, FALSE 
	otherwise.
	*/
	BOOL ThemeSupport() const;

protected:
	//{{AFX_VIRTUAL(CPropPageFrameDefault)
	//}}AFX_VIRTUAL

// message handlers
protected:
	//{{AFX_MSG(CPropPageFrameDefault)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// attributes
protected:
	/** 
	Image list that contains only the current icon or nothing if there
	is no icon.
	*/
	CImageList m_Images;

private:
	DWORD m_osVer;
};


} //namespace TreePropSheet


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROPPAGEFRAMEDEFAULT_H__5C5B7AC9_2DF5_4E8C_8F5E_DE2CC04BBED7__INCLUDED_