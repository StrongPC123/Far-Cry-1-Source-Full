#if !defined(AFX_SELECTIONCOMBO_H__66952646_5AAD_4182_AC95_845711841B8D__INCLUDED_)
#define AFX_SELECTIONCOMBO_H__66952646_5AAD_4182_AC95_845711841B8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectionCombo.h : header file
//

#include <XTToolkit.h>

/////////////////////////////////////////////////////////////////////////////
// CSelectionCombo window

class CSelectionCombo : public CXTFlatComboBox
{
// Construction
public:
	CSelectionCombo();

	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectionCombo)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSelectionCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSelectionCombo)
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTIONCOMBO_H__66952646_5AAD_4182_AC95_845711841B8D__INCLUDED_)
