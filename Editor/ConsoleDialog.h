#if !defined(AFX_CONSOLEDIALOG_H__D9B5584C_9F90_4AA9_B54B_4E4B6BD3A977__INCLUDED_)
#define AFX_CONSOLEDIALOG_H__D9B5584C_9F90_4AA9_B54B_4E4B6BD3A977__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConsoleDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConsoleDialog dialog

class CConsoleDialog : public CDialog
{
// Construction
public:
	CConsoleDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConsoleDialog)
	enum { IDD = IDD_CONSOLE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConsoleDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CConsoleDialog)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONSOLEDIALOG_H__D9B5584C_9F90_4AA9_B54B_4E4B6BD3A977__INCLUDED_)
