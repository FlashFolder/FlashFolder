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
 */
#pragma once

#include <boost\shared_ptr.hpp>

typedef boost::shared_ptr< ITEMIDLIST __unaligned > SpITEMIDLIST;
typedef boost::shared_ptr< const ITEMIDLIST __unaligned > SpcITEMIDLIST;

template< typename T >
struct SharedPidlDeleter
{
	void operator()( T pidl ) { ::CoTaskMemFree( (LPVOID) pidl ); }
};

inline SpITEMIDLIST MakeSharedPidl( LPITEMIDLIST pidl )
{
	return SpITEMIDLIST( pidl, SharedPidlDeleter< LPITEMIDLIST >() );
}

inline SpcITEMIDLIST MakeSharedPidl( LPCITEMIDLIST pidl )
{
	return SpcITEMIDLIST( pidl, SharedPidlDeleter< LPCITEMIDLIST >() );
}

inline HRESULT GetPidlFromPath( PIDLIST_ABSOLUTE* pidlResult, LPCWSTR path )
{
	SFGAOF attr = 0;
	return ::SHParseDisplayName( path, NULL, pidlResult, 0, &attr );
}

HRESULT GetPidlFromPath( SpITEMIDLIST* pidlResult, LPCWSTR path );
