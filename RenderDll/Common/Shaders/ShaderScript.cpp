/*=============================================================================
  ShaderScript.cpp : loading/reloading/hashing of shader scipts.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "I3DEngine.h"
#include "CryHeaders.h"
#include "../RendElements/CREScreenCommon.h"

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#include <io.h>
#endif

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

extern char *gShObjectNotFound;

//=================================================================================================

void CShader::mfRefreshLayer(SShaderPass *sl, SShader *sh)
{
  int i;
  STexPic *tp;

  for (i=0; i<sl->m_TUnits.Num(); i++)
  {
    SShaderTexUnit *tl = &sl->m_TUnits[i];
    if (!(tp=tl->m_TexPic) || !tp->m_bBusy || tp->m_LoadFrame == gRenDev->GetFrameID())
      continue;
    tp->m_LoadFrame = gRenDev->GetFrameID();
    if (tp->m_Bind >= TX_FIRSTBIND)
      gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
  }
}

void CPShader::mfReload(int nFlags)
{
  if (m_FrameLoad == gRenDev->GetFrameID())
    return;
  m_FrameLoad = gRenDev->GetFrameID();

  char name[256];
  char dir[256];

  if (!m_bCGType)
  {
    sprintf(dir, "%sDeclarations/PShaders/", gRenDev->m_cEF.m_HWPath);
    sprintf(name, "%s.cryps", m_Name.c_str());
  }
  else
  {
    sprintf(dir, "%sDeclarations/CGPShaders/", gRenDev->m_cEF.m_HWPath);
    sprintf(name, "%s.crycg", m_Name.c_str());
  }
  mfReloadScript(dir, name, nFlags, m_nMaskGen);
}

void CPShader::mfReloadScript(const char *szPath, const char *szName, int nFlags, uint64 nMaskGen)
{
#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)

  const char *ext = GetExtension(szName);
  char name1[256], name2[256];
  char resName[256];
  int i;
  bool bCG = false;
  if (!stricmp(ext, ".crycg"))
  {
    sprintf(name1, "%sDeclarations/CGPShaders/", gRenDev->m_cEF.m_HWPath);
    bCG = true;
  }
  else
  if (!stricmp(ext, ".cryps"))
  {
    sprintf(name1, "%sDeclarations/PShaders/", gRenDev->m_cEF.m_HWPath);
    bCG = false;
  }
  else
    return;

  strcpy(name2, szPath);
  ConvertDOSToUnixName(name1, name1);
  ConvertDOSToUnixName(name2, name2);
  int len = strlen(name2);
  if (name2[len-1] != '/')
  {
    name2[len-1] = '/';
    name2[len] = 0;
  }
  if (!stricmp(name1, name2))
  {
    char name[256];
    StripExtension(szName, name);
    for (i=0; i<m_PShaders.Num(); i++)
    {
      if (!m_PShaders[i])
        continue;
      if (!stricmp(name, m_PShaders[i]->m_Name.c_str()))
      {
        if (nMaskGen != -1)
        {
          if (nMaskGen != m_PShaders[i]->m_nMaskGen)
            continue;
        }
        CPShader *ps = m_PShaders[i];
        strcpy(resName, name1);
        strcat(resName, szName);
        FILETIME writetime;
        FILE *statussrc = iSystem->GetIPak()->FOpen(resName, "rb");
        if (statussrc)
        {
          writetime = iSystem->GetIPak()->GetModificationTime(statussrc);
          iSystem->GetIPak()->FClose (statussrc);
          if (CompareFileTime(&writetime, &ps->m_WriteTime)==0)
            continue;
          ps->m_WriteTime = writetime;
        }
        iLog->Log("Reload pixel shader '%s' (%I64x)", name, ps->m_nMaskGen);
        ps->mfReset();
        FILE *fp = iSystem->GetIPak()->FOpen(resName, "rb");
        if (!fp)
        {
          Warning( 0,0,"WARNING: Couldn't find pixel shader '%s'", resName);
          continue;
        }
        iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
        int len = iSystem->GetIPak()->FTell(fp);
        char *buf = new char [len+1];
        iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
        len = iSystem->GetIPak()->FRead(buf, 1, len, fp);
        iSystem->GetIPak()->FClose(fp);
        buf[len] = 0;
        gRenDev->m_cEF.m_Macros = ps->m_Macros;
        buf = gRenDev->m_cEF.mfScriptPreprocessor(buf, name2, szName);
        ps->mfCompile(buf);
        delete [] buf;
        if (nMaskGen != -1)
          return;
      }
    }
  }
#endif
}

void CVProgram::mfReload(int nFlags)
{
  if (m_FrameLoad == gRenDev->GetFrameID())
    return;
  m_FrameLoad = gRenDev->GetFrameID();

  char name[256];
  char dir[256];

  sprintf(dir, "%sDeclarations/CGVShaders/", gRenDev->m_cEF.m_HWPath);
  sprintf(name, "%s.crycg", m_Name.c_str());
  CVProgram::mfReloadScript(dir, name, nFlags, m_nMaskGen);
}

void CVProgram::mfReloadScript(const char *szPath, const char *szName, int nFlags, uint64 nMaskGen)
{
#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  const char *ext = GetExtension(szName);
  char name1[256], name2[256], resName[256];
  int i;
  if (!stricmp(ext, ".crycg"))
    sprintf(name1, "%sDeclarations/CGVShaders/", gRenDev->m_cEF.m_HWPath);
  else
    return;

  strcpy(name2, szPath);
  ConvertDOSToUnixName(name1, name1);
  ConvertDOSToUnixName(name2, name2);
  int len = strlen(name2);
  if (name2[len-1] != '/')
  {
    name2[len-1] = '/';
    name2[len] = 0;
  }
  if (!stricmp(name1, name2))
  {
    char name[256];
    StripExtension(szName, name);
    for (i=0; i<m_VPrograms.Num(); i++)
    {
      if (!m_VPrograms[i])
        continue;
      if (!stricmp(name, m_VPrograms[i]->m_Name.c_str()))
      {
        if (nMaskGen != -1)
        {
          if (nMaskGen != m_VPrograms[i]->m_nMaskGen)
            continue;
        }
        CVProgram *vp = m_VPrograms[i];
        strcpy(resName, name1);
        strcat(resName, szName);
        FILETIME writetime;
        FILE *statussrc = iSystem->GetIPak()->FOpen(resName, "rb");
        if (statussrc)
        {
          writetime = iSystem->GetIPak()->GetModificationTime(statussrc);
          iSystem->GetIPak()->FClose (statussrc);
          if (CompareFileTime(&writetime, &vp->m_WriteTime) == 0)
            continue;
          vp->m_WriteTime = writetime;
        }
        iLog->Log("Reload vertex shader '%s' (%I64x)", name, vp->m_nMaskGen);
        vp->mfReset();
        FILE *fp = iSystem->GetIPak()->FOpen(resName, "rb");
        if (!fp)
        {
          Warning( 0,0,"WARNING: Couldn't find vertex shader '%s'", resName);
          continue;
        }
        iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
        int len = iSystem->GetIPak()->FTell(fp);
        char *buf = new char [len+1];
        iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
        len = iSystem->GetIPak()->FRead(buf, 1, len, fp);
        iSystem->GetIPak()->FClose(fp);
        buf[len] = 0;
        gRenDev->m_cEF.m_Macros = vp->m_Macros;
        buf = gRenDev->m_cEF.mfScriptPreprocessor(buf, name2, szName);
        vp->mfCompile(buf);
        delete [] buf;
        if (nMaskGen != -1)
          return;
      }
    }
  }
#endif
}

char *CShader::mfRescanScript(int type, int nInd, SShader *pSHOrg, uint64 nMaskGen)
{
  char *pFinalScript = mfScriptForFileName(m_FileNames[type][nInd].c_str(), pSHOrg, nMaskGen);
  if (!pFinalScript)
    return NULL;
  ShaderFilesMapItor itor = m_RefEfs[type]->begin();
  TArray<SRefEfs *> REs;
  TArray<const char *> REnames;
  int ind = nInd;
  while(itor != m_RefEfs[type]->end())
  {
    SRefEfs* re = itor->second;
    if (re->m_Ind == nInd)
    {
      REs.AddElem(re);
      REnames.AddElem(itor->first.c_str());
    }
    itor++;
  }
  for (int n=0; n<REs.Num(); n++)
  {
    delete [] REs[n];
    m_RefEfs[type]->erase(REnames[n]);
  }
  char Er[256];
  sprintf(Er, "File '%s' script error!\n", m_FileNames[type][nInd].c_str());
  gShObjectNotFound = Er;
  mfScanScript(pFinalScript, ind);
  return pFinalScript;
}

bool CShader::mfReloadShaderScript(const char *szShaderName, int nFlags, SShader *pSH)
{
  CRenderer *rd = gRenDev;
  SRefEfsLoaded *fe;
  char name[256];
  strcpy(name, szShaderName);
  strlwr(name);
  LoadedShadersMapItor it = m_RefEfsLoaded.find(name);
  if (it == m_RefEfsLoaded.end())
    fe = NULL;
  else
    fe = &it->second;
  if (!fe)
  {
    Warning( 0,0,"Attempt to reload non-loaded shader '%s'", szShaderName);
    return false;
  }
  SShader *pSHOrg = fe->m_Ef;
  int nD = -1;
  int nDCount = 0;
  if (pSHOrg && pSHOrg->m_DerivedShaders && pSHOrg->m_DerivedShaders->Num())
    nDCount = pSHOrg->m_DerivedShaders->Num();
  while (true)
  {
    if (nDCount)
    {
      nD++;
      if (nD >= nDCount)
        return true;
      pSH = pSHOrg->m_DerivedShaders->Get(nD);
    }
    else
      pSH = pSHOrg;
    if (pSH->m_nRefreshFrame == m_nFrameForceReload)
    {
      if (nD < 0)
        return true;
      continue;
    }
    pSH->m_nRefreshFrame = m_nFrameForceReload;

    for (int type=0; type<2; type++)
    {
      SRefEfs *fe;
      ShaderFilesMapItor it;
      it = m_RefEfs[type]->find(szShaderName);
      if (it == m_RefEfs[type]->end())
        fe = NULL;
      else
        fe = it->second;
      if (!fe)
        continue;
      m_CurEfsNum = type;
      if (m_nFrameReload[type][fe->m_Ind] != m_nFrameForceReload)
      {
        FILE *status = iSystem->GetIPak()->FOpen(m_FileNames[type][fe->m_Ind].c_str(), "rb");

        if (!status)
          return false;

        FILETIME writetime = iSystem->GetIPak()->GetModificationTime(status);
        iSystem->GetIPak()->FClose (status);
        if (!(nFlags & FRO_FORCERELOAD))
        {
          if (CompareFileTime(&writetime, &pSH->m_WriteTime) == 0)
            return true;
        }
        m_nFrameReload[type][fe->m_Ind] = m_nFrameForceReload;
        m_WriteTime[type][fe->m_Ind] = writetime;
        pSH->m_WriteTime = writetime;
        char *pFinalScript;
        if (!(pFinalScript=mfRescanScript(type, fe->m_Ind, pSHOrg, pSH ? pSH->m_nMaskGen : 0)))
          continue;
        delete [] pFinalScript;
      }

      m_CurEfsNum = 0;
      m_bReload = true;
      if (nDCount)
        iLog->Log("Reload shader '%s(%I64x)'", szShaderName, pSH->m_nMaskGen);
      else
        iLog->Log("Reload shader '%s'", szShaderName);
      SShader *ef = gRenDev->m_cEF.mfForName(szShaderName, eSH_Misc, SF_RELOAD, NULL, pSH ? pSH->m_nMaskGen : 0);
      m_bReload = false;

      if (nD < 0)
        return true;
    }
  }

  return false;
}

bool SShader::Reload(int nFlags)
{
  int i, j;

  if (nFlags & FRO_SHADERS)
  {
    gRenDev->m_cEF.mfReloadShaderScript(m_Name.c_str(), nFlags, this);

    if (m_Flags & (EF_HASVSHADER | EF3_HASRCOMBINER | EF3_HASPSHADER))
    {
      for (i=0; i<m_HWTechniques.Num(); i++)
      {
        for (j=0; j<m_HWTechniques[i]->m_Passes.Num(); j++)
        {
          if (m_HWTechniques[i]->m_Passes[j].m_FShader)
            m_HWTechniques[i]->m_Passes[j].m_FShader->mfReload(nFlags);
          if (m_HWTechniques[i]->m_Passes[j].m_VProgram)
            m_HWTechniques[i]->m_Passes[j].m_VProgram->mfReload(nFlags);
        }
      }
    }
  }

  if (nFlags & FRO_SHADERTEXTURES)
  {
    for (i=0; i<m_Passes.Num(); i++)
    {
      gRenDev->m_cEF.mfRefreshLayer(&m_Passes[i], this);
    }
    for (i=0; i<m_HWTechniques.Num(); i++)
    {
      SShaderTechnique *hw = m_HWTechniques[i];
      for (j=0; j<hw->m_Passes.Num(); j++)
      {
        gRenDev->m_cEF.mfRefreshLayer(&hw->m_Passes[j], this);
      }
    }
    if (m_Templates)
    {
      for (i=0; i<m_Templates->m_TemplShaders.Num(); i++)
      {
        if (m_Templates->m_TemplShaders[i])
          m_Templates->m_TemplShaders[i]->Reload(nFlags);
      }
    }
  }

  return true;
}

bool CShader::mfReloadAllShaders(int nFlags)
{
  int i;

  bool bState = true;
  m_nFrameForceReload++;

  for (i=0; i<m_Nums; i++)
  {
    SShader *sh = SShader::m_Shaders_known[i];
    if (!sh)
      continue;

    if (!sh->Reload(nFlags))
      bState = false;
  }
#ifndef NULL_RENDERER
  for (i=0; i<CPShader::m_PShaders.Num(); i++)
  {
    CPShader *ps = CPShader::m_PShaders[i];
    if (!ps)
      continue;
    ps->mfReload(nFlags);
  }
  for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
  {
    CVProgram *vs = CVProgram::m_VPrograms[i];
    if (!vs)
      continue;
    vs->mfReload(nFlags);
  }
#endif

  return bState;
}

bool CShader::mfReloadShader(const char *szName, int nFlags)
{
  SRefEfsLoaded *fe;
  char name[256];
  strcpy(name, szName);
  strlwr(name);
  LoadedShadersMapItor it = m_RefEfsLoaded.find(name);
  if (it == m_RefEfsLoaded.end())
    fe = NULL;
  else
    fe = &it->second;
  if (!fe)
    return false;
  return fe->m_Ef->Reload(nFlags);
}

bool CShader::mfReloadShaderFile(const char *szName, int nFlags)
{
  TArray<char *> ShNames;
  char **EfNames = gRenDev->EF_GetShadersForFile(szName, -1);
  if (EfNames)
  {
    int j = 0;
    bool bRes = false;
    while (EfNames[j])
    {
      ShNames.AddElem(EfNames[j]);
      EfNames[j] = NULL;
      j++;
    }
    for (j=0; j<ShNames.Num(); j++)
    {
      if (mfReloadShader(ShNames[j], nFlags))
        bRes = true;
      delete [] ShNames[j];
    }
    return bRes;
  }
  return false;
}

bool CShader::mfReloadFile(const char *szPath, const char *szName, int nFlags)
{
  m_nFrameForceReload++;

  const char *ext = GetExtension(szName);
  char fName[256];
  strcpy(fName, szPath);
  strcat(fName, szName);
  if (!strnicmp(ext, ".csl", 4))
  {
    return mfReloadShaderFile(fName, nFlags);
  }
  else
  if (!stricmp(ext, ".csi"))
  {
    int i, j;
    char path[256];
    TArray<char *> CheckNames;
    TArray<char *> AffectedFiles;
    strcpy(path, szPath);
    strcat(path, szName);
    strlwr(path);
    CheckNames.AddElem(path);

    for (j=0; j<CheckNames.Num(); j++)
    {
      for (i=0; i<2; i++)
      {
        mfCheckAffectedFiles(m_ShadersPath[i], j, CheckNames, AffectedFiles);
      }
    }
    for (i=1; i<CheckNames.Num(); i++)
    {
      SAFE_DELETE_ARRAY(CheckNames[i]);
    }
    if (!AffectedFiles.Num())
      return false;
    bool bRes = false;
    for (i=0; i<AffectedFiles.Num(); i++)
    {
      if (mfReloadShaderFile(AffectedFiles[i], nFlags | FRO_FORCERELOAD))
        bRes = true;
      SAFE_DELETE_ARRAY(AffectedFiles[i]);
    }
    return bRes;
  }
  else
  if (!stricmp(ext, ".crycg"))
  {
    char name[256];
    strcpy(name, szPath);
    strlwr(name);
    if (strstr(name, "cgvshaders"))
      CVProgram::mfReloadScript(szPath, szName, nFlags, -1);
    else
    if (strstr(name, "cgpshaders"))
      CPShader::mfReloadScript(szPath, szName, nFlags, -1);
  }

  return false;
}

void CShader::mfCheckAffectedFiles(const char *ShadersPath, int nCheckFile, TArray<char *>& CheckNames, TArray<char *>& AffectedFiles)
{
  struct _finddata_t fileinfo;
  intptr_t handle;
  int len;
  FILE *fp;
  char *buf;
  char nmf[256];
  char dirn[256], *ddir;
  int i;

  ddir = dirn;

  strcpy(dirn, ShadersPath);
  strcat(dirn, "*.*");
  ConvertUnixToDosName(dirn, dirn);

  handle = iSystem->GetIPak()->FindFirst (ddir, &fileinfo);
  if (handle == -1)
    return;

  do
  {
    if (fileinfo.name[0] == '.')
      continue;

    if (fileinfo.attrib & _A_SUBDIR)
    {
      char next[256];
      sprintf(next, "%s%s/", ShadersPath, fileinfo.name);

      mfCheckAffectedFiles(next, nCheckFile, CheckNames, AffectedFiles);
      continue;
    }

    strcpy(nmf, ShadersPath);
    strcat(nmf, fileinfo.name);
    len = strlen(nmf);
    while (len && nmf[len] != '.')
      len--;
    if (len <= 0)
      continue;
    if (strnicmp(&nmf[len], ".csl", 4) && stricmp(&nmf[len], ".csi"))
      continue;
    if (nmf[len+4])
    {
      if (!stricmp(&nmf[len+4], "new"))
      {
        if (CRenderer::CV_r_usehwshaders!=2)
          continue;
      }
      else
      if (!stricmp(&nmf[len+4], "old"))
      {
        if (CRenderer::CV_r_usehwshaders!=1)
          continue;
      }
      else
        continue;
    }

    fp = iSystem->GetIPak()->FOpen(nmf, "rb");
    if (!fp)
      continue;

    iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
    int ln = iSystem->GetIPak()->FTell(fp);
    if (!ln)
    {
      iSystem->GetIPak()->FClose(fp);
      continue;
    }
    buf = new char [ln+1];
    if (!buf)
    {
      iSystem->GetIPak()->FClose(fp);
      iConsole->Exit("Can't allocate %d bytes for shader file '%s'\n", len+1, nmf);
    }

    buf[ln] = 0;
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
    iSystem->GetIPak()->FRead(buf, 1, ln, fp);
    iSystem->GetIPak()->FClose(fp);
    strlwr(buf);
    char fname[256];
    char fext[16];
    _splitpath(CheckNames[nCheckFile], NULL, NULL, fname, fext);
    strcat(fname, fext);
    if (strstr(buf, fname))
    {
      ln = strlen(nmf)+1;
      char *fullname = new char[ln];
      strcpy(fullname, nmf);
      strlwr(fullname);
      if (!strnicmp(&nmf[len], ".csl", 4))
      {
        for (i=0; i<AffectedFiles.Num(); i++)
        {
          if (!strcmp(AffectedFiles[i], fullname))
            break;
        }
        if (i == AffectedFiles.Num())
          AffectedFiles.AddElem(fullname);
      }
      else
      {
        for (i=0; i<CheckNames.Num(); i++)
        {
          if (!strcmp(CheckNames[i], fullname))
            break;
        }
        if (i == CheckNames.Num())
          CheckNames.AddElem(fullname);
      }
    }
    delete [] buf;
  } while (iSystem->GetIPak()->FindNext( handle, &fileinfo ) != -1);

  iSystem->GetIPak()->FindClose (handle);
}

//=============================================================================

static char *sEfNames[256];

char **CShader::mfListInScript (char *scr)
{
  char* name;
  long cmd;
  char *params;
  int num = 0;
  float ver = 0;
  char *buf;

  enum {eShader=1, eVersion, eSubrScript};
  static tokenDesc commands[] =
  {
    {eShader, "Shader"},
    {eVersion, "Version"},
    {eSubrScript, "SubrScript"},
    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
    case eVersion:
      ver = shGetFloat(params);
      if (ver != SHADER_VERSION)
      {
        Warning( 0,0,"Warning: Shader Script version (%f) must be %f\n", ver, SHADER_VERSION);
        return NULL;
      }
      break;

    case eShader:
      buf = new char[strlen(name)+1];
      strcpy(buf, name);
      if (sEfNames[num])
        delete [] sEfNames[num];
      sEfNames[num++] = buf;
      break;
    }
  }

  sEfNames[num] = NULL;

  return &sEfNames[0];
}

static SRefEfs *sFE;

void CShader::mfScanScript (char *scr, int n)
{
  char* name;
  long cmd;
  char *params;
  float ver = 0;

  char *s = scr;

  enum {eShader=1, eVersion, eSubrScript};
  static tokenDesc commands[] =
  {
    {eShader, "Shader"},
    {eVersion, "Version"},
    {eSubrScript, "SubrScript"},
    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) >= 0)
  {
    switch (cmd)
    {
      case eVersion:
        ver = shGetFloat(params);
        if (ver != SHADER_VERSION)
        {
          Warning( 0,0,"Warning: Shader Script version (%f) must be %f\n", ver, SHADER_VERSION);
          return;
        }
        break;

      case eShader:
        {
          SRefEfs *fe;
          char nameEf[256];
          //StripExtension(name, nameEf);
          strcpy(nameEf, name);
          ConvertDOSToUnixName(nameEf, nameEf);
          strlwr(nameEf);
          ShaderFilesMapItor it = m_RefEfs[m_CurEfsNum]->find(nameEf);
          if (it == m_RefEfs[m_CurEfsNum]->end())
            fe = NULL;
          else
            fe = it->second;
          if (fe)
          {
            Warning( 0,0,"Warning: Shader '%s' is duplicated\n", nameEf);
            break;
          }
          fe = new SRefEfs;
          fe->m_Ind = n;
          fe->m_Offset = params - s;
          fe->m_Size = strlen(params);
          m_RefEfs[m_CurEfsNum]->insert(ShaderFilesMapItor::value_type(nameEf, fe));
        }
        break;
    }
  }
}

char *CShader::mfPreprCheckIncludes (char *buf, const char *drn, const char *name) 
{
  if (!strchr(buf, '#'))
    return buf;
  TArray <char> nBuf;

  int len = strlen(buf)+1;
  char tdrn[512];
  strcpy(tdrn, drn);

  int nPos = 0;
  nBuf.Create(len);
  memcpy(&nBuf[0], buf, len);
  bool bFind = false;
  int nIncl;
  while (true)
  {
    char *pos = strchr(&nBuf[nPos], '#');
    if (!pos)
      break;
    nIncl = pos - &nBuf[0];
    if (strncmp(&pos[1], "include", 7))
    {
      nPos += pos - &nBuf[nPos] + 1;
      continue;
    }
    nPos += pos - &nBuf[nPos] + 1 + 7;
    while (nBuf[nPos] != '"')
    {
      if (nBuf[nPos] == 0 || nBuf[nPos] == 0xa)
      {
        Warning( 0,0,"Warning: Missing include name for shader file '%s'\n", name);
        break;
      }
      nPos++;
    }
    char ni[512];
    nPos++;
    int nPrevPos = nPos;
    bool bRes = true;
    int n = strlen(tdrn);
    strcpy(ni, tdrn);
    while (nBuf[nPos] != '"')
    {
      if (nBuf[nPos] == 0 || nBuf[nPos] == 0xa || n == 128)
      {
        Warning( 0,0,"Warning: Missing or invalid include name for shader file '%s'\n", name);
        bRes = false;
        break;
      }
      ni[n] = nBuf[nPos];
      n++;
      nPos++;
    }
    if (!bRes)
    {
      nPos = nPrevPos;
      continue;
    }
    ni[n] = 0;
    nPos++;
    FILE *fp = iSystem->GetIPak()->FOpen(ni, "rb");
    if (!fp)
    {
      Warning( 0,0,"Warning: Missing include file '%s' for shader file '%s'\n", ni, name);
      continue;
    }
    char drv[16], dirn[512], drnn[512]; 
    _splitpath(ni, drv, dirn, NULL, NULL);
    strcpy(drnn, drv);
    strcat(drnn, dirn);
    bFind = true;
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
    int li = iSystem->GetIPak()->FTell(fp);
    int nPrev = nBuf.Num();
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
    char *pbuf = new char [li+1];
    li = iSystem->GetIPak()->FRead(pbuf, 1, li, fp);
    iSystem->GetIPak()->FClose(fp);
    pbuf[li] = 0;
    RemoveCR(pbuf);
    char *npbuf = RemoveComments(pbuf);
    assert(npbuf == pbuf);
    npbuf = mfPreprCheckIncludes(pbuf, drnn, name);
    if (pbuf != npbuf)
    {
      pbuf = npbuf;
      li = strlen(pbuf);
    }
    nBuf.AddIndex(li-(nPos-nIncl));
    memmove(&nBuf[nIncl+li], &nBuf[nPos], nPrev-nPos);
    memcpy(&nBuf[nIncl], pbuf, li);
    delete [] pbuf;
    nPos = nIncl + li;
  }
  if (!bFind)
    return buf;
  char *b = new char [nBuf.Num()];
  memcpy(b, &nBuf[0], nBuf.Num());
  delete buf;

  return b;
}

int CShader::mfRemoveScript_ifdef(char *posStart, char *posEnd, bool bRemoveAll, int nPos, char *buf, const char *fileName)
{
  int nLevel = 0;
  int nNewPos = -1;
  while (true)
  {
    char *posS = strchr(&buf[nPos], '#');
    if (!posS)
    {
      char dir[256];
      strncpy(dir, posStart, posEnd-posStart+1);
      dir[posEnd-posStart+1] = 0;
      Warning( 0,0,"Couldn't find #endif directive for associated #ifdef %s for shader file %s", dir, fileName);
      break;
    }
    if (posS[1]=='i' && posS[2]=='f')
    {
      nLevel++;
      if (nNewPos < 0)
        nNewPos = nPos;
      nPos += posS - &buf[nPos] + 1;
      continue;
    }
    if (!strncmp(&posS[1], "endif", 5))
    {
      if (!nLevel)
      {
        if (bRemoveAll)
        {
          char *b = posEnd;
          while (b != posS)
          {
            if (*b != 0xa)
              *b = 0x20;
            b++;
          }
        }
        memset(posS, 0x20202020, 6);
        memset(posStart, 0x20202020, posEnd-posStart);
        nPos += posS - &buf[nPos] + 1 + 6;
        break;
      }
      nLevel--;
      nPos += posS - &buf[nPos] + 1;
      continue;
    }
    nPos += posS - &buf[nPos] + 1;
  }
  if (nNewPos > 0)
  {
    assert(nNewPos < nPos);
    char ch = buf[nPos];
    buf[nPos] = 0;
    mfPreprCheckConditions(&buf[nNewPos], fileName);
    buf[nPos] = ch;
  }
  return nPos;
}

char *CShader::mfPreprCheckConditions(char *buf, const char *nameFile) 
{
  if (!strchr(buf, '#'))
    return buf;

  int nPos = 0;
  bool bAccept = false;
  char *VP = gRenDev->GetVertexProfile(false);
  char *PP = gRenDev->GetPixelProfile(false);
  char *VPSup = gRenDev->GetVertexProfile(true);
  char *PPSup = gRenDev->GetPixelProfile(true);
  int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;
  while (true)
  {
    char *posStart = strchr(&buf[nPos], '#');
    if (!posStart)
      break;
    if (strncmp(&posStart[1], "ifdef", 5))
    {
      nPos += posStart - &buf[nPos] + 1;
      continue;
    }
    nPos += posStart - &buf[nPos] + 1 + 5;
    while (buf[nPos] == 0x20 || buf[nPos] == 0x9)
    {
      nPos++;
    }
    if (buf[nPos] == 0x20 || buf[nPos] == 0xa || buf[nPos] == 0)
    {
      Warning( 0,0,"Warning: Missing ifdef parameter for shader file '%s'\n", nameFile);
      break;
    }
    char ni[512];
    int n = 0;
    ni[0] = 0;
    while (buf[nPos] != 0x20 && buf[nPos] != 0xa && buf[nPos] != 0 && buf[nPos] != 0x9)
    {
      ni[n] = buf[nPos];
      n++;
      nPos++;
    }
    char *posEnd = &buf[nPos];
    ni[n] = 0;
    if (!stricmp(ni, "OTHER"))
    {
      if (bAccept)
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      else
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = false;
    }
    else
    if (!strnicmp(ni, "PROFILE_VS", 10))
    {
      if (!stricmp(ni, VP))
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!strnicmp(ni, "PROFILE_PS", 10))
    {
      if (!stricmp(ni, PP))
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!strnicmp(ni, "SUPPORT_PROFILE_VS", 18))
    {
      if (!stricmp(&ni[8], VPSup))
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!strnicmp(ni, "SUPPORT_PROFILE_PS", 18))
    {
      if (!stricmp(&ni[8], PPSup))
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "DIRECT3D") || !stricmp(ni, "D3D"))
    {
#if defined (DIRECT3D8) || defined (DIRECT3D9) || defined (XBOX)
      nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = true;
#else
      nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      bAccept = false;
#endif
    }
    else
    if (!stricmp(ni, "DIRECT3D8") || !stricmp(ni, "D3D8"))
    {
#if defined (DIRECT3D8)
      nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = true;
#else
      nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      bAccept = false;
#endif
    }
    else
    if (!stricmp(ni, "DIRECT3D9") || !stricmp(ni, "D3D9"))
    {
#if defined (DIRECT3D9)
      nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = true;
#else
      nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      bAccept = false;
#endif
    }
    else
    if (!stricmp(ni, "XBOX"))
    {
#if defined (XBOX)
      nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = true;
#else
      nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      bAccept = false;
#endif
    }
    else
    if (!stricmp(ni, "OPENGL"))
    {
#if defined (OPENGL)
      nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = true;
#else
      nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      bAccept = false;
#endif
    }
    else
    if (!stricmp(ni, "GAMECUBE") || !stricmp(ni, "GC"))
    {
#if defined (GC)
      nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = true;
#else
      nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      bAccept = false;
#endif
    }
    else
    if (!stricmp(ni, "PS2"))
    {
#if defined (PS2)
      nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
      bAccept = true;
#else
      nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
      bAccept = false;
#endif
    }
    else
    if (!strnicmp(ni, "DEPTHMAP", 8))
    {
      if (gRenDev->GetFeatures() & RFT_DEPTHMAPS)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!strnicmp(ni, "SELFSHADOW", 10))
    {
      if (gRenDev->GetFeatures() & RFT_SHADOWMAP_SELFSHADOW)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "PROJECTEDENVBUMP"))
    {
      if (gRenDev->GetFeatures() & RFT_HW_ENVBUMPPROJECTED)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "NV1X") || !stricmp(ni, "NV10") || !stricmp(ni, "NV17") || !stricmp(ni, "GF2"))
    {
      if (nGPU == RFT_HW_GF2)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "NV2X") || !stricmp(ni, "NV20") || !stricmp(ni, "NV25") || !stricmp(ni, "GF3"))
    {
      if (nGPU == RFT_HW_GF3)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "NV3X") || !stricmp(ni, "NV30") || !stricmp(ni, "NV35") || !stricmp(ni, "GEFORCEFX"))
    {
      if (nGPU == RFT_HW_GFFX)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "NV4X") || !stricmp(ni, "NV40") || !stricmp(ni, "NV45"))
    {
      if (nGPU == RFT_HW_NV4X)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "R250") || !strnicmp(ni, "RADEON", 6))
    {
      if (nGPU == RFT_HW_RADEON)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "R300"))
    {
      if (nGPU == RFT_HW_RADEON)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "HDR"))
    {
      if (gRenDev->GetFeatures() & RFT_HW_HDR)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "HDR_FP16"))
    {
      if ((gRenDev->GetFeatures() & RFT_HW_HDR) && gRenDev->m_nHDRType == 1)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "PS30"))
    {
      if (gRenDev->m_nEnabled_PS30)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    else
    if (!stricmp(ni, "PS2X"))
    {
      if (gRenDev->m_nEnabled_PS2X)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
    if (!stricmp(ni, "PS30|PS2X"))
    {
      if (gRenDev->m_nEnabled_PS30 || gRenDev->m_nEnabled_PS2X)
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, false, nPos, buf, nameFile);
        bAccept = true;
      }
      else
      {
        nPos = mfRemoveScript_ifdef(posStart, posEnd, true, nPos, buf, nameFile);
        bAccept = false;
      }
    }
  }

  return buf;
}


char *CShader::mfPreprCheckMacros(char *buf, const char *nameFile) 
{
  /*FILE *fp = iSystem->GetIPak()->FOpen("test.txt", "r");
  if (fp)
  {
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
    int len = iSystem->GetIPak()->FTell(fp);
    buf = new char [len+1];
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
    len = iSystem->GetIPak()->FRead(buf, 1, len, fp);
    iSystem->GetIPak()->FClose(fp);
    buf[len] = 0;
  }*/
  if (!strchr(buf, '#'))
    return buf;
  TArray <char> nBuf;

  int len = strlen(buf)+1;

  TArray<char> macro;
  nBuf.Create(len);
  memcpy(&nBuf[0], buf, len);
  bool bFind = false;
  int nPos = 0;
  int nIFDepth = 0;
  while (true)
  {
    if (nBuf[nPos] == 0)
      break;

    char word[1024];
    int nw = 0;
    word[0] = 0;
    while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9 || nBuf[nPos] == 0xa || nBuf[nPos] == ',' || nBuf[nPos] == ';') { nPos++; }
    int nPrevPos = nPos;
    while (nBuf[nPos] != 0x20 && nBuf[nPos] != 9 && nBuf[nPos] != 0xa && nBuf[nPos] != ',' && nBuf[nPos] != ';' && nBuf[nPos] != 0)
    {
      word[nw++] = nBuf[nPos++];
    }
    word[nw] = 0;
    if (word[0])
    {
      if (word[0] == '#')
      {
        if (!strcmp(&word[1], "if") || !strcmp(&word[1], "elif"))
        {
          if (word[1] == 'i')
          {
            bFind = true;
            nIFDepth++;
          }
          else
          {
            if (!nIFDepth)
              Warning( 0,0,"#elif without #if in shader file %s", nameFile);
          }
          int nStartIF = nPrevPos;
          while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9) { nPos++; }
          int nLog = 0;
          int nStartIFExp = nPos;
          bool bCond[32];
          byte bLogic[32];
          while (true)
          {
            nw = 0;
            while (nBuf[nPos] != 0x20 && nBuf[nPos] != 9 && nBuf[nPos] != 0xa && nBuf[nPos] != ',' && nBuf[nPos] != ';' && nBuf[nPos] != 0)
            {
              word[nw++] = nBuf[nPos++];
            }
            word[nw] = 0;
            bool bNegative = false;
            int n = 0;
            if (word[0] == '!')
            {
              n = 1;
              bNegative = true;
            }
            ShaderMacroItor it = m_Macros.find(&word[n]);
            if (it != m_Macros.end())
              bCond[nLog] = bNegative ? false : true;
            else
              bCond[nLog] = bNegative ? true : false;
            while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9) { nPos++; }
            nLog++;
            if (nBuf[nPos] != 0xa)
            {
              bLogic[nLog] = 0;
              if (nBuf[nPos] == '|' && nBuf[nPos+1] == '|')
                bLogic[nLog] = 1;
              else
              if (nBuf[nPos] == '&' && nBuf[nPos+1] == '&')
                bLogic[nLog] = 2;
              if (bLogic[nLog])
              {
                nPos += 2;
                while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9) { nPos++; }
              }
            }
            if (nBuf[nPos] == 0xa)
              break;
          }
          int nEndIFExp = nPos;
          bool bResultCond = false;
          for (int i=0; i<nLog; i++)
          {
            if (!i)
              bResultCond = bCond[i];
            else
            if (bLogic[i] == 1)
              bResultCond = bResultCond | bCond[i];
            else
            if (bLogic[i] == 2)
              bResultCond = bResultCond & bCond[i];
          }
          char *posStart = NULL;
          int nNewPos = nPos;
          int nLevel = 0;
          if (bResultCond)
          {
            while (true)
            {
              char *posS = strchr(&nBuf[nPos], '#');
              if (!posS)
              {
                char dir[256];
                strncpy(dir, &nBuf[nStartIFExp], nEndIFExp-nStartIFExp+1);
                dir[nEndIFExp-nStartIFExp+1] = 0;
                Warning( 0,0,"Couldn't find #endif directive for associated #if %s for shader file %s", dir, nameFile);
                break;
              }
              if (posS[1]=='i' && posS[2]=='f' && (posS[3]==0x20 || posS[3]==9))
              {
                nLevel++;
                nPos += posS - &nBuf[nPos] + 1 + 3;
                continue;
              }
              if (!strncmp(&posS[1], "elif", 4) || !strncmp(&posS[1], "else", 4))
              {
                if (!nLevel && !posStart)
                  posStart = posS;
                nPos += posS - &nBuf[nPos] + 1 + 5;
                continue;
              }
              if (!strncmp(&posS[1], "endif", 5))
              {
                if (!nLevel)
                {
                  memset(&nBuf[nStartIF], 0x20202020, nEndIFExp-nStartIF);
                  if (posStart)
                    memset(posStart, 0x20202020, posS-posStart);
                  else
                    memset(posS, 0x20202020, 6);
                  nPos += posS - &nBuf[nPos] + 1 + 6;
                  break;
                }
                nLevel--;
                nPos += posS - &nBuf[nPos] + 1;
                continue;
              }
              nPos += posS - &nBuf[nPos] + 1;
            }
          }
          else
          {
            posStart = &nBuf[nStartIF];
            while (true)
            {
              char *posS = strchr(&nBuf[nPos], '#');
              if (!posS)
              {
                char dir[256];
                strncpy(dir, &nBuf[nStartIFExp], nEndIFExp-nStartIFExp+1);
                dir[nEndIFExp-nStartIFExp+1] = 0;
                Warning( 0,0,"Couldn't find #endif directive for associated #if %s for shader file %s", dir, nameFile);
                break;
              }
              if (posS[1]=='i' && posS[2]=='f' && (posS[3]==0x20 || posS[3]==9))
              {
                nLevel++;
                nPos += posS - &nBuf[nPos] + 1 + 3;
                continue;
              }
              if (!strncmp(&posS[1], "elif", 4) || !strncmp(&posS[1], "else", 4))
              {
                if (!nLevel)
                {
                  nNewPos = posS - &nBuf[0];
                  memset(&nBuf[nStartIF], 0x20202020, nEndIFExp-nStartIF);
                  memset(posStart, 0x20202020, posS-posStart);
                  break;
                }
                nPos += posS - &nBuf[nPos] + 1 + 5;
                continue;
              }
              if (!strncmp(&posS[1], "endif", 5))
              {
                if (!nLevel)
                {
                  memset(&nBuf[nStartIF], 0x20202020, nEndIFExp-nStartIF);
                  memset(posStart, 0x20202020, posS-posStart);
                  memset(posS, 0x20202020, 6);
                  nPos += posS - &nBuf[nPos] + 1 + 6;
                  break;
                }
                nLevel--;
                nPos += posS - &nBuf[nPos] + 1;
                continue;
              }
              nPos += posS - &nBuf[nPos] + 1;
            }
          }
          nPos = nNewPos;
          continue;
        }
        else
        if (!strcmp(word, "#else"))
        {
          if (!nIFDepth)
            Warning( 0,0,"#else without #if in shader file %s", nameFile);
          memset(&nBuf[nPrevPos], 0x20202020, nPos-nPrevPos);
          continue;
        }
        else
        if (!strcmp(word, "#endif"))
        {
          if (!nIFDepth)
            Warning( 0,0,"#endif without #if in shader file %s", nameFile);
          else
            nIFDepth--;
          memset(&nBuf[nPrevPos], 0x20202020, nPos-nPrevPos);
          continue;
        }
        else
        if (!strcmp(word, "#define"))
        {
          bFind = true;
          nw = 0;
          word[0] = 0;
          macro.SetUse(0);
          while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9) { nPos++; }
          while (nBuf[nPos] != 0x20 && nBuf[nPos] != 9 && nBuf[nPos] != 0xa && nBuf[nPos] != 0)
          {
            word[nw++] = nBuf[nPos++];
          }
          word[nw] = 0;
          /*if (word[0] == '%')
          {
            int nnn = 0;
          }*/
          if (!word[0])
          {
            memset(&nBuf[nPrevPos], 0x20202020, nPos-nPrevPos); 
            continue;
          }
          while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9) { nPos++; }
          bool bAllowCR = false;
          int bWasCR = 0;
          while (nBuf[nPos] != 0)
          {
            if (nBuf[nPos] == 0xa)
            {
              if (!bAllowCR)
                break;
              if (nBuf[nPos] == 0xa)
                bWasCR |= 1;
              if (bWasCR == 1)
              {
                bAllowCR = false;
                bWasCR = 0;
              }
            }
            else
            if (nBuf[nPos] == '\\')
            {
              bAllowCR = true;
              nPos++;
              continue;
            }
            else
            if (bWasCR)
            {
              bAllowCR = false;
              bWasCR = 0;
            }
            macro.AddElem(nBuf[nPos]);
            nPos++;
          }
          if (!macro.Num())
            macro.AddElem(' ');
          macro.AddElem(0);
          m_Macros.insert(ShaderMacroItor::value_type(word, &macro[0]));
          memset(&nBuf[nPrevPos], 0x20202020, nPos-nPrevPos); 
          continue;
        }
        else
        if (!strcmp(word, "#undefine"))
        {
          bFind = true;
          nw = 0;
          while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9) { nPos++; }
          while (nBuf[nPos] != 0x20 && nBuf[nPos] != 9 && nBuf[nPos] != 0xa && nBuf[nPos] != 0)
          {
            word[nw++] = nBuf[nPos++];
            word[nw] = 0;
          }
          ShaderMacroItor it = m_Macros.find(word);
          if (it == m_Macros.end())
            Warning( 0,0,"Warning: Couldn't find macro '%s' in file '%s'\n", word, nameFile);
          nBuf.Remove(nPrevPos, nPos-nPrevPos);
          nPos = nPrevPos;
          if (!word[0])
            continue;
          m_Macros.erase(word);
          continue;
        }
        else
        if (!strcmp(word, "#warning"))
        {
          nw = 0;
          while (nBuf[nPos] == 0x20 || nBuf[nPos] == 9) { nPos++; }
          while (nBuf[nPos] != 0xa && nBuf[nPos] != 0)
          {
            word[nw++] = nBuf[nPos++];
          }
          word[nw] = 0;
          while (nPrevPos != nPos)
          {
            nBuf[nPrevPos++] = 0x20;
          }
          Warning( 0,0,"Warning: <SHADER>: '%s' (in %s)", word, nameFile);
          continue;
        }
      }
      if (!stricmp(word, "CGVProgram") || !stricmp(word, "CGPShader"))
      {
        SLocalMacros LM;
        LM.m_nOffset = nPrevPos;
        //char *pTmp = strstr(&nBuf[0], "CGVProgram");
        //int nTmp = pTmp-&nBuf[0];
        LM.m_Macros = new ShaderMacro;
        *LM.m_Macros = m_Macros;
        m_LocalMacros.AddElem(LM);
      }
      ShaderMacroItor it = m_Macros.find(word);
      if (it != m_Macros.end())
      {
        bFind = true;
        nBuf.Remove(nPrevPos, nPos-nPrevPos);
        const char *macro = it->second.c_str();
        int len = strlen(macro);
        nBuf.AddIndex(len);
        memmove(&nBuf[nPrevPos+len], &nBuf[nPrevPos], nBuf.Num()-nPrevPos-len);
        memcpy(&nBuf[nPrevPos], macro, len);
        nPos = nPrevPos;
      }
    }
  }
  if (!bFind)
    return buf;
  /*fp = fopen("test1.txt", "w");
  if (fp)
  {
    fprintf(fp, &nBuf[0]);
    fclose(fp);
  }*/
  //char *pTmp = strstr(&nBuf[0], "CGVProgram");
  //int nTmp = pTmp-&nBuf[0];

  char *b = new char [nBuf.Num()];
  memcpy(b, &nBuf[0], nBuf.Num());
  delete buf;

  return b;
}

void CShader::mfStartScriptPreprocess()
{
  m_Macros.clear();
  for (int i=0; i<m_LocalMacros.Num(); i++)
  {
    delete m_LocalMacros[i].m_Macros;
  }
  m_LocalMacros.Free();
}

uint64 CShader::mfScriptPreprocessorMask(SShader *pSH, int nOffset) 
{
  uint64 nMask = 0;
  int i;
  for (i=0; i<m_LocalMacros.Num(); i++)
  {
    if (m_LocalMacros[i].m_nOffset == nOffset)
      break;
  }
  if (i == m_LocalMacros.Num())
  {
    iLog->Log("Warning: couldn't find Local macros state for offset %d in shader '%s'", pSH->GetName());
    return nMask;
  }
  m_Macros = *m_LocalMacros[i].m_Macros;
	ShaderMacroItor itor=m_LocalMacros[i].m_Macros->begin();
  while(itor!=m_LocalMacros[i].m_Macros->end())
  {
    if (itor->first[0] == '%')
    {
      const char *val = itor->second.c_str();
      int len = strlen(val);
      uint64 n;
      if (len > 2 && val[0] == '0' && val[1] == 'x')
        sscanf(&val[2], "%I64x", &n);
      else
        n = atoi(val);
      if (!n)
        iLog->Log("Warning: zero mask for parameter macro '%s' in shader '%s'", itor->first.c_str(), pSH->GetName());
      if (n & nMask)
      {
        iLog->Log("Warning: mask 0x%I64x already exist for parameter macro in shader '%s'", n, itor->first.c_str(), pSH->GetName());
	      ShaderMacroItor itor=m_LocalMacros[i].m_Macros->begin();
        while(itor!=m_LocalMacros[i].m_Macros->end())
        {
          if (itor->first[0] == '%')
          {
            const char *val = itor->second.c_str();
            int len = strlen(val);
            uint64 nn;
            if (len > 2 && val[0] == '0' && val[1] == 'x')
              sscanf(&val[2], "%I64x", &nn);
            else
              nn = atoi(val);
            if (nn & n)
              iLog->Log("Parameter: %s", itor->first.c_str());
          }
          itor++;
        }
      }
      nMask |= n;
    }
    itor++;
  }
  return nMask;
}

char *CShader::mfScriptPreprocessor (char *buf, const char *drn, const char *name) 
{
  RemoveCR(buf);
  buf = RemoveComments(buf);
  buf = mfPreprCheckIncludes(buf, drn, name);
  buf = mfPreprCheckConditions(buf, name);
  buf = mfPreprCheckMacros(buf, name);

  /*FILE *fp = fopen(name, "w");
  if (fp)
  {
    fprintf(fp, buf);
    fclose(fp);
  }*/

#ifndef _XBOX
  if (CRenderer::CV_r_shaderssave > 2)
  {
    char nn[256];
    StripExtension(name, nn);
    AddExtension(nn, ".csf");
    FILE *fp = fopen(nn, "w");
    if (fp)
    {
      fprintf(fp, buf);
      fclose(fp);
    }
  }
#endif

  return buf;
}

char *CShader::mfScriptForFileName(const char *name, SShader *shGen, uint64 nMaskGen)
{
  FILE *fp = iSystem->GetIPak()->FOpen(name, "rb");
  if (!fp)
    return NULL;
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
  int len = iSystem->GetIPak()->FTell(fp);
  if (!len)
  {
    iSystem->GetIPak()->FClose(fp);
    return NULL;
  }
  TArray<char> custMacros;
  if (shGen && shGen->m_ShaderGenParams)
  {
    int i;
    //assert(nMaskGen);
    SShaderGen *shG = shGen->m_ShaderGenParams;
    assert(shG);
    for (i=0; i<shG->m_BitMask.Num(); i++)
    {
      if (shG->m_BitMask[i]->m_Mask & nMaskGen)
      {
        char macro[256];
        sprintf(macro, "#define %s 0x%I64x\n", shG->m_BitMask[i]->m_ParamName.c_str(), shG->m_BitMask[i]->m_Mask);
        int size = strlen(macro);
        int nOffs = custMacros.Num();
        custMacros.Grow(size);
        memcpy(&custMacros[nOffs], macro, size);
      }
    }
  }
  int size = custMacros.Num();
  char *buf = new char [len+size+1];
  if (!buf)
  {
    iSystem->GetIPak()->FClose(fp);
    Warning( 0,0,"Error: Can't allocate %d bytes for shader file '%s'\n", len+1, name);
    return NULL;
  }
  memcpy(buf, &custMacros[0], custMacros.Num());
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
  len = iSystem->GetIPak()->FRead(&buf[size], 1, len, fp);
  iSystem->GetIPak()->FClose(fp);
  buf[len+size] = 0;

  char nmf[256], drn[256], drv[16], dirn[256], fln[256], extn[16];
  _splitpath(name, drv, dirn, fln, extn);
  strcpy(drn, drv);
  strcat(drn, dirn);
  strcpy(nmf, fln);
  strcat(nmf, extn);
  mfStartScriptPreprocess();
  char *pFinalScript = mfScriptPreprocessor(buf, drn, nmf);

  return pFinalScript;
}

int CShader::mfLoadSubdir (char *drn, int n) 
{
  struct _finddata_t fileinfo;
  intptr_t handle;
  int len;
  char nmf[256];
  char dirn[256], *ddir;

  if (n == MAX_EF_FILES)
    return n;

  ddir = dirn;
    
  strcpy(dirn, drn);
  strcat(dirn, "*.*");
  ConvertUnixToDosName(dirn, dirn);

  /*{
    handle = iSystem->GetIPak()->FindFirst ("Shaders\\HWScripts\\Declarations\\CGVShaders\\*.crycg", &fileinfo);
    if (handle == -1)
    {
      assert(0);
    }

    do
    {
      if (fileinfo.name[0] == '.')
        continue;

      if (fileinfo.attrib & _A_SUBDIR)
        continue;

      strcpy(nmf, "Shaders\\HWScripts\\Declarations\\CGVShaders\\");
      strcat(nmf, fileinfo.name);

      FILE *fp = iSystem->GetIPak()->FOpen(nmf, "r");
      if (!fp)
        continue;

      iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
      int ln = iSystem->GetIPak()->FTell(fp);
      if (!ln)
      {
        iSystem->GetIPak()->FClose(fp);
        continue;
      }
      char *buf = new char [ln+1];
      if (!buf)
      {
        iSystem->GetIPak()->FClose(fp);
      }

      iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
      ln = iSystem->GetIPak()->FRead(buf, 1, ln, fp);
      iSystem->GetIPak()->FClose(fp);
      buf[ln] = 0;

      TArray <char> aStr1;
      TArray <char> aStr2;
      char *buf1 = strstr(buf, "IN_");
      char *buff1 = buf1;
      if (buf1)
      {
        int nSpac = 0;
        char *b = buf1-1;
        while (*b == 0x20 || *b == 9)
        {
          nSpac++;
          b--;
        }
        b++;
        char str[256];
        strcpy(str, "struct appin\n");
        aStr1.Copy(str, strlen(str));
        memcpy(str, b, nSpac);
        strcpy(&str[nSpac], "{\n");
        aStr1.Copy(str, strlen(str));
        strcpy(&str[nSpac], "  IN_P\n");
        aStr1.Copy(str, strlen(str));
        nSpac += 2;
        buf1 += 3;
        char s[64];
        s[0] = 0;
        int n = 0;
        bool bFlush = true;
        while (true)
        {
          if (buf1[0] == '_' || buf1[0] == '\n' || buf1[0] == 0xd || buf1[0] == 0x20 || buf1[0] == 9)
          {
            s[n] = 0;
            bool bValid = true;
            if (!strcmp(s, "C0"))
              strcpy(&str[nSpac], "IN_C0\n");
            else
            if (!strcmp(s, "C1"))
              strcpy(&str[nSpac], "IN_C1\n");
            else
            if (!strcmp(s, "N"))
              strcpy(&str[nSpac], "IN_N\n");
            else
            if (!strcmp(s, "TN"))
              strcpy(&str[nSpac], "IN_TN\n");
            else
            if (!strcmp(s, "T0"))
              strcpy(&str[nSpac], "IN_T0\n");
            else
            if (!strcmp(s, "T1"))
              strcpy(&str[nSpac], "IN_T1\n");
            else
            if (!strcmp(s, "T2"))
              strcpy(&str[nSpac], "IN_T2\n");
            else
            if (!strcmp(s, "T3"))
              strcpy(&str[nSpac], "IN_T3\n");
            else
            if (!strcmp(s, "T4"))
              strcpy(&str[nSpac], "IN_T4\n");
            else
            if (!strcmp(s, "TANG"))
              strcpy(&str[nSpac], "IN_TANG\n");
            else
            if (!strcmp(s, "P") || !strcmp(s, "TNORMVEC"))
              bValid = false;
            else
            if (!strcmp(s, "TANGVECS") || !strcmp(s, "TNORMVEC"))
            {
              bFlush = false;
              break;
            }
            else
            {
              assert(0);
              bValid = 0;
            }
            if (bValid)
              aStr1.Copy(str, strlen(str));

            if (buf1[0] == '\n' || buf1[0] == 0xd || buf1[0] == 0x20 || buf1[0] == 9)
            {
              strcpy(&str[nSpac-2], "};\n");
              aStr1.Copy(str, strlen(str));
              break;
            }
            buf1++;
            n = 0;
          }
          s[n++] = *buf1;
          buf1++;
        }
        if (!bFlush)
          continue;
      }

      char *buf2 = strstr(buf, "OUT_");
      char *buff2 = buf2;
      if (buf2)
      {
        int nSpac = 0;
        char *b = buf2-1;
        while (*b == 0x20 || *b == 9)
        {
          nSpac++;
          b--;
        }
        b++;
        char str[256];
        memcpy(str, b, nSpac);
        strcpy(&str[nSpac], "struct vertout\n");
        aStr2.Copy(str, strlen(str));
        strcpy(&str[nSpac], "{\n");
        aStr2.Copy(str, strlen(str));
        strcpy(&str[nSpac], "  OUT_P\n");
        aStr2.Copy(str, strlen(str));
        nSpac += 2;
        buf2 += 4;
        char s[64];
        s[0] = 0;
        int n = 0;
        while (true)
        {
          if (buf2[0] == '_' || buf2[0] == '\n' || buf2[0] == 0xd || buf2[0] == 0x20 || buf2[0] == 9)
          {
            s[n] = 0;
            bool bValid = true;
            if (!strcmp(s, "C0"))
              strcpy(&str[nSpac], "OUT_C0\n");
            else
            if (!strcmp(s, "C1"))
              strcpy(&str[nSpac], "OUT_C1\n");
            else
            if (!strcmp(s, "T0"))
              strcpy(&str[nSpac], "OUT_T0\n");
            else
            if (!strcmp(s, "T02"))
              strcpy(&str[nSpac], "OUT_T0_2\n");
            else
            if (!strcmp(s, "T1"))
              strcpy(&str[nSpac], "OUT_T1\n");
            else
            if (!strcmp(s, "T12"))
              strcpy(&str[nSpac], "OUT_T1_2\n");
            else
            if (!strcmp(s, "T2"))
              strcpy(&str[nSpac], "OUT_T2\n");
            else
            if (!strcmp(s, "T22"))
              strcpy(&str[nSpac], "OUT_T2_2\n");
            else
            if (!strcmp(s, "T3"))
              strcpy(&str[nSpac], "OUT_T3\n");
            else
            if (!strcmp(s, "T32"))
              strcpy(&str[nSpac], "OUT_T3_2\n");
            else
            if (!strcmp(s, "T4"))
              strcpy(&str[nSpac], "OUT_T4\n");
            else
            if (!strcmp(s, "T5"))
              strcpy(&str[nSpac], "OUT_T5\n");
            else
            if (!strcmp(s, "T6"))
              strcpy(&str[nSpac], "OUT_T6\n");
            else
            if (!strcmp(s, "T7"))
              strcpy(&str[nSpac], "OUT_T7\n");
            else
            if (!strcmp(s, "P"))
              bValid = false;
            else
            {
              assert(0);
              bValid = 0;
            }
            if (bValid)
              aStr2.Copy(str, strlen(str));

            if (buf2[0] == '\n' || buf2[0] == 0xd || buf2[0] == 0x20 || buf2[0] == 9)
            {
              strcpy(&str[nSpac-2], "};\n");
              aStr2.Copy(str, strlen(str));
              break;
            }
            buf2++;
            n = 0;
          }
          s[n++] = *buf2;
          buf2++;
        }
      }
      if (!buf1 || !buf2)
        continue;
      char *bufNew = new char[strlen(buf)+aStr1.Num()+aStr2.Num()];
      memcpy(bufNew, buf, buff1-buf);
      if (aStr1.Num())
        memcpy(&bufNew[buff1-buf], &aStr1[0], aStr1.Num());
      if (aStr2.Num())
        memcpy(&bufNew[buff1-buf+aStr1.Num()], &aStr2[0], aStr2.Num());
      strcpy(&bufNew[buff1-buf+aStr1.Num()+aStr2.Num()], buf2);
      
      fp = iSystem->GetIPak()->FOpen(nmf, "w");
      iSystem->GetIPak()->FWrite(bufNew, 1, strlen(bufNew), fp);
      iSystem->GetIPak()->FClose(fp);
      delete [] bufNew;

    } while (iSystem->GetIPak()->FindNext( handle, &fileinfo ) != -1);

    iSystem->GetIPak()->FindClose (handle);
  }*/
  /*{
    handle = iSystem->GetIPak()->FindFirst ("Shaders\\HWScripts\\Declarations\\CGPShaders\\*.crycg", &fileinfo);
    if (handle == -1)
    {
      assert(0);
    }

    do
    {
      if (fileinfo.name[0] == '.')
        continue;

      if (fileinfo.attrib & _A_SUBDIR)
        continue;

      strcpy(nmf, "Shaders\\HWScripts\\Declarations\\CGPShaders\\");
      strcat(nmf, fileinfo.name);

      FILE *fp = iSystem->GetIPak()->FOpen(nmf, "r");
      if (!fp)
        continue;

      iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
      int ln = iSystem->GetIPak()->FTell(fp);
      if (!ln)
      {
        iSystem->GetIPak()->FClose(fp);
        continue;
      }
      char *buf = new char [ln+1];
      if (!buf)
      {
        iSystem->GetIPak()->FClose(fp);
      }

      iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
      ln = iSystem->GetIPak()->FRead(buf, 1, ln, fp);
      iSystem->GetIPak()->FClose(fp);
      buf[ln] = 0;

      TArray <char> aStr2;
      char *buf2 = strstr(buf, "OUT_");
      char *buff2 = buf2;
      if (buf2)
      {
        int nSpac = 0;
        char *b = buf2-1;
        while (*b == 0x20 || *b == 9)
        {
          nSpac++;
          b--;
        }
        b++;
        char str[256];
        strcpy(str, "struct vertout\n");
        aStr2.Copy(str, strlen(str));
        memcpy(str, b, nSpac);
        strcpy(&str[nSpac], "{\n");
        aStr2.Copy(str, strlen(str));
        strcpy(&str[nSpac], "  ");
        nSpac += 2;
        buf2 += 4;
        char s[64];
        s[0] = 0;
        int n = 0;
        while (true)
        {
          if (buf2[0] == '_' || buf2[0] == '\n' || buf2[0] == 0xd || buf2[0] == 0x20 || buf2[0] == 9)
          {
            s[n] = 0;
            bool bValid = true;
            if (!strcmp(s, "C0"))
              strcpy(&str[nSpac], "OUT_C0\n");
            else
            if (!strcmp(s, "C1"))
              strcpy(&str[nSpac], "OUT_C1\n");
            else
            if (!strcmp(s, "T0"))
              strcpy(&str[nSpac], "OUT_T0\n");
            else
            if (!strcmp(s, "T02"))
              strcpy(&str[nSpac], "OUT_T0_2\n");
            else
            if (!strcmp(s, "T1"))
              strcpy(&str[nSpac], "OUT_T1\n");
            else
            if (!strcmp(s, "T12"))
              strcpy(&str[nSpac], "OUT_T1_2\n");
            else
            if (!strcmp(s, "T2"))
              strcpy(&str[nSpac], "OUT_T2\n");
            else
            if (!strcmp(s, "T22"))
              strcpy(&str[nSpac], "OUT_T2_2\n");
            else
            if (!strcmp(s, "T3"))
              strcpy(&str[nSpac], "OUT_T3\n");
            else
            if (!strcmp(s, "T32"))
              strcpy(&str[nSpac], "OUT_T3_2\n");
            else
            if (!strcmp(s, "T4"))
              strcpy(&str[nSpac], "OUT_T4\n");
            else
            if (!strcmp(s, "T5"))
              strcpy(&str[nSpac], "OUT_T5\n");
            else
            if (!strcmp(s, "T6"))
              strcpy(&str[nSpac], "OUT_T6\n");
            else
            if (!strcmp(s, "T7"))
              strcpy(&str[nSpac], "OUT_T7\n");
            else
            if (!strcmp(s, "P"))
              bValid = false;
            else
            {
              assert(0);
              bValid = 0;
            }
            if (bValid)
              aStr2.Copy(str, strlen(str));

            if (buf2[0] == '\n' || buf2[0] == 0xd || buf2[0] == 0x20 || buf2[0] == 9)
            {
              strcpy(&str[nSpac-2], "};\n");
              aStr2.Copy(str, strlen(str));
              break;
            }
            buf2++;
            n = 0;
          }
          s[n++] = *buf2;
          buf2++;
        }
      }
      if (!buf2)
        continue;
      char *bufNew = new char[strlen(buf)+aStr2.Num()];
      memcpy(bufNew, buf, buff2-buf);
      if (aStr2.Num())
        memcpy(&bufNew[buff2-buf], &aStr2[0], aStr2.Num());
      strcpy(&bufNew[buff2-buf+aStr2.Num()], buf2);
      
      fp = iSystem->GetIPak()->FOpen(nmf, "w");
      iSystem->GetIPak()->FWrite(bufNew, 1, strlen(bufNew), fp);
      iSystem->GetIPak()->FClose(fp);
      delete [] bufNew;

    } while (iSystem->GetIPak()->FindNext( handle, &fileinfo ) != -1);

    iSystem->GetIPak()->FindClose (handle);
  }*/



  handle = iSystem->GetIPak()->FindFirst (ddir, &fileinfo);
  if (handle == -1)
    return n;

  do
  {
    if (fileinfo.name[0] == '.')
      continue;

    if (fileinfo.attrib & _A_SUBDIR)
    {
      char ddd[256];
      sprintf(ddd, "%s%s/", drn, fileinfo.name);

      n = mfLoadSubdir(ddd, n);
      continue;
    }

    strcpy(nmf, drn);
    strcat(nmf, fileinfo.name);
    len = strlen(nmf);
    while (len && nmf[len] != '.')
      len--;
    if (len <= 0)
      continue;
    if (strnicmp(&nmf[len], ".csl", 4))
      continue;
    if (nmf[len+4])
    {
      if (!stricmp(&nmf[len+4], "new"))
      {
        if (CRenderer::CV_r_usehwshaders!=2)
          continue;
      }
      else
      if (!stricmp(&nmf[len+4], "old"))
      {
        if (CRenderer::CV_r_usehwshaders!=1)
          continue;
      }
      else
        continue;
    }

    char *pFinalScript = mfScriptForFileName(nmf, NULL, 0);
    if (!pFinalScript)
      continue;
    char Er[1024];
    sprintf(Er, "File '%s' script error!\n", nmf);
    gShObjectNotFound = Er;
    mfScanScript(pFinalScript, n);
    gShObjectNotFound = NULL;
    delete [] pFinalScript;

    FILE *status = iSystem->GetIPak()->FOpen(nmf, "rb");
    if (status)
    {
      FILETIME writetime = iSystem->GetIPak()->GetModificationTime(status);
      iSystem->GetIPak()->FClose (status);
      m_WriteTime[m_CurEfsNum][n] = writetime;
    }
    m_FileNames[m_CurEfsNum][n] = nmf;
    m_nFrameReload[m_CurEfsNum][n] = m_nFrameForceReload;

    n++;
    if (n == MAX_EF_FILES)
      return n;

  } while (iSystem->GetIPak()->FindNext( handle, &fileinfo ) != -1);

  iSystem->GetIPak()->FindClose (handle);

  return n;
}

void CShader::mfLoadFromFiles (int num) 
{
  int n = 0;
  char dir[256];

  m_NumFiles[num] = 0;
  m_CurEfsNum = num;
  UsePath("", m_ShadersPath[num], dir);
  ConvertDOSToUnixName(dir, dir);

  if (m_RefEfs[num])
  {
    ShaderFilesMapItor itor=m_RefEfs[num]->begin();
    while(itor!=m_RefEfs[num]->end())
    {
      SAFE_DELETE (itor->second);
      itor++;
    }
    m_RefEfs[num]->clear();
    SAFE_DELETE (m_RefEfs[num]);
  }
  
  if (num == 1)
    strcpy(m_HWPath, dir);

  if (!num)
    iLog->Log("\n  Load all common shader scripts (scanning directory '%s')...\n", dir);
  else
  if (num == 1)
    iLog->Log("\n  Load HW-specific shader scripts (scanning directory '%s')...\n", dir);

  m_RefEfs[num] = new ShaderFilesMap;
  n = mfLoadSubdir(dir, n);
  if (!n)
  {
    Warning( 0,0,"Warning: Shaders couldn't be found in directory '%s'", dir);
    m_NumFiles[num] = 0;
    return;
  }
  if (n==MAX_EF_FILES)
  {
    Warning( 0,0,"Warning: MAX_EF_FILES were hit (truncation list)\n");
    m_NumFiles[num] = n;
  }

  iLog->Log("  %d Shader files found.\n\n", n);
  m_NumFiles[num] = n;

  if (!m_NumFiles[num])
  {
    Warning( 0,0,"Warning: No Shaders text loaded!\n");
    return;
  }
}


char *CShader::mfFindInAllText (char *name, char *&pBuf, SShader *shGen, uint64 nMaskGen)
{
  char *saved = NULL;

  if (!m_RefEfs[m_CurEfsNum])
    return NULL;

  SRefEfs *fe;
  ShaderFilesMapItor it = m_RefEfs[m_CurEfsNum]->find(name);
  if (it == m_RefEfs[m_CurEfsNum]->end())
    fe = NULL;
  else
    fe = it->second;
  if (!fe)
    return NULL;

  if (fe)
  {
    char *pFinalScript;
    if (m_nFrameReload[m_CurEfsNum][fe->m_Ind] != m_nFrameForceReload)
    {
      m_nFrameReload[m_CurEfsNum][fe->m_Ind] = m_nFrameForceReload;
      pFinalScript = mfRescanScript(m_CurEfsNum, fe->m_Ind, shGen, nMaskGen);
      delete [] pFinalScript;
      ShaderFilesMapItor it = m_RefEfs[m_CurEfsNum]->find(name);
      if (it == m_RefEfs[m_CurEfsNum]->end())
        fe = NULL;
      else
        fe = it->second;
      if (!fe)
        return NULL;
    }
    pFinalScript = mfScriptForFileName(m_FileNames[m_CurEfsNum][fe->m_Ind].c_str(), shGen, nMaskGen);
    if (!pFinalScript)
      return NULL;
    sFE = fe;
    pBuf = pFinalScript;
    if (!shGen)
    {
      pFinalScript[fe->m_Offset+fe->m_Size] = 0;
      return &pFinalScript[fe->m_Offset];
    }
    return pFinalScript;
  }
  return NULL;
}

void CShader::mfRemoveFromHash(SShader *ef)
{
  LoadedShadersMapItor it = m_RefEfsLoaded.find(ef->m_Name.c_str());
  if (it != m_RefEfsLoaded.end())
  {
    m_RefEfsLoaded.erase(ef->m_Name.c_str());
  }
}

void CShader::mfAddToHash(char *name, SShader *ef)
{
  char nameEf[256];
  strncpy(nameEf, name, 256);
  strlwr(nameEf);
  SRefEfsLoaded fe;
  fe.m_Ef = ef;
  m_RefEfsLoaded.insert(LoadedShadersMapItor::value_type(nameEf, fe));
  ef->m_Name = nameEf;
}

void CShader::mfAddToHashLwr(char *nameEf, SShader *ef)
{
  SRefEfsLoaded fe;
  fe.m_Ef = ef;
  m_RefEfsLoaded.insert(LoadedShadersMapItor::value_type(nameEf, fe));
  ef->m_Name = nameEf;
}


SShader *CShader::mfSpawn (char *name, SShader *ef, SShader *efGen, uint64 nMaskGen)
{
  char Er[1024];
  char *BackEr;
  char *txt;
  int BackCurEfsNum;
  char *pScriptBuf;

  gShObjectNotFound = NULL;
  BackCurEfsNum = m_CurEfsNum;

  // First search in HW specific directory
  m_CurEfsNum = 1;
  txt = mfFindInAllText(name, pScriptBuf, efGen, nMaskGen);
  if (!txt)
  {
    // At last search in Common directory
    if (!txt)
    {
      m_CurEfsNum = 0;
      txt = mfFindInAllText(name, pScriptBuf, efGen, nMaskGen);
    }
  }

  if (txt)
  {
    // compile:
    BackEr = gShObjectNotFound;
    sprintf(Er, "Shader '%s' script error!\n", name);
    gShObjectNotFound = Er;
    m_pCurScript = pScriptBuf;
    SShader *ef1;
    if (efGen)
      ef1 = mfCompileShader(ef, txt);
    else
      ef1 = mfCompile(ef, txt);
    if (ef1)
      ef1->m_WriteTime = m_WriteTime[m_CurEfsNum][sFE->m_Ind];
    else
      assert(0);
    gShObjectNotFound = BackEr;
    delete [] pScriptBuf;
    m_CurEfsNum = BackCurEfsNum;

    return ef1;
  }

  m_CurEfsNum = BackCurEfsNum;

  if (CRenderer::CV_r_logloadshaders)
    Warning( 0,0,"WARNING: Shader '%s' couldn't be found!", name);

  return NULL;
}

SShader *CShader::mfForName (const char *nameSh, EShClass Class, int flags, const SInputShaderResources *Res, uint64 nMaskGen)
{
  SShader *ef, *efGen, *ef1;
  int id;
  char name[256];

  if (!nameSh || !nameSh[0])
  {
    Warning( 0,0,"Warning: CShader::mfForName: NULL name\n");
    gRenDev->m_cEF.m_DefaultShader->AddRef();
    return gRenDev->m_cEF.m_DefaultShader;
  }
  char nameEf[256];
  mfShaderNameForAlias(nameSh, nameEf, 256);

  ef = NULL;
  efGen = NULL;
  //StripExtension(nameEf, name);
  strcpy(name, nameEf);
  ConvertDOSToUnixName(name, name);
  strlwr(name);

  // Check if this shader already loaded
  SRefEfsLoaded *fe, *feGen;
  LoadedShadersMapItor it = m_RefEfsLoaded.find(name);
  if (it == m_RefEfsLoaded.end())
    fe = NULL;
  else
    fe = &it->second;
  char nameNew[256];
  if (fe && fe->m_Ef && fe->m_Ef->m_ShaderGenParams)
  {
    efGen = fe->m_Ef;
    ef = efGen;
    feGen = fe;
    sprintf(nameNew, "%s(%I64x)", name, nMaskGen);
    it = m_RefEfsLoaded.find(nameNew);
    if (it == m_RefEfsLoaded.end())
      fe = NULL;
    else
      fe = &it->second;
    if (fe)
    {
      assert(fe->m_Ef->m_nMaskGen == nMaskGen);
      assert(fe->m_Ef->m_pGenShader == efGen);
    }
  }

  if (fe && !(flags & SF_RELOAD))
  {
    fe->m_Ef->AddRef();
    fe->m_Ef->m_Flags |= flags;
    return fe->m_Ef;
  }
  if (fe)
  {
    ef = fe->m_Ef;
    ef->mfFree();
  }

  if (!ef)
  {
    ef = mfNewShader(Class, -1);
    if (!ef)
      return m_DefaultShader;

    // Add current hash reference
    mfAddToHashLwr(name, ef);
  }

  if (!efGen)
  {
    sprintf(nameNew, "Shaders\\%s.ext", name);
    FILE *fp = iSystem->GetIPak()->FOpen(nameNew, "rb");
    if (fp)
    {
      efGen = ef;
      ef->m_ShaderGenParams = new SShaderGen;
      iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
      int ln = iSystem->GetIPak()->FTell(fp);
      char *buf = new char [ln+1];
      if (buf)
      {
        buf[ln] = 0;
        iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
        iSystem->GetIPak()->FRead(buf, 1, ln, fp);
        iSystem->GetIPak()->FClose(fp);
        mfCompileShaderGen(ef, ef->m_ShaderGenParams, buf);
      }
      else
      {
        efGen = NULL;
        iSystem->GetIPak()->FClose(fp);
      }
    }
  }
  if (!(flags & SF_RELOAD))
  {
    if (efGen)
    {
      sprintf(nameNew, "%s(%I64x)", name, nMaskGen);
      ef = mfNewShader(Class, -1);
      if (!ef)
        return m_DefaultShader;

      // Add current hash reference
      mfAddToHashLwr(nameNew, ef);
      ef->m_nMaskGen = nMaskGen;
      ef->m_pGenShader = efGen;
    }
    if (efGen && ef)
    {
      assert(efGen != ef);
      if (!efGen->m_DerivedShaders)
        efGen->m_DerivedShaders = new TArray<SShader *>;
      efGen->m_DerivedShaders->AddElem(ef);
    }
  }
  id = ef->m_Id;

#ifndef NULL_RENDERER
  // Check for the new cryFX format
#ifdef USE_FX
  sprintf(nameNew, "%sCryFX/%s.cryFX", m_ShadersPath[1], name);
  FILE *fp = iSystem->GetIPak()->FOpen(nameNew, "rb");
  if (fp)
  {
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
    int len = iSystem->GetIPak()->FTell(fp);

    TArray<char> custMacros;
    if (efGen && efGen->m_ShaderGenParams)
    {
      int i;
      SShaderGen *shG = efGen->m_ShaderGenParams;
      for (i=0; i<shG->m_BitMask.Num(); i++)
      {
        if (shG->m_BitMask[i]->m_Mask & nMaskGen)
        {
          char macro[256];
          sprintf(macro, "#define %s 0x%I64x\n", shG->m_BitMask[i]->m_ParamName.c_str(), shG->m_BitMask[i]->m_Mask);
          int size = strlen(macro);
          int nOffs = custMacros.Num();
          custMacros.Grow(size);
          memcpy(&custMacros[nOffs], macro, size);
        }
      }
    }
    int size = custMacros.Num();
    char *buf = new char [len+size+1];
    if (!buf)
    {
      iSystem->GetIPak()->FClose(fp);
      Warning( 0,0,"Error: Can't allocate %d bytes for shader file '%s'\n", len+1, nameNew);
      return NULL;
    }
    memcpy(buf, &custMacros[0], custMacros.Num());
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
    len = iSystem->GetIPak()->FRead(&buf[size], 1, len, fp);
    iSystem->GetIPak()->FClose(fp);
    buf[len+size] = 0;
    RemoveCR(buf);
    ef1 = mfParseFX(buf, ef, efGen, nMaskGen);
  }
  else
#endif
#endif
  {
    ef1 = mfSpawn(name, ef, efGen, nMaskGen);
  }
  if (ef1 == ef)
  {
    ef->m_Flags |= flags;
    return ef;
  }

  ef->m_Flags |= EF_NOTFOUND;

  ef->m_Passes.Reserve(1);
  ef->m_Passes[0].mfAddNewTexUnits(1);

  ef->m_Passes[0].m_TUnits[0].m_TexPic = gRenDev->m_TexMan->m_Text_NoTexture;
  gRenDev->m_TexMan->m_Text_NoTexture->AddRef();
  if (ef->m_Passes[0].m_TUnits[0].m_TexPic)
    ef->m_Passes[0].m_TUnits[0].m_TexPic->m_Flags2 |= FT2_DIFFUSETEXTURE;
  if (Res && Res->m_LMaterial)
    ef->m_Flags |= EF_NEEDNORMALS;

  ef->m_Flags |= EF_COMPILEDLAYERS | flags;

  mfConstruct(ef);

  ef->m_Passes[0].m_RenderState = GS_DEPTHWRITE;
  
  return ef;
}


