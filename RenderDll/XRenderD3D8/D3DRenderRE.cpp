/*=============================================================================
  D3DRenderRE.cpp : implementation of the Rendering RenderElements pipeline.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"
//#include "../cry3dengine/CryAnimation.h"

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

  m_arrVerts[0].x = (float)w;
  m_arrVerts[0].y = 0;
  m_arrVerts[0].z = 0;
  m_arrVerts[0].s = 1;
  m_arrVerts[0].t = 0;

  m_arrVerts[1].x = 0;
  m_arrVerts[1].y = 0;
  m_arrVerts[1].z = 0;
  m_arrVerts[1].s = 0;
  m_arrVerts[1].t = 0;

  m_arrVerts[2].x = (float)w;
  m_arrVerts[2].y = (float)h;
  m_arrVerts[2].z = 0;
  m_arrVerts[2].s = 1;
  m_arrVerts[2].t = 1;

  m_arrVerts[3].x = 0;
  m_arrVerts[3].y = (float)h;
  m_arrVerts[3].z = 0;
  m_arrVerts[3].s = 0;
  m_arrVerts[3].t = 1;
}

void CREFlareGeom::mfCheckVis(CFColor &col, CCObject *obj)
{
  float Depth[3][3];
  float ft;
  float f;

  CD3D8Renderer *rd = gcpRendD3D;
  int re = 0;//rd->mfGetRecurseLevel();
  SFlareFrame *ff = &mFlareFr[re];

  LPDIRECT3DDEVICE8 pd3dDevice = rd->mfGetD3DDevice();
  IDirect3DSurface8 *ZSurf = rd->mfGetZSurface();
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
  HRESULT ddrval = ZSurf->LockRect( &rc, NULL, D3DLOCK_READONLY );
  if( ddrval!=D3D_OK )
  {
    iLog->Log("D3D Driver: Lock on Zbuffer failed (%s)\n", rd->D3DError(ddrval));
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

bool CREOcLeaf::mfPreDraw(SShaderPass *sl)
{
  CLeafBuffer *lb = m_pBuffer->GetVertexContainer();
  for (int i=0; i<VSF_NUM; i++)
  {
    if (lb->m_pVertexBuffer->m_VS[i].m_bLocked)
    {
      gcpRendD3D->UnlockBuffer(lb->m_pVertexBuffer, i);
      lb->m_pVertexBuffer->m_VS[i].m_bLocked = false;
    }
  }
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;

  h = dv->SetStreamSource( 0, (IDirect3DVertexBuffer8 *)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr, m_VertexSize[lb->m_pVertexBuffer->m_vertexformat]);

  if (gRenDev->m_RP.m_FlagsModificators & RBMF_TANGENTSUSED)
  {
    gRenDev->m_RP.m_PersFlags |= RBPF_USESTREAM1;
    h = dv->SetStreamSource( 1, (IDirect3DVertexBuffer8 *)lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr, sizeof(SPipTangents));
  }
  else
  if (gRenDev->m_RP.m_PersFlags & RBPF_USESTREAM1)
  {
    gRenDev->m_RP.m_PersFlags &= ~RBPF_USESTREAM1;
    h = dv->SetStreamSource( 1, NULL, 0);
  }

  if (gRenDev->m_RP.m_FlagsModificators & RBMF_LMTCUSED)
  {
    gRenDev->m_RP.m_PersFlags |= RBPF_USESTREAM2;
    if (gRenDev->m_RP.m_pCurObject->m_pLMTCBufferO && gRenDev->m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer)
      h = dv->SetStreamSource( 2, (IDirect3DVertexBuffer8 *)gRenDev->m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr, sizeof(Vec3d));
    else
      h = dv->SetStreamSource( 2, (IDirect3DVertexBuffer8 *)lb->m_pVertexBuffer->m_VS[VSF_LMTC].m_VertBuf.m_pPtr, sizeof(SMRendTexVert));
  }
  else
  if (gRenDev->m_RP.m_PersFlags & RBPF_USESTREAM2)
  {
    gRenDev->m_RP.m_PersFlags &= ~RBPF_USESTREAM2;
    h = dv->SetStreamSource( 2, NULL, 0);
  }

  IDirect3DIndexBuffer8 *pIndBuf = (IDirect3DIndexBuffer8 *)lb->m_Indices.m_VertBuf.m_pPtr;
  assert(pIndBuf);
  h = dv->SetIndices(pIndBuf, 0);

  return true;
}

bool CRETempMesh::mfPreDraw(SShaderPass *sl)
{
  CVertexBuffer *vb = m_VBuffer;

  for (int i=0; i<VSF_NUM; i++)
  {
    if (vb->m_VS[i].m_bLocked)
    {
      gcpRendD3D->UnlockBuffer(vb, i);
      vb->m_VS[i].m_bLocked = false;
    }
  }

  if (!m_Inds.m_nItems)
    return false;

  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;

  h = dv->SetStreamSource( 0, (IDirect3DVertexBuffer8 *)vb->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr, m_VertexSize[vb->m_vertexformat]);

  if (gRenDev->m_RP.m_FlagsModificators & RBMF_TANGENTSUSED)
  {
    gRenDev->m_RP.m_PersFlags |= RBPF_USESTREAM1;
    h = dv->SetStreamSource( 1, (IDirect3DVertexBuffer8 *)vb->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr, sizeof(SPipTangents));
  }
  else
  if (gRenDev->m_RP.m_PersFlags & RBPF_USESTREAM1)
  {
    gRenDev->m_RP.m_PersFlags &= ~RBPF_USESTREAM1;
    h = dv->SetStreamSource( 1, NULL, 0);
  }

  if (gRenDev->m_RP.m_FlagsModificators & RBMF_LMTCUSED)
  {
    gRenDev->m_RP.m_PersFlags |= RBPF_USESTREAM2;
    h = dv->SetStreamSource( 2, (IDirect3DVertexBuffer8 *)vb->m_VS[VSF_LMTC].m_VertBuf.m_pPtr, sizeof(SMRendTexVert));
  }
  else
  if (gRenDev->m_RP.m_PersFlags & RBPF_USESTREAM2)
  {
    gRenDev->m_RP.m_PersFlags &= ~RBPF_USESTREAM2;
    h = dv->SetStreamSource( 2, NULL, 0);
  }

  h = dv->SetIndices((IDirect3DIndexBuffer8 *)m_Inds.m_VertBuf.m_pPtr, 0);

  return true;
}

CREOcclusionQuery::~CREOcclusionQuery()
{
  int i;

  for (i=0; i<8; i++)
  {
  }
}

bool CREOcclusionQuery::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  return true;
}

bool CREFlashBang::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  // Drawing of Flashbang texture over the screen
  // FlashBang texture is already set.
  return true;
}

bool CRETempMesh::mfDraw(SShader *ef, SShaderPass *sl)
{
  CD3D8Renderer *r = gcpRendD3D;
  CVertexBuffer *vb = m_VBuffer;
  if (!vb)
    return false;

  // Hardware effector
  int nInds = m_Inds.m_nItems;
  int nPrimType = R_PRIMV_TRIANGLES;
  if (nInds)
  {
    r->m_RP.m_RendNumIndices = nInds;
    r->m_RP.m_RendNumVerts = vb->m_NumVerts;
    r->EF_DrawIndexedMesh(nPrimType);
  }
  vb->m_bFenceSet = true;

  return true;
}

bool CREOcLeaf::mfDraw(SShader *ef, SShaderPass *sl)
{
  CD3D8Renderer *r = gcpRendD3D;
  
  CLeafBuffer *lb = m_pBuffer;
  
  // Hardware effector
  if (ef->m_HWTechniques.Num())
  {
    r->EF_DrawIndexedMesh(lb->m_nPrimetiveType);    
    return true;
  }
  
  r->DrawBuffer(lb->m_pVertexBuffer, &lb->m_Indices, m_pChunk->nNumIndices, m_pChunk->nFirstIndexId, lb->m_nPrimetiveType, m_pChunk->nFirstVertId,m_pChunk->nNumVerts);
  
  return (true);
}

void CREClientPoly2D::mfLock()
{
  CRenderer *r = gRenDev;
  uint Start;
  r->m_RP.m_Ptr.Ptr_D_1T = r->m_RP.m_VB.VBPtr_D_1T->Lock(r->m_RP.m_MaxVerts, Start);
  r->m_RP.m_RendIndices = r->m_RP.m_IndexBuf->Lock(r->m_RP.m_MaxTris*3, Start);
  r->m_RP.m_bLocked = true;  
  r->m_RP.m_NextPtr = r->m_RP.m_Ptr;
}

void CREClientPoly::mfLock()
{
  CRenderer *r = gRenDev;
  uint Start;
  r->m_RP.m_Ptr.Ptr_D_1T = r->m_RP.m_VB.VBPtr_D_1T->Lock(r->m_RP.m_MaxVerts, Start);
  r->m_RP.m_RendIndices = r->m_RP.m_IndexBuf->Lock(r->m_RP.m_MaxTris*3, Start);
  r->m_RP.m_bLocked = true;  
  r->m_RP.m_NextPtr = r->m_RP.m_Ptr;
}

void CREPolyMesh::mfLock()
{
  CRenderer *r = gRenDev;
  uint Start;
  r->m_RP.m_Ptr.Ptr_D_1T = r->m_RP.m_VB.VBPtr_D_1T->Lock(r->m_RP.m_MaxVerts, Start);
  r->m_RP.m_RendIndices = r->m_RP.m_IndexBuf->Lock(r->m_RP.m_MaxTris*3, Start);
  r->m_RP.m_bLocked = true;
  r->m_RP.m_NextPtr = r->m_RP.m_Ptr;
}


#define GLARE_OFS 1.0/32

void GlareQuad(const CFColor & color, CD3D8Renderer * gcpRendD3D)
{
  /*
	glBegin (GL_QUADS);
	  glTexCoord2f(-1, 1);
	  glVertex3f (0.1f, 1, 1);
	  glTexCoord2f(0, 1);
	  glVertex3f (0.1f, -1, 1);
	  glTexCoord2f(0, 0);
	  glVertex3f (0.1f, -1, -1);
	  glTexCoord2f(-1, 0);
	  glVertex3f (0.1f, 1, -1);
	glEnd ();
  */
  gcpRendD3D->DrawQuad3D(Vec3d(0.1f, 1, 1), Vec3d(0.1f, -1, 1), Vec3d(0.1f, -1, -1), Vec3d(0.1f, 1, -1),
             color,  -1,1,  0,0);
}


bool CREGlare::mfDraw(SShader *ef, SShaderPass *sfm)
{
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();

  CTexMan::m_Text_Glare->Set();
  //gcpOGL->m_Text_Glare->SaveJPG("Glare.jpg", false);

  //glDisableClientState(GL_COLOR_ARRAY);
  //gcpOGL->EF_SetColorOp(eCO_MODULATE);
  gRenDev->SetEnviMode(R_MODE_MODULATE);
  gRenDev->SetState(GS_NODEPTHTEST | GS_BLSRC_ONE | GS_BLDST_ONE);
  //glDisable(GL_CULL_FACE);
  gRenDev->SetCullMode(R_CULL_NONE);

  //glPushMatrix();
  gcpRendD3D->PushMatrix();
  //glLoadIdentity ();
  gcpRendD3D->m_matView->LoadIdentity();
  //glRotatef (-90,  1, 0, 0);	    // put Z going up
  gcpRendD3D->m_matView->RotateAxis(&D3DXVECTOR3(1,0,0), 3.141593f * (-90) / 180);
  //glRotatef (90,  0, 0, 1);	    // put Z going up
  gcpRendD3D->m_matView->RotateAxis(&D3DXVECTOR3(0,0,1), 3.141593f * (90) / 180);
  dv->SetTransform(D3DTS_VIEW, gcpRendD3D->m_matView->GetTop()); 

  //glMatrixMode(GL_PROJECTION);
  //glPushMatrix();
  gcpRendD3D->m_matProj->Push();
  //dv->SetTransform(D3DTS_VIEW, m_matProj->GetTop()); 
  //glLoadIdentity ();
  gcpRendD3D->m_matProj->LoadIdentity();
  dv->SetTransform(D3DTS_PROJECTION, gcpRendD3D->m_matProj->GetTop()); 

  //glMatrixMode(GL_MODELVIEW);

  CFColor col;

  if (CRenderer::CV_r_glare != 2)
  {
    //glColor4f (0.2f*CRenderer::CV_r_glareamount, 0.2f*CRenderer::CV_r_glareamount, 0.2f*CRenderer::CV_r_glareamount, 0.2f*CRenderer::CV_r_glareamount);
    col = CFColor(0.2f*CRenderer::CV_r_glareamount, 0.2f*CRenderer::CV_r_glareamount, 0.2f*CRenderer::CV_r_glareamount, 0.2f*CRenderer::CV_r_glareamount);

    //glPushMatrix();
    gRenDev->PushMatrix();
    //glTranslatef(0,0,GLARE_OFS);
    gRenDev->TranslateMatrix(0,0,GLARE_OFS);
    GlareQuad(col, gcpRendD3D);
    //glPopMatrix();
    gRenDev->PopMatrix();

    gRenDev->PushMatrix();
    //glTranslatef(0,0,-GLARE_OFS);
    gRenDev->TranslateMatrix(0,0,-GLARE_OFS);
    GlareQuad(col, gcpRendD3D);
    gRenDev->PopMatrix();

    gRenDev->PushMatrix();
    //glTranslatef(0,-GLARE_OFS,0);
    gRenDev->TranslateMatrix(0,-GLARE_OFS,0);
    GlareQuad(col, gcpRendD3D);
    gRenDev->PopMatrix();

    gRenDev->PushMatrix();
    //glTranslatef(0,GLARE_OFS,0);
    gRenDev->TranslateMatrix(0,GLARE_OFS,0);
    GlareQuad(col, gcpRendD3D);
    gRenDev->PopMatrix();
  }
  else
    //glColor4f (1.0f*CRenderer::CV_r_glareamount, 1.0f*CRenderer::CV_r_glareamount, 1.0f*CRenderer::CV_r_glareamount, 1.0f*CRenderer::CV_r_glareamount);
    col = CFColor(1.0f*CRenderer::CV_r_glareamount, 1.0f*CRenderer::CV_r_glareamount, 1.0f*CRenderer::CV_r_glareamount, 1.0f*CRenderer::CV_r_glareamount);
  
  GlareQuad(col, gcpRendD3D);

  //glMatrixMode(GL_PROJECTION);
  //glPopMatrix();
  gcpRendD3D->m_matProj->Pop();
  dv->SetTransform(D3DTS_PROJECTION, gcpRendD3D->m_matProj->GetTop()); 
  
  //glMatrixMode(GL_MODELVIEW);
  //glPopMatrix();
  gRenDev->PopMatrix();

  gRenDev->ResetToDefault();

  return true;
}

void CREOcLeaf::mfEndFlush(void)
{
}

void CREOcean::UpdateTexture()
{
}

bool CREOcean::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  return true;
}

bool CRETriMeshShadow::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  CD3D8Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE8 dv = rd->mfGetD3DDevice();

  rd->SetState(0);
  dv->SetRenderState(D3DRS_STENCILENABLE, TRUE);
  dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
  dv->SetRenderState(D3DRS_STENCILREF, 0);
  dv->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
  dv->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);

  rd->EF_SetColorOp(eCO_DISABLE);

	if (gRenDev->CV_ind_VisualizeShadowVolumes)
	{
    rd->EF_SetGlobalColor(1,1,1,1);
    rd->SetState(GS_NODEPTHTEST);
	}
	else
    rd->SetState(GS_NOCOLMASK);

/* if(m_pBuffer)
	{
    dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT);
    dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

    rd->D3DSetCull(eCULL_Front);

	  //rd->DrawBuffer(m_pVertexBuffer, m_upIndicies, m_nNumIndices, R_PRIMV_TRIANGLES);
	  rd->DrawBuffer(m_pBuffer->m_pVertexBuffer, m_upIndicies, m_nNumIndices, R_PRIMV_TRIANGLES);

    dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECRSAT);
    rd->D3DSetCull(eCULL_Back);
  	
	  //gRenDev->DrawBuffer(m_pVertexBuffer, m_upIndicies, m_nNumIndices, R_PRIMV_TRIANGLES);
	  rd->DrawBuffer(m_pBuffer->m_pVertexBuffer, m_upIndicies, m_nNumIndices, R_PRIMV_TRIANGLES);
  	rd->m_nShadowVolumePolys+=m_pBuffer->GetIndices().Count()/3;
  }*/

  dv->SetRenderState(D3DRS_STENCILENABLE, FALSE);

  return true;
}

/*
bool CRETriMeshAdditionalShadow::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  return true;
}
*/

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
  // added by wat
  if (gRenDev->m_RP.m_PersFlags & RBPF_DONTDRAWSUN)
    return false;


  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();

  if (!obj)
    return false;

  Vec3d or = obj->m_Trans;
  bool bVis = false;
  bool bSun = false;
  
  float len = or.Length();
  if (len > 4096.0f)
  {
    bSun = true;
    or.Normalize();
    or *= 500.0f;
    or += gRenDev->m_RP.m_ViewOrg;
  }
  if (bSun && (gRenDev->m_RP.m_PersFlags & RBPF_DONTDRAWSUN))
    return false;

  if (CRenderer::CV_r_SunStyleCoronas)
    bSun = true;

  /*
  if (CRenderer::CV_r_checkSunVis == 1 || !bSun || !gRenDev->GetAlphaBpp())
  {
    if (bSun)
      CRenderer::CV_r_checkSunVis = 1;

    float sx, sy, sz;
    float fDepth;
    gRenDev->ProjectToScreen(or.x, or.y, or.z, &sx, &sy, &sz);
    if (sx>=0 && sy>=0 && sx<vp[2] && sy<vp[3])
    {
      //wat
      //glReadPixels((GLint)sx, (GLint)sy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fDepth);

      if (fDepth>sz)
        bVis = true;
    }    
  }
  else
  */

//*

  if (CRenderer::CV_r_checkSunVis == 2)
  {
    gRenDev->ResetToDefault();

    int sizeMask  = 64;

    int sizeMask2  = sizeMask/2;

    int sizeTex  = 128;//m_Map->GetWidth();
    if (!obj->m_TexId0)
    {
      byte *data = new byte[sizeMask*sizeMask*4];
      char name[128];
      sprintf(name, "$AutoCoronas_%d", gcpRendD3D->m_TexGenID++);
      STexPic *tp = gcpRendD3D->m_TexMan->CreateTexture(name, sizeMask, sizeMask, 1, FT_NOMIPS | FT_ALLOCATED, FT2_NODXT, data, eTT_Base, -1.0f, -1.0f, 0, NULL);
      delete [] data;
      obj->m_TexId0 = tp->m_Id;

      IDirect3DSurface8 * pTar = gcpRendD3D->mfGetBackSurface();
      D3DSURFACE_DESC dc;
      pTar->GetDesc(&dc);
      if(tp->m_RefTex->m_VidTex)
        ((LPDIRECT3DTEXTURE8)tp->m_RefTex->m_VidTex)->Release();
#ifndef _XBOX
      HRESULT h = D3DXCreateTexture(dv, sizeMask, sizeMask, 1, D3DUSAGE_DYNAMIC, dc.Format, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE8* )& tp->m_RefTex->m_VidTex ));
#else //_XBOX
      HRESULT h = D3DXCreateTexture(dv, sizeMask, sizeMask, 1, 0, dc.Format, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE8* )& tp->m_RefTex->m_VidTex ));
#endif //_XBOX
    }
    if (!obj->m_TexId1)
    {
      byte *data = new byte[sizeTex*sizeTex*4];
      char name[128];
      sprintf(name, "$AutoCoronas_%d", gcpRendD3D->m_TexGenID++);
      STexPic *tp = gcpRendD3D->m_TexMan->CreateTexture(name, sizeTex, sizeTex, 1, FT_NOMIPS | FT_ALLOCATED, FT2_NODXT, data, eTT_Base, -1.0f, -1.0f, 0, NULL);
      delete [] data;
      obj->m_TexId1 = tp->m_Id;

      IDirect3DSurface8 * pTar = gcpRendD3D->mfGetBackSurface();
      D3DSURFACE_DESC dc;
      pTar->GetDesc(&dc);
      if(tp->m_RefTex->m_VidTex)
        ((LPDIRECT3DTEXTURE8)tp->m_RefTex->m_VidTex)->Release();
#ifndef _XBOX
      HRESULT h = D3DXCreateTexture(dv, sizeTex, sizeTex, 1, D3DUSAGE_DYNAMIC, dc.Format, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE8* )& tp->m_RefTex->m_VidTex ));
      //HRESULT h = D3DXCreateTexture(dv, sizeTex, sizeTex, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE8* )& tp->m_RefTex->m_VidTex ));
#else //_XBOX
      HRESULT h = D3DXCreateTexture(dv, sizeTex, sizeTex, 1, 0, dc.Format, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE8* )& tp->m_RefTex->m_VidTex ));
      //HRESULT h = D3DXCreateTexture(dv, sizeTex, sizeTex, 1, 0, D3DFMT_A8, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE8* )& tp->m_RefTex->m_VidTex ));
#endif //_XBOX
    }

    float sx, sy, sz;
    int vp[4];
    gRenDev->GetViewport(&vp[0], &vp[1], &vp[2], &vp[3]);
    gRenDev->ProjectToScreen(or.x, or.y, or.z, &sx, &sy, &sz);

    if (sx<vp[0]-sizeMask2 || sy<vp[1]-sizeMask2 || sx>vp[2]+sizeMask2 || sy>vp[3]+sizeMask2 || sz > 1.0f)
    {
      bVis = false;
    }
    else
    {
      //sx = floorf(sx);
      //sy = floorf(sy);

      int bFog=0; // remember fog value
      ///glGetIntegerv(GL_FOG,&bFog);
      bFog = gRenDev->m_FS.m_bEnable;
      ///glDisable(GL_FOG);
      dv->SetRenderState( D3DRS_FOGENABLE, FALSE);

      ///glMatrixMode(GL_PROJECTION);
      ///glPushMatrix();
      gcpRendD3D->m_matProj->Push();
      //glLoadIdentity();
      //glOrtho(0.0, gRenDev->GetWidth(), 0.0, gRenDev->GetHeight(), -20.0, 0.0);
      D3DXMatrixOrthoRH( gcpRendD3D->m_matProj->GetTop(), (float)gRenDev->GetWidth(), (float)gRenDev->GetHeight(), -20.0, 0.0);
      dv->SetTransform(D3DTS_PROJECTION, gcpRendD3D->m_matProj->GetTop()); 

      //glMatrixMode(GL_MODELVIEW);
      //glPushMatrix();
      gRenDev->PushMatrix();
      //glLoadIdentity();
      gcpRendD3D->m_matView->LoadIdentity();
      dv->SetTransform(D3DTS_VIEW, gcpRendD3D->m_matView->GetTop()); 

      gRenDev->SetCullMode(R_CULL_NONE);
      gRenDev->SetEnviMode(R_MODE_MODULATE);

      //gRenDev->m_Text_White->Set();
      CTexMan::m_Text_White->Set();

      float fsizeMask2 = (float)sizeMask2;

      // set alpha to 0
      {
        gRenDev->SetState(GS_NODEPTHTEST);
        gcpRendD3D->DrawQuad(sx-fsizeMask2, sy-fsizeMask2, sx+fsizeMask2, sy+fsizeMask2, CFColor(1.0f));
      }

      // Set alpha mask of visible areas (1 is visible)
      {
        gRenDev->SetState(0);
        gcpRendD3D->DrawQuad(sx-fsizeMask2, sy-fsizeMask2, sx+fsizeMask2, sy+fsizeMask2, CFColor(0.0f));
      }

      // Generate alpha-mask texture with mip-maps
      //gRenDev->SetTexture(obj->m_TexId0, eTT_Base);
      //glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)sx-sizeMask2, (int)sy-sizeMask2, sizeMask, sizeMask);

      //byte *data = new byte [sizeMask * sizeMask * 3];
      //int n = 0;
      //glGetTexImage(GL_TEXTURE_2D, n, GL_RGB, GL_UNSIGNED_BYTE, data);
      //int s = sizeMask*sizeMask;
      //for (int i=0; i<s; i++)
      //{
      //  n += data[i*3];
      //}
      //n /= s;
      //delete [] data;

      DWORD dwPix = 0;
      RECT rc = {Max((int)sx-sizeMask2+1, vp[0]), Max((int)sy-sizeMask2+1,vp[1]), Min((int)sx+sizeMask2,vp[2]-1), Min((int)sy+sizeMask2,vp[3]-1)};
      if((rc.right - rc.left) > 0 && rc.bottom - rc.top >0)
      {
        int dev = (rc.right - rc.left) * (rc.bottom - rc.top);
        //IDirect3DTexture8 * pID3DTexture = (IDirect3DTexture8 * )obj->m_TexId0;
        STexPic *t = (STexPic *)gcpRendD3D->m_TexMan->m_Textures[obj->m_TexId0];
        IDirect3DTexture8 * pID3DTexture = (IDirect3DTexture8 * )t->m_RefTex->m_VidTex;
        if(pID3DTexture)
        {
          IDirect3DSurface8 * pTexSurf;
          pID3DTexture->GetSurfaceLevel(0, &pTexSurf);
          IDirect3DSurface8 * pTar = gcpRendD3D->mfGetBackSurface();

          DWORD dwPixSize = 4;
          D3DSURFACE_DESC dc;
          pTar->GetDesc(&dc);
          switch(dc.Format)
          {
#ifndef _XBOX
            case D3DFMT_R8G8B8:
              dwPixSize = 3; break;
#endif // _XBOX
            case D3DFMT_R5G6B5:
            case D3DFMT_X1R5G5B5:
            case D3DFMT_A1R5G5B5:
              dwPixSize = 2; break;
          };

          POINT p = {0,0};
          if(dv->CopyRects(pTar, &rc, 1, pTexSurf, &p)==S_OK)
          {
            D3DLOCKED_RECT lr;
            if(pTexSurf->LockRect(&lr, 0, D3DLOCK_READONLY)==S_OK)
            {
              byte * pBuf = (byte*)lr.pBits;
              for(int i = 0; i< rc.right-rc.left; i++)
                for(int j = 0; j< rc.bottom-rc.top; j++)
                  if(dwPixSize == 2)
                    dwPix += ((pBuf[i*dwPixSize + sizeMask * j * dwPixSize]) & 0x1f)? 255 : 0;
                  else
                    dwPix += pBuf[i*dwPixSize + sizeMask * j * dwPixSize];
              dwPix /= dev;
              pTexSurf->UnlockRect();
            }
          }
          pTexSurf->Release();
        }
      }
      DWORD n = dwPix;


      obj->m_TempVars[0] = (float)n / 255.0f;
      if (n < 20)
      {
        bVis = false;
      }
      else
      {
        // Scale factor
        obj->m_TempVars[1] = 8.0f;
        int nPasses = 32;
        float fSizeTex = (float)sizeTex;
        // Result blured texture size
        obj->m_TempVars[2] = fSizeTex;
        float fScale = m_fScale * obj->m_TempVars[1];
        float fDif = fSizeTex / (float)sizeMask;
        float fSizeTex2 = fSizeTex / 2.0f;
        float fLast = fSizeTex2 / (fScale * fDif);
        float fCur = fSizeTex2 * 2;
        float fDelt = (fLast - fCur) / (float)nPasses;
        float fCurAlpha = 0.0f;
        float fDeltAlpha = 1.0f / (float)nPasses;
        float fPosx = (float)(gRenDev->GetWidth()/2);
        float fPosy = (float)(gRenDev->GetHeight()/2);


        gRenDev->SetState(GS_NODEPTHTEST);
        CTexMan::m_Text_White->Set();
        
        gcpRendD3D->DrawQuad(fPosx-fSizeTex2, fPosy-fSizeTex2, fPosx+fSizeTex2, fPosy+fSizeTex2, CFColor(1.0f));

        //gRenDev->SetTexture(obj->m_TexId0, eTT_Base);
        STexPic *t = (STexPic *)gcpRendD3D->m_TexMan->m_Textures[obj->m_TexId0];
        gRenDev->SetTexture(t->m_Bind, eTT_Base);
        dv->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
        dv->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

        // Radially blur the alpha texture
        for (int pass=0; pass<nPasses; pass++)
        {
          gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
          //gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCCOL | GS_NODEPTHTEST);
          
          //dv->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
          //dv->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
          //dv->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

#ifndef _XBOX
          gcpRendD3D->DrawQuad(fPosx-fCur, fPosy-fCur, fPosx+fCur, fPosy+fCur, CFColor(fDeltAlpha));
#else //_XBOX
          gcpRendD3D->DrawQuad(fPosx-fCur, fPosy-fCur, fPosx+fCur, fPosy+fCur, CFColor(fDeltAlpha), 0, 0, (float)sizeMask, (float)sizeMask);
#endif //_XBOX

          fCur += fDelt;
          fCurAlpha += fDeltAlpha;
        }

        // Generate blured alpha texture without mip-maps
        //gRenDev->SetTexture(obj->m_TexId1, eTT_Base);
        //glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)(fPosx-fSizeTex2), (int)(fPosy-fSizeTex2), sizeTex, sizeTex);
        RECT rc = {Max((int)(fPosx-fSizeTex2)+1, vp[0]), Max((int)(fPosy-fSizeTex2)+1,vp[1]), Min((int)(fPosx+fSizeTex2),vp[2]-1), Min((int)(fPosy+fSizeTex2),vp[3]-1)};
        if((rc.right - rc.left) > 0 && rc.bottom - rc.top >0)
        {
          //IDirect3DTexture8 * pID3DTexture = (IDirect3DTexture8 * )obj->m_TexId1;
          STexPic *t = (STexPic *)gcpRendD3D->m_TexMan->m_Textures[obj->m_TexId1];
          IDirect3DTexture8 * pID3DTexture = (IDirect3DTexture8 * )t->m_RefTex->m_VidTex;
          if(pID3DTexture)
          {
            IDirect3DSurface8 * pTexSurf;
            pID3DTexture->GetSurfaceLevel(0, &pTexSurf);
            IDirect3DSurface8 * pTar = gcpRendD3D->mfGetBackSurface();
            POINT p = {0,0};
            dv->CopyRects(pTar, &rc, 1, pTexSurf, &p);
            RECT rec = {0, 0, rc.right - rc.left, rc.bottom - rc.top};
            pTexSurf->Release();
          }
          bVis = true;
        }
        else
          bVis = false;
      }

      gRenDev->ResetToDefault();
      //gRenDev->SetState(GS_NODEPTHTEST);

      gRenDev->PopMatrix();
      gcpRendD3D->m_matProj->Pop();
      dv->SetTransform(D3DTS_PROJECTION, gcpRendD3D->m_matProj->GetTop()); 

      if(bFog)
        dv->SetRenderState( D3DRS_FOGENABLE, TRUE);

      gRenDev->SetTexture(0, eTT_Base);
      // !!! WARNING !!!
      //gcpRendD3D->EF_ClearBuffers(true);
    }
  }
  else
  if (CRenderer::CV_r_checkSunVis == 3)
  {
    gRenDev->ResetToDefault();
    int sizeMask  = 64;
    int sizeMask2  = sizeMask/2;

    int sizeTex  = 128;//m_Map->GetWidth();
    float sx, sy, sz;
    int vp[4];
    gRenDev->GetViewport(&vp[0], &vp[1], &vp[2], &vp[3]);
    gRenDev->ProjectToScreen(or.x, or.y, or.z, &sx, &sy, &sz);

    if (sx<vp[0]-sizeMask2 || sy<vp[1]-sizeMask2 || sx>vp[2]+sizeMask2 || sy>vp[3]+sizeMask2)
    {
      bVis = false;
    }
    else
    {
      int bFog=0; // remember fog value
      bFog = gRenDev->m_FS.m_bEnable;
      dv->SetRenderState( D3DRS_FOGENABLE, FALSE);

      gcpRendD3D->m_matProj->Push();
      D3DXMatrixOrthoRH( gcpRendD3D->m_matProj->GetTop(), (float)gRenDev->GetWidth(), (float)gRenDev->GetHeight(), -20.0, 0.0);
      dv->SetTransform(D3DTS_PROJECTION, gcpRendD3D->m_matProj->GetTop()); 
      
      gRenDev->PushMatrix();
      gcpRendD3D->m_matView->LoadIdentity();
      dv->SetTransform(D3DTS_VIEW, gcpRendD3D->m_matView->GetTop()); 

      gRenDev->SetCullMode(R_CULL_NONE);
      gRenDev->SetEnviMode(R_MODE_MODULATE);

      float fsizeMask2 = (float)sizeMask2;

      CTexMan::m_Text_White->Set();
      // set alpha to 0
      gRenDev->SetState(GS_NODEPTHTEST);
      gcpRendD3D->DrawQuad(sx-fsizeMask2, sy-fsizeMask2, sx+fsizeMask2, sy+fsizeMask2, CFColor(0.0f));
      // Set alpha mask of visible areas
      gRenDev->SetState(0);
      gcpRendD3D->DrawQuad(sx-fsizeMask2, sy-fsizeMask2, sx+fsizeMask2, sy+fsizeMask2, CFColor(1.0f));

      // Generate alpha-mask texture with mip-maps
      DWORD dwPix = 0;
      RECT rc = {Max((int)sx-sizeMask2+1, vp[0]), Max((int)sy-sizeMask2+1,vp[1]), Min((int)sx+sizeMask2,vp[2]-1), Min((int)sy+sizeMask2,vp[3]-1)};
      if((rc.right - rc.left) > 0 && rc.bottom - rc.top >0)
      {
        int dev = (rc.right - rc.left) * (rc.bottom - rc.top);
        IDirect3DSurface8 * pTar = gcpRendD3D->mfGetBackSurface();

        DWORD dwPixSize = 4;
        D3DSURFACE_DESC dc;
        pTar->GetDesc(&dc);
        switch(dc.Format)
        {
#ifndef _XBOX
          case D3DFMT_R8G8B8:
            dwPixSize = 3; break;
#endif // _XBOX
          case D3DFMT_R5G6B5:
          case D3DFMT_X1R5G5B5:
          case D3DFMT_A1R5G5B5:
            dwPixSize = 2; break;
        };

        POINT p = {0,0};
        D3DLOCKED_RECT lr;
        if(pTar->LockRect(&lr, &rc, D3DLOCK_READONLY)==S_OK)
        {
          byte * pBuf = (byte*)lr.pBits;
          for(int i = 0; i< rc.right-rc.left; i++)
            for(int j = 0; j< rc.bottom-rc.top; j++)
              if(dwPixSize == 2)
                dwPix += ((pBuf[i*dwPixSize + j * lr.Pitch]) & 0x1f)? 255 : 0;
              else
                dwPix += pBuf[i*dwPixSize + j * lr.Pitch];
          dwPix /= dev;
          pTar->UnlockRect();
        }
      }
      gRenDev->SetState(GS_COLMASKONLYALPHA | GS_NODEPTHTEST);

      obj->m_TempVars[0] = ((float)dwPix / 255.0f) * 1.5f;
      obj->m_TempVars[0] = Clamp(obj->m_TempVars[0], 0.0f, 1.0f);
      if (dwPix < 50)
        bVis = false;
      else
        bVis = true;

      gRenDev->ResetToDefault();
      //gRenDev->SetState(GS_NODEPTHTEST);

      gRenDev->PopMatrix();
      gcpRendD3D->m_matProj->Pop();
      dv->SetTransform(D3DTS_PROJECTION, gcpRendD3D->m_matProj->GetTop()); 

      if(bFog)
        dv->SetRenderState( D3DRS_FOGENABLE, TRUE);

      gRenDev->SetTexture(0, eTT_Base);
    }
  }
  
  if (bVis)
  {
    if (!obj->m_bVisible)
    {
      obj->m_bVisible = true;
      obj->m_fLightFadeTime = gRenDev->m_RP.m_RealTime;
    }
  }
  else
  {
    if (obj->m_bVisible)
    {
      obj->m_bVisible = false;
      obj->m_fLightFadeTime = gRenDev->m_RP.m_RealTime;
    }
  }

  return bVis;
}







//////////////////////  STOP  ///////////////////////////





/*
void CREFlare::mfDrawFlares(SShader *ef, CFColor &col)
{
  CSunFlares *lfl = ef->m_Flares;

  int i;
  SSunFlare *fl;
  Vec3d lv;
  
  if (!CRenderer::CV_r_flares || !lfl)
    return;
    
  CCObject *obj = gRenDev->m_RP.m_CurObject;

  Vec3d or = obj->m_Trans;
  
  float len = or.Length();
  if (len > 4096.0f)
  {
    or.Normalize();
    or *= 500.0f;
    or += gRenDev->m_RP.m_ViewOrg;
    if (gRenDev->m_bHeatVision)
      return;
  }

  Vec3d vFromPt = gRenDev->m_RP.m_ViewOrg;
  Vec3d vViewVec = gRenDev->m_RP.m_CamVecs[0];
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
  
  gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
  gRenDev->SetCullMode(R_CULL_NONE);
  
  Vec3d VecX = gRenDev->m_RP.m_CamVecs[1];
  Vec3d VecY = gRenDev->m_RP.m_CamVecs[2];
  Vec3d v;

  fDot *= col.a;
  if (CRenderer::CV_r_checkSunVis > 1)
    fDot *= obj->m_TempVars[0];
  
  // Do the flares
  for (i=0; i<lfl->m_NumFlares; i++)
  {
    fl = &lfl->m_Flares[i];
    
    Vec3d vx = VecX * fl->m_RenderSize;
    Vec3d vy = VecY * fl->m_RenderSize;

    CFColor col = fl->m_Color * fDot;
    fl->m_Tex->Set();
    glColor3fv(&col[0]);
    
    glBegin(GL_QUADS);
    
    v = fl->m_Position + vx + vy;
    glTexCoord2f(0, 0);
    glVertex3fv(&v[0]);
    
    v = fl->m_Position - vx + vy;
    glTexCoord2f(1, 0);
    glVertex3fv(&v[0]);
    
    v = fl->m_Position - vx - vy;
    glTexCoord2f(1, 1);
    glVertex3fv(&v[0]);
    
    v = fl->m_Position + vx - vy;
    glTexCoord2f(0, 1);
    glVertex3fv(&v[0]);
    
    glEnd();

    gRenDev->m_nPolygons += 2;
  }
}
*/


void CREFlare::mfDrawCorona(SShader *ef, CFColor &col)
{
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();

  Vec3d vx, vy, or, v, norm;

  if (!CRenderer::CV_r_coronas)
    return;
  
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  or = obj->m_Trans;

  bool bSun = true;
  float len = or.Length();
  if (len > 4096.0f)
  {
    bSun = true;
    or.Normalize();
    or *= 500.0f;
    or += gRenDev->m_RP.m_ViewOrg;
  }
  if (bSun && gRenDev->m_bHeatVision)
    return;
  
  gcpRendD3D->EF_PushFog();
  gcpRendD3D->EnableFog(false);

  vx = gRenDev->m_RP.m_CamVecs[1];
  vy = gRenDev->m_RP.m_CamVecs[2];

  v = or - gRenDev->m_RP.m_ViewOrg;
  float distX = v.Length();
  if (m_fDistSizeFactor != 1.0f)
    distX = powf(distX, m_fDistSizeFactor);
  float distY = distX;

  distX *= m_fScale * CRenderer::CV_r_coronasizescale;
  distY *= m_fScale * CRenderer::CV_r_coronasizescale;

  float fDecay = col[3];
  if (CRenderer::CV_r_checkSunVis == 3 && bSun)
    fDecay *= Max(m_fMinLight, obj->m_TempVars[0]);
  fDecay *= CRenderer::CV_r_coronacolorscale;

/*
  norm = gRenDev->m_RP.m_CamVecs[0];
  if (m_bBlind && gRenDev->m_RP.m_DynLMask)
  {
    gRenDev->EF_BuildLightsList();
    if (gRenDev->m_RP.m_NumActiveDLights)
    {
      CDLight *dl = gRenDev->m_RP.m_pActiveDLights[0];
      if (dl->m_Flags & DLF_PROJECT)
      {
        v.Normalize();
        float fDot = -(v * dl->m_Orientation.m_vForward);
        if (fDot < 0.0f)
          return;
        float fBlindSize = fDot * m_fSizeBlindScale + m_fSizeBlindBias;
        float fBlindIntens = fDot * m_fIntensBlindScale + m_fIntensBlindBias;
        fDecay *= fBlindIntens;
        distX *= fBlindSize;
        distY *= fBlindSize;
      }
    }
  }
*/
  vx *= distX;
  vy *= distY;
  norm = gRenDev->m_RP.m_CamVecs[0];

  if (bSun)
    gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
  else
    gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
  gcpRendD3D->SetCullMode(R_CULL_NONE);

  CFColor c = col;

  c[0] = c[0] * fDecay;
  c[1] = c[1] * fDecay;
  c[2] = c[2] * fDecay;
  if (bSun)
    c[3] = fDecay;

  if (CRenderer::CV_r_SunStyleCoronas)
    bSun = true;

  if (CRenderer::CV_r_checkSunVis == 1 || CRenderer::CV_r_checkSunVis == 3 || !bSun || obj->m_TexId1<=0)
  {
    m_Map->Set();
    gRenDev->m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
    gcpRendD3D->DrawQuad3D(or + vx + vy, or - vx + vy, or - vx - vy, or + vx - vy, c);
  }
  else
  if (CRenderer::CV_r_checkSunVis == 2)
  {
    gRenDev->SelectTMU(0);
    gRenDev->EnableTMU(true);  
    m_Map->Set();

    gRenDev->SelectTMU(1);
    gRenDev->EnableTMU(true);

    STexPic *t = (STexPic *)gcpRendD3D->m_TexMan->m_Textures[obj->m_TexId1];
    gRenDev->m_TexMan->SetTexture(t->m_Bind, eTT_Base);

    CPShader *pRC = NULL;
    if (gRenDev->GetFeatures() & RFT_HW_RC && !gRenDev->m_RP.m_RCSun)
    {
      pRC = CPShader::mfForName("RCSun", false);
      gRenDev->m_RP.m_RCSun = pRC;
    }
    else
      pRC = gRenDev->m_RP.m_RCSun;

    //if (pRC)
    //  pRC->mfSet(true);

    gRenDev->m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;

    /*
    dv->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    dv->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_CURRENT);
    dv->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    dv->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE );
    dv->SetTextureStageState(1, D3DTSS_COLORARG0, D3DTA_CURRENT);
    dv->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE );


    //dv->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
    //dv->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
    /**/
    
    dv->SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
    dv->SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );


#ifndef _XBOX
    gcpRendD3D->DrawQuad3D(or + vx - vy, or + vx + vy, or - vx + vy, or - vx - vy, c);
#else  //_XBOX
    gcpRendD3D->DrawQuad3D(or + vx - vy, or + vx + vy, or - vx + vy, or - vx - vy, c, 0,0, 1, 1, 0,0, 128, 128);
#endif //_XBOX

    if (pRC)
      pRC->mfSet(false);
    gRenDev->EnableTMU(false);  
    gRenDev->SelectTMU(0);
  }

  // I do it in DrawQuad3D()
  //gRenDev->m_nPolygons += 2;
  gcpRendD3D->EF_PopFog();
}





bool CREFlare::mfDraw(SShader *ef, SShaderPass *sfm)
{
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  if (!CRenderer::CV_r_flares && !CRenderer::CV_r_coronas)
    return false;

  Matrix m = gRenDev->GetCamera().GetMatrix();
  //glLoadMatrixf(&m.m_values[0][0]);
  dv->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m(0,0));

  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  float len = obj->m_Trans.Length();
  if (len > 4096.0f && (gRenDev->m_RP.m_PersFlags & RBPF_DONTDRAWSUN))
    return false;

  float ft;
  if (obj->m_bVisible)
    ft = obj->m_Color.a + (gRenDev->m_RP.m_RealTime - obj->m_fLightFadeTime) / gRenDev->CV_r_coronafade;
  else
    ft = obj->m_Color.a - (gRenDev->m_RP.m_RealTime - obj->m_fLightFadeTime) / gRenDev->CV_r_coronafade;
  ft = Clamp(ft, 0.0f, 1.0f);

  if (!ft)
    return false;

  obj->m_Color.r = m_Color.r;
  obj->m_Color.g = m_Color.g;
  obj->m_Color.b = m_Color.b;
  obj->m_Color.a = ft;
  mfDrawCorona(ef, obj->m_Color);
  //mfDrawFlares(ef, obj->m_Color);

  return true;
}

//================================================================================

bool CRESky::mfDraw(SShader *ef, SShaderPass *sfm)
{   
  int bPrevClipPl = gcpRendD3D->m_RP.m_ClipPlaneEnabled;
  if (bPrevClipPl)
    gcpRendD3D->EF_SetClipPlane(false, NULL, false);

  if(sfm == &ef->m_Passes[1])
  { // draw sky sphere vertices at pass 1
    bool bPrevFog = gRenDev->EnableFog(false);
    DrawSkySphere();
    gRenDev->EnableFog(bPrevFog);
    if (bPrevClipPl)
      gcpRendD3D->EF_SetClipPlane(true, &gcpRendD3D->m_RP.m_CurClipPlane.m_Normal.x, gcpRendD3D->m_RP.m_bClipPlaneRefract);
    return true;
  }

  // pass 0 - skybox

  if (!ef->m_Sky || !ef->m_Sky->m_SkyBox[0])
  {
    if (bPrevClipPl)
      gcpRendD3D->EF_SetClipPlane(true, &gcpRendD3D->m_RP.m_CurClipPlane.m_Normal.x, gcpRendD3D->m_RP.m_bClipPlaneRefract);
    return false;
  }

  bool bPrevFog = gRenDev->EnableFog(false);
  
  gRenDev->ResetToDefault();
  gRenDev->SetEnviMode(R_MODE_MODULATE);
  gRenDev->EnableDepthTest(false);
  gRenDev->EnableDepthWrites(false);

  gRenDev->SetMaterialColor(1,1,1,m_fAlpha);
  if(m_fAlpha<1.f)
  {
    gRenDev->EnableBlend(true);
    gRenDev->SetBlendMode(R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA);
  }

  const float fSkyBoxSize = 32;

  { // top
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    {
      { fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize, 1, 1.f-1},
      {-fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize, 0, 1.f-1},
      { fSkyBoxSize, fSkyBoxSize, fSkyBoxSize, 1, 1.f-0},
      {-fSkyBoxSize, fSkyBoxSize, fSkyBoxSize, 0, 1.f-0}
    };

    gRenDev->SetTexture(ef->m_Sky->m_SkyBox[2]->m_Bind);
    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
  }

  Vec3d camera = gRenDev->GetCamera().GetPos();

  float P = (camera.z-m_fTerrainWaterLevel)/fSkyBoxSize/2.0f;
  float D = (camera.z-m_fTerrainWaterLevel)/10.0f*fSkyBoxSize/124.0f - P;

  { // s
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
     -fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize, 1.0, 1.f-1.0,
      fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize, 0.0, 1.f-1.0,
     -fSkyBoxSize,-fSkyBoxSize,-P,           1.0, 1.f-0.5,
      fSkyBoxSize,-fSkyBoxSize,-P,           0.0, 1.f-0.5,
     -fSkyBoxSize,-fSkyBoxSize,-D,           1.0, 1.f-0.5,
      fSkyBoxSize,-fSkyBoxSize,-D,           0.0, 1.f-0.5
    };

    gRenDev->SetTexture(ef->m_Sky->m_SkyBox[1]->m_Bind);
    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
  }
  { // e
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
     -fSkyBoxSize, fSkyBoxSize, fSkyBoxSize, 1.0, 1.f-0.0,
     -fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize, 0.0, 1.f-0.0,
     -fSkyBoxSize, fSkyBoxSize,-P,           1.0, 1.f-0.5,
     -fSkyBoxSize,-fSkyBoxSize,-P,           0.0, 1.f-0.5,
     -fSkyBoxSize, fSkyBoxSize,-D,           1.0, 1.f-0.5,
     -fSkyBoxSize,-fSkyBoxSize,-D,           0.0, 1.f-0.5
    };

    gRenDev->SetTexture(ef->m_Sky->m_SkyBox[1]->m_Bind);
    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
  }
  { // n
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
      fSkyBoxSize, fSkyBoxSize, fSkyBoxSize, 1.0, 1.f-1.0,
     -fSkyBoxSize, fSkyBoxSize, fSkyBoxSize, 0.0, 1.f-1.0,
      fSkyBoxSize, fSkyBoxSize,-P,           1.0, 1.f-0.5,
     -fSkyBoxSize, fSkyBoxSize,-P,           0.0, 1.f-0.5,
      fSkyBoxSize, fSkyBoxSize,-D,           1.0, 1.f-0.5,
     -fSkyBoxSize, fSkyBoxSize,-D,           0.0, 1.f-0.5
    };

    gRenDev->SetTexture(ef->m_Sky->m_SkyBox[0]->m_Bind);
    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
  }
  { // w
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
      fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize, 1.0, 1.f-0.0,
      fSkyBoxSize, fSkyBoxSize, fSkyBoxSize, 0.0, 1.f-0.0,
      fSkyBoxSize,-fSkyBoxSize,-P,           1.0, 1.f-0.5,
      fSkyBoxSize, fSkyBoxSize,-P,           0.0, 1.f-0.5,
      fSkyBoxSize,-fSkyBoxSize,-D,           1.0, 1.f-0.5,
      fSkyBoxSize, fSkyBoxSize,-D,           0.0, 1.f-0.5
    };

    gRenDev->SetTexture(ef->m_Sky->m_SkyBox[0]->m_Bind);
    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
  }
  
  gRenDev->ResetToDefault();

  gRenDev->mCurState = GS_DEPTHWRITE;

  gRenDev->EnableFog(bPrevFog);

  if (bPrevClipPl)
    gcpRendD3D->EF_SetClipPlane(true, &gcpRendD3D->m_RP.m_CurClipPlane.m_Normal.x, gcpRendD3D->m_RP.m_bClipPlaneRefract);

  return true;
}

void CRESky::DrawSkySphere()
{
  float nWSize = 256/8;  
 
  float a_in  = 1, a_out = 1;

  float clouds_r = 1.f;//m_vColor[0]*m_fBrightness;
  float clouds_g = 1.f;//m_vColor[1]*m_fBrightness;
  float clouds_b = 1.f;//m_vColor[2]*m_fBrightness;

  struct_VERTEX_FORMAT_P3F_COL4UB vert;
  vert.r=uchar(clouds_r*255);
  vert.g=uchar(clouds_g*255);
  vert.b=uchar(clouds_b*255);

  list2<struct_VERTEX_FORMAT_P3F_COL4UB> lstVertData;

  for(float r=0; r<3; r++)
  {
    a_in = a_out;
    a_out = 1.f-(r+1)/8;    
    a_out*=a_out; a_out*=a_out; a_out*=a_out;
  
    lstVertData.Clear();

    for(int i=0; i<=360; i+=40)
    {
      float rad = (i) * (M_PI/180);

      vert.x = Fsin(rad)*nWSize*r;
      vert.y = Fcos(rad)*nWSize*r;
      vert.z = 5.f - 10.f*(r)/8.f;
      vert.a = uchar(a_in*255.0f);
      lstVertData.Add(vert);
      
      vert.x = Fsin(rad)*nWSize*(r+1);
      vert.y = Fcos(rad)*nWSize*(r+1);
      vert.z = 5.f - 10.f*(r+1)/8.f;
      vert.a = uchar(a_out*255.0f);
      lstVertData.Add(vert);
    }

    gRenDev->DrawTriStrip(&CVertexBuffer(&lstVertData[0],VERTEX_FORMAT_P3F_COL4UB),lstVertData.Count());
  }
}

void CLeafBuffer::DrawImmediately()
{
}

bool CREClearStencil::mfDraw(SShader *ef, SShaderPass *sfm)
{
  return true;
}