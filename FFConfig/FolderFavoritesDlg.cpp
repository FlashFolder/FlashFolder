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
#include "FolderFavoritesDlg.h"
#include ".\folderfavoritesdlg.h"

//-----------------------------------------------------------------------------------------------

enum FavCols
{
	COL_TITLE,
	COL_PATH
};

struct ItemData
{
	CString targetPath;
};

//-----------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CFolderFavoritesDlg, CDialog)
CFolderFavoritesDlg::CFolderFavoritesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFolderFavoritesDlg::IDD, pParent),
	m_sel( -1 )
{}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LST_FAVS, m_listFavs);
	DDX_Control(pDX, IDC_ED_TITLE, m_edTitle);
	DDX_Control(pDX, IDC_ED_PATH, m_edPath);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFolderFavoritesDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BTN_BROWSE, OnBnClickedBtnBrowse)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LST_FAVS, OnLstFavs_ItemChanged)
	ON_BN_CLICKED(IDC_RD_OWN_FAVORITES, OnBnClickedRdOwnFavorites)
	ON_BN_CLICKED(IDC_RD_TC_FAVORITES, OnBnClickedRdTcFavorites)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LST_FAVS, OnLstFavs_DeleteItem)
	ON_EN_CHANGE(IDC_ED_TITLE, OnEnChangeEdTitle)
	ON_EN_CHANGE(IDC_ED_PATH, OnEnChangeEdPath)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_REMOVE, OnBnClickedBtnRemove)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LST_FAVS, OnLstFavs_KeyDown)
	ON_BN_CLICKED(IDC_BTN_REVERT, OnBnClickedBtnRevert)
	ON_NOTIFY(NM_DBLCLK, IDC_LST_FAVS, OnListFavs_DblClick)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CFolderFavoritesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	tstring tcIniPath;
	bool isTcInstalled = GetTotalCmdLocation( NULL, &tcIniPath );
	bool useTcFavs = false;
	if( isTcInstalled )
	{
		useTcFavs = g_profile.GetInt( _T("main"), _T("UseTcFavorites") ) != 0;
	}
	else
	{
		GetDlgItem( IDC_RD_TC_FAVORITES )->EnableWindow( FALSE ); 	
		GetDlgItem( IDC_RD_OWN_FAVORITES )->EnableWindow( FALSE ); 	
	}
	CheckDlgButton( useTcFavs ? IDC_RD_TC_FAVORITES : IDC_RD_OWN_FAVORITES, 1 );

	m_listFavs.SetExtendedStyle( LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT );
	CRect rcCol;
	rcCol = CRect( 0, 0, 130, 1 ); MapDialogRect( rcCol );
	m_listFavs.InsertColumn( 0, _T("Title"), LVCFMT_LEFT, rcCol.Width() );
	rcCol = CRect( 0, 0, 200, 1 ); MapDialogRect( rcCol );
	m_listFavs.InsertColumn( 1, _T("Path"), LVCFMT_LEFT, rcCol.Width() );

	UpdateFavList();

	m_sizeGrip.Create( this );
	m_anchor.Init( this );
	m_anchor.Add( IDC_LST_FAVS, CDlgAnchor::ANCHOR_ALL );
	m_anchor.Add( IDC_ST_TITLE, CDlgAnchor::ANCHOR_BOTTOMLEFT );
	m_anchor.Add( IDC_ST_PATH, CDlgAnchor::ANCHOR_BOTTOMLEFT );
	m_anchor.Add( IDC_ED_TITLE, CDlgAnchor::ANCHOR_BOTTOMLEFT | CDlgAnchor::ANCHOR_RIGHT );
	m_anchor.Add( IDC_ED_PATH, CDlgAnchor::ANCHOR_BOTTOMLEFT | CDlgAnchor::ANCHOR_RIGHT );
	m_anchor.Add( IDC_BTN_BROWSE, CDlgAnchor::ANCHOR_BOTTOMRIGHT );
	m_anchor.Add( IDC_RD_OWN_FAVORITES, CDlgAnchor::ANCHOR_BOTTOMLEFT );
	m_anchor.Add( IDC_RD_TC_FAVORITES, CDlgAnchor::ANCHOR_BOTTOMLEFT );
	m_anchor.Add( IDOK, CDlgAnchor::ANCHOR_BOTTOM );
	m_anchor.Add( IDCANCEL, CDlgAnchor::ANCHOR_BOTTOM );
	m_anchor.Add( IDC_BTN_REVERT, CDlgAnchor::ANCHOR_BOTTOM );
	m_anchor.Add( m_sizeGrip, CDlgAnchor::ANCHOR_BOTTOMRIGHT );

	// get dialog size from registry
	int width = g_profile.GetInt( _T("main"), _T("FavoritesDlgWidth") );
	int height = g_profile.GetInt( _T("main"), _T("FavoritesDlgHeight") );
	SetWindowPos( NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	m_anchor.OnSize();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnGetMinMaxInfo( MINMAXINFO* pm )
{
	CDialog::OnGetMinMaxInfo( pm );
	CRect rc( 0, 0, 250, 165 );	 MapDialogRect( rc );
	pm->ptMinTrackSize.x = rc.Width();
	pm->ptMinTrackSize.y = rc.Height();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::UpdateFavList( DirFavoritesSrc source )
{
	m_listFavs.SetRedraw( FALSE );
	m_listFavs.DeleteAllItems();

	FavoritesList favs;
	GetDirFavorites( &favs, source );
	for( int i = 0; i != favs.size(); ++i )
	{
		FavoritesList::value_type& fav = favs[ i ];
		int nItem = m_listFavs.InsertItem( m_listFavs.GetItemCount(), _T("") );
		CString title = fav.title.c_str();
		if( title.IsEmpty() ) 
			title = _T("-untitled-");
		m_listFavs.SetItemText( nItem, COL_TITLE, title );
		m_listFavs.SetItemText( nItem, COL_PATH, fav.path.c_str() );
		
		ItemData* pData = new ItemData;
		pData->targetPath = fav.targetpath.c_str();
		m_listFavs.SetItemData( nItem, reinterpret_cast<DWORD_PTR>( pData ) );
	}
	m_listFavs.SetRedraw( TRUE );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnLstFavs_DeleteItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	LPNMLISTVIEW pnm = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	ItemData* pData = reinterpret_cast<ItemData*>( m_listFavs.GetItemData( pnm->iItem ) );
	delete pData;
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnOK()
{
	g_profile.SetInt( _T("main"), _T("UseTcFavorites"), IsDlgButtonChecked( IDC_RD_TC_FAVORITES ) );

	FavoritesList favList;
	for( int i = 0; i != m_listFavs.GetItemCount(); ++i )
	{
		ItemData* pData = reinterpret_cast<ItemData*>( m_listFavs.GetItemData( i ) );
		
		FavoritesList::value_type fav;
		fav.path = m_listFavs.GetItemText( i, COL_PATH ).GetString();
		fav.title = m_listFavs.GetItemText( i, COL_TITLE ).GetString();
		if( fav.title == _T("-untitled-") || fav.title.empty() )
			fav.title = fav.path;
		fav.targetpath = pData->targetPath;		
		if( ! ( fav.title.empty() && fav.path.empty() ) )
			favList.push_back( fav );
	}
	SetDirFavorites( favList );

	SaveDialogSize();
	
	CDialog::OnOK();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnCancel() 
{ 
	SaveDialogSize(); 
	CDialog::OnCancel(); 
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::SaveDialogSize()
{
	CRect rc; GetWindowRect( rc );
	g_profile.SetInt( _T("main"), _T("FavoritesDlgWidth"), rc.Width() );
	g_profile.SetInt( _T("main"), _T("FavoritesDlgHeight"), rc.Height() );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnBrowse()
{
	CString path; GetDlgItemText( IDC_ED_PATH, path );
	CFolderDlg dlg( _T("Select a directory:"), path, this );
	if( dlg.DoModal() == IDOK )
	{
		SetDlgItemText( IDC_ED_PATH, dlg.GetFolderPath() );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::SelectItem( int sel )
{
	m_listFavs.SetItemState( m_sel, 0, LVIS_SELECTED );
	m_listFavs.SetItemState( sel, LVIS_SELECTED, LVIS_SELECTED );
	m_listFavs.SetSelectionMark( sel );
	m_sel = sel;
	UpdateSelItemEditControls();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::UpdateSelItemEditControls()
{
	if( m_sel == -1 )
		return;
	CString title = m_listFavs.GetItemText( m_sel, COL_TITLE );
	CString path = m_listFavs.GetItemText( m_sel, COL_PATH );

	// set edit controls, but disable "reverse update" of list control
	int curSel = m_sel;
	m_sel = -1;
	SetDlgItemText( IDC_ED_TITLE, title );
	SetDlgItemText( IDC_ED_PATH, path );
	m_sel = curSel;
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnLstFavs_ItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	LPNMLISTVIEW pnm = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if( pnm->uNewState & LVIS_SELECTED )
	{
		GetDlgItem( IDC_ED_PATH )->EnableWindow( TRUE );
		GetDlgItem( IDC_ED_TITLE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BTN_BROWSE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BTN_REMOVE )->EnableWindow( TRUE );
		m_sel = pnm->iItem;
		UpdateSelItemEditControls();
	}
	else if( pnm->uOldState & LVIS_SELECTED )
	{
		int oldSel = m_sel; m_sel = -1;
		SetDlgItemText( IDC_ED_PATH, _T("") );
		SetDlgItemText( IDC_ED_TITLE, _T("") );
		m_sel = oldSel;
		GetDlgItem( IDC_ED_PATH )->EnableWindow( FALSE );
		GetDlgItem( IDC_ED_TITLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BTN_BROWSE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BTN_REMOVE )->EnableWindow( FALSE );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnEnChangeEdTitle()
{
	// update current list item
	if( m_sel != -1 )
	{
		CString title;
		GetDlgItemText( IDC_ED_TITLE, title );
		m_listFavs.SetItemText( m_sel, COL_TITLE, title );
	}
}

void CFolderFavoritesDlg::OnEnChangeEdPath()
{
	// update current list item
	if( m_sel != -1 )
	{
		CString path;
		GetDlgItemText( IDC_ED_PATH, path );
		m_listFavs.SetItemText( m_sel, COL_PATH, path );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedRdOwnFavorites()
{
	UpdateFavList( DFS_FLASHFOLDER );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedRdTcFavorites()
{
	UpdateFavList( DFS_TOTALCMD );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnAdd()
{
	if( m_sel == -1 )
		m_sel = m_listFavs.GetItemCount();
	int nItem = m_listFavs.InsertItem( m_sel, _T("-untitled-") );
	ItemData* pData = new ItemData;
	m_listFavs.SetItemData( nItem, reinterpret_cast<DWORD_PTR>( pData ) );
	SelectItem( nItem );
	GetDlgItem( IDC_ED_PATH )->SetFocus();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnRemove()
{
	if( m_sel == -1 )
		return;
	m_listFavs.DeleteItem( m_sel );
	if( m_sel == m_listFavs.GetItemCount() )
		m_sel--;
	SelectItem( m_sel );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnLstFavs_KeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	LPNMLVKEYDOWN pnm = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	if( pnm->wVKey == VK_INSERT )
		OnBnClickedBtnAdd();
	else if( pnm->wVKey == VK_DELETE )
		OnBnClickedBtnRemove();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnRevert()
{
	UpdateFavList( IsDlgButtonChecked( IDC_RD_TC_FAVORITES ) ? DFS_TOTALCMD : DFS_FLASHFOLDER );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnListFavs_DblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	if( m_listFavs.GetSelectionMark() == -1 )
		return;

	OnBnClickedBtnBrowse();
}
