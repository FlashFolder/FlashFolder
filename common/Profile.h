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

//-----------------------------------------------------------------------------------------------
/**
 * \brief Abstract base class for defining user profiles. 
 *
 * A profile consists of keys and values. Each key references one value.\n
 * Keys are grouped in sections (with no nesting).\n\n
 *
 * A profile can have default values associated. These are relevant for the Get...() methods. 
 * Default value is read instead of the original value if the original value does not exist.
**/
class Profile
{
public:
	/// bit flags for Set...() functions
	enum SetFlags
	{
		DONT_OVERWRITE = 0x001   ///< if set, don't overwrite existing values
	};

	Profile() : m_pDefaults( NULL ) {}

	/// \brief Determine if a value exists. 
	virtual bool ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const = 0;
	/// \brief Determine if a section exists. 
	virtual bool SectionExists( LPCTSTR pSectionName ) const = 0;

	/// \brief Get a string value.
	virtual tstring GetString( LPCTSTR pSectionName, LPCTSTR pValueName ) const = 0;
	/// \brief Get an integer value.
	virtual int GetInt( LPCTSTR pSectionName, LPCTSTR pValueName ) const = 0;

	/// \brief Set a string value.
	virtual void SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags = 0 ) = 0;
	/// \brief Set an integer value.
	virtual void SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags = 0 ) = 0;

	/// \brief Complete remove a section including all values.
	virtual void DeleteSection( LPCTSTR pSectionName ) = 0;
	/// \brief Clear the contents of a section.
	virtual void ClearSection( LPCTSTR pSectionName ) = 0;
	/// \brief Remove a single value.
	virtual void DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName ) = 0;
	/// \brief Remove all sections and values from the profile.
	virtual void Clear() = 0;

	/// \brief Get a string list from a section. Strings are expected to be stored in keys that 
	/// consist of sequential numbers beginning from zero.
	void GetStringList( std::vector<tstring>* pList, LPCTSTR pSectionName ) const;
	/// \brief Store a string list into a section. The section is cleared first, then the strings 
	/// are stored in the format that GetStringList() expects.
	void SetStringList( LPCTSTR pSectionName, const std::vector<tstring>& list );

	/// \brief Associate default values with this profile.
	void SetDefaults( const Profile* pDefaults ) { m_pDefaults = pDefaults; }
	/// \brief Get default values association for this profile.
	const Profile* GetDefaults() const           { return m_pDefaults; }

private:
	const Profile* m_pDefaults;
};

//-----------------------------------------------------------------------------------------------
/**
 * \brief Read / write program configuration in memory.
**/
class MemoryProfile : public Profile
{
public:
	bool IsEmpty() const { return m_data.empty(); }

	virtual bool ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const;
	virtual bool SectionExists( LPCTSTR pSectionName ) const;

	virtual tstring GetString( LPCTSTR pSectionName, LPCTSTR pValueName ) const;
	virtual int GetInt( LPCTSTR pSectionName, LPCTSTR pValueName ) const;

	virtual void SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags );
	virtual void SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags );

	virtual void DeleteSection( LPCTSTR pSectionName );
	virtual void ClearSection( LPCTSTR pSectionName );
	virtual void DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName );

	virtual void Clear() { m_data.clear(); }

private:
	struct LessNoCase 
	{
		bool operator()( const tstring& s1, const tstring& s2 ) const 
			{ return _tcsicmp( s1.c_str(), s2.c_str() ) < 0; }		
	};
	typedef std::map< tstring, tstring, LessNoCase > ValueMap;
	typedef std::map< tstring, ValueMap, LessNoCase > SectionMap;
	SectionMap m_data;
};

//-----------------------------------------------------------------------------------------------
/**
 * \brief Read / write program configuration into either user-specific or shared registry storage.
**/
class RegistryProfile : public Profile
{
public:
	RegistryProfile() : m_isShared( false ) {}
	RegistryProfile( LPCTSTR rootPath ) { SetRoot( rootPath ); }

	void SetRoot( LPCTSTR rootPath );
	bool IsShared() const {	return m_isShared; } 

	virtual bool ValueExists( LPCTSTR pSectionName, LPCTSTR pValueName ) const;
	virtual bool SectionExists( LPCTSTR pSectionName ) const;

	virtual tstring GetString( LPCTSTR pSectionName, LPCTSTR pValueName ) const;
	virtual int GetInt( LPCTSTR pSectionName, LPCTSTR pValueName ) const;

	virtual void SetString( LPCTSTR pSectionName, LPCTSTR pValueName, LPCTSTR pValue, DWORD flags = 0 );
	virtual void SetInt( LPCTSTR pSectionName, LPCTSTR pValueName, int value, DWORD flags = 0 );

	virtual void DeleteSection( LPCTSTR pSectionName );
	virtual void ClearSection( LPCTSTR pSectionName );
	virtual void DeleteValue( LPCTSTR pSectionName, LPCTSTR pValueName );

	virtual void Clear();

private:
	tstring m_regPath;
	HKEY m_hRootKey;
	bool m_isShared;
};