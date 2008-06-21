//-------------------------------------------------------------------------------------------------
// <copyright file="wcalog.cpp" company="Microsoft">
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
//    Windows Installer XML CustomAction utility library logging functions
// </summary>
//-------------------------------------------------------------------------------------------------
// This file was changed by zett42:
// - consistently use Unicode for strings 

#include "precomp.h"


/********************************************************************
 IsVerboseLogging() - internal helper function to detect if doing
                      verbose logging

********************************************************************/
static BOOL IsVerboseLogging()
{
    static int iVerbose = -1;
    LPWSTR pwzMsiLogging = NULL;

    if (0 > iVerbose)
    {
        iVerbose = WcaIsPropertySet(L"LOGVERBOSE");
        if (0 == iVerbose) 
        {
            // if the property wasn't set, check the MsiLogging property (MSI 4.0+)
            HRESULT hr = WcaGetProperty(L"MsiLogging", &pwzMsiLogging);
            ExitOnFailure(hr, L"failed to get MsiLogging property");
            int cchMsiLogging = lstrlenW(pwzMsiLogging);
            if (0 < cchMsiLogging)
            {
                for (int i = 0; i < cchMsiLogging; i++)
                {
                    if (L'v' == pwzMsiLogging[i] || L'V' == pwzMsiLogging[i])
                    {
                        iVerbose = 1;
                        break;
                    }
                }
            }

            // last chance: Check the registry to see if the logging policy was turned on
            if (0 == iVerbose) 
            {
                HKEY hkey = NULL;
                WCHAR rgwc[16] = { 0 };
                DWORD cb = sizeof(rgwc);
                if (ERROR_SUCCESS == ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Policies\\Microsoft\\Windows\\Installer", 0, KEY_QUERY_VALUE, &hkey))
                {
                    if (ERROR_SUCCESS == ::RegQueryValueExW(hkey, L"Logging", 0, NULL, reinterpret_cast<BYTE*>(rgwc), &cb))
                    {
                        for (LPCWSTR pwc = rgwc; (cb / sizeof(WCHAR)) > static_cast<DWORD>(pwc - rgwc) && *pwc; pwc++)
                        {
                            if (L'v' == *pwc || L'V' == *pwc)
                            {
                                iVerbose = 1;
                                break;
                            }
                        }
                    }

                    ::RegCloseKey(hkey);
                }
            }
        }
    }

LExit:
    ReleaseStr(pwzMsiLogging);
    Assert(iVerbose >= 0);
    return (BOOL)iVerbose;
}


/********************************************************************
 WcaLog() - outputs trace and log info

*******************************************************************/
extern "C" void WcaLog(
    __in LOGLEVEL llv,
    __in_z __format_string PCWSTR fmt, ...
    )
{
    static WCHAR szFmt[LOG_BUFFER];
    static WCHAR szBuf[LOG_BUFFER];
    static bool fInLogPrint = false;

    // prevent re-entrant logprints.  (recursion issues between assert/logging code)
    if (fInLogPrint)
        return;
    fInLogPrint = true;

    if (LOGMSG_STANDARD == llv || 
        (LOGMSG_VERBOSE == llv && IsVerboseLogging())
#ifdef DEBUG
        || LOGMSG_TRACEONLY == llv
#endif
        )
    {
        va_list args;
        va_start(args, fmt);

        LPCWSTR szLogName = WcaGetLogName();
        if (szLogName[0] != 0)
            StringCchPrintfW(szFmt, countof(szFmt), L"%s:  %s", szLogName, fmt);
        else
            StringCchCopyW(szFmt, countof(szFmt), fmt);

        StringCchVPrintfW(szBuf, countof(szBuf), szFmt, args);
        va_end(args);

#ifdef DEBUG
        // always write to the log in debug
#else
        if (llv == LOGMSG_STANDARD || (llv == LOGMSG_VERBOSE && IsVerboseLogging()))
#endif
        {
            PMSIHANDLE hrec = MsiCreateRecord(1);

            ::MsiRecordSetStringW(hrec, 0, szBuf);
            // TODO:  Recursion on failure.  May not be safe to assert from here.
            WcaProcessMessage(INSTALLMESSAGE_INFO, hrec);
        }

#if DEBUG
        StringCchCatW(szBuf, countof(szBuf), L"\n");
        OutputDebugStringW(szBuf);
#endif
    }

    fInLogPrint = false;
    return;
}


/********************************************************************
 WcaDisplayAssert() - called before Assert() dialog shows

 NOTE: writes the assert string to the MSI log
********************************************************************/
extern "C" BOOL WIXAPI WcaDisplayAssert(
    __in LPCWSTR sz
    )
{
    WcaLog(LOGMSG_STANDARD, L"Debug Assert Message: %s", sz);
    return TRUE;
}


/********************************************************************
 WcaLogError() - called before ExitOnXXX() macro exists the function

 NOTE: writes the hresult and error string to the MSI log
********************************************************************/
extern "C" void WcaLogError(
    __in HRESULT hr,
    __in LPCWSTR szMessage,
    ...
    )
{
    WCHAR szBuffer[LOG_BUFFER];
    va_list dots;

    va_start(dots, szMessage);
    StringCchVPrintfW(szBuffer, countof(szBuffer), szMessage, dots);
    va_end(dots);

    // log the message if using Wca common layer
    if (WcaIsInitialized())
        WcaLog(LOGMSG_STANDARD, L"Error 0x%x: %s", hr, szBuffer);
}
