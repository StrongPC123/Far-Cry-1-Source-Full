// XTMenuBar.h interface for the CXTMenuBar class.
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

#if !defined(__XTMENUBAR_H__)
#define __XTMENUBAR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forwards

class CXTMenu;
class CXTMenuBarItem;
class CXTToolsManager;
class CXTSysMenuBarItem;
class CXTSubMenuBarItem;
class CXTControlMenuBarItem;
class CXTMenuCustomHandler;

//////////////////////////////////////////////////////////////////////
// Summary: XT_TRACK_STATE - Enumeration used by CXTMenuBar to determine the current
//			state of the menu bar.
typedef enum XT_TRACK_STATE
{
	TRACK_NONE = 0, // Normal, not tracking anything.
	TRACK_BUTTON,   // Tracking buttons (F10/Alt mode).
	TRACK_POPUP     // Tracking popups.
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTMenuBarItem - Descriptor of a generic menu bar item
class _XT_EXT_CLASS CXTMenuBarItem : public CObject
{
	bool				m_bWrapped; // Tells if this item is a line wrap
	CPoint				m_ptOrigin; // Origin of this item (its NW corner)
	CXTMenuBar* const	m_pMenuBar; // Owner of this item

public:

	// Input:	pMenuBar - Points to a CXTMenuBar object.
	// Summary:	Constructor initializes with default values
	CXTMenuBarItem(CXTMenuBar* pMenuBar);

	// Returns: true if the item is hidden, otherwise returns false.
	// Summary: Tells if this item is currently hidden.
	virtual bool IsHidden() const;

    // Returns: A pointer to a CXTMenuBar object.
    // Summary: Call this member function to return a pointer to the menubar.
	CXTMenuBar* GetMenuBar() const;

	// Input:	pDC - Pointer to a valid device context.
	// Summary:	Renders this item.
	virtual void Render(CDC* pDC) = 0;

    // Input:   pchAccelerator - A NULL terminated string.
	// Returns: true if successful, otherwise returns false.
	// Summary: Retrieves accelerator for this item if one exists .
	virtual bool GetAccelerator(TCHAR* pchAccelerator);

	// Returns: A CSize object.
	// Summary: Gets this item extent.
	virtual CSize GetExtent() = 0;

    // Returns: true if the menubar is wrapped, otherwise returns false.
    // Summary: Call this member function to determine if the menubar is wrapped.
	bool IsWrapped() const;

    // Returns: true if the menubar is wrappable, otherwise returns false.
    // Summary: Call this member function to determine if the menubar is wrappable.
	virtual bool IsWrappable() const;

    // Input:   bWrapped - True if the menubar is to be wrappable.
    // Summary: Call this member funciton to set the wrappable state for the menubar.
	void SetWrapped(bool bWrapped);

	// Returns: A CPoint object.
	// Summary: Gets this item origin (its NW point)
	CPoint GetOrigin() const;

	// Input:	ptOrigin - A CPoint object.
	// Summary:	Gets this item origin (its NW point).
	void SetOrigin(CPoint ptOrigin);

    // Returns: The amount in pixels representing spacing.
	// Summary:	Spacer specifies a distance between this and the following
	//          menu bar item, applicable if there is next item and this one is not
	//          wrapped.
	virtual int GetSpacer() const;

	// Summary:	Tracks associated popup menu, if any.
	virtual void TrackMenu();

	// Summary:	Handles double click on this item.
	virtual void OnDblClick();
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE bool CXTMenuBarItem::IsHidden() const {
	/*// not hidden by default*/return false;
}
AFX_INLINE CXTMenuBar* CXTMenuBarItem::GetMenuBar() const {
	return m_pMenuBar;
}
AFX_INLINE bool CXTMenuBarItem::GetAccelerator(TCHAR* /*pchAccelerator*/) {
	return false;
}
AFX_INLINE bool CXTMenuBarItem::IsWrapped() const {
	return IsWrappable() && m_bWrapped;
}
AFX_INLINE bool CXTMenuBarItem::IsWrappable() const {
	return true;
}
AFX_INLINE void CXTMenuBarItem::SetWrapped(bool bWrapped) {
	ASSERT(IsWrappable()); m_bWrapped = bWrapped;
}
AFX_INLINE CPoint CXTMenuBarItem::GetOrigin() const {
	return m_ptOrigin;
}
AFX_INLINE void CXTMenuBarItem::SetOrigin(CPoint ptOrigin) {
	m_ptOrigin = ptOrigin;
}
AFX_INLINE int CXTMenuBarItem::GetSpacer() const {
	return 0;
}
AFX_INLINE void CXTMenuBarItem::TrackMenu() {
}
AFX_INLINE void CXTMenuBarItem::OnDblClick() {
}

// Summary: CMap definition for mapping OLE items.
typedef CMap<long, long, BOOL, BOOL> CXTMenu2OLEItemMap;

// Summary: CTypedPtrArray definition for CXTMenuBarItem object arrays.
typedef CTypedPtrArray<CObArray, CXTMenuBarItem*> CXTMenuBarItemArray;

//////////////////////////////////////////////////////////////////////
// Summary: CXTMenuBar is a CXTControlBar derived class.  It is used to create an
//			Office&trade; style menu bar.  Use it the way you would a CToolBar, only you
//			need to call LoadMenuBar instead of LoadToolbar.
class _XT_EXT_CLASS CXTMenuBar : public CXTControlBar
{
	DECLARE_DYNAMIC(CXTMenuBar)

public:

	// Summary: Constructs a CXTMenuBar object.
	CXTMenuBar();

	// Summary: Destroys a CXTMenuBar object, handles cleanup and de-allocation.
	virtual ~CXTMenuBar();

protected:

    int                 m_iDepressedItem;       // Index of the currently depressed item.
    int                 m_iTracking;            // Index of the frame control button being tracked.
    int                 m_iHotItem;             // Index of the currently hot item.
    int                 m_nIDEvent;             // Menu timer ID.
    int                 m_nTimeOut;             // Time-out value, in milliseconds.
    int                 m_iPopupTracking;       // Index of which popup is being tracked, if any.
    int                 m_iNewPopup;            // Index of the next menu to track.
    int                 m_iButton;              // Index of the currently selected menu bar item.
    int                 m_nMenuID;              // Menu bar resource ID.
    int                 m_nMRUMaxWidth;         // Most recently used max width, or -1 if not set.
    bool                m_bTimerActive;         // true when the timer is activated.
    bool                m_bMoreWindows;         // true to display the "More Windows" menu command for MDI document management.
    bool                m_bStretchToFit;        // true if the menu bar is stretched to fit the entire window space.
    bool                m_bPtMouseInit;         // Tells if the last mouse position (m_ptMouse) has been initialized.
    bool                m_bShowMDIButtons;      // Tells if MDI buttons shall be rendered when the MDI child is maximized.
    bool                m_bDown;                // true when the button is pressed.
    bool                m_bProcessRightArrow;   // true to process left/right arrow keys.
    bool                m_bProcessLeftArrow;    // true to move to prev/next popup.
    bool                m_bEscapeWasPressed;    // true if the user pressed escape to exit the menu.
    bool                m_bDelayCheckMouse;     // Tells if idle update shall check for the mouse position to reset the hot item once the mouse leaves the window.
    BOOL                m_bMenuUnderlines;      // Win2000, TRUE if 'use menu underlines' is turned off in the control panel.
    BOOL                m_bActive;              // TRUE, if the application has activation.
    BOOL                m_bMDIMaximized;        // TRUE, if the window is maximized.
    HWND                m_hWndMDIClient;        // If this is an MDI application.
    HMENU               m_hMenu;                // Handle to the currently active menu.
    HMENU               m_hMenuDefault;         // Handle to the menu loaded via LoadMenuBar.
    HMENU               m_hMenuShared;          // Handle to the "Window" menu.
    CPoint              m_ptMouse;              // Mouse location when tracking the popup.
    CString             m_strValueName;         // Null-terminated string that specifies the value name in the registry.
    CString             m_strSubKey;            // Null-terminated string that specifies the key name in the registry.
    CXTMenu*            m_pMenuPopup;           // Pointer to the current popup being tracked.
    CUIntArray          m_arrHiddenCommands;    // Array of hidden menu commands.
    static int          m_iHookRefCount;        // Counts number of times the hook has been set.
    CImageList          m_imageList;            // Image list used by the MDI frame buttons.
    CToolTipCtrl        m_toolTip;              // MDI frame button tooltip.
    static HHOOK        m_hMsgHook;             // Handle to the message hook.  Set during menu tracking.
    XT_TRACK_STATE      m_iTrackingState;       // Current tracking state.
    CXTMBarWndHook*     m_pFrameHook;           // Hooks frame window messages.
    CXTMBarMDIWndHook*  m_pChildFrameHook;      // Hooks MDI client messages.
    static CXTMenuBar*  m_pMenuBar;             // Holds a 'this' pointer.  Set during menu tracking.
    CXTMenu2OLEItemMap  m_mapMenu2OLEItem;      // OLE menu item routing map.
    CXTMenuBarItemArray m_arrItems;             // Array of menu bar items.
	
private:

	CXTMenuCustomHandler*			m_pCustomHandler;
	CMap<HMENU,HMENU,HMENU,HMENU>	m_mapTools;

public:

	static bool m_bShowAll; // true when hidden menu items are displayed.
	static bool m_bAltKey;  // true when the alt key is pressed.

	// Input:	pMsg - Pointer to an MSG structure.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This function translates special menu keys and mouse actions, and is
	//			called from CXTFrameWnd's PreTranslateMessage. 
	virtual BOOL TranslateFrameMessage(MSG* pMsg);

	// Input:	hMenu - Handle to the new menu.
	//			hMenuShared - Handle to the new shared menu.
	// Returns: A handle to the old menu.  'hMenuShared' is the MDI "Window" menu, if any 
	//			(similar to WM_MDISETMENU).
	// Summary:	Call this function to load a different menu.  The HMENU must not belong
	//			to any CMenu, and you must free it when you are done.  
	HMENU LoadMenu(HMENU hMenu,HMENU hMenuShared);

	// Returns: A CMenu pointer object to the currently active menu.
	// Summary:	This member function retrieves a pointer to the currently active menu.
	CMenu* GetMenu() const;

	// Input:	nMenuID - Resource ID of the menu to load.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	Call this member function to load the menu bar specified by 'nMenuID'.
	BOOL LoadMenuBar(UINT nMenuID);

	// Input:	lpszMenuName - Pointer to the resource name of the menu bar to be loaded.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	Call this member function to load the menu bar specified by 'lpszMenuName'.
	BOOL LoadMenuBar(LPCTSTR lpszMenuName);

	// BULLETED LIST:

	// Input:	pParentWnd - Pointer to the window that is the menu bar’s parent.
	//			dwStyle - The menu bar style. Additional menu bar styles supported are: 
	//			[ul]
	//			[li]<b>CBRS_TOP</b> Control bar is at the top of the frame window.[/li]
	//			[li]<b>CBRS_BOTTOM</b> Control bar is at the bottom of the frame window.[/li]
	//			[li]<b>CBRS_NOALIGN</b> Control bar is not repositioned when the
	//			parent is resized.[/li]
	//			[li]<b>CBRS_TOOLTIPS</b> Control bar displays tool tips.[/li]
	//			[li]<b>CBRS_SIZE_DYNAMIC</b> Control bar is dynamic.[/li]
	//			[li]<b>CBRS_SIZE_FIXED</b> Control bar is fixed.[/li]
	//			[li]<b>CBRS_FLOATING</b> Control bar is floating.[/li]
	//			[li]<b>CBRS_FLYBY</b> Status bar displays information about the button.[/li]
	//			[li]<b>CBRS_HIDE_INPLACE</b> Control bar is not displayed to the user.[/li]
	//			[/ul]
	//			nID - The menu bar’s child-window ID.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function creates a Windows menu bar, a child window, and
	//			associates it with the CXTMenuBar object.  It also sets the menu bar
	//			height to a default value. 
	virtual BOOL Create(CWnd* pParentWnd,DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,UINT nID = AFX_IDW_MENUBAR);

	// Input:	pParentWnd - Pointer to the window that is the menu bar’s parent.
	//			dwUnused - Not used, should be zero.
	//			dwStyle - The menu bar style.  See Toolbar Control and Button Styles in the Platform
	//			SDK for a list of appropriate styles.
	//			rcBorders - A CRect object that defines the widths of the menu bar window borders.
	//			These borders are set to (0,0,0,0) by default, thereby resulting in
	//			a menu bar window with no borders.
	//			nID - The menu bar’s child-window ID.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	Call this member function to create a Windows menu bar, a child window,
	//			and associate it with the CXTToolBar object.  It also sets the menu bar
	//			height to a default value.
	//
	//			Use CreateEx, instead of Create, when certain styles need to be present
	//			during the creation of the embedded menu bar control.  For example, set
	//			'dwCtrlStyle' to TBSTYLE_FLAT | TBSTYLE_TRANSPARENT to create a menu bar
	//			that resembles the Internet Explorer 4 menu bars.
	virtual BOOL CreateEx(CWnd* pParentWnd,DWORD dwUnused = 0,DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP,CRect rcBorders = CRect(0, 0, 0, 0),UINT nID = AFX_IDW_MENUBAR);

	// Input:	nElapse - Specifies the time-out value, in milliseconds.
	//			nIDEvent - Specifies a nonzero timer identifier.
	// Summary:	This member function will set the timer identifier and time-out value
	//			for the cool menu.  The timer is activated when the mouse hovers over a
	//			menu bar button when using intelligent menus.  NOTE:  You will need to also
	//			call CXTCoolMenu::SetTimerInfo() if you are changing the 'nIDEvent' parameter.
	void SetTimerInfo(UINT nElapse,UINT nIDEvent=1000);

	// Input:	bStretchToFit - TRUE to stretch to fit the entire application area, FALSE to fit only
	//			the area occupied by menu items.
	// Summary:	Call this member function to enable the menu bar to occupy the entire 
	//			application area or size the menu bar to the area occupied by menu items
	//			only.
	void SetStretchToFit(BOOL bStretchToFit);

	// Returns: TRUE if the menu occupies the entire application area, or FALSE if it only
	//			occupies the area the size of the menu items.
	// Summary:	Call this member function to get the stretch state of the menu bar.  
	BOOL GetStretchToFit();

	// Input:	bShowMDIButtons - TRUE to display MDI buttons, FALSE to hide them.
	// Summary:	Call this member function to enable or disable rendering of the MDI buttons
	//			when an MDI frame is maximized.  Call this function after you have created
	//			your menubar.  The button layout will be automatically adjusted. 
	void ShowMDIButtons(BOOL bShowMDIButtons);

	// Returns: TRUE if the MDI buttons are rendered, otherwise returns FALSE.
	// Summary:	This member function tells if MDI buttons are currently rendered.
	BOOL IsShowMDIButtons() const;

	// Input:	nCommandID - The command ID of a menu item to hide.
	// Returns: TRUE if successful, otherwise returns FALSE.        
	// Summary:	This member function adds the specified command to the list of
	//			menu items to hide until activated by clicking on the chevron.
	virtual BOOL HideCommand(UINT nCommandID);

	// Input:	nCommandIDs - An array of command IDs, of menu items, to hide.
	//			nSize - Size of the array passed in.
	// Returns: TRUE if successful, otherwise returns FALSE.        
	// Summary:	This member function adds the specified commands to the list of
	//			menu items to hide until activated by clicking on the chevron.
	virtual BOOL HideCommands(const UINT* nCommandIDs,int nSize);

	// Input:	pObject - Represents a valid CObject pointer.
	// Returns: TRUE if the object passed in is a CXTMenuBar object, otherwise returns FALSE.
	// Summary:	This function checks if the object passed in as 'pObject' is a CXTMenuBar
	//			object. 
	static BOOL IsMenuBar(CObject* pObject);

	// Returns: TRUE if the active MDI frame is maximized, otherwise returns FALSE.
	// Summary:	This function tells if the active MDI frame is currently maximized.
	BOOL IsMDIMaximized() const;

	// Input:	bEnable - true to show "More Windows", or false to hide.
	// Summary:	Call this member function to enable or disable the "More Windows" menu
	//			item that is displayed in the "Windows" pull down menu.
	void EnableMoreWindows(bool bEnable=true);

	// Input:	bSend - A reference to a valid BOOL value.
	//			nMsg - Specifies the message to be sent. 
	//			wParam - Specifies additional message-specific information.
	//			lParam - Specifies additional message-specific information.
	// Returns: An HWND data type that represents the window handle.
	// Summary:	This menu function is called to determine the window handle for command
	//			routing. 
	HWND OleMenuDescriptor(BOOL& bSend,UINT nMsg,WPARAM wParam,LPARAM lParam);

	// Returns: A pointer to a valid CFrameWnd object.
	// Summary:	This member function returns a pointer to the menu bar owner frame.
	CFrameWnd* GetOwnerFrame();
	
	// Returns: A pointer to a valid COleDocument object.
	// Summary:	This member function returns a pointer to the OLE document that sent
	//			the last command if the menu bar is used with an OLE framework. 
	COleDocument* GetCmdSentOleDoc();
	
	// Returns: A pointer to a valid CWnd object.
	// Summary:	This member function returns a pointer to the OLE window that sent
	//			the last command if the menu bar is used with an OLE framework. 
	CWnd* GetCmdSentOleWnd();

	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function is called to fill the routing map for the OLE
	//			framework. 
	BOOL FillCommandRoutingMap();

protected:

	// Input:	rcButton - A reference to a valid CRect object that represents the size of the
	//			button.
	//			tpm - Reference to a TPMPARAMS struct.
	//			nFlags - A reference to TrackPopupMenu flags
	// Returns: A CPoint object.
	// Summary:	This member function will insure the menu always appears inside the
	//			window. It will, given a button rectangle, compute point and "exclude
	//			rect" for TrackPopupMenu, based on the current docking style, so that
	//			the menu will always appear inside the window. 
	virtual CPoint ComputeMenuTrackPoint(const CRect& rcButton, TPMPARAMS& tpm, UINT &nFlags);

	// Summary: This member function initializes an MDI "Window" menu by adding names
	//			of all the MDI children.  This duplicates what the default handler
	//			for WM_MDISETMENU does, but it is necessary to reinvent the wheel since
	//			menu bars manage the menus themselves.  This function is called when
	//			the frame gets WM_INITMENUPOPUP.
	virtual void OnInitWindowMenu();

	// BULLETED LIST:

	// Input:	message - Message identifier.
	//			nFlags - Indicates whether various virtual keys are down.  This parameter 
	//			can be any combination of the following values: 
	//			[ul]
	//			[li]<b>MK_CONTROL</b> Set if the CTRL key is down.[/li]
	//			[li]<b>MK_LBUTTON</b> Set if the left mouse button is down.[/li]
	//			[li]<b>MK_MBUTTON</b> Set if the middle mouse button is down.[/li]
	//			[li]<b>MK_RBUTTON</b> Set if the right mouse button is down.[/li]
	//			[li]<b>MK_SHIFT</b> Set if the SHIFT key is down.[/li]
	//			[/ul]
	//			pt - Specifies the x- and y-coordinate of the cursor.  These coordinates 
	//			are always relative to the upper-left corner of the window.
	// Returns: TRUE if handled; i.e., caller should eat the mouse message.
	// Summary:	Call this member function to handle a mouse message.  It looks for
	//			a click on one of the buttons.  
	virtual BOOL OnMouseMessage(UINT message,UINT nFlags,CPoint pt);

	// Summary: This member function makes the frame recalculate the control bar sizes
	//			after a menu change.
	void DoLayout();

	// Input:	iButton - Index of the starting button.
	//			bPrev - TRUE to search for the previous button.  FALSE to search for the next
	//			button.
	// Returns: The index of the previous, or next, button found.
	// Summary:	This member function gets the button index of the button before, or
	//			after, a given button specified by 'iButton'. 
	int GetNextOrPrevButton(int iButton,BOOL bPrev) const;

	// Input:	iState - Passed in XT_TRACT_STATE structure.
	//			iButton - Index of the button to set the state for.
	// Summary:	This member function sets the tracking state for a button to either none,
	//			button, or popup.
	void SetTrackingState(XT_TRACK_STATE iState,int iButton=-1);

	// Input:	iButton - The index of the button item to make hot.
	// Summary:	This member function sets an 'iButton' item as hot-selected.
	void SetHotItem(int iButton);

	// Input:	iButton - The index of the button item to depress.
	// Summary:	This member function makes an 'iButton' item appear pressed.  Pass in
	//			-1 to release all buttons.
	void PressItem(int iButton);

	// Input:	pItem - A pointer to a valid CXTMenuBarItem object.
	// Returns: The index of the item, or -1 if the item is not found.
	// Summary:	This member function searches for a CXTMenuBarItem object. 
	int FindItem(CXTMenuBarItem* pItem);

	// Input:	iButton - Index of the button to track.
	// Summary:	This member function tracks the popup submenu associated with the active 
	//			button in the menu bar. This function actually goes into a loop, tracking
	//			different menus until the user selects a command or exits the menu.
	void TrackPopup(int iButton);

	// Input:	pt - Specifies the x- and y-coordinate of the cursor.  These coordinates 
	//			are always relative to the upper-left corner of the window.
	//			itemsBegin - Pointer to the first entry in an array of item indices that need to be
	//			checked.
	//			itemsEnd - Pointer past the last entry in an array of item indices that need
	//			to be checked. 
	// Returns: The index of the item that contains 'pt', or -1 if there was no match.
	// Summary:	This member function determines which menu bar item, within the range of
	//			'itemsBegin' to 'itemsEnd', a point is in ('pt' in client coordinates).
	virtual int HitTest(CPoint pt,int* itemsBegin,int* itemsEnd) const;

	// Summary: This member function toggles the state from home state to button-tracking,
	//			and back.
	void ToggleTrackButtonMode();

	// Input:	iButton - Index of the new popup to track, or -1 to quit tracking.
	// Summary:	This member function cancels the current popup menu by posting WM_CANCELMODE,
	//			and tracks a new menu.
	void CancelMenuAndTrackNewOne(int iButton);

	// Input:	hMenu - Handle to the menu.
	//			nItemID - Menu item ID or submenu index of the item selected.
	// Summary:	This member function is called by the menu bar when a user selects a
	//			new menu item.  This will determine if the selected item is a submenu
	//			and or parent menu item.  This way the menu bar knows whether the right
	//			or left arrow key should move to the next menu popup.
	void OnMenuSelect(HMENU hMenu,UINT nItemID);

	// Input:	bDoLayout - Tells if the layout shall be recalculated if a change is detected.
	// Summary:	This member function checks whether the MDI maximized state has changed.
	//			If so, add or delete the min/max/close buttons to or from the menu bar.
	void CheckMinMaxState(bool bDoLayout);

	// Input:	iButton - Index of the button to check.
	// Returns: TRUE if it is a valid index, otherwise returns FALSE.
	// Summary:	This member function is called to check to see if the button specified
	//			by 'iButton' is a valid index. 
	BOOL IsValidButton(int iButton) const;

	// Input:	m - A Reference to a valid tagMSG structure.
	//			pWndMenu - A pointer to a valid CWnd object.
	// Returns: TRUE if message is handled (to eat it).
	// Summary:	This member function handles a menu input event.  It looks for a left
	//			or right arrow key to change the popup menu, or mouse movement over
	//			a different menu button for "hot" popup effect. 
	virtual BOOL OnMenuInput(MSG& m,CWnd* pWndMenu = NULL);

	// Input:	bAltKey - true if the alt key was used to activate menu.
	// Summary:	This member function is called to update the display for Windows 2000 menus.
	void UpdateDisplay(bool bAltKey);

	// Returns: The window styles for the active MDI client window.
	// Summary:	The control bar calls this method to return the window style for the
	//			active MDI child. 
	long GetMDIWindowStyle();

	// Summary: This member function removes all menu bar items.
	void RemoveAll();

	// Input:	itemsBegin - Beginning of the buffer to store indices.
	//			itemsEnd - Points past the last item in the buffer.
	//			flags - OR'ed (|) combination of the ITEMTYPE_... flags.
	// Returns: A pointer past the last initialized buffer item.	
	// Summary:	This member function gets the indices of all the items currently visible.
	int* CXTMenuBar::GetItems(int* itemsBegin,int* itemsEnd,int flags) const;

	// Input:	pItem - A pointer to a valid CXTMenuBarItem object.
	// Returns: A CRect object that represents the item.
	// Summary:	This member function computes the item rectangle, as per current menu bar
	//			orientation. 
	CRect GetItemRect(CXTMenuBarItem* pItem) const;

	// Input:	bHorz - true if horizontal border size is to be calculated.
	// Returns: A CSize object that represents the width and height of all the borders.
	// Summary:	This member function computes the total width and height of all the
	//			borders. 
	CSize GetBorderSize(bool bHorz) const;

	// Summary: This member function resets the state tracking variables.
	void ResetTrackingState();

	// Returns: true to enable full window drag, or false to use the wire frame.
	// Summary:	This member function tells if this menu bar shall be visualized when
	//			dragging or if the system shall render it with a wire frame.  Default
	//			implementation uses full window drag. 
	virtual bool IsFullWindowDrag();

	// BULLETED LIST:

	// Input:	pSize - [ul]
	//			[li]<b>IN</b> Extent of the rectangle in which the hot spot must be defined.[/li]
	//			[li]<b>OUT</b> Offset of the hot spot from the rect's top-left corner.[/li]
	//			[/ul]
	// Returns: true if the menu bar defines a hot spot (the default) in which case a 
	//			buffer, pointed to by 'pSize', is filled with the hot spot offset from 
	//			the top-left corner.  Returns false to indicate that no hot spot is defined.
	// Summary:	This member function calculates the position of the menu bar hot spot,
	//			i.e., the point that is used to pin the control bar rect to the mouse
	//			cursor when dragging it. 
	virtual bool GetHotSpot(LPSIZE pSize);

	// Input:	dwMenuPopupID - Popup menu ID
	// Returns:	A menu popup by its CRC32-based ID
	// Summary:	This member function returns a HMENU handle for the popup menu specified by dwMenuPopupID.
	HMENU GetMenuPopupByCrc(DWORD dwMenuPopupID);

	// Ignore:
	//{{AFX_VIRTUAL(CXTMenuBar)
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	//}}AFX_VIRTUAL

	virtual XT_TRACK_STATE GetTrackingState(int& iPopup);
	virtual void OnInitMenuPopup();
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void OnBarStyleChange(DWORD dwOldStyle,  DWORD dwNewStyle);
	virtual bool HasCmdHandlers(CWnd* pWnd);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Ignore:
	//{{AFX_MSG(CXTMenuBar)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
 
	DECLARE_MESSAGE_MAP()

	// Input:	code - An integer value.
	//			wParam - Specifies additional message-specific information.
	//			lParam - Specifies additional message-specific information.
	// Returns:	An LRESULT Object
	// Summary:	Menu filter hook just passes to virtual CXTMenuBar function.
	static LRESULT CALLBACK MenuInputFilter(int code, WPARAM wParam, LPARAM lParam);

	// drawing helpers

	void OnDrawIcon(CDC* pDC, CXTSysMenuBarItem* pItem);
	void OnDrawMenuItem(CDC* pDC, CXTSubMenuBarItem* pItem);
	void OnRenderControl(CDC* pDC, CXTControlMenuBarItem* pItem);
	void DrawMenuItemText(CDC* pDC, CRect& rcItem, CXTSubMenuBarItem* pItem, bool bHorz);
	void DrawVertText(CDC* pDC, CRect& rcItem, CXTSubMenuBarItem* pItem, COLORREF crText);
	void DrawHorzText(CDC* pDC, CRect& rcItem, CXTSubMenuBarItem* pItem);

	// these track menu popups

	void OnTrackSubMenu(CXTSubMenuBarItem* pItem);
	void OnTrackWindowMenu(CXTSysMenuBarItem* pItem);
	void OnDblClickWindowMenu(CXTSysMenuBarItem* pItem);

	// Layout helpers

	CSize CalcLayout(DWORD nMode, int nLength = -1);
	CSize CalcItemExtent(bool bHorz);
	int WrapMenuBar(int nWidth);
	void SizeMenuBar(int nLength, bool bHorz);
	void CalcItemLayout(bool bHorz); 
	int GetClipBoxLength(bool bHorz);

	// alignment (CBRS_ALIGN_...) with regard to floating status
	
	DWORD GetAlignmentStyle() const;
    void DrawXPFrameControl(CDC* pDC, CRect& r, UINT uStyle, bool bHilite=false, bool bPushed=false);
	CFont& GetTextFont() const;
    CSize GetTextSize(CDC* pDC, CString strMenuText) const;
    TCHAR GetHotKey(LPCTSTR lpszMenuName) const;
	virtual void ActivateToolTips(CPoint point, UINT uState);

	virtual BOOL IsFloating() const;
	virtual BOOL IsHorzDocked() const;

	// Input:	lp - Pointer to a rebar bar descriptor to use to fill in the sizing information.
	//			bHorz - Tells if this control bar must be oriented horizontally.
	// Summary:	This notification is called whenever this menu bar is added to a CXTReBar object.
	virtual bool OnAddedToRebar(REBARBANDINFO* lp,bool bHorz);

	// Input:	pInfo - Descriptor of the band 
	//			bHorz - Tells if horizontally oriented
	// Summary:	Called whenever this menu bar is embedded in CXTReBar control
	//			that has just resized the band in which this menu bar resides.
	virtual void OnRebarBandResized(XT_REBARSIZECHILDINFO* pInfo, bool bHorz);

	// Input:	bMode - True to enable customization.
	// Summary:	Sets customization mode on/off.
	void SetCustMode(bool bMode);

	// Returns: An RGB color value.
	// Summary:	This member function is used to determine the correct background fill
	//			color to be used during paint operations. 
	virtual COLORREF GetBackgroundColor() const;

	friend class CXTMenu;
	friend class CXTCustTools;
	friend class CXTMBarWndHook;
    friend class CXTToolsManager;
	friend class CXTMBarMDIWndHook;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE BOOL CXTMenuBar::IsValidButton(int iButton) const {
	ASSERT(::IsWindow(m_hWnd)); return 0 <= iButton && iButton < m_arrItems.GetSize();
}
AFX_INLINE XT_TRACK_STATE CXTMenuBar::GetTrackingState(int& iPopup) {
	ASSERT(::IsWindow(m_hWnd)); iPopup = m_iPopupTracking; return m_iTrackingState;
}
AFX_INLINE CMenu* CXTMenuBar::GetMenu() const {
	ASSERT(::IsWindow(m_hWnd)); return CMenu::FromHandle(m_hMenu);
}
AFX_INLINE void CXTMenuBar::SetStretchToFit(BOOL bStretchToFit) {
	m_bStretchToFit = (bStretchToFit != 0);
}
AFX_INLINE BOOL CXTMenuBar::GetStretchToFit() {
	return m_bStretchToFit;
}
AFX_INLINE void CXTMenuBar::ShowMDIButtons(BOOL bShowMDIButtons) {
	ASSERT(::IsWindow(m_hWnd)); m_bShowMDIButtons = (bShowMDIButtons != 0); DoLayout();
}
AFX_INLINE BOOL CXTMenuBar::IsShowMDIButtons() const {
	return m_bShowMDIButtons;
}
AFX_INLINE BOOL CXTMenuBar::IsMenuBar(CObject* pObject) {
	return pObject->IsKindOf(RUNTIME_CLASS(CXTMenuBar));
}
AFX_INLINE void CXTMenuBar::EnableMoreWindows(bool bEnable/*=true*/) {
	m_bMoreWindows = bEnable;
}
AFX_INLINE BOOL CXTMenuBar::IsMDIMaximized() const {
	return m_bMDIMaximized;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTMENUBAR_H__)