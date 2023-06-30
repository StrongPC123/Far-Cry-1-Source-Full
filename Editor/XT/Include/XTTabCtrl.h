// XTTabCtrl.h interface for the CXTTabCtrl class.
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

#if !defined(__XTTABCTRL_H__)
#define __XTTABCTRL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define MESSAGE_MAP_ENTRIES_CXTTabCtrlBase \
	ON_WM_PAINT() \
	ON_WM_ERASEBKGND() \
	ON_WM_SETTINGCHANGE() \
	ON_WM_SYSCOLORCHANGE()

#define MESSAGE_MAP_ENTRIES_CXTTabCtrlBaseEx \
	MESSAGE_MAP_ENTRIES_CXTTabCtrlBase \
	ON_WM_RBUTTONDOWN() \
	ON_WM_CREATE() \
	ON_WM_DESTROY() \
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange) \
	ON_NOTIFY_REFLECT(TCN_SELCHANGING, OnSelchanging) \
	ON_WM_WINDOWPOSCHANGED() \
	ON_MESSAGE(XTWM_INITIAL_UPDATE, OnInitialize) \
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)

class CXTTabCtrlButtons;

// Summary: XT_NAVBTNFLAGS is an enumeration used by CXTTabCtrl to determine navigation button style.
enum XT_NAVBTNFLAGS
{
	XT_SHOW_ARROWS = 1, // To show arrow buttons.
	XT_SHOW_CLOSE = 2,  // To show close button.
	XT_SHOW_ALL = 3     // To show arrow and close buttons.
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTTabCtrlBase is a stand alone base class.  It is used to draw an XP
//			style tab control.
class _XT_EXT_CLASS CXTTabCtrlBase
{
public:

    // Summary: Constructs a CXTTabCtrlBase object.
	CXTTabCtrlBase();

	// Summary: Destroys a CXTTabCtrlBase object, handles cleanup and de-allocation.
	virtual ~CXTTabCtrlBase();

protected:

	int			m_iEdge;	// System metrics for SM_CYEDGE (XP only).
	CPen		m_penBlack; // Black pen (XP only).
	CPen		m_penWhite; // White pen (XP only).
	CPen		m_penFace;	// 3D face pen (XP only).
	CPen		m_penText;	// Non-selected text pen (XP only).
	CTabCtrl*	m_pTabCtrl; // Pointer to the tab control associated with this object.

	// Input:	pTabCtrl - Points to a valid tab control object.
	// Summary:	Call this member function to associate the tab control with this object.
	void ImplAttach(CTabCtrl *pTabCtrl);

public:

	bool m_bBoldFont;		// true to set the selected tab font to bold.
	bool m_bXPBorder;		// true to draw an XP border around the tab child window.
	bool m_bDisableXPMode;	// true to override system defaults and disable XP look for this control.
	BOOL m_bAutoCondensing; // TRUE for auto-condensing tabs.

public:

	// Input:	rcChild - A reference to a CRect object to receive the client coordinates.
	// Summary:	This member function copies the child coordinates of the CTabCtrl client
	//			area into the object referenced by 'rcChild'.  The client coordinates
	//			specify the upper-left and lower-right corners of the client area. 
	virtual void GetChildRect(CRect& rcChild) const;

	// BULLETED LIST:

	// Input:	dwFlags - The value can be one or more of the following:
    //			[ul]
    //			[li]<b>XT_SHOW_ARROWS</b> To show arrow buttons.[/li]
    //			[li]<b>XT_SHOW_CLOSE</b> To show close button.[/li]
	//			[li]<b>XT_SHOW_ALL</b> To show arrow and close buttons.[/li]
    //			[/ul]
	// Summary:	Call this member function to set visibility of the navigation buttons.  These
	//			buttons are used inplace of the default forward and back buttons that are
	//			displayed when the tab control is not wide enough to display all tabs.  You can
	//			also define a close button to be used to close the active tab.  This will give
	//			the tab control a VS.NET style tabbed interface.
	void ShowNavButtons(DWORD dwFlags);

protected:

	// Input:	pDC - Points to the client device context.
	//			rcClient - Size of the client area to paint.
	//			rcItem - Size of the selected tab item.
    // Summary:	This member function is called by the tab control to draw top aligned
	//			XP style tabs.
    virtual void OnDrawTop(CDC* pDC,CRect rcClient,CRect rcItem);

	// Input:	pDC - Points to the client device context.
	//			rcClient - Size of the client area to paint.
	//			rcItem - Size of the selected tab item.
    // Summary:	This member function is called by the tab control to draw bottom aligned
	//			XP style tabs.
	virtual void OnDrawBottom(CDC* pDC,CRect rcClient,CRect rcItem);

	// Input:	pDC - Points to the client device context.
	//			rcClient - Size of the client area to paint.
	//			rcItem - Size of the selected tab item.
    // Summary:	This member function is called by the tab control to draw left aligned
	//			XP style tabs.
    virtual void OnDrawLeft(CDC* pDC,CRect rcClient,CRect rcItem);

	// Input:	pDC - Points to the client device context.
	//			rcClient - Size of the client area to paint.
	//			rcItem - Size of the selected tab item.
    // Summary:	This member function is called by the tab control to draw right aligned
	//			XP style tabs.
    virtual void OnDrawRight(CDC* pDC,CRect rcClient,CRect rcItem);

	// Input:	strLabelText - Tab label to add padding to.
    // Summary:	This member function is called by the tab control to add padding to a
	//			tab label for use with XP style tabs.
    virtual void OnAddPadding(CXTString& strLabelText);

	BOOL OnEraseBkgndImpl(CDC* pDC);
	void OnPaintImpl();
	void OnSettingChangeImpl_Post(UINT uFlags, LPCTSTR lpszSection);
	void OnSysColorChangeImpl_Post();
	void PaintButtons();
	CXTTabCtrlButtons* m_pNavBtns;
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTTabCtrlBaseEx is a CXTTabCtrlBase derived class.  It is used to create
//			a CXTTabCtrlBaseEx class object.
class _XT_EXT_CLASS CXTTabCtrlBaseEx : public CXTTabCtrlBase
{
protected:
	
	int			m_nPos;				// Index of the popup menu contained in the menu.
	UINT		m_popupMenuID;		// Popup menu resource ID.
	BOOL		m_bInitialUpdate;	// TRUE to send initial update to views when created.
	CWnd*		m_pParentWnd;		// Points to the parent and will equal 'm_pParentFrame' in non-dialog applications.
	CView*		m_pLastActiveView;	// Points to the last active view that belongs to the main frame window.
	CFrameWnd*	m_pParentFrame;		// Points to the parent frame.


private:

	int        m_nOldIndex;
	BOOL       m_bInitialUpdatePending;
	DWORD      m_dwInitSignature;

public:

    CList <XT_TCB_ITEM*, XT_TCB_ITEM*> m_tcbItems; // Template list containing tab information.

    // Summary: Constructs a CXTTabCtrlBase object.
	CXTTabCtrlBaseEx();

	// Summary: Destroys a CXTTabCtrlBase object, handles cleanup and de-allocation.
	virtual ~CXTTabCtrlBaseEx();

	// Summary: This member function is called to initialize the font for the tab control
	//			associated with this view.
    virtual void InitializeFont();

	// Returns: The handle of the tooltip control if successful, otherwise returns NULL.
	// Summary:	This member function retrieves the handle of the tooltip control associated
	//			with the tab control.  The tab control creates a tooltip control if
	//			it has the TCS_TOOLTIPS style.  You can also assign a tooltip control
	//			to a tab control by using the SetToolTips member function. 
	virtual CToolTipCtrl* GetTips();

	// Input:	pWndTip - Pointer to a tooltip control.
	// Summary:	Call this function to assign a tooltip control to the tab control.
	//			You can associate the tooltip control with a tab control by making
	//			a call to GetToolTips.
	virtual void SetTips(CToolTipCtrl* pWndTip);

	// Input:	nIDTab - Index of the tab.
	//			lpszText - Pointer to the text for the tool.
	// Summary:	Call this function to register a tab with the tooltip control, so
	//			that the information stored in the tooltip is displayed when the cursor
	//			is on the tab.
	virtual void AddToolTip(UINT nIDTab,LPCTSTR lpszText);

	// Input:	nIDTab - Index of the tab.
	//			lpszText - Pointer to the text for the tool.        
	// Summary:	Call this function to update the tooltip text for the specified tab.
	virtual void UpdateToolTip(int nIDTab,LPCTSTR lpszText);

	// Input:	pViewClass - CRuntimeClass associated with the tab.
	//			lpszText - Pointer to the text for the tool.
	// Summary:	Call this function to update the tooltip text for the specified tab.
	virtual void UpdateToolTip(CRuntimeClass *pViewClass,LPCTSTR lpszText);

	// Summary: This member function is called to reset the values for the tooltip
	//			control based upon the information stored for each tab.
	virtual void ResetToolTips();

	// Input:	bEnable - TRUE to enable tooltip usage.
	// Returns: TRUE if the tooltip control was found and updated, otherwise returns
	//			FALSE.
	// Summary:	Call this member function to enable or disable tooltip usage. 
	virtual BOOL EnableToolTipsImpl(BOOL bEnable);
    
	// Input:	lpszLabel - Pointer to the text for the tab associated with the view.
	//			pViewClass - CView runtime class associated with the tab.
	//			pDoc - CDocument associated with the view.
	//			pContext - Create context for the view.
	//			iIndex - -1 to add to the end.
	//			iIconIndex - Icon index for the tab.  If -1, 'iIndex' is used to determine the index.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to add a view to the tab control associated
	//			with this view. 
    virtual BOOL AddView(LPCTSTR lpszLabel,CRuntimeClass *pViewClass,CDocument* pDoc=NULL,CCreateContext* pContext=NULL,int iIndex=-1,int iIconIndex=-1);

	// Input:	lpszLabel - Pointer to the text for the tab associated with the view.
	//			pView - An existing view to be added to the tab control.
	//			iIndex - -1 to add to the end.
	//			iIconIndex - Icon index for the tab.  If -1, nIndex is used to determine the index.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to add a view to the tab control associated
	//			with this view. 
    virtual BOOL AddView(LPCTSTR lpszLabel,CView* pView,int iIndex=-1,int iIconIndex=-1);

	// Input:	lpszLabel - Pointer to the text for the tab associated with the view.
	//			pWnd - CWnd object associated with the tab.
	//			iIndex - Tab index of where to insert the new view.  Default is -1 to add to
	//			the end.
	//			iIconIndex - Icon index for the tab.  If -1, 'iIndex' is used to determine the index.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function is called to add a control to the tab control
	//			associated with this view. 
    virtual BOOL AddControl(LPCTSTR lpszLabel,CWnd* pWnd,int iIndex=-1,int iIconIndex=-1);

	// Input:	pViewClass - CView runtime class associated with the tab.
	// Returns: A pointer to a CView object, otherwise returns NULL.
	// Summary:	This member function returns a pointer to a view from the specified
	//			runtime class. 
	virtual CWnd* GetView(CRuntimeClass *pViewClass);

	// Input:	nView - Tab index.
	// Returns: A pointer to a CView object, otherwise returns NULL.
	// Summary:	This member function returns a pointer to a view from the specified
	//			runtime class. 
	virtual CWnd* GetView(int nView);

	// Returns: A pointer to the active view, otherwise returns NULL.
	// Summary:	This member function returns a pointer to the active view associated
	//			with the selected tab. 
	virtual CWnd* GetActiveView();

	// Input:	pTabView - CWnd object to make active.
	// Summary:	This member function is called to activate the specified view and deactivate
	//			all remaining views.
	virtual void ActivateView(CWnd* pTabView);

	// Input:	pViewClass - CView runtime class associated with the tab.
	// Summary:	This member function will set a view active based on the specified
	//			runtime class.
	virtual void SetActiveView(CRuntimeClass *pViewClass);

	// Input:	pTabView - CWnd object to make active.
	// Summary:	This member function will set a view active based on the specified
	//			runtime class.
    virtual void SetActiveView(CWnd* pTabView);

	// Input:	nActiveTab - Tab index.
	// Summary:	This member function will set a view active based on the specified
	//			tab index.
    virtual void SetActiveView(int nActiveTab);

	// Input:	nView - Tab index of the view.
	//			bDestroyWnd - TRUE to destroy the list item.
	// Summary:	This member function will remove a view based on the specified
	//			tab index.
	virtual void DeleteView(int nView,BOOL bDestroyWnd=TRUE);

	// Input:	pView - Points to the CWnd object associated with the tab.
	//			bDestroyWnd - TRUE to destroy the list item.
	// Summary:	This member function will remove a view based on the specified CWnd
	//			pointer associated with the tab.
	virtual void DeleteView(CWnd* pView,BOOL bDestroyWnd=TRUE);

	// Input:	pViewClass - CView runtime class associated with the tab.
	//			bDestroyWnd - TRUE to destroy the list item.
	// Summary:	This member function will remove a view based on the specified CView
	//			runtime class associated with the tab.
    virtual void DeleteView(CRuntimeClass *pViewClass,BOOL bDestroyWnd=TRUE);

	// Input:	nView - Tab index of the view.
	// Returns: A NULL terminated string that represents the tab item text.
	// Summary:	This member function will return the name for a view based on the tab
	//			index. 
    virtual LPCTSTR GetViewName(int nView);

	// Input:	pViewClass - CView runtime class associated with the tab.
	// Returns: A NULL terminated string that represents the tab item text.
	// Summary:	This member function will return the name for a view based on the tab
	//			index. 
    virtual LPCTSTR GetViewName(CRuntimeClass *pViewClass);

	// Input:	pView - A pointer to a CWnd object to be resized.
	// Summary:	Call this member function to resize the tab view specified by 'pView'.
	virtual void ResizeTabView(CWnd* pView);

	// Input:	pos - The POSITION value of the item to be removed.
	//			bDestroyWnd - TRUE to destroy the list item.
	// Summary:	This member function is used by the tab control bar to remove an item
	//			from the tab view list.
	virtual void RemoveListItem(POSITION pos,BOOL bDestroyWnd=TRUE);

	// Input:	bDestroyWnd - TRUE to destroy the window associated with the tab item.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to remove all the tabs, including all associated
	//			views. 
	virtual BOOL RemoveAllTabs(BOOL bDestroyWnd=TRUE);

	// Input:	point - Pointer to a CPoint object that contains the cursor screen coordinates.
	//			Use default for current cursor position.
	// Returns: An integer based index of the tab, or –1, if no tab is at the specified 'point'.
	// Summary:	Call this member function to retrieve the tab index from the current cursor
	//			position. 
	virtual int GetTabFromPoint(CPoint point);

	// Input:	pView - A pointer to a CWnd object.
	// Returns: TRUE if the specified CWnd object is a child of the tab control, otherwise 
	//			returns FALSE.
	// Summary:	Call this member function to see if the specified CWnd object is a
	//			child of the tab control. 
	virtual BOOL IsChildView(CWnd* pView);

	// Input:	nTab - Index of the tab.
	//			lpszLabel - New text for the tab label.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to set the text for the specified tab. 
	BOOL SetTabText(int nTab,LPCTSTR lpszLabel);

	// Input:	pView - CWnd object associated with the tab.
	//			lpszLabel - New text for the tab label.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to set the text for the specified tab. 
    BOOL SetTabText(CWnd* pView,LPCTSTR lpszLabel);

	// Input:	pViewClass - CRuntimeClass of the CWnd associated with the tab.
	//			lpszLabel - New text for the tab label.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to set the text for the specified tab. 
    BOOL SetTabText(CRuntimeClass *pViewClass,LPCTSTR lpszLabel);

	// Returns: A CWnd pointer to the newly activated view.
	// Summary:	This member function is called to activate the next view in the tab
	//			control. 
	CWnd* NextView();

	// Returns: A CWnd pointer to the newly activated view.
	// Summary:	This member function is called to activate the previous view in the
	//			tab control. 
	CWnd* PrevView();

	// Input:	bEnable - TRUE to enable auto-condense mode.
	// Summary:	Call this member function to enable or disable the tab auto-condensing
	//			mode.  Auto-condensing mode affects the tab control's behavior when
	//			there is not enough room to fit all tabs.  Without auto-condensation,
	//			the CXTTabCtrl control behaves like a standard tab control (i.e. it
	//			will display a slider control that allows the user to pan between tabs).
	//			With the auto-condensing mode enabled, CXTTabCtrl attempts to fit all
	//			tabs in the available space by trimming the tab label text.  This behavior
	//			is similar to the behavior displayed by Visual C++'s Workspace View.
	//			For instance, you can see the FileView tab shrink if you shrink the
	//			Workspace View.
    void SetAutoCondense(BOOL bEnable);

	// Returns: TRUE if auto-condense is enabled, or FALSE if it is disabled.
	// Summary:	This member function returns the state of the tab control's auto-condense
	//			mode. See SetAutoCondense() for a full explanation of this mode. 
    BOOL GetAutoCondense();

	// Input:	dwRemove - Specifies window styles to be removed during style modification.
	//			dwAdd - Specifies window styles to be added during style modification.
	//			nFlags - Flags to be passed to SetWindowPos, or zero if SetWindowPos should
	//			not be called. The default is zero.  See CWnd::ModifyStyle for more
	//			details.
	// Returns: Nonzero if the style was successfully modified, otherwise returns zero. 
	// Summary:	This member function will modify the style for the tab control associated
	//			with this view and set the appropriate font depending on the tab's
	//			orientation. 
	virtual BOOL ModifyTabStyle(DWORD dwRemove,DWORD dwAdd,UINT nFlags=0);

	// Returns: A CView pointer to the last known view.
	// Summary:	This member function is used to get the last known view that belongs
	//			to the frame. 
	CView* GetLastKnownChildView();

	// Input:	popupMenuID - ID for the tab control popup menu.
	//			nPos - Index position in the menu resource.
	// Summary:	This member function is used to set the resource ID for the popup menu
	//			used by the tab control.
	virtual void SetMenuID(UINT popupMenuID,int nPos=0);

	// Returns: The resource ID of the menu associated with the tab control.
	// Summary:	This member function returns the menu resource associated with the
	//			tab control. 
	virtual UINT GetMenuID();

	// Input:	bInitialUpdate - TRUE to send initial update message.
	// Summary:	Call this member function to allow WM_INITIALUPATE message to be sent
	//			to views after creation.
	virtual void SendInitialUpdate(BOOL bInitialUpdate);

protected:

	// Summary: This member function is a virtual method that is called to handle a
	//			TCN_SELCHANGING event. Override in your derived class to add additional
	//			functionality.
	virtual void OnSelChanging();

	// Summary: This member function is a virtual method that is called to handle a
	//			TCN_SELCHANGE event. Override in your derived class to add additional
	//			functionality.
	virtual void OnSelChange();

	// Input:	nTab - Index of the tab.
	//			pMember - Address of an XT_TCB_ITEM struct associated with the tab.
	//			lpszLabel - NULL terminated string that represents the new tab label.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function is called to set the tooltip and tab text for
	//			the specified tab. 
	BOOL UpdateTabLabel(int nTab,XT_TCB_ITEM* pMember,LPCTSTR lpszLabel);

	// Input:	pViewClass - CView runtime class to be created.
	//			pDocument - CDocument associated with view.
	//			pContext - Create context for the view.
	// Returns: A pointer to the newly created CWnd object, otherwise returns NULL.
    // Summary:	This member function creates the CWnd object that is associated
	//			with a tab control item. 
	virtual CWnd* CreateTabView(CRuntimeClass *pViewClass,CDocument *pDocument,CCreateContext *pContext);

	// Input:	pDC - Points to the current device context.
	//			sLabel - Represents the tab label text.
	//			bHasIcon - Set to true if the tab item has an icon.
	// Returns: An integer value that represents the width of a tab.
    // Summary:	This member function is used internally by the tab control to calculate
	//			the width of a tab based on its label text. 
    int CalculateTabWidth(CDC *pDC,CString& sLabel,bool bHasIcon);

    // Summary: This member function is used internally by the tab control to shrink,
	//			or unshrink, tabs based on the control's width and the state of the
	//			auto-condensation mode. See SetAutoCondense() for more information.
    void Condense();

	void OnRButtonDownImpl(UINT nFlags, CPoint point);
	int OnCreateImpl_Post(LPCREATESTRUCT lpCreateStruct);
	void OnDestroyImpl_Pre();
	void OnSelchangeImpl(NMHDR* pNMHDR, LRESULT* pResult);
	void OnSelchangingImpl(NMHDR* pNMHDR, LRESULT* pResult);
	void OnWindowPosChangedImpl_Pre(WINDOWPOS FAR* lpwndpos);
	void OnWindowPosChangedImpl_Post(WINDOWPOS FAR* lpwndpos);
	BOOL PreTranslateMessageImpl(MSG* pMsg);
	void PreSubclassWindowImpl_Post();
	BOOL OnCmdMsgImpl_Pre(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	LRESULT OnInitializeImpl(WPARAM, LPARAM);
	void OnInitialUpdateImpl();
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTTabCtrl is a multiple inheritance class derived from CTabCtrl and
//			CXTTabCtrlBaseEx. It is used to create a CXTTabCtrl class object.  See
//			CXTTabCtrlBaseEx for additional functionality.
class _XT_EXT_CLASS CXTTabCtrl : public CTabCtrl, public CXTTabCtrlBaseEx
{
	DECLARE_DYNAMIC(CXTTabCtrl)

	friend class CXTTabCtrlBase;
	friend class CXTTabCtrlBaseEx;

public:
	
    // Summary: Constructs a CXTTabCtrl object.
	CXTTabCtrl();

	// Summary: Destroys a CXTTabCtrl object, handles cleanup and de-allocation.
    virtual ~CXTTabCtrl();

protected:

public:

	// Input:	bEnable - TRUE to enable tooltip usage.
	// Returns: TRUE if the tooltip control was found and updated, otherwise returns FALSE.
	// Summary:	Call this member function to enable or disable tooltip usage. 
	virtual BOOL EnableToolTips(BOOL bEnable);

    // Ignore:
	//{{AFX_VIRTUAL(CXTTabCtrl)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTTabCtrl)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnPaint();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG

	afx_msg LRESULT OnInitialize(WPARAM wp, LPARAM lp) { return OnInitializeImpl(wp, lp); };
	afx_msg void OnInitialUpdate() {OnInitialUpdateImpl(); }

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE UINT CXTTabCtrlBaseEx::GetMenuID() {
	ASSERT(::IsWindow(m_pTabCtrl->GetSafeHwnd())); return m_popupMenuID;
}
AFX_INLINE CWnd* CXTTabCtrlBaseEx::GetActiveView() {
	return GetView(m_pTabCtrl->GetCurSel());
}
AFX_INLINE void CXTTabCtrlBaseEx::SetMenuID(UINT popupMenuID, int nPos) {
	m_popupMenuID = popupMenuID; m_nPos = nPos;
}
AFX_INLINE void CXTTabCtrlBaseEx::SendInitialUpdate(BOOL bInitialUpdate) {
	m_bInitialUpdate = bInitialUpdate;
}
AFX_INLINE CView* CXTTabCtrlBaseEx::GetLastKnownChildView() {
 	return m_pLastActiveView;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTTABCTRL_H__)