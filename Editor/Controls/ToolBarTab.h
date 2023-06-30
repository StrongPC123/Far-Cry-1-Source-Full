#if !defined(AFX_TOOLBARTAB_H__691888C3_A10C_4EB0_94B8_C1E68110106E__INCLUDED_)
#define AFX_TOOLBARTAB_H__691888C3_A10C_4EB0_94B8_C1E68110106E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarTab.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CToolBarTab window

#include "HiColorToolBar.h"

class CToolBarTab : public CTabCtrl
{
// Construction
public:
	CToolBarTab();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarTab)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolBarTab();

	void ResizeAllContainers();

	// Generated message map functions
protected:

	CScrollBar m_cScrollBar;

	//{{AFX_MSG(CToolBarTab)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARTAB_H__691888C3_A10C_4EB0_94B8_C1E68110106E__INCLUDED_)
