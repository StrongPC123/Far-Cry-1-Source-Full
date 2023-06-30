/*
=======================================================================
FILE : CREScreenProcess.cpp
DESC : screen processing render element
PROJ : Crytek Engine
CODER: Tiago Sousa

TODO: 
.Convert all screen based effects into this interface

Last Update: 16/06/2003
=======================================================================
*/

#include "RenderPCH.h"

// some helper macros
#define SET_PARAMETER(pProcess, pParam, pType, pValue) \
  case (pProcess): (pParam)=*((pType*) pValue); break; \

#define RETURN_PARAMETER(pProcess, pParam) \
  case (pProcess):  return (void*)&pParam;\

// constructor/destructor
CREScreenProcess::CREScreenProcess()
{
  // setup screen process renderer type
  mfSetType(eDATA_ScreenProcess);
  mfUpdateFlags(FCEF_TRANSFORM);
  m_pVars=0;

  // create class vars if necessary...
  if(!m_pVars)
  {
    m_pVars=new CScreenVars;
    m_pVars->Create();
  }
}

CREScreenProcess::~CREScreenProcess()
{  
  SAFE_DELETE(m_pVars)
};

// prepare screen processing
void CREScreenProcess:: mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  // make sure this is not processed, when rendering to refractive texture..
  if (gRenDev->m_RP.m_bDrawToTexture)
  {
    gRenDev->m_RP.m_pRE = NULL;
  }
  else
  {
    gRenDev->m_RP.m_pRE = this;
  }

  gRenDev->m_RP.m_FlagsPerFlush |= RBSI_DRAWAS2D;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

void CREScreenProcess::mfReset()
{
  if(m_pVars)
  {
    m_pVars->Reset();
  }
}

// activate screen processes
void CREScreenProcess:: mfActivate(int pProcess)
{
  switch(pProcess)
  {
  case  SCREENPROCESS_FADE:
    if(m_pVars->m_fFadeTime)
    {
      m_pVars->m_bFadeActive=1;
    }
    else      
    {
      // reset fade
      m_pVars->m_bFadeActive=0;
      m_pVars->m_fFadePreTime=0.0f;        
      m_pVars->m_fFadeCurrPreTime=0.0f;     
    }
    break;

  case  SCREENPROCESS_BLUR:
    m_pVars->m_bBlurActive=1;
    break;
  case  SCREENPROCESS_COLORTRANSFER:
    m_pVars->m_bColorTransferActive=1;
    break;
  case  SCREENPROCESS_MOTIONBLUR:
    m_pVars->m_bMotionBlurActive=1;
    break;
  case  SCREENPROCESS_GLARE:
    m_pVars->m_bGlareActive=1;
    break;
  case  SCREENPROCESS_NIGHTVISION:
//    m_pVars->m_bNightVisionActive=1;
	  if(m_pVars->m_pCVHeatVision) 
	  {
		  m_pVars->m_pCVHeatVision->Set(1);
	  }

    break;
  case  SCREENPROCESS_HEATVISION:  
	  if(m_pVars->m_pCVHeatVision) 
	  {
		  m_pVars->m_pCVHeatVision->Set(1);
	  }
    break;
  case  SCREENPROCESS_CARTOON:
    m_pVars->m_bCartoonActive=1;
    // reset fade amount    
    if(iConsole)
    {
      if(m_pVars->m_pCVMaxTexLodBias)
      {
        if(m_pVars->m_bCartoonActive)
        {
          m_pVars->m_fPrevMaxTexLodBias=m_pVars->m_pCVMaxTexLodBias->GetFVal();
          m_pVars->m_pCVMaxTexLodBias->Set(-1.5f);
        }
        else
        {
          m_pVars->m_fPrevMaxTexLodBias=m_pVars->m_pCVMaxTexLodBias->GetFVal();
          m_pVars->m_pCVMaxTexLodBias->Set(0.0f);
        }            
      } 
    }
    break;
  case  SCREENPROCESS_FLASHBANG:
    m_pVars->m_bFlashBangActive=1;
    // reset flash time out
    m_pVars->m_fFlashBangTimeOut=1.0f;
    break;
  case  SCREENPROCESS_DOF:
    m_pVars->m_bDofActive=1;
    // reset dof focal distance plane
    m_pVars->m_fDofFocalDistance=20.0f;
    break;
  default:
    break;
  };
}

// set screen processing parameters
int CREScreenProcess:: mfSetParameter(int iProcess, int iParams, void *dwValue)
{ 
  switch(iProcess) 
  {
    // set screen fade parameters
  case SCREENPROCESS_FADE:
    switch(iParams)
    {
      SET_PARAMETER(SCREENPROCESS_FADECOLOR, m_pVars->m_pFadeColor, color4f, dwValue)
      
    case SCREENPROCESS_ACTIVE:
      if(*((bool*) dwValue)==1)
      {
        m_pVars->m_bFadeActive=1;
      }
      else      
      {
        // reset fade
        m_pVars->m_bFadeActive=0;
        m_pVars->m_fFadePreTime=0.0f;        
        m_pVars->m_fFadeCurrPreTime=0.0f;     
      }
      break; 

    case SCREENPROCESS_TRANSITIONTIME:
      m_pVars->m_fFadeTime=*((float*) dwValue);
      m_pVars->m_fFadeCurrTime=(float) fabs(*((float*) dwValue));
      break;
      
      SET_PARAMETER(SCREENPROCESS_PRETRANSITIONTIME, m_pVars->m_fFadePreTime, float, dwValue)
    }
    break;

    // set screen blur parameters
  case SCREENPROCESS_BLUR:
    switch(iParams) 
    {
      SET_PARAMETER(SCREENPROCESS_BLURAMOUNT, m_pVars->m_fBlurAmount, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bBlurActive, bool, dwValue)

      case SCREENPROCESS_BLURCOLORRED:
        m_pVars->m_pBlurColor.r=*(float*)dwValue;
          break;
      case SCREENPROCESS_BLURCOLORGREEN:
        m_pVars->m_pBlurColor.g=*(float*)dwValue;
        break;
      case SCREENPROCESS_BLURCOLORBLUE:
        m_pVars->m_pBlurColor.b=*(float*)dwValue;
        break;
    }
    break;

    // set color transfer parameters
  case SCREENPROCESS_COLORTRANSFER:
    switch(iParams) 
    {
      SET_PARAMETER(SCREENPROCESS_COLORTRANSFERAMOUNT, m_pVars->m_fColorTransferAmount, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_COLORTRANSFERCOLOR, m_pVars->m_pColorTransferColor, color4f, dwValue)
      SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bColorTransferActive, bool, dwValue)
    }
    break;

    // set motion blur parameters
  case SCREENPROCESS_MOTIONBLUR:
    switch(iParams)
    {
      SET_PARAMETER(SCREENPROCESS_MOTIONBLURDISPLACE, m_pVars->m_iMotionBlurDisplace, int, dwValue)
      SET_PARAMETER(SCREENPROCESS_MOTIONBLURAMOUNT, m_pVars->m_fMotionBlurAmount, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_MOTIONBLURTYPE, m_pVars->m_iMotionBlurType, int, dwValue)
      SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bMotionBlurActive, bool, dwValue)
    }
    break;

    // set glare parameters
  case SCREENPROCESS_GLARE:
    switch(iParams)
    {
      SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bGlareActive, bool, dwValue)
      SET_PARAMETER(SCREENPROCESS_GLAREAMOUNT, m_pVars->m_fGlareAmount, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_GLARELUMSIZE, m_pVars->m_iGlareLumSize, int, dwValue)
      SET_PARAMETER(SCREENPROCESS_GLAREMAXAMOUNT, m_pVars->m_fGlareMaxAmount, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_GLAREBOXSIZE, m_pVars->m_iGlareSize, int, dwValue)
      SET_PARAMETER(SCREENPROCESS_GLARETHRESHOLD, m_pVars->m_fGlareThreshold, float, dwValue)
    }
    break;

    // set nightvision parameters
  case SCREENPROCESS_NIGHTVISION:
    switch(iParams)
    {
      //SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bNightVisionActive, bool, dwValue)
      case SCREENPROCESS_ACTIVE:
      //  m_pVars->m_bNightVisionActive=*(bool*)dwValue;
	  
		      if(m_pVars->m_pCVHeatVision) 
		      {
			      m_pVars->m_pCVHeatVision->Set(*((int*) dwValue));
    			  	
						/*
				    // must disable some rendering in order to cryvision function properly
				    if(m_pVars->m_pCVHeatVision->GetIVal()) 
				    {
					    m_pVars->m_iPrevStencilShadows=m_pVars->m_pCVStencilShadows->GetIVal();
					    m_pVars->m_iPrevShadowMaps=m_pVars->m_pCVShadowMaps->GetIVal();
					    m_pVars->m_iPrevVolFog=m_pVars->m_pCVVolFog->GetIVal();
					    //m_pVars->m_iPrevFog=m_pVars->m_pCVFog->GetIVal();

					    m_pVars->m_pCVStencilShadows->Set(0);         
					    m_pVars->m_pCVShadowMaps->Set(0);         
					    m_pVars->m_pCVVolFog->Set(0);
					    //m_pVars->m_pCVFog->Set(0); 
				    }
				    else
				    {
					    m_pVars->m_pCVStencilShadows->Set(m_pVars->m_iPrevStencilShadows);         
					    m_pVars->m_pCVShadowMaps->Set(m_pVars->m_iPrevShadowMaps);         
					    m_pVars->m_pCVVolFog->Set(m_pVars->m_iPrevVolFog);                  
					    //m_pVars->m_pCVFog->Set(m_pVars->m_iPrevFog);
				    }*/
		      }
          break;

      case SCREENPROCESS_NIGHTVISIONCOLORRED:
        m_pVars->m_pNightVisionColor.r=*(float*)dwValue;
        break;
      case SCREENPROCESS_NIGHTVISIONCOLORGREEN:
        m_pVars->m_pNightVisionColor.g=*(float*)dwValue;
        break;
      case SCREENPROCESS_NIGHTVISIONCOLORBLUE:
        m_pVars->m_pNightVisionColor.b=*(float*)dwValue;
        break;
    }
    break;

    // set heatvision parameters
  case SCREENPROCESS_HEATVISION:
    switch(iParams)
    {
		case SCREENPROCESS_ACTIVE:
			if(m_pVars->m_pCVHeatVision) 
			{
				m_pVars->m_pCVHeatVision->Set(*((int*) dwValue));
			}
			break;
		
      //SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bHeatVisionActive, bool, dwValue)
    } 
    break;

    // set flashbang parameters
  case SCREENPROCESS_FLASHBANG:
    switch(iParams)
    {
      case SCREENPROCESS_ACTIVE:
        m_pVars->m_bFlashBangActive= *((bool*) dwValue);
        // reset flash time out
        m_pVars->m_fFlashBangTimeOut=1.0f;
        break;
             
      SET_PARAMETER(SCREENPROCESS_FLASHBANGTIMESCALE, m_pVars->m_fFlashBangTimeScale, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_FLASHBANGFLASHPOSX, m_pVars->m_fFlashBangFlashPosX, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_FLASHBANGFLASHPOSY, m_pVars->m_fFlashBangFlashPosY, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_FLASHBANGFLASHSIZEX, m_pVars->m_fFlashBangFlashSizeX, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_FLASHBANGFLASHSIZEY, m_pVars->m_fFlashBangFlashSizeY, float, dwValue)
      SET_PARAMETER(SCREENPROCESS_FLASHBANGFORCEAFTERIMAGE, m_pVars->m_iFlashBangForce, int, dwValue)      
    }
    break;

    // set cartoon parameters
  case SCREENPROCESS_CARTOON:
    switch(iParams)
    {
      //SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bCartoonActive, bool, dwValue)
      case SCREENPROCESS_ACTIVE:      
        m_pVars->m_bCartoonActive= *((bool*) dwValue);
        break;
    }
    break;

    // set dof parameters
  case  SCREENPROCESS_DOF:
    switch(iParams)
    {
      SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bDofActive, bool, dwValue)
        SET_PARAMETER(SCREENPROCESS_DOFFOCALDISTANCE, m_pVars->m_fDofFocalDistance, float, dwValue)
    }
    break;

    // set screentex parameters
  case SCREENPROCESS_SCREENTEX:
    switch(iParams)
    {
      SET_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bScreenTexActive, bool, dwValue)
    }
    break;

    //default:
    //  break;
  }

  return 0;
}

// get screen processing parameters
void *CREScreenProcess:: mfGetParameter(int iProcess, int iParams)
{
  switch(iProcess)
  {
    // return screen fade parameters
  case SCREENPROCESS_FADE:
    switch(iParams)
    {
      RETURN_PARAMETER(SCREENPROCESS_TRANSITIONTIME, m_pVars->m_fFadeTime)
      RETURN_PARAMETER(SCREENPROCESS_PRETRANSITIONTIME, m_pVars->m_fFadePreTime)
      RETURN_PARAMETER(SCREENPROCESS_FADECOLOR, m_pVars->m_pFadeColor)
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bFadeActive)
    }
    break;

    // return screen blur parameters
  case SCREENPROCESS_BLUR:
    switch(iParams) 
    {
      RETURN_PARAMETER(SCREENPROCESS_BLURAMOUNT, m_pVars->m_fBlurAmount)
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bBlurActive)
      RETURN_PARAMETER(SCREENPROCESS_BLURCOLORRED, m_pVars->m_pBlurColor.r)
      RETURN_PARAMETER(SCREENPROCESS_BLURCOLORGREEN, m_pVars->m_pBlurColor.g)
      RETURN_PARAMETER(SCREENPROCESS_BLURCOLORBLUE, m_pVars->m_pBlurColor.b)
    }
    break;

    // return color transfer parameters
  case SCREENPROCESS_COLORTRANSFER:
    switch(iParams)
    {
      RETURN_PARAMETER(SCREENPROCESS_COLORTRANSFERAMOUNT, m_pVars->m_fColorTransferAmount)
      RETURN_PARAMETER(SCREENPROCESS_COLORTRANSFERCOLOR, m_pVars->m_pColorTransferColor)
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bColorTransferActive)
    }
    break;

    // return motion blur parameters
  case SCREENPROCESS_MOTIONBLUR:
    switch(iParams)
    {
      RETURN_PARAMETER(SCREENPROCESS_MOTIONBLURDISPLACE, m_pVars->m_iMotionBlurDisplace)
      RETURN_PARAMETER(SCREENPROCESS_MOTIONBLURAMOUNT, m_pVars->m_fMotionBlurAmount)
      RETURN_PARAMETER(SCREENPROCESS_MOTIONBLURTYPE, m_pVars->m_iMotionBlurType)
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bMotionBlurActive)
    }
    break;

    // return glare parameters
  case SCREENPROCESS_GLARE:
    switch(iParams)
    {
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bGlareActive)
      RETURN_PARAMETER(SCREENPROCESS_GLAREAMOUNT, m_pVars->m_fGlareAmount)
      RETURN_PARAMETER(SCREENPROCESS_GLARELUMSIZE, m_pVars->m_iGlareLumSize)
      RETURN_PARAMETER(SCREENPROCESS_GLAREMAXAMOUNT, m_pVars->m_fGlareMaxAmount)
      RETURN_PARAMETER(SCREENPROCESS_GLAREBOXSIZE, m_pVars->m_iGlareSize)
      RETURN_PARAMETER(SCREENPROCESS_GLARETHRESHOLD, m_pVars->m_fGlareThreshold)
    }
    break;

    // return nightvision parameters
  case SCREENPROCESS_NIGHTVISION:
    switch(iParams)
    {
		case SCREENPROCESS_ACTIVE:
		  //RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bNightVisionActive)
			  if(m_pVars->m_pCVHeatVision) 
			  {
				  m_pVars->m_iNightVisionActive=m_pVars->m_pCVHeatVision->GetIVal();
			  }
			  return (void*)&m_pVars->m_iNightVisionActive;		
  		break;

      RETURN_PARAMETER(SCREENPROCESS_NIGHTVISIONCOLORRED, m_pVars->m_pNightVisionColor.r)
      RETURN_PARAMETER(SCREENPROCESS_NIGHTVISIONCOLORGREEN, m_pVars->m_pNightVisionColor.g)
      RETURN_PARAMETER(SCREENPROCESS_NIGHTVISIONCOLORBLUE, m_pVars->m_pNightVisionColor.b)
    }
    break;

    // return heatvision parameters
  case SCREENPROCESS_HEATVISION:
    switch(iParams)
    {
		case SCREENPROCESS_ACTIVE:
			  if(m_pVars->m_pCVHeatVision) 
			  {
				  m_pVars->m_iHeatVisionActive=m_pVars->m_pCVHeatVision->GetIVal();
			  }
			  return (void*)&m_pVars->m_iHeatVisionActive;		
		  break;
		//RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bHeatVisionActive)		
    }
    break;

    // return flashbang parameters
  case SCREENPROCESS_FLASHBANG:
    switch(iParams)
    {
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bFlashBangActive)
      RETURN_PARAMETER(SCREENPROCESS_FLASHBANGTIMESCALE, m_pVars->m_fFlashBangTimeScale)
      RETURN_PARAMETER(SCREENPROCESS_FLASHBANGTIMEOUT, m_pVars->m_fFlashBangTimeOut)
      RETURN_PARAMETER(SCREENPROCESS_FLASHBANGFLASHPOSX, m_pVars->m_fFlashBangFlashPosX)
      RETURN_PARAMETER(SCREENPROCESS_FLASHBANGFLASHPOSY, m_pVars->m_fFlashBangFlashPosY)
      RETURN_PARAMETER(SCREENPROCESS_FLASHBANGFLASHSIZEX, m_pVars->m_fFlashBangFlashSizeX)
      RETURN_PARAMETER(SCREENPROCESS_FLASHBANGFLASHSIZEY, m_pVars->m_fFlashBangFlashSizeY)
      RETURN_PARAMETER(SCREENPROCESS_FLASHBANGFORCEAFTERIMAGE, m_pVars->m_iFlashBangForce)      
    }
    break;

    // return cartoon parameters
  case SCREENPROCESS_CARTOON:
    switch(iParams)
    {
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bCartoonActive)
    }
    break;

    // return dof parameters
  case  SCREENPROCESS_DOF:
    switch(iParams)
    {
      RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bDofActive)
        RETURN_PARAMETER(SCREENPROCESS_DOFFOCALDISTANCE, m_pVars->m_fDofFocalDistance)
    }
    break;

    // return screentex parameters
  case SCREENPROCESS_SCREENTEX:
    switch(iParams)
    {
    RETURN_PARAMETER(SCREENPROCESS_ACTIVE, m_pVars->m_bScreenTexActive)
    }
    break;

  default:
    break;
  }

  return 0;
}

// get shaders
void CScreenVars:: Create(void) 
{
#if SHADERSCHECK
  m_pRCBlur = CPShader::mfForName("CGRCBlur");
#ifdef OPENGL
  m_pRCBlurRECT = CPShader::mfForName("CGRCBlurRECT");
#else
  m_pRCBlurRECT = NULL;
#endif
  m_pVPBlur = CVProgram::mfForName("CGVProgBlur");
  m_pRCMotion = CPShader::mfForName("CGRCMotion");
  m_pRCMotionAmount = CPShader::mfForName("CGRCMotionAmount");
  m_pVPMotion = CVProgram::mfForName("CGVProgMotion");
  m_pRCGlareMap = CPShader::mfForName("CGRCGlareMap"); 
  m_pRCGlareAmountMap = CPShader::mfForName("CGRCGlareAmount"); 
  m_pRCGlare  = CPShader::mfForName("CGRCGlare"); 
  m_pRCRenderModeCold = CPShader::mfForName("CGRCRenderModeCold"); 
  m_pRCRenderModeAdv= CPShader::mfForName("CGRCRenderModeAdv"); 
  m_pVPGlare  = CVProgram::mfForName("CGVProgGlare");
  m_pRCColorTransfer = CPShader::mfForName("CGRCColorTransfer"); 
  m_pVPColorTransfer = CVProgram::mfForName("CGVProgColorTransfer");
  m_pRCBluryScreen = CPShader::mfForName("CGRCBluryScreen"); 
  m_pVPBluryScreen = CVProgram::mfForName("CGVProgBluryScreen");
  m_pRCNightVision = CPShader::mfForName("CGRCNightVisionGlare"); 
  m_pVPNightVision = CVProgram::mfForName("CGVProgNightVisionGlare");
  m_pRCHeatVision = CPShader::mfForName("CGRCHeatSourcePass");
  m_pRCHeatSourceDecode = CPShader::mfForName("CGRCHeatSourceDecode");
  m_pVPFlashBang = CVProgram::mfForName("CGVProg_FlashBang");
  m_pRCFlashBang = CPShader::mfForName("CGRCFlashBang");
  m_pVPCartoon = CVProgram::mfForName("CGVProgCartoon");
  m_pRCCartoon = CPShader::mfForName("CGRCCartoon");
  m_pRCCartoonSilhouette = CPShader::mfForName("CGRCCartoonSilhouete");
  m_pRCDof = CPShader::mfForName("CGRCDof");
  m_pRCReplRgbToAlpha = CPShader::mfForName("CGRCReplRgbToAlpha");
#endif    

  if(iConsole)
  {
    m_pCVDisableSfx= iConsole->GetCVar("r_DisableSfx"); 
    m_pCVResetSfx=iConsole->GetCVar("r_ResetScreenFx");
    m_pCVNormalGlare=iConsole->GetCVar("r_Glare"); 
    m_pCVMotionBlur=iConsole->GetCVar("r_MotionBlur");    
    m_pCVScreenColorTransfer=iConsole->GetCVar("r_ScreenColorTransfer");  
    m_pCVMotionBlurAmount=iConsole->GetCVar("r_MotionBlurAmount");
    m_pCVMotionBlurDisplace=iConsole->GetCVar("r_MotionBlurDisplace");
    m_pCVRenderMode=iConsole->GetCVar("r_RenderMode");

    // note: these are used for correct cryvision functioning  
    m_pCVStencilShadows=iConsole->GetCVar("e_stencil_shadows");
    m_iPrevStencilShadows=m_pCVStencilShadows->GetIVal();

    m_pCVShadowMaps=iConsole->GetCVar("e_shadow_maps");
    m_iPrevShadowMaps=m_pCVShadowMaps->GetIVal();

    m_pCVVolFog=iConsole->GetCVar("r_VolumetricFog");
    m_iPrevVolFog= m_pCVVolFog->GetIVal();

    m_pCVFog=iConsole->GetCVar("e_fog");
    m_iPrevFog=m_pCVFog->GetIVal();

    m_pCVMaxTexLodBias=iConsole->GetCVar("r_MaxTexLodBias");
    m_fPrevMaxTexLodBias=m_pCVMaxTexLodBias->GetFVal();

	  m_pCVHeatVision=iConsole->GetCVar("r_Cryvision");
	  m_iHeatVisionActive=m_pCVHeatVision->GetIVal();

    // reset fade amount    
    ICVar *pHudFadeAmount=iConsole->GetCVar("hud_fadeamount");
    if(pHudFadeAmount)
    {
      pHudFadeAmount->Set(1);
    } 
  }

  // fade process vars
  m_bFadeActive=0;
  m_fFadeTime=0;
  m_fFadePreTime=0;
  m_fFadeCurrPreTime=0;
  m_fFadeCurrTime=0;
  m_pFadeColor.set(0,0,0,0);
  m_pFadeCurrColor.set(0,0,0,0);
}

// release/free data..
void CScreenVars:: Release(void) 
{
/*  // check if cryvision is active when exiting game, need to restore console vars..
  if(m_pCVHeatVision)
  {
    if(m_pCVHeatVision->GetIVal()) 
    {
      if(m_pCVStencilShadows)
      {
        m_pCVStencilShadows->Set(m_iPrevStencilShadows);         
      }

      if(m_pCVShadowMaps->GetIVal())
      {
        m_pCVShadowMaps->Set(m_iPrevShadowMaps);         
      }

      if(m_pCVVolFog->GetIVal())
      {
        m_pCVVolFog->Set(m_iPrevVolFog);                  
      }        
    }
  }
*/
}

// reset effects state
void CScreenVars:: Reset(void)
{
  // color transfer vars
  m_bColorTransferActive=0;
  m_pColorTransferColor.set(0,0,0,0);
  m_fColorTransferAmount=1;

  // blury screen vars
  m_bBlurActive=0;
  m_fBlurAmount=1.0f;
  m_pBlurColor.set(1,1,1,1);

  // nightvision vars  
  m_iNightVisionActive=0;
  m_pNightVisionColor.set(-0.1f, 0.2f, 0.11f, 1.0f);

  // heatvision vars
  if(m_pCVHeatVision)
  {	
    //	m_pHeatVision->Set(0);
	  m_iHeatVisionActive=m_pCVHeatVision->GetIVal();
  }
  else
  {
	  m_iHeatVisionActive=0;
  }
  
  // flashbang vars
  m_bFlashBangActive=0;  
  m_fFlashBangTimeScale=1.0f;
  m_fFlashBangTimeOut=1.0f;
  m_fFlashBangFlashPosX=200;
  m_fFlashBangFlashPosY=100;
  m_fFlashBangFlashSizeX=400;
  m_fFlashBangFlashSizeY=400;

  // screentex vars
  m_bScreenTexActive=0;  
  m_pCurrGlareMapConst.set(0.2f, 0.2f, 0.2f, 1.0f);
  m_pCurrSaturation.set(0.0f, 0.0f, 0.0f, 0.2f);
  m_pCurrContrast.set(0.0f, 0.0f, 0.85f, 0.15f);


  // reset fade amount    
  ICVar *pHudFadeAmount=iConsole->GetCVar("hud_fadeamount");
  if(pHudFadeAmount)
  {
    pHudFadeAmount->Set(1);
  } 

  m_bFadeActive=0;
  m_fFadeTime=0;
  m_fFadePreTime=0;
  m_fFadeCurrPreTime=0;
  m_fFadeCurrTime=0;
  m_pFadeColor.set(0,0,0,0);
  m_pFadeCurrColor.set(0,0,0,0);
}