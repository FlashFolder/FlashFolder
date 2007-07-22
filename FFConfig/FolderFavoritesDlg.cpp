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

//-----------------------------------------------------------------------------------------------

enum FavCols
{
	COL_TITLE,
	COL_COMMAND,
	COL_TARGETPATH
};

//-----------------------------------------------------------------------------------------------

struct TempFocus
{
	TempFocus( HWND hWnd )
	{
		hWndFocus = ::SetFocus( hWnd );		
	}
	~TempFocus()
	{
		if( hWndFocus ) 
			::SetFocus( hWndFocus );
	}
	HWND hWndFocus;
};

//-----------------------------------------------------------------------------------------------

CString GetDefaultTitle( const CString& cmd )
{
	CString title;
	tstring token, args;
	SplitTcCommand( cmd, &token, &args );
	if( CString( token.c_str() ).CompareNoCase( _T("cd") ) == 0 )
		title = args.c_str();
	else
		title = cmd;
	if( title.IsEmpty() )
		title = _T("-untitled-");
	return title;
}

//-----------------------------------------------------------------------------------------------

CFolderFavoritesDlg::CFolderFavoritesDlg(CWnd* pParent /*=NULL*/)
	: CResizableDlg(CFolderFavoritesDlg::IDD, pParent),
	m_hSelItem( NULL )
{}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ED_TITLE, m_edTitle);
	DDX_Control(pDX, IDC_ED_COMMAND, m_edPath);
	DDX_Control(pDX, IDC_ED_TARGETPATH, m_edTargetPath);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFolderFavoritesDlg, CResizableDlg)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED( IDC_BTN_BROWSE, OnBnClickedBtnBrowse )
	ON_NOTIFY( TVN_SELCHANGED, IDC_LST_FAVS, OnTree_SelChanged )
	ON_EN_CHANGE( IDC_ED_TITLE, OnEnChangeEdTitle )
	ON_EN_CHANGE( IDC_ED_COMMAND, OnEnChangeEdCommand )
	ON_EN_CHANGE( IDC_ED_TARGETPATH, OnEnChangeEdTargetPath )
	ON_BN_CLICKED( IDC_BTN_ADD, OnBnClickedBtnAdd )
	ON_BN_CLICKED( IDC_BTN_ADD_DIVIDER, OnBnClickedBtnAddDivider )
	ON_BN_CLICKED( IDC_BTN_ADD_SUBMENU, OnBnClickedBtnAddSubmenu )
	ON_BN_CLICKED( IDC_BTN_REMOVE, OnBnClickedBtnRemove )
	ON_BN_CLICKED( IDC_BTN_IMPORT, OnBnClickedBtnImport )
	ON_BN_CLICKED(IDC_BTN_TARGETBROWSE, OnBnClickedBtnTargetbrowse)
	ON_BN_CLICKED(IDC_BTN_REVERT, OnBnClickedBtnRevert)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CFolderFavoritesDlg::OnInitDialog()
{
	CResizableDlg::OnInitDialog();

	//--- Create and populate tree control
	
	CRect rcTree; GetDlgItem( IDC_ST_TREE_PLACEHOLDER )->GetWindowRect( rcTree );
	ScreenToClient( rcTree );
	m_tree.Create( this, rcTree, IDC_LST_FAVS );
	m_tree.SetWindowPos( GetDlgItem( IDC_ST_FAVMENU ), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	m_tree.SetFont( GetFont() );
	m_tree.GetHeaderCtrl().ModifyStyle( HDS_BUTTONS, 0 );

	CRect rcItem( 0, 0, 1, 12 ); MapDialogRect( rcItem );
	m_tree.GetTree().SetItemHeight( rcItem.bottom );

	m_tree.SetDragCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_DRAGCURSOR ) );

	int iconCX = 16, iconCY = 16;
	m_treeIcons.Create( iconCX, iconCY, ILC_COLOR32 | ILC_MASK, 1, 1 );
	HICON hIcon;
	hIcon = (HICON) ::LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_SUBMENU ), 
		IMAGE_ICON, iconCX, iconCY, LR_SHARED );
	m_treeIcons.Add( hIcon );
	hIcon = (HICON) ::LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_MENUITEM ), 
		IMAGE_ICON, iconCX, iconCY, LR_SHARED );
	m_treeIcons.Add( hIcon );
	m_tree.GetTree().SetImageList( &m_treeIcons, TVSIL_NORMAL );

	CRect rcCol;
	rcCol = CRect( 0, 0, 130, 1 ); MapDialogRect( rcCol );
	m_tree.InsertColumn( 0, _T("Title"), LVCFMT_LEFT, rcCol.Width() );
	rcCol = CRect( 0, 0, 150, 1 ); MapDialogRect( rcCol );
	m_tree.InsertColumn( 1, _T("Command"), LVCFMT_LEFT, rcCol.Width() );
	rcCol = CRect( 0, 0, 150, 1 ); MapDialogRect( rcCol );
	m_tree.InsertColumn( 2, _T("Target path"), LVCFMT_LEFT, rcCol.Width() );

	LoadFavorites();

	//--- Dialog resizing stuff	

	Anchor( IDC_LST_FAVS, ANCHOR_ALL );
	Anchor( IDC_ST_TITLE, ANCHOR_BOTTOMLEFT );
	Anchor( IDC_ST_PATH, ANCHOR_BOTTOMLEFT );
	Anchor( IDC_ST_TARGETPATH, ANCHOR_BOTTOMLEFT );
	Anchor( IDC_ED_TITLE, ANCHOR_BOTTOMLEFT | ANCHOR_RIGHT );
	Anchor( IDC_ED_COMMAND, ANCHOR_BOTTOMLEFT | ANCHOR_RIGHT );
	Anchor( IDC_ED_TARGETPATH, ANCHOR_BOTTOMLEFT | ANCHOR_RIGHT );
	Anchor( IDC_BTN_BROWSE, ANCHOR_BOTTOMRIGHT );
	Anchor( IDC_BTN_TARGETBROWSE, ANCHOR_BOTTOMRIGHT );
	Anchor( IDOK, ANCHOR_BOTTOMRIGHT );
	Anchor( IDCANCEL, ANCHOR_BOTTOMRIGHT );
	Anchor( IDC_BTN_REVERT, ANCHOR_BOTTOMRIGHT );

	// Set size from dialog template as minimum size
	SetMinSize();

	// get dialog size from registry
	int width = g_profile.GetInt( _T("main"), _T("FavoritesDlgWidth") );
	int height = g_profile.GetInt( _T("main"), _T("FavoritesDlgHeight") );
	SetWindowPos( NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------------------------
/// Load favorites menu from profile

void CFolderFavoritesDlg::LoadFavorites()
{
	AutoRedraw( m_tree.GetSafeHwnd() );

	m_tree.DeleteAllItems();

	FavoritesList favs;
	GetDirFavorites( &favs );

	size_t iItem = 0;

	LoadFavorites_worker( TVI_ROOT, favs, iItem );

	GetDlgItem( IDC_BTN_REVERT )->EnableWindow( FALSE );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::LoadFavorites_worker( HTREEITEM hParent, const FavoritesList& favs, size_t& iItem )
{
	while( iItem < favs.size() )
	{
		const FavoritesList::value_type& fav = favs[ iItem ];
		
		HTREEITEM hItem = NULL;

		if( fav.title == _T("--") )
		{
			// end of submenu

			++iItem;
			return;
		}
		else if( fav.title.size() > 1 && fav.title.substr( 0, 1 ) == _T("-") )
		{
			// insert submenu recursively

			hItem = m_tree.GetTree().InsertItem( fav.title.substr( 1 ).c_str(), 0, 0, hParent, TVI_LAST );
			++iItem;

			LoadFavorites_worker( hItem, favs, iItem );			
		}
		else if( fav.title == _T("-") )
		{
			// insert divider
			
			hItem = m_tree.GetTree().InsertItem( _T(""), -1, -1, hParent, TVI_LAST );
			m_tree.SetItemDivider( hItem );

			++iItem;
		}
		else
		{
			// insert normal item

			hItem = m_tree.GetTree().InsertItem( fav.title.c_str(), 1, 1, hParent, TVI_LAST );
			m_tree.SetItemText( hItem, COL_COMMAND, fav.command.c_str() );
			m_tree.SetItemText( hItem, COL_TARGETPATH, fav.targetpath.c_str() );

			++iItem;
		}
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnOK()
{
	SaveFavorites();

	SaveDialogSize();

	CResizableDlg::OnOK();
}

//-----------------------------------------------------------------------------------------------
/// Save favorites menu into profile

void CFolderFavoritesDlg::SaveFavorites()
{
	FavoritesList favs;
	SaveFavorites_worker( favs, TVI_ROOT );
	SetDirFavorites( favs );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::SaveFavorites_worker( FavoritesList& favs, HTREEITEM hParent )
{
	for( HTREEITEM hItem = m_tree.GetTree().GetChildItem( hParent ); hItem; 
	     hItem = m_tree.GetTree().GetNextItem( hItem, TVGN_NEXT ) )
	{
		CString title = m_tree.GetItemText( hItem, COL_TITLE ).GetString();
		// remove special tokens
		if( title == _T("-") || title == _T("--") )
			title = _T(" ") + title;

		CString command = m_tree.GetItemText( hItem, COL_COMMAND ).GetString();

		// remove incomplete items
		if( ! m_tree.IsItemDivider( hItem ) )
			if( ( title.IsEmpty() || title == _T("-untitled-") ) && command.IsEmpty() )
				continue;

		FavoritesItem fav;

		if( m_tree.GetTree().ItemHasChildren( hItem ) )
		{
			// add submenu

			fav.title = _T("-") + title;
			favs.push_back( fav );

			if( m_tree.HasRealChildren( hItem ) )
				SaveFavorites_worker( favs, hItem );
		}
		else
		{
			if( m_tree.IsItemDivider( hItem ) )
			{
				// add divider

				fav.title = _T("-");
				favs.push_back( fav );
			}
			else
			{
				// add normal item

				if( title.IsEmpty() )
					title = GetDefaultTitle( command );
				fav.title = title.GetString();
				if( IsFilePath( command ) )
					command = _T("cd ") + command;
				fav.command = command.GetString();
				

				fav.targetpath = m_tree.GetItemText( hItem, COL_TARGETPATH ).GetString();
				favs.push_back( fav );		
			}
		}
	}
	if( hParent != TVI_ROOT )
	{
		// end of submenu
		FavoritesItem fav;
		fav.title = _T("--");
		favs.push_back( fav );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnCancel() 
{ 
	SaveDialogSize(); 
	CResizableDlg::OnCancel(); 
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
	CString path;
	CString cmd; GetDlgItemText( IDC_ED_COMMAND, cmd );
	if( IsFilePath( cmd ) )
	{
		path = cmd;
	}
	else
	{
		tstring token, args;
		SplitTcCommand( cmd, &token, &args );
		if( CString( token.c_str() ).CompareNoCase( _T("cd") ) == 0 )
			path = args.c_str();
	}

	CFolderDlg dlg( _T("Select a directory:"), path, this );
	if( dlg.DoModal() == IDOK )
	{
		SetDlgItemText( IDC_ED_COMMAND,	CString( _T("cd ") ) + dlg.GetFolderPath() );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnTargetbrowse()
{
	CString path; GetDlgItemText( IDC_ED_TARGETPATH, path );
	CFolderDlg dlg( _T("Select a directory:"), path, this );
	if( dlg.DoModal() == IDOK )
	{
		SetDlgItemText( IDC_ED_TARGETPATH, dlg.GetFolderPath() );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::UpdateSelItemEditControls()
{
	if( ! m_hSelItem )
		return;
	CString title, command, targetPath;
	if( ! m_tree.IsItemDummy( m_hSelItem ) )
	{
		title = m_tree.GetItemText( m_hSelItem, COL_TITLE );
		command = m_tree.GetItemText( m_hSelItem, COL_COMMAND );
		targetPath = m_tree.GetItemText( m_hSelItem, COL_TARGETPATH );
	}

	// set edit controls, but disable "reverse update" of list control
	HTREEITEM curSel = m_hSelItem;
	m_hSelItem = NULL;

	CString defTitle = GetDefaultTitle( command );
	if( title == _T("-untitled-") || title == defTitle )
	{
		m_edTitle.SetHintText( title );
		title = _T("");
	}
	SetDlgItemText( IDC_ED_TITLE, title );

	SetDlgItemText( IDC_ED_COMMAND, command );
	SetDlgItemText( IDC_ED_TARGETPATH, targetPath );
	
	m_hSelItem = curSel;
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnTree_SelChanged( NMHDR *pNMHDR, LRESULT *pResult )
{
	*pResult = 0;
	LPNMTREEVIEW pnm = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// clear edit controls without updating tree
	HTREEITEM oldSel = m_hSelItem; m_hSelItem = NULL;
	SetDlgItemText( IDC_ED_COMMAND, _T("") );
	SetDlgItemText( IDC_ED_TARGETPATH, _T("") );
	SetDlgItemText( IDC_ED_TITLE, _T("") );
	m_edTitle.SetHintText( _T("") );
	m_hSelItem = oldSel;

	// reset button states
	GetDlgItem( IDC_BTN_REMOVE )->EnableWindow( FALSE );
	GetDlgItem( IDC_ED_COMMAND )->EnableWindow( FALSE );
	GetDlgItem( IDC_ED_TARGETPATH )->EnableWindow( FALSE );
	GetDlgItem( IDC_ED_TITLE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BTN_BROWSE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BTN_TARGETBROWSE )->EnableWindow( FALSE );

	// set new button states

	if( pnm->itemNew.hItem )
	{
		m_hSelItem = pnm->itemNew.hItem;
		UpdateSelItemEditControls();

		if( ! m_tree.IsItemDummy( pnm->itemNew.hItem ) )
		{
			GetDlgItem( IDC_BTN_REMOVE )->EnableWindow( TRUE );

			if( ! m_tree.IsItemDivider( pnm->itemNew.hItem ) )
			{
				GetDlgItem( IDC_ED_TITLE )->EnableWindow( TRUE );

				if( ! m_tree.GetTree().ItemHasChildren( pnm->itemNew.hItem ) )
				{
					GetDlgItem( IDC_ED_COMMAND )->EnableWindow( TRUE );
					GetDlgItem( IDC_ED_TARGETPATH )->EnableWindow( TRUE );
					GetDlgItem( IDC_BTN_BROWSE )->EnableWindow( TRUE );
					GetDlgItem( IDC_BTN_TARGETBROWSE )->EnableWindow( TRUE );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnEnChangeEdTitle()
{
	CString title;
	GetDlgItemText( IDC_ED_TITLE, title );

	// set default title
	CString cmd;
	GetDlgItemText( IDC_ED_COMMAND, cmd );
	cmd.Trim();
	CString defTitle = GetDefaultTitle( cmd );
	m_edTitle.SetHintText( defTitle );

	// update current list item
	if( m_hSelItem )
	{
		if( title.IsEmpty() )
			title = defTitle;

		m_tree.SetItemText( m_hSelItem, COL_TITLE, title );

		GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
	}
}

void CFolderFavoritesDlg::OnEnChangeEdCommand()
{
	CString cmd;
	GetDlgItemText( IDC_ED_COMMAND, cmd );
	cmd.Trim();

	// set default title
	CString title = GetDefaultTitle( cmd );
	m_edTitle.SetHintText( title );

	// update current list item
	if( m_hSelItem )
	{
		if( GetDlgItem( IDC_ED_TITLE )->GetWindowTextLength() == 0 )
			m_tree.SetItemText( m_hSelItem, COL_TITLE, title );

		m_tree.SetItemText( m_hSelItem, COL_COMMAND, cmd );

		GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
	}
}

void CFolderFavoritesDlg::OnEnChangeEdTargetPath()
{
	// update current list item
	if( m_hSelItem )
	{
		CString path;
		GetDlgItemText( IDC_ED_TARGETPATH, path );
		path.Trim();
		m_tree.SetItemText( m_hSelItem, COL_TARGETPATH, path );

		GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnAdd()
{
	// without setting focus, subsequent inserts will not work 
	TempFocus tempFocus( m_tree );

	HTREEITEM hInsert = m_tree.GetTree().GetFocusedItem();
	HTREEITEM hParent = TVI_ROOT;
	if( hInsert )
		hParent = m_tree.GetTree().GetParentItem( hInsert );
	else
		hInsert = TVI_LAST;

	HTREEITEM hItem = m_tree.GetTree().InsertItem( _T("-untitled-"), 1, 1, hParent, hInsert );

	m_tree.GetTree().SelectAll( FALSE );
	m_tree.GetTree().SetItemState( hItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_tree.GetTree().EnsureVisible( hItem );

	GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnAddDivider()
{
	// without setting focus, subsequent inserts will not work 
	TempFocus tempFocus( m_tree );

	HTREEITEM hInsert = m_tree.GetTree().GetFocusedItem();
	HTREEITEM hParent = TVI_ROOT;
	if( hInsert )
		hParent = m_tree.GetTree().GetParentItem( hInsert );
	else
		hInsert = TVI_LAST;

	HTREEITEM hItem = m_tree.GetTree().InsertItem( _T(""), -1, -1, hParent, hInsert );
	m_tree.SetItemDivider( hItem );

	m_tree.GetTree().SelectAll( FALSE );
	m_tree.GetTree().SetItemState( hItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_tree.GetTree().EnsureVisible( hItem );

	GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnAddSubmenu()
{
	// without setting focus, subsequent inserts will not work 
	TempFocus tempFocus( m_tree );

	HTREEITEM hInsert = m_tree.GetTree().GetFocusedItem();
	HTREEITEM hParent = TVI_ROOT;
	if( hInsert )
		hParent = m_tree.GetTree().GetParentItem( hInsert );
	else
		hInsert = TVI_LAST;

	HTREEITEM hItem = m_tree.GetTree().InsertItem( _T("Submenu"), 0, 0, hParent, hInsert );
	m_tree.SetItemIsFolder( hItem );

	m_tree.GetTree().Expand( hItem, TVE_EXPAND );
	m_tree.GetTree().SelectAll( FALSE );
	m_tree.GetTree().SetItemState( hItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_tree.GetTree().EnsureVisible( hItem );

	GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnRemove()
{
	CTreeItemList selection;
	m_tree.GetTree().GetSelectedList( selection );
	m_tree.DeleteItemList( selection );

	// if an "-empty-" item is selected, then update button state by reselecting the item
	HTREEITEM hItem = m_tree.GetTree().GetFocusedItem();
	if( m_tree.IsItemDummy( hItem ) )
	{
		m_tree.GetTree().SelectAll( FALSE );
		m_tree.GetTree().SetItemState( hItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}

	GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnRevert()
{
	LoadFavorites();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnImport()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.

	GetDlgItem( IDC_BTN_REVERT )->EnableWindow( TRUE );
}

