#if !defined(AFX_INFOPROGRESSBAR_H__C432F3A1_15CB_4573_82A3_AB1739604215__INCLUDED_)
#define AFX_INFOPROGRESSBAR_H__C432F3A1_15CB_4573_82A3_AB1739604215__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoProgressBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInfoProgressBar dialog

class CInfoProgressBar : public CDialog
{
// Construction
public:
	CInfoProgressBar(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInfoProgressBar)
	enum { IDD = IDD_INFO_PROGRESS_BAR };
	CProgressCtrl	m_progress;
	CStatic	m_info;
	CCustomButton	m_cancel;
	//}}AFX_DATA

	void BeginProgress( const CString &infoText );
	bool UpdateProgress( int percent );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoProgressBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInfoProgressBar)
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	bool m_bCanceled;
	int m_percent;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFOPROGRESSBAR_H__C432F3A1_15CB_4573_82A3_AB1739604215__INCLUDED_)
