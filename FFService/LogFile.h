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
	
	bool Open( LPCWSTR fileNameOrPath );
	void Close();
	void Write( LPCWSTR text );
	
private:
	FILE* m_file;
	DWORD m_pid;
};
