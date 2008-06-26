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

const WCHAR MY_PRODUCT_REG[] = L"Software\\zett42\\FlashFolder";

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

//----------------------------------------------------------------------------------------------
/// Immediate CA for adding temporary rows to Registry table so we can better decide WHEN 
/// regkeys / regvalues should be created or removed. 
///
/// This way it is possible for instance to set a registry value only on first install but
/// neither "repair" nor uninstall it (e. g. for changing existing system regvalues).
///
/// Another example is a conditional uninstall of a regkey (standard MSI evaluates conditions
/// only during install / reinstall).
///
/// Using temporary rows is better than doing this stuff directly with the registry API since
/// we don't need to care about rollback as MSI is doing this automatically.

UINT __stdcall CA_CustomRegistry( MSIHANDLE hInst )
{
	HRESULT hResult = WcaInitialize( hInst, L"CA_CustomRegistry" );
	if( FAILED( hResult ) ) return ERROR_INSTALL_FAILURE;
	
	// Get installation state
	bool installed = HasMsiProp( L"Installed" ) != 0;
	bool removeAll = GetMsiProp( L"REMOVE" ) == L"ALL";
	
	try
	{
		// On uninstall, remove user regkeys, if desired.
		if( removeAll && ! HasMsiProp( L"MY_KEEP_SETTINGS" ) )	
		{
			WcaLog( LOGMSG_STANDARD, L"Scheduling removal of user settings" );
			PMSIHANDLE hTable, hColumns;
			
			MyWcaAddTempRecordRegistry( &hTable, &hColumns,  
				L"RemoveProductReg", msidbRegistryRootLocalMachine, MY_PRODUCT_REG, L"-", NULL, L"ProgramRegkeys" );			
		}

		WcaDumpTable( L"Registry" );

		return WcaFinalize( ERROR_SUCCESS );
	}
	catch( const MyWcaError& e )
	{
		WcaLogError( e.GetHResult(), e.wwhat() );
		return WcaFinalize( ERROR_INSTALL_FAILURE );	
	}
}