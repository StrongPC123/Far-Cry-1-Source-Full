/*=============================================================================
  GLCGVPrograms.cpp : OpenGL cg vertex programs support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "GLCGVProgram.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

TArray<CVProgram *> CVProgram::m_VPrograms;

vec4_t CCGVProgram_GL::m_CurParams[96];
vec4_t CCGVProgram_GL::m_CurParamsARB[96];
bool CCGVProgram_GL::m_bCreateNoise = false;

//=======================================================================

CVProgram *CVProgram::mfForName(const char *name, std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc, EShaderVersion eSHV, uint64 nMaskGen)
{
  assert(0);
  return 0;
}

//=======================================================================

CVProgram *CVProgram::mfForName(const char *name, uint64 nMask)
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
    if (!stricmp(name, m_VPrograms[i]->m_Name.c_str()) && m_VPrograms[i]->m_nMaskGen == nMask)
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
  buf = gRenDev->m_cEF.mfScriptPreprocessor(buf, dir, scrname);

  CVProgram *p = NULL;
  {
    CVProgram *pr;
    pr = new CCGVProgram_GL;
    FILE *handle = iSystem->GetIPak()->FOpen(scrname, "rb");
    if (handle)
    {
      pr->m_WriteTime = iSystem->GetIPak()->GetModificationTime(handle);
      iSystem->GetIPak()->FClose(handle);
    }
    pr->m_Name = name;
    pr->m_Id = i;
    m_VPrograms.AddElem(pr);
    pr->m_nRefCounter = 1;
    pr->m_nMaskGen = nMask;
    p = pr;
  }

  p->mfCompile(buf);
  delete [] buf;
  if (CRenderer::CV_r_shadersprecache==1)
    p->mfPrecache();

  return p;
}

//=======================================================================


TArray<SCGScript *> CCGVProgram_GL::m_CGScripts;

void SCGScript::mfRemoveFromList()
{
  int i;

  for (i=0; i<CCGVProgram_GL::m_CGScripts.Num(); i++)
  {
    SCGScript *scr = CCGVProgram_GL::m_CGScripts[i];
    if (scr == this)
      CCGVProgram_GL::m_CGScripts[i] = NULL;
  }
}

void CCGVProgram_GL::mfPrecache()
{
  bool bPrevFog = gRenDev->m_FS.m_bEnable;
  gRenDev->m_FS.m_bEnable = true;

  int Mask;
  if ((m_Flags & VPFI_NOFOG) || !(gRenDev->m_Features & RFT_FOGVP) || !gRenDev->m_FS.m_bEnable)
    Mask = VPVST_NOFOG;
  else
    Mask = VPVST_FOGGLOBAL;

  CVProgram *pPosVP = this;
  int Id = mfGetCGInstanceID(Mask, pPosVP);
  mfActivate(pPosVP);

  Mask |= VPVST_CLIPPLANES3;
  Id = mfGetCGInstanceID(Mask, pPosVP);
  mfActivate(pPosVP);

  gRenDev->m_FS.m_bEnable = bPrevFog;
}

void CCGVProgram_GL::mfReset()
{
  for (int i=0; i<m_Insts.Num(); i++)
  {
    m_CurInst = i;
    mfDel();
  }
  m_Insts.Free();
  m_dwFrame++;
}

void CCGVProgram_GL::mfFree()
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

CCGVProgram_GL::~CCGVProgram_GL()
{
  mfFree();
  CVProgram::m_VPrograms[m_Id] = NULL;
}

void CCGVProgram_GL::Release()
{
  m_nRefCounter--;
  if (!m_nRefCounter)
    delete this;
}

bool CCGVProgram_GL::mfHasPointer(ESrcPointer ePtr)
{
  for (int i=0; i<m_Pointers.Num(); i++)
  {
    if (m_Pointers[i]->ePT == ePtr)
      return true;
  }
  return false;
}

static char *sAdditionalVP[] = 
{
  {
    "return OUT;\n"
    "}\n"
  },
  {
    "OUT.FogC = dot(ModelViewProj._31_32_33_34, vPos);\n"
    "return OUT;\n"
    "}\n"
  },
  {
    "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
    "return OUT;\n"
    "}\n"
  },
  {
    "OUT.FogC = dot(ModelViewProj._31_32_33_34, vPos);\n"
    "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
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
    "return OUT;\n"
    "}\n"
  },
  {
    "OUT.Tex3.z = dot(vPos, ClipPlane);\n"
    "return OUT;\n"
    "}\n"
  }
};

SCGScript *CCGVProgram_GL::mfGenerateScriptVP(CVProgram *pPosVP)
{
  TArray<char> newScr;

  if (m_DeclarationsScript)
    newScr.Copy(m_DeclarationsScript->m_Script, strlen(m_DeclarationsScript->m_Script));

  char *szInputParms = "";
  if (m_InputParmsScript)
    szInputParms = m_InputParmsScript->m_Script;
  if (m_PosScript && m_PosScript->m_Name != m_Insts[m_CurInst].m_PosScriptName)
  {
    char str[1024];
    strcpy(str, szInputParms);
    szInputParms = str;
    if (pPosVP && pPosVP->m_bCGType)
    {
      CCGVProgram_GL *pVPGL = (CCGVProgram_GL *)pPosVP;
      for (int i=0; i<pVPGL->m_ParamsNoObj.Num(); i++)
      {
        if (!strstr(str, pVPGL->m_ParamsNoObj[i].m_Name.c_str()))
        {
          strcat(str, ", uniform float4 ");
          strcat(str, pVPGL->m_ParamsNoObj[i].m_Name.c_str());
        }
      }
      for (int i=0; i<pVPGL->m_ParamsObj.Num(); i++)
      {
        if (!strstr(str, pVPGL->m_ParamsObj[i].m_Name.c_str()))
        {
          strcat(str, ", uniform float4 ");
          strcat(str, pVPGL->m_ParamsObj[i].m_Name.c_str());
        }
      }
    }
  }

  if (m_SubroutinesScript)
    newScr.Copy(m_SubroutinesScript->m_Script, strlen(m_SubroutinesScript->m_Script));

  char str[1024];
  sprintf(str, "vertout main(appin IN, %s)\n{\n  vertout OUT;\n", szInputParms);
  newScr.Copy(str, strlen(str));

  if (pPosVP)
  {
    CCGVProgram_GL *pVP = (CCGVProgram_GL *)pPosVP;
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

  char *pExit = "return OUT;\n}";
  newScr.Copy(pExit, strlen(pExit));
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

char *CCGVProgram_GL::mfGenerateTCScript(char *Script, int nt)
{
  int tcGOLMask = VPVST_TCGOL0<<nt;
  int tcGRMMask = VPVST_TCGRM0<<nt;
  int tcGNMMask = VPVST_TCGNM0<<nt;
  int tcGSMMask = VPVST_TCGSM0<<nt;
  int tcMMask = VPVST_TCM0<<nt;
  int n, m;
  if (m_Insts[m_CurInst].m_Mask & (tcGOLMask | tcGRMMask | tcGNMMask | tcGSMMask | tcMMask))
  {
    if (!strstr(Script, "IN.TexCoord0") && !strstr(Script, "IN.baseTC"))
    {
      if (!(m_Insts[m_CurInst].m_Mask & (tcGOLMask | tcGRMMask | tcGNMMask | tcGSMMask)))
        return Script;
    }
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
        sprintf(sStr1,  "float4 tcOL = mul(%s, vPos);\n", sMTCG);
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
          sprintf(sStr, "%s %s = mul(%s, tcOL);\n", sStr1, sOTex, sMTCM);
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
          sprintf(sStr, "%s %s = tcOL;\n", sStr1, sOTex);
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
        if (!strstr(Script, ": NORMAL"))
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
              len = strlen("float3 TNormal : NORMAL;\n");
              size_t len2 = strlen(Script)+1;
              char *newScr2 = new char[len+len2];
              n = sNewStr-Script+n;
              strncpy(newScr2, Script, n);
              strcpy(&newScr2[n], "float3 TNormal : NORMAL;\n");
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

char *CCGVProgram_GL::mfCreateAdditionalVP(CVProgram *pPosVP)
{
  SCGScript *pScript = m_Script;
  if (!pScript)
    pScript = mfGenerateScriptVP(pPosVP);
  if (!pScript)
    return NULL;
  const char *scr = pScript->m_Script;
  int len, n;

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
    sNewStr += n;
  }

  n = sNewStr - pScript->m_Script;
  len = strlen(pScript->m_Script)+1;
  int lenN = strlen(sAdditionalVP[Mask&7]);
  int newSize = n+1+lenN;
  char *newScr = new char [newSize];
  strncpy(newScr, scr, n);
  strcpy(&newScr[n], sAdditionalVP[Mask&7]);

  if ((Mask & VPVST_NOISE) && !strstr(newScr, "register(c30)"))
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
        len = strlen(", uniform float4 pg[66] : register(c30)");
        int len2 = strlen(newScr)+1;
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

  if (!(Mask & VPVST_NOFOG))
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
        while (sNewStr[n]==0x20 || sNewStr[n]==8 || sNewStr[n]==0xa) {n++;}
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
  }
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

void CCGVProgram_GL::mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params)
{
  gRenDev->m_cEF.mfCompileCGParam(scr, ef, Params);
}

void CCGVProgram_GL::mfCompileParamStateMatrix(char *scr, SShader *ef, TArray<SCGMatrix> *Params)
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
  pr.m_dwBind = 0;
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
    Params->AddElem(pr);
}

void CCGVProgram_GL::mfCompileVertAttributes(char *scr, SShader *ef)
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
            iLog->Log("Warning: Missing parameters for VertArray in Shader '%s'\n", ef->m_Name.c_str());
            break;
          }
          gRenDev->m_cEF.mfCompileArrayPointer(m_Pointers, params, ef);
        }
        break;
    }
  }
}

bool CCGVProgram_GL::mfCompile(char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eScript=1, eParam4f, eParamStateMatrix, eVS20Only, eVS30Only, eAutoEnumTC, eProjected, eDefaultPos, eNoFog, eNoise, eUnifiedPosScript, eVertAttributes, eMainInput, eSubrScript, eDeclarationsScript, ePositionScript, eCoreScript, eSupportsInstancing, eSupportsMultiLights, eInst_Param4f, eNoSpecular};
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
    {eNoise, "Noise"},
    {eVS20Only, "VS20Only"},
    {eVS30Only, "VS30Only"},
    {eProjected, "Projected"},
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

      case eNoFog:
        m_Flags |= VPFI_NOFOG;
        break;

      case eNoSpecular:
        m_Flags |= VPFI_NOSPECULAR;
        break;

      case eNoise:
        m_Flags |= VPFI_NOISE;
        break;

      case eAutoEnumTC:
        m_Flags |= VPFI_AUTOENUMTC;
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
        if (m_CGProfileType == CG_PROFILE_VP20)
        {
#ifndef WIN64
					// NOTE: AMD64 port: find the 64-bit CG runtime
          m_CGProfileType = cgGLGetLatestProfile(CG_GL_VERTEX);
#endif
          if (SUPPORTS_GL_ARB_vertex_program)
            m_CGProfileType = CG_PROFILE_ARBVP1;
        }
        break;

      case eVS30Only:
        m_Flags |= VPFI_VS20ONLY;
        break;

      case eDefaultPos:
        m_Flags |= VPFI_DEFAULTPOS;
        break;

      case eUnifiedPosScript:
        m_Flags |= VPFI_UNIFIEDPOS;
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
    m_MatrixObj.AddElem(m);
    m_MatrixObj.Shrink();
  }

  gRenDev->m_cEF.mfCheckObjectDependParams(&m_ParamsNoObj, &m_ParamsObj);

  return 1;
}

void CCGVProgram_GL::mfSetVariables(TArray<SCGParam4f>* Vars)
{
  int i;

  for (i=0; i<Vars->Num(); i++)
  {
    SCGParam4f *p = &Vars->Get(i);
    if (p->m_nComponents > 1)
    {
      float matr[16][4];
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

void CCGVProgram_GL::mfSetVariables(bool bObj, TArray<SCGParam4f>* Parms)
{
  if (m_Insts[m_CurInst].m_dwHandle <= 0)
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

void CCGVProgram_GL::mfSetPointers(int nFlags)
{
  int nType = 0;
  if (nFlags & VPF_SETPOINTERSFORPASS)
    nType = 1;
  for (int i=0; i<m_Pointers.Num(); i++)
  {
    SArrayPointer *vp = m_Pointers[i];
    vp->mfSet(nType);
  }
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

void CCGVProgram_GL::mfGetSrcFileName(char *srcname, int nSize)
{
  strncpy(srcname, gRenDev->m_cEF.m_HWPath, nSize);
  strncat(srcname, "Declarations/CGVShaders/", nSize);
  strncat(srcname, m_Name.c_str(), nSize);
  strncat(srcname, ".crycg", nSize);
}

void CCGVProgram_GL::mfGetDstFileName(char *dstname, int nSize)
{
  char *type;
  if (m_Flags & VPFI_AUTOENUMTC)
    type = "GL_Auto";
  else
  if (m_CGProfileType == CG_PROFILE_VP20)
    type = "NV";
  else
  if (m_CGProfileType == CG_PROFILE_ARBVP1)
    type = "ARB";
  else
    type = "Unknown";

  char *fog;
  if (m_Insts[m_CurInst].m_Mask & VPVST_FOGGLOBAL)
    fog = "Fog";
  else
  if (m_Insts[m_CurInst].m_Mask & VPVST_NOFOG)
    fog = "NoFog";
  
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
  if (pr[0])
  {
    strncat(dstname, "$", nSize);
    strncat(dstname, pr, nSize);
  }
  strncat(dstname, "#", nSize);
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

  if (m_nMaskGen)
  {
    char str[32];
    sprintf(str, "(%x)", m_nMaskGen);
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

struct STempStr
{
  char name[128];
  int nComponents;
  int nId;
};

bool CCGVProgram_GL::mfActivate(CVProgram *pPosVP)
{
  if (!m_Insts[m_CurInst].m_dwHandle)
  {
    int VS20Profile = CG_PROFILE_ARBVP1;

    if (m_Flags & VPFI_VS30ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS30)
        m_CGProfileType = CG_PROFILE_ARBVP2;
      else
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
        m_CGProfileType = VS20Profile;
      else
        m_CGProfileType = CG_PROFILE_VP20;
    }
    else
    if (m_Flags & VPFI_VS20ONLY)
    {
      if (gRenDev->GetFeatures() & RFT_HW_PS20)
        m_CGProfileType = VS20Profile;
      else
        m_CGProfileType = CG_PROFILE_VP20;
    }
    else
    if (SUPPORTS_GL_NV_vertex_program)
      m_CGProfileType = CG_PROFILE_VP20;
    else
    if (SUPPORTS_GL_ARB_vertex_program)
      m_CGProfileType = VS20Profile;
    if (m_CGProfileType == CG_PROFILE_VP20 && !SUPPORTS_GL_NV_vertex_program)
      return false;
    if (m_CGProfileType == CG_PROFILE_ARBVP1 && !SUPPORTS_GL_ARB_vertex_program)
      return false;

    if (m_Insts[m_CurInst].m_Mask & (VPVST_INSTANCING_NOROT | VPVST_INSTANCING_ROT))
    {
      if (m_CGProfileType < CG_PROFILE_ARBVP1)
        m_CGProfileType = VS20Profile;
    }

    char strVer[128];
    char strVer0[128];
    sprintf(strVer, "//CGVER%.1f\n", CG_VP_CACHE_VER);
    bool bCreate = false;
    char namesrc[256];
    char namedst[256];
    char namedst1[256];
    FILETIME writetimesrc,writetimedst;
    FILE *statussrc, *statusdst;
    statussrc = NULL;
    statusdst = NULL;
    mfGetSrcFileName(namesrc, 256);
    CCGVProgram_GL *pVP = (CCGVProgram_GL *)pPosVP;
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
      scr = mfCreateAdditionalVP(pPosVP);

      const char *code = mfLoadCG(scr);
      delete [] scr;
      if (m_CGProfileType == CG_PROFILE_ARBVP1)
      {
        char *cd = (char *)code;
        while (cd)
        {
          cd = (char*)strstr(code, "program.local");
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
      else
      {
#ifdef WIN64
        iLog->LogError ("\002 Can't create CG vertex program '%s' (cache name: %s): NO CG Runtime in 64-bit version", m_Name.c_str(), namedst);
#endif
        m_Insts[m_CurInst].m_dwHandle = 0;
      }
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
        if (strstr(pbuf, "!!ARBvp1.0"))
        {
          if (!SUPPORTS_GL_ARB_vertex_program)
          {
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_ARBVP1;
        }
        else
        {
          if (!SUPPORTS_GL_NV_vertex_program)
          {
            bCreate = true;
            goto create;
          }
          m_CGProfileType = CG_PROFILE_VP20;
        }
      }

      RemoveCR(pbuf);
      mfLoad(pbuf);

      // parse variables and constants from CG object code
      /*if (m_CGProfileType == CG_PROFILE_ARBVP1)
      {
        char *scr = pbuf;
        char *token = strstr(scr, "PARAM");
        while (token)
        {
          int n = 0;
          while (token[n]!=0x20 && token[n]!=8) { n++; }
          while (token[n]==0x20 || token[n]==8) { n++; }
          token += n;
          if (token[0] == 'c')
          {
            int nBind = atoi(&token[1]);
            n = 0;
            while (token[n]!=0x20 && token[n]!=8) { n++; }
            while (token[n]==0x20 || token[n]==8) { n++; }
            token += n;
            if (token[0] == '=')
            {
              n = 1;
              while (token[n]==0x20 || token[n]==8) { n++; }
              token += n;
              if (token[0] == '{')
              {
                float v[4];
                n = sscanf(&token[1], "%f, %f, %f, %f", &v[0], &v[1], &v[2], &v[3]);
                if (n == 4)
                {
                  if (!m_Insts[m_CurInst].m_BindConstants)
                    m_Insts[m_CurInst].m_BindConstants = new TArray<SCGBindConst>;
                  SCGBindConst cgp;
                  cgp.m_dwBind = nBind;
                  cgp.m_nComponents = 1;
                  cgp.m_Val[0] = v[0];
                  cgp.m_Val[1] = v[1];
                  cgp.m_Val[2] = v[2];
                  cgp.m_Val[3] = v[3];
                  m_Insts[m_CurInst].m_BindConstants->AddElem(cgp);
                }
              }
            }
          }
          token = strstr(token, "PARAM");
        }
      }*/
      int nComps = 1;
      char *scr = pbuf;
      char *token = strtok(scr, "#");
      TArray<STempStr> FoundNames;
      int i;
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
          int n = 0;
          if (szName)
          {
            char *s = strchr(szName, '[');
            if (s)
              s[0] = 0;
            int i;
            for (i=0; i<FoundNames.Num(); i++)
            {
              if (!stricmp(szName, FoundNames[i].name))
              {
                FoundNames[i].nComponents = atol(&s[1])+1;
                break;
              }
            }
            if (i != FoundNames.Num())
              continue;
            STempStr name;
            strcpy(name.name, szName);
            n = FoundNames.Num();
            FoundNames.AddElem(name);
            FoundNames[n].nId = -1;
          }
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
          if (!szvAttr || szvAttr[0] != '$')
          {
            if (!m_Insts[m_CurInst].m_BindVars)
              m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;
            SCGBind cgp;
            cgp.m_nComponents = nComps;
            cgp.m_Name = szName;
            cgp.m_dwBind = atoi(&szhReg[2]);
            FoundNames[n].nId = m_Insts[m_CurInst].m_BindVars->Num();
            FoundNames[n].nComponents = nComps;
            m_Insts[m_CurInst].m_BindVars->AddElem(cgp);
          }
        }
        token = strtok(NULL, "#");
      }
      SAFE_DELETE_ARRAY(pbuf);
      for (i=0; i<FoundNames.Num(); i++)
      {
        if (FoundNames[i].nId >= 0)
          m_Insts[m_CurInst].m_BindVars->Get(FoundNames[i].nId).m_nComponents = FoundNames[i].nComponents;
      }
    }

    mfUnbind();

    if (!m_bCreateNoise)
    {
      m_bCreateNoise = true;

      // create a perlin noise permuation table
	    vec4_t c[32*2+2];
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

      if (m_CGProfileType == CG_PROFILE_ARBVP1)
      {
	      for(i=0; i<64; i++) 
	      {
          glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 30+i, &c[i][0]);
	      }
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 30+i,   &c[0][0]);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 30+i+1, &c[1][0]);
        glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, VSCONST_0_025_05_1, 0, 0.25f, 0.5f, 1.0f);
      }
      else
      {
        for(i=0; i<64; i++) 
        {
          glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, 30+i, &c[i][0]);
        }
        glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, 30+i,   &c[0][0]);
        glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, 30+i+1, &c[1][0]);
        glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, VSCONST_0_025_05_1, 0, 0.25f, 0.5f, 1.0f);
      }
    }
  }

  if (!m_Insts[m_CurInst].m_dwHandle)
    return false;
  return true;
}

void CCGVProgram_GL::mfSetGlobalParams()
{
  vec4_t v;
  int n;
  CGLRenderer *r = gcpOGL;
  if (!(r->m_Features & RFT_HW_VS))
    return;

  v[0] = 0; v[1] = 0.25f; v[2] = 0.5f; v[3] = 1.0f;
  n = VSCONST_0_025_05_1;
  //if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
  {
    m_CurParams[n][0] = v[0];
    m_CurParams[n][1] = v[1];
    m_CurParams[n][2] = v[2];
    m_CurParams[n][3] = v[3];
    if (SUPPORTS_GL_ARB_vertex_program)
      glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, n, m_CurParams[n]);
    else
      glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, n, m_CurParams[n]);
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
      if (SUPPORTS_GL_ARB_fragment_program)
        glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, n, m_CurParams[n]);
    }
  }
}

bool CCGVProgram_GL::mfSet(bool bEnable, SShaderPassHW *slw, int nFlags)
{
  //PROFILE_FRAME(Shader_VShaders);

  int Mask;
  CGLRenderer *rd = gcpOGL;

  if (!bEnable)
  {
#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log == 4)
      rd->Logv(SRendItem::m_RecurseLevel, "--- Reset CGVProgram \"%s\"\n", m_Name.c_str());
#endif
    mfDisable();
    rd->m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
  }
  else
  {
    if ((m_Flags & VPFI_NOFOG) || !(rd->m_Features & RFT_FOGVP) || !rd->m_FS.m_bEnable || !CRenderer::CV_r_vpfog)
      Mask = VPVST_NOFOG;
    else
      Mask = VPVST_FOGGLOBAL;
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
      if (gRenDev->m_RP.m_RendPass)
        pVP = rd->m_RP.m_LastVP;
      else
      if (gRenDev->m_RP.m_pShader->m_DefaultVProgram)
        pVP = rd->m_RP.m_pShader->m_DefaultVProgram;
      if (pVP && pVP->m_bCGType)
      {
        pPosVP = pVP;
        if (pVP->m_Flags & VPFI_NOISE)
          Mask |= VPVST_NOISE;
      }
    }

    int Type = mfGetCGInstanceID(Mask, pPosVP);

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log >= 3)
    rd->Logv(SRendItem::m_RecurseLevel, "--- Set CGVProgram \"%s\", Mask: 0x%x (%d Type)\n", m_Name.c_str(), m_nMaskGen, Mask);
#endif

    if ((int)m_Insts[Type].m_dwHandle == -1)
    {
      m_LastTypeVP = Mask;
      return false;
    }

    if (!m_Insts[Type].m_dwHandle)
    {
      rd->m_RP.m_CurVP = this;
      if (!mfActivate(pPosVP))
      {
        m_Insts[Type].m_dwHandle = -1;
        return false;
      }
      m_LastVP = NULL;
    }

    if (m_Frame != rd->m_nFrameUpdateID)
    {
      m_Frame = rd->m_nFrameUpdateID;
      rd->m_RP.m_PS.m_NumVShaders++;
    }
    if (m_LastVP != this || m_LastTypeVP != Mask)
    {
      m_LastVP = this;
      m_LastTypeVP = Mask;
      mfBind();
    }
    rd->m_RP.m_PersFlags |= RBPF_VSNEEDSET;
    if (nFlags & (VPF_SETPOINTERSFORSHADER | VPF_SETPOINTERSFORPASS))
      mfSetPointers(nFlags);

    if (!(nFlags & VPF_DONTSETMATRICES))
      mfSetStateMatrices();

    if (!(m_Flags & VPFI_UNIFIEDPOS))
      rd->m_RP.m_LastVP = this;
  }
  return true;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif
