// XTResize.h: interface for the CXTResize class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//--------------------------------------------------------------------
// Based on the resizable classes created by Torben B. Haagh. Used by permission.
// http://www.codeguru.com/dialog/torbenResizeDialog.shtml
//--------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////

#if !defined(__XTRESIZE_H__)
#define __XTRESIZE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: The XT_SIZING structure is a stand alone helper structure class.  It
//			is used by the CXTResize class to maintain size and ID for a particular
//			window being sized.
struct XT_SIZING
{
	UINT			id;		// Control identifier of the window sized.
	XT_RESIZERECT	rrc;	// Size of the window sized.
};

//////////////////////////////////////////////////////////////////////
// Summary: The CXTSizeIcon class is a CStatic derived helper class. It is used
//			by CXTResize to display the sizing grip in the lower right corner of
//			a sizing window.
class _XT_EXT_CLASS CXTSizeIcon : public CStatic
{
public:
	
	// Summary: Constructs a CXTSizeIcon object.
	CXTSizeIcon();

protected:

	HCURSOR m_hCursor;		// Handle to the cursor displayed for the size icon.
	CBitmap	m_bmSizeIcon;	// Size icon bitmap

	// Ignore:
	//{{AFX_MSG(CXTSizeIcon)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTItem is a stand alone helper class.  It is used by CXTResize to
//			maintain information about each item to be sized or moved.
class _XT_EXT_CLASS CXTItem
{
public:
	
	// Input:	pwnd - Pointer to the window to be sized or moved.
	//			rrcSizing - Reference to a CXTResizeRect object.
	//			rcWindow - Reference to a CRect object.
	//			bDeleteWnd - TRUE if the window is to be deleted.
	// Summary:	Constructs a CXTItem object.
	CXTItem(CWnd* pwnd,const CXTResizeRect& rrcSizing,CRect& rcWindow,BOOL bDeleteWnd);

	// Summary: Destroys a CXTItem object, handles cleanup and de-allocation.
	virtual ~CXTItem();
	
	BOOL m_bDeleteWnd;				// TRUE if the window is to be deleted
	BOOL m_bInitialSize;			// Initial size/move has been completed.
	CWnd* m_pwnd;					// A pointer to the window to be sized or moved.
	CXTResizeRect m_rrcSizing;		// Sizing option.
	CXTResizeRect m_rrcWindow;		// Last control size.
	CXTResizeRect m_rrcInitWindow;  // Initial control size.
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTResize is a base class.  It is used by resizing dialogs, property
//			sheets, and form views.  It acts as a manager to maintain size and location
//			of the dialog and dialog items.
class _XT_EXT_CLASS CXTResize
{
public:

	// BULLETED LIST:

	// Input:	object - Points to the parent or owner window object, of type CWnd, to which the
	//			resizing object belongs.
	//			nFlags - Flags that are to be passed to CXTResize that specify the attributes
	//			of the resizing property page.  They can be one or more of the following,
	//			and can be combined using the or (|) operator:
	//			[ul]
    //			[li]<b>SZ_NOSIZEICON</b> Do not add size icon.[/li]
    //			[li]<b>SZ_NOHORISONTAL</b> No horizontal resizing.[/li]
    //			[li]<b>SZ_NOVERTICAL</b> No vertical resizing.[/li]
    //			[li]<b>SZ_NOMINSIZE</b> Do not require a minimum size.[/li]
    //			[li]<b>SZ_NOCLIPCHILDREN</b> Do not set clip children style.[/li]
    //			[li]<b>SZ_NOTRANSPARENTGROUP</b> Do not set transparent style
	//			for group boxes.[/li]
	//			[/ul]
    // Summary:	Constructs a CXTResize object.
	CXTResize(CWnd* pwnd,const UINT nFlags = 0);

	// Summary: Destroys a CXTResize object, handles cleanup and de-allocation.
	virtual ~CXTResize();

	// Input:	nID - Specifies the control's ID.
	//			left - How much the left side will move when the dialog is resized.
	//			top - How much the top side will move when the dialog is resized.
	//			right - How much the right side will move when the dialog is resized.
	//			bottom - How much the bottom side will move when the dialog is resized.
	// Summary:	The SetResize function specifies how much each side of a control will
	//			move when the dialog is resized.  If a control should be repositioned
	//			(e.g. an OK button) then all four sides should move by the same amount
	//			of pixels, as the dialog is resized.  If a control should be resized
	//			just as much as the dialog (e.g. the list control in the file dialog),
	//			then the left and top sides shouldn't move, and the right and bottom
	//			sides should move by the same amount of pixels as the dialog.
	void SetResize(const UINT nID,XT_RESIZE left,XT_RESIZE top,XT_RESIZE right,XT_RESIZE bottom);

	// Input:	nID - Specifies the control's ID.
	//			rrcSizing - How much the left, top, right and bottom sides will move when 
	//			the dialog is resized.
	// Summary:	The SetResize function specifies how much each side of a control will
	//			move when the dialog is resized.  If a control should be repositioned
	//			(e.g. an OK button) then all four sides should move by the same amount
	//			of pixels, as the dialog is resized.  If a control should be resized
	//			just as much as the dialog (e.g. the list control in the file dialog),
	//			then the left and top sides shouldn't move, and the right and bottom
	//			sides should move by the same amount of pixels as the dialog.
	void SetResize(const UINT nID,const XT_RESIZERECT& rrcSizing);

	// Input:	nID - Specifies the control's ID.
	//			hWnd - HWND of the dialog item to be sized.
	//			rrcSizing - How much the left, top, right and bottom sides will move when 
	//			the dialog is resized.
	// Summary:	The SetResize function specifies how much each side of a control will
	//			move when the dialog is resized.  If a control should be repositioned
	//			(e.g. an OK button) then all four sides should move by the same amount
	//			of pixels, as the dialog is resized.  If a control should be resized
	//			just as much as the dialog (e.g. the list control in the file dialog),
	//			then the left and top sides shouldn't move, and the right and bottom
	//			sides should move by the same amount of pixels as the dialog.
	void SetResize(const UINT nID,const HWND hWnd,const XT_RESIZERECT& rrcSizing);

	// Input:	nID - Specifies the control's ID.
	//			hWnd - HWND of the dialog item to be sized.
	//			rpTopLeft - How much the top and left sides will move when the dialog is resized.
	//			rpBottomRight - How much the bottom and right sides will move when the dialog is resized.
	// Summary:	The SetResize function specifies how much each side of a control will
	//			move when the dialog is resized.  If a control should be repositioned
	//			(e.g. an OK button) then all four sides should move by the same amount
	//			of pixels, as the dialog is resized.  If a control should be resized
	//			just as much as the dialog (e.g. the list control in the file dialog),
	//			then the left and top sides shouldn't move, and the right and bottom
	//			sides should move by the same amount of pixels as the dialog.
	void SetResize(const UINT nID,const HWND hWnd,const XT_RESIZEPOINT& rpTopLeft,const XT_RESIZEPOINT& rpBottomRight);

	// Input:	nID - Specifies the control's ID.
	//			rpTopLeft - How much the top and left sides will move when the dialog is resized.
	//			rpBottomRight - How much the bottom and right sides will move when the dialog is resized.
	// Summary:	The SetResize function specifies how much each side of a control will
	//			move when the dialog is resized.  If a control should be repositioned
	//			(e.g. an OK button) then all four sides should move by the same amount
	//			of pixels, as the dialog is resized.  If a control should be resized
	//			just as much as the dialog (e.g. the list control in the file dialog),
	//			then the left and top sides shouldn't move, and the right and bottom
	//			sides should move by the same amount of pixels as the dialog.
	void SetResize(const UINT nID,const XT_RESIZEPOINT& rpTopLeft,const XT_RESIZEPOINT& rpBottomRight);

	// Input:	arr[] - Array of XT_SIZING structures that specify how much the left, top,
	//			right and bottom sides of the dialog item will move when the dialog
	//			is resized.
	// Summary:	The SetResize function specifies how much each side of a control will
	//			move when the dialog is resized.  If a control should be repositioned
	//			(e.g. an OK button) then all four sides should move by the same amount
	//			of pixels, as the dialog is resized.  If a control should be resized
	//			just as much as the dialog (e.g. the list control in the file dialog),
	//			then the left and top sides shouldn't move, and the right and bottom
	//			sides should move by the same amount of pixels as the dialog.
	void SetResize(XT_SIZING arr[]);

	// Input:	sz - Specifies the minimum width and height the dialog can be sized to.
	// Summary:	This member function sets the minimum size explicitly.  Initial size
	//			is the default.
	void SetMinSize(CSize& sz);

	// Input:	sz - Specifies the maximum width and height the dialog can be sized to.
	// Summary:	This member function sets the maximum size.  No maximum is the default.
	void SetMaxSize(CSize& sz);

	// Input:	pszSection - Name of a section in the initialization file or a key in the Windows
	//			registry where placement information is stored.
	// Summary:	This member function saves the window placement to the registry.
	void SavePlacement(LPCTSTR pszSection);

	// Input:	pszSection - Name of a section in the initialization file or a key in the Windows
	//			registry where placement information is stored.
	// Summary:	This member function loads saved window placement information from
	//			the registry.
	void LoadPlacement(LPCTSTR pszSection);

	// Input:	pszSection - Name of a section in the initialization file or a key in the Windows
	//			registry where placement information is stored.
	// Summary:	This member function loads saved window placement information from
	//			the registry.  This version is the same as LoadPlacement, but there
	//			is no need for calling SavePlacement when the window is destroyed,
	//			this will be called automatically.
	void AutoLoadPlacement(LPCTSTR pszSection);

protected:

	// Input:	pwnd - Points to the dialog item to be resized.
	//			rrcSizing - How much the left, top, right, and bottom sides will move when the
	//			dialog is resized.
	//			rcWindow - Initial size of the dialog item.
	// Summary:	This member function is used by the resize manager to add a dialog
	//			item to the list of items to be resized.
	void SetResize(CWnd* pwnd,const CXTResizeRect& rrcSizing,CRect rcWindow);

	// Input:	nID - Specifies the control's ID.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function is called to remove the specified dialog item
	//			from the list of items to be resized. 
	BOOL RemoveResize(const UINT nID);

	// Summary: This member function is called to purge the list that contains dialog
	//			items to be sized.
	void RemoveAllControls();

	// Summary: This member function is called from OnInitDialog or OnInitialUpdate
	//			to initialize the resize manager.
	void Init();

	// Summary: This member function is called from OnSize to move and resize the dialog
	//			items that are managed.
	void Size();

	// Summary: This member function is called, when a property sheet in wizard mode
	//			has changed pages, to alert the resize manager that the property sheet
	//			(common control) has moved the page back to its original size/position
	//			on the sheet.
	void Reset();

	// Input:	pMMI - Points to a MINMAXINFO structure that contains information about a
	//			window’s maximized size and position, and its minimum and maximum
	//			tracking size.  For more information about this structure, see the
	//			MINMAXINFO structure.
	// Summary:	This member function is called from OnGetMinMaxInfo to get the
	//			maximized position or dimensions, or the minimum or maximum tracking
	//			size.  The maximized size is the size of the window when its borders
	//			are fully extended.  The maximum tracking size of the window is the
	//			largest window size that can be achieved by using the borders to size
	//			the window.  The minimum tracking size of the window is the smallest
	//			window size that can be achieved by using the borders to size the window.
	void GetMinMaxInfo(MINMAXINFO* pMMI);

protected: // flags
	
	// Summary: Style attributes for the resizing dialog, property sheet, or form view.
	enum EFlags
	{
		SZ_NOSIZEICON			= 0x01, // Do not add size icon.
		SZ_NOHORISONTAL			= 0x02, // No horizontal resizing.
		SZ_NOVERTICAL			= 0x04, // No vertical resizing.
		SZ_NOMINSIZE			= 0x08, // Do not require a minimum size.
		SZ_NOCLIPCHILDREN		= 0x10, // Do not set clip children style.
		SZ_NOTRANSPARENTGROUP	= 0x20, // Do not set transparent style for group boxes.
	};

	// BULLETED LIST:

	// Input:	eFlag - Flag to check. It can be one of the following:
	//			[ul]
    //			[li]<b>SZ_NOSIZEICON</b> Do not add size icon.[/li]
    //			[li]<b>SZ_NOHORISONTAL</b> No horizontal resizing.[/li]
    //			[li]<b>SZ_NOVERTICAL</b> No vertical resizing.[/li]
    //			[li]<b>SZ_NOMINSIZE</b> Do not require a minimum size.[/li]
    //			[li]<b>SZ_NOCLIPCHILDREN</b> Do not set clip children style.[/li]
    //			[li]<b>SZ_NOTRANSPARENTGROUP</b> Do not set transparent style
	//			for group boxes.[/li]
	//			[/ul]
	// Returns: TRUE if the specified flag has been set, otherwise returns FALSE.
    // Summary:	This member function is called to determine if the specified flag
	//			has been set for the resize manager.  
	BOOL HasFlag(EFlags eFlag);
	
	// BULLETED LIST:

	// Input:	eFlag - Flag to set. It can be one of the following:
	//			[ul]
    //			[li]<b>SZ_NOSIZEICON</b> Do not add size icon.[/li]
    //			[li]<b>SZ_NOHORISONTAL</b> No horizontal resizing.[/li]
    //			[li]<b>SZ_NOVERTICAL</b> No vertical resizing.[/li]
    //			[li]<b>SZ_NOMINSIZE</b> Do not require a minimum size.[/li]
    //			[li]<b>SZ_NOCLIPCHILDREN</b> Do not set clip children style.[/li]
    //			[li]<b>SZ_NOTRANSPARENTGROUP</b> Do not set transparent style
	//			for group boxes.[/li]
	//			[/ul]
    // Summary:	This member function is called to set a specific flag for the resize
	//			manager.
	void SetFlag(EFlags eFlag);

protected: // helper methods

	BOOL Defer(HDWP&, CXTItem*, int dx, int dy);

protected: // helper data

	typedef CArray<CXTItem*, CXTItem*&> CXTItemArray;

	UINT			m_nFlagsXX;		// flags passed from constructor
	CWnd*			m_pwnd;			// the associative relation to the window to be resized
	CRect			m_rcWindow;		// last dialog size
	CRect			m_rcInitWindow; // Initial dialog size
	CSize			m_szMin;		// smallest size allowed
	CSize			m_szMax;		// largest size allowed
	CString			m_strSection;	// section in registry where window placement information is saved.
	CXTSizeIcon		m_scSizeIcon;	// size icon window
	CXTItemArray	m_arrItems;		// array of controls
	
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTResize::SetMinSize(CSize& sz) { 
	m_szMin = sz; 
}
AFX_INLINE void CXTResize::SetMaxSize(CSize& sz) { 
	m_szMax = sz; 
}
AFX_INLINE BOOL CXTResize::HasFlag(EFlags eFlag) {
	return (m_nFlagsXX & eFlag) != 0;
}
AFX_INLINE void CXTResize::SetResize(const UINT nID, const HWND hWnd, const XT_RESIZEPOINT& rpTopLeft, const XT_RESIZEPOINT& rpBottomRight) {
	SetResize(nID, hWnd, CXTResizeRect(rpTopLeft.x, rpTopLeft.y, rpBottomRight.x, rpBottomRight.y));
}
AFX_INLINE void CXTResize::SetResize(const UINT nID, const XT_RESIZERECT& rrcSizing) {
	SetResize(nID,NULL,rrcSizing);
}
AFX_INLINE void CXTResize::SetResize(const UINT nID, const XT_RESIZEPOINT& rpTopLeft, const XT_RESIZEPOINT& rpBottomRight) {
	SetResize(nID, CXTResizeRect(rpTopLeft.x, rpTopLeft.y, rpBottomRight.x, rpBottomRight.y));
}
AFX_INLINE void CXTResize::SetResize(const UINT nID, XT_RESIZE left, XT_RESIZE top, XT_RESIZE right, XT_RESIZE bottom) {
	SetResize(nID, CXTResizeRect(left, top, right, bottom));
}

//////////////////////////////////////////////////////////////////////

#endif // !defined(__XTRESIZE_H__)