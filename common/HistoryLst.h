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

#include <vector>
#include <commonUtils\Profile.h>

//----------------------------------------------------------------------------------

class HistoryLst 
{
public:
	HistoryLst();

	void Add( LPCTSTR s );
	void AddFolder( LPCTSTR s );
	
    void SetMaxEntries( int count );
    inline size_t GetMaxEntries() const { return m_maxEntries; };
    inline size_t GetCount() const { return m_list.size(); };
	
    inline const tstring &operator [](int n) const { return m_list[n]; };
    inline const std::vector<tstring> &GetList() const { return m_list; };

    bool LoadFromProfile( const Profile& profile, LPCTSTR sectionName );
    void SaveToProfile( Profile& profile, LPCTSTR sectionName );

private:
    typedef std::vector<tstring> HISTORYLIST;
    HISTORYLIST m_list;
    int m_maxEntries;
};

