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

// globals
HMODULE g_hInstCADLL;

// statics
static BOOL s_fInitialized;
static MSIHANDLE s_hInstall;
static MSIHANDLE s_hDatabase;
static WCHAR s_szCustomActionLogName[32];
static UINT s_iRetVal;


/********************************************************************
 WcaGlobalInitialize() - initializes the Wca library, should be 
                         called once per custom action Dll during
                         DllMain on DLL_PROCESS_ATTACH

********************************************************************/
extern "C" void WIXAPI WcaGlobalInitialize(
    __in HINSTANCE hInst
    )
{
    g_hInstCADLL = hInst;
    MemInitialize();

    AssertSetModule(g_hInstCADLL);
    AssertSetDisplayFunction(WcaDisplayAssert);
}


/********************************************************************
 WcaGlobalFinalize() - finalizes the Wca library, should be the
                       called once per custom action Dll during
                       DllMain on DLL_PROCESS_DETACH

********************************************************************/
extern "C" void WIXAPI WcaGlobalFinalize()
{
#ifdef DEBUG
    if (WcaIsInitialized())
    {
        CHAR szBuf[2048];
        StringCchPrintfA(szBuf, countof(szBuf), L"CustomAction %s called WcaInitialize() but not WcaFinalize()", WcaGetLogName());

        AssertSz(FALSE, szBuf);
    }
#endif
    MemUninitialize();
    g_hInstCADLL = NULL;
}


/********************************************************************
 WcaInitialize() - initializes the Wca framework, should be the first 
                   thing called by all CustomActions

********************************************************************/
HRESULT WIXAPI WIXAPI WcaInitialize(
    __in MSIHANDLE hInstall,
    __in_z PCWSTR szCustomActionLogName
    )
{
    // these statics should be called once per CustomAction invocation.
    // Darwin does doesn't preserve DLL state across CustomAction calls so
    // these should always be initialized to NULL.  If that behavior changes
    // we would need to do a careful review of all of our module/global data.
    AssertSz(!s_fInitialized, L"WcaInitialize() should only be called once per CustomAction");
    Assert(NULL == s_hInstall);
    Assert(NULL == s_hDatabase);
    Assert(0 == *s_szCustomActionLogName);

    HRESULT hr = S_OK;

    s_fInitialized = TRUE;
    s_iRetVal = ERROR_SUCCESS; // assume all will go well

    s_hInstall = hInstall;
    s_hDatabase = ::MsiGetActiveDatabase(s_hInstall); // may return null if deferred CustomAction

    hr = ::StringCchCopyW(s_szCustomActionLogName, countof(s_szCustomActionLogName), szCustomActionLogName);
    ExitOnFailure1(hr, L"Failed to copy CustomAction log name: %s", szCustomActionLogName);

    Assert(s_hInstall);
LExit:
    if (FAILED(hr))
    {
        if (s_hDatabase)
        {
            ::MsiCloseHandle(s_hDatabase);
            s_hDatabase = NULL;
        }

        s_hInstall = NULL;
        s_fInitialized = FALSE;
    }

    return hr;
}


/********************************************************************
 WcaFinalize() - cleans up after the Wca framework, should be the last 
                 thing called by all CustomActions

********************************************************************/
extern "C" UINT WIXAPI WcaFinalize(
    __in UINT iReturnValue
    )
{
    AssertSz(!WcaIsWow64Initialized(), L"WcaFinalizeWow64() should be called before calling WcaFinalize()");

    // clean up after our initialization
    if (s_hDatabase)
    {
        ::MsiCloseHandle(s_hDatabase);
        s_hDatabase = NULL;
    }

    s_hInstall = NULL;
    s_fInitialized = FALSE;

    // if no error occurred during the processing of the CusotmAction return the passed in return value
    // otherwise return the previous failure
    return (ERROR_SUCCESS == s_iRetVal) ? iReturnValue : s_iRetVal; 
}


/********************************************************************
 WcaIsInitialized() - determines if WcaInitialize() has been called

********************************************************************/
extern "C" BOOL WIXAPI WcaIsInitialized()
{
    return s_fInitialized;
}


/********************************************************************
 WcaGetInstallHandle() - gets the handle to the active install session

********************************************************************/
extern "C" MSIHANDLE WIXAPI WcaGetInstallHandle()
{
    AssertSz(s_hInstall, L"WcaInitialize() should be called before attempting to access the install handle.");
    return s_hInstall;
}


/********************************************************************
 WcaGetDatabaseHandle() - gets the handle to the active database

 NOTE: this function can only be used in immediate CustomActions.
       Deferred CustomActions do not have access to the active
       database.
********************************************************************/
extern "C" MSIHANDLE WIXAPI WcaGetDatabaseHandle()
{
    AssertSz(s_hDatabase, L"WcaInitialize() should be called before attempting to access the install handle.  Also note that deferred CustomActions do not have access to the active database.");
    return s_hDatabase;
}


/********************************************************************
 WcaGetLogName() - gets the name of the CustomAction used in logging

********************************************************************/
extern "C" const WCHAR* WIXAPI WcaGetLogName()
{
    return s_szCustomActionLogName;
}


/********************************************************************
 WcaSetReturnValue() - sets the value to return from the CustomAction

********************************************************************/
extern "C" void WIXAPI WcaSetReturnValue(
    __in UINT iReturnValue
    )
{
    s_iRetVal = iReturnValue;
}


/********************************************************************
 WcaCancelDetected() - determines if the user has canceled yet

 NOTE: returns true when WcaSetReturnValue() is set to ERROR_INSTALL_USEREXIT
********************************************************************/
extern "C" BOOL WIXAPI WcaCancelDetected()
{
    return ERROR_INSTALL_USEREXIT == s_iRetVal;
}
