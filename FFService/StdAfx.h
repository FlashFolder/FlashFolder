
#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely used Windows-Headers

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_WINDOWS 0x0500 

#pragma warning(disable:4995) // "deprecated" warnings caused by strsafe.h

// Windows headers
#include <windows.h>

// CRT headers
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#include "../commonUtils/utils.h"
