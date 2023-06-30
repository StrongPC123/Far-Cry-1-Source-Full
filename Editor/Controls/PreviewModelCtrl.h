#if !defined(AFX_PREVIEWMODELCTRL_H__138DA368_C705_4A79_A8FA_5A5D328B8C67__INCLUDED_)
#define AFX_PREVIEWMODELCTRL_H__138DA368_C705_4A79_A8FA_5A5D328B8C67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewModelCtrl.h : header file
//

struct IEntity;

/////////////////////////////////////////////////////////////////////////////
// CPreviewModelCtrl window

class CPreviewModelCtrl : public CWnd
{
// Construction
public:
	CPreviewModelCtrl();

// Attributes
public:

// Operations
public:
	BOOL Create( CWnd *pWndParent,const CRect &rc,DWORD dwStyle=WS_CHILD|WS_VISIBLE );
	void LoadFile( const CString &modelFile,bool changeCamera=true );
	Vec3 GetSize() const { return m_size; };

	void SetEntity( IEntityRender *entity );
	void SetObject( IStatObj *pObject );
	void SetCameraLookAtBox( const BBox &box );
	void SetGrid( bool bEnable ) { m_bGrid = bEnable; }
	void SetRotation( bool bEnable );

	void EnableUpdate( bool bEnable );
	void Update();

	void DeleteRenderContex();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewModelCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPreviewModelCtrl();

	bool CreateContext();
	void ReleaseObject();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPreviewModelCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:
	void SetCamera( CCamera &cam );
	void SetOrbitAngles( const Vec3d &ang );
	bool Render();
	void DrawGrid();

	IStatObj *m_object;
	ICryCharInstance *m_character;
	CCamera m_camera;

	IRenderer* m_renderer;
	I3DEngine* m_engine;
  ICryCharManager* m_pAnimationSystem;
	bool m_bContextCreated;


	Vec3 m_size;
	Vec3 m_objectAngles;
	Vec3 m_pos;
	int m_nTimer;

	CString m_loadedFile;
	std::vector<CDLight> m_lights;

	Vec3d m_bboxMin;
	Vec3d m_bboxMax;
	
	// Camera control.
	Vec3d m_camTarget;
	float m_camRadius;
	Vec3d m_camAngles;
	bool m_bInRotateMode;
	bool m_bInMoveMode;
	CPoint m_mousePos;
	IEntityRender* m_entity;

	bool m_bHaveAnythingToRender;
	bool m_bGrid;
	bool m_bUpdate;
	float m_fov;
	// Rotate object in preview.
	bool m_bRotate;
	float m_rotateAngle;

protected:
	virtual void PreSubclassWindow();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWMODELCTRL_H__138DA368_C705_4A79_A8FA_5A5D328B8C67__INCLUDED_)
