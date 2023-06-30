////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_sector.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: sector initialiazilation, objects rendering
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "terrain_sector.h"
#include "terrain.h"
#include "objman.h"

CSectorInfo::CSectorInfo(CTerrain * pTerrain) 
{ 
  ZeroStruct(*this);
  m_cGeometryMML = 100;
  m_cLastTimeRendered = m_cLastTimeUsed = (int)GetCurTimeSec() + 20;
  m_bBeachPresent=false;
  m_pTerrain = pTerrain;
  m_cNewTextMML = 100;
	m_nLowResTerrainIdxRange[0][0] = m_nLowResTerrainIdxRange[1][0] = (uint)-1;
}

CSectorInfo::~CSectorInfo() 
{
  ReleaseHeightMapVertBuffer();

  if(m_nLowLodTextureID)
    m_pTerrain->m_pTexturePool->RemoveTexture(m_nLowLodTextureID);
  if(m_nTextureID != m_nLowLodTextureID)
    m_pTerrain->m_pTexturePool->RemoveTexture(m_nTextureID);

  m_nTextureID=0;
  m_nLowLodTextureID=0;
  m_bHasHoles=0;

  Unload();

	UnregisterDynamicEntities();

  GetRenderer()->DeleteLeafBuffer(m_pLeafBufferBeach);
  assert(m_pLeafBuffer==0);
}

int CSectorInfo::GetMML(int nDist, int mmMin, int mmMax)
{
  const int nStep = 48;

  for(int i=mmMin; i<mmMax; i++)
    if(nStep<<i > nDist)
      return i;

  return mmMax;
}

void CSectorInfo::SetLOD()
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate geometry LOD

  {
    const Vec3d & vCamPos = GetViewCamera().GetPos();  
    float camera_h = 0.25f*(float)fabs(vCamPos.z - m_fMidZ);//  vBoxMm_pTerrain->GetZSafe(vCamPos.x,vCamPos.y));

    if((m_fDistance+camera_h)<CTerrain::GetSectorSize()+(CTerrain::GetSectorSize()>>2))
      m_cGeometryMML = ZERO_MML_LEVEL;
    else
    {
      float allowed_error = 
        ( m_pTerrain->m_fLodLFactor * 
          GetCVars()->e_terrain_lod_ratio * 
        ( m_fDistance+camera_h))/(180.f)*2.5f;

      int l;
      for( l=MAX_MML_LEVEL; l>ZERO_MML_LEVEL; l-- )
        if(/*GetCVars()->e_lod_ratio**/m_arrGeomErrors[l]<allowed_error)
          break;

      m_cGeometryMML = l;
    }
  }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate Texture LOD
                    // not ready first time
  float fTexSizeK = m_pTerrain->m_nSectorTextureReadedSize ? m_pTerrain->m_nSectorTextureReadedSize/128.f : 1;
  m_cNewTextMML = GetMML(int(fTexSizeK*0.3f*m_fDistance*m_pTerrain->m_fTextureLodRatio), ZERO_MML_LEVEL, MAX_TEX_MML_LEVEL);
}

/*
BOOL CSectorInfo::IsCanBeReflected()
{
  assert(m_pTerrain->m_pViewCamera);

  if(m_pTerrain->m_pViewCamera->GetPos().x<0 || m_pTerrain->m_pViewCamera->GetPos().y<0 || m_pTerrain->m_pViewCamera->GetPos().x>=CTerrain::GetTerrainSize() || m_pTerrain->m_pViewCamera->GetPos().y>=CTerrain::GetTerrainSize())
    return 1;

  float dx = (m_nOriginX+CTerrain::GetSectorSize()_2) - m_pTerrain->m_pViewCamera->GetPos().x;
  float dy = (m_nOriginY+CTerrain::GetSectorSize()_2) - m_pTerrain->m_pViewCamera->GetPos().y;

  float tests_num = m_fDistance/16;

  dx /= tests_num;
  dy /= tests_num;

  float x = m_pTerrain->m_pViewCamera->GetPos().x;
  float y = m_pTerrain->m_pViewCamera->GetPos().y;

  for(int i=0; i<tests_num; i++)
  {
    CSectorInfo * sector = GetSectorFromPoint(fastftol(x), fastftol(y));
    if(sector->minZ<CTerrain::GetWaterLevel())
      return 1;

    x += dx;
    y += dy;
  }
  
  return 0;
}*/

void CSectorInfo::SetMinMaxMidZ()
{
  m_bHasHoles = false;
  // calculate min, max, mid, hole
  for( int x=0; x<=CTerrain::GetSectorSize(); x+=CTerrain::GetHeightMapUnitSize())
  {
    for( int y=0; y<=CTerrain::GetSectorSize(); y+=CTerrain::GetHeightMapUnitSize())
    {
      if(m_fMaxZ < m_pTerrain->GetZ(m_nOriginX+x,m_nOriginY+y))
        m_fMaxZ = m_pTerrain->GetZ(m_nOriginX+x,m_nOriginY+y);
      else if(m_fMinZ > m_pTerrain->GetZ(m_nOriginX+x,m_nOriginY+y))
        m_fMinZ = m_pTerrain->GetZ(m_nOriginX+x,m_nOriginY+y);

      if(m_pTerrain->GetHole(m_nOriginX+x,m_nOriginY+y))
        m_bHasHoles = true;
    }
  }

  m_fMidZ = (m_fMinZ+m_fMaxZ)*0.5f;
}

void CSectorInfo::InitSectorBoundsAndErrorLevels(int _x1, int _y1, FILE * geom_file_to_read, FILE * geom_file_to_write)
{
  assert(m_nLowLodTextureID==0);
  assert(m_nTextureID==0);
  assert(m_bHasHoles==0);
  assert(m_lstEntities[STATIC_ENTITIES].Count()==0);
  assert(m_lstEntities[DYNAMIC_ENTITIES].Count()==0);

  m_nOriginX = _x1;
  m_nOriginY = _y1;

//////////////////////////////////////////////////////////////////////////////
// init error levels
//////////////////////////////////////////////////////////////////////////////
  memset(m_arrGeomErrors,0,sizeof(m_arrGeomErrors));

  if(geom_file_to_read) // load from file if file found
  {
    GetSystem()->GetIPak()->FRead(m_arrGeomErrors, 1, sizeof(m_arrGeomErrors), geom_file_to_read);
    GetSystem()->GetIPak()->FRead(&m_fMinZ, 1, sizeof(float), geom_file_to_read);
    GetSystem()->GetIPak()->FRead(&m_fMaxZ, 1, sizeof(float), geom_file_to_read);
    GetSystem()->GetIPak()->FRead(&m_fMidZ, 1, sizeof(float), geom_file_to_read);

    m_fMinZ=m_fMaxZ=m_pTerrain->GetZ(m_nOriginX,m_nOriginY);
    SetMinMaxMidZ();
  }
  else // generate errors table, set min,max, mid Z values
  { 
    m_fMinZ=m_fMaxZ=m_pTerrain->GetZ(m_nOriginX,m_nOriginY);
    SetMinMaxMidZ();
    
    for(int nLod=0; nLod<=MAX_MML_LEVEL; nLod++)
    { // calculate max diffeence between this detail level and nolod
      float max_diff = 0;

      int nCellSize = (1<<nLod)*CTerrain::GetHeightMapUnitSize();

      for(int X=m_nOriginX; X<m_nOriginX+CTerrain::GetSectorSize(); X+=nCellSize)
      for(int Y=m_nOriginY; Y<m_nOriginY+CTerrain::GetSectorSize(); Y+=nCellSize)
      {
        for( int x=0; x<=nCellSize; x+=CTerrain::GetHeightMapUnitSize())
        {
          float kx = (float)x/(float)nCellSize;
        
          float z1 = (1.f-kx)*m_pTerrain->GetZ(X+0,Y+        0) + (kx)*m_pTerrain->GetZ(X+nCellSize,Y+        0);
          float z2 = (1.f-kx)*m_pTerrain->GetZ(X+0,Y+nCellSize) + (kx)*m_pTerrain->GetZ(X+nCellSize,Y+nCellSize);

          for( int y=0; y<=nCellSize; y+=CTerrain::GetHeightMapUnitSize())
          {
            // skip borders
            if((X+x) < 16 || (Y+y)<16)
              continue;
            if((X+x) > (CTerrain::GetTerrainSize()-16) || (Y+y) > (CTerrain::GetTerrainSize()-16))
              continue;

            float ky = (float)y/nCellSize;

            float avrz = (1.f-ky)*z1 + ky*z2;

            float var = (m_pTerrain->GetZ(X+x,Y+y)-avrz);            

            if((float)fabs(var) > max_diff)
              max_diff = (float)fabs(var);
          }
        }
      }
      m_arrGeomErrors[nLod] = max_diff;
    }

    if(geom_file_to_write) // write calculated data into file
    {
      GetSystem()->GetIPak()->FWrite(m_arrGeomErrors, 1, sizeof(m_arrGeomErrors), geom_file_to_write);
      GetSystem()->GetIPak()->FWrite(&m_fMinZ,      1, sizeof(float),      geom_file_to_write);
      GetSystem()->GetIPak()->FWrite(&m_fMaxZ,      1, sizeof(float),      geom_file_to_write);
      GetSystem()->GetIPak()->FWrite(&m_fMidZ,      1, sizeof(float),      geom_file_to_write);
    }
  }

  // note: result m_arrGeomErrors table can be non incremental

  // set distance to very far to force loading only low res texture first time
  m_fDistance = 2.f*CTerrain::GetTerrainSize();
  if(m_fDistance<0)
    m_fDistance=0;

  m_cLastTimeUsed = (uint)-1;//GetCurTimeSec() + 100;

  // init sector bbox
  m_vBoxMin = Vec3d((float)m_nOriginX,(float)m_nOriginY,m_fMinZ);
  m_vBoxMax = Vec3d((float)m_nOriginX+CTerrain::GetSectorSize(),
                    (float)m_nOriginY+CTerrain::GetSectorSize(),m_fMaxZ);

  if(m_vBoxMax.z < m_pTerrain->GetWaterLevel())
    m_vBoxMax.z = m_pTerrain->GetWaterLevel();
}

struct TerrainSurfaceLayerAmount
{
  TerrainSurfaceLayerAmount()
  {
    ucSurfaceType = 255;
    nUnitsNum = 0;
  }

  uchar ucSurfaceType;
  uint nUnitsNum;
};

int __cdecl CSectorInfo__Cmp_UnitsNum(const void* v1, const void* v2)
{
  TerrainSurfaceLayerAmount * p1 = ((TerrainSurfaceLayerAmount*)v1);
  TerrainSurfaceLayerAmount * p2 = ((TerrainSurfaceLayerAmount*)v2);

  if(!p1 || !p2)
    return 0;

  if(p1->nUnitsNum > p2->nUnitsNum)
    return -1;
  else if(p1->nUnitsNum < p2->nUnitsNum)
    return 1;

  return 0;
}

void CSectorInfo::SetDetailLayersPalette()
{
  TerrainSurfaceLayerAmount lstSurfaceTypesInSector[MAX_SURFACE_TYPES_COUNT];
  for(int i=0; i<MAX_SURFACE_TYPES_COUNT; i++)
    lstSurfaceTypesInSector[i].ucSurfaceType = i;

  for(int X=m_nOriginX; X<=m_nOriginX+CTerrain::GetSectorSize(); X+=CTerrain::GetHeightMapUnitSize())
  for(int Y=m_nOriginY; Y<=m_nOriginY+CTerrain::GetSectorSize(); Y+=CTerrain::GetHeightMapUnitSize())
  {
    uchar ucSurfaceTypeID = m_pTerrain->GetSurfaceTypeID(X,Y);
    if(ucSurfaceTypeID>=0 && ucSurfaceTypeID<MAX_SURFACE_TYPES_COUNT)
    {
      if(m_pTerrain->m_DetailTexInfo[ucSurfaceTypeID].ucProjAxis != 'Z' && lstSurfaceTypesInSector[ucSurfaceTypeID].nUnitsNum<100000)
        lstSurfaceTypesInSector[ucSurfaceTypeID].nUnitsNum += 100000;       
      lstSurfaceTypesInSector[ucSurfaceTypeID].nUnitsNum ++;
    }
  }

  qsort(&lstSurfaceTypesInSector[0], MAX_SURFACE_TYPES_COUNT, sizeof(lstSurfaceTypesInSector[0]), 
    CSectorInfo__Cmp_UnitsNum);

  memset(m_arrDetailTexInfo,0,sizeof(m_arrDetailTexInfo));
  for(int i=0, n=0; n<3 && i<MAX_DETAIL_LAYERS_IN_SECTOR && lstSurfaceTypesInSector[i].nUnitsNum; i++)
  {
    uchar ucSurfaceTypeID = lstSurfaceTypesInSector[i].ucSurfaceType;
    assert(ucSurfaceTypeID>=0 && ucSurfaceTypeID<MAX_SURFACE_TYPES_COUNT);
		if(m_pTerrain->m_DetailTexInfo[ucSurfaceTypeID].ucProjAxis != 'Z')
			m_arrDetailTexInfo[n++] = &m_pTerrain->m_DetailTexInfo[ucSurfaceTypeID];
  }

	for(int i=0, n=0; n<4 && i<MAX_DETAIL_LAYERS_IN_SECTOR && lstSurfaceTypesInSector[i].nUnitsNum; i++)
	{
		uchar ucSurfaceTypeID = lstSurfaceTypesInSector[i].ucSurfaceType;
		assert(ucSurfaceTypeID>=0 && ucSurfaceTypeID<MAX_SURFACE_TYPES_COUNT);
		if(m_pTerrain->m_DetailTexInfo[ucSurfaceTypeID].ucProjAxis == 'Z')
			m_arrDetailTexInfo[3+n++] = &m_pTerrain->m_DetailTexInfo[ucSurfaceTypeID];
	}
}

void CSectorInfo::RenderEntities(CObjManager * pObjManager, bool bNotAllInFrustum, char*fake, int nStatics)
{    
	if(m_lstEntities[nStatics].Count())
		DrawEntities( m_pFogVolume ? m_pFogVolume->nRendererVolumeID : 0,
			GetRenderer()->EF_GetHeatVision() ? m_nDynLightMaskNoSun : m_nDynLightMask, 
			false, GetViewCamera(), NULL, NULL, m_pFogVolume, bNotAllInFrustum, 
			(m_nOriginX == 0 && m_nOriginY == 0) ? 0 : (m_fDistance-TERRAIN_SECTORS_MAX_OVERLAPPING), 
			pObjManager, m_fDistance<GetCVars()->e_area_merging_distance, "", nStatics); 
}

void CSectorInfo::GetMemoryUsage(ICrySizer*pSizer)
{
	m_ArrayInfo.GetMemoryUsage(pSizer);
	m_ArrayInfo_MSALB.GetMemoryUsage(pSizer);
  pSizer->AddContainer(m_lstLowResTerrainIdxArray[0]);
  pSizer->AddContainer(m_lstLowResTerrainIdxArray[1]);

  pSizer->AddContainer(m_lstEntities[STATIC_ENTITIES]);
  pSizer->AddContainer(m_lstEntities[DYNAMIC_ENTITIES]);

  int nSize=0;
  for(int nStatic=0; nStatic<2; nStatic++)
  for(int i=0; i<m_lstEntities[nStatic].Count(); i++)
    nSize += m_lstEntities[nStatic][i]->GetMemoryUsage();

  pSizer->AddObject(this,sizeof(*this)+nSize);
}
