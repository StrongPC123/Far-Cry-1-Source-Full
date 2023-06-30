/*=============================================================================
  ShaderTemplate.cpp : implementation of the Shaders templates support.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "I3DEngine.h"
#include "CryHeaders.h"

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#include <io.h>
#elif defined(LINUX)

#endif

//===============================================================================

SRenderShaderResources::~SRenderShaderResources()
{
  int i;

  for (i=0; i<EFTT_MAX; i++)
  {
    if (m_Textures[i])
    {
      delete m_Textures[i];
      m_Textures[i] = NULL;
    }
  }
  SAFE_RELEASE(m_LMaterial);
  SShader::m_ShaderResources_known[m_Id] = NULL;
}

SRenderShaderResources::SRenderShaderResources(SInputShaderResources *pSrc)
{
  Reset();
  if (pSrc)
  {
    m_TexturePath = pSrc->m_TexturePath;
    m_ResFlags = pSrc->m_ResFlags;
    m_Opacity = pSrc->m_Opacity;
    m_AlphaRef = pSrc->m_AlphaRef;
    m_LMaterial = pSrc->m_LMaterial;
    m_ShaderParams.Copy(pSrc->m_ShaderParams);
  }
  for (int i=0; i<EFTT_MAX; i++)
  {
    if (pSrc && (!pSrc->m_Textures[i].m_Name.empty() || pSrc->m_Textures[i].m_TU.m_TexPic))
    {
      if (!m_Textures[i])
        AddTextureMap(i);
      *m_Textures[i] = pSrc->m_Textures[i];
    }
    else
    {
      if (m_Textures[i])
        m_Textures[i]->Reset();
      m_Textures[i] = NULL;
    }
  }

}

SRenderShaderResources *CShader::mfCreateShaderResources(const SInputShaderResources *Res, bool bShare)
{
  int i, j;

  SInputShaderResources RS;
  RS = *Res;

  for (i=0; i<EFTT_MAX; i++)
  {
    RS.m_Textures[i].m_TU.m_TexPic = NULL;
    if (RS.m_Textures[i].m_TexFlags & TEXMAP_NOMIPMAP)
      RS.m_Textures[i].m_TU.m_nFlags |= FTU_NOMIPS;

    if (i == EFTT_DETAIL_OVERLAY && !RS.m_Textures[i].m_Name.empty())
    {
      RS.m_Textures[i].m_TU.m_TexPic = mfLoadResourceTexture(RS.m_Textures[i].m_Name.c_str(), RS.m_TexturePath.c_str(), RS.m_Textures[i].m_TU.GetTexFlags(), RS.m_Textures[i].m_TU.GetTexFlags2(), eTT_Base, NULL, &RS.m_Textures[i], RS.m_Textures[i].m_Amount);
      if (!RS.m_Textures[i].m_TU.m_TexPic->IsTextureLoaded())
      {
        Warning( VALIDATOR_FLAG_TEXTURE,RS.m_Textures[i].m_Name.c_str(),"Error: CShader::mfCreateShaderResources: Couldn't find detail texture\n'%s' in path '%s'\n", RS.m_Textures[i].m_Name.c_str(), RS.m_TexturePath.c_str());
        RS.m_Textures[i].m_TU.m_TexPic = mfLoadResourceTexture(CRenderer::CV_r_detaildefault->GetString(), RS.m_TexturePath.c_str(), RS.m_Textures[i].m_TU.GetTexFlags(), RS.m_Textures[i].m_TU.GetTexFlags2(), eTT_Base, NULL, &RS.m_Textures[i], RS.m_Textures[i].m_Amount);
        if (!RS.m_Textures[i].m_TU.m_TexPic->IsTextureLoaded())
          RS.m_Textures[i].m_TU.m_TexPic = mfLoadResourceTexture("Textures/detail/rock", RS.m_TexturePath.c_str(), RS.m_Textures[i].m_TU.GetTexFlags(), RS.m_Textures[i].m_TU.GetTexFlags2(), eTT_Base, NULL, &RS.m_Textures[i], RS.m_Textures[i].m_Amount);
      }
    }

    if (i == EFTT_DECAL_OVERLAY && !RS.m_Textures[i].m_Name.empty())
    {
      RS.m_Textures[i].m_TU.m_TexPic = mfLoadResourceTexture(RS.m_Textures[i].m_Name.c_str(), RS.m_TexturePath.c_str(), RS.m_Textures[i].m_TU.GetTexFlags(), RS.m_Textures[i].m_TU.GetTexFlags2(), eTT_Base, NULL, &RS.m_Textures[i], RS.m_Textures[i].m_Amount);
      if (!RS.m_Textures[i].m_TU.m_TexPic->IsTextureLoaded())
        Warning( VALIDATOR_FLAG_TEXTURE,RS.m_Textures[i].m_Name.c_str(),"Error: CShader::mfCreateShaderResources: Couldn't find decal texture\n'%s' in path '%s'\n", RS.m_Textures[i].m_Name.c_str(), RS.m_TexturePath.c_str());
    }

    if (i == EFTT_SUBSURFACE && !RS.m_Textures[i].m_Name.empty())
    {
      RS.m_Textures[i].m_TU.m_TexPic = mfLoadResourceTexture(RS.m_Textures[i].m_Name.c_str(), RS.m_TexturePath.c_str(), RS.m_Textures[i].m_TU.GetTexFlags(), RS.m_Textures[i].m_TU.GetTexFlags2(), eTT_Base, NULL, &RS.m_Textures[i], RS.m_Textures[i].m_Amount);
      if (!RS.m_Textures[i].m_TU.m_TexPic->IsTextureLoaded())
        Warning( VALIDATOR_FLAG_TEXTURE,RS.m_Textures[i].m_Name.c_str(),"Error: CShader::mfCreateShaderResources: Couldn't find subsurface texture\n'%s' in path '%s'\n", RS.m_Textures[i].m_Name.c_str(), RS.m_TexturePath.c_str());
    }

    if (i == EFTT_ATTENUATION2D)
    {
      SEfResTexture *Tex = &RS.m_Textures[EFTT_ATTENUATION2D];
      if (!Tex->m_TU.m_TexPic)
      {
        if (!Tex->m_Name.empty())
          Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), RS.m_TexturePath.c_str(), Tex->m_TU.GetTexFlags(), Tex->m_TU.GetTexFlags2(), eTT_Base, NULL, Tex);
        if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/PointLight2D", RS.m_TexturePath.c_str(), Tex->m_TU.GetTexFlags(), Tex->m_TU.GetTexFlags2(), eTT_Base, NULL, Tex);
      }
    }
    if (i == EFTT_ATTENUATION1D)
    {
      SEfResTexture *Tex = &RS.m_Textures[EFTT_ATTENUATION1D];
      if (!Tex->m_TU.m_TexPic)
      {
        if (!Tex->m_Name.empty())
          Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), RS.m_TexturePath.c_str(), Tex->m_TU.GetTexFlags(), Tex->m_TU.GetTexFlags2(), eTT_Base, NULL, Tex);
        if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
        {
          if (gRenDev->m_TexMan->m_Text_Atten1D)
            Tex->m_TU.m_TexPic = gRenDev->m_TexMan->m_Text_Atten1D;
          else
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/PointLight1D", RS.m_TexturePath.c_str(), Tex->m_TU.GetTexFlags(), Tex->m_TU.GetTexFlags2(), eTT_Base, NULL, Tex);
        }
      }
    }
  }

  if (RS.m_LMaterial)
    RS.m_LMaterial = SLightMaterial::mfAdd(NULL, RS.m_LMaterial);

  int nFree = -1;
  for (i=1; i<SShader::m_ShaderResources_known.Num(); i++)
  {
    SRenderShaderResources *pSR = SShader::m_ShaderResources_known[i];
    if (!pSR)
    {
      nFree = i;
      if (!bShare || Res->m_ShaderParams.Num())
        break;
      continue;
    }
    if (!bShare || Res->m_ShaderParams.Num())
      continue;
    if (RS.m_ResFlags == pSR->m_ResFlags && RS.m_LMaterial == pSR->m_LMaterial && RS.m_Opacity == pSR->m_Opacity && RS.m_AlphaRef == pSR->m_AlphaRef && !stricmp(RS.m_TexturePath.c_str(), pSR->m_TexturePath.c_str()))
    {
      for (j=0; j<EFTT_MAX; j++)
      {
        if (!pSR->m_Textures[j] || pSR->m_Textures[j]->m_Name.empty())
        {
          if (RS.m_Textures[j].m_Name.empty())
            continue;
          break;
        }
        else
        if (RS.m_Textures[j].m_Name.empty())
          break;
        if (RS.m_Textures[j] != *pSR->m_Textures[j])
          break;
      }
      if (j == EFTT_MAX)
      {
        pSR->m_nRefCounter++;
        SAFE_RELEASE(RS.m_LMaterial);
        return pSR;
      }
    }
  }

  SRenderShaderResources *pSR = new SRenderShaderResources(&RS);
  pSR->m_nRefCounter = 1;
  pSR->m_nCheckedTemplates = 0;
  if (!SShader::m_ShaderResources_known.Num())
  {
    SShader::m_ShaderResources_known.AddIndex(1);
    SRenderShaderResources *pSRNULL = new SRenderShaderResources;
    pSRNULL->m_nRefCounter = 1;
    SShader::m_ShaderResources_known[0] = pSRNULL;
  }
  else
  if (SShader::m_ShaderResources_known.Num() >= MAX_SHADER_RES)
  {
    Warning( 0,0,"ERROR: CShader::mfCreateShaderResources: MAX_SHADER_RESOURCES hit");
    return SShader::m_ShaderResources_known[1];
  }
  if (nFree > 0)
  {
    pSR->m_Id = nFree;
    SShader::m_ShaderResources_known[nFree] = pSR;
  }
  else
  {
    pSR->m_Id = SShader::m_ShaderResources_known.Num();
    SShader::m_ShaderResources_known.AddElem(pSR);
  }
  pSR->PostLoad();

  return pSR;
}

void CShader::mfShaderNameForAlias(const char *nameAlias, char *nameEf, int nSize)
{
  CName cn = CName(nameAlias, eFN_Find);
  SNameAlias *na;
  int i;
  strncpy(nameEf, nameAlias, nSize);
  if (cn.GetIndex())
  {
    for (i=0; i<m_AliasNames.Num(); i++)
    {
      na = &m_AliasNames[i];
      if (na->m_Alias == cn)
        break;
    }
    if (i<m_AliasNames.Num())
      strncpy(nameEf, na->m_Name.c_str(), nSize);

    cn = CName(nameEf, eFN_Find);
    for (i=0; i<m_CustomAliasNames.Num(); i++)
    {
      na = &m_CustomAliasNames[i];
      if (na->m_Alias == cn)
        break;
    }
    if (i<m_CustomAliasNames.Num())
      strncpy(nameEf, na->m_Name.c_str(), nSize);
  }
}

SShaderItem CShader::mfShaderItemForName (const char *nameEf, EShClass Class, bool bShare, const char *tmplName, int flags, const SInputShaderResources *Res, uint64 nMaskGen)
{
  SShaderItem SI;

  SLightMaterial mtl;

  if (Res && !Res->m_LMaterial)
  {
    SInputShaderResources *pRes = (SInputShaderResources *)Res;
    int nQBM = CRenderer::CV_r_Quality_BumpMapping;
    if (((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GF2) || nQBM == 0)
      pRes->m_LMaterial = &mtl;
  }

  SI.m_pShaderResources = mfCreateShaderResources(Res, bShare);
  m_pCurResources = SI.m_pShaderResources;

  char sNewShaderName[256]="";
  char templName[256];
  if (tmplName && tmplName[0])
    mfShaderNameForAlias(tmplName, templName, 256);
  else
    templName[0] = 0;

  SI.m_pShader = mfForName(nameEf, Class, flags, Res, 0);
  int n = 0;

  while (true)
  {
    bool bTryNew = false;
    SEfTemplates *eft = SI.m_pShader->GetTemplates();
    if (eft && eft->m_TemplShaders.Num() && eft->m_TemplShaders[EFT_DEFAULT])
    {
      if (!templName)
        bTryNew = true;
      else
      {
        SShader *pSH = eft->m_TemplShaders[EFT_DEFAULT];
        if (stricmp(pSH->GetName(), templName) != 0)
          bTryNew = true;
      }
    }
    else
    if ((SI.m_pShader->GetFlags3() & EF3_NOTEMPLATE) && templName)
      bTryNew = true;
    if (!bTryNew)
      break;
    n++;
    sprintf(sNewShaderName, "%s_%d", nameEf, n);
    int Flags3 = SI.m_pShader->GetFlags3();
    SI.m_pShader->Release();
    SI.m_pShader = mfForName(sNewShaderName, Class, flags, Res, 0);
    SI.m_pShader->SetFlags3(SI.m_pShader->GetFlags3() | (Flags3 & EF3_NODRAW));
  }

  if (templName && templName[0])
  {
    int nTemp = EFT_DEFAULT;
    if(!SI.m_pShader->AddTemplate(SI.m_pShaderResources, nTemp, templName, true, nMaskGen))
    {
      iLog->Log("Error: Specified shader template not found: %s\nFull material name: %s", templName, nameEf);
      SI.m_pShader->SetFlags3(EF3_NOTEMPLATE);
    }
  }
  else
    SI.m_pShader->SetFlags3(EF3_NOTEMPLATE);

  SI.m_pShaderResources = m_pCurResources;
  SI.m_pShaderResources->PostLoad();
  m_pCurResources = NULL;
  return SI;
}


//=================================================================================================

STexPic *CShader::mfCheckTemplateTexName(char *mapname, ETexType eTT, short &nFlags)
{
  STexPic *TexPic = NULL;
  if (mapname[0] != '$')
    return NULL;
  
  if (!stricmp(mapname, "$NormalizeCubemap") || !stricmp(mapname, "$NormalizationCubemap"))
    TexPic = gRenDev->m_TexMan->m_Text_NormalizeCMap;
  else
  if (!stricmp(mapname, "$Lightmap"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_LIGHTMAP];
  else
  if (!stricmp(mapname, "$LightmapDirection"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_LIGHTMAP_DIR];
  else
  if (!strnicmp(mapname, "$VFog", 4))
    TexPic = gRenDev->m_TexMan->m_Text_VFog;
  else
  if (!strnicmp(mapname, "$FogEnter", 8))
    TexPic = gRenDev->m_TexMan->m_Text_Fog_Enter;
  else
  if (!strnicmp(mapname, "$Fog", 4))
    TexPic = gRenDev->m_TexMan->m_Text_Fog;
  else
  if (!strnicmp(mapname, "$Flare", 6))
    TexPic = gRenDev->m_TexMan->m_Text_Flare;
  else
  if (!stricmp(mapname, "$LightCubemap"))
    TexPic = gRenDev->m_TexMan->m_Text_LightCMap;
  else
  if (!stricmp(mapname, "$FromRE") || !stricmp(mapname, "$FromRE0"))
    TexPic = gRenDev->m_TexMan->m_Text_FromRE[0];
  else
  if (!stricmp(mapname, "$FromRE1"))
    TexPic = gRenDev->m_TexMan->m_Text_FromRE[1];
  else
  if (!stricmp(mapname, "$FromRE2"))
    TexPic = gRenDev->m_TexMan->m_Text_FromRE[2];
  else
  if (!stricmp(mapname, "$FromRE3"))
    TexPic = gRenDev->m_TexMan->m_Text_FromRE[3];
	else
  if (!stricmp(mapname, "$FromRE4"))
		TexPic = gRenDev->m_TexMan->m_Text_FromRE[4];
  else
	if (!stricmp(mapname, "$FromRE5"))
		TexPic = gRenDev->m_TexMan->m_Text_FromRE[5];
	else
	if (!stricmp(mapname, "$FromRE6"))
		TexPic = gRenDev->m_TexMan->m_Text_FromRE[6];
	else
	if (!stricmp(mapname, "$FromRE7"))
		TexPic = gRenDev->m_TexMan->m_Text_FromRE[7];
	else
  if (!stricmp(mapname, "$FromObj") || !stricmp(mapname, "$FromObj0"))
    TexPic = gRenDev->m_TexMan->m_Text_FromObj;
  else
  if (!stricmp(mapname, "$FromLight"))
    TexPic = gRenDev->m_TexMan->m_Text_FromLight;
  else
  if (!strnicmp(mapname, "$Phong_", 7))
  {
    int n = atoi(&mapname[7]);
    TexPic = gRenDev->EF_MakePhongTexture(n);
  }
  else
  if (!strnicmp(mapname, "$Phong", 6))
  {
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_PHONG];
  }
  else
  if (!stricmp(mapname, "$WhiteShadow"))
    TexPic = gRenDev->m_TexMan->LoadTexture(*gRenDev->m_TexMan->m_Text_WhiteShadow->m_SearchName, 0, FT2_NOANISO, eTT);
  else
  if (!strnicmp(mapname, "$White", 6))
    TexPic = gRenDev->m_TexMan->LoadTexture(*gRenDev->m_TexMan->m_Text_White->m_SearchName, 0, FT2_NOANISO, eTT);
  else
  if (!strnicmp(mapname, "$EnvCMap", 8) || !stricmp(mapname, "$EnvironmentCubeMap"))
    TexPic = gRenDev->m_TexMan->m_Text_EnvCMap;
  else
  if (!strnicmp(mapname, "$EnvLightCMap", 13) || !stricmp(mapname, "$EnvironmentLightCubeMap"))
    TexPic = gRenDev->m_TexMan->m_Text_EnvLCMap;
  else
  if (!stricmp(mapname, "$Ghost"))
    TexPic = gRenDev->m_TexMan->m_Text_Ghost;
  else
  if (!stricmp(mapname, "$RainMap"))
    TexPic = gRenDev->m_TexMan->m_Text_RainMap;
  else
  if (!strnicmp(mapname, "$EnvTex", 7) || !stricmp(mapname, "$EnvironmentTexture"))
    TexPic = gRenDev->m_TexMan->m_Text_EnvTex;
  else
  if (!strnicmp(mapname, "$EnvScr", 7) || !stricmp(mapname, "$EnvironmentScreen"))
    TexPic = gRenDev->m_TexMan->m_Text_EnvScr;
  else
  if (!strnicmp(mapname, "$WaterMap", 9))
    TexPic = gRenDev->m_TexMan->m_Text_WaterMap;
  else
  if (!strnicmp(mapname, "$CustomCMap", 11))
  {
    int n;
    if (mapname[11] == '_')
      n = atoi(&mapname[12]);
    else
      n = atoi(&mapname[11]);
    TexPic = gRenDev->m_TexMan->m_CustomCMaps[n].m_Tex;
  }
  else
  if (!strnicmp(mapname, "$CustomTexture", 14))
  {
    int n;
    if (mapname[11] == '_')
      n = atoi(&mapname[12]);
    else
      n = atoi(&mapname[11]);
    TexPic = gRenDev->m_TexMan->m_CustomTextures[n].m_Tex;
  }
  else
  if (!stricmp(mapname, "$Diffuse"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_DIFFUSE];
  else
  if (!stricmp(mapname, "$DecalOverlay"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_DECAL_OVERLAY];
  else
  if (!stricmp(mapname, "$Detail"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_DETAIL_OVERLAY];
  else
  if (!stricmp(mapname, "$Opacity"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_OPACITY];
  else
  if (!strnicmp(mapname, "$Specular", 9))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_SPECULAR];
  else
  if (!stricmp(mapname, "$Attenuation2D"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_ATTENUATION2D];
  else
  if (!stricmp(mapname, "$Attenuation1D"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_ATTENUATION1D];
  else
  if (!strnicmp(mapname, "$BumpPlants", 11))
  {
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_BUMP];
    nFlags |= FTU_BUMPPLANTS;
  }
  else
  if (!strnicmp(mapname, "$BumpDiffuse", 12))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_BUMP_DIFFUSE];
  else
  if (!strnicmp(mapname, "$BumpHeight", 10))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_BUMP_HEIGHT];
  else
  if (!strnicmp(mapname, "$Bump", 5))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_BUMP];
  else
  if (!strnicmp(mapname, "$Subsurface", 11))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_SUBSURFACE];
  else
  if (!stricmp(mapname, "$Cubemap"))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_CUBEMAP];
  else
  if (!strnicmp(mapname, "$Occlusion", 10))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_OCCLUSION];
  else
  if (!strnicmp(mapname, "$Gloss", 6))
    TexPic = &gRenDev->m_TexMan->m_Templates[EFTT_GLOSS];
  else // tiago: added
  if (!stricmp(mapname, "$FlashBangMap"))
    TexPic = gRenDev->m_TexMan->m_Text_FlashBangMap;
  else 
  if (!stricmp(mapname, "$ScreenTexMap"))
      TexPic = gRenDev->m_TexMan->m_Text_ScreenMap;
  else
  if (!stricmp(mapname, "$PrevScreenTexMap"))
    TexPic = gRenDev->m_TexMan->m_Text_PrevScreenMap;
  else 
  if (!stricmp(mapname, "$ScreenLuminosityMap"))
    TexPic = gRenDev->m_TexMan->m_Text_ScreenLuminosityMap;
  else 
  if (!stricmp(mapname, "$ScreenCurrLuminosityMap"))
    TexPic = gRenDev->m_TexMan->m_Text_ScreenCurrLuminosityMap;
  else 
  if (!stricmp(mapname, "$ScreenLowMap"))
    TexPic = gRenDev->m_TexMan->m_Text_ScreenLowMap;
  else
  if (!stricmp(mapname, "$ScreenAvg1x1"))
    TexPic = gRenDev->m_TexMan->m_Text_ScreenAvg1x1;
  else
  if (!stricmp(mapname, "$DofTexMap"))
    TexPic = gRenDev->m_TexMan->m_Text_DofMap;

  return TexPic;
}


bool CShader::mfCheckAnimatedSequence(SShaderTexUnit *tl, STexPic *tx)
{
  if (tl->m_AnimInfo)
    return false;
  if (!tx)
    tx = tl->m_TexPic;
  if (!tx || (tx->m_Flags & FT_NOTFOUND))
    return true;
  if (!tx->m_NextTxt)
    return true;

  STexAnim *ta = new STexAnim; 
  ta->m_bLoop = true;
  STexPic *tp = tx;
  while (tp)
  {
    ta->m_Time = tp->m_fAnimSpeed;
    ta->m_TexPics.AddElem(tp);
    tp = tp->m_NextTxt;
  }
  ta->m_NumAnimTexs = ta->m_TexPics.Num();
  tl->m_AnimInfo = ta;
  tl->m_TexPic = tx;

  return false;
}

STexPic *CShader::mfTryToLoadTexture(const char *nameTex, int Flags, int Flags2, byte eTT, SShader *sh, float fAmount1, float fAmount2)
{
  STexPic *tx = NULL;                    
  if (nameTex && (strchr(nameTex, '#') && !strstr(nameTex, " #")) || (strchr(nameTex, '$') && !strstr(nameTex, " $"))) // test for " #" to skip max material names
  {
    TArray<STexPic *> Texs;
    int n = mfReadTexSequence(sh, Texs, nameTex, eTT, Flags, Flags2, fAmount1, fAmount2);
    if (n > 1)
    {
      STexPic *tp = NULL;
      for (int j=0; j<Texs.Num(); j++)
      {
        STexPic *t = Texs[j];
        if (!j)
        {
          tx = t;
          t->m_NextTxt = NULL;
        }
        else
          tp->m_NextTxt = t;
        tp = t;
      }
    }
  }
  if (!tx)
  {
    tx = (STexPic *)gRenDev->EF_LoadTexture(nameTex, Flags, Flags2, eTT, fAmount1, fAmount2);
    tx->m_NextTxt = NULL;
  }

  return tx;
}

STexPic *CShader::mfLoadResourceTexture(const char *nameTex, const char *path, int Flags, int Flags2, byte eTT, SShader *sh, SEfResTexture *Tex, float fAmount1, float fAmount2)
{
  STexPic *tx = mfTryToLoadTexture(nameTex, Flags, Flags2, eTT, sh, fAmount1, fAmount2);

  if ((!tx || !tx->IsTextureLoaded()) && nameTex && path)
  {
    if (strnicmp(nameTex, path, strlen(path)))
    {
      if (tx)
        tx->Release(false);
      char name[256];
      char ext[32];
      char pname[256];
      char *pPath = (char *)path;
      size_t ln = strlen(nameTex);
      size_t lp = strlen(path);
      if (ln + lp >= 250)
      {
        Warning( VALIDATOR_FLAG_TEXTURE,nameTex,"Warning: Too long texture name (path: '%s', name: '%s')\n", path, nameTex);
        pPath = "Textures\\";
      }
      _splitpath(nameTex, NULL, NULL, name, ext);
      strcat(name, ext);
      UsePath((char *)name, (char *)pPath, pname);
      tx = mfTryToLoadTexture(pname, Flags, Flags2, eTT, sh, fAmount1, fAmount2);
      if ((!tx || !tx->IsTextureLoaded()) && nameTex && path)
      {
        if (tx)
          tx->Release(false);

        strcpy(name, pPath);
        strcat(name, nameTex);
        tx = mfTryToLoadTexture(name, Flags, Flags2, eTT, sh, fAmount1, fAmount2);
      }
    }
  }

  if (Tex && tx && tx->IsTextureLoaded())
  {
    mfCheckAnimatedSequence(&Tex->m_TU, tx);
    if (!strchr(tx->m_SourceName.c_str(), '+'))
      Tex->m_Name = tx->m_SourceName;
  }

  return tx;
}

void CShader::mfCheckShaderResTextures(TArray<SShaderPass> &Dst, SShader *ef, SRenderShaderResources *Res)
{
  const char *patch = Res->m_TexturePath.c_str();
  SEfResTexture *Tex;
  for (int i=0; i<Dst.Num(); i++)
  {
    for (int j=0; j<Dst[i].m_TUnits.Num(); j++)
    {
      int Flags = 0;
      if (Dst[i].m_TUnits[j].m_nFlags & FTU_CLAMP)
        Flags |= FT_CLAMP;
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_DIFFUSE])
      {
        if (!Res->m_Textures[EFTT_DIFFUSE])
        {
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_DIFFUSE), ef->m_Name.c_str());
          Res->AddTextureMap(EFTT_DIFFUSE);
        }
        Tex = Res->m_Textures[EFTT_DIFFUSE];
        if (!Tex->m_TU.m_TexPic)
        {
          Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          if (Tex->m_TU.m_TexPic && Tex->m_TU.m_TexPic->IsTextureLoaded())
            Tex->m_TU.m_TexPic->m_Flags2 |= FT2_DIFFUSETEXTURE;
        }
        else
          mfCheckAnimatedSequence(&Res->m_Textures[EFTT_DIFFUSE]->m_TU, Res->m_Textures[EFTT_DIFFUSE]->m_TU.m_TexPic);
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_PHONG])
      {
        if (!Res->m_Textures[EFTT_PHONG])
        {
          Res->AddTextureMap(EFTT_PHONG);
          Tex = Res->m_Textures[EFTT_PHONG];
          if (!Tex->m_TU.m_TexPic)
          {
            float n = CRenderer::CV_r_shininess;
            if (Res->m_LMaterial)
              n = Res->m_LMaterial->Front.m_SpecShininess;
            Tex->m_TU.m_TexPic = gRenDev->EF_MakePhongTexture((int)n);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_GLOSS])
      {
        if (!Res->m_Textures[EFTT_GLOSS])
        {
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_GLOSS), ef->m_Name.c_str());
          Res->AddTextureMap(EFTT_GLOSS);
        }
        Tex = Res->m_Textures[EFTT_GLOSS];
        if (!Tex->m_TU.m_TexPic)
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            if (!stricmp(CRenderer::CV_r_glossdefault->GetString(), "$Diffuse"))
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(Res->m_Textures[EFTT_DIFFUSE]->m_Name.c_str(), patch, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags2(), eTT_Base, ef, Res->m_Textures[EFTT_GLOSS]);
            else
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(CRenderer::CV_r_glossdefault->GetString(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_DETAIL_OVERLAY])
      {
        if (!Res->m_Textures[EFTT_DETAIL_OVERLAY])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_DETAIL_OVERLAY), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_DETAIL_OVERLAY];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(CRenderer::CV_r_detaildefault->GetString(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
              if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
                Tex->m_TU.m_TexPic = mfLoadResourceTexture("detail/dirty", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_OPACITY])
      {
        if (!Res->m_Textures[EFTT_OPACITY])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_OPACITY), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_OPACITY];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(CRenderer::CV_r_opacitydefault->GetString(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
              if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
                Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_SPECULAR])
      {
        if (!Res->m_Textures[EFTT_SPECULAR])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_SPECULAR), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_SPECULAR];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              if (Res->m_Textures[EFTT_DIFFUSE])
                Tex->m_TU.m_TexPic = mfLoadResourceTexture(Res->m_Textures[EFTT_DIFFUSE]->m_Name.c_str(), patch, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_BUMP])
      {
        if (Res->m_Textures[EFTT_BUMP] && Res->m_Textures[EFTT_NORMALMAP])
        {
          char fullname[256];
          sprintf(fullname, "%s+norm_%s", Res->m_Textures[EFTT_BUMP]->m_Name.c_str(), Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str());
          Res->m_Textures[EFTT_BUMP]->m_TU.m_TexPic = mfLoadResourceTexture(fullname, patch, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags2(), eTT_Bumpmap, ef, Res->m_Textures[EFTT_BUMP], (float)Res->m_Textures[EFTT_BUMP]->m_Amount, (float)Res->m_Textures[EFTT_NORMALMAP]->m_Amount);
        }
        if (!Res->m_Textures[EFTT_BUMP])
          Res->AddTextureMap(EFTT_BUMP);
        Tex = Res->m_Textures[EFTT_BUMP];
        if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
        {
          Tex = Res->m_Textures[EFTT_BUMP];
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Bumpmap, ef, Tex, (float)Tex->m_Amount);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            if (Res->m_Textures[EFTT_NORMALMAP])
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str(), patch, Res->m_Textures[EFTT_NORMALMAP]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_NORMALMAP]->m_TU.GetTexFlags2(), eTT_Bumpmap, ef, Res->m_Textures[EFTT_BUMP], (float)Res->m_Textures[EFTT_NORMALMAP]->m_Amount);
          }
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            Tex->m_TU.m_nFlags |= FTU_NOBUMP;
            if (!(Tex->m_TU.m_nFlags & FTU_BUMPPLANTS))
              Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white_ddn", patch, Flags, 0, eTT_Bumpmap, ef, Tex);
            else
              Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white_ddn", patch, Flags, 0, eTT_Bumpmap, ef, Tex);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_DECAL_OVERLAY])
      {
        if (!Res->m_Textures[EFTT_DECAL_OVERLAY])
          Res->AddTextureMap(EFTT_DECAL_OVERLAY);
        Tex = Res->m_Textures[EFTT_DECAL_OVERLAY];
        if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex, (float)Tex->m_Amount);
          else
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex, (float)Tex->m_Amount);
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_ATTENUATION2D])
      {
        if (!Res->m_Textures[EFTT_ATTENUATION2D])
          Res->AddTextureMap(EFTT_ATTENUATION2D);
        Tex = Res->m_Textures[EFTT_ATTENUATION2D];
        if (!Tex->m_TU.m_TexPic)
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/PointLight2D", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_SUBSURFACE])
      {
        if (!Res->m_Textures[EFTT_SUBSURFACE])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_SUBSURFACE), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_SUBSURFACE];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              Tex->m_TU.m_TexPic = NULL;
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_ATTENUATION1D])
      {
        if (!Res->m_Textures[EFTT_ATTENUATION1D])
          Res->AddTextureMap(EFTT_ATTENUATION1D);
        Tex = Res->m_Textures[EFTT_ATTENUATION1D];
        if (!Tex->m_TU.m_TexPic)
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            if (gRenDev->m_TexMan->m_Text_Atten1D)
              Tex->m_TU.m_TexPic = gRenDev->m_TexMan->m_Text_Atten1D;
            else
              Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/PointLight1D", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_CUBEMAP])
      {
        if (!Res->m_Textures[EFTT_CUBEMAP])
        {
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_CUBEMAP), ef->m_Name.c_str());
          Res->AddTextureMap(EFTT_CUBEMAP);
        }
        Tex = Res->m_Textures[EFTT_CUBEMAP];
        if (!Tex->m_TU.m_TexPic)
        {
          Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Cubemap, ef, Tex);
          if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/DefaultCM", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Cubemap, ef, Tex);
          }
        }
      }
    }
  }
}

void CShader::mfCheckShaderResTexturesHW(TArray<SShaderPassHW> &Dst, SShader *ef, SRenderShaderResources *Res)
{
  const char *patch = Res->m_TexturePath.c_str();
  SEfResTexture *Tex;
  for (int i=0; i<Dst.Num(); i++)
  {
    for (int j=0; j<Dst[i].m_TUnits.Num(); j++)
    {
      int Flags = 0;
      if (Dst[i].m_TUnits[j].m_nFlags & FTU_CLAMP)
        Flags |= FT_CLAMP;
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_DIFFUSE])
      {
        if (!Res->m_Textures[EFTT_DIFFUSE])
        {
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_DIFFUSE), ef->m_Name.c_str());
          Res->AddTextureMap(EFTT_DIFFUSE);
        }
        Tex = Res->m_Textures[EFTT_DIFFUSE];
        if (!Tex->m_TU.m_TexPic)
        {
          Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), Tex->m_TU.m_eTexType, ef, Tex);
          if (Tex->m_TU.m_TexPic && Tex->m_TU.m_TexPic->IsTextureLoaded())
            Tex->m_TU.m_TexPic->m_Flags2 |= FT2_DIFFUSETEXTURE;
        }
        else
          mfCheckAnimatedSequence(&Res->m_Textures[EFTT_DIFFUSE]->m_TU, Res->m_Textures[EFTT_DIFFUSE]->m_TU.m_TexPic);
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_PHONG])
      {
        if (!Res->m_Textures[EFTT_PHONG])
        {
          Res->AddTextureMap(EFTT_PHONG);
          Tex = Res->m_Textures[EFTT_PHONG];
          if (!Tex->m_TU.m_TexPic)
          {
            float n = CRenderer::CV_r_shininess;
            if (Res->m_LMaterial)
              n = Res->m_LMaterial->Front.m_SpecShininess;
            Tex->m_TU.m_TexPic = gRenDev->EF_MakePhongTexture((int)n);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_GLOSS])
      {
        if (!Res->m_Textures[EFTT_GLOSS])
        {
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_GLOSS), ef->m_Name.c_str());
          Res->AddTextureMap(EFTT_GLOSS);
        }
        Tex = Res->m_Textures[EFTT_GLOSS];
        if (!Tex->m_TU.m_TexPic)
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            if (!stricmp(CRenderer::CV_r_glossdefault->GetString(), "$Diffuse"))
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(Res->m_Textures[EFTT_DIFFUSE]->m_Name.c_str(), patch, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags2(), eTT_Base, ef, Res->m_Textures[EFTT_GLOSS]);
            else
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(CRenderer::CV_r_glossdefault->GetString(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_DETAIL_OVERLAY])
      {
        if (!Res->m_Textures[EFTT_DETAIL_OVERLAY])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_DETAIL_OVERLAY), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_DETAIL_OVERLAY];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(CRenderer::CV_r_detaildefault->GetString(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
              if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
                Tex->m_TU.m_TexPic = mfLoadResourceTexture("detail/dirty", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_OPACITY])
      {
        if (!Res->m_Textures[EFTT_OPACITY])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_OPACITY), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_OPACITY];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(CRenderer::CV_r_opacitydefault->GetString(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
              if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
                Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_SPECULAR])
      {
        if (!Res->m_Textures[EFTT_SPECULAR])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_SPECULAR), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_SPECULAR];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              if (Res->m_Textures[EFTT_DIFFUSE])
                Tex->m_TU.m_TexPic = mfLoadResourceTexture(Res->m_Textures[EFTT_DIFFUSE]->m_Name.c_str(), patch, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_DIFFUSE]->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_BUMP])
      {
#ifdef USE_3DC
        if (gRenDev->m_bDeviceSupportsComprNormalmaps && (ef->m_Flags & EF_ALLOW3DC))
        {
          Flags |= FT_ALLOW3DC;
          if (CRenderer::CV_r_texnormalmapcompressed)
            Flags |= FT_3DC;
        }
#endif
        if (Res->m_Textures[EFTT_BUMP] && !Res->m_Textures[EFTT_BUMP]->m_Name.empty() && Res->m_Textures[EFTT_NORMALMAP] && !Res->m_Textures[EFTT_NORMALMAP]->m_Name.empty())
        {
          char fullname[512];
          sprintf(fullname, "%s+norm_%s", Res->m_Textures[EFTT_BUMP]->m_Name.c_str(), Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str());
          Res->m_Textures[EFTT_BUMP]->m_TU.m_TexPic = mfLoadResourceTexture(fullname, patch, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags2(), eTT_Bumpmap, ef, Res->m_Textures[EFTT_BUMP], (float)Res->m_Textures[EFTT_BUMP]->m_Amount, (float)Res->m_Textures[EFTT_NORMALMAP]->m_Amount);
        }
        if (!Res->m_Textures[EFTT_BUMP])
          Res->AddTextureMap(EFTT_BUMP);
        Tex = Res->m_Textures[EFTT_BUMP];
        if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
        {
          Tex = Res->m_Textures[EFTT_BUMP];
          ETexType eTT = Dst[i].m_TUnits[j].m_eTexType==eTT_DSDTBump ? eTT_DSDTBump : eTT_Bumpmap;
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT, ef, Tex, (float)Tex->m_Amount);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            if (Res->m_Textures[EFTT_NORMALMAP])
            {
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str(), patch, Res->m_Textures[EFTT_NORMALMAP]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_NORMALMAP]->m_TU.GetTexFlags2(), eTT, ef, Res->m_Textures[EFTT_BUMP], (float)Res->m_Textures[EFTT_NORMALMAP]->m_Amount);
              SAFE_DELETE(Res->m_Textures[EFTT_NORMALMAP]);
            }
          }
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            Tex->m_TU.m_nFlags |= FTU_NOBUMP;
            if (!(Dst[i].m_TUnits[j].m_nFlags & FTU_BUMPPLANTS))
              Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white_ddn", patch, Flags, 0, eTT_Bumpmap, ef, Tex);
            else
              Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white_ddn", patch, Flags, 0, eTT_Bumpmap, ef, Tex);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_BUMP_DIFFUSE])
      {
        char nameBump[256];
        char nameNorm[256];
        nameBump[0] = 0;
        nameNorm[0] = 0;
        if (Res->m_Textures[EFTT_BUMP] && !Res->m_Textures[EFTT_BUMP]->m_Name.empty())
        {
          char *str;
          if (str=strstr(Res->m_Textures[EFTT_BUMP]->m_Name.c_str(), "_ddn"))
          {
            int nSize = str - Res->m_Textures[EFTT_BUMP]->m_Name.c_str();
            memcpy(nameBump, Res->m_Textures[EFTT_BUMP]->m_Name.c_str(), nSize);
            memcpy(&nameBump[nSize], "_ddndif", 7);
            strcpy(&nameBump[nSize+7], &str[4]);
            FILE *fp = iSystem->GetIPak()->FOpen(nameBump, "rb");
            if (!fp)
              nameBump[0] = 0;
            else
              iSystem->GetIPak()->FClose(fp);
          }
          if (!nameBump[0])
            strcpy(nameBump, Res->m_Textures[EFTT_BUMP]->m_Name.c_str());
        }
        if (Res->m_Textures[EFTT_NORMALMAP] && !Res->m_Textures[EFTT_NORMALMAP]->m_Name.empty())
        {
          char *str;
          if (str=strstr(Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str(), "_ddn"))
          {
            int nSize = str - Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str();
            memcpy(nameNorm, Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str(), nSize);
            memcpy(&nameNorm[nSize], "_ddndif", 7);
            strcpy(&nameNorm[nSize+7], &str[4]);
            FILE *fp = iSystem->GetIPak()->FOpen(nameNorm, "rb");
            if (!fp)
              nameNorm[0] = 0;
            else
              iSystem->GetIPak()->FClose(fp);
          }
          if (!nameNorm[0])
            strcpy(nameNorm, Res->m_Textures[EFTT_NORMALMAP]->m_Name.c_str());
        }
        if (!Res->m_Textures[EFTT_BUMP_DIFFUSE])
          Res->AddTextureMap(EFTT_BUMP_DIFFUSE);
        if (nameBump[0] && nameNorm[0])
        {
          char fullname[256];
          sprintf(fullname, "%s+norm_%s", nameBump, nameNorm);
          Res->m_Textures[EFTT_BUMP_DIFFUSE]->m_TU.m_TexPic = mfLoadResourceTexture(fullname, patch, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags2(), eTT_Bumpmap, ef, Res->m_Textures[EFTT_BUMP_DIFFUSE], (float)Res->m_Textures[EFTT_BUMP]->m_Amount, (float)Res->m_Textures[EFTT_NORMALMAP]->m_Amount);
        }
        Tex = Res->m_Textures[EFTT_BUMP_DIFFUSE];
        if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
        {
          ETexType eTT = Dst[i].m_TUnits[j].m_eTexType==eTT_DSDTBump ? eTT_DSDTBump : eTT_Bumpmap;
          if (nameBump[0])
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(nameBump, patch, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags2(), eTT, ef, Tex, (float)Res->m_Textures[EFTT_BUMP]->m_Amount);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            if (nameNorm[0])
              Tex->m_TU.m_TexPic = mfLoadResourceTexture(nameNorm, patch, Res->m_Textures[EFTT_NORMALMAP]->m_TU.GetTexFlags() | Flags, Res->m_Textures[EFTT_NORMALMAP]->m_TU.GetTexFlags2(), eTT, ef, Tex, (float)Res->m_Textures[EFTT_NORMALMAP]->m_Amount);
          }
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            Tex->m_TU.m_nFlags |= FTU_NOBUMP;
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white_ddn", patch, Flags, 0, eTT_Bumpmap, ef, Tex);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_DECAL_OVERLAY])
      {
        if (!Res->m_Textures[EFTT_DECAL_OVERLAY])
          Res->AddTextureMap(EFTT_DECAL_OVERLAY);
        Tex = Res->m_Textures[EFTT_DECAL_OVERLAY];
        if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex, (float)Tex->m_Amount);
          else
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/white", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex, (float)Tex->m_Amount);
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_ATTENUATION2D])
      {
        if (!Res->m_Textures[EFTT_ATTENUATION2D])
          Res->AddTextureMap(EFTT_ATTENUATION2D);
        Tex = Res->m_Textures[EFTT_ATTENUATION2D];
        if (!Tex->m_TU.m_TexPic)
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/PointLight2D", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_SUBSURFACE])
      {
        if (!Res->m_Textures[EFTT_SUBSURFACE])
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_SUBSURFACE), ef->m_Name.c_str());
        else
        {
          Tex = Res->m_Textures[EFTT_SUBSURFACE];
          if (!Tex->m_TU.m_TexPic)
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
            if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
            {
              Tex->m_TU.m_TexPic = NULL;
            }
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_ATTENUATION1D])
      {
        if (!Res->m_Textures[EFTT_ATTENUATION1D])
          Res->AddTextureMap(EFTT_ATTENUATION1D);
        Tex = Res->m_Textures[EFTT_ATTENUATION1D];
        if (!Tex->m_TU.m_TexPic)
        {
          if (!Tex->m_Name.empty())
            Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          if (!Tex->m_TU.m_TexPic || !Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            if (gRenDev->m_TexMan->m_Text_Atten1D)
              Tex->m_TU.m_TexPic = gRenDev->m_TexMan->m_Text_Atten1D;
            else
              Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/PointLight1D", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Base, ef, Tex);
          }
        }
      }
      else
      if (Dst[i].m_TUnits[j].m_TexPic == &gRenDev->m_TexMan->m_Templates[EFTT_CUBEMAP])
      {
        if (!Res->m_Textures[EFTT_CUBEMAP])
        {
          Warning( VALIDATOR_FLAG_TEXTURE,0,"WARNING: Missed texture '%s' for shader template '%s'", mfTemplateTexIdToName(EFTT_CUBEMAP), ef->m_Name.c_str());
          Res->AddTextureMap(EFTT_CUBEMAP);
        }
        Tex = Res->m_Textures[EFTT_CUBEMAP];
        if (!Tex->m_TU.m_TexPic)
        {
          Tex->m_TU.m_TexPic = mfLoadResourceTexture(Tex->m_Name.c_str(), patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Cubemap, ef, Tex);
          if (!Tex->m_TU.m_TexPic->IsTextureLoaded())
          {
            Tex->m_TU.m_TexPic = mfLoadResourceTexture("Textures/Defaults/DefaultCM", patch, Tex->m_TU.GetTexFlags() | Flags, Tex->m_TU.GetTexFlags2(), eTT_Cubemap, ef, Tex);
          }
        }
      }
    }
  }
}

bool CShader::mfSetOpacity (SShaderPass *Layer, float Opa, SShader *ef, int Mode)
{
  if (!Layer)
    return false;

  if (Mode == 0)
  {
    Layer->m_RenderState = GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
    if (ef->m_Flags3 & EF3_DEPTHWRITE)
      Layer->m_RenderState |= GS_DEPTHWRITE;

    //if (!ef->m_LightMaterial)
    {
      if (ef->m_eSort <= eS_Opaque)
        ef->m_eSort = eS_SeeThrough;
      Layer->m_FixedColor.dcolor = -1;
      float opa = CLAMP(Opa, 0.0f, 1.0f);
      Layer->m_FixedColor.bcolor[3] = (byte)(opa * 255.0f);
      Layer->m_eEvalRGB = eERGB_Fixed;
      Layer->m_eEvalAlpha = eEALPHA_Fixed;
    }
  }
  else
  {
    Layer->m_RenderState = GS_BLSRC_SRCALPHA | GS_BLDST_ONE;
    //if (!ef->m_LightMaterial)
    {
      if (ef->m_eSort <= eS_Opaque)
        ef->m_eSort = eS_SeeThrough;
      Layer->m_FixedColor.dcolor = -1;
      float opa = CLAMP(Opa, 0.0f, 1.0f);
      Layer->m_FixedColor.bcolor[3] = (byte)(opa * 255.0f);
      Layer->m_eEvalRGB = eERGB_Fixed;
      Layer->m_eEvalAlpha = eEALPHA_Fixed;
    }
  }

  return true;
}

bool SShader::mfSetOpacity(float Opa, int Mode)
{
  if (m_Passes.Num())
    gRenDev->m_cEF.mfSetOpacity(&m_Passes[0], Opa, this, Mode);
  else
  for (int i=0; i<m_HWTechniques.Num(); i++)
  {
    if (m_HWTechniques[i]->m_Passes.Num())
      gRenDev->m_cEF.mfSetOpacity(&m_HWTechniques[i]->m_Passes[0], Opa, this, Mode);
  }
  if (m_Templates)
  {
    for (int i=0; i<m_Templates->m_TemplShaders.Num(); i++)
    {
      if (!m_Templates->m_TemplShaders[i])
        continue;
      m_Templates->m_TemplShaders[i]->mfSetOpacity(Opa, Mode);
    }
  }
  return true;
}

//======================================================================================
// Templates support 

void SEfTemplates::mfFree(SShader *ef)
{
  for (int i=0; i<m_TemplShaders.Num(); i++)
  {
    if (m_TemplShaders[i])
      m_TemplShaders[i]->Release();
  }
  m_TemplShaders.Free();
  delete this;
}

void SEfTemplates::mfSetPreferred(SShader *ef)
{
  if (m_nPreferred >= 0 && m_TemplShaders.Num()>m_nPreferred && m_TemplShaders[m_nPreferred])
  {
    m_Preferred = m_TemplShaders[m_nPreferred];
    return;
  }
  if (m_TemplShaders.Num() > EFT_DEFAULT && m_TemplShaders[EFT_DEFAULT])
  {
    m_Preferred = m_TemplShaders[EFT_DEFAULT];
    return;
  }
  for (int i=0; i<m_TemplShaders.Num(); i++)
  {
    if (m_TemplShaders[i])
    {
      m_Preferred = m_TemplShaders[i];
      return;
    }
  }
  if (!(ef->m_Flags & EF_NOTFOUND) && (ef->m_Flags & EF_COMPILEDLAYERS))
  {
    m_Preferred = ef;
    return;
  }
}

void SEfTemplates::mfClear(SShader *e)
{
  for (int i=0; i<m_TemplShaders.Num(); i++)
  {
    SShader *ef = m_TemplShaders[i];
    if (!ef)
      continue;
    if (m_nMaskAuto & (1<<i))
    {
      delete ef;
      m_TemplShaders[i] = NULL;
    }
  }
  mfSetPreferred(e);
}

bool CShader::mfRegisterTemplate(int nTemplId, char *Name, bool bReplace, bool bDefault)
{
  if (nTemplId < 0)
  {
    Warning( 0,0,"Warning: CShader::mfRegisterTemplate: invalid template number %d\n", nTemplId);
    return false;
  }
  if (nTemplId < EFT_USER_FIRST && !bDefault)
    Warning( 0,0,"Warning: CShader::mfRegisterTemplate: attempt to redefine fixed template %d\n", nTemplId);

  if (nTemplId >= m_KnownTemplates.Num())
  {
    m_KnownTemplates.ReserveNew(nTemplId + 1);
  }
  if (m_KnownTemplates[nTemplId].m_Name[0] && !bReplace)
  {
    Warning( 0,0,"Warning: CShader::mfRegisterTemplate: template %d already defined\n", nTemplId);
    return false;
  }
  strncpy(m_KnownTemplates[nTemplId].m_Name, Name, 63);
  // Preload shader
  m_KnownTemplates[nTemplId].m_pShader = mfForName(Name, eSH_World, 0, NULL);
  return true;
}

void CShader::mfUnregisterDefaultTemplates()
{
  int i;
  for (i=0; i<m_KnownTemplates.Num(); i++)
  {
    SRegTemplate *rt = &m_KnownTemplates[i];
    SAFE_RELEASE(rt->m_pShader);
  }
  m_KnownTemplates.Free();
}

void CShader::mfRegisterDefaultTemplates()
{
  mfRegisterTemplate(EFT_WHITE, "White", false, true);
  mfRegisterTemplate(EFT_WHITESHADOW, "WhiteShadow", false, true);
  mfRegisterTemplate(EFT_DECAL, "TemplDecal", false, true);
  //mfRegisterTemplate(EFT_DETAIL_OVERLAY, "TemplDetail_Overlay", false, true);
  //mfRegisterTemplate(EFT_DETAIL_OVERLAY_VP, "TemplDetail_Overlay_VP", false, true);

  //mfRegisterTemplate(EFT_BUMP_MODEL, "TemplBumpDiffuse_NOCM", false, true);
  //mfRegisterTemplate(EFT_BUMP, "TemplBumpDiffuse", false, true);
  //mfRegisterTemplate(EFT_BUMPSPEC, "TemplBumpSpec", false, true);
  mfRegisterTemplate(EFT_HEATVISION, "TemplHeatVis_Sources", false, true);
  mfRegisterTemplate(EFT_INVLIGHT, "TemplInvLight", false, true);
  mfRegisterTemplate(EFT_DOF, "TemplDof", false, true);

  //mfRegisterTemplate(EFT_REFLECTCM, "TemplReflCM", false, true);
  //mfRegisterTemplate(EFT_REFLECTCMDECAL, "TemplReflCMDecal", false, true);
  //mfRegisterTemplate(EFT_REFLECTENVDECAL, "TemplReflEnvDecal", false, true);
  //mfRegisterTemplate(EFT_REFRACTCM, "TemplRefrCM", false, true);
  //mfRegisterTemplate(EFT_BUMPREFLECTCM, "TemplBumpReflCM", false, true);
  //mfRegisterTemplate(EFT_BUMPREFRACTCM, "TemplBumpRefrCM", false, true);
  //mfRegisterTemplate(EFT_BUMPREFLECTENVCM, "TemplBumpReflEnvCM", false, true);
  //mfRegisterTemplate(EFT_BUMPREFRACTENVCM, "TemplBumpRefrEnvCM", false, true);
  //mfRegisterTemplate(EFT_REFLECTENVCM, "TemplReflEnvCM", false, true);
  //mfRegisterTemplate(EFT_REFRACTENVCM, "TemplRefrEnvCM", false, true);
  //mfRegisterTemplate(EFT_REFRACTENVTEX, "TemplRefrEnvTex", false, true);
  //mfRegisterTemplate(EFT_DECALALPHATEST, "TemplDecalAlphaTestForShadows", false, true);
  //mfRegisterTemplate(EFT_ALPHABLENDDECAL, "TemplAlphaBlend", false, true);
  //mfRegisterTemplate(EFT_ENVBUMPENV2D, "TemplBumpOffsetEnv", false, true);
}

void CShader::mfRefreshResources(SShader *eft, SRenderShaderResources *Res)
{
  mfCheckShaderResTextures(eft->m_Passes, eft, Res);
  for (int i=0; i<eft->m_HWTechniques.Num(); i++)
  {
    SShaderTechnique *hs = eft->m_HWTechniques[i];
    mfCheckShaderResTexturesHW(hs->m_Passes, eft, Res);
  }
  if (Res && Res->m_Textures[EFTT_BUMP] && Res->m_Textures[EFTT_BUMP]->m_TU.m_TexPic && (Res->m_Textures[EFTT_BUMP]->m_TU.m_TexPic->m_Flags & FT_3DC) && (Res->m_Textures[EFTT_BUMP]->m_TU.m_TexPic->m_Flags & FT_HASALPHA) && (eft->m_Flags & EF_OFFSETBUMP))
  {
    const char *patch = Res->m_TexturePath.c_str();
    if (!Res->m_Textures[EFTT_BUMP_HEIGHT])
      Res->AddTextureMap(EFTT_BUMP_HEIGHT);
    Res->m_Textures[EFTT_BUMP_HEIGHT]->m_TU.m_TexPic = mfLoadResourceTexture(Res->m_Textures[EFTT_BUMP]->m_Name.c_str(), patch, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags() | FT_3DC | FT_3DC_A, Res->m_Textures[EFTT_BUMP]->m_TU.GetTexFlags2(), eTT_Bumpmap, eft, Res->m_Textures[EFTT_BUMP_HEIGHT], (float)Res->m_Textures[EFTT_BUMP]->m_Amount);
  }
}

bool CShader::mfAddTemplate(SRenderShaderResources *Res, SShader *ef, int IdTempl, const char *Name, uint64 nMaskGen)
{
  if (IdTempl < 0 || IdTempl >= m_KnownTemplates.Num() && !Name)
  {
    Warning( 0,0,"Warning: CShader::mfAddTemplate: invalid template number IdTempl=%d (not registered)\n", IdTempl);
    return false;
  }
  if (Name && IdTempl != EFT_DEFAULT && IdTempl < EFT_USER_FIRST)
  {
    Warning( 0,0,"Warning: CShader::mfAddTemplate: you cannot rename fixed template %d\n", IdTempl);
    return false;
  }
  SRegTemplate *rt = NULL;
  if (IdTempl < EFT_USER_FIRST)
    rt = &m_KnownTemplates[IdTempl];
  const char *name = Name;
  if (!name)
    name = rt->m_Name;

  if (!name || !name[0])
  {
    if (IdTempl == EFT_DEFAULT)
      Warning( 0,0,"Warning: CShader::mfAddTemplate: missed name for default template\n");
    else
      Warning( 0,0,"Warning: CShader::mfAddTemplate: template %d not registered\n", IdTempl);
    return false;
  }

  // Load template shader
  SShader *eft = mfForName(name, eSH_Misc, 0, NULL, nMaskGen);
  if (!eft || (eft->m_Flags & EF_NOTFOUND))
  {
    eft->Release();
    Warning( 0,0,"Warning: CShader::mfAddTemplate: Couldn't find template shader '%s' for '%s'\n", name, ef->m_Name.c_str());
    return false;
  }
  ef->m_Templates->m_nMaskAuto |= (1<<IdTempl);
  SShader *efdt = (SShader *)ef->GetTemplate(-1);
  bool bSep = false;
  if (IdTempl == EFT_DEFAULT)
  {
    ef->m_Flags3 = eft->m_Flags3;
  }
  if (eft->m_Flags3 & (EF3_USEPARENTCULL | EF3_USEPARENTSORT))
    bSep = true;
  if (IdTempl != EFT_DEFAULT)
  {
    if (efdt->m_Flags3 & (EF3_HASVCOLORS | EF3_HASALPHATEST | EF3_HASALPHABLEND))
      bSep = true;
  }
  else
  {
    ef->m_eCull = eft->m_eCull;
    ef->m_eSort = eft->m_eSort;
    ef->m_Flags3 |= (eft->m_Flags3 & (EF3_HASVCOLORS | EF3_HASALPHATEST | EF3_HASALPHABLEND));
  }
  if (bSep)
  {
    char nameEf[128];
    char sufName[128];
    strcpy(sufName, name);
    strlwr(sufName);
    sprintf(nameEf, "%s#%s", ef->m_Name.c_str(), sufName);
    SShader *efn = mfNewShader(eft->m_eClass, -1);
    int id = efn->m_Id;
    *efn = *eft;
    efn->m_Name = nameEf;
    efn->m_Id = id;
    if (efn->m_Flags3 & EF3_USEPARENTSORT)
      efn->m_eSort = ef->m_eSort;
    if (efn->m_Flags3 & EF3_USEPARENTCULL)
      efn->m_eCull = ef->m_eCull;
    efn->m_Flags3 |= (efdt->m_Flags3 & (EF3_HASVCOLORS | EF3_HASALPHATEST | EF3_HASALPHABLEND | EF3_NODRAW));
    eft = efn;
  }

  if (Res)
    mfRefreshResources(eft, Res);

  ef->m_Templates->mfReserve(IdTempl);
  ef->m_Templates->m_TemplShaders[IdTempl] = eft;
  return true;
}

void SShader::RemoveTemplate(int TemplId)
{
  if (!m_Templates || !m_Templates->m_TemplShaders[TemplId])
    return;
  if (m_Templates->m_TemplShaders[TemplId] != this)
    m_Templates->m_TemplShaders[TemplId]->Release();
  m_Templates->m_TemplShaders[TemplId] = NULL;
}

bool SShader::AddTemplate(SRenderShaderResources *Res, int& TemplId, const char *Name, bool bSetPreferred, uint64 nMaskGen)
{
  bool bRes = true;
  bool bHasT = true;
  char name[128] = "";

  if (m_Flags2 & (EF2_TEMPLATE | EF_SYSTEM))
    return false;

  if (!Name && (TemplId == EFT_HEATVISION))
  {
    if (m_Templates && m_Templates->m_TemplShaders.Num() && m_Templates->m_TemplShaders[0] && m_Templates->m_TemplShaders[0]->m_eSort > eS_Additive)
      return false;
  }

  if (Name && Name[0])
  {
    strcpy(name, Name);
    strlwr(name);
    if (m_Name == name)
      return true;

    // Try to find free template slot (or already registered with the same name)
    if (TemplId < 0)
    {
      if (!m_Templates)
        TemplId = EFT_USER_FIRST;
      else
      {
        int nFirst = -1;
        int i;
        for (i=EFT_USER_FIRST; i<m_Templates->m_TemplShaders.Num(); i++)
        {
          SShader *sh = m_Templates->m_TemplShaders[i];
          if (!sh)
          {
            nFirst = i;
            continue;
          }
          if (!strcmp(name, sh->m_Name.c_str()))
          {
            TemplId = i;
            return true;
          }
        }
        if (nFirst > 0)
          TemplId = nFirst;
        else
          TemplId = i;
      }
      if (TemplId < 0)
        return false;
    }
  }
  else
  if (TemplId < 0)
    return false;

  if (!m_Templates)
  {
    bHasT = false;
    m_Templates = new SEfTemplates;
  }

  if (Res && (!Res->m_Textures[EFTT_DIFFUSE] || Res->m_Textures[EFTT_DIFFUSE]->m_Name.empty()))
  {
    if (!Res->m_Textures[EFTT_DIFFUSE])
      Res->AddTextureMap(EFTT_DIFFUSE); 
  }

  //if (Res && (!Res->m_Textures[EFTT_SUBSURFACE] || Res->m_Textures[EFTT_SUBSURFACE]->m_Name.empty()))
  //{
  //  if (!Res->m_Textures[EFTT_SUBSURFACE])
  //    Res->AddTextureMap(EFTT_SUBSURFACE); 
  //}

  if (m_Flags3 & EF3_REBUILD)
    m_Templates->mfClear(this);
  bool bCheckResources = false;
  if (Res)
  {
    if (!(Res->m_nCheckedTemplates & (1<<TemplId)))
    {
      Res->m_nCheckedTemplates |= (1<<TemplId);
      bCheckResources = true;
    }
    if (Name)
    {
      for (int i=0; i<m_Templates->m_TemplShaders.Num(); i++)
      {
        if (!m_Templates->m_TemplShaders[i])
          continue;
        if (!(Res->m_nCheckedTemplates & (1<<i)))
        {
          Res->m_nCheckedTemplates |= (1<<i);
          gRenDev->m_cEF.mfRefreshResources(m_Templates->m_TemplShaders[i], Res);
        }
      }
    }
  }
  if (TemplId < m_Templates->m_TemplShaders.Num() && m_Templates->m_TemplShaders[TemplId])
  {
    if (!Name || !strcmp(name, m_Templates->m_TemplShaders[TemplId]->m_Name.c_str()))
    {
      if (bCheckResources)
        gRenDev->m_cEF.mfRefreshResources(m_Templates->m_TemplShaders[TemplId], Res);
      return true;
    }
    m_Templates->m_TemplShaders[TemplId]->Release();
    m_Templates->m_TemplShaders[TemplId] = NULL;
  }

  m_Templates->mfReserve(TemplId);

  bRes = gRenDev->m_cEF.mfAddTemplate(Res, this, TemplId, Name, nMaskGen);

  if (TemplId < EFT_USER_FIRST && TemplId != EFT_WHITE && TemplId != EFT_WHITESHADOW && bRes && bSetPreferred)
    m_Templates->mfSetPreferred(this);

  return bRes;
}

int CShader::mfTemplateNameToId(char *name)
{
  if (isdigit(name[0]))
    return atoi(name);

  int n = -1;
  if (!stricmp("Default", name))
    n = EFT_DEFAULT;
  else
  if (!stricmp("Decal", name))
    n = EFT_DECAL;
  else
  if (!stricmp("InvLight", name))
    n = EFT_INVLIGHT;
  else
  if (!stricmp("White", name))
    n = EFT_WHITE;
  else
  if (!stricmp("WhiteShadow", name))
    n = EFT_WHITESHADOW;
  else
  if (!stricmp("HeatVision", name))
    n = EFT_HEATVISION;
  else
  if (!strnicmp("User", name, 4))
    n = atoi(&name[4])+EFT_USER_FIRST;
  return n;
}

const char *CShader::mfTemplateTexIdToName(int Id)
{
  switch(Id)
  {
    case EFTT_DIFFUSE:
      return "Diffuse";
    case EFTT_GLOSS:
      return "Gloss";
    case EFTT_BUMP:
      return "Bump";
    case EFTT_CUBEMAP:
      return "Cubemap";
    case EFTT_OCCLUSION:
      return "Occlusion";
    case EFTT_ATTENUATION2D:
      return "Attenuation2D";
    case EFTT_SUBSURFACE:
      return "SubSurface";
    case EFTT_ATTENUATION1D:
      return "Attenuation1D";
    case EFTT_SPECULAR:
      return "Specular";
    case EFTT_DETAIL_OVERLAY:
      return "Detail";
    case EFTT_REFLECTION:
      return "Reflection";
    case EFTT_OPACITY:
      return "Opacity";
    case EFTT_LIGHTMAP:
    case EFTT_LIGHTMAP_HDR:
      return "Lightmap";
    case EFTT_DECAL_OVERLAY:
      return "Decal";
    default:
      return "Unknown";
  }
  return "Unknown";
}

void CShader::mfCompileTemplate(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  if (!ef->m_Templates)
    ef->m_Templates = new SEfTemplates;
  SEfTemplates *st = ef->m_Templates;
  int n = -1;
  SInputShaderResources Res(m_pCurResources);

  enum {eTexDecal = 1, eTexSubSurface, eTexAttenuation2D, eTexAttenuation1D, eTexProcedure, eTexDetail, eTexSpecular, ePreferred, eTexCubemap, eTexBump, eTexOcclusion, eTexGloss, eTemplate};
  static tokenDesc commands[] =
  {
    {eTexAttenuation2D, "TexAttenuation2D"},
    {eTexAttenuation1D, "TexAttenuation1D"},
    {eTexDecal, "TexDecal"},
    {eTexDetail, "TexDetail"},
    {eTexProcedure, "TexProcedure"},
    {eTexCubemap, "TexCubemap"},
    {eTexBump, "TexBump"},
    {eTexOcclusion, "TexOcclusion"},
    {eTexGloss, "TexGloss"},
    {eTexSpecular, "TexSpecular"},
    {eTexSubSurface, "TexSubSurface"},
    {eTemplate, "Template"},
    {ePreferred, "Preferred"},

    {0,0}
  };

  SShader *eft;
  char szTemplName[128];
  char szTemplate[128];

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
      case eTexDecal:
        Res.m_Textures[EFTT_DIFFUSE].m_Name = data;
        Res.m_Textures[EFTT_DIFFUSE].m_TU.m_TexPic = NULL;
        break;

      case eTexAttenuation2D:
        Res.m_Textures[EFTT_ATTENUATION2D].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_ATTENUATION2D].m_Name = data;
        Res.m_Textures[EFTT_ATTENUATION2D].m_TU.m_nFlags |= FTU_CLAMP;
        break;

      case eTexSubSurface:
        Res.m_Textures[EFTT_SUBSURFACE].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_SUBSURFACE].m_Name = data;
        break;

      case eTexAttenuation1D:
        Res.m_Textures[EFTT_ATTENUATION1D].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_ATTENUATION1D].m_Name = data;
        Res.m_Textures[EFTT_ATTENUATION1D].m_TU.m_nFlags |= FTU_CLAMP;
        break;

      case eTexDetail:
        Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_DETAIL_OVERLAY].m_Name = data;
        break;

      case eTexBump:
        Res.m_Textures[EFTT_BUMP].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_BUMP].m_Name = data;
        Res.m_Textures[EFTT_BUMP].m_TU.m_eTexType = eTT_Bumpmap;
        break;

      case eTexSpecular:
        Res.m_Textures[EFTT_SPECULAR].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_SPECULAR].m_Name = data;
        break;

      case eTexOcclusion:
        Res.m_Textures[EFTT_OCCLUSION].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_OCCLUSION].m_Name = data;
        Res.m_Textures[EFTT_OCCLUSION].m_TU.m_eTexType = eTT_DSDTBump;
        break;

      case eTexCubemap:
        Res.m_Textures[EFTT_CUBEMAP].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_CUBEMAP].m_Name = data;
        Res.m_Textures[EFTT_CUBEMAP].m_TU.m_eTexType = eTT_Cubemap;
        break;

      case eTexGloss:
        Res.m_Textures[EFTT_GLOSS].m_TU.m_TexPic = NULL;
        Res.m_Textures[EFTT_GLOSS].m_Name = data;
        break;

      case ePreferred:
        st->m_nPreferred = mfTemplateNameToId(data);
        break;

      case eTemplate:
        strcpy(szTemplName, name);
        strcpy(szTemplate, params);
        break;
    }
  }
  m_pCurResources = mfCreateShaderResources(&Res, true);

  if (!szTemplate[0] || !szTemplName[0])
    return;

  eft = gRenDev->m_cEF.mfForName(szTemplate, ef->m_eClass, 0);
  eft->m_Flags2 |= EF2_TEMPLATE;
  if (!eft || (eft->m_Flags & EF_NOTFOUND))
    Warning( 0,0,"Warning: Couldn't find template shader '%s' for name '%s'", params, ef->m_Name.c_str());
  else
  {
    int flags3 = (ef->m_Flags3 | eft->m_Flags3) & (EF3_HASVCOLORS | EF3_HASALPHATEST | EF3_HASALPHABLEND);
    ef->m_Flags3 |= flags3;
    eft->m_Flags3 |= flags3;

    mfCheckShaderResTextures(eft->m_Passes, eft, m_pCurResources);
    for (int i=0; i<eft->m_HWTechniques.Num(); i++)
    {
      SShaderTechnique *hs = eft->m_HWTechniques[i];
      mfCheckShaderResTexturesHW(hs->m_Passes, eft, m_pCurResources);
    }
    n = mfTemplateNameToId(szTemplName);

    if (st->m_nPreferred < 0)
      st->m_nPreferred = n;
    if (n >= 0 && eft)
    {
      st->mfReserve(n);
      st->m_TemplShaders[n] = eft;
    }
  }

  st->mfSetPreferred(ef);
}

