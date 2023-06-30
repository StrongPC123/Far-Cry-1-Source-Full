// XTColorPageCustom.h : header file
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

#if !defined(__XTCOLORCUSTOM_H__)
#define __XTCOLORCUSTOM_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: XT_HAS_FOCUS is an enumeration used by CXTColorBase for determining 
//			which color window has focus.
typedef	enum XT_HAS_FOCUS
{
	none,	// Neither color window has focus.
    wnd,	// Color wheel window has focus.
    lum		// Lumination window has focus.
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorBase is a CStatic derived base class.  It is used to create
//			custom color selection windows.
class _XT_EXT_CLASS CXTColorBase : public CStatic
{
public:

    // Summary: Constructs a CXTColorBase object.
	CXTColorBase();

	// Summary: Destroys a CXTColorBase object, handles cleanup and de-allocation.
    virtual ~CXTColorBase();

protected:

    CDC                 m_dcPicker;  // Background device context.
    COLORREF            m_clrColor;  // An RGB value that represents the color selection.
    static XT_HAS_FOCUS m_eHasFocus; // Determines which color has focus.

protected:
	
	bool m_bInitControl; // true for initialization.

public:

    double m_nLum;       // Current lumination value.
    double m_nSat;       // Current saturation value.
    double m_nHue;       // Current hue value.
    CPoint m_ptMousePos; // Current mouse postion relative to the device context.

	// Input:	clr - An RGB value that represents the color.
	//			bUpdate - true to update the cursor position.
	// Summary: Call this member function to set the color for the selection window.
	virtual void SetColor(COLORREF clr, bool bUpdate=true);

	// Returns: An RGB value that indicates the color selection.
	// Summary: Call this member function to return the current color selection.
	virtual COLORREF GetColor();

	// Input:	point - Current cursor location relative to the device context.
	//			bNotify - TRUE to notify the parent of change.
	// Summary: Call this member function to update the cursor position.
	virtual void UpdateCursorPos(CPoint point, BOOL bNotify=TRUE);

	// Input:	color - An RGB value that represents the value to convert to HSL.
	//			h -  Represents the color hue.
	//			s -  Represents the color saturation.
	//			l - Represents the color lumination.
	// Summary: This member function is called to convert an RGB color value to
	//			an HSL value.
	static void RGBtoHSL(COLORREF color, double *h, double *s, double *l );

	// Input:	h - Represents the color hue.
	//			l - Represents the color lumination.
	//			s - Represents the color saturation.
	// Returns: A COLORREF value.
    // Summary: This member function is called to convert an HLS value to an RGB
	//			color value.
	static COLORREF HLStoRGB(double h, double l, double s );

protected:

	// Ignore:
	//{{AFX_VIRTUAL(CXTColorBase)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL
    
protected:

    // Ignore:
	//{{AFX_MSG(CXTColorBase)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE COLORREF CXTColorBase::GetColor() {
	return m_clrColor;
}

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorWnd is a CXTColorBase derived class.  It is used to create a
//			color selection window.
class _XT_EXT_CLASS CXTColorWnd : public CXTColorBase
{
public:

    // Summary: Constructs a CXTColorWnd object.
	CXTColorWnd();

	// Summary: Destroys a CXTColorWnd object, handles cleanup and de-allocation.
    virtual ~CXTColorWnd();

public:

	// Input:	clr - An RGB value that represents the color.
	//			bUpdate - true to update cursor position.
	// Summary: Call this member function to set the color for the selection window.
	virtual void SetColor(COLORREF clr, bool bUpdate=true);

	// Input:	point - Current cursor location relative to the device context.
	//			bNotify - TRUE to notify the parent of change.
	// Summary: Call this member function to update the cursor position.
	virtual void UpdateCursorPos(CPoint point, BOOL bNotify=TRUE);

	// Input:	pDC - A CDC pointer that represents the current device context.
	// Summary: This member function is called to update the cross hair cursor.
	void DrawCrossHair(CDC* pDC);

	// Input:	h - Represents the new hue value to set.
	// Summary: This member function is called to set the hue for the color window.
	void SetHue(double h);

	// Input:	s - Represents the new saturation value to set.
	// Summary: This member function is called to set the saturation for the color
	//			window.
	void SetSaturation(double s);

    // Ignore:
	//{{AFX_VIRTUAL(CXTColorWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
    // Ignore:
	//{{AFX_MSG(CXTColorWnd)
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
    
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorLum is a CXTColorBase derived class.  It is used to create a
//			color luminance selection bar.
class _XT_EXT_CLASS CXTColorLum : public CXTColorBase
{
public:

    // Summary: Constructs a CXTColorLum object.
	CXTColorLum();

	// Summary: Destroys a CXTColorLum object, handles cleanup and de-allocation.
    virtual ~CXTColorLum();

protected:
	
	int m_nSliderPos; // Current location of the slider.

public:

	// Input:	clr - An RGB value that represents the color.
	//			bUpdate - true to update the cursor position.
	// Summary: Call this member function to set the color for the selection window.
	virtual void SetColor(COLORREF clr, bool bUpdate=true);

	// Input:	point - Current cursor location relative to the device context.
	//			bNotify - TRUE to notify the parent of change.
	// Summary: Call this member function to update the cursor position.
	virtual void UpdateCursorPos(CPoint point, BOOL bNotify=TRUE);

	// Input:	pDC - A CDC pointer that represents the current device context.
	// Summary: This member function is called to draw the indicator arrow.
	void DrawSliderArrow(CDC* pDC);

	// Input:	pDC - A CDC pointer that represents the current device context.
	// Summary: This member function is called to draw the selection bar.
	void DrawLuminanceBar(CDC* pDC);

	// Input:	l - Represents the new luminance value to set.
	// Summary: This member function is called to set the luminance for the color window.
	void SetLuminance(double l);

	// Input:	rect - Address to a CRect object.
	// Summary: This member function is called to get the display size of the luminance
	//			bar.
	void GetLumBarRect(CRect& rect);

    // Ignore:
	//{{AFX_VIRTUAL(CXTColorLum)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTColorLum)
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
    
	DECLARE_MESSAGE_MAP()
};

// forwards

class CXTColorDialog;

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorPageCustom is a multiple inheritance class derived from CXTDialogState
//			and CPropertyPage.  It is used to create a CXTColorPageCustom dialog.
class _XT_EXT_CLASS CXTColorPageCustom : CXTDialogState, public CPropertyPage
{
	DECLARE_DYNCREATE(CXTColorPageCustom)

public:

	// Input:	pParentSheet - A pointer to the parent sheet.
    // Summary: Constructs a CXTColorPageCustom object.
	CXTColorPageCustom(CXTColorDialog* pParentSheet=NULL);

	// Summary: Destroys a CXTColorPageCustom object, handles cleanup and de-allocation.
    virtual ~CXTColorPageCustom();

	//{{AFX_DATA(CXTColorPageCustom)
	
	enum { IDD = XT_IDD_COLORCUSTOM };
	CXTColorWnd	m_colorWnd;
	CXTColorLum	m_colorLum;
	CStatic	m_txtSat;
	CStatic	m_txtRed;
	CStatic	m_txtLum;
	CStatic	m_txtHue;
	CStatic	m_txtGreen;
	CStatic	m_txtBlue;
	CSpinButtonCtrl	m_spinSat;
	CSpinButtonCtrl	m_spinRed;
	CSpinButtonCtrl	m_spinLum;
	CSpinButtonCtrl	m_spinHue;
	CSpinButtonCtrl	m_spinGreen;
	CSpinButtonCtrl	m_spinBlue;
	CXTEdit	m_editHue;
	CXTEdit	m_editGreen;
	CXTEdit	m_editBlue;
	CXTEdit	m_editLum;
	CXTEdit	m_editRed;
	CXTEdit	m_editSat;
	int		m_nR;
	int		m_nB;
	int		m_nG;
	int		m_nH;
	int		m_nL;
	int		m_nS;
	//}}AFX_DATA

protected:

	CXTColorDialog* m_pParentSheet;  // A pointer to the parent sheet.

public:

	// Input:	color - An RGB value that represents the color.
	//			lum - Address of the integer to receive the lumination value (0-255).
	//			sat - Address of the integer to receive the saturation value (0-255).
	//			hue - Address of the integer to receive the hue value (0-255).
	// Summary: This member function is called to retrieve the HSL values of the RGB
	//			specified by color.
	void RGBtoHSL(COLORREF color, int* lum, int* sat, int* hue);

    // Ignore:
	//{{AFX_VIRTUAL(CXTColorPageCustom)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTColorPageCustom)
	afx_msg void OnChangeEdit();
	afx_msg void OnChangeEditLum();
	afx_msg void OnChangeEditHue();
	afx_msg void OnChangeEditSat();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	afx_msg LRESULT OnUpdateColor(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTCOLORCUSTOM_H__)