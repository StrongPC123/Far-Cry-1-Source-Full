/*=============================================================================
  GLShaders.cpp : OpenGL specific shaders functions implementation.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "I3dengine.h"

#include "nvparse/nvparse.h"
#include "GLCGVProgram.h"
#include "GLCGPShader.h"


#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//============================================================================

void CShader::mfCompileVarsPak(char *scr, TArray<CVarCond>& Vars, SShader *ef)
{
  char *var;
  char *val;

  while ((shGetVar (&scr, &var, &val)) >= 0)
  {
    if (!var)
      continue;

    ICVar *vr = iConsole->GetCVar(var);
    if (!vr)
    {
      iLog->Log("Warning: Couldn't find console variable '%s' in shader '%s'\n", var, ef->m_Name.c_str());
      continue;
    }
    float v = shGetFloat(val);
    CVarCond vc;
    vc.m_Var = vr;
    vc.m_Val = v;
    Vars.AddElem(vc);
  }
}

bool CShader::mfCompileHWShadeLayer(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers)
{
  char* name;
  long cmd;
  char *params;
  SShaderPassHW *sm;
  CPShader *ps;
  char *data;

  int n = Layers.Num();
  Layers.ReserveNew(n+1);
  sm = &Layers[n];
  sm->m_RenderState = GS_DEPTHWRITE;

  enum {eLayer = 1, eLightType, eCGVProgram, eCGPShader, eLMVertexLight, eLMOnlyMaterialAmbient, eCGVPParam, eCGPSParam, eCGPSParmRect, eDeformVertexes, eArray, eMatrix, eNoLight, eLMIgnoreLights, eLMIgnoreProjLights, eLMNoAmbient, eLMBump, eLMNoSpecular, eLMNoAddSpecular, eLMDivideAmb4, eLMDivideAmb2, eLMDivideDif4, eLMDivideDif2, ePolyOffset, eLMNoAlpha, eColorMaterial, eRendState, eSecondPassRendState, eNoBump, e1Samples, e2Samples, e3Samples, e4Samples, eAffectMask, eOcclusionMap};
  static tokenDesc commands[] =
  {
    {eLayer, "Layer"},
    {eCGVProgram, "CGVProgram"},
    {eLightType, "LightType"},
    {eDeformVertexes, "DeformVertexes"},
    {eNoLight, "NoLight"},
    {eCGPShader, "CGPShader"},
    {eCGPSParam, "CGPSParam"},
    {eCGPSParmRect, "CGPSParmRect"},
    {eCGVPParam, "CGVPParam"},
    {eArray, "Array"},
    {eMatrix, "Matrix"},
    {eRendState, "RendState"},
    {eSecondPassRendState, "SecondPassRendState"},

    {ePolyOffset, "PolyOffset"},
    {eNoBump, "NoBump"},
    {eAffectMask, "AffectMask"},

    {eOcclusionMap, "OcclusionMap"},

    {eLMIgnoreLights, "LMIgnoreLights"},
    {eLMIgnoreProjLights, "LMIgnoreProjLights"},
    {eLMNoAmbient, "LMNoAmbient"},
    {eLMNoAlpha, "LMNoAlpha"},
    {eColorMaterial, "ColorMaterial"},
    {eLMBump, "LMBump"},
    {eLMNoSpecular, "LMNoSpecular"},
    {eLMNoAddSpecular, "LMNoAddSpecular"},
    {eLMVertexLight, "LMVertexLight"},
    {eLMDivideAmb4, "LMDivideAmb4"},
    {eLMDivideAmb2, "LMDivideAmb2"},
    {eLMDivideDif4, "LMDivideDif4"},
    {eLMDivideDif2, "LMDivideDif2"},
    {eLMOnlyMaterialAmbient, "LMOnlyMaterialAmbient"},

    {e1Samples, "1Samples"},
    {e2Samples, "2Samples"},
    {e3Samples, "3Samples"},
    {e4Samples, "4Samples"},

    {0,0}
  };

  bool bVertLight = false;
  int nAffectMask = 0;
  int nl;
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
      case eRendState:
        sm->m_RenderState = mfCompileRendState(ef, sm, params);
        break;

      case eSecondPassRendState:
        sm->m_SecondRenderState = mfCompileRendState(ef, sm, params);
        sm->m_Flags |= SHPF_USEDSECONDRS;
        break;

      case eAffectMask:
        nAffectMask = shGetHex(data);
        break;

      case eCGVProgram:
        {
          if (!(gRenDev->GetFeatures() & (RFT_HW_VS)))
          {
            iLog->Log("Error: Attempt to load vertex shader '%s' for shader '%s'", params, ef->m_Name.c_str());
          }
          else
          {
            ef->m_Flags |= EF_HASVSHADER;
            uint nMask = 0;
            if (nAffectMask)
              nMask = mfScriptPreprocessorMask(ef, pCurCommand-m_pCurScript);
            sm->m_VProgram = CVProgram::mfForName(params, nMask & nAffectMask);
            if (!ef->m_DefaultVProgram && sm->m_VProgram && (sm->m_VProgram->m_Flags & VPFI_DEFAULTPOS))
              ef->m_DefaultVProgram = sm->m_VProgram;
            if (nAffectMask)
              sm->m_VProgram->m_Macros = m_Macros;
          }
        }
        break;

      case eNoLight:
        sm->m_LMFlags |= LMF_DISABLE;
        break;

      case eLayer:
        if (name)
          nl = atoi(name);
        else
          nl = 0;
        mfCompileLayer(ef, nl, params, sm);
        break;

      case ePolyOffset:
        sm->m_LMFlags |= LMF_POLYOFFSET;
        break;

      case eNoBump:
        sm->m_LMFlags |= LMF_NOBUMP;
        break;

      case eOcclusionMap:
        sm->m_LMFlags |= LMF_USEOCCLUSIONMAP;
        break;

      case eLightType:
        if (!strnicmp(data, "Direct", 6))
          sm->m_LightFlags |= DLF_DIRECTIONAL;
        else
        if (!strnicmp(data, "Point", 5))
          sm->m_LightFlags |= DLF_POINT;
        else
        if (!strnicmp(data, "Project", 7))
        {
          ef->m_Flags |= EF_USEPROJLIGHTS;
          sm->m_LightFlags |= DLF_PROJECT;
        }
        else
        if (!strnicmp(data, "OnlySpec", 8))
          sm->m_LightFlags |= DLF_LM;
        else
          iLog->Log("Warning: Unknown light type for light pass in effector '%s'\n", ef->m_Name.c_str());
        break;

      case eDeformVertexes:
        {
          if (!sm->m_Deforms)
            sm->m_Deforms = new TArray<SDeform>;
          int i = sm->m_Deforms->Num();
          sm->m_Deforms->ReserveNew(i+1);
          SDeform *df = &sm->m_Deforms->Get(i);
          mfCompileDeform(ef, df, name, params);
        }
        break;
        
      case eCGPShader:
        if (!(gRenDev->GetFeatures() & (RFT_HW_RC | RFT_HW_TS | RFT_HW_PS20)))
        {
          iLog->Log("Error: Attempt to load pixel shader '%s' for shader '%s'", params, ef->m_Name.c_str());
        }
        else
        {
          ef->m_Flags3 |= EF3_HASPSHADER;
          uint nMask = 0;
          if (nAffectMask)
            nMask = mfScriptPreprocessorMask(ef, pCurCommand-m_pCurScript);
          ps = CPShader::mfForName(params, nMask & nAffectMask);
          if (ps)
          {
            if (nAffectMask)
              ps->m_Macros = m_Macros;
            sm->m_FShader = ps;
          }
        }
        break;

      case eCGPSParam:
        if (!sm->m_CGFSParamsNoObj)
          sm->m_CGFSParamsNoObj = new TArray<SCGParam4f>;
        mfCompileCGParam(params, ef, sm->m_CGFSParamsNoObj);
        break;

      case eCGPSParmRect:
        {
          if (!sm->m_CGFSParamsNoObj)
            sm->m_CGFSParamsNoObj = new TArray<SCGParam4f>;
          int n = sm->m_CGFSParamsNoObj->Num();
          mfCompileCGParam(params, ef, sm->m_CGFSParamsNoObj);
          for (int i=n; i<sm->m_CGFSParamsNoObj->Num(); i++)
          {
            sm->m_CGFSParamsNoObj->Get(i).m_dwBind |= 0x80000;
          }
        }
        break;

      case eCGVPParam:
        mfCompileCGParam(params, ef, &sm->m_VPParamsNoObj);
        break;

      case eArray:
        mfCompileArrayPointer(sm->m_Pointers, params, ef);
        break;

      case eMatrix:
        if (!sm->m_MatrixOps)
          sm->m_MatrixOps = new TArray<SMatrixTransform>;
        mfCompileMatrixOp(sm->m_MatrixOps, params, name, ef);
        break;

      case eLMNoSpecular:
        sm->m_LMFlags |= LMF_NOSPECULAR;
        break;
      case eLMNoAddSpecular:
        sm->m_LMFlags |= LMF_NOADDSPECULAR;
        break;
      case eLMDivideAmb4:
        sm->m_LMFlags |= LMF_DIVIDEAMB4;
        break;
      case eLMDivideAmb2:
        sm->m_LMFlags |= LMF_DIVIDEAMB2;
        break;
      case eLMDivideDif4:
        sm->m_LMFlags |= LMF_DIVIDEDIFF4;
        break;
      case eLMDivideDif2:
        sm->m_LMFlags |= LMF_DIVIDEDIFF2;
        break;
      case eLMOnlyMaterialAmbient:
        sm->m_LMFlags |= LMF_ONLYMATERIALAMBIENT;
        break;

      case e1Samples:
        sm->m_LMFlags |= LMF_1SAMPLES;
        break;
      case e2Samples:
        sm->m_LMFlags |= LMF_2SAMPLES;
        break;
      case e3Samples:
        sm->m_LMFlags |= LMF_3SAMPLES;
        break;
      case e4Samples:
        sm->m_LMFlags |= LMF_4SAMPLES;
        break;

      case eLMIgnoreLights:
        sm->m_LMFlags |= LMF_IGNORELIGHTS;
        break;

      case eLMIgnoreProjLights:
        sm->m_LMFlags |= LMF_IGNOREPROJLIGHTS;
        break;

      case eLMNoAmbient:
        sm->m_LMFlags |= LMF_NOAMBIENT;
        break;

      case eLMNoAlpha:
        sm->m_LMFlags |= LMF_NOALPHA;
        break;

      case eLMBump:
        sm->m_LMFlags |= LMF_BUMPMATERIAL;
        break;

      case eLMVertexLight:
        bVertLight = true;
        break;

      case eColorMaterial:
        sm->m_LMFlags |= LMF_COLMAT_AMB;
        break;
    }
  }
  if (sm->m_VProgram && !bVertLight)
    sm->m_LMFlags |= LMF_BUMPMATERIAL;
  if (ef->m_Flags & EF_TEMPLNAMES)
    sm->m_SecondRenderState = GS_BLSRC_ONE | GS_BLDST_ONE | GS_DEPTHFUNC_EQUAL;

  if (n > 0 && !sm->m_LightFlags)
  {
    if (!(sm->m_SecondRenderState & (GS_BLEND_MASK | GS_COLMASKONLYRGB | GS_COLMASKONLYALPHA)))
      Warning( 0,0,"Shader '%s' has opaque maps defined after pass 0!!!\n", ef->m_Name.c_str());
    
    if (sm->m_SecondRenderState & GS_DEPTHWRITE)
    {
      if (!(sm->m_SecondRenderState & (GS_COLMASKONLYRGB | GS_COLMASKONLYALPHA)))
        Warning( 0,0,"Shader '%s' has depthwrite enabled after pass 0!!!\n", ef->m_Name.c_str());
    }
  }
  if (!(ef->m_nPreprocess & FSPR_SCANLCM))
  {
    for (int i=0; i<sm->m_VPParamsNoObj.Num(); i++)
    {
      if (sm->m_VPParamsNoObj[i].m_Comps[0] && sm->m_VPParamsNoObj[i].m_Comps[0]->m_eType == EParamComp_EnvColor)
      {
        ef->m_nPreprocess |= FSPR_SCANLCM;
        break;
      }
    }
  }
  mfCheckObjectDependParams(&sm->m_VPParamsNoObj, &sm->m_VPParamsObj);
  if (sm->m_CGFSParamsNoObj)
  {
    TArray<SCGParam4f>* PObj = new TArray<SCGParam4f>;
    mfCheckObjectDependParams(sm->m_CGFSParamsNoObj, PObj);
    if (PObj->Num())
      sm->m_CGFSParamsObj = PObj;
    else
      delete PObj;
  }
  sm->m_VPParamsNoObj.Shrink();
  sm->m_VPParamsObj.Shrink();
  if (sm->m_CGFSParamsNoObj)
    sm->m_CGFSParamsNoObj->Shrink();

  if (sm->m_CGFSParamsObj)
    sm->m_CGFSParamsObj->Shrink();

  sm->m_Pointers.Shrink();

  return true;
}

void CShader::mfCompileLayers(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers, EShaderPassType eType)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eShadeLayer = 1, ePass};
  static tokenDesc commands[] =
  {
    {eShadeLayer, "ShadeLayer"},
    {ePass, "Pass"},

    {0,0}
  };

  if (eType == eSHP_DiffuseLight || eType == eSHP_SpecularLight)
    ef->m_Flags |= EF_USELIGHTS;

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
      case eShadeLayer:
      case ePass:
        {
          int n = Layers.Num();
          mfCompileHWShadeLayer(ef, params, Layers);
          Layers[n].m_ePassType = eType;
          if (Layers[n].m_CGFSParamsNoObj)
          {
            for (int i=0; i<Layers[n].m_CGFSParamsNoObj->Num(); i++)
            {
              SParam *pr = &Layers[n].m_CGFSParamsNoObj->Get(i);
              if (pr->m_Comps[0] && (pr->m_Comps[0]->m_eType == EParamComp_AmbLightColor || pr->m_Comps[0]->m_eType == EParamComp_WorldColor))
              {
                Layers[n].m_LightFlags |= DLF_HASAMBIENT;
                break;
              }
            }
          }
          if (Layers[n].m_CGFSParamsObj)
          {
            for (int i=0; i<Layers[n].m_CGFSParamsObj->Num(); i++)
            {
              SParam *pr = &Layers[n].m_CGFSParamsObj->Get(i);
              if (pr->m_Comps[0] && (pr->m_Comps[0]->m_eType == EParamComp_AmbLightColor || pr->m_Comps[0]->m_eType == EParamComp_WorldColor))
              {
                Layers[n].m_LightFlags |= DLF_HASAMBIENT;
                break;
              }
            }
          }
          Layers[n].m_LightFlags |= DLF_ACTIVE;
        }
        break;
    }
  }
}

void CShader::mfCompileHWConditions(SShader *ef, char *scr, SShaderTechnique *hs, int Id)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  
  enum {eSingleLight = 1, eMultipleLights, eRETexBind1, eRETexBind2, eRETexBind3, eRETexBind4, eRETexBind5, eRETexBind6, eRETexBind7, eNoLights, eOnlyDirectional, eHasProjectedLights, eInShadow, eVars, eSpecular, eBended, eAlphaBlended, eNoBump, eHasLM, eHasDOT3LM, eDepthMaps, eHasVColors, eHasAlphaTest, eHasAlphaBlend, eHeatVision, eHotAmbient, eInFogVolume, eHasEnvLCMap, eHasResource};
  static tokenDesc commands[] =
  {
    {eMultipleLights, "MultipleLights"},
    {eSingleLight, "SingleLight"},
    {eNoLights, "NoLights"},
    {eOnlyDirectional, "OnlyDirectional"},
    {eHasProjectedLights, "HasProjectedLights"},
    {eInShadow, "InShadow"},
    {eSpecular, "Specular"},
    {eBended, "Bended"},
    {eHasLM, "HasLM"},
    {eHasDOT3LM, "HasDOT3LM"},
    {eNoBump, "NoBump"},
    {eAlphaBlended, "AlphaBlended"},
    {eHasAlphaTest, "HasAlphaTest"},
    {eHasAlphaBlend, "HasAlphaBlend"},
    {eHasVColors, "HasVColors"},
    {eHeatVision, "HeatVision"},
    {eHotAmbient, "HotAmbient"},
    {eHasResource, "HasResource"},
    {eHasEnvLCMap, "HasEnvLCMap"},
    {eInFogVolume, "InFogVolume"},
    {eRETexBind1, "RETexBind1"},
    {eRETexBind2, "RETexBind2"},
    {eRETexBind3, "RETexBind3"},
    {eRETexBind4, "RETexBind4"},
    {eRETexBind5, "RETexBind5"},
    {eRETexBind6, "RETexBind6"},
    {eRETexBind7, "RETexBind7"},
    {eVars, "Vars"},
      
    {0,0}
  };

  ef->m_HWConditions.Expand(Id+1);
  SHWConditions *hc = &ef->m_HWConditions[Id];
  
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
      case eMultipleLights:
        hc->m_Flags |= SHCF_MULTIPLELIGHTS;
        break;

      case eSingleLight:
        hc->m_Flags |= SHCF_SINGLELIGHT;
        break;

      case eNoLights:
        hc->m_Flags |= SHCF_NOLIGHTS;
        break;

      case eHasVColors:
        hc->m_Flags |= SHCF_HASVCOLORS;
        break;

      case eHasAlphaTest:
        hc->m_Flags |= SHCF_HASALPHATEST;
        break;

      case eHasAlphaBlend:
        hc->m_Flags |= SHCF_HASALPHABLEND;
        break;

      case eHasEnvLCMap:
        hc->m_Flags |= SHCF_ENVLCMAP;
        break;

      case eHeatVision:
        hc->m_Flags |= SHCF_HEATVISION;
        break;

      case eHotAmbient:
        hc->m_Flags |= SHCF_HOTAMBIENT;
        break;

      case eBended:
        hc->m_Flags |= SHCF_BENDED;
        break;

      case eNoBump:
        hc->m_Flags |= SHCF_NOBUMP;
        break;

      case eRETexBind1:
      case eRETexBind2:
      case eRETexBind3:
      case eRETexBind4:
      case eRETexBind5:
      case eRETexBind6:
      case eRETexBind7:
        hc->m_Flags |= (cmd-eRETexBind1+1) << 12;
        ef->m_Flags2 |= EF2_REDEPEND;
        break;

      case eAlphaBlended:
        hc->m_Flags |= SHCF_ALPHABLENDED;
        break;

      case eOnlyDirectional:
        hc->m_Flags |= SHCF_ONLYDIRECTIONAL;
        break;

      case eHasProjectedLights:
        hc->m_Flags |= SHCF_HASPROJECTEDLIGHTS;
        break;

      case eInShadow:
        hc->m_Flags |= SHCF_INSHADOW;
        break;

      case eInFogVolume:
        hc->m_Flags |= SHCF_INFOGVOLUME;
        break;

      case eHasLM:
        hc->m_Flags |= SHCF_HASLM;
        ef->m_Flags3 |= EF3_HASLM;
        break;

      case eHasResource:
        hc->m_Flags |= SHCF_HASRESOURCE;
        break;

      case eHasDOT3LM:
        hc->m_Flags |= SHCF_HASDOT3LM;
        ef->m_Flags3 |= EF3_HASLM;
        break;

      case eSpecular:
        hc->m_Flags |= SHCF_SPECULAR;
        break;

      case eVars:
        {
          TArray<CVarCond> Vars;
          mfCompileVarsPak(params, Vars, ef);
          if (Vars.Num())
          {
            hc->m_NumVars = Vars.Num();
            hc->m_Vars = new CVarCond[hc->m_NumVars];
            memcpy(hc->m_Vars, &Vars[0], sizeof(CVarCond)*hc->m_NumVars);
          }
        }
        break;
    }
  }

  /*for (int i=0; i<m_HWConditions.Num(); i++)
  {
    if (hc->m_Flags != m_HWConditions[i]->m_Flags)
      continue;
    for (int j=0; j<hc->m_Vars.Num(); j++)
    {
      CVarCond *vr = &hc->m_Vars[j];
      for (int n=0; n<m_HWConditions[i]->m_Vars.Num(); n++)
      {
        if (m_HWConditions[i]->m_Vars[n].m_Var == vr->m_Var && m_HWConditions[i]->m_Vars[n].m_Val == vr->m_Val)
          break;
      }
      if (n == m_HWConditions[i]->m_Vars.Num())
        break;
    }
    if (j == hc->m_Vars.Num() && m_HWConditions[i]->m_Vars.Num() == hc->m_Vars.Num())
    {
      hs->m_Conditions = m_HWConditions[i];
      delete hc;
      break;
    }
  }
  if (i == m_HWConditions.Num())
  {
    m_HWConditions.AddElem(hc);
    hs->m_Conditions = m_HWConditions[m_HWConditions.Num()-1];
  }*/
}

SShaderTechnique *CShader::mfCompileHW(SShader *ef, char *scr, int Id)
{
  if (!m_CurEfsNum)
  {
    iLog->Log("Warning: Hardware section not allowed for Shader '%s'\n", ef->m_Name.c_str());
    return NULL;
  }

  char* name;
  long cmd;
  char *params;
  char *data;

#ifdef DEBUGALLOC
#undef new
#endif
  SShaderTechnique *hs = new SShaderTechnique();
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
  hs->m_eCull = (ECull)-1;

  enum {eCull=1, eDeclareCGScript, eShadeLayer, eLight, eFirstLight, eArray, eMatrix, eConditions, ePass, eShadow, eNoMerge, eFur, eSimulatedFur, eMultiShadows, eMultiLights};
  static tokenDesc commands[] =
  {
    {eDeclareCGScript, "DeclareCGScript"},
    {eShadeLayer, "ShadeLayer"},
    {ePass, "Pass"},
    {eLight, "Light"},
    {eMultiLights, "MultiLights"},
    {eFirstLight, "FirstLight"},
    {eArray, "Array"},
    {eCull, "Cull"},
    {eShadow, "Shadow"},
    {eMatrix, "Matrix"},
    {eConditions, "Conditions"},
    {eNoMerge, "NoMerge"},
    {eFur, "Fur"},
    {eSimulatedFur, "SimulatedFur"},
    {eMultiShadows, "MultiShadows"},
    
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
      case eDeclareCGScript:
        CCGVProgram_GL::mfAddNewScript(name, params);
        break;

      case eCull:
        if (!data || !data[0])
        {
          iLog->Log( "Warning: missing Cull argument in Shader '%s'\n", ef->m_Name.c_str());
          hs->m_eCull = eCULL_Front;
          break;
        }
        ef->m_Flags |= EF_HASCULL;
        if (!stricmp(data, "None") || !stricmp(data, "TwoSided") || !stricmp(data, "Disable"))
          hs->m_eCull = eCULL_None;
        else
        if (!strnicmp(data, "Back", 4))
          hs->m_eCull = eCULL_Back;
        else
        if (!strnicmp(data, "Front", 5))
          hs->m_eCull = eCULL_Front;
        else
          iLog->Log( "Warning: invalid Cull parm '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eShadeLayer:
      case ePass:
        mfCompileHWShadeLayer(ef, params, hs->m_Passes);
        ef->m_Flags3 |= EF3_HASAMBPASSES;
        break;

      case eConditions:
        mfCompileHWConditions(ef, params, hs, Id);
        break;

      case eFirstLight:
      case eLight:
        {
          if (!hs->m_Passes.Num())
            hs->m_Flags |= FHF_FIRSTLIGHT;
          EShaderPassType eType = eSHP_DiffuseLight;
          if (name && !strnicmp(name, "Spec", 4))
            eType = eSHP_SpecularLight;
          mfCompileLayers(ef, params, hs->m_Passes, eType);
        }
        break;

      case eMultiShadows:
        {
          EShaderPassType eType = eSHP_MultiShadows;
          int nNum = hs->m_Passes.Num();
          hs->m_Passes.AddIndex(1);
          SShaderPassHW *pass = &hs->m_Passes[nNum];
          memset(pass, 0, sizeof(*pass));
          pass->m_ePassType = eType;
        }
        break;

      case eMultiLights:
        {
          EShaderPassType eType = eSHP_MultiLights;
          mfCompileLayers(ef, params, hs->m_Passes, eType);
        }
        break;

      case eShadow:
        {
          EShaderPassType eType = eSHP_Shadow;
          mfCompileLayers(ef, params, hs->m_Passes, eType);
        }
        break;

      case eFur:
        {
          EShaderPassType eType = eSHP_Fur;
          mfCompileLayers(ef, params, hs->m_Passes, eType);
        }
        break;

      case eSimulatedFur:
        {
          EShaderPassType eType = eSHP_SimulatedFur;
          mfCompileLayers(ef, params, hs->m_Passes, eType);
        }
        break;

      case eArray:
        mfCompileArrayPointer(hs->m_Pointers, params, ef);
        break;

      case eMatrix:
        mfCompileMatrixOp(hs->m_MatrixOps, params, name, ef);
        break;

      case eNoMerge:
        hs->m_Flags |= FHF_NOMERGE;
        break;
    }
  }
  int i;
  if (!(hs->m_Flags & FHF_NOMERGE))
  {
    for (i=0; i<hs->m_Passes.Num(); i++)
    {
      SShaderPassHW *pass = &hs->m_Passes[i];
      if (mfUpdateMergeStatus(hs, &pass->m_VPParamsObj))
        break;
      if (mfUpdateMergeStatus(hs, &pass->m_VPParamsNoObj))
        break;
      if (pass->m_VProgram)
      {
        CCGVProgram_GL *vp = (CCGVProgram_GL *)pass->m_VProgram;
        if (mfUpdateMergeStatus(hs, vp->mfGetParams(0)))
          break;
        if (mfUpdateMergeStatus(hs, vp->mfGetParams(1)))
          break;
      }
      if (pass->m_FShader)
      {
        CCGPShader_GL *vp = (CCGPShader_GL *)pass->m_FShader;
        if (mfUpdateMergeStatus(hs, vp->mfGetParams(0)))
          break;
        if (mfUpdateMergeStatus(hs, vp->mfGetParams(1)))
          break;
      }
      if (hs->m_Flags & FHF_NOMERGE)
        break;
    }
  }

  return hs;
}

//===================================================================

SGenTC *SGenTC_NormalMap::mfCopy()
{
  SGenTC_NormalMap *gc = new SGenTC_NormalMap;
  gc->m_Mask = m_Mask;
  return gc;
}

bool SGenTC_NormalMap::mfSet(bool bEnable)
{
  if (bEnable)
  {
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_NV);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_NV);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_NV);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
  }
  else
  {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
  }
  return true;
}


void SGenTC_NormalMap::mfCompile(char *params, SShader *ef)
{
}

SGenTC *SGenTC_ReflectionMap::mfCopy()
{
  SGenTC_ReflectionMap *gc = new SGenTC_ReflectionMap;
  gc->m_Mask = m_Mask;
  return gc;
}

bool SGenTC_ReflectionMap::mfSet(bool bEnable)
{
  if (bEnable)
  {
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

  }
  else
  {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
  }
  return true;
}

void SGenTC_ReflectionMap::mfCompile(char *params, SShader *ef)
{
}

SGenTC *SGenTC_ObjectLinear::mfCopy()
{
  SGenTC_ObjectLinear *gc = new SGenTC_ObjectLinear;
  gc->m_Mask = m_Mask;
  for (int i=0; i<m_Params.Num(); i++)
  {
    int n = gc->m_Params.Num();
    gc->m_Params.AddIndex(1);
    gc->m_Params[n] = m_Params[i];
  }
  gc->m_Params.Shrink();
  return gc;
}

bool SGenTC_ObjectLinear::mfSet(bool bEnable)
{
  if (bEnable)
  {
    if (gRenDev->m_RP.m_pRE)
      gRenDev->m_RP.m_pRE->m_nCountCustomData = 0;
		//int nninc[4] = {'X','T','R','Q'};		
		//int nndec[4] = {'Z','G','F','E'};		
		//static float ff[4];
    for (int i=0; i<4; i++)
    {
      if (m_Mask & (1<<i))
      {
        if (m_Params.Num()>i && m_Params[i].mfIsValid())
        {
          float *Vals = m_Params[i].mfGet();					
					
          glTexGenf(GL_S+i, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
          glTexGenfv(GL_S+i, GL_OBJECT_PLANE, Vals);
          glEnable(GL_TEXTURE_GEN_S+i);
        }
      }
    }
  }
  else
  {
    for (int i=0; i<4; i++)
    {
      if (m_Mask & (1<<i))
        glDisable(GL_TEXTURE_GEN_S+i);
    }
  }
  return true;
}

void SGenTC_ObjectLinear::mfCompile(char *scr, SShader *ef)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  int i, j;

  if (!scr)
    return;
  
  enum {eMask = 1, ePlaneS, ePlaneT, ePlaneR, ePlaneQ, eComponents};
  static tokenDesc commands[] =
  {
    {eMask, "Mask"},
    {ePlaneS, "PlaneS"},
    {ePlaneT, "PlaneT"},
    {ePlaneR, "PlaneR"},
    {ePlaneQ, "PlaneQ"},
    {eComponents, "Components"},
    
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
      case eMask:
        m_Mask = shGetInt(data);
        break;

      case eComponents:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;

      case ePlaneS:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;

      case ePlaneT:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;

      case ePlaneR:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;

      case ePlaneQ:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;
    }
  }
  for (i=0; i<m_Params.Num(); i++)
  {
    for (j=0; j<4; j++)
    {
      if (m_Params[i].m_Comps[j] && m_Params[i].m_Comps[j]->m_bDependsOnObject)
      {
        m_bDependsOnObject = true;
        break;
      }
    }
    if (j != 4)
      break;
  }
}

SGenTC *SGenTC_EyeLinear::mfCopy()
{
  SGenTC_EyeLinear *gc = new SGenTC_EyeLinear;
  gc->m_Mask = m_Mask;
  for (int i=0; i<m_Params.Num(); i++)
  {
    int n = gc->m_Params.Num();
    gc->m_Params.AddIndex(1);
    gc->m_Params[n] = m_Params[i];
  }
  gc->m_Params.Shrink();
  return gc;
}

bool SGenTC_EyeLinear::mfSet(bool bEnable)
{
  if (bEnable)
  {
    if (gRenDev->m_RP.m_pRE)
      gRenDev->m_RP.m_pRE->m_nCountCustomData = 0;
    for (int i=0; i<4; i++)
    {
      if (m_Mask & (1<<i))
      {
        if (m_Params.Num()>i && m_Params[i].mfIsValid())
        {
          float *Vals = m_Params[i].mfGet();
          glTexGenf(GL_S+i, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
          glTexGenfv(GL_S+i, GL_EYE_PLANE, Vals);
          glEnable(GL_TEXTURE_GEN_S+i);
        }
      }
    }
  }
  else
  {
    for (int i=0; i<4; i++)
    {
      if (m_Mask & (1<<i))
        glDisable(GL_TEXTURE_GEN_S+i);
    }
  }
  return true;
}

void SGenTC_EyeLinear::mfCompile(char *scr, SShader *ef)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  if (!scr)
    return;
  
  enum {eMask = 1, ePlaneS, ePlaneT, ePlaneR, ePlaneQ};
  static tokenDesc commands[] =
  {
    {eMask, "Mask"},
    {ePlaneS, "PlaneS"},
    {ePlaneT, "PlaneT"},
    {ePlaneR, "PlaneR"},
    {ePlaneQ, "PlaneQ"},
    
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
      case eMask:
        m_Mask = shGetInt(data);
        break;
        
      case ePlaneS:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;
        
      case ePlaneT:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;
        
      case ePlaneR:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;
        
      case ePlaneQ:
        gRenDev->m_cEF.mfCompileParam(data, ef, &m_Params);
        break;
      }
  }
}

SGenTC *SGenTC_SphereMap::mfCopy()
{
  SGenTC_SphereMap *gc = new SGenTC_SphereMap;
  gc->m_Mask = m_Mask;
  return gc;
}

bool SGenTC_SphereMap::mfSet(bool bEnable)
{
  if (bEnable)
  {
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
  }
  else
  {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
  }
  return true;
}

void SGenTC_SphereMap::mfCompile(char *params, SShader *ef)
{
}


SGenTC *SGenTC_EmbossMap::mfCopy()
{
  SGenTC_EmbossMap *gc = new SGenTC_EmbossMap;
  gc->m_Mask = m_Mask;
  return gc;
}


bool SGenTC_EmbossMap::mfSet(bool bEnable)
{
  if (bEnable)
  {
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EMBOSS_MAP_NV);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EMBOSS_MAP_NV);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EMBOSS_MAP_NV);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
  }
  else
  {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
  }
  return true;
}

void SGenTC_EmbossMap::mfCompile(char *params, SShader *ef)
{
}

bool CShader::mfCompileTexGen(char *name, char *params, SShader *ef, SShaderTexUnit *ml)
{
  bool bRes = true;
  if (!name)
  {
    name = params;
    params = NULL;
  }
  if (!stricmp(name, "HW_NormalMap"))
  {
    ml->m_GTC = (SGenTC *)new SGenTC_NormalMap;
    ml->m_GTC->mfCompile(params, ef);
  }
  else
  if (!strnicmp(name, "HW_Reflection", 13))
  {
    ml->m_GTC = (SGenTC *)new SGenTC_ReflectionMap;
    ml->m_GTC->mfCompile(params, ef);
  }
  else
  if (!stricmp(name, "HW_ObjectLinear"))
  {
    ml->m_GTC = (SGenTC *)new SGenTC_ObjectLinear;
    ml->m_GTC->mfCompile(params, ef);
  }
  else
  if (!stricmp(name, "HW_EyeLinear"))
  {
    ml->m_GTC = (SGenTC *)new SGenTC_EyeLinear;
    ml->m_GTC->mfCompile(params, ef);
  }
  else
  if (!stricmp(name, "HW_SphereMap"))
  {
    ml->m_GTC = (SGenTC *)new SGenTC_SphereMap;
    ml->m_GTC->mfCompile(params, ef);
  }
  else
  if (!stricmp(name, "HW_EmbossMap"))
  {
    ml->m_GTC = (SGenTC *)new SGenTC_EmbossMap;
    ml->m_GTC->mfCompile(params, ef);
  }
  else
    bRes = false;

  return bRes;
}

//====================================================================
// Matrix operations


void CShader::mfCompileMatrixOp(TArray<SMatrixTransform>* List, char *scr, char *nmMat, SShader *ef)
{
  int Matr;
  if (!nmMat || !nmMat[0])
  {
    iLog->Log("Warning: Missing matrix name in Shader '%s'\n", ef->m_Name.c_str());
    return;
  }
  if (!stricmp(nmMat, "GL_TEXTURE"))
    Matr = GL_TEXTURE;
  else
  if (!stricmp(nmMat, "GL_MATRIX0_NV"))
    Matr = GL_MATRIX0_NV;
  else
  if (!stricmp(nmMat, "GL_MATRIX1_NV"))
    Matr = GL_MATRIX1_NV;
  else
  if (!stricmp(nmMat, "GL_MATRIX2_NV"))
    Matr = GL_MATRIX2_NV;
  else
  if (!stricmp(nmMat, "GL_MATRIX3_NV"))
    Matr = GL_MATRIX3_NV;
  else
  if (!stricmp(nmMat, "GL_MATRIX4_NV"))
    Matr = GL_MATRIX4_NV;
  else
  if (!stricmp(nmMat, "GL_MATRIX5_NV"))
    Matr = GL_MATRIX5_NV;
  else
  if (!stricmp(nmMat, "GL_MATRIX6_NV"))
    Matr = GL_MATRIX6_NV;
  else
  if (!stricmp(nmMat, "GL_MATRIX7_NV"))
    Matr = GL_MATRIX7_NV;
  else
  {
    iLog->Log("Warning: Unknown matrix name '%s' in Shader '%s'\n", nmMat, ef->m_Name.c_str());
    return;
  }

  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eIdentity = 1, eProjected, eCoords, eTranslate, eRotateX, eRotateY, eRotateZ, eRotate_XY, eRotate_XZ, eRotate_YZ, eScale, eTexStage, eMatrix, eLightCMProject};
  static tokenDesc commands[] =
  {
    {eIdentity, "Identity"},
    {eTranslate, "Translate"},
    {eRotateX, "RotateX"},
    {eRotateY, "RotateY"},
    {eRotateZ, "RotateZ"},
    {eRotate_XY, "Rotate_XY"},
    {eRotate_XZ, "Rotate_XZ"},
    {eRotate_YZ, "Rotate_YZ"},
    {eTexStage, "TexStage"},
    {eScale, "Scale"},
    {eMatrix, "Matrix"},
    {eLightCMProject, "LightCMProject"},
    {eProjected, "Projected"},
    {eCoords, "Coords"},
    
    {0,0}
  };

  int Stage = 0;
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
      case eTexStage:
        Stage = shGetInt(data);
        break;

      case eProjected:
        /*{
          SMatrixTransform *mt = &List->Get(nFirst);
          if (mt)
            mt->m_Matrix |= 0x8000;
        }*/
        break;

      case eCoords:
        /*{
          SMatrixTransform *mt = &List->Get(nFirst);
          if (mt && data)
            mt->m_Matrix |= atol(data)<<16;
        }*/
        break;

      case eIdentity:
        {
          SMatrixTransform_Identity mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eMatrix:
        {
          SMatrixTransform_Matrix mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          int n = List->Num();
          TArray<SParam> Params;
          mfCompileParam(data, ef, &Params);
          for (int i=0; i<Params.Num(); i++)
          {
            mt.m_Params[i] = Params[i];
          }
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eLightCMProject:
        {
          SMatrixTransform_LightCMProject mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eTranslate:
        {
          SMatrixTransform_Translate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eScale:
        {
          SMatrixTransform_Scale mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;

      case eRotateX:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 1;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotateY:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 2;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotateZ:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 4;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotate_XY:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 3;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotate_XZ:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 5;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
      case eRotate_YZ:
        {
          SMatrixTransform_Rotate mt;
          mt.m_Matrix = Matr;
          mt.m_Stage = Stage;
          mt.m_Offs = 6;
          TArray<SParam> Params;
          mfCompileParam(params, ef, &Params);
          if (Params.Num())
            mt.m_Params[0] = Params[0];
          int n = List->Num();
          List->AddIndex(1);
          memcpy(&List->Get(n), &mt, sizeof(SMatrixTransform));
        }
        break;
    }
  }
}

void SMatrixTransform_LightCMProject::mfSet(bool bSet)
{
  if (bSet)
  {
    if (gRenDev->m_RP.m_nCurLight < gRenDev->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num())
    {
      CDLight *dl = gRenDev->m_RP.m_pCurLight;
      if (dl && dl->m_pLightImage!=0)
      {
        glLoadIdentity();

        //scale the cubemap to adjust the default 45 degree 1/2 angle fustrum to 
        //the desired angle (0 to 90 degrees)
        float scaleFactor = cry_tanf((90.0f-dl->m_fLightFrustumAngle)*M_PI/180.0f);
        glScalef(1, scaleFactor, scaleFactor);

        //we need to rotate the cubemap to account for the spotlights orientation
        //convert the orienations ortho normal basis (ONB) into XYZ space, and then
        //into the base direction space (using ONB prevents having to calculate angles)
        Matrix44 m = dl->m_Orientation.matrixBasisToXYZ();
        m.Transpose();
        glMultMatrixf(&m(0,0));

        //translate the vertex relative to the light position
        Vec3d vObjPos = gRenDev->m_RP.m_pCurObject->GetTranslation();
        glTranslatef(-(dl->m_Origin.x-vObjPos.x), -(dl->m_Origin.y-vObjPos.y), -(dl->m_Origin.z-vObjPos.z));
      }
    }
  }
}
void SMatrixTransform_LightCMProject::mfSet(Matrix44& matr)
{
  if (gRenDev->m_RP.m_nCurLight < gRenDev->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num())
  {
    CDLight *dl = gRenDev->m_RP.m_pCurLight;
    if (dl && dl->m_pLightImage!=0)
    {
      matr.SetIdentity();

      //scale the cubemap to adjust the default 45 degree 1/2 angle fustrum to 
      //the desired angle (0 to 90 degrees)
      float scaleFactor = cry_tanf((90.0f-dl->m_fLightFrustumAngle)*M_PI/180.0f);
      mathScale(matr.GetData(), Vec3d(1, scaleFactor, scaleFactor), g_CpuFlags);

      //we need to rotate the cubemap to account for the spotlights orientation
      //convert the orienations ortho normal basis (ONB) into XYZ space, and then
      //into the base direction space (using ONB prevents having to calculate angles)
      Matrix44 m = dl->m_Orientation.matrixBasisToXYZ();
      mathMatrixTranspose(m.GetData(), m.GetData(), g_CpuFlags);
      matr *= m;

      //translate the vertex relative to the light position
      Vec3d vObjPos = gRenDev->m_RP.m_pCurObject->GetTranslation();
      glTranslatef(-(dl->m_Origin.x-vObjPos.x), -(dl->m_Origin.y-vObjPos.y), -(dl->m_Origin.z-vObjPos.z));
    }
  }
}


void SMatrixTransform_Identity::mfSet(bool bSet)
{
  if (bSet)
    glLoadIdentity();
}
void SMatrixTransform_Identity::mfSet(Matrix44& matr)
{
  matr.SetIdentity();
}

void SMatrixTransform_Translate::mfSet(bool bSet)
{
  float *p;
  if (bSet)
  {
    p = m_Params[0].mfGet();
    glTranslatef(p[0], p[1], p[2]);
  }
}
void SMatrixTransform_Translate::mfSet(Matrix44& matr)
{
  float *p;
  p = m_Params[0].mfGet();
  SGLFuncs::glTranslate(&matr(0,0), p[0], p[1], p[2]);
}

void SMatrixTransform_Scale::mfSet(bool bSet)
{
  float *p;
  if (bSet)
  {
    p = m_Params[0].mfGet();
    glScalef(p[0], p[1], p[2]);
  }
}
void SMatrixTransform_Scale::mfSet(Matrix44& matr)
{
  float *p;
  p = m_Params[0].mfGet();
  mathScale(&matr(0,0), Vec3d(p[1], p[2], p[2]), g_CpuFlags);
}

void SMatrixTransform_Matrix::mfSet(bool bSet)
{
  float *p;
  float matr[4][4];
  if (bSet)
  {
    if (gRenDev->m_RP.m_pRE)
      gRenDev->m_RP.m_pRE->m_nCountCustomData = 0;
    for (int i=0; i<4; i++)
    {
      p = m_Params[i].mfGet();
      memcpy(&matr[i][0], p, 4*4);
    }
    glLoadMatrixf(&matr[0][0]);
  }
}
void SMatrixTransform_Matrix::mfSet(Matrix44& matr)
{
  float *p;
  for (int i=0; i<4; i++)
  {
    p = m_Params[i].mfGet();
    memcpy(&matr(i,0), p, 4*4);
  }
}

void SMatrixTransform_Rotate::mfSet(bool bSet)
{
  float *p;
  if (bSet)
  {
    p = m_Params[0].mfGet();
    switch (m_Offs)
    {
      case 1:
        glRotatef(p[0], 1, 0, 0);
        break;
      case 2:
        glRotatef(p[0], 0, 1, 0);
        break;
      case 4:
        glRotatef(p[0], 0, 0, 1);
        break;
      case 3:
        glRotatef(p[0], 1, 1, 0);
        break;
      case 5:
        glRotatef(p[0], 1, 0, 1);
        break;
      case 6:
        glRotatef(p[0], 1, 1, 0);
        break;
      case 7:
        glRotatef(p[0], 1, 1, 1);
        break;
    }
  }
}
void SMatrixTransform_Rotate::mfSet(Matrix44& matr)
{
  float *p;
  p = m_Params[0].mfGet();
  switch (m_Offs)
  {
    case 1:
      SGLFuncs::glRotate(&matr(0,0), p[0], 1, 0, 0);
      break;
    case 2:
      SGLFuncs::glRotate(&matr(0,0), p[0], 0, 1, 0);
      break;
    case 4:
      SGLFuncs::glRotate(&matr(0,0), p[0], 0, 0, 1);
      break;
    case 3:
      SGLFuncs::glRotate(&matr(0,0), p[0], 1, 1, 0);
      break;
    case 5:
      SGLFuncs::glRotate(&matr(0,0), p[0], 1, 0, 1);
      break;
    case 6:
      SGLFuncs::glRotate(&matr(0,0), p[0], 1, 1, 0);
      break;
    case 7:
      SGLFuncs::glRotate(&matr(0,0), p[0], 1, 1, 1);
      break;
  }
}

//====================================================================
// Array pointers for OpenGL
//================================================================

void SArrayPointer_Vertex::mfSet(int Id)
{
  if (gRenDev->m_RP.m_pRE)
  {
    int Stride;
    void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
	  if(!p)
	  {
		  iLog->Log("Error: SArrayPointer_Vertex::mfSet: EF_GetPointer returns zero");
		  return;
	  }

    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, gRenDev->m_RP.m_nCurBufferID);
      if (gRenDev->m_RP.m_nCurBufferID != (INT_PTR)m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf)	//AMD Port
      {
        m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
        m_pLastPointer = (void *)gRenDev->m_RP.m_nCurBufferID;
        glVertexPointer(NumComponents, Type, Stride, BUFFER_OFFSET(gRenDev->m_RP.m_nCurBufferOffset));
      }
    }
    else
    if (p != m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf)
    {
      m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
      m_pLastPointer = p;
      glVertexPointer(NumComponents, Type, Stride, p);
    }
  }
  m_CurEnabled |= PFE_POINTER_VERT;
  if (Id)
    m_CurEnabledPass |= PFE_POINTER_VERT;
}

//=========================================================================================

void SArrayPointer_Normal::mfSet(int Id)
{
  int Stride;
  if (gRenDev->m_RP.m_pRE)
  {
    void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, gRenDev->m_RP.m_nCurBufferID);
      if (p && (gRenDev->m_RP.m_nCurBufferID != (INT_PTR)m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf))	//AMD Port
      {
        m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
        m_pLastPointer = (void *)gRenDev->m_RP.m_nCurBufferID;
        glNormalPointer(Type, Stride, BUFFER_OFFSET(gRenDev->m_RP.m_nCurBufferOffset));
      }
    }
    else
    if (p && (p != m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf))
    {
      m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
      m_pLastPointer = p;
      glNormalPointer(Type, Stride, p);
    }
  }
  m_CurEnabled |= PFE_POINTER_NORMAL;
  if (Id)
    m_CurEnabledPass |= PFE_POINTER_NORMAL;
}

//=========================================================================================

void SArrayPointer_Texture::mfSet(int Id)
{
  int Stride;
  if (gRenDev->m_RP.m_pRE)
  {
    void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, (ESrcPointer)(ePT+Stage), Stage << FGP_STAGE_SHIFT);
    if (ePT == eSrcPointer_TexLM)
      gRenDev->m_RP.m_nLMStage = Stage;
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, gRenDev->m_RP.m_nCurBufferID);
      if (gRenDev->m_RP.m_nCurBufferID != (INT_PTR)m_pLastPointer[Stage] || m_nFrameCreateBuf[Stage] != gcpOGL->m_nFrameCreateBuf)	//AMD Port
      {
        m_nFrameCreateBuf[Stage] = gcpOGL->m_nFrameCreateBuf;
        m_pLastPointer[Stage] = (void *)gRenDev->m_RP.m_nCurBufferID;
        gcpOGL->EF_SelectTMU(Stage);
        glTexCoordPointer(NumComponents, Type, Stride, BUFFER_OFFSET(gRenDev->m_RP.m_nCurBufferOffset));
      }
    }
    else
    if (p != m_pLastPointer[Stage] || m_nFrameCreateBuf[Stage] != gcpOGL->m_nFrameCreateBuf)
    {
      m_nFrameCreateBuf[Stage] = gcpOGL->m_nFrameCreateBuf;
      m_pLastPointer[Stage] = p;
      gcpOGL->EF_SelectTMU(Stage);
      glTexCoordPointer(NumComponents, Type, Stride, p);
    }
  }
  if (Id)
    m_CurEnabledPass |= PFE_POINTER_TEX0<<Stage;
  m_CurEnabled |= PFE_POINTER_TEX0<<Stage;
}

//=========================================================================================

void SArrayPointer_Color::mfSet(int Id)
{
  //if(!(gRenDev->m_RP.m_FlagsPerFlush & (RBSI_ALPHAGEN | RBSI_RGBGEN)))
  {
    if (gRenDev->m_RP.m_pRE)
    {
      int Stride;
      void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
      if (SUPPORTS_GL_ARB_vertex_buffer_object)
      {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, gRenDev->m_RP.m_nCurBufferID);
        if (gRenDev->m_RP.m_nCurBufferID != (INT_PTR)m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf)	//AMD Port
        {
          m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
          m_pLastPointer = (void *)gRenDev->m_RP.m_nCurBufferID;
          glColorPointer(NumComponents, Type, Stride, BUFFER_OFFSET(gRenDev->m_RP.m_nCurBufferOffset));
        }
      }
      else
      if (p != m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf)
      {
        m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
        m_pLastPointer = p;
        glColorPointer(NumComponents, Type, Stride, p);
      }
    }
    m_CurEnabled |= PFE_POINTER_COLOR;
    if (Id)
      m_CurEnabledPass |= PFE_POINTER_COLOR;
  }
}

void SArrayPointer_SecColor::mfSet(int Id)
{
  if (gRenDev->m_RP.m_pRE)
  {
    eDst = eDstPointer_SecColor;
    int Stride;
    void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, gRenDev->m_RP.m_nCurBufferID);
      if (gRenDev->m_RP.m_nCurBufferID != (INT_PTR)m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf)	//AMD Port
      {
        m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
        m_pLastPointer = (void *)gRenDev->m_RP.m_nCurBufferID;
        glSecondaryColorPointerEXT(NumComponents, Type, Stride, BUFFER_OFFSET(gRenDev->m_RP.m_nCurBufferOffset));
      }
    }
    else
    if (p != m_pLastPointer || m_nFrameCreateBuf != gcpOGL->m_nFrameCreateBuf)
    {
      m_nFrameCreateBuf = gcpOGL->m_nFrameCreateBuf;
      m_pLastPointer = p;
      glSecondaryColorPointerEXT(NumComponents, Type, Stride, p);
    }
  }
  if (Id)
    m_CurEnabledPass |= PFE_POINTER_SECCOLOR;
  m_CurEnabled |= PFE_POINTER_SECCOLOR;
}

//==================================================================================

float SParamComp_Fog::mfGet()
{
  if (m_Type == 0)
    return gRenDev->m_FS.m_FogDensity;
  else
  if (m_Type == 1)
    return gRenDev->m_FS.m_FogStart;
  else
  if (m_Type == 2)
    return gRenDev->m_FS.m_FogEnd;
  else
  if (m_Type == 3)
    return 1.0f / (gRenDev->m_FS.m_FogEnd - gRenDev->m_FS.m_FogStart);
  return 0;
}

