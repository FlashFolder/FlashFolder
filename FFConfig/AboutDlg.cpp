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
 *
 */
#include "stdafx.h"
#include "FFConfig.h"
#include "AboutDlg.h"
#include ".\aboutdlg.h"

//-----------------------------------------------------------------------------------------

bool GetFileVersion( VS_FIXEDFILEINFO* pVer, LPCTSTR pFilePath )
{
	memset( pVer, 0, sizeof(VS_FIXEDFILEINFO) );
	DWORD dummy;
	DWORD size = ::GetFileVersionInfoSize( pFilePath, &dummy );
	if( size == 0 )
		return false;

	std::vector<char> verInfo( size );
	if( ! ::GetFileVersionInfo( pFilePath, 0, size, &verInfo[0] ) )
		return false;
    
	TCHAR name[] = _T("\\");
	VS_FIXEDFILEINFO* pVerData = NULL;
	UINT len = 0;
	if( ! ::VerQueryValue( &verInfo[0], name, (void**) &pVerData, &len ) )
		return false;
	if( len < sizeof(VS_FIXEDFILEINFO) )
		return false;
	memcpy( pVer, pVerData, sizeof(VS_FIXEDFILEINFO) );

	return true;
}

//-----------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

CAboutDlg::CAboutDlg( CWnd* pParent ) :
	CDialog( IDD_ABOUT, pParent )
{}

//-----------------------------------------------------------------------------------------------

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_HOMEPAGE, OnBnClickedBtnHomepage)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	VS_FIXEDFILEINFO ver;

	TCHAR appPath[ MAX_PATH + 1 ] = _T("");
	::GetModuleFileName( NULL, appPath, MAX_PATH );
	CString appDir;
	if( TCHAR* p = _tcsrchr( appPath, '\\' ) )
		appDir = CString( appPath, p - appPath + 1 );

	if( GetFileVersion( &ver, appPath ) )
	{
		CString s; s.Format( _T(" %d.%d.%d.%d"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
		CString s1; GetDlgItemText( IDC_ST_FFCONFIG, s1 );
		SetDlgItemText( IDC_ST_FFCONFIG, s1 + s );
	}
	if( GetFileVersion( &ver, appDir + _T("FlashFolder.exe") ) )
	{
		CString s; s.Format( _T(" %d.%d.%d.%d"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
		CString s1; GetDlgItemText( IDC_ST_FFSERVICE, s1 );
		SetDlgItemText( IDC_ST_FFSERVICE, s1 + s );
	}	
	if( GetFileVersion( &ver, appDir + _T("fflib6439.dll") ) )
	{
		CString s; s.Format( _T(" %d.%d.%d.%d"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
		CString s1; GetDlgItemText( IDC_ST_FFLIB, s1 );
		SetDlgItemText( IDC_ST_FFLIB, s1 + s );
	}

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CAboutDlg::OnBnClickedBtnHomepage()
{
	::ShellExecute( GetSafeHwnd(), _T("open"), _T("http://sourceforge.net/projects/flashfolder/"),
		NULL, NULL, SW_SHOW );
}
