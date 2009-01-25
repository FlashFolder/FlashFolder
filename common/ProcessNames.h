#pragma once

#ifdef WIN64
	#define FLASHFOLDER_EXE L"FlashFolder64.exe"
	#define FFCONFIG_EXE L"FFConfig64.exe"
	#define FFLIB_DLL L"fflib6439_64.dll"
	#define FLASHFOLDER_NAME L"FlashFolder (x64)"
#else
	#define FLASHFOLDER_EXE L"FlashFolder.exe"
	#define FFCONFIG_EXE L"FFConfig.exe"
	#define FFLIB_DLL L"fflib6439.dll"
	#define FLASHFOLDER_NAME L"FlashFolder (x86)"
#endif