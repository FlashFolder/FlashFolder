// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WINVER 0x0600		
#define _WIN32_WINNT WINVER	
#define _WIN32_WINDOWS WINVER
#define _WIN32_IE 0x0600

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

// Defend some kind of buffer overrun possibilities.
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#pragma warning(disable:4995)  // caused by strsafe.h

//--- MFC headers

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

// XML parser
#include <TinyXml/TinyXml.h>

// utilities
#include <tstring.h>
#include <Utils.h>
#include <StringUtils.h>
#include <UnicodeUtils.h>
#include <Profile.h>
#include <Registry.h>
#include <TotalCmdUtils.h>
#include <DownloadThread.h>
#include <VersionInfo.h>

// UI
#include <GdiUtils.h>
#include <GroupCheck.h>
#include <ResizableDlg.h>
#include <FolderDlg.h>
#include <EditEx.h>
#include <HyperLink.h>
#include <DlgExpander.h>
#include <TreeListCtrl/EditableTreeListCtrl.h>
#include <BalloonHelp.h>
#include <TreePropSheet/TreePropSheet.h>
#include <TreePropSheet/PropPageFrame.h>
#include <TreePropSheet/PropPageFrameDefault.h>
#include <PopupDlg.h>
#include <CacheImageList.h>
#include <MessageBox.h>
#include <VistaFontHelper.h>

// project-specific
#include "../common/ff_utils.h"
#include "../common/Favorites.h"
#include "../common/ProfileDefaults.h"
