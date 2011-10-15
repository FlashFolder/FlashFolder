
#pragma once

/// Wrapper for HMODULE

class Module : boost::noncopyable
{
public:
	Module( HMODULE handle = NULL ) :
		m_handle( handle )
	{}

	Module( LPCTSTR path ) :
		m_handle( ::LoadLibrary( path ) )
	{}

	~Module() { Free(); }

	void Attach( HMODULE handle ) { Free(); m_handle = handle; }

	HMODULE Detach() { HMODULE h = m_handle; m_handle = NULL; return h; }

	void Free()
	{
		if( m_handle )
		{
			::FreeLibrary( m_handle );
			m_handle = NULL;
		}
	}

	operator HMODULE() const { return m_handle; }

private:
	HMODULE m_handle;
};
