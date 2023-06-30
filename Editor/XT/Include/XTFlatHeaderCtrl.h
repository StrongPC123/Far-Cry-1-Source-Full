// XTFlatHeaderCtrl.h interface for the CXTFlatHeaderCtrl class.
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

#if !defined(__XTFLATHEADERCTRL_H__)
#define __XTFLATHEADERCTRL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTHeaderCtrl is a CHeaderCtrl derived class.  It is used to create
//			a CXTHeaderCtrl window similar to CHeaderCtrl, but with additional functionality. 
// 
//			A “header control” is a window usually positioned above columns of text
//			or numbers. It contains a title for each column, and it can be divided
//			into parts. The user can drag the dividers that separate the parts to
//			set the width of each column.
//
//			Use a header control, represented by class CXTHeaderCtrl, to display
//			column headers for a columnar list.  For example, a header control would
//			be useful for implementing column controls in a spreadsheet.
//			
//			The header control is usually divided into parts, called "header items,"
//			each bearing a title for the associated column of text or numbers. Depending
//			on the styles you set, you can provide a number of direct ways for users
//			to manipulate the header items.
class _XT_EXT_CLASS CXTHeaderCtrl : public CHeaderCtrl
{
    DECLARE_DYNAMIC(CXTHeaderCtrl)

public:

    // Summary: Constructs a CXTHeaderCtrl object.
	CXTHeaderCtrl();

    // Summary: Destroys a CXTHeaderCtrl object, handles cleanup and de-allocation.
    virtual ~CXTHeaderCtrl();

protected:

	int			m_iMinSize;		// Minimum column size for an auto-size header control.
	bool		m_bRTL;			// Used internally to determine if text is right-to-left or left-to-right (depends on system locale).
	bool		m_bAutoSize;	// true if the header control columns are auto-sizing.
	CUIntArray	m_arFrozenCols;	// List of columns that are not sizable.

public:

	// Input:	bEnable - true to enable an auto-sizing header control.
    // Summary:	Call this member function to enable auto-sizing for the header control.
	//			This will cause the columns in the list control to be sized to fit
	//			in the available space when the list control is resized.
	void EnableAutoSize(bool bEnable=true);

	// Input:	iCol - Index of the column to freeze.
    // Summary:	Call this member function to freeze a column in the header control.
	//			Freezing a column will disable sizing for the column.
	void FreezeColumn(int iCol);

	// Input:	iCol - Index of the column to thaw.
	// Summary:	Call this member function to thaw a column in the header control.
	//			Thawing a column will enable sizing for the column if it was previously
	//			frozen.
	void ThawColumn(int iCol);

	// Input:	iCol - Index of the column to check.
	// Returns: true if the column is frozen, otherwise returns false.
    // Summary:	Call this member function to determine if the specified column is
	//			frozen.  
	bool IsColFrozen(int iCol);

	// Input:	iMinSize - Minimum column size.
    // Summary:	Call this member to set the minimum size for auto-sizing columns.
	//			The minimum size represents the smallest size that all columns can
	//			be sized to.
	void SetMinSize(int iMinSize);

protected:

	// Returns: true if the text alignment is right-to-left, and false if the text 
	//			alignment is left-to-right.
    // Summary:	This member function is used by the header control to determine
	//			the text alignment for the system locale.  
	virtual bool DetermineRTL();

	// Input:	iNewWidth - New width to resize all columns to.
    // Summary:	This member function is used by the header control to auto-size the
	//			columns for the list control.
	virtual void ApplyFieldWidths(int iNewWidth);

	// Input:	iNewWidth - New width to resize all columns to.
	// Summary:	This member function is used by the header control to determine
	//			the new width for auto-sized columns.
	virtual void FitFieldWidths(int iNewWidth);

	// Returns: The combined size, in pixels, of all frozen columns.
    // Summary:	This member function is used by the header control to determine
	//			the total width of all frozen columns in the header control.  
	virtual int GetFrozenColWidth();
    
	// Ignore:
	//{{AFX_VIRTUAL(CXTHeaderCtrl)
	//}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTHeaderCtrl)
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	
	afx_msg BOOL OnItemchanging(NMHDR* pNMHDR, LRESULT* pResult);
	
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTFlatHeaderCtrl is a CHeaderCtrl derived class.  It is used to create
//			list box flat header controls that are similar to list box flat header
//			controls seen in Visual Studio&trade and Outlook&trade;.
class _XT_EXT_CLASS CXTFlatHeaderCtrl : public CXTHeaderCtrl
{
    DECLARE_DYNAMIC(CXTFlatHeaderCtrl)

public:

    // Summary: Constructs a CXTFlatHeaderCtrl object.
    CXTFlatHeaderCtrl();

    // Summary: Destroys a CXTFlatHeaderCtrl object, handles cleanup and de-allocation.
    virtual ~CXTFlatHeaderCtrl();

protected:
    
	int		m_nPos;			// Index of the popup menu contained in the menu.
	int		m_nOffset;		// Amount to offset the sort arrow.
	int		m_nSortedCol;	// Last column pressed during sort.
	UINT	m_popupMenuID;	// Popup menu resource ID.
	BOOL	m_bLBtnDown;	// TRUE if left mouse button is pressed.
	BOOL	m_bAscending;	// Used, when column is pressed, to draw a sort arrow.
	BOOL	m_bSortArrow;	// TRUE to draw a sort arrow.
	BOOL	m_bEnableMenus;	// TRUE to disable the popup menu display.
	bool	m_bInitControl;	// true for initialization.
	CPoint	m_pt;			// Point where right click took place.

public:

	// Returns: TRUE if the header control displays a sort arrow, otherwise returns FALSE.
	// Summary:	Call this member function to determine if the header control displays
	//			a sort arrow. 
	BOOL HasSortArrow();

	// Input:	popupMenuID - Resource ID for the popup menu used with the header.
	//			nPos - Position of the submenu to be displayed.
    // Summary:	This member function is called to associate a menu and toolbar 
    //			resource with the context menu.
    virtual void SetMenuID(UINT popupMenuID,int nPos=0);

	// Input:	bBoldFont - TRUE if the header's font should be bold.
    // Summary:	This member function must be called after creation to initialize 
    //			the font the header will use.
    virtual void InitializeHeader(BOOL bBoldFont);

	// Input:	bBoldFont - TRUE if the header's font should be bold.
    // Summary:	This member function can be used to toggle the font from bold to normal.
    virtual void SetFontBold(BOOL bBoldFont = TRUE);

	// Input:	nCol - Zero-based index of the column to set the sort image for.
	//			bAsc - TRUE if ascending, otherwise FALSE.
	// Returns: A zero-based index of the previously sorted column.
    // Summary:	This member function will set the sort image for the specified column.
    virtual int SetSortImage(int nCol,BOOL bAsc);

	// Input:	bSortArrow - Set to TRUE to draw the column sort arrow.
    // Summary:	Call this member function to enable or disable the column sort arrow.
    virtual void ShowSortArrow(BOOL bSortArrow);

	// Input:	pt  - Point to be tested.
	// Returns: The index of the item at the position specified by 'pt', otherwise returns -1.
    // Summary:	This member function determines which header item, if any, is at
	//			a specified cursor position.  
    virtual int HitTest(CPoint pt) const;

	// Input:	nFlag - Text alignment, either LVCFMT_CENTER, LVCFMT_LEFT, or LVCFMT_RIGHT.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	Call this member function to set the text justification for the
	//			header control. 
    virtual BOOL SetAlingment(int nFlag);

	// Returns: The index of the currently sorted column.
    // Summary:	Call this member function to return the index to the currently sorted
	//			column. 
    int GetSortedCol() const;
    
	// Returns: TRUE if the sorting order is ascending, otherwise returns FALSE.
    // Summary:	Call this member function to return the current sorting order. 
    BOOL GetAscending() const;

	// Input:	iCol - Zero-based index of the column to set the sort image for.
	//			uBitmapID - Resource ID of the bitmap to use.
	//			dwRemove - Style bits to be removed from the HD_ITEM::fmt variable.  For
    //			a column that does not display text, pass in HDF_STRING.
    // Summary:	Call this member function to set the bitmap image for the specified
    //			header item.
    void SetBitmap(int iCol,UINT uBitmapID,DWORD dwRemove=NULL);

	// Input:	bEnableMenus - TRUE to enable popup menus, and FALSE to disable.
	// Summary:	Call this member function to enable or disable the popup menu display whenever
	//			a user right clicks on the header control.
	void EnablePopupMenus(BOOL bEnableMenus);

protected:

	// Input:	pDC - Points to the current device context.
	//			rect - Area to be drawn.
    // Summary:	This member function is called by the header during paint operations.
    virtual void DrawBorders(CDC* pDC,CRect& rect);

#if _MSC_VER < 1200 // MFC 5.0

	// Returns: The number of header control items, if successful, otherwise returns -1.
    // Summary:	This member function retrieves a count of the items in a header
	//			control. 
    virtual int GetItemCount() const;

	// Input:	piArray - A pointer to the address of a buffer that receives the index 
    //			values of the items in the header control, in the order in which 
    //			they appear from left to right. 
	//			iCount - The number of header control items.
    // Summary:	This member function retrieves the index values of the items in
	//			the header control, in the order in which they appear from left to right.
    //			If you use the default value of 'iCount', GetOrderArray fills the
	//			parameter using GetItemCount. Returns nonzero if successful, otherwise,
	//			it returns zero.
    virtual BOOL GetOrderArray(LPINT piArray,int iCount=-1);

	// Input:	nIndex - The zero-based index of the header control item.
	//			lpRect - A pointer to the address of a RECT structure that receives the 
    //			bounding rectangle information.
	// Returns: Nonzero if successful, otherwise it returns zero.
    // Summary:	This method implements the behavior of the Win32 message HDM_GETITEMRECT, 
    //			as described in the Platform SDK. 
    virtual BOOL GetItemRect(int nIndex,LPRECT lpRect) const;
#endif

	// Input:	iIndex - Index of the column to be sorted.
    // Summary:	Called to send WM_NOTIFY to tell parent's owner that the column needs
	//			to be sorted.
    void SendNotify(int iIndex);

    // Ignore:
	//{{AFX_VIRTUAL(CXTFlatHeaderCtrl)
    public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTFlatHeaderCtrl)
    afx_msg void OnPaint();
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
    //}}AFX_MSG
    
	afx_msg void OnSortAsc();
    afx_msg void OnSortDsc();
    afx_msg void OnAlignLeft();
    afx_msg void OnAlignCenter();
    afx_msg void OnAlignRight();
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE BOOL CXTFlatHeaderCtrl::HasSortArrow() {
	return m_bSortArrow;
}
AFX_INLINE void CXTFlatHeaderCtrl::SetFontBold(BOOL bBoldFont) {
    ASSERT(::IsWindow(m_hWnd)); SetFont(bBoldFont?&xtAfxData.fontBold:&xtAfxData.font);
}
AFX_INLINE void CXTFlatHeaderCtrl::ShowSortArrow(BOOL bSortArrow) {
    ASSERT(::IsWindow(m_hWnd)); m_bSortArrow = bSortArrow; Invalidate();
}
AFX_INLINE int CXTFlatHeaderCtrl::GetSortedCol() const {
    ASSERT(::IsWindow(m_hWnd)); return m_nSortedCol;
}
AFX_INLINE BOOL CXTFlatHeaderCtrl::GetAscending() const {
    ASSERT(::IsWindow(m_hWnd)); return m_bAscending;
}
AFX_INLINE void CXTFlatHeaderCtrl::EnablePopupMenus(BOOL bEnableMenus) {
	m_bEnableMenus = bEnableMenus;
}
#if _MSC_VER < 1200 // MFC 5.0
AFX_INLINE int CXTFlatHeaderCtrl::GetItemCount() const {
    ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, HDM_GETITEMCOUNT, 0, 0L);
}
AFX_INLINE BOOL CXTFlatHeaderCtrl::GetItemRect(int nIndex, LPRECT lpRect) const {
    ASSERT(::IsWindow(m_hWnd)); ASSERT(lpRect != NULL); return (BOOL)::SendMessage(m_hWnd, HDM_GETITEMRECT, nIndex, (LPARAM)lpRect);
}
#endif

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTFLATHEADERCTRL_H__)