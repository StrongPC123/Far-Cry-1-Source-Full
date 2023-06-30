// XTHyperLink.h interface for the CXTHyperLink class.
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

#if !defined(__XTHYPERLINK_H__)
#define __XTHYPERLINK_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTHyperLink is a CStatic derived class.  It is used to display a text
//			string that is used as a hyperlink control.  The link's active, hover,
//			and selected color attributes are configurable.
class _XT_EXT_CLASS CXTHyperLink : public CStatic
{
    DECLARE_DYNAMIC(CXTHyperLink)

public:

    // Summary: Constructs a CXTHyperLink object.
    CXTHyperLink();

    // Summary: Destroys a CXTHyperLink object, handles cleanup and de-allocation.
    virtual ~CXTHyperLink();

// Attributes
protected:
	
	bool			m_bShellExec;	// true to cause the hyperlink to call ShellExecute for the specified URL.
	bool			m_bInitControl; // true for initialization.
	bool			m_bUnderline;	// Set to true for underlined text.
	bool			m_bMouseOver;	// Set to true when the cursor is hovering over the control.
	bool			m_bVisited;		// Set to true when the hyperlink has been activated.
	bool			m_bTipEnabled;	// Set to false to disable tooltips.
	CString			m_strLink;		// String object that represents the URL for this control.
	CString			m_strTipText;	// String object that represents the tooltip text for this control.
	HCURSOR			m_hcurHand;		// Handle to the default cursor for this control. 
	COLORREF		m_clrLink;		// An RGB value representing the color for the hyperlink text color.
	COLORREF		m_clrHover;		// An RGB value representing the color for the hyperlink text color when the mouse is hovering.
	COLORREF		m_clrVisited;	// An RGB value representing the color for the hyperlink text color when the URL has been visited.
	COLORREF		m_clrBack;		// An RGB value representing the background fill color for the hyperlink control.
	CToolTipCtrl	m_toolTip;		// Tooltip to be displayed for the control.

public:

	// Input:	bEnable - true to enable ShellExecute, false to disable.
	// Summary:	Call this member function to enable or disable ShellExecute to be called
	//			for the URL that is specified for the hyperlink control.  If disabled,
	//			you can add a BN_CLICKED message handler to perform your own task when
	//			the link is clicked.
	void EnableShellExecute(bool bEnable);

	// Input:	lpszLink - A NULL terminated character string that
    //			represents the URL for the hyperlink.
    // Summary:	This member function sets the URL for the hyperlink control.
    virtual void SetURL(LPCTSTR lpszLink);

	// Returns: A CString object that represents the URL for the hyperlink control.
    // Summary:	This member function retrieves the URL for the hyperlink. 
    virtual CString GetURL() const;
    
	// Input:	clrLink - An RGB value representing the color for the hyperlink text color.
	//			clrVisited - An RGB value representing the color for the hyperlink text color
	//			when the mouse is hovering.
	//			clrHover - An RGB value representing the color for the hyperlink text color
	//			when the URL has been visited.
    // Summary:	This member function will set the RGB color values for the hyperlink text.
    virtual void SetColors(COLORREF clrLink,COLORREF clrVisited,COLORREF clrHover);

	// Returns: An RGB color value that represents the current hyperlink text color.
    // Summary:	This member function retrieves the current hyperlink text color.
    virtual COLORREF GetLinkColor() const;

	// Returns: An RGB color value that represents the current hyperlink text color when 
	//			the link has been activated.
    // Summary:	This member function retrieves the current hyperlink text color
	//			when the link has been activated. 
    virtual COLORREF GetVisitedColor() const;

	// Returns: An RGB color value that represents the current hyperlink text color when 
	//			the mouse hovers over the hyperlink.
    // Summary:	This member function retrieves the current hyperlink text color
	//			when the mouse hovers over the hyperlink. 
    virtual COLORREF GetHoverColor() const;

	// Input:	bVisited - A boolean flag that, when set to true, causes the control to
	//			display the hyperlink text color as visited.
    // Summary:	This member function will cause the control to display the hyperlink
	//			text color as visited.
    virtual void SetVisited(bool bVisited = true);
    
	// Returns: true if the URL has been visited, otherwise returns false.
    // Summary:	This member function checks to see if the user has visited the URL
	//			associated with the hyperlink. 
    virtual bool GetVisited() const;

	// Input:	hCursor - Handle of the cursor to be associated with the hyperlink control.
    // Summary:	This member function will set the cursor to be displayed when the
	//			mouse hovers over the hyperlink control.
    virtual void SetLinkCursor(HCURSOR hCursor);

	// Returns: A handle to the current cursor for the hyperlink control.
    // Summary:	This member function will get a handle to the current cursor associated
	//			with the hyperlink control. 
    virtual HCURSOR GetLinkCursor() const;

	// Input:	bUnderline - A boolean flag that, when set to true, causes the control to
	//			display the text as underlined.
    // Summary:	This member function will cause the hyperlink to display the text
	//			as underlined.  The default value is 'true.'
    virtual void SetUnderline(bool bUnderline = true);

	// Returns: true if the hyperlink text is underlined, otherwise returns false.
    // Summary:	This member function checks to see if the hyperlink text is underlined.
    virtual bool GetUnderline() const;

	// Input:	clrBack - An RGB value representing the background fill color.
	// Summary:	This member function will set the background fill color for the hyperlink
	//			control.  By default the hyperlink control uses the system color for COLOR_3DFACE.
	virtual void SetBackColor(COLORREF clrBack);

	// Returns: A COLOREF value.
	// Summary:	This member function will return the current background fill color for the
	//			hyperlink control.  
	virtual COLORREF GetBackColor() const;

	// Input:	lpszLink - Points to a null terminated string that represents the URL to
	//			jump to.  If NULL, the default URL is activated.
	//			nShowCmd - Display state for shell command associated with the URL.  The default
	//			is SW_SHOW.
	// Returns: A value greater than 32 if successful, or an error value that is less than
	//			or equal to 32 if unsuccessful.
    // Summary:	This member function is called to execute the shell command associated
	//			with the current URL. 
    virtual HINSTANCE GotoURL(LPCTSTR lpszLink=NULL,int nShowCmd=SW_SHOW);

	// Returns: true if the hyperlink control is a text control, otherwise returns false.
    // Summary:	This member function checks to see if the hyperlink control is a
	//			text control. 
    virtual bool IsTextControl();

	// Input:	bEnable - true to enable tooltips.
	// Summary:	Call this member function to enable or disable tooltips for the hyperlink
	//			control.
	void EnableToolTips(bool bEnable=true);

	// Input:	lpszTipText - NULL terminated string that represents the new tooltip text to display.
	// Summary:	Call this member function to set the tooltip text for the tooltip associated 
	//			with the hyperlink control.
	void SetTipText(LPCTSTR lpszTipText);

    // Ignore:
	//{{AFX_VIRTUAL(CXTHyperLink)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

	virtual bool IsLinkHot();
	virtual void ReportError(int iErrorNo);

    // Ignore:
	//{{AFX_MSG(CXTHyperLink)
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg UINT OnNcHitTest(CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnClicked();
	//}}AFX_MSG
	
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);
    
    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTHyperLink::SetURL(LPCTSTR lpszLink) {
    m_strLink = lpszLink; if (!m_bInitControl) m_toolTip.UpdateTipText(lpszLink, this);
}
AFX_INLINE CString CXTHyperLink::GetURL() const {
    return m_strLink;
}
AFX_INLINE void CXTHyperLink::SetColors(COLORREF clrLink, COLORREF clrVisited, COLORREF clrHover) {
    m_clrLink = clrLink; m_clrVisited = clrVisited; m_clrHover = clrHover; Invalidate();
}
AFX_INLINE COLORREF CXTHyperLink::GetLinkColor() const {
    return m_clrLink;
}
AFX_INLINE COLORREF CXTHyperLink::GetVisitedColor() const {
    return m_clrVisited;
}
AFX_INLINE COLORREF CXTHyperLink::GetHoverColor() const {
    return m_clrHover;
}
AFX_INLINE void CXTHyperLink::SetVisited(bool bVisited) {
    m_bVisited = bVisited; InvalidateRect(NULL);
}
AFX_INLINE bool CXTHyperLink::GetVisited() const {
    return m_bVisited;
}
AFX_INLINE void CXTHyperLink::SetLinkCursor(HCURSOR hCursor) {
    m_hcurHand = hCursor;
}
AFX_INLINE HCURSOR CXTHyperLink::GetLinkCursor() const {
    return m_hcurHand;
}
AFX_INLINE void CXTHyperLink::SetUnderline(bool bUnderline) {
    m_bUnderline = bUnderline; SetFont(m_bUnderline ? &xtAfxData.fontULine : &xtAfxData.font); Invalidate();
}
AFX_INLINE bool CXTHyperLink::GetUnderline() const {
    return m_bUnderline;
}
AFX_INLINE bool CXTHyperLink::IsTextControl() {
    return ((GetStyle() & 0xFF) <= SS_RIGHT) ? true:false;
}
AFX_INLINE void CXTHyperLink::EnableShellExecute(bool bEnable) {
	m_bShellExec = bEnable;
}
AFX_INLINE void CXTHyperLink::SetBackColor(COLORREF clrBack) {
	m_clrBack = clrBack;
}
AFX_INLINE COLORREF CXTHyperLink::GetBackColor() const {
	return ( m_clrBack == -1 ) ? xtAfxData.clr3DFace : m_clrBack;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTHYPERLINK_H__)