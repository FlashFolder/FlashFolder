
#include "stdafx.h"
#include "LogFile.h"
#include "../_version.h"

//---------------------------------------------------------------------------

bool LogFile::Open( LPCWSTR path )
{	
	Close(); 
	
	m_pid = ::GetCurrentProcessId();
		
	if( m_file = _wfsopen( path, L"ab", _SH_DENYWR ) )
	{
		fseek( m_file, 0, SEEK_END );
		fpos_t pos = 0;	fgetpos( m_file, &pos );
		if( pos == 0 )
		{
			WORD utf16bom = 0xFEFF;
			fwrite( &utf16bom, 2, 1, m_file );
		}

		Write( L"===== Logging started =====" );

		WCHAR s[ 2048 ];
		
		swprintf_s( s, L"Process version: %d.%d.%d.%d, %s",
			APP_VER_MAJOR, APP_VER_MINOR, APP_VER_BUILD, APP_VER_MICRO, APP_BUILD_TIME ); 
		Write( s );
	
		wcscpy_s( s, L"Cmd: " );
		wcscat_s( s, ::GetCommandLine() );
		Write( s );
	
		return true;
	}				
	return false; 
}

//---------------------------------------------------------------------------
		
void LogFile::Close() 
{ 
	if( m_file ) 
	{
		Write( L"===== Logging ended =====" );	
		fclose( m_file ); 
		m_file = NULL;
	}
}

//---------------------------------------------------------------------------
	
void LogFile::Write( LPCWSTR text )
{
	if( m_file )
	{
		SYSTEMTIME time = { 0 };
		::GetLocalTime( &time );
		fwprintf( m_file, L"%02d-%02d-%02d %02d:%02d:%02d.%03d (%02X): ", 
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
			m_pid & 0xFF );
	
		fwrite( text, sizeof( WCHAR ), wcslen( text ), m_file ); 
		fwrite( L"\r\n", sizeof(WCHAR), 2, m_file ); 
		fflush( m_file );
	}
}

//----------------------------------------------------------------------------------------------------

void LogFile::GetPath( LPWSTR path, LPCWSTR fileNameOrPath )
{
	*path = 0;
	
	if( ::PathIsRelative( fileNameOrPath ) )
	{
		// Prepend directory of current process to filename
		::GetModuleFileName( NULL, path, MAX_PATH );
		LPWSTR p = wcsrchr( path, '\\' );
		if( p ) *(++p) = 0;
		wcscat_s( path, MAX_PATH, fileNameOrPath );  	
	}
	else
	{
		wcscpy_s( path, MAX_PATH, fileNameOrPath );
	}
}
