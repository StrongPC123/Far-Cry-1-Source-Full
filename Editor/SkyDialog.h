#if !defined(AFX_SKYDIALOG_H__DC3CC2E7_F964_4C16_A5CD_C7E70BB1D292__INCLUDED_)
#define AFX_SKYDIALOG_H__DC3CC2E7_F964_4C16_A5CD_C7E70BB1D292__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkyDialog.h : header file
//

#include "Controls\PropertyCtrl.h"
#include "ToolbarDialog.h"

enum PreviewDirection
{
	PDNorth,
	PDEast,
	PDSouth,
	PDWest
};

/////////////////////////////////////////////////////////////////////////////
// CSkyDialog dialog

class CSkyDialog : public CToolbarDialog
{
// Construction
public:
	CSkyDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSkyDialog)
	enum { IDD = IDD_SKY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSkyDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSkyNorth();
	afx_msg void OnSkySouth();
	afx_msg void OnSkyWest();
	afx_msg void OnSkyEast();
	afx_msg void OnPaint();
	afx_msg void OnSkyClouds();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CToolBar m_cDlgToolBar;
	PreviewDirection m_PD;

	CPropertyCtrl m_propWnd;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKYDIALOG_H__DC3CC2E7_F964_4C16_A5CD_C7E70BB1D292__INCLUDED_)
