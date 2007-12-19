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
 */
#include "StdAfx.h"
#include "DownloadThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// uncomment this for debugging
//#define TRACEME TRACE
#define TRACEME

//-----------------------------------------------------------------------------------------------
/*
 * Class for receiving progress information from URLDownloadToFile() API
 */
class CDownloadThread::DownloadCallback : public IBindStatusCallback
{
public:
	DownloadCallback( CDownloadThread* pThread ) :
		m_refCount( 0 ),
		m_pThread( pThread ) 
	{}

	//--- IUnknown boilerplate code
	
	virtual ULONG __stdcall AddRef() 
	{ 
		TRACEME("UpdateCheckCallback::AddRef %d\n", m_refCount );
		return ::InterlockedIncrement( &m_refCount ); 
	}
	virtual ULONG __stdcall Release()    
	{ 
		TRACEME("UpdateCheckCallback::Release %d\n", m_refCount );
		return ::InterlockedDecrement( &m_refCount ); 
	}
	virtual HRESULT __stdcall QueryInterface( const IID& riid, void** ppvObject )
	{
		TRACEME(_T("UpdateCheckCallback::QueryInterface\n"));

		*ppvObject = NULL;
		
		if( ::IsEqualIID( riid, __uuidof( IUnknown ) ) )
		{
			TRACEME(_T("UpdateCheckCallback::QueryInterface( IUnknown )\n"));
			*ppvObject = this;
		}
		else if( ::IsEqualIID( riid, __uuidof( IBindStatusCallback ) ) )
		{
			TRACEME(_T("UpdateCheckCallback::QueryInterface( IBindStatusCallback )\n"));
			*ppvObject = this;
		}

		if( *ppvObject )
		{
			(*reinterpret_cast<LPUNKNOWN*>( ppvObject ))->AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;		
	}

	//--- IBindStatusCallback implementation

    virtual HRESULT __stdcall OnProgress( ULONG ulProgress, ULONG ulProgressMax,
        ULONG ulStatusCode, LPCWSTR szStatusText )
	{
		TRACEME( "UpdateCheckCallback::OnProgress %d/%d, %d\n", ulProgress, ulProgressMax, ulStatusCode );

		CDownloadThread::StatusMsg status;
		status.ulProgress = ulProgress;
		status.ulProgressMax = ulProgressMax;
		status.ulStatusCode = ulStatusCode;
		status.szStatusText = szStatusText;

		// TODO: provide way to localize status messages
		CString msg;
		switch( ulStatusCode )
		{
			case BINDSTATUS_FINDINGRESOURCE: msg = _T("Finding resource %s");
			break;
			case BINDSTATUS_CONNECTING: msg = _T("Connecting to %s");
			break;
			case BINDSTATUS_REDIRECTING: msg = _T("Redirecting to %s");
			break;
			case BINDSTATUS_SENDINGREQUEST: msg = _T("Sending request...");
			break;
			case BINDSTATUS_MIMETYPEAVAILABLE: msg = _T("Mime type: %s");
			break;
			case BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE: msg = _T("Verified mime type: %s");
			break;	
			case BINDSTATUS_BEGINDOWNLOADDATA: msg = _T("Beginning download from %s");
			break;
			case BINDSTATUS_DOWNLOADINGDATA: msg = _T("Downloading...");
			break;
			case BINDSTATUS_ENDDOWNLOADDATA: msg = _T("Download finished.");
			break;
			case BINDSTATUS_USINGCACHEDCOPY: msg = _T("Using cached copy.");
			break;
			case BINDSTATUS_CACHEFILENAMEAVAILABLE: msg = _T("Cache filename: %s");
			break;
		}
		if( szStatusText )
			status.friendlyText.Format( msg, CString( szStatusText ) );
		else
			status.friendlyText = msg;
		status.friendlyText.Trim();

		::SendMessage( m_pThread->m_params.notifyHwnd, m_pThread->m_params.notifyMsg, CDownloadThread::ON_STATUS, 
		               reinterpret_cast<LPARAM>( &status ) );

		if( m_pThread->IsCanceled() )
			return E_ABORT;
		return S_OK;		
	}

    virtual HRESULT __stdcall OnStartBinding( DWORD dwReserved, IBinding *pib )
	{
		TRACEME( "UpdateCheckCallback::OnStartBinding\n" );
		return S_OK;
	}

    virtual HRESULT __stdcall OnStopBinding( HRESULT hresult, LPCWSTR szError)
	{
		TRACEME( "UpdateCheckCallback::OnStopBinding\n" );
		return S_OK;				
	}
    
    virtual HRESULT __stdcall GetBindInfo( DWORD *grfBINDF, BINDINFO *pbi ) 
	{
		*grfBINDF = 0;
		TRACEME("UpdateCheckCallback::GetBindInfo\n" );
		return S_OK;				
	}

    virtual HRESULT __stdcall GetPriority( LONG *pnPriority )
	{
		TRACEME( "UpdateCheckCallback::GetPriority\n" );
		*pnPriority = THREAD_PRIORITY_NORMAL;
		return S_OK;	
	}
    
    virtual HRESULT __stdcall OnLowResource( DWORD reserved )
	{
		TRACEME( "UpdateCheckCallback::OnLowResource\n" );
		return S_OK;		
	}      
   
    virtual HRESULT __stdcall OnDataAvailable( DWORD grfBSCF, DWORD dwSize,
        FORMATETC *pformatetc, STGMEDIUM *pstgmed )
	{
		TRACEME( "UpdateCheckCallback::OnDataAvailable\n" );
		return S_OK;				
	}
    
    virtual HRESULT __stdcall OnObjectAvailable( REFIID riid, IUnknown *punk )
	{
		TRACEME( "UpdateCheckCallback::OnDataAvailable\n" );
		return S_OK;				
	}

private:
	CDownloadThread* m_pThread;
	volatile LONG m_refCount;
};


//-----------------------------------------------------------------------------------------------

bool CDownloadThread::Start( const Params& params )
{
	if( IsRunning() )
		return false;

	m_params = params;

	return CreateThread();
}

//-----------------------------------------------------------------------------------------------

UINT CDownloadThread::ThreadFunc()
{
	// Make sure that end-message is always send, even if exception occurs.
	struct Cleanup
	{
		Cleanup( CDownloadThread::Params* pParams_ ) : pParams( pParams_ ), err( S_OK ) {}
		~Cleanup() { ::PostMessage( pParams->notifyHwnd, pParams->notifyMsg, ON_ENDED, err ); }
		CDownloadThread::Params* pParams;
		int err;
	}
	cleanup( &m_params );

	::SendMessage( m_params.notifyHwnd, m_params.notifyMsg, ON_STARTED, 0 );

	DownloadCallback callback( this );
	cleanup.err = ::URLDownloadToFile( NULL, m_params.url, m_params.destFilePath, 0, &callback );

	return 0;
}
