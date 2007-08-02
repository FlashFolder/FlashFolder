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

#include <stdafx.h>
#include <shlwapi.h>

#include <assert.h>

#include "utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------------------
// GetAppDir()
//
//   retrieves the directory where the specified application or DLL is located
//   szDir must point to a buffer with a size of at least MAX_PATH characters
//-----------------------------------------------------------------------------------------
void GetAppDir( HINSTANCE hInstApp, LPTSTR szDir)
{
	szDir[0] = 0;
    ::GetModuleFileName( hInstApp, szDir, MAX_PATH - 1 );
	LPTSTR p = _tcsrchr( szDir, _T('\\') );
	if( p ) p[1] = 0;
}

//-----------------------------------------------------------------------------------------------

void GetAppFilename( HINSTANCE hInstApp, LPTSTR pName )
{
	*pName = 0;
	TCHAR exePath[ MAX_PATH + 1 ] = _T("");
	::GetModuleFileName( NULL, exePath, MAX_PATH );
	if( LPTSTR p = _tcsrchr( exePath, _T('\\') ) )
		StringCchCopy( pName, MAX_PATH, p + 1 );
}

//-----------------------------------------------------------------------------------------

bool IsFilePath( LPCTSTR path )
{
    if( ! path ) return false;
    if( path[0] == 0 ) return false;
    if( path[1] == 0 ) return false;
    if( _istalpha( path[0] ) && path[1] == _T(':') ) return true;
    if( path[0] == _T('\\') && path[1] == _T('\\') ) return true;
    return false;
}

//------------------------------------------------------------------------------

bool IsRelativePath( LPCTSTR path )
{
	if( path[0] == 0 ) return false;
	if( path[1] == 0 ) return false;
	if( path[0] == _T('\\') && path[1] == _T('\\') )
		return false;
	if( _istalpha( path[0] ) && path[1] == _T(':') )
		return false;
	return true;
}

//-----------------------------------------------------------------------------------------------

int ComparePath( LPCTSTR path1, LPCTSTR path2 )
{
	TCHAR temp1[ MAX_PATH + 1 ];
	TCHAR temp2[ MAX_PATH + 1 ];
	if( ! ::PathCanonicalize( temp1, path1 ) )
	{
		assert( false );
		return 1;
	}
	::PathRemoveBackslash( temp1 );
	if( ! ::PathCanonicalize( temp2, path2 ) )
	{
		assert( false );
		return 1;
	}
	::PathRemoveBackslash( temp2 );
	return _tcsicmp( temp1, temp2 );
}

//-----------------------------------------------------------------------------------------------

void GetTempFilePath( LPTSTR pResult, LPCTSTR pPrefix )
{
	pResult[ 0 ] = 0;
	TCHAR tempDir[ MAX_PATH + 1 ] = _T("");
	::GetTempPath( MAX_PATH, tempDir );
	::GetTempFileName( tempDir, pPrefix, 0, pResult );  
}

//-----------------------------------------------------------------------------------------

bool IsIniSectionNotEmpty( LPCTSTR filename, LPCTSTR sectionName )
{
    TCHAR buffer[16];
    int count = ::GetPrivateProfileSection( sectionName, buffer, 
                        (sizeof(buffer) - 2) / sizeof(TCHAR), filename);
	return count > 0;
}

//-----------------------------------------------------------------------------------------
// AddTextInput()
//
//   adds keyboard input events (key down, key up) for a given string to a 
//   vector<INPUT> instance to be used with the ::SendInput() API 
//-----------------------------------------------------------------------------------------
void AddTextInput( std::vector<INPUT>* pInput, LPCTSTR pText )
{
	INPUT inp = { 0 };
	inp.type = INPUT_KEYBOARD;

	size_t len = _tcslen( pText );

#ifdef UNICODE
	LPCWSTR pwBuf = pText;
#else
	#error This code must be compiled with Unicode charset.
#endif
	
	for( size_t i = 0; i < len; i++ )
	{
		inp.ki.wScan = pwBuf[i];
		inp.ki.dwFlags = KEYEVENTF_UNICODE;
		pInput->push_back( inp );
		inp.ki.dwFlags |= KEYEVENTF_KEYUP; 
		pInput->push_back( inp );
	}
}

//-----------------------------------------------------------------------------------------------

void EnableDlgItem( HWND hDlg, UINT idCtrl, BOOL bEnable )
{
	HWND hCtrl = ::GetDlgItem( hDlg, idCtrl );
	if( ! hCtrl )
		return;
	HWND hFocus = ::GetFocus();
	::EnableWindow( hCtrl, bEnable );
	if( ! bEnable && hFocus == hCtrl )
		::SendMessage( hDlg, WM_NEXTDLGCTL, 0, 0 );			
}

//-----------------------------------------------------------------------------------------------

void DebugOut( LPCTSTR pFormat, ... )
{
	const size_t bufsize = 1024;
	TCHAR buf[ bufsize ];
	va_list args;
	va_start( args, pFormat );
	_vsntprintf( buf, bufsize - 1, pFormat, args );
	va_end( args );
	::OutputDebugString( buf );
}
