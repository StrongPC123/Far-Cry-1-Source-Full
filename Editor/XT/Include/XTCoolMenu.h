// XTCoolMenu.h : interface for the CXTCoolMenu class.
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

#if !defined(__XTCOOLMENU_H__)
#define __XTCOOLMENU_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTItemData is a stand alone helper class.  It is used by CXTCoolMenu.
class _XT_EXT_CLASS CXTItemData
{
public:

    // Summary: Constructs a CXTItemData object.
    CXTItemData ();

    // Summary: Destroys a CXTItemData object, handles cleanup and de-allocation.
    virtual ~CXTItemData();

	long	m_itemID;		// Unique identifying number.
	UINT	m_nPosition;	// Menu item position.
	UINT	m_commandID;	// Menu item (command) ID.
	BOOL	m_bHidden;		// TRUE if the item has been removed.
	HMENU	m_hMenu;		// Handle to the menu this item belongs to.
	HMENU	m_hSubMenu;		// Handle to a submenu, if any, that belongs to this item.
	DWORD	m_dwType;		// Original item type flags.
	DWORD	m_dwState;		// Item state.
	DWORD	m_dwHideType;	// Used for drawing hidden items.
	DWORD	m_iMBAlign;		// Alignment of the associated menu bar.
	CString	m_strText;		// Item text.

	// Returns: TRUE if the cool menu belongs to to the cool menu object.
	// Summary: This member function is called to determine if the item data for the
	//			cool menu belongs to to the cool menu object. 
    BOOL IsXTItemData();
};

//////////////////////////////////////////////////////////////////////

class CXTMemDC;
class CXTCoolMenu;
class CXTWndShadow;

class CXTPopupMenu : public CXTWndHook
{
protected:

	bool			m_bOffset;
	BOOL			m_bAnimationFinished;
	HMENU			m_hMenu;
	CRect			m_rectExclude;
	static int      m_iRefCount;
	static BOOL     m_bSysMenuAnimation;
	static BOOL     m_bSysDropShadows;
	static BOOL     m_bSysDropShadowsAvailable;
	CXTCoolMenu*	m_pParent;

	enum { WM_XT_FIX_XP_PAINT = WM_APP + 10, WM_XT_DEFER_DRAW_SHADOW };

public:

	CXTPopupMenu();
	virtual ~CXTPopupMenu();

	HWND GetHWnd();
	HMENU GetHMenu();

	void SetParams(HMENU hMenu);
	void SetCoolMenuParent(CXTCoolMenu *pParent);
	void HideBeforeExpand();
	void DisableXPEffects();
	void OnDrawBorder(CDC* pDC);
	void ShowShadows();

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	friend class CXTCoolMenu;
};

// Summary: CList definition for the popup menu array.
typedef CList<CXTPopupMenu*, CXTPopupMenu*&> CXTPopupMenuList;

// Summary: CList definition for the CMenu array.
typedef	CList<CMenu*,CMenu*> CXTMenuList;

// Summary: CList definition for the CXTItemData array.
typedef CList<CXTItemData*,CXTItemData*> CXTItemList;

// NUMBERED LIST:

//////////////////////////////////////////////////////////////////////
// Summary: CXTCoolMenu is a CXTWndHook derived class.  CXTCoolMenu manages the
//			cool menus for the application.  The images are drawn based upon toolbar
//			resources that are passed in and are displayed next to the menu text
//			associated with the toolbar command ID.
//
//			Some restrictions apply to cool menus if the XT menu bar (CXTMenuBar) is
//			not used:
//			[ol]
//			[li]Intelligent menus cannot be used.[/li]
//			[li]XP skinning and shadows can only be used on Win2k or later operating
//			   systems.[/li]
//			[/ol]
class _XT_EXT_CLASS CXTCoolMenu : public CXTWndHook
{
    friend class CXTColorPopup;
    friend class CXTMenu;

    DECLARE_DYNAMIC(CXTCoolMenu)

public:

	// Input:	bNoToolbar - DEPRECATED: The value of this parameter is ignored.
    // Summary: Constructs a CXTCoolMenu object.
    CXTCoolMenu(BOOL bNoToolbar=FALSE);

    // Summary: Destroys a CXTCoolMenu object, handles cleanup and de-allocation.
    virtual ~CXTCoolMenu();

protected:

    int             m_nRecentList;     // Number of items the recent item list cycles through.
    int             m_nIDEvent;        // Menu timer ID.
    int             m_nTimeOut;        // Time-out value, in milliseconds.
    BOOL            m_bIntelligent;    // TRUE if intelligent menus are enabled.
    bool            m_bTimerActive;    // true when the timer has been activated.
    BOOL            m_bHasMnemonics;   // (Win2K) TRUE if 'use menu underlines' is turned off in the control panel.
    BOOL            m_bIsTrackPopup;   // TRUE if the current menu was activated through TrackPopupMenu.
    CSize           m_sizeBmp;         // Size of the button bitmap.
    CSize           m_sizeBtn;         // Size of the button, including shadow.
    static int      m_iIgnoreCount;    // Incremented when user calls IgnoreNextPopup();.
    CUIntArray      m_arCommandID;     // Array of command IDs to hide.
    static bool     m_bLastMouseEvent; // true if last message was WM_MOUSEMOVE and not WM_KEYDOWN
    static CSize    m_szPopupBorder;   // Size of the border of popup menus.
    static CSize    m_szIconMargin;    // Size of the icon margins. 
	CXTMenuList     m_menuList;        // List of the CMenu objects initialized.
	CXTItemList     m_itemList;        // List of the item data pointers.

public:

    static int  m_nAnimationDelay;   // The number of milliseconds for animation effect. Default value is 300.
    static int  m_nAnimationSteps;   // The number of steps used in the animation effect. Default value is 5.
    static bool m_bShowAll;          // true when the cool menu displays hidden commands.
    static bool m_bAllowIntelligent; // Set to true when the popup is generated through CXTMenu which implements part of the intelligent menu's logic.
    static bool m_bIsPopup;          // true if the menu is not owned by CXTMenuBar.

	// BULLETED LIST:

    // Summary:  Type of animation to perform. 
    //           This parameter can be one of the following values:
	//			 [ul]
    //           [li]<b>animateWindowsDefault</b> Animation as defined in the "Display" settings.[/li]
    //           [li]<b>animateRandom</b> Any of the first three in random selection.[/li]
    //           [li]<b>animateUnfold</b>  Unfold top to bottom.[/li]
    //           [li]<b>animateSlide</b> Slide in from left.[/li]
    //           [li]<b>animateFade</b> Fade-in.[/li]
    //           [li]<b>animateNone</b> No animation.[/li]
	//			 [/ul]
    //           The default value is animateWindowsDefault. 
    //           You can add a new animation effect, see the CXTAnimationMemDC description.
	// See Also: CXTAnimationMemDC
	static int m_nAnimationType;
    
    static int   m_iMenuBarAlign; // Used by menu bar for linking menu bar button to menu (XP style)
    static CRect m_rectExclude;
    
    // Input:	nID - The command ID to get the index for.
	//			bEnabled - true if the command ID is from the enabled image list.
    // Returns: The index of the specified command, or -1 if the command
	//			is not found.
    // Summary: This member function returns the index for the specified command.
    int GetButtonIndex(WORD nID, bool bEnabled=true);

    // Input:	nElapse - Specifies the time-out value, in milliseconds.
	//			nIDEvent - Specifies a nonzero timer identifier.
    // Summary: This member function will set the timer identifier and time-out value
    //			for the cool menu. The timer is activated when the mouse hovers over a
    //			chevron when using intelligent menus. NOTE: if you are not using
    //			the CXTMenuBar with your application you should set this to a
    //			value of 100, otherwise you will need to also call CXTMenuBar::SetTimerInfo()
    //			if you are changing the 'nIDEvent' parameter.
    void SetTimerInfo(UINT nElapse, UINT nIDEvent=1000);

	// Input:	bIntelligent - TRUE to enable intelligent menus. FALSE to disable intelligent menus.
    // Summary: This member function will enable or disable the intelligent menu feature.
    void SetIntelligentMode(BOOL bIntelligent=TRUE);

	// Input:	pFrame - Pointer to a CFrameWnd object that represents the application's main
	//			window. If NULL, this will uninstall the cool menus.
    // Summary: This member function is called to install cool menus for the application.
    virtual void Install(CFrameWnd* pFrame);

	// Input:	nToolbarID - The resource ID of a toolbar to extract images
    //			from, to be displayed in cool menus.
	//			bEnabled - true if the toolbar is for enabled images.
    // Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary: This member function will load a toolbar resource and extract
    //			each button image to be used with its associated menu command.
    virtual BOOL LoadToolbar(UINT nToolbarID, bool bEnabled=true);

    // Input:	toolbar - Reference to a CToolBarCtrl object.
	//			bEnabled - true if the toolbar is for enabled images.
    // Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary: This member function will load a toolbar resource and extract
    //			each button image to be used with its associated menu command.
    virtual BOOL LoadToolbar(CToolBarCtrl& toolbar, bool bEnabled=true);

    // Input:	nToolbarIDs - An array of toolbar resource IDs used to extract 
    //			images from, to be displayed in cool menus.
	//			nSize - Size of the array passed in.
	//			bEnabled - true if the toolbar array is for enabled images.
    // Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary: This member function will load a toolbar resource and extract
    //			each button image to be used with its associated menu command.
    virtual BOOL LoadToolbars(const UINT* nToolbarIDs, int nSize, bool bEnabled=true);

    // Input:	nCommandID - The command ID of a menu item to hide.
    // Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary: This member function adds the specified command to the list of
    //			menu items to hide until activated by clicking on the chevron.
    virtual BOOL HideCommand(UINT nCommandID);

    // Input:	lpszItem - Text string representing the popup menu item to hide.
    // Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary: This member function adds the specified menu item to the list of
    //			menu items to hide until activated by clicking on the chevron.
    virtual BOOL HideCommand(LPCTSTR lpszItem);

    // Input:	nCommandIDs - An array of command IDs, of menu items, to hide.
	//			nSize - Size of the array passed in.
    // Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary: This member function adds the specified commands to the list of
    //			menu items to hide until activated by clicking on the chevron.
    virtual BOOL HideCommands(const UINT* nCommandIDs, int nSize);

    // Input:	lpszItems - An array of command IDs, of menu items, to hide.
	//			nSize - Size of the array passed in.
    // Returns: TRUE if successful, otherwise returns FALSE.        
    // Summary: This member function adds the specified menu items to the list of
    //			menu items to hide until activated by clicking on the chevron.
    virtual BOOL HideCommands(const LPCTSTR lpszItems, int nSize);

    // Input:	hIcon - Handle to the icon to add.
	//			uCmdID - Command id of the icon.
	//			bEnabled - true if the image is to be added to the enabled image list.
    // Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: This member function will append the specified image resource (hIcon)
    //			to the existing cool menu image list, and map the associated
    //			command ID (uCmdID) to the newly added image.
	virtual BOOL AppendImageList(HICON hIcon, UINT uCmdID, bool bEnabled/*=true*/);

    // Input:	nBitmapID - Resource ID of the new bitmap to add to the cool
    //			menu's image list.
	//			arCmdIDs - Array of menu command(s) to be associated with the 
    //			newly added image.
	//			nSize - Size of the array passed in.
	//			bEnabled - true if the image is to be added to the enabled image list.
    // Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: This member function will append the specified image resource (bitmap)
    //			to the existing cool menu image list, and map the associated
    //			command IDs to the newly added images.
    virtual BOOL AppendImageList(UINT nBitmapID, UINT* arCmdIDs, int nSize, bool bEnabled=true);

	// Input:	hBitmap - Handle of the new bitmap to add to the cool menu's image list.
	//			arCmdIDs - Array of menu command(s) to be associated with the 
    //			newly added image.
	//			nSize - Size of the array passed in.
	//			bEnabled - true if the image is to be added to the enabled image list.
    // Returns: TRUE if successful, otherwise returns FALSE.
    // Summary: This member function will append the specified image resource (bitmap)
    //			to the existing cool menu image list, and map the associated
    //			command IDs to the newly added images.
    virtual BOOL AppendImageList(HBITMAP hBitmap, UINT* arCmdIDs, int nSize, bool bEnabled=true);

    // Input:	pMenu - Points to the active menu.
	//			nCommandID - Command ID of a menu item.
    // Returns: The position of the menu item if it exists, otherwise returns -1.
    // Summary: Call this member function to check for the existence of a menu item.
    virtual int MenuItemExists(CMenu* pMenu, UINT nCommandID);

	// Input:	nLength - Number of items the recent item list contains.  Set to 0 to
    //			disable this feature.
    // Summary: This member function sets the number of hidden items that can be
    //			added to the short menu at any given time.
    virtual void SetRecentItemLength(int nLength);

    // Summary: This member function is called to reset the recent item list used to
	//			add recently used hidden items to the short menu.
    virtual void ResetUsageData();

    // Summary: Call this member function if the next popup menu is not to be 
    //			transformed by the CXTCoolMenu class.  For instance, use this before
    //			displaying system menus.
    static void IgnoreNextPopup();

	// Input:	size - Represents the new size of the cool menu icons.
	// Summary: Call this member function to set the default icon size for cool menus.
	//			This will override the size that was set when LoadToolbar was called.
	//			This member should be called <b>after</b> InstallCoolMenus has been called
	//			in the frame class.  Standard size is 16x15 pixels.
	virtual void SetIconSize(CSize size);

	// Returns: A CSize object that represents the current icon size for cool menu
	//			images.
	// Summary: Call this member function to get the default icon size for cool menus.
	virtual CSize GetIconSize();

	// Input:	nIDCmd - Command ID to associate with the newly added image.
	//			hIcon - Icon handle of the image to add.
	//			sz - Width and height of the newly added item.
	//			bEnabled - true if the image is to be added to the enabled image list.
	// Summary: This member function is called to add an image to the image map for
	//			the cool menus.
	virtual void AddImageToMap(UINT nIDCmd, HICON hIcon, CSize sz, bool bEnabled=true);
public:
	virtual BOOL HookWindow(HWND hWnd);
	virtual BOOL HookWindow(CWnd* pWnd);

protected:

	// Input:	pDC - Points to the current device context.
	//			rect - Size of the area to paint.
	//			strText - A CString object that represents the text to be displayed.
	//			clrColor - An RGB value that represents the color of the text to be displayed.
	// Summary:	This member function is called by the cool menu to paint a menu item's text.
	virtual void DrawMenuText(CDC* pDC, CRect rect, CString strText, COLORREF clrColor);
    
    // Input:	pDC - Points to the current device context.
	//			rect - Size of the area to paint.
	//			bSelected - TRUE if the checkmark is to be drawn highlighted.
	//			bDisabled - TRUE if the checkmark is to be drawn disabled.
	//			hbmCheck - HBITMAP handle to the menu item info checkmark. It can be NULL.
	// Summary:	This member function is called by the cool menu to paint checkmarks
    //			and radio buttons for each item.
	virtual void Draw3DCheckmark(CDC* pDC,const CRect& rect,BOOL bSelected,BOOL bDisabled,HBITMAP hbmCheck);

	// Input:	pDC - Points to the current device context.
	//			rcItem - Size of the area to paint.
	//			bSelected - TRUE if the chevron is to be drawn highlighted.
    // Summary:	This member function is called by the cool menu to paint a chevron
	//			that is used to display hidden menu items.
    virtual void DrawChevron(CDC* pDC,CRect rcItem,BOOL bSelected);

	// Input:	pMenu - Points to a CMenu object to be made 'cool'.
	//			bHasButtons - TRUE if the menu has icons.
    // Summary: This member function is called by the cool menu to convert the specified
	//			CMenu object to a 'cool' menu.
    virtual void ConvertMenu(CMenu* pMenu,BOOL bHasButtons);

    // Input:	nCommandID - Command ID of a menu item.
	// Returns: TRUE if the item is hidden, otherwise returns FALSE.        
    // Summary: This member function is called by the cool menu to check to see
	//			if the specified menu item is to be hidden. 
    virtual BOOL IsItemHidden(UINT nCommandID);

    // Input:	pMenu - Points to the active menu.
	//			nPos - Position of a menu item.
	// Returns: TRUE if the menu item is a separator, otherwise returns FALSE.
    // Summary: This member function is called by the cool menu to check to see
	//			if the menu item is a separator. 
	BOOL IsItemSeparator(CMenu* pMenu,int nPos);

	// Input:	pMenu - Points to the active menu.
	//			nPos - Position of a menu item.
	// Returns: TRUE if the previous menu item is hidden, otherwise returns FALSE.
    // Summary: This member function is called by the cool menu to check to see
	//			if the previous menu item is hidden.
    virtual BOOL IsPrevItemHidden(CMenu* pMenu,int nPos);

	// Input:	pMenu - Points to the active menu.
	//			nPos - Position of a menu item.
	// Returns: TRUE if the next menu item is hidden, otherwise returns FALSE.      
    // Summary: This member function is called by the cool menu to check to see
	//			if the next menu item is hidden. 
    virtual BOOL IsNextItemHidden(CMenu* pMenu,int nPos);

    // Input:	hMenu - Points to the active menu.
	//			nPos - Position of a menu item.
    // Returns: TRUE if the menu item is the first one visible, otherwise returns FALSE.
    // Summary: This member function is called by the cool menu to check to see
	//			if the menu item is the first one visible. 
	virtual BOOL IsFirstVisibleItem(HMENU hMenu,UINT nPos);

    // Input:	hMenu - Points to the active menu.
	//			nPos - Position of a menu item.
	//			cmd - Command ID of the item to check.
	// Returns: TRUE if the menu item is the last one visible, otherwise returns FALSE.
    // Summary: This member function is called by the cool menu to check to see if
    //			the menu item is the last one visible. 
    virtual BOOL IsLastVisibleItem(HMENU hMenu,UINT nPos,UINT cmd);
    
    // Input:	pMenu - CMenu pointer to the active menu.
	//			bAppend - TRUE to append the menu.
	// Returns: TRUE if the menu was actually modified.
	// Summary: This member function is called by the cool menu to append a chevron
	//			menu item if the menu has hidden commands. 
    virtual BOOL UpdateChevronItem(CMenu* pMenu,BOOL bAppend);

    // Input:	pMenu - CMenu pointer to the active menu.
	// Returns: TRUE if the menu item is a system menu, otherwise returns FALSE.        
    // Summary: This member function is called by the cool menu to verify if the
	//			specified menu is a system menu or not. 
    virtual BOOL IsSysMenu(CMenu* pMenu);

	// Input:	pMenu - CMenu pointer to the active menu.
	// Returns: TRUE if there was a change to the menu.
    // Summary: This member function is called by the cool menu to restore previously
	//			hidden menu items. 
    virtual BOOL RestoreMenuItems(CMenu* pMenu);

	// Input:	pMenu - CMenu pointer to the active menu.
    // Summary: This member function is called by the cool menu to hide menu items.
    virtual void HideMenuItems(CMenu* pMenu);

    // Input:	pMenu - CMenu pointer to the active menu.
	// Returns: TRUE if the menu has hidden items, otherwise returns FALSE.        
    // Summary: This member function is called by the cool menu to check for hidden
	//			items. 
    virtual BOOL HasHiddenItems(CMenu* pMenu);

    // Input:	nItem - Menu item command ID.
	// Returns: TRUE if the menu item was added to the recent item list, otherwise returns FALSE.
    // Summary: This member function is called by the cool menu to add a hidden
	//			item to the short menu. 
    virtual BOOL AddRecentItem(UINT nItem);

    // Input:	nItem - Menu item command ID.
	// Returns: TRUE if the menu item is in the recent item list, otherwise returns FALSE.
    // Summary: This member function is called by the cool menu to see if the specified
	//			item is in the recent item list. 
    virtual BOOL IsRecentItem(UINT nItem);

	// Returns: true if the menus are allowed to use intelligent mode, otherwise returns false.
	//			The default implementation allows intelligent menus only if the menu was 
	//			created through the CXTMenu	class.
	// Summary: This internal member function checks to see if the menus are allowed
	//			to use intelligent mode. 
    virtual bool IsIntelligentModeAllowed();

    
    // Used by owner-drawn menu overrides

	static void UnhookCBTHook();
    static void HookCBTHook();
	static void RemoveBorder(HWND hWnd);
    static HHOOK m_hCBTHook;
    static LRESULT CALLBACK CBTHook(int nCode, WPARAM wParam, LPARAM lParam);
    
    friend class CXTCoolMenuInit;
    friend class CXTPopupMenu;
    
    // Ignore:
	//{{AFX_VIRTUAL(CXTCoolMenu)
	virtual void Destroy();
    virtual void Refresh();
    virtual void OnMenuTimer(int nIDEvent);
    virtual void OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu);
    virtual BOOL OnMeasureItem(LPMEASUREITEMSTRUCT lpms);
    virtual BOOL OnDrawItem(LPDRAWITEMSTRUCT lpds);
    virtual LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
    virtual void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL

public:
	static bool				m_bInstalled;
    
private:
	static bool				m_bIs95orNT4;
	static bool				m_bChevronSelected;
	static HMENU			m_hMenuLast;
	static CXTPopupMenu*	m_pPopupLast;
	static CXTPopupMenuList m_listPopups;
	int						m_nTextHeight;
};

// BULLETED LIST:

//////////////////////////////////////////////////////////////////////
// Summary: CXTMenu is a CMenu derived class.  It is used to create a CXTMenu object.
//			Use this class if you want your popup to be 'intelligent'.
//
//			Please note that intelligent menus have somewhat more limited capabilities
//			under <b>Windows 95 and Windows NT 4</b> (all other versions of Windows do 
//			not suffer from these limitations):
//			[ul]
//			[li]The menu will not expand automatically if you hover over the chevron.[/li]
//			[li]Clicking on a chevron in a submenu will cause the submenu to close.[/li]
//			[li]Intelligent menus are not available if you do not use the docking
//			XT menu bar (CXTMenuBar).[/li]
//			[/ul]
class _XT_EXT_CLASS CXTMenu : public CMenu
{
public:

    // Summary: Constructs a CXTMenu object.
    CXTMenu();

	// Summary: Destroys a CXTMenu object, handles cleanup and de-allocation.
    virtual ~CXTMenu();
    
	// Input:	nFlags - nFlags - Specifies a screen-position flag and a mouse-button flag.
	//			x - Specifies the horizontal position in screen coordinates 
    //			of the popup menu.
	//			y - Specifies the vertical position, in screen coordinates, of 
    //			the top of the menu on the screen.
	//			pWnd - Identifies the window that owns the popup menu.
	//			lpRect - Points to a RECT structure or a CRect object that contains 
    //			the screen coordinates of a rectangle within which the user 
    //			can click, without dismissing the popup menu.
	//			bNotify - TRUE to send WM_COMMAND notification back to the owner window.
    // Returns: Nonzero if the function is successful, otherwise returns zero.
    // Summary: Call this member function to display a floating popup menu at the
	//			specified location and track the selection of items on the popup menu.
    BOOL TrackPopupMenu(UINT nFlags,int x,int y,CWnd* pWnd,LPCRECT lpRect = NULL,BOOL bNotify = TRUE);

    // Input:	nFlags - Specifies a screen-position flag and a mouse-button flag.
	//			x - Specifies the horizontal position, in screen coordinates, 
    //			of the popup menu.
	//			y - Specifies the vertical position, in screen coordinates, of 
    //			the top of the menu on the screen.
	//			pWnd - Identifies the window that owns the popup menu.
	//			lptpm - Pointer to a TPMPARAMS structure that specifies an area 
    //			of the screen the menu should not overlap.
	//			bNotify - TRUE to send WM_COMMAND notification back to the owner window.
    // Returns: Nonzero if the function is successful, otherwise returns zero.
    // Summary: Call this member function to display a floating popup menu at the
	//			specified location and track the selection of items on the popup menu.
    BOOL TrackPopupMenuEx(UINT nFlags,int x,int y,CWnd* pWnd,LPTPMPARAMS lptpm = NULL,BOOL bNotify = TRUE);

	// Input:	hMenu - A Windows handle to a menu.
	// Returns: A pointer to a CMenu object.  The pointer may be temporary or permanent. 
    // Summary: Call this member function to get a pointer to a CMenu object given
	//			a Windows handle to a menu. 
    static CXTMenu* PASCAL FromHandle(HMENU hMenu);

	// Input:	hMenu - A Windows handle to a menu.
	// Returns: A pointer to a CMenu object.  If a CMenu object is not attached to the handle,
	//			NULL is returned.
    // Summary: Call this member function to get a pointer to a CMenu object when
	//			given a handle to a window. 
    static CXTMenu* PASCAL FromHandlePermanent(HMENU hMenu);

	// Input:	nPos - Specifies the position of the popup menu contained in the menu.
	// Returns: A pointer to a CMenu object, if a popup menu exists at the given position,
	//			otherwise returns NULL.
    // Summary: Call this member function to get a pointer to a CMenu object whose
	//			m_hMenu member contains a handle to the popup menu, if a popup menu
	//			exists at the given position. 
    CXTMenu* GetSubMenu(int nPos) const;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTCoolMenu::SetIntelligentMode(BOOL bIntelligent/*=TRUE*/) {
    m_bIntelligent = bIntelligent;
}
AFX_INLINE void CXTCoolMenu::SetRecentItemLength(int nLength) {
    m_nRecentList = nLength;
}
AFX_INLINE BOOL CXTCoolMenu::HookWindow(CWnd* pWnd) {
    return HookWindow(pWnd->GetSafeHwnd());
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTCOOLMENU_H__)