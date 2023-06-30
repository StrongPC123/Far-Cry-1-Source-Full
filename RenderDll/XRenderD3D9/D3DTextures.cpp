/*=============================================================================
  D3DTextures.cpp : Direct3D specific texture manager implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "I3dengine.h"
#include "CryHeaders.h"
#include "../Common/RendElements/CREScreenCommon.h"

// tiago: added
#include "D3DCGPShader.h"
#include "D3DCGVProgram.h"

//TArray<SDynTX> gDTX;

int CD3D9TexMan::m_Format = D3DFMT_A8R8G8B8;
int CD3D9TexMan::m_FirstBind = 1;
FILE *STexPicD3D::m_TexUseLogFile = NULL;
int STexPicD3D::CV_r_LogTextureUsage;

//TTextureMap CD3D9TexMan::m_RefTexs;

//#define TEXPOOL D3DPOOL_DEFAULT
#define TEXPOOL D3DPOOL_MANAGED
//===============================================================================

void STexPic::ReleaseDriverTexture()
{
}

void STexPic::SetFilter()
{
}

void STexPic::SetWrapping()
{
}

void STexPic::Set(int nTexSlot)
{
}
void STexPic::SetClamp(bool bEnable)
{
}

bool STexPic::SetFilter(int nFilter)
{
  switch(nFilter)
  {
    case FILTER_NONE:
      m_RefTex.m_MinFilter = D3DTEXF_POINT;
      m_RefTex.m_MagFilter = D3DTEXF_POINT;
      m_RefTex.m_MipFilter = D3DTEXF_NONE;
      break;
    case FILTER_LINEAR:
      m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
      m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
      m_RefTex.m_MipFilter = D3DTEXF_NONE;
      break;
    case FILTER_BILINEAR:
      m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
      m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
      m_RefTex.m_MipFilter = D3DTEXF_POINT;
      break;
    case FILTER_TRILINEAR:
      m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
      m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
      m_RefTex.m_MipFilter = D3DTEXF_LINEAR;
      break;
    default:
      return false;
  }

  return true;
}

void STexPicD3D::Release(bool bForce)
{
  STexPic::Release(bForce);
}

void STexPicD3D::ReleaseDriverTexture()
{
  if (!(m_Flags2 & FT2_WASUNLOADED) && (m_Bind && m_Bind != TX_FIRSTBIND)) 
  {
    m_Flags2 &= ~FT2_PARTIALLYLOADED;
    if (m_LoadedSize >= 0)
    {
      if (m_LoadedSize)
        gRenDev->m_TexMan->m_StatsCurTexMem -= m_LoadedSize;
      else
        gRenDev->m_TexMan->m_StatsCurTexMem -= m_Size;
    }
    if (m_Mips[0])
    {
      int nSides = m_eTT == eTT_Cubemap ? 6 : 1;
      for (int nS=0; nS<nSides; nS++)
      {
        if (!m_Mips[nS])
          continue;
        for (int i=0; i<m_nMips; i++)
        {
          SMipmap *mp = m_Mips[nS][i];
          if (!mp)
            continue;
          mp->m_bUploaded = false;
        }
      }
    }
    Unlink();

    //gRenDev->m_TexMan->ValidateTexSize();

    m_LoadedSize = 0;
    if (m_pPoolItem)
    {
      STexPoolItem *pPool = m_pPoolItem;
      //assert(pPool->m_pAPITexture == m_RefTex.m_VidTex);
      RemoveFromPool();
      pPool->Unlink();
      delete pPool;
    }

    CD3D9Renderer *r = gcpRendD3D;
    LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
    IDirect3DBaseTexture9 *pTex = (IDirect3DBaseTexture9*)m_RefTex.m_VidTex;
    //sRemoveTX(this);
    SAFE_RELEASE(pTex);
    m_RefTex.m_VidTex = NULL;
  }
  else
  if (m_Bind == TX_FIRSTBIND && !m_Id)
  {
    if (m_LoadedSize)
      gRenDev->m_TexMan->m_StatsCurTexMem -= m_LoadedSize;
    else
      gRenDev->m_TexMan->m_StatsCurTexMem -= m_Size;
  }
}

byte *STexPic::GetData32()
{
  return NULL;
}

byte *STexPicD3D::GetData32()
{
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  IDirect3DTexture9 *pID3DTexture = NULL;
  IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
  LPDIRECT3DTEXTURE9 pID3DDstTexture = NULL;
  D3DLOCKED_RECT d3dlr;
  int wdt = m_Width;
  int hgt = m_Height;
  HRESULT h;
  LPDIRECT3DSURFACE9 pDestSurf;
  LPDIRECT3DSURFACE9 pSourceSurf;

  if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DDstTexture)))
    return NULL;
  h = pID3DDstTexture->GetSurfaceLevel(0, &pDestSurf);

  int nPrevStr = CRenderer::CV_r_texturesstreamingsync;
  if (m_Flags2 & (FT2_WASUNLOADED | FT2_PARTIALLYLOADED))
  {
    CRenderer::CV_r_texturesstreamingsync = 1;
    LoadFromCache(FPR_IMMEDIATELLY, 0);
  }

  if (m_eTT == eTT_Cubemap)
    pID3DCubeTexture = (IDirect3DCubeTexture9*)m_RefTex.m_VidTex;
  else
    pID3DTexture = (IDirect3DTexture9*)m_RefTex.m_VidTex;
  byte *pDst = NULL;

  if (pID3DCubeTexture)
  {
    pDst = new byte [wdt*hgt*4*6];
    for (int CubeSide=0; CubeSide<6; CubeSide++)
    {
      h = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)CubeSide, 0, &pSourceSurf);
      h = D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
      pDestSurf->LockRect(&d3dlr, NULL, 0);
      cryMemcpy(&pDst[wdt*hgt*4*CubeSide], d3dlr.pBits, wdt*hgt*4);
      pDestSurf->UnlockRect();
      SAFE_RELEASE(pDestSurf);
    }
  }
  else
  {
    pDst = new byte [wdt*hgt*4];
    h = pID3DTexture->GetSurfaceLevel(0, &pSourceSurf);
    h = D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
    pDestSurf->LockRect(&d3dlr, NULL, 0);
    cryMemcpy(pDst, d3dlr.pBits, wdt*hgt*4);
    pDestSurf->UnlockRect();
    SAFE_RELEASE(pSourceSurf);
  }
  SAFE_RELEASE(pDestSurf);
  SAFE_RELEASE(pID3DDstTexture);

  CRenderer::CV_r_texturesstreamingsync = nPrevStr;

  return pDst;
}


void STexPicD3D::SetClamp(bool bEnable)
{
  if (!m_RefTex.m_VidTex)
    return;
  if (bEnable)
    m_RefTex.bRepeats = false;
  else
    m_RefTex.bRepeats = true;
}

void CD3D9TexMan::SetTexture(int Id, ETexType eTT)
{
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  int tmu = r->m_TexMan->m_CurStage;

  STexPic *t = NULL;
  if (Id >= TX_FIRSTBIND)
  {
    STexPicD3D *tp = (STexPicD3D *)m_Textures[Id-TX_FIRSTBIND];
    if (tp && tp->m_Bind == Id)
    {
      tp->Set();
      return;
    }
  }
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it == m_RefTexs.end())
  {
    dv->SetTexture(tmu, NULL);
    r->m_RP.m_TexStages[tmu].Texture = NULL;
    return;
  }
  STexPic *tp = it->second;
  assert(tp);
  if (tp)
    tp->Set();
}

void STexPicD3D::Set(int nTexSlot)
{
  //PROFILE_FRAME(Texture_Changes);

  static int sRecursion = 0;
  if (!sRecursion)
  {
    if (CRenderer::CV_r_texbindmode>=1)
    {
      if (CRenderer::CV_r_texbindmode==1 && !(m_Flags & FT_FONT))
      {
        sRecursion++;
        gRenDev->m_TexMan->m_Text_NoTexture->Set();
        sRecursion--;
        return;
      }
      else
      if (CRenderer::CV_r_texbindmode==2 && (m_Flags2 & FT2_WASLOADED) && !(m_Flags & FT_NOREMOVE) && m_eTT == eTT_Base)
      {
        sRecursion++;
        gRenDev->m_TexMan->SetGridTexture(this);
        sRecursion--;
        return;
      }
      if (CRenderer::CV_r_texbindmode==3 && (m_Flags2 & FT2_WASLOADED) && !(m_Flags & FT_NOREMOVE))
      {
        sRecursion++;
        if (m_eTT == eTT_Bumpmap)
          gRenDev->m_TexMan->SetGridTexture(this);
        else
          gRenDev->m_TexMan->m_Text_Gray->Set();
        sRecursion--;
        return;
      }
      if (CRenderer::CV_r_texbindmode==4 && (m_Flags2 & FT2_WASLOADED) && !(m_Flags & FT_NOREMOVE) && m_eTT == eTT_Base)
      {
        sRecursion++;
        gRenDev->m_TexMan->m_Text_Gray->Set();
        sRecursion--;
        return;
      }
      if (CRenderer::CV_r_texbindmode==5 && (m_Flags2 & FT2_WASLOADED) && !(m_Flags & FT_NOREMOVE) && m_eTT == eTT_Bumpmap)
      {
        sRecursion++;
        gRenDev->m_TexMan->m_Text_WhiteBump->Set();
        sRecursion--;
        return;
      }
      if (CRenderer::CV_r_texbindmode==6 && nTexSlot==EFTT_DIFFUSE)
      {
        sRecursion++;
        gRenDev->m_TexMan->m_Text_White->Set();
        sRecursion--;
        return;
      }
    }
  }

  CD3D9Renderer *r = gcpRendD3D;
  int tmu = r->m_TexMan->m_CurStage;
  if ((m_Flags2 & (FT2_WASUNLOADED | FT2_PARTIALLYLOADED | FT2_NEEDTORELOAD)))
  {
    PROFILE_FRAME(Texture_ChangesUpload);
    int Size = m_LoadedSize;
    Restore();
    if (Size != m_LoadedSize)
      r->m_RP.m_TexStages[tmu].Texture = NULL;
  }
  else
    Relink(&STexPic::m_Root);

  if (!m_RefTex.m_VidTex)
    return;

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log >= 3)
    r->Logv(SRendItem::m_RecurseLevel, "STexPic::Set(): (%d) \"%s\"\n", tmu, m_SourceName.c_str());
#endif

#ifdef DO_RENDERSTATS
  int nFrameID = r->m_nFrameUpdateID;
  if (m_AccessFrame != nFrameID)
  {
    m_AccessFrame = nFrameID;
    r->m_RP.m_PS.m_NumTextures++;
    if (m_LoadedSize)
      r->m_RP.m_PS.m_TexturesSize += m_LoadedSize;
    else
      r->m_RP.m_PS.m_TexturesSize += m_Size;
  }
#endif

  HRESULT hr;
  if (r->m_RP.m_TexStages[tmu].Texture == this)
    return;
  r->m_RP.m_TexStages[tmu].Texture = this;
  r->m_RP.m_PS.m_NumTextChanges++;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  hr = dv->SetTexture(tmu, (IDirect3DBaseTexture9*)m_RefTex.m_VidTex);

  if (m_RefTex.m_Pal>0 && (m_Flags & FT_PALETTED) && r->m_RP.m_TexStages[tmu].Palette != m_RefTex.m_Pal)
  {
    r->m_RP.m_TexStages[tmu].Palette = m_RefTex.m_Pal;
    dv->SetCurrentTexturePalette(m_RefTex.m_Pal);
  }

  if(m_RefTex.m_MipFilter!=r->m_RP.m_TexStages[tmu].nMipFilter)
  {
    r->m_RP.m_TexStages[tmu].nMipFilter = m_RefTex.m_MipFilter;
    dv->SetSamplerState(tmu, D3DSAMP_MIPFILTER, m_RefTex.m_MipFilter);
  }
  if(m_RefTex.m_MinFilter!=r->m_RP.m_TexStages[tmu].MinFilter || m_RefTex.m_MagFilter!=r->m_RP.m_TexStages[tmu].MagFilter || r->m_RP.m_TexStages[tmu].Anisotropic!=m_RefTex.m_AnisLevel)
  {
    r->m_RP.m_TexStages[tmu].MinFilter = m_RefTex.m_MinFilter;
    r->m_RP.m_TexStages[tmu].MagFilter = m_RefTex.m_MagFilter;
    r->m_RP.m_TexStages[tmu].Anisotropic = m_RefTex.m_AnisLevel;
    dv->SetSamplerState(tmu, D3DSAMP_MINFILTER, m_RefTex.m_MinFilter);
    dv->SetSamplerState(tmu, D3DSAMP_MAGFILTER, m_RefTex.m_MagFilter);
    if (m_RefTex.m_MinFilter == D3DTEXF_ANISOTROPIC)
      dv->SetSamplerState( tmu, D3DSAMP_MAXANISOTROPY, m_RefTex.m_AnisLevel);
  }
  if (m_RefTex.bRepeats != r->m_RP.m_TexStages[tmu].Repeat)
  {
    r->m_RP.m_TexStages[tmu].Repeat = m_RefTex.bRepeats;
    if (m_RefTex.bRepeats == 2)
    {
      dv->SetSamplerState(tmu, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRRORONCE);
      dv->SetSamplerState(tmu, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRRORONCE);
    }
    else
    if (m_RefTex.bRepeats == 0)
    {
      dv->SetSamplerState(tmu, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
      dv->SetSamplerState(tmu, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
    else
    if (m_RefTex.bRepeats == 1)
    {
      dv->SetSamplerState(tmu, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
      dv->SetSamplerState(tmu, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    if (m_RefTex.m_Type == TEXTGT_CUBEMAP || m_RefTex.m_Type == TEXTGT_3D)
    {
      if (m_RefTex.bRepeats == 1)
        dv->SetSamplerState(tmu, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
      else
        dv->SetSamplerState(tmu, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    }
  }
  if (m_RefTex.bProjected != r->m_RP.m_TexStages[tmu].Projected)
  {
    r->m_RP.m_TexStages[tmu].Projected = m_RefTex.bProjected;
    int nFlags = m_RefTex.bProjected ? D3DTTFF_PROJECTED : D3DTTFF_DISABLE;
    if (m_RefTex.bProjected && !(r->m_RP.m_FlagsPerFlush & RBSI_USEVP))
    {
      if (m_RefTex.m_Type == TEXTGT_CUBEMAP)
        nFlags |= D3DTTFF_COUNT4;
      else
        nFlags |= D3DTTFF_COUNT3;
    }
    dv->SetTextureStageState(tmu, D3DTSS_TEXTURETRANSFORMFLAGS, nFlags);
  }
  //SaveJPG("BugCube.jpg", false);

  r->m_TexMan->m_LastTex = this;
}

int SShaderTexUnit::mfSetTexture(int nt)
{
  CD3D9Renderer *rd = gcpRendD3D;
  
  rd->m_TexMan->m_CurStage = nt;

  SShaderTexUnit *pSTU = this;
  int nSetID = -1;
  int nTexSlot = -1;
  if (m_TexPic && m_TexPic->m_Bind < EFTT_MAX)
  {
    if (m_TexPic->m_Bind >= EFTT_LIGHTMAP && m_TexPic->m_Bind <= EFTT_OCCLUSION)
    {
      if (m_TexPic->m_Bind == EFTT_LIGHTMAP && rd->m_RP.m_pCurObject->m_nLMId)
      {
        rd->m_RP.m_FlagsPerFlush |= RBSI_USE_LM;
        nSetID = rd->m_RP.m_pCurObject->m_nLMId;
      }
      else
      if (m_TexPic->m_Bind == EFTT_LIGHTMAP_DIR && rd->m_RP.m_pCurObject->m_nLMDirId)
        nSetID = rd->m_RP.m_pCurObject->m_nLMDirId;
      else
      if (m_TexPic->m_Bind == EFTT_OCCLUSION && rd->m_RP.m_pCurObject->m_nOcclId)
        nSetID = rd->m_RP.m_pCurObject->m_nOcclId;
    }
    else
    if (nSetID < 0)
    {
      if (!rd->m_RP.m_pShaderResources || !rd->m_RP.m_pShaderResources->m_Textures[m_TexPic->m_Bind])
        Warning( VALIDATOR_FLAG_TEXTURE,0,"SShaderTexUnit::mfSetTexture: Missed template texture '%s' for shader '%s'\n", gRenDev->m_cEF.mfTemplateTexIdToName(m_TexPic->m_Bind), rd->m_RP.m_pShader->GetName());
      else
      {
        nTexSlot = m_TexPic->m_Bind;
        pSTU = &rd->m_RP.m_pShaderResources->m_Textures[nTexSlot]->m_TU;
        rd->m_RP.m_pShaderResources->m_Textures[nTexSlot]->Update(nt);
      }
    }
  }

  if (pSTU->m_AnimInfo)
    pSTU->mfUpdate();

  if (pSTU->m_TexPic) 
  {
    if (nSetID > 0)
    {
      gRenDev->m_TexMan->SetTexture(nSetID, eTT_Base);
    }
    else
    {
      int bind = pSTU->m_TexPic->m_Bind;
      if (bind >= TX_FIRSTBIND)
      {
        if (pSTU->m_TexPic->m_Flags & FT_3DC)
          rd->m_RP.m_FlagsPerFlush |= RBSI_USE_3DC;
        pSTU->m_TexPic->Set(nTexSlot);
        //pSTU->m_TexPic->SaveTGA("Phong1.tga", false);
      }
      else
      switch (bind)
      {
        case TO_FROMRE0:
        case TO_FROMRE1:
        case TO_FROMRE2:
        case TO_FROMRE3:
        case TO_FROMRE4:
        case TO_FROMRE5:
        case TO_FROMRE6:
        case TO_FROMRE7:
          {
            if (rd->m_RP.m_pRE)
              bind = rd->m_RP.m_pRE->m_CustomTexBind[bind-TO_FROMRE0];
            else
              bind = rd->m_RP.m_RECustomTexBind[bind-TO_FROMRE0];
            if (bind < 0)
              return false;
            rd->SetTexture(bind, pSTU->m_TexPic->m_eTT);
          }
          break;

        case TO_FROMOBJ:
          {
            if (rd->m_RP.m_pCurObject)
              bind = rd->m_RP.m_pCurObject->m_NumCM;
            if (bind <= 0)
              return 0;
            rd->SetTexture(bind, eTT_Base);
          }
          break;

        case TO_FROMLIGHT:
          {
            bool bRes = false;
            if (rd->m_RP.m_nCurLight < rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num())
            {
              CDLight *dl = rd->m_RP.m_pCurLight;
              if (dl && dl->m_pLightImage!=0)
              {
                bRes = true;
                STexPic *tp = (STexPic *)((ITexPic*)dl->m_pLightImage);
                if (dl->m_NumCM >= 0)
                  tp = rd->m_TexMan->m_CustomCMaps[dl->m_NumCM].m_Tex;
                else
                if (dl->m_fAnimSpeed)
                {
                  int n = 0;
                  STexPic *t = tp;
                  while (t)
                  {
                    t = t->m_NextTxt;
                    n++;
                  }
                  if (n > 1)
                  {
                    int m = (int)(rd->m_RP.m_RealTime / dl->m_fAnimSpeed) % n;
                    for (int i=0; i<m; i++)
                    {
                      tp = tp->m_NextTxt;
                    }
                  }
                }
                tp->Set();
              }
            }
            if (!bRes && !(rd->m_RP.m_PersFlags & RBPF_MULTILIGHTS))
              Warning( VALIDATOR_FLAG_TEXTURE,0,"Couldn't set projected texture for %d light source (Shader: '%s')\n", rd->m_RP.m_nCurLight, rd->m_RP.m_pShader->m_Name.c_str());
          }
          break;

        case TO_ENVIRONMENT_CUBE_MAP:
          {
            SEnvTexture *cm = NULL;
            cm = rd->m_cEF.mfFindSuitableEnvCMap(rd->m_RP.m_pCurObject->GetTranslation(), true, 0, 0);
            if (cm && cm->m_Tex)
              cm->m_Tex->Set();
            else
              return false;
          }
          break;

        case TO_ENVIRONMENT_LIGHTCUBE_MAP:
          {
            SEnvTexture *cm = NULL;
            cm = rd->m_cEF.mfFindSuitableEnvLCMap(rd->m_RP.m_pCurObject->GetTranslation(), true, 0, 0);
            if (cm && cm->m_Tex)
              cm->m_Tex->Set();
            else
              return false;
          }
          break;

        case TO_ENVIRONMENT_TEX:
          {
            SEnvTexture *cm = NULL;
            CCamera cam = rd->GetCamera();
            Vec3d Angs = cam.GetAngles();
            Vec3d Pos = cam.GetPos();
            bool bReflect = false;
            if ((rd->m_RP.m_pShader->m_Flags3 & (EF3_CLIPPLANE_FRONT | EF3_REFLECTION)))
              bReflect = true;
            cm = rd->m_cEF.mfFindSuitableEnvTex(Pos, Angs, true, 0, false, rd->m_RP.m_pShader, rd->m_RP.m_pShaderResources, rd->m_RP.m_pCurObject, bReflect, gRenDev->m_RP.m_pRE);
            if (cm)
              cm->m_Tex->Set();
            else
              return false;
          }
          break;

        case TO_SCREENMAP:
          if (rd->m_RP.m_PersFlags & RBPF_HDR)
            rd->m_TexMan->m_Text_ScreenMap_HDR->Set();
          else
            rd->m_TexMan->m_Text_ScreenMap->Set();
          break;

        default:
          {
            if (bind >= TO_CUSTOM_CUBE_MAP_FIRST && bind <= TO_CUSTOM_CUBE_MAP_LAST)
            {
              SEnvTexture *cm = &gRenDev->m_TexMan->m_CustomCMaps[bind-TO_CUSTOM_CUBE_MAP_FIRST];
              if (!cm->m_bReady)
              {
                Warning( VALIDATOR_FLAG_TEXTURE,0,"Custom CubeMap %d don't ready\n", bind-TO_CUSTOM_CUBE_MAP_FIRST);
                return false;
              }
              cm->m_Tex->Set();
            }
            else
            if (bind >= TO_CUSTOM_TEXTURE_FIRST && bind <= TO_CUSTOM_TEXTURE_LAST)
            {
              SEnvTexture *cm = &gRenDev->m_TexMan->m_CustomTextures[bind-TO_CUSTOM_TEXTURE_FIRST];
              if (!cm->m_bReady)
              {
                Warning( VALIDATOR_FLAG_TEXTURE,0,"Custom Texture %d don't ready\n", bind-TO_CUSTOM_TEXTURE_FIRST);
                return false;
              }
              cm->m_Tex->Set();
            }
            else
            {
              pSTU->m_TexPic->Set();
              //pSTU->m_TexPic->SaveJPG("NCMap.jpg", false);
            }
          }
          break;
      }
    }
  }
  else
  {
    rd->m_pd3dDevice->SetTexture(nt, NULL);
    rd->m_RP.m_TexStages[nt].Texture = NULL;
  }

  // Override texture sampler settings
  if (m_nFlags & (FTU_CLAMP | FTU_NOMIPS | FTU_PROJECTED | FTU_FILTERBILINEAR | FTU_FILTERTRILINEAR | FTU_FILTERNEAREST))
  {
    STexPicD3D *tp = (STexPicD3D *)rd->m_TexMan->m_LastTex;
    LPDIRECT3DDEVICE9 dv = rd->mfGetD3DDevice();
    if ((m_nFlags & FTU_CLAMP) && rd->m_RP.m_TexStages[nt].Repeat)
    {
      rd->m_RP.m_TexStages[nt].Repeat = false;
      dv->SetSamplerState(nt, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
      dv->SetSamplerState(nt, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
      if (tp->m_RefTex.m_Type == TEXTGT_CUBEMAP)
        dv->SetSamplerState(nt, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    }
    if ((m_nFlags & FTU_FILTERNEAREST) && (rd->m_RP.m_TexStages[nt].nMipFilter != D3DTEXF_NONE || rd->m_RP.m_TexStages[nt].MinFilter != D3DTEXF_POINT || rd->m_RP.m_TexStages[nt].MagFilter != D3DTEXF_POINT))
    {
      rd->m_RP.m_TexStages[nt].nMipFilter = D3DTEXF_NONE;
      rd->m_RP.m_TexStages[nt].MagFilter = D3DTEXF_POINT;
      rd->m_RP.m_TexStages[nt].MinFilter = D3DTEXF_POINT;
      dv->SetSamplerState(nt, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
      dv->SetSamplerState(nt, D3DSAMP_MINFILTER, D3DTEXF_POINT);
      dv->SetSamplerState(nt, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    }
    if ((m_nFlags & FTU_NOMIPS) && rd->m_RP.m_TexStages[nt].nMipFilter)
    {
      rd->m_RP.m_TexStages[nt].nMipFilter = 0;
      dv->SetSamplerState(nt, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    }
    else
    {
      if ((m_nFlags & FTU_FILTERBILINEAR) && rd->m_RP.m_TexStages[nt].nMipFilter != D3DTEXF_POINT)
      {
        rd->m_RP.m_TexStages[nt].nMipFilter = D3DTEXF_POINT;
        dv->SetSamplerState(nt, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
      }
      else
      if ((m_nFlags & FTU_FILTERTRILINEAR) && rd->m_RP.m_TexStages[nt].nMipFilter != D3DTEXF_LINEAR)
      {
        rd->m_RP.m_TexStages[nt].nMipFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(nt, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      }
    }
    if ((m_nFlags & FTU_PROJECTED) && !rd->m_RP.m_TexStages[nt].Projected)
    {
      rd->m_RP.m_TexStages[nt].Projected = true;
      int nFlags = D3DTTFF_PROJECTED;
      if (!(rd->m_RP.m_FlagsPerFlush & RBSI_USEVP))
      {
        if (tp->m_RefTex.m_Type == TEXTGT_CUBEMAP)
          nFlags |= D3DTTFF_COUNT4;
        else
          nFlags |= D3DTTFF_COUNT3;
      }
      dv->SetTextureStageState(nt, D3DTSS_TEXTURETRANSFORMFLAGS, nFlags);
    }
  }

  // Set tex. transforms/tex. gen modes (for fixed pipeline)
  if (m_GTC)
  {
    if (!m_GTC->mfSet(true))
      return 0;
    if (m_GTC->m_bDependsOnObject)
    {
      rd->m_RP.m_pGTC[nt] = m_GTC;
      rd->m_RP.m_FrameGTC = rd->m_RP.m_Frame;
    }
    else
      rd->m_RP.m_pGTC[nt] = NULL;
  }
  else
    rd->m_RP.m_pGTC[nt] = NULL;
  
  // Set texture color ops. (for fixed pipeline)
  if (m_eColorOp != eCO_NOSET)
  {
    if (m_eHDRColorOp && rd->m_nHDRType==1 && (rd->m_RP.m_PersFlags & RBPF_HDR))
      rd->m_RP.m_TexStages[nt].m_CO = m_eHDRColorOp;
    else
      rd->m_RP.m_TexStages[nt].m_CO = m_eColorOp;
    rd->m_RP.m_TexStages[nt].m_AO = m_eAlphaOp;
    rd->m_RP.m_TexStages[nt].m_CA = m_eColorArg;
    rd->m_RP.m_TexStages[nt].m_AA = m_eAlphaArg;
  }

  return 1;
}

bool SShaderPass::mfSetTextures()
{
  int i;
  CD3D9Renderer *rd = gcpRendD3D;
  rd->m_RP.m_FlagsPerFlush &= ~(RBSI_USE_LM | RBSI_USE_HDRLM | RBSI_USE_SPECANTIALIAS);
  for (i=0; i<m_TUnits.Num(); i++)
  {
    SShaderTexUnit *tl = &m_TUnits[i];
    tl->mfSetTexture(i);
    if (rd->m_RP.m_FlagsPerFlush & RBSI_USEVP)
    {
      if (rd->m_RP.m_TexStages[i].TCIndex != i)
      {
        rd->m_RP.m_TexStages[i].TCIndex = i;
        rd->m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
      }
    }
  }
  if (rd->m_RP.m_FlagsPerFlush & (RBSI_USE_LM | RBSI_USE_3DC | RBSI_USE_SPECANTIALIAS))
  {
    if ((rd->m_RP.m_FlagsPerFlush & RBSI_USE_LM) && (rd->m_RP.m_PersFlags & RBPF_HDR) && rd->m_RP.m_pCurObject->m_nHDRLMId)
    {
      rd->m_RP.m_FlagsPerFlush |= RBSI_USE_HDRLM;
      rd->EF_SelectTMU(i);
      rd->m_TexMan->SetTexture(rd->m_RP.m_pCurObject->m_nHDRLMId, eTT_Base);
      i++;
    }
    if ((rd->m_RP.m_FlagsPerFlush & RBSI_USE_3DC) && (rd->m_RP.m_pShader->m_Flags & EF_OFFSETBUMP))
    {
      if (rd->m_RP.m_pShaderResources && rd->m_RP.m_pShaderResources->m_Textures[EFTT_BUMP_HEIGHT])
      {
        rd->m_RP.m_FlagsPerFlush |= RBSI_USE_3DC_A;
        rd->EF_SelectTMU(i);
        rd->m_RP.m_pShaderResources->m_Textures[EFTT_BUMP_HEIGHT]->m_TU.m_TexPic->Set();
        i++;
      }
    }
  }
  CD3D9TexMan::BindNULL(i);
  return true;
}

void SShaderPass::mfResetTextures()
{
  int i;
  for (i=0; i<m_TUnits.Num(); i++)
  {
    SShaderTexUnit *tl = &m_TUnits[i];
    if (tl->m_GTC)
    {
      gcpRendD3D->EF_SelectTMU(i);
      tl->m_GTC->mfSet(false);
    }
  }
}

//=========================================================================

STexPic *CD3D9TexMan::GetByID(int Id)
{
  if (Id >= TX_FIRSTBIND)
  {
    int n = Id - TX_FIRSTBIND;
    if (n < m_Textures.Num())
    {
      STexPic *tp = m_Textures[n];
      if (tp && tp->m_Bind == Id)
        return tp;
    }
  }
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it != m_RefTexs.end())
    return it->second;
  return NULL;
}

STexPic *CD3D9TexMan::AddToHash(int Id, STexPic *ti)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it == m_RefTexs.end())
    m_RefTexs.insert(TTextureMapItor::value_type(Id, ti));
  else
  {
    STexPic *tpOther = it->second;
    assert(ti == tpOther);
  }
  return ti;
}

void CD3D9TexMan::RemoveFromHash(int Id, STexPic *ti)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it != m_RefTexs.end())
  {
    if (ti)
      assert(ti == it->second);
    IDirect3DBaseTexture9* vt = (IDirect3DBaseTexture9*)ti->m_RefTex.m_VidTex;
    //if (vt)
    //  sRemoveTX(ti);
    SAFE_RELEASE (vt);
    ti->m_RefTex.m_VidTex = NULL;
    m_RefTexs.erase(Id);
  }
}

CD3D9TexMan::~CD3D9TexMan()
{
  if (STexPicD3D::m_TexUseLogFile != NULL)
  {
    fclose(STexPicD3D::m_TexUseLogFile);
  }

  STexPicD3D::m_TexUseLogFile = NULL;
  SAFE_DELETE(m_TexCache);
}


STexPic *CD3D9TexMan::CreateTexture()
{
#ifdef DEBUGALLOC
#undef new
#endif
  return new STexPicD3D;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
}

bool CD3D9TexMan::SetFilter(char *tex)  
{
  int i;
  struct textype
  {
    char *name;
    uint typemin;
    uint typemag;
    uint typemip;
  };

  static textype tt[] =
  {
    {"NEAREST", D3DTEXF_POINT, D3DTEXF_POINT, D3DTEXF_NONE},
    {"LINEAR", D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_NONE},
    {"BILINEAR", D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_POINT},
    {"TRILINEAR", D3DTEXF_LINEAR, D3DTEXF_LINEAR, D3DTEXF_LINEAR},
  };

  m_CurAnisotropic = CLAMP(CRenderer::CV_r_texture_anisotropic_level, 1, gcpRendD3D->m_MaxAnisotropyLevel);
  if (!(gRenDev->GetFeatures() & RFT_ALLOWANISOTROPIC))
    m_CurAnisotropic = 1;
  CRenderer::CV_r_texture_anisotropic_level = m_CurAnisotropic;
  strcpy(m_CurTexFilter, CD3D9Renderer::CV_d3d9_texturefilter->GetString());
  if ((gRenDev->GetFeatures() & RFT_ALLOWANISOTROPIC) && m_CurAnisotropic > 1)
    tex = "TRILINEAR";

  for (i=0; i<4; i++)
  {
    if (!stricmp(tex, tt[i].name) )
    {
      m_MinFilter = tt[i].typemin;
      m_MagFilter = tt[i].typemag;
      m_MipFilter = tt[i].typemip;
      if (i == 3 && m_CurAnisotropic > 1)
        iLog->Log("Apply anisotropic texture filtering (level: %d)", m_CurAnisotropic);
      else
        iLog->Log("Apply %s texture filtering", tex);

      for (i=0; i<m_Textures.Num(); i++)
      {
        if (m_Textures[i] && m_Textures[i]->m_bBusy)
        {
          if ((m_Textures[i]->m_Flags & FT_NOMIPS) && i > 1)
            continue;
          STexPicD3D *pTP = (STexPicD3D *)m_Textures[i];
          if (m_CurAnisotropic > 1 && !(pTP->m_Flags2 & FT2_NOANISO) && !(pTP->m_Flags & FT_NOMIPS))
            pTP->m_RefTex.m_AnisLevel = m_CurAnisotropic;
          else
            pTP->m_RefTex.m_AnisLevel = 1;
          if (pTP->m_RefTex.m_AnisLevel > 1)
          {
            if (gcpRendD3D->m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
              pTP->m_RefTex.m_MinFilter = D3DTEXF_ANISOTROPIC;
            else
              pTP->m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
            if (gcpRendD3D->m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
              pTP->m_RefTex.m_MagFilter = D3DTEXF_ANISOTROPIC;
            else
              pTP->m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
            if (gcpRendD3D->m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
              pTP->m_RefTex.m_MipFilter = D3DTEXF_LINEAR;
            else
              pTP->m_RefTex.m_MipFilter = D3DTEXF_POINT;
          }
          else
          {
            pTP->m_RefTex.m_MinFilter = GetMinFilter();
            pTP->m_RefTex.m_MagFilter = GetMagFilter();
            pTP->m_RefTex.m_MipFilter = GetMipFilter();
          }
          if (pTP->m_Flags & FT_NOMIPS)
            pTP->m_RefTex.m_MipFilter = D3DTEXF_NONE;
        }
      }
      return true;
    }
  }
  
  Warning( VALIDATOR_FLAG_TEXTURE,0,"Bad texture filter name <%s>\n", tex);
  return false;
}

int STexPic::DstFormatFromTexFormat(ETEX_Format eTF)
{
  return 0;
}

int STexPicD3D::DstFormatFromTexFormat(ETEX_Format eTF)
{
  switch(eTF)
  {
    case eTF_8888:
      return D3DFMT_A8R8G8B8;
      break;
    case eTF_0888:
      if (m_Flags & FT_HASALPHA)
        return D3DFMT_A8R8G8B8;
      else
        return D3DFMT_X8R8G8B8;
      break;
    case eTF_DXT1:
      return D3DFMT_DXT1;
    case eTF_DXT3:
      return D3DFMT_DXT3;
    case eTF_DXT5:
      return D3DFMT_DXT5;
    case eTF_DSDT_MAG:
      return D3DFMT_X8L8V8U8;
    default:
      assert(0);
      return D3DFMT_UNKNOWN;
  }
}

int STexPic::TexSize(int Width, int Height, int DstFormat)
{
  return 0;
}
int STexPicD3D::TexSize(int Width, int Height, int DstFormat)
{
  return CD3D9TexMan::TexSize(Width, Height, DstFormat);
}

int CD3D9TexMan::TexSize(int wdt, int hgt, int mode)
{
  switch (mode)
  {
    case D3DFMT_X8R8G8B8:
    case D3DFMT_X8L8V8U8:
    case D3DFMT_Q8W8V8U8:
    case D3DFMT_D24S8:
      return wdt * hgt * 4;

    case D3DFMT_V8U8:
      return wdt * hgt * 2;
    case D3DFMT_CxV8U8:
      return wdt * hgt * 2;

    case D3DFMT_V16U16:
      return wdt * hgt * 4;

#ifndef _XBOX
    case D3DFMT_R8G8B8:
      return wdt * hgt * 3;
#endif

    case D3DFMT_A8R8G8B8:
      return wdt * hgt * 4;

    case D3DFMT_A8:
      return wdt * hgt;

    case D3DFMT_A8L8:
      return wdt * hgt * 2;

    case D3DFMT_A4R4G4B4:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_R5G6B5:
    case D3DFMT_D16:
      return wdt * hgt * 2;

    case D3DFMT_DXT1:
    case D3DFMT_DXT3:
    case D3DFMT_DXT5:
      {
        int blockSize = (mode == D3DFMT_DXT1) ? 8 : 16;
        return ((wdt+3)/4)*((hgt+3)/4)*blockSize;
      }
    case MAKEFOURCC('A', 'T', 'I', '2'):
      {
        int blockSize = 16;
        return ((wdt+3)/4)*((hgt+3)/4)*blockSize;
      }

    case D3DFMT_P8:
      return wdt * hgt;

    case D3DFMT_A16B16G16R16F:
    case D3DFMT_A16B16G16R16:
      return wdt * hgt * 8;

    case D3DFMT_G16R16:
    case D3DFMT_G16R16F:
      return wdt * hgt * 4;

    case D3DFMT_R16F:
      return wdt * hgt * 2;

    case D3DFMT_R32F:
      return wdt * hgt * 4;

    default:
      assert(0);
      break;

  }
  return 0;
}

void CD3D9TexMan::CalcMipsAndSize(STexPic *ti)
{
  ti->m_nMips = 0;
  ti->m_Size = 0;
  int wdt = ti->m_Width;
  int hgt = ti->m_Height;
  STexPicD3D *tp = (STexPicD3D *)ti;
  int mode = tp->m_DstFormat;
  while (wdt || hgt)
  {
    if (!wdt)
      wdt = 1;
    if (!hgt)
      hgt = 1;
    ti->m_nMips++;
    ti->m_Size += TexSize(wdt,hgt,mode);
    if (ti->m_Flags & FT_NOMIPS)
      break;
    wdt >>= 1;
    hgt >>= 1;
  }
  ti->m_Size *= ti->m_Depth;
}

void STexPic::SaveTGA(const char *nam, bool bMips)
{
}
void STexPic::SaveJPG(const char *nam, bool bMips)
{
}

void STexPicD3D::SaveTGA(const char *name, bool bMips)
{
  if (!m_RefTex.m_VidTex)
    return;
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  IDirect3DTexture9 *pID3DTexture = NULL;
  IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
  if (m_eTT == eTT_Cubemap)
    pID3DCubeTexture = (IDirect3DCubeTexture9*)m_RefTex.m_VidTex;
  else
    pID3DTexture = (IDirect3DTexture9*)m_RefTex.m_VidTex;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  
  D3DSURFACE_DESC ddsdDescDest;
  if (pID3DTexture)
    pID3DTexture->GetLevelDesc(0, &ddsdDescDest);
  else
    pID3DCubeTexture->GetLevelDesc(0, &ddsdDescDest);
  int sComps = 0;
  int sCompDst = 0;
  int sCompSrc = 0;
  switch (ddsdDescDest.Format)
  {
    case D3DFMT_X8R8G8B8:
#ifndef _XBOX
    case D3DFMT_X8L8V8U8:
#endif
    case D3DFMT_A8R8G8B8:
      sComps = 4;
      sCompDst = 4;
      break;
  }
  if (sComps)
  {
    byte *pDst = new byte [ddsdDescDest.Width*ddsdDescDest.Height*4];
    if (pID3DCubeTexture)
    {
      for (int CubeSide=0; CubeSide<6; CubeSide++)
      {
        char nm[128];
        sprintf(nm, "%s_%d", name, CubeSide);

        // Lock the texture to copy the image data into the texture
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)CubeSide, 0, &d3dlr, NULL, 0);
        // Copy data to the texture 
        byte *pSrc = (byte *)d3dlr.pBits;
        byte *pd = pDst;

        for (uint j=0; j<ddsdDescDest.Width*ddsdDescDest.Height; j++)
        {
          for (int n=0; n<sComps; n++)
          {
            pd[n] = pSrc[n];
          }
          pd += sCompDst;
          pSrc += sComps;
        }
        WriteTGA(pDst, ddsdDescDest.Width, ddsdDescDest.Height, nm, 32);
        // Unlock the texture
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, 0);
      }
    }
    else
    {
      // Lock the texture to copy the image data into the texture
      h = pID3DTexture->LockRect(0, &d3dlr, NULL, 0);
      // Copy data to the texture 
      byte *pSrc = (byte *)d3dlr.pBits;
      byte *pd = pDst;

      for (uint j=0; j<ddsdDescDest.Width*ddsdDescDest.Height; j++)
      {
        for (int n=0; n<sComps; n++)
        {
          pd[n] = pSrc[n];
        }
        pd += sCompDst;
        pSrc += sComps;
      }
      WriteTGA(pDst, ddsdDescDest.Width, ddsdDescDest.Height, (char *)name, 32);
      // Unlock the texture
      pID3DTexture->UnlockRect(0);
    }
    delete [] pDst;
  }
}

void STexPicD3D::SaveJPG(const char *nam, bool bMips)
{
  if (!m_RefTex.m_VidTex)
    return;
  char name[64];
  StripExtension(nam, name);
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  IDirect3DTexture9 *pID3DTexture = NULL;
  IDirect3DTexture9 *pID3DSrcTexture = NULL;
  IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
  LPDIRECT3DSURFACE9 pDestSurf;
  LPDIRECT3DSURFACE9 pSourceSurf;
  if (m_eTT == eTT_Cubemap)
    pID3DCubeTexture = (IDirect3DCubeTexture9*)m_RefTex.m_VidTex;
  else
    pID3DTexture = (IDirect3DTexture9*)m_RefTex.m_VidTex;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  D3DSURFACE_DESC ddsdDescDest;
  
  int wdt = m_Width;
  int hgt = m_Height;
  if(FAILED(h = D3DXCreateTexture(dv, wdt, hgt, m_nMips, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture)))
    return;

  if (pID3DCubeTexture)
  {
    static char* scubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
    for (int CubeSide=0; CubeSide<6; CubeSide++)
    {
      char nm[128];
      sprintf(nm, "%s_%s.jpg", name, scubefaces[CubeSide]);

      h = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)CubeSide, 0, &pDestSurf);
      h = pID3DSrcTexture->GetSurfaceLevel(0, &pSourceSurf);
      h = D3DXLoadSurfaceFromSurface(pSourceSurf, NULL, NULL, pDestSurf, NULL, NULL, D3DX_FILTER_NONE, 0);
      SAFE_RELEASE(pDestSurf);
      SAFE_RELEASE(pSourceSurf);

      pID3DCubeTexture->GetLevelDesc(0, &ddsdDescDest);

      // Lock the texture to copy the image data into the texture
      h = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
      // Copy data to the texture 
      byte *pSrc = (byte *)d3dlr.pBits;
#ifndef _XBOX
      WriteJPG(pSrc, ddsdDescDest.Width, ddsdDescDest.Height, nm);
#endif
      // Unlock the texture
      pID3DSrcTexture->UnlockRect(0);
    }
  }
  else
  if (!bMips)
  {
    char nm[128];
    sprintf(nm, "%s.jpg", name);

    h = pID3DTexture->GetSurfaceLevel(0, &pDestSurf);
    h = pID3DSrcTexture->GetSurfaceLevel(0, &pSourceSurf);
    h = D3DXLoadSurfaceFromSurface(pSourceSurf, NULL, NULL, pDestSurf, NULL, NULL, D3DX_FILTER_NONE, 0);
    SAFE_RELEASE(pDestSurf);
    SAFE_RELEASE(pSourceSurf);

    pID3DTexture->GetLevelDesc(0, &ddsdDescDest);

    // Lock the texture to copy the image data into the texture
    h = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to the texture 
    byte *pSrc = (byte *)d3dlr.pBits;
#ifndef _XBOX
    WriteJPG(pSrc, ddsdDescDest.Width, ddsdDescDest.Height, (char *)nm);
#endif
    // Unlock the texture
    pID3DSrcTexture->UnlockRect(0);
  }
  else
  {
    for (int i=0; i<m_nMips; i++)
    {
      char nm[128];
      sprintf(nm, "%s[%d].jpg", name, i);

      h = pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
      h = pID3DSrcTexture->GetSurfaceLevel(i, &pSourceSurf);
      h = D3DXLoadSurfaceFromSurface(pSourceSurf, NULL, NULL, pDestSurf, NULL, NULL, D3DX_FILTER_NONE, 0);
      SAFE_RELEASE(pDestSurf);
      SAFE_RELEASE(pSourceSurf);

      pID3DTexture->GetLevelDesc(i, &ddsdDescDest);
      // Lock the texture to copy the image data into the texture
      h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
      // Copy data to the texture 
      byte *pSrc = (byte *)d3dlr.pBits;
#ifndef _XBOX
      WriteJPG(pSrc, ddsdDescDest.Width, ddsdDescDest.Height, (char *)nm);
#endif
      // Unlock the texture
      pID3DSrcTexture->UnlockRect(i);
    }
  }
  SAFE_RELEASE(pID3DSrcTexture);
}

LPDIRECT3DTEXTURE9 CD3D9TexMan::D3DCreateSrcTexture(STexPicD3D *ti, byte *src, D3DFORMAT srcFormat, int SizeSrc, int DXTSize)
{
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  LPDIRECT3DTEXTURE9 pID3DSrcTexture = NULL;
  D3DLOCKED_RECT d3dlr;
  int wdt = ti->m_Width;
  int hgt = ti->m_Height;
  int i;
  int offset = 0;
  int size;
  HRESULT hr;

  if( FAILED( hr = D3DXCreateTexture(dv, wdt, hgt, ti->m_nMips ? ti->m_nMips : 1, 0, srcFormat, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture ) ) )
    return NULL;
  if (ti->m_nMips)
  {
    int w = wdt;
    int h = hgt;
    for (i=0; i<ti->m_nMips; i++)
    {
      if (!w)
        w = 1;
      if (!h)
        h = 1;
      if (ti->m_Flags & FT_DXT)
      {
        if (offset >= DXTSize)
          break;
        int blockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
        size = ((w+3)/4)*((h+3)/4)*blockSize;

        // Lock the texture to copy the image data into the texture
        hr = pID3DSrcTexture->LockRect(i, &d3dlr, NULL, 0);
        // Copy data to the texture 
        cryMemcpy(d3dlr.pBits, &src[offset], size);
        // Unlock the texture
        pID3DSrcTexture->UnlockRect(i);

        offset += size;
      }
      else
      if (srcFormat == D3DFMT_X8R8G8B8 || srcFormat == D3DFMT_A8R8G8B8 || srcFormat == D3DFMT_X8L8V8U8 || srcFormat == D3DFMT_Q8W8V8U8)
      {
        size = w * h * 4;
        // Lock the texture to copy the image data into the texture
        hr = pID3DSrcTexture->LockRect(i, &d3dlr, NULL, 0);
        // Copy data to the texture 
        cryMemcpy(d3dlr.pBits, src, size);
        // Unlock the texture
        pID3DSrcTexture->UnlockRect(i);
        src += size;
      }
      else
      if (srcFormat == D3DFMT_P8)
      {
        size = w * h;
        // Lock the texture to copy the image data into the texture
        hr = pID3DSrcTexture->LockRect(i, &d3dlr, NULL, 0);
        // Copy data to the texture 
        cryMemcpy(d3dlr.pBits, src, size);
        // Unlock the texture
        pID3DSrcTexture->UnlockRect(i);
        src += size;
      }
      w >>= 1;
      h >>= 1;
    }
  }
  else
  {
    // Lock the texture to copy the image data into the texture
    hr = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to the texture 
    cryMemcpy(d3dlr.pBits, src, SizeSrc);
    // Unlock the texture
    pID3DSrcTexture->UnlockRect(0);
  }
  return pID3DSrcTexture;
}

void CD3D9TexMan::D3DCreateVideoTexture(int tgt, byte *src, int wdt, int hgt, int depth, D3DFORMAT SrcFormat, D3DFORMAT DstFormat, STexPicD3D *ti, bool bMips, int CubeSide, PALETTEENTRY *pe, int DXTSize)
{
  int i = 0;
  int offset = 0;
  int size;
  
  LPDIRECT3DSURFACE9 pDestSurf;

  byte *pTemp = NULL;

  HRESULT hr;
  D3DLOCKED_RECT d3dlr;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  LPDIRECT3DCUBETEXTURE9 pID3DCubeTexture = NULL;
  LPDIRECT3DTEXTURE9 pID3DSrcTexture = NULL;
  LPDIRECT3DVOLUMETEXTURE9 pID3DVolTexture = NULL;

  ti->m_DstFormat = DstFormat;
  ti->m_CubeSide = CubeSide;  
  int D3DUsage = 0;
  D3DPOOL D3DPool = TEXPOOL;
  bool bGenMip = false;
  if (ti->m_eTT == eTT_Cubemap && CubeSide)
  {
    D3DSURFACE_DESC desc;
    pID3DCubeTexture = (LPDIRECT3DCUBETEXTURE9)m_pCurCubeTexture;
    pID3DCubeTexture->GetLevelDesc(0, &desc);
    if (bMips && !(desc.Usage & D3DUSAGE_AUTOGENMIPMAP) && ti->m_nMips <= 1)
      bGenMip = true;
  }
  else
  if (bMips && ti->m_nMips <= 1)
    bGenMip = true;

  if (bGenMip)
  {
    if(FAILED(hr = D3DXCreateTexture(dv, wdt, hgt, D3DX_DEFAULT, 0, SrcFormat, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture)))
      return;
    size = TexSize(wdt, hgt, SrcFormat);
    hr = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to src texture
    cryMemcpy((byte *)d3dlr.pBits, src, size);
    // Unlock the system texture
    pID3DSrcTexture->UnlockRect(0);
    hr = D3DXFilterTexture(pID3DSrcTexture, NULL, 0, D3DX_FILTER_LINEAR);
    size = 0;
    int w = wdt;
    int h = hgt;
    while (w || h)
    {
      if (!w)
        w = 1;
      if (!h)
        h = 1;
      size += TexSize(w, h, SrcFormat);
      w >>= 1;
      h >>= 1;
    }
    pTemp = new byte[size];
    int n = pID3DSrcTexture->GetLevelCount();
    w = wdt;
    h = hgt;
    int offs = 0;
    i = 0;
    while (w || h)
    {
      size = TexSize(w, h, SrcFormat);
      hr = pID3DSrcTexture->LockRect(i, &d3dlr, NULL, 0);
      // Copy data to src texture
      cryMemcpy(&pTemp[offs], (byte *)d3dlr.pBits, size);
      // Unlock the system texture
      hr = pID3DSrcTexture->UnlockRect(i);
      w >>= 1;
      h >>= 1;
      offs += size;
      i++;
    }
    ti->m_nMips = i;
    i = 0;
    src = pTemp;
    SAFE_RELEASE(pID3DSrcTexture);
  }

  if (bMips && ti->m_nMips <= 1 && !bGenMip)
    D3DUsage |= D3DUSAGE_AUTOGENMIPMAP;
  if (ti->m_Flags & FT_DYNAMIC)
  {
    //D3DUsage |= D3DUSAGE_DYNAMIC;
    // NVidia workaround
    // Use 16 bit video playing
    if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GF2)
    {
      DstFormat = D3DFMT_R5G6B5;
      ti->m_DstFormat = DstFormat;
    }
    D3DPool = D3DPOOL_MANAGED;
  }
  if (ti->m_Flags2 & FT2_RENDERTARGET)
  {
    D3DUsage |= D3DUSAGE_RENDERTARGET;
    D3DPool = D3DPOOL_DEFAULT;
  }
  if (DstFormat == D3DFMT_D24S8 || DstFormat == D3DFMT_D16)
  {
    D3DUsage |= D3DUSAGE_DEPTHSTENCIL;
    D3DUsage &= ~D3DUSAGE_RENDERTARGET;
    D3DPool = D3DPOOL_DEFAULT;
  }

  if (ti->m_eTT == eTT_Cubemap)
  {
    if (!ti->m_CubeSide)
    {
      IDirect3DBaseTexture9 *pTex = (IDirect3DBaseTexture9*)ti->m_RefTex.m_VidTex;
      SAFE_RELEASE(pTex);
    }
  }
  else
  {
    IDirect3DBaseTexture9 *pTex = (IDirect3DBaseTexture9*)ti->m_RefTex.m_VidTex;
    SAFE_RELEASE(pTex);
  }

  D3DTEXTUREFILTERTYPE MipFilter;
  switch (gcpRendD3D->CV_d3d9_texmipfilter)
  {
    case 0:
      MipFilter = D3DTEXF_POINT;
      break;
    case 1:
      MipFilter = D3DTEXF_LINEAR;
      break;
  }

  // Create the video texture
  if (tgt == TEXTGT_2D)
  {
    if (!(ti->m_Flags & FT_3DC) || gcpRendD3D->m_bDeviceSupportsComprNormalmaps > 1)
    {
      if( FAILED( hr = D3DXCreateTexture(dv, wdt, hgt, bMips ? D3DX_DEFAULT : 1, D3DUsage, DstFormat, D3DPool, &pID3DTexture ) ) )
      {
        iLog->Log("Error: CD3D9TexMan::D3DCreateVideoTexture: failed to create the texture %s (%s)", ti->m_SourceName.c_str(), gcpRendD3D->D3DError(hr));
        return;
      }
    }
    else
    {
      int nMips = 0;
      int w = wdt;
      int h = hgt;
      while (w && h)
      {
        if (!w)
          w = 1;
        if (!h)
          h = 1;
        if (w >= 4 && h >= 4)
          nMips++;
        w >>= 1;
        h >>= 1;
      }
      if (FAILED(hr=dv->CreateTexture(wdt, hgt, nMips, D3DUsage, DstFormat, D3DPool, &pID3DTexture, NULL)))
      {
        iLog->Log("Error: CD3D9TexMan::D3DCreateVideoTexture: failed to create the texture %s (%s)", ti->m_SourceName.c_str(), gcpRendD3D->D3DError(hr));
        return;
      }
    }
    if (D3DUsage & D3DUSAGE_AUTOGENMIPMAP)
      hr = pID3DTexture->SetAutoGenFilterType(MipFilter);
    ti->m_RefTex.m_VidTex = pID3DTexture;
    //if (D3DPool == D3DPOOL_DEFAULT)
    //  sAddTX(ti, NULL);
    if (D3DUsage & D3DUSAGE_RENDERTARGET)
      return;
    if (DstFormat == D3DFMT_D24S8 || DstFormat == D3DFMT_D16)
      return;
  }
  else
  if (tgt == TEXTGT_CUBEMAP)
  {
    int nMips = bMips ? D3DX_DEFAULT : 1;
    if (!(gcpRendD3D->m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP))
    {
      nMips = 1;
      ti->m_nMips = 1;
      bMips = false;
    }
    if (CubeSide == 0)
    {
      if( FAILED(hr = D3DXCreateCubeTexture(dv, wdt, bMips ? D3DX_DEFAULT : 1, D3DUsage,  DstFormat, D3DPool, &pID3DCubeTexture )))
        return;
      ti->m_RefTex.m_VidTex = pID3DCubeTexture;
      if (D3DUsage & D3DUSAGE_AUTOGENMIPMAP)
        hr = pID3DCubeTexture->SetAutoGenFilterType(MipFilter);
      m_pCurCubeTexture = pID3DCubeTexture;
      //if (D3DPool == D3DPOOL_DEFAULT)
      //  sAddTX(ti, NULL);
    }
    else
      pID3DCubeTexture = (LPDIRECT3DCUBETEXTURE9)m_pCurCubeTexture;
    if (ti->m_Flags2 & FT2_RENDERTARGET)
      return;
  }
  else
  if (tgt == TEXTGT_3D)
  {
    int nMips = bMips ? D3DX_DEFAULT : 1;
    if (!(gcpRendD3D->m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_MIPVOLUMEMAP))
    {
      nMips = 1;
      ti->m_nMips = 1;
      bMips = false;
    }
    if( FAILED(hr = D3DXCreateVolumeTexture(dv, wdt, hgt, depth, bMips ? D3DX_DEFAULT : 1, D3DUsage,  DstFormat, D3DPool, &pID3DVolTexture )))
      return;
    ti->m_RefTex.m_VidTex = pID3DVolTexture;
    if (D3DUsage & D3DUSAGE_AUTOGENMIPMAP)
      hr = pID3DVolTexture->SetAutoGenFilterType(MipFilter);
    //if (D3DPool == D3DPOOL_DEFAULT)
    //  sAddTX(ti, NULL);
    if (ti->m_Flags2 & FT2_RENDERTARGET)
      return;
  }

#if 0
  if ((SrcFormat==D3DFMT_DXT1 || SrcFormat==D3DFMT_DXT3 || SrcFormat==D3DFMT_DXT5) && SrcFormat==DstFormat)
  {
    int DxtBlockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
    ti->m_Size = 0;
    while (wdt || hgt)
    {
      if (offset == DXTSize)
        break;
      if (wdt == 0)
        wdt = 1;
      if (hgt == 0)
        hgt = 1;
      
      size = ((wdt+3)/4)*((hgt+3)/4)*DxtBlockSize;
      
      ti->m_Size += size;
      
      // Lock the texture to copy the image data into the texture
      if (pID3DCubeTexture)
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)CubeSide, i, &d3dlr, NULL, 0);
      else
        h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
      // Copy data to video texture 
      cryMemcpy((byte *)d3dlr.pBits, &src[offset], size);
      // Unlock the system texture
      if (pID3DCubeTexture)
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, i);
      else
        pID3DTexture->UnlockRect(i);
      i++;
      offset += size;
      wdt >>= 1;
      hgt >>= 1;
      if (!bMips)
        break;
    }
    if (pID3DCubeTexture)
      ti->m_RefTex.m_VidTex = pID3DCubeTexture;
    else
      ti->m_RefTex.m_VidTex = pID3DTexture;
    return;
  }
  if ((SrcFormat==D3DFMT_P8) && SrcFormat==DstFormat)
  {
#ifndef _XBOX
    HRESULT hr = dv->SetPaletteEntries(m_CurPal, pe);
    ti->m_RefTex.m_Pal = m_CurPal++;
#else
    HRESULT hr = dv->CreatePalette(D3DPALETTE_256, &ti->m_RefTex.m_pPal);
    if (FAILED(hr))
    {
      Warning( VALIDATOR_FLAG_TEXTURE,ti->m_SearchName.c_str(),"Couldn't create palette for texture '%s'\n",ti->m_SearchName.c_str() );
    }
    else
    {
      D3DCOLOR *pPal;
      ti->m_RefTex.m_pPal->Lock(&pPal, D3DLOCK_NOOVERWRITE);
      for (int i=0; i<256; i++)
      {
        pPal[i] = (pe[i].peFlags<<24) | (pe[i].peRed<<16) | (pe[i].peGreen<<8) | pe[i].peBlue;
      }
      ti->m_RefTex.m_pPal->Unlock();
    }
#endif
    
    byte *out = new byte [wdt*hgt];
    byte *outRet = out;
    while (wdt || hgt)
    {
      if (wdt == 0)
        wdt = 1;
      if (hgt == 0)
        hgt = 1;

      // Lock the texture to copy the image data into the texture
      if (pID3DCubeTexture)
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)CubeSide, i, &d3dlr, NULL, 0);
      else
        h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
      // Copy data to video texture P8
      cryMemcpy((byte *)d3dlr.pBits, src, wdt*hgt);
      // Unlock the system texture
      if (pID3DCubeTexture)
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, i);
      else
        pID3DTexture->UnlockRect(i);

      if (ti->m_nMips)
        src += wdt * hgt;
      else
      {
        MipMap8Bit (ti, src, out, wdt, hgt);
        Exchange(out, src);
      }
      wdt >>= 1;
      hgt >>= 1;

      i++;
      if (!bMips)
        break;
    }
    delete [] outRet;
    
    if (pID3DCubeTexture)
      ti->m_RefTex.m_VidTex = pID3DCubeTexture;
    else
      ti->m_RefTex.m_VidTex = pID3DTexture;
    return;
  }
  else
  if ((SrcFormat == D3DFMT_A8R8G8B8 || SrcFormat == D3DFMT_X8R8G8B8 || SrcFormat == D3DFMT_X8L8V8U8) && (DstFormat == D3DFMT_A8R8G8B8 || DstFormat == D3DFMT_X8R8G8B8 || DstFormat == D3DFMT_X8L8V8U8))
  {    
    ti->m_Size = 0;
    while (wdt || hgt)
    {
      if (wdt == 0)
        wdt = 1;
      if (hgt == 0)
        hgt = 1;

      size = wdt*hgt*4;

      ti->m_Size += size;

      // Lock the texture to copy the image data into the texture
      if (pID3DCubeTexture)
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)CubeSide, i, &d3dlr, NULL, 0);
      else
        h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
      // Copy data to video texture 
      cryMemcpy((byte *)d3dlr.pBits, &src[offset], size);
      // Unlock the system texture
      if (pID3DCubeTexture)
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, i);
      else
        pID3DTexture->UnlockRect(i);

      i++;
      offset += size;
      wdt >>= 1;
      hgt >>= 1;
      if (!bMips)
        break;
      if (i >= ti->m_nMips)
        break;
    }
    if (pID3DCubeTexture)
      ti->m_RefTex.m_VidTex = pID3DCubeTexture;
    else
      ti->m_RefTex.m_VidTex = pID3DTexture;
  }
  else
#else
  {
    int nMips = (bMips && ti->m_nMips) ? ti->m_nMips : 1;
    offset = 0;
    if (tgt == TEXTGT_3D)
    {
      assert(SrcFormat == DstFormat);
      
      // Fill the volume texture
      D3DLOCKED_BOX LockedBox;

      int w = wdt;
      int h = hgt;
      int d = depth;
      offset = 0;
      for (i=0; i<nMips; i++)
      {
        hr = pID3DVolTexture->LockBox(i, &LockedBox, 0, 0);
        if (!w && !h && !d)
          break;
        if (!w) w = 1;
        if (!h) h = 1;
        if (!d) d = 1;
        size = TexSize(w, 1, SrcFormat);
        for (int r=0; r<d; r++)
        {
          byte* pSliceStart = (byte*)LockedBox.pBits;
          for (int t=0; t<h; t++)
          {
            cryMemcpy((byte *)LockedBox.pBits, &src[offset], size);
            LockedBox.pBits = (BYTE*)LockedBox.pBits + LockedBox.RowPitch;
            offset += size;
          }
          LockedBox.pBits = pSliceStart + LockedBox.SlicePitch;
        }
        w >>= 1;
        h >>= 1;
        d >>= 1;

        pID3DVolTexture->UnlockBox(i);
      }
    }
    else
    {
      for (i=0; i<nMips; i++)
      {
        if (!wdt && !hgt)
          break;
        if (!wdt)
          wdt = 1;
        if (!hgt)
          hgt = 1;
        size = TexSize(wdt, hgt, SrcFormat);

        if ((ti->m_Flags & FT_3DC) && gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 1)
        {
          DWORD lockFlag = 0;
          if (i == 0)                                       // Figure out our locking flags, we can only discard on level 0
            lockFlag |= D3DLOCK_DISCARD;
          hr = pID3DTexture->LockRect(i, &d3dlr, NULL, lockFlag);
          // 3DC Workaround: we can't load mips less than 4x4 size
          if (wdt >= 4 && hgt >= 4)
            memcpy(d3dlr.pBits, &src[offset], size);
          hr = pID3DTexture->UnlockRect(i);
        }
        else
        {
          if (pID3DCubeTexture)
            hr = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)CubeSide, i, &pDestSurf);
          else
          if (pID3DTexture)
            hr = pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
          else
            assert(0);

          RECT srcRect;
          srcRect.left = 0;
          srcRect.top = 0;
          srcRect.right = wdt;
          srcRect.bottom = hgt;
          int nPitch;
          if (!(ti->m_Flags & FT_DXT))
            nPitch = size / hgt;
          else
          {
            int blockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
            nPitch = (wdt+3)/4 * blockSize;
          }
          /*D3DSURFACE_DESC ddsdDescDest;
          pID3DTexture->GetLevelDesc(0, &ddsdDescDest);
          hr = pDestSurf->LockRect(&d3dlr, NULL, 0);
          hr = pDestSurf->UnlockRect();*/

          assert(pDestSurf);
          hr = D3DXLoadSurfaceFromMemory(pDestSurf, NULL, NULL, &src[offset], SrcFormat, nPitch, NULL, &srcRect, D3DX_FILTER_NONE, 0);

          SAFE_RELEASE(pDestSurf);
        }

        offset += size;
        wdt >>= 1;
        hgt >>= 1;
      }
      if (nMips > 1 && (wdt || hgt))
      {
        if (wdt <= 2 && hgt <= 2)
        {
          offset -= size;
          while (wdt>0 || hgt>0)
          {
            if (wdt == 0)
              wdt = 1;
            if (hgt == 0)
              hgt = 1;
            if (pID3DCubeTexture)
              hr = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)CubeSide, i, &pDestSurf);
            else
              hr = pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
            RECT srcRect;
            srcRect.left = 0;
            srcRect.top = 0;
            srcRect.right = wdt;
            srcRect.bottom = hgt;
            int nPitch;
            if (!(ti->m_Flags & FT_DXT))
              nPitch = size / hgt;
            else
            {
              int blockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
              nPitch = (wdt+3)/4 * blockSize;
            }
            hr = D3DXLoadSurfaceFromMemory(pDestSurf, NULL, NULL, &src[offset], SrcFormat, nPitch, NULL, &srcRect, D3DX_FILTER_NONE, 0);
            SAFE_RELEASE(pDestSurf);
            i++;

            wdt >>= 1;
            hgt >>= 1;
          }
        }
        assert (!wdt && !hgt);
        if(wdt || hgt)
          Warning(0, ti->GetName(), "CD3D9TexMan::BuildMips: Texture has no requested mips: %s", ti->GetName());
      }
    }
  }
#endif

  if (pID3DCubeTexture)
    ti->m_RefTex.m_VidTex = pID3DCubeTexture;
  else
  if (pID3DVolTexture)
    ti->m_RefTex.m_VidTex = pID3DVolTexture;
  else
    ti->m_RefTex.m_VidTex = pID3DTexture;

  SAFE_DELETE_ARRAY(pTemp);
}

void CD3D9TexMan::D3DCompressTexture(int tgt, STexPicD3D *ti, int CubeSide)
{
  D3DSURFACE_DESC desc;
  D3DFORMAT Format;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  int i;

  LPDIRECT3DTEXTURE9 pID3DSrcTexture = NULL;
  LPDIRECT3DCUBETEXTURE9 pID3DSrcCubeTexture = NULL;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  LPDIRECT3DCUBETEXTURE9 pID3DCubeTexture = NULL;
  if (tgt == TEXTGT_2D)
    pID3DSrcTexture = (LPDIRECT3DTEXTURE9)ti->m_RefTex.m_VidTex;
  else
  {
    if (CubeSide < 5)
      return;
    pID3DSrcCubeTexture = (LPDIRECT3DCUBETEXTURE9)ti->m_RefTex.m_VidTex;
  }
  int wdt = ti->m_Width;
  int hgt = ti->m_Height;

  if (pID3DSrcTexture)
  {
    pID3DSrcTexture->GetLevelDesc(0, &desc);
    Format = desc.Format;
  }
  else
  {
    pID3DSrcCubeTexture->GetLevelDesc(0, &desc);
    Format = desc.Format;
  }
  
  if (Format == D3DFMT_X8R8G8B8 || Format == D3DFMT_R5G6B5 || Format == D3DFMT_X1R5G5B5)
    Format = D3DFMT_DXT1;
  else
    Format = D3DFMT_DXT3;

  HRESULT h;
    
  if (pID3DSrcTexture)
  {
    // Create the video texture
    if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, ti->m_RefTex.m_MipFilter ? D3DX_DEFAULT : 1, 0, Format, D3DPOOL_MANAGED, &pID3DTexture)))
      return;

    for (i=0; i<(int)pID3DSrcTexture->GetLevelCount(); i++) 
    {
      LPDIRECT3DSURFACE9 pDestSurf;
      LPDIRECT3DSURFACE9 pSourceSurf;
      pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
      pID3DSrcTexture->GetSurfaceLevel(i, &pSourceSurf);
      D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
      SAFE_RELEASE(pDestSurf);
      SAFE_RELEASE(pSourceSurf);  
    }
    ti->m_RefTex.m_VidTex = pID3DTexture;

    SAFE_RELEASE(pID3DSrcTexture);  
  }
  else
  if (pID3DSrcCubeTexture)
  {
    // Create the video texture
    if( FAILED( h = D3DXCreateCubeTexture(dv, hgt, ti->m_RefTex.m_MipFilter ? D3DX_DEFAULT : 1, 0, Format, D3DPOOL_MANAGED, &pID3DCubeTexture)))
      return;

    int side;
    for (side=0; side<6; side++)
    {
      for (i=0; i<(int)pID3DSrcCubeTexture->GetLevelCount(); i++) 
      {
        LPDIRECT3DSURFACE9 pDestSurf;
        LPDIRECT3DSURFACE9 pSourceSurf;
        pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)side, i, &pDestSurf);
        pID3DSrcCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)side, i, &pSourceSurf);
        D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
        SAFE_RELEASE(pDestSurf);
        SAFE_RELEASE(pSourceSurf);  
      }
    }
    
    SAFE_RELEASE(pID3DSrcCubeTexture);  
    m_pCurCubeTexture = pID3DCubeTexture;
  }
}

byte *CD3D9TexMan::GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips)
{
  D3DFORMAT Format, srcFormat;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  int i;
  D3DLOCKED_RECT d3dlr;

  srcFormat = D3DFMT_A8R8G8B8;

  int wdt = ti->m_Width;
  int hgt = ti->m_Height;

  int Size = wdt * hgt * 4;

  LPDIRECT3DTEXTURE9 pID3DSrcTexture = NULL;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;

  HRESULT h;

  if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, bMips ? D3DX_DEFAULT : 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture ) ) )
    return NULL;
  // Lock the texture to copy the image data into the texture
  h = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
  // Copy data to the texture 
  cryMemcpy(d3dlr.pBits, dst, Size);
  // Unlock the texture
  pID3DSrcTexture->UnlockRect(0);

  if (eF == eIF_DXT1)  
    Format = D3DFMT_DXT1;
  else
  if (eF == eIF_DXT3)  
    Format = D3DFMT_DXT3;
  else
  if (eF == eIF_DXT5)  
    Format = D3DFMT_DXT5;

  DWORD MipFilter;
  switch (gcpRendD3D->CV_d3d9_texmipfilter)
  {
    case 0:
      MipFilter = D3DX_FILTER_POINT;
      break;
    case 1:
    default:
      MipFilter = D3DX_FILTER_BOX;
      break;
    case 2:
      MipFilter = D3DX_FILTER_LINEAR;
      break;
    case 3:
      MipFilter = D3DX_FILTER_TRIANGLE;
      break;
  }

  if (bMips)
    h = D3DXFilterTexture(pID3DSrcTexture, NULL, 0, MipFilter);

  if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, bMips ? D3DX_DEFAULT : 1, 0, Format, D3DPOOL_SYSTEMMEM, &pID3DTexture ) ) )
  {
    SAFE_RELEASE(pID3DSrcTexture);  
    return NULL;
  }

  for (i=0; i<(int)pID3DSrcTexture->GetLevelCount(); i++) 
  {
    LPDIRECT3DSURFACE9 pDestSurf;
    LPDIRECT3DSURFACE9 pSourceSurf;
    pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
    pID3DSrcTexture->GetSurfaceLevel(i, &pSourceSurf);
    D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
    SAFE_RELEASE(pDestSurf);
    SAFE_RELEASE(pSourceSurf);  
  }

  SAFE_RELEASE(pID3DSrcTexture);  

  int nOffs = 0;
  Size = 0;
  for (i=0; i<(int)pID3DTexture->GetLevelCount(); i++) 
  {
    h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
    Size += TexSize(wdt, hgt, Format);
    h = pID3DTexture->UnlockRect(i);
    wdt >>= 1;
    hgt >>= 1;
    if (wdt < 1)
      wdt = 1;
    if (hgt < 1)
      hgt = 1;
  }
  *numMips = pID3DTexture->GetLevelCount();
  *DXTSize = Size;
  wdt = ti->m_Width;
  hgt = ti->m_Height;
  byte *data = new byte [Size];
  for (i=0; i<(int)pID3DTexture->GetLevelCount(); i++) 
  {
    h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
    Size = TexSize(wdt, hgt, Format);
    cryMemcpy(&data[nOffs], d3dlr.pBits, Size);
    nOffs += Size;
    h = pID3DTexture->UnlockRect(i);
    wdt >>= 1;
    hgt >>= 1;
    if (wdt < 1)
      wdt = 1;
    if (hgt < 1)
      hgt = 1;
  }

  SAFE_RELEASE(pID3DTexture);  

  return data;
}

STexPic *CD3D9TexMan::CopyTexture(const char *name, STexPic *tiSrc, int CubeSide)
{
  STexPic *ti = TextureInfoForName(name, -1, tiSrc->m_eTT, tiSrc->m_Flags, tiSrc->m_Flags2, 0);
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();

  ti->m_bBusy = true;
  ti->m_Flags = tiSrc->m_Flags;
  ti->m_Flags2 = tiSrc->m_Flags2;
  ti->m_Bind = TX_FIRSTBIND + ti->m_Id;
  AddToHash(ti->m_Bind, ti);
  ti->m_Width = tiSrc->m_Width;
  ti->m_Height = tiSrc->m_Height;
  ti->m_nMips = tiSrc->m_nMips;
  ti->m_ETF = tiSrc->m_ETF;
  ti->m_CubeSide = CubeSide;
  ti->m_DstFormat = tiSrc->m_DstFormat;

  LPDIRECT3DSURFACE9 pDestSurf;
  LPDIRECT3DSURFACE9 pSrcSurf;
  LPDIRECT3DCUBETEXTURE9 pID3DCubeTexture = NULL;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  HRESULT h;

  if (tiSrc->m_eTT == eTT_Cubemap)
  {
    pID3DCubeTexture = (LPDIRECT3DCUBETEXTURE9)tiSrc->m_RefTex.m_VidTex;
    for (int i=0; i<tiSrc->m_nMips; i++)
    {
      h = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)ti->m_CubeSide, i, &pDestSurf);
      h = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)tiSrc->m_CubeSide, i, &pSrcSurf);
      h = D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSrcSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
      SAFE_RELEASE(pDestSurf);
      SAFE_RELEASE(pSrcSurf);
    }
  }
  else
  {
    if( FAILED( h = D3DXCreateTexture(dv, ti->m_Width, ti->m_Height, tiSrc->m_nMips, 0, (D3DFORMAT)ti->m_DstFormat, D3DPOOL_MANAGED, &pID3DTexture)))
      return NULL;
    ti->m_RefTex.m_VidTex = (void *)pID3DTexture;
    LPDIRECT3DTEXTURE9 pID3DSrcTexture = (LPDIRECT3DTEXTURE9)tiSrc->m_RefTex.m_VidTex;
    for (int i=0; i<tiSrc->m_nMips; i++)
    {
      h = pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
      h = pID3DSrcTexture->GetSurfaceLevel(i, &pSrcSurf);
      h = D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSrcSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
      SAFE_RELEASE(pDestSurf);
      SAFE_RELEASE(pSrcSurf);
    }
  }
  return ti;
}

STexPic *CD3D9TexMan::CreateTexture(const char *name, int wdt, int hgt, int depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1, float fAmount2, int DXTSize, STexPic *tp, int bind, ETEX_Format eTF, const char *szSourceName)
{
  byte *dst1 = NULL;
  int i;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  LPDIRECT3DTEXTURE9 pID3DTexture;
  pID3DTexture = NULL;
  int DxtBlockSize = 0;
  int DxtOneSize = 0;
  bool bMips;
  int CubeSide = -1;
  PALETTEENTRY pe[256];

  //int w = ilog2(wdt);
  //int h = ilog2(hgt);
  //assert (w == wdt && h == hgt);

  if (!tp)
  {
    tp = TextureInfoForName(name, -1, eTT, flags, flags2, bind);

    tp->m_bBusy = true;
    tp->m_Flags = flags;
    tp->m_Flags2 = flags2;
    tp->m_Bind = TX_FIRSTBIND + tp->m_Id;
    AddToHash(tp->m_Bind, tp);
    tp->m_Height = hgt;
    tp->m_Width = wdt;
    tp->m_Depth = depth;
    tp->m_nMips = 0;
    tp->m_ETF = eTF;
    bind = tp->m_Bind;
  }
  if (szSourceName)
    tp->m_SourceName = szSourceName;

  STexPicD3D *ti = (STexPicD3D *)tp;
  if ((ti->m_Flags & FT_NOMIPS) || ti->m_nMips == 1)
  {
    bMips = false;
    ti->m_Flags |= FT_NOMIPS;
  }
  else
    bMips = true;
  if (ti->m_Flags & FT_DXT)
  {
    DxtBlockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
    DxtOneSize = ((wdt+3)/4)*((hgt+3)/4)*DxtBlockSize;
  }
  ti->m_DXTSize = DXTSize;
  ti->m_fAmount1 = fAmount1;
  ti->m_fAmount2 = fAmount2;

  if (dst)
  {
    if (ti->m_Flags & FT_CONV_GREY)
      ti->m_pData32 = ConvertRGB_Gray(dst, ti, ti->m_Flags, eTF);
    if (ti->m_Flags & FT_NODOWNLOAD)
    {
      if (ti->m_Flags & FT_DXT)
        ti->m_pData32 = ImgConvertDXT_RGBA(dst, ti, DXTSize);
      else
        ti->m_pData32 = dst;
      return ti;
    }

    D3DFORMAT format = D3DFMT_UNKNOWN;
    D3DFORMAT srcFormat = D3DFMT_X8R8G8B8;
    int SizeSrc = 0;
    if (eTF == eTF_8888 || eTF == eTF_RGBA)
    {
      srcFormat = D3DFMT_A8R8G8B8;
      SizeSrc = wdt * hgt * 4;
    }
    else
    if (eTF == eTF_0888)
    {
      int size = 0;
      int w = wdt;
      int h = hgt;
      if (ti->m_nMips)
      {
        for (int l=0; l<ti->m_nMips; l++)
        {
          if (!w)
            w = 1;
          if (!h)
            h = 1;
          size += w*h*4;
          w >>= 1;
          h >>= 1;
        }
        dst1 = new byte [size];
        w = wdt;
        h = hgt;
        byte *ds1 = dst1;
        byte *ds = dst;
        for (int l=0; l<ti->m_nMips; l++)
        {
          if (!w)
            w = 1;
          if (!h)
            h = 1;
          for (i=0; i<w*h; i++)
          {
            ds1[i*4+0] = ds[i*3+0];
            ds1[i*4+1] = ds[i*3+1];
            ds1[i*4+2] = ds[i*3+2];
            ds1[i*4+3] = 255;
          }
          ds1 += w*h*4;
          ds  += w*h*3;
          w >>= 1;
          h >>= 1;
        }
      }
      else
      {
        dst1 = new byte [wdt*hgt*4];
        byte *ds1 = dst1;
        byte *ds = dst;
        for (i=0; i<w*h; i++)
        {
          ds1[i*4+0] = ds[i*3+0];
          ds1[i*4+1] = ds[i*3+1];
          ds1[i*4+2] = ds[i*3+2];
          ds1[i*4+3] = 255;
        }
      }
      dst = dst1;
      SizeSrc = wdt * hgt * 4;
      srcFormat = D3DFMT_X8R8G8B8;
      eTF = eTF_8888;
    }
    else
    if (eTF == eTF_8000)
    {
      SizeSrc = wdt * hgt * depth;
      srcFormat = D3DFMT_A8;
      format = D3DFMT_A8;
      ti->m_Flags |= FT_HASALPHA;
    }
    else
    if (eTF == eTF_0088)
    {
      SizeSrc = wdt * hgt * depth * 2;
      srcFormat = D3DFMT_A8L8;
      format = D3DFMT_A8L8;
      ti->m_Flags |= FT_HASALPHA;
    }
    else
    if (eTF == eTF_DEPTH)
    {
      SizeSrc = wdt * hgt * 4;
      if (gcpRendD3D->mFormatDepth16.BitsPerPixel)
      {
        srcFormat = gcpRendD3D->mFormatDepth16.Format;
        format = gcpRendD3D->mFormatDepth16.Format;
      }
      else
      if (gcpRendD3D->mFormatDepth24.BitsPerPixel)
      {
        srcFormat = gcpRendD3D->mFormatDepth24.Format;
        format = gcpRendD3D->mFormatDepth24.Format;
      }
      else
        return NULL;
    }
    else
    if (eTF == eTF_DXT1 || eTF == eTF_DXT3 || eTF == eTF_DXT5)
    {
      if (eTF == eTF_DXT1)
        srcFormat = D3DFMT_DXT1;
      else
      if (eTF == eTF_DXT3)
        srcFormat = D3DFMT_DXT3;
      else
      if (eTF == eTF_DXT5)
        srcFormat = D3DFMT_DXT5;
    }
    else
    if (eTF == eTF_0565 || eTF == eTF_0555)
    {
      srcFormat = D3DFMT_R5G6B5;
    }
    else
    if (eTF == eTF_4444)
    {
      srcFormat = D3DFMT_A4R4G4B4;
      format = D3DFMT_A4R4G4B4;
    }

    if (!(ti->m_Flags & FT_DXT) && !ti->m_pPalette)
    {
      if (format != D3DFMT_A8 && !(gRenDev->GetFeatures() & RFT_HWGAMMA))
      {
        if (!CRenderer::CV_r_noswgamma && ti->m_eTT != eTT_Bumpmap)
          BuildImageGamma(ti->m_Width, ti->m_Height, dst, false);
      }
    }
    if (ti->m_Flags2 & FT2_FORCEDXT)
    {
      if (!(ti->m_Flags & FT_HASALPHA))
        format = D3DFMT_DXT1;
      else
        format = D3DFMT_DXT3;
    }
    else
    if (ti->m_Flags & FT_DXT)
    {
      if (ti->m_Flags & FT_DXT1)
        format = D3DFMT_DXT1;
      else
      if (ti->m_Flags & FT_DXT3)
        format = D3DFMT_DXT3;
      else
      if (ti->m_Flags & FT_DXT5)
        format = D3DFMT_DXT5;
      else
      {
        Warning( VALIDATOR_FLAG_TEXTURE,ti->m_SearchName.c_str(),"Unknown DXT format for texture %s", ti->m_SearchName.c_str());      
        format = D3DFMT_DXT1;
      }
      srcFormat = format;
      SizeSrc = DxtOneSize;
    }
    if (ti->m_Flags & FT_3DC)
    {
      if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 1)
      {
        format = (D3DFORMAT)(MAKEFOURCC('A', 'T', 'I', '2'));
        srcFormat = format;
        int DxtBlockSize = 16;
        SizeSrc = ((ti->m_Width+3)/4)*((ti->m_Height+3)/4)*DxtBlockSize;
      }
      else
      if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps > 1)
      {
        if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 2)
          format = D3DFMT_V8U8;
        else
        if (gcpRendD3D->m_bDeviceSupportsComprNormalmaps == 3)
          format = D3DFMT_CxV8U8;
        else
        {
          assert(0);
        }
        srcFormat = format;
        SizeSrc = ti->m_Width*ti->m_Height*2;
      }
    }
    else
    if (format == D3DFMT_UNKNOWN)
    {
      if (ti->m_Flags & FT_DYNAMIC)
      {
        if (ti->m_Flags & FT_HASALPHA)
          format = D3DFMT_A8R8G8B8;
        else
          format = D3DFMT_X8R8G8B8;
      }
      else
      if (!(ti->m_Flags & FT_HASALPHA) || ti->m_eTT == eTT_Bumpmap || ti->m_eTT == eTT_DSDTBump) // 3 components;
      {
        if (ti->m_eTT == eTT_Bumpmap)
        {
          if (CRenderer::CV_r_texbumpquality == 0)
          {
            if (ti->m_Flags & FT_HASALPHA)
              format = D3DFMT_A8R8G8B8;
            else
              format = D3DFMT_X8R8G8B8;
          }
          else
          if (CRenderer::CV_r_texbumpquality == 1)
          {
            ti->m_Flags |= FT_PALETTED;
            ti->m_pPalette = &m_NMPalette[0][0];
            dst1 = ConvertNMToPalettedFormat(dst, ti, eTF);
            dst = dst1;
            format = D3DFMT_P8;
            SizeSrc = wdt*hgt;
          }
          else
          if (CRenderer::CV_r_texbumpquality == 2)
            format = D3DFMT_DXT1;
          else
            format = D3DFMT_Q8W8V8U8;
        }
        else
        if (ti->m_eTT == eTT_DSDTBump)
        {
          format = D3DFMT_X8L8V8U8;
          srcFormat = D3DFMT_X8L8V8U8;
        }
        else
        if (srcFormat == D3DFMT_R5G6B5)
          format = srcFormat;
        else
        {
          if (CRenderer::CV_r_texquality == 0)
            format = D3DFMT_X8R8G8B8;
          else
          if (CRenderer::CV_r_texquality == 1)
            format = D3DFMT_R5G6B5;
          else
          if (CRenderer::CV_r_texquality == 2)
            format = D3DFMT_DXT1;
          else
            format = D3DFMT_X8R8G8B8;
        }
      }
      else
      if ((ti->m_Flags & FT_FONT))
        format = D3DFMT_A8;
      else
      {
        if (CRenderer::CV_r_texquality == 0)
          format = D3DFMT_A8R8G8B8;
        else
        if (CRenderer::CV_r_texquality == 1)
          format = D3DFMT_A4R4G4B4;
        else
        if (CRenderer::CV_r_texquality == 2)
          format = D3DFMT_DXT3;
        else
          format = D3DFMT_A8R8G8B8;
      }
    }
    if (ti->m_Flags & FT_PALETTED)
    {
      if (gRenDev->GetFeatures() & RFT_PALTEXTURE)
      {
        srcFormat = D3DFMT_P8;
        format = D3DFMT_P8;
        SizeSrc = wdt * hgt;
      }
      else
      {
        srcFormat = D3DFMT_A8R8G8B8;
        SRGBPixel *pal = ti->m_pPalette;
        ti->m_Flags &= ~FT_PALETTED;
        ti->m_ETF = eTF_8888;
        byte *pDst = new byte[wdt*hgt*4];
        for (int i=0; i<wdt*hgt; i++)
        {
          int l = dst[i];
          pDst[i*4+0] = pal[l].red;
          pDst[i*4+1] = pal[l].green;
          pDst[i*4+2] = pal[l].blue;
          pDst[i*4+3] = pal[l].alpha;
        }
        dst1 = pDst;
        dst  = pDst;
      }
    }

    // Create the src texture in system memory
#ifdef _XBOX
    if (srcFormat == D3DFMT_A8)
      srcFormat = D3DFMT_LIN_A8;
    else
    if (srcFormat == D3DFMT_A8R8G8B8)
      srcFormat = D3DFMT_LIN_A8R8G8B8;
    else
    if (srcFormat == D3DFMT_X8R8G8B8)
      srcFormat = D3DFMT_LIN_X8R8G8B8;
    else
    if (srcFormat == D3DFMT_A4R4G4B4)
      srcFormat = D3DFMT_LIN_A4R4G4B4;
    else
    if (srcFormat == D3DFMT_R5G6B5)
      srcFormat = D3DFMT_LIN_R5G6B5;
#endif

    int tgt = TEXTGT_2D;
    {
      {
        if (ti->m_eTT == eTT_Cubemap)
        {
          tgt = TEXTGT_CUBEMAP;
          int n = strlen(ti->m_SearchName.c_str()) - 4;
          if (!strcmp(&ti->m_SearchName.c_str()[n], "posx"))
            CubeSide = 0;
          else
          if (!strcmp(&ti->m_SearchName.c_str()[n], "negx"))
            CubeSide = 1;
          else
          if (!strcmp(&ti->m_SearchName.c_str()[n], "posy"))
            CubeSide = 2;
          else
          if (!strcmp(&ti->m_SearchName.c_str()[n], "negy"))
            CubeSide = 3;
          else
          if (!strcmp(&ti->m_SearchName.c_str()[n], "posz"))
            CubeSide = 4;
          else
          if (!strcmp(&ti->m_SearchName.c_str()[n], "negz"))
            CubeSide = 5;
          else
            CubeSide = 0;
          m_CurCubeFaces[CubeSide] = ti;
        }
        else
        if (ti->m_eTT == eTT_3D)
          tgt = TEXTGT_3D;

        if (!(ti->m_Flags & FT_DXT))
        {
          if (ti->m_pPalette && (ti->m_pData || dst))
          {
            if ((ti->m_Flags & FT_PALETTED) && (gRenDev->GetFeatures() & RFT_PALTEXTURE))
            {
              SRGBPixel *pal = ti->m_pPalette;
              
              // Create the color table
              for (i=0; i<256; i++)
              {
                pe[i].peBlue = pal[i].red;
                pe[i].peGreen = pal[i].green;
                pe[i].peRed = pal[i].blue;
                pe[i].peFlags = pal[i].alpha;
              }
              
              if (!ti->m_p8to24table)
                ti->m_p8to24table = new uint [256];
              if (!ti->m_p15to8table)
                ti->m_p15to8table = new uchar [32768];
              uint *table = ti->m_p8to24table;
              uint as = 255;
              uint r, g, b, a;
              uint v;
              for (i=0; i<256; i++)
              {
                r = pal->red;
                g = pal->green;
                b = pal->blue;
                a = pal->alpha;
                as &= a;
                pal++;
              
                v = (a<<24) + (r<<0) + (g<<8) + (b<<16);
                *table++ = v;
              }
              if (as < 255)
                ti->m_bAlphaPal = 1;

              for (i=0; i<(1<<15); i++)
              {
                /* Maps
                  0000 0000 0000 0000
                  0000 0000 0001 1111 = Red  = 0x1F
                  0000 0011 1110 0000 = Blue = 0x03E0
                  0111 1100 0000 0000 = Grn  = 0x7C00
                */
                r = ((i & 0x1F) << 3)+4;
                g = ((i & 0x03E0) >> 2)+4;
                b = ((i & 0x7C00) >> 7)+4;
                uchar *pal = (uchar *)ti->m_p8to24table;
                uint v;
                int j,k,l;
                int r1,g1,b1;
                for (v=0,k=0,l=10000*10000; v<256; v++,pal+=4)
                {
                  r1 = r-pal[0];
                  g1 = g-pal[1];
                  b1 = b-pal[2];
                  j = (r1*r1)+(g1*g1)+(b1*b1);
                  if (j<l)
                  {
                    k = v;
                    l = j;
                  }
                }
                ti->m_p15to8table[i] = k;
              }
            }
          }
        }
      }
      if (ti->m_Flags & FT_HDR)
        format = D3DFMT_A16B16G16R16F;
      ti->m_RefTex.m_MipFilter = bMips ? GetMipFilter() : 0;
      if (dst)
        D3DCreateVideoTexture(tgt, dst, wdt, hgt, depth, srcFormat, format, ti, bMips, CubeSide, pe, DXTSize);
      ti->m_RefTex.bProjected = (ti->m_Flags & FT_PROJECTED) ? true : false;
      if (ti->m_eTT == eTT_Cubemap)
      {
        ti->m_RefTex.bRepeats = false;
        ti->m_RefTex.m_MinFilter = GetMinFilter();
        ti->m_RefTex.m_MagFilter = GetMagFilter();
        ti->m_RefTex.m_AnisLevel = gcpRendD3D->GetAnisotropicLevel();
        if (ti->m_Flags2 & FT2_NOANISO)
          ti->m_RefTex.m_AnisLevel = 1;
        if (ti->m_RefTex.m_AnisLevel > 1 && ti->m_RefTex.m_MipFilter == D3DTEXF_LINEAR)
        {
          if (gcpRendD3D->m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
            ti->m_RefTex.m_MinFilter = D3DTEXF_ANISOTROPIC;
          else
            ti->m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
          if (gcpRendD3D->m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
            ti->m_RefTex.m_MagFilter = D3DTEXF_ANISOTROPIC;
          else
            ti->m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
        }
      }
      else
      {
        if (ti->m_Flags & FT_CLAMP)
          ti->m_RefTex.bRepeats = false;
        else
          ti->m_RefTex.bRepeats = true;
        ti->m_RefTex.m_MinFilter = GetMinFilter();
        ti->m_RefTex.m_MagFilter = GetMinFilter();
        ti->m_RefTex.m_AnisLevel = gcpRendD3D->GetAnisotropicLevel();
        if (ti->m_Flags2 & FT2_NOANISO)
          ti->m_RefTex.m_AnisLevel = 1;
        if (ti->m_RefTex.m_AnisLevel > 1 && ti->m_RefTex.m_MipFilter == D3DTEXF_LINEAR)
        {
          if (gcpRendD3D->m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
            ti->m_RefTex.m_MinFilter = D3DTEXF_ANISOTROPIC;
          else
            ti->m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
          if (gcpRendD3D->m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
            ti->m_RefTex.m_MagFilter = D3DTEXF_ANISOTROPIC;
          else
            ti->m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
        }
      }
      ti->m_RefTex.m_Type = tgt;
      if (CubeSide == 5)
      {
        for (i=0; i<6; i++)
        {
          if (i == 0)
          {
            m_CurCubeFaces[i]->m_RefTex.m_VidTex = m_pCurCubeTexture;
            //m_CurCubeFaces[i]->SaveJPG("Cube.jpg", false);
          }
          else
            m_CurCubeFaces[i]->m_RefTex.m_VidTex = NULL;
        }
      }
      
    } // if (!gIsDedicated)
  }  // if (dst)

  CD3D9TexMan::CalcMipsAndSize(ti);

  if (ti->m_Flags & FT_NOMIPS)
    ti->m_nMips = 1;
  if (ti->m_eTT == eTT_Cubemap)
  {
    ti->m_Size *= 6;
    if (CubeSide == 0)
      m_FirstCMSide = ti;
    else
    if (m_LastCMSide)
      m_LastCMSide->m_NextCMSide = ti;
    if (CubeSide == 5)
      m_LastCMSide = NULL;
    else
      m_LastCMSide = ti;
  }
  if (ti->m_eTT != eTT_Cubemap || !ti->m_CubeSide)
  {
    gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;
    ti->Unlink();
    ti->Link(&STexPic::m_Root);
    //sTestStr.AddElem(ti);
  }
  CheckTexLimits(NULL);

  if (ti->m_eTT != eTT_Cubemap || ti->m_CubeSide == 5)
  {
    if (m_Streamed == 1)
    {
      if (ti->m_eTT != eTT_Cubemap)
        ti->Unload();
      else
        m_FirstCMSide->Unload();
    }
  }

  if (dst1)
    delete [] dst1;

  return ti;
}

//============================================================================

void CD3D9TexMan::BuildMipsSub(byte* src, int wdt, int hgt)
{
}

void CD3D9TexMan::UpdateTextureRegion(STexPic *pic, byte *data, int X, int Y, int USize, int VSize)
{
  STexPicD3D *ti = (STexPicD3D *)pic;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  LPDIRECT3DCUBETEXTURE9 pID3DCubeTexture = NULL;
  int CubeSide = ti->m_CubeSide;
  HRESULT h;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  LPDIRECT3DSURFACE9 pDestSurf;
  if (ti->m_RefTex.m_Type == TEXTGT_2D)
  {
    pID3DTexture = (LPDIRECT3DTEXTURE9)ti->m_RefTex.m_VidTex;
    assert(pID3DTexture);
  }
  else
  {
    pID3DCubeTexture = (LPDIRECT3DCUBETEXTURE9)ti->m_RefTex.m_VidTex;
    assert(pID3DCubeTexture);
  }
  RECT rc;
  rc.left = X;
  rc.right = X + USize;
  rc.top = Y;
  rc.bottom = Y + VSize;
  RECT rcs;
  rcs.left = 0;
  rcs.right = USize;
  rcs.top = 0;
  rcs.bottom = VSize;
  h = pID3DTexture->GetSurfaceLevel(0, &pDestSurf);
  h = D3DXLoadSurfaceFromMemory(pDestSurf, NULL, &rc, data, D3DFMT_A8R8G8B8, USize*4, NULL, &rcs, D3DX_FILTER_NONE, 0);
  SAFE_RELEASE(pDestSurf);
}

void CD3D9TexMan::UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal)
{
  STexPicD3D *ti = (STexPicD3D *)pic;
  LPDIRECT3DTEXTURE9 pID3DTexture = NULL;
  LPDIRECT3DCUBETEXTURE9 pID3DCubeTexture = NULL;
  int CubeSide = ti->m_CubeSide;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  D3DSURFACE_DESC ddsdDescDest;
  if (ti->m_RefTex.m_Type == TEXTGT_2D)
  {
    pID3DTexture = (LPDIRECT3DTEXTURE9)ti->m_RefTex.m_VidTex;
    assert(pID3DTexture);
  }
  else
  {
    pID3DCubeTexture = (LPDIRECT3DCUBETEXTURE9)ti->m_RefTex.m_VidTex;
    assert(pID3DCubeTexture);
  }
  if (bPal)
  {
    bool bMips;
    if (CD3D9Renderer::CV_d3d9_mipprocedures && !(pic->m_Flags & FT_NOMIPS))
      bMips = true;
    else
      bMips = false;
    if (!ti->m_RefTex.m_MipFilter)
      bMips = false;
    ti->m_RefTex.m_MipFilter = bMips ? m_MipFilter : 0;
    int wdt = USize;
    int hgt = VSize;
    int i = 0;
    byte *out = new byte [wdt*hgt];
    byte *outRet = out;
    while (wdt || hgt)
    {
      if (wdt == 0)
        wdt = 1;
      if (hgt == 0)
        hgt = 1;

      // Lock the texture to copy the image data into the texture
      if (pID3DCubeTexture)
      {
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)CubeSide, i, &d3dlr, NULL, 0);
        pID3DCubeTexture->GetLevelDesc(i, &ddsdDescDest);
      }
      else
      {
        h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
        pID3DTexture->GetLevelDesc(i, &ddsdDescDest);
      }
      // Copy data to video texture P8
      cryMemcpy((byte *)d3dlr.pBits, data, ddsdDescDest.Width*ddsdDescDest.Height);
      // Unlock the system texture
      if (pID3DCubeTexture)
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, i);
      else
        pID3DTexture->UnlockRect(i);
      
      if (!bMips)
        break;

      MipMap8Bit (ti, (byte *)data, out, wdt, hgt);
      wdt >>= 1;
      hgt >>= 1;

      Exchange(out, (byte *)data);
      
      i++;
    }
    delete [] outRet;
  }
  else
  {
    bool bMips;
    if (CD3D9Renderer::CV_d3d9_mipprocedures && !(pic->m_Flags & FT_NOMIPS))
      bMips = true;
    else
      bMips = false;
    if (!ti->m_RefTex.m_MipFilter)
      bMips = false;
    ti->m_RefTex.m_MipFilter = bMips ? m_MipFilter : 0;
    int wdt = USize;
    int hgt = VSize;
    int i = 0;
    byte *out = new byte [wdt*hgt];
    byte *outRet = out;
    while (wdt || hgt)
    {
      if (wdt == 0)
        wdt = 1;
      if (hgt == 0)
        hgt = 1;

      // Lock the texture to copy the image data into the texture
      if (pID3DCubeTexture)
      {
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)CubeSide, i, &d3dlr, NULL, 0);
        pID3DCubeTexture->GetLevelDesc(i, &ddsdDescDest);
      }
      else
      {
        h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
        pID3DTexture->GetLevelDesc(i, &ddsdDescDest);
      }
      // Copy data to video texture A8R8G8B8
      cryMemcpy((byte *)d3dlr.pBits, data, ddsdDescDest.Width*ddsdDescDest.Height*4);
      // Unlock the texture
      if (pID3DCubeTexture)
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, i);
      else
        pID3DTexture->UnlockRect(i);
      
      if (!bMips)
        break;

      wdt >>= 1;
      hgt >>= 1;
      MipMap32Bit (ti, (byte *)data, out, wdt, hgt);

      Exchange(out, (byte *)data);
      
      i++;
    }
    delete [] outRet;
  }
  ti->Set();
}

static _inline int sLimitSizeByScreenRes(int size)
{
  while(true)
  {
    if (size>gRenDev->GetWidth() || size>gRenDev->GetHeight())
      size >>= 1;
    else
      break;
  }
  return size;
}

//TODO:replace with ARB_Buffer_Region and move into the class
void ClearBufferWithQuad(int x2,int y2,int x1,int y1,float fR,float fG,float fB,STexPic *pImage)
{ 
}

void CD3D9TexMan::ClearBuffer(int Width, int Height, bool bEnd, STexPic *pImage, int Side)
{
  gcpRendD3D->EF_ClearBuffers(true, true, NULL);
}

//===================================================================================

void CD3D9TexMan::DrawCubeSide( const float *angle, Vec3d& Pos, int tex_size, int side, int RendFlags, float fMaxDist)
{
  if (!iSystem)
    return;

  CRenderer * renderer = gRenDev;
  CCamera tmp_camera = renderer->GetCamera();
  CCamera prevCamera = tmp_camera;

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMinDist = 0.25f;

  tmp_camera.Init(tex_size,tex_size, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  tmp_camera.SetPos(Pos);
  tmp_camera.SetAngle(Vec3d(angle[0], angle[1], angle[2]));
  tmp_camera.Update();
  gRenDev->m_RP.m_PersFlags |= RBPF_DRAWMIRROR;
  gRenDev->m_RP.m_bDrawToTexture = true;

  iSystem->SetViewCamera(tmp_camera);
  gRenDev->SetCamera(tmp_camera);

  gRenDev->SetViewport(0, 0, tex_size, tex_size);

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. DrawLowDetail .. (DrawCubeSide %d)\n", side);
#endif

  //RendFlags &= ~DLD_TERRAIN_FULLRES;
  eng->DrawLowDetail(RendFlags);

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. End DrawLowDetail .. (DrawCubeSide %d)\n", side);
#endif

  gRenDev->m_RP.m_bDrawToTexture = false;
  gRenDev->m_RP.m_PersFlags &= ~RBPF_DRAWMIRROR;
  iSystem->SetViewCamera(prevCamera);
  gRenDev->SetCamera(prevCamera);
}

static float sAngles[6][5] = 
{
  {   90, -90, 0,  0, 0 },  //posx
  {   90, 90,  0,  1, 0 },  //negx
  {  180, 180, 0,  2, 0 },  //posy
  {   0, 180,  0,  0, 1 },  //negy
  {   90, 180, 0,  1, 1 },  //posz
  {   90, 0,   0,  2, 1 },  //negz
};

bool CD3D9TexMan::ScanEnvironmentCM (const char *name, int size, Vec3d& Pos)
{
  char szName[256];

  int RendFlags = -1;
  RendFlags &= ~DLD_ENTITIES;
  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  StripExtension(name, szName);
  //gRenDev->EF_SaveDLights();

  bool bBegin = false;
  if (!gcpRendD3D->m_SceneRecurseCount)
  {
    gcpRendD3D->m_nFrameID++;
    gcpRendD3D->m_pd3dDevice->BeginScene();
    bBegin = true;
  }
  gcpRendD3D->m_SceneRecurseCount++;

  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  LPDIRECT3DCUBETEXTURE9 pID3DTexture = NULL;
  LPDIRECT3DSURFACE9 pSrcSurf;
  LPDIRECT3DTEXTURE9 pID3DSysTexture;
  LPDIRECT3DSURFACE9 pSysSurf;
  D3DLOCKED_RECT d3dlrSys;
  HRESULT h;

  if( FAILED( h = D3DXCreateCubeTexture(dv, size, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pID3DTexture )))
    return false;

  if( FAILED( h = D3DXCreateTexture(dv, size, size, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSysTexture )))
    return false;
  h = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);

  int *pFR = (int *)gRenDev->EF_Query(EFQ_Pointer2FrameID);
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMaxDist = eng->GetMaxViewDistance();
  for(int n=0; n<6; n++)
  { 
    (*pFR)++;

    h = pID3DTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)n, 0, &pSrcSurf);
    gcpRendD3D->EF_SetRenderTarget(pSrcSurf, true);

    D3DCOLOR cColor = D3DRGBA(0.0f,0.0f,0.0f,0.0f);
    // render object
    dv->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

    DrawCubeSide( &sAngles[n][0], Pos, size, n, RendFlags, fMaxDist);
    SAFE_RELEASE(pSrcSurf);
    gcpRendD3D->EF_RestoreRenderTarget();
    h = pID3DTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)n, 0, &pSrcSurf);
    static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
    char str[256];
    int width = size;
    int height = size;
    h = dv->GetRenderTargetData(pSrcSurf, pSysSurf);
    h = pID3DSysTexture->LockRect(0, &d3dlrSys, NULL, 0);
    byte *pic = new byte [size*size*4];
    byte *src = (byte *)d3dlrSys.pBits;
    for (int i=0; i<size; i++)
    {
      for (int j=0; j<size; j++)
      {
        *(uint *)&pic[i*size*4+j*4] = *(uint *)&src[j*4];
        Exchange(pic[i*size*4+j*4], pic[i*size*4+j*4+2]);
      }
      src += d3dlrSys.Pitch;
    }
    h = pID3DSysTexture->UnlockRect(0);
    sprintf(str, "%s_%s.jpg", szName, cubefaces[n]);
    WriteJPG(pic, size, size, str); 
    delete [] pic;

    SAFE_RELEASE(pSrcSurf);
  }
  gcpRendD3D->EF_RestoreRenderTarget();
  gRenDev->SetViewport(vX, vY, vWidth, vHeight);
  //gRenDev->EF_RestoreDLights();

  SAFE_RELEASE(pSysSurf);
  SAFE_RELEASE(pID3DSysTexture);
  SAFE_RELEASE(pID3DTexture);

  if (bBegin)
  {
    gcpRendD3D->m_SceneRecurseCount--;
    if (!gcpRendD3D->m_SceneRecurseCount)
      gcpRendD3D->m_pd3dDevice->EndScene();
  }
  return true;
}

void CD3D9TexMan::GetAverageColor(SEnvTexture *cm, int nSide)
{
  assert (nSide>=0 && nSide<=5);

  if (!cm->m_RenderTargets[nSide])
    return;

  int i, j;
  HRESULT hr;
  LPDIRECT3DSURFACE9 pTargSurf = (LPDIRECT3DSURFACE9)cm->m_RenderTargets[nSide];
  D3DLOCKED_RECT d3dlr;
  hr = pTargSurf->LockRect(&d3dlr, NULL, D3DLOCK_READONLY);

  CFColor Col;
  int r = 0;
  int g = 0;
  int b = 0;
  int a = 0;
  byte *pData = (byte *)d3dlr.pBits;
  for (i=0; i<cm->m_TexSize; i++)
  {
    byte *pSrc = &pData[d3dlr.Pitch*i];
    for (j=0; j<cm->m_TexSize; j++)
    {
      b += pSrc[0];
      g += pSrc[1];
      r += pSrc[2];
      a += pSrc[3];

      pSrc += 4; 
    }
  }
  pTargSurf->UnlockRect();
  int size = cm->m_TexSize * cm->m_TexSize;
  cm->m_nFrameCreated[nSide] = -1;
  cm->m_EnvColors[nSide].bcolor[0] = r / size;
  cm->m_EnvColors[nSide].bcolor[1] = g / size;
  cm->m_EnvColors[nSide].bcolor[2] = b / size;
  cm->m_EnvColors[nSide].bcolor[3] = a / size;
}

void CD3D9TexMan::ScanEnvironmentCube(SEnvTexture *cm, int RendFlags, int Size, bool bLightCube)
{
  int i;

  if (cm->m_bInprogress)
    return;

  CD3D9Renderer *r = gcpRendD3D;
  int tex_size = Size;
  int nSizeTemp = 0;
  HRESULT hr;
  if (tex_size > 0)
  {
    int nlogSize = ilog2(tex_size);
    if (nlogSize != tex_size)
      tex_size = nlogSize;
    nSizeTemp = tex_size*2;
    if(nSizeTemp < 16)
      nSizeTemp = 16;
    if (!cm->m_RenderTargets[0] || cm->m_nFrameReset != r->m_nFrameReset || cm->m_TexSize != tex_size)
    {
      cm->m_nFrameReset = r->m_nFrameReset;
      cm->m_TexSize = tex_size;
      for (i=0; i<6; i++)
      {
        IDirect3DSurface9* pSurface;
        hr = r->mfGetD3DDevice()->CreateRenderTarget(tex_size,tex_size, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurface, NULL);
        if (!FAILED(hr))
        {
          cm->m_RenderTargets[i] = pSurface;
          cm->m_Tex->m_Flags |= FT_ALLOCATED;
        }
      }
    }
    if (!cm->m_TexTemp || cm->m_TexTemp->GetWidth() != nSizeTemp)
    {
      if (cm->m_TexTemp)
        cm->m_TexTemp->Release(false);
      char name[128];
      sprintf(name, "$TempLCMap%d", cm->m_Id);
      byte *data = new byte [nSizeTemp*nSizeTemp*4];
      cm->m_TexTemp = gRenDev->m_TexMan->CreateTexture(name, nSizeTemp, nSizeTemp, 1, FT_NOMIPS, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, NULL);
      delete [] data;
    }
  }
  else
  {
    if (tex_size <= 0)
    {
      switch (CRenderer::CV_r_envcmresolution)
      {
        case 0:
          tex_size = 64;
          break;
        case 1:
          tex_size = 128;
          break;
        case 2:
        default:
          tex_size = 256;
          break;
      }
    }
    while(true)
    {
      if (tex_size>r->GetWidth() || tex_size>r->GetHeight())
        tex_size >>= 1;
      else
        break;
    }
  }

  int n;
  Vec3d cur_pos;

  cm->m_bInprogress = true;
  int tid;
  if (bLightCube)
    tid = TO_ENVIRONMENT_LIGHTCUBE_MAP_REAL + cm->m_Id;
  else
    tid = TO_ENVIRONMENT_CUBE_MAP_REAL + cm->m_Id;
  if (!cm->m_RenderTargets[0])
  {
    if (!(cm->m_Tex->m_Flags & FT_ALLOCATED) || tex_size != cm->m_TexSize)
    {
      cm->m_Tex->m_Flags |= FT_ALLOCATED | FT_NOMIPS;
      cm->m_Tex->m_Flags &= ~FT_DXT;
      cm->m_Tex->m_Flags2 |= FT2_NODXT;
      if (!cm->m_TexTemp)
        cm->m_Tex->m_Flags2 |= FT2_RENDERTARGET;
      else
        cm->m_Tex->m_Flags |= FT_DYNAMIC;
      cm->m_Tex->m_nMips = 0;
      cm->m_Tex->m_Width = tex_size;
      cm->m_Tex->m_Height = tex_size;
      cm->m_TexSize = tex_size;
      byte *data = new byte [tex_size*tex_size*4];
      gRenDev->m_TexMan->CreateTexture(NULL, tex_size, tex_size, 1, FT_NOMIPS, cm->m_Tex->m_Flags2, data, eTT_Cubemap, -1.0f, -1.0f, 0, cm->m_Tex);
      SetTexture(0, eTT_Cubemap);
      delete [] data;
    }
  }
  cm->m_TexSize = tex_size;

  STexPicD3D *tp = (STexPicD3D *)cm->m_Tex;
  STexPicD3D *tpTemp = (STexPicD3D *)cm->m_TexTemp;
  LPDIRECT3DCUBETEXTURE9 pID3DTargetTextureCM = NULL;
  LPDIRECT3DTEXTURE9 pID3DTargetTexture = NULL;
  if (tpTemp)
    pID3DTargetTexture = (LPDIRECT3DTEXTURE9)tpTemp->m_RefTex.m_VidTex;
  if (tp)
    pID3DTargetTextureCM = (LPDIRECT3DCUBETEXTURE9)tp->m_RefTex.m_VidTex;

  //gRenDev->EF_SaveDLights();

  int Start, End;
  if (!cm->m_bReady || CRenderer::CV_r_envcmwrite)
  {
    Start = 0;
    End = 6;
  }
  else
  {
    Start = cm->m_MaskReady;
    End = cm->m_MaskReady+1;
  }
  //Start = 0;
  //End = 6;
  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;
  Vec3d Pos;
  gRenDev->m_RP.m_pRE->mfCenter(Pos, gRenDev->m_RP.m_pCurObject);
  //Pos += cm->m_CamPos;
//  assert(cm->m_Id == 0);

  HRESULT h;
  int *pFR = (int *)gRenDev->EF_Query(EFQ_Pointer2FrameID);
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMaxDist = eng->GetMaxViewDistance();
  if (bLightCube)
    fMaxDist *= 0.25f;
  for(n=Start; n<End; n++)
  { 
    (*pFR)++;
    LPDIRECT3DSURFACE9 pTargSurf, pSrcSurf;
    if (pID3DTargetTexture)
    {
      // Get average texture color before drawing to it
      if (cm->m_nFrameCreated[n]>0)
      {
        cm->m_nFrameCreated[n] = -1;
        gRenDev->m_TexMan->GetAverageColor(cm, n);
      }
      h = pID3DTargetTexture->GetSurfaceLevel(0, &pTargSurf);
    }
    else
    if (pID3DTargetTextureCM)
      h = pID3DTargetTextureCM->GetCubeMapSurface((D3DCUBEMAP_FACES)n, 0, &pTargSurf);
    h = r->EF_SetRenderTarget(pTargSurf, true);

    D3DCOLOR cColor = D3DRGBA(0.0f,0.0f,0.0f,0.0f);
    // render object
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);
    if (pID3DTargetTexture)
      DrawCubeSide( &sAngles[n][0], Pos, nSizeTemp, n, RendFlags, fMaxDist);
    else
    if (pID3DTargetTextureCM)
      DrawCubeSide( &sAngles[n][0], Pos, tex_size, n, RendFlags, fMaxDist);
    SAFE_RELEASE(pTargSurf);

    // Post process (bluring)
    if (pID3DTargetTexture && tpTemp)
    {
      /*{
        int width = nSizeTemp;
        int height = nSizeTemp;
        byte *pic = new byte [width * height * 4];
        LPDIRECT3DSURFACE9 pSysSurf, pTargetSurf;
        LPDIRECT3DTEXTURE9 pID3DSysTexture;
        D3DLOCKED_RECT d3dlrSys;
        h = pID3DTargetTexture->GetSurfaceLevel(0, &pTargetSurf);
        h = D3DXCreateTexture(r->m_pd3dDevice, nSizeTemp, nSizeTemp, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
        h = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);
        h = r->m_pd3dDevice->GetRenderTargetData(pTargetSurf, pSysSurf);
        h = pID3DSysTexture->LockRect(0, &d3dlrSys, NULL, 0);
        byte *src = (byte *)d3dlrSys.pBits;
        for (int i=0; i<width*height; i++)
        {
          *(uint *)&pic[i*4] = *(uint *)&src[i*4];
          Exchange(pic[i*4+0], pic[i*4+2]);
        }
        h = pID3DSysTexture->UnlockRect(0);
        WriteJPG(pic, width, height, "Cube1.jpg"); 
        delete [] pic;
        SAFE_RELEASE (pTargetSurf);
        SAFE_RELEASE (pSysSurf);
        SAFE_RELEASE (pID3DSysTexture);
      }*/
      cm->m_nFrameCreated[n] = r->GetFrameID();
      assert(cm->m_RenderTargets[n]);
      pSrcSurf = (LPDIRECT3DSURFACE9)cm->m_RenderTargets[n];
      h = r->EF_SetRenderTarget(pSrcSurf, true);
      r->SetViewport(0, 0, tex_size, tex_size);
      //r->Set2DMode(true,tex_size,tex_size);
      r->SetState(GS_NODEPTHTEST);
      r->SetColorOp(eCO_MODULATE,eCO_MODULATE,DEF_TEXARG0,DEF_TEXARG0);

      // Now blur the texture
      CREScreenProcess *sp = gRenDev->m_pREScreenProcess;
      CScreenVars *pVars = gRenDev->m_pREScreenProcess->GetVars();

      // set current vertex/fragment program    
      // get vertex/fragment program  
      /*CCGVProgram_D3D *vpBlur=(CCGVProgram_D3D *) pVars->m_pVPBlur;
      CCGPShader_D3D *fpBlur=(CCGPShader_D3D *) pVars->m_pRCBlur;
      vpBlur->mfSet(true, 0);
      fpBlur->mfSet(true, 0);
      // setup texture offsets, for texture neighboors sampling
      float s1=1.0f/(float) tex_size;     
      float t1=1.0f/(float) tex_size; 
      s1 *= 2;
      t1 *= 2;
      float pfOffset0[]={  s1*0.5f,    t1, 0.0f, 0.0f}; 
      float pfOffset1[]={  -s1,   t1*0.5f, 0.0f, 0.0f}; 
      float pfOffset2[]={ -s1*0.5f,   -t1, 0.0f, 0.0f}; 
      float pfOffset3[]={  s1,     -t1*0.5f, 0.0f, 0.0f};  */
      r->SelectTMU(0);
      tpTemp->Set();
      /*r->SelectTMU(1);
      tpTemp->Set();
      r->SelectTMU(2);
      tpTemp->Set();
      r->SelectTMU(3);
      tpTemp->Set();
      r->SelectTMU(0);
      vpBlur->mfParameter4f("Offset0", pfOffset0);
      vpBlur->mfParameter4f("Offset1", pfOffset1);
      vpBlur->mfParameter4f("Offset2", pfOffset2);
      vpBlur->mfParameter4f("Offset3", pfOffset3);
      float s_off=s1*0.5f; 
      float t_off=t1*0.5f; 

      // setup screen aligned quad
      struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F pScreenBlur[] =  
      {
        0,0,-1, 255,255,255,255, s_off, t_off,   
        0,tex_size,-1, 255,255,255,255, s_off, 1+t_off,    
        tex_size,0,-1, 255,255,255,255, 1+s_off, t_off,   
        tex_size,tex_size,-1, 255,255,255,255, 1+s_off, 1+t_off,   
      };*/ 
      int iBlurAmount = 1;
      for(int iBlurPasses=1; iBlurPasses<=iBlurAmount; iBlurPasses++) 
      {
        // set texture coordinates scale (needed for rectangular textures in gl ..)
        //float pfScale[]={ 1.0f, 1.0f, 1.0f, (float) iBlurPasses};     
        //vpBlur->mfParameter4f("vTexCoordScale", pfScale);

        // set current rendertarget
        //pRenderer->m_pd3dDevice->SetRenderTarget( 0, pTexSurf);
        // render screen aligned quad...
        r->DrawQuad(0,0,(float)tex_size,(float)tex_size,Col_White,1);
        //gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenBlur,VERTEX_FORMAT_P3F_COL4UB_TEX2F)), 4);  
      }
      //vpBlur->mfSet(false, 0);
      //fpBlur->mfSet(false, 0);
      //r->SelectTMU(3);
      //r->EnableTMU(false);
      //r->SelectTMU(2);
      //r->EnableTMU(false);
      //r->SelectTMU(1);
      //r->EnableTMU(false);
      //r->SelectTMU(0);
      //SAFE_RELEASE(pSrcSurf);
      //r->Set2DMode(false,1,1);
    }
  }
  r->EF_RestoreRenderTarget();

  if (CRenderer::CV_r_envcmwrite)
  {
    for(n=Start; n<End; n++)
    { 
      static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
      char str[64];
      int width = tex_size;
      int height = tex_size;
      byte *pic = new byte [width * height * 4];
      LPDIRECT3DSURFACE9 pSysSurf, pTargetSurf;
      LPDIRECT3DTEXTURE9 pID3DSysTexture;
      D3DLOCKED_RECT d3dlrSys;
      if (!pID3DTargetTextureCM)
        pTargetSurf = (LPDIRECT3DSURFACE9)cm->m_RenderTargets[n];
      else
        h = pID3DTargetTextureCM->GetCubeMapSurface((D3DCUBEMAP_FACES)n, 0, &pTargetSurf);
      h = D3DXCreateTexture(r->m_pd3dDevice, tex_size, tex_size, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
      h = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);
      if (pID3DTargetTexture)
        h = D3DXLoadSurfaceFromSurface(pSysSurf, NULL, NULL, pTargetSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
      else
        h = r->m_pd3dDevice->GetRenderTargetData(pTargetSurf, pSysSurf);
      h = pID3DSysTexture->LockRect(0, &d3dlrSys, NULL, 0);
      byte *src = (byte *)d3dlrSys.pBits;
      for (int i=0; i<width*height; i++)
      {
        *(uint *)&pic[i*4] = *(uint *)&src[i*4];
        Exchange(pic[i*4+0], pic[i*4+2]);
      }
      h = pID3DSysTexture->UnlockRect(0);
      sprintf(str, "Cube_%s.jpg", cubefaces[n]);
      WriteJPG(pic, width, height, str); 
      delete [] pic;
      if (pID3DTargetTextureCM)
        SAFE_RELEASE (pTargetSurf);
      SAFE_RELEASE (pSysSurf);
      SAFE_RELEASE (pID3DSysTexture);
    }
    CRenderer::CV_r_envcmwrite = 0;
  }

  cm->m_Tex->m_Bind = tid;

  gRenDev->SetViewport(vX, vY, vWidth, vHeight);
  ClearBuffer(tex_size,tex_size,true,NULL,0);
  cm->m_bInprogress = false;
  cm->m_MaskReady = End;
  if (cm->m_MaskReady == 6)
  {
    cm->m_MaskReady = 0;
    cm->m_bReady = true;
  }
  SetTexture(0, eTT_Cubemap);

  gRenDev->m_RP.m_bDrawToTexture = false;
  gRenDev->EF_PopFog();
  //gRenDev->EF_RestoreDLights();
}

//////////////////////////////////////////////////////////////////////////
static Matrix44 sMatrixLookAt( const Vec3d &dir,const Vec3d &up,float rollAngle=0 )
{
  Matrix44 M;
  // LookAt transform.
  Vec3d xAxis,yAxis,zAxis;
  Vec3d upVector = up;

  yAxis = GetNormalized(-dir);

  //if (zAxis.x == 0.0 && zAxis.z == 0) up.Set( -zAxis.y,0,0 ); else up.Set( 0,1.0f,0 );

  xAxis = GetNormalized((upVector.Cross(yAxis)));
  zAxis = GetNormalized((xAxis.Cross(yAxis)));

  // OpenGL kind of matrix.
  M[0][0] = xAxis.x;
  M[1][0] = yAxis.x;
  M[2][0] = zAxis.x;
  M[3][0] = 0;

  M[0][1] = xAxis.y;
  M[1][1] = yAxis.y;
  M[2][1] = zAxis.y;
  M[3][1] = 0;

  M[0][2] = xAxis.z;
  M[1][2] = yAxis.z;
  M[2][2] = zAxis.z;
  M[3][2] = 0;

  M[0][3] = 0;
  M[1][3] = 0;
  M[2][3] = 0;
  M[3][3] = 1;

  if (rollAngle != 0)
  {
    Matrix44 RollMtx;
    RollMtx.SetIdentity();

    float cossin[2];
    cry_sincosf(rollAngle*gf_DEGTORAD, cossin);

    RollMtx[0][0] = cossin[0]; RollMtx[2][0] = -cossin[1];
    RollMtx[0][2] = cossin[1]; RollMtx[2][2] = cossin[0];

    // Matrix multiply.
    M = RollMtx * M;
  }

  return M;
}

void CD3D9TexMan::ScanEnvironmentTexture(SEnvTexture *cm, SShader *pSH, SRenderShaderResources *pRes, int RendFlags, bool bUseExistingREs)
{
  static float Smat[16] = 
  {
    0.5f, 0,    0,    0,
    0,    -0.5f, 0,    0,
    0,    0,    0.5f, 0,
    0.5f, 0.5f, 0.5f, 1.0f
  };

  if (cm->m_bInprogress)
    return;

  // increase frame id to support multiple reflections
  gcpRendD3D->m_nFrameID++;

  CD3D9Renderer *r = gcpRendD3D;
  bool bUseClipPlanes = false;
  bool bUseReflection = false;
  if (pSH)
  {
    if (pSH->m_Flags3 & (EF3_CLIPPLANE_BACK | EF3_CLIPPLANE_FRONT))
      bUseClipPlanes = true;
    if (bUseClipPlanes || (pSH->m_Flags3 & EF3_REFLECTION))
      bUseReflection = true;
  }

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMinDist = 0.25f;

  ECull eCull = eCULL_None;
  if (pSH)
    eCull = pSH->m_eCull;
  if (pRes && (pRes->m_ResFlags & MTLFLAG_2SIDED))
    eCull = eCULL_None;

  bool bWater = (pSH && ((pSH->m_nPreprocess & FSPR_SCANTEXWATER) != 0));
  Plane Pl, PlTr;
  float plane[4];
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  if (bUseClipPlanes || bUseReflection)
  {
    if (!bWater)
      r->m_RP.m_pRE->mfGetPlane(Pl);
    else
    {
      Pl.n = Vec3d(0,0,1);
      Pl.d = eng->GetWaterLevel();
    }
    if (r->m_RP.m_pCurObject)
    {
      r->m_RP.m_FrameObject++;
      PlTr = TransformPlane(obj->GetMatrix(), Pl);
    }
    else
      PlTr = Pl;
    if (pSH->m_Flags3 & EF3_CLIPPLANE_BACK)
    {
      PlTr.n = -PlTr.n;
      PlTr.d = -PlTr.d;
    }
    if (eCull != eCULL_None)
    {
      CCamera tmp_camera = r->GetCamera();
      Vec3d pos = tmp_camera.GetPos();
      float dot = pos.Dot(PlTr.n) - PlTr.d;
      if (dot <= 0.1f)
        return;
    }
    plane[0] = PlTr.n[0];
    plane[1] = PlTr.n[1];
    plane[2] = PlTr.n[2];
    plane[3] = -PlTr.d;

    if (!bWater)
      fMinDist = r->m_RP.m_pRE->mfMinDistanceToCamera(obj);
//    fMinDist = 0.25f;
  }

  int tex_size;
  switch (CRenderer::CV_r_envtexresolution)
  {
    case 0:
      tex_size = 64;
      break;
    case 1:
      tex_size = 128;
      break;
    case 2:
    default:
      tex_size = 256;
      break;
    case 3:
      tex_size = 512;
      break;
  }
  
  Vec3d cur_pos;
  
  int nHDRFl = (r->m_RP.m_PersFlags & RBPF_HDR) ? FT_HDR : 0;

  cm->m_Tex->m_Flags |= FT_BUILD;
  cm->m_bInprogress = true;
  int tid = TO_ENVIRONMENT_TEX_MAP_REAL + cm->m_Id;
  if (!(cm->m_Tex->m_Flags & FT_ALLOCATED) || tex_size != cm->m_TexSize || (cm->m_Tex->m_Flags & FT_HDR) != nHDRFl)
  {
    cm->m_Tex->m_Flags |= nHDRFl;
    cm->m_Tex->m_Flags |= FT_ALLOCATED | FT_NOMIPS | FT_CLAMP;
    cm->m_Tex->m_Flags &= ~FT_DXT;
    cm->m_Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
    AddToHash(cm->m_Tex->m_Bind, cm->m_Tex);
    cm->m_TexSize = tex_size;
    cm->m_Tex->m_Width = cm->m_Tex->m_Height = tex_size;
    cm->m_Tex->m_nMips = 0;
    byte *data = new byte [tex_size*tex_size*4];
    r->m_TexMan->CreateTexture(NULL, tex_size, tex_size, 1, FT_NOMIPS | FT_CLAMP, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, cm->m_Tex);
    delete [] data;
  }

  STexPicD3D *tp = (STexPicD3D *)cm->m_Tex;
  if (!tp->m_RefTex.m_VidTex)
    return;

  LPDIRECT3DTEXTURE9 pID3DTargetTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
  LPDIRECT3DSURFACE9 pSrcSurf;
  HRESULT h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  r->EF_SetRenderTarget(pSrcSurf, true);
  
  DWORD cColor = D3DRGBA(gRenDev->m_vClearColor[0], gRenDev->m_vClearColor[1], gRenDev->m_vClearColor[2], 0);
  if (r->m_sbpp)
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);
  else
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);
  
  int vX, vY, vWidth, vHeight;
  r->GetViewport(&vX, &vY, &vWidth, &vHeight);
  r->EF_PushFog();
  //r->EF_SaveDLights();
  r->m_RP.m_bDrawToTexture = true;
  
  float fMaxDist = eng->GetMaxViewDistance();
  int prevFlags = gRenDev->m_RP.m_PersFlags;

  {
    CCamera tmp_camera = r->GetCamera();
    CCamera prevCamera = tmp_camera;

    // camera reflection by plane
    if ((pSH && (pSH->m_Flags3 & EF3_CLIPPLANE_FRONT)) || bUseReflection)
    {
      // mirror case
      Vec3d vPrevPos = tmp_camera.GetPos();
      Matrix44 camMat = tmp_camera.GetVCMatrixD3D9();
      Vec3d vPrevDir = Vec3d(-camMat(0,2), -camMat(1,2), -camMat(2,2));
      Vec3d vPrevUp = Vec3d(camMat(0,1), camMat(1,1), camMat(2,1));
      Vec3d vNewDir = PlTr.MirrorVector(vPrevDir);
      Vec3d vNewUp = PlTr.MirrorVector(vPrevUp);
      Matrix44 m = sMatrixLookAt( vNewDir, vNewUp, tmp_camera.GetAngles()[1] );

      float fDot = vPrevPos.Dot(PlTr.n) - PlTr.d;
      Vec3d vNewPos = vPrevPos - PlTr.n * 2.0f*fDot;
      Vec3d vNewOccPos;// = vPrevPos - PlTr.n * 0.99f*fDot;
      gRenDev->m_RP.m_pRE->mfCenter(vNewOccPos, gRenDev->m_RP.m_pCurObject);
      if (fDot < 0)
      {
        plane[0] = -plane[0];
        plane[1] = -plane[1];
        plane[2] = -plane[2];
        plane[3] = -plane[3];
      }

      //QUAT_CHANGED_BY_IVO
     // CryQuat q(m);
      CryQuat q = Quat( GetTransposed44(m) );

      Vec3d vNewAngs = Ang3::GetAnglesXYZ(Matrix33(q));
      vNewAngs = RAD2DEG(vNewAngs);

      tmp_camera.SetAngle(vNewAngs);
      tmp_camera.SetPos(vNewPos);
      tmp_camera.SetOccPos(vNewOccPos);
      tmp_camera.Init(tex_size, tex_size, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
      tmp_camera.Update();

      iSystem->SetViewCamera(tmp_camera);
      gRenDev->SetCamera(tmp_camera);
    }
    else
    {
      //tmp_camera.Init(tex_size, tex_size, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
      iSystem->SetViewCamera(tmp_camera);
      r->SetCamera(tmp_camera);
    }
    if (bUseClipPlanes || bUseReflection)
    {
      Plane p;
      p.n.Set(plane[0], plane[1], plane[2]);
      p.d = plane[3];
      tmp_camera.SetFrustumPlane(FR_PLANE_NEAR, p);
    }

    if (!cm->m_Tex->m_Matrix)
      cm->m_Tex->m_Matrix = new float[16];

    Matrix44 m, m1;
    m = (Matrix44&)Smat;
    D3DXMATRIXA16 *matProj = (D3DXMATRIXA16 *)gcpRendD3D->m_matProj->GetTop();
    D3DXMATRIXA16 *matView = (D3DXMATRIXA16 *)gcpRendD3D->m_matView->GetTop();
    D3DXMatrixMultiply((D3DXMATRIX *)&m1(0,0), matProj, (D3DXMATRIX *)&m(0,0));
    D3DXMatrixMultiply((D3DXMATRIX *)cm->m_Tex->m_Matrix, matView, (D3DXMATRIX *)&m1(0,0));
    //D3DXMatrixMultiply((D3DXMATRIX *)cm->m_Tex->m_Matrix, (D3DXMATRIX *)&r->m_ObjMatrix.m_values[0][0], (D3DXMATRIX *)&m1.m_values[0][0]);

    if (bUseClipPlanes)
      r->EF_SetClipPlane(true, plane, bWater);

    r->SetViewport(0, 0, tex_size, tex_size);

    r->m_RP.m_PersFlags |= RBPF_NOCLEARBUF;

    if (r->m_LogFile)
      r->Logv(SRendItem::m_RecurseLevel, "*** Begin Draw environment to texture ***\n");

    if (bUseExistingREs)
      gcpRendD3D->EF_RenderPipeLine(CD3D9Renderer::EF_Flush);
    else
    {
      I3DEngine *eng = (I3DEngine *)iSystem->GetIProcess();
      eng->DrawLowDetail(RendFlags);
    }

    if (r->m_LogFile)
      r->Logv(SRendItem::m_RecurseLevel, "*** End Draw environment to texture ***\n");

    r->m_RP.m_PersFlags &= ~RBPF_NOCLEARBUF;

    iSystem->SetViewCamera(prevCamera);
    r->SetCamera(prevCamera);    
  }

  SAFE_RELEASE(pSrcSurf);
  r->EF_RestoreRenderTarget();
  
  if (bUseClipPlanes)
    r->EF_SetClipPlane(false, NULL, false);

  if (r->m_sbpp)
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);
  else
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

  r->m_RP.m_PersFlags &= ~(RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);
  r->m_RP.m_PersFlags |= prevFlags & (RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);

  /*{
    LPDIRECT3DTEXTURE9 pID3DSysTexture;
    D3DLOCKED_RECT d3dlr;
    LPDIRECT3DSURFACE9 pTargetSurf;
    h = pID3DTargetTexture->GetSurfaceLevel(0, &pTargetSurf);
    LPDIRECT3DSURFACE9 pSysSurf;
    h = D3DXCreateTexture(r->m_pd3dDevice, tex_size, tex_size, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
    h = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);
    h = r->m_pd3dDevice->GetRenderTargetData(pTargetSurf, pSysSurf);
    h = pID3DSysTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to the texture 
    WriteTGA((byte *)d3dlr.pBits, tex_size, tex_size, "Problem.tga");
    // Unlock the texture
    pID3DSysTexture->UnlockRect(0);
    pSysSurf->Release();
    pTargetSurf->Release();
    SAFE_RELEASE (pID3DSysTexture);
  }*/
    
  r->SetViewport(vX, vY, vWidth, vHeight);
  r->EF_PopFog();
  //r->EF_RestoreDLights();

  cm->m_bInprogress = false;
  cm->m_bReady = true;
  cm->m_MaskReady = 1;
  r->m_RP.m_bDrawToTexture = false;
}

//========================================================================================

void CD3D9TexMan::StartCubeSide(CCObject *obj)
{
}

void CD3D9TexMan::EndCubeSide(CCObject *obj, bool bNeedClear)
{
}

void CD3D9TexMan::DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags)
{
  static float Smat[16] = 
  {
    0.5f, 0,   0,   0,
    0,   -0.5f, 0,   0,
    0,   0,   0.5f, 0,
    0.5f, 0.5f, 0.5f, 1.0f
  };

  if (Tex->m_Flags & FT_BUILD)
    return;
  CD3D9Renderer *r = gcpRendD3D;
  float plane[4];
  
  plane[0] = Pl.n[0];
  plane[1] = Pl.n[1];
  plane[2] = Pl.n[2];
  plane[3] = -Pl.d;
  
  int nWidth = sLimitSizeByScreenRes(512);
  int nHeight = nWidth;
  if (nWidth != Tex->m_Width || nHeight != Tex->m_Height)
  {
    int nSize = min(nWidth, nHeight);
    Tex->m_Width = nSize;
    Tex->m_Height = nSize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }

//#define WATTEST
  int nHDRFl = (r->m_RP.m_PersFlags & RBPF_HDR) ? FT_HDR : 0;

  Tex->m_Flags |= FT_BUILD;
  if (!(Tex->m_Flags & FT_ALLOCATED) || (Tex->m_Flags & FT_HDR) != nHDRFl)
  {
    Tex->m_Flags |= nHDRFl;
    Tex->m_Flags |= FT_ALLOCATED;
    Tex->m_Flags &= ~FT_DXT;
    // Preallocate texture
    int tex_size = Tex->m_Width;
    byte *data = new byte [tex_size*tex_size*4];
    //memset(data, 0, tex_size*tex_size*4);
#ifndef WATTEST    
    Tex->m_Flags |= FT_NOMIPS | FT_CLAMP;
    Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
#else
    Tex->m_Flags |= FT_NOMIPS | FT_ALLOCATED | FT_CLAMP;
    Tex->m_Flags2 |= FT2_NODXT;
#endif

    Tex->m_nMips = 0;
    AddToHash(Tex->m_Bind, Tex);
#ifndef WATTEST
    r->m_TexMan->CreateTexture(NULL, tex_size, tex_size, 1, FT_NOMIPS | FT_CLAMP, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, Tex);
#else
    STexPic *tp = 
    r->m_TexMan->CreateTexture(NULL, tex_size, tex_size, 1, FT_NOMIPS | FT_ALLOCATED, FT2_NODXT, data, eTT_Base, -1.0f, -1.0f, 0, Tex);

    IDirect3DSurface9 * pTar = r->mfGetBackSurface();
    D3DSURFACE_DESC dc;
    pTar->GetDesc(&dc);
    if(tp->m_RefTex.m_VidTex)
      ((LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex)->Release();
    HRESULT h = D3DXCreateTexture(((CD3D9Renderer *)gRenDev)->mfGetD3DDevice(), tex_size, tex_size, 1, D3DUSAGE_DYNAMIC, dc.Format, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE9* )& tp->m_RefTex.m_VidTex ));
#endif
    delete [] data;
  }
  if (!Tex->m_RefTex.m_VidTex)
    return;

  //gRenDev->EF_SaveDLights();

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  float fMinDist = min(SKY_BOX_SIZE*0.5f, eng->GetDistanceToSectorWithWater()); // 16 is half of skybox size
  float fMaxDist = eng->GetMaxViewDistance();

  CCamera tmp_cam = r->GetCamera();
  CCamera prevCamera = tmp_cam;

  Vec3d vPrevPos = tmp_cam.GetPos();
  Vec3d vPrevAngles = tmp_cam.GetAngles();
  Matrix44 camMat = tmp_cam.GetVCMatrixD3D9();
  Vec3d vPrevDir = Vec3d(-camMat(0,2), -camMat(1,2), -camMat(2,2));
  Vec3d vPrevUp = Vec3d(camMat(0,1), camMat(1,1), camMat(2,1));
  Vec3d vNewDir = Pl.MirrorVector(vPrevDir);
  Vec3d vNewUp = Pl.MirrorVector(vPrevUp);
  Matrix44 mMir = sMatrixLookAt( vNewDir, vNewUp, vPrevAngles[1] );

  float fDot = vPrevPos.Dot(Pl.n) - Pl.d;
  Vec3d vNewPos = vPrevPos - Pl.n * 2.0f*fDot;
  Vec3d vOccPos = vPrevPos - Pl.n * 0.99f*fDot;

  CryQuat q = Quat( GetTransposed44(mMir) );

  Vec3d vNewAngs = Ang3::GetAnglesXYZ(Matrix33(q));
  vNewAngs = RAD2DEG(vNewAngs);

  // increase frame id to support multiple recursive draws
  r->m_nFrameID++;

  //vNewAngs[0] = -vPrevAngles[0];
  tmp_cam.SetAngle(vNewAngs);
  tmp_cam.SetPos(vNewPos);
  tmp_cam.SetOccPos(vOccPos);
  tmp_cam.Init(Tex->m_Width, Tex->m_Height, tmp_cam.GetFov(), fMaxDist, 1.0f, fMinDist);
  tmp_cam.Update();
  
#ifndef WATTEST
  STexPicD3D *tp = (STexPicD3D *)Tex;
  LPDIRECT3DTEXTURE9 pID3DTargetTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
  LPDIRECT3DSURFACE9 pSrcSurf;
  HRESULT h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  h = r->EF_SetRenderTarget(pSrcSurf, true);
#endif
    
  int TempX, TempY, TempWidth, TempHeight;
  r->GetViewport(&TempX, &TempY, &TempWidth, &TempHeight);
  r->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );
  r->EF_PushFog();
  r->m_RP.m_bDrawToTexture = true;

  Vec3d vCol = r->m_vClearColor;
  DWORD cColor = D3DRGBA(r->m_vClearColor[0], r->m_vClearColor[1], r->m_vClearColor[2], 0);
  if (r->m_sbpp)
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);
  else
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

  eng->SetCamera(tmp_cam,false);
  
  if (!Tex->m_Matrix)
    Tex->m_Matrix = new float[16];

  float matProj[16], matView[16];
  r->GetModelViewMatrix(matView);
  r->GetProjectionMatrix(matProj);
  Matrix44 m;
  D3DXMatrixMultiply((D3DXMATRIX *)&m(0,0), (D3DXMATRIX *)matProj, (D3DXMATRIX *)Smat);
  D3DXMatrixMultiply((D3DXMATRIX *)Tex->m_Matrix, (D3DXMATRIX *)matView, (D3DXMATRIX *)&m(0,0));

  r->m_RP.m_PersFlags |= RBPF_NOCLEARBUF;

  r->EF_SetClipPlane(true, plane, false);
  
  /*static int sRFlags;
  sRFlags |= RendFlags & ~DLD_TERRAIN_FULLRES;
  //RendFlags &= ~DLD_TERRAIN_FULLRES;
  if ((GetAsyncKeyState('F') & 0x8000))
    sRFlags |= DLD_TERRAIN_FULLRES;
  if ((GetAsyncKeyState('U') & 0x8000))
    sRFlags &= ~DLD_TERRAIN_FULLRES;*/
  eng->DrawLowDetail(RendFlags);

  /*{
    int tex_size = Tex->m_Width;
    LPDIRECT3DTEXTURE9 pID3DSysTexture;
    D3DLOCKED_RECT d3dlr;
    LPDIRECT3DSURFACE9 pTargetSurf;
    h = pID3DTargetTexture->GetSurfaceLevel(0, &pTargetSurf);
    LPDIRECT3DSURFACE9 pSysSurf;
    h = D3DXCreateTexture(r->m_pd3dDevice, tex_size, tex_size, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
    h = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);
    h = r->m_pd3dDevice->GetRenderTargetData(pTargetSurf, pSysSurf);
    h = pID3DSysTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to the texture 
    ::WriteJPG((byte *)d3dlr.pBits, tex_size, tex_size, "Problem.jpg");
    // Unlock the texture
    pID3DSysTexture->UnlockRect(0);
    pSysSurf->Release();
    pTargetSurf->Release();
    SAFE_RELEASE (pID3DSysTexture);
  }*/

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log)
    r->Logv(SRendItem::m_RecurseLevel, ".. DrawLowDetail .. (End DrawToTexture)\n");
#endif

  vCol = gRenDev->m_vClearColor;

  r->m_RP.m_PersFlags &= ~RBPF_NOCLEARBUF;
  r->EF_SetClipPlane(false, plane, false);
  Tex->m_Flags &= ~FT_BUILD;

#ifndef WATTEST
  h = r->EF_RestoreRenderTarget();
  SAFE_RELEASE(pSrcSurf);
#else

  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0f, 0);

  STexPicD3D *tp = (STexPicD3D *)Tex;
  LPDIRECT3DTEXTURE9 pID3DTargetTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
  LPDIRECT3DSURFACE9 pSrcSurf;
  h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  
  IDirect3DSurface9 * pTar = r->mfGetBackSurface();
  POINT p = {0,0};
  r->m_pd3dDevice->UpdateSurface(pTar, 0, pSrcSurf, 0);
  pSrcSurf->Release();
#endif
    
  r->SetCamera(prevCamera);
  iSystem->SetViewCamera(prevCamera);

  r->SetViewport(TempX, TempY, TempWidth, TempHeight);
  r->m_RP.m_bDrawToTexture = false;
  r->EF_PopFog();
  //gRenDev->EF_RestoreDLights();
}

void CD3D9TexMan::DrawToTextureForRainMap(int Id)
{
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  STexPic *Tex = gRenDev->m_TexMan->m_Text_RainMap;
  if (Tex->m_Flags & FT_BUILD)
    return;

  CD3D9Renderer *r = gcpRendD3D;

  if (CRenderer::CV_r_rainmapsize != gRenDev->m_RP.m_RainMapSize)
  {
    if (CRenderer::CV_r_rainmapsize <= 64)
      CRenderer::CV_r_rainmapsize = 64;
    else
    if (CRenderer::CV_r_rainmapsize <= 128)
      CRenderer::CV_r_rainmapsize = 128;
    else
    if (CRenderer::CV_r_rainmapsize <= 256)
      CRenderer::CV_r_rainmapsize = 256;
    else
    if (CRenderer::CV_r_rainmapsize <= 512)
      CRenderer::CV_r_rainmapsize = 512;

    gRenDev->m_RP.m_RainMapSize = CRenderer::CV_r_rainmapsize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }
  int nTexWidth = gRenDev->m_RP.m_RainMapSize;
  int nTexHeight = gRenDev->m_RP.m_RainMapSize;

  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  Tex->m_Flags |= FT_BUILD;

  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Width = nTexWidth;
    Tex->m_Height = nTexHeight;
    Tex->m_Flags |= FT_ALLOCATED;
    Tex->m_Flags &= ~FT_DXT;
    // Preallocate texture
    byte *data = new byte [Tex->m_Width*Tex->m_Height*4];
    Tex->m_Flags |= FT_NOMIPS|FT_HASALPHA;
    Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
    AddToHash(Tex->m_Bind, Tex);
    gRenDev->m_TexMan->CreateTexture(NULL, Tex->m_Width, Tex->m_Height, 1, FT_NOMIPS|FT_HASALPHA, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, Tex); 
    delete [] data;
  }

  //gRenDev->EF_SaveDLights();

  CCamera tmp_cam = gRenDev->GetCamera();
  CCamera prevCamera = tmp_cam;
  //tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  //tmp_cam.Update();  

  int TempX, TempY, TempWidth, TempHeight;
  gRenDev->GetViewport(&TempX, &TempY, &TempWidth, &TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;

  eng->SetCamera(tmp_cam,false);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Draw scene to texture for rainmap ***\n");

  STexPicD3D *tp = (STexPicD3D *)Tex;
  LPDIRECT3DTEXTURE9 pID3DTargetTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
  LPDIRECT3DSURFACE9 pSrcSurf;
  HRESULT h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  h = r->EF_SetRenderTarget(pSrcSurf, true);

  DWORD cColor = D3DRGBA(gRenDev->m_vClearColor[0], gRenDev->m_vClearColor[1], gRenDev->m_vClearColor[2], 0);
  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

  r->m_RP.m_PersFlags |= RBPF_NOCLEARBUF;

  eng->DrawRain();

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. End DrawLowDetail .. (DrawToTextureForRainMap)\n");

  r->m_RP.m_PersFlags &= ~RBPF_NOCLEARBUF;
  Tex->m_Flags &= ~FT_BUILD;

  SAFE_RELEASE(pSrcSurf);
  h = r->EF_RestoreRenderTarget();

  /*{
    int tex_size = Tex->m_Width;
    LPDIRECT3DTEXTURE9 pID3DSysTexture;
    D3DLOCKED_RECT d3dlr;
    LPDIRECT3DSURFACE9 pTargetSurf;
    h = pID3DTargetTexture->GetSurfaceLevel(0, &pTargetSurf);
    LPDIRECT3DSURFACE9 pSysSurf;
    h = D3DXCreateTexture(r->m_pd3dDevice, tex_size, tex_size, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSysTexture );
    h = pID3DSysTexture->GetSurfaceLevel(0, &pSysSurf);
    h = r->m_pd3dDevice->GetRenderTargetData(pTargetSurf, pSysSurf);
    h = pID3DSysTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to the texture 
    WriteTGA((byte *)d3dlr.pBits, tex_size, tex_size, "Problem.tga", 32);
    // Unlock the texture
    pID3DSysTexture->UnlockRect(0);
    pSysSurf->Release();
    pTargetSurf->Release();
    SAFE_RELEASE (pID3DSysTexture);
  }*/

  iSystem->SetViewCamera(prevCamera);
  gRenDev->SetCamera(prevCamera);

  gRenDev->SetViewport(TempX, TempY, TempWidth, TempHeight);

  Tex->m_Flags &= ~FT_BUILD;
  gRenDev->m_RP.m_bDrawToTexture = false;

  gRenDev->EF_PopFog();
  //gRenDev->EF_RestoreDLights();
}

//==================================================================================
// Heat map

_inline void D3DQuad(float wdt, float hgt, CFColor fcol)
{
  DWORD col = fcol.GetTrue();

  CD3D9Renderer *r = gcpRendD3D;
  int nOffs;
  struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *vQuad = gcpRendD3D->GetVBPtr2D(4, nOffs);

  // Define the quad
  vQuad[0].x = 0.0f;
  vQuad[0].y = 0.0f;
  vQuad[0].z = 0.0f;
  vQuad[0].rhw = 1.0f;
  vQuad[0].color.dcolor = col;
  vQuad[0].st[0] = 0.0f;
  vQuad[0].st[1] = 0.0f;

  vQuad[1].x = wdt;
  vQuad[1].y = 0.0f;
  vQuad[1].z = 0.0f;
  vQuad[1].rhw = 1.0f;
  vQuad[1].color.dcolor = col;
  vQuad[1].st[0] = 1.0f;
  vQuad[1].st[1] = 0.0f;

  vQuad[2].x = wdt;
  vQuad[2].y = hgt;
  vQuad[2].z = 0.0f;
  vQuad[2].rhw = 1.0f;
  vQuad[2].color.dcolor = col;
  vQuad[2].st[0] = 1.0f;
  vQuad[2].st[1] = 1.0f;

  vQuad[3].x = 0.0f;
  vQuad[3].y = hgt;
  vQuad[3].z = 0.0f;
  vQuad[3].rhw = 1.0f;
  vQuad[3].color.dcolor = col;
  vQuad[3].st[0] = 0.0f;
  vQuad[3].st[1] = 1.0f;

  // We are finished with accessing the vertex buffer
  r->UnlockVB2D();
  // Bind our vertex as the first data stream of our device
  r->m_pd3dDevice->SetStreamSource(0, r->m_pVB2D, 0, sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F));
  r->EF_SetVertexDeclaration(0, VERTEX_FORMAT_TRP3F_COL4UB_TEX2F);
  // Render the two triangles from the data stream
  HRESULT hr = r->m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);
}

// <<TODO>> Clean redundant code

void CD3D9TexMan::EndHeatMap()
{
  gRenDev->m_RP.m_bDrawToTexture = false;
}

void CD3D9TexMan::StartHeatMap(int Id)
{
}

void CD3D9TexMan::EndNightMap()
{
  gRenDev->m_RP.m_bDrawToTexture = false;
}

void CD3D9TexMan::StartNightMap(int Id)
{
}


void CD3D9TexMan::StartScreenMap(int Id)
{
}

void CD3D9TexMan::EndScreenMap()
{
}

void CD3D9TexMan::StartRefractMap(int Id)
{
}
void CD3D9TexMan::EndRefractMap()
{
}

// ===============================================================
// DrawToTextureForGlare - glare fx
// Last Update: 28/06/2003

void CD3D9TexMan::DrawToTextureForGlare(int Id)
{

  gRenDev->m_RP.m_bDrawToTexture = false; 
}

// ===============================================================
// DrawFlashBangMap - flashbang fx's
// Last Update: 04/06/2003

void CD3D9TexMan::DrawFlashBangMap(int Id, int RendFlags, CREFlashBang *pRE)
{
  // set flags  
  gRenDev->m_RP.m_bDrawToTexture = false; 
}

// ===============================================================
// ScreenTexMap - get screen sized texture, for shared use
// Last Update: 19/09/2003

// prepare and set rendertarget
void CD3D9TexMan::StartScreenTexMap(int Id)
{
  if(gRenDev)
  {
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** StartScreenTexMap ***\n");
  }

  // get data  
  STexPic *pTex =GetByID(Id); 
  // this SHOULD be always valid, but ok...
  assert(pTex);

  CD3D9Renderer *pRenderer = gcpRendD3D;
  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  int iWidth = m_TempWidth, iHeight = m_TempHeight;

  // recreate texture when necessary
  if(pTex->m_Width!=iWidth || pTex->m_Height!=iHeight)
  {
    pTex->m_Flags &= ~FT_ALLOCATED;
  }

  // if not created yet, create texture
  if (!(pTex->m_Flags & FT_ALLOCATED))
  {
    // set rendertarget flags
    pTex->m_Flags |= FT_ALLOCATED | FT_NOMIPS | FT_HASALPHA;
    pTex->m_Flags &= ~FT_DXT;
    pTex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;

    pTex->m_Width = iWidth;
    pTex->m_Height = iHeight;
    pTex->m_nMips = 0;

    // must pass empty buffer into create texture..
    byte *pData = new byte [pTex->m_Width*pTex->m_Height*4];

    if(!pRenderer->m_TexMan->CreateTexture(NULL, pTex->m_Width, pTex->m_Height, 1, pTex->m_Flags, pTex->m_Flags2, pData, eTT_Base, -1.0f, -1.0f, 0, pTex))
    {
      // error creating texure
      delete [] pData;
      return;
    }

    delete [] pData;
  }

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);

  // set flags
  gRenDev->m_RP.m_bDrawToTexture = true;
  gRenDev->m_RP.m_PersFlags |= RBPF_DRAWSCREENTEXMAP; //|RBPF_NOCLEARBUF;  

  D3DCOLOR cColor = D3DRGBA(0.0f,0.0f,0.0f,0.0f);
  // render object
  //pRenderer->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, cColor, 1.0f, 0); 
}

// restore back-buffer
void CD3D9TexMan::EndScreenTexMap()
{
  if(gRenDev)
  {
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** EndScreenTexMap ***\n");
  }

  // get data
  STexPic *tx = NULL;
  if (gRenDev->m_RP.m_PersFlags & RBPF_HDR)
    tx = gRenDev->m_TexMan->m_Text_ScreenMap_HDR; 
  else
    tx = gRenDev->m_TexMan->m_Text_ScreenMap; 

  // this SHOULD be always valid, but ok...
  assert(tx);

  CD3D9Renderer *pRenderer = gcpRendD3D; 

  int iTempX, iTempY, iWidth, iHeight;
  gRenDev->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

  // get texture surface
  LPDIRECT3DSURFACE9 pTexSurfCopy;
  // this SHOULD be always valid, but ok...
  assert(tx->m_RefTex.m_VidTex);
  LPDIRECT3DTEXTURE9 plD3DTextureCopy = (LPDIRECT3DTEXTURE9) tx->m_RefTex.m_VidTex;    
  HRESULT hr = plD3DTextureCopy->GetSurfaceLevel(0, &pTexSurfCopy);

  if (FAILED(hr))
    return;

  // Beware with e_widescreen usage. Only copy visible screen area.
  RECT pSrcRect={iTempX, iTempY, iTempX+iWidth, iTempY+iHeight };
  RECT pDstRect={0, 0, iWidth, iHeight };

  if (pRenderer->m_RP.m_PersFlags & RBPF_HDR)
  {
    hr = pRenderer->m_pd3dDevice->StretchRect(pRenderer->m_pHDRTargetSurf, &pSrcRect, pTexSurfCopy, &pDstRect, D3DTEXF_NONE);   
  }
  else
  {
    hr = pRenderer->m_pd3dDevice->StretchRect(pRenderer->m_pCurBackBuffer, &pSrcRect, pTexSurfCopy, &pDstRect, D3DTEXF_NONE);   
  }
   
  // reset flags/free data
  gRenDev->m_RP.m_PersFlags &= ~(RBPF_DRAWSCREENTEXMAP); //|RBPF_NOCLEARBUF); 
  gRenDev->m_RP.m_bDrawToTexture = false;
  SAFE_RELEASE(pTexSurfCopy)  
}

// =====================================================================
// DrawToTextureForDof - generate depth of field, focal distance texture
// Last Update: 18/03/2004

void CD3D9TexMan::DrawToTextureForDof(int Id)
{
}  

//==================================================================================

int __cdecl TexCallback( const VOID* arg1, const VOID* arg2 )
{
  STexPic **pi1 = (STexPic **)arg1;
  STexPic **pi2 = (STexPic **)arg2;
  STexPic *ti1 = *pi1;
  STexPic *ti2 = *pi2;
  if (ti1->m_Size > ti2->m_Size)
    return -1;
  if (ti1->m_Size < ti2->m_Size)
    return 1;
  return 0;
}



void CD3D9TexMan::Update()
{
  CD3D9Renderer *rd = gcpRendD3D;
  int i;
  char buf[256]="";

  CheckTexLimits(NULL);

  bool bChangedNormalMapCompressed = false;
#ifdef USE_3DC
  if (rd->m_bDeviceSupportsComprNormalmaps && CRenderer::CV_r_texnormalmapcompressed != m_CurTexNormalMapCompressed)
    bChangedNormalMapCompressed = true;
#endif

  if (bChangedNormalMapCompressed || CRenderer::CV_r_texresolution != m_CurTexResolution || CRenderer::CV_r_texbumpresolution != m_CurTexBumpResolution || CRenderer::CV_r_texquality != m_CurTexQuality || CRenderer::CV_r_texbumpquality != m_CurTexBumpQuality)
  {
    for (i=0; i<m_Textures.Num(); i++)
    {
      STexPic *tp = m_Textures[i];
      if (!tp || !tp->m_bBusy)
        continue;
      if (!(tp->m_Flags2 & FT2_WASLOADED))
        continue;
      if (tp->m_Flags & FT_NOREMOVE)
        continue;
      if (bChangedNormalMapCompressed && !(tp->m_Flags & FT_ALLOW3DC))
        continue;
      if (tp->m_eTT == eTT_Cubemap)
      {
        //if (tp->m_CubeSide > 0)
          continue;
        if (CRenderer::CV_r_texresolution != m_CurTexResolution || CRenderer::CV_r_texquality != m_CurTexQuality)
        {
          gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
        }
      }
      else
      if (tp->m_eTT == eTT_Bumpmap || tp->m_eTT == eTT_DSDTBump)
      {
        int nFlags = tp->m_Flags;
        bool bReload = false;
#ifdef USE_3DC
        if (bChangedNormalMapCompressed && (nFlags & FT_ALLOW3DC))
        {
          bReload = true;
          if (CRenderer::CV_r_texnormalmapcompressed)
            nFlags |= FT_3DC;
          else
            nFlags &= ~FT_3DC;
        }
#endif
        if (bReload || CRenderer::CV_r_texbumpresolution != m_CurTexBumpResolution || CRenderer::CV_r_texbumpquality != m_CurTexBumpQuality )
        {
          gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), nFlags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
        }
      }
      else
      {
        if (CRenderer::CV_r_texresolution != m_CurTexResolution || CRenderer::CV_r_texquality != m_CurTexQuality)
        {
          gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
        }
      }
    }
    m_CurTexResolution = CRenderer::CV_r_texresolution;
    m_CurTexBumpResolution = CRenderer::CV_r_texbumpresolution;
    m_CurTexQuality = CRenderer::CV_r_texquality;
    m_CurTexBumpQuality = CRenderer::CV_r_texbumpquality;
#ifdef USE_3DC
    m_CurTexNormalMapCompressed = CRenderer::CV_r_texnormalmapcompressed;
#endif
  }

  if (CRenderer::CV_r_logusedtextures == 1 || CRenderer::CV_r_logusedtextures == 3 || CRenderer::CV_r_logusedtextures == 4)
  {
    FILE *fp = NULL;
    TArray<STexPic *> Texs;
    int Size = 0;
    int PartSize = 0;

    static char *sTexType[] = 
    {
      "Base","Cubemap","AutoCubemap","Bump","DSDTBump","Rectangle","3D"
    };
    static char *sTexFormat[] = 
    {
      "Unknown","Index8","HSV","0888","8888","RGBA","8000","0565","0555","4444","1555","DXT1","DXT3","DXT5","SIGNED_HILO16","SIGNED_HILO8","SIGNED_RGB8","RGB8","DSDT_MAG","DSDT","V8U8","V16U16","0088","DEPTH"
    };

    if (CRenderer::CV_r_logusedtextures == 1 || CRenderer::CV_r_logusedtextures == 3)
    {
      for (i=0; i<m_Textures.Num(); i++)
      {
        if (CRenderer::CV_r_logusedtextures == 3 && m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind == gRenDev->m_TexMan->m_Text_NoTexture->m_Bind)
        {
          if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
            Texs.AddElem(m_Textures[i]);
        }
        else
        if (CRenderer::CV_r_logusedtextures == 1 && m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && (m_Textures[i]->m_Flags2 & FT2_WASLOADED) && !(m_Textures[i]->m_Flags2 & FT2_WASUNLOADED))
        {
          if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
            Texs.AddElem(m_Textures[i]);
        }
      }
      if (CRenderer::CV_r_logusedtextures == 3)
        fp = fxopen("MissingTextures.txt", "w");
      else
        fp = fxopen("UsedTextures.txt", "w");
      fprintf(fp, "*** All textures: ***\n");
      qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
      for (i=0; i<Texs.Num(); i++)
      {
        fprintf(fp, "%d\t\tType: %s\t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size, sTexType[Texs[i]->m_eTT], (Texs[i]->m_Flags & FT_3DC) ? "3DC" : sTexFormat[Texs[i]->m_ETF], *Texs[i]->m_SearchName);
        Size += Texs[i]->m_Size;
        PartSize += Texs[i]->m_LoadedSize;
      }
      fprintf(fp, "*** Total Size: %d\n\n", Size, PartSize, PartSize);

      Texs.Free();
    }
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && m_Textures[i]->m_AccessFrame == rd->GetFrameID())
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
          Texs.AddElem(m_Textures[i]);
      }
    }
    if (fp)
      fprintf(fp, "\n\n*** Textures used in current frame: ***\n");
    else
      rd->TextToScreenColor(4,13, 1,1,0,1, "*** Textures used in current frame: ***");
    int nY = 17;
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    Size = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      if (fp)
        fprintf(fp, "%.3fKb\t\tType: %s\t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], (Texs[i]->m_Flags & FT_3DC) ? "3DC" : sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
      else
      {
        sprintf(buf, "%.3fKb  Type: %s  Format: %s  (%s)", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], (Texs[i]->m_Flags & FT_3DC) ? "3DC" : sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
        rd->TextToScreenColor(4,nY, 0,1,0,1, buf);
        nY += 3;
      }
      Size += Texs[i]->m_Size;
    }
    if (fp)
    {
      fprintf(fp, "*** Total Size: %.3fMb\n\n", Size/(1024.0f*1024.0f));
    }
    else
    {
      sprintf(buf, "*** Total Size: %.3fMb", Size/(1024.0f*1024.0f));
      rd->TextToScreenColor(4,nY+1, 0,1,1,1, buf);
    }



    Texs.Free();
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && !(m_Textures[i]->m_Flags2 & FT2_WASUNLOADED))
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
          Texs.AddElem(m_Textures[i]);
      }
    }
    if (fp)
      fprintf(fp, "\n\n*** Textures loaded: ***\n");
    else
      rd->TextToScreenColor(4,13, 1,1,0,1, "*** Textures loaded: ***");
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    Size = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      if (strstr(Texs[i]->m_SourceName.c_str(), "efault"))
      {
        int nnn = 0;
      }
      if (Texs[i]->m_Flags2 & FT2_WASUNLOADED)
        continue;
      if (fp)
        fprintf(fp, "%.3fKb %d mips (%.3fKb)\t\tType: %s \t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size/1024.0f, Texs[i]->m_nMips, Texs[i]->m_LoadedSize/1024.0f, sTexType[Texs[i]->m_eTT], (Texs[i]->m_Flags & FT_3DC) ? "3DC" : sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
      else
      {
        sprintf(buf, "%.3fKb  Type: %s  Format: %s  (%s)", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], (Texs[i]->m_Flags & FT_3DC) ? "3DC" : sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
        rd->TextToScreenColor(4,nY, 0,1,0,1, buf);
        nY += 3;
      }
      if (Texs[i]->m_LoadedSize)
        Size += Texs[i]->m_LoadedSize;
      else
        Size += Texs[i]->m_Size;
    }
    if (fp)
    {
      fprintf(fp, "*** Total Size: %.3fMb\n\n", Size/(1024.0f*1024.0f));
    }
    else
    {
      sprintf(buf, "*** Total Size: %.3fMb", Size/(1024.0f*1024.0f));
      rd->TextToScreenColor(4,nY+1, 0,1,1,1, buf);
    }


    Texs.Free();
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && !m_Textures[i]->IsStreamed())
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
          Texs.AddElem(m_Textures[i]);
      }
    }
    if (fp)
      fprintf(fp, "\n\n*** Textures non-streamed: ***\n");
    else
      rd->TextToScreenColor(4,13, 1,1,0,1, "*** Textures non-streamed: ***");
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    Size = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      if (fp)
        fprintf(fp, "%.3fKb\t\tType: %s\t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], (Texs[i]->m_Flags & FT_3DC) ? "3DC" : sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
      else
      {
        sprintf(buf, "%.3fKb  Type: %s  Format: %s  (%s)", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], (Texs[i]->m_Flags & FT_3DC) ? "3DC" : sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
        rd->TextToScreenColor(4,nY, 0,1,0,1, buf);
        nY += 3;
      }
      Size += Texs[i]->m_Size;
    }
    if (fp)
    {
      fprintf(fp, "*** Total Size: %.3fMb\n\n", Size/(1024.0f*1024.0f));
      fclose (fp);
    }
    else
    {
      sprintf(buf, "*** Total Size: %.3fMb", Size/(1024.0f*1024.0f));
      rd->TextToScreenColor(4,nY+1, 0,1,1,1, buf);
    }

    if (CRenderer::CV_r_logusedtextures != 4)
      CRenderer::CV_r_logusedtextures = 0;
  }
  else
  if (CRenderer::CV_r_logusedtextures == 2)
  {
    //char *str = GetTexturesStatusText();

    TArray<STexPic *> Texs;
    TArray<STexPic *> TexsNM;
    int i;
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind)
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
        {
          /*for (int nn = 0; nn<sTestStr.Num(); nn++)
          {
          if (sTestStr[nn] == m_Textures[i])
          break;
          }
          if (nn == sTestStr.Num())
          {
          int nnn = 0;
          }*/
          Texs.AddElem(m_Textures[i]);
          if (m_Textures[i]->m_eTT == eTT_Bumpmap)
            TexsNM.AddElem(m_Textures[i]);
        }
      }
    }
    int nNOTO = 0;
    for (i=0; i<TX_FIRSTBIND; i++)
    {
      STexPic *tp = GetByID(i);
      if (!tp)
      {
        nNOTO++;
      }
    }
    /*for (int nn = 0; nn<sTestTx.Num(); nn++)
    {
    for (i=0; i<Texs.Num(); i++)
    {
    if (sTestTx[nn] == Texs[i])
    break;
    }
    if (i == Texs.Num())
    {
    int nnn = 0;
    }
    }*/
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    int AllSize = 0;
    int AllSizeNM = 0;
    int Size = 0;
    int PartSize = 0;
    int NonStrSize = 0;
    int SizeNM = 0;
    int PartSizeNM = 0;
    int nLoaded = 0;
    int nNoStr = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      AllSize += Texs[i]->m_Size;
      if (!Texs[i]->IsStreamed())
      {
        NonStrSize += Texs[i]->m_Size;
        nNoStr++;
      }
      if (!(Texs[i]->m_Flags2 & FT2_WASUNLOADED))
      {
        nLoaded++;
        Size += Texs[i]->m_Size;
        if (Texs[i]->m_LoadedSize)
          PartSize += Texs[i]->m_LoadedSize;
        else
          PartSize += Texs[i]->m_Size;
      }
    }
    for (i=0; i<TexsNM.Num(); i++)
    {
      AllSizeNM += TexsNM[i]->m_Size;
      if (!(Texs[i]->m_Flags2 & FT2_WASUNLOADED))
      {
        SizeNM += TexsNM[i]->m_Size;
        if (TexsNM[i]->m_LoadedSize)
          PartSizeNM += TexsNM[i]->m_LoadedSize;
        else
          PartSizeNM += TexsNM[i]->m_Size;
      }
    }
    sprintf(buf, "All texture objects: %d (Size: %.3fMb), NonStreamed: %d (Size: %.3fMb)", Texs.Num(), AllSize/(1024.0f*1024.0f), nNoStr, NonStrSize/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,13, 1,1,0,1, buf);
    sprintf(buf, "All loaded texture objects: %d (All MIPS: %.3fMb, Loaded MIPS: %.3fMb)", nLoaded, Size/(1024.0f*1024.0f), PartSize/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,16, 1,1,0,1, buf);
    sprintf(buf, "All Normal Maps: %d (FullSize: %.3fMb, All Loaded Size: %.3fMb, Loaded MIPS: %.3fMb)", TexsNM.Num(), AllSizeNM/(1024.0f*1024.0f), SizeNM/(1024.0f*1024.0f), PartSizeNM/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,19, 1,1,0,1, buf);

    Texs.Free();
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && m_Textures[i]->m_AccessFrame == rd->m_nFrameUpdateID)
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
          Texs.AddElem(m_Textures[i]);
      }
    }
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    Size = 0;
    PartSize = 0;
    NonStrSize = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      Size += Texs[i]->m_Size;
      if (Texs[i]->m_LoadedSize)
        PartSize += Texs[i]->m_LoadedSize;
      else
        PartSize += Texs[i]->m_Size;
      if (!Texs[i]->IsStreamed())
        NonStrSize += Texs[i]->m_Size;
    }
    sprintf(buf, "Current tex. objects: %d (Size: %.3fMb, Loaded: %.3f, NonStreamed: %.3f)", Texs.Num(), Size/(1024.0f*1024.0f), PartSize/(1024.0f*1024.0f), NonStrSize/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,24, 1,0,0,1, buf);
  }
}

//==================================================================================

STexPic *CD3D9Renderer::EF_MakePhongTexture(int Exp)
{
  char name[128];

  sprintf(name, "$Phong_%d", Exp);
  STexPic *ti = m_TexMan->LoadTexture(name, 0, 0, eTT_Base);
  if (ti->m_Flags & FT_ALLOCATED)
    return ti;
  ti->m_Flags |= FT_ALLOCATED;
  ti->Set();

  float shininess = (float)Exp;
  int imgsize = 256;
  unsigned char * img = new unsigned char[imgsize*imgsize*4];
  unsigned char * ip = img;
  for(int j=0; j<imgsize; j++)
  {
    unsigned char a = (unsigned char)(255.99 * pow(j/(imgsize-1.0), (double)shininess));
    for(int i=0; i<imgsize; i++)
    {
      byte b = (unsigned char)(255.99 * (i/(imgsize-1.0)));
      *ip++ = b;
      *ip++ = b;
      *ip++ = b;
      *ip++ = a;
    }
  }
  LPDIRECT3DTEXTURE9 pPhongTexture;
  if(SUCCEEDED(D3DXCreateTexture(gcpRendD3D->mfGetD3DDevice(), imgsize, imgsize, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pPhongTexture)))
  {
    D3DLOCKED_RECT d3dlr;
    
    // Lock the texture to copy the image data into the texture
    HRESULT hr = pPhongTexture->LockRect(0, &d3dlr, NULL, 0);
    if (SUCCEEDED(hr))
    {
      cryMemcpy((byte *)d3dlr.pBits, img, imgsize*imgsize*4);

      // Unlock the texture
      pPhongTexture->UnlockRect(0);
      hr = D3DXFilterTexture(pPhongTexture, NULL, 0, D3DX_FILTER_LINEAR);
    }
  }
  //::WriteTGA(img, imgsize, imgsize, "Phong.tga", 32); 
  ti->m_RefTex.m_VidTex = (void *)pPhongTexture;
  ti->m_RefTex.m_MinFilter = m_TexMan->GetMinFilter();
  ti->m_RefTex.m_MagFilter = m_TexMan->GetMagFilter();
  ti->m_RefTex.bRepeats = false;
  ti->m_RefTex.m_AnisLevel = gcpRendD3D->GetAnisotropicLevel();
    
  delete [] img;

  m_TexMan->SetTexture(0, eTT_Base);

  return ti;
}

static float sfExp;

static float spec_func(float s, float NaH, float NaNa)
{
  float toksvig = sqrt(NaNa)/(sqrt(NaNa)+s*(1-sqrt(NaNa)));
  return (1.0f+toksvig*s)/(1.0f+s)*pow(NaH/sqrt(NaNa), toksvig*s);
}

VOID WINAPI FillSpecularTexture(D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize,	LPVOID pData)
{
	float f  = spec_func(sfExp,pTexCoord->x,pTexCoord->y);
	pOut->x = f;
	pOut->y = f;
	pOut->z = f;
	pOut->w = 0;
}

STexPic *CD3D9Renderer::EF_MakeSpecularTexture(float fExp)
{
  char name[128];

  bool bMips = false;
  sprintf(name, "$Specular_%d", (int)fExp);
  int nFlags = FT_NOREMOVE | FT_NOSTREAM | FT_CLAMP;
  CD3D9TexMan *tm = (CD3D9TexMan *)m_TexMan;
  STexPicD3D *ti = (STexPicD3D *)tm->LoadTexture(name, nFlags, FT2_NODXT | FT2_NOANISO, eTT_Base, -1.0f, -1.0f);
  if (ti->m_RefTex.m_VidTex)
    return ti;
  LPDIRECT3DTEXTURE9 pTexture;
  D3DFORMAT d3dFMT = D3DFMT_G16R16F;
  int nWidth = 256;
  int nHeight = 256;
	// Create attenuation texture
	if (FAILED(gcpRendD3D->mfGetD3DDevice()->CreateTexture(nWidth, nHeight, 1, 0, d3dFMT, D3DPOOL_MANAGED, &pTexture, NULL)))
		return NULL;
  sfExp = fExp*2;
	if (FAILED(D3DXFillTexture(pTexture, FillSpecularTexture, 0)))
		return NULL;

  if (!bMips)
    nFlags |= FT_NOMIPS;
  ti->m_RefTex.m_VidTex = (void *)pTexture;
  ti->m_RefTex.m_MipFilter = bMips ? D3DTEXF_POINT : D3DTEXF_NONE;
  if (m_bDeviceSupportsFP16Filter)
  {
    ti->m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
    ti->m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
  }
  else
  {
    ti->m_RefTex.m_MinFilter = D3DTEXF_POINT;
    ti->m_RefTex.m_MagFilter = D3DTEXF_POINT;
  }
  ti->m_RefTex.m_AnisLevel = 1;
  ti->m_RefTex.m_Type = TEXTGT_2D;
  ti->m_DstFormat = d3dFMT;
  ti->m_Width = nWidth;
  ti->m_Height = nHeight;
  CD3D9TexMan::CalcMipsAndSize(ti);
  tm->AddToHash(ti->m_Bind, ti);
  ti->Unlink();
  ti->Link(&STexPic::m_Root);
  gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;

  return ti;
}

float CD3D9TexMan::CalcFogVal(float fi, float fj)
{
  float fDelta = fabsf(fj - fi);
  if (fj > 0 && fi > 0)
    return 0;
  float fThr = -8;
  if (fj < fThr && fi < fThr)
    return 1.0f;
  float f1, f2, ff;
  if (fj > 0)
    ff = fj;
  else
    if (fi > 0)
      ff = fi;
    else
      ff = 0;
  if (fi > fj)
  {
    f1 = fi;
    f2 = fj;
  }
  else
  {
    f1 = fj;
    f2 = fi;
  }
  if (f1 > 0)
    f1 = 0;
  if (f2 < fThr)
    f2 = fThr;
  fThr = 1.0f / 8.0f;
  if (fDelta == 0)
    return -(fThr * fi);
  float fFog = (1.0f - ((f2 + f1) * fThr * -0.5f)) * (f1 - f2);
  ff = ((fDelta - ff) - fFog) / fDelta;
  float fMin = min(fi, fj) / -30.0f;
  if (fMin >= 1.0f)
    return 1;
  return (1.0f - fMin) * ff + fMin;
}

void CD3D9TexMan::GenerateFogMaps()
{
  int i, j;
  {
    float fdata[256];
    byte Data1[128][128][4];
    float f = 1.0f;
    for (i=0; i<256; i++)
    {
      fdata[i] = f;
      f *= 0.982f;
    }
    fdata[0] = 0;
    fdata[255] = 0;
    for (i=0; i<128; i++)
    {
      int ni = i - 64;
      for (j=0; j<128; j++)
      {
        int nj = j - 64;
        int nRes = nj*nj + ni*ni;
        float m = (float)(nRes);
        float fsq;
        if (nRes == 0)
          fsq = 1.0f;
        else
          fsq = 1.0f / cry_sqrtf(m); // [marco] crash, division by zero
        int iIndexF = (int)((fsq * m) / 63.0f * 255.0f);
        iIndexF = CLAMP(iIndexF, 0, 255);
        int iFog = (int)((1.0f - fdata[iIndexF]) * 255.0f);
        if (!i || i==127 || !j || j==127)
          iFog = 255;
        Data1[j][i][0] = Data1[j][i][1] = Data1[j][i][2] = 255;
        Data1[j][i][3] = (byte)iFog;
      }
    }
    gRenDev->m_TexMan->m_Text_Fog = CreateTexture("$Fog", 128, 128, 1, FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT | FT2_NOANISO, &Data1[0][0][0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
    //gRenDev->m_TexMan->m_Text_Fog->SaveTGA("Fog.tga", false);

    /*byte Data2[64][64][4];
    gcpOGL->EF_InitFogTables();
    for (i=0; i<64; i++)
    {
    for (j=0; j<64; j++)
    {
    //float fFog = CalcFogVal((float)(i-32), (float)(j-32));

    float fi = (float)i-63-10;
    float fj = (float)j-32;
    float fifi = fi/32.0f;
    fifi *= fifi;
    int nInd = (int)(((-fj/8.0f) * fifi) * 255.0f);
    nInd = Clamp(nInd, 0, 255);
    float fFog = SEvalFuncs::m_tFogFloats[nInd];
    int iFog = Clamp(int(fFog*255.0f), 0, 255);
    Data2[j][i][0] = Data2[j][i][1] = Data2[j][i][2] = 255;
    Data2[j][i][3] = (byte)iFog;
    }
    }*/

    //gRenDev->m_TexMan->m_Text_Fog_Enter = DownloadTexture("(FogEnter)", 64, 64, FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT, &Data2[0][0][0], eTT_Base, 0, NULL, 0, eTF_8888);
    //gRenDev->m_TexMan->m_Text_Fog_Enter->SaveTGA("FogEnter.tga", false);
    gRenDev->m_TexMan->m_Text_Fog_Enter = LoadTexture("Textures/FogEnter", FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT | FT2_NOANISO);
    gRenDev->m_TexMan->m_Text_VFog = LoadTexture("Textures/FogTex", FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT | FT2_NOANISO);
    //gRenDev->m_TexMan->m_Text_Fog_Enter->SaveTGA("FogEnter.tga", false);
    //gRenDev->m_TexMan->m_Text_Fog_Enter->SaveTGA("FogEnter.tga", false);
  }
}

//==============================================================================================================

VOID CFnMap9::Fill2DWrapper(D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize, LPVOID pData)
{
  CFnMap9* map = (CFnMap9*)pData;
  const D3DXCOLOR& c = map->Function( pTexCoord, pTexelSize );
  *pOut = D3DXVECTOR4((const float*)c);
}

HRESULT CFnMap9::Initialize()
{
  LPDIRECT3DTEXTURE9 pTexture;
  HRESULT hr;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();

  if ( FAILED( hr = D3DXCheckTextureRequirements(dv, &m_dwWidth, &m_dwHeight, &m_dwLevels, 0, &m_Format, D3DPOOL_MANAGED)))
  {
    OutputDebugString("Can't find valid texture format.\n");
    return hr;
  }

  if (FAILED (hr = dv->CreateTexture(m_dwWidth, m_dwHeight, 0, 0, m_Format, D3DPOOL_MANAGED, &pTexture, NULL)))
  {
    OutputDebugString("Can't create texture\n");
    return hr;
  }

  if (FAILED (hr = D3DXFillTexture(pTexture, Fill2DWrapper, (void*)this)))
    return hr;

  STexPicD3D *tp = (STexPicD3D *)m_pTex;
  SAFE_DELETE(tp->m_pFuncMap);
  tp->m_pFuncMap = this;
  tp->m_RefTex.m_VidTex = pTexture;
  tp->m_Width = m_dwWidth;
  tp->m_Height = m_dwHeight;

  return S_OK;
}

void CD3D9TexMan::GenerateGhostMap()
{
  CGhostMap *pGhostMap = new CGhostMap(256, m_Text_Ghost);
  pGhostMap->Initialize();
}

const float INV_RAND_MAX = 1.0 / (RAND_MAX + 1);
inline float frnd(float max=1.0) { return max * INV_RAND_MAX * rand(); }
inline float frnd(float min, float max) { return min + (max - min) * INV_RAND_MAX * rand(); }

void CFurMap::Bind(float layer, int nTMU)
{
  gcpRendD3D->EF_SelectTMU(nTMU);
  assert(m_nCurInst>=0 && m_nCurInst<m_Inst.Num());
  SFurLayers *fl = m_Inst[m_nCurInst];
  int temp = (int)floor(fl->m_Layers.Num() * max(0.0f, min(0.9999f, layer)));
  fl->m_Layers[temp]->Set();
}

void CFurMap::Update(int nNumLayers)
{
  int nInst;

  for (nInst=0; nInst<m_Inst.Num(); nInst++)
  {
    if (m_Inst[nInst]->m_Layers.Num() == nNumLayers)
    {
      m_nCurInst = nInst;
      return;
    }
  }
  SFurLayers *fl = new SFurLayers;
  m_nCurInst = nInst;
  m_Inst.AddElem(fl);
  Initialize(timeGetTime(), 128, nNumLayers, fl);
}

HRESULT CFurMap::Initialize(int seed, int size, int num, SFurLayers *fl)
{
  srand(seed);

  if (num > 60)
    num = 60;
  for (int i=0; i<fl->m_Layers.Num(); i++)
  {
    STexPic *tp = fl->m_Layers[i];
    if (tp)
      tp->Release(false);
  }
  fl->m_Layers.Free();

  CFColor *data = new CFColor[num*size*size];
  #define DATA(layer, x, y) data[size*size*(layer) + size*(y) + (x)]

  for (int x=0; x<size; x++)
  {
    for (int y=0; y<size; y++)
    {
      int layer;
      CFColor color;
      float t;

      // hair color
      color.r = 1.0f;
      color.g = 1.0f;
      color.b = 1.0f;
      if (frnd() < 0.15f)
        color *= frnd(0.2f, 0.4f);
      else
        color *= frnd(0.7f, 1.0f);

      for (layer=0; layer<num; layer++)
      {
        // lower layer is darker
        t = max(0, 1 - 2 * float(layer) / (num-1));
        DATA(layer, x, y) = color * float(1 - 0.7f * powf(t, 1.5f));
      }

      // length of the hair
      int length = 1 + uint(num * powf(frnd(), 3.0f));
      length = min(num, length);

      for (layer=0; layer<length; layer++)
      {
        // tip of the hair is semi-transparent
        t = max(0, -1 + 2 * float(layer+1) / length);
        DATA(layer, x, y).a = 1 - 0.85f * powf(t, 2.0f);
      }
    }
  }

  for (int layer=0; layer<num; layer++)
  {
    DWORD *pixels = new DWORD[size*size];
    DWORD *p = pixels;
    CFColor *pColor = &DATA(layer, 0, 0);
    for(int i=0; i<size*size; i++)
    {
      *p++ = pColor->GetTrue();
      pColor++;
    }
    char name[128];
    sprintf(name, "$AutoFur_%d", gcpRendD3D->m_TexGenID++);
    STexPic *tp = gcpRendD3D->m_TexMan->CreateTexture(name, size, size, 1, FT_HASALPHA, 0, (byte *)pixels, eTT_Base);
    fl->m_Layers.AddElem(tp);

    delete [] pixels;
  }

  delete [] data;

  return S_OK;
}

void CD3D9TexMan::GenerateFurMap()
{
  STexPic *ti = gcpRendD3D->m_TexMan->LoadTexture("$Fur", 0, 0, eTT_Base);
  CFurMap *pFurMap = new CFurMap(ti);
  ti->m_pFuncMap = pFurMap;
}

HRESULT CFurNormalMap::Initialize()
{
  // texture map for clamping
  {
    int size = 64;
    byte *pixels = new byte[size*size*4];
    byte *p = pixels;

    for (int j=0; j<size; j++)
    for (int i=0; i<size; i++)
    {
      D3DXVECTOR2 vec;
      vec.x = -1 + 2 * i / float(size - 1);
      vec.y = -1 + 2 * j / float(size - 1);
      float len = D3DXVec2Length(&vec);
      if (len > 1.0)
        vec /= len;
      *p++ = 0;
      *p++ = (unsigned char)(128 + 127 * vec.y);
      *p++ = (unsigned char)(128 + 127 * vec.x);
      *p++ = 0;
    }
    m_pTexClamp = gcpRendD3D->m_TexMan->CreateTexture("$FurTexClamp", size, size, 1, FT_CLAMP | FT_NOMIPS, 0, pixels, eTT_Base);

    delete [] pixels;
  }

  // texture map for normalization
  {
    int size = 64;
    byte *pixels = new byte[size*size*4];
    byte *p = pixels;

    for (int j=0; j<size; j++)
    for (int i=0; i<size; i++)
    {
      D3DXVECTOR3 n;
      n.x = -1 + 2 * i / float(size - 1);
      n.y = -1 + 2 * j / float(size - 1);
      n.z = 0;
      float len = D3DXVec3Length(&n);
      if (len > 1.0) n /= len;
      n.z = 1.0;
      D3DXVec3Normalize(&n, &n);
      *p++ = (unsigned char)(128 + 127 * n.z);
      *p++ = (unsigned char)(128 + 127 * n.y);
      *p++ = (unsigned char)(128 + 127 * n.x);
      *p++ = 0;
    }

    m_pTexNormalize = gcpRendD3D->m_TexMan->CreateTexture("$FurTexNormalize", size, size, 1, FT_CLAMP | FT_NOMIPS, 0, pixels, eTT_Base);

    delete [] pixels;
  }

  return S_OK;
}

void CFurNormalMap::Bind(int nTMU)
{
  assert(m_nCurInst>=0 && m_nCurInst<m_Inst.Num());

  SDynFurInstance *fr = &m_Inst[m_nCurInst];
  assert(fr->m_bPrepared && fr->m_pTexNormal);
  gcpRendD3D->EF_SelectTMU(nTMU);
  fr->m_pTexNormal->Set();
}

void CFurNormalMap::Update(EShaderPassType eShPass, float dt, SShaderPassHW *slw, bool bUseSimulation)
{
  SParamComp_User user;
  int i;

  CD3D9Renderer *rd = gcpRendD3D;
  SDynFurInstance *fr;

  Vec3 pos = rd->m_RP.m_pCurObject->GetTranslation();
  int nInst;
  if (!bUseSimulation)
    nInst = 0;
  else
  {
    for (nInst=1; nInst<m_Inst.Num(); nInst++)
    {
      fr = &m_Inst[nInst];
      if ((fr->m_Trans-pos).Length() < 0.25f)
        break;
    }
    if (nInst == m_Inst.Num())
    {
      int nFr = -1;
      for (i=1; i<m_Inst.Num(); i++)
      {
        fr = &m_Inst[i];
        if (nFr > (int)fr->m_nUsedFrame)
        {
          nFr = fr->m_nUsedFrame;
          nInst = i;
        }
      }
      if (nFr > rd->GetFrameID()-40)
      {
        nInst = m_Inst.Num();
        if (nInst == 0)
          nInst = 1;
      }
    }
  }
  if (nInst >= m_Inst.Num())
    m_Inst.ReserveNew(nInst+1);
  fr = &m_Inst[nInst];
  if (!fr->m_bPrepared)
  {
    if (!nInst)
      fr->m_pTexNormal = rd->m_TexMan->LoadTexture("Textures/white_ddn", 0, 0, eTT_Bumpmap);
    else
    {
      char name[128];
      byte *pixels = new byte[m_dwWidth*m_dwHeight*4];
      memset(pixels, 128, m_dwWidth*m_dwHeight*4);
      int num = rd->m_TexGenID++;
      sprintf(name, "$FurTexOffset0_%d", num);
      fr->m_pTexOffset0 = rd->m_TexMan->CreateTexture(name, m_dwWidth, m_dwHeight, 1, FT_NOMIPS, FT2_FILTER_NEAREST | FT2_RENDERTARGET, pixels, eTT_Base);
      sprintf(name, "$FurTexOffset0_%d", num);
      fr->m_pTexOffset1 = rd->m_TexMan->CreateTexture(name, m_dwWidth, m_dwHeight, 1, FT_NOMIPS, FT2_FILTER_NEAREST | FT2_RENDERTARGET, pixels, eTT_Base);
      sprintf(name, "$FurNormalMap_%d", num);
      fr->m_pTexNormal = rd->m_TexMan->CreateTexture(name, m_dwWidth, m_dwHeight, 1, FT_NOMIPS, FT2_RENDERTARGET, pixels, eTT_Base);
      delete [] pixels;
      ClearRenderTarget(rd->m_pd3dDevice, fr->m_pTexOffset0, 128, 128, 128, 128);
      ClearRenderTarget(rd->m_pd3dDevice, fr->m_pTexOffset1, 128, 128, 128, 128);
      ClearRenderTarget(rd->m_pd3dDevice, fr->m_pTexNormal, 128, 128, 128, 128);
    }
    fr->m_bPrepared = true;
  }
  fr->m_nUsedFrame = rd->GetFrameID();
  fr->m_Trans = pos;
  m_nCurInst = nInst;
  // Inst 0 only for non-simulated fur
  if (!nInst)
    return;

  static bool bUseGravity = true;
  if ((GetAsyncKeyState('G') & 0x8000))
    bUseGravity = true;
  if ((GetAsyncKeyState('N') & 0x8000))
    bUseGravity = false;

  // gravity in model space
  Vec3 gravity;
  if (!bUseGravity)
    gravity = Vec3(0,0,-0.01f);
  else
  {
    user.m_Name = "furgravityx";
    gravity.x = user.mfGet();
    user.m_Name = "furgravityy";
    gravity.y = user.mfGet();
    user.m_Name = "furgravityz";
    gravity.z = user.mfGet();
  }

  Vec3 velocity(0,0,0);
  Vec3 omega(0,0,0);
  Vec3 trGravity, trVelocity, trOmega;
  Matrix44 &m = rd->m_RP.m_pCurObject->GetInvMatrix();
  trGravity = m.TransformVectorOLD(gravity);
  trVelocity = m.TransformVectorOLD(velocity);
  trOmega = m.TransformVectorOLD(omega);

  STexPic *pTexTarg, *pTexCur;
  if (!(fr->m_nFrame & 1))
  {
    pTexTarg = fr->m_pTexOffset0;
    pTexCur  = fr->m_pTexOffset1;
  }
  else
  {
    pTexTarg = fr->m_pTexOffset1;
    pTexCur  = fr->m_pTexOffset0;
  }
  float filter = min(0.25f, dt/0.1f);
  static float dt2 = 0.025f;
  dt2 = 0.9f * dt2 + 0.1f * dt;

  CD3D9TexMan::BindNULL(2);

  // translational m_Acceleration
  fr->m_Accel = (1.0f - filter) * fr->m_Accel + filter * ((trVelocity - fr->m_PrevVel) / dt2);
  fr->m_PrevVel = trVelocity;

  // angular m_Acceleration
  fr->m_OmegaAccel = (1.0f - filter) * fr->m_OmegaAccel + filter * ((trOmega - fr->m_PrevOmega) / dt2);
  fr->m_PrevOmega = trOmega;

  float damping = max(0.0625f, min(0.5f, dt/0.3f));

  // get current viewport
  int iTmpX, iTmpY, iTempWidth, iTempHeight;
  rd->GetViewport(&iTmpX, &iTmpY, &iTempWidth, &iTempHeight);   
  rd->SetViewport(0, 0, pTexTarg->m_Width, pTexTarg->m_Height);    

  LPDIRECT3DSURFACE9 pTexSurf;
  LPDIRECT3DTEXTURE9 plD3DTexture;
  plD3DTexture = (LPDIRECT3DTEXTURE9)pTexTarg->m_RefTex.m_VidTex;
  plD3DTexture->GetSurfaceLevel(0, &pTexSurf);    
  // set current rendertarget  
  rd->EF_SetRenderTarget(pTexSurf, 1);
  SAFE_RELEASE(pTexSurf);

  int nFlags = rd->m_RP.m_FlagsModificators;
  rd->m_RP.m_FlagsModificators = RBMF_TANGENTSUSED;

  CCGVProgram_D3D *vpOffsGen = (CCGVProgram_D3D *)rd->m_RP.m_VPFur_OffsGen;
  CCGPShader_D3D *fpOffsGen = (CCGPShader_D3D *)rd->m_RP.m_RCFur_OffsGen;
  if (vpOffsGen && fpOffsGen)
  {
    vpOffsGen->mfSet(true, 0);
    fpOffsGen->mfSet(true, 0);

    vpOffsGen->mfParameter4f("Force", sfparam(trGravity - 0.12f*trVelocity - 0.018f*fr->m_Accel));
    vpOffsGen->mfParameter4f("Omega", sfparam(0.12f*trOmega + 0.018f*fr->m_OmegaAccel));
    vpOffsGen->mfParameter4f("Damping", sfparam(damping));

    float v[4];
    CREOcLeaf *re = (CREOcLeaf *)rd->m_RP.m_pRE;
    CLeafBuffer *lb = re->m_pBuffer;
    v[2] = -lb->m_fMinU;
    v[0] = 1.0f / (lb->m_fMaxU + v[2]);
    v[3] = -lb->m_fMinV;
    v[1] = 1.0f / (lb->m_fMaxV + v[3]);
    vpOffsGen->mfParameter4f("ScaleBiasTC", v);

    rd->EF_SelectTMU(0);
    pTexCur->Set();
    rd->EF_SelectTMU(1);
    m_pTexClamp->Set();

    rd->SetCullMode(R_CULL_NONE);
    rd->EF_SetState(GS_NODEPTHTEST);
    rd->EF_Draw(rd->m_RP.m_pShader, NULL);

    vpOffsGen->mfSet(false, 0);
    fpOffsGen->mfSet(false, 0);
  }
  plD3DTexture = (LPDIRECT3DTEXTURE9)fr->m_pTexNormal->m_RefTex.m_VidTex;
  plD3DTexture->GetSurfaceLevel(0, &pTexSurf);    
  // set current rendertarget  
  rd->EF_SetRenderTarget(pTexSurf, 1);
  SAFE_RELEASE(pTexSurf);

  CCGVProgram_D3D *vpNormGen = (CCGVProgram_D3D *)rd->m_RP.m_VPFur_NormGen;
  CCGPShader_D3D *fpNormGen = (CCGPShader_D3D *)rd->m_RP.m_RCFur_NormGen;
  if (vpNormGen && fpNormGen)
  {
    vpNormGen->mfSet(true, 0);
    fpNormGen->mfSet(true, 0);

    rd->EF_SelectTMU(0);
    pTexTarg->Set();
    rd->EF_SelectTMU(1);
    m_pTexNormalize->Set();

    // setup screen aligned quad
    struct_VERTEX_FORMAT_P3F_TEX2F pScreen[] =  
    {
      Vec3(-1, -1, 0), 0, 1,   
      Vec3(-1, 1, 0), 0, 0,    
      Vec3(1, -1, 0), 1, 1,   
      Vec3(1, 1, 0), 1, 0,   
    }; 
    rd->DrawTriStrip(&(CVertexBuffer (pScreen,VERTEX_FORMAT_P3F_TEX2F)), 4);  

    vpNormGen->mfSet(false, 0);
    fpNormGen->mfSet(false, 0);
  }
  rd->EF_RestoreRenderTarget();
  rd->SetViewport(iTmpX, iTmpY, iTempWidth, iTempHeight);
  rd->m_RP.m_FlagsModificators = nFlags;

  fr->m_nFrame++;
}

void CD3D9TexMan::GenerateFurNormalMap()
{
  gRenDev->m_TexMan->m_Text_FurNormalMap = LoadTexture("$FurNormalMap", FT_CLAMP | FT_NOREMOVE, FT2_NODXT);

  CFurNormalMap *pFurNormalMap = new CFurNormalMap(gRenDev->m_TexMan->m_Text_FurNormalMap, 64, 32);
  gRenDev->m_TexMan->m_Text_FurNormalMap->m_pFuncMap = pFurNormalMap;
  pFurNormalMap->Initialize();
}

void CD3D9TexMan::GenerateFurLightMap()
{
  // texture size (as small as possible in order to minimize cache miss rate)
  int s_size = 16;
  int t_size = 32;
  int r_size = 16;

  // exponents for diffuse and specular
  float diff_power = 6;
  float spec_power = 32;

  byte *pixels = new byte[2*s_size*t_size*r_size];
  byte *p = pixels;

  for(int k=0; k<r_size; k++)
  for(int j=0; j<t_size; j++)
  for(int i=0; i<s_size; i++)
  {
    float LN = float(i)/(s_size-1); // s = L dot N
    float HN = float(j)/(t_size-1); // t = H dot N
    float Lz = float(2*k)/(r_size-1) - 1; // r = L.z

    float diffuse  = 0.2f + 0.2f * max(0, Lz) + 0.6f * powf(1 - LN*LN, 0.5f*diff_power);
    float specular = powf(1 - HN*HN, 0.5f*spec_power);
    float shadow   = powf(cosf(D3DX_PI/2 * CLAMP(-0.2f-1.4f*Lz, 0.0f, 1.0f)), 4);

    *p++ = (unsigned char)(255 * CLAMP(diffuse, 0.0f, 1.0f));
    *p++ = (unsigned char)(255 * CLAMP(specular*shadow, 0.0f, 1.0f));
  }

  gRenDev->m_TexMan->m_Text_FurLightMap = CreateTexture("$FurLightMap", s_size, t_size, r_size, FT_CLAMP | FT_NOMIPS, FT2_NODXT, pixels, eTT_3D, -1, -1, 0, NULL, 0, eTF_0088);
  gRenDev->m_TexMan->m_Text_FurLightMap->m_RefTex.bRepeats = 2;
}

//========================================================================

void CD3D9TexMan::GenerateFlareMap()
{
  int i, j;

  byte data[4][32][4];
  for (i=0; i<32; i++)
  {
    float f = 1.0f - ((fabsf((float)i - 15.5f) - 0.5f) / 16.0f);
    int n = (int)(f*f*255.0f);
    for (j=0; j<4; j++)
    {
      byte b = n;
      if (n < 0)
        b = 0;
      else
        if (n > 255)
          b = 255;
      data[j][i][0] = b;
      data[j][i][1] = b;
      data[j][i][2] = b;
      data[j][i][3] = 255;
    }
  }
  gRenDev->m_TexMan->m_Text_Flare = CreateTexture("$Flare", 32, 4, 1, FT_CLAMP | FT_NOREMOVE, FT2_NODXT | FT2_NOANISO, &data[0][0][0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
}

#define NOISE_DIMENSION 128
#define NOISE_MIP_LEVELS 1

static float s_Noise[NOISE_DIMENSION][NOISE_DIMENSION][NOISE_DIMENSION];
static double drand48(void)
{
   double dVal;
   dVal=(double)rand()/(double)RAND_MAX;
   
   return dVal;
}


//-----------------------------------------------------------------------------
// Name: InitWhiteNoiseArray()
// Desc: Initializes a user memory array of white noise which will be used
//       during noise texture generation
//-----------------------------------------------------------------------------
static void InitWhiteNoiseArray (void)
{
  int i,j,k;

  for (i=0; i<NOISE_DIMENSION; i++)
  {
    for (j=0; j<NOISE_DIMENSION; j++)
    {
      for (k=0; k<NOISE_DIMENSION; k++)
      {
        s_Noise[i][j][k] = (float)drand48();
      }
    }
  }
}

//-----------------------------------------------------------------------------
// Name: lerpNoise()
// Desc: Helper function which does wrapped interpolation of noise.  This is
//       necessary for generating tilable noise
//-----------------------------------------------------------------------------
float lerpNoise(float x, float y,float z, int wrapX, int wrapY, int wrapZ)
{
  int ix,iy,iz,ixd,iyd,izd;  // integer parts of index into texture array
  float rx,ry,rz;           // fractional part of xyz  
  float accum;            // value returned

  ix=((int)x);
  iy=((int)y);
  iz=((int)z);

  rx=x-(float)ix;
  ry=y-(float)iy;
  rz=z-(float)iz;
  ix=ix&(wrapX-1);
  iy=iy&(wrapY-1);
  iz=iz&(wrapZ-1);
  ixd=(ix+1)&(wrapX-1);
  iyd=(iy+1)&(wrapY-1);
  izd=(iz+1)&(wrapZ-1);

  accum=0;
  accum+=rx*ry*rz*s_Noise[izd][iyd][ixd];
  accum+=(1-rx)*ry*rz*s_Noise[izd][iyd][ix];
  accum+=rx*(1-ry)*rz*s_Noise[izd][iy][ixd];
  accum+=(1-rx)*(1-ry)*rz*s_Noise[izd][iy][ix];

  accum+=rx*ry*(1-rz)*s_Noise[iz][iyd][ixd];
  accum+=(1-rx)*ry*(1-rz)*s_Noise[iz][iyd][ix];
  accum+=rx*(1-ry)*(1-rz)*s_Noise[iz][iy][ixd];
  accum+=(1-rx)*(1-ry)*(1-rz)*s_Noise[iz][iy][ix];

  return accum;
}

//-----------------------------------------------------------------------------
// Name: turbulence()
// Desc: 
//-----------------------------------------------------------------------------
float turbulence(float x, float y, float z, int wrapX, int wrapY, int wrapZ, float maxScale)
{
  float t=0;
  float scale = maxScale;

  while(scale>=1)
  {
    t+=lerpNoise(x/scale, y/scale, z/scale, wrapX/(int)scale, wrapY/(int)scale, wrapZ/(int)scale) * 128*(scale/maxScale);
    scale/=2.0f;
  }

  return(t);
}

//-----------------------------------------------------------------------------
// Name: turbulenceFill()
// Desc: Passed to D3DXFillVolumeTexture to fill in noise volume texture
//-----------------------------------------------------------------------------
VOID WINAPI turbulenceFill (D3DXVECTOR4* pOut, const D3DXVECTOR3* pTexCoord, const D3DXVECTOR3* pTexelSize, LPVOID pData)
{
  float fSX = 1.0f / pTexelSize->x;
  float fSY = 1.0f / pTexelSize->y;
  float fSZ = 1.0f / pTexelSize->z;
  float turb = turbulence(pTexCoord->x*fSX, pTexCoord->y*fSY, pTexCoord->z*fSZ, (int)fSX, (int)fSY, (int)fSZ, fSZ/4) / 256.0f; // Get turbulence from tilable noise

  turb = max(0.0f, min(1.0f, turb));    // Clamp

  *pOut = D3DXVECTOR4(turb, turb, turb, turb);
}

void CD3D9TexMan::GenerateNoiseVolumeMap()
{  
  InitWhiteNoiseArray();

  HRESULT hr;
  LPDIRECT3DVOLUMETEXTURE9 pID3DVolTexture = NULL;

  if (FAILED(hr=D3DXCreateVolumeTexture (gcpRendD3D->mfGetD3DDevice(), NOISE_DIMENSION, NOISE_DIMENSION, NOISE_DIMENSION, NOISE_MIP_LEVELS, 0, D3DFMT_A8, D3DPOOL_MANAGED ,&pID3DVolTexture)))
    return;

  if (FAILED(hr=D3DXFillVolumeTexture (pID3DVolTexture, turbulenceFill, NULL)))
    return;

  if (!gRenDev->m_TexMan->m_Text_NoiseVolumeMap)
    gRenDev->m_TexMan->m_Text_NoiseVolumeMap = LoadTexture("$NoiseVolume", FT_NOREMOVE | FT_NOSTREAM | FT_NOMIPS, FT2_NODXT, eTT_3D, -1.0f, -1.0f, -1);
  STexPicD3D *ti = (STexPicD3D *)gRenDev->m_TexMan->m_Text_NoiseVolumeMap;
  ti->m_RefTex.m_VidTex = (void *)pID3DVolTexture;
  ti->m_RefTex.m_MipFilter = D3DTEXF_NONE;
  ti->m_RefTex.m_MinFilter = D3DTEXF_LINEAR;
  ti->m_RefTex.m_MagFilter = D3DTEXF_LINEAR;
  ti->m_RefTex.m_AnisLevel = 1;
  ti->m_RefTex.bRepeats = true;
  ti->m_RefTex.m_Type = TEXTGT_3D;
  ti->m_DstFormat = D3DFMT_A8;
  ti->m_Width  = NOISE_DIMENSION;
  ti->m_Height = NOISE_DIMENSION;
  ti->m_Depth  = NOISE_DIMENSION;
  CD3D9TexMan::CalcMipsAndSize(ti);
  AddToHash(ti->m_Bind, ti);
  ti->Unlink();
  ti->Link(&STexPic::m_Root);
  gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;
}

//=================================================================================

VOID WINAPI FillAttenuationTexture(D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize,	LPVOID pData)
{
	const unsigned int index = unsigned int(pTexCoord->y * float(NUM_ATTENUATION_FUNCTIONS) * float(BILERP_PROTECTION));
	const unsigned int matNum = index / BILERP_PROTECTION;

  if ( matNum <= AF_LINEAR )
	{
		// just flip things over so at distance 0 attenuation = 1 
		float atten = 1.f - pTexCoord->x;
		pOut->x = atten;
	}
  else
  if ( matNum <= AF_SQUARED )
	{
		// square the flipped input
		float atten = 1.f - pTexCoord->x;
		atten = atten * atten;
		pOut->x = atten;
	}
  else
  if ( matNum <= AF_SHAPE1 )
	{
		// a constant start (for 0.5f)then linear to 0 at in->x = 1
		float atten = 1.f - pTexCoord->x;
		atten = (atten * 1.5f);
		if( atten > 1 )
			atten = 1;
		pOut->x = atten;
	}
  else
  if ( matNum <= AF_SHAPE2 )
	{
		// a constant start (for 0.3f) then squared to 0 at in->x = 1
		float atten = 1.f - pTexCoord->x;
		atten = (atten * 1.3f);
		if( atten > 1 )
			atten = 1;
		atten /= 1.3f;
		atten = atten * atten;
		pOut->x = atten;
	}
	if( pTexCoord->x > 0.9999f )
	{
		pOut->x = 0;
	}
  pOut->y = pOut->x;
  pOut->z = pOut->x;
  pOut->w = pOut->x;
}

void CD3D9TexMan::GenerateAttenMap()
{
  LPDIRECT3DTEXTURE9 pTexture;

  if (!gcpRendD3D->mFormatU16V16.BitsPerPixel)
    return;

  CD3D9Renderer *rd = gcpRendD3D;
  bool bMips = false;
  D3DFORMAT d3dFMT;
  if (rd->mFormatA8L8.BitsPerPixel)
    d3dFMT = D3DFMT_A8L8;
  else
  if (rd->mFormatA8.BitsPerPixel)
    d3dFMT = D3DFMT_A8;
  int nWidth = ATTENUATION_WIDTH;
  int nHeight = NUM_ATTENUATION_FUNCTIONS * BILERP_PROTECTION;
	// Create attenuation texture
	if (FAILED(gcpRendD3D->mfGetD3DDevice()->CreateTexture(nWidth, nHeight, 1, 0, d3dFMT, D3DPOOL_MANAGED, &pTexture, NULL)))
		return;
	if (FAILED(D3DXFillTexture(pTexture, FillAttenuationTexture, 0)))
		return;

  int nFlags = FT_NOREMOVE | FT_NOSTREAM | FT_CLAMP;
  if (!bMips)
    nFlags |= FT_NOMIPS;
  STexPicD3D *ti = (STexPicD3D *)LoadTexture("$Attenuation1D", nFlags, FT2_NODXT | FT2_NOANISO, eTT_Base, -1.0f, -1.0f);
  ti->m_RefTex.m_VidTex = (void *)pTexture;
  ti->m_RefTex.m_MipFilter = bMips ? GetMipFilter() : D3DTEXF_NONE;
  ti->m_RefTex.m_MinFilter = GetMinFilter();
  ti->m_RefTex.m_MagFilter = GetMagFilter();
  ti->m_RefTex.m_AnisLevel = 1;
  ti->m_RefTex.m_Type = TEXTGT_2D;
  ti->m_DstFormat = d3dFMT;
  ti->m_Width = nWidth;
  ti->m_Height = nHeight;
  CD3D9TexMan::CalcMipsAndSize(ti);
  AddToHash(ti->m_Bind, ti);
  ti->Unlink();
  ti->Link(&STexPic::m_Root);
  gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;
  gRenDev->m_TexMan->m_Text_Atten1D = ti;
}

void CD3D9TexMan::GenerateDepthLookup()
{
  int i;

  DWORD data[2048];
  DWORD* pMap = data;
  for (i=0; i<2048; i++)
  {
    *pMap++ = D3DCOLOR_RGBA(i&0xFF, (i&0xFF00)>>3, 0, 0);
  }
  gRenDev->m_TexMan->m_Text_DepthLookup = CreateTexture("$DepthMap", 2048, 1, 1, FT_CLAMP | FT_NOREMOVE | FT_HASALPHA | FT_NOMIPS | FT_PROJECTED, FT2_NODXT | FT2_NOANISO, (byte *)&data[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);


  DWORD data2[4][4];
  pMap = &data2[0][0];
  for (i=0; i<4*4; i++)
  {
    *pMap++ = D3DCOLOR_RGBA(0xff, 0xe0, 0, 0 );
  }
  gRenDev->m_TexMan->m_Text_Depth = CreateTexture("$Depth", 4, 4, 1, FT_CLAMP | FT_NOREMOVE | FT_HASALPHA | FT_NOMIPS, FT2_NODXT, (byte *)&data2[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  pMap = &data2[0][0];
  for (i=0; i<4*4; i++)
  {
    *pMap++ = D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0 );
  }
  gRenDev->m_TexMan->m_Text_WhiteShadow = CreateTexture("$WhiteShadow", 4, 4, 1, FT_CLAMP | FT_NOREMOVE | FT_HASALPHA | FT_NOMIPS, FT2_NODXT | FT2_NOANISO, (byte *)&data2[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  byte data3[256];
  byte *pbMap = &data3[0];
  for (i=0; i<256; i++)
  {
    *pbMap++ = i;
  }
  gRenDev->m_TexMan->m_Text_Gradient = CreateTexture("$AlphaGradient", 256, 1, 1, FT_NOREMOVE | FT_HASALPHA | FT_NOMIPS, FT2_NODXT | FT2_NOANISO, (byte *)&data3[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8000);
  //gRenDev->m_TexMan->m_Text_Depth->SaveTGA("Depth.tga", false);
}

#include "D3DCubemaps.h"

void CD3D9TexMan::GenerateFuncTextures()
{
  LPDIRECT3DCUBETEXTURE9 pNormCubeTexture;
  LPDIRECT3DCUBETEXTURE9 pLightCubeTexture;
  if (gRenDev->GetFeatures() & RFT_BUMP)
  {
    SSingleLight f(32); // TO_LIGHT_CUBE_MAP
    if(SUCCEEDED(D3DXCreateCubeTexture(gcpRendD3D->mfGetD3DDevice(), 64, D3DX_DEFAULT, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pLightCubeTexture)))
    {
      int nl = pLightCubeTexture->GetLevelCount();
      MakeCubeMap<SSingleLight>(f, pLightCubeTexture, 64, true);
    }

    SNormalizeVector norm;  // TO_NORMALIZE_CUBE_MAP
    if(SUCCEEDED(D3DXCreateCubeTexture(gcpRendD3D->mfGetD3DDevice(), 256, D3DX_DEFAULT, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pNormCubeTexture)))
      MakeCubeMap<SNormalizeVector>(norm, pNormCubeTexture, 256, true);
    if (!gRenDev->m_TexMan->m_Text_NormalizeCMap)
      gRenDev->m_TexMan->m_Text_NormalizeCMap = LoadTexture("$NormalizeCMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_NORMALIZE_CUBE_MAP);
    STexPicD3D *ti = (STexPicD3D *)gRenDev->m_TexMan->m_Text_NormalizeCMap;
    ti->m_RefTex.m_VidTex = (void *)pNormCubeTexture;
    ti->m_RefTex.m_MipFilter = GetMipFilter();
    ti->m_RefTex.m_MinFilter = GetMinFilter();
    ti->m_RefTex.m_MagFilter = GetMagFilter();
    ti->m_RefTex.m_AnisLevel = 1;
    ti->m_RefTex.m_Type = TEXTGT_CUBEMAP;
    ti->m_DstFormat = D3DFMT_X8R8G8B8;
    ti->m_Width = 256;
    ti->m_Height = 256;
    CD3D9TexMan::CalcMipsAndSize(ti);
    ti->m_Size *= 6;
    AddToHash(ti->m_Bind, ti);
    ti->Unlink();
    ti->Link(&STexPic::m_Root);
    gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;
    CheckTexLimits(NULL);
  }

  GenerateFogMaps();
  GenerateFlareMap();
  GenerateGhostMap();
  GenerateDepthLookup();
  GenerateAttenMap();
  if (CRenderer::CV_r_Quality_BumpMapping > 2 && (gRenDev->GetFeatures() & RFT_HW_PS20))
  {
    //GenerateNoiseVolumeMap();
    GenerateFurMap();
    GenerateFurNormalMap();
    GenerateFurLightMap();
  }

  //byte *dat = new byte[16*16*2];
  //gRenDev->DownLoadToVideoMemory(dat, 16, 16, eTF_4444, eTF_4444, 1);
}

void STexPic::Preload (int Flags)
{
}
void STexPicD3D::Preload (int Flags)
{
  IDirect3DTexture9 *pID3DTexture = NULL;
  IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
  if (m_Flags2 & FT2_RENDERTARGET)
    return;
  if (m_eTT == eTT_Cubemap)
  {
    pID3DCubeTexture = (IDirect3DCubeTexture9*)m_RefTex.m_VidTex;
    if (pID3DCubeTexture)
      pID3DCubeTexture->PreLoad();
  }
  else
  {
    pID3DTexture = (IDirect3DTexture9*)m_RefTex.m_VidTex;
    if (pID3DTexture)
      pID3DTexture->PreLoad();
  }
}
