// XTShellListView.h : header file
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

#ifndef __XTSHELLLISTVIEW_H__
#define __XTSHELLLISTVIEW_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTShellListView is a multiple inheritance class derived from CXTListView
//			and CXTShellPidl. It is used to create a CXTShellListView class object.
class _XT_EXT_CLASS CXTShellListView : public CXTListView, public CXTShellPidl
{
	DECLARE_DYNCREATE(CXTShellListView)

public:

    // Summary: Constructs a CXTShellListView object.
    CXTShellListView();

    // Summary: Destroys a CXTShellListView object, handles cleanup and de-allocation.
    virtual ~CXTShellListView();

protected:

	UINT				m_uFlags;		// Flags indicating which items to include in the enumeration.
	BOOL				m_bUninitOLE;	// TRUE if OleUninitialize has to be called. 
	BOOL				m_bContextMenu; // TRUE to display the shell context menu on right item click.
	LPITEMIDLIST		m_pidlINet;		// Points to the CSIDL_INTERNET folder location.
	CXTShellSettings	m_shSettings;	// Contains SHELLFLAGSTATE info.

	// Input:	lptvid - Pointer to tree view item data.
	//			lpsf - Pointer to the parent shell folder.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function adds items to the list view. 
    virtual BOOL InitListViewItems(XT_TVITEMDATA* lptvid,LPSHELLFOLDER lpsf);

	// Input:	lpifq - Fully qualified item ID list for the current item.
	//			lptvitem - Pointer to the tree view item about to be added to the tree.
    // Summary:	This member function gets the index for the normal and selected
	//			icons of the current item.
    virtual void GetNormalAndSelectedIcons(LPITEMIDLIST lpifq,LPTV_ITEM lptvitem);

public:

	// Input:	bEnable - TRUE to display a context menu.
    // Summary:	Call this member function to enable or disable the display of the
	//			shell context menu on the right click of the item.
    virtual void EnableContextMenu(BOOL bEnable);

	// BULLETED LIST:

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
	//			in the shell enumeration. The default is SHCONTF_FOLDERS | SHCONTF_NONFOLDERS.
    virtual void SetEnumFlags(UINT uFlags);

	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function associates the system image list with the list
	//			control. 
    virtual BOOL InitSystemImageLists();

	// Input:	lptvid - Pointer to TreeView item data.
	//			lpsf - Pointer to the parent shell folder.
	// Returns: TRUE if successful, otherwise return FALSE.
    // Summary:	This member function populates the list view control. 
    virtual BOOL PopulateListView(XT_TVITEMDATA* lptvid,LPSHELLFOLDER lpsf);

	// Returns: The index of the item that was double clicked, or -1, if the item 
	//			was not found.
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
    // Summary:	This member function retrieves the path for the specified list view
	//			item. 
    virtual BOOL GetItemPath(int iItem,CString &strItemPath);

    // Summary: This member function creates default columns for the list view.
    virtual void BuildDefaultColumns();

	// Input:	nCol - Passed in from the control. The index of the column clicked.
	//			bAscending  - Passed in from the control. true if the sort order should be ascending.
	// Returns: true if successful, otherwise returns false.
    // Summary:	Override this member function in your derived class to perform custom
	//			sort routines. 
    virtual bool SortList(int nCol,bool bAscending );

    // Ignore:
	//{{AFX_VIRTUAL(CXTShellListView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
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
	//{{AFX_MSG(CXTShellListView)
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginRDrag(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTShellListView::SetEnumFlags(UINT uFlags) {
	m_uFlags = uFlags;
}
AFX_INLINE void CXTShellListView::EnableContextMenu(BOOL bEnable) {
	m_bContextMenu = bEnable;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __XTSHELLLISTVIEW_H__