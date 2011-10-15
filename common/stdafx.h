
#pragma once

#define WINVER 0x0601	
#define _WIN32_WINNT WINVER	
#define _WIN32_WINDOWS WINVER
#define _WIN32_IE 0x0600

#define WIN32_LEAN_AND_MEAN		
#define VC_EXTRALEAN
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	
#define _AFX_ALL_WARNINGS

// Defend some kind of buffer overrun possibilities.
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#pragma warning(disable:4995)  // caused by strsafe.h

//--- MFC headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxdlgs.h>
#include <afxdisp.h>
#include <afxocc.h>
#include <afxpriv.h>

//--- ATL headers

#include <atlbase.h>
#include <atlrx.h>
#include <atlfile.h>
#include <atlsecurity.h>

//--- Windows headers

#include <shlwapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shlobj.h>
#include <urlmon.h>
#include <Wininet.h>
#include <commdlg.h>
#include <dlgs.h>
#include <cpl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <dwmapi.h>

//--- STD headers

#include <vector>
#include <map>
#include <set>
#include <string>
#include <iterator>
#include <cstdlib>
#include <cstdio>
#include <crtdbg.h>   //for _ASSERTE
#include <assert.h>
#include <memory>
#include <strsafe.h>  // include after all STD headers to avoid false warnings

//--- boost headers

#include <boost\foreach.hpp>
#define foreach BOOST_FOREACH

//--- CommonUtils headers

#include <CommonUtils\_autolink.h>
#include <CommonUtils\Profile.h>
#include <CommonUtils\Utils.h>
#include <commonUtils\GdiUtils.h>
#include <commonUtils\FileFinder.h>
#include <commonUtils\Module.h>

//--- common headers

#include <common\TotalCmdUtils.h>

