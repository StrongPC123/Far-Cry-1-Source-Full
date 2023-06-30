////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cameraobject.h
//  Version:     v1.00
//  Created:     29/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __cameraobject_h__
#define __cameraobject_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Entity.h"

/*!
 *	CCameraObject is an object that represent Source or Target of camera.
 *
 */
class CCameraObject : public CEntity
{
public:
	DECLARE_DYNCREATE(CCameraObject)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	CString GetTypeDescription() const { return GetTypeName(); };
	void Display( DisplayContext &disp );

	//////////////////////////////////////////////////////////////////////////
	virtual void SetName( const CString &name );
	virtual void SetScale( const Vec3d &scale );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	//! Called when object is being created.
	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	bool HitTest( HitContext &hc );
	bool HitTestRect( HitContext &hc );
	void Serialize( CObjectArchive &ar );

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );
	//////////////////////////////////////////////////////////////////////////

	//! Get Camera Field Of View angle.
	float GetFOV() const;

	//! Get Camera's aspect ratio.
	float GetAspect() const;

protected:
	//! Dtor must be protected.
	CCameraObject();
	IAnimNode* CreateAnimNode();
	// overrided from IAnimNodeCallback
	void OnNodeAnimated();

	void OnFovChange( IVariable *var );

	void GetConePoints( Vec3 q[4],float dist );
	void DrawCone( DisplayContext &dc,float dist,float fScale=1 );
	void CreateTarget();

	//////////////////////////////////////////////////////////////////////////
	//! Field of view.
	CVariable<float> mv_fov;

	//////////////////////////////////////////////////////////////////////////
	// Mouse callback.
	int m_creationStep;
};

/*!
 *	CCameraObjectTarget is a target object for Camera.
 *
 */
class CCameraObjectTarget : public CEntity
{
public:
	DECLARE_DYNCREATE(CCameraObjectTarget)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	CString GetTypeDescription() const { return GetTypeName(); };
	void Display( DisplayContext &disp );
	bool HitTest( HitContext &hc );
	void GetBoundBox( BBox &box );
	void SetScale( const Vec3d &scale ) {};
	void SetAngles( const Vec3d &scale ) {};
	void Serialize( CObjectArchive &ar );
	//////////////////////////////////////////////////////////////////////////

protected:
	//! Dtor must be protected.
	CCameraObjectTarget();
};

/*!
 * Class Description of CameraObject.	
 */
class CCameraObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {23612EE3-B568-465d-9B31-0CA32FDE2340}
		static const GUID guid = { 0x23612ee3, 0xb568, 0x465d, { 0x9b, 0x31, 0xc, 0xa3, 0x2f, 0xde, 0x23, 0x40 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_ENTITY; };
	const char* ClassName() { return "Camera"; };
	const char* Category() { return "Camera"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CCameraObject); };
	int GameCreationOrder() { return 202; };
};

/*!
 * Class Description of CameraObjectTarget.
 */
class CCameraObjectTargetClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {1AC4CF4E-9614-4de8-B791-C0028D0010D2}
		static const GUID guid = { 0x1ac4cf4e, 0x9614, 0x4de8, { 0xb7, 0x91, 0xc0, 0x2, 0x8d, 0x0, 0x10, 0xd2 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_ENTITY; };
	const char* ClassName() { return "CameraTarget"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CCameraObjectTarget); };
	int GameCreationOrder() { return 202; };
};

#endif // __cameraobject_h__