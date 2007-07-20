
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
