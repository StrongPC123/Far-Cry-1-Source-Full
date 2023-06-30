#if !defined(AFX_SETHEIGHTDLG_H__41C883CF_80BE_4F97_A503_4ADC77634FD6__INCLUDED_)
#define AFX_SETHEIGHTDLG_H__41C883CF_80BE_4F97_A503_4ADC77634FD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetHeightDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetHeightDlg dialog

class CSetHeightDlg : public CDialog
{
// Construction
public:
	CSetHeightDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetHeightDlg)
	enum { IDD = IDD_SET_HEIGHT };
	int		m_sldHeight;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void SetHeight( float height );
	float GetHeight();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetHeightDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetHeightDlg)
		// NOTE: the ClassWizard will add member functions here
		virtual BOOL OnInitDialog();
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	float m_height;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETHEIGHTDLG_H__41C883CF_80BE_4F97_A503_4ADC77634FD6__INCLUDED_)
