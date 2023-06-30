/*=============================================================================
  D3DShaders.cpp : Direct3D specific effectors/shaders functions implementation.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"

#include "D3DCGPShader.h"
#include "D3DCGVProgram.h"


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
			Warning( 0,0,"Couldn't find console variable %s in shader '%s'",var,ef->m_Name.c_str() );
      //iLog->Log("Couldn't find console variable in shader '%s'\n", ef->m_Name.c_str());
      continue;
    }
    float v = shGetFloat(val);
    CVarCond vc;
    vc.m_Var = vr;
    vc.m_Val = v;
    Vars.AddElem(vc);
  }
}

#define GL_CONSTANT_COLOR0_NV               0x852A
#define GL_CONSTANT_COLOR1_NV               0x852B

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

  enum {eLayer = 1, eLightType, eCGPShader, eLMVertexLight, eLMOnlyMaterialAmbient, eCGVPParam, eCGVPIParam, eCGPSParam, eCGPSIParam, eCGPSParmRect, eDeformVertexes, eArray, eMatrix, eNoLight, eLMNoSpecular, eLMNoAddSpecular, eCGVProgram, eLMIgnoreLights, eLMIgnoreProjLights, eLMNoAmbient, eLMBump, ePolyOffset, eLMNoAlpha, eLMDivideAmb4, eLMDivideAmb2, eLMDivideDif4, eLMDivideDif2, eColorMaterial, eRendState, eSecondPassRendState, eNoBump, e1Samples, e2Samples, e3Samples, e4Samples, eHasAmbient, eHasDOT3LM, eAffectMask, eAmbMaxLights, eOcclusionMap, eAllowSpecAntialias};
  static tokenDesc commands[] =
  {
    {eLayer, "Layer"},
    {eLightType, "LightType"},
    {eDeformVertexes, "DeformVertexes"},
    {eNoLight, "NoLight"},
    {eCGVProgram, "CGVProgram"},
    {eCGVPParam, "CGVPParam"},
    {eCGVPIParam, "CGVPIParam"},
    {eCGPSParam, "CGPSParam"},
    {eCGPSParmRect, "CGPSParmRect"},
    {eCGPSIParam, "CGPSIParam"},
    {eCGPShader, "CGPShader"},
    {eArray, "Array"},
    {eMatrix, "Matrix"},
    {eSecondPassRendState, "SecondPassRendState"},
    {eRendState, "RendState"},
    {eSecondPassRendState, "SecondPassRendState"},

    {ePolyOffset, "PolyOffset"},
    {eAffectMask, "AffectMask"},

    {eLMIgnoreLights, "LMIgnoreLights"},
    {eLMIgnoreProjLights, "LMIgnoreProjLights"},
    {eLMNoAmbient, "LMNoAmbient"},
    {eLMNoAlpha, "LMNoAlpha"},
    {eLMNoSpecular, "LMNoSpecular"},
    {eLMNoAddSpecular, "LMNoAddSpecular"},
    {eLMOnlyMaterialAmbient, "LMOnlyMaterialAmbient"},

    {e1Samples, "1Samples"},
    {e2Samples, "2Samples"},
    {e3Samples, "3Samples"},
    {e4Samples, "4Samples"},
    {eHasAmbient, "HasAmbient"},
    {eHasDOT3LM, "HasDOT3LM"},


    {eOcclusionMap, "OcclusionMap"},
    {eAmbMaxLights, "AmbMaxLights"},

    {eColorMaterial, "ColorMaterial"},
    {eLMBump, "LMBump"},
    {eLMVertexLight, "LMVertexLight"},
    {eLMDivideAmb4, "LMDivideAmb4"},
    {eLMDivideAmb2, "LMDivideAmb2"},
    {eLMDivideDif4, "LMDivideDif4"},
    {eLMDivideDif2, "LMDivideDif2"},

    {eAllowSpecAntialias, "AllowSpecAntialias"},

    {0,0}
  };

  bool bVertLight = false;
  uint64 nAffectMask = 0;
  int nAmbMaxLights = gRenDev->m_bDeviceSupports_PS2X ? NUM_PPLIGHTS_PERPASS_PS2X : NUM_PPLIGHTS_PERPASS_PS30;
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

      case eAllowSpecAntialias:
        sm->m_Flags |= SHPF_ALLOW_SPECANTIALIAS;
        break;

      case eNoLight:
        sm->m_LMFlags |= LMF_DISABLE;
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
      case eAffectMask:
        nAffectMask = shGetHex64(data);
        break;
      case eAmbMaxLights:
        nAmbMaxLights = shGetInt(data);
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
      case eHasAmbient:
        sm->m_LMFlags |= LMF_HASAMBIENT;
        break;
      case eHasDOT3LM:
        sm->m_LMFlags |= LMF_HASDOT3LM;
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
            uint64 nMask = 0;
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

      case eLayer:
        if (name)
          nl = atoi(name);
        else
          nl = 0;
        mfCompileLayer(ef, nl, params, sm);
        break;

      case eLightType:
        if (!strnicmp(data, "Direct", 6))
          sm->m_LightFlags |= DLF_DIRECTIONAL;
        else
        if (!strnicmp(data, "Point", 5))
          sm->m_LightFlags |= DLF_POINT;
        else
        if (!strnicmp(data, "Project", 7))
          sm->m_LightFlags |= DLF_PROJECT;
        else
        if (!strnicmp(data, "OnlySpec", 8))
          sm->m_LightFlags |= DLF_LM;
        else
					Warning( 0,0,"Unknown light type for light pass in effector '%s'\n", ef->m_Name.c_str());
          //iLog->Log("Unknown light type for light pass in effector '%s'\n", ef->m_Name.c_str());
        break;

      case eOcclusionMap:
        sm->m_LMFlags |= LMF_USEOCCLUSIONMAP;
        break;

      case eNoBump:
        sm->m_LMFlags |= LMF_NOBUMP;
        break;

      case eDeformVertexes:
        {
          if (!ef->m_Deforms)
            ef->m_Deforms = new TArray<SDeform>;
          int i = ef->m_Deforms->Num();
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
          uint64 nMask = 0;
          if (nAffectMask)
            nMask = mfScriptPreprocessorMask(ef, pCurCommand-m_pCurScript);
          ps = CPShader::mfForName(params, nMask & nAffectMask);
          if (ps)
          {
            if (nAffectMask)
              ps->m_Macros = m_Macros;
            sm->m_FShader = ps;
            sm->m_LMFlags |= LMF_HASPSHADER;
          }
        }
        break;

      case eCGVPParam:
        mfCompileCGParam(params, ef, &sm->m_VPParamsNoObj);
        break;

      case eCGPSParam:
      case eCGPSParmRect:
        if (!sm->m_CGFSParamsNoObj)
          sm->m_CGFSParamsNoObj = new TArray<SCGParam4f>;
        mfCompileCGParam(params, ef, sm->m_CGFSParamsNoObj);
        break;

      case eCGPSIParam:
        if (!sm->m_CGFSParamsNoObj)
          sm->m_CGFSParamsNoObj = new TArray<SCGParam4f>;
        {
          int n = sm->m_CGFSParamsNoObj->Num();
          mfCompileCGParam(params, ef, sm->m_CGFSParamsNoObj);
          for (int i=0; i<n; i++)
          {
            sm->m_CGFSParamsNoObj->Get(i).m_Flags |= PF_INTEGER;
          }
        }
        break;

      case eCGVPIParam:
        {
          int n = sm->m_VPParamsNoObj.Num();
          mfCompileCGParam(params, ef, &sm->m_VPParamsNoObj);
          for (int i=0; i<n; i++)
          {
            sm->m_VPParamsNoObj[i].m_Flags |= PF_INTEGER;
          }
        }
        break;

      case eArray:
        mfCompileArrayPointer(sm->m_Pointers, params, ef);
        break;

      case eMatrix:
        if (!sm->m_MatrixOps)
          sm->m_MatrixOps = new TArray<SMatrixTransform>;
        mfCompileMatrixOp(sm->m_MatrixOps, params, name, ef);
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
  sm->m_nAmbMaxLights = nAmbMaxLights;
  if (sm->m_VProgram && !bVertLight)
    sm->m_LMFlags |= LMF_BUMPMATERIAL;

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
  
  enum {eSingleLight = 1, eRETexBind1, eRETexBind2, eRETexBind3, eRETexBind4, eRETexBind5, eRETexBind6, eRETexBind7, eMultipleLights, eNoLights, eOnlyDirectional, eHasProjectedLights, eInShadow, eHasEnvLCMap, eVars, eSpecular, eBended, eAlphaBlended, eNoBump, eHasLM, eHasDOT3LM, eHasVColors, eHasAlphaTest, eHasAlphaBlend, eHasResource, eHeatVision, eHotAmbient, eInFogVolume};
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
    {eHasResource, "HasResource"},
    {eHasVColors, "HasVColors"},
    {eHeatVision, "HeatVision"},
    {eHotAmbient, "HotAmbient"},
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

      case eHasDOT3LM:
        hc->m_Flags |= SHCF_HASDOT3LM;
        break;

      case eHasResource:
        hc->m_Flags |= SHCF_HASRESOURCE;
        break;

      case eSpecular:
        hc->m_Flags |= SHCF_SPECULAR;
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
    Warning( 0,0,"Hardware section not allowed for Shader '%s'\n", ef->m_Name.c_str());
    return NULL;
  }

  char* name;
  long cmd;
  char *params;
  char *data;

#ifdef DEBUGALLOC
#undef new
#endif
  SShaderTechnique *hs = new SShaderTechnique;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
  hs->m_eCull = (ECull)-1;

  enum {eCull=1, eDeclareCGScript, eShadeLayer, ePass, eLight, eFirstLight, eArray, eMatrix, eConditions, eShadow, eMultiLights, eNoMerge, eFur, eSimulatedFur, eMultiShadows};
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
    {eFur, "Fur"},
    {eSimulatedFur, "SimulatedFur"},
    {eMatrix, "Matrix"},
    {eConditions, "Conditions"},
    {eNoMerge, "NoMerge"},
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
        CCGVProgram_D3D::mfAddNewScript(name, params);
        break;

      case eCull:
        if (!data || !data[0])
        {
          Warning( 0,0, "missing Cull argument in Shader '%s'\n", ef->m_Name.c_str());
          ef->m_eCull = eCULL_Front;
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
          Warning( 0,0, "invalid Cull parm '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
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
        CCGVProgram_D3D *vp = (CCGVProgram_D3D *)pass->m_VProgram;
        if (mfUpdateMergeStatus(hs, vp->mfGetParams(0)))
          break;
        if (mfUpdateMergeStatus(hs, vp->mfGetParams(1)))
          break;
      }
      if (pass->m_FShader)
      {
        CCGPShader_D3D *vp = (CCGPShader_D3D *)pass->m_FShader;
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

//====================================================================

SGenTC *SGenTC_NormalMap::mfCopy()
{
  SGenTC_NormalMap *gc = new SGenTC_NormalMap;
  gc->m_Mask = m_Mask;
  return gc;
}

bool SGenTC_NormalMap::mfSet(bool bEnable)
{
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gcpRendD3D->m_TexMan->m_CurStage;
  if (bEnable)
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL | cs);
  else
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
  gcpRendD3D->m_RP.m_TexStages[cs].TCIndex = cs;

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
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gcpRendD3D->m_TexMan->m_CurStage;
  if (bEnable)
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | cs);
  else
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
  gcpRendD3D->m_RP.m_TexStages[cs].TCIndex = cs;
  return true;
}

void SGenTC_ReflectionMap::mfCompile(char *params, SShader *ef)
{
}

SGenTC *SGenTC_SphereMap::mfCopy()
{
  SGenTC_SphereMap *gc = new SGenTC_SphereMap;
  gc->m_Mask = m_Mask;
  return gc;
}

bool SGenTC_SphereMap::mfSet(bool bEnable)
{
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gcpRendD3D->m_TexMan->m_CurStage;
  if (bEnable)
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_SPHEREMAP | cs);
  else
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
  gcpRendD3D->m_RP.m_TexStages[cs].TCIndex = cs;
  return true;
}

void SGenTC_SphereMap::mfCompile(char *params, SShader *ef)
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
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gRenDev->m_TexMan->m_CurStage;
  if (bEnable)
  {
    if (gRenDev->m_RP.m_pRE)
      gRenDev->m_RP.m_pRE->m_nCountCustomData = 0;
    int n = 0;
    D3DXMATRIX mat, ma, *mi;
    D3DXMatrixIdentity(&mat);
    for (int i=0; i<4; i++)
    {
      if (m_Mask & (1<<i))
      {
        n++;
        if (m_Params.Num()>i && m_Params[i].mfIsValid())
        {
          float *Vals = m_Params[i].mfGet();					
          mat.m[0][i] = Vals[0];
          mat.m[1][i] = Vals[1];
          mat.m[2][i] = Vals[2];
          mat.m[3][i] = Vals[3];
        }
      }
    }
    mi = gcpRendD3D->EF_InverseMatrix();
    D3DXMatrixMultiply(&ma, mi, &mat);
    dv->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+cs), &ma );
    int nFlags = 0;
    if (gcpRendD3D->m_RP.m_TexStages[cs].Projected)
      nFlags = D3DTTFF_PROJECTED;
    if (n == 4)
    {
      if (nFlags)
        dv->SetTextureStageState( cs, D3DTSS_TEXTURETRANSFORMFLAGS, 4 | nFlags);
      else
        dv->SetTextureStageState( cs, D3DTSS_TEXTURETRANSFORMFLAGS, 3);
    }
    else
      dv->SetTextureStageState( cs, D3DTSS_TEXTURETRANSFORMFLAGS, n | nFlags);
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | cs);
  }
  else
  {
    D3DXMATRIX mat;
    D3DXMatrixIdentity(&mat);
    dv->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+cs), &mat );
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
    dv->SetTextureStageState( cs, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
  }
  gcpRendD3D->m_RP.m_TexStages[cs].TCIndex = cs;
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
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gRenDev->m_TexMan->m_CurStage;
  if (bEnable)
  {
    if (gRenDev->m_RP.m_pRE)
      gRenDev->m_RP.m_pRE->m_nCountCustomData = 0;
    int n = 0;
    D3DXMATRIX mat, *mi, mr;
    D3DXMatrixIdentity(&mat);
    for (int i=0; i<4; i++)
    {
      if (m_Mask & (1<<i))
      {
        n++;
        if (m_Params.Num()>i && m_Params[i].mfIsValid())
        {
          float *Vals = m_Params[i].mfGet();					
          mat.m[0][i] = Vals[0];
          mat.m[1][i] = Vals[1];
          mat.m[2][i] = Vals[2];
          mat.m[3][i] = Vals[3];
        }
      }
    }
    mi = gcpRendD3D->EF_InverseMatrix();
    D3DXMatrixMultiply(&mr, mi, &mat);
    dv->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+cs), &mat );
    dv->SetTextureStageState( cs, D3DTSS_TEXTURETRANSFORMFLAGS, 4 | D3DTTFF_PROJECTED );
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | cs);
  }
  else
  {
    D3DXMATRIX mat;
    D3DXMatrixIdentity(&mat);
    dv->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+cs), &mat );
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
  }
  gcpRendD3D->m_RP.m_TexStages[cs].TCIndex = cs;
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

SGenTC *SGenTC_EmbossMap::mfCopy()
{
  SGenTC_EmbossMap *gc = new SGenTC_EmbossMap;
  gc->m_Mask = m_Mask;
  return gc;
}


bool SGenTC_EmbossMap::mfSet(bool bEnable)
{
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
    Warning( 0,0,"Missing matrix name in Shader '%s'\n", ef->m_Name.c_str());
    return;
  }
  if (!stricmp(nmMat, "D3D_TEXTURE") || !stricmp(nmMat, "GL_TEXTURE"))
    Matr = D3DTS_TEXTURE0;
  else
  {
    Warning( 0,0,"Unknown matrix name '%s' in Shader '%s'\n", nmMat, ef->m_Name.c_str());
    return;
  }

  char* name;
  long cmd;
  char *params;
  char *data;

  int nFirst = List->Num();

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
    {eScale, "Scale"},
    {eTexStage, "TexStage"},
    {eProjected, "Projected"},
    {eCoords, "Coords"},
    {eMatrix, "Matrix"},
    {eLightCMProject, "LightCMProject"},
    
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
        if (Matr == D3DTS_TEXTURE0)
          Matr += Stage;
        break;

      case eProjected:
        {
          SMatrixTransform *mt = &List->Get(nFirst);
          if (mt)
            mt->m_Matrix |= 0x8000;
        }
        break;

      case eCoords:
        {
          SMatrixTransform *mt = &List->Get(nFirst);
          if (mt && data)
            mt->m_Matrix |= atol(data)<<16;
        }
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
  SMatrixTransform *mt = &List->Get(nFirst);
  if (mt && (mt->m_Matrix>>16) == 0)
    mt->m_Matrix |= 2<<16;
}

void SMatrixTransform_Identity::mfSet(bool bSet)
{
  if (bSet)
    D3DXMatrixIdentity(gcpRendD3D->m_CurOpMatrix);
}
void SMatrixTransform_Identity::mfSet(Matrix44& matr)
{
}

void SMatrixTransform_Translate::mfSet(bool bSet)
{
  float *p;
  if (bSet)
  {
    p = m_Params[0].mfGet();
    D3DXMATRIX mat;
    D3DXMatrixTranslation(&mat, p[0], p[1], p[2]);
    D3DXMatrixMultiply(gcpRendD3D->m_CurOpMatrix, gcpRendD3D->m_CurOpMatrix, &mat);
  }
}
void SMatrixTransform_Translate::mfSet(Matrix44& matr)
{
}

void SMatrixTransform_LightCMProject::mfSet(bool bSet)
{
  if (bSet)
  {
  }
}
void SMatrixTransform_LightCMProject::mfSet(Matrix44& matr)
{
}

void SMatrixTransform_Scale::mfSet(bool bSet)
{
  float *p;
  if (bSet)
  {
    p = m_Params[0].mfGet();
    D3DXMATRIX mat;
    D3DXMatrixScaling(&mat, p[0], p[1], p[2]);
    D3DXMatrixMultiply(gcpRendD3D->m_CurOpMatrix, gcpRendD3D->m_CurOpMatrix, &mat);
  }
}
void SMatrixTransform_Scale::mfSet(Matrix44& matr)
{
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
    memcpy(&gcpRendD3D->m_CurOpMatrix->m[0][0], &matr[0][0], 4*4*4);
  }
}
void SMatrixTransform_Matrix::mfSet(Matrix44& matr)
{
}

void SMatrixTransform_Rotate::mfSet(bool bSet)
{
  float *p;
  D3DXMATRIX mat;
  if (bSet)
  {
    p = m_Params[0].mfGet();
    switch (m_Offs)
    {
      case 1:
        D3DXMatrixRotationX(&mat, p[0]*M_PI/180.0f);
        break;
      case 2:
        D3DXMatrixRotationY(&mat, p[0]*M_PI/180.0f);
        break;
      case 4:
        D3DXMatrixRotationZ(&mat, p[0]*M_PI/180.0f);
        break;
      case 3:
        D3DXMatrixRotationAxis(&mat, &D3DXVECTOR3(1,1,0), p[0]*M_PI/180.0f);
        break;
      case 5:
        D3DXMatrixRotationAxis(&mat, &D3DXVECTOR3(1,0,1), p[0]*M_PI/180.0f);
        break;
      case 6:
        D3DXMatrixRotationAxis(&mat, &D3DXVECTOR3(0,1,1), p[0]*M_PI/180.0f);
        break;
      case 7:
        D3DXMatrixRotationAxis(&mat, &D3DXVECTOR3(1,1,1), p[0]*M_PI/180.0f);
        break;
      default:
        return;
    }
    D3DXMatrixMultiply(gcpRendD3D->m_CurOpMatrix, gcpRendD3D->m_CurOpMatrix, &mat);
  }
}
void SMatrixTransform_Rotate::mfSet(Matrix44& matr)
{
}

//====================================================================
// Array pointers for Direct3D
//================================================================

void SArrayPointer_Vertex::mfSet(int Id)
{
}

//=========================================================================================

void SArrayPointer_Normal::mfSet(int Id)
{
}

//=========================================================================================

void SArrayPointer_Texture::mfSet(int Id)
{
  int nIndex = ePT-eSrcPointer_Tex;
  assert(nIndex < 2);
  if (gcpRendD3D->m_RP.m_TexStages[Stage].TCIndex != nIndex)
  {
    gcpRendD3D->m_RP.m_TexStages[Stage].TCIndex = nIndex;
    gcpRendD3D->m_pd3dDevice->SetTextureStageState(Stage, D3DTSS_TEXCOORDINDEX, nIndex);
  }
}

//=========================================================================================

void SArrayPointer_Color::mfSet(int Id)
{
}

void SArrayPointer_SecColor::mfSet(int Id)
{
}

//==================================================================================


float SParamComp_Fog::mfGet()
{
  if (m_Type == 0)
    return gRenDev->m_FS.m_FogDensity;
  else
  if (m_Type == 1)
    return gRenDev->m_FS.m_FogStart / gRenDev->GetCamera().GetZMax();
  else
  if (m_Type == 2)
  {
    float fEnd =  gRenDev->m_FS.m_FogEnd; // / gRenDev->GetCamera().GetZMax();
    float fStart =  gRenDev->m_FS.m_FogStart; // / gRenDev->GetCamera().GetZMax();
    if (fEnd == fStart)
      return 1.0f;
    return 1.0f / (fEnd - fStart);
  }
  else
  if (m_Type == 3)
  {
    float fEnd =  gRenDev->m_FS.m_FogEnd; // / gRenDev->GetCamera().GetZMax();
    float fStart =  gRenDev->m_FS.m_FogStart; // / gRenDev->GetCamera().GetZMax();
    if (fEnd == fStart)
      return 10.0f;
    return fEnd / (fEnd - fStart);
  }
  return 0;
}

