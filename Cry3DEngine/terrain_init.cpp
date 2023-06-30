////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: init
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_sector.h"
#include "StatObj.h"
#include "objman.h"
#include "cbuffer.h"
#include "terrain_water.h"
#include "detail_grass.h"

int CTerrain::m_nUnitSize    = 2;
float CTerrain::m_fInvUnitSize = 1.0f/2.0f;
int CTerrain::m_nTerrainSize = 2048; 
int CTerrain::m_nSectorSize  = 64;
int CTerrain::m_nSectorsTableSize = 32; 

//#define TERR_ERROR_METR_FILENAME "terrain\\err_metr_7.tmp"
#define TERR_BEACHES_FILENAME "beach7.tmp"

void CTerrain::InitBeaches(bool bEditorMode)
{
  if(!GetCVars()->e_beach || !m_pSHShore)
    return;

  if( !bEditorMode && //0 &&
    CXFile::IsFileExist(GetLevelFilePath(TERR_BEACHES_FILENAME)) && 
//   !CXFile::IsOutOfDate(GetLevelFilePath(TERR_BEACHES_FILENAME), GetLevelFilePath("terrain\\LAND_MAP.H16")) &&   
    CXFile::GetLength  (GetLevelFilePath(TERR_BEACHES_FILENAME))>0 )
  {
    UpdateLoadingScreen("  Loading beaches ...");
    
    FILE * hBeachReadFile = GetSystem()->GetIPak()->FOpen(GetLevelFilePath(TERR_BEACHES_FILENAME), "rb");

    for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
    for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
      m_arrSecInfoTable[x][y]->LoadBeach(hBeachReadFile);

    GetSystem()->GetIPak()->FClose(hBeachReadFile);
  }
  else
  {
    UpdateLoadingScreen("  Calculating beaches ...");

    if(!m_pBeachGenerator)
      m_pBeachGenerator = new CBeachGenerator(this);

    int nAreasFound = m_pBeachGenerator->MarkWaterAreas();
//    UpdateLoadingScreenPlus(" %d areas found",nAreasFound);

    int x;
    for(     x=0; x<CTerrain::GetSectorsTableSize(); x++)
    for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
      m_arrSecInfoTable[x][y]->MakeBeachStage1();

    FILE * hBeachFile = GetSystem()->GetIPak()->FOpen(GetLevelFilePath(TERR_BEACHES_FILENAME), "wb");
    if(bEditorMode && !hBeachFile)
      GetLog()->Log("Error opening %s for writing", TERR_BEACHES_FILENAME);

    for(     x=0; x<CTerrain::GetSectorsTableSize(); x++)
    for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
      m_arrSecInfoTable[x][y]->MakeBeachStage2(hBeachFile);

    if(hBeachFile)
      GetSystem()->GetIPak()->FClose(hBeachFile);

    delete m_pBeachGenerator;
    m_pBeachGenerator=0;
  }
  
  GetLog()->LogPlus("ok");// (%.2f MB)",0.001f*0.001f*sizeof(m_arrSecInfoTable));
}

void CTerrain::InitSectors(bool bEditorMode)
{
//  FILE * file_to_write = 0;
//  FILE * file_to_read  = 0;

  // if found - load precompiled error metrics and some additional info for sectors
//  if(!bEditorMode)
////  if(!CXFile::IsOutOfDate(GetLevelFilePath(TERR_ERROR_METR_FILENAME), GetLevelFilePath("terrain\\LAND_MAP.H16")))
//    file_to_read = GetSystem()->GetIPak()->FOpen(GetLevelFilePath(TERR_ERROR_METR_FILENAME), "rb");
/*
  if(!file_to_read)
  {
    file_to_write = GetSystem()->GetIPak()->FOpen(GetLevelFilePath(TERR_ERROR_METR_FILENAME), "wb");
    if(bEditorMode && !file_to_write)
      GetLog()->Log("Error opening %s for writing", TERR_ERROR_METR_FILENAME);
    GetLog()->Log("  Generating %s", TERR_ERROR_METR_FILENAME);
  }
*/
  GetLog()->UpdateLoadingScreen("  Initializing sectors ...");

  m_arrSecInfoTable.Allocate(CTerrain::GetSectorsTableSize());

  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
  {
    m_arrSecInfoTable[x][y] = new CSectorInfo(this);
    m_arrSecInfoTable[x][y]->InitSectorBoundsAndErrorLevels(x*CTerrain::GetSectorSize(),y*CTerrain::GetSectorSize(),NULL,NULL);//file_to_read, file_to_write);
  }
/*
  if(file_to_write)
    GetSystem()->GetIPak()->FClose(file_to_write);
  if(file_to_read)
    GetSystem()->GetIPak()->FClose(file_to_read);
*/
//  m_nUploadsInFrame = -10000;

  //PreCacheArea(vPos, fRadius);

  m_nUploadsInFrame=0;
}

void CTerrain::InitDetailTextureLayers()
{
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
  {
    assert(m_arrSecInfoTable[x][y]);
    if(m_arrSecInfoTable[x][y])
      m_arrSecInfoTable[x][y]->SetDetailLayersPalette();
  }
}

void CTerrain::InitTerrainWater(bool bEditorMode, IShader * pTerrainWaterShader, int nWaterBottomTexId, IShader * pSunRoadShader, float fWaterTranspRatio, float fWaterReflectRatio, float fWaterBumpAmountX, float fWaterBumpAmountY, float fWaterBorderTranspRatio)
{
//  if(!bEditorMode)
    InitBeaches(false);

  // make water
  int nFogId = m_pWater ? m_pWater->GetFogVolumrId() : 0;
  delete m_pWater;
  m_pWater = new CWaterOcean(pTerrainWaterShader, nWaterBottomTexId, pSunRoadShader, fWaterTranspRatio, fWaterReflectRatio, fWaterBumpAmountX, fWaterBumpAmountY, fWaterBorderTranspRatio);
  if(nFogId)
    m_pWater->SetFogVolumrId(nFogId);
}

void CTerrain::MakeUnderWaterSmoothHMap(int nWaterUnitSize)
{
  int nSize  = GetTerrainSize()/nWaterUnitSize;
  int nScale = nWaterUnitSize/GetHeightMapUnitSize();

  m_arrusUnderWaterSmoothHMap.Allocate(nSize);
  
  for(int x=0; x<nSize; x++)
  {
    for(int y=0; y<nSize; y++)
    {
      m_arrusUnderWaterSmoothHMap[x][y] = GetHMValue(x*nScale*GetHeightMapUnitSize(),y*nScale*GetHeightMapUnitSize());
    }
  }

  // blur underwater terrain
  for(int nPass=0; nPass<2; nPass++)
  { 
    Array2d<unsigned short> arrusTemp;
    arrusTemp = m_arrusUnderWaterSmoothHMap;

		int nDim = nSize-2;
    for(int x=2; x<nDim; x++)
    {
      for(int y=2; y<nDim; y++)
      {
        if(GetHMValue(x*nScale*GetHeightMapUnitSize(),y*nScale*GetHeightMapUnitSize())*TERRAIN_Z_RATIO <= GetWaterLevel())
        {
          float fVal = 0;
          fVal += arrusTemp[x  ][y  ];
          fVal += arrusTemp[x+1][y+1];
          fVal += arrusTemp[x-1][y+1];
          fVal += arrusTemp[x+1][y-1];
          fVal += arrusTemp[x-1][y-1];
/*          float fDiv = 1.0f;
          // Don't take into account bound points
          if (x+1 != nSize-1)
            fDiv += 1.0f;
          if (x-1 != 0)
            fDiv += 1.0f;
          if (y+1 != nSize-1)
            fDiv += 1.0f;
          if (y-1 != 0)
            fDiv += 1.0f;*/
          m_arrusUnderWaterSmoothHMap[x][y] = fastftol_positive(fVal*0.2f);
        }
      }
    }
  }
}
