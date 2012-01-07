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

namespace totalcmdutils
{

//-------------------------------------------------------------------------------------------------
/// \brief Class to get information from a running Total Commander instance.

class TcInstance
{
public:
    TcInstance( HWND hwndTotalCmd = NULL ) { SetTCmdWnd( hwndTotalCmd ); }
    void SetTCmdWnd( HWND hwndTotalCmd ); 
    
    HWND GetTCmdWnd() const { return m_hwnd; }
	HWND GetLeftPathWnd()    const { return m_hwndLeft; }
	HWND GetRightPathWnd()   const { return m_hwndRight; }
	HWND GetActivePathWnd()  const { return m_hwndActive; }

	bool IsLeftDirActive() const; 
    bool GetDirs( LPWSTR pLeftDir = NULL, unsigned leftDirLen = 0, 
                  LPWSTR pRightDir = NULL, unsigned rightDirLen = 0 ) const;
	bool GetActiveDir( LPWSTR pDir, unsigned len ) const;
	
private:
    struct FindSubWindowsData
    {
        TcInstance* m_thisptr;
		FindSubWindowsData() : m_thisptr( NULL ) {}
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
bool GetTotalCmdLocation( std::wstring* pInstallDir = NULL, std::wstring* pIniPath = NULL );

/// Split a TC command
void SplitTcCommand( LPCWSTR pCmd, std::wstring* pToken, std::wstring* pArgs = NULL );

/// Bit flags for SetCurrentPathes()
enum 
{
	STC_SOURCE_AND_TARGET = 0x0001,
	STC_BACKGROUND_TAB    = 0x0002
};

/// Set current TC pathes
bool SetCurrentPathes( HWND hWndTC, LPCWSTR pPath1, LPCWSTR pPath2, DWORD flags = 0 );

/// Get HWND of TC that is topmost in z-order
HWND FindTopTcWnd( bool currentThreadOnly = false );

/// Check if a HWND identifies one of the left/right path controls of TC
bool IsTcPathControl( HWND hwnd );

/// Get the directory path from a TC path control (strips backslash and filter)
void GetPathFromTcControl( HWND hwnd, LPWSTR pPath, size_t nSize ); 

//----------------------------------------------------------------------------------------------------

/// TC folder favorites menu item.
struct FavMenuItem { std::wstring menu, cmd, path, param; };

/// TC folder favorites menu.
typedef std::vector< FavMenuItem > FavMenu;

/// Load the folder favorites or the starter menu from a TC INI file.
/// Note: RedirectSection key is currently not supported.
bool LoadFavoritesMenu( FavMenu* items, LPCWSTR iniPath, LPCWSTR iniSection = L"DirMenu" );

/// Save the folder favorites or the starter menu to a TC INI file, replacing all previous menu items
/// of the given section.
/// Note: RedirectSection key is currently not supported. The INI file will not be changed if 
/// a RedirectSection key is encountered.
bool SaveFavoritesMenu( const FavMenu& items, LPCWSTR iniPath, LPCWSTR iniSection = L"DirMenu" );

//----------------------------------------------------------------------------------------------------

}; //namespace