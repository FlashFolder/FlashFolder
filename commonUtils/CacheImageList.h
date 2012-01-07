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

#pragma once

#include <map>

class CCacheImageList : public CImageList
{
public:
	/// Get icon from cache / file
	int GetCacheIcon( LPCTSTR pPath );

	/// Get icon from cache / application resource (pPath is only used to identify icon in cache)
	int GetCacheIcon( HINSTANCE hInst, int resId,  const CString& path );

	/// Get ID of icon in cache
	int GetCacheImageId( const CString& path, int resId = -1 ) const;

	/// Get path and resource ID from image ID
	bool GetCacheImagePath( CString* pPath, int* pResId, int imgId );

	/// Mark image as deleted (without changing other image IDs) 
	void RemoveFromCache( const CString& path, int resId );

	/// Free the space of images marked as deleted (may change existing image IDs)
	void CompactCache();
	
	/// Free all memory allocated for the image list
	void ClearCache()
	{
		DeleteImageList();
		m_idToPath.clear();
		m_pathToId.clear();
		m_freeList.clear();
	}

private:
	struct LessNoCase
	{
		bool operator()( const CString& s1, const CString& s2 ) const
			{ return s1.CompareNoCase( s2 ) < 0; }
	};

	// map image IDs to file pathes
	typedef std::map< int, CString > IdToPath;
	IdToPath m_idToPath;
	
	// map file pathes to image IDs 
	typedef std::map< CString, int, LessNoCase > PathToId;
	PathToId m_pathToId;

	// save deleted image IDs for later "recycling"
	typedef std::set<int> FreeList;
	FreeList m_freeList;
};
