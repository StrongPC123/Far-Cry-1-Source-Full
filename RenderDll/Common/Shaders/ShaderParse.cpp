/*=============================================================================
  ShaderParse.cpp : implementation of the Shaders parser part of shaders manager.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "I3DEngine.h"
#include "CryHeaders.h"

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#include <io.h>
#elif defined(LINUX)
#endif



//=================================================================================================

//============================================================
// Compile functions
//============================================================

void CShader::mfCompileFogParms(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;

  enum {eColor = 1, eDensity, eAxis};
  static tokenDesc commands[] =
  {
    {eColor, "Color"},
    {eDensity, "Density"},
    {eAxis, "Axis"},
    {0,0}
  };

  if (!ef->m_FogInfo)
    ef->m_FogInfo = new SFogInfo;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eColor:
        shGetColor(params, ef->m_FogInfo->m_FogColor);
        //COLCONV(ef->FogInfo.FogColor);
        break;

      case eDensity:
        ef->m_FogInfo->m_FogDensity = shGetFloat(params);
        break;

      case eAxis:
        ef->m_FogInfo->m_FogAxis = shGetInt(params);
        break;
    }
  }
}

void CShader::mfCompileRGBAStyle(char *scr, SShader *ef, SShaderPass *sm, bool bRGB)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eStyle=1, eType, eColor};
  static tokenDesc commands[] =
  {
    {eStyle, "Style"},
    {eType, "Type"},
    {eColor, "Color"},
  };

  sm->m_eEvalRGB = eERGB_StyleIntens;

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
      case eStyle:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing Style argument for RGBGen style in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        sm->m_Style = shGetInt(data);
        break;

      case eType:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing RConst argument for RGBGen style in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        if (!strnicmp(data, "Intens", 6))
          sm->m_eEvalRGB = eERGB_StyleIntens;
        else
        if (!stricmp(data, "Color"))
          sm->m_eEvalRGB = eERGB_StyleColor;
        else
        {
          Warning( 0,0,"Warning: unknown Style type for RGBGen style in Shader '%s'\n", ef->m_Name.c_str());
          sm->m_eEvalRGB = eERGB_StyleIntens;
        }
        break;

      case eColor:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing Color argument for RGBGen style in Shader '%s'\n", ef->m_Name.c_str());
          sm->m_FixedColor.dcolor = -1;
          break;
        }
        CFColor col;
        shGetColor(params, col);
        COLCONV(col);
        sm->m_FixedColor.dcolor = col.GetTrue();
        break;

    }
  }
}

void CShader::mfCompileRGBNoise(SRGBGenNoise *cn, char *scr, SShader *ef)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eRRange=1, eRConst, eGRange, eGConst, eBRange, eBConst};
  static tokenDesc commands[] =
  {
    {eRRange, "RRange"},
    {eRConst, "RConst"},
    {eGRange, "GRange"},
    {eGConst, "GConst"},
    {eBRange, "BRange"},
    {eBConst, "BConst"},
  };

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
      case eRRange:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing RRange argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_RangeR = shGetFloat(data);
        break;

      case eRConst:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing RConst argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_ConstR = shGetFloat(data);
        break;

      case eGRange:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing GRange argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_RangeG = shGetFloat(data);
        break;

      case eGConst:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing GConst argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_ConstG = shGetFloat(data);
        break;

      case eBRange:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing BRange argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_RangeB = shGetFloat(data);
        break;

      case eBConst:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing BConst argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_ConstB = shGetFloat(data);
        break;
    }
  }
}

void CShader::mfCompileAlphaNoise(SAlphaGenNoise *cn, char *scr, SShader *ef)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eAlphaRange=1, eAlphaConst};
  static tokenDesc commands[] =
  {
    {eAlphaRange, "AlphaRange"},
    {eAlphaConst, "AlphaConst"},
  };

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
      case eAlphaRange:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing AlphaRange argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_RangeA = shGetFloat(data);
        break;

      case eAlphaConst:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing AlphaConst argument for AlphaNoise in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        cn->m_ConstA = shGetFloat(data);
        break;
    }
  }
}

void CShader::mfCompileWaveForm(SWaveForm *wf, char *scr)
{
  char* name;
  long cmd;
  char *params;

  enum {eType=1, eLevel, eAmp, ePhase, eFreq, eClamp};
  static tokenDesc commands[] =
  {
    {eType, "Type"},
    {eLevel, "Level"},
    {eAmp, "Amp"},
    {ePhase, "Phase"},
    {eFreq, "Freq"},
    {eClamp, "Clamp"},
    {0,0}
  };

  wf->m_Flags = 0;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eType:
        if (!stricmp(params, "Sin"))
          wf->m_eWFType = eWF_Sin;
        else
        if (!stricmp(params, "HalfSin"))
          wf->m_eWFType = eWF_HalfSin;
        else
        if (!stricmp(params, "InvHalfSin"))
          wf->m_eWFType = eWF_InvHalfSin;
        else
        if (!stricmp(params, "Square"))
          wf->m_eWFType = eWF_Square;
        else
        if (!stricmp(params, "Triangle"))
          wf->m_eWFType = eWF_Triangle;
        else
        if (!stricmp(params, "SawTooth"))
          wf->m_eWFType = eWF_SawTooth;
        else
        if (!stricmp(params, "InverseSawTooth") || !stricmp(params, "InvSawTooth"))
          wf->m_eWFType = eWF_InvSawTooth;
        else
        if (!stricmp(params, "Hill"))
          wf->m_eWFType = eWF_Hill;
        else
        if (!stricmp(params, "InverseHill") || !stricmp(params, "InvHill"))
          wf->m_eWFType = eWF_InvHill;
        else
          wf->m_eWFType = eWF_None;
        break;

      case eLevel:
        shGetFloat(params, &wf->m_Level, &wf->m_Level1);
        break;

      case eAmp:
        shGetFloat(params, &wf->m_Amp, &wf->m_Amp1);
        break;

      case ePhase:
        shGetFloat(params, &wf->m_Phase, &wf->m_Phase1);
        break;

      case eFreq:
        shGetFloat(params, &wf->m_Freq, &wf->m_Freq1);
        break;

      case eClamp:
        wf->m_Flags |= WFF_CLAMP;
        break;
    }
  }
  if (wf->m_Freq1 != wf->m_Freq || wf->m_Amp != wf->m_Amp1 || wf->m_Level != wf->m_Level1 || wf->m_Phase != wf->m_Phase1)
    wf->m_Flags |= WFF_LERP;
}

void CShader::mfCompileDeform(SShader *ef, SDeform *df, char *dname, char *scr)
{
  char* name;
  long cmd;
  char *params;
  float f;

  enum {eDiv=1, eDeformGen, eFlareSize};
  static tokenDesc commands[] =
  {
    {eDiv, "Div"},
    {eDeformGen, "DeformGen"},
    {eFlareSize, "FlareSize"}
  };

  EDeformType eType;
  if (!stricmp(dname, "Bulge"))
    eType = eDT_Bulge;
  else
  if (!stricmp(dname, "Wave"))
    eType = eDT_Wave;
  else
  if (!stricmp(dname, "Flare"))
  {
    ef->m_Flags3 |= EF3_SHAREVERTS;
    eType = eDT_Flare;
  }
  else
  if (!stricmp(dname, "Beam"))
    eType = eDT_Beam;
  else
  if (!stricmp(dname, "VerticalWave"))
    eType = eDT_VerticalWave;
  else
  if (!stricmp(dname, "Squeeze"))
    eType = eDT_Squeeze;
  else
  if (!stricmp(dname, "FromCenter"))
    eType = eDT_FromCenter;
  else
  {
    Warning( 0,0,"Warning: Unknown deform type %s in Shader '%s' (skipping)\n", dname, ef->m_Name.c_str());
    return;
  }

  df->m_eType = eType;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eDiv:
        f = shGetFloat(params);
        if (f)
        {
          df->m_ScaleVerts = 1.0f / (f/100.0f);
        }
        else
        {
          df->m_ScaleVerts = 1.0f;
          Warning( 0,0,"Warning: illegal div value (0) in DeformVertexes command for Shader '%s'\n", ef->m_Name.c_str());
        }
        break;

      case eDeformGen:
        mfCompileWaveForm(&df->m_DeformGen, params);
        df->m_DeformGen.m_Level /= 100.0f;
        df->m_DeformGen.m_Amp /= 100.0f;
        break;

      case eFlareSize:
        {
          df->m_fFlareSize = shGetFloat(params);
        }
        break;
    }
  }
}

void CShader::mfParseLightStyle(CLightStyle *ls, char *lstr)
{
  int i;
  char str[64], *pstr1, *pstr2;
  CFColor col;

  col = CFColor(0.0f);

  ls->m_Map.Free();

  int n = 0;
  while (true)
  {
    pstr1 = strchr(lstr, '|');
    if (!pstr1)
      break;
    pstr2 = strchr(pstr1+1, '|');
    if (!pstr2)
      break;
    if (pstr2-pstr1-1 > 0)
    {
      strncpy(str, pstr1+1, pstr2-pstr1-1);
      str[pstr2-pstr1-1] = 0;
      i = sscanf(str, "%f %f %f %f", &col[0], &col[1], &col[2], &col[3]);
      switch (i)
      {
        default:
          continue;

        case 1:
          col[1] = col[2] = col[0];
          col[3] = 1.0f;
          break;

        case 2:
          col[2] = 1.0f;
          col[3] = 1.0f;
          break;

        case 3:
          col[3] = 1.0f;
          break;
      }
      ls->m_Map.AddElem(col);
      n++;
    }

    lstr = pstr2;
  }
  ls->m_Map.Shrink();
  assert(ls->m_Map.Num() == n);
}

bool CShader::mfCompileLightStyle(SShader *ef, int num, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eValueString=1, eSpeed};
  static tokenDesc commands[] =
  {
    {eValueString, "ValueString"},
    {eSpeed, "Speed"},
  };

  ef->m_Flags |= EF_LIGHTSTYLE;
  if (CLightStyle::m_LStyles.Num() <= num)
    CLightStyle::m_LStyles.ReserveNew(num+1);
  CLightStyle *ls = CLightStyle::m_LStyles[num];
  if (!ls)
  {
    ls = new CLightStyle;
    ls->m_LastTime = 0;
    ls->m_Color = Col_White;
    CLightStyle::m_LStyles[num] = ls;
  }
  ls->m_TimeIncr = 60;


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
      case eValueString:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing ValueString argument for LightStyle %i in Shader '%s'\n", num, ef->m_Name.c_str());
          break;
        }
        mfParseLightStyle(ls, data);
        break;

      case eSpeed:
        ls->m_TimeIncr = shGetFloat(data);
        break;
    }
  }

  return true;
}

void CShader::mfCheckObjectDependParams(TArray<SCGParam4f>* PNoObj, TArray<SCGParam4f>* PObj)
{
#ifdef PIPE_USE_INSTANCING
  if (!PNoObj)
    return;
  int i, j;
  for (i=0; i<PNoObj->Num(); i++)
  {
    for (j=0; j<4; j++)
    {
      if (PNoObj->Get(i).m_Comps[j] && PNoObj->Get(i).m_Comps[j]->m_bDependsOnObject)
        break;
    }
    if (j != 4)
    {
      PObj->AddElem(PNoObj->Get(i));
      PNoObj->Remove(i);
      i--;
    }
  }
  PNoObj->Shrink();
  PObj->Shrink();
#endif
}

void CShader::mfCheckObjectDependParams(TArray<SParam>* PNoObj, TArray<SParam>* PObj)
{
#ifdef PIPE_USE_INSTANCING
  int i, j;
  for (i=0; i<PNoObj->Num(); i++)
  {
    for (j=0; j<4; j++)
    {
      if (PNoObj->Get(i).m_Comps[j] && PNoObj->Get(i).m_Comps[j]->m_bDependsOnObject)
        break;
    }
    if (j != 4)
    {
      PObj->AddElem(PNoObj->Get(i));
      PNoObj->Remove(i);
      i--;
    }
  }
#endif
}

SShaderGenBit *CShader::mfCompileShaderGenProperty(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  SShaderGenBit *shgm = new SShaderGenBit;

  enum {eName=1, eProperty, eDescription, eMask, eHidden};
  static tokenDesc commands[] =
  {
    {eName, "Name"},
    {eProperty, "Property"},
    {eDescription, "Description"},
    {eMask, "Mask"},
    {eHidden, "Hidden"},

    {0,0}
  };

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
      case eName:
        shgm->m_ParamName = data;
        break;

      case eProperty:
        shgm->m_ParamProp = data;
        break;

      case eDescription:
        shgm->m_ParamDesc = data;
        break;

      case eHidden:
        shgm->m_Flags |= SHGF_HIDDEN;
        break;

      case eMask:
        if (data && data[0])
        {
          if (data[0] == '0' && (data[1] == 'x' || data[1] == 'X'))
            shgm->m_Mask = shGetHex64(&data[2]);
          else
            shgm->m_Mask = shGetInt(data);
        }
        break;
    }
  }

  return shgm;
}

bool CShader::mfCompileShaderGen(SShader *ef, SShaderGen *shg, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  SShaderGenBit *shgm;

  enum {eProperty=1, eVersion};
  static tokenDesc commands[] =
  {
    {eProperty, "Property"},
    {eVersion, "Version"},

    {0,0}
  };

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
      case eProperty:
        shgm = mfCompileShaderGenProperty(ef, params);
        if (shgm)
          shg->m_BitMask.AddElem(shgm);
        break;

      case eVersion:
        break;
    }
  }

  return shg->m_BitMask.Num() != 0;
}

bool CShader::mfCompileParams(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  int i;
  char *data;

  enum {eNoMipmaps=1, eRefracted, eDontSortByDistance, ePolygonOffset, eOverlay, eTemplate, eFogParms, eFogGen, eNoCastShadows, ePortal, eGlare, eHeatMap, eNightMap, eRainMap, eDeformVertexes, eTessSize, eSkyBox, eSkyHDR, eCull, eFogShader, eFogOnly, eSort, eNoSpots, eDetail, eNoDetail, eNormal, eBumpScale, eBumpOffset, eColorMaterial, eShadowMapGen, eNoDraw, eLMIgnoreLights, eLMIgnoreProjLights, eLMBump, eClipPlane, eReflection, eIgnoreDirectionalLight, eNoLight, eHasVColors, eHasAlphaTest, eHasAlphaBlend, eUseParentSort, eUseParentCull,   eMotionBlurMap, eScreenProcess, eScreenLuminosityMap, eFlashBangMap, eScreenTexture, eIgnoreResourceState, eUseLightMaterial, eSkyLayerHeight, eDofMap, eDefaultVertexFormat, eOffsetBump, eAllow3DC};
  static tokenDesc commands[] =
  {
    {eNoDraw, "NoDraw"},
    {eNoLight, "NoLight"},
    {eNoMipmaps, "NoMipmaps"},
    {eNoSpots, "NoSpots"},
    {eNoCastShadows, "NoCastShadows"},
    {ePolygonOffset, "PolygonOffset"},
    {eOverlay, "Overlay"},
    {eFogParms, "FogParms"},
    {eFogGen, "FogGen"},
    {ePortal, "Portal"},
    {eTemplate, "Template"},
    {eGlare, "Glare"},
    {eDontSortByDistance, "DontSortByDistance"},
    {eHeatMap, "HeatMap"},
    {eNightMap, "NightMap"},
    {eIgnoreResourceState, "IgnoreResourceState"},

    {eRainMap, "RainMap"},
    {eDeformVertexes, "DeformVertexes"},
    {eTessSize, "TessSize"},
    {eSkyBox, "SkyBox"},
    {eSkyHDR, "SkyHDR"},
    {eSkyLayerHeight, "SkyLayerHeight"},
    {eCull, "Cull"},
    {eFogShader, "FogShader"},
    {eFogOnly, "FogOnly"},
    {eSort, "Sort"},
    {eDetail, "Detail"},
    {eNoDetail, "NoDetail"},
    {eClipPlane, "ClipPlane"},
    {eReflection, "Reflection"},
    {eNormal, "Normal"},
    {eBumpScale, "BumpScale"},
    {eBumpOffset, "BumpOffset"},
    {eColorMaterial, "ColorMaterial"},
    {eShadowMapGen, "ShadowMapGen"},
    {eRefracted, "Refracted"},

    {eLMBump, "LMBump"},
    {eLMIgnoreLights, "LMIgnoreLights"},
    {eLMIgnoreProjLights, "LMIgnoreProjLights"},

    {eIgnoreDirectionalLight, "IgnoreDirectionalLight"},

    {eHasVColors, "HasVColors"},
    {eHasAlphaTest, "HasAlphaTest"},
    {eHasAlphaBlend, "HasAlphaBlend"},
    {eUseParentSort, "UseParentSort"},
    {eUseParentCull, "UseParentCull"},

    {eUseLightMaterial, "UseLightMaterial"},

    // tiago: added
    {eMotionBlurMap, "MotionBlurMap"},
    {eScreenProcess, "ScreenProcess"},
    {eScreenLuminosityMap, "ScreenLuminosityMap" },
    {eFlashBangMap, "FlashBangMap"},
    {eScreenTexture, "ScreenTexture"},
    {eDofMap, "DofMap"},
    {eDefaultVertexFormat, "DefaultVertexFormat"},
    {eOffsetBump, "OffsetBump"},
    {eAllow3DC, "Allow3DC"},

    {0,0}
  };

  float fSkyLayerHeight = 0;

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
      case eLMBump:
        ef->m_LMFlags |= LMF_BUMPMATERIAL;
        break;

      case eLMIgnoreLights:
        ef->m_LMFlags |= LMF_IGNORELIGHTS;
        break;

      case eLMIgnoreProjLights:
        ef->m_LMFlags |= LMF_IGNOREPROJLIGHTS;
        break;

      case eHasAlphaTest:
        ef->m_Flags3 |= EF3_HASALPHATEST;
        break;

      case eOffsetBump:
        ef->m_Flags |= EF_OFFSETBUMP;
        break;

      case eGlare:
        break;

      case eAllow3DC:
        ef->m_Flags |= EF_ALLOW3DC;
        break;

      case eUseLightMaterial:
        ef->m_Flags2 |= EF2_USELIGHTMATERIAL;
        break;

      case eDontSortByDistance:
        ef->m_Flags2 |= EF2_DONTSORTBYDIST;
        break;

      case eIgnoreResourceState:
        ef->m_Flags2 |= EF2_IGNORERESOURCESTATES;
        break;

      case eDefaultVertexFormat:
        ef->m_Flags2 |= EF2_DEFAULTVERTEXFORMAT;
        break;

      case eHasAlphaBlend:
        ef->m_Flags3 |= EF3_HASALPHABLEND;
        break;

      case eHasVColors:
        ef->m_Flags3 |= EF3_HASVCOLORS;
        break;

      case eUseParentSort:
        ef->m_Flags3 |= EF3_USEPARENTSORT;
        break;

      case eUseParentCull:
        ef->m_Flags3 |= EF3_USEPARENTCULL;
        break;

      case eNoDraw:
        ef->m_Flags3 |= EF3_NODRAW;
        break;

      case eNoLight:
        ef->m_LMFlags |= LMF_DISABLE;
        break;

      case eScreenTexture:
        ef->m_Flags3 |= EF3_SCREENTEXTURE;
        break;

      case eIgnoreDirectionalLight:
        ef->m_Flags3 |= EF3_IGNOREDIRECTIONALLIGHT;
        break;

      case eReflection:
        ef->m_Flags3 |= EF3_REFLECTION;
        break;

      case eClipPlane:
        if (!stricmp(data, "Back"))
          ef->m_Flags3 |= EF3_CLIPPLANE_BACK;
        else
        if (!stricmp(data, "Front"))
          ef->m_Flags3 |= EF3_CLIPPLANE_FRONT;
        else
        if (!stricmp(data, "WaterFront"))
          ef->m_Flags3 |= EF3_CLIPPLANE_WATER_FRONT;
        else
        if (!stricmp(data, "WaterBack"))
          ef->m_Flags3 |= EF3_CLIPPLANE_WATER_BACK;
        else
          ef->m_Flags3 |= EF3_CLIPPLANE_FRONT;
        break;

      case eNoMipmaps:
        ef->m_Flags |= EF_NOMIPMAPS;
        break;

      case eTemplate:
        ef->m_Flags |= EF_TEMPLNAMES;
        break;

      case eNoSpots:
        ef->m_Flags |= EF_NOSPOTS;
        break;

      case eNoCastShadows:
        ef->m_Flags2 |= EF2_NOCASTSHADOWS;
        break;

      case eNoDetail:
        ef->m_Flags3 |= EF3_NODETAIL;
        break;

      case eShadowMapGen:
        ef->m_nPreprocess |= FSPR_SHADOWMAPGEN;
        break;

      case eRefracted:
        ef->m_nPreprocess |= FSPR_REFRACTED;
        break;

      case ePolygonOffset:
        ef->m_Flags |= EF_POLYGONOFFSET;
        break;

      case eOverlay:
        ef->m_Flags |= EF_OVERLAY;
        break;

      case eFogParms:
        mfCompileFogParms(ef, params);
        break;

      case eFogGen:
        if (!ef->m_FogInfo)
          ef->m_FogInfo = new SFogInfo;
        mfCompileWaveForm(&ef->m_FogInfo->m_WaveFogGen, params);
        break;

      case ePortal:
        ef->m_nPreprocess |= FSPR_PORTAL;
        break;

      case eHeatMap:
        ef->m_nPreprocess |= FSPR_HEATVIS;
        break;

      // tiago: changed preprocessing..
      case eNightMap:
        ef->m_nPreprocess |= FSPR_SCREENTEXMAP; // ePR_NightVis;
        break;

      case eDofMap:
        ef->m_nPreprocess |= FSPR_DOFMAP;       
        break; 

      case eRainMap:
        ef->m_nPreprocess |= FSPR_RAINOVERLAY;
        break;


      case eFlashBangMap:
        ef->m_nPreprocess |= FSPR_FLASHBANG;
        break;

      case eDeformVertexes:
        {
          if (!ef->m_Deforms)
            ef->m_Deforms = new TArray<SDeform>;
          int i = ef->m_Deforms->Num();
          ef->m_Deforms->ReserveNew(i+1);
          SDeform *df = &ef->m_Deforms->Get(i);
          mfCompileDeform(ef, df, name, params);
        }
        break;

      case eTessSize:
        ef->m_Flags2 |= EF2_TESSSIZE;
        break;

      case eSkyLayerHeight:
        fSkyLayerHeight = shGetFloat(data);
        break;

      case eSkyHDR:
        ef->m_Flags |= EF_SKY_HDR;
        break;

      case eSkyBox:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing SkyBox argument in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        if (!ef->m_Sky)
          ef->m_Sky = new SSkyInfo;

        ef->m_Flags |= EF_SKY;

        for (i=0; i<3; i++)
        {
          static char *skypostfix[6] = {"12", "34", "5"};
          char nsky[64];

          sprintf(nsky, "%s_%s", data, skypostfix[i]);
          ef->m_Sky->m_SkyBox[i] = (STexPic*)gRenDev->EF_LoadTexture(nsky, FT_SKY, FT2_NODXT, eTT_Base);
        }
        break;

      case eCull:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing Cull argument in Shader '%s'\n", ef->m_Name.c_str());
          ef->m_eCull = eCULL_Back;
          break;
        }
        ef->m_Flags |= EF_HASCULL;
        if (!stricmp(data, "None") || !stricmp(data, "TwoSided") || !stricmp(data, "Disable"))
          ef->m_eCull = eCULL_None;
        else
        if (!strnicmp(data, "Back", 4))
          ef->m_eCull = eCULL_Back;
        else
        if (!strnicmp(data, "Front", 5))
          ef->m_eCull = eCULL_Front;
        else
          Warning( 0,0,"Warning: invalid Cull parm '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eDetail:
        break;

      case eFogShader:
      case eFogOnly:
        ef->m_Flags |= EF_FOGSHADER;
        break;

      case eNormal:
        if (!ef->m_NormGen)
          ef->m_NormGen = new SNormalsGen;
        if (!stricmp(data, "Custom"))
        {
          ef->m_NormGen->m_eNormal = eNORM_Custom;
          if (!params || !params[0])
          {
            Warning( 0,0,"Warning: missing RgbStyle Fixed value in Shader '%s' (use 1.0)\n", ef->m_Name.c_str());
            ef->m_NormGen->m_CustomNormal = Vec3d(1, 0, 0);
          }
          else
          {
            shGetVector(params, ef->m_NormGen->m_CustomNormal);
          }
        }
        else
        if (!stricmp(data, "Wave"))
        {
          ef->m_NormGen->m_eNormal = eNORM_Wave;
          mfCompileWaveForm(&ef->m_NormGen->m_WaveEvalNormal, params);
        }
        else
        if (!strnicmp(data, "Front", 5))
          ef->m_NormGen->m_eNormal = eNORM_Front;
        else
        if (!strnicmp(data, "Back", 4))
          ef->m_NormGen->m_eNormal = eNORM_Back;
        else
        if (!stricmp(data, "Edge"))
          ef->m_NormGen->m_eNormal = eNORM_Edge;
        else
        if (!stricmp(data, "InvEdge"))
          ef->m_NormGen->m_eNormal = eNORM_InvEdge;
        break;

      case eSort:
        if (!stricmp(data, "ZBuff"))
          ef->m_eSort = eS_ZBuff;
        else
        if (!stricmp(data, "Portal"))
          ef->m_nPreprocess |= FSPR_PORTAL;
        else
        if (!stricmp(data, "Stencil"))
          ef->m_eSort = eS_Stencil;
        else
        if (!stricmp(data, "Terrain"))
          ef->m_eSort = eS_Terrain;
        else
        if (!stricmp(data, "TerrainShadowPass"))
          ef->m_eSort = eS_TerrainShadowPass;
        else
        if (!stricmp(data, "TerrainLightPass"))
          ef->m_eSort = eS_TerrainLightPass;
        else
        if (!stricmp(data, "TerrainDetailTextures"))
          ef->m_eSort = eS_TerrainDetailTextures;
        else
        if (!stricmp(data, "WaterBeach"))
          ef->m_eSort = eS_WaterBeach;
        else
        if (!stricmp(data, "TerrainDetailObjects"))
          ef->m_eSort = eS_TerrainDetailObjects;
        else
        if (!stricmp(data, "TerrainParticles"))
          ef->m_eSort = eS_Particles;
        else
        if (!stricmp(data, "Sky"))
          ef->m_eSort = eS_Sky;
        else
        if (!stricmp(data, "Opaque"))
          ef->m_eSort = eS_Opaque;
        else
        if (!strnicmp(data, "Tree", 4))
          ef->m_eSort = eS_Trees;
        else
        if (!strnicmp(data, "Sprite", 6))
          ef->m_eSort = eS_Sprites;
        else
        if (!stricmp(data, "Decal"))
          ef->m_eSort = eS_Decal;
        else
        if (!stricmp(data, "SeeThrough"))
          ef->m_eSort = eS_SeeThrough;
        else
        if (!stricmp(data, "Shadowmap"))
          ef->m_eSort = eS_ShadowMap;
        else
        if (!stricmp(data, "Banner"))
          ef->m_eSort = eS_Banner;
        else
        if (!stricmp(data, "UnderWater"))
          ef->m_eSort = eS_UnderWater;
        else
        if (!stricmp(data, "MuzzleFlash"))
          ef->m_eSort = eS_MuzzleFlash;
        else
        if (!stricmp(data, "Water"))
          ef->m_eSort = eS_Water;
        else
        if (!stricmp(data, "Additive"))
          ef->m_eSort = eS_Additive;
        else
        if (!stricmp(data, "Nearest"))
          ef->m_eSort = eS_Nearest;
        else
        if (!stricmp(data, "OcclusionTest"))
          ef->m_eSort = eS_OcclusionTest;
        else
        if (!stricmp(data, "HeatVision"))
          ef->m_eSort = eS_HeatVision;
        else
        if (!stricmp(data, "Glare"))
          ef->m_eSort = eS_Glare;
        else
        if (!stricmp(data, "HDR"))
          ef->m_eSort = eS_HDR;
        else
        if (!strnicmp(data, "Refract", 7))
          ef->m_eSort = eS_Refractive;
        else
        if (!stricmp(data, "Preprocess"))
          ef->m_eSort = eS_PreProcess;
        else
        {
          if (!isdigit(data[0]))
            Warning( 0,0,"Warning: Bad Sort value '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
          else
          {
            i = shGetInt(data);
            if (i <= 0 || i >= eS_Max)
              Warning( 0,0,"Warning: Sort value '%s' out of range in Shader '%s'\n", data, ef->m_Name.c_str());
            else
              ef->m_eSort = (EF_Sort)i;
          }
        }
        break;

      case eBumpScale:
        //ef->m_fBumpScale = shGetFloat(data);
        break;

      case eBumpOffset:
        //ef->m_fBumpOffset = shGetFloat(data);
        break;

      case eColorMaterial:
        ef->m_LMFlags |= LMF_COLMAT_AMB;
        break;

      default:
        Warning( 0,0,"Warning: Unknown general Shader parameter '%s' in '%s'\n", params, ef->m_Name.c_str());
        return false;
    }
  }
  if (ef->m_Sky)
    ef->m_Sky->m_fSkyLayerHeight = fSkyLayerHeight;

  return true;
}

bool CShader::mfCompilePublic(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eByte=1, eShort, eInt, eFloat, eString, eColor, eVector};
  static tokenDesc commands[] =
  {
    {eByte, "Byte"},
    {eShort, "Short"},
    {eInt, "Int"},
    {eFloat, "Float"},
    {eString, "String"},
    {eColor, "Color"},
    {eVector, "Vector"},

    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    SShaderParam pr;
    pr.m_Type = eType_UNKNOWN;

    switch (cmd)
    {
      case eByte:
        pr.m_Type = eType_BYTE;
        strlwr(name);
        strncpy(pr.m_Name, name, 32);
        pr.m_Value.m_Byte = shGetInt(params);
        break;

      case eShort:
        pr.m_Type = eType_SHORT;
        strlwr(name);
        strncpy(pr.m_Name, name, 32);
        pr.m_Value.m_Short = shGetInt(params);
        break;

      case eInt:
        pr.m_Type = eType_INT;
        strlwr(name);
        strncpy(pr.m_Name, name, 32);
        pr.m_Value.m_Int = shGetInt(params);
        break;

      case eFloat:
        pr.m_Type = eType_FLOAT;
        strlwr(name);
        strncpy(pr.m_Name, name, 32);
        pr.m_Value.m_Float = shGetFloat(params);
        break;

      case eColor:
        pr.m_Type = eType_FCOLOR;
        strlwr(name);
        strncpy(pr.m_Name, name, 32);
        shGetColor(params, pr.m_Value.m_Color);
        break;

      case eVector:
        pr.m_Type = eType_VECTOR;
        strlwr(name);
        strncpy(pr.m_Name, name, 32);
        shGetVector(params, pr.m_Value.m_Vector);
        break;

      case eString:
        {
          pr.m_Type = eType_STRING;
          strlwr(name);
          strncpy(pr.m_Name, name, 32);

          size_t len = strlen(params)+1;
          char *str = new char[len];
          strcpy(str, params);
          pr.m_Value.m_String = str;
        }
        break;

      default:
        Warning( 0,0,"Warning: Unknown general Shader public parameter '%s' in '%s'\n", params, ef->m_Name.c_str());
        return false;
    }
    if (pr.m_Type != eType_UNKNOWN)
      ef->m_PublicParams.AddElem(pr);
  }

  return true;
}


bool CShader::mfCompileRenderParams(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eDrawWater=1, eDrawTerrain, eDrawPlayer, eDrawDetailTextures, eDrawDetailObjects, eDrawIndoors, eDrawFarSprites, eDrawStaticObjects, eDrawEntities, eDrawParticles, eUseLights, eUpdateFactor, eFullDetailTerrain};
  static tokenDesc commands[] =
  {
    {eDrawWater, "DrawWater"},
    {eDrawPlayer, "DrawPlayer"},
    {eDrawTerrain, "DrawTerrain"},
    {eDrawDetailTextures, "DrawDetailTextures"},
    {eDrawDetailObjects, "DrawDetailObjects"},
    {eDrawIndoors, "DrawIndoors"},
    {eDrawFarSprites, "DrawFarSprites"},
    {eDrawStaticObjects, "DrawStaticObjects"},
    {eDrawEntities, "DrawEntities"},
    {eDrawParticles, "DrawParticles"},
    {eUseLights, "UseLights"},
    {eUpdateFactor, "UpdateFactor"},
    {eFullDetailTerrain, "FullDetailTerrain"},

    {0,0}
  };

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
      case eDrawWater:
        ef->m_DLDFlags |= DLD_TERRAIN_WATER;
        break;

      case eDrawPlayer:
        ef->m_DLDFlags |= DLD_FIRST_PERSON_CAMERA_OWNER;
        break;

      case eDrawTerrain:
        ef->m_DLDFlags |= DLD_TERRAIN;
        break;

      case eDrawDetailTextures:
        ef->m_DLDFlags |= DLD_DETAIL_TEXTURES;
        break;

      case eDrawDetailObjects:
        ef->m_DLDFlags |= DLD_DETAIL_OBJECTS;
        break;

      case eFullDetailTerrain:
        ef->m_DLDFlags |= DLD_TERRAIN_FULLRES;
        break;

      case eDrawIndoors:
//        ef->m_DLDFlags |= DLD_INDOORS;
        break;

      case eDrawFarSprites:
        ef->m_DLDFlags |= DLD_FAR_SPRITES;
        break;

      case eDrawStaticObjects:
        ef->m_DLDFlags |= DLD_STATIC_OBJECTS;
        break;

      case eDrawEntities:
        ef->m_DLDFlags |= DLD_ENTITIES;
        break;

      case eDrawParticles:
        ef->m_DLDFlags |= DLD_PARTICLES;
        break;

      case eUseLights:
//        ef->m_DLDFlags |= DLD_ADD_LIGHTSOURCES;
        break;

      case eUpdateFactor:
        ef->m_fUpdateFactor = shGetFloat(data);
        break;

      default:
        Warning( 0,0,"Warning: Unknown general Shader render parameter '%s' in '%s'\n", params, ef->m_Name.c_str());
        return false;
    }
  }

  return true;
}

void sGetBlend (char *data, SShader *ef, uint *src, uint *dst)
{
  char *token;
  char sbfsrc[32];
  char sbfdst[32];
  int bfsrc, bfdst;

  token = strtok(data, " ");
  if (!token)
  {
    Warning( 0,0,"Warning: invalid Blend name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
    return;
  }
  strcpy(sbfsrc, token);
  token = strtok(NULL, " ");
  if (!token)
  {
    Warning( 0,0,"Warning: invalid Blend name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
    return;
  }
  strcpy(sbfdst, token);

  // find SRC factor
  if (!stricmp(sbfsrc, "ONE"))
    bfsrc = GS_BLSRC_ONE;
  else
  if (!stricmp(sbfsrc, "ZERO"))
    bfsrc = GS_BLSRC_ZERO;
  else
  if (!stricmp(sbfsrc, "DST_COLOR"))
    bfsrc = GS_BLSRC_DSTCOL;
  else
  if (!stricmp(sbfsrc, "ONE_MINUS_DST_COLOR"))
    bfsrc = GS_BLSRC_ONEMINUSDSTCOL;
  else
  if (!stricmp(sbfsrc, "SRC_ALPHA") || !stricmp(sbfsrc, "SrcAlpha"))
    bfsrc = GS_BLSRC_SRCALPHA;
  else
  if (!stricmp(sbfsrc, "ONE_MINUS_SRC_ALPHA") || !stricmp(sbfsrc, "InvSrcAlpha"))
    bfsrc = GS_BLSRC_ONEMINUSSRCALPHA;
  else
  if (!stricmp(sbfsrc, "DST_ALPHA"))
    bfsrc = GS_BLSRC_DSTALPHA;
  else
  if (!stricmp(sbfsrc, "ONE_MINUS_DST_ALPHA"))
    bfsrc = GS_BLSRC_ONEMINUSDSTALPHA;
  else
  if (!stricmp(sbfsrc, "SRC_ALPHA_SATURATE"))
    bfsrc = GS_BLSRC_ALPHASATURATE;
  else
  {
    Warning( 0,0,"Warning: unknown SRC BlendMode '%s' in Shader '%s', substituting ONE\n", sbfsrc, ef->m_Name.c_str());
    bfsrc = GS_BLSRC_ONE;
  }

  // find DST factor
  if (!stricmp(sbfdst, "ONE"))
    bfdst = GS_BLDST_ONE;
  else
  if (!stricmp(sbfdst, "ZERO"))
    bfdst = GS_BLDST_ZERO;
  else
  if (!stricmp(sbfdst, "SRC_ALPHA") || !stricmp(sbfsrc, "SrcAlpha"))
    bfdst = GS_BLDST_SRCALPHA;
  else
  if (!stricmp(sbfdst, "ONE_MINUS_SRC_ALPHA") || !stricmp(sbfsrc, "InvSrcAlpha"))
    bfdst = GS_BLDST_ONEMINUSSRCALPHA;
  else
  if (!stricmp(sbfdst, "DST_ALPHA"))
    bfdst = GS_BLDST_DSTALPHA;
  else
  if (!stricmp(sbfdst, "ONE_MINUS_DST_ALPHA"))
    bfdst = GS_BLDST_ONEMINUSDSTALPHA;
  else
  if (!stricmp(sbfdst, "SRC_COLOR"))
    bfdst = GS_BLDST_SRCCOL;
  else
  if (!stricmp(sbfdst, "ONE_MINUS_SRC_COLOR"))
    bfdst = GS_BLDST_ONEMINUSSRCCOL;
  else
  {
    Warning( 0,0,"Warning: unknown DST BlendMode '%s' in Shader '%s', substituting ONE\n", sbfdst, ef->m_Name.c_str());
    bfdst = GS_BLDST_ONE;
  }
  *src = bfsrc;
  *dst = bfdst;
}

uint CShader::mfCompileRendState(SShader *ef, SShaderPass *sm, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eBlend = 1, eNoColorMask, eColorMaskOnlyAlpha, eColorMaskOnlyRGB, eColorMaskOnlyColor, eAlphaFunc, eDepthFunc, eDepthWrite, eDepthMask, eNoDepthTest, ePolyLine, eDepthTest};
  static tokenDesc commands[] =
  {
    {eBlend, "Blend"},
    {eDepthFunc, "DepthFunc"},
    {eAlphaFunc, "AlphaFunc"},
    {eNoColorMask, "NoColorMask"},
    {eColorMaskOnlyAlpha, "ColorMaskOnlyAlpha"},
    {eColorMaskOnlyRGB, "ColorMaskOnlyRGB"},
    {eColorMaskOnlyColor, "ColorMaskOnlyColor"},
    {ePolyLine, "PolyLine"},
    {eDepthMask, "DepthMask"},
    {eDepthWrite, "DepthWrite"},
    {eNoDepthTest, "NoDepthTest"},
    {eDepthTest, "DepthTest"},
    {0,0}
  };

  uint af = 0;
  uint df = 0;
  uint bfsrc = 0;
  uint bfdst = 0;
  uint dm = GS_DEPTHWRITE;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (params)
      data = params;
    else
    if (name)
      data = name;

    switch (cmd)
    {
      case eNoColorMask:
        af |= GS_NOCOLMASK;
        break;

      case eColorMaskOnlyAlpha:
        af |= GS_COLMASKONLYALPHA;
        break;

      case eColorMaskOnlyRGB:
      case eColorMaskOnlyColor:
        af |= GS_COLMASKONLYRGB;
        break;

      case ePolyLine:
        af |= GS_POLYLINE;
        break;

      case eAlphaFunc:
        if (!stricmp(data, "GT0"))
          af = GS_ALPHATEST_GREATER0;
        else
        if (!stricmp(data, "LT128"))
          af = GS_ALPHATEST_LESS128;
        else
        if (!stricmp(data, "GE128"))
          af = GS_ALPHATEST_GEQUAL128;
        else
          Warning( 0,0,"Warning: invalid AlphaFunc name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eDepthFunc:
        if (!stricmp(data, "LEqual"))
          df = 0;
        else
        if (!stricmp(data, "Equal"))
          df = GS_DEPTHFUNC_EQUAL;
        else
          Warning( 0,0,"Warning: unknown DepthFunc name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eBlend:
        sGetBlend(data, ef, &bfsrc, &bfdst);
        dm = 0;
        break;

      case eDepthMask:
      case eDepthWrite:
        if (data && data[0])
        {
          if (shGetBool(data))
          {
            dm |= GS_DEPTHWRITE;
            ef->m_Flags3 |= EF3_DEPTHWRITE;
          }
          else
          {
            dm &= ~GS_DEPTHWRITE;
          }
        }
        else
        {
          dm |= GS_DEPTHWRITE;
          ef->m_Flags3 |= EF3_DEPTHWRITE;
        }
        break;

      case eDepthTest:
        if (data && data[0])
        {
          if (!shGetBool(data))
            dm |= GS_NODEPTHTEST;
          else
            dm &= ~GS_NODEPTHTEST;
        }
        else
          dm &= ~GS_NODEPTHTEST;
        break;

      case eNoDepthTest:
        dm |= GS_NODEPTHTEST;
        dm &= ~GS_DEPTHWRITE;
        break;
    }
  }
  uint factor = dm | df | bfsrc | bfdst | af;

  return factor;
}

byte sGetColorOp(char *data, SShader *ef)
{
  if (!stricmp(data, "NoSet"))
    return eCO_NOSET;
  else
  if (!stricmp(data, "Disable") || !stricmp(data, "None"))
    return eCO_DISABLE;
  else
  if (!stricmp(data, "Replace") || !stricmp(data, "SelectArg1"))
    return eCO_REPLACE;
  else
  if (!stricmp(data, "SelectArg2"))
    return eCO_ARG2;
  else
  if (!stricmp(data, "Decal"))
    return eCO_DECAL;
  else
  if (!stricmp(data, "BlendDiffuseAlpha"))
    return eCO_BLENDDIFFUSEALPHA;
  else
  if (!stricmp(data, "Modulate"))
    return eCO_MODULATE;
  else
  if (!stricmp(data, "Modulate4X"))
    return eCO_MODULATE4X;
  else
  if (!stricmp(data, "Modulate2X"))
    return eCO_MODULATE2X;
  else
  if (!stricmp(data, "Add"))
    return eCO_ADD;
  else
  if (!stricmp(data, "MultiplyAdd"))
    return eCO_MULTIPLYADD;
  else
  if (!stricmp(data, "AddSigned") || !stricmp(data, "Add_Signed"))
    return eCO_ADDSIGNED;
  else
  if (!stricmp(data, "AddSigned2X"))
    return eCO_ADDSIGNED2X;
  else
  if (!stricmp(data, "BumpEnvMap"))
    return eCO_BUMPENVMAP;
  else
  if (!stricmp(data, "MODULATEALPHA_ADDCOLOR"))
    return eCO_MODULATEALPHA_ADDCOLOR;
  else
  if (!stricmp(data, "MODULATECOLOR_ADDALPHA"))
    return eCO_MODULATECOLOR_ADDALPHA;
  else
  if (!stricmp(data, "MODULATEINVALPHA_ADDCOLOR"))
    return eCO_MODULATEINVALPHA_ADDCOLOR;
  else
  if (!stricmp(data, "MODULATEINVCOLOR_ADDALPHA"))
    return eCO_MODULATEINVCOLOR_ADDALPHA;
  else
  if (!stricmp(data, "DOTPRODUCT3"))
    return eCO_DOTPRODUCT3;
  else
  if (!stricmp(data, "LERP"))
    return eCO_LERP;
  else
  if (!stricmp(data, "SUBTRACT"))
    return eCO_SUBTRACT;
  else
  {
    Warning( 0,0,"Warning: unknown TexColorOp parameter '%s' in Shader '%s' (Skipping)\n", data, ef->m_Name.c_str());
    return eCO_MODULATE;
  }
}

byte sGetColorArg(char *data, SShader *ef)
{
  if (!stricmp(data, "Texture"))
    return eCA_Texture;
  else
  if (!stricmp(data, "Diffuse"))
    return eCA_Diffuse;
  else
  if (!stricmp(data, "Specular"))
    return eCA_Specular;
  else
  if (!stricmp(data, "Previous") || !stricmp(data, "Current"))
    return eCA_Previous;
  else
  if (!stricmp(data, "Constant") || !stricmp(data, "TFactor"))
    return eCA_Constant;
  else
  {
    Warning( 0,0,"Warning: unknown TexColorArg parameter '%s' in Shader '%s' (Skipping)\n", data, ef->m_Name.c_str());
    return eCA_Texture;
  }
}

bool CShader::mfCompileLayer(SShader *ef, int nl, char *scr, SShaderPass *sm)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  char mapname[128] = "";
  uint af = 0;
  uint df = 0;
  uint ctc = 0;
  uint bfsrc = 0;
  uint bfdst = 0;
  uint dm = GS_DEPTHWRITE;
  ETexType eTT = eTT_Base;
  bool bNoMips = false;
  int BumpDet = 0;
  int Flags2 = 0;
  bool bProjected = false;
  float fBumpAmount = -1.0f;
  bool bNoScale = false;

  SShaderTexUnit *ml;
  if (!sm)
  {
    ef->m_Passes.ReserveNew(nl+1);
    sm = &ef->m_Passes[nl];
    sm->m_RenderState = GS_DEPTHWRITE;
    sm->mfAddNewTexUnits(1);
    ml = &sm->m_TUnits[0];
  }
  else
  {
    sm->mfAddNewTexUnits(1);
    ml = &sm->m_TUnits[sm->m_TUnits.Num()-1];
  }
  ml->m_eColorOp = eCO_MODULATE;
  ml->m_eAlphaOp = eCO_MODULATE;
  if (sm->m_TUnits.Num() == 1)
  {
    ml->m_eColorArg = eCA_Texture | (eCA_Diffuse << 3);
    ml->m_eAlphaArg = eCA_Texture | (eCA_Diffuse << 3);
  }
  else
  {
    ml->m_eColorArg = eCA_Texture | (eCA_Previous << 3);
    ml->m_eAlphaArg = eCA_Texture | (eCA_Previous << 3);
  }

  enum {eMap = 1, eBlend, eProjected, eNoColorMask, eColorMaskOnlyAlpha, eColorMaskOnlyRGB, eColorMaskOnlyColor, eNoDXT, eAlphaFunc, eDepthFunc, eDepthWrite, eClampTexCoords, eUClamp, eVClamp, ergbGen, eAlphaGen, eTexGen, etcGen, eDepthMask, eNoDepthTest, eDepthTest, eSequence, eTexType, eNoMipmaps, eBumpDetect, eTexColorOp, eTexAlphaOp, eTexColorArg0, eTexColorArg1, eTexColorArg2, eTexAlphaArg0, eTexAlphaArg1, eTexAlphaArg2, eForceOpaque, ePolyLine, eBumpAmount, eTexLodBias, eBumpHighRes, eBumpLowRes, eBumpCompressed, eBumpInverted, eSecondPassRendState, eRendState, eTexFilter, eTexNoScale, eHDRTexColorOp};
  static tokenDesc commands[] =
  {
    {eMap, "Map"},
    {eTexType, "TexType"},
    {eBlend, "Blend"},
    {eProjected, "Projected"},
    {eRendState, "RendState"},
    {eSecondPassRendState, "SecondPassRendState"},
    {eDepthFunc, "DepthFunc"},
    {eAlphaFunc, "AlphaFunc"},
    {eClampTexCoords, "ClampTexCoords"},
    {eUClamp, "UClamp"},
    {eVClamp, "VClamp"},
    {ergbGen, "rgbGen"},
    {eNoColorMask, "NoColorMask"},
    {eNoDXT, "NoDXT"},
    {eColorMaskOnlyAlpha, "ColorMaskOnlyAlpha"},
    {eColorMaskOnlyRGB, "ColorMaskOnlyRGB"},
    {eColorMaskOnlyColor, "ColorMaskOnlyColor"},
    {eAlphaGen, "AlphaGen"},
    {eTexGen, "TexGen"},
    {ePolyLine, "PolyLine"},
    {etcGen, "tcGen"},
    {eDepthMask, "DepthMask"},
    {eDepthWrite, "DepthWrite"},
    {eNoDepthTest, "NoDepthTest"},
    {eDepthTest, "DepthTest"},
    {eSequence, "Sequence"},
    {eNoMipmaps, "NoMipmaps"},
    {eBumpDetect, "BumpDetect"},
    {eBumpAmount, "BumpAmount"},
    {eBumpHighRes, "BumpHighRes"},
    {eBumpLowRes, "BumpLowRes"},
    {eBumpCompressed, "BumpCompressed"},
    {eBumpInverted, "BumpInverted"},
    {eTexColorOp, "TexColorOp"},
    {eTexAlphaOp, "TexAlphaOp"},
    {eTexColorArg0, "TexColorArg0"},
    {eTexColorArg1, "TexColorArg1"},
    {eTexColorArg2, "TexColorArg2"},
    {eTexAlphaArg0, "TexAlphaArg0"},
    {eTexAlphaArg1, "TexAlphaArg1"},
    {eTexAlphaArg2, "TexAlphaArg2"},
    {eTexLodBias, "TexLodBias"},
    {eTexNoScale, "TexNoScale"},

    {eTexFilter, "TexFilter"},

    {eForceOpaque, "ForceOpaque"},
    {eHDRTexColorOp, "HDRTexColorOp"},
    {0,0}
  };

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
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing Map argument for layer %d in Shader '%s'\n", sm-&ef->m_Passes[0], ef->m_Name.c_str());
          break;
        }
        strcpy(mapname, data);
        break;

      case eNoColorMask:
        af |= GS_NOCOLMASK;
        break;

      case eNoDXT:
        Flags2 |= FT2_NODXT;
        break;

      case eTexNoScale:
        bNoScale = true;
        break;

      case eTexFilter:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing TexFilter argument for layer %d in Shader '%s'\n", sm-&ef->m_Passes[0], ef->m_Name.c_str());
          break;
        }
        if (!stricmp(data, "Bilinear"))
          Flags2 |= FT2_FILTER_BILINEAR;
        else
        if (!stricmp(data, "Trilinear"))
          Flags2 |= FT2_FILTER_TRILINEAR;
        else
        if (!stricmp(data, "Nearest"))
          Flags2 |= FT2_FILTER_NEAREST;
        else
        if (!stricmp(data, "Anisotropic"))
          Flags2 |= FT2_FILTER_ANISOTROPIC;
        else
        {
          Warning( 0,0,"Warning: unknown TexFilter argument (%s) for layer %d in Shader '%s'\n", data, sm-&ef->m_Passes[0], ef->m_Name.c_str());
        }
        break;

      case eSecondPassRendState:
        sm->m_SecondRenderState = mfCompileRendState(ef, sm, params);
        sm->m_Flags |= SHPF_USEDSECONDRS;
        break;

      case eRendState:
        sm->m_RenderState = mfCompileRendState(ef, sm, params);
        break;

      case eColorMaskOnlyAlpha:
        af |= GS_COLMASKONLYALPHA;
        break;

      case eColorMaskOnlyRGB:
      case eColorMaskOnlyColor:
        af |= GS_COLMASKONLYRGB;
        break;

      case eTexType:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing TexType argument for layer %d in Shader '%s'\n", sm-&ef->m_Passes[0], ef->m_Name.c_str());
          break;
        }
        if (!strnicmp(data, "Base", 4))
          eTT = eTT_Base;
        else
        if (!strnicmp(data, "Bump", 4))
          eTT = eTT_Bumpmap;
        else
        if (!strnicmp(data, "DSDTBump", 8))
          eTT = eTT_DSDTBump;
        else
        if (!strnicmp(data, "Volume", 6))
          eTT = eTT_3D;
        else
        if (!strnicmp(data, "Cube", 4))
          eTT = eTT_Cubemap;
        else
        {
          Warning( 0,0,"Warning: unknown TexType argument (%s) for layer %d in Shader '%s'\n", data, sm-&ef->m_Passes[0], ef->m_Name.c_str());
          eTT = eTT_Base;
        }
        break;

      case eSequence:
        mfCompileSequence(ef, ml, nl, params, eTT, 0, Flags2, fBumpAmount);
        break;

      case eForceOpaque:
        ef->m_Flags2 |= EF2_HASOPAQUE;
        ml->m_nFlags |= FTU_OPAQUE;
        break;

      case eNoMipmaps:
        bNoMips = true;
        break;

      case eTexLodBias:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing TexLodBias argument for layer %d in Shader '%s'\n", sm-&ef->m_Passes[0], ef->m_Name.c_str());
          break;
        }
        ml->m_fTexFilterLodBias = shGetFloat(data);
        break;

      case eBumpAmount:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing BumpAmount argument for layer %d in Shader '%s'\n", sm-&ef->m_Passes[0], ef->m_Name.c_str());
          break;
        }
        fBumpAmount = CLAMP(shGetFloat(data), 2.0f, 100.0f);
        break;

      case eBumpHighRes:
        Flags2 |= FT2_BUMPHIGHRES;
        break;

      case eBumpLowRes:
        Flags2 |= FT2_BUMPLOWRES;
        break;

      case eBumpCompressed:
        Flags2 |= FT2_BUMPCOMPRESED;
        break;

      case eBumpInverted:
        Flags2 |= FT2_BUMPINVERTED;
        break;

      case eProjected:
        bProjected = true;
        break;

      case eBumpDetect:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing BumpDetect argument for layer %d in Shader '%s'\n", sm-&ef->m_Passes[0], ef->m_Name.c_str());
          break;
        }
        if (!stricmp(data, "Red"))
          BumpDet = FT_BUMP_DETRED;
        else
        if (!stricmp(data, "Blue"))
          BumpDet = FT_BUMP_DETBLUE;
        else
        if (!stricmp(data, "Intensity"))
          BumpDet = FT_BUMP_DETINTENS;
        else
        if (!stricmp(data, "Alpha"))
          BumpDet = FT_BUMP_DETALPHA;
        else
        {
          Warning( 0,0,"Warning: invalid BumpDetect argument '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
          BumpDet = FT_BUMP_DETINTENS;
        }
        break;

      case ePolyLine:
        af |= GS_POLYLINE;
        break;

      case eAlphaFunc:
        if (!stricmp(data, "GT0"))
          af = GS_ALPHATEST_GREATER0;
        else
        if (!stricmp(data, "LT128"))
          af = GS_ALPHATEST_LESS128;
        else
        if (!stricmp(data, "GE128"))
          af = GS_ALPHATEST_GEQUAL128;
        else
          Warning( 0,0,"Warning: invalid AlphaFunc name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eDepthFunc:
        if (!stricmp(data, "LEqual"))
          df = 0;
        else
        if (!stricmp(data, "Equal"))
          df = GS_DEPTHFUNC_EQUAL;
        else
        if (!strnicmp(data, "Great", 5))
          df = GS_DEPTHFUNC_GREAT;
        else
          Warning( 0,0,"Warning: unknown DepthFunc name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eClampTexCoords:
        ctc |= GS_TEXPARAM_CLAMP;
        break;

      case eUClamp:
        ctc |= GS_TEXPARAM_UCLAMP;
        break;

      case eVClamp:
        ctc |= GS_TEXPARAM_VCLAMP;
        break;

      case eTexColorOp:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexColorOp in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eColorOp = sGetColorOp(data, ef);
        break;

      case eHDRTexColorOp:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for HDRTexColorOp in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eHDRColorOp = sGetColorOp(data, ef);
        break;

      case eTexAlphaOp:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexAlphaOp in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eAlphaOp = sGetColorOp(data, ef);
        break;

      case eTexColorArg0:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexColorArg0 in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eColorArg &= ~7;
        ml->m_eColorArg |= sGetColorArg(data, ef);
        break;

      case eTexColorArg1:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexColorArg1 in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eColorArg &= ~(7<<3);
        ml->m_eColorArg |= sGetColorArg(data, ef) << 3;
        break;

      case eTexColorArg2:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexColorArg2 in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eColorArg &= ~(7<<6);
        ml->m_eColorArg |= sGetColorArg(data, ef) << 6;
        break;

      case eTexAlphaArg0:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexAlphaArg0 in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eAlphaArg &= ~7;
        ml->m_eAlphaArg |= sGetColorArg(data, ef);
        break;

      case eTexAlphaArg1:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexAlphaArg1 in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eAlphaArg &= ~(7<<3);
        ml->m_eAlphaArg |= sGetColorArg(data, ef) << 3;
        break;

      case eTexAlphaArg2:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexAlphaArg2 in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ml->m_eAlphaArg &= ~(7<<6);
        ml->m_eAlphaArg |= sGetColorArg(data, ef) << 6;
        break;

      case eBlend:
        sGetBlend(data, ef, &bfsrc, &bfdst);
        dm = 0;
        break;

      case ergbGen:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for rgbGen in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ef->m_Flags3 |= EF3_HASRGBGEN;
        if (!stricmp(data, "Wave"))
        {
          sm->m_WaveEvalRGB = new SWaveForm;
          mfCompileWaveForm(sm->m_WaveEvalRGB, params);
          sm->m_eEvalRGB = eERGB_Wave;
        }
        else
        if (!stricmp(data, "Noise"))
        {
          sm->m_RGBNoise = new SRGBGenNoise;
          mfCompileRGBNoise(sm->m_RGBNoise, params, ef);
          sm->m_eEvalRGB = eERGB_Noise;
        }
        else
        if (!strnicmp(data, "Comp", 4))
        {
          sm->m_RGBComps = new SParam;
          mfCompileParamComps(sm->m_RGBComps, params, ef);
          sm->m_eEvalRGB = eERGB_Comps;
        }
        else
        if (!stricmp(data, "Identity"))
          sm->m_eEvalRGB = eERGB_Identity;
        else
        if (!stricmp(data, "NoFill"))
          sm->m_eEvalRGB = eERGB_NoFill;
        else
        if (!stricmp(data, "Object") || !strnicmp(data, "FromObj", 7))
          sm->m_eEvalRGB = eERGB_Object;
        else
        if (!stricmp(data, "OneMinusObject"))
          sm->m_eEvalRGB = eERGB_OneMinusObject;
        else
        if (!stricmp(data, "RE") || !stricmp(data, "FromRE"))
          sm->m_eEvalRGB = eERGB_RE;
        else
        if (!stricmp(data, "OneMinusRE"))
          sm->m_eEvalRGB = eERGB_OneMinusRE;
        else
        if (!stricmp(data, "World") || !stricmp(data, "FromWorld") || !stricmp(data, "WorldColor"))
          sm->m_eEvalRGB = eERGB_World;
        else
        if (!stricmp(data, "FromClient"))
        {
          sm->m_eEvalRGB = eERGB_FromClient;
          sm->m_eEvalAlpha = eEALPHA_FromClient;
        }
        else
        if (!stricmp(data, "OneMinusFromClient"))
          sm->m_eEvalRGB = eERGB_OneMinusFromClient;
        else
        if (!stricmp(data, "Fixed"))
        {
          sm->m_eEvalRGB = eERGB_Fixed;
          sm->m_eEvalAlpha = eEALPHA_Fixed;
          if (!params || !params[0])
          {
            Warning( 0,0,"Warning: missing RgbGen Fixed value in Shader '%s' (use 1.0)\n", ef->m_Name.c_str());
            sm->m_FixedColor.bcolor[0] = 255;
            sm->m_FixedColor.bcolor[1] = 255;
            sm->m_FixedColor.bcolor[2] = 255;
          }
          else
          {
            CFColor col;
            shGetColor(params, col);
            COLCONV(col);
            sm->m_FixedColor.dcolor = col.GetTrue();
          }
        }
        else
        if (!stricmp(data, "Style"))
        {
          mfCompileRGBAStyle(params, ef, sm, true);
          sm->m_eEvalRGB = eERGB_StyleIntens;
        }
        else
          Warning( 0,0,"Warning: unknown rgbGen parameter '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eAlphaGen:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for AlphaGen in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        ef->m_Flags3 |= EF3_HASALPHAGEN;
        if (!stricmp(data, "Wave"))
        {
          sm->m_WaveEvalAlpha = new SWaveForm;
          mfCompileWaveForm(sm->m_WaveEvalAlpha, params);
          sm->m_eEvalAlpha = eEALPHA_Wave;
        }
        else
        if (!stricmp(data, "Noise"))
        {
          sm->m_ANoise = new SAlphaGenNoise;
          mfCompileAlphaNoise(sm->m_ANoise, params, ef);
          sm->m_eEvalAlpha = eEALPHA_Noise;
        }
        else
        if (!stricmp(data, "Style"))
        {
          mfCompileRGBAStyle(params, ef, sm, false);
          sm->m_eEvalAlpha = eEALPHA_Style;
        }
        else
        if (!stricmp(data, "Identity"))
          sm->m_eEvalAlpha = eEALPHA_Identity;
        else
        if (!strnicmp(data, "Comp", 4))
        {
          sm->m_RGBComps = new SParam;
          mfCompileParamComps(sm->m_RGBComps, params, ef);
          sm->m_eEvalAlpha = eEALPHA_Comps;
        }
        else
        if (!stricmp(data, "NoFill"))
          sm->m_eEvalAlpha = eEALPHA_NoFill;
        else
        if (!stricmp(data, "Object") || !strnicmp(data, "FromObj", 7))
          sm->m_eEvalAlpha = eEALPHA_Object;
        else
        if (!stricmp(data, "OneMinusObject"))
          sm->m_eEvalAlpha = eEALPHA_OneMinusObject;
        else
        if (!stricmp(data, "RE") || !stricmp(data, "FromRE"))
          sm->m_eEvalAlpha = eEALPHA_RE;
        else
        if (!stricmp(data, "OneMinusRE"))
          sm->m_eEvalAlpha = eEALPHA_OneMinusRE;
        else
        if (!stricmp(data, "World") || !stricmp(data, "FromWorld") || !stricmp(data, "WorldColor"))
          sm->m_eEvalAlpha = eEALPHA_World;
        else
        if (!stricmp(data, "FromClient"))
          sm->m_eEvalAlpha = eEALPHA_FromClient;
        else
        if (!stricmp(data, "OneMinusFromClient"))
          sm->m_eEvalAlpha = eEALPHA_OneMinusFromClient;
        else
        if (!stricmp(data, "Portal"))
          sm->m_eEvalAlpha = eEALPHA_Portal;
        else
        if (!stricmp(data, "Fixed"))
        {
          sm->m_eEvalAlpha = eEALPHA_Fixed;
          if (!params || !params[0])
          {
            Warning( 0,0,"Warning: missing AlphaGen Fixed value in Shader '%s' (use 1.0)\n", ef->m_Name.c_str());
            sm->m_FixedColor.bcolor[3] = 255;
          }
          else
            sm->m_FixedColor.bcolor[3] = (int)(shGetFloat(params) * 255.0);
        }
        else
        if (!stricmp(data, "Beam"))
        {
          sm->m_eEvalAlpha = eEALPHA_Beam;
        }
        else
          Warning( 0,0,"Warning: unknown AlphaGen parameter '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eTexGen:
      case etcGen:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for TexGen in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        if (!stricmp(data, "NoFill"))
          ml->m_eGenTC = eGTC_NoFill;
        else
        if (!stricmp(data, "Environment"))
          ml->m_eGenTC = eGTC_Environment;
        else
        if (!stricmp(data, "SphereMap"))
          ml->m_eGenTC = eGTC_SphereMap;
        else
        if (!stricmp(data, "Projection"))
          ml->m_eGenTC = eGTC_Projection;
        else
        if (!stricmp(data, "SphereMapEnvironment"))
          ml->m_eGenTC = eGTC_SphereMapEnvironment;
        else
        if (!stricmp(data, "ShadowMap"))
          ml->m_eGenTC = eGTC_ShadowMap;
        else
        if (!stricmp(data, "Lightmap"))
          ml->m_eGenTC = eGTC_LightMap;
        else
        if (!stricmp(data, "Quad"))
          ml->m_eGenTC = eGTC_Quad;
        else
        if (!stricmp(data, "Texture") || !stricmp(data, "Base"))
          ml->m_eGenTC = eGTC_Base;
        else
        if (!mfCompileTexGen(name, params, ef, ml))
          Warning( 0,0,"Warning: unknown TexGen parameter '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eDepthMask:
      case eDepthWrite:
        if (data && data[0])
        {
          if (shGetBool(data))
          {
            dm |= GS_DEPTHWRITE;
            ef->m_Flags3 |= EF3_DEPTHWRITE;
          }
          else
          {
            dm &= ~GS_DEPTHWRITE;
          }
        }
        else
        {
          dm |= GS_DEPTHWRITE;
          ef->m_Flags3 |= EF3_DEPTHWRITE;
        }
        break;

      case eNoDepthTest:
        dm |= GS_NODEPTHTEST;
        dm &= ~GS_DEPTHWRITE;
        break;

      case eDepthTest:
        if (data && data[0])
        {
          if (!shGetBool(data))
            dm |= GS_NODEPTHTEST;
          else
            dm &= ~GS_NODEPTHTEST;
        }
        else
          dm &= ~GS_NODEPTHTEST;
        break;

      default:
        Warning( 0,0,"Warning: unknown parameter '%s' in Shader '%s' for layer %d\n", strtok(params, " \n"), ef->m_Name.c_str(), sm - &ef->m_Passes[0]);
        return false;
    }
  }

  /*if (bfsrc == GS_BLSRC_ONE && bfdst == GS_BLDST_ZERO)
  {
    bfsrc = bfdst = 0;
    dm = GS_DEPTHWRITE;
  }
  if (bf1src == GS_BLSRC_ONE && bf1dst == GS_BLDST_ZERO)
  {
    bf1src = bf1dst = 0;
    dm1 = GS_DEPTHWRITE;
  }*/
  sm->m_RenderState = ctc | df | af | dm | bfsrc | bfdst;

  int State = sm->m_RenderState;
  int mp = ((ef->m_Flags & EF_NOMIPMAPS) || bNoMips) ? FT_NOMIPS : 0;
  mp |= BumpDet;
  if (ef->m_eClass == eSH_Screen)
    mp |= FT_NOREMOVE;
  if (bNoScale)
    mp |= FT_NORESIZE;
  if (ctc & GS_TEXPARAM_CLAMP)
  {
    mp |= FT_CLAMP;
    ml->m_nFlags |= FTU_CLAMP;
  }
  if (ctc & GS_TEXPARAM_UCLAMP)
  {
    Flags2 |= FT2_UCLAMP;
    ml->m_nFlags |= FTU_CLAMP;
  }
  if (ctc & GS_TEXPARAM_VCLAMP)
  {
    Flags2 |= FT2_VCLAMP;
    ml->m_nFlags |= FTU_CLAMP;
  }
  if (bProjected)
  {
    mp |= FT_PROJECTED;
    ml->m_nFlags |= FTU_PROJECTED;
  }
  if (bNoMips)
    ml->m_nFlags |= FTU_NOMIPS;
  if (eTT == eTT_Bumpmap || eTT == eTT_DSDTBump)
  {
    if ((gRenDev->GetFeatures() & RFT_BUMP))
    {
      State = GS_BUMP;
      ef->m_LMFlags |= LMF_BUMPMATERIAL;
    }
    else
      return false;
  }
  if (!ml->m_TexPic)
    ml->m_TexPic = mfCheckTemplateTexName(mapname, eTT, ml->m_nFlags);

  if (Flags2 & FT2_FILTER_BILINEAR)
    ml->m_nFlags |= FTU_FILTERBILINEAR;
  else
  if (Flags2 & FT2_FILTER_TRILINEAR)
    ml->m_nFlags |= FTU_FILTERTRILINEAR;
  else
  if (Flags2 & FT2_FILTER_NEAREST)
    ml->m_nFlags |= FTU_FILTERNEAREST;

  if (!ml->m_TexPic)
  {
    if (mapname[0])
    {
      if (stricmp(mapname, "$None") != 0)
        ml->m_TexPic = (STexPic*)gRenDev->EF_LoadTexture(mapname, mp, Flags2, eTT, fBumpAmount);
    }
  }

  if (stricmp(mapname, "$None") && !ml->m_TexPic)
  {
    if (!gRenDev->m_IsDedicated)
      Warning( 0,0,"Warning: Couldn't load Texture '%s' in Shader '%s'\n", mapname, ef->m_Name.c_str());
    return false;
  }
  ml->m_eTexType = eTT;
  if (ml->m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_DIFFUSE])
    ef->m_Flags |= EF_HASDIFFUSEMAP;

  if (!(sm->m_Flags & SHPF_USEDSECONDRS))
    sm->m_SecondRenderState = sm->m_RenderState;

  return true;
}

void CShader::mfCompileOrient(SShader *ef, int num, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  SOrient *orn;

  enum {eX = 1, eY, eOrigin};
  static tokenDesc commands[] =
  {
    {eX, "X"},
    {eY, "Y"},
    {eOrigin, "Origin"},
    {0,0}
  };

  if (num >= MAX_ORIENTS)
  {
    Warning( 0,0,"Warning: MAX_ORIENTS hit\n");
    return;
  }
  orn = &m_Orients[num];
  if (num >= m_NumOrients)
    m_NumOrients = num+1;
  memset(orn, 0, sizeof(SOrient));

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
      case eX:
        shGetVector(data, orn->m_Coord.m_Vecs[1]);
        if (GetLengthSquared(orn->m_Coord.m_Vecs[1]) == 0)
          orn->m_Flags |= FOR_ORTHO;
        else
          orn->m_Coord.m_Vecs[1].Normalize();
        break;

      case eY:
        shGetVector(data, orn->m_Coord.m_Vecs[2]);
        orn->m_Coord.m_Vecs[2].Normalize();
        break;

      case eOrigin:
        shGetVector(data, orn->m_Coord.m_Org);
        break;
    }
  }
  orn->m_Coord.m_Vecs[0] = orn->m_Coord.m_Vecs[1] ^ orn->m_Coord.m_Vecs[2];
}

//==================================================================================================

void CShader::mfCompileLightMove(SShader *ef, char *nameMove, SLightEval *le, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eDirection = 1, eWave, eSpeed};
  static tokenDesc commands[] =
  {
    {eDirection, "Direction"},
    {eWave, "Wave"},
    {eSpeed, "Speed"},
    {0,0}
  };

  if (le->m_LightMove)
    delete le->m_LightMove;
  le->m_LightMove = new SLightMove;
  if (!stricmp(nameMove, "Wave"))
    le->m_LightMove->m_eLMType = eLMT_Wave;
  else
  if (!stricmp(nameMove, "Patch"))
    le->m_LightMove->m_eLMType = eLMT_Patch;
  else
    Warning( 0,0,"Warning: Unknown LightMove Type int shader '%s'\n", ef->m_Name.c_str());
  le->m_LightMove->m_Dir = Vec3d(0,0,1);

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
      case eDirection:
        shGetVector(data, le->m_LightMove->m_Dir);
        break;

      case eSpeed:
        le->m_LightMove->m_fSpeed = shGetFloat(data);
        break;

      case eWave:
        mfCompileWaveForm(&le->m_LightMove->m_Wave, params);
        break;
    }
  }
}

void CShader::mfCompileEvalLight(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eStyle = 1, eUpdateStyle, eProjRotate, eLightRotate, eLightMove, eLightOffset, eLightPos};
  static tokenDesc commands[] =
  {
    {eStyle, "Style"},
    {eUpdateStyle, "UpdateStyle"},
    {eProjRotate, "ProjRotate"},
    {eLightRotate, "LightRotate"},
    {eLightMove, "LightMove"},
    {eLightPos, "LightPos"},
    {eLightOffset, "LightOffset"},
    {0,0}
  };

  if (ef->m_EvalLights)
    delete ef->m_EvalLights;
  ef->m_EvalLights = new SLightEval;

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
      case eStyle:
        ef->m_EvalLights->m_LightStyle = shGetInt(data);
        break;

      case eUpdateStyle:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for UpdateStyle in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        if (!strnicmp(data, "Intens", 6))
          ef->m_EvalLights->m_EStyleType = eLS_Intensity;
        else
        if (!stricmp(data, "Color") || !stricmp(data, "RGB"))
          ef->m_EvalLights->m_EStyleType = eLS_RGB;
        else
          Warning( 0,0,"Warning: unknown UpdateStyle parameter '%s' in Shader '%s' (Skipping)\n", data, ef->m_Name.c_str());
        break;

      case eProjRotate:
        mfCompileParamComps(&ef->m_EvalLights->m_ProjRotate, data, ef);
        break;

      case eLightRotate:
        shGetVector(data, ef->m_EvalLights->m_LightRotate);
        break;

      case eLightMove:
        mfCompileLightMove(ef, name, ef->m_EvalLights, params);
        break;

      case eLightOffset:
      case eLightPos:
        shGetVector(data, ef->m_EvalLights->m_LightOffset);
        break;
    }
  }
}

static int sGetOp(char *op, SShader *ef)
{
  if (!stricmp(op, "KEEP"))
    return FSS_STENCOP_KEEP;
  if (!stricmp(op, "REPLACE"))
    return FSS_STENCOP_REPLACE;
  if (!stricmp(op, "INCR"))
    return FSS_STENCOP_INCR;
  if (!stricmp(op, "DECR"))
    return FSS_STENCOP_DECR;
  if (!stricmp(op, "ZERO"))
    return FSS_STENCOP_ZERO;

  Warning( 0,0,"Invalid StencilOp '%s' in Shader '%s\n", op, ef->m_Name.c_str());
  return FSS_STENCOP_KEEP;
}

void CShader::mfCompileStencil(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  int n = 0;

  if (!ef->m_State)
    return;

  ef->m_State->m_Stencil = new SStencil;
  SStencil *sm = ef->m_State->m_Stencil;

  enum {eFunc = 1, eOp};
  static tokenDesc commands[] =
  {
    {eFunc, "Func"},
    {eOp, "Op"},
    {0,0}
  };

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
      case eFunc:
        {
          char func[3][32];
          sscanf(data, "%s %s %s", func[0], func[1], func[2]);
          if (!stricmp(func[0], "ALWAYS"))
            sm->m_State |= FSS_STENCFUNC_ALWAYS;
          else
          if (!stricmp(func[0], "NEVER"))
            sm->m_State |= FSS_STENCFUNC_NEVER;
          else
          if (!stricmp(func[0], "LESS"))
            sm->m_State |= FSS_STENCFUNC_LESS;
          else
          if (!stricmp(func[0], "LEQUAL"))
            sm->m_State |= FSS_STENCFUNC_LEQUAL;
          else
          if (!stricmp(func[0], "GREATER"))
            sm->m_State |= FSS_STENCFUNC_GREATER;
          else
          if (!stricmp(func[0], "GEQUAL"))
            sm->m_State |= FSS_STENCFUNC_GEQUAL;
          else
          if (!stricmp(func[0], "EQUAL"))
            sm->m_State |= FSS_STENCFUNC_EQUAL;
          else
          if (!stricmp(func[0], "NOTEQUAL"))
            sm->m_State |= FSS_STENCFUNC_NOTEQUAL;
          else
            Warning( 0,0,"invalid StencilFunc '%s' in Shader '%s\n", func[0]);

          sm->m_FuncRef = atol(func[1]);
          sm->m_FuncMask = atol(func[2]);
        }
        break;

      case eOp:
        {
          char func[3][32];
          sscanf(data, "%s %s %s", func[0], func[1], func[2]);
          sm->m_State |= sGetOp(func[0], ef) << FSS_STENCFAIL_SHIFT;
          sm->m_State |= sGetOp(func[1], ef) << FSS_STENCZFAIL_SHIFT;
          sm->m_State |= sGetOp(func[2], ef) << FSS_STENCPASS_SHIFT;
        }
        break;
    }
  }
}


void CShader::mfCompileState(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eStencil = 1, eBlend, eAlphaGen, eRGBGen, eCullFront, eDepthFunc, eAlphaFunc, eDepthMask, eDepthWrite, eNoDepthTest, eNoColorMask, eColorMask, eClampTexCoords, eClearStencil, ePolyLine, eNoCull};
  static tokenDesc commands[] =
  {
    {eStencil, "Stencil"},
    {eClearStencil, "ClearStencil"},
    {eBlend, "Blend"},
    {eNoCull, "NoCull"},
    {eCullFront, "CullFront"},
    {eDepthFunc, "DepthFunc"},
    {eAlphaFunc, "AlphaFunc"},
    {eDepthMask, "DepthMask"},
    {eDepthWrite, "DepthWrite"},
    {eNoDepthTest, "NoDepthTest"},
    {ePolyLine, "PolyLine"},
    {eNoColorMask, "NoColorMask"},
    {eColorMask, "ColorMask"},
    {eClampTexCoords, "ClampTexCoords"},
    {eAlphaGen, "AlphaGen"},
    {eRGBGen, "RGBGen"},
    {0,0}
  };

  ef->m_State = new SEfState;
  ef->m_State->m_Stencil = NULL;
  ef->m_State->m_Flags = 0;
  ef->m_State->m_bClearStencil = false;
  ef->m_State->m_State = 0;
  ef->m_State->m_FixedColor[0] = ef->m_State->m_FixedColor[1] = ef->m_State->m_FixedColor[2] = ef->m_State->m_FixedColor[3] = 0;
  int dm = GS_DEPTHWRITE;
  uint bfsrc = 0;
  uint bfdst = 0;
  int af = 0;
  int ctc = 0;
  int df = 0;


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
      case eStencil:
        mfCompileStencil(ef, data);
        break;

      case eClearStencil:
        ef->m_State->m_bClearStencil = true;
        break;

      case eBlend:
        sGetBlend(data, ef, &bfsrc, &bfdst);
        dm = 0;
        ef->m_State->m_Flags = ESF_STATE;
        break;

      case eAlphaGen:
        ef->m_State->m_Flags |= ESF_ALPHAGEN;
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for AlphaGen in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        if (!stricmp(data, "Fixed"))
        {
          ef->m_State->m_eEvalAlpha = eEALPHA_Fixed;
          if (!params || !params[0])
          {
            Warning( 0,0,"Warning: missing AlphaGen Fixed value in Shader '%s' (use 1.0)\n", ef->m_Name.c_str());
            ef->m_State->m_FixedColor[3] = 255;
          }
          else
            ef->m_State->m_FixedColor[3] = (int)(shGetFloat(params) * 255.0);
        }
        break;

      case eRGBGen:
        ef->m_State->m_Flags |= ESF_RGBGEN;
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for AlphaGen in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        if (!stricmp(data, "Fixed"))
        {
          ef->m_State->m_eEvalRGB = eERGB_Fixed;
          if (!params || !params[0])
          {
            Warning( 0,0,"Warning: missing RgbGen Fixed value in Shader '%s' (use 1.0)\n", ef->m_Name.c_str());
            ef->m_State->m_FixedColor[0] = 255;
            ef->m_State->m_FixedColor[1] = 255;
            ef->m_State->m_FixedColor[2] = 255;
          }
          else
          {
            CFColor col;
            shGetColor(params, col);
            COLCONV(col);
            ef->m_State->m_FixedColor[0] = (int)(col[0] * 255.0);
            ef->m_State->m_FixedColor[1] = (int)(col[1] * 255.0);
            ef->m_State->m_FixedColor[2] = (int)(col[2] * 255.0);
            ef->m_State->m_FixedColor[3] = (int)(col[3] * 255.0);
          }
        }
        else
          Warning( 0,0,"Warning: unknown rgbGen parameter '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eColorMask:
        ef->m_State->m_Flags |= ESF_COLORMASK;
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing parameters for ColorMask in Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          break;
        }
        if (!params || !params[0])
        {
          Warning( 0,0,"Warning: missing ColorMask values in Shader '%s' (use 1.0)\n", ef->m_Name.c_str());
          ef->m_State->m_ColorMask[0] = 1;
          ef->m_State->m_ColorMask[1] = 1;
          ef->m_State->m_ColorMask[2] = 1;
        }
        else
        {
          CFColor col;
          shGetColor(params, col);
          ef->m_State->m_ColorMask[0] = (int)(col[0]);
          ef->m_State->m_ColorMask[1] = (int)(col[1]);
          ef->m_State->m_ColorMask[2] = (int)(col[2]);
          ef->m_State->m_ColorMask[3] = (int)(col[3]);
        }
        break;

      case eDepthMask:
      case eDepthWrite:
        ef->m_State->m_Flags |= ESF_STATE;
        if (data && data[0])
        {
          if (shGetBool(data))
            dm |= GS_DEPTHWRITE;
          else
            dm &= ~GS_DEPTHWRITE;
        }
        else
          dm |= GS_DEPTHWRITE;
        break;

      case eNoDepthTest:
        ef->m_State->m_Flags |= ESF_STATE;
        dm |= GS_NODEPTHTEST;
        dm &= ~GS_DEPTHWRITE;
        break;

      case eNoColorMask:
        ef->m_State->m_Flags |= ESF_STATE;
        af |= GS_NOCOLMASK;
        break;

      case ePolyLine:
        ef->m_State->m_Flags |= ESF_POLYLINE;
        break;

      case eNoCull:
        ef->m_State->m_Flags |= ESF_NOCULL;
        break;

      case eCullFront:
        ef->m_State->m_Flags |= ESF_CULLFRONT;
        break;

      case eAlphaFunc:
        ef->m_State->m_Flags |= ESF_STATE;
        if (!stricmp(data, "GT0"))
          af = GS_ALPHATEST_GREATER0;
        else
        if (!stricmp(data, "LT128"))
          af = GS_ALPHATEST_LESS128;
        else
        if (!stricmp(data, "GE128"))
          af = GS_ALPHATEST_GEQUAL128;
        else
          Warning( 0,0,"Warning: invalid AlphaFunc name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eDepthFunc:
        ef->m_State->m_Flags |= ESF_STATE;
        if (!stricmp(data, "LEqual"))
          df = 0;
        else
        if (!stricmp(data, "Equal"))
          df = GS_DEPTHFUNC_EQUAL;
        else
        if (!strnicmp(data, "Great", 5))
          df = GS_DEPTHFUNC_GREAT;
        else
          Warning( 0,0,"Warning: unknown DepthFunc name '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eClampTexCoords:
        ef->m_State->m_Flags |= ESF_STATE;
        ctc |= GS_TEXPARAM_CLAMP;
        break;
    }
  }

  if (ef->m_State->m_Flags & ESF_STATE)
    dm |= GS_ADDITIONALSTATE;
  ef->m_State->m_State = dm | bfsrc | bfdst | af | ctc | df;
}

SShader *CShader::mfCompile(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  int n = 0;

  m_CurShader = ef;

  enum {eParams = 1, eRenderParams, ePublic, eCorona, eState, eLayer, eFlare, eSunFlares, eCurFlare, eClientEffect, eOrient, eLightStyle, eHW, eTemplates, eEvalLight};
  static tokenDesc commands[] =
  {
    {ePublic, "Public"},
    {eParams, "Params"},
    {eRenderParams, "RenderParams"},
    {eState, "State"},
    {eFlare, "Flare"},
    {eLayer, "Layer"},
    {eSunFlares, "SunFlares"},
    {eCurFlare, "CurFlare"},
    {eClientEffect, "ClientEffect"},
    {eOrient, "Orient"},
    {eLightStyle, "LightStyle"},
    {eHW, "HW"},
    {eTemplates, "Templates"},
    {eEvalLight, "EvalLight"},

    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eParams:
        mfCompileParams(ef, params);
        break;

      case eRenderParams:
        mfCompileRenderParams(ef, params);
        break;

      case ePublic:
        mfCompilePublic(ef, params);
        break;

      case eFlare:
        {
          CREFlareProp *ps = new CREFlareProp;
          if (ps->mfCompile(ef, params))
            ef->m_REs.AddElem(ps);
          else
            delete ps;
        }
        break;

      case eSunFlares:
        mfCompileSunFlares(ef, name, params);
        break;

      case eState:
        mfCompileState(ef, params);
        break;

      case eEvalLight:
        mfCompileEvalLight(ef, params);
        break;

      case eCurFlare:
        if (!params || !params[0])
        {
          Warning( 0,0,"Warning: missing CurFlare name in Shader '%s' (skipping)!\n", ef->m_Name.c_str());
          break;
        }
        CSunFlares::m_CurFlares = CSunFlares::mfForName(params);
        ef->m_Flares = CSunFlares::m_CurFlares;
        if (!CSunFlares::m_CurFlares)
        {
          Warning( 0,0,"Warning: Can't find Global Flare '%s' in Shader '%s' (skipping)!\n", params, ef->m_Name.c_str());
          break;
        }
        ef->m_Flags2 |= EF2_HASSUNFLARE;
        break;

      case eHW:
        {
          SShaderTechnique *hw = mfCompileHW(ef, params, ef->m_HWTechniques.Num());
          if (hw)
          {
            hw->m_Id = ef->m_HWTechniques.Num();
            ef->m_HWTechniques.AddElem(hw);
          }
        }
        break;

      case eClientEffect:
        mfClEfCompile(ef, params, name);
        break;

      case eOrient:
        if (!name || !name[0])
        {
          Warning( 0,0,"Warning: missing Orients number in Orient Shader (skipping)!\n");
          break;
        }
        mfCompileOrient(ef, shGetInt(name), params);
        ef->m_Flags |= EF_ORIENT | EF_SYSTEM;
        break;

      case eLightStyle:
        if (!name || !name[0])
        {
          Warning( 0,0,"Warning: missing LightStyle number in LightStyles Shader (skipping)!\n");
          break;
        }
        mfCompileLightStyle(ef, shGetInt(name), params);
        break;

      case eLayer:
        if (mfCompileLayer(ef, n, params, NULL))
        {
          if (n > 0)
          {
            if (!(ef->m_Passes[n].m_RenderState & GS_BLEND_MASK))
              Warning( 0,0,"Warning: Shader '%s' has opaque maps defined after layer 0!!!\n", ef->m_Name.c_str());

            if ((ef->m_Passes[n].m_RenderState & GS_DEPTHWRITE))
              Warning( 0,0,"Warning: Shader '%s' has depthmask enabled after layer 0!!!\n", ef->m_Name.c_str());
          }
          n++;
        }
        else
        {
          ef->m_Passes.SetNum(n);
          ef->m_Passes.Realloc();
        }
        break;

      case eTemplates:
        mfCompileTemplate(ef, params);
        break;
    }
  }
  ef->m_HWTechniques.Shrink();

  if (!ef->m_Passes.Num() && !ef->m_HWTechniques.Num() && !(ef->m_Flags & EF_SKY) && !(ef->m_Flags & EF_FOGSHADER))
  {
    mfConstruct(ef);
    return ef;
  }

  ef->m_Flags |= EF_COMPILEDLAYERS;

  mfConstruct(ef);

  return ef;
}

SShader *CShader::mfCompileShader(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;

  enum {eShader=1, eVersion, eSubrScript};
  static tokenDesc commands[] =
  {
    {eShader, "Shader"},
    {eVersion, "Version"},
    {eSubrScript, "SubrScript"},

    {0,0}
  };

  SShader *ef1 = NULL;
  float ver;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eShader:
        ef1 = mfCompile(ef, params);
        break;

    case eVersion:
      ver = shGetFloat(params);
      if (ver != SHADER_VERSION)
      {
        Warning( 0,0,"Warning: Shader Script version (%f) must be %f\n", ver, SHADER_VERSION);
        return NULL;
      }
      break;
    }
  }

  return ef1;
}

//===========================================================================

bool CShader::mfCompileSequence(SShader *ef, SShaderTexUnit *tl, int nLayer, char *scr, ETexType eTT, int Flags, int Flags2, float fBumpAmount)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  char mapname[256];

  float time = 0;
  int n;
  int rn = 0;
  bool loop = 0;


  enum {eMaps = 1, eTime, eRandom, eLoop};
  static tokenDesc commands[] =
  {
    {eMaps, "Maps"},
    {eTime, "Time"},
    {eRandom, "Random"},
    {eLoop, "Loop"},
    {0,0}
  };

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
      case eMaps:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing Maps Sequence argument for layer %d in Shader '%s'\n", nLayer, ef->m_Name.c_str());
          return false;
        }
        strcpy(mapname, data);
        break;

      case eTime:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing Time Sequence argument for layer %d in Shader '%s'\n", nLayer, ef->m_Name.c_str());
          return false;
        }
        time = shGetFloat(data);
        break;

      case eLoop:
        loop = 1;
        break;

      case eRandom:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: missing Random Sequence argument for layer %d in Shader '%s'\n", nLayer, ef->m_Name.c_str());
          return false;
        }
        rn = shGetInt(data);
        break;

    }
  }

  if (!mapname[0])
    return false;

  STexAnim *at = new STexAnim;
  memset(at, 0, sizeof(STexAnim));
  tl->m_AnimInfo = at;

//  at->Sequence = mapname;

  if (strstr(mapname, "*.*"))
    n = mfReadAllImgFiles(ef, tl, at, mapname);
  else
  if (strchr(mapname, '#') || strchr(mapname, '$'))
    n = mfReadTexSequence(ef, tl->m_AnimInfo->m_TexPics, mapname, eTT, Flags, Flags2, fBumpAmount);
  else
    tl->m_TexPic = (STexPic*)gRenDev->EF_LoadTexture(mapname, Flags, Flags2, eTT, fBumpAmount);

  if (!tl->m_TexPic && !tl->m_AnimInfo->m_TexPics.Num())
  {
    delete at;
    tl->m_AnimInfo = NULL;
    return false;
  }

  at->m_NumAnimTexs = n;

  STexPic *tx;
  if (at->m_TexPics.Num())
  {
    int rni;
    if (rn >= 0)
      rni = -rn;
    else
    {
      int nn = min(n, -rn);
      rni = rand() % nn;
    }

    tx = at->m_TexPics[0];
    int nn = 0;
    while (rni)
    {
      if (nn >= at->m_TexPics.Num())
      {
        tx = at->m_TexPics[0];
        break;
      }
      tx = at->m_TexPics[nn];
      rni--;
      nn++;
    }
    tl->m_TexPic = tx;
    if (loop)
      at->m_bLoop = loop;
  }
  at->m_Rand = rn;
  at->m_Time = time;

  return true;
}


int CShader::mfReadTexSequence(SShader *ef, TArray<STexPic *>& tl, const char *na, byte eTT, int Flags, int Flags2, float fAmount1, float fAmount2)
{
  char prefix[64];
  char postfix[64];
  char *nm;
  int i, j, l, m;
  char nam[64];
  int n;
  STexPic *tx, *tp;
  int startn, endn, nums;
  char name[128];

  tx = NULL;

  StripExtension(na, name);

  char chSep = '#';
  nm = strchr(name, chSep);
  if (!nm)
  {
    nm = strchr(name, '$');
    if (!nm)
      return 0;
    char chSep = '$';
  }

  float fSpeed = 0.05f;
  {
    char nName[128];
    strcpy(nName, name);
    nm = strchr(nName, '(');
    if (nm)
    {
      name[nm-nName] = 0;
      char *speed = &nName[nm-nName+1];
      if(nm=strchr(speed, ')'))
        speed[nm-speed] = 0;
      fSpeed = (float)atof(speed);
    }
  }

  j = 0;
  n = 0;
  l = -1;
  m = -1;
  while (name[n])
  {
    if (name[n] == chSep)
    {
      j++;
      if (l == -1)
        l = n;
    }
    else
    if (l >= 0 && m < 0)
      m = n;
    n++;
  }
  if (!j)
    return 0;

  strncpy(prefix, name, l);
  prefix[l] = 0;
  char dig[64];
  l = 0;
  if (m < 0)
  {
    startn = 0;
    endn = 999;
    postfix[0] = 0;
  }
  else
  {
    while(isdigit(name[m]))
    {
      dig[l++] = name[m];
      m++;
    }
    dig[l] = 0;
    startn = strtol(dig, NULL, 10);
    m++;

    l = 0;
    while(isdigit(name[m]))
    {
      dig[l++] = name[m];
      m++;
    }
    dig[l] = 0;
    endn = strtol(dig, NULL, 10);

    strcpy(postfix, &name[m]);
  }

  nums = endn-startn+1;

  n = 0;
  char frm[64];
  char frd[4];

  frd[0] = j + '0';
  frd[1] = 0;

  strcpy(frm, "%s%.");
  strcat(frm, frd);
  strcat(frm, "d%s");
  for (i=0; i<nums; i++)
  {
    sprintf(nam, frm, prefix, startn+i, postfix);
    tp = (STexPic*)gRenDev->EF_LoadTexture(nam, Flags, Flags2, eTT, fAmount1, fAmount2);
    if (!tp || (tp->m_Flags & FT_NOTFOUND))
    {
      if (tp)
        tp->Release(0);
      break;
    }
    tl.AddElem(tp);
    tp->m_fAnimSpeed = fSpeed;
    n++;
  }

  return n;
}

int CShader::mfReadAllImgFiles(SShader *ef, SShaderTexUnit *tl, STexAnim *ta, char *name)
{
  struct __finddata64_t fileinfo;
  intptr_t handle;
  char d[1024];
  char drn[1024], ddr[1024];
  char dirn[1024];
  char en[64];
  char fn[1024], fn1[1024];
  int ns = 0;
  STexPic *tp, *tx;
  char *path, *pt, nm[1024];

  int fl = 0;
  tx = NULL;

  pt = NULL;
  path = strtok(name, "%");
  if (path)
  {
    if (!stricmp(path, "ModelsPath"))
      pt = m_ModelsPath;
    else
    if (!stricmp(path, "TexturesPath"))
      pt = m_TexturesPath;
    else
    if (!stricmp(path, "SystemPath"))
      pt = m_SystemPath;

    if (pt)
      UsePath((char *)ef->m_Name.c_str(), pt, nm);
    else
      strcpy(nm, path);
  }
  else
  {
    strcpy(nm, name);
    fl = 1;
  }

  ConvertUnixToDosName( d, nm );

  _splitpath(d, drn, dirn, fn, en);

  strcpy(ddr, dirn);

  if (!fl)
    strcat(dirn, "*.*");

  handle = _findfirst64 (dirn, &fileinfo);

  if (handle == -1)
    return 0;

  do
  {
    if (fileinfo.attrib & _A_SUBDIR)
      continue;
    if (fileinfo.name[0] == '.')
      continue;

    strcpy(fn1, drn);
    strcat(fn1, ddr);
    strcat(fn1, fileinfo.name);

    tp = (STexPic*)gRenDev->EF_LoadTexture(fn1, 0, 0, eTT_Base);
    if ( !tp )
      continue;
    ta->m_TexPics.AddElem(tp);
    ns++;
  } while (_findnext64( handle, &fileinfo ) != -1);

  return ns;
}

//===========================================================================

//#include "RendElements/ProcTree/Tree.h"
//#include "RendElements/Terrain/CTerrainRE.h"

void CShader::mfClEfCompile(SShader *ef, char *scr, char *name)
{
  if (!name || !name[0])
  {
    Warning( 0,0,"Warning: missing parameter in ClientEffect for Shader '%s' (skipping)\n", ef->m_Name.c_str());
    return;
  }

  if (!stricmp(name, "PolyBlend"))
  {
    CREPolyBlend *ps = new CREPolyBlend;
    if (ps->mfCompile(ef, scr))
      ef->m_REs.AddElem(ps);
    else
      delete ps;
    return;
  }
  else
  if (!stricmp(name, "AnimPolyBlend"))
  {
    CREAnimPolyBlend *ps = new CREAnimPolyBlend;
    if (ps->mfCompile(ef, scr))
      ef->m_REs.AddElem(ps);
    else
      delete ps;
    return;
  }
  else
  if (!stricmp(name, "ParticleSpray"))
  {
#ifdef DEBUGALLOC
#undef new
#endif
    CREParticleSpray *ps = new CREParticleSpray;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
    if (ps->mfCompile(ef, scr))
      ef->m_REs.AddElem(ps);
    else
      delete ps;
    return;
  }
  else
  if (!stricmp(name, "Corona"))
  {
    CREFlare *ps = new CREFlare;
    if (ps->mfCompile(ef, scr))
    {
      ef->m_REs.AddElem(ps);
      ef->m_nPreprocess |= FSPR_CORONA;
    }
    else
      delete ps;
    return;
  }
  else
  if (!stricmp(name, "Beam"))
  {
    CREBeam *ps = new CREBeam;
    if (ps->mfCompile(ef, scr))
    {
      ef->m_REs.AddElem(ps);
    }
    else
      delete ps;
    return;
  }
  else
  if (!stricmp(name, "Tree"))
  {
    /*CRETree *ps = new CRETree;
    if (ps->mfCompile(ef, scr))
      ef->m_RE = ps;
    else
      delete ps;*/
    return;
  }
  else
  if (!stricmp(name, "GeomPrefab") || !stricmp(name, "Prefab"))
  {
    CREPrefabGeom *ps = new CREPrefabGeom;
    if (ps->mfCompile(ef, scr))
      ef->m_REs.AddElem(ps);
    else
      delete ps;
    return;
  }
  else
  if (!stricmp(name, "Flare"))
  {
    CREFlareProp *ps = new CREFlareProp;
    if (ps->mfCompile(ef, scr))
      ef->m_REs.AddElem(ps);
    else
      delete ps;
  }
  else
  if (!stricmp(name, "Terrain"))
  {
    /*CRETerrain *ps = new CRETerrain;
    if (ps->mfCompile(ef, scr))
      ef->m_RE = ps;
    else
      delete ps;*/
  }
  else
  if (!stricmp(name, "SkyZone"))
  {
    CRESkyZone *ps = NULL;//new CRESkyZone;
    if (ps->mfCompile(ef, scr))
      ef->m_REs.AddElem(ps);
    else
      delete ps;
    return;
  }
  else
  if (!stricmp(name, "Ocean"))
  {
#ifdef DEBUGALLOC
#undef new
#endif
    CREOcean *ps = new CREOcean;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
    if (ps->mfCompile(ef, scr))
      ef->m_REs.AddElem(ps);
    else
      delete ps;
    return;
  }
  else
    Warning( 0,0,"Warning: unknown ClientEffect (%s) in Shader '%s' (skipping)\n", name, ef->m_Name.c_str());
}


//=========================================================================

TArray<CSunFlares *> CSunFlares::m_SunFlares;
CSunFlares *CSunFlares::m_CurFlares = NULL;

CSunFlares *CSunFlares::mfForName(char *name)
{
  int i;

  for (i=0; i<m_SunFlares.Num(); i++)
  {
    if (!stricmp(m_SunFlares[i]->m_Name, name))
      return m_SunFlares[i];
  }
  return NULL;
}

bool CShader::mfCompileFlare(SShader *ef, SSunFlare *fl, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  char namefl[64];

  enum {eName = 1, eLocation, eScale, eColor};
  static tokenDesc commands[] =
  {
    {eName, "Name"},
    {eLocation, "Location"},
    {eScale, "Scale"},
    {eColor, "Color"},

    {0,0}
  };

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
      case eName:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: Missing flare name in system Shader '%s'\n", ef->m_Name.c_str());
          return false;
        }
        strcpy(namefl, data);
        break;

      case eLocation:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: Missing Location arg in system Shader '%s'\n", ef->m_Name.c_str());
          return false;
        }
        fl->m_Loc = shGetFloat(data);
        break;

      case eScale:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: Missing Scale arg in system Shader '%s'\n", ef->m_Name.c_str());
          return false;
        }
        fl->m_Scale = shGetFloat(data);
        break;

      case eColor:
        if (!data || !data[0])
        {
          Warning( 0,0,"Warning: Missing Color arg in system Shader '%s'\n", ef->m_Name.c_str());
          return false;
        }
        shGetColor(data, fl->m_Color);
        COLCONV(fl->m_Color);
        break;
    }
  }

  fl->m_Tex = (STexPic*)gRenDev->EF_LoadTexture(namefl, FT_NOMIPS, 0, eTT_Base);

  return true;
}

bool CShader::mfCompileSunFlares(SShader *ef, char *namefl, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  CSunFlares *sf;
  SSunFlare flares[64];
  int numfl = 0;

  enum {eFlare = 1};
  static tokenDesc commands[] =
  {
    {eFlare, "Flare"},

    {0,0}
  };

  sf = new CSunFlares;
  strcpy(sf->m_Name, namefl);
  CSunFlares::m_SunFlares.AddElem(sf);

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
      case eFlare:
        if (mfCompileFlare(ef, &flares[numfl], params))
        {
          numfl++;
          if (numfl >= 64)
          {
            Warning( 0,0,"Warning: Too many flares in system Shader '%s'\n", ef->m_Name.c_str());
            goto dl;
          }
        }
        break;
    }
  }
dl:
  if (numfl>0)
  {
    sf->m_Flares = new SSunFlare[numfl];
    for (int i=0; i<numfl; i++)
    {
      memcpy(&sf->m_Flares[i], &flares[i], sizeof(SSunFlare));
      if (sf->m_Flares[i].m_Tex)
        sf->m_Flares[i].m_Tex->AddRef();
    }
    sf->m_NumFlares = numfl;
    ef->m_Flags |= EF_SUNFLARES;
    return true;
  }

  return false;
}

