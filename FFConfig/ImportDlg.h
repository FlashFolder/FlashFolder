#pragma once

class CImportDlg : public CDialog
{
	DECLARE_DYNAMIC(CImportDlg)

public:
	CImportDlg(CWnd* pParent = NULL);   // Standardkonstruktor

	enum { IDD = IDD_IMPORT };

	CString GetTcIniPath() const { return m_enteredPath; }
	bool GetReplaceExisting() const { return m_replaceExisting; }

private:
	// overrides CDialog
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	// overrides CDialog
	BOOL OnInitDialog();
	// overrides CDialog
	void OnOK();

	afx_msg void OnBnClickedCheckInstalled();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnEnChangeEdit1();
	DECLARE_MESSAGE_MAP()

	CString m_installedIniPath, m_enteredPath;
	bool m_replaceExisting;
};

//----------------------------------------------------------------------------------------------------

void GetTcFavorites( FavoritesList* pFavs, CString tcIniPath );

