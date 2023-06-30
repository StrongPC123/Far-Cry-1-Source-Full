// XTGlobal.h interface for the XT_AUX_DATA struct.
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

#if !defined(__XTGLOBALS_H__)
#define __XTGLOBALS_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Uncomment this definition if your application uses a resource dll,
// and add #include "XTResource.rc" to your resource dll's .rc2 file.
// You will have to call xtAfxData.InitResources(HINSTANCE) once you 
// have a resource handle, to initialize the Xtreme Toolkit resources.
// You will need to rebuild the library after you have done this.
//#define _XT_USES_RESOURCE_DLL

//////////////////////////////////////////////////////////////////////
// Summary: XT_ASSERT_MSG is similar to ASSERT, except that it allows a custom
//			message to be specified.
#ifdef _DEBUG

#ifndef _AFX_NO_DEBUG_CRT

#define XT_ASSERT_MSG(exp, msg) \
{ \
	if ( !(exp) && (_CrtDbgReport( _CRT_ASSERT, __FILE__, __LINE__, NULL, "\n-----------------------\n" msg "\n-----------------------" ) ) ) \
		AfxDebugBreak(); \
} \

#else
#define XT_ASSERT_MSG(exp, msg) (void)( (exp) || (_assert("\n-----------------------\n" msg "\n-----------------------", __FILE__, __LINE__), 0) )
#endif//_AFX_NO_DEBUG_CRT

#else
#define XT_ASSERT_MSG(exp, msg) ((void)0)
#endif//_DEBUG

//////////////////////////////////////////////////////////////////////
// Summary: XT_DROPDOWNBUTTON is a stand alone helper structure class.  It is used
//			by the CXTToolBar class to handle popup menu and color picker.
struct XT_DROPDOWNBUTTON
{
public:

	// Summary: Constructs a XT_DROPDOWNBUTTON object with default values.
	XT_DROPDOWNBUTTON();

	// Input:	pOther - A pointer to a valid XT_DROPDOWNBUTTON structure.
	// Summary:	Constructs a XT_DROPDOWNBUTTON object by copying values from an existing
	//			XT_DROPDOWNBUTTON structure.
	XT_DROPDOWNBUTTON(const XT_DROPDOWNBUTTON* pOther);

    UINT     idButton;         // Command ID of the button.
    UINT     idMenu;           // Popup menu to display.
    BOOL     bShowColor;       // TRUE to draw a color box for color selection.
    BOOL     bMenuBarPopup;    // It is mutually exclusive with 'bShowColor'. TRUE if a toolbar dropdown was created from a menu bar popup.
    BOOL     bColorPicker;     // TRUE if the dropdown is a color picker.
    BOOL     bArrow;           // TRUE to render arrows for the dropdown.
    DWORD    dwPopup;          // Style flags for color popup window.
    DWORD    dwMenuBarPopupID; // Unique ID of the menu bar popup. It is valid if 'bMenuBarPopup' is TRUE.
    COLORREF clrDefault;       // Default color for the color popup window.
    COLORREF clrColor;         // An RGB value for the current color for the color popup window.
	

};


//////////////////////////////////////////////////////////////////////
// Summary: XT_LVITEMDATA is a stand alone helper structure class.  It holds list
//			item data for a shell list. It is used by CXTShellTreeCtrl, CXTShellTreeView,
//			CXTShellListCtrl and CXTShellListView.
struct XT_LVITEMDATA
{
	ULONG			ulAttribs;	// Shell item attributes.
	LPITEMIDLIST	lpi;		// Pointer to an item ID list.
	LPSHELLFOLDER	lpsfParent; // Points to the parent shell folder item.
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_TVITEMDATA is a stand alone helper structure class.  It is used to
//			create a structure that holds tree item data for a shell tree.  It is
//			used by CXTShellTreeCtrl, CXTShellTreeView, CXTShellListCtrl and CXTShellListView.
struct XT_TVITEMDATA
{
	LPITEMIDLIST	lpi;		// Pointer to an item ID list.
	LPITEMIDLIST	lpifq;		// Pointer to an item ID list.
	LPSHELLFOLDER	lpsfParent; // Pointer to the parent shell folder item.
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_CMDINFO is a stand alone helper structure class. It is used by 
//			CXTCommandsListBox and CXTCustCommands during toolbar customization.
struct XT_CMDINFO
{
	UINT	nCmdID;		// Command identifier.
	HICON	hIcon;		// Icon associated with the command.
	LPCTSTR lpszItem;	// Text for the menu or toolbar command.
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_OUTBAR_INFO is a stand alone helper structure class. It is used by
//			CXTOutBarCtrl to store and send information for an Outlook bar folder
//			or item.
struct XT_OUTBAR_INFO 
{
	int		nIndex;		// Index of the item.
	int		nDragTo;	// Ending drag index.
	int		nDragFrom;	// Starting drag index.
	bool	bFolder;	// true if the item is a folder	
	LPCTSTR lpszText;	// Item text.

};

//////////////////////////////////////////////////////////////////////
// Summary: XT_TCB_ITEM is a stand alone helper structure class.  It is used by
//			CXTTabCtrl and CXTTabView to store information for a particular tab
//			item.
struct XT_TCB_ITEM
{
	UINT	uiToolTipId;		// Resource ID for the tooltip.
	CWnd*	pWnd;				// A CWnd pointer to the window associated with a tab.
	CString szTabLabel;			// User specified label for the tab.
	CString szToolTipLabel;		// Tooltip text for the tab.
	CString szCondensedLabel;	// The label actually being displayed for auto-condensing tabs. 
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_DLLVERSIONINFO is a stand alone helper structure class.  It is used 
//			to store version information for a specified module.
struct XT_DLLVERSIONINFO
{
	DWORD cbSize;			// Size of the structure, in bytes. This member <b>must</b> be filled in <b>before</b> calling the function.
	DWORD dwMajorVersion;	// Major version of the DLL. If the DLL's version is 4.0.950, this  value will be 4.
	DWORD dwMinorVersion;	// Minor version of the DLL. If the DLL's version is 4.0.950, this  value will be 0.
	DWORD dwBuildNumber;	// Build number of the DLL. If the DLL's version is 4.0.950, this  value will be 950.

	// BULLETED LIST:
	
	// Identifies the platform for which the DLL was built. This can 
	// be one of the following values: 
	// [ul]
	// [li]<b>DLLVER_PLATFORM_WINDOWS</b> The DLL was built for all Windows
	//        platforms.[/li]
	// [li]<b>DLLVER_PLATFORM_NT</b> The DLL was built specifically for
	//        Windows NT.[/li]
	// [/ul]
	DWORD dwPlatformID;
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_CLRFONT is a stand alone helper structure class.  It is used by CXTTreeCtrl
//			and CXTTreeView to store information for a specified tree item.
struct XT_CLRFONT
{
    LOGFONT  logfont; // A LOGFONT object that represents the tree item font.
    COLORREF color;   // An RGB value that represents the color for a tree item.
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_TOOLBARDATA is a stand alone helper structure class.  It is used
//			by CXTTabCtrl and CXTTabView to store information for a specified tab
//			item.
struct XT_TOOLBARDATA
{
    WORD wVersion;   // Version number should be 1.
    WORD wWidth;     // Width of one bitmap.
    WORD wHeight;    // Height of one bitmap.
    WORD wItemCount; // Number of items.
    WORD items[1];   // Array of command IDs. The actual size is 'wItemCount'.

	// Input:	nItem - Index of the item to retrieve the command ID for.
	// Returns: A WORD object that represents the command identifier.
    // Summary:	This function is used by CXTCoolMenu and CXTToolBar to return 
    //			the command identifier specified by the index 'nItem'. 
    WORD GetItem(int nItem);
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE WORD XT_TOOLBARDATA::GetItem( int nItem ) {
	return items[nItem]; 
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Summary: XT_LOGFONT is a self initializing LOGFONT derived structure class. It
//			will allow you to create or copy a LOGFONT object, and defines the attributes
//			of a font.
struct XT_LOGFONT : public LOGFONT
{
    // Summary: Constructs an XT_LOGFONT object.
    XT_LOGFONT();

	// Input:	logfont - Valid address of a LOGFONT structure.
    // Summary:	Copy constructor will construct an XT_LOGFONT object and copy the
	//			data specified by 'logfont' into the structure's data members.
    XT_LOGFONT(LOGFONT& logfont);
    
	// Input:	logfont - Valid address of a LOGFONT structure.
    // Summary:	This overloaded operator will copy the data specified by 'logfont'
    // into the structure's data members.
    void operator = (LOGFONT& logfont);

	DWORD dwType;  // Used to hold the font type, i.e. TT_FONT, DEVICE_FONT.
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE XT_LOGFONT::XT_LOGFONT() {
	::ZeroMemory(this, sizeof(XT_LOGFONT)); dwType = 0;
}    
AFX_INLINE XT_LOGFONT::XT_LOGFONT( LOGFONT& logfont ) {
	::CopyMemory((void*)&*this, (const void*) &logfont, (DWORD)sizeof(LOGFONT)); dwType = 0;
}
AFX_INLINE void XT_LOGFONT::operator = (LOGFONT& logfont) {
	::CopyMemory((void*)&*this, (const void*)&logfont, (DWORD)sizeof(LOGFONT));
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Summary: XT_MENUITEMINFO is a self initializing MENUITEMINFO derived structure
//			class.  It is used by CXTCoolMenu and CXTMenuBar, and contains
//			information about menu items.
struct _XT_EXT_CLASS XT_MENUITEMINFO : public MENUITEMINFO
{
    // Summary: Constructs an XT_MENUITEMINFO object.
    XT_MENUITEMINFO();
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_NONCLIENTMETRICS is a self initializing NONCLIENTMETRICS derived
//			structure class. It contains the scalable metrics associated with the
//			non-client area of a non-minimized window.  This structure is used by
//			the SPI_GETNONCLIENTMETRICS and SPI_SETNONCLIENTMETRICS actions of 
//			SystemParametersInfo.
struct XT_NONCLIENTMETRICS : public NONCLIENTMETRICS
{
    // Summary: Constructs an XT_NONCLIENTMETRICS object.
    XT_NONCLIENTMETRICS();
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE XT_NONCLIENTMETRICS::XT_NONCLIENTMETRICS() { 
	memset(this, 0, sizeof(NONCLIENTMETRICS)); cbSize = sizeof(NONCLIENTMETRICS); /*// Retrieves the value of the specified system-wide parameter.*/VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), this, 0));
}

//////////////////////////////////////////////////////////////////////
// Summary: XT_REBARSIZECHILDINFO is a stand alone structure class.  It is used to 
//			create an XT_REBARSIZECHILDINFO structure.
struct XT_REBARSIZECHILDINFO
{
    REBARBANDINFO*    pBandInfo; // Points to a REBARBANDINFO structure that contains information that defines a band in a rebar control.
    NMREBARCHILDSIZE* pSizeInfo; // Points to a NMREBARCHILDSIZE structure that contains information used in handling the RBN_CHILDSIZE notification message.
};

//////////////////////////////////////////////////////////////////////
// Summary: XT_AUX_DATA is a stand alone global item data structure class. It is
//			used by the toolkit to initialize and store resource and item data shared
//			by all objects.  Items include system colors, icons, fonts and OS flags.
struct _XT_EXT_CLASS XT_AUX_DATA
{
private:
	
    // Summary: Constructs an XT_AUX_DATA object.
    XT_AUX_DATA();

public:

    // Summary: Destroys an XT_AUX_DATA object, handles cleanup and de-allocation.
    virtual ~XT_AUX_DATA();

	////////////////////////////////////////////////////////////////////
    // (RGB) System colors used by Xtreme Toolkit
	////////////////////////////////////////////////////////////////////

	COLORREF clr3DFace;						// An RGB value that represents the face color for three dimensional display elements.
	COLORREF clr3DShadow;					// An RGB value that represents the shadow color for three dimensional display elements.
	COLORREF clr3DDkShadow;					// An RGB value that represents the dark shadow for three dimensional display elements.
	COLORREF clr3DHilight;					// An RGB value that represents the highlight color for three dimensional display elements.
	COLORREF clr3DLight;					// An RGB value that represents the light color for three dimensional display elements.
	COLORREF clrBtnText;					// An RGB value that represents the text color on push buttons.
	COLORREF clrGrayText;					// An RGB value that represents the grayed (disabled) text.
	COLORREF clrHighlight;					// An RGB value that represents the item(s) selected in a control.
	COLORREF clrHighlightText;				// An RGB value that represents the text color of item(s) selected in a control.
	COLORREF clrMenu;						// An RGB value that represents the menu background.
	COLORREF clrMenuText;					// An RGB value that represents the text color in menus.
	COLORREF clrWindow;						// An RGB value that represents the window background.
	COLORREF clrWindowFrame;				// An RGB value that represents the window frame.
	COLORREF clrWindowText;					// An RGB value that represents the text color in windows.
	COLORREF clrActiveCaption;				// An RGB value that represents the active window title bar.
	COLORREF clrInActiveCaption;			// An RGB value that represents the inactive window title bar.
	COLORREF clrGradActiveCapt;				// An RGB value that represents the gradient active title bar.
	COLORREF clrGradInActiveCapt;			// An RGB value that represents the gradient inactive title bar.
	COLORREF clrActiveCaptText;				// An RGB value that represents the active caption text.
	COLORREF clrInactiveCaptText;			// An RGB value that represents the inactive caption text.
	COLORREF clrXPBarFace;					// An RGB value that represents the XP toolbar background color.
	COLORREF clrXPHighlight;				// An RGB value that represents the XP menu item selected color.
	COLORREF clrXPHighlightBorder;			// An RGB value that represents the XP menu item selected border color.
	COLORREF clrXPHighlightPushed;			// An RGB value that represents the XP menu item pushed color.
	COLORREF clrXPIconShadow;				// An RGB value that represents the XP menu item icon shadow.
	COLORREF clrXPGrayText;					// An RGB value that represents the XP menu item disabled text color.
	COLORREF clrXPHighlightChecked;			// An RGB value that represents the XP menu item checked color.
	COLORREF clrXPHighlightCheckedBorder;	// An RGB value that represents the XP menu item checked border color.
	COLORREF clrXPGripper;					// An RGB value that represents the XP toolbar gripper color.
	COLORREF clrXPSeparator;				// An RGB value that represents the XP toolbar separator color.
	COLORREF clrXPDisabled;					// An RGB value that represents the XP menu icon disabled color.
	COLORREF clrXPMenuTextBack;				// An RGB value that represents the XP menu item text background color.
	COLORREF clrXPMenuExpanded;				// An RGB value that represents the XP hidden menu commands background color.
	COLORREF clrXPMenuBorder;				// An RGB value that represents the XP menu border color.
	COLORREF clrXPMenuText;					// An RGB value that represents the XP menu item text color.
	COLORREF clrXPHighlightText;			// An RGB value that represents the XP menu item selected text color.
	COLORREF clrXPBarText;					// An RGB value that represents the XP toolbar text color.
	COLORREF clrXPBarTextPushed;			// An RGB value that represents the XP toolbar pushed text color.
	COLORREF clrXPTabInactiveBack;			// An RGB value that represents the XP inactive tab background color.
	COLORREF clrXPTabInactiveText;			// An RGB value that represents the XP inactive tab text color.
	
	////////////////////////////////////////////////////////////////////
    // Cursors used by Xtreme Toolkit
	////////////////////////////////////////////////////////////////////

	HCURSOR hcurDragCopy;	// Drag copy.
	HCURSOR hcurDragMove;	// Drag move.
	HCURSOR hcurDragNone;	// Drag none.
	HCURSOR hcurHand;		// Hand.
	HCURSOR hcurHandNone;	// No Hand.
	HCURSOR hcurHSplitBar;	// Horizontal Splitter.
	HCURSOR hcurVSplitBar;	// Vertical Splitter.
	HCURSOR hcurMove;		// 4 way move.
	
	////////////////////////////////////////////////////////////////////
    // System metrics for small icons.
	////////////////////////////////////////////////////////////////////

	int cxSmIcon;		// cx small icon size (width).
	int cySmIcon;		// cy small icon size (height).
	int cxSize;			// Width, in pixels, of a button in a window's caption or title bar.
	int cySize;			// Height, in pixels, of a button in a window's caption or title bar.
	int cxHThumb;		// Width, in pixels, of the thumb box in a horizontal scroll bar.
	int cyVThumb;		// Height, in pixels, of the thumb box in a vertical scroll bar.
	int cyMenuItem;		// Height, in pixels, of single-line menu bar. 
	int nMenuAnimation; // Type of menu animation.
	
	////////////////////////////////////////////////////////////////////
    // Fonts used by Xtreme Toolkit
	////////////////////////////////////////////////////////////////////

	CFont font;				// Default GUI font.
	CFont fontBold;			// Default bold GUI font.
	CFont fontULine;		// Default underlined GUI font.
	CFont fontHCapt;		// Default horizontal caption font.
	CFont fontVCapt;		// Default vertical caption font.
	CFont fontVCaptBold;	// Default vertical caption bold font.
	CFont fontVCaptR;		// Default vertical caption font, right aligned.
	CFont fontVCaptRBold;	// Default vertical caption bold font, right aligned.

	////////////////////////////////////////////////////////////////////
    // Fonts used by Xtreme Toolkit ( face name )
	////////////////////////////////////////////////////////////////////

	CString strHorzFont; // Face name of the horizontal font used by the toolkit.
	CString strVertFont; // Face name of the vertical font used by the toolkit.

	////////////////////////////////////////////////////////////////////
    // OS flags used by Xtreme Toolkit
	////////////////////////////////////////////////////////////////////

	int			iComCtlVersion;			// Common control dll (comctl32.dll) version information.
	BOOL		bWin95;					// TRUE if Win 95.
	BOOL		bWin98;					// TRUE if Win 98.
	BOOL		bWinNT;					// TRUE if Win NT.
	BOOL		bWin2K;					// TRUE if Win 2000.
	BOOL		bWinXP;					// TRUE if Win XP.
	BOOL		bCoolMenuCompatMode;	// TRUE to enable cool menu compatibility mode.
	BOOL		bXPMode;				// TRUE to use XP style menus.
	BOOL		bMenuShadows;			// TRUE to use shadows under the menus.
	BOOL		bToolBarVisualize;		// TRUE to render the toolbar while dragging or resizing.
	BOOL		bControlBarMenus;		// TRUE to display right click control bar menus.
	BOOL		bDockBarMenus;			// TRUE to display right click dockbar menus.
	BOOL		bMenuRecentCommands;	// TRUE to use intelligent menus to hide selected menu commands.
	BOOL		bShowFullAfterDelay;	// TRUE to display hidden menu commands after a short delay.
	BOOL		bToolBarScreenTips;		// TRUE to show tooltips on toolbar commands.
	BOOL		bToolBarAccelTips;		// TRUE to add accelerator keys to toolbar tips.
	BOOL		bDisableFadedIcons;		// TRUE to disable icons fading .
	BOOL		bUseSolidShadows;		// TRUE to use solid shadows in Operation Systems that are not supported layered windows.
	CString		strINIFileName;			// Name of an ini file for registry settings.
	HINSTANCE	hInstance;				// Instance handle for resources.

    // Summary: This member function loads the cursors used by Xtreme Toolkit.
    void LoadSysCursors();

    // Summary: This member function frees cursor resources.
    void FreeSysCursors();

    // Summary: This member function creates the fonts used by Xtreme Toolkit.
    void LoadSysFonts();

    // Summary: This member function frees font resources.
    void FreeSysFonts();

    // Summary: This member function updates system colors used by Xtreme Toolkit.
    void UpdateSysColors(); 

    // Summary: This member function updates system metrics used by Xtreme Toolkit.
    void UpdateSysMetrics();

	// Returns: A DWORD value.
    // Summary:	This member function is called to retrieve the version information
	//			for the common control dll (comctl32.dll). 
    DWORD GetComCtlVersion();

	// Input:	hInst - Instance handle for resources.
    // Summary:	This member function is called by the library to initialize resources.
    void InitResources(HINSTANCE hInst);

	// Input:	pFont - Points to a valid CFont object that is used to define the fonts
	//			the toolkit will use.
	//			pVertFont - Points to a valid CFont object that is used to define the vertical
	//			fonts used by the toolkit.  If NULL, 'pFont' will be used.
	// Example: <pre>CGUI_VisualStudioApp::CGUI_VisualStudioApp()
    //			{
    //			    // TODO: add construction code here,
    //			    m_bFirstTime = true;
    //			
    //			    // Place all significant initialization in InitInstance
    //			    xtAfxData.SetGlobalFont( _T( "Verdana" ) );
    //			}</pre>
	// Summary: Call this member function to set the font the toolkit will use.
	//			This member function should be called from the constructor of your
	//			CWinApp derived class for the fonts to be properly initialized.
    void SetGlobalFont(CFont* pFont,CFont* pVertFont=NULL);

	// Input:	lpszFaceName - Points to a NULL terminated string that is used to define the fonts
	//			the toolkit will use.
	//			lpszVertFaceName - Points to a NULL terminated string that is used to define the vertical
	//			fonts used by the toolkit.  If NULL, the name defined in 'lpszFaceName'
	//			will be used.
    // Example: <pre>CGUI_VisualStudioApp::CGUI_VisualStudioApp()
    //			{
    //			    // TODO: add construction code here,
    //			    m_bFirstTime = true;
    //			
    //			    // Place all significant initialization in InitInstance
    //			    xtAfxData.SetGlobalFont( _T( "Verdana" ) );
    //			}</pre>
	// Summary:	Call this member function to set the font the toolkit will use.
	//			This member function should be called from the constructor of your
	//			CWinApp derived class for the fonts to be properly initialized.
    void SetGlobalFont(LPCTSTR lpszFaceName,LPCTSTR lpszVertFaceName=NULL);

	// Input:	lpszINIFullPath - Full path to the ini file.
    // Summary:	Call this member function to store registry settings in an ini file
    //			instead of the system's registry.
    void SetINIFileName(LPCTSTR lpszINIFullPath);
	
	// Returns: An LPCTSTR data type.
    // Summary:	This member function will get the full path to the ini file
    //			used to store registry settings. 
    LPCTSTR GetINIFileName();

	// Returns: A reference to the one and only XT_AUX_DATA object.
	// Example: <pre>XT_AUX_DATA::Get().InitResources( m_hInstance );</pre>
	// Summary:	This static member function will retrieve a reference to the one
	//			and only XT_AUX_DATA object.  You can use this function to access
	//			data members for the XT_AUX_DATA structure.  You can also use the
	//			macro xtAfxData.
	static XT_AUX_DATA& Get();

	// Input:	bVerNumOnly - true to return the version number only, minus "Xtreme Toolkit v".
	// Returns: A NULL terminated string that indicates the version of the Xtreme Toolkit.
	// Summary:	Call this member function to retrieve a CString object that represents
	//			the current version of the Xtreme Toolkit.  The string returned is
	//			formatted like so: "Xtreme Toolkit v1.94". 
	CString GetXTVersion(bool bVerNumOnly=false);

private:
	bool FontExists(CString& strFaceName);
	bool CreateSysFont(XT_LOGFONT& lf, CFont& font, long lfWeight=FW_NORMAL, char lfUnderline=0, long lfOrientation=0, long lfEscapement=0);
};

//:Associate with "Global Data"

// Summary: XT_AUX_DATA is a singleton object and can only be instantiated
//			one time.  The macro xtAfxData is used to access the members of this
//			structure.
#define xtAfxData XT_AUX_DATA::Get()

//////////////////////////////////////////////////////////////////////
// Summary: CXTIconHandle is a stand alone helper class.  It is used to automatically
//			destroy dynamically created hIcon handles.
class _XT_EXT_CLASS CXTIconHandle
{
public:

    // Summary: Constructs a CXTIconHandle object.
	CXTIconHandle();

	// Input:	hIcon - Handle to a dynamically created icon.
    // Summary:	Constructs a CXTIconHandle object.
    CXTIconHandle(HICON hIcon);

    // Summary: Destroys a CXTIconHandle object, handles cleanup and de-allocation.
    virtual ~CXTIconHandle();

protected:

	HICON m_hIcon;  // Handle to a dynamically created icon.

public:
	
	// Returns: An HICON handle to the icon.
	// Summary:	This operator is used to retrieve a handle to the icon associated with
	//			the CXTIconHandle object. 
    operator HICON() const;

	// Input:	hIcon - Handle to a dynamically created icon.
	// Summary:	This operator is used to initialize the icon associated with the 
	//			CXTIconHandle object.
	CXTIconHandle& operator =(HICON hIcon);

	// Input:	hIcon - Icon handle whose dimensions are to be retrieved.
	// Returns: A CSize object.
	// Summary:	This member function gets the extent of an icon. 
	static CSize GetExtent(HICON hIcon);

	// Returns: A CSize object.
	// Summary: This member function gets the extent of an icon attached to this object.
	CSize GetExtent() const;

	// Input:	hIcon - Icon to be fitted.
	//			desiredExtent - Desired icon extent.
	// Returns: An icon handle.
	// Summary:	This member function scales an icon to fit into a rectangle. The width
	//			and height ration is retained as much as possible. The caller assumes
	//			ownership of the returned icon handle. 
	static HICON ScaleToFit(HICON hIcon,CSize desiredExtent);

	// Input:	desiredExtent - Desired icon extent.
	// Returns: An icon handle.
	// Summary:	This member function scales an icon to fit into a rectangle. The width
	//			and height ration is retained as much as possible.  The caller assumes
	//			ownership of the returned icon handle. 
	HICON ScaleToFit(CSize desiredExtent) const;

};

//////////////////////////////////////////////////////////////////////
// Summary: CXTLoadLibrary is a stand alone utility class.  It is used to load a
//			module (DLL) and free the instance handle upon destruction.  It wraps
//			the LoadLibrary and the FreeLibrary API's.
class _XT_EXT_CLASS CXTLoadLibrary
{
public:
    
	// Input:	lpszModule - Pointer to a null-terminated string that names the .DLL file. The
	//			name specified is the filename of the module and is not related to
	//			the name stored in the library module itself, as specified by the
	//			LIBRARY keyword in the module-definition (.DEF) file. 
    //
			    // If the string specifies a path but the file does not exist in
	//			the specified directory, the function fails.  When specifying a path,
	//			be sure to use backslashes (\), not forward slashes (/). 
    // Summary:	Constructs a CXTLoadLibrary object.
    CXTLoadLibrary(LPCTSTR lpszModule=NULL);

    // Summary: Destroys a CXTLoadLibrary object, handles cleanup and de-allocation.
    virtual ~CXTLoadLibrary();

protected:

	HINSTANCE m_hInstance;  // A handle to the module indicates success.
    
public:

	// Input:	lpszModule - Pointer to a null-terminated string that names the .DLL file. The
	//			name specified is the filename of the module and is not related to
	//			the name stored in the library module itself, as specified by the
	//			LIBRARY keyword in the module-definition (.DEF) file. 
    //
    //			If the module does not exist in the specified directory, the
	//			function will fail.  When specifying a path, be sure to use backslashes
	//			(\), not forward slashes (/). 
    // Summary:	This member function is called to load the library specified by
	//			'lpszModule'.  Once the library is loaded, you can retrieve the instance
	//			handle by using the HINSTANCE operator.
	void LoadLibrary(LPCTSTR lpszModule);
	
	// Returns: A handle to the module if successful, otherwise returns NULL.
    // Summary: This overloaded operator returns a handle to the module indicating
	// success. NULL indicates failure.
    operator HINSTANCE() const;

	// Returns: A DWORD value if successful, otherwise 0L.
	// Summary:	Use this member function to return the version number of the
	//			module attached to this CLoadLibrary object. The high-order
	//			word of the return value represents the major version number and
	//			the low-order word of the returned value represents the minor
	//			version number.  
	DWORD GetModuleVersion();
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE CXTLoadLibrary::operator HINSTANCE() const {
	return m_hInstance;
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTNoFlickerWnd is a BASE_CLASS derived general purpose template helper
//			class. CXTNoFlickerWnd class is used for drawing flicker free controls.
//			To use, instantiate the template using any CWnd derived class. For example,
//			to create a tab control that is flicker free you would use the 
//			following declaration:
//
//			<pre>CXTNoFlickerWnd <CTabCtrl> m_tabCtrl;</pre>
template<class BASE_CLASS>
class CXTNoFlickerWnd : public BASE_CLASS
{
public:

	// Summary: Constructs a CXTNoFlickerWnd object.
	inline CXTNoFlickerWnd() {m_crBack = xtAfxData.clr3DFace;}

	// Input:	crBack - An RGB value.
	// Summary:	This member function is called to set the background fill 
	//			color for the flicker free control.
	inline void SetBackColor(COLORREF crBack) {m_crBack = crBack;}

	// Returns: An RGB value.
	// Summary:	This member function is called to retrieve the background fill color
	//			for the flicker free control. 
	inline COLORREF GetBackColor() {
		return m_crBack; // An RGB value.
	}

protected:

	COLORREF m_crBack; // An RGB value.

	// Input:	message - Specifies the Windows message to be processed.
	//			wParam - Provides additional information used in processing the message. The
	//			parameter value depends on the message.
	//			lParam - Provides additional information used in processing the message. The
	//			parameter value depends on the message.
	// Returns:	The return value depends on the message.
	// Summary:	This method provides a CE procedure (WindowProc) for a CWnd object.
	//			It dispatches messages through the window message map. 
	virtual LRESULT WindowProc(UINT message,WPARAM wParam,LPARAM lParam)
	{
		switch (message)
		{
		case WM_PAINT:
			{
				CPaintDC dc(this);
				
				// Get the client rect, and paint to a memory device context.  This
				// will help reduce screen flicker.  Pass the memory device context to the
				// default window procedure to do default painting.

				CRect r;
				GetClientRect(&r);
				CXTMemDC memDC(&dc, r, m_crBack);
				
				return BASE_CLASS::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
			}
			
		case WM_ERASEBKGND:
			{
				return TRUE;
			}
		}
		
		return BASE_CLASS::WindowProc(message, wParam, lParam);
	}
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTDialogState is a stand alone class. It is an internal class used
//			by toolkit dialogs to manage resource context.
class CXTDialogState
{
protected:

	// Summary: Constructs a CXTDialogState object.
	CXTDialogState();

    // Summary: Destroys a CXTDialogState object, handles cleanup and de-allocation.
	~CXTDialogState();

    // Summary: Restores the application resource context.  Should be used within
    //			the derived class's constructor.
    void RevertResourceContext();

    class CXTManageState *m_pState; // A pointer is used so we don't have to include the definition of the private CXTManageState in a public header
};

#if _MSC_VER < 1200 // MFC 5.0

/////////////////////////////////////////////////////////////////////////////
// CXTString class is for VC5 CString compatibility only.

class _XT_EXT_CLASS CXTString : public CString
{
public:

    CXTString() { }
    CXTString(CString strIn);
    int Find(LPCTSTR lpszSub, int nStart) const;
    int Find( TCHAR ch ) const { return CString::Find(ch); }
    int Find( LPCTSTR lpszSub ) const { return CString::Find(lpszSub); }
    int Insert(int nIndex, TCHAR ch);
    int Insert(int nIndex, LPCTSTR pstr);
    const CString& operator =( const CString& stringSrc ) { return CString::operator =(stringSrc); }
    const CString& operator =( TCHAR ch ) { return CString::operator =(ch); }
    const CString& operator =( const unsigned char* psz ) { return CString::operator =(psz); }
    const CString& operator =( LPCWSTR lpsz ) { return CString::operator =(lpsz); }
    const CString& operator =( LPCSTR lpsz ) { return CString::operator =(lpsz); }
    int Remove(TCHAR chRemove);
	int Replace(TCHAR chOld, TCHAR chNew);
    int Replace(LPCTSTR lpszOld, LPCTSTR lpszNew);
    int Delete(int nIndex, int nCount=1);
    CXTString Left(int nCount) const;
};

#else

typedef CString CXTString;

#endif // MFC 5.0

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTGLOBALS_H__)
