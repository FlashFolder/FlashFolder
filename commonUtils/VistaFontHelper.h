/* This file is part of FlashFolder. 
 * Copyright (C) 2009 zett42 ( zett42 at users.sourceforge.net ) 
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

#include <afxpriv.h>
#if _MSC_VER < 1300
#include <..\src\occimpl.h> 
#else
#include "afxocc.h"
#endif

//-------------------------------------------------------------------------------------------
/// Get the standard OS font and size used in dialogs.

void GetStandardOsFont( LOGFONT *pLF, WORD* pDefSize );


//-------------------------------------------------------------------------------------------
/// Class to automatically set the standard OS font for all dialogs. Especially important
/// for Vista, where the standard font is no longer "Tahoma", but typically "Segoe UI" with a 
/// different font size.
///
/// To use this class, put the following line in InitInstance() method of your application:
/// 	AfxEnableControlContainer( new CFontOccManager() );
///
/// Note: This only sets the font for normal dialogs (i. e. CDialog-derived).

class CFontOccManager : public COccManager
{
public:
    CFontOccManager() { }

	virtual const DLGTEMPLATE* PreCreateDialog(_AFX_OCC_DIALOG_INFO* pOccDialogInfo,
	      const DLGTEMPLATE* pOrigTemplate);
};

//----------------------------------------------------------------------------------------------------
/// Class that can be used to modify existing dialog templates in memory 
/// (e. g. to set the font for property sheets).
///
/// Note: When you have finished modifying the template, call Detach().

class CModifyDialogTemplate : public CDialogTemplate
{
public:
    void Attach( LPDLGTEMPLATE pTemplate )
    {
		m_hTemplate      = ::GlobalHandle( pTemplate );
		m_dwTemplateSize = GetTemplateSize( pTemplate );
    }
};

//----------------------------------------------------------------------------------------------------
/// Modify a dialog resource in memory to use the standard OS font and size.
/// Use it for instance in CPropertySheet::BuildPropPageArray().

void SetStandardOsFontInDlgResource( LPDLGTEMPLATE pTemplate );