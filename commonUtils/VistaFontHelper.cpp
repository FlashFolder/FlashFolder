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
#include "StdAfx.h"
#include "VistaFontHelper.h"
#include "GdiUtils.h"

//------------------------------------------------------------------------------------------------------

const DLGTEMPLATE* CFontOccManager::PreCreateDialog(_AFX_OCC_DIALOG_INFO* pOccDialogInfo,
	const DLGTEMPLATE* pOrigTemplate)
{
	const DLGTEMPLATE *lpNewTemplate = COccManager::PreCreateDialog( pOccDialogInfo, pOrigTemplate );

	if( (BYTE) GetVersion() >= 6 && ! pOccDialogInfo->m_pNewTemplate ) 
	{
		CDialogTemplate temp( lpNewTemplate );
		LOGFONT lf; WORD wDefSize = 0;
		GetStandardOsFont( &lf, &wDefSize );
		temp.SetFont(lf.lfFaceName, wDefSize);

		pOccDialogInfo->m_pNewTemplate = (DLGTEMPLATE*) temp.Detach();
		return pOccDialogInfo->m_pNewTemplate;
	}
	else
		return lpNewTemplate;
}

//----------------------------------------------------------------------------------------------------

void SetStandardOsFontInDlgResource( LPDLGTEMPLATE pTemplate )
{
	LOGFONT lf; WORD fontSize = 0;
	GetStandardOsFont( &lf, &fontSize );

    CModifyDialogTemplate dlgTemplate;
    dlgTemplate.Attach( pTemplate );
    dlgTemplate.SetFont( lf.lfFaceName, fontSize );
    dlgTemplate.Detach();
}