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
/** \file Main file to compile "TC_FlashFolder.exe".\n
 *     The EXE is used to give the user access to the global folder history 
 *     from within Total Commander.
 *     When the EXE ist started, it displays the global folder history menu at
 *     the current mouse position.
 *     Command line parameter "/rightpanel" can be used to open the selected folder
 *     in the right Total Commander panel else it is opened in the left one.
 *
 */
#include "stdafx.h"
#include "..\fflib\ff_utils.h"
#include "..\fflib\HistoryLst.h"

//--------------------------------------------------------------------------------------------

HWND CreateMessageWnd(HINSTANCE hInst)
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = &DefWindowProc;
	wc.hInstance = hInst;
	wc.lpszClassName = _T("TC_FlashFolderMsgWnd");
	if (RegisterClassEx(&wc) == 0)
		return NULL;

	// the window must have the visible flag, else the popup menu will not disappear
	//   properly when clicking outside the menu
	//   'cause the window has width/height of 0, it won't be really visible either
	// we use WS_EX_TOOLWINDOW to hide the window from the taskbar
	return CreateWindowEx(WS_EX_TOOLWINDOW, _T("TC_FlashFolderMsgWnd"), NULL, 
		WS_VISIBLE | WS_POPUP, 0, 0, 0, 0, 
		NULL, NULL, hInst, NULL); 
}

//--------------------------------------------------------------------------------------------

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPCTSTR     lpCmdLine,
                     int       nCmdShow )
{
	// create a global event object that will be used to check whether TC_FlashFolder
	// is already running
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, _T("TC_FlashFolder") );
	if (hEvent == NULL)	
	{
		MessageBox(0, _T("Could not create event object."), _T("TC FlashFolder"), 0);
		return -1;
	}	

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;	// TC_FlashFolder already running --> quit

	bool bUseRightPanel = (_tcsstr(lpCmdLine, _T("/rightpanel") ) != NULL);

	TCHAR iniPath[MAX_PATH+1];
	GetAppDir(NULL, iniPath);
	_tcscat(iniPath, _T("FlashFolderGlobal.ini"));
	
	CMTHistoryLst folderHistory;
	folderHistory.LoadFromIniFile(iniPath, _T("GlobalFolderHistory") );
	if (folderHistory.GetCount() > 0)
	{
		POINT mp;
		GetCursorPos(&mp);

		HMENU hMenu = CreatePopupMenu();

		for (int i = 0; i < folderHistory.GetCount(); i++)
		{
			AppendMenu(hMenu, MF_STRING,  i + 1, folderHistory[i].c_str());
		}

		//TrackPopupMenu needs a parent window, so make it happy
		HWND wnd = CreateMessageWnd(hInstance);

		int id = TrackPopupMenu(hMenu, 
			TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, 
			mp.x, mp.y, 0, wnd, NULL);
		if (id > 0 && id < folderHistory.GetCount() + 2)
		{
			TCHAR totalCmdDir[MAX_PATH+1];
			TCHAR totalCmdPath[MAX_PATH+1];
			if (GetProgFolderPathFromRegistry( _T("Totalcmd"), totalCmdDir))
			{
				_tcscpy(totalCmdPath, totalCmdDir);
				_tcscat(totalCmdPath, _T("totalcmd.exe") );
				TCHAR param[MAX_PATH+1] = _T("/O "); 
				if (bUseRightPanel)	
					_tcscat(param, _T("/R=\"") );
				else
					_tcscat(param, _T("\"") );
				_tcscat(param, folderHistory[id-1].c_str());
				_tcscat(param, _T("\"") );
				ShellExecute(NULL, _T("open"), totalCmdPath, param, totalCmdDir, SW_SHOWNORMAL); 
			}
			else
				MessageBox(0, _T("Sorry, could not retrieve path to Total Commander program folder from the registry."), _T("TC FlashFolder"), 0);	
		}

		DestroyMenu(hMenu);

	}

	return 0;
}



