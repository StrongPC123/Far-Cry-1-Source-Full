////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   detail_grass.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: create and draw grass/detail object volume right in front of camera
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "statobj.h"
#include "detail_grass.h"

#include <IXMLDOM.h>

// class to store vertex buffer for single grass object 
struct GrassType : public Cry3DEngineBase
{
public:
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pVertices;
  uint * uipIndices, nIndCount, nVertCount;
  int nTexID;
  char sObjectFileName[256];
	CStatObj * m_pObject;

  GrassType(char * fname)
  { // load object and make buffers
    memset(this,0,sizeof(*this));
    strncpy(sObjectFileName,fname,sizeof(sObjectFileName));

    m_pObject = new CStatObj( );
    if(!m_pObject->Load(fname, NULL, evs_ShareAndSortForCache, false, false))    
    {
      delete m_pObject;
			m_pObject=0;
      return;
    }

    int tris_count = 0;
    CLeafBuffer * pLeafBuffer = m_pObject->GetLeafBuffer();

		if(!pLeafBuffer)
			return; // objects may have no leaf buffer created on dedicated server

    pLeafBuffer->GetIndices(&tris_count);
    tris_count/=3;

    if(tris_count>12)
    {
#if !defined(LINUX)
      Warning( 0,0,"Detail object has more than 12 triangles, skiped: %s (%d tris)",fname,tris_count );
#endif
      return;
    }

    nVertCount = pLeafBuffer->m_SecVertCount;

    pVertices  = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[nVertCount];
    uipIndices = new uint[tris_count*3];

    Vec3d vCenter = (m_pObject->GetBoxMax()+m_pObject->GetBoxMin())/2;

    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pSecVerts = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)pLeafBuffer->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
    ushort *pInds = pLeafBuffer->GetIndices(NULL);
    int nStrPos;
    byte *pPos = pLeafBuffer->GetPosPtr(nStrPos, 0, true);
    int nStrST;
    byte *pST = pLeafBuffer->GetUVPtr(nStrST, 0, true);

    for(int i=0; i<tris_count*3; i++)
    {     
      uint id = pInds[i];
      Vec3d *vSrcPos = (Vec3d *)&pPos[nStrPos*id];
      float *fSrcST = (float *)&pST[nStrST*id];

      pVertices[id].xyz.x = vSrcPos->x - vCenter.x;
      pVertices[id].xyz.y = vSrcPos->y - vCenter.y;
      pVertices[id].xyz.z = vSrcPos->z;// - pObject->GetBoxMin().z;
      pVertices[id].st[0] = fSrcST[0];
      pVertices[id].st[1] = fSrcST[1];

      /*      if(pLeafBuffer->m_pLoadedColors)
      {
      pVertices[id].r = pSecVerts[id].color[0];
      pVertices[id].g = pSecVerts[id].color[1];
      pVertices[id].b = pSecVerts[id].color[2];
      pVertices[id].a = 255;
      }
      else*/
      {
        pVertices[id].color.dcolor = -1;
      }

      uipIndices[nIndCount++] = id;
    }

    if( m_pObject->GetLeafBuffer()->m_pMats->Count() && 
				m_pObject->GetLeafBuffer()->m_pMats->Get(0)->shaderItem.m_pShader &&
				m_pObject->GetLeafBuffer()->m_pMats->Get(0)->shaderItem.m_pShaderResources->m_Textures[EFTT_DIFFUSE] &&
				m_pObject->GetLeafBuffer()->m_pMats->Get(0)->shaderItem.m_pShaderResources->m_Textures[EFTT_DIFFUSE]->m_TU.m_ITexPic )
    {
      nTexID = (m_pObject->GetLeafBuffer()->m_pMats->Get(0)->shaderItem.m_pShaderResources->m_Textures[EFTT_DIFFUSE]->m_TU.m_ITexPic)->GetTextureID();
    }
    else
    {
      nTexID = GetRenderer()->LoadTexture("none");
    }

    //	assert(nTexID>=4096);
  }

  ~GrassType()
  {
		delete m_pObject;
    delete [] pVertices;
    delete [] uipIndices;
  }

  int GetMemoryUsage()
  {
    return sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F) * nVertCount + sizeof(uint) * nIndCount;
  }
};

// load objects
CDetailGrass::CDetailGrass(XDOM::IXMLDOMNodeListPtr pDetTexTagList)
{
  memset(this,0,sizeof(*this));

  //  m_pGrassVertices = 0;
  m_GrassVerticesCount = 0;
  m_GrassFocusX = m_GrassFocusY = -CTerrain::GetTerrainSize(); // grass focus
  m_nGrassDensity = 2;

  if( !m_nGrassDensity )
    return;

  UpdateLoadingScreen("Loading detail objects ..."); 
  m_GrassModelsArray.Clear(); 

  // load detail textures
  if (pDetTexTagList)
  {
    XDOM::IXMLDOMNodePtr pDetTexTag;
    pDetTexTagList->reset();
    pDetTexTag = pDetTexTagList->nextNode();
    XDOM::IXMLDOMNodeListPtr pDetTexList;
    pDetTexList = pDetTexTag->getElementsByTagName("SurfaceType");
    if (pDetTexList)
    {
      pDetTexList->reset();
      XDOM::IXMLDOMNodePtr pDetTex;
      int nId = 0;
      while (pDetTex = pDetTexList->nextNode())
      {
        XDOM::IXMLDOMNodeListPtr pDetailObjectsList = pDetTex->getElementsByTagName("DetailObjects");
        pDetailObjectsList->reset();
        XDOM::IXMLDOMNodePtr pDetailObject;
        while(pDetailObject = pDetailObjectsList->nextNode())
        {
          XDOM::IXMLDOMNodeListPtr pObjList = pDetailObject->getElementsByTagName("Object");
          if(pObjList)
          {
            XDOM::IXMLDOMNodePtr pObj;
            pObjList->reset();
            while(pObj = pObjList->nextNode())
            {
              XDOM::IXMLDOMNodePtr pName = pObj->getAttribute("Name");
              char sObjectsName[256];
              strcpy(sObjectsName,pName->getText());
              if(!sObjectsName[0])
                break;

              // try to find already loaded
              int l=0;
              for(l=0; l<m_GrassModelsArray.Count(); l++)
              {
                if(strcmp(m_GrassModelsArray[l]->sObjectFileName,sObjectsName)==0)
                  break;
              }

              if(l>=m_GrassModelsArray.Count())
              { // not found
                GrassType * triData = new GrassType( sObjectsName );

                if(triData && (*triData).nIndCount)
                  m_GrassModelsArray.Add(triData);
                else
                {
                  delete triData;
                  break;
                }

                m_arrlstSurfaceObjects[nId].Add(triData);
              }
              else
              { // use already loaded
                m_arrlstSurfaceObjects[nId].Add(m_GrassModelsArray[l]);
              }
            }
          }
        }
        nId++;
      }
    }
  }

  UpdateLoadingScreen("  %d detail objects loaded", m_GrassModelsArray.Count()); 

  m_GrassTID = 4096;
  for(int t=0; t<m_GrassModelsArray.Count(); t++)
  {
    if(m_GrassModelsArray[t]->nTexID && m_GrassModelsArray[t]->nTexID != 4096)
    { // if some valid texture found - ise it for all objects
      m_GrassTID = m_GrassModelsArray[t]->nTexID;
      break;
    }
  }

  if(!m_GrassTID && m_GrassModelsArray.Count())
  {
#if !defined(LINUX)
    Warning( 0,0,"Texture for detail objects was not loaded");
#endif
    m_GrassTID = 0x1000;
  }

  if(!m_GrassModelsArray.Count())
    return;

  m_pShader = GetRenderer()->EF_LoadShader("TerrainDetailObjects", eSH_World, 0);

  // make leaf buffer
  //  m_pGrassVertices = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[DETAIL_GRASS_PIP_BUFFER_SIZE];
  m_GrassIndices.PreAllocate(DETAIL_GRASS_PIP_BUFFER_SIZE*3,DETAIL_GRASS_PIP_BUFFER_SIZE*3);

  m_pLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
    0/*m_pGrassVertices*/, DETAIL_GRASS_PIP_BUFFER_SIZE, VERTEX_FORMAT_P3F_COL4UB_TEX2F,
    m_GrassIndices.GetElements(), m_GrassIndices.Count(), R_PRIMV_TRIANGLES, "Grass", eBT_Dynamic, 1 , 0, PrepareBufferCallback, this);
  m_pLeafBuffer->SetRECustomData(m_arrfShaderInfo);

  assert(m_pLeafBuffer);

  m_pLeafBuffer->SetChunk(m_pShader,0,DETAIL_GRASS_PIP_BUFFER_SIZE,0,m_GrassIndices.Count(),0);
}

bool CDetailGrass::PrepareBufferCallback(CLeafBuffer * pLeafBuffer, bool bNeedTangents)
{
  CDetailGrass * pThis = (CDetailGrass *)pLeafBuffer->m_pCustomData;

  //  if(pTerrain->m_nRenderStackLevel==0)
  {
//    Matrix44 mat;
  //  pThis->GetRenderer()->GetModelViewMatrix(mat.GetData());
    Vec3d forward = -pThis->GetViewCamera().GetVCMatrixD3D9().GetColumn(2);	

    //CELL_CHANGED_BY_IVO
    //forward(-mat.cell(2), -mat.cell(6), -mat.cell(10)); 

    forward.Normalize();

    if(!forward.x && !forward.y)
      return false;

    Vec3d vCamPos = pThis->GetViewCamera().GetPos();  

    int c_X = int(vCamPos.x + forward.x*CAMERA_GRASS_SHIFT);
    int c_Y = int(vCamPos.y + forward.y*CAMERA_GRASS_SHIFT);

    //  GetRenderer()->DrawBall(c_X,c_Y,m_pTerrain->GetZApr(c_X,c_Y),1);

    // check focus
    float dx = float(c_X-pThis->m_GrassFocusX);
    float dy = float(c_Y-pThis->m_GrassFocusY);

    float d2 = (dx*dx+dy*dy);

    if(d2>16 && pThis->m_pTerrain)
    {
      pThis->m_GrassFocusX = c_X;
      pThis->m_GrassFocusY = c_Y;

      if(pLeafBuffer->m_pVertexBuffer && pLeafBuffer->m_pVertexBuffer->m_vertexformat == VERTEX_FORMAT_P3F_COL4UB_TEX2F)
      {
        pThis->GetRenderer()->UpdateBuffer(pLeafBuffer->m_pVertexBuffer,0,0,0);
        pThis->CreateSectorGrass(int(CAMERA_GRASS_SHIFT+2),int(1), 
          (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)pLeafBuffer->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData);

				// unlock
				pThis->GetRenderer()->UpdateBuffer(pLeafBuffer->m_pVertexBuffer, NULL, 0, true, 0, 1);

				//pLeafBuffer->UpdateSysIndices(pThis->m_GrassIndices.GetElements(), pThis->m_GrassIndices.Count());
				pLeafBuffer->UpdateVidIndices(pThis->m_GrassIndices.GetElements(), pThis->m_GrassIndices.Count());

        if(pThis->m_GrassIndices.Count()==0)
          pLeafBuffer->SetChunk(pThis->m_pShader,0,-1,0,-1,0);
        else
          pLeafBuffer->SetChunk(pThis->m_pShader,0,pThis->m_GrassVerticesCount,0,pThis->m_GrassIndices.Count(),0);


        pLeafBuffer->SetShader(pThis->m_pShader,pThis->m_GrassTID);
      }
			else
			{
				assert(0);
			}
    }                                        

    // set alpha glow texgen params
    const float fDistFadeK = 1.f/(CAMERA_GRASS_SHIFT*4.f);
    pThis->m_arrfShaderInfo[2] = fDistFadeK;
    pThis->m_arrfShaderInfo[3] = 0.5f-vCamPos.y*fDistFadeK;
    pThis->m_arrfShaderInfo[4] = 0.5f-vCamPos.x*fDistFadeK;
//		assert(pThis->m_pLeafBuffer && pThis->m_pLeafBuffer == pLeafBuffer);
		if(pThis->m_pLeafBuffer) // lb may be not created yet, this call can come from CreateLeafBufferInitialized()
			pThis->m_pLeafBuffer->SetRECustomData(pThis->m_arrfShaderInfo);
  }

  return true;
}

CDetailGrass::~CDetailGrass()
{
  for(int i=0; i<m_GrassModelsArray.Count(); i++)
    delete m_GrassModelsArray[i];

  m_GrassModelsArray.Reset();

  if(m_pLeafBuffer)
    GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);  
  m_pLeafBuffer=0;

  //  delete [] m_pGrassVertices; m_pGrassVertices=0;

  ///  GetRenderer()->RemoveTexture(m_GrassTID);
}

// copy object into buffer
void CDetailGrass::AddIndexedArray(GrassType * o, float X, float Y, float Z, float fObjBr, 
                                   float fSizeRatio, float fXSign, float fYSign, int nSwapXY,
                                   struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pGrassVertices)
{
  if(fObjBr > 1.f)
    fObjBr = 1.f;

  fObjBr *= 255.f;

  assert(o->nIndCount%3 == 0);

  for( uint i=0; i<o->nIndCount; i+=3 )
  {
    assert(o->uipIndices[i]<o->nVertCount);
    m_GrassIndices.Add(o->uipIndices[i+0] + m_GrassVerticesCount);

    assert(o->uipIndices[i+1]<o->nVertCount);
    m_GrassIndices.Add(o->uipIndices[i+1] + m_GrassVerticesCount);

    assert(o->uipIndices[i+2]<o->nVertCount);
    m_GrassIndices.Add(o->uipIndices[i+2] + m_GrassVerticesCount);
  }

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pVert       = &pGrassVertices[m_GrassVerticesCount];
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pSourceVert = &o->pVertices[0];

  unsigned char grass_r = (unsigned char)fObjBr;
  unsigned char grass_g = (unsigned char)fObjBr;
  unsigned char grass_b = (unsigned char)fObjBr;

  for(uint v=0; v<o->nVertCount; v++)
  { 
    switch(nSwapXY)
    {
    case 0:
      pVert->xyz.x = (pSourceVert->xyz.x*fSizeRatio)*fXSign + X;
      pVert->xyz.y = (pSourceVert->xyz.y*fSizeRatio)*fYSign + Y;
      pVert->xyz.z = (pSourceVert->xyz.z/fSizeRatio)				+ Z;
      break;

    case 1: // swap
      pVert->xyz.x = (pSourceVert->xyz.y*fSizeRatio)*fXSign + X;
      pVert->xyz.y = (pSourceVert->xyz.x*fSizeRatio)*fYSign + Y;
      pVert->xyz.z = (pSourceVert->xyz.z/fSizeRatio)				+ Z;
      break;
    }

    pVert->st[0] = pSourceVert->st[0];
    pVert->st[1] = pSourceVert->st[1];

    pVert->color.bcolor[0] = grass_r;
    pVert->color.bcolor[1] = grass_g;
    pVert->color.bcolor[2] = grass_b;
    pVert->color.bcolor[3] = 255;

    pVert++;
    pSourceVert++;
  }

  m_GrassVerticesCount += o->nVertCount;
}

void CDetailGrass::CreateSectorGrassInUnit(const int x, const int y, const int nStep, 
                                           struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pGrassVertices)
{
  const float grass_level_min = m_pTerrain->GetWaterLevel() + 0.25f;
  if(m_pTerrain->GetZSafe(m_GrassFocusX+x,m_GrassFocusY+y)<grass_level_min)
    return;

  if(m_GrassVerticesCount+4096 > DETAIL_GRASS_PIP_BUFFER_SIZE)
  {
    GetLog()->Log("CreateSectorGrass: grass buffer overflow");
    GetLog()->Log("grass_verts = %d/%d", m_GrassVerticesCount, int(DETAIL_GRASS_PIP_BUFFER_SIZE));
    return;
  }

  srand(((m_GrassFocusX+x)*(m_GrassFocusY+y)));
  srand(rand());
  srand(rand());

  int nSurfaceID = 0;

  int nUnitSize = CTerrain::GetHeightMapUnitSize();

  // todo skip this if there is has_holes flag not set in sector
  for(int _x = m_GrassFocusX+x-nUnitSize; _x <= m_GrassFocusX+x+nUnitSize; _x += nUnitSize)
    for(int _y = m_GrassFocusY+y-nUnitSize; _y <= m_GrassFocusY+y+nUnitSize; _y += nUnitSize)
    {
      if(m_pTerrain->IsBurnedOut(_x-1,_y-1))
        return;

      nSurfaceID = m_pTerrain->GetSurfaceTypeID(_x-1,_y-1);
      if(nSurfaceID == STYPE_HOLE) 
        return;
    }

    nSurfaceID = m_pTerrain->GetSurfaceTypeID(m_GrassFocusX+x,m_GrassFocusY+y);
    if(nSurfaceID == STYPE_HOLE) return;

    assert(nSurfaceID<MAX_SURFACE_TYPES_COUNT);

    if(!m_arrlstSurfaceObjects[nSurfaceID].Count())
      return;

    assert(nStep == 1);

    CVars * pCVars = GetCVars();

    for(int i=0; i<m_nGrassDensity; i++)
    {
      float objX = (float)rnd() - 0.5f+(m_GrassFocusX+x);
      float objY = (float)rnd() - 0.5f+(m_GrassFocusY+y);

      ///    if(m_pTerrain->IsBurnedOut((int)objX,(int)objY))
      //   continue;

      int nObjIdId = rand() % m_arrlstSurfaceObjects[nSurfaceID].Count();
      GrassType * o = m_arrlstSurfaceObjects[nSurfaceID][nObjIdId];

      if(!o->nIndCount)
        continue;

      float objZ = m_pTerrain->GetZApr(objX,objY);

      float fBr = 
        //pCVars->e_lighting_bit_test ? 
        //m_pTerrain->IsOnTheLight((int)objX,(int)objY) :
        0.6f+0.4f*(float)m_pTerrain->IsOnTheLight((int)objX,(int)objY);

      AddIndexedArray(o, objX,objY,objZ, fBr, 
        0.75f+rnd()*0.5f,  // size, proportions
        (rand() > RAND_MAX/2) ? 1.f : -1.f, // x sign
        (rand() > RAND_MAX/2) ? 1.f : -1.f, // y sign
        (rand() > RAND_MAX/2), // xy swap
        pGrassVertices);
    }
}
// generating sector grass mesh
void CDetailGrass::CreateSectorGrass(const int nRange, const int nStep, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pGrassVertices)
{
  m_GrassVerticesCount = 0;
  m_GrassIndices.Clear();

  if(!m_GrassModelsArray.Count())
    return;

  m_GrassFocusX = m_GrassFocusX/nStep*nStep;
  m_GrassFocusY = m_GrassFocusY/nStep*nStep;

  if(m_GrassFocusX-nRange<0 || m_GrassFocusY-nRange<0)
    return;
  if(m_GrassFocusX+nRange>=CTerrain::GetTerrainSize() || m_GrassFocusY+nRange>=CTerrain::GetTerrainSize())
    return;

  int nRangeX = nRange;
  int nRangeY = nRange;

  int nStepX = nStep;
  int nStepY = nStep;

  Vec3d vCamPos = GetViewCamera().GetPos();  

  if(vCamPos.x > m_GrassFocusX)
  {
    if(vCamPos.y > m_GrassFocusY)
    {
      for(int x=-nRangeX; x<= nRangeX; x+=nStepX)
        for(int y=-nRangeY; y<= nRangeY; y+=nStepY)
          CreateSectorGrassInUnit(x, y , nStep, pGrassVertices);
    }
    else
    {
      for(int x=-nRangeX; x<= nRangeX; x+=nStepX)
        for(int y= nRangeY; y>=-nRangeY; y-=nStepY)
          CreateSectorGrassInUnit(x, y , nStep, pGrassVertices);
    }
  }
  else
  {
    if(vCamPos.y > m_GrassFocusY)
    {
      for(int x= nRangeX; x>=-nRangeX; x-=nStepX)
        for(int y=-nRangeY; y<= nRangeY; y+=nStepY)
          CreateSectorGrassInUnit(x, y , nStep, pGrassVertices);
    }
    else
    {
      for(int x= nRangeX; x>=-nRangeX; x-=nStepX)
        for(int y= nRangeY; y>=-nRangeY; y-=nStepY)
          CreateSectorGrassInUnit(x, y , nStep, pGrassVertices);
    }
  }


  //  if(m_GrassVerticesCount>12000)
  /*  {
  GetLog()->Log("grass_verts = %d/%d", m_GrassVerticesCount, int(DETAIL_GRASS_PIP_BUFFER_SIZE));
  GetLog()->Log("grass_indic = %d", m_GrassIndices.Count());
  }*/
}

void CDetailGrass::PreloadResources()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(m_GrassTID);
	if(pTexPic)
		GetRenderer()->EF_PrecacheResource(pTexPic, 0, 1.f, 0);
}

// check focus and render
void CDetailGrass::RenderDetailGrass(CTerrain * pTerrain)
{
  if( !GetCVars()->e_detail_objects || !m_nGrassDensity || !pTerrain || !m_pLeafBuffer )
    return;

  m_pTerrain = pTerrain;

  if(!m_GrassTID)
    return;

  if(!(GetRenderer()->GetFeatures()&RFT_DIRECTACCESSTOVIDEOMEMORY))
    return;

  Vec3d vCamPos = GetViewCamera().GetPos();  
  const int map_border = 12;
  if(vCamPos.x < map_border || vCamPos.x > CTerrain::GetTerrainSize()-map_border)
    return;
  if(vCamPos.y < map_border || vCamPos.y > CTerrain::GetTerrainSize()-map_border)
    return;

  //  if(!m_pGrassVertices)
  //  return;

  float camera_h = vCamPos.z - m_pTerrain->GetZSafe(vCamPos.x,vCamPos.y);

  if(camera_h > 64 || camera_h < -8)
    return;
  /*
  if(pTerrain->m_nRenderStackLevel==0)
  {
  Matrix44 mat;
  GetRenderer()->GetModelViewMatrix(mat.GetData());
  Vec3d forward = -mat.GetColumn(2);	

  //CELL_CHANGED_BY_IVO
  //forward(-mat.cell(2), -mat.cell(6), -mat.cell(10)); 

  forward.Normalize();

  if(!forward.x && !forward.y)
  return;

  int c_X = int(vCamPos.x + forward.x*CAMERA_GRASS_SHIFT);
  int c_Y = int(vCamPos.y + forward.y*CAMERA_GRASS_SHIFT);

  //  GetRenderer()->DrawBall(c_X,c_Y,m_pTerrain->GetZApr(c_X,c_Y),1);

  // check focus
  float dx = float(c_X-m_GrassFocusX);
  float dy = float(c_Y-m_GrassFocusY);

  float d2 = (dx*dx+dy*dy);

  if(d2>16)
  {
  m_GrassFocusX = c_X;
  m_GrassFocusY = c_Y;

  PrepareBufferCallback(m_pLeafBuffer, 0, 0);
  }                                        

  // set alpha glow texgen params
  const float fDistFadeK = 1.f/(CAMERA_GRASS_SHIFT*4.f);
  m_arrfShaderInfo[2] = fDistFadeK;
  m_arrfShaderInfo[3] = 0.5f-vCamPos.y*fDistFadeK;
  m_arrfShaderInfo[4] = 0.5f-vCamPos.x*fDistFadeK;
  m_pLeafBuffer->SetRECustomData(m_arrfShaderInfo);
  }
  */
  //  if(!m_GrassVerticesCount)
  //  return;

  // find fog volume
  Vec3d vPlus(30,30,30);  
	int nFogId;
  for(nFogId = 0; nFogId<pTerrain->m_lstFogVolumes.Count(); nFogId++)
  {
    if(vCamPos.x > (pTerrain->m_lstFogVolumes[nFogId].vBoxMin-vPlus).x)
      if(vCamPos.y > (pTerrain->m_lstFogVolumes[nFogId].vBoxMin-vPlus).y)
        if(vCamPos.z > (pTerrain->m_lstFogVolumes[nFogId].vBoxMin-vPlus).z)
          if(vCamPos.x < (pTerrain->m_lstFogVolumes[nFogId].vBoxMax+vPlus).x)
            if(vCamPos.y < (pTerrain->m_lstFogVolumes[nFogId].vBoxMax+vPlus).y)
              if(vCamPos.z < (pTerrain->m_lstFogVolumes[nFogId].vBoxMax+vPlus).z)
                break;
  }

  int nDLMask = Get3DEngine()->GetLightMaskFromPosition(vCamPos, CAMERA_GRASS_SHIFT);

  // render
  CCObject * pObj = GetRenderer()->EF_GetObject(true);
  if(GetRenderer()->EF_GetHeatVision())
    pObj->m_ObjFlags |= FOB_HEATVISION;
  pObj->m_Matrix.SetIdentity();

  pObj->m_Color.a = 0.99f;

  m_pLeafBuffer->m_vBoxMin = vCamPos;
  m_pLeafBuffer->m_vBoxMax = vCamPos;

  pObj->m_SortId = -1;

  if(pTerrain->m_nRenderStackLevel==0)
    m_pLeafBuffer->InvalidateVideoBuffer();

  if(m_pLeafBuffer->m_Indices.m_nItems)
    m_pLeafBuffer->AddRenderElements( pObj, nDLMask,-1, (nFogId<pTerrain->m_lstFogVolumes.Count()) ? pTerrain->m_lstFogVolumes[nFogId].nRendererVolumeID : 0, eS_TerrainDetailObjects );
  else
  { // nothing to draw now but we must to add render element to be able to generate mesh right before rendering
    m_pLeafBuffer->AddRenderElements( pObj, nDLMask,-1, 
      (nFogId<pTerrain->m_lstFogVolumes.Count()) ? pTerrain->m_lstFogVolumes[nFogId].nRendererVolumeID : 0 );
  }
}

int CDetailGrass::GetMemoryUsage()
{
  int nSize=0;
  nSize += m_GrassIndices.GetMemoryUsage();
  nSize += m_GrassModelsArray.GetMemoryUsage();
  for(int i=0; i<m_GrassModelsArray.Count(); i++)
    nSize += sizeof(*m_GrassModelsArray[i]) + m_GrassModelsArray[i]->GetMemoryUsage();

  return nSize;
}

