#if !defined(AFX_SIZEDIALOG_H__C8E98A11_BBE9_499F_AB62_6EA12BDBCF84__INCLUDED_)
#define AFX_SIZEDIALOG_H__C8E98A11_BBE9_499F_AB62_6EA12BDBCF84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SizeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSizeDialog dialog

class CSizeDialog : public CDialog
{
// Construction
public:
	CSizeDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSizeDialog)
	enum { IDD = IDD_SIZE };
	UINT	m_txtTerrainSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSizeDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSizeDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIZEDIALOG_H__C8E98A11_BBE9_499F_AB62_6EA12BDBCF84__INCLUDED_)
