#if !defined(AFX_MAINTOOLS_H__C2F9FB5D_C947_49FC_9109_73E43C1C2584__INCLUDED_)
#define AFX_MAINTOOLS_H__C2F9FB5D_C947_49FC_9109_73E43C1C2584__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainTools.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMainTools dialog

class CMainTools : public CDialog
{
// Construction
public:
	CMainTools(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMainTools();

// Dialog Data
	//{{AFX_DATA(CMainTools)
	enum { IDD = IDD_MAINTOOLS };
	//}}AFX_DATA

	void	UncheckAll();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainTools)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CreateButtons();
	void ReleaseButtons();
	void OnButtonPressed( int i );
	void StartTimer();
	void StopTimer();

	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CMainTools)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	std::vector<CColorCheckBox*> m_buttons;
	int m_lastPressed;
	int m_nTimer;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINTOOLS_H__C2F9FB5D_C947_49FC_9109_73E43C1C2584__INCLUDED_)
