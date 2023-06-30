
#ifndef __CREFLARES_H__
#define __CREFLARES_H__

//=================================================
// Flares

struct SFlareFrame
{
  int mFrame;
  float mDecayTime;
  bool mbVis;
  int mX;
  int mY;
  float mDepth;
  CFColor mColor;
  float mScale;
};

class CREFlareProp;

class CREFlareGeom : public CRendElement
{
  friend class CRender3D;

public:
  SFlareFrame mFlareFr[8];
  CFColor mColor;
  Vec3d mNormal;
  int mLightStyle;
  int mDLightStyle;
  float mBreakTime;
  float mEndBreakTime;

  float mfGetScale(float rtime)
  {
    float scale = 1.0f;

    if (mBreakTime && rtime+0.01>=mBreakTime && mEndBreakTime>rtime)
    {
      float time = rtime - mBreakTime;

      if (time<0.25)
      {
        scale = 1 - time*4;
      }
      else
      {
        time = mEndBreakTime - rtime;
        if (time<1)
          scale= 1 - time;
        else
          scale = 0;
      }
      return scale;
    }
    return -1;
  }

  CREFlareGeom()
  {
    mfSetType(eDATA_FlareGeom);
    mfUpdateFlags(FCEF_TRANSFORM);
  }
  virtual ~CREFlareGeom()
  {
  }

  void mfCheckVis(CFColor &col, CCObject *obj);
  bool mfCullFlare(CCObject *obj, CREFlareProp *fp);
};

enum ELightRGB
{
  eLIGHT_Identity,
  eLIGHT_Fixed,
  eLIGHT_Poly,
  eLIGHT_Style,
  eLIGHT_Object,
};

enum EEmitLight
{
  eEMIT_Poly,
  eEMIT_Point,
};


class CREFlareProp : public CRendElement
{
  friend class CRender3D;

public:
  SShader *FlareShader;
  ELightRGB eLightRGB;
  float LightColorScale;
  CFColor LightColor;
  int LightStyle;
  float offsLight;
  EEmitLight eLightEmit;

  CREFlareProp()
  {
    mfSetType(eDATA_FlareProp);
    mfUpdateFlags(FCEF_TRANSFORM);
    LightColorScale = 1;
    eLightEmit = eEMIT_Point;
    LightStyle = 0;
    FlareShader = 0;
	  eLightRGB = eLIGHT_Identity;
  }
  virtual ~CREFlareProp()
  {
  }
  virtual bool mfCompile(SShader *ef, char *scr);
  virtual CRendElement *mfCreateWorldRE(SShader *ef, SInpData *ds);
};


class CREFlare : public CRendElement
{
  friend class CRender3D;
  friend class CGLRenderer;

public:
  Vec3d mNormal;
  float m_fScaleCorona;
  SParamComp *m_pScaleCoronaParams;
  float m_fMinLight;
  float m_fDistSizeFactor;
  float m_fDistIntensityFactor;
  bool  m_bBlind;
  float m_fSizeBlindBias;
  float m_fSizeBlindScale;
  float m_fIntensBlindBias;
  float m_fIntensBlindScale;
  float m_fFadeTime;
  float m_fVisAreaScale;
  STexPic *m_Map;
  ELightRGB m_eLightRGB;
  int m_LightStyle;
  CFColor m_fColor;
  int m_UpdateFrame;
  int m_nFrameQuery;
  SShaderPassHW *m_Pass;
  int m_Importance;

  CREFlare()
  {
    mfSetType(eDATA_Flare);
    mfUpdateFlags(FCEF_TRANSFORM);
    m_Map = NULL;
    m_fMinLight = 0.0f;
    m_eLightRGB = eLIGHT_Identity;
    m_fColor = Col_White;
    m_UpdateFrame = -1;
    m_fDistSizeFactor = 1.0f;
    m_fDistIntensityFactor = 1.0f;
    m_fSizeBlindBias = 0;
    m_nFrameQuery = -1;
    m_fSizeBlindScale = 1.0f;
    m_fIntensBlindBias = 0;
    m_fIntensBlindScale = 1.0f;
    m_fVisAreaScale = 1.0f;
    m_fFadeTime = -1.0f;
    m_bBlind = false;
    m_fScaleCorona = 0.6f;
    m_pScaleCoronaParams = NULL;
    m_Pass = NULL;
    m_Importance = 1;
  }
  virtual ~CREFlare()
  {
    if (m_Map)
    {
      m_Map->Release(false);
      m_Map = NULL;
    }
    SAFE_DELETE(m_Pass);
  }
  virtual bool mfCompile(SShader *ef, char *scr);
  virtual void mfPrepare(void);
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);

  void mfDrawCorona(SShader *ef, CFColor &col);
  void mfDrawFlares(SShader *ef, CFColor &col);
  bool mfCheckVis(CCObject *obj);
};

#endif  // __CREFLARES_H__
