////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ModelViewport.h
//  Version:     v1.00
//  Created:     8/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ModelViewport_h__
#define __ModelViewport_h__

#if _MSC_VER > 1000
#pragma once
#endif


#include "RenderViewport.h"

/////////////////////////////////////////////////////////////////////////////
// CModelViewport window

class CModelViewport : public CRenderViewport
{
	DECLARE_DYNCREATE(CModelViewport)
// Construction
public:
	CModelViewport();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModelViewport)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CModelViewport();

	virtual EViewportType GetType() { return ET_ViewportModel; }
	virtual void SetType( EViewportType type ) { assert(type == ET_ViewportModel); };

	void LoadObject( const CString &obj,float scale );

	virtual void OnActivate();
	virtual void OnDeactivate();

	void AttachObject( const CString &model,const CString &bone );
	bool AddSubmesh (const CString& model);
	bool SetSubmesh (int nSlot, const CString& model);
	void StopAnimation(int nLayer);

	// Callbacks.
	void OnShowShaders( IVariable *var );
	void OnShowNormals( IVariable *var );
	void OnShowTangents( IVariable *var );

	void OnShowPortals( IVariable *var );
	void OnShowShadowVolumes( IVariable *var );
	void OnShowTextureUsage( IVariable *var );
	void OnShowAllTextures( IVariable *var );

	void On2Lights( IVariable *var );
	void OnLightColor( IVariable *var );
	void OnDisableVisibility( IVariable *var );

	void OnSubmeshSetChanged();
	ICryCharInstance* GetCharacter(){return m_character;}
protected:
	void LoadStaticObject( const CString &file );
	void ReleaseObject();

	// Called to render stuff.
	virtual void OnRender();
	void Update();
	
	virtual void SetViewerPos( const Vec3 &pos ) { m_viewerPos = pos; };
	virtual void SetViewerAngles( const Vec3 &angles ) { m_viewerAngles = angles; };
	virtual Vec3 GetViewerPos() const { return m_viewerPos; };
	virtual Vec3 GetViewerAngles() const { return m_viewerAngles; };

	void DrawGrid();
	void SetOrbitAngles( const Vec3d &ang );

	void DrawModel();
	void DrawSkyBox();

	void SetConsoleVar( const char *var,int value );
	void SetCharacterUIInfo();

	void StartAnimation (const char*szName);

	IStatObj *m_object;
	ICryCharInstance *m_character;
	IStatObj *m_weaponModel;
	// this is the character to attach, instead of weaponModel
	ICryCharInstance *m_attachedCharacter;
	CString m_attachBone;

	Vec3d m_bboxMin;
	Vec3d m_bboxMax;
	
	// Camera control.
	Vec3d m_camTarget;
	float m_camRadius;
	Vec3d m_camAngles;

	Vec3d m_objectAngles;
	
	Vec3 m_viewerPos;
	Vec3 m_viewerAngles;

	// True to show grid.
	bool m_bGrid;

	class ModelViewPanel* m_modelPanel;
	class ModelViewSubmeshPanel* m_modelSubmeshPanel;
	int m_rollupIndex;
	int m_rollupIndex2;

	CString m_loadedFile;
	std::vector<CDLight*> m_lights;

	bool m_bNavigateBuilding;
	
	float m_currTime;

	CRESky* m_pRESky;
	ICVar* m_pSkyboxName;
  IShader *m_pSkyBoxShader;

	CFColor m_currLightDiffuseColor;
	CFColor m_currLightSpecularColor;

	CVariable<bool> mv_wireframe;
	CVariable<bool> mv_showGrid;

	CVariable<bool> mv_showShaders;
	CVariable<bool> mv_showSkyBox;
	CVariable<bool> mv_showNormals;
	CVariable<bool> mv_showTangents;

	CVariable<bool> mv_showShadowVolumes;
	CVariable<bool> mv_showTextureUsage;
	CVariable<bool> mv_showAllTextures;

	CVariable<bool> mv_lighting;
	CVariable<bool> mv_animateLights;
	CVariable<bool> mv_2LightSources;

	CVariable<bool> mv_disableLod;

	CVariable<Vec3> mv_lightDiffuseColor;
	CVariable<Vec3> mv_lightSpecularColor;
	CVariable<Vec3> mv_objectAmbientColor;
	CVariable<Vec3> mv_backgroundColor;

	CVariable<bool> mv_disableVisibility;
	CVariable<float> mv_fov;

	CVarObject m_vars;

	// Generated message map functions
protected:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnAnimBack();
	afx_msg void OnAnimFastBack();
	afx_msg void OnAnimFastForward();
	afx_msg void OnAnimFront();
	afx_msg void OnAnimPlay();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __ModelViewport_h__
