#pragma once

//---------------------------------------------------------------------------

struct LogFile
{
	LogFile() : m_file( NULL ) {}
	
	bool Open( LPCWSTR fileNameOrPath )
	{	
		Close(); 
		
		WCHAR path[ MAX_PATH ] = L"";
		
		if( ::PathIsRelative( fileNameOrPath ) )
		{
			// Prepend directory of current process to filename
			::GetModuleFileName( NULL, path, _countof(path) );
			LPWSTR p = wcsrchr( path, '\\' );
			if( p ) *(++p) = 0;
			wcscat_s( path, fileNameOrPath );  	
		}
		else
		{
			wcscpy_s( path, fileNameOrPath );
		}

		return ( m_file = _wfopen( path, L"ab" ) ) != NULL; 
	}
		
	void Close() { if( m_file ) fclose( m_file ); }
	
	~LogFile() { Close(); }
	
	void Write( LPCWSTR text )
	{
		if( m_file )
		{
			SYSTEMTIME time = { 0 };
			::GetLocalTime( &time );
			fwprintf( m_file, L"%02d-%02d-%02d %02d:%02d:%02d.%03d (%02X): ", 
				time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
				::GetCurrentProcessId() & 0xFF );
		
			fwrite( text, sizeof( WCHAR ), wcslen( text ), m_file ); 
			fwrite( L"\r\n", sizeof(WCHAR), 2, m_file ); 
			fflush( m_file );
		}
	}
	
	FILE* m_file;
};
