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

#pragma warning(disable:4267) // size_t --> int

//-----------------------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------------------

inline CStringW Utf8ToUtf16( LPCSTR pUtf8 )
{
	if( ! pUtf8 )
		return L"";
	CStringW buf;
	int len = strlen( pUtf8 );
	int res = ::MultiByteToWideChar( CP_UTF8, 0, pUtf8, len, buf.GetBuffer( len * 4 ), len * 4 );
	if( res == ERROR_NO_UNICODE_TRANSLATION )
		return L"";
    buf.ReleaseBuffer( res );
	return buf;
}

inline CString Utf8ToStr( LPCSTR pUtf8 )
{
	if( ! pUtf8 )
		return _T("");
	return Utf8ToUtf16( pUtf8 );
}

//-----------------------------------------------------------------------------------------------

// trim an std::basic_string 
template<typename T>
void trim( T& s, const T& t = T(' ') + T('\t') + T('\r') + T('\n') )
{
	T::size_type p1, p2;
	p1 = s.find_first_not_of( t );
	if( p1 == T::npos )
	{
		s.clear();
		return;
	}
	p2 = s.find_last_not_of( t ) + 1;
	s = s.substr( p1, p2 - p1 );
}

