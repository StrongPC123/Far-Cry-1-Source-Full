////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EntityCamera.h
//  Version:     v1.00
//  Created:     14/8/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Wraps Camera attached to the Entity.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EntityCamera_h__
#define __EntityCamera_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include <Cry_Camera.h>
#include <IEntitySystem.h>
#include <IConsole.h>
#include <IPhysics.h>

struct IEntity;

class CEntityCamera : 
public IEntityCamera
{
public:
	void SetCameraMode(const Vec3d &lookat, const Vec3d &lookat_angles, IPhysicalEntity *physic);
	void Release() { delete this; };

	//! Set/Get camera position.
	void SetPos( const Vec3d &p ) { if (!(GetLengthSquared(p)>=0)) return; m_camera.SetPos(p); };
	Vec3d GetPos() const { return m_camera.GetPos(); };

	//! Set/Get camera angles.
	void SetAngles( const Vec3d &p ) { m_camera.SetAngle(p); };
	Vec3d GetAngles() const { return m_camera.GetAngles(); };

	//! Set/Get camera FOV.
	void SetFov( const float &f, const unsigned int iWidth, const unsigned int iHeight ) {
		m_camera.SetFov(f);
		m_camera.Init(iWidth,iHeight, f);
		m_camera.Update();
	};
	float GetFov() const { return m_camera.GetFov(); };

	//! Set/Get Matrix.
//	void SetMatrix( const Matrix44 &m ) { m_camera.SetVCMatrix(m); };
	Matrix44 GetMatrix() const { return m_camera.GetVCMatrixD3D9(); };

	void Update() { m_camera.Update(); }

	//! Access to wraped camera.
	CCamera& GetCamera() { return m_camera; }
	void SetCamera( const CCamera &cam ) { m_camera = cam; }

	void SetThirdPersonMode( const Vec3d &pos,const Vec3d &angles,int mode,float frameTime,float range,int dangleAmmount,
							IPhysicalEntity *physic, IPhysicalEntity *physicMore,
							I3DEngine* p3DEngine, float safe_range=0.0f);



	void SetViewOffset(float f) { m_sParam.m_viewoffset = f; };
	float GetViewOffset() { return m_sParam.m_viewoffset; };

	void SetCamOffset(Vec3d v) { 
		m_sParam.m_camoffset = m_camera.m_vOffset = v; 
	};
	Vec3d& GetCamOffset() { return m_sParam.m_camoffset; };

	void Init(IPhysicalWorld *pIPhysWorld, UINT iWidth, UINT iHeight, IConsole *pConsole); 

	void SetParameters(const EntityCameraParam *pParam) { memcpy(&m_sParam, pParam, sizeof(EntityCameraParam)); };
	void GetParameters(EntityCameraParam *pParam) { memcpy(pParam, &m_sParam, sizeof(EntityCameraParam)); };

	void SetCameraOffset(const Vec3d &offset) { m_vCameraOffset=m_camera.m_vOffset=offset; }
	void GetCameraOffset(Vec3d &offset) {offset = m_vCameraOffset; }

private:

	IConsole *m_pConsole;
	CCamera m_camera;
	EntityCameraParam m_sParam;
	IPhysicalWorld *m_pIPhysWorld;
	Vec3d		m_vCameraOffset;
	float		m_fTimeIdle;
	float		m_fDeltaDist;
};

#endif // __EntityCamera_h__
