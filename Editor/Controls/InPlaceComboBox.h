////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   inplacecombobox.h
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __inplacecombobox_h__
#define __inplacecombobox_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "DropWnd.h"

class CInPlaceCBEdit : public CEdit
{
	CInPlaceCBEdit(const CInPlaceCBEdit& d);
	CInPlaceCBEdit& operator=(const CInPlaceCBEdit& d);

public:
	CInPlaceCBEdit();
	virtual ~CInPlaceCBEdit();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceCBEdit)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceCBEdit)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

inline CInPlaceCBEdit::CInPlaceCBEdit()
{
}

inline CInPlaceCBEdit::~CInPlaceCBEdit()
{
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCBListBox

class CInPlaceCBListBox : public CListBox
{
	CInPlaceCBListBox(const CInPlaceCBListBox& d);
	CInPlaceCBListBox& operator=(const CInPlaceCBListBox& d);

public:
	CInPlaceCBListBox();
	virtual ~CInPlaceCBListBox();

	void SetScrollBar( CScrollBar *sb ) { m_pScrollBar = sb; };

	int GetBottomIndex();
	void SetTopIdx(int nPos, BOOL bUpdateScrollbar=FALSE );

	// Operations
protected:
	void ProcessSelected(bool bProcess = true);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceCBListBox)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceCBListBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	int m_nLastTopIdx;
	CScrollBar* m_pScrollBar;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CInPlaceCBScrollBar : public CScrollBar
{
	// Construction
public:
	CInPlaceCBScrollBar();

	// Attributes
public:

	// Operations
public:
	void SetListBox( CInPlaceCBListBox* pListBox );

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceCBScrollBar)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CInPlaceCBScrollBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceCBScrollBar)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void VScroll(UINT nSBCode, UINT nPos);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CInPlaceCBListBox* m_pListBox;
};

/////////////////////////////////////////////////////////////////////////////
// CInPlaceComboBox

class CInPlaceComboBox : public CWnd
{
	CInPlaceComboBox(const CInPlaceComboBox& d);
	CInPlaceComboBox operator=(const CInPlaceComboBox& d);

protected:
	DECLARE_DYNAMIC(CInPlaceComboBox)

public:
	typedef Functor0 UpdateCallback;

	CInPlaceComboBox();
	virtual ~CInPlaceComboBox();

	void SetUpdateCallback( UpdateCallback cb ) { m_updateCallback = cb; };
	void SetReadOnly( bool bEnable ) { m_bReadOnly = bEnable; };

	// Attributes
public:
	int GetCurrentSelection() const;
	CString GetTextData() const;

	// Operations
public:
	int GetCount() const;
	int GetCurSel() const { return GetCurrentSelection(); };
	int SetCurSel(int nSelect, bool bSendSetData = true);
	void SelectString( LPCTSTR pStrText );
	CString GetSelectedString();
	int AddString(LPCTSTR pStrText, DWORD nData = 0);

	void ResetContent();
	void ResetListBoxHeight();

	void MoveControl(CRect& rect);

private:
	void SetCurSelToEdit(int nSelect);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceComboBox)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceComboBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnSelectionOk(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelectionCancel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEditChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewSelection(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void HideListBox();
	void GetDropDownRect( CRect &rect );
	
	// Data
private:
	static int m_nButtonDx;

	int m_minListWidth;
	bool m_bReadOnly;

	int m_nCurrentSelection;
	UpdateCallback m_updateCallback;

	CInPlaceCBEdit m_wndEdit;
	//CInPlaceCBListBox m_wndList;
	//CInPlaceCBScrollBar m_scrollBar;
	//CWnd m_wndDropDown;

	CDropWnd m_wndDropDown;

public:
	afx_msg void OnMove(int x, int y);
};

inline CInPlaceComboBox::~CInPlaceComboBox()
{
}

inline int CInPlaceComboBox::GetCurrentSelection() const
{
	return m_nCurrentSelection;
}


#endif // __inplacecombobox_h__#pragma once


