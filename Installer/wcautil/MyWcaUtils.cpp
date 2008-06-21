/*
//    Copyright (C) 2008 zett42 ( zett42 at users.sourceforge.net )
//
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
*/
#include "precomp.h"
#include "MyWcaUtils.h"

#include <vector>

using namespace std;

//----------------------------------------------------------------------------------------------

std::wstring GetMsiProp( MSIHANDLE hInst, LPCWSTR name )
{
	DWORD size = 1024;
	vector<WCHAR> buf( size );
	int res = ::MsiGetPropertyW( hInst, name, &buf[ 0 ], &size );
	if( res == ERROR_MORE_DATA )
	{
		++size;  // returned size doesn't count \0
		buf.resize( size );
		res = ::MsiGetPropertyW( hInst, name, &buf[ 0 ], &size );
	}
	if( res != ERROR_SUCCESS )
		return wstring();
	return wstring( &buf[ 0 ], size );
}

//----------------------------------------------------------------------------------------------

int GetMsiPropInt( MSIHANDLE hInst, LPCWSTR name )
{
	WCHAR buf[ 32 ];
	DWORD size = _countof( buf );
	if( ::MsiGetPropertyW( hInst, name, buf, &size ) == ERROR_SUCCESS )
		return _wtoi( buf );
	return 0;
} 

//----------------------------------------------------------------------------------------------

bool HasMsiProp( MSIHANDLE hInst, LPCWSTR name )
{
	DWORD cchProperty = 0;
	WCHAR szEmpty[1] = L"";
	::MsiGetPropertyW( hInst, name, szEmpty, &cchProperty );
	return 0 < cchProperty; // property is set if the length is greater than zero
}

//----------------------------------------------------------------------------------------------

std::wstring GetMsiRecordStr( MSIHANDLE hRec, int field )
{
	DWORD size = 1024;
	vector<WCHAR> buf( size );
	int res = ::MsiRecordGetStringW( hRec, field, &buf[ 0 ], &size );
	if( res == ERROR_MORE_DATA )
	{
		++size;  // returned size doesn't count \0
		buf.resize( size );
		res = ::MsiRecordGetStringW( hRec, field, &buf[ 0 ], &size );
	}
	if( res != ERROR_SUCCESS )
		return wstring();
	return wstring( &buf[ 0 ], size );
}

//----------------------------------------------------------------------------------------------

void MyWcaAddTempRecord(
    __inout MSIHANDLE* phTableView,
    __inout MSIHANDLE* phColumns,
    __in LPCWSTR wzTable,
    __out_opt MSIDBERROR* pdbError,
    __in UINT uiUniquifyColumn,
    __in UINT cColumns,
    ... )
{
	va_list args;
	va_start( args, cColumns );
	HRESULT hr = WcaAddTempRecordV(  phTableView, phColumns, wzTable, pdbError, uiUniquifyColumn, cColumns, args );
	va_end( args );
	if( ! SUCCEEDED( hr ) )
	{
		WCHAR msg[ 256 ] = L"";
		swprintf_s( msg, L"Failed to add temporary row to table '%s'", wzTable );
		throw MyWcaError( hr, msg ); 
	}	
}
 