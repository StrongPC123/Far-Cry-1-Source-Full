// XTTreeCtrl.h interface for the CXTTreeCtrl class.
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

#if !defined(__XTTREECTRL_H__)
#define __XTTREECTRL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Summary: CMap definition for mapping XT_CLRFONT structures.
typedef CMap<void*,void*,XT_CLRFONT,XT_CLRFONT&> CXTColorFontMap;

//////////////////////////////////////////////////////////////////////
// Summary: CXTTreeCtrl is a CTreeCtrl derived class.  It extends the CTreeCtrl
//			class to add additional functionality, including tree traversal, searching,
//			color, and settings.
class _XT_EXT_CLASS CXTTreeCtrl : public CTreeCtrl
{
    DECLARE_DYNAMIC(CXTTreeCtrl)

public:

    // Summary:	Constructs a CXTTreeCtrl object.
    CXTTreeCtrl();

    // Summary:	Destroys a CXTTreeCtrl object, handles cleanup and de-allocation.
    virtual ~CXTTreeCtrl();

protected:

	BOOL			m_bMultiSelect; // TRUE for a multi-selection tree control.
	BOOL			m_bBandLabel;   // TRUE to include the label when selecting tree items.
	HTREEITEM		m_hSelect;      // For shift selection.
    CXTColorFontMap m_mapColorFont; // Struct that contains the color and logfont for the tree item.

private:

    bool m_bActionDone;
	bool m_bTreeNotify;
	bool m_bOkToEdit;

public:

	// BULLETED LIST:

	// Input:	hItem -  Handle of a tree item.
	//			nCode - A flag indicating the type of relation to 'hItem'.  This flag
	//			can be one of the following values:
    //			[ul]
    //			[li]<b>TVGN_CARET</b> Retrieves the currently selected item.[/li]
    //			[li]<b>TVGN_CHILD</b> Retrieves the first child item.  The
	//			'hItem' parameter <b>must</b> be NULL.[/li]
    //			[li]<b>TVGN_DROPHILITE</b> Retrieves the item that is the target
	//			of a drag-and-drop operation.[/li]
    //			[li]<b>TVGN_FIRSTVISIBLE</b> Retrieves the first visible item.[/li]
    //			[li]<b>TVGN_NEXT</b> Retrieves the next sibling item.[/li]
    //			[li]<b>TVGN_NEXTVISIBLE</b> Retrieves the next visible item
	//			that follows the specified item.[/li]
    //			[li]<b>TVGN_PARENT</b> Retrieves the parent of the specified
	//			item.[/li]
    //			[li]<b>TVGN_PREVIOUS</b> Retrieves the previous sibling item.[/li]
    //			[li]<b>TVGN_PREVIOUSVISIBLE</b> Retrieves the first visible
	//			item that precedes the specified item.[/li]
    //			[li]<b>TVGN_ROOT</b> Retrieves the first child item of the
	//			root item of which the specified item is a part.[/li]
    //			[/ul]
	// Returns: The handle of the next item if successful, otherwise returns NULL.
    // Summary:	This member function will retrieve the tree view item that has the
	//			specified relationship, indicated by the 'nCode' parameter, to 'hItem'.
    virtual HTREEITEM GetNextItem(HTREEITEM hItem,UINT nCode);

	// Input:	hItem - Handle of the reference item.
	// Returns: The item immediately below the reference item.
    // Summary:	This member function gets the next item as if the outline was completely
	//			expanded. 
    virtual HTREEITEM GetNextItem(HTREEITEM hItem);

	// Input:	hItem - Handle of the reference item.
	// Returns: The item immediately above the reference item.
    // Summary:	This member function gets the previous item as if the outline was
	//			completely expanded. 
    virtual HTREEITEM GetPrevItem(HTREEITEM hItem);

	// Input:	hItem - Node identifying the branch.  NULL will return the last item
	//			in the outline.
	// Returns: The handle of the last item.
    // Summary:	This member function gets the last item in the branch. 
    virtual HTREEITEM GetLastItem(HTREEITEM hItem);

	// Input:	lpszSearch - String to search for.
	//			bCaseSensitive - TRUE if the search should be case sensitive.
	//			bDownDir - TRUE for down.
	//			bWholeWord - TRUE if search should match whole words.
	//			hItem - Handle of the tree item to start searching from, NULL to use
	//			the currently selected tree item.
	// Returns: The handle to the item, or returns NULL.
    // Summary:	This member function finds an item that contains the search string.
    virtual HTREEITEM FindItem(LPCTSTR lpszSearch,BOOL bCaseSensitive=FALSE,BOOL bDownDir=TRUE,BOOL bWholeWord=FALSE,HTREEITEM hItem=NULL);

	// Input:	hItem - Handle of the reference item.
	//			logfont - New font for the tree item.
    // Summary:	This member function sets the font for the reference tree item.
    virtual void SetItemFont(HTREEITEM hItem,LOGFONT& logfont);

	// Input:	hItem - Handle of the reference item.
	//			plogfont - Pointer to receive LOGFONT information.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function retrieves the current LOGFONT font used by
	//			the tree item. 
    virtual BOOL GetItemFont(HTREEITEM hItem,LOGFONT* plogfont);

	// Input:	hItem - Handle of the reference item.
	//			bBold - TRUE for bold font.
    // Summary:	This member function sets the reference tree item font to bold.
    virtual void SetItemBold(HTREEITEM hItem,BOOL bBold=TRUE);

	// Input:	hItem - Handle of the reference item.
	// Returns: TRUE if the tree item has a bold font, otherwise returns FALSE.
    // Summary:	This member function checks to see if the tree item has a bold font.
    virtual BOOL GetItemBold(HTREEITEM hItem);

	// Input:	hItem - Handle of the reference item.
	//			color - RGB value for the tree item's text.
    // Summary:	This member function sets the tree item text color.
    virtual void SetItemColor(HTREEITEM hItem,COLORREF color);

	// Input:	hItem - Handle of the reference item.
	// Returns: An RGB value for the referenced tree item, or -1, if color was not set.
    // Summary:	This member function returns the RGB text value for the referenced
	//			tree item, or (COLORREF)-1, if color was not set. 
    virtual COLORREF GetItemColor(HTREEITEM hItem);

	// Input:	bMultiSelect - TRUE for multi-selection tree control.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function enables or disables multi-selection for the
	//			tree control. 
    virtual BOOL EnableMultiSelect(BOOL bMultiSelect=TRUE);

	// Input:	hItem - Handle of the reference item.
	// Returns: A handle to the previously selected tree item.
    // Summary:	This member function returns the previously selected tree item in
	//			a multi-selection tree control. 
    virtual HTREEITEM GetPrevSelectedItem(HTREEITEM hItem);

	// Input:	hItem - Handle of the reference item.
	// Returns: A handle to the next selected tree item.
    // Summary:	This member function returns the next selected item in a multi-selection
	//			tree control. 
	HTREEITEM GetNextSelectedItem(HTREEITEM hItem) const;

	// Returns: A handle to the first selected tree item.
    // Summary:	This member function retrieves the first selected item in a multi-selection
	//			tree control. 
	HTREEITEM GetFirstSelectedItem() const;

	// Returns: A UINT representing the number of tree items selected.
    // Summary:	This member function retrieves the number of tree items that are
	//			selected. 
	UINT GetSelectedCount() const;

	// Input:	hItem - Handle of the item whose state is to be set.
	//			nState - Specifies new states for the item.
	//			nStateMask - Specifies which states are to be changed.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function is a replacement for the base class function of
	//			the same name, to handle TVIS_FOCUSED in a multi-select tree control.
	//			It sets the state of the item specified by 'hItem'.  
	BOOL SetItemState(HTREEITEM hItem,UINT nState,UINT nStateMask);

	// Input:	hItem - Handle of the item whose state is to be retrieved.
	//			nStateMask - Mask indicating which states are to be retrieved.  For more information
	//			on possible values for 'nStateMask', see the discussion of the 'state'
	//			and 'stateMask' members of the TVITEM structure in the Platform SDK.
	// Returns: The state of the item specified by 'hItem'.
	// Summary:	This member function is a replacement for the base class function of
	//			the same name, to handle TVIS_FOCUSED in a multi-select tree control.
	UINT GetItemState(HTREEITEM hItem,UINT nStateMask) const;

	// Input:	hItem - Handle of a tree item.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function is a replacement for the base class function of
	//			the same name, to handle TVIS_FOCUSED in a multi-select tree control.
	//			Call this function to select the given tree view item.  If 'hItem' is
	//			NULL, then this function selects no item.  
	BOOL SelectItem(HTREEITEM hItem);

	// Returns: The handle of the item that has focus, otherwise returns NULL.
	// Summary:	This member function returns the handle to the tree item that currently
	//			has focus. 
	HTREEITEM GetFocusedItem() const;

	// Input:	hItem - Handle of a tree item.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will set the focus for the tree item specified
	//			by 'hItem'.  
	BOOL FocusItem(HTREEITEM hItem);

	// Input:	bSelect - TRUE to select all items, or FALSE to clear the selection.
	// Summary:	Call this member function to clear, or select, all of the visible items
	//			in the tree control.  This will not effect the focus of the tree items.
	void SelectAll(BOOL bSelect = TRUE);

	// Input:	hItemFrom - The item to start selecting from.
	//			hItemTo - The item to end selection at.
	//			bOnly - TRUE to only select the specified range, or FALSE to keep existing
	//			selections.
    // Summary:	This member function selects items from 'hItemFrom' to 'hItemTo' in
	//			a multi-selection tree control.  It does not select a child item if
	//			the parent is collapsed.  It will remove selection from all other items
	//			if 'bOnly' is set to TRUE.
	void SelectItems(HTREEITEM hItemFrom,HTREEITEM hItemTo,BOOL bOnly = TRUE);

	// Input:	hItem - Handle of a tree item.
	// Returns: TRUE if the specified item is selected, otherwise returns FALSE.
	// Summary:	This member function checks to see if the specified item is selected.
	BOOL IsSelected(HTREEITEM hItem) const;

	// Input:	hParent - Handle of the tree item to begin selection from.
	//			bSelect - TRUE to only select the child items, or FALSE to keep existing selections.
	//			bRecurse - TRUE to recurse all siblings, or FALSE to only select children of the
	//			parent item.
	// Returns: TRUE if focus was on a child item, otherwise returns FALSE.
	// Summary:	This member function will cause all of the children of the specified
	//			tree item to be selected or deselected. 
	BOOL SelectChildren(HTREEITEM hParent,BOOL bSelect = TRUE,BOOL bRecurse = TRUE);

	// Input:	list - Reference to a CTypedPtrList<CPtrList, HTREEITEM> object.
	// Summary:	This member function will retrieve a reference to the typed pointer
	//			array that contains the items selected in the tree control.
	void GetSelectedList(CTypedPtrList<CPtrList, HTREEITEM>& list) const;

	// Returns: TRUE if the tree control is a multi-select tree, otherwise returns FALSE.
	// Summary:	This member function checks to see if the tree control is a multi-select
	//			tree. 
	BOOL IsMultiSelect() const;

	// Input:	bLabel - TRUE to select items only when banding rect passes over the text label,
	//			or FALSE to select items when banding rect passes over any part of
	//			the tree item.
	// Returns: The previous banding state.
	// Summary:	This member function sets the banding mode for a multi-selection tree
	//			control.  If 'bLabel' is TRUE, then items are selected only when the
	//			banding rect passes over the tree item label.  If 'bLabel' is FALSE,
	//			then passing over any part of the tree item will cause selection to
	//			be made when the banding rect passes over it.  
	BOOL SetBandingHit(BOOL bLabel);

	// Returns: true if the tree control sent the WM_NOTIFY message, otherwise, returns false.
	// Summary:	Call this member function to determine if the WM_NOTIFY message that was
	//			sent was sent by the tree control or the framework.  
	bool IsTreeNotify();

protected:

	// Ignore:
	//{{AFX_VIRTUAL(CXTTreeCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

	virtual LRESULT SendNotify(LPNMHDR pNMHDR);
	virtual void SelectAllIgnore(BOOL bSelect, HTREEITEM hIgnore);
	virtual BOOL OnButtonDown(BOOL bLeft, UINT nFlags, CPoint point);
	virtual void DoPreSelection(HTREEITEM hItem, BOOL bLeft, UINT nFlags);
	virtual void DoAction(HTREEITEM hItem, BOOL bLeft, UINT nFlags, CPoint point);
	virtual void DoBanding(UINT nFlags, CPoint point);
	virtual void UpdateSelectionForRect(LPCRECT pRect, UINT nFlags, CTypedPtrList<CPtrList, HTREEITEM>& list);
    virtual BOOL IsFindValid(HTREEITEM hti);

protected:

	// Ignore:
	//{{AFX_MSG(CXTTreeCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE HTREEITEM CXTTreeCtrl::GetNextItem( HTREEITEM hItem, UINT nCode ) {
    ASSERT(::IsWindow(m_hWnd)); return CTreeCtrl::GetNextItem( hItem, nCode );
}
AFX_INLINE HTREEITEM CXTTreeCtrl::GetFocusedItem() const {
	ASSERT(m_bMultiSelect); return CTreeCtrl::GetSelectedItem();
}
AFX_INLINE BOOL CXTTreeCtrl::IsSelected(HTREEITEM hItem) const {
	return !!(TVIS_SELECTED & CTreeCtrl::GetItemState(hItem, TVIS_SELECTED));
}
AFX_INLINE BOOL CXTTreeCtrl::IsMultiSelect() const {
	return m_bMultiSelect;
};
AFX_INLINE BOOL CXTTreeCtrl::SetBandingHit(BOOL bLabel) {
	BOOL bReturn = m_bBandLabel; m_bBandLabel = bLabel; return bReturn;
};
AFX_INLINE bool CXTTreeCtrl::IsTreeNotify() {
	return m_bTreeNotify;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XTTREECTRL_H__E69F83F6_7DC0_4ED8_801D_F24E359A13A9__INCLUDED_)