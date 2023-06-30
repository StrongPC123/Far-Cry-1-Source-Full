#if !defined(AFX_NUMBERDLG_H__D8526DC9_AED2_4ACD_8E1C_97A1AAA0B986__INCLUDED_)
#define AFX_NUMBERDLG_H__D8526DC9_AED2_4ACD_8E1C_97A1AAA0B986__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumberDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumberDlg dialog

class CNumberDlg : public CDialog
{
// Construction
public:
	CNumberDlg( CWnd* pParent = NULL,float defValue = 0,const char *title = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNumberDlg)
	enum { IDD = IDD_NUMBER };
	//}}AFX_DATA

	float GetValue();
	void SetRange( float min,float max );
	void SetInteger( bool bEnable );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumberDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	float m_value;
	bool m_bInteger;

	// Generated message map functions
	//{{AFX_MSG(CNumberDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CNumberCtrl m_num;
	CString m_title;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMBERDLG_H__D8526DC9_AED2_4ACD_8E1C_97A1AAA0B986__INCLUDED_)
