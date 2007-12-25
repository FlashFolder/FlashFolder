// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2007 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "StdAfx.h"
//#include "resource.h"			//if you defined some IDS_MSGBOX_xxxx this include is needed!
#include "messagebox.h"

/// CMessageBoxEx margins in dialog units
enum CMessageBoxMargins
{
	MESSAGEBOX_BUTTONMARGIN = 5,
	MESSAGEBOX_ICONMARGIN = 7,
	MESSAGEBOX_BORDERMARGINX = 7,
	MESSAGEBOX_BORDERMARGINY = 7,
	MESSAGEBOX_TEXTBUTTONMARGIN = 14,
	MESSAGEBOX_BUTTONCHECKMARGIN = 5,
	MESSAGEBOX_BUTTONX = 5,
	MESSAGEBOX_BUTTONY = 3
};

CMessageBoxEx::CMessageBoxEx(void)
{
	m_hIcon = NULL;
	m_uButton1Ret = ID_BUTTON1;
	m_uButton2Ret = ID_BUTTON2;
	m_uButton3Ret = ID_BUTTON3;
	m_uCancelRet = 0;
	m_bShowCheck = FALSE;
	m_bDestroyIcon = FALSE;
	
	m_buttonId1 = ID_BUTTON1;
	m_buttonId2 = ID_BUTTON2;
	m_buttonId3 = ID_BUTTON3;
}

CMessageBoxEx::~CMessageBoxEx(void)
{
	if (m_bDestroyIcon)
		::DestroyIcon(m_hIcon);
}

UINT CMessageBoxEx::ShowCheck(HWND hWnd, UINT nMessage, UINT nCaption, int nDef, int nCancel, LPCTSTR icon, 
		UINT nButton1, UINT nButton2, UINT nButton3, LPCTSTR lpRegistry, UINT nCheckMessage/* = NULL*/)
{
	CString sButton1;
	CString sButton2;
	CString sButton3;
	CString sMessage;
	CString sCaption;
	CString nCheckMsg;
	sButton1.LoadString(nButton1);
	sButton2.LoadString(nButton2);
	sButton3.LoadString(nButton3);
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	nCheckMsg.LoadString(nCheckMessage);
	return CMessageBoxEx::ShowCheck(hWnd, sMessage, sCaption, nDef, nCancel, icon, sButton1, sButton2, sButton3, lpRegistry, nCheckMsg);
}

UINT CMessageBoxEx::ShowCheck(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, int nDef, int nCancel, LPCTSTR icon, 
		LPCTSTR lpButton1, LPCTSTR lpButton2, LPCTSTR lpButton3, LPCTSTR lpRegistry, LPCTSTR lpCheckMessage/* = NULL*/)
{
	//check the registry if we have to show the box or just return with the last used returnvalue
	//this would be the case if the user pressed "do not show again".
	DWORD dwRetVal;
	HKEY hKey;
	CString path;
#ifdef XMESSAGEBOX_APPREGPATH
	path = XMESSAGEBOX_APPREGPATH;
#else
	path = "Software\\";
	path += AfxGetAppName();
#endif
	if (RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_EXECUTE, &hKey)==ERROR_SUCCESS)
	{
		int size = sizeof(dwRetVal);
		DWORD type;
		if (RegQueryValueEx(hKey, lpRegistry, NULL, &type, (BYTE*) &dwRetVal,(LPDWORD) &size)==ERROR_SUCCESS)
		{
			ASSERT(type==REG_DWORD);
			RegCloseKey(hKey);
			return (UINT)dwRetVal;			//return with the last saved value
		}
		else
		{
			RegCloseKey(hKey);
		}
	}

	CMessageBoxEx box;
	box.m_bShowCheck = TRUE;
	box.m_sRegistryValue = lpRegistry;
	if (lpCheckMessage == NULL)
	{
#ifndef IDS_MSGBOX_DONOTSHOWAGAIN
		box.m_sCheckbox = _T("do not show again");
#else
		CString m_i18l;
		m_i18l.LoadString(IDS_MSGBOX_DONOTSHOWAGAIN);
		box.m_sCheckbox = m_i18l;
#endif
	}
	else
		box.m_sCheckbox = lpCheckMessage;
	box.m_sButton1 = lpButton1;
	box.m_sButton2 = lpButton2;
	box.m_sButton3 = lpButton3;
	box.m_hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	if (box.m_hIcon == NULL)
		box.m_hIcon = (HICON)::LoadImage(NULL, icon, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
	else
		box.m_bDestroyIcon = TRUE;
	if (!IsWindow(hWnd))
		hWnd = NULL;
	return box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, nDef, nCancel);	
}

UINT CMessageBoxEx::Show(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, int nDef, int nCancel, LPCTSTR icon, 
	LPCTSTR lpButton1, LPCTSTR lpButton2/* = NULL*/, LPCTSTR lpButton3/* = NULL*/)
{
	CMessageBoxEx box;
	box.m_sButton1 = lpButton1;
	box.m_sButton2 = lpButton2;
	box.m_sButton3 = lpButton3;
	box.m_hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	if (box.m_hIcon == NULL)
		box.m_hIcon = (HICON)::LoadImage(NULL, icon, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
	else
		box.m_bDestroyIcon = TRUE;
	if (!IsWindow(hWnd))
		hWnd = NULL;
	return box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, nDef, nCancel );
}

UINT CMessageBoxEx::Show(HWND hWnd, UINT nMessage, UINT nCaption, int nDef, int nCancel, LPCTSTR icon, 
	UINT nButton1, UINT nButton2, UINT nButton3)
{
	CString sButton1;
	CString sButton2;
	CString sButton3;
	CString sMessage;
	CString sCaption;
	sButton1.LoadString(nButton1);
	sButton2.LoadString(nButton2);
	sButton3.LoadString(nButton3);
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	return CMessageBoxEx::Show(hWnd, sMessage, sCaption, nDef, nCancel, icon, sButton1, sButton2, sButton3);
}


UINT CMessageBoxEx::ShowCheck(HWND hWnd, UINT nMessage, UINT nCaption, UINT uType, LPCTSTR lpRegistry, UINT nCheckMessage)
{
	CString sMessage;
	CString sCaption;
	CString sCheckMsg;
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	sCheckMsg.LoadString(nCheckMessage);
	return CMessageBoxEx::ShowCheck(hWnd, sMessage, sCaption, uType, lpRegistry, sCheckMsg);
}

UINT CMessageBoxEx::ShowCheck(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, UINT uType, LPCTSTR lpRegistry, LPCTSTR lpCheckMessage)
{
	//check the registry if we have to show the box or just return with the last used returnvalue
	//this would be the case if the user pressed "do not show again".
	DWORD dwRetVal;
	HKEY hKey;
	CString path;
#ifdef XMESSAGEBOX_APPREGPATH
	path = XMESSAGEBOX_APPREGPATH;
#else
	path = "Software\\";
	path += AfxGetAppName();
#endif
	if (RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_EXECUTE, &hKey)==ERROR_SUCCESS)
	{
		int size = sizeof(dwRetVal);
		DWORD type;
		if (RegQueryValueEx(hKey, lpRegistry, NULL, &type, (BYTE*) &dwRetVal,(LPDWORD) &size)==ERROR_SUCCESS)
		{
			ASSERT(type==REG_DWORD);
			RegCloseKey(hKey);
			return (UINT)dwRetVal;			//return with the last saved value
		}
		else
		{
			RegCloseKey(hKey);
		}
	}

	CMessageBoxEx box;
	box.m_bShowCheck = TRUE;
	box.m_sRegistryValue = lpRegistry;
	if (lpCheckMessage == NULL)
	{
#ifndef IDS_MSGBOX_DONOTSHOWAGAIN
		box.m_sCheckbox = _T("do not show again");
#else
		CString m_i18l;
		m_i18l.LoadString(IDS_MSGBOX_DONOTSHOWAGAIN);
		box.m_sCheckbox = m_i18l;
#endif
	}
	else
		box.m_sCheckbox = lpCheckMessage;
	if (!IsWindow(hWnd))
		hWnd = NULL;
	return box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, box.FillBoxStandard(uType), 0 );
}

UINT CMessageBoxEx::Show(HWND hWnd, UINT nMessage, UINT nCaption, UINT uType, LPCTSTR sHelpPath)
{
	CString sMessage;
	CString sCaption;
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);
	return CMessageBoxEx::Show(hWnd, sMessage, sCaption, uType, sHelpPath);
}

UINT CMessageBoxEx::Show(HWND hWnd, LPCTSTR lpMessage, LPCTSTR lpCaption, UINT uType, LPCTSTR sHelpPath)
{
	CMessageBoxEx box;
	
	if (!IsWindow(hWnd))
		hWnd = NULL;
	if (sHelpPath)
		box.SetHelpPath(sHelpPath);
	return box.GoModal(CWnd::FromHandle(hWnd), lpCaption, lpMessage, box.FillBoxStandard(uType), 0);
}

UINT CMessageBoxEx::Show(HWND hWnd, UINT nMessage, UINT nCaption, UINT uType, UINT nHelpID)
{
	CMessageBoxEx box;
	CString sMessage;
	CString sCaption;
	sMessage.LoadString(nMessage);
	sCaption.LoadString(nCaption);

	if (!IsWindow(hWnd))
		hWnd = NULL;
	box.SetHelpID(nHelpID);

	return box.GoModal(CWnd::FromHandle(hWnd), sCaption, sMessage, box.FillBoxStandard(uType), 0);
}

int CMessageBoxEx::FillBoxStandard(UINT uType)
{
	int ret = 1;
	m_uType = uType;
	m_uCancelRet = IDCANCEL;
	//load the icons according to uType
	switch (uType & 0xf0)
	{
	case MB_ICONEXCLAMATION:
		m_hIcon = (HICON)::LoadImage(NULL, IDI_EXCLAMATION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONEXCLAMATION);
		break;
	case MB_ICONASTERISK:
		m_hIcon = (HICON)::LoadImage(NULL, IDI_ASTERISK, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONASTERISK);
		break;
	case MB_ICONQUESTION:
		m_hIcon = (HICON)::LoadImage(NULL, IDI_QUESTION, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONQUESTION);
		break;
	case MB_ICONHAND:
		m_hIcon = (HICON)::LoadImage(NULL, IDI_HAND, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
		::MessageBeep(MB_ICONHAND);
		break;
	}
	//set up the button texts
	switch (uType & 0xf)
	{
	case MB_ABORTRETRYIGNORE:
#ifndef IDS_MSGBOX_ABORT
		m_sButton1 = "Abort";
#else
		m_i18l.LoadString(IDS_MSGBOX_ABORT);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDABORT;
#ifndef IDS_MSGBOX_RETRY
		m_sButton2 = "Retry";
#else
		m_i18l.LoadString(IDS_MSGBOX_RETRY);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDRETRY;
#ifndef IDS_MSGBOX_IGNORE
		m_sButton3 = "Ignore";
#else
		m_i18l.LoadString(IDS_MSGBOX_IGNORE);
		m_sButton3 = m_i18l;
#endif
		m_uButton3Ret = IDIGNORE;
		break;
	case MB_CANCELTRYCONTINUE:
#ifndef IDS_MSGBOX_CANCEL
		m_sButton1 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDCANCEL;
#ifndef IDS_MSGBOX_TRYAGAIN
		m_sButton2 = "Try Again";
#else
		m_i18l.LoadString(IDS_MSGBOX_TRYAGAIN);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDTRYAGAIN;
#ifndef IDS_MSGBOX_CONTINUE
		m_sButton3 = "Continue";
#else
		m_i18l.LoadString(IDS_MSGBOX_CONTINUE);
		m_sButton3 = m_i18l;
#endif
		m_uButton3Ret = IDCONTINUE;
		break;
	case MB_OKCANCEL:
#ifndef IDS_MSGBOX_OK
		m_sButton1 = "Ok";
#else
		m_i18l.LoadString(IDS_MSGBOX_OK);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDOK;
#ifndef IDS_MSGBOX_CANCEL
		m_sButton2 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDCANCEL;
		break;
	case MB_RETRYCANCEL:
#ifndef IDS_MSGBOX_RETRY
		m_sButton1 = "Retry";
#else
		m_i18l.LoadString(IDS_MSGBOX_RETRY);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDRETRY;
#ifndef IDS_MSGBOX_CANCEL
		m_sButton2 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDCANCEL;
		break;
	case MB_YESNO:
#ifndef IDS_MSGBOX_YES
		m_sButton1 = "Yes";
#else
		m_i18l.LoadString(IDS_MSGBOX_YES);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDYES;
#ifndef IDS_MSGBOX_NO
		m_sButton2 = "No";
#else
		m_i18l.LoadString(IDS_MSGBOX_NO);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDNO;
		break;
	case MB_YESNOCANCEL:
#ifndef IDS_MSGBOX_YES
		m_sButton1 = "Yes";
#else
		m_i18l.LoadString(IDS_MSGBOX_YES);
		m_sButton1 = m_i18l;
#endif
		m_uButton1Ret = IDYES;
#ifndef IDS_MSGBOX_NO
		m_sButton2 = "No";
#else
		m_i18l.LoadString(IDS_MSGBOX_NO);
		m_sButton2 = m_i18l;
#endif
		m_uButton2Ret = IDNO;
#ifndef IDS_MSGBOX_CANCEL
		m_sButton3 = "Cancel";
#else
		m_i18l.LoadString(IDS_MSGBOX_CANCEL);
		m_sButton3 = m_i18l;
#endif
		m_uButton3Ret = IDCANCEL;
		break;
	case MB_OK:
	default:
#ifndef IDS_MSGBOX_OK
		m_sButton1 = "Ok";
#else
		m_i18l.LoadString(IDS_MSGBOX_OK);
		m_sButton1 = m_i18l;
#endif
	}
	//now set the default button
	switch (uType & 0xf00)
	{
	case MB_DEFBUTTON2:
		ret = 2;
		break;
	case MB_DEFBUTTON3:
		ret = 3;
		break;
	}
	// do we need to add a help button?
	if (uType & MB_HELP)
	{
		CString sHelpText;
#ifndef IDS_MSGBOX_HELP
		sHelpText = _T("Help");
#else
		m_i18l.LoadString(IDS_MSGBOX_HELP);
		sHelpText = m_i18l;
#endif
		if (m_sButton2.IsEmpty())
		{
			m_sButton2 = sHelpText;
			m_uButton2Ret = IDHELP;
		}
		else if (m_sButton3.IsEmpty())
		{
			m_sButton3 = sHelpText;
			m_uButton3Ret = IDHELP;
		}
	}
	return ret;
}

UINT CMessageBoxEx::GoModal(CWnd * pWnd, const CString& title, const CString& msg, int nDefaultButton, int nCancelButton )
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0));
    memcpy(&m_LogFont, &(ncm.lfMessageFont), sizeof(LOGFONT));

	//the problem with the LOGFONT lfHeight is that it is not in pixels,
	//but the dialog template needs the height in pixels.
	//We need to convert those values first:
	CDC * pDC;
	if (pWnd)
		pDC = pWnd->GetDC();
	else
		pDC = GetDesktopWindow()->GetDC();
	int pix = -MulDiv(m_LogFont.lfHeight, 72, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY));

	if( nCancelButton == 1 )
		m_buttonId1 = IDCANCEL;
	else if( nCancelButton == 2 )
		m_buttonId2 = IDCANCEL;
	else if( nCancelButton == 3 )
		m_buttonId3 = IDCANCEL;	 
	
	CDlgTemplate dialogTemplate = CDlgTemplate(title, WS_CAPTION | DS_CENTER | DS_MODALFRAME | WS_SYSMENU,
		0, 0, 0, 0, m_LogFont.lfFaceName, pix);
	dialogTemplate.AddButton(_T("Button1"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | ((nDefaultButton == 1) ? BS_DEFPUSHBUTTON : 0), 0,
		2 + 3, 62, 56, 13, m_buttonId1 );
	dialogTemplate.AddButton(_T("Button2"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | ((nDefaultButton == 2) ? BS_DEFPUSHBUTTON : 0), 0,
		2 + 3, 62, 56, 13, m_buttonId2 );
	dialogTemplate.AddButton(_T("Button3"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | ((nDefaultButton == 3) ? BS_DEFPUSHBUTTON : 0), 0,
		2 + 3, 62, 56, 13, m_buttonId3 );
	dialogTemplate.AddButton(_T("Checkbox"), WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 0,
		0, 0, 0, 0, ID_CHECKBOX);
		
	if( nCancelButton > 0 )
		m_uCancelRet = IDCANCEL;	
	m_nDefButton = nDefaultButton;
	m_sMessage = msg;
	InitModalIndirect(dialogTemplate, pWnd);

	return (UINT)DoModal();
}

void CMessageBoxEx::SetRegistryValue(const CString& sValue, DWORD value)
{
	CString path;
#ifdef XMESSAGEBOX_APPREGPATH
	path = XMESSAGEBOX_APPREGPATH;
#else
	path = "Software\\";
	path += AfxGetAppName();
#endif
	DWORD disp;
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disp)!=ERROR_SUCCESS)
	{
		return;
	}
	RegSetValueEx(hKey, sValue, 0, REG_DWORD,(const BYTE*) &value, sizeof(value));
	RegCloseKey(hKey);
}

CSize CMessageBoxEx::GetTextSize(const CString& str)
{
	CRect rect;
	GetWindowRect(&rect);

	CDC * pDC = GetDC();

	CDC memDC;
	CBitmap bitmap;
	memDC.CreateCompatibleDC(pDC);
	bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	//get the minimum size of the rectangle of the tooltip
	CSize sz = DrawHTML(&memDC, rect, str, m_LogFont, TRUE);

	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	bitmap.DeleteObject();

	ReleaseDC(pDC);

	return sz;
}

CSize CMessageBoxEx::GetIconSize(HICON hIcon)
{
	ICONINFO ii;
	CSize sz (0, 0);

	if (hIcon != NULL)
	{
		//get icon dimensions
		::ZeroMemory(&ii, sizeof(ICONINFO));
		if (::GetIconInfo(hIcon, &ii))
		{
			sz.cx = (DWORD)(ii.xHotspot * 2);
			sz.cy = (DWORD)(ii.yHotspot * 2);
			//release icon mask bitmaps
			if(ii.hbmMask)
				::DeleteObject(ii.hbmMask);
			if(ii.hbmColor)
				::DeleteObject(ii.hbmColor);
		}
	}
	m_szIcon = sz;
	return sz;
}

CSize CMessageBoxEx::GetButtonSize()
{
	// size of standard button according to Windows UI guidelines
	CRect minBtnSize( 0, 0, 50, 14 );
	MapDialogRect( minBtnSize );
	
	CSize sz;
	int nButtons = 0;		//number of buttons - 1

	SetDlgItemText(m_buttonId1, m_sButton1);
	SetDlgItemText(m_buttonId2, m_sButton2);
	//GetDlgItem(IDC_MESSAGEBOX_BUTTON2)->SendMessage(BM_SETSTYLE, BS_DEFPUSHBUTTON, 1);
	SetDlgItemText(m_buttonId3, m_sButton3);
	SetDlgItemText(ID_CHECKBOX, m_sCheckbox);
	
	CSize sz1 = GetTextSize(m_sButton1);
	CSize sz2 = GetTextSize(m_sButton2);
	CSize sz3 = GetTextSize(m_sButton3);

	sz1.cx += 2*m_MESSAGEBOX_BUTTONX;
	sz1.cx = max( sz1.cx, minBtnSize.right );
	sz1.cy = minBtnSize.bottom;

	if (sz2.cx)
	{
		sz2.cx += 2*m_MESSAGEBOX_BUTTONX;
		sz2.cx = max( sz2.cx, minBtnSize.right );
		sz2.cy = minBtnSize.bottom;
		nButtons++;
	}
	if (sz3.cx)
	{
		sz3.cx += 2*m_MESSAGEBOX_BUTTONX;
		sz3.cx = max( sz3.cx, minBtnSize.right );
		sz3.cy = minBtnSize.bottom;
		nButtons++;
	}
	
	GetDlgItem(m_buttonId1)->MoveWindow(0, 0, sz1.cx, sz1.cy);
	GetDlgItem(m_buttonId2)->MoveWindow(0, 0, sz2.cx, sz2.cy);
	GetDlgItem(m_buttonId3)->MoveWindow(0, 0, sz3.cx, sz3.cy);


	sz.cx = sz1.cx + sz2.cx + sz3.cx + (nButtons * m_MESSAGEBOX_BUTTONMARGIN);
	sz.cy = max(sz1.cy, sz2.cy);
	sz.cy = max(sz.cy, sz3.cy);
	m_szButtons = sz;
	if (m_bShowCheck)
	{
		CSize szCheck = GetTextSize(m_sCheckbox);
		szCheck.cx += 2*GetSystemMetrics(SM_CXMENUCHECK);
		szCheck.cy += 2*m_MESSAGEBOX_BUTTONY;
		sz.cx = max(sz.cx, szCheck.cx);
		sz.cy += szCheck.cy + m_MESSAGEBOX_BUTTONCHECKMARGIN;
		GetDlgItem(ID_CHECKBOX)->MoveWindow(0, 0, szCheck.cx, szCheck.cy);
	}
	m_szAllButtons = sz;
	return sz;
}

BEGIN_MESSAGE_MAP(CMessageBoxEx, CDialog)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CMessageBoxEx::OnPaint()
{
	CPaintDC dc(this); // device context for painting


	CRect rect;
	CRect drawrect;
	GetClientRect(&rect);
	GetClientRect(&drawrect);

	//create a memory device-context. This is done to help reduce
	//screen flicker, since we will paint the entire control to the
	//off screen device context first.
	CDC memDC;
	CBitmap bitmap;
	memDC.CreateCompatibleDC(&dc);
	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap); 
	
	memDC.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dc, 0,0, SRCCOPY);

	memDC.SetBkMode(TRANSPARENT);
	memDC.SetBkColor(GetSysColor(COLOR_WINDOW));
	memDC.SetTextColor(GetSysColor(COLOR_WINDOWTEXT)); 

	//OnDrawBackground();
	drawrect.DeflateRect(m_MESSAGEBOX_BORDERMARGINX, m_MESSAGEBOX_BORDERMARGINY);
	if (m_hIcon != NULL)
	{
		DrawIconEx(memDC.m_hDC, drawrect.left, drawrect.top + 
			((drawrect.Height() - m_szAllButtons.cy - m_MESSAGEBOX_TEXTBUTTONMARGIN - m_szIcon.cy) / 2), 
			m_hIcon, m_szIcon.cx, m_szIcon.cy, 0, NULL, DI_NORMAL);

		drawrect.left += m_szIcon.cx + m_MESSAGEBOX_ICONMARGIN; 
	}


	DrawHTML(&memDC, drawrect, m_sMessage, m_LogFont);
	

	//Copy the memory device context back into the original DC.
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &memDC, 0,0, SRCCOPY);
	
	//Cleanup resources.
	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	bitmap.DeleteObject(); 


}

void CMessageBoxEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if (IsPointOverALink(point))
	{
		m_Cursor.SetCursor(IDC_HAND);
	}
	else
	{
		m_Cursor.Restore();	
	}

	__super::OnMouseMove(nFlags, point);
}

void CMessageBoxEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsPointOverALink(point))
	{
		CString url = GetLinkForPoint(point);
		ShellExecute(NULL, _T("open"), url, NULL,NULL, 0);
	}

	__super::OnLButtonUp(nFlags, point);
}

BOOL CMessageBoxEx::OnCommand( WPARAM wp, LPARAM lp )
{
	if( __super::OnCommand( wp, lp ) )
		return TRUE;
	WPARAM msg = wp >> 16;
	if( msg == BN_CLICKED )
	{
		WPARAM idCtl = wp & 0xFFFF;
		if(      idCtl == m_buttonId1 )
		{
			OnButton1();
			return TRUE;
		}
		else if( idCtl == m_buttonId2 )
		{
			OnButton2();
			return TRUE;
		}
		else if( idCtl == m_buttonId3 )
		{
			OnButton3();
			return TRUE;
		}
	}	
	return FALSE;
} 

void CMessageBoxEx::OnButton1()
{
	if (GetDlgItem(ID_CHECKBOX)->SendMessage(BM_GETCHECK, 0, 0)==BST_CHECKED)
		SetRegistryValue(m_sRegistryValue, m_uButton1Ret);
	EndDialog(m_uButton1Ret);
}

void CMessageBoxEx::OnButton2()
{
	if (GetDlgItem(ID_CHECKBOX)->SendMessage(BM_GETCHECK, 0, 0)==BST_CHECKED)
		SetRegistryValue(m_sRegistryValue, m_uButton2Ret);
	if ((m_uButton2Ret == IDHELP)&&(!m_sHelpPath.IsEmpty()))
	{
		typedef HWND (WINAPI* FPHH)(HWND, LPCWSTR, UINT, DWORD);
		FPHH pHtmlHelp=NULL; // Function pointer
		HINSTANCE hInstHtmlHelp = LoadLibrary(_T("HHCtrl.ocx"));
		HWND hHelp = NULL;
		if (hInstHtmlHelp != NULL)
		{
			(FARPROC&)pHtmlHelp = GetProcAddress(hInstHtmlHelp, "HtmlHelpW");
			if (pHtmlHelp)
				hHelp = pHtmlHelp(m_hWnd, (LPCTSTR)m_sHelpPath, HH_DISPLAY_TOPIC, NULL);
		}
		if (hHelp == NULL)
			::MessageBox(m_hWnd, _T("could not show help file"), _T("Help"), MB_ICONERROR);
	}
	else if (m_uButton2Ret == IDHELP)
	{
		OnHelp();
	}
	else
		EndDialog(m_uButton2Ret);
}

void CMessageBoxEx::OnButton3()
{
	if (GetDlgItem(ID_CHECKBOX)->SendMessage(BM_GETCHECK, 0, 0)==BST_CHECKED)
		SetRegistryValue(m_sRegistryValue, m_uButton3Ret);
	if ((m_uButton3Ret == IDHELP)&&(!m_sHelpPath.IsEmpty()))
	{
		typedef HWND (WINAPI* FPHH)(HWND, LPCWSTR, UINT, DWORD);
		FPHH pHtmlHelp=NULL; // Function pointer
		HINSTANCE hInstHtmlHelp = LoadLibrary(_T("HHCtrl.ocx"));
		HWND hHelp = NULL;
		if (hInstHtmlHelp != NULL)
		{
			(FARPROC&)pHtmlHelp = GetProcAddress(hInstHtmlHelp, "HtmlHelpW");
			if (pHtmlHelp)
				hHelp = pHtmlHelp(m_hWnd, (LPCTSTR)m_sHelpPath, HH_DISPLAY_TOPIC, NULL);
		}
		if (hHelp == NULL)
			::MessageBox(m_hWnd, _T("could not show help file"), _T("Help"), MB_ICONERROR);
	}
	else if (m_uButton3Ret == IDHELP)
	{
		OnHelp();
	}
	else
		EndDialog(m_uButton3Ret);
}

void CMessageBoxEx::OnCancel()
{
	if (m_uCancelRet == IDCANCEL)
		EndDialog(m_uCancelRet);
	//__super::OnCancel();
}

BOOL CMessageBoxEx::OnInitDialog()
{
	__super::OnInitDialog();
	
	// translate margins from dialog units to pixels
	CRect rcMarg;
	rcMarg = CRect( 0, 0, MESSAGEBOX_BUTTONMARGIN, 1 ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_BUTTONMARGIN = rcMarg.right;
	rcMarg = CRect( 0, 0, MESSAGEBOX_ICONMARGIN, 1 ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_ICONMARGIN = rcMarg.right;
	rcMarg = CRect( 0, 0, MESSAGEBOX_BORDERMARGINX, 1 ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_BORDERMARGINX = rcMarg.right;
	rcMarg = CRect( 0, 0, 1, MESSAGEBOX_BORDERMARGINY ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_BORDERMARGINY = rcMarg.bottom;
	rcMarg = CRect( 0, 0, 1, MESSAGEBOX_TEXTBUTTONMARGIN ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_TEXTBUTTONMARGIN = rcMarg.bottom;
	rcMarg = CRect( 0, 0, 1, MESSAGEBOX_BUTTONCHECKMARGIN ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_BUTTONCHECKMARGIN = rcMarg.bottom;
	rcMarg = CRect( 0, 0, MESSAGEBOX_BUTTONX, 1 ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_BUTTONX = rcMarg.right;
	rcMarg = CRect( 0, 0, 1, MESSAGEBOX_BUTTONY ); MapDialogRect( rcMarg );
	m_MESSAGEBOX_BUTTONY = rcMarg.bottom;
	
	CRect rect(0, 0, 0, 0);

	//determine the required size of the messagebox
	CSize szText = GetTextSize(m_sMessage);
	CSize szIcon = GetIconSize(m_hIcon);
	CSize szButtons = GetButtonSize();

	CSize szIconText;
	szIconText.cx = szText.cx + szIcon.cx + ((szIcon.cx == 0) ? m_MESSAGEBOX_ICONMARGIN : (2*m_MESSAGEBOX_ICONMARGIN));
	szIconText.cy = max(szIcon.cy, szText.cy);

	rect.right = max(szButtons.cx, szIconText.cx);
	rect.right += 2*GetSystemMetrics(SM_CXBORDER);
	rect.right += 2*m_MESSAGEBOX_BORDERMARGINX;
	rect.bottom = szIconText.cy;
	rect.bottom += szButtons.cy;
	rect.bottom += 2*m_MESSAGEBOX_BORDERMARGINY + m_MESSAGEBOX_TEXTBUTTONMARGIN;
	rect.bottom += GetSystemMetrics(SM_CYCAPTION);
	rect.bottom += 2*GetSystemMetrics(SM_CYBORDER);

	MoveWindow(rect);
	CenterWindow();
	
	GetClientRect(rect);
	
	//now size and position the buttons as we need them
	ASSERT(!m_sButton1.IsEmpty());		//at least the first button must be there!
	if (m_sButton2.IsEmpty())
	{
		//only one button
		CRect rt;
		GetDlgItem(m_buttonId1)->GetWindowRect(rt);
		ScreenToClient(rt);
		rt.MoveToX( rect.right - m_szButtons.cx - m_MESSAGEBOX_BORDERMARGINX );
		rt.MoveToY( rect.bottom - m_MESSAGEBOX_BORDERMARGINY - m_szButtons.cy );
		GetDlgItem(m_buttonId1)->MoveWindow(rt);
		//hide the other two buttons
		GetDlgItem(m_buttonId2)->ShowWindow(SW_HIDE);
		GetDlgItem(m_buttonId3)->ShowWindow(SW_HIDE);
	}
	else if (m_sButton3.IsEmpty())
	{
		//two buttons
		CRect rt1;
		CRect rt2;
		GetDlgItem(m_buttonId1)->GetWindowRect(rt1);
		ScreenToClient(rt1);
		GetDlgItem(m_buttonId2)->GetWindowRect(rt2);
		ScreenToClient(rt2);
		rt1.MoveToX( rect.right - m_szButtons.cx - m_MESSAGEBOX_BORDERMARGINX );
		rt1.MoveToY(rect.bottom - m_MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		rt2.MoveToX(rt1.right + m_MESSAGEBOX_BUTTONMARGIN);
		rt2.MoveToY(rect.bottom - m_MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		GetDlgItem(m_buttonId1)->MoveWindow(rt1);
		GetDlgItem(m_buttonId2)->MoveWindow(rt2);
		//hide the third button
		GetDlgItem(m_buttonId3)->ShowWindow(SW_HIDE);
	}
	else
	{
		//three buttons
		CRect buttonrect;
		GetDlgItem(m_buttonId1)->GetWindowRect(buttonrect);
		CRect rt1;
		CRect rt2;
		CRect rt3;
		GetDlgItem(m_buttonId1)->GetWindowRect(rt1);
		ScreenToClient(rt1);
		GetDlgItem(m_buttonId2)->GetWindowRect(rt2);
		ScreenToClient(rt2);
		GetDlgItem(m_buttonId3)->GetWindowRect(rt3);
		ScreenToClient(rt3);
		rt1.MoveToX( rect.right - m_szButtons.cx - m_MESSAGEBOX_BORDERMARGINX );
		rt1.MoveToY(rect.bottom - m_MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		rt2.MoveToX(rt1.right + m_MESSAGEBOX_BUTTONMARGIN);
		rt2.MoveToY(rect.bottom - m_MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		rt3.MoveToX(rt2.right + m_MESSAGEBOX_BUTTONMARGIN);
		rt3.MoveToY(rect.bottom - m_MESSAGEBOX_BORDERMARGINY - m_szButtons.cy);
		GetDlgItem(m_buttonId1)->MoveWindow(rt1);
		GetDlgItem(m_buttonId2)->MoveWindow(rt2);
		GetDlgItem(m_buttonId3)->MoveWindow(rt3);
	}
	if (m_bShowCheck)
	{
		CRect rt;
		GetDlgItem(ID_CHECKBOX)->GetWindowRect(rt);
		ScreenToClient(rt);
		rt.MoveToX(rect.left + m_MESSAGEBOX_BORDERMARGINX/*+ ((rect.Width() - szButtons.cx)/2)*/);
		rt.MoveToY(rect.bottom - m_MESSAGEBOX_BORDERMARGINY - szButtons.cy);
		GetDlgItem(ID_CHECKBOX)->MoveWindow(rt);
		GetDlgItem(ID_CHECKBOX)->ShowWindow(SW_SHOW);
	}
	else
		GetDlgItem(ID_CHECKBOX)->ShowWindow(SW_HIDE);

	if (m_nDefButton == 1)
		GotoDlgCtrl( GetDlgItem(m_buttonId1) );
	else if (m_nDefButton == 2)
		GotoDlgCtrl( GetDlgItem(m_buttonId2) );
	else if (m_nDefButton == 3)
		GotoDlgCtrl( GetDlgItem(m_buttonId3) );

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CMessageBoxEx::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case 'C':
			{
				if (GetAsyncKeyState(VK_CONTROL)&0x8000)
				{
					CStringA sClipboard = CStringA(m_sMessage);
					if (OpenClipboard())
					{
						EmptyClipboard();
						HGLOBAL hClipboardData;
						hClipboardData = GlobalAlloc(GMEM_DDESHARE, sClipboard.GetLength()+1);
						char * pchData;
						pchData = (char*)GlobalLock(hClipboardData);
						strcpy_s(pchData, sClipboard.GetLength()+1, (LPCSTR)sClipboard);
						GlobalUnlock(hClipboardData);
						SetClipboardData(CF_TEXT,hClipboardData);
						CloseClipboard();
					}
					return TRUE;
				}
			}
			break;
		case VK_ESCAPE:
			{
				switch (m_uType & 0xf)
				{
				case MB_ABORTRETRYIGNORE:
					EndDialog(m_uButton1Ret);
					break;
				case MB_CANCELTRYCONTINUE:
					EndDialog(m_uButton1Ret);
					break;
				case MB_OKCANCEL:
					EndDialog(m_uButton2Ret);
					break;
				case MB_RETRYCANCEL:
					EndDialog(m_uButton2Ret);
					break;
				case MB_YESNO:
					EndDialog(m_uButton2Ret);
					break;
				case MB_YESNOCANCEL:
					EndDialog(m_uButton3Ret);
					break;
				}
			}
			break;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}







