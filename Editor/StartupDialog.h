#if !defined(AFX_STARTUPDIALOG_H__24CFEC34_F2BD_4E27_B298_100A6D1BE41D__INCLUDED_)
#define AFX_STARTUPDIALOG_H__24CFEC34_F2BD_4E27_B298_100A6D1BE41D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StartupDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStartupDialog dialog

class CStartupDialog : public CDialog
{
// Construction
public:
	CStartupDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStartupDialog)
	enum { IDD = IDD_STARTUP };
	CButton	m_cUseAsBase;
	CListBox	m_lstRecentMaps;
	int		m_optSelection;
	CString	m_strMapFileName;
	CString	m_strDirName;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartupDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStartupDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseForMap();
	afx_msg void OnChangeLevDir();
	afx_msg void OnSelectedRecentDoc();
	afx_msg void OnDblClickRecentDoc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STARTUPDIALOG_H__24CFEC34_F2BD_4E27_B298_100A6D1BE41D__INCLUDED_)
