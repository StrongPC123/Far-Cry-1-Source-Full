////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terraingrid.h
//  Version:     v1.00
//  Created:     14/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __terraingrid_h__
#define __terraingrid_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CHeightmap;

class CTerrainTexGen;

/** Represent single terrain sector.
*/
class CTerrainSector
{
public:
	CTerrainSector()
	{
		textureId = 0;
	}

public:
	//! Id of surface texture.
	unsigned int textureId;
};

/** Manages grid of terrain sectors.
*/
class CTerrainGrid
{
public:
	CTerrainGrid( CHeightmap* heightmap );
	~CTerrainGrid();

	//! Initialize grid.
	void InitSectorGrid( int numSectors );
	
	//! Set target texture resolution.
	void SetResolution( int resolution );

	//! Clear all sectors.
	void ReleaseSectorGrid();

	//! Get terrain sector.
	CTerrainSector* GetSector( CPoint sector );

	//! Lock texture of sector and return texture id.
	int LockSectorTexture( CPoint sector,CTerrainTexGen &texGen,int genFlags );

	//////////////////////////////////////////////////////////////////////////
	// Coordinate conversions.
	//////////////////////////////////////////////////////////////////////////
	void GetSectorRect( CPoint sector,CRect &rect );
	//! Map from sector coordinates to texture coordinates.
	CPoint SectorToTexture( CPoint sector );
	//! Map world position to sector space.
	CPoint WorldToSector( const Vec3 &wp );
	//! Map sector coordinates to world coordinates.
	Vec3 SectorToWorld( CPoint sector );
	//! Map world position to texture space.
	CPoint WorldToTexture( const Vec3 &wp );

	//! Return number of sectors per side.
	int GetNumSectors() const { return m_numSectors; }


private:
	//! Heightmap for this terrain.
	CHeightmap *m_heightmap;

	//! Sector grid.
	std::vector<CTerrainSector*> m_sectorGrid;

	//! Number of sectors per side.
	int m_numSectors;
	
	//! Resolution of texture and heightmap.
	int m_resolution;

	//! Sector texture size.
	int m_sectorResolution;

	//! Size of sector in meters.
	int m_sectorSize;

	//! Texture Pixels per meter.
	float m_pixelsPerMeter;
};

#endif // __terraingrid_h__
