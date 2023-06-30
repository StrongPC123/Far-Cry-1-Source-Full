// XTColorPageStandard.h : header file
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

#if !defined(__XTCOLORSTANDARD_H__)
#define __XTCOLORSTANDARD_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: XT_COLORCELL is a stand alone helper structure class.  It is used by
//			the CXTColorHex class.
struct XT_COLORCELL
{
    int      direction[4]; // Array that indicates which cell index is to the left, top, right and bottom of the color cell.
    BOOL     bSmall;       // TRUE if the color cell is a standard selection rectangle.
    CPoint*  pPoint;       // Represents the current cursor position.
    COLORREF clr;          // An RGB value.
};

// Summary: Definition for the color cell array.
typedef CList<XT_COLORCELL*,XT_COLORCELL*>   CCXTColorCell;

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorHex is a CStatic derived class.  It displays a color selection
//			grid used by CXTColorPageStandard.
class _XT_EXT_CLASS CXTColorHex : public CStatic
{
public:

    // Summary: Constructs a CXTColorHex object.
    CXTColorHex();

    // Summary: Destroys a CXTColorHex object, handles cleanup and de-allocation.
    virtual ~CXTColorHex();

protected:

    CDC           m_dcPicker;     // Background device context.
    bool          m_bInitControl; // true for initialization.
    BOOL          m_bLBtnDown;    // TRUE when the left mouse button is pressed.
    BOOL          m_bSmallCell;   // TRUE when a small color cell is selected.
    CPoint        m_ptCurrent;    // Holds the last known selection point.
    COLORREF      m_clrColor;     // A COLORREF value that contains the RGB information for the current color.
    CCXTColorCell m_arCells;      // Array of XT_COLORCELL structs that represent displayed color cells.

public:

	// Input:	point - XY location of the color to get RGB information for.
	// Summary: Call this member function to retrieve RGB information for the color
	//			found at the location specified by point. Returns a COLORREF value.
	COLORREF ColorFromPoint(CPoint point);

	// Input:	clr - An RGB value that represents the color.
	// Returns: A pointer to an XT_COLORCELL struct.
    // Summary: Call this member function to get a pointer to the XT_COLORCELL struct
	//			that is represented by 'clr'. 
	XT_COLORCELL* GetColorCell(COLORREF clr);

	// Input:	iIndex - Index into m_arCells list.
	// Summary: Call this member function to select the color specified by 'iIndex'.
	void SetSelectedColor(int iIndex);

	// Input:	clr - An RGB value that represents the color.
	// Summary: Call this member function to select the color specified by 'clr'.
	void SetSelectedColor(COLORREF clr);
	
 	// Returns: An RGB color value that represents the selected color.
	// Summary: Call this member function to return the currently selected color.
	COLORREF GetSelectedColor() const;

protected:

	// Input:	pDC - A CDC pointer that represents the current device context.
	//			point - XY position of the starting point for the first pixel drawn.
	//			clr - An RGB value that represents the color of the cell to draw.
	//			l - Index of the cell to be selected when VK_LEFT is pressed.
	//			u - Index of the cell to be selected when VK_UP is pressed.
	//			r - Index of the cell to be selected when VK_RIGHT is pressed.
	//			d - Index of the cell to be selected when VK_DOWN is pressed.
	// Summary: This member function is called by the class to draw a single color cell.
	void DrawCell(CDC* pDC, CPoint point, COLORREF clr, int l, int u, int r, int d);

	// Input:	pDC - A CDC pointer that represents the current device context.
	//			point - XY position of the starting point for the first pixel drawn.
	//			clr - An RGB value that represents the color of the cell to draw.
	//			l - Index of the cell to be selected when VK_LEFT is pressed.
	//			u - Index of the cell to be selected when VK_UP is pressed.
	//			r - Index of the cell to be selected when VK_RIGHT is pressed.
	//			d - Index of the cell to be selected when VK_DOWN is pressed.
	// Summary: This member function is called by the class to draw a single large
	//			color cell.
    void DrawLargeCell(CDC* pDC, CPoint point, COLORREF clr, int l, int u, int r, int d);

	// Input:	pDC - A CDC pointer that represents the current device context.
	// Summary: This member function is called to draw the selection window.
	void DrawColorSelector(CDC* pDC);

	// Input:	pDC - A CDC pointer that represents the current device context.
	// Summary: This member function is called to select a color cell if any are selected.
	void DrawSelectCell(CDC* pDC);

	// Input:	pDC - A CDC pointer that represents the current device context.
	// Summary: This member function is called to select a large color cell if any
	//			are selected.
	void DrawLargeSelectCell(CDC* pDC);

	// Input:	point - Current location of the color to select.
	// Summary: This member function updates the color selection based on the XY 
	//			coordinates	specified by 'point'.
	void UpdateSelection(CPoint point);

	// Input:	pDC - A CDC pointer that represents the current device context.
	// Summary: This member function is called to select a cell when a new selection is made.
	void SelectColorCell(CDC* pDC);

    // Ignore:
	//{{AFX_VIRTUAL(CXTColorHex)
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	virtual bool IsValidColor(COLORREF cr) const;

protected:

    // Ignore:
	//{{AFX_MSG(CXTColorHex)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE COLORREF CXTColorHex::GetSelectedColor() const {
	return m_clrColor;
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorPageStandard is a multiple inheritance class derived from 
//			CXTDialogState and CPropertyPage.  It is used to create a CXTColorPageStandard
//			dialog.
class _XT_EXT_CLASS CXTColorPageStandard : CXTDialogState, public CPropertyPage
{
	DECLARE_DYNCREATE(CXTColorPageStandard)

public:

	// Input:	pParentSheet - Points to the parent property sheet.
    // Summary: Constructs a CXTColorPageStandard object.
	CXTColorPageStandard(CXTColorDialog* pParentSheet=NULL);

	// Summary: Destroys a CXTColorPageStandard object, handles cleanup and de-allocation.
    virtual ~CXTColorPageStandard();

	// Input:	clr - An RGB value that represents the color.
	// Summary: This member function is called to set the selected color for the page.
	void SetColor(COLORREF clr);

protected:

	CXTColorDialog* m_pParentSheet; // Points to the parent property sheet.

	//{{AFX_DATA(CXTColorPageStandard)

	enum { IDD = XT_IDD_COLORSTANDARD };
	CXTColorHex	m_colorHex;
	//}}AFX_DATA
    	
	// Ignore:
	//{{AFX_VIRTUAL(CXTColorPageStandard)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL
    
protected:

    // Ignore:
	//{{AFX_MSG(CXTColorPageStandard)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	afx_msg LRESULT OnUpdateColor(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTColorPageStandard::SetColor(COLORREF clr) {
	m_colorHex.SetSelectedColor(clr);
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTCOLORSTANDARD_H__)