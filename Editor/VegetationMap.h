////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   vegetationmap.h
//  Version:     v1.00
//  Created:     31/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __vegetationmap_h__
#define __vegetationmap_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "VegetationObject.h"

/** This is description of single static vegetation object instance.
*/
struct CVegetationInstance
{
	CVegetationInstance *next;	//! Next object instance
	CVegetationInstance *prev;	//! Prev object instance.

	Vec3	pos;									//!< Instance position.
	float scale;								//!< Instance scale factor.
	CVegetationObject *object;	//!< Object of this instance.
	uchar brightness;						//!< Brightness of this object.
	uchar flags;								//!< Per instance flags.

	int m_refCount;								//!< Number of references to this vegetation instance.

	//! Add new refrence to this object.
	void AddRef() {	m_refCount++; };

	//! Release refrence to this object.
	//! when reference count reaches zero, object is deleted.
	void Release()
	{
		if ((--m_refCount) <= 0)
			delete this;
	}
private:
	// Private destructor, to not be able to delete it explicitly.
	~CVegetationInstance() {};
};

//////////////////////////////////////////////////////////////////////////
/** CVegetationMap stores static objects distributed over terrain.
		It keeps a list of all allocated geometry objects and all places instances.
*/
class CVegetationMap
{
public:
	CVegetationMap();
	~CVegetationMap();

	//////////////////////////////////////////////////////////////////////////
	// Map
	//////////////////////////////////////////////////////////////////////////
	//! Allocate sectors map.
	//! @param terrainSize Size of terrain in meters.
	void Allocate( CHeightmap *heightmap );

	//! Get number of sectors at side.
	int GetNumSectors() const;

	//! Get total Size of vegetation map.
	int GetSize() const;

	//! Convert world coordinate to sector coordinate.
	int WorldToSector( float worldCoord ) const;

	//! Place all objects in vegetation map to 3d engine terrain.
	void PlaceObjectsOnTerrain();
	//! Remove all object in vegetation map from 3d engine terrain.
	void RemoveObjectsFromTerrain();

	//! Get total number of vegetation instances.
	int GetNumInstances() const { return m_numInstances; };

	//////////////////////////////////////////////////////////////////////////
	// Vegetation Objects
	//////////////////////////////////////////////////////////////////////////
	//! Get number of use vegetation objects.
	int GetObjectCount() const { return m_objects.size(); }
	//! Get vegetation object.
	CVegetationObject*	GetObject( int i ) { return m_objects[i]; }

	//! Create new object.
	//! @param prev Source object to clone from.
	CVegetationObject* CreateObject( CVegetationObject *prev=0 );
	//! Remove object.
	void RemoveObject( CVegetationObject *obj );
	//! Replace one vegetation object with another.
	void ReplaceObject( CVegetationObject *pOldObject,CVegetationObject *pNewObject );
	//! Remove all objects.
	void ClearObjects();

	//! Hide all instances of this object.
	void HideObject( CVegetationObject *object,bool bHide );

	//! Export static object and all its instances.
	//! @return Number of serialized instances.
	int ExportObject( CVegetationObject *object,XmlNodeRef &node,CRect *saveRect=NULL );
	void ImportObject( XmlNodeRef &node,const Vec3& offset=Vec3(0,0,0) );

	//! Export part of vegetation map.
	void ExportBlock( const CRect &rect,CXmlArchive &ar );
	//! Import part of vegetation map.
	void ImportBlock( CXmlArchive &ar,CPoint placeOffset=CPoint(0,0) );

	//! Unload all rendering geometry from objects.
	void UnloadObjectsGeometry();

	//////////////////////////////////////////////////////////////////////////
	// Object Painting.
	//////////////////////////////////////////////////////////////////////////

	//! Place single object at specified location.
	CVegetationInstance* PlaceObjectInstance( const Vec3 &worldPos,CVegetationObject* brush );
	void DeleteObjInstance( CVegetationInstance *obj );
	//! Find object instances closes to the point withing givven radius.
	CVegetationInstance* GetNearestInstance( const Vec3 &worldPos,float radius );
	void GetObjectInstances( float x1,float y1,float x2,float y2,std::vector<CVegetationInstance*> &instances );
	void GetAllInstances( std::vector<CVegetationInstance*> &instances );
	//! Move instance to new position.
	void MoveInstance( CVegetationInstance* obj,const Vec3 &newPos );
	//! Remove object from 3D engine and place it back again.
	void RepositionObject( CVegetationObject *object );

	//! Scale all instances of this objects.
	void ScaleObjectInstances( CVegetationObject *object,float fScale );

	//! Remove any object at specified location.
	void RemoveObjectInstance( const Vec3 &worldPos );

	//! Paint objects on rectangle using givven brush.
	void PaintBrush( CRect &rc,bool bCircle,CVegetationObject* brush );

	//! Clear objects in rectangle using givven brush.
	//! @param brush Object to remove, if NULL all object will be cleared.
	void ClearBrush( CRect &rc,bool bCircle,CVegetationObject* brush );

	//! Clear all object within mask.
	void ClearMask( const CString &maskFile );

	//! Sets this brighness to all objects within specified rectangle.
	//! x,y,w,h are specified in world units (meters).
	//! \param brightness 0..255 brightness without ground texture, with hill shadows but without object shadows
	//! \param brightness_shadowmap 0..255 brightness without ground texture, with hill shadows and with object shadows
	void PaintBrightness( float x,float y,float w,float h,uchar brightness,uchar brightness_shadowmap );
	void SetUpdateOnPaintBrightness( bool bOn ) { m_bUpdateOnPaintBrightness = bOn; };

	//////////////////////////////////////////////////////////////////////////
	//! Serialize vegetation map to archive.
	void Serialize( CXmlArchive &xmlAr );

	//! Serialize vegetation instances
	void SerializeInstances( CXmlArchive &xmlAr,CRect *rect=NULL );

	//! Generate shadows from static objects and place them in shadow map bitarray.
	void GenerateShadowMap( CByteImage &shadowmap,float shadowAmmount,const Vec3 &sunVector );

	//! Draw sectors to texture.
	void DrawToTexture( uint *texture,int textureWidth,int textureHeight,int srcX,int srcY );

	//! Record undo info for vegetation instance.
	void RecordUndo( CVegetationInstance *obj );

	//////////////////////////////////////////////////////////////////////////
	// Validate Vegetation map about errors.
	//////////////////////////////////////////////////////////////////////////
	void Validate( CErrorReport &report );

private:
	struct SectorInfo
	{
		CVegetationInstance *first; // First vegetation object instance in this sector.
	};

	//! Get sector by world coordinates.
	SectorInfo*	GetSector( const Vec3 &worldPos );

	//! Get sector by 2d map coordinates.
	SectorInfo*	GetSector( int x,int y );

	//! Create new object instance in map.
	CVegetationInstance* CreateObjInstance( CVegetationObject *object,const Vec3 &pos );
	void DeleteObjInstance( CVegetationInstance *obj,SectorInfo *sector );
	//! Only to be used by undo/redo.
	void AddObjInstance( CVegetationInstance *obj );

	void SectorLink( CVegetationInstance *object,SectorInfo *sector );
	void SectorUnlink( CVegetationInstance *object,SectorInfo *sector );

	//! Return true if there is no specified objects within radius from givven position.
	bool CanPlace( CVegetationObject *object,const Vec3 &pos,float radius );

	//! Returns true if theres no any objects within radius from givven position.
	bool IsPlaceEmpty( const Vec3 &pos,float radius,CVegetationInstance *ignore );

	void Clear();
	void ClearSectors();

	void LoadOldStuff( CXmlArchive &xmlAr );

private:
	friend class CUndoVegInstanceCreate;
	void RepaintInstance( CVegetationInstance *obj );
	//! 2D Array of sectors that store vegetation objects.
	SectorInfo*	m_sectors;
	//! Size of single sector in meters.
	int m_sectorSize;
	//! Number of sectors in each dimension in map (Resolution of sectors map).
	int m_numSectors;
	//! Size of all map in meters.
	int m_mapSize;

	//! Minimal distance between two objects.
	//! Objects cannot be placed closer together that this distance.
	float m_minimalDistance;

	//! world to sector scaling ratio.
	float m_worldToSector;

	typedef std::vector<TSmartPtr<CVegetationObject> > Objects;
	Objects m_objects;

	//! Taken group ids.
	std::set<int> m_usedIds;

	//////////////////////////////////////////////////////////////////////////
	I3DEngine *m_I3DEngine;
	CHeightmap* m_heightmap;

	bool m_bUpdateOnPaintBrightness;
	int m_numInstances;
};

//////////////////////////////////////////////////////////////////////////
inline CVegetationMap::SectorInfo*	CVegetationMap::GetSector( int x,int y )
{
	assert( x >= 0 && x < m_numSectors && y >= 0 && y < m_numSectors );
	return &m_sectors[y*m_numSectors + x];
}

//////////////////////////////////////////////////////////////////////////
inline CVegetationMap::SectorInfo*	CVegetationMap::GetSector( const Vec3 &worldPos )
{
	int x = ftoi(worldPos.x*m_worldToSector);
	int y = ftoi(worldPos.y*m_worldToSector);
	if (x >= 0 && x < m_numSectors && y >= 0 && y < m_numSectors )
		return &m_sectors[y*m_numSectors + x];
	else
		return 0;


}

#endif // __vegetationmap_h__
