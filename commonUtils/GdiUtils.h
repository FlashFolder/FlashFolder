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

#include "boost\noncopyable.hpp"
#include "OsVersion.h"
#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>

//----------------------------------------------------------------------------------------------------
/// RAII wrapper for HDC which calls ::DeleteDC() in destructor.

class DC : boost::noncopyable
{
public:
	explicit DC( HDC handle = NULL ) : m_handle( handle ) {}
	
	~DC() { Close(); }
	
	HDC Attach( HDC h ) { Close(); m_handle = h; return h; }
	
	HDC Detach() { HDC h = m_handle; m_handle = NULL; return h; }

	operator HDC() const { return m_handle; }

	void Close() 
	{ 
		if( m_handle )
			::DeleteDC( m_handle ); m_handle = NULL; 
	}

private:
	HDC m_handle;
};

//----------------------------------------------------------------------------------------------------
/// RAII wrapper for HDC which calls ::ReleaseDC() in destructor.

class WindowDC : boost::noncopyable
{
public:
	explicit WindowDC( HWND hwnd = NULL, HDC handle = NULL ) : 
		m_hwnd( hwnd ), m_handle( handle ) {}
	
	~WindowDC() { Close(); }
	
	HDC Detach() { HDC h = m_handle; m_handle = NULL; return h; }

	operator HDC() const { return m_handle; }

	void Close() 
	{ 
		if( m_handle )
		{
			::ReleaseDC( m_hwnd, m_handle ); 
			m_handle = NULL; m_hwnd = NULL; 
		}
	}

private:
	HWND m_hwnd;
	HDC m_handle;
};

//----------------------------------------------------------------------------------------------------
/// RAII wrapper for ::BeginPaint() / ::EndPaint()

class PaintDC : boost::noncopyable
{
public:
	explicit PaintDC( HWND hwnd ) :
		m_hwnd( hwnd ),
		m_hdc( NULL )
	{
		if( ::GetUpdateRect( hwnd, NULL, FALSE ) )
			m_hdc = ::BeginPaint( hwnd, &m_ps );
		else
			memset( &m_ps, 0, sizeof( m_ps ) );
	}

	~PaintDC()
	{
		if( m_hdc )
			::EndPaint( m_hwnd, &m_ps );
	}
	
	operator HDC() const { return m_hdc; }

	const PAINTSTRUCT& PS() const { return m_ps; }
	
private:
	HWND m_hwnd;
	HDC m_hdc;
	PAINTSTRUCT m_ps;
};

//----------------------------------------------------------------------------------------------------
/// RAII wrapper for GDI objects which calls ::DeleteObject() in destructor.

template< typename T >
class GdiObject : boost::noncopyable
{
public:
	explicit GdiObject( T handle = NULL ) : m_handle( handle ) {}
	
	~GdiObject() { Close(); }
	
	T Attach( T h ) { Close(); m_handle = h; return h; }
	
	T Detach() { HDC h = m_handle; m_handle = NULL; return h; }

	operator T() const { return m_handle; }

	void Close() 
	{ 
		if( m_handle )
		{
			::DeleteObject( m_handle ); 
			m_handle = NULL; 
		}
	}

private:
	T m_handle;
};

typedef GdiObject< HBITMAP > Bitmap;
typedef GdiObject< HFONT > Font;
typedef GdiObject< HBRUSH > Brush;
typedef GdiObject< HPEN > Pen;
typedef GdiObject< HRGN > Region;

//----------------------------------------------------------------------------------------------------
/// 32 bpp paint buffer (DIB section + associated device context).

class PaintBuf : boost::noncopyable
{
public:
	PaintBuf( int width, int height )
	{
		Create( width, height );
	}

	PaintBuf( const RECT& rc ) 
	{
		Create( rc.right - rc.left, rc.bottom - rc.top );
	}
	
	~PaintBuf()
	{
		if( m_dc && m_oldBmp )
			::SelectObject( m_dc, m_oldBmp );
	}
	
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	operator HDC() const { return m_dc; }

	HBITMAP GetBitmap() const { return m_bmp; }
	
	DWORD* GetBits() const { return m_bits; }

private:
	void Create( int width, int height )
	{
		m_width = m_height = 0;
		m_bits = NULL;
		m_oldBmp = NULL;

		BITMAPINFO bmi = { sizeof( bmi ) };
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biWidth = width;
		bmi.bmiHeader.biHeight = -height;
		if( m_bmp.Attach( ::CreateDIBSection( NULL, &bmi, DIB_RGB_COLORS, (void**) &m_bits, NULL, 0 ) ) )
			if( m_dc.Attach( ::CreateCompatibleDC( NULL ) ) )
			{
				m_oldBmp = ::SelectObject( m_dc, m_bmp );
				m_width = width;
				m_height = height;
			}
	}

	DWORD* m_bits;
	int m_width, m_height;
	Bitmap m_bmp;
	DC m_dc;	
	HGDIOBJ m_oldBmp;
};

//-----------------------------------------------------------------------------------------------

/// RAII Wrapper for SelectObject() API
class AutoSelectObj : boost::noncopyable
{
public:
	explicit AutoSelectObj( HDC hDC, HGDIOBJ hObject = NULL )
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

/// RAII Wrapper for SaveDC() API
class AutoSaveDC : boost::noncopyable
{
public:
	explicit AutoSaveDC( HDC hDC ) 
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

/// RAII Wrapper for WM_SETREDRAW
class AutoRedraw : boost::noncopyable
{
public:
	explicit AutoRedraw( HWND hwnd, UINT flags = RDW_INVALIDATE ) :
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

//-----------------------------------------------------------------------------------------------
/// RAII wrapper for HTHEME (::OpenThemeData() / ::CloseThemeData())

class Theme : boost::noncopyable
{
public:
	explicit Theme( HTHEME hTheme = NULL ) : m_hTheme( hTheme ) {}

	Theme( HWND hwnd, LPCWSTR classNames )
	{
		m_hTheme = ::OpenThemeData( hwnd, classNames );
	}

	~Theme() { Close(); }

	void Close() { if( m_hTheme ) ::CloseThemeData( m_hTheme ); m_hTheme = NULL; }

	void Attach( HTHEME h ) { Close(); m_hTheme = h; }

	HTHEME Detach() { HTHEME h = m_hTheme; m_hTheme = NULL; return h; }

	operator HTHEME() const { return m_hTheme; }

private:
	HTHEME m_hTheme;
};

//----------------------------------------------------------------------------------------------------
/// Class wrapper for POINT

struct Point : POINT
{
	Point() { x = y = 0; } 
	Point( LONG x_, LONG y_ ) { x = x_; y = y_; }
	bool operator==( const POINT& other ) const { return x == other.x && y == other.y; }

	operator const POINT*() const { return this; }
};

/// Class wrapper for SIZE
struct Size : SIZE
{
	Size() { cx = cy = 0; }
	Size( LONG cx_, LONG cy_ ) { cx = cx_; cy = cy_; }
	bool operator==( const SIZE& other ) const { return cx == other.cx && cy == other.cy; }

	operator const SIZE*() const { return this; }
};

/// Class wrapper for RECT
struct Rect : RECT
{
	Rect() { left = top = right = bottom = 0; }
	Rect( LONG left_, LONG top_, LONG right_, LONG bottom_ ) 
		{ left = left_; top = top_; right = right_; bottom = bottom_; }
	Rect( const POINT& pos, const SIZE& size )
		{ left = pos.x; top = pos.y; right = pos.x + size.cx; bottom = pos.y + size.cy; }

	LONG Width() const { return right - left; }
	LONG Height() const { return bottom - top; }

	Point TopLeft() const { return Point( left, top ); }
	Point BottomRight() const { return Point( right, bottom ); }
	Size GetSize() const { return Size( Width(), Height() ); }

	Rect Offset( LONG cx, LONG cy ) const { return Rect( left + cx, top + cy, right + cx, bottom + cy ); }
	Rect Inflate( LONG cx, LONG cy ) const { return Rect( left - cx, top - cy, right + cx, bottom + cy ); } 

	bool operator==( const RECT& other ) const 
		{ return left == other.left && top == other.top && right == other.right && bottom == other.bottom; }

	operator LPCRECT() const { return this; }
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

/// Convenience wrapper for GetClientRect() API
inline Rect GetClientRect( HWND hwnd )
{
	Rect rc; ::GetClientRect( hwnd, &rc );
	return rc;
}

/// Convenience wrapper for GetWindowRect() API
inline Rect GetWindowRect( HWND hwnd )
{
	Rect rc; ::GetWindowRect( hwnd, &rc );
	return rc;
}

/// Convenience wrapper for GetWindowRect() / ScreenToClient() API
inline Rect GetChildRect( HWND parent, HWND child )
{
	Rect rc = GetWindowRect( child );
	ScreenToClientRect( parent, &rc );
	return rc;
}

/// Convenience wrapper for GetWindowRect() / ScreenToClient() API
inline Rect GetChildRect( HWND child )
{
	return GetChildRect( ::GetParent( child ), child );
}

/// Convenience wrapper for GetWindowRect() / ScreenToClient() API
inline Rect GetChildRect( HWND parent, int childId )
{
	return GetChildRect( parent, ::GetDlgItem( parent, childId ) );
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

//-----------------------------------------------------------------------------------------------

/// Get a stock brush.
inline HBRUSH GetStockBrush( int i ) { return reinterpret_cast<HBRUSH>( ::GetStockObject( i ) ); }

//-----------------------------------------------------------------------------------------------

/// Call ::IsThemeActive() if current OS version supports it.
/// Requires to use delay-load for uxtheme.dll!
inline bool IsThemeSupportedAndActive()
{
	return OSVERSION >= WINVER_XP && ::IsThemeActive();
}

/// Call ::DwmIsCompositionEnabled() if current OS version supports it.
/// Requires to use delay-load for dwmapi.dll!
inline bool IsCompositionSupportedAndActive()
{
	BOOL enabled = FALSE;
	return OSVERSION >= WINVER_VISTA && SUCCEEDED( ::DwmIsCompositionEnabled( &enabled ) ) && enabled;
}