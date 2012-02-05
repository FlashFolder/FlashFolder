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
 */
#ifndef CmnFolderDlgHook_H__INCLUDED
#define CmnFolderDlgHook_H__INCLUDED

#if _MSC_VER > 1000
	#pragma once
#endif

#include "HookBase.h"

//-----------------------------------------------------------------------------------------
// class CmnFolderDlgHook
//
// Specific code for hooking of common file dialogs.
//-----------------------------------------------------------------------------------------

class CmnFolderDlgHook : public FileDlgHookBase
{
public:
	CmnFolderDlgHook() : 
		m_hwndFileDlg( 0 ), m_fileDialogCanceled( false ),
        m_initDone( false ), m_isWindowActive( false ),
		m_oldParentWndProc( NULL )
	{}

	// overridings of FileDlgHookBase
	virtual bool Init( HWND hwndFileDlg, HWND hWndTool );
	virtual bool SetFolder( PCIDLIST_ABSOLUTE folder );
	virtual SpITEMIDLIST GetFolder() const;
	virtual bool SetFilter( LPCTSTR filter );

private:
	static LRESULT CALLBACK HookWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK TreeParentWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void ResizeFileDialog();
	bool UpdateCurrentPath( HWND hwndTree, HTREEITEM hItem );

	HWND m_hwndFileDlg, m_hwndTool;
	WNDPROC m_oldWndProc, m_oldParentWndProc;
	bool m_isWindowActive;
	bool m_fileDialogCanceled;
	bool m_initDone;
	SpITEMIDLIST m_currentFolder;

	// options read from INI file specified in Init()

	int m_minFileDialogWidth;			    // prefered minimum size of file dialog
	int m_minFileDialogHeight;
	int m_centerFileDialog;				// true if file dialog should be centered
};

//-----------------------------------------------------------------------------------------

#endif //CmnFolderDlgHook_H__INCLUDED