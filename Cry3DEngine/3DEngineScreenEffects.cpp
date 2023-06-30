/*
=====================================================================
FILE : 3dEngineScreenEffects.cpp
DESC : Screen processing effects
PROJ : Crytek Engine
CODER: Tiago Sousa

Last Update: 09/12/2003
TODO: 
.Clean up code...
.Remove all SCREEN_PROCESS stuff (Set/GetParameter, etc)
=====================================================================
*/

#include "stdafx.h"
#include "3dEngine.h"
#include "objman.h"
#include "../RenderDll/Common/RendElements/CREScreenCommon.h"

// some helper macros
#define GET_ACTIVEPARAMETER(pProcess) \
  *((bool*)m_pREScreenProcess->mfGetParameter((pProcess), SCREENPROCESS_ACTIVE)) \

#define GET_CVAR(pVar)\
  (GetConsole()->GetCVar((pVar)))\

// interfaces for script usage

// set screen mask texture
void C3DEngine:: SetBlurMask(ITexPic *pMask)
{
  if(!m_pBlurObj)
  {
    return;
  }

  //m_pBlurObj->SetBlurMask((STexPic*) pMask);
}

// set screen mask texture
void C3DEngine:: SetScreenMask(ITexPic *pMask)
{
  if(!m_pScreenObj)
  {
    return;
  }

  //m_pScreenObj->SetScreenMask((STexPic*) pMask);
}

// setup current screen fx 
void C3DEngine:: SetScreenFx(const char *pEffectName, int iActive)
{
  bool bActive=iActive? 1:0;
  // to avoid an untreackable crash in mscvdll strcmp
	if (!pEffectName)
  {
    return; 
  }	

  if(!strcmp(pEffectName, "NightVision"))
  {     
    int iActive=bActive;
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_NIGHTVISION, SCREENPROCESS_ACTIVE, &iActive);
  }
  else
  if(!strcmp(pEffectName, "HeatVision"))
  {
    int iActive=bActive;
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_HEATVISION, SCREENPROCESS_ACTIVE, &iActive);
  } 
  else
  if(!strcmp(pEffectName, "ScreenBlur"))
  {
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_ACTIVE, &bActive);
  } 
  else
  if(!strcmp(pEffectName, "FlashBang"))
  {
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_ACTIVE, &bActive);
  } 
  else
  if(!strcmp(pEffectName, "ScreenFade"))
  {
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FADE, SCREENPROCESS_ACTIVE, &bActive);
  } 
}

// Setup current screen fx parameter
void C3DEngine:: SetScreenFxParam(const char *pEffectName, const char *pEffectParam, void *pValue)
{
  // <<TODO>> add required/missing parameters

  // to avoid an untreackable crash in mscvdll strcmp
  if (!pEffectName || !pEffectParam)
  {
    return; 
  }

  // set parameters for screen fade
  if(!stricmp(pEffectName, "ScreenFade"))
  {    
    if(!stricmp(pEffectParam, "ScreenFadeTime"))  // set screen fade flag
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FADE, SCREENPROCESS_TRANSITIONTIME, pValue);
    }
    else    
    if(!stricmp(pEffectParam, "ScreenPreFadeTime")) // set screen pre fade flag (count number of frames, before start fading)
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FADE, SCREENPROCESS_PRETRANSITIONTIME, pValue);
    }    
  }
  else   
  if(!stricmp(pEffectName, "ScreenBlur")) // set parameters for screen blur
  {    
    if(!stricmp(pEffectParam, "ScreenBlurAmount"))  // screen blur amount
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURAMOUNT, pValue);
    }    
    else    
    if(!stricmp(pEffectParam, "ScreenBlurColorRed"))  // screen blur color red..
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURCOLORRED, pValue);
    }    
    else    
    if(!stricmp(pEffectParam, "ScreenBlurColorGreen"))  // screen blur color green..
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURCOLORGREEN, pValue);
    }    
    else   
    if(!stricmp(pEffectParam, "ScreenBlurColorBlue")) // screen blur color blue..
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURCOLORBLUE, pValue);
    }    
  }
  else  // get parameters for flashbang  
  if(!stricmp(pEffectName, "FlashBang"))
  {    
    if(!stricmp(pEffectParam, "FlashBangTimeScale"))  // flashbang timeout
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGTIMESCALE, pValue);
    }    
    else        
    if(!stricmp(pEffectParam, "FlashBangFlashPosX"))  // flashbangflash position x
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHPOSX, pValue);
    }    
    else        
    if(!stricmp(pEffectParam, "FlashBangFlashPosY"))  // flashbangflash position y
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHPOSY, pValue);
    }    
    else        
    if(!stricmp(pEffectParam, "FlashBangFlashSizeX")) // flashbangflash size x
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHSIZEX, pValue);
    }    
    else
    if(!stricmp(pEffectParam, "FlashBangFlashSizeY")) // flashbangflash size y
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHSIZEY, pValue);
    }    
    else
    if(!stricmp(pEffectParam, "FlashBangForce")) // force flashbang after image texture creation
    {
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFORCEAFTERIMAGE, pValue);
    }    
  }
}

int C3DEngine::GetScreenFx(const char *pEffectName)
{
  // to avoid an untreackable crash in mscvdll strcmp
  if (!pEffectName)
  {
    return -1; 
  }	

  if(!strcmp(pEffectName, "NightVision"))
  {         
    bool bVal=*((bool*)m_pREScreenProcess->mfGetParameter(SCREENPROCESS_NIGHTVISION, SCREENPROCESS_ACTIVE));
    return bVal;
  }
  else
  if(!strcmp(pEffectName, "HeatVision"))
  {    
    bool bVal=*((bool*)m_pREScreenProcess->mfGetParameter(SCREENPROCESS_HEATVISION, SCREENPROCESS_ACTIVE));
    return bVal;
  } 
  else
  if(!strcmp(pEffectName, "ScreenBlur"))
  {
    bool bVal=*((bool*)m_pREScreenProcess->mfGetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_ACTIVE));
    return bVal;
  } 
  else
  if(!strcmp(pEffectName, "FlashBang"))
  {
    bool bVal=*((bool*)m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_ACTIVE));
    return (int)bVal;     
  } 
  else
  if(!strcmp(pEffectName, "ScreenFade"))
  {
    bool bVal=*((bool*)m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FADE, SCREENPROCESS_ACTIVE));
    return bVal;
  } 

  return -1;
}

int C3DEngine::GetScreenFxParam(const char *pEffectName, const char *pEffectParam, void *&pValue)
{
  // to avoid an untreackable crash in mscvdll strcmp
  if (!pEffectName || !pEffectParam)
  {
    return 0; 
  }

  // get parameters for screen fade
  if(!stricmp(pEffectName, "ScreenFade"))
  {    
    if(!stricmp(pEffectParam, "ScreenFadeTime"))  // set screen fade flag
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FADE, SCREENPROCESS_TRANSITIONTIME);
    }    
    else    
    if(!stricmp(pEffectParam, "ScreenPreFadeTime")) // set screen pre fade flag (count number of frames, before start fading)
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FADE, SCREENPROCESS_PRETRANSITIONTIME);
    }    
    else 
    {
      // parameter doens't exist
      return -1;
    }
  }
  else   
  if(!stricmp(pEffectName, "ScreenBlur")) // get parameters for screen blur
  {    
    if(!stricmp(pEffectParam, "ScreenBlurAmount"))  // screen blur amount
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURAMOUNT);
    } 
    else    
    if(!stricmp(pEffectParam, "ScreenBlurColorRed"))  // screen blur color red..
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURCOLORRED);
    }   
    else    
    if(!stricmp(pEffectParam, "ScreenBlurColorGreen"))  // screen blur color green..
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURCOLORGREEN);
    }   
    else    
    if(!stricmp(pEffectParam, "ScreenBlurColorBlue")) // screen blur color blue..
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_BLUR, SCREENPROCESS_BLURCOLORBLUE);
    }    
    else 
    {
      // parameter doens't exist
      return -1;
    }
  }
  else  
  if(!stricmp(pEffectName, "FlashBang"))  // get parameters for flashbang  
  {    
    if(!stricmp(pEffectParam, "FlashBangTimeScale"))  // flashbang timeout
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGTIMESCALE);
    }    
    else
    if(!stricmp(pEffectParam, "FlashBangFlashPosX"))  // flashbangflash position x
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHPOSX);
    }    
    else
    if(!stricmp(pEffectParam, "FlashBangFlashPosY"))  // flashbangflash position y
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHPOSY);
    }    
    else
    if(!stricmp(pEffectParam, "FlashBangFlashSizeX")) // flashbangflash size x
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHSIZEX);
    }        
    else
    if(!stricmp(pEffectParam, "FlashBangFlashSizeY")) // flashbangflash size y
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFLASHSIZEY);
    }    
    else
    if(!stricmp(pEffectParam, "FlashBangTimeOut")) // get current time out time
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGTIMEOUT);      
    }    
    else
    if(!stricmp(pEffectParam, "FlashBangForce")) // force flashbang after image texture creation
    {
      pValue=m_pREScreenProcess->mfGetParameter(SCREENPROCESS_FLASHBANG, SCREENPROCESS_FLASHBANGFORCEAFTERIMAGE);      
    }    
    else 
    {
      // parameter doens't exist
      return -1;
    }
  }
  else 
  {
    // parameter doens't exist
    return -1;
  }

  return 1;
}

// reset current screen effects
void C3DEngine::ResetScreenFx(void)
{
  if(m_pREScreenProcess)
  {
    m_pREScreenProcess->mfReset();
  }  
}

// process all screen space special fx's
void C3DEngine::ProcessScreenEffects()
{
  // don't allow this in recursive rendering..
  if(m_pObjManager->m_nRenderStackLevel!=0 || !m_pREScreenProcess)
  {
    return;
  }  

  FUNCTION_PROFILER( GetSystem(), PROFILE_RENDERER );
  
  // get console vars 
  static ICVar *pDisableSfx=GET_CVAR("r_DisableSfx");
  static ICVar *pResetSfx=GET_CVAR("r_ResetScreenFx");

  // reset screen effects state
  if(pResetSfx->GetIVal()!=0)
  {
    m_pREScreenProcess->mfReset();      
  }
  
  // disable fx's
  if(!pDisableSfx->GetIVal())
  {    
    static ICVar *pNormalGlare=GET_CVAR("r_Glare"); 
    static ICVar *pMotionBlur=GET_CVAR("r_MotionBlur");    
    static ICVar *pScreenColorTransfer=GET_CVAR("r_ScreenColorTransfer");  
    static ICVar *pMotionBlurAmount=GET_CVAR("r_MotionBlurAmount");
    static ICVar *pMotionBlurDisplace=GET_CVAR("r_MotionBlurDisplace");
    static ICVar *pRenderMode=GET_CVAR("r_RenderMode");

    // get console vars values  
    int iUseMotionBlur=pMotionBlur->GetIVal();
    int iRenderMode=pRenderMode->GetIVal();  
    int iUseGlare=((pNormalGlare->GetIVal()) && (iRenderMode>0));
    
    // activate fx's
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_MOTIONBLUR, SCREENPROCESS_ACTIVE, &iUseMotionBlur);
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_GLARE, SCREENPROCESS_ACTIVE, &iUseGlare);

    // cartoon render mode active ?
    bool bCartoon=0;
    if(iRenderMode==4)
    {
      bCartoon=1;
    }
    m_pREScreenProcess->mfSetParameter(SCREENPROCESS_CARTOON, SCREENPROCESS_ACTIVE, &bCartoon);
        
    // get screen texture if necessary
    if(GET_ACTIVEPARAMETER(SCREENPROCESS_FLASHBANG) || GET_ACTIVEPARAMETER(SCREENPROCESS_COLORTRANSFER) || 
      GET_ACTIVEPARAMETER(SCREENPROCESS_BLUR) || GET_ACTIVEPARAMETER(SCREENPROCESS_NIGHTVISION) || 
      GET_ACTIVEPARAMETER(SCREENPROCESS_HEATVISION) || GET_ACTIVEPARAMETER(SCREENPROCESS_GLARE) ||
      GET_ACTIVEPARAMETER(SCREENPROCESS_CARTOON) ||GET_ACTIVEPARAMETER(SCREENPROCESS_MOTIONBLUR))
    {
      CCObject  *pScreenObj=GetRenderer()->EF_GetObject(true, -1);
      pScreenObj->m_Matrix.SetIdentity();
      GetRenderer()->EF_AddEf(0, m_pREDummy,  m_pSHScreenTexMap, NULL, pScreenObj, 0, NULL, eS_Glare);
    }
    
    // process screen glare/set glare parameters
    if(GET_ACTIVEPARAMETER(SCREENPROCESS_GLARE))
    {
      IVisArea *pCurrLocation= GetVisAreaFromPos(GetViewCamera().GetPos());

      static ICVar *pGlareQuality=GET_CVAR("r_GlareQuality"); 
      if(pGlareQuality->GetIVal()!=4)
      {
        if(!pCurrLocation)
        {
          pNormalGlare->Set(2);
        }
        else
        {
          pNormalGlare->Set(3);
        }
      }
    }

    // activate motion blur  
    if(GET_ACTIVEPARAMETER(SCREENPROCESS_MOTIONBLUR))
    {
      // get console variables
      float fMotionBlurAmount=pMotionBlurAmount->GetFVal();
      int   iMotionBlurDisplace=pMotionBlurDisplace->GetIVal();

      // pass parameters and activate effect
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_MOTIONBLUR, SCREENPROCESS_MOTIONBLURDISPLACE, &iMotionBlurDisplace);
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_MOTIONBLUR, SCREENPROCESS_MOTIONBLURAMOUNT, &fMotionBlurAmount);
      m_pREScreenProcess->mfSetParameter(SCREENPROCESS_MOTIONBLUR, SCREENPROCESS_MOTIONBLURTYPE, &iUseMotionBlur);
      m_pREScreenProcess->mfActivate(SCREENPROCESS_MOTIONBLUR);
    }
  }
  
  CCObject  *pScreenProcessObj=GetRenderer()->EF_GetObject(true, -1); 
  pScreenProcessObj->m_Matrix.SetIdentity();
  GetRenderer()->EF_AddEf(0, m_pREScreenProcess, m_pSHScreenProcess, NULL, pScreenProcessObj, 0, NULL, eS_Glare);     
}
