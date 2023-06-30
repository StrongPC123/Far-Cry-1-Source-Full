// XTColorDialog.h : header file
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

#if !defined(__XTCOLORSHEET_H__)
#define __XTCOLORSHEET_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorDialog is a multiple inheritance class derived from CXTDialogState
//			and CPropertySheet. It is an Office&trade; style color dialog and can be used
//			as a drop in replacement for the MFC CColorDialog API.
class _XT_EXT_CLASS CXTColorDialog : CXTDialogState, public CPropertySheet
{
	DECLARE_DYNAMIC(CXTColorDialog)

public:

	// BULLETED LIST:

	// Input:	clrNew - An RGB value that represents the new color selection.
	//			clrCurrent - An RGB value that represents the default color selection.
	//			dwFlags - Style for color dialog. It can be one of the following:
	//			[ul]
	//			[li]<b>CPS_XT_SHOW3DSELECTION</b> Displays the color selection
	//			box with a 3D raised border.[/li]
	//			[li]<b>CPS_XT_SHOWHEXVALUE</b> Displays the hex equivalent of
	//			the selected color.[/li]
	//			[/ul]
	//			pWndParent - A pointer to the dialog box’s parent or owner window.
    // Summary:	Constructs a CXTColorDialog object.
	CXTColorDialog(COLORREF clrNew,COLORREF clrCurrent,DWORD dwFlags = 0L,CWnd* pWndParent = NULL);

	// Summary: Destroys a CXTColorDialog object, handles cleanup and de-allocation.
    virtual ~CXTColorDialog();

protected:

    DWORD    m_dwStyle;    // Styles for the dialog.
    CEdit    m_editHex;    // Edit window to display the color hex value.
    CRect    m_rcNew;      // Size of the rectangle that represents the new color.
    CRect    m_rcCurrent;  // Size of the rectangle that represents the current color.
    COLORREF m_clrNew;     // A COLORREF value that contains the RGB information for the new color.
    COLORREF m_clrCurrent; // A COLORREF value that contains the RGB information for the current color.

public:

	// Input:	clr - An RGB value that represents the color.
	//			bNotify - TRUE to notify tab pages of a color change.
	// Summary: Call this member function to set the current color selection to the
	//			color value specified in 'clr'.  The dialog box will automatically update
	//			the user’s selection based on the value of the 'clr' parameter.
	void SetNewColor(COLORREF clr, BOOL bNotify=TRUE);

	// Input:	clr - An RGB value that represents the color.
	// Summary: Call this function to set the new color selection to the color 
	//			value specified in 'clr'.
	void SetCurrentColor(COLORREF clr);

	// Returns: A COLORREF value that contains the RGB information for
	//			the current color specified when the dialog was instantiated.
	// Summary: Call this member function to retrieve the information about the current
	//			color. 
	COLORREF GetCurrentColor();
	
	// Returns: A COLORREF value that contains the RGB information
	//			for the new color selected in the color dialog box.
    // Summary: Call this member function to retrieve the information about the color
	//			the user selected. 
	COLORREF GetColor();

	// Input:	strText - A NULL terminated string.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: Call this member function to copy the string specified by 'strText' to the
	//			Windows clipboard. 
	BOOL CopyToClipboard(CString strText);

	// Input:	clr - An RGB value that represents the color.
	// Returns: A CString object.
    // Summary: This member function returns a CString object that represents the
	//			HEX conversion for the specified RGB color. 
	CString RGBtoHex(COLORREF clr) const;

    // Ignore:
	//{{AFX_VIRTUAL(CXTColorDialog)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTColorDialog)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE COLORREF CXTColorDialog::GetCurrentColor() {
	return m_clrCurrent;
}
AFX_INLINE COLORREF CXTColorDialog::GetColor() {
	return m_clrNew;
}

#define WM_XT_UPDATECOLOR	(WM_USER+1024)

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTCOLORSHEET_H__)