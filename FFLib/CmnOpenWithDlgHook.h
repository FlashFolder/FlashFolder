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
#ifndef CMNOPENWITHDLGHOOK_H__INCLUDED
#define CMNOPENWITHDLGHOOK_H__INCLUDED

#if _MSC_VER > 1000
	#pragma once
#endif

#include "filedlg_base.h"

//-----------------------------------------------------------------------------------------
// class CmnOpenWithDlgHook
//
// Specific code for hooking of common "open with" dialogs.
//-----------------------------------------------------------------------------------------

class CmnOpenWithDlgHook : public FileDlgHook_base
{
public:
	CmnOpenWithDlgHook() : 
		m_hwndFileDlg( 0 ),  
        m_initDone( false ), m_isWindowActive( false ), m_hwndSizeGrip( NULL ) {}

	// overridings of FileDlgHook_base
	virtual bool Init( HWND hwndFileDlg, HWND hWndTool );
	virtual bool SetFolder( LPCTSTR path );
	virtual bool GetFolder( LPTSTR folderPath );
	virtual bool SetFilter( LPCTSTR filter );

private:
	static LRESULT CALLBACK HookWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ResizeDialog();

	HWND m_hwndFileDlg;
	WNDPROC m_oldWndProc;
	bool m_isWindowActive;
	bool m_initDone;

	HWND m_hwndSizeGrip;
	RECT m_rcInitial;

	// options read from INI file specified in Init()

	int m_minFileDialogWidth;			    // prefered minimum size of file dialog
	int m_minFileDialogHeight;
	int m_centerFileDialog;				// true if file dialog should be centered
	bool m_bResizeNonResizableDlgs;			// true if those non-resizable dialogs should
};

//-----------------------------------------------------------------------------------------

#endif //CMNOPENWITHDLGHOOK_H__INCLUDED