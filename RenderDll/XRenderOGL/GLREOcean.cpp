/*=============================================================================
  GLREOcean.cpp : implementation of the Ocean Rendering.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "I3dengine.h"
#include "../Common/NvTriStrip/NVTriStrip.h"
#include "GLCGVProgram.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//=======================================================================

void CREOcean::mfReset()
{
}

void CREOcean::GenerateIndices(int nLodCode)
{
  SPrimitiveGroup pg;
  TArray<ushort> Indicies;
  int size;

  if (m_OceanIndicies[nLodCode])
    return;
  SOceanIndicies *oi = new SOceanIndicies;
  m_OceanIndicies[nLodCode] = oi;
  if (!(nLodCode & ~LOD_MASK))
  {
    int nL = nLodCode & LOD_MASK;
    pg.offsIndex = 0;
    pg.numIndices = m_pIndices[nL].Num();
    pg.numTris = pg.numIndices-2;
    pg.type = PT_STRIP;
    oi->m_Groups.AddElem(pg);
    oi->m_nInds = pg.numIndices;
    int size = pg.numIndices * sizeof(ushort);
    oi->m_pIndicies = new ushort[size];
    memcpy(oi->m_pIndicies, &m_pIndices[nL][0], size);
    return;
  }
  int nLod = nLodCode & LOD_MASK;
  int nl = 1<<nLod;
  int nGrid = (OCEANGRID+1);
  // set indices
  int iIndex = nGrid*nl+nl;
  int yStep = nGrid * nl;
  for(int a=nl; a<nGrid-1-nl; a+=nl)
  {
    for(int i=nl; i<nGrid-nl; i+=nl, iIndex+=nl)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex + yStep);
    }

    int iNextIndex = (a+nl) * nGrid + nl;

    // connect two strips by inserting two degenerated triangles 
    if(a < nGrid-1-nl*2)
    {
      Indicies.AddElem(iIndex + yStep - nl);
      Indicies.AddElem(iNextIndex);
    }
    iIndex = iNextIndex;
  }
  pg.numIndices = Indicies.Num();
  pg.numTris = pg.numIndices-2;
  pg.type = PT_STRIP;
  pg.offsIndex = 0;
  oi->m_Groups.AddElem(pg);

  // Left
  pg.offsIndex = Indicies.Num();
  iIndex = nGrid*(nGrid-1);
  if (!(nLodCode & (1<<LOD_LEFTSHIFT)))
  {
    Indicies.AddElem(iIndex);
    Indicies.AddElem(iIndex - nGrid * nl + nl);
    Indicies.AddElem(iIndex - nGrid * nl);
    iIndex = iIndex - nGrid * nl + nl;
    yStep = -(nGrid * nl) - nl;
    for(int i=nl; i<nGrid-nl; i+=nl, iIndex-=nGrid*nl)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex + yStep);
    }
  }
  else
  {
    for(int i=0; i<nGrid-nl; i+=nl*2)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex - (nGrid * nl) + nl);
      Indicies.AddElem(iIndex - nGrid * nl * 2);
      if (i < nGrid-nl-nl*2)
        Indicies.AddElem(iIndex - nGrid * nl * 2 + nl);
      iIndex -= nGrid * nl * 2;
    }
  }
  pg.numIndices = Indicies.Num()-pg.offsIndex;
  pg.numTris = pg.numIndices-2;
  oi->m_Groups.AddElem(pg);

  // Bottom
  pg.offsIndex = Indicies.Num();
  iIndex = 0;
  if (!(nLodCode & (1<<LOD_BOTTOMSHIFT)))
  {
    Indicies.AddElem(iIndex);
    Indicies.AddElem(iIndex + nGrid*nl + nl);
    Indicies.AddElem(iIndex + nl);
    iIndex = iIndex + nGrid*nl + nl;
    yStep = -(nGrid * nl) + nl;
    for(int i=nl; i<nGrid-nl; i+=nl, iIndex+=nl)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex + yStep);
    }
  }
  else
  {
    for(int i=0; i<nGrid-nl; i+=nl*2)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex + (nGrid * nl) + nl);
      Indicies.AddElem(iIndex + nl * 2);
      if (i < nGrid-nl-nl*2)
        Indicies.AddElem(iIndex + nGrid * nl + nl * 2);
      iIndex += nl * 2;
    }
  }
  pg.numIndices = Indicies.Num()-pg.offsIndex;
  pg.numTris = pg.numIndices-2;
  oi->m_Groups.AddElem(pg);

  // Right
  pg.offsIndex = Indicies.Num();
  iIndex = nGrid-1;
  if (!(nLodCode & (1<<LOD_RIGHTSHIFT)))
  {
    Indicies.AddElem(iIndex);
    Indicies.AddElem(iIndex + nGrid * nl - nl);
    Indicies.AddElem(iIndex + nGrid * nl);
    iIndex = iIndex + nGrid * nl - nl;
    yStep = nGrid * nl + nl;
    for(int i=nl; i<nGrid-nl; i+=nl, iIndex+=nGrid*nl)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex + yStep);
    }
  }
  else
  {
    for(int i=0; i<nGrid-nl; i+=nl*2)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex + (nGrid * nl) - nl);
      Indicies.AddElem(iIndex + nGrid * nl * 2);
      if (i < nGrid-nl-nl*2)
        Indicies.AddElem(iIndex + nGrid * nl * 2 - nl);
      iIndex += nGrid * nl * 2;
    }
  }
  pg.numIndices = Indicies.Num()-pg.offsIndex;
  pg.numTris = pg.numIndices-2;
  oi->m_Groups.AddElem(pg);

  // Top
  pg.offsIndex = Indicies.Num();
  iIndex = nGrid*(nGrid-1)+nGrid-1;
  if (!(nLodCode & (1<<LOD_TOPSHIFT)))
  {
    Indicies.AddElem(iIndex);
    Indicies.AddElem(iIndex - nGrid*nl - nl);
    Indicies.AddElem(iIndex - nl);
    iIndex = iIndex - nGrid*nl - nl;
    yStep = nGrid * nl - nl;
    for(int i=nl; i<nGrid-nl; i+=nl, iIndex-=nl)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex + yStep);
    }
  }
  else
  {
    for(int i=0; i<nGrid-nl; i+=nl*2)
    {
      Indicies.AddElem(iIndex);
      Indicies.AddElem(iIndex - (nGrid * nl) - nl);
      Indicies.AddElem(iIndex - nl * 2);
      if (i < nGrid-nl-nl*2)
        Indicies.AddElem(iIndex - nGrid * nl - nl * 2);
      iIndex -= nl * 2;
    }
  }
  pg.numIndices = Indicies.Num()-pg.offsIndex;
  pg.numTris = pg.numIndices-2;
  oi->m_Groups.AddElem(pg);

  size = Indicies.Num()*sizeof(ushort); 
  oi->m_pIndicies = new ushort[size];
  oi->m_nInds = Indicies.Num();
  cryMemcpy(oi->m_pIndicies, &Indicies[0], size);
}


void CREOcean::UpdateTexture()
{
  if (m_CustomTexBind[0]>0 && !CRenderer::CV_r_oceantexupdate)
    return;

  CGLRenderer *r = gcpOGL;
  float data[OCEANGRID][OCEANGRID][2];
  double time0 = 0;
  ticks(time0);
  if (m_CustomTexBind[0] <= 0)
  {
    uint tnum = 0;
    glGenTextures(1, &tnum);	
    assert(tnum<14000);
    m_CustomTexBind[0] = tnum;
    r->SetTexture(tnum, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DSDT_NV, OCEANGRID, OCEANGRID, 0, GL_DSDT_NV, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    r->SetTexture(0, eTT_Base);
  }
  for(unsigned int y=0; y<OCEANGRID; y++)
  {
    for( unsigned int x=0; x<OCEANGRID; x++)
    {
      data[y][x][0] = m_Normals[y][x].x;
      data[y][x][1] = m_Normals[y][x].y;
      //data[y][x][2] = (byte)QRound(m_Normals[y][x].z * 127.0f);
    }
  }
  r->SetTexture(m_CustomTexBind[0], eTT_Base);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, OCEANGRID, OCEANGRID, GL_DSDT_NV, GL_FLOAT, &data[0][0][0]);
  r->SetTexture(0, eTT_Base);

  unticks(time0);
  m_RS.m_StatsTimeTexUpdate = (float)(time0*1000.0*g_SecondsPerCycle);
}

void CREOcean::DrawOceanSector(SOceanIndicies *oi)
{
  for (int i=0; i<oi->m_Groups.Num(); i++)
  {
    SPrimitiveGroup *g = &oi->m_Groups[i];
    switch (g->type)
    {
      case PT_STRIP:
        glDrawElements(GL_TRIANGLE_STRIP, g->numIndices, GL_UNSIGNED_SHORT, &oi->m_pIndicies[g->offsIndex]);
        break;

      case PT_LIST:
        glDrawElements(GL_TRIANGLES, g->numIndices, GL_UNSIGNED_SHORT, &oi->m_pIndicies[g->offsIndex]);
        break;

      case PT_FAN:
        glDrawElements(GL_TRIANGLE_FAN, g->numIndices, GL_UNSIGNED_SHORT, &oi->m_pIndicies[g->offsIndex]);
        break;
    }
    gRenDev->m_nPolygons += g->numTris;
  }
}

static _inline int Compare(SOceanSector *& p1, SOceanSector *& p2)
{
  if(p1->m_Flags > p2->m_Flags)
    return 1;
  else
  if(p1->m_Flags < p2->m_Flags)
    return -1;
  
  return 0;
}

static _inline float sCalcSplash(SSplash *spl, float fX, float fY)
{
  CGLRenderer *r = gcpOGL;

  float fDeltaTime = r->m_RP.m_RealTime - spl->m_fStartTime;
  float fScaleFactor = 1.0f / (r->m_RP.m_RealTime - spl->m_fLastTime + 1.0f);

  float vDelt[2];

  // Calculate 2D distance
  vDelt[0] = spl->m_Pos[0] - fX; vDelt[1] = spl->m_Pos[1] - fY;
  float fSqDist = vDelt[0]*vDelt[0] + vDelt[1]*vDelt[1];

  // Inverse square root
  unsigned int *n1 = (unsigned int *)&fSqDist;
  unsigned int nn = 0x5f3759df - (*n1 >> 1);
  float *n2 = (float *)&nn;
  float fDistSplash = 1.0f / ((1.5f - (fSqDist * 0.5f) * *n2 * *n2) * *n2);

  // Emulate sin waves
  float fDistFactor = fDeltaTime*10.0f - fDistSplash + 4.0f;
  fDistFactor = CLAMP(fDistFactor, 0.0f, 1.0f);
  float fRad = (fDistSplash - fDeltaTime*10) * 0.4f / 3.1416f * 1024.0f;
  float fSin = gRenDev->m_RP.m_tSinTable[QRound(fRad)&0x3ff] * fDistFactor;

  return fSin * fScaleFactor * spl->m_fForce;
}

float *CREOcean::mfFillAdditionalBuffer(SOceanSector *os, int nSplashes, SSplash *pSplashes[], int& nCurSize, int nLod, float fSize)
{
  CGLRenderer *r = gcpOGL;
  int nSize = sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * r->m_RP.m_MaxVerts / sizeof(float);

  float *pHM, *pTZ;

  //int nFloats = (os->m_Flags & OSF_NEEDHEIGHTS) ? 1 : 0;
  //if (nSplashes)
  //  nFloats++;
  int nFloats = 2;

  if (gcpOGL->m_RP.m_NumFences)
  {
    if (nCurSize+(OCEANGRID+1)*(OCEANGRID+1)*nFloats >= nSize)
    {
      nCurSize = 0;
      //r->EF_SetFence(true);
    }
    pTZ = &r->m_RP.m_Ptr.VBPtr_0->x+nCurSize;
    nCurSize += (OCEANGRID+1)*(OCEANGRID+1)*nFloats;
  }
  else
    pTZ = &gcpOGL->m_RP.m_Ptr.VBPtr_0->x;
  pHM = pTZ;
  int nStep = 1<<nLod;
  if (!nSplashes)
  {
    int nIncr = nStep*nFloats;
    for (int ty=0; ty<OCEANGRID+1; ty+=nStep)
    {
      pTZ = &pHM[ty*(OCEANGRID+1)*nFloats];
      for (int tx=0; tx<OCEANGRID+1; tx+=nStep)
      {
        float fX = m_Pos[ty][tx][0]*fSize+os->x;
        float fY = m_Pos[ty][tx][1]*fSize+os->y;
        float fZ = GetHMap(fX, fY);
        pTZ[0] = fZ;
        pTZ += nIncr;
      }
    }
  }
  else
  if (!(os->m_Flags & OSF_NEEDHEIGHTS))
  {
    for (int ty=0; ty<OCEANGRID+1; ty+=nStep)
    {
      pTZ = &pHM[ty*(OCEANGRID+1)*nFloats];
      for (int tx=0; tx<OCEANGRID+1; tx+=nStep)
      {
        float fX = m_Pos[ty][tx][0]*fSize+os->x;
        float fY = m_Pos[ty][tx][1]*fSize+os->y;
        float fSplash = 0;
        for (int n=0; n<nSplashes; n++)
        {
          SSplash *spl = pSplashes[n];
          fSplash += sCalcSplash(spl, fX, fY);
        }
        pTZ[1] = fSplash;
        pTZ += nStep*2;
      }
    }
  }
  else
  {
    for (int ty=0; ty<OCEANGRID+1; ty+=nStep)
    {
      pTZ = &pHM[ty*(OCEANGRID+1)*2];
      for (int tx=0; tx<OCEANGRID+1; tx+=nStep)
      {
        float fX = m_Pos[ty][tx][0]*fSize+os->x;
        float fY = m_Pos[ty][tx][1]*fSize+os->y;
        float fZ = GetHMap(fX, fY);
        pTZ[0] = fZ;
        float fSplash = 0;
        for (int n=0; n<nSplashes; n++)
        {
          SSplash *spl = pSplashes[n];
          fSplash += sCalcSplash(spl, fX, fY);
        }
        pTZ[1] = fSplash; // / (float)nSplashes;
        pTZ += nStep*2;
      }
    }
  }
  return pHM;
}

void CREOcean::mfDrawOceanSectors()
{ 
  float x, y;
  CGLRenderer *r = gcpOGL;
  CVertexBuffer *vb = m_pBuffer;
  int i;

  m_RS.m_StatsNumRendOceanSectors = 0;

  ushort *saveInds = r->m_RP.m_RendIndices;
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMaxDist = eng->GetMaxViewDistance() / 1.0f;
  CCamera cam = r->GetCamera();
  Vec3d mins;
  Vec3d maxs;
  Vec3d cameraPos = cam.GetPos();
  float fCurX = (float)((int)cameraPos[0] & ~255) + 128.0f;
  float fCurY = (float)((int)cameraPos[1] & ~255) + 128.0f;

  float fSize = (float)CRenderer::CV_r_oceansectorsize;
  float fHeightScale = (float)CRenderer::CV_r_oceanheightscale;
  if (fSize != m_fSectorSize)
  {
    m_fSectorSize = fSize;
    for (int i=0; i<256; i++)
    {
      m_OceanSectorsHash[i].Free();
    }
  }
  float fWaterLevel = eng->GetWaterLevel();
  CVProgram *vp = NULL;
  int nMaxSplashes = CLAMP(CRenderer::CV_r_oceanmaxsplashes, 0, 16);

  //x = y = 0;
  //x = fCurX;
  //y = fCurY;
  m_VisOceanSectors.SetUse(0);
  int osNear = -1;
  int nMinLod = 65536;
  for (y=fCurY-fMaxDist; y<fCurY+fMaxDist; y+=fSize)
  {
    for (x=fCurX-fMaxDist; x<fCurX+fMaxDist; x+=fSize)
    {
      SOceanSector *os = GetSectorByPos(x, y);
      if (!(os->m_Flags & (OSF_FIRSTTIME | OSF_VISIBLE)))
        continue;
      mins.x = x+m_MinBound.x*fSize;
      mins.y = y+m_MinBound.y*fSize;
      mins.z = fWaterLevel+m_MinBound.z*fHeightScale;
      maxs.x = x+fSize+m_MaxBound.x*fSize;
      maxs.y = y+fSize+m_MaxBound.y*fSize;
      maxs.z = fWaterLevel+m_MaxBound.z*fHeightScale;
      int cull = cam.IsAABBVisible_hierarchical( AABB(mins,maxs) );
      if (cull != CULL_EXCLUSION)
      {
        Vec3d vCenter = (mins + maxs) * 0.5f;
        os->nLod = GetLOD(cameraPos, vCenter);
        if (os->nLod < nMinLod)
        {
          nMinLod = os->nLod;
          osNear = m_VisOceanSectors.Num();
        }
        os->m_Frame = gRenDev->m_cEF.m_Frame;
        os->m_Flags &= ~OSF_LODUPDATED;
        m_VisOceanSectors.AddElem(os);
      }
    }
  }
  if (m_VisOceanSectors.Num())
  {
    LinkVisSectors(fSize);
    ::Sort(&m_VisOceanSectors[0], m_VisOceanSectors.Num());
  }
  bool bCurNeedBuffer = false;
  int nSize = sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * r->m_RP.m_MaxVerts / sizeof(float);
  int nCurSize = 0;
  float *pHM, *pTZ;
  int nIndVP = CRenderer::CV_r_waterreflections ? 0 : 1;
  for (i=0; i<m_VisOceanSectors.Num(); i++)
  {
    SOceanSector *os = m_VisOceanSectors[i];
    //if ((int)os != 0xf3888a0)
    //  continue;
    int nLod = os->nLod;
    bool bL = (nLod < GetSectorByPos(os->x-fSize, os->y)->nLod);
    bool bR = (nLod < GetSectorByPos(os->x+fSize, os->y)->nLod);
    bool bT = (nLod < GetSectorByPos(os->x, os->y+fSize)->nLod);
    bool bB = (nLod < GetSectorByPos(os->x, os->y-fSize)->nLod);
    int nLodCode = nLod + (bL<<LOD_LEFTSHIFT) + (bR<<LOD_RIGHTSHIFT) + (bT<<LOD_TOPSHIFT) + (bB<<LOD_BOTTOMSHIFT);
    if (!m_OceanIndicies[nLodCode])
      GenerateIndices(nLodCode);
    int nSplashes = 0;
    SSplash *pSplashes[16];
    int nS;
    for (nS=0; nS<r->m_RP.m_Splashes.Num(); nS++)
    {
      SSplash *spl = &r->m_RP.m_Splashes[nS];
      float fCurRadius = spl->m_fCurRadius;
      if (spl->m_Pos[0]-fCurRadius > os->x+fSize ||
        spl->m_Pos[1]-fCurRadius > os->y+fSize ||
        spl->m_Pos[0]+fCurRadius < os->x ||
        spl->m_Pos[1]+fCurRadius < os->y)
        continue;
      pSplashes[nSplashes++] = spl;
      if (nSplashes == nMaxSplashes)
        break;
    }
    if (os->m_Flags & OSF_FIRSTTIME)
    {
      if (gcpOGL->m_RP.m_NumFences)
      {
        if (nCurSize+(OCEANGRID+1)*(OCEANGRID+1) >= nSize)
        {
          nCurSize = 0;
          //r->EF_SetFence(true);
        }
        pTZ = &r->m_RP.m_Ptr.VBPtr_0->x+nCurSize;
        nCurSize += (OCEANGRID+1)*(OCEANGRID+1);
      }
      else
        pTZ = &gcpOGL->m_RP.m_Ptr.VBPtr_0->x;
      pHM = pTZ;

      float fMinLevel = 99999.0f;
      bool bBlend = false;
      bool bNeedHeights = false;
      for (uint ty=0; ty<OCEANGRID+1; ty++)
      {
        for (uint tx=0; tx<OCEANGRID+1; tx++)
        {
          float fX = m_Pos[ty][tx][0]*fSize+os->x;
          float fY = m_Pos[ty][tx][1]*fSize+os->y;
          float fZ = GetHMap(fX, fY);
          pTZ[0] = fZ;
          pTZ += 2;
          fMinLevel = min(fMinLevel, fZ);
          if (!bBlend && fWaterLevel-fZ <= 1.0f)
            bBlend = true;
          if (fWaterLevel-fZ < 16.0f)
            bNeedHeights = true;
        }
      }
      os->m_Flags &= ~OSF_FIRSTTIME;
      if (fMinLevel <= fWaterLevel)
        os->m_Flags |= OSF_VISIBLE;
      //bNeedHeights = bBlend = false;
      if (bNeedHeights)
        os->m_Flags |= OSF_NEEDHEIGHTS;
      if (bBlend)
      {
        os->m_Flags |= OSF_BLEND;
        os->RenderState = GS_DEPTHWRITE | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
      }
      else
        os->RenderState = GS_DEPTHWRITE;
    }
    else
    if ((os->m_Flags & OSF_NEEDHEIGHTS) || nSplashes)
      pHM = mfFillAdditionalBuffer(os, nSplashes, pSplashes, nCurSize, nLod, fSize);
    m_RS.m_StatsNumRendOceanSectors++;

    r->EF_SetState(os->RenderState);
    int nVP = 0;
    if (os->m_Flags & OSF_NEEDHEIGHTS)
      nVP += OVP_HEIGHT;
    if (nSplashes)
      nVP++;

    CVProgram *curVP = m_VPs[nVP];
    if (!curVP)
      continue;
    if (vp != curVP)
    {
      vp = curVP;
      vp->mfSet(true, NULL, 0);
    }
    int nFloats = (os->m_Flags & OSF_NEEDHEIGHTS) ? 1 : 0;
    if (nSplashes)
      nFloats++;
    if (nFloats)
    {
      if (glVertexAttribPointerNV)
        glVertexAttribPointerNV(1, 2, GL_FLOAT, sizeof(float)*2, pHM);
      if (!bCurNeedBuffer)
      {
        glEnableClientState(GL_VERTEX_ATTRIB_ARRAY1_NV);
        bCurNeedBuffer = true;
      }
    }
    else
    if (bCurNeedBuffer)
    {
      glDisableClientState(GL_VERTEX_ATTRIB_ARRAY1_NV);
      bCurNeedBuffer = false;
    }
    float param[4];
    param[0] = os->x;
    param[1] = os->y;
    param[2] = 0;
    param[3] = 0;
    CCGVProgram_GL *vpGL = (CCGVProgram_GL *)vp;
    SCGBind *pBind = vpGL->mfGetParameterBind("PosOffset");
    if (pBind)
      vpGL->mfParameter4f(pBind, param);
    pBind = vpGL->mfGetParameterBind("PosScale");
    param[0] = fSize;
    param[1] = fSize;
    param[2] = fHeightScale;
    param[3] = 1;
    if (pBind)
      vpGL->mfParameter4f(pBind, param);

    /*if (nSplashes)
    {
      static Vec3d sPos;
      static float sTime;
      sPos = Vec3d(306, 1942, 0);
      if ((GetAsyncKeyState(VK_NUMPAD1) & 0x8000))
      sTime = r->m_RP.m_RealTime;
      float fForce = 1.5f;
      cgBindIter *pBind = vpGL->mfGetParameterBind("Splash1");

      if (pBind)
      {
        param[0] = sPos[0];
        param[1] = sPos[1];
        param[2] = sPos[2];
        param[3] = 0;
        vpGL->mfParameter4f(pBind, param);
      }
      pBind = vpGL->mfGetParameterBind("Time");
      if (pBind)
      {
        float fDeltaTime = r->m_RP.m_RealTime - sTime;
        float fScaleFactor = 1.0f / (r->m_RP.m_RealTime - sTime + 1.0f);
        param[0] = fForce * fScaleFactor;
        param[1] = 0;
        param[2] = -fDeltaTime*10;
        param[3] = fDeltaTime*10.0f;
        vpGL->mfParameter4f(pBind, param);
      }
    }*/

    DrawOceanSector(m_OceanIndicies[nLodCode]);
    //if (bCurNeedBuffer && glSetFenceNV)
    //  glSetFenceNV(r->m_RP.mBufs[r->m_RP.m_CurFence].mFence, GL_ALL_COMPLETED_NV);
  }
  if (bCurNeedBuffer)
    glDisableClientState(GL_VERTEX_ATTRIB_ARRAY1_NV);

  if (glSetFenceNV)
    glSetFenceNV(m_pBuffer->m_fence, GL_ALL_COMPLETED_NV);
  m_pBuffer->m_bFenceSet = true;
}

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]
static void matmul4( GLfloat *product, const GLfloat *a, const GLfloat *b )
{
  int i;
  for (i=0; i<4; i++)
  {
    float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
  }
}
#undef A
#undef B
#undef P

static void transform_point(float out[4], const float m[16], const float in[4])
{
#define M(row,col)  m[col*4+row]
  out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

void CREOcean::mfDrawOceanScreenLod()
{ 
  CGLRenderer *r = gcpOGL;
  int nScreenY = r->GetHeight();
  int nScreenX = r->GetWidth();

	if(!nScreenY || !nScreenX)
		return;

  float ProjectionMatrix[16];
  float ModelMatrix[16];
  int   Viewport[4];
  glGetFloatv(GL_MODELVIEW_MATRIX,   ModelMatrix);
  glGetFloatv(GL_PROJECTION_MATRIX,  ProjectionMatrix);
  glGetIntegerv(GL_VIEWPORT,         Viewport);
  float m[16], A[16];
  matmul4(A, ProjectionMatrix, ModelMatrix);
  QQinvertMatrixf(m, A);


  m_DWQVertices.Free();
  m_DWQIndices.Free();

  int fParts = 25;

  int nScreenXP = nScreenX/fParts;
  int nScreenYP = nScreenY/fParts;

	if(!nScreenYP || !nScreenXP)
		return;

  unsigned short nIdx = 0;
  int y_size = int(float(nScreenY)/(nScreenYP)+1.f);

  const Vec3d vCamPos = r->GetCamera().GetPos();

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fWaterLevel = eng->GetWaterLevel();

  int nSize = 256;
  float fSize = (float)nSize;
  float fiSize = 1.0f / fSize;

  bool bWaterVisible = false;

  float *pBuf, *pBuf1;
  //if (r->m_RP.m_NumFences)
    //r->EF_SetFence(true);
  pBuf = &r->m_RP.m_Ptr.VBPtr_0->x;
  pBuf1 = pBuf;

  float fScale = fSize / OCEANGRID;
  float fiScale = 1.0f / fScale;

  for(int x=0; x<=nScreenX/(nScreenXP)*(nScreenXP); x+=int(nScreenXP))
  {
    for(int y=0; y<=nScreenY/(nScreenYP)*(nScreenYP); y+=int(nScreenYP))
    {
      Vec3d n, p;

      n((float)x, (float)y, 0.0f);

      float in[4], out[4];
      /* transformation coordonnees normalisees entre -1 et 1 */
      in[0] = (n.x - Viewport[0]) * 2 / Viewport[2] - 1.0f;
      in[1] = (n.y - Viewport[1]) * 2 / Viewport[3] - 1.0f;
      in[2] = 2.0f * n.z - 1.0f;
      in[3] = 1.0;

      transform_point(out, m, in);
      if (out[3] == 0.0f)
        continue;
      p.x = out[0] / out[3];
      p.y = out[1] / out[3];
      p.z = out[2] / out[3];
      //r->UnProjectFromScreen(n.x, n.y, n.z, &p.x, &p.y, &p.z);

      float z1 = vCamPos.z - p.z;
      float t;
      if(z1<0.0001f)
        z1=0.0001f;

      {
        float z2 = vCamPos.z - fWaterLevel;
        t = z2/z1;
        if( t > 300*(vCamPos.z - fWaterLevel) )
          t = 300*(vCamPos.z - fWaterLevel);
        p = p - vCamPos;
        p = vCamPos + p*t;
      }

      float fX = p.x * fiScale;
      float fY = p.y * fiScale;
      long nX = QInt(fX);
      long nY = QInt(fY);
      register float dx = fX - (float)nX;
      register float dy = fY - (float)nY;
      register float dix = 1.0f - dx;
      register float diy = 1.0f - dy;
      nX &= OCEANGRID-1;
      nY &= OCEANGRID-1;
      fX = (float)QInt(p.x * fiSize);
      fY = (float)QInt(p.y * fiSize);
      long nX1 = (nX+1);// & (OCEANGRID-1);
      long nY1 = (nY+1);// & (OCEANGRID-1);
      
      register float d0, d1;
      Vec3d Pos, Nor;
      d0 = dix * m_Pos[nY][nX][0]  + dx * m_Pos[nY][nX1][0];
      d1 = dix * m_Pos[nY1][nX][0] + dx * m_Pos[nY1][nX1][0];
      Pos.x = (diy * d0 + dy * d1) + fX;

      d0 = dix * m_Pos[nY][nX][1]  + dx * m_Pos[nY][nX1][1];
      d1 = dix * m_Pos[nY1][nX][1] + dx * m_Pos[nY1][nX1][1];
      Pos.y = (diy * d0 + dy * d1) + fY;

      d0 = dix * m_HX[nY][nX]  + dx * m_HX[nY][nX1];
      d1 = dix * m_HX[nY1][nX] + dx * m_HX[nY1][nX1];
      Pos.z = -(diy * d0 + dy * d1);

      d0 = dix * m_Normals[nY][nX].x  + dx * m_Normals[nY][nX1].x;
      d1 = dix * m_Normals[nY1][nX].x + dx * m_Normals[nY1][nX1].x;
      Nor.x = diy * d0 + dy * d1;

      d0 = dix * m_Normals[nY][nX].y  + dx * m_Normals[nY][nX1].y;
      d1 = dix * m_Normals[nY1][nX].y + dx * m_Normals[nY1][nX1].y;
      Nor.y = diy * d0 + dy * d1;

      d0 = dix * m_Normals[nY][nX].z  + dx * m_Normals[nY][nX1].z;
      d1 = dix * m_Normals[nY1][nX].z + dx * m_Normals[nY1][nX1].z;
      Nor.z = diy * d0 + dy * d1;

      pBuf1[0] = Pos.x;
      pBuf1[1] = Pos.y;
      pBuf1[2] = Pos.z;

      pBuf1[3] = Nor.x;
      pBuf1[4] = Nor.y;
      pBuf1[5] = Nor.z;

      float fZ = GetHMap(Pos.x * fSize, Pos.y * fSize);
      pBuf1[6] = fZ;

      if(fZ <= fWaterLevel)
        bWaterVisible = true;

      pBuf1 += 7;
  
      if(x<nScreenX/(nScreenXP)*(nScreenXP) && y<nScreenY/(nScreenYP)*(nScreenYP) && fabs(p.z-fWaterLevel)<1)
      {
        unsigned short nIdx2 = nIdx+y_size;

        unsigned short _nIdx = nIdx+1;
        unsigned short _nIdx2 = (nIdx+y_size)+1;

        m_DWQIndices.Add(_nIdx);
        m_DWQIndices.Add(nIdx);
        m_DWQIndices.Add(nIdx2);

        m_DWQIndices.Add(_nIdx);
        m_DWQIndices.Add(nIdx2);
        m_DWQIndices.Add(_nIdx2);
      }

      nIdx++;
    }
  }

  r->EF_SetState(GS_DEPTHWRITE | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);

  glVertexAttribPointerNV(0, 3, GL_FLOAT, sizeof(float)*7, pBuf);
  glEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);
  glVertexAttribPointerNV(2, 3, GL_FLOAT, sizeof(float)*7, pBuf+3);
  glEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);
  glVertexAttribPointerNV(1, 1, GL_FLOAT, sizeof(float)*7, pBuf+6);
  glEnableClientState(GL_VERTEX_ATTRIB_ARRAY1_NV);

  CVProgram *vp = m_VPQ;
  if (!vp)
    return;
  if (vp)
    vp->mfSet(true, false);

  glDrawElements(GL_TRIANGLES, m_DWQIndices.Num(), GL_UNSIGNED_SHORT, &m_DWQIndices[0]);

  if (vp)
    vp->mfSet(false, false);

  glDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);
  glDisableClientState(GL_VERTEX_ATTRIB_ARRAY1_NV);
  glDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);
}

bool CREOcean::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  double time0 = 0;
  ticks(time0);

  if (CRenderer::CV_r_oceanrendtype == 0)
    mfDrawOceanSectors();
  else
    mfDrawOceanScreenLod();
  unticks(time0);
  m_RS.m_StatsTimeRendOcean = (float)(time0*1000.0*g_SecondsPerCycle);

//  gRenDev->PostLoad();

  return true;
}


bool CREOcean::mfPreDraw(SShaderPass *sl)
{
  return true;
}


char BoxSides[0x40*8] = {
	0,0,0,0, 0,0,0,0, //00
		0,4,6,2, 0,0,0,4, //01
		7,5,1,3, 0,0,0,4, //02
		0,0,0,0, 0,0,0,0, //03
		0,1,5,4, 0,0,0,4, //04
		0,1,5,4, 6,2,0,6, //05
		7,5,4,0, 1,3,0,6, //06
		0,0,0,0, 0,0,0,0, //07
		7,3,2,6, 0,0,0,4, //08
		0,4,6,7, 3,2,0,6, //09
		7,5,1,3, 2,6,0,6, //0a
		0,0,0,0, 0,0,0,0, //0b
		0,0,0,0, 0,0,0,0, //0c
		0,0,0,0, 0,0,0,0, //0d
		0,0,0,0, 0,0,0,0, //0e
		0,0,0,0, 0,0,0,0, //0f
		0,2,3,1, 0,0,0,4, //10
		0,4,6,2, 3,1,0,6, //11
		7,5,1,0, 2,3,0,6, //12
		0,0,0,0, 0,0,0,0, //13
		0,2,3,1, 5,4,0,6, //14
		1,5,4,6, 2,3,0,6, //15
		7,5,4,0, 2,3,0,6, //16
		0,0,0,0, 0,0,0,0, //17
		0,2,6,7, 3,1,0,6, //18
		0,4,6,7, 3,1,0,6, //19
		7,5,1,0, 2,6,0,6, //1a
		0,0,0,0, 0,0,0,0, //1b
		0,0,0,0, 0,0,0,0, //1c
		0,0,0,0, 0,0,0,0, //1d
		0,0,0,0, 0,0,0,0, //1e
		0,0,0,0, 0,0,0,0, //1f
		7,6,4,5, 0,0,0,4, //20
		0,4,5,7, 6,2,0,6, //21
		7,6,4,5, 1,3,0,6, //22
		0,0,0,0, 0,0,0,0, //23
		7,6,4,0, 1,5,0,6, //24
		0,1,5,7, 6,2,0,6, //25
		7,6,4,0, 1,3,0,6, //26
		0,0,0,0, 0,0,0,0, //27
		7,3,2,6, 4,5,0,6, //28
		0,4,5,7, 3,2,0,6, //29
		6,4,5,1, 3,2,0,6, //2a
		0,0,0,0, 0,0,0,0, //2b
		0,0,0,0, 0,0,0,0, //2c
		0,0,0,0, 0,0,0,0, //2d
		0,0,0,0, 0,0,0,0, //2e
		0,0,0,0, 0,0,0,0, //2f
		0,0,0,0, 0,0,0,0, //30
		0,0,0,0, 0,0,0,0, //31
		0,0,0,0, 0,0,0,0, //32
		0,0,0,0, 0,0,0,0, //33
		0,0,0,0, 0,0,0,0, //34
		0,0,0,0, 0,0,0,0, //35
		0,0,0,0, 0,0,0,0, //36
		0,0,0,0, 0,0,0,0, //37
		0,0,0,0, 0,0,0,0, //38
		0,0,0,0, 0,0,0,0, //39
		0,0,0,0, 0,0,0,0, //3a
		0,0,0,0, 0,0,0,0, //3b
		0,0,0,0, 0,0,0,0, //3c
		0,0,0,0, 0,0,0,0, //3d
		0,0,0,0, 0,0,0,0, //3e
		0,0,0,0, 0,0,0,0, //3f
};
