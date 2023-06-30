// XTTabCtrlBar.h interface for the CXTTabCtrlBar class.
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

#if !defined(__XTTABCTRLBAR_H__)
#define __XTTABCTRLBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTTabCtrlBar is a CXTDockWindow derived class.  It is used to implement
//			a tabbed window similar to Visual Studio's workspace window.
class _XT_EXT_CLASS CXTTabCtrlBar : public CXTDockWindow
{
	DECLARE_DYNAMIC(CXTTabCtrlBar)

public:
	
    // Summary: Constructs a CXTTabCtrlBar object.
	CXTTabCtrlBar();

	// Summary: Destroys a CXTTabCtrlBar object, handles cleanup and de-allocation.
    virtual ~CXTTabCtrlBar();

protected:

	CXTTabCtrl m_tabCtrl; // The tab control.

public:

	// Returns: A reference to the CXTTabCtrl object associated with this view.
	// Summary:	Call this member function to return a reference pointer to the CXTTabCtrl
	//			object associated with this view. 
	virtual CXTTabCtrl& GetTabCtrl () const;

	// Input:	pImageList - Pointer to the image list to be assigned to the tab control.
	// Returns: A pointer to the previous image list, or NULL, if there is no previous 
	//			image list.
	// Summary:	Call this member function to assign an image list to the tab control
	//			associated with this view. 
	virtual CImageList* SetTabImageList(CImageList *pImageList);

	// Input:	popupMenuID - ID for the tab control popup menu.
	//			nPos - Index position in the menu resource.
	// Summary:	This member function is used to set the resource ID for the popup menu
	//			used by the tab control.
	virtual void SetTabMenuID(UINT popupMenuID,int nPos=0);

	// Returns: The resource ID of the menu associated with the tab control.
	// Summary:	This member function returns the menu resource associated with the
	//			tab control. 
	virtual UINT GetTabMenuID();

	// Input:	rcChild - A reference to a CRect object to receive the client coordinates.
	// Summary:	This member function copies the child coordinates of the CTabCtrl client
	//			area into the object referenced by 'rcChild'.  The client coordinates
	//			specify the upper-left and lower-right corners of the client area. 
	virtual void GetChildRect(CRect& rcChild) const;

	// Input:	pView - A pointer to a CWnd object to be resized.
	// Summary:	Call this member function to resize the tab view specified by 'pView'.
	virtual void ResizeTabView(CWnd* pView);

	// Summary: This member function is called to initialize the font for the tab control
	//			associated with this view.
	virtual void InitializeFont();

	// Input:	dwRemove - Specifies window styles to be removed during style modification.
	//			dwAdd - Specifies window styles to be added during style modification.
	//			nFlags - Flags to be passed to SetWindowPos, or zero if SetWindowPos
	//			should not be called.  The default is zero.  See CWnd::ModifyStyle
	//			for more details.
	// Returns: Nonzero if style was successfully modified, otherwise returns zero.
	// Summary:	This member function will modify the style for the tab control associated
	//			with this view and set the appropriate font depending on the tab's
	//			orientation. 
	virtual BOOL ModifyTabStyle(DWORD dwRemove,DWORD dwAdd,UINT nFlags=0);

	// Returns: The handle of the tooltip control if successful, otherwise returns NULL.
	// Summary:	This member function retrieves the handle of the tooltip control associated
	//			with the tab control.  The tab control creates a tooltip control if
	//			it has the TCS_TOOLTIPS style.  You can also assign a tooltip control
	//			to a tab control by using the SetToolTips member function. 
	virtual CToolTipCtrl* GetTips();

	// Input:	pWndTip - Handle of the tooltip control.
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
	// Summary:	Call this function to update the tooltip text for the tab specified
	//			by 'pViewClass'.
	virtual void UpdateToolTip(CRuntimeClass *pViewClass,LPCTSTR lpszText);

	// Summary: This member function is called to reset the values for the tooltip
	//			control based upon the information stored for each tab.
	virtual void ResetToolTips();

	// Input:	bEnable - TRUE to enable tooltip usage.
	// Returns: TRUE if the tooltip control was found and updated, otherwise returns
	//			FALSE.
	// Summary:	Call this member function to enable or disable tooltip usage. 
	virtual BOOL EnableToolTips(BOOL bEnable);

	// Input:	lpszLabel - Pointer to the text for the tab associated with the view.
	//			pViewClass - CView runtime class associated with the tab.
	//			pDoc - CDocument associated with the view.
	//			pContext - Create context for the view.
	//			iIndex - Tab index of where to insert the new view. The default is -1 to
	//			insert the new view at the end.
	//			iIconIndex - Icon index for the tab.  If -1, 'iIndex' is used to determine
	//			the index.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to add a view to the tab control associated
	//			with this view. 
	virtual BOOL AddView(LPCTSTR lpszLabel,CRuntimeClass *pViewClass,CDocument* pDoc=NULL,CCreateContext* pContext=NULL,int iIndex=-1,int iIconIndex=-1);

	// Input:	lpszLabel - Pointer to the text for the tab associated with the view.
	//			pView - An existing view to be added to the tab control.
	//			iIndex - Tab index of where to insert the new view. The default is -1 to
	//			insert the new view at the end.
	//			iIconIndex - Icon index for the tab.  If -1, 'iIndex' is used to determine
	//			the index.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to add a view to the tab control associated
	//			with this view. 
    virtual BOOL AddView(LPCTSTR lpszLabel,CView* pView,int iIndex=-1,int iIconIndex=-1);

	// Input:	lpszLabel - Pointer to the text for the tab associated with the view.
	//			pView - CWnd object associated with the tab.
	//			iIndex - Tab index of where to insert the new view. The default is -1 to
	//			add the new view to the end.
	//			iIconIndex - Icon index for the tab.  If -1, 'iIndex' is used to determine
	//			the index.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function is called to add a control to the tab control
	//			associated with this view. 
	virtual BOOL AddControl(LPCTSTR lpszLabel,CWnd* pView,int iIndex=-1,int iIconIndex=-1);

	// Input:	pViewClass - CView runtime class associated with the tab.
	// Returns: A pointer to a CView object, otherwise returns NULL.
	// Summary:	This member function returns a pointer to a view from the specified
	//			runtime class. 
	virtual CWnd* GetView(CRuntimeClass *pViewClass);

	// Input:	nView - Tab index.
	// Returns: A pointer to a CView object, otherwise returns NULL.
	// Summary:	This member function returns a pointer to a view from the specified
	//			tab index. 
	virtual CWnd* GetView(int nView);

	// Returns: A pointer to the active view, otherwise returns NULL.
	// Summary:	This member function returns a pointer to the active view associated
	//			with the selected tab. 
	virtual CWnd* GetActiveView();

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
	// Summary:	This member function will remove the view specified by 'pView' from
	//			the tab control.
    virtual void DeleteView(CWnd* pView,BOOL bDestroyWnd=TRUE);

	// Input:	pViewClass - CView runtime class associated with the tab.
	//			bDestroyWnd - TRUE to destroy the list item.
	// Summary:	This member function will remove the view specified by 'pViewClass' from
	//			the tab control.
    virtual void DeleteView(CRuntimeClass *pViewClass,BOOL bDestroyWnd=TRUE);

	// Input:	nView - Tab index of the view.
	// Returns: A NULL terminated string that represents the tab item text.
	// Summary:	This member function will return the name for a view based on the tab index. 
    virtual LPCTSTR GetViewName(int nView);

	// Input:	pViewClass - CView runtime class associated with the tab.
	// Returns: A NULL terminated string that represents the tab item text.
	// Summary:	This member function will retrieve the name for a view based upon a CWnd object. 
    virtual LPCTSTR GetViewName(CRuntimeClass* pViewClass);

	// Input:	bDestroyWnd - TRUE to destroy the window associated with the tab item.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to remove all the tabs including all associated views. 
	virtual BOOL RemoveAllTabs(BOOL bDestroyWnd=TRUE);

	// Input:	point - Pointer to a CPoint object that contains the cursor screen coordinates.
	//			Use default for the current cursor position.
	// Returns: The zero-based index of the tab, or –1, if no tab is at the specified point.
	// Summary:	Call this member function to retrieve the tab index from the current
	//			cursor position. 
	virtual int GetTabFromPoint(CPoint point);
	
	// Input:	pView - A pointer to a CWnd object.
	// Returns: TRUE if the specified CWnd object is a child of the tab control, otherwise 
	//			returns FALSE.
	// Summary:	Call this member function to see if the specified CWnd object is a
	//			child of the tab control. 
	virtual BOOL IsChildView(CWnd* pView);

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

	// Returns: TRUE if auto-condense is enabled, otherwise returns FALSE.
	// Summary:	This member function returns the state of the tab control's auto-condense
	//			mode. See SetAutoCondense() for a full explanation of this mode. 
	BOOL GetAutoCondense();

protected:

    // Ignore:
	//{{AFX_VIRTUAL(CXTTabCtrlBar)
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	//}}AFX_VIRTUAL

	virtual void OnTabSelChange(int nIDCtrl, CXTTabCtrl* pTabCtrl);
	virtual void OnTabSelChanging(int nIDCtrl, CXTTabCtrl* pTabCtrl);

    // Ignore:
	//{{AFX_MSG(CXTTabCtrlBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	afx_msg LRESULT OnTabSelChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTabSelChanging(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE CXTTabCtrl& CXTTabCtrlBar::GetTabCtrl() const {
	ASSERT_VALID(this); return (CXTTabCtrl&)m_tabCtrl;
}
AFX_INLINE CImageList* CXTTabCtrlBar::SetTabImageList(CImageList *pImageList) {
	ASSERT_VALID(this); return GetTabCtrl().SetImageList(pImageList);
}
AFX_INLINE void CXTTabCtrlBar::SetTabMenuID(UINT popupMenuID, int nPos) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().SetMenuID(popupMenuID, nPos);
}
AFX_INLINE UINT CXTTabCtrlBar::GetTabMenuID() {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetMenuID();
}
AFX_INLINE void CXTTabCtrlBar::GetChildRect(CRect& rcChild) const {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().GetChildRect(rcChild);
}
AFX_INLINE void CXTTabCtrlBar::ResizeTabView(CWnd* pView) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().ResizeTabView(pView);
}
AFX_INLINE void CXTTabCtrlBar::InitializeFont() {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().InitializeFont();
}
AFX_INLINE BOOL CXTTabCtrlBar::ModifyTabStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().ModifyTabStyle(dwRemove, dwAdd, nFlags);
}
AFX_INLINE CToolTipCtrl* CXTTabCtrlBar::GetTips() {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetTips();
}
AFX_INLINE void CXTTabCtrlBar::SetTips(CToolTipCtrl* pWndTip) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().SetTips(pWndTip);
}
AFX_INLINE void CXTTabCtrlBar::AddToolTip(UINT nIDTab, LPCTSTR lpszText) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().AddToolTip(nIDTab, lpszText);
}
AFX_INLINE void CXTTabCtrlBar::UpdateToolTip(int nIDTab, LPCTSTR lpszText) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().UpdateToolTip(nIDTab, lpszText);
}
AFX_INLINE void CXTTabCtrlBar::UpdateToolTip(CRuntimeClass *pViewClass, LPCTSTR lpszText) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().UpdateToolTip(pViewClass, lpszText);
}
AFX_INLINE void CXTTabCtrlBar::ResetToolTips() {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().ResetToolTips();
}
AFX_INLINE BOOL CXTTabCtrlBar::EnableToolTips(BOOL bEnable) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().EnableToolTips(bEnable);
}
AFX_INLINE BOOL CXTTabCtrlBar::AddControl(LPCTSTR lpszLabel, CWnd* pView, int iIndex, int iIconIndex) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().AddControl(lpszLabel, pView, iIndex, iIconIndex);
}
AFX_INLINE CWnd* CXTTabCtrlBar::GetView(CRuntimeClass *pViewClass) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetView(pViewClass);
}
AFX_INLINE CWnd* CXTTabCtrlBar::GetView(int nView) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetView(nView);
}
AFX_INLINE CWnd* CXTTabCtrlBar::GetActiveView() {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetActiveView();
}
AFX_INLINE void CXTTabCtrlBar::SetActiveView(CRuntimeClass *pViewClass) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().SetActiveView(pViewClass);
}
AFX_INLINE void CXTTabCtrlBar::SetActiveView(CWnd* pTabView) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().SetActiveView(pTabView);
}
AFX_INLINE void CXTTabCtrlBar::SetActiveView(int nActiveTab) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().SetActiveView(nActiveTab);
}
AFX_INLINE void CXTTabCtrlBar::DeleteView(int nView, BOOL bDestroyWnd/*=TRUE*/) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().DeleteView(nView, bDestroyWnd);
}
AFX_INLINE void CXTTabCtrlBar::DeleteView(CWnd* pView, BOOL bDestroyWnd/*=TRUE*/) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().DeleteView(pView, bDestroyWnd);
}
AFX_INLINE void CXTTabCtrlBar::DeleteView(CRuntimeClass *pViewClass, BOOL bDestroyWnd/*=TRUE*/) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); GetTabCtrl().DeleteView(pViewClass, bDestroyWnd);
}
AFX_INLINE LPCTSTR CXTTabCtrlBar::GetViewName(int nView) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetViewName(nView);
}
AFX_INLINE LPCTSTR CXTTabCtrlBar::GetViewName(CRuntimeClass *pViewClass) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetViewName(pViewClass);
}
AFX_INLINE BOOL CXTTabCtrlBar::RemoveAllTabs(BOOL bDestroyWnd/*=TRUE*/) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().RemoveAllTabs(bDestroyWnd);
}
AFX_INLINE int CXTTabCtrlBar::GetTabFromPoint(CPoint point) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().GetTabFromPoint(point);
}
AFX_INLINE BOOL CXTTabCtrlBar::IsChildView(CWnd* pView) {
	ASSERT(::IsWindow(GetTabCtrl().m_hWnd)); return GetTabCtrl().IsChildView(pView);
}
AFX_INLINE void CXTTabCtrlBar::SetAutoCondense(BOOL bEnable) {
    GetTabCtrl().SetAutoCondense(bEnable);
}
AFX_INLINE BOOL CXTTabCtrlBar::GetAutoCondense() {
    return GetTabCtrl().GetAutoCondense();
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTTABCTRLBAR_H__)