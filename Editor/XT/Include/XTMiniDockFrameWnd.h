// XTMiniDockFrameWnd.h interface for the CXTMiniDockFrameWnd class.
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

#if !defined(__XTMINIDOCKFRAMEWND_H__)
#define __XTMINIDOCKFRAMEWND_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// forwards

class CXTDockBar;
class CXTControlBar;
class CXTDockContext;
class CXTFrameButton;
class CXTExpMenuWnd;

//////////////////////////////////////////////////////////////////////
// Summary:	CXTMiniDockFrameWnd is a CMiniDockFrameWnd class.  It is used to
//          handle docking for the CXTDockWindow class.
class _XT_EXT_CLASS CXTMiniDockFrameWnd : public CMiniDockFrameWnd
{
	DECLARE_DYNCREATE(CXTMiniDockFrameWnd)

public:

    // Summary:	Constructs a CXTMiniDockFrameWnd object.
	CXTMiniDockFrameWnd();

	// Summary:	Destroys a CXTMiniDockFrameWnd object, handles cleanup and de-allocation.
    virtual ~CXTMiniDockFrameWnd();

protected:

    // Summary: Appearance as most recently rendered
    enum APPEARANCE_STYLE
    {
        APPEARANCE_UNDEFINED,   // Undefined style
        APPEARANCE_XPSTYLE,     // Office xp style.
        APPEARANCE_CLASSIC,     // Classic Windows style.
	} m_currentAppearance;

	// Returns:	An APPEARANCE_STYLE enumeration.
	// Summary:	Prescribed current appearance
	static APPEARANCE_STYLE GetCurrentAppearance();  

	bool			m_bInitCompleted;	// Tells if initial update completed
	CPtrList        m_arrButtons;       // Frame button array
	CImageList      m_imageList;		// Image list used by mini-frame buttons.
	CToolTipCtrl    m_toolTip;          // Button tooltip control.
	CXTExpMenuWnd*  m_pPopupWnd;        // Customize popup window.
	CXTFrameButton* m_pPressedBtn;      // Points to the active frame button.
	
    // Ignore:
	//{{AFX_VIRTUAL(CXTMiniDockFrameWnd)
	public:
	virtual BOOL Create(CWnd* pParent, DWORD dwBarStyle);
	//}}AFX_VIRTUAL

	virtual bool IsValidBar(CControlBar* pBar) const;
    virtual bool IsControlBar(CControlBar* pBar) const;
    virtual bool IsToolbar(CControlBar* pBar) const;
    virtual bool IsMenubar(CControlBar* pBar) const;
    virtual bool IsDockWindow(CControlBar* pBar) const;
    virtual CXTDockBar* GetXTDockBar() const;
    virtual CXTControlBar* GetXTControlBar() const;
    virtual CXTDockContext* GetXTDockContext(CControlBar* pBar) const;
	virtual void DrawXPFrameControl(CDC* pDC, CRect& r, CXTFrameButton* pFrameButton);
	virtual void ActivateToolTips(CPoint point, bool bIsCloseButton);
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);

    // Ignore:
	//{{AFX_MSG(CXTMiniDockFrameWnd)
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnCustomize();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG

    afx_msg LRESULT OnInitMiniFrame(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM, LPARAM);
    afx_msg void OnPopupClosed();

	DECLARE_MESSAGE_MAP()

	friend class CDockBar;
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTMINIDOCKFRAMEWND_H__)