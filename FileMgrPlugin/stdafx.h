
#pragma once

#ifndef WINVER				
#define WINVER 0x0501		
#endif

#ifndef _WIN32_WINNT		                   
#define _WIN32_WINNT WINVER	
#endif						

#ifndef _WIN32_IE			
#define _WIN32_IE 0x0600
#endif

#define WIN32_LEAN_AND_MEAN		// Selten verwendete Teile der Windows-Header nicht einbinden.
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <stdlib.h>
#include <tchar.h>
#include <string>
#include <vector>

#include <common\_autolink.h>
#include <common\PluginApi.h>
#include <common\TotalCmdUtils.h>
#include <common\ff_utils.h>

#include <commonUtils\_autolink.h>
#include <commonUtils\StringUtils.h>
#include <commonUtils\Utils.h>
#include <commonUtils\Registry.h>

#include <strsafe.h>
