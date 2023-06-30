// XTOutlookBar.h interface for the CXTContentItems class.
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

#if !defined(__XTOUTLOOKBAR_H__)
#define __XTOUTLOOKBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: XT_CONTENT_ITEM is a stand alone helper structure class.  It is used
//			by CXTOutlookBar.
struct XT_CONTENT_ITEM
{
	int				m_nIndex;	// Zero-based index for the menu item.
	CString			m_strText;	// Text label for the menu item.
	CXTIconHandle	m_hIcon;	// Icon handle for the menu item.
};

// Summary: CList definition for XT_CONTENT_ITEM structure list.
typedef	CList<XT_CONTENT_ITEM*,XT_CONTENT_ITEM*> CXTContentItemList;

//////////////////////////////////////////////////////////////////////
// Summary: CCXTOutlookBar is a CListBox derived class.  It is used to implement
//			an Outlook bar style control.  It can only be used with the LBS_OWNERDRAWVARIABLE
//			style bit set. This is a simpler version of CXTOutBarCtrl and does not
//			allow for shortcut folders.
class _XT_EXT_CLASS CXTOutlookBar : public CListBox
{
    DECLARE_DYNAMIC(CXTOutlookBar)

public:

    // Summary: Constructs a CXTOutlookBar object.
    CXTOutlookBar();

    // Summary: Destroys a CXTOutlookBar object, handles cleanup and de-allocation.
    virtual ~CXTOutlookBar();

protected:

    int                 m_cxIcon;           // Width of the menu icon.
    int                 m_cyIcon;           // Height of the menu icon.
    int                 m_nIndex;           // Currently selected menu index.
    bool                m_bHilight;         // true when the menu item is selected.
    bool                m_bUserColors;      // true when the user has defined custom colors for the Outlook bar.
    CPoint              m_point;            // Holds the cursor position.
    COLORREF            m_clrBack;          // RGB value representing the background color.
    COLORREF            m_clrText;          // RGB value representing the text color.
    CXTContentItemList  m_arContentItems;   // Array of XT_CONTENT_ITEM structs that represent each item in the Outlook bar.

public:

	// Input:	dwStyle - Window style.
	//			rect - The size and position of the window, in client coordinates of 
	//			'pParentWnd'.
	//			pParentWnd - The parent window.
	//			nID - The ID of the child window.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function creates an Outlook Bar control. 
	virtual BOOL Create(DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID);

	// Input:	clrText - RGB value representing the text color.
	//			clrBack - RGB value representing the background color.
    // Summary:	Call this member function to set the text and background colors for
    //			the Outlook bar.
    void SetColors(COLORREF clrText,COLORREF clrBack);

	// Returns: An RGB value that represents the background color.
	// Summary:	This member function gets an RGB value that represents the control's
	//			background color. 
	COLORREF GetBackColor();

	// Returns: An RGB value that represents the text color.
	// Summary:	This member function gets an RGB value that represents the control's
	//			text color. 
	COLORREF GetTextColor();

	// Input:	iIndex - Specifies the zero-based index of the position to insert the
	//			menu item. If this parameter is -1, the menu item is added to the
	//			end of the list.
	//			nIconID - Resource ID of the icon associated with this menu item.
	//			lpszText - Points to the null-terminated string for the menu item
	// Returns: The zero-based index of the position at which the menu item was inserted. 
	//			The return value is LB_ERR if an error occurs.  The return value is LB_ERRSPACE
	//			if insufficient space is available to store the new menu item.
	// Summary:	This member function inserts a menu item into the Outlook bar.  Unlike
	//			the AddMenuItem member function, InsertMenuItem does not cause an Outlook
	//			bar with the LBS_SORT style to be sorted. 
	int InsertMenuItem(int iIndex,UINT nIconID,LPCTSTR lpszText);

	// Input:	nIconID - Resource ID of the icon associated with this menu item.
	//			lpszText - Points to the null-terminated string for the menu item.
	// Returns: The zero-based index to the menu item in the Outlook bar.  The return 
	//			value is LB_ERR if an error occurs.  The return value is LB_ERRSPACE 
	//			if insufficient space is available to store the new menu item.
	// Summary:	Call this member function to add a menu item to an Outlook bar.  If
	//			the Outlook bar was not created with the LBS_SORT style, the menu item
	//			is added to the end of the Outlook bar.  Otherwise, the menu item is
	//			inserted into the Outlook bar, and the Outlook bar is sorted.  If the
	//			Outlook bar was created with the LBS_SORT style but not the LBS_HASSTRINGS
	//			style, the framework sorts the Outlook bar by one or more calls to
	//			the CompareItem member function.  Use InsertMenuItem to insert a menu
	//			item into a specific location within the Outlook bar. 
	int AddMenuItem(UINT nIconID,LPCTSTR lpszText);

	// Input:	iItem - Specifies the zero-based index of the menu item to retrieve.
	// Returns: An XT_CONTENT_ITEM pointer.
	// Summary:	Call this member function to return an XT_CONTENT_ITEM object that
	//			represents the menu item specified by 'iItem'. 
	XT_CONTENT_ITEM* GetMenuItem(int iItem);

    // Ignore:
	//{{AFX_VIRTUAL(CXTOutlookBar)
    public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    //}}AFX_VIRTUAL

protected:

	// Input:	pDC - Points to the current device context.
	// Summary:	This member function is called by the control for flicker free drawing.
	void OnNoFlickerPaint(CDC* pDC);

    // Ignore:
	//{{AFX_MSG(CXTOutlookBar)
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTOutlookBar::SetColors(COLORREF clrText, COLORREF clrBack) {
    m_clrBack = clrBack; m_clrText = clrText; m_bUserColors = true;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTOUTLOOKBAR_H__)