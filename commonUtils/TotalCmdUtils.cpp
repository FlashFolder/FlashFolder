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

#include "TotalCmdUtils.h"
#include "StringUtils.h"

//-------------------------------------------------------------------------------------------------

HWND CTotalCmdUtils::FindTopTCmdWnd()
{
    HWND hwnd = NULL;
    ::EnumWindows( FindTopWnd_Proc, reinterpret_cast<LPARAM>( &hwnd ) );
    return hwnd;
}

//-------------------------------------------------------------------------------------------------

BOOL CALLBACK CTotalCmdUtils::FindTopWnd_Proc( HWND hwnd, LPARAM lParam )
{
    TCHAR classname[256] = _T("");
    if( ::GetClassName( hwnd, classname, 255 ) > 0 )
        if( _tcscmp( classname, _T("TTOTAL_CMD") ) == 0 )
        {
            HWND* phwndRes = reinterpret_cast<HWND*>( lParam );
            *phwndRes = hwnd;
            return FALSE;
        }

    return TRUE;
}


//-------------------------------------------------------------------------------------------------

bool CTotalCmdUtils::GetDirs( LPTSTR pLeftDir, unsigned leftDirLen, 
                              LPTSTR pRightDir, unsigned rightDirLen )
{
    pLeftDir[0] = pRightDir[0] = 0;
    if( ! m_hwnd ) return false;

    if( m_hwndLeft && m_hwndRight )
    {
        ::GetWindowText( m_hwndLeft, pLeftDir, leftDirLen );
        ::GetWindowText( m_hwndRight, pRightDir, rightDirLen );        
        return true;
    }

    CFindSubWindowsData data;
    data.m_thisptr = this;
    data.m_pLeftDir = pLeftDir;
    data.m_leftDirLen = leftDirLen;
    data.m_pRightDir = pRightDir;
    data.m_rightDirLen = rightDirLen;
    
    ::EnumChildWindows( m_hwnd, FindSubWindows_Proc, reinterpret_cast<LPARAM>( &data ) );
    
    return m_hwndLeft || m_hwndRight;
}

//-------------------------------------------------------------------------------------------------

BOOL CALLBACK CTotalCmdUtils::FindSubWindows_Proc( HWND hwnd, LPARAM lParam )
{
    CFindSubWindowsData *pData = reinterpret_cast<CFindSubWindowsData*>( lParam );
    TCHAR wndtext[MAX_PATH + 1] = _T("");
    if( ::GetWindowText( hwnd, wndtext, MAX_PATH ) > 0 )
        if( IsFilePath( wndtext ) )
        {
            //--- check for command line label - it contains the path of the active dir
            bool bIsActive = false;
            if( ! pData->m_thisptr->m_hwndActive )
            {
                size_t len = _tcslen( wndtext );
                TCHAR* pLast = _tcsninc( wndtext, len - 1 );
                if( *pLast == _T('>') )
                {
                    pData->m_thisptr->m_hwndActive = hwnd;                
                    bIsActive = true;
                }
            }

            if( ! bIsActive )
            {
                RECT rc;
                ::GetWindowRect( hwnd, &rc );
                POINT pt = { rc.left, rc.top };
                ::ScreenToClient( pData->m_thisptr->m_hwnd, &pt );
                if( pt.x < 20 )
                {
                    pData->m_thisptr->m_hwndLeft = hwnd;
                    ::GetWindowText( hwnd, pData->m_pLeftDir, pData->m_leftDirLen );
                    TCHAR* p = _tcsrchr( pData->m_pLeftDir, _T('\\') );
                    if( p ) *p = 0;
                }
                else
                {
                    pData->m_thisptr->m_hwndRight = hwnd;
                    ::GetWindowText( hwnd, pData->m_pRightDir, pData->m_rightDirLen );                
                    TCHAR* p = _tcsrchr( pData->m_pRightDir, _T('\\') ); 
                    if( p ) *p = 0;
                }
            }
        }

    return ! (pData->m_thisptr->m_hwndRight && pData->m_thisptr->m_hwndLeft && 
              pData->m_thisptr->m_hwndActive );
}

//-------------------------------------------------------------------------------------------------

bool GetTotalCmdLocation( tstring* pInstallDir, tstring* pIniPath )
{
	// Try to get EXE path from one of following locations:
	// - registry HKCU
	// - registry HKLM
	// - Windows directory

	if( pInstallDir )
		*pInstallDir = _T("");
	if( pIniPath )
		*pIniPath = _T("");

	bool needInstDir = pInstallDir != NULL; 
	bool needIniPath = pIniPath != NULL;

	TCHAR pathBuf[ MAX_PATH + 1 ] = _T("");

	HKEY hRegRoots[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
	for( int iRoot = 0; iRoot != 2; ++iRoot )
	{
		RegKey reg;
		if( reg.Open( hRegRoots[ iRoot ], _T("Software\\Ghisler\\Total Commander") ) ) 
		{
			tstring instDir = reg.GetString( _T("InstallDir") );
			::ExpandEnvironmentStrings( instDir.c_str(), pathBuf, MAX_PATH );
			::PathAddBackslash( pathBuf );
			instDir = pathBuf;
			if( needInstDir && ! instDir.empty() )
			{
				*pInstallDir = instDir;
				needInstDir = false;
			}

			tstring iniPath = reg.GetString( _T("IniFileName") ); 
			if( needIniPath && ! iniPath.empty() )
			{
				::ExpandEnvironmentStrings( iniPath.c_str(), pathBuf, MAX_PATH );
				tstring tmpPath;
				if( IsRelativePath( pathBuf ) )
					tmpPath = instDir + tstring( pathBuf );
				else
					tmpPath = pathBuf;
				::PathCanonicalize( pathBuf, tmpPath.c_str() );
				*pIniPath = pathBuf;

				needIniPath = false;
			}
		}
		if( ! needInstDir && ! needIniPath )
			return true;
	}

	if( needIniPath )
	{
		TCHAR path[ MAX_PATH + 1 ] = _T("");
		::GetWindowsDirectory( path, MAX_PATH );
		::PathAddBackslash( path );
		StringCbCat( path, sizeof(path), _T("wincmd.ini") );	
		if( FileExists( path ) )
		{
			*pIniPath = path;
			needIniPath = false;
		}
	}

	if( ! needInstDir && ! needIniPath )
		return true;
	return false;
}

//-----------------------------------------------------------------------------------------------

void SplitTcCommand( LPCTSTR pCmd, tstring* pToken, tstring* pArgs )
{
	*pToken = _T("");
	if( pArgs )
		*pArgs = _T("");

	LPCTSTR p = _tcschr( pCmd, ' ' );
	if( p )
	{
		*pToken = tstring( pCmd, p - pCmd );
		if( pArgs )
		{
			while( *p == ' ' ) ++p;
			*pArgs = p;
		}
	}
}
