// XTOutBarCtrl.h interface for the CXTOutBarCtrl class.
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

#if !defined(__XTOUTBARCTRL_H__)
#define __XTOUTBARCTRL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// forwards

class CXTBarItem;
class CXTBarFolder;
class CXTEditItem;

//////////////////////////////////////////////////////////////////////
// Summary: CXTOutBarCtrl is a CWnd derived class.  It is used to create a shortcut
//			bar window similar to the shortcut bar seen in Outlook&trade;.
class _XT_EXT_CLASS CXTOutBarCtrl : public CWnd
{
	DECLARE_DYNCREATE(CXTOutBarCtrl)

public:

    // Summary: Constructs a CXTOutBarCtrl object.
	CXTOutBarCtrl();

	// Summary: Destroys a CXTOutBarCtrl object, handles cleanup and de-allocation.
    virtual ~CXTOutBarCtrl();

protected:

	// used internally

	int m_nFolderHeight; 
	int m_nSelFolder; 
	int m_nFolderHilighted; 
	int m_nItemHilighted; 
	int m_nLastFolderSelected; 
	int m_nLastItemSelected; 
	int m_nFirstItem; 
	int m_nIconSpacingLarge; 
	int m_nIconSpacingSmall; 
	int m_nHitInternal1; 
	int m_nHitInternal2; 
	int m_nLastDragItemDraw; 
	int m_nLastDragItemDrawType; 
	int m_nSelAnimCount; 
	int m_nSelAnimTiming; 
	int m_nAnimationTickCount; 
	CPen m_penBlack; 
	BOOL m_bUpArrow; 
	BOOL m_bDownArrow; 
	BOOL m_bUpPressed; 
	BOOL m_bDownPressed; 
	BOOL m_bIconPressed; 
	BOOL m_bLooping; 
	BOOL m_bPressedHighlight; 
	BOOL m_bUserClrBack; 
	BOOL m_bUserClrText; 
	CFont m_font; 
	DWORD m_dwFlags; 
	CRect m_rcUpArrow; 
	CRect m_rcDownArrow; 
	CSize m_sizeOffset; 
	CSize m_sizeMargin; 
	COLORREF m_clrBack; 
	COLORREF m_clrText; 
	CPtrArray m_arFolder; 
	CImageList* m_pLargeImageList; 
	CImageList* m_pSmallImageList; 

    // used internally

	typedef enum { F_NORMAL, F_SELECT, F_HILIGHT } FOLDER_HILIGHT; 

public:

	// Input:	iTime - Specifies the time, in milliseconds, that the selected item will animate.
	// Summary:	Call this member function to set an animation effect for the currently
	//			selected item.  Not to be used with OBS_XT_SELHIGHLIGHT flag.
	virtual void SetAnimSelHighlight(const int iTime);

	// Input:	iFolder - The index of the folder to retrieve item data for.  If -1, the currently
	//			selected folder item data is returned.
	// Returns: A DWORD value.
	// Summary:	Call this member function to retrieve the item data that was set for
	//			the specified folder. 
	virtual DWORD GetFolderData(int iFolder = -1);

	// Input:	iFolder - Index of the folder to retrieve the CWnd object for, if -1 the currently
	//			selected folder CWnd object is used.
	// Returns:	If 'iFolder' is -1, the child of the currently selected folder is returned.
	//			If no object has been set for the folder, the return value is NULL.
	// Summary:	Call this member function to retrieve the CWnd object that has been set
	//			for the folder specified by 'iFolder'.  
	virtual CWnd* GetFolderChild(int iFolder = -1);

	// Input:	lpszFolderName - Name of the folder to add.
	//			pWndChild - Points to a valid CWnd object.  The object must be created before
	//			inserting.
	//			dwData - Item data (lParam) for the folder.
	// Returns: The integer value that represents the index of the added folder.
	// Summary:	Call this member function to add a folder with a CWnd child nested
	//			inside of it.  You can insert a folder with any CWnd object, such as
	//			a tree control (see the OutlookBar sample). 
	virtual int AddFolderBar(LPCTSTR lpszFolderName,CWnd* pWndChild,const DWORD dwData = 0);

	// Input:	iIndex - Index of the item to retrieve the text for.
	// Returns: A CString object containing the retrieved text.
	// Summary:	Call this member function to get the text of the specified item for
	//			the currently selected folder. 
	virtual CString GetItemText(const int iIndex);

	// Input:	lValue - Specifies the time, in milliseconds, between animation.  A value of
	//			-1 will disable animation playback.
	// Summary:	Call this member function to set the tick count, in milliseconds, between
	//			each animation frame in folder scrolling.  If you set a value of -1,
	//			or minor, no animation will be played.  Animation requires the OBS_XT_ANIMATION
	//			flag be set.
	virtual void SetAnimationTickCount(const long lValue);

	// Returns: An integer value representing the current tick count.
	// Summary:	Call this member function to get the current animation tick count.
	virtual int GetAnimationTickCount();

	// Input:	iIndex - Index of the item in the currently selected folder.
	//			iImage - Index of the image, in the image list, to use for the specified item.
	// Summary:	Call this member function to set the image index, in the image list,
	//			for the 'iIndex' item of the currently selected folder.
	virtual void SetItemImage(const int iIndex,const int iImage);

	// Input:	iIndex - Index of the item to set item data for.
	//			dwData - Item data (lParam) to set.
	// Summary:	Call this member function to set the item data (lParam) for the specified
	//			item in the currently selected folder.
	virtual void SetItemData(const int iIndex,const DWORD dwData);

	// Input:	iIndex - Index of the item to retrieve the image index for.
	// Returns: An integer value that represents the index of the desired image.
	// Summary:	Call this member function to retrieve the index of the image associated
	//			with the specified item in the currently selected folder. 
	virtual int  GetItemImage(const int iIndex) const;

	// Input:	iIndex - Index of the item to retrieve item data for.
	// Returns: A DWORD value.
	// Summary:	Call this member function to get the item data (lParam) for the specified
	//			item in the currently selected folder. 
	virtual DWORD GetItemData(const int iIndex) const;

	// Input:	iFolder - Index of the folder to insert the item into.
	//			bNofify - true to send an XTWM_OUTBAR_NOTIFY message.
	// Summary:	Call this member function to remove all items from the folder specified
	//			by 'iFolder'.
	virtual void RemoveAllItems(int iFolder,bool bNofify=false);

	// Input:	iIndex - Index of the item to remove.
	// Summary:	Call this member function to remove the specified item from the currently
	//			selected folder.
	virtual void RemoveItem(const int iIndex);

	// Input:	iIndex - Index of the item to set the text for.
	//			lpszItemName - Points to a NULL terminated string.
	// Summary:	Call this member function to set the text for the specified item in the
	//			currently selected folder.
	virtual void SetItemText(const int iIndex,LPCTSTR lpszItemName);

	// Input:	iIndex - Index of the item to begin editing for.
	// Summary:	Call this member function to begin local editing of the specified item
	//			in the currently selected folder.
	virtual void StartItemEdit(const int iIndex);

	// Input:	iIndex - Index of the folder to set the text label for.
	//			lpszFolderName - Points to a NULL terminated string.
	// Summary:	Call this member function to set the text label for the specified folder.
	virtual void SetFolderText(const int iIndex,LPCTSTR lpszFolderName);

	// Input:	iIndex - Index of the folder to begin editing.
	// Summary:	Call this member function to begin editing of the specified folder
	//			item's label.
	virtual void StartGroupEdit(const int iIndex);

	// Input:	iIndex - Index of the folder to retrieve the image list for.
	//			bSmall - TRUE to return the small image list.  FALSE to return the large image
	//			list.
	// Returns: A CImageList pointer representing the image list for the folder specified 
	//			by 'iIndex'.
	// Summary:	Call this member function to get a pointer to the image list for the
	//			specified folder. 
	virtual CImageList* GetFolderImageList(const int iIndex,const BOOL bSmall) const;

	// Input:	dwImageList - If OBS_XT_SMALLICON, the small image list is returned, if OBS_XT_LARGEICON,
	//			the large image list is returned.
	// Returns: A CImageList pointer representing the global image list for the OutlookBar 
	//			control.
	// Summary:	Call this member function to return the global image list for the OutlookBar
	//			control. 
	virtual CImageList* GetImageList(DWORD dwImageList);

	// Input:	iFolder - Index of the folder to set the image list for.
	//			pImageList - Points to the new image list.
	//			dwImageList - If OBS_XT_SMALLICON, the small image list is set, if OBS_XT_LARGEICON,
	//			the large image list is set.
	// Returns: A pointer to the previously set image list, or NULL if no previous
	//			image list exists.
	// Summary:	Call this member function to set the image list for the specified folder.
	virtual CImageList* SetFolderImageList(const int iFolder,CImageList* pImageList,DWORD dwImageList);

	// Input:	pImageList - Points to the new image list.
	//			dwImageList - If OBS_XT_SMALLICON, the small image list is set, if OBS_XT_LARGEICON,
	//			the large image list is set.
	// Returns: A pointer to the previously set image list, or NULL if no previous image 
	//			list exists.
	// Summary:	This member function will set the main image list.  You can link different
	//			imagelists to the folders using the SetFolderImageList function.  If a
	//			folder has been linked to an image list with the SetFolderImageList function,
	//			it will own the linked image list.  Otherwise, it will use the image list
	//			set with this function.  
	virtual CImageList* SetImageList(CImageList* pImageList,DWORD dwImageList);

	// Input:	iIndex - Index of the folder to remove.
	// Summary:	Call this member function to remove the specified folder and its items.
	virtual void RemoveFolder(const int iIndex);

	// Returns: An integer value representing the index of the currently selected folder.
	// Summary:	This member function will get the index of the currently selected folder.
	virtual int GetSelFolder() const;

	// Returns: An integer value representing the number of folders in the Outlook bar.
	// Summary:	This member function will get the total number of folders found in
	//			the Outlook bar. 
	virtual int GetFolderCount() const;

	// Input:	iIndex - Index of the new selected folder.
	// Summary:	This member function will set the selected folder for the Outlook bar.
	virtual void SetSelFolder(const int iIndex);

	// Returns: An integer value representing the number of items in the current folder.
	// Summary:	This member function gets the number of items found in the currently
	//			selected folder. 
	virtual int GetItemCount() const;

	// Input:	iFolder - Index of the folder to insert the item into.
	//			iIndex - Index or position of the item to insert into the folder.
	//			lpszItemName - A NULL terminated string that represents the item label.  This value
	//			<b>cannot</b> be set to NULL.
	//			iImage - Index into the folder's image list.
	//			dwData - User defined item data that you can assign to the item. Use GetItemData
	//			and SetItemData to access and change this data.
	// Returns: The index of the newly inserted item.
	// Summary:	Call this member function to insert an item into the specified folder.
	virtual int InsertItem(const int iFolder,const int iIndex,LPCTSTR lpszItemName,const int iImage = -1,const DWORD dwData = 0);

	// Input:	lpszFolderName - A NULL terminated string that represents the folder's label.
	//			dwData - User defined item data for the folder.
	// Returns: The index of the newly inserted folder.
	// Summary:	Call this member function to add a folder to the Outlook bar control.
	virtual int AddFolder(LPCTSTR lpszFolderName,const DWORD dwData);

	// BULLETED LIST:

	// Input:	dwRemove - Specifies OBS_XT_ styles to be removed during style modification.
	//			dwAdd - Specifies OBS_XT_ styles to be added during style modification.
	//			bRedraw - true to redraw the Outlook bar.
    // Summary:	Call this member function to modify an Outlook bar style.  Styles
	//			to be added or removed can be combined by using the bitwise OR (|) 
    //			operator.
	//
    //			The desired styles for the Outlook bar can be one or more of the 
    //			following:
    //			[ul]
    //			[li]<b>OBS_XT_EDITGROUPS</b> Enables folder local editing (renaming).[/li] 
    //			[li]<b>OBS_XT_EDITITEMS</b> Enables item local editing (renaming).[/li]
    //			[li]<b>OBS_XT_REMOVEGROUPS</b> Enables the "Remove" command for
	//			folders in context menu.[/li]
    //			[li]<b>OBS_XT_REMOVEITEMS</b> Enables the "Remove" command for
	//			items in context menu.[/li]
    //			[li]<b>OBS_XT_ADDGROUPS</b> Enables folder insertion.[/li]
    //			[li]<b>OBS_XT_DRAGITEMS</b> Enables item dragging to rearrange
	//			position.[/li]
    //			[li]<b>OBS_XT_ANIMATION</b> Enables animation while changing folder
	//			selection.[/li] 
    //			[li]<b>OBS_XT_SELHIGHLIGHT</b> Enables dimmed highlight of last
	//			pressed item.[/li]
    //			[li]<b>OBS_XT_DEFAULT</b> Same as OBS_XT_DRAGITEMS | OBS_XT_EDITGROUPS
	//			 | OBS_XT_EDITITEMS | OBS_XT_REMOVEGROUPS | OBS_XT_REMOVEITEMS
	//			 | OBS_XT_ADDGROUPS.[/li]
    //			[/ul]
	virtual void ModifyFlag(const DWORD& dwRemove,const DWORD& dwAdd,const bool bRedraw = false);

	// Returns: A DWORD value representing the current style of the Outlook bar.
	// Summary:	Call this member function to get the current style set for the Outlook bar.
	virtual DWORD GetFlag() const;

	// Input:	bSet - TRUE to display small icons, or FALSE to display large icons.
	//			iFolder - Index of the folder to set the icon size for.  If -1, all folder icons
	//			are set.
	// Summary:	Call this member function to set the size of the icons displayed in the 
	//			Outlook bar control for the specified folder.
	virtual void SetSmallIconView(const BOOL bSet,const int iFolder=-1);

	// Input:	iFolder - Index of the folder to check.  If -1, the currently selected folder
	//			is checked.
	// Returns: TRUE if small icons are displayed, and FALSE if large icons are displayed.
	// Summary:	Call this member function to return the current state of the icon display
	//			for the Outlook bar control.  
	virtual BOOL IsSmallIconView(const int iFolder=-1) const;

	// Input:	dwStyle - Style for the Outlook bar. It usually includes the 
	//			WS_CHILD|WS_VISIBLE	flags.
	//			rect - Size of the Outlook bar.
	//			pParentWnd - Parent of the control.
	//			nID - Identifier of the Outlook bar control.
	//			dwFlag - Specifies the style flags for the control.  See ModifyFlag for a list
	//			of available styles.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to create the Outlook bar control.  
	virtual BOOL Create(DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID,const DWORD dwFlag = OBS_XT_DEFAULT);

	// Input:	dwExStyle - Extended style for the Outlook bar such as WS_EX_STATICEDGE. It can
	//			be NULL.
	//			dwStyle - Style for the Outlook bar. It usually includes the WS_CHILD|WS_VISIBLE
	//			flags.
	//			rect - Size of the Outlook bar.
	//			pParentWnd - Parent of the control.
	//			nID - Identifier of the Outlook bar control.
	//			dwFlag - Specifies the style flags for the control.  See ModifyFlag for a list
	//			of available styles.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to create the Outlook bar control.  
	virtual BOOL CreateEx(DWORD dwExStyle,DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID,const DWORD dwFlag = OBS_XT_DEFAULT);

	// Input:	pFont - Points to the font to be used by the Outlook bar.
	// Summary:	Call this member function to set the font used by the Outlook bar control.
	virtual void SetFontX(CFont* pFont);

	// Returns: A pointer to a CFont object representing the font used by the Outlook bar 
	//			control.
	// Summary:	Call this member function to retrieve the font used by the Outlook
	//			bar control. 
	virtual CFont* GetFontX();

	// Input:	iFolder - Index of the folder to retrieve.
	// Returns: A pointer to a CXTBarFolder object.
	// Summary:	Call this member function to return a pointer to the CXTBarFolder data
	//			that is associated with the specified folder.  
	virtual CXTBarFolder* GetBarFolder(const int iFolder);

	// Input:	iFolder - Index of the folder to retrieve.
	//			iIndex - Index of the item to retrieve.
	// Returns: A pointer to a CXTBarItem object.
	// Summary:	Call this member function to return a pointer to the CXTBarItem data
	//			that is associated with the specified folder and item.  
	virtual CXTBarItem* GetBarFolderItem(const int iFolder,const int iIndex);

	// Input:	clrBack - An RGB value that represents the background color.
	// Summary:	Call this member function to set the background color for the Outlook
	//			bar control.
	virtual void SetBackColor(COLORREF clrBack);

	// Input:	clrText - An RGB value that represents the text item color.
	// Summary:	Call this member function to set the text color for items in the Outlook
	//			bar control.
	virtual void SetTextColor(COLORREF clrText);

	// Input:	iFolder - Index of the folder where the item is located.
	//			iIndex - Index of the item.
	//			rect - Address of a CRect object that will receive the label size.
	// Summary:	Call this member function to retrieve the size of the label for the
	//			specified item.
	virtual void GetLabelRect(const int iFolder,const int iIndex,CRect& rect);

	// Input:	iFolder - Index of the folder where the item is located.
	//			iIndex - Index of the item.
	//			rect - Address of a CRect object that will receive the icon size.
	// Summary:	Call this member function to retrieve the size of the icon for the
	//			specified item.
	virtual void GetIconRect(const int iFolder,const int iIndex,CRect& rect);

	// Input:	rect - Address of a CRect object that will receive the size.
	// Summary:	Call this member function to retrieve the size of the client area for
	//			the Outlook bar.  This is the inside area that contains the folders.
	virtual void GetInsideRect(CRect& rect) const;

	// Input:	iFolder - Index of the folder where the item is located.
	//			iIndex - Index of the item.
	//			rect - Address of a CRect object that will receive the item size.
	// Summary:	Call this member function to retrieve the size of the specified item.
	//			The size includes the area occupied by the item's label and icon.
	virtual void GetItemRect(const int iFolder,const int iIndex,CRect& rect);

	// Input:	iIndex - Index of the folder item.
	//			rect - Address of a CRect object that will receive the folder size.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to retrieve the size of the specified folder.
	virtual BOOL GetFolderRect(const int iIndex,CRect& rect) const;

	// Returns: An integer value representing the index of the selected item.
	// Summary:	This member function will return the index of the currently selected
	//			item for the currently selected folder. 
    inline int GetCurSel();

	// Input:	iItem - Index of the item to select.
	//			bPressed - true if the item is to be pressed when selected.
	// Summary:	This member function will set the currently selected item for the currently
	//			selected folder.
    inline void SetCurSel(int iItem,const BOOL bPressed=false);

	// Input:	iFolder - Index of the folder that owns the item.
	//			iItem - Index of the item to enable or disable.
	//			bEnable - true to enable item, false to disable.
	// Summary:	Call this member function to enable or disable a folder item.
	void EnableItem(int iFolder,int iItem,bool bEnable);
	
protected:

	virtual void DrawScrollButton(CDC* pDC, CRect rect, UINT uType, UINT uState);
	virtual void DrawItem(CDC* pDC, const int iFolder, CRect rc, const int iIndex, const BOOL bOnlyImage = false);
	virtual void DrawDragArrow(CDC* pDC, const int iFrom, const int iTo);
	virtual void DrawAnimItem(const int iOffsetX, const int iOffsetY, const int iIndex);
	virtual void DrawFolder(CDC* pDC, const int iIndex, CRect rect, const FOLDER_HILIGHT eHilight);
    virtual void DrawIcon(CDC* pDC, int iIcon, int iFolder, bool bHilight);
	virtual void PaintItems(CDC* pDC, const int iFolder, CRect rc);
	virtual void GetVisibleRange(const int iFolder, int& iFirst, int& iLast);
	virtual int GetDragItemRect(const int iIndex, CRect& rect);
	virtual CSize GetItemSize(const int iFolder, const int iIndex, const int iType);
	virtual void AnimateFolderScroll(const int iFrom, const int iTo);
	virtual void HighlightItem(const int iIndex, const BOOL bPressed = false);
	virtual void HighlightFolder(const int iIndex);
	virtual int HitTestEx(const CPoint& point, int& iIndex);
	virtual BOOL IsValidItem(const int iIndex) const;
	virtual void EndLabelEdit(CXTEditItem* pEdit, bool bIsFolder);
	virtual void OnLabelChanged (const XT_OUTBAR_INFO* pObi);
	virtual void DrawItemIcon( CDC* pDC, CPoint pt, CXTIconHandle hIcon, BOOL bEnabled );
	virtual void DrawItemText( CDC* pDC, CRect rc, CString strText, UINT nFormat, BOOL bEnabled );

    // Ignore:
	//{{AFX_VIRTUAL(CXTOutBarCtrl)
	//}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTOutBarCtrl)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLargeIcon();
	afx_msg void OnUpdateLargeIcon(CCmdUI* pCmdUI);
	afx_msg void OnSmallIcon();
	afx_msg void OnUpdateSmallIcon(CCmdUI* pCmdUI);
	afx_msg void OnRemoveItem();
	afx_msg void OnUpdateRemoveItem(CCmdUI* pCmdUI);
	afx_msg void OnRenameItem();
	afx_msg void OnUpdateRenameItem(CCmdUI* pCmdUI);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

	friend class CXTEditItem;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE int CXTOutBarCtrl::GetCurSel() {
	return m_nItemHilighted;
}
AFX_INLINE void CXTOutBarCtrl::SetCurSel(int iItem, const BOOL bPressed/*=false*/) {
    HighlightItem(iItem, bPressed);
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTBarItem is a stand alone helper class.  It is used by the CXTOutBarCtrl
//			to maintain information about each folder item specified in the Outlook
//			bar control.
class _XT_EXT_CLASS CXTBarItem
{
public:

	// Input:	lpszName - A NULL terminated string that represents the item name.
	//			nImageIndex - An index into the folders image list.
	//			dwData - User item data (lParam).
    // Summary:	Constructs a CXTBarItem object.
	CXTBarItem(LPCTSTR lpszName,const int nImageIndex,DWORD dwData);

	// Summary: Destroys a CXTBarItem object, handles cleanup and de-allocation.
    virtual ~CXTBarItem();

	// Returns: An integer value that represents the zero-based index of the folder item.
	// Summary:	Call this member function to retrieve the zero-based index of the folder
	//			item. 
	int GetIndex();

	// Input:	iIndex - New index of the folder item.
	// Summary:	Call this member function to set the zero-based index of the folder item.
	void SetIndex(int iIndex);

	// Returns: A DWORD value that represents the user data.
	// Summary:	Call this member function to return the user specified item data (lParam)
	//			for the folder item.  
	DWORD GetData();

	// Input:	dwData - Specifies the user data (lparam) value to be associated with the 
	//			folder item.
	// Summary:	Call this member function to set a user data (lParam) value for the
	//			folder item.
	void SetData(DWORD dwData);

	// Returns: A CString object that contains the folder item label.
	// Summary:	Call this member function to retrieve the label of the folder item. 
	CString GetName();

	// Input:	strName - A NULL terminated string that represents the item label.
	// Summary:	Call this member function to set the label of the folder item.
	void SetName(CString strName);

	// Input:	bSelected - true to set the item state to selected.
	// Summary:	Call this member function to set the items selected state.
	void SelectItem(bool bSelected);

	// Returns: true if the item is selected, otherwise returns false.
	// Summary:	Call this member function to see if the item is selected. 
	bool IsSelected();

	// Input:	bEnable - true to enable, false to disable.
	// Summary:	Call this member function to toggle the enabled state for the outlook
	//			bar item.
	void EnableItem(bool bEnable);

	// Returns: true if the item is enabled, otherwise returns false.
	// Summary:	Call this member function to determine if the item is enabled.  
	bool IsEnabled();

protected:

	int		m_nIndex;		
	bool	m_bEnabled;		// true if the icon is enabled.
	bool	m_bSelected;	
	DWORD	m_dwData;		
	CString m_strName;		

	friend class CXTOutBarCtrl;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTBarItem::SelectItem(bool bSelected) {
	m_bSelected = bSelected;
}
AFX_INLINE bool CXTBarItem::IsSelected() {
	return m_bSelected;
}
AFX_INLINE void CXTBarItem::EnableItem(bool bEnable) {
	m_bEnabled = bEnable;
}
AFX_INLINE bool CXTBarItem::IsEnabled() {
	return m_bEnabled;
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTBarFolder is a stand alone helper class.  It is used by the CXTOutBarCtrl
//			to maintain information about each folder specified in the Outlook bar
//			control.
class _XT_EXT_CLASS CXTBarFolder
{
public:

	// Input:	lpszName - A NULL terminated string that represents the item name.
	//			dwData - User item data (lParam).
    // Summary:	Constructs a CXTBarFolder object.
	CXTBarFolder(LPCTSTR lpszName,DWORD dwData);

	// Summary: Destroys a CXTBarFolder object, handles cleanup and de-allocation.
    virtual ~CXTBarFolder();

	// Returns: The number of folders in the Outlook bar.
	// Summary:	Call this member function to get the number of folders found in the
	//			Outlook bar. 
	int GetItemCount();

	// Input:	iIndex - Index of the item.
	//			lpszName - A NULL terminated string that represents the label of the item.
	//			nImage - Index into the folder's image list.
	//			dwData - User item data (lParam).
	// Returns: An integer value.
	// Summary:	Call this member function to insert an item into the Outlook bar folder.
	int	InsertItem(int iIndex,LPCTSTR lpszName,const int nImage,const DWORD dwData);

	// Input:	iIndex - Index of the item to retrieve.
	// Returns:	A pointer to a CXTBarItem object.
	// Summary:	Call this member function to retrieve the specified item.  
	CXTBarItem* GetItemAt(int iIndex);

	// Input:	iIndex - Zero-based index of where to insert the new item.
	//			pBarItem - Points to a valid CXTBarItem object.
	// Summary:	Call this member function to insert a folder item into the location
	//			specified by 'iIndex'.
	void InsertItemAt(int iIndex,CXTBarItem* pBarItem);

	// Input:	iIndex - Index of the item to remove.
	// Returns: A pointer to the removed item.
	// Summary:	Call this member function to remove the specified item from the folder.
	CXTBarItem* RemoveItemAt(int iIndex);

	// Returns: A CString object.
	// Summary:	Call this member function to return the label of the folder item. 
	CString GetName();

	// Input:	strName - A NULL terminated string that represents the folder's new label.
	// Summary:	Call this member function to set the label for the folder item.
	void SetName(CString strName);

	// Returns: A DWORD value that represents the item data.
	// Summary:	Call this member function to return the user item data (lParam) for
	//			the folder. 
	DWORD GetData();

	// Input:	dwData - User item data (lParam).
	// Summary:	Call this member function to set the user item data (lParam) for the
	//			folder.
	void SetData(DWORD dwData);

	// Returns: A pointer to CImageList object if successful, otherwise returns NULL.
	// Summary:	Call this member function to return a pointer to the large image list
	//			for the folder.  
	CImageList* GetLargeImageList();

	// Input:	pLargeList - Points to a CImageList object.
	// Summary:	Call this member function to set the large image list for the folder.
	void SetLargeImageList(CImageList* pLargeList);

	// Returns: A CImageList object if successful, otherwise returns NULL.
	// Summary:	Call this member function to return a pointer to the small image list
	//			for the folder.  
	CImageList* GetSmallImageList();

	// Input:	pSmallList - Points to a CImageList object.
	// Summary:	Call this member function to set the small image list for the folder.
	void SetSmallImageList(CImageList* pSmallList);

	// Returns: A CWnd pointer to the child associated with the folder, or NULL if no 
	//			objects were found.
	// Summary:	Call this member function to retrieve a CWnd pointer to the child object
	//			that is associated with this folder item.  
	CWnd* GetChild();

	// Input:	pChild - Points to a valid CWnd object.
	// Summary:	Call this member function to set the CWnd child to be associated with this
	//			folder item.
	void SetChild(CWnd* pChild);

	// Input:	iItem - Index of the item to select
	// Summary:	Call this member function to set the currently selected item for the folder.
	void SetSelItem(int iItem);

	// Returns: A pointer to a CXTBarItem object if successful, otherwise returns NULL.
	// Summary:	Call this member function to return a pointer to the currently selected
	//			item. 
	CXTBarItem* GetSelItem();

	// Returns: The index of the currently selected item if successful, otherwise returns -1.
	// Summary:	Call this member function to return the index for the currently selected
	//			item. 
	int GetSelIndex();

protected:

	BOOL							m_bSmallIcons;	
	CWnd*							m_pChild;
	DWORD							m_dwData;
	CString							m_strName;
	CImageList*						m_pLargeList;
	CImageList*						m_pSmallList;
	CList<CXTBarItem*, CXTBarItem*> m_barItems;

	friend class CXTOutBarCtrl;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTOutBarCtrl::SetFontX(CFont* pFont) {
	ASSERT_VALID(pFont); LOGFONT lf; pFont->GetLogFont(&lf); m_font.DeleteObject(); m_font.CreateFontIndirect(&lf);
}
AFX_INLINE CFont* CXTOutBarCtrl::GetFontX() {
	return &m_font;
}
AFX_INLINE void CXTOutBarCtrl::SetAnimationTickCount(const long lValue) {
	m_nAnimationTickCount = lValue;
}
AFX_INLINE int CXTOutBarCtrl::GetAnimationTickCount() {
	return m_nAnimationTickCount;
}
AFX_INLINE CXTBarFolder* CXTOutBarCtrl::GetBarFolder(const int iFolder) {
	return (CXTBarFolder*)m_arFolder.GetAt(iFolder);
}
AFX_INLINE CXTBarItem* CXTOutBarCtrl::GetBarFolderItem(const int iFolder, const int iIndex) {
	return GetBarFolder(iFolder)->GetItemAt(iIndex);
}
AFX_INLINE void CXTOutBarCtrl::SetBackColor(COLORREF clrBack) {
    m_bUserClrBack = TRUE;
	m_clrBack = clrBack;
}
AFX_INLINE void CXTOutBarCtrl::SetTextColor(COLORREF clrText) {
    m_bUserClrText = TRUE;
	m_clrText = clrText;
}

//////////////////////////////////////////////////////////////////////

AFX_INLINE int CXTBarItem::GetIndex() {
	return m_nIndex;
}
AFX_INLINE void CXTBarItem::SetIndex(int iIndex) {
	m_nIndex = iIndex;
}
AFX_INLINE DWORD CXTBarItem::GetData() {
	return m_dwData;
}
AFX_INLINE void CXTBarItem::SetData(DWORD dwData) {
	m_dwData = dwData;
}
AFX_INLINE CString CXTBarItem::GetName() {
	return m_strName;
}
AFX_INLINE void CXTBarItem::SetName(CString strName) {
	m_strName = strName;
}

//////////////////////////////////////////////////////////////////////

AFX_INLINE int CXTBarFolder::GetItemCount() {
	return (int)m_barItems.GetCount();
}
AFX_INLINE CString CXTBarFolder::GetName() {
	return m_strName;
}
AFX_INLINE void CXTBarFolder::SetName(CString strName) {
	m_strName = strName;
}
AFX_INLINE DWORD CXTBarFolder::GetData() {
	return m_dwData;
}
AFX_INLINE void CXTBarFolder::SetData(DWORD dwData) {
	m_dwData = dwData;
}
AFX_INLINE CImageList* CXTBarFolder::GetLargeImageList() {
	return m_pLargeList;
}
AFX_INLINE void CXTBarFolder::SetLargeImageList(CImageList* pLargeList) {
	m_pLargeList = pLargeList;
}
AFX_INLINE CImageList* CXTBarFolder::GetSmallImageList() {
	return m_pSmallList;
}
AFX_INLINE void CXTBarFolder::SetSmallImageList(CImageList* pSmallList) {
	m_pSmallList = pSmallList;
}
AFX_INLINE CWnd* CXTBarFolder::GetChild() {
	return m_pChild;
}
AFX_INLINE void CXTBarFolder::SetChild(CWnd* pChild) {
	m_pChild = pChild;
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Summary: CXTEditItem is a helper class derived from CXTEdit.  It is used by 
//			CXTOutBarCtrl, an Outlook like control, to create an in-place edit box
//			that is used to modify a folder or an item within the control.
class _XT_EXT_CLASS CXTEditItem : public CXTEdit
{
	DECLARE_DYNAMIC(CXTEditItem)

public:

    // Summary: Constructs a CXTEditItem object.
	CXTEditItem();

	// Summary: Destroys a CXTEditItem object, handles cleanup and de-allocation.
    virtual ~CXTEditItem();

protected:

	int				m_iIndex;		// Index of the folder or item.
	bool			m_bEscapeKey;	// true if the escape key was pressed.
	bool			m_bSmallIcons;	// true if the folder is using small icons.
	bool			m_bIsFolder;	// true if the edit box is for a folder.
	CRect			m_rcOriginal;	// Original size of the edit box when it was first created.
	CString			m_strText;		// Original string of the edit box when it was first created, and the new text on edit completion.
	CXTOutBarCtrl*	m_pParentWnd;	// Parent Outlook bar control.

public:
	
	// Returns: An integer value that represents the index of the folder or item.
	// Summary:	Call this member function to return the index of the folder or item
	//			currently being edited. 
	int GetIndex() const;

	// Returns: A CString object that represents the text originally
	//			set for the edit control.  If called after the edit is destroyed, it
	//			returns the modified text value.
	// Summary:	Call this member function to return the text associated with this edit
	//			control.  
	CString GetText() const;

	// Input:	lpszText - NULL terminated string to be displayed in the edit control.
	//			dwStyle - Window style for the edit control.
	//			rect - Size of the edit control.
	//			pParentWnd - Owner window.
	//			nID - Control ID.
	//			nIndex - Folder or item index.
	//			bIsFolder - true if the edit is for a folder item.
	//			bSmallIcons - true if the parent folder is displaying small icons.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	The Outlook bar control calls this member to create an in-place edit
	//			control. 
	virtual BOOL Create(LPCTSTR lpszText,DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID,UINT nIndex,bool bIsFolder,bool bSmallIcons);

    // Ignore:
	//{{AFX_VIRTUAL(CXTEditItem)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTEditItem)

	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE int CXTEditItem::GetIndex() const {
	return m_iIndex;
}
AFX_INLINE CString CXTEditItem::GetText() const {
	return m_strText;
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTToolBox is a CXTOutBarCtrl derived class.  It is used to create a
//			toolbox control similar to the toolbox control seen in VisualStudio.NET.
class _XT_EXT_CLASS CXTToolBox : public CXTOutBarCtrl
{
	DECLARE_DYNCREATE(CXTToolBox)

public:
    
    // Summary: Constructs a CXTToolBox object.
	CXTToolBox();
    
	// Summary: Destroys a CXTToolBox object, handles cleanup and de-allocation.
	virtual ~CXTToolBox();

protected:

	int  m_iFirst;		// Index of the first visible item.
	int  m_iLast;		// Index of the last visible item.
	bool m_bAnimating;	// true if the folder selection is changing.

public:

	// Input:	iItem - Index of the item to select.
	//			iFolder - Index of the folder the item belongs to.
	// Summary:	Call this member function to set the selection for the item specified
	//			by 'iItem'.
	void SetSelItem(int iItem,int iFolder);

	// Input:	dwExStyle - Extended style for the toolbox, such as WS_EX_STATICEDGE. It can be
	//			NULL.
	//			dwStyle - Style for the toolbox. It usually includes the WS_CHILD|WS_VISIBLE
	//			flags.
	//			rect - Size of the toolbox.
	//			pParentWnd - Parent of the control.
	//			nID - Identifier of the toolbox control.
	//			dwFlag - Specifies the style flags for the control.  See CXTOutBarCtrl::ModifyFlag
	//			for a list of available styles.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to create the toolbox control.  
	virtual BOOL CreateEx(DWORD dwExStyle,DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID,const DWORD dwFlag = OBS_XT_DEFAULT);

	// Input:	iFolder - Index of the folder where the item is located.
	//			iIndex - Index of the item.
	//			rect - Address of a CRect object that will receive the icon size.
	// Summary:	Call this member function to retrieve the size of the icon for the
	//			specified item.
	virtual void GetIconRect(const int iFolder,const int iIndex,CRect& rect);

	// Input:	iFolder - Index of the folder where the item is located.
	//			iIndex - Index of the item.
	//			rect - Address of a CRect object that will receive the item size.
	// Summary:	Call this member function to retrieve the size of the specified item.
	//			The size includes the area occupied by the item's label and icon.
	virtual void GetItemRect(const int iFolder,const int iIndex,CRect& rect);

	// Input:	iIndex - Index of the new selected folder.
	// Summary:	This member function will set the selected folder for the toolbox.
	virtual void SetSelFolder(const int iIndex);

protected:

	// Ignore:
	//{{AFX_VIRTUAL(CXTToolBox)
	//}}AFX_VIRTUAL

    virtual void DrawIcon(CDC* pDC, int iIcon, int iFolder, bool bHilight);
    virtual void DrawItem(CDC* pDC, const int iFolder, CRect rc, const int iIndex, const BOOL bOnlyImage);
	virtual void DrawFolder(CDC* pDC, const int iIndex, CRect rect, const FOLDER_HILIGHT eHilight);
	virtual void DrawScrollButton(CDC* pDC, CRect rect, UINT uType, UINT uState);

	// Ignore:
	//{{AFX_MSG(CXTToolBox)

	afx_msg void OnPaint();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTOUTBARCTRL_H__)