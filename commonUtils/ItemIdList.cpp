/* This file is part of FlashFolder. 
 * Copyright (C) 2007-2012 zett42.de ( zett42 at users.sourceforge.net ) 
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
#include "StdAfx.h"
#include "ItemIdList.h"

HRESULT GetPidlFromPath( SpITEMIDLIST* pidlResult, LPCWSTR path )
{
	pidlResult->reset();
	PIDLIST_ABSOLUTE pidl = NULL;
	SFGAOF attr = 0;
	HRESULT hr = ::SHParseDisplayName( path, NULL, &pidl, 0, &attr );
	if( SUCCEEDED( hr ) )
		*pidlResult = MakeSharedPidl( pidl );
	return hr;
}
