// XTShellTreeView.h : header file
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

#ifndef __XTSHELLTREEVIEW_H__
#define __XTSHELLTREEVIEW_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTShellTreeView is a multiple inheritance class derived from CXTTreeView
//			and CXTShellPidl. It is used to create a CXTShellTreeView class object.
class _XT_EXT_CLASS CXTShellTreeView : public CXTTreeView, public CXTShellPidl
{
	DECLARE_DYNCREATE(CXTShellTreeView)

public:

    // Summary: Constructs a CXTShellTreeView object.
    CXTShellTreeView();

    // Summary: Destroys a CXTShellTreeView object, handles cleanup and de-allocation.
    virtual ~CXTShellTreeView();

protected:

	UINT			 m_uFlags;		 // Flags indicating which items to include in the enumeration.
	bool			 m_bTunneling;	 // true if tree is currently traversing.
	BOOL			 m_bContextMenu; // TRUE to display the shell context menu on right item click.
	CWnd*			 m_pComboBox;	 // CComboBox that is associated with this control.  See AssociateCombo(...)
	CXTShellSettings m_shSettings;	 // Contains SHELLFLAGSTATE info.

	// Summary: Enumeration that holds the type of object, folder or drive.
	enum FindAttribs 
	{
		type_drive,  // The object type is a drive.
		type_folder  // The object type is a folder.
	};

	// Input:	lpsf - Pointer to the parent shell folder.
	//			lpifq - Fully qualified item ID list to the item having items enumerated.
    //			This is the PIDL to the item identified by the 'lpsf' parameter.
	//			hParent - Parent tree node.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function fills a branch of the TreeView control.  Given
	//			the shell folder, it enumerates the subitems of this folder, and adds
	//			the appropriate items to the tree. 
    virtual BOOL InitTreeViewItems(LPSHELLFOLDER lpsf,LPITEMIDLIST lpifq,HTREEITEM hParent);

	// Input:	lpifq - Fully qualified item ID list for the current item.
	//			lptvitem - Pointer to the tree view item being added to the tree.
    // Summary:	This member function gets the index for the normal and selected
	//			icons for the current item.
    virtual void GetNormalAndSelectedIcons(LPITEMIDLIST lpifq,LPTV_ITEM lptvitem);

public:

	// Input:	bEnable - TRUE to display a context menu.
    // Summary:	Call this member function to enable or disable the display of the
	//			shell context menu on right item click.
    void EnableContextMenu(BOOL bEnable);

	// BULLETED LIST:

	// Input:	uFlags - Determines the type of items included in an enumeration. It
	//			can be one or more of the following:
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
	//			in the shell enumeration. The default is SHCONTF_FOLDERS | SHCONTF_NONFOLDERS.
    void SetEnumFlags(UINT uFlags);

	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function obtains a handle to the system image list and
	//			attaches it to the tree control. 
    virtual BOOL InitSystemImageLists();

    // Summary: This member function adds items to the tree view.
    virtual void PopulateTreeView();

	// Input:	pNMTreeView - Address of an NM_TREEVIEW struct.
    // Summary:	This member function responds to a TVN_ITEMEXPANDING message in
	//			order to fill up subdirectories.
    virtual void OnFolderExpanding(NM_TREEVIEW* pNMTreeView);

	// Returns: A handle to the currently selected HTREEITEM.
    // Summary:	This member function displays the system popup menu for the selected
	//			item or folder. 
    virtual HTREEITEM GetContextMenu();

	// Input:	pNMTreeView - Address of an NM_TREEVIEW struct.
	//			strFolderPath - Address of a CString object to receive the file system path.
	// Returns: TRUE if the folder path was found, otherwise returns FALSE.
    // Summary:	This member function responds to a TVN_SELCHANGED message to retrieve
	//			the specified folder path. 
    virtual BOOL OnFolderSelected(NM_TREEVIEW* pNMTreeView,CString &strFolderPath);

	// Input:	strFolderPath - Address of a CString object to receive the file system path.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function retrieves the path of the currently selected
	//			tree item. 
    virtual BOOL GetSelectedFolderPath(CString &strFolderPath);

	// Input:	hItem - Handle to the tree item to search from.
	//			lplvid - Pointer to the list view item data.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function recursively searches the tree control, beginning
	//			at 'hItem', to find the item specified by 'lplvid'. This is typically used when
	//			the user double clicks an item in the list view. 
    virtual BOOL FindTreeItem(HTREEITEM hItem,XT_LVITEMDATA* lplvid);

	// Input:	pWnd - Points to the combo box that is associated with the tree.
    // Summary:	This member function is used to associate a CComboBox object with
	//			the control.  Whenever the path changes, the combo is updated.
    virtual void AssociateCombo(CWnd* pWnd);

	// Input:	hItem - Handle to a tree node.
	//			lptvid - Pointer to the list view item data.
	// Summary:	This member function is called to initialize a branch of the shell
	//			tree.
    virtual void InitTreeNode(HTREEITEM hItem,XT_TVITEMDATA* lptvid);

	// Input:	hItem - Tree item to begin the search from.
	//			strSearchName - String to search for.
	//			attr - Looking for folder or drive.
	//			bFindRoot - TRUE to look for a drive letter only.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function searches the tree for a specified folder. 
    virtual BOOL SearchTree(HTREEITEM hItem,CString strSearchName,FindAttribs attr,BOOL bFindRoot=FALSE);

	// Input:	strFindPath - Path to find.
    // Summary:	This member function will "tunnel" the tree to find the specified
	//			path. This will only work when the PopulateTreeView() method is used
	//			to populate the tree.
    virtual void TunnelTree(CString strFindPath);

	// Input:	hItem - Tree item to get the path for.
	//			strFolderPath - Reference to a CString object to contain the folder path.
	// Returns: TRUE if the path is not in the file system(e.g.
	//			MyComputer), or if none is selected, it returns FALSE.
    // Summary:	This member function retrieves the path of a folder item, which
	//			does not have to be selected.  Pass a CString object that will hold
	//			the folder path. 
    virtual BOOL GetFolderItemPath(HTREEITEM hItem,CString &strFolderPath);

	// Input:	lpszPath - Path to populate.
    // Summary:	This member function populates a tree based upon a path.
    virtual void PopulateTree(LPCTSTR lpszPath);

    // Ignore:
	//{{AFX_VIRTUAL(CXTShellTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

	virtual void OnDragDrop(NM_TREEVIEW* pNMTreeView);
	virtual HTREEITEM InsertDesktopItem();

protected:

	// Input:	hItem - Handle to HTREEITEM node.
	//			dwAttributes - Flags retrieved from SHELLFOLDER::GetAttributesOf.
	// Summary:	This member function sets the shell attribute flags for the specified
	//			tree item.
	void SetAttributes(HTREEITEM hItem,DWORD dwAttributes);

    // Ignore:
	//{{AFX_MSG(CXTShellTreeView)
	afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginRDrag(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTShellTreeView::SetEnumFlags(UINT uFlags) {
	m_uFlags = uFlags;
}
AFX_INLINE void CXTShellTreeView::EnableContextMenu(BOOL bEnable) {
	m_bContextMenu = bEnable;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __XTSHELLTREEVIEW_H__