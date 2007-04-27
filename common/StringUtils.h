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

#include <string>

template< typename T_iter, typename T_char >
void SplitString( T_iter itDest, const T_char* pStr, T_char chSep = ',' )
{
	const T_char* p1 = pStr;
	for(;;)
	{
		while( *pStr != chSep && *pStr != 0 )
			++pStr;
		std::basic_string<T_char> tmp( p1, pStr - p1 );
		*itDest++ = tmp.c_str();
		if( *pStr == 0 )
			break;
		pStr++;
		p1 = pStr;
	}
}
