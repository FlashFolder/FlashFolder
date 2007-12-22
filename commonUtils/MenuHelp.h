/****************************************************************************************
* Author : Simon Wang
*	Created time : 2003.09
*	home : http://www.winmsg.com/
*	contact us : inte2000@263.net
****************************************************************************************/
#ifndef __MENUHELP_H__
#define __MENUHELP_H__

template <class T>
class CMenuHelp
{
public:
	CMenuHelp()
	{
		m_hActiveMenu = NULL;
		m_crText = RGB(0,0,0);
		m_crHiText = RGB(0,0,0);
//		m_crBkGnd = RGB(222,222,222);
		m_crBkGnd = RGB(255,255,220);
		m_crHiBkGnd = RGB(255,238,194);
		m_nLeftSpace = 18;
	}

	~CMenuHelp() 
	{
	}
	
	HMENU AttachActiveMenu(HMENU hActiveMenu)
	{
		HMENU hTmp = m_hActiveMenu;
		m_hActiveMenu = hActiveMenu;
		int Count = GetMenuItemCount(m_hActiveMenu);
		for(int i = 0; i < Count; i++)
		{
			MENUITEMINFO mii;
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_FTYPE;
			GetMenuItemInfo(m_hActiveMenu,i,TRUE,&mii);
			mii.fType |= MFT_OWNERDRAW;
			SetMenuItemInfo(m_hActiveMenu,i,TRUE,&mii);
		}
		
		return hTmp;
	}

	HMENU DetachActiveMenu()
	{
		HMENU hTmp = m_hActiveMenu;
		m_hActiveMenu = NULL;
		return hTmp;
	}

	BEGIN_MSG_MAP(CMenuHelp)
		MESSAGE_HANDLER(WM_ENTERMENULOOP, OnEnterMenuLoop)
		MESSAGE_HANDLER(WM_EXITMENULOOP, OnExitMenuLoop)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
//		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	END_MSG_MAP()

	LRESULT OnEnterMenuLoop(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnExitMenuLoop(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DetachActiveMenu();
		return 0;
	}

	LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnInitMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
//		T*   pT = static_cast<T*>(this);
		HMENU hMenu = (HMENU)wParam;
		int Count = GetMenuItemCount(hMenu);
		ATLTRACE(_T("OnInitMenu : Count = %d,HMenu = %x\n"),Count,hMenu);
		AttachActiveMenu(hMenu);
		return 0;
	}

//@**#---2003-10-29 21:15:48 (NoName)---#**@
//	LRESULT OnInitMenuPopup(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	{
//		HMENU hMenu = (HMENU)wParam;
//		return 0;
//	}
	
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UINT uCtrlID = (UINT)wParam;
		LPMEASUREITEMSTRUCT pMI = (LPMEASUREITEMSTRUCT)lParam;
		if(pMI->CtlType == ODT_MENU)//We only want menu
		{
			ATLASSERT(m_hActiveMenu != NULL);

			MENUITEMINFO mii;
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_FTYPE | MIIM_STRING;
			mii.dwTypeData = NULL;
			GetMenuItemInfo(m_hActiveMenu,pMI->itemID,FALSE,&mii);//get the string length

			if(mii.fType & MFT_SEPARATOR)
			{
				pMI->itemWidth = 0;
				pMI->itemHeight = GetSystemMetrics(SM_CYMENU) >> 1;
			}
			else
			{
				int nWidth = 4;//left space : 2; right space 4
				int nHeight = 18;
				TCHAR *pszText = new TCHAR[mii.cch + 1];
				if(pszText != NULL)
				{
					mii.fMask = MIIM_FTYPE | MIIM_STRING;
					mii.dwTypeData = pszText;
					mii.cch += 1;
					GetMenuItemInfo(m_hActiveMenu,pMI->itemID,FALSE,&mii);//get menu text

					//for fix BUG on window 9X,Get bitmap handle sep
					mii.fMask = MIIM_STATE | MIIM_FTYPE | MIIM_BITMAP;
					mii.cch = 0;
					GetMenuItemInfo(m_hActiveMenu,pMI->itemID,FALSE,&mii);//get menu text
//					if((mii.fState & MFS_CHECKED) == MFS_CHECKED || (mii.fState & MFS_UNCHECKED) == MFS_UNCHECKED)
/*
					if((mii.fState & MFS_CHECKED) == MFS_CHECKED)
					{
						m_nLeftSpace = max(16,m_nLeftSpace);
						nWidth += m_nLeftSpace;
					}
*/
					if(mii.hbmpItem != NULL)
					{
						m_nLeftSpace = max(18,m_nLeftSpace);
						nWidth += m_nLeftSpace;
					}
					else
					{
						m_nLeftSpace = max(10,m_nLeftSpace);
						nWidth += m_nLeftSpace;
					}

					HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
					HWND hWnd = ::GetDesktopWindow();
					HDC hDC = ::GetDC(hWnd);
					HGDIOBJ of = ::SelectObject(hDC,hFont);
					RECT rect;
					rect.left=rect.top=0;
					SIZE size;
					size.cy = ::DrawText(hDC,pszText,-1,&rect,DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_CALCRECT);
					size.cx = rect.right - rect.left + 3;
					::SelectObject(hDC,of);
					::ReleaseDC(hWnd,hDC);  // Release the DC
					// Set width and height:
					nWidth += (size.cx + 1);//GAP;
					int temp = GetSystemMetrics(SM_CYMENU);
					nHeight = temp > 20 ? temp : 20;
				
					delete []pszText;
				}

				pMI->itemWidth = nWidth;
				pMI->itemHeight = nHeight;
			}

			return 1;
		}
		else
			return 0;
	}

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UINT uCtrlID = (UINT)wParam;
		LPDRAWITEMSTRUCT pDI = (LPDRAWITEMSTRUCT)lParam;
		
		if(pDI->CtlType == ODT_MENU)//We only want menu
		{
			ATLASSERT(m_hActiveMenu != NULL);
/*			
			HWND hMenuWnd = WindowFromDC(pDI->hDC);
			HDC hWndDC = GetDC(hMenuWnd);
			RECT rcWnd,rcClient;
			GetWindowRect(hMenuWnd,&rcWnd);
			GetClientRect(hMenuWnd,&rcClient);
			
//			ScreenToClient(hMenuWnd, (LPPOINT)&rcWnd);
//			ScreenToClient(hMenuWnd, ((LPPOINT)&rcWnd)+1);
			rcWnd.right = rcWnd.right - rcWnd.left;
			rcWnd.bottom = rcWnd.bottom - rcWnd.top;
			rcWnd.left = rcWnd.top = 0;

			CDCHandle dc1 = hWndDC;
			dc1.FillSolidRect(&rcWnd,m_crBkGnd);
			ReleaseDC(hMenuWnd,hWndDC);
*/
			CDCHandle dc = pDI->hDC;
			RECT rect = pDI->rcItem;

			MENUITEMINFO mii;
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_FTYPE | MIIM_STRING;
			mii.dwTypeData = NULL;
			GetMenuItemInfo(m_hActiveMenu,pDI->itemID,FALSE,&mii);//get the string length

			if(mii.fType & MFT_SEPARATOR)
			{
				RECT rcLeft = rect;
				rcLeft.right = rcLeft.left + m_nLeftSpace;//bitmap width
				rect.left += m_nLeftSpace;
				dc.FillSolidRect(&rcLeft,RGB(215,241,215));
				dc.FillSolidRect(&rect,m_crBkGnd);

				rect.top += ((rect.bottom - rect.top) >> 1);
				dc.DrawEdge(&rect,EDGE_ETCHED,BF_TOP);			
			}
			else
			{
				TCHAR *pszText = new TCHAR[mii.cch + 1];
				if(pszText != NULL)
				{
					mii.fMask = MIIM_FTYPE | MIIM_STRING;
					mii.dwTypeData = pszText;
					mii.cch += 1;
					GetMenuItemInfo(m_hActiveMenu,pDI->itemID,FALSE,&mii);//get the string length

					mii.fMask = MIIM_STATE | MIIM_FTYPE | MIIM_BITMAP;
					mii.cch = 0;
					GetMenuItemInfo(m_hActiveMenu,pDI->itemID,FALSE,&mii);//get bitmap handle


					RECT rcText,rcIcon;
					int dy;
					
					HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
					HFONT of = dc.SelectFont(hFont);
					
					dy = (rect.bottom - rect.top - 4 - 15) / 2;
					dy = (dy < 0) ? 0 : dy;

//					if(mii.hbmpItem != NULL || (mii.fState & MFS_CHECKED) == MFS_CHECKED)
						SetRect(&rcText,rect.left + m_nLeftSpace + 6,rect.top,rect.right,rect.bottom);
//					else
//						SetRect(&rcText,rect.left + 2,rect.top,rect.right,rect.bottom);
					
					rcIcon.left = rect.left + 2;
					rcIcon.right = rcIcon.left + m_nLeftSpace;
					rcIcon.top = rect.top + 2;
					rcIcon.bottom = rcIcon.top + 16;

					if(pDI->itemAction & ODA_DRAWENTIRE)
					{
						RECT rcLeft = rect;
						rcLeft.right = rcLeft.left + m_nLeftSpace;//bitmap width
						rect.left += m_nLeftSpace;
						dc.FillSolidRect(&rcLeft,RGB(215,241,215));
						dc.FillSolidRect(&rect,m_crBkGnd);

						if(mii.hbmpItem != NULL)//have icon
						{
							//draw the bitmap,not support now
						}
						else if((mii.fState & MFS_CHECKED) == MFS_CHECKED)
						{
							//draw the check mark
							HPEN hCheckPen = CreatePen(PS_SOLID, 2, m_crText);
							HPEN hop = dc.SelectPen(hCheckPen);

							dc.MoveTo(rcIcon.left + 2,rcIcon.top + 3);
							dc.LineTo(rcIcon.left + 3,rcIcon.top + 8);
							dc.MoveTo(rcIcon.left + 3,rcIcon.top + 9);
							dc.LineTo(rcIcon.left + 9,rcIcon.top + 3);

							dc.SelectPen(hop);
							DeleteObject(hCheckPen);
						}

						RECT rect2;
						SetRect(&rect2,rcText.left + 4,rcText.top + 1,rcText.right - 2,rcText.bottom - 1);
						COLORREF oclr = dc.SetTextColor(m_crText);
						int mode = dc.SetBkMode(TRANSPARENT);
						dc.DrawText(pszText,-1,&rect2,DT_LEFT|DT_NOCLIP|DT_SINGLELINE|DT_VCENTER);
						dc.SetBkMode(mode);
						dc.SetTextColor(oclr);
					}//if (lpDIS->itemAction & ODA_DRAWENTIRE)

					//Draws the selected item
					if((pDI->itemState & ODS_SELECTED) && (pDI->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
					{
						RECT rect2;
						if(mii.hbmpItem != NULL || (mii.fState & MFS_CHECKED) == MFS_CHECKED)
						{
							SetRect(&rect2,rcIcon.left - 2,rcIcon.top - 2,rcIcon.right + 2,rcIcon.bottom + 2);
							dc.Draw3dRect(&rect2,::GetSysColor(COLOR_3DHILIGHT),::GetSysColor(COLOR_3DDKSHADOW));
							rect2 = rcText;
						}
						else
						{
							SetRect(&rect2,rect.left + 2,rect.top + 1,rect.right - 2,rect.bottom - 1);
						}
						
						//draw hightlight background
//@**#---2004-04-14 20:54:30 (NoName)---#**@
//						RECT rcLeft = rect;
//						rcLeft.right = rcLeft.left + m_nLeftSpace;//bitmap width
//						rect.left += m_nLeftSpace;
//						dc.FillSolidRect(&rcLeft,RGB(215,241,215));
//						dc.FillSolidRect(&rect,m_crBkGnd);
						
						dc.FillSolidRect(&rect2,m_crHiBkGnd);
						HPEN penFrame = CreatePen(PS_SOLID, 1, RGB(0,0,128));
						HPEN hOP = dc.SelectPen(penFrame);
						dc.SelectStockBrush(NULL_BRUSH);
						dc.Rectangle(&rect2);
						dc.SelectPen(hOP);
						DeleteObject(penFrame);
						//draw highlight text
						SetRect(&rect2,rcText.left + 4,rcText.top + 1,rcText.right - 2,rcText.bottom - 1);
						COLORREF oclr = dc.SetTextColor(m_crHiText);
						COLORREF crBgOld = dc.SetBkColor(m_crHiBkGnd);
						dc.DrawText(pszText,-1,&rect2,DT_LEFT|DT_NOCLIP|DT_SINGLELINE|DT_VCENTER);
						dc.SetBkColor(crBgOld);
						dc.SetTextColor(oclr);
					}

					//Draws the deselected item
					if(!(pDI->itemState & ODS_SELECTED) && (pDI->itemAction & ODA_SELECT))
					{
						RECT rect2;
						if(mii.hbmpItem != NULL || (mii.fState & MFS_CHECKED) == MFS_CHECKED)
						{
							SetRect(&rect2,rcIcon.left-2,rcIcon.top-2,rcIcon.right+2,rcIcon.bottom+2);
							dc.Draw3dRect(&rect2,m_crBkGnd,m_crBkGnd);
							rect2 = rcText;
						}
						else
						{
							SetRect(&rect2,rect.left + 2,rect.top + 1,rect.right - 2,rect.bottom - 1);
						}

						//draw background
						RECT rcLeft = rect;
						rcLeft.right = rcLeft.left + m_nLeftSpace;//bitmap width
						rect.left += m_nLeftSpace;
						dc.FillSolidRect(&rcLeft,RGB(215,241,215));
						dc.FillSolidRect(&rect,m_crBkGnd);
//						dc.FillSolidRect(&rect2,m_crBkGnd);
						SetRect(&rect2,rcText.left+4,rcText.top+1,rcText.right-2,rcText.bottom-1);
						//draw Text
						COLORREF oclr = dc.SetTextColor(m_crText);
						COLORREF crBgOld = dc.SetBkColor(m_crBkGnd);
						dc.DrawText(pszText,-1,&rect2,DT_LEFT|DT_NOCLIP|DT_SINGLELINE|DT_VCENTER);
						dc.SetBkColor(crBgOld);
						dc.SetTextColor(oclr);
					}
					
					dc.SelectFont(of);
					
					delete []pszText;
				}
			}

			return 1;
		}
		else
			return 0;
	}


protected:
	HMENU m_hActiveMenu;
	COLORREF m_crText;
	COLORREF m_crHiText;
	COLORREF m_crBkGnd;
	COLORREF m_crHiBkGnd;
	int m_nLeftSpace;
	//CSimpleArray<t_ButtonClass *> m_arButtons;
};


#endif //__MENUHELP_H__