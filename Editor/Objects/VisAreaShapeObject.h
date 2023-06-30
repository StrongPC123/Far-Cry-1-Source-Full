////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   VisAreaShapeObject.h
//  Version:     v1.00
//  Created:     10/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __visareashapeobject_h__
#define __visareashapeobject_h__
#pragma once

#include "ShapeObject.h"

/** Represent Visibiility Area, visibility areas can be connected with portals.
*/
class CVisAreaShapeObject : public CShapeObject
{
	DECLARE_DYNCREATE(CVisAreaShapeObject)
public:
	CVisAreaShapeObject();

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	bool CreateGameObject();
	//////////////////////////////////////////////////////////////////////////

protected:
	virtual bool IsOnlyUpdateOnUnselect() const { return true; }
	virtual void UpdateGameArea( bool bRemove=false );

	// Visibilty area in 3d engine.
	struct IVisArea* m_area;
	CVariable<Vec3d> mv_vAmbientColor; 
	CVariable<bool>  mv_bAffectedBySun;
	CVariable<Vec3d> mv_vDynAmbientColor; 
	CVariable<float> mv_fViewDistRatio;
	CVariable<bool>  mv_bSkyOnly;
};

/** Represent Portal Area.
		Portal connect visibility areas, visibility between vis areas are only done with portals.
*/
class CPortalShapeObject : public CVisAreaShapeObject
{
	DECLARE_DYNCREATE(CPortalShapeObject)
public:
	CPortalShapeObject();

protected:
	virtual int GetMaxPoints() const { return 4; };
	virtual void UpdateGameArea( bool bRemove=false );

	CVariable<bool>  mv_bUseDeepness;
	CVariable<bool>  mv_bDoubleSide;
};

/** Represent Occluder Area.
Areas that occlude objects behind it.
*/
class COccluderShapeObject : public CVisAreaShapeObject
{
	DECLARE_DYNCREATE(COccluderShapeObject)
public:
	COccluderShapeObject();

protected:
	virtual int GetMaxPoints() const { return 4; };
	virtual void UpdateGameArea( bool bRemove=false );

	CVariable<bool>  mv_bUseInIndoors;
	CVariable<bool>  mv_bDoubleSide;
};

/*!
 * Class Description of CVisAreaShapeObject.
 */
class CVisAreaShapeObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {31B912A0-D351-4abc-AA8F-B819CED21231}
		static const GUID guid = { 0x31b912a0, 0xd351, 0x4abc, { 0xaa, 0x8f, 0xb8, 0x19, 0xce, 0xd2, 0x12, 0x31 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_VOLUME; };
	const char* ClassName() { return "VisArea"; };
	const char* Category() { return "Area"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CVisAreaShapeObject); };
	int GameCreationOrder() { return 10; };
};

/*!
* Class Description of CPortalShapeObject.
*/
class CPortalShapeObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {D8665C26-AE2D-482b-9574-B6E2C9253E40}
		static const GUID guid = { 0xd8665c26, 0xae2d, 0x482b, { 0x95, 0x74, 0xb6, 0xe2, 0xc9, 0x25, 0x3e, 0x40 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_VOLUME; };
	const char* ClassName() { return "Portal"; };
	const char* Category() { return "Area"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CPortalShapeObject); };
	int GameCreationOrder() { return 11; };
};

/*!
* Class Description of COccluderShapeObject.
*/
class COccluderShapeObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {76D000C3-47E8-420a-B4C9-17F698C1A607}
		static const GUID guid = { 0x76d000c3, 0x47e8, 0x420a, { 0xb4, 0xc9, 0x17, 0xf6, 0x98, 0xc1, 0xa6, 0x7 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_VOLUME; };
	const char* ClassName() { return "OccluderArea"; };
	const char* Category() { return "Area"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(COccluderShapeObject); };
	int GameCreationOrder() { return 12; };
};

#endif // __visareashapeobject_h__
