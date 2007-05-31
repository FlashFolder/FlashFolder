/*
 *  CDlgAnchor class
 *
 *  uemakeXp development team
 *  http://www.uemake.com
 *
 *  you may freely use or modify this code
 *  -------------------------------------------------------------
 *  Controls can be anchored to one or more edges of their parent
 *  window.  Anchoring  a  control to its parent ensures that the
 *  anchored edges  remain  in  the same position relative to the
 *  edges  of  the  parent container when the parent container is
 *  resized. Controls  can  be  also  docked to one edge of their
 *  parent window or can be docked to all edges and fill it.
 *
 *  Modifications by zett42 (zett42 at users.sourceforge.net):
 *  - put all definitions into the CDlgAnchor class
 *  - use std::map as internal data structure
 *  - remove drawing artefacts/flickering by using DeferWindowPos() API
 *  - moved implementation into .cpp file
 */
#pragma once

#include <map>

//-------------------------------------------------------------------------------------------------

class CDlgAnchor 
{
public:
    enum AlignFlagsEnum { 
        ANCHOR_TOP          = 0x0001,
        ANCHOR_LEFT         = 0x0002,
        ANCHOR_RIGHT        = 0x0004,
        ANCHOR_BOTTOM       = 0x0008,
        ANCHOR_TOPLEFT      = 0x0003,
        ANCHOR_TOPRIGHT     = 0x0005,
        ANCHOR_BOTTOMLEFT   = 0x000a,
        ANCHOR_BOTTOMRIGHT  = 0x000c,
        ANCHOR_ALL          = 0x000f,
        DOCK_TOP            = 0x0100,
        DOCK_LEFT           = 0x0200,
        DOCK_RIGHT          = 0x0400,
        DOCK_BOTTOM         = 0x0800,
        DOCK_FILL           = 0x0f00,
        
        REDRAW_NOCOPYBITS   = 0x010000 };   
	          //use this flag only if redraw problems occur

	// constructor
	CDlgAnchor();

	// initialize
	BOOL Init(HWND hDlgWindow);
	BOOL Init(const CWnd &wnd);
	BOOL Init(const CWnd *pWnd);

	BOOL Add(HWND hWnd, UINT uFlag);
	BOOL Add(UINT uID, UINT uFlag);
	BOOL Add(const CWnd &wnd, UINT uFlag);

	BOOL Remove(UINT uID);
	BOOL Remove(HWND hWnd);
	BOOL Remove(const CWnd &wnd);

	BOOL Update(UINT uID);
	BOOL Update(HWND hWnd);
	BOOL Update(const CWnd &wnd);

	void UpdateAll();
	void RemoveAll();

	BOOL OnSize(BOOL bRepaint=TRUE);

private:
	struct AnchorItem 
	{
		UINT m_uFlag;        // anchor flags
		RECT m_rect;                // window rect
        AnchorItem( UINT uFlag, LPRECT lpRect ) 
            : m_uFlag( uFlag ), m_rect( *lpRect ) {}
	};

	typedef std::map<HWND, AnchorItem> AnchorItemMap;

private:
	HWND m_hWnd;
	RECT m_rect;
	AnchorItemMap m_controls;
	int m_controlsCount;
};