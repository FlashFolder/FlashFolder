
#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely used Windows-Headers

// Defend some kind of buffer overrun possibilities.
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

// Windows headers
#include <windows.h>
#include <WtsApi32.h>

// ATL headers
#include <atlbase.h>
#include <atlfile.h>
#include <atlsecurity.h>

// CRT headers
#include <cstdio>
#include <tchar.h>

// Common headers
#include <common\_autolink.h>
#include <common\ProcessNames.h>
#include <commonUtils\_autolink.h>
#include <commonUtils\utils.h>

#include <strsafe.h>

#pragma comment( lib, "fflib" )
#pragma comment( lib, "WtsApi32" )

//---

#define foreach BOOST_FOREACH