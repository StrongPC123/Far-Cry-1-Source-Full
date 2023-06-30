////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_sector_render.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Create buffer, copy it into var memory, draw
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_sector.h"
#include "objman.h"
#include "terrain_water.h"
 
// draw tri strips
void CSectorInfo::DrawArray(CArrayInfo * pArrayInfo, CCObject * pTerrainCCObject)
{
	bool bAllowSingePassZ(GetViewCamera().GetFov() < GetCVars()->e_detail_texture_min_fov);

  if(GetCVars()->e_terrain_texture_bind && m_nTextureID && m_pLeafBuffer->m_pMats->Count() && m_pLeafBuffer->m_pMats->GetAt(0).pRE)
  {
		if(m_arrTexOffsets[2] == 1.f/CTerrain::GetTerrainSize())
			m_pLeafBuffer->SetShader(m_pCurShader, m_pTerrain->m_pLowLodCoverMapTex->GetTextureID());
		else
			m_pLeafBuffer->SetShader(m_pCurShader, m_nTextureID);

		if(!GetCVars()->e_terrain_single_pass || !GetCVars()->e_detail_texture_quality)
		{
			for(int i=1; i<MAX_CUSTOM_TEX_BINDS_NUM; i++)
				m_pLeafBuffer->m_pMats->GetAt(0).pRE->m_CustomTexBind[i] = 0;
		}
		else if(!m_pTerrain->m_nRenderStackLevel)
    {
			assert(MAX_DETAIL_LAYERS_IN_SECTOR <= MAX_CUSTOM_TEX_BINDS_NUM);

			for(int i=0; i<MAX_DETAIL_LAYERS_IN_SECTOR; i++)
				m_pLeafBuffer->m_pMats->GetAt(0).pRE->m_CustomTexBind[i+1] = m_arrDetailTexInfo[i] ? m_arrDetailTexInfo[i]->nTexID : 0;

      // slots 0,1,2 used by base texture
      for(int t=0; t<MAX_DETAIL_LAYERS_IN_SECTOR; t++)
      if(m_arrDetailTexInfo[t])
      {
        assert(4+t*8 < ARR_TEX_OFFSETS_SIZE);
        float * pOutParams = m_arrTexOffsets + 4 + t*8;

				float fMinDist = m_arrTexOffsets[3] = 0;
				if(m_arrDetailTexInfo[t]->pMatInfo && !bAllowSingePassZ)
				{ // if material is used - do not render single pass detail texture near camera
					// it will be replaced with material layer
					const float fZProjMaxDistRatio = 3.0f;
					const float fMaxViewDistSq = DETAIL_TEXTURE_VIEW_DIST*DETAIL_TEXTURE_VIEW_DIST;
					float fDistRatio = 1.f/1.7f;
					m_arrTexOffsets[3] = fMinDist = sqrt(fMaxViewDistSq/fDistRatio);
				}

        // setup proj direction
        if(m_arrDetailTexInfo[t]->ucProjAxis == 'X')
        {
          pOutParams[0] = 0;
          pOutParams[1] = m_arrDetailTexInfo[t]->fScaleX;
          pOutParams[2] = 0;
          pOutParams[3] = fMinDist;
          
          pOutParams[4] = 0;
          pOutParams[5] = 0;
          pOutParams[6] = m_arrDetailTexInfo[t]->fScaleY;
          pOutParams[7] = 0;
        }
        else if(m_arrDetailTexInfo[t]->ucProjAxis=='Y')
        {
          pOutParams[0] = m_arrDetailTexInfo[t]->fScaleX;
          pOutParams[1] = 0;
          pOutParams[2] = 0;
          pOutParams[3] = fMinDist;

          pOutParams[4] = 0;
          pOutParams[5] = 0;
          pOutParams[6] = m_arrDetailTexInfo[t]->fScaleY;
          pOutParams[7] = 0;
        }
        else // Z
        {
          pOutParams[0] = 0;
          pOutParams[1] = m_arrDetailTexInfo[t]->fScaleX;
          pOutParams[2] = 0;
          pOutParams[3] = fMinDist;

          pOutParams[4] = m_arrDetailTexInfo[t]->fScaleY;
          pOutParams[5] = 0;
          pOutParams[6] = 0;
          pOutParams[7] = 0;
        }
      }
    }
  }
  else
    m_pLeafBuffer->SetShader(m_pCurShader, 0x1000);

	// render terrain ground
  m_pLeafBuffer->AddRenderElements( pTerrainCCObject, 0, -1, 
		(m_pCurShader == m_pTerrain->m_pTerrainWithFog && m_pFogVolume) ? m_pFogVolume->nRendererVolumeID : 0, 
		EFSLIST_STENCIL );

	// if there are more than 3 detail texture layers - render second pass
	float fZoomFactor = 0.1f+0.9f*(RAD2DEG(GetViewCamera().GetFov())/90.f);      
	if(bAllowSingePassZ && m_arrDetailTexInfo[3] && m_fDistance*fZoomFactor < 90 && !m_pTerrain->m_nRenderStackLevel)
	{
		m_pLeafBuffer->AddRenderElements( pTerrainCCObject, 0, -1, 
			(m_pCurShader == m_pTerrain->m_pTerrainWithFog && m_pFogVolume) ? m_pFogVolume->nRendererVolumeID : 0, 
			EFSLIST_STENCIL, &m_pTerrain->m_matSecondPass );
	}

	// terrain fog pass
  if( m_pCurShader != m_pTerrain->m_pTerrainWithFog && m_pFogVolume && m_pFogVolume->pShader )
	{	
		if(!m_pFogVolume->bOcean || 
			(	m_pTerrain->m_pWater && m_pTerrain->m_pWater->IsWaterVisible() &&
				m_pTerrain->GetObjManager()->m_dwRecursionDrawFlags[Cry3DEngineBase::m_nRenderStackLevel] & DLD_TERRAIN_WATER))
		{
			CCObject * pTerrainFogCCObject = GetRenderer()->EF_GetObject(true);
			pTerrainFogCCObject->m_ObjFlags |= FOB_FOGPASS;
			pTerrainFogCCObject->m_Matrix.SetIdentity();
			if(!m_nRenderStackLevel)
			{
				pTerrainFogCCObject->m_nScissorX1 = GetViewCamera().m_ScissorInfo.x1;
				pTerrainFogCCObject->m_nScissorY1 = GetViewCamera().m_ScissorInfo.y1;
				pTerrainFogCCObject->m_nScissorX2 = GetViewCamera().m_ScissorInfo.x2;
				pTerrainFogCCObject->m_nScissorY2 = GetViewCamera().m_ScissorInfo.y2;
			}
			m_pLeafBuffer->AddRenderElements( pTerrainFogCCObject, 0, -1, m_pFogVolume->nRendererVolumeID, eS_TerrainFogPass );
		}
	}

	// plane on the top of the fog when camera is above this plane
  /*if(	m_pFogVolume && !m_pFogVolume->bOcean &&
			m_pFogVolume->nLastRenderedFrameId != GetFrameID() && 
			m_pFogVolume && m_pFogVolume->pShader && 
			GetViewCamera().GetPos().z > m_pFogVolume->vBoxMax.z)
	{
		m_pFogVolume->nLastRenderedFrameId = GetFrameID();

		SColorVert verts[4];
		verts[0].vert(m_pFogVolume->vBoxMin.x,	m_pFogVolume->vBoxMin.y,	m_pFogVolume->vBoxMax.z);
		verts[0].color.dcolor = (DWORD)-1;
		verts[0].dTC[0]=0; 
		verts[0].dTC[1]=0;

		verts[1].vert(m_pFogVolume->vBoxMax.x,	m_pFogVolume->vBoxMin.y,	m_pFogVolume->vBoxMax.z);
		verts[1].color.dcolor = (DWORD)-1;
		verts[1].dTC[0]=1; 
		verts[1].dTC[1]=0;

		verts[2].vert(m_pFogVolume->vBoxMax.x,	m_pFogVolume->vBoxMax.y,	m_pFogVolume->vBoxMax.z);
		verts[2].color.dcolor = (DWORD)-1;
		verts[2].dTC[0]=1; 
		verts[2].dTC[1]=1;

		verts[3].vert(m_pFogVolume->vBoxMin.x,	m_pFogVolume->vBoxMax.y,	m_pFogVolume->vBoxMax.z);
		verts[3].color.dcolor = (DWORD)-1;
		verts[3].dTC[0]=0; 
		verts[3].dTC[1]=1;

		// fog not affecting this plane
		//GetRenderer()->EF_AddPolyToScene3D(m_pFogVolume->pShader->GetID(),4, verts, pTerrainCCObject);
	}*/
}

// update data in video buffer
void CSectorInfo::UpdateVarBuffer()
{
  CArrayInfo * pArrayInfo = &m_ArrayInfo;

  // if changing lod or there is no buffer allocated - reallocate
  if(!m_pLeafBuffer || m_cPrevGeomMML != m_cGeometryMML)
  { 
    if(m_pLeafBuffer)
      GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);  

    m_pLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
      m_pTerrain->m_lstSectorVertArray.GetElements(), m_pTerrain->m_lstSectorVertArray.Count(), VERTEX_FORMAT_P3F_N_COL4UB_COL4UB,
      pArrayInfo->idx_array.GetElements(), pArrayInfo->idx_array.Count(),
      R_PRIMV_MULTI_STRIPS, "TerrainSector", eBT_Static,
      CTerrain::GetSectorSize()+16, 
	      m_nTextureID ? m_nTextureID : 0x1000, NULL, NULL, false, false);

    assert(m_pLeafBuffer);

    m_nSecVertsCount = m_pTerrain->m_lstSectorVertArray.Count();

    if(GetCVars()->e_terrain_log)
      GetLog()->Log("LeafBuffer created %d", GetSecIndex());

    // Set sector BBox
    m_pLeafBuffer->m_vBoxMin = Vec3d((float)m_nOriginX,(float)m_nOriginY,m_fMinZ);
		m_pLeafBuffer->m_vBoxMax = Vec3d((float)m_nOriginX+CTerrain::GetSectorSize(),
												             (float)m_nOriginY+CTerrain::GetSectorSize(),m_fMaxZ);
  }

  int i; 

  m_pLeafBuffer->UpdateSysIndices(pArrayInfo->idx_array.GetElements(), pArrayInfo->idx_array.Count());

  while(m_pLeafBuffer->m_pMats->Count()>pArrayInfo->strip_info.Count())
    m_pLeafBuffer->m_pMats->Delete(m_pLeafBuffer->m_pMats->Count()-1);

  int j=0;
  for( i=0; i<pArrayInfo->strip_info.Count(); i++)
  {
    if(pArrayInfo->strip_info[i].end - pArrayInfo->strip_info[i].begin < 3)
    {
      j++;
      continue;
    }

    m_pLeafBuffer->SetChunk(m_pCurShader,0,m_nSecVertsCount,
      pArrayInfo->strip_info[i].begin,
      pArrayInfo->strip_info[i].end - pArrayInfo->strip_info[i].begin, i-j);
    
    if(m_pLeafBuffer->m_pMats->Get(i-j)->pRE)
      m_pLeafBuffer->m_pMats->Get(i-j)->pRE->m_CustomData = m_arrTexOffsets;
  }

  assert(m_pLeafBuffer->m_pMats->Count() <= CTerrain::GetSectorSize()+16);
}

void CSectorInfo::ReleaseHeightMapVertBuffer()
{
  if(m_pLeafBuffer)
    GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);  
  m_pLeafBuffer=0;
  m_ArrayInfo.Clear();
  m_pTerrain->m_lstSectorVertArray.Clear();
}

// fill vertex buffer
void CSectorInfo::FillBuffer(int nStep)
{
  bool bRGB = (GetRenderer()->GetFeatures() & RFT_RGBA) != 0;
  if ((GetRenderer()->GetFeatures() & RFT_HW_MASK) == RFT_HW_GF2)
    bRGB = true;

  for( int x=m_nOriginX; x<=m_nOriginX+CTerrain::GetSectorSize(); x+=nStep )
  for( int y=m_nOriginY; y<=m_nOriginY+CTerrain::GetSectorSize(); y+=nStep )
  {
    struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB vert;   
    vert.xyz.x = (float)(x);
    vert.xyz.y = (float)(y);
    vert.xyz.z = (m_pTerrain->GetZ(x,y));    

    // calculate surface normal
    float sx;
    if((x+CTerrain::GetHeightMapUnitSize())<CTerrain::GetTerrainSize() && x>=CTerrain::GetHeightMapUnitSize())
      sx = m_pTerrain->GetZ(x+CTerrain::GetHeightMapUnitSize(),y  ) - m_pTerrain->GetZ(x-CTerrain::GetHeightMapUnitSize(),y  );
    else
      sx = 0;

    float sy;
    if((y+CTerrain::GetHeightMapUnitSize())<CTerrain::GetTerrainSize() && y>=CTerrain::GetHeightMapUnitSize())
      sy = m_pTerrain->GetZ(x  ,y+CTerrain::GetHeightMapUnitSize()) - m_pTerrain->GetZ(x  ,y-CTerrain::GetHeightMapUnitSize());
    else
      sy = 0;

    float fBright = 1.f - 0.5f*(float)m_pTerrain->IsBurnedOut(x,y);

    // z component of normal will be used as point brightness ( for burned terrain )
    Vec3d vNorm = Vec3d(-sx/CTerrain::GetHeightMapUnitSize(), -sy/CTerrain::GetHeightMapUnitSize(), 1.0f);
    vNorm.NormalizeFast();
		vert.normal = vNorm;

    uchar ucSurfaceTypeID = m_pTerrain->GetSurfaceTypeID(x,y);
    if (bRGB)
    {
      vert.color.bcolor[0] = byte(m_arrDetailTexInfo[0] && m_arrDetailTexInfo[0]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
      vert.color.bcolor[1] = byte(m_arrDetailTexInfo[1] && m_arrDetailTexInfo[1]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
      vert.color.bcolor[2] = byte(m_arrDetailTexInfo[2] && m_arrDetailTexInfo[2]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.color.bcolor[3] = (byte)FtoI(fBright*255.0f);

			vert.seccolor.bcolor[0] = byte(m_arrDetailTexInfo[3] && m_arrDetailTexInfo[3]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.seccolor.bcolor[1] = byte(m_arrDetailTexInfo[4] && m_arrDetailTexInfo[4]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.seccolor.bcolor[2] = byte(m_arrDetailTexInfo[5] && m_arrDetailTexInfo[5]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.seccolor.bcolor[3] = 0;
		}
    else
    {
			vert.color.bcolor[2] = byte(m_arrDetailTexInfo[0] && m_arrDetailTexInfo[0]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.color.bcolor[1] = byte(m_arrDetailTexInfo[1] && m_arrDetailTexInfo[1]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.color.bcolor[0] = byte(m_arrDetailTexInfo[2] && m_arrDetailTexInfo[2]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.color.bcolor[3] = (byte)FtoI(fBright*255.0f);

			vert.seccolor.bcolor[2] = byte(m_arrDetailTexInfo[3] && m_arrDetailTexInfo[3]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.seccolor.bcolor[1] = byte(m_arrDetailTexInfo[4] && m_arrDetailTexInfo[4]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.seccolor.bcolor[0] = byte(m_arrDetailTexInfo[5] && m_arrDetailTexInfo[5]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
			vert.seccolor.bcolor[3] = byte(m_arrDetailTexInfo[6] && m_arrDetailTexInfo[6]->ucThisSurfaceTypeId == ucSurfaceTypeID)*255;
		}

    m_pTerrain->m_lstSectorVertArray.Add(vert);
  }
}

// entry point
void CSectorInfo::RenderSector(CCObject * pTerrainCCObject)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

//	if(m_pTerrain->GetSecInfo(GetViewCamera().GetPos())!=this)
	//	return;

  assert(m_cGeometryMML>=0 && m_cGeometryMML<= MAX_MML_LEVEL);
  
  int nStep = (1<<m_cGeometryMML)*CTerrain::GetHeightMapUnitSize();

  // get status of borders
  bool r = ( m_nOriginX < (CTerrain::GetTerrainSize()-CTerrain::GetSectorSize()) && m_cGeometryMML < m_pTerrain->GetSecMML(m_nOriginX+CTerrain::GetSectorSize(),m_nOriginY) );
  bool l = ( m_nOriginX >= CTerrain::GetSectorSize()             && m_cGeometryMML < m_pTerrain->GetSecMML(m_nOriginX-CTerrain::GetSectorSize(),m_nOriginY) );
  bool t = ( m_nOriginY < (CTerrain::GetTerrainSize()-CTerrain::GetSectorSize()) && m_cGeometryMML < m_pTerrain->GetSecMML(m_nOriginX,m_nOriginY+CTerrain::GetSectorSize()) );
  bool b = ( m_nOriginY >= CTerrain::GetSectorSize()             && m_cGeometryMML < m_pTerrain->GetSecMML(m_nOriginX,m_nOriginY-CTerrain::GetSectorSize()) );

  int nBoundCode = r + 2*l + 4*t + 8*b;

  if(!m_pLeafBuffer && m_pTerrain->m_nRenderStackLevel)
    return;

  if(m_pLeafBuffer)
  if(/*m_pTerrain->m_lstSectorVertArray.Count() && */( m_pTerrain->m_nRenderStackLevel ||( m_cPrevGeomMML == m_cGeometryMML && nBoundCode == m_cCurrBoundCode )))
  { DrawArray(&m_ArrayInfo, pTerrainCCObject); return; }

  m_cCurrBoundCode = nBoundCode;

  assert(nStep);

  if(nStep > (CTerrain::GetSectorSize()>>1))
    nStep = (CTerrain::GetSectorSize()>>1);

  int x1=0,y1=0;
  int x2=CTerrain::GetSectorSize();
  int y2=CTerrain::GetSectorSize();

  int X1 = x1;
  int Y1 = y1;
  int X2 = x2;
  int Y2 = y2;

  m_ArrayInfo.Clear();
  
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
  
  if(r)
  { //  right side ok
    int x = x2-nStep;
    m_ArrayInfo.BeginStrip();
    for( int y=y1; y<=y2; y+=nStep )
    {     
      m_ArrayInfo.AddIndex(x,y,nStep);

      if(y&nStep && r)
        m_ArrayInfo.AddIndex(x+nStep, y+nStep,nStep);
      else
        m_ArrayInfo.AddIndex(x+nStep, y,nStep);
    }
    m_ArrayInfo.EndStrip();

    X2-=nStep;
  }

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
  
  if(l)
  { //  left side ok
    int x = x1;
    m_ArrayInfo.BeginStrip();
    for( int y=y1; y<=y2; y+=nStep )
    {
      if(y&nStep && l)
        m_ArrayInfo.AddIndex(x, y-nStep,nStep);
      else
        m_ArrayInfo.AddIndex(x, y,nStep);

      m_ArrayInfo.AddIndex(x+nStep, y,nStep);
    }
    m_ArrayInfo.EndStrip();
    X1+=nStep;
  }

/////////////////////////////////////////////////////////////////////////////////////////////
//  glCullFace(GL_FRONT);
/////////////////////////////////////////////////////////////////////////////////////////////
  
  if(t)
  { //  top side ok
    int y = y2-nStep;
    m_ArrayInfo.BeginStrip();

    m_ArrayInfo.AddIndex(x1,y,nStep); // deg

    for( int x=x1; x<=x2; x+=nStep )
    {
      m_ArrayInfo.AddIndex(x,y,nStep);

      if(x&nStep && t)
        m_ArrayInfo.AddIndex(x+nStep,y+nStep,nStep);
      else
        m_ArrayInfo.AddIndex(x,y+nStep,nStep);
    }
    m_ArrayInfo.EndStrip();
    Y2-=nStep;
  }

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
  
  if(b)
  { //  bottom side
    int y = y1;
    m_ArrayInfo.BeginStrip();

    if(x1&nStep && b) // deg
      m_ArrayInfo.AddIndex(x1-nStep,y,nStep);
    else
      m_ArrayInfo.AddIndex(x1,y,nStep);
  
    for( int x=x1; x<=x2; x+=nStep )
    {
      if(x&nStep && b)
        m_ArrayInfo.AddIndex(x-nStep,y,nStep);
      else
        m_ArrayInfo.AddIndex(x,y,nStep);

      m_ArrayInfo.AddIndex(x,y+nStep,nStep);
    }
    m_ArrayInfo.EndStrip();
    Y1+=nStep;
  }

/////////////////////////////////////////////////////////////////////////////////////////////
//glCullFace(GL_BACK);
/////////////////////////////////////////////////////////////////////////////////////////////

  if(m_bHasHoles && nStep == CTerrain::GetHeightMapUnitSize())
  { // draw with holes
    for( int x=X1; x<X2; x+=nStep )
    {   
      bool in_strip = false;

      for( int y=Y1; y<=Y2; y+=nStep )
      {
        if(!m_pTerrain->GetHole(m_nOriginX+x,m_nOriginY+y))
        {
          if(!in_strip)
            m_ArrayInfo.BeginStrip();
          in_strip = true;

          m_ArrayInfo.AddIndex(x,y,nStep);
          m_ArrayInfo.AddIndex(x+nStep,y,nStep);
        }
        else if(in_strip)
        { // hole
          m_ArrayInfo.AddIndex(x,y,nStep);
          m_ArrayInfo.AddIndex(x+nStep,y,nStep);
          
          m_ArrayInfo.EndStrip();
          in_strip = false;
        }
      }

      if(in_strip)
        m_ArrayInfo.EndStrip();
    }
  }
  else
  { // base index (degenerative strip)
    if(nStep == CTerrain::GetHeightMapUnitSize()) // non degenerative only for closest sectors (deg. tris sometimes are visible)
    {
      for( int x=X1; x<X2; x+=nStep )
      {   
        m_ArrayInfo.BeginStrip();
        for( int y=Y1; y<=Y2; y+=nStep )
        {
          m_ArrayInfo.AddIndex(x,y,nStep);
          m_ArrayInfo.AddIndex(x+nStep,y,nStep);
        }
        m_ArrayInfo.EndStrip();
      }
    }
    else
    { // degenerative strips
      m_ArrayInfo.BeginStrip();
      for( int x=X1; x<X2; x+=nStep )
      {   
        int y;
        for( y=Y1; y<=Y2; y+=nStep )
        {
          m_ArrayInfo.AddIndex(x,y,nStep);
          m_ArrayInfo.AddIndex(x+nStep,y,nStep);
        }

        x+=nStep;

        if(x<X2)
        {
          for( y=Y2; y>=Y1; y-=nStep )
          {
            m_ArrayInfo.AddIndex(x+nStep,y,nStep);
            m_ArrayInfo.AddIndex(x,y,nStep);
          }
        }
      }
      m_ArrayInfo.EndStrip();
    }
  }

  // new indices are created at this point

  if(!m_pLeafBuffer || m_cPrevGeomMML != m_cGeometryMML)// || !m_pTerrain->m_lstSectorVertArray.Count())
  {
    m_pTerrain->m_lstSectorVertArray.Clear();
    FillBuffer(nStep);
  }

  UpdateVarBuffer();

//  GetRenderer()->UpdateIndices(m_pLeafBuffer->m_pVertexBuffer, 
  //  pipBuffer,
    //&m_ArrayInfo.idx_array[0],m_ArrayInfo.idx_array.Count());
/*
  CArrayInfo * pArrayInfo = &m_ArrayInfo;
  assert(m_pLeafBuffer->m_Indices.Count() >= pArrayInfo->idx_array.Count());
  memcpy(&m_pLeafBuffer->m_Indices[0], &pArrayInfo->idx_array[0], m_pLeafBuffer->m_Indices.Count()*sizeof(unsigned short));
  */

//  CArrayInfo * pArrayInfo = &m_ArrayInfo;
  //m_pLeafBuffer->UpdateIndices(&pArrayInfo->idx_array);

  m_cPrevGeomMML = m_cGeometryMML;

  DrawArray(&m_ArrayInfo, pTerrainCCObject);
}

void CSectorInfo::AddLowResSectorIndex(int _x, int _y, int _step, int _lod)
{
  unsigned short id = _x/_step*(CTerrain::GetTerrainSize()/_step+1) + _y/_step;

	m_nLowResTerrainIdxRange[_lod][0] = min(m_nLowResTerrainIdxRange[_lod][0],id);
	m_nLowResTerrainIdxRange[_lod][1] = max(m_nLowResTerrainIdxRange[_lod][1],id);

  m_lstLowResTerrainIdxArray[_lod].Add(id);
}

void CSectorInfo::MergeSectorIntoLowResTerrain(bool bCalcFarTerrain)
{
	// prepare index buffers if not ready // todo: why for all lods
  for(int lod=0; lod<2; lod++)
  {
    if(m_lstLowResTerrainIdxArray[lod].Count()==0)
    {
      int nVertStep = (1<<(MAX_MML_LEVEL))*CTerrain::GetHeightMapUnitSize();
      int nStep = (1<<(MAX_MML_LEVEL+lod))*CTerrain::GetHeightMapUnitSize();
      for( int x=m_nOriginX; x< m_nOriginX+CTerrain::GetSectorSize(); x+=nStep )
      for( int y=m_nOriginY; y< m_nOriginY+CTerrain::GetSectorSize(); y+=nStep )
      {
        AddLowResSectorIndex(x,       y,       nVertStep,lod);
        AddLowResSectorIndex(x+nStep, y,       nVertStep,lod);
        AddLowResSectorIndex(x,       y+nStep, nVertStep,lod);

        AddLowResSectorIndex(x,       y+nStep, nVertStep,lod);
        AddLowResSectorIndex(x+nStep, y,       nVertStep,lod);
        AddLowResSectorIndex(x+nStep, y+nStep, nVertStep,lod);
      }
    }
  }

  if(bCalcFarTerrain)
	{ // low res optimized rendering can be used only on low res sectors
		assert(m_cGeometryMML == MAX_MML_LEVEL);
    m_pTerrain->MergeLowResTerrainSectorIndices(&m_lstLowResTerrainIdxArray[0]);
    m_cPrevGeomMML = -1;
    m_nLastMergedFrameID = GetFrameID();
	}
  else    
    m_pTerrain->MergeReflectedTerrainSectorIndices(&m_lstLowResTerrainIdxArray[1]);
}

void CSectorInfo::GenerateIndicesForQuad(int x1, int y1, int x2, int y2, CArrayInfo * pArrayInfo, ShadowMapFrustum * pFrustum, Vec3d * pvFrustPos, float fFrustScale)
{
  pArrayInfo->Clear();

  x1 -= m_nOriginX;
  y1 -= m_nOriginY;
  x2 -= m_nOriginX;
  y2 -= m_nOriginY;

  if(x1<0) x1=0; else if(x1>=CTerrain::GetSectorSize()) return;
  if(y1<0) y1=0; else if(y1>=CTerrain::GetSectorSize()) return;
  if(x2<=0) return; else if(x2>CTerrain::GetSectorSize()) x2=CTerrain::GetSectorSize();
  if(y2<=0) return; else if(y2>CTerrain::GetSectorSize()) y2=CTerrain::GetSectorSize();

  int nStep = (1<<m_cGeometryMML)*CTerrain::GetHeightMapUnitSize();

//	if(pFrustum)
	//	pFrustum->InitFrustum(1.f,GetRenderer());

  pArrayInfo->BeginStrip();
  for( int x=x1; x<x2; x+=nStep )
  {   
    for( int y=y1; y<y2; y+=nStep )
    {
			if(!m_pTerrain->GetHole(m_nOriginX+x,m_nOriginY+y))
			{
/*				Vec3d vPos(m_nOriginX+x+0.5f*nStep,m_nOriginY+y+0.5f*nStep,0);
				vPos.z = m_pTerrain->GetZApr(vPos.x,vPos.y);

				if(pFrustum && pvFrustPos && !pFrustum->IsSphereInsideFrustum(vPos-*pvFrustPos,nStep,1.f,GetRenderer()))
					continue;
*/
				pArrayInfo->AddIndex(x,y,nStep);
				pArrayInfo->AddIndex(x+nStep,y,nStep);
				pArrayInfo->AddIndex(x,y+nStep,nStep);

				pArrayInfo->AddIndex(x+nStep,y,nStep);
				pArrayInfo->AddIndex(x+nStep,y+nStep,nStep);
				pArrayInfo->AddIndex(x,y+nStep,nStep);
			}
    }
  }
  pArrayInfo->EndStrip();
}

CLeafBuffer * CSectorInfo::MakeSubAreaLeafBuffer(const Vec3d & vPos, float fRadius, 
                                                 CLeafBuffer * pPrevLeafBuffer, 
                                                 IShader * pShader,
                                                 bool bRecalcLeafBuffer,
																								 const char * szLSourceName,
																								 ShadowMapFrustum * pFrustum,
																								 Vec3d * pvFrustPos, float fFrustScale)
{
  bRecalcLeafBuffer |= (pPrevLeafBuffer && m_pLeafBuffer && 
		(	pPrevLeafBuffer->GetVertexContainer() != m_pLeafBuffer || // vert container was deleted or should be changed
			pPrevLeafBuffer->m_UpdateFrame != m_pLeafBuffer->m_UpdateFrame )); // vert container was modified

  if(!m_pLeafBuffer || m_pLeafBuffer->m_SecVertCount==0)
  {
    if(pPrevLeafBuffer)
    {
      GetRenderer()->DeleteLeafBuffer(pPrevLeafBuffer);
      pPrevLeafBuffer=0;
    }        

    return 0;
  }

  if(!bRecalcLeafBuffer && pPrevLeafBuffer)
  {
    pPrevLeafBuffer->SetVertexContainer(m_pLeafBuffer);
//    pPrevLeafBuffer->m_SecVertCount = m_pLeafBuffer->m_SecVertCount;
    pPrevLeafBuffer->SetShader(pShader,m_nTextureID);
    return pPrevLeafBuffer;
  }

  m_ArrayInfo_MSALB.Clear();

  list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB> EmptyVertBuffer;
  EmptyVertBuffer.Add(struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB());

  if(!pPrevLeafBuffer)
  {
//		char * sLBName = new char[128];
	//	snprintf(sLBName,"TerrainLightPass(%d,%d,%s)",m_nOriginX,m_nOriginY,szLSourceName);
    pPrevLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
      EmptyVertBuffer.GetElements(), EmptyVertBuffer.Count(), VERTEX_FORMAT_P3F_N_COL4UB_COL4UB,
      m_ArrayInfo_MSALB.idx_array.GetElements(), m_ArrayInfo_MSALB.idx_array.Count(),
      R_PRIMV_TRIANGLES, szLSourceName, eBT_Static, 1, m_nTextureID, NULL, NULL, false, false);
  }

  pPrevLeafBuffer->SetVertexContainer(m_pLeafBuffer);
  //pPrevLeafBuffer->m_SecVertCount = m_pLeafBuffer->m_SecVertCount;

  GenerateIndicesForQuad(int(vPos.x-fRadius), int(vPos.y-fRadius), 
		int(vPos.x+fRadius), int(vPos.y+fRadius), &m_ArrayInfo_MSALB, pFrustum, pvFrustPos, fFrustScale);

  pPrevLeafBuffer->UpdateSysIndices(m_ArrayInfo_MSALB.idx_array.GetElements(), m_ArrayInfo_MSALB.idx_array.Count());

  pPrevLeafBuffer->SetChunk(pShader, 0, (*m_pLeafBuffer).m_SecVertCount, 0, m_ArrayInfo_MSALB.idx_array.Count());
  pPrevLeafBuffer->SetShader(pShader,m_nTextureID);
	pPrevLeafBuffer->m_UpdateFrame = m_pLeafBuffer->m_UpdateFrame;

  return pPrevLeafBuffer;
}