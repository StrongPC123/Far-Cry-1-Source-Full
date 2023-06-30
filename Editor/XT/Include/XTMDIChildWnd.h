// XTMDIChildWnd.h interface for the CXTMDIChildWnd class.
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

#if !defined(__XTMDICHILDWND_H__)
#define __XTMDICHILDWND_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTMDIChildWnd is a multiple inheritance class derived from CMDIChildWnd
//			and CXTFrameImpl. CXTMDIChildWnd extends the standard CMDIChildWnd class
//			to allow CXTDockWindow and CXTToolBar docking, customization, and cool
//			menu support.
class _XT_EXT_CLASS CXTMDIChildWnd : public CMDIChildWnd, public CXTFrameImpl
{
	DECLARE_DYNCREATE(CXTMDIChildWnd)

public:

    // Summary: Constructs a CXTMDIChildWnd object.
	CXTMDIChildWnd();

	// Summary: Destroys a CXTMDIChildWnd object, handles cleanup and de-allocation.
    virtual ~CXTMDIChildWnd();

	// Input:	lpszProfileName - Name of a section in the initialization file or a key in the
	//			Windows registry where state information is stored.
    // Summary:	This member function is called by the frame window to restore the
	//			settings of the control bar.
    virtual void LoadBarState(LPCTSTR lpszProfileName);

	// Input:	lpszProfileName - Name of a section in the initialization file or a key in the
	//			Windows registry where state information is stored.
    // Summary:	This member function is called by the frame window to save the settings
	//			of the control bar.
    virtual void SaveBarState(LPCTSTR lpszProfileName) const;

	// BULLETED LIST:

	// Input:	dwDockStyle - Specifies whether the control bar supports docking and the sides
	//			of its parent window to which the control bar can be docked, if supported.
    //			The style can be one or more of the following: 
    //			[ul]
    //			[li]<b>CBRS_ALIGN_TOP</b> Allows docking at the top of the
	//			client area.[/li]
    //			[li]<b>CBRS_ALIGN_BOTTOM</b> Allows docking at the bottom of
	//			the client area.[/li]
    //			[li]<b>CBRS_ALIGN_LEFT</b> Allows docking on the left side
	//			of the client area.[/li]
    //			[li]<b>CBRS_ALIGN_RIGHT</b> Allows docking on the right side
	//			of the client area.[/li]
    //			[li]<b>CBRS_ALIGN_ANY</b> Allows docking on any side of the
	//			client area.[/li]
    //			[li]<b>CBRS_FLOAT_MULTI</b> Allows multiple control bars to
	//			be floated in a single mini-frame window.[/li]
    //			[/ul]
    //			If 0 (that is, indicating no flags), the control bar will not
	//			dock.
    // Summary:	Call this function to enable a control bar to be docked.  The sides
	//			specified must match one of the sides enabled for docking in the destination
	//			frame window, or the control bar cannot be docked to that frame window.
    void EnableDocking(DWORD dwDockStyle);

	// BULLETED LIST:

	// Input:	dwDockStyle - Specifies whether the control bar supports docking and the sides
	//			of its parent window to which the control bar can be docked, if supported.
    //			The style can be one or more of the following: 
    //			[ul]
    //			[li]<b>CBRS_ALIGN_TOP</b> Allows docking at the top of the
	//			client area.[/li]
    //			[li]<b>CBRS_ALIGN_BOTTOM</b> Allows docking at the bottom of
	//			the client area.[/li]
    //			[li]<b>CBRS_ALIGN_LEFT</b> Allows docking on the left side
	//			of the client area.[/li]
    //			[li]<b>CBRS_ALIGN_RIGHT</b> Allows docking on the right side
	//			of the client area.[/li]
    //			[li]<b>CBRS_ALIGN_ANY</b> Allows docking on any side of the
	//			client area.[/li]
    //			[li]<b>CBRS_FLOAT_MULTI</b> Allows multiple control bars to
	//			be floated in a single mini-frame window.[/li]
    //			[/ul]
    //			If 0 (that is, indicating no flags), the control bar will not
	//			dock.
	//			dwFlatStyle - Specifies the look of the splitters that are inside dockbars.
    //			It can be one of the following: 
    //			[ul]
    //			[li]<b>CBRS_XT_NONFLAT</b> Thick devstudio like non-flat splitters.[/li]
    //			[li]<b>CBRS_XT_SEMIFLAT</b> Thin 3D non-flat splitters.[/li]
    //			[li]<b>CBRS_XT_FLAT</b> Flat splitters.[/li]
    //			[/ul]
    // Summary:	Call this function to enable a control bar to be docked. The sides
	//			specified must match one of the sides enabled for docking in the destination
	//			frame window, or the control bar cannot be docked to that frame window.
    void EnableDockingEx(DWORD dwDockStyle,DWORD dwFlatStyle);

	// Input:	pBar - Points to the control bar to be docked.
	//			pDockBar - Points to the dockbar the control bar is docked to.
	//			lpRect - Determines, in screen coordinates, where the control bar will
	//			be docked in the non-client area of the destination frame window.
    // Summary:	This member function causes a control bar to be docked to the frame
	//			window.  The control bar will be docked to one of the sides of the
	//			frame window specified in the calls to both CXTDockWindow::EnableDocking
	//			and CXTFrameWnd::EnableDocking.  The side chosen is determined by the
	//			dockbar specified by 'pDockBar'.
    void DockControlBar(CControlBar* pBar,CDockBar* pDockBar,LPCRECT lpRect = NULL);

	// BULLETED LIST:

	// Input:	pBar - Points to the control bar to be docked.
	//			nDockBarID - Determines which sides of the frame window to consider for docking.
	//			It can be 0, or one or more of the following: 
    //			[ul]
    //			[li]<b>AFX_IDW_DOCKBAR_TOP</b> Dock to the top side of the
	//			frame window.[/li]
    //			[li]<b>AFX_IDW_DOCKBAR_BOTTOM</b> Dock to the bottom side of
	//			the frame window.[/li]
    //			[li]<b>AFX_IDW_DOCKBAR_LEFT</b> Dock to the left side of the
	//			frame window.[/li]
    //			[li]<b>AFX_IDW_DOCKBAR_RIGHT</b> Dock to the right side of
	//			the frame window.[/li]
    //			[/ul]
    //			If 0, the control bar can be docked to any side enabled for
	//			docking in the destination frame window.
	//			lpRect - Determines, in screen coordinates, where the control bar will
	//			be docked in the non-client area of the destination frame window.
    // Summary:	This member function causes a control bar to be docked to the frame
	//			window.  The control bar will be docked to one of the sides of the
	//			frame window specified in the calls to both CXTDockWindow::EnableDocking
	//			and CXTFrameWnd::EnableDocking.  The side chosen is determined by 
    //			'nDockBarID'.
    void DockControlBar(CControlBar* pBar,UINT nDockBarID = 0,LPCRECT lpRect = NULL);

	// Input:	pBar1 - A CControlBar pointer to the control bar to be docked.
	//			pBar2 - A CControlBar pointer to the already docked control bar to be
	//			redocked on the left of 'pBar1'.
    // Summary:	This member function will redock a control bar specified by 'pBar2'
	//			to the left of a newly docked control bar specified by 'pBar1'. 
    virtual void DockControlBarLeftOf(CControlBar* pBar1,CControlBar* pBar2);

    // Summary: Call this function to display the Customize Toolbar dialog box.
	//			This dialog box allows the user to customize the toolbar by adding
	//			and deleting buttons.
	void Customize();

protected:

	CXTCoolMenu m_hookCoolMenus;  // Cool menu hook.

    // Ignore:
	//{{AFX_VIRTUAL(CXTMDIChildWnd)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs, UINT uIcon);
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	//}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTMDIChildWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	afx_msg void OnCustomizeBar();
	afx_msg void OnUpdateChevron(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()

	friend class CXTMDIFrameWnd;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTMDIChildWnd::Customize() {
	OnCustomizeBar();
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTMDICHILDWND_H__)