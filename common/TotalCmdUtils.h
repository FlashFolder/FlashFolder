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
#include <commonUtils\tstring.h>

//-------------------------------------------------------------------------------------------------
/// \brief Class to get information from a running Total Commander instance.

class CTotalCmdUtils
{
public:
    CTotalCmdUtils( HWND hwndTotalCmd = NULL ) { SetTCmdWnd( hwndTotalCmd ); }
    void SetTCmdWnd( HWND hwndTotalCmd ); 
    
    HWND GetTCmdWnd() const { return m_hwnd; }
	HWND GetLeftPathWnd()    const { return m_hwndLeft; }
	HWND GetRightPathWnd()   const { return m_hwndRight; }
	HWND GetActivePathWnd()  const { return m_hwndActive; }

	bool IsLeftDirActive() const; 
    bool GetDirs( LPTSTR pLeftDir = NULL, unsigned leftDirLen = 0, 
                  LPTSTR pRightDir = NULL, unsigned rightDirLen = 0 ) const;
	bool GetActiveDir( LPTSTR pDir, unsigned len ) const;
	
private:
    struct CFindSubWindowsData
    {
        CTotalCmdUtils* m_thisptr;
		CFindSubWindowsData() : m_thisptr( NULL ) {}
    };

private:
    void FindSubWindows();
    static BOOL CALLBACK FindTopWnd_Proc( HWND hwnd, LPARAM lParam );
    static BOOL CALLBACK FindSubWindows_Proc( HWND hwnd, LPARAM lParam );
   
private:
    HWND m_hwnd, m_hwndLeft, m_hwndRight, m_hwndActive;
    bool m_isLeftDirActive;
};

//-------------------------------------------------------------------------------------------------

/// \brief Get install-directory and path of .INI-file of Total Commander
bool GetTotalCmdLocation( tstring* pInstallDir = NULL, tstring* pIniPath = NULL );

/// Split a TC command
void SplitTcCommand( LPCTSTR pCmd, tstring* pToken, tstring* pArgs = NULL );

/// Bit flags for SetTcCurrentPathes()
enum 
{
	STC_SOURCE_AND_TARGET = 0x0001,
	STC_BACKGROUND_TAB    = 0x0002
};

/// Set current TC pathes
bool SetTcCurrentPathesW( HWND hWndTC, LPCWSTR pPath1, LPCWSTR pPath2, DWORD flags = 0 );

/// Get HWND of TC that is topmost in z-order
HWND FindTopTcWnd( bool currentThreadOnly = false );

/// Check if a HWND identifies one of the left/right path controls of TC
bool IsTcPathControl( HWND hwnd );

/// Get the directory path from a TC path control (strips backslash and filter)
void GetPathFromTcControl( HWND hwnd, LPTSTR pPath, size_t nSize ); 

