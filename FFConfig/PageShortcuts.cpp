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
#include "PageShortcuts.h"

using namespace std;

//-----------------------------------------------------------------------------------------------

CString GetHotkeyName( DWORD hotkey );

//-----------------------------------------------------------------------------------------------

enum ShortcutColumns
{
	COL_TITLE,
	COL_SHORTCUTKEY
};

//-----------------------------------------------------------------------------------------------

const CString PROFILE_GROUP = _T("Hotkeys");

//-----------------------------------------------------------------------------------------------

CPageShortcuts::CPageShortcuts()
	: base(CPageShortcuts::IDD),
	m_bReadDefaults( false )
{}

//-----------------------------------------------------------------------------------------------

void CPageShortcuts::DoDataExchange(CDataExchange* pDX)
{
	base::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LST_SHORTCUTS, m_lstShortcuts );
	DDX_Control( pDX, IDC_HOTKEY, m_hotkeyCtrl );
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageShortcuts, CPageShortcuts::base)
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_LST_SHORTCUTS, OnLvnItemchangedLstShortcuts )
	ON_EN_CHANGE( IDC_HOTKEY, OnShortcutChange )
	ON_BN_CLICKED(IDC_BTN_CLEAR, OnBnClickedBtnClear)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CPageShortcuts::OnInitDialog()
{
	base::OnInitDialog();

	//--- init controls

	m_lstShortcuts.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );
	m_lstShortcuts.InsertColumn( 0, _T("Function"), LVCFMT_LEFT, MapDialogX( *this, 105 ) );
	m_lstShortcuts.InsertColumn( 1, _T("Shortcut Key"), LVCFMT_LEFT, MapDialogX( *this, 105 ) );

	//--- get profile data

	if( m_bReadDefaults )
		ReadProfile( g_profileDefaults );
	else
		ReadProfile( g_profile );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

void CPageShortcuts::ReadProfile( const Profile& profile )
{
	if( ! GetSafeHwnd() )
	{
		m_bReadDefaults = true;
		return;
	}

	m_lstShortcuts.DeleteAllItems();

	m_mapTitleToCmd.clear();

	CString title, cmd;

	int nItem = 0;
	title = _T("Go to last used folder"); 
	cmd   = _T("ff_LastFolder");
	m_mapTitleToCmd[ title ] = cmd;
	m_lstShortcuts.InsertItem( nItem, title );
	m_lstShortcuts.SetItemData( nItem, profile.GetInt( PROFILE_GROUP, cmd ) );

	++nItem;
	title = _T("Menu: global folder history");
	cmd   = _T("ff_MenuFolderHistory");
	m_mapTitleToCmd[ title ] = cmd;
	m_lstShortcuts.InsertItem( nItem, title );
	m_lstShortcuts.SetItemData( nItem, profile.GetInt( PROFILE_GROUP, cmd ) );

	++nItem;
	title = _T("Menu: folders of open files");
	cmd   = _T("ff_MenuOpenFolders");
	m_mapTitleToCmd[ title ] = cmd;
	m_lstShortcuts.InsertItem( nItem, title );
	m_lstShortcuts.SetItemData( nItem, profile.GetInt( PROFILE_GROUP, cmd ) );

	++nItem;
	title = _T("Menu: favorite folders");
	cmd   = _T("ff_MenuFavorites");
	m_mapTitleToCmd[ title ] = cmd;
	m_lstShortcuts.InsertItem( nItem, title );
	m_lstShortcuts.SetItemData( nItem, profile.GetInt( PROFILE_GROUP, cmd ) );

	++nItem;
	title = _T("View all files");
	cmd   = _T("ff_ViewAllFiles");
	m_mapTitleToCmd[ title ] = cmd;
	m_lstShortcuts.InsertItem( nItem, title );
	m_lstShortcuts.SetItemData( nItem, profile.GetInt( PROFILE_GROUP, cmd ) );

	++nItem;
	title = _T("Focus path edit field");
	cmd   = _T("ff_FocusPathEdit");
	m_mapTitleToCmd[ title ] = cmd;
	m_lstShortcuts.InsertItem( nItem, title );
	m_lstShortcuts.SetItemData( nItem, profile.GetInt( PROFILE_GROUP, cmd ) );

	for( int i = 0; i <= nItem; ++i )
	{
		DWORD hotkey = m_lstShortcuts.GetItemData( i );
		m_lstShortcuts.SetItemText( i, COL_SHORTCUTKEY, GetHotkeyName( hotkey ) );
	}

	m_selItem = -1;
}

//-----------------------------------------------------------------------------------------------

BOOL CPageShortcuts::OnApply()
{
	int count = m_lstShortcuts.GetItemCount();
	for( int i = 0; i < count; ++i )
	{
		DWORD hotkey = m_lstShortcuts.GetItemData( i );
		if( hotkey != 0 )
		{
			CString title = m_lstShortcuts.GetItemText( i, COL_TITLE );
			std::map<CString,CString>::iterator itCmd = m_mapTitleToCmd.find( title );
			ASSERT( itCmd != m_mapTitleToCmd.end() );
			g_profile.SetInt( PROFILE_GROUP, itCmd->second, hotkey );
		}
	}

	return base::OnApply();
}

//-----------------------------------------------------------------------------------------------

void CPageShortcuts::OnLvnItemchangedLstShortcuts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pnm = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if( ! ( pnm->uChanged & LVIF_STATE ) )
		return;

	if( pnm->uNewState & LVIS_SELECTED )
	{
		//--- Get new hotkey
		m_selItem = pnm->iItem;

		DWORD hotkey = m_lstShortcuts.GetItemData( m_selItem );

		EnableDlgItem( *this, IDC_HOTKEY, TRUE );
		EnableDlgItem( *this, IDC_BTN_CLEAR, hotkey != 0 );

		m_hotkeyCtrl.SetHotKey( hotkey & 0xFF, hotkey >> 8 );
	}
	else
	{
		m_selItem = -1;
		
		EnableDlgItem( *this, IDC_HOTKEY, FALSE );
		EnableDlgItem( *this, IDC_BTN_CLEAR, FALSE );
		
		m_hotkeyCtrl.SetHotKey( 0, 0 );
	}
}

//-----------------------------------------------------------------------------------------------

void CPageShortcuts::OnShortcutChange()
{
	//--- Save current hotkey

	if( m_selItem == -1 )
		return;

	WORD vkey = 0, mod = 0;
    m_hotkeyCtrl.GetHotKey( vkey, mod );

	EnableDlgItem( *this, IDC_BTN_CLEAR, vkey != 0 );

	if( vkey == 0 && mod != 0 )
		return;
	if( vkey == 0 && mod == 0 )
	{
		m_lstShortcuts.SetItemData( m_selItem, 0 );
		m_lstShortcuts.SetItemText( m_selItem, COL_SHORTCUTKEY, _T("") );
		return;
	}

	// filter unmodified keys, except function keys
	if( mod == 0 && 
		! ( vkey >= VK_F1 && vkey <= VK_F24 ||
	        vkey >= VK_BROWSER_BACK && vkey <= VK_LAUNCH_APP2 ) )
	{
		mod = HOTKEYF_CONTROL;
		m_hotkeyCtrl.SetHotKey( vkey, mod );
	}
	DWORD hotkey = vkey | mod << 8;

	// remove duplicate hotkey, if any
	int count = m_lstShortcuts.GetItemCount();
	for( int i = 0; i < count; ++i )
	{
		if( m_lstShortcuts.GetItemData( i ) == hotkey )
		{
			m_lstShortcuts.SetItemData( i, 0 );
			m_lstShortcuts.SetItemText( i, COL_SHORTCUTKEY, _T("") );
		}
	}

	m_lstShortcuts.SetItemData( m_selItem, hotkey );
	m_lstShortcuts.SetItemText( m_selItem, COL_SHORTCUTKEY, GetHotkeyName( hotkey ) );
}

//-----------------------------------------------------------------------------------------------

void CPageShortcuts::OnBnClickedBtnClear()
{
	m_hotkeyCtrl.SetHotKey( 0, 0 );
	if( m_selItem != -1 )
	{
		m_lstShortcuts.SetItemData( m_selItem, 0 );
		m_lstShortcuts.SetItemText( m_selItem, COL_SHORTCUTKEY, _T("") );		
	}
}

//-----------------------------------------------------------------------------------------------

CString GetHotkeyName( DWORD hotkey )
{
	WORD vk  = hotkey & 0xFF;
	WORD mod = hotkey >> 8;
	
	CString s;
	
	if( mod & HOTKEYF_CONTROL )
	{
		s = CHotKeyCtrl::GetKeyName( VK_CONTROL, FALSE );
	}
	if( mod & HOTKEYF_SHIFT )
	{
		if( ! s.IsEmpty() )
			s += _T(" + ");
		s += CHotKeyCtrl::GetKeyName( VK_SHIFT, FALSE );
	}
	if( mod & HOTKEYF_ALT )
	{
		if( ! s.IsEmpty() )
			s += _T(" + ");
		s += CHotKeyCtrl::GetKeyName( VK_MENU, FALSE );
	}
	if( ! s.IsEmpty() )
		s += _T(" + ");
	s += CHotKeyCtrl::GetKeyName( vk, mod & HOTKEYF_EXT );

	return s;
}

