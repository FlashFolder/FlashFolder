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
#include "AddFavoriteDlg.h"
#include "FolderFavoritesDlg.h"

//-----------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CAddFavoriteDlg, CDialog)

//-----------------------------------------------------------------------------------------------

CAddFavoriteDlg::CAddFavoriteDlg( CWnd* pParent, const CString& path, const CString& targetPath )
	: CDialog(CAddFavoriteDlg::IDD, pParent),
	m_path( path ), 
	m_targetPath( targetPath ),
	m_newItemId( -1 )
{}

//-----------------------------------------------------------------------------------------------

void CAddFavoriteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_CB_TITLE, m_cbTitle );
}

//-----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAddFavoriteDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAddFavoriteDlg::OnBnClickedOk)
	ON_CBN_DBLCLK(IDC_CB_TITLE, &CAddFavoriteDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_EDIT_MENU, &CAddFavoriteDlg::OnBnClickedEditMenu)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------

BOOL CAddFavoriteDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Create menu item title suggestions
	CString sub1 = ExtractSubPath( m_path );
	m_cbTitle.AddString( sub1 );
	CString sub2 = ExtractSubPath( m_path, 2 );
	if( sub2 != sub1 )
		m_cbTitle.AddString( sub2 );
	if( m_path != sub2 )
		m_cbTitle.AddString( m_path );
	m_cbTitle.SetCurSel( 0 );	
	
	if( m_targetPath.IsEmpty() )
		GetDlgItem( IDC_CHK_SAVE_TARGETDIR )->ShowWindow( SW_HIDE );

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

bool CAddFavoriteDlg::Save()
{
	FavoritesList favs;
	GetDirFavorites( &favs );
	
	//--- Check for duplicate favorite item		
	CString checkTargetPath;
	if( IsDlgButtonChecked( IDC_CHK_SAVE_TARGETDIR ) == 1 )
		checkTargetPath = m_targetPath;
	int duplicateItemId = GetFavItemByPath( favs, m_path, checkTargetPath );
	if( duplicateItemId != -1 )
	{
		int res = CMessageBoxEx::Show( *this,  
			_T("There already exists a favorites item with the same folder path.\n\n")
			_T("You can choose to either replace the existing item or continue to add the duplicate item."), 
			_T("Duplicate Favorites Item"),
			3, 3, IDI_WARNING,
			_T("&Replace existing"), _T("&Add duplicate"), _T("Cancel") );
		if( res == IDCANCEL )
		{
			EndDialog( IDCANCEL );
			return false;
		}
		if( res == CMessageBoxEx::ID_BUTTON1 )
		{
			favs.erase( favs.begin() + duplicateItemId );
		}
	}
	
	//--- Add new item
	FavoritesItem item;
	CString title; m_cbTitle.GetWindowText( title );
	item.title = title;
	item.command = tstring( _T("cd ") ) + m_path.GetString();
	if( IsDlgButtonChecked( IDC_CHK_SAVE_TARGETDIR ) == 1 )
		item.targetpath = m_targetPath.GetString();
	favs.push_back( item );
	
	m_newItemId = favs.size() - 1 ;
	
	SetDirFavorites( favs );

	return true;	
}

//-----------------------------------------------------------------------------------------------

void CAddFavoriteDlg::OnBnClickedOk()
{
	if( Save() )
		EndDialog( IDOK );
}

//-----------------------------------------------------------------------------------------------

void CAddFavoriteDlg::OnBnClickedEditMenu()
{
	if( Save() )
		EndDialog( ID_EDITMENU );		
}

