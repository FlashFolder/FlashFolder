
#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely used Windows-Headers

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

//--- windows headers
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shlobj.h>
#include <dlgs.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <shellapi.h>

//--- crt headers
#include <tchar.h>
#include <crtdbg.h>   //for _ASSERTE
#include <cstdio>
#include <vector>
#include <string>
#include <set>
#include <memory>
using namespace std;

//--- STL headers
#include <map>

//--- own headers
#include "../_version.h"
#include "../common/tstring.h"
#include "../common/Profile.h"
#include "../common/ff_utils.h"
#include "../common/NtKernelApi.h"
#include "../common/NtKernelUtils.h"
#include "../common/HistoryLst.h"
#include "../common/TotalCmdUtils.h"
#include "../common/Registry.h"
#include "../common/Favorites.h"

