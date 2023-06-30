/*=============================================================================
  GLCGPShader.cpp : OpenGL cg fragment programs support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "GLCGPShader.h"
#include "GLCGVProgram.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

TArray<CPShader *> CPShader::m_PShaders;
CPShader *CPShader::m_CurRC;

#include "nvparse/nvparse.h"

#ifdef WIN64
#pragma warning( push )							//AMD Port
#pragma warning( disable : 4267 )				// conversion from 'size_t' to 'int', possible loss of data
#endif

// init memory pool usage
#ifndef PS2
#ifndef _XBOX
 _ACCESS_POOL;
#endif
#endif

//=======================================================================

vec4_t CCGPShader_GL::m_CurParams[32];
vec4_t CCGPShader_GL::m_CurParamsARB[32];

CPShader *CPShader::mfForName(const char *name, std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc, EShaderVersion eSHV, uint64 nMaskGen)
{
  assert(0);
  return 0;
}

//=======================================================================

CPShader *CPShader::mfForName(const char *name, uint64 nMask)
{
  int i;

  if (!(gRenDev->GetFeatures() & (RFT_HW_RC | RFT_HW_TS | RFT_HW_PS20)))
    return NULL;

  for (i=0; i<m_PShaders.Num(); i++)
  {
    if (!m_PShaders[i])
      continue;
    if (!stricmp(name, m_PShaders[i]->m_Name.c_str()) && m_PShaders[i]->m_nMaskGen == nMask)
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
  pr = new CCGPShader_GL;
  pr->m_Name = name;
  pr->m_Id = i;
  m_PShaders.AddElem(pr);
  pr->m_nRefCounter = 1;
  pr->m_nMaskGen = nMask;
  p = pr;

  FILE *handle = iSystem->GetIPak()->FOpen(scrname, "rb");
  if (handle)
  {
    p->m_WriteTime = iSystem->GetIPak()->GetModificationTime(handle);
    iSystem->GetIPak()->FClose(handle);
  }

  p->mfCompile(buf);

  if (CRenderer::CV_r_shadersprecache==1)
    p->mfPrecache();
  delete [] buf;

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


void CCGPShader_GL::mfPrecache()
{
  bool bPrevFog = gRenDev->m_FS.m_bEnable;
  gRenDev->m_FS.m_bEnable = true;
  mfSet(true, NULL);
  gRenDev->m_FS.m_bEnable = bPrevFog;
}

void CCGPShader_GL::mfReset()
{
  for (int i=0; i<m_Insts.Num(); i++)
  {
    m_CurInst = i;
    mfDel();
  }
  m_Insts.Free();
  m_dwFrame++;
}

CCGPShader_GL::CCGPShader_GL()
{
  mfInit();
#ifndef WIN64
	// NOTE: AMD64 port: find the 64-bit CG runtime
  m_CGProfileType = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  if (m_CGProfileType == CG_PROFILE_FP30)
#endif
		m_CGProfileType = CG_PROFILE_ARBFP1;
  if (CGLRenderer::CV_gl_psforce11 && SUPPORTS_GL_NV_register_combiners)
    m_CGProfileType = CG_PROFILE_FP20;
}

void CCGPShader_GL::mfFree()
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

CCGPShader_GL::~CCGPShader_GL()
{
  mfFree();
  CPShader::m_PShaders[m_Id] = NULL;
}

void CCGPShader_GL::Release()
{
  m_nRefCounter--;
  if (!m_nRefCounter)
    delete this;
}

char *CCGPShader_GL::mfGenerateScriptPS()
{
  TArray<char> newScr;

  if (m_CGProfileType == CG_PROFILE_FP20)
    newScr.Copy("# define _PS_1_1\n", strlen("# define _PS_1_1\n"));

  if (m_Insts[m_CurInst].m_Mask & VPVST_HDR)
    newScr.Copy("# define _HDR\n", strlen("# define _HDR\n"));

  if (CRenderer::CV_r_hdrfake)
    newScr.Copy("# define _HDR_FAKE\n", strlen("# define _HDR_FAKE\n"));

  if (m_Insts[m_CurInst].m_Mask & VPVST_HDRLM)
    newScr.Copy("# define _HDRLM\n", strlen("# define _HDRLM\n"));

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

  char str[8192];
  sprintf(str, "pixout main(vertout IN, %s) : COLOR\n{\n  pixout OUT;\n", szInputParms);
  newScr.Copy(str, strlen(str));

  if (m_CoreScript)
    newScr.Copy(m_CoreScript->m_Script, strlen(m_CoreScript->m_Script));

  char *pExit = "return OUT;\n}";
  newScr.Copy(pExit, strlen(pExit));
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
      s[10] = 0x30 + n;
      n++;
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

char *CCGPShader_GL::mfCreateAdditionalPS()
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
      if (m_CGProfileType >= CG_PROFILE_ARBFP1)
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

void CCGPShader_GL::mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params)
{
  gRenDev->m_cEF.mfCompileCGParam(scr, ef, Params);
}

bool CCGPShader_GL::mfCompile(char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eScript=1, eParam4f, eNoFog, eAutoEnumTC, ePS20Only, ePS30Only, eMainInput, eSubrScript, eDeclarationsScript, eCoreScript, eSupportsInstancing, eSupportsMultiLights, eNoSpecular};
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
    {ePS20Only, "PS20Only"},
    {ePS30Only, "PS30Only"},

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
        m_InputParmsScript = CCGVProgram_GL::mfAddNewScript(name, params);
        break;

      case eDeclarationsScript:
        m_DeclarationsScript = CCGVProgram_GL::mfAddNewScript(name, params);
        break;

      case eCoreScript:
        m_CoreScript = CCGVProgram_GL::mfAddNewScript(name, params);
        break;

      case eSubrScript:
        m_SubroutinesScript = CCGVProgram_GL::mfAddNewScript(name, params);
        break;

      case eParam4f:
        mfCompileParam4f(params, ef, &m_ParamsNoObj);
        break;

      case eNoFog:
        m_Flags |= PSFI_NOFOG;
        break;

      case eAutoEnumTC:
        m_Flags |= VPFI_AUTOENUMTC;
        break;

      case ePS20Only:
        m_Flags |= PSFI_PS20ONLY;
        break;

      case ePS30Only:
        m_Flags |= PSFI_PS30ONLY;
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
    }
  }

  gRenDev->m_cEF.mfCheckObjectDependParams(&m_ParamsNoObj, &m_ParamsObj);

  return 1;
}

void CCGPShader_GL::mfSetVariables(TArray<SCGParam4f>* Vars)
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
      mfParameter4f(p, v);
    }
  }
}

void CCGPShader_GL::mfSetVariables(bool bObj, TArray<SCGParam4f>* Parms)
{
  if (m_Insts[m_CurInst].m_dwHandle <= 0)
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
    mfSetVariables((TArray<SCGParam4f> *)Parms);
}

void CCGPShader_GL::mfGetSrcFileName(char *srcname, int nSize)
{
  strncpy(srcname, gRenDev->m_cEF.m_HWPath, nSize);
  strncat(srcname, "Declarations/CGPShaders/", nSize);
  strncat(srcname, m_Name.c_str(), nSize);
  strncat(srcname, ".crycg", nSize);
}

void CCGPShader_GL::mfGetDstFileName(char *dstname, int nSize)
{
  char *type;
  if (m_Flags & PSFI_AUTOENUMTC)
    type = "GL_Auto";
  else
  if (m_CGProfileType == CG_PROFILE_ARBFP1)
    type = "ARB";
  else
  if (m_CGProfileType == CG_PROFILE_FP20)
    type = "NV";
  else
  if (m_CGProfileType == CG_PROFILE_FP30)
    type = "NV30";
  else
    type = "Unknown";

  char *fog;
  if (m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL)
    fog = "Fog";
  else
  if (m_Insts[m_CurInst].m_Mask & VPVST_NOFOG)
    fog = "NoFog";
  else
    fog = "Unknown";
  
  strncpy(dstname, gRenDev->m_cEF.m_ShadersCache, nSize);
  strncat(dstname, "CGPShaders/", nSize);
  strncat(dstname, m_Name.c_str(), nSize);
  strncat(dstname, "$", nSize);
  strncat(dstname, type, nSize);
  strncat(dstname, "$", nSize);
  strncat(dstname, fog, nSize);
  if (m_nMaskGen)
  {
    char str[32];
    sprintf(str, "(%x)", m_nMaskGen);
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

bool CCGPShader_GL::mfActivate()
{
  if (!m_Insts[m_CurInst].m_dwHandle)
  {
    if (m_Flags & PSFI_PS30ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS30)
        m_CGProfileType = CG_PROFILE_ARBFP2;
      else
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
        m_CGProfileType = CG_PROFILE_ARBFP1;
      else
        m_CGProfileType = CG_PROFILE_FP20;
    }
    else
    if (m_Flags & PSFI_PS20ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
        m_CGProfileType = CG_PROFILE_ARBFP1;
      else
        m_CGProfileType = CG_PROFILE_FP20;
    }
    else
    if (SUPPORTS_GL_NV_register_combiners)
      m_CGProfileType = CG_PROFILE_FP20;
    else
    if (SUPPORTS_GL_ARB_fragment_program)
      m_CGProfileType = CG_PROFILE_ARBFP1;

    if (m_CGProfileType == CG_PROFILE_FP20 && !SUPPORTS_GL_NV_register_combiners)
      return false;
    if (m_CGProfileType == CG_PROFILE_ARBFP1 && !SUPPORTS_GL_ARB_fragment_program)
      return false;

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
    mfGetDstFileName(namedst, 256);
    StripExtension(namedst, namedst1);
    AddExtension(namedst1, ".cgasm");
    statusdst = iSystem->GetIPak()->FOpen(namedst1, "r");
    if (statusdst == NULL)
    {
      if (CRenderer::CV_r_shadersprecache == 2)
        bCreate = true;
      else
      {
        statusdst = iSystem->GetIPak()->FOpen(namedst, "r");
        if (statusdst == NULL)
          bCreate = true;
        else
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
    if (bCreate)
    {
      char *scr;
      scr = mfCreateAdditionalPS();

      char *code = mfLoadCG(scr);
      delete [] scr;
      if (m_CGProfileType == CG_PROFILE_ARBFP1)
      {
        char *cd = (char *)code;
        while (cd)
        {
          cd = strstr(code, "program.local");
          if (cd)
          {
            memcpy(cd, "  program.env", 13);
            cd += 13;
          }
        }
      }

      if (code)
      {
        FILE *fp = fopen(namedst, "wb");
        if (fp)
        {
          if (m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL)
          {
            if (m_CGProfileType == CG_PROFILE_FP20)
            {
              char *sStr = strstr(code, "out.rgb");
              int n = 7;
              if (sStr)
              {
                char op[128];
                char newStr[128];
                int m = 0;
                while (sStr[n]==0x20 || sStr[n]==8 || sStr[n] == '=') {n++;}
                while (sStr[n]!=';') {op[m++]=sStr[n++];}
                op[m] = 0;
                int nn = n+1;
                sprintf(newStr, "final_product = %s * fog.a;\nout.rgb = unsigned_invert(fog.a) * fog.rgb + final_product;\n", op);
                int len = strlen(newStr);
                int len2 = strlen(code)+1;
                char *newScr2 = new char[len+len2];
                n = sStr-code;
                strncpy(newScr2, code, n);
                strcpy(&newScr2[n], newStr);
                strcpy(&newScr2[n+len], &code[nn+n]);
                delete [] code;
                code = newScr2;
              }
            }
            else
            if (m_CGProfileType == CG_PROFILE_ARBFP1)
            {
              char *sStr = strstr(code, "!!ARBfp1.0");
              int n = 10;
              if (sStr)
              {
                char newStr[128];
                int m = 0;
                while (sStr[n]!=0xa) {n++;}
                int nn = n+1;
                switch(gRenDev->m_FS.m_nFogMode)
                {
                  case R_FOGMODE_LINEAR:
                  default:
                    sprintf(newStr, "OPTION %s;\n", "ARB_fog_linear");
                	  break;
                  case R_FOGMODE_EXP2:
                    sprintf(newStr, "OPTION %s;\n", "ARB_fog_exp2");
                	  break;
                }
                int len = strlen(newStr);
                int len2 = strlen(code)+1;
                char *newScr2 = new char[len+len2];
                strncpy(newScr2, code, nn);
                strncpy(&newScr2[nn], newStr, len);
                strcpy(&newScr2[nn+len], &code[nn]);
                delete [] code;
                code = newScr2;
              }
            }
          }
          fputs(strVer, fp);
          fputs(code, fp);
          fclose(fp);
        }
        HANDLE hdst = CreateFile(namedst,GENERIC_WRITE,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
        FILE *hsrc = iSystem->GetIPak()->FOpen(namesrc, "r");
        writetimesrc = iSystem->GetIPak()->GetModificationTime(hsrc);
        SetFileTime(hdst,NULL,NULL,&writetimesrc);
        iSystem->GetIPak()->FClose(hsrc);
        CloseHandle(hdst);

        delete [] code;
      }
#ifdef WIN64
      iLog->LogError ("\002 Can't create CG pixel program '%s' (cache name: %s): NO CG Runtime in 64-bit version", m_Name.c_str(), namedst);
#endif
      m_Insts[m_CurInst].m_dwHandle = 0;
      statusdst = iSystem->GetIPak()->FOpen(namedst, "r");
    }
    if (!statusdst)
      return false;
    else
    {
      iSystem->GetIPak()->FSeek(statusdst, 0, SEEK_END);
      int len = iSystem->GetIPak()->FTell(statusdst);
      iSystem->GetIPak()->FSeek(statusdst, 0, SEEK_SET);
      char *pbuf = new char [len+1];
      iSystem->GetIPak()->FGets(strVer0, 128, statusdst);
      len = iSystem->GetIPak()->FRead(pbuf, 1, len, statusdst);
      pbuf[len] = 0;
      iSystem->GetIPak()->FClose(statusdst);
      statusdst = NULL;

      if (!bCreate && (m_Flags & PSFI_AUTOENUMTC))
      {
        if (strstr(pbuf, "!!ARBfp1.0"))
        {
          if (!SUPPORTS_GL_ARB_fragment_program)
          {
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_ARBFP1;
        }
        else
        {
          if (!SUPPORTS_GL_NV_register_combiners)
          {
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_FP20;
        }
      }
      RemoveCR(pbuf);
      mfLoad(pbuf);

      // parse variables and constants from CG or HLSL object code
      int nComps = 1;
      char *scr = pbuf;
      char *token;
      if (m_CGProfileType == CG_PROFILE_FP20)
        token = strtok(scr, "//");
      else
        token = strtok(scr, "#");
      while (token)
      {
        if (!strncmp(token, "var", 3))
        {
          token += 3;
          char *szType = sGetText(&token);
          char *szName = sGetText(&token);
          char *szvAttr = sGetText(&token);
          char *szhReg = sGetText(&token);
          if (szhReg)
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
            if (!szvAttr || szvAttr[0] != '$')
            {
              if (!m_Insts[m_CurInst].m_BindVars)
                m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
              SCGBind cgp;
              cgp.m_nComponents = nComps;
              cgp.m_Name = szName;
              if (!strncmp(szhReg, "COMBINER_STAGE_CONST", 20))
              {
                cgp.m_dwBind = atoi(&szhReg[20]);
                cgp.m_dwBind |= (atoi(&szhReg[22])+1) << 28;
              }
              else
              if (!strncmp(szhReg, "OFFSET_TEXTURE_MATRIX", 21))
              {
                cgp.m_dwBind = GL_OFFSET_TEXTURE_2D_MATRIX_NV;
                cgp.m_dwBind |= (atoi(&szhReg[22])+1) << 28;
              }
              else
                cgp.m_dwBind = atoi(&szhReg[2]);
              m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
            }
          }
        }
        if (m_CGProfileType == CG_PROFILE_FP20)
          token = strtok(NULL, "//");
        else
          token = strtok(NULL, "#");
      }
      SAFE_DELETE_ARRAY(pbuf);
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
    }
    mfUnbind();
  }

  if (!m_Insts[m_CurInst].m_dwHandle)
    return false;
  return true;
}

bool CCGPShader_GL::mfSet(bool bEnable, SShaderPassHW *slw, int nFlags)
{
  //PROFILE_FRAME(Shader_PShaders);

  int Mask;
  CGLRenderer *rd = gcpOGL;

  if (!bEnable)
  {
#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
      rd->Logv(SRendItem::m_RecurseLevel, "--- Reset CGPShader \"%s\"\n", m_Name.c_str());
#endif
    mfDisable();
    rd->m_RP.m_PersFlags &= ~RBPF_PS2NEEDSET;
    rd->m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;
  }
  else
  {
    if ((m_Flags & PSFI_NOFOG) || !(rd->m_Features & RFT_FOGVP) || !rd->m_FS.m_bEnable || !CRenderer::CV_r_vpfog)
      Mask = VPVST_NOFOG;
    else
      Mask = VPVST_FOGGLOBAL;
    if (rd->m_RP.m_ClipPlaneEnabled == 1)
    {
      if (rd->m_MaxActiveTexturesARB_VP > 6)
        Mask |= VPVST_CLIPPLANES3;
      else
        Mask |= VPVST_CLIPPLANES3;
    }

    int Type = mfGetCGInstanceID(Mask, 0);

#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
      rd->Logv(SRendItem::m_RecurseLevel, "--- Set CGPShader \"%s\", Mask: 0x%x (%d Type)\n", m_Name.c_str(), m_nMaskGen, Mask);
#endif

    if ((INT_PTR)m_Insts[Type].m_dwHandle == -1)
    {
      m_LastTypeVP = Mask;
      return false;
    }

    if (!m_Insts[Type].m_dwHandle)
    {
      rd->m_RP.m_CurPS = this;
      if (!mfActivate())
      {
        m_Insts[Type].m_dwHandle = -1;
        return false;
      }
      m_LastVP = NULL;
    }

    if (m_Frame != rd->m_nFrameUpdateID)
    {
      m_Frame = rd->m_nFrameUpdateID;
      rd->m_RP.m_PS.m_NumPShaders++;
    }
    if (m_LastVP != this || m_LastTypeVP != Mask)
    {
      m_LastVP = this;
      m_LastTypeVP = Mask;
      mfBind();
    }
    else
    if (m_CGProfileType == CG_PROFILE_FP20)
      rd->m_RP.m_PersFlags |= RBPF_PS1NEEDSET;
    else
      rd->m_RP.m_PersFlags |= RBPF_PS2NEEDSET;

    if (slw && slw->m_CGFSParamsNoObj)
      mfSetVariables(false, slw->m_CGFSParamsNoObj);
    else
      mfSetVariables(false, NULL);
  }
  m_CurRC = this;
  return true;
}

#ifdef WIN64
#pragma warning( pop )							//AMD Port
#endif