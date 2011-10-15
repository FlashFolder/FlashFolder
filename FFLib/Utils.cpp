
#include "StdAfx.h"
#include "Utils.h"

//-----------------------------------------------------------------------------------------------
/// Get path of "FFConfig.exe" - prefer x64 version, if available. 

void GetConfigProcessPath( HINSTANCE hInstDll, LPWSTR path, DWORD nChars )
{
	WCHAR dir[ 4096 ] = L"";
	GetAppDir( hInstDll, dir, _countof( dir ) );
	wcscpy_s( path, nChars, dir );
	wcscat_s( path, nChars, L"FFConfig64.exe" );
	if( FileExists( path ) )
		return;
	wcscpy_s( path, nChars, dir );
	wcscat_s( path, nChars, L"FFConfig.exe" );	
}
