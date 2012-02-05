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
#pragma once

#include "HookBase.h"
#include "resource.h"

class ToolWnd : public ATL::CDialogImpl< ToolWnd >
{
public:
	typedef ATL::CDialogImpl< ToolWnd > base;

	enum { IDD = IDD_TOOLWND };

	ToolWnd() : 
		m_hFileDialog( NULL ),
		m_fileDlgHook( NULL ),
		m_pluginMgr( NULL ),
		m_hToolbarImages( NULL ),
		m_hStdFont( NULL ),
		m_hGetMessageHook( NULL ),
		m_hAccTable( NULL ),
		m_isToolWndVisible( false )
	{}

	HWND Create( HWND hFileOrFolderDialog, bool isFileDialog, FileDlgHookBase* hook, const PluginManager* pluginMgr );

	void OnFileDialogInitDone();
	void OnFileDialogFolderChange();
	void OnFileDialogResize();
	void OnFileDialogEnable( bool bEnable );
	void OnFileDialogShow( bool bShow );
	void OnFileDialogActivate( WPARAM wParam, LPARAM lParam );
	void OnFileDialogDestroy( bool isOkBtnPressed );
	void OnFileDialogSetTimer( DWORD interval );

protected:
	BEGIN_MSG_MAP( ToolWnd )
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		MESSAGE_HANDLER( WM_NCDESTROY, OnNcDestroy )	
		MESSAGE_HANDLER( WM_COMMAND, OnCommand )
		MESSAGE_HANDLER( WM_NOTIFY, OnNotify )
		MESSAGE_HANDLER( WM_WINDOWPOSCHANGING, OnWindowPosChanging ) 
		MESSAGE_HANDLER( WM_WINDOWPOSCHANGED, OnWindowPosChanged )
		MESSAGE_HANDLER( WM_CLOSE, OnClose )
		MESSAGE_HANDLER( WM_TIMER, OnTimer )
		MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBkgnd )
		MESSAGE_HANDLER( WM_PAINT, OnPaint )		
		MESSAGE_HANDLER( WM_PRINTCLIENT, OnPrintClient )
		MESSAGE_HANDLER( WM_NCACTIVATE, OnNcActivate )
		MESSAGE_HANDLER( WM_NCCALCSIZE, OnNcCalcSize )
	END_MSG_MAP()

	LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnNcDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnNotify( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnWindowPosChanging( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnWindowPosChanged( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnEraseBkgnd( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnPrintClient( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnNcActivate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT OnNcCalcSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

private:
	static LRESULT CALLBACK EditPathProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
		UINT_PTR subclassId, DWORD_PTR refData );
	LRESULT EditPathProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK GetMessageProc( int code, WPARAM wParam, LPARAM lParam );

	void AdjustToolWindowPos();
	void ExecuteToolbarCommand( UINT cmd );
	HACCEL CreateMyAcceleratorTable();
	LPCTSTR GetCommandName( int cmd );
	void NavigateToFolder( PCIDLIST_ABSOLUTE folder );
	void UpdatePathEdit();
	HMENU CreateFolderMenu( const std::vector<tstring> &folderList, HMENU hMenu = NULL );
	void FavMenu_Create( HMENU hMenu, const PluginManager::FavoriteItems& favs, size_t& iItem );
	int FavMenu_Display( int x, int y, const PluginManager::FavoriteItems& favs );
	void DisplayMenu_Favorites();
	void FavMenu_AddFolder( LPCWSTR displayName, PCIDLIST_ABSOLUTE folder );
	int DisplayFolderMenu( HMENU hMenu, int buttonID );
	void DisplayMenu_OpenDirs();
	void GotoLastDir();
	void DisplayMenu_GlobalHist();
	void DisplayMenu_Config();

	HWND m_hFileDialog;
	FileDlgHookBase* m_fileDlgHook;
	Profile* m_profile;
	const PluginManager* m_pluginMgr;
	
	HACCEL m_hAccTable;
	HHOOK m_hGetMessageHook;
	typedef std::map< HWND, ToolWnd* > GetMessageMap;
	static GetMessageMap s_getMessageMap;

	HIMAGELIST m_hToolbarImages;
	HFONT m_hStdFont;
	Rect m_toolbarOffset;
	bool m_isToolWndVisible;
};
