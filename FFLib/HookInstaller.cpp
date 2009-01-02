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
#include "HookInstaller.h"

//---------------------------------------------------------------------------

bool FFHookInstaller::Install()
{
	// SECURITY: Define an ACL for the shared memory holding the hook handle 
	// to include read access for standard users. If we would not do this, 
	// nobody except admins could use the hook.

	CSecurityDesc sd;
	try 
	{
		CDacl dacl;
		dacl.AddAllowedAce( Sids::System(), FILE_MAP_ALL_ACCESS );
		dacl.AddAllowedAce( Sids::Admins(), FILE_MAP_ALL_ACCESS );
		dacl.AddAllowedAce( Sids::Users(), FILE_MAP_READ );
		sd.SetDacl( dacl );
	} 
	catch( CAtlException ) 
	{
		SetLastError( ERROR_ACCESS_DENIED );		
		return false;
	}	
	CSecurityAttributes sa( sd );	

	//--- Create the shared memory for the hook handle based on the ACL defined above.
	
	BOOL fileMapExisted = FALSE;
	HRESULT hr = m_sharedHHook.MapSharedMem( sizeof(HHOOK), L"FlashFolder_HHOOK_83472903",
		&fileMapExisted, &sa );
	if( FAILED( hr ) || fileMapExisted )
	{
		// SECURITY: We cannot reuse an existing shared memory object because we
		//    cannot trust it (it could have been created by lower-privileged process).
		int err = FAILED( hr ) ? ::GetLastError() : ERROR_ALREADY_EXISTS; 
		::SetLastError( err );
		return false;
	}

    //--- Install the hook and store the handle in the shared memory where it
    //    will be readable by all processes.

	// Force the delay-load DLL to load NOW so that we can reference the hook function.
	HINSTANCE hMod = GetFFLibHandle();
	// Need to use GetProcAddress instead of func. pointer because of delay-loading.
	HOOKPROC hookProc = (HOOKPROC) ::GetProcAddress( hMod, "FFHook_CBT" );
	
	HHOOK hHook = ::SetWindowsHookEx( WH_CBT, hookProc, hMod, 0 );
	*m_sharedHHook = hHook;

	return hHook != NULL;
}

//---------------------------------------------------------------------------
	
bool FFHookInstaller::Uninstall()
{
	HHOOK hHook = m_sharedHHook ? *m_sharedHHook : NULL;
    if( hHook )
    {
		if( ::UnhookWindowsHookEx( hHook ) )
		{
			*m_sharedHHook = NULL;
			return true;
		}
    }

	return false;
}
