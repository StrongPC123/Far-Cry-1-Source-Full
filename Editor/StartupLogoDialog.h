#if !defined(AFX_STARTUPLOGODIALOG_H__F22FC7E2_D431_4746_BFB8_9D3E7EF36D8D__INCLUDED_)
#define AFX_STARTUPLOGODIALOG_H__F22FC7E2_D431_4746_BFB8_9D3E7EF36D8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StartupLogoDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStartupLogoDialog dialog

class CStartupLogoDialog : public CDialog
{
// Construction
public:
	CStartupLogoDialog(CWnd* pParent = NULL);   // standard constructor

	void SetVersion( const Version &v );
	void SetInfo( const CString &text );

// Dialog Data
	//{{AFX_DATA(CStartupLogoDialog)
	enum { IDD = IDD_STARTUP_LOGO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartupLogoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStartupLogoDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STARTUPLOGODIALOG_H__F22FC7E2_D431_4746_BFB8_9D3E7EF36D8D__INCLUDED_)
