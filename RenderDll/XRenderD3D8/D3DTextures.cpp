/*=============================================================================
  D3DTextures.cpp : Direct3D specific texture manager implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"
#include "I3dengine.h"


int CD3D8TexMan::m_Format = D3DFMT_A8R8G8B8;
int CD3D8TexMan::m_FirstBind = 1;
TTextureMap CD3D8TexMan::m_RefTexs;

//===============================================================================

void STexPic::ReleaseHW()
{
}

void STexPic::SetFilter()
{
}

void STexPic::SetWrapping()
{
}

void STexPic::Set()
{
}
void STexPic::SetClamp(bool bEnable)
{
}

void STexPicD3D::Release(bool bForce)
{
  STexPic::Release(bForce);
}

void STexPicD3D::ReleaseHW()
{
  if (!(m_Flags2 & FT2_WASUNLOADED) && (m_Bind && m_Bind != TX_FIRSTBIND)) 
  {
    m_Flags2 &= ~FT2_PARTIALLYLOADED;
    if (m_LoadedSize)
      CTexMan::m_StatsCurTexMem -= m_LoadedSize;
    else
      CTexMan::m_StatsCurTexMem -= m_Size;
    m_LoadedSize = 0;
    Unlink();

    if (!m_RefTex)
      return;

    CD3D8Renderer *r = gcpRendD3D;
    LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
    IDirect3DBaseTexture8 *pTex = (IDirect3DBaseTexture8*)m_RefTex->m_VidTex;
    SAFE_RELEASE(pTex);
    m_RefTex->m_VidTex = NULL;
  }
}


void STexPicD3D::SetClamp(bool bEnable)
{
  if (!m_RefTex || !m_RefTex->m_VidTex)
    return;
  if (bEnable)
    m_RefTex->bRepeats = false;
  else
    m_RefTex->bRepeats = true;
}

void STexPicD3D::Set()
{
  if (!m_RefTex)
    return;
  CD3D8Renderer *r = gcpRendD3D;
  int tmu = r->m_TexMan->m_CurStage;
  if ((m_Flags2 & (FT2_WASUNLOADED | FT2_PARTIALLYLOADED)))
  {
    int Size = m_LoadedSize;
    Restore();
    if (Size != m_LoadedSize)
      r->mStages[tmu].Texture = NULL;
  }
  else
    Relink(&STexPic::m_Root);

  if (CRenderer::CV_r_log == 3 || CRenderer::CV_r_log == 4)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "STexPic::Set(): (%d) \"%s\"\n", tmu, m_SourceName.c_str());

  if (!m_RefTex->m_VidTex)
    return;

  LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
  HRESULT hr;
  if (r->mStages[tmu].Texture == this)
    return;
  r->mStages[tmu].Texture = this;
  hr = dv->SetTexture(tmu, (IDirect3DBaseTexture8*)m_RefTex->m_VidTex);

#ifndef _XBOX
  if (m_RefTex->m_Pal>0 && (m_Flags & FT_PALETTED) && r->mStages[tmu].Palette != m_RefTex->m_Pal)
  {
    r->mStages[tmu].Palette = m_RefTex->m_Pal;
    dv->SetCurrentTexturePalette(m_RefTex->m_Pal);
  }
#else
  if (m_RefTex->m_pPal && (m_Flags & FT_PALETTED) && r->mStages[tmu].pPalette != m_RefTex->m_pPal)
  {
    r->mStages[tmu].pPalette = m_RefTex->m_pPal;
    dv->SetPalette(tmu, m_RefTex->m_pPal);
  }
  if (m_eTT == eTT_DSDTBump)
    dv->SetTextureStageState( tmu, D3DTSS_COLORSIGN, D3DTSIGN_RSIGNED|D3DTSIGN_GSIGNED|D3DTSIGN_BSIGNED );
  else
    dv->SetTextureStageState( tmu, D3DTSS_COLORSIGN, 0 );

#endif

  if(m_RefTex->bMips!=r->mStages[tmu].UseMips || m_RefTex->m_MagFilter!=r->mStages[tmu].MagFilter || m_RefTex->m_MinFilter!=r->mStages[tmu].MinFilter || m_RefTex->m_AnisLevel!=r->mStages[tmu].Anisotropic)
  {
    r->mStages[tmu].UseMips = m_RefTex->bMips;
    r->mStages[tmu].Anisotropic = m_RefTex->m_AnisLevel;
    r->mStages[tmu].MagFilter = m_RefTex->m_MagFilter;
    r->mStages[tmu].MinFilter = m_RefTex->m_MinFilter;
    if (!m_RefTex->bMips)
    {
      dv->SetTextureStageState( tmu, D3DTSS_MIPFILTER, D3DTEXF_NONE);
      dv->SetTextureStageState( tmu, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
      dv->SetTextureStageState( tmu, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    }
    else
    if (m_RefTex->m_AnisLevel > 1)
    {
      dv->SetTextureStageState( tmu, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
      dv->SetTextureStageState( tmu, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC );
      dv->SetTextureStageState( tmu, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC );
      dv->SetTextureStageState( tmu, D3DTSS_MAXANISOTROPY, m_RefTex->m_AnisLevel);
    }
    else
    {
      CD3D8TexMan *tm = (CD3D8TexMan *)r->m_TexMan;
      dv->SetTextureStageState(tmu, D3DTSS_MIPFILTER, tm->GetMipFilter() );
      dv->SetTextureStageState(tmu, D3DTSS_MAGFILTER, m_RefTex->m_MagFilter );
      dv->SetTextureStageState(tmu, D3DTSS_MINFILTER, m_RefTex->m_MinFilter );
    }
  }
  if (m_RefTex->bRepeats != r->mStages[tmu].Repeat)
  {
    r->mStages[tmu].Repeat = m_RefTex->bRepeats;
    dv->SetTextureStageState(tmu, D3DTSS_ADDRESSU, m_RefTex->bRepeats ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    dv->SetTextureStageState(tmu, D3DTSS_ADDRESSV, m_RefTex->bRepeats ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    if (m_RefTex->m_Type == TEXTGT_CUBEMAP)
      dv->SetTextureStageState(tmu, D3DTSS_ADDRESSW, m_RefTex->bRepeats ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
  }
  gRenDev->m_TexMan->m_LastTex = this;
//  SaveTGA("bug.tga", false);
}

void CD3D8TexMan::SetTexture(int Id, ETexType eTT)
{
  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
  int tmu = r->m_TexMan->m_CurStage;

  SRefTex *rt = NULL;
  if (Id > TX_FIRSTBIND)
  {
    STexPicD3D *tp = (STexPicD3D *)m_Textures[Id-TX_FIRSTBIND];
    if (tp && tp->m_Bind == Id)
    {
      tp->Set();
      return;
    }
  }
  if (!rt)
  {
    TTextureMapItor it = m_RefTexs.find(Id);
    if (it == m_RefTexs.end())
    {
      dv->SetTexture(tmu, NULL);
      r->mStages[tmu].Texture = NULL;
      return;
    }
    rt = it->second;
  }
  r->mStages[tmu].Texture = NULL;
  dv->SetTexture(tmu, (IDirect3DBaseTexture8*)rt->m_VidTex);

#ifndef _XBOX
  if (rt->m_Pal>0 && rt->m_Pal!=r->mStages[tmu].Palette)
  {
    r->mStages[tmu].Palette = rt->m_Pal;
    dv->SetCurrentTexturePalette(rt->m_Pal);
  }
#else
  if (rt->m_pPal && r->mStages[tmu].pPalette != rt->m_pPal)
  {
    r->mStages[tmu].pPalette = rt->m_pPal;
    dv->SetPalette(tmu, rt->m_pPal);
  }
  dv->SetTextureStageState( tmu, D3DTSS_COLORSIGN, 0 );
#endif

  if(rt->bMips!=r->mStages[tmu].UseMips || rt->m_MagFilter!=r->mStages[tmu].MagFilter || rt->m_MinFilter!=r->mStages[tmu].MinFilter || rt->m_AnisLevel!=r->mStages[tmu].Anisotropic)
  {
    r->mStages[tmu].UseMips = rt->bMips;
    r->mStages[tmu].Anisotropic = rt->m_AnisLevel;
    r->mStages[tmu].MagFilter = rt->m_MagFilter;
    r->mStages[tmu].MinFilter = rt->m_MinFilter;
    if (!rt->bMips)
    {
      dv->SetTextureStageState( tmu, D3DTSS_MIPFILTER, D3DTEXF_NONE);
      dv->SetTextureStageState( tmu, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
      dv->SetTextureStageState( tmu, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    }
    else
    if (rt->m_AnisLevel > 1)
    {
      dv->SetTextureStageState( tmu, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
      dv->SetTextureStageState( tmu, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC );
      dv->SetTextureStageState( tmu, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC );
      dv->SetTextureStageState( tmu, D3DTSS_MAXANISOTROPY, rt->m_AnisLevel);
    }
    else
    {
      dv->SetTextureStageState(tmu, D3DTSS_MIPFILTER, m_MipFilter );
      dv->SetTextureStageState(tmu, D3DTSS_MAGFILTER, rt->m_MagFilter );
      dv->SetTextureStageState(tmu, D3DTSS_MINFILTER, rt->m_MinFilter );
    }
  }
  if (rt->bRepeats != r->mStages[tmu].Repeat)
  {
    r->mStages[tmu].Repeat = rt->bRepeats;
    dv->SetTextureStageState(tmu, D3DTSS_ADDRESSU, rt->bRepeats ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    dv->SetTextureStageState(tmu, D3DTSS_ADDRESSV, rt->bRepeats ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    if (rt->m_Type == TEXTGT_CUBEMAP)
      dv->SetTextureStageState(tmu, D3DTSS_ADDRESSW, rt->bRepeats ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
  }
  if (rt->m_SrcTex)
    m_LastTex = rt->m_SrcTex;
}


int SShaderTexUnit::mfSetTexture(int State, int nt)
{
  CD3D8Renderer *rd = gcpRendD3D;
  
  if (nt >= 0 && rd->m_TexMan->m_CurStage != nt)
  {
    rd->m_TexMan->m_CurStage = nt;
  }

  SShaderTexUnit *pSTU;
  int nSetID = -1;
  if (m_TexPic && m_TexPic->m_Bind < EFTT_MAX)
  {
    if (m_TexPic->m_Bind == EFTT_LIGHTMAP)
    {
      if (gRenDev->m_RP.m_pCurObject->m_nLMId)
        nSetID = gRenDev->m_RP.m_pCurObject->m_nLMId;
    }
    if (m_TexPic->m_Bind == EFTT_LIGHTMAP_DIR)
    {
      if (gRenDev->m_RP.m_pCurObject->m_nLMDirId)
        nSetID = gRenDev->m_RP.m_pCurObject->m_nLMDirId;
    }
    if (nSetID < 0)
    {
      if (!rd->m_RP.m_pShaderResources || !rd->m_RP.m_pShaderResources->m_Textures[m_TexPic->m_Bind])
      {
        iLog->Log("WARNING: SShaderTexUnit::mfSetTexture: Missed template texture '%s' for shader '%s'\n", gcEf.mfTemplateTexIdToName(m_TexPic->m_Bind), rd->m_RP.m_pShader->GetName());
        pSTU = this;
      }
      else
        pSTU = &rd->m_RP.m_pShaderResources->m_Textures[m_TexPic->m_Bind]->m_TU;
    }
  }
  else
    pSTU = this;

  if (pSTU->m_AnimInfo)
    pSTU->mfUpdate();

  if (pSTU->m_TexPic) 
  {
    if (CRenderer::CV_r_nobindtextures==1)
      rd->SetTexture(0);
    else if (CRenderer::CV_r_nobindtextures==2)
      rd->SetTexture(4096);
    else
    if (nSetID > 0)
		{
			gRenDev->m_TexMan->SetTexture(nSetID, eTT_Base);
		}
    else
    {
      int bind = pSTU->m_TexPic->m_Bind;
      if (bind >= TX_FIRSTBIND)
      {
        pSTU->m_TexPic->Set();
        //pSTU->m_TexPic->SaveJPG("white_ddn.jpg", false);
      }
      else
      switch (bind)
      {
        case TO_FROMRE0:
        case TO_FROMRE1:
        case TO_FROMRE2:
        case TO_FROMRE3:
          {
            if (rd->m_RP.m_pRE)
              bind = rd->m_RP.m_pRE->m_CustomTexBind[bind-TO_FROMRE0];
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
              return false;
            rd->SetTexture(bind, eTT_Base);
          }
          break;

        case TO_FROMLIGHT:
          {
            bool bRes = false;
            if (rd->m_RP.m_nCurLight < rd->m_RP.m_DLights.Num())
            {
              CDLight *dl = rd->m_RP.m_pCurLight;
              if (dl && dl->m_pLightImage)
              {
                bRes = true;
                STexPic *tp = (STexPic *)dl->m_pLightImage;
                if (dl->m_NumCM >= 0)
                  tp = CTexMan::m_CustomCMaps[dl->m_NumCM].m_Tex;
                tp->Set();
              }
            }
            if (!bRes)
              iLog->Log("Warning: Couldn't set projected texture for %d light source (Shader: '%s')\n", rd->m_RP.m_nCurLight, rd->m_RP.m_pShader->m_Name.c_str());
          }
          break;

        case TO_ENVIRONMENT_CUBE_MAP:
          {
            SEnvTexture *cm = NULL;
            cm = gcEf.mfFindSuitableEnvCMap(Vec3d(rd->m_RP.m_pCurObject->m_Matrix[3][0], rd->m_RP.m_pCurObject->m_Matrix[3][1], rd->m_RP.m_pCurObject->m_Matrix[3][2]), true, 0);
            if (cm)
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
            cm = gcEf.mfFindSuitableEnvTex(Pos, Angs, true, 0);
            if (cm)
              cm->m_Tex->Set();
            else
              return false;
          }
          break;

        case TO_TEXTURE_MOTIONBLUR_FIRST:
          {
            State &= 0xf;
            if (!((1<<State) & gcEf.m_NightMapReady))
              return false;
            bind = State + bind;
            rd->SetTexture(bind, pSTU->m_TexPic->m_eTT);
          }
          break;

        default:
          {
            if (bind >= TO_CUSTOM_CUBE_MAP_FIRST && bind <= TO_CUSTOM_CUBE_MAP_LAST)
            {
              SEnvTexture *cm = &CTexMan::m_CustomCMaps[bind-TO_CUSTOM_CUBE_MAP_FIRST];
              if (!cm->m_bReady)
              {
                iLog->Log("Warning: Custom CubeMap %d don't ready\n", bind-TO_CUSTOM_CUBE_MAP_FIRST);
                return false;
              }
              cm->m_Tex->Set();
            }
            else
            if (bind >= TO_CUSTOM_TEXTURE_FIRST && bind <= TO_CUSTOM_TEXTURE_LAST)
            {
              SEnvTexture *cm = &CTexMan::m_CustomTextures[bind-TO_CUSTOM_TEXTURE_FIRST];
              if (!cm->m_bReady)
              {
                iLog->Log("Warning: Custom Texture %d don't ready\n", bind-TO_CUSTOM_TEXTURE_FIRST);
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

  if (m_GTC)
  {
    if (!m_GTC->mfSet(true))
      return 0;
  }
  
  if (m_eColorOp != eCO_NOSET)
    rd->EF_SetColorOp(m_eColorOp);

  return 1;
}

bool SShaderPass::mfSetTextures(bool bEnable)
{
  CD3D8Renderer *rd = gcpRendD3D;
  if (bEnable)
  {
    bool bRes = false;
    int n = rd->m_numtmus;
    for (int i=0; i<m_TUnits.Num() && i<rd->m_numtmus; i++)
    {
      SShaderTexUnit *tl = m_TUnits[i];

      if (tl->mfSetTexture(0, i))
        bRes = true;
      else
				n = Min(n, i);
    }
  	n = Min(n, i);
    CD3D8TexMan::BindNULL(n);
    return bRes;
  }
  else
  {
    for (int i=0; i<m_TUnits.Num() && i<rd->m_numtmus; i++)
    {
      SShaderTexUnit *tl = m_TUnits[i];
      if (tl->m_GTC)
      {
        gcpRendD3D->EF_SelectTMU(i);
        tl->m_GTC->mfSet(false);
      }
    }
  }
  rd->EF_SelectTMU(0);
  return true;
}

//===================================================================================

STexPic *CD3D8TexMan::GetByID(int Id)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it != m_RefTexs.end())
  {
    SRefTex *rt = it->second;
    return rt->m_SrcTex;
  }
  return NULL;
}

SRefTex *CD3D8TexMan::AddToHash(int Id, STexPic *ti)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  SRefTex *rt;
  if (it == m_RefTexs.end())
  {
    rt = new SRefTex;
    m_RefTexs.insert(TTextureMapItor::value_type(Id, rt));
  }
  else
    rt = it->second;

  if (ti)
  {
    STexPicD3D *t = (STexPicD3D *)ti;
    t->m_RefTex = rt;
    rt->m_SrcTex = t;
  }
  return rt;
}

void CD3D8TexMan::RemoveFromHash(int Id, STexPic *ti)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it != m_RefTexs.end())
  {
    SRefTex *rt = it->second;
    IDirect3DBaseTexture8* vt = (IDirect3DBaseTexture8*)rt->m_VidTex;
    if (vt)
      vt->Release();
    delete rt;
    m_RefTexs.erase(Id);
  }
  if (ti)
  {
    STexPicD3D *t = (STexPicD3D *)ti;
    t->m_RefTex = NULL;
  }
}

CD3D8TexMan::~CD3D8TexMan()
{
}


STexPic *CD3D8TexMan::CreateTexture()
{
#ifdef DEBUGALLOC
#undef new
#endif
  return new STexPicD3D;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
}

bool CD3D8TexMan::SetFilter(char *tex)  
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
    {"BILINEAR", D3DTEXF_LINEAR, D3DTEXF_LINEAR,D3DTEXF_POINT},
    {"TRILINEAR", D3DTEXF_LINEAR, D3DTEXF_LINEAR,D3DTEXF_LINEAR},
  };

  m_CurAnisotropic = Clamp(CRenderer::CV_r_texture_anisotropic_level, 1, gcpRendD3D->m_MaxAnisotropyLevel);
  CRenderer::CV_r_texture_anisotropic_level = m_CurAnisotropic;
  if ((gRenDev->GetFeatures() & RFT_ALLOWANISOTROPIC) && m_CurAnisotropic > 1)
    tex = "TRILINEAR";

  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  for (i=0; i<2; i++)
  {
    if (!stricmp(tex, tt[i].name) )
    {
      m_MinFilter = tt[i].typemin;
      m_MagFilter = tt[i].typemag;
      m_MipFilter = tt[i].typemip;

      for (i=0; i<m_Textures.Num(); i++)
      {
        if (m_Textures[i] && m_Textures[i]->m_bBusy && !(m_Textures[i]->m_Flags & FT_NOMIPS))
        {
          STexPicD3D *pTP = (STexPicD3D *)m_Textures[i];
          if (pTP->m_RefTex)
          {
            pTP->m_RefTex->m_MinFilter = GetMinFilter();
            pTP->m_RefTex->m_MagFilter = GetMagFilter();
            pTP->m_RefTex->m_AnisLevel = gcpRendD3D->GetAnisotropicLevel();
          }
        }
      }
      return true;
    }
  }
  
  iLog->Log("Bad texture filter name <%s>\n", tex);
  return false;
}

int CD3D8TexMan::TexSize(int wdt, int hgt, int mode)
{
  switch (mode)
  {
    case D3DFMT_X8R8G8B8:
#ifndef _XBOX
    case D3DFMT_X8L8V8U8:
#endif //_XBOX
      return wdt * hgt * 4;

#ifndef _XBOX
    case D3DFMT_R8G8B8:
      return wdt * hgt * 3;
#endif

    case D3DFMT_A8R8G8B8:
      return wdt * hgt * 4;

    case D3DFMT_A8:
      return wdt * hgt;

    case D3DFMT_A4R4G4B4:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_R5G6B5:
      return wdt * hgt * 2;

    case D3DFMT_DXT1:
    case D3DFMT_DXT3:
    case D3DFMT_DXT5:
      {
        int blockSize = (mode == D3DFMT_DXT1) ? 8 : 16;
        return ((wdt+3)/4)*((hgt+3)/4)*blockSize;
      }

    case D3DFMT_P8:
      return wdt * hgt;

    default:
      assert(0);
      break;

  }
  return 0;
}

void CD3D8TexMan::CalcMipsAndSize(STexPic *ti)
{
  ti->m_nMips = 0;
  ti->m_Size = 0;
  int wdt = ti->m_Width;
  int hgt = ti->m_Height;
  STexPicD3D *tp = (STexPicD3D *)ti;
  int mode = tp->m_RefTex->m_DstFormat;
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
}

void STexPic::SaveTGA(const char *nam, bool bMips)
{
}
void STexPic::SaveJPG(const char *nam, bool bMips)
{
}

void STexPicD3D::SaveTGA(const char *name, bool bMips)
{
  if (!m_RefTex || !m_RefTex->m_VidTex)
    return;
  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
  IDirect3DTexture8 *pID3DTexture = NULL;
  IDirect3DCubeTexture8 *pID3DCubeTexture = NULL;
  if (m_eTT == eTT_Cubemap)
    pID3DCubeTexture = (IDirect3DCubeTexture8*)m_RefTex->m_VidTex;
  else
    pID3DTexture = (IDirect3DTexture8*)m_RefTex->m_VidTex;
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
        WriteTGA(pDst, ddsdDescDest.Width, ddsdDescDest.Height, nm);
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
  if (!m_RefTex || !m_RefTex->m_VidTex)
    return;
  char name[64];
  StripExtension(nam, name);
  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
  IDirect3DTexture8 *pID3DTexture = NULL;
  IDirect3DCubeTexture8 *pID3DCubeTexture = NULL;
  if (m_eTT == eTT_Cubemap)
    pID3DCubeTexture = (IDirect3DCubeTexture8*)m_RefTex->m_VidTex;
  else
    pID3DTexture = (IDirect3DTexture8*)m_RefTex->m_VidTex;
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
      static char* scubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
      for (int CubeSide=0; CubeSide<6; CubeSide++)
      {
        char nm[128];
        sprintf(nm, "%s_%s.jpg", name, scubefaces[CubeSide]);

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
#ifndef _XBOX
        WriteJPG(pDst, ddsdDescDest.Width, ddsdDescDest.Height, nm);
#endif
        // Unlock the texture
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, 0);
      }
    }
    else
    {
      char nm[128];
      sprintf(nm, "%s.jpg", name);
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
#ifndef _XBOX
      WriteJPG(pDst, ddsdDescDest.Width, ddsdDescDest.Height, (char *)nm);
#endif
      // Unlock the texture
      pID3DTexture->UnlockRect(0);
    }
    delete [] pDst;
  }
}

void CD3D8TexMan::D3DCreateVideoTexture(int tgt, byte *src, int wdt, int hgt, D3DFORMAT SrcFormat, D3DFORMAT DstFormat, STexPicD3D *ti, bool bMips, int CubeSide, PALETTEENTRY *pe, int DXTSize)
{
  int i = 0;
  int offset = 0;
  int size;

  LPDIRECT3DSURFACE8 pDestSurf;
  LPDIRECT3DSURFACE8 pSourceSurf;
  
  if (!ti->m_RefTex)
    return;

  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  
  LPDIRECT3DTEXTURE8 pID3DTexture = NULL;
  LPDIRECT3DCUBETEXTURE8 pID3DCubeTexture = NULL;
  LPDIRECT3DTEXTURE8 pID3DSrcTexture = NULL;

  ti->m_RefTex->m_DstFormat = DstFormat;
  ti->m_CubeSide = CubeSide;  
  
  // Create the video texture
  if (tgt == TEXTGT_2D)
  {
    if (ti->m_Flags2 & FT2_RENDERTARGET)
    {
      if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pID3DTexture)))
        return;
      ti->m_RefTex->m_VidTex = pID3DTexture;
      return;
    }
    if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, bMips ? D3DX_DEFAULT : 1, 0, DstFormat, D3DPOOL_MANAGED, &pID3DTexture)))
      return;
  }
  else
  if (tgt == TEXTGT_CUBEMAP)
  {
    if (CubeSide == 0)
    {
      if (ti->m_Flags2 & FT2_RENDERTARGET)
      {
        if( FAILED( h = dv->CreateCubeTexture(wdt, 1, D3DUSAGE_RENDERTARGET,  DstFormat, D3DPOOL_DEFAULT, &pID3DCubeTexture)))
          return;
        ti->m_RefTex->m_VidTex = pID3DCubeTexture;
      }
      else
      if( FAILED( h = D3DXCreateCubeTexture(dv, wdt, bMips ? D3DX_DEFAULT : 1, 0, DstFormat, D3DPOOL_MANAGED, &pID3DCubeTexture)))
        return;
      m_pCurCubeTexture = pID3DCubeTexture;
    }
    else
      pID3DCubeTexture = m_pCurCubeTexture;
    if (ti->m_Flags2 & FT2_RENDERTARGET)
      return;
  }
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
      memcpy((byte *)d3dlr.pBits, &src[offset], size);
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
      ti->m_RefTex->m_VidTex = pID3DCubeTexture;
    else
      ti->m_RefTex->m_VidTex = pID3DTexture;
    return;
  }
  else
  if ((SrcFormat==D3DFMT_P8) && SrcFormat==DstFormat)
  {
#ifndef _XBOX
    HRESULT hr = dv->SetPaletteEntries(m_CurPal, pe);
    ti->m_RefTex->m_Pal = m_CurPal++;
#else
    HRESULT hr = dv->CreatePalette(D3DPALETTE_256, &ti->m_RefTex->m_pPal);
    if (FAILED(hr))
    {
      iLog->Log("Couldn't create palette for texture '%s'\n", ti->m_SearchName);
    }
    else
    {
      D3DCOLOR *pPal;
      ti->m_RefTex->m_pPal->Lock(&pPal, D3DLOCK_NOOVERWRITE);
      for (int i=0; i<256; i++)
      {
        pPal[i] = (pe[i].peFlags<<24) | (pe[i].peRed<<16) | (pe[i].peGreen<<8) | pe[i].peBlue;
      }
      ti->m_RefTex->m_pPal->Unlock();
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
      memcpy((byte *)d3dlr.pBits, src, wdt*hgt);
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
      ti->m_RefTex->m_VidTex = pID3DCubeTexture;
    else
      ti->m_RefTex->m_VidTex = pID3DTexture;
    return;
  }
  else
#ifdef _XBOX
  if ((SrcFormat == D3DFMT_LIN_A8R8G8B8 || SrcFormat == D3DFMT_LIN_X8R8G8B8 || SrcFormat == D3DFMT_A8R8G8B8 || SrcFormat == D3DFMT_X8R8G8B8 || SrcFormat == D3DFMT_X8L8V8U8 || SrcFormat == D3DFMT_LIN_X8L8V8U8) && (DstFormat == D3DFMT_A8R8G8B8 || DstFormat == D3DFMT_X8R8G8B8 || DstFormat == D3DFMT_X8L8V8U8 || DstFormat == D3DFMT_LIN_X8L8V8U8))
#else
  if ((SrcFormat == D3DFMT_A8R8G8B8 || SrcFormat == D3DFMT_X8R8G8B8 || SrcFormat == D3DFMT_X8L8V8U8) && (DstFormat == D3DFMT_A8R8G8B8 || DstFormat == D3DFMT_X8R8G8B8 || DstFormat == D3DFMT_X8L8V8U8))
#endif
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
      
#ifndef _XBOX
      // Lock the texture to copy the image data into the texture
      if (pID3DCubeTexture)
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)CubeSide, i, &d3dlr, NULL, 0);
      else
        h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
      // Copy data to video texture 
      memcpy((byte *)d3dlr.pBits, &src[offset], size);
      // Unlock the system texture
      if (pID3DCubeTexture)
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)CubeSide, i);
      else
        pID3DTexture->UnlockRect(i);
#else

      if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, 1, 0, SrcFormat, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture ) ) )
        return;

      h = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
      // Copy data to src texture
      memcpy((byte *)d3dlr.pBits, &src[offset], size);
      // Unlock the system texture
      pID3DSrcTexture->UnlockRect(0);

      if (pID3DCubeTexture)
        h = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)CubeSide, i, &pDestSurf);
      else
        h = pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
      h = pID3DSrcTexture->GetSurfaceLevel(0, &pSourceSurf);
      h = D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
      SAFE_RELEASE(pDestSurf);
      SAFE_RELEASE(pSourceSurf);

      if (pID3DSrcTexture)
        SAFE_RELEASE(pID3DSrcTexture);
#endif
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
      ti->m_RefTex->m_VidTex = pID3DCubeTexture;
    else
      ti->m_RefTex->m_VidTex = pID3DTexture;
  }
  else
  {
    if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, 1, 0, SrcFormat, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture ) ) )
      return;

#ifndef _XBOX
    if (SrcFormat == D3DFMT_A8)
#else
    if (SrcFormat == D3DFMT_A8 || SrcFormat == D3DFMT_LIN_A8)
#endif
      size = wdt*hgt;
    else
      size = wdt*hgt*4;

    h = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to src texture
    memcpy((byte *)d3dlr.pBits, &src[offset], size);
    // Unlock the system texture
    pID3DSrcTexture->UnlockRect(0);

    if (pID3DCubeTexture)
      h = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)CubeSide, i, &pDestSurf);
    else
      h = pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
    h = pID3DSrcTexture->GetSurfaceLevel(0, &pSourceSurf);
    h = D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
    SAFE_RELEASE(pDestSurf);
    SAFE_RELEASE(pSourceSurf);

    if (pID3DSrcTexture)
      SAFE_RELEASE(pID3DSrcTexture);
  }

  if (bMips && ti->m_nMips <= 1)
  {
    DWORD MipFilter;
    switch (gcpRendD3D->CV_d3d8_texmipfilter)
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
    if (pID3DCubeTexture)
    {
      if (CubeSide == 5)
      {
#ifdef _XBOX
        h = D3DXFilterCubeTexture(pID3DCubeTexture, NULL, 0, MipFilter);
#else
        h = D3DXFilterTexture((LPDIRECT3DTEXTURE8)pID3DCubeTexture, NULL, 0, MipFilter);
#endif
      }
    }
    else
      h = D3DXFilterTexture(pID3DTexture, NULL, 0, MipFilter);
  }

  if (pID3DCubeTexture)
    ti->m_RefTex->m_VidTex = pID3DCubeTexture;
  else
    ti->m_RefTex->m_VidTex = pID3DTexture;
}

void CD3D8TexMan::D3DCompressTexture(int tgt, STexPicD3D *ti, int CubeSide)
{
  D3DSURFACE_DESC desc;
  D3DFORMAT Format;
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  int i;

  LPDIRECT3DTEXTURE8 pID3DSrcTexture = NULL;
  LPDIRECT3DCUBETEXTURE8 pID3DSrcCubeTexture = NULL;
  LPDIRECT3DTEXTURE8 pID3DTexture = NULL;
  LPDIRECT3DCUBETEXTURE8 pID3DCubeTexture = NULL;
  if (tgt == TEXTGT_2D)
    pID3DSrcTexture = (LPDIRECT3DTEXTURE8)ti->m_RefTex->m_VidTex;
  else
  {
    if (CubeSide < 5)
      return;
    pID3DSrcCubeTexture = (LPDIRECT3DCUBETEXTURE8)ti->m_RefTex->m_VidTex;
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
    
  if (pID3DSrcTexture)
  {
    // Create the video texture
    if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, ti->m_RefTex->bMips ? D3DX_DEFAULT : 1, 0, Format, D3DPOOL_MANAGED, &pID3DTexture ) ) )
      return;

    for (i=0; i<(int)pID3DSrcTexture->GetLevelCount(); i++) 
    {
      LPDIRECT3DSURFACE8 pDestSurf;
      LPDIRECT3DSURFACE8 pSourceSurf;
      pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
      pID3DSrcTexture->GetSurfaceLevel(i, &pSourceSurf);
      D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
      SAFE_RELEASE(pDestSurf);
      SAFE_RELEASE(pSourceSurf);  
    }
    ti->m_RefTex->m_VidTex = pID3DTexture;

    SAFE_RELEASE(pID3DSrcTexture);  
  }
  else
  if (pID3DSrcCubeTexture)
  {
    // Create the video texture
    if( FAILED( h = D3DXCreateCubeTexture(dv, hgt, ti->m_RefTex->bMips ? D3DX_DEFAULT : 1, 0, Format, D3DPOOL_MANAGED, &pID3DCubeTexture ) ) )
      return;

    int side;
    for (side=0; side<6; side++)
    {
      for (i=0; i<(int)pID3DSrcCubeTexture->GetLevelCount(); i++) 
      {
        LPDIRECT3DSURFACE8 pDestSurf;
        LPDIRECT3DSURFACE8 pSourceSurf;
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

byte *CD3D8TexMan::GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips)
{
  D3DFORMAT Format, srcFormat;
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  int i;
  D3DLOCKED_RECT d3dlr;

#ifndef _XBOX  
  srcFormat = D3DFMT_A8R8G8B8;
#else
  srcFormat = D3DFMT_LIN_A8R8G8B8;
#endif

  int wdt = ti->m_Width;
  int hgt = ti->m_Height;

  int Size = wdt * hgt * 4;

  LPDIRECT3DTEXTURE8 pID3DSrcTexture = NULL;
  LPDIRECT3DTEXTURE8 pID3DTexture = NULL;
  LPDIRECT3DTEXTURE8 pID3DTempTexture = NULL;

  if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, 1, 0, srcFormat, D3DPOOL_SYSTEMMEM, &pID3DTempTexture ) ) )
    return NULL;
  // Lock the texture to copy the image data into the texture
  h = pID3DTempTexture->LockRect(0, &d3dlr, NULL, 0);
  // Copy data to the texture 
  memcpy(d3dlr.pBits, dst, Size);
  // Unlock the texture
  pID3DTempTexture->UnlockRect(0);

  if( FAILED( h = D3DXCreateTexture(dv, wdt, hgt, bMips ? D3DX_DEFAULT : 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture ) ) )
    return NULL;
  {
    LPDIRECT3DSURFACE8 pDestSurf;
    LPDIRECT3DSURFACE8 pSourceSurf;
    pID3DSrcTexture->GetSurfaceLevel(0, &pDestSurf);
    pID3DTempTexture->GetSurfaceLevel(0, &pSourceSurf);
    D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
    SAFE_RELEASE(pDestSurf);
    SAFE_RELEASE(pSourceSurf);  
  }
  SAFE_RELEASE(pID3DTempTexture);

  if (eF == eIF_DXT1)  
    Format = D3DFMT_DXT1;
  else
  if (eF == eIF_DXT3)  
    Format = D3DFMT_DXT3;
  else
  if (eF == eIF_DXT5)  
    Format = D3DFMT_DXT5;

  DWORD MipFilter;
  switch (gcpRendD3D->CV_d3d8_texmipfilter)
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
    LPDIRECT3DSURFACE8 pDestSurf;
    LPDIRECT3DSURFACE8 pSourceSurf;
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
    memcpy(&data[nOffs], d3dlr.pBits, Size);
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

STexPic *CD3D8TexMan::CopyTexture(const char *name, STexPic *tiSrc, int CubeSide)
{
  return NULL;
}

STexPic *CD3D8TexMan::CreateTexture(const char *name, int wdt, int hgt, int depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1, float fAmount2, int DXTSize, STexPic *tp, int bind, ETEX_Format eTF, const char *szSourceName)
{
  byte *dst1 = NULL;
  int i;
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  LPDIRECT3DTEXTURE8 pID3DTexture;
  pID3DTexture = NULL;
  int DxtBlockSize = 0;
  int DxtOneSize = 0;
  bool bMips;
  int CubeSide = -1;
  PALETTEENTRY pe[256];
  
  int w = ilog2(wdt);
  int h = ilog2(hgt);
  assert (w == wdt && h == hgt);

  if (!tp)
  {
    tp = TextureInfoForName(name, NULL, -1, eTT, flags, flags2, bind);

    tp->m_bBusy = true;
    tp->m_Flags = flags;
    tp->m_Flags2 = flags2;
    tp->m_Bind = TX_FIRSTBIND + tp->m_Id;
    AddToHash(tp->m_Bind, tp);
    tp->m_Height = tp->m_HeightReal = hgt;
    tp->m_Width = tp->m_WidthReal = wdt;
    tp->m_nMips = 0;
    tp->m_ETF = eTF;
    bind = tp->m_Bind;
  }
  if (szSourceName)
    tp->m_SourceName = szSourceName;

  STexPicD3D *ti = (STexPicD3D *)tp;
  if (ti->m_Flags & FT_NOMIPS)
    bMips = false;
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
      ti->m_pData32 = Convert8888_Gray(dst, ti, ti->m_Flags);
    if (ti->m_Flags & FT_NODOWNLOAD)
    {
      if (ti->m_Flags & FT_DXT)
        ti->m_pData32 = ImgConvertDXT_RGBA(dst, ti, DXTSize);
      else
        ti->m_pData32 = dst;
      return ti;
    }

    D3DFORMAT format = D3DFMT_X8R8G8B8;
    D3DFORMAT srcFormat;
    int SizeSrc = 0;
    if (eTF == eTF_8888 || eTF == eTF_RGBA)
    {
      srcFormat = D3DFMT_A8R8G8B8;
      SizeSrc = wdt * hgt * 4;
    }
    else
    if (eTF == eTF_0888 || eTF == eTF_DSDT_MAG)
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
            ds1[i*4+3] = 0;
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
          ds1[i*4+3] = 0;
        }
      }
      dst = dst1;
      SizeSrc = wdt * hgt * 4;
      srcFormat = D3DFMT_X8L8V8U8;
      eTF = eTF_8888;
    }
    else
    if (eTF == eTF_8000)
    {
      SizeSrc = wdt * hgt;
      srcFormat = D3DFMT_A8;
      format = D3DFMT_A8;
      ti->m_Flags |= FT_HASALPHA;
    }

    if (!(ti->m_Flags & FT_DXT) && !ti->m_pPalette)
    {
      if (format != D3DFMT_A8 && !(gRenDev->GetFeatures() & RFT_HWGAMMA) && (!(ti->m_Flags & FT_NOGAMMA) || (ti->m_Flags & FT_NOREMOVE)) )
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

      if (ti->m_Flags & FT_DXT1)
        srcFormat = D3DFMT_DXT1;
      else
      if (ti->m_Flags & FT_DXT3)
        srcFormat = D3DFMT_DXT3;
      else
      if (ti->m_Flags & FT_DXT5)
        srcFormat = D3DFMT_DXT5;
      else
      {
        iLog->Log("Unknown DXT format for texture %s", ti->m_SearchName.c_str());      
        srcFormat = D3DFMT_DXT1;
      }
      SizeSrc = DxtOneSize;
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
        iLog->Log("Unknown DXT format for texture %s", ti->m_SearchName.c_str());      
        format = D3DFMT_DXT1;
      }
      srcFormat = format;
      SizeSrc = DxtOneSize;
    }
    else
    if (format != D3DFMT_A8)
    {
      if (!(ti->m_Flags & FT_HASALPHA) || ti->m_eTT == eTT_Bumpmap || ti->m_eTT == eTT_DSDTBump) // 3 components;
      {
        if (ti->m_eTT == eTT_Bumpmap)
        {
          if (CRenderer::CV_r_texbumpquality == 0)
            format = D3DFMT_X8L8V8U8;
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
            format = D3DFMT_X8L8V8U8;
        }
        else
        if (ti->m_eTT == eTT_DSDTBump)
          format = D3DFMT_X8L8V8U8;
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
    else
      SizeSrc = wdt * hgt;
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
    if (srcFormat == D3DFMT_X8L8V8U8)
      srcFormat = D3DFMT_LIN_X8L8V8U8;
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
      ti->m_RefTex->bMips = bMips;
      if (dst)
        D3DCreateVideoTexture(tgt, dst, wdt, hgt, srcFormat, format, ti, bMips, CubeSide, pe, DXTSize);
      if (ti->m_eTT == eTT_Cubemap)
      {
        ti->m_RefTex->bRepeats = false;
        ti->m_RefTex->m_MinFilter = GetMinFilter();
        ti->m_RefTex->m_MagFilter = GetMagFilter();
        ti->m_RefTex->m_AnisLevel = gcpRendD3D->GetAnisotropicLevel();
      }
      else
      {
        if (ti->m_Flags & FT_CLAMP)
          ti->m_RefTex->bRepeats = false;
        else
          ti->m_RefTex->bRepeats = true;
        ti->m_RefTex->m_MinFilter = GetMinFilter();
        ti->m_RefTex->m_MagFilter = GetMagFilter();
        ti->m_RefTex->m_AnisLevel = gcpRendD3D->GetAnisotropicLevel();
      }
      ti->m_RefTex->m_Type = tgt;
      if (CubeSide == 5)
      {
        for (i=0; i<6; i++)
        {
          if (i == 0)
            m_CurCubeFaces[i]->m_RefTex->m_VidTex = m_pCurCubeTexture;
          else
            m_CurCubeFaces[i]->m_RefTex->m_VidTex = NULL;
        }
      }
      
    } // if (!gIsDedicated)
  }  // if (dst)
//  ti->Set();  

  CD3D8TexMan::CalcMipsAndSize(ti);

  if (ti->m_Flags & FT_NOMIPS)
    ti->m_nMips = 1;
  if (ti->m_eTT == eTT_Cubemap)
  {
    ti->m_Size *= 6;
    if (m_LastCMSide)
      m_LastCMSide->m_NextCMSide = ti;
    if (CubeSide == 5)
      m_LastCMSide = NULL;
    else
      m_LastCMSide = ti;
  }
  if (ti->m_eTT != eTT_Cubemap || !ti->m_CubeSide)
  {
    CTexMan::m_StatsCurTexMem += ti->m_Size;
    ti->Unlink();
    ti->Link(&STexPic::m_Root);
    //sTestStr.AddElem(ti);
  }
  CheckTexLimits();
  if (m_Streamed == 2)
    ti->Unload();

  if (dst1)
    delete [] dst1;

  return ti;
}

//============================================================================

void CD3D8TexMan::BuildMipsSub(byte* src, int wdt, int hgt)
{
}

void CD3D8TexMan::UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal)
{
  STexPicD3D *ti = (STexPicD3D *)pic;
  LPDIRECT3DTEXTURE8 pID3DTexture = NULL;
  LPDIRECT3DCUBETEXTURE8 pID3DCubeTexture = NULL;
  int CubeSide = ti->m_CubeSide;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();
  D3DSURFACE_DESC ddsdDescDest;
  if (ti->m_RefTex->m_Type == TEXTGT_2D)
  {
    pID3DTexture = (LPDIRECT3DTEXTURE8)ti->m_RefTex->m_VidTex;
    assert(pID3DTexture);
  }
  else
  {
    pID3DCubeTexture = (LPDIRECT3DCUBETEXTURE8)ti->m_RefTex->m_VidTex;
    assert(pID3DCubeTexture);
  }
  if (bPal)
  {
    bool bMips;
    if (CD3D8Renderer::CV_d3d8_mipprocedures && !(pic->m_Flags & FT_NOMIPS))
      bMips = true;
    else
      bMips = false;
    if (!ti->m_RefTex->bMips)
      bMips = false;
    ti->m_RefTex->bMips = bMips;
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
      memcpy((byte *)d3dlr.pBits, data, ddsdDescDest.Size);
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
    if (CD3D8Renderer::CV_d3d8_mipprocedures && !(pic->m_Flags & FT_NOMIPS))
      bMips = true;
    else
      bMips = false;
    if (!ti->m_RefTex->bMips)
      bMips = false;
    ti->m_RefTex->bMips = bMips;
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
      memcpy((byte *)d3dlr.pBits, data, ddsdDescDest.Size);
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
void ClearBufferWithQuad(int x2,int y2,int x1,int y1,float fR,float fG,float fB,STexPic *pImage, int Side)
{
  if (pImage)
  {
		if (gRenDev->CV_r_ReplaceCubeMap==1)
			pImage = (STexPic *)gRenDev->EF_LoadTexture("textures/cube_face", 0, 0, eTT_Cubemap);
    gRenDev->Set2DMode(true, 256, 256);
    Vec3d crd[4];
    if (pImage->m_eTT == eTT_Cubemap)
    {
      switch(Side)
      {
        case 0: //posx
          crd[0] = Vec3d(1,1,1);
          crd[1] = Vec3d(1,-1,1);
          crd[2] = Vec3d(1,-1,-1);
          crd[3] = Vec3d(1,1,-1);
          break;
        case 1: //negx
          crd[0] = Vec3d(-1,1,1);
          crd[1] = Vec3d(-1,-1,1);
          crd[2] = Vec3d(-1,-1,-1);
          crd[3] = Vec3d(-1,1,-1);
          break;
        case 2: //posy
          crd[0] = Vec3d(1,1,1);
          crd[1] = Vec3d(-1,1,1);
          crd[2] = Vec3d(-1,1,-1);
          crd[3] = Vec3d(1,1,-1);
          break;
        case 3: //negy
          crd[0] = Vec3d(1,-1,1);
          crd[1] = Vec3d(-1,-1,1);
          crd[2] = Vec3d(-1,-1,-1);
          crd[3] = Vec3d(1,-1,-1);
          break;
        case 4: //posz
          crd[0] = Vec3d(1,1,1);
          crd[1] = Vec3d(-1,1,1);
          crd[2] = Vec3d(-1,-1,1);
          crd[3] = Vec3d(1,-1,1);
          break;
        case 5: //negz
          crd[0] = Vec3d(1,1,-1);
          crd[1] = Vec3d(-1,1,-1);
          crd[2] = Vec3d(-1,-1,-1);
          crd[3] = Vec3d(1,-1,-1);
          break;
      }
    }

    //glDisable(GL_BLEND);  
    gRenDev->EnableBlend(false);
	  //glDisable(GL_STENCIL_TEST);
    //gRenDev->EnableStencilTest(false);
    ((CD3D8Renderer*)gRenDev)->mfGetD3DDevice()->SetRenderState(D3DRS_STENCILENABLE, false);
  	
    //glDisable(GL_DEPTH_TEST);
    gRenDev->EnableDepthTest(false);
    //glDepthMask(1);
    gRenDev->EnableDepthWrites(true);
    //glDisable(GL_CULL_FACE);
    gRenDev->SetCullMode(R_CULL_DISABLE);


    //glEnable(GL_TEXTURE_2D);  		
    gRenDev->EnableTMU(true);
    pImage->Set();
    
    //!!! WARNING !!!
    //glEnable(pImage->m_TargetType);

    ////
    /*
    glBegin(GL_QUADS);
	    glColor3f(fR,fG,fB);

      glTexCoord3f(crd[0][0], crd[0][1], crd[0][2]);
	    glVertex3f(0.0f, 0.0f, 0.0f);				// Top Left

      glTexCoord3f(crd[1][0], crd[1][1], crd[1][2]);
	    glVertex3f(0.0f, 256.0f, 0.0f);				// Bottom Left

      glTexCoord3f(crd[2][0], crd[2][1], crd[2][2]);
	    glVertex3f(256.0f, 256.0f, 0.0f);				// Bottom Right

      glTexCoord3f(crd[3][0], crd[3][1], crd[3][2]);
	    glVertex3f(256.0f, 0.0f, 0.0f);				// Top Right
    glEnd();    
    */

    {

struct SPipeTRVertex_D_3T_w
{
  float      dvSX;
  float      dvSY;
  float      dvSZ;
  float      dvRHW;
  DWORD      dcColor;
  float      dvTU[3];
};




  LPDIRECT3DDEVICE8 dv = ((CD3D8Renderer*)gRenDev)->mfGetD3DDevice();


  CFColor color(fR,fG,fB,1);

  DWORD col = (DWORD(color.a * 255) << 24) | (DWORD(color.r * 255) << 16) |
              (DWORD(color.g * 255)<<8) | DWORD(color.b * 255);

  SPipeTRVertex_D_3T_w *vQuad;

#ifdef _XBOX
  HRESULT hr = ((CD3D8Renderer*)gRenDev)->m_pQuadVB->Lock(0, 0, (BYTE **) &vQuad, 0);
#else
  HRESULT hr = ((CD3D8Renderer*)gRenDev)->m_pQuadVB->Lock(0, 0, (BYTE **) &vQuad, D3DLOCK_DISCARD);
#endif

  float x0 = 0.0f;
  float y0 = 0.0f;
  float x1 = 256.0f;
  float y1 = 256.0f;

  vQuad[0].dvSX = x0;
  vQuad[0].dvSY = y0;
  vQuad[0].dvSZ = 1.0f;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = col;
  vQuad[0].dvTU[0] = crd[0][0];
  vQuad[0].dvTU[1] = crd[0][1];
  vQuad[0].dvTU[2] = crd[0][2];

  vQuad[1].dvSX = x0;
  vQuad[1].dvSY = y1;
  vQuad[1].dvSZ = 1.0f;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = col;
  vQuad[1].dvTU[0] = crd[1][0];
  vQuad[1].dvTU[1] = crd[1][1];
  vQuad[1].dvTU[2] = crd[1][2];
  
  vQuad[2].dvSX = x1;
  vQuad[2].dvSY = y1;
  vQuad[2].dvSZ = 1.0f;
  vQuad[2].dvRHW = 1.0f;
  vQuad[2].dcColor = col;
  vQuad[2].dvTU[0] = crd[2][0];
  vQuad[2].dvTU[1] = crd[2][1];
  vQuad[2].dvTU[2] = crd[2][2];

  vQuad[3].dvSX = x1;
  vQuad[3].dvSY = y0;
  vQuad[3].dvSZ = 1.0f;
  vQuad[3].dvRHW = 1.0f;
  vQuad[3].dcColor = col;
  vQuad[3].dvTU[0] = crd[3][0];
  vQuad[3].dvTU[1] = crd[3][1];
  vQuad[3].dvTU[2] = crd[3][2];

  ((CD3D8Renderer*)gRenDev)->m_pQuadVB->Unlock();

  dv->SetStreamSource(0, ((CD3D8Renderer*)gRenDev)->m_pQuadVB, sizeof(SPipeTRVertex_D_3T_w));
  dv->SetVertexShader((D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)));
  dv->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

  gRenDev->m_nPolygons += 2;
    }

    // !!! WARNING !!!
    //if (pImage->m_eTT == eTT_Cubemap)
    //  glDisable(pImage->m_TargetType);

    gRenDev->Set2DMode(false, 256, 256);

	  //glEnable(GL_TEXTURE_2D);
    gRenDev->EnableTMU(true);
	  //glEnable(GL_CULL_FACE);
    gRenDev->SetCullMode(R_CULL_BACK);
	  //glEnable(GL_DEPTH_TEST);
    gRenDev->EnableDepthTest(true);
    //glClear(GL_DEPTH_BUFFER_BIT);
    gRenDev->ClearDepthBuffer();


    {
      /*int width = 256;
      int height = 256;
      byte *pic = new byte [width * height * 4];
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
      WriteTGA(pic,width,height,"EndCube.tga"); 
      delete [] pic;*/
    }
  }
  else
  {
    //glClearColor(1,0,0,0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glScissor(0, 0, x2, y2);
    //glDisable (GL_SCISSOR_TEST);
    
    // !!! WARNING !!!
    ///gcpOGL->EF_ClearBuffers(true, false);
        
    
    //glScissor(0, 0, gcpOGL->GetWidth(), gcpOGL->GetHeight());
    /*gcpOGL->m_bWasCleared = true;
    glClearColor(gcpOGL->m_vClearColor.x,gcpOGL->m_vClearColor.y,gcpOGL->m_vClearColor.z,0);
    if (gRenDev->GetStencilBpp())
	    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    else
	    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);*/
  }
}

void CD3D8TexMan::ClearBuffer(int Width, int Height, bool bEnd, STexPic *pImage, int Side)
{
}

//===================================================================================

void CD3D8TexMan::DrawCubeSide( const float *angle, Vec3d& Pos, int tex_size, int side, int RendFlags)
{
  if (!iSystem)
    return;

  CRenderer * renderer = gRenDev;
  CCamera tmp_camera = renderer->GetCamera();
  CCamera prevCamera = tmp_camera;

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  tmp_camera.Init(tex_size,tex_size, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  tmp_camera.SetPos(Pos);
  tmp_camera.SetAngle(Vec3d(angle[0], angle[1], angle[2]));
  tmp_camera.Update();

  iSystem->SetViewCamera(tmp_camera);
  gRenDev->SetCamera(tmp_camera);

  gRenDev->SetViewport(0, 0, tex_size, tex_size);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. DrawLowDetail .. (DrawCubeSide %d)\n", side);

  eng->DrawLowDetail(RendFlags);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. End DrawLowDetail .. (DrawCubeSide %d)\n", side);

  iSystem->SetViewCamera(prevCamera);
  gRenDev->SetCamera(prevCamera);
}

bool CD3D8TexMan::ScanEnvironmentCM (const char *name, int size, Vec3d& Pos)
{
  return false;
}

void CD3D8TexMan::ScanEnvironmentCube(SEnvTexture *cm, int RendFlags)
{
  if (cm->m_bInprogress)
    return;

  CRenderer * renderer = gRenDev;
  int tex_size;
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
  while(true)
  {
    if (tex_size*3>renderer->GetWidth() || tex_size*2>renderer->GetHeight())
      tex_size >>= 1;
    else
      break;
  }
  if (tex_size <= 8)
    return;

  int n;
  Vec3d cur_pos;
  static float sAngles[6][5] = 
  {
    {   90, -90, 0,  0, 0 },  //posx
    {   90, 90,  0,  1, 0 },  //negx
    {  180, 180, 0,  2, 0 },  //posy
    {   0, 180,  0,  0, 1 },  //negy
    {   90, 180, 0,  1, 1 },  //posz
    {   90, 0,   0,  2, 1 },  //negz
  };

  cm->m_bInprogress = true;
  int tid = TO_ENVIRONMENT_CUBE_MAP_REAL + cm->m_Id;
  if (!(cm->m_Tex->m_Flags & FT_ALLOCATED) || tex_size != cm->m_TexSize)
  {
    cm->m_Tex->m_Flags |= FT_ALLOCATED | FT_NOMIPS;
    cm->m_Tex->m_Flags &= ~FT_DXT;
    cm->m_Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
    cm->m_Tex->m_nMips = 0;
    cm->m_Tex->m_Width = cm->m_Tex->m_WidthReal = tex_size;
    cm->m_Tex->m_Height = cm->m_Tex->m_HeightReal = tex_size;
    cm->m_TexSize = tex_size;
    byte *data = new byte [tex_size*tex_size*4];
    gRenDev->m_TexMan->CreateTexture(NULL, tex_size, tex_size, 1, FT_NOMIPS, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Cubemap, -1.0f, -1.0f, 0, cm->m_Tex);
    SetTexture(0, eTT_Cubemap);
    delete [] data;
  }

  STexPicD3D *tp = (STexPicD3D *)cm->m_Tex;
  if (!tp->m_RefTex || !tp->m_RefTex->m_VidTex)
    return;
  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DCUBETEXTURE8 pID3DTargetTexture = (LPDIRECT3DCUBETEXTURE8)tp->m_RefTex->m_VidTex;

  gRenDev->EF_SaveDLights();

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
  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;
  Vec3d Pos;
  gRenDev->m_RP.m_pRE->mfCenter(Pos, gRenDev->m_RP.m_pCurObject);
  Pos += cm->m_CamPos;

  for(n=Start; n<End; n++)
  { 
    LPDIRECT3DSURFACE8 pSrcSurf;
    h = pID3DTargetTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)n, 0, &pSrcSurf);
    r->m_pd3dDevice->SetRenderTarget( pSrcSurf, r->m_pZBuffer );
    pSrcSurf->Release();

    D3DCOLOR cColor = D3DRGBA(0.0f,0.0f,0.0f,0.0f);
    // render object
    r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);
    DrawCubeSide( &sAngles[n][0], Pos, tex_size, n, RendFlags);
    if (CRenderer::CV_r_envcmwrite)
    {
      static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
      char str[64];
      int width = tex_size;
      int height = tex_size;
      byte *pic = new byte [width * height * 4];
      //glReadPixels(tex_size*(int)sAngles[n][3], tex_size*(int)sAngles[n][4], width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
      sprintf(str, "Cube_%s.jpg", cubefaces[n]);
#ifndef _XBOX
      WriteJPG(pic, width, height, str); 
#endif
      delete [] pic;

    }
  }
  r->m_pd3dDevice->SetRenderTarget( r->m_pBackBuffer, r->m_pZBuffer );
  CRenderer::CV_r_envcmwrite = 0;

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
  gRenDev->EF_RestoreDLights();
}

void CD3D8TexMan::ScanEnvironmentTexture(SEnvTexture *cm, int RendFlags)
{
  if (cm->m_bInprogress)
    return;

  CRenderer * renderer = gRenDev;
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
  while(true)
  {
    if (tex_size>renderer->GetWidth() || tex_size>renderer->GetHeight())
      tex_size >>= 1;
    else
      break;
  }
  if (tex_size <= 8)
    return;

  Vec3d cur_pos;

  cm->m_bInprogress = true;
  int tid = TO_ENVIRONMENT_TEX_MAP_REAL + cm->m_Id;
  if (!(cm->m_Tex->m_Flags & FT_ALLOCATED) || tex_size != cm->m_TexSize)
  {
    cm->m_Tex->m_Flags |= FT_ALLOCATED | FT_NOMIPS;
    cm->m_Tex->m_Flags &= ~FT_DXT;
    cm->m_Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
    cm->m_TexSize = tex_size;
    cm->m_Tex->m_Width = cm->m_Tex->m_Height = tex_size;
    cm->m_Tex->m_WidthReal = cm->m_Tex->m_HeightReal = tex_size;
    cm->m_Tex->m_nMips = 0;
    byte *data = new byte [tex_size*tex_size*4];
    gRenDev->m_TexMan->CreateTexture(NULL, tex_size, tex_size, 1, FT_NOMIPS, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, cm->m_Tex);
    delete [] data;
  }

  STexPicD3D *tp = (STexPicD3D *)cm->m_Tex;
  if (!tp->m_RefTex || !tp->m_RefTex->m_VidTex)
    return;

  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DTEXTURE8 pID3DTargetTexture = (LPDIRECT3DTEXTURE8)tp->m_RefTex->m_VidTex;
  LPDIRECT3DSURFACE8 pSrcSurf;
  h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  r->m_pd3dDevice->SetRenderTarget( pSrcSurf, r->m_pZBuffer );
  pSrcSurf->Release();

  D3DCOLOR cColor = D3DRGBA(0.0f,0.0f,0.0f,0.0f);
  // render object
  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;

  {
    CCamera tmp_camera = renderer->GetCamera();
    CCamera prevCamera = tmp_camera;

    r->SetViewport(0, 0, tex_size, tex_size);

    I3DEngine *eng = (I3DEngine *)iSystem->GetIProcess();
    eng->DrawLowDetail(0);

    iSystem->SetViewCamera(prevCamera);
    r->SetCamera(prevCamera);    
  }

  r->m_pd3dDevice->SetRenderTarget( r->m_pBackBuffer, r->m_pZBuffer );

  /*  int width = tex_size;
  int height = tex_size;
  byte *pic = new byte [width * height * 3];
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pic);
  CImage::SaveTga(pic,FORMAT_24_BIT,width,height,"TexEnv.tga",false); 
  delete [] pic;*/

  gRenDev->SetViewport(vX, vY, vWidth, vHeight);
  gRenDev->EF_PopFog();

  cm->m_bInprogress = false;
  cm->m_bReady = true;
  cm->m_MaskReady = 1;
  gRenDev->m_RP.m_bDrawToTexture = false;
}

//========================================================================================

void CD3D8TexMan::StartCubeSide(CCObject *obj)
{
}

void CD3D8TexMan::EndCubeSide(CCObject *obj, bool bNeedClear)
{
}

void CD3D8TexMan::DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags)
{
  float Smat[16] = 
  {
    0.5f, 0,   0,   0,
    0,   -0.5f, 0,   0,
    0,   0,   0.5f, 0,
    0.5f, 0.5f, 0.5f, 1.0f
  };

  if (Tex->m_Flags & FT_BUILD)
    return;
  CD3D8Renderer *r = gcpRendD3D;
  float plane[4];
  
  plane[0] = Pl.n[0];
  plane[1] = Pl.n[1];
  plane[2] = Pl.n[2];
  plane[3] = -Pl.d;
  
  Tex->m_Width = sLimitSizeByScreenRes(Tex->m_Width);
  Tex->m_Height = sLimitSizeByScreenRes(Tex->m_Height);

  Tex->m_Flags |= FT_BUILD;
  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    int tex_size = Tex->m_Width;
    byte *data = new byte [tex_size*tex_size*4];
    //memset(data, 0, tex_size*tex_size*4);
    Tex->m_Flags |= FT_NOMIPS | FT_CLAMP;
    Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
    Tex->m_nMips = 0;
    AddToHash(Tex->m_Bind, Tex);
    gRenDev->m_TexMan->CreateTexture(NULL, tex_size, tex_size, 1, FT_NOMIPS | FT_CLAMP, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, Tex);
    delete [] data;
  }
  
  gRenDev->EF_SaveDLights();

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

	float fMinDist = Min(16.0f, eng->GetDistanceToSectorWithWater()); // 16 is half of skybox size
  float fMaxDist = eng->GetMaxViewDistance();

  CCamera tmp_cam = gRenDev->GetCamera();
  CCamera prevCamera = tmp_cam;
  tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  Vec3d pos = tmp_cam.GetPos();
  Vec3d vMirPos = Pl.MirrorPosition(pos);
  Vec3d vOccPos = pos - Pl.n * (0.99f * ((Pl.n | pos) - Pl.d));
  tmp_cam.SetPos(vMirPos);
  tmp_cam.SetOccPos(vOccPos);
  Vec3d ang = tmp_cam.GetAngles();
  ang[0] = -ang[0];
  tmp_cam.SetAngle(ang);
  tmp_cam.Update();  
  
  int TempX, TempY, TempWidth, TempHeight;

  DWORD cColor = D3DRGBA(gRenDev->m_vClearColor[0], gRenDev->m_vClearColor[1], gRenDev->m_vClearColor[2], 0);
#ifdef _XBOX
  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);
#endif //_XBOX

  STexPicD3D *tp = (STexPicD3D *)Tex;
  LPDIRECT3DTEXTURE8 pID3DTargetTexture = (LPDIRECT3DTEXTURE8)tp->m_RefTex->m_VidTex;
  LPDIRECT3DSURFACE8 pSrcSurf;
  h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  h = r->m_pd3dDevice->SetRenderTarget(pSrcSurf, r->m_pZBuffer);
  pSrcSurf->Release();

  gRenDev->GetViewport(&TempX, &TempY, &TempWidth, &TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;

#ifndef _XBOX
  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);
#endif //_XBOX

  // render object
  iSystem->SetViewCamera(tmp_cam);
  gRenDev->SetCamera(tmp_cam);
  
  if (!Tex->m_Matrix)
    Tex->m_Matrix = new float[16];

  float matProj[16], matView[16];
  r->GetModelViewMatrix(matView);
  r->GetProjectionMatrix(matProj);
  Matrix m;
  m = Matrix(Smat);
  D3DXMatrixMultiply((D3DXMATRIX *)&m(0,0), (D3DXMATRIX *)matProj, (D3DXMATRIX *)&m(0,0));
  D3DXMatrixMultiply((D3DXMATRIX *)Tex->m_Matrix, (D3DXMATRIX *)matView, (D3DXMATRIX *)&m(0,0));

  r->m_RP.m_PersFlags |= RBPF_NOCLEARBUF;

  //r->m_pd3dDevice->SetClipPlane( 0, plane);
  //r->m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);
  
  eng->DrawLowDetail(RendFlags);

  /*{
    int tex_size = Tex->m_Width;
    LPDIRECT3DTEXTURE8 pID3DSrcTexture;
    D3DLOCKED_RECT d3dlr;
    h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
    LPDIRECT3DSURFACE8 pDstSurf;
    h = D3DXCreateTexture(r->m_pd3dDevice, tex_size, tex_size, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture );
    h = pID3DSrcTexture->GetSurfaceLevel(0, &pDstSurf);
    h = r->m_pd3dDevice->CopyRects(pSrcSurf, NULL, 0, pDstSurf, NULL);
    h = pID3DSrcTexture->LockRect(0, &d3dlr, NULL, 0);
    // Copy data to the texture 
#ifdef _XBOX
    DWORD *pSrcBits = new DWORD[tex_size*tex_size];
    //XGUnswizzleRect( d3dlr.pBits, tex_size, tex_size, NULL, pSrcBits, d3dlr.Pitch, NULL, 4 );
    //WriteTGA((byte *)pSrcBits, tex_size, tex_size, "d:\\Problem.tga", 32);
    delete [] pSrcBits;
#else
    WriteTGA((byte *)d3dlr.pBits, tex_size, tex_size, "d:\\Problem.tga", 32);
#endif
    // Unlock the texture
    pID3DSrcTexture->UnlockRect(0);
    pSrcSurf->Release();
    pDstSurf->Release();
    SAFE_RELEASE (pID3DSrcTexture);
  }*/

  r->m_RP.m_PersFlags &= ~RBPF_NOCLEARBUF;

  Tex->m_Flags &= ~FT_BUILD;
  //r->m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);

  h = r->m_pd3dDevice->SetRenderTarget( r->m_pBackBuffer, r->m_pZBuffer );

  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);

  gRenDev->SetCamera(prevCamera);
  iSystem->SetViewCamera(prevCamera);

  gRenDev->SetViewport(TempX, TempY, TempWidth, TempHeight);
  gRenDev->m_RP.m_bDrawToTexture = false;
  gRenDev->EF_PopFog();
  gRenDev->EF_RestoreDLights();
}

void CD3D8TexMan::DrawToTextureForRainMap(int Id)
{
}

void CD3D8TexMan::CreateBufRegion(int Width, int Height)
{
  /// !!! WARNING !!!
}

_inline byte mulc(int i1)
{
  //amplify colors a bit
  int r = i1 + (i1/4);

  if (r > 255)
    return 255;
  else
    return r;
}

void CD3D8TexMan::AmplifyGlare(SByteColor *glarepixels, int width, int height)
{
  int i;

  for (i=0; i<width*height; i++)
  {
    if ( glarepixels[i].r > CRenderer::CV_r_glarethreshold || glarepixels[i].g > CRenderer::CV_r_glarethreshold || glarepixels[i].b > CRenderer::CV_r_glarethreshold)
    {
      glarepixels[i].r = mulc(glarepixels[i].r);
      glarepixels[i].g = mulc(glarepixels[i].g);
      glarepixels[i].b = mulc(glarepixels[i].b);
    }
    else
    {
      glarepixels[i].r = 0; //divc(glarepixels[i].r);
      glarepixels[i].g = 0; //divc(glarepixels[i].g);
      glarepixels[i].b = 0; //divc(glarepixels[i].b);
    }
  }
}

void CD3D8TexMan::SmoothGlare(SByteColor *src, int src_w, int src_h, SLongColor *dst)
{
  int y,x;

  for (y=0;y<src_h;y++)
  {
    for (x=0;x<src_w;x++)
    {
      dst->r = src[0].r;
      dst->g = src[0].g;
      dst->b = src[0].b;

      if (x > 0)
      {
        dst->r += dst[-1].r;
        dst->g += dst[-1].g;
        dst->b += dst[-1].b;
      }
      if (y > 0)
      {
        dst->r += dst[-src_w].r;
        dst->g += dst[-src_w].g;
        dst->b += dst[-src_w].b;
      }
      if (x > 0 && y > 0)
      {
        dst->r -= dst[-src_w-1].r;
        dst->g -= dst[-src_w-1].g;
        dst->b -= dst[-src_w-1].b;
      }
      dst++;
      src++;
    }
  }
}

// this is a utility function used by DoBoxBlur below
_inline SLongColor *ReadP(SLongColor *p, int w, int h, int x, int y, int UBits)
{
  if (x < 0)
    x = 0;
  else
  if (x >= w)
    x = w-1;
  if (y < 0)
    y = 0;
  else
  if (y >= h)
    y = h-1;
  return &p[x+(y<<UBits)];
}

void CD3D8TexMan::BlurGlare(SByteColor *src, int src_w, int src_h, SByteColor *dst, SLongColor *p, int boxw, int boxh)
{
  SLongColor *to1, *to2, *to3, *to4;
  int y,x;
  float fmul;

  if (boxw < 0 || boxh < 0)
  {
    memcpy(dst,src,src_w*src_h*4); // deal with degenerate kernel sizes
    return;
  }
  fmul=1.f/((boxw*2+1)*(boxh*2+1));

  int n = 0;
  while (!(src_w & (1<<n)))
    n++;
  int UBits = n;

  for (y=0;y<src_h;y++)
  {
    for (x=0;x<src_w;x++)
    {
      to1 = ReadP(p,src_w,src_h,x+boxw,y+boxh, UBits);
      to2 = ReadP(p,src_w,src_h,x-boxw,y-boxh, UBits);
      to3 = ReadP(p,src_w,src_h,x-boxw,y+boxh, UBits);
      to4 = ReadP(p,src_w,src_h,x+boxw,y-boxh, UBits);

      dst->r = (byte)QRound((to1->r + to2->r - to3->r - to4->r)*fmul);
      dst->g = (byte)QRound((to1->g + to2->g - to3->g - to4->g)*fmul);
      dst->b = (byte)QRound((to1->b + to2->b - to3->b - to4->b)*fmul);

      dst++;
      src++;
    }
  }
}



void CD3D8TexMan::DrawToTextureForGlare(int Id)
{
  //*

  LPDIRECT3DDEVICE8 dv = gcpRendD3D->mfGetD3DDevice();  

  if (!iSystem)
    return;
  
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  CREGlare *pRE = gRenDev->m_RP.m_pREGlare;
  STexPic *Tex = CTexMan::m_Text_Glare;

  if (Tex->m_Flags & FT_BUILD)
    return;

  Tex->m_Flags |= FT_BUILD;

  if (CRenderer::CV_r_glaresize != gRenDev->m_RP.m_GlareSize)
  {
    if (CRenderer::CV_r_glaresize <= 32)
      CRenderer::CV_r_glaresize = 32;
    else
    if (CRenderer::CV_r_glaresize <= 64)
      CRenderer::CV_r_glaresize = 64;
    else
    if (CRenderer::CV_r_glaresize <= 128)
      CRenderer::CV_r_glaresize = 128;
    else
      CRenderer::CV_r_glaresize = 256;

    CRenderer::CV_r_glaresize = sLimitSizeByScreenRes(CRenderer::CV_r_glaresize);
    gRenDev->m_RP.m_GlareSize = CRenderer::CV_r_glaresize;
    Tex->m_Flags &= ~FT_ALLOCATED;
    pRE->m_GlareWidth = pRE->m_GlareHeight = gRenDev->m_RP.m_GlareSize;
    pRE->mfInit();
  }

  int nTexWidth = pRE->m_GlareWidth;
  int nTexHeight = pRE->m_GlareHeight;

  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  int width = 512;
  int height = 512;

  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    /// !!! WARNING !!!
    /*
    Tex->m_Width = Tex->m_WidthReal = nTexWidth;
    Tex->m_Height = Tex->m_HeightReal = nTexHeight;
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    AddToHash(Tex->m_Bind, Tex);

    SetTexture(Tex->m_Bind, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Tex->m_Width, Tex->m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    SetTexture(0, eTT_Base);
    */
  }

  CreateBufRegion(width, height);
  ClearBuffer(width, height, true, NULL, 0);

  gRenDev->EF_SaveDLights();

  CCamera tmp_cam = gRenDev->GetCamera();
  CCamera prevCamera = tmp_cam;
  //tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  //tmp_cam.Update();  

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, width, height );
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;

  eng->SetCamera(tmp_cam,false);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Draw scene to texture for glare ***\n");

  int nDrawFlags = DLD_INDOORS | DLD_ADD_LIGHTSOURCES | DLD_ENTITIES | DLD_PARTICLES | DLD_STATIC_OBJECTS | DLD_FAR_SPRITES | DLD_TERRAIN_FULLRES;
  eng->DrawLowDetail(nDrawFlags);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. End DrawLowDetail .. (DrawToTextureForGlare)\n");

  // Generate texture with mip-maps
  {
    /// !!! WARNING !!!!
    static uint sTexId;
    if (!sTexId)
    {
      /*
      glGenTextures(1, &sTexId);	
      SetTexture(sTexId, eTT_Base);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      */

      byte *data = new byte[width*height*4];
      char name[128];
      sprintf(name, "$AutoGlare_%d", gcpRendD3D->m_TexGenID++);
      STexPic *tp = gcpRendD3D->m_TexMan->CreateTexture(name, width, height, 1, FT_NOMIPS | FT_ALLOCATED, FT2_NODXT, data, eTT_Base, -1.0f, -1.0f, 0, NULL);
      delete [] data;
      sTexId = tp->m_Bind;

      IDirect3DSurface8 * pTar = gcpRendD3D->mfGetBackSurface();
      D3DSURFACE_DESC dc;
      pTar->GetDesc(&dc);
      if(tp->m_RefTex->m_VidTex)
        ((LPDIRECT3DTEXTURE8)tp->m_RefTex->m_VidTex)->Release();
      HRESULT h = D3DXCreateTexture(dv, width, height, 1, D3DUSAGE_DYNAMIC, dc.Format, D3DPOOL_DEFAULT, ((LPDIRECT3DTEXTURE8* )& tp->m_RefTex->m_VidTex ));

    }
    SetTexture(sTexId, eTT_Base);

    ///!!! WARNING !!!
    ///if (SUPPORTS_GL_SGIS_generate_mipmap)
    ///  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    ///glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

    
    {
      //byte *pDst = new byte [width*height*4];
      //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

      //WriteJPG(pDst, width, height, "GlareBig.jpg");
      //delete [] pDst;
    }

    gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);
    ///glMatrixMode(GL_PROJECTION);
    ///glLoadIdentity();
    ///glOrtho(0.0, gRenDev->GetWidth(), 0.0, gRenDev->GetHeight(), -20.0, 0.0);
    gcpRendD3D->m_matProj->LoadIdentity();
    dv->SetTransform(D3DTS_PROJECTION, gcpRendD3D->m_matProj->GetTop());

    ///glMatrixMode(GL_MODELVIEW);
    ///glLoadIdentity();
    gcpRendD3D->m_matView->LoadIdentity();
    dv->SetTransform(D3DTS_VIEW, gcpRendD3D->m_matView->GetTop()); 

    gRenDev->SetCullMode(R_CULL_NONE);

    // Reduce texture size
    {
      /// !!! WARNING !!!
      /*
      glDisable(GL_DEPTH_TEST);
      SetTexture(sTexId, eTT_Base);

//      gRenDev->SetState(GS_NODEPTHTEST);

      glBegin(GL_QUADS);

      glTexCoord2f(0, 0);
      glVertex3f(0, 0, 0);

      glTexCoord2f(1, 0);
      glVertex3f((float)Tex->m_Width, 0, 0);

      glTexCoord2f(1, 1);
      glVertex3f((float)Tex->m_Width, (float)Tex->m_Height, 0);

      glTexCoord2f(0, 1);
      glVertex3f(0, (float)Tex->m_Height, 0);

      glEnd();
      */
    }
    
    /// !!! WARNING !!!
    ///if (SUPPORTS_GL_SGIS_generate_mipmap)
    ///  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
  }

  /// !!! WARNING !!!
  ///glReadPixels (0, 0, Tex->m_Width, Tex->m_Height, GL_RGBA, GL_UNSIGNED_BYTE, pRE->m_pGlarePixels);
  
  
  //WriteJPG((byte *)pRE->m_pGlarePixels, 64, 64, "GlareSmall.jpg");

  if (CRenderer::CV_r_glare == 3)
  {
    CRenderer::CV_r_glare = 1;
#ifndef _XBOX
    WriteJPG((byte *)pRE->m_pGlarePixels, Tex->m_Width, Tex->m_Height, "Glare.jpg");
#endif
  }

  AmplifyGlare(pRE->m_pGlarePixels, Tex->m_Width, Tex->m_Height);
  SmoothGlare(pRE->m_pGlarePixels, Tex->m_Width, Tex->m_Height, pRE->m_pGlareSum);
  BlurGlare(pRE->m_pGlarePixels, Tex->m_Width, Tex->m_Height, pRE->m_pGlareBlurPixels, pRE->m_pGlareSum, CRenderer::CV_r_glareboxsize, CRenderer::CV_r_glareboxsize);

  /// !!! WARNING !!!
  /*
  SetTexture(Tex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Tex->m_Width, Tex->m_Height, GL_RGBA, GL_UNSIGNED_BYTE, pRE->m_pGlareBlurPixels);
  */

  //Tex->SaveJPG("GlareFinal.jpg", false);

  SetTexture(0, eTT_Base);

  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);

  gRenDev->SetCamera(prevCamera);
  iSystem->SetViewCamera(prevCamera);
  ClearBuffer(width, height, true, NULL,0);

  Tex->m_Flags &= ~FT_BUILD;
  gRenDev->m_RP.m_bDrawToTexture = false;
  gRenDev->EF_PopFog();

  gRenDev->EF_RestoreDLights();
  /**/
}


//==================================================================================
// Heat map

_inline void D3DQuad(float wdt, float hgt, CFColor fcol)
{
  DWORD col = fcol.GetTrue();

  CD3D8Renderer *r = gcpRendD3D;
  void *pVertices;
#ifndef _XBOX
  HRESULT hr = r->m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#else //_XBOX
  HRESULT hr = r->m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#endif //_XBOX
  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;

  // Define the quad
  vQuad[0].dvSX = 0.0f;
  vQuad[0].dvSY = 0.0f;
  vQuad[0].dvSZ = 0.0f;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = col;
  vQuad[0].dvTU[0] = 0.0f;
  vQuad[0].dvTU[1] = 0.0f;

  vQuad[1].dvSX = wdt;
  vQuad[1].dvSY = 0.0f;
  vQuad[1].dvSZ = 0.0f;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = col;
  vQuad[1].dvTU[0] = 1.0f;
  vQuad[1].dvTU[1] = 0.0f;

  vQuad[2].dvSX = wdt;
  vQuad[2].dvSY = hgt;
  vQuad[2].dvSZ = 0.0f;
  vQuad[2].dvRHW = 1.0f;
  vQuad[2].dcColor = col;
  vQuad[2].dvTU[0] = 1.0f;
  vQuad[2].dvTU[1] = 1.0f;

  vQuad[3].dvSX = 0.0f;
  vQuad[3].dvSY = hgt;
  vQuad[3].dvSZ = 0.0f;
  vQuad[3].dvRHW = 1.0f;
  vQuad[3].dcColor = col;
  vQuad[3].dvTU[0] = 0.0f;
  vQuad[3].dvTU[1] = 1.0f;

  // We are finished with accessing the vertex buffer
  r->m_pQuadVB->Unlock();
  // Bind our vertex as the first data stream of our device
  r->m_pd3dDevice->SetStreamSource(0, r->m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  r->m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);
  // Render the two triangles from the data stream
  hr = r->m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

void CD3D8TexMan::EndHeatMap()
{
  STexPic *Tex = CTexMan::m_Text_HeatMap;
  CD3D8Renderer *r = gcpRendD3D;

  r->m_RP.m_PersFlags &= ~(RBPF_NOCLEARBUF | RBPF_DRAWHEATMAP);
  Tex->m_Flags &= ~FT_BUILD;

  if (CRenderer::CV_r_heattype == 1)
  {
    int i;
    float wdt = (float)Tex->m_Width;
    float hgt = (float)Tex->m_Height;
    r->EF_SetVertColor();
    r->m_matProj->Push();
    D3DXMATRIX *m = r->m_matProj->GetTop();
	  D3DXMatrixOrthoOffCenterLH(m, 0.0, wdt, 0.0, hgt, -20.0, 0.0);
    r->m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
    r->EF_PushMatrix();
    m = r->m_matView->GetTop();
    r->m_matView->LoadIdentity();
    r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);

    r->SetCullMode(R_CULL_NONE);
    r->SetEnviMode(R_MODE_MODULATE);
    r->SetState(GS_NODEPTHTEST);
    //CTexMan::m_Text_White->Set();
    Tex->Set();

    //D3DQuad(wdt, hgt, CFColor(1,1,1,1));

    r->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
    for (i=0; i<4; i++)
    {
      float fOffs = (i+1)*(Tex->m_Width/1024.0f);
      float fAlpha = 0.1f;

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(fOffs,0,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(-fOffs,0,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(0,fOffs,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(0,-fOffs,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();
    }

    float fRandU = RandomNum() * 2.0f;
    float fRandV = RandomNum() * 2.0f;
    gRenDev->SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_NODEPTHTEST);
    STexPic *tp = gRenDev->m_TexMan->LoadTexture("Textures/Defaults/HeatNoise", 0, 0);
    tp->Set();

    void *pVertices;
#ifndef _XBOX
    HRESULT hr = r->m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#else //_XBOX
    HRESULT hr = r->m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#endif //_XBOX
    SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;
    DWORD col = -1;

    // Define the quad
    vQuad[0].dvSX = 0.0f;
    vQuad[0].dvSY = 0.0f;
    vQuad[0].dvSZ = 0.0f;
    vQuad[0].dvRHW = 1.0f;
    vQuad[0].dcColor = col;
    vQuad[0].dvTU[0] = fRandU;
    vQuad[0].dvTU[1] = fRandV;

    vQuad[1].dvSX = wdt;
    vQuad[1].dvSY = 0.0f;
    vQuad[1].dvSZ = 0.0f;
    vQuad[1].dvRHW = 1.0f;
    vQuad[1].dcColor = col;
    vQuad[1].dvTU[0] = 4.0f+fRandU;
    vQuad[1].dvTU[1] = fRandV;

    vQuad[2].dvSX = wdt;
    vQuad[2].dvSY = hgt;
    vQuad[2].dvSZ = 0.0f;
    vQuad[2].dvRHW = 1.0f;
    vQuad[2].dcColor = col;
    vQuad[2].dvTU[0] = 4.0f+fRandU;
    vQuad[2].dvTU[1] = 4.0f+fRandV;

    vQuad[3].dvSX = 0.0f;
    vQuad[3].dvSY = hgt;
    vQuad[3].dvSZ = 0.0f;
    vQuad[3].dvRHW = 1.0f;
    vQuad[3].dcColor = col;
    vQuad[3].dvTU[0] = fRandU;
    vQuad[3].dvTU[1] = 4.0f+fRandV;

    // We are finished with accessing the vertex buffer
    r->m_pQuadVB->Unlock();
    // Bind our vertex as the first data stream of our device
    r->m_pd3dDevice->SetStreamSource(0, r->m_pQuadVB, STRIDE_TR_D_1T);
    // Set the vertex shader to the FVF fixed function shader
    r->m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);
    // Render the two triangles from the data stream
    hr = r->m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

    r->EF_PopMatrix();
    r->m_matProj->Pop();
    m = r->m_matProj->GetTop();
    r->m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  }

  h = r->m_pd3dDevice->SetRenderTarget(0, r->m_pBackBuffer);
    
  iSystem->SetViewCamera(m_PrevCamera);
  gRenDev->SetCamera(m_PrevCamera);

  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);
  gRenDev->m_RP.m_bDrawToTexture = false;

  if (CRenderer::CV_r_heatmapsave)
  {
    CRenderer::CV_r_heatmapsave = 0;
    Tex->SaveJPG("HeatMap.jpg", false);
  }
}

void CD3D8TexMan::StartHeatMap(int Id)
{
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  STexPic *Tex = CTexMan::m_Text_HeatMap;
  if (Tex->m_Flags & FT_BUILD)
    return;

  CD3D8Renderer *r = gcpRendD3D;

  if (CRenderer::CV_r_heatsize != gRenDev->m_RP.m_HeatSize)
  {
    if (CRenderer::CV_r_heatsize <= 64)
      CRenderer::CV_r_heatsize = 64;
    else
    if (CRenderer::CV_r_heatsize <= 128)
      CRenderer::CV_r_heatsize = 128;
    else
    if (CRenderer::CV_r_heatsize <= 256)
      CRenderer::CV_r_heatsize = 256;
    else
    if (CRenderer::CV_r_heatsize <= 512)
      CRenderer::CV_r_heatsize = 512;

    gRenDev->m_RP.m_HeatSize = CRenderer::CV_r_heatsize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }
  int nTexWidth = gRenDev->m_RP.m_HeatSize;
  int nTexHeight = gRenDev->m_RP.m_HeatSize;

  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  Tex->m_Flags |= FT_BUILD;

  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Width = Tex->m_WidthReal = nTexWidth;
    Tex->m_Height = Tex->m_HeightReal = nTexHeight;
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    byte *data = new byte [Tex->m_Width*Tex->m_Height*4];
    Tex->m_Flags |= FT_NOMIPS;
    Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
    AddToHash(Tex->m_Bind, Tex);
    gRenDev->m_TexMan->CreateTexture(NULL, Tex->m_Width, Tex->m_Height, 1, FT_NOMIPS, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, Tex);
    delete [] data;
  }

  CCamera tmp_cam = gRenDev->GetCamera();
  m_PrevCamera = tmp_cam;
  //tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  //tmp_cam.Update();  

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );

  iSystem->SetViewCamera(tmp_cam);
  gRenDev->SetCamera(tmp_cam);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Draw scene to texture for heat ***\n");

  STexPicD3D *tp = (STexPicD3D *)Tex;
  LPDIRECT3DTEXTURE8 pID3DTargetTexture = (LPDIRECT3DTEXTURE8)tp->m_RefTex->m_VidTex;
  LPDIRECT3DSURFACE8 pSrcSurf;
  h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  h = r->m_pd3dDevice->SetRenderTarget(0, pSrcSurf);
  pSrcSurf->Release();

  DWORD cColor = D3DRGBA(gRenDev->m_vClearColor[0], gRenDev->m_vClearColor[1], gRenDev->m_vClearColor[2], 0);
  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

  r->m_RP.m_PersFlags |= RBPF_NOCLEARBUF | RBPF_DRAWHEATMAP;
  gRenDev->m_RP.m_bDrawToTexture = true;

  //int nDrawFlags = DLD_INDOORS | DLD_ADD_LIGHTSOURCES | DLD_ENTITIES | DLD_PARTICLES | DLD_STATIC_OBJECTS | DLD_FAR_SPRITES | DLD_TERRAIN_FULLRES | DLD_TERRAIN_LIGHT | DLD_TERRAIN_WATER | DLD_NEAR_OBJECTS;
  //eng->DrawLowDetail(nDrawFlags);

}

void CD3D8TexMan::EndNightMap()
{
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  STexPicD3D *Tex = (STexPicD3D *)CTexMan::m_Text_NightVisMap;
  CD3D8Renderer *r = gcpRendD3D;

  r->m_RP.m_PersFlags &= ~(RBPF_NOCLEARBUF | RBPF_DRAWNIGHTMAP);
  Tex->m_Flags &= ~FT_BUILD;
  r->m_RP.m_bDrawToTexture = false;

  if (CRenderer::CV_r_nighttype == 1)
  {
    int i;
    float wdt = (float)Tex->m_Width;
    float hgt = (float)Tex->m_Height;
    r->EF_SetVertColor();
    r->m_matProj->Push();
    D3DXMATRIX *m = r->m_matProj->GetTop();
	  D3DXMatrixOrthoOffCenterLH(m, 0.0, wdt, 0.0, hgt, -20.0, 0.0);
    r->m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
    r->EF_PushMatrix();
    m = r->m_matView->GetTop();
    r->m_matView->LoadIdentity();
    r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);

    r->SetCullMode(R_CULL_NONE);
    r->SetEnviMode(R_MODE_MODULATE);
    r->SetState(GS_NODEPTHTEST);

    LPDIRECT3DSURFACE8 pDstSurf, pSrcSurf;  
    LPDIRECT3DTEXTURE8 pID3DTargetTexture, pID3DTexture;
    pID3DTargetTexture = (LPDIRECT3DTEXTURE8)Tex->m_RefTex->m_VidTex;
    h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
    h = D3DXCreateTexture(r->m_pd3dDevice, Tex->m_Width, Tex->m_Height, 1, 0, (D3DFORMAT)Tex->m_RefTex->m_DstFormat, D3DPOOL_DEFAULT, &pID3DTexture );
    h = pID3DTexture->GetSurfaceLevel(0, &pDstSurf);
    h = r->m_pd3dDevice->CopyRects(pSrcSurf, NULL, 0, pDstSurf, NULL);
    SAFE_RELEASE (pSrcSurf);
    SAFE_RELEASE (pDstSurf);
    h = r->m_pd3dDevice->SetTexture(0, (IDirect3DBaseTexture8*)pID3DTexture);
    r->m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    r->m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    r->m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    r->m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    r->m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

    D3DQuad(wdt, hgt, CFColor(1,1,1,1));

    r->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
    for (i=0; i<0; i++)
    {
      float fOffs = (i+1)*(Tex->m_Width/1024.0f);
      float fAlpha = 0.1f;

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(fOffs,0,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(-fOffs,0,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(0,fOffs,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();

      r->EF_PushMatrix();
      r->m_matView->TranslateLocal(0,-fOffs,0);
      m = r->m_matView->GetTop();
      r->m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
      D3DQuad(wdt, hgt, CFColor(1,1,1,fAlpha));
      r->EF_PopMatrix();
    }

    float fRandU = RandomNum() * 2.0f;
    float fRandV = RandomNum() * 2.0f;
    gRenDev->SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_NODEPTHTEST);
    STexPic *tp = gRenDev->m_TexMan->LoadTexture("Textures/Defaults/HeatNoise", 0, 0);
    tp->Set();

    void *pVertices;
#ifndef _XBOX
    HRESULT hr = r->m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#else //_XBOX
    HRESULT hr = r->m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#endif //_XBOX

    SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;
    DWORD col = -1;

    // Define the quad
    vQuad[0].dvSX = 0.0f;
    vQuad[0].dvSY = 0.0f;
    vQuad[0].dvSZ = 0.0f;
    vQuad[0].dvRHW = 1.0f;
    vQuad[0].dcColor = col;
    vQuad[0].dvTU[0] = fRandU;
    vQuad[0].dvTU[1] = fRandV;

    vQuad[1].dvSX = wdt;
    vQuad[1].dvSY = 0.0f;
    vQuad[1].dvSZ = 0.0f;
    vQuad[1].dvRHW = 1.0f;
    vQuad[1].dcColor = col;
    vQuad[1].dvTU[0] = 4.0f+fRandU;
    vQuad[1].dvTU[1] = fRandV;

    vQuad[2].dvSX = wdt;
    vQuad[2].dvSY = hgt;
    vQuad[2].dvSZ = 0.0f;
    vQuad[2].dvRHW = 1.0f;
    vQuad[2].dcColor = col;
    vQuad[2].dvTU[0] = 4.0f+fRandU;
    vQuad[2].dvTU[1] = 4.0f+fRandV;

    vQuad[3].dvSX = 0.0f;
    vQuad[3].dvSY = hgt;
    vQuad[3].dvSZ = 0.0f;
    vQuad[3].dvRHW = 1.0f;
    vQuad[3].dcColor = col;
    vQuad[3].dvTU[0] = fRandU;
    vQuad[3].dvTU[1] = 4.0f+fRandV;

    // We are finished with accessing the vertex buffer
    r->m_pQuadVB->Unlock();
    // Bind our vertex as the first data stream of our device
    r->m_pd3dDevice->SetStreamSource(0, r->m_pQuadVB, STRIDE_TR_D_1T);
    // Set the vertex shader to the FVF fixed function shader
    r->m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);
    // Render the two triangles from the data stream
    //hr = r->m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

    pID3DTexture->Release();

    r->EF_PopMatrix();
    r->m_matProj->Pop();
    m = r->m_matProj->GetTop();
    r->m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  }

  h = r->m_pd3dDevice->SetRenderTarget(0, r->m_pBackBuffer);
    
  iSystem->SetViewCamera(m_PrevCamera);
  gRenDev->SetCamera(m_PrevCamera);

  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);

  if (CRenderer::CV_r_nightmapsave)
  {
    CRenderer::CV_r_nightmapsave = 0;
    Tex->SaveJPG("NightMap.jpg", false);
  }
}

void CD3D8TexMan::StartNightMap(int Id)
{
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  STexPic *Tex = CTexMan::m_Text_NightVisMap;
  if (Tex->m_Flags & FT_BUILD)
    return;

  CD3D8Renderer *r = gcpRendD3D;

  if (CRenderer::CV_r_nightsize != gRenDev->m_RP.m_NightSize)
  {
    if (CRenderer::CV_r_nightsize <= 64)
      CRenderer::CV_r_nightsize = 64;
    else
    if (CRenderer::CV_r_nightsize <= 128)
      CRenderer::CV_r_nightsize = 128;
    else
    if (CRenderer::CV_r_nightsize <= 256)
      CRenderer::CV_r_nightsize = 256;
    else
    if (CRenderer::CV_r_nightsize <= 512)
      CRenderer::CV_r_nightsize = 512;

    gRenDev->m_RP.m_NightSize = CRenderer::CV_r_nightsize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }
  int nTexWidth = gRenDev->m_RP.m_NightSize;
  int nTexHeight = gRenDev->m_RP.m_NightSize;

  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  Tex->m_Flags |= FT_BUILD;

  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Width = Tex->m_WidthReal = nTexWidth;
    Tex->m_Height = Tex->m_HeightReal = nTexHeight;
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    byte *data = new byte [Tex->m_Width*Tex->m_Height*4];
    Tex->m_Flags |= FT_NOMIPS;
    Tex->m_Flags2 |= FT2_NODXT | FT2_RENDERTARGET;
    Tex->m_Flags &= ~(FT_DXT);
    AddToHash(Tex->m_Bind, Tex);
    gRenDev->m_TexMan->CreateTexture(NULL, Tex->m_Width, Tex->m_Height, 1, FT_NOMIPS, FT2_NODXT | FT2_RENDERTARGET, data, eTT_Base, -1.0f, -1.0f, 0, Tex);    delete [] data;
  }

  CCamera tmp_cam = gRenDev->GetCamera();
  m_PrevCamera = tmp_cam;
  //tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  //tmp_cam.Update();  

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;

  iSystem->SetViewCamera(tmp_cam);
  gRenDev->SetCamera(tmp_cam);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Start Draw scene to texture for heat ***\n");

  STexPicD3D *tp = (STexPicD3D *)Tex;
  LPDIRECT3DTEXTURE8 pID3DTargetTexture = (LPDIRECT3DTEXTURE8)tp->m_RefTex->m_VidTex;
  LPDIRECT3DSURFACE8 pSrcSurf;
  h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  h = r->m_pd3dDevice->SetRenderTarget(0, pSrcSurf);
  pSrcSurf->Release();

  DWORD cColor = D3DRGBA(gRenDev->m_vClearColor[0], gRenDev->m_vClearColor[1], gRenDev->m_vClearColor[2], 0);
  r->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

  r->m_RP.m_PersFlags |= RBPF_NOCLEARBUF | RBPF_DRAWNIGHTMAP;

  //int nDrawFlags = DLD_INDOORS | DLD_ADD_LIGHTSOURCES | DLD_ENTITIES | DLD_PARTICLES | DLD_STATIC_OBJECTS | DLD_FAR_SPRITES | DLD_TERRAIN_FULLRES | DLD_TERRAIN_LIGHT;
  //eng->DrawLowDetail(nDrawFlags);
}

//==================================================================================

void CD3D8TexMan::StartMotionMap(int Id, CCObject *pObject)
{
}

void CD3D8TexMan::EndMotionMap(void)
{
}

void CD3D8TexMan::StartScreenMap(int Id)
{
}
void CD3D8TexMan::EndScreenMap()
{
}

void CD3D8TexMan::DrawFlashBangMap(int Id, int RendFlags, CREFlashBang *pRE)
{
}

// tiago: added
void CD3D8TexMan::StartScreenTexMap(int Id)
{

}

void CD3D8TexMan::EndScreenTexMap()
{

}

//==================================================================================

int TexCallback( const VOID* arg1, const VOID* arg2 )
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


void CD3D8TexMan::Update()
{
  CD3D8Renderer *rd = gcpRendD3D;
  int i;
  char buf[256]="";

  CheckTexLimits();

  if (CRenderer::CV_r_logusedtextures == 1 || CRenderer::CV_r_logusedtextures == 3 || CRenderer::CV_r_logusedtextures == 4)
  {
    FILE *fp = NULL;
    TArray<STexPic *> Texs;
    int Size = 0;
    int PartSize = 0;

    static char *sTexType[] = 
    {
      "Base","Bump","DSDTBump","Cubemap","AutoCubemap","Rectangle"
    };
    static char *sTexFormat[] = 
    {
      "Unknown","Index8","HSV","0888","8888","RGBA","8000","0565","0555","4444","1555","DXT1","DXT3","DXT5","SIGNED_HILO16","SIGNED_HILO8","SIGNED_RGB8","RGB8","DSDT_MAG","DSDT","0088"
    };

    if (CRenderer::CV_r_logusedtextures == 1 || CRenderer::CV_r_logusedtextures == 3)
    {
      for (i=0; i<m_Textures.Num(); i++)
      {
        if (CRenderer::CV_r_logusedtextures == 3 && m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind == CTexMan::m_Text_NoTexture->m_Bind)
        {
          if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
            Texs.AddElem(m_Textures[i]);
        }
        else
          if (CRenderer::CV_r_logusedtextures == 1 && m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != CTexMan::m_Text_NoTexture->m_Bind && (m_Textures[i]->m_Flags2 & FT2_WASLOADED) && !(m_Textures[i]->m_Flags2 & FT2_WASUNLOADED))
          {
            if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
              Texs.AddElem(m_Textures[i]);
          }
      }
      if (CRenderer::CV_r_logusedtextures == 3)
        fp = fxopen("MissingTextures.txt", "w");
      else
        fp = fxopen("UsedTextures.txt", "w");
      fprintf(fp, "*** All loaded textures: ***\n");
      qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
      for (i=0; i<Texs.Num(); i++)
      {
        fprintf(fp, "%d\t\tType: %s\t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size, sTexType[Texs[i]->m_eTT], sTexFormat[Texs[i]->m_ETF], *Texs[i]->m_SearchName);
        Size += Texs[i]->m_Size;
        PartSize += Texs[i]->m_LoadedSize;
      }
      fprintf(fp, "*** Total Size: %d\n\n", Size, PartSize, PartSize);

      Texs.Free();
    }
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != CTexMan::m_Text_NoTexture->m_Bind && m_Textures[i]->m_AccessFrame == rd->GetFrameID())
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
        fprintf(fp, "%.3fKb\t\tType: %s\t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
      else
      {
        sprintf(buf, "%.3fKb  Type: %s  Format: %s  (%s)", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
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
        if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != CTexMan::m_Text_NoTexture->m_Bind)
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
      for (i=0; i<Texs.Num(); i++)
      {
        AllSize += Texs[i]->m_Size;
        if (!Texs[i]->IsStreamed())
          NonStrSize += Texs[i]->m_Size;
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
        AllSizeNM += Texs[i]->m_Size;
        if (!(Texs[i]->m_Flags2 & FT2_WASUNLOADED))
        {
          SizeNM += TexsNM[i]->m_Size;
          if (TexsNM[i]->m_LoadedSize)
            PartSizeNM += TexsNM[i]->m_LoadedSize;
          else
            PartSizeNM += TexsNM[i]->m_Size;
        }
      }
      sprintf(buf, "All texture objects: %d (Size: %.3fMb, NonStreamed: %.3fMb)", Texs.Num(), AllSize/(1024.0f*1024.0f), NonStrSize/(1024.0f*1024.0f));
      rd->TextToScreenColor(4,13, 1,1,0,1, buf);
      sprintf(buf, "All loaded texture objects: %d (All MIPS: %.3fMb, Loaded MIPS: %.3fMb)", nLoaded, Size/(1024.0f*1024.0f), PartSize/(1024.0f*1024.0f));
      rd->TextToScreenColor(4,16, 1,1,0,1, buf);
      sprintf(buf, "All Normal Maps: %d (FullSize: %.3fMb, All MIPS: %.3fMb, Loaded MIPS: %.3fMb)", TexsNM.Num(), AllSizeNM/(1024.0f*1024.0f), SizeNM/(1024.0f*1024.0f), PartSizeNM/(1024.0f*1024.0f));
      rd->TextToScreenColor(4,19, 1,1,0,1, buf);

      Texs.Free();
      for (i=0; i<m_Textures.Num(); i++)
      {
        if (m_Textures[i] && m_Textures[i]->m_bBusy && (m_Textures[i]->m_Flags2 & FT2_WASLOADED) && m_Textures[i]->m_Bind != CTexMan::m_Text_NoTexture->m_Bind && m_Textures[i]->m_AccessFrame == rd->GetFrameID())
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

char * CD3D8Renderer::GetTexturesStatusText()
{
  static char buf[256]="";

  TArray<STexPic *> Texs;
  int i;
  for (i=0; i<CTexMan::m_Textures.Num(); i++)
  {
    if (CTexMan::m_Textures[i] && CTexMan::m_Textures[i]->m_bBusy && CTexMan::m_Textures[i]->m_Bind != CTexMan::m_Text_NoTexture->m_Bind)
      Texs.AddElem(CTexMan::m_Textures[i]);
  }
  qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
  int Size = 0;
  for (i=0; i<Texs.Num(); i++)
  {
    Size += Texs[i]->m_Size;
  }
  sprintf(buf, "All loaded textures: %d (Size: %.3fMb)", Texs.Num(), Size*1e-6);

  Texs.Free();
  for (i=0; i<CTexMan::m_Textures.Num(); i++)
  {
    if (CTexMan::m_Textures[i] && CTexMan::m_Textures[i]->m_bBusy && CTexMan::m_Textures[i]->m_Bind != CTexMan::m_Text_NoTexture->m_Bind && CTexMan::m_Textures[i]->m_AccessFrame == GetFrameID())
      Texs.AddElem(CTexMan::m_Textures[i]);
  }
  qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
  Size = 0;
  for (i=0; i<Texs.Num(); i++)
  {
    Size += Texs[i]->m_Size;
  }
  sprintf(buf, "%s Current textures: %d (Size: %.3fMb)", buf, Texs.Num(), Size*1e-6);

  return buf;
}


//==================================================================================

STexPic *CD3D8Renderer::EF_MakePhongTexture(int Exp)
{
  char name[128];

  sprintf(name, "(Phong_%d)", Exp);
  STexPic *ti = m_TexMan->LoadTexture(name, 0, 0, eTT_Base);
  if (ti->m_Flags & FT_ALLOCATED)
    return ti;
  ti->m_Flags |= FT_ALLOCATED;
  ti->Set();

  double shininess = Exp;
	int imgsize = 256;
	unsigned char * img = new unsigned char[imgsize*imgsize*4];
	unsigned char * ip = img;
	for(int j=0; j<imgsize; j++)
	{
		unsigned char a = (unsigned char)(255.99 * pow(j/(imgsize-1.0), shininess));
		for(int i=0; i<imgsize; i++)
		{
      byte b = (unsigned char)(255.99 * (i/(imgsize-1.0)));
			*ip++ = b;
      *ip++ = b;
      *ip++ = b;
			*ip++ = a;
		}
	}
  LPDIRECT3DTEXTURE8 pPhongTexture;
  if(SUCCEEDED(D3DXCreateTexture(gcpRendD3D->mfGetD3DDevice(), imgsize, imgsize, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pPhongTexture)))
  {
    D3DLOCKED_RECT d3dlr;
    
    // Lock the texture to copy the image data into the texture
    HRESULT hr = pPhongTexture->LockRect(0, &d3dlr, NULL, 0);
    if (SUCCEEDED(hr))
    {
      memcpy((byte *)d3dlr.pBits, img, imgsize*imgsize*4);

      // Unlock the texture
      pPhongTexture->UnlockRect(0);
      hr = D3DXFilterTexture(pPhongTexture, NULL, 0, D3DX_FILTER_LINEAR);
    }
  }
  ti->m_RefTex->m_VidTex = (void *)pPhongTexture;
  ti->m_RefTex->m_MinFilter = m_TexMan->GetMinFilter();
  ti->m_RefTex->m_MagFilter = m_TexMan->GetMagFilter();
  ti->m_RefTex->m_DstFormat = D3DFMT_A8R8G8B8;
  ti->m_RefTex->bRepeats = false;
  ti->m_RefTex->m_AnisLevel = gcpRendD3D->GetAnisotropicLevel();
    
	delete [] img;

  m_TexMan->SetTexture(0, eTT_Base);

  return ti;
}

float CD3D8TexMan::CalcFogVal(float fi, float fj)
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
  float fMin = Min(fi, fj) / -30.0f;
  if (fMin >= 1.0f)
    return 1;
  return (1.0f - fMin) * ff + fMin;
}

void CD3D8TexMan::GenerateFogMaps()
{
  int i, j;
  {
    float fdata[256];
    byte *Data1 = new byte[128*128*4];
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
        float m = (float)(nj*nj + ni*ni);
        float fsq = 1.0f / sqrtf(m);
        int iIndexF = (int)((fsq * m) / 63.0f * 255.0f);
        iIndexF = Clamp(iIndexF, 0, 255);
        int iFog = (int)((1.0f - fdata[iIndexF]) * 255.0f);
        if (!i || i==127 || !j || j==127)
          iFog = 255;
        Data1[j*128*4+i*4+0] = Data1[j*128*4+i*4+1] = Data1[j*128*4+i*4+2] = 255;
        Data1[j*128*4+i*4+3] = (byte)iFog;
      }
    }
    CTexMan::m_Text_Fog = CreateTexture("$Fog", 128, 128, 1, FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT, Data1, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
    delete [] Data1;

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

    //CTexMan::m_Text_Fog_Enter = DownloadTexture("(FogEnter)", 64, 64, FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT, &Data2[0][0][0], eTT_Base, 0, NULL, 0, eTF_8888);
    //CTexMan::m_Text_Fog_Enter->SaveTGA("FogEnter.tga", false);
    CTexMan::m_Text_Fog_Enter = LoadTexture("Textures/FogEnter", FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT);
  }
}

void CD3D8TexMan::GenerateFlareMap()
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
  CTexMan::m_Text_Flare = CreateTexture("$Flare", 32, 4, 1, FT_CLAMP | FT_NOREMOVE, FT2_NODXT, &data[0][0][0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
}

#include "D3DCubemaps.h"

#ifdef _XBOX
static void MakeCubeMapXBox(LPDIRECT3DCUBETEXTURE8 pCubeMap, int dwSize, bool bMips)
{
  // Allocate temp space for swizzling the cubemap surfaces
  DWORD* pSourceBits = new DWORD[dwSize * dwSize];

  // Fill all six sides of the cubemap
  for(DWORD i=0; i<6; i++)
  {
    // Lock the i'th cubemap surface
    LPDIRECT3DSURFACE8 pCubeMapFace;
    pCubeMap->GetCubeMapSurface((D3DCUBEMAP_FACES)i, 0, &pCubeMapFace);

    // Write the RGBA-encoded normals to the surface pixels
    DWORD*      pPixel = pSourceBits;
    D3DXVECTOR3 n;
    FLOAT       w, h;

    for(int y=0; y<dwSize; y++)
    {
      h  = (FLOAT)y / (FLOAT)(dwSize-1);  // 0 to 1
      h  = ( h * 2.0f ) - 1.0f;           // -1 to 1
      
      for(int x = 0; x < dwSize; x++)
      {
        w = (FLOAT)x / (FLOAT)(dwSize-1);   // 0 to 1
        w = ( w * 2.0f ) - 1.0f;            // -1 to 1

        // Calc the normal for this texel
        switch( i )
        {
          case D3DCUBEMAP_FACE_POSITIVE_X:    // +x
              n.x = +1.0;
              n.y = -h;
              n.z = -w;
              break;
              
          case D3DCUBEMAP_FACE_NEGATIVE_X:    // -x
              n.x = -1.0;
              n.y = -h;
              n.z = +w;
              break;
              
          case D3DCUBEMAP_FACE_POSITIVE_Y:    // y
              n.x = +w;
              n.y = +1.0;
              n.z = +h;
              break;
              
          case D3DCUBEMAP_FACE_NEGATIVE_Y:    // -y
              n.x = +w;
              n.y = -1.0;
              n.z = -h;
              break;
              
          case D3DCUBEMAP_FACE_POSITIVE_Z:    // +z
              n.x = +w;
              n.y = -h;
              n.z = +1.0;
              break;
              
          case D3DCUBEMAP_FACE_NEGATIVE_Z:    // -z
              n.x = -w;
              n.y = -h;
              n.z = -1.0;
              break;
        }

        // Store the normal as an RGBA color
        D3DXVec3Normalize( &n, &n );
        D3DCOLOR r = (D3DCOLOR)( ( n.x + 1.0f ) * 127.5f );
        D3DCOLOR g = (D3DCOLOR)( ( n.y + 1.0f ) * 127.5f );
        D3DCOLOR b = (D3DCOLOR)( ( n.z + 1.0f ) * 127.5f );
        D3DCOLOR a = (D3DCOLOR)( 255.0f );
        *pPixel++ = ((a<<24L) + (r<<16L) + (g<<8L) + (b<<0L));
      }
    }
    
    // Swizzle the result into the cubemap face surface
    D3DLOCKED_RECT lock;
    pCubeMapFace->LockRect( &lock, 0, 0L );
    XGSwizzleRect( pSourceBits, 0, NULL, lock.pBits, dwSize, dwSize, NULL, sizeof(DWORD) );
    pCubeMapFace->UnlockRect();

    // Release the cubemap face
    pCubeMapFace->Release();
  }

  // Free temp space
  SAFE_DELETE_ARRAY( pSourceBits );
}
#endif

void CD3D8TexMan::GenerateFuncTextures()
{
  LPDIRECT3DCUBETEXTURE8 pNormCubeTexture;
  LPDIRECT3DCUBETEXTURE8 pLightCubeTexture;
  if (gRenDev->GetFeatures() & RFT_BUMP_DOT3)
  {
    SSingleLight f(32); // TO_LIGHT_CUBE_MAP
    if(SUCCEEDED(D3DXCreateCubeTexture(gcpRendD3D->mfGetD3DDevice(), 64, D3DX_DEFAULT, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pLightCubeTexture)))
    {
      int nl = pLightCubeTexture->GetLevelCount();
      MakeCubeMap<SSingleLight>(f, pLightCubeTexture, 64, true);
    }

    SNormalizeVector norm;  // TO_NORMALIZE_CUBE_MAP
    if(SUCCEEDED(D3DXCreateCubeTexture(gcpRendD3D->mfGetD3DDevice(), 256, D3DX_DEFAULT, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pNormCubeTexture)))
    {
#ifndef _XBOX
      MakeCubeMap<SNormalizeVector>(norm, pNormCubeTexture, 256, false);
#else
      MakeCubeMapXBox(pNormCubeTexture, 256, false);
#endif
    }
    if (!CTexMan::m_Text_NormalizeCMap)
      CTexMan::m_Text_NormalizeCMap = LoadTexture("$NormalizeCMap", FT_NOREMOVE, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_NORMALIZE_CUBE_MAP);
    STexPicD3D *ti = (STexPicD3D *)CTexMan::m_Text_NormalizeCMap;
    ti->m_RefTex->m_VidTex = (void *)pNormCubeTexture;
    ti->m_RefTex->bMips = false;
    ti->m_RefTex->m_DstFormat = D3DFMT_X8R8G8B8;
    ti->m_RefTex->m_Type = TEXTGT_CUBEMAP;
    ti->m_Width = 256;
    ti->m_Height = 256;
    CD3D8TexMan::CalcMipsAndSize(ti);
    ti->m_Size *= 6;
    AddToHash(ti->m_Bind, ti);
    ti->Unlink();
    ti->Link(&STexPic::m_Root);
    CTexMan::m_StatsCurTexMem += ti->m_Size;
    CheckTexLimits();
  }

  GenerateFogMaps();
  GenerateFlareMap();
}

