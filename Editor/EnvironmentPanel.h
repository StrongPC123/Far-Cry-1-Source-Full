#include "afxwin.h"
#if !defined(AFX_ENVIRONMENTPANEL_H__6D9EACA8_44EB_4609_AA48_B47B808E3725__INCLUDED_)
#define AFX_ENVIRONMENTPANEL_H__6D9EACA8_44EB_4609_AA48_B47B808E3725__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnvironmentPanel.h : header file
//

#include "Controls\PropertyCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CEnvironmentPanel dialog

class CEnvironmentPanel : public CDialog
{
// Construction
public:
	CEnvironmentPanel(CWnd* pParent = NULL);   // standard constructor
	~CEnvironmentPanel();

// Dialog Data
	//{{AFX_DATA(CEnvironmentPanel)
	enum { IDD = IDD_PANEL_ENVIRONMENT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEnvironmentPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void OnPropertyChanged( XmlNodeRef node );

	// Generated message map functions
	//{{AFX_MSG(CEnvironmentPanel)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CPropertyCtrl m_wndProps;
	XmlNodeRef m_node;
public:
	afx_msg void OnBnClickedApply();
private:
	CCustomButton m_applyBtn;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENVIRONMENTPANEL_H__6D9EACA8_44EB_4609_AA48_B47B808E3725__INCLUDED_)
