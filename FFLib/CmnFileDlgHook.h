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
#ifndef CMNFILEDLGHOOK_H__INCLUDED
#define CMNFILEDLGHOOK_H__INCLUDED

#if _MSC_VER > 1000
	#pragma once
#endif

#include "filedlg_base.h"

//-----------------------------------------------------------------------------------------
// class CmnFileDlgHook
//
// Specific code for hooking of common file dialogs.
//-----------------------------------------------------------------------------------------

class CmnFileDlgHook : public FileDlgHook_base
{
public:
	CmnFileDlgHook() { Reset();	}

	// overridings of FileDlgHook_base

	virtual bool Init( HWND hwndFileDlg, HWND hWndTool );
	virtual void Uninstall();
	
	virtual bool SetFolder( LPCTSTR path );
	virtual bool GetFolder( LPTSTR folderPath );
	virtual bool SetFilter( LPCTSTR filter );

private:
	void Reset()
	{
		m_hwndFileDlg = NULL;
		m_shellWnd = NULL; 
		m_pShellBrowser = NULL;
        m_fileDlgShown = false;
		m_fileDialogCanceled = false;
        m_isWindowActive = false;
        m_shellViewMode = FVM_AUTO;
        m_shellViewImageSize = -1;
        m_folderPath = L"";
	}
	
	static LRESULT CALLBACK HookWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
		UINT_PTR subclassId, DWORD_PTR refData );
	static LRESULT CALLBACK HookShellWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 	
		UINT_PTR subclassId, DWORD_PTR refData );
	void ResizeFileDialog();
	void ResizeNonResizableFileDialog( int x, int y, int newWidth, int newHeight );
	void InitShellWnd();

	HWND m_hwndFileDlg, m_hwndTool;
	bool m_isWindowActive;
	bool m_fileDialogCanceled;
	bool m_fileDlgShown;
	FOLDERVIEWMODE m_shellViewMode;
	int m_shellViewImageSize;
	HWND m_shellWnd;
	IShellBrowser *m_pShellBrowser;
	tstring m_folderPath;

	// options read from INI file specified in Init()

	int m_minFileDialogWidth;			    // prefered minimum size of file dialog
	int m_minFileDialogHeight;
	int m_centerFileDialog;				// true if file dialog should be centered
	bool m_bResizeNonResizableDlgs;			// true if those non-resizable dialogs should
											// be resized by FlashFolder
	int m_folderComboHeight;				// prefered heights of the combo boxes of 
	int m_filetypesComboHeight;				//   the file dialog
};

//-----------------------------------------------------------------------------------------

#endif //CMNFILEDLGHOOK_H__INCLUDED