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
#include "stdafx.h"
#include "HookInstaller.h"
#include "fflib_exports.h"

//---------------------------------------------------------------------------

bool FFHookInstaller::Install()
{
	if( m_hHook )
	{
		::SetLastError( ERROR_ALREADY_EXISTS );
		return false;
	}

	// Force the delay-load DLL to load NOW so that we can reference the hook function.
	HINSTANCE hMod = GetFFLibHandle();
	// Need to use GetProcAddress instead of func. pointer because of delay-loading.
	HOOKPROC hookProc = (HOOKPROC) ::GetProcAddress( hMod, "FFHook_CBT" );
	
	m_hHook = ::SetWindowsHookEx( WH_CBT, hookProc, hMod, 0 );

	return m_hHook != NULL;
}

//---------------------------------------------------------------------------
	
bool FFHookInstaller::Uninstall()
{
    if( m_hHook )
    {
		if( ::UnhookWindowsHookEx( m_hHook ) )
		{
			m_hHook = NULL;
			return true;
		}
    }

	return false;
}
