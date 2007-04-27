
#if !defined(AFX_NTKERNELUTILS_H__C585FB9E_3931_4B7D_AA09_8D8E0A2568F5__INCLUDED_)
#define AFX_NTKERNELUTILS_H__C585FB9E_3931_4B7D_AA09_8D8E0A2568F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

//-------------------------------------------------------------------------------------------------

bool MapNtFilePathToUserPath( LPTSTR pUserPath, unsigned userPathLen, LPCTSTR ntPath );
bool MapUserPathToNtFilePath( LPTSTR pNtPath, unsigned ntPathLen, LPCTSTR userPath );

//-------------------------------------------------------------------------------------------------

#endif // !defined(AFX_NTKERNELUTILS_H__C585FB9E_3931_4B7D_AA09_8D8E0A2568F5__INCLUDED_)
