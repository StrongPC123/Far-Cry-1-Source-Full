#if !defined(AFX_TOOLBARCONTAINER_H__1EE442FA_871E_4FE8_BCCA_F2E53919548C__INCLUDED_)
#define AFX_TOOLBARCONTAINER_H__1EE442FA_871E_4FE8_BCCA_F2E53919548C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarContainer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CToolBarContainer window

class CToolBarContainer : public CWnd
{
// Construction
public:
	CToolBarContainer();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarContainer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolBarContainer();

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolBarContainer)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARCONTAINER_H__1EE442FA_871E_4FE8_BCCA_F2E53919548C__INCLUDED_)
