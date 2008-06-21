//-------------------------------------------------------------------------------------------------
// <copyright file="wcautil.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    Windows Installer XML CustomAction utility library.
// </summary>
//-------------------------------------------------------------------------------------------------
// modified by z42:
// - get it compiled with VC8
// - consistently use Unicode for strings 

#include "precomp.h"

static HMODULE s_hKernel32;
static BOOL s_fWow64Initialized;
static BOOL (*s_pfnDisableWow64)(__out PVOID* );
static BOOL (*s_pfnRevertWow64)(__in PVOID );
static PVOID s_Wow64FSRevertState;
static BOOL s_fWow64FSDisabled;

/********************************************************************
 WcaInitializeWow64() - Initializes the Wow64 API

********************************************************************/
HRESULT WIXAPI WcaInitializeWow64()
{
    AssertSz(WcaIsInitialized(), L"WcaInitialize() should be called before calling WcaInitializeWow64()");
    AssertSz(!WcaIsWow64Initialized(), L"WcaInitializeWow64() should not be called twice without calling WcaFinalizeWow64()");

    s_fWow64Initialized = FALSE;
    HRESULT hr = S_OK;
    s_Wow64FSRevertState = NULL;
    s_fWow64FSDisabled = false;

    // Test if we have access to the Wow64 API, and store the result in bWow64APIPresent
    s_hKernel32 = GetModuleHandle("kernel32.dll");
    if (!s_hKernel32)
    {
        ExitOnLastError(hr, L"failed to get handle to kernel32.dll");
    }

    // This will test if we have access to the Wow64 API
    if (NULL != GetProcAddress(s_hKernel32,"IsWow64Process"))
    {
        s_pfnDisableWow64 = (BOOL (*)(PVOID *))GetProcAddress(s_hKernel32, "Wow64DisableWow64FsRedirection");
        // If we fail, log the error but proceed, because we may not need a particular function, or the Wow64 API at all
        if (!s_pfnDisableWow64)
        {
            WcaLog(LOGMSG_STANDARD, L"Found Wow64 API, but unable to link to Wow64DisableWow64FsRedirection function in kernel32.dll");
        }

        s_pfnRevertWow64 = (BOOL (*)(PVOID))GetProcAddress(s_hKernel32, "Wow64RevertWow64FsRedirection");
        if (!s_pfnRevertWow64)
        {
            WcaLog(LOGMSG_STANDARD, L"Found Wow64 API, but unable to link to Wow64RevertWow64FsRedirection function in kernel32.dll");
        }

        if (s_pfnDisableWow64 && s_pfnRevertWow64)
            s_fWow64Initialized = TRUE;
    }

LExit:

    return hr;
}

/********************************************************************
 WcaIsWow64Initialized() - determines if WcaInitializeWow64() has
                           been successfully called

********************************************************************/
extern "C" BOOL WIXAPI WcaIsWow64Initialized()
{
    return s_fWow64Initialized;
}

/********************************************************************
 WcaDisableWow64FSRedirection() - Disables Wow64 FS Redirection

********************************************************************/
extern "C" BOOL WIXAPI WcaDisableWow64FSRedirection()
{
    AssertSz(s_fWow64Initialized && s_pfnDisableWow64 != NULL, L"WcaDisableWow64FSRedirection() called, but Wow64 API was not initialized");

#ifdef DEBUG
    AssertSz(!s_fWow64FSDisabled, L"You must call WcaRevertWow64FSRedirection() before calling WcaDisableWow64FSRedirection() again");
#endif

    BOOL bRetval = s_pfnDisableWow64(&s_Wow64FSRevertState);

    if (bRetval)
    {
        s_fWow64FSDisabled = TRUE;
    }

    return bRetval;
}

/********************************************************************
 WcaRevertWow64FSRedirection() - Reverts Wow64 FS Redirection to its
                                 pre-disabled state

********************************************************************/
extern "C" BOOL WIXAPI WcaRevertWow64FSRedirection()
{
    AssertSz(s_fWow64Initialized && s_pfnDisableWow64 != NULL, L"WcaRevertWow64FSRedirection() called, but Wow64 API was not initialized");

#ifdef DEBUG
    AssertSz(s_fWow64FSDisabled, L"You must call WcaDisableWow64FSRedirection() before calling WcaRevertWow64FSRedirection()");
#endif

    BOOL bRetval = s_pfnRevertWow64(s_Wow64FSRevertState);

    s_fWow64FSDisabled = FALSE;

    return bRetval;
}

/********************************************************************
 WcaFinalizeWow64() - Cleans up after Wow64 API Initialization

********************************************************************/
HRESULT WIXAPI WIXAPI WcaFinalizeWow64()
{
    if (s_fWow64FSDisabled)
    {
#ifdef DEBUG
        AssertSz(FALSE, L"WcaFinalizeWow64() called while Filesystem redirection was disabled.");
#else
        // If we aren't in debug mode, let's do our best to recover gracefully
        WcaRevertWow64FSRedirection();
#endif
    }

    s_fWow64Initialized = FALSE;
    s_pfnDisableWow64 = NULL;
    s_pfnRevertWow64 = NULL;

    return S_OK;
}
