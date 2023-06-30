////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_render.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: draw vis sectors
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "terrain_sector.h"
#include "objman.h"
#include "visareas.h"
#include "cbuffer.h"

bool CTerrain::IsSectorNonMergable(CSectorInfo * info)
{
  if( info->m_cGeometryMML<MAX_MML_LEVEL || 
      info->m_fDistance<MIN_ALLOWED_MERGED_SECTORS_DISTANCE || 
      !(GetCVars()->e_terrain_merge_far_sectors && CTerrain::GetHeightMapUnitSize()>=2)||
      info->m_pFogVolume ||
      (m_nRenderStackLevel && info->m_nLastMergedFrameID != GetFrameID()))
      return true;
  
  return false;
}

void CTerrain::DrawVisibleSectors()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

  if(!GetCVars()->e_terrain)
    return;

  if(!(GetFrameID() % TEXTURE_UPDATE_PERIOD_IN_FRAMES)) 
    m_nUploadsInFrame = 0; // allow to update texture only every x frames

  const float dCSS = 1.f/CTerrain::GetSectorSize();
//  const float fDistFadeK = 1.f/(2.f*fMaxViewDist);
  const Vec3d & vCamPos = m_pViewCamera->GetPos();

//  m_lstReflectedTerrainIdxArray.Clear();
  m_lstLowResTerrainIdxArray.Clear();

	CCObject * pTerrainCCObject = GetRenderer()->EF_GetObject(true);
	if(GetRenderer()->EF_GetHeatVision())
		pTerrainCCObject->m_ObjFlags |= FOB_HEATVISION;
  pTerrainCCObject->m_Matrix.SetIdentity();

	if(!m_pObjManager->m_nRenderStackLevel)
	{
		pTerrainCCObject->m_nScissorX1 = GetViewCamera().m_ScissorInfo.x1;
		pTerrainCCObject->m_nScissorY1 = GetViewCamera().m_ScissorInfo.y1;
		pTerrainCCObject->m_nScissorX2 = GetViewCamera().m_ScissorInfo.x2;
		pTerrainCCObject->m_nScissorY2 = GetViewCamera().m_ScissorInfo.y2;
	}

  const CVars * pCVars = GetCVars();

  bool bOutdoorsVisible = GetVisAreaManager()->IsOutdoorAreasVisible();

	int cCurrTime = fastftol_positive(GetCurTimeSec());

	float fZoomFactor = 0.1f+0.9f*(RAD2DEG(GetViewCamera().GetFov())/90.f);      

  for(int i=0; i<m_lstVisSectors.Count(); i++)
  {
    CSectorInfo * info = m_lstVisSectors[i];

		if(info->m_fDistance<8) // make sure near sectors are always potentialy visible
			info->m_cLastTimeUsed = cCurrTime; 

    if(!info->m_bGroundVisible)
      continue;

    info->m_cLastTimeRendered = info->m_cLastTimeUsed = cCurrTime;

		if(!bOutdoorsVisible)
      continue;

    // set texgen offset
		if(info->m_fDistance*fZoomFactor > MIN_ALLOWED_MERGED_SECTORS_DISTANCE)
		{
			info->m_arrTexOffsets[0] = 0;
			info->m_arrTexOffsets[1] = 0;
			info->m_arrTexOffsets[2] = 1.f/CTerrain::GetTerrainSize();
		}
		else
		{
			info->m_arrTexOffsets[0] = -dCSS*info->m_nOriginY;
			info->m_arrTexOffsets[1] = -dCSS*info->m_nOriginX;
			info->m_arrTexOffsets[2] = 1.f/CTerrain::GetSectorSize();
		}

    // set shader for zoom mode
/*    if(!pCVars->e_terrain_single_pass &&
      m_pViewCamera->GetFov() < DETAIL_TEXTURE_MIN_FOV && 
      pCVars->e_detail_texture) // zoom
      info->m_pCurShader = m_pTerrainEf_WithDefaultDetailTexture;
    else*/
		if(GetCVars()->e_detail_texture_quality)
		{
			if(!info->m_nDynLightMaskNoSun && info->m_pFogVolume && 
				GetCVars()->e_terrain_single_pass_vol_fog && info->m_fDistance>32)
				info->m_pCurShader = m_pTerrainWithFog;
			else
				info->m_pCurShader = m_pTerrainEf;
		}
		else
			info->m_pCurShader = m_pTerrainEf_WithDefaultDetailTexture;

    // check when we can't use lowest lod (and sectors merging)
    if(IsSectorNonMergable(info))
    {
			info->SetTextures();

/*      if( m_pTerrainCausticsEf && 
          info->m_fMinZ < m_fGlobalWaterLevel && 
          info->m_pCurShader == m_pTerrainEf &&
          info->m_fDistance < 80.f &&
					GetCVars()->e_terrain_caustics &&
          !GetCVars()->e_terrain_single_pass)
      { // do caustics
        info->m_pCurShader = m_pTerrainCausticsEf;
        info->RenderSector(pTerrainCCObject);
      }
      else*/
        info->RenderSector(pTerrainCCObject);
    }
    else if(!m_nRenderStackLevel) // do not calc low res indices in recursion //if((GetFrameID()%64)==0)
      info->MergeSectorIntoLowResTerrain(true);

//    if(!m_nRenderStackLevel) 
  //    info->MergeSectorIntoLowResTerrain(false);

    if(pCVars->e_terrain_debug==1)
    {
      GetRenderer()->ResetToDefault();

      GetRenderer()->SetMaterialColor(1,0,0,1);
      GetRenderer()->Draw3dBBox(info->m_vBoxMin, info->m_vBoxMax);

			GetRenderer()->SetMaterialColor(0,1,0,1);
			GetRenderer()->Draw3dBBox(
				Vec3d((float)info->m_nOriginX-TERRAIN_SECTORS_MAX_OVERLAPPING,
							(float)info->m_nOriginY-TERRAIN_SECTORS_MAX_OVERLAPPING,0),
				Vec3d((float)info->m_nOriginX+CTerrain::GetSectorSize()+TERRAIN_SECTORS_MAX_OVERLAPPING,
							(float)info->m_nOriginY+CTerrain::GetSectorSize()+TERRAIN_SECTORS_MAX_OVERLAPPING,512.f));

			GetRenderer()->ResetToDefault();
    }
  }

//	if(!m_nRenderStackLevel)
	RenderLowResTerrain();
}

void CTerrain::MergeLowResTerrainSectorIndices(list2<unsigned short> * pIndices)
{
  m_lstLowResTerrainIdxArray.AddList(*pIndices);
}

void CTerrain::MergeReflectedTerrainSectorIndices(list2<unsigned short> * pIndices)
{
  m_lstReflectedTerrainIdxArray.AddList(*pIndices);
}

void CTerrain::RenderLowResTerrain()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	if(CTerrain::GetHeightMapUnitSize()<2)
		return;

  if(!m_pLowResTerrainLeafBuffer)
  {
    int nStep = (1<<MAX_MML_LEVEL)*CTerrain::GetHeightMapUnitSize();

    // fill vert buffer
    list2<struct_VERTEX_FORMAT_P3F> lstLowResTerrainVertArray;
    for( int x=0; x<=CTerrain::GetTerrainSize(); x+=nStep )
    for( int y=0; y<=CTerrain::GetTerrainSize(); y+=nStep )
    {
      struct_VERTEX_FORMAT_P3F vert;
      vert.xyz.x = (float)(x);
      vert.xyz.y = (float)(y);
      vert.xyz.z = (GetZ(x,y));    
      lstLowResTerrainVertArray.Add(vert);
    }

    // make leaf buffer
    m_pLowResTerrainLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
      lstLowResTerrainVertArray.GetElements(), lstLowResTerrainVertArray.Count(), VERTEX_FORMAT_P3F,
      m_lstLowResTerrainIdxArray.GetElements(), m_lstLowResTerrainIdxArray.Count(),
      R_PRIMV_TRIANGLES,"LowResTerrain", eBT_Dynamic, 1,m_pLowLodCoverMapTex->GetTextureID());

//    m_pLowResTerrainShader = GetRenderer()->EF_LoadShader("TerrainLowLOD", eSH_World, EF_SYSTEM);

    assert(m_pLowResTerrainLeafBuffer);

    m_nLowResTerrainVertCount = lstLowResTerrainVertArray.Count();
  }

  if(!m_nRenderStackLevel) 
  {
    m_pLowResTerrainLeafBuffer->UpdateSysIndices(m_lstLowResTerrainIdxArray.GetElements(), m_lstLowResTerrainIdxArray.Count());
    m_pLowResTerrainLeafBuffer->SetChunk(m_pLowResTerrainShader,0,m_nLowResTerrainVertCount,
      0, m_lstLowResTerrainIdxArray.Count());

    m_lstLowResTerrainIdxArray.Clear();
  }
                     
  m_arrLowResTerrainShaderCustomData[0] = 1;
  m_arrLowResTerrainShaderCustomData[1] = 1;
  m_arrLowResTerrainShaderCustomData[2] = 1.f/CTerrain::GetTerrainSize();    
  m_arrLowResTerrainShaderCustomData[3] = 0;
  m_arrLowResTerrainShaderCustomData[4] = 0;
  m_pLowResTerrainLeafBuffer->SetRECustomData(m_arrLowResTerrainShaderCustomData,1);                       

  m_pLowResTerrainLeafBuffer->m_vBoxMin = Vec3d(0,0,0);
  m_pLowResTerrainLeafBuffer->m_vBoxMax = Vec3d((float)CTerrain::GetTerrainSize(),(float)CTerrain::GetTerrainSize(),255);

	CCObject * pObj = GetRenderer()->EF_GetObject(true);	
	pObj->m_Matrix.SetIdentity();
	if(GetRenderer()->EF_GetHeatVision())
		pObj->m_ObjFlags |= FOB_HEATVISION;

	if(!m_pObjManager->m_nRenderStackLevel)
	{
		pObj->m_nScissorX1 = GetViewCamera().m_ScissorInfo.x1;
		pObj->m_nScissorY1 = GetViewCamera().m_ScissorInfo.y1;
		pObj->m_nScissorX2 = GetViewCamera().m_ScissorInfo.x2;
		pObj->m_nScissorY2 = GetViewCamera().m_ScissorInfo.y2;
	}

	m_pLowResTerrainLeafBuffer->AddRenderElements( pObj );
}

void CTerrain::RenderReflectedTerrain()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	if(CTerrain::GetHeightMapUnitSize()<2)
		return;

  if(!GetCVars()->e_terrain)
    return;

  if( !m_pLowResTerrainLeafBuffer || 
//      !m_pReflectedTerrainLeafBuffer ||
      !m_pLowResTerrainLeafBuffer->m_SecVertCount || 
      !m_lstReflectedTerrainIdxArray.Count())
    return;

  if(!m_pReflectedTerrainLeafBuffer)
  {
    int nStep = (1<<MAX_MML_LEVEL)*CTerrain::GetHeightMapUnitSize();

    // fill vert buffer
/*    list2<struct_VERTEX_FORMAT_P3F> lstLowResTerrainVertArray;
//    for( int x=0; x<=CTerrain::GetTerrainSize(); x+=nStep )
//    for( int y=0; y<=CTerrain::GetTerrainSize(); y+=nStep )
    {
      struct_VERTEX_FORMAT_P3F vert;
      vert.x = 0;//(float)(x);
      vert.y = 0;//(float)(y);
      vert.z = 0;//(GetZ(x,y));    
      lstLowResTerrainVertArray.Add(vert);
    }
	*/
    // make leaf buffer
    m_pReflectedTerrainLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
      /*&lstLowResTerrainVertArray[0], lstLowResTerrainVertArray.Count(), */0,0,VERTEX_FORMAT_P3F,
      m_lstReflectedTerrainIdxArray.GetElements(), m_lstReflectedTerrainIdxArray.Count(),
      R_PRIMV_TRIANGLES, "ReflectedTerrain", eBT_Dynamic, 1,m_pLowLodCoverMapTex->GetTextureID());

//    m_pLowResTerrainShader = GetRenderer()->EF_LoadShader("TerrainLowLOD", eSH_World);

    assert(m_pReflectedTerrainLeafBuffer);

//    m_nLowResTerrainVertCount = lstLowResTerrainVertArray.Count();
  }

  m_pReflectedTerrainLeafBuffer->SetVertexContainer(m_pLowResTerrainLeafBuffer);

//  if(!m_nRenderStackLevel) 
  {
    if(m_lstReflectedTerrainIdxArray.Count()>60000)
      Warning( 0,0,"Warning: CTerrain::RenderReflectedTerrain: m_lstReflectedTerrainIdxArray.Count()>60000");

    while(m_lstReflectedTerrainIdxArray.Count()>60000)
      m_lstReflectedTerrainIdxArray.DeleteLast();
    m_pReflectedTerrainLeafBuffer->UpdateSysIndices(m_lstReflectedTerrainIdxArray.GetElements(), m_lstReflectedTerrainIdxArray.Count());
    m_pReflectedTerrainLeafBuffer->SetChunk(m_pLowResTerrainShader,0,m_nLowResTerrainVertCount,
      0, m_lstReflectedTerrainIdxArray.Count());

//    m_lstReflectedTerrainIdxArray.Clear();
  }
                     
  m_arrLowResTerrainShaderCustomData[0] = 1;
  m_arrLowResTerrainShaderCustomData[1] = 1;
  m_arrLowResTerrainShaderCustomData[2] = 1.f/CTerrain::GetTerrainSize();    
  m_arrLowResTerrainShaderCustomData[3]  = 0;
  m_arrLowResTerrainShaderCustomData[4] = 0;
  m_pReflectedTerrainLeafBuffer->SetRECustomData(m_arrLowResTerrainShaderCustomData,1);                       

  m_pReflectedTerrainLeafBuffer->m_vBoxMin = Vec3d(0,0,0);
  m_pReflectedTerrainLeafBuffer->m_vBoxMax = Vec3d((float)CTerrain::GetTerrainSize(),(float)CTerrain::GetTerrainSize(),255);

//	CCObject * pObj = GetRenderer()->EF_GetObject(true);	
//	if(GetRenderer()->EF_GetHeatVision())
	//	pObj->m_ObjFlags |= FOB_HEATVISION;

  if(m_pReflectedTerrainLeafBuffer->m_Indices.m_nItems)
    m_pReflectedTerrainLeafBuffer->AddRenderElements();

///  m_lstReflectedTerrainIdxArray.Clear();
}