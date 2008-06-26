#pragma once
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

#include <string>

/// Get an MSI property string from an installation session.
std::wstring GetMsiProp( MSIHANDLE hInst, LPCWSTR name ); 
inline std::wstring GetMsiProp( LPCWSTR name ) { return GetMsiProp( WcaGetInstallHandle(), name ); }

/// Get an MSI property and convert it to integer.
int GetMsiPropInt( MSIHANDLE hInst, LPCWSTR name );
inline int GetMsiPropInt( LPCWSTR name ) { return GetMsiPropInt( WcaGetInstallHandle(), name ); }

/// Returns true if given MSI property is defined.
bool HasMsiProp( MSIHANDLE hInst, LPCWSTR name );
inline bool HasMsiProp( LPCWSTR name ) { return HasMsiProp( WcaGetInstallHandle(), name ); }

/// Get an MSI record field string
std::wstring GetMsiRecordStr( MSIHANDLE hRec, int field );

/// Exception for MyWcaAddTempRecord
class MyWcaError : public std::exception
{
public:
	MyWcaError() : m_hr( 0 ), m_wwhat( L"MyWcaError" ) {}
	explicit MyWcaError( HRESULT hResult, const WCHAR* what = L"MyWcaError" ) :
		m_hr( hResult ), m_wwhat( what ) {}
	virtual const char *what() const { return "MyWcaError"; }
	virtual const WCHAR *wwhat() const { return m_wwhat; }
	virtual const HRESULT GetHResult() const { return m_hr; }
private:
	HRESULT m_hr;
	const WCHAR* m_wwhat;
};

/// Add temporary row to any MSI table, throws MyWcaError exception on error.
void MyWcaAddTempRecord(
    __inout MSIHANDLE* phTableView,
    __inout MSIHANDLE* phColumns,
    __in LPCWSTR wzTable,
    __in UINT uiUniquifyColumn,
    __in UINT cColumns,
    ... );
    
/// Add temp. row to registry table, throws MyWcaError exception on error.
inline void MyWcaAddTempRecordRegistry( MSIHANDLE* phTableView, MSIHANDLE* phColumns,
	LPCWSTR registry, msidbRegistryRoot root, LPCWSTR key, LPCWSTR name, LPCWSTR value, LPCWSTR component_ )
{
	MyWcaAddTempRecord( phTableView, phColumns, L"Registry", 1, 6, 
		registry, root, key, name, value, component_ );
}
