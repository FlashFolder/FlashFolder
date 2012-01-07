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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

//-----------------------------------------------------------------------------------------------

class CApp : public CWinApp
{
public:
	static void SetReadDefaults( bool defaults ) { s_app.m_isReadDefaults = defaults; }

	static Profile& GetReadProfile() 
	{ 
		if( s_app.m_isReadDefaults )
			return s_app.m_profileDefaults;
		return s_app.m_profile;  
	}

	static Profile& GetWriteProfile()            { return s_app.m_profile; }

private:
	static CApp s_app;

	CApp();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()

	bool m_isReadDefaults;	
	RegistryProfile m_profile;
	MemoryProfile m_profileDefaults;
};

