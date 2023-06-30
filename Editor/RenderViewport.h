#if !defined(AFX_RENDERVIEWPORT_H__323772B6_5A57_4867_B973_A9102FE3001B__INCLUDED_)
#define AFX_RENDERVIEWPORT_H__323772B6_5A57_4867_B973_A9102FE3001B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenderViewport.h : header file
//

#include "Viewport.h"

// forward declarations.
class CBaseObject;

/////////////////////////////////////////////////////////////////////////////
// CRenderViewport window

class CRenderViewport : public CViewport
{
	DECLARE_DYNCREATE(CRenderViewport)
// Construction
public:
	CRenderViewport();

	/** Get type of this viewport.
	*/
	virtual EViewportType GetType() { return ET_ViewportCamera; }
	virtual void SetType( EViewportType type ) { assert(type == ET_ViewportCamera); };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenderViewport)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRenderViewport();

	virtual void Update();

	virtual void OnTitleMenu( CMenu &menu );
	virtual void OnTitleMenuCommand( int id );

	void	SetCamera( const CCamera &camera );

	//! Map world space position to viewport position.
	virtual CPoint	WorldToView( Vec3 wp );
	//! Map viewport position to world space position.
	virtual Vec3		ViewToWorld( CPoint vp,bool *collideWithTerrain=0,bool onlyTerrain=false );
	virtual void		ViewToWorldRay( CPoint vp,Vec3 &raySrc,Vec3 &rayDir );
	virtual float		GetScreenScaleFactor( const Vec3 &worldPoint );

	virtual void DrawTextLabel( DisplayContext &dc,const Vec3& pos,float size,const CFColor& color,const char *text );
	virtual bool HitTest( CPoint point,ObjectHitInfo &hitInfo,int flags=0 );
	virtual bool IsBoundsVisible( const BBox &box ) const;
	virtual void CenterOnSelection();

protected:
	// Called to render stuff.
	virtual void OnRender();
	
	virtual void SetViewerPos( const Vec3 &pos );
	virtual void SetViewerAngles( const Vec3 &angles );
	virtual Vec3 GetViewerPos() const;
	virtual Vec3 GetViewerAngles() const;

	//! Get currently active camera object.
	CBaseObject* GetCameraObject() const;
	void SetCameraObject( CBaseObject *cameraObject );

	void RenderTerrainGrid( float x1,float y1,float x2,float y2 );
	void RenderMarker();
	void RenderCursorString();
	void RenderSafeFrame();
	void ProcessKeys();

protected:
	void RenderAll();
	void DrawAxis();

	virtual bool CreateRenderContext();
	virtual void DestroyRenderContext();

	//! Assigned renderer.
	IRenderer*	m_renderer;
	I3DEngine*	m_engine;
	ICryCharManager* m_pAnimationSystem;
	bool m_bRenderContextCreated;
	bool m_bInRotateMode;
	bool m_bInMoveMode;
	CPoint m_mousePos;

	float m_moveSpeed;

	// Camera field of view.
	//float m_cameraFov;
	CCamera m_camera;

	// Render options.
	bool m_bWireframe;
	bool m_bDisplayLabels;
	bool m_bShowSafeFrame;

	CSize m_viewSize;
	CRect m_rcClient;

	// Index of camera objects.
	mutable GUID m_cameraObjectId;
	bool m_bSequenceCamera;
	bool m_bUpdating;

  int m_nPresedKeyState;

	// Generated message map functions
protected:
	//{{AFX_MSG(CRenderViewport)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg void OnSwitchcameraDefaultcamera();
	afx_msg void OnSwitchcameraSequencecamera();
	afx_msg void OnSwitchcameraSelectedcamera();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERVIEWPORT_H__323772B6_5A57_4867_B973_A9102FE3001B__INCLUDED_)
