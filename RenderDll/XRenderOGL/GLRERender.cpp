/*=============================================================================
  GLRenderRE.cpp : implementation of the Rendering RenderElements pipeline.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "I3dengine.h"

// tiago:added
#include "GLCGPShader.h"
#include "GLCGVProgram.h"
#include "../Common/RendElements/CREScreenCommon.h"

//#include "../cry3dengine/StatObj.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//=======================================================================

void CREFlareGeom::mfCheckVis(CFColor &col, CCObject *obj)
{
  float Depth;
  float ft, f;

  CGLRenderer *rd = gcpOGL;

  int re = 0;//rd->GetRecurseLevel();
  SFlareFrame *ff = &mFlareFr[re];

  glReadPixels(ff->mX, ff->mY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);
  Depth = (Depth-rd->mMinDepth) / (rd->mMaxDepth-rd->mMinDepth);

  f = gRenDev->m_ProjMatrix(3,2)/((Depth+Depth-1.0f)*gRenDev->m_ProjMatrix(2,3)-gRenDev->m_ProjMatrix(2,2)) - ff->mDepth;
  if (f < 24)
  {
    if (!ff->mbVis)
    {
      ff->mbVis = true;
      ff->mDecayTime = gRenDev->m_RP.m_RealTime-0.001f;
    }
    ft = (gRenDev->m_RP.m_RealTime - ff->mDecayTime) * CRenderer::CV_r_coronafade;
  }
  else
  {
    if (ff->mbVis)
    {
      ff->mbVis = false;
      ff->mDecayTime = gRenDev->m_RP.m_RealTime-0.001f;
    }
    ft = 1.0f - (gRenDev->m_RP.m_RealTime - ff->mDecayTime) * CRenderer::CV_r_coronafade;
  }
  col = ff->mColor;
  col.a = ft;
  col.ClampAlpha();
}

//#include "..\common\shadow_renderer.h"

///////////////////////////////////////////////////////////////////

CREOcclusionQuery::~CREOcclusionQuery()
{
  mfReset();
}

void CREOcclusionQuery::mfReset()
{
  if (m_nOcclusionID)
    glDeleteOcclusionQueriesNV(1, &m_nOcclusionID);
  m_nOcclusionID = 0;
}

void CGLRenderer__Draw3dBBoxSolid(const Vec3d &mins,const Vec3d &maxs);

bool CREOcclusionQuery::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  CGLRenderer *r = gcpOGL;
  int nFrame = r->GetFrameID();

//	if(nFrame&1)
	//  return true;

	if (SUPPORTS_GL_NV_occlusion_query && CGLRenderer::CV_gl_hp_occlusion_test)
  {
    if ( m_nCheckFrame != nFrame )
    {
			if(m_nCheckFrame)
			{
        double time = sCycles2();
				m_nVisSamples=0;
				glGetOcclusionQueryuivNV(m_nOcclusionID, GL_PIXEL_COUNT_NV, &m_nVisSamples);
        r->m_RP.m_PS.m_fOcclusionTime += (float)((sCycles2()+34-time)*1000.0*g_SecondsPerCycle);
			}
      m_nCheckFrame = nFrame;
    }

    if (!m_nOcclusionID)
      glGenOcclusionQueriesNV(1, &m_nOcclusionID);

    if (m_nDrawFrame != nFrame)
		{ // draw bbox
			glBeginOcclusionQueryNV(m_nOcclusionID);
			CGLRenderer__Draw3dBBoxSolid(m_vBoxMin-Vec3d(0.05f,0.05f,0.05f),m_vBoxMax+Vec3d(0.05f,0.05f,0.05f));
			glEndOcclusionQueryNV();
			m_nDrawFrame = nFrame;
		}
  }
  else
  if (SUPPORTS_GL_HP_occlusion_test && CGLRenderer::CV_gl_hp_occlusion_test)
  {
    if ( m_nCheckFrame != nFrame )
    {
			if(m_nCheckFrame)
				glGetIntegerv(GL_OCCLUSION_TEST_RESULT_HP, &m_nVisSamples);
      m_nCheckFrame = nFrame;
      m_nVisSamples *= r->GetWidth()*r->GetHeight();
    }
    // Enable the occlusion test and render the geometry.
    if (m_nDrawFrame != nFrame)
		{ // draw bbox
			glBeginOcclusionQueryNV(m_nOcclusionID);
			CGLRenderer__Draw3dBBoxSolid(m_vBoxMin-Vec3d(0.05f,0.05f,0.05f),m_vBoxMax+Vec3d(0.05f,0.05f,0.05f));
			glEndOcclusionQueryNV();
			m_nDrawFrame = nFrame;
		}
  }
  else
    m_nVisSamples = r->GetWidth()*r->GetHeight();

  return true;
}

// ===============================================================
// FlashBang fx
// Last Update: 24/04/2003

// render flashbang
bool CREFlashBang::mfDraw(SShader *ef, SShaderPass *sfm)
{   
  // sincronize
  ITimer *pTimer=iSystem->GetITimer();  
  m_fFlashTimeOut-= (0.00009f*m_fTimeScale*(pTimer->GetFrameTime()*1000.0f));

  // reset animation
  if(m_fFlashTimeOut<=0.01f) 
  {    
    m_bIsActive=0;
    m_fFlashTimeOut=1.0f;
  }      

  // screen aligned quad
  static struct_VERTEX_FORMAT_P3F_TEX2F pData[]=
  {
    Vec3(800, 600, 0), 1,1.f-1,
    Vec3(800, 0, 0), 1,1.f-0,
    Vec3(0, 600, 0), 0,1.f-1,
    Vec3(0, 0, 0), 0,1.f-0,
  };

  // render quad
  gRenDev->DrawTriStrip(&(CVertexBuffer (pData,VERTEX_FORMAT_P3F_TEX2F)), 4);

  return true;
}

bool CREOcLeaf::mfPreDraw(SShaderPass *sl)
{
  CLeafBuffer *lb = m_pBuffer->GetVertexContainer();

  assert(!lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_bLocked);
  assert(!lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_bLocked);

  if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_pBuffer->m_Indices.m_VertBuf.m_nID);
    gRenDev->m_RP.m_FlagsPerFlush |= RBSI_INDEXSTREAM;
    gRenDev->m_RP.m_RendIndices = NULL;
  }

  return true;
}

void CRE2DQuad::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_FlagsPerFlush |= RBSI_DRAWAS2D;

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_FirstVertex = 0;
  gRenDev->m_RP.m_RendNumVerts = 4;

  float w=800;
  float h=600;
  bool bRect = gRenDev->m_RP.m_pCurObject->m_nTemplId != 0;

  m_arrVerts[0].xyz.x = w;
  m_arrVerts[0].xyz.y = 0;
  m_arrVerts[0].xyz.z = 0;
  m_arrVerts[0].st[0] = bRect ? w : 1;
  m_arrVerts[0].st[1] = bRect ? h : 1;

  m_arrVerts[1].xyz.x = 0;
  m_arrVerts[1].xyz.y = 0;
  m_arrVerts[1].xyz.z = 0;
  m_arrVerts[1].st[0] = 0;
  m_arrVerts[1].st[1] = bRect ? h : 1;

  m_arrVerts[2].xyz.x = w;
  m_arrVerts[2].xyz.y = h;
  m_arrVerts[2].xyz.z = 0;
  m_arrVerts[2].st[0] = bRect ? w : 1;
  m_arrVerts[2].st[1] = 0;

  m_arrVerts[3].xyz.x = 0;
  m_arrVerts[3].xyz.y = h;
  m_arrVerts[3].xyz.z = 0;
  m_arrVerts[3].st[0] = 0;
  m_arrVerts[3].st[1] = 0;
}

bool CRE2DQuad::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  //  gRenDev->EnableDepthTest(false);
  //  gRenDev->EnableDepthWrites(false);
  ///  gRenDev->EnableBlend(true);
  //  gRenDev->SetBlendMode(R_BLEND_MODE__DST_COLOR__SRC_COLOR);

  bool bRect = gRenDev->m_RP.m_pCurObject->m_nTemplId != 0;

  STexPic *tx = gRenDev->m_TexMan->m_Text_EnvScr;

  m_arrVerts[0].st[0] = bRect ? (float)tx->m_Width : 1;
  m_arrVerts[0].st[1] = bRect ? (float)tx->m_Height : 1;

  m_arrVerts[1].st[0] = 0;
  m_arrVerts[1].st[1] = bRect ? (float)tx->m_Height : 1;

  m_arrVerts[2].st[0] = bRect ? (float)tx->m_Width : 1;
  m_arrVerts[2].st[1] = 0;

  m_arrVerts[3].st[0] = 0;
  m_arrVerts[3].st[1] = 0;

  //gRenDev->SetColorMask(0,0,0,1);  
  gRenDev->DrawTriStrip(&CVertexBuffer(m_arrVerts,VERTEX_FORMAT_P3F_TEX2F),4);
  //gRenDev->SetColorMask(1,1,1,1); 

  return true;
}

#define GLARE_OFS 1.0/32

void GlareQuad()
{
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
}

// ===============================================================
// Glare fx
// Last Update: 05/05/2003

bool CREGlare::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  gRenDev->Set2DMode(true, 800, 600);

  // screen aligned quad
  static struct_VERTEX_FORMAT_P3F_TEX2F pData[]=
  {
    Vec3(800, 600, 0), 1, 0,
    Vec3(800,   0, 0), 1, 1,
    Vec3(  0, 600, 0), 0, 0,
    Vec3(  0,   0, 0), 0, 1,
  };

  // render quad
  gRenDev->DrawTriStrip(&(CVertexBuffer (pData,VERTEX_FORMAT_P3F_TEX2F)), 4);
  gRenDev->Set2DMode(false, 800, 600);

  return true;
}

void CGLRenderer__Draw3dBBoxSolid(const Vec3d &mins,const Vec3d &maxs)
{
  glBegin(GL_QUADS);
  glVertex3f(mins.x,mins.y,mins.z); //0
  glVertex3f(mins.x,mins.y,maxs.z); //1
  glVertex3f(maxs.x,mins.y,maxs.z); //2
  glVertex3f(maxs.x,mins.y,mins.z); //3
  glEnd();

  glBegin(GL_QUADS);
  glVertex3f(mins.x,mins.y,mins.z); //0
  glVertex3f(mins.x,mins.y,maxs.z); //1
  glVertex3f(mins.x,maxs.y,maxs.z); //6
  glVertex3f(mins.x,maxs.y,mins.z); //4
  glEnd();

  glBegin(GL_QUADS);
  glVertex3f(mins.x,maxs.y,mins.z); //4
  glVertex3f(mins.x,maxs.y,maxs.z); //6
  glVertex3f(maxs.x,maxs.y,maxs.z); //7
  glVertex3f(maxs.x,maxs.y,mins.z); //5
  glEnd();

  glBegin(GL_QUADS);
  glVertex3f(maxs.x,maxs.y,mins.z); //5
  glVertex3f(maxs.x,maxs.y,maxs.z); //7
  glVertex3f(maxs.x,mins.y,maxs.z); //2
  glVertex3f(maxs.x,mins.y,mins.z); //3
  glEnd();

  // top
  glBegin(GL_QUADS);
  glVertex3f(maxs.x,maxs.y,maxs.z); //5
  glVertex3f(mins.x,maxs.y,maxs.z); //7
  glVertex3f(mins.x,mins.y,maxs.z); //3
  glVertex3f(maxs.x,mins.y,maxs.z); //2
  glEnd();

  // bottom
  glBegin(GL_QUADS);
  glVertex3f(maxs.x,mins.y,mins.z); //2
  glVertex3f(mins.x,mins.y,mins.z); //3
  glVertex3f(mins.x,maxs.y,mins.z); //7
  glVertex3f(maxs.x,maxs.y,mins.z); //5
  glEnd();

	gRenDev->m_nPolygons+=12;
}

bool CREOcLeaf::mfDraw(SShader *ef, SShaderPass *sl)
{
  CGLRenderer *r = gcpOGL;
  CLeafBuffer *lb = m_pBuffer;

  // modify fog far/near dist if requested
  float fFogStart, fFogEnd;
  if(m_fFogScale)
  {
    fFogStart = gRenDev->m_FS.m_FogStart; 
    fFogEnd = gRenDev->m_FS.m_FogEnd; 
    glFogf(GL_FOG_START, fFogStart*m_fFogScale); 
    glFogf(GL_FOG_END  , fFogEnd  *m_fFogScale); 
  }

  // Hardware effector
  if (ef->m_HWTechniques.Num())
  {
    assert(m_pChunk->nFirstIndexId<60000);

    ushort *pInds;
    int nInds;
    int nPrimType;
    SShaderPassHW *slw = (SShaderPassHW *)sl;
    if (CRenderer::CV_r_cullgeometryforlights && r->EF_IsOnlyLightPass(slw) && r->m_RP.m_pCurLightIndices != &r->m_RP.m_FakeLightIndices)
    {
      pInds = r->m_RP.m_pCurLightIndices->GetIndices(nInds);
      nPrimType = R_PRIMV_TRIANGLES;
    }
    else
    {
      if(!lb->m_SecIndices.Num())
        return true; // todo: check that this is never happend, do not add such render elements

      if (r->m_RP.m_FlagsPerFlush & RBSI_INDEXSTREAM)
        pInds = r->m_RP.m_RendIndices;
      else
        pInds = (ushort *)lb->m_Indices.m_VData;
      pInds = &pInds[m_pChunk->nFirstIndexId];
      nInds = m_pChunk->nNumIndices;
      nPrimType = lb->m_nPrimetiveType;
    }
    if (nInds)
    {
      ushort *pSaveInds = r->m_RP.m_RendIndices;
      r->m_RP.m_RendIndices = pInds;
      r->m_RP.m_RendNumIndices = nInds;
      r->EF_DrawIndexedMesh(nPrimType);
      r->m_RP.m_RendNumIndices = 0;
      r->m_RP.m_RendIndices = pSaveInds;
    }
    lb = lb->GetVertexContainer();
    if(!lb->m_pVertexBuffer->m_bFenceSet || lb->m_bDynamic)
    {
      if (SUPPORTS_GL_NV_vertex_array_range)
        glSetFenceNV(lb->m_pVertexBuffer->m_fence, GL_ALL_COMPLETED_NV);
      lb->m_pVertexBuffer->m_bFenceSet = true;
    }

    // restore fog
    if(m_fFogScale)
    {
      glFogf(GL_FOG_START, fFogStart); 
      glFogf(GL_FOG_END  , fFogEnd  ); 
    }

    return true;
  }

  lb = lb->GetVertexContainer();
  if(!lb->m_pVertexBuffer)
  {
    iLog->Log("Warning: CREOcLeaf::mfDraw m_pBuffer->m_pVertexBuffer==0 '%s'",lb->m_sSource!=0?lb->m_sSource:"<NULL>");return(true);
  }

  assert(lb->m_pVertexBuffer);
  r->DrawBuffer(lb->m_pVertexBuffer, &lb->m_Indices, m_pChunk->nNumIndices, m_pChunk->nFirstIndexId, lb->m_nPrimetiveType, m_pChunk->nFirstVertId, m_pChunk->nNumVerts, m_pChunk);

  // restore fog
  if(m_fFogScale)
  {
    glFogf(GL_FOG_START, fFogStart); 
    glFogf(GL_FOG_END  , fFogEnd  ); 
  }

  return (true);
}

///////////////////////////////////////////////////////////////////
void CREOcLeaf::mfEndFlush(void)
{
}

//===============================================================================

bool CRETempMesh::mfPreDraw(SShaderPass *sl)
{
  if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_Inds.m_VertBuf.m_nID);
    gRenDev->m_RP.m_FlagsPerFlush |= RBSI_INDEXSTREAM;
    gRenDev->m_RP.m_RendIndices = NULL;
  }

  return true;
}
void CRETempMesh::mfReset()
{
}

bool CRETempMesh::mfDraw(SShader *ef, SShaderPass *sl)
{
  CGLRenderer *r = gcpOGL;
  CVertexBuffer *vb = m_VBuffer;
  if (!vb)
    return false;

  // Hardware effector
  ushort *pInds;
  if (r->m_RP.m_FlagsPerFlush & RBSI_INDEXSTREAM)
    pInds = r->m_RP.m_RendIndices;
  else
    pInds = (ushort *)m_Inds.m_VData;
  int nPrimType = R_PRIMV_TRIANGLES;
  r->m_RP.m_RendIndices = pInds;
  r->EF_DrawIndexedMesh(nPrimType);
  r->m_RP.m_RendNumIndices = 0;

  if (SUPPORTS_GL_NV_vertex_array_range)
    glSetFenceNV(vb->m_fence, GL_ALL_COMPLETED_NV);
  vb->m_bFenceSet = true;

  return true;
}

//=========================================================================================

bool CREFlare::mfCheckVis(CCObject *obj)
{
  if (!obj)
    return false;

  Vec3d or = obj->GetTranslation();
  //bool bVis = false;
  bool bSun = false;
  CGLRenderer *rd = gcpOGL;

  
  //if (SRendItem::m_RecurseLevel <= 1)
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

    Matrix44 projMatr, camMatr;
    camMatr = rd->m_prevCamera.GetVCMatrixD3D9();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // camera.fov is for horizontal -> GL needs it vertical
    // projection.ratio is height/width -> GL needs width/height
    gluPerspective(rd->m_prevCamera.GetFov()/(gf_PI/180.0f)*rd->m_prevCamera.GetProjRatio(), 1.0f/rd->m_prevCamera.GetProjRatio(), rd->m_prevCamera.GetZMin(), rd->m_prevCamera.GetZMax());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glLoadMatrixf(camMatr.GetData());
    rd->GetProjectionMatrix(projMatr.GetData());

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
    float fsx, fsy, fsz;
    float fDepth;
    int vp[4];
    gRenDev->GetViewport(&vp[0], &vp[1], &vp[2], &vp[3]);
    SGLFuncs::gluProject(or.x, or.y, or.z, camMatr.GetData(), projMatr.GetData(), vp, &fsx, &fsy, &fsz);
    obj->m_Trans2[0] = fsx;
    obj->m_Trans2[1] = fsy;
    obj->m_Trans2[2] = fsz;
    float fCheckInterval = CRenderer::CV_r_coronafade*0.125f;
    if (bRays)
      fCheckInterval *= 0.5f;
    if (obj->m_fLightFadeTime-1.0f > rd->m_RP.m_RealTime)
      obj->m_fLightFadeTime = rd->m_RP.m_RealTime;
    if (gRenDev->m_RP.m_RealTime-obj->m_fLightFadeTime < fCheckInterval)
    {
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      return true;
    }
    obj->m_fLightFadeTime = gRenDev->m_RP.m_RealTime;
    fsx = obj->m_Trans2[0];
    fsy = obj->m_Trans2[1];
    fsz = obj->m_Trans2[2];
    float fIntens = 0;
    if (fsx>=0 && fsy>=0 && fsx<vp[2] && fsy<vp[3] && fFade)
    {
      // Lock back surface.
      int minx, miny, maxx, maxy;
      int wdt = rd->GetWidth();
      int hgt = rd->GetHeight();
      int sx = (int)fsx;
      int sy = (int)fsy;
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

      gRenDev->m_TexMan->m_Text_White->Set();
      rd->GLSetCull(eCULL_None);

      if (!(rd->m_Features & RFT_OCCLUSIONTEST))
      {
        glReadPixels((GLint)sx, (GLint)sy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fDepth);

        obj->m_AmbColor[0] = obj->m_AmbColor[1];
        obj->m_AmbColor[1] = obj->m_AmbColor[2];
        obj->m_AmbColor[2] = obj->m_TempVars[3];
        obj->m_TempVars[3] = obj->m_Angs2[0];
        obj->m_Angs2[0] = obj->m_Angs2[1];
        obj->m_Angs2[1] = obj->m_Angs2[2];
        obj->m_Angs2[2] = obj->m_TempVars[4];
        obj->m_TempVars[4] = fDepth>fsz ? 1.0f : 0;
      }
      else
      {
        if (!obj->m_RE)
        {
          // Create visibility queries
          obj->m_RE = rd->EF_CreateRE(eDATA_OcclusionQuery);
          CREOcclusionQuery *pRE = (CREOcclusionQuery *)obj->m_RE;
          glGenOcclusionQueriesNV(1, &pRE->m_nOcclusionID);
          pRE->m_nVisSamples = 0;
        }
        else
        {
          CREOcclusionQuery *pRE = (CREOcclusionQuery *)obj->m_RE;
          int nFrame = gRenDev->GetFrameID();

          if ( pRE->m_nCheckFrame != nFrame )
          {
		        if(pRE->m_nCheckFrame)
		        {
              double time = sCycles2();
			        pRE->m_nVisSamples=0;
              // Stupidly block until we have a query result
        			glGetOcclusionQueryuivNV(pRE->m_nOcclusionID, GL_PIXEL_COUNT_NV, &pRE->m_nVisSamples);
              rd->m_RP.m_PS.m_fOcclusionTime += (float)((sCycles2()+34-time)*1000.0*g_SecondsPerCycle);
            }
            pRE->m_nCheckFrame = nFrame;
          }
        }
        CREOcclusionQuery *pRE = (CREOcclusionQuery *)obj->m_RE;
        uint nVizQuery = 0;
        if (pRE)
          nVizQuery = pRE->m_nOcclusionID;
        Vec3d vx0, vy0, v;
        v = or - rd->m_prevCamera.GetPos();
        float dist = v.Length();
        if (m_fDistSizeFactor != 1.0f)
          dist = cry_powf(dist, m_fDistSizeFactor);
        float fScaleCorona = m_fScaleCorona;
        dist *= fScaleCorona * CRenderer::CV_r_coronasizescale;
        vx0 = camVecs[1] * dist * 0.1f * m_fVisAreaScale;
        vy0 = camVecs[2] * dist * 0.1f * m_fVisAreaScale;
        rd->EF_SetState(GS_NOCOLMASK);
        CGLTexMan::BindNULL(1);
        rd->EF_SelectTMU(0);

        glBeginOcclusionQueryNV(nVizQuery);

        Vec3d ProjV[4];
        Vec3d vQuad[4];

        glBegin(GL_QUADS);

        vQuad[0] = or + vx0 + vy0;
        glTexCoord2f(0, 0);
        glVertex3fv(&vQuad[0][0]);

        vQuad[1] = or + vx0 - vy0;
        glTexCoord2f(1, 0);
        glVertex3fv(&vQuad[1][0]);

        vQuad[2] = or - vx0 - vy0;
        glTexCoord2f(1, 1);
        glVertex3fv(&vQuad[2][0]);

        vQuad[3] = or - vx0 + vy0;
        glTexCoord2f(0, 1);
        glVertex3fv(&vQuad[3][0]);

        glEnd();

        glEndOcclusionQueryNV();

        for (int n=0; n<4; n++)
        {
          rd->ProjectToScreen(vQuad[n].x, vQuad[n].y, vQuad[n].z, &ProjV[n].x, &ProjV[n].y, &ProjV[n].z);
          ProjV[n].x = ProjV[n].x / 100 * (float)vp[2];
          ProjV[n].y = ProjV[n].y / 100 * (float)vp[3];
        }
        float nX = fabsf(ProjV[1].x - ProjV[0].x);
        float nY = fabsf(ProjV[2].y - ProjV[0].y);

        float area = nX * nY; //(float)(gRenDev->GetWidth() * gRenDev->GetHeight());
        fIntens = (float)pRE->m_nVisSamples / area;
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
        //obj->m_TempVars[0] = fIntens; //Min(1.0f, ((float)pRE->m_nVisSamples+50.0f) / 100.0f);
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
      int n = 1;
      int sizeMask  = 64;
      int sizeMask2  = sizeMask/2;
      int sizeTex  = 128; //m_Map->GetWidth();

      if (!obj->m_TexId0)
      {
        glGenTextures(1, (uint *)&obj->m_TexId0);
        assert(obj->m_TexId0<14000);
        rd->m_TexMan->SetTexture(obj->m_TexId0, eTT_Base);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, sizeMask, sizeMask, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      }
      if (!obj->m_TexId1)
      {
        glGenTextures(1, (uint *)&obj->m_TexId1);
        assert(obj->m_TexId0<14000);
        rd->m_TexMan->SetTexture(obj->m_TexId1, eTT_Base);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, sizeTex, sizeTex, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      }
      bool bFog = rd->m_FS.m_bEnable;
      rd->EnableFog(false);

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0.0, rd->GetWidth(), 0.0, rd->GetHeight(), -20.0, 0.0);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();

      rd->SetCullMode(R_CULL_NONE);
      rd->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

      gRenDev->m_TexMan->m_Text_White->Set();

      float fsizeMask2 = (float)sizeMask2;

      // set alpha to 0
      {
        rd->EF_SetState(GS_COLMASKONLYALPHA | GS_NODEPTHTEST);

        glColor4f(0,0,0,0);
        glDisableClientState(GL_COLOR_ARRAY);

        glBegin(GL_QUADS);

        glTexCoord2f(0, 0);
        glVertex2f(fsx-fsizeMask2, fsy-fsizeMask2);

        glTexCoord2f(1, 0);
        glVertex2f(fsx+fsizeMask2, fsy-fsizeMask2);

        glTexCoord2f(1, 1);
        glVertex2f(fsx+fsizeMask2, fsy+fsizeMask2);

        glTexCoord2f(0, 1);
        glVertex2f(fsx-fsizeMask2, fsy+fsizeMask2);

        glEnd();

        rd->m_nPolygons += 2;
      }

      // Set alpha mask of visible areas (1 is visible)
      {
        rd->EF_SetState(GS_COLMASKONLYALPHA);

        glColor4f(1,1,1,1);

        glBegin(GL_QUADS);

        glTexCoord2f(0, 0);
        glVertex2f(fsx-fsizeMask2, fsy-fsizeMask2);

        glTexCoord2f(1, 0);
        glVertex2f(fsx+fsizeMask2, fsy-fsizeMask2);

        glTexCoord2f(1, 1);
        glVertex2f(fsx+fsizeMask2, fsy+fsizeMask2);

        glTexCoord2f(0, 1);
        glVertex2f(fsx-fsizeMask2, fsy+fsizeMask2);

        glEnd();

        rd->m_nPolygons += 2;
      }

      // Read z-mask to target texture 0 as source for bluring
      rd->SetTexture(obj->m_TexId0, eTT_Base);

  		glReadBuffer(GL_BACK);
      if (SUPPORTS_GL_SGIS_generate_mipmap)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)fsx-sizeMask2, (int)fsy-sizeMask2, sizeMask, sizeMask);
      //if (SUPPORTS_GL_SGIS_generate_mipmap)
      //  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);


      /*{
        byte *data = new byte [sizeMask * sizeMask];
        byte *pic = new byte [sizeMask * sizeMask * 4];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
        for (int i=0; i<sizeMask*sizeMask; i++)
        {
          byte a = data[i];
          pic[i*4+0] = a;
          pic[i*4+1] = a;
          pic[i*4+2] = a;
          pic[i*4+3] = 255;
        }
        WriteTGA(pic,sizeMask,sizeMask,"SunMapAlpha.tga"); 
        delete [] pic;
        delete [] data;
      }*/

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

      gRenDev->m_TexMan->m_Text_White->Set();

      // set alpha to 0.2
      {
        gRenDev->EF_SetState(GS_COLMASKONLYALPHA | GS_NODEPTHTEST);

        glColor4f(1,1,1,0.2f);

        glBegin(GL_QUADS);

        glTexCoord2f(0, 0);
        glVertex2f(fPosx-fSizeTex2, fPosy-fSizeTex2);

        glTexCoord2f(1, 0);
        glVertex2f(fPosx+fSizeTex2, fPosy-fSizeTex2);

        glTexCoord2f(1, 1);
        glVertex2f(fPosx+fSizeTex2, fPosy+fSizeTex2);

        glTexCoord2f(0, 1);
        glVertex2f(fPosx-fSizeTex2, fPosy+fSizeTex2);

        glEnd();

        gRenDev->m_nPolygons += 2;
      }

      gRenDev->SetTexture(obj->m_TexId0, eTT_Base);
      // Radially blur the alpha texture
      for (int pass=0; pass<nPasses; pass++)
      {
        gRenDev->EF_SetState(GS_COLMASKONLYALPHA | GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);

        glBegin(GL_QUADS);

        glColor4f(1, 1, 1, fDeltAlpha);

        glTexCoord2f(0, 0);
        glVertex3f(fPosx-fCur, fPosy-fCur, 0);

        glTexCoord2f(1.0f, 0);
        glVertex3f(fPosx+fCur, fPosy-fCur, 0);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(fPosx+fCur, fPosy+fCur, 0);

        glTexCoord2f(0, 1.0f);
        glVertex3f(fPosx-fCur, fPosy+fCur, 0);

        glEnd();

        fCur += fDelt;
        fCurAlpha += fDeltAlpha;
      }

      rd->m_nPolygons += 2 * nPasses;

      // Generate blured alpha texture without mip-maps
      rd->SetTexture(obj->m_TexId1, eTT_Base);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)(fPosx-fSizeTex2), (int)(fPosy-fSizeTex2), sizeTex, sizeTex);

      /*{
        byte *data = new byte [sizeTex * sizeTex];
        byte *pic = new byte [sizeTex * sizeTex * 4];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
        for (int i=0; i<sizeTex*sizeTex; i++)
        {
          byte a = data[i];
          pic[i*4+0] = a;
          pic[i*4+1] = a;
          pic[i*4+2] = a;
          pic[i*4+3] = 255;
        }
        WriteTGA(pic,sizeTex,sizeTex,"SunMapAlpha_Ready.tga"); 
        delete [] pic;
        delete [] data;
      }*/

      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);

      rd->EnableFog(bFog);

      rd->SetTexture(0, eTT_Base);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }  

  return true;
}

void CREFlare::mfDrawFlares(SShader *ef, CFColor &col)
{
  CSunFlares *lfl = ef->m_Flares;

  int i;
  SSunFlare *fl;
  Vec3d lv;
  
  if (!CRenderer::CV_r_flares || !lfl)
    return;
    
  CCObject *obj = gRenDev->m_RP.m_pCurObject;

  Vec3d or = obj->GetTranslation();  
  if (obj->m_ObjFlags & FOB_DRSUN)
  {
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
  
  gRenDev->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
  gRenDev->SetCullMode(R_CULL_NONE);
  
  Vec3d VecX = gRenDev->m_RP.m_CamVecs[1];
  Vec3d VecY = gRenDev->m_RP.m_CamVecs[2];
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

void CREFlare::mfDrawCorona(SShader *ef, CFColor &col)
{
  Vec3d vx0, vy0, vx1, vy1, or, v, norm;

  if (!CRenderer::CV_r_coronas)
    return;
  
  CGLRenderer *rd = gcpOGL;
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
  
  rd->EF_PushFog();
  rd->EnableFog(false);

  vx0 = rd->m_RP.m_CamVecs[2];
  vx1 = vx0;
  vy0 = rd->m_RP.m_CamVecs[1];
  vy1 = vy0;

  v = or - gRenDev->m_RP.m_ViewOrg;
  float dist = v.Length();
  float distX = dist;
  if (m_fDistSizeFactor != 1.0f)
    distX = cry_powf(distX, m_fDistSizeFactor);
  float distY = distX;

  float fScaleCorona = m_fScaleCorona;
  if (m_pScaleCoronaParams)
    fScaleCorona = m_pScaleCoronaParams->mfGet();

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

  float fDecay = col[3];
  fDecay *= CRenderer::CV_r_coronacolorscale;
  norm = gRenDev->m_RP.m_CamVecs[0];
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

  CGLTexMan::BindNULL(1);
  rd->EF_SelectTMU(0);
  rd->GLSetCull(eCULL_None);
  rd->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

  CFColor c = col;

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
  bool bRays = bSun && CRenderer::CV_r_checkSunVis == 2;

  //if (CRenderer::CV_r_checkSunVis == 1 || CRenderer::CV_r_checkSunVis == 3 || !bSun || obj->m_TexId1<=0)
  {
    if (bRays)
      rd->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE);
    else
      rd->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
    if (m_Map)
      m_Map->Set();
    if (m_Pass)
    {
      // Render the 8 triangles from the data stream
      if (m_Pass && m_Pass->m_TUnits.Num())
      {
        CVProgram *curVP = NULL;
        CVProgram *newVP;
        SShaderPassHW *slw = m_Pass;
        if (slw->mfSetTextures())
        {
          newVP = slw->m_VProgram;

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
          rd->EF_CommitPS();
          rd->EF_CommitVS();
          if (rd->m_FS.m_bEnable)
            bFogOverrided = rd->EF_FogCorrection(false, false);

          {
            glColor4fv(&c[0]);
            glDisableClientState(GL_COLOR_ARRAY);
            gRenDev->m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;

            glBegin(GL_TRIANGLE_FAN);

            v = or;
            glTexCoord2f(0.5f, 0.5f);
            glVertex3fv(&v[0]);

            v = or + vx0 + vy0;
            glTexCoord2f(0, 0);
            glVertex3fv(&v[0]);

            v = or + vx0;
            glTexCoord2f(0, 0.5f);
            glVertex3fv(&v[0]);

            v = or + vx0 + vy1;
            glTexCoord2f(0, 1);
            glVertex3fv(&v[0]);

            v = or + vy1;
            glTexCoord2f(0.5f, 1);
            glVertex3fv(&v[0]);

            v = or + vx1 + vy1;
            glTexCoord2f(1, 1);
            glVertex3fv(&v[0]);

            v = or + vx1;
            glTexCoord2f(1, 0.5f);
            glVertex3fv(&v[0]);

            v = or + vx1 + vy0;
            glTexCoord2f(1, 0);
            glVertex3fv(&v[0]);

            v = or + vy0;
            glTexCoord2f(0.5f, 0);
            glVertex3fv(&v[0]);

            v = or + vx0 + vy0;
            glTexCoord2f(0, 0);
            glVertex3fv(&v[0]);

            glEnd();
          }

          rd->EF_FogCorrectionRestore(bFogOverrided);
        }

        slw->mfResetTextures();      
        rd->EF_ApplyMatrixOps(slw->m_MatrixOps, false);
      }
    }
    else
    {
      glColor4fv(&c[0]);
      glDisableClientState(GL_COLOR_ARRAY);
      gRenDev->m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;

      glBegin(GL_TRIANGLE_FAN);

      v = or;
      glTexCoord2f(0.5f, 0.5f);
      glVertex3fv(&v[0]);

      v = or + vx0 + vy0;
      glTexCoord2f(0, 0);
      glVertex3fv(&v[0]);

      v = or + vx0;
      glTexCoord2f(0, 0.5f);
      glVertex3fv(&v[0]);

      v = or + vx0 + vy1;
      glTexCoord2f(0, 1);
      glVertex3fv(&v[0]);

      v = or + vy1;
      glTexCoord2f(0.5f, 1);
      glVertex3fv(&v[0]);

      v = or + vx1 + vy1;
      glTexCoord2f(1, 1);
      glVertex3fv(&v[0]);

      v = or + vx1;
      glTexCoord2f(1, 0.5f);
      glVertex3fv(&v[0]);

      v = or + vx1 + vy0;
      glTexCoord2f(1, 0);
      glVertex3fv(&v[0]);

      v = or + vy0;
      glTexCoord2f(0.5f, 0);
      glVertex3fv(&v[0]);

      v = or + vx0 + vy0;
      glTexCoord2f(0, 0);
      glVertex3fv(&v[0]);

      glEnd();
    }
  }
  //else
  if (bRays)
  {
    rd->EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
    rd->EF_SelectTMU(0);
    rd->EnableTMU(true);  
    m_Map->Set();
    rd->EF_SelectTMU(1);
    rd->EnableTMU(true);  
    rd->m_TexMan->SetTexture(obj->m_TexId1, eTT_Base);

    CPShader *pRC = NULL;
    // If device supports register combiners use advanced sun rays
    if (rd->GetFeatures() & RFT_HW_RC && !rd->m_RP.m_RCSun)
    {
      pRC = CPShader::mfForName("CGRCSun");
      rd->m_RP.m_RCSun = pRC;
    }
    else
      pRC = rd->m_RP.m_RCSun;

    glColor3fv(&c[0]);
    glDisableClientState(GL_COLOR_ARRAY);

    if(pRC)
    {
      pRC->mfSet(true);  
      rd->EF_CommitPS();
    }

    glBegin(GL_TRIANGLE_FAN);

    v = or;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.5f, 0.5f);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.5f, 0.5f);
    glVertex3fv(&v[0]);

    v = or + vx0 + vy0;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 1);
    glVertex3fv(&v[0]);

    v = or + vx0;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0.5f);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 0.5f);
    glVertex3fv(&v[0]);

    v = or + vx0 + vy1;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 0);
    glVertex3fv(&v[0]);

    v = or + vy1;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.5f, 0);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.5f, 0);
    glVertex3fv(&v[0]);

    v = or + vx1 + vy1;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0);
    glVertex3fv(&v[0]);

    v = or + vx1;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0.5f);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0.5f);
    glVertex3fv(&v[0]);

    v = or + vx1 + vy0;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 1);
    glVertex3fv(&v[0]);

    v = or + vy0;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.5f, 1);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.5f, 1);
    glVertex3fv(&v[0]);

    v = or + vx0 + vy0;
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 1);
    glVertex3fv(&v[0]);

    glEnd();

    if (pRC)
    {
      pRC->mfSet(false);
      rd->EF_CommitPS();
    }
    rd->EnableTMU(false);  
    rd->EF_SelectTMU(0);
  }

  rd->m_nPolygons += 8;
  rd->EF_PopFog();
}

bool CREFlare::mfDraw(SShader *ef, SShaderPass *sfm)
{
  if (!CRenderer::CV_r_flares && !CRenderer::CV_r_coronas)
    return false;

  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  if ((obj->m_ObjFlags & FOB_DRSUN) && (gRenDev->m_RP.m_PersFlags & RBPF_DONTDRAWSUN))
    return false;

  float fBrightness = (obj->m_AmbColor[0]+obj->m_AmbColor[1]+obj->m_AmbColor[2]+
                       obj->m_Angs2[0]+obj->m_Angs2[1]+obj->m_Angs2[2]+
                       obj->m_TempVars[3]+obj->m_TempVars[4]) * 0.125f;

  if (!fBrightness)
    return false;

  Matrix44 m = gRenDev->GetCamera().GetVCMatrixD3D9();
  glLoadMatrixf(&m(0,0));

  obj->m_Color.r = m_Color.r;
  obj->m_Color.g = m_Color.g;
  obj->m_Color.b = m_Color.b;
  obj->m_Color.a = fBrightness;

  mfDrawCorona(ef, obj->m_Color);
  mfDrawFlares(ef, obj->m_Color);

  return true;
}

//=====================================================================================

bool CREClearStencil::mfDraw(SShader *ef, SShaderPass *sfm)
{    
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
  return true;
}

bool CREHDRProcess::mfDraw(SShader *ef, SShaderPass *sfm)
{
  return true;
}