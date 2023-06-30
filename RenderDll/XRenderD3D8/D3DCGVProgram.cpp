/*=============================================================================
  D3DCGVPrograms.cpp : Direct3D cg vertex programs support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"
#include "D3DCGVProgram.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

TArray<CVProgram *> CVProgram::m_VPrograms;

//=======================================================================

CVProgram *CVProgram::mfForName(char *name, bool bCGType)
{
  int i;

  if (!name || !name[0])
    return NULL;

  for (i=0; i<m_VPrograms.Num(); i++)
  {
    if (!m_VPrograms[i])
      continue;
    if (!stricmp(name, m_VPrograms[i]->m_Name.c_str()))
    {
      m_VPrograms[i]->m_RefCounter++;
      return m_VPrograms[i];
    }
  }

  char scrname[128];
  char dir[128];
  if (!bCGType)
  {
    iLog->Log("Error: Attempt to load assembly vertex shader '%s'", name);
    iLog->Log("Error: Assembly vertex shaders aren't supported anymore");
    return NULL;
  }
  else
  {
    sprintf(dir, "%sDeclarations/CGVShaders/", gcEf.m_HWPath);
    sprintf(scrname, "%s%s.crycg", dir, name);
  }
  FILE *fp = iSystem->GetIPak()->FOpen(scrname, "r");
  if (!fp)
  {
    iLog->Log("WARNING: Couldn't find vertex shader '%s'", name);
    return NULL;
  }
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
  int len = iSystem->GetIPak()->FTell(fp);
  char *buf = new char [len+1];
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
  len = iSystem->GetIPak()->FRead(buf, 1, len, fp);
  iSystem->GetIPak()->FClose(fp);
  buf[len] = 0;
  if (bCGType)
    buf = gcEf.mfScriptPreprocessor(buf, dir, scrname);

  CVProgram *p = NULL;
  {
    CVProgram *pr = new CCGVProgram_D3D;
    HANDLE statussrc = CreateFile(scrname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (statussrc != INVALID_HANDLE_VALUE)
    {
      GetFileTime(statussrc,NULL,NULL,&pr->m_WriteTime);
      CloseHandle(statussrc);
    }
    pr->m_Name = name;
    pr->m_Id = i;
    m_VPrograms.AddElem(pr);
    pr->m_RefCounter = 1;
    p = pr;
  }

  p->mfCompile(buf);
  delete [] buf;

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
  for (int i=0; i<m_Insts.Num(); i++)
  {
    m_CurInst = i;
    mfDel();
  }
  m_Insts.Free();
}

void CCGVProgram_D3D::mfFree()
{
  m_Flags = 0;

  if (m_Script && !m_Script->m_Name.GetIndex())
    SAFE_RELEASE(m_Script);
  if (m_CoreScript && !m_CoreScript->m_Name.GetIndex())
    SAFE_RELEASE(m_CoreScript);
  if (m_DeclarationsScript && !m_DeclarationsScript->m_Name.GetIndex())
    SAFE_RELEASE(m_DeclarationsScript);
  if (m_InputParmsScript && !m_InputParmsScript->m_Name.GetIndex())
    SAFE_RELEASE(m_InputParmsScript);
  if (m_PosScript && !m_PosScript->m_Name.GetIndex())
    SAFE_RELEASE(m_PosScript);

  m_Pointers.Free();
  m_ParamsNoObj.Free();
  m_ParamsObj.Free();
  m_MatrixNoObj.Free();
  m_MatrixObj.Free();

  mfReset();
}

CCGVProgram_D3D::~CCGVProgram_D3D()
{
  mfFree();
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

static char *sAdditionalVP[] = 
{
  {
    "OUT.FogC = IN.Fog.x;\n"
    "return OUT;\n"
    "}\n"
  },
  {
    "float fCameraSpacePosZ = dot(ModelViewProj._31_32_33_34, vPos);\n"
    "OUT.FogC = vPos.w - ((Fog.x - fCameraSpacePosZ) * Fog.y);\n"
    "return OUT;\n"
    "}\n"
  },
  {
    "OUT.FogC = IN.Fog.x;\n"
    "OUT.Tex3.w = dot(IN.Position, ClipPlane);\n"
    "OUT.Tex3.xyz = IN.Position.w;\n"
    "return OUT;\n"
    "}\n"
  },
  {
    "float fCameraSpacePosZ = dot(ModelViewProj._31_32_33_34, vPos);\n"
    "OUT.FogC = vPos.w - ((Fog.x - fCameraSpacePosZ) * Fog.y);\n"
    "OUT.Tex3.w = dot(IN.Position, ClipPlane);\n"
    "OUT.Tex3.xyz = IN.Position.w;\n"
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
    "OUT.Tex3.w = dot(IN.Position, ClipPlane);\n"
    "OUT.Tex3.xyz = IN.Position.w;\n"
    "return OUT;\n"
    "}\n"
  },
  {
    "OUT.Tex3.w = dot(IN.Position, ClipPlane);\n"
    "OUT.Tex3.xyz = IN.Position.w;\n"
    "return OUT;\n"
    "}\n"
  }
};

SCGScript *CCGVProgram_D3D::mfGenerateScriptVP(SCGScript *pPosScript)
{
  TArray<char> newScr;

  if (m_DeclarationsScript)
    newScr.Copy(m_DeclarationsScript->m_Script, strlen(m_DeclarationsScript->m_Script));

  char *szInputParms = m_InputParmsScript->m_Script;
  if (pPosScript && m_PosScript->m_Name != m_Insts[m_CurInst].m_PosScriptName)
  {
    char str[1024];
    strcpy(str, szInputParms);
    szInputParms = str;
    if (gRenDev->m_RP.m_pShader->m_DefaultVProgram && gRenDev->m_RP.m_pShader->m_DefaultVProgram->m_bCGType)
    {
      CCGVProgram_D3D *pVPD3D = (CCGVProgram_D3D *)gRenDev->m_RP.m_pShader->m_DefaultVProgram;
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

  char str[512];
  sprintf(str, "vertout main(appin IN, %s)\n{\n  vertout OUT;\n", szInputParms);
  newScr.Copy(str, strlen(str));

  if (pPosScript)
    newScr.Copy(pPosScript->m_Script, strlen(pPosScript->m_Script));

  if (m_CoreScript)
    newScr.Copy(m_CoreScript->m_Script, strlen(m_CoreScript->m_Script));

  char *pExit = "return OUT;\n}";
  newScr.Copy(pExit, strlen(pExit));
  newScr.AddElem(0);

  SCGScript *cgs = mfAddNewScript(NULL, &newScr[0]);

  return cgs;
}

void CCGVProgram_D3D::mfPrecache()
{
  int bPrevCP = gRenDev->m_RP.m_ClipPlaneEnabled;
  gRenDev->m_RP.m_ClipPlaneEnabled = false;
  mfSet(true, 0);
  gRenDev->m_RP.m_ClipPlaneEnabled = true;
  mfSet(true, 0);
  gRenDev->m_RP.m_ClipPlaneEnabled = bPrevCP;
  mfSet(false, 0);
}

char *CCGVProgram_D3D::mfCreateAdditionalVP(SCGScript *pPosScript)
{
  SCGScript *pScript   = m_Script;
  if (!pScript)
    pScript = mfGenerateScriptVP(pPosScript);
  if (!pScript)
    return NULL;
  const char *scr = pScript->m_Script;
  int len, n;

  char *sNewStr = pScript->m_Script;
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
    while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=0xa) {n++;}
    while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
    if (!strncmp(&sNewStr[n], "OUT", 3))
      break;
  }

  n = sNewStr - pScript->m_Script;
  len = strlen(pScript->m_Script)+1;
  int lenN = strlen(sAdditionalVP[m_Insts[m_CurInst].m_Mask]);
  int newSize = n+1+lenN;
  char *newScr = new char [newSize];
  strncpy(newScr, scr, n);
  strcpy(&newScr[n], sAdditionalVP[m_Insts[m_CurInst].m_Mask]);

  if ((m_Insts[m_CurInst].m_Mask & VPVST_NOISE) && !strstr(newScr, "register(c32)"))
  {
    sNewStr = newScr;
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
        len = strlen(", uniform float4 pg[64] : register(c32)");
        int len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], ", uniform float4 pg[64] : register(c32)");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
        break;
      }
    }
  }

  if (m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL)
  {
    sNewStr = newScr;
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
        len = strlen(", uniform float3 Fog");
        int len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], ", uniform float3 Fog");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
        break;
      }
    }
  }

  if (!(m_Insts[m_CurInst].m_Mask & VPVST_NOFOG))
  {
    sNewStr = newScr;
    while (true)
    {
      sNewStr = strstr(sNewStr, "vertout");
      if (!sNewStr)
        break;
      n = 0;
      while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=0xa && sNewStr[n] != '{') {n++;}
      while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
      if (sNewStr[n] == '{')
      {
        n++;
        while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xd || sNewStr[n]==0xa) {n++;}
        len = strlen("float FogC : FOG;\n");
        int len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], "float FogC : FOG;\n");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
        break;
      }
    }
  }

  if (m_Insts[m_CurInst].m_Mask & VPVST_CLIPPLANES)
  {
    sNewStr = newScr;
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
        len = strlen(", uniform float4 ClipPlane");
        int len2 = strlen(newScr)+1;
        char *newScr2 = new char[len+len2];
        n = sNewStr-newScr+n;
        strncpy(newScr2, newScr, n);
        strcpy(&newScr2[n], ", uniform float4 ClipPlane");
        strcpy(&newScr2[n+len], &newScr[n]);
        delete [] newScr;
        newScr = newScr2;
        break;
      }
    }
    sNewStr = newScr;
    if (!strstr(sNewStr, "TEX3"))
    {
      while (true)
      {
        sNewStr = strstr(sNewStr, "vertout");
        if (!sNewStr)
          break;
        n = 0;
        while (sNewStr[n]!=0x20 && sNewStr[n]!=8 && sNewStr[n]!=0xa && sNewStr[n] != '{') {n++;}
        while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
        if (sNewStr[n] == '{')
        {
          n++;
          while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
          len = strlen("float4 Tex3 : TEXCOORD3;\n");
          int len2 = strlen(newScr)+1;
          char *newScr2 = new char[len+len2];
          n = sNewStr-newScr+n;
          strncpy(newScr2, newScr, n);
          strcpy(&newScr2[n], "float4 Tex3 : TEXCOORD3;\n");
          strcpy(&newScr2[n+len], &newScr[n]);
          delete [] newScr;
          newScr = newScr2;
          break;
        }
      }
    }
  }

  return newScr;
}

void CCGVProgram_D3D::mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params, int nLights)
{
  guard(CCGVProgram_D3D::mfCompileParam4f);

  gcEf.mfCompileCGParam(scr, ef, Params, nLights);

  unguard;
}

void CCGVProgram_D3D::mfCompileParamStateMatrix(char *scr, SShader *ef, TArray<SCGMatrix> *Params)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  guard(CCGVProgram_D3D::mfCompileParamStateMatrix);

  enum {eType=1, eName};
  static tokenDesc commands[] =
  {
    {eType, "Type"},
    {eName, "Name"},

    {0,0}
  };

  SCGMatrix pr;
  pr.m_pBind = NULL;
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

  unguard;
}

void CCGVProgram_D3D::mfCompileVertAttributes(char *scr, SShader *ef)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  guard(CCGVProgram_D3D::mfCompileVertAttributes);

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
            iLog->Log("Warning: Missing parameters for VertArray in Shader '%s'\n", ef->m_Name.c_str());
            break;
          }
          gcEf.mfCompileArrayPointer(m_Pointers, params, ef);
        }
        break;
    }
  }
  unguard;
}

bool CCGVProgram_D3D::mfCompile(char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  guard(CCGVProgram_D3D::mfCompile);

  enum {eScript=1, eParam4f, eVertAttributes, eParamStateMatrix, eNoFog, eNoise, eUnifiedPosScript, eMainInput, eDeclarationsScript, ePositionScript, eCoreScript};
  static tokenDesc commands[] =
  {
    {eScript, "Script"},
    {eParam4f, "Param4f"},
    {eParamStateMatrix, "ParamStateMatrix"},
    {eVertAttributes, "VertAttributes"},
    {eMainInput, "MainInput"},
    {eDeclarationsScript, "DeclarationsScript"},
    {ePositionScript, "PositionScript"},
    {eCoreScript, "CoreScript"},
    {eNoFog, "NoFog"},
    {eNoise, "Noise"},
    {eUnifiedPosScript, "UnifiedPosScript"},

    {0,0}
  };

  mfFree();

  SShader *ef = gcEf.m_CurShader;
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

      case eParam4f:
        mfCompileParam4f(params, ef, &m_ParamsNoObj, -1);
        break;

      case eParamStateMatrix:
        mfCompileParamStateMatrix(params, ef, &m_MatrixNoObj);
        break;

      case eUnifiedPosScript:
        m_Flags |= VPFI_UNIFIEDPOS;
        break;

      case eNoFog:
        m_Flags |= VPFI_NOFOG;
        break;

      case eNoise:
        m_Flags |= VPFI_NOISE;
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
  for (i=0; i<m_MatrixNoObj.Num(); i++)
  {
    SCGMatrix *m = &m_MatrixNoObj[i];
    if (m->m_eCGParamType == ECGP_Matr_ViewProj)
      break;
  }
  if (i == m_MatrixNoObj.Num())
  {
    SCGMatrix m;
    m.m_eCGParamType = ECGP_Matr_ViewProj;
    m.m_Name = "ModelViewProj";
    m_MatrixNoObj.AddElem(m);
  }

  unguard;

  return 1;
}

void CCGVProgram_D3D::mfSetVariables(TArray<SCGParam4f>* Vars)
{
  int i;

  for (i=0; i<Vars->Num(); i++)
  {
    SCGParam4f *p = &Vars->Get(i);
    if (p->m_nComponents > 1)
    {
      float matr[4][4];
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
      continue;
    }

    float *v = p->mfGet();
    mfParameter4f(p, v);
  }
}

void CCGVProgram_D3D::mfSetVariables(bool bObj, TArray<SCGParam4f>* Parms)
{
  if (m_Insts[m_CurInst].m_dwHandle == 0 || m_Insts[m_CurInst].m_dwHandle == -1)
    return;

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

void CCGVProgram_D3D::mfGetSrcFileName(char *srcname)
{
  sprintf(srcname, "%sDeclarations/CGVShaders/%s.crycg", gcEf.m_HWPath, m_Name.c_str());
}

void CCGVProgram_D3D::mfGetDstFileName(char *dstname, SCGScript *pPosScr)
{
  char *type;
#ifndef _XBOX
  if (m_CGProfileType == CG_PROFILE_VS_1_1)
    type = "D3D8_VS11";
  else
  if (m_CGProfileType == CG_PROFILE_VS_2_0)
    type = "D3D8_VS20";
  else
  if (m_CGProfileType == CG_PROFILE_VS_2_X)
    type = "D3D8_VS2X";
#else
    type = "D3D8_VS11";
#endif

  char *fog;
  if (m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL)
    fog = "Fog";
  else
  if (m_Insts[m_CurInst].m_Mask & VPVST_NOFOG)
    fog = "NoFog";
  
  char *cp;
  if (m_Insts[m_CurInst].m_Mask & VPVST_CLIPPLANES)
    cp = "CP";
  else
    cp = "NoCP";
  const char *szPos = pPosScr ? pPosScr->m_Name.c_str() : "None";
  sprintf(dstname, "%sDeclarations/CGVShaders/Cache/%s$%s$%s$%s#%s.cgvp", gcEf.m_HWPath, m_Name.c_str(), type, fog, cp, szPos);
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

bool CCGVProgram_D3D::mfActivate(CCGVProgram_D3D *pPosVP)
{
  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
  if (!m_Insts[m_CurInst].m_dwHandle)
  {
    bool bCreate = false;
    char namesrc[256];
    char namedst[256];
    char strVer[128];
    char strVer0[128];
    sprintf(strVer, "//CGVER%.1f\n", CG_CACHE_VER);
    mfGetSrcFileName(namesrc);
    mfGetDstFileName(namedst, pPosVP->m_PosScript);
#ifndef _XBOX
    FILETIME writetimesrc,writetimedst;
    HANDLE statussrc, statusdst;
    statusdst = CreateFile(namedst,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (statusdst == INVALID_HANDLE_VALUE)
      bCreate = true;
    else
    {
      statussrc = CreateFile(namesrc,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

      GetFileTime(statussrc,NULL,NULL,&writetimesrc);
      GetFileTime(statusdst,NULL,NULL,&writetimedst);
      CloseHandle(statussrc);
      CloseHandle(statusdst);
      if (CompareFileTime(&writetimesrc,&writetimedst)!=0)
        bCreate = true;
      FILE *fp = iSystem->GetIPak()->FOpen(namedst, "r");
      if (!fp)
        bCreate = true;
      else
      {
        iSystem->GetIPak()->FGets(strVer0, 128, fp);
        iSystem->GetIPak()->FClose(fp);
        if (strcmp(strVer, strVer0))
          bCreate = true;
      }
    }
    if (bCreate)
    {
      char *scr;
      scr = mfCreateAdditionalVP(pPosVP->m_PosScript);

      const char *code = mfLoadCG(scr);
      delete [] scr;

      if (code)
      {
        FILE *fp = fopen(namedst, "w");
        if (fp)
        {
          fprintf(fp, "%s%s", strVer, code);
          fclose(fp);
        }
        statusdst = CreateFile(namedst,GENERIC_WRITE,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
        statussrc = CreateFile(namesrc,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
        GetFileTime(statussrc,NULL,NULL,&writetimesrc);
        SetFileTime(statusdst,NULL,NULL,&writetimesrc);
        CloseHandle(statussrc);
        CloseHandle(statusdst);
      }
      if (m_Insts[m_CurInst].m_CGProgram)
        cgDestroyProgram(m_Insts[m_CurInst].m_CGProgram);
      m_Insts[m_CurInst].m_CGProgram = NULL;
    }
#endif
    FILE *fp = iSystem->GetIPak()->FOpen(namedst, "r");
    if (!fp)
      iLog->Log("Error: couldn't open for read vertex shader file '%s'", namedst);
    else
    {
      iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
      int len = iSystem->GetIPak()->FTell(fp);
      iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
      char *pbuf = new char [len+1];
      iSystem->GetIPak()->FGets(strVer0, 128, fp);
      len = iSystem->GetIPak()->FRead(pbuf, 1, len, fp);
      pbuf[len] = 0;
      iSystem->GetIPak()->FClose(fp);

#ifdef _XBOX
      char *s = pbuf;
      while (s)
      {
        s = strstr(s, ", v");
        if (s)
        {
          char dig[3];
          dig[0] = s[3];
          if (isdigit(s[4]))
          {
            dig[1] = s[4];
            dig[2] = 0;
          }
          else
            dig[1] = 0;
          int n = atoi(dig);
          switch(n)
          {
            case 3:
              s[3] = '2';
          	  break;
            case 5:
              s[3] = '3';
          	  break;
            case 6:
              s[3] = '4';
              break;
            case 7:
              s[3] = '9';
          	  break;
            case 8:
              s[1] = 'v';
              s[2] = '1';
              s[3] = '0';
              break;
            case 9:
              s[1] = 'v';
              s[2] = '1';
			        s[3] = '1';
              break;
			      case 0:
			      case 1:
			      case 2:
              break;
            case 10:
              s[3] = '1';
              s[4] = '2';
              break;
            default:
              assert(0);
          }
          s = &s[4];
        }
      }
#endif
      mfLoad(pbuf);

      // parse variables and constants from CG object code
      int nComps = 1;
      char *scr = pbuf;
      char *token = strtok(scr, "//");
      while (token)
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
          if (!szvAttr)
          {
            if (!m_Insts[m_CurInst].m_BindVars)
              m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
            SCGBind cgp;
            cgp.m_nComponents = nComps;
            cgp.m_Name = szName;
            cgp.m_dwBind = atoi(&szhReg[2]);
            m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
          }
        }
        token = strtok(NULL, "//");
      }
    }

    mfUnbind();

    if (!m_bCreateNoise)
    {
      m_bCreateNoise = true;

      // create a table containing a random permuation of the integers from 0-63

      vec4_t c[64];
      int i;

      // want reproducable behaviour for debug
      //srand(1);

      for(i=0; i<64; i++) 
      {
        c[i][3] = (float)i;
        c[i][0] = frand();
        c[i][1] = frand();
        c[i][2] = frand();
      }

      for(i=0; i<500; i++) 
      {
        // choose two entries at random and swap
        int a, b;
        float t;
        a = (rand() >> 4) & 63;		// lower bits of rand() are bogus sometimes
        b = (rand() >> 4) & 63;
        t = c[a][3];
        c[a][3] = c[b][3];
        c[b][3] = t;
      }

      for(i=0; i<64; i++) 
      {
        dv->SetVertexShaderConstant(32+i, &c[i][0], 1);
      }
      float cons[4];
      cons[0] = 0; cons[1] = 0.25f; cons[2] = 0.5f; cons[3] = 1.0f;
      dv->SetVertexShaderConstant(28, cons, 1);
    }
  }

  if (!m_Insts[m_CurInst].m_dwHandle)
    return false;
  return true;
}

bool CCGVProgram_D3D::mfSet(bool bStat, SShaderPassHW *slw, int nSetPointers)
{
  if (!bStat)
  {
    if (CRenderer::CV_r_log == 4)
      gRenDev->Logv(SRendItem::m_RecurseLevel, "--- Reset VProgram \"%s\"\n", m_Name.c_str());
    m_LastVP = NULL;
    return true;
  }

  gRenDev->m_RP.m_CurVP = this;

  int CurV = gRenDev->m_RP.m_CurD3DVFormat;
  int Num;
  if ((m_Flags & VPFI_NOFOG) || !CRenderer::CV_r_vpfog || (gRenDev->GetFeatures() & RFT_HW_RADEON8500))
    Num = VPVST_NOFOG;
  else
    Num = VPVST_FOGGLOBAL;
  if (gRenDev->m_RP.m_ClipPlaneEnabled == 1)
    Num |= VPVST_CLIPPLANES;
  //Num = VPVST_NOFOG;

  int Streams = gRenDev->m_RP.m_FlagsModificators & 7;

  CCGVProgram_D3D *pPosVP = this;
  if (m_Flags & VPFI_UNIFIEDPOS)
  {
    CVProgram *pVP;
    if (gRenDev->m_RP.m_RendPass)
      pVP = gRenDev->m_RP.m_LastVP;
    else
      pVP = gRenDev->m_RP.m_pShader->m_DefaultVProgram;
    if (pVP && pVP->m_bCGType)
    {
      CCGVProgram_D3D *pVPD3D = (CCGVProgram_D3D *)pVP;
      if (pVPD3D->m_PosScript)
        pPosVP = pVPD3D;
    }
  }

  if (CRenderer::CV_r_log == 4)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "--- Set VProgram \"%s\" (%d Type)\n", m_Name.c_str(), Num);

  int Id = mfGetCGInstanceID(Streams, CurV, Num, pPosVP);
  m_LastTypeVP = Id;

#ifndef _XBOX
  if ((int)m_Insts[Id].m_CGProgram == -1)
#else
  if (m_Insts[Id].m_dwHandle == -1)
#endif
    return false;

#ifndef _XBOX
  if (!m_Insts[Id].m_CGProgram)
#else
  if (!m_Insts[Id].m_dwHandle)
#endif
  {
    if (!mfActivate(pPosVP))
    {
      m_Insts[Id].m_dwHandle = -1;
      return false;
    }
    m_LastVP = NULL;
  }

  if (m_Frame != gRenDev->GetFrameID())
  {
    m_Frame = gRenDev->GetFrameID();
    gRenDev->m_RP.m_PS.m_NumVShaders++;
  }
  m_LastVP = m_Id;

  // select the vertex shader
  //if (m_LastVP != m_Id || m_LastTypeVP != Num)
  {
    //m_LastTypeVP = Num;
    mfBind();
  }

  gRenDev->m_RP.m_PersFlags |= RBPF_VSNEEDSET;
  mfSetStateMatrices(false);

  if (slw && slw->m_VPParamsNoObj.Num())
    mfSetVariables(false, &slw->m_VPParamsNoObj);
  else
    mfSetVariables(false, NULL);

  if (!(m_Flags & VPFI_UNIFIEDPOS))
    gRenDev->m_RP.m_LastVP = this;

  return true;
}
