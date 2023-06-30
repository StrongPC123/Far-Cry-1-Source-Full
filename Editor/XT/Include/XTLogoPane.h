// XTLogoPane.h interface for the CXTLogoPane class.
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

#if !defined(__XTLOGOPANE_H__)
#define __XTLOGOPANE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTBasePane is a CWnd derived class.  It is the base class used for
//			creating CWnd objects to place in status bar panes that do custom paint
//			routines.
class _XT_EXT_CLASS CXTBasePane : public CWnd
{
	DECLARE_DYNCREATE(CXTBasePane)

public:
	
    // Summary: Constructs a CXTBasePane object.
	CXTBasePane();

	// Summary: Destroys a CXTBasePane object, handles cleanup and de-allocation.
    virtual ~CXTBasePane();

protected:

	CString m_strWindowText;  // NULL terminated string that represents the pane text.
	
    // Ignore:
	//{{AFX_VIRTUAL(CXTLogoPane)
	//}}AFX_VIRTUAL

public:
	
	// Input:	lpszWindowName - Text string to be associated with this pane.
	//			pParentWnd - Pointer to the owner status bar window.
	//			dwStyle - Window style.
	//			nID - Control ID.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to create a status bar pane object that performs
	//			custom draw routines.  
	virtual BOOL Create(LPCTSTR lpszWindowName,CWnd* pParentWnd,DWORD dwStyle=WS_CHILD|WS_VISIBLE,UINT nID=0xffff);

protected:

	// Input:	pDC - Pointer to the device context.
	//			rcClient - Size of the client area to draw.
	// Summary:	Override this virtual function in your derived class to perform your
	//			custom drawing routines.
	virtual void DoPaint(CDC* pDC,CRect& rcClient);

    // Ignore:
	//{{AFX_MSG(CXTBasePane)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTLogoPane is a CXTBasePane derived class.  CXTLogoPane works with
//			CXTStatusBar and allows you to create a logo to place in your status
//			bar area.
class _XT_EXT_CLASS CXTLogoPane : public CXTBasePane
{
public:
	
    // Summary: Constructs a CXTLogoPane object.
	CXTLogoPane();

	// Summary: Destroys a CXTLogoPane object, handles cleanup and de-allocation.
    virtual ~CXTLogoPane();

protected:

	CFont m_Font;		// Font to be used.
	CSize m_sizeText;	// Size of the text to be displayed.

public:

	// Input:	lpszLogoText - A NULL terminated string that represents the text to be displayed.
	// Summary:	This member function will set the text to be displayed in the logo
	//			pane.
	virtual void SetLogoText(LPCTSTR lpszLogoText);

	// Returns: A CString object that represents the text that is displayed
	//			in the logo pane.
	// Summary:	This member function returns a CString that represents the logo text.
	virtual CString GetLogoText();

	// Input:	lpszFontName - A NULL terminated string that represents the text to be displayed.
	//			nHeight - Initial height for the font.
	//			nWeight - Initial weight for the font.
	//			bItalic - TRUE if the font is italic.
	//			bUnderline - TRUE if the font is underlined.
	// Summary:	This member function sets the font to be displayed in the logo pane.
	virtual void SetLogoFont(LPCTSTR lpszFontName,int nHeight=24,int nWeight=FW_BOLD,BOOL bItalic=TRUE,BOOL bUnderline=FALSE);

	// Input:	logFont - Address of a LOGFONT structure.
	// Summary:	This member function sets the font to be displayed in the logo pane.
	virtual void SetLogoFont(LOGFONT& logFont);

	// Returns: A CSize object that represents the current size of the logo pane text.
	// Summary:	This member function will return the size of the text displayed in
	//			the logo pane. 
	CSize GetTextSize();

	// Summary: This member function will calculate the size of the text that is displayed
	//			in the logo pane and initializes 'm_sizeText'.
	void SetTextSize();

    // Ignore:
	//{{AFX_VIRTUAL(CXTLogoPane)
	//}}AFX_VIRTUAL

protected:

	// Input:	pDC - Pointer to the device context.
	//			rcClient - Size of the client area to draw.
	// Summary:	Override this virtual function in your derived class to perform your
	//			custom drawing routines.
	virtual void DoPaint(CDC* pDC,CRect& rcClient);

    // Ignore:
	//{{AFX_MSG(CXTLogoPane)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE CSize CXTLogoPane::GetTextSize() {
	SetTextSize(); m_sizeText.cx+=2; return m_sizeText;
}
AFX_INLINE void CXTLogoPane::SetLogoText(LPCTSTR lpszLogoText) {
	m_strWindowText = lpszLogoText;
}
AFX_INLINE CString CXTLogoPane::GetLogoText() {
	return m_strWindowText;
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTIconPane is a CXTBasePane derived class.  CXTIconPane works with
//			CXTStatusBar and allows you to create a logo to place in your status
//			bar area.
class _XT_EXT_CLASS CXTIconPane : public CXTBasePane
{
public:
	
    // Summary: Constructs a CXTIconPane object.
	CXTIconPane();

	// Summary: Destroys a CXTIconPane object, handles cleanup and de-allocation.
    virtual ~CXTIconPane();

protected:

	CSize			m_sizeIcon; // Size of the icon to display.
	CXTIconHandle	m_hIcon;	// Icon that is displayed in the status pane.

public:

	// Input:	nIconID - Resource ID of the icon to display.
	// Summary:	Call this member function to set the pane icon for this object.
	void SetPaneIcon(int nIconID);

	// Input:	lpszIconID - Resource ID of the icon to display.
	// Summary:	Call this member function to set the pane icon for this object.
	void SetPaneIcon(LPCTSTR lpszIconID);

	// Input:	pDC - Pointer to the device context.
	//			rcClient - Size of the client area to draw.
	// Summary:	Override this virtual function in your derived class to perform your
	//			custom drawing routines.
	virtual void DoPaint(CDC* pDC,CRect& rcClient);

    // Ignore:
	//{{AFX_MSG(CXTIconPane)
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTLOGOPANE_H__)