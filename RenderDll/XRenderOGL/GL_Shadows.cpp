//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File:GlRenderer.cpp
//  Description: Implementation of the OpenGL renderer API
//  shadow map calculations
//
//  History:
//  -Jan 31,2001:Created by Vladimir Kajain
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include <IEntityRenderState.h>
#include "../Common/shadow_renderer.h"
#include "../Common/RendElements/CREScreenCommon.h"
#include "GLCGVProgram.h"
#include "GLCGPShader.h"

#include "I3dengine.h"

void WriteTGA8(byte *data8, int width, int height, char *filename);
void BlurImage8(byte * pImage, int nSize, int nPassesNum);
void MakePenumbraTextureFromDepthMap(byte * pDepthMapIn, int nSize, byte * pPenumbraMapOut);

void CGLRenderer::DrawAllShadowsOnTheScreen()
{
	float width=800;
	float height=600;
	Set2DMode(true, (int)width, (int)height);

	float fArrDim = max(4, sqrt(float(MAX_DYNAMIC_SHADOW_MAPS_COUNT)));
	float fPicDimX = width/fArrDim;
	float fPicDimY = height/fArrDim;
	int nShadowId=0;
  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
	for(float x=0; nShadowId<MAX_DYNAMIC_SHADOW_MAPS_COUNT && x<width-10;  x+=fPicDimX)
	for(float y=0; nShadowId<MAX_DYNAMIC_SHADOW_MAPS_COUNT && y<height-10; y+=fPicDimY)
	{
		ShadowMapTexInfo * pInf = &m_ShadowTexIDBuffer[nShadowId++];
		if(pInf->nTexId && pInf->pOwner)
		{
			STexPic *tp = (STexPic *)EF_GetTextureByID(pInf->nTexId);
			if (tp)
			{
				byte bSaveProj = tp->m_RefTex.bProjected;
				tp->m_RefTex.bProjected = 1;
				SetState( GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST );
				Draw2dImage(x, y, fPicDimX, fPicDimY, pInf->nTexId, 0,0,1,1,180);
				TextToScreen(x/width*100.f, y/height*100.f,
					"%8s", pInf->pOwner->GetName());
				tp->m_RefTex.bProjected = bSaveProj;
			}
		}
	}

	Set2DMode(false, m_width, m_height);
}

void CGLRenderer::BlurImage(int nSizeX, int nSizeY, int nType, int nTexIdSrc, int nTexIdTmp)
{
  if (nType == 1)
  {
    EF_SetState(GS_NODEPTHTEST);
    m_TexMan->SetTexture(nTexIdSrc, eTT_Base);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, nSizeX, nSizeY, 0);
    Set2DMode(true, 1, 1);

    struct_VERTEX_FORMAT_P3F_TEX2F pScreenBlur[] =  
    {
      Vec3(0, 1, 0), 0, 0,   
      Vec3(0, 0, 0), 0, 1,    
      Vec3(1, 1, 0), 1, 0,   
      Vec3(1, 0, 0), 1, 1,   
    }; 
  	EF_SetColorOp(eCO_REPLACE, eCO_REPLACE, DEF_TEXARG0, DEF_TEXARG0);
    DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  
 
    EF_SetState(GS_NODEPTHTEST | GS_COLMASKONLYALPHA);

    // set current vertex/fragment program    
    CCGVProgram_GL *vpBlur=(CCGVProgram_GL *)m_RP.m_VPBlur;
    CCGPShader_GL *fpBlur=(CCGPShader_GL *)m_RP.m_RCBlur;
    if (vpBlur && fpBlur)
    {
      vpBlur->mfSet(true, 0);
      fpBlur->mfSet(true, 0);
      // setup texture offsets, for texture neighboors sampling
      float s1=1.0f/(float) nSizeX;     
      float t1=1.0f/(float) nSizeY; 
      float pfOffset0[]={  s1*0.5f,    t1, 0.0f, 0.0f}; 
      float pfOffset1[]={  -s1,   t1*0.5f, 0.0f, 0.0f}; 
      float pfOffset2[]={ -s1*0.5f,   -t1, 0.0f, 0.0f}; 
      float pfOffset3[]={  s1,     -t1*0.5f, 0.0f, 0.0f};  
      EF_SelectTMU(0);
      m_TexMan->SetTexture(nTexIdSrc, eTT_Base);
      EF_SelectTMU(1);
      m_TexMan->SetTexture(nTexIdSrc, eTT_Base);
      EF_SelectTMU(2);
      m_TexMan->SetTexture(nTexIdSrc, eTT_Base);
      EF_SelectTMU(3);
      m_TexMan->SetTexture(nTexIdSrc, eTT_Base);
      vpBlur->mfParameter4f("Offset0", pfOffset0);
      vpBlur->mfParameter4f("Offset1", pfOffset1);
      vpBlur->mfParameter4f("Offset2", pfOffset2);
      vpBlur->mfParameter4f("Offset3", pfOffset3);
      int iBlurAmount = CV_r_shadowblur-1;
      EF_CommitPS();
      EF_CommitVS();
      for(int iBlurPasses=1; iBlurPasses<=iBlurAmount; iBlurPasses++) 
      {
        // set texture coordinates scale (needed for rectangular textures in gl ..)
        float pfScale[]={ 1.0f, 1.0f, 1.0f, (float) iBlurPasses};     
        vpBlur->mfParameter4f("vTexCoordScale", pfScale);

        // set current rendertarget
        //pRenderer->m_pd3dDevice->SetRenderTarget( 0, pTexSurf);
        // render screen aligned quad...
        DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  
      }
      vpBlur->mfSet(false, 0);
      fpBlur->mfSet(false, 0);
      EF_CommitPS();
      EF_CommitVS();
      EnableTMU(false);
      EF_SelectTMU(2);
      EnableTMU(false);
      EF_SelectTMU(1);
      EnableTMU(false);
      EF_SelectTMU(0);
    }
    Set2DMode(false, 1, 1);
  }
  else
  if (nType == 2)
  {
    assert(false);
  }
}

void CGLRenderer::OnEntityDeleted(IEntityRender * pEntityRender)
{
}

// render depth/shadow map into texture from light source position
void CGLRenderer::PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid)
{
  PROFILE_FRAME(Prep_PrepareDepthMap);

	if(!lof || !lof->pLs)
		return;

  static int nCurTexIdSlot = 0;
  //lof->bUpdateRequested = true;

  lof->nTexSize = max(lof->nTexSize, 32);
  if(lof->nTexIdSlot>=0)
  {
    if(m_ShadowTexIDBuffer[lof->nTexIdSlot].pOwner == lof->pOwner)
    {
      char * pName = 0;
      if(lof->pOwner)
        pName = (char*)lof->pOwner->GetName();
      if(m_ShadowTexIDBuffer[lof->nTexIdSlot].pOwnerGroup == lof->pOwnerGroup)
        if(m_ShadowTexIDBuffer[lof->nTexIdSlot].dwFlags == lof->dwFlags)
          if(lof->depth_tex_id && !lof->bUpdateRequested)
          {
            m_ShadowTexIDBuffer[lof->nTexIdSlot].nLastFrameID = GetFrameID();
            return;
          }
    }
  }

  int nShadowTexSize = lof->nTexSize;
  lof->bUpdateRequested = false;
  //assert(lof->nTexSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Render objects into frame and Z buffers
////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // remember fog value
  int bFog=0; 
  glGetIntegerv(GL_FOG,&bFog);
  glDisable(GL_FOG);

  // glPush
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  // check extension
  bool bDepth = (m_Features & RFT_DEPTHMAPS) != 0;
  //if(!bSGIX_shadow && lof->shadow_type == EST_DEPTH_BUFFER)
  //  return;

  // normalize size
  while(nShadowTexSize>m_height || nShadowTexSize>m_width)
    nShadowTexSize/=2;

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "   Really generating %dx%d shadow map for %s \n", nShadowTexSize, nShadowTexSize, lof->pOwner ? lof->pOwner->GetName() : "NoOwner");

  // setup matrices
  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  gRenDev->SetViewport(0, 0, nShadowTexSize, nShadowTexSize);  

  float lightFrustumMatrix[16];
  float lightViewMatrix[16];

  glMatrixMode(GL_PROJECTION);
  makeProjectionMatrix(lof->FOV, lof->ProjRatio, lof->min_dist, lof->max_dist, lightFrustumMatrix);
  glLoadMatrixf(lightFrustumMatrix);

  glMatrixMode(GL_MODELVIEW);
  SGLFuncs::gluLookAt( lof->pLs->vSrcPos.x, lof->pLs->vSrcPos.y, lof->pLs->vSrcPos.z, 
             lof->target.x,   lof->target.y,   lof->target.z, 
             0, 0, 1, lightViewMatrix);
  glLoadMatrixf(lightViewMatrix);
  //iLog->Log("Generate SM: From (%.3f,%.3f,%.3f) To (%.3f,%.3f,%.3f), FOV: %.3f, MinD: %.3f, MaxD: %.3f", lof->pLs->vSrcPos.x, lof->pLs->vSrcPos.y, lof->pLs->vSrcPos.z, lof->target.x, lof->target.y, lof->target.z, lof->FOV, lof->min_dist, lof->max_dist);

  memcpy(lof->debugLightFrustumMatrix,lightFrustumMatrix,sizeof(lof->debugLightFrustumMatrix));
  memcpy(lof->debugLightViewMatrix,   lightViewMatrix,   sizeof(lof->debugLightViewMatrix));

  // clear frame buffer
  CFColor col;
  if (!(m_Features & RFT_DEPTHMAPS) && (m_Features & RFT_SHADOWMAP_SELFSHADOW))
    col = CFColor(1,0.879f,0,0);
  else
    col = CFColor(0,0,0,0);
  EF_ClearBuffers(true, false, &col[0]);

  // disable rendering of borders
  glScissor(1, 1, nShadowTexSize-2, nShadowTexSize-2);
  glEnable (GL_SCISSOR_TEST);

  IShader * pStateShader = EF_LoadShader("StateNoCull", eSH_World);

  // draw static objects (not entities)
  if(lof->pModelsList && lof->pModelsList->Count())
  { 
    // cut underground geometry
    float plane[] = {0,0,1,0};
    
    //if(!bDepth)
    // gRenDev->SetClipPlane(0,plane);

    EF_StartEf(); 
    for(int m=0; m<lof->pModelsList->Count(); m++)
    {
			SRendParams rParms;
			rParms.pStateShader = pStateShader;
			if(!lof->m_fBending)
				rParms.nShaderTemplate = EFT_WHITESHADOW;

			// set pos relative to entity 0
			if(lof->pEntityList && lof->pEntityList->Count())
				rParms.vPos -= (*lof->pEntityList)[0]->GetPos();
			rParms.dwFObjFlags |= FOB_TRANS_MASK;
			rParms.dwFObjFlags |= FOB_RENDER_INTO_SHADOWMAP;

			rParms.fBending = lof->m_fBending;

			(*lof->pModelsList)[m]->Render(rParms,Vec3(zero),0);
    }
    EF_EndEf3D(true);

    gRenDev->SetClipPlane(0,NULL);
  }

  // draw entities
	EF_StartEf(); 
	for(int m=0; lof->pEntityList && m_RP.m_pCurObject && m<lof->pEntityList->Count(); m++)
	{ 
		IEntityRender * pEnt  = (*lof->pEntityList)[m];
		const char *name = pEnt->GetName();
    Vec3d vOffSetDir = GetNormalized( m_RP.m_pCurObject->GetTranslation() - lof->pLs->vSrcPos )*(0.1f+0.035f*(256.f/nShadowTexSize));
		IEntityRender * pEnt0 = (*lof->pEntityList)[0];
		SRendParams rParams;
    rParams.nShaderTemplate = bDepth ? EFT_WHITE : EFT_WHITESHADOW;
		rParams.pStateShader = pStateShader;
    rParams.vPos = vOffSetDir + (pEnt->GetPos() - lof->pOwner->GetPos());
    rParams.dwFObjFlags |= FOB_RENDER_INTO_SHADOWMAP;
    rParams.dwFObjFlags |= FOB_TRANS_MASK;
		pEnt->DrawEntity(rParams);
	}
	EF_EndEf3D(true);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Now make texture from frame buffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////

  if(make_new_tid)
  { // new id for static objects
    glGenTextures(1,&lof->depth_tex_id);
		assert(lof->depth_tex_id<14000);
  }
  else
  { 
    // try to reuse slot if it was not modified
    if( lof->nTexIdSlot>=0 &&
      m_ShadowTexIDBuffer[lof->nTexIdSlot].nTexId == lof->depth_tex_id &&
      m_ShadowTexIDBuffer[lof->nTexIdSlot].nTexSize == nShadowTexSize &&
      m_ShadowTexIDBuffer[lof->nTexIdSlot].pOwner == lof->pOwner &&
      m_ShadowTexIDBuffer[lof->nTexIdSlot].pOwnerGroup == lof->pOwnerGroup &&
			m_ShadowTexIDBuffer[lof->nTexIdSlot].dwFlags == lof->dwFlags)
    {
      nCurTexIdSlot = lof->nTexIdSlot;
    }
    else
    { // find oldest slot
      int nOldestSlot = -1;
      int nOldestFrameId = GetFrameID();
      for(int i=0; i<MAX_DYNAMIC_SHADOW_MAPS_COUNT; i++)
      {
        if(m_ShadowTexIDBuffer[i].nLastFrameID < nOldestFrameId && nShadowTexSize == m_ShadowTexIDBuffer[i].nTexSize)
        {
          nOldestFrameId = m_ShadowTexIDBuffer[i].nLastFrameID;
          nOldestSlot = i;
        }
      }

      if(nOldestSlot<0)
        nCurTexIdSlot++;
      else
        nCurTexIdSlot=nOldestSlot;

			nCurTexIdSlot = nCurTexIdSlot;
    }
    
    if(nCurTexIdSlot>=MAX_DYNAMIC_SHADOW_MAPS_COUNT)
      nCurTexIdSlot=0;

    if(!m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId)
    {
      glGenTextures(1,&m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId);
			assert(m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId<14000);
      make_new_tid = true;
    }
    if (m_ShadowTexIDBuffer[nCurTexIdSlot].nTexSize != nShadowTexSize)
      make_new_tid = true;

    lof->nTexIdSlot = nCurTexIdSlot;
    lof->depth_tex_id = m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId;    

    m_ShadowTexIDBuffer[nCurTexIdSlot].pOwner = lof->pOwner;
    m_ShadowTexIDBuffer[nCurTexIdSlot].pOwnerGroup = lof->pOwnerGroup;
		m_ShadowTexIDBuffer[nCurTexIdSlot].dwFlags = lof->dwFlags;
    m_ShadowTexIDBuffer[nCurTexIdSlot].nLastFrameID = GetFrameID();
    m_ShadowTexIDBuffer[nCurTexIdSlot].nTexSize = nShadowTexSize;
  }  
  ShadowMapTexInfo *st = NULL;
  if (CV_r_shadowblur && !(m_Features & RFT_DEPTHMAPS))
  {
    int i;
    for (i=0; i<m_TempShadowTextures.Num(); i++)
    {
      st = &m_TempShadowTextures[i];
      if (st->nTexSize == nShadowTexSize)
        break;
    }
    if (i == m_TempShadowTextures.Num())
    {
      ShadowMapTexInfo smt;
      smt.nTexId = 0;
      smt.nTexSize = nShadowTexSize;
      m_TempShadowTextures.AddElem(smt);
    }
    st = &m_TempShadowTextures[i];
    if (!st->nTexId)
    {
      glGenTextures(1,&st->nTexId);
      gRenDev->m_TexMan->SetTexture(st->nTexId, eTT_Base);
    }
    if (CV_r_shadowblur > 2 && !CRenderer::CV_r_nops20 && !st->nTexIdTemp)
      glGenTextures(1,&st->nTexIdTemp);
    int BlurType = CV_r_shadowblur-1;
    if (CV_r_shadowblur > 2 && CRenderer::CV_r_nops20)
      BlurType = 1;
    BlurImage(nShadowTexSize, nShadowTexSize, BlurType, st->nTexId, st->nTexIdTemp);
  }
  //assert(nShadowTexSize);
  //assert(lof->nTexSize);

	static ICVar * pVar = iConsole->GetCVar("e_shadow_maps_debug");
	if (pVar && pVar->GetIVal()==1)
  {
    int width = nShadowTexSize;
    int height = nShadowTexSize;
    if (bDepth)
    {
      byte *pic = new byte [width * height];
      glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pic);    
      char buff[128];
      sprintf(buff, "ShadowMap%00d.tga", nCurTexIdSlot);
      WriteTGA8(pic,width,height,buff); 
      delete [] pic;
    }
    else
    {
      byte *pic = new byte [width * height * 4];
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);    
      char buff[128];
      sprintf(buff, "ShadowMap%00d.tga", nCurTexIdSlot);
      ::WriteTGA(pic,width,height,buff,32); 
      delete [] pic;
    }
  }

  gRenDev->m_TexMan->SetTexture(lof->depth_tex_id, eTT_Base);

  // make texture of requested type
  if(bDepth)
  {
    glCopyTexImage2D( GL_TEXTURE_2D,           
      0,             
      bDepth ? (m_zbpp == 24 ? GL_DEPTH_COMPONENT24_SGIX : GL_DEPTH_COMPONENT16_SGIX) : GL_DEPTH_COMPONENT,
      0,                 
      0,                 
      nShadowTexSize,           
      nShadowTexSize,          
      0);
  }
  else if(lof->shadow_type == EST_PENUMBRA)
  { // make penumbra texture
    int width = nShadowTexSize;
    int height = nShadowTexSize;
    byte * pDepthMap = new byte [width * height];
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pDepthMap);
    WriteTGA8(pDepthMap,width,height,"_DepthMap.tga"); 

    byte * pPenumbra = new byte [width * height];
    assert(width==height);
    MakePenumbraTextureFromDepthMap(pDepthMap, width, pPenumbra);
    WriteTGA8(pPenumbra,width,height,"_Penumbra.tga"); 
    delete [] pDepthMap;

    // make gl texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, width, width, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pPenumbra);
    delete [] pPenumbra;
  }
  else
  {
    if(1)//lof->dynamic)
    { // entities
      if(make_new_tid)
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, nShadowTexSize, nShadowTexSize, 0);
      else
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, nShadowTexSize, nShadowTexSize);
    }
    else
    { // static objects
      assert(nShadowTexSize<=512);
      byte *depth_map = new byte[512*512];
      glReadPixels(0, 0, nShadowTexSize, nShadowTexSize, GL_RED, GL_UNSIGNED_BYTE, depth_map);
     
      for(int x=0; x<nShadowTexSize; x++)
      for(int y=0; y<nShadowTexSize; y++)
      {
        byte b;
        if(depth_map[x+y*nShadowTexSize] < 12) // we render normal object, not black
          b = 0;
        else
          b = 255;

        // clear border
        if(x==0 || x==nShadowTexSize-1)
        if(y==0 || y==nShadowTexSize-1)
          b = 0;

        depth_map[x+y*nShadowTexSize] = b;
      }
     
      glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_ALPHA_ARB,//GL_ALPHA,
        nShadowTexSize, nShadowTexSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, depth_map);

      delete [] depth_map;
    }
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if(bDepth)
  {  // enable extension
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_SGIX, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_OPERATOR_SGIX, GL_TEXTURE_LEQUAL_R_SGIX);
  }

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  gRenDev->SetViewport(vX, vY, vWidth, vHeight);
  gRenDev->SetScissor(0, 0, 0, 0);
  glDisable(GL_SCISSOR_TEST);

	glClearColor(m_vClearColor.x,m_vClearColor.y,m_vClearColor.z,0);
  SetState(GS_DEPTHWRITE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(bFog)
    glEnable(GL_FOG);

  if(lof->pPenumbra && lof->pPenumbra->bUpdateRequested)
    PrepareDepthMap(lof->pPenumbra, make_new_tid);
}

void MakePenumbraTextureFromDepthMap(byte * pDepthMapIn, int nSize, byte * pPenumbraMapOut)
{
  memcpy(pPenumbraMapOut,pDepthMapIn,nSize*nSize);

  BlurImage8(pPenumbraMapOut, nSize, 3);

#define DATA_TMP(_x,_y) (pTemp[(_x)+nSize*(_y)])
#define DATA(_x,_y) (pPenumbraMapOut[(_x)+nSize*(_y)])

  { // substract
    for(int x=0; x<nSize; x++)
    for(int y=0; y<nSize; y++)
    {
      float fVal = (float)DATA(x,y) - (float)pDepthMapIn[x+nSize*y];
      
     fVal = (fVal*2.f) + 127.f;

      DATA(x,y) = uchar( max(min(fVal,255),0) );
    }
  }

  memcpy(pDepthMapIn,pPenumbraMapOut,nSize*nSize);

  { // per fragment normalization
    int nRange = 1;
    for(int X=nRange; X<nSize-nRange; X++)
    for(int Y=nRange; Y<nSize-nRange; Y++)
    {     
//      DATA(X,Y) = DATA(X,Y)>4 ? 255 : 0;
    }
  }

/*
  { // per fragment normalization
    int nRange = 8;
    for(int X=nRange; X<nSize-nRange-1; X++)
    for(int Y=nRange; Y<nSize-nRange-1; Y++)
    {     
      float fMax = 0;
      for(int x=X-nRange; x<=X+nRange; x++)
      for(int y=Y-nRange; y<=Y+nRange; y++)
      {
        if(fMax < pDepthMapIn[x+nSize*y])
          fMax = pDepthMapIn[x+nSize*y];
      }
      
      if(fMax)
      {
        float fValue = (float)pDepthMapIn[X+nSize*Y];
        float fNewValue = (fValue/fMax)*255.f;
        DATA(X,Y) = uchar( max(min(fNewValue,255),0) );
      }
      else
      {
        DATA(X,Y) = 0;
      }
    }
  }*/

#undef DATA
#undef DATA_IN

//  BlurImage8(pPenumbraMapOut, nSize, 1);
}

inline void glShadowVertex(int x, int y) 
{ 
  glVertex3f((float)x, (float)y, iSystem->GetI3DEngine()->GetTerrainZ(x,y)+0.05f);//iTerrain->GetZSafe(x,y)/*+0.05f*/); 
}

void CGLRenderer::SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3d * vShadowTrans, const float fShadowScale, 
                                      Vec3d vObjTrans, float fObjScale, const Vec3d vObjAngles, Matrix44 * pObjMat)
{  
  if(!pFrustum)
    return;

  float lightFrustumMatrix[16];
  float lightViewMatrix[16];

  if(vShadowTrans)
  { // make tmp matrix for this obj position if shadow frustum is not translated (translate original mats)
 //   float fDist = (pFrustum->min_dist + pFrustum->max_dist)*0.5f;
//    float fRadius = (pFrustum->max_dist - pFrustum->min_dist)*0.5f*fShadowScale;
    makeProjectionMatrix(pFrustum->FOV*fShadowScale, pFrustum->ProjRatio, pFrustum->min_dist, pFrustum->max_dist, lightFrustumMatrix);
    
    Vec3d mv_trans = *vShadowTrans; // - vObjTrans;
    float mat[16];

    SGLFuncs::gluLookAt( 
      pFrustum->pLs->vSrcPos.x+mv_trans.x, 
      pFrustum->pLs->vSrcPos.y+mv_trans.y, 
      pFrustum->pLs->vSrcPos.z+mv_trans.z, 
      fShadowScale*pFrustum->target.x+mv_trans.x, 
      fShadowScale*pFrustum->target.y+mv_trans.y, 
      fShadowScale*pFrustum->target.z+mv_trans.z, 
      0, 0, 1, mat );

    if(pObjMat)
      mathMatrixMultiply(lightViewMatrix, mat, pObjMat->GetData(), g_CpuFlags);
    else
    {
      mathRotateZ(mat, vObjAngles.z, g_CpuFlags);
      mathRotateY(mat, vObjAngles.y, g_CpuFlags);
      mathRotateX(mat, vObjAngles.x, g_CpuFlags);
      mathScale(mat, Vec3d(fObjScale,fObjScale,fObjScale), g_CpuFlags);
      memcpy(lightViewMatrix, mat, sizeof(float)*16);
    }
  }

  CGLTexMan::BindNULL(1);

  gcpOGL->ConfigShadowTexgen(Num, 0, pFrustum, lightFrustumMatrix, lightViewMatrix);
}

// Make 8-bit identity texture that maps (s)=(z) to [0,255]/255.
int MakeShadowIdentityTexture()
{ 
  uchar texmap[256];
  for (unsigned int i=0; i<256; i++) 
    texmap[i] = i;

  unsigned int tid;
  glGenTextures(1,&tid);
  assert(tid<14000);
  glBindTexture(GL_TEXTURE_1D, tid);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_INTENSITY8, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texmap);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return tid;
}

// setup projection texgen
void CGLRenderer::ConfigShadowTexgen(int Num, int rangeMap, ShadowMapFrustum * pFrustum, float * pLightFrustumMatrix, float * pLightViewMatrix)
{  
  float m1[16], m2[16], mtexSc[16];

  if (rangeMap) 
  {
    static int to_map_8bit = MakeShadowIdentityTexture();

    float RSmatrix[16] = 
    {
        0,    0,   0,   0,
        0,    0,   0,   0,
        0.5f, 128, 0,   0,
        0.5f, 128, 0,   1.0f
    };    

    if (Num >= 0)
		{
      m_RP.m_pRE->m_CustomTexBind[Num] = to_map_8bit;
			gRenDev->m_TexMan->SetTexture(to_map_8bit, eTT_Base);
		}
    else
    {
      glBindTexture(GL_TEXTURE_1D, to_map_8bit);
      glEnable(GL_TEXTURE_1D);
      glDisable(GL_TEXTURE_2D);
    }

    memcpy(mtexSc,RSmatrix,sizeof(m1));
  } 
  else 
  {
    float Smatrix[16] = 
    {
      0.5f, 0,    0,    0,
      0,    0.5f, 0,    0,
      0,    0,    0.5f, 0,
      0.5f, 0.5f, 0.5f, 1.0f
    };

    if (Num >= 0)
		{
			if(pFrustum->depth_tex_id<=0)
				iLog->Log("Warning: CGLRenderer::ConfigShadowTexgen: pFrustum->depth_tex_id not set");
			else
      {
        if (m_RP.m_pRE)
        {
          m_RP.m_pRE->m_CustomTexBind[Num] = pFrustum->depth_tex_id;
          m_RP.m_pRE->m_Color[Num] = pFrustum->fAlpha;
        }
        else
        {
          m_RP.m_RECustomTexBind[Num] = pFrustum->depth_tex_id;
          m_RP.m_REColor[Num] = pFrustum->fAlpha;
        }
      }
		}
    else
    {
      gRenDev->m_TexMan->SetTexture(pFrustum->depth_tex_id, eTT_Base);
    }

    memcpy(mtexSc,Smatrix,sizeof(m1));
  }

  mathMatrixMultiply(m2, pLightFrustumMatrix, pLightViewMatrix, g_CpuFlags);

  if (Num >= 0)
  {
    Matrix44 *mt = &gRenDev->m_cEF.m_TempMatrices[Num][0];
    float *pf = mt->GetData();
    mathMatrixMultiply(m1, mtexSc, m2, g_CpuFlags);
    mathMatrixTranspose(pf, m1, g_CpuFlags);

    if ((m_Features & RFT_SHADOWMAP_SELFSHADOW) && !(m_Features & RFT_DEPTHMAPS))
    {
      Matrix44 *mt = &gRenDev->m_cEF.m_TempMatrices[Num][7];
      float *pf = mt->GetData();
      mathMatrixTranspose(pf, m2, g_CpuFlags);
    }
  }
  else
  {
    assert(false);
  }
}

//#include "nvparse/nvparse.h"

///////////////////////////////////////////////////////////////////
// Render shadows on the object as first pass
///////////////////////////////////////////////////////////////////
/*bool CREShadowMap::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  if(m_pShadowFrustum==0 && m_fAlpha == -1)
  { // special case: draw all tree shadows
    iSystem->GetI3DEngine()->DrawTerrainShadowMaps();
    return true;
  }

  if(gRenDev->m_RP.m_pCurObject->m_Trans.GetDistance(Vec3d(512,512,100))>2000)
    return true;
//    iConsole->Exit("CREShadowMap::mfDraw");

  if(gRenDev->m_RP.m_pCurObject->m_ObjFlags != FOB_USEMATRIX)
    gRenDev->MakeMatrix(gRenDev->m_RP.m_pCurObject->m_Trans, gRenDev->m_RP.m_pCurObject->m_Angs, Vec3d(1,1,1), &(gRenDev->m_RP.m_pCurObject->m_Matrix));

  // draw shadow on the ground
  ShadowMapLightSource * pFrustum = (ShadowMapLightSource*)m_pShadowFrustum;
  if(pFrustum && pFrustum->m_LightFrustums.Count() && m_fAlpha>0)
  {
//    float alpha = gRenDev->m_RP.m_pCurObject->m_Trans.Distance(iSystem->GetViewCamera().GetPos());
  //  alpha = 1 - alpha/32;
    if(m_fAlpha>0)
    {
      // restore world space
      Matrix mat(gRenDev->m_RP.m_pCurObject->m_Matrix);
      mat.Invert();
      gRenDev->PushMatrix();
      gRenDev->MultMatrix(&mat.m_values[0][0]);
      gRenDev->ResetToDefault();
      gRenDev->DrawShadowGrid(gRenDev->m_RP.m_pCurObject->m_Trans, gRenDev->m_RP.m_pCurObject->m_Scale, 
        &pFrustum->m_LightFrustums[0], false, m_fAlpha, 0, 0);
      gRenDev->PopMatrix();
    }
  }

  return true;
	*/
/*  if(!m_pBuffer->m_pVertexBuffer || !gRenDev->m_RP.m_pCurObject->m_pShadowCasters)
    return true;

  list2<ShadowMapLightSourceInstance> * lsources = (list2<ShadowMapLightSourceInstance>*)gRenDev->m_RP.m_pCurObject->m_pShadowCasters;

  if(lsources->Count()==1)
    if((*lsources)[0].ls->m_LightFrustums.Count()==0)
      return true;

  // restore world space
  Matrix mat(gRenDev->m_RP.m_pCurObject->m_Matrix);
  mat.Invert();
  gRenDev->PushMatrix();
  gRenDev->MultMatrix(&mat.m_values[0][0]);
  gRenDev->ResetToDefault();

  // draw white
  gRenDev->ResetToDefault();
  gRenDev->EnableTMU(false);
  gRenDev->SetMaterialColor(1, 1, 1, 1);
  gRB.mStateIgnore |= (RBSI_ALPHAGEN | RBSI_RGBGEN);
  gRenDev->SetEnviMode(R_MODE_MODULATE);

  gRenDev->PushMatrix();
  gRenDev->MultMatrix(gRenDev->m_RP.m_pCurObject->m_Matrix);

  static int lid =0;
  if(!lid)
  {
    lid = glGenLists(1);
    glNewList(lid,GL_COMPILE);
    nvparse(
      "!!RC1.0"
      "out.rgb = col0;"
		  "out.a = tex0.a;"
      );
    glEndList();
  }

  glCallList(lid);

  glEnable(GL_REGISTER_COMBINERS_NV);

  char * const * const p = nvparse_get_errors();
  assert(!p[0]);

  gRenDev->EnableAlphaTest(true);
  gRenDev->SetCullMode(R_CULL_NONE);
  gRenDev->EnableTMU(true);

  gRB.mStateIgnore = 0;

  for(int m=0; m<m_pBuffer->m_pMats->Count(); m++)
  {
    CMatInfo * pMat = &(*m_pBuffer->m_pMats)[m];
    if(pMat->nNumIndices && pMat->pShader)
    {
      SShader *ef = pMat->pShader->mfGetTemplate(-1);
      int bind = ef->m_Layers[0].m_MTLayers[0]->m_TexPic->m_Bind;

      gRenDev->SetTexture(bind);

      gRenDev->DrawBuffer(m_pBuffer->m_pVertexBuffer, 
        &m_pBuffer->m_Indices[pMat->nFirstIndexId], 
        pMat->nNumIndices, 
        m_pBuffer->m_nPrimetiveType);
    }
  }

  gRenDev->PopMatrix();

  glDisable(GL_REGISTER_COMBINERS_NV);
  
  // draw closest shadows 
  for(int s=0; s<lsources->Count() && s<4; s++)
    if((*lsources)[s].ls->m_LightFrustums.Count())
  {
    gRenDev->ResetToDefault();
    gRenDev->SetCullMode(R_CULL_NONE);
    gRenDev->SetupShadowOnlyPass(&((*lsources)[s].ls->m_LightFrustums[0]), &((*lsources)[s].translation));

    gRenDev->EnableBlend(true);
    gRenDev->SetEnviMode(R_MODE_MODULATE);
    gRenDev->SetBlendMode(R_BLEND_MODE__ZERO__ONE_MINUS_SRC_COLOR);  
    gRenDev->SetDepthFunc(R_EQUAL);
    gRenDev->SetMaterialColor(1, 1, 1, 1);
    gRB.mStateIgnore |= (RBSI_ALPHAGEN | RBSI_RGBGEN);

    gRenDev->PushMatrix();
    gRenDev->MultMatrix(gRenDev->m_RP.m_pCurObject->m_Matrix);
    gRenDev->DrawBuffer(m_pBuffer->m_pVertexBuffer, &m_pBuffer->m_Indices[0], m_pBuffer->m_Indices.Count(), m_pBuffer->m_nPrimetiveType);
    gRenDev->PopMatrix();

    gRenDev->ResetToDefault();
  }

  gRenDev->PopMatrix();
              
  gRenDev->CheckError("CREShadowMap::mfDraw");
              
  return true;*/
/*}
*/
/*void CGLRenderer::ClearAlphaBuffer(float fAlphaValue)
{
  glColorMask(0,0,0,1);
  glColor4f(fAlphaValue,fAlphaValue,fAlphaValue,fAlphaValue);

  SelectTMU(1);
  EnableTMU(false);
  SelectTMU(0);
  EnableTMU(false);

  int x1=0,x2=m_width,y1=0,y2=m_height;  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(x1, x2, y2, y1, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
    
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glBegin(GL_QUADS);   
  glVertex2i(x1,y1);    
  glVertex2i(x2,y1);  
  glVertex2i(x2,y2);  
  glVertex2i(x1,y2);  
  glEnd();    
   
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);          

  EnableTMU(true);
}
*/