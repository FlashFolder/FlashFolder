// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WINVER 0x0501		
#define _WIN32_WINNT 0x0501	
#define _WIN32_WINDOWS 0x0501 


// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#pragma warning(disable:4995)  // caused by strsafe.h

//--- WMFC headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxdlgs.h>

//--- ATL headers

#include <atlbase.h>
#include <atlrx.h>

//--- Windows headers

#include <shlwapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shlobj.h>
#include <urlmon.h>
#include <Wininet.h>

//--- STD headers

#include <strsafe.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iterator>
#include <cstdlib>

//--- own headers

// TinyXML
#include "../commonUtils/TinyXml/TinyXml.h"

// utilities
#include "../commonUtils/tstring.h"
#include "../commonUtils/Utils.h"
#include "../commonUtils/StringUtils.h"
#include "../commonUtils/Profile.h"
#include "../commonUtils/Registry.h"
#include "../commonUtils/TotalCmdUtils.h"
#include "../commonUtils/DownloadThread.h"
#include "../commonUtils/VersionInfo.h"
// UI
#include "../commonUtils/GroupCheck.h"
#include "../commonUtils/ResizableDlg.h"
#include "../commonUtils/FolderDlg.h"
#include "../commonUtils/DragListCtrl.h"
#include "../commonUtils/EditEx.h"
#include "../commonUtils/HyperLink.h"
#include "../commonUtils/DlgExpander.h"
// UI - TreePropSheet headers
#include "../commonUtils/TreePropSheet/TreePropSheet.h"
#include "../commonUtils/TreePropSheet/PropPageFrame.h"
#include "../commonUtils/TreePropSheet/PropPageFrameDefault.h"

// project-specific
#include "../common/ff_utils.h"
#include "../common/Favorites.h"
