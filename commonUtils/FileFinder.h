
#pragma once

#include <tchar.h>

/// FindFirstFile() / FindNextFile() wrapper.
/// Automatically skips "." and ".." folders.

class FileFinder : public boost::noncopyable
{
public:
	FileFinder( LPCTSTR dir, LPCTSTR filter = _T("*.*") )
	{
		memset( &m_data, 0, sizeof( m_data ) );
		
		TCHAR path[ 1024 ];
		_tcscpy_s( path, dir );
		::PathAddBackslash( path );
		_tcscat_s( path, filter );

		m_hFind = ::FindFirstFile( path, &m_data );
		SkipDots();
	}

	void FindNext()
	{
		if( FindNextInternal() )
			SkipDots();
	}

	~FileFinder()
	{
		if( IsValid() )
			::FindClose( m_hFind );
	}

	bool IsValid() const { return m_hFind != INVALID_HANDLE_VALUE; }

	operator bool() const { return IsValid(); }

	const WIN32_FIND_DATA& GetData() const { return m_data; }

	ULONGLONG GetFileSize() const
		{ return static_cast<ULONGLONG>( m_data.nFileSizeHigh ) << 32 |
		         static_cast<ULONGLONG>( m_data.nFileSizeLow ); }

	DWORD GetAttributes() const { return m_data.dwFileAttributes; }

	bool IsDir() const { return ( GetAttributes() & FILE_ATTRIBUTE_DIRECTORY ) != 0; }

	FILETIME GetCreationTime() const { return m_data.ftCreationTime; }

	FILETIME GetLastAccessTime() const { return m_data.ftLastAccessTime; }

	FILETIME GetLastWriteTime() const { return m_data.ftLastWriteTime; }

	LPCTSTR GetFileName() const { return m_data.cFileName; }

	LPCTSTR GetAlternateFileName() const { return m_data.cAlternateFileName; }

private:

	bool FindNextInternal()
	{
		if( ::FindNextFile( m_hFind, &m_data ) )
			return true;

		::FindClose( m_hFind );
		m_hFind = INVALID_HANDLE_VALUE;
		memset( &m_data, 0, sizeof( m_data ) );
		return false;
	}

	void SkipDots()
	{
		while( IsValid() && 
		       ( _tcscmp( m_data.cFileName, _T(".") ) == 0 ||
		         _tcscmp( m_data.cFileName, _T("..") ) == 0 ) )
			FindNextInternal();
	}

	HANDLE m_hFind;
	WIN32_FIND_DATA m_data;
};
