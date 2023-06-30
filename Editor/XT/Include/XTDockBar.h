// XTDockBar.h interface for the CXTDockBar class.
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

#if !defined(__XTDOCKBAR_H__)
#define __XTDOCKBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// class forwards.

class CXTDockWindow;

// Summary: CArray definition for CXTSplitterDock pointer array.
typedef CArray <CXTSplitterDock *, CXTSplitterDock *> CXTSplitterDockArray;

//////////////////////////////////////////////////////////////////////
// Summary: CXTDockBar is a CDockBar derived class.  It is used by CXTFrameWnd and
//			CXTMDIFrameWnd to offset CXTToolBars to allow 3D borders.
class _XT_EXT_CLASS CXTDockBar : public CDockBar
{
	DECLARE_DYNAMIC(CXTDockBar)

public:

    // Summary: Constructs a CXTDockBar object.
	CXTDockBar();

	// Summary: Destroys a CXTDockBar object, handles cleanup and de-allocation.
    virtual ~CXTDockBar();

protected:

	
    int                  m_nTrackSplitter; // Splitter used to drag.
    BOOL                 m_bTracking;      // TRUE when tracking.
    HWND                 m_hWndFocus;      // Previous focus window when tracking.
    DWORD                m_dwXTStyle;      // FLAT styles.
    CXTSplitterDockArray m_arrSplitters;   // Array of CXTSplitterDock objects.

public:

	// Input:	nBar - Index of the bar in the control bar array.
	// Returns: A pointer to a valid CXTDockWindow object.
	// Summary:	This member function is called to determine if the docking window specified
	//			by 'nBar' is maximized. 
	CXTDockWindow* IsRowMaximized(int nBar);

	// Input:	pBar - A pointer to a valid CControlBar object.
	//			lpRect - Initial size of the docked bar.
	// Summary:	This member function is called to dock the control bar specified by 'pBar'.
	void DockControlBar(CControlBar* pBar,LPCRECT lpRect);

	// Input:	pMaxBar - A pointer to a valid CXTDockWindow object.
	//			bRecalcLayout - TRUE if the parent frame needs to recalculate the layout.
	// Summary:	This member function is called to maximize the control bar specified
	//			by 'pMaxBar'.
	void Maximize(CXTDockWindow* pMaxBar,BOOL bRecalcLayout = TRUE);

	// Input:	pMaxBar - A pointer to a valid CXTDockWindow object.
	//			bRecalcLayout - TRUE if the parent frame needs to recalculate the layout.
	// Summary:	This member function is called to normalize the control bar specified
	//			by 'pMaxBar'.
	void Normalize(CXTDockWindow* pMaxBar,BOOL bRecalcLayout = TRUE);

	// Returns:  The current CBRS_XT_ (Control bar) settings for the control bar. See 
	//			 CXTDockWindow::ModifyXTBarStyle for the complete list of available
	//			 styles.
	// Summary:	 Call this member function to determine which CBRS_XT_ (control bar) 
	//			 settings are currently set for the control bar. It does not handle
	//			 WS_ (window style) or CBRS_ (control bar style). 
	// See Also: CXTDockWindow::ModifyXTBarStyle
	DWORD GetXTBarStyle();

	// Input:	dwNewStyle - New CBRS_XT_ style for the control bar.  See CXTDockWindow::ModifyXTBarStyle
	//			for the complete list of available styles.
	// Summary:	This member function will set the style for the control bar. It does not 
	//			handle WS_ (window style) or CBRS_ (control bar style).
	void SetXTBarStyle(DWORD dwNewStyle);

    // Ignore:
	//{{AFX_VIRTUAL(CXTDockBar)
	public:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void HitTest(const CPoint& point);
	virtual void DoPaint(CDC* pDC);
	//}}AFX_VIRTUAL

	virtual int InsertEx(CControlBar* pBarIns, CRect rect, CPoint ptMid);

#if _MSC_VER < 1200 // MFC 5.0
	virtual void DrawGripper(CDC* pDC, const CRect& rect);
#endif //_MSC_VER < 1200

protected:

	// Input:	nInitPos - Initial position of the control bar whose row is to have its borders
	//			removed.
	//			dwBarStyle - Style for the row can be any CBRS_BORDER style.  See 
	//			CControlBar::SetBarStyle for more details.
	//			bOnlySized - TRUE if the control bar is to be only one sized.
	// Summary:	This member function is called to remove the row borders for the row
	//			of the control bar specified by 'nInitPos'.
	void RemoveRowBorders(int nInitPos,DWORD dwBarStyle = CBRS_BORDER_ANY,BOOL bOnlySized = FALSE);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			nNewHeight - Size, in pixels, of the new row height.
	// Returns: The amount, in pixels, the row was adjusted.
	// Summary:	This member function is called to set the height for the row of the
	//			control bar specified by 'nInitPos'. 
	int SetRowHeight(int nInitPos,int nNewHeight);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			nIncWidth - Amount, in pixels, to increment the width by.
	// Returns: The amount, in pixels, the row was adjusted.
	// Summary:	This member function is called to stretch the row of control bars to
	//			the left. 
	int StretchRowLeft(int nInitPos,int nIncWidth);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			nIncWidth - Amount, in pixels, to increment the width by.
	// Returns: The amount, in pixels, the row was adjusted.
	// Summary:	This member function is called to stretch the row of control bars to
	//			the right. 
	int StretchRowRight(int nInitPos,int nIncWidth);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			pNewBar - A pointer to a valid CControlBar object.
	// Returns: The number of control bars docked on the row specified.
	// Summary:	This member function gets the number of control bars docked on the
	//			row that the control bar, specified by 'nInitPos', resides. 
	int GetRowSizeBars(int nInitPos,CControlBar* pNewBar = NULL);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			pNewBar - A pointer to a valid CControlBar object.
	// Returns: The width of the fixed bar including the splitters.
	// Summary:	This member function gets the width of the fixed bar including the
	//			splitter sizes. 
	int GetRowFixedWidth(int nInitPos,CControlBar* pNewBar = NULL);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			pNewBar - A pointer to a valid CControlBar object.
	// Returns: The minimum width of the row specified.
	// Summary:	This member function gets the minimum width of the row for the control
	//			bar specified by 'nInitPos'. 
	int GetRowMinSizedWidth(int nInitPos,CControlBar* pNewBar = NULL);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			pNewBar - A pointer to a valid CControlBar object.
	// Returns: The normal width of the row specified.
	// Summary:	This member function gets the normal width of the row for the control
	//			bar specified by 'nInitPos'. 
	int GetRowSizedWidth(int nInitPos,CControlBar* pNewBar = NULL);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			pNewBar - A pointer to a valid CControlBar object.
	// Returns: The minimum height of the row specified.
	// Summary:	This member function gets the minimum height of the row for the
	//			control bar specified by 'nInitPos'. 
	int GetRowMaxHeight(int nInitPos,CControlBar* pNewBar = NULL);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	//			pNewBar - A pointer to a valid CControlBar object.
	// Returns: The minimum height of the row specified.
	// Summary:	This member function gets the minimum height of the row for the
	//			control bar specified by 'nInitPos'. 
	int GetRowMinHeight(int nInitPos,CControlBar* pNewBar = NULL);

	// Returns: The dockbar's available height.
	// Summary:	This member function gets the available height for the dockbar. 
	int GetAvailableHeight();

	// Input:	pBar - A pointer to a valid CControlBar object.
	// Returns: An int value that represents first control bar in
	//			the row.
	// Summary:	This member function gets a pointer to the first control bar in
	//			the row. 
	int GetFirstBarInRow(CControlBar* pBar);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	// Summary:	This member function is called to adjust the row sizes of the row for
	//			the control bar specified by 'nInitPos'.
	void AdjustRowSizes(int nInitPos);

	// Input:	dc - A CDC pointer to a valid device context.
	// Summary:	This member function is called to draw the splitter bars that separate
	//			the docked control bars.
	void DrawSplitters(CDC* dc);

	// Input:	nStart - Starting position of the bar in the control bar array.
	//			nType - Type of splitter bar; either XT_SPLITTER_VERT or XT_SPLITTER_HORZ.
	//			nLength - Initial size of the splitter.
	// Summary:	This member function is called to set the length of the splitters for
	//			the docked control bars.
	void SetSplittersLength(int nStart,int nType,int nLength);

	// Summary:	This member function is called to free resources allocated for dock
	//			splitters.
	void DeleteSplitters();

	// Input:	rcSplitter - A CRect object.
	//			nType - Type of splitter bar; either XT_SPLITTER_VERT or XT_SPLITTER_HORZ.
	//			bInterRow - TRUE if the splitter is an interior row.
	//			nPos - Initial position of the splitter bar.
	// Summary:	This member function is called to add a splitter to the dockbar.
	void AddSplitter(CRect rcSplitter,int nType,BOOL bInterRow,int nPos);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	// Returns: A pointer to a valid CControlBar object.
	// Summary:	This member function is called to return a pointer to the bar specified
	//			by 'nInitPos'. 
	CControlBar *FindNewBar(int nInitPos);

	// Input:	nInitPos - Initial position of the bar in the control bar array.
	// Returns: A pointer to a valid CXTDockWindow object.
	// Summary:	This member function is called to return a pointer to the control bar
	//			specified by 'nInitPos'. 'nInitPos' is the index of the bar in the control
	//			bar array. 
	CXTDockWindow *FindUniqueBar(int nInitPos);

	// Summary:	This member function is called during sizing operations to cancel the
	//			drag event.
	void OnCancel();

	// Input:	pBar - A pointer to a valid CControlBar object.
	// Returns: A pointer to a valid CXTDockWindow object, otherwise returns NULL.
	// Summary:	This member function is called to get a valid handle for the control
	//			bar specified by 'pBar'. 
	CXTDockWindow* GetSafeDockWindow(CControlBar* pBar);

	// Input:	nPos - Initial position of the bar in the control bar array.
	// Returns: true if the bar is the last in the row, otherwise returns false.
    // Summary:	This member function is called to determine if the control bar specified
	//			by 'nPos' is the last control bar in the row.  
	bool IsLastBarInRow(int nPos);

	// Summary:	This member function is called to determine the number of
	//			visible docked control bars.
	int GetDockedVisibleCount() const;

	// Input:	pDC - A CDC pointer to a valid device context.
	//			rect - A reference to a valid CRect object.
	// Summary:	This member function is called during paint operations to draw the
	//			non-client areas of the dockbar.
	void DrawNcBorders(CDC* pDC,CRect& rect);

    // Ignore:
	//{{AFX_MSG(CXTDockBar)
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	//}}AFX_MSG
    
	DECLARE_MESSAGE_MAP()

	friend class CXTDockWindow;
	friend class CXTSplitterWnd;
	friend class CXTSplitterDock;
	friend class CXTSplitterRowDock;
	friend class CXTMiniDockFrameWnd;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTDockBar::SetXTBarStyle(DWORD dwNewStyle) {
	m_dwXTStyle = dwNewStyle;
}
AFX_INLINE DWORD CXTDockBar::GetXTBarStyle() {
	ASSERT(::IsWindow(m_hWnd));	return m_dwXTStyle;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTDOCKBAR_H__)

