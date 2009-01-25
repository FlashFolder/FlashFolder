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

#pragma warning(disable:4244)

//-----------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

CAboutDlg::CAboutDlg( CWnd* pParent ) :
	CDialog( IDD_ABOUT, pParent )
{}

//-----------------------------------------------------------------------------------------------

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_ST_HOMEPAGE, m_linkHomepage );
	DDX_Control( pDX, IDC_ST_PROJECTPAGE, m_linkProjectPage );
	DDX_Control( pDX, IDC_ST_BUGREPORT, m_linkBugReport );
	DDX_Control( pDX, IDC_ST_FEATUREREQ, m_linkFeatureReq );
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	VS_FIXEDFILEINFO ver;

	CString appDir;
	GetAppDir( NULL, appDir.GetBuffer( 4096 ), 4096 );
	appDir.ReleaseBuffer();
	
	CString s, s0, s1; 

	s0 = L"";
	if( GetFileVersion( &ver, appDir + L"FFConfig.exe" ) )
		s0.Format( _T(" %d.%d.%d.%d (32)"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
	if( GetFileVersion( &ver, appDir + L"FFConfig64.exe" ) )
		s1.Format( _T(" %d.%d.%d.%d (64)"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
	GetDlgItemText( IDC_ST_FFCONFIG, s );
	SetDlgItemText( IDC_ST_FFCONFIG, s + s0 + s1 );

	if( GetFileVersion( &ver, appDir + _T("FlashFolder.exe") ) )
		s0.Format( _T(" %d.%d.%d.%d (32)"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
	if( GetFileVersion( &ver, appDir + _T("FlashFolder64.exe") ) )
		s1.Format( _T(" %d.%d.%d.%d (64)"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
	GetDlgItemText( IDC_ST_FFCONFIG, s );
	SetDlgItemText( IDC_ST_FFCONFIG, s + s0 + s1 );

	if( GetFileVersion( &ver, appDir + _T("fflib6439.dll") ) )
		s0.Format( _T(" %d.%d.%d.%d (32)"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
	if( GetFileVersion( &ver, appDir + _T("fflib6439_64.dll") ) )
		s1.Format( _T(" %d.%d.%d.%d (64)"),
			ver.dwFileVersionMS >> 16, ver.dwFileVersionMS & 0xFFFF,
			ver.dwFileVersionLS >> 16, ver.dwFileVersionLS & 0xFFFF );
	GetDlgItemText( IDC_ST_FFCONFIG, s );
	SetDlgItemText( IDC_ST_FFCONFIG, s + s0 + s1 );

	m_linkHomepage.SetURL( _T("http://www.zett42.de/flashfolder/") );
	m_linkProjectPage.SetURL( _T("http://sourceforge.net/projects/flashfolder/") );
	m_linkBugReport.SetURL( _T("http://sourceforge.net/tracker/?group_id=195039&atid=951838") );
	m_linkFeatureReq.SetURL( _T("http://sourceforge.net/tracker/?group_id=195039&atid=951841") );

	return TRUE;
}
