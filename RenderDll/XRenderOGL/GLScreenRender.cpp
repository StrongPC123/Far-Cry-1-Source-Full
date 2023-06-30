/*
=======================================================================
FILE : GLScreenRender.cpp
DESC : screen renderer
PROJ : Crytek Engine
CODER: Tiago Sousa

Last Update: 18/11/2003

Todo: 
- Clean up code
- Remove redundant data/tests
=======================================================================
*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "I3dengine.h"

// tiago: added
#include "GLCGPShader.h"
#include "GLCGVProgram.h"
#include "../Common/RendElements/CREScreenCommon.h"

// check shader presence
#define CHECK_SHADER(s, msg)\
  if(!(s))\
  {\
    if (gRenDev->m_LogFile)\
    {\
      gRenDev->Logv(SRendItem::m_RecurseLevel, "ERROR! %s program not present !\n", msg);\
    }\
    return 0;\
  }\

// log helper
#define LOG_EFFECT(msg)\
  if (gRenDev->m_LogFile)\
  {\
  gRenDev->Logv(SRendItem::m_RecurseLevel, msg);\
  }\

// ====================================================================
// compute scaled texture coordinates for texel-to-pixel correct output

#define TEXEL_TO_SCREEN(size) \
  ((size)==0)? 0.0f: 0.5f/((float)(size))  

// ===============================================================
// SetTexture - sets texture stage

inline void SetTexture(CGLRenderer *pRenderer, STexPic *pTex, int iStage, int iMinFilter, int iMagFilter, bool bClamp)
{
  pRenderer->EF_SelectTMU(iStage);
  if(pTex)
  {
    pTex->m_RefTex.m_MinFilter=iMinFilter; 
    pTex->m_RefTex.m_MagFilter=iMagFilter;
    pTex->m_RefTex.bRepeats=(!bClamp);  
    pTex->Set(); 
    
    GLenum iTextureMode = pTex->m_TargetType;
    glTexParameteri(iTextureMode, GL_TEXTURE_MIN_FILTER, iMinFilter);
    glTexParameteri(iTextureMode, GL_TEXTURE_MAG_FILTER, iMagFilter);

    if(!pTex->m_RefTex.bRepeats)
    {
      glTexParameteri(iTextureMode, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(iTextureMode, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);      
    }
  }
  else
  {
    pRenderer->SetTexture(0);
  }
}

// =================================================================================
// CopyScreenToTexture - copy screen into texture (note: this assumes texture is screen sized)

inline void CopyScreenToTexture(CGLRenderer *pRenderer, STexPic *pTex)
{
  // copy to previous frame buffer texture
  SetTexture(pRenderer, pTex, 0, GL_NEAREST, GL_NEAREST, 1); 

  glCopyTexSubImage2D(pTex->m_TargetType, 0, 0, 0, 0, 0, pTex->m_Width, pTex->m_Height);

  /*{
      byte *pDst = new byte [pTex->m_Width*pTex->m_Height*4];
      glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

      WriteJPG(pDst, pTex->m_Width, pTex->m_Height, "Screen.jpg");
      delete [] pDst;
  }*/
}

// ===============================================================
// CreateRenderTarget - texture targets creation helper
// Last Update: 28/05/2003

bool CreateRenderTarget(CGLRenderer *pRenderer, STexPic *&pTex, int iWidth, int iHeight, bool bUseAlpha, bool bLockable)
{
  // check if parameters are valid
  if(!pRenderer || !pTex || !iWidth || !iHeight)
  {
    return 0;
  }

  // recreate texture when necessary
  if(pTex->m_Width!=iWidth || pTex->m_Height!=iHeight)
  {
    pTex->m_Flags&= ~FT_ALLOCATED;
  }
  
  GLenum srcFormat=(bUseAlpha)?GL_RGBA : GL_RGB;
  GLenum dstFormat=(bUseAlpha)?GL_RGBA8 : GL_RGB8;

  // if not created yet, create texture
  if (!(pTex->m_Flags & FT_ALLOCATED))
  {
    // set texture flags
    pTex->m_Flags |= FT_ALLOCATED;
    pTex->m_Flags2 |= FT2_RECTANGLE;
    pTex->m_TargetType = GL_TEXTURE_RECTANGLE_NV;

    pTex->m_Width = iWidth;
    pTex->m_Height = iHeight;    
    pRenderer->m_TexMan->SetTexture(pTex->m_Bind, eTT_Rectangle);
    glTexImage2D(pTex->m_TargetType, 0, dstFormat, iWidth, iHeight, 0, srcFormat, GL_UNSIGNED_BYTE, 0);
    return 1;
  }

  return 0;
}

// ===============================================================
// BlurTextureHw - blur texture in hardware
// Last Update: 30/07/2003
// Todo: add diferent blur types/optimize also
// Notes: only rectangular texture supported

bool BlurTextureHw(CScreenVars *pVars, CGLRenderer *pRenderer, STexPic *&pTex, int iBlurType, int iBlurAmount, bool bDefineRT, bool bRestoreRT)
{
  // make sure data ok
  if(!pRenderer || !pTex)
  {
    return 0;
  }

  LOG_EFFECT("*** Begin texture bluring process... ***\n") 

  // get vertex/fragment program  
  CCGVProgram_GL *vpBlur=(CCGVProgram_GL *) pVars->m_pVPBlur; 
  CCGPShader_GL *fpBlur;
  if (pTex->m_TargetType == GL_TEXTURE_RECTANGLE_NV)
    fpBlur=(CCGPShader_GL *) pVars->m_pRCBlurRECT;
  else
    fpBlur=(CCGPShader_GL *) pVars->m_pRCBlur;
  CHECK_SHADER(vpBlur, "Blur vertex") 
  CHECK_SHADER(fpBlur, "Blur fragment")

  // get current viewport
  int iTmpX, iTmpY, iTempWidth, iTempHeight;
  gRenDev->GetViewport(&iTmpX, &iTmpY, &iTempWidth, &iTempHeight);   

  // resize screen to fit texture 
  gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height ); 

  // blur texture

  // setup texture offsets, for texture neighboors sampling
  float s1=1.0f;///((float)pTex->m_Width);        
  float t1=1.0f;///((float)pTex->m_Height);          
  float s_off=s1*0.5f; 
  float t_off=t1*0.5f;    

  float pfOffset0[]={ s1*0.5f,       t1, 0.0f, 0.0f};    
  float pfOffset1[]={  -s1,  t1*0.5f, 0.0f, 0.0f}; 
  float pfOffset2[]={-s1*0.5f,      -t1, 0.0f, 0.0f}; 
  float pfOffset3[]={      s1, -t1*0.5f, 0.0f, 0.0f}; 
   
  // render quad
  SetTexture(pRenderer, pTex, 0, GL_LINEAR, GL_LINEAR, 1);
  SetTexture(pRenderer, pTex, 1, GL_LINEAR, GL_LINEAR, 1);
  SetTexture(pRenderer, pTex, 2, GL_LINEAR, GL_LINEAR, 1);
  SetTexture(pRenderer, pTex, 3, GL_LINEAR, GL_LINEAR, 1);

  // set current vertex/fragment program
  vpBlur->mfSet(true, 0); 
  fpBlur->mfSet(true, 0);
  pRenderer->EF_CommitVS();
  pRenderer->EF_CommitPS();

  // set vertex program consts
 
  // set texture coordinates, displacement offsets
  vpBlur->mfParameter4f("Offset0", pfOffset0);
  vpBlur->mfParameter4f("Offset1", pfOffset1);
  vpBlur->mfParameter4f("Offset2", pfOffset2); 
  vpBlur->mfParameter4f("Offset3", pfOffset3);


  // setup screen aligned quad 
  struct_VERTEX_FORMAT_P3F_TEX2F pScreenBlur[] =  
  {
    Vec3(1, 1, 0), (float)pTex->m_Width+s_off, 0,        
    Vec3(1, 0, 0), (float)pTex->m_Width+s_off, (float)pTex->m_Height+t_off,         
    Vec3(0, 1, 0),      0, 0,       
    Vec3(0, 0, 0),      0, (float)pTex->m_Height+t_off,             
  }; 

  // blur texture  
  if(iBlurType==0)  // simple box blur
  {
    for(int iBlurPasses=0; iBlurPasses< iBlurAmount;  iBlurPasses++) 
    {
      // set texture coordinates scale (needed for rectangular textures in gl ..)
      float pfScale[]={ 1, 1, 1.0f, 1.0f } ;  //iBlurPasses};   
      vpBlur->mfParameter4f("vTexCoordScale", pfScale);

      // render screen aligned quad...
      gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);             
      
      CopyScreenToTexture(pRenderer, pTex);
    } 
  }

  vpBlur->mfSet(false, 0);  
  fpBlur->mfSet(false, 0);

  pRenderer->EF_SelectTMU(1);
  pRenderer->EnableTMU(false);
  pRenderer->EF_SelectTMU(2);
  pRenderer->EnableTMU(false);
  pRenderer->EF_SelectTMU(3);
  pRenderer->EnableTMU(false);
  pRenderer->EF_SelectTMU(0);

  // restore previous viewport  
  gRenDev->SetViewport( 0, 0, iTempWidth, iTempHeight); 

  LOG_EFFECT("*** End texture bluring process... ***\n")
  return 1; 
}

// =========================================================================
// ResizeTextureHw - resize a texture in hardware, using bilinear resampling
bool ResizeTextureHw(CScreenVars *pVars, CGLRenderer *pRenderer, STexPic *&pSrc, STexPic *&pDst)
{
  // make sure data ok
  if(!pRenderer || !pSrc || !pDst || !pVars)
  {
    return 0;
  }

  LOG_EFFECT("*** Begin texture resampling process... ***\n")

  // get vertex/fragment program  
  CCGVProgram_GL *vpBlur=(CCGVProgram_GL *) pVars->m_pVPBlur;
  CCGPShader_GL *fpBlur;
  if (pSrc->m_TargetType == GL_TEXTURE_RECTANGLE_NV)
    fpBlur=(CCGPShader_GL *) pVars->m_pRCBlurRECT;
  else
    fpBlur=(CCGPShader_GL *) pVars->m_pRCBlur;

  gRenDev->SetViewport( 0, 0, pDst->m_Width, pDst->m_Height );
  
  // set current vertex/fragment program    
  vpBlur->mfSet(true, 0);
  fpBlur->mfSet(true, 0);
  pRenderer->EF_CommitVS();
  pRenderer->EF_CommitPS();

  // setup texture offsets, for texture neighboors sampling
  float s1=1.0f;     
  float t1=1.0f;   
  float s_off=s1*0.5f; 
  float t_off=t1*0.5f; 

  float pfOffset0[]={ s1*0.5f,       t1, 0.0f, 0.0f};    
  float pfOffset1[]={  -s1,  t1*0.5f, 0.0f, 0.0f}; 
  float pfOffset2[]={-s1*0.5f,      -t1, 0.0f, 0.0f}; 
  float pfOffset3[]={      s1, -t1*0.5f, 0.0f, 0.0f}; 

  // render quad
  SetTexture(pRenderer, pSrc, 0, GL_LINEAR, GL_LINEAR, 1); 
  SetTexture(pRenderer, pSrc, 1, GL_LINEAR, GL_LINEAR, 1);
  SetTexture(pRenderer, pSrc, 2, GL_LINEAR, GL_LINEAR, 1);
  SetTexture(pRenderer, pSrc, 3, GL_LINEAR, GL_LINEAR, 1);

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
    Vec3(1, 1, 0), (float)pSrc->m_Width, 0,        
    Vec3(1, 0, 0), (float)pSrc->m_Width, (float)pSrc->m_Height,          
    Vec3(0, 1, 0),  0, 0,       
    Vec3(0, 0, 0),  0, (float)pSrc->m_Height,             
  }; 

  // resample texture
  // set texture coordinates scale (needed for rectangular textures in gl ..)
  float pfScale[]={ 1, 1, 1.0f, 0};     
  vpBlur->mfParameter4f("vTexCoordScale", pfScale);
  
  // render screen aligned quad...
  gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_TEX2F)), 4);  

  CopyScreenToTexture(pRenderer, pDst);

  vpBlur->mfSet(false, 0);
  fpBlur->mfSet(false, 0);

  pRenderer->EF_SelectTMU(1);
  pRenderer->EnableTMU(false);
  pRenderer->EF_SelectTMU(2);
  pRenderer->EnableTMU(false);
  pRenderer->EF_SelectTMU(3);
  pRenderer->EnableTMU(false);
  pRenderer->EF_SelectTMU(0);

  LOG_EFFECT("*** End texture resampling process... ***\n")
  return 1; 
}

// flashbang helper (remove this, not necessary, use simple lerp)
inline float InterpolateCubic(float fCp1, float fCp2, float fCp3, float fCp4, float fTime) 
{ 
  float fTimeSquare = fTime*fTime; 
  return (((-fCp1 * 2.0f) + (fCp2 * 5.0f) - (fCp3 * 4) + fCp4) / 6.0f) * fTimeSquare * fTime + 
    (fCp1 + fCp3 - (2.0f * fCp2)) * fTimeSquare + (((-4.0f * fCp1) + fCp2 + (fCp3 * 4.0f) - fCp4) / 6.0f) * fTime + fCp2; 
}

// =============================================================
// Render screen post-processing effects for low-spec machines..
// Tiago - Do not alter anything without consulting me
// Last Update: 04/12/2003

bool CREScreenProcess:: mfDrawLowSpec(SShader *ef, SShaderPass *sfm)
{  
  // MUST RESET TO DEFAULT
  gRenDev->ResetToDefault(); 

  // get data
  ITimer *pTimer=iSystem->GetITimer();
  CGLRenderer *pRenderer = gcpOGL;
  int iTempX, iTempY, iWidth, iHeight;
  gRenDev->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

  // setup shared renderstate
  gRenDev->Set2DMode(true, 1, 1);
  gRenDev->SetState(GS_NODEPTHTEST);

  // add texel displacement adjustment
  float fSoff=0.5f/(float)iWidth;
  float fToff=0.5f/(float)iHeight;

  // setup shared screen aligned quad
  struct_VERTEX_FORMAT_P3F_TEX2F pScreenQuad[] =
  {
    Vec3(0, 0, 0),    fSoff, fToff,
    Vec3(0, 1, 0),    fSoff, 1+fToff,
    Vec3(1, 0, 0), 1+ fSoff, fToff,
    Vec3(1, 1, 0), 1+ fSoff, 1+fToff, 
  };

  // Cryvision
  if(m_pVars->m_iNightVisionActive)
  {    
    // some bug in  set material color, lets pass color trough vertices instead ...
    gRenDev->SetMaterialColor(0.4f, 0.5f, 0.6f, 1.0f); 

    static float fNoise=0;    
    fNoise+=(pTimer->GetFrameTime())*20.0f;
    if(fNoise>2.0f)
    {
      fNoise=0.0f;
    }

    // setup shared screen aligned quad
    struct_VERTEX_FORMAT_P3F_TEX2F pScrQuad[] =
    {
      Vec3(0, 0, 0), 0, fNoise,
      Vec3(0, 1, 0), 0, 2+fNoise, 
      Vec3(1, 0, 0), 2, 0+fNoise,
      Vec3(1, 1, 0), 2, 2+fNoise, 
    };

    // bright color a bit ..
    gRenDev->SetMaterialColor(0.0f, 0.2f, 0.3f, 1.0f);     
    STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;
    SetTexture(pRenderer, pWhiteTex, 0, GL_LINEAR, GL_LINEAR, 0); 

    gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);    
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScrQuad,VERTEX_FORMAT_P3F_TEX2F)),4);  

    // set color..    
    STexPic *pScreenNoiseTex = gRenDev->m_TexMan->m_Text_ScreenNoise;
    SetTexture(pRenderer, pScreenNoiseTex, 0, GL_LINEAR, GL_LINEAR, 0); 

    gRenDev->SetMaterialColor(0.4f, 0.6f, 0.8f, 1.0f);
    gRenDev->SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_NODEPTHTEST);     
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScrQuad,VERTEX_FORMAT_P3F_TEX2F)),4);

    gRenDev->Set2DMode(false, 1, 1); 
    gRenDev->ResetToDefault();    
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
    SetTexture(pRenderer, pWhiteTex, 0, GL_NEAREST, GL_NEAREST, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)), 4);

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

  // ===================================================
  // Fade Fx - simple fade between screen and some color

  if(m_pVars->m_bFadeActive)
  {
    gRenDev->ResetToDefault();
    LOG_EFFECT("*** Begin screen fade process... ***\n")
            
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
    CRenderer::CV_r_fadeamount=1-fStep;
    if(!m_pVars->m_bFadeActive)
    {
      CRenderer::CV_r_fadeamount=1.0f;
    }

    // set render states
    gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

    // some bug in  set material color, lets pass color trough vertices instead ...
    gRenDev->SetMaterialColor(m_pVars->m_pFadeCurrColor.r, m_pVars->m_pFadeCurrColor.g, m_pVars->m_pFadeCurrColor.b, m_pVars->m_pFadeCurrColor.a);

    STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;      
    SetTexture(pRenderer, pWhiteTex, 0, GL_LINEAR, GL_LINEAR, 1);

    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)),4);    
    LOG_EFFECT("*** End screen fade process... ***\n")
  }

  gRenDev->Set2DMode(false, 1, 1); 
  gRenDev->ResetToDefault();

  return 1;
}

// =============================================================
// Returns closest power of 2 texture size
inline int GetClosestPow2Size(int size)
{    
  // clamp maximum texture size to 512
  if(size>=512)
  {
    return 512;
  }  
  if(size>=256)
  {
    return 256;
  }
  if(size>=128)
  {
    return 128;
  }
  if(size>=64)
  {
    return 64;
  }
  if(size>=32)
  {
    return 32;
  }
  if(size>=16)
  {
    return 16;
  }
  if(size>=8)
  {
    return 8;
  }

  return size;
}

// =============================================================
// Render screen post-processing effects
// Tiago - Do not alter anything without consulting me
// Last Update: 17/09/2003

bool CREScreenProcess::mfDraw(SShader *ef, SShaderPass *sfm)
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

  // get renderer
  CGLRenderer *pRenderer = gcpOGL;

  // no support for rectangular textures and pixel shaders, just process low-spec screen effects
  if ((!SUPPORTS_GL_NV_texture_rectangle && !SUPPORTS_GL_EXT_texture_rectangle) || !(pRenderer->GetFeatures() & RFT_HW_TS) || CRenderer::CV_r_disable_sfx)
  {
    mfDrawLowSpec(ef, sfm);
    return 0;
  }

  // make sure every state ok..
  gRenDev->ResetToDefault();
  
  // force no wireframe
  pRenderer->SetPolygonMode(R_SOLID_MODE);  

  ITimer *pTimer=iSystem->GetITimer();

  // get shared screen image
  STexPic *pScreenTex=gRenDev->m_TexMan->m_Text_ScreenMap;
  int iTempX, iTempY, iWidth, iHeight;
  gRenDev->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

  // update screen texture..
  LOG_EFFECT("*** Begin updating screen texture... ***\n")  
  gRenDev->SetViewport(0, 0, iWidth, iHeight);
   
  // create necessary textures at once
  CreateRenderTarget(pRenderer, pScreenTex, iWidth, iHeight, 1, 0);
  CreateRenderTarget(pRenderer, gRenDev->m_TexMan->m_Text_PrevScreenMap, iWidth, iHeight, 0, 0);
  CreateRenderTarget(pRenderer, gRenDev->m_TexMan->m_Text_ScreenLowMap, GetClosestPow2Size(iWidth/4), GetClosestPow2Size(iWidth/4), 0, 0);     
  CreateRenderTarget(pRenderer, gRenDev->m_TexMan->m_Text_Glare, GetClosestPow2Size(iWidth/8), GetClosestPow2Size(iWidth/8), 0, 0);
  CreateRenderTarget(pRenderer, gRenDev->m_TexMan->m_Text_FlashBangMap, GetClosestPow2Size(iWidth/8), GetClosestPow2Size(iWidth/8), 0, 0);
  CreateRenderTarget(pRenderer, gRenDev->m_TexMan->m_Text_ScreenAvg1x1, 2, 2, 0, 0); 
  CreateRenderTarget(pRenderer, gRenDev->m_TexMan->m_Text_ScreenLuminosityMap, 1, 1, 0, 0);   

  // add texel displacement adjustment
  float fSoff=0.5f/(float)iWidth;
  float fToff=0.5f/(float)iHeight;

  // update screen texture..
  CopyScreenToTexture(pRenderer, pScreenTex);      

  LOG_EFFECT("*** End updating screen texture... ***\n") 

  // setup shared screen aligned quad
  struct_VERTEX_FORMAT_P3F_TEX2F pScreenQuad[] =  
  {
    Vec3(1, 1, 0), 1, 0,
    Vec3(1, 0, 0), 1, 1,     
    Vec3(0, 1, 0), 0, 0,     
    Vec3(0, 0, 0), 0, 1,   
  }; 

  // setup shared screen sized aligned quad
  struct_VERTEX_FORMAT_P3F_TEX2F pScreenSizeQuad[] =  
  {
    Vec3(1, 1, 0), (float)pScreenTex->m_Width, 0, 
    Vec3(1, 0, 0), (float)pScreenTex->m_Width, (float)pScreenTex->m_Height,      
    Vec3(0, 1, 0),                          0, 0,     
    Vec3(0, 0, 0),                          0, (float)pScreenTex->m_Height,   
  }; 

  // setup shared renderstate
  //gRenDev->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  gRenDev->SetState(GS_NODEPTHTEST);
  gRenDev->Set2DMode(true, 1, 1);

  // ==============================================================================================
  // any effect requires screen low res image version ?
  if((m_pVars->m_bBlurActive || m_pVars->m_bGlareActive) && !m_pVars->m_bCartoonActive) // && !m_pVars->m_iNightVisionActive)// || m_pVars->m_bNightVisionActive 
  {
    STexPic *pScreenLow = gRenDev->m_TexMan->m_Text_ScreenLowMap,
            *pScreenAvg = gRenDev->m_TexMan->m_Text_ScreenAvg1x1; 
    // update screen texture..
    LOG_EFFECT("*** Begin updating screen mip maps textures... ***\n")                  

    // resize textures using bilinear resampling
    ResizeTextureHw(m_pVars, pRenderer, pScreenTex, pScreenLow);

    // only glare requires all mip maps levels
    if(m_pVars->m_bGlareActive && !m_pVars->m_bBlurActive && !m_pVars->m_iNightVisionActive)  
    {
      ResizeTextureHw(m_pVars, pRenderer, pScreenLow, pScreenAvg);
    }
    LOG_EFFECT("*** End updating screen mip maps textures... ***\n")                  
  }

  // ==============================================================================================
  // Glare/Nightvision - Glare fx and Nightvision w/ glare
  // TODO: add multiple glare type filters

  if((m_pVars->m_iNightVisionActive && m_pVars->m_bCartoonActive) || (m_pVars->m_bGlareActive && !m_pVars->m_bCartoonActive) || m_pVars->m_iNightVisionActive)  
  {
    LOG_EFFECT("*** Begin glare... ***\n"); 

    pRenderer->EF_SelectTMU(1);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(2);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(3);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(0);

    // compute screen luminosity if necessary 
    if(!m_pVars->m_iNightVisionActive)
    {      
      // get data    
      CREGlare  *pRE = gRenDev->m_RP.m_pREGlare;
      STexPic   *pTex = gRenDev->m_TexMan->m_Text_Glare,
                *pScreenLuminosityTex = gRenDev->m_TexMan->m_Text_ScreenLuminosityMap,
                *pScreenLow = gRenDev->m_TexMan->m_Text_ScreenLowMap,
                *pScreenAvg = gRenDev->m_TexMan->m_Text_ScreenAvg1x1;

      // get vertex/fragment programs    
      CCGPShader_GL *fpGlareAmount=(CCGPShader_GL *) m_pVars->m_pRCGlareAmountMap;
      CCGPShader_GL *fpGlareMap=(CCGPShader_GL *) m_pVars->m_pRCGlareMap;
      CHECK_SHADER(fpGlareAmount, "GlareAmount vertex")
      CHECK_SHADER(fpGlareMap, "GlareMap fragment")

      CCGVProgram_GL *vpGlare=(CCGVProgram_GL *) m_pVars->m_pVPGlare;
      CCGPShader_GL *fpGlare=(CCGPShader_GL *) m_pVars->m_pRCGlare;
      CCGPShader_GL *fpRenderModeCold=(CCGPShader_GL *) m_pVars->m_pRCRenderModeCold;
      CHECK_SHADER(vpGlare, "Glare vertex")
      CHECK_SHADER(fpGlare, "Glare fragment")
      CHECK_SHADER(fpRenderModeCold, "RenderModeCold fragment")

      gRenDev->SetMaterialColor(1,1,1,1);
      gRenDev->SetState(GS_NODEPTHTEST);
      gRenDev->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

      // get average screen luminosity
      gRenDev->SetViewport( 0, 0, iWidth, iHeight); 
 
      // clamp...
      int iGlareQuality=CRenderer::CV_r_glarequality;
      if(iGlareQuality>3) 
      {
        iGlareQuality=3;
      } 
      else
      if(iGlareQuality<0)
      {
        iGlareQuality=0;
      }

      // ---------------------------------
      // compute screen luminosity texture 
      // ---------------------------------
      {   
        // set constants      
        float fLastGlareAmount=0;  
        static float fFrameCounter=0;

        // slowdown transition ..
        if(fFrameCounter>=CRenderer::CV_r_glaretransition/(pTimer->GetFrameTime()*125.0f+0.001f))    
        {
          // get average screen luminosity
          gRenDev->SetViewport( 0, 0, pScreenLuminosityTex->m_Width, pScreenLuminosityTex->m_Height); 

          // set pixel/vertex programs
          vpGlare->mfSet(true, 0);
          fpGlareAmount->mfSet(true, 0);
          pRenderer->EF_CommitPS();
          pRenderer->EF_CommitVS();

          // set texture coordinates scale (needed for rectangular textures in gl ..)         
          float pfTexel01[4]= { (float)pScreenAvg->m_Width, (float)pScreenAvg->m_Height, -0.5f, -0.5f};        
          vpGlare->mfParameter4f("vTexCoordScale01", pfTexel01); 
          float pfTexel02[4]= { (float)pScreenAvg->m_Width, (float)pScreenAvg->m_Height, -0.5f, 0.5f};      
          vpGlare->mfParameter4f("vTexCoordScale02", pfTexel02); 
          float pfTexel03[4]= { (float)pScreenAvg->m_Width, (float)pScreenAvg->m_Height, 0.5f, -0.5f};      
          vpGlare->mfParameter4f("vTexCoordScale03", pfTexel03); 
          float pfTexel04[4]= { (float)pScreenAvg->m_Width, (float)pScreenAvg->m_Height, 0.5f, 0.5f};      
          vpGlare->mfParameter4f("vTexCoordScale04", pfTexel04); 

          fLastGlareAmount=0.0025f*pTimer->GetFrameTime()*1000.0f;       
          fFrameCounter=0;
          // set screen texture/state       
          SetTexture(pRenderer, pScreenAvg, 0, GL_NEAREST, GL_NEAREST, 1);  
          SetTexture(pRenderer, pScreenAvg, 1, GL_NEAREST, GL_NEAREST, 1); 
          SetTexture(pRenderer, pScreenAvg, 2, GL_NEAREST, GL_NEAREST, 1); 
          SetTexture(pRenderer, pScreenAvg, 3, GL_NEAREST, GL_NEAREST, 1); 

          float pGlare[]= { 1, 1, 1,  fLastGlareAmount}; 
          fpGlareAmount->mfParameter4f("Glare", pGlare);

          // use motion blur to lerp between brightness values
          gRenDev->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
          gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);

          // copy screen
          CopyScreenToTexture(pRenderer, pScreenLuminosityTex);

          // reset state            
          vpGlare->mfSet(false, 0);
          fpGlareAmount->mfSet(false, 0);   
        }
        else
        {
          // no need for processing, just keep old luminosity texture                        
        } 

        fFrameCounter+=(float) 1.0f;
      }

      // cold render mode ?  then must apply unsharp filter on screen texture
      if(CRenderer::CV_r_rendermode==3)       
      {
        // ---------------------------------------------------
        // First must create blured texture for unsharp filter 
        // Notes: possible optimization, create an low-res 
        // screen always, for sharing with other effects
        // ---------------------------------------------------

        //BlurTextureHw(m_pVars, pRenderer, pScreenLow, 0, 2, 0, 0);   


        // --------------------------------------
        // apply unsharp filter on screen texture
        // --------------------------------------

        // set current rendertarget                      
        gRenDev->SetViewport( 0, 0, pScreenTex->m_Width, pScreenTex->m_Height);

        // set pixel/vertex programs
        vpGlare->mfSet(true, 0);      
        fpRenderModeCold->mfSet(true, 0); 
        pRenderer->EF_CommitPS();
        pRenderer->EF_CommitVS();

        float pfScale01[]={ (float) pScreenTex->m_Width, (float) pScreenTex->m_Height, 0, 0};    
        vpGlare->mfParameter4f("vTexCoordScale01", pfScale01); 
        float pfBluredSize[4]={ (float) pScreenLow->m_Width, (float) pScreenLow->m_Height, 0, 0};    
        vpGlare->mfParameter4f("vTexCoordScale02", pfBluredSize); 

        SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1); 
        SetTexture(pRenderer, pScreenLow, 1, GL_LINEAR, GL_LINEAR, 1);  

        // render quad 
        gRenDev->SetState(GS_NODEPTHTEST);
        gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);    

        fpGlare->mfSet(false, 0); 
        fpRenderModeCold->mfSet(false, 0);
        pRenderer->EF_CommitPS();
        pRenderer->EF_CommitVS();

        // copy framebuffer into texture
        CopyScreenToTexture(pRenderer, pScreenTex);
      }      
        
      // ------------------ 
      // generate glare map
      // ------------------
      // if not reset to default, for some reason texture coordinates get wrong
      gRenDev->ResetToDefault(); 
      gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height);
 
      // set fragment program    
      vpGlare->mfSet(true, 0);      
      fpGlareMap->mfSet(true, 0);
      pRenderer->EF_CommitPS();
      pRenderer->EF_CommitVS();

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

      float pfBluredSize[4]={ (float)pScreenLow->m_Width, (float) pScreenLow->m_Height, 0, 0};      
      vpGlare->mfParameter4f("vTexCoordScale01", pfBluredSize); 
      float pfLumSize[4]={ (float)pScreenLuminosityTex->m_Width, (float)pScreenLuminosityTex->m_Height, 0, 0};    
      vpGlare->mfParameter4f("vTexCoordScale02", pfLumSize); 

      // use first mip map
      SetTexture(pRenderer, pScreenLow, 0, GL_NEAREST, GL_NEAREST, 1);             
      SetTexture(pRenderer, pScreenLuminosityTex, 1, GL_NEAREST, GL_NEAREST, 1); 

      gRenDev->SetState(GS_NODEPTHTEST);
      gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)), 4);               
      fpGlareMap->mfSet(false, 0);
      vpGlare->mfSet(false, 0); 

      // get lowres screen texture
      CopyScreenToTexture(pRenderer, pTex);

      // blur texture
      if(CRenderer::CV_r_rendermode!=3)   
      {
        gRenDev->SetViewport( 0, 0, pScreenTex->m_Width, pScreenTex->m_Height);
        BlurTextureHw(m_pVars, pRenderer, pTex, 0, iGlareQuality*4, 0, 0);         
      } 

      // if not reset to default, for some reason texture coordinates get wrong
      gRenDev->ResetToDefault();  

      // add glare to screen 
      gRenDev->SetViewport(0, 0, pScreenTex->m_Width, pScreenTex->m_Height);    

      // ----------------------------------------
      // apply glare in image and post-process it
      // ----------------------------------------
      {
        // set pixel/vertex programs
        vpGlare->mfSet(true, 0);
        fpGlare->mfSet(true, 0); 
        pRenderer->EF_CommitPS();
        pRenderer->EF_CommitVS();

        // due to cg bug, need to pass 2 diferent constants..
        float pCurrContrast01[]= { 0.0f, 0.0f, 0.0f, m_pVars->m_pCurrContrast.b }; 
        float pCurrContrast02[]= { 0.0f, 0.0f, 0.0f, m_pVars->m_pCurrContrast.a };       
        // set pixel program constants
        fpGlare->mfParameter4f("vSaturationAmount", pSaturationConsts);        
        fpGlare->mfParameter4f("vConstrastAmount", pCurrContrast01);    
        fpGlare->mfParameter4f("vConstrastAmount02", pCurrContrast02);  

        // set texture coordinates scale (needed for rectangular textures in gl ..)        
        float pfTexel01[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, 0, 0};    
        vpGlare->mfParameter4f("vTexCoordScale01", pfTexel01); 
        float pfTexel02[4]={ (float)pTex->m_Width, (float)pTex->m_Height, 0, 0};      
        vpGlare->mfParameter4f("vTexCoordScale02", pfTexel02); 
        float pfTexel03[4]= { 1, 1, 0, 0};       
        vpGlare->mfParameter4f("vTexCoordScale03", pfTexel03); 

        // set screen, glare map and glare adjustment texture
        SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);      
        SetTexture(pRenderer, pTex, 1, GL_LINEAR, GL_LINEAR, 1);    

        // render quad
        gRenDev->SetState(GS_NODEPTHTEST);
        gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);  

        if(m_pVars->m_bFlashBangActive ||  m_pVars->m_bCartoonActive || m_pVars->m_bBlurActive)
          CopyScreenToTexture(pRenderer, pScreenTex);

        fpGlare->mfSet(false, 0); 
        vpGlare->mfSet(false, 0);
      }
    }  
    else
    { 
      // make sure, stuff is disabled (can be enabled by menu options or console vars)
      if(m_pVars->m_pCVStencilShadows->GetIVal()) 
      {
        m_pVars->m_iPrevStencilShadows=m_pVars->m_pCVStencilShadows->GetIVal();      
        m_pVars->m_pCVStencilShadows->Set(0);         
      }

      if(m_pVars->m_pCVShadowMaps->GetIVal())
      {
        m_pVars->m_iPrevShadowMaps=m_pVars->m_pCVShadowMaps->GetIVal();      
        m_pVars->m_pCVShadowMaps->Set(0);         
      }

      if(m_pVars->m_pCVVolFog->GetIVal())
      {
        m_pVars->m_iPrevVolFog=m_pVars->m_pCVVolFog->GetIVal();      
        m_pVars->m_pCVVolFog->Set(0);
      }          

      // ------------------ 
      // process cry-vision
      // ------------------

      LOG_EFFECT("*** Begin Cryvision... ***\n");
      // get data      
      STexPic   *pTex = gRenDev->m_TexMan->m_Text_Glare,                        
                *pScreenLow = gRenDev->m_TexMan->m_Text_ScreenLowMap,
                *pHeatTexture = gRenDev->m_TexMan->m_Text_ScreenLowMap;

      // get vertex/fragment programs    
      CCGPShader_GL *fpGlareMap=(CCGPShader_GL *) m_pVars->m_pRCGlareMap;      

      CCGVProgram_GL *vpGlare=(CCGVProgram_GL *) m_pVars->m_pVPGlare;
      CCGPShader_GL *fpGlare=(CCGPShader_GL *) m_pVars->m_pRCGlare;

      CCGVProgram_GL *vpNightGlare=(CCGVProgram_GL *) m_pVars->m_pVPNightVision;
      CCGPShader_GL *fpNightGlare=(CCGPShader_GL *) m_pVars->m_pRCNightVision;

      CCGPShader_GL *fpHeatSource=(CCGPShader_GL *) m_pVars->m_pRCHeatVision;
      CCGPShader_GL *fpHeatSourceDecode=(CCGPShader_GL *) m_pVars->m_pRCHeatSourceDecode;    

      gRenDev->SetViewport(0, 0, iWidth, iHeight);
      gRenDev->SetMaterialColor(1,1,1,1);
      gRenDev->SetState(GS_NODEPTHTEST);

      // ---------------------------------
      // get heat mask from screen texture    
      // ---------------------------------
      {
        gRenDev->SetViewport(0, 0, pHeatTexture->m_Width, pHeatTexture->m_Height); 

        // set pixel/vertex programs 
        vpNightGlare->mfSet(true, 0);
        fpHeatSource->mfSet(true, 0);  
        pRenderer->EF_CommitPS();
        pRenderer->EF_CommitVS();

        // set texture coordinates scale (needed for rectangular textures in gl ..)
        float pfScale01[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, 1, 1};    
        vpNightGlare->mfParameter4f("vTexCoordScale01", pfScale01);        
        float pfScale02[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, 1, 1};    
        vpNightGlare->mfParameter4f("vTexCoordScale02", pfScale02);   
        float pfScalePal[4]= { 1, 1, 0, 0};       
        vpNightGlare->mfParameter4f("vTexCoordScale03", pfScalePal);
        float pfParams[]= { 1, 1, 0, 0};
        vpNightGlare->mfParameter4f("fNoiseParams", pfParams);   

        float fMotionBlurAmount=75.0f*pTimer->GetFrameTime();
        if(fMotionBlurAmount>1) fMotionBlurAmount=1;

        float pfHeatParams[]= { 1, 1, 1, fMotionBlurAmount};
        fpHeatSource->mfParameter4f("vHeatConstants", pfHeatParams);   

        // setup texture stages/states    
        STexPic *pPaleteTex = gRenDev->m_TexMan->m_Text_HeatPalete;
        SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);
        SetTexture(pRenderer, pScreenTex, 1, GL_NEAREST, GL_NEAREST, 1);                      
        SetTexture(pRenderer, pPaleteTex, 2, GL_NEAREST, GL_NEAREST, 1);  
        // render quad (note: use alpha blending to add motion blur on heat sources )
        gRenDev->SetState(GS_NODEPTHTEST);  
        gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);  

        // copy screen texture
        CopyScreenToTexture(pRenderer, pHeatTexture);

        vpNightGlare->mfSet(false, 0);  
        fpHeatSource->mfSet(false, 0); 

        // blur texture
        BlurTextureHw(m_pVars, pRenderer, pHeatTexture, 0, 3, 0, 0);            
      }

      // ------------------------------------
      // delete heat mask from screen texture
      // ---------------------------------
      // if not reset to default, for some reason texture coordinates get wrong
      gRenDev->ResetToDefault();   

      {
        gRenDev->SetViewport(0, 0, iWidth, iHeight);
        
        // setup texture stages/states    
        STexPic *pPaleteTex = gRenDev->m_TexMan->m_Text_HeatPalete;
        SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);
        SetTexture(pRenderer, pScreenTex, 1, GL_NEAREST, GL_NEAREST, 1);      
        SetTexture(pRenderer, pPaleteTex, 2, GL_NEAREST, GL_NEAREST, 1);

        // set pixel/vertex programs
        vpNightGlare->mfSet(true, 0);
        fpHeatSourceDecode->mfSet(true, 0); 
        pRenderer->EF_CommitPS();
        pRenderer->EF_CommitVS();

        // set texture coordinates scale (needed for rectangular textures in gl ..)
        float pfScale01[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, 1, 1};    
        vpNightGlare->mfParameter4f("vTexCoordScale01", pfScale01);        
        float pfScale02[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, 1, 1};    
        vpNightGlare->mfParameter4f("vTexCoordScale02", pfScale02);   
        float pfScalePal[4]= { 1, 1, 0, 0};       
        vpNightGlare->mfParameter4f("vTexCoordScale03", pfScalePal);
        float pfParams[]= { 1, 1, 0, 0};
        vpNightGlare->mfParameter4f("fNoiseParams", pfParams);   

        // render quad, use alpha blending to add motion blur on heat sources      
        gRenDev->SetState(GS_NODEPTHTEST);
        gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);  

        // copy screen texture
        CopyScreenToTexture(pRenderer, pScreenTex);

        vpNightGlare->mfSet(false, 0); 
        fpHeatSourceDecode->mfSet(false, 0); 
      }

      // ------------------ 
      // generate glare map
      // ------------------
      // if not reset to default, for some reason texture coordinates get wrong
      gRenDev->ResetToDefault(); 
      gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height);

      // set fragment program    
      vpGlare->mfSet(true, 0);      
      fpGlareMap->mfSet(true, 0);
      pRenderer->EF_CommitPS();
      pRenderer->EF_CommitVS();

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

      float pfBluredSize[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, 0, 0};      
      vpGlare->mfParameter4f("vTexCoordScale01", pfBluredSize); 
      float pfLumSize[4]= { 1, 1, 0, 0 };  
      vpGlare->mfParameter4f("vTexCoordScale02", pfLumSize); 

      // use first mip map
      SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);             
      STexPic *pWhiteTex = gRenDev->m_TexMan->m_Text_White;            
      SetTexture(pRenderer, pWhiteTex, 1, GL_NEAREST, GL_NEAREST, 1); 

      gRenDev->SetState(GS_NODEPTHTEST);
      gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)), 4);               

      // copy glare texture
      CopyScreenToTexture(pRenderer, pTex);

      fpGlareMap->mfSet(false, 0);
      vpGlare->mfSet(false, 0); 

      gRenDev->SetViewport( 0, 0, pScreenTex->m_Width, pScreenTex->m_Height);
      BlurTextureHw(m_pVars, pRenderer, pTex, 0, 3, 0, 0);        


      // if not reset to default, for some reason texture coordinates get wrong
      gRenDev->ResetToDefault();  

      gRenDev->SetViewport(0, 0, pScreenTex->m_Width, pScreenTex->m_Height);    

      // ----------------- 
      // nightvision+glare

      // get noise texture
      STexPic *pScreenNoiseTex = gRenDev->m_TexMan->m_Text_ScreenNoise;

      // set pixel/vertex programs
      vpNightGlare->mfSet(true, 0);
      fpNightGlare->mfSet(true, 0); 
      pRenderer->EF_CommitPS();
      pRenderer->EF_CommitVS();

      // setup noise parameters
      static float fOffsetU=0, fOffsetV=0;    
      fOffsetV+=(pTimer->GetFrameTime())*20.0f;
      if(fOffsetV>2.0f)
      {
        fOffsetV=0.0f;
      }

      float pfNoiseParams[]= { 0.25f, 2.5f, fOffsetU, fOffsetV };
      SCGBind *pNoiseParams = vpNightGlare->mfGetParameterBind("fNoiseParams");
      vpNightGlare->mfParameter(pNoiseParams, pfNoiseParams, 1);

      // set pixel program constants
      float pGlareData[]= { m_pVars->m_fGlareThreshold, m_pVars->m_fGlareThreshold, m_pVars->m_fGlareThreshold, m_pVars->m_fGlareAmount};
      fpNightGlare->mfParameter4f("Glare", pGlareData);

      float pNightVisionColor[]= { m_pVars->m_pNightVisionColor.r, m_pVars->m_pNightVisionColor.g, m_pVars->m_pNightVisionColor.b, m_pVars->m_fGlareAmount};
      fpNightGlare->mfParameter4f("NVColor", pNightVisionColor);

      // set texture coordinates scale (needed for rectangular textures in gl ..)
      float pfScale01[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, 0, 0};    
      vpNightGlare->mfParameter4f("vTexCoordScale01", pfScale01);   
      float pfScale02[4]={ (float)pTex->m_Width, (float)pTex->m_Height, 0, 0};    
      vpNightGlare->mfParameter4f("vTexCoordScale02", pfScale02);   
      float pfScale03[4]={ (float)pHeatTexture->m_Width, (float)pHeatTexture->m_Height, 0, 0};     
      vpNightGlare->mfParameter4f("vTexCoordScale03", pfScale03);   

      // set screen texture
      SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);
      // set glare texture
      SetTexture(pRenderer, pTex, 1, GL_LINEAR, GL_LINEAR, 1); 
      // heat texture
      SetTexture(pRenderer, pHeatTexture, 2, GL_NEAREST, GL_NEAREST, 1); 
      // set noise texture
      SetTexture(pRenderer, pScreenNoiseTex, 3, GL_NEAREST, GL_NEAREST, 0);

      // render quad
      gRenDev->SetState(GS_NODEPTHTEST);
      gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)), 4);  

      if(m_pVars->m_bFlashBangActive ||  m_pVars->m_bCartoonActive || m_pVars->m_bBlurActive)
      {
        CopyScreenToTexture(pRenderer, pScreenTex);
      }

      vpNightGlare->mfSet(false, 0);
      fpNightGlare->mfSet(false, 0); 

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
    CCGVProgram_GL *vpFlashBang=(CCGVProgram_GL *) m_pVars->m_pVPFlashBang;    
    CCGPShader_GL *fpFlashBang=(CCGPShader_GL *) m_pVars->m_pRCFlashBang;    

    pRenderer->EF_SelectTMU(1);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(2);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(3);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(0);

    // generate flashbang texture if necessary
    if(m_pVars->m_fFlashBangTimeOut==1.0f || m_pVars->m_iFlashBangForce)  
    {
      m_pVars->m_iFlashBangForce=0;

      // setup viewport
      gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height );

      // set screen texture
      SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);

      float fOffX=0.5f/(float)pTex->m_Width, fOffY=0.5f/(float)pTex->m_Height;
      gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenSizeQuad,VERTEX_FORMAT_P3F_TEX2F)),4);   

      // render flashbang flash 
      STexPic *pFlashBangFlash = gRenDev->m_TexMan->m_Text_FlashBangFlash;
      gRenDev->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);

      // get flashbang properties
      float fPosX, fPosY, fSizeX, fSizeY;
      fPosX=m_pVars->m_fFlashBangFlashPosX;  
      fPosY=m_pVars->m_fFlashBangFlashPosY;  

      fSizeX=m_pVars->m_fFlashBangFlashSizeX;
      fSizeY=m_pVars->m_fFlashBangFlashSizeY;
      
      gRenDev->Draw2dImage((fPosX-fSizeX*0.5f), (fPosY-fSizeY*0.5f), fSizeX, fSizeY, pFlashBangFlash->GetTextureID()); 
      //gRenDev->EnableBlend(false);
      gRenDev->SetState(GS_NODEPTHTEST);
      
      CopyScreenToTexture(pRenderer, pTex);

      // blur texture
      BlurTextureHw(m_pVars, pRenderer, pTex, 0, 2, 0, 0);       
    }

    // render flashbang
    gRenDev->SetViewport(0, 0, iWidth, iHeight);

    // set vertex program
    vpFlashBang->mfSet(true, 0);
    // set fragment program
    fpFlashBang->mfSet(true, 0);
    pRenderer->EF_CommitPS();
    pRenderer->EF_CommitVS();

    // set texture coordinates scale (needed for rectangular textures in gl ..)    
    float pfScale01[4]={ (float)pScreenTex->m_Width, (float)pScreenTex->m_Height, (float)pTex->m_Width, (float)pTex->m_Height};     
    vpFlashBang->mfParameter4f("vTexCoordScale", pfScale01); 

    // set fragment vars
    float fBrightness=1.0f;  
    fBrightness=InterpolateCubic(0.0f, .0f, 1.0f, .5f, m_pVars->m_fFlashBangTimeOut);

    float pFlashBang[]= { 0, 0, 0, fBrightness};
    fpFlashBang->mfParameter4f("FlashBang", pFlashBang);

    // set texture states
    SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);
    SetTexture(pRenderer, pTex, 1, GL_LINEAR, GL_LINEAR, 1);

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

    // just render..
    gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)), 4);

    // check if motion blur is not active, else we must update screen texture/or get latest screen texture
    if(m_pVars->m_bCartoonActive || m_pVars->m_bBlurActive) 
    {
      CopyScreenToTexture(pRenderer, pScreenTex);
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

    LOG_EFFECT("*** End FlashBang... ***\n");
  }

  // ==============================================================================================
  // Blur - simple screen blur

  if(m_pVars->m_bBlurActive)
  {
    LOG_EFFECT("*** Begin blury screen process... ***\n")     

    STexPic *pScreenBluredTex = gRenDev->m_TexMan->m_Text_ScreenLowMap;  

    // get vertex/fragment program    
    CCGVProgram_GL *vpBluryMap=(CCGVProgram_GL *) m_pVars->m_pVPBluryScreen;  
    CCGPShader_GL *fpBluryMap=(CCGPShader_GL *) m_pVars->m_pRCBluryScreen;

    gRenDev->SetViewport(0, 0, pScreenBluredTex->m_Width, pScreenBluredTex->m_Height);

    pRenderer->EF_SelectTMU(1);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(2);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(3);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(0);

    // setup texture stages/states
    gRenDev->SetMaterialColor(1,1,1, 1);   
    gRenDev->SetState(GS_NODEPTHTEST);

    // setup texture stages/states
    SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1);

    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenSizeQuad,VERTEX_FORMAT_P3F_TEX2F)),4); 

    CopyScreenToTexture(pRenderer, pScreenBluredTex);

    BlurTextureHw(m_pVars, pRenderer, pScreenBluredTex, 0, 4, 0, 0); 

    gRenDev->SetViewport(0, 0, iWidth, iHeight);

    // set current vertex/fragment program
    vpBluryMap->mfSet(true, NULL);
    fpBluryMap->mfSet(true, NULL);
    pRenderer->EF_CommitPS();
    pRenderer->EF_CommitVS();

    // set constants
    float pBluryParams[]= { m_pVars->m_pBlurColor.r, m_pVars->m_pBlurColor.g, m_pVars->m_pBlurColor.b, m_pVars->m_fBlurAmount };
    fpBluryMap->mfParameter4f("fBluryParams", pBluryParams);

    // set texture coordinates scale (needed for rectangular textures in gl ..)    
    float pfScale01[4]={ (float)pScreenBluredTex->m_Width, (float)pScreenBluredTex->m_Height, (float)pScreenTex->m_Width, (float)pScreenTex->m_Height};    
    vpBluryMap->mfParameter4f("vTexCoordScale", pfScale01);   

    // setup texture stages/states
    SetTexture(pRenderer, pScreenBluredTex, 0, GL_LINEAR, GL_LINEAR, 1);
    SetTexture(pRenderer, pScreenTex, 1, GL_NEAREST, GL_NEAREST, 1);

    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)),4);

    // check if motion blur is not active, else we must update screen texture/or get latest screen texture
    if(m_pVars->m_bCartoonActive) 
    {
      CopyScreenToTexture(pRenderer, pScreenTex);
    }

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

    // get vertex/fragment program
    CCGVProgram_GL *vpCartoon=(CCGVProgram_GL *) m_pVars->m_pVPCartoon;
    CCGPShader_GL *fpCartoon=(CCGPShader_GL *) m_pVars->m_pRCCartoon;
    CCGPShader_GL *fpCartoonSilhouete=(CCGPShader_GL *) m_pVars->m_pRCCartoonSilhouette;

    gRenDev->SetViewport(0, 0, iWidth, iHeight);

    // set current vertex/fragment program
    vpCartoon->mfSet(true, NULL); 
    fpCartoon->mfSet(true, NULL);
    pRenderer->EF_CommitPS();
    pRenderer->EF_CommitVS();

    // set vertex program consts      
    // setup texture offsets, for texture neighboors sampling
    float s1=0;      
    float t1=0;  

    float pfNoOffset[]={ s1, t1, 0, 0 }; 
    // set texture coordinates scale (needed for rectangular textures in gl ..)        
    float pfScale01[]={ (float)iWidth, (float)iHeight, (float)iWidth, (float)iHeight};        
    vpCartoon->mfParameter4f("vTexCoordScale", pfScale01);     
    vpCartoon->mfParameter4f("Offset0", pfNoOffset);
    vpCartoon->mfParameter4f("Offset1", pfNoOffset);
    vpCartoon->mfParameter4f("Offset2", pfNoOffset);
    vpCartoon->mfParameter4f("Offset3", pfNoOffset);

    // render quad
    SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1); 

    // just render
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)),4); 

    vpCartoon->mfSet(false, NULL);
    fpCartoon->mfSet(false, NULL); 

    CopyScreenToTexture(pRenderer, pScreenTex);

    // set current vertex/fragment program 
    vpCartoon->mfSet(true, NULL);
    fpCartoonSilhouete->mfSet(true, NULL);
    pRenderer->EF_CommitPS();
    pRenderer->EF_CommitVS();

    s1=1.0f;    
    t1=1.0f; 

    float pfOffset0[]={ s1*0.5f,       t1, 0.0f, 0.0f}; 
    float pfOffset1[]={     -s1,  t1*0.5f, 0.0f, 0.0f}; 
    float pfOffset2[]={-s1*0.5f,      -t1, 0.0f, 0.0f}; 
    float pfOffset3[]={      s1, -t1*0.5f, 0.0f, 0.0f}; 

    // set vertex program consts      
    vpCartoon->mfParameter4f("vTexCoordScale", pfScale01);     
    vpCartoon->mfParameter4f("Offset0",pfOffset0);
    vpCartoon->mfParameter4f("Offset1",pfOffset1);
    vpCartoon->mfParameter4f("Offset2",pfOffset2);
    vpCartoon->mfParameter4f("Offset3",pfOffset3);

    // render quad
    SetTexture(pRenderer, pScreenTex, 0, GL_NEAREST, GL_NEAREST, 1); 
    SetTexture(pRenderer, pScreenTex, 1, GL_NEAREST, GL_NEAREST, 1);
    SetTexture(pRenderer, pScreenTex, 2, GL_NEAREST, GL_NEAREST, 1);
    SetTexture(pRenderer, pScreenTex, 3, GL_NEAREST, GL_NEAREST, 1);

    // just render
    gRenDev->SetState(GS_BLSRC_ZERO | GS_BLDST_SRCCOL | GS_NODEPTHTEST);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)),4); 

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
    CCGVProgram_GL *vpMotion=(CCGVProgram_GL *) m_pVars->m_pVPMotion;
    CCGPShader_GL *fpMotion=(CCGPShader_GL *) m_pVars->m_pRCMotion;

    pRenderer->EF_SelectTMU(1);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(2);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(3);
    pRenderer->EnableTMU(false);
    pRenderer->EF_SelectTMU(0);

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
        gRenDev->SetColorOp(eCO_MODULATE, eCO_MODULATE, eCA_Texture | (eCA_Constant<<3), eCA_Texture | (eCA_Constant<<3));
        SetTexture(pRenderer, pTex, 0, GL_NEAREST, GL_NEAREST, 1);             
        gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenSizeQuad,VERTEX_FORMAT_P3F_TEX2F)),4);            
      } 
      break; 

      // radial blur
    case 2:
      float fAmount=CLAMP(m_pVars->m_fMotionBlurAmount, 0.0f, 1.0f);

      // set current vertex/fragment program
      vpMotion->mfSet(true, NULL);
      fpMotion->mfSet(true, NULL);
      pRenderer->EF_CommitPS();
      pRenderer->EF_CommitVS();

      // setup texture stages/states
      SetTexture(pRenderer, pTex, 0, GL_NEAREST, GL_NEAREST, 1);
      SetTexture(pRenderer, pTex, 1, GL_NEAREST, GL_NEAREST, 1);
      SetTexture(pRenderer, pTex, 2, GL_NEAREST, GL_NEAREST, 1);
      SetTexture(pRenderer, pTex, 3, GL_NEAREST, GL_NEAREST, 1);

      // setup vertex program
      float pfTexSize[]= {(float) pScreenTex->m_Width, (float) pScreenTex->m_Height, 0, 0 }; 
      SCGBind *pTexSize = vpMotion->mfGetParameterBind("TexSize");   
      vpMotion->mfParameter(pTexSize, pfTexSize, 1);

      SCGBind *pTexScale = vpMotion->mfGetParameterBind("TexScale");  

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
      vpMotion->mfParameter(pTexScale, pfTexScale, 1); 

      // just render
      gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad, VERTEX_FORMAT_P3F_TEX2F)),4);

      // disable current vertex/fragment program
      vpMotion->mfSet(false, NULL);
      fpMotion->mfSet(false, NULL);
      break;
    }

    // copy into 'previous framebuffer' texture
    CopyScreenToTexture(pRenderer, pTex);

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
    SetTexture(pRenderer, pWhiteTex, 0, GL_NEAREST, GL_NEAREST, 1);
    gRenDev->DrawTriStrip(&(CVertexBuffer(pScreenQuad,VERTEX_FORMAT_P3F_TEX2F)),4);    

    LOG_EFFECT("*** End screen fade process... ***\n")
  }

  gRenDev->Set2DMode(false, 1, 1); 
  gRenDev->SetViewport(iTempX, iTempY, iWidth, iHeight);
  gRenDev->ResetToDefault();

  return 1; 
} 