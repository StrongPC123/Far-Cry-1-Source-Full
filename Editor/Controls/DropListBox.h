/////////////////////////////////////////////////////////////////////////////
// DropListBox.h : header file
// 
// CAdvComboBox Control
// Version: 2.1
// Date: September 2002
// Author: Mathias Tunared
// Email: Mathias@inorbit.com
// Copyright (c) 2002. All Rights Reserved.
//
// This code, in compiled form or as source code, may be redistributed 
// unmodified PROVIDING it is not sold for profit without the authors 
// written consent, and providing that this notice and the authors name 
// and all copyright notices remains intact.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DROPLISTBOX_H__E49DED7C_CBC2_4458_A72C_F4C428927132__INCLUDED_)
#define AFX_DROPLISTBOX_H__E49DED7C_CBC2_4458_A72C_F4C428927132__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DropListBox.h : header file
//

#include "DropScrollBar.h"

#define WM_SELECTED_ITEM		(WM_USER+1)
#define WM_DESTROY_DROPLIST		(WM_USER+2)
#define WM_VRC_SETCAPTURE		(WM_USER+3)
#define WM_VRC_RELEASECAPTURE	(WM_USER+4)

class CDropScrollBar;

/////////////////////////////////////////////////////////////////////////////
// CDropListBox window

class CDropListBox : public CListBox
{
// Construction
public:
	CDropListBox();

// Attributes
public:

// Operations
public:
	void SetScrollBar( CDropScrollBar* pScroll ) { m_pScroll = pScroll; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDropListBox)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	int SetCurSel( int nSelect );
	void GetTextSize( LPCTSTR lpszText, int nCount, CSize& size );
	void SetTopIdx( int nPos, BOOL bUpdateScrollbar = FALSE );
	int GetBottomIndex();
	virtual ~CDropListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDropListBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnSetCapture( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnReleaseCapture( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()

private:
	CWnd* m_pComboParent;
	CDropScrollBar* m_pScroll;
	int m_nLastTopIdx;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DROPLISTBOX_H__E49DED7C_CBC2_4458_A72C_F4C428927132__INCLUDED_)
