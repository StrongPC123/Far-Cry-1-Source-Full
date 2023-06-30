#if !defined(AFX_TOOLBUTTON_H__5CB83738_43F3_48F5_A1DD_940B3BF330CA__INCLUDED_)
#define AFX_TOOLBUTTON_H__5CB83738_43F3_48F5_A1DD_940B3BF330CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolButton.h : header file
//
#include "ColorCheckBox.h"

/////////////////////////////////////////////////////////////////////////////
// CToolButton window

class CToolButton : public CColorCheckBox
{
DECLARE_DYNAMIC(CToolButton)
// Construction
public:
	CToolButton();

	void SetToolClass( CRuntimeClass *toolClass,void *userData=0 );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolButton)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolButton)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnClicked();
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void StartTimer();
	void StopTimer();

	//! Tool associated with this button.
	CRuntimeClass *m_toolClass;
	void *m_userData;
	int m_nTimer;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBUTTON_H__5CB83738_43F3_48F5_A1DD_940B3BF330CA__INCLUDED_)
