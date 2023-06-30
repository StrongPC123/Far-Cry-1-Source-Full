// XTPopupTearOffWnd.h : header file
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

#if !defined(__XTPOPUPTEAROFFWND_H__)
#define __XTPOPUPTEAROFFWND_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXTToolBar;
class CXTWndHook;
class CXTToolbarTearOff;

/////////////////////////////////////////////////////////////////////////////
// Summary: CXTPopupTearOffWnd is a CWnd derived class window .  It is used to display a
//			"tear-off" popup window to be activated when the user presses the down arrow on
//			a toolbar
class _XT_EXT_CLASS CXTPopupTearOffWnd : public CWnd
{
public:

	// Input:	iCmdID - Command ID that is currently active for the popup window.
    // Summary:	Constructs a CXTPopupTearOffWnd object.
	CXTPopupTearOffWnd(int iCmdID);

	// Summary: Destroys a CXTPopupTearOffWnd object, handles cleanup and de-allocation.
	virtual ~CXTPopupTearOffWnd();

// Attributes
protected:
	
	int			m_iCmdID;		// Command ID of the button that created this window.
	BOOL		m_bTearOff;		// If window is tear-off
	CWnd*		m_pChild;		// Child window
	CRect		m_rcExclude;	// Exclusion rectangle for drawing adjacent borders.
	CXTToolBar* m_pWndParent;	// Pointer to the parent toolbar.
	CXTWndHook* m_pHook;		// Message hook.

private:	

	BOOL m_bPressed;
	BOOL m_bHighlighted;
	CXTControlBar* m_pControlBar;
	
// Operations
public:

	// Input:	pParent - Points to a valid parent toolbar.
	//			pControlBar - Points to a valid child controlbar.		
	// Summary:	Call this member function to create a CXTToolBarPopupWnd object.  
	virtual BOOL Create(CXTToolBar* pParent,CXTControlBar* pControlBar);

	// Ignore:
	//{{AFX_VIRTUAL(CXTPopupTearOffWnd)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

protected:
	BOOL PtInTearOffRect(CPoint point);
	virtual void OnHookedCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnStartDrag(CPoint point);
	BOOL _Create(CXTToolBar* pParent, CWnd* pChild, CXTControlBar* pControlBar);
	virtual CWnd* CloneChild(CXTControlBar* pControlBar) = 0;
			
protected:

	// Ignore:
	//{{AFX_MSG(CXTPopupTearOffWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) ;
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	//}}AFX_MSG
	
	class CXTPopupTearOffWndWndHook;
	friend class CXTPopupTearOffWndWndHook;
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Summary: CXTPopupColorTearOff is a CXTPopupTearOffWnd derived class window .  It is used to display a
//			"tear-off" popup window with Color Selector as a child
class _XT_EXT_CLASS CXTPopupColorTearOff : public CXTPopupTearOffWnd
{
public:

	// Input:	iCmdID - Command ID that is currently active for the popup window.
    // Summary:	Constructs a CXTPopupToolbarTearOff object.
	CXTPopupColorTearOff(int iCmdID);
	
protected:

	CWnd* CloneChild(CXTControlBar* pControlBar);

	// Ignore:
	//{{AFX_MSG(CXTPopupColorTearOff)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnSelEndOK(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
		
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Summary: CXTPopupToolbarTearOff is a CXTPopupTearOffWnd derived class window .  It is used to 
//			display a "tear-off" popup window with Toolbar as a child
class _XT_EXT_CLASS CXTPopupToolbarTearOff : public CXTPopupTearOffWnd
{
public:

	// Input:	iCmdID - Command ID that is currently active for the popup window.
    // Summary:	Constructs a CXTPopupToolbarTearOff object.
	CXTPopupToolbarTearOff(int& iCmdID);

	// Input:	pParent - Points to a valid parent toolbar.
	//			pControlBar - Points to a valid child toolbar.
	//			iNumCols - Number of columns to display when the toolbar is displayed.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this member function to create a CXTPopupToolbarTearOff object.  
	virtual BOOL Create(CXTToolBar* pParent,CXTToolBar* pControlBar,int iNumCols=5);

private:	
	int& m_iCmd;
	void OnHookedCommand(WPARAM wParam, LPARAM lParam);
	CWnd* CloneChild(CXTControlBar* pControlBar);
	
protected:

	// Ignore:
	//{{AFX_MSG(CXTPopupToolbarTearOff)
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //__XTPOPUPTEAROFFWND_H__