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
 *
 */
#pragma once

//-----------------------------------------------------------------------------------------------
/// \brief Abstract base class for property pages.
///
/// - automatically sets modified flag (SetModified()) by handling change-notifications send 
///   by child controls
/// - provides virtual method for reading profile data
///
/// Currently implemented handlers for:
/// Checkbox, Radiobutton, Edit Control,  

class CAutoPropertyPage : public CPropertyPage
{
public:
	CAutoPropertyPage() {}
	CAutoPropertyPage( UINT resId );

	virtual void ReadProfile() = 0;

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand( WPARAM wp, LPARAM lp );

	afx_msg LRESULT OnAfterInitDialog( WPARAM, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	bool m_isInitialized;
};