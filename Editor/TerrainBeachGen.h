////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrainbeachgen.h
//  Version:     v1.00
//  Created:     1/7/2003 by Vladimir.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __terrainbeachgen_h__
#define __terrainbeachgen_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
// Dynamic replacement for static 2d array
//////////////////////////////////////////////////////////////////////////
template <class T> struct Array2d
{
	Array2d() 
	{ 
		m_nSize = 0; 
		m_pData = 0;
	}

	void GetMemoryUsage(ICrySizer*pSizer)
	{
		pSizer->AddObject (m_pData, m_nSize*m_nSize*sizeof(T));
	}

	void Allocate(int nSize) 
	{ 
		if(m_nSize == nSize)
			return;

		delete m_pData;

		m_nSize = nSize; 
		m_pData = new T [nSize*nSize];
		memset(m_pData, 0, nSize*nSize*sizeof(T));
	}

	~Array2d() 
	{ 
		delete [] m_pData;
	}

	T * m_pData;
	int m_nSize;

	T * operator [] (const int & nPos) const
	{
		assert(nPos>=0 && nPos<m_nSize);
		return &m_pData[nPos*m_nSize];
	}

	void operator = (const Array2d & other)
	{
		Allocate(other.m_nSize);
		memcpy(m_pData,other.m_pData,m_nSize*m_nSize*sizeof(T));
	}
};

//////////////////////////////////////////////////////////////////////////
struct CTerrainSectorBeachInfo
{
	bool m_bBeachPresent;

	struct BeachPairStruct 
	{ 
		BeachPairStruct () { ZeroStruct(*this); }
		Vec3d pos, water_dir; float distance; int busy; 
		Vec3d pos1,posm,pos2;
	};
#define MAX_BEACH_GROUPS 16
	std::vector<BeachPairStruct> m_arrlistBeachVerts[MAX_BEACH_GROUPS];
	std::vector<BeachPairStruct> m_lstUnsortedBeachVerts; // tmp

	CLeafBuffer * m_pLeafBufferBeach;
	int m_nOriginX, m_nOriginY; // sector origin
};

// Shore geometry generator
class CTerrainBeachGenerator
{
public:
	CTerrainBeachGenerator( CHeightmap *pTerrain);
	void Generate( CFile &file );

private:
	int MarkWaterAreas();
	void MakeBeachStage1( CTerrainSectorBeachInfo *pSector );
	void MakeBeachStage2( CTerrainSectorBeachInfo *pSector );

	float GetZSafe( int x,int y );
	float GetZSafe( float x,float y );
	float GetZApr(float x1, float y1);

	int GetSecIndex( CTerrainSectorBeachInfo *pSector )
	{
		return (pSector->m_nOriginX/m_sectorSize)*m_sectorTableSize + (pSector->m_nOriginY/m_sectorSize); }

	CHeightmap* m_pTerrain;
	struct CBeachInfo { bool beach,in_water; };
	Array2d<CBeachInfo> m_arrBeachMap;
	Array2d<char>  m_WaterAreaMap;
	std::vector<int> m_lstWaterAreaSizeTable;
	std::vector<CTerrainSectorBeachInfo*> m_sectors;
	int m_nMaxAreaSize;

	int m_sectorTableSize;
	int m_sectorSize;
	int m_unitSize;
	int m_terrainSize;
	int m_heightmapSize;
	float m_waterLevel;
	float m_fShoreSize;
	float* m_pHeightmapData;

	CFile *m_pFile;
};

#endif // __terrainbeachgen_h__
