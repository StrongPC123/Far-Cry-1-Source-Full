////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ShapeObject.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: ShapeObject object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ShapeObject_h__
#define __ShapeObject_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"
#include "SafeObjectsArray.h"

#define SHAPE_CLOSE_DISTANCE 0.8f
#define SHAPE_Z_OFFSET 0.1f

/*!
 *	CShapeObject is an object that represent named 3d position in world.
 *
 */
class CShapeObject : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CShapeObject)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	bool CreateGameObject();

	void Display( DisplayContext &dc );

	//////////////////////////////////////////////////////////////////////////
	void SetName( const CString &name );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	//! Called when object is being created.
	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );

	bool HitTest( HitContext &hc );
	bool HitTestRect( HitContext &hc );

	void OnEvent( ObjectEvent event );

	void Serialize( CObjectArchive &ar );
	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

	int GetAreaId();

	void SetClosed( bool bClosed );
	bool IsClosed() { return mv_closed; };

	//! Insert new point to shape at givven index.
	//! @return index of newly inserted point.
	int InsertPoint( int index,const Vec3 &point );
	//! Remve point from shape at givven index.
	void RemovePoint( int index );

	//! Get number of points in shape.
	int GetPointCount() { return m_points.size(); };
	//! Get point at index.
	const Vec3&	GetPoint( int index ) const { return m_points[index]; };
	//! Set point position at specified index.
	void SetPoint( int index,const Vec3 &pos );

	void SelectPoint( int index );
	int GetSelectedPoint() const { return m_selectedPoint;};

	//! Set shape width.
	float GetWidth() const { return mv_width; };

	//! Set shape height.
	float GetHeight() const { return mv_height; };

	//! Find shape point nearest to givven point.
	int GetNearestPoint( const Vec3 &raySrc,const Vec3 &rayDir,float &distance );

	//! Find shape edge nearest to givven point.
	void GetNearestEdge( const Vec3 &pos,int &p1,int &p2,float &distance,Vec3 &intersectPoint );

	//! Find shape edge nearest to givven ray.
	void GetNearestEdge( const Vec3 &raySrc,const Vec3 &rayDir,int &p1,int &p2,float &distance,Vec3 &intersectPoint );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	virtual void SetSelected( bool bSelect );

	//////////////////////////////////////////////////////////////////////////
	// Binded entities.
	//////////////////////////////////////////////////////////////////////////
	//! Bind entity to this shape.
	void AddEntity( CBaseObject *entity );
	void RemoveEntity( int index );
	CBaseObject* GetEntity( int index );
	int GetEntityCount() { return m_entities.GetCount(); }

	void CalcBBox();

protected:
	virtual void PostClone( CBaseObject *pFromObject,CObjectCloneContext &ctx );

	bool RayToLineDistance( const Vec3 &rayLineP1,const Vec3 &rayLineP2,const Vec3 &pi,const Vec3 &pj,float &distance,Vec3 &intPnt );
	virtual bool IsOnlyUpdateOnUnselect() const { return false; }
	virtual int GetMaxPoints() const { return 1000; };
	////! Calculate distance between 
	//float DistanceToShape( const Vec3 &pos );
	void DrawTerrainLine( DisplayContext &dc,const Vec3 &p1,const Vec3 &p2 );

	// Ignore default draw highlight.
	void DrawHighlight( DisplayContext &dc ) {};

	void EndCreation();

	//! Update game area.
	virtual void UpdateGameArea( bool bRemove=false );

	//overrided from CBaseObject.
	void InvalidateTM();

	//! Called when shape variable changes.
	void OnShapeChange( IVariable *var );

	//! Dtor must be protected.
	CShapeObject();

	void DeleteThis() { delete this; };

	IEditor *m_ie;
	BBox m_bbox;

	std::vector<Vec3> m_points;
	
	struct IXArea *m_IArea;

	bool m_bIgnoreGameUpdate;
	//////////////////////////////////////////////////////////////////////////
	
	//! List of binded entities.
	//std::vector<uint> m_entities;

	// Entities.
	CSafeObjectsArray m_entities;

	//////////////////////////////////////////////////////////////////////////
	// Shape parameters.
	//////////////////////////////////////////////////////////////////////////
	CVariable<float> mv_width;
	CVariable<float> mv_height;
	CVariable<int> mv_areaId;
	CVariable<int> mv_groupId;
	CVariable<bool> mv_closed;
	//! Display triangles filling closed shape.
	CVariable<bool> mv_displayFilled;

	int m_selectedPoint;
	float m_lowestHeight;
	bool m_bAreaModified;
	//! Forces shape to be always 2D. (all vertices lie on XY plane).
	bool m_bForce2D;
	bool m_bDisplayFilledWhenSelected;

	static int m_rollupId;
	static class CShapePanel* m_panel;
};

/*!
 * Class Description of ShapeObject.	
 */
class CShapeObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {6167DD9D-73D5-4d07-9E92-CF12AF451B08}
		static const GUID guid = { 0x6167dd9d, 0x73d5, 0x4d07, { 0x9e, 0x92, 0xcf, 0x12, 0xaf, 0x45, 0x1b, 0x8 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_SHAPE; };
	const char* ClassName() { return "Shape"; };
	const char* Category() { return "Area"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CShapeObject); };
	int GameCreationOrder() { return 50; };
};

//////////////////////////////////////////////////////////////////////////
// Special object for forbidden area.
//////////////////////////////////////////////////////////////////////////
class CAIForbiddenAreaObject : public CShapeObject
{
	DECLARE_DYNCREATE(CAIForbiddenAreaObject)
public:
	CAIForbiddenAreaObject();
	// Ovverride game area creation.
	virtual void UpdateGameArea( bool bRemove=false );
};

//////////////////////////////////////////////////////////////////////////
// Special object for AI Walkabale paths.
//////////////////////////////////////////////////////////////////////////
class CAIPathObject : public CShapeObject
{
	DECLARE_DYNCREATE(CAIPathObject)
public:
	CAIPathObject();
	// Ovverride game area creation.
	virtual void UpdateGameArea( bool bRemove=false );
};

//////////////////////////////////////////////////////////////////////////
// Special object for AI Walkabale paths.
//////////////////////////////////////////////////////////////////////////
class CAINavigationModifierObject : public CShapeObject
{
	DECLARE_DYNCREATE(CAINavigationModifierObject)
public:
	CAINavigationModifierObject();
	// Ovverride game area creation.
	virtual void UpdateGameArea( bool bRemove=false );
};

//////////////////////////////////////////////////////////////////////////
// Special object for AI Walkabale paths.
//////////////////////////////////////////////////////////////////////////
class CAIOcclusionPlaneObject : public CShapeObject
{
	DECLARE_DYNCREATE(CAIOcclusionPlaneObject)
public:
	CAIOcclusionPlaneObject();
	// Ovverride game area creation.
	virtual void UpdateGameArea( bool bRemove=false );
};

/*!
 * Class Description of CForbiddenAreaObject.
 */
class CAIForbiddenAreaObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {9C9D5FD9-761B-4734-B2AA-38990E4C2EB9}
		static const GUID guid = { 0x9c9d5fd9, 0x761b, 0x4734, { 0xb2, 0xaa, 0x38, 0x99, 0xe, 0x4c, 0x2e, 0xb9 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_SHAPE; };
	const char* ClassName() { return "ForbiddenArea"; };
	const char* Category() { return "AI"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CAIForbiddenAreaObject); };
	int GameCreationOrder() { return 50; };
};

/*!
 * Class Description of CForbiddenAreaObject.
 */
class CAIPathObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {06380C56-6ECB-416f-9888-60DE08F0280B}
		static const GUID guid = { 0x6380c56, 0x6ecb, 0x416f, { 0x98, 0x88, 0x60, 0xde, 0x8, 0xf0, 0x28, 0xb } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_SHAPE; };
	const char* ClassName() { return "AIPath"; };
	const char* Category() { return "AI"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CAIPathObject); };
	int GameCreationOrder() { return 50; };
};

/*!
 * Class Description of CForbiddenAreaObject.
 */
class CAINavigationModifierObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {2AD95C7B-5548-435b-BF0C-63632D7FEA40}
		static const GUID guid = { 0x2ad95c7b, 0x5548, 0x435b, { 0xbf, 0xc, 0x63, 0x63, 0x2d, 0x7f, 0xea, 0x40 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_SHAPE; };
	const char* ClassName() { return "AINavigationModifier"; };
	const char* Category() { return "AI"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CAINavigationModifierObject); };
	int GameCreationOrder() { return 50; };
};

/*!
* Class Description of CForbiddenAreaObject.
*/
class CAIOcclusionPlaneObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {85F4FEB1-1E97-4d78-BC0F-CACEA0D539C3}
		static const GUID guid = 
		{ 0x85f4feb1, 0x1e97, 0x4d78, { 0xbc, 0xf, 0xca, 0xce, 0xa0, 0xd5, 0x39, 0xc3 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_SHAPE; };
	const char* ClassName() { return "AIHorizontalOcclusionPlane"; };
	const char* Category() { return "AI"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CAIOcclusionPlaneObject); };
	int GameCreationOrder() { return 50; };
};

#endif // __ShapeObject_h__