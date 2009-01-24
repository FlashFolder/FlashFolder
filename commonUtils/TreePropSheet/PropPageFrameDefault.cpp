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
*    ADD: Draw page headline with task dialog "main instruction" style
*********************************************************************/

#include "stdafx.h"
#include "PropPageFrameDefault.h"

#pragma warning(disable:4800) // convert BOOL to bool
#pragma warning(disable:4244) // convert to smaller type


namespace TreePropSheet
{


//uncomment the following line, if you don't have installed the
//new platform SDK
#define XPSUPPORT

#ifdef XPSUPPORT
#include <uxtheme.h>
#include <tmschema.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// class CPropPageFrameDefault
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPropPageFrameDefault, CWnd)
	//{{AFX_MSG_MAP(CPropPageFrameDefault)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CPropPageFrameDefault::CPropPageFrameDefault()
{
	OSVERSIONINFO verInfo = { sizeof(verInfo) };
	::GetVersionEx( &verInfo );
	m_osVer = ( verInfo.dwMajorVersion << 8 ) | verInfo.dwMinorVersion;
}


CPropPageFrameDefault::~CPropPageFrameDefault()
{
	if (m_Images.GetSafeHandle())
		m_Images.DeleteImageList();
}


/////////////////////////////////////////////////////////////////////
// Overridings

BOOL CPropPageFrameDefault::Create(DWORD dwWindowStyle, const RECT &rect, CWnd *pwndParent, UINT nID)
{
	return CWnd::Create(
		AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, AfxGetApp()->LoadStandardCursor(IDC_ARROW), GetSysColorBrush(COLOR_3DFACE)),
		_T("Page Frame"),
		dwWindowStyle, rect, pwndParent, nID);
}


CWnd* CPropPageFrameDefault::GetWnd()
{
	return static_cast<CWnd*>(this);
}


void CPropPageFrameDefault::SetCaption(LPCTSTR lpszCaption, HICON hIcon /*= NULL*/)
{
	CPropPageFrame::SetCaption(lpszCaption, hIcon);

	// build image list
	if (m_Images.GetSafeHandle())
		m_Images.DeleteImageList();
	if (hIcon)
	{
		ICONINFO	ii;
		if (!GetIconInfo(hIcon, &ii))
			return;

		CBitmap	bmMask;
		bmMask.Attach(ii.hbmMask);
		if (ii.hbmColor) DeleteObject(ii.hbmColor);

		BITMAP	bm;
		bmMask.GetBitmap(&bm);

		if (!m_Images.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR32|ILC_MASK, 0, 1))
			return;

		if (m_Images.Add(hIcon) == -1)
			m_Images.DeleteImageList();
	}
}


CRect CPropPageFrameDefault::CalcMsgArea()
{
	CRect	rect;
	GetClientRect(rect);

	bool isThemed = false;
	if( m_osVer >= 0x0501 )
		isThemed = ::IsThemeActive();
	if( isThemed )
	{
		if( HTHEME hTheme = ::OpenThemeData(m_hWnd, L"Tab") )
		{
			CRect	rectContent;
			CDC		*pDc = GetDC();
			::GetThemeBackgroundContentRect(hTheme, pDc->m_hDC, TABP_PANE, 0, rect, rectContent);
			ReleaseDC(pDc);
			::CloseThemeData(hTheme);
			
			if (GetShowCaption())
				rectContent.top = rect.top+GetCaptionHeight()+1;
			rect = rectContent;
		}
	}
	else
	{
		if (GetShowCaption())
			rect.top+= GetCaptionHeight()+1;
	}	
	return rect;
}


CRect CPropPageFrameDefault::CalcCaptionArea()
{
	CRect	rect;
	GetClientRect(rect);

	bool isThemed = false;
	if( m_osVer >= 0x0501 )
		isThemed = ::IsThemeActive();
	if( isThemed )
	{
		if( HTHEME hTheme = ::OpenThemeData(m_hWnd, L"Tab") )
		{
			CRect	rectContent;
			CDC		*pDc = GetDC();
			::GetThemeBackgroundContentRect(hTheme, pDc->m_hDC, TABP_PANE, 0, rect, rectContent);
			ReleaseDC(pDc);
			::CloseThemeData(hTheme);
			
			if (GetShowCaption())
				rectContent.bottom = rect.top+GetCaptionHeight();
			else
				rectContent.bottom = rectContent.top;

			rect = rectContent;
		}		
	}
	else
	{
		if (GetShowCaption())
			rect.bottom = rect.top+GetCaptionHeight();
		else
			rect.bottom = rect.top;
	}

	return rect;
}

void CPropPageFrameDefault::DrawCaption(CDC *pDc, CRect rect, LPCTSTR lpszCaption, HICON hIcon)
{
	bool isThemed = false;
	if( m_osVer >= 0x0501 )
		isThemed = ::IsThemeActive();

	//--- draw background

	if( m_osVer < 0x0600 || ! isThemed )
	{
		COLORREF	clrLeft = ::GetSysColor( COLOR_HIGHLIGHT );
		COLORREF	clrRight = ::GetSysColor( COLOR_BTNFACE );	
		TRIVERTEX gVert[ 2 ] = { 
			rect.left, rect.top,     ( clrLeft & 0xFF ) << 8, clrLeft & 0xFF00, ( clrLeft & 0xFF0000 ) >> 8, 0,
			rect.right, rect.bottom, ( clrRight & 0xFF ) << 8, clrRight & 0xFF00, ( clrRight & 0xFF0000 ) >> 8, 0
		};
		GRADIENT_RECT gRect = { 0, 1 };
		pDc->GradientFill( gVert, 2, &gRect, 1, GRADIENT_FILL_RECT_H );
	}

	CDialog* pParent = (CDialog*) GetParent();
	CRect rcMargin( 0, 0, 2, 1 );
	pParent->MapDialogRect( rcMargin );

	//--- draw icon

	if (hIcon && m_Images.GetSafeHandle() && m_Images.GetImageCount() == 1)
	{
		IMAGEINFO	ii;
		m_Images.GetImageInfo(0, &ii);
		CPoint		pt(3, rect.CenterPoint().y - (ii.rcImage.bottom-ii.rcImage.top)/2);
		m_Images.Draw(pDc, 0, pt, ILD_TRANSPARENT);
		rect.left+= (ii.rcImage.right-ii.rcImage.left) + 3;
	}

	//--- draw text

	rect.left += rcMargin.Width();

	if( m_osVer < 0x0600 || ! isThemed )
	{
		int	nBkStyle = pDc->SetBkMode(TRANSPARENT);

		// get correctly themed font
		LOGFONT lf;
		GetSysMessageFont( &lf, GetSafeHwnd() );
		lf.lfWeight = FW_BOLD;
		CFont font;
		font.CreateFontIndirect( &lf );
		CFont* pOldFont = pDc->SelectObject( &font );

		COLORREF clrPrev = pDc->SetTextColor( GetSysColor( COLOR_HIGHLIGHTTEXT ) );

		pDc->DrawText(lpszCaption, rect, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

		pDc->SetTextColor(clrPrev);
		pDc->SetBkMode(nBkStyle);
		pDc->SelectObject(pOldFont);
	}
	else
	{
		// Vista: draw caption with the style of task dialog "main instruction".
	
		HTHEME hTheme = ::OpenThemeData( *this, L"TEXTSTYLE" );
		
		CRect margins( 0, 0, 6, 6 ); ::MapDialogRect( *this, margins );
		rect.top += margins.bottom;
		rect.left += margins.right;		
		
		::DrawThemeText( hTheme, *pDc, TEXT_MAININSTRUCTION, 0, lpszCaption, -1, 
			DT_SINGLELINE, 0, rect );
		
		::CloseThemeData( hTheme );	
	}
}

/////////////////////////////////////////////////////////////////////
// message handlers

void CPropPageFrameDefault::OnPaint() 
{
	CPaintDC dc(this);
	Draw(&dc);	
}


BOOL CPropPageFrameDefault::OnEraseBkgnd(CDC* pDC) 
{
	bool isThemed = false;
	if( m_osVer >= 0x0501 )
		isThemed = ::IsThemeActive();
	if( isThemed )
	{
		HTHEME	hTheme = ::OpenThemeData(m_hWnd, L"TREEVIEW");
		if (hTheme)
		{
			CRect	rect;
			GetClientRect(rect);
			::DrawThemeBackground(hTheme, pDC->m_hDC, 0, 0, rect, NULL);

			::CloseThemeData(hTheme);
		}
		return TRUE;
	}
	else
	{
		BOOL res = CWnd::OnEraseBkgnd(pDC);
		CRect rc; GetClientRect( rc );
		pDC->DrawEdge( rc, EDGE_ETCHED, BF_BOTTOM );
		return res;
	}
}



} //namespace TreePropSheet