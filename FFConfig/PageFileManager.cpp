/* This file is part of FlashFolder.
 * Copyright (C) 2007-2012 zett42.de ( zett42 at users.sourceforge.net )
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
#include "PageFileManager.h"
#include <common\PluginManager.h>

using namespace std;

//-----------------------------------------------------------------------------------------------

const CString PROFILE_GROUP = L"FileManager";

//-----------------------------------------------------------------------------------------------

CPageFileManager::CPageFileManager()
	: base(CPageFileManager::IDD)
{}

//-----------------------------------------------------------------------------------------------

void CPageFileManager::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SOURCE, m_lstSource);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageFileManager, CPageFileManager::base)
	ON_BN_CLICKED(IDC_CHK_SHOW_ALL, &CPageFileManager::OnBnClickedChkShowAll)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST_SOURCE, OnLvnItemchangingListSource)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPageFileManager::OnInitDialog()
{
	base::OnInitDialog();

	//--- init controls

	UINT listStyleEx = LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP;

	// On Vista and above...
	if( ( ::GetVersion() & 0xFF ) >= 6 )
	{
		// Use explorer theme if possible
		::SetWindowTheme( m_lstSource, L"explorer", NULL );
		// Improve selection visibility when not focused.
		listStyleEx |= LVS_EX_CHECKBOXES | LVS_EX_AUTOCHECKSELECT;
	}

	m_lstSource.SetExtendedStyle( listStyleEx );

	m_lstSource.InsertColumn( 0, L"Homepage", 0, MapDialogX( *this, 120 ) );
	m_lstSource.InsertColumn( 0, L"Program", 0, MapDialogX( *this, 80 ) );

	LVGROUP grp = { sizeof( grp ), LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN | LVGF_STATE };
	grp.uAlign = LVGA_HEADER_CENTER;

	grp.iGroupId = 0;
	grp.pszHeader = L"Not installed";
	grp.state = grp.stateMask = LVGS_HIDDEN;
	m_lstSource.InsertGroup( 0, &grp );

	grp.iGroupId = 1;
	grp.pszHeader = L"Installed";
	grp.state = grp.stateMask = 0;
	m_lstSource.InsertGroup( 0, &grp );

	CWaitCursor wc;

	m_pluginMgr.reset( new PluginManager );
	m_fileMgrs = m_pluginMgr->GetSupportedFileManagers();

	bool allProgramsInstalled = true;
	foreach( const PluginManager::FileMgr& fm, m_fileMgrs )
		if( ! fm.program.isInstalled )
		{
			allProgramsInstalled = false;
			break;
		}

	if( allProgramsInstalled )
		GetDlgItem( IDC_CHK_SHOW_ALL )->EnableWindow( FALSE );

	UpdateFavoritesSources();

	//--- get profile data

	ReadProfile();

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CPageFileManager::UpdateFavoritesSources()
{
	bool showNotInstalled = !! IsDlgButtonChecked( IDC_CHK_SHOW_ALL );

	m_lstSource.DeleteAllItems();

	m_lstSource.EnableGroupView( showNotInstalled );

	bool hasNotInstalledPrograms = false;

	foreach( const PluginManager::FileMgr& fm, m_fileMgrs )
	{
		// Skip unsupported programs.
		if( ( fm.program.capabilities & ffplug::FMC_EnumFavorites ) == 0 )
			continue;
		if( ! fm.program.isInstalled && ! showNotInstalled )
			continue;

		int nItem = m_lstSource.InsertItem( m_lstSource.GetItemCount(), fm.program.displayName );
		m_lstSource.SetItemText( nItem, 1, fm.program.websiteUrl );

		if( showNotInstalled )
		{
			LVITEM item = { LVIF_GROUPID };
			item.iItem = nItem;
			item.iGroupId = !! fm.program.isInstalled;
			m_lstSource.SetItem( &item );
		}
	}
}

//-----------------------------------------------------------------------------------------------

void CPageFileManager::OnBnClickedChkShowAll()
{
	UpdateFavoritesSources();
}

//-----------------------------------------------------------------------------------------------

void CPageFileManager::OnLvnItemchangingListSource(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW nm = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	// prevent selection of not installed programs
	if( ( nm->uNewState & LVIS_SELECTED ) && ! m_fileMgrs[ nm->iItem ].program.isInstalled )
	{
		*pResult = TRUE;  
		return;
	}

	SetModified();
}

//-----------------------------------------------------------------------------------------------

void CPageFileManager::ReadProfile()
{
	const Profile& profile = CApp::GetReadProfile();

	tstring pluginName = profile.GetString( PROFILE_GROUP, L"FavoritesSrc.PluginName" );
	tstring programId = profile.GetString( PROFILE_GROUP, L"FavoritesSrc.ProgramId" );

	int i = 0;
	foreach( const PluginManager::FileMgr& fm, m_fileMgrs )
	{
		const PluginManager::Plugin& plugin = m_pluginMgr->GetPlugin( fm.nPlugin );		

		if( _wcsicmp( plugin.name.c_str(), pluginName.c_str() ) == 0 &&
			wcscmp( fm.program.id, programId.c_str() ) == 0 )
		{
			m_lstSource.SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
			break;
		}
		++i;
	}
}

//-----------------------------------------------------------------------------------------------

BOOL CPageFileManager::OnApply()
{
	Profile& profile = CApp::GetWriteProfile();

	if( m_lstSource.GetItemCount() == 0 )
		return base::OnApply();

	int sel = 0;
	POSITION pos = m_lstSource.GetFirstSelectedItemPosition();
	if( pos )
		sel = m_lstSource.GetNextSelectedItem( pos );
	else
		m_lstSource.SetItemState( 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	const PluginManager::FileMgr& fm = m_fileMgrs[ sel ];
	const PluginManager::Plugin& plugin = m_pluginMgr->GetPlugin( fm.nPlugin );		

	profile.SetString( PROFILE_GROUP, L"FavoritesSrc.PluginName", plugin.name.c_str() );
	profile.SetString( PROFILE_GROUP, L"FavoritesSrc.ProgramId", fm.program.id );

	return base::OnApply();
}
