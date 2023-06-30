// XTTrayIcon.h: interface for the CXTTrayIcon class.
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

#if !defined(__XTTRAYICON_H__)
#define __XTTRAYICON_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: XT_TRAYICON is a stand alone helper structure.  It is used by CXTTrayIcon
//			to maintain an animation icon list.
struct XT_TRAYICON
{
	HICON	hIcon;		// Handle to the icon displayed.
	CString strToolTip; // A NULL terminated string that represents the tooltip displayed for the tray icon.
};

// Summary: Animated icon array.
typedef CList<XT_TRAYICON, XT_TRAYICON&> CXTTrayIconList;

//////////////////////////////////////////////////////////////////////
// Summary: CXTTrayIcon is a CWnd derived class of the NOTIFYICONDATA structure.
//			It is used to display a system tray icon with animation.
class _XT_EXT_CLASS CXTTrayIcon : public CWnd
{
public:

	// Summary: Constructs a CXTTrayIcon object.
    CXTTrayIcon();

    // Summary: Destroys a CXTTrayIcon object, handles cleanup and de-allocation.
    virtual ~CXTTrayIcon();

protected:

	UINT			m_uFlags;			 // Style settings for icon restore.
	UINT			m_nIconID;			 // Resource ID for the default icon.
	UINT			m_nIDEvent;			 // Timer event ID.
	UINT			m_nCounter;			 // Holds the current position in the timer loop.
	UINT			m_uDefMenuItemID;	 // Default popup menu item ID.
	bool			m_bDefMenuItemByPos; // Determines if the default menu item is a command or index.
	bool			m_bHidden;			 // State of the icon. true to indicate the icon is hidden.
	bool			m_bRemoved;			 // true if the icon has been removed from the system tray.
	bool			m_bShowPending;		 // true if the icon display is pending.
	HWND			m_hWndNotify;		 // Handle to the window that receives command notification.
	CWnd			m_wndMinimize;		 // Hidden window used during minimize and restore functions.
	size_t			m_iMaxTipSize;		 // Maximum size for tooltip string.
	CString			m_strToolTip;		 // Tooltip for the default icon.
	CXTIconHandle	m_hIcon;			 // Default icon.
	NOTIFYICONDATA	m_NIData;			 // Tray icon structure.
	CXTTrayIconList m_arTrayIcons;		 // Array of icons and text that are displayed during animation.

public:
	
	// Input:	lpszCaption - Default tooltip text to display for the icon.
	//			pParentWnd - Pointer to the window that will receive notification messages
	//			associated with an icon in the taskbar status area.
	//			nIconID - Resource ID for the default tray icon.
	//			uMenuID - Popup menu resource identifier.
	//			uDefMenuItemID - Command ID that represents the default item for the menu.
	//			bDefMenuItemByPos - true if the default is defined by its position. false if it is
	//			defined by its command ID.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function will create the system tray icon. 
    virtual bool Create(LPCTSTR lpszCaption,CWnd* pParentWnd,UINT nIconID,UINT uMenuID=0,UINT uDefMenuItemID=0,bool bDefMenuItemByPos=false);

	// BULLETED LIST:

	// Input:	lpszInfo - Pointer to a null-terminated string with the text for a balloon
	//			ToolTip.  It can have a maximum of 255 characters.
	//			lpszInfoTitle - Pointer to a null-terminated string containing a title for a
	//			balloon ToolTip.  This title appears in boldface above the text. 
	//			It can have a maximum of 63 characters.
	//			dwInfoFlag - Flags that can be set to add an icon to a balloon ToolTip. 
	//			It is placed to the left of the title.  If the 'lpszInfoTitle' member
	//			is zero-length, the icon is not shown, can be any of the following:
    //			[ul]
    //			[li]<b>NIIF_ERROR</b> An error icon.[/li]
    //			[li]<b>NIIF_INFO</b> An information icon.[/li]
    //			[li]<b>NIIF_NONE</b> No icon.[/li]
    //			[li]<b>NIIF_WARNING</b> A warning icon.[/li]
    //			[/ul]
	//			uTimeout - The timeout value, in milliseconds, for a balloon ToolTip. 
	//			The system enforces minimum and maximum timeout values. 'uTimeout'
	//			values that are too large are set to the maximum value, and values
	//			that are too small default to the minimum value. The system minimum
	//			and maximum timeout values are currently set at 10 seconds and 30
	//			seconds, respectively.
	// Returns: true if successful, otherwise returns false.
    // Summary:	Call this member function to display a balloon tooltip for the system
	//			tray icon. Must have Windows 2000 or later.  
    bool ShowBalloonTip(LPCTSTR lpszInfo=NULL,LPCTSTR lpszInfoTitle=NULL, DWORD dwInfoFlags=NIIF_NONE,UINT uTimeout=10);

    // Summary: This member function will stop the tray icon animation timer.
    virtual void StopAnimation();

	// Input:	uElapse - Specifies the time-out value, in milliseconds, between frames.
    // Summary:	This member function will start the tray icon animation timer.
    virtual void StartAnimation(UINT uElapse=500);

	// Input:	lpIDArray - An array of resource IDs that represent the icons to display
	//			in the caption area.
	//			nIDCount - Size of the array 'lpStrTipArray'.
	//			lpStrTipArray - An array of tooltips that match the icons passed in as 'lpStrTipArray'.
	//			If NULL, the default tooltip will be used.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function will set the icons used for animated system
	//			tray icons.  To use, call SetAnimationIcons, then SetTimer.  
    virtual bool SetAnimationIcons(const UINT* lpIDArray,int nIDCount,const CString* lpStrTipArray=NULL);

	// Input:	lpszTipText - Null terminated string that represents the tooltip text to display
	//			for the icon.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to set the tooltip text for the tray icon.
    bool SetTooltipText(LPCTSTR lpszTipText);

	// Input:	nTipText - Windows string resource ID that represents the tooltip text
	//			to display for the icon.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to set the tooltip text for the tray icon.
	bool SetTooltipText(UINT nTipText);

	// Returns: A CString object that represents the tooltip text.
	// Summary:	Call this member function to retrieve the tooltip text that is displayed
	//			by the tray icon. 
	CString GetTooltipText() const;

	// Summary: This member function is called to reset the tray icon control settings
	//			to their default value.
	void SetDefaultValues();

	// Input:	uNewCallbackMessage - Notification message ID.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function will set the callback message to be used by the
	//			tray icon. 
	bool SetCallbackMessage(UINT uNewCallbackMessage);

	// Returns: An ID to a notification message.
	// Summary:	This member function will retrieve the notification message ID used
	//			by the tray icon. 
	UINT GetCallbackMessage();

	// Input:	uItem - Command identifier or position of the default menu item.
	//			bByPos - true if 'uItem' is the menu item's index, false if it is the menu
	//			item's command ID.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function will set the default menu item for the tray icon's
	//			popup menu if the tray icon handles notification messages. 
	bool SetDefaultMenuItem(UINT uItem,bool bByPos);

	// Input:	uItem - Reference to a UINT that is to receive the default menu item ID.
	//			bByPos - Reference to a bool that is to receive the default menu item position
	//			flag.
	// Summary:	Call this member function to retrieve the item ID and position flag
	//			for the tray icon.
	void GetDefaultMenuItem(UINT& uItem,bool& bByPos);

	// Input:	pWndNotify - Points to a valid CWnd object.
	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to set the window that is to receive notification
	//			messages from the tray icon. 
	bool SetNotificationWnd(CWnd* pWndNotify);

	// Returns: A pointer to a valid CWnd object.
	// Summary:	Call this member function to retrieve a pointer to the window that
	//			receives notification messages from the tray icon. 
	CWnd* GetNotificationWnd();

	// Input:	hIcon - Handle to the icon to display.
	// Returns: true if the icon was set, otherwise returns false.
	// Summary:	Call this member function to set the icon that is displayed in the
	//			system tray. 
	bool SetIcon(HICON hIcon);

	// Input:	lpszIconName - Resource name of the icon to display.
	// Returns: true if the icon was set, otherwise returns false.
	// Summary:	Call this member function to set the icon that is displayed in the
	//			system tray. 
	bool SetIcon(LPCTSTR lpszIconName);

	// Input:	nIDResource - Resource identifier of the icon to display.
	// Returns: true if the icon was set, otherwise returns false.
	// Summary:	Call this member function to set the icon that is displayed in the
	//			system tray. 
	bool SetIcon(UINT nIDResource);

	// Returns: A handle to the icon displayed in the system tray.
	// Summary:	Call this member function to return a handle to the icon displayed
	//			in the system tray. 
	HICON GetIcon() const;

	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to add the icon to the system tray. 
	bool AddIcon();

	// Returns: true if successful, otherwise returns false.
	// Summary:	Call this member function to remove the icon from the system tray.
	bool RemoveIcon();

	// Returns: true if successful, otherwise returns false.
    // Summary:	Call this member function to hide the system tray icon, in Windows
	//			2000 or greater.  
    bool HideIcon();

	// Returns: true if successful, otherwise returns false.
    // Summary:	Call this member function to show the system tray icon, in Windows
	//			2000 or greater.  
    bool ShowIcon();

	// Input:	pWnd - Window to minimize.
	// Summary:	Call this member function to minimize the specified window to the system
	//			tray.  If window animation is supported, the window will glide down
	//			to the system tray.
	void MinimizeToTray(CWnd* pWnd);

	// Input:	pWnd - Window to maximize.
	// Summary:	Call this member function to maximize the specified window from the
	//			system tray.  If window animation is supported, the window will glide
	//			up from the system tray.
	void MaximizeFromTray(CWnd* pWnd);

    // Ignore:
	//{{AFX_VIRTUAL(CXTTrayIcon)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

    virtual void InstallIconPending();
	virtual bool CreateMinimizeWnd(CWnd* pWndApp);
	virtual void RemoveAnimationIcons();

	// Ignore:
	//{{AFX_MSG(CXTTrayIcon)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
    afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

public:
	// OBSOLETE

	virtual bool Create(LPCTSTR lpszCaption, DWORD dwStyle, CWnd* pParentWnd, UINT nIconID);
    virtual void SetTooltip(CString strToolTip);
    virtual void SetTrayIcon(UINT nIcon, DWORD dwMessage=NIM_ADD);
    virtual void SetAnimationIcons(const UINT* lpIDArray,const CString* lpStrTipArray,int nIDCount);
    virtual void KillTimer();
    virtual void SetTimer(UINT nIDEvent, UINT uElapse);
	// END OBSOLETE

};

//////////////////////////////////////////////////////////////////////

// OBSOLETE

AFX_INLINE bool CXTTrayIcon::Create(LPCTSTR lpszCaption, DWORD dwStyle, CWnd* pParentWnd, UINT nIconID) {
	UNREFERENCED_PARAMETER(dwStyle); return Create(lpszCaption, pParentWnd, nIconID);
}
AFX_INLINE void CXTTrayIcon::SetTooltip(CString strToolTip) {
	SetTooltipText(strToolTip);
}
AFX_INLINE void CXTTrayIcon::SetTrayIcon(UINT nIcon, DWORD dwMessage) {
	UNREFERENCED_PARAMETER(dwMessage); SetIcon(nIcon);
}
AFX_INLINE void CXTTrayIcon::SetAnimationIcons(const UINT* lpIDArray,const CString* lpStrTipArray,int nIDCount) {
	SetAnimationIcons(lpIDArray, nIDCount, lpStrTipArray);
}
AFX_INLINE void CXTTrayIcon::KillTimer() {
	StopAnimation();
}
AFX_INLINE void CXTTrayIcon::SetTimer(UINT nIDEvent, UINT uElapse) {
	m_nIDEvent = nIDEvent; StartAnimation(uElapse);
}
// END OBSOLETE

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__TRAYICON_H__)