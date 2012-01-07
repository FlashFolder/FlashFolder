
#pragma once

//--- Windows headers

#define WINVER 0x0600		
#define _WIN32_WINNT WINVER	
#define _WIN32_IE 0x0600
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>

//--- ATL headers

#include <atlbase.h>
#include <atlrx.h>
#include <atlfile.h>
#include <atlsecurity.h>

//--- std headers

#include <stdlib.h>
#include <tchar.h>
#include <string>
#include <vector>

//--- own headers

#include <common\_autolink.h>
#include <common\PluginApi.h>

#include <commonUtils\_autolink.h>
#include <commonUtils\StringUtils.h>
#include <commonUtils\Utils.h>
#include <commonUtils\Registry.h>
#include <commonUtils\FileFinder.h>
#include <commonUtils\ItemIdList.h>

// include after all others to avoid false compiler warnings
#include <strsafe.h>
