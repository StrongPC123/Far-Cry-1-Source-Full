// Viewport.h: interface for the CViewport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITWND_H__DB6E5168_5AA2_439C_A986_DDD440C82BA3__INCLUDED_)
#define AFX_EDITWND_H__DB6E5168_5AA2_439C_A986_DDD440C82BA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forward declarations.
class CBaseObject;
struct DisplayContext;
struct ObjectHitInfo;
class CCryEditDoc;
class CViewManager;

/** Type of viewport.
*/
enum EViewportType
{
	ET_ViewportUnknown = 0,
	ET_ViewportXY,
	ET_ViewportXZ,
	ET_ViewportYZ,
	ET_ViewportCamera,
	ET_ViewportMap,
	ET_ViewportModel,
	ET_ViewportZ, //!< Z Only viewport.
	
	ET_ViewportLast,
};

/** Base class for all Editor Viewports.
*/
class CViewport : public CWnd  
{
	DECLARE_DYNAMIC(CViewport)
public:
	enum ViewMode
	{
		NothingMode = 0,
		ScrollZoomMode,
		SelectMode,
		MoveMode,
		RotateMode,
		ScaleMode,
		ScrollMode,
		ZoomMode,
	};

	CViewport();
	virtual ~CViewport();

	//! Called while window is idle.
	virtual void Update();

	/** Set name of this viewport.
	*/
	void SetName( const CString &name );
	
	/** Get name of viewport
	*/
	CString GetName() const;

	/** Must be overriden in derived classes.
	*/
	virtual void SetType( EViewportType type ) {};

	/** Get type of this viewport.
	*/
	virtual EViewportType GetType() { return ET_ViewportUnknown; }

	//! Make this view active or deactive.
	virtual void SetActive( bool bActive );
	//! Check if this view is active.
	virtual bool IsActive() const;

	virtual void ResetContent();
	virtual void UpdateContent( int flags );

	virtual bool Create( CWnd *hWndParent,int id,const char *szTitle );

	//! Set current zoom factor for this viewport.
	virtual void SetZoomFactor(float fZoomFactor);

	//! Get current zoom factor for this viewport.
	virtual float GetZoomFactor() const;

	virtual void OnActivate();
	virtual void OnDeactivate();

	//! Map world space position to viewport position.
	virtual CPoint	WorldToView( Vec3 wp );
	//! Map viewport position to world space position.
	virtual Vec3		ViewToWorld( CPoint vp,bool *collideWithTerrain=0,bool onlyTerrain=false );
	//! Convert point on screen to world ray.
	virtual void		ViewToWorldRay( CPoint vp,Vec3 &raySrc,Vec3 &rayDir );

	//! Map view point to world space using current construction plane.
	virtual Vec3 MapViewToCP( CPoint point );

	//! This method return a vector (p2-p1) in world space alligned to construction plane and restriction axises.
	//! p1 and p2 must be givven in world space and lie on construction plane.
	virtual Vec3 GetCPVector( const Vec3 &p1,const Vec3 &p2 );

	//! Snap any givven 3D world position to grid lines if snap is enabled.
	virtual Vec3d SnapToGrid( Vec3d vec );

	//! Returns the screen scale factor for a point given in world coordinates.
	//! This factor gives the width in world-space units at the point's distance of the viewport.
	virtual float GetScreenScaleFactor( const Vec3 &worldPoint ) { return 1; };

	virtual void SetViewMode(ViewMode eViewMode) 	{ m_eViewMode = eViewMode; };
	ViewMode GetViewMode() { return m_eViewMode; };

	//! Change the cursor to point at specified object.
	virtual void SetObjectCursor( CBaseObject *hitObj,bool bChangeNow=false );

	void SetAxisConstrain( int axis );
	int GetAxisConstrain() const { return m_activeAxis; };

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewport)
	//}}AFX_VIRTUAL

	virtual void OnTitleMenu( CMenu &menu ) {};
	virtual void OnTitleMenuCommand( int id ) {};

	//////////////////////////////////////////////////////////////////////////
	void CaptureMouse();
	void ReleaseMouse();

	//////////////////////////////////////////////////////////////////////////
	//! Get current document.
	CCryEditDoc* GetDocument();

	//////////////////////////////////////////////////////////////////////////
	// Selection.
	//////////////////////////////////////////////////////////////////////////
	//! Resets current selection region.
	virtual void ResetSelectionRegion();
	//! Set 2D selection rectangle.
	virtual void SetSelectionRectangle( CPoint p1,CPoint p2 );
	//! Return 2D selection rectangle.
	virtual CRect GetSelectionRectangle() const { return m_selectedRect; };
	//! Called when dragging selection rectangle.
	virtual void OnDragSelectRectangle( CPoint p1,CPoint p2,bool bNormilizeRect=false );
	//! Callback called when objects within 2d rectangles must be selected.
	virtual void SelectObjectsInRect( const CRect &rect,bool bSelect );
	//! Get selection procision tollerance.
	float GetSelectionTolerance() const { return m_selectionTollerance; }
	//! Center viewport on selection.
	virtual void CenterOnSelection() {};

	//////////////////////////////////////////////////////////////////////////
	//! Draw brush on this viewport.
	virtual void DrawBrush( DisplayContext &dc,struct SBrush *brush,const Matrix44 &brushTM,int flags ) {};

	// Draw text in viewport.
	virtual void DrawTextLabel( DisplayContext &dc,const Vec3& pos,float size,const CFColor& color,const char *text ) {};

	//! Performs hit testing of point in view to find which object hit.
	virtual bool HitTest( CPoint point,ObjectHitInfo &hitInfo,int flags=0 );
	
	//! Do 2D hit testing of line in world space.
	virtual bool HitTestLine( const Vec3 &lineP1,const Vec3 &lineP2,CPoint hitpoint,int pixelRadius );

	//////////////////////////////////////////////////////////////////////////
	// View matrix.
	//////////////////////////////////////////////////////////////////////////
	//! Set current view matrix,
	//! This is a matrix that transforms from world to view space.
	void SetViewTM( const Matrix44 &tm ) { m_viewTM = tm; };
	
	//! Get current view matrix.
	//! This is a matrix that transforms from world space to view space.
	const Matrix44& GetViewTM() const { return m_viewTM; };

	//////////////////////////////////////////////////////////////////////////

	//! Access to view manager.
	CViewManager* GetViewManager() const { return m_viewManager; };

	//! Update current construction plane.
	void UpdateConstrPlane();
	//! Set construction plane from givven position and axis settings.
	//! @param axis 1=X,2=Y,3=Z,4=XY etc..
	virtual void SetConstrPlane( CPoint cursor,const Matrix44 &xform );
	virtual void SetConstrPlane( CPoint cursor,const Vec3& planeOrigin );

	// Get constrain axises for this viewport.
	//virtual void GetConstrainAxis( Vec3 &xAxis,Vec3 &yAxis,Vec3 &zAxis );

	void DegradateQuality( bool bEnable );

	//////////////////////////////////////////////////////////////////////////
	// Undo for viewpot operations.
	void BeginUndo();
	void AcceptUndo( const CString &undoDescription );
	void CancelUndo();
	void RestoreUndo();
	bool IsUndoRecording() const;
	//////////////////////////////////////////////////////////////////////////

	//! Get prefered original size for this viewport.
	//! if 0, then no preference.
	virtual CSize GetIdealSize() const;

	//! Check if world space bounding box is visible in this view.
	virtual bool IsBoundsVisible( const BBox &box ) const;

	//////////////////////////////////////////////////////////////////////////
	bool CheckVirtualKey( int virtualKey );

	virtual void StartAVIRecording( const char *filename );
	virtual void StopAVIRecording();
	virtual void PauseAVIRecording( bool bPause );
	virtual bool IsAVIRecording() const;
	virtual bool IsAVIRecordingPaused() const { return m_bAVIPaused; };

protected:
	friend class CViewManager;

	//! Ctrl-Click in move mode to move selected objects to givven pos.
	void MoveSelectionToPos( Vec3 &pos );

	// Changed my view manager.
	void SetViewManager( CViewManager* viewMgr ) { m_viewManager = viewMgr; };

	void SetCurrentCursor( HCURSOR hCursor );
	void AVIRecordFrame();

	//{{AFX_MSG(CViewport)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	float m_selectionTollerance;
	CMenu m_cViewMenu;

	//! View this window is attached to.
	CString m_name;

	mutable float m_fZoomFactor;

	bool m_bActive;
	ViewMode m_eViewMode;

	CPoint m_cMouseDownPos;

	//! Current selected rectangle.
	CRect m_selectedRect;

	int m_activeAxis;

	// Viewport matrix.
	Matrix44 m_viewTM;

	//! Current construction plane.
	Plane m_constructionPlane;
	Matrix44 m_constructionMatrix;
	Matrix44 m_constructionOriginalMatrix;
	CPoint m_constructionCursorPos;
	
	//! Viewport matrix for construction.
	//! This matrix can be different from ViewTM.
	Matrix44 m_constructionViewTM;
	
	CViewManager *m_viewManager;


	//////////////////////////////////////////////////////////////////////////
	// Standart cursors.
	//////////////////////////////////////////////////////////////////////////
	HCURSOR m_hDefaultCursor;
	HCURSOR m_hHitCursor;
	HCURSOR m_hMoveCursor;
	HCURSOR m_hRotateCursor;
	HCURSOR m_hScaleCursor;
	HCURSOR m_hSelectionPlusCursor;
	HCURSOR m_hSelectionMinusCursor;
	HCURSOR m_hCurrCursor;

	bool m_bRMouseDown;
	//! Mouse is over this object.
	CBaseObject* m_pMouseOverObject;
	CString m_cursorStr;

	static bool m_bDegradateQuality;

	class CAVI_Writer *m_pAVIWriter;
	CImage m_aviFrame;
	float m_aviFrameRate;
	float m_aviLastFrameTime;
	bool m_bAVICreation;
	bool m_bAVIPaused;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITWND_H__DB6E5168_5AA2_439C_A986_DDD440C82BA3__INCLUDED_)
