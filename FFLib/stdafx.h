
#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely used Windows-Headers

#define WINVER 0x0600
#define _WIN32_WINNT WINVER
#define _WIN32_WINDOWS WINVER 
#define _WIN32_IE 0x0600

// Defend some kind of buffer overrun possibilities.
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#pragma warning(disable:4995) // "deprecated" warnings caused by strsafe.h

//--- windows headers
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shlobj.h>
#include <dlgs.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <cpl.h>
#include <dwmapi.h>

//--- ATL headers
#include <atlfile.h>

//--- crt headers
#include <tchar.h>
#include <crtdbg.h>   //for _ASSERTE
#include <cstdio>
#include <memory>
using namespace std;

//--- STL headers
#include <vector>
#include <map>
#include <set>
#include <string>

//--- common project files
#include <common\_autolink.h>
#include <common\Favorites.h>
#include <common\ff_utils.h>
#include <common\HistoryLst.h>
#include <common\ProfileDefaults.h>
#include <common\ProcessNames.h>

//--- utilities
#include <commonUtils\_autolink.h>
#include <commonUtils\tstring.h>
#include <commonUtils\OsVersion.h>
#include <commonUtils\Utils.h>
#include <commonUtils\GdiUtils.h>
#include <commonUtils\Profile.h>
#include <commonUtils\TotalCmdUtils.h>
#include <commonUtils\Registry.h>

// include after all other headers to avoid false warnings
#include <strsafe.h>
