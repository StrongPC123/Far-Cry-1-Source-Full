// XTColorSelectorCtrl.h : interface for the CXTColorSelectorCtrl class.
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

#if !defined(__XTCOLORCTRL_H__)
#define __XTCOLORCTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: XT_COLOR_CELL is a stand alone helper structure class.  It is used by
//			the CXTColorSelectorCtrl class.
struct XT_COLOR_CELL
{
    UINT     nID;         // Command ID of the color cell.
    UINT     nIndex;      // Index of the color cell.
    bool     bChecked;    // true if the cell is checked.
    CRect    rect;        // Size of the color cell.
    DWORD    dwStyle;     // Windows style for the cell.
    TCHAR    szText[256]; // Tooltip text displayed for the color cell.
    COLORREF clr;         // An RGB value that represents the color of the cell.
};

// Summary: CList definition for the color cell structure array.
typedef CList<XT_COLOR_CELL*,XT_COLOR_CELL*> CXTColorCellArray;

// Summary: Array of listener windows to send notifications to.
typedef CArray<HWND, HWND> CXTListenerArray;

/////////////////////////////////////////////////////////////////////////////
// Summary: CXTColorSelectorCtrl is a CWnd derived class. It is used to create a CXTColorSelectorCtrl 
//			control that will allow a user to select colors.
class _XT_EXT_CLASS CXTColorSelectorCtrl : public CWnd
{

public:

	// Summary: Constructs a CXTColorSelectorCtrl object.
	CXTColorSelectorCtrl();

    // Summary: Destroys a CXTColorSelectorCtrl object, handles cleanup and de-allocation.
    virtual ~CXTColorSelectorCtrl();

protected:

    int               m_nRows;           // Number of rows in the color popup window.
    int               m_nCols;           // Number of columns in the color popup window.
    int               m_nLastIndex;      // Index of the last button on the popup window.
    int               m_nBtnCount;       // Number of buttons in this window.
    int               m_nCurSel;         // Currently selected index.
    int               m_nPressed;        // Pressed button.
    BOOL              m_bColorDlg;       // TRUE if the color dialog is open.
    CWnd*             m_pParentWnd;      // Points to the parent window for the popup window.    
    CSize             m_sizeButton;      // cx and cy size for a color picker button.
    CRect             m_rcWnd;           // Rect for the popup window.
    CRect             m_rcBorders;       // Control borders
    DWORD             m_dwPopup;         // Color popup window style.
    CPoint            m_point;           // Last known cursor position.
    COLORREF          m_clrColor;        // An RGB value that represents the currently selected color.
    COLORREF          m_clrDefault;      // An RGB value that represents the default color for the popup window. 
    CToolTipCtrl      m_tooltip;         // Tooltip control.
    CXTListenerArray  m_listeners;       // Array of listener windows to be sent notifications.
    CXTColorCellArray m_arCells;         // Array of color items.
    static CUIntArray m_arUserDefColors; // Array of user defined colors.

public:

	// Input:	pColorCell - Receives a pointer to the currently selected button.
	// Returns: The zero (0) based index of the currently selected button.
    // Summary: This member function will return the index of the currently selected
	//			color and will initialize 'pColorCell' struct. 
    int GetCurSel(XT_COLOR_CELL* pColorCell);

	// Input:	nIndex - An integer value that represents the zero (0) based
    //			index of the button to be selected.
	// Summary: This member function will select a button based upon its index.
    void SetCurSel(int nIndex);

	// BULLETED LIST:

	// Input:	rect - A reference to a CRect object that represents the
    //			size of the color popup window.
	//			pParentWnd - Points to the parent window for the color popup.
	//			dwPopup - Style for the popup window. Styles can be one or 
    //			more of the following:
	//			[ul]
    //			[li]<b>CPS_XT_NOFILL</b> The color picker will display a No Fill
	//			button rather than the default Automatic Color button.[/li]
    //			[li]<b>CPS_XT_EXTENDED</b> The color picker will display 40
	//			extended colors rather than the default 16 colors.[/li]
    //			[li]<b>CPS_XT_MORECOLORS</b> The color picker will display
	//			a More Colors button which will display a CXTColorDialog.[/li]
    //			[li]<b>CPS_XT_SHOW3DSELECTION</b> Displays the color selection
	//			box with a 3D raised border in CXTColorDialog.[/li]
    //			[li]<b>CPS_XT_SHOWHEXVALUE</b> Displays the hex equivalent
	//			of the selected color.[/li]
	//			[/ul]
    //			clrColor - An RGB value that represents the currently selected color for the
	//			popup window.
	//			clrDefault - Specifies the default color for the color popup.  If the
    //			current style includes CPS_NOFILL this parameter is
    //			ignored.
	// Returns:	TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function handles the creation of the color popup window.
	BOOL Create(CRect rect,CWnd* pParentWnd,DWORD dwPopup,COLORREF clrColor,COLORREF clrDefault=CLR_DEFAULT);

	// Input:	hwndListener - A handle to the listener window.  Messages will be 
	//			sent to it.
	// Summary: This member function adds a window to send color picker notifications to.
	void AddListener(HWND hwndListener);

	// Input:	hwndListener - A handle to the listener window to remove.
	// Summary: This member function remove a window from notification list.
	void RemoveListener(HWND hwndListener);

	// Input:	pColorCell - Points to an XT_COLOR_CELL object.
	// Summary: This member function is called to select a color cell.
    void SelectColorCell(XT_COLOR_CELL* pColorCell);
	
	// Input:	clr - Color of selected cell.
	// Summary: This member function is called to select a color cell.
    void SelectColor(COLORREF clr);

	// Input:	iIndex - Index into the color cell array.
	// Returns: An XT_COLOR_CELL object.
    // Summary: This member function is called to return an XT_COLOR_CELL struct
	//			from the color cell array. 
    XT_COLOR_CELL* GetCellFromIndex(int iIndex);

	// Input:	cxLeft - Specifies the left position.
	//			cyTop - Specifies the top.
	//			cxRight - Specifies the right.
	//			cyBottom - Specifies the bottom.
    // Summary: This member function is called to set the size of the borders
    //			for the control.
	void SetBorders(int cxLeft = 0, int cyTop = 0, int cxRight = 0, int cyBottom = 0);
	
protected:

	// Input:	pDC - A CDC pointer that represents the current device context.
    // Summary: This member function is called to draw the color selector.
    void DrawColorSelector(CDC* pDC);

	// Input:	pColorCell - An XT_COLOR_CELL object.
	//			pDC - A CDC pointer that represents the current device context.
	//			bHilite - TRUE to highlight the color cell.
	//			bPressed - TRUE to press the color cell.
    // Summary: This member function is called to draw the cell specified by 'pColorCell'.
	void DrawColorCell(XT_COLOR_CELL* pColorCell, CDC* pDC, BOOL bHilite, BOOL bPressed);

    // Input:	nCurSel - Current index of the selected color box or button
    //			in the color popup window.
	// Summary: This member function will finish the selection process for the color
    //			box or button in the color popup window.
	virtual void EndSelection(int nCurSel);

    // Ignore:
	//{{AFX_VIRTUAL(CXTColorSelectorCtrl)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    protected:
    //}}AFX_VIRTUAL

	void _EndSelection(int nCurSel, LPARAM callerParam);
	
protected:
	// Ignore:
	//{{AFX_MSG(CXTColorSelectorCtrl)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseLeave();
	afx_msg UINT OnNcHitTest(CPoint point);
	//}}AFX_MSG

	friend class CXTPopupColorTearOff;

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTColorSelectorCtrl::SetBorders(int cxLeft, int cyTop, int cxRight, int cyBottom) {
	m_rcBorders.left = cxLeft; m_rcBorders.top = cyTop; m_rcBorders.right = cxRight; m_rcBorders.bottom = cyBottom;	
}

//////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //__XTCOLORCTRL_H__
