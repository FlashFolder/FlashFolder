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
/// \brief Various classes / functions for easing GDI API programming.

#pragma once

//-----------------------------------------------------------------------------------------------

/// Wrapper for SelectObject() API
class AutoSelectObj
{
public:
	AutoSelectObj( HDC hDC, HGDIOBJ hObject = NULL )
		: m_hDC( hDC ), m_hOldObject( NULL )
	{
		if( hObject )
			m_hOldObject = ::SelectObject( hDC, hObject );
	}
	void Select( HGDIOBJ hObject )
	{
		HGDIOBJ hOldObj = ::SelectObject( m_hDC, hObject );
		if( ! m_hOldObject )
            m_hOldObject = hOldObj;		
	}
	void Restore()
	{
		if( m_hOldObject ) 
		{
			::SelectObject( m_hDC, m_hOldObject ); 
			m_hOldObject = NULL;
		}
	}
	~AutoSelectObj() { Restore(); }

private:
	HDC m_hDC;
	HGDIOBJ m_hOldObject;
};

//-----------------------------------------------------------------------------------------------

/// Wrapper for SaveDC() API
class AutoSaveDC
{
public:
	AutoSaveDC( HDC hDC ) 
		: m_hDC( hDC )
	{ 
		m_saveId = ::SaveDC( hDC ); 
	}
	void Restore()
	{
		if( m_saveId )
		{
			::RestoreDC( m_hDC, m_saveId );
			m_saveId = 0;
		}
	}
	~AutoSaveDC() { Restore(); }

private:
	HDC m_hDC;
	int m_saveId;
};

//-----------------------------------------------------------------------------------------------

/// Wrapper for WM_SETREDRAW
class AutoRedraw
{
public:
	AutoRedraw( HWND hwnd, UINT flags = RDW_INVALIDATE ) :
		m_hwnd( hwnd ), m_flags( flags )
	{
		::SendMessage( m_hwnd, WM_SETREDRAW, FALSE, 0 );	
	}
	~AutoRedraw()
	{
		::SendMessage( m_hwnd, WM_SETREDRAW, TRUE, 0 );
		if( m_flags != 0 )
			::RedrawWindow( m_hwnd, NULL, NULL, m_flags );	
	}
private:
	HWND m_hwnd;
	UINT m_flags;
};

//----------------------------------------------------------------------------------------------------

/// Blend two colors
inline DWORD BlendColor( DWORD col1, DWORD col2, int blend = 128 )
{
    //red / blue   
    DWORD c1 = col1 & 0xFF00FF;
    DWORD c2 = col2 & 0xFF00FF;
    DWORD c3 = c1 + ( ( ( c2 - c1 ) * blend ) >> 8 );
    DWORD res = c3 & 0xFF00FF;
    //alpha / green
    c1 = ( col1 & 0xFF00FF00 ) >> 8;
    c2 = ( col2 & 0xFF00FF00 ) >> 8;
    c3 = c1 + ( ( ( c2 - c1 ) * blend ) >> 8 );
    return res | ( ( c3 & 0xFF00FF ) << 8 );
}

//-----------------------------------------------------------------------------------------------

/// Preferred way to get the standard font for text in dialog boxes as opposed to
/// GetStockObject( DEFAULT_GUI_FONT ) which returns the wrong font on XP / Vista and doesn't 
/// respect themes.\n
/// NOTE: When using a high-DPI setting, the resulting font size is too big, i.e. the font size
/// differs from the dialog font that was set in the dialog template.  

void GetStandardOsFont( LOGFONT *pLF, WORD* pDefSize = NULL );

#ifdef _MFC_VER
inline void CreateStandardOsFont( CFont* pFont )
{
	LOGFONT lf;
	GetStandardOsFont( &lf );
	pFont->CreateFontIndirect( &lf );
}
#endif

inline HFONT CreateStandardOsFont()
{
	LOGFONT lf;
	GetStandardOsFont( &lf );
	return ::CreateFontIndirect( &lf );	
}

//-----------------------------------------------------------------------------------------------

/// Convenience wrapper for MapDialogRect()
inline int MapDialogX( HWND hDlg, int x )
{
	RECT rc = { 0, 0, x, 1 };
	MapDialogRect( hDlg, &rc );
	return rc.right;
}

/// Convenience wrapper for MapDialogRect()
inline int MapDialogY( HWND hDlg, int y )
{
	RECT rc = { 0, 0, 1, y };
	MapDialogRect( hDlg, &rc );
	return rc.bottom;
}

/// Convenience wrapper for MapDialogRect()
inline POINT MapDialogPoint( HWND hDlg, POINT pt )
{
	RECT rc = { 0, 0, pt.x, pt.y };
	MapDialogRect( hDlg, &rc );
	POINT res = { rc.right, rc.bottom };
	return res;
}

/// Convenience wrapper for MapDialogRect()
inline RECT MapDialogRect( HWND hDlg, const RECT& rc )
{
	RECT res = rc;
	MapDialogRect( hDlg, &res );
	return res;
}

//-----------------------------------------------------------------------------------------------

/// Convenience wrapper for ScreenToClient() API
inline void ScreenToClientRect( HWND hwnd, RECT* prc )
{
	POINT pt1 = { prc->left, prc->top };
	::ScreenToClient( hwnd, &pt1 );
	POINT pt2 = { prc->right, prc->bottom };
	::ScreenToClient( hwnd, &pt2 );
	prc->left = pt1.x; prc->top = pt1.y; prc->right = pt2.x; prc->bottom = pt2.y;
}

/// Convenience wrapper for ClientToScreen() API
inline void ClientToScreenRect( HWND hwnd, RECT* prc )
{
	POINT pt1 = { prc->left, prc->top };
	::ClientToScreen( hwnd, &pt1 );
	POINT pt2 = { prc->right, prc->bottom };
	::ClientToScreen( hwnd, &pt2 );
	prc->left = pt1.x; prc->top = pt1.y; prc->right = pt2.x; prc->bottom = pt2.y;
}

