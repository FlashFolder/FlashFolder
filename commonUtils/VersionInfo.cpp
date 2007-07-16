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
#include "VersionInfo.h"

//-----------------------------------------------------------------------------------------------

bool GetFileVersion( VS_FIXEDFILEINFO* pVer, LPCTSTR pFilePath )
{
	memset( pVer, 0, sizeof(VS_FIXEDFILEINFO) );
	DWORD dummy;
	DWORD size = ::GetFileVersionInfoSize( pFilePath, &dummy );
	if( size == 0 )
		return false;

	std::vector<char> verInfo( size );
	if( ! ::GetFileVersionInfo( pFilePath, 0, size, &verInfo[0] ) )
		return false;
    
	TCHAR name[] = _T("\\");
	VS_FIXEDFILEINFO* pVerData = NULL;
	UINT len = 0;
	if( ! ::VerQueryValue( &verInfo[0], name, (void**) &pVerData, &len ) )
		return false;
	if( len < sizeof(VS_FIXEDFILEINFO) )
		return false;
	memcpy( pVer, pVerData, sizeof(VS_FIXEDFILEINFO) );

	return true;
}

//-----------------------------------------------------------------------------------------------

int CompareVersions( const int* v1, const int* v2 )
{
	if( v1[0] < v2[0] )	return -1;
	if( v1[0] > v2[0] )	return 1;
	if( v1[1] < v2[1] )	return -1;
	if( v1[1] > v2[1] )	return 1;
	if( v1[2] < v2[2] )	return -1;
	if( v1[2] > v2[2] )	return 1;
	if( v1[3] < v2[3] )	return -1;
	if( v1[3] > v2[3] )	return 1;
	return 0;
}
