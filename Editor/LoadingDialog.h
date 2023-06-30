#if !defined(AFX_LOADINGDIALOG_H__4384DEC2_0B2C_4326_8E8D_03DDC6BE3FEB__INCLUDED_)
#define AFX_LOADINGDIALOG_H__4384DEC2_0B2C_4326_8E8D_03DDC6BE3FEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadingDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoadingDialog dialog

#include "resource.h"

class CLoadingDialog : public CDialog
{
// Construction
public:
	CLoadingDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoadingDialog)
	enum { IDD = IDD_LOADING };
	CListBox	m_lstConsoleOutput;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadingDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoadingDialog)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADINGDIALOG_H__4384DEC2_0B2C_4326_8E8D_03DDC6BE3FEB__INCLUDED_)
