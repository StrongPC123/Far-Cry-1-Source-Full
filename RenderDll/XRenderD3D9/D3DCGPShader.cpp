/*=============================================================================
  GLCGPShader.cpp : OpenGL cg fragment programs support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "D3DCGPShader.h"
#include "D3DCGVProgram.h"

#ifndef PS2
#include <direct.h>
#include <io.h>
#else
#include "File.h"
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//=======================================================================

// init memory pool usage
#ifndef PS2
 _ACCESS_POOL;
#endif

vec4_t CCGPShader_D3D::m_CurParams[32];

TArray<CPShader *> CPShader::m_PShaders;
CPShader *CPShader::m_CurRC;

CPShader *CPShader::mfForName(const char *name, std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc, EShaderVersion eSHV, uint64 nMaskGen)
{
  int i;

  if (!name || !name[0])
    return NULL;

  if (!(gRenDev->GetFeatures() & (RFT_HW_TS)))
    return NULL;

  for (i=0; i<m_PShaders.Num(); i++)
  {
    if (!m_PShaders[i])
      continue;
    if (!stricmp(name, m_PShaders[i]->m_Name.c_str()) && m_PShaders[i]->m_nMaskGen == nMaskGen)
    {
      m_PShaders[i]->m_nRefCounter++;
      return m_PShaders[i];
    }
  }

  CPShader *p = NULL;
  {
    CPShader *pr = new CCGPShader_D3D;
    pr->m_Name = name;
    pr->m_Id = i;
    m_PShaders.AddElem(pr);
    pr->m_nRefCounter = 1;
    pr->m_nMaskGen = nMaskGen;
    p = pr;
  }
  if (eSHV == eSHV_PS_2_0)
    p->m_Flags |= PSFI_PS20ONLY;
  else
  if (eSHV == eSHV_PS_3_0)
    p->m_Flags |= PSFI_PS30ONLY;
  else
  if (eSHV == eSHV_PS_2_x)
    p->m_Flags |= PSFI_PS2XONLY;
  p->m_Flags |= PSFI_FX;
  CCGPShader_D3D *pr = (CCGPShader_D3D *)p;
  pr->mfConstructFX(Structs, Macros, entryFunc);

  return p;
}

void CCGPShader_D3D::mfConstructFX(std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc)
{
  int i;
  char dir[128];
  sprintf(dir, "%sDeclarations/CGPShaders/", gRenDev->m_cEF.m_HWPath);
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

void CCGPShader_D3D::mfPostLoad()
{
  gRenDev->m_cEF.mfCheckObjectDependParams(&m_ParamsNoObj, &m_ParamsObj);
}

void CCGPShader_D3D::mfAddFXParameter(SFXParam *pr, const char *ParamName, SShader *ef)
{
  SCGParam4f CGpr;
  int i;
  char scr[256];
  
  if (pr->m_Assign.size())
  {
    const char *translatedParam = pr->m_Assign.c_str();
    if (!stricmp(translatedParam, "Ambient"))
      translatedParam = "AmbLightColor";
    else
    if (!stricmp(translatedParam, "Diffuse"))
      translatedParam = "DiffuseColor";
    if (pr->m_nRows == 4)
      sprintf(scr, "Name=%s Comp '%s[0]' Comp '%s[1]' Comp '%s[2]' Comp '%s[3]'", ParamName, translatedParam, translatedParam, translatedParam, translatedParam);
    else
    if (pr->m_nRows == 3)
      sprintf(scr, "Name=%s Comp '%s[0]' Comp '%s[1]' Comp '%s[2]'", ParamName, translatedParam, translatedParam, translatedParam);
    else
    if (pr->m_nRows <= 1)
      sprintf(scr, "Name=%s Comp '%s'", ParamName, translatedParam);
    gRenDev->m_cEF.mfCompileCGParam(scr, ef, &m_ParamsNoObj);
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

bool CCGPShader_D3D::mfGetFXParamNameByID(int nParam, char *ParamName)
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

struct SAliasName
{
  bool bParam;
  std::string Name;
  std::string NameINT;
};
struct SAliasSampler
{
  SFXSampler *fxSampler;
  std::string NameINT;
  SAliasSampler()
  {
    fxSampler = NULL;
  }
};

void CCGPShader_D3D::mfGatherFXParameters(const char *buf, SShaderPassHW *pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader *ef)
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
  SAliasSampler samps[MAX_TMU];
  int nMaxSampler = -1;
  std::vector<SAliasName> Aliases;
  int nParam = 0;
  char PSParam[256];
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
        SAliasName an;
        an.bParam = true;
        an.Name = szParam;
        if (mfGetFXParamNameByID(nParam, PSParam))
          an.NameINT = PSParam;
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
          SAliasName an;
          an.bParam = false;
          an.Name = szParam;
          if (mfGetFXParamNameByID(nParam, PSParam))
            an.NameINT = PSParam;
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
      SAliasName *al = &Aliases[j];
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
      {
        // const parameters aren't listed in Params
        // assert(0);
      }
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
    if (!smp)
      continue;
    assert(smp->m_nTextureID < Textures.size());
    SFXTexture *tx = &Textures[smp->m_nTextureID];
#ifdef USE_FX
    gRenDev->m_cEF.mfParseFXTechnique_LoadShaderTexture(smp, tx, pSHPass, ef, i, eCO_NOSET, eCO_NOSET, DEF_TEXARG0, DEF_TEXARG0);
#endif
  }
}

//=========================================================================================

CPShader *CPShader::mfForName(const char *name, uint64 nMaskGen)
{
  int i;

  if (!name || !name[0])
    return NULL;

  if (!(gRenDev->GetFeatures() & (RFT_HW_RC | RFT_HW_TS | RFT_HW_PS20)))
    return NULL;

  for (i=0; i<m_PShaders.Num(); i++)
  {
    if (!m_PShaders[i])
      continue;
    if (!stricmp(name, m_PShaders[i]->m_Name.c_str()) && m_PShaders[i]->m_nMaskGen == nMaskGen)
    {
      m_PShaders[i]->m_nRefCounter++;
      return m_PShaders[i];
    }
  }
  char scrname[128];
  char dir[128];
  sprintf(dir, "%sDeclarations/CGPShaders/", gRenDev->m_cEF.m_HWPath);
  sprintf(scrname, "%s%s.crycg", dir, name);
  FILE *fp = iSystem->GetIPak()->FOpen(scrname, "r");
  if (!fp)
  {
		Warning( 0,scrname,"Couldn't find pixel shader '%s'",name );
    iLog->Log("WARNING: Couldn't find pixel shader '%s'", name);
    iLog->Log("WARNING: Full file name: '%s'", scrname);
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

  CPShader *p = NULL;
  CPShader *pr;
  pr = new CCGPShader_D3D;
  pr->m_Name = name;
  pr->m_Id = i;
  m_PShaders.AddElem(pr);
  pr->m_nRefCounter = 1;
  pr->m_nMaskGen = nMaskGen;

  p = pr;
  FILE *handle = iSystem->GetIPak()->FOpen(scrname, "rb");
  if (handle)
  {
    p->m_WriteTime = iSystem->GetIPak()->GetModificationTime(handle);
    iSystem->GetIPak()->FClose(handle);
  }

  p->mfCompile(buf);
  delete [] buf;
  if (CRenderer::CV_r_shadersprecache)
    p->mfPrecache();

  return p;
}

void CPShader::mfClearAll(void)
{
  for (int i=0; i<m_PShaders.Num(); i++)
  {
    CPShader *vp = m_PShaders[i];
    if (vp)
      delete vp;
  }
  m_PShaders.Free();
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

void CCGPShader_D3D::mfPrecacheLights(int nMask)
{
  int i, j;
  int nLightMask;

  m_Flags |= PSFI_PRECACHEPHASE;
  int nInst = m_Insts.Num();
  SCGInstance cgi;
  m_Insts.AddElem(cgi);
  SCGInstance *pi = &m_Insts[nInst];
  int nTempInst = m_CurInst;
  m_CurInst = nInst;
  int nShSave = CRenderer::CV_r_shaderssave;
  CRenderer::CV_r_shaderssave = 0;

  m_nFailed = 0;
  bool bSpec = (m_Flags & PSFI_NOSPECULAR) == 0;
  pi->m_Mask = nMask;
  pi->m_LightMask = 0;
  m_Flags &= ~PSFI_WASGENERATED;
  mfActivate();
  SAFE_DELETE(pi->m_BindVars);
  int nMaxLights = (gRenDev->m_bDeviceSupports_PS2X) ? NUM_PPLIGHTS_PERPASS_PS2X : NUM_PPLIGHTS_PERPASS_PS30;
  if (m_Flags & PSFI_WASGENERATED)
  {
    TArray<int> Masks;
    int Types[5];

    int nCombinations = 0;
    iLog->Log("Precache light shader %s(0x%I64x)\n", m_Name.c_str(), m_nMaskGen);

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
            mfActivate();
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
  m_Flags &= ~PSFI_PRECACHEPHASE;
  CRenderer::CV_r_shaderssave = nShSave;
}

void CCGPShader_D3D::mfPrecache()
{
  bool bPrevFog = gRenDev->m_FS.m_bEnable;
  gRenDev->m_FS.m_bEnable = true;

  int Mask;
  if ((m_Flags & PSFI_NOFOG) || !(gRenDev->m_Features & RFT_FOGVP))
    Mask = VPVST_NOFOG;
  else
    Mask = VPVST_FOGGLOBAL;

  int nLightMask = 0;
  if (m_Flags & PSFI_SUPPORTS_MULTILIGHTS)
  {
    if (CRenderer::CV_r_shadersprecache < 2)
    {
      mfPrecacheLights(Mask);
      if (gRenDev->m_Features & RFT_HW_HDR)
      {
        Mask |= VPVST_HDR;
        mfPrecacheLights(Mask);
      }
      if (gRenDev->m_bDeviceSupportsComprNormalmaps)
      {
        Mask &= ~VPVST_HDR;
        Mask |= VPVST_3DC;
        mfPrecacheLights(Mask);
      }
    }
    gRenDev->m_FS.m_bEnable = bPrevFog;
    return;
  }

  int Type = mfGetCGInstanceID(Mask, nLightMask);
  mfActivate();

  if (gcpRendD3D->m_d3dCaps.MaxUserClipPlanes == 0)
  {
    Mask |= VPVST_CLIPPLANES3;
    Type = mfGetCGInstanceID(Mask, nLightMask);
    mfActivate();
  }
  if (gRenDev->m_Features & RFT_HW_HDR)
  {
    Mask |= VPVST_HDR;
    Type = mfGetCGInstanceID(Mask, nLightMask);
    mfActivate();
  }
  if ((m_Flags & PSFI_SUPPORTS_INSTANCING) && (gRenDev->m_Features & RFT_HW_PS30))
  {
    Mask |= VPVST_INSTANCING_ROT | VPVST_INSTANCING_NOROT;
    Type = mfGetCGInstanceID(Mask, nLightMask);
    mfActivate();

    if (Mask & VPVST_HDR)
    {
      Mask &= ~VPVST_HDR;
      Type = mfGetCGInstanceID(Mask, nLightMask);
      mfActivate();
    }
  }

  gRenDev->m_FS.m_bEnable = bPrevFog;
}

void CCGPShader_D3D::mfReset()
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

void CCGPShader_D3D::mfFree()
{
  m_Flags = 0;

  if (m_CoreScript && !m_CoreScript->m_Name.GetIndex())
    SAFE_RELEASE(m_CoreScript);
  if (m_SubroutinesScript && !m_SubroutinesScript->m_Name.GetIndex())
    SAFE_RELEASE(m_SubroutinesScript);
  if (m_DeclarationsScript && !m_DeclarationsScript->m_Name.GetIndex())
    SAFE_RELEASE(m_DeclarationsScript);
  if (m_InputParmsScript && !m_InputParmsScript->m_Name.GetIndex())
    SAFE_RELEASE(m_InputParmsScript);

  m_ParamsNoObj.Free();
  m_ParamsObj.Free();

  mfReset();
}

CCGPShader_D3D::~CCGPShader_D3D()
{
  mfFree();
  CPShader::m_PShaders[m_Id] = NULL;
}

void CCGPShader_D3D::Release()
{
  m_nRefCounter--;
  if (!m_nRefCounter)
    delete this;
}

char *CCGPShader_D3D::mfGenerateScriptPS()
{
  TArray<char> newScr;
  int i;

  newScr.Copy("# define _PS\n", strlen("# define _PS\n"));

  if (m_CGProfileType == CG_PROFILE_PS_1_1)
    newScr.Copy("# define _PS_1_1\n", strlen("# define _PS_1_1\n"));

  if (m_Insts[m_CurInst].m_Mask & VPVST_HDR)
  {
    newScr.Copy("# define _HDR\n", strlen("# define _HDR\n"));
    //if (gcpRendD3D->m_nHDRType == 2)
    //  newScr.Copy("# define _HDR_MRT\n", strlen("# define _HDR_MRT\n"));
    if (gcpRendD3D->m_nHDRType == 2)
      newScr.Copy("# define _HDR_ATI\n", strlen("# define _HDR_ATI\n"));
    if (CRenderer::CV_r_hdrfake)
      newScr.Copy("# define _HDR_FAKE\n", strlen("# define _HDR_FAKE\n"));
    if (m_Insts[m_CurInst].m_Mask & VPVST_HDRLM)
      newScr.Copy("# define _HDRLM\n", strlen("# define _HDRLM\n"));
  }

  if (m_Insts[m_CurInst].m_Mask & VPVST_3DC)
  {
    if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 1)
      newScr.Copy("# define _3DC\n", strlen("# define _3DC\n"));
    else
    if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 2)
      newScr.Copy("# define _V8U8\n", strlen("# define _V8U8\n"));
    else
    if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 3)
      newScr.Copy("# define _CxV8U8\n", strlen("# define _CxV8U8\n"));

    if (m_Insts[m_CurInst].m_Mask & VPVST_3DC_A)
      newScr.Copy("# define _3DC_A\n", strlen("# define _3DC_A\n"));
  }
  if (m_Insts[m_CurInst].m_Mask & VPVST_SPECANTIALIAS)
    newScr.Copy("# define _SPEC_ANTIALIAS\n", strlen("# define _SPEC_ANTIALIAS\n"));

  if (m_Insts[m_CurInst].m_Mask & VPVST_INSTANCING_NOROT)
    newScr.Copy("# define _INST\n", strlen("# define _INST\n"));
  if (m_Flags & PSFI_SUPPORTS_MULTILIGHTS)
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
    char str[8192];
    sprintf(str, "pixout main(vertout IN, %s)\n{\n  pixout OUT;\n", szInputParms);
    newScr.Copy(str, strlen(str));
  }

  if (m_CoreScript)
    newScr.Copy(m_CoreScript->m_Script, strlen(m_CoreScript->m_Script));

  if (!m_Functions.size())
  {
    char *pExit = "return OUT;\n}";
    newScr.Copy(pExit, strlen(pExit));
  }
  newScr.AddElem(0);

  char *cgs = new char[newScr.Num()];
  memcpy(cgs, &newScr[0], newScr.Num());

  if (m_Flags & PSFI_AUTOENUMTC)
  {
    int n = 0;
    char *s = cgs;
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
    n = 0;
    s = cgs;
    while (true)
    {
      s = strstr(s, "register(sn)");
      if (!s)
        break;
      char *ss = s;
      bool bIncr = true;
      while (ss != cgs)
      {
        if (ss[0] == 0xa)
          break;
        ss--;
      }
      if (ss != cgs)
      {
        ss--;
        while (ss != cgs)
        {
          if (ss[0] == 0xa)
            break;
          ss--;
        }
        if (ss != cgs)
        {
          SkipCharacters(&ss, kWhiteSpace);
          if (!strncmp(ss, "# ifdef", 7))
          {
            ss += 7;
            SkipCharacters(&ss, kWhiteSpace);
            if (!strncmp(ss, "_3DC_A", 6) && !(m_Insts[m_CurInst].m_Mask & VPVST_3DC_A))
              bIncr = false;
            else
            if (!strncmp(ss, "_SPEC_ANTIALIAS", 15) && !(m_Insts[m_CurInst].m_Mask & VPVST_SPECANTIALIAS))
              bIncr = false;
            else
            if (!strncmp(ss, "_HDRLM", 6) && !(m_Insts[m_CurInst].m_Mask & VPVST_HDRLM))
              bIncr = false;
          }
        }
      }
      if (bIncr)
      {
        s[10] = 0x30 + n;
        n++;
      }
      else
        s[10] = 0x37;
    }

    char *sNewStr = strstr(cgs, "main");
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

  return cgs;
}

char *CCGPShader_D3D::mfCreateAdditionalPS()
{
  char *pScr = mfGenerateScriptPS();
  if (!pScr)
    return NULL;

  char *sNewStr, *newScr;
  int Mask = m_Insts[m_CurInst].m_Mask;
  int n, len;

  // Calculate fog explicitly in the pixel shader if HDR is enabled
  sNewStr = pScr;
  while (true)
  {
    sNewStr = strstr(sNewStr, "main");
    if (!sNewStr)
      break;
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
      char *strFog = ", uniform float3 GlobalFogColor : register(c7)";
      if (m_CGProfileType > CG_PROFILE_PS_1_1)
        strFog = ", uniform float3 GlobalFogColor : register(c31)";
      len = strlen(strFog);
      int len2 = strlen(pScr)+1;
      newScr = new char[len+len2];
      n = sNewStr-pScr+n;
      strncpy(newScr, pScr, n);
      strcpy(&newScr[n], strFog);
      strcpy(&newScr[n+len], &pScr[n]);
      delete [] pScr;
      pScr = newScr;
      break;
    }
  }

  return pScr;
}

void CCGPShader_D3D::mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params)
{
  gRenDev->m_cEF.mfCompileCGParam(scr, ef, Params);
}

bool CCGPShader_D3D::mfCompile(char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eScript=1, eParam4f, eNoFog, ePS20Only, ePS30Only, ePS2XOnly, eAutoEnumTC, eMainInput, eSubrScript, eDeclarationsScript, eCoreScript, eSupportsInstancing, eSupportsMultiLights, eNoSpecular, eMRT};
  static tokenDesc commands[] =
  {
    {eScript, "Script"},
    {eAutoEnumTC, "AutoEnumTC"},
    {eParam4f, "Param4f"},
    {eMainInput, "MainInput"},
    {eDeclarationsScript, "DeclarationsScript"},
    {eCoreScript, "CoreScript"},
    {eSubrScript, "SubrScript"},
    {eNoFog, "NoFog"},
    {eMRT, "MRT"},
    {ePS20Only, "PS20Only"},
    {ePS30Only, "PS30Only"},
    {ePS2XOnly, "PS2XOnly"},
    {eNoSpecular, "NoSpecular"},
    {eSupportsInstancing, "SupportsInstancing"},
    {eSupportsMultiLights, "SupportsMultiLights"},

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
      case eMainInput:
        m_InputParmsScript = CCGVProgram_D3D::mfAddNewScript(name, params);
        break;

      case eDeclarationsScript:
        m_DeclarationsScript = CCGVProgram_D3D::mfAddNewScript(name, params);
        break;

      case eCoreScript:
        m_CoreScript = CCGVProgram_D3D::mfAddNewScript(name, params);
        break;

      case eSubrScript:
        m_SubroutinesScript = CCGVProgram_D3D::mfAddNewScript(name, params);
        break;

      case eParam4f:
        mfCompileParam4f(params, ef, &m_ParamsNoObj);
        break;

      case eNoFog:
        m_Flags |= PSFI_NOFOG;
        break;

      case eMRT:
        m_Flags |= PSFI_MRT;
        break;

      case eNoSpecular:
        m_Flags |= PSFI_NOSPECULAR;
        break;

      case eSupportsInstancing:
        m_Flags |= PSFI_SUPPORTS_INSTANCING;
        break;

      case eSupportsMultiLights:
        m_Flags |= PSFI_SUPPORTS_MULTILIGHTS;
        break;

      case eAutoEnumTC:
        m_Flags |= PSFI_AUTOENUMTC;
        break;

      case ePS20Only:
        m_Flags |= PSFI_PS20ONLY;
        break;

      case ePS30Only:
        m_Flags |= PSFI_PS30ONLY;
        break;

      case ePS2XOnly:
        m_Flags |= PSFI_PS2XONLY;
        break;
    }
  }

  gRenDev->m_cEF.mfCheckObjectDependParams(&m_ParamsNoObj, &m_ParamsObj);

  return 1;
}

void CCGPShader_D3D::mfSetVariables(TArray<SCGParam4f>* Vars)
{
  int i;

  for (i=0; i<Vars->Num(); i++)
  {
    SCGParam4f *p = &Vars->Get(i);
    if (p->m_nComponents > 1)
    {
      float matr[4][4];
      int nLast = i+p->m_nComponents;
      assert(nLast <= Vars->Num());
      int n = 0;
      for (; i<nLast; i++, n++)
      {
        SCGParam4f *Param = &Vars->Get(i);
        float *v = Param->mfGet();
        matr[n][0] = v[0]; matr[n][1] = v[1]; matr[n][2] = v[2]; matr[n][3] = v[3];      	
      }
      i--;
      mfParameter(p, &matr[0][0], p->m_nComponents);
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

void CCGPShader_D3D::mfSetVariables(bool bObj, TArray<SCGParam4f>* Parms)
{
  if (m_Insts[m_CurInst].m_pHandle == NULL || (INT_PTR)m_Insts[m_CurInst].m_pHandle == -1)
    return;

  //PROFILE_FRAME(Shader_PShadersParms);

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
    mfSetVariables(Parms);
}

void CCGPShader_D3D::mfGetSrcFileName(char *srcname, int nSize)
{
  strncpy(srcname, gRenDev->m_cEF.m_HWPath, nSize);
  strncat(srcname, "Declarations/CGPShaders/", nSize);
  strncat(srcname, m_Name.c_str(), nSize);
  strncat(srcname, ".crycg", nSize);
}

void CCGPShader_D3D::mfGetDstFileName(char *dstname, int nSize, bool bUseASCIICache)
{
  char *type;
  bool bFog = false;
  if (m_Flags & PSFI_AUTOENUMTC)
  {
    if (m_CGProfileType == CG_PROFILE_PS_2_X)
    {
      bFog = true;
      int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;
      if (nGPU == RFT_HW_GFFX)
      {
        if (CRenderer::CV_r_sm2xpath == 2)
          type = "D3D9_PS2A";
        else
          type = "D3D9_PS2X";
      }
      else
        type = "D3D9_PS2X";
    }
    else
      type = "D3D9_Auto";
  }
  else
  if (m_CGProfileType == CG_PROFILE_PS_1_1)
    type = "D3D9_PS11";
  else
  if (m_CGProfileType == CG_PROFILE_PS_1_2)
    type = "D3D9_PS11";
  else
  if (m_CGProfileType == CG_PROFILE_PS_1_3)
    type = "D3D9_PS11";
  else
  if (m_CGProfileType == CG_PROFILE_PS_2_0)
    type = "D3D9_PS20";
  else
  if (m_CGProfileType == CG_PROFILE_PS_3_0)
    type = "D3D9_PS30";
  else
  if (m_CGProfileType == CG_PROFILE_PS_2_X)
  {
    int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;
    if (nGPU == RFT_HW_GFFX)
    {
      if (CRenderer::CV_r_sm2xpath == 2)
        type = "D3D9_PS2A";
      else
        type = "D3D9_PS2X";
    }
    else
      type = "D3D9_PS2X";
  }
  else
    type = "Unknown";

  char *fog;
  if ((m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL) || bFog)
    fog = "Fog";
  else
  if (m_Insts[m_CurInst].m_Mask & VPVST_NOFOG)
    fog = "NoFog";
  
  strncpy(dstname, gRenDev->m_cEF.m_ShadersCache, nSize);
  strncat(dstname, "CGPShaders/", nSize);
  strncat(dstname, m_Name.c_str(), nSize);
  if (type)
  {
    strncat(dstname, "$", nSize);
    strncat(dstname, type, nSize);
  }
  /*if ((m_Flags & PSFI_AUTOENUMTC) || m_CGProfileType == CG_PROFILE_PS_2_0 || m_CGProfileType == CG_PROFILE_PS_2_X)
  {
    if (D3DSHADER_VERSION_MAJOR(gcpRendD3D->m_d3dCaps.PixelShaderVersion) >= 2)
    {
      if (!(gcpRendD3D->m_d3dCaps.PS20Caps.Caps & D3DPS20CAPS_ARBITRARYSWIZZLE))
      {
        strncat(dstname, "$", nSize);
        strncat(dstname, "20_NO_ASW", nSize);
      }
    }
  }*/
  strncat(dstname, "$", nSize);
  strncat(dstname, fog, nSize);

  if (m_Insts[m_CurInst].m_Mask & VPVST_INSTANCING_NOROT)
    strncat(dstname, "$INST", nSize);
  if (m_Insts[m_CurInst].m_Mask & VPVST_HDR)
  {
    if (!(m_Insts[m_CurInst].m_Mask & VPVST_HDRREAL))
      strncat(dstname, "$HDR", nSize);
    else
      strncat(dstname, "$HDR_REAL", nSize);
    if (gcpRendD3D->m_nHDRType == 2)
      strncat(dstname, "$MRT", nSize);
  }
  if (m_Insts[m_CurInst].m_Mask & VPVST_HDRLM)
    strncat(dstname, "$HDRLM", nSize);
  if (m_Insts[m_CurInst].m_Mask & VPVST_3DC)
  {
    if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 1)
      strncat(dstname, "$3DC", nSize);
    else
    if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 2)
      strncat(dstname, "$V8U8", nSize);
    else
    if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 3)
      strncat(dstname, "$CxV8U8", nSize);
  }
  if (m_Insts[m_CurInst].m_Mask & VPVST_3DC_A)
    strncat(dstname, "$3DCA", nSize);
  if (m_Insts[m_CurInst].m_Mask & VPVST_SPECANTIALIAS)
    strncat(dstname, "$SA", nSize);
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

  strncat(dstname, ".cgps", nSize);
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

bool CCGPShader_D3D::ActivateCacheItem(SShaderCacheHeaderItem *pItem)
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
  HRESULT hr = gcpRendD3D->mfGetD3DDevice()->CreatePixelShader((DWORD*)pData, (IDirect3DPixelShader9 **)&m_Insts[m_CurInst].m_pHandle);
  if (FAILED(hr))
    return false;
  return true;
}
bool CCGPShader_D3D::CreateCacheItem(int nMask, byte *pData, int nLen)
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
  if (!(m_Flags & PSFI_PRECACHEPHASE))
    gRenDev->m_cEF.FlushCacheFile(m_InstCache[m_Insts[m_CurInst].m_nCacheID].m_pCache);

  return bRes;
}

// Compile pixel shader for the current instance properties
bool CCGPShader_D3D::mfActivate()
{
  PROFILE_FRAME(Shader_PShaderActivate);

  if (!m_Insts[m_CurInst].m_pHandle)
  {
    if (m_Flags & PSFI_PS30ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS30)
        m_CGProfileType = CG_PROFILE_PS_3_0;
      else
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
      {
        if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
          m_CGProfileType = CG_PROFILE_PS_2_X;
        else
          m_CGProfileType = CG_PROFILE_PS_2_0;
      }
      else
        m_CGProfileType = CG_PROFILE_PS_1_1;
    }
    else
    if (m_Flags & PSFI_PS2XONLY)
    {
      if (gRenDev->m_bDeviceSupports_PS2X)
        m_CGProfileType = CG_PROFILE_PS_2_X;
      else
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
      {
        if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
          m_CGProfileType = CG_PROFILE_PS_2_X;
        else
          m_CGProfileType = CG_PROFILE_PS_2_0;
      }
      else
        m_CGProfileType = CG_PROFILE_PS_1_1;
    }
    else
    if (m_Flags & PSFI_PS20ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
      {
        if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
          m_CGProfileType = CG_PROFILE_PS_2_X;
        else
          m_CGProfileType = CG_PROFILE_PS_2_0;
      }
      else
        m_CGProfileType = CG_PROFILE_PS_1_1;
    }
    else
      m_CGProfileType = CG_PROFILE_PS_1_1;

    if ((m_Insts[m_CurInst].m_Mask & VPVST_HDR) && m_CGProfileType == CG_PROFILE_PS_1_1)
    {
      // HDR requires at least PS2.0 shader profile
      if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
        m_CGProfileType = CG_PROFILE_PS_2_X;
      else
        m_CGProfileType = CG_PROFILE_PS_2_0;
    }

    char strVer[128];
    char strVer0[128];
    sprintf(strVer, "//CGVER%.1f\n", CG_FP_CACHE_VER);
    bool bCreate = false;
    char namesrc[256];
    char namedst[256];
    char namedst1[256];
    FILETIME writetimesrc,writetimedst;
    FILE *statussrc, *statusdst;
    statussrc = NULL;
    statusdst = NULL;
    mfGetSrcFileName(namesrc, 256);

    bool bUseACIICache = true;
    if (CRenderer::CV_r_shadersprecache < 2 && (m_Flags & PSFI_SUPPORTS_MULTILIGHTS))
      bUseACIICache = false;
    mfGetDstFileName(namedst, 256, bUseACIICache);
    StripExtension(namedst, namedst1);
    // Use binary cache files for PS30 and PS20b shaders
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
        if (m_Flags & PSFI_PRECACHEPHASE)
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
          if (CompareFileTime(&writetimesrc, &writetimedst) != 0)
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
      m_Flags |= PSFI_WASGENERATED;
      char *scr = mfCreateAdditionalPS();

      char *code = mfLoadCG(scr);
      delete [] scr;
      pbuf = code;

      // Create asm cache file
      if (bUseACIICache)
      {
        if (code)
        {
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
        m_Insts[m_CurInst].m_pHandle = NULL;
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
      if (!bCreate && (m_Flags & PSFI_AUTOENUMTC) && !(m_Flags & PSFI_SUPPORTS_MULTILIGHTS))
      {
        if (strstr(pbuf, "ps_2_x"))
        {
          if (D3DSHADER_VERSION_MAJOR(gcpRendD3D->m_d3dCaps.PixelShaderVersion) < 2)
          {
            delete [] pbuf;
            bCreate = true;
            goto create;
          }
          else
          {
            int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;
            if (nGPU == RFT_HW_RADEON || nGPU == RFT_HW_NV4X)
            {
              delete [] pbuf;
              m_CGProfileType = CG_PROFILE_PS_2_0;
              bCreate = true;
              goto create;
            }
          }
          m_CGProfileType = CG_PROFILE_PS_2_X;
        }
        else
        if (strstr(pbuf, "ps_2_0"))
        {
          if (D3DSHADER_VERSION_MAJOR(gcpRendD3D->m_d3dCaps.PixelShaderVersion) < 2)
          {
            delete [] pbuf;
            bCreate = true;
            goto create;
          }
          else
          if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
          {
            delete [] pbuf;
            m_CGProfileType = CG_PROFILE_PS_2_X;
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_PS_2_0;
        }
        else
        if (strstr(pbuf, "ps_3_0"))
        {
          if (D3DSHADER_VERSION_MAJOR(gcpRendD3D->m_d3dCaps.PixelShaderVersion) < 3)
          {
            delete [] pbuf;
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_PS_3_0;
        }
        else
          m_CGProfileType = CG_PROFILE_PS_1_1;
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
      while (token)
      {
        if (m_CGProfileType == CG_PROFILE_PS_2_0 || m_CGProfileType == CG_PROFILE_PS_2_X || m_CGProfileType == CG_PROFILE_PS_3_0)
        {
          while (token[0]==0x20) {token++;}
          if (token[0] == '$')
            token++;
          char *szName = sGetText(&token);
          char *szReg = sGetText(&token);
          if (szReg[0] == 'c' && isdigit(szReg[1]))
          {
            SCGBind cgp;
            if (!m_Insts[m_CurInst].m_BindVars)
              m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
            char *szSize = sGetText(&token);
            cgp.m_nComponents = atoi(szSize);
            cgp.m_Name = szName;
            cgp.m_dwBind = atoi(&szReg[1]);
            m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
          }
          else
          if (szReg[0] == 's' && isdigit(szReg[1]) && (m_Flags & PSFI_FX))
          {
            SCGBind cgp;
            if (!m_Insts[m_CurInst].m_BindVars)
              m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
            char *szSize = sGetText(&token);
            cgp.m_nComponents = atoi(szSize);
            cgp.m_Name = szName;
            cgp.m_dwBind = atoi(&szReg[1]) | SHADER_BIND_SAMPLER;
            m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
          }
        }
        else
        {
          if (!strncmp(token, "const", 5))
          {
            token += 5;
            char *szhReg = sGetText(&token);
            char *szEqual = sGetText(&token);

            char *szF0 = sGetText(&token);
            char *szF1 = sGetText(&token);
            char *szF2 = sGetText(&token);
            char *szF3 = sGetText(&token);

            if (!m_Insts[m_CurInst].m_BindConstants)
              m_Insts[m_CurInst].m_BindConstants = new TArray<SCGBindConst>;
            SCGBindConst cgp;
            cgp.m_nComponents = 1;
            cgp.m_Val[0] = (float)atof(szF0);
            cgp.m_Val[1] = (float)atof(szF1);
            cgp.m_Val[2] = (float)atof(szF2);
            cgp.m_Val[3] = (float)atof(szF3);
            cgp.m_dwBind = atoi(&szhReg[2]);
            m_Insts[m_CurInst].m_BindConstants->AddElem(cgp);
          }
          else
          if (!strncmp(token, "var", 3))
          {
            token += 3;
            char *szType = sGetText(&token);
            char *szName = sGetText(&token);
            char *szvAttr = sGetText(&token);
            char *szhReg = sGetText(&token);
            if ((m_Flags & PSFI_FX) && !strncmp(szType, "sampler", 7))
            {
              SCGBind cgp;
              if (!m_Insts[m_CurInst].m_BindVars)
                m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
              cgp.m_nComponents = 1;
              cgp.m_Name = szName;
              assert (!strncmp(szhReg, "texunit", 7));
              char *szSize = sGetText(&token);
              cgp.m_dwBind = atoi(szSize) | SHADER_BIND_SAMPLER;
              m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
            }
            else
            if (szhReg)
            {
              if (szhReg[0] == 'c' && szhReg[1] == '[')
              {
                int len = strlen(szhReg);
                if (szhReg[len-1] == ',')
                {
                  char *sznComps = sGetText(&token);
                  nComps = atoi(sznComps);
                }
                else
                  nComps = 1;
                char *szVarType = sGetText(&token);
                char *sznVar = sGetText(&token);
                if (!m_Insts[m_CurInst].m_BindVars)
                  m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
                SCGBind cgp;
                cgp.m_nComponents = nComps;
                cgp.m_Name = szName;
                cgp.m_dwBind = atoi(&szhReg[2]);
                m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
              }
              else
              if (!strncmp(szhReg, "OFFSET_TEXTURE_MATRIX", 21))
              {
                if (!m_Insts[m_CurInst].m_BindVars)
                  m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
                SCGBind cgp;
                cgp.m_nComponents = nComps;
                cgp.m_Name = szName;
                cgp.m_dwBind = GL_OFFSET_TEXTURE_2D_MATRIX_NV;
                cgp.m_dwBind |= (atoi(&szhReg[22])+1) << 28;
                m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
              }
            }
          }
        }
        token = strtok(NULL, "//");
      }
      if (m_Insts[m_CurInst].m_BindVars)
      {
        m_Insts[m_CurInst].m_BindVars->Shrink();
        for (int i=0; i<m_Insts[m_CurInst].m_BindVars->Num(); i++)
        {
          SCGBind *pBind = &m_Insts[m_CurInst].m_BindVars->Get(i);
          for (int j=i+1; j<m_Insts[m_CurInst].m_BindVars->Num(); j++)
          {
            SCGBind *pBind0 = &m_Insts[m_CurInst].m_BindVars->Get(j);
            if (pBind0->m_Name == pBind->m_Name)
            {
              pBind->m_pNext = pBind0;
              break;
            }
          }
        }
      }
      assert(!m_Insts[m_CurInst].m_BindVars || m_Insts[m_CurInst].m_BindVars->Num()<=30);
      SAFE_DELETE_ARRAY(pbuf);
      if (pCode && !bUseACIICache)
        CreateCacheItem(m_Insts[m_CurInst].m_LightMask, (byte *)pCode->GetBufferPointer(), pCode->GetBufferSize());
      SAFE_RELEASE(pCode);
    }
    if (!(m_Flags & PSFI_PRECACHEPHASE))
      mfUnbind();
  }

  if (!m_Insts[m_CurInst].m_pHandle)
    return false;
  return true;
}

bool CCGPShader_D3D::mfSet(bool bEnable, SShaderPassHW *slw, int nFlags)
{
  //PROFILE_FRAME(Shader_PShaders);

  int Mask;
  CD3D9Renderer *rd = gcpRendD3D;

  if (!bEnable)
  {
#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log == 4)
      rd->Logv(SRendItem::m_RecurseLevel, "--- Reset CGPShader \"%s\"\n", m_Name.c_str());
#endif
    m_LastVP = 0;
    mfUnbind();
    return true;
  }
  else
  {
    if (slw && (slw->m_Flags & SHPF_ALLOW_SPECANTIALIAS) && CRenderer::CV_r_specantialias)
    {
      rd->m_RP.m_FlagsPerFlush |= RBSI_USE_SPECANTIALIAS;
      if (rd->m_RP.m_pShaderResources)
      {
        if (!rd->m_RP.m_pShaderResources->m_Textures[EFTT_PHONG])
          rd->m_RP.m_pShaderResources->AddTextureMap(EFTT_PHONG);
        if (rd->m_RP.m_pShaderResources->m_Textures[EFTT_PHONG]->m_Amount != rd->m_RP.m_fCurSpecShininess)
        {
          if (rd->m_RP.m_pShaderResources->m_Textures[EFTT_PHONG]->m_TU.m_TexPic)
            rd->m_RP.m_pShaderResources->m_Textures[EFTT_PHONG]->m_TU.m_TexPic->Release(false);
          rd->m_RP.m_pShaderResources->m_Textures[EFTT_PHONG]->m_Amount = (byte)rd->m_RP.m_fCurSpecShininess;
          rd->m_RP.m_pShaderResources->m_Textures[EFTT_PHONG]->m_TU.m_TexPic = rd->EF_MakeSpecularTexture(rd->m_RP.m_fCurSpecShininess);
        }
        rd->EF_SelectTMU(CTexMan::m_nCurStages);
        rd->m_RP.m_pShaderResources->m_Textures[EFTT_PHONG]->m_TU.m_TexPic->Set();
        CTexMan::m_nCurStages++;
      }
    }

    if ((m_Flags & PSFI_NOFOG) || !CRenderer::CV_r_vpfog || !(rd->m_Features & RFT_FOGVP))
      Mask = VPVST_NOFOG;
    else
      Mask = VPVST_FOGGLOBAL;
    if (nFlags & PSF_INSTANCING)
      Mask |= VPVST_INSTANCING_NOROT;
    if (rd->m_RP.m_FlagsPerFlush & RBSI_USE_3DC)
    {
      Mask |= VPVST_3DC;
      if (rd->m_RP.m_FlagsPerFlush & RBSI_USE_3DC_A)
        Mask |= VPVST_3DC_A;
    }
    if (rd->m_RP.m_FlagsPerFlush & RBSI_USE_SPECANTIALIAS)
      Mask |= VPVST_SPECANTIALIAS;
    if (rd->m_RP.m_PersFlags & RBPF_HDR)
    {
      Mask |= VPVST_HDR;
      if (rd->m_RP.m_FlagsPerFlush & RBSI_USE_HDRLM)
        Mask |= VPVST_HDRLM;
      if (!CRenderer::CV_r_hdrfake)
        Mask |= VPVST_HDRREAL;
    }
    if (rd->m_RP.m_ClipPlaneEnabled == 1)
      Mask |= VPVST_CLIPPLANES3;

    int Type = mfGetCGInstanceID(Mask, rd->m_RP.m_ShaderLightMask);

#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
      rd->Logv(SRendItem::m_RecurseLevel, "--- Set CGPShader \"%s\", LightMask: 0x%x, Mask: 0x%I64x (0x%x Type)\n", m_Name.c_str(), m_Insts[m_CurInst].m_LightMask, m_nMaskGen, Mask);
#endif

    if ((INT_PTR)m_Insts[Type].m_pHandle == -1)
    {
      m_LastTypeVP = Mask;
      return false;
    }

    if (!m_Insts[Type].m_pHandle)
    {
      rd->m_RP.m_CurPS = this;
      if (!mfActivate())
      {
        m_Insts[Type].m_pHandle = (CGprogram)-1;
        return false;
      }
      m_LastVP = NULL;
    }

#ifdef DO_RENDERSTATS
    if (m_Frame != rd->m_nFrameUpdateID)
    {
      m_Frame = rd->m_nFrameUpdateID;
      rd->m_RP.m_PS.m_NumPShaders++;
    }
#endif

    if (m_LastVP != this || m_LastTypeVP != Mask || m_LastLTypeVP != m_Insts[m_CurInst].m_LightMask)
    {
      rd->m_RP.m_PS.m_NumPShadChanges++;
      m_LastVP = this;
      m_LastTypeVP = Mask;
      m_LastLTypeVP = m_Insts[m_CurInst].m_LightMask;
      mfBind();
    }
    rd->m_RP.m_PersFlags |= RBPF_PS1NEEDSET;
    if (slw && slw->m_CGFSParamsNoObj)
      mfSetVariables(false, slw->m_CGFSParamsNoObj);
    else
      mfSetVariables(false, NULL);
  }
  m_CurRC = this;
  return true;
}
