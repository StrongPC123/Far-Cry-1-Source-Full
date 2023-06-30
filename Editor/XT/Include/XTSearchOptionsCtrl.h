// XTSearchOptionsCtrl.h : header file
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTSEARCHOPTIONSCTRL_H__)
#define __XTSEARCHOPTIONSCTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTSearchOptionsCtrl is a CStatic derived class.  CXTSearchOptionsCtrl
//			is used to create a control similar to the Search Options item found
//			in the search pane of Windows Explorer as is seen in Windows 2000 and
//			later.  This class allows you to associate a group of controls to be
//			hidden or shown when the item is expanded and contracted, and a set
//			of controls that need to move depending on the CXTSearchOptionsCtrl state.
//
//			Use with CXTSearchOptionsView form view class to create a pane similar
//			to the Windows Explorer search pane.  To use the control, define a set
//			of controls that are to be hidden and moved depending on the 
//			CXTSearchOptionsCtrl state.
//
// Example: <pre>
//			void CExpandTestView::OnInitialUpdate()
//			{
//			   CXTSearchOptionsView::OnInitialUpdate();
//
//			   m_expand.AddControl(&m_check1);
//			   m_expand.AddControl(&m_check2);
//			   m_expand.AddControl(&m_edit1);
//			   m_expand.AddControl(&m_edit2);
//
//			   m_expand.MoveControl(&m_button1);
//			   m_expand.MoveControl(&m_button2);
//			   m_expand.MoveControl(&m_combo1);
//  
//			   m_expand.SetLabelText(
//			       _T("Search Options <<"), _T("Search Options >>"));
//			}</pre>
//
//			See the "SearchOptions" demo for a complete example.
class _XT_EXT_CLASS CXTSearchOptionsCtrl : public CStatic
{
	DECLARE_DYNAMIC(CXTSearchOptionsCtrl)

public:

    // Summary: Constructs a CXTSearchOptionsCtrl object.
	CXTSearchOptionsCtrl();

    // Summary: Destroys a CXTSearchOptionsCtrl object, handles cleanup and de-allocation.
	virtual ~CXTSearchOptionsCtrl();

protected:

	int			m_iMinSize;			// Height of the control when contracted.
	int			m_iMaxSize;			// Height of the control when expanded.
	bool		m_bExpanded;		// true when the control is expanded.
	CRect		m_rcLabel;			// Size of the label that is displayed.
	DWORD		m_dwInitSignature;  // used for one-time initialization.
	CString		m_strExpandLabel;	// Label to display when the control is expanded.
	CString		m_strContractLabel; // Label to display when the control is contracted.
	CPtrArray	m_arHideCtrls;		// List of controls to show or hide.
	CPtrArray	m_arMoveCtrls;		// List of controls to move when expanded or contracted.
									


public:

	// Input:	pWndCtrl - Points to a valid CWnd object to hide.
	// Summary:	Call this member function to add a control to the list of controls
	//			that are displayed when the hide item control is expanded.
	void AddControl(CWnd* pWndCtrl);

	// Input:	pWndCtrl - Points to a valid CWnd object to move.
	// Summary:	Call this member function to add a control to the list of controls
	//			that are moved when the hide item control is expanded or contracted.
	void MoveControl(CWnd* pWndCtrl);

	// Input:	lpszExpand - NULL terminated string that represents the text displayed when 
	//			the control is expanded.
	//			lpszContract - NULL terminated string that represents the text displayed when 
	//			the control is contracted.
	// Summary:	Call this member function to set the text that is displayed when the
	//			hide item control is expanded or contracted.
	void SetLabelText(LPCTSTR lpszExpand,LPCTSTR lpszContract);

	// Summary: Call this member function to expand the hide item control and display
	//			CWnd objects contained in the hide item list.  Called by the control
	//			whenever the user clicks on the expand label.
	void Expand();

	// Summary: Call this member function to contract the hide item control and hide
	//			CWnd objects contained in the hide item list.  Called by the control
	//			whenever the user clicks on the contract label.
	void Contract();

	// Returns: An integer value that represents the height of the
	//			control when it is contracted.
	// Summary:	Call this member function to return the minimum height of the hide
	//			item control. 
	int GetMinSize();

	// Returns: An integer value that represents the height of the
	//			control when it is expanded.
	// Summary:	Call this member function to return the maximum height of the hide
	//			item control. 
	int GetMaxSize();

	// Returns: An integer value that represents the distance that controls will be moved.
	// Summary:	Call this member function to return the offset size for the 
	//			CXTSearchOptionsCtrl object.  This is the distance that controls will
	//			be moved to accommodate for the expansion and contraction of the control.
	//			Also used by CXTSearchOptionsView for adjusting scroll sizes. 
	int GetOffsetSize();

protected:

	// Summary: defers control initialization
	void DeferInitialUpdate();

	// Ignore:
	//{{AFX_VIRTUAL(CXTSearchOptionsCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

protected:

	// Ignore:
	//{{AFX_MSG(CXTSearchOptionsCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);
		
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE int CXTSearchOptionsCtrl::GetMinSize() {
	return m_iMinSize;
}
AFX_INLINE int CXTSearchOptionsCtrl::GetMaxSize() {
	return m_iMaxSize;
}
AFX_INLINE int CXTSearchOptionsCtrl::GetOffsetSize() {
	return GetMaxSize()-GetMinSize();
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTSearchOptionsView is a CXTResizeFormView derived helper class.  It
//			is to be used with a CXTSearchOptionsCtrl object. It is used to paint
//			the background and control background color white. 
class _XT_EXT_CLASS CXTSearchOptionsView : public CXTResizeFormView
{
	DECLARE_DYNAMIC(CXTSearchOptionsView)

protected:

	// Input:	nID - Contains the ID number of a dialog template resource.
	//			nFlags - Flags that are to be passed to CXTResize that specify the attributes
	//			of the resizing property page.  They can be one or more of the following,
	//			and can be combined using the or (|) operator:
	//			[ul]
    //			[li]<b>SZ_NOSIZEICON</b> Do not add size icon.[/li]
    //			[li]<b>SZ_NOHORISONTAL</b> No horizontal resizing.[/li]
    //			[li]<b>SZ_NOVERTICAL</b> No vertical resizing.[/li]
    //			[li]<b>SZ_NOMINSIZE</b> Do not require a minimum size.[/li]
    //			[li]<b>SZ_NOCLIPCHILDREN</b> Do not set clip children style.[/li]
    //			[li]<b>SZ_NOTRANSPARENTGROUP</b> Do not set transparent style
	//			for group boxes.[/li]
	//			[/ul]
    // Summary:	Constructs a CXTSearchOptionsView object, same as if you were constructing
	//			a CXTResizeFormView object.
	CXTSearchOptionsView(const UINT nID = 0,const UINT nFlags = 0);

    // Summary: Destroys a CXTSearchOptionsView object, handles cleanup and de-allocation.
	virtual ~CXTSearchOptionsView();

	HBRUSH m_hBrush;  // Handle to the current background brush.

	// Ignore:
	//{{AFX_VIRTUAL(CXTSearchOptionsView)
	//}}AFX_VIRTUAL

	// Ignore:
	//{{AFX_MSG(CXTSearchOptionsView)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTSEARCHOPTIONSCTRL_H__)