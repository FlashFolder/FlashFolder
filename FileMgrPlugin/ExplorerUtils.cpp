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
 */
#include "stdafx.h"

//-----------------------------------------------------------------------------------------

HRESULT GetCurrentExplorerFolders( std::vector< PIDLIST_ABSOLUTE >* folders )
{
	folders->clear();

	HRESULT hr = S_OK;

	CComPtr< IShellWindows > shellWnds;
	hr = shellWnds.CoCreateInstance( CLSID_ShellWindows );
	if( FAILED( hr ) )
		return hr;

	long count = 0;
	hr = shellWnds->get_Count( &count );
	if( FAILED( hr ) )
		return hr;

	for( int i = 0; i < count; ++i )
	{
		CComVariant vi( i );
		CComPtr< IDispatch > disp;
		hr = shellWnds->Item( vi, &disp  );
		if( FAILED( hr ) )
			return hr;

		if( ! disp )
			// Skip - this shell window was registered with a NULL IDispatch
			continue;

		CComQIPtr< IWebBrowserApp > app( disp );
		if( ! app )
			continue;

		CComQIPtr< IServiceProvider > psp( app );
		if( ! psp )
			continue;

		CComPtr< IShellBrowser > browser;
		if( FAILED( psp->QueryService( SID_STopLevelBrowser, &browser ) ) )
			continue;

		CComPtr< IShellView > shellView;
		if( FAILED( browser->QueryActiveShellView( &shellView ) ) )
			continue;
			
		CComQIPtr< IFolderView > folderView( shellView );
		if( ! folderView )
			continue;
			 
		CComPtr< IPersistFolder2 > persistFolder;
		if( FAILED( folderView->GetFolder( IID_IPersistFolder2, (void**) &persistFolder ) ) )
			continue;
			
		LPITEMIDLIST pidl = NULL;
		if( SUCCEEDED( persistFolder->GetCurFolder( &pidl ) ) ) 
			folders->push_back( pidl );
	}
	
	return S_OK;	
}