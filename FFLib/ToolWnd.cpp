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
#include "StdAfx.h"
#include "resource.h"
#include "ToolWnd.h"
#include "utils.h"
#include "fflib.h"

//----------------------------------------------------------------------------------------------------

ToolWnd::GetMessageMap ToolWnd::s_getMessageMap;

//-----------------------------------------------------------------------------------------

BOOL CALLBACK ToolWndSetFont( HWND hwnd, LPARAM lParam )
{
	SendMessage( hwnd, WM_SETFONT, lParam, 0 );
	return TRUE;
}

//-----------------------------------------------------------------------------------------

HWND ToolWnd::Create( HWND hFileOrFolderDialog, bool isFileDialog, FileDlgHookBase* hook, const PluginManager* pluginMgr )
{
	HWND result = NULL;

	m_hFileDialog = hFileOrFolderDialog;
	m_fileDlgHook = hook;
	m_pluginMgr = pluginMgr;

	bool isThemed = IsThemeSupportedAndActive();
	bool isComposited = IsCompositionSupportedAndActive();

	//--- create the window ---
	{
		m_isToolWndVisible = false;

		result = base::Create( hFileOrFolderDialog );
		if( result == NULL )
			return NULL;

		int height = 0;
		DWORD wndPosFlags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;

		if( isComposited )
		{
			height = MapDialogY( m_hWnd, 20 );			
		}
		else
		{
			LONG_PTR style = ::GetWindowLongPtr( m_hWnd, GWL_STYLE );
			SetWindowLongPtr( GWL_STYLE, style & ~( WS_CAPTION | DS_MODALFRAME ) );
			wndPosFlags |= SWP_FRAMECHANGED;

			height = MapDialogY( m_hWnd, 18 );
		}

		::SetWindowPos( m_hWnd, NULL, 0, 0, 0, height, wndPosFlags );			
		AdjustToolWindowPos();
	}

	Rect rcClient; 
	GetClientRect( &rcClient );

	//--- create the toolbar ---

	HWND hTb = NULL;
	Rect tbRect;
	{
		vector<TBBUTTON> tbButtons;
		{
			TBBUTTON btn = { 5, ID_FF_LASTDIR,                 0, BTNS_BUTTON, 0, 0, 0, 0 };
			tbButtons.push_back( btn );
		}
		{
			TBBUTTON btn = { 0, ID_FF_GLOBALHIST,              0, BTNS_BUTTON, 0, 0, 0, 0 };
			tbButtons.push_back( btn );
		}
		{
			TBBUTTON btn = { 2, ID_FF_OPENDIRS,  TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0, 0 };
			tbButtons.push_back( btn );
		}
		{
			TBBUTTON btn = { 3, ID_FF_FAVORITES, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0, 0 };
			tbButtons.push_back( btn );
		}	  
		if( isFileDialog )
		{
			TBBUTTON btn = { 4, ID_FF_SHOWALL,   TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0, 0 };
			tbButtons.push_back( btn );
		}
		{
			TBBUTTON btn = { 6, ID_FF_CONFIG,    TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0, 0 };
			tbButtons.push_back( btn );
		}

		// Check whether the 32 bpp version of the toolbar bitmap is supported. 
		// For this, OS must be >= WinXP and display mode >= 16 bpp.
		bool isToolbar32bpp = false;
		if( OSVERSION >= WINVER_XP )
		{
			DC dc( ::CreateIC( L"DISPLAY", NULL, NULL, NULL ) );
			isToolbar32bpp = ::GetDeviceCaps( dc, BITSPIXEL ) >= 16;
		}
		
		UINT tbStyle = WS_CHILD | WS_TABSTOP | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT |
			CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_CUSTOMERASE;

		hTb = ::CreateToolbarEx( m_hWnd, tbStyle,
			ID_FF_TOOLBAR, (int) tbButtons.size(), 
			isToolbar32bpp ? NULL : _AtlBaseModule.GetModuleInstance(), isToolbar32bpp ? 0 : ID_FF_TOOLBAR, 
			&tbButtons[ 0 ], (int) tbButtons.size(), 16,16, 16,16, sizeof(TBBUTTON) );

		::SendMessage( hTb, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );
		::ShowWindow( hTb, SW_SHOW );

		if( isToolbar32bpp )
		{
			m_hToolbarImages = ::ImageList_LoadImage( _AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE( ID_FF_TOOLBAR_XP ), 
				16, 1, CLR_NONE, IMAGE_BITMAP, LR_CREATEDIBSECTION );
			::SendMessage( hTb, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>( m_hToolbarImages ) );
		}

		// calculate width of the toolbar from position of last button
		::SendMessage( hTb, TB_AUTOSIZE, 0, 0 );
		::SendMessage( hTb, TB_GETRECT, tbButtons.back().idCommand, reinterpret_cast<LPARAM>( &tbRect ) );

		tbRect.top = ( rcClient.bottom - tbRect.bottom ) / 2;
		tbRect.left = tbRect.top;
		tbRect.bottom += tbRect.top;
		tbRect.right += tbRect.left;
				
		::SetWindowPos( hTb, NULL, tbRect.left, tbRect.top, tbRect.Width(), tbRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
	}

	//--- create + sub-class the edit control 
	{
		Rect rcDiv( 0, 0, 6, 1 ); 
		Rect rcDivR( 0, 0, 4, 1 ); 
		// use themed border if possible
		DWORD edStyleEx = isThemed ? WS_EX_CLIENTEDGE : WS_EX_STATICEDGE;
		if( OSVERSION >= WINVER_VISTA && isThemed )
		{
			rcDiv.bottom = rcDivR.bottom = 2;
			edStyleEx = 0;
		}
		::MapDialogRect( m_hWnd, &rcDiv ); 
		::MapDialogRect( m_hWnd, &rcDivR ); 
		int xEdit = tbRect.right + rcDiv.right;
		int editHeight = MapDialogY( m_hWnd, 10 );

		HWND hEdit = ::CreateWindowEx( edStyleEx, L"Edit", NULL, 
			WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP, 
			xEdit, ( rcClient.bottom - editHeight ) / 2, 
			rcClient.right - rcClient.left - xEdit - rcDivR.right, 
			editHeight, 
			m_hWnd, (HMENU) ID_FF_PATH, _AtlBaseModule.GetModuleInstance(), NULL);

		if( isComposited )
			::SetWindowTheme( hEdit, NULL, L"EditComposited::Edit" );
							
		// enable auto-complete for the edit control
		::SHAutoComplete( hEdit, SHACF_FILESYS_DIRS | SHACF_AUTOSUGGEST_FORCE_ON );

		::SetWindowSubclass( hEdit, EditPathProc, 0, reinterpret_cast<DWORD_PTR>( this ) );
	}

    //--- set default font for all child controls
    
    if( ! m_hStdFont )
		m_hStdFont = CreateStandardOsFont();  
	EnumChildWindows( m_hWnd, ToolWndSetFont, (LPARAM) m_hStdFont );

	//--- read options from global configuration

	m_toolbarOffset.left = g_profile.GetInt( L"Toolbar", L"OffsetX" );
	m_toolbarOffset.top = g_profile.GetInt( L"Toolbar", L"OffsetY" );
	m_toolbarOffset.right = g_profile.GetInt( L"Toolbar", L"OffsetWidth" );
 
	//--- enable toolbar buttons / leave disabled ---
	
	if( ! g_profile.GetString( L"GlobalFolderHistory", L"0" ).empty() )
	{
		SendMessage( hTb, TB_SETSTATE, ID_FF_GLOBALHIST, MAKELONG( TBSTATE_ENABLED, 0 ) );
		SendMessage( hTb, TB_SETSTATE, ID_FF_LASTDIR, MAKELONG(TBSTATE_ENABLED, 0 ) );
	}

	return result;
}

//-----------------------------------------------------------------------------------------------

struct PressTbButton
{
	PressTbButton( HWND hwnd, UINT cmd ) : 
		m_hWnd( hwnd ), m_cmd( cmd ) 
	{
		::SendDlgItemMessage( m_hWnd, ID_FF_TOOLBAR, TB_PRESSBUTTON, m_cmd, TRUE );
	} 
	~PressTbButton()
	{
		::SendDlgItemMessage( m_hWnd, ID_FF_TOOLBAR, TB_PRESSBUTTON, m_cmd, FALSE );
	}
	
	HWND m_hWnd;
	UINT m_cmd;
};

//----------------------------------------------------------------------------------------------------

void ToolWnd::ExecuteToolbarCommand( UINT cmd )
{
	switch( cmd )
	{
		case ID_FF_LASTDIR:
		{
			GotoLastDir();
			break;
		}
		case ID_FF_SHOWALL:
		{
			m_fileDlgHook->SetFilter( L"*.*" );
			break;
		}
		case ID_FF_FOCUSPATH:
		{
			::SetForegroundWindow( m_hWnd );
			HWND hEdit = ::GetDlgItem( m_hWnd, ID_FF_PATH );
			::SendMessage( m_hWnd, WM_NEXTDLGCTL, (WPARAM) hEdit, TRUE ); 
			break;
		}
		case ID_FF_GLOBALHIST:
		{
			PressTbButton ptb( m_hWnd, ID_FF_GLOBALHIST );				
			DisplayMenu_GlobalHist();
			break;
		}
		case ID_FF_OPENDIRS:
		{
			PressTbButton ptb( m_hWnd, ID_FF_OPENDIRS );				
			DisplayMenu_OpenDirs();
			break;
		}
		case ID_FF_FAVORITES:
		{
			PressTbButton ptb( m_hWnd, ID_FF_FAVORITES );
			DisplayMenu_Favorites();
			break;
		}
		case ID_FF_CONFIG:
		{
			PressTbButton ptb( m_hWnd, ID_FF_CONFIG );
			DisplayMenu_Config();
			break;
		}
		default:
		{
			DebugOut( L"[fflib] ERROR: invalid command" );
		}
	}	
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if( IsCompositionSupportedAndActive() )
	{
		// make full glass window
		MARGINS glassMargins = { -1, -1, -1, -1 };
		::DwmExtendFrameIntoClientArea( m_hWnd, &glassMargins );

		// Inform the window of the frame change made by WM_NCCALCSIZE.
		::SetWindowPos( m_hWnd, NULL, 0, 0, 0, 0, 
			SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	}

	m_hAccTable = CreateMyAcceleratorTable();

	// This is the only reliable way to override default hotkeys of the file dialog if
	// the user whishes so.
	m_hGetMessageHook = ::SetWindowsHookEx( WH_GETMESSAGE, GetMessageProc, NULL, ::GetCurrentThreadId() );
	if( m_hGetMessageHook )
		s_getMessageMap[ m_hFileDialog ] = this;

	// no default focus wanted
	return FALSE;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnNcDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if( m_hGetMessageHook )
	{
		::UnhookWindowsHookEx( m_hGetMessageHook );
		m_hGetMessageHook = NULL;
		s_getMessageMap.erase( m_hFileDialog );
	}
	if( m_hAccTable )
	{
		::DestroyAcceleratorTable( m_hAccTable );
		m_hAccTable = NULL;
	}
	
	return 0;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	WORD wNotifyCode = HIWORD( wParam );             // notification code 
	WORD wID = LOWORD( wParam );                     // item, control, or accelerator identifier 

	if( wNotifyCode == BN_CLICKED || wNotifyCode == 1 )
		ExecuteToolbarCommand( wID );
	
	return 0;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnNotify( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	NMHDR* pnm = reinterpret_cast<NMHDR*>( lParam );
	switch( pnm->code )
	{
		case TBN_DROPDOWN:
		{
			// drop-down button pressed

			NMTOOLBAR* pnmt = reinterpret_cast<NMTOOLBAR*>( pnm );
			ExecuteToolbarCommand( pnmt->iItem );

			return TBDDRET_DEFAULT;
		}
		
		case TTN_NEEDTEXT:
		{
			// tooltip text requested
		
			NMTTDISPINFO* pTTT = reinterpret_cast<NMTTDISPINFO*>( pnm );

			pTTT->hinst = NULL;

			const int TOOLTIP_BUFSIZE = MAX_PATH + 64;
			static TCHAR s_tooltipBuf[ TOOLTIP_BUFSIZE + 1 ];
			s_tooltipBuf[ 0 ] = 0;
			pTTT->lpszText = s_tooltipBuf;
		
			if( pTTT->hdr.idFrom == ID_FF_LASTDIR )
			{
				// use most recent entry of global history as tooltip
				tstring sLastDir = g_profile.GetString( L"GlobalFolderHistory", L"0" );
				if( sLastDir.empty() )
					::LoadString( _AtlBaseModule.GetModuleInstance(), (UINT) pTTT->hdr.idFrom, s_tooltipBuf, TOOLTIP_BUFSIZE );
				else
					StringCbCopy( s_tooltipBuf, sizeof(s_tooltipBuf), sLastDir.c_str() );
			}
			else
			{
				::LoadString( _AtlBaseModule.GetModuleInstance(), (UINT) pTTT->hdr.idFrom, s_tooltipBuf, TOOLTIP_BUFSIZE );
			}

			// append hotkey name
			if( int hotkey = g_profile.GetInt( L"Hotkeys", GetCommandName( (int) pTTT->hdr.idFrom ) ) )
			{
				StringCchCat( s_tooltipBuf, TOOLTIP_BUFSIZE, L"\nShortcut: " ); 					
				TCHAR hkName[ 256 ]; GetHotkeyName( hkName, 255, hotkey );
				StringCchCat( s_tooltipBuf, TOOLTIP_BUFSIZE, hkName ); 					
			}
			
			break;
		}

		case NM_CUSTOMDRAW:
		{
			LPNMTBCUSTOMDRAW pcd = reinterpret_cast<LPNMTBCUSTOMDRAW>( lParam );

			HWND hTb = GetDlgItem( ID_FF_TOOLBAR );
			if( pcd->nmcd.hdr.hwndFrom == hTb && pcd->nmcd.dwDrawStage == CDDS_PREERASE )
			{
				// Use black brush to make toolbar background transparent on aero glass.
				HBRUSH brush = IsCompositionSupportedAndActive() ?
					GetStockBrush( BLACK_BRUSH ) : GetSysColorBrush( COLOR_BTNFACE );
				::FillRect( pcd->nmcd.hdc, ::GetClientRect( hTb ), brush );
				
				return CDRF_SKIPDEFAULT;
			}		

			break;
		}
	}
	
	return 0;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnWindowPosChanging( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled ) 
{
	// Make sure the toolbar window is initially shown hidden, since dialog manager always
	// calls ShowWindow( SW_SHOW ).
	WINDOWPOS* wp = (WINDOWPOS*) lParam;
    if( ! m_isToolWndVisible )
		wp->flags &= ~SWP_SHOWWINDOW;
	
	return 0;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnWindowPosChanged( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	//resize path edit control
	
	WINDOWPOS *wp = reinterpret_cast<WINDOWPOS*>( lParam );
	
	Rect rcClient = ::GetClientRect( m_hWnd );

	HWND hPath = GetDlgItem( ID_FF_PATH );
	Rect rcPath = ::GetChildRect( m_hWnd, hPath ); 

	Rect rcDivR( 0, 0, 4, 1 );
	MapDialogRect( &rcDivR ); 

	::SetWindowPos( hPath, NULL, 0, 0, rcClient.right - rcPath.left - rcDivR.right, rcPath.bottom - rcPath.top, 
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE );

	// Update glowing path text
	if( IsCompositionSupportedAndActive() && GetFocus() != hPath )
		InvalidateRect( NULL, FALSE );
	
	return 0;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	::PostMessage( m_hFileDialog, WM_CLOSE, 0, 0 );

	return 0;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	m_fileDlgHook->OnTimer();

	return 0;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnEraseBkgnd( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	return TRUE;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )		
{
	PaintDC dc( m_hWnd );
	return OnPrintClient( WM_PRINTCLIENT, reinterpret_cast<WPARAM>( static_cast<HDC>( dc ) ), PRF_CLIENT, bHandled );
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnPrintClient( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )	
{
	if( ! ( lParam & PRF_CLIENT ) )
	{
		bHandled = FALSE;
		return 0;
	}

	HDC dc = reinterpret_cast<HDC>( wParam );
	Rect rcClient; GetClientRect( &rcClient );
	
	if( IsCompositionSupportedAndActive() )
	{
		HWND hEdit = GetDlgItem( ID_FF_PATH );
		if( ::GetFocus() != hEdit )
		{
			// Draw glowing text for background of edit control when it is not focused.
			// So the text looks good on aero glass but still has enough contrast on dark background.
			
			Theme theme( m_hWnd, L"CompositedWindow::Window" );

			PaintBuf buf( rcClient );

			HFONT font = reinterpret_cast<HFONT>( ::SendMessage( hEdit, WM_GETFONT, 0, 0 ) );
			AutoSelectObj selFont( buf, font );

			WCHAR text[ MAX_PATH ] = L"";
			::GetWindowText( hEdit, text, _countof( text ) );

			Rect rcText = GetChildRect( m_hWnd, hEdit );
			rcText.left += static_cast<int>( ::SendMessage( hEdit, EM_GETMARGINS, 0, 0 ) & 0xFFFF );

			DTTOPTS opt = { sizeof( opt ), DTT_COMPOSITED | DTT_GLOWSIZE };
			opt.iGlowSize = 12;
			::DrawThemeTextEx( theme, buf, WP_CAPTION, CS_ACTIVE, text, -1, DT_NOPREFIX | DT_PATH_ELLIPSIS, &rcText, &opt );

			::BitBlt( dc, 0, 0, rcClient.right, rcClient.bottom, buf, 0, 0, SRCCOPY );
		}
		else
		{
			::FillRect( dc, rcClient, GetStockBrush( BLACK_BRUSH ) );
		}
	}
	else
	{
		::FillRect( dc, rcClient, ::GetSysColorBrush( COLOR_BTNFACE ) );
	}
	
	return 0;	
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnNcActivate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	// Keep the toolbar always in the inactive visual state, which looks better.
	if( ! wParam )
		bHandled = FALSE;
	return TRUE;
}

//----------------------------------------------------------------------------------------------------

LRESULT ToolWnd::OnNcCalcSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	// Extend client area into whole glass window (removing non-client area).
	if( IsCompositionSupportedAndActive() && wParam != FALSE )
		return 0;

	bHandled = FALSE;
	return 0;
}

//-----------------------------------------------------------------------------------------
/// Handles keyboard accelerators.

LRESULT CALLBACK ToolWnd::GetMessageProc( int code, WPARAM wParam, LPARAM lParam )
{
	if( code == HC_ACTION && wParam == PM_REMOVE )
	{
		MSG* msg = reinterpret_cast<MSG*>( lParam );

		// Get pointer to ToolWnd
		GetMessageMap::const_iterator it = s_getMessageMap.find( msg->hwnd );
		if( it != s_getMessageMap.end() )
		{
			// Try to handle accelerator message with ToolWnd.
			ToolWnd* thisPtr = it->second;

			if( thisPtr->m_hAccTable && 
			    ::TranslateAccelerator( thisPtr->m_hWnd, thisPtr->m_hAccTable, msg ) )
			{
				// Message handled -> prevent further message processing.
				msg->message = WM_NULL;
				msg->hwnd = NULL;
				msg->wParam = msg->lParam = 0;
			}
		}
	}

	return ::CallNextHookEx( NULL, code, wParam, lParam );
}

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK ToolWnd::EditPathProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
	UINT_PTR subclassId, DWORD_PTR refData )
{
	ToolWnd* thisPtr = reinterpret_cast< ToolWnd* >( refData );
	return thisPtr->EditPathProc( hwnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------------------------------

LRESULT ToolWnd::EditPathProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_KEYDOWN:
			if( wParam == VK_RETURN )
			{
				WCHAR path[ MAX_PATH ] = L"";
				::GetWindowText( hwnd, path, _countof( path ) );
				if( DirectoryExists( path ) )
				{
					SpITEMIDLIST folder;
					if( SUCCEEDED( GetPidlFromPath( &folder, path ) ) )
					{
						SetForegroundWindow( m_hFileDialog );
						m_fileDlgHook->SetFolder( folder.get() );
					}
				}
			}
			else if( wParam == VK_ESCAPE ) 
				::PostMessage( m_hFileDialog, WM_CLOSE, 0, 0 );
			break;

		case WM_LBUTTONDBLCLK:
			//select all on dbl-click / select & copy on ctrl + dbl-click
			if (wParam == MK_LBUTTON)
			{
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			}
			else if (wParam & MK_CONTROL)
			{
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				SendMessage(hwnd, WM_COPY, 0, 0);		
				return 0;
			}
			break;
			
		case WM_PAINT:
		{
			if( IsCompositionSupportedAndActive() && ::GetFocus() != hwnd )
			{
				// Paint only background of parent (glowing text) when not editing.								
				PaintDC dc( hwnd );
				if( dc )
				{
					Rect rcClient; ::GetClientRect( hwnd, &rcClient );
					PaintBuf buf( rcClient );
					::DrawThemeParentBackground( hwnd, buf, &dc.PS().rcPaint );
					::BitBlt( dc, dc.PS().rcPaint.left, dc.PS().rcPaint.top, buf.GetWidth(), buf.GetHeight(), 
					          buf, dc.PS().rcPaint.left, dc.PS().rcPaint.top, SRCCOPY );
				}		
				return 0;
			}
		
			break;
		}
		
		case WM_SETFOCUS:
			// redraw parent background to remove glowing text
			::InvalidateRect( m_hWnd, NULL, FALSE );
			break;

		case WM_KILLFOCUS:
			// redraw parent background to show glowing text
			::InvalidateRect( m_hWnd, NULL, FALSE );
			break;

		case WM_SETTEXT:
			// redraw parent background to update glowing text
			if( ::GetFocus() != hwnd )
			{
				LRESULT res = ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
				::InvalidateRect( m_hWnd, NULL, FALSE );
				return res;
			}
			break;
	}

	//call original message handler
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------------------------------

void ToolWnd::UpdatePathEdit()
{
	if( SpITEMIDLIST folder = m_fileDlgHook->GetFolder() )
	{
		WCHAR path[ MAX_PATH ] = L"";
		if( ::SHGetPathFromIDList( folder.get(), path ) )
			::SetDlgItemText( m_hWnd, ID_FF_PATH, path );
	}
}

//-----------------------------------------------------------------------------------------

void ToolWnd::AdjustToolWindowPos()
{
	//calculates the position + size of the tool window accordingly to the size
	// of the file dialog

	Rect rc;
	::GetWindowRect( m_hFileDialog, &rc );

	Rect rcTool; 
	::GetWindowRect( m_hWnd, &rcTool );
	rcTool.left = rc.left + m_toolbarOffset.left;
	rcTool.top = rc.top - rcTool.bottom + rcTool.top + m_toolbarOffset.top;
	rcTool.right = rc.right + m_toolbarOffset.left + m_toolbarOffset.right;
	rcTool.bottom = rc.top + m_toolbarOffset.top;

	::SetWindowPos( m_hWnd, NULL, rcTool.left, rcTool.top, rcTool.right - rcTool.left, rcTool.bottom - rcTool.top, 
		            SWP_NOZORDER | SWP_NOACTIVATE );
}

//-----------------------------------------------------------------------------------------------

HACCEL ToolWnd::CreateMyAcceleratorTable()
{
	struct
	{
		LPCWSTR name;
		UINT id;
	}
	cmdList[] = {
		{ L"ff_LastFolder", ID_FF_LASTDIR },
		{ L"ff_ViewAllFiles", ID_FF_SHOWALL },
		{ L"ff_FocusPathEdit", ID_FF_FOCUSPATH },
		{ L"ff_MenuFolderHistory", ID_FF_GLOBALHIST },
		{ L"ff_MenuOpenFolders", ID_FF_OPENDIRS },
		{ L"ff_MenuFavorites", ID_FF_FAVORITES },
		{ L"ff_MenuConfig", ID_FF_CONFIG }
	};

	vector< ACCEL > accel;

	for( int i = 0; i < _countof( cmdList ); ++i )
	{
		int hotkey = g_profile.GetInt( L"Hotkeys", cmdList[ i ].name );  
		if( hotkey != 0 )
		{
			UINT vk, mod;
			SplitHotKey( &vk, &mod, hotkey );
			ACCEL ac = { FVIRTKEY, vk, cmdList[ i ].id };
			if( mod & MOD_CONTROL )
				ac.fVirt |= FCONTROL;
			if( mod & MOD_ALT )
				ac.fVirt |= FALT;
			if( mod & MOD_SHIFT )
				ac.fVirt |= FSHIFT;
			accel.push_back( ac );
		}
	}

	if( ! accel.empty() )
		return ::CreateAcceleratorTable( &accel[ 0 ], static_cast<int>( accel.size() ) );
	return NULL;
}

//-----------------------------------------------------------------------------------------------

LPCTSTR ToolWnd::GetCommandName( int cmd )
{
	switch( cmd )
	{
		case ID_FF_LASTDIR:     return L"ff_LastFolder";
		case ID_FF_SHOWALL:     return L"ff_ViewAllFiles";
		case ID_FF_FOCUSPATH:   return L"ff_FocusPathEdit";
		case ID_FF_GLOBALHIST:  return L"ff_MenuFolderHistory";
		case ID_FF_OPENDIRS:    return L"ff_MenuOpenFolders";
		case ID_FF_FAVORITES:   return L"ff_MenuFavorites";
		case ID_FF_CONFIG:      return L"ff_MenuConfig";
	}
	return L"";
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::NavigateToFolder( PCIDLIST_ABSOLUTE folder )
{
	PCIDLIST_ABSOLUTE folderToNavigate = NULL;
	SpITEMIDLIST newFolder;
	tstring existingPath;
	WCHAR path[ MAX_PATH ] = L"";

	if( ::SHGetPathFromIDList( folder, path ) )
	{
		// If file system folder does not exist anymore, get first existing ancestor folder.
		existingPath = GetExistingDirOrParent( path );
		if( ! existingPath.empty() && SUCCEEDED( GetPidlFromPath( &newFolder, existingPath.c_str() ) ) )
			folderToNavigate = newFolder.get();
	}
	else
	{
		folderToNavigate = folder;	
	}

	if( folderToNavigate )
	{
		if( ! existingPath.empty() )
			::SetDlgItemText( m_hWnd, ID_FF_PATH, existingPath.c_str() );

		m_fileDlgHook->SetFolder( folder );
	}	
}

//----------------------------------------------------------------------------------------------------

HMENU ToolWnd::CreateFolderMenu( const std::vector<tstring> &folderList, HMENU hMenu )
{
    if( ! hMenu ) hMenu = CreatePopupMenu();
	if( hMenu == NULL ) 
		return NULL;

	for( size_t i = 0; i < folderList.size(); i++ )
	{
		if( folderList[ i ] == L"-" )
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		else
	        ::AppendMenu( hMenu, MF_STRING, i + 1, folderList[i].c_str() );
	}
	return hMenu;
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::FavMenu_Create( HMENU hMenu, const PluginManager::FavoriteItems& favs, size_t& iItem )
{
	while( iItem < favs.size() )
	{
		const PluginManager::FavoriteItem& fav = favs[ iItem ];
		++iItem;
		
		switch( fav.type )
		{
			case ffplug::FavMenuItem::T_SUBMENU_END:
			{
				// return from recursion
				return;
			}

			case ffplug::FavMenuItem::T_SUBMENU:
			{
				// insert submenu recursively

				HMENU hSubMenu = ::CreatePopupMenu();
				::AppendMenu( hMenu, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>( hSubMenu ),
				              fav.displayName.c_str() );

				FavMenu_Create( hSubMenu, favs, iItem );

				break;
			}

			case ffplug::FavMenuItem::T_SEPARATOR:
			{
				::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
				break;
			}

			case ffplug::FavMenuItem::T_FOLDER_FAVORITE:
			{
				::AppendMenu( hMenu, MF_STRING, iItem, fav.displayName.c_str() );
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------

int ToolWnd::FavMenu_Display( int x, int y, const PluginManager::FavoriteItems& favs )
{
    HMENU hMenu = ::CreatePopupMenu();
	size_t iItem = 0;
	FavMenu_Create( hMenu, favs, iItem );

	if( ! favs.empty() )
		::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );

	::AppendMenu( hMenu, MF_STRING, 1000, L"&Add current folder" );
	::AppendMenu( hMenu, MF_STRING, 1001, L"&Configure..." );

	int id = ::TrackPopupMenu( hMenu, TPM_RETURNCMD | TPM_NONOTIFY, x, y, 0, m_hWnd, NULL );

	::DestroyMenu( hMenu );

    return id;	
}

//-------------------------------------------------------------------------------------------------

void ToolWnd::DisplayMenu_Favorites()
{
	//--- get and show menu

	tstring pluginName = g_profile.GetString( L"FileManager", L"FavoritesSrc.PluginName" );
	tstring programId = g_profile.GetString( L"FileManager", L"FavoritesSrc.ProgramId" );

	PluginManager::FavoriteItems favMenu;
	if( ! m_pluginMgr->GetFavMenu( pluginName.c_str(), programId.c_str(), &favMenu ) )
	{
		WCHAR msg[ 200 ];
		swprintf_s( msg, L"The favorites menu could not be read.\n\n"
			L"Plugin: '%s'\nProgramId: '%s'\nError code: %d", 
			pluginName.c_str(), programId.c_str(), ::GetLastError() );
		::MessageBox( m_hFileDialog, msg, L"FlashFolder", MB_ICONERROR );
		return;
	}

	HWND hTb = GetDlgItem( ID_FF_TOOLBAR );
	Rect rc;
	::SendMessage( hTb, TB_GETRECT, ID_FF_FAVORITES, reinterpret_cast<LPARAM>( &rc ) );
	ClientToScreenRect( hTb, &rc );

	int id = FavMenu_Display( rc.left, rc.bottom, favMenu );

	// SetForegroundWindow is convenient for the user and can be required
	// to manipulate MSO file dialogs (since input is simulated to navigate to new folder).
	SetForegroundWindow( m_hFileDialog );

	CComPtr< IShellFolder > desktopFolder;
	::SHGetDesktopFolder( &desktopFolder );

	//--- execute selected command

	if( id == 1000 )
	{
		// Add folder to favorites.

		if( SpITEMIDLIST folder = m_fileDlgHook->GetFolder() )
		{
			// TODO BEGIN: show dialog to get menu item name from user
			
			WCHAR displayName[ MAX_PATH ] = L"";
			CComPtr< IShellFolder > parentFolder;
			LPCITEMIDLIST lastPidl = NULL;
			if( SUCCEEDED( ::SHBindToParent( folder.get(), IID_IShellFolder, (void**) &parentFolder, &lastPidl ) ) )
			{
				STRRET str = { 0 };
				if( SUCCEEDED( parentFolder->GetDisplayNameOf( lastPidl, SHGDN_NORMAL, &str ) ) )
					StrRetToBuf( &str, lastPidl, displayName, _countof( displayName ) );
			}

			// TODO END

			if( wcslen( displayName ) )
				FavMenu_AddFolder( displayName, folder.get() );
		}
	}
	else if( id == 1001 )
	{
		// Start favorites editor.

//		FavMenu_StartEditor( hFileDialog );
	}
	else if( id > 0 )
	{
		// Navigate to new folder.

		NavigateToFolder( favMenu[ id - 1 ].folder.get() );
	}
}

//-----------------------------------------------------------------------------------------------
/*
void FavMenu_StartEditor( HWND hWndParent )
{
	TCHAR path[ 4096 ] = L"";
	GetConfigProcessPath( path, _countof( path ) );

	TCHAR params[ 256 ] = L"";
	StringCbPrintf( params, sizeof(params),	L"%d --fav", hWndParent ); 

	::ShellExecute( hWndParent, L"open", path, params, NULL, SW_SHOW );
}
*/
//-----------------------------------------------------------------------------------------------

void ToolWnd::FavMenu_AddFolder( LPCWSTR displayName, PCIDLIST_ABSOLUTE folder )
{
	tstring pluginName = g_profile.GetString( L"FileManager", L"FavoritesSrc.PluginName" );
	tstring programId = g_profile.GetString( L"FileManager", L"FavoritesSrc.ProgramId" );

	if( ! m_pluginMgr->AddFavMenuItem( pluginName.c_str(), programId.c_str(), displayName, folder ) )
	{
		WCHAR msg[ 200 ];
		swprintf_s( msg, L"The folder could not be added to the favorites menu.\n\n"
			L"Plugin: '%s'\nProgramId: '%s'\nError code: %d", 
			pluginName.c_str(), programId.c_str(), ::GetLastError() );
		::MessageBox( m_hWnd, msg, L"FlashFolder", MB_ICONERROR );
		return;
	}
}

//-----------------------------------------------------------------------------------------

int ToolWnd::DisplayFolderMenu( HMENU hMenu, int buttonID )
{
	HWND hTb = GetDlgItem( ID_FF_TOOLBAR );
	Rect rc;
	SendMessage(hTb, TB_GETRECT, buttonID, (LPARAM) &rc);
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc ) ); 
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc.right ) ); 

	int id = TrackPopupMenu( hMenu, 
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
		rc.left, rc.bottom, 0, m_hWnd, NULL);

	if( id > 0 )
	{
        // get directory path from menu item text
        TCHAR path[MAX_PATH+1] = L"";
        MENUITEMINFO mi = { sizeof(mi) };
        mi.fMask = MIIM_STRING;
        mi.dwTypeData = path;
        mi.cch = MAX_PATH;
        ::GetMenuItemInfo( hMenu, id, FALSE, &mi );

		// strip additional text to get the path only
		LPCTSTR pPath = mi.dwTypeData;
		if( pPath[ 0 ] == '[' )
			pPath = _tcsstr( pPath, L"] " ) + 2;

		if( IsFilePath( pPath ) )
        {
			SpITEMIDLIST folder;
			if( SUCCEEDED( GetPidlFromPath( &folder, pPath ) ) )
				NavigateToFolder( folder.get() );
        }
	}

	return id;
}

//-----------------------------------------------------------------------------------------------

void ToolWnd::DisplayMenu_OpenDirs()
{
	HMENU hMenu = ::CreatePopupMenu();

	//--- add current folders from supported file managers

	PluginManager::CurrentFolders folders = m_pluginMgr->GetCurrentFolders();

	UINT menuId = 2000;
	bool hasItems = false;
	foreach( const PluginManager::CurrentFolder& folder, folders )
	{
		WCHAR pidlPath[ MAX_PATH ] = L"";
		if( ::SHGetPathFromIDList( folder.folder.get(), pidlPath ) )
		{
			wstring s = wstring( L"[" ) + folder.programShortName + L"] " + pidlPath;
			::AppendMenu( hMenu, MF_STRING, menuId++, s.c_str() );
			hasItems = true;
		}
	}

	if( hasItems )
		::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );

	//--- add application dir

	TCHAR s[ MAX_PATH + 20 ] = L"[Application] ";
	WCHAR appDir[ 1024 ] = L"";
	GetAppDir( NULL, appDir, _countof( appDir ) );
	wcscat_s( s, appDir );
	::AppendMenu( hMenu, MF_STRING, 1, s );
    
    //--- show menu
    
	DisplayFolderMenu( hMenu, ID_FF_OPENDIRS );
    
	DestroyMenu( hMenu );

	SetForegroundWindow( m_hFileDialog );
}

//----------------------------------------------------------------------------------------

void ToolWnd::GotoLastDir()
{
	SetForegroundWindow( m_hFileDialog );

	HistoryLst history;
	if( ! history.LoadFromProfile( g_profile, L"GlobalFolderHistory" ) )
		return;

	SetDlgItemText( ID_FF_PATH, history.GetList().front().c_str() );

	SpITEMIDLIST folder;
	if( SUCCEEDED( GetPidlFromPath( &folder, history.GetList().front().c_str() ) ) )
		m_fileDlgHook->SetFolder( folder.get() );
}

//-----------------------------------------------------------------------------------------

void ToolWnd::DisplayMenu_GlobalHist()
{
	HistoryLst history;
	if( history.LoadFromProfile( g_profile, L"GlobalFolderHistory" ) )
	{
		HMENU hMenu = CreateFolderMenu( history.GetList() );
		DisplayFolderMenu( hMenu, ID_FF_GLOBALHIST );
		DestroyMenu( hMenu );
	}
		
	SetForegroundWindow( m_hFileDialog );
}

//-----------------------------------------------------------------------------------------------

void ToolWnd::DisplayMenu_Config()
{
	// get menu position
	HWND hTb = ::GetDlgItem( m_hWnd, ID_FF_TOOLBAR );
	Rect rc;
	::SendMessage( hTb, TB_GETRECT, ID_FF_CONFIG, (LPARAM) &rc );
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc ) ); 
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc.right ) ); 

	HMENU hMenu = ::CreatePopupMenu();
	::AppendMenu( hMenu, MF_STRING, 1, L"Options..." );
	::AppendMenu( hMenu, MF_STRING, 2, L"Check for updates" );
	::AppendMenu( hMenu, MF_STRING, 3, L"About FlashFolder..." );

	int id = TrackPopupMenu( hMenu, 
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
		rc.left, rc.bottom, 0, m_hWnd, NULL);

	TCHAR path[ 4096 ] = L"";
	GetConfigProcessPath( _AtlBaseModule.GetModuleInstance(), path, _countof( path ) );
	
	TCHAR params[ 256 ] = L"";
	StringCbPrintf( params, sizeof(params), L"%d", m_hWnd ); 
	
	if( id == 1 )
	{
		::ShellExecute( m_hFileDialog, L"open", path, params, NULL, SW_SHOW );
	}
	else if( id == 2 )
	{
		StringCbCat( params, sizeof(params), L" --updatecheck" );
		::ShellExecute( m_hFileDialog, L"open", path, params, NULL, SW_SHOW );	
	}
	else if( id == 3 )
	{
		StringCbCat( params, sizeof(params), L" --about" );
		::ShellExecute( m_hFileDialog, L"open", path, params, NULL, SW_SHOW );
	}

	::SetForegroundWindow( m_hFileDialog );
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogInitDone()
{
	UpdatePathEdit();

	m_isToolWndVisible = true;
	::ShowWindow( m_hWnd, SW_SHOWNA );
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogFolderChange()
{
	UpdatePathEdit();
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogResize()
{
	AdjustToolWindowPos();
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogEnable( bool bEnable )
{
	::EnableWindow( m_hWnd, bEnable );
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogShow( bool bShow )
{
	::ShowWindow( m_hWnd, bShow ? SW_SHOW : SW_HIDE );
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogActivate( WPARAM wParam, LPARAM lParam )
{ /*unused*/ }

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogSetTimer( DWORD interval )
{
	::SetTimer( m_hWnd, 1, interval, NULL );
}

//----------------------------------------------------------------------------------------------------

void ToolWnd::OnFileDialogDestroy( bool isOkBtnPressed )
{
	// destroy tool window + class

	::DestroyWindow( m_hWnd );
	m_hWnd = NULL;
	m_hFileDialog = NULL;

	// destroy additional resources

	if( m_hToolbarImages )
	{
		::ImageList_Destroy( m_hToolbarImages );
		m_hToolbarImages = NULL;
	}
	
	if( m_hStdFont )
	{
		::DeleteObject( m_hStdFont );
		m_hStdFont = NULL;
	}
}
