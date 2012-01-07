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
#pragma once

#include <commonUtils\Profile.h>
#include <commonUtils\GdiUtils.h>

//-----------------------------------------------------------------------------------------
/**
 * Get default program settings.
**/
void GetProfileDefaults( Profile* pProfile );

//-----------------------------------------------------------------------------------------------

// If the absolute value of any dialog metrics read from the profile is greater than this constant,
// it means the value is stored in dialog units rather than pixels. In this case MapDialogRect() 
// must be used to calculate the pixel values.
const int DIALOG_UNITS_FLAG = 0x40000000;

inline int MapProfileX( HWND hDlg, int x )
{
	if( x > DIALOG_UNITS_FLAG )
		return MapDialogX( hDlg, x - DIALOG_UNITS_FLAG );
	return x;
}

inline int MapProfileY( HWND hDlg, int y )
{
	if( y > DIALOG_UNITS_FLAG )
		return MapDialogY( hDlg, y - DIALOG_UNITS_FLAG );
	return y;
}