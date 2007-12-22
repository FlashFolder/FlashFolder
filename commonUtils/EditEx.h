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
#pragma once

/**
 * \brief An edit control that can display a differently styled hint text if it is empty and not focused.
**/
class CEditEx : public CEdit
{
	DECLARE_DYNAMIC(CEditEx)

public:
	/// bit flags for SetHintFontStyle()
	enum StyleFlags
	{
		FF_ITALIC = 0x0001,
		FF_BOLD   = 0x0002
	};

	CEditEx() : 
		m_hintFontStyle( 0 ),
		m_hintColor( 0xFF000000 | COLOR_GRAYTEXT ) {}

	void SetHintText( const CString& text )
	{
		m_hintText = text;
		Invalidate();
	}
	CString GetHintText() const { return m_hintText; }

	void SetHintFontStyle( DWORD style )
	{
		if( style != m_hintFontStyle )
		{
			m_hintFontStyle = style;
			Invalidate();
		}
	}
	DWORD GetHintFontStyle() const { return m_hintFontStyle; }

	void SetHintColor( COLORREF color )
	{
		if( color != m_hintColor )
		{
			m_hintColor = color;
			Invalidate();
		}
		
	}
	COLORREF GetHintColor() const { return m_hintColor; }

protected:
	afx_msg void OnPaint();
	afx_msg void OnSetFocus( CWnd* );
	afx_msg void OnKillFocus( CWnd* );
	DECLARE_MESSAGE_MAP()

private:
	CString m_hintText;
	DWORD m_hintFontStyle;
	COLORREF m_hintColor;
};


