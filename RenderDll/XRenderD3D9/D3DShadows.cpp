/*=============================================================================
  D3DShadows.cpp : shadows support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include <IEntityRenderState.h>
#include "../Common/Shadow_Renderer.h"
#include "../Common/RendElements/CREScreenCommon.h"
#include "D3DCGVProgram.h"
#include "D3DCGPShader.h"

#include "I3dengine.h"

void WriteTGA8(byte *data8, int width, int height, char *filename);
void BlurImage8(byte * pImage, int nSize, int nPassesNum);
void MakePenumbraTextureFromDepthMap(byte * pDepthMapIn, int nSize, byte * pPenumbraMapOut);

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

// Texture coordinate rectangle
struct CoordRect
{
  float fLeftU, fTopV;
  float fRightU, fBottomV;
};

HRESULT GetTextureRect(STexPic *pTexture, RECT* pRect);
HRESULT GetSampleOffsets_GaussBlur5x5(DWORD dwD3DTexWidth, DWORD dwD3DTexHeight, D3DXVECTOR4* avTexCoordOffset, D3DXVECTOR4* avSampleWeight, FLOAT fMultiplier);
HRESULT GetSampleOffsets_GaussBlur5x5Bilinear(DWORD dwD3DTexWidth, DWORD dwD3DTexHeight, D3DXVECTOR4* avTexCoordOffset, D3DXVECTOR4* avSampleWeight, FLOAT fMultiplier);
HRESULT GetTextureCoords( STexPic *pTexSrc, RECT* pRectSrc, STexPic *pTexDest, RECT* pRectDest, CoordRect* pCoords);

void CD3D9Renderer::BlurImage(int nSizeX, int nSizeY, int nType, ShadowMapTexInfo *st, int nTexDst)
{
  IDirect3DSurface9 *pIRGBTargetSurf = NULL;
  IDirect3DSurface9 *pIZBufferSurf = NULL;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  LPDIRECT3DTEXTURE9 pIRGBTarget = NULL;
  HRESULT hReturn;

  STexPicD3D *tpSrc = (STexPicD3D *)m_TexMan->GetByID(st->nTexId0);
  STexPicD3D *tpDst = (STexPicD3D *)m_TexMan->GetByID(nTexDst);
  if (tpDst)
  {
    pID3DTexture = (LPDIRECT3DTEXTURE9)tpDst->m_RefTex.m_VidTex;
    hReturn = pID3DTexture->GetSurfaceLevel(0, &pIRGBTargetSurf);
  }

  m_RP.m_PersFlags &= ~(RBPF_VSNEEDSET | RBPF_PS1NEEDSET);
  m_RP.m_FlagsModificators = 0;
  m_RP.m_CurrentVLights = 0;
  m_RP.m_FlagsPerFlush = 0;
  EF_CommitShadersState();
  EF_CommitVLightsState();
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
  if (m_RP.m_TexStages[0].TCIndex != 0)
  {
    m_RP.m_TexStages[0].TCIndex = 0;
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
  }
  EF_Scissor(false, 0, 0, 0, 0);
  SetCullMode(R_CULL_NONE);

  CD3D9TexMan::BindNULL(1);
  EF_SelectTMU(0);
  if(tpSrc)
  {
    tpSrc->m_RefTex.m_MinFilter=D3DTEXF_LINEAR; 
    tpSrc->m_RefTex.m_MagFilter=D3DTEXF_LINEAR;
    tpSrc->m_RefTex.bRepeats=false;  
    tpSrc->Set(); 
  }
  hReturn = m_pd3dDevice->SetRenderTarget(0, pIRGBTargetSurf);
  EF_SetState(GS_NODEPTHTEST);
	EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  DrawQuad(0,0,(float)nSizeX,(float)nSizeY,Col_White,1);
  if (nType == 1 || (nType > 1 && !(m_Features & RFT_HW_PS20)))
  {
    EF_SetState(GS_NODEPTHTEST | GS_COLMASKONLYALPHA);
    Set2DMode(true, 1, 1);

    // set current vertex/fragment program    
    CCGVProgram_D3D *vpBlur=(CCGVProgram_D3D *)m_RP.m_VPBlur;
    CCGPShader_D3D *fpBlur=(CCGPShader_D3D *)m_RP.m_RCBlur;
    if (vpBlur && fpBlur)
    {
      vpBlur->mfSet(true, 0);
      fpBlur->mfSet(true, 0);
      // setup texture offsets, for texture neighboors sampling
      float s1=1.0f/(float) nSizeX;     
      float t1=1.0f/(float) nSizeY; 
      float s_off=s1*0.5f; 
      float t_off=t1*0.5f; 
      float pfOffset0[]={  s1*0.5f,    t1, 0.0f, 0.0f}; 
      float pfOffset1[]={  -s1,   t1*0.5f, 0.0f, 0.0f}; 
      float pfOffset2[]={ -s1*0.5f,   -t1, 0.0f, 0.0f}; 
      float pfOffset3[]={  s1,     -t1*0.5f, 0.0f, 0.0f};  
      EF_SelectTMU(0);
      tpSrc->Set();
      EF_SelectTMU(1);
      tpSrc->Set();
      EF_SelectTMU(2);
      tpSrc->Set();
      EF_SelectTMU(3);
      tpSrc->Set();
      vpBlur->mfParameter4f("Offset0", pfOffset0);
      vpBlur->mfParameter4f("Offset1", pfOffset1);
      vpBlur->mfParameter4f("Offset2", pfOffset2);
      vpBlur->mfParameter4f("Offset3", pfOffset3);
      // setup screen aligned quad
      struct_VERTEX_FORMAT_P3F_TEX2F pScreenBlur[] =  
      {
        Vec3(-s_off, -t_off, 0), 0, 0,   
        Vec3(-s_off, 1-t_off, 0), 0, 1,    
        Vec3(1-s_off, -t_off, 0), 1, 0,   
        Vec3(1-s_off, 1-t_off, 0), 1, 1,   
      };     
      int iBlurAmount = 1;
      for(int iBlurPasses=1; iBlurPasses<=iBlurAmount; iBlurPasses++) 
      {
        // set texture coordinates scale (needed for rectangular textures in gl ..)
        float pfScale[]={ 1.0f, 1.0f, 1.0f, (float) iBlurPasses};     
        vpBlur->mfParameter4f("vTexCoordScale", pfScale);

        // set current rendertarget
        //pRenderer->m_pd3dDevice->SetRenderTarget( 0, pTexSurf);
        // render screen aligned quad...
        gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  
      }
      vpBlur->mfSet(false, 0);
      fpBlur->mfSet(false, 0);
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
    EF_SetState(GS_NODEPTHTEST | GS_COLMASKONLYALPHA);
    Set2DMode(true, 1, 1);

    float s1=1.0f/(float) nSizeX;     
    float t1=1.0f/(float) nSizeY; 
    float s_off=s1*0.5f; 
    float t_off=t1*0.5f; 

    D3DXVECTOR4 avSampleOffsets[16];
    D3DXVECTOR4 avSampleWeights[16];

    // setup screen aligned quad
    struct_VERTEX_FORMAT_P3F_TEX2F pScreenBlur[] =  
    {
      Vec3(-s_off, -t_off, 0), 0, 0,   
      Vec3(-s_off, 1-t_off, 0), 0, 1,    
      Vec3(1-s_off, -t_off, 0), 1, 0,   
      Vec3(1-s_off, 1-t_off, 0), 1, 1,   
    };     

    RECT rectSrc;
    GetTextureRect(tpSrc, &rectSrc);
    InflateRect(&rectSrc, -1, -1);

    RECT rectDest;
    GetTextureRect(tpDst, &rectDest);
    InflateRect(&rectDest, -1, -1);

    CoordRect coords;
    GetTextureCoords(tpSrc, &rectSrc, tpDst, &rectDest, &coords);

    HRESULT hr = GetSampleOffsets_GaussBlur5x5Bilinear(tpSrc->m_Width, tpSrc->m_Height, avSampleOffsets, avSampleWeights, 1.0f);

    if(tpSrc)
    {
      tpSrc->m_RefTex.m_MinFilter=D3DTEXF_LINEAR; 
      tpSrc->m_RefTex.m_MagFilter=D3DTEXF_LINEAR;
      tpSrc->m_RefTex.bRepeats=false;  
      tpSrc->Set(); 
    }

    CCGPShader_D3D *fpGaussBlur5x5 = (CCGPShader_D3D *)PShaderForName(m_RP.m_PS_HDRGaussBlur5x5_Bilinear, "CGRC_HDR_GaussBlur5x5_Bilinear_PS20");
    if (fpGaussBlur5x5)
    {
      fpGaussBlur5x5->mfSet(true);

      SCGBind *bind = fpGaussBlur5x5->mfGetParameterBind("vSampleOffsets");
      assert(bind);
      fpGaussBlur5x5->mfParameter(bind, &avSampleOffsets[0].x, 9);

      bind = fpGaussBlur5x5->mfGetParameterBind("vSampleWeights");
      assert(bind);
      fpGaussBlur5x5->mfParameter(bind, &avSampleWeights[0].x, 9);

      // Draw a fullscreen quad to sample the RT
      DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  

      fpGaussBlur5x5->mfSet(false);
    }
    Set2DMode(false, 1, 1);
  }
  else
  if (nType >= 3)
  {
    if (!st->nTexId1)
      st->nTexId1 = GenShadowTexture(st->nTexSize, false);
    STexPicD3D *tpDst2 = (STexPicD3D *)m_TexMan->GetByID(st->nTexId1);
    LPDIRECT3DTEXTURE9 pIRGBTarget2 = NULL;
    IDirect3DSurface9 *pIRGBTargetSurf2 = NULL;
    pIRGBTarget2 = (LPDIRECT3DTEXTURE9)tpDst2->m_RefTex.m_VidTex;
    hReturn = pIRGBTarget2->GetSurfaceLevel(0, &pIRGBTargetSurf2);
    hReturn = m_pd3dDevice->SetRenderTarget(0, pIRGBTargetSurf2);
    SAFE_RELEASE(pIRGBTargetSurf2);
    Set2DMode(true, 1, 1);
    EF_SetState(GS_NODEPTHTEST | GS_COLMASKONLYALPHA);

    // setup screen aligned quad
    struct_VERTEX_FORMAT_P3F_TEX2F pScreenBlur[] =  
    {
      Vec3(0, 0, 0), 0, 0,   
      Vec3(0, 1, 0), 0, 1,    
      Vec3(1, 0, 0), 1, 0,   
      Vec3(1, 1, 0), 1, 1,   
    };     

    if(tpSrc)
    {
      tpSrc->m_RefTex.m_MinFilter=D3DTEXF_LINEAR; 
      tpSrc->m_RefTex.m_MagFilter=D3DTEXF_LINEAR;
      tpSrc->m_RefTex.bRepeats=false;  
      tpSrc->Set(); 
    }

    CCGPShader_D3D *fpGaussSep = (CCGPShader_D3D *)PShaderForName(m_RP.m_PS_GaussBlurSep, "CGRCBlurSep");
    CCGVProgram_D3D *vpGaussSep = (CCGVProgram_D3D *)VShaderForName(m_RP.m_VS_GaussBlurSep, "CGVProgBlurSep");
    if (fpGaussSep && vpGaussSep)
    {
      static float sW[8] = {0.2537f, 0.2185f, 0.0821f, 0.0461f, 0.0262f, 0.0162f, 0.0102f, 0.0052f};
      int i;

      fpGaussSep->mfSet(true);
      vpGaussSep->mfSet(true);

      vec4_t v;
      v[0] = 1.0f/(float)nSizeX;
      v[1] = 1.0f/(float)nSizeY;
      v[2] = 0;
      v[3] = 0;
      SCGBind *bindOffs = vpGaussSep->mfGetParameterBind("PixelOffset");
      assert(bindOffs);
      vpGaussSep->mfParameter(bindOffs, v, 1);

      // X Blur
      v[0] = 1.0f/(float)nSizeX;
      v[1] = 0;
      bindOffs = fpGaussSep->mfGetParameterBind("BlurOffset");
      assert(bindOffs);
      fpGaussSep->mfParameter(bindOffs, v, 1);

      vec4_t vWeight[8];
      for (i=0; i<8; i++)
      {
        vWeight[i][0] = sW[i];
        vWeight[i][1] = sW[i];
        vWeight[i][2] = sW[i];
        vWeight[i][3] = sW[i];
      }
      SCGBind *bindW = fpGaussSep->mfGetParameterBind("vPixelWeights");
      assert(bindW);
      fpGaussSep->mfParameter(bindW, &vWeight[0][0], 8);
  
      // Draw a fullscreen quad to sample the RT
      DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  

      // Y Blur
      v[0] = 0;
      v[1] = 1.0f/(float)nSizeY;
      fpGaussSep->mfParameter(bindOffs, v, 1);

      hReturn = m_pd3dDevice->SetRenderTarget(0, pIRGBTargetSurf);
      SAFE_RELEASE(pIRGBTargetSurf);
      tpDst2->Set();

      // Draw a fullscreen quad to sample the RT
      DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  

      fpGaussSep->mfSet(false);
      vpGaussSep->mfSet(false);
    }
    Set2DMode(false, 1, 1);
  }
  SAFE_RELEASE(pIRGBTargetSurf);
}

unsigned int CD3D9Renderer::GenShadowTexture(int nSize, bool bProjected)
{
  byte *data = new byte[nSize*nSize*4];
  char name[128];
  sprintf(name, "$AutoShadowMaps_%d", m_TexGenID++);
  int flags = FT_NOMIPS | FT_CLAMP | FT_NOSTREAM | FT_HASALPHA;
  if (bProjected)
    flags |= FT_PROJECTED;
  int flags2 = FT2_RENDERTARGET | FT2_NODXT;
  ETEX_Format eTF;
  if ((m_Features & RFT_DEPTHMAPS) && bProjected)
    eTF = eTF_DEPTH;
  else
    eTF = eTF_8888;
  STexPic *tp = m_TexMan->CreateTexture(name, nSize, nSize, 1, flags, flags2, data, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF);
  STexPicD3D *t = (STexPicD3D *)tp;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  delete [] data;
  if (!t)
    return 0;
  pID3DTexture = (LPDIRECT3DTEXTURE9)t->m_RefTex.m_VidTex;
  if (!pID3DTexture)
    return 0;
  return t->m_Bind;
}

// draw grid and project depth map on it
/*void CD3D9Renderer::DrawShadowGrid(const Vec3d & pos, const Vec3d & Scale, ShadowMapFrustum*lf, bool translate_projection, float alpha, IndexedVertexBuffer* pVertexBuffer, float anim_angle)
{
}	*/

// Make 8-bit identity texture that maps (s)=(z) to [0,255]/255.
int CD3D9Renderer::MakeShadowIdentityTexture()
{ 
  return 0;
}

void CD3D9Renderer::OnEntityDeleted(IEntityRender * pEntityRender)
{ // remove references to the entity
	for(int i=0; i<MAX_DYNAMIC_SHADOW_MAPS_COUNT; i++)
	{
		ShadowMapTexInfo * pInf = &m_ShadowTexIDBuffer[i];
		if(pInf->pOwner == pEntityRender)
			pInf->pOwner = NULL;
	}
}

void DrawText(ISystem * pSystem, int x, int y, const char * format, ...)
{
  char buffer[512];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  ICryFont *pCryFont = pSystem->GetICryFont();
  if (!pCryFont)
    return;

  IFFont *pFont = pCryFont->GetFont("console");
  if (!pFont)
    return;

  pFont->UseRealPixels(false);
  pFont->SetSameSize(true);
  pFont->SetCharWidthScale(1);
  pFont->SetSize(vector2f(12, 12));
  pFont->SetColor(color4f(1,1,1,1));
  pFont->SetCharWidthScale(.5f);
  pFont->DrawString( (float)x/*-pFont->GetCharWidth() * strlen(buffer) * pFont->GetCharWidthScale(*)*/, (float)y, buffer );
}

void CD3D9Renderer::DrawAllShadowsOnTheScreen()

{
  float width=800;
  float height=600;
  Set2DMode(true, (int)width, (int)height);

  float fArrDim = max(4, sqrt(float(MAX_DYNAMIC_SHADOW_MAPS_COUNT)));
  float fPicDimX = width/fArrDim;
  float fPicDimY = height/fArrDim;
  int nShadowId=0;
  for(float x=0; nShadowId<MAX_DYNAMIC_SHADOW_MAPS_COUNT && x<width-10;  x+=fPicDimX)
    for(float y=0; nShadowId<MAX_DYNAMIC_SHADOW_MAPS_COUNT && y<height-10; y+=fPicDimY)
    {
      ShadowMapTexInfo * pInf = &m_ShadowTexIDBuffer[nShadowId++];
      if(pInf->nTexId0 && (pInf->pOwner || pInf->pOwnerGroup))
      {
        STexPic *tp = (STexPic *)EF_GetTextureByID(pInf->nTexId0);
        if (tp)
        {
          byte bSaveProj = tp->m_RefTex.bProjected;
          tp->m_RefTex.bProjected = false;
          SetState( GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST );
          Draw2dImage(x, y, fPicDimX, fPicDimY, pInf->nTexId0, 0,0,1,1,180);
          const char * pName = pInf->pOwner ? pInf->pOwner->GetName() : pInf->pOwnerGroup->GetFileName();
          int nLen = strlen(pName);
          DrawText(iSystem, (int)(x/width*800.f), (int)(y/height*600.f), "%8s-%d", pName + max(0, nLen - 12), pInf->nLastFrameID&7);
          tp->m_RefTex.bProjected = bSaveProj;
        }
      }
    }

    Set2DMode(false, m_width, m_height);
}

void CD3D9Renderer::PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid)
{
	PROFILE_FRAME(Prep_PrepareDepthMap);

  if(!lof || !lof->pLs)
		return;

  static int nCurTexIdSlot = 0;

  //lof->bUpdateRequested = true;


  if (lof->nResetID != m_nFrameReset)
  {
    lof->nResetID = m_nFrameReset;
    lof->bUpdateRequested = true;
  }
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Render objects into frame and Z buffers
////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // remember fog value
  EF_PushFog(); 
  EnableFog(false);
  EF_PushMatrix();
  m_matProj->Push();

  // normalize size
  //while(nShadowTexSize>m_height || nShadowTexSize>m_width)
  //  nShadowTexSize/=2;

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "   Really generating %dx%d shadow map for %s, [%s] \n", 
		nShadowTexSize, nShadowTexSize, 
		lof->pOwner ? lof->pOwner->GetName() : "NoOwner", 
		lof->pOwnerGroup ? lof->pOwnerGroup->GetFileName() : "NoGroup");

  // setup matrices
  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  gRenDev->SetViewport(0, 0, nShadowTexSize, nShadowTexSize);  

  Matrix44 camMatr = m_CameraMatrix;

  D3DXMATRIX *m = m_matProj->GetTop();
  if (m_Features & RFT_DEPTHMAPS)
    D3DXMatrixPerspectiveFovRH(m, lof->FOV*(gf_PI/180.0f), lof->ProjRatio, lof->min_dist, lof->max_dist+50);
  else
    D3DXMatrixPerspectiveFovRH(m, lof->FOV*(gf_PI/180.0f), lof->ProjRatio, lof->min_dist, lof->max_dist);
  //makeProjectionMatrix(lof->FOV, 1, lof->min_dist, lof->max_dist, (float *)m);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  m_bInvertedMatrix = false;
  memcpy(lof->debugLightFrustumMatrix,m,sizeof(lof->debugLightFrustumMatrix));

  m = m_matView->GetTop();  
  D3DXVECTOR3 Eye = D3DXVECTOR3(lof->pLs->vSrcPos.x, lof->pLs->vSrcPos.y, lof->pLs->vSrcPos.z);
  D3DXVECTOR3 At = D3DXVECTOR3(lof->target.x, lof->target.y, lof->target.z);
  D3DXVECTOR3 Up = D3DXVECTOR3(0,0,1);
  D3DXMatrixLookAtRH(m, &Eye, &At, &Up);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
  memcpy(lof->debugLightViewMatrix,   m,   sizeof(lof->debugLightViewMatrix));
  //EF_SetCameraInfo();

  if (!m_SceneRecurseCount)
    m_pd3dDevice->BeginScene();
  m_SceneRecurseCount++;


  if(make_new_tid)
  { // new id for static objects
    assert (!lof->depth_tex_id);
    lof->depth_tex_id = GenShadowTexture(nShadowTexSize, true);
    assert(lof->depth_tex_id<14000);
  }
  else
  { 
    // try to reuse slot if it was not modified
    if( lof->nTexIdSlot>=0 &&
      m_ShadowTexIDBuffer[lof->nTexIdSlot].nTexId0 == lof->depth_tex_id &&
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
    }

    if(nCurTexIdSlot>=MAX_DYNAMIC_SHADOW_MAPS_COUNT)
      nCurTexIdSlot=0;

    if(!m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId0)
    {
      //assert(false);
      m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId0 = GenShadowTexture(nShadowTexSize, true);
      assert(m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId0<14000);
			//assert(m_TexMan->GetByID(m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId));
      make_new_tid = true;
    }

    lof->nTexIdSlot = nCurTexIdSlot;
    STexPic *tpOld = m_TexMan->GetByID(m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId0);
		assert(tpOld);
    if (!tpOld || tpOld->m_Width != nShadowTexSize)
    {
      //assert (false);
      //ResetToDefault();
			if(tpOld)
				tpOld->Release(0);
      m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId0 = lof->depth_tex_id = GenShadowTexture(nShadowTexSize, true);
    }
    else
      lof->depth_tex_id = m_ShadowTexIDBuffer[nCurTexIdSlot].nTexId0;    


		m_ShadowTexIDBuffer[nCurTexIdSlot].pOwner = lof->pOwner;
		m_ShadowTexIDBuffer[nCurTexIdSlot].pOwnerGroup = lof->pOwnerGroup;
		m_ShadowTexIDBuffer[nCurTexIdSlot].dwFlags = lof->dwFlags;
    m_ShadowTexIDBuffer[nCurTexIdSlot].nLastFrameID = GetFrameID();
    m_ShadowTexIDBuffer[nCurTexIdSlot].nTexSize = nShadowTexSize;
  }  

	assert(m_ShadowTexIDBuffer[0].nTexId0 ? m_TexMan->GetByID(m_ShadowTexIDBuffer[0].nTexId0)!=NULL : 1);
//	assert(m_ShadowTexIDBuffer[1].nTexId ? m_TexMan->GetByID(m_ShadowTexIDBuffer[1].nTexId)!=NULL : 1);

  ShadowMapTexInfo *st = NULL;
  HRESULT hReturn;
  IDirect3DSurface9 *pIRGBTargetSurf = NULL;
  IDirect3DSurface9 *pIZBufferSurf = NULL;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  LPDIRECT3DTEXTURE9 pIRGBTarget = NULL;
  // Create color buffer render target. We aren't actually going to use it for anything,
  // but D3D required one as rendertarget
  bool bStatus = false;
  if (m_Features & RFT_DEPTHMAPS)
  {
    bStatus = true;
    // Set render target
    STexPicD3D *tp = (STexPicD3D *)m_TexMan->GetByID(lof->depth_tex_id);
    assert(tp->m_ETF == eTF_DEPTH);
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
      smt.nTexId0 = 0;
      smt.nTexSize = nShadowTexSize;
      m_TempShadowTextures.AddElem(smt);
    }
    st = &m_TempShadowTextures[i];
    if (!st->nTexId0)
      st->nTexId0 = GenShadowTexture(nShadowTexSize, false);
    STexPicD3D *tpRGB = (STexPicD3D *)m_TexMan->GetByID(st->nTexId0);
    if (tp)
    {
      pID3DTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
      if (pID3DTexture)
        hReturn = pID3DTexture->GetSurfaceLevel(0, &pIZBufferSurf);
    }
    //hReturn = m_pd3dDevice->CreateTexture(nShadowTexSize, nShadowTexSize, 1, D3DUSAGE_RENDERTARGET, m_d3dsdBackBuffer.Format, D3DPOOL_DEFAULT, &pIRGBTarget, NULL);
    if (tpRGB)
    {
      pIRGBTarget = (LPDIRECT3DTEXTURE9)tpRGB->m_RefTex.m_VidTex;
      if (pIRGBTarget)
        hReturn = pIRGBTarget->GetSurfaceLevel(0, &pIRGBTargetSurf);
    }
    hReturn = m_pd3dDevice->SetRenderTarget(0, pIRGBTargetSurf);
    hReturn = m_pd3dDevice->SetDepthStencilSurface(pIZBufferSurf);

    // Disable color writes
    m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
  }
  else
  {
    bStatus = true;
    // Set render target
    STexPicD3D *tp = (STexPicD3D *)m_TexMan->GetByID(lof->depth_tex_id);
    //assert(tp->m_ETF == eTF_DEPTH);
    if (tp)
      pID3DTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
    if (CV_r_shadowblur)
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
        smt.nTexId0 = 0;
        smt.nTexSize = nShadowTexSize;
        m_TempShadowTextures.AddElem(smt);
      }
      st = &m_TempShadowTextures[i];
      if (!st->nTexId0)
        st->nTexId0 = GenShadowTexture(nShadowTexSize, false);
      tp = (STexPicD3D *)m_TexMan->GetByID(st->nTexId0);
      if (tp)
        pID3DTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
    }
    if (pID3DTexture)
    {
      hReturn = pID3DTexture->GetSurfaceLevel(0, &pIRGBTargetSurf);
      hReturn = m_pd3dDevice->SetRenderTarget(0, pIRGBTargetSurf);
      hReturn = m_pd3dDevice->SetDepthStencilSurface(m_pTempZBuffer);
    }
  }

  int ClipPlanes = m_RP.m_ClipPlaneEnabled;
  SPlane clP = m_RP.m_CurClipPlane;
  int nPersFlags = m_RP.m_PersFlags;
  m_RP.m_PersFlags &= ~RBPF_HDR;

  if (bStatus)
  {
    // clear frame buffer
    CFColor col;
    if (!(m_Features & RFT_DEPTHMAPS) && (m_Features & RFT_SHADOWMAP_SELFSHADOW))
    {
      col = CFColor(1,0.879f,0,0);
      EF_ClearBuffers(true, false, &col[0]);
    }
    else
    {
      col = CFColor(0,0,0,0);
      EF_ClearBuffers(true, false, &col[0]);
    }

    RECT scRect;
    scRect.left = 3;
    scRect.right = nShadowTexSize-3;
    scRect.top = 3;
    scRect.bottom = nShadowTexSize-3;
    m_pd3dDevice->SetScissorRect(&scRect);
    m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

    IShader * pStateShader = EF_LoadShader("StateNoCull", eSH_World, EF_SYSTEM);

    // draw static objects (not entities)
    if(lof->pModelsList && lof->pModelsList->Count())
    { 
      if (!(m_Features & RFT_DEPTHMAPS | RFT_SHADOWMAP_SELFSHADOW))
      {
        // cut underground geometry
        float plane[] = {0,0,1,0};
        gRenDev->SetClipPlane(0,plane);
      }

      EF_StartEf(); 
      for(int m=0; m<lof->pModelsList->Count(); m++)
      {
        SRendParams rParms;
        rParms.pStateShader = pStateShader;
        if (m_Features & RFT_DEPTHMAPS)
          rParms.nShaderTemplate = EFT_WHITE;
        else
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
    }

    // draw entities
    EF_StartEf(); 
    for(int m=0; lof->pEntityList && m_RP.m_pCurObject && m<lof->pEntityList->Count(); m++)
    { 
      IEntityRender * pEnt  = (*lof->pEntityList)[m];
      const char *name = pEnt->GetName();
      Vec3d vOffSetDir = -GetNormalized( lof->pLs->vSrcPos )*(0.1f+0.035f*(256.f/nShadowTexSize));
      IEntityRender * pEnt0 = (*lof->pEntityList)[0];
		  SRendParams rParams;
      if (m_Features & RFT_DEPTHMAPS)
        rParams.nShaderTemplate = EFT_WHITE;
      else
        rParams.nShaderTemplate = EFT_WHITESHADOW;
      rParams.pStateShader = pStateShader;
		  rParams.vPos = vOffSetDir*lof->fOffsetScale + (pEnt->GetPos() - lof->pOwner->GetPos());
      rParams.dwFObjFlags |= FOB_RENDER_INTO_SHADOWMAP;
      rParams.dwFObjFlags |= FOB_TRANS_MASK;
		  pEnt->DrawEntity(rParams);
    }
	  EF_EndEf3D(true);
    //memcpy(lof->debugLightViewMatrix, &CCGVProgram_D3D::m_CurParams[0][0], 4*4*sizeof(float));

  if (m_Features & RFT_DEPTHMAPS)
  {
    // Enable color writes
    m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
  }
  // Blur 2D texture
  if (st && !(m_Features & RFT_DEPTHMAPS))
  {
    SAFE_RELEASE(pIRGBTargetSurf);

    int BlurType = CV_r_shadowblur-1;
    if (CV_r_shadowblur > 2)
    {
      if (CRenderer::CV_r_nops20)
        BlurType = 1;
    }
    BlurImage(nShadowTexSize, nShadowTexSize, BlurType, st, lof->depth_tex_id);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //  Now make texture from frame buffer
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////


    if (CD3D9Renderer::CV_d3d9_savedepthmaps)
    {
      if (m_Features & RFT_DEPTHMAPS)
      {
        m_matProj->Push();
        D3DXMatrixOrthoRH( m_matProj->GetTop(), (float)nShadowTexSize, (float)nShadowTexSize, -20.0f, 0.0f);
        m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m_matProj->GetTop()); 
        EF_PushMatrix();
        m_matView->LoadIdentity();
        m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 

        m_TexMan->m_Text_White->Set();
        EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
        SetCullMode(R_CULL_NONE);
        EF_SetState(GS_NODEPTHTEST);
        DrawQuad(0, 0, (float)nShadowTexSize, (float)nShadowTexSize, CFColor(0.0f));
        EF_SetState(0);
        DrawQuad(0, 0, (float)nShadowTexSize, (float)nShadowTexSize, CFColor(1.0f));
        LPDIRECT3DTEXTURE9 pID3DSysTexture;
        LPDIRECT3DSURFACE9 pSysSurf;
        D3DLOCKED_RECT d3dlrSys;
        hReturn = D3DXCreateTexture(m_pd3dDevice, nShadowTexSize, nShadowTexSize, 1, 0, m_d3dsdBackBuffer.Format, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
        hReturn = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);
        hReturn = m_pd3dDevice->GetRenderTargetData(pIRGBTargetSurf, pSysSurf);
        hReturn = pID3DSysTexture->LockRect(0, &d3dlrSys, NULL, 0);
        byte *pic = new byte [nShadowTexSize * nShadowTexSize * 4];
        // Copy data to the texture 
        cryMemcpy(pic, d3dlrSys.pBits, nShadowTexSize*nShadowTexSize*4);
        char buff[128];
        sprintf(buff, "ShadowMap%02d.tga", nCurTexIdSlot);
        ::WriteTGA(pic,nShadowTexSize,nShadowTexSize,buff,32); 
        delete [] pic;
        hReturn = pID3DSysTexture->UnlockRect(0);
        SAFE_RELEASE(pSysSurf);
        SAFE_RELEASE(pID3DSysTexture);
        EF_PopMatrix();
        m_matProj->Pop();
      }
      else
      {
        LPDIRECT3DTEXTURE9 pID3DSysTexture;
        LPDIRECT3DSURFACE9 pSysSurf;
        D3DLOCKED_RECT d3dlrSys;
        //hReturn = D3DXCreateTexture(m_pd3dDevice, nShadowTexSize, nShadowTexSize, 1, 0, D3DFMT_R5G6B5, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
        hReturn = D3DXCreateTexture(m_pd3dDevice, nShadowTexSize, nShadowTexSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
        hReturn = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);
        hReturn = m_pd3dDevice->GetRenderTargetData(pIRGBTargetSurf, pSysSurf);
        hReturn = pID3DSysTexture->LockRect(0, &d3dlrSys, NULL, 0);
        byte *pic = new byte [nShadowTexSize*nShadowTexSize*4];
        byte *ds = (byte *)d3dlrSys.pBits;
        memcpy(pic, ds, nShadowTexSize*nShadowTexSize*4);
        /*for (int i=0; i<nShadowTexSize; i++)
        {
          int ni0 = (i*nShadowTexSize)*4;
          int ni1 = i*d3dlrSys.Pitch;
          ushort *ds1  = (ushort *)&ds[ni1];
					uint *pic1 = (uint *)&pic[ni0];
					for (int j=0; j<nShadowTexSize; j++)
					{
  					uint argb = *ds1++;
						*pic1++ = (((argb << 8)*4) & 0xff0000) |
											(((argb << 5)*5) & 0xff00) |
											(((argb << 3)*4) &  0x00FF);
					}
        }*/
        hReturn = pID3DSysTexture->UnlockRect(0);
        char buff[128];
        sprintf(buff, "ShadowMap%02d.tga", nCurTexIdSlot);
        ::WriteTGA(pic,nShadowTexSize,nShadowTexSize,buff,32); 
        delete [] pic;
        SAFE_RELEASE(pSysSurf);
        SAFE_RELEASE(pID3DSysTexture);
      }
    }
  }
  if (ClipPlanes)
    EF_SetClipPlane(true, &clP.m_Normal.x, false);

  m_RP.m_PersFlags = nPersFlags;

  if (m_Features & RFT_DEPTHMAPS)
  {
    hReturn = m_pd3dDevice->SetDepthStencilSurface(m_pCurZBuffer);
    if (m_RP.m_PersFlags & RBPF_HDR)
      hReturn = m_pd3dDevice->SetRenderTarget(0, m_pHDRTargetSurf);
    else
      hReturn = m_pd3dDevice->SetRenderTarget(0, m_pCurBackBuffer);

    SAFE_RELEASE(pIRGBTargetSurf);
    //SAFE_RELEASE(pIRGBTarget);
    SAFE_RELEASE(pIZBufferSurf);
  }
  else
  {
    if (m_RP.m_PersFlags & RBPF_HDR)
      hReturn = m_pd3dDevice->SetRenderTarget(0, m_pHDRTargetSurf);
    else
      hReturn = m_pd3dDevice->SetRenderTarget(0, m_pCurBackBuffer);

    hReturn = m_pd3dDevice->SetDepthStencilSurface(m_pCurZBuffer);

    SAFE_RELEASE(pIRGBTargetSurf);
  }

  EF_PopMatrix();
  m_matProj->Pop();
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m_matProj->GetTop());
  m_CameraMatrix = camMatr;
  m_matView->LoadMatrix((D3DXMATRIX *)camMatr.GetData());
  EF_SetCameraInfo();
  m_bInvertedMatrix = false;

  SetViewport(vX, vY, vWidth, vHeight);
  m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
  m_sPrevX = 1;
  m_sPrevY = 1;
  m_sPrevWdt = nShadowTexSize-1;
  m_sPrevHgt = nShadowTexSize-1;

  EF_PopFog();

  m_SceneRecurseCount--;
  if (!m_SceneRecurseCount)
    m_pd3dDevice->EndScene();

	assert(m_ShadowTexIDBuffer[0].nTexId0 ? m_TexMan->GetByID(m_ShadowTexIDBuffer[0].nTexId0)!=NULL : 1);

  if(lof->pPenumbra && lof->pPenumbra->bUpdateRequested)
    PrepareDepthMap(lof->pPenumbra, make_new_tid);
}

void MakePenumbraTextureFromDepthMap(byte * pDepthMapIn, int nSize, byte * pPenumbraMapOut)
{
  cryMemcpy(pPenumbraMapOut,pDepthMapIn,nSize*nSize);

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

  cryMemcpy(pDepthMapIn,pPenumbraMapOut,nSize*nSize);

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

void CD3D9Renderer::SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3d * vShadowTrans, const float fShadowScale, Vec3d vObjTrans, float fObjScale, const Vec3d vObjAngles, Matrix44 * pObjMat)
{  
  if(!pFrustum)
    return;

  D3DXMATRIX lightFrustumMatrix;
  D3DXMATRIX lightViewMatrix;

  if(vShadowTrans)
  { // make tmp matrix for this obj position if shadow frustum is not translated (translate original mats)
    //float fDist = (pFrustum->min_dist + pFrustum->max_dist)*0.5f;
    //float fDiam = (pFrustum->max_dist - pFrustum->min_dist);//*fShadowScale;
    //D3DXMatrixPerspectiveFovRH(&lightFrustumMatrix, pFrustum->FOV*fShadowScale*(gf_PI/180.0f), 1, pFrustum->min_dist, pFrustum->max_dist+10);
    if (m_Features & RFT_DEPTHMAPS)
      makeProjectionMatrix(pFrustum->FOV*fShadowScale, pFrustum->ProjRatio, pFrustum->min_dist, pFrustum->max_dist+50, lightFrustumMatrix);
    else
      D3DXMatrixPerspectiveFovRH(&lightFrustumMatrix, pFrustum->FOV*fShadowScale*(gf_PI/180.0f), pFrustum->ProjRatio, pFrustum->min_dist, pFrustum->max_dist);

    Vec3d mv_trans = *vShadowTrans;// - vObjTrans;
    
    D3DXMATRIX mat;

    D3DXVECTOR3 Eye = D3DXVECTOR3(
      pFrustum->pLs->vSrcPos.x+mv_trans.x, 
      pFrustum->pLs->vSrcPos.y+mv_trans.y, 
      pFrustum->pLs->vSrcPos.z+mv_trans.z);
    D3DXVECTOR3 At = D3DXVECTOR3(
      fShadowScale*pFrustum->target.x+mv_trans.x, 
      fShadowScale*pFrustum->target.y+mv_trans.y, 
      fShadowScale*pFrustum->target.z+mv_trans.z);
    D3DXVECTOR3 Up = D3DXVECTOR3(0,0,1);

    D3DXMatrixLookAtRH(&mat, &Eye, &At, &Up);

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

  CD3D9TexMan::BindNULL(1);

  ConfigShadowTexgen(Num, 0, pFrustum, lightFrustumMatrix, lightViewMatrix, pFrustum->debugLightViewMatrix);
}

// setup projection texgen
void CD3D9Renderer::ConfigShadowTexgen(int Num, int rangeMap, ShadowMapFrustum * pFrustum, float * pLightFrustumMatrix, float * pLightViewMatrix, float *ModelVPMatrix)
{
  float m1[16], m2[16];
  float *mtexSc;

  if (rangeMap) 
  {
    int to_map_8bit = MakeShadowIdentityTexture();

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
      //glBindTexture(GL_TEXTURE_1D, to_map_8bit);
      //glEnable(GL_TEXTURE_1D);
      //glDisable(GL_TEXTURE_2D);
    }

    memcpy(m1,RSmatrix,sizeof(m1));
  } 
  else 
  {
    if ((m_Features & RFT_DEPTHMAPS))
    {
      // Compensate for D3D's texel adressing, we need 1:1 texel - pixel matching
      float fOffsetX = 0.5f + (0.5f / (float) pFrustum->nTexSize);
      float fOffsetY = 0.5f + (0.5f / (float) pFrustum->nTexSize);

      float Smatrix[16] = 
      {
        0.5f,     0,        0,             0,
        0,       -0.5f,     0,             0,
        0,        0,        0.5f,          0,
        fOffsetX, fOffsetY, 0.5f,          1.0f
      };

      mtexSc = Smatrix;
    }
    else
    {
      static float Smatrix[16] = 
      {
        0.5f, 0,     0,    0,
        0,    -0.5f, 0,    0,
        0,    0,     0,    0,
        0.5f, 0.5f,  1.0f, 1.0f
      };

      mtexSc = Smatrix;
    }
  }
  mathMatrixMultiply(m2, pLightFrustumMatrix, pLightViewMatrix, g_CpuFlags);
  mathMatrixMultiply(m1, mtexSc, m2, g_CpuFlags);

  {
    Matrix44 *mt = &gRenDev->m_cEF.m_TempMatrices[Num][0];
    float *pf = mt->GetData();
    //memcpy(pf, m1, 4*4*4);
    mathMatrixTranspose(pf, m1, g_CpuFlags);
  }

  if ((m_Features & RFT_SHADOWMAP_SELFSHADOW) && !(m_Features & RFT_DEPTHMAPS))
  {
    Matrix44 *mt = &gRenDev->m_cEF.m_TempMatrices[Num][7];
    float *pf = mt->GetData();
    mathMatrixTranspose(pf, m2, g_CpuFlags);
  }

  if (Num >= 0)
  {
    if(pFrustum->depth_tex_id<=0)
      Warning( 0,0,"Warning: CD3D9Renderer::ConfigShadowTexgen: pFrustum->depth_tex_id not set");
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
    if (m_RP.m_pCurLight)
    {
      float fRadius;
      if (m_RP.m_ObjFlags & FOB_TRANS_MASK)
      {
        float fLen = m_RP.m_pCurObject->m_Matrix(0,0)*m_RP.m_pCurObject->m_Matrix(0,0) + m_RP.m_pCurObject->m_Matrix(0,1)*m_RP.m_pCurObject->m_Matrix(0,1) + m_RP.m_pCurObject->m_Matrix(0,2)*m_RP.m_pCurObject->m_Matrix(0,2);
        unsigned int *n1 = (unsigned int *)&fLen;
        unsigned int n = 0x5f3759df - (*n1 >> 1);
        float *n2 = (float *)&n;
        float fISqrt = (1.5f - (fLen * 0.5f) * *n2 * *n2) * *n2;
        fRadius = m_RP.m_pCurLight->m_fRadius * fISqrt;
      }
      else
        fRadius = m_RP.m_pCurLight->m_fRadius;

      m_RP.m_REColor[3] = 1.0f / fRadius;
      if (m_RP.m_pRE)
        m_RP.m_pRE->m_Color[3] = m_RP.m_REColor[3];
    }
    else
    {
      m_RP.m_REColor[3] = 1.0f / 10000.0f;
      if (m_RP.m_pRE)
        m_RP.m_pRE->m_Color[3] = m_RP.m_REColor[3];
    }
    STexPic *tp = (STexPic *)EF_GetTextureByID(pFrustum->depth_tex_id);
    tp->m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
    tp->m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
  }
}


