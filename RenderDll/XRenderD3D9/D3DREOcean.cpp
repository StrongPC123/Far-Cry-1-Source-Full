/*=============================================================================
  D3DREOcean.cpp : implementation of the Ocean Rendering.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "D3DCGVProgram.h"
#include "D3DCGPShader.h"
#include "I3dengine.h"
#include "../Common/NvTriStrip/NVTriStrip.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//=======================================================================

void CREOcean::GenerateIndices(int nLodCode)
{
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;
  SPrimitiveGroup pg;
  IDirect3DIndexBuffer9* ibuf;
  int size;
  int flags = D3DUSAGE_WRITEONLY;
  D3DPOOL Pool = D3DPOOL_MANAGED;
  ushort *dst;
  TArray<ushort> Indicies;

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
    size = pg.numIndices*sizeof(ushort); 
    h = dv->CreateIndexBuffer(size, flags, D3DFMT_INDEX16, Pool, (IDirect3DIndexBuffer9**)&oi->m_pIndicies, NULL);
    ibuf = (IDirect3DIndexBuffer9*)oi->m_pIndicies;
    oi->m_nInds = pg.numIndices;
    h = ibuf->Lock(0, 0, (void **) &dst, 0);
    cryMemcpy(dst, &m_pIndices[nL][0], pg.numIndices*sizeof(ushort));
    h = ibuf->Unlock();
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
  h = dv->CreateIndexBuffer(size, flags, D3DFMT_INDEX16, Pool, (IDirect3DIndexBuffer9**)&oi->m_pIndicies, NULL);
  ibuf = (IDirect3DIndexBuffer9*)oi->m_pIndicies;
  oi->m_nInds = Indicies.Num();
  h = ibuf->Lock(0, 0, (void **) &dst, 0);
  cryMemcpy(dst, &Indicies[0], size);
  h = ibuf->Unlock();
}


void CREOcean::UpdateTexture()
{
  if (m_CustomTexBind[0]>0 && !CRenderer::CV_r_oceantexupdate)
    return;
  HRESULT h;

  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  byte data[OCEANGRID][OCEANGRID][4];
  double time0 = 0;
  ticks(time0);
  for(unsigned int y=0; y<OCEANGRID; y++)
  {
    for( unsigned int x=0; x<OCEANGRID; x++)
    {
      data[y][x][0] = (byte)(m_Normals[y][x].x * 127.0f);
      data[y][x][1] = (byte)(m_Normals[y][x].y * 127.0f);
      data[y][x][2] = (byte)(m_Normals[y][x].z * 127.0f);
      data[y][x][3] = 0;
    }
  }
  if (m_CustomTexBind[0] <= 0)
  {
    char name[128];
    sprintf(name, "$AutoOcean_%d", r->m_TexGenID++);
    STexPic *tp = r->m_TexMan->CreateTexture(name, OCEANGRID, OCEANGRID, 1, FT_NOMIPS | FT_NOSTREAM, FT2_NODXT, &data[0][0][0], eTT_DSDTBump, -1.0f, -1.0f, 0, NULL, 0, eTF_0888);
    m_CustomTexBind[0] = tp->m_Bind;
  }
  else
  {
    STexPicD3D *tp = (STexPicD3D *)gRenDev->m_TexMan->m_Textures[m_CustomTexBind[0]-TX_FIRSTBIND];
    IDirect3DTexture9 *pID3DTexture = (IDirect3DTexture9*)tp->m_RefTex.m_VidTex;
    D3DLOCKED_RECT d3dlr;
    h = pID3DTexture->LockRect(0, &d3dlr, NULL, 0);
    D3DSURFACE_DESC ddsdDescDest;
    pID3DTexture->GetLevelDesc(0, &ddsdDescDest);
    if (d3dlr.Pitch == tp->m_Width*4)
      cryMemcpy(d3dlr.pBits, data, tp->m_Width*tp->m_Height*4);
    else
    {
      switch(ddsdDescDest.Format)
      {
        case D3DFMT_X8L8V8U8:
          {
            byte *pDst = (byte *)d3dlr.pBits;
            byte *pSrc = &data[0][0][0];
            for (int i=0; i<tp->m_Height; i++)
            {
              cryMemcpy(pDst, pSrc, tp->m_Width*4);
              pDst += d3dlr.Pitch;
              pSrc += tp->m_Width*4;
            }
          }
      	  break;
        case D3DFMT_L6V5U5:
      	  break;
        case D3DFMT_V8U8:
          {
            byte *pDst = (byte *)d3dlr.pBits;
            byte *pSrc = &data[0][0][0];
            for (int i=0; i<tp->m_Height; i++)
            {
              for (int j=0; j<tp->m_Width; j++)
              {
                pDst[j*2+0] = pSrc[j*4+0];
                pDst[j*2+1] = pSrc[j*4+1];
              }
              pDst += d3dlr.Pitch;
              pSrc += tp->m_Width*4;
            }
          }
          break;
        default:
          assert(0);
      }
    }
    h = pID3DTexture->UnlockRect(0);
  }

  unticks(time0);
  m_RS.m_StatsTimeTexUpdate = (float)(time0*1000.0*g_SecondsPerCycle);
}

void CREOcean::DrawOceanSector(SOceanIndicies *oi)
{
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;
  h = dv->SetIndices((IDirect3DIndexBuffer9 *)oi->m_pIndicies);
  for (int i=0; i<oi->m_Groups.Num(); i++)
  {
    SPrimitiveGroup *g = &oi->m_Groups[i];
    switch (g->type)
    {
      case PT_STRIP:
        if (FAILED(h=dv->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, (OCEANGRID+1)*(OCEANGRID+1), g->offsIndex, g->numTris)))
        {
          r->Error("CREOcean::DrawOceanSector: DrawIndexedPrimitive error", h);
          return;
        }
        break;

      case PT_LIST:
        if (FAILED(h=dv->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, (OCEANGRID+1)*(OCEANGRID+1), g->offsIndex, g->numTris)))
        {
          r->Error("CREOcean::DrawOceanSector: DrawIndexedPrimitive error", h);
          return;
        }
        break;

      case PT_FAN:
        if (FAILED(h=dv->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, (OCEANGRID+1)*(OCEANGRID+1), g->offsIndex, g->numTris)))
        {
          r->Error("CREOcean::DrawOceanSector: DrawIndexedPrimitive error", h);
          return;
        }
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
  CD3D9Renderer *r = gcpRendD3D;

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
  float *pHM, *pTZ;

  int nFloats = (os->m_Flags & OSF_NEEDHEIGHTS) ? 1 : 0;
  if (nSplashes)
    nFloats++;

  pTZ = (float *)GetVBPtr((OCEANGRID+1)*(OCEANGRID+1));
  pHM = pTZ;
  int nStep = 1<<nLod;
  if (!nSplashes)
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
        pTZ += nStep*2;
      }
    }
  }
  else
  if (!(os->m_Flags & OSF_NEEDHEIGHTS))
  {
    for (int ty=0; ty<OCEANGRID+1; ty+=nStep)
    {
      pTZ = &pHM[ty*(OCEANGRID+1)*2];
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
        pTZ[0] = 0;
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

void CREOcean::mfReset()
{
  for (int i=0; i<NUM_OCEANVBS; i++)
  {
    IDirect3DVertexBuffer9* vb = (IDirect3DVertexBuffer9*)m_pVertsPool[i];
    SAFE_RELEASE(vb);
    m_pVertsPool[i] = NULL;
  }
  if (m_pBuffer)
  {
    gRenDev->ReleaseBuffer(m_pBuffer);
    m_pBuffer = NULL;
  }
}

void CREOcean::InitVB()
{
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;
  m_nNumVertsInPool = (OCEANGRID+1)*(OCEANGRID+1);
  int size = m_nNumVertsInPool * sizeof(struct_VERTEX_FORMAT_TEX2F);
  for (int i=0; i<NUM_OCEANVBS; i++)
  {
    h = dv->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_TEX1, D3DPOOL_DEFAULT, (IDirect3DVertexBuffer9**)&m_pVertsPool[i], NULL);
  }
  m_nCurVB = 0;
  m_bLockedVB = false;

  {
    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},   // normal
      D3DDECL_END()
    };
    h = dv->CreateVertexDeclaration(&elem[0], (IDirect3DVertexDeclaration9**)&m_VertDecl);
  }
  {
    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},   // normal
      {1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0}, // heightmpa/splashes
      D3DDECL_END()
    };
    h = dv->CreateVertexDeclaration(&elem[0], (IDirect3DVertexDeclaration9**)&m_VertDeclHeightSplash);
  }
}

struct_VERTEX_FORMAT_TEX2F *CREOcean::GetVBPtr(int nVerts)
{
  HRESULT h;

  struct_VERTEX_FORMAT_TEX2F *pVertices = NULL;
  if (nVerts > m_nNumVertsInPool)
  {
    assert(0);
    return NULL;
  }
  if (m_bLockedVB)
    UnlockVBPtr();
  m_nCurVB++;
  if (m_nCurVB >= NUM_OCEANVBS)
    m_nCurVB = 0;
  IDirect3DVertexBuffer9 *pVB = (IDirect3DVertexBuffer9 *)m_pVertsPool[m_nCurVB];
  h = pVB->Lock(0, 0, (void **) &pVertices, D3DLOCK_DISCARD);

  m_bLockedVB = true;
  return pVertices;
}

void CREOcean::UnlockVBPtr()
{
  if (m_bLockedVB)
  {
    IDirect3DVertexBuffer9 *pVB = (IDirect3DVertexBuffer9 *)m_pVertsPool[m_nCurVB];
    HRESULT hr = pVB->Unlock();
    m_bLockedVB = false;
  }
}

void CREOcean::mfDrawOceanSectors()
{ 
  float x, y;
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;
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
  fMaxDist = (float)((int)(fMaxDist / fSize + 1.0f)) * fSize;
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
  int nCurSize;
  bool bCurNeedBuffer = false;
  h = dv->SetVertexDeclaration((LPDIRECT3DVERTEXDECLARATION9)m_VertDecl);
  float *pHM, *pTZ;
  int nIndVP = CRenderer::CV_r_waterreflections ? 0 : 1;
  for (i=0; i<m_VisOceanSectors.Num(); i++)
  {
    SOceanSector *os = m_VisOceanSectors[i];
//    if ((int)os != 0x1ac32240 && (int)os != 0xe985fc8)
//      continue;
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
      pTZ = (float *)GetVBPtr((OCEANGRID+1)*(OCEANGRID+1));
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
    UnlockVBPtr();
    if (nFloats)
    {
      IDirect3DVertexBuffer9 *pVB = (IDirect3DVertexBuffer9 *)m_pVertsPool[m_nCurVB];
      h = dv->SetStreamSource(1, pVB, 0, sizeof(struct_VERTEX_FORMAT_TEX2F));
      if (!bCurNeedBuffer)
      {
        h = dv->SetVertexDeclaration((LPDIRECT3DVERTEXDECLARATION9)m_VertDeclHeightSplash);
        bCurNeedBuffer = true;
      }
    }
    else
    if (bCurNeedBuffer)
    {
      h = dv->SetStreamSource(1, NULL, 0, 0);
      h = dv->SetVertexDeclaration((LPDIRECT3DVERTEXDECLARATION9)m_VertDecl);
      bCurNeedBuffer = false;
    }
    float param[4];
    param[0] = os->x;
    param[1] = os->y;
    param[2] = 0;
    param[3] = 0;
    CCGVProgram_D3D *vpD3D = (CCGVProgram_D3D *)vp;
    SCGBind *pBind = vpD3D->mfGetParameterBind("PosOffset");
    if (pBind)
      vpD3D->mfParameter4f(pBind, param);
    pBind = vpD3D->mfGetParameterBind("PosScale");
    param[0] = fSize;
    param[1] = fSize;
    param[2] = fHeightScale;
    param[3] = 1;
    if (pBind)
      vpD3D->mfParameter4f(pBind, param);

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
  }
}

void CREOcean::mfDrawOceanScreenLod()
{ 
}

bool CREOcean::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  double time0 = 0;
  ticks(time0);

  if (!m_pVertsPool[0])
    InitVB();
  if (!m_pBuffer)
    m_pBuffer = gRenDev->CreateBuffer((OCEANGRID+1)*(OCEANGRID+1), VERTEX_FORMAT_P3F_N, "Ocean", true);

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
  CVertexBuffer *vb = m_pBuffer;

  for (int i=0; i<VSF_NUM; i++)
  {
    if (vb->m_VS[i].m_bLocked)
      gcpRendD3D->UnlockBuffer(vb, i);
  }

  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;

  h = dv->SetStreamSource(0, (IDirect3DVertexBuffer9 *)vb->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr, 0, m_VertexSize[vb->m_vertexformat]);

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



