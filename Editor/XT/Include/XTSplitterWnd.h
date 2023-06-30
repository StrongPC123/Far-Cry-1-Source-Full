// XTSplitterWnd.h : header file
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

#if !defined(__XTSPLITTERWND_H__)
#define __XTSPLITTERWND_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTSplitterWnd is a CSplitterWnd derived class.  It adds the ability
//			to hide and show splitter panes, based upon its index.
class _XT_EXT_CLASS CXTSplitterWnd : public CSplitterWnd
{
    DECLARE_DYNAMIC(CXTSplitterWnd)

public:

    // Summary: Constructs a CXTSplitterWnd object.
    CXTSplitterWnd();

    // Summary: Destroys a CXTSplitterWnd object, handles cleanup and de-allocation.
    virtual ~CXTSplitterWnd();

protected:

	int		m_nHiddenCol;	// Index of the hidden column.
	int		m_nHiddenRow;	// Index of the hidden row.
	BOOL	m_bFullDrag;	// TRUE if full window dragging is enabled.
	BOOL	m_bFlatSplit;	// TRUE if the flat splitter style is used.
	DWORD	m_dwxStyle;		// The style of the splitter window.  See SetSplitterStyle(...).
	CPoint	m_point;		// Previous cursor position.
	
public:

    // Summary: This member function is called to show the column that was previously
	//			hidden.
    virtual void ShowColumn();

	// Input:	nColHide - Index of the column to hide.
    // Summary:	This member function will hide a column based upon its index.
    virtual void HideColumn(int nColHide);

    // Summary: This member function is called to show the row that was previously
	//			hidden.
    virtual void ShowRow();

	// Input:	nRowHide - Index of the row to hide.
    // Summary:	This member function will hide a row based upon its index.
    virtual void HideRow(int nRowHide);

	// Input:	nRow - Specifies a row.
	//			nCol - Specifies a column.
	//			pNewView - Specifies the view to switch the specified pane with.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function is called to switch, or swap, a splitter view
	//			with another. 
    virtual BOOL SwitchView(int nRow,int nCol,CView *pNewView);

	// Input:	nRow - Specifies a row.
	//			nCol - Specifies a column.
	//			pViewClass - Specifies the runtime class to replace the specified pane with.
	// Returns: A CView pointer to the view that was replaced, otherwise returns NULL.
    // Summary:	This member function is called to replace an existing splitter view
	//			with another. 
    virtual CView* ReplaceView(int nRow,int nCol,CRuntimeClass *pViewClass);

	// Input:	nRow - Specifies a row.
	//			nCol - Specifies a column.
	//			pNewView - Points to an already existing view.
	// Returns: A CView pointer to the view that was replaced, otherwise returns NULL.
    // Summary:	This member function is called to replace an existing splitter view
	//			with another. 
    virtual CView* ReplaceView(int nRow,int nCol,CView* pNewView);

	// BULLETED LIST:

	// Input:	dwxStyle - Specifies XT_SPLIT_ styles to be added during style modification.
	//			The desired styles for the control bar can be one or more of the 
    //			following:
    //			[ul]
    //			[li]<b>XT_SPLIT_DOTTRACKER</b> The splitter window will use a
	//			dotted tracker rather than the splitter default.[/li]
    //			[li]<b>XT_SPLIT_NOFULLDRAG</b> Disable the "Show window contents while
	//			dragging" option, even if it is set in Windows.[/li]
    //			[li]<b>XT_SPLIT_NOBORDER</b> The splitter window will not draw a
	//			border around the pane.[/li]
    //			[li]<b>XT_SPLIT_NOSIZE</b> Do not allow splitter window panes to
	//			be resized.[/li]
    //			[/ul]
    // Summary:	Call this member function to modify a splitter window's style. Styles
	//			to be added or removed can be combined by using the bitwise OR (|)
	//			operator.
    virtual void SetSplitterStyle(DWORD dwxStyle);

	// Returns:  The current style of the splitter window.
    // Summary:  Call this member function to return the current style for the splitter
	//			 window.  
	// See Also: SetSplitterStyle()
    virtual DWORD GetSplitterStyle();

	// Input:	bFlatSplitter - TRUE to enable flat splitters.
    // Summary:	Call this member function to enable or disable flat splitters.
    virtual void EnableFlatLook(BOOL bFlatSplitter);

	// Returns: The index of the hidden column or -1 if no columns are hidden.
	// Summary:	Call this member function to return the index of the column that is 
	//			currently hidden.  
	int GetHiddenColIndex();

	// Returns: The index of the hidden row or -1 if no rows are hidden.
	// Summary:	Call this member function to return the index of the row that is 
	//			currently hidden.  
	int GetHiddenRowIndex();

    // Ignore:
	//{{AFX_VIRTUAL(CXTSplitterWnd)
    protected:
    virtual void SetSplitCursor(int ht);
    virtual void OnInvertTracker(const CRect& rect);
    virtual void StartTracking(int ht);
    virtual void StopTracking(BOOL bAccept);
    virtual void DrawTracker(const CRect& rect, CBrush* pBrush);
    virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg);
    //}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTSplitterWnd)
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
    
    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE DWORD CXTSplitterWnd::GetSplitterStyle() {
    return m_dwxStyle;
}
AFX_INLINE int CXTSplitterWnd::GetHiddenColIndex() {
	return m_nHiddenCol;
}
AFX_INLINE int CXTSplitterWnd::GetHiddenRowIndex() {
	return m_nHiddenRow;
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTSplitterWndEx is a CXTSplitterWnd derived class.  It can be used
//			with the CXTOutBarCtrl to display a gap between the top of the splitter
//			and the toolbar area.
class _XT_EXT_CLASS CXTSplitterWndEx : public CXTSplitterWnd  
{
    DECLARE_DYNAMIC(CXTSplitterWndEx)

public:
    
    // Summary: Constructs a CXTSplitterWndEx object.
    CXTSplitterWndEx();

    // Summary: Destroys a CXTSplitterWndEx object, handles cleanup and de-allocation.
    virtual ~CXTSplitterWndEx();
    
protected:

	int  m_cyTopBorderGap; // Size, in pixels, of the top border.
	bool m_bShowTopBorder; // true to draw a top border line.

public:

	// Input:	bShowTopBorder - If TRUE, a white edge will be drawn along the top.
	//			cyTopBorderGap - Amount, in pixels, to offset the splitter edge.
    // Summary:	Call this member function to show a top border for the splitter
	//			window, similar to Outlook.  Enabled by default.
    virtual void ShowTopBorder(bool bShowTopBorder=true,int cyTopBorderGap=7);

    // Ignore:
	//{{AFX_VIRTUAL(CXTSplitterWndEx)
    //}}AFX_VIRTUAL
	protected:
    virtual void GetInsideRect(CRect& rect) const;
	public:
 	virtual void RecalcLayout();
    virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg);
    
    // Ignore:
	//{{AFX_MSG(CXTSplitterWndEx)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG
    
    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTSplitterWndEx::ShowTopBorder(bool bShowTopBorder, int cyTopBorderGap) {
    m_bShowTopBorder = bShowTopBorder; m_cyTopBorderGap = cyTopBorderGap;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTSPLITTERWND_H__)