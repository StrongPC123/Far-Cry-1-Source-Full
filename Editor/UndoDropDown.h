#if !defined(AFX_UNDODROPDOWN_H__A77AAA18_1F9B_45C3_8E23_D9DC0DEE4179__INCLUDED_)
#define AFX_UNDODROPDOWN_H__A77AAA18_1F9B_45C3_8E23_D9DC0DEE4179__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UndoDropDown.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUndoDropDown dialog

class CUndoDropDown : public CDialog
{
// Construction
public:
	//! If bUndo is false then its Redo dialog.
	CUndoDropDown( const CPoint &pos,bool bUndo,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUndoDropDown)
	enum { IDD = IDD_UNDO_DROPDOWN };
	CCustomButton	m_undoClear;
	CCustomButton	m_undoButton;
	CListBox	m_undo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUndoDropDown)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUndoDropDown)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeUndo();
	afx_msg void OnUndoButton();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnUndoClear();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//! True if undo, false if Redo.
	bool m_bUndo;
	CPoint m_pos;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNDODROPDOWN_H__A77AAA18_1F9B_45C3_8E23_D9DC0DEE4179__INCLUDED_)
