////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   vegetationmap.cpp
//  Version:     v1.00
//  Created:     31/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "VegetationMap.h"
#include "Heightmap.h"
#include "Layer.h"
#include "VegetationBrush.h"

#include <I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
// CVegetationMap implementation.
//////////////////////////////////////////////////////////////////////////

#pragma pack(push,1)
// Structure of vegetation object instance in file.
struct SVegInst
{
	Vec3	pos;
	float scale;
	uchar objectIndex;
	uchar brightness;
	uchar flags;
};
#pragma pack(pop)

#define MAX_TERRAIN_SIZE 1024
#define MIN_MASK_VALUE 32

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Undo object for CBaseObject.
class CUndoVegInstance : public IUndoObject
{
public:
	CUndoVegInstance( CVegetationInstance *obj )
	{
		// Stores the current state of this object.
		assert( obj != 0 );
		m_obj = obj;
		m_obj->AddRef();

		ZeroStruct(m_redo);
		m_undo.pos = m_obj->pos;
		m_undo.scale = m_obj->scale;
		m_undo.objectIndex = m_obj->object->GetIndex();
		m_undo.brightness = m_obj->brightness;
		m_undo.flags = m_obj->flags;
	}
	~CUndoVegInstance()
	{
		m_obj->Release();
	}
protected:
	virtual int GetSize() { return sizeof(*this); }
	virtual const char* GetDescription() { return "Vegetation Modify"; };

	virtual void Undo( bool bUndo )
	{
		if (bUndo)
		{
			m_redo.pos = m_obj->pos;
			m_redo.scale = m_obj->scale;
			m_redo.objectIndex = m_obj->object->GetIndex();
			m_redo.brightness = m_obj->brightness;
			m_redo.flags = m_obj->flags;
		}
		m_obj->scale = m_undo.scale;
		m_obj->brightness = m_undo.brightness;
		m_obj->flags = m_undo.flags;
		GetIEditor()->GetVegetationMap()->MoveInstance( m_obj,m_undo.pos );
	}
	virtual void Redo()
	{
		m_obj->scale = m_redo.scale;
		m_obj->brightness = m_redo.brightness;
		m_obj->flags = m_redo.flags;
		GetIEditor()->GetVegetationMap()->MoveInstance( m_obj,m_redo.pos );
	}
private:
	CVegetationInstance *m_obj;
	SVegInst m_undo;
	SVegInst m_redo;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Undo object for CBaseObject.
class CUndoVegInstanceCreate : public IUndoObject
{
public:
	CUndoVegInstanceCreate( CVegetationInstance *obj,bool bDeleted )
	{
		// Stores the current state of this object.
		assert( obj != 0 );
		m_obj = obj;
		m_obj->AddRef();
		m_bDeleted = bDeleted;
	}
	~CUndoVegInstanceCreate()
	{
		m_obj->Release();
	}
protected:
	virtual int GetSize() { return sizeof(*this); }
	virtual const char* GetDescription() { return "Vegetation Create"; };

	virtual void Undo( bool bUndo )
	{
		CVegetationMap *vegMap = GetIEditor()->GetVegetationMap();
		if (m_bDeleted)
		{
			vegMap->AddObjInstance( m_obj );
			vegMap->MoveInstance( m_obj,m_obj->pos );
		}
		else
			vegMap->DeleteObjInstance( m_obj );
	}
	virtual void Redo()
	{
		CVegetationMap *vegMap = GetIEditor()->GetVegetationMap();
		if (!m_bDeleted)
		{
			vegMap->AddObjInstance( m_obj );
			vegMap->MoveInstance( m_obj,m_obj->pos );
		}
		else
			vegMap->DeleteObjInstance( m_obj );
	}
private:
	CVegetationInstance *m_obj;
	bool m_bDeleted;
};


//////////////////////////////////////////////////////////////////////////
CVegetationMap::CVegetationMap()
{
	m_numSectors = 0;
	m_sectors = 0;
	m_worldToSector = 0;

	m_minimalDistance = 0.1f;
	m_numInstances = 0;
	m_bUpdateOnPaintBrightness = true;

	m_I3DEngine = GetIEditor()->Get3DEngine();

	// Initialize the random number generator
	srand( GetTickCount() );
}

//////////////////////////////////////////////////////////////////////////
CVegetationMap::~CVegetationMap()
{
	Clear();

	if (m_sectors)
		free( m_sectors );
}

void CVegetationMap::Clear()
{
	ClearObjects();
}
	
void CVegetationMap::ClearSectors()
{
	// Delete all objects in sectors.
	// Iterator over all sectors.
	CVegetationInstance *next;
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		SectorInfo *si = &m_sectors[i];
		// Iterate on every object in sector.
		for (CVegetationInstance *obj = si->first; obj; obj = next)
		{
			next = obj->next;
			obj->Release();
		}
		si->first = 0;
	}
	m_numInstances = 0;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::Allocate( CHeightmap *heightmap )
{
	m_I3DEngine = GetIEditor()->Get3DEngine();
	m_heightmap = heightmap;

	int terrainSize = m_heightmap->GetUnitSize() * max(m_heightmap->GetWidth(),m_heightmap->GetHeight());

	int sectorSize;
	int numSectors;

	if (terrainSize < MAX_TERRAIN_SIZE)
	{
		sectorSize = 1;
	}
	else
	{
		sectorSize = terrainSize / MAX_TERRAIN_SIZE;
	}
	assert( sectorSize != 0 );
	numSectors = terrainSize / sectorSize;

	if (sectorSize != m_sectorSize || numSectors != m_numSectors || !m_sectors)
	{
		ClearSectors();
		if (m_sectors)
			free( m_sectors );

		m_numSectors = numSectors;
		m_sectorSize = sectorSize;
		// allocate sectors map.
		int sz = sizeof(SectorInfo)*m_numSectors*m_numSectors;
		m_sectors = (SectorInfo*)malloc( sz );
		memset( m_sectors,0,sz );
	}

	m_mapSize = terrainSize;
	m_worldToSector = 1.0f / m_sectorSize;
}

//////////////////////////////////////////////////////////////////////////
int CVegetationMap::GetNumSectors() const
{
	return m_numSectors;
}

//////////////////////////////////////////////////////////////////////////
int CVegetationMap::GetSize() const
{
	return m_numSectors * m_sectorSize;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::PlaceObjectsOnTerrain()
{
	if (!m_I3DEngine)
		return;

	// Clear all objects from 3d Engine.
	m_I3DEngine->RemoveAllStaticObjects();

	// Iterator over all sectors.
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		SectorInfo *si = &m_sectors[i];
		// Iterate on every object in sector.
		for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
		{
			if (!obj->object->IsHidden())
			{
				// Stick vegetation to terrain.
				obj->pos.z = m_I3DEngine->GetTerrainElevation(obj->pos.x,obj->pos.y);
				m_I3DEngine->AddStaticObject( obj->object->GetId(),obj->pos,obj->scale,obj->brightness );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::RemoveObjectsFromTerrain()
{
	if (m_I3DEngine)
		m_I3DEngine->RemoveAllStaticObjects();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::HideObject( CVegetationObject *object,bool bHide )
{
	if (object->IsHidden() == bHide)
		return;

	object->SetHidden(bHide);

	if (object->GetNumInstances() > 0)
	{
		// Iterate over all sectors.
		for (int i = 0; i < m_numSectors*m_numSectors; i++)
		{
			SectorInfo *si = &m_sectors[i];
			// Iterate on every object in sector.
			for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
			{
				if (obj->object == object)
				{
					// Remove/Add to terrain.
					if (bHide)
						m_I3DEngine->RemoveStaticObject( -1,obj->pos );
					else
					{
						// Stick vegetation to terrain.
						obj->pos.z = m_I3DEngine->GetTerrainElevation(obj->pos.x,obj->pos.y);
						m_I3DEngine->AddStaticObject( obj->object->GetId(),obj->pos,obj->scale,obj->brightness );
					}
				}
			}
		}
	}
}

void CVegetationMap::SectorLink( CVegetationInstance *obj,SectorInfo *sector )
{
	/*
	if (sector->first)
	{
		// Add to the end of current list.
		CVegetationInstance *head = sector->first;
		CVegetationInstance *tail = head->prev;
		obj->prev = tail;
		obj->next = 0;

		tail->next = obj;
		head->prev = obj; // obj is now last object.
	}
	else
	{
		sector->first = obj;
		obj->prev = obj;
		obj->next = 0;
	}
	*/
	if (sector->first)
	{
		// Add to the end of current list.
		CVegetationInstance *head = sector->first;
		obj->prev = 0;
		obj->next = head;
		head->prev = obj;
		sector->first = obj;
	}
	else
	{
		sector->first = obj;
		obj->prev = 0;
		obj->next = 0;
	}
}
	
void CVegetationMap::SectorUnlink( CVegetationInstance *obj,SectorInfo *sector )
{
	if (obj == sector->first) // if head of list.
	{
		sector->first = obj->next;
		if (sector->first)
			sector->first->prev = 0;
	}
	else
	{
		//assert( obj->prev != 0 );
		if (obj->prev)
			obj->prev->next = obj->next;
		if (obj->next)
			obj->next->prev = obj->prev;
	}
}

void CVegetationMap::AddObjInstance( CVegetationInstance *obj )
{
	SectorInfo *si = GetSector(obj->pos);
	if (!si)
	{
		obj->next = obj->prev = 0;
		return;
	}
	obj->AddRef();

	CVegetationObject *object = obj->object;
	// Add object to end of the list of instances in sector.
	// Increase number of instances.
	object->SetNumInstances( object->GetNumInstances() + 1 );
	m_numInstances++;

	SectorLink( obj,si );
}

//////////////////////////////////////////////////////////////////////////
CVegetationInstance* CVegetationMap::CreateObjInstance( CVegetationObject *object,const Vec3 &pos )
{
	SectorInfo *si = GetSector(pos);
	if (!si)
		return 0;

	CVegetationInstance *obj = new CVegetationInstance;
	obj->m_refCount = 1; // Starts with 1 reference.
	//obj->AddRef();
	obj->pos = pos;
	obj->scale = 1;
	obj->object = object;
	obj->brightness = 255;
	obj->flags = 0;

	if (CUndo::IsRecording())
		CUndo::Record( new CUndoVegInstanceCreate(obj,false) );
	
	// Add object to end of the list of instances in sector.
	// Increase number of instances.
	object->SetNumInstances( object->GetNumInstances() + 1 );
	m_numInstances++;
	
	SectorLink( obj,si );
	return obj;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::DeleteObjInstance( CVegetationInstance *obj,SectorInfo *sector )
{
	if (CUndo::IsRecording())
		CUndo::Record( new CUndoVegInstanceCreate(obj,true) );

	m_I3DEngine->RemoveStaticObject( -1, obj->pos );
	SectorUnlink(obj,sector);
	obj->object->SetNumInstances( obj->object->GetNumInstances() - 1 );
	m_numInstances--;
	assert( m_numInstances >= 0 );
	obj->Release();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::DeleteObjInstance( CVegetationInstance *obj )
{
	SectorInfo *sector = GetSector( obj->pos );
	if (sector)
		DeleteObjInstance( obj,sector );
}

//////////////////////////////////////////////////////////////////////////
CVegetationInstance* CVegetationMap::GetNearestInstance( const Vec3 &pos,float radius )
{
	// check all sectors intersected by radius.
	float r = radius*m_worldToSector;
	float px = pos.x*m_worldToSector;
	float py = pos.y*m_worldToSector;
	int sx1 = ftoi(px-r); sx1 = max(sx1,0);
	int sx2 = ftoi(px+r);	sx2 = min(sx2,m_numSectors-1);
	int sy1 = ftoi(py-r);	sy1 = max(sy1,0);
	int sy2 = ftoi(py+r);	sy2 = min(sy2,m_numSectors-1);
	
	CVegetationInstance *nearest = 0;
	float minDist = FLT_MAX;

	for (int y = sy1; y <= sy2; y++)
	{
		for (int x = sx1; x <= sx2; x++)
		{
			// For each sector check if any object is nearby.
			SectorInfo *si = GetSector(x,y);
			if (si->first)
			{
				for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
				{
					if (fabs(obj->pos.x-pos.x) < radius && fabs(obj->pos.y-pos.y) < radius)
					{
						float dist = GetSquaredDistance(pos,obj->pos);
						if (dist < minDist)
						{
							minDist = dist;
							nearest = obj;
						}
					}
				}
			}
		}
	}
	return nearest;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::GetObjectInstances( float x1,float y1,float x2,float y2,std::vector<CVegetationInstance*> &instances )
{
	instances.reserve(100);
	// check all sectors intersected by radius.
	int sx1 = ftoi(x1*m_worldToSector); sx1 = max(sx1,0);
	int sx2 = ftoi(x2*m_worldToSector);	sx2 = min(sx2,m_numSectors-1);
	int sy1 = ftoi(y1*m_worldToSector);	sy1 = max(sy1,0);
	int sy2 = ftoi(y2*m_worldToSector);	sy2 = min(sy2,m_numSectors-1);
	for (int y = sy1; y <= sy2; y++)
	{
		for (int x = sx1; x <= sx2; x++)
		{
			// For each sector check if any object is nearby.
			SectorInfo *si = GetSector(x,y);
			if (si->first)
			{
				for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
				{
					if (obj->pos.x >= x1 && obj->pos.x <= x2 && obj->pos.y >= y1 && obj->pos.y <= y2)
					{
						instances.push_back(obj);
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::GetAllInstances( std::vector<CVegetationInstance*> &instances )
{
	int k = 0;
	instances.resize( m_numInstances );
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		// Iterate on every object in sector.
		for (CVegetationInstance *obj = m_sectors[i].first; obj; obj = obj->next)
		{
			instances[k++] = obj;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationMap::IsPlaceEmpty( const Vec3 &pos,float radius,CVegetationInstance *ignore )
{
	// check all sectors intersected by radius.
	if (pos.x < 0 || pos.y < 0 || pos.x > m_mapSize || pos.y > m_mapSize)
		return false;

	// check all sectors intersected by radius.
	float r = radius*m_worldToSector;
	float px = pos.x*m_worldToSector;
	float py = pos.y*m_worldToSector;
	int sx1 = ftoi(px-r); sx1 = max(sx1,0);
	int sx2 = ftoi(px+r);	sx2 = min(sx2,m_numSectors-1);
	int sy1 = ftoi(py-r);	sy1 = max(sy1,0);
	int sy2 = ftoi(py+r);	sy2 = min(sy2,m_numSectors-1);
	for (int y = sy1; y <= sy2; y++)
	{
		for (int x = sx1; x <= sx2; x++)
		{
			// For each sector check if any object is within this radius.
			SectorInfo *si = GetSector(x,y);
			if (si->first)
			{
				for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
				{
					if (obj != ignore && fabs(obj->pos.x-pos.x) < radius && fabs(obj->pos.y-pos.y) < radius)
						return false;
				}
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::MoveInstance( CVegetationInstance* obj,const Vec3 &newPos )
{
	if (!IsPlaceEmpty(newPos,m_minimalDistance,obj))
		return;

	if (obj->pos != newPos)
		RecordUndo( obj );

	// Then delete object.
	m_I3DEngine->RemoveStaticObject( -1, obj->pos );

	SectorInfo *from = GetSector(obj->pos);
	SectorInfo *to = GetSector(newPos);
	if (from != to)
	{
		// Relink object between sectors.
		SectorUnlink( obj,from );
		if (to)
		{
			SectorLink( obj,to );
		}
	}

	obj->pos = newPos;
	// Stick vegetation to terrain.
	obj->pos.z = m_I3DEngine->GetTerrainElevation(obj->pos.x,obj->pos.y);
	m_I3DEngine->AddStaticObject( obj->object->GetId(),newPos,obj->scale,obj->brightness );
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationMap::CanPlace( CVegetationObject *object,const Vec3 &pos,float radius )
{
	// check all sectors intersected by radius.
	if (pos.x < 0 || pos.y < 0 || pos.x > m_mapSize || pos.y > m_mapSize)
		return false;

	float r = radius*m_worldToSector;
	float px = pos.x*m_worldToSector;
	float py = pos.y*m_worldToSector;
	int sx1 = ftoi(px-r); sx1 = max(sx1,0);
	int sx2 = ftoi(px+r);	sx2 = min(sx2,m_numSectors-1);
	int sy1 = ftoi(py-r);	sy1 = max(sy1,0);
	int sy2 = ftoi(py+r);	sy2 = min(sy2,m_numSectors-1);
	for (int y = sy1; y <= sy2; y++)
	{
		for (int x = sx1; x <= sx2; x++)
		{
			// For each sector check if any object is within this radius.
			SectorInfo *si = GetSector(x,y);
			if (si->first)
			{
				for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
				{
					// Only check objects that we need.
					if (obj->object == object)
					{
						if (fabs(obj->pos.x-pos.x) < radius && fabs(obj->pos.y-pos.y) < radius)
							return false;
					}
					else
					{
						if (fabs(obj->pos.x-pos.x) < m_minimalDistance && fabs(obj->pos.y-pos.y) < m_minimalDistance)
							return false;
					}
				}
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
CVegetationInstance* CVegetationMap::PlaceObjectInstance( const Vec3 &worldPos,CVegetationObject* object )
{
	float fScale = object->CalcVariableSize();
	// Check if this place is empty.
	if (CanPlace( object,worldPos,m_minimalDistance))
	{
		CVegetationInstance *obj = CreateObjInstance( object,worldPos );
		if (obj)
		{
			obj->scale = fScale;

			// Stick vegetation to terrain.
			obj->pos.z = m_I3DEngine->GetTerrainElevation(obj->pos.x,obj->pos.y);
			m_I3DEngine->AddStaticObject( object->GetId(),obj->pos,obj->scale,obj->brightness );
		}
		return obj;
	}
	return 0;
}
	
//! Remove any object at specified location.
void CVegetationMap::RemoveObjectInstance( const Vec3 &worldPos )
{
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::PaintBrush( CRect &rc,bool bCircle,CVegetationObject* object )
{
	assert( object != 0 );

	GetIEditor()->SetModifiedFlag();

	Vec3 p(0,0,0);

	int unitSize = m_heightmap->GetUnitSize();

	int mapSize = m_numSectors*m_sectorSize;

	bool bProgress = rc.right-rc.left >= mapSize;
	CWaitProgress wait("Distributing objects",bProgress);

	// Intersect with map rectangle.
	// Offset by 1 from each side.
	float brushRadius2 = (rc.right-rc.left)*(rc.right-rc.left)/4;
	rc &= CRect( 1,1,mapSize-2,mapSize-2 );

	float AltMin = object->GetElevationMin();
	float AltMax = object->GetElevationMax();
	float SlopeMin = object->GetSlopeMin();
	float SlopeMax = object->GetSlopeMax();

	float density = object->GetDensity();
	if (density <= 0)
		density = m_minimalDistance;

	int area = rc.Width() * rc.Height();
	int count = area / (density*density);
	
	// Limit from too much objects.
	if (count > area)
		count = area;

	int j = 0;

	float x1 = rc.left;
	float y1 = rc.top;
	float width2 = (rc.right-rc.left)/2.0f;
	float height2 = (rc.bottom-rc.top)/2.0f;

	float cx = (rc.right+rc.left)/2.0f;
	float cy = (rc.bottom+rc.top)/2.0f;

	// Calculate the vegetation for every point in the area marked by the brush
	for (int i = 0; i < count; i++)
	{
		if (bProgress)
		{
			j++;
			if (j > 200)
			{
				if (!wait.Step( 100*i/count ))
					break;
			}
		}

		float x = x1 + (frand()+1)*width2;
		float y = y1 + (frand()+1)*height2;

		// Skip all coordinates outside the brush circle
		if (bCircle) {
			if (((x-cx)*(x-cx) + (y-cy)*(y-cy)) > brushRadius2)
				continue;
		}
			
		// Get the height of the current point
		// swap x/y
		int hy = ftoi(x/unitSize);
		int hx = ftoi(y/unitSize);

		float currHeight = m_heightmap->GetXY(hx,hy);
		// Check if height valie is withing brush min/max altitude.
		if (currHeight < AltMin || currHeight > AltMax)
			continue;

		// Calculate the slope for this spot
		float slope = m_heightmap->GetSlope( hx,hy );

		// Check if slope is withing brush min/max slope.
		if (slope < SlopeMin || slope > SlopeMax)
			continue;

		p.x = x;
		p.y = y;
		float fScale = object->CalcVariableSize();
		float placeRadius = fScale*object->GetObjectSize()*0.5f;
		// Check if this place is empty.
		if (!CanPlace( object,p,placeRadius ))
			continue;

		p.z = m_I3DEngine->GetTerrainElevation(p.x,p.y);
		CVegetationInstance *obj = CreateObjInstance( object,p );
		if (obj)
		{
			obj->scale = fScale;
			m_I3DEngine->AddStaticObject( object->GetId(),p,fScale,obj->brightness );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::PaintBrightness( float x,float y,float w,float h,uchar brightness,uchar brightness_shadowmap )
{
	// Find sector range from world positions.
	int startSectorX = ftoi(x*m_worldToSector);
	int startSectorY = ftoi(y*m_worldToSector);
	int endSectorX = ftoi((x + w)*m_worldToSector);
	int endSectorY = ftoi((y + h)*m_worldToSector);

	// Clamp start and end sectors to valid range.
	if (startSectorX < 0)
		startSectorX = 0;
	if (startSectorY < 0)
		startSectorY = 0;
	if (endSectorX >= m_numSectors)
		endSectorX = m_numSectors-1;
	if (endSectorY >= m_numSectors)
		endSectorY = m_numSectors-1;

	float x2 = x+w;
	float y2 = y+h;

	// Iterate all sectors in range.
	for (int sy = startSectorY; sy <= endSectorY; sy++)
	{
		for (int sx = startSectorX; sx <= endSectorX; sx++)
		{
			// Iterate all objects in sector.
			SectorInfo *si = &m_sectors[sy*m_numSectors + sx];
			if (!si->first)
				continue;
			for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
			{
				if (obj->pos.x >= x && obj->pos.x < x2 && obj->pos.y >= y && obj->pos.y <= y2)
				{
					bool bNeedUpdate = false;
					if (!obj->object->IsPrecalcShadow())
					{
						if (obj->brightness != brightness_shadowmap)
							bNeedUpdate = true;
						// If object is not casting precalculated shadow (small grass etc..) affect it by shadow map.
						obj->brightness = brightness_shadowmap;
					}
					else
					{
						if (obj->brightness != brightness)
							bNeedUpdate = true;
						obj->brightness = brightness;
					}

					if (m_bUpdateOnPaintBrightness && bNeedUpdate)
					{
						RepaintInstance( obj );
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ClearBrush( CRect &rc,bool bCircle,CVegetationObject* object )
{
	GetIEditor()->SetModifiedFlag();

	Vec3 p(0,0,0);

	int unitSize = m_heightmap->GetUnitSize();

	int mapSize = m_numSectors*m_sectorSize;

	// Intersect with map rectangle.
	// Offset by 1 from each side.
	float brushRadius2 = (rc.right-rc.left)*(rc.right-rc.left)/4;
	float cx = (rc.right+rc.left)/2;
	float cy = (rc.bottom+rc.top)/2;

	float x1 = rc.left;
	float y1 = rc.top;
	float x2 = rc.right;
	float y2 = rc.bottom;

	// check all sectors intersected by radius.
	int sx1 = ftoi(x1*m_worldToSector);
	int sx2 = ftoi(x2*m_worldToSector);
	int sy1 = ftoi(y1*m_worldToSector);
	int sy2 = ftoi(y2*m_worldToSector);
	sx1 = max(sx1,0);
	sx2 = min(sx2,m_numSectors-1);
	sy1 = max(sy1,0);
	sy2 = min(sy2,m_numSectors-1);

	CVegetationInstance *next = 0;
	for (int y = sy1; y <= sy2; y++)
	{
		for (int x = sx1; x <= sx2; x++)
		{
			// For each sector check if any object is within this radius.
			SectorInfo *si = GetSector(x,y);
			if (si->first)
			{
				for (CVegetationInstance *obj = si->first; obj; obj = next)
				{
					next = obj->next;
					if (obj->object != object && object != NULL)
						continue;

					if (bCircle)
					{
						// Skip objects outside the brush circle
						if (((obj->pos.x-cx)*(obj->pos.x-cx) + (obj->pos.y-cy)*(obj->pos.y-cy)) > brushRadius2)
							continue;
					}
					else
					{
						// Withing rectangle.
						if (obj->pos.x < x1 || obj->pos.x > x2 || obj->pos.y < y1 || obj->pos.y > y2)
							continue;
					}

					// Then delete object.
					DeleteObjInstance( obj,si );
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ClearMask( const CString &maskFile )
{
	// Swap x/y
	Vec3 p(0,0,0);

	CLayer layer;
	layer.SetAutoGen(false);
	layer.SetSmooth(false);
	if (!layer.LoadMask( maskFile ))
	{
		CString str;
		str.Format( "Error loading mask file %s",(const char*)maskFile );
		AfxGetMainWnd()->MessageBox( str,"Warning",MB_OK|MB_ICONEXCLAMATION );
		return;
	}
	int layerSize = m_numSectors;
	layer.UpdateLayerMask16( 0,layerSize,layerSize,false );
	if (!layer.IsValid())
		return;

	GetIEditor()->SetModifiedFlag();
	CWaitProgress wait( "Clearing mask" );

	for (int y = 0; y < layerSize; y++)
	{
		if (!wait.Step( 100*y/layerSize ))
			break;
		for (int x = 0; x < layerSize; x++)
		{
			if (layer.GetLayerMaskPoint(x,y) > MIN_MASK_VALUE)
			{
				// Find sector.
				// Swap X/Y.
				SectorInfo *si = &m_sectors[y + x*m_numSectors];
				// Delete all instances in this sectors.
				CVegetationInstance *next = 0;
				for (CVegetationInstance *obj = si->first; obj; obj = next)
				{
					next = obj->next;
					DeleteObjInstance( obj,si );
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CVegetationObject* CVegetationMap::CreateObject( CVegetationObject *prev )
{
	int id = -1;
	// Generate New id.
	// Cannot create more then 256 objects.
	for (int i = 0; i < 256; i++)
	{
		if (m_usedIds.find(i) == m_usedIds.end())
		{
			id = i;
			break;
		}
	}
	if (id < 0)
	{
		// Free id not found, created more then 256 objects
		AfxMessageBox( _T("Vegetation objects limit is reached.\r\nMaximum of 256 vegetation objects are supported."),MB_OK|MB_ICONWARNING );
		return 0;
	}
	
	// Mark id as used.
	m_usedIds.insert(id);

	CVegetationObject *obj = new CVegetationObject( id,this );
	if (prev)
		obj->CopyFrom( *prev );

	m_objects.push_back( obj );
	return obj;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::RemoveObject( CVegetationObject *object )
{
	// Free id for this object.
	m_usedIds.erase( object->GetId() );

	if (object->GetNumInstances() > 0)
	{
		CVegetationInstance *next = 0;
		// Delete this object in sectors.
		for (int i = 0; i < m_numSectors*m_numSectors; i++)
		{
			SectorInfo *si = &m_sectors[i];
			// Iterate on every object in sector.
			for (CVegetationInstance *obj = si->first; obj; obj = next)
			{
				next = obj->next;
				if (obj->object == object)
				{
					DeleteObjInstance( obj,si );
				}
			}
		}
	}
	Objects::iterator it = std::find(m_objects.begin(),m_objects.end(),object);
	if (it != m_objects.end())
		m_objects.erase( it );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ReplaceObject( CVegetationObject *pOldObject,CVegetationObject *pNewObject )
{
	if (pOldObject->GetNumInstances() > 0)
	{
		pNewObject->SetNumInstances( pNewObject->GetNumInstances() + pOldObject->GetNumInstances() );
		CVegetationInstance *next = 0;
		// Delete this object in sectors.
		for (int i = 0; i < m_numSectors*m_numSectors; i++)
		{
			SectorInfo *si = &m_sectors[i];
			// Iterate on every object in sector.
			for (CVegetationInstance *obj = si->first; obj; obj = next)
			{
				next = obj->next;
				if (obj->object == pOldObject)
				{
					obj->object = pNewObject;
				}
			}
		}
	}
	RemoveObject( pOldObject );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ClearObjects()
{
	ClearSectors();
	m_objects.clear();
}


//////////////////////////////////////////////////////////////////////////
void CVegetationMap::RepaintInstance( CVegetationInstance *obj )
{
	m_I3DEngine->RemoveStaticObject( -1, obj->pos );
	m_I3DEngine->AddStaticObject( obj->object->GetId(),obj->pos,obj->scale,obj->brightness );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::RepositionObject( CVegetationObject *object )
{
	// Iterator over all sectors.
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		SectorInfo *si = &m_sectors[i];
		// Iterate on every object in sector.
		for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
		{
			if (obj->object == object)
			{
				RepaintInstance( obj );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ScaleObjectInstances( CVegetationObject *object,float fScale )
{
	// Iterator over all sectors.
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		SectorInfo *si = &m_sectors[i];
		// Iterate on every object in sector.
		for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
		{
			if (obj->object == object)
			{
				obj->scale *= fScale;
				RepaintInstance( obj );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::Serialize( CXmlArchive &xmlAr )
{
	int i;
	if (xmlAr.bLoading)
	{
		CLogFile::WriteLine("Loading Vegetation Map...");

		ClearObjects();

		CWaitProgress progress( _T("Loading Static Objects") );

		XmlNodeRef mainNode = xmlAr.root->findChild( "VegetationMap" );
		if (mainNode)	
		{
			XmlNodeRef objects = mainNode->findChild( "Objects" );
			if (objects)
			{
				int numObjects = objects->getChildCount();
				for (i = 0; i < numObjects; i++)
				{
					if (!progress.Step( 100*i/numObjects ))
						break;
					CVegetationObject *obj = CreateObject();
					if (obj)
						obj->Serialize( objects->getChild(i),xmlAr.bLoading );
				}
			}
		}

		SerializeInstances( xmlAr );

		LoadOldStuff( xmlAr );

		// Now display all objects on terrain.
		PlaceObjectsOnTerrain();
	}
	else
	{
		// Storing
		CLogFile::WriteLine("Saving Vegetation Map...");

		//xmlAr.pNamedData->AddDataBlock( "StatObjectsArray",m_map.GetData(),m_width*m_height );

		XmlNodeRef mainNode = xmlAr.root->newChild( "VegetationMap" );

		// Save objects.
		XmlNodeRef objects = mainNode->newChild( "Objects" );
		for (i = 0; i < GetObjectCount(); i++)
		{
			XmlNodeRef obj = objects->newChild( "Object" );
			GetObject(i)->Serialize(obj,xmlAr.bLoading);
		}

		// Store objects.
		SerializeInstances( xmlAr );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::SerializeInstances( CXmlArchive &xmlAr,CRect *saveRect )
{
	int i;
	if (xmlAr.bLoading)
	{
		Vec3 posofs(0,0,0);
		// Loading.
		if (!saveRect)
		{
			ClearSectors();
		}
		else
		{
			posofs.x = saveRect->left;
			posofs.y = saveRect->top;
		}

		int numObjects = m_objects.size();
		int arraySize;
		void *pData = 0;
		if (!xmlAr.pNamedData->GetDataBlock( "VegetationInstancesArray",pData,arraySize ))
			return;
		SVegInst *array = (SVegInst*)pData;
		int numInst = arraySize / sizeof(SVegInst);
		for (int i = 0; i < numInst; i++)
		{
			if (array[i].objectIndex >= numObjects)
				continue;
			CVegetationObject *object = m_objects[array[i].objectIndex];
			CVegetationInstance *obj = CreateObjInstance( object,array[i].pos+posofs );
			if (obj)
			{
				obj->scale = array[i].scale;
				obj->brightness = array[i].brightness;
				obj->flags = array[i].flags;
			}
		}

		//assert( m_numInstances == numInst );
	}
	else
	{
		// Saving.
		if (m_numInstances == 0)
			return;
		int arraySize = sizeof(SVegInst)*m_numInstances;
		SVegInst *array = (SVegInst*)malloc( arraySize );

		int k = 0;

		// Assign indices to objects.
		for (i = 0; i < GetObjectCount(); i++)
		{
			GetObject(i)->SetIndex(i);
		}

		float x1,y1,x2,y2;
		if (saveRect)
		{
			x1 = saveRect->left;
			y1 = saveRect->top;
			x2 = saveRect->right;
			y2 = saveRect->bottom;
		}

		// Fill array.
		for (i = 0; i < m_numSectors*m_numSectors; i++)
		{
			SectorInfo *si = &m_sectors[i];
			// Iterate on every object in sector.
			for (CVegetationInstance *obj = si->first; obj; obj = obj->next)
			{
				if (saveRect)
				{
					if (obj->pos.x < x1 || obj->pos.x > x2 || obj->pos.y < y1 || obj->pos.y > y2)
						continue;
				}
				array[k].pos = obj->pos;
				array[k].scale = obj->scale;
				array[k].brightness = obj->brightness;
				array[k].flags = obj->flags;
				array[k].objectIndex = obj->object->GetIndex();
				k++;
			}
		}
		// Save this array.
		xmlAr.pNamedData->AddDataBlock( "VegetationInstancesArray",array,k*sizeof(SVegInst) );

		free( array );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::LoadOldStuff( CXmlArchive &xmlAr )
{
	XmlNodeRef mainNode = xmlAr.root->findChild( "VegetationMap" );
	if (!mainNode)	
		return;

	int i;
	// Backward compatability loading of brushes.
	XmlNodeRef brushes = mainNode->findChild( "Brushes" );
	if (brushes)
	{
		for (i = 0; i < brushes->getChildCount(); i++)
		{
			CVegetationBrush br;
			br.Serialize( brushes->getChild(i),xmlAr.bLoading );
		}
	}

	void *pData = 0;
	int nSize;
	if (xmlAr.pNamedData->GetDataBlock( "StatObjectsArray",pData,nSize ))
	{
		if (nSize != 2048*2048)
			return;

		int numObjects = m_objects.size();
		CVegetationObject* usedObjects[256];
		ZeroStruct(usedObjects);

		for (i = 0; i < m_objects.size(); i++)
		{
			usedObjects[m_objects[i]->GetIndex()] = m_objects[i];
		}


		Vec3 pos;
		int i = 0;
		uchar *pMap = (uchar*)pData;
		for (int y = 0; y < 2048; y++)
		{
			for (int x = 0; x < 2048; x++,i++)
			{
				i = x + 2048*y;
				if (!pMap[i])
					continue;

				unsigned int objIndex = pMap[i] - 1;
				if (!usedObjects[objIndex])
					continue;

				//Swap x/y
				pos.x = y;
				pos.y = x;
				pos.z = m_I3DEngine->GetTerrainElevation(pos.x,pos.y);
				CVegetationInstance *obj = CreateObjInstance( usedObjects[objIndex],pos );
				if (obj)
				{
					obj->scale = usedObjects[objIndex]->CalcVariableSize();
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//! Generate shadows from static objects and place them in shadow map bitarray.
void CVegetationMap::GenerateShadowMap( CByteImage &shadowmap,float shadowAmmount,const Vec3 &sunVector )
{
	//	if(!m_pTerrain)
	//	return;

	int width = shadowmap.GetWidth();
	int height = shadowmap.GetHeight();
	int i =0;

	//@FIXME: Hardcoded.
	int sectorSizeInMeters = 64;

	int unitSize = m_heightmap->GetUnitSize();
	int numSectors = (m_heightmap->GetWidth() * unitSize) / sectorSizeInMeters;

	int sectorSize = shadowmap.GetWidth()/numSectors;
	int sectorSize2 = sectorSize*2;
	assert( sectorSize > 0 );

	uint shadowValue = shadowAmmount;
	
	bool bProgress = width >= 2048;
	CWaitProgress wait( "Calculating Objects Shadows",bProgress );

	unsigned char *sectorImage = (unsigned char*)malloc(sectorSize*sectorSize*3);
	unsigned char *sectorImage2 = (unsigned char*)malloc(sectorSize2*sectorSize2*3);

	for (int y = 0; y < numSectors; y++)
	{
		if (bProgress)
		{
			if (!wait.Step( y*100/numSectors ))
				break;
		}

		for (int x = 0; x < numSectors; x++)
		{
			m_I3DEngine->MakeSectorLightMap( y*sectorSizeInMeters,x*sectorSizeInMeters,sectorImage2,sectorSize2 );

			// Scale sector image down and store into the shadow map.
			{
				int pos;
				uint color;
				int x1 = x*sectorSize;
				int y1 = y*sectorSize;
				for (int j = 0; j < sectorSize; j++)
				{
					int sx1 = x1+(sectorSize-j-1);
					for (int i = 0; i < sectorSize; i++)
					{
						pos = (i + j*sectorSize2)*2*3;
						color = (shadowValue*
							((uint)
							(255-sectorImage2[pos]) + 
							(255-sectorImage2[pos+3]) +
							(255-sectorImage2[pos+sectorSize2*3]) +
							(255-sectorImage2[pos+sectorSize2*3+3])
							)) >> 10;
//						color = color*shadowValue >> 8;
						// swap x/y
						//color = (255-sectorImage2[(i+j*sectorSize)*3]);
						shadowmap.ValueAt(sx1,y1+i) = color;
					}
				}
			}
		}
	}
	free( sectorImage );
	free( sectorImage2 );
}

//////////////////////////////////////////////////////////////////////////
int CVegetationMap::ExportObject( CVegetationObject *object,XmlNodeRef &node,CRect *saveRect )
{
	int numSaved = 0;
	assert( object != 0 );
	object->Serialize( node,false );
	if (object->GetNumInstances() > 0)
	{
		float x1,y1,x2,y2;
		if (saveRect)
		{
			x1 = saveRect->left;
			y1 = saveRect->top;
			x2 = saveRect->right;
			y2 = saveRect->bottom;
		}
		// Export instances.
		XmlNodeRef instancesNode = node->newChild("Instances");
		// Iterator over all sectors.
		for (int i = 0; i < m_numSectors*m_numSectors; i++)
		{
			// Iterate on every object in sector.
			for (CVegetationInstance *obj = m_sectors[i].first; obj; obj = obj->next)
			{
				if (obj->object == object)
				{
					if (saveRect)
					{
						if (obj->pos.x < x1 || obj->pos.x > x2 || obj->pos.y < y1 || obj->pos.y > y2)
							continue;
					}
					numSaved++;
					XmlNodeRef inst = instancesNode->newChild( "Instance" );
					inst->setAttr( "Pos",obj->pos );
					if (obj->scale != 1)
						inst->setAttr( "Scale",obj->scale );
					if (obj->brightness != 255)
						inst->setAttr( "Brightness",(int)obj->brightness );
					if (obj->flags != 0)
						inst->setAttr( "Flags",(int)obj->flags );
				}
			}
		}
	}
	return numSaved;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ImportObject( XmlNodeRef &node,const Vec3& offset )
{
	CVegetationObject *object = CreateObject();
	if (!object)
		return;
	object->Serialize( node,true );

	// Check if object with this GUID. already exists.
	for (int i = 0; i < GetObjectCount(); i++)
	{
		CVegetationObject *pOldObject = GetObject(i);
		if (pOldObject == object)
			continue;
		if (pOldObject->GetGUID() == object->GetGUID())
		{
			ReplaceObject( pOldObject,object );
			/*
			// 2 objects have same GUID;
			CString msg;
			msg.Format( _T("Vegetation Object %s %s already exists in the level!.\r\nOverride existing object?"),
										GuidUtil::ToString(pOldObject->GetGUID()),(const char*)pOldObject->GetFileName() );

			if (AfxMessageBox( msg,MB_YESNO|MB_ICONWARNING ) == IDYES)
			{
				ReplaceObject( pOldObject,object );
			}
			else
			{
				RemoveObject( object );
				object = pOldObject;
			}
			*/
			break;
		}
	}

	Vec3 pos(0,0,0);
	float scale = 1;
	int brightness = 255;
	int flags = 0;

	XmlNodeRef instancesNode = node->findChild("Instances");
	if (instancesNode)
	{
		int numChilds = instancesNode->getChildCount();
		for (int i = 0; i < numChilds; i++)
		{
			pos.Set(0,0,0);
			scale = 1;
			brightness = 255;
			flags = 0;

			XmlNodeRef inst = instancesNode->getChild(i);
			inst->getAttr( "Pos",pos );
			inst->getAttr( "Scale",scale );
			inst->getAttr( "Brightness",brightness );
			inst->getAttr( "Flags",flags );

			pos += offset;
			CVegetationInstance *obj = GetNearestInstance( pos,0.01f );
			if (obj && obj->pos == pos)
			{
				// Delete pevious object at same position.
				DeleteObjInstance( obj );
			}
			obj = CreateObjInstance( object,pos );
			if (obj)
			{
				obj->scale = scale;
				obj->brightness = brightness;
				obj->flags = flags;
				// Stick vegetation to terrain.
				obj->pos.z = m_I3DEngine->GetTerrainElevation(obj->pos.x,obj->pos.y);
			}
		}
	}
	RepositionObject(object);
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::DrawToTexture( uint *texture,int texWidth,int texHeight,int srcX,int srcY )
{
	assert( texture != 0 );

	for (int y = 0; y < texHeight; y++)
	{
		int trgYOfs = y*texWidth;
		for (int x = 0; x < texWidth; x++)
		{
			int sx = x + srcX;
			int sy = y + srcY;
			// Swap X/Y
			SectorInfo *si = &m_sectors[sy + sx*m_numSectors];
			if (si->first)
				texture[x+trgYOfs] = 0xFFFFFFFF;
			else
				texture[x+trgYOfs] = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CVegetationMap::WorldToSector( float worldCoord ) const
{
	return ftoi(worldCoord*m_worldToSector);
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::UnloadObjectsGeometry()
{
	for (int i = 0; i < GetObjectCount(); i++)
	{
		GetObject(i)->UnloadObject();
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ExportBlock( const CRect &subRc,CXmlArchive &ar )
{
	XmlNodeRef mainNode = ar.root->newChild( "VegetationMap" );
	mainNode->setAttr( "X1",subRc.left );
	mainNode->setAttr( "Y1",subRc.top );
	mainNode->setAttr( "X2",subRc.right );
	mainNode->setAttr( "Y2",subRc.bottom );

	CRect rect = subRc;
	for (int i = 0; i < GetObjectCount(); i++)
	{
		XmlNodeRef vegObjNode = mainNode->createNode( "VegetationObject" );
		CVegetationObject *pObject = GetObject(i);
		int numSaved = ExportObject( pObject,vegObjNode,&rect );
		if (numSaved > 0)
		{
			mainNode->addChild( vegObjNode );
		}
	}

	//SerializeInstances( ar,&rect );
}
	
//////////////////////////////////////////////////////////////////////////
void CVegetationMap::ImportBlock( CXmlArchive &ar,CPoint placeOffset )
{
	XmlNodeRef mainNode = ar.root->findChild( "VegetationMap" );
	if (!mainNode)
		return;

	CRect subRc( 0,0,1,1 );
	mainNode->getAttr( "X1",subRc.left );
	mainNode->getAttr( "Y1",subRc.top );
	mainNode->getAttr( "X2",subRc.right );
	mainNode->getAttr( "Y2",subRc.bottom );

	subRc.OffsetRect(placeOffset);

	// Clear all vegetation instances in this rectangle.
	ClearBrush( subRc,false,NULL );

	// Serialize vegitation objects.
	for (int i = 0; i < mainNode->getChildCount(); i++)
	{
		XmlNodeRef vegObjNode = mainNode->getChild(i);
		ImportObject( vegObjNode,Vec3(placeOffset.x,placeOffset.y,0) );
	}
	// Clear object in this rectangle.
	/*
	CRect rect(placeOffset.x,placeOffset.y,placeOffset.x,placeOffset.y);
	SerializeInstances( ar,&rect );
	*/

	// Now display all objects on terrain.
	PlaceObjectsOnTerrain();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::RecordUndo( CVegetationInstance *obj )
{
	if (CUndo::IsRecording())
		CUndo::Record( new CUndoVegInstance(obj) );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationMap::Validate( CErrorReport &report )
{
	for (int i = 0; i < GetObjectCount(); i++)
	{
		GetObject(i)->Validate( report );
	}
}