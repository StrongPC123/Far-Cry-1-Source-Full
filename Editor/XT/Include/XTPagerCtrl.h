// XTPagerCtrl.h interface for the CXTPagerCtrl class.
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

#if !defined(__XTPAGERCTRL_H__)
#define __XTPAGERCTRL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTPagerCtrl is a CWnd derived class.  It is used to contain and scroll
//			another window, and wraps the windows pager API.
class _XT_EXT_CLASS CXTPagerCtrl : public CWnd
{
    DECLARE_DYNAMIC(CXTPagerCtrl)

public:

    // Summary: Constructs a CXTPagerCtrl object.
    CXTPagerCtrl();

    // Summary: Destroys a CXTPagerCtrl object, handles cleanup and de-allocation.
    virtual ~CXTPagerCtrl();

protected:

public:

	// Input:	dwStyle - Specifies the window style attributes.  The following styles are
	//			also used when creating pager controls, and can be combined by using
	//			the or (|) operator:
	//			[ul]
	//			[li]<b>PGS_AUTOSCROLL</b> The pager control will scroll when the
	//			user hovers the mouse over one of the scroll buttons.[/li]
    //			[li]<b>PGS_DRAGNDROP</b> The contained window can be a drag-and-drop
	//			target.  The pager control will automatically scroll if an item
	//			is dragged from outside the pager over one of the scroll buttons.[/li]
	//			[li]<b>PGS_HORZ</b> Creates a pager control that can be scrolled
	//			horizontally.  This style and the PGS_VERT style are mutually
	//			exclusive and cannot be combined.[/li]
    //			[li]<b>PGS_VERT</b> Creates a pager control that can be scrolled
	//			vertically.  This is the default direction no direction style
	//			is specified.  This style and the PGS_HORZ style are mutually
	//			exclusive and cannot be combined.[/li]
    //			[/ul]
	//			rect - The size and position of the window, in client coordinates of
	//			'pParentWnd'.
	//			pParentWnd - The parent window.
	//			nID - The ID of the child window.
	// Summary:	This member function creates a pager child window and attaches it to
	//			this CWnd object. Returns nonzero if successful, otherwise returns zero.
    virtual BOOL Create(DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID);

	// Input:	hwndChild - Handle to the window to be contained.
    // Summary:	This member function sets the contained window for the pager control.
	//			This command will not change the parent of the contained window.  It
	//			only assigns a window handle to the pager control for scrolling.  In
	//			most cases, the contained window will be a child window.  If this is
	//			the case, the contained window should be a child of the pager control.
    void SetChild(HWND hwndChild);

    // Summary: This member function forces the pager control to recalculate the
	//			size of the contained window.  Using this command will result in a
	//			PGN_CALCSIZE notification being sent.
    void RecalcSize();

	// Input:	bForward - BOOL value that determines if mouse forwarding is enabled or
	//			disabled.  If this value is nonzero, mouse forwarding is enabled.
	//			If this value is zero, mouse forwarding is disabled.
    // Summary:	This member function enables or disables mouse forwarding for the
	//			pager control.  When mouse forwarding is enabled, the pager control
	//			forwards WM_MOUSEMOVE messages to the contained window.
    void ForwardMouse(BOOL bForward);

	// Input:	clr - COLORREF value that contains the new background color of the
	//			pager control.
	// Returns: A COLORREF value that contains the previous background color.     
    // Summary:	This member function sets the current background color for the pager
	//			control. 
    COLORREF SetBkColor(COLORREF clr);

	// Returns: A COLORREF value that contains the current background color.
    // Summary:	This member function retrieves the current background color for
	//			the pager control. 
    COLORREF GetBkColor();

	// Input:	iBorder - New size of the border, in pixels.  This value should not be 
	//			larger than the pager button or less than zero.  If 'iBorder' 
	//			is too large, the border will be drawn the same size as the 
	//			button.  If 'iBorder' is negative, the border size will be set 
	//			to zero.
	// Returns: An integer value that contains the previous border size, in pixels.
    // Summary:	This member function sets the current border size for the pager
	//			control. 
    int SetBorder(int iBorder);

	// Returns: An integer value that contains the current border size, in pixels.
    // Summary:	This member function retrieves the current border size for the pager
	//			control. 
    int GetBorder();

	// Input:	iPos - Integer value that contains the new scroll position, in pixels.        
    // Summary:	This member function sets the scroll position for the pager control.
    void SetPos(int iPos);

	// Returns: An integer value that contains the current scroll position, in pixels.
    // Summary:	This member function retrieves the current scroll position of the
	//			pager control. 
    int GetPos();

	// Input:	iSize - Integer value that contains the new button size, in pixels.
	// Returns: An integer value that contains the previous button size, in pixels.
    // Summary:	This member function sets the current button size for the pager
	//			control. 
    int SetButtonSize(int iSize);

	// Returns: An integer value that contains the current button size, in pixels.
    // Summary:	This member function retrieves the current button size for the pager
	//			control. 
    int GetButtonSize();

	// Input:	iButton - Indicates which button to retrieve the state for.  See the description
	//			for 'iButton' in PGM_GETBUTTONSTATE for a list of possible values.
	// Returns: The state of the button specified in 'iButton'. See the return value 
	//			description in PGM_GETBUTTONSTATE for a list of possible values.
    // Summary:	This member function retrieves the state of the specified button
	//			in a pager control. 
    DWORD GetButtonState(int iButton);

	// Input:	ppdt - Address of an IDropTarget pointer that receives the interface
	//			pointer.  It is the caller's responsibility to call Release on this
	//			pointer when it is no longer needed.
    // Summary:	This member function retrieves a pager control's IDropTarget interface
	//			pointer.
    void GetDropTarget(IDropTarget **ppdt);

    // Ignore:
	//{{AFX_VIRTUAL(CXTPagerCtrl)
    //}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTPagerCtrl)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTPagerCtrl::SetChild(HWND hwndChild) {
    ASSERT(::IsWindow(m_hWnd)); Pager_SetChild(m_hWnd, hwndChild);
}
AFX_INLINE void CXTPagerCtrl::RecalcSize() {
    ASSERT(::IsWindow(m_hWnd)); Pager_RecalcSize(m_hWnd);
}
AFX_INLINE void CXTPagerCtrl::ForwardMouse(BOOL bForward) {
    ASSERT(::IsWindow(m_hWnd)); Pager_ForwardMouse(m_hWnd, bForward);
}
AFX_INLINE COLORREF CXTPagerCtrl::SetBkColor(COLORREF clr) {
    ASSERT(::IsWindow(m_hWnd)); return Pager_SetBkColor(m_hWnd, clr);
}
AFX_INLINE COLORREF CXTPagerCtrl::GetBkColor() {
    ASSERT(::IsWindow(m_hWnd)); return Pager_GetBkColor(m_hWnd);
}
AFX_INLINE int CXTPagerCtrl::SetBorder(int iBorder) {
    ASSERT(::IsWindow(m_hWnd)); return Pager_SetBorder(m_hWnd, iBorder);
}
AFX_INLINE int CXTPagerCtrl::GetBorder() {
    ASSERT(::IsWindow(m_hWnd)); return Pager_GetBorder(m_hWnd);
}
AFX_INLINE void CXTPagerCtrl::SetPos(int iPos) {
    ASSERT(::IsWindow(m_hWnd)); Pager_SetPos(m_hWnd, iPos);
}
AFX_INLINE int CXTPagerCtrl::GetPos() {
    ASSERT(::IsWindow(m_hWnd)); return Pager_GetPos(m_hWnd);
}
AFX_INLINE int CXTPagerCtrl::SetButtonSize(int iSize) {
    ASSERT(::IsWindow(m_hWnd)); return Pager_SetButtonSize(m_hWnd, iSize);
}
AFX_INLINE int CXTPagerCtrl::GetButtonSize() {
    ASSERT(::IsWindow(m_hWnd)); return Pager_GetButtonSize(m_hWnd);
}
AFX_INLINE DWORD CXTPagerCtrl::GetButtonState(int iButton) {
    ASSERT(::IsWindow(m_hWnd)); return Pager_GetButtonState(m_hWnd, iButton);
}
AFX_INLINE void CXTPagerCtrl::GetDropTarget(IDropTarget **ppdt) {
    ASSERT(::IsWindow(m_hWnd)); Pager_GetDropTarget(m_hWnd, ppdt);
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTPAGERCTRL_H__)