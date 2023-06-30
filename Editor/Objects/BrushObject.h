////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushobject.h
//  Version:     v1.00
//  Created:     8/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushobject_h__
#define __brushobject_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"
#include "..\Brush\Brush.h"

struct SBrush;
class CEdMesh;

/*!
 *	CTagPoint is an object that represent named 3d position in world.
 *
 */
class CBrushObject : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CBrushObject)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	void Display( DisplayContext &dc );
	bool CreateGameObject();

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );

	bool HitTest( HitContext &hc );
	int HitTestAxis( HitContext &hc );

	virtual void SetScale( const Vec3d &scale );
	virtual void SetSelected( bool bSelect );
	virtual IPhysicalEntity* GetCollisionEntity() const;

	//////////////////////////////////////////////////////////////////////////
	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );
	void BeginEditMultiSelParams( bool bAllOfSameType );
	void EndEditMultiSelParams();

	//! Called when object is being created.
	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	void Serialize( CObjectArchive &ar );
	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );

	virtual void Validate( CErrorReport *report );
	virtual bool IsSimilarObject( CBaseObject *pObject );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//! Assign brush to object.
	void SetBrush( SBrush *brush );
	//! Retrieve brush assigned to object.
	SBrush* GetBrush();

	void SelectBrushSide( const Vec3 &raySrc,const Vec3 &rayDir,bool shear );
	void MoveSelectedPoints( const Vec3 &worldOffset );
	void ResetToPrefabSize();
	void ReloadPrefabGeometry();

	//////////////////////////////////////////////////////////////////////////
	// Used by brush exporter.
	//////////////////////////////////////////////////////////////////////////
	IStatObj* GetPrefabGeom() const;
	Matrix44 GetBrushMatrix() const;
	int GetRenderFlags() const { return m_renderFlags; };
	IEntityRender* GetEngineNode() const { return m_engineNode; };
	float GetRatioLod() const { return mv_ratioLOD; };
	float GetRatioViewDist() const { return mv_ratioViewDist; };

	//////////////////////////////////////////////////////////////////////////
	// Material
	//////////////////////////////////////////////////////////////////////////
	void SetMaterial( CMaterial *mtl );
	CMaterial* GetMaterial() const;

	//////////////////////////////////////////////////////////////////////////
	bool IsRecieveLightmap() const { return mv_recvLightmap; };

protected:
	//! Dtor must be protected.
	CBrushObject();

	virtual void UpdateVisibility( bool visible );

	//! Convert ray givven in world coordinates to the ray in local brush coordinates.
	void WorldToLocalRay( Vec3 &raySrc,Vec3 &rayDir );

	bool ConvertFromObject( CBaseObject *object );
	void DeleteThis() { delete this; };
	void InvalidateTM();

	void CreateBrushFromPrefab( const char *meshFilname );
	void UpdateEngineNode( bool bOnlyTransform=false );

	void OnFileChange( CString filename );

	//////////////////////////////////////////////////////////////////////////
	// Local callbacks.
	void OnPrefabChange( IVariable *var );
	void OnRenderVarChange( IVariable *var );
	//////////////////////////////////////////////////////////////////////////

	BBox m_bbox;
	Matrix44 m_invertTM;
	
	//! Actual brush.
	TSmartPtr<SBrush> m_brush;
	
	// Selected points of this brush.
	SBrushSubSelection m_subSelection;
	//std::vector<Vec3*> m_selectedPoints;

	//! Engine node.
	//! Node that registered in engine and used to render brush prefab
	IEntityRender* m_engineNode;

	TSmartPtr<CEdMesh> m_prefabGeom;
	CBrushIndoor *m_indoor;

	//////////////////////////////////////////////////////////////////////////
	// Brush parameters.
	//////////////////////////////////////////////////////////////////////////
	CVariable<CString> mv_prefabName;

	//////////////////////////////////////////////////////////////////////////
	// Brush rendering parameters.
	//////////////////////////////////////////////////////////////////////////
	CVariable<bool> mv_outdoor;
	CVariable<bool> mv_castShadows;
	CVariable<bool> mv_selfShadowing;
	CVariable<bool> mv_castShadowMaps;
	CVariable<bool> mv_castLightmap;
	CVariable<bool> mv_recvLightmap;
	CVariable<bool> mv_recvShadowMaps;
	CVariable<bool> mv_hideable;
	CVariable<int> mv_ratioLOD;
	CVariable<int> mv_ratioViewDist;
	CVariable<bool> mv_excludeFromTriangulation;
	CVariable<float> mv_lightmapQuality;

	//////////////////////////////////////////////////////////////////////////
	// Rendering flags.
	int m_renderFlags;

	//////////////////////////////////////////////////////////////////////////
	// Material assigned to this brush.
	//////////////////////////////////////////////////////////////////////////
	TSmartPtr<CMaterial> m_pMaterial;
	GUID m_materialGUID;

	bool m_bIgnoreNodeUpdate;
};

/*!
 * Class Description of CBrushObject.	
 */
class CBrushObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {032B8809-71DB-44d7-AAA1-69F75C999463}
		static const GUID guid = { 0x32b8809, 0x71db, 0x44d7, { 0xaa, 0xa1, 0x69, 0xf7, 0x5c, 0x99, 0x94, 0x63 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_BRUSH; };
	const char* ClassName() { return "Brush"; };
	const char* Category() { return "Brush"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CBrushObject); };
	const char* GetFileSpec() { return "Objects\\*.cgf"; };
	int GameCreationOrder() { return 150; };
};

#endif // __brushobject_h__