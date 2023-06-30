/*=============================================================================
  D3DCGVPrograms.cpp : Direct3D cg vertex programs support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "D3DCGVProgram.h"

#ifndef PS2
#include <direct.h>
#include <io.h>
#else
#include "File.h"
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

TArray<CVProgram *> CVProgram::m_VPrograms;
vec4_t CCGVProgram_D3D::m_CurParams[256];
int CCGVProgram_D3D::m_nResetDeviceFrame = -1;


//=======================================================================

CVProgram *CVProgram::mfForName(const char *name, std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc, EShaderVersion eSHV, uint64 nMaskGen)
{
  int i;

  if (!name || !name[0])
    return NULL;

  if (!(gRenDev->GetFeatures() & (RFT_HW_VS)))
    return NULL;

  for (i=0; i<m_VPrograms.Num(); i++)
  {
    if (!m_VPrograms[i])
      continue;
    if (!stricmp(name, m_VPrograms[i]->m_Name.c_str()) && m_VPrograms[i]->m_nMaskGen == nMaskGen)
    {
      m_VPrograms[i]->m_nRefCounter++;
      return m_VPrograms[i];
    }
  }

  CVProgram *p = NULL;
  {
    CVProgram *pr = new CCGVProgram_D3D;
    pr->m_Name = name;
    pr->m_Id = i;
    m_VPrograms.AddElem(pr);
    pr->m_nRefCounter = 1;
    pr->m_nMaskGen = nMaskGen;
    p = pr;
  }
  if (eSHV == eSHV_VS_2_0)
    p->m_Flags |= VPFI_VS20ONLY;
  else
  if (eSHV == eSHV_VS_3_0)
    p->m_Flags |= VPFI_VS30ONLY;
  p->m_Flags |= VPFI_FX;
  CCGVProgram_D3D *pr = (CCGVProgram_D3D *)p;
  pr->mfConstructFX(Structs, Macros, entryFunc);

  return p;
}

void CCGVProgram_D3D::mfConstructFX(std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc)
{
  int i;
  char dir[128];
  sprintf(dir, "%sDeclarations/CGVShaders/", gRenDev->m_cEF.m_HWPath);
  m_EntryFunc = entryFunc;
  for (i=0; i<Macros.size(); i++)
  {
    char s[4096];
    SPair *pr = &Macros[i];
    _snprintf(s, 4096, "#define %s %s", pr->MacroName.c_str(), pr->Macro.c_str());
    SFXStruct str;
    str.m_Name = "";
    str.m_Struct = s;
    m_Functions.push_back(str);
  }
  for (i=0; i<Structs.size(); i++)
  {
    m_Functions.push_back(Structs[i]);
  }

  if (CRenderer::CV_r_shadersprecache)
    mfPrecache();
}

void CCGVProgram_D3D::mfPostLoad()
{
  gRenDev->m_cEF.mfCheckObjectDependParams(&m_ParamsNoObj, &m_ParamsObj);
}

void CCGVProgram_D3D::mfAddFXParameter(SFXParam *pr, const char *ParamName, SShader *ef)
{
  SCGMatrix mt;
  SCGParam4f CGpr;
  int i;
  char scr[256];

  if (pr->m_Assign.size())
  {
    // Ignore composite matrix
    if (!strnicmp(pr->m_Assign.c_str(), "WorldViewProjection", strlen("WorldViewProjection")) || !strnicmp(pr->m_Assign.c_str(), "ViewProjection", strlen("ViewProjection")))
      mt.m_eCGParamType = ECGP_Matr_ViewProj;
    else
    if (!stricmp(pr->m_Assign.c_str(), "World"))
      mt.m_eCGParamType = ECGP_Matr_World;
    else
    if (!stricmp(pr->m_Assign.c_str(), "WorldView") || !stricmp(pr->m_Assign.c_str(), "View"))
      mt.m_eCGParamType = ECGP_Matr_View;
    else
    if (!stricmp(pr->m_Assign.c_str(), "WorldViewInverse") || !stricmp(pr->m_Assign.c_str(), "ViewInverse"))
      mt.m_eCGParamType = ECGP_Matr_View_I;
    else
    if (!strnicmp(pr->m_Assign.c_str(), "ObjMatrix", strlen("ObjMatrix")))
      mt.m_eCGParamType = ECGP_Matr_Obj;
    else
    if (!strnicmp(pr->m_Assign.c_str(), "InvObjMatrix", strlen("InvObjMatrix")))
      mt.m_eCGParamType = ECGP_Matr_Obj_I;
    else
    if (!strnicmp(pr->m_Assign.c_str(), "TranspObjMatrix", strlen("TranspObjMatrix")))
      mt.m_eCGParamType = ECGP_Matr_Obj_T;
    else
    if (!stricmp(pr->m_Assign.c_str(), "InvTranspObjMatrix"))
      mt.m_eCGParamType = ECGP_Matr_Obj_IT;
    if (mt.m_eCGParamType != ECGP_Unknown)
    {
      assert(pr->m_nRows == 4 && pr->m_nColumns == 4);
      mt.m_Name = ParamName;
      m_MatrixObj.AddElem(mt);
    }
    else
    {
      const char *translatedParam = pr->m_Assign.c_str();
      scr[0] = 0;
      if (pr->m_nRows == 3)
      {
        if (!strnicmp(translatedParam, "OSCameraPos", strlen("OSCameraPos")))
          sprintf(scr, "Name=%s Comp '%s pos 0' Comp '%s pos 1' Comp '%s pos 2'", ParamName, translatedParam, translatedParam, translatedParam);
        else
          sprintf(scr, "Name=%s Comp '%s[0]' Comp '%s[1]' Comp '%s[2]'", ParamName, translatedParam, translatedParam, translatedParam);
      }
      else
      if (pr->m_nRows == 4)
        sprintf(scr, "Name=%s Comp '%s[0]' Comp '%s[1]' Comp '%s[2]' Comp '%s[3]'", ParamName, translatedParam, translatedParam, translatedParam, translatedParam);
      else
      if (pr->m_nRows <= 1)
        sprintf(scr, "Name=%s Comp '%s'", ParamName, translatedParam);
      if (scr[0])
        gRenDev->m_cEF.mfCompileCGParam(scr, ef, &m_ParamsNoObj);
      else
        assert(0);
    }
  }
  else
  {
    SParamComp_Const pc;
    CGpr.m_Name = ParamName;
    switch(pr->m_Type)
    {
      case eType_FLOAT:
        pc.m_Val = pr->m_Value.m_Float;
        CGpr.m_Comps[0] = SParamComp::mfAdd(&pc);
    	  break;
      case eType_VECTOR:
        {
          for (i=0; i<3; i++)
          {
            pc.m_Val = pr->m_Value.m_Vector[i];
            CGpr.m_Comps[i] = SParamComp::mfAdd(&pc);
          }
        }
    	  break;
      case eType_FCOLOR:
        {
          for (i=0; i<4; i++)
          {
            pc.m_Val = pr->m_Value.m_Color[i];
            CGpr.m_Comps[i] = SParamComp::mfAdd(&pc);
          }
        }
    	  break;
      default:
        assert(0);
    }
    m_ParamsNoObj.AddElem(CGpr);
  }
}

bool CCGVProgram_D3D::mfGetFXParamNameByID(int nParam, char *ParamName)
{
  int i;
  ParamName[0] = 0;
  if (!m_Functions.size())
    return false;
  if (!m_EntryFunc.GetIndex())
    return false;
  for (i=0; i<m_Functions.size(); i++)
  {
    const char *funcName = m_Functions[i].m_Name.c_str();
    if (!strcmp(m_EntryFunc.c_str(), funcName))
    {
      const char *pFunc = m_Functions[i].m_Struct.c_str();
      char *s = strchr(pFunc, '(');
      s = strchr(s, ',');
      s++;
      int n = 0;
      char Param[128];
      while (nParam != n)
      {
        while (*s != ',' && *s != ')' && *s != 0) { s++; }
        if (*s == ')' || *s == 0)
          break;
        n++;
        s++;
      }
      if (n != nParam)
        return false;
      SkipCharacters(&s, kWhiteSpace);
      n = 0;
      while (*s != ',' && *s != ')') { Param[n++] = *s++; }
      Param[n] = 0;
      n = 0;
      if (!strncmp(Param, "uniform", 7))
        n = sizeof("uniform");
      s = &Param[n];
      SkipCharacters(&s, kWhiteSpace);
      while (*s > 0x20) { s++; }
      SkipCharacters(&s, kWhiteSpace);
      n = 0;
      while (*s > 0x20 && *s != ',' && *s != ')') { ParamName[n++] = *s++; }
      ParamName[n] = 0;
      break;
    }
  }
  return true;
}

struct SVSAliasName
{
  bool bParam;
  std::string NameINT;
  std::string Name;
};
struct SVSAliasSampler
{
  SFXSampler *fxSampler;
  std::string NameINT;
  SVSAliasSampler()
  {
    fxSampler = NULL;
  }
};

void CCGVProgram_D3D::mfGatherFXParameters(const char *buf, SShaderPassHW *pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader *ef)
{
  assert(m_Insts.Num());
  assert(!pSHPass->m_TUnits.Num());

  if (!m_Insts.Num())
    return;
  SCGInstance *cgi = &m_Insts[0];
  if (!cgi->m_BindVars)
    return;
  int i, j;
  assert(*buf == '(');
  buf++;
  SVSAliasSampler samps[MAX_TMU];
  int nMaxSampler = -1;
  std::vector<SVSAliasName> Aliases;
  int nParam = 0;
  char VSParam[256];
  while (true)
  {
    int n = 0;
    char szParam[128];
    while(*buf <= 0x20 || *buf == ',')  { buf++; }
    while(*buf > 0x20 && *buf != ',' && *buf != ')')  { szParam[n++] = *buf++; }
    szParam[n] = 0;
    if (!n)
      break;
    for (i=0; i<Params.size(); i++)
    {
      SFXParam *pr = &Params[i];
      if (!stricmp(pr->m_Name.c_str(), szParam))
      {
        SVSAliasName an;
        an.bParam = true;
        an.Name = szParam;
        if (mfGetFXParamNameByID(nParam, VSParam))
          an.NameINT = VSParam;
        else
        {
          assert(0);
          an.NameINT = szParam;
        }
        Aliases.push_back(an);
        break;
      }
    }
    if (i == Params.size())
    {
      for (i=0; i<Samplers.size(); i++)
      {
        SFXSampler *sm = &Samplers[i];
        if (!stricmp(sm->m_Name.c_str(), szParam))
        {
          SVSAliasName an;
          an.bParam = false;
          an.Name = szParam;
          if (mfGetFXParamNameByID(nParam, VSParam))
            an.NameINT = VSParam;
          else
          {
            assert(0);
            an.NameINT = szParam;
          }
          Aliases.push_back(an);
          break;
        }
      }
      if (i == Samplers.size())
        iLog->Log("Warning: Couldn't find declaration of parameter '%s' for vertex shader '%s' (Shader: %s)", szParam, m_EntryFunc.c_str(), ef->m_Name.c_str());
    }
    nParam++;
  }
  for (i=0; i<cgi->m_BindVars->Num(); i++)
  {
    SCGBind *bn = &cgi->m_BindVars->Get(i);
    const char *param = bn->m_Name.c_str();
    const char *paramINT = param;
    bool bSampler = (bn->m_dwBind & SHADER_BIND_SAMPLER) != 0;
    for (j=0; j<Aliases.size(); j++)
    {
      SVSAliasName *al = &Aliases[j];
      if (al->NameINT == paramINT)
      {
        if (!bSampler)
          assert (al->bParam);
        else
          assert (!al->bParam);
        param = al->Name.c_str();
        break;
      }
    }
    if (!bSampler)
    {
      for (j=0; j<Params.size(); j++)
      {
        SFXParam *pr = &Params[j];
        if (!stricmp(pr->m_Name.c_str(), param))
        {
          mfAddFXParameter(pr, paramINT, ef);
          break;
        }
      }
      if (j == Params.size())
        assert(0);
    }
    else
    {
      for (j=0; j<Samplers.size(); j++)
      {
        SFXSampler *sm = &Samplers[j];
        if (!stricmp(sm->m_Name.c_str(), param))
        {
          int nSampler = bn->m_dwBind & 0xf;
          nMaxSampler = max(nSampler, nMaxSampler);
          samps[nSampler].fxSampler = sm;
          samps[nSampler].NameINT = paramINT;
          break;
        }
      }
      if (j == Samplers.size())
        assert(0);
    }
  }
  for (i=0; i<=nMaxSampler; i++)
  {
    SFXSampler *smp = samps[i].fxSampler;
    assert(smp->m_nTextureID < Textures.size());
    SFXTexture *tx = &Textures[smp->m_nTextureID];
#ifdef USE_FX
    gRenDev->m_cEF.mfParseFXTechnique_LoadShaderTexture(smp, tx, pSHPass, ef, i, eCO_NOSET, eCO_NOSET, DEF_TEXARG0, DEF_TEXARG0);
#endif
  }
}

int CCGVProgram_D3D::mfVertexFormat(bool &bUseTangents, bool &bUseLM)
{
  int nVFormat = VERTEX_FORMAT_P3F;
  bool bNormal = false;
  bool bTangent = false;
  bool bTC0 = false;
  bool bTC1 = false;
  bool bCol = false;
  bool bSecCol = false;
  int i, j;
  if (m_Functions.size() && m_EntryFunc.GetIndex())
  {
    for (i=0; i<m_Functions.size(); i++)
    {
      const char *funcName = m_Functions[i].m_Name.c_str();
      if (!strcmp(m_EntryFunc.c_str(), funcName))
      {
        const char *pFunc = m_Functions[i].m_Struct.c_str();
        const char *s = strchr(pFunc, '(');
        if (s)
        {
          s++;
          while (*s <= 0x20) { s++; }
          char pSTR0[128];
          const char *pSTR1;
          int n = 0;
          while (*s > 0x20) { pSTR0[n++] = *s++; }
          pSTR0[n] = 0;
          for (j=0; j<m_Functions.size(); j++)
          {
            const char *pSTR1 = m_Functions[j].m_Name.c_str();
            if (!strcmp(pSTR1, pSTR0))
              break;
          }
          if (j == m_Functions.size())
          {
            assert(0);
            return nVFormat;
          }
          pSTR1 = m_Functions[j].m_Struct.c_str();
          s = pSTR1;
          while (true)
          {
            s = strchr(s, ';');
            if (!s)
              break;
            const char *ss = s-1;
            while (*ss <= 0x20) { ss--; }
            while (*ss > 0x20 && *ss != ':') { ss--; }
            int n = 0;
            char str[64];
            ss++;
            while (*ss > 0x20 && *ss != ';') { str[n++] = *ss++; }
            str[n] = 0;
            if (!stricmp(str, "COLOR0"))
              bCol = true;
            else
            if (!stricmp(str, "COLOR1"))
              bSecCol = true;
            else
            if (!stricmp(str, "TEXCOORD0"))
              bTC0 = true;
            else
            if (!stricmp(str, "TEXCOORD1"))
              bTC1 = true;
            else
            if (!stricmp(str, "NORMAL"))
              bNormal = true;
            else
            if (!strnicmp(str, "TANGENT", 7) || !strnicmp(str, "BINORMAL", 8) || !strnicmp(str, "BLENDWEIGHT", 11))
              bTangent = true;
            s++;
          }
          break;
        }
        else
        {
          iLog->Log("Error: Wrong VertexShader '%s' function '%s'", m_Name.c_str(), m_EntryFunc.c_str());
          break;
        }
      }
    }
  }
  bUseLM = bTC1;
  bUseTangents = bTangent;

  nVFormat = VertFormatForComponents(bCol, bSecCol, bNormal, bTC0);

  return nVFormat;
}

//=========================================================================================

CVProgram *CVProgram::mfForName(const char *name, uint64 nMaskGen)
{
  int i;

  if (!name || !name[0])
    return NULL;

  if (!(gRenDev->GetFeatures() & (RFT_HW_VS)))
    return NULL;

  for (i=0; i<m_VPrograms.Num(); i++)
  {
    if (!m_VPrograms[i])
      continue;
    if (!stricmp(name, m_VPrograms[i]->m_Name.c_str()) && m_VPrograms[i]->m_nMaskGen == nMaskGen)
    {
      m_VPrograms[i]->m_nRefCounter++;
      return m_VPrograms[i];
    }
  }

  char scrname[128];
  char dir[128];
  sprintf(dir, "%sDeclarations/CGVShaders/", gRenDev->m_cEF.m_HWPath);
  sprintf(scrname, "%s%s.crycg", dir, name);
  FILE *fp = iSystem->GetIPak()->FOpen(scrname, "r");
  if (!fp)
  {
		Warning( 0,0,"Couldn't find vertex shader '%s'", name);
    return NULL;
  }
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
  int len = iSystem->GetIPak()->FTell(fp);
  char *buf = new char [len+1];
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
  len = iSystem->GetIPak()->FRead(buf, 1, len, fp);
  iSystem->GetIPak()->FClose(fp);
  buf[len] = 0;
  buf = gRenDev->m_cEF.mfScriptPreprocessor(buf, dir, scrname);

  CVProgram *p = NULL;
  {
    CVProgram *pr = new CCGVProgram_D3D;
    pr->m_Name = name;
    pr->m_Id = i;
    m_VPrograms.AddElem(pr);
    pr->m_nRefCounter = 1;
    pr->m_nMaskGen = nMaskGen;
    p = pr;
    FILE *handle = iSystem->GetIPak()->FOpen(scrname, "rb");
    if (handle)
    {
      p->m_WriteTime = iSystem->GetIPak()->GetModificationTime(handle);
      iSystem->GetIPak()->FClose(handle);
    }
  }

  p->mfCompile(buf);
  delete [] buf;
  if (CRenderer::CV_r_shadersprecache)
    p->mfPrecache();

  return p;
}

//=======================================================================

TArray<SCGScript *> CCGVProgram_D3D::m_CGScripts;

void SCGScript::mfRemoveFromList()
{
  int i;

  for (i=0; i<CCGVProgram_D3D::m_CGScripts.Num(); i++)
  {
    SCGScript *scr = CCGVProgram_D3D::m_CGScripts[i];
    if (scr == this)
      CCGVProgram_D3D::m_CGScripts[i] = NULL;
  }
}

void CCGVProgram_D3D::mfReset()
{
  int i;
  for (i=0; i<m_Insts.Num(); i++)
  {
    m_CurInst = i;
    mfDelInst();
  }
  m_Insts.Free();

  for (i=0; i<m_InstCache.Num(); i++)
  {
    if (m_InstCache[i].m_pCache)
    {
      gRenDev->m_cEF.CloseCacheFile(m_InstCache[i].m_pCache);
      m_InstCache[i].m_pCache = NULL;
    }
  }
  m_InstCache.Free();

  m_dwFrame++;
}

void CCGVProgram_D3D::mfFree()
{
  m_Flags = 0;

  if (m_Script && !m_Script->m_Name.GetIndex())
    SAFE_RELEASE(m_Script);
  if (m_CoreScript && !m_CoreScript->m_Name.GetIndex())
    SAFE_RELEASE(m_CoreScript);
  if (m_SubroutinesScript && !m_SubroutinesScript->m_Name.GetIndex())
    SAFE_RELEASE(m_SubroutinesScript);
  if (m_DeclarationsScript && !m_DeclarationsScript->m_Name.GetIndex())
    SAFE_RELEASE(m_DeclarationsScript);
  if (m_InputParmsScript && !m_InputParmsScript->m_Name.GetIndex())
    SAFE_RELEASE(m_InputParmsScript);
  if (m_PosScript && !m_PosScript->m_Name.GetIndex())
    SAFE_RELEASE(m_PosScript);

  m_Pointers.Free();
  m_ParamsNoObj.Free();
  m_ParamsObj.Free();
  m_MatrixObj.Free();

  mfReset();
}

CCGVProgram_D3D::~CCGVProgram_D3D()
{
  mfFree();
  CVProgram::m_VPrograms[m_Id] = NULL;
}

void CCGVProgram_D3D::Release()
{
  m_nRefCounter--;
  if (!m_nRefCounter)
    delete this;
}

bool CCGVProgram_D3D::mfHasPointer(ESrcPointer ePtr)
{
  for (int i=0; i<m_Pointers.Num(); i++)
  {
    SArrayPointer *vp = m_Pointers[i];
    if (vp->ePT == ePtr)
      return true;
  }
  return false;
}

static char *sAdditionalVP[][2][16] = 
{
  {
    {
      {
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(ModelViewProj._31_32_33_34, vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(ModelViewProj._31_32_33_34, vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },

      {
        "return OUT;\n"
        "}\n"
      },
      {
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      }
    },
    {
      {
        "OUT.FogC = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(ModelViewProj._31_32_33_34, vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(ModelViewProj._31_32_33_34, vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },

      {
        "OUT.FogC = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      }
    }
  },

  // Instancing support
  {
    {
      {
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(_CompMatrix[2], vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(_CompMatrix[2], vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },

      {
        "return OUT;\n"
        "}\n"
      },
      {
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      }
    },
    {
      {
        "OUT.FogC = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(_CompMatrix[2], vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "float fCameraSpacePosZ = dot(_CompMatrix[2], vPos);\n"
        "OUT.FogC = clamp((Fog.y - Fog.x*fCameraSpacePosZ), g_VSCONST_0_025_05_1.x, g_VSCONST_0_025_05_1.w);\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },

      {
        "OUT.FogC = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      },
      {
        "OUT.FogC = vPos.w;\n"
        "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
        "OUT.Tex3.xyw = vPos.w;\n"
        "return OUT;\n"
        "}\n"
      }
    }
  },
};

SCGScript *CCGVProgram_D3D::mfGenerateScriptVP(CVProgram *pPosVP)
{
  TArray<char> newScr;
  int i;

  newScr.Copy("# define _VS\n", strlen("# define _VS\n"));

  if (m_Insts[m_CurInst].m_Mask & VPVST_HDR)
    newScr.Copy("# define _HDR\n", strlen("# define _HDR\n"));

  if (m_Insts[m_CurInst].m_Mask & VPVST_INSTANCING_ROT)
    newScr.Copy("# define _INST_R\n", strlen("# define _INST_R\n"));
  else
  if (m_Insts[m_CurInst].m_Mask & VPVST_INSTANCING_NOROT)
    newScr.Copy("# define _INST_NR\n", strlen("# define _INST_NR\n"));

  if (m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL)
  {
    newScr.Copy("# define _FOG\n", strlen("# define _FOG\n"));
    newScr.Copy("# define _FOG_LIN\n", strlen("# define _FOG_LIN\n"));
  }
  
  if (m_Flags & VPFI_SUPPORTS_MULTILIGHTS)
  {
    char str[256];
    int nLights = m_Insts[m_CurInst].m_LightMask & 0xf;
    sprintf(str, "# define _NUM_LIGHTS %d\n", nLights);
    newScr.Copy(str, strlen(str));
    for (int i=0; i<nLights; i++)
    {
      int nLightType = (m_Insts[m_CurInst].m_LightMask >> (SLMF_LTYPE_SHIFT + i*4)) & SLMF_TYPE_MASK;
      int nOnlySpec = ((m_Insts[m_CurInst].m_LightMask >> (SLMF_LTYPE_SHIFT + i*4)) & SLMF_ONLYSPEC) != 0;
      int nSpecOccl = ((m_Insts[m_CurInst].m_LightMask >> (SLMF_LTYPE_SHIFT + i*4)) & SLMF_SPECOCCLUSION) != 0;
      sprintf(str, "# define _LIGHT%d_TYPE %d\n", i, nLightType);
      newScr.Copy(str, strlen(str));
      sprintf(str, "# define _LIGHT%d_ONLYSPEC %d\n", i, nOnlySpec);
      newScr.Copy(str, strlen(str));
      sprintf(str, "# define _LIGHT%d_SPECOCCLUSION %d\n", i, nSpecOccl);
      newScr.Copy(str, strlen(str));
    }
  }

  if (m_DeclarationsScript)
    newScr.Copy(m_DeclarationsScript->m_Script, strlen(m_DeclarationsScript->m_Script));

  char *szInputParms = "";
  if (m_InputParmsScript)
    szInputParms = m_InputParmsScript->m_Script;
  SCGScript *posScr = m_PosScript;
  char str[4096];
  char sStr[4096];
  if (posScr && posScr->m_Name != m_Insts[m_CurInst].m_PosScriptName)
  {
    strcpy(str, szInputParms);
    szInputParms = str;
    if (pPosVP && pPosVP->m_bCGType)
    {
      CCGVProgram_D3D *pVPD3D = (CCGVProgram_D3D *)pPosVP;
      for (int i=0; i<pVPD3D->m_ParamsNoObj.Num(); i++)
      {
        if (!strstr(str, pVPD3D->m_ParamsNoObj[i].m_Name.c_str()))
        {
          strcat(str, ", uniform float4 ");
          strcat(str, pVPD3D->m_ParamsNoObj[i].m_Name.c_str());
        }
      }
      for (int i=0; i<pVPD3D->m_ParamsObj.Num(); i++)
      {
        if (!strstr(str, pVPD3D->m_ParamsObj[i].m_Name.c_str()))
        {
          strcat(str, ", uniform float4 ");
          strcat(str, pVPD3D->m_ParamsObj[i].m_Name.c_str());
        }
      }
    }
  }

  if (m_SubroutinesScript)
    newScr.Copy(m_SubroutinesScript->m_Script, strlen(m_SubroutinesScript->m_Script));

  if (m_Functions.size())
  {
    for (i=0; i<m_Functions.size(); i++)
    {
      newScr.Copy(m_Functions[i].m_Struct.c_str(), strlen(m_Functions[i].m_Struct.c_str()));
      newScr.AddElem('\n');
    }
  }
  else
  {
    sprintf(sStr, "vertout main(appin IN, %s)\n{\n  vertout OUT;\n", szInputParms);
    newScr.Copy(sStr, strlen(sStr));
  }

  if (pPosVP)
  {
    CCGVProgram_D3D *pVP = (CCGVProgram_D3D *)pPosVP;
    SCGScript *pScr = pVP->m_PosScript;
    if (CName(pPosVP->m_Name.c_str(), eFN_Find) != m_Insts[m_CurInst].m_PosScriptName)
    {
      SCGScript *pS = mfAddNewScript(m_Insts[m_CurInst].m_PosScriptName.c_str(), NULL);
      if (pS)
        pScr = pS;
    }
    if (pScr)
      newScr.Copy(pScr->m_Script, strlen(pScr->m_Script));
  }

  if (m_CoreScript)
    newScr.Copy(m_CoreScript->m_Script, strlen(m_CoreScript->m_Script));

  if (!m_Functions.size())
  {
    char *pExit = "return OUT;\n}";
    newScr.Copy(pExit, strlen(pExit));
  }
  newScr.AddElem(0);

  if (m_Flags & VPFI_AUTOENUMTC)
  {
    int n = 0;
    char *s = &newScr[0];
    while (true)
    {
      s = strstr(s, "TEXCOORDN");
      if (!s)
        break;
      s[8] = 0x30 + n;
      int m = -1;
      while (s[m] == 0x20 || s[m] == 0x8) {m--;}
      if (s[m] == ':')
      {
        m--;
        while (s[m] == 0x20 || s[m] == 0x8) {m--;}
      }
      if (s[m] == ']')
      {
        while (s[m] != '[') {m--;}
        char ss[16];
        m++;
        int mm = 0;
        while (s[m] != ']') {ss[mm++] = s[m++];}
        ss[mm] = 0;
        n += atoi(ss);
      }
      else
        n++;
    }

    char *sNewStr = strstr(&newScr[0], "main");
    int nDepth = 0;
    if (sNewStr)
    {
      n = 0;
      while (true)
      {
        if (sNewStr[n] == '(')
        {
          nDepth++;
        }
        else
        if (sNewStr[n] == ')')
        {
          if (nDepth == 1)
          {
            n--;
            while (sNewStr[n] == 0x20 || sNewStr[n] == 9 || sNewStr[n] == 0xa) {n--;}
            if (sNewStr[n] == ',')
              sNewStr[n] = 0x20;
            break;
          }
          nDepth--;
        }
        n++;
      }
    }
  }

  SCGScript *cgs = mfAddNewScript(NULL, &newScr[0]);

  return cgs;
}

//===============================================================================
// TexGen / TexTransform modifications

static char *sEyeSpaceCamVecVP = 
{
  "float4 tcEPos = mul(ModelView, vPos);\n"
  "float3 tcCamVec = normalize(tcEPos.xyz);\n"
};
static char *sEyeSpaceNormal = 
{
  "float3 tcNormal = normalize(mul((float3x3)ModelViewIT, IN.TNormal.xyz));\n"
};
static char *sReflectVec = 
{
  "float3 tcRef = reflect(tcCamVec, tcNormal.xyz);\n"
};
static char *sSphereMapGenScriptVP[] = 
{
  sEyeSpaceCamVecVP,
  sEyeSpaceNormal,
  sReflectVec,
  {
    "float3 tcEm = tcRef + float3(0,0,1);\n"
    "tcEm.x = 2 * sqrt(dot(tcEm, tcEm));\n"
    "float4 tcSM = {0,0,0,1};\n"
    "tcSM.xy = tcRef.xy/tcEm.x + 0.5;\n"
  },
  NULL
};
static char *sReflectionMapGenScriptVP[] = 
{
  sEyeSpaceCamVecVP,
  sEyeSpaceNormal,
  sReflectVec,
  {
    "float4 tcRM;\n"
    "tcRM.xyz = tcRef.xyz;\n"
    "tcRM.w = vPos.w;\n"
  },
  NULL
};
static char *sNormalMapGenScriptVP[] = 
{
  sEyeSpaceNormal,
  {
    "float4 tcNM;\n"
    "tcNM.xyz = tcNormal.xyz;\n"
    "tcNM.w = vPos.w;\n"
  },
  NULL
};

char *CCGVProgram_D3D::mfGenerateTCScript(char *Script, int nt)
{
  int tcGOLMask = VPVST_TCGOL0<<nt;
  int tcGRMMask = VPVST_TCGRM0<<nt;
  int tcGNMMask = VPVST_TCGNM0<<nt;
  int tcGSMMask = VPVST_TCGSM0<<nt;
  int tcMMask = VPVST_TCM0<<nt;
  int n, m;
  if (m_Insts[m_CurInst].m_Mask & (tcGOLMask | tcGRMMask | tcGNMMask | tcGSMMask | tcMMask))
  {
    /*if (!strstr(Script, "IN.TexCoord0") && !strstr(Script, "IN.baseTC"))
    {
      if (!(m_Insts[m_CurInst].m_Mask & (tcGOLMask | tcGRMMask | tcGNMMask | tcGSMMask)))
        return Script;
    }*/
    char sOTex[64];
    char sMTCG[64];
    char sMTCM[64];
    char sStr1[512];
    char sStr[512];
    sprintf(sOTex, "OUT.Tex%d", nt);
    char *svo;
    if (svo=strstr(Script, "vertout"))
    {
      char TC[32];
      sprintf(TC, "TEXCOORD%d", nt);
      n = 0;
      if (svo=strstr(svo, TC))
      {
        n--;
        while (svo[n]==0x20 || svo[n]==8 || svo[n]==':')
        {
          if (svo[n]==0xa)
            break;
          n--;
        }
        if (svo[n] != 0xa)
        {
          while (svo[n]!=0x20 && svo[n]!=8) { n--; }
          n++;
          m = 4;
          while (svo[n]!=0x20 && svo[n]!=8 && svo[n]!=':')
          {
            sOTex[m++] = svo[n];
            n++;
          }
          sOTex[m] = 0;
        }
      }
    }

    sprintf(sMTCG, "MatrixTCG%d", nt);
    sprintf(sMTCM, "MatrixTCM%d", nt);
    char *sNewStr = strstr(Script, sOTex);
    if (sNewStr)
    {
      bool bCamPos = false;
      bool bModV = false;
      bool bModVIT = false;
      if (m_Insts[m_CurInst].m_Mask & (tcGRMMask | tcGSMMask))
      {
        if (strstr(Script, "CameraPos"))
          bCamPos = true;
      }
      if (strstr(Script, "ModelViewIT"))
        bModVIT = true;
      char *sss = strstr(Script, "ModelView");
      while (sss)
      {
        if (sss[9]==',' || sss[9]==' ' || sss[9]==')' || sss[9]==0x9 || sss[9]==0xa)
        {
          bModV = true;
          break;
        }
        sss = strstr(sss+1, "ModelView");
      }

      char *sNStr = sNewStr;
      char sINTex[64];
      strcpy(sINTex, "IN.TexCoord0");
      while (sNStr = strstr(sNStr, sOTex))
      {
        n = 0;
        while (sNStr[n]!=0xa && sNStr[n]!=0)
        {
          if (sNStr[n] == '=')
          {
            sNStr[n] = 0x20;
            n++;
            while (sNStr[n]==0x20 || sNStr[n]==8) { n++; }
            m = 0;
            while (sNStr[n]!=';')
            {
              sINTex[m++] = sNStr[n];
              sINTex[m] = 0;
              sNStr[n++] = 0x20;
              if (sNStr[n]=='.' && m > 3)
                break;
            }
            while (sNStr[n]!=0xa && sNStr[n]!=0) {sNStr[n]=0x20; n++;}
            break;
          }
          sNStr[n]=0x20;
          n++;
        }
      }
      n = 0;
      while (sNewStr[n]!=0xa && sNewStr[n]!=0) {sNewStr[n]=0x20; n++;}

      // Add texgen code to the final VP
      char **pNewScripts = NULL;
      sStr1[0] = 0;
      if (m_Insts[m_CurInst].m_Mask & tcGOLMask)
        sprintf(sStr1,  "float4 tcOL%d = mul(%s, vPos);\n", nt, sMTCG);
      else
      if (m_Insts[m_CurInst].m_Mask & tcGRMMask)
        pNewScripts = sReflectionMapGenScriptVP;
      else
      if (m_Insts[m_CurInst].m_Mask & tcGSMMask)
        pNewScripts = sSphereMapGenScriptVP;
      else
      if (m_Insts[m_CurInst].m_Mask & tcGNMMask)
        pNewScripts = sNormalMapGenScriptVP;
      if (pNewScripts)
      {
        n = 0;
        while(pNewScripts[n])
        {
          if (!strstr(Script, pNewScripts[n]))
          {
            strcat(sStr1, pNewScripts[n]);
          }
          n++;
        }
      }

      if (m_Insts[m_CurInst].m_Mask & tcMMask)
      {
        if (m_Insts[m_CurInst].m_Mask & tcGOLMask)
          sprintf(sStr, "%s %s = mul(%s, tcOL%d);\n", sStr1, sOTex, sMTCM, nt);
        else
        if (m_Insts[m_CurInst].m_Mask & tcGRMMask)
          sprintf(sStr, "%s %s = mul(%s, tcRM);\n", sStr1, sOTex, sMTCM);
        else
        if (m_Insts[m_CurInst].m_Mask & tcGSMMask)
          sprintf(sStr, "%s %s = mul(%s, tcSM);\n", sStr1, sOTex, sMTCM);
        else
        if (m_Insts[m_CurInst].m_Mask & tcGNMMask)
          sprintf(sStr, "%s %s = mul(%s, tcNM);\n", sStr1, sOTex, sMTCM);
        else
          sprintf(sStr, "%s = mul(%s, %s);\n", sOTex, sMTCM, sINTex);
      }
      else
      {
        if (m_Insts[m_CurInst].m_Mask & tcGOLMask)
          sprintf(sStr, "%s %s = tcOL%d;\n", sStr1, sOTex, nt);
        else
        if (m_Insts[m_CurInst].m_Mask & tcGRMMask)
          sprintf(sStr, "%s %s = tcRM;\n", sStr1, sOTex);
        else
        if (m_Insts[m_CurInst].m_Mask & tcGSMMask)
          sprintf(sStr, "%s %s = tcSM;\n", sStr1, sOTex);
        else
        if (m_Insts[m_CurInst].m_Mask & tcGNMMask)
          sprintf(sStr, "%s %s = tcNM;\n", sStr1, sOTex);
      }
      size_t len = strlen(sStr);
      size_t len2 = strlen(Script)+1;
      char *newScr2 = new char[len+len2];
      n = sNewStr-Script;
      strncpy(newScr2, Script, n);
      strcpy(&newScr2[n], sStr);
      strcpy(&newScr2[n+len], &Script[n]);
      delete [] Script;
      Script = newScr2;

      if (m_Insts[m_CurInst].m_Mask & (tcGOLMask | tcMMask | tcGRMMask | tcGSMMask | tcGNMMask))
      {
        sNewStr = strstr(Script, "main");
        if (sNewStr)
        {
          n = 0;
          while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=0xa && sNewStr[n] != '(') {n++;}
          while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
          if (sNewStr[n] == '(')
          {
            int m = 0;
            n++;
            while (true)
            {
              if (sNewStr[n]==')')
              {
                if (!m)
                  break;
                m--;
              }
              if (sNewStr[n]=='(')
                m++;
              n++;
            }
            sStr[0] = 0;
            if (m_Insts[m_CurInst].m_Mask & tcMMask)
              sprintf(sStr,  ", uniform float4x4 %s\n", sMTCM);
            if (m_Insts[m_CurInst].m_Mask & tcGOLMask)
            {
              sprintf(sStr1,  ", uniform float4x4 %s\n", sMTCG);
              strcat(sStr, sStr1);
            }
            if (m_Insts[m_CurInst].m_Mask & (tcGRMMask | tcGSMMask))
            {
              if (!bCamPos)
              {
                sprintf(sStr1,  ", uniform float4 CameraPos\n");
                strcat(sStr, sStr1);
              }
              if (!bModV)
              {
                sprintf(sStr1,  ", uniform float4x4 ModelView\n");
                strcat(sStr, sStr1);
              }
              if (!bModVIT)
              {
                sprintf(sStr1,  ", uniform float4x4 ModelViewIT\n");
                strcat(sStr, sStr1);
              }
            }
            if (m_Insts[m_CurInst].m_Mask & tcGNMMask)
            {
              if (!bModVIT)
              {
                sprintf(sStr1,  ", uniform float4x4 ModelViewIT\n");
                strcat(sStr, sStr1);
              }
            }
            len = strlen(sStr);
            size_t len2 = strlen(Script)+1;
            char *newScr2 = new char[len+len2];
            n = sNewStr-Script+n;
            strncpy(newScr2, Script, n);
            strcpy(&newScr2[n], sStr);
            strcpy(&newScr2[n+len], &Script[n]);
            delete [] Script;
            Script = newScr2;
          }
        }
      }

      if (m_Insts[m_CurInst].m_Mask & (tcGRMMask | tcGNMMask | tcGSMMask))
      {
        if (!strstr(Script, "BLENDWEIGHT"))
        {
          while (true)
          {
            sNewStr = strstr(Script, "appin");
            if (!sNewStr)
              break;
            n = 0;
            while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=0xa && sNewStr[n] != '{') {n++;}
            while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
            if (sNewStr[n] == '{')
            {
              n++;
              while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
              len = strlen("float3 TNormal : BLENDWEIGHT;\n");
              size_t len2 = strlen(Script)+1;
              char *newScr2 = new char[len+len2];
              n = sNewStr-Script+n;
              strncpy(newScr2, Script, n);
              strcpy(&newScr2[n], "float3 TNormal : BLENDWEIGHT;\n");
              strcpy(&newScr2[n+len], &Script[n]);
              delete [] Script;
              Script = newScr2;
              break;
            }
          }
        }
      }
    }
  }
  return Script;
}


char *CCGVProgram_D3D::mfCreateAdditionalVP(CVProgram *pPosVP)
{
  SCGScript *pScript = m_Script;
  if (!pScript)
    pScript = mfGenerateScriptVP(pPosVP);
  if (!pScript)
    return NULL;
  const char *scr = pScript->m_Script;
  if (m_Flags & VPFI_FX)
    return (char *)scr;
  size_t len;
	int n;

  char *sNewStr = pScript->m_Script;
  int Mask = m_Insts[m_CurInst].m_Mask;
  if (Mask & VPVST_CLIPPLANES3)
  {
    char *s = strstr(sNewStr, "vertout");
    if (s && strstr(s, "TEXCOORD3"))
    {
      //m_Flags |= VPFI_NOFAKECLIPPLANE;
      Mask &= ~VPVST_CLIPPLANES3;
    }
  }

  sNewStr = pScript->m_Script;
  while (true)
  {
    sNewStr = strstr(sNewStr, "return");
    if (!sNewStr)
    {
      len = strlen(pScript->m_Script)+1;
      sNewStr = new char[len];
      strcpy(sNewStr, pScript->m_Script);
      return sNewStr;
    }
    n = 0;
    while (sNewStr[n] && sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=9 && sNewStr[n]!=0xa) {n++;}
    while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==9 || sNewStr[n]==0xa) {n++;}
    if (!strncmp(&sNewStr[n], "OUT", 3))
      break;
    sNewStr += n;
  }

  n = sNewStr - pScript->m_Script;
  len = strlen(pScript->m_Script)+1;
  int nScr = 0;
  int nInst = 0;
  if (gRenDev->m_Features & RFT_FOGVP)
    nScr = 1;
  if (Mask & (VPVST_INSTANCING_ROT | VPVST_INSTANCING_NOROT))
    nInst = 1;
  int lenN = strlen(sAdditionalVP[nInst][nScr][Mask&7]);
  int newSize = n+1+lenN;
  char *newScr = new char [newSize];
  strncpy(newScr, scr, n);
  strcpy(&newScr[n], sAdditionalVP[nInst][nScr][Mask&7]);

  if ((Mask & VPVST_NOISE) && !strstr(newScr, "register(c30)"))
  {
    sNewStr = newScr;
    while (true)
    {
      sNewStr = strstr(sNewStr, "main");
      if (!sNewStr)
        break;
      n = 0;
      while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=9 && sNewStr[n]!=0xa && sNewStr[n] != '(') {n++;}
      while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==9 || sNewStr[n]==0xa) {n++;}
      if (sNewStr[n] == '(')
      {
        int m = 0;
        n++;
        while (true)
        {
          if (sNewStr[n]==')')
          {
            if (!m)
              break;
            m--;
          }
          if (sNewStr[n]=='(')
            m++;
          n++;
        }
        len = strlen(", uniform float4 pg[66] : register(c30)");
        size_t len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], ", uniform float4 pg[66] : register(c30)");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
        break;
      }
    }
  }

  if (((Mask & (VPVST_FOGGLOBAL | VPVST_HDR)) || m_CGProfileType == CG_PROFILE_VS_3_0) && !(m_Flags & VPFI_NOFOG))
  {
    sNewStr = newScr;
    while (true)
    {
      sNewStr = strstr(sNewStr, "main");
      if (!sNewStr)
        break;
      n = 0;
      while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=9 && sNewStr[n]!=0xa && sNewStr[n] != '(') {n++;}
      while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==9 || sNewStr[n]==0xa) {n++;}
      if (sNewStr[n] == '(')
      {
        int m = 0;
        n++;
        while (true)
        {
          if (sNewStr[n]==')')
          {
            if (!m)
              break;
            m--;
          }
          if (sNewStr[n]=='(')
            m++;
          n++;
        }
        len = strlen(", uniform float3 Fog : register(c29)");
        int len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], ", uniform float3 Fog : register(c29)");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
        break;
      }
    }
  }

  if (gRenDev->m_Features & RFT_FOGVP)
  {
    sNewStr = newScr;
    while (true)
    {
      sNewStr = strstr(sNewStr, "vertout");
      if (!sNewStr)
        break;
      n = 0;
      while (sNewStr[n]!=0x20 && sNewStr[n]!=9 && sNewStr[n]!=8 && sNewStr[n]!=0xa && sNewStr[n] != '{') {n++;}
      while (sNewStr[n]==0x20 || sNewStr[n]==9 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
      if (sNewStr[n] == '{')
      {
        n++;
        while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==9 || sNewStr[n]==0xa) {n++;}
        len = strlen("float FogC : FOG;\n");
        int len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], "float FogC : FOG;\n");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
      }
      break;
    }
  }
  if (Mask & VPVST_CLIPPLANES3)
  {
    sNewStr = newScr;
    while (true)
    {
      sNewStr = strstr(sNewStr, "main");
      if (!sNewStr)
        break;
      n = 0;
      while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=9 && sNewStr[n]!=0xa && sNewStr[n] != '(') {n++;}
      while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==9 || sNewStr[n]==0xa) {n++;}
      if (sNewStr[n] == '(')
      {
        int m = 0;
        n++;
        while (true)
        {
          if (sNewStr[n]==')')
          {
            if (!m)
              break;
            m--;
          }
          if (sNewStr[n]=='(')
            m++;
          n++;
        }
        len = strlen(", uniform float4 ClipPlane : register(c14)");
        int len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], ", uniform float4 ClipPlane : register(c14)");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
      }
      break;
    }
    sNewStr = newScr;
    if (!strstr(sNewStr, "float4 Tex3"))
    {
      while (true)
      {
        sNewStr = strstr(sNewStr, "vertout");
        if (!sNewStr)
          break;
        n = 0;
        while (sNewStr[n] != '{' && sNewStr[n]!=0) {n++;}
        if (sNewStr[n] == '{')
        {
          n++;
          while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==9 || sNewStr[n]==0xa) {n++;}
          len = strlen("float4 Tex3 : TEXCOORD3;\n");
          int len2 = strlen(newScr)+1;
          char *newScr2 = new char[len+len2];
          n = sNewStr-newScr+n;
          strncpy(newScr2, newScr, n);
          strcpy(&newScr2[n], "float4 Tex3 : TEXCOORD3;\n");
          strcpy(&newScr2[n+len], &newScr[n]);
          delete [] newScr;
          newScr = newScr2;
        }
        break;
      }
    }
  }

  sNewStr = newScr;
  while (true)
  {
    sNewStr = strstr(sNewStr, "main");
    if (!sNewStr)
      break;
    n = 0;
    while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=9 && sNewStr[n]!=0xa && sNewStr[n] != '(') {n++;}
    while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==9 || sNewStr[n]==0xa) {n++;}
    if (sNewStr[n] == '(')
    {
      int m = 0;
      n++;
      while (true)
      {
        if (sNewStr[n]==')')
        {
          if (!m)
            break;
          m--;
        }
        if (sNewStr[n]=='(')
          m++;
        n++;
      }
      len = strlen(", uniform float4 g_VSCONST_0_025_05_1 : register(c28)");
      int len2 = strlen(newScr)+1;
      char *newScr2 = new char[len+len2];
      n = sNewStr-newScr+n;
      strncpy(newScr2, newScr, n);
      strcpy(&newScr2[n], ", uniform float4 g_VSCONST_0_025_05_1 : register(c28)");
      strcpy(&newScr2[n+len], &newScr[n]);
      delete [] newScr;
      newScr = newScr2;
      break;
    }
  }

  if (Mask & VPVST_TCMASK)
  {
    for (int i=0; i<4; i++)
    {
      newScr = mfGenerateTCScript(newScr, i);
    }
  }

  if (pScript != m_Script)
    pScript->Release();

  return newScr;
}

void CCGVProgram_D3D::mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params)
{
  gRenDev->m_cEF.mfCompileCGParam(scr, ef, Params);
}

void CCGVProgram_D3D::mfCompileParamStateMatrix(char *scr, SShader *ef, TArray<SCGMatrix> *Params)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eType=1, eName};
  static tokenDesc commands[] =
  {
    {eType, "Type"},
    {eName, "Name"},

    {0,0}
  };

  SCGMatrix pr;
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
        pr.m_Name = data;
        break;

      case eType:
        if (!stricmp(data, "ViewProj"))
          pr.m_eCGParamType = ECGP_Matr_ViewProj;
        else
        if (!stricmp(data, "View"))
          pr.m_eCGParamType = ECGP_Matr_View;
        else
        if (!stricmp(data, "View_I"))
          pr.m_eCGParamType = ECGP_Matr_View_I;
        else
        if (!stricmp(data, "View_T"))
          pr.m_eCGParamType = ECGP_Matr_View_T;
        else
        if (!stricmp(data, "View_IT"))
          pr.m_eCGParamType = ECGP_Matr_View_IT;
        else
          pr.m_eCGParamType = ECGP_Unknown;
    }
  }
  if (pr.m_eCGParamType != ECGP_Unknown && pr.m_Name.GetIndex())
  {
    Params->AddElem(pr);
  }
}

void CCGVProgram_D3D::mfCompileVertAttributes(char *scr, SShader *ef)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eVertArray=1};
  static tokenDesc commands[] =
  {
    {eVertArray, "VertArray"},

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
      case eVertArray:
        {
          if (!data || !data[0])
          {
            Warning( 0,0,"Missing parameters for VertArray in Shader '%s'\n", ef->m_Name.c_str());
            break;
          }
          gRenDev->m_cEF.mfCompileArrayPointer(m_Pointers, params, ef);
        }
        break;
    }
  }
}

bool CCGVProgram_D3D::mfCompile(char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eScript=1, eParam4f, eVertAttributes, eParamStateMatrix, eDefaultPos, eNoFog, eVS20Only, eVS30Only, eAutoEnumTC, eProjected, eNoise, eUnifiedPosScript, eMainInput, eDeclarationsScript, ePositionScript, eCoreScript, eSubrScript, eSupportsInstancing, eSupportsMultiLights, eInst_Param4f, eNoSpecular};
  static tokenDesc commands[] =
  {
    {eScript, "Script"},
    {eAutoEnumTC, "AutoEnumTC"},
    {eParam4f, "Param4f"},
    {eParamStateMatrix, "ParamStateMatrix"},
    {eVertAttributes, "VertAttributes"},
    {eMainInput, "MainInput"},
    {eDeclarationsScript, "DeclarationsScript"},
    {ePositionScript, "PositionScript"},
    {eCoreScript, "CoreScript"},
    {eSubrScript, "SubrScript"},
    {eNoFog, "NoFog"},
    {eVS20Only, "VS20Only"},
    {eVS30Only, "VS30Only"},
    {eProjected, "Projected"},
    {eNoise, "Noise"},
    {eUnifiedPosScript, "UnifiedPosScript"},
    {eDefaultPos, "DefaultPos"},
    {eNoSpecular, "NoSpecular"},

    {eSupportsInstancing, "SupportsInstancing"},
    {eSupportsMultiLights, "SupportsMultiLights"},
    {eInst_Param4f, "Inst_Param4f"},

    {0,0}
  };

  mfFree();

  SShader *ef = gRenDev->m_cEF.m_CurShader;
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
      case eVertAttributes:
        mfCompileVertAttributes(params, ef);
        break;

      case eMainInput:
        m_InputParmsScript = mfAddNewScript(name, params);
        break;

      case eDeclarationsScript:
        m_DeclarationsScript = mfAddNewScript(name, params);
        break;

      case ePositionScript:
        if (strchr(params, ';'))
          m_PosScript = mfAddNewScript(name, params);
        else
        {
          m_PosScript = mfAddNewScript(params, NULL);
          if (!m_PosScript)
            m_PosScript = mfAddNewScript("PosCommon", NULL);
        }
        break;

      case eCoreScript:
        m_CoreScript = mfAddNewScript(name, params);
        break;

      case eSubrScript:
        m_SubroutinesScript = mfAddNewScript(name, params);
        break;

      case eParam4f:
        mfCompileParam4f(params, ef, &m_ParamsNoObj);
        break;

      case eInst_Param4f:
        mfCompileParam4f(params, ef, &m_Params_Inst);
        break;

      case eParamStateMatrix:
        mfCompileParamStateMatrix(params, ef, &m_MatrixObj);
        break;

      case eUnifiedPosScript:
        m_Flags |= VPFI_UNIFIEDPOS;
        break;

      case eNoFog:
        m_Flags |= VPFI_NOFOG;
        break;

      case eNoSpecular:
        m_Flags |= VPFI_NOSPECULAR;
        break;

      case eSupportsInstancing:
        m_Flags |= VPFI_SUPPORTS_INSTANCING;
        break;

      case eSupportsMultiLights:
        m_Flags |= VPFI_SUPPORTS_MULTILIGHTS;
        break;

      case eProjected:
        if (gRenDev->GetFeatures() & RFT_HW_ENVBUMPPROJECTED)
          m_Flags |= VPFI_PROJECTED;
        break;

      case eVS20Only:
        m_Flags |= VPFI_VS20ONLY;
				break;

      case eVS30Only:
        m_Flags |= VPFI_VS30ONLY;
        break;

      case eAutoEnumTC:
        m_Flags |= VPFI_AUTOENUMTC;
        break;

      case eNoise:
        m_Flags |= VPFI_NOISE;
        break;

      case eDefaultPos:
        m_Flags |= VPFI_DEFAULTPOS;
        break;

      case eScript:
        {
          SAFE_RELEASE(m_Script);
          m_Script = mfAddNewScript(NULL, data);
        }
        break;
    }
  }

  int i;
  for (i=0; i<m_MatrixObj.Num(); i++)
  {
    SCGMatrix *m = &m_MatrixObj[i];
    if (m->m_eCGParamType == ECGP_Matr_ViewProj)
      break;
  }
  if (i == m_MatrixObj.Num())
  {
    SCGMatrix m;
    m.m_eCGParamType = ECGP_Matr_ViewProj;
    m.m_Name = "ModelViewProj";
    m_MatrixObj.AddElemNoCache(m);
  }

  gRenDev->m_cEF.mfCheckObjectDependParams(&m_ParamsNoObj, &m_ParamsObj);

  return 1;
}

/* returns a random floating point number between 0.0 and 1.0 */
static float frand()
{
    return (float) (rand() / (float) RAND_MAX);
}

/* returns a random floating point number between -1.0 and 1.0 */
static float sfrand()
{
    return (float) (rand() * 2.0f/ (float) RAND_MAX) - 1.0f;
}

void CCGVProgram_D3D::mfGetSrcFileName(char *srcname, int nSize)
{
  strncpy(srcname, gRenDev->m_cEF.m_HWPath, nSize);
  strncat(srcname, "Declarations/CGVShaders/", nSize);
  strncat(srcname, m_Name.c_str(), nSize);
  strncat(srcname, ".crycg", nSize);
}

void CCGVProgram_D3D::mfGetDstFileName(char *dstname, int nSize, bool bUseASCIICache)
{
  char *type;

  if (m_Flags & VPFI_AUTOENUMTC)
    type = "D3D9_Auto";
  else
  if (m_CGProfileType == CG_PROFILE_VS_1_1)
    type = "D3D9_VS11";
  else
  if (m_CGProfileType == CG_PROFILE_VS_2_0)
    type = "D3D9_VS20";
  else
  if (m_CGProfileType == CG_PROFILE_VS_2_X)
    type = "D3D9_VS2X";
  else
  if (m_CGProfileType == CG_PROFILE_VS_3_0)
    type = "D3D9_VS30";
  else
    type = "Unknown";

  char *fog = "";
  if ((m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL) || m_CGProfileType == CG_PROFILE_VS_3_0)
    fog = "Fog";
  else
  if (m_Insts[m_CurInst].m_Mask & VPVST_NOFOG)
  {
    if (gRenDev->m_Features & RFT_FOGVP)
      fog = "Fog0";
    else
      fog = "NoFog";
  }
  
  char *cp;
  if (m_Insts[m_CurInst].m_Mask & VPVST_CLIPPLANES3)
    cp = "CP";
  else
    cp = "NoCP";

  char *pr = "";
  if (m_Flags & VPFI_PROJECTED)
    pr = "Proj";

  strncpy(dstname, gRenDev->m_cEF.m_ShadersCache, nSize);
  strncat(dstname, "CGVShaders/", nSize);
  strncat(dstname, m_Name.c_str(), nSize);
  strncat(dstname, "$", nSize);
  strncat(dstname, type, nSize);
  strncat(dstname, "$", nSize);
  strncat(dstname, fog, nSize);
  strncat(dstname, "$", nSize);
  strncat(dstname, cp, nSize);
  if (m_Insts[m_CurInst].m_Mask & VPVST_INSTANCING_NOROT)
    strncat(dstname, "$INST_NR", nSize);
  else
  if (m_Insts[m_CurInst].m_Mask & VPVST_INSTANCING_ROT)
    strncat(dstname, "$INST_R", nSize);
  if (m_Insts[m_CurInst].m_Mask & VPVST_HDR)
    strncat(dstname, "$HDR", nSize);
  if (pr[0])
  {
    strncat(dstname, "$", nSize);
    strncat(dstname, pr, nSize);
  }
  strncat(dstname, "$", nSize);
  strncat(dstname, m_Insts[m_CurInst].m_PosScriptName.c_str(), nSize);

  for (int i=0; i<4; i++)
  {
    char str[64];
    if (m_Insts[m_CurInst].m_Mask & (VPVST_TCM0<<i))
    {
      sprintf(str, "$TCM%d", i);
      strncat(dstname, str, nSize);
    }
    if (m_Insts[m_CurInst].m_Mask & (VPVST_TCGOL0<<i))
    {
      sprintf(str, "$TCGOL%d", i);
      strncat(dstname, str, nSize);
    }
    if (m_Insts[m_CurInst].m_Mask & (VPVST_TCGRM0<<i))
    {
      sprintf(str, "$TCGRM%d", i);
      strncat(dstname, str, nSize);
    }
    if (m_Insts[m_CurInst].m_Mask & (VPVST_TCGNM0<<i))
    {
      sprintf(str, "$TCGNM%d", i);
      strncat(dstname, str, nSize);
    }
    if (m_Insts[m_CurInst].m_Mask & (VPVST_TCGSM0<<i))
    {
      sprintf(str, "$TCGSM%d", i);
      strncat(dstname, str, nSize);
    }
  }
  if (bUseASCIICache && m_Insts[m_CurInst].m_LightMask)
  {
    char str[32];
    sprintf(str, "$%x", m_Insts[m_CurInst].m_LightMask);
    strncat(dstname, str, nSize);
  }
  if (m_nMaskGen)
  {
    char str[32];
    sprintf(str, "(%I64x)", m_nMaskGen);
    strncat(dstname, str, nSize);
  }

  strncat(dstname, ".cgvp", nSize);
}

static char *sGetText(char **buf)
{
  bool bBetween = false;
  SkipCharacters(buf, " ");
  if (**buf == ':')
  {
    ++*buf;
    SkipCharacters(buf, " ");
    if (**buf == ':')
      return NULL;
  }
  char *result = *buf;

  // now, we need to find the next whitespace to end the data
  char theChar;

  while ((theChar = **buf) != 0)
  {
    if (theChar <= 0x20)        // space and control chars
      break;
    ++*buf;
  }
  **buf = 0;                    // null terminate it
  if (theChar)                  // skip the terminator
    ++*buf;
  return result;
}

_inline bool sIncrTypes(int *Types, int nInd, bool bSpec)
{
  Types[nInd]++;
  if ((Types[nInd] & 3) == 3)
  {
    if (!bSpec)
      return true;
    if ((Types[nInd] & SLMF_ONLYSPEC))
    {
      if (Types[nInd] & SLMF_SPECOCCLUSION)
      {
        Types[nInd] = 0;
        return true;
      }
      Types[nInd] |= SLMF_SPECOCCLUSION;
    }
    else
      Types[nInd] |= SLMF_ONLYSPEC;
    Types[nInd] &= ~3;
  }
  return false;
}

void CCGVProgram_D3D::mfPrecacheLights(int nMask)
{
  int i, j;
  int nLightMask;

  m_Flags |= VPFI_PRECACHEPHASE;
  int nInst = m_Insts.Num();
  SCGInstance cgi;
  m_Insts.AddElem(cgi);
  SCGInstance *pi = &m_Insts[nInst];
  int nTempInst = m_CurInst;
  m_CurInst = nInst;
  int nShSave = CRenderer::CV_r_shaderssave;
  CRenderer::CV_r_shaderssave = 0;

  CVProgram *pPosVP = this;
  bool bSpec = (m_Flags & VPFI_NOSPECULAR) == 0;

  pi->m_Mask = nMask;
  pi->m_LightMask = 0;
  pi->m_PosScriptName = m_PosScript ? m_PosScript->m_Name : "None";
  m_Flags &= ~VPFI_WASGENERATED;
  mfActivate(pPosVP);
  SAFE_DELETE(pi->m_BindVars);
  int nMaxLights = (gRenDev->m_bDeviceSupports_PS2X) ? NUM_PPLIGHTS_PERPASS_PS2X : NUM_PPLIGHTS_PERPASS_PS30;
  if (m_Flags & VPFI_WASGENERATED)
  {
    TArray<int> Masks;
    int Types[5];

    int nCombinations = 0;
    iLog->Log("Precache light shader %s(0x%I64x)\n", m_Name.c_str(), m_nMaskGen);
    m_nFailed = 0;

    // Light nums
    for (i=1; i<=nMaxLights; i++)
    {
      for (j=0; j<i; j++)
      {
        Types[j] = 0;
      }
      Types[i] = 0;
      while (!Types[i])
      {
        int nProjs = 0;
        int nDirs = 0;
        nLightMask = i;
        int sTypes[4];
        for (int n=0; n<i; n++)
        {
          sTypes[n] = Types[n];
        }
        SortLightTypes(sTypes, i);
        for (j=0; j<i; j++)
        {
          if ((sTypes[j]&3) == SLMF_PROJECTED)
            nProjs++;
          else
          if ((sTypes[j]&3) == SLMF_DIRECT)
            nDirs++;
          nLightMask |= sTypes[j] << (SLMF_LTYPE_SHIFT + j*4);
        }
        // Usually we have not more then one directional light source
        // and one projected light source
        if (nProjs <= 1 && nDirs <= 1)
        {
          int n;
          for (n=0; n<Masks.Num(); n++)
          {
            if (Masks[n] == nLightMask)
              break;
          }
          if (n == Masks.Num())
          {
            Masks.AddElem(nLightMask);
            pi->m_LightMask = nLightMask;
            mfActivate(pPosVP);
            SAFE_DELETE(pi->m_BindVars);
            nCombinations++;
          }
        }

        if (sIncrTypes(Types, 0, bSpec))
        {
          if (sIncrTypes(Types, 1, bSpec))
          {
            if (sIncrTypes(Types, 2, bSpec))
            {
              if (sIncrTypes(Types, 3, bSpec))
              {
                sIncrTypes(Types, 4, bSpec);
              }
            }
          }
        }
      }
    }
    if (pi->m_nCacheID >= 0)
      gRenDev->m_cEF.FlushCacheFile(m_InstCache[pi->m_nCacheID].m_pCache);

    iLog->Log("Precaching finished (%d combinations total, %d combinations failed)\n", nCombinations, m_nFailed);
  }

  m_Insts.DelElem(nInst);
  m_CurInst = nTempInst;
  m_Flags &= ~VPFI_PRECACHEPHASE;
  CRenderer::CV_r_shaderssave = nShSave;
}

void CCGVProgram_D3D::mfPrecache()
{
  bool bPrevFog = gRenDev->m_FS.m_bEnable;
  gRenDev->m_FS.m_bEnable = true;

  int Mask;
  if ((m_Flags & VPFI_NOFOG) || !CRenderer::CV_r_vpfog || !(gRenDev->m_Features & RFT_FOGVP))
    Mask = VPVST_NOFOG;
  else
    Mask = VPVST_FOGGLOBAL;

  int nLightMask = 0;
  if (m_Flags & VPFI_SUPPORTS_MULTILIGHTS)
  {
    if (CRenderer::CV_r_shadersprecache < 2)
    {
      mfPrecacheLights(Mask);
      if (gRenDev->m_Features & RFT_HW_HDR)
      {
        Mask |= VPVST_HDR;
        mfPrecacheLights(Mask);
      }
    }
    gRenDev->m_FS.m_bEnable = bPrevFog;
    return;
  }

  CVProgram *pPosVP = this;
  int Id = mfGetCGInstanceID(Mask, pPosVP, nLightMask);
  mfActivate(pPosVP);

  if (gRenDev->m_Features & RFT_HW_HDR)
  {
    Mask |= VPVST_HDR;
    Id = mfGetCGInstanceID(Mask, pPosVP, nLightMask);
    mfActivate(pPosVP);
  }
  if ((gRenDev->m_Features & RFT_HW_PS30) && (m_Flags & VPFI_SUPPORTS_INSTANCING))
  {
    Mask |= VPVST_INSTANCING_NOROT;
    Id = mfGetCGInstanceID(Mask, pPosVP, nLightMask);
    mfActivate(pPosVP);

    if (Mask & VPVST_HDR)
    {
      Mask &= ~VPVST_HDR;
      Id = mfGetCGInstanceID(Mask, pPosVP, nLightMask);
      mfActivate(pPosVP);
    }

    Mask &= ~(VPVST_INSTANCING_NOROT | VPVST_HDR);
  }

  if (gcpRendD3D->m_d3dCaps.MaxUserClipPlanes == 0)
  {
    Mask |= VPVST_CLIPPLANES3;
    Id = mfGetCGInstanceID(Mask, pPosVP, nLightMask);
    mfActivate(pPosVP);
  }
  gRenDev->m_FS.m_bEnable = bPrevFog;
}

bool CCGVProgram_D3D::ActivateCacheItem(SShaderCacheHeaderItem *pItem)
{
  int i;
  byte *pData = (byte *)pItem;
  pData += sizeof(SShaderCacheHeaderItem);
  SShaderCacheHeaderItemVar *pVars = (SShaderCacheHeaderItemVar *)pData;
  for (i=0; i<pItem->m_nVariables; i++)
  {
    if (!m_Insts[m_CurInst].m_BindVars)
      m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
    SCGBind cgp;
    cgp.m_nComponents = pVars[i].m_nCount;
    cgp.m_Name = pVars[i].m_Name;
    cgp.m_dwBind = pVars[i].m_Reg;
    m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
  }
  pData += pItem->m_nVariables*sizeof(SShaderCacheHeaderItemVar);
  HRESULT hr = gcpRendD3D->mfGetD3DDevice()->CreateVertexShader((DWORD*)pData, (IDirect3DVertexShader9 **)&m_Insts[m_CurInst].m_pHandle);
  if (FAILED(hr))
    return false;
  return true;
}
bool CCGVProgram_D3D::CreateCacheItem(int nMask, byte *pData, int nLen)
{
  int i;
  int nVars = m_Insts[m_CurInst].m_BindVars ? m_Insts[m_CurInst].m_BindVars->Num() : 0;
  int nNewSize = nVars*sizeof(SShaderCacheHeaderItemVar)+nLen;
  byte *pNewData = new byte [nNewSize];
  SShaderCacheHeaderItemVar *pVars = (SShaderCacheHeaderItemVar *)pNewData;
  for (i=0; i<nVars; i++)
  {
    SCGBind *bnd = &m_Insts[m_CurInst].m_BindVars->Get(i);
    memset(pVars[i].m_Name, 0, sizeof(pVars[i].m_Name));
    strcpy(pVars[i].m_Name, bnd->m_Name.c_str());
    pVars[i].m_nCount = bnd->m_nComponents;
    pVars[i].m_Reg = bnd->m_dwBind;
  }
  memcpy(&pNewData[nNewSize-nLen], pData, nLen);
  SShaderCacheHeaderItem h;
  h.m_nMask = nMask;
  h.m_nVariables = nVars;
  bool bRes = gRenDev->m_cEF.AddCacheItem(m_InstCache[m_Insts[m_CurInst].m_nCacheID].m_pCache, &h, pNewData, nNewSize, false);
  delete [] pNewData;
  if (!(m_Flags & VPFI_PRECACHEPHASE))
    gRenDev->m_cEF.FlushCacheFile(m_InstCache[m_Insts[m_CurInst].m_nCacheID].m_pCache);

  return bRes;
}

struct STempStr
{
  char name[128];
  int nComponents;
  int nId;
};

// Compile vertex shader for the current instance properties
bool CCGVProgram_D3D::mfActivate(CVProgram *pPosVP)
{
  PROFILE_FRAME(Shader_VShaderActivate);

  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  if (!m_Insts[m_CurInst].m_CGProgram)
  {
    int VS20Profile = CG_PROFILE_VS_2_0;
    if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX || (gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_NV4X)
      VS20Profile = CG_PROFILE_VS_2_X;
    if (m_Flags & VPFI_VS30ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS30)
        m_CGProfileType = CG_PROFILE_VS_3_0;
      else
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
        m_CGProfileType = VS20Profile;
      else
        m_CGProfileType = CG_PROFILE_VS_1_1;
    }
    else
    if (m_Flags & VPFI_VS20ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
        m_CGProfileType = VS20Profile;
      else
        m_CGProfileType = CG_PROFILE_VS_1_1;
    }
    else
      m_CGProfileType = CG_PROFILE_VS_1_1;

    if (m_Insts[m_CurInst].m_Mask & (VPVST_INSTANCING_NOROT | VPVST_INSTANCING_ROT))
    {
      if (m_CGProfileType < CG_PROFILE_VS_2_0)
        m_CGProfileType = VS20Profile;
    }

    bool bCreate = false;
    char namesrc[256];
    char namedst[256];
    char namedst1[256];
    char strVer[128];
    char strVer0[128];
    sprintf(strVer, "//CGVER%.1f\n", CG_VP_CACHE_VER);
    FILETIME writetimesrc,writetimedst;
    FILE *statussrc, *statusdst;
    statussrc = NULL;
    statusdst = NULL;
    mfGetSrcFileName(namesrc, 256);
    CCGVProgram_D3D *pVP = (CCGVProgram_D3D *)pPosVP;
    
    bool bUseACIICache = true;
    if (CRenderer::CV_r_shadersprecache < 2 && (m_Flags & VPFI_SUPPORTS_MULTILIGHTS))
      bUseACIICache = false;
    mfGetDstFileName(namedst, 256, bUseACIICache);
    StripExtension(namedst, namedst1);

    // Use binary cache files for PS30 shaders
    if (!bUseACIICache)
    {
      if (m_Insts[m_CurInst].m_nCacheID == -1)
      {
        AddExtension(namedst1, ".cgbin");
        m_Insts[m_CurInst].m_nCacheID = mfGetCacheInstanceID(m_Insts[m_CurInst].m_Mask, namedst1);
      }
      SShaderCacheHeaderItem *pCacheItem = gRenDev->m_cEF.GetCacheItem(m_InstCache[m_Insts[m_CurInst].m_nCacheID].m_pCache, m_Insts[m_CurInst].m_LightMask);
      if (pCacheItem)
      {
        if (m_Flags & VPFI_PRECACHEPHASE)
        {
          gRenDev->m_cEF.FreeCacheItem(m_InstCache[m_Insts[m_CurInst].m_nCacheID].m_pCache, m_Insts[m_CurInst].m_LightMask);
          return true;
        }
        bool bRes = ActivateCacheItem(pCacheItem);
        gRenDev->m_cEF.FreeCacheItem(m_InstCache[m_Insts[m_CurInst].m_nCacheID].m_pCache, m_Insts[m_CurInst].m_LightMask);
        if (bRes)
          return true;
        pCacheItem = NULL;
      }
      bCreate = true;
    }
    else
    {
      AddExtension(namedst1, ".cgasm");
      statusdst = iSystem->GetIPak()->FOpen(namedst1, "r");
    }
    if (statusdst == NULL && bUseACIICache)
    {
      if (CRenderer::CV_r_shadersprecache == 3)
        bCreate = true;
      else
      {
        statusdst = iSystem->GetIPak()->FOpen(namedst, "r");
        if (statusdst == NULL)
          bCreate = true;
        else
        if (!m_Functions.size())
        {
          statussrc = iSystem->GetIPak()->FOpen(namesrc, "r");
          writetimesrc = iSystem->GetIPak()->GetModificationTime(statussrc);
          writetimedst = iSystem->GetIPak()->GetModificationTime(statusdst);;
          if (CompareFileTime(&writetimesrc,&writetimedst)!=0)
            bCreate = true;
          iSystem->GetIPak()->FGets(strVer0, 128, statusdst);
          if (strcmp(strVer, strVer0))
            bCreate = true;
          iSystem->GetIPak()->FClose(statussrc);
        }
      }
    }
    else
      strcpy(namedst, namedst1);
create:
    char *pbuf = NULL;
    if (bCreate)
    {
      if (statusdst)
      {
        iSystem->GetIPak()->FClose(statusdst);
        statusdst = NULL;
      }

      m_Flags |= VPFI_WASGENERATED;
      char *scr = mfCreateAdditionalVP(pPosVP);

      char *code = mfLoadCG(scr);
      delete [] scr;
      pbuf = code;

      // Create asm cache file
      if (bUseACIICache)
      {
        if (code)
        {
          char *str = new char [strlen(code)+1];
          strcpy(str, code);
          char *s = strstr(str, "oFog.x");
          if (s)
            memcpy(s, "oFog  ", 6);
          FILE *fp = fopen(namedst, "wb");
          if (fp)
          {
            fputs(strVer, fp);
            fputs(code, fp);
            fclose(fp);
          }
          if (!m_Functions.size())
          {
            HANDLE hdst = CreateFile(namedst,GENERIC_WRITE,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            FILE *hsrc = iSystem->GetIPak()->FOpen(namesrc, "r");
            writetimesrc = iSystem->GetIPak()->GetModificationTime(hsrc);
            SetFileTime(hdst,NULL,NULL,&writetimesrc);
            iSystem->GetIPak()->FClose(hsrc);
            CloseHandle(hdst);
          }
        }
#ifndef WIN64
			  // NOTE: AMD64 port: find the 64-bit CG runtime
        if (m_Insts[m_CurInst].m_CGProgram)
          cgDestroyProgram(m_Insts[m_CurInst].m_CGProgram);
#endif
        m_Insts[m_CurInst].m_CGProgram = NULL;
        statusdst = iSystem->GetIPak()->FOpen(namedst, "r");
      }
    }
    if (statusdst)
    {
      iSystem->GetIPak()->FSeek(statusdst, 0, SEEK_END);
      int len = iSystem->GetIPak()->FTell(statusdst);
      iSystem->GetIPak()->FSeek(statusdst, 0, SEEK_SET);
      pbuf = new char [len+1];
      iSystem->GetIPak()->FGets(strVer0, 128, statusdst);
      len = iSystem->GetIPak()->FRead(pbuf, 1, len, statusdst);
      pbuf[len] = 0;
      iSystem->GetIPak()->FClose(statusdst);
      statusdst = NULL;

      // Regenerate shader if hardware was changed
      if (!bCreate && (m_Flags & VPFI_AUTOENUMTC))
      {
        if (strstr(pbuf, "vs_2_x"))
        {
          if (D3DSHADER_VERSION_MAJOR(gcpRendD3D->m_d3dCaps.VertexShaderVersion) < 2)
          {
            SAFE_DELETE_ARRAY(pbuf);
            bCreate = true;
            goto create;
          }
          else
          {
            int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;
            if (nGPU == RFT_HW_RADEON || nGPU == RFT_HW_NV4X)
            {
              SAFE_DELETE_ARRAY(pbuf);
              m_CGProfileType = CG_PROFILE_VS_2_0;
              bCreate = true;
              goto create;
            }
          }
          m_CGProfileType = CG_PROFILE_VS_2_X;
        }
        else
        if (strstr(pbuf, "vs_2_0"))
        {
          if (D3DSHADER_VERSION_MAJOR(gcpRendD3D->m_d3dCaps.VertexShaderVersion) < 2)
          {
            SAFE_DELETE_ARRAY(pbuf);
            bCreate = true;
            goto create;
          }
          else
          if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
          {
            SAFE_DELETE_ARRAY(pbuf);
            m_CGProfileType = CG_PROFILE_VS_2_X;
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_VS_2_0;
        }
        else
        if (strstr(pbuf, "vs_3_0"))
        {
          if (D3DSHADER_VERSION_MAJOR(gcpRendD3D->m_d3dCaps.VertexShaderVersion) < 3)
          {
            SAFE_DELETE_ARRAY(pbuf);
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_VS_3_0;
        }
        else
          m_CGProfileType = CG_PROFILE_VS_1_1;
      }
    }
    assert(!m_Insts[m_CurInst].m_BindVars);
    if (pbuf)
    {
      RemoveCR(pbuf);
      LPD3DXBUFFER pCode = mfLoad(pbuf);

      // parse variables and constants from CG or HLSL object code
      int nComps = 1;
      char *scr = pbuf;
      char *token = strtok(scr, "//");
      TArray<STempStr> FoundNames;
      int i;
      while (token)
      {
        while (token[0]==0x20) {token++;}
        if (token[0] == '$')
          token++;
        char *szName = sGetText(&token);
        char *szReg = sGetText(&token);
        if (szReg[0] == 'c' && isdigit(szReg[1]))
        {
          if (!m_Insts[m_CurInst].m_BindVars)
            m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
          SCGBind cgp;
          char *szSize = sGetText(&token);
          cgp.m_nComponents = atoi(szSize);
          cgp.m_Name = szName;
          cgp.m_dwBind = atoi(&szReg[1]);
          m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
        }
        token = strtok(NULL, "//");
      }
      for (i=0; i<FoundNames.Num(); i++)
      {
        if (FoundNames[i].nId >= 0)
          m_Insts[m_CurInst].m_BindVars->Get(FoundNames[i].nId).m_nComponents = FoundNames[i].nComponents;
      }

      assert(!m_Insts[m_CurInst].m_BindVars || m_Insts[m_CurInst].m_BindVars->Num()<=30);
      SAFE_DELETE_ARRAY(pbuf);
      if (pCode && !bUseACIICache)
        CreateCacheItem(m_Insts[m_CurInst].m_LightMask, (byte *)pCode->GetBufferPointer(), pCode->GetBufferSize());
      SAFE_RELEASE(pCode);
    }

    if (!(m_Flags & VPFI_PRECACHEPHASE))
      mfUnbind();

    if (m_CGProfileType = CG_PROFILE_VS_3_0)
    {
      if (bUseACIICache)
      {
        int nnn = 0;
      }
    }
  }

  if (!m_Insts[m_CurInst].m_CGProgram)
    return false;
  return true;
}

void CCGVProgram_D3D::mfSetGlobalParams()
{
  vec4_t v;
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  if (!(r->m_Features & RFT_HW_VS))
    return;

  if (m_nResetDeviceFrame != r->m_nFrameReset)
  {
    m_nResetDeviceFrame = r->m_nFrameReset;
    // create a perlin noise permuation table
    vec4_t c[66];
    int i;

    // want reproducable behaviour for debug
    //srand(1);

	  for(i=0; i<32; i++) 
	  {
		  c[i][3] = (float)i;
      Vec3d v;
		  v[0] = frand();
		  v[1] = frand();
		  v[2] = frand();
      v.Normalize();
      c[i][0] = v[0];
      c[i][1] = v[1];
      c[i][2] = v[2];
	  }

	  for(i=0; i<32; i++) 
	  {
		  // choose two entries at random and swap
		  int j = (rand() >> 4) & 31;		// lower bits of rand() are bogus sometimes
		  Exchange (c[j][3], c[i][3]);
      c[i+32][0] = c[i][0];
      c[i+32][1] = c[i][1];
      c[i+32][2] = c[i][2];
	  }

    for(i=0; i<64; i++) 
    {
      dv->SetVertexShaderConstantF(30+i, &c[i][0], 1);
    }
    dv->SetVertexShaderConstantF(30+i,   &c[0][0], 1);
    dv->SetVertexShaderConstantF(30+i+1, &c[1][0], 1);
  }

  SParamComp_Fog p;
  p.m_Type = 2;
  v[0] = p.mfGet();
  p.m_Type = 3;
  v[1] = p.mfGet();
  v[2] = 0;
  v[3] = 0;
  int n = VSCONST_FOG;
  //if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
  {
    m_CurParams[n][0] = v[0];
    m_CurParams[n][1] = v[1];
    m_CurParams[n][2] = v[2];
    m_CurParams[n][3] = v[3];
    dv->SetVertexShaderConstantF(n, v, 1);
  }

  v[0] = 0; v[1] = 0.25f; v[2] = 0.5f; v[3] = 1.0f;
  n = VSCONST_0_025_05_1;
  //if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
  {
    m_CurParams[n][0] = v[0];
    m_CurParams[n][1] = v[1];
    m_CurParams[n][2] = v[2];
    m_CurParams[n][3] = v[3];
    dv->SetVertexShaderConstantF(n, v, 1);
  }

  if ((gRenDev->m_Features & RFT_HW_HDR) || (r->m_Features & RFT_HW_PS30))
  {
    n = PSCONST_HDR_FOGCOLOR;
    //if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
    {
      m_CurParams[n][0] = r->m_FS.m_FogColor[0];
      m_CurParams[n][1] = r->m_FS.m_FogColor[1];
      m_CurParams[n][2] = r->m_FS.m_FogColor[2];
      m_CurParams[n][3] = r->m_FS.m_FogColor[3];
      dv->SetPixelShaderConstantF(n, &r->m_FS.m_FogColor[0], 1);
    }
  }
}

void CCGVProgram_D3D::mfSetVariables(TArray<SCGParam4f>* Vars)
{
  int i;

  for (i=0; i<Vars->Num(); i++)
  {
    SCGParam4f *p = &Vars->Get(i);
    if (p->m_nComponents > 1)
    {
      float matr[16][4];
      int nLast = i+p->m_nComponents;
      int n = 0;
      SCGParam4f *pMatr = p;
      for (; i<nLast; i++, n++)
      {
        p = &Vars->Get(i);
        float *v = p->mfGet();
        matr[n][0] = v[0]; matr[n][1] = v[1]; matr[n][2] = v[2]; matr[n][3] = v[3];      	
      }
      i--;
      mfParameter(pMatr, &matr[0][0], pMatr->m_nComponents);
    }
    else
    {
      float *v = p->mfGet();
      if (p->m_Flags & PF_INTEGER)
        mfParameter4i(p, v);
      else
        mfParameter4f(p, v);
    }
  }
}

void CCGVProgram_D3D::mfSetVariables(bool bObj, TArray<SCGParam4f>* Parms)
{
  if (m_Insts[m_CurInst].m_pHandle == 0 || (INT_PTR)m_Insts[m_CurInst].m_pHandle == -1)
    return;

  //PROFILE_FRAME(Shader_VShadersParms);

  if (!bObj)
  {
    if (m_ParamsNoObj.Num())
      mfSetVariables(&m_ParamsNoObj);

    if (m_Insts[m_CurInst].m_ParamsNoObj)
      mfSetVariables(m_Insts[m_CurInst].m_ParamsNoObj);
  }
  else
  {
    if (m_ParamsObj.Num())
      mfSetVariables(&m_ParamsObj);

    if (m_Insts[m_CurInst].m_ParamsObj)
      mfSetVariables(m_Insts[m_CurInst].m_ParamsObj);
  }

  if (Parms)
    mfSetVariables((TArray<SCGParam4f> *)Parms);
}


bool CCGVProgram_D3D::mfSet(bool bStat, SShaderPassHW *slw, int nFlags)
{
  //PROFILE_FRAME(Shader_VShaders);

  CD3D9Renderer *rd = gcpRendD3D;
  if (!bStat)
  {
#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log == 4)
      rd->Logv(SRendItem::m_RecurseLevel, "--- Reset CGVProgram \"%s\"\n", m_Name.c_str());
#endif
    m_LastVP = 0;
    mfUnbind();
    return true;
  }

  rd->m_RP.m_CurVP = this;

  int Mask;
  if ((m_Flags & VPFI_NOFOG) || !CRenderer::CV_r_vpfog || !(rd->m_Features & RFT_FOGVP))
    Mask = VPVST_NOFOG;
  else
    Mask = VPVST_FOGGLOBAL;
  if (nFlags & (VPF_INSTANCING_NOROTATE | VPF_INSTANCING_ROTATE))
  {
    if (nFlags & VPF_INSTANCING_NOROTATE)
      Mask |= VPVST_INSTANCING_NOROT;
    else
      Mask |= VPVST_INSTANCING_ROT;
  }
  if (rd->m_RP.m_PersFlags & RBPF_HDR)
    Mask |= VPVST_HDR;
  if (rd->m_RP.m_ClipPlaneEnabled == 1)
    Mask |= VPVST_CLIPPLANES3;
  Mask |= (rd->m_RP.m_FlagsModificators & VPVST_TCMASK);

  CVProgram *pPosVP = this;
  if (m_Flags & VPFI_UNIFIEDPOS)
  {
    CVProgram *pVP = NULL;
    if (rd->m_RP.m_pRE && rd->m_RP.m_pRE->m_LastVP)
      pVP = rd->m_RP.m_pRE->m_LastVP;
    else
    if (rd->m_RP.m_RendPass)
      pVP = rd->m_RP.m_LastVP;
    else
    if (rd->m_RP.m_pShader->m_DefaultVProgram)
      pVP = rd->m_RP.m_pShader->m_DefaultVProgram;
    if (pVP && pVP->m_bCGType)
    {
      pPosVP = pVP;
      if (pVP->m_Flags & VPFI_NOISE)
        Mask |= VPVST_NOISE;
    }
  }

  int Id = mfGetCGInstanceID(Mask, pPosVP, rd->m_RP.m_ShaderLightMask);

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log >= 3)
    rd->Logv(SRendItem::m_RecurseLevel, "--- Set CGVProgram \"%s\", LightMask: 0x%x, Mask: 0x%I64x (0x%x Type)\n", m_Name.c_str(), m_Insts[m_CurInst].m_LightMask, m_nMaskGen, Mask);
#endif

  if ((INT_PTR)m_Insts[Id].m_CGProgram == -1)
    return false;

  if (!m_Insts[Id].m_CGProgram)
  {
    if (!mfActivate(pPosVP))
    {
      m_Insts[Id].m_pHandle = (void *)-1;
      return false;
    }
    m_LastVP = NULL;
  }

#ifdef DO_RENDERSTATS
  if (m_Frame != rd->m_nFrameUpdateID)
  {
    m_Frame = rd->m_nFrameUpdateID;
    rd->m_RP.m_PS.m_NumVShaders++;
  }
#endif

  if (m_LastVP != this || m_LastTypeVP != Mask || m_LastLTypeVP != m_Insts[m_CurInst].m_LightMask)
  {
    rd->m_RP.m_PS.m_NumVShadChanges++;
    m_LastVP = this;
    m_LastTypeVP = Mask;
    m_LastLTypeVP = m_Insts[m_CurInst].m_LightMask;
    // set the vertex shader
    mfBind();
  }
  rd->m_RP.m_PersFlags |= RBPF_VSNEEDSET;
  if (!(nFlags & VPF_DONTSETMATRICES))
    mfSetStateMatrices();

  if (!(m_Flags & VPFI_UNIFIEDPOS))
    rd->m_RP.m_LastVP = this;

  return true;
}
