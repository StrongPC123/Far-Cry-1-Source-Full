#if !defined(AFX_ANIMSEQUENCES_H__AF987D7B_1EC7_403A_B190_E05E2B2D3FD6__INCLUDED_)
#define AFX_ANIMSEQUENCES_H__AF987D7B_1EC7_403A_B190_E05E2B2D3FD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimSequences.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimSequences window

class CAnimSequences : public CComboBox
{
// Construction
public:
	CAnimSequences();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimSequences)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAnimSequences();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAnimSequences)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropdown();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMSEQUENCES_H__AF987D7B_1EC7_403A_B190_E05E2B2D3FD6__INCLUDED_)
