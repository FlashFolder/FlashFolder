// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WINVER 0x0500		
#define _WIN32_WINNT 0x0500	
#define _WIN32_WINDOWS 0x0500 


// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

//--- MFC headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <shlwapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shlobj.h>

//--- ATL headers

#include <atlbase.h>

//--- STD headers

#include <vector>
#include <map>
#include <set>
#include <string>
#include <iterator>
#include <cstdlib>
#include <strsafe.h>

//--- own headers
// utilities
#include "../common/tstring.h"
#include "../common/StringUtils.h"
#include "../common/Profile.h"
#include "../common/ff_utils.h"
#include "../common/Registry.h"
#include "../common/TotalCmdUtils.h"
#include "../common/Favorites.h"
// UI
#include "../common/GroupCheck.h"
#include "../common/SizeGrip.h"
#include "../common/DlgAnchor.h"
#include "../common/FolderDlg.h"
#include "../common/DragListCtrl.h"
#include "../common/EditEx.h"
#include "../common/HyperLink.h"
