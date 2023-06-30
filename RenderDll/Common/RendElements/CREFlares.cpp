/*=============================================================================
	CFlare.cpp : implementation of light coronas and flares RE.
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"

void CopyLightStyle(int dest, int src);

//===============================================================

CRendElement *CREFlareProp::mfCreateWorldRE(SShader *ef, SInpData *ds)
{
  return NULL;
}


bool CREFlareGeom::mfCullFlare(CCObject *obj, CREFlareProp *fp)
{
  return false;
}

//========================================================================================

// Parsing

bool CREFlareProp::mfCompile(SShader *ef, char *scr)
{
  return false;
}

bool CREFlare::mfCompile(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  
  enum {eMap = 1, eScale, eRGBStyle, eBlind, eColor, eDistFactor, eDistIntensityFactor, eMinLight, eSizeBlindScale, eSizeBlindBias, eIntensBlindScale, eIntensBlindBias, eLayer, eImportance, eFadeTime, eVisAreaScale};
  static tokenDesc commands[] =
  {
    {eRGBStyle, "RGBStyle"},
    {eScale, "Scale"},
    {eBlind, "Blind"},
    {eSizeBlindScale, "SizeBlindScale"},
    {eSizeBlindBias, "SizeBlindBias"},
    {eIntensBlindScale, "IntensBlindScale"},
    {eIntensBlindBias, "IntensBlindBias"},    
    {eMinLight, "MinLight"},
    {eDistFactor, "DistFactor"},
    {eDistIntensityFactor, "DistIntensityFactor"},
    {eMap, "Map"},
    {eFadeTime, "FadeTime"},
    {eColor, "Color"},
    {eLayer, "Layer"},
    {eImportance, "Importance"},
    {eVisAreaScale, "VisAreaScale"},
    {0, 0},
  };

  CFColor col;
  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;
      
    switch (cmd)
    {
      case eMap: 
        m_Map = (STexPic *)gRenDev->EF_LoadTexture(data, FT_NORESIZE, 0, eTT_Base);
        break;

      case eColor:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing Color argument for Light Stage in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        shGetColor(data, m_fColor);
        break;

      case eRGBStyle:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing RgbStyle argument for Light Stage in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        if (!stricmp(data, "Poly"))
          m_eLightRGB = eLIGHT_Poly;
        else
        if (!stricmp(data, "Identity"))
          m_eLightRGB = eLIGHT_Identity;
        else
        if (!strnicmp(data, "FromObj", 7) || !stricmp(data, "FromLight"))
          m_eLightRGB = eLIGHT_Object;
        else
        if (!stricmp(data, "LightStyle"))
        {
          m_eLightRGB = eLIGHT_Style;
          m_Color = CFColor(1.0f);
          if (!params || !params[0])
          {
            Warning( 0,0,"missing RgbStyle LightStyle value in Shader '%s' (use 0)\n", ef->m_Name.c_str());
            m_LightStyle = 0;
          }
          else
            m_LightStyle = shGetInt(params);
        }
        else
        if (!stricmp(data, "Fixed"))
        {
          m_eLightRGB = eLIGHT_Fixed;
          if (!params || !params[0])
          {
            Warning( 0,0,"missing RgbStyle Fixed value in Shader '%s' (use 1.0)\n", ef->m_Name.c_str());
            m_Color = CFColor(1.0f);
          }
          else
          {
            CFColor col;
            shGetColor(params, col);
            m_Color = col;
          }
        }
        else
          m_eLightRGB = eLIGHT_Identity;
        break;

      case eScale:
        if (!name)
          m_fScaleCorona = shGetFloat(data);
        else
        {
          TArray<SParam> Params;
          gRenDev->m_cEF.mfCompileParam(params, ef, &Params);
          if (Params.Num())
            m_pScaleCoronaParams = Params[0].m_Comps[0];
        }
        break;

      case eBlind:
        m_bBlind = true;
        break;

      case eImportance:
        m_Importance = shGetInt(data);
        break;

      case eFadeTime:
        m_fFadeTime = shGetFloat(data);
        break;

      case eSizeBlindScale:
        m_fSizeBlindScale = shGetFloat(data);
        break;

      case eSizeBlindBias:
        m_fSizeBlindBias = shGetFloat(data);
        break;

      case eIntensBlindScale:
        m_fIntensBlindScale = shGetFloat(data);
        break;

      case eIntensBlindBias:
        m_fIntensBlindBias = shGetFloat(data);
        break;

      case eMinLight:
        m_fMinLight = shGetFloat(data);
        break;

      case eDistFactor:
        m_fDistSizeFactor = shGetFloat(data);
        break;

      case eVisAreaScale:
        m_fVisAreaScale = shGetFloat(data);
        break;

      case eDistIntensityFactor:
        m_fDistIntensityFactor = shGetFloat(data);
        break;

      case eLayer:
        {
          if (!m_Pass)
            m_Pass = new SShaderPassHW;
          gRenDev->m_cEF.mfCompileLayer(ef, 0, params, m_Pass);
        }
        break;

    }
  }

  return true;
}

void CREFlare::mfPrepare(void)
{
  gRenDev->EF_CheckOverflow(0, 0, this);
  
  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;

  switch(m_eLightRGB)
  {
    case eLIGHT_Identity:
      m_Color = CFColor(1.0f);
      break;
    case eLIGHT_Style:
      {
        if (m_LightStyle>=0 && m_LightStyle<CLightStyle::m_LStyles.Num() && CLightStyle::m_LStyles[m_LightStyle])
        {
          CLightStyle *ls = CLightStyle::m_LStyles[m_LightStyle];
          ls->mfUpdate(gRenDev->m_RP.m_RealTime);
          m_Color = m_fColor * ls->m_fIntensity;
        }
      }
      break;
    case eLIGHT_Object:
      if (gRenDev->m_RP.m_pCurObject)
        m_Color = gRenDev->m_RP.m_pCurObject->m_Color;
      else
        m_Color = CFColor(1.0f);
      break;
  }
}
