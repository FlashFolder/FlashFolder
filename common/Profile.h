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

#include "tstring.h"

/**
 * Class to read / write program configuration into either user-specific or shared storage (i.e. registry).
**/
class Profile
{
public:
	/// bit flags for Set...() functions
	enum SetFlags
	{
		DONT_OVERWRITE = 0x001   ///< if set, don't overwrite existing values
	};

	Profile( LPCTSTR rootKey )
		: m_rootKey( rootKey ) {}

	void SetRoot( LPCTSTR rootKey ) { m_rootKey = rootKey; }
	tstring GetRoot() const { return m_rootKey; }

	bool IsShared() const; 
	void SetShared( bool isShared );

	bool ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const;
	bool SectionExists( LPCTSTR pSectionName ) const;

	tstring GetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pDefaultValue = _T("") ) const;
	int GetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int defaultValue = 0 ) const;

	void SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags = 0 );
	void SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags = 0 );

	void DeleteSection( LPCTSTR pSectionName );
	void DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName );

private:
	void GetRootKey( HKEY* pKey, tstring* pPath, LPCTSTR pSectionName = NULL ) const;

	tstring m_rootKey; 		
};