
#include "StdAfx.h"
#include "FolderMenus.h"
#include "utils.h"
#include "resource.h"

//-----------------------------------------------------------------------------------------

HMENU CreateFolderMenu( const std::vector<tstring> &folderList, HMENU hMenu )
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

void FavMenu_Create( HMENU hMenu, const PluginManager::FavMenuItems& favs, size_t& iItem )
{
	while( iItem < favs.size() )
	{
		const FavMenuItem& fav = favs[ iItem ];
		++iItem;
		
		switch( fav.type )
		{
			case FavMenuItem::T_SUBMENU_END:
			{
				// return from recursion
				return;
			}

			case FavMenuItem::T_SUBMENU:
			{
				// insert submenu recursively

				HMENU hSubMenu = ::CreatePopupMenu();
				::AppendMenu( hMenu, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>( hSubMenu ),
				              fav.displayName );

				FavMenu_Create( hSubMenu, favs, iItem );

				break;
			}

			case FavMenuItem::T_SEPARATOR:
			{
				::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
				break;
			}

			case FavMenuItem::T_FOLDER_FAVORITE:
			{
				::AppendMenu( hMenu, MF_STRING, iItem, fav.displayName );
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------

int FavMenu_Display( HWND hWndParent, int x, int y, const PluginManager::FavMenuItems& favs )
{
    HMENU hMenu = ::CreatePopupMenu();
	size_t iItem = 0;
	FavMenu_Create( hMenu, favs, iItem );

	if( ! favs.empty() )
		::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );

	::AppendMenu( hMenu, MF_STRING, 1000, L"&Add current folder" );
	::AppendMenu( hMenu, MF_STRING, 1001, L"&Configure..." );

	int id = ::TrackPopupMenu( hMenu, TPM_RETURNCMD | TPM_NONOTIFY, 
		x, y, 0, hWndParent, NULL );

	::DestroyMenu( hMenu );

    return id;	
}

//-------------------------------------------------------------------------------------------------

void FavMenu_DisplayForFileDialog( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook,
	const PluginManager& pluginMgr, const Profile& profile )
{
	//--- get and show menu

	tstring pluginName = profile.GetString( L"FileManager", L"FavoritesSrc.PluginName" );
	tstring programId = profile.GetString( L"FileManager", L"FavoritesSrc.ProgramId" );

	PluginManager::FavMenuItems favMenu;
	if( ! pluginMgr.GetFavMenu( pluginName.c_str(), programId.c_str(), &favMenu ) )
	{
		WCHAR msg[ 200 ];
		swprintf_s( msg, L"The favorites menu could not be read.\n\n"
			L"Plugin: '%s'\nProgramId: '%s'\nError code: %d", 
			pluginName.c_str(), programId.c_str(), ::GetLastError() );
		::MessageBox( hFileDialog, msg, L"FlashFolder", MB_ICONERROR );
		return;
	}

	HWND hTb = GetDlgItem( hToolWnd, ID_FF_TOOLBAR );
	Rect rc;
	::SendMessage( hTb, TB_GETRECT, ID_FF_FAVORITES, reinterpret_cast<LPARAM>( &rc ) );
	ClientToScreenRect( hTb, &rc );

	int id = FavMenu_Display( hToolWnd, rc.left, rc.bottom, favMenu );

	//--- execute selected command

	if( id == 1000 )
	{
		TCHAR path[ MAX_PATH + 1 ];
		if( fileDlgHook->GetFolder( path ) )
			FavMenu_AddDir( hFileDialog, pluginMgr, profile, path );
	}
	else if( id == 1001 )
	{
//		FavMenu_StartEditor( hFileDialog );
	}
	else if( id > 0 )
	{
		//--- execute favorites menu item

		const FavMenuItem& fav = favMenu[ id - 1 ];

		tstring path = GetExistingDirOrParent( fav.folder );
		if( ! path.empty() )
			if( DirectoryExists( path.c_str() ) )
			{
				SetDlgItemText( hToolWnd, ID_FF_PATH, path.c_str() );
			
				SetForegroundWindow( hFileDialog );
				fileDlgHook->SetFolder( path.c_str() );
			}
	}		

	SetForegroundWindow( hFileDialog );
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

void FavMenu_AddDir( HWND hwndParent, const PluginManager& pluginMgr, const Profile& profile, LPCTSTR path )
{
	tstring pluginName = profile.GetString( L"FileManager", L"FavoritesSrc.PluginName" );
	tstring programId = profile.GetString( L"FileManager", L"FavoritesSrc.ProgramId" );

	if( ! pluginMgr.AddFavMenuItem( pluginName.c_str(), programId.c_str(), path, path ) )
	{
		WCHAR msg[ 200 ];
		swprintf_s( msg, L"The folder could not be added to the favorites menu.\n\n"
			L"Plugin: '%s'\nProgramId: '%s'\nError code: %d", 
			pluginName.c_str(), programId.c_str(), ::GetLastError() );
		::MessageBox( hwndParent, msg, L"FlashFolder", MB_ICONERROR );
		return;
	}
}

//-----------------------------------------------------------------------------------------

int DisplayFolderMenu( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook,
                       HMENU hMenu, int buttonID )
{
	HWND hTb = GetDlgItem(hToolWnd, ID_FF_TOOLBAR);
	Rect rc;
	SendMessage(hTb, TB_GETRECT, buttonID, (LPARAM) &rc);
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc ) ); 
	::ClientToScreen( hTb, reinterpret_cast<POINT*>( &rc.right ) ); 

	int id = TrackPopupMenu( hMenu, 
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
		rc.left, rc.bottom, 0, hToolWnd, NULL);

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
			tstring existingDir = GetExistingDirOrParent( pPath );
			if( ! existingDir.empty() )
			{ 
				SetDlgItemText( hToolWnd, ID_FF_PATH, existingDir.c_str() );
			
				SetForegroundWindow( hFileDialog );
				fileDlgHook->SetFolder( existingDir.c_str() );
			}
        }
	}

	return id;
}

//-----------------------------------------------------------------------------------------------

void DisplayMenu_OpenDirs( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook, 
                           const PluginManager& pluginMgr )
{
	HMENU hMenu = ::CreatePopupMenu();

	//--- add current folders from supported file managers

	PluginManager::CurrentFolders folders = pluginMgr.GetCurrentFolders();

	UINT menuId = 2000;
	foreach( const PluginManager::CurrentFolder& folder, folders )
	{
		wstring s = wstring( L"[" ) + folder.programShortName + L"] " + folder.folder;
		::AppendMenu( hMenu, MF_STRING, menuId++, s.c_str() );
	}

	if( ! folders.empty() )
		::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );

	//--- add application dir

	TCHAR s[ MAX_PATH + 20 ] = L"[Application] ";
	WCHAR appDir[ 1024 ] = L"";
	GetAppDir( NULL, appDir, _countof( appDir ) );
	wcscat_s( s, appDir );
	::AppendMenu( hMenu, MF_STRING, 1, s );
    
    //--- show menu
    
	DisplayFolderMenu( hFileDialog, hToolWnd, fileDlgHook, hMenu, ID_FF_OPENDIRS );
    
	DestroyMenu( hMenu );

	SetForegroundWindow( hFileDialog );
}

//----------------------------------------------------------------------------------------------------

void AddCurrentFolderToHistory( FileDlgHook_base* fileDlgHook, Profile* profile )
{
    WCHAR folderPath[ MAX_PATH ] = L"";
	if( ! fileDlgHook->GetFolder( folderPath ) )
		return;

	HistoryLst history;
	history.LoadFromProfile( *profile, L"GlobalFolderHistory" );
	history.SetMaxEntries( profile->GetInt( L"main", L"MaxGlobalHistoryEntries" ) );
	history.AddFolder( folderPath );
	history.SaveToProfile( *profile, L"GlobalFolderHistory" );
}

//----------------------------------------------------------------------------------------

void GotoLastDir( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook, const Profile& profile )
{
	SetForegroundWindow( hFileDialog );

	HistoryLst history;
	if( ! history.LoadFromProfile( profile, L"GlobalFolderHistory" ) )
		return;

	SetDlgItemText( hToolWnd, ID_FF_PATH, history.GetList().front().c_str() );

	wstring path = GetExistingDirOrParent( history.GetList().front().c_str() );
	if( ! path.empty() )
		fileDlgHook->SetFolder( path.c_str() );
}

//-----------------------------------------------------------------------------------------

void DisplayMenu_GlobalHist( HWND hFileDialog, HWND hToolWnd, FileDlgHook_base* fileDlgHook, const Profile& profile )
{
	HistoryLst history;
	if( history.LoadFromProfile( profile, L"GlobalFolderHistory" ) )
	{
		HMENU hMenu = CreateFolderMenu( history.GetList() );
		DisplayFolderMenu( hFileDialog, hToolWnd, fileDlgHook, hMenu, ID_FF_GLOBALHIST );
		DestroyMenu( hMenu );
	}
		
	SetForegroundWindow( hFileDialog );
}

//-----------------------------------------------------------------------------------------------

void DisplayMenu_Config( HINSTANCE hInstDll, HWND hFileDialog, HWND hToolWnd )
{
	// get menu position
	HWND hTb = ::GetDlgItem( hToolWnd, ID_FF_TOOLBAR );
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
		rc.left, rc.bottom, 0, hToolWnd, NULL);

	TCHAR path[ 4096 ] = L"";
	GetConfigProcessPath( hInstDll, path, _countof( path ) );
	
	TCHAR params[ 256 ] = L"";
	StringCbPrintf( params, sizeof(params), L"%d", hFileDialog ); 
	
	if( id == 1 )
	{
		::ShellExecute( hFileDialog, L"open", path, params, NULL, SW_SHOW );
	}
	else if( id == 2 )
	{
		StringCbCat( params, sizeof(params), L" --updatecheck" );
		::ShellExecute( hFileDialog, L"open", path, params, NULL, SW_SHOW );	
	}
	else if( id == 3 )
	{
		StringCbCat( params, sizeof(params), L" --about" );
		::ShellExecute( hFileDialog, L"open", path, params, NULL, SW_SHOW );
	}

	::SetForegroundWindow( hFileDialog );
}