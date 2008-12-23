/* This file is part of the installer library "WiX_CptUI".
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

#include "stdafx.h"

using namespace std;

//----------------------------------------------------------------------------------------------

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD reason, LPVOID lpReserved )
{
	// Initialize / finalize WiX custom action library.
	if( reason == DLL_PROCESS_ATTACH )
		WcaGlobalInitialize( hModule );
	else if( reason == DLL_PROCESS_DETACH )
		WcaGlobalFinalize();

    return TRUE;
}

//----------------------------------------------------------------------------------------------

UINT __stdcall MsgBox(MSIHANDLE hModule)
{
	MessageBox(NULL, _T("CustomAction \"MsgBox\" running"), _T("Installer"), MB_ICONINFORMATION);
	return ERROR_SUCCESS;
}
