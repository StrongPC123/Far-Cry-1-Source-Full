/*
=======================================================================
FILE : D3DScreenRender.cpp
DESC : screen renderer
PROJ : Crytek Engine
CODER: Tiago Sousa 

Last Update: 14/06/2004

Todo: 
- Clean up code
- Remove redundant data/tests
=======================================================================
*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "D3DCGPShader.h"
#include "D3DCGVProgram.h"

// helper macros
#define REQUIRES_PS20 (gRenDev->GetFeatures()&RFT_HW_PS20)

// Log helper
#define LOG_EFFECT(msg)\
  if(gRenDev->m_LogFile)\
  {\
   gRenDev->Logv(SRendItem::m_RecurseLevel, msg);\
  }

// Compute scaled texture coordinates for texel-to-pixel correct output
#define TEXEL_TO_SCREEN(size) \
  ((size)==0)? 0.0f: 0.5f/((float)(size))

// Returns closest power of 2 texture size
int GetClosestPow2Size(int size)
{
  float fPower= floorf(logf((float)size)/logf(2.0f));
  int iResize=int(powf(2.0f, fPower));

  // clamp to 512
  if(iResize>=512)
  {
    iResize=512;
  }

  return iResize;
}

// SetTexture - sets texture stage
void SetTexture(STexPic *pTex, int iStage, int iMinFilter, int iMagFilter, bool bClamp)
{
  gcpRendD3D->EF_SelectTMU(iStage);
  if(pTex)
  {
    pTex->m_RefTex.m_MinFilter=iMinFilter; 
    pTex->m_RefTex.m_MagFilter=iMagFilter;
    pTex->m_RefTex.bRepeats=(!bClamp);  
    pTex->Set(); 
   }
  else
  {
    gRenDev->SetTexture(0);
  }
}

// Returns pointer to texture surface mipmap level
LPDIRECT3DSURFACE9 GetTextureSurface(const STexPic *pTex, int iLevel=0)
{  
  LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)pTex->m_RefTex.m_VidTex;
  assert(pTexture);
  
  LPDIRECT3DSURFACE9 pSurf = 0;
  HRESULT hr = pTexture->GetSurfaceLevel(iLevel, &pSurf);

  return pSurf;
}

// CreateRenderTarget - texture targets creation helper
bool CreateRenderTarget(STexPic *pTex, int iWidth, int iHeight, bool bUseAlpha, bool bMipMaps)
{
  // check if parameters are valid
  if(!pTex || !iWidth || !iHeight)
  {
    return 0;
  }

  // recreate texture when necessary
  if(pTex->m_Width!=iWidth || pTex->m_Height!=iHeight)
  {
    pTex->m_Flags &= ~FT_ALLOCATED;
  }

  unsigned int nHasAlpha=(bUseAlpha)? FT_HASALPHA: 0;
  unsigned int nHasMips=(bMipMaps)? 0: FT_NOMIPS;

  // if not created yet, create texture
  if (!(pTex->m_Flags & FT_ALLOCATED))
  {
    // set rendertarget flags
    pTex->m_Flags |= FT_ALLOCATED | nHasMips| nHasAlpha;
    pTex->m_Flags &= ~FT_DXT;
    pTex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;

    pTex->m_Width = iWidth;
    pTex->m_Height = iHeight;
    pTex->m_nMips = 0;

    // must pass empty buffer into create texture..
    byte *pData = new byte [pTex->m_Width*pTex->m_Height*((nHasAlpha)? 4: 3)];

    if(!gcpRendD3D->m_TexMan->CreateTexture(NULL, pTex->m_Width, pTex->m_Height, 1, pTex->m_Flags, pTex->m_Flags2, pData, eTT_Base, -1.0f, -1.0f, 0, pTex))
    {
      // error creating texure
      delete [] pData;
      return 0;
    }

    // Clear render target surface before using it
    ClearRenderTarget(gcpRendD3D->m_pd3dDevice, pTex, 0,0,0,1);

    delete [] pData;
  }

  return pTex->m_RefTex.m_VidTex ? 1 : 0;
}

bool CD3D9TexMan::PreloadScreenFxMaps(void)  
{
  int iTempX, iTempY, iWidth, iHeight;
  gcpRendD3D->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

  // create necessary textures at once
  bool bOk = true;
  bOk = CreateRenderTarget(m_Text_ScreenMap, iWidth, iHeight, 1, 0);
  if(bOk)
    bOk = CreateRenderTarget(m_Text_PrevScreenMap, iWidth, iHeight, 1, 0); 
  if(bOk)
    bOk = CreateRenderTarget(m_Text_ScreenLowMap, GetClosestPow2Size(iWidth/4), GetClosestPow2Size(iWidth/4), 1, 0);     
  if(bOk)
    bOk = CreateRenderTarget(m_Text_Glare, GetClosestPow2Size(iWidth/8), GetClosestPow2Size(iWidth/8), 1, 0);
  if(bOk)
    bOk = CreateRenderTarget(m_Text_FlashBangMap, GetClosestPow2Size(iWidth/8), GetClosestPow2Size(iWidth/8), 1, 0);
  if(bOk)
    bOk = CreateRenderTarget(m_Text_ScreenAvg1x1, 2, 2, 1, 0); 
  if(bOk)
    bOk = CreateRenderTarget(m_Text_ScreenCurrLuminosityMap, 1, 1, 1, 0);  

  //Failed to create render targets
  if (!bOk)
  {
    iLog->Log("Error: Failed to create render targets for screen post-processing (screen effects is disabled)\n");
    gRenDev->m_bTemporaryDisabledSFX = true;
    return 0;
  }

  return 1;
}

// Clear a render target. Optimization: Using colorfill now
void ClearRenderTarget(LPDIRECT3DDEVICE9 plD3DDevice, STexPic *&pTex, uchar r, uchar g, uchar b, uchar a)
{
  LPDIRECT3DSURFACE9 pTexSurf=GetTextureSurface(pTex);  
  plD3DDevice->ColorFill(pTexSurf, 0, D3DCOLOR_RGBA(r, g, b, a));
  SAFE_RELEASE(pTexSurf)
}

// Copy screen into texture
void CopyScreenToTexture(RECT &pSrcRect, STexPic *&pDst)
{
  LPDIRECT3DSURFACE9 pBackSurface=gcpRendD3D->mfGetBackSurface();
  LPDIRECT3DSURFACE9 pTexSurf= GetTextureSurface(pDst);

  RECT pDstRect={0, 0, pDst->m_Width, pDst->m_Height };
  gcpRendD3D->m_pd3dDevice->StretchRect(pBackSurface, &pSrcRect, pTexSurf, &pDstRect, D3DTEXF_NONE);
  SAFE_RELEASE(pTexSurf) 
}

// BlurTextureHw - blur texture in hardware
bool BlurTextureHw(CScreenVars *pVars, STexPic *&pTex, int iBlurType, int iBlurAmount)
{
  // make sure data ok
  if(!pTex)
  {
    return 0;
  }

  LOG_EFFECT("*** Begin texture bluring process... ***\n")

  // get vertex/fragment program  
  CCGVProgram_D3D *vpBlur=(CCGVProgram_D3D *) pVars->m_pVPBlur;
  CCGPShader_D3D *fpBlur=(CCGPShader_D3D *) pVars->m_pRCBlur;
  
  // get current viewport
  int iTmpX, iTmpY, iTempWidth, iTempHeight;
  gcpRendD3D->GetViewport(&iTmpX, &iTmpY, &iTempWidth, &iTempHeight);   

  // resize screen to fit texture 
  gcpRendD3D->SetViewport( iTmpX, iTmpY, pTex->m_Width, pTex->m_Height );

  // blur texture

  // set current vertex/fragment program    
  vpBlur->mfSet(true, 0);
  fpBlur->mfSet(true, 0);

  // setup texture offsets, for texture neighboors sampling
  float s1=1.0f/(float) pTex->m_Width;     
  float t1=1.0f/(float) pTex->m_Height;   
  float s_off=s1*0.5f; 
  float t_off=t1*0.5f; 

  float pfOffset0[]={  s1*0.5f,    t1, 0.0f, 0.0f}; 
  float pfOffset1[]={  -s1,   t1*0.5f, 0.0f, 0.0f}; 
  float pfOffset2[]={ -s1*0.5f,   -t1, 0.0f, 0.0f}; 
  float pfOffset3[]={  s1,     -t1*0.5f, 0.0f, 0.0f};  

  // render quad
  SetTexture( pTex, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1); 
  SetTexture( pTex, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);
  SetTexture( pTex, 2, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);
  SetTexture( pTex, 3, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);

  // set current vertex/fragment program
  vpBlur->mfSet(true, NULL);
  fpBlur->mfSet(true, NULL);
  
  // set vertex program consts 
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

  // blur texture  
  //if(iBlurType==0)  // simple box blur
  {
    for(int iBlurPasses=1; iBlurPasses<= iBlurAmount;  iBlurPasses++) 
    {
      // set texture coordinates scale (needed for rectangular textures in gl ..)
      float pfScale[]={ 1.0f, 1.0f, 1.0f, (float) iBlurPasses};     
      vpBlur->mfParameter4f("vTexCoordScale", pfScale);

      // optimization based on kawase filter, to reduce texture bluring to half passes
      float fPasses=(float)iBlurPasses*0.5f;   
      
      if(iBlurType==1)
      {
        fPasses=1; //0.25f;                         
      }

      float pfOffset0[]={  s1*0.5f*fPasses,    t1*fPasses, 0.0f, 0.0f};      
      float pfOffset1[]={  -s1*fPasses,   t1*0.5f*fPasses, 0.0f, 0.0f}; 
      float pfOffset2[]={ -s1*0.5f*fPasses,   -t1*fPasses, 0.0f, 0.0f}; 
      float pfOffset3[]={  s1*fPasses,     -t1*0.5f*fPasses, 0.0f, 0.0f};  
      // set vertex program consts
      vpBlur->mfParameter4f("Offset0", pfOffset0);
      vpBlur->mfParameter4f("Offset1", pfOffset1);
      vpBlur->mfParameter4f("Offset2", pfOffset2);
      vpBlur->mfParameter4f("Offset3", pfOffset3);

      gcpRendD3D->SetState(GS_NODEPTHTEST);
       // render screen aligned quad...
      gcpRendD3D->DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  

      // copy screen to texture      
      RECT pSrcRect={iTmpX, iTmpY, iTmpX+pTex->m_Width, iTmpY+pTex->m_Height };
      CopyScreenToTexture(pSrcRect, pTex);
    }
  }

  vpBlur->mfSet(false, 0);
  fpBlur->mfSet(false, 0);

  // restore previous viewport  
  gcpRendD3D->SetViewport(iTmpX, iTmpY, iTempWidth, iTempHeight);

  LOG_EFFECT("*** End texture bluring process... ***\n")
  return 1; 
}

// ResizeTextureHw - resize a texture in hardware, using bilinear resampling
// Note: Change this so that no pixel shader is required at all, i just need harware linear resampling...
bool ResizeTextureHw(CScreenVars *pVars, STexPic *&pSrc, STexPic *&pDst)
{
  // make sure data ok
  if(!pSrc || !pDst || !pVars)
  {
    return 0;
  }

  LOG_EFFECT("*** Begin texture resampling process... ***\n")

  int iTempX, iTempY, iWidth, iHeight;
  gcpRendD3D->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

  // get vertex/fragment program  
  CCGVProgram_D3D *vpBlur=(CCGVProgram_D3D *) pVars->m_pVPBlur;
  CCGPShader_D3D *fpBlur=(CCGPShader_D3D *) pVars->m_pRCBlur;

  // get texture/surface data
  LPDIRECT3DSURFACE9 plDstSurf=GetTextureSurface(pDst);

  // resize screen to fit texture   
  gcpRendD3D->EF_SetRenderTarget(plDstSurf, 1);
  gcpRendD3D->SetViewport( 0, 0, pDst->m_Width, pDst->m_Height );

  // set current vertex/fragment program    
  vpBlur->mfSet(true, 0);
  fpBlur->mfSet(true, 0);

  // setup texture offsets, for texture neighboors sampling
  float s1=1.0f/(float) pSrc->m_Width;     
  float t1=1.0f/(float) pSrc->m_Height;   
  float s_off=s1*0.5f; 
  float t_off=t1*0.5f; 

  float pfOffset0[]={  s1*0.5f,      t1, 0.0f, 0.0f}; 
  float pfOffset1[]={      -s1, t1*0.5f, 0.0f, 0.0f}; 
  float pfOffset2[]={ -s1*0.5f,     -t1, 0.0f, 0.0f}; 
  float pfOffset3[]={       s1,-t1*0.5f, 0.0f, 0.0f};  

  // render quad
  SetTexture( pSrc, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1); 
  SetTexture( pSrc, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);
  SetTexture( pSrc, 2, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);
  SetTexture( pSrc, 3, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);

  // set current vertex/fragment program
  vpBlur->mfSet(true, NULL);
  fpBlur->mfSet(true, NULL);

  gRenDev->SetState(GS_NODEPTHTEST);

  // set vertex program consts
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

  // resample texture
  // set texture coordinates scale (needed for rectangular textures in gl ..)
  float pfScale[]={ 1.0f, 1.0f, 1.0f, 0};
  vpBlur->mfParameter4f("vTexCoordScale", pfScale);

  // render screen aligned quad...
  gcpRendD3D->DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  

  // restore previous render target
  gcpRendD3D->EF_RestoreRenderTarget();
  gcpRendD3D->SetViewport(iTempX, iTempY, iWidth, iHeight);
  SAFE_RELEASE(plDstSurf)

  vpBlur->mfSet(false, 0);
  fpBlur->mfSet(false, 0);

  LOG_EFFECT("*** End texture resampling process... ***\n")
  return 1; 
}


// flashbang helper
inline float InterpolateCubic(float fCp1, float fCp2, float fCp3, float fCp4, float fTime) 
{ 
  float fTimeSquare = fTime*fTime; 
  return (((-fCp1 * 2.0f) + (fCp2 * 5.0f) - (fCp3 * 4) + fCp4) / 6.0f) * fTimeSquare * fTime + 
    (fCp1 + fCp3 - (2.0f * fCp2)) * fTimeSquare + (((-4.0f * fCp1) + fCp2 + (fCp3 * 4.0f) - fCp4) / 6.0f) * fTime + fCp2; 
}

// Render screen post-processing effects for low-spec machines..
bool CREScreenProcess:: mfDrawLowSpec(SShader *ef, SShaderPass *sfm)
{    
  // MUST RESET TO DEFAULT
  gRenDev->ResetToDefault(); 

  // get data
  ITimer *pTimer=iSystem->GetITimer();
  CD3D9Renderer *pRenderer = gcpRendD3D;   
  int iTempX, iTempY, iWidth, iHeight;
  gRenDev->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

  // setup shared renderstates
  gRenDev->Set2DMode(true, 1, 1);
  gRenDev->SetState(GS_NODEPTHTEST);

  // setup shared screen aligned quad

  // add texel displacement adjustment
  float fSoff=0.5f/(float)iWidth;
  float fToff=0.5f/(float)iHeight;

  struct_VERTEX_FORMAT_P3F_TEX2F pScreenQuad[] =
  {
    Vec3(-fSoff, -fToff, 0), 0, 0,   
    Vec3(-fSoff, 1-fToff, 0), 0, 1,    
    Vec3(1-fSoff, -fToff, 0), 1, 0,   
    Vec3(1-fSoff, 1-fToff, 0), 1, 1,   
  };

  // =======================
  // Cryvision screen effect
  if(m_pVars->m_iNightVisionActive)
  {        
    LOG_EFFECT("*** Begin cryvision process... ***\n")

    // some bug in  set material color, must pass color trough vertices instead ...
    gRenDev->SetMaterialColor(0.4f, 0.5f, 0.6f, 1.0f);  

    // simple 'noise', using texture fast translation
    static float fNoise=0;    
    fNoise+=(pTimer->GetFrameTime())*20.0f; 

    if(fNoise>2.0f)
    {
      fNoise=0.0f;
    }

    struct_VERTEX_FORMAT_P3F_TEX2F pScrQuad[] =
    {
      Vec3(0, 0, 0), 0, fNoise,
      Vec3(0, 1, 0), 0, 2+fNoise, 
      Vec3(1, 0, 0), 2, 0+fNoise,
      Vec3(1, 1, 0), 2, 2+fNoise, 
    };

    // 1st pass: set stages, bright color a bit & render
    gRenDev->SetMaterialColor(0.0f, 0.2f, 0.3f, 1.0f);     
    gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);    

    STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;
    SetTexture( pWhiteTex, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 0); 
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScrQuad,VERTEX_FORMAT_P3F_TEX2F)),4);  

    // 2nd pass: set stages, set cryvision color & render
    gRenDev->SetMaterialColor(0.4f, 0.6f, 0.8f, 1.0f);   
    gRenDev->SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_NODEPTHTEST);     

    STexPic *pScreenNoiseTex = gRenDev->m_TexMan->m_Text_ScreenNoise;
    SetTexture( pScreenNoiseTex, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 0); 
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScrQuad,VERTEX_FORMAT_P3F_TEX2F)),4);       

    LOG_EFFECT("*** End cryvision process... ***\n")
  }

  // ==============================================================================================
  // FlashBang - grenade flashbang fx
  if(m_pVars->m_bFlashBangActive)
  {
    LOG_EFFECT("*** Begin FlashBang... ***\n");

    // render flashbang flash 
    STexPic *pFlashBangFlash = gRenDev->m_TexMan->m_Text_FlashBangFlash;

    // get flashbang properties
    float fPosX, fPosY, fSizeX, fSizeY;
    fPosX=m_pVars->m_fFlashBangFlashPosX;
    fPosY=m_pVars->m_fFlashBangFlashPosY;
    fSizeX=m_pVars->m_fFlashBangFlashSizeX;
    fSizeY=m_pVars->m_fFlashBangFlashSizeY;

    float fBrightness=1.0f;  
    fBrightness=InterpolateCubic(0.0f, .0f, 1.0f, .5f, m_pVars->m_fFlashBangTimeOut);

    // set render states        
    gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
    gRenDev->SetMaterialColor(fBrightness, fBrightness, fBrightness, 1);   
    STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;      
    SetTexture( pWhiteTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)), 4);

    // display flashbang flash (not much visible, check ways to improve)
    //gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
    //gRenDev->Draw2dImage(fPosX-fSizeX*0.5f*0.8f, fPosY-fSizeY*0.5f*0.8f, fSizeX*0.8f, fSizeY*0.8f, pFlashBangFlash->GetTextureID());

    // sincronize
    float fTimeScale=1.0f;
    if(m_pVars->m_fFlashBangTimeScale)
    {
      fTimeScale=1.0f/m_pVars->m_fFlashBangTimeScale;
    }

    m_pVars->m_fFlashBangTimeOut-=fTimeScale*pTimer->GetFrameTime();

    // reset animation
    if(m_pVars->m_fFlashBangTimeOut<=0.01f) 
    {
      m_pVars->m_bFlashBangActive=0;
      m_pVars->m_fFlashBangTimeOut=1.0f;
    }

    LOG_EFFECT("*** End FlashBang... ***\n");
  }

  // ============================================
  // Fade Fx - fade between screen and some color
  if(m_pVars->m_bFadeActive)
  {
    LOG_EFFECT("*** Begin screen fade process... ***\n")

      gRenDev->ResetToDefault();    

    // interpolate values
    float fStep, fSign=(m_pVars->m_fFadeTime<0.0f)? -1.0f: 1.0f;    
    if(m_pVars->m_fFadeCurrPreTime>=m_pVars->m_fFadePreTime)
    {
      m_pVars->m_fFadeCurrTime-= pTimer->GetFrameTime();

      // fade its only disabled in 'fade in' case, in fade out case, user should disable it
      if(m_pVars->m_fFadeCurrTime<0.0f)
      {       
        m_pVars->m_fFadeCurrTime=0.0f;
        m_pVars->m_fFadePreTime=0.0f;
        m_pVars->m_bFadeActive=0;
        m_pVars->m_fFadeCurrPreTime=0.0f;                  
      }
    }

    fStep=m_pVars->m_fFadeCurrTime/fabsf(m_pVars->m_fFadeTime); 

    if(fSign>0.0f) 
    {
      fStep=1.0f-fStep;       
    }

    // count 'pre-fade' frame number
    m_pVars->m_fFadeCurrPreTime+=1.0f; 
    m_pVars->m_pFadeCurrColor.set(m_pVars->m_pFadeColor.r, m_pVars->m_pFadeColor.g, m_pVars->m_pFadeColor.b, fStep);        

    // copy fade amount    
    ICVar *pHudFadeAmount=iConsole->GetCVar("hud_fadeamount");
    if(pHudFadeAmount)
    {
      pHudFadeAmount->Set(1-fStep);
    }

    // pass: set stages, fade color & render
    gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);    
    gRenDev->SetMaterialColor(m_pVars->m_pFadeCurrColor.r, m_pVars->m_pFadeCurrColor.g, m_pVars->m_pFadeCurrColor.b, m_pVars->m_pFadeCurrColor.a);

    STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;      
    SetTexture( pWhiteTex, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)),4);    

    LOG_EFFECT("*** End screen fade process... ***\n")
  }

  gRenDev->Set2DMode(false, 1, 1); 
  gRenDev->ResetToDefault();
  return 1;
}

// Render screen post-processing effects
bool CREScreenProcess:: mfDraw(SShader *ef, SShaderPass *sfm)
{
  // make sure everything's ok..
  if(!gRenDev || !ef || !sfm)
  {
    return 0;
  }
  FRAME_PROFILER( "ScreenRender:mfDraw",iSystem,PROFILE_GAME );
  
  // get cryvision state
  m_pVars->m_iNightVisionActive=CRenderer::CV_r_cryvision;

  if(CRenderer::CV_r_disable_sfx==1)
  {
    CRenderer::CV_r_maxtexlod_bias=0.0f;
  }
  else
  {    
    if(m_pVars->m_bCartoonActive)
    {
      CRenderer::CV_r_maxtexlod_bias=-1.5f;
    }
    else
    {
      CRenderer::CV_r_maxtexlod_bias=0.0f;
    }
  }

  // Do not allow cinematic rendermode in patch02
  if(CRenderer::CV_r_rendermode>4)
  {
    CRenderer::CV_r_rendermode=4;
  }
  
  // get data
  ITimer *pTimer=iSystem->GetITimer();
  CD3D9Renderer *pRenderer = gcpRendD3D;

  // no pixel shaders support? low spec active? then process low-spec screen effects..
  if(!(pRenderer->GetFeatures() & RFT_HW_TS) || CRenderer::CV_r_disable_sfx || gRenDev->m_bTemporaryDisabledSFX)
  {      
    mfDrawLowSpec(ef, sfm);
    return 0;
  }

  // reset states to default, just to make sure everything ok
  gRenDev->ResetToDefault();  

  // setup shared renderstates
  pRenderer->SetPolygonMode(R_SOLID_MODE);  
  gRenDev->Set2DMode(true, 1, 1);
  gRenDev->SetState(GS_NODEPTHTEST);

  // helpers for render to texture
  //LPDIRECT3DTEXTURE9 plD3DTexture;
  LPDIRECT3DSURFACE9 pTexSurf; 
  LPDIRECT3DSURFACE9 pBackSurface=((CD3D9Renderer *)gRenDev)->mfGetBackSurface(); 

  int iTempX, iTempY, iWidth, iHeight;
  gRenDev->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);
  gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight); 
  RECT pScreenRect={iTempX, iTempY, iTempX+iWidth, iTempY+iHeight};
  
  // make sure all textures created
  STexPic *pScreenTex=gRenDev->m_TexMan->m_Text_ScreenMap;  
      
  // preload all textures
  if(!gRenDev->m_TexMan->PreloadScreenFxMaps())
  {
    return 0;
  }

  // add texel displacement adjustment
  float fSoff=0.5f/(float)iWidth;
  float fToff=0.5f/(float)iHeight;

 // setup shared screen aligned quad
  struct_VERTEX_FORMAT_P3F_TEX2F pDefaultScreenVB[] =
  {
    Vec3(0, 0, 0), 0, 0,    
    Vec3(0, 1, 0), 0, 1,    
    Vec3(1, 0, 0), 1, 0,   
    Vec3(1, 1, 0), 1, 1,   
  };

  // setup shared screen aligned quad
  struct_VERTEX_FORMAT_P3F_TEX2F pScreenQuad[] =
  {
    Vec3(-fSoff, -fToff, 0), 0, 0,   
    Vec3(-fSoff, 1-fToff, 0), 0, 1,    
    Vec3(1-fSoff, -fToff, 0), 1, 0,   
    Vec3(1-fSoff, 1-fToff, 0), 1, 1,   
  };
  
  // fixed scale..
  float pfScale[]={ 1, 1, 1, 1};    
 
  // make sure texture created..
  if(pScreenTex->m_RefTex.m_VidTex) 
  {       
    LOG_EFFECT("*** Begin updating screen texture... ***\n")  
    CopyScreenToTexture(pScreenRect, pScreenTex);
    LOG_EFFECT("*** End updating screen texture... ***\n") 
  }
     
  // ==============================================================================================
  // any effect requires screen low res image version ?
  if((m_pVars->m_bBlurActive || m_pVars->m_bGlareActive) && !m_pVars->m_bCartoonActive)
  {                    
    LOG_EFFECT("*** Begin updating screen mip maps textures... ***\n")

    // resize textures using bilinear resampling
    if(pScreenTex->m_RefTex.m_VidTex) 
    {
      STexPic *pScreenLow = gRenDev->m_TexMan->m_Text_ScreenLowMap;
      ResizeTextureHw(m_pVars, pScreenTex, pScreenLow);

      // only glare requires all mip maps levels
      if(m_pVars->m_bGlareActive && !m_pVars->m_bBlurActive && !m_pVars->m_iNightVisionActive)  
      {
        STexPic *pScreenAvg = gRenDev->m_TexMan->m_Text_ScreenAvg1x1; 
        ResizeTextureHw(m_pVars, pScreenLow, pScreenAvg);
      }
    }

    LOG_EFFECT("*** End updating screen mip maps textures... ***\n")
  } 

  // ==============================================================================================
  // Glare/Nightvision - Glare fx and Nightvision w/ glare
  // TODO: add multiple glare type filters  
  
  if((m_pVars->m_iNightVisionActive && m_pVars->m_bCartoonActive) || (m_pVars->m_bGlareActive && !m_pVars->m_bCartoonActive) || m_pVars->m_iNightVisionActive)  
  {   
    LOG_EFFECT("*** Begin glare... ***\n");

    if(!m_pVars->m_iNightVisionActive && ( (CRenderer::CV_r_rendermode!=5) || (CRenderer::CV_r_rendermode==5 && REQUIRES_PS20)) )
    {      
      // get data      
      STexPic   *pTex = gRenDev->m_TexMan->m_Text_Glare,
                *pScreenCurrLuminosityTex = gRenDev->m_TexMan->m_Text_ScreenCurrLuminosityMap,
                *pScreenLow = gRenDev->m_TexMan->m_Text_ScreenLowMap,
                *pScreenAvg = gRenDev->m_TexMan->m_Text_ScreenAvg1x1;
      
      // get vertex/fragment programs    
      CCGPShader_D3D *fpGlareAmount=(CCGPShader_D3D *) m_pVars->m_pRCGlareAmountMap;
      CCGPShader_D3D *fpGlareMap=(CCGPShader_D3D *) m_pVars->m_pRCGlareMap;

      CCGVProgram_D3D *vpGlare=(CCGVProgram_D3D *) m_pVars->m_pVPGlare;
      CCGPShader_D3D *fpGlare=(CCGPShader_D3D *) m_pVars->m_pRCGlare;
      CCGPShader_D3D *fpRenderModeCold=(CCGPShader_D3D *) m_pVars->m_pRCRenderModeCold;
      CCGPShader_D3D *fpRenderModeAdv=(CCGPShader_D3D *) m_pVars->m_pRCRenderModeAdv;

      gRenDev->SetViewport(0, 0, iWidth, iHeight);      
      int iGlareQuality=CLAMP(CRenderer::CV_r_glarequality, 0, 3);

      // set renderstate    
      gRenDev->SetMaterialColor(1,1,1,1);
      gRenDev->SetState(GS_NODEPTHTEST);

      // ---------------------------------
      // compute screen luminosity texture 
      // ---------------------------------
      if(CRenderer::CV_r_rendermode!=5) 
      {   
        // set constants      
        float fLastGlareAmount=0;  
        static float fFrameCounter=0;

        // slowdown transition ..
        if(fFrameCounter>=CRenderer::CV_r_glaretransition/(pTimer->GetFrameTime()*125.0f+0.001f))    
        {
          // get texture surface      
          pTexSurf=GetTextureSurface(pScreenCurrLuminosityTex);

          // set current rendertarget              
          pRenderer->EF_SetRenderTarget(pTexSurf, 1);
          SAFE_RELEASE(pTexSurf) 

          // get average screen luminosity
          gRenDev->SetViewport( 0, 0, pScreenCurrLuminosityTex->m_Width, pScreenCurrLuminosityTex->m_Height); 

          // set pixel/vertex programs
          vpGlare->mfSet(true, 0);
          fpGlareAmount->mfSet(true, 0);

          // set texture coordinates scale (needed for rectangular textures in gl ..)         
          float pfTexel01[4]= { 1, 1, -0.5f, -0.5f};       
          vpGlare->mfParameter4f("vTexCoordScale01", pfTexel01); 
          float pfTexel02[4]= { 1, 1, -0.5f, 0.5f};      
          vpGlare->mfParameter4f("vTexCoordScale02", pfTexel02); 
          float pfTexel03[4]= { 1, 1, 0.5f, -0.5f};      
          vpGlare->mfParameter4f("vTexCoordScale03", pfTexel03); 
          float pfTexel04[4]= { 1, 1, 0.5f, 0.5f};      
          vpGlare->mfParameter4f("vTexCoordScale04", pfTexel04); 

          fLastGlareAmount=0.0025f*pTimer->GetFrameTime()*1000.0f;       
          fFrameCounter=0;

          // set screen texture/state       
          SetTexture( pScreenAvg, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);  
          SetTexture( pScreenAvg, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1); 
          SetTexture( pScreenAvg, 2, D3DTEXF_POINT, D3DTEXF_POINT, 1); 
          SetTexture( pScreenAvg, 3, D3DTEXF_POINT, D3DTEXF_POINT, 1); 

          float pGlare[]= { 1, 1, 1,  fLastGlareAmount}; 
          fpGlareAmount->mfParameter4f("Glare", pGlare);

          // use motion blur to lerp between brightness values
          gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
 
          gRenDev->DrawTriStrip(&(CVertexBuffer (pDefaultScreenVB, VERTEX_FORMAT_P3F_TEX2F)), 4);
          
          vpGlare->mfSet(false, 0);
          fpGlareAmount->mfSet(false, 0);   
          
          pRenderer->EF_RestoreRenderTarget();
          gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
        }

        fFrameCounter+=(float) 1.0f;
      }
      else
      {
        // get texture surface      
        ClearRenderTarget(gcpRendD3D->m_pd3dDevice, pScreenCurrLuminosityTex, 0,0,0,0);        
      }

      // cold render mode ?  then must apply unsharp filter on screen texture
      if(CRenderer::CV_r_rendermode==3)      
      {
        // set current rendertarget                      
        gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);

        // set pixel/vertex programs
        vpGlare->mfSet(true, 0);      
        fpRenderModeCold->mfSet(true, 0); 

        float pfScreenSize[4]= { 1, 1, TEXEL_TO_SCREEN(pScreenTex->m_Width), TEXEL_TO_SCREEN(pScreenTex->m_Height) };      
        vpGlare->mfParameter4f("vTexCoordScale01", pfScreenSize); 
        float pfBluredSize[4]= { 1, 1, TEXEL_TO_SCREEN(pScreenLow->m_Width), TEXEL_TO_SCREEN(pScreenLow->m_Height) };  
        vpGlare->mfParameter4f("vTexCoordScale02", pfBluredSize); 

        SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1); 
        SetTexture( pScreenLow, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);  

        // render quad 
        gRenDev->SetState(GS_NODEPTHTEST);
        gRenDev->DrawTriStrip(&(CVertexBuffer (pDefaultScreenVB, VERTEX_FORMAT_P3F_TEX2F)), 4);    

        fpGlare->mfSet(false, 0); 
        fpRenderModeCold->mfSet(false, 0);

        // update screen texture          
        CopyScreenToTexture(pScreenRect, pScreenTex);
      }

      // ------------------ 
      // generate glare map
      // ------------------

      // get texture surface    
      pTexSurf=GetTextureSurface(pTex);

      // set current rendertarget  
      pRenderer->EF_SetRenderTarget(pTexSurf, 1);
      SAFE_RELEASE(pTexSurf) 
      gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height);

      // set fragment program    
      vpGlare->mfSet(true, 0);      
      fpGlareMap->mfSet(true, 0);

      // set default image enhancing values
      float pGlareMapConsts[]= { 0.2f, 0.2f, 0.2f, 1.0f };
      float pSaturationConsts[]= { 0.0f, 0.0f, 0.0f, 0.2f };
      float pContrastConsts[]= { 0.0f, 0.0f, 0.85f, 0.15f };

      // is this really necessary... ?      
      if(CRenderer::CV_r_rendermode==1) // normal render mode
      {      
        // adjust glare properties      
        if(CRenderer::CV_r_glare==2)  // outdoor position ?
        {
          // threshold
          pGlareMapConsts[0]= 0.2f;
          pGlareMapConsts[1]= 0.2f;
          pGlareMapConsts[2]= 0.2f;
          // glare amount
          pGlareMapConsts[3]= 0.5f;
        }
        else 
          if(CRenderer::CV_r_glare==3)  // indoor position ?
          {
            // threshold
            pGlareMapConsts[0]= 0.25f;
            pGlareMapConsts[1]= 0.25f;
            pGlareMapConsts[2]= 0.25f;
            // glare amount
            pGlareMapConsts[3]= 0.8f;
          }

          // set saturation amount
          pSaturationConsts[3]=0.2f; 

          // set contrast amount
          pContrastConsts[2]= 0.05f; 
          pContrastConsts[3]= 0.95f;        
      }
      else
        if(CRenderer::CV_r_rendermode==2) // paradisiacal render mode
        {      
          // adjust glare properties     
          if(CRenderer::CV_r_glare==2)   // outdoor position ?
          {
            // threshold
            pGlareMapConsts[0]= 0.2f;
            pGlareMapConsts[1]= 0.2f;
            pGlareMapConsts[2]= 0.2f;
            // glare amount
            pGlareMapConsts[3]= 1.0f;
          }
          else 
            if(CRenderer::CV_r_glare==3)  // indoor position ?
            {
              // threshold
              pGlareMapConsts[0]= 0.1f;
              pGlareMapConsts[1]= 0.1f;
              pGlareMapConsts[2]= 0.1f;
              // glare amount
              pGlareMapConsts[3]= 0.8f;
            }

            // set saturation amount
            pSaturationConsts[3]=0.1f;

            // set contrast amount 
            pContrastConsts[2]= 0.05f;
            pContrastConsts[3]= 0.95f;       
        }
        else
          if(CRenderer::CV_r_rendermode==3) // cold reality render mode
          {            
            // adjust glare properties     
            if(CRenderer::CV_r_glare==2)   // outdoor position ?
            {
              // threshold
              pGlareMapConsts[0]= 0.2f;
              pGlareMapConsts[1]= 0.2f;
              pGlareMapConsts[2]= 0.2f;
              // glare amount
              pGlareMapConsts[3]= 0.5f;
            }
            else 
              if(CRenderer::CV_r_glare==3)  // indoor position ?
              {
                // threshold
                pGlareMapConsts[0]= 0.1f;
                pGlareMapConsts[1]= 0.1f;
                pGlareMapConsts[2]= 0.1f;
                // glare amount
                pGlareMapConsts[3]= 0.65f;
              }

              // set saturation amount
              pSaturationConsts[3]=0.425f; 

              // set contrast amount 
              pContrastConsts[2]= 0.0f;
              pContrastConsts[3]= 1.0f;   
          }
          else
          if(CRenderer::CV_r_rendermode==5)
          {
            pSaturationConsts[3]=0;
          } 

          // glare vars                        
                      
          // make smooth transition            
          float fAdjustStep=pTimer->GetFrameTime();            
          m_pVars->m_pCurrGlareMapConst.r=CLAMP(m_pVars->m_pCurrGlareMapConst.r+(pGlareMapConsts[0]-m_pVars->m_pCurrGlareMapConst.r)*fAdjustStep, 0.0f, 1.0f);
          m_pVars->m_pCurrGlareMapConst.g=CLAMP(m_pVars->m_pCurrGlareMapConst.g+(pGlareMapConsts[1]-m_pVars->m_pCurrGlareMapConst.g)*fAdjustStep, 0.0f, 1.0f);
          m_pVars->m_pCurrGlareMapConst.b=CLAMP(m_pVars->m_pCurrGlareMapConst.b+(pGlareMapConsts[2]-m_pVars->m_pCurrGlareMapConst.b)*fAdjustStep, 0.0f, 1.0f);
          m_pVars->m_pCurrGlareMapConst.a=CLAMP(m_pVars->m_pCurrGlareMapConst.a+(pGlareMapConsts[3]-m_pVars->m_pCurrGlareMapConst.a)*fAdjustStep, 0.0f, 1.0f);
          m_pVars->m_pCurrSaturation.a=CLAMP(m_pVars->m_pCurrSaturation.a+(pSaturationConsts[3]-m_pVars->m_pCurrSaturation.a)*fAdjustStep, 0.0f, 1.0f);
          m_pVars->m_pCurrContrast.b=CLAMP(m_pVars->m_pCurrContrast.b+(pContrastConsts[2]-m_pVars->m_pCurrContrast.b)*fAdjustStep, 0.0f, 1.0f);
          m_pVars->m_pCurrContrast.a=CLAMP(m_pVars->m_pCurrContrast.a+(pContrastConsts[3]-m_pVars->m_pCurrContrast.a)*fAdjustStep, 0.0f, 1.0f);

          // pass constants (note: i negated values from here, since cg generates wrong instructions)
          float pNegCurrGlareMapConst[]={ -m_pVars->m_pCurrGlareMapConst.r, -m_pVars->m_pCurrGlareMapConst.g, -m_pVars->m_pCurrGlareMapConst.b, m_pVars->m_pCurrGlareMapConst.a};      
          fpGlareMap->mfParameter4f("vGlare", pNegCurrGlareMapConst); 
          
          if(CRenderer::CV_r_rendermode!=5)
          {
            float pAmountConst[]={ 0, 0, 0, 0};      
            fpGlareMap->mfParameter4f("vGlareAmount", pAmountConst); 
          }
          else
          {
            float pAmountConst[]={ 1, 1, 1, 1};      
            fpGlareMap->mfParameter4f("vGlareAmount", pAmountConst); 
          }          

          float pfBluredSize[4]= { 1, 1, TEXEL_TO_SCREEN(pScreenLow->m_Width), TEXEL_TO_SCREEN(pScreenLow->m_Height) };  
          vpGlare->mfParameter4f("vTexCoordScale01", pfBluredSize); 
          float pfLumSize[4]= { 1, 1, 0, 0 };  
          vpGlare->mfParameter4f("vTexCoordScale02", pfLumSize); 

          // use first mip map
          SetTexture( pScreenLow, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);             
          SetTexture( pScreenCurrLuminosityTex, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1);                     

          gRenDev->SetState(GS_NODEPTHTEST);
          gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)), 4);               
          fpGlareMap->mfSet(false, 0);
          vpGlare->mfSet(false, 0); 

          // restore backbuffer
          pRenderer->EF_RestoreRenderTarget(pBackSurface);
          gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);

          // blur texture
          if(CRenderer::CV_r_rendermode!=3)   
          {              
            BlurTextureHw(m_pVars, pTex, 0, (iGlareQuality*2<3)? 3:iGlareQuality*2);
          } 
           
          gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
          
          // ----------------------------------------
          // apply glare in image and post-process it
          // ----------------------------------------
          if(CRenderer::CV_r_rendermode!=5)
          {
            // set pixel/vertex programs
            vpGlare->mfSet(true, 0);
            fpGlare->mfSet(true, 0); 

            // due to cg bug, need to pass 2 diferent constants..
            float pCurrContrast01[]= { 0.0f, 0.0f, 0.0f, m_pVars->m_pCurrContrast.b }; 
            float pCurrContrast02[]= { 0.0f, 0.0f, 0.0f, m_pVars->m_pCurrContrast.a };       
            // set pixel program constants
            fpGlare->mfParameter4f("vSaturationAmount", pSaturationConsts);        
            fpGlare->mfParameter4f("vConstrastAmount", pCurrContrast01);    
            fpGlare->mfParameter4f("vConstrastAmount02", pCurrContrast02);  

            // set texture coordinates scale (needed for rectangular textures in gl ..)        
            float pfTexel01[4]= { 1, 1, TEXEL_TO_SCREEN(pScreenTex->m_Width), TEXEL_TO_SCREEN(pScreenTex->m_Height)};       
            vpGlare->mfParameter4f("vTexCoordScale01", pfTexel01); 
            float pfTexel02[4]= { 1, 1, TEXEL_TO_SCREEN(pTex->m_Width), TEXEL_TO_SCREEN(pTex->m_Height)};      
            vpGlare->mfParameter4f("vTexCoordScale02", pfTexel02); 
            float pfTexel03[4]= { 1, 1, 0, 0};       
            vpGlare->mfParameter4f("vTexCoordScale03", pfTexel03); 

            // set screen, glare map and glare adjustment texture
            SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);      
            SetTexture( pTex, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);    

            // render quad
            gRenDev->SetState(GS_NODEPTHTEST);
            gRenDev->DrawTriStrip(&(CVertexBuffer (pDefaultScreenVB, VERTEX_FORMAT_P3F_TEX2F)), 4);  

            fpGlare->mfSet(false, 0); 
            vpGlare->mfSet(false, 0);
          }    
          else            
          {
            // set pixel/vertex programs
            vpGlare->mfSet(true, 0);
            fpRenderModeAdv->mfSet(true, 0); 

            float pParams01[]= { CRenderer::CV_r_pp_contrast, CRenderer::CV_r_pp_saturation, CRenderer::CV_r_pp_brightness, CRenderer::CV_r_pp_glareintensity }; 
            float pParams02[]= { 0.0f, 0.0f, CRenderer::CV_r_pp_sharpenamount, (CRenderer::CV_r_pp_gamma>0)?1.0f/CRenderer::CV_r_pp_gamma: 0.0f};       
            float pCMYKParams[]= { CRenderer::CV_r_pp_cmyk_c, CRenderer::CV_r_pp_cmyk_m, CRenderer::CV_r_pp_cmyk_y, CRenderer::CV_r_pp_cmyk_k };       

            // set pixel program constants
            fpRenderModeAdv->mfParameter4f("vParams01", pParams01);        
            fpRenderModeAdv->mfParameter4f("vParams02", pParams02);    
            fpRenderModeAdv->mfParameter4f("vCMYKParams", pCMYKParams);  

            // set texture coordinates scale (needed for rectangular textures in gl ..)        
            float pfTexel01[4]= { 1, 1, TEXEL_TO_SCREEN(pScreenTex->m_Width), TEXEL_TO_SCREEN(pScreenTex->m_Height)};       
            vpGlare->mfParameter4f("vTexCoordScale01", pfTexel01); 
            float pfTexel02[4]= { 1, 1, TEXEL_TO_SCREEN(pTex->m_Width), TEXEL_TO_SCREEN(pTex->m_Height)};      
            vpGlare->mfParameter4f("vTexCoordScale02", pfTexel02); 
            float pfTexel03[4]= { 1, 1, TEXEL_TO_SCREEN(pScreenLow->m_Width), TEXEL_TO_SCREEN(pScreenLow->m_Height)};        
            vpGlare->mfParameter4f("vTexCoordScale03", pfTexel03); 

            // set screen, glare map and glare adjustment texture
            SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);      
            SetTexture( pTex, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);         
            SetTexture( pScreenLow, 2, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);      

            // render quad
            gRenDev->SetState(GS_NODEPTHTEST);
            gRenDev->DrawTriStrip(&(CVertexBuffer (pDefaultScreenVB, VERTEX_FORMAT_P3F_TEX2F)), 4);  

            fpRenderModeAdv->mfSet(false, 0);   
            vpGlare->mfSet(false, 0);           
          }

          // check if motion blur is not active, else we must update screen texture/or get latest screen texture
          if(m_pVars->m_bFlashBangActive ||  m_pVars->m_bCartoonActive || m_pVars->m_bBlurActive || m_pVars->m_bDofActive)
          {
            CopyScreenToTexture(pScreenRect, pScreenTex);
          }

          gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight); 
    }
    else
    { 
      // ------------------ 
      // process cry-vision
      // ------------------

      LOG_EFFECT("*** Begin Cryvision... ***\n");
      // get data      
      STexPic   *pTex = gRenDev->m_TexMan->m_Text_Glare,
                *pHeatTexture = gRenDev->m_TexMan->m_Text_ScreenLowMap;

      // get vertex/fragment programs    
      CCGPShader_D3D *fpGlareMap=(CCGPShader_D3D *) m_pVars->m_pRCGlareMap;      

      CCGVProgram_D3D *vpGlare=(CCGVProgram_D3D *) m_pVars->m_pVPGlare;
      CCGPShader_D3D *fpGlare=(CCGPShader_D3D *) m_pVars->m_pRCGlare;

      CCGVProgram_D3D *vpNightGlare=(CCGVProgram_D3D *) m_pVars->m_pVPNightVision;
      CCGPShader_D3D *fpNightGlare=(CCGPShader_D3D *) m_pVars->m_pRCNightVision;

      CCGPShader_D3D *fpHeatSource=(CCGPShader_D3D *) m_pVars->m_pRCHeatVision;
      CCGPShader_D3D *fpHeatSourceDecode=(CCGPShader_D3D *) m_pVars->m_pRCHeatSourceDecode;    

      gRenDev->SetMaterialColor(1,1,1,1);
      gRenDev->SetState(GS_NODEPTHTEST);

      // ---------------------------------
      // get heat mask from screen texture    
      // ---------------------------------
      {        
        // get texture surface    
        pTexSurf=GetTextureSurface(pHeatTexture);

        // set current rendertarget  
        pRenderer->EF_SetRenderTarget(pTexSurf, 1);
        SAFE_RELEASE(pTexSurf)      

        gRenDev->SetViewport(0, 0, pHeatTexture->m_Width, pHeatTexture->m_Height); 

        // set pixel/vertex programs 
        vpNightGlare->mfSet(true, 0);
        fpHeatSource->mfSet(true, 0);  

        // set texture coordinates scale (needed for rectangular textures in gl ..)
        vpNightGlare->mfParameter4f("vTexCoordScale01", pfScale);        
        vpNightGlare->mfParameter4f("vTexCoordScale02", pfScale);   
        vpNightGlare->mfParameter4f("vTexCoordScale03", pfScale);
        float pfParams[]= { 1, 1, 0, 0};
        vpNightGlare->mfParameter4f("fNoiseParams", pfParams);   

        float fMotionBlurAmount=75.0f*pTimer->GetFrameTime();
        if(fMotionBlurAmount>1) fMotionBlurAmount=1;

        float pfHeatParams[]= { 1, 1, 1, fMotionBlurAmount};
        fpHeatSource->mfParameter4f("vHeatConstants", pfHeatParams);   

        // setup texture stages/states    
        STexPic *pPaleteTex = gRenDev->m_TexMan->m_Text_HeatPalete;
        SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
        SetTexture( pScreenTex, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1);
        SetTexture( pPaleteTex, 2, D3DTEXF_POINT, D3DTEXF_POINT, 1);

        // render quad (note: use alpha blending to add motion blur on heat sources )
        gRenDev->SetState(GS_NODEPTHTEST);  
        gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);  

        vpNightGlare->mfSet(false, 0);  
        fpHeatSource->mfSet(false, 0); 

        // blur texture
        pRenderer->EF_RestoreRenderTarget();
        gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
        BlurTextureHw(m_pVars, pHeatTexture, 0, 3);                
      }

      // ------------------------------------
      // delete heat mask from screen texture
      // ---------------------------------

      {
        gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);

        // setup texture stages/states    
        STexPic *pPaleteTex = gRenDev->m_TexMan->m_Text_HeatPalete;
        SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
        SetTexture( pScreenTex, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1);      
        SetTexture( pPaleteTex, 2, D3DTEXF_POINT, D3DTEXF_POINT, 1);

        // set pixel/vertex programs
        vpNightGlare->mfSet(true, 0);
        fpHeatSourceDecode->mfSet(true, 0); 

        // set texture coordinates scale (needed for rectangular textures in gl ..)
        vpNightGlare->mfParameter4f("vTexCoordScale01", pfScale);        
        vpNightGlare->mfParameter4f("vTexCoordScale02", pfScale);   
        vpNightGlare->mfParameter4f("vTexCoordScale03", pfScale);
        float pfParams[]= { 1, 1, 0, 0};
        vpNightGlare->mfParameter4f("fNoiseParams", pfParams);   

        // render quad, use alpha blending to add motion blur on heat sources      
        gRenDev->SetState(GS_NODEPTHTEST);
        gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);  

        vpNightGlare->mfSet(false, 0); 
        fpHeatSourceDecode->mfSet(false, 0); 

        // update screen texture        
        CopyScreenToTexture(pScreenRect, pScreenTex);
      }

      // ------------------ 
      // generate glare map
      // ------------------

      // get texture surface    
      pTexSurf=GetTextureSurface(pTex);

      // set current rendertarget  
      pRenderer->EF_SetRenderTarget(pTexSurf, 1);
      SAFE_RELEASE(pTexSurf)   

      gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height);

      // set fragment program
      vpGlare->mfSet(true, 0);
      fpGlareMap->mfSet(true, 0);

      // set default image enhancing values
      float pGlareMapConsts[]= { 0.2f, 0.2f, 0.2f, 1.0f };
      float pSaturationConsts[]= { 0.0f, 0.0f, 0.0f, 0.2f };
      float pContrastConsts[]= { 0.0f, 0.0f, 0.85f, 0.15f };

      // make smooth transition
      float fAdjustStep=pTimer->GetFrameTime();
      m_pVars->m_pCurrGlareMapConst.r=CLAMP(m_pVars->m_pCurrGlareMapConst.r+(pGlareMapConsts[0]-m_pVars->m_pCurrGlareMapConst.r)*fAdjustStep, 0.0f, 1.0f);
      m_pVars->m_pCurrGlareMapConst.g=CLAMP(m_pVars->m_pCurrGlareMapConst.g+(pGlareMapConsts[1]-m_pVars->m_pCurrGlareMapConst.g)*fAdjustStep, 0.0f, 1.0f);
      m_pVars->m_pCurrGlareMapConst.b=CLAMP(m_pVars->m_pCurrGlareMapConst.b+(pGlareMapConsts[2]-m_pVars->m_pCurrGlareMapConst.b)*fAdjustStep, 0.0f, 1.0f);
      m_pVars->m_pCurrGlareMapConst.a=CLAMP(m_pVars->m_pCurrGlareMapConst.a+(pGlareMapConsts[3]-m_pVars->m_pCurrGlareMapConst.a)*fAdjustStep, 0.0f, 1.0f);

      // pass constants (note: i negated values from here, since cg generates wrong instructions)
      float pNegCurrGlareMapConst[]={ -m_pVars->m_pCurrGlareMapConst.r, -m_pVars->m_pCurrGlareMapConst.g, -m_pVars->m_pCurrGlareMapConst.b, m_pVars->m_pCurrGlareMapConst.a};      
      fpGlareMap->mfParameter4f("vGlare", pNegCurrGlareMapConst); 
      float pAmountConst[]={ 0.35f, 0.35f, 0.35f, 0.35f}; 
      fpGlareMap->mfParameter4f("vGlareAmount", pAmountConst); 

      float pfBluredSize[4]= { 1, 1, TEXEL_TO_SCREEN(pScreenTex->m_Width), TEXEL_TO_SCREEN(pScreenTex->m_Height) };  
      vpGlare->mfParameter4f("vTexCoordScale01", pfBluredSize); 
      float pfLumSize[4]= { 1, 1, 0, 0 };  
      vpGlare->mfParameter4f("vTexCoordScale02", pfLumSize); 

      // use first mip map
      SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
      STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;
      SetTexture( pWhiteTex, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1); 

      gRenDev->SetState(GS_NODEPTHTEST);
      gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)), 4);
      fpGlareMap->mfSet(false, 0);
      vpGlare->mfSet(false, 0); 

      // restore backbuffer
      pRenderer->EF_RestoreRenderTarget();

      gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
      BlurTextureHw(m_pVars, pTex, 0, 3);
      
      gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);

      // -----------------
      // nightvision+glare

      // get noise texture
      STexPic *pScreenNoiseTex = gRenDev->m_TexMan->m_Text_ScreenNoise;

      // set pixel/vertex programs
      vpNightGlare->mfSet(true, 0);
      fpNightGlare->mfSet(true, 0); 

      // setup noise parameters
      static float fOffsetU=0, fOffsetV=0;    
      fOffsetV+=(pTimer->GetFrameTime())*20.0f;
      if(fOffsetV>2.0f)
      {
        fOffsetV=0.0f;
      }

      float pfNoiseParams[]= { 0.25f, 2.5f, fOffsetU, fOffsetV };
      vpNightGlare->mfParameter4f("fNoiseParams", pfNoiseParams);

      // set pixel program constants
      float pGlareData[]= { m_pVars->m_fGlareThreshold, m_pVars->m_fGlareThreshold, m_pVars->m_fGlareThreshold, m_pVars->m_fGlareAmount};
      fpNightGlare->mfParameter4f("Glare", pGlareData);

      float pNightVisionColor[]= { m_pVars->m_pNightVisionColor.r, m_pVars->m_pNightVisionColor.g, m_pVars->m_pNightVisionColor.b, m_pVars->m_fGlareAmount};
      fpNightGlare->mfParameter4f("NVColor", pNightVisionColor);

      // set texture coordinates scale (needed for rectangular textures in gl ..)
      vpNightGlare->mfParameter4f("vTexCoordScale01", pfScale);
      vpNightGlare->mfParameter4f("vTexCoordScale02", pfScale);
      vpNightGlare->mfParameter4f("vTexCoordScale03", pfScale);

      // set screen texture
      SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
      // set glare texture
      SetTexture( pTex, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1); 
      // heat texture
      SetTexture( pHeatTexture, 2, D3DTEXF_POINT, D3DTEXF_POINT, 1); 
      // set noise texture
      SetTexture( pScreenNoiseTex, 3, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 0);

      // render quad
      gRenDev->SetState(GS_NODEPTHTEST);   
      gRenDev->DrawTriStrip(&(CVertexBuffer (pDefaultScreenVB, VERTEX_FORMAT_P3F_TEX2F)), 4);  

      vpNightGlare->mfSet(false, 0);
      fpNightGlare->mfSet(false, 0); 

      // check if motion blur is not active, else we must update screen texture/or get latest screen texture
      if(m_pVars->m_bFlashBangActive ||  m_pVars->m_bCartoonActive || m_pVars->m_bBlurActive || m_pVars->m_bDofActive) 
      {
        CopyScreenToTexture(pScreenRect, pScreenTex);
      }

      gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight); 

      LOG_EFFECT("*** End Cryvision ***\n") 
    }     
  }

  // ==============================================================================================
  // FlashBang - grenade flashbang fx

  if(m_pVars->m_bFlashBangActive)
  {
    LOG_EFFECT("*** Begin FlashBang... ***\n");

    // get flashbang texture
    STexPic *pTex = gRenDev->m_TexMan->m_Text_FlashBangMap; 
    // get vertex/fragment program
    CCGVProgram_D3D *vpFlashBang=(CCGVProgram_D3D *) m_pVars->m_pVPFlashBang;    
    CCGPShader_D3D *fpFlashBang=(CCGPShader_D3D *) m_pVars->m_pRCFlashBang;    

    // generate flashbang texture if necessary
    if(m_pVars->m_fFlashBangTimeOut==1.0f || m_pVars->m_iFlashBangForce)  
    {
      m_pVars->m_iFlashBangForce=0;                  

      // get texture surface      
      pTexSurf=GetTextureSurface(pTex);

      // set current rendertarget
      pRenderer->EF_SetRenderTarget(pTexSurf, 1);       
      SAFE_RELEASE(pTexSurf)

      gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height );
      
      // set screen texture
      SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);

      float fOffX=0.5f/(float)pTex->m_Width, fOffY=0.5f/(float)pTex->m_Height;
      // setup screen aligned quad
      struct_VERTEX_FORMAT_P3F_TEX2F pFlashBangScreen[] =
      {
        Vec3(0, 0, 0), fOffX,   fOffY,
        Vec3(0, 1, 0), fOffX,   1+fOffY,
        Vec3(1, 0, 0), 1+fOffX, fOffY,  
        Vec3(1, 1, 0), 1+fOffX, 1+fOffY, 
      }; 

      // set screen mode
      gRenDev->DrawTriStrip(&(CVertexBuffer(pFlashBangScreen, VERTEX_FORMAT_P3F_TEX2F)), 4); 

      // render flashbang flash 
      STexPic *pFlashBangFlash = gRenDev->m_TexMan->m_Text_FlashBangFlash;

      gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);

      // get flashbang properties
      // get flashbang properties
      float fPosX, fPosY, fSizeX, fSizeY;
      fPosX=m_pVars->m_fFlashBangFlashPosX;  
      fPosY=m_pVars->m_fFlashBangFlashPosY;  

      fSizeX=m_pVars->m_fFlashBangFlashSizeX;
      fSizeY=m_pVars->m_fFlashBangFlashSizeY;

      gRenDev->Draw2dImage((fPosX-fSizeX*0.5f), (fPosY-fSizeY*0.5f), fSizeX, fSizeY, pFlashBangFlash->GetTextureID()); 
      //gRenDev->EnableBlend(false);
      gRenDev->SetState(GS_NODEPTHTEST);
      
      // blur texture      
      pRenderer->EF_RestoreRenderTarget();
      gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
      BlurTextureHw(m_pVars, pTex, 0, 2);
    }

    // render flashbang
    gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);

    // set vertex program
    vpFlashBang->mfSet(true, 0);
    // set fragment program
    fpFlashBang->mfSet(true, 0);

    // set texture coordinates scale (needed for rectangular textures in gl ..)    
    vpFlashBang->mfParameter4f("vTexCoordScale", pfScale); 

    // set fragment vars
    float fBrightness=1.0f;  
    fBrightness=InterpolateCubic(0.0f, .0f, 1.0f, .5f, m_pVars->m_fFlashBangTimeOut);

    float pFlashBang[]= { 0, 0, 0, fBrightness};
    fpFlashBang->mfParameter4f("FlashBang", pFlashBang);

    // set texture states
    SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    SetTexture( pTex, 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);

    // activate blending for motion blur
    //gRenDev->EnableBlend();
    //if(m_pVars->m_fFlashBangTimeOut==1.0f)
    //{
    //  gRenDev->EnableBlend(0);    
    //}    
    //gRenDev->SetBlendMode();  

    // still some issues when using motion blur on this one..
    //gRenDev->EnableBlend(0);
    gRenDev->SetState(GS_NODEPTHTEST);

    // check if motion blur is not active, else we must update screen texture/or get latest screen texture
    if(m_pVars->m_bCartoonActive || m_pVars->m_bBlurActive)
    {
      pTexSurf=GetTextureSurface(pScreenTex);

      // set current rendertarget
      pRenderer->EF_SetRenderTarget(pTexSurf, 1);
      SAFE_RELEASE(pTexSurf)
      gRenDev->SetViewport(0, 0, iWidth, iHeight);      
    }
    else
    {
      // restore backbuffer
      pRenderer->EF_RestoreRenderTarget();      
      gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
    }

    // just render.. 
    gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)), 4);

    // check if motion blur is not active, else we must update screen texture/or get latest screen texture
    if(m_pVars->m_bCartoonActive || m_pVars->m_bBlurActive) 
    {
      // restore backbuffer
      pRenderer->EF_RestoreRenderTarget();
    }

    // restore stuff    
    gRenDev->SetState(GS_NODEPTHTEST);
    vpFlashBang->mfSet(false, 0); 
    fpFlashBang->mfSet(false, 0);

    // sincronize    
    float fTimeScale=1.0f;

    if(m_pVars->m_fFlashBangTimeScale)
    {
      fTimeScale=1.0f/m_pVars->m_fFlashBangTimeScale;
    }

    float fFrameTime=pTimer->GetFrameTime();
    if(fFrameTime<0.002f)
    {
      fFrameTime=0.002f;
    }
    else
    if(fFrameTime>0.5f)
    {
      fFrameTime=0.5f;
    }

    m_pVars->m_fFlashBangTimeOut-=fTimeScale*fFrameTime;

    // reset animation
    if(m_pVars->m_fFlashBangTimeOut<=0.01f) 
    {
      m_pVars->m_bFlashBangActive=0;
      m_pVars->m_fFlashBangTimeOut=1.0f;
    }
    gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);

    LOG_EFFECT("*** End FlashBang... ***\n");
  }

  // ==============================================================================================
  // Blur - simple screen blur

  if(m_pVars->m_bBlurActive)
  {
    LOG_EFFECT("*** Begin blury screen process... ***\n")     

    STexPic *pScreenBluredTex = gRenDev->m_TexMan->m_Text_ScreenLowMap;  

    // get vertex/fragment program    
    CCGVProgram_D3D *vpBluryMap=(CCGVProgram_D3D *) m_pVars->m_pVPBluryScreen;  
    CCGPShader_D3D *fpBluryMap=(CCGPShader_D3D *) m_pVars->m_pRCBluryScreen;

    // get texture surface  
    pTexSurf=GetTextureSurface(pScreenBluredTex);

    // set current rendertarget  
    pRenderer->EF_SetRenderTarget(pTexSurf, 1);
    SAFE_RELEASE(pTexSurf)

    gRenDev->SetViewport(0, 0, pScreenBluredTex->m_Width, pScreenBluredTex->m_Height);

    // setup texture stages/states
    gRenDev->SetMaterialColor(1,1,1, 1);   
    gRenDev->SetState(GS_NODEPTHTEST);

    // setup texture stages/states
    SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);

    float fTexelWidth=0.5f/(float)pScreenBluredTex->m_Width, fTexelHeight=0.5f/(float)pScreenBluredTex->m_Height;
    // setup screen aligned quad
    struct_VERTEX_FORMAT_P3F_TEX2F pScreenBluredQuad[] =
    {
      Vec3(0, 0, 0), fTexelWidth, fTexelHeight,
      Vec3(0, 1, 0), fTexelWidth, 1+fTexelHeight,
      Vec3(1, 0, 0), 1+fTexelWidth, fTexelHeight,
      Vec3(1, 1, 0), 1+fTexelWidth, 1+fTexelHeight, 
    };

    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenBluredQuad,VERTEX_FORMAT_P3F_TEX2F)),4); 

    // blur texture    
    pRenderer->EF_RestoreRenderTarget();
    gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
    BlurTextureHw(m_pVars, pScreenBluredTex, 0, 3);        

    // check if cartoon mode active
    if(m_pVars->m_bCartoonActive)
    {
      pTexSurf=GetTextureSurface(pScreenTex);

      // set current rendertarget
      pRenderer->EF_SetRenderTarget(pTexSurf, 1);
      SAFE_RELEASE(pTexSurf)
      gRenDev->SetViewport(0, 0, iWidth, iHeight);      
    }
    else
    {
      // restore backbuffer
      pRenderer->EF_RestoreRenderTarget(pBackSurface);      
      gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
    }
      
    // set current vertex/fragment program
    vpBluryMap->mfSet(true, NULL);
    fpBluryMap->mfSet(true, NULL);

    // set constants
    float pBluryParams[]= { m_pVars->m_pBlurColor.r, m_pVars->m_pBlurColor.g, m_pVars->m_pBlurColor.b, m_pVars->m_fBlurAmount };
    fpBluryMap->mfParameter4f("fBluryParams", pBluryParams);

    // set texture coordinates scale (needed for rectangular textures in gl ..)
    vpBluryMap->mfParameter4f("vTexCoordScale", pfScale);

    // setup texture stages/states
    SetTexture( pScreenBluredTex, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 1);
    SetTexture( pScreenTex, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1);

    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)),4);

    // check if motion blur is not active, else we must update screen texture/or get latest screen texture
    if(m_pVars->m_bCartoonActive) 
    {
      // restore backbuffer
      pRenderer->EF_RestoreRenderTarget(pBackSurface);
    }

    gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);

    // restore states
    vpBluryMap->mfSet(false, NULL);
    fpBluryMap->mfSet(false, NULL);

    // set flags      
    LOG_EFFECT("*** End blury screen process... ***\n")
  } 
  
  // ==================================================================
  // Cartoon Fx - cartoon rendering mode

  if(m_pVars->m_bCartoonActive)
  {    
    LOG_EFFECT("*** Begin cartoon mode process... ***\n")

    CCGVProgram_D3D *vpCartoon=(CCGVProgram_D3D *) m_pVars->m_pVPCartoon;
    CCGPShader_D3D *fpCartoon=(CCGPShader_D3D *) m_pVars->m_pRCCartoon;
    CCGPShader_D3D *fpCartoonSilhouete=(CCGPShader_D3D *) m_pVars->m_pRCCartoonSilhouette;

    gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight); 

    // set current vertex/fragment program
    vpCartoon->mfSet(true, NULL); 
    fpCartoon->mfSet(true, NULL);

    // set vertex program consts      
    // setup texture offsets, for texture neighboors sampling
    float s1=TEXEL_TO_SCREEN(pScreenTex->m_Width);      
    float t1=TEXEL_TO_SCREEN(pScreenTex->m_Height);  

    float pfNoOffset[]={ s1, t1, 0, 0 }; 
    // set texture coordinates scale (needed for rectangular textures in gl ..)        
    vpCartoon->mfParameter4f("vTexCoordScale", pfScale);     
    vpCartoon->mfParameter4f("Offset0", pfNoOffset);
    vpCartoon->mfParameter4f("Offset1", pfNoOffset);
    vpCartoon->mfParameter4f("Offset2", pfNoOffset);
    vpCartoon->mfParameter4f("Offset3", pfNoOffset);

    // render quad
    SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1); 

    // just render
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB, VERTEX_FORMAT_P3F_TEX2F)),4);     

    vpCartoon->mfSet(false, NULL);
    fpCartoon->mfSet(false, NULL); 

    // update screen texture
    CopyScreenToTexture(pScreenRect, pScreenTex);

    // set current vertex/fragment program 
    vpCartoon->mfSet(true, NULL);
    fpCartoonSilhouete->mfSet(true, NULL);

    s1=1.0f/(float)pScreenTex->m_Width;      
    t1=1.0f/(float)pScreenTex->m_Height;  
    float s_off=s1*0.5f;
    float t_off=t1*0.5f;

    float pfOffset0[]={   s_off,   -t1+t_off, 0.0f, 0.0f}; // up
    float pfOffset1[]={   s_off,   t1+t_off, 0.0f, 0.0f};  // down
    float pfOffset2[]={   -s1+s_off,  t_off, 0.0f, 0.0f};  // left 
    float pfOffset3[]={   s1+s_off,   t_off, 0.0f, 0.0f};  // right

    // set vertex program consts      
    vpCartoon->mfParameter4f("Offset0",pfOffset0);
    vpCartoon->mfParameter4f("Offset1",pfOffset1);
    vpCartoon->mfParameter4f("Offset2",pfOffset2);
    vpCartoon->mfParameter4f("Offset3",pfOffset3);

    // render quad
    SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1); 
    SetTexture( pScreenTex, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    SetTexture( pScreenTex, 2, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    SetTexture( pScreenTex, 3, D3DTEXF_POINT, D3DTEXF_POINT, 1);

    // just render
    gRenDev->SetState(GS_BLSRC_ZERO | GS_BLDST_SRCCOL | GS_NODEPTHTEST);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB, VERTEX_FORMAT_P3F_TEX2F)),4); 

    vpCartoon->mfSet(false, NULL);
    fpCartoonSilhouete->mfSet(false, NULL);

    LOG_EFFECT("*** End cartoon mode process... ***\n")    
  }
  
  // ==============================================================================================
  // Motion Blur - simple motion blur, also radial blur
  // ideas: 
  //  .adding some translation/rotation into radial blur, would look interesting
  //  .lerping between radial blur and normal screen  

  if(m_pVars->m_bMotionBlurActive)
  {    
    LOG_EFFECT("*** Begin motion blur... ***\n")

    // get vertex/fragment program
    CCGVProgram_D3D *vpMotion=(CCGVProgram_D3D *) m_pVars->m_pVPMotion;
    CCGPShader_D3D *fpMotion=(CCGPShader_D3D *) m_pVars->m_pRCMotion;

    // create texture if necessary 
    STexPic *pTex = gRenDev->m_TexMan->m_Text_PrevScreenMap;  
    gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight); 

    // setup renderstate        
    gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);   

    switch(m_pVars->m_iMotionBlurType) 
    {
      // simple motion blur
    case 1:
      {
        float fAmount=CLAMP(m_pVars->m_fMotionBlurAmount*(75.0f*pTimer->GetFrameTime()), 0.0f, 1.0f);

        // setup texture stages/states
        gRenDev->SetMaterialColor(1,1,1, 1-fAmount);
        SetTexture( pTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);             
        gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)),4);            
      } 
      break; 

      // radial blur
    case 2:
      float fAmount=CLAMP(m_pVars->m_fMotionBlurAmount, 0.0f, 1.0f);

      // set current vertex/fragment program
      vpMotion->mfSet(true, NULL);
      fpMotion->mfSet(true, NULL);

      // setup texture stages/states
      SetTexture( pTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
      SetTexture( pTex, 1, D3DTEXF_POINT, D3DTEXF_POINT, 1);
      SetTexture( pTex, 2, D3DTEXF_POINT, D3DTEXF_POINT, 1);
      SetTexture( pTex, 3, D3DTEXF_POINT, D3DTEXF_POINT, 1);

      // setup vertex program
      float pfTexSize[]= { 1, 1, 0, 0 }; 
      vpMotion->mfParameter4f("TexSize", pfTexSize);

      // setup fragment program

      // set constants
      float pMotionParams[]= { 1, 1, 1, 1-fAmount };
      fpMotion->mfParameter4f("fMotionParams", pMotionParams);

      float fScaleInc= ((float) m_pVars->m_iMotionBlurDisplace)/(float)iWidth; 
      float pfTexScale[4];

      // set texture scale
      pfTexScale[0]=1.0f;//0.99f - (fScaleInc+=fScaleInc);
      pfTexScale[1]=0.99f - (fScaleInc+=fScaleInc*2.0f);
      pfTexScale[2]=0.99f - (fScaleInc+=fScaleInc*0.5f);
      pfTexScale[3]=0.99f - (fScaleInc+=fScaleInc*0.25f);
      vpMotion->mfParameter4f("TexScale", pfTexScale);

      // just render
      gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)),4);

      // disable current vertex/fragment program
      vpMotion->mfSet(false, NULL);
      fpMotion->mfSet(false, NULL);
      break;
    }

    // copy into 'previous framebuffer' texture
    CopyScreenToTexture(pScreenRect, pTex);

    LOG_EFFECT("*** End motion blur process... ***\n")
  }

  // ===================================================
  // Fade Fx - simple fade between screen and some color

  if(m_pVars->m_bFadeActive)    
  {    
    LOG_EFFECT("*** Begin screen fade process... ***\n")

    // sincronize
    float fStep, fSign=(m_pVars->m_fFadeTime<0.0f)? -1.0f: 1.0f;

    // interpolate values
    if(m_pVars->m_fFadeCurrPreTime>=m_pVars->m_fFadePreTime)
    {
      m_pVars->m_fFadeCurrTime-= pTimer->GetFrameTime();
      // fade its only disabled in 'fade in' case, in fade out case, user should disable it
      if(m_pVars->m_fFadeCurrTime<0.0f)
      {       
        m_pVars->m_fFadeCurrTime=0.0f;
        m_pVars->m_fFadePreTime=0.0f;
        m_pVars->m_bFadeActive=0;
        m_pVars->m_fFadeCurrPreTime=0.0f;                   
      }
    }

    fStep=m_pVars->m_fFadeCurrTime/fabsf(m_pVars->m_fFadeTime); 

    if(fSign>0.0f) 
    {
      fStep=1.0f-fStep;       
    }

    // count 'pre-fade' frame number
    m_pVars->m_fFadeCurrPreTime+=1.0f; 
    m_pVars->m_pFadeCurrColor.set(m_pVars->m_pFadeColor.r, m_pVars->m_pFadeColor.g, m_pVars->m_pFadeColor.b, fStep);        

    // copy fade amount    
    ICVar *pHudFadeAmount=iConsole->GetCVar("hud_fadeamount");
    if(pHudFadeAmount)
    {
      pHudFadeAmount->Set(1-fStep);
    }

    // set render states
    gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
    gRenDev->SetMaterialColor(m_pVars->m_pFadeCurrColor.r, m_pVars->m_pFadeCurrColor.g, m_pVars->m_pFadeCurrColor.b, m_pVars->m_pFadeCurrColor.a);
    STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;      
    SetTexture( pWhiteTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)),4);    

    LOG_EFFECT("*** End screen fade process... ***\n")
  }

  // debug screen effects textures
  if(CRenderer::CV_r_debugscreenfx) 
  {
    gRenDev->SetState(GS_NODEPTHTEST);
    gRenDev->SetViewport(10, 50, 100, 100);         
    SetTexture( pScreenTex, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)),4);    

    gRenDev->SetViewport(120, 50, 100, 100);   
    SetTexture( gRenDev->m_TexMan->m_Text_PrevScreenMap, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)),4);    

    gRenDev->SetViewport(230, 50, 100, 100);   
    SetTexture( gRenDev->m_TexMan->m_Text_FlashBangMap, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)),4);    

    gRenDev->SetViewport(340, 50, 100, 100);   
    SetTexture( gRenDev->m_TexMan->m_Text_ScreenLowMap, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)),4);    

    gRenDev->SetViewport(450, 50, 100, 100);   
    SetTexture( gRenDev->m_TexMan->m_Text_Glare, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)),4);    

    gRenDev->SetViewport(560, 50, 100, 100);   
    SetTexture( gRenDev->m_TexMan->m_Text_ScreenAvg1x1, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)),4);    

    gRenDev->SetViewport(670, 50, 100, 100);     
    SetTexture( gRenDev->m_TexMan->m_Text_ScreenCurrLuminosityMap, 0, D3DTEXF_POINT, D3DTEXF_POINT, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pDefaultScreenVB,VERTEX_FORMAT_P3F_TEX2F)),4);            
  }

  // restore stuff
  gRenDev->Set2DMode(false, 1, 1); 
  gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight); 
  gRenDev->ResetToDefault();

  pTimer->MeasureTime("ScreenRender Up");
  return 1;
}
