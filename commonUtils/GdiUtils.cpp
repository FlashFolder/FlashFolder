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

//-----------------------------------------------------------------------------------------------

void GetStandardOsFont( LOGFONT *pLF, WORD* pDefSize )
{
	// Use explicit size here, otherwise when compiling with WINVER >= 0x0600, it would break
	// backwards compatibility with XP.
	NONCLIENTMETRICS ncm = { sizeof(UINT) + 9 * sizeof(int) + 5 * sizeof(LOGFONT) };
	::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0 );
	*pLF = ncm.lfMessageFont;

	if( pDefSize )
	{
		// CreateIC() is less overhead than GetDC().
		HDC hScreenIC = ::CreateIC( _T("DISPLAY"), NULL, NULL, NULL );

		int h = pLF->lfHeight < 0 ? -pLF->lfHeight : pLF->lfHeight;
		*pDefSize = (WORD) MulDiv( h, 72, GetDeviceCaps( hScreenIC, LOGPIXELSY ) );
		::DeleteDC( hScreenIC );
	}
}
