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
#include <commonUtils\Registry.h>

using namespace std;

namespace totalcmdutils
{

//-------------------------------------------------------------------------------------------------

BOOL CALLBACK FindTopWnd_Proc( HWND hwnd, LPARAM lParam )
{
    WCHAR classname[256] = L"";
    if( ::GetClassName( hwnd, classname, 255 ) > 0 )
        if( wcscmp( classname, L"TTOTAL_CMD" ) == 0 )
        {
            HWND* phwndRes = reinterpret_cast<HWND*>( lParam );
            *phwndRes = hwnd;
            return FALSE;
        }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

HWND FindTopTcWnd( bool currentThreadOnly )
{
    HWND hwnd = NULL;
	if( currentThreadOnly )
		::EnumThreadWindows( ::GetCurrentThreadId(), FindTopWnd_Proc, reinterpret_cast<LPARAM>( &hwnd ) );
	else
		::EnumWindows( FindTopWnd_Proc, reinterpret_cast<LPARAM>( &hwnd ) );
    return hwnd;
}

//-------------------------------------------------------------------------------------------------

bool IsTcPathControl( HWND hwnd )
{
    WCHAR wndtext[ MAX_PATH + 1 ] = L"";
    if( ::GetWindowText( hwnd, wndtext, MAX_PATH ) > 0 )
        if( IsFilePath( wndtext ) )
        {
            // check if this is not the command line label
            size_t len = wcslen( wndtext );
            WCHAR* pLast = _wcsninc( wndtext, len - 1 );
            return *pLast != '>';
		}
	return false;
}

//-----------------------------------------------------------------------------------------------

void TcInstance::SetTCmdWnd( HWND hwndTotalCmd ) 
{ 
    m_hwnd = hwndTotalCmd; 
	m_hwndLeft = m_hwndRight = m_hwndActive = NULL; 
    if( ! m_hwnd ) return;

	// Find the child windows which contain the left / right path
    FindSubWindowsData data;
    data.m_thisptr = this;
    ::EnumChildWindows( m_hwnd, FindSubWindows_Proc, reinterpret_cast<LPARAM>( &data ) );

	// Determine left and right windows
	if( m_hwndLeft && m_hwndRight )
	{
		RECT rc1, rc2;
		::GetWindowRect( m_hwndLeft, &rc1 );
		::GetWindowRect( m_hwndRight, &rc2 );
		if( rc1.left > rc2.left )
			std::swap( m_hwndLeft, m_hwndRight );
	}    
}

//-----------------------------------------------------------------------------------------------

bool TcInstance::IsLeftDirActive() const
{
	WCHAR leftDir[ MAX_PATH + 1 ];
	WCHAR activeDir[ MAX_PATH + 1 ];
	GetDirs( leftDir, MAX_PATH );
	GetActiveDir( activeDir, MAX_PATH );
	return ComparePath( leftDir, activeDir ) == 0;
} 

//-----------------------------------------------------------------------------------------------

bool TcInstance::GetDirs( LPWSTR pLeftDir, unsigned leftDirLen, 
                              LPWSTR pRightDir, unsigned rightDirLen ) const
{
	if( pLeftDir )
	{
		pLeftDir[ 0 ] = NULL;
		if( m_hwndLeft )
			GetPathFromTcControl( m_hwndLeft, pLeftDir, leftDirLen );
	}
	if( pRightDir )
	{
		pRightDir[ 0 ] = NULL;
		if( m_hwndRight )
			GetPathFromTcControl( m_hwndRight, pRightDir, rightDirLen );
	}    
    return m_hwndLeft || m_hwndRight;
}

//-----------------------------------------------------------------------------------------------

bool TcInstance::GetActiveDir( LPWSTR pDir, unsigned len ) const
{
	pDir[ 0 ] = 0;
	if( ! m_hwndActive )
		return false;

    WCHAR wndtext[ MAX_PATH + 1 ] = L"";
    if( ::GetWindowText( m_hwndActive, wndtext, MAX_PATH ) > 0 )
	{
        size_t len = wcslen( wndtext );
        WCHAR* pLast = wndtext + len - 1;
        if( *pLast == L'>' )
			*pLast = 0;

		StringCchCopy( pDir, len, wndtext );

		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------

BOOL CALLBACK TcInstance::FindSubWindows_Proc( HWND hwnd, LPARAM lParam )
{
    FindSubWindowsData *pData = reinterpret_cast<FindSubWindowsData*>( lParam );
    WCHAR wndtext[MAX_PATH + 1] = L"";
    if( ::GetWindowText( hwnd, wndtext, MAX_PATH ) > 0 )
        if( IsFilePath( wndtext ) )
        {
            //--- check for command line label - it contains the path of the active dir
            bool bIsActive = false;
            if( ! pData->m_thisptr->m_hwndActive )
            {
                size_t len = wcslen( wndtext );
                WCHAR* pLast = wndtext + len - 1;
                if( *pLast == L'>' )
                {
                    pData->m_thisptr->m_hwndActive = hwnd;                
                    bIsActive = true;
                }
            }

            if( ! bIsActive )
            {
                if( ! pData->m_thisptr->m_hwndLeft )
                    pData->m_thisptr->m_hwndLeft = hwnd;
                else if( ! pData->m_thisptr->m_hwndRight )
                    pData->m_thisptr->m_hwndRight = hwnd;
            }
        }

    return ! (pData->m_thisptr->m_hwndRight && pData->m_thisptr->m_hwndLeft && 
              pData->m_thisptr->m_hwndActive );
}

//-------------------------------------------------------------------------------------------------

bool GetTotalCmdLocation( wstring* pInstallDir, wstring* pIniPath )
{
	// Try to get EXE path from one of following locations:
	// - registry HKCU
	// - registry HKLM
	// - Windows directory

	if( pInstallDir )
		*pInstallDir = L"";
	if( pIniPath )
		*pIniPath = L"";

	bool needInstDir = pInstallDir != NULL; 
	bool needIniPath = pIniPath != NULL;

	WCHAR pathBuf[ MAX_PATH + 1 ] = L"";

	HKEY hRegRoots[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
	for( int iRoot = 0; iRoot != 2; ++iRoot )
	{
		RegKey reg;
		if( reg.Open( hRegRoots[ iRoot ], L"Software\\Ghisler\\Total Commander" ) ) 
		{
			wstring instDir = reg.GetString( L"InstallDir" );
			::ExpandEnvironmentStrings( instDir.c_str(), pathBuf, MAX_PATH );
			::PathAddBackslash( pathBuf );
			instDir = pathBuf;
			if( needInstDir && ! instDir.empty() )
			{
				*pInstallDir = instDir;
				needInstDir = false;
			}

			wstring iniPath = reg.GetString( L"IniFileName" ); 
			if( needIniPath && ! iniPath.empty() )
			{
				::ExpandEnvironmentStrings( iniPath.c_str(), pathBuf, MAX_PATH );
				wstring tmpPath;
				if( IsRelativePath( pathBuf ) )
					tmpPath = instDir + wstring( pathBuf );
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
		WCHAR path[ MAX_PATH + 1 ] = L"";
		::GetWindowsDirectory( path, MAX_PATH );
		::PathAddBackslash( path );
		StringCbCat( path, sizeof(path), L"wincmd.ini" );	
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

void SplitTcCommand( LPCWSTR pCmd, wstring* pToken, wstring* pArgs )
{
	*pToken = L"";
	if( pArgs )
		*pArgs = L"";

	LPCWSTR p = wcschr( pCmd, ' ' );
	if( p )
	{
		*pToken = wstring( pCmd, p - pCmd );
		if( pArgs )
		{
			while( *p == ' ' ) ++p;
			*pArgs = p;
		}
	}
}

//-----------------------------------------------------------------------------------------------

bool SetCurrentPathes( HWND hWndTC, LPCWSTR pPath1, LPCWSTR pPath2, DWORD flags )
{
	if( ! hWndTC )
		hWndTC = FindTopTcWnd();
	if( ! hWndTC )
		return false;

	// Pathes must be ANSI encoded for TC
	char ansiBuf[ MAX_PATH + 1 ];
	char cmdBuf[ MAX_PATH * 2 + 4 ];
	char chFlags[ 3 ] = "";

	::WideCharToMultiByte( CP_ACP, 0, pPath1, -1, cmdBuf, MAX_PATH, 0, NULL );
	::WideCharToMultiByte( CP_ACP, 0, pPath2, -1, ansiBuf, MAX_PATH, 0, NULL );
	StringCbCatA( cmdBuf, sizeof(cmdBuf), "\r" );
	StringCbCatA( cmdBuf, sizeof(cmdBuf), ansiBuf );
	if( flags & STC_SOURCE_AND_TARGET )
		StringCbCatA( chFlags, sizeof(chFlags), "S" );
	if( flags & STC_BACKGROUND_TAB )
		StringCbCatA( chFlags, sizeof(chFlags), "T" );
	size_t len = strlen( cmdBuf );
	StringCbCatA( cmdBuf + len + 1, sizeof(cmdBuf) - len - 1, chFlags );

	// prepare TC protocol
	COPYDATASTRUCT cd;
	cd.dwData = 'C' | 'D' << 8;
	cd.cbData = static_cast<DWORD>( len + 1 + strlen( chFlags ) );
	cd.lpData = cmdBuf;

	return ::SendMessage( hWndTC, WM_COPYDATA, 0, reinterpret_cast<LPARAM>( &cd ) ) != 0;
}

//-----------------------------------------------------------------------------------------------

void GetPathFromTcControl( HWND hwnd, LPWSTR pPath, size_t nSize )
{ 
	pPath[ 0 ] = 0;
	::GetWindowText( hwnd, pPath, static_cast<int>( nSize ) );
	if( WCHAR* p = wcsrchr( pPath, '\\' ) )
		if( p - pPath > 2 )
			*p = 0;
		else
			*(++p) = 0;
}

//----------------------------------------------------------------------------------------------------

bool LoadFavoritesMenu( FavMenu* items, LPCWSTR iniPath, LPCWSTR iniSection )
{
	items->clear();

	WCHAR key[ 32 ] = L"";
	WCHAR buf[ 1024 ] = L"";

	if( ::GetPrivateProfileString( iniSection, L"RedirectSection", L"", buf, _countof( buf ), iniPath ) )
		return false;

	for( int i = 1;; ++i )
	{
		swprintf_s( key, L"menu%d", i );
		if( ! ::GetPrivateProfileString( iniSection, key, L"", buf, _countof( buf ), iniPath ) )
			break;

		FavMenuItem item;
		item.menu = wstring( buf );

		swprintf_s( key, L"cmd%d", i );
		if( ::GetPrivateProfileString( iniSection, key, L"", buf, _countof( buf ), iniPath ) )
			item.cmd = wstring( buf );
	
		swprintf_s( key, L"path%d", i );
		if( ::GetPrivateProfileString( iniSection, key, L"", buf, _countof( buf ), iniPath ) )
			item.path = wstring( buf );

		swprintf_s( key, L"param%d", i );
		if( ::GetPrivateProfileString( iniSection, key, L"", buf, _countof( buf ), iniPath ) )
			item.param = wstring( buf );

		items->push_back( item );
	}

	return true;
}

//----------------------------------------------------------------------------------------------------

bool SaveFavoritesMenu( const FavMenu& items, LPCWSTR iniPath, LPCWSTR iniSection )
{
	WCHAR key[ 32 ] = L"";
	WCHAR buf[ 1024 ] = L"";

	if( ::GetPrivateProfileString( iniSection, L"RedirectSection", L"", buf, _countof( buf ), iniPath ) )
		return false;
	
	// Wipe out old menu.

	if( ! ::WritePrivateProfileSection( iniSection, L"\0", iniPath ) )
		return false;

	// Write modified menu back.

	for( size_t i = 0; i < items.size(); ++i )
	{
		const FavMenuItem& item = items[ i ];

		swprintf_s( key, L"menu%d", i + 1 );
		::WritePrivateProfileString( iniSection, key, item.menu.c_str(), iniPath );

		if( ! item.cmd.empty() )
		{
			swprintf_s( key, L"cmd%d", i + 1 );
			::WritePrivateProfileString( iniSection, key, item.cmd.c_str(), iniPath );
		}

		if( ! item.path.empty() )
		{
			swprintf_s( key, L"path%d", i + 1 );
			::WritePrivateProfileString( iniSection, key, item.path.c_str(), iniPath );
		}

		if( ! item.param.empty() )
		{
			swprintf_s( key, L"param%d", i + 1 );
			::WritePrivateProfileString( iniSection, key, item.path.c_str(), iniPath );
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------

} //namespace