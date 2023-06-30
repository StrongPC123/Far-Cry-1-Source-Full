/*=============================================================================
  D3DRenderRE.cpp : implementation of the Rendering RenderElements pipeline.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "I3dengine.h"
#include "D3DCGPShader.h"
#include "D3DCGVProgram.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//=======================================================================

void CRE2DQuad::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_FirstVertex = 0;
  gRenDev->m_RP.m_RendNumVerts = 4;

  int w=800;
  int h=600;

  m_arrVerts[0].xyz.x = (float)w;
  m_arrVerts[0].xyz.y = 0;
  m_arrVerts[0].xyz.z = 0;
  m_arrVerts[0].st[0] = 1;
  m_arrVerts[0].st[1] = 0;

  m_arrVerts[1].xyz.x = 0;
  m_arrVerts[1].xyz.y = 0;
  m_arrVerts[1].xyz.z = 0;
  m_arrVerts[1].st[0] = 0;
  m_arrVerts[1].st[1] = 0;

  m_arrVerts[2].xyz.x = (float)w;
  m_arrVerts[2].xyz.y = (float)h;
  m_arrVerts[2].xyz.z = 0;
  m_arrVerts[2].st[0] = 1;
  m_arrVerts[2].st[1] = 1;

  m_arrVerts[3].xyz.x = 0;
  m_arrVerts[3].xyz.y = (float)h;
  m_arrVerts[3].xyz.z = 0;
  m_arrVerts[3].st[0] = 0;
  m_arrVerts[3].st[1] = 1;
}


bool CRE2DQuad::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  // setup screen aligned quad...
  struct_VERTEX_FORMAT_P3F_TEX2F pScreenQuad[] =
  {
    Vec3(0, 0, 0), 0, 0,
    Vec3(0, 1, 0), 0, 1,
    Vec3(1, 0, 0), 1, 0,
    Vec3(1, 1, 0), 1, 1, 
  };

  gRenDev->Set2DMode(true, 1, 1);  
  gRenDev->DrawTriStrip(&CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F),4);
  gRenDev->Set2DMode(false, 1, 1);
  
  return true;
}

void CREFlareGeom::mfCheckVis(CFColor &col, CCObject *obj)
{
  float Depth[3][3];
  float ft;
  float f;

  CD3D9Renderer *rd = gcpRendD3D;
  int re = 0;//rd->mfGetRecurseLevel();
  SFlareFrame *ff = &mFlareFr[re];

  LPDIRECT3DDEVICE9 pd3dDevice = rd->mfGetD3DDevice();
  IDirect3DSurface9 *ZSurf = rd->mfGetZSurface();
  D3DSURFACE_DESC *ddsd = rd->mfGetZSurfaceDesc();

  // Lock Z surface.
  int minx, miny, maxx, maxy;
  int hgt = rd->GetHeight();
  int wdt = rd->GetWidth();
  int y = hgt - ff->mY;
  if (ff->mX-5 < 0)
    minx = 0;
  else
    minx = ff->mX-5;
  if (y-5 < 0)
    miny = 0;
  else
    miny = y-5;
  if (ff->mX+5 > wdt)
    maxx = wdt;
  else
    maxx = ff->mX+5;
  if (y+5 > hgt)
    maxy = hgt;
  else
    maxy = y+5;


  D3DLOCKED_RECT rc;
  //HRESULT ddrval = ZSurf->LockRect( &rc, NULL, D3DLOCK_READONLY );
  //if( ddrval!=D3D_OK )
  {
    //iLog->Log("D3D Driver: Lock on Zbuffer failed (%s)\n", rd->D3DError(ddrval));
    return;
  }

  int i, j;
  switch( ddsd->Format )
  {
    case D3DFMT_D16:
      {
        ushort* src = (ushort*)rc.pBits;
        int pitch = rc.Pitch>>1;
        src = &src[minx+miny*pitch];
        for (i=0; i<3; i++)
        {
          for (j=0; j<3; j++)
          {
            Depth[i][j] = src[j*5] / 65536.0f;
          }
          src += pitch*5;
        }
      }
      break;

    case D3DFMT_D24S8:
      {
        DWORD* src = (DWORD*)rc.pBits;
        int pitch = rc.Pitch>>2;
        src = &src[minx+miny*pitch];
        for (i=0; i<3; i++)
        {
          for (j=0; j<3; j++)
          {
            Depth[i][j] = (src[j*5]>>8) / 16777216.0f;
          }
          src += pitch*5;
        }
      }
      break;

#ifndef _XBOX      
    case D3DFMT_D32:
      {
        DWORD* src = (DWORD*)rc.pBits;
        int pitch = rc.Pitch>>2;
        src = &src[minx+miny*pitch];
        for (i=0; i<3; i++)
        {
          for (j=0; j<3; j++)
          {
            Depth[i][j] = src[j*5] / 4294967296.0f;
          }
          src += pitch*5;
        }
      }
      break;
#endif
  }
  ZSurf->UnlockRect();

  float Scale = 0;
  for (i=0; i<3; i++)
  {
    for (j=0; j<3; j++)
    {
      float Dept = (Depth[i][j]-rd->mMinDepth) / (rd->mMaxDepth-rd->mMinDepth);
      f = gRenDev->m_ProjMatrix(3,2)/(Dept*gRenDev->m_ProjMatrix(2,3)-gRenDev->m_ProjMatrix(2,2)) - ff->mDepth;
      if (f < 24.0f)
      {
        if (i==1 && j==1)
          Scale += 3.0f;
        else
        if (i==1 || j==1)
          Scale += 2.0f;
        else
          Scale += 1.0f;
      }
    }
  }
  ff->mScale = Scale / 9.0f;
  if (ff->mScale > 1.0f)
    ff->mScale = 1.0f;
  if (ff->mScale)
  {
    if (!ff->mbVis)
    {
      ff->mbVis = true;
      ff->mDecayTime = gRenDev->m_RP.m_RealTime-0.001f;
    }
    ft = (gRenDev->m_RP.m_RealTime - ff->mDecayTime) * CRenderer::CV_r_coronafade;
    if (ft > 1.0f)
      ft = 1.0f;
    ft *= ff->mScale;
  }
  else
  {
    if (ff->mbVis)
    {
      ff->mbVis = false;
      ff->mDecayTime = gRenDev->m_RP.m_RealTime-0.001f;
    }
    ft = 1.0f - (gRenDev->m_RP.m_RealTime - ff->mDecayTime) * CRenderer::CV_r_coronafade;
    if (ft > 1.0f)
      ft = 1.0f;
    ft *= ff->mScale;
  }
  col = ff->mColor;
  col.a = ft;
  col.ClampAlpha();
}

//=========================================================================================

static void _Draw3dBBoxSolid(const Vec3d &mins,const Vec3d &maxs)
{
  int nOffs;
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  HRESULT hr;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad;

  // Set the vertex shader to the FVF fixed function shader
  r->EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

  vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)r->GetVBPtr3D(4, nOffs);
  vQuad[0].xyz = Vec3d(mins.x,mins.y,mins.z); vQuad[0].st[0] = vQuad[0].st[1] = 0; vQuad[0].color.dcolor = -1;
  vQuad[1].xyz = Vec3d(mins.x,mins.y,maxs.z); vQuad[1].st[0] = vQuad[1].st[1] = 0; vQuad[1].color.dcolor = -1;
  vQuad[2].xyz = Vec3d(maxs.x,mins.y,maxs.z); vQuad[2].st[0] = vQuad[2].st[1] = 0; vQuad[2].color.dcolor = -1;
  vQuad[3].xyz = Vec3d(maxs.x,mins.y,mins.z); vQuad[3].st[0] = vQuad[3].st[1] = 0; vQuad[3].color.dcolor = -1;
  r->UnlockVB3D();
  // Bind our vertex as the first data stream of our device
  dv->SetStreamSource(0, r->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  // Render the two triangles from the data stream
  hr = dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)r->GetVBPtr3D(4, nOffs);
  vQuad[0].xyz = Vec3d(mins.x,mins.y,mins.z); vQuad[0].st[0] = vQuad[0].st[1] = 0; vQuad[0].color.dcolor = -1;
  vQuad[1].xyz = Vec3d(mins.x,mins.y,maxs.z); vQuad[1].st[0] = vQuad[1].st[1] = 0; vQuad[1].color.dcolor = -1;
  vQuad[2].xyz = Vec3d(mins.x,maxs.y,maxs.z); vQuad[2].st[0] = vQuad[2].st[1] = 0; vQuad[2].color.dcolor = -1;
  vQuad[3].xyz = Vec3d(mins.x,maxs.y,mins.z); vQuad[3].st[0] = vQuad[3].st[1] = 0; vQuad[3].color.dcolor = -1;
  r->UnlockVB3D();
  // Bind our vertex as the first data stream of our device
  dv->SetStreamSource(0, r->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  // Render the two triangles from the data stream
  hr = dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)r->GetVBPtr3D(4, nOffs);
  vQuad[0].xyz = Vec3d(mins.x,maxs.y,mins.z); vQuad[0].st[0] = vQuad[0].st[1] = 0; vQuad[0].color.dcolor = -1;
  vQuad[1].xyz = Vec3d(mins.x,maxs.y,maxs.z); vQuad[1].st[0] = vQuad[1].st[1] = 0; vQuad[1].color.dcolor = -1;
  vQuad[2].xyz = Vec3d(maxs.x,maxs.y,maxs.z); vQuad[2].st[0] = vQuad[2].st[1] = 0; vQuad[2].color.dcolor = -1;
  vQuad[3].xyz = Vec3d(maxs.x,maxs.y,mins.z); vQuad[3].st[0] = vQuad[3].st[1] = 0; vQuad[3].color.dcolor = -1;
  r->UnlockVB3D();
  // Bind our vertex as the first data stream of our device
  dv->SetStreamSource(0, r->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  // Render the two triangles from the data stream
  hr = dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)r->GetVBPtr3D(4, nOffs);
  vQuad[0].xyz = Vec3d(maxs.x,maxs.y,mins.z); vQuad[0].st[0] = vQuad[0].st[1] = 0; vQuad[0].color.dcolor = -1;
  vQuad[1].xyz = Vec3d(maxs.x,maxs.y,maxs.z); vQuad[1].st[0] = vQuad[1].st[1] = 0; vQuad[1].color.dcolor = -1;
  vQuad[2].xyz = Vec3d(maxs.x,mins.y,maxs.z); vQuad[2].st[0] = vQuad[2].st[1] = 0; vQuad[2].color.dcolor = -1;
  vQuad[3].xyz = Vec3d(maxs.x,mins.y,mins.z); vQuad[3].st[0] = vQuad[3].st[1] = 0; vQuad[3].color.dcolor = -1;
  r->UnlockVB3D();
  // Bind our vertex as the first data stream of our device
  dv->SetStreamSource(0, r->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  // Render the two triangles from the data stream
  hr = dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  // top
  vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)r->GetVBPtr3D(4, nOffs);
  vQuad[0].xyz = Vec3d(maxs.x,maxs.y,maxs.z); vQuad[0].st[0] = vQuad[0].st[1] = 0; vQuad[0].color.dcolor = -1;
  vQuad[1].xyz = Vec3d(mins.x,maxs.y,maxs.z); vQuad[1].st[0] = vQuad[1].st[1] = 0; vQuad[1].color.dcolor = -1;
  vQuad[2].xyz = Vec3d(mins.x,mins.y,maxs.z); vQuad[2].st[0] = vQuad[2].st[1] = 0; vQuad[2].color.dcolor = -1;
  vQuad[3].xyz = Vec3d(maxs.x,mins.y,maxs.z); vQuad[3].st[0] = vQuad[3].st[1] = 0; vQuad[3].color.dcolor = -1;
  r->UnlockVB3D();
  // Bind our vertex as the first data stream of our device
  dv->SetStreamSource(0, r->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  // Render the two triangles from the data stream
  hr = dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  // bottom
  vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)r->GetVBPtr3D(4, nOffs);
  vQuad[0].xyz = Vec3d(maxs.x,mins.y,mins.z); vQuad[0].st[0] = vQuad[0].st[1] = 0; vQuad[0].color.dcolor = -1;
  vQuad[1].xyz = Vec3d(mins.x,mins.y,mins.z); vQuad[1].st[0] = vQuad[1].st[1] = 0; vQuad[1].color.dcolor = -1;
  vQuad[2].xyz = Vec3d(mins.x,maxs.y,mins.z); vQuad[2].st[0] = vQuad[2].st[1] = 0; vQuad[2].color.dcolor = -1;
  vQuad[3].xyz = Vec3d(maxs.x,maxs.y,mins.z); vQuad[3].st[0] = vQuad[3].st[1] = 0; vQuad[3].color.dcolor = -1;
  r->UnlockVB3D();
  // Bind our vertex as the first data stream of our device
  dv->SetStreamSource(0, r->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  // Render the two triangles from the data stream
  hr = dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  r->m_nPolygons+=12;
}

CREOcclusionQuery::~CREOcclusionQuery()
{
  int i;

  mfReset();

  for (i=0; i<8; i++)
  {
  }
}

void CREOcclusionQuery::mfReset()
{
  LPDIRECT3DQUERY9  pVizQuery = (LPDIRECT3DQUERY9)m_nOcclusionID;
  if (pVizQuery)
  {
    pVizQuery->Release();
    m_nOcclusionID = 0;
  }
}

bool CREOcclusionQuery::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  HRESULT hr;

  if (!(r->m_Features & RFT_OCCLUSIONTEST))
  {
    m_nVisSamples = r->GetWidth()*r->GetHeight();
    return true;
  }

  int nFrame = r->GetFrameID();

//	if(nFrame&1)
	//  return true;

  if ( m_nCheckFrame != nFrame )
  {
		if(m_nCheckFrame)
		{
			m_nVisSamples=0;
      // Stupidly block until we have a query result
      LPDIRECT3DQUERY9  pVizQuery = (LPDIRECT3DQUERY9)m_nOcclusionID;
      if (pVizQuery)
      {
        double time = sCycles2();
        bool bInfinite = false;
        while (pVizQuery->GetData((void *) &m_nVisSamples, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE)
        {
          double dif = sCycles2()+34-time;
          if (dif*1000.0*g_SecondsPerCycle > 5000)
          {
            // 5 seconds in the loop
            bInfinite = true;
            break;
          }
        }
        r->m_RP.m_PS.m_fOcclusionTime += (float)((sCycles2()+34-time)*1000.0*g_SecondsPerCycle);
        if (bInfinite)
          iLog->Log("Error: Seems like infinite loop in occlusion query");
      }
    }
    m_nCheckFrame = nFrame;
  }

  if (!m_nOcclusionID)
  {
    // Create the query if we can
    {
      // Create visibility queries
      LPDIRECT3DQUERY9  pVizQuery = NULL;
      hr = dv->CreateQuery (D3DQUERYTYPE_OCCLUSION, &pVizQuery);
      if (pVizQuery)
        m_nOcclusionID = (UINT_PTR)pVizQuery;
    }
  }

  if (m_nDrawFrame != nFrame)
	{ // draw bbox
    LPDIRECT3DQUERY9  pVizQuery = (LPDIRECT3DQUERY9)m_nOcclusionID;
    if (pVizQuery)
    {
      pVizQuery->Issue (D3DISSUE_BEGIN);
		  _Draw3dBBoxSolid(m_vBoxMin-Vec3d(0.05f,0.05f,0.05f),m_vBoxMax+Vec3d(0.05f,0.05f,0.05f));
      pVizQuery->Issue (D3DISSUE_END);
    }
		m_nDrawFrame = nFrame;
	}

  return true;
}

void CRETempMesh::mfReset()
{
  gRenDev->ReleaseBuffer(m_VBuffer);
  gRenDev->ReleaseIndexBuffer(&m_Inds);
  m_VBuffer = NULL;
}

bool CRETempMesh::mfPreDraw(SShaderPass *sl)
{
  CVertexBuffer *vb = m_VBuffer;

  for (int i=0; i<VSF_NUM; i++)
  {
    if (vb->m_VS[i].m_bLocked)
      gcpRendD3D->UnlockBuffer(vb, i);
  }

  if (!m_Inds.m_nItems)
    return false;

  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;

  h = dv->SetStreamSource( 0, (IDirect3DVertexBuffer9 *)vb->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr, 0, m_VertexSize[vb->m_vertexformat]);

  if (gRenDev->m_RP.m_FlagsModificators & RBMF_TANGENTSUSED)
  {
    gRenDev->m_RP.m_PersFlags |= RBPF_USESTREAM1;
    h = dv->SetStreamSource( 1, (IDirect3DVertexBuffer9 *)vb->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr, 0, sizeof(SPipTangents));
  }
  else
  if (gRenDev->m_RP.m_PersFlags & RBPF_USESTREAM1)
  {
    gRenDev->m_RP.m_PersFlags &= ~RBPF_USESTREAM1;
    h = dv->SetStreamSource( 1, NULL, 0, 0);
  }

  if (gRenDev->m_RP.m_PersFlags & RBPF_USESTREAM2)
  {
    gRenDev->m_RP.m_PersFlags &= ~RBPF_USESTREAM2;
    h = dv->SetStreamSource( 2, NULL, 0, 0);
  }

  h = dv->SetIndices((IDirect3DIndexBuffer9 *)m_Inds.m_VertBuf.m_pPtr);

  return true;
}


bool CRETempMesh::mfDraw(SShader *ef, SShaderPass *sl)
{
  CD3D9Renderer *r = gcpRendD3D;
  CVertexBuffer *vb = m_VBuffer;
  if (!vb)
    return false;

  // Hardware shader
  int nPrimType = R_PRIMV_TRIANGLES;
  r->EF_DrawIndexedMesh(nPrimType);
  vb->m_bFenceSet = true;

  return true;
}

bool CREOcLeaf::mfPreDraw(SShaderPass *sl)
{
  CLeafBuffer *lb = m_pBuffer->GetVertexContainer();
  // Should never happen. Video buffer is missing
  if (!lb->m_pVertexBuffer)
    return false;
  CD3D9Renderer *rd = gcpRendD3D;
  for (int i=0; i<VSF_NUM; i++)
  {
    if (lb->m_pVertexBuffer->m_VS[i].m_bLocked)
      rd->UnlockBuffer(lb->m_pVertexBuffer, i);
  }
  LPDIRECT3DDEVICE9 dv = rd->mfGetD3DDevice();
  HRESULT h;

  int nOffs;
  IDirect3DVertexBuffer9 *vptr = (IDirect3DVertexBuffer9 *)lb->m_pVertexBuffer->GetStream(VSF_GENERAL, &nOffs);
  vptr = vptr ? vptr : rd->m_pVB3D[1];
  h = dv->SetStreamSource( 0, vptr, nOffs, m_VertexSize[lb->m_pVertexBuffer->m_vertexformat]);

  if (rd->m_RP.m_FlagsModificators & RBMF_TANGENTSUSED)
  {
    rd->m_RP.m_PersFlags |= RBPF_USESTREAM1;
    vptr = (IDirect3DVertexBuffer9 *)lb->m_pVertexBuffer->GetStream(VSF_TANGENTS, &nOffs);
    vptr = vptr ? vptr : rd->m_pVB3D[2];
    h = dv->SetStreamSource( 1, vptr, nOffs, sizeof(SPipTangents));
  }
  else
  if (rd->m_RP.m_PersFlags & RBPF_USESTREAM1)
  {
    rd->m_RP.m_PersFlags &= ~RBPF_USESTREAM1;
    h = dv->SetStreamSource( 1, NULL, 0, 0);
  }

  if (rd->m_RP.m_FlagsModificators & RBMF_LMTCUSED)
  {
    rd->m_RP.m_PersFlags |= RBPF_USESTREAM2;
    if (rd->m_RP.m_pCurObject->m_pLMTCBufferO && rd->m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer)
    {
      vptr = (IDirect3DVertexBuffer9 *)rd->m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer->GetStream(VSF_GENERAL, &nOffs);
      h = dv->SetStreamSource(2, vptr, nOffs, m_VertexSize[rd->m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer->m_vertexformat]);
    }
  }
  else
  if (rd->m_RP.m_PersFlags & RBPF_USESTREAM2)
  {
    rd->m_RP.m_PersFlags &= ~RBPF_USESTREAM2;
    h = dv->SetStreamSource( 2, NULL, 0, 0);
  }

  if (CRenderer::CV_r_cullgeometryforlights && rd->EF_IsOnlyLightPass((SShaderPassHW *)sl) && rd->m_RP.m_pCurLightIndices != &rd->m_RP.m_FakeLightIndices)
  {
    assert(rd->m_RP.m_pCurLightIndices);
    SVertexStream *vi = rd->m_RP.m_pCurLightIndices->GetIndexBuffer();
    IDirect3DIndexBuffer9 *pIndBuf = (IDirect3DIndexBuffer9 *)vi->m_VertBuf.m_pPtr;
    assert(pIndBuf);
    h = dv->SetIndices(pIndBuf);
  }
  else
  {
    IDirect3DIndexBuffer9 *pIndBuf = (IDirect3DIndexBuffer9 *)m_pBuffer->m_Indices.m_VertBuf.m_pPtr;
    //assert(pIndBuf);
    h = dv->SetIndices(pIndBuf);
  }

  return true;
}


bool CREOcLeaf::mfDraw(SShader *ef, SShaderPass *sl)
{
  CD3D9Renderer *r = gcpRendD3D;
  
  CLeafBuffer *lb = m_pBuffer;
  
  // Hardware effector
  if (ef->m_HWTechniques.Num())
  {
    r->EF_DrawIndexedMesh(lb->m_nPrimetiveType);    
    return true;
  }
  
  r->DrawBuffer(lb->m_pVertexBuffer, &lb->m_Indices, m_pChunk->nNumIndices, m_pChunk->nFirstIndexId, lb->m_nPrimetiveType,m_pChunk->nFirstVertId,m_pChunk->nNumVerts);
  
  return (true);
}

// ===============================================================
// mfDraw - glare fx's
// Last Update: 04/06/2003

bool CREGlare::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  //// setup screen aligned quad
  //struct_VERTEX_FORMAT_P3F_TEX2F pScreenQuad[] =
  //{
  //  0, 0, 0, 0, 0,
  //  0, 1, 0, 0, 1,
  //  1, 0, 0, 1, 0,   
  //  1, 1, 0, 1, 1, 
  //}; 

  //// render quad
  //gRenDev->Set2DMode(true, 1, 1);
  //gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);  
  //gRenDev->Set2DMode(false, 1, 1);

  return true;
}

// ===============================================================
// FlashBang fx
// Last Update: 24/04/2003

// render flashbang
bool CREFlashBang::mfDraw(SShader *ef, SShaderPass *sfm)
{   
  //// sincronize
  //ITimer *pTimer=iSystem->GetITimer();  
  //m_fFlashTimeOut-= (0.00009f*m_fTimeScale*(pTimer->GetFrameTime()*1000.0f));

  //// reset animation
  //if(m_fFlashTimeOut<=0.01f) 
  //{    
  //  m_bIsActive=0;
  //  m_fFlashTimeOut=1.0f;
  //} 

  //// setup screen aligned quad
  //struct_VERTEX_FORMAT_P3F_TEX2F pScreenQuad[] =  
  //{
  //  800, 600, 0, 1, 1,
  //  800, 0, 0, 1, 0, 
  //  0, 600, 0, 0, 1,
  //  0, 0, 0, 0, 0, 
  //};

  //// render quad
  //gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)), 4);

  return true;
}


void CREOcLeaf::mfEndFlush(void)
{
}

bool CRETriMeshShadow::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  if(m_nCurrInst<0 || m_nCurrInst>=MAX_SV_INSTANCES)
  {
//    iLog->Log("Warning: CRETriMeshShadow::mfDraw: m_nCurrInst not set");
    return false;
  }

  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->mfGetD3DDevice();

  rd->CV_ind_VisualizeShadowVolumes = (rd->GetPolygonMode() == R_WIREFRAME_MODE);

  CDLight *pDL = NULL;
  if (rd->m_RP.m_DynLMask)
  {
    for (int n=0; n<rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); n++)
    {
      if (rd->m_RP.m_DynLMask & (1<<n))
      {
        pDL = rd->m_RP.m_DLights[SRendItem::m_RecurseLevel][n];
        break;
      }
    }
  }
  if (pDL && pDL->m_sWidth && pDL->m_sHeight)
    rd->EF_Scissor(true, pDL->m_sX, pDL->m_sY, pDL->m_sWidth, pDL->m_sHeight);

  if (gRenDev->CV_ind_VisualizeShadowVolumes)
  {
    rd->EF_SetGlobalColor(1,1,1,1);
    rd->EF_SetState(GS_NODEPTHTEST | GS_STENCIL);
  }
  else
    rd->EF_SetState(GS_NOCOLMASK | GS_STENCIL);

  rd->EF_SetColorOp(eCO_DISABLE, eCO_DISABLE, DEF_TEXARG0, DEF_TEXARG0);

  if (rd->m_d3dCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED)
  {
    // using 2-sided stencil
    if(m_arrLBuffers[m_nCurrInst].pVB && m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer)
    {   
      rd->EF_SetStencilState(STENCOP_FAIL(FSS_STENCOP_KEEP) |
                             STENCOP_ZFAIL(FSS_STENCOP_DECR_WRAP) |
                             STENCOP_PASS(FSS_STENCOP_KEEP) |
                             STENC_FUNC(FSS_STENCFUNC_ALWAYS) | 
                             STENCOP_CCW_FAIL(FSS_STENCOP_KEEP) |
                             STENCOP_CCW_ZFAIL(FSS_STENCOP_INCR_WRAP) |
                             STENCOP_CCW_PASS(FSS_STENCOP_KEEP) |
                             STENC_CCW_FUNC(FSS_STENCFUNC_ALWAYS) |
                             FSS_STENCIL_TWOSIDED,
                             0, -1);

      rd->D3DSetCull(eCULL_None);

      CVertexBuffer * pVB = m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer;
      SVertexStream *Indices = &m_arrLBuffers[m_nCurrInst].pVB->m_Indices;

      int nIndices;
      if (m_nRendIndices)
        nIndices = m_nRendIndices;
      else
        nIndices = Indices->m_nItems;

      assert(nIndices);

      if (Indices->m_bLocked)
        rd->UpdateIndexBuffer(Indices, NULL, 0, true);

      rd->DrawBuffer(pVB, Indices, nIndices, 0, R_PRIMV_TRIANGLES);

      gRenDev->m_nShadowVolumePolys += nIndices/3;
    }
  }
  else
  {
    if(m_arrLBuffers[m_nCurrInst].pVB && m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer)
    {     
      rd->EF_SetStencilState(STENCOP_FAIL(FSS_STENCOP_KEEP) |
                             STENCOP_ZFAIL(FSS_STENCOP_INCR) |
                             STENCOP_PASS(FSS_STENCOP_KEEP) |
                             STENC_FUNC(FSS_STENCFUNC_ALWAYS),
                             0, -1);

      rd->D3DSetCull(eCULL_Front);

      CVertexBuffer * pVB = m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer;
      SVertexStream *Indices = &m_arrLBuffers[m_nCurrInst].pVB->m_Indices;

      int nIndices;
      if (m_nRendIndices)
        nIndices = m_nRendIndices;
      else
        nIndices = Indices->m_nItems;

      assert(nIndices);

      if (Indices->m_bLocked)
        rd->UpdateIndexBuffer(Indices, NULL, 0, true);

      rd->DrawBuffer(pVB, Indices, nIndices, 0, R_PRIMV_TRIANGLES);

      rd->EF_SetStencilState(STENCOP_FAIL(FSS_STENCOP_KEEP) |
                             STENCOP_ZFAIL(FSS_STENCOP_DECR) |
                             STENCOP_PASS(FSS_STENCOP_KEEP) |
                             STENC_FUNC(FSS_STENCFUNC_ALWAYS),
                             0, -1);

      rd->D3DSetCull(eCULL_Back);

      rd->DrawBuffer(pVB, Indices, nIndices, 0, R_PRIMV_TRIANGLES);

      rd->SetFenceCompleted(pVB); // set it explicitly since second DrawBuffer do not set it

      gRenDev->m_nShadowVolumePolys += nIndices/3;
    }
  }

  m_nCurrInst = -1;

  return true;
}
/*
bool CRETriMeshAdditionalShadow::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  return true;
} */

///////////////////////////////////////////////////////////////////
// Render shadows on the object as first pass
///////////////////////////////////////////////////////////////////
/*bool CREShadowMap::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  return true;
}	*/

//===================================================================================

bool CREFlare::mfCheckVis(CCObject *obj)
{
  if (!obj)
    return false;

  Vec3d or = obj->GetTranslation();
  //bool bVis = false;
  bool bSun = false;
  int i, j;
  
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->mfGetD3DDevice();
  HRESULT hr;

  if (m_Importance <= CRenderer::CV_r_coronas)
  {
    float fScaleCorona = m_fScaleCorona;
    if (m_pScaleCoronaParams)
      fScaleCorona = m_pScaleCoronaParams->mfGet();
    if (obj->m_ObjFlags & FOB_DRSUN)
    {
      bSun = true;
      //or.Normalize();
      //or *= 500.0f;
      //or += gRenDev->m_RP.m_ViewOrg;
    }
    if (bSun && (gRenDev->m_RP.m_PersFlags & RBPF_DONTDRAWSUN))
      return false;

    D3DXMATRIX mIdent;
    D3DXMatrixIdentity(&mIdent);
    Matrix44 projMatr, camMatr;
    camMatr = rd->m_prevCamera.GetVCMatrixD3D9();
    float fov=rd->m_prevCamera.GetFov()*rd->m_prevCamera.GetProjRatio();
    D3DXMatrixPerspectiveFovRH((D3DXMATRIX *)&projMatr, fov, 1.0f/rd->m_prevCamera.GetProjRatio(), rd->m_prevCamera.GetZMin(), rd->m_prevCamera.GetZMax());
    dv->SetTransform(D3DTS_VIEW, (D3DXMATRIX *)&camMatr); 
    dv->SetTransform(D3DTS_PROJECTION, (D3DXMATRIX *)&projMatr); 
    Vec3 camVecs[3];
    camVecs[1][0] = camMatr(0,1);
    camVecs[1][1] = camMatr(1,1);
    camVecs[1][2] = camMatr(2,1);

    camVecs[2][0] = camMatr(0,0);
    camVecs[2][1] = camMatr(1,0);
    camVecs[2][2] = camMatr(2,0);

    if (CRenderer::CV_r_SunStyleCoronas)
      bSun = true;
    float fFade = 1.0f;
    bool bRays = bSun && CRenderer::CV_r_checkSunVis == 2;
    if (!bSun)
    {
      float fFogEnd = rd->m_FS.m_FogEnd;
      float fDist = (rd->m_RP.m_ViewOrg-or).Length();
      float fStartFade = fFogEnd/3.0f*2.0f;
      float fEndFade = fStartFade*1.5f;
      if (fDist > fEndFade)
        fFade = 0;
      else
      if (fDist < fStartFade)
        fFade = 1.0f;
      else
        fFade = (fEndFade-fDist) / (fEndFade-fStartFade);
    }
    obj->m_TempVars[2] = 0;

    int vp[4];
    rd->GetViewport(&vp[0], &vp[1], &vp[2], &vp[3]);
    D3DVIEWPORT9 D3DVP;
    D3DVP.X = vp[0];
    D3DVP.Y = vp[1];
    D3DVP.Width = vp[2];
    D3DVP.Height = vp[3];
    D3DVP.MinZ = 0;
    D3DVP.MaxZ = 1;

    Vec3 vScr;
    D3DXVec3Project((D3DXVECTOR3 *)&vScr, (D3DXVECTOR3 *)&or, &D3DVP, (D3DXMATRIX *)&projMatr, (D3DXMATRIX *)&camMatr, &mIdent);

    int n = 2;
    obj->m_Trans2[0] = vScr.x;
    obj->m_Trans2[1] = vScr.y;
    obj->m_Trans2[2] = vScr.z;
    float fCheckInterval = m_fFadeTime;
    if (fCheckInterval < 0)
      fCheckInterval = CRenderer::CV_r_coronafade*0.125f;
    if (bRays)
      fCheckInterval *= 0.5f;
    if (obj->m_fLightFadeTime-1.0f > rd->m_RP.m_RealTime)
      obj->m_fLightFadeTime = rd->m_RP.m_RealTime;
    if (rd->m_RP.m_RealTime-obj->m_fLightFadeTime < fCheckInterval)
    {
      dv->SetTransform(D3DTS_VIEW, rd->m_matView->GetTop()); 
      dv->SetTransform(D3DTS_PROJECTION, rd->m_matProj->GetTop()); 
      return true;
    }
    obj->m_fLightFadeTime = gRenDev->m_RP.m_RealTime;
    vScr.x = obj->m_Trans2[0];
    vScr.y = obj->m_Trans2[1];
    vScr.z = obj->m_Trans2[2];
    float fIntens = 0;
    if (vScr.x>=0 && vScr.y>=0 && vScr.x<vp[2] && vScr.y<vp[3] && fFade)
    {
      int minx, miny, maxx, maxy;
      int wdt = rd->m_d3dsdBackBuffer.Width;
      int hgt = rd->m_d3dsdBackBuffer.Height;
      int sx = (int)vScr.x;
      int sy = (int)vScr.y;
      if (sx-2 < 0)
        minx = 0;
      else
        minx = sx-2;
      if (sy-2 < 0)
        miny = 0;
      else
        miny = sy-2;
      if (sx+2 > wdt)
        maxx = wdt;
      else
        maxx = sx+2;
      if (sy+2 > hgt)
        maxy = hgt;
      else
        maxy = sy+2;

      D3DLOCKED_RECT rc;

      gRenDev->m_TexMan->m_Text_White->Set();
      rd->D3DSetCull(eCULL_None);

      if (!(rd->m_Features & RFT_OCCLUSIONTEST))
      {
        // Lock back surface.

        rd->m_matProj->Push();
        D3DXMatrixOrthoRH(rd->m_matProj->GetTop(), (float)rd->GetWidth(), (float)rd->GetHeight(), -20.0, 0.0);
        dv->SetTransform(D3DTS_PROJECTION, rd->m_matProj->GetTop()); 

        rd->PushMatrix();
        rd->m_matView->LoadIdentity();
        dv->SetTransform(D3DTS_VIEW, rd->m_matView->GetTop()); 
        rd->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

        // set alpha to 0
        {
          rd->EF_SetState(GS_NODEPTHTEST);
          rd->DrawQuad((float)minx, (float)miny, (float)maxx, (float)maxy, CFColor(0.0f), vScr.z);
        }

        // Set alpha mask of visible areas (1 is visible)
        {
          rd->EF_SetState(0);
          rd->DrawQuad((float)minx, (float)miny, (float)maxx, (float)maxy, CFColor(1.0f), vScr.z);
        }
        IDirect3DSurface9 * pTar = rd->mfGetBackSurface();
        DWORD dwPixSize = 4;
        D3DSURFACE_DESC dc;
        pTar->GetDesc(&dc);
        switch(dc.Format)
        {
          case D3DFMT_R8G8B8:
            dwPixSize = 3;
            break;
          case D3DFMT_R5G6B5:
          case D3DFMT_X1R5G5B5:
          case D3DFMT_A1R5G5B5:
            dwPixSize = 2;
            break;
        };

        hr = pTar->LockRect( &rc, NULL, D3DLOCK_READONLY );
        if( hr!=D3D_OK )
        {
//          iLog->Log("D3D Driver: Lock on BackBuffer failed (%s)\n", rd->D3DError(hr));
          return false;
        }
        int dwAccum = 0;
        for (i=miny; i<maxy; i++)
        {
          byte *pPtr = (byte *)rc.pBits;
          pPtr += i*rc.Pitch + minx*dwPixSize;
          for (j=minx; j<maxx; j++)
          {
            int Val = *pPtr;
            if (abs(i-sy) > 1)
              Val >>= 1;
            if (abs(j-sx) > 1)
              Val >>= 1;
            dwAccum += Val;
            pPtr += dwPixSize;
          }
        }
        hr = pTar->UnlockRect();

        rd->PopMatrix();
        rd->m_matProj->Pop();
        dv->SetTransform(D3DTS_PROJECTION, rd->m_matProj->GetTop()); 

        dwAccum /= (int)((maxy-miny) * (maxx-minx));
        
        //bVis = dwAccum > 10 ? true : false;
        fIntens = min(1.0f, ((float)dwAccum/190.0f));
        obj->m_AmbColor[0] = obj->m_AmbColor[1];
        obj->m_AmbColor[1] = obj->m_AmbColor[2];
        obj->m_AmbColor[2] = obj->m_TempVars[3];
        obj->m_TempVars[3] = obj->m_Angs2[0];
        obj->m_Angs2[0] = obj->m_Angs2[1];
        obj->m_Angs2[1] = obj->m_Angs2[2];
        obj->m_Angs2[2] = obj->m_TempVars[4];
        obj->m_TempVars[4] = fIntens;
      }
      else
      {
        CREOcclusionQuery *pRE = (CREOcclusionQuery *)obj->m_RE;
        if (!pRE || m_nFrameQuery != rd->m_nFrameReset || !pRE->m_nOcclusionID)
        {
          m_nFrameQuery = rd->m_nFrameReset;
          obj->m_RE = NULL;
          // Create visibility queries
          LPDIRECT3DQUERY9  pVizQuery = NULL;
          hr = dv->CreateQuery (D3DQUERYTYPE_OCCLUSION, &pVizQuery);
          if (pVizQuery)
          {
            obj->m_RE = rd->EF_CreateRE(eDATA_OcclusionQuery);
            pRE = (CREOcclusionQuery *)obj->m_RE;
            if (pRE)
            {
              pRE->m_nOcclusionID = (UINT_PTR)pVizQuery;
              pRE->m_nVisSamples = 0;
            }
          }
        }
        else
        {
          CREOcclusionQuery *pRE = (CREOcclusionQuery *)obj->m_RE;
          int nFrame = rd->GetFrameID();

          if ( pRE->m_nCheckFrame != nFrame )
          {
		        if(pRE->m_nCheckFrame)
		        {
			        pRE->m_nVisSamples=0;
              // Stupidly block until we have a query result
              LPDIRECT3DQUERY9  pVizQuery = (LPDIRECT3DQUERY9)pRE->m_nOcclusionID;
              if (pVizQuery)
              {
                double time = sCycles2();
                bool bInfinite = false;
                while (pVizQuery->GetData((void *) &pRE->m_nVisSamples, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE)
                {
                  double dif = sCycles2()+34-time;
                  if (dif*1000.0*g_SecondsPerCycle > 5000.0)
                  {
                    // 5 seconds in the loop
                    bInfinite = true;
                    break;
                  }
                }
                rd->m_RP.m_PS.m_fOcclusionTime += (float)((sCycles2()+34-time)*1000.0*g_SecondsPerCycle);
                if (bInfinite)
                  iLog->Log("Error: Seems like infinite loop in flare occlusion query");
              }
            }
            pRE->m_nCheckFrame = nFrame;
          }
        }
        pRE = (CREOcclusionQuery *)obj->m_RE;
        LPDIRECT3DQUERY9  pVizQuery = NULL;
        if (pRE)
          pVizQuery = (LPDIRECT3DQUERY9)pRE->m_nOcclusionID;
        if (pVizQuery)
        {
          Vec3d vx0, vy0, v;
          v = or - rd->m_prevCamera.GetPos();
          float dist = v.Length();
          if (m_fDistSizeFactor != 1.0f)
            dist = cry_powf(dist, m_fDistSizeFactor);
          float fScaleCorona = m_fScaleCorona;
          if (obj->m_pLight && obj->m_pLight->m_fCoronaScale)
            fScaleCorona *= obj->m_pLight->m_fCoronaScale;
          dist *= fScaleCorona * CRenderer::CV_r_coronasizescale;
          vx0 = camVecs[1] * dist * 0.1f * m_fVisAreaScale;
          vy0 = camVecs[2] * dist * 0.1f * m_fVisAreaScale;
          rd->EF_SetState(GS_NOCOLMASK);
          CD3D9TexMan::BindNULL(1);
          rd->EF_SelectTMU(0);
          UCol col;
          col.dcolor = -1;
          int nOffs;
          struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)rd->GetVBPtr3D(4, nOffs);

          vQuad[0].xyz = or + vx0 + vy0;
          vQuad[0].st[0] = 0.0f; vQuad[0].st[1] = 0.0f;
          vQuad[0].color = col;

          vQuad[1].xyz = or + vx0 - vy0;
          vQuad[1].st[0] = 0.0f; vQuad[1].st[1] = 1.0f;
          vQuad[1].color = col;

          vQuad[2].xyz = or - vx0 - vy0;
          vQuad[2].st[0] = 1.0f; vQuad[2].st[1] = 1.0f;
          vQuad[2].color = col;

          vQuad[3].xyz = or - vx0 + vy0;
          vQuad[3].st[0] = 1.0f; vQuad[3].st[1] = 0.0f;
          vQuad[3].color = col;

          rd->UnlockVB3D();
          pVizQuery->Issue (D3DISSUE_BEGIN);
          // Bind our vertex as the first data stream of our device
          dv->SetStreamSource(0, rd->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

          rd->EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

          // Render the 2 triangles from the data stream
          dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);
          pVizQuery->Issue (D3DISSUE_END);
          //bVis = pRE->m_nVisSamples > 10 ? true : false;

          Vec3 ProjV[4];
          for (int n=0; n<4; n++)
          {
            D3DXVec3Project((D3DXVECTOR3 *)&ProjV[n], (D3DXVECTOR3 *)&vQuad[n].xyz, &D3DVP, (D3DXMATRIX *)projMatr.GetData(), (D3DXMATRIX *)camMatr.GetData(), &mIdent);
          }
          float nX = fabsf(ProjV[1].x - ProjV[0].x);
          float nY = fabsf(ProjV[2].y - ProjV[0].y);
          float area = nX * nY; //(float)(gRenDev->GetWidth() * gRenDev->GetHeight());
          fIntens = (float)pRE->m_nVisSamples / area;
          fIntens *= fFade;
          if (fIntens < 0.05f)
            fIntens = 0;
          else
          if (bRays)
            fIntens = max(0.75f, fIntens);
          obj->m_AmbColor[0] = obj->m_AmbColor[1];
          obj->m_AmbColor[1] = obj->m_AmbColor[2];
          obj->m_AmbColor[2] = obj->m_TempVars[3];
          obj->m_TempVars[3] = obj->m_Angs2[0];
          obj->m_Angs2[0] = obj->m_Angs2[1];
          obj->m_Angs2[1] = obj->m_Angs2[2];
          obj->m_Angs2[2] = obj->m_TempVars[4];
          obj->m_TempVars[4] = fIntens;
        }
      }
    }
    else
    {
      obj->m_AmbColor[0] = obj->m_AmbColor[1];
      obj->m_AmbColor[1] = obj->m_AmbColor[2];
      obj->m_AmbColor[2] = obj->m_TempVars[3];
      obj->m_TempVars[3] = obj->m_Angs2[0];
      obj->m_Angs2[0] = obj->m_Angs2[1];
      obj->m_Angs2[1] = obj->m_Angs2[2];
      obj->m_Angs2[2] = obj->m_TempVars[4];
      obj->m_TempVars[4] = 0;
    }
    if (fIntens >= 0.05f && bRays)
    {
      int sizeMask  = 64;
      int sizeMask2  = sizeMask/2;

      if ((rd->m_RP.m_PersFlags & RBPF_HDR) && rd->m_nHDRType == 1)
        hr = dv->SetRenderTarget(0, rd->mfGetBackSurface());

      int sizeTex  = 128;//m_Map->GetWidth();
      if (!obj->m_TexId0)
      {
        byte *data = new byte[sizeMask*sizeMask*4];
        char name[128];
        sprintf(name, "$AutoCoronas_%d", gcpRendD3D->m_TexGenID++);
        STexPic *tp = rd->m_TexMan->CreateTexture(name, sizeMask, sizeMask, 1, FT_NOMIPS | FT_ALLOCATED | FT_HASALPHA, FT2_NODXT | FT2_RENDERTARGET | FT_CLAMP, data, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
        delete [] data;
        obj->m_TexId0 = tp->m_Id;
      }
      if (!obj->m_TexId1)
      {
        byte *data = new byte[sizeTex*sizeTex*4];
        char name[128];
        sprintf(name, "$AutoCoronas_%d", gcpRendD3D->m_TexGenID++);
        STexPic *tp = rd->m_TexMan->CreateTexture(name, sizeTex, sizeTex, 1, FT_NOMIPS | FT_ALLOCATED | FT_HASALPHA, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
        delete [] data;
        obj->m_TexId1 = tp->m_Id;
      }

      rd->EF_PushFog();
      rd->EnableFog(false);

      rd->m_matProj->Push();
      D3DXMatrixOrthoRH(rd->m_matProj->GetTop(), (float)rd->GetWidth(), (float)rd->GetHeight(), -20.0, 0.0);
      dv->SetTransform(D3DTS_PROJECTION, rd->m_matProj->GetTop()); 

      rd->PushMatrix();
      rd->m_matView->LoadIdentity();
      dv->SetTransform(D3DTS_VIEW, rd->m_matView->GetTop()); 

      rd->SetCullMode(R_CULL_NONE);
      rd->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

      gRenDev->m_TexMan->m_Text_White->Set();

      float fsizeMask2 = (float)sizeMask2;

      // set alpha to 0
      {
        rd->EF_SetState(GS_NODEPTHTEST);
        rd->DrawQuad(vScr.x-fsizeMask2, vScr.y-fsizeMask2, vScr.x+fsizeMask2, vScr.y+fsizeMask2, CFColor(0.0f));
      }

      // Set alpha mask of visible areas (1 is visible)
      {
        rd->EF_SetState(0);
        rd->DrawQuad(vScr.x-fsizeMask2, vScr.y-fsizeMask2, vScr.x+fsizeMask2, vScr.y+fsizeMask2, CFColor(1.0f));
      }

      // Read z-mask to target texture 0 as source for bluring
      RECT rc = {max((int)vScr.x-sizeMask2+1, vp[0]), max((int)vScr.y-sizeMask2+1,vp[1]), min((int)vScr.x+sizeMask2,vp[2]-1), min((int)vScr.y+sizeMask2,vp[3]-1)};
      if((rc.right - rc.left) > 0 && rc.bottom - rc.top >0)
      {
        STexPic *t = (STexPic *)rd->m_TexMan->m_Textures[obj->m_TexId0];
        IDirect3DTexture9 * pID3DTexture = (IDirect3DTexture9 * )t->m_RefTex.m_VidTex;
        if(pID3DTexture)
        {
          IDirect3DSurface9 * pTexSurf;
          pID3DTexture->GetSurfaceLevel(0, &pTexSurf);
          IDirect3DSurface9 * pTar = rd->mfGetBackSurface();
          dv->StretchRect(pTar, &rc, pTexSurf, NULL, D3DTEXF_NONE);
          pTexSurf->Release();
        }
      }

      // Scale factor
      obj->m_TempVars[1] = 8.0f;
      int nPasses = 32;
      float fSizeTex = (float)sizeTex;
      // Result blured texture size
      obj->m_TempVars[2] = fSizeTex;
      float fScale = fScaleCorona * obj->m_TempVars[1];
      float fDif = fSizeTex / (float)sizeMask;
      float fSizeTex2 = fSizeTex / 2.0f;
      float fLast = fSizeTex2 / (fScale * fDif);
      float fCur = fSizeTex2 * 2;
      float fDelt = (fLast - fCur) / (float)nPasses;
      float fCurAlpha = 0.0f;
      float fDeltAlpha = 1.0f / (float)nPasses;
      float fPosx = (float)(rd->GetWidth()/2);
      float fPosy = (float)(rd->GetHeight()/2);

      rd->EF_SetState(GS_COLMASKONLYALPHA | GS_NODEPTHTEST);
      gRenDev->m_TexMan->m_Text_White->Set();
      
      rd->DrawQuad(fPosx-fSizeTex2, fPosy-fSizeTex2, fPosx+fSizeTex2, fPosy+fSizeTex2, CFColor(0.2f));

      //gRenDev->SetTexture(obj->m_TexId0, eTT_Base);
      STexPic *t = (STexPic *)rd->m_TexMan->m_Textures[obj->m_TexId0];
      rd->SetTexture(t->m_Bind, eTT_Base);

      // Radially blur the alpha texture
      for (int pass=0; pass<nPasses; pass++)
      {
        rd->EF_SetState(GS_COLMASKONLYALPHA | GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
        
        // for inverse color
        //gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

        rd->DrawQuad(fPosx-fCur, fPosy-fCur, fPosx+fCur, fPosy+fCur, CFColor(fDeltAlpha));

        fCur += fDelt;
        fCurAlpha += fDeltAlpha;
      }

      // Generate blured alpha texture without mip-maps
      {
        RECT rc = {max((int)(fPosx-fSizeTex2)+1, vp[0]), max((int)(fPosy-fSizeTex2)+1,vp[1]), min((int)(fPosx+fSizeTex2),vp[2]-1), min((int)(fPosy+fSizeTex2),vp[3]-1)};
        if((rc.right - rc.left) > 0 && rc.bottom - rc.top >0)
        {
          STexPic *t = (STexPic *)rd->m_TexMan->m_Textures[obj->m_TexId1];
          IDirect3DTexture9 * pID3DTexture = (IDirect3DTexture9 * )t->m_RefTex.m_VidTex;
          if(pID3DTexture)
          {
            IDirect3DSurface9 * pTexSurf;
            pID3DTexture->GetSurfaceLevel(0, &pTexSurf);
            IDirect3DSurface9 * pTar = rd->mfGetBackSurface();
            HRESULT hr = dv->StretchRect(pTar, &rc, pTexSurf, NULL, D3DTEXF_NONE);
            pTexSurf->Release();
          }
        }
      }
      rd->PopMatrix();
      rd->m_matProj->Pop();
      dv->SetTransform(D3DTS_PROJECTION, rd->m_matProj->GetTop()); 

      rd->EF_PopFog();
      rd->SetTexture(0, eTT_Base);

      if ((rd->m_RP.m_PersFlags & RBPF_HDR) && rd->m_nHDRType == 1)
        hr = dv->SetRenderTarget(0, rd->m_pHDRTargetSurf);
    }
    dv->SetTransform(D3DTS_VIEW, rd->m_matView->GetTop()); 
    dv->SetTransform(D3DTS_PROJECTION, rd->m_matProj->GetTop()); 
  }  

  return true;
}

void CREFlare::mfDrawCorona(SShader *ef, CFColor &col)
{
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->mfGetD3DDevice();
  Vec3d vx0, vy0, vx1, vy1, or, v, norm;

  if (m_Importance > CRenderer::CV_r_coronas)
    return;
  
  CCObject *obj = rd->m_RP.m_pCurObject;
  or = obj->GetTranslation();

  bool bSun = false;
  if (obj->m_ObjFlags & FOB_DRSUN)
  {
    bSun = true;
    //or.Normalize();
    //or *= 500.0f;
    //or += gRenDev->m_RP.m_ViewOrg;
  }
  if (bSun && rd->m_bHeatVision)
    return;
  
  vx0 = rd->m_RP.m_CamVecs[1];
  vx1 = vx0;
  vy0 = rd->m_RP.m_CamVecs[2];
  vy1 = vy0;

  v = or - rd->m_RP.m_ViewOrg;
  //Vec3d cn = v.normalize();
  //or = cn * 512;
  float dist = v.Length();
  float distX = dist;
  if (m_fDistSizeFactor != 1.0f)
    distX = cry_powf(distX, m_fDistSizeFactor);
  float distY = distX;

  float fScaleCorona = m_fScaleCorona;
  if (m_pScaleCoronaParams)
    fScaleCorona = m_pScaleCoronaParams->mfGet();
  if (obj->m_pLight && obj->m_pLight->m_fCoronaScale)
    fScaleCorona *= obj->m_pLight->m_fCoronaScale;

  if (obj->m_TempVars[2])
  {
    distX *= fScaleCorona * (obj->m_TempVars[2]/128.0f) * CRenderer::CV_r_coronasizescale;
    distY *= fScaleCorona * (obj->m_TempVars[2]/128.0f) * CRenderer::CV_r_coronasizescale;
  }
  else
  {
    distX *= fScaleCorona * CRenderer::CV_r_coronasizescale;
    distY *= fScaleCorona * CRenderer::CV_r_coronasizescale;
  }
  if (rd->m_RP.m_PersFlags & RBPF_HDR)
  {
    //distX *= 0.25f;
    //distY *= 0.25f;
  }

  float fDecay = col[3];
  fDecay *= CRenderer::CV_r_coronacolorscale;
  norm = rd->m_RP.m_CamVecs[0];
  bool bBlind = false;
  if (m_bBlind && obj->m_pLight)
  {
    CDLight *dl = obj->m_pLight;
    if (dl->m_Flags & DLF_PROJECT)
    {
      bBlind = true;
      v.Normalize();
      float fDot = -(v * dl->m_Orientation.m_vForward);
      if (fDot < 0.0f)
        return;
      float fBlindSize = fDot * m_fSizeBlindScale + m_fSizeBlindBias;
      float fBlindIntens = fDot * m_fIntensBlindScale + m_fIntensBlindBias;
      fDecay *= fBlindIntens;
      distX *= fBlindSize;
      distY *= fBlindSize;

      /*
      vx0 += distX * dl->m_Orientation.m_vForward;
      vy0 += distY * dl->m_Orientation.m_vForward;
      vx1 += distX * -dl->m_Orientation.m_vForward;
      vy1 += distY * -dl->m_Orientation.m_vForward;
      */

      vx0 *= distX;// * dl->m_Orientation.m_vForward;
      vy0 *= distY;// * dl->m_Orientation.m_vForward;
      vx1 *= -distX;// * -dl->m_Orientation.m_vForward;
      vy1 *= -distY;// * -dl->m_Orientation.m_vForward;

      float fDot1 = (rd->m_RP.m_CamVecs[1] * dl->m_Orientation.m_vForward);
      float k=0.1f;
      fDot1 = fDot1*(1.0f-k) + k;
      vx0 *= fDot1+1.0f;
      vx1 *= 1.0f - fDot1 + k;

      float fDot2 = (rd->m_RP.m_CamVecs[2] * dl->m_Orientation.m_vForward);
      fDot2 = fDot2*(1.0f-k) + k;
      vy0 *= fDot2+1.0f;
      vy1 *= 1.0f - fDot2 + k;
    }
  }
  if (!bBlind)
  {
    vx0 *= distX;
    vy0 *= distY;
    vx1 *= -distX;
    vy1 *= -distY;
  }
  if (m_fDistIntensityFactor != 1.0f)
    fDecay = fDecay * cry_powf(dist, m_fDistIntensityFactor);

  if (fDecay <= 0.001f)
    return;
  else
  if (fDecay > 1.0f)
    fDecay = 1.0f;

  CD3D9TexMan::BindNULL(1);
  rd->EF_SelectTMU(0);
  rd->D3DSetCull(eCULL_None);
  rd->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  if (rd->m_RP.m_TexStages[0].TCIndex != 0)
  {
    rd->m_RP.m_TexStages[0].TCIndex = 0;
    dv->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
  }

  CFColor c;
  float fMax = max(col.r, max(col.b, col.g));
  if (fMax > 1.0f)
    col.NormalizeCol(c);
  else
    c = col;

  if (m_Pass)
  {
    int St = m_Pass->m_RenderState & GS_BLEND_MASK;
    if (St == (GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA))
    {
      c[3] = fDecay;
    }
    else
    {
      c[0] = c[0] * fDecay;
      c[1] = c[1] * fDecay;
      c[2] = c[2] * fDecay;
    }
  }
  else
  {
    if (bSun)
    {
      c[0] = c[0] * fDecay;
      c[1] = c[1] * fDecay;
      c[2] = c[2] * fDecay;
      c[3] = fDecay;
    }
    else
    {
      c[0] = c[0] * fDecay;
      c[1] = c[1] * fDecay;
      c[2] = c[2] * fDecay;
    }
  }

  if (CRenderer::CV_r_SunStyleCoronas)
    bSun = true;
  bool bRays = CRenderer::CV_r_checkSunVis == 2 && bSun;

  rd->EF_PushFog();
  rd->EnableFog(false);

  CCGPShader_D3D *fpDrawFlare = NULL;
  if ((rd->m_RP.m_PersFlags & RBPF_HDR) && rd->m_nHDRType == 1)
  {
    if (!rd->m_RP.m_PS_HDR_DrawFlare)
      rd->m_RP.m_PS_HDR_DrawFlare = CPShader::mfForName("CGRC_HDR_DrawFlare_PS20");
    fpDrawFlare = (CCGPShader_D3D *)rd->m_RP.m_PS_HDR_DrawFlare;
    if (fpDrawFlare)
      fpDrawFlare->mfSet(true, 0);
  }

  //if (CRenderer::CV_r_checkSunVis == 1 || CRenderer::CV_r_checkSunVis == 3 || !bSun || obj->m_TexId1<=0)
  {
    if (bRays)
      rd->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE);
    else
      rd->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
    if (m_Map)
      m_Map->Set();
    int nOffs;
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)rd->GetVBPtr3D(10, nOffs);
    if (vQuad)
    {
      DWORD col = D3DRGBA(c.r, c.g, c.b, c.a);

      vQuad[0].xyz = or;
      vQuad[0].st[0] = 0.5f; vQuad[0].st[1] = 0.5f;
      vQuad[0].color.dcolor = col;

      vQuad[1].xyz = or + vx0 + vy0;
      vQuad[1].st[0] = 0.0f; vQuad[1].st[1] = 0.0f;
      vQuad[1].color.dcolor = col;

      vQuad[2].xyz = or + vx0;
      vQuad[2].st[0] = 0.0f; vQuad[2].st[1] = 0.5f;
      vQuad[2].color.dcolor = col;

      vQuad[3].xyz = or + vx0 + vy1;
      vQuad[3].st[0] = 0.0f; vQuad[3].st[1] = 1.0f;
      vQuad[3].color.dcolor = col;

      vQuad[4].xyz = or + vy1;
      vQuad[4].st[0] = 0.5f; vQuad[4].st[1] = 1.0f;
      vQuad[4].color.dcolor = col;

      vQuad[5].xyz = or + vx1 + vy1;
      vQuad[5].st[0] = 1.0f; vQuad[5].st[1] = 1.0f;
      vQuad[5].color.dcolor = col;

      vQuad[6].xyz = or + vx1;
      vQuad[6].st[0] = 1.0f; vQuad[6].st[1] = 0.5f;
      vQuad[6].color.dcolor = col;

      vQuad[7].xyz = or + vx1 + vy0;
      vQuad[7].st[0] = 1.0f; vQuad[7].st[1] = 0.0f;
      vQuad[7].color.dcolor = col;

      vQuad[8].xyz = or + vy0;
      vQuad[8].st[0] = 0.5f; vQuad[8].st[1] = 0.0f;
      vQuad[8].color.dcolor = col;

      vQuad[9].xyz = or + vx0 + vy0;
      vQuad[9].st[0] = 0.0f; vQuad[9].st[1] = 0.0f;
      vQuad[9].color.dcolor = col;

      rd->UnlockVB3D();
      // Bind our vertex as the first data stream of our device
      dv->SetStreamSource(0, gcpRendD3D->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
      rd->EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

      // Render the 8 triangles from the data stream
      if (m_Pass && m_Pass->m_TUnits.Num())
      {
        CVProgram *curVP = NULL;
        CVProgram *newVP;
        SShaderPassHW *slw = m_Pass;
        newVP = slw->m_VProgram;
        if (newVP)
          rd->m_RP.m_FlagsPerFlush |= RBSI_USEVP;
        if (slw->mfSetTextures())
        {
          // Set vertex program for the current pass if needed
          if (newVP != curVP)
          {
            if (newVP)
            {
              curVP = newVP;
              curVP->mfSet(true, slw, VPF_DONTSETMATRICES);
            }
            else
              curVP = NULL;
          }

          rd->EF_ApplyMatrixOps(slw->m_MatrixOps, true);

          // Set Render states for the current pass
          int State;
          if (rd->m_RP.m_RendPass || (rd->m_RP.m_ObjFlags & FOB_LIGHTPASS))
            State = slw->m_SecondRenderState;
          else
            State = slw->m_RenderState;
          rd->EF_SetState(State);

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }
          else
          {
            rd->m_RP.m_CurrentVLights = 0;
            rd->m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
          }

          // Set Pixel shaders and Register combiners for the current pass
          if (slw->m_FShader)
            slw->m_FShader->mfSet(true, slw);
          else
            rd->m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;

          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
            rd->EF_CommitTexStageState();

          rd->m_RP.m_RendPass++;

          int bFogOverrided = 0;
          rd->EF_CommitShadersState();
          if (rd->m_FS.m_bEnable)
            bFogOverrided = rd->EF_FogCorrection(false, false);

          {
            PROFILE_FRAME(Draw_ShaderIndexMesh);
            dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 8);
          }

          rd->EF_FogRestore(bFogOverrided);
        }

        slw->mfResetTextures();      
        rd->EF_ApplyMatrixOps(slw->m_MatrixOps, false);
      }
      else
        dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 8);
    }
  }
  if (fpDrawFlare)
    fpDrawFlare->mfSet(false, 0);

  //else
  if (bRays)
  {
    rd->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
    rd->EF_SelectTMU(0);
    rd->EnableTMU(true);  
    m_Map->Set();

    rd->EF_SelectTMU(1);
    rd->EnableTMU(true);
    STexPic *t = (STexPic *)rd->m_TexMan->m_Textures[obj->m_TexId1];
    rd->m_TexMan->SetTexture(t->m_Bind, eTT_Base);

    CPShader *pRC = PShaderForName(rd->m_RP.m_RCSun, "CGRCSun");
    if (pRC)
      pRC->mfSet(true);

    rd->m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;

    int nOffs;
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)rd->GetVBPtr3D(10, nOffs);
    DWORD col = D3DRGBA(c.r, c.g, c.b, c.a);

    vQuad[0].xyz = or;
    vQuad[0].st[1] = 0.5f; vQuad[0].st[0] = 0.5f;
    vQuad[0].color.dcolor = col;

    vQuad[1].xyz = or + vx0 + vy0;
    vQuad[1].st[1] = 0.0f; vQuad[1].st[0] = 1.0f;
    vQuad[1].color.dcolor = col;

    vQuad[2].xyz = or + vx0;
    vQuad[2].st[1] = 0.0f; vQuad[2].st[0] = 0.5f;
    vQuad[2].color.dcolor = col;

    vQuad[3].xyz = or + vx0 + vy1;
    vQuad[3].st[1] = 0.0f; vQuad[3].st[0] = 0.0f;
    vQuad[3].color.dcolor = col;

    vQuad[4].xyz = or + vy1;
    vQuad[4].st[1] = 0.5f; vQuad[4].st[0] = 0.0f;
    vQuad[4].color.dcolor = col;

    vQuad[5].xyz = or + vx1 + vy1;
    vQuad[5].st[1] = 1.0f; vQuad[5].st[0] = 0.0f;
    vQuad[5].color.dcolor = col;

    vQuad[6].xyz = or + vx1;
    vQuad[6].st[1] = 1.0f; vQuad[6].st[0] = 0.5f;
    vQuad[6].color.dcolor = col;

    vQuad[7].xyz = or + vx1 + vy0;
    vQuad[7].st[1] = 1.0f; vQuad[7].st[0] = 1.0f;
    vQuad[7].color.dcolor = col;

    vQuad[8].xyz = or + vy0;
    vQuad[8].st[1] = 0.5f; vQuad[8].st[0] = 1.0f;
    vQuad[8].color.dcolor = col;

    vQuad[9].xyz = or + vx0 + vy0;
    vQuad[9].st[1] = 0.0f; vQuad[9].st[0] = 1.0f;
    vQuad[9].color.dcolor = col;

    rd->UnlockVB3D();
    // Bind our vertex as the first data stream of our device
    dv->SetStreamSource(0, gcpRendD3D->m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

    rd->EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

    // Render the 8 triangles from the data stream
    dv->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
    rd->m_RP.m_TexStages[1].TCIndex = 0;
    dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 8);

    if (pRC)
      pRC->mfSet(false);
    rd->EnableTMU(false);  
    rd->EF_SelectTMU(0);
  }

  rd->m_nPolygons += 8;
  rd->EF_PopFog();
}

void CREFlare::mfDrawFlares(SShader *ef, CFColor &col)
{
  CSunFlares *lfl = ef->m_Flares;
  CD3D9Renderer *rd = gcpRendD3D;

  int i;
  SSunFlare *fl;
  Vec3d lv;
  
  if (!CRenderer::CV_r_flares || !lfl)
    return;
    
  CCObject *obj = rd->m_RP.m_pCurObject;

  Vec3d or = obj->GetTranslation();  
  if (obj->m_ObjFlags & FOB_DRSUN)
  {
    if (rd->m_bHeatVision)
      return;
  }

  Vec3d vFromPt = rd->m_RP.m_ViewOrg;
  Vec3d vViewVec = rd->m_RP.m_CamVecs[0];
  Vec3d vLightVec = or - vFromPt;
  vLightVec.Normalize();
  
  // Compute the vector and center point for the lens flare axis
  float fDot = vLightVec | vViewVec;
  if (fDot <= 0.2f)
    return;
  
  
  Vec3d vNewLightPt = vFromPt + 1.0f/fDot * vLightVec;
  Vec3d vCenterPt   = vFromPt + vViewVec;
  Vec3d vAxisVec    = vNewLightPt - vCenterPt;
  
  // Store the lens flares positions for each flare
  for (i=0; i<lfl->m_NumFlares; i++)
  {
    fl = &lfl->m_Flares[i];
    
    // Store the position of the flare along the axis
    fl->m_Position = vCenterPt + vAxisVec * fl->m_Loc;
    
    // Store the render size of the flare. This is the lens flare size
    // corrected for the orientation of the flaring axis.
    fl->m_RenderSize = fl->m_Scale;//fViewScale * fl->scale;
  }
  
  rd->EF_SelectTMU(0);
  rd->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
  rd->SetCullMode(R_CULL_NONE);
  rd->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  
  Vec3d VecX = rd->m_RP.m_CamVecs[1];
  Vec3d VecY = rd->m_RP.m_CamVecs[2];
  Vec3d v;

  fDot *= col.a;
  
  // Do the flares
  for (i=0; i<lfl->m_NumFlares; i++)
  {
    fl = &lfl->m_Flares[i];
    
    Vec3d vx = VecX * fl->m_RenderSize;
    Vec3d vy = VecY * fl->m_RenderSize;

    CFColor col = fl->m_Color * fDot;
    fl->m_Tex->Set();
    rd->DrawQuad3D(fl->m_Position+vx+vy, fl->m_Position-vx+vy, fl->m_Position-vx-vy, fl->m_Position+vx-vy, col);

    rd->m_nPolygons += 2;
  }
}

static float sInterpolate(float& pprev, float& prev, float& next, float& nnext, float ppweight, float pweight, float nweight, float nnweight)
{
  return pprev*ppweight + prev*pweight + next*nweight + nnext*nnweight;
}

static float sSpline(float x)
{
  float fX = fabsf(x);

  if(fX > 2.0f)
    return 0;
  if(fX > 1.0f)
    return (2.0f-fX)*(2.0f-fX)*(2.0f-fX)/6.0f;
  return 2.0f/3.0f-fX*fX+0.5f*fX*fX*fX;
}

static float *sObjBright(float fTime, CCObject *obj)
{
  if (fTime <= obj->m_Angs2[0])
    return &obj->m_AmbColor[0];
  if (fTime <= obj->m_Angs2[1])
    return &obj->m_AmbColor[1];
  if (fTime <= obj->m_Angs2[2])
    return &obj->m_AmbColor[2];
  return &obj->m_fHeatFactor;
}

bool CREFlare::mfDraw(SShader *ef, SShaderPass *sfm)
{
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->mfGetD3DDevice();
  if (!CRenderer::CV_r_flares && m_Importance > CRenderer::CV_r_coronas)
    return false;

  dv->SetTransform(D3DTS_VIEW, (D3DMATRIX *)rd->m_CameraMatrix.GetData());

  CCObject *obj = rd->m_RP.m_pCurObject;
  if ((obj->m_ObjFlags & FOB_DRSUN) && (rd->m_RP.m_PersFlags & RBPF_DONTDRAWSUN))
    return false;

  float fBrightness = (obj->m_AmbColor[0]+obj->m_AmbColor[1]+obj->m_AmbColor[2]+
                       obj->m_Angs2[0]+obj->m_Angs2[1]+obj->m_Angs2[2]+
                       obj->m_TempVars[3]+obj->m_TempVars[4]) * 0.125f;

  if (!fBrightness)
    return false;

  obj->m_Color.r = m_Color.r;
  obj->m_Color.g = m_Color.g;
  obj->m_Color.b = m_Color.b;
  obj->m_Color.a = fBrightness; //ft;
  mfDrawCorona(ef, obj->m_Color);
  mfDrawFlares(ef, obj->m_Color);

  return true;
}

void CLeafBuffer::DrawImmediately()
{
}

bool CREClearStencil::mfDraw(SShader *ef, SShaderPass *sfm)
{
  gcpRendD3D->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 1.0f, 0);
  return true;
}

bool CREHDRProcess::mfDraw(SShader *ef, SShaderPass *sfm)
{
  assert (gcpRendD3D->m_RP.m_PersFlags & RBPF_HDR);
  assert(gcpRendD3D->m_pHDRTargetSurf);
  if (!(gcpRendD3D->m_RP.m_PersFlags & RBPF_HDR))
    return false;
  if (!gcpRendD3D->m_pHDRTargetSurf)
    return false;
  gcpRendD3D->EF_HDRPostProcessing();
  return true;
}