#pragma once

//---------------------------------------------------------------------------

class LogFile
{
public:
	LogFile() : 
		m_file( NULL ),
		m_pid( 0 )	
	{}

	~LogFile() { Close(); }
	
	bool Open( LPCWSTR path );
	void Close();
	void Write( LPCWSTR text );
		
	static void GetPath( LPWSTR path, LPCWSTR fileNameOrPath );
	
private:
	FILE* m_file;
	DWORD m_pid;
};

