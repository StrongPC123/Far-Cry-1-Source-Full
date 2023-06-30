#if !defined(AFX_SELECTFILEDLG_H__3B89851F_924E_4587_A12F_F705CD1DF09D__INCLUDED_)
#define AFX_SELECTFILEDLG_H__3B89851F_924E_4587_A12F_F705CD1DF09D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectFileDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectFileDlg dialog

#include "Controls\FileTree.h"
#include "Controls\PreviewModelCtrl.h"

class CSelectFileDlg : public CDialog
{
// Construction
public:
	bool SelectFileName(CString *pFileNameOut, CString *pRelativeFileNameOut, 
		CString strFileSpec, CString strSearchFolder);
	CSelectFileDlg(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CSelectFileDlg)
	enum { IDD = IDD_SELECT_FILE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdatePreview(CString strFileToPreview);

	CFileTree m_cFileTree;

	bool m_previewModelEnabled;
	CPreviewModelCtrl m_previewModel;

	// Generated message map functions
	//{{AFX_MSG(CSelectFileDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CString m_strSelectedFile;
	static CString m_strLastPath;
	static CString m_strLastSearchFolder;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTFILEDLG_H__3B89851F_924E_4587_A12F_F705CD1DF09D__INCLUDED_)
