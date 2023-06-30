////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: check vis
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
#include "VisAreas.h"

void CTerrain::PreCacheArea(const Vec3d & vPos, float fRadius)
{
//  if(!m_pObjManager)
    return;

#ifndef _DEBUG
#if !defined(_XBOX) && !defined(LINUX)
  assert(GetConsole()->Exit("Assert should do nothing in release mode")); // just check
#endif // _XBOX
#endif // DEBUG

  m_nUploadsInFrame = -10000;

  GetLog()->UpdateLoadingScreen("Preloading terrain textures for (%.1f, %.1f, %.1f) ...", 
    vPos.x,vPos.y,vPos.z);

  int nLoaded=0;
  for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
  for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
  {
    m_arrSecInfoTable[x][y]->m_fDistance = 
      GetDist2D( 
      float(m_arrSecInfoTable[x][y]->m_nOriginX+(CTerrain::GetSectorSize()>>1)), 
      float(m_arrSecInfoTable[x][y]->m_nOriginY+(CTerrain::GetSectorSize()>>1)), 
      vPos.x, vPos.y ) - (CTerrain::GetSectorSize()>>1);
    
    if(m_arrSecInfoTable[x][y]->m_fDistance < fRadius)
    {
      m_arrSecInfoTable[x][y]->SetLOD();
      if(IsSectorNonMergable(m_arrSecInfoTable[x][y]))
      {
        m_arrSecInfoTable[x][y]->SetTextures();
        nLoaded++;
      }
      m_arrSecInfoTable[x][y]->m_cLastTimeUsed = fastftol_positive(GetCurTimeSec()) + 20;
    }
  }
  
  GetLog()->LogPlus(" %d sectors loaded", nLoaded);

  m_nUploadsInFrame = 0;
}

int CTerrain::GetSecMML(int x, int y)
{ 
  assert(x/CTerrain::GetSectorSize()>=0);
  assert(y/CTerrain::GetSectorSize()>=0);
  assert(x/CTerrain::GetSectorSize()<CTerrain::GetSectorsTableSize());
  assert(y/CTerrain::GetSectorSize()<CTerrain::GetSectorsTableSize());
  
  CSectorInfo * info = m_arrSecInfoTable[x/CTerrain::GetSectorSize()][y/CTerrain::GetSectorSize()];
  return info->m_cGeometryMML; 
}

void CTerrain::AddVisSetcor(CSectorInfo * newsec)
{
  m_lstVisSectors.Add(newsec);
}

void CTerrain::LinkVisSetcors()
{
  for(int i=0; i<m_lstVisSectors.Count(); i++)
  {
    CSectorInfo * info = (CSectorInfo*)m_lstVisSectors[i];

    int sx1 = info->m_nOriginX/CTerrain::GetSectorSize();
    int sy1 = info->m_nOriginY/CTerrain::GetSectorSize();

    if(sx1+1<CTerrain::GetSectorsTableSize())
      m_arrSecInfoTable[sx1+1][sy1]->CheckGeomCompWithLOD(info->m_cGeometryMML);

    if(sy1+1<CTerrain::GetSectorsTableSize())
      m_arrSecInfoTable[sx1][sy1+1]->CheckGeomCompWithLOD(info->m_cGeometryMML);

    if(sx1>0)
      m_arrSecInfoTable[sx1-1][sy1]->CheckGeomCompWithLOD(info->m_cGeometryMML);

    if(sy1>0)
      m_arrSecInfoTable[sx1][sy1-1]->CheckGeomCompWithLOD(info->m_cGeometryMML);
  }
}

void CTerrain::RenderTerrain( CObjManager * pObjManager, const int & DrawFlags )
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

  m_pObjManager = pObjManager;

  // precache tarrain data if camera was teleported more than 32 meters
  if(m_nRenderStackLevel==0)
  {
    if(GetDistance(m_vPrevCameraPos, GetViewCamera().GetPos()) > 32)
      PreCacheArea(GetViewCamera().GetPos(), GetViewCamera().GetZMax()*1.5f);
    m_vPrevCameraPos = GetViewCamera().GetPos();
  }

  if(m_nRenderStackLevel==0)
    m_fDistanceToSectorWithWater=-1;
  m_ucTerrainFrame++;

  m_lstVisSectors.Clear();

  if(!m_nRenderStackLevel)
    m_lstReflectedTerrainIdxArray.Clear();

  m_pObjManager->m_lstFarObjects[m_nRenderStackLevel].Clear();

  m_vCameraPos = m_pViewCamera->GetPos();
  RefineSector(0, CTerrain::GetTerrainSize(), 0, CTerrain::GetTerrainSize(), 0);

  LinkVisSetcors();

	if(m_nRenderStackLevel==0)
	{
		m_bOceanIsVisibe = m_fDistanceToSectorWithWater > -1 || !m_lstVisSectors.Count();

		m_fDistanceToSectorWithWater-=CTerrain::GetSectorSize()*0.5f;
		if(m_fDistanceToSectorWithWater<1.0f)
			m_fDistanceToSectorWithWater=1.0f;

		if(!m_lstVisSectors.Count())
			m_fDistanceToSectorWithWater=0;

		m_fDistanceToSectorWithWater = max(m_fDistanceToSectorWithWater, (m_pViewCamera->GetPos().z-m_fGlobalWaterLevel)*0.05f);
	}

	if(m_pDetailObjects)
		if(DrawFlags & DLD_DETAIL_OBJECTS)
			m_pDetailObjects->RenderDetailGrass(this);
}

void CSectorInfo :: CheckGeomCompWithLOD(int minMML)
{
  if(m_cLastFrameUsed == m_pTerrain->m_ucTerrainFrame)
    if(m_cGeometryMML < 100)
      if(m_cGeometryMML>minMML+1)
  {
    m_cGeometryMML = minMML+1;
    int sx1 = m_nOriginX/CTerrain::GetSectorSize();
    int sy1 = m_nOriginY/CTerrain::GetSectorSize();

    if(sx1+1<CTerrain::GetSectorsTableSize())
      m_pTerrain->m_arrSecInfoTable[sx1+1][sy1]->CheckGeomCompWithLOD(m_cGeometryMML);

    if(sy1+1<CTerrain::GetSectorsTableSize())
      m_pTerrain->m_arrSecInfoTable[sx1][sy1+1]->CheckGeomCompWithLOD(m_cGeometryMML);

    if(sx1>0)
      m_pTerrain->m_arrSecInfoTable[sx1-1][sy1]->CheckGeomCompWithLOD(m_cGeometryMML);

    if(sy1>0)
      m_pTerrain->m_arrSecInfoTable[sx1][sy1-1]->CheckGeomCompWithLOD(m_cGeometryMML);
  }
}

// todo: use real quadtree here
void CTerrain::RefineSector(int x1, int x2, int y1, int y2, bool bAllIN)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

  int xm = (x1+x2)>>1;
  int ym = (y1+y2)>>1;

  if((x2-x1) > CTerrain::GetSectorSize())
  {                                                                    
    Vec3d bmin((float)x1-TERRAIN_SECTORS_MAX_OVERLAPPING, (float)y1-TERRAIN_SECTORS_MAX_OVERLAPPING,-256.f);
    Vec3d bmax((float)x2+TERRAIN_SECTORS_MAX_OVERLAPPING, (float)y2+TERRAIN_SECTORS_MAX_OVERLAPPING, 512.f);
		if( bAllIN || m_pViewCamera->IsAABBVisible_hierarchical(AABB(bmin,bmax),&bAllIN) )
    {
      if(m_vCameraPos.x < xm)
      {
        if(m_vCameraPos.y < ym)
        {
          RefineSector(x1, xm, y1, ym, bAllIN );
          
          RefineSector(xm, x2, y1, ym, bAllIN );
          RefineSector(x1, xm, ym, y2, bAllIN );
          
          RefineSector(xm, x2, ym, y2, bAllIN );
        }
        else
        {
          RefineSector(x1, xm, ym, y2, bAllIN );
          
          RefineSector(xm, x2, ym, y2, bAllIN );
          RefineSector(x1, xm, y1, ym, bAllIN );          
          
          RefineSector(xm, x2, y1, ym, bAllIN );
        }
      }
      else
      {
        if(m_vCameraPos.y < ym)
        {
          RefineSector(xm, x2, y1, ym, bAllIN );
          
          RefineSector(xm, x2, ym, y2, bAllIN );
          RefineSector(x1, xm, y1, ym, bAllIN );          
          
          RefineSector(x1, xm, ym, y2, bAllIN );
        }
        else
        {
          RefineSector(xm, x2, ym, y2, bAllIN );
          
          RefineSector(xm, x2, y1, ym, bAllIN );
          RefineSector(x1, xm, ym, y2, bAllIN );
          
          RefineSector(x1, xm, y1, ym, bAllIN );
        }
      }
    }
  }
  else
  {
    CSectorInfo * info = m_arrSecInfoTable[x1/CTerrain::GetSectorSize()][y1/CTerrain::GetSectorSize()];  

		// test higher bbox to include all flying dynamic stuff and reflected in water terrain
		if(!m_pViewCamera->IsAABBVisible_hierarchical( 
			AABB(
			Vec3d((float)x1-TERRAIN_SECTORS_MAX_OVERLAPPING,(float)y1-TERRAIN_SECTORS_MAX_OVERLAPPING,0),
			Vec3d((float)x2+TERRAIN_SECTORS_MAX_OVERLAPPING,(float)y2+TERRAIN_SECTORS_MAX_OVERLAPPING,512.f)), 
			&info->m_bAllStaticsInFrustum))
			return;

		// debug: render this sector only
		if(GetCVars()->e_terrain_draw_this_sector_only)
			if(GetSecInfo(GetViewCamera().GetPos()) != info)
				return;

//		if(info->m_nOriginX != 1344 || info->m_nOriginY!=1344)
	//		return;

    if(!m_nRenderStackLevel) // for reflections
      info->MergeSectorIntoLowResTerrain(false);

		// draw dynamic objects
		info->RenderEntities(m_pObjManager, !info->m_bAllStaticsInFrustum, "", DYNAMIC_ENTITIES);

		// test exact bbox of sector, it includes all static geometry
		if(!m_pViewCamera->IsAABBVisible_hierarchical( AABB(info->m_vBoxMin,info->m_vBoxMax), &info->m_bAllStaticsInFrustum))
			return;

		// test ground visibility
    if(info->m_bAllStaticsInFrustum)
      info->m_bGroundVisible = true;
    else
    {
      float fOrgX = float(info->m_nOriginX);
      float fOrgY = float(info->m_nOriginY);
      info->m_bGroundVisible = m_pViewCamera->IsAABBVisibleFast( AABB(Vec3d(fOrgX,fOrgY,info->m_fMinZ), Vec3d(fOrgX+CTerrain::GetSectorSize(),fOrgY+CTerrain::GetSectorSize(),max(info->m_fMaxZ,m_fGlobalWaterLevel))));
    }

		// calc distance
    info->m_fDistance = GetDist2D( (float)xm, (float)ym, m_vCameraPos.x, m_vCameraPos.y ) - (CTerrain::GetSectorSize()>>1)*1.3333f;
    if(info->m_fDistance > m_pViewCamera->GetZMax()) 
      return;
    if(info->m_fDistance<1)
      info->m_fDistance=1;
																									/*
    if( info->m_fDistance>128 && info->m_vBoxMax.z <= m_fGlobalWaterLevel && 
        m_vCameraPos.z > m_fGlobalWaterLevel+1 && m_bCullUnderwaterTerrain && m_fGlobalWaterLevel)
      return;																				*/

    // occlusion test (affects only static objects)
		if(GetCVars()->e_terrain_occlusion_culling)
    if(m_pObjManager->IsBoxOccluded(info->m_vBoxMin,info->m_vBoxMax,info->m_fDistance, &info->m_OcclusionTestClient))
      return;

		// test occl by antiportals
		if(GetVisAreaManager() && GetVisAreaManager()->IsOccludedByOcclVolumes(info->m_vBoxMin,info->m_vBoxMax))
			return;

    if(m_nRenderStackLevel==0)
      info->m_cLastFrameUsed = m_ucTerrainFrame;

    if( info->m_bGroundVisible || info->m_fDistance<4 )
      info->SetLOD();

//    else
  //    info->m_cGeometryMML = MAX_MML_LEVEL;

    // update distance to the ocean
    if(m_nRenderStackLevel==0)
    {
      if(info->m_bGroundVisible)
      if(info->m_fMinZ<m_fGlobalWaterLevel)
      if(m_fDistanceToSectorWithWater == -1)
        m_fDistanceToSectorWithWater = info->m_fDistance;
    }

    AddVisSetcor(info);

		// render static objects
    if(info)
			info->RenderEntities(m_pObjManager, !info->m_bAllStaticsInFrustum, "", STATIC_ENTITIES);
  }
}

void CTerrain::UnloadOldSectors(float fMaxViewDist)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

  static int nStaticLoadedTexturesNum[2]={0,0};

	int cCurTimeSec = fastftol_positive(GetCurTimeSec());
	/////////////////////////////////////////////////////////////////////////////////////
// x/y cycle
/////////////////////////////////////////////////////////////////////////////////////
  for(int t=0; t<16; t++)
  {
    m_nOldSectorsY++;
    if(m_nOldSectorsY>=CTerrain::GetSectorsTableSize())
    {
      m_nOldSectorsY=0;
      m_nOldSectorsX++;
      if(m_nOldSectorsX>=CTerrain::GetSectorsTableSize())
      {
        m_nOldSectorsY=m_nOldSectorsX=0;
        m_arrLoadedTexturesNum[0] = nStaticLoadedTexturesNum[0];
        m_arrLoadedTexturesNum[1] = nStaticLoadedTexturesNum[1];
        nStaticLoadedTexturesNum[0] = nStaticLoadedTexturesNum[1]=0;
      }
    }
  
/////////////////////////////////////////////////////////////////////////////////////
  
    {
      CSectorInfo * info = m_arrSecInfoTable[m_nOldSectorsX][m_nOldSectorsY];    

			if(info->m_bLockTexture)
				return; // do not unload locked texture

      if(info->m_nTextureID && info->m_nTextureID != info->m_nLowLodTextureID)
        nStaticLoadedTexturesNum[0]++;

      if(info->m_nLowLodTextureID)
        nStaticLoadedTexturesNum[1]++;

      float fDistanse = GetDist2D( 
        (float)info->m_nOriginX+(CTerrain::GetSectorSize()>>1), 
        (float)info->m_nOriginY+(CTerrain::GetSectorSize()>>1), 
        m_pViewCamera->GetPos().x, m_pViewCamera->GetPos().y ) - (CTerrain::GetSectorSize()>>1);
    
      if((fDistanse > (1.5f*fMaxViewDist)) || (info->m_cLastTimeUsed < (cCurTimeSec - 4)))
      { // try to release vert buffer if not in use int time
        info->ReleaseHeightMapVertBuffer();

        // release static shadow vertex buffers
//        info->ReleaseStaticShadowVertexBuffers();

//        if(GetCVars()->e_terrain_log)
  //        GetLog()->Log("buffer unloaded %d", info->GetSecIndex());
      }      
      
      if(GetCVars()->e_terrain_debug==3 || !GetCVars()->e_terrain_texture_pool)
      {
        if( fDistanse > (1.5f*fMaxViewDist) || (info->m_cLastTimeUsed < (cCurTimeSec - 8)))
        {
					info->UnloadHeighFieldTexture(fDistanse, fMaxViewDist);
        }
      }
    }
  }
}

int CTerrain::RenderEntitiesOutOfTheMap(CObjManager * pObjManager)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

  if(!GetCVars()->e_objects || GetCVars()->e_terrain_draw_this_sector_only == 2)
    return 0;

  // this sector used also for entities outside of the map and for draw near objects
	m_arrSecInfoTable[0][0]->RenderEntities(pObjManager, true, "", STATIC_ENTITIES);
	m_arrSecInfoTable[0][0]->RenderEntities(pObjManager, true, "", DYNAMIC_ENTITIES);

  return 0;
}

int CTerrain::RenderTerrainWater(bool bRenderShore)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

  if(!GetCVars()->e_water_ocean)
    return 0;

  m_pWater->Render(m_nRenderStackLevel);
  
  float fCamZ = GetViewCamera().GetPos().z;
  if(fCamZ>m_fGlobalWaterLevel)
  if(bRenderShore && m_pSHShore && !m_nRenderStackLevel && GetCVars()->e_beach)
  {
    float fZoomFactor = 0.1f+0.9f*(RAD2DEG(GetViewCamera().GetFov())/90.f);      
    for( int i=0; i<m_lstVisSectors.Count(); i++ )
      m_lstVisSectors[i]->RenderBeach(m_pSHShore, fZoomFactor, fCamZ);
  }

  return 0;
}

CSectorInfo * CTerrain::GetSectorFromPoint(int _x, int _y)
{ 
  int x = _x/CTerrain::GetSectorSize();
  int y = _y/CTerrain::GetSectorSize();

  if( x>=0 && y>=0 && x<CTerrain::GetSectorsTableSize() && y<CTerrain::GetSectorsTableSize() )
    return (m_arrSecInfoTable)[x][y];

  return 0;
}

void CTerrain::ResetDLightMaskInSectors()
{
  CSectorInfo ** p     = &m_arrSecInfoTable[0][0];
  CSectorInfo ** pEnd  = &m_arrSecInfoTable[CTerrain::GetSectorsTableSize()-1][CTerrain::GetSectorsTableSize()-1];
  while(p <= pEnd)
  {
		register CSectorInfo *pSector = *p;
    pSector->m_nDynLightMaskNoSun = 0;
    pSector->m_nDynLightMask = 0;
    ++p;
  }
}

void CTerrain::ApplyForceToEnvironment(Vec3d vPos, float fRadius, float fAmountOfForce)
{
  if(fRadius<=0)
    return;

	if(!_finite(vPos.x) || !_finite(vPos.y) || !_finite(vPos.z))
	{
		GetLog()->Log("Error: 3DEngine::ApplyForceToEnvironment: Undefined position passed to the function");
		return;
	}

  if (fRadius > 50.f)
    fRadius = 50.f;

	if(fAmountOfForce>1.f)
		fAmountOfForce = 1.f;

  if( (GetDistance(vPos,m_pViewCamera->GetPos()) > 50.f+fRadius*2.f ) || // too far
		vPos.z < (GetZApr(vPos.x, vPos.y)-1.f)) // under ground
    return;
/*
  int min_x = (int)(((vPos.x-fRadius)/CTerrain::GetSectorSize()))-1;
  int min_y = (int)(((vPos.y-fRadius)/CTerrain::GetSectorSize()))-1;
  int max_x = (int)(((vPos.x+fRadius)/CTerrain::GetSectorSize()))+1;
  int max_y = (int)(((vPos.y+fRadius)/CTerrain::GetSectorSize()))+1;

	if(min_x<0)
		min_x=0;
	if(min_y<0)
		min_y=0;
	if(max_x>=CTerrain::GetSectorsTableSize())
		max_x = CTerrain::GetSectorsTableSize()-1;
	if(max_y>=CTerrain::GetSectorsTableSize())
		max_y = CTerrain::GetSectorsTableSize()-1;

  for(int x=min_x; x<=max_x; x++)
  for(int y=min_y; y<=max_y; y++)
	{
		list2<IEntityRender*> * pList = &m_arrSecInfoTable[x][y]->m_lstEntities[STATIC_ENTITIES];
		for(int s=0; s<pList->Count(); s++)
			if(pList->GetAt(s)->GetEntityRenderType()==eERType_Vegetation)
		{
			CStatObjInst * pEnt = (CStatObjInst *)pList->GetAt(s);
			float fDist = GetDist2D(pEnt->GetPos(true).x,pEnt->GetPos(true).y, vPos.x,vPos.y);
			if(fDist<fRadius)    
			{
				pEnt->m_fCurrentBending = (1.f - fDist/fRadius)*fAmountOfForce*0.25;
				pEnt->m_fCurrentBending=pEnt->m_fCurrentBending;
			}
		}
	}*/

	// calc area
	int x1=int(vPos.x-fRadius-CTerrain::GetHeightMapUnitSize());
	int y1=int(vPos.y-fRadius-CTerrain::GetHeightMapUnitSize());
	int x2=int(vPos.x+fRadius+CTerrain::GetHeightMapUnitSize());
	int y2=int(vPos.y+fRadius+CTerrain::GetHeightMapUnitSize());

	int nUnitSize = GetHeightMapUnitSize();

	x1=x1/nUnitSize*nUnitSize;
	x2=x2/nUnitSize*nUnitSize;
	y1=y1/nUnitSize*nUnitSize;
	y2=y2/nUnitSize*nUnitSize;

	// limits
	if(x1<0) x1=0;
	if(y1<0) y1=0;
	if(x2>=CTerrain::GetTerrainSize()) x2=CTerrain::GetTerrainSize()-1;
	if(y2>=CTerrain::GetTerrainSize()) y2=CTerrain::GetTerrainSize()-1;

	// get near sectors
	list2<CSectorInfo*> lstNearSecInfos;
	for(int x=x1; x<=x2; x++)
	for(int y=y1; y<=y2; y++)
	{
		CSectorInfo * pInfo = GetSecInfo(x,y);
		if(pInfo && lstNearSecInfos.Find(pInfo)<0)
			lstNearSecInfos.Add(pInfo);
	}

	{
		CSectorInfo * pInfo = GetSecInfo(0,0);
		if(pInfo && lstNearSecInfos.Find(pInfo)<0)
			lstNearSecInfos.Add(pInfo);
	}

	// affect objects around
	for( int s=0; s<lstNearSecInfos.Count(); s++)
	{
		CSectorInfo * pSecInfo = lstNearSecInfos[s];
		for(int i=0; i<pSecInfo->m_lstEntities[STATIC_ENTITIES].Count(); i++)
		if(pSecInfo->m_lstEntities[STATIC_ENTITIES][i]->GetEntityRenderType() == eERType_Vegetation)
		{
			CStatObjInst * pStatObjInst =	(CStatObjInst*)pSecInfo->m_lstEntities[STATIC_ENTITIES][i];
			float fDist = GetDist2D(pStatObjInst->GetPos(true).x,pStatObjInst->GetPos(true).y, vPos.x,vPos.y);
			if(fDist<fRadius)    
			{
				float fNewBending = (1.f - fDist/fRadius)*fAmountOfForce;
				if(fNewBending > pStatObjInst->m_fCurrentBending)
					pStatObjInst->m_fCurrentBending = fNewBending;
			}
		}
	}
}

Vec3d CTerrain::GetTerrainSurfaceNormal(Vec3d vPos)
{ 
  Vec3d v1 = Vec3d(vPos.x-0.5f, vPos.y-0.5f,  GetZApr(vPos.x-0.5f,vPos.y-0.5f));
  Vec3d v2 = Vec3d(vPos.x+0.5f, vPos.y,       GetZApr(vPos.x+0.5f,vPos.y     ));
  Vec3d v3 = Vec3d(vPos.x,      vPos.y+0.5f,  GetZApr(vPos.x,     vPos.y+0.5f));
  return (v2-v1).Cross(v3-v1); 
}

bool CTerrain::UnRegisterInAllSectors(IEntityRender * pEntityRS)
{
  assert(pEntityRS);

  bool bRes = 0;

  if(pEntityRS)
  {
    int nStatic = (pEntityRS->GetEntityRenderType() != eERType_Unknown);

    for( int x=0; x<CTerrain::GetSectorsTableSize(); x++ )
    for( int y=0; y<CTerrain::GetSectorsTableSize(); y++ )
    if(m_arrSecInfoTable[x][y])
      bRes |= m_arrSecInfoTable[x][y]->m_lstEntities[nStatic].Delete(pEntityRS);

    pEntityRS->m_pSector = 0;
  }

  return bRes;
}

ushort * CTerrain::GetUnderWaterSmoothHMap(int & nDimensions) 
{ 
  nDimensions = m_arrusUnderWaterSmoothHMap.m_nSize; 
  return m_arrusUnderWaterSmoothHMap.m_pData; 
}

void CTerrain::GetMemoryUsage( class ICrySizer*pSizer)
{
	{
		SIZER_COMPONENT_NAME(pSizer, "HMap");
		GetHighMapMemoryUsage(pSizer);
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "SecInfoTable");
		m_arrSecInfoTable.GetMemoryUsage(pSizer);
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "SectorsData");
		for( int x=0; x<CTerrain::GetSectorsTableSize(); x++)
		for( int y=0; y<CTerrain::GetSectorsTableSize(); y++)
			m_arrSecInfoTable[x][y]->GetMemoryUsage(pSizer);
	}

	int nSize = 0;
	m_arrusUnderWaterSmoothHMap.GetMemoryUsage(pSizer);
	nSize += m_lstFogVolumes.GetMemoryUsage();
	nSize += m_lstLowResTerrainIdxArray.GetMemoryUsage();
	nSize += m_lstReflectedTerrainIdxArray.GetMemoryUsage();
	nSize += m_lstSectorVertArray.GetMemoryUsage();
	nSize += m_lstVisSectors.GetMemoryUsage();
//	nSize += m_pBugsManager->GetMemoryUsage();
	if(m_pCoverageBuffer)
		nSize += sizeof(*m_pCoverageBuffer);

	if(m_pDetailObjects)
		nSize += sizeof(*m_pDetailObjects) + m_pDetailObjects->GetMemoryUsage();

	if(m_pTexturePool)
		m_pTexturePool->GetMemoryUsage(pSizer);

	if(m_pWater)
		nSize += sizeof(*m_pWater) + m_pWater->GetMemoryUsage();

	nSize += m_nSectorTextureDataSizeBytes;

	pSizer->AddObject(this, nSize + sizeof(*this));
}

void CTexturePool::TexInfo::GetSize (ICrySizer*pSizer)const
{
	pSizer->Add (*this);
	pSectorInfo->GetMemoryUsage (pSizer);
}

void CTexturePool::GetMemoryUsage(class ICrySizer*pSizer)
{
	for (int i = 0; i < sizeof(m_TexturePool)/sizeof(m_TexturePool[0]); ++i)
	{
		list2<TexInfo>& arrTexInfo = m_TexturePool[i];
		for (unsigned j = 0; j < arrTexInfo.size(); ++j)
			arrTexInfo[j].GetSize (pSizer);
	}
}

void CTerrain::UnregisterFogVolumeFromOutdoor(VolumeInfo * pFogVolume)
{
	// reset fog volume pointer in all affected terrain sectors
	for(int x=0; x<CTerrain::GetSectorsTableSize(); x++)
	for(int y=0; y<CTerrain::GetSectorsTableSize(); y++)
	{
		CSectorInfo * pSecInfo = m_arrSecInfoTable[x][y];
		if(pSecInfo && pSecInfo->m_pFogVolume == pFogVolume)
    {
      pSecInfo->m_pFogVolume = 0;    
      SetSectorFogVolume(pSecInfo);
    }
	}
}


void CTerrain::MoveAllEntitiesIntoList(list2<IEntityRender*> * plstVisAreasEntities, 
                                       const Vec3d & vBoxMin, const Vec3d & vBoxMax)
{
  // gat 2d bounds
  int min_x = (int)(((vBoxMin.x - 1.f)/CTerrain::GetSectorSize()));
  int min_y = (int)(((vBoxMin.y - 1.f)/CTerrain::GetSectorSize()));
  int max_x = (int)(((vBoxMax.x + 1.f)/CTerrain::GetSectorSize()));
  int max_y = (int)(((vBoxMax.y + 1.f)/CTerrain::GetSectorSize()));

  if( min_x<0 ) min_x = 0; else if( min_x>=CTerrain::GetSectorsTableSize() ) min_x = CTerrain::GetSectorsTableSize()-1;
  if( min_y<0 ) min_y = 0; else if( min_y>=CTerrain::GetSectorsTableSize() ) min_y = CTerrain::GetSectorsTableSize()-1;
  if( max_x<0 ) max_x = 0; else if( max_x>=CTerrain::GetSectorsTableSize() ) max_x = CTerrain::GetSectorsTableSize()-1;
  if( max_y<0 ) max_y = 0; else if( max_y>=CTerrain::GetSectorsTableSize() ) max_y = CTerrain::GetSectorsTableSize()-1;

  // make list of sectors
	list2<CSectorInfo*> TerrainSectorsList;
  for(int x=min_x; x<=max_x; x++)
  for(int y=min_y; y<=max_y; y++)
    TerrainSectorsList.Add(m_arrSecInfoTable[x][y]);

  // this sector contains fps weapons and entities from outside of the map
  TerrainSectorsList.Add(m_arrSecInfoTable[0][0]);

  // add lists of entities where not added
  for(int i=0; i<TerrainSectorsList.Count(); i++)
	{
    // note: lstEntitiesInArea will have duplicates but it's not a problem
    plstVisAreasEntities->AddList(TerrainSectorsList.GetAt(i)->m_lstEntities[DYNAMIC_ENTITIES]);
    plstVisAreasEntities->AddList(TerrainSectorsList.GetAt(i)->m_lstEntities[STATIC_ENTITIES]);
    TerrainSectorsList.GetAt(i)->m_lstEntities[STATIC_ENTITIES].Clear();
    TerrainSectorsList.GetAt(i)->m_lstEntities[DYNAMIC_ENTITIES].Clear();
	}

  // remove duplicates
  for(int i=0; i<plstVisAreasEntities->Count(); i++)
  {
    IEntityRender * pEnt = plstVisAreasEntities->GetAt(0);
    plstVisAreasEntities->Delete(pEnt);
    plstVisAreasEntities->Add(pEnt);
  }
}

bool CTerrain::PreloadResources()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	static int nCurTexPreloadX=0, nCurTexPreloadY=0;

	nCurTexPreloadX++;
	if(nCurTexPreloadX>=CTerrain::GetSectorsTableSize())
	{
		nCurTexPreloadX=0;
		nCurTexPreloadY++;
	}

	if(nCurTexPreloadY>=CTerrain::GetSectorsTableSize())
	{
		nCurTexPreloadY = 0;

		DrawDetailTextures(32, 64, false);
		if(m_pDetailObjects)
			m_pDetailObjects->PreloadResources();
	}

	assert(nCurTexPreloadX>=0 && nCurTexPreloadX<CTerrain::GetSectorsTableSize());
	assert(nCurTexPreloadY>=0 && nCurTexPreloadY<CTerrain::GetSectorsTableSize());

	CSectorInfo * pSecInfo = m_arrSecInfoTable[nCurTexPreloadX][nCurTexPreloadY];
	if(pSecInfo)
	{
    pSecInfo->m_fDistance = 
      GetDist2D(  float(pSecInfo->m_nOriginX+(CTerrain::GetSectorSize()>>1)), 
                  float(pSecInfo->m_nOriginY+(CTerrain::GetSectorSize()>>1)), 
                  m_vCameraPos.x, m_vCameraPos.y ) - (CTerrain::GetSectorSize()>>1);

		pSecInfo->SetLOD();
		if(pSecInfo->m_fDistance<GetViewCamera().GetZMax()*2)
		{
			pSecInfo->SetTextures();
      //CCObject * pTerrainCCObject = GetRenderer()->EF_GetObject(true);
      //pSecInfo->RenderSector(pTerrainCCObject);
			pSecInfo->PreloadResources(GetViewCamera().GetPos(), 0);
			pSecInfo->m_cLastTimeUsed = fastftol_positive( GetCurTimeSec() );
		}
	}

	return !nCurTexPreloadX && !nCurTexPreloadY;
}

void CTerrain::GetObjects(list2<struct IEntityRender*> * pLstObjects)
{
	// reset fog volume pointer in all affected terrain sectors
	for(int x=0; x<CTerrain::GetSectorsTableSize(); x++)
	for(int y=0; y<CTerrain::GetSectorsTableSize(); y++)
	{
		CBasicArea * pSecInfo = m_arrSecInfoTable[x][y];	
		for(int nStatic=0; nStatic<2; nStatic++)
		{
			if(pLstObjects)
				pLstObjects->AddList(pSecInfo->m_lstEntities[nStatic]);
			pSecInfo->m_lstEntities[nStatic].Reset();				
		}
	}
}

const void * CTerrain::GetShoreGeometry(int & nPosStride, int & nVertCount, int nSectorX, int nSectorY)
{
	CSectorInfo * pSecInfo = GetSecInfo(nSectorX, nSectorY);	
	if(!pSecInfo)
		return 0;

	return pSecInfo->GetShoreGeometry(nPosStride, nVertCount);
}