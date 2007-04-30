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
#ifndef MSOFILEDLGHOOK_H__INCLUDED
#define MSOFILEDLGHOOK_H__INCLUDED

#if _MSC_VER > 1000
	#pragma once
#endif

#include "filedlg_base.h"

//-----------------------------------------------------------------------------------------
// class MsoFileDlgHook
//
// Specific code for hooking of MS Office file dialogs.
//-----------------------------------------------------------------------------------------

class MsoFileDlgHook : public FileDlgHook_base
{
public:
	MsoFileDlgHook() :
		m_hwndFileDlg( 0 ), m_fileDialogCanceled( false ),
        m_initDone( false ), m_isWindowActive( false ) {}


	// overridings of FileDlgHook_base
	virtual bool Init( HWND hwndFileDlg, HWND hWndTool );
	virtual bool SetFolder( LPCTSTR path );
	virtual bool GetFolder( LPTSTR folderPath );
	virtual bool SetFilter( LPCTSTR filter );
	virtual void OnTimer();

private:
	static LRESULT CALLBACK HookWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ResizeFileDialog();
	bool EnterFilenameEditText( LPCTSTR text );

	HWND m_hwndFileDlg, m_hwndTool;
	WNDPROC m_oldWndProc;
	bool m_isWindowActive;
	bool m_fileDialogCanceled;
	bool m_initDone;
	TCHAR m_currentDir[ MAX_PATH + 1 ];
	
	int m_centerFileDialog;
	int m_minFileDialogWidth;			    // prefered minimum size of file dialog
	int m_minFileDialogHeight;
};

//-----------------------------------------------------------------------------------------

#endif //MSOFILEDLGHOOK_H__INCLUDED