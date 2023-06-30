// XTFlatTabCtrl.h interface for the CXTFlatTabCtrl class.
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

#if !defined(__XTFLATTABCTRL_H__)
#define __XTFLATTABCTRL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTFlatTabCtrl is a CWnd derived class.  It is used to create an Excel
//			style sheet control. This control allows you to define if you want the
//			control to have home, end, back, and next buttons.
class _XT_EXT_CLASS CXTFlatTabCtrl : public CWnd
{
	DECLARE_DYNAMIC(CXTFlatTabCtrl)

public:

	// Summary: Constructs a CXTFlatTabCtrl object.
	CXTFlatTabCtrl();

	// Summary: Destroys a CXTFlatTabCtrl object, handles cleanup and de-allocation.
	virtual ~CXTFlatTabCtrl();

protected:

	int				m_cx;					// Width for each arrow button.
	int				m_cy;					// Height for each arrow button.
	int				m_nCurSel;				// Index of the currently selected tab.
	int				m_nClientWidth;			// Width, in pixels, of the tab control client area.
	int				m_nClientHeight;		// Height, in pixels, of the tab control client area.
	int				m_nOffset;				// Amount, in pixels, of the displayed tab offset.
	int				m_xGripperPos;			// The current gripper position, in pixels, from the left.
	int				m_iGripperPosPerCent;	// The current gripper position in percent of the control width.
	bool			m_bManagingViews;		// true if the control is managing views.
	bool			m_bUserColors;			// true if user defined colors are used.
	DWORD			m_dwStyle;				// Tab control style.
	CRect			m_rectTabs;				// Area occupied by tabs.
	CRect			m_rectViews;			// Area occupied by managed views.
	CRect			m_rectSB_H;				// Area occupied by the horizontal scroll bar.
	CRect			m_rectGripper;			// Area occupied by the sizing gripper.
	CFont*			m_pNormFont;			// Font that is used for non-selected tabs.
	CFont*			m_pBoldFont;			// Font that is used for selected tabs.
	COLORREF		m_clr3DShadow;			// RGB value that represents the tab shadow color.
	COLORREF		m_clrBtnText;			// RGB value that represents the tab outline color.
	COLORREF		m_clr3DHilight;			// RGB value that represents the tab highlight color.
	COLORREF		m_clrWindow;			// RGB value that represents the selected tab face color.
	COLORREF		m_clr3DFace;			// RGB value that represents the normal tab face color.
	COLORREF		m_clrWindowText;		// RGB value that represents the tab text color.
	CScrollBar		m_wndHScrollBar;		// The horizontal scroll bar (used with FTS_XT_HSCROLL).
	CToolTipCtrl	m_ToolTip;				// Tooltip for the flat tab control.

	// Pens used by painting code

	CPen m_penShadow;
	CPen m_penBackSel;
	CPen m_penBackNonSel;
	CPen m_penOutline;
	CPen m_penWindow;
	
	// Summary: Template list containing tab information.
	CArray <XT_TCB_ITEM*, XT_TCB_ITEM*> m_tcbItems;

	// Tracking related variables

	bool m_bTracking;
	int m_xTrackingDelta;
	CWnd * m_pWndLastFocus;

	// Summary: Enumerated type that specifies which arrow to display for a particular button.
	enum icon_type
	{
		arrow_left          =   0x0200, // Left arrow display.
		arrow_left_home     =   0x0201, // Left home arrow display.
		arrow_right         =   0x0000, // Right arrow display.
		arrow_right_home    =   0x0001  // Right home arrow display.
	};

	// Internal structures/variables used to control button information

	class CXTFTButtonState
	{
	public:
		CXTFTButtonState();

		void SetInfo(CRect rect, int iCommand, icon_type iconType);

		CRect m_rect;
		bool  m_bPressed;
		bool  m_bEnabled;
		int   m_iCommand;
		icon_type m_IconType;
	};

	int				 m_iBtnLeft;	// Index of "left" button in button array.
	int				 m_iBtnRight;	// Index of "right" button in button array.
	int				 m_iBtnHome;	// Index of "home" button in button array.
	int				 m_iBtnEnd;		// Index of "end" button in button array.
	CXTFTButtonState m_buttons[4];  // Array of button information.

public:

	// Input:	nIndex - The index of the tab whose text is to be retrieved.
	// Returns: The text of a particular tab, or NULL if an error occurs.
	// Summary:	This member function gets the text of a specific tab.  
	LPCTSTR GetItemText(int nIndex) const;

	// Input:	nIndex - The index of the tab whose text is to be changed.
	//			pszText - The new title for the tab.
	// Returns: true when successful.
	// Summary:	This member function will set the text of a particular tab.  
	bool SetItemText(int nIndex,LPCTSTR pszText);

	// Input:	nIndex - The index of the tab whose managed window is to be retrieved.
	// Returns: A pointer to the window that is associated with a tab, or it returns NULL 
	//			if no window is associated with (managed by) the tab.
	// Summary:	This member function gets a CWnd pointer to the window that is associated
	//			with a specific tab.  
	CWnd *GetItemWindow(int nIndex) const;

	// Input:	pNormFont - Represents the font used by non-selected tabs.
	//			pBoldFont - Represents the font used by selected tabs.
	// Summary:	This member function will set the fonts to be used by the tab control.
	virtual void SetTabFonts(CFont* pNormFont,CFont* pBoldFont);

	// Input:	nItem - Index of the tab to insert.
	//			nTextID - String resource ID of the tab label.
	//			pWndControl - Optional pointer to the managed control.
	// Returns: The index of the tab that has been inserted if successful, otherwise
	//			returns -1.
	// Summary:	This member function will insert a tab into the flat tab control. 
	virtual int InsertItem(int nItem,UINT nTextID,CWnd *pWndControl = NULL);

	// Input:	nItem - Index of the tab to insert.
	//			lpszItem - NULL terminated string that represents the tab label.
	//			pWndControl - Optional pointer to the managed control.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will insert a tab into the flat tab control. 
	virtual BOOL InsertItem(int nItem,LPCTSTR lpszItem,CWnd *pWndControl = NULL);

	// Input:	nItem - Index of the tab to delete.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will delete the tab specified by 'nItem' from the
	//			tab control. 
	virtual BOOL DeleteItem(int nItem);

	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will remove all of the tabs from the tab control.
	virtual BOOL DeleteAllItems();

	// Input:	nItem - Index of the tab to retrieve the size of.
	//			lpRect - Points to a RECT structure to receive the size of the tab.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will retrieve the size of the tab specified by
	//			'nItem'. 
	virtual BOOL GetItemRect(int nItem,LPRECT lpRect);

	// Input:	pHitTestInfo - Pointer to a TCHITTESTINFO structure, as 
	//			described in the Platform SDK, which specifies 
	//			the screen position to test.
	// Returns: The zero-based index of the tab, or returns -1 if no tab is at the specified
	//			position.
	// Summary:	Call this function to determine which tab, if any, is at the specified
	//			screen position. 
	virtual int HitTest(TCHITTESTINFO *pHitTestInfo) const;

	// Returns: A zero-based index of the selected tab if successful, or returns -1 if no tab
	//			is selected.
	// Summary:	Call this function to retrieve the currently selected tab in a flat
	//			tab control. 
	virtual int GetCurSel() const;

	// Input:	nItem - The zero-based index of the item to be selected.
	// Returns: A zero-based index of the previously selected tab if successful, otherwise
	//			returns -1.
	// Summary:	This member function selects a tab in a flat tab control. 
	virtual int SetCurSel(int nItem);

	// Returns: The number of items in the tab control.
	// Summary:	Call this function to retrieve the number of tabs in the tab control.
	virtual int GetItemCount() const;

	// Input:	nItem - The zero-based index of tab to receive the tooltip text.
	//			lpszTabTip - A pointer to a string containing the tooltip text.
	// Summary:	This member function will set the tooltip for the tab specified
	//			by 'nItem'.
	virtual void SetTipText(int nItem,LPCTSTR lpszTabTip);

	// Input:	nItem - The zero-based index of the tab to retrieve the tooltip 
	//			text for.
	// Returns: A CString object containing the text to be used in the tooltip.
	// Summary:	This member function will get the tooltip text associated with the
	//			tab specified by 'nItem'. 
	virtual CString GetTipText(int nItem);

	// Summary: This member function will cause the tab control to reposition
	//			the tabs to the home position.
	virtual void Home();

	// Input:	x - The position for the gripper, relative to the left-hand-side of
	//			the control.
	//			bPercent - Indicates that the position is a percent of the control width,
	//			as opposed to an absolute location in pixels.
	// Summary:	This member function changes the location of the sizing gripper.
	//			The function has no effect if the FTS_XT_HSCROLL is not used.
	void SetGripperPosition(int x,bool bPercent);

	// Returns: An integer value representing the location of the sizing gripper, in pixels,
	//			relative to the left hand side of the control.
	// Summary:	This member function gets the location of the sizing gripper, in pixels,
	//			relative to the left hand side of the control. 
	int GetGripperPosition() const;

	// Summary: Call this function to synchronize the tab control's horizontal scroll
	//			bar with the horizontal scroll bar of the current view.
	// 
	//			You should call this function if anything happens in the view that
	//			affects the horizontal scroll bar (e.g. a user typing text into an
	//			edit control could make the text wider, thus requiring a call to this
	//			function).
	void SyncScrollBar();
	
	// Input:	crShadow - RGB value that represents the tab shadow color.
	// Summary:	This member function is called to set the shadow color for all tabs.
	void SetTabShadowColor(COLORREF crShadow);
	
	// Returns: An RGB value that represents the tab shadow color.
	// Summary:	This member function gets an RGB value that represents the shadow color
	//			of the tab. 
	COLORREF GetTabShadowColor() const;
	
	// Input:	crHilight - RGB value that represents the tab highlight color.
	// Summary:	This member function is called to set the highlight color for all tabs.
	void SetTabHilightColor(COLORREF crHilight);
	

	// Returns: An RGB value that represents the tab highlight color.
	// Summary:	This member function gets an RGB value that represents the highlight
	//			color of the tab. 
	COLORREF GetTabHilightColor() const;
	
	// Input:	crBack - RGB value that represents the tab background color.
	// Summary:	This member function is called to set the background color for normal tabs.
	void SetTabBackColor(COLORREF crBack);
	
	// Returns: An RGB value that represents the tab background color.
	// Summary:	This member function gets an RGB value that represents the background
	//			color of the tab. 
	COLORREF GetTabBackColor() const;
	
	// Input:	crText - RGB value that represents the tab text color.
	// Summary:	This member function is called to set the text color for normal tabs.
	void SetTabTextColor(COLORREF crText);
	
	// Returns: An RGB value that represents the tab text color.
	// Summary:	This member function gets an RGB value that represents the text color
	//			of the tab. 
	COLORREF GetTabTextColor() const;
	
	// Input:	crBack - RGB value that represents the selected tab background color.
	// Summary:	This member function is called to set the background color for selected tabs.
	void SetSelTabBackColor(COLORREF crBack);
	
	// Returns: An RGB value that represents the selected tabs background color.
	// Summary:	This member function gets an RGB value that represents the background
	//			color for selected tabs. 
	COLORREF GetSelTabBackColor() const;
	
	// Input:	crText - RGB value that represents the selected tab text color.
	// Summary:	This member function is called to set the text color for selected tabs.
	void SetSelTabTextColor(COLORREF crText);
	
	// Returns: An RGB value that represents the selected tab text color.
	// Summary:	This member function gets an RGB value that represents the text color
	//			for selected tabs. 
	COLORREF GetSelTabTextColor() const;

	// Summary: Call this member function to reset the tab control to use default system colors.
	void UpdateDefaultColors();

	// Ignore:
	//{{AFX_VIRTUAL(CXTFlatTabCtrl)
	virtual BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnNotify(WPARAM, LPARAM lParam, LRESULT* pResult);
    virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	//}}AFX_VIRTUAL
	
protected:

	// Input:	pt - A CPoint reference representing the specified screen position.
	// Returns: The zero-based index of the button within the button's array or -1 if no 
	//			button is at the specified position.
	// Summary:	Call this function to determine which button, if any, is at the specified
	//			screen position. 
	virtual int ButtonHitTest(CPoint& pt) const;

	// Input:	nItem - The zero-based index of the tab to retrieve the width for.
	// Returns: The width, in pixels, of the tab.
	// Summary:	This member function will get the width, in pixels, of the tab specified
	//			by 'nItem'. 
	int GetTabWidth(int nItem) const;

	// Returns: The total combined width, in pixels, of all the tabs in the control.
	// Summary:	This member function will return the total width of all of the tabs
	//			in the flat tab control. 
	int GetTotalTabWidth() const;

	// Returns: The total width, in pixels, of all the visible arrow buttons.
	// Summary:	This member function will return the total width of all of the arrow
	//			buttons that are visible in the flat tab control. 
	int GetTotalArrowWidth() const;

	// Returns: The total area width, in pixels, of all the tabs in the flat tab control.
	// Summary:	This member function will return the total area width of all of the
	//			tabs in the flat tab control. 
	int GetTotalTabAreaWidth() const;

	// Input:	pDC - Points to the device context to draw the tab to.
	//			pt - XY location of the top left corner of the tab to draw.
	//			bSelected - true if the tab is currently selected.
	//			lpszTabLabel - A NULL terminated string that represents the tab label.
	// Returns: The x position of the next tab to be drawn.
	// Summary:	This member function will draw a tab to the device context specified
	//			by 'pDC'. 
	int DrawTab(CDC* pDC,const CPoint& pt,bool bSelected,LPCTSTR lpszTabLabel);

	// Input:	pDC - Points to the device context to draw the tab to.
	//			button_state - XY location of the top left corner of the tab to draw.
	// Summary:	This member function is used by the flat tab control to draw an arrow 
	//			button to the device context specified by 'pDC'.
	void DrawButton(CDC* pDC,CXTFTButtonState& button_state) const;

	// Summary: This member function will force all of the tabs to be repainted.
	void InvalidateTabs();

	// Summary: This member function will enable or disable the arrow buttons
	//			depending on the current tab display state.
	void EnableButtons();

	// Summary: This member function will free the resources allocated for the
	//			icons used by the arrow buttons.
	void FreeButtonIcons();

	// Summary: This member function will create the icon resources that are
	//			used by the arrow buttons.
	void CreateButtonIcons();

	// Input:	pDC - Points to the device context to draw the gripper to.
	//			rect - Location of the gripper.
	// Summary:	This member function will draw the horizontal sizing gripper at a 
	//			specified location.
	void DrawGripper(CDC* pDC,CRect rect) const;

	// Input:	bTracking - true to enable tracking, or false to disable tracking.
	// Summary:	The member function is used internally to toggle the state of the
	//			sizing-grip tracking mode.
	void SetTracking(bool bTracking);

	// Summary: This member function frees all memory occupied by the tab items.
	void ClearAllItems();

	// Summary: This member function is called when the tab control is resized.  It 
	//			is responsible for updating internal structures which are dependant
	//			on the control's size.
	void RecalcLayout();

	// Input:	nItem - Index of the tab to delete.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This internal function deletes an item from the tab item list. 
	virtual BOOL _DeleteItem(int nItem);

	// Returns: An integer value that represents the overlap between the tabs.
	// Summary:	This internal function calculates the overlap between two tabs. 
	virtual int GetOverlap() const;

	// Ignore:
	//{{AFX_MSG(CXTFlatTabCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLeftArrow();
	afx_msg void OnRightArrow();
	afx_msg void OnHomeArrow();
	afx_msg void OnEndArrow();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE BOOL CXTFlatTabCtrl::InsertItem(int nItem, UINT nTextID, CWnd *pWndControl) {
	ASSERT(IsWindow(m_hWnd)); CString strItem; strItem.LoadString(nTextID); return InsertItem(nItem, strItem, pWndControl);
}
AFX_INLINE int CXTFlatTabCtrl::GetCurSel() const {
	ASSERT(IsWindow(m_hWnd)); return m_nCurSel;
}
AFX_INLINE int CXTFlatTabCtrl::GetItemCount() const {
	ASSERT(IsWindow(m_hWnd)); int iItemCount = (int)m_tcbItems.GetSize(); return iItemCount;
}
AFX_INLINE void CXTFlatTabCtrl::SetTabFonts(CFont* pNormFont, CFont* pBoldFont) {
	if (pNormFont){ m_pNormFont = pNormFont; } if (pBoldFont){ m_pBoldFont = pBoldFont; }
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTFLATTABCTRL_H__)