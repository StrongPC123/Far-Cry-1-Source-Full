// XTButton.h : interface for the CXTButton class.
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

#if !defined(__XTBUTTON_H__)
#define __XTBUTTON_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTButton is a CButton derived class.  It is used to create flat style
//			and image buttons.  This control has several features including flat,
//			semi-flat and three dimensional borders.
class _XT_EXT_CLASS CXTButton : public CButton
{
	DECLARE_DYNAMIC(CXTButton)

public:
	
    // Summary: Constructs a CXTButton object.
	CXTButton();

	// Summary: Destroys a CXTButton object, handles cleanup and de-allocation.
    virtual ~CXTButton();

protected:

    int      m_nBorderGap;    // Gap between the button edge and the image.
    int      m_nImageGap;     // Gap between the button text and the image.
    BOOL     m_bHilite;       // TRUE if the button is highlighted.
    BOOL     m_bUserPosition; // TRUE if the user has defined the XY coordinates for the image and text.
    BOOL     m_bLBtnDown;     // TRUE if the left mouse button is pressed.
    BOOL     m_bAltColor;     // TRUE if user defined colors are used.
    BOOL     m_bPainted;      // Used during paint operations.
    BOOL     m_bChecked;      // TRUE if the button is checked.
    CSize    m_sizeImage;     // Initial size of the button.
    DWORD    m_dwxStyle;      // The style of the button.  See SetXButtonStyle().
    HICON    m_hIcon;         // Handle to the icon associated with the button.
    HICON    m_hIconPushed;   // Handle to the icon associated with the button when it is pressed.
    CPoint   m_ptImage;       // XY location of the button image.
    CPoint   m_ptText;        // XY location of the button text.
    HBITMAP  m_hBitmap;       // Bitmap associated with the button.
    HBITMAP  m_hBitmapMono;   // Disabled bitmap associated with the button.
    COLORREF m_clrBtnText;    // An RGB value that represents the button text color.
    COLORREF m_clr3DFace;     // An RGB value that represents the button face color.
    COLORREF m_clr3DHilight;  // An RGB value that represents the 3D border highlight color.
    COLORREF m_clr3DShadow;   // An RGB value that represents the 3D border shadow color.

	//////////////////////////////////////////////////////////////////////
    // XP Flat colors
	//////////////////////////////////////////////////////////////////////

    BOOL     m_bXPFUserColors;   // TRUE if the user set custom XP-Flat colors.
    DWORD    m_dwInitSignature;  // Used for one-time initialization.
    COLORREF m_clrXPFHighlight;  // An RGB value that represents the background color for highlighted buttons in XP-Flat mode.
    COLORREF m_clrXPFPressed;    // An RGB value that represents the background color for pressed buttons in XP-Flat mode.
    COLORREF m_clrXPFBorder;     // An RGB value that represents the frame color for highlighted/pressed buttons in XP-Flat mode.

public:

	// Input:	bChecked - Specifies whether the button is to be checked.  TRUE will 
	//			check the button. FALSE will uncheck it.
    // Summary: This member function sets the highlighting state of a button control.
	//			This is the same as its CButton counterpart, except the m_bChecked flag
	//			is set.  This is done because MFC does not recognize the button as being
	//			checked unless it a radio or check box.
	void SetStateX(BOOL bChecked);

	// Input:	nGap - Amount, in pixels, of the gap between the button edge and the image.
	// Returns: The previous border gap value.
    // Summary: This member function will set the gap between the button's edge and
	//			the image. 
	int SetBorderGap(int nGap);

	// Input:	nGap - Amount, in pixels, of the gap between the button text and the image.
	// Returns: The previous border gap value.
    // Summary: This member function will set the gap between the button's text and
	//			the image. 
	int SetImageGap(int nGap);

	// Input:	clr3DFace - An RGB value that represents the user defined face color for 
	//			three dimensional display elements.
	//			clr3DHilight - An RGB value that represents the user defined highlight 
	//			color for three dimensional display elements (edges facing the light 
	//			source.)
	//			clr3DShadow - An RGB value that represents the user defined shadow color
	//			for three dimensional display elements (edges facing away from the
	//			light source).
	//			clrBtnText - An RGB value that represents the user defined text color
	//			on push buttons.
	// Summary: This method will allow the user to define the default colors for the
	//			background shadow and highlight colors for the button.
	virtual void SetAlternateColors(COLORREF clr3DFace, COLORREF clr3DHilight, COLORREF clr3DShadow, COLORREF clrBtnText);

	// Input:	clrFace - An RGB value that represents the user defined face color.
	// Summary: This member function sets the default face color for the button.
	virtual void SetColorFace(COLORREF clrFace);
    
	// Returns: An RGB value that represents the current face color.
	// Summary: This member function gets the current face color.
    virtual COLORREF GetColorFace();

	// Input:	clrHilite - An RGB value that represents the user defined highlight color.
	// Summary: This member function sets the default highlight color for the button.
	virtual void SetColorHilite(COLORREF clrHilite);

	// Input:	clrShadow - An RGB value that represents the user defined shadow color.
	// Summary: This member function sets the default shadow color for the button.
    virtual void SetColorShadow(COLORREF clrShadow);

	// Input:	clrText - An RGB value that represents the user defined text color.
	// Summary: This member function sets the default text color for the button.
	virtual void SetColorText(COLORREF clrText);

	// Input:	clrBorder - An RGB value that represents the user defined border color 
	//			for the flat button.
	//			clrHighlight - An RGB value that represents the user defined highlight 
	//			color for the flat button.
	//			clrPressed - An RGB value that represents the user defined color for when
	//			the flat button is pressed.
	// Summary: This member function sets the colors for the BS_XT_XPFLAT mode.
    virtual void SetXPFlatColors(COLORREF clrBorder, COLORREF clrHighlight, COLORREF clrPressed);

	// Input:	size - CSize object that represents the size of the icon.
	//			hIcon - Handle to the normal icon.
	//			hIconPushed - Handle to the pressed icon.
	//			bRedraw - Specifies whether the button is to be redrawn.  A nonzero value
	//			redraws the button. A zero value does not redraw the button. The button is
	//			redrawn by default.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: This member function will set the normal and pushed state icons for
	//			the push button. 
	virtual BOOL SetIcon(CSize size, HICON hIcon, HICON hIconPushed=NULL, BOOL bRedraw=TRUE);

	// Input:	size - CSize object that represents the size of the icon.
	//			nID - Resource ID for the normal icon.
	//			nPushedID - Resource ID for the pressed icon.
	//			bRedraw - Specifies whether the button is to be redrawn.  A nonzero value
	//			redraws the button. A zero value does not redraw the button.  The button is
	//			redrawn by default.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: This member function will set the normal and pushed state icons for
	//			the push button.
	virtual BOOL SetIcon(CSize size, UINT nID, UINT nPushedID=0, BOOL bRedraw=TRUE);

	// Input:	size - CSize object that represents the size of the icon.
	//			lpszID - Resource string ID for the normal icon.
	//			lpszPushedID - Resource string ID for the pressed icon.
	//			bRedraw - Specifies whether the button is to be redrawn.  A nonzero value
	//			redraws the button. A zero value does not redraw the button.  The button is
	//			redrawn by default.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: This member function will set the normal and pushed state icons for
	//			the push button. 
	virtual BOOL SetIcon(CSize size, LPCTSTR lpszID, LPCTSTR lpszPushedID=NULL, BOOL bRedraw=TRUE);
	
	// Input:	size - CSize object that represents the size of the bitmap.
	//			nID - Resource ID for the bitmap.
	//			bRedraw - Specifies whether the button is to be redrawn.  A nonzero value redraws
	//			the button. A zero value does not redraw the button.  The button is redrawn
	//			by default.
	// Returns:	TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will set the bitmap associated with the button.
	virtual BOOL SetBitmap(CSize size,UINT nID,BOOL bRedraw=TRUE);

	// Input:	ptImage - XY location of the image displayed on the button.
	//			ptText - XY location of the text displayed on the button.
	//			bRedraw - Specifies whether the button is to be redrawn.  A nonzero value redraws
	//			the button. A zero value does not redraw the button.  The button is redrawn
	//			by default.
	// Returns:	TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will set the XY location of the text and image
	//			that is displayed on the push button.
	virtual BOOL SetTextAndImagePos(CPoint ptImage,CPoint ptText,BOOL bRedraw=TRUE);

	// BULLETED LIST:

	// Input:	dwxStyle - Specifies the button style. It can be one or more of the following:
	//			[ul]
	//			[li]<b>BS_XT_FLAT</b> Draws a flat button.[/li]
	//			[li]<b>BS_XT_SEMIFLAT</b> Draws a semi-flat button.[/li]
	//			[li]<b>BS_XT_TWOROWS</b> Draws images and text that are centered.[/li]
	//			[li]<b>BS_XT_SHOWFOCUS</b> Draws a focus rect when the button
	//			has input focus.[/li]
	//			[li]<b>BS_XT_HILITEPRESSED</b> Highlights the button when pressed.[/li]
	//			[li]<b>BS_XT_XPFLAT</b> Draws a flat button ala Office XP.[/li]
	//			[/ul]
	//			bRedraw - Specifies whether the button is to be redrawn.  A nonzero value redraws
	//			the button. A zero value does not redraw the button.  The button is redrawn
	//			by default.
	// Returns: The previous style that was set.
	// Summary: This member function will set the display style for the button.
	DWORD SetXButtonStyle(DWORD dwxStyle,BOOL bRedraw=TRUE);

	// Returns:	The button styles for this CXTButton object.
	// Summary:	This function returns only the BS_XT_ style values, not any of the
	//			other window styles.
	virtual DWORD GetXButtonStyle();

	// Summary: This function removes the icon or bitmap from the button.  All resources
	//			taken by the image are freed.
	virtual void ResetImage();

protected:

	// Input:	point - XY location of the text and image that are displayed.
	//			size - Initial size of the image associated with the button.
	// Summary: This member function will set the proper XY coordinates for the button
	//			text and image.
    void OffsetPoint(CPoint& point, CSize size);

	// Summary: This member function is called to free all associated GDI resources
	//			that have been allocated.
	virtual void CleanUpGDI();

	// Input:	pDC - A CDC pointer that represents the current device context.
	//			nState - A UINT value that represents the current state for the button.
	//			rcItem - A CRect reference that represents the current size for the button.
	// Summary: This member function draws the icon for the button, if any.
    virtual void DrawButtonIcon(CDC* pDC, UINT nState, CRect& rcItem);

	// Input:	pDC - A CDC pointer that represents the current device context.
	//			nState - A UINT value that represents the current state for the button.
	//			rcItem - A CRect reference that represents the current size for the button.
	// Summary: This member function will draw the text for the button, if any.
	virtual void DrawButtonText(CDC* pDC, UINT nState, CRect& rcItem);

	// Input:	pDC - A CDC pointer that represents the current device context.
	//			nState - A UINT value that represents the current state for the button.
	//			rcItem - A CRect reference that represents the current size for the	button.
	// Summary: This member function will draw the bitmap for the button, if any.
	virtual void DrawButtonBitmap(CDC* pDC, UINT nState, CRect& rcItem);

	// Input:	bRemoveAmpersand - TRUE to remove the '&' used for mnemonics.
	// Returns: A CString object that represents the button's text.
    // Summary: Call this member function to return the buttons text minus the '&'.
	virtual CString GetButtonText(BOOL bRemoveAmpersand);

	// Input:	pDC - A CDC pointer that represents the current device context.
	//			nState - A UINT value that represents the current state for the button.
	//			rcItem - A CRect reference that represents the current size for the	button.
	//			bHasPushedImage - Indicates if the button has a separate image for the 
	//			pushed state.
    // Returns: The top-left position for the button image.
	// Summary: Call this member function to calculate the position of the button image.
    virtual CPoint CalculateImagePosition(CDC* pDC, UINT nState, CRect& rcItem, bool bHasPushedImage);


	// Input:	bDepressed - TRUE if the button is pressed, otherwise FALSE.
    // Summary: Call this helper function to set the pressed state and redraw the button.
	void NoteButtonDepressed(BOOL bDepressed);

	// Summary: Defers control initialization
	void DeferInitialUpdate();

    // Ignore:
	//{{AFX_VIRTUAL(CXTButton)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL
    
	// Ignore:
	//{{AFX_MSG(CXTButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysColorChange();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE int CXTButton::SetImageGap(int nGap) {
	int nOldGap = m_nImageGap; m_nImageGap = nGap; return nOldGap;
}
AFX_INLINE int CXTButton::SetBorderGap(int nGap) {
	int nOldGap = m_nBorderGap; m_nBorderGap = nGap; return nOldGap;
}
AFX_INLINE DWORD CXTButton::GetXButtonStyle() {
	ASSERT(::IsWindow(m_hWnd)); return m_dwxStyle;
}
AFX_INLINE void CXTButton::SetStateX(BOOL bChecked) {
	CButton::SetState(bChecked); m_bChecked = bChecked;
}
AFX_INLINE COLORREF CXTButton::GetColorFace() {
    return m_clr3DFace;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTBUTTON_H__)
