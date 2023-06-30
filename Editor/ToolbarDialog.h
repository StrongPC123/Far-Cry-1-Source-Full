// ToolbarDialog.h: interface for the CToolbarDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLBARDIALOG_H__31620F0B_DB2D_45BA_A86B_E71BD4F79414__INCLUDED_)
#define AFX_TOOLBARDIALOG_H__31620F0B_DB2D_45BA_A86B_E71BD4F79414__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Controls\DlgBars.h"

class CToolbarDialog : public CDialog
{
	DECLARE_DYNAMIC(CToolbarDialog)
public:
	CToolbarDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	CToolbarDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CToolbarDialog();

	void RecalcLayout();
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainDialog)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTerrainDialog)
	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg	BOOL CToolbarDialog::OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

#endif // !defined(AFX_TOOLBARDIALOG_H__31620F0B_DB2D_45BA_A86B_E71BD4F79414__INCLUDED_)
