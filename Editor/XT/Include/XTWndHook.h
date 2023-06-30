// XTWndHook.h interface for the CXTWndHook class.
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

#if !defined(__XTWNDHOOK_H__)
#define __XTWNDHOOK_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTWndHook is a CWnd derived class.  It is used to hook a CWnd object
//			in order to intercept and act upon window messages that are received.
class _XT_EXT_CLASS CXTWndHook : public CObject
{
    DECLARE_DYNAMIC(CXTWndHook)

public:

    // Summary: Constructs a CXTWndHook object.
    CXTWndHook();

    // Summary: Destroys a CXTWndHook object, handles cleanup and de-allocation.
    virtual ~CXTWndHook();

protected:

	HWND		m_hWnd;			// The window hooked.
	WNDPROC		m_pOldWndProc;	// The original window procedure.
	CXTWndHook* m_pNextHook;	// Next, in chain of hooks, for this window.

public:

	// Returns: TRUE if the window has been hooked, otherwise returns FALSE.
    // Summary:	This member function will check to see if the current window has
	//			been hooked. 
    virtual BOOL IsWindowHooked();

	// Input:	pWnd - Points to a CWnd object that represents the window to hook.
	//			If set to NULL, the window hook is removed.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function will hook a window so that its messages are
	//			intercepted before they are passed on to the specified window. 
    virtual BOOL HookWindow(CWnd* pWnd);

	// Input:	hWnd - A handle to a window that represents the window whose window
	//			procedure is to be hooked.  If set to NULL, the WNDPROC is removed.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function will install a new window procedure that directs
	//			messages to the CXTWndHook. 
    virtual BOOL HookWindow(HWND hWnd);

	// Input:	message - Specifies the Windows message to be processed.
	//			wParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	//			lParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	// Returns: An LRESULT value. The return value depends on the message.
    // Summary:	This member function provides a Windows procedure (WindowProc) for
	//			a CXTWndHook object.  It dispatches messages through the window's message
	//			map. 
    virtual LRESULT WindowProc(UINT message,WPARAM wParam,LPARAM lParam);

	// Returns: An LRESULT value. The return value depends on the message.
    // Summary:	This member function calls the default window procedure.  The default
	//			window procedure provides default processing for any window message
	//			that an application does not process.  This member function ensures
	//			that every message is processed. 
    LRESULT Default();

    friend class CXTWindowMap;
};

// class forwards

class CXTMenuBar;

//////////////////////////////////////////////////////////////////////
// Summary: CXTMBarWndHook is a CXTWndHook derived class.  CXTMBarWndHook is used
//			to trap menu related messages sent to the owning frame.  The same class
//			is also used to trap messages sent to the MDI client window in an MDI
//			app.  I should really use two classes for this, but it uses less code
//			to share the same class.  Note, however, that there are two different
//			INSTANCES of CXTMBarWndHook in CXTMenuBar: one for the frame, and one
//			for the MDI client window.
class _XT_EXT_CLASS CXTMBarWndHook : public CXTWndHook
{
    DECLARE_DYNAMIC(CXTMBarWndHook)

public:

    // Summary: Constructs a CXTMBarWndHook object.
    CXTMBarWndHook();

    // Summary: Destroys a CXTMBarWndHook object, handles cleanup and de-allocation.
    virtual ~CXTMBarWndHook();

protected:

	CXTMenuBar* m_pMenuBar;  // Points to the owner menu bar.

public:

	// Input:	pMenuBar - Pointer to the owner menu bar.
	//			hWndToHook - Handle to the frame window.
	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function will install a new window procedure that directs
	//			messages to the CXTWndHook. 
    virtual BOOL Install(CXTMenuBar* pMenuBar,HWND hWndToHook);

	// Input:	message - Specifies the Windows message to be processed.
	//			wParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	//			lParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	// Returns: An LRESULT value. The return value depends on the message.
    // Summary:	This member function provides a Windows procedure (WindowProc) for
	//			a CXTWndHook object.  It dispatches messages through the window's message
	//			map. 
    virtual LRESULT WindowProc(UINT message,WPARAM wParam,LPARAM lParam);

   friend class CXTMenuBar;
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTMBarMDIWndHook is a CXTMBarWndHook derived class. CXTMBarWndHook
//			is used to trap menu related messages sent to the owning frame.  The
//			same class is also used to trap messages sent to the MDI client window
//			in an MDI app.  I should really use two classes for this, but it uses
//			less code to share the same class.  Note, however, that there are two
//			different INSTANCES of CXTMBarWndHook in CXTMenuBar; one for the frame,
//			and one for the MDI client window.
class _XT_EXT_CLASS CXTMBarMDIWndHook : public CXTMBarWndHook
{
    DECLARE_DYNAMIC(CXTMBarMDIWndHook)

public:
	
	// Input:	message - Specifies the Windows message to be processed.
	//			wParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	//			lParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	// Returns: An LRESULT value. The return value depends on the message.
	// Summary:	This member function provides a Windows procedure (WindowProc) for
	//			a CXTWndHook object. It dispatches messages through the window's message
	//			map. 
    virtual LRESULT WindowProc(UINT message,WPARAM wParam,LPARAM lParam);

	friend class CXTMenuBar;
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTWindowMap is a CMapPtrToPtr derived class.  It is used to create
//			a CXTWindowMap class object.
class _XT_EXT_CLASS CXTWindowMap : private CMapPtrToPtr
{
    DECLARE_DYNAMIC(CXTWindowMap)

public:

    // Summary: Constructs a CXTWindowMap object.
    CXTWindowMap();

    // Summary: Destroys a CXTWindowMap object, handles cleanup and de-allocation.
    virtual ~CXTWindowMap();

	// Input:	hWnd - Valid window handle.
	//			pXTWndHook - A pointer to a CXTWndHook object.
    // Summary:	This member function adds a hook to the map, that is, it associates
	//			the hook with the window.
    void Add(HWND hWnd,CXTWndHook* pXTWndHook);

	// Input:	pXTWndHook - A pointer to a CXTWndHook object.
    // Summary:	This member function removes a hook from the map.
    void Remove(CXTWndHook* pXTWndHook);

	// Input:	hWnd - Valid window handle.
    // Summary:	This member function removes all the hooks for a window.
    void RemoveAll(HWND hWnd);

	// Input:	hWnd - Valid window handle.
	//			message - Specifies the Windows message to be processed.
	//			wParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	//			lParam - Provides additional information used in processing the message.
	//			The parameter value depends on the message.
	// Returns: An LRESULT object
    // Summary:	This member function is a subclassed window procedure for message
	//			hooks, and replaces AfxWndProc, or whatever else was there before.
    static LRESULT CALLBACK HookWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

	// Input:	hWnd - Valid window handle.
	// Returns: A CXTWndHook pointer.
    // Summary:	This member function finds the first hook associated with the window.
    CXTWndHook* Lookup(HWND hWnd);
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE BOOL CXTWndHook::IsWindowHooked() {
    return (m_hWnd != NULL);
}
AFX_INLINE BOOL CXTWndHook::HookWindow(CWnd* pWnd) {
    return HookWindow(pWnd->GetSafeHwnd());
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTWNDHOOK_H__)