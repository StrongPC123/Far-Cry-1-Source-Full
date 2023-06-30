// XTMDIWndTab.h : header file
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

#if !defined(__XTMDIWNDTAB_H__)
#define __XTMDIWNDTAB_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forwards

class CXTMDIWndTab;

//////////////////////////////////////////////////////////////////////
// Summary: CXTMDIClientWnd is a CWnd derived helper class for the CXTMDIWndTab tab
//			control.  This class routes messages sent to the client window back
//			to the tab control.
class _XT_EXT_CLASS CXTMDIClientWnd : public CWnd
{
public:

	// Input:	pMDIWndTab - Pointer to a CXTMDITabWnd object.
    // Summary:	Constructs a CXTMDIClientWnd object.
	CXTMDIClientWnd(CXTMDIWndTab* pMDIWndTab);

	// Summary: Destroys a CXTMDIClientWnd object, handles cleanup and de-allocation.
    virtual ~CXTMDIClientWnd();

protected:

	int				m_iBorderGap; // Amount, in pixels, between the client and the tab control.
	CXTMDIWndTab*	m_pMDIWndTab; // Pointer to the MDI tab control.

public:

	// Input:	iSize - Amount, in pixels, of gap between the tab control and the client.
	// Summary:	This member function will set the size of the gap between the client
	//			area and the tab control.
	void SetBorderGap(int iSize);

    // Ignore:
	//{{AFX_VIRTUAL(CXTMDIClientWnd)
	protected:
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	//}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTMDIClientWnd)
	//}}AFX_MSG

	afx_msg LRESULT OnMDICreate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMDIDestroy(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMDIActivate(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTMDIClientWnd::SetBorderGap(int iBorderGap) {
	m_iBorderGap = iBorderGap;
}

//////////////////////////////////////////////////////////////////////
// Summary: XT_MDICHILD is a stand alone structure class.  It is used to create
//			an XT_MDICHILD structure.
struct XT_MDICHILD
{
	int		iItem;		// Index of the item in the tab control.
	HWND	hWnd;		// Window handle of the tab item.
	CString strItem;	// Tab label.
};

// Summary: Array of XT_MDICHILD structs that represent each item in the tab control.
typedef CList<XT_MDICHILD*,XT_MDICHILD*> CMDIChildArray;

// Summary: CMap definition for mapping icons with window handles.
typedef CMap<HWND, HWND, HICON, HICON&>	CXTIconWndMap;

// Summary: CMap definition for mapping strings to window handles.
typedef CMap<HWND, HWND, CString, CString&> CXTStringMap;

//////////////////////////////////////////////////////////////////////
// Summary: CXTMDIWndTab is a multiple inheritance class derived from CTabCtrl and
//			CXTTabCtrlBase. CXTMDIWndTab is used to create a tab control for a multiple
//			document interface (MDI) application. 
class _XT_EXT_CLASS CXTMDIWndTab : public CTabCtrl, public CXTTabCtrlBase
{
	DECLARE_DYNAMIC(CXTMDIWndTab)

public:

    // Summary: Constructs a CXTMDIWndTab object.
	CXTMDIWndTab();

    // Summary: Destroys a CXTMDIWndTab object, handles cleanup and de-allocation.
	virtual ~CXTMDIWndTab();

protected:

	int					m_iBorderGap;			// Amount, in pixels, between the client and the tab control.
	int					m_iHitTest;				// Index of the tab that received a right click.
	int					m_nPos;					// Index of the popup menu contained in the menu.
	UINT				m_nDefCmd;				// Command ID of the default menu item for the popup menu.
	UINT				m_popupMenuID;			// Popup menu resource ID.
	HWND				m_hActiveChild;			// Active MDI child.
	BOOL				m_bNoIcons;				// TRUE if no icons are used.
	BOOL				m_bRecalcLayoutPending; // Tells if a request to recalc the parent frame is pending.
	DWORD				m_dwInitSignature;		// Initialization state signature to synch up posted init requests.
	CImageList			m_imageList;			// Tab image list.
	CMDIFrameWnd*		m_pMDIFrameWnd;			// Points to the owner frame.
	CMDIChildArray		m_arMDIChildern;		// Array of the MDI windows added to the tab control.
	CXTMDIClientWnd*	m_pMDIClientWnd;		// Window that receives messages on behalf of the MDI client.
	CXTIconWndMap	    m_mapTabIcons;			// Hash table that maps icons to the MDI child frame windows.
	CXTStringMap        m_mapTabLabels;			// Hash table that maps labels to the MDI child frame windows.

public:

	// Input:	iIndex - Zero-based index of the tab to set the tab icon for.
	//			hIcon - Reference to an HICON handle that represents the tab icon.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to set the icon for the tab item specified
	//			by 'iIndex'. 
	bool SetTabIcon(int iIndex,HICON& hIcon);

	// Input:	hChildWnd - Valid HWND of the MDI child window to set the tab icon for.
	//			hIcon - Reference to an HICON handle that represents the tab icon.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to set the icon for the tab item specified
	//			by 'hChildWnd'. 
	bool SetTabIcon(HWND hChildWnd,HICON& hIcon);

	// Input:	iIndex - Zero-based index of the tab to set the tab text for.
	//			strLabel - Reference to a CString object that represents the tab label.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to set the label for the tab item specified
	//			by 'iIndex'. 
	bool SetTabLabel(int iIndex,CString& strLabel);
	
	// Input:	hChildWnd - Valid HWND of the MDI child window to set the tab text for.
	//			strLabel - Reference to a CString object that represents the tab label.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to set the label for the tab item specified
	//			by 'hChildWnd'. 
	bool SetTabLabel(HWND hChildWnd,CString& strLabel);

	// Input:	pMDIFrameWnd - Points to the parent MDI frame window.
	//			dwStyle - Style for the tab control. It can be any of the TCS_ values.
	//			bNoIcons - Set to TRUE for no icon to display
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will install the MDI tab views with your application.
	virtual BOOL Install(CMDIFrameWnd* pMDIFrameWnd,DWORD dwStyle=TCS_BOTTOM|TCS_HOTTRACK,BOOL bNoIcons=xtAfxData.bXPMode);

	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function will uninstall the MDI tab views from your application.
	virtual BOOL UnInstall();

	// Input:	point - Pointer to a CPoint object that contains the cursor screen 
	//			coordinates.  Use default for the current cursor position.
	// Returns: The zero-based index of the tab, or -1 if no tab is at the specified point.
	// Summary:	Call this member function to retrieve the tab index from the current
	//			cursor position. 
	int TabFromPoint(CPoint point) const;

	// Input:	iIndex - Index of the tab control.  If -1, then the tab under the mouse when
	//			the last right click was performed will be used.
	// Returns: A CWnd object if successful, otherwise returns NULL.
	// Summary:	This member function will return a pointer to the child frame associated
	//			with the tab item. 
	CMDIChildWnd* GetFrameWnd(int iIndex=-1) const;

	// Input:	point - CPoint object that represents the cursor position.
	// Returns: A CWnd object if successful, otherwise returns NULL.
	// Summary:	This member function will return a pointer to the child frame associated
	//			with the tab item. 
	CMDIChildWnd* GetFrameWnd(CPoint point) const;

	// Input:	popupMenuID - ID for the tab control popup menu.
	//			nPos - Index position in the menu resource.
	//			nDefCmd - ID of the default menu command, will display bold.
	// Summary:	This member function is used to set the resource ID for the popup menu
	//			used by the tab control.
	void SetMenuID(UINT popupMenuID,int nPos=0,UINT nDefCmd=-1);

	// Input:	iSize - Amount, in pixels, of gap between the tab control and the client.
	// Summary:	This member function will set the size of the gap between the client
	//			area and the tab control.
	void SetBorderGap(int iSize);

	// Input:	hWnd - Handle to a valid CMDIChildWnd object.
	// Returns: A const CString object that represents the document title.
	// Summary:	This member function will get the text for the specified MDI child
	//			window.
	CString GetChildWndText(HWND hWnd) const;

protected:

	// Summary: This member function is called to initialize the font for the tab control
	//			associated with this view.
	virtual void InitializeFont();

	// Input:	pChildFrame - A pointer to a valid child frame window.
	//			bRecalcLayout - TRUE to force the MDI frame window to recalc the layout.
	// Summary:	This member function is called to insert a child frame into the MDI
	//			tab control.
	virtual void InsertTabWnd(CMDIChildWnd* pChildFrame,BOOL bRecalcLayout=TRUE);

	// Returns: FALSE if there is no active document, otherwise returns TRUE.
	// Summary:	This member function is called by the tab control to ensure the current
	//			selection matches the active document.  
	BOOL RefreshActiveSel();

	// Summary: This member function is called by the tab control to ensure the tab
	//			labels match their corresponding views.
	void RefreshTabLabels();

	// Summary: This member function is called by the tab control to ensure the stored
	//			indexes match their corresponding tab indexes.
	void RefreshIndexes();

    // Ignore:
	//{{AFX_VIRTUAL(CXTMDIWndTab)
	//}}AFX_VIRTUAL

	virtual void OnMDICreate(HWND hWnd);
	virtual void OnMDIDestroy(HWND hWnd);
	virtual void OnMDIActivate(HWND hWnd);

    // Ignore:
	//{{AFX_MSG(CXTMDIWndTab)
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	afx_msg void OnIdleUpdateCmdUI();
	afx_msg LRESULT OnXtUpdate(WPARAM, LPARAM);
	afx_msg void OnPaint();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTabClose();

	DECLARE_MESSAGE_MAP()

	friend class CXTMDIClientWnd;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTMDIWndTab::SetMenuID(UINT popupMenuID, int nPos, UINT nDefCmd) {
	m_popupMenuID = popupMenuID; m_nPos = nPos; m_nDefCmd = nDefCmd;
}
AFX_INLINE void CXTMDIWndTab::SetBorderGap(int iBorderGap) {
	m_iBorderGap = iBorderGap; m_pMDIClientWnd->SetBorderGap(iBorderGap + ::GetSystemMetrics(SM_CXSIZEFRAME));
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTMDIWNDTAB_H__)