// XTShellListCtrl.h : header file
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

#if !defined(__XTSHELLLISTCTRL_H__)
#define __XTSHELLLISTCTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTShellListCtrl is a multiple inheritance class derived from CXTListCtrl
//			and CXTShellPidl. It is used to create a CXTShellListCtrl window.
class _XT_EXT_CLASS CXTShellListCtrl : public CXTListCtrl, public CXTShellPidl
{

public:

    // Summary: Constructs a CXTShellListCtrl object.
    CXTShellListCtrl();

    // Summary: Destroys a CXTShellListCtrl object, handles cleanup and de-allocation.
    virtual ~CXTShellListCtrl();

protected:

	UINT				m_uFlags;		// Flags indicating which items to include in the enumeration.
	BOOL				m_bContextMenu; // TRUE to display the shell context menu on right item click.
	CWnd*				m_pTreeCtrl;	// Window that receives update notification, usually a CXTShellTreeCtrl.
	LPITEMIDLIST		m_pidlINet;		// Points to the CSIDL_INTERNET folder location.
	CXTShellSettings	m_shSettings;	// Contains SHELLFLAGSTATE info.

	// Input:	lptvid - Pointer to tree view item data.
	//			lpsf - Pointer to the parent shell folder.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function adds items to the list view. 
    virtual BOOL InitListViewItems(XT_TVITEMDATA* lptvid,LPSHELLFOLDER lpsf);

	// Input:	lpifq - Fully qualified item ID list for the current item.
	//			lptvitem - Pointer to a tree view item about to be added to the tree.
    // Summary:	This member function gets the index for the normal and selected icons
	//			of the current item.
    virtual void GetNormalAndSelectedIcons(LPITEMIDLIST lpifq,LPTV_ITEM lptvitem);

public:

	// Input:	bEnable - TRUE to display a context menu.
    // Summary:	Call this member function to enable or disable the display of the shell
    //			context menu on the right click of an item.
    virtual void EnableContextMenu(BOOL bEnable);

	// Input:	uFlags - Determines the type of items included in an enumeration, and can
	//			be one or more of the following:
    //			[ul]
    //			[li]<b>SHCONTF_FOLDERS</b> Include items that are folders in
	//			the enumeration.[/li]
    //			[li]<b>SHCONTF_NONFOLDERS</b> Include items that are not folders
	//			in the enumeration.[/li]
    //			[li]<b>SHCONTF_INCLUDEHIDDEN</b> Include hidden items in the
	//			enumeration.[/li]
    //			[li]<b>SHCONTF_INIT_ON_FIRST_NEXT</b> IShellFolder::EnumObjects
	//			can return without validating the enumeration object. Validation
	//			can be postponed until the first call to IEnumIDList::Next.
	//			This flag is intended to be used when a user interface may be
	//			displayed prior to the first IEnumIDList::Next call. For a
	//			user interface to be presented, 'hwndOwner' must be set to a valid
	//			window handle.[/li]
    //			[li]<b>SHCONTF_NETPRINTERSRCH</b> The caller is looking for
	//			printer objects.[/li]
    //			[/ul]
    // Summary:	Call this member function to determine the type of items included
	//			in the shell enumeration.  Default is SHCONTF_FOLDERS | SHCONTF_NONFOLDERS.
    virtual void SetEnumFlags(UINT uFlags);

	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function associates the system image list with the list
	//			control. 
    virtual BOOL InitSystemImageLists();

	// Input:	lptvid - Pointer to TreeView item data
	//			lpsf - Pointer to the parent shell folder.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function populates the list view control. 
    virtual BOOL PopulateListView(XT_TVITEMDATA* lptvid,LPSHELLFOLDER lpsf);

	// Returns: The index of the item that was double clicked, or -1, if the item was 
	//			not found.
    // Summary:	This member function returns the index of the list view item that
	//			was double clicked on.  
    virtual int GetDoubleClickedItem();

	// Returns: The index of the item selected, or -1, if the item was not found. 
    // Summary:	This member function displays the system popup menu for the selected
	//			item or folder.  
    virtual int GetContextMenu();

	// Input:	iItem - Index of the list view item clicked on.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function executes, via the Shell, the item clicked on
	//			in the list control.  
    virtual bool ShellOpenItem(int iItem);

	// Input:	lplvid - Pointer to the list view item data.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function executes, via the Shell, the item clicked on
	//			in the list control.  
	virtual bool ShellOpenItem(XT_LVITEMDATA* lplvid);

	// Input:	iItem - Index of the list view item to get the path of.
	//			strItemPath - Reference to a CString object that receives the path string.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function retrieves the path of the specified list view item. 
    virtual BOOL GetItemPath(int iItem,CString &strItemPath);

    // This member function creates default columns for the list view.
    virtual void BuildDefaultColumns();

	// Input:	nCol - Passed in from the control. The index of the column clicked.
	//			bAscending  - Passed in from the control,  true if the sort order should
	//			be ascending.
	// Returns: true if successful, otherwise returns false.
    // Summary:	Override this member function in your derived class to perform custom
	//			sort routines. 
    virtual bool SortList(int nCol,bool bAscending );

	// Input:	pWnd - Points to the tree control that is associated with the list.
    // Summary:	Call this member function to associate the tree control with the list.
    virtual void AssociateTree(CWnd* pWnd);

	// Call this member function to initialize the shell list control.
	virtual void InitializeControl();

    // Ignore:
	//{{AFX_VIRTUAL(CXTShellListCtrl)
    protected:
    //}}AFX_VIRTUAL

	virtual void OnDragDrop(NM_LISTVIEW* pNMListView);
	virtual TCHAR* InsertCommas(LONGLONG value, TCHAR* szBufferOut, UINT nSize);

protected:

	// Input:	iItem - Index of the item to set the attributes for.
	//			dwAttributes - Flags retrieved from SHELLFOLDER::GetAttributesOf.
    // Summary:	This member function sets the shell attribute flags for the specified
	//			list item.
    virtual void SetAttributes(int iItem,DWORD dwAttributes);

    // Ignore:
	//{{AFX_MSG(CXTShellListCtrl)
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginRDrag(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
    afx_msg LRESULT OnUpdateShell(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};

#define CXTShellList CXTShellListCtrl

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTShellListCtrl::SetEnumFlags(UINT uFlags) {
    m_uFlags = uFlags;
}
AFX_INLINE void CXTShellListCtrl::AssociateTree(CWnd* pWnd) {
    ASSERT_VALID(pWnd); m_pTreeCtrl = pWnd;
}
AFX_INLINE void CXTShellListCtrl::EnableContextMenu(BOOL bEnable) {
    m_bContextMenu = bEnable;
}

/////////////////////////////////////////////////////////////////////////////
// Summary: CXTShellListCtrlEx is a CXTShellListCtrl derived class.  It is used to create
//			a stand-alone shell list control that is not dependant on a CXTShellTreeCtrl
//			for initialization. It is used to create a CXTShellListCtrlEx window for 
//			displaying the contents of file folders.
class _XT_EXT_CLASS CXTShellListCtrlEx : public CXTShellListCtrl
{

public:

    // Summary: Constructs a CXTShellListCtrlEx object.
	CXTShellListCtrlEx();

    // Summary: Destroys a CXTShellListCtrlEx object, handles cleanup and de-allocation.
	virtual ~CXTShellListCtrlEx();

protected:

	CWnd*   m_pSyncWnd;    // Points to the window to synchronize with the shell list control.
	CString m_strItemPath; // A NULL terminated string that represents the currently selected folder.

public:

	// Input:	lpszPath - A NULL terminated string that represents the folder to select.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to browse to a particular file folder.  
	bool BrowseToFolder(LPCTSTR lpszPath);
	
	// Input:	pSyncWnd - Points to a valid CWnd object.
	// Summary:	This member function will associate a CWnd object with the shell list control.  This
	//			window is usually a CEdit control, but can be any CWnd object.  This window will have
	//			its window text updated whenever the selected folder is changed.
	void SetSyncWnd(CWnd* pSyncWnd);

	// Ignore:
	//{{AFX_VIRTUAL(CXTShellListCtrlEx)
	//}}AFX_VIRTUAL

protected:

	// Ignore:
	//{{AFX_MSG(CXTShellListCtrlEx)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTShellListCtrlEx::SetSyncWnd(CWnd* pSyncWnd) {
	m_pSyncWnd = pSyncWnd;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTSHELLLISTCTRL_H__)