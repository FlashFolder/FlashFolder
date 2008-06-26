
#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely used Windows-Headers

// Windows headers
#include <windows.h>
#include <WtsApi32.h>

// ATL headers
#include <atlbase.h>

// CRT headers
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#pragma comment( lib, "fflib" )
#pragma comment( lib, "WtsApi32" )
