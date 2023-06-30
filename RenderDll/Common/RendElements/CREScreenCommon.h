/*
=========================================================================
FILE : CRECommon.h
DESC : render elements common stuff
PROJ : Crytek Engine
CODER: Tiago Sousa

NOTE/TODO/FIX:

. All shaders, specialized textures used, should be initialized/destroyed
in CScreenVars class

. All data should be fx/name independent, accessible trought data index
  
. Give better data organization..

Last Update: 13/10/2003
=========================================================================
*/

#ifndef _CRECOMMON_H_
#define _CRECOMMON_H_

//-------------------------------------------------------------------------------------------------
// screen processing types parameter


enum EProcessType
{
  SCREENPROCESS_FADE,
  SCREENPROCESS_BLUR,
  SCREENPROCESS_COLORTRANSFER,
  SCREENPROCESS_CONTRAST,
  SCREENPROCESS_MOTIONBLUR,
  SCREENPROCESS_GLARE,
  SCREENPROCESS_NIGHTVISION,
  SCREENPROCESS_HEATVISION,  
  SCREENPROCESS_FLASHBANG,
  SCREENPROCESS_CARTOON,
  SCREENPROCESS_DOF,
  
  // special case, to get screen texture, after all post-processing
  SCREENPROCESS_SCREENTEX,
};

//-------------------------------------------------------------------------------------------------
// screen processing types parameters

enum EProcessParameter
{
  SCREENPROCESS_FADECOLOR,

  SCREENPROCESS_BLURAMOUNT,
  SCREENPROCESS_BLURCOLORRED,
  SCREENPROCESS_BLURCOLORGREEN,
  SCREENPROCESS_BLURCOLORBLUE,

  SCREENPROCESS_COLORTRANSFERCOLOR,
  SCREENPROCESS_COLORTRANSFERAMOUNT,

  SCREENPROCESS_CONTRASTAMOUNT,

  SCREENPROCESS_MOTIONBLURTYPE,
  SCREENPROCESS_MOTIONBLURAMOUNT,
  SCREENPROCESS_MOTIONBLURDISPLACE,

  SCREENPROCESS_GLAREAMOUNT,
  SCREENPROCESS_GLARELUMSIZE,
  SCREENPROCESS_GLAREMAXAMOUNT,
  SCREENPROCESS_GLAREBOXSIZE,
  SCREENPROCESS_GLARETHRESHOLD,

  SCREENPROCESS_NIGHTVISIONCOLORRED,
  SCREENPROCESS_NIGHTVISIONCOLORGREEN,
  SCREENPROCESS_NIGHTVISIONCOLORBLUE,

  SCREENPROCESS_FLASHBANGTIMESCALE,
  SCREENPROCESS_FLASHBANGFLASHPOSX,
  SCREENPROCESS_FLASHBANGFLASHPOSY,
  SCREENPROCESS_FLASHBANGFLASHSIZEX,
  SCREENPROCESS_FLASHBANGFLASHSIZEY,
  SCREENPROCESS_FLASHBANGTIMEOUT,
  SCREENPROCESS_FLASHBANGFORCEAFTERIMAGE,

  SCREENPROCESS_DOFFOCALDISTANCE,  
  SCREENPROCESS_DOFBLURAMOUNT,

  SCREENPROCESS_TRANSITIONTIME,  
  SCREENPROCESS_PRETRANSITIONTIME,  
  SCREENPROCESS_ACTIVE
};

#define SHADERSCHECK !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)

//-------------------------------------------------------------------------------------------------
// screen processing vars class

class CScreenVars 
{
public:

  CScreenVars()
  {
    // initialize members

    // set shaders to null...
    m_pRCColorTransfer=0;
    m_pVPColorTransfer=0;
    m_pRCMotion=0;
    m_pRCMotionAmount=0;
    m_pVPMotion=0;
    m_pRCBluryScreen=0;
    m_pVPBluryScreen=0;
    m_pRCGlareMap=0;
    m_pRCGlareAmountMap=0;
    m_pRCGlare=0;
    m_pRCRenderModeCold=0;
    m_pRCRenderModeAdv=0;
    m_pVPGlare=0;
    m_pRCNightVision=0;
    m_pVPNightVision=0;
    m_pRCHeatVision=0;
    m_pRCHeatSourceDecode=0;
    m_pRCFlashBang=0;
    m_pVPFlashBang=0;
    m_pRCFlashBang=0;
    m_pVPFlashBang=0;
    m_pRCBlur=0;
    m_pVPBlur=0;
    m_pRCReplRgbToAlpha=0;
    m_pVPCartoon=0;
    m_pRCCartoon=0;
    m_pRCCartoonSilhouette=0;
    m_pRCDof=0;

    // fade process vars
    m_bFadeActive=0;
    m_fFadeTime=0;
    m_fFadePreTime=0;
    m_fFadeCurrPreTime=0;
    m_fFadeCurrTime=0;
    m_pFadeColor.set(0,0,0,0);
    m_pFadeCurrColor.set(0,0,0,0);

    // color transfer vars
    m_bColorTransferActive=0;
    m_pColorTransferColor.set(0,0,0,0);
    m_fColorTransferAmount=1;

    // motion blur vars
    m_bMotionBlurActive=0;
    m_fMotionBlurAmount=0.9f;
    m_iMotionBlurType=1;
    m_iMotionBlurDisplace=0;

    // blury screen vars
    m_bBlurActive=0;
    m_fBlurAmount=1.0f;
    m_pBlurColor.set(1,1,1,1);

    // glare vars
    m_bGlareActive=0;
    m_fGlareThreshold=75;
    m_iGlareBox=2;
    m_iGlareLumSize=2;
    m_iGlareSize=128;
    m_fGlareAmount=1.0f;
    m_fGlareAmountDynamic=1.0f;
    m_fGlareMaxAmount=4.0f;
    m_pCurrGlareMapConst.set(0.2f, 0.2f, 0.2f, 1.0f);
    m_pCurrSaturation.set(0.0f, 0.0f, 0.0f, 0.2f);
    m_pCurrContrast.set(0.0f, 0.0f, 0.85f, 0.15f);

    // nightvision vars
    m_iNightVisionActive=0;
    m_pNightVisionColor.set(-0.1f, 0.2f, 0.11f, 1.0f);
	      
    // heatvision vars
    m_iHeatVisionActive=0;
	  m_pCVHeatVision=0;
        
    // flashbang vars
    m_bFlashBangActive=0;  
    m_iFlashBangForce=0;
    m_fFlashBangTimeScale=1.0f;
    m_fFlashBangTimeOut=1.0f;
    m_fFlashBangFlashPosX=200;
    m_fFlashBangFlashPosY=100;
    m_fFlashBangFlashSizeX=400;
    m_fFlashBangFlashSizeY=400;
    
    // cartoon vars
    m_bCartoonActive=0;

    // depth of field vars
    m_bDofActive=0;
    m_fDofFocalDistance=0;

    // screentex vars
    m_bScreenTexActive=0;

    // console vars
    m_pCVDisableSfx=0; //GET_CVAR("r_DisableSfx");
    m_pCVResetSfx=0; //GET_CVAR("r_ResetScreenFx");
    m_pCVNormalGlare=0; //GET_CVAR("r_Glare"); 
    m_pCVMotionBlur=0; //GET_CVAR("r_MotionBlur");    
    m_pCVScreenColorTransfer=0; //GET_CVAR("r_ScreenColorTransfer");  
    m_pCVMotionBlurAmount=0; //GET_CVAR("r_MotionBlurAmount");
    m_pCVMotionBlurDisplace=0; //GET_CVAR("r_MotionBlurDisplace");
    m_pCVRenderMode=0; //GET_CVAR("r_RenderMode");

    // note: these are used for correct cryvision functioning  
    m_pCVStencilShadows=0; //GET_CVAR("e_stencil_shadows")
    m_iPrevStencilShadows=0;

    m_pCVShadowMaps=0; //GET_CVAR("e_shadow_maps");
    m_iPrevShadowMaps=0;

    m_pCVVolFog=0; //GET_CVAR("r_volumetricfog");
    m_iPrevVolFog=0;

    m_pCVFog=0; //GET_CVAR("e_fog");
    m_iPrevFog=0;
  };

  ~CScreenVars()
  {
    Release();
  };

  // shaders used:

  // color transfer shaders
  void     *m_pRCColorTransfer;
  void     *m_pVPColorTransfer;
  
  // motion blur shaders
  void     *m_pRCMotion,
           *m_pRCMotionAmount;
  void     *m_pVPMotion;

  // blury screen shaders
  void      *m_pRCBluryScreen;
  void      *m_pVPBluryScreen;

  // image enhancement shaders
  void      *m_pRCGlareMap,
            *m_pRCGlareAmountMap,
            *m_pRCGlare,
            *m_pRCRenderModeCold,  
            *m_pRCRenderModeAdv;

  void      *m_pVPGlare;

  // nightvision shaders
  void      *m_pRCNightVision;
  void      *m_pVPNightVision;

  // heatvision shaders
  void      *m_pRCHeatVision,
            *m_pRCHeatSourceDecode;

  // flashbang shaders
  void      *m_pRCFlashBang;
  void      *m_pVPFlashBang;

  // cartoon shaders
  void      *m_pRCCartoon,
            *m_pRCCartoonSilhouette;
  void      *m_pVPCartoon;

  // depth of field shaders
  void      *m_pRCDof;

  // shared shaders
  void      *m_pRCBlur;
  void      *m_pRCBlurRECT;
  void      *m_pVPBlur;
  void      *m_pRCReplRgbToAlpha;

  // fade process vars
  bool          m_bFadeActive;
  float         m_fFadeTime,
                m_fFadeCurrTime,
                m_fFadePreTime,
                m_fFadeCurrPreTime;

  color4f				m_pFadeColor,
                m_pFadeCurrColor;

  // color transfer vars
  bool          m_bColorTransferActive;
  color4f				m_pColorTransferColor;
  float         m_fColorTransferAmount;

  // motion blur vars
  bool          m_bMotionBlurActive;
  float         m_fMotionBlurAmount;
  int           m_iMotionBlurType,
                m_iMotionBlurDisplace;

  // blury screen vars
  bool          m_bBlurActive;
  float         m_fBlurAmount;
  float         m_iBlurWidth,
                m_iBlurHeight;
  color4f				m_pBlurColor;

  // image enhancement vars
  bool          m_bGlareActive;
  int           m_iGlareBox,
                m_iGlareLumSize,
                m_iGlareSize;
  float         m_fGlareAmount,
                m_fGlareThreshold,
                m_fGlareAmountDynamic,
                m_fGlareMaxAmount;
  color4f				m_pCurrGlareMapConst,
                m_pCurrSaturation,
                m_pCurrContrast;
  // nightvision vars
  int           m_iNightVisionActive;
  color4f				m_pNightVisionColor;


  // heatvision vars
  int          m_iHeatVisionActive;
  ICVar		    *m_pCVHeatVision; //GET_CVAR("CV_r_cryvision");
    
  // flashbang vars  
  bool          m_bFlashBangActive;
  int           m_iFlashBangForce;  
  float         m_fFlashBangTimeScale,
                m_fFlashBangTimeOut,
                m_fFlashBangFlashPosX,
                m_fFlashBangFlashPosY,
                m_fFlashBangFlashSizeX,
                m_fFlashBangFlashSizeY;

  // cartoon rendering vars
  bool          m_bCartoonActive;

  // depth of field vars
  bool          m_bDofActive;
  float         m_fDofFocalDistance;

  // screen texture vars
  bool          m_bScreenTexActive;  
  
  // console vars
  ICVar *m_pCVDisableSfx; //GET_CVAR("r_DisableSfx");
  ICVar *m_pCVResetSfx; //GET_CVAR("r_ResetScreenFx");
  ICVar *m_pCVNormalGlare; //GET_CVAR("r_Glare"); 
  ICVar *m_pCVMotionBlur; //GET_CVAR("r_MotionBlur");    
  ICVar *m_pCVScreenColorTransfer; //GET_CVAR("r_ScreenColorTransfer");  
  ICVar *m_pCVMotionBlurAmount; //GET_CVAR("r_MotionBlurAmount");
  ICVar *m_pCVMotionBlurDisplace; //GET_CVAR("r_MotionBlurDisplace");
  ICVar *m_pCVRenderMode; //GET_CVAR("r_RenderMode");
  
  // note: these are used for correct cryvision functioning  
  ICVar *m_pCVStencilShadows; //GET_CVAR("e_stencil_shadows")
  int   m_iPrevStencilShadows;

  ICVar *m_pCVShadowMaps; //GET_CVAR("e_shadow_maps");
  int   m_iPrevShadowMaps;

  ICVar *m_pCVVolFog; //GET_CVAR("r_volumetricfog");
  int   m_iPrevVolFog;

  ICVar *m_pCVFog; //GET_CVAR("e_fog");
  int   m_iPrevFog;

  ICVar *m_pCVMaxTexLodBias; //GET_CVAR("r_MaxTexLodBias");
  float  m_fPrevMaxTexLodBias;

  void Create(void);
  void Release(void);
  void Reset(void);
};

#endif
