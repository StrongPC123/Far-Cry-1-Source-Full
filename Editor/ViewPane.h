#if !defined(AFX_VIEWPANE_H__034B5C96_6A7E_4640_B142_4D2EAA760D3F__INCLUDED_)
#define AFX_VIEWPANE_H__034B5C96_6A7E_4640_B142_4D2EAA760D3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewPane.h : header file
//

#include "Viewport.h"

/////////////////////////////////////////////////////////////////////////////
// CViewPane view

class CViewPane : public CView
{
public:
	CViewPane();
	virtual ~CViewPane();

	DECLARE_DYNCREATE(CViewPane)

// Operations
public:
	// Set get this pane id.
	void SetId( int id ) { m_id = id; }
	int GetId() { return m_id; }

	//! Assign viewport to this pane.
	void AssignViewport( EViewportType type );
	
	//! Assign viewport to this pane.
	void SwapViewports( CViewPane *pView );
	//! Detach viewport from this pane.
	void DetachViewport();

	void SetFullscren( bool f );
	bool IsFullscreen() const { return m_bFullscreen; };

	void SetFullscreenViewport( bool b );

	CViewport* GetViewport() { return m_viewport; };

	//////////////////////////////////////////////////////////////////////////
	bool IsActiveView() const { return m_active; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewPane)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewPane)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void DrawView( CDC &dc );

private:
	void SetViewport( CViewport *pViewport );

	EViewportType m_viewportType;
	bool m_bFullscreen;

	int m_id;
	int m_nBorder;

	int m_titleHeight;

	CViewport* m_viewport;
	//CToolBar m_toolBar;
	bool m_active;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWPANE_H__034B5C96_6A7E_4640_B142_4D2EAA760D3F__INCLUDED_)
