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
#ifndef FILEDLG_BASE_H__INCLUDED
#define FILEDLG_BASE_H__INCLUDED

#if _MSC_VER > 1000
	#pragma once
#endif

#include <windows.h>

//-----------------------------------------------------------------------------------
// Abstract base class for file dialog hooks.

class FileDlgHookBase
{
public:
	virtual ~FileDlgHookBase() {}
	virtual bool Init( HWND hWndFileDlg, HWND hWndTool ) = 0;
	virtual bool SetFolder( PCIDLIST_ABSOLUTE folder ) = 0;
	virtual SpITEMIDLIST GetFolder() const = 0;
	virtual bool SetFilter( LPCTSTR filter ) = 0;
	virtual void OnTimer() {}
};

//-----------------------------------------------------------------------------------

#endif //FILEDLG_BASE_H__INCLUDED
