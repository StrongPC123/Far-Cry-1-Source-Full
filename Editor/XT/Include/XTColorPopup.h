// XTColorPopup.h : interface for the CXTColorPopup class.
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

#if !defined(__XTCOLORPOPUP_H__)
#define __XTCOLORPOPUP_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorPopup is a CXTColorSelectorCtrl derived class.  It is used to create a CXTColorPopup
//			window that will allow a user to select colors and add custom colors
//			to a color list.
class _XT_EXT_CLASS CXTColorPopup : public CXTColorSelectorCtrl
{
public:

	// Input:	bAutoDelete - TRUE if the color picker window is to be self deleting.
	//			callerParam - Caller parameter, will be reported back as an LPARAM of 
	//			color popup notification messages.
    // Summary: Constructs a CXTColorPopup object.
    CXTColorPopup(BOOL bAutoDelete=FALSE, LPARAM callerParam = 0);

    // Summary: Destroys a CXTColorPopup object, handles cleanup and de-allocation.
    virtual ~CXTColorPopup();

protected:

    BOOL  m_bAutoDelete;        // TRUE if the popup window is to be self deleting.
    BOOL  m_bDisplayShadow;     // TRUE if the popup window shall render its shadow.
    CRect m_rcExclude;          // Area to exclude from shadow display.
    const LPARAM m_callerParam; // Caller parameter that will be reported back as an LPARAM of all notification messages.

public:

	// Input:	clrColor - An RGB value that represents the user defined color to display in
	//			the recent color list.
	// Summary: This member function will add a color to the user defined color list.
	static void AddUserColor(COLORREF clrColor);

	// Summary: Call this member function to reset the user defined color list.  Calling
	//			this function will remove all user defined colors from the MRU list.
	static void ResetUserColors();

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
    //			current style includes CPS_NOFILL this parameter is	ignored.
	// Returns:	TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function handles the creation of the color popup window.
    virtual BOOL Create(CRect& rect,CWnd* pParentWnd,DWORD dwPopup,COLORREF clrColor,COLORREF clrDefault=CLR_DEFAULT);

	// Input:	hwndListener - A handle to the listener window.  Messages will be sent to it.
	// Summary: This member function adds a window to send color picker notifications to.
	void AddListener(HWND hwndListener);

	// Input:	bDisplayShadow - Flag that tells if the shadow is to be displayed.
    // Summary: This member function enables or disables the drawing of popup shadows.
	//			Call this function after instantiating the color popup as a part of an
	//			object's initialization.  Do <b>not</b> call it after popup creation since
	//			doing so may cause unpredictable results.
    void DisplayShadow(BOOL bDisplayShadow = TRUE);

protected:

	// Input:	nCurSel - Current index of the selected color box or button
    //			in the color popup window.
    // Summary: This member function will finish the selection process for the color
    //			box or button in the color popup window.
    virtual void EndSelection(int nCurSel);
    
    // Ignore:
	//{{AFX_VIRTUAL(CXTColorPopup)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    protected:
    virtual void PostNcDestroy();
    //}}AFX_VIRTUAL

protected:

    // Ignore:
	//{{AFX_MSG(CXTColorPopup)
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnDestroy();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	//}}AFX_MSG

public:
#if _MFC_VER >= 0x0700 //MFC 7.0
    afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
#else
    afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#endif //MFC 7.0

    DECLARE_MESSAGE_MAP()

};

//////////////////////////////////////////////////////////////////////

AFX_INLINE 	void CXTColorPopup::AddUserColor(COLORREF clrColor) {
	m_arUserDefColors.Add( clrColor );
}
AFX_INLINE void CXTColorPopup::ResetUserColors() {
	m_arUserDefColors.RemoveAll();
}
AFX_INLINE void CXTColorPopup::DisplayShadow( BOOL bDisplayShadow/* = TRUE*/) {
	m_bDisplayShadow = bDisplayShadow;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPOPUP_H__C0330BB1_9A77_4EE4_9F15_00ECAFC5FFC9__INCLUDED_)