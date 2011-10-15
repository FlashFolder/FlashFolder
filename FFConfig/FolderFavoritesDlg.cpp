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

#pragma warning(disable:4244) // numeric conversion

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
		title = ExtractSubPath( args.c_str() );
	else
		title = cmd;
	if( title.IsEmpty() )
		title = _T("-untitled-");
	return title;
}

//-----------------------------------------------------------------------------------------------

CFolderFavoritesDlg::CFolderFavoritesDlg(CWnd* pParent, int selectItemId )
	: CResizableDlg(CFolderFavoritesDlg::IDD, pParent),
	m_hSelItem( NULL ),
	m_isOwned( pParent ? true : false ),
	m_selectItemId( selectItemId ),
	m_selectTreeItem( NULL )
{}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CB_TITLE, m_cbTitle);
	DDX_Control(pDX, IDC_ED_COMMAND, m_edPath);
	DDX_Control(pDX, IDC_ED_TARGETPATH, m_edTargetPath);
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFolderFavoritesDlg, CResizableDlg)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED( IDC_BTN_BROWSE, OnBnClickedBtnBrowse )
	ON_NOTIFY( TVN_SELCHANGED, IDC_LST_FAVS, OnTree_SelChanged )
	ON_NOTIFY( CEditableTreeListCtrl::TVN_INSERTITEM, IDC_LST_FAVS, OnTree_InsertItem )
	ON_NOTIFY( TVN_DELETEITEM, IDC_LST_FAVS, OnTree_DeleteItem )
	ON_EN_CHANGE( IDC_ED_COMMAND, OnEnChangeEdCommand )
	ON_EN_CHANGE( IDC_ED_TARGETPATH, OnEnChangeEdTargetPath )
	ON_CBN_EDITCHANGE(IDC_CB_TITLE, &CFolderFavoritesDlg::OnCbnEditchangeCbTitle)
	ON_BN_CLICKED( IDC_BTN_ADD, OnBnClickedBtnAdd )
	ON_BN_CLICKED( IDC_BTN_ADD_DIVIDER, OnBnClickedBtnAddDivider )
	ON_BN_CLICKED( IDC_BTN_ADD_SUBMENU, OnBnClickedBtnAddSubmenu )
	ON_BN_CLICKED( IDC_BTN_REMOVE, OnBnClickedBtnRemove )
	ON_BN_CLICKED(IDC_BTN_TARGETBROWSE, OnBnClickedBtnTargetbrowse)
	ON_BN_CLICKED(IDC_BTN_REVERT, OnBnClickedBtnRevert)
	ON_WM_CONTEXTMENU()
	ON_CBN_SELENDOK(IDC_CB_TITLE, &CFolderFavoritesDlg::OnCbnSelendokCbTitle)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CFolderFavoritesDlg::OnInitDialog()
{
	CResizableDlg::OnInitDialog();

	const Profile& profile = CApp::GetReadProfile();

	CString exeName;
	GetAppFilename( AfxGetInstanceHandle(), exeName.GetBuffer( 4096 ), 4096 );
	exeName.ReleaseBuffer();

	if( m_isOwned )
	{
		ModifyStyle( WS_MINIMIZEBOX, 0, SWP_FRAMECHANGED );
		GetSystemMenu( FALSE )->EnableMenuItem( SC_MINIMIZE, MF_BYCOMMAND | MF_GRAYED );
	}
	
	//--- Enable autocomplete

	::SHAutoComplete( m_edPath, SHACF_FILESYS_DIRS | SHACF_AUTOSUGGEST_FORCE_ON );
	::SHAutoComplete( m_edTargetPath, SHACF_FILESYS_DIRS | SHACF_AUTOSUGGEST_FORCE_ON );
	
	CWnd* pTitleEdit = m_cbTitle.GetWindow( GW_CHILD );
	m_edTitle.SubclassWindow( *pTitleEdit );

	//--- Create and populate tree control
	
	CRect rcTree; GetDlgItem( IDC_ST_TREE_PLACEHOLDER )->GetWindowRect( rcTree );
	ScreenToClient( rcTree );
	m_tree.Create( this, rcTree, IDC_LST_FAVS, WS_CHILD | WS_VISIBLE |
		TVS_HASBUTTONS | TVS_FULLROWSELECT | TVS_LINESATROOT | TVS_SHOWSELALWAYS );
	m_tree.SetWindowPos( GetDlgItem( IDC_ST_FAVMENU ), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	m_tree.SetFont( GetFont() );
	m_tree.GetHeaderCtrl().ModifyStyle( HDS_BUTTONS, 0 );

	int iconCX = 16, iconCY = 16;

	CRect rcItem( 0, 0, 1, 12 ); MapDialogRect( rcItem );
	m_tree.GetTree().SetItemHeight( max( rcItem.bottom, iconCY ) );

	m_tree.SetDragCursor( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDC_DRAGCURSOR ) );

	m_treeIcons.Create( iconCX, iconCY, ILC_COLOR32 | ILC_MASK, 1, 1 );
	m_treeIcons.GetCacheIcon( AfxGetInstanceHandle(), IDI_SUBMENU, exeName );
	m_treeIcons.GetCacheIcon( AfxGetInstanceHandle(), IDI_MENUITEM, exeName );
	m_tree.GetTree().SetImageList( &m_treeIcons, TVSIL_NORMAL );

	int colWidth;
	colWidth = MapProfileX( *this, profile.GetInt( _T("Favorites.Options"), _T("ColWidth_title") ) );
	m_tree.InsertColumn( 0, _T("Title"), LVCFMT_LEFT, colWidth );

	colWidth = MapProfileX( *this, profile.GetInt( _T("Favorites.Options"), _T("ColWidth_command") ) );
	m_tree.InsertColumn( 1, _T("Command"), LVCFMT_LEFT, colWidth );

	colWidth = MapProfileX( *this, profile.GetInt( _T("Favorites.Options"), _T("ColWidth_targetPath") ) );
	m_tree.InsertColumn( 2, _T("Target path"), LVCFMT_LEFT, colWidth );

	LoadFavorites();

	//--- Dialog resizing stuff	

	Anchor( IDC_LST_FAVS, ANCHOR_ALL );
	Anchor( IDC_ST_TITLE, ANCHOR_BOTTOMLEFT );
	Anchor( IDC_ST_PATH, ANCHOR_BOTTOMLEFT );
	Anchor( IDC_ST_TARGETPATH, ANCHOR_BOTTOMLEFT );
	Anchor( IDC_CB_TITLE, ANCHOR_BOTTOMLEFT | ANCHOR_RIGHT );
	Anchor( IDC_ED_COMMAND, ANCHOR_BOTTOMLEFT | ANCHOR_RIGHT );
	Anchor( IDC_ED_TARGETPATH, ANCHOR_BOTTOMLEFT | ANCHOR_RIGHT );
	Anchor( IDC_BTN_BROWSE, ANCHOR_BOTTOMRIGHT );
	Anchor( IDC_BTN_TARGETBROWSE, ANCHOR_BOTTOMRIGHT );
	Anchor( IDOK, ANCHOR_BOTTOMRIGHT );
	Anchor( IDCANCEL, ANCHOR_BOTTOMRIGHT );
	Anchor( IDC_BTN_REVERT, ANCHOR_BOTTOMRIGHT );

	// Set size from dialog template as minimum size
	SetMinSize();

	// get dialog size from profile
	int width = MapProfileX( *this, profile.GetInt( _T("main"), _T("FavoritesDlgWidth") ) );
	int height = MapProfileY( *this, profile.GetInt( _T("main"), _T("FavoritesDlgHeight") ) );

	SetWindowPos( NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE );

	if( m_selectTreeItem )
	{
		m_tree.GetTree().EnsureVisible( m_selectTreeItem );
		m_tree.GetTree().Select( m_selectTreeItem, TVGN_CARET );
	}

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

	LoadFavorites_worker( TVI_ROOT, TVI_LAST, favs, iItem );

	EnableDlgItem( *this, IDC_BTN_REVERT, FALSE );

	// select first item (if any)
	m_tree.GetTree().SelectAll( FALSE );
	if( HTREEITEM hItem = m_tree.GetTree().GetChildItem( TVI_ROOT ) )
		m_tree.GetTree().SetItemState( hItem, TVIS_SELECTED | TVIS_FOCUSED, TVIS_SELECTED | TVIS_FOCUSED );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::LoadFavorites_worker( 
	HTREEITEM hParent, HTREEITEM hInsertAfter, const FavoritesList& favs, size_t& iItem, 
	bool bSelectInsertedItems )
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

			hItem = m_tree.InsertItem( fav.title.substr( 1 ).c_str(), 0, 0, hParent, hInsertAfter );
			m_tree.SetItemIsFolder( hItem );

			++iItem;

			LoadFavorites_worker( hItem, TVI_FIRST, favs, iItem, false );			
		}
		else if( fav.title == _T("-") )
		{
			// insert divider
			
			hItem = m_tree.InsertItem( _T(""), -1, -1, hParent, hInsertAfter );
			m_tree.SetItemDivider( hItem );

			++iItem;
		}
		else
		{
			// insert normal item

			hItem = m_tree.InsertItem( fav.title.c_str(), 1, 1, hParent, hInsertAfter );
			m_tree.SetItemText( hItem, COL_COMMAND, fav.command.c_str() );
			m_tree.SetItemText( hItem, COL_TARGETPATH, fav.targetpath.c_str() );
			
			if( iItem == m_selectItemId )
				m_selectTreeItem = hItem;

			++iItem;
		}

		// select inserted root items
		if( bSelectInsertedItems )
			m_tree.GetTree().SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );

		hInsertAfter = hItem;
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
		if( m_tree.IsItemDummy( hItem ) )
			continue;

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

			// end of submenu
			FavoritesItem fav;
			fav.title = _T("--");
			favs.push_back( fav );
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
	Profile& profile = CApp::GetWriteProfile();

	CRect rc; GetWindowRect( rc );
	profile.SetInt( _T("main"), _T("FavoritesDlgWidth"), rc.Width() );
	profile.SetInt( _T("main"), _T("FavoritesDlgHeight"), rc.Height() );

	int colWidth;
	colWidth = m_tree.GetHeaderCtrl().GetItemWidth( COL_TITLE );
	profile.SetInt( _T("Favorites.Options"), _T("ColWidth_title"), colWidth );

	colWidth = m_tree.GetHeaderCtrl().GetItemWidth( COL_COMMAND );
	profile.SetInt( _T("Favorites.Options"), _T("ColWidth_command"), colWidth );

	colWidth = m_tree.GetHeaderCtrl().GetItemWidth( COL_TARGETPATH );
	profile.SetInt( _T("Favorites.Options"), _T("ColWidth_targetPath"), colWidth );
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
	{
		// clear edit controls without updating tree
		HTREEITEM oldSel = m_hSelItem; m_hSelItem = NULL;
		SetDlgItemText( IDC_CB_TITLE, _T("") );
		SetDlgItemText( IDC_ED_COMMAND, _T("") );
		SetDlgItemText( IDC_ED_TARGETPATH, _T("") );
		m_edTitle.SetHintText( _T("") );
		m_hSelItem = oldSel;
		return;
	}

	CString title, command, targetPath, iconPath;
	if( ! m_tree.IsItemDummy( m_hSelItem ) )
	{
		title = m_tree.GetItemText( m_hSelItem, COL_TITLE );
		command = m_tree.GetItemText( m_hSelItem, COL_COMMAND );
		targetPath = m_tree.GetItemText( m_hSelItem, COL_TARGETPATH );
	}

	// disable "reverse update" of list control
	HTREEITEM curSel = m_hSelItem;
	m_hSelItem = NULL;

	CString defTitle = GetDefaultTitle( command );

	if( title == _T("-untitled-") || ComparePath( title, defTitle ) == 0 )
	{
		m_edTitle.SetHintText( title );
		title = _T("");
	}
	SetDlgItemText( IDC_CB_TITLE, title );

	SetDlgItemText( IDC_ED_COMMAND, command );
	SetDlgItemText( IDC_ED_TARGETPATH, targetPath );
	
	m_hSelItem = curSel;
}

//-----------------------------------------------------------------------------------------------
// Update states/content of controls upon selection change in tree

void CFolderFavoritesDlg::OnTree_SelChanged( NMHDR *pNMHDR, LRESULT *pResult )
{
	*pResult = 0;
	LPNMTREEVIEW pnm = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	HTREEITEM hFocus = m_tree.GetTree().GetFocusedItem();
	if( hFocus != m_hSelItem )
	{
		m_hSelItem = hFocus;

		UpdateSelItemEditControls();

		BOOL bBtnRemove = FALSE;
		BOOL bEdTitle = FALSE;
		BOOL bCommandAndTarget = FALSE;

		if( ! m_tree.IsItemDummy( m_hSelItem ) )
		{
			bBtnRemove = TRUE;

			if( ! m_tree.IsItemDivider( m_hSelItem ) )
			{
				bEdTitle = TRUE;

				if( ! m_tree.GetTree().ItemHasChildren( m_hSelItem ) )
				{
					bCommandAndTarget = TRUE;
				}
			}
		}
		
		EnableDlgItem( *this, IDC_BTN_REMOVE, bBtnRemove );
		EnableDlgItem( *this, IDC_CB_TITLE, bEdTitle );
		EnableDlgItem( *this, IDC_ED_COMMAND, bCommandAndTarget );
		EnableDlgItem( *this, IDC_BTN_BROWSE, bCommandAndTarget );
		EnableDlgItem( *this, IDC_ED_TARGETPATH, bCommandAndTarget );
		EnableDlgItem( *this, IDC_BTN_TARGETBROWSE, bCommandAndTarget );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnTree_InsertItem( NMHDR *pNMHDR, LRESULT *pResult )
{
	*pResult = 0;
	EnableDlgItem( *this, IDC_BTN_REVERT );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnTree_DeleteItem( NMHDR *pNMHDR, LRESULT *pResult )
{
	*pResult = 0;
	EnableDlgItem( *this, IDC_BTN_REVERT );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnCbnEditchangeCbTitle()
{
	CString title;
	m_edTitle.GetWindowText( title );

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

		EnableDlgItem( *this, IDC_BTN_REVERT );
	}
}

void CFolderFavoritesDlg::OnCbnSelendokCbTitle()
{
	CString title; 
	int sel = m_cbTitle.GetCurSel();
	if( sel != CB_ERR )
	{
		m_cbTitle.GetLBText( sel, title );
		m_edTitle.SetWindowText( title ); 
		OnCbnEditchangeCbTitle();
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
	
	UpdateTitleSuggestions();

	// update current list item
	if( m_hSelItem )
	{
		if( GetDlgItem( IDC_CB_TITLE )->GetWindowTextLength() == 0 )
			m_tree.SetItemText( m_hSelItem, COL_TITLE, title );

		m_tree.SetItemText( m_hSelItem, COL_COMMAND, cmd );

		EnableDlgItem( *this, IDC_BTN_REVERT );
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

		EnableDlgItem( *this, IDC_BTN_REVERT );
	}
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::UpdateTitleSuggestions()
{
	// Create title suggestions for dropdown-list of title combobox based on current command

	CString title; m_cbTitle.GetWindowText( title );
	m_cbTitle.ResetContent();
	m_cbTitle.SetWindowText( title );
	
	CString cmd; GetDlgItemText( IDC_ED_COMMAND, cmd );
	tstring token, args;
	SplitTcCommand( cmd, &token, &args );
	CString path;
	if( CString( token.c_str() ).CompareNoCase( _T("cd") ) == 0 )
		path = args.c_str();
	else
		path = cmd;
		
	CString sub1 = ExtractSubPath( path );
	m_cbTitle.AddString( sub1 );
	CString sub2 = ExtractSubPath( path, 2 );
	if( sub2 != sub1 )
		m_cbTitle.AddString( sub2 );
	if( path != sub2 )
		m_cbTitle.AddString( path );
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

	HTREEITEM hItem = m_tree.InsertItem( _T("-untitled-"), 1, 1, hParent, hInsert );

	m_tree.GetTree().SelectAll( FALSE );
	m_tree.GetTree().SetItemState( hItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_tree.GetTree().EnsureVisible( hItem );

	EnableDlgItem( *this, IDC_BTN_REVERT );
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

	HTREEITEM hItem = m_tree.InsertItem( _T(""), -1, -1, hParent, hInsert );
	m_tree.SetItemDivider( hItem );

	m_tree.GetTree().SelectAll( FALSE );
	m_tree.GetTree().SetItemState( hItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_tree.GetTree().EnsureVisible( hItem );

	EnableDlgItem( *this, IDC_BTN_REVERT );
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

	HTREEITEM hItem = m_tree.InsertItem( _T("Submenu"), 0, 0, hParent, hInsert );
	m_tree.SetItemIsFolder( hItem );

	m_tree.GetTree().Expand( hItem, TVE_EXPAND );
	m_tree.GetTree().SelectAll( FALSE );
	m_tree.GetTree().SetItemState( hItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	m_tree.GetTree().EnsureVisible( hItem );

	EnableDlgItem( *this, IDC_BTN_REVERT );
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnRemove()
{
	m_tree.Delete();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnBnClickedBtnRevert()
{
	LoadFavorites();
}

//-----------------------------------------------------------------------------------------------

void CFolderFavoritesDlg::OnContextMenu(CWnd* pWnd, CPoint pt )
{
	if( pWnd != &m_tree.GetTree() )
	{
		CDialog::OnContextMenu( pWnd, pt );
		return;
	}

	CPoint menuPos;

	if( pt.x != -1 || pt.y != -1 )
	{
		// Context menu has been invoked by mouse, position it at cursor pos.

		menuPos = pt;		
	}
	else
	{
		// Context menu handler has not been invoked by mouse, e.g. by Shift+F10.
		// Set context menu position to focused item, if any.

		if( HTREEITEM hFocused = m_tree.GetTree().GetFocusedItem() )
		{
			CRect rc; 
			m_tree.GetTree().GetItemRect( hFocused, rc, TRUE );
			menuPos = CPoint( rc.left + MapDialogX( *this, 24 ), rc.bottom );
		}
		else
		{
			menuPos = MapDialogPoint( *this, CPoint( 8, 16 ) );			
		}

		// Clip menu pos to tree rectangle (since focused item could be scolled out of view).
		CRect rcClient;
		m_tree.GetClientRect( rcClient );
		m_tree.MapWindowPoints( &m_tree.GetTree(), rcClient );
		menuPos.x = min( max( rcClient.left, menuPos.x ), rcClient.right - MapDialogX( *this, 40 ) );
		menuPos.y = min( max( rcClient.top, menuPos.y ), rcClient.bottom - MapDialogY( *this, 20 ) );	

		m_tree.GetTree().ClientToScreen( &menuPos );
	}

	CMenu menu;
	menu.LoadMenu( IDR_FAVORITES_CONTEXTMENU );

	if( m_tree.GetTree().GetSelectedCount() == 0 )
	{
		menu.EnableMenuItem( ID_FAVCONTEXT_CUT, MF_BYCOMMAND | MF_GRAYED );
		menu.EnableMenuItem( ID_FAVCONTEXT_COPY, MF_BYCOMMAND | MF_GRAYED );
		menu.EnableMenuItem( ID_FAVCONTEXT_REMOVE, MF_BYCOMMAND | MF_GRAYED );
	}
	if( ! m_tree.CanPaste() )
		menu.EnableMenuItem( ID_FAVCONTEXT_PASTE, MF_BYCOMMAND | MF_GRAYED );

    UINT cmd = (UINT) menu.GetSubMenu( 0 )->TrackPopupMenu( TPM_RETURNCMD | TPM_LEFTALIGN, menuPos.x, menuPos.y, this );
	if( cmd == 0 )
		return;

	switch( cmd )
	{
		case ID_FAVCONTEXT_NEWITEM:
			OnBnClickedBtnAdd();
			break;
		case ID_FAVCONTEXT_NEWDIVIDER:
			OnBnClickedBtnAddDivider();
			break;
		case ID_FAVCONTEXT_NEWSUBMENU:
			OnBnClickedBtnAddSubmenu();
			break;
		case ID_FAVCONTEXT_CUT:
			m_tree.Cut();
			break;
		case ID_FAVCONTEXT_COPY:
			m_tree.Copy();
			break;
		case ID_FAVCONTEXT_PASTE:
			m_tree.Paste();
			break;
		case ID_FAVCONTEXT_REMOVE:
			m_tree.Delete();
			break;
		default: ASSERT( false );
	}
}
