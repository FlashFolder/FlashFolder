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
#include "CacheImageList.h"

//-----------------------------------------------------------------------------------------------

// unnamed namespace for private stuff
namespace {

//-----------------------------------------------------------------------------------------------


// The format of icon group resources (not defined in any SDK header).

// #pragmas are used here to insure that the structure's
// packing in memory matches the packing of the EXE or DLL.

#pragma pack( push )
#pragma pack( 2 )
struct GRPICONDIRENTRY
{
   BYTE   bWidth;               // Width, in pixels, of the image
   BYTE   bHeight;              // Height, in pixels, of the image
   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;            // Reserved
   WORD   wPlanes;              // Color Planes
   WORD   wBitCount;            // Bits per pixel
   DWORD  dwBytesInRes;         // how many bytes in this resource?
   WORD   nID;                  // the ID
};
#pragma pack( pop )

#pragma pack( push )
#pragma pack( 2 )
struct GRPICONDIR
{
   WORD            idReserved;   // Reserved (must be 0)
   WORD            idType;       // Resource type (1 for icons)
   WORD            idCount;      // How many images?
   GRPICONDIRENTRY idEntries[1]; // The entries for each image
};
#pragma pack( pop )

//-----------------------------------------------------------------------------------------------

inline CString MakeResourceKey( CString path, int resId )
{
	CString s; s.Format( _T(",%d"), resId );
	path += s;
	return path;
}

//-----------------------------------------------------------------------------------------------

}; //unnamed namespace 

//-----------------------------------------------------------------------------------------------

int CCacheImageList::GetCacheImageId( const CString& path, int resId ) const
{
	CString key = MakeResourceKey( path, resId );
	PathToId::const_iterator it = m_pathToId.find( key );
	return it == m_pathToId.end() ? -1 : it->second;
}

//-----------------------------------------------------------------------------------------------

bool CCacheImageList::GetCacheImagePath( CString* pPath, int* pResId, int imgId )
{
	*pPath = _T("");
	*pResId = 0;

	IdToPath::const_iterator it = m_idToPath.find( imgId );
	if( it == m_idToPath.end() )
		return false;

	int p = it->second.ReverseFind( ',' );
	if( p == -1 )
	{
		*pPath = it->second;	
	}
	else
	{
		*pPath = it->second.Left( p );
		*pResId = _ttoi( it->second.Mid( p + 1 ) );
	}
	return true;
}

//-----------------------------------------------------------------------------------------------
// TODO: implement loading of icon from .ICO file

int CCacheImageList::GetCacheIcon( LPCTSTR pPath )
{
	assert( m_hImageList ); // make sure the image list has been created
	return -1;
}

//-----------------------------------------------------------------------------------------------

int CCacheImageList::GetCacheIcon( HINSTANCE hInst, int resId, const CString& path )
{
	assert( m_hImageList ); // make sure the image list has been created

	// Check if icon is already in image list
	int id = GetCacheImageId( path, resId );
	if( id != -1 )
		return id;

	// Try to load the best-quality icon, by finding the one that best matches the format 
	// of the image list (icon size and color depth).
	
	int desiredCX = 0, desiredCY = 0;
	::ImageList_GetIconSize( m_hImageList, &desiredCX, &desiredCY );

	// Get the icon group resource
	const GRPICONDIR* pIconDir = NULL;
	int iconDirSize = 0;
	if( HRSRC hRes = ::FindResource( hInst, MAKEINTRESOURCE( resId ), RT_GROUP_ICON ) )
		if( HGLOBAL hData = ::LoadResource( hInst, hRes ) )
			if( pIconDir = (const GRPICONDIR*) ::LockResource( hData ) )
				iconDirSize = ::SizeofResource( hInst, hRes );
	if( ! pIconDir )
		return -1;

	// Avoid reading from bad memory locations, possibly caused by corrupted data.
	if( sizeof( GRPICONDIR ) > iconDirSize )
		return -1;
	const BYTE* pIconDirStart = (const BYTE*) pIconDir;
	const BYTE* pIconDirEnd = (const BYTE*) &pIconDir->idEntries[ pIconDir->idCount ];
	if( pIconDirEnd - pIconDirStart > iconDirSize )
		return -1;

	// Scaling images down looks better than upscaling. So find the icon size that 
	// is >= the image list icon size. If no such icon exists, use the largest icon. 
	int cx = 0x10000, cy = 0x10000;
	int maxCX = 0, maxCY = 0;
	for( int i = 0; i < pIconDir->idCount; ++i )
	{
		const GRPICONDIRENTRY& entry = pIconDir->idEntries[ i ];
		if( entry.bWidth < cx && entry.bWidth >= desiredCX &&
			entry.bHeight < cy && entry.bHeight >= desiredCY )
		{
			cx = entry.bWidth; cy = entry.bHeight;
		}
		if( entry.bWidth > maxCX &&	entry.bHeight > maxCY )
		{
			maxCX = entry.bWidth; maxCY = entry.bHeight;
		}
	}
	if( cx == 0x10000 )
		{ cx = maxCX; cy = maxCY; }

	// Find the highest color depth version of this icon size.
	int iconId = -1;
    int maxBitCount = 0;
	for( int i = 0; i < pIconDir->idCount; ++i )
	{
		const GRPICONDIRENTRY& entry = pIconDir->idEntries[ i ];
		if( entry.bWidth == cx && entry.bHeight == cy && entry.wBitCount > maxBitCount )
		{
			maxBitCount = entry.wBitCount;
			iconId = entry.nID;
		}
	}
	if( iconId == -1 )
		return -1;

	// Load the icon and put it into the image list
	HICON hIcon = NULL;
	if( HRSRC hRes = ::FindResource( hInst, MAKEINTRESOURCE( iconId ), RT_ICON ) )
		if( HGLOBAL hData = ::LoadResource( hInst, hRes ) )
			if( BYTE* pData = (BYTE*) ::LockResource( hData ) )
			{
				DWORD size = ::SizeofResource( hInst, hRes );
				hIcon = ::CreateIconFromResourceEx( 
					pData, size, TRUE, 0x00030000, cx, cy, 0 );				
			}
	if( ! hIcon )
		return -1;

	int imgId = Add( hIcon );
	
	::DestroyIcon( hIcon );

	if( imgId != -1 )
	{
		// create mapping between path + resource ID and image ID 
		CString key = MakeResourceKey( path, resId );
		m_pathToId[ key ] = imgId;
		m_idToPath[ imgId ] = key;
	}

	return imgId;
}

//-----------------------------------------------------------------------------------------------

void CCacheImageList::RemoveFromCache( const CString& path, int resId )
{
	assert( m_hImageList ); // make sure the image list has been created

	CString key = MakeResourceKey( path, resId );
	PathToId::iterator it = m_pathToId.find( key );
	if( it == m_pathToId.end() )
		return;

	m_freeList.insert( it->second );
	m_idToPath.erase( it->second );
	m_pathToId.erase( it );
}

//-----------------------------------------------------------------------------------------------

void CCacheImageList::CompactCache()
{
	assert( false );  // TODO: implement

	assert( m_hImageList ); // make sure the image list has been created


}

