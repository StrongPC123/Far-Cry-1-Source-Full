/*=============================================================================
  D3DShaders.cpp : Direct3D specific effectors/shaders functions implementation.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"

#include "D3DPShaders.h"
#include "D3DCGVProgram.h"


#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//============================================================================

static int sGetOp(char *op, SShader *ef)
{
  if (!stricmp(op, "KEEP"))
    return D3DSTENCILOP_KEEP;
  if (!stricmp(op, "REPLACE"))
    return D3DSTENCILOP_REPLACE;
  if (!stricmp(op, "INCR"))
    return D3DSTENCILOP_INCR;
  if (!stricmp(op, "DECR"))
    return D3DSTENCILOP_DECR;
  if (!stricmp(op, "ZERO"))
    return D3DSTENCILOP_ZERO;
  
  iLog->Log("Invalid StencilOp '%s' in Shader '%s\n", op, ef->m_Name.c_str());
  return D3DSTENCILOP_KEEP;
}

void SStencil_D3D::mfSet()
{
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();

  dv->SetRenderState(D3DRS_STENCILENABLE, TRUE);

  dv->SetRenderState(D3DRS_STENCILFUNC, Func);
  dv->SetRenderState(D3DRS_STENCILREF, FuncRef);
  dv->SetRenderState(D3DRS_STENCILMASK, FuncMask);

  dv->SetRenderState(D3DRS_STENCILFAIL, OpFail);
  dv->SetRenderState(D3DRS_STENCILZFAIL, OpZFail);
  dv->SetRenderState(D3DRS_STENCILPASS, OpZPass);
}

void SStencil_D3D::mfReset()
{
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  dv->SetRenderState(D3DRS_STENCILENABLE, FALSE);
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
  
  ef->m_State->m_Stencil = new SStencil_D3D;
  SStencil_D3D *sm = (SStencil_D3D *)ef->m_State->m_Stencil;

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
            sm->Func = D3DCMP_ALWAYS;
          else
          if (!stricmp(func[0], "NEVER"))
            sm->Func = D3DCMP_NEVER;
          else
          if (!stricmp(func[0], "LESS"))
            sm->Func = D3DCMP_LESS;
          else
          if (!stricmp(func[0], "LEQUAL"))
            sm->Func = D3DCMP_LESSEQUAL;
          else
          if (!stricmp(func[0], "GREATER"))
            sm->Func = D3DCMP_GREATER;
          else
          if (!stricmp(func[0], "GEQUAL"))
            sm->Func = D3DCMP_GREATEREQUAL;
          else
          if (!stricmp(func[0], "EQUAL"))
            sm->Func = D3DCMP_EQUAL;
          else
          if (!stricmp(func[0], "NOTEQUAL"))
            sm->Func = D3DCMP_NOTEQUAL;
          else
            iLog->Log("invalid StencilFunc '%s' in Shader '%s\n", func[0]);

          sm->FuncRef = atol(func[1]);
          sm->FuncMask = atol(func[2]);
        }
        break;

      case eOp:
        {
          char func[3][32];
          sscanf(data, "%s %s %s", func[0], func[1], func[2]);
          sm->OpFail = sGetOp(func[0], ef);
          sm->OpZFail = sGetOp(func[1], ef);
          sm->OpZPass = sGetOp(func[2], ef);
        }
        break;
    }
  }
}

void CShader::mfCompileVarsPak(char *scr, TArray<CVarCond>& Vars, SShader *ef)
{
  guard (CShader::mfCompileVarsPak);

  char *var;
  char *val;

  while ((shGetVar (&scr, &var, &val)) >= 0)
  {
    if (!var)
      continue;

    ICVar *vr = iConsole->GetCVar(var);
    if (!vr)
    {
      iLog->Log("Couldn't find console variable in shader '%s'\n", ef->m_Name.c_str());
      continue;
    }
    float v = shGetFloat(val);
    CVarCond vc;
    vc.m_Var = vr;
    vc.m_Val = v;
    Vars.AddElem(vc);
  }

  unguard;
}

void CShader::mfCompilePassConditions(SShader *ef, char *scr, SShaderPassHW *sl)
{
  guard(CShader::mfCompilePassConditions);

  char* name;
  long cmd;
  char *params;
  char *data;
  
  enum {eAmbient = 1, eVars, eHasLM};
  static tokenDesc commands[] =
  {
    {eAmbient, "Ambient"},
    {eHasLM, "HasLM"},
    {eVars, "Vars"},
      
    {0,0}
  };

  SPassConditions *pc = new SPassConditions;
  
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
      case eAmbient:
        pc->m_Flags |= SHPF_AMBIENT;
        break;

      case eHasLM:
        pc->m_Flags |= SHPF_HASLM;
        break;

      case eVars:
        mfCompileVarsPak(params, pc->m_Vars, ef);
        break;
    }
  }
  
  for (int i=0; i<m_PassConditions.Num(); i++)
  {
    if (pc->m_Flags != m_PassConditions[i]->m_Flags)
      continue;
    for (int j=0; j<pc->m_Vars.Num(); j++)
    {
      CVarCond *vr = &pc->m_Vars[j];
      for (int n=0; n<m_PassConditions[i]->m_Vars.Num(); n++)
      {
        if (m_PassConditions[i]->m_Vars[n].m_Var == vr->m_Var && m_PassConditions[i]->m_Vars[n].m_Val == vr->m_Val)
          break;
      }
      if (n == m_PassConditions[i]->m_Vars.Num())
        break;
    }
    if (j == pc->m_Vars.Num() && pc->m_Vars.Num() == m_PassConditions[i]->m_Vars.Num())
    {
      sl->m_Conditions = m_PassConditions[i];
      delete pc;
      break;
    }
  }
  if (i == m_PassConditions.Num())
  {
    m_PassConditions.AddElem(pc);
    sl->m_Conditions = m_PassConditions[m_PassConditions.Num()-1];
  }
  
  unguard;
}

#define GL_CONSTANT_COLOR0_NV               0x852A
#define GL_CONSTANT_COLOR1_NV               0x852B

bool CShader::mfCompileHWShadeLayer(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers)
{
  guard(CShader::mfCompileHWShadeLayer);

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

  enum {eLayer = 1, eLightType, ePShader, eRCombiner, eCGVPParam, eDeformVertexes, ePSParam, eTSParam, eRCParam, eArray, eMatrix, eNumLayers, eLightMaterial, eConditions, eCGVProgram, eLMIgnoreLights, eLMIgnoreProjLights, eLMNoAmbient, eLMBump, ePolyOffset, eLMNoAlpha, eColorMaterial};
  static tokenDesc commands[] =
  {
    {eLayer, "Layer"},
    {eLightType, "LightType"},
    {eDeformVertexes, "DeformVertexes"},
    {eLightMaterial, "LightMaterial"},
    {eNumLayers, "NumLayers"},
    {ePShader, "PShader"},
    {eCGVProgram, "CGVProgram"},
    {eRCombiner, "RCombiner"},
    {eCGVPParam, "CGVPParam"},
    {ePSParam, "PSParam"},
    {eRCParam, "RCParam"},
    {eTSParam, "TSParam"},
    {eArray, "Array"},
    {eMatrix, "Matrix"},
    {eConditions, "Conditions"},

    {ePolyOffset, "PolyOffset"},

    {eLMIgnoreLights, "LMIgnoreLights"},
    {eLMIgnoreProjLights, "LMIgnoreProjLights"},
    {eLMNoAmbient, "LMNoAmbient"},
    {eLMNoAlpha, "LMNoAlpha"},
    {eColorMaterial, "ColorMaterial"},
    {eLMBump, "LMBump"},

    {0,0}
  };

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
      case eNumLayers:
        sm->m_NumLayers = shGetInt(data);
        break;

      case eLightMaterial:
        sm->m_LightMaterial = SLightMaterial::mfGet(data);
        break;

      case eCGVProgram:
        {
          ef->m_Flags |= EF_HASVSHADER;
          sm->m_VProgram = CVProgram::mfForName(params, true);
          if (sm->m_VProgram && CRenderer::CV_r_precachecgshaders)
            sm->m_VProgram->mfPrecache();
          if (!ef->m_DefaultVProgram)
            ef->m_DefaultVProgram = sm->m_VProgram;
        }
        break;

      case eLayer:
        nl = atoi(name);
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
          iLog->Log("Unknown light type for light pass in effector '%s'\n", ef->m_Name.c_str());
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
        
      case ePShader:
        ps = CPShader::mfForName(params, false);
        if (ps)
          sm->m_FShader = ps;
        break;

      case eRCombiner:
        ps = CPShader::mfForName(params, false);
        if (ps)
          sm->m_FShader = ps;
        break;

      case eCGVPParam:
        mfCompileCGParam(params, ef, &sm->m_VPParamsNoObj, -1);
        break;

      case eTSParam:
      case eRCParam:
      case ePSParam:
        {
          if (!sm->m_RCParamsNoObj)
            sm->m_RCParamsNoObj = new TArray<SParam>;
          int n = sm->m_RCParamsNoObj->Num();
          mfCompileParam(params, ef, sm->m_RCParamsNoObj);
          if (n != sm->m_RCParamsNoObj->Num())
          {
            if (sm->m_RCParamsNoObj->Get(n).m_Reg == GL_CONSTANT_COLOR0_NV)
              sm->m_RCParamsNoObj->Get(n).m_Reg = 0;
            else
            if (sm->m_RCParamsNoObj->Get(n).m_Reg == GL_CONSTANT_COLOR1_NV)
              sm->m_RCParamsNoObj->Get(n).m_Reg = 1;
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

      case eConditions:
        mfCompilePassConditions(ef, params, sm);
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

      case eColorMaterial:
        sm->m_LMFlags |= LMF_COLMAT_AMB;
        break;
    }
  }
  if (n > 0 && !sm->m_LightFlags && !sm->m_Conditions)
  {
    if (!(sm->m_RenderState & GS_BLEND_MASK) && !(ef->m_Flags & EF_NEEDTANGENTS))
      iLog->Log("Shader '%s' has opaque maps defined after pass 0!!!\n", ef->m_Name.c_str());
    
    if ((sm->m_RenderState & GS_DEPTHWRITE) && !(ef->m_Flags & EF_NEEDTANGENTS))
      iLog->Log("Shader '%s' has depthwrite enabled after pass 0!!!\n", ef->m_Name.c_str());
  }
  sm->m_VPParamsNoObj.Shrink();
  sm->m_VPParamsObj.Shrink();
  if (sm->m_RCParamsNoObj)
    sm->m_RCParamsNoObj->Shrink();
  if (sm->m_RCParamsObj)
    sm->m_RCParamsObj->Shrink();
  sm->m_Pointers.Shrink();

  return true;

  unguard;
}

void CShader::mfCompileLightLayers(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers)
{
  guard(CShader::mfCompileLightLayers);

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
          if (!n)
            Layers[n].m_LightFlags |= DLF_HASAMBIENT;
          else
          if (Layers[n].m_RCParamsNoObj)
          {
            for (int i=0; i<Layers[n].m_RCParamsNoObj->Num(); i++)
            {
              SParam *pr = &Layers[n].m_RCParamsNoObj->Get(i);
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

  unguard;
}

void CShader::mfCompileHWConditions(SShader *ef, char *scr, SShaderTechnique *hs, int Id)
{
  guard(CShader::mfCompileHWConditions);

  char* name;
  long cmd;
  char *params;
  char *data;
  
  enum {eSingleLight = 1, eMultipleLights, eNoLights, eOnlyDirectional, eHasProjectedLights, eInShadow, eVars, eSpecular, eBended, eAlphaBlended, eNoBump, eHasLM, eHasDOT3LM, eDepthMaps, eHasVColors, eHasAlphaTest, eHasAlphaBlend, eHeatVision, eHotAmbient, eInFogVolume};
  static tokenDesc commands[] =
  {
    {eMultipleLights, "MultipleLights"},
    {eSingleLight, "SingleLight"},
    {eNoLights, "NoLights"},
    {eOnlyDirectional, "OnlyDirectional"},
    {eHasProjectedLights, "HasProjectedLights"},
    {eInShadow, "InShadow"},
    {eDepthMaps, "DepthMaps"},
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
    {eInFogVolume, "InFogVolume"},
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

      case eHeatVision:
        hc->m_Flags |= SHCF_HEATVISION;
        break;

      case eHotAmbient:
        hc->m_Flags |= SHCF_HOTAMBIENT;
        break;

      case eDepthMaps:
        hc->m_Flags |= SHCF_DEPTHMAPS;
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
        break;

      case eHasDOT3LM:
        hc->m_Flags |= SHCF_HASDOT3LM;
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
  
  unguard;
}

SShaderTechnique *CShader::mfCompileHW(SShader *ef, char *scr, int Id)
{
  guard(CShader::mfCompileHW);

  if (!m_CurEfsNum)
  {
    iLog->Log("Hardware section not allowed for Shader '%s'\n", ef->m_Name.c_str());
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

  enum {eVProgram = 1, eCGVProgram, eCull, eDeclareCGScript, eShadeLayer, ePass, eLight, eFirstLight, eArray, eMatrix, eConditions};
  static tokenDesc commands[] =
  {
    {eVProgram, "VProgram"},
    {eCGVProgram, "CGVProgram"},
    {eDeclareCGScript, "DeclareCGScript"},
    {eShadeLayer, "ShadeLayer"},
    {ePass, "Pass"},
    {eLight, "Light"},
    {eFirstLight, "FirstLight"},
    {eArray, "Array"},
    {eCull, "Cull"},
    {eMatrix, "Matrix"},
    {eConditions, "Conditions"},
    
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
          iLog->Log( "missing Cull argument in Shader '%s'\n", ef->m_Name.c_str());
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
          iLog->Log( "invalid Cull parm '%s' in Shader '%s'\n", data, ef->m_Name.c_str());
        break;

      case eVProgram:
        {
          ef->m_Flags |= EF_HASVSHADER;
          hs->m_VProgram = CVProgram::mfForName(params, false);
          if (!ef->m_DefaultVProgram)
            ef->m_DefaultVProgram = hs->m_VProgram;
        }
        break;

      case eCGVProgram:
        {
          ef->m_Flags |= EF_HASVSHADER;
          hs->m_VProgram = CVProgram::mfForName(params, true);
          if (hs->m_VProgram && CRenderer::CV_r_precachecgshaders)
            hs->m_VProgram->mfPrecache();
          if (!ef->m_DefaultVProgram)
            ef->m_DefaultVProgram = hs->m_VProgram;
        }
        break;

      case eShadeLayer:
      case ePass:
        mfCompileHWShadeLayer(ef, params, hs->m_Passes);
        break;

      case eConditions:
        mfCompileHWConditions(ef, params, hs, Id);
        break;

      case eFirstLight:
      case eLight:
        if (!hs->m_Passes.Num())
          hs->m_Flags |= FHF_FIRSTLIGHT;
        if (name && !strnicmp(name, "Spec", 4))
          mfCompileLightLayers(ef, params, hs->m_LightPasses[1]);
        else
          mfCompileLightLayers(ef, params, hs->m_LightPasses[0]);
        break;

      case eArray:
        mfCompileArrayPointer(hs->m_Pointers, params, ef);
        break;

      case eMatrix:
        mfCompileMatrixOp(hs->m_MatrixOps, params, name, ef);
        break;
    }
  }
  
  return hs;

  unguard;
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
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gRenDev->m_TexMan->m_CurStage;
  if (bEnable)
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL | cs);
  else
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
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
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gRenDev->m_TexMan->m_CurStage;
  if (bEnable)
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | cs);
  else
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
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
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gRenDev->m_TexMan->m_CurStage;
  if (bEnable)
  {
    if (gRenDev->m_RP.m_pRE)
      gRenDev->m_RP.m_pRE->m_nCountCustomData = 0;
    if (gRenDev->m_RP.m_pCurObject)
      gRenDev->m_RP.m_pCurObject->m_nCountCustomData = 0;
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
    dv->SetTextureStageState( cs, D3DTSS_TEXTURETRANSFORMFLAGS, n);
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | cs);
  }
  else
  {
    D3DXMATRIX mat;
    D3DXMatrixIdentity(&mat);
    dv->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+cs), &mat );
    dv->SetTextureStageState( cs, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | cs);
  }
  return true;
}

void SGenTC_ObjectLinear::mfCompile(char *scr, SShader *ef)
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
        gcEf.mfCompileParam(data, ef, &m_Params);
        break;

      case ePlaneT:
        gcEf.mfCompileParam(data, ef, &m_Params);
        break;

      case ePlaneR:
        gcEf.mfCompileParam(data, ef, &m_Params);
        break;

      case ePlaneQ:
        gcEf.mfCompileParam(data, ef, &m_Params);
        break;
    }
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
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  int cs = gRenDev->m_TexMan->m_CurStage;
  if (bEnable)
  {
    if (gRenDev->m_RP.m_pRE)
      gRenDev->m_RP.m_pRE->m_nCountCustomData = 0;
    if (gRenDev->m_RP.m_pCurObject)
      gRenDev->m_RP.m_pCurObject->m_nCountCustomData = 0;
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
        gcEf.mfCompileParam(data, ef, &m_Params);
        break;
        
      case ePlaneT:
        gcEf.mfCompileParam(data, ef, &m_Params);
        break;
        
      case ePlaneR:
        gcEf.mfCompileParam(data, ef, &m_Params);
        break;
        
      case ePlaneQ:
        gcEf.mfCompileParam(data, ef, &m_Params);
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

void CShader::mfCompileMatrixOp_Matrix(SMatrixTransform_Matrix *mt, SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  
  enum {eRow0 = 1, eRow1, eRow2, eRow3};
  static tokenDesc commands[] =
  {
    {eRow0, "Row0"},
    {eRow1, "Row1"},
    {eRow2, "Row2"},
    {eRow3, "Row3"},
    
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

    TArray<SParam> Params;
    Params.Free();
    
    switch (cmd)
    {
      case eRow0:
        mfCompileParam(params, ef, &Params);
        if (Params.Num())
          mt->m_Params[0] = Params[0];
        break;

      case eRow1:
        mfCompileParam(params, ef, &Params);
        if (Params.Num())
          mt->m_Params[1] = Params[0];
        break;

      case eRow2:
        mfCompileParam(params, ef, &Params);
        if (Params.Num())
          mt->m_Params[2] = Params[0];
        break;

      case eRow3:
        mfCompileParam(params, ef, &Params);
        if (Params.Num())
          mt->m_Params[3] = Params[0];
        break;
    }
  }
}

void CShader::mfCompileMatrixOp(TArray<SMatrixTransform>* List, char *scr, char *nmMat, SShader *ef)
{
  int Matr;
  if (!nmMat || !nmMat[0])
  {
    iLog->Log("Missing matrix name in Shader '%s'\n", ef->m_Name.c_str());
    return;
  }
  if (!stricmp(nmMat, "D3D_TEXTURE") || !stricmp(nmMat, "GL_TEXTURE"))
    Matr = D3DTS_TEXTURE0;
  else
  {
    iLog->Log("Unknown matrix name '%s' in Shader '%s'\n", nmMat, ef->m_Name.c_str());
    return;
  }

  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eIdentity = 1, eTranslate, eRotateX, eRotateY, eRotateZ, eRotate_XY, eRotate_XZ, eRotate_YZ, eScale, eTexStage, eMatrix, eLightCMProject};
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
    {eScale, "Scale"},
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
          mfCompileMatrixOp_Matrix(&mt, ef, params);
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

void SMatrixTransform_Identity::mfSet(bool bSet)
{
  if (bSet)
    D3DXMatrixIdentity(gcpRendD3D->m_CurOpMatrix);
}
void SMatrixTransform_Identity::mfSet(Matrix& matr)
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
void SMatrixTransform_Translate::mfSet(Matrix& matr)
{
}

void SMatrixTransform_LightCMProject::mfSet(bool bSet)
{
  if (bSet)
  {
  }
}
void SMatrixTransform_LightCMProject::mfSet(Matrix& matr)
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
void SMatrixTransform_Scale::mfSet(Matrix& matr)
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
    if (gRenDev->m_RP.m_pCurObject)
      gRenDev->m_RP.m_pCurObject->m_nCountCustomData = 0;
    for (int i=0; i<4; i++)
    {
      p = m_Params[i].mfGet();
      memcpy(&matr[i][0], p, 4*4);
    }
    memcpy(&gcpRendD3D->m_CurOpMatrix->m[0][0], &matr[0][0], 4*4*4);
  }
}
void SMatrixTransform_Matrix::mfSet(Matrix& matr)
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
void SMatrixTransform_Rotate::mfSet(Matrix& matr)
{
}

//====================================================================
// Array pointers for Direct3D
//================================================================

void SArrayPointer_Vertex::mfSet(int Id)
{
  int Stride;
  void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
}

//=========================================================================================

void SArrayPointer_Normal::mfSet(int Id)
{
  int Stride;
  void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
}

//=========================================================================================

void SArrayPointer_Texture::mfSet(int Id)
{
  int Stride;
  void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, (ESrcPointer)(eSrcPointer_Tex+Stage), 0);
  gRenDev->SelectTMU(0);
}

//=========================================================================================

void SArrayPointer_Color::mfSet(int Id)
{
  int Stride;
  void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
}

void SArrayPointer_SecColor::mfSet(int Id)
{
  int Stride;
  void *p = gRenDev->EF_GetPointer(ePT, &Stride, Type, ePT, 0);
}

//=========================================================================================

float SParamComp_Fog::mfGet()
{
  if (m_Type == 0)
    return gRenDev->m_FS.m_FogDensity;
  else
  if (m_Type == 1)
    return gRenDev->m_FS.m_FogStart / gRenDev->GetCamera().GetZMax();
  else
  if (m_Type == 2)
    return gRenDev->m_FS.m_FogEnd / gRenDev->GetCamera().GetZMax();
  else
  if (m_Type == 3)
  {
    float fEnd =  gRenDev->m_FS.m_FogEnd / gRenDev->GetCamera().GetZMax();
    float fStart =  gRenDev->m_FS.m_FogStart / gRenDev->GetCamera().GetZMax();
    return 1.0f / (fEnd - fStart);
  }
  return 0;
}
