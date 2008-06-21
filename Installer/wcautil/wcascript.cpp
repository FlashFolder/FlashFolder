//-------------------------------------------------------------------------------------------------
// <copyright file="wcascript.cpp" company="Microsoft">
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
//    Windows Installer XML CustomAction utility library CaScript functions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


static HRESULT CaScriptFileName(
    __in WCA_ACTION action,
    __in WCA_CASCRIPT script,
    __in BOOL fImpersonated,
    __in LPCWSTR wzScriptKey,
    __out LPWSTR* pwzScriptName
    );


/********************************************************************
 WcaCaScriptCreateKey() - creates a unique script key for this
                          CustomAction.

********************************************************************/
HRESULT WIXAPI WIXAPI WcaCaScriptCreateKey(
    __out LPWSTR* ppwzScriptKey
    )
{
    AssertSz(WcaIsInitialized(), L"WcaInitialize() should have been called before calling this function.");
    HRESULT hr = S_OK;

    hr = StrAllocString(ppwzScriptKey, WcaGetLogName(), 0);
    ExitOnFailure(hr, L"Failed to create script key.");

LExit:
    return hr;
}


/********************************************************************
 WcaCaScriptCreate() - creates the appropriate script for this
                       CustomAction Script Key.

********************************************************************/
HRESULT WIXAPI WIXAPI WcaCaScriptCreate(
    __in WCA_ACTION action,
    __in WCA_CASCRIPT script,
    __in BOOL fImpersonated,
    __in LPCWSTR wzScriptKey,
    __in BOOL fAppend,
    __in WCA_CASCRIPT_HANDLE* phScript
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzScriptPath = NULL;
    HANDLE hScriptFile = INVALID_HANDLE_VALUE;

    hr = CaScriptFileName(action, script, fImpersonated, wzScriptKey, &pwzScriptPath);
    ExitOnFailure(hr, L"Failed to calculate script file name.");

    hScriptFile = ::CreateFileW(pwzScriptPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, fAppend ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (INVALID_HANDLE_VALUE == hScriptFile)
    {
        ExitWithLastError1(hr, L"Failed to open CaScript: %s", pwzScriptPath);
    }

    if (fAppend && INVALID_SET_FILE_POINTER == ::SetFilePointer(hScriptFile, 0, NULL, FILE_END))
    {
        ExitWithLastError(hr, L"Failed to seek to end of file.");
    }

    *phScript = reinterpret_cast<WCA_CASCRIPT_HANDLE>(MemAlloc(sizeof(WCA_CASCRIPT_STRUCT), TRUE));
    ExitOnNull(*phScript, hr, E_OUTOFMEMORY, L"Failed to allocate space for cascript handle.");

    (*phScript)->pwzScriptPath = pwzScriptPath;
    pwzScriptPath = NULL;
    (*phScript)->hScriptFile = hScriptFile;
    hScriptFile = INVALID_HANDLE_VALUE;

LExit:
    if (INVALID_HANDLE_VALUE != hScriptFile)
    {
        ::CloseHandle(hScriptFile);
    }

    ReleaseStr(pwzScriptPath);
    return hr;
}


/********************************************************************
 WcaCaScriptOpen() - opens the appropriate script for this CustomAction
                     Script Key.

********************************************************************/
HRESULT WIXAPI WIXAPI WcaCaScriptOpen(
    __in WCA_ACTION action,
    __in WCA_CASCRIPT script,
    __in BOOL fImpersonated,
    __in LPCWSTR wzScriptKey,
    __in WCA_CASCRIPT_HANDLE* phScript
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzScriptPath = NULL;
    HANDLE hScriptFile = INVALID_HANDLE_VALUE;

    hr = CaScriptFileName(action, script, fImpersonated, wzScriptKey, &pwzScriptPath);
    ExitOnFailure(hr, L"Failed to calculate script file name.");

    hScriptFile = ::CreateFileW(pwzScriptPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (INVALID_HANDLE_VALUE == hScriptFile)
    {
        ExitWithLastError1(hr, L"Failed to open CaScript: %s", pwzScriptPath);
    }

    *phScript = reinterpret_cast<WCA_CASCRIPT_HANDLE>(MemAlloc(sizeof(WCA_CASCRIPT_HANDLE), TRUE));
    ExitOnNull(*phScript, hr, E_OUTOFMEMORY, L"Failed to allocate space for cascript handle.");

    (*phScript)->pwzScriptPath = pwzScriptPath;
    pwzScriptPath = NULL;
    (*phScript)->hScriptFile = hScriptFile;
    hScriptFile = INVALID_HANDLE_VALUE;

LExit:
    if (INVALID_HANDLE_VALUE != hScriptFile)
    {
        ::CloseHandle(hScriptFile);
    }

    ReleaseStr(pwzScriptPath);
    return hr;
}


/********************************************************************
 WcaCaScriptClose() - closes an open script handle.

********************************************************************/
extern "C" void WIXAPI WcaCaScriptClose(
    __in WCA_CASCRIPT_HANDLE hScript,
    __in WCA_CASCRIPT_CLOSE closeOperation
    )
{
    if (hScript)
    {
        if (INVALID_HANDLE_VALUE != hScript->hScriptFile)
        {
            ::CloseHandle(hScript->hScriptFile);
        }

        if (hScript->pwzScriptPath)
        {
            if (WCA_CASCRIPT_CLOSE_DELETE == closeOperation)
            {
                ::DeleteFileW(hScript->pwzScriptPath);
            }

            StrFree(hScript->pwzScriptPath);
        }

        MemFree(hScript);
    }
}


/********************************************************************
 WcaCaScriptReadAsCustomActionData() - read the ca script into a format
                                       that is useable by other CA data
                                       functions.

********************************************************************/
HRESULT WIXAPI WIXAPI WcaCaScriptReadAsCustomActionData(
    __in WCA_CASCRIPT_HANDLE hScript,
    __out LPWSTR* ppwzCustomActionData
    )
{
    HRESULT hr = S_OK;
    LARGE_INTEGER liScriptSize = { 0 };
    BYTE* pbData = NULL;
    DWORD cbData = 0;

    if (!::GetFileSizeEx(hScript->hScriptFile, &liScriptSize))
    {
        ExitWithLastError(hr, L"Failed to get size of ca script file.");
    }

    if (0 != liScriptSize.HighPart || 0 != (liScriptSize.LowPart % sizeof(WCHAR)))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, L"Invalid data read from ca script.");
    }

    cbData = liScriptSize.LowPart;
    pbData = static_cast<BYTE*>(MemAlloc(cbData, TRUE));
    ExitOnNull(pbData, hr, E_OUTOFMEMORY, L"Failed to allocate memory to read in ca script.");

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(hScript->hScriptFile, 0, NULL, FILE_BEGIN))
    {
        ExitWithLastError(hr, L"Failed to reset to beginning of ca script.");
    }

    DWORD cbTotalRead = 0;
    DWORD cbRead = 0;
    do
    {
        if (!::ReadFile(hScript->hScriptFile, pbData + cbTotalRead, cbData - cbTotalRead, &cbRead, NULL))
        {
            ExitWithLastError(hr, L"Failed to read from ca script.");
        }

        cbTotalRead += cbRead;
    } while (cbRead && cbTotalRead < cbData);

    if (cbTotalRead != cbData)
    {
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, L"Failed to completely read ca script.");
    }

    // Add one to the allocated space because the data stored in the script is not
    // null terminated.  After copying the memory over, we'll ensure the string is
    // null terminated.
    DWORD cchData = cbData / sizeof(WCHAR) + 1;
    hr = StrAlloc(ppwzCustomActionData, cchData);
    ExitOnFailure(hr, L"Failed to copy ca script.");

    CopyMemory(*ppwzCustomActionData, pbData, cbData);
    (*ppwzCustomActionData)[cchData - 1] = L'\0';

LExit:
    ReleaseMem(pbData);
    return hr;
}


/********************************************************************
 WcaCaScriptWriteString() - writes a string to the ca script.

********************************************************************/
HRESULT WIXAPI WIXAPI WcaCaScriptWriteString(
    __in WCA_CASCRIPT_HANDLE hScript,
    __in LPCWSTR wzValue
    )
{
    HRESULT hr = S_OK;
    DWORD cbFile = 0;
    DWORD cbWrite = 0;
    DWORD cbTotalWritten = 0;
    WCHAR delim[] = { MAGIC_MULTISZ_DELIM }; // magic char followed by NULL terminator

    cbFile = ::SetFilePointer(hScript->hScriptFile, 0, NULL, FILE_END);
    if (INVALID_SET_FILE_POINTER == cbFile)
    {
        ExitWithLastError(hr, L"Failed to move file pointer to end of file.");
    }

    // If there is existing data in the file, append on the magic delimeter
    // before adding our new data on the end of the file.
    if (0 < cbFile)
    {
        cbWrite = sizeof(delim);
        cbTotalWritten = 0;
        while (cbTotalWritten < cbWrite)
        {
            DWORD cbWritten = 0;
            if (!::WriteFile(hScript->hScriptFile, reinterpret_cast<BYTE*>(delim) + cbTotalWritten, cbWrite - cbTotalWritten, &cbWritten, NULL))
            {
                ExitWithLastError(hr, L"Failed to write data to ca script.");
            }

            cbTotalWritten += cbWritten;
        }
    }

    cbWrite = lstrlenW(wzValue) * sizeof(WCHAR);
    cbTotalWritten = 0;
    while (cbTotalWritten < cbWrite)
    {
        DWORD cbWritten = 0;
        if (!::WriteFile(hScript->hScriptFile, reinterpret_cast<const BYTE*>(wzValue) + cbTotalWritten, cbWrite - cbTotalWritten, &cbWritten, NULL))
        {
            ExitWithLastError(hr, L"Failed to write data to ca script.");
        }

        cbTotalWritten += cbWritten;
    }

LExit:
    return hr;
}


/********************************************************************
 WcaCaScriptWriteNumber() - writes a number to the ca script.

********************************************************************/
HRESULT WIXAPI WIXAPI WcaCaScriptWriteNumber(
    __in WCA_CASCRIPT_HANDLE hScript,
    __in DWORD dwValue
    )
{
    HRESULT hr = S_OK;
    WCHAR wzBuffer[13] = { 0 };

    hr = ::StringCchPrintfW(wzBuffer, countof(wzBuffer), L"%u", dwValue);
    ExitOnFailure(hr, L"Failed to convert number into string.");

    hr = WcaCaScriptWriteString(hScript, wzBuffer);
    ExitOnFailure(hr, L"Failed to write number to script.");

LExit:
    return hr;
}


/********************************************************************
 WcaCaScriptFlush() - best effort function to get script written to
                      disk.

********************************************************************/
extern "C" void WIXAPI WcaCaScriptFlush(
    __in WCA_CASCRIPT_HANDLE hScript
    )
{
    ::FlushFileBuffers(hScript->hScriptFile);
}


/********************************************************************
 WcaCaScriptCleanup() - best effort clean-up of any cascripts left
                        over from this install/uninstall.

********************************************************************/
extern "C" void WIXAPI WcaCaScriptCleanup(
    __in LPCWSTR wzProductCode,
    __in BOOL fImpersonated
    )
{
    HRESULT hr = S_OK;
    WCHAR wzTempPath[MAX_PATH];
    LPWSTR pwzWildCardPath = NULL;
    WIN32_FIND_DATAW fd = { 0 };
    HANDLE hff = INVALID_HANDLE_VALUE;
    LPWSTR pwzDeletePath = NULL;

    if (fImpersonated)
    {
        if (!::GetTempPathW(countof(wzTempPath), wzTempPath))
        {
            ExitWithLastError(hr, L"Failed to get temp path.");
        }
    }
    else
    {
        if (!::GetWindowsDirectoryW(wzTempPath, countof(wzTempPath)))
        {
            ExitWithLastError(hr, L"Failed to get windows path.");
        }

        hr = ::StringCchCatW(wzTempPath, countof(wzTempPath), L"\\Installer\\");
        ExitOnFailure(hr, L"Failed to concat Installer directory on windows path string.");
    }

    hr = StrAllocFormatted(&pwzWildCardPath, L"%swix%s.*.???", wzTempPath, wzProductCode);
    ExitOnFailure(hr, L"Failed to allocate wildcard path to ca scripts.");

    hff = ::FindFirstFileW(pwzWildCardPath, &fd);
    if (INVALID_HANDLE_VALUE == hff)
    {
        ExitWithLastError1(hr, L"Failed to find files with pattern: %s", pwzWildCardPath);
    }

    do
    {
        hr = StrAllocFormatted(&pwzDeletePath, L"%s%s", wzTempPath, fd.cFileName);
        if (SUCCEEDED(hr))
        {
            if (!::DeleteFileW(pwzDeletePath))
            {
                DWORD er = ::GetLastError();
                WcaLog(LOGMSG_VERBOSE, L"Failed to clean up CAScript file: %s, er: %d", fd.cFileName, er);
            }
        }
        else
        {
            WcaLog(LOGMSG_VERBOSE, L"Failed to allocate path to clean up CAScript file: %s, hr: 0x%x", fd.cFileName, hr);
        }
    } while(::FindNextFileW(hff, &fd));

LExit:
    if (INVALID_HANDLE_VALUE == hff)
    {
        ::FindClose(hff);
    }

    ReleaseStr(pwzDeletePath);
    ReleaseStr(pwzWildCardPath);
    return;
}


static HRESULT CaScriptFileName(
    __in WCA_ACTION action,
    __in WCA_CASCRIPT script,
    __in BOOL fImpersonated,
    __in LPCWSTR wzScriptKey,
    __out LPWSTR* ppwzScriptName
    )
{
    HRESULT hr = S_OK;
    WCHAR wzTempPath[MAX_PATH];
    LPWSTR pwzProductCode = NULL;
    WCHAR chInstallOrUninstall = action == WCA_ACTION_INSTALL ? L'i' : L'u';
    WCHAR chScheduledOrRollback = script == WCA_CASCRIPT_SCHEDULED ? L's' : L'r';
    WCHAR chUserOrMachine = fImpersonated ? L'u' : L'm';

    if (fImpersonated)
    {
        if (!::GetTempPathW(countof(wzTempPath), wzTempPath))
        {
            ExitWithLastError(hr, L"Failed to get temp path.");
        }
    }
    else
    {
        if (!::GetWindowsDirectoryW(wzTempPath, countof(wzTempPath)))
        {
            ExitWithLastError(hr, L"Failed to get windows path.");
        }

        hr = ::StringCchCatW(wzTempPath, countof(wzTempPath), L"\\Installer\\");
        ExitOnFailure(hr, L"Failed to concat Installer directory on windows path string.");
    }

    hr = WcaGetProperty(L"ProductCode", &pwzProductCode);
    ExitOnFailure(hr, L"Failed to get ProductCode.");

    hr = StrAllocFormatted(ppwzScriptName, L"%swix%s.%s.%c%c%c", wzTempPath, pwzProductCode, wzScriptKey, chScheduledOrRollback, chUserOrMachine, chInstallOrUninstall);
    ExitOnFailure(hr, L"Failed to allocate path to ca script.");

LExit:
    ReleaseStr(pwzProductCode);
    return hr;
}
