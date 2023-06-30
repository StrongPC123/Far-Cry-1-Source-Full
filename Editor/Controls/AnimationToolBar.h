#if !defined(AFX_ANIMATIONTOOLBAR_H__4ED3FCE4_CB04_487F_84B5_AE815BE2711D__INCLUDED_)
#define AFX_ANIMATIONTOOLBAR_H__4ED3FCE4_CB04_487F_84B5_AE815BE2711D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimationToolBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimationToolBar window

#include "AnimSequences.h"

class CAnimationToolBar : public CToolBar
{
// Construction
public:
	CAnimationToolBar();

// Attributes
public:

// Operations
public:
	CString GetAnimName();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAnimationToolBar();

	CSliderCtrl m_cKeyframes;        // IDR_KEYFRAMES
	CAnimSequences m_cAnimSequences; // IDR_ANIM_SEQUENCES

	// Generated message map functions
protected:
	//{{AFX_MSG(CAnimationToolBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATIONTOOLBAR_H__4ED3FCE4_CB04_487F_84B5_AE815BE2711D__INCLUDED_)
