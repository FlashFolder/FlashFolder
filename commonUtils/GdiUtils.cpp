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

#include "stdafx.h"
#include "GdiUtils.h"

#ifndef TMT_MSGBOXFONT
	#define TMT_MSGBOXFONT 805
#endif

//-----------------------------------------------------------------------------------------------

void GetSysMessageFont( LOGFONT* plf, HWND hwnd )
{
	bool isThemed = false, hasFont = false;
	OSVERSIONINFO ovi = { sizeof(ovi) };
	GetVersionEx( &ovi );
	if( ( ovi.dwMajorVersion << 8 | ovi.dwMinorVersion ) >= 0x0501 ) 
		isThemed = ::IsThemeActive() != 0;
	if( isThemed )
	{
		if( HTHEME hTheme = ::OpenThemeData( hwnd, L"WINDOW") )
		{
			if( ::GetThemeSysFont( hTheme, TMT_MSGBOXFONT, plf ) == S_OK )
				hasFont = true;
			::CloseThemeData( hTheme );
		}
	}
	if( ! hasFont )
	{
		NONCLIENTMETRICS ncm = { sizeof(ncm) };
		::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0 );
		*plf = ncm.lfMessageFont;
	}
}
