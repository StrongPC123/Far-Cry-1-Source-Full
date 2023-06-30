/*=============================================================================
  ShaderCore.cpp : implementation of the Shaders manager.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "I3DEngine.h"
#include "CryHeaders.h"
#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#include <io.h>
#elif defined(LINUX)

#endif


SShader *CShader::m_DefaultShader;
#ifndef NULL_RENDERER
SShader *CShader::m_ShaderVFog;
SShader *CShader::m_ShaderVFogCaust;
SShader *CShader::m_ShaderFog;
SShader *CShader::m_ShaderFogCaust;
SShader *CShader::m_ShaderFog_FP;
SShader *CShader::m_ShaderFogCaust_FP;
SShader *CShader::m_ShaderStateNoCull;
SShader *CShader::m_ZBuffPassShader;
SShader *CShader::m_ShadowMapShader;
SShader *CShader::m_ShaderHDRProcess;
SShader *CShader::m_GlareShader;
SShader *CShader::m_ShaderSunFlares;
SShader *CShader::m_ShaderLightStyles;
SShader *CShader::m_ShaderCGPShaders;
SShader *CShader::m_ShaderCGVProgramms;
#else
SShaderItem CShader::m_DefaultShaderItem;
#endif
    
int SArrayPointer::m_CurEnabled;
int SArrayPointer::m_LastEnabled;
int SArrayPointer::m_CurEnabledPass;
int SArrayPointer::m_LastEnabledPass;

TArray<SShader *> SShader::m_Shaders_known;
TArray<SRenderShaderResources *> SShader::m_ShaderResources_known;
TArray <CLightStyle *> CLightStyle::m_LStyles;

CVProgram *CVProgram::m_LastVP;
int CVProgram::m_LastTypeVP;
int CVProgram::m_LastLTypeVP;

CPShader *CPShader::m_LastVP;
int CPShader::m_LastTypeVP;
int CPShader::m_LastLTypeVP;
int CVProgram::m_FrameObj;

bool gbRgb;

//=================================================================================================

int SShader::GetTexId()
{
  STexPic *tp = (STexPic *)GetBaseTexture(NULL, NULL);
  if (!tp)
    return -1;
  return tp->GetTextureID();
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

int SShader::mfSize()
{
  int i;

  int nSize = sizeof(SShader);
  nSize += m_Name.size();
  if (m_Templates)
    nSize += m_Templates->Size();
  for (i=0; i<m_Passes.GetSize(); i++)
  {
    nSize += m_Passes[i].Size();
  }
  for (i=0; i<m_HWConditions.GetSize(); i++)
  {
    nSize += m_HWConditions[i].Size();
  }
  nSize += m_HWTechniques.GetSize() * sizeof(SShaderTechnique *);
  for (i=0; i<m_HWTechniques.Num(); i++)
  {
    nSize += m_HWTechniques[i]->Size();
  }
  if (m_Deforms)
    nSize += m_Deforms->GetSize() * sizeof(SDeform);
  nSize += m_PublicParams.GetMemoryUsage();
  if (m_NormGen)
    nSize += m_NormGen->Size();

  if (m_FogInfo)
    nSize += m_FogInfo->Size();

  if (m_Sky)
    nSize += m_Sky->Size();

  if (m_Flares)
    nSize += m_Flares->Size();

  nSize += sizeof(CRendElement *) * m_REs.GetSize();
  for (i=0; i<m_REs.Num(); i++)
  {
    nSize += m_REs[i]->Size();
  }
  if (m_EvalLights)
    nSize += m_EvalLights->Size();
  if (m_State)
    nSize += m_State->Size();

  return nSize;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

SSkyInfo::~SSkyInfo()
{
  for (int i=0; i<3; i++)
  {
    if (m_SkyBox[i])
      m_SkyBox[i]->Release(false);
  }
}

void SShader::mfFree()
{
  int i;
  
  for (i=0; i<m_REs.Num(); i++)
  {
    SAFE_DELETE(m_REs[i]);
  }
  m_REs.Free();

  SAFE_DELETE(m_EvalLights);
  SAFE_DELETE(m_NormGen);
  SAFE_DELETE(m_FogInfo);
  SAFE_DELETE(m_Sky);
  if (m_State)
  {
    SAFE_DELETE(m_State->m_Stencil);
    SAFE_DELETE(m_State);
  }
  
  for (i=0; i<m_Passes.Num(); i++)
  {
    SShaderPass *sl = &m_Passes[i];
    sl->mfFree();
  }
  m_Passes.Free();
  
  if (m_Deforms)
  {
    delete m_Deforms;
    m_Deforms = NULL;
  }
  for (i=0; i<m_HWTechniques.Num(); i++)
  {
    SAFE_DELETE(m_HWTechniques[i]);
  }
  m_HWTechniques.Free();

  for (i=0; i<m_HWConditions.Num(); i++)
  {
    SHWConditions *hc = &m_HWConditions[i];
    if (hc->m_Vars)
      delete [] hc->m_Vars;
  }
  m_HWConditions.Free();
  
  if (m_Templates)
  {
    m_Templates->mfFree(this);
    m_Templates = NULL;
  }
  if (m_ShaderGenParams)
  {
    for (i=0; i<m_ShaderGenParams->m_BitMask.Num(); i++)
    {
      SShaderGenBit *shgb = m_ShaderGenParams->m_BitMask[i];
      SAFE_DELETE(shgb);
    }
    SAFE_DELETE(m_ShaderGenParams);
  }
  m_PublicParams.Free();
  m_Flags &= ~(EF_POLYGONOFFSET | EF_HASVSHADER);
  m_Flags3 &= ~(EF3_CLIPPLANE | EF3_NODRAW);
  m_DLDFlags = 0;
  m_DefaultVProgram = NULL;
}

SShader::~SShader()
{
  gRenDev->m_cEF.mfRemoveFromHash(this);

  mfFree();

  SShader::m_Shaders_known[m_Id] = NULL;
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4311 )						// I believe the int cast below is okay.
#endif

SShader& SShader::operator = (const SShader& src)
{
  int i;

  mfFree();
  
  int Offs = (int)&(((SShader *)0)->m_Id);
  byte *d = (byte *)this;
  byte *s = (byte *)&src;
  memcpy(&d[Offs], &s[Offs], sizeof(SShader)-Offs);

  m_Name = src.m_Name;

  if (src.m_REs.Num())
  {
    m_REs.Create(src.m_REs.Num());
    for (i=0; i<src.m_REs.Num(); i++)
    {
      if (src.m_REs[i])
        m_REs[i] = src.m_REs[i]->mfCopyConstruct();
    }
  }
  if (src.m_HWTechniques.Num())
  {
    m_HWTechniques.Create(src.m_HWTechniques.Num());
    for (i=0; i<src.m_HWTechniques.Num(); i++)
    {
#ifdef DEBUGALLOC
#undef new
#endif
      m_HWTechniques[i] = new SShaderTechnique;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
      *m_HWTechniques[i] = *src.m_HWTechniques[i];
    }
  }

  m_PublicParams.Copy(src.m_PublicParams);

  if (src.m_Passes.Num())
  {
    m_Passes.Create(src.m_Passes.Num());
    for (i=0; i<src.m_Passes.Num(); i++)
    {
      const SShaderPass *s = &src.m_Passes[i];
      SShaderPass *d = &m_Passes[i];
      *d = *s;
    }
  }
  if (src.m_Deforms)
  {
    m_Deforms = new TArray<SDeform>;
    m_Deforms->Create(src.m_Deforms->Num());
    memcpy(&m_Deforms->Get(0), &src.m_Deforms->Get(0), sizeof(SDeform)*m_Deforms->Num());
  }
  if (src.m_HWConditions.Num())
  {
    m_HWConditions.Create(src.m_HWConditions.Num());
    for (i=0; i<src.m_HWConditions.Num(); i++)
    {
      const SHWConditions *hc = &src.m_HWConditions[i];
      SHWConditions *hcd = &m_HWConditions[i];
      *hcd = *hc;
      if (hc->m_Vars)
      {
        hcd->m_Vars = new CVarCond[hc->m_NumVars];
        memcpy(hcd->m_Vars, hc->m_Vars, sizeof(CVarCond)*hc->m_NumVars);
      }
    }
  }

  m_Templates = NULL;

  return *this;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

SShaderPassHW::SShaderPassHW()
{
  m_ePassType = eSHP_General;
  m_VProgram = NULL;
  m_FShader = NULL;
  m_Deforms = NULL;
  m_CGFSParamsNoObj = NULL;
  m_CGFSParamsObj = NULL;
  m_MatrixOps = NULL;
  m_LightFlags = 0;
  m_LMFlags = 0;
  m_nAmbMaxLights = gRenDev->m_bDeviceSupports_PS2X ? NUM_PPLIGHTS_PERPASS_PS2X : NUM_PPLIGHTS_PERPASS_PS30;
}

STexPic *SShader::mfFindBaseTexture(TArray<SShaderPass>& Passes, int *nPass, int *nTU, int nT)
{
  int np, nt;
  STexPic *tx;
  np = nt = -1;
  bool bOpaq = ((m_Flags2 & EF2_HASOPAQUE) != 0);

  for (np=0; np<Passes.Num(); np++)
  {
    bool bFound = false;
    for (nt=0; nt<Passes[np].m_TUnits.Num(); nt++)
    {
      if (bOpaq)
      {
        if (Passes[np].m_TUnits[nt].m_nFlags & FTU_OPAQUE)
          bFound = true;
      }
      else
      {
        if (!nT)
        {
          if ((Passes[np].m_TUnits[nt].m_TexPic && (Passes[np].m_TUnits[nt].m_TexPic->m_Flags2 & FT2_DIFFUSETEXTURE)) ||
              (Passes[np].m_TUnits[nt].m_AnimInfo && (Passes[np].m_TUnits[nt].m_AnimInfo->m_TexPics[0]->m_Flags2 & FT2_DIFFUSETEXTURE)) )
            bFound = true;
        }
        else
        if (Passes[np].m_TUnits[nt].m_TexPic || Passes[np].m_TUnits[nt].m_AnimInfo)
          bFound = true;
      }
      if (bFound)
        break;
    }
    if (bFound)
      break;
  }
  if (np==Passes.Num() || nt==Passes[np].m_TUnits.Num() || (!Passes[np].m_TUnits[nt].m_TexPic && !Passes[np].m_TUnits[nt].m_AnimInfo))
  {
    np = nt = -1;
    tx = NULL;
  }
  else
  {
    if (Passes[np].m_TUnits[nt].m_TexPic)
      tx = Passes[np].m_TUnits[nt].m_TexPic;
    else
      tx = Passes[np].m_TUnits[nt].m_AnimInfo->m_TexPics[0];
  }
  if (nPass)
    *nPass = np;
  if (nTU)
    *nTU = nt;
  
  return tx;
}

STexPic *SShader::mfFindBaseTexture(TArray<SShaderPassHW>& Passes, int *nPass, int *nTU, int nT)
{
  int np, nt;
  STexPic *tx;
  np = nt = -1;
  bool bOpaq = ((m_Flags2 & EF2_HASOPAQUE) != 0);

  for (np=0; np<Passes.Num(); np++)
  {
    bool bFound = false;
    for (nt=0; nt<Passes[np].m_TUnits.Num(); nt++)
    {
      if (bOpaq)
      {
        if (Passes[np].m_TUnits[nt].m_nFlags & FTU_OPAQUE)
          bFound = true;
      }
      else
      {
        if (!nT)
        {
          if ((Passes[np].m_TUnits[nt].m_TexPic && (Passes[np].m_TUnits[nt].m_TexPic->m_Flags2 & FT2_DIFFUSETEXTURE)) ||
              (Passes[np].m_TUnits[nt].m_AnimInfo && (Passes[np].m_TUnits[nt].m_AnimInfo->m_TexPics[0]->m_Flags2 & FT2_DIFFUSETEXTURE)) )
            bFound = true;
        }
        else
        if (Passes[np].m_TUnits[nt].m_TexPic || Passes[np].m_TUnits[nt].m_AnimInfo)
          bFound = true;
      }
      if (bFound)
        break;
    }
    if (bFound)
      break;
  }
  if (np==Passes.Num() || nt==Passes[np].m_TUnits.Num() || (!Passes[np].m_TUnits[nt].m_TexPic && !Passes[np].m_TUnits[nt].m_AnimInfo))
  {
    np = nt = -1;
    tx = NULL;
  }
  else
  {
    if (Passes[np].m_TUnits[nt].m_TexPic)
      tx = Passes[np].m_TUnits[nt].m_TexPic;
    else
      tx = Passes[np].m_TUnits[nt].m_AnimInfo->m_TexPics[0];
  }
  if (nPass)
    *nPass = np;
  if (nTU)
    *nTU = nt;

  return tx;
}

ITexPic *SShader::GetBaseTexture(int *nPass, int *nTU)
{
  for (int nT=0; nT<2; nT++)
  {
    STexPic *tx = mfFindBaseTexture(m_Passes, nPass, nTU, nT);
    if (tx)
      return tx;
    for (int i=0; i<m_HWTechniques.Num(); i++)
    {
      SShaderTechnique *hw = m_HWTechniques[i];
      tx = mfFindBaseTexture(hw->m_Passes, nPass, nTU, nT);
      if (tx)
        return tx;
    }
  }
  if (nPass)
    *nPass = -1;
  if (nTU)
    *nTU = -1;
  return NULL;
}

unsigned int SShader::GetUsedTextureTypes (void)
{
  uint nMask = 0;
  int i, j, n;
  SShaderPass *ps;
  SShaderPassHW *psHW;

  for (i=0; i<m_Passes.Num(); i++)
  {
    ps = &m_Passes[i];
    for (j=0; j<ps->m_TUnits.Num(); j++)
    {
      if (ps->m_TUnits[j].m_TexPic && ps->m_TUnits[j].m_TexPic->m_Bind < EFTT_MAX)
        nMask |= 1<<ps->m_TUnits[j].m_TexPic->m_Bind;
    }
  }
  for (n=0; n<m_HWTechniques.Num(); n++)
  {
    SShaderTechnique *pHT = m_HWTechniques[n];
    for (i=0; i<pHT->m_Passes.Num(); i++)
    {
      psHW = &pHT->m_Passes[i];
      for (j=0; j<psHW->m_TUnits.Num(); j++)
      {
        if (psHW->m_TUnits[j].m_TexPic && psHW->m_TUnits[j].m_TexPic->m_Bind < EFTT_MAX)
          nMask |= 1<<psHW->m_TUnits[j].m_TexPic->m_Bind;
      }
    }
  }

  return nMask;
}

//================================================================================

void CShader::mfClearShaders (TArray<SShader *> &Efs, int *Nums)
{
  int i;

  if (!(*Nums))
    return;

  for (i=0; i<*Nums; i++)
  {
    if (Efs[i])
    {
      if (CRenderer::CV_r_printmemoryleaks)
        iLog->Log("Warning: CShader::mfClearAll: Shader %s was not deleted (%d)", Efs[i]->GetName(), Efs[i]->m_nRefCounter);
      //Efs[i]->Release();
    }
  }
  *Nums = 0;
}

SSunFlare::~SSunFlare()
{
  if (m_Tex)
  {
    m_Tex->Release(false);
    m_Tex = NULL;
  }
}

void CShader::mfClearAll (void)
{
  int i;
  
  if (gRenDev->m_cEF.m_bInitialized)
  {
    mfClearShaders(SShader::m_Shaders_known, &m_Nums);
  }
  SShader::m_Shaders_known.Free();

  for (i=0; i<SShader::m_ShaderResources_known.Num(); i++)
  {
    SRenderShaderResources *pSR = SShader::m_ShaderResources_known[i];
    if (!pSR)
      continue;
    if (i)
    {
      if (CRenderer::CV_r_printmemoryleaks)
        iLog->Log("Warning: CShader::mfClearAll: Shader resource 0x%x was not deleted", pSR);
    }
    delete pSR;
  }
  SShader::m_ShaderResources_known.Free();

  CSunFlares::m_CurFlares = NULL;
  for (i=0; i<CSunFlares::m_SunFlares.Num(); i++)
  {
    delete CSunFlares::m_SunFlares[i];
  }
  CSunFlares::m_SunFlares.Free();

  for (i=0; i<SArrayPointer::m_Arrays.Num(); i++)
  {
    SArrayPointer *ap = SArrayPointer::m_Arrays[i];
    delete ap;
  }
  SArrayPointer::m_Arrays.Free();

  for (i=0; i<SParamComp::m_ParamComps.Num(); i++)
  {
    delete SParamComp::m_ParamComps[i];
  }
  SParamComp::m_ParamComps.Free();

#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)

  SAFE_RELEASE(gRenDev->m_RP.m_RCDetail);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSprites_Heat);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSprites_FV);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSprites);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSun);
  SAFE_RELEASE(gRenDev->m_RP.m_RCFog);
  SAFE_RELEASE(gRenDev->m_RP.m_VPDetail);
  SAFE_RELEASE(gRenDev->m_RP.m_VPFog);
  SAFE_RELEASE(gRenDev->m_RP.m_VPTransformTexture);
  SAFE_RELEASE(gRenDev->m_RP.m_VPPlantBendingSpr);
  SAFE_RELEASE(gRenDev->m_RP.m_VPPlantBendingSpr_FV);
  SAFE_RELEASE(gRenDev->m_RP.m_RCBlur);
  SAFE_RELEASE(gRenDev->m_RP.m_VPBlur);

  for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
  {
    if (CVProgram::m_VPrograms[i] && CRenderer::CV_r_printmemoryleaks)
      iLog->Log("Warning: CShader::mfClearAll: Vertex shader %s was not deleted", CVProgram::m_VPrograms[i]->m_Name.c_str());
    delete CVProgram::m_VPrograms[i];
  }
  CVProgram::m_VPrograms.Free();
  
  for (i=0; i<CPShader::m_PShaders.Num(); i++)
  {
    if (CPShader::m_PShaders[i] && CRenderer::CV_r_printmemoryleaks)
      iLog->Log("Warning: CShader::mfClearAll: Pixel shader %s was not deleted", CPShader::m_PShaders[i]->m_Name.c_str());
    delete CPShader::m_PShaders[i];
  }
  CPShader::m_PShaders.Free();
#endif

  for (i=0; i<CLightStyle::m_LStyles.Num(); i++)
  {
    delete CLightStyle::m_LStyles[i];
  }
  CLightStyle::m_LStyles.Free();

  for (i=0; i<SLightMaterial::known_materials.Num(); i++)
  {
    if (SLightMaterial::known_materials[i] && SLightMaterial::known_materials[i]->name[0] == '$')
      iLog->Log("Warning: CShader::mfClearAll: Light material %s was not deleted (%d)", SLightMaterial::known_materials[i]->name, SLightMaterial::known_materials[i]->m_nRefCounter);
    SAFE_DELETE(SLightMaterial::known_materials[i])
  }
  SLightMaterial::known_materials.Free();
  
  gRenDev->m_cEF.m_bInitialized = false;
	gRenDev->m_cEF.m_KnownTemplates.Free();
}

void CShader::mfShutdown(void)
{
  int i, j;

  mfStartScriptPreprocess();
  mfUnregisterDefaultTemplates();

  if (m_DefaultShader)
  {
    m_DefaultShader->Release(true);
    m_DefaultShader = NULL;
  }
#ifndef NULL_RENDERER
  if (m_ShaderVFog)
  {
    m_ShaderVFog->Release(true);
    m_ShaderVFog = NULL;
  }
  SAFE_RELEASE_FORCE(m_ShaderVFogCaust);
  if (m_ShaderFog_FP != m_ShaderFog)
  {
    SAFE_RELEASE_FORCE(m_ShaderFog);
    SAFE_RELEASE_FORCE(m_ShaderFog_FP);
  }
  else
  {
    SAFE_RELEASE_FORCE(m_ShaderFog);
    m_ShaderFog_FP = NULL;
  }

  if (m_ShaderFogCaust_FP != m_ShaderFogCaust)
  {
    SAFE_RELEASE_FORCE(m_ShaderFogCaust);
    SAFE_RELEASE_FORCE(m_ShaderFogCaust_FP);
  }
  else
  {
    SAFE_RELEASE_FORCE(m_ShaderFogCaust);
    m_ShaderFogCaust_FP = NULL;
  }

  SAFE_RELEASE_FORCE(m_ShaderStateNoCull);
  SAFE_RELEASE_FORCE(m_ZBuffPassShader);
  SAFE_RELEASE_FORCE(m_ShadowMapShader);
  SAFE_RELEASE_FORCE(m_ShaderHDRProcess);
  SAFE_RELEASE_FORCE(m_GlareShader);
  //SAFE_RELEASE_FORCE(m_ShaderSunFlares); // Releases in 3dengine
  SAFE_RELEASE_FORCE(m_ShaderLightStyles);
#if !defined(PS2) && !defined(GC) && !defined (NULL_RENDERER)
  SAFE_RELEASE_FORCE(m_ShaderCGPShaders);
  SAFE_RELEASE_FORCE(m_ShaderCGVProgramms);
#endif

#endif
  SAFE_RELEASE(gRenDev->m_RP.m_RCDetail);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSprites_Heat);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSprites_FV);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSprites);
  SAFE_RELEASE(gRenDev->m_RP.m_RCSun);
  SAFE_RELEASE(gRenDev->m_RP.m_RCFog);
  SAFE_RELEASE(gRenDev->m_RP.m_VPDetail);
  SAFE_RELEASE(gRenDev->m_RP.m_VPFog);
  SAFE_RELEASE(gRenDev->m_RP.m_VPTransformTexture);
  SAFE_RELEASE(gRenDev->m_RP.m_VPPlantBendingSpr);
  SAFE_RELEASE(gRenDev->m_RP.m_RCBlur);
  SAFE_RELEASE(gRenDev->m_RP.m_VPBlur);

  for (i=0; i<2; i++)
  {
    for (j=0; j<MAX_EF_FILES; j++)
    {
      if (!m_FileNames[i][j].empty())
        m_FileNames[i][j] = "";
    }

    if (m_RefEfs[i])
    {
  		ShaderFilesMapItor itor=m_RefEfs[i]->begin();
      while(itor!=m_RefEfs[i]->end())
      {
        SAFE_DELETE (itor->second);
        itor++;
      }
      m_RefEfs[i]->clear();
      SAFE_DELETE (m_RefEfs[i]);
    }
  }

	/*LoadedShadersMapItor itor=m_RefEfsLoaded.begin();
  while(itor!=m_RefEfsLoaded.end())
  {
    SAFE_DELETE (itor->second);
    itor++;
  }*/
  m_RefEfsLoaded.clear();
}

SShader *CShader::mfCopyShader (SShader *ef)
{
  SShader *efc;

  efc = mfNewShader(eSH_Temp, -1);
  if (!efc)
    return NULL;

  int id = efc->m_Id;
  *efc = *ef;
  efc->m_Id = id;

  return efc;
}

void CShader::mfInit (void) 
{
  int i, j;

  gRenDev->m_TexMan->LoadDefaultTextures();

  if (!m_bInitialized)
  {
    //m_RefEfsLoaded.resize(1024);
    strcpy(m_ShadersPath[0], "Shaders/Scripts/");
    strcpy(m_ShadersPath[1], "Shaders/HWScripts/");
    strcpy(m_ShadersCache, "Shaders/Cache/");

    CShader::m_Nums = 0;
    CShader::m_MaxNums = (MAX_SHADERS - 256) - 1;
    CShader::m_FirstCopyNum = MAX_SHADERS - 256;
    SShader::m_Shaders_known.Alloc(MAX_SHADERS);
    memset(&SShader::m_Shaders_known[0], 0, sizeof(SShader *)*MAX_SHADERS);

    m_AliasNames.Free();
    fxParserInit();

#if !defined(NULL_RENDERER)
    //FILE *fp = fxopen("Shaders/Aliases.txt", "r");
    FILE *fp = iSystem->GetIPak()->FOpen("Shaders/Aliases.txt", "r");
    if (fp)
    {
      while (!iSystem->GetIPak()->FEof(fp))
      {
        char name[128];
        char alias[128];
        char str[256];
        iSystem->GetIPak()->FGets(str, 256, fp);
        if (sscanf(str, "%s %s", alias, name) == 2)
        {
          SNameAlias na;
          na.m_Alias = CName(alias, eFN_Add);
          na.m_Name = CName(name, eFN_Add);
          m_AliasNames.AddElem(na);
        }
      }
      iSystem->GetIPak()->FClose(fp);
    }

    int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;

    // load CustomAliases (by wat)
    m_CustomAliasNames.Free();
    fp = iSystem->GetIPak()->FOpen("Shaders/CustomAliases.txt", "r");
    if (fp)
    {
      char arg[3][256];
      bool bCond = true;
      while (!iSystem->GetIPak()->FEof(fp))
      {
        char str[256];
        iSystem->GetIPak()->FGets(str, 256, fp);

        char * p = strchr(str, '=');
        if(p)
        {
          memmove(p+2,p,strlen(p)+1);
          str[p-str] = ' '; str[p-str+1] = '='; str[p-str+2] = ' ';
        }

        int nArgs = sscanf(str, "%s %s %s", arg[0], arg[1], arg[2]);
        // parser comments
        for(int i = 0; i<nArgs; i++)
          if(arg[i][0] == ';')
          {
            nArgs = i;
            break;
          }
        // parser aliases
        if(nArgs == 2 && bCond)
        {
          SNameAlias na;
          na.m_Alias = CName(arg[0], eFN_Add);
          na.m_Name = CName(arg[1], eFN_Add);
          //if(m_CustomAliasNames.Find(na) == -1)
          m_CustomAliasNames.AddElem(na);
        }
        // parser conditions vars.
        else if(nArgs == 3 && arg[1][0] == '=')
        {
          ICVar * pVar = iConsole->GetCVar(arg[0]);
          if(pVar)
          {
            if( pVar->GetType()== CVAR_INT && pVar->GetIVal() != atoi(arg[2]))
              bCond = false;
            if( pVar->GetType()== CVAR_FLOAT && pVar->GetFVal() != float(atof(arg[2])))
              bCond = false;
          }
          else
          if(!strnicmp(arg[0], "GPU", 3))
          {
            if(!strnicmp(arg[2], "NV1X", 4) && nGPU != RFT_HW_GF2)
              bCond = false;
            else
            if(!strnicmp(arg[2], "NV2X", 4) && nGPU != RFT_HW_GF3)
              bCond = false;
            else
            if(!strnicmp(arg[2], "R300", 4) && nGPU != RFT_HW_RADEON)
              bCond = false;
            else
            if(!strnicmp(arg[2], "NV4X", 4) && nGPU != RFT_HW_NV4X)
              bCond = false;
          }
        }
        else if(arg[0][0] == '}')
          bCond = true;
      }
      iSystem->GetIPak()->FClose(fp);
    }
    for (i=0; i<m_CustomAliasNames.Num(); i++)
    {
      CName nm = m_CustomAliasNames[i].m_Name;
      bool bChanged = false;
      for (j=i+1; j<m_CustomAliasNames.Num(); j++)
      {
        if (m_CustomAliasNames[j].m_Alias == nm)
        {
          m_CustomAliasNames[i].m_Name = m_CustomAliasNames[j].m_Name;
          bChanged = true;
        }
      }
    }
    m_CustomAliasNames.Shrink();


    /* Old version:
    fp = iSystem->GetIPak()->FOpen("Shaders/CustomAliases.txt", "r");
    if (fp)
    {
      while (!iSystem->GetIPak()->FEof(fp))
      {
        char name[128];
        char alias[128];
        char str[256];
        iSystem->GetIPak()->FGets(str, 256, fp);
        if (sscanf(str, "%s %s", alias, name) == 2)
        {
          SNameAlias na;
          na.m_Alias = CName(alias, eFN_Add);
          na.m_Name = CName(name, eFN_Add);
          m_CustomAliasNames.AddElem(na);
        }
      }
      iSystem->GetIPak()->FClose(fp);
    }
    */
    /*{
      PackCache();
    }*/

    mfLoadFromFiles(0);
    if (CRenderer::CV_r_usehwshaders)
      mfLoadFromFiles(1);
    m_CurEfsNum = 0;
#endif //NULL_RENDERER
    mfSetDefaults();

    m_bInitialized = true;
  }
  for (i=0; i<m_Nums; i++)
  {
    SShader *ef = SShader::m_Shaders_known[i];
    if (ef)
    {
      for (int i=0; i<ef->m_REs.Num(); i++)
      {
        if (ef->m_REs[i])
          ef->m_REs[i]->mfBuildGeometry(ef);
      }
    }
  }

#if !defined(PS2) && !defined (GC)&& !defined (NULL_RENDERER)

  gRenDev->m_RP.m_RCDetail = CPShader::mfForName("CGRCDetailAtten");
  gRenDev->m_RP.m_RCSprites_Heat = CPShader::mfForName("CGRCHeat_TreesSprites");
  gRenDev->m_RP.m_RCSprites_FV = CPShader::mfForName("CGRCTreeSprites_FV");
  gRenDev->m_RP.m_RCSprites = CPShader::mfForName("CGRCTreeSprites");
  gRenDev->m_RP.m_RCSun    = CPShader::mfForName("CGRCSun");
  gRenDev->m_RP.m_RCFog    = CPShader::mfForName("CGRCFog");
  gRenDev->m_RP.m_VPDetail = CVProgram::mfForName("CGVProgDetail");
  gRenDev->m_RP.m_VPFog = CVProgram::mfForName("CGVProgFog");
  gRenDev->m_RP.m_VPTransformTexture = CVProgram::mfForName("CGVProgTransformTexture");
  gRenDev->m_RP.m_VPPlantBendingSpr = CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite");
  gRenDev->m_RP.m_VPPlantBendingSpr_FV = CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite_FV");
  gRenDev->m_RP.m_RCBlur    = CPShader::mfForName("CGRCBlur");
  gRenDev->m_RP.m_VPBlur    = CVProgram::mfForName("CGVProgBlur");
  if (gRenDev->GetFeatures() & RFT_HW_PS20)
  {
    gRenDev->m_RP.m_VPFur_NormGen  = CVProgram::mfForName("CGVProgFur_NormGen");
    gRenDev->m_RP.m_VPFur_OffsGen  = CVProgram::mfForName("CGVProgFur_OffsGen");
    gRenDev->m_RP.m_RCFur_NormGen  = CPShader::mfForName("CGRCFur_NormGen");
    gRenDev->m_RP.m_RCFur_OffsGen  = CPShader::mfForName("CGRCFur_OffsGen");
  }

#endif

}


void CShader::mfSetDefaults (void)
{
  static byte b = 0;

  SShader *ef;

  if (!b)
    iLog->Log("Construct Shader '<Default>'...");
  ef = mfNewShader(eSH_Misc, -1);
  m_DefaultShader = ef;
  mfAddToHash("<Default>", ef);
  ef->m_Passes.ReserveNew(1);
  ef->m_Passes[0].mfAddNewTexUnits(1);
  ef->m_Passes[0].m_TUnits[0].m_TexPic = gRenDev->m_TexMan->m_Text_White;
  ef->m_Passes[0].m_RenderState = GS_DEPTHWRITE;
  ef->m_Flags |= EF_SYSTEM;
  mfConstruct(ef);
  if (!b)
    iLog->LogPlus("ok");

#ifndef NULL_RENDERER
  if (!b)
    iLog->Log("Construct Shader 'ZBuffPass'...");
  ef = mfNewShader(eSH_Misc, -1);
  m_ZBuffPassShader = ef;
  mfAddToHash("ZBuffPass", ef);
  ef->m_Passes.ReserveNew(1);
  ef->m_Passes[0].mfAddNewTexUnits(1);
  ef->m_Passes[0].m_TUnits[0].m_TexPic = NULL;
  ef->m_Passes[0].m_RenderState = GS_DEPTHWRITE | GS_NOCOLMASK;
  ef->m_eSort = eS_ZBuff;
  ef->m_Passes[0].m_eEvalRGB=eERGB_Fixed;
  ef->m_Passes[0].m_FixedColor.dcolor=0;
  ef->m_Flags |= EF_SYSTEM;
  mfConstruct(ef);
  if (!b)
    iLog->LogPlus("ok");

  if (!b)
    iLog->Log("Construct Shader 'ShadowMap'...");
  ef = mfNewShader(eSH_Misc, -1);
  m_ShadowMapShader = ef;
  mfAddToHash("ShadowMap", ef); // maps shadows from shadow casters to the object
  ef->m_Passes.ReserveNew(1);
  ef->m_Passes[0].mfAddNewTexUnits(1);
  ef->m_Passes[0].m_TUnits[0].m_TexPic = NULL;
  ef->m_Passes[0].m_RenderState = GS_DEPTHWRITE | GS_NOCOLMASK;
  ef->m_eSort = eS_ShadowMap;
  ef->m_Flags |= EF_SYSTEM;
  mfConstruct(ef);
  if (!b)
    iLog->LogPlus("ok");

 // ef = mfForName("glassCM", eSH_Misc, EF_SYSTEM);

  if (!b)
    iLog->Log("Compile System Shader 'StateNoCull'...");
  ef = mfForName("StateNoCull", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderStateNoCull = ef;

  if (!b)
    iLog->Log("Compile System Shader 'HDRProcess'...");
  ef = mfForName("HDRProcess", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderHDRProcess = ef;

  if (!b)
    iLog->Log("Compile System Shader 'SunFlares'...");
  ef = mfForName("SunFlares", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
    {
      iLog->LogPlus("ok");
      iLog->Log("  %d Sun flares was parsed\n", CSunFlares::m_SunFlares.Num());
    }
  }
  m_ShaderSunFlares = ef;

  if (!b)
    iLog->Log("Compile System Shader 'LightStyles'...");
  ef = mfForName("LightStyles", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
    {
      iLog->LogPlus("ok");
      iLog->Log("  %d Light styles was parsed\n", CLightStyle::m_LStyles.Num());
    }
  }
  m_ShaderLightStyles = ef;

  if (!b)
    iLog->Log("Compile Glare Shader ...");
  m_GlareShader = mfForName("Glare", eSH_Screen, EF_SYSTEM);

#else
  m_DefaultShaderItem.m_pShader = m_DefaultShader;
  m_DefaultShaderItem.m_pShaderResources = new SRenderShaderResources;
#endif

#if !defined(PS2) && !defined(GC) && !defined (NULL_RENDERER)
  if (!b)
    iLog->Log("Compile System HW Shader 'CGVProgramms'...");
  ef = mfForName("CGVProgramms", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderCGVProgramms = ef;

  if (!b)
    iLog->Log("Compile System HW Shader 'CGPShaders'...");
  ef = mfForName("CGPShaders", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderCGPShaders = ef;
#endif

#ifndef NULL_RENDERER
  if (!b)
    iLog->Log("Compile System Shader 'TemplFog'...");
  ef = mfForName("TemplFog", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderFog = ef;

  if (!b)
    iLog->Log("Compile System Shader 'TemplVFog'...");
  ef = mfForName("TemplVFog", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderVFog = ef;

  if (!b)
    iLog->Log("Compile System Shader 'TemplFog_FP'...");
  ef = mfForName("TemplFog_FP", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderFog_FP = ef;

  if (!b)
    iLog->Log("Compile System Shader 'TemplFogCaustics'...");
  ef = mfForName("TemplFogCaustics", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderFogCaust = ef;

  if (!b)
    iLog->Log("Compile System Shader 'TemplVFogCaustics'...");
  ef = mfForName("TemplVFogCaustics", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderVFogCaust = ef;

  if (!b)
    iLog->Log("Compile System Shader 'TemplFogCaustics_FP'...");
  ef = mfForName("TemplFogCaustics_FP", eSH_Misc, EF_SYSTEM);
  if (!b)
  {
    if (!ef || (ef->m_Flags & EF_NOTFOUND))
      iLog->LogPlus("Fail.\n");
    else
      iLog->LogPlus("ok");
  }
  m_ShaderFogCaust_FP = ef;

  mfRegisterDefaultTemplates();
#endif

  if (!b)
    iLog->Log("\n");

  b = 1;

  //ef = mfForName("Illumination", eSH_Misc, EF_SYSTEM, NULL, 0x9f43);
  //ef = mfForName("TemplIllum", eSH_Misc, EF_SYSTEM, NULL, 0x9703);
  //ef = mfForName("TemplIllum", eSH_Misc, EF_SYSTEM, NULL, 0x9743);

  m_bInitialized = true;
}

//===================================================================

static byte pCounts[eSrcPointer_Max];

static void sSetFillColHW(SShader *ef, int& flt, EEvalRGB RGB[64], EEvalAlpha Alpha[64], byte bus[64][8], TArray<SShaderPassHW>& Layers)
{
  int i;
  for (i=0; i<64; i++)
  {
    if (!bus[i][0])
      break;
    if (Alpha[i] != eEALPHA_FromClient || RGB[i] != eERGB_FromClient)
      break;
  }
  if (!bus[i][0])
  {
    flt |= FLT_COL;
    for (i=0; i<64; i++)
    {
      if (!bus[i][0])
        break;
      Layers[i].m_eEvalRGB = eERGB_NoFill;
      Layers[i].m_eEvalAlpha = eEALPHA_NoFill;
    }
  }
}

static bool sNeedColorArray(EEvalRGB eRGB, EEvalAlpha eA)
{
  if ((eRGB == eERGB_Fixed || eRGB == eERGB_Identity || eRGB == eERGB_NoFill || eRGB == eERGB_Wave || eRGB == eERGB_Object || eRGB == eERGB_OneMinusObject || eRGB == eERGB_RE || eRGB == eERGB_OneMinusRE || eRGB == eERGB_World || eRGB == eERGB_Noise || eRGB == eERGB_Comps) && (eA == eEALPHA_Fixed || eA == eEALPHA_Identity || eA == eEALPHA_NoFill || eA == eEALPHA_Wave || eA == eEALPHA_Object || eA == eEALPHA_OneMinusObject || eA == eEALPHA_RE || eA == eEALPHA_OneMinusRE || eA == eEALPHA_World || eA == eEALPHA_Noise || eA == eEALPHA_Comps))
    return false;
  return true;
}

void CShader::mfOptimizeShaderHW(SShader *ef, TArray<SShaderPassHW>& Layers, int Stage)
{
  int i, j; 

  if (Stage == 0)
  {
    bool bOpaq = false;;
    for (i=0; i<Layers.Num(); i++)
    {
      for (j=0; j<Layers[i].m_Pointers.Num(); j++)
      {
        pCounts[Layers[i].m_Pointers[j]->ePT]++;
      }
      if (!Layers[i].m_TUnits.Num())
        continue;

      if (Layers[i].m_RenderState & GS_DEPTHWRITE)
        bOpaq = true;

      if (!Layers[i].m_TUnits[0].m_eGenTC)
        Layers[i].m_TUnits[0].m_eGenTC = eGTC_Base;
      
      if ((Layers[i].m_RenderState & GS_BLEND_MASK) && (Layers[0].m_RenderState & GS_BLEND_MASK))
      {
        if (ef->m_eSort==eS_Unknown)
          ef->m_eSort = (Layers[i].m_RenderState & GS_DEPTHWRITE) ? eS_SeeThrough : eS_Banner;
      }
      else
      {
        if (ef->m_eSort==eS_Unknown && !(Layers[i].m_RenderState & GS_NODEPTHTEST))
          ef->m_eSort = eS_Opaque;
      }
    }
    if (bOpaq && ef->m_eSort < eS_SeeThrough)
      ef->m_Flags2 |= EF2_OPAQUE;
  }
  else
  if (Stage == 1)
  {
    if (Layers.Num())
    {
      byte bus[64][8];
      byte bEx[64];
      EEvalAlpha Alpha[64];
      EEvalRGB RGB[64];
      memset(bEx, 0, 64);
      memset(&bus[0][0], 0, 64*8);
      for (i=0; i<Layers.Num(); i++)
      {
        if (Layers[i].m_Flags & SHPF_RADIOSITY)
          ef->m_nPreprocess |= FSPR_SCANLCM;
        Alpha[i] = Layers[i].m_eEvalAlpha;
        RGB[i] = Layers[i].m_eEvalRGB;
        for (j=0; j<Layers[i].m_TUnits.Num(); j++)
        {
          SShaderTexUnit *tl = &Layers[i].m_TUnits[j];
          if (!tl)
            continue;
          //if (!ef->m_ePreprocess)
          {
            if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvCMap)
              ef->m_nPreprocess |= FSPR_SCANCM;
            else
            if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvLCMap)
              ef->m_nPreprocess |= FSPR_SCANLCM;
            else
            if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvTex)
              ef->m_nPreprocess |= FSPR_SCANTEX;
            else
            if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvScr)
              ef->m_nPreprocess |= FSPR_SCANSCR;
            else // tiago: added
            if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_ScreenMap)
              ef->m_nPreprocess |= FSPR_SCREENTEXMAP;
            else // tiago: added
            if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_DofMap)
              ef->m_nPreprocess |= FSPR_DOFMAP;
            else
            if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_WaterMap)
              ef->m_nPreprocess |= FSPR_SCANTEXWATER;
            if (tl->m_TexPic)
            {
              if (tl->m_TexPic->m_Bind >= TO_CUSTOM_CUBE_MAP_FIRST && tl->m_TexPic->m_Bind <= TO_CUSTOM_CUBE_MAP_LAST)
                ef->m_nPreprocess |= FSPR_CUSTOMCM;
              else
              if (tl->m_TexPic->m_Bind >= TO_CUSTOM_TEXTURE_FIRST && tl->m_TexPic->m_Bind <= TO_CUSTOM_TEXTURE_LAST)
                ef->m_nPreprocess |= FSPR_CUSTOMTEXTURE;
            }          
          }

          bus[i][j] = tl->m_eGenTC;
          bEx[bus[i][j]]++;
          if (j)
            bEx[bus[i][j]] |= 0x80;
        }
      }
      int fl = ef->m_Flags;
      int flt = 0;
      if (ef->m_Flags & EF_NEEDNORMALS)
        flt |= FLT_N;

      if (bEx[eGTC_Base] && bEx[eGTC_LightMap] && !(bEx[eGTC_Base] & 0x80) && !(bEx[eGTC_LightMap] & 0x80))
      {
        flt |= FLT_SYSBASE | FLT_SYSLM;
      }
      else
      {
        if (bEx[eGTC_Base])
        {
          flt |= FLT_BASE;
          for (i=0; i<64; i++)
          {
            if (!bus[i][0])
              break;
            for (j=0; j<8; j++)
            {
              if (bus[i][j] == eGTC_Base)
                Layers[i].m_TUnits[j].m_eGenTC = eGTC_NoFill;
            }
          }
        }
        if (bEx[eGTC_LightMap])
        {
          flt |= FLT_LM;
          for (i=0; i<64; i++)
          {
            if (!bus[i][0])
              break;
            for (j=0; j<8; j++)
            {
              if (bus[i][j] == eGTC_LightMap)
                Layers[i].m_TUnits[j].m_eGenTC = eGTC_NoFill;
            }
          }
        }
      }
  
      sSetFillColHW(ef, flt, RGB, Alpha, bus, Layers);
      flt |= FLT_COL;

      //==========================================================
      // Find suitable vertex format for RenderElements
      if ((flt & FLT_N))
        m_bNeedNormal = true;
      TArray<SArrayPointer *> Pts;
      for (i=0; i<Layers.Num(); i++)
      {
        if (Layers[i].m_Deforms)
          m_bNeedSysBuf = true;
        for (j=0; j<Layers[i].m_TUnits.Num(); j++)
        {
          if (Layers[i].m_TUnits[j].m_TexPic && (Layers[i].m_TUnits[j].m_TexPic->m_eTT == eTT_Bumpmap))
            Layers[i].m_LMFlags |= LMF_BUMPMATERIAL;
        }
        int nT = 0;
        if (sNeedColorArray(Layers[i].m_eEvalRGB, Layers[i].m_eEvalAlpha))
          m_bNeedCol = true;
        for (j=0; j<Layers[i].m_Pointers.Num(); j++)
        {
          Pts.AddElem(Layers[i].m_Pointers[j]);
          switch(Layers[i].m_Pointers[j]->ePT)
          {
            case eSrcPointer_Binormal:
            case eSrcPointer_Tangent:
            case eSrcPointer_TNormal:
              Layers[i].m_Flags |= SHPF_TANGENTS;
              break;
            case eSrcPointer_TexLM:
              Layers[i].m_Flags |= SHPF_LMTC;
              m_nTC = max(m_nTC, 2);
              break;
          }
        }
        if (Layers[i].m_VProgram)
        {
          if (Layers[i].m_VProgram->mfHasPointer(eSrcPointer_Binormal))
          {
            Layers[i].m_Flags |= SHPF_TANGENTS;
            m_bNeedTangents = true;
          }
          if (Layers[i].m_VProgram->mfHasPointer(eSrcPointer_TNormal))
          {
            Layers[i].m_Flags |= SHPF_TANGENTS;
            m_bNeedTangents = true;
          }
          if (Layers[i].m_VProgram->mfHasPointer(eSrcPointer_Tangent))
          {
            Layers[i].m_Flags |= SHPF_TANGENTS;
            m_bNeedTangents = true;
          }
          if (Layers[i].m_VProgram->mfHasPointer(eSrcPointer_Color))
            m_bNeedCol = true;
          if (Layers[i].m_VProgram->mfHasPointer(eSrcPointer_SecColor))
            m_bNeedSecCol = true;
          if (Layers[i].m_VProgram->mfHasPointer(eSrcPointer_Normal))
            m_bNeedNormal = true;
          for (int n=1; n>=0; n--)
          {
            if (Layers[i].m_VProgram->mfHasPointer((ESrcPointer)(eSrcPointer_Tex+n)))
            {
              m_nTC = max(m_nTC, n+1);
              if (n)
                Layers[i].m_Flags |= SHPF_LMTC;
              break;
            }
          }
        }
      }
      for (i=0; i<ef->m_HWTechniques.Num(); i++)
      {
        for (j=0; j<ef->m_HWTechniques[i]->m_Pointers.Num(); j++)
        {
          Pts.AddElem(ef->m_HWTechniques[i]->m_Pointers[j]);
        }
      }
      for (i=0; i<Pts.Num(); i++)
      {
        switch (Pts[i]->ePT)
        {
          case eSrcPointer_Color:
            m_bNeedCol = true;
            break;
          case eSrcPointer_SecColor:
            m_bNeedSecCol = true;
            break;
          case eSrcPointer_NormLightVector:
          case eSrcPointer_LightVector:
          case eSrcPointer_HalfAngleVector:
          case eSrcPointer_Attenuation:
          case eSrcPointer_LAttenuationSpec0:
          case eSrcPointer_LAttenuationSpec1:
          case eSrcPointer_Refract:
          case eSrcPointer_Project:
          case eSrcPointer_ProjectTexture:
          case eSrcPointer_ProjectAttenFromCamera:
            m_bNeedSysBuf = true;
          case eSrcPointer_Binormal:
          case eSrcPointer_Tangent:
          case eSrcPointer_TNormal:
            m_bNeedTangents = true;
            break;
          case eSrcPointer_Normal:
            m_bNeedNormal = true;
            break;
          case eSrcPointer_Tex:
            m_nTC = max(m_nTC, 1);
            break;
        }
      }
      
      //==========================================================

      if (ef->m_eSort <= eS_Opaque)
        ef->m_Flags2 |= EF2_FOGOVERLAY1;
      else
      if (ef->m_Flags & EF_FOGSHADER)
        ef->m_Flags2 |= EF2_FOGOVERLAY2;
    }
  }
}

void CShader::mfOptimizeShader(SShader *ef, TArray<SShaderPass>& Layers, int Stage)
{
  int i, j; 

  if (Stage == 0)
  {
    bool bOpaq = false;;
    for (i=0; i<Layers.Num(); i++)
    {
      if (!Layers[i].m_TUnits.Num())
        continue;

      if (!(Layers[i].m_RenderState & GS_BLEND_MASK))
        bOpaq = true;

      if (!Layers[i].m_TUnits[0].m_eGenTC)
        Layers[i].m_TUnits[0].m_eGenTC = eGTC_Base;
      
      if ((Layers[i].m_RenderState & GS_BLEND_MASK) && (Layers[0].m_RenderState & GS_BLEND_MASK))
      {
        if (ef->m_eSort==eS_Unknown)
          ef->m_eSort = (Layers[i].m_RenderState & GS_DEPTHWRITE) ? eS_SeeThrough : eS_Banner;
      }
      else
      {
        if (ef->m_eSort==eS_Unknown && !(Layers[i].m_RenderState & GS_NODEPTHTEST))
          ef->m_eSort = eS_Opaque;
      }
    }
    if (bOpaq && ef->m_eSort != eS_SeeThrough)
      ef->m_Flags2 |= EF2_OPAQUE;
  }
  else
  if (Stage == 1)
  {
    if (Layers.Num())
    {      
      for (i=0; i<Layers.Num(); i++)
      {
        for (j=0; j<Layers[i].m_TUnits.Num(); j++)
        {
          SShaderTexUnit *tl = &Layers[i].m_TUnits[j];
          if (!tl)
            continue;
          if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvCMap)
            ef->m_nPreprocess |= FSPR_SCANCM;
          else
          if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvLCMap)
            ef->m_nPreprocess |= FSPR_SCANLCM;
          else
          if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvTex)
            ef->m_nPreprocess |= FSPR_SCANTEX;
          else
          if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_EnvScr)
            ef->m_nPreprocess |= FSPR_SCANSCR;
          else // tiago: added
          if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_ScreenMap)
            ef->m_nPreprocess |= FSPR_SCREENTEXMAP;
          else // tiago: added
          if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_DofMap)
            ef->m_nPreprocess |= FSPR_DOFMAP;
          else
          if (tl->m_TexPic == gRenDev->m_TexMan->m_Text_WaterMap)
            ef->m_nPreprocess |= FSPR_SCANTEXWATER;
          if (tl->m_TexPic)
          {
            if (tl->m_TexPic->m_Bind >= TO_CUSTOM_CUBE_MAP_FIRST && tl->m_TexPic->m_Bind <= TO_CUSTOM_CUBE_MAP_LAST)
              ef->m_nPreprocess |= FSPR_CUSTOMCM;
            else
            if (tl->m_TexPic->m_Bind >= TO_CUSTOM_TEXTURE_FIRST && tl->m_TexPic->m_Bind <= TO_CUSTOM_TEXTURE_LAST)
              ef->m_nPreprocess |= FSPR_CUSTOMTEXTURE;
          }          
        }
      }
      if (ef->m_REs.Num() && ef->m_REs[0]->mfGetType() == eDATA_AnimPolyBlend)
        ef->m_Flags2 |= EF2_CUSTOMANIMTEX;

      int fl = ef->m_Flags;
      int flt = 0;

      if (ef->m_Flags & EF_NEEDNORMALS)
        flt |= FLT_N;
    
      //==========================================================
      // Find suitable vertex format for RenderElements
      for (i=0; i<Layers.Num(); i++)
      {
        int nT = 0;
        if (sNeedColorArray(Layers[i].m_eEvalRGB, Layers[i].m_eEvalAlpha))
          m_bNeedCol = true;
        for (j=0; j<Layers[i].m_TUnits.Num(); j++)
        {
          SShaderTexUnit *tl = &Layers[i].m_TUnits[j];
          if (!tl)
            continue;
          if (tl->m_GTC || (tl->m_eGenTC != eGTC_Base && tl->m_eGenTC != eGTC_LightMap && tl->m_eGenTC != eGTC_NoFill && tl->m_eGenTC != eGTC_Projection))
            continue;
          nT++;
        }
        m_nTC = max(m_nTC, nT);
      }

      if (ef->m_eSort <= eS_Opaque)
        ef->m_Flags2 |= EF2_FOGOVERLAY1;
      else
      if (ef->m_Flags & EF_FOGSHADER)
        ef->m_Flags2 |= EF2_FOGOVERLAY2;
    }
  }
}

void CShader::mfConstruct (SShader *ef)
{
  int i;

  // Sky shader
  if (ef->m_Flags & EF_SKY)
  {
    ef->m_eSort = eS_Sky;
  }

  // Fog shader
  if (ef->m_Flags & EF_FOGSHADER)
    ef->m_eSort = eS_FogShader;

  // Default RGB/Alpha modes if not specified in script
  i = 0;
  if (ef->m_Passes.Num() && ef->m_Passes[0].m_TUnits.Num())
  {
    for (i=0; i<ef->m_Passes.Num(); i++)
    {
      if (!ef->m_Passes[i].m_TUnits[0].m_eColorOp)
        ef->m_Passes[i].m_TUnits[0].m_eColorOp = eCO_MODULATE;

      if (ef->m_Passes[i].m_eEvalRGB == eERGB_Fixed && (ef->m_Passes[i].m_FixedColor.dcolor & 0x00ffffff) == 0x00ffffff)
        ef->m_Passes[i].m_eEvalRGB = eERGB_Identity;
      if (ef->m_Passes[i].m_eEvalAlpha == eEALPHA_Fixed && ef->m_Passes[i].m_FixedColor.bcolor[3] == 255)
        ef->m_Passes[i].m_eEvalAlpha = eEALPHA_Identity;
      
      if (ef->m_REs.Num() && ef->m_REs[0]->mfGetType() == eDATA_AnimPolyBlend && ef->m_Passes[i].m_TUnits[0].m_AnimInfo)
        ef->m_Passes[i].m_RenderState |= GS_TEXANIM;
    }
  }

  if (ef->m_Deforms)
    ef->m_Flags |= EF_NEEDNORMALS;
  else
  {
    for (i=0; i<ef->m_Passes.Num(); i++)
    {
      if (!ef->m_Passes[i].m_TUnits.Num())
        continue;
      if (ef->m_Passes[i].m_TUnits[0].m_eGenTC > eGTC_Base)
        ef->m_Flags |= EF_NEEDNORMALS;
    }
  }

  i = ef->m_Passes.Num();

  memset(pCounts, 0, eSrcPointer_Max);
  for (i=0; i<ef->m_HWTechniques.Num(); i++)
  {
    mfOptimizeShaderHW(ef, ef->m_HWTechniques[i]->m_Passes, 0);
  }
  mfOptimizeShader(ef, ef->m_Passes, 0);

  pCounts[eSrcPointer_LightVector]+=pCounts[eSrcPointer_LAttenuationSpec0];
  pCounts[eSrcPointer_LightVector]+=pCounts[eSrcPointer_LAttenuationSpec1];
  pCounts[eSrcPointer_LightVector]+=pCounts[eSrcPointer_HalfAngleVector];
  pCounts[eSrcPointer_LightVector]+=pCounts[eSrcPointer_NormLightVector];
  if (pCounts[eSrcPointer_LightVector] > 1)
    ef->m_Flags3 |= EF3_PREPARELV;
  if (pCounts[eSrcPointer_HalfAngleVector] > 1)
    ef->m_Flags3 |= EF3_PREPAREHAV;
  if (pCounts[eSrcPointer_LAttenuationSpec0] > 1)
    ef->m_Flags3 |= EF3_PREPARELAS0;
  if (pCounts[eSrcPointer_LAttenuationSpec1] > 1)
    ef->m_Flags3 |= EF3_PREPARELAS1;

  i = ef->m_Passes.Num();
    
  if (ef->m_Flags & EF_SKY)
    ef->m_Flags2 |= EF2_NOCASTSHADOWS;

  m_bNeedSysBuf = false;
  m_bNeedCol = false;
  m_bNeedSecCol = false;
  m_bNeedTangents = false;
  m_bNeedNormal = false;
  m_nTC = 0;
  for (i=0; i<ef->m_HWTechniques.Num(); i++)
  {
    bool bTangs = m_bNeedTangents;
    bool bNormal = m_bNeedNormal;
    int nTC = m_nTC;
    m_bNeedTangents = false;
    m_bNeedNormal = false;
    m_nTC = 0;
    mfOptimizeShaderHW(ef, ef->m_HWTechniques[i]->m_Passes, 1);
    if (m_bNeedTangents)
      ef->m_HWTechniques[i]->m_Flags |= FHF_TANGENTS;
    if (m_nTC > 1)
      ef->m_HWTechniques[i]->m_Flags |= FHF_LMTC;
    m_bNeedTangents = max(m_bNeedTangents, bTangs);
    m_bNeedNormal = max(m_bNeedNormal, bNormal);
    m_nTC = max(m_nTC, nTC);
  }
  mfOptimizeShader(ef, ef->m_Passes, 1);

  int vf;
  if (m_bNeedTangents)
  {
    ef->m_Flags |= EF_NEEDTANGENTS;
    ef->m_LMFlags |= LMF_BUMPMATERIAL;
  }
  if (m_bNeedNormal || (ef->m_Flags2 & EF2_USELIGHTMATERIAL))
    ef->m_Flags |= EF_NEEDNORMALS;

  vf = VertFormatForComponents(m_bNeedCol, m_bNeedSecCol, m_bNeedNormal, m_nTC!=0);
  ef->m_VertexFormatId = vf;
  
  if (m_bNeedSysBuf)
    ef->m_Flags3 |= EF3_NEEDSYSBUF;

  //==========================================================

  if (ef->m_eSort == eS_Unknown)
    ef->m_eSort = eS_Opaque;


  if (ef->m_REs.Num() && ef->m_REs[0]->mfGetType() == eDATA_AnimPolyBlend)
    ef->m_Flags2 |= EF2_CUSTOMANIMTEX;

  if (ef->m_eSort == eS_Unknown)
    ef->m_eSort = eS_Opaque;
    
  if (ef->m_eSort <= eS_Opaque)
    ef->m_Flags2 |= EF2_FOGOVERLAY1;
  else
  if (ef->m_Flags & EF_FOGSHADER)
    ef->m_Flags2 |= EF2_FOGOVERLAY2;
}

SShader *CShader::mfNewShader(EShClass Class, int num)
{
  SShader *ef;
  int n;

  if (Class == eSH_Temp)
  {
    for (n=CShader::m_FirstCopyNum; n<MAX_SHADERS; n++)
    {
      if (!SShader::m_Shaders_known[n])
        goto create;
    }
    iConsole->Exit("MAX_TEMP_SHADERS hit\n");
    return NULL;
  }
  for (n=0; n<CShader::m_Nums; n++)
  {
    if (!SShader::m_Shaders_known[n])
      goto create;
  }
  if ((n=CShader::m_Nums) >= CShader::m_MaxNums)
  {
    iConsole->Exit("MAX_SHADERS hit\n");
    return NULL;
  }

  if (num<0)
    CShader::m_Nums++;
  else
    n = num;

create:
#ifdef DEBUGALLOC
#undef new
#endif
  ef = new SShader;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
  if (!ef)
  {
    iConsole->Exit("CShader::mfNewShader: Couldn't allocate shader %d\n", n);
    return m_DefaultShader;
  }

  SShader::m_Shaders_known[n] = ef;

  ef->m_Id = n;
  ef->m_nRefCounter = 1;

  ef->m_eClass = Class;

  return ef;
}


//=========================================================

bool CShader::mfUpdateMergeStatus(SShaderTechnique *hs, TArray<SCGParam4f> *p)
{
  for (int n=0; n<p->Num(); n++)
  {
    if (p->Get(n).m_Flags & PF_DONTALLOW_DYNMERGE)
    {
      hs->m_Flags |= FHF_NOMERGE;
      break;
    }
  }
  if (hs->m_Flags & FHF_NOMERGE)
    return true;
  return false;
}


//=========================================================================

bool SShader::mfIsValidTime(CCObject *obj, float curtime)
{
  int i, j;
  SShaderTexUnit *shm;
  SShaderPass *sfm;

  if (m_REs.Num())
    return m_REs[0]->mfIsValidTime(this, obj, curtime);
  else
  {
    for (i=0; i<m_Passes.Num(); i++)
    {
      sfm = &m_Passes[i];

      for (j=0; j<sfm->m_TUnits.Num(); j++)
      {
        shm = &sfm->m_TUnits[j];
        if (!shm->m_AnimInfo)
          continue;
        if (shm->m_AnimInfo->m_Time && shm->m_AnimInfo->m_TexPics.Num())
        {
          float t = curtime - obj->m_StartTime;
          int m = (int)(t / shm->m_AnimInfo->m_Time);
          if (shm->m_AnimInfo->m_bLoop)
            m = m % shm->m_AnimInfo->m_NumAnimTexs;
          else
          if (m < 0)
          {
            obj->m_StartTime = curtime;
            m = 0;
          }
          int n = 0;
          while (m)
          {
            STexPic *tx = shm->m_AnimInfo->m_TexPics[n];
            if (!tx)
              return false;
            m--;
            n++;
          }
        }
      }
    }
  }
  return true;
}

// Animating textures
void SShaderTexUnit::mfUpdate(void)
{
  if (m_AnimInfo && m_AnimInfo->m_Time && gRenDev->m_bPauseTimer==0)
  {
    assert(gRenDev->m_RP.m_RealTime>=0);
    uint m = (uint)(gRenDev->m_RP.m_RealTime / m_AnimInfo->m_Time) % m_AnimInfo->m_NumAnimTexs;
    assert(m<(uint)m_AnimInfo->m_TexPics.Num());
    m_TexPic = m_AnimInfo->m_TexPics[m];
  }
}

void SShaderTexUnit::mfUpdateAnim(CCObject *obj, int o)
{
  if (m_AnimInfo && m_AnimInfo->m_Time && m_AnimInfo->m_TexPics.Num())
  {
    float t = gRenDev->m_RP.m_RealTime - obj->m_StartTime;
    int m = (int)(t / m_AnimInfo->m_Time);
    m += o;
    if (m < 0)
    {
      obj->m_StartTime = gRenDev->m_RP.m_RealTime;
      m = 0;
    }
    if (m_AnimInfo->m_bLoop)
      m = m % m_AnimInfo->m_NumAnimTexs;
    int n = 0;
    STexPic *tx = NULL;
    while (m)
    {
      tx = m_AnimInfo->m_TexPics[n];
      if (!tx)
      {
        //obj->m_ObjFlags |= FOB_NOTVISIBLE;
        break;
      }
      n++;
      m--;
    }
    m_TexPic = tx;
  }
}

SEnvTexture *CShader::mfFindSuitableEnvLCMap(Vec3d& Pos, bool bMustExist, int RendFlags, float fDistToCam, CCObject *pObj)
{
  double time0 = 0;
  ticks(time0);

  SEnvTexture *cm = NULL;
  int i;
  
  float dist = 999999;
  int firstForUse = -1;
  int firstFree = -1;
  for (i=0; i<MAX_ENVLIGHTCUBEMAPS; i++)
  {
    SEnvTexture *cur = &gRenDev->m_TexMan->m_EnvLCMaps[i];
    Vec3d delta = cur->m_CamPos - Pos;
    float s = GetLengthSquared(delta);
    if (s < dist)
    {
      dist = s;
      firstForUse = i;
      if (!dist)
        break;
    }
    if (!cur->m_bReady && firstFree < 0)
      firstFree = i;
  }
  if (bMustExist)
  {
    if (firstForUse >= 0)
    {
      unticks(time0);
      gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
      return &gRenDev->m_TexMan->m_EnvLCMaps[firstForUse];
    }
    else
      return NULL;
  }

	float curTime = iTimer->GetCurrTime();
  int nUpdate = -2;
  dist = sqrtf(dist);
  if (bMustExist)
    nUpdate = -2;
  else
  if (dist > MAX_ENVLIGHTCUBEMAPSCANDIST_THRESHOLD)
  {
    if (firstFree >= 0)
      nUpdate = firstFree;
    else
      nUpdate = -1;
  }
  else
  {
    float fTimeInterval = max(fDistToCam, 1.0f) * CRenderer::CV_r_envlcmupdateinterval;
    float fDelta = curTime - gRenDev->m_TexMan->m_EnvLCMaps[firstForUse].m_TimeLastUsed;
    if (fDelta > fTimeInterval)
      nUpdate = firstForUse;
  }

  if (nUpdate == -2)
  {
    if (!bMustExist)
    {
      /*for (int i=0; i<6; i++)
      {
        if (gRenDev->m_TexMan->m_EnvLCMaps[firstForUse].m_nFrameCreated[i]>0 && gRenDev->GetFrameID()-gRenDev->m_TexMan->m_EnvLCMaps[firstForUse].m_nFrameCreated[i]>=CRenderer::CV_r_envlightcmupdatefrequence)
        {
          gRenDev->m_TexMan->m_EnvLCMaps[firstForUse].m_nFrameCreated[i] = -1;
          //gRenDev->m_TexMan->GetAverageColor(&gRenDev->m_TexMan->m_EnvLCMaps[firstForUse], i);
        }
      }*/
    }

    // No need to update (Up to date)
    unticks(time0);
    gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
    return &gRenDev->m_TexMan->m_EnvLCMaps[firstForUse];
  }
  if (nUpdate >= 0)
  {
    if (!(gRenDev->m_TexMan->m_EnvLCMaps[nUpdate].m_Tex->m_Flags & FT_ALLOCATED) || (fDistToCam <= MAX_ENVLIGHTCUBEMAPSCANDIST_UPDATE && gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime < 0.1f))
    {
      // Reuse old cube-map
      if (!(gRenDev->m_TexMan->m_EnvLCMaps[nUpdate].m_Tex->m_Flags & FT_ALLOCATED) && pObj)
      {
        UCol Amb;
        Amb.bcolor[0] = (byte)(pObj->m_AmbColor[0]*255.0f);
        Amb.bcolor[1] = (byte)(pObj->m_AmbColor[1]*255.0f);
        Amb.bcolor[2] = (byte)(pObj->m_AmbColor[2]*255.0f);
        Amb.bcolor[3] = 255;
        for (i=0; i<6; i++)
        {
          gRenDev->m_TexMan->m_EnvLCMaps[nUpdate].m_EnvColors[i].dcolor = Amb.dcolor;
        }
      }
      int n = nUpdate;
      gRenDev->m_TexMan->m_EnvLCMaps[n].m_TimeLastUsed = curTime;
      gRenDev->m_TexMan->m_EnvLCMaps[n].m_CamPos = Pos;
      gRenDev->m_TexMan->ScanEnvironmentCube(&gRenDev->m_TexMan->m_EnvLCMaps[n], RendFlags, CRenderer::CV_r_envlightcmsize, true);
    }
    unticks(time0);
    gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
    return &gRenDev->m_TexMan->m_EnvLCMaps[nUpdate];
  }

  // Find oldest slot
  dist = 0;
  int nOldest = -1;
  for (i=0; i<MAX_ENVLIGHTCUBEMAPS; i++)
  {
    SEnvTexture *cur = &gRenDev->m_TexMan->m_EnvLCMaps[i];
    if (dist < curTime-cur->m_TimeLastUsed && !cur->m_bInprogress)
    {
      dist = curTime - cur->m_TimeLastUsed;
      nOldest = i;
    }
  }
  if (nOldest < 0)
  {
    unticks(time0);
    gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
    return NULL;
  }
  int n = nOldest;
  gRenDev->m_TexMan->m_EnvLCMaps[n].m_TimeLastUsed = curTime;
  gRenDev->m_TexMan->m_EnvLCMaps[n].m_CamPos = Pos;
  // Fill box colors by nearest cube
  if (firstForUse >= 0)
  {
    for (i=0; i<6; i++)
    {
      gRenDev->m_TexMan->m_EnvLCMaps[n].m_EnvColors[i].dcolor = gRenDev->m_TexMan->m_EnvLCMaps[firstForUse].m_EnvColors[i].dcolor;
    }
  }
  // Start with positive X
  gRenDev->m_TexMan->m_EnvLCMaps[n].m_MaskReady = 0;
  gRenDev->m_TexMan->ScanEnvironmentCube(&gRenDev->m_TexMan->m_EnvLCMaps[n], RendFlags, CRenderer::CV_r_envlightcmsize, true);

  unticks(time0);
  gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
  return &gRenDev->m_TexMan->m_EnvLCMaps[n];
}

SEnvTexture *CShader::mfFindSuitableEnvCMap(Vec3d& Pos, bool bMustExist, int RendFlags, float fDistToCam)
{
  double time0 = 0;
  ticks(time0);

  SEnvTexture *cm = NULL;
  int i;
  
  float dist = 999999;
  int firstForUse = -1;
  int firstFree = -1;
  for (i=0; i<MAX_ENVCUBEMAPS; i++)
  {
    SEnvTexture *cur = &gRenDev->m_TexMan->m_EnvCMaps[i];
    Vec3d delta = cur->m_CamPos - Pos;
    float s = GetLengthSquared(delta);
    if (s < dist)
    {
      dist = s;
      firstForUse = i;
      if (!dist)
        break;
    }
    if (!(cur->m_Tex->m_Flags & FT_ALLOCATED) && firstFree < 0)
      firstFree = i;
  }

	float curTime = iTimer->GetCurrTime();
  int nUpdate = -2;
  float fTimeInterval = fDistToCam * CRenderer::CV_r_envcmupdateinterval + CRenderer::CV_r_envcmupdateinterval*0.5f;
  float fDelta = curTime - gRenDev->m_TexMan->m_EnvCMaps[firstForUse].m_TimeLastUsed;
  if (bMustExist)
    nUpdate = -2;
  else
  if (dist > MAX_ENVCUBEMAPSCANDIST_THRESHOLD)
  {
    if (firstFree >= 0)
      nUpdate = firstFree;
    else
      nUpdate = -1;
  }
  else
  if (fDelta > fTimeInterval)
    nUpdate = firstForUse;
  if (nUpdate == -2)
  {
    // No need to update (Up to date)
    unticks(time0);
    gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
    return &gRenDev->m_TexMan->m_EnvCMaps[firstForUse];
  }
  if (nUpdate >= 0)
  {
    if (!(gRenDev->m_TexMan->m_EnvCMaps[nUpdate].m_Tex->m_Flags & FT_ALLOCATED) || gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime < 0.1f)
    {
      int n = nUpdate;
      gRenDev->m_TexMan->m_EnvCMaps[n].m_TimeLastUsed = curTime;
      gRenDev->m_TexMan->m_EnvCMaps[n].m_CamPos = Pos;
      gRenDev->m_TexMan->ScanEnvironmentCube(&gRenDev->m_TexMan->m_EnvCMaps[n], RendFlags, -1, false);
    }
    unticks(time0);
    gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
    return &gRenDev->m_TexMan->m_EnvCMaps[nUpdate];
  }

  dist = 0;
  firstForUse = -1;
  for (i=0; i<MAX_ENVCUBEMAPS; i++)
  {
    SEnvTexture *cur = &gRenDev->m_TexMan->m_EnvCMaps[i];
    if (dist < curTime-cur->m_TimeLastUsed && !cur->m_bInprogress)
    {
      dist = curTime - cur->m_TimeLastUsed;
      firstForUse = i;
    }
  }
  if (firstForUse < 0)
  {
    unticks(time0);
    gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
    return NULL;
  }
  int n = firstForUse;
  gRenDev->m_TexMan->m_EnvCMaps[n].m_TimeLastUsed = curTime;
  gRenDev->m_TexMan->m_EnvCMaps[n].m_CamPos = Pos;
  gRenDev->m_TexMan->ScanEnvironmentCube(&gRenDev->m_TexMan->m_EnvCMaps[n], RendFlags, -1, false);

  unticks(time0);
  gRenDev->m_RP.m_PS.m_fEnvCMapUpdateTime += (float)(time0*1000.0*g_SecondsPerCycle);
  return &gRenDev->m_TexMan->m_EnvCMaps[n];
}

Vec3d sDeltAngles(Vec3d Ang0, Vec3d Ang1)
{
  Vec3d out;
  for (int i=0; i<3; i++)
  {
    float a0 = Ang0[i];
    a0 = (float)((360.0/65536) * ((int)(a0*(65536/360.0)) & 65535)); // angmod
    float a1 = Ang1[i];
    a1 = (float)((360.0/65536) * ((int)(a0*(65536/360.0)) & 65535));
    out[i] = a0 - a1;
  }
  return out;
}

SEnvTexture *CShader::mfFindSuitableEnvTex(Vec3d& Pos, Vec3d& Angs, bool bMustExist, int RendFlags, bool bUseExistingREs, SShader *pSH, SRenderShaderResources *pRes, CCObject *pObj, bool bReflect, CRendElement *pRE)
{
  SEnvTexture *cm = NULL;

  int i;
  float distO = 999999;
  float adist = 999999;
  int firstForUse = -1;
  Vec3d objPos;
  if (!pObj)
    bReflect = false;
  else
  {
    if (pRE)
      pRE->mfCenter(objPos, pObj);
    else
      objPos = pObj->GetTranslation();
  }
  float dist = 999999;
  for (i=0; i<MAX_ENVTEXTURES; i++)
  {
    SEnvTexture *cur = &gRenDev->m_TexMan->m_EnvTexts[i];
    if (cur->m_bReflected != bReflect)
      continue;
    float s = GetLengthSquared((cur->m_CamPos - Pos));
    Vec3d angDelta = sDeltAngles(Angs, cur->m_Angle);
    float a = GetLengthSquared(angDelta);
    float so = 0;
    if (bReflect)
      so = GetLengthSquared((cur->m_ObjPos - objPos));
    if (s < dist && a < adist && so < distO)
    {
      dist = s;
      adist = a;
      distO = so;
      firstForUse = i;
      if (!so && !s && !a)
        break;
    }
  }
  if (bMustExist && firstForUse >= 0)
    return &gRenDev->m_TexMan->m_EnvTexts[firstForUse];
  if (bReflect)
    dist = distO;
  //return NULL;
  float curTime = iTimer->GetCurrTime();
  if (firstForUse >= 0 && dist <= MAX_ENVTEXSCANDIST)
  {
    if (!bMustExist && curTime - gRenDev->m_TexMan->m_EnvTexts[firstForUse].m_TimeLastUsed > CRenderer::CV_r_envtexupdateinterval)
    {
      int n = firstForUse;
      gRenDev->m_TexMan->m_EnvTexts[n].m_TimeLastUsed = curTime;
      gRenDev->m_TexMan->m_EnvTexts[n].m_CamPos = Pos;
      gRenDev->m_TexMan->m_EnvTexts[n].m_Angle = Angs;
      gRenDev->m_TexMan->m_EnvTexts[n].m_ObjPos = objPos;
      gRenDev->m_TexMan->m_EnvTexts[n].m_bReflected = bReflect;
      gRenDev->m_TexMan->ScanEnvironmentTexture(&gRenDev->m_TexMan->m_EnvTexts[firstForUse], pSH, pRes, RendFlags, bUseExistingREs);
    }
    return &gRenDev->m_TexMan->m_EnvTexts[firstForUse];
  }
  if (bMustExist)
    return NULL;

  dist = 0;
  firstForUse = -1;
  for (i=0; i<MAX_ENVTEXTURES; i++)
  {
    SEnvTexture *cur = &gRenDev->m_TexMan->m_EnvTexts[i];
    if (dist < curTime-cur->m_TimeLastUsed && !cur->m_bInprogress)
    {
      dist = curTime - cur->m_TimeLastUsed;
      firstForUse = i;
    }
  }
  if (firstForUse < 0)
    return NULL;
  i = firstForUse;
  gRenDev->m_TexMan->m_EnvTexts[i].m_TimeLastUsed = curTime;
  gRenDev->m_TexMan->m_EnvTexts[i].m_CamPos = Pos;
  gRenDev->m_TexMan->m_EnvTexts[i].m_ObjPos = objPos;
  gRenDev->m_TexMan->m_EnvTexts[i].m_Angle = Angs;
  gRenDev->m_TexMan->m_EnvTexts[i].m_bReflected = bReflect;
  gRenDev->m_TexMan->ScanEnvironmentTexture(&gRenDev->m_TexMan->m_EnvTexts[i], pSH, pRes, RendFlags, bUseExistingREs);
  return &gRenDev->m_TexMan->m_EnvTexts[i];
}

//=================================================================================================

static float sFRand()
{
  return rand() / (FLOAT)RAND_MAX;
}

// Update TexGen and TexTransform matrices for current material texture
void SEfResTexture::Update(int nTU)
{
  int i;

  if (m_TexModificator.m_UpdateFlags & RBMF_NOUPDATE)
    return;

  gRenDev->m_RP.m_ShaderTexResources[nTU] = this;

  int nFrameID = gRenDev->GetFrameID();
  if (m_TexModificator.m_nFrameUpdated == nFrameID && m_TexModificator.m_nLastRecursionLevel == SRendItem::m_RecurseLevel)
  {
    gRenDev->m_RP.m_FlagsModificators |= m_TexModificator.m_UpdateFlags<<nTU;
    return;
  }
  m_TexModificator.m_nFrameUpdated = nFrameID;
  m_TexModificator.m_nLastRecursionLevel = SRendItem::m_RecurseLevel;
  m_TexModificator.m_UpdateFlags = 0;

  bool bTr = false;
  bool bTranspose = false;
  Plane Pl;
  Plane PlTr;
  if (m_TexModificator.m_Tiling[0] == 0)
    m_TexModificator.m_Tiling[0] = 1.0f;
  if (m_TexModificator.m_Tiling[1] == 0)
    m_TexModificator.m_Tiling[1] = 1.0f;

  if (m_TexModificator.m_eUMoveType != ETMM_NoChange || m_TexModificator.m_eVMoveType != ETMM_NoChange || m_TexModificator.m_eRotType != ETMR_NoChange 
   || m_TexModificator.m_Offs[0]!=0.0f || m_TexModificator.m_Offs[1]!=0.0f || m_TexModificator.m_Tiling[0]!=1.0f || m_TexModificator.m_Tiling[1]!=1.0f)
  {
    m_TexModificator.m_TexMatrix.SetIdentity();
    float fTime = gRenDev->m_RP.m_RealTime;

    bTr = true;

    switch(m_TexModificator.m_eRotType)
    {
      case ETMR_Fixed:
        {
          m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
            Matrix44(1,0,0,0,
            0,1,0,0,
            m_TexModificator.m_RotOscCenter[0],m_TexModificator.m_RotOscCenter[1],1,0,
            0,0,0,1);
          if (m_TexModificator.m_RotOscPhase[0])
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationX(Word2Degr(m_TexModificator.m_RotOscPhase[0])*PI/180.0f);
          if (m_TexModificator.m_RotOscPhase[1])
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationY(Word2Degr(m_TexModificator.m_RotOscPhase[1])*PI/180.0f);
          if (m_TexModificator.m_RotOscPhase[2])
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationZ(Word2Degr(m_TexModificator.m_RotOscPhase[2])*PI/180.0f);
          m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
            Matrix44(1,0,0,0,
            0,1,0,0,
            -m_TexModificator.m_RotOscCenter[0],-m_TexModificator.m_RotOscCenter[1],1,0,
            0,0,0,1);
        }
        break;

      case ETMR_Constant:
        {
          fTime *= 1000.0f;
          float fxAmp = Word2Degr(m_TexModificator.m_RotOscAmplitude[0]) * fTime * PI/180.0f;
          float fyAmp = Word2Degr(m_TexModificator.m_RotOscAmplitude[1]) * fTime * PI/180.0f;
          float fzAmp = Word2Degr(m_TexModificator.m_RotOscAmplitude[2]) * fTime * PI/180.0f;
          m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
            Matrix44(1,0,0,0,
            0,1,0,0,
            m_TexModificator.m_RotOscCenter[0],m_TexModificator.m_RotOscCenter[1],1,0,
            0,0,0,1);
          if (fxAmp)
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationX(fxAmp);
          if (fyAmp)
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationY(fyAmp);
          if (fzAmp)
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationZ(fzAmp);
          m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
            Matrix44(1,0,0,0,
            0,1,0,0,
            -m_TexModificator.m_RotOscCenter[0],-m_TexModificator.m_RotOscCenter[1],1,0,
            0,0,0,1);
        }
        break;

      case ETMR_Oscillated:
        {
          m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
            Matrix44(1,0,0,0,
            0,1,0,0,
            -m_TexModificator.m_RotOscCenter[0],-m_TexModificator.m_RotOscCenter[1],1,0,
            0,0,0,1);
			    float S_X = fTime * Word2Degr(m_TexModificator.m_RotOscRate[0]);
			    float d_X = Word2Degr(m_TexModificator.m_RotOscAmplitude[0]) * cry_sinf(2.0f * PI * ((S_X - cry_floorf(S_X)) + Word2Degr(m_TexModificator.m_RotOscPhase[0])));
          float S_Y = fTime * Word2Degr(m_TexModificator.m_RotOscRate[1]);
          float d_Y = Word2Degr(m_TexModificator.m_RotOscAmplitude[1]) * cry_sinf(2.0f * PI * ((S_Y - cry_floorf(S_Y)) + Word2Degr(m_TexModificator.m_RotOscPhase[1])));
          float S_Z = fTime * Word2Degr(m_TexModificator.m_RotOscRate[2]);
          float d_Z = Word2Degr(m_TexModificator.m_RotOscAmplitude[2]) * cry_sinf(2.0f * PI * ((S_Z - cry_floorf(S_Z)) + Word2Degr(m_TexModificator.m_RotOscPhase[2])));
          if (d_X)
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationX(d_X);
          if (d_Y)
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationY(d_Y);
          if (d_Z)
            m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationZ(d_Z);
          m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
            Matrix44(1,0,0,0,
            0,1,0,0,
            -m_TexModificator.m_RotOscCenter[0],-m_TexModificator.m_RotOscCenter[1],1,0,
            0,0,0,1);
        }
        break;
    }


    float Su = gRenDev->m_RP.m_RealTime * m_TexModificator.m_UOscRate;
    float Sv = gRenDev->m_RP.m_RealTime * m_TexModificator.m_VOscRate;
    switch(m_TexModificator.m_eUMoveType)
    {
      case ETMM_Pan:
        {
          float  du = m_TexModificator.m_UOscAmplitude * cry_sinf(2.0f * PI * (Su - cry_floorf(Su)) + 2.f * PI * m_TexModificator.m_UOscPhase);
          m_TexModificator.m_TexMatrix(2,0) = du;
        }
        break;
      case ETMM_Fixed:
        {
          float du = m_TexModificator.m_Offs[0];
          m_TexModificator.m_TexMatrix(2,0) = du;
        }
    	  break;
      case ETMM_Constant:
        {
          float du = m_TexModificator.m_UOscAmplitude * Su; //(Su - cry_floorf(Su));
          m_TexModificator.m_TexMatrix(2,0) = du;
        }
        break;
      case ETMM_Jitter:
        {
          if( m_TexModificator.m_LastUTime < 1.0f || m_TexModificator.m_LastUTime > Su + 1.0f )
            m_TexModificator.m_LastUTime = m_TexModificator.m_UOscPhase + cry_floorf(Su);
          if( Su-m_TexModificator.m_LastUTime > 1.0f )
          {
            m_TexModificator.m_CurrentUJitter = sFRand() * m_TexModificator.m_UOscAmplitude;
            m_TexModificator.m_LastUTime = m_TexModificator.m_UOscPhase + cry_floorf(Su);
          }
          m_TexModificator.m_TexMatrix(2,0) = m_TexModificator.m_CurrentUJitter;
        }
        break;
      case ETMM_Stretch:
        {
          float du = m_TexModificator.m_UOscAmplitude * cry_sinf(2.0f * PI * (Su - cry_floorf(Su)) + 2.0f * PI * m_TexModificator.m_UOscPhase);
          m_TexModificator.m_TexMatrix(0,0) = 1.0f+du;
        }
        break;
      case ETMM_StretchRepeat:
        {
          float du = m_TexModificator.m_UOscAmplitude * cry_sinf(0.5f * PI * (Su - cry_floorf(Su)) + 2.0f * PI * m_TexModificator.m_UOscPhase);
          m_TexModificator.m_TexMatrix(0,0) = 1.0f+du;
        }
        break;
    }

    switch(m_TexModificator.m_eVMoveType)
    {
      case ETMM_Pan:
        {
          float dv = m_TexModificator.m_VOscAmplitude * cry_sinf(2.0f * PI * (Sv - cry_floorf(Sv)) + 2.0f * PI * m_TexModificator.m_VOscPhase);
          m_TexModificator.m_TexMatrix(2,1) = dv;
        }
        break;
      case ETMM_Fixed:
        {
          float dv = m_TexModificator.m_Offs[1];
          m_TexModificator.m_TexMatrix(2,1) = dv;
        }
    	  break;
      case ETMM_Constant:
        {
          float dv = m_TexModificator.m_VOscAmplitude * Sv; //(Sv - cry_floorf(Sv));
          m_TexModificator.m_TexMatrix(2,1) = dv;
        }
        break;
      case ETMM_Jitter:
        {
          if( m_TexModificator.m_LastVTime < 1.0f || m_TexModificator.m_LastVTime > Sv + 1.0f )
            m_TexModificator.m_LastVTime = m_TexModificator.m_VOscPhase + cry_floorf(Sv);
          if( Sv-m_TexModificator.m_LastVTime > 1.0f )
          {
            m_TexModificator.m_CurrentVJitter = sFRand() * m_TexModificator.m_VOscAmplitude;
            m_TexModificator.m_LastVTime = m_TexModificator.m_VOscPhase + cry_floorf(Sv);
          }
          m_TexModificator.m_TexMatrix(2,1) = m_TexModificator.m_CurrentVJitter;
        }
    	  break;
      case ETMM_Stretch:
		    {
			    float dv = m_TexModificator.m_VOscAmplitude * cry_sinf(2.0f * PI * (Sv - cry_floorf(Sv)) + 2.0f * PI * m_TexModificator.m_VOscPhase);
			    m_TexModificator.m_TexMatrix(1,1) = 1.0f+dv;
		    }
        break;
      case ETMM_StretchRepeat:
        {
          float dv = m_TexModificator.m_VOscAmplitude * cry_sinf(0.5f * PI * (Sv - cry_floorf(Sv)) + 2.0f * PI * m_TexModificator.m_VOscPhase);
          m_TexModificator.m_TexMatrix(1,1) = 1.0f+dv;
        }
        break;
    }

    if (m_TexModificator.m_Offs[0]!=0.0f || m_TexModificator.m_Offs[1]!=0.0f || m_TexModificator.m_Tiling[0]!=1.0f || m_TexModificator.m_Tiling[1]!=1.0f || m_TexModificator.m_Rot[0] || m_TexModificator.m_Rot[1] || m_TexModificator.m_Rot[2])
    {
      float du = m_TexModificator.m_Offs[0];
      float dv = m_TexModificator.m_Offs[1];
      float su = m_TexModificator.m_Tiling[0];
      float sv = m_TexModificator.m_Tiling[1];
      m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
        Matrix44(su,0,0,0,
                 0,sv,0,0,
                 du,dv,1,0,
                 0,0,0,1);
      if (m_TexModificator.m_Rot[0])
        m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationX(Word2Degr(m_TexModificator.m_Rot[0])*PI/180.0f);
      if (m_TexModificator.m_Rot[1])
        m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationY(Word2Degr(m_TexModificator.m_Rot[1])*PI/180.0f);
      if (m_TexModificator.m_Rot[2])
        m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix * Matrix33::CreateRotationZ(Word2Degr(m_TexModificator.m_Rot[2])*PI/180.0f);
      if (m_TexModificator.m_Rot[0] || m_TexModificator.m_Rot[1] || m_TexModificator.m_Rot[2])
        m_TexModificator.m_TexMatrix = m_TexModificator.m_TexMatrix *
        Matrix44(su,0,0,0,
        0,sv,0,0,
        -du,-dv,1,0,
        0,0,0,1);
    }
  }

  if (m_TexModificator.m_eTGType != ETG_Stream)
  {
    switch (m_TexModificator.m_eTGType)
    {
      case ETG_World:
        {
          m_TexModificator.m_UpdateFlags |= RBMF_TCGOL0;
          for (i=0; i<4; i++)
          {
            memset(&Pl, 0, sizeof(Pl));
            float *fPl = (float *)&Pl;
            fPl[i] = 1.0f;
            PlTr = TransformPlane2_NoTrans(gRenDev->m_RP.m_pCurObject->m_Matrix, Pl);
            m_TexModificator.m_TexGenMatrix(i,0) = PlTr.n.x;
            m_TexModificator.m_TexGenMatrix(i,1) = PlTr.n.y;
            m_TexModificator.m_TexGenMatrix(i,2) = PlTr.n.z;
            m_TexModificator.m_TexGenMatrix(i,3) = PlTr.d;
          }
        }
        break;
      case ETG_Camera:
        {
          m_TexModificator.m_UpdateFlags |= RBMF_TCGOL0;
          for (i=0; i<4; i++)
          {
            memset(&Pl, 0, sizeof(Pl));
            float *fPl = (float *)&Pl;
            fPl[i] = 1.0f;
            PlTr = TransformPlane2_NoTrans(gRenDev->m_ViewMatrix, Pl);
            m_TexModificator.m_TexGenMatrix(i,0) = PlTr.n.x;
            m_TexModificator.m_TexGenMatrix(i,1) = PlTr.n.y;
            m_TexModificator.m_TexGenMatrix(i,2) = PlTr.n.z;
            m_TexModificator.m_TexGenMatrix(i,3) = PlTr.d;
          }
        }
        break;
      case ETG_WorldEnvMap:
        {
          m_TexModificator.m_UpdateFlags |= RBMF_TCGRM0;
          if (bTr)
            m_TexModificator.m_TexMatrix = gRenDev->m_CameraMatrix * m_TexModificator.m_TexMatrix;
          else
            m_TexModificator.m_TexMatrix = gRenDev->m_CameraMatrix;
          bTr = true;
          if (m_TU.m_eTexType == eTT_Cubemap)
            bTranspose = true;
        }
        break;
      case ETG_CameraEnvMap:
        {
          m_TexModificator.m_UpdateFlags |= RBMF_TCGRM0;
        }
        break;
      case ETG_SphereMap:
        {
          m_TexModificator.m_UpdateFlags |= RBMF_TCGSM0;
          if (bTr)
            m_TexModificator.m_TexMatrix = gRenDev->m_RP.m_pCurObject->m_Matrix * m_TexModificator.m_TexMatrix;
          else
            m_TexModificator.m_TexMatrix = gRenDev->m_RP.m_pCurObject->m_Matrix;
          //bTr = true;
          //bTranspose = true;
        }
        break;
      case ETG_NormalMap:
        {
          m_TexModificator.m_UpdateFlags |= RBMF_TCGNM0;
        }
        break;
    }
  }

  if (bTr)
  {
    m_TexModificator.m_UpdateFlags |= RBMF_TCM0;
    if (m_TexModificator.m_bTexGenProjected)
    {
      m_TexModificator.m_TexMatrix(0,3) = m_TexModificator.m_TexMatrix(0,2);
      m_TexModificator.m_TexMatrix(1,3) = m_TexModificator.m_TexMatrix(1,2);
      m_TexModificator.m_TexMatrix(2,3) = m_TexModificator.m_TexMatrix(2,2);
      m_TexModificator.m_TexMatrix(3,3) = m_TexModificator.m_TexMatrix(3,2);
    }
    else
    {
      m_TexModificator.m_TexMatrix(3,0) = m_TexModificator.m_TexMatrix(2,0);
      m_TexModificator.m_TexMatrix(3,1) = m_TexModificator.m_TexMatrix(2,1);
    }
    if (bTranspose)
      m_TexModificator.m_TexMatrix.Transpose();
  }
  if (!(m_TexModificator.m_UpdateFlags & (RBMF_TCG | RBMF_TCM)))
    m_TexModificator.m_UpdateFlags |= RBMF_NOUPDATE;
  gRenDev->m_RP.m_FlagsModificators |= m_TexModificator.m_UpdateFlags<<nTU;
}


void CShader::mfBeginFrame()
{
  memset(&gRenDev->m_RP.m_PS, 0, sizeof(SPipeStat));
  m_Frame++;
  gRenDev->m_RP.m_Profile.Free();
  gRenDev->m_RP.m_bStartPipeline = false;

  gRenDev->m_RP.m_WasPortals = 0;
  gRenDev->m_RP.m_CurPortal = 0;
  gRenDev->m_RP.m_CurWarp = NULL;
  gRenDev->m_RP.m_fMinDepthRange = 0;
  gRenDev->m_RP.m_fMaxDepthRange = 1.0f;

  SRendItem::m_RecurseLevel = 0;
}

// Shaders caching
//=================================================================

SShaderCache::~SShaderCache()
{
  SAFE_DELETE(m_pRes);
}

bool CShader::CloseCacheFile(SShaderCache *pC)
{
  SAFE_DELETE (pC);

  return true;
}

SShaderCache *CShader::OpenCacheFile(const char *szName, float fVersion)
{
  SShaderCache *pCache = new SShaderCache;
  SShaderCacheHeader hd;
  bool bValid = true;

  CResFile *rf = new CResFile(szName, eFSD_id);
  if (!rf->mfOpen(RA_READ))
  {
    rf->mfClose();
    bValid = false;
  }
  else
  {
    rf->mfFileRead(0xffff, &hd);
    if (hd.m_SizeOf != sizeof(SShaderCacheHeader))
      bValid = false;
    else
    if (hd.m_MajorVer != (int)fVersion || hd.m_MinorVer != (int)(((float)fVersion - (float)(int)fVersion)*10.1f))
      bValid = false;
    else
    if (rf->mfGetHolesSize()/rf->mfGetResourceSize()*100 > 20)
      bValid = false;
    rf->mfClose();
    if (bValid)
    {
      if (!rf->mfOpen(RA_READ|RA_WRITE))
      {
        rf->mfClose();
        bValid = false;
      }
    }
  }
  if (!bValid)
  {
    rf->mfOpen(RA_CREATE);

    SDirEntry de;
    de.ID = 0xffff;
    de.earc = eARC_NONE;
    de.size = sizeof(SShaderCacheHeader);
    de.eid = eRI_BIN;
    de.user.data = &hd;
    hd.m_SizeOf = sizeof(SShaderCacheHeader);
    hd.m_MinorVer = (int)(((float)fVersion - (float)(int)fVersion)*10.1f);
    hd.m_MajorVer = (int)fVersion;
    rf->mfFileAdd(&de);
    rf->mfFlush();
  }
  pCache->m_pRes = rf;
  pCache->m_Name = szName;
  pCache->m_Header = hd;

  return pCache;
}

bool CShader::FlushCacheFile(SShaderCache *pCache)
{
  if (!pCache || !pCache->m_pRes)
    return false;

  pCache->m_pRes->mfFlush();

  return true;
}

bool CShader::AddCacheItem(SShaderCache *pCache, SShaderCacheHeaderItem *pItem, byte *pData, int nLen, bool bFlush)
{
  assert (pCache && pCache->m_pRes);
  if (!pCache || !pCache->m_pRes)
    return false;

  assert(pItem->m_nVariables < 32);

  byte *pNew = new byte[sizeof(SShaderCacheHeaderItem)+nLen];
  memcpy(pNew, pItem, sizeof(SShaderCacheHeaderItem));
  memcpy(&pNew[sizeof(SShaderCacheHeaderItem)], pData, nLen);
  SDirEntry de;
  de.ID = pItem->m_nMask;
  de.earc = eARC_NONE;
  de.size = nLen+sizeof(SShaderCacheHeaderItem);
  de.eid = eRI_BIN;
  de.user.data = pNew;
  de.flags = RF_TEMPDATA;
  pCache->m_pRes->mfFileAdd(&de);
  if (bFlush)
    pCache->m_pRes->mfFlush();

  return true;
}

bool CShader::FreeCacheItem(SShaderCache *pCache, int nMask)
{
  assert (pCache && pCache->m_pRes);
  if (!pCache || !pCache->m_pRes)
    return false;
  CResFile *rf = pCache->m_pRes;
  SDirEntry *de = rf->mfGetEntry(nMask);
  if (!de)
    return false;
  SAFE_DELETE_ARRAY(de->user.data);
  return true;
}

SShaderCacheHeaderItem *CShader::GetCacheItem(SShaderCache *pCache, int nMask)
{
  assert (pCache && pCache->m_pRes);
  if (!pCache || !pCache->m_pRes)
    return NULL;
  CResFile *rf = pCache->m_pRes;
  SDirEntry *de = rf->mfGetEntry(nMask);
  if (!de)
    return NULL;
  rf->mfFileRead(de);
  void *pData = rf->mfFileGetBuf(de);
  if (!pData)
    return NULL;
  return (SShaderCacheHeaderItem *) pData;
}

static void sConvert(const char *Path)
{
  struct _finddata_t fileinfo;
  intptr_t handle;
  char nmf[256];
  char nmfDst[256];
  char dirn[256];

  strcpy(dirn, Path);
  strcat(dirn, "*.cgbin");
  ConvertUnixToDosName(dirn, dirn);

  handle = iSystem->GetIPak()->FindFirst (dirn, &fileinfo);
  if (handle == -1)
    return;
  do
  {
    if (fileinfo.name[0] == '.')
      continue;
    if (fileinfo.attrib & _A_SUBDIR)
      continue;
    strcpy(nmf, Path);
    strcat(nmf, fileinfo.name);

    SShaderCacheHeader hd;

    CResFile *rfSrc = new CResFile(nmf, eFSD_id);
    if (!rfSrc->mfOpen(RA_READ))
    {
      rfSrc->mfClose();
      continue;
    }
    int i;
    StripExtension(nmf, nmfDst);
    AddExtension(nmfDst, ".cgb");

    CResFile *rfDst = new CResFile(nmfDst, eFSD_id);
    rfDst->mfOpen(RA_CREATE);

    TArray<SDirEntry *> Dir;
    int nNum = rfSrc->mfGetNumFiles();
    rfSrc->mfGetDir(Dir);

    for (i=0; i<Dir.Num(); i++)
    {
      SDirEntry *d = Dir[i];
      int size = rfSrc->mfFileRead(d);
      void *pData = rfSrc->mfFileGetBuf(d);
      if (pData)
      {
        d->size = size;
        d->earc = eARC_NONE;
        rfDst->mfFileAdd(d);
      }
    }
    rfDst->mfFlush();

    rfDst->mfClose();
    rfSrc->mfClose();
  } while (iSystem->GetIPak()->FindNext( handle, &fileinfo ) != -1);

  iSystem->GetIPak()->FindClose (handle);
}

bool CShader::PackCache()
{
  sConvert("Shaders\\Cache\\CGPShaders\\");
  sConvert("Shaders\\Cache\\CGVShaders\\");

  return true;
}