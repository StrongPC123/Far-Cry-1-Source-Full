// XTFrameWnd.h interface for the CXTFrameWnd class.
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

#if !defined(__XTFRAMEWND_H__)
#define __XTFRAMEWND_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTFrameWnd is a multiple inheritance class derived from CFrameWnd and
//			CXTFrameImpl. CXTFrameWnd extends the standard CFrameWnd class to allow
//			CXTDockWindow and CXTToolBar docking, customization, and cool menu support.
class _XT_EXT_CLASS CXTFrameWnd : public CFrameWnd, public CXTFrameImpl
{
    DECLARE_DYNCREATE(CXTFrameWnd)

public:

    // Summary: Constructs a CXTFrameWnd object.
    CXTFrameWnd();

    // Summary: Destroys a CXTFrameWnd object, handles cleanup and de-allocation.
    virtual ~CXTFrameWnd();

protected:

public:

	// Input:	pViewClass - A pointer to a CRuntimeClass structure associated with your view.
	//			pDocument - A pointer to the document to be used with the view. If NULL
	//			the same document will be used for all SDI views.
	//			pContext - Specifies the type of view and document, if NULL this will be 
    //			created for you.
    // Summary:	Call this member function to switch views for your SDI application.
    virtual void SwitchSDIView(CRuntimeClass *pViewClass,CDocument *pDocument=NULL,CCreateContext *pContext=NULL);

	// Returns: A CXTCoolMenu pointer that represents the cool menu object associated with 
	//			the frame.
    // Summary:	Call this member function to retrieve a pointer to the cool menu
	//			object associated with the frame.  
    CXTCoolMenu* GetCoolMenu();

	// Returns: A CXTMenuBar pointer that represents the menu bar associated with the frame.
    // Summary:	Call this member function to retrieve a pointer to the menu bar associated
	//			with the frame.  
    CXTMenuBar* GetMenuBar();

	// Input:	nIDToolBars - Array of toolbar resource IDs.  The cool menu will use the toolbar
	//			commands to map the icons placed next to the corresponding menu commands.
	//			nSize - Size of the array of toolbars.
    // Summary:	Call this member function to install cool menus for your application.
	//			Cool menus are menus that appear with icons next to the menu titles.
	//			Pass in your toolbar resource array to initialize.
    void InstallCoolMenus(const UINT* nIDToolBars,int nSize);

	// Input:	nIDToolBar - Toolbar resource ID.  The cool menu will use the toolbar commands
    //			to map the icons placed next to the corresponding menu commands.
    // Summary:	Call this member function to install cool menus for your application.
	//			Cool menus are menus that appear with icons next to the menu titles.
	//			Pass in your toolbar resource to initialize.
    void InstallCoolMenus(const UINT nIDToolBar);

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
    //			If 0 (that is, indicating no flags), the control bar will not dock.
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
	//			dwFlatStyle - Specifies the splitter's, inside dockbars, look. It can be one
	//			of the following: 
    //			[ul]
    //			[li]<b>CBRS_XT_NONFLAT</b> Thick devstudio like non-flat splitters.[/li]
    //			[li]<b>CBRS_XT_SEMIFLAT</b> Thin 3D non-flat splitters.[/li]
    //			[li]<b>CBRS_XT_FLAT</b> Flat splitters.[/li]
    //			[/ul]
    // Summary:	Call this function to enable a control bar to be docked.  The sides
	//			specified must match one of the sides enabled for docking in the destination
	//			frame window, or the control bar cannot be docked to that frame window.
    void EnableDockingEx(DWORD dwDockStyle,DWORD dwFlatStyle);

	// Input:	pBar - A CControlBar pointer to the control bar to be docked.
	//			pDockBar - A CDockBar pointer to the dockbar the control bar is docked to.
	//			lpRect - Determines, in screen coordinates, where the control bar will
	//			be docked in the non-client area of the destination frame window.
    // Summary:	This member function causes a control bar to be docked to the frame
	//			window. The control bar will be docked to one of the sides of the frame
	//			window specified in the calls to both CXTDockWindow::EnableDocking
	//			and CXTFrameWnd::EnableDocking. The side chosen is determined by the
	//			dockbar represented by 'pDockBar'.
    void DockControlBar(CControlBar* pBar,CDockBar* pDockBar,LPCRECT lpRect = NULL);

	// BULLETED LIST:

	// Input:	pBar - A CControlBar pointer to the control bar to be docked.
	//			nDockBarID - Determines which sides of the frame window to consider for docking.
	//			It can be 0, or one or more of the following: 
    //			[ul]
    //			[li]<b>AFX_IDW_DOCKBAR_TOP</b> to the top side of the frame
	//			window.[/li]
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
	//			and CXTFrameWnd::EnableDocking.  The side chosen is determined by nDockBarID.
    void DockControlBar(CControlBar* pBar,UINT nDockBarID = 0,LPCRECT lpRect = NULL);

	// Input:	pBar1 - A CControlBar pointer to the control bar to be docked.
	//			pBar2 - A CControlBar pointer to the already docked control bar to be
	//			redocked on the left of 'pBar1'.
    // Summary:	This member function will redock a control bar specified by 'pBar2'
	//			to the left of a newly docked control bar specified by 'pBar1'. 
    virtual void DockControlBarLeftOf(CControlBar* pBar1,CControlBar* pBar2);

	// Returns: A pointer to a CMenu object that represents the active menu for the frame.
    // Summary:	This member function retrieves a pointer to the menu for the frame
	//			window. 
    virtual CMenu* GetMenu() const;

	// Input:	nCommandID - The command ID of a menu item to hide.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function adds the specified command to the list of menu
	//			items to hide until activated by clicking on the chevron. 
    virtual BOOL HideMenuItem(UINT nCommandID);

	// Input:	lpszItem - Text string representing the popup menu item to hide.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function adds the specified menu item to the list of
	//			menu items to hide until activated by clicking on the chevron. 
    virtual BOOL HideMenuItem(LPCTSTR lpszItem);

	// Input:	nCommandIDs - An array of command IDs, of menu items, to hide.
	//			nSize - Size of the array passed in.
	// Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary:	This member function adds the specified commands to the list of
	//			menu items to hide until activated by clicking on the chevron. 
    virtual BOOL HideMenuItems(const UINT* nCommandIDs,int nSize);

	// Input:	lpszItems - An array of command IDs, of menu items, to hide.
	//			nSize - Size of the array passed in.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function adds the specified menu items to the list of
	//			menu items to hide until activated by clicking on the chevron. 
    virtual BOOL HideMenuItems(const LPCTSTR lpszItems,int nSize);

    // Summary: This member function redraws the menu bar. If a menu bar is changed
	//			after Windows has created the window, call this function to draw the
	//			changed menu bar.  Overrides the CWnd implementation.
    void DrawMenuBarX();

    // Summary: Call this function to display the Customize Toolbar dialog box.
	//			This dialog box allows the user to customize the toolbar by adding
	//			and deleting buttons.
	void Customize();

	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function will initialize the accelerator manager for the
	//			framework. 
	bool InitAccelManager();
	
	// Input:	iNormalIndex - Index where the 'Tools' menu should be inserted into the standard menu.
	//			iWindowIndex - Index where the 'Tools' menu should be inserted into the MDI window menu.
	//			iArgPopupMenu - Resource ID of the popup menu to be displayed for the Arguments
	//			browse edit box.
	//			iDirPopupMenu - Resource ID of the popup menu to be displayed for the Initial
	//			Directory browse edit box.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to initialize the Tools manager for your
	//			application. The tools manager will insert a "Tools" menu into your
	//			application's menu which allows the user to customize and add custom
	//			commands to the menu.  Typically used with toolbar customization, the
	//			Tools manager should be initialized when your frame is loaded by overriding
	//			the virtual function CFrameWnd::LoadFrame and can be managed by selecting
	//			the 'Tools' tab in the customize dialog.  
	bool InitToolsManager(int iNormalIndex,int iWindowIndex=-1,int iArgPopupMenu=0,int iDirPopupMenu=0);

	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to initialize the Options manager for your
	//			application. Typically used with toolbar customization, the Options
	//			manager will allow the user to configure options related to the toolbar
	//			and menu behavior for the application.  The Options manager should
	//			be initialized when your frame is loaded by overriding the virtual
	//			function CFrameWnd::LoadFrame and can be managed by selecting the 'Options'
	//			tab in the Customize dialog.  
	bool InitOptionsManager();

protected:

    virtual void SetDockState(const CXTDockState& state);
    virtual void GetDockState(CXTDockState& state) const;

    // Ignore:
	//{{AFX_VIRTUAL(CXTFrameWnd)
	public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs, UINT uIcon);
	//}}AFX_VIRTUAL

    // Ignore:
	//{{AFX_MSG(CXTFrameWnd)
    afx_msg void OnSysColorChange();
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	//}}AFX_MSG
    
	afx_msg void OnCustomizeBar();
	afx_msg BOOL OnToolsManager(UINT nID);
	afx_msg void OnUpdateToolsManager(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChevron(CCmdUI* pCmdUI);
    
    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTFrameWnd::Customize() {
	OnCustomizeBar();
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTFRAMEWND_H__)
