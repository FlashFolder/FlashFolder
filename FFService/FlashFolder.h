// FlashFolder.h : Haupt-Header-Datei f�r die Anwendung FLASHFOLDER
//

#if !defined(AFX_FLASHFOLDER_H__C5E276C1_460A_11D8_B7CB_9B1B4EA90ABA__INCLUDED_)
#define AFX_FLASHFOLDER_H__C5E276C1_460A_11D8_B7CB_9B1B4EA90ABA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CFlashFolderApp:
// Siehe FlashFolder.cpp f�r die Implementierung dieser Klasse
//

class CFlashFolderApp : public CWinApp
{
public:
	CFlashFolderApp();

// �berladungen
	// Vom Klassenassistenten generierte �berladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CFlashFolderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CFlashFolderApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_FLASHFOLDER_H__C5E276C1_460A_11D8_B7CB_9B1B4EA90ABA__INCLUDED_)
