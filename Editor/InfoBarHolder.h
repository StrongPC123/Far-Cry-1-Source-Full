#if !defined(AFX_INFOBARHOLDER_H__4170D6C2_E52E_4573_84D3_E6D16D0DCE5B__INCLUDED_)
#define AFX_INFOBARHOLDER_H__4170D6C2_E52E_4573_84D3_E6D16D0DCE5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoBarHolder.h : header file
//

#include <XTToolkit.h>

// forward declarations.
class CInfoBar;
class CInfoProgressBar;

/////////////////////////////////////////////////////////////////////////////
// CInfoBarHolder dialog

class CInfoBarHolder : public CXTDialogBar
{
// Construction
public:
	CInfoBarHolder();   // standard constructor
	~CInfoBarHolder();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CInfoBarHolder)
	enum { IDD = IDD_INFO_BAR_HOLDER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	BOOL Create( CWnd* pParentWnd, UINT nStyle, UINT nID );
	void IdleUpdate();

	CInfoProgressBar* GetProgressBar() { return m_progressBar; };

	void EnableProgressBar( bool bEnable );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoBarHolder)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInfoBarHolder)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CInfoBar *m_infoBar;
	CInfoProgressBar *m_progressBar;
	bool m_bInProgressBarMode;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFOBARHOLDER_H__4170D6C2_E52E_4573_84D3_E6D16D0DCE5B__INCLUDED_)
