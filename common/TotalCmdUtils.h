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
#pragma once

#include <windows.h>
#include <tchar.h>

//-------------------------------------------------------------------------------------------------
/// \brief Class to get information from a running Total Commander instance.

class CTotalCmdUtils
{
public:
    CTotalCmdUtils( HWND hwndTotalCmd = NULL ) { SetTCmdWnd( hwndTotalCmd ); }
    
    void SetTCmdWnd( HWND hwndTotalCmd ) 
    { 
        m_hwnd = hwndTotalCmd; 
        m_hwndLeft = m_hwndRight = m_hwndActive = NULL; 
    }

    HWND GetTCmdWnd() const { return m_hwnd; }

    static HWND FindTopTCmdWnd();

    bool GetDirs( LPTSTR pLeftDir, unsigned leftDirLen, 
                  LPTSTR pRightDir, unsigned rightDirLen );

private:
    struct CFindSubWindowsData
    {
        CTotalCmdUtils* m_thisptr;
        LPTSTR m_pLeftDir;
        LPTSTR m_pRightDir;
        unsigned m_leftDirLen, m_rightDirLen;

        CFindSubWindowsData() { memset( this, 0, sizeof(*this) ); }
    };

private:
    void FindSubWindows();
    static BOOL CALLBACK FindTopWnd_Proc( HWND hwnd, LPARAM lParam );
    static BOOL CALLBACK FindSubWindows_Proc( HWND hwnd, LPARAM lParam );
   
private:
    HWND m_hwnd, m_hwndLeft, m_hwndRight, m_hwndActive;
};

//-------------------------------------------------------------------------------------------------

/// \brief Get install-directory and path of .INI-file of Total Commander
bool GetTotalCmdLocation( tstring* pInstallDir = NULL, tstring* pIniPath = NULL );

