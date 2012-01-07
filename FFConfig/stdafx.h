
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

//--- boost headers

#include <boost\foreach.hpp>
#include <boost\scoped_ptr.hpp>
#define foreach BOOST_FOREACH

//--- own headers

// utilities
#include <commonUtils\_autolink.h>
#include <CommonUtils\tstring.h>
#include <CommonUtils\Utils.h>
#include <CommonUtils\StringUtils.h>
#include <CommonUtils\UnicodeUtils.h>
#include <CommonUtils\Profile.h>
#include <CommonUtils\Registry.h>
#include <CommonUtils\DownloadThread.h>
#include <CommonUtils\VersionInfo.h>
#include <commonUtils\TinyXml\TinyXml.h>

// UI
#include <CommonUtils\GdiUtils.h>
#include <CommonUtils\GroupCheck.h>
#include <CommonUtils\ResizableDlg.h>
#include <CommonUtils\FolderDlg.h>
#include <CommonUtils\EditEx.h>
#include <CommonUtils\HyperLink.h>
#include <CommonUtils\DlgExpander.h>
#include <CommonUtils\TreeListCtrl/EditableTreeListCtrl.h>
#include <CommonUtils\BalloonHelp.h>
#include <CommonUtils\TreePropSheet/TreePropSheet.h>
#include <CommonUtils\TreePropSheet/PropPageFrame.h>
#include <CommonUtils\TreePropSheet/PropPageFrameDefault.h>
#include <CommonUtils\PopupDlg.h>
#include <CommonUtils\CacheImageList.h>
#include <CommonUtils\MessageBox.h>
#include <CommonUtils\VistaFontHelper.h>
#include <CommonUtils\ItemIdList.h>

// FlashFolder-specific
#include <common\_autolink.h>
#include <common\ff_utils.h>
#include <common\ProfileDefaults.h>
#include <common\ProcessNames.h>

