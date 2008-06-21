//-------------------------------------------------------------------------------------------------
// <copyright file="wcawrap.cpp" company="Microsoft">
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
//    Windows Installer XML CustomAction utility library wrappers for MSI API
// </summary>
//-------------------------------------------------------------------------------------------------
// modified by z42:
// - get it compiled with VC8
// - consistently use Unicode for strings 

#include "precomp.h"


/********************************************************************
 WcaProcessMessage() - sends a message from the CustomAction

********************************************************************/
UINT WIXAPI WcaProcessMessage(
	__in INSTALLMESSAGE eMessageType,
	__in MSIHANDLE hRecord
	)
{
	UINT er = ::MsiProcessMessage(WcaGetInstallHandle(), eMessageType, hRecord);
	if (ERROR_INSTALL_USEREXIT == er || IDCANCEL == er)
		WcaSetReturnValue(ERROR_INSTALL_USEREXIT);

	return er;
}


/********************************************************************
 WcaErrorMessage() - sends an error message from the CustomAction using 
                     the Error table

 NOTE: Any and all var_args (...) must be WCHAR*
********************************************************************/
UINT WIXAPI WcaErrorMessage(
	__in int iError, 
	__in HRESULT hrError, 
	__in UINT uiType, 
	__in DWORD cArgs, 
	...
	)
{
	UINT er;
	MSIHANDLE hRec = NULL;
	va_list args;

	uiType |= INSTALLMESSAGE_ERROR;  // ensure error type is set
	hRec = ::MsiCreateRecord(cArgs + 2);
	if (!hRec)
	{
		er = ERROR_OUTOFMEMORY;
		ExitOnFailure(HRESULT_FROM_WIN32(er), L"failed to create record when sending error message");
	}

	er = ::MsiRecordSetInteger(hRec, 1, iError);
	ExitOnFailure(HRESULT_FROM_WIN32(er), L"failed to set error code into error message");

	er = ::MsiRecordSetInteger(hRec, 2, hrError);
	ExitOnFailure(HRESULT_FROM_WIN32(er), L"failed to set hresult code into error message");

	va_start(args, cArgs);
	for (DWORD i = 0; i < cArgs; i++)
	{
		er = ::MsiRecordSetStringW(hRec, i + 3, va_arg(args, WCHAR*));
		ExitOnFailure(HRESULT_FROM_WIN32(er), L"failed to set string string into error message");
	}
	va_end(args);

	er = WcaProcessMessage(static_cast<INSTALLMESSAGE>(uiType), hRec);
LExit:
	if (hRec)
		::MsiCloseHandle(hRec);

	return er;
}


/********************************************************************
 WcaProgressMessage() - extends the progress bar or sends a progress 
                        update from the CustomAction

********************************************************************/
HRESULT WIXAPI WcaProgressMessage(
	__in UINT uiCost,
	__in BOOL fExtendProgressBar
	)
{
	static BOOL fExplicitProgressMessages = FALSE;

	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;
	MSIHANDLE hRec = ::MsiCreateRecord(3);

	// if aren't extending the progress bar and we haven't switched into explicit message mode
	if (!fExtendProgressBar && !fExplicitProgressMessages)
	{
		AssertSz(::MsiGetMode(WcaGetInstallHandle(), MSIRUNMODE_SCHEDULED) ||
		         ::MsiGetMode(WcaGetInstallHandle(), MSIRUNMODE_COMMIT) ||
		         ::MsiGetMode(WcaGetInstallHandle(), MSIRUNMODE_ROLLBACK), L"can only send progress bar messages in a deferred CustomAction");

		// tell Darwin to use explicit progress messages
		::MsiRecordSetInteger(hRec, 1, 1);
		::MsiRecordSetInteger(hRec, 2, 1);
		::MsiRecordSetInteger(hRec, 3, 0);

		er = WcaProcessMessage(INSTALLMESSAGE_PROGRESS, hRec);
		if (0 == er || IDOK == er || IDYES == er)
			hr = S_OK;
		else if (IDABORT == er || IDCANCEL == er)
		{
			WcaSetReturnValue(ERROR_INSTALL_USEREXIT); // note that the user said exit
			ExitFunction1(hr = S_FALSE);
		}
		else
			hr = E_UNEXPECTED;
		ExitOnFailure(hr, L"failed to tell Darwin to use explicit progress messages");

		fExplicitProgressMessages = TRUE;
	}
#if DEBUG
	else if (fExtendProgressBar)   // if we are extending the progress bar, make sure we're not deferred
	{
		AssertSz(!::MsiGetMode(WcaGetInstallHandle(), MSIRUNMODE_SCHEDULED), L"cannot add ticks to progress bar length from deferred CustomAction");
	}
#endif

	// send the progress message
	::MsiRecordSetInteger(hRec, 1, (fExtendProgressBar) ? 3 : 2);
	::MsiRecordSetInteger(hRec, 2, uiCost);
	::MsiRecordSetInteger(hRec, 3, 0);

	er = WcaProcessMessage(INSTALLMESSAGE_PROGRESS, hRec);
	if (0 == er || IDOK == er || IDYES == er)
	{
		hr = S_OK;
	}
	else if (IDABORT == er || IDCANCEL == er)
	{
		WcaSetReturnValue(ERROR_INSTALL_USEREXIT); // note that the user said exit
		hr = S_FALSE;
	}
	else
		hr = E_UNEXPECTED;

LExit:
	if (hRec)
		::MsiCloseHandle(hRec);

	return hr;
}


/********************************************************************
 WcaIsInstalling() - determines if a pair of installstates means install

********************************************************************/
BOOL WIXAPI WcaIsInstalling(
	__in INSTALLSTATE isInstalled,
	__in INSTALLSTATE isAction
	)
{
	return (INSTALLSTATE_LOCAL == isAction ||
	        INSTALLSTATE_SOURCE == isAction ||
	        (INSTALLSTATE_DEFAULT == isAction &&
	         (INSTALLSTATE_LOCAL == isInstalled ||
	          INSTALLSTATE_SOURCE == isInstalled)));
}

/********************************************************************
 WcaIsReInstalling() - determines if a pair of installstates means install

********************************************************************/
BOOL WIXAPI WcaIsReInstalling(
	__in INSTALLSTATE isInstalled,
	__in INSTALLSTATE isAction
	)
{
	return ((INSTALLSTATE_LOCAL == isAction ||
			INSTALLSTATE_SOURCE == isAction ||
			INSTALLSTATE_DEFAULT == isAction) &&
			(INSTALLSTATE_LOCAL == isInstalled ||
			INSTALLSTATE_SOURCE == isInstalled));
}


/********************************************************************
 WcaIsUninstalling() - determines if a pair of installstates means uninstall

********************************************************************/
BOOL WIXAPI WcaIsUninstalling(
	__in INSTALLSTATE isInstalled,
	__in INSTALLSTATE isAction
	)
{
	return ((INSTALLSTATE_ABSENT == isAction ||
	         INSTALLSTATE_REMOVED == isAction) &&
	        (INSTALLSTATE_LOCAL == isInstalled ||
	         INSTALLSTATE_SOURCE == isInstalled));
}


/********************************************************************
 WcaGetComponentToDo() - gets a component's install states and 
    determines if they mean install, uninstall, or reinstall.
********************************************************************/
WCA_TODO WcaGetComponentToDo(
    __in LPCWSTR wzComponentId
    )
{
    INSTALLSTATE isInstalled = INSTALLSTATE_UNKNOWN;
    INSTALLSTATE isAction = INSTALLSTATE_UNKNOWN;
    if (ERROR_SUCCESS != ::MsiGetComponentStateW(WcaGetInstallHandle(), wzComponentId, &isInstalled, &isAction))
    {
        return WCA_TODO_UNKNOWN;
    }
    
    if (WcaIsReInstalling(isInstalled, isAction))
    {
        return WCA_TODO_REINSTALL;
    }
    else if (WcaIsUninstalling(isInstalled, isAction))
    {
        return WCA_TODO_UNINSTALL;
    }
    else if (WcaIsInstalling(isInstalled, isAction))
    {
        return WCA_TODO_INSTALL;
    }
    else
    {
        return WCA_TODO_UNKNOWN;
    }
}


/********************************************************************
 WcaSetComponentState() - sets the install state of a Component

********************************************************************/
HRESULT WIXAPI WcaSetComponentState(
	__in LPCWSTR wzComponent,
	__in INSTALLSTATE isState
	)
{
	UINT er = ::MsiSetComponentStateW(WcaGetInstallHandle(), wzComponent, isState);
	if (ERROR_INSTALL_USEREXIT == er)
		WcaSetReturnValue(er);

	return HRESULT_FROM_WIN32(er);
}


/********************************************************************
 WcaTableExists() - determines if installing database contains a table

********************************************************************/
HRESULT WIXAPI WcaTableExists(
	__in LPCWSTR wzTable
	)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;

// NOTE:  The following line of commented out code should work in a 
//        CustomAction but does not in Windows Installer v1.1
	// er = ::MsiDatabaseIsTablePersistentW(hDatabase, wzTable);

	// a L"most elegant" workaround a Darwin v1.1 bug
	PMSIHANDLE hRec;
	er = ::MsiDatabaseGetPrimaryKeysW(WcaGetDatabaseHandle(), wzTable, &hRec);

	if (ERROR_SUCCESS == er)
		hr = S_OK;
	else if (ERROR_INVALID_TABLE == er)
		hr = S_FALSE;
	else
		hr = E_FAIL;
	Assert(SUCCEEDED(hr));

	return hr;
}


/********************************************************************
 WcaOpenView() - opens a view on the installing database

********************************************************************/
HRESULT WIXAPI WcaOpenView(
	__in LPCWSTR wzSql,
	__out MSIHANDLE* phView
	)
{
	if (!wzSql || !*wzSql|| !phView)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ::MsiDatabaseOpenViewW(WcaGetDatabaseHandle(), wzSql, phView);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure1(hr, L"failed to open view on database with SQL: %s", wzSql);

LExit:
	return hr;
}


/********************************************************************
 WcaExecuteView() - executes a parameterized open view on the installing database

********************************************************************/
HRESULT WIXAPI WcaExecuteView(
	__in MSIHANDLE hView,
	__in MSIHANDLE hRec
	)
{
	if (!hView)
		return E_INVALIDARG;
	AssertSz(hRec, L"Use WcaOpenExecuteView() if you don't need to pass in a record");

	HRESULT hr = S_OK;
	UINT er = ::MsiViewExecute(hView, hRec);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to execute view");

LExit:
	return hr;
}


/********************************************************************
 WcaOpenExecuteView() - opens and executes a view on the installing database

********************************************************************/
HRESULT WIXAPI WcaOpenExecuteView(
	__in LPCWSTR wzSql,
	__out MSIHANDLE* phView
	)
{
	if (!wzSql || !*wzSql|| !phView)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ::MsiDatabaseOpenViewW(WcaGetDatabaseHandle(), wzSql, phView);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to open view on database");

	er = ::MsiViewExecute(*phView, NULL);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to execute view");

LExit:
	return hr;
}


/********************************************************************
 WcaFetchRecord() - gets the next record from a view on the installing database

********************************************************************/
HRESULT WIXAPI WcaFetchRecord(
	__in MSIHANDLE hView,
	__out MSIHANDLE* phRec
	)
{
	if (!hView|| !phRec)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ::MsiViewFetch(hView, phRec);
	hr = HRESULT_FROM_WIN32(er);
	if (FAILED(hr) && E_NOMOREITEMS != hr)
		ExitOnFailure(hr, L"failed to fetch record from view");

LExit:
	return hr;
}


/********************************************************************
 WcaFetchSingleRecord() - gets a single record from a view on the installing database

********************************************************************/
HRESULT WIXAPI WcaFetchSingleRecord(
	__in MSIHANDLE hView,
	__out MSIHANDLE* phRec
	)
{
	if (!hView|| !phRec)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ::MsiViewFetch(hView, phRec);
	if (ERROR_NO_MORE_ITEMS == er)
		hr = S_FALSE;
	else
		hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to fetch single record from view");

#ifdef DEBUG // only do this in debug to verify that a single record was returned
	MSIHANDLE hRecTest;
	er = ::MsiViewFetch(hView, &hRecTest);
	AssertSz(ERROR_NO_MORE_ITEMS == er && NULL == hRecTest, L"WcaSingleFetch() did not fetch a single record");
	::MsiCloseHandle(hRecTest);
#endif

LExit:
	return hr;
}


/********************************************************************
 WcaGetProperty - gets a string property value from the active install

********************************************************************/
HRESULT WIXAPI WcaGetProperty(
	__in LPCWSTR wzProperty,
	__inout LPWSTR* ppwzData
	)
{
	if (!wzProperty || !*wzProperty || !ppwzData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;
	DWORD_PTR cch = 0;

	if (!*ppwzData)
	{
		WCHAR szEmpty[1] = L"";
		er = ::MsiGetPropertyW(WcaGetInstallHandle(), wzProperty, szEmpty, (DWORD *)&cch);
		if (ERROR_MORE_DATA == er || ERROR_SUCCESS == er)
		{
			hr = StrAlloc(ppwzData, ++cch);
		}
		else
			hr = HRESULT_FROM_WIN32(er);
		ExitOnFailure1(hr, L"Failed to allocate string for Property '%s'", wzProperty);
	}
	else
	{
		hr = StrMaxLength(*ppwzData, &cch);
		ExitOnFailure(hr, L"Failed to get previous size of property data string.");
	}

	er = ::MsiGetPropertyW(WcaGetInstallHandle(), wzProperty, *ppwzData, (DWORD *)&cch);
	if (ERROR_MORE_DATA == er)
	{
		Assert(*ppwzData);
		hr = StrAlloc(ppwzData, ++cch);
		ExitOnFailure1(hr, L"Failed to allocate string for Property '%s'", wzProperty);

		er = ::MsiGetPropertyW(WcaGetInstallHandle(), wzProperty, *ppwzData, (DWORD *)&cch);
	}
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure1(hr, L"Failed to get data for property '%s'", wzProperty);

LExit:
	return hr;
}


/********************************************************************
 WcaGetFormattedProperty - gets a formatted string property value from 
                           the active install

********************************************************************/
HRESULT WIXAPI WcaGetFormattedProperty(
	__in LPCWSTR wzProperty,
	__out LPWSTR* ppwzData
	)
{
	if (!wzProperty || !*wzProperty || !ppwzData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	LPWSTR pwzPropertyValue = NULL;
	

	hr = WcaGetProperty(wzProperty, &pwzPropertyValue);
	ExitOnFailure1(hr, L"failed to get %s", wzProperty);

	hr = WcaGetFormattedString(pwzPropertyValue, ppwzData);
	ExitOnFailure2(hr, L"failed to get formatted value for property: '%s' with value: '%s'", wzProperty, pwzPropertyValue);

LExit:
	ReleaseStr(pwzPropertyValue);
	return hr;
}


/********************************************************************
 WcaGetFormattedString - gets a formatted string value from 
                           the active install

********************************************************************/
HRESULT WIXAPI WcaGetFormattedString(
	__in LPCWSTR wzString,
	__out LPWSTR* ppwzData
	)
{
	if (!wzString || !*wzString || !ppwzData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;
	PMSIHANDLE hRecord = ::MsiCreateRecord(1);
	DWORD_PTR cch = 0;

	er = ::MsiRecordSetStringW(hRecord, 0, wzString);
	ExitOnFailure1(hr = HRESULT_FROM_WIN32(er), L"Failed to set record field 0 with '%s'", wzString);

	if (!*ppwzData)
	{
		WCHAR szEmpty[1] = L"";
		er = ::MsiFormatRecordW(WcaGetInstallHandle(), hRecord, szEmpty, (DWORD *)&cch);
		if (ERROR_MORE_DATA == er || ERROR_SUCCESS == er)
		{
			hr = StrAlloc(ppwzData, ++cch);
		}
		else
			hr = HRESULT_FROM_WIN32(er);
		ExitOnFailure1(hr, L"Failed to allocate string for formatted string: '%s'", wzString);
	}
	else
	{
		hr = StrMaxLength(*ppwzData, &cch);
		ExitOnFailure(hr, L"Failed to get previous size of property data string");
	}

	er = ::MsiFormatRecordW(WcaGetInstallHandle(), hRecord, *ppwzData, (DWORD *)&cch);
	if (ERROR_MORE_DATA == er)
	{
		hr = StrAlloc(ppwzData, ++cch);
		ExitOnFailure1(hr, L"Failed to allocate string for formatted string: '%s'", wzString);

		er = ::MsiFormatRecordW(WcaGetInstallHandle(), hRecord, *ppwzData, (DWORD *)&cch);
	}
	ExitOnFailure1(hr = HRESULT_FROM_WIN32(er), L"Failed to get formatted string: '%s'", wzString);

LExit:
	return hr;
}


/********************************************************************
 WcaGetIntProperty - gets an integer property value from the active install

********************************************************************/
HRESULT WIXAPI WcaGetIntProperty(
	__in LPCWSTR wzProperty,
	__inout int* piData
	)
{
	if (!piData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er;

	WCHAR wzValue[32];
	DWORD cch = countof(wzValue) - 1;

	er = ::MsiGetPropertyW(WcaGetInstallHandle(), wzProperty, wzValue, &cch);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure1(hr, L"Failed to get data for property '%s'", wzProperty);

	*piData = wcstol(wzValue, NULL, 10);

LExit:
	return hr;
}


/********************************************************************
 WcaGetTargetPath - gets the target path for a specified folder

********************************************************************/
HRESULT WIXAPI WcaGetTargetPath(
	__in LPCWSTR wzFolder,
	__out LPWSTR* ppwzData
	)
{
	if (!wzFolder || !*wzFolder || !ppwzData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	
	UINT er = ERROR_SUCCESS;
	DWORD_PTR cch = 0;

	if (!*ppwzData)
	{
		WCHAR szEmpty[1] = L"";
		er = ::MsiGetTargetPathW(WcaGetInstallHandle(), wzFolder, szEmpty, (DWORD*)&cch);
		if (ERROR_MORE_DATA == er || ERROR_SUCCESS == er)
		{
			cch++; //Add one for the null terminator
			hr = StrAlloc(ppwzData, cch);
		}
		else
			hr = HRESULT_FROM_WIN32(er);
		ExitOnFailure1(hr, L"Failed to allocate string for target path of folder: '%s'", wzFolder);
	}
	else
	{
		hr = StrMaxLength(*ppwzData, &cch);
		ExitOnFailure(hr, L"Failed to get previous size of string");
	}

	er = ::MsiGetTargetPathW(WcaGetInstallHandle(), wzFolder, *ppwzData, (DWORD*)&cch);
	if (ERROR_MORE_DATA == er)
	{
		cch++;
		hr = StrAlloc(ppwzData, cch);
		ExitOnFailure1(hr, L"Failed to allocate string for target path of folder: '%s'", wzFolder);

		er = ::MsiGetTargetPathW(WcaGetInstallHandle(), wzFolder, *ppwzData, (DWORD*)&cch);
	}
	ExitOnFailure1(hr = HRESULT_FROM_WIN32(er), L"Failed to get target path for folder '%s'", wzFolder);

LExit:
	return hr;
}


/********************************************************************
 WcaSetProperty - sets a string property value in the active install

********************************************************************/
HRESULT WIXAPI WcaSetProperty(
	__in LPCWSTR wzPropertyName,
	__in LPCWSTR wzPropertyValue
	)
{
	if (!wzPropertyName || !*wzPropertyName || !wzPropertyValue)
		return E_INVALIDARG;

	UINT er = ::MsiSetPropertyW(WcaGetInstallHandle(), wzPropertyName, wzPropertyValue);
	HRESULT hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure1(hr, L"failed to set property: %s", wzPropertyName);

LExit:
	return hr;
}


/********************************************************************
 WcaSetIntProperty - sets a integer property value in the active install

********************************************************************/
HRESULT WIXAPI WcaSetIntProperty(
	__in LPCWSTR wzPropertyName,
	__in int nPropertyValue
	)
{
	if (!wzPropertyName || !*wzPropertyName)
		return E_INVALIDARG;

	// 12 characters should be enough for a 32-bit int: 10 digits, 1 sign, 1 null
	WCHAR wzPropertyValue[13];
	HRESULT hr = StringCchPrintfW(wzPropertyValue, countof(wzPropertyValue), L"%d", nPropertyValue);
	ExitOnFailure1(hr, L"failed to convert into string property value: %d", nPropertyValue);

	UINT er = ::MsiSetPropertyW(WcaGetInstallHandle(), wzPropertyName, wzPropertyValue);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure1(hr, L"failed to set property: %s", wzPropertyName);

LExit:
	return hr;
}


/********************************************************************
 WcaIsPropertySet() - returns TRUE if property is set

********************************************************************/
BOOL WIXAPI WcaIsPropertySet(
	__in LPCWSTR szProperty
	)
{
	DWORD cchProperty = 0;
	WCHAR szEmpty[1] = L"";
#ifdef DEBUG
	UINT er =
#endif
            ::MsiGetPropertyW(WcaGetInstallHandle(), szProperty, szEmpty, &cchProperty);
	AssertSz(ERROR_INVALID_PARAMETER != er && ERROR_INVALID_HANDLE != er, L"Unexpected return value from ::MsiGetProperty()");

	return 0 < cchProperty; // property is set if the length is greater than zero
}


/********************************************************************
 WcaGetRecordInteger() - gets an integer field out of a record

 NOTE: returns S_FALSE if the field was null
********************************************************************/
HRESULT WIXAPI WcaGetRecordInteger(
	__in MSIHANDLE hRec,
	__in UINT uiField,
	__inout int* piData
	)
{
	if (!hRec || !piData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	*piData = ::MsiRecordGetInteger(hRec, uiField);
	if (MSI_NULL_INTEGER == *piData)
		hr = S_FALSE;

//LExit:
	return hr;
}


/********************************************************************
 WcaGetRecordString() - gets a string field out of a record

********************************************************************/
HRESULT WIXAPI WcaGetRecordString(
	__in MSIHANDLE hRec,
	__in UINT uiField,
	__inout LPWSTR* ppwzData
	)
{
	if (!hRec || !ppwzData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er;
	DWORD_PTR cch = 0;

	if (!*ppwzData)
	{
		WCHAR szEmpty[1] = L"";
		er = ::MsiRecordGetStringW(hRec, uiField, szEmpty, (DWORD*)&cch);
		if (ERROR_MORE_DATA == er || ERROR_SUCCESS == er)
		{
			hr = StrAlloc(ppwzData, ++cch);
		}
		else
			hr = HRESULT_FROM_WIN32(er);
		ExitOnFailure(hr, L"Failed to allocate memory for record string");
	}
	else
	{
		hr = StrMaxLength(*ppwzData, &cch);
		ExitOnFailure(hr, L"Failed to get previous size of string");
	}

	er = ::MsiRecordGetStringW(hRec, uiField, *ppwzData, (DWORD*)&cch);
	if (ERROR_MORE_DATA == er)
	{
		hr = StrAlloc(ppwzData, ++cch);
		ExitOnFailure(hr, L"Failed to allocate memory for record string");

		er = ::MsiRecordGetStringW(hRec, uiField, *ppwzData, (DWORD*)&cch);
	}
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"Failed to get string from record");

LExit:
	return hr;
}


/********************************************************************
 HideNulls() - internal helper function to escape [~] in formatted strings

********************************************************************/
static void HideNulls(
	__in LPWSTR wzData
	)
{
	LPWSTR pwz = wzData;

	while(*pwz)
	{
		if (pwz[0] == L'[' && pwz[1] == L'~' && pwz[2] == L']') // found a null [~]
		{
			pwz[0] = L'!'; // turn it into !$!
			pwz[1] = L'$';
			pwz[2] = L'!';
			pwz += 3;
		}
		else
			pwz++;
	}
}


/********************************************************************
 RevealNulls() - internal helper function to unescape !$! in formatted strings

********************************************************************/
static void RevealNulls(
	__in LPWSTR wzData
	)
{
	LPWSTR pwz = wzData;

	while(*pwz)
	{
		if (pwz[0] == L'!' && pwz[1] == L'$' && pwz[2] == L'!') // found the fake null !$!
		{
			pwz[0] = L'['; // turn it back into [~]
			pwz[1] = L'~';
			pwz[2] = L']';
			pwz += 3;
		}
		else
			pwz++;
	}
}


/********************************************************************
 WcaGetRecordFormattedString() - gets formatted string filed from record

********************************************************************/
HRESULT WIXAPI WcaGetRecordFormattedString(
	__in MSIHANDLE hRec,
	__in UINT uiField,
	__inout LPWSTR* ppwzData
	)
{
	if (!hRec || !ppwzData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er;
	DWORD_PTR cch = 0;
	PMSIHANDLE hRecFormat;

	// get the format string
	hr = WcaGetRecordString(hRec, uiField, ppwzData);
	ExitOnFailure(hr, L"failed to get string from record");

	if (!**ppwzData)
		ExitFunction();

	// hide the nulls '[~]' so we can get them back after formatting
	HideNulls(*ppwzData);

	// set up the format record
	hRecFormat = ::MsiCreateRecord(1);
	ExitOnNull(hRecFormat, hr, E_UNEXPECTED, L"Failed to create record to format string");
	hr = WcaSetRecordString(hRecFormat, 0, *ppwzData);
	ExitOnFailure(hr, L"failed to set string to format record");

	// format the string
	hr = StrMaxLength(*ppwzData, &cch);
	ExitOnFailure(hr, L"failed to get max length of string");

	er = ::MsiFormatRecordW(WcaGetInstallHandle(), hRecFormat, *ppwzData, (DWORD*)&cch);
	if (ERROR_MORE_DATA == er)
	{
		hr = StrAlloc(ppwzData, ++cch);
		ExitOnFailure(hr, L"Failed to allocate memory for record string");

		er = ::MsiFormatRecordW(WcaGetInstallHandle(), hRecFormat, *ppwzData, (DWORD*)&cch);
	}
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"Failed to format string");

	// put the nulls back
	RevealNulls(*ppwzData);

LExit:
	return hr;
}


/********************************************************************
 WcaAllocStream() - creates a byte stream of the specified size

 NOTE: Use WcaFreeStream() to release the byte stream
********************************************************************/
HRESULT WIXAPI WcaAllocStream(
	__inout BYTE** ppbData,
	__in DWORD cbData
	)
{
	Assert(ppbData);
	HRESULT hr;
	BYTE* pbNewData;

	if (*ppbData)
		pbNewData = (BYTE*)MemReAlloc(*ppbData, cbData, TRUE);
	else
		pbNewData = (BYTE*)MemAlloc(cbData, TRUE);

	if (!pbNewData)
		ExitOnLastError(hr, L"Failed to allocate string");
	*ppbData = pbNewData;
	pbNewData = NULL;

	hr = S_OK;
LExit:
	if (pbNewData)
		MemFree(pbNewData);

	return hr;
}


/********************************************************************
 WcaFreeStream() - frees a byte stream

********************************************************************/
HRESULT WIXAPI WcaFreeStream(
	__in BYTE* pbData
	)
{
	if (!pbData)
		return E_INVALIDARG;

	HRESULT hr = MemFree(pbData);
	return hr;
}


/********************************************************************
 WcaReadRecordStream() - gets a byte stream field from record

********************************************************************/
HRESULT WIXAPI WcaGetRecordStream(
	__in MSIHANDLE hRecBinary,
	__in UINT uiField, 
	__inout BYTE** ppbData,
	__inout DWORD* pcbData
	)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;

	if (!hRecBinary || !ppbData || !pcbData)
		return E_INVALIDARG;

	*pcbData = 0;
	er = ::MsiRecordReadStream(hRecBinary, uiField, NULL, pcbData);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to get size of stream");

	hr = WcaAllocStream(ppbData, *pcbData);
	ExitOnFailure(hr, L"failed to allocate data for stream");

	er = ::MsiRecordReadStream(hRecBinary, uiField, (char*)*ppbData, pcbData);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to read from stream");

LExit:
	return hr;
}


/********************************************************************
 WcaSetRecordString() - set a string field in record

********************************************************************/
HRESULT WIXAPI WcaSetRecordString(
	__in MSIHANDLE hRec,
	__in UINT uiField,
	__in LPCWSTR wzData
	)
{
	if (!hRec || !wzData)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ::MsiRecordSetStringW(hRec, uiField, wzData);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to set string in record");

LExit:
	return hr;
}


/********************************************************************
 WcaSetRecordInteger() - set a integer field in record

********************************************************************/
HRESULT WIXAPI WcaSetRecordInteger(
	__in MSIHANDLE hRec,
	__in UINT uiField,
	__in int iValue
	)
{
	if (!hRec)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	UINT er = ::MsiRecordSetInteger(hRec, uiField, iValue);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"failed to set integer in record");

LExit:
	return hr;
}


/********************************************************************

 WcaDoDeferredAction() - schedules an action at this point in the script

********************************************************************/
HRESULT WIXAPI WcaDoDeferredAction(
	__in LPCWSTR wzAction,
	__in LPCWSTR wzCustomActionData,
	__in UINT uiCost
	)
{
	HRESULT hr = S_OK;
	UINT er;

	if (wzCustomActionData && *wzCustomActionData)
	{
		er = ::MsiSetPropertyW(WcaGetInstallHandle(), wzAction, wzCustomActionData);
		hr = HRESULT_FROM_WIN32(er);
		ExitOnFailure(hr, L"Failed to set CustomActionData for deferred action");
	}

	if (0 < uiCost)
	{
		hr = WcaProgressMessage(uiCost, TRUE);  // add ticks to the progress bar
		// TODO: handle the return codes correctly
	}

	er = ::MsiDoActionW(WcaGetInstallHandle(), wzAction);
	hr = HRESULT_FROM_WIN32(er);
	ExitOnFailure(hr, L"Failed MsiDoAction on deferred action");

LExit:
	return hr;
}


/********************************************************************
 WcaCountOfCustomActionDataRecords() - counts the number of records 
                                       passed to a deferred CustomAction

********************************************************************/
DWORD WIXAPI WcaCountOfCustomActionDataRecords(
	__in LPCWSTR wzData
	)
{
	WCHAR delim[] = {MAGIC_MULTISZ_DELIM, 0}; // magic char followed by NULL terminator
	DWORD dwCount = 0;

	// Loop through until there are no delimiters, we are at the end of the string, or the delimiter is the last character in the string
	for (LPCWSTR pwzCurrent = wzData; pwzCurrent && *pwzCurrent && *(pwzCurrent + 1); pwzCurrent = wcsstr(pwzCurrent, delim))
	{
		dwCount++;
		pwzCurrent++;
	}

	return dwCount;
}


/********************************************************************
 BreakDownCustomActionData() - internal helper to chop up CustomActionData

 NOTE: this modifies the passed in data
********************************************************************/
static LPWSTR BreakDownCustomActionData(
	__inout LPWSTR* ppwzData
	)
{
	if (!ppwzData)
		return NULL;
	if (0 == *ppwzData)
		return NULL;

	WCHAR delim[] = {MAGIC_MULTISZ_DELIM, 0}; // magic char followed by Null terminator

	LPWSTR pwzReturn = *ppwzData;
	LPWSTR pwz = wcsstr(pwzReturn, delim);
	if (pwz)
	{
		*pwz = 0;
		*ppwzData = pwz + 1;
	}
	else
		*ppwzData = 0;

	return pwzReturn;
}


/********************************************************************
 WcaReadStringFromCaData() - reads a string out of the CustomActionData

 NOTE: this modifies the passed in ppwzCustomActionData variable
********************************************************************/
HRESULT WIXAPI WcaReadStringFromCaData(
	__inout LPWSTR* ppwzCustomActionData,
	__inout LPWSTR* ppwzString
	)
{
	HRESULT hr = S_OK;

	LPCWSTR pwz = BreakDownCustomActionData(ppwzCustomActionData);
	if (!pwz)
		return E_NOMOREITEMS;

	hr = StrAllocString(ppwzString, pwz, 0);
	ExitOnFailure(hr, L"failed to allocate memory for string");

	hr  = S_OK;
LExit:
	return hr;
}


/********************************************************************
 WcaReadIntegerFromCaData() - reads an integer out of the CustomActionData

 NOTE: this modifies the passed in ppwzCustomActionData variable
********************************************************************/
HRESULT WIXAPI WcaReadIntegerFromCaData(
	__inout LPWSTR* ppwzCustomActionData,
	__inout int* piResult
	)
{
	LPCWSTR pwz = BreakDownCustomActionData(ppwzCustomActionData);
	if (!pwz)
		return E_NOMOREITEMS;

	*piResult = wcstol(pwz, NULL, 10);
	return S_OK;
}


/********************************************************************
 WcaReadStreamFromCaData() - reads a stream out of the CustomActionData

 NOTE: this modifies the passed in ppwzCustomActionData variable
 NOTE: returned stream should be freed with WcaFreeStream()
********************************************************************/
HRESULT WIXAPI WcaReadStreamFromCaData(
	__inout LPWSTR* ppwzCustomActionData,
	__out BYTE** ppbData,
	__out DWORD_PTR* pcbData
	)
{
	HRESULT hr;

	LPCWSTR pwz = BreakDownCustomActionData(ppwzCustomActionData);
	if (!pwz)
		return E_NOMOREITEMS;

	hr = StrAllocBase85Decode(pwz, ppbData, pcbData);
	ExitOnFailure(hr, L"failed to decode string into stream");

LExit:
	return hr;
}


/********************************************************************
 WcaWriteStringToCaData() - adds a string to the CustomActionData to 
                            feed a deferred CustomAction

********************************************************************/
HRESULT WIXAPI WcaWriteStringToCaData(
	__in LPCWSTR wzString,
	__inout LPWSTR* ppwzCustomActionData
	)
{
	HRESULT hr = S_OK;
	WCHAR delim[] = {MAGIC_MULTISZ_DELIM, 0}; // magic char followed by NULL terminator

	if (!ppwzCustomActionData)
		return E_INVALIDARG;

	DWORD cchString = lstrlenW(wzString) + 1; // assume we'll be adding the delim
	DWORD_PTR cchCustomActionData = 0;

	if (*ppwzCustomActionData)
	{
		hr = StrMaxLength(*ppwzCustomActionData, &cchCustomActionData);
		ExitOnFailure(hr, L"failed to get length of custom action data");
	}

	if ((cchCustomActionData - lstrlenW(*ppwzCustomActionData)) < cchString + 1)
	{
		cchCustomActionData += cchString + 1 + 255;  // add 255 for good measure
		hr = StrAlloc(ppwzCustomActionData, cchCustomActionData);
		ExitOnFailure(hr, L"Failed to allocate memory for CustomActionData string");
		ExitOnNull(*ppwzCustomActionData, hr, E_OUTOFMEMORY, L"Failed to allocate memory for CustomActionData string");
	}

	if (**ppwzCustomActionData) // if data exists toss the delimiter on before adding more to the end
		StringCchCatW(*ppwzCustomActionData, cchCustomActionData, delim);
	StringCchCatW(*ppwzCustomActionData, cchCustomActionData, wzString);

LExit:
	return hr;
}


/********************************************************************
 WcaWriteStringToCaData() - adds an integer to the CustomActionData to 
                            feed a deferred CustomAction

********************************************************************/
HRESULT WIXAPI WcaWriteIntegerToCaData(
	__in int i, 
	__inout LPWSTR* ppwzCustomActionData
	)
{
	WCHAR wzBuffer[13];
	StringCchPrintfW(wzBuffer, countof(wzBuffer), L"%d", i);

	return WcaWriteStringToCaData(wzBuffer, ppwzCustomActionData);
}


/********************************************************************
 WcaWriteStringToCaData() - adds a byte stream to the CustomActionData to 
                            feed a deferred CustomAction

********************************************************************/
HRESULT WIXAPI WcaWriteStreamToCaData(
	__in_bcount(cbData) const BYTE* pbData,
	__in DWORD cbData,
	__inout LPWSTR* ppwzCustomActionData
	)
{
	HRESULT hr;
	LPWSTR pwzData = NULL;

	hr = StrAllocBase85Encode(pbData, cbData, &pwzData);
	ExitOnFailure(hr, L"failed to encode data into string");

	hr = WcaWriteStringToCaData(pwzData, ppwzCustomActionData);

LExit:
	ReleaseStr(pwzData);
	return hr;
}


/********************************************************************
WcaAddTempRecord - adds a temporary record to the active database

NOTE: you cannot use PMSIHANDLEs for the __in/__out parameters
	comment by zett42: it seems to work, and why shouldn't it?
	
NOTE: uiUniquifyColumn can be 0 if no column needs to be made unique
********************************************************************/
HRESULT WIXAPI WcaAddTempRecord(
    __inout MSIHANDLE* phTableView,
    __inout MSIHANDLE* phColumns,
    __in LPCWSTR wzTable,
    __out_opt MSIDBERROR* pdbError,
    __in UINT uiUniquifyColumn,
    __in UINT cColumns,
    ...
    )
{
	va_list args;
	va_start( args, cColumns );
	HRESULT hr = WcaAddTempRecordV( phTableView, phColumns, wzTable, pdbError, uiUniquifyColumn, cColumns, args );
	va_end( args );
	return hr;
}

/********************************************************************
WcaAddTempRecordV - same as WcaAddTempRecord, but with va_list parameter
********************************************************************/

HRESULT WIXAPI WcaAddTempRecordV(
    __inout MSIHANDLE* phTableView,
    __inout MSIHANDLE* phColumns,
    __in LPCWSTR wzTable,
    __out_opt MSIDBERROR* pdbError,
    __in UINT uiUniquifyColumn,
    __in UINT cColumns,
    va_list args
    )
{
    Assert(phTableView && phColumns);

    static DWORD dwUniquifyValue = ::GetTickCount();

    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    LPWSTR pwzQuery = NULL;
    PMSIHANDLE hTempRec;
    DWORD i;

    LPWSTR pwzData = NULL;
    LPWSTR pwzUniquify = NULL;

    //
    // if we don't have a table and its columns already
    //
    if (NULL == *phTableView)
    {
        // set the query
        hr = StrAllocFormatted(&pwzQuery, L"SELECT * FROM `%s`",wzTable);
        ExitOnFailure(hr, L"failed to allocate string for query");

        // Open and Execute the temp View
        hr = WcaOpenExecuteView(pwzQuery, phTableView);
        ExitOnFailure1(hr, L"failed to openexecute temp view with query %s", pwzQuery);
    }

    if (NULL == *phColumns)
    {
        // use GetColumnInfo to populate the datatype record
        er = ::MsiViewGetColumnInfo(*phTableView, MSICOLINFO_TYPES, phColumns);
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure1(hr, L"failed to columns for table: %s", wzTable);
    }
    AssertSz(::MsiRecordGetFieldCount(*phColumns) == cColumns, L"passed in argument does not match number of columns in table");

    //
    // create the temp record
    //
    hTempRec = ::MsiCreateRecord(cColumns);
    ExitOnNull1(hTempRec, hr, E_UNEXPECTED, L"could not create temp record for table: %s", wzTable);

    //
    // loop through all the columns filling in the data
    //
    for (i = 1; i <= cColumns; i++)
    {
        hr = WcaGetRecordString(*phColumns, i, &pwzData);
        ExitOnFailure1(hr, L"failed to get the data type for %d", i);

        // if data type is string write string
        if (L's' == *pwzData || L'S' == *pwzData || L'g' == *pwzData || L'G' == *pwzData || L'l' == *pwzData || L'L' == *pwzData)
        {
            LPCWSTR wz = va_arg(args, WCHAR*);

            // if this is the column that is supposed to be unique add the time stamp on the end
            if (uiUniquifyColumn == i)
            {
                hr = StrAllocFormatted(&pwzUniquify, L"%s%u", wz, ++dwUniquifyValue);   // up the count so we have no collisions on the unique name
                ExitOnFailure1(hr, L"failed to allocate string for unique column: %d", uiUniquifyColumn);

                wz = pwzUniquify;
            }

            er = ::MsiRecordSetStringW(hTempRec, i, wz);
            hr = HRESULT_FROM_WIN32(er);
            ExitOnFailure1(hr, L"failed to set string value at position %d", i);
        }
        // if data type is integer write integer
        else if (L'i' == *pwzData || L'I' == *pwzData || L'j' == *pwzData || L'J' == *pwzData)
        {
            AssertSz(uiUniquifyColumn != i, L"Cannot uniquify an integer column");
            int iData = va_arg(args, int);

            er = ::MsiRecordSetInteger(hTempRec, i, iData);
            hr = HRESULT_FROM_WIN32(er);
            ExitOnFailure1(hr, L"failed to set integer value at position %d", i);
        }
        else
        {
            // not supporting binary streams so error out
            hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
            ExitOnFailure2(hr, L"unsupported data type '%s' in column: %d", pwzData, i);
        }
    }

    //
    // add the temporary record to the MSI
    //
    er = ::MsiViewModify(*phTableView, MSIMODIFY_INSERT_TEMPORARY, hTempRec);
    hr = HRESULT_FROM_WIN32(er);
    if (FAILED(hr))
    {
        if (pdbError)
        {
            // MSI provides only a generic ERROR_FUNCTION_FAILED if a temporary row
            // can't be inserted; if we're being asked to provide the detailed error,
            // get it using the MSIMODIFY_VALIDATE_NEW flag
            er = ::MsiViewModify(*phTableView, MSIMODIFY_VALIDATE_NEW, hTempRec);
            hr = HRESULT_FROM_WIN32(er);
        }

        WCHAR wzBuf[MAX_PATH];
        DWORD cchBuf = countof(wzBuf);
        MSIDBERROR dbErr = ::MsiViewGetErrorW(*phTableView, wzBuf, &cchBuf);
        if (pdbError)
        {
            *pdbError = dbErr;
        }
        ExitOnFailure2(hr, L"failed to add temporary row, dberr: %d, err: %s", dbErr, wzBuf);
    }

LExit:
    ReleaseStr(pwzUniquify);
    ReleaseStr(pwzData);
    ReleaseStr(pwzQuery);

    return hr;
}


/********************************************************************
WcaDumpTable - dumps a table to the log file

********************************************************************/
HRESULT WIXAPI WIXAPI WcaDumpTable(
    __in LPCWSTR wzTable
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    LPWSTR pwzQuery = NULL;
    PMSIHANDLE hView;
    PMSIHANDLE hColumns;
    DWORD cColumns = 0;
    PMSIHANDLE hRec;

    LPWSTR pwzData = NULL;
    LPWSTR pwzPrint = NULL;

    hr = StrAllocFormatted(&pwzQuery, L"SELECT * FROM `%s`",wzTable);
    ExitOnFailure(hr, L"failed to allocate string for query");

    // Open and Execute the temp View
    hr = WcaOpenExecuteView(pwzQuery, &hView);
    ExitOnFailure1(hr, L"failed to openexecute temp view with query %s", pwzQuery);

    // Use GetColumnInfo to populate the names of the columns.
    er = ::MsiViewGetColumnInfo(hView, MSICOLINFO_NAMES, &hColumns);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure1(hr, L"failed to columns for table: %s", wzTable);

    cColumns = ::MsiRecordGetFieldCount(hColumns);

    WcaLog(LOGMSG_STANDARD, L"--- Begin Table Dump %s ---", wzTable);

    // Loop through all the columns filling in the data.
    for (DWORD i = 1; i <= cColumns; i++)
    {
        hr = WcaGetRecordString(hColumns, i, &pwzData);
        ExitOnFailure1(hr, L"failed to get the column name for %d", i);

        hr = StrAllocConcat(&pwzPrint, pwzData, 0);
        ExitOnFailure(hr, L"Failed to add column name.");

        hr = StrAllocConcat(&pwzPrint, L"\t", 1);
        ExitOnFailure(hr, L"Failed to add column name.");
    }

    WcaLog(LOGMSG_STANDARD, L"%s", pwzPrint);

    // Now dump the actual rows.
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        if (pwzPrint && *pwzPrint)
        {
            *pwzPrint = L'\0';
        }

        for (DWORD i = 1; i <= cColumns; i++)
        {
            hr = WcaGetRecordString(hRec, i, &pwzData);
            ExitOnFailure1(hr, L"failed to get the column name for %d", i);

            hr = StrAllocConcat(&pwzPrint, pwzData, 0);
            ExitOnFailure(hr, L"Failed to add column name.");

            hr = StrAllocConcat(&pwzPrint, L"\t", 1);
            ExitOnFailure(hr, L"Failed to add column name.");
        }

        WcaLog(LOGMSG_STANDARD, L"%s", pwzPrint);
    }

    WcaLog(LOGMSG_STANDARD, L"--- End Table Dump %s ---", wzTable);

LExit:
    ReleaseStr(pwzPrint);
    ReleaseStr(pwzData);
    ReleaseStr(pwzQuery);

    return hr;
}


HRESULT WIXAPI WcaDeferredActionRequiresReboot()
{
    HRESULT hr = S_OK;
    ATOM atomReboot = 0;

    atomReboot = ::GlobalAddAtomW(L"WcaDeferredActionRequiresReboot");
    ExitOnNullWithLastError(atomReboot, hr, L"Failed to create WcaDeferredActionRequiresReboot global atom.");

LExit:
    return hr;
}


BOOL WIXAPI WcaDidDeferredActionRequireReboot()
{
    // NOTE: This function does not delete the global atom.  That is done
    // purposefully so that any other installs that occur after this point also
    // require a reboot.
    ATOM atomReboot = ::GlobalFindAtomW(L"WcaDeferredActionRequiresReboot");
    return 0 != atomReboot;
}