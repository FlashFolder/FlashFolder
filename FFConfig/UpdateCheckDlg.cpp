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
#include "stdafx.h"
#include "FFConfig.h"
#include "UpdateCheckDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4244) // numeric conversion

using namespace std;

//-----------------------------------------------------------------------------------------------

// URL of RSS-feed to check 
const TCHAR UPDATECHECK_URL[] = _T("http://sourceforge.net/export/rss2_projnews.php?group_id=195039");
//const TCHAR UPDATECHECK_URL[] = _T("http://sourceforge.net/export/rss2_projnews.php?group_id=195039&rss_fulltext=1");

// Regular expression for parsing version number from RSS item title. Must be lower-case.
const TCHAR RSS_ITEMTITLE_VERSION_EXPR[] = _T("flashfolder\\b+{\\d+(\\.\\d+)+}.*\\b+released");

// URL for downloading manually
const TCHAR DOWNLOAD_URL[] = _T("http://sourceforge.net/project/platformdownload.php?group_id=195039");

//-----------------------------------------------------------------------------------------------

const UINT WM_APP_THREAD_NOTIFY = WM_APP + 1;

//-----------------------------------------------------------------------------------------------

CUpdateCheckDlg::CUpdateCheckDlg(CWnd* pParent /*=NULL*/) :
	CResizableDlg(CUpdateCheckDlg::IDD, pParent),
	m_expander( this ),
	m_isCanceled( false )
{}

//-----------------------------------------------------------------------------------------------

void CUpdateCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDlg::DoDataExchange( pDX );
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CUpdateCheckDlg, CResizableDlg)
	ON_MESSAGE( WM_APP_THREAD_NOTIFY, OnThreadNotify )
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD, OnBnClickedBtnDownload)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CUpdateCheckDlg::OnInitDialog()
{
	CResizableDlg::OnInitDialog();

	m_tooltip.Create( this );
	m_tooltip.AddTool( GetDlgItem( IDC_BTN_DOWNLOAD ), DOWNLOAD_URL );

	Anchor( IDCANCEL, ANCHOR_BOTTOM );

	m_expander.SetExpandRect( IDC_ST_EXPAND );
	m_expander.SetExpanded( false );

	GetTempFilePath( m_tempPath.GetBuffer( MAX_PATH ), _T("_FF") );
	m_tempPath.ReleaseBuffer();

	// Start the download thread
	CDownloadThread::Params params;
		params.url = UPDATECHECK_URL;
		params.destFilePath = m_tempPath;
		params.notifyHwnd = GetSafeHwnd();
		params.notifyMsg = WM_APP_THREAD_NOTIFY;
	m_thread.Start( params );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

LRESULT CUpdateCheckDlg::OnThreadNotify( WPARAM wp, LPARAM lp )
{
	switch( wp )
	{
		case CDownloadThread::ON_STARTED :
		{
			SetDlgItemText( IDC_ST_STATUS, _T("Preparing connection...") );	
		}
		break;

		case CDownloadThread::ON_ENDED :
		{
			m_thread.WaitFor();

			if( m_isCanceled )
			{
				EndDialog( IDCANCEL );
				return 0;
			}
            GetDlgItem( IDCANCEL )->EnableWindow();
			SetDlgItemText( IDCANCEL, _T("Close") );

			if( lp == S_OK )
			{
				if( ! ProcessDownloadedFile() )
					SetDlgItemText( IDC_ST_STATUS, _T("Could not process RSS feed.") );
			}
			else
			{
				SetDlgItemText( IDC_ST_STATUS, _T("Could not connect to server.") );
			}
		}
		break;

		case CDownloadThread::ON_STATUS :
		{
			CDownloadThread::StatusMsg status = *reinterpret_cast<CDownloadThread::StatusMsg*>( lp );
			// Let the thread continue since we copied the message data.
			ReplyMessage( 0 );

			if( ! status.friendlyText.IsEmpty() )
			{
				SetDlgItemText( IDC_ST_STATUS, status.friendlyText );
				::OutputDebugString( _T("[ffconfig] ") + status.friendlyText + _T("\n") );
			}
		}
		break;
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------

void CUpdateCheckDlg::OnCancel()
{
	if( m_thread.IsRunning() )
	{
		GetDlgItem( IDCANCEL )->EnableWindow( FALSE );
		m_thread.SetCanceled();
		m_isCanceled = true;
		return;
	}
	EndDialog( IDCANCEL );
}

//-----------------------------------------------------------------------------------------------
/// Extract version number from title string.

bool GetVersionFromTitle( int* pVersion, LPCTSTR pTitle, LPCTSTR pFilterExpr )
{
	// Build regular expression for parsing version number from RSS item title
	CAtlRegExp<> reVer;
	if( reVer.Parse( pFilterExpr ) != REPARSE_ERROR_OK )
	{
		ASSERT( 0 );
		return false;
	}

	CString title = pTitle;
	title.MakeLower();
	CAtlREMatchContext<> mcVer;
	if( reVer.Match( title, &mcVer ) )
	{
		if( mcVer.m_uNumGroups != 1 )
		{
			ASSERT( 0 );  // unexpected regular expression
			return false;
		}

		LPCTSTR pStart = NULL, pEnd = NULL;
		mcVer.GetMatch( 0, &pStart, &pEnd );
		CString sVerNum( pStart, pEnd - pStart );

		vector<CString> sVerNumSplit;
		SplitString( back_inserter( sVerNumSplit ), sVerNum.GetString(), _T('.') );
		sVerNumSplit.resize( 4 );
		for( int i = 0; i < 4; ++i )
			pVersion[ i ] = _ttoi( sVerNumSplit[ i ] );

		return true;
	}
	return false;	
}

//-----------------------------------------------------------------------------------------------
/// Extract version number from title of first matching item of an RSS feed.

bool GetVersionFromRSS( int* pVersion, TiXmlNode* pDocRoot, LPCTSTR pFilterExpr )
{
	TiXmlHandle doc( pDocRoot );
	if( TiXmlElement* pChannel = doc.FirstChild( "rss" ).FirstChild( "channel" ).Element() )
	{
		TiXmlNode* pItem = NULL;
		while( pItem = pChannel->IterateChildren( "item", pItem ) )
		{
			CString title;
			if( TiXmlElement* pElem = pItem->FirstChildElement( "title" ) )
				title = Utf8ToStr( pElem->GetText() );

			// Assume that the first matching item is the newest item.
            if( GetVersionFromTitle( pVersion, title, pFilterExpr ) )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------------------------

bool CUpdateCheckDlg::ProcessDownloadedFile()
{
	bool parseRes = m_rssDoc.LoadFile( CStringA( m_tempPath ) );
	::DeleteFile( m_tempPath );
	if( ! parseRes )
		return false;

	//--- get highest version number from downloaded RSS feed

	int onlineVer[ 4 ] = { 0 };
	GetVersionFromRSS( onlineVer, &m_rssDoc, RSS_ITEMTITLE_VERSION_EXPR ); 

	//--- get installed version number

	CString appPath;
	GetAppDir( NULL, appPath.GetBuffer( MAX_PATH ) );
	appPath.ReleaseBuffer();
	appPath += _T("FlashFolder.exe");
	VS_FIXEDFILEINFO ver = { 0 };
	GetFileVersion( &ver, appPath );
	m_instVer[ 0 ] = ver.dwFileVersionMS >> 16;
	m_instVer[ 1 ] = ver.dwFileVersionMS & 0xFFFF;
	m_instVer[ 2 ] = ver.dwFileVersionLS >> 16;
	m_instVer[ 3 ] = ver.dwFileVersionLS & 0xFFFF;

	//--- compare

	if( CompareVersions( onlineVer, m_instVer ) > 0 )
	{
		SetDlgItemText( IDC_ST_STATUS, _T("There is a newer version available.") );
		
		CString sInstVer; 
		sInstVer.Format( _T("%d.%d.%d.%d"), m_instVer[ 0 ], m_instVer[ 1 ], m_instVer[ 2 ], m_instVer[ 3 ] );
		SetDlgItemText( IDC_ST_INSTVER, sInstVer );
		
		CString sOnlineVer; 
		sOnlineVer.Format( _T("%d.%d.%d.%d"), onlineVer[ 0 ], onlineVer[ 1 ], onlineVer[ 2 ], onlineVer[ 3 ] );
		SetDlgItemText( IDC_ST_ONLINEVER, sOnlineVer );

		m_expander.SetExpanded();	
	}
	else
	{
		SetDlgItemText( IDC_ST_STATUS, _T("You already have the latest version.") );
	}

	return true;
}

//-----------------------------------------------------------------------------------------------

void CUpdateCheckDlg::OnBnClickedBtnDownload()
{
	int res = (int) ::ShellExecute( GetSafeHwnd(), _T("open"), DOWNLOAD_URL, NULL, NULL, SW_SHOW );
	if( res > 32 )
	{
		EndDialog( IDCANCEL );
		return;
	}
	LPCTSTR pBuf = NULL;
	if( ::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
		::GetModuleHandle( _T("kernel32.dll") ), res, 0, (LPTSTR) &pBuf, 256, NULL ) )
	{
		AfxMessageBox( pBuf, MB_ICONERROR );
		::LocalFree( (HLOCAL) pBuf );
	}
}

//-----------------------------------------------------------------------------------------------

BOOL CUpdateCheckDlg::PreTranslateMessage( MSG* pMsg )
{
	m_tooltip.RelayEvent( pMsg );
	return CResizableDlg::PreTranslateMessage( pMsg );
}
