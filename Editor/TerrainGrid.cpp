////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terraingrid.cpp
//  Version:     v1.00
//  Created:     14/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TerrainGrid.h"

#include "Heightmap.h"
#include "TerrainTexGen.h"

#include <I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
CTerrainGrid::CTerrainGrid( CHeightmap* heightmap )
{
	m_heightmap = heightmap;
	m_numSectors = 0;
	m_resolution = 0;
	m_sectorSize = 0;
	m_sectorResolution = 0;
	m_pixelsPerMeter = 0;
}

//////////////////////////////////////////////////////////////////////////
CTerrainGrid::~CTerrainGrid()
{
	ReleaseSectorGrid();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::InitSectorGrid( int numSectors )
{
	ReleaseSectorGrid();
	m_numSectors = numSectors;
	m_sectorGrid.resize( m_numSectors*m_numSectors );
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		m_sectorGrid[i] = 0;
	}

	SSectorInfo si;
	m_heightmap->GetSectorsInfo( si );
	m_sectorSize = si.sectorSize;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::ReleaseSectorGrid()
{
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		if (m_sectorGrid[i])
			delete m_sectorGrid[i];
	}
	m_sectorGrid.clear();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::SetResolution( int resolution )
{
	m_resolution = resolution;
	m_sectorResolution = m_resolution / m_numSectors;
	m_pixelsPerMeter = ((float)m_sectorResolution) / m_sectorSize;
}

//////////////////////////////////////////////////////////////////////////
CTerrainSector* CTerrainGrid::GetSector( CPoint sector )
{
	assert( sector.x >= 0 && sector.x < m_numSectors && sector.y >= 0 && sector.y < m_numSectors );
	int index = sector.x + sector.y*m_numSectors;
	if (!m_sectorGrid[index])
		m_sectorGrid[index] = new CTerrainSector;
	return m_sectorGrid[index];
}

//////////////////////////////////////////////////////////////////////////
CPoint CTerrainGrid::SectorToTexture( CPoint sector )
{
	return CPoint( sector.x*m_sectorResolution,sector.y*m_sectorResolution );
}

//////////////////////////////////////////////////////////////////////////
CPoint CTerrainGrid::WorldToSector( const Vec3 &wp )
{
	//swap x/y
	return CPoint( int(wp.y / m_sectorSize),int(wp.x / m_sectorSize) );
}

//////////////////////////////////////////////////////////////////////////
Vec3 CTerrainGrid::SectorToWorld( CPoint sector )
{
	//swap x/y
	return Vec3(sector.y*m_sectorSize,sector.x*m_sectorSize,0 );
}

//////////////////////////////////////////////////////////////////////////
CPoint CTerrainGrid::WorldToTexture( const Vec3 &wp )
{
	//swap x/y
	return CPoint( int(wp.y*m_pixelsPerMeter),int(wp.x*m_pixelsPerMeter) );
}

//////////////////////////////////////////////////////////////////////////
void CTerrainGrid::GetSectorRect( CPoint sector,CRect &rect )
{
	rect.left = sector.x * m_sectorResolution;
	rect.top = sector.y * m_sectorResolution;
	rect.right = rect.left + m_sectorResolution;
	rect.bottom = rect.top + m_sectorResolution;
}

//////////////////////////////////////////////////////////////////////////
int CTerrainGrid::LockSectorTexture( CPoint sector,CTerrainTexGen &texGen,int genFlags )
{
	CTerrainSector *st = GetSector(sector);
	assert( st );
	if (!st->textureId)
	{
		// If textureID is not yet initialized, generate texture for it.
		int sectorTexSize;
		//st->textureId = m_3DEngine->LockTerrainSectorTexture( m_pointerPos.x,m_pointerPos.y,sectorTexSize );

		I3DEngine *p3Engine = GetIEditor()->Get3DEngine();
		IRenderer *pRenderer = GetIEditor()->GetRenderer();
		
		Vec3 wp = SectorToWorld(sector);
		st->textureId = p3Engine->LockTerrainSectorTexture( wp.x+0.1f,wp.y+0.1f,sectorTexSize );
		if (st->textureId != 0)
		{
			// Texture id already present, need to load to it uncompressed copy.
			CImage image;
			image.Allocate( m_sectorResolution,m_sectorResolution );
			CRect rect( 0,0,m_sectorResolution,m_sectorResolution );

			texGen.GenerateSectorTexture( sector,rect,genFlags|ETTG_ABGR,image );

			unsigned char *pSrc = (unsigned char*)image.GetData();
			int tid = pRenderer->DownLoadToVideoMemory( pSrc,m_sectorResolution,m_sectorResolution,eTF_8888,eTF_8888,0,0,0,st->textureId );
			//pRenderer->UpdateTextureInVideoMemory( tid,pSrc,0,0,m_sectorResolution,m_sectorResolution,eTF_8888 );

			// Check if render did not change texture id.
			assert( tid == st->textureId );
		}
	}
	return st->textureId;
}