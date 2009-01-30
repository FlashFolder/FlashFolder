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

#include "NtKernelUtils.h"
#include <tchar.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//-------------------------------------------------------------------------------------------------

bool MapNtFilePathToUserPath( LPTSTR pUserPath, unsigned userPathLen, LPCTSTR ntPath )
{
    pUserPath[0] = 0;
    if( userPathLen <= 2 ) 
        return false;

    if( _tcsncmp( ntPath, _T("\\Device\\LanmanRedirector\\"), 25 ) == 0 )
    {
        // ntPath is a network path

		StringCchCopy( pUserPath, userPathLen, _T("\\\\") );
        StringCchCat( pUserPath, userPathLen, &ntPath[25]);
        return true;
    }
    else
    {
        // ntPath is a drive-based path, replace the device name part of the path 
        // with the drive letter by mapping all existing drive letters to their NT names

        DWORD drives = ::GetLogicalDrives();
        TCHAR drive[] = _T(" :");
        TCHAR ntDrivePath[256];
        for( int nDrive = 0; nDrive < 26; nDrive++ )
        {
            if( ( drives >> nDrive ) & 1 )
            {
                drive[0] = _T('A') + nDrive;
                
                if( ::QueryDosDevice( drive, ntDrivePath, 255 ) )
                {
                    size_t len = _tcslen( ntDrivePath );
                    if( len <= _tcslen( ntPath ) &&
                        _tcsncmp( ntPath, ntDrivePath, len ) == 0 )
                    {
						StringCchCopy( pUserPath, userPathLen, drive );
                        StringCchCat( pUserPath, userPathLen, &ntPath[len] );
                        return true;
                    }
                }
            }
        } //for       
    } //else

    return false;
}

//-------------------------------------------------------------------------------------------------

bool MapUserPathToNtFilePath( LPTSTR pNtPath, unsigned ntPathLen, LPCTSTR userPath )
{
    pNtPath[0] = 0;
    if( ntPathLen < 7 ) return false;     // minimum length for "\??\..."
    if( userPath[0] == 0 ) return false;
    if( userPath[1] == 0 ) return false;    

    if( userPath[0] == _T('\\') && userPath[1] == _T('\\') )
    {
        //**** TODO: network (UNC) path
    }
    else if( _istalpha( userPath[0] ) && userPath[1] == _T(':') )
    {
        // drive-based path

		StringCchCopy( pNtPath, ntPathLen, _T("\\??\\") );
        StringCchCat( pNtPath, ntPathLen, userPath ); 
    }
    
    return false;
}