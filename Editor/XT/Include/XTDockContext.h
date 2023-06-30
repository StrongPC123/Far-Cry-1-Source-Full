// XTDockContext.h interface for the CXTDockContext class.
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

#if !defined(__XTDOCKCONTEXT_H__)
#define __XTDOCKCONTEXT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// class forwards

class CXTDockBar;
class CXTDockWindow;

//////////////////////////////////////////////////////////////////////
// Summary: CXTDockContext is a CDockContext derived class.  It is used by CXTDockWindow
//			and CXTMiniDockFrameWnd for docking and sizing control bars.
class _XT_EXT_CLASS CXTDockContext : public CDockContext
{
public:
	// Listeners for drag state changes.

	typedef CXTCallbacks<CXTDockContext*, 4> LISTENERS;
	typedef LISTENERS::NOTIFYCB LISTENERCB;
	
public:

	// Input:	pBar - Pointer to a CControlBar object.
    // Summary:	Constructs a CXTDockContext object.
    CXTDockContext(CControlBar* pBar);

    // Summary: Destroys a CXTDockContext object, handles cleanup and de-allocation.
    virtual ~CXTDockContext();

protected:

    bool                                 m_bFullWindowDrag;        // Tells if full window drag is shall be executed.
    bool                                 m_bRecalcPending;
    bool                                 m_bTracking;              // Tells if the context currently tracking mouse during drag/resize operation
    CPoint                               m_ptOrig;                 // Cursor position.
    LISTENERS                            m_trackStateListeners;    // Listeners for tracking state change notifications
    CTypedPtrArray<CObArray, CFrameWnd*> m_arrFramesPendingRecalc;

public:

	// Returns: true if it is tracking a mouse, otherwise it returns false.
	// Summary:	This member function tells if this object is currently tracking a mouse
	//			during a drag or resize operation. 
	bool IsTracking() const;

    // Summary: This member function is called by CXTDockContext when a drag operation
	//			completes.
    void EndDrag();

	// Input:	pt - CPoint object that represents the current cursor location.
    // Summary:	This member function is called by CXTDockWindow and CXTMiniDockFrameWnd
	//			whenever a drag operation begins.
    virtual void StartDrag(CPoint pt);

	// Returns: A DWORD value.
    // Summary: This member function is called by CXTDockContext to determine whether
	//			a control bar is dockable. 
    DWORD CanDock();

	// Input:	rect - Size of the control bar.
	//			dwDockStyle - Control bar style.
	//			ppDockBar - Points to a CDockBar pointer.
	// Returns: A DWORD value.
    // Summary:	This member function is called by CXTDockContext to determine whether
	//			a control bar is dockable. 
    DWORD CanDock(CRect rect,DWORD dwDockStyle,CDockBar** ppDockBar = NULL);

	// Returns: TRUE if successful, otherwise returns FALSE.
    // Summary:	This member function is called by CXTDockContext whenever a drag
	//			operation begins. 
    BOOL Track();

	// Input:	dwOverDockStyle - Control bar style.
	// Returns: A CXTDockBar pointer.
    // Summary:	This member function is called by CXTDockContext to get a pointer
	//			to a CXTDockBar object. 
    CXTDockBar* GetDockBar(DWORD dwOverDockStyle);

    // Summary: This member function is used to toggle the docked state of a control
	//			bar.
    virtual void ToggleDocking();

	// Input:	pt - CPoint object that represents the current cursor location.
    // Summary:	This member function is called to enable diagonal sizing for 
    //			the control bar.
    void Stretch(CPoint pt);

	// Input:	listener - A pointer to a CObject object.
	//			cb - LISTENERCB object.
	// Summary:	This member function adds a listener for drag state change notifications.
	void AddTrackStateListener(CObject* listener, LISTENERCB cb);

	// Input:	listener - A pointer to a CObject object.
	// Summary:	This member function removes a listener.
	void RemoveListener(CObject* listener);

    void DeferRecalcLayout(CFrameWnd* pFrame);    
	virtual void StartResize(int nHitTest, CPoint pt);
    void Move(CPoint pt);
    void DrawFocusRect(BOOL bRemoveRect = FALSE);
    void OnKey(int nChar, BOOL bDown);
    void UpdateState(BOOL* pFlag, BOOL bNewValue);
	bool CanDock(CFrameWnd* pFrame, DWORD& dwResult, CRect rect, DWORD dwDockStyle, CDockBar** ppDockBar);
	void FlushRecalcLayout(CFrameWnd* pFrame);
	virtual CFrameWnd* GetAlternateSite();

private:
    void InitLoop();
	void CancelLoop();
    void EndResize();

    void AdjustRectangle(CRect& rect, CPoint pt);    
    void FloatControlBar(DWORD dwFloatStyle);
    friend class CXTControlBar;
    friend class CXTTrackDockContextCleanup;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE bool CXTDockContext::IsTracking() const {
	return m_bTracking;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTDOCKCONTEXT_H__)