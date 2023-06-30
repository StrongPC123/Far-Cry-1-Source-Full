/*=============================================================================
  TexMan.cpp : Common Texture manager implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "../CommonRender.h"

#ifdef USE_3DC
#include "../3Dc/CompressorLib.h"
#endif

#ifdef PS2
#include "DXTCImage.h"
#include "S3TC.h"
#endif
#if defined(LINUX)
#include "ILog.h"
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

int CTexMan::m_CurStage;
int CTexMan::m_nCurStages;

STexPic STexPic::m_Root;

char *texdir = "Textures";

CTexMan::CTexMan()
{
  m_bRGBA = true;
  m_MinFilter = 0;
  m_MagFilter = 0;
  m_LastTex = NULL;
  m_CurStage = 0;
  m_Streamed = CRenderer::CV_r_texturesstreaming;
  m_CurTexMaxSize = CRenderer::CV_r_texmaxsize;
  m_CurTexSkyResolution = CRenderer::CV_r_texskyresolution;
  m_CurTexSkyQuality = CRenderer::CV_r_texskyquality;
  m_CurTexResolution = CRenderer::CV_r_texresolution;
#ifdef USE_3DC
  m_CurTexNormalMapCompressed = CRenderer::CV_r_texnormalmapcompressed;
#endif
  m_CurTexQuality = CRenderer::CV_r_texquality;
  m_CurTexBumpResolution = CRenderer::CV_r_texbumpresolution;
  m_CurTexBumpQuality = CRenderer::CV_r_texbumpquality;
  if (!m_FreeTexPoolItems.m_NextFree)
  {
    m_FreeTexPoolItems.m_NextFree = &m_FreeTexPoolItems;
    m_FreeTexPoolItems.m_PrevFree = &m_FreeTexPoolItems;
  }
  m_nTexSizeHistory = 0;
  m_nProcessedTextureID1 = 0;
  m_pProcessedTexture1 = NULL;
  m_nProcessedTextureID2 = 0;
  m_pProcessedTexture2 = NULL;
  m_nPhaseProcessingTextures = 0;
  m_nCustomMip = 0;
}

void CTexMan::Shutdown()
{
  int i;

  if(GetISystem()->GetIRenderer()->GetType()==R_NULL_RENDERER)	// workaround to fix crash when quitting the dedicated server - because the textures are shared
    return;																											// should be fixed soon

  SAFE_RELEASE_FORCE(m_Text_White);
  SAFE_RELEASE_FORCE(m_Text_WhiteShadow);
  SAFE_RELEASE_FORCE(m_Text_WhiteBump);
  SAFE_RELEASE_FORCE(m_Text_Gradient);
  SAFE_RELEASE_FORCE(m_Text_Depth);
  SAFE_RELEASE_FORCE(m_Text_Atten2D);
  SAFE_RELEASE_FORCE(m_Text_Edge);
  SAFE_RELEASE_FORCE(m_Text_NoiseVolumeMap);
  SAFE_RELEASE_FORCE(m_Text_NormalizeCMap);
  SAFE_RELEASE_FORCE(m_Text_EnvLCMap);
  SAFE_RELEASE_FORCE(m_Text_EnvTex);
  SAFE_RELEASE_FORCE(m_Text_EnvScr);
  SAFE_RELEASE_FORCE(m_Text_Glare);
  SAFE_RELEASE_FORCE(m_Text_HeatMap);
  SAFE_RELEASE_FORCE(m_Text_RefractMap);
  SAFE_RELEASE_FORCE(m_Text_WaterMap);
  SAFE_RELEASE_FORCE(m_Text_MotionBlurMap);
  SAFE_RELEASE_FORCE(m_Text_NightVisMap);
  SAFE_RELEASE_FORCE(m_Text_FlashBangMap);
  SAFE_RELEASE_FORCE(m_Text_RainMap);
  SAFE_RELEASE_FORCE(m_Text_LightCMap);
  for (i=0; i<8; i++)
  {
    SAFE_RELEASE_FORCE(m_Text_FromRE[i]);
  }
  SAFE_RELEASE_FORCE(m_Text_FromObj);
  SAFE_RELEASE_FORCE(m_Text_FromLight);
  SAFE_RELEASE_FORCE(m_Text_Fog);
  SAFE_RELEASE_FORCE(m_Text_Fog_Enter);
  SAFE_RELEASE_FORCE(m_Text_VFog);
  SAFE_RELEASE_FORCE(m_Text_Flare);
  SAFE_RELEASE_FORCE(m_Text_Ghost);
  SAFE_RELEASE_FORCE(m_Text_DepthLookup);
  SAFE_RELEASE_FORCE(m_Text_FlashBangFlash);
  SAFE_RELEASE_FORCE(m_Text_ScreenNoise);
  SAFE_RELEASE_FORCE(m_Text_HeatPalete);
  SAFE_RELEASE_FORCE(m_Text_ScreenMap);
  SAFE_RELEASE_FORCE(m_Text_PrevScreenMap);  

  SAFE_RELEASE_FORCE(m_Text_ScreenLuminosityMap);
  SAFE_RELEASE_FORCE(m_Text_ScreenCurrLuminosityMap);
  SAFE_RELEASE_FORCE(m_Text_ScreenLowMap);
  SAFE_RELEASE_FORCE(m_Text_ScreenAvg1x1);
  SAFE_RELEASE_FORCE(m_Text_DofMap); 

  for (i=0; i<MAX_ENVLIGHTCUBEMAPS; i++)
  {
    SAFE_RELEASE_FORCE(m_EnvLCMaps[i].m_Tex);
  }
  for (i=0; i<MAX_ENVCUBEMAPS; i++)
  {
    SAFE_RELEASE_FORCE(m_EnvCMaps[i].m_Tex);
  }
  for (i=0; i<MAX_ENVTEXTURES; i++)
  {
    SAFE_RELEASE_FORCE(m_EnvTexts[i].m_Tex);
  }
  for (i=0; i<16; i++)
  {
    SAFE_RELEASE_FORCE(m_CustomCMaps[i].m_Tex);
  }
  for (i=0; i<16; i++)
  {
    SAFE_RELEASE_FORCE(m_CustomTextures[i].m_Tex);
  }
  SAFE_RELEASE_FORCE(m_Text_Gray);

  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_Gray);
  SAFE_RELEASE_FORCE(m_Text_NoTexture);
}

void CTexMan::ReloadTextures()
{
  for (int i=0; i<m_Textures.Num(); i++)
  {
    STexPic *tp = m_Textures[i];
    if (!tp)
      continue;
    if (!tp->m_bBusy)
      continue;
    if (tp->m_Flags2 & FT2_WASLOADED)
    {
      gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
    }
  }
}

bool CTexMan::IsTextureLoaded(const char *pName)
{
  STexPic *tp = GetByName(pName);
  if (!tp)
    return false;
  return true;
}

STexPic *CTexMan::GetByName(const char *pName)
{
  STexLoaded *tl = NULL;
  CName nmf(pName, eFN_Find);
  if (!nmf.GetIndex())
    return NULL;
  LoadedTexsMapItor it = m_TexsMap.find(nmf.GetIndex());
  if (it != m_TexsMap.end())
  {
    tl = it->second;
    if (tl->m_NumTextures)
      return tl->m_Textures[0];
  }
  return NULL;
}

int STexPic::GetFileNames(char *name0, char *name1, int nLen)
{
  const char *name = strstr(m_SourceName.c_str(), "+norm_");
  if (!name)
  {
    name = strchr(m_SourceName.c_str(), '$');
    if (name)
    {
      const char *name1 = strchr(&name[1], '$');
      if (name1)
      {
        nLen = min(nLen, name1-name-1);
        strncpy(name0, &name[1], nLen);
        name0[nLen] = 0;
        return 1;
      }
    }
    strncpy(name0, m_SourceName.c_str(), nLen);
    return 1;
  }
  int nL = min(nLen, name-m_SourceName.c_str());
  strncpy(name0, m_SourceName.c_str(), nL);
  name0[nL] = 0;
  nL = min(nLen, (int)strlen(&name[6]));
  strncpy(name1, &name[6], nL);
  name1[nL] = 0;

  return 2;
}

void STexPic::Release(int bForce)
{
  //gRenDev->m_TexMan->ValidateTexSize();

  if (m_Bind == TX_FIRSTBIND && m_Id < 4 && m_Id != 0)
  {
    if (bForce == 2)
    {
      RemoveFromSearchHash();
      gRenDev->m_TexMan->m_Textures[m_Id] = NULL;
      if (m_Id != gRenDev->m_TexMan->m_Textures.Num()-1)
        gRenDev->m_TexMan->m_FreeSlots.AddElem(m_Id);

      Unlink();
      delete this;
    }
    return;
  }

  if (!bForce)
  {
    if (m_Flags & FT_NOREMOVE)
      return;
    m_nRefCounter--;
    if (m_nRefCounter)
      return;
  }

  SAFE_DELETE_ARRAY(m_pData);
  SAFE_DELETE_ARRAY(m_pData32);
  SAFE_DELETE_ARRAY(m_Matrix);
  if (m_pPalette)
  {
    if (m_pPalette != &gRenDev->m_TexMan->m_NMPalette[0][0])
      delete [] m_pPalette;
    m_pPalette = NULL;
  }
  SAFE_DELETE_ARRAY(m_p8to24table);
  SAFE_DELETE_ARRAY(m_p15to8table);
  SAFE_DELETE(m_pFuncMap);
  SAFE_DELETE(m_pFileTexMips);
  SAFE_DELETE(m_pSH);

  if (m_pPoolItem)
    m_pPoolItem->LinkFree(&gRenDev->m_TexMan->m_FreeTexPoolItems);
  RemoveFromPool();
  RemoveMips(0);

  if (m_Bind != TX_FIRSTBIND || bForce == 2)
    gRenDev->m_TexMan->RemoveFromHash(m_Bind, this);

  RemoveFromSearchHash();

  ReleaseDriverTexture();

  gRenDev->m_TexMan->m_Textures[m_Id] = NULL;
  gRenDev->m_TexMan->m_FreeSlots.AddElem(m_Id);

  Unlink();

  //gRenDev->m_TexMan->ValidateTexSize();

  delete this;
}

void STexPic::RemoveFromSearchHash()
{
  STexLoaded *tl = m_TL;
  int i;
  for (i=0; i<tl->m_NumTextures; i++)
  {
    if (this == tl->m_Textures[i])
      break;
  }
  if (i != tl->m_NumTextures)
  {
    int j = 0;
    STexPic *tp[8];
    for (i=0; i<tl->m_NumTextures; i++)
    {
      if (this == tl->m_Textures[i])
        continue;
      tp[j] = tl->m_Textures[i];
      j++;
    }
    tl->m_NumTextures = j;
    for (int i=0; i<j; i++)
    {
      tl->m_Textures[i] = tp[i];
    }
  }
  if (!tl->m_NumTextures)
  {
    gRenDev->m_TexMan->m_TexsMap.erase(m_SearchName.GetIndex());
    delete tl;
  }
  else
  {
/*#ifdef _DEBUG
      for (int nn=0; nn<tl->m_NumTextures; nn++)
      {
        for (int nnn=nn+1; nnn<tl->m_NumTextures; nnn++)
        {
          if (tl->m_Textures[nn]->m_eTT == tl->m_Textures[nnn]->m_eTT)
            assert(0);
        }
      }
#endif*/
  }
  m_TL = NULL;
}

void STexPic::AddToSearchHash()
{
  CName nmf = m_SearchName;
  LoadedTexsMapItor it = gRenDev->m_TexMan->m_TexsMap.find(nmf.GetIndex());
  STexLoaded *tl;
  if (it != gRenDev->m_TexMan->m_TexsMap.end())
  {
    tl = it->second;
    int j;
    for (j=0; j<tl->m_NumTextures; j++)
    {
      if (j == 7)
      {
        Warning( VALIDATOR_FLAG_TEXTURE,*nmf,"Too many texture types for name '%s'\n", *nmf);
        j = 6;
        break;
      }
      if (this == tl->m_Textures[j])
        break;
    }
    if (j == tl->m_NumTextures)
    {
      tl->m_NumTextures = j+1;
      tl->m_Textures[j] = this;
    }
    m_TL = tl;
  }
  else
  {
    tl = new STexLoaded;
    tl->m_NumTextures = 1;
    tl->m_Textures[0] = this;
    gRenDev->m_TexMan->m_TexsMap.insert(LoadedTexsMapItor::value_type(nmf.GetIndex(), tl));
    m_TL = tl;
  }
  assert(tl->m_NumTextures < 4);

#ifdef _DEBUG
  for (int nn=0; nn<tl->m_NumTextures; nn++)
  {
    for (int nnn=nn+1; nnn<tl->m_NumTextures; nnn++)
    {
      if (tl->m_Textures[nn]->m_eTT == tl->m_Textures[nnn]->m_eTT)
        assert(0);
    }
  }
#endif
}

//extern TArray<STexPic *> sTestStr;
//extern TArray<STexPic *> sTestTx;

void CTexMan::ValidateTexSize()
{
  return;
  int nSize = 0;
  STexPic *pTP = STexPic::m_Root.m_Prev;
  //sTestTx.Free();
  while (pTP != &STexPic::m_Root)
  {
    if (pTP->m_LoadedSize >= 0)
    {
      if (pTP->m_LoadedSize)
        nSize += pTP->m_LoadedSize;
      else
        nSize += pTP->m_Size;
    }

    //sTestTx.AddElem(pTP);
    pTP = pTP->m_Prev;
  }
  STexPoolItem *pIT = gRenDev->m_TexMan->m_FreeTexPoolItems.m_PrevFree;
  while (pIT != &gRenDev->m_TexMan->m_FreeTexPoolItems)
  {
    nSize += pIT->m_pOwner->m_Size;
    pIT = pIT->m_PrevFree;
  }
  assert(nSize == m_StatsCurTexMem);
}

void sLogTexture (const char *name, int Size)
{
  struct STexLogs
  {
    char Name[256];
  };
  static STexLogs *sTexLogs = NULL;
  static int sNumTexLogs = 0;
  static FILE *fp = NULL;
  if (!sTexLogs)
  {
    sTexLogs = new STexLogs[2048];
    fp = fxopen("UsedTextures.txt", "w");
  }
  int i;
  for (i=0; i<sNumTexLogs; i++)
  {
    if (!stricmp(sTexLogs[i].Name, name))
      break;
  }
  if (i == sNumTexLogs)
  {
    strcpy(sTexLogs[i].Name, name);
    sNumTexLogs++;
    fprintf(fp, "%s (%d)\n", name, Size);
  }
}

#define GL_TEXTURE_2D                       0x0DE1
#define GL_TEXTURE_3D                       0x806F
#define GL_TEXTURE_CUBE_MAP_EXT             0x8513
#define GL_TEXTURE_RECTANGLE_NV             0x84F5

STexPic *CTexMan::TextureInfoForName(const char *nameTex, int numT, byte eTT, uint flags, uint flags2, int bind)
{
  char fullnm[256];
  int i;

	STexPic *ti = NULL; 
	STexPic *tiFound = NULL;
	STexLoaded *tl = NULL;

  if (!nameTex)
  {
    Warning( VALIDATOR_FLAG_TEXTURE,0,"CTexMan::TextureInfoForName: NULL name\n");
    return NULL;
  }
  //iLog->Log("TexInfo: %s", nameTex);

  if (!(flags2 & FT2_DISCARDINCACHE))
    StripExtension(nameTex, fullnm);
  else
    strcpy(fullnm, nameTex);
  ConvertDOSToUnixName(fullnm, fullnm);

  CName nmf = CName(fullnm);
  if (numT>=0)
  {
    i = numT;
    if (!m_Textures[i]->m_bBusy)
      goto create;
    if (nmf == m_Textures[i]->m_SearchName)
      return m_Textures[i];
    if (m_Textures[i]->m_eTT != eTT_Cubemap && m_Textures[i]->m_Bind > TX_FIRSTBIND || m_Textures[i]->m_Bind < 0xf00)
      Warning( VALIDATOR_FLAG_TEXTURE,fullnm,"Error custom texture definition for '%s'\n", fullnm);
    else
    if (m_Textures[i]->m_eTT == eTT_Cubemap)
      m_Textures[i]->m_SearchName = nmf;
    return m_Textures[i];
  }

	if (flags & FT_NODOWNLOAD)
    i = m_Textures.Num();
  else
  {
    LoadedTexsMapItor it = m_TexsMap.find(nmf.GetIndex());
    if (it != m_TexsMap.end())
    {
      tl = it->second;
      for (i=0; i<tl->m_NumTextures; i++)
      {
        STexPic *ti = tl->m_Textures[i];
        if (ti->m_bBusy)
        {
          if ((flags ^ ti->m_Flags) & FT_DYNAMIC)
            continue;
          if ((flags ^ ti->m_Flags) & FT_NORESIZE)
            continue;
          if ((flags ^ ti->m_Flags) & (FT_3DC_A | FT_ALLOW3DC))
            continue;
          if ((flags2 ^ ti->m_Flags2) & (FT2_REPLICATETOALLSIDES | FT2_CHECKFORALLSEQUENCES))
            continue;
          if (eTT != ti->m_eTT)
            continue;
          if (bind > 0 && ti->m_Bind != bind)
            continue;
          ti->m_nRefCounter++;
          return ti;
        }
        else
        if (!tiFound)
          tiFound = ti;
      }
      if (tiFound)
        i = tiFound->m_Id;
      else
        i = m_Textures.Num();
    }
    else
      i = m_Textures.Num();
  }
  if (i == m_Textures.Num())
  {
    int n;
    if (n = m_FreeSlots.Num())
    {
      i = m_FreeSlots[n-1];
      m_FreeSlots.Remove(n-1, 1);
    }
  }

create:
  if (i>=m_Textures.Num() || !m_Textures[i])
  {
    ti = CreateTexture();
    if (i < m_Textures.Num())
      m_Textures[i] = ti;
    else
      m_Textures.AddElem(ti);
  }

  ti = m_Textures[i];
  if (tl)
  {
    int j;
    for (j=0; j<tl->m_NumTextures; j++)
    {
      if (ti == tl->m_Textures[j])
        break;
      if (j == 6)
      {
        assert(0);
        Warning( VALIDATOR_FLAG_TEXTURE,fullnm,"Too many texture types for name '%s'\n", fullnm);
        break;
      }
    }
    if (j == tl->m_NumTextures)
    {
      tl->m_NumTextures = j+1;
      tl->m_Textures[j] = ti;
      assert(tl->m_NumTextures < 8);
    }
  }
  else
  {
    tl = new STexLoaded;
    tl->m_NumTextures = 1;
    tl->m_Textures[0] = ti;
    m_TexsMap.insert(LoadedTexsMapItor::value_type(nmf.GetIndex(), tl));
  }
  ti->m_SearchName = nmf;
  ti->m_Name = fullnm;
  ti->m_SourceName = fullnm;
  ti->m_eTT = (ETexType)eTT;
  ti->m_bBusy = false;
  ti->m_Id = i;
  ti->m_nRefCounter = 1;
  ti->m_TL = tl;
/*#ifdef _DEBUG
  for (int nn=0; nn<tl->m_NumTextures; nn++)
  {
    for (int nnn=nn+1; nnn<tl->m_NumTextures; nnn++)
    {
      if (tl->m_Textures[nn]->m_eTT == tl->m_Textures[nnn]->m_eTT)
        assert(0);
    }
  }
#endif*/

  if (ti->m_eTT == eTT_Cubemap)
    ti->m_TargetType = GL_TEXTURE_CUBE_MAP_EXT;
  else
  if (ti->m_eTT == eTT_Rectangle)
    ti->m_TargetType = GL_TEXTURE_RECTANGLE_NV;
  else
  if (ti->m_eTT == eTT_3D)
    ti->m_TargetType = GL_TEXTURE_3D;
  else
    ti->m_TargetType = GL_TEXTURE_2D;

  return ti;
}

#ifdef WIN64
#pragma warning( push )							//AMD Port
#pragma warning( disable : 4267 )				// conversion from 'size_t' to 'int', possible loss of data
#endif

static inline void sAddTexToArray(TArray<STexPic *>& arrTex, STexPic *tx)
{
  if (tx && tx->m_NextCMSide)
  {
    assert(tx->m_NextCMSide != tx);
    sAddTexToArray(arrTex, tx->m_NextCMSide);
    tx->m_NextCMSide = NULL;
  }
  if (tx)
    arrTex.AddElem(tx);
}

static inline void sRemoveGarbage(STexPic *tx)
{
  int i;
  TArray<STexPic *> arrTexs;
  sAddTexToArray(arrTexs, tx->m_NextCMSide);
  tx->m_NextCMSide = NULL;
  for (i=0; i<arrTexs.Num(); i++)
  {
    arrTexs[i]->Release(false);
  }
}

STexPic *CTexMan::LoadCubeTex(const char *mapname, uint flags, uint flags2, int State, byte eTT, int RState, int Id, int BindId, float fAmount)
{
  static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
  char name[256];
  StripExtension(mapname, name);
  int len = strlen(name);
  if (len > 5)
  {
    for (int i=0; i<6; i++)
    {
      if (!stricmp(&name[len-4], cubefaces[i]))
      {
        if (name[len-5] == '_') 
          len--;
        name[len-4] = 0;
        break;
      }
    }
  }

  int tId = Id;
  int tbId = BindId;
  char cube[256];
  STexPic *tx;
  for (int i=0; i<6; i++)
  {
    sprintf(cube, "%s_%s", name, cubefaces[i]);
    STexPic *ti = NULL;
    if (!(flags2 & FT2_RELOAD) || !(flags2 & FT2_CUBEASSINGLETEXTURE))
      ti = LoadTexture(cube, flags, flags2, eTT, fAmount, fAmount, tbId, tId);
    // if we can't load all 6 sides of cube texture
    // try to create cube-map from one texture (if FT2_FORCECUBEMAP specified)
    if (!ti || (ti->m_Flags & FT_NOTFOUND))
    {
      if (flags2 & FT2_FORCECUBEMAP)
      {
        int nNumSeq = -1;
        if (!(flags2 & FT2_RELOAD))
          ti->Release(0);
        StripExtension(mapname, name);
        int len = strlen(name);
        for (int j=0; j<6; j++)
        {
          if (!stricmp(&name[len-4], cubefaces[j]))
          {
            if (name[len-5] == '_')
              len--;
            name[len-4] = 0;
            break;
          }
		    }
        STexPic *pPrevTex = NULL;
        STexPic *pTX = NULL;
        int n;
        while (true)
        {
          ti = LoadTexture(name, flags, flags2, eTT, fAmount, fAmount, BindId, Id);
          if (ti->m_Flags & FT_NOTFOUND)
          {
            if (pTX)
              return pTX;
            return ti;
          }
          if (ti->m_Flags2 & FT2_WASFOUND)
          {
            if ((ti->m_Flags2 & FT2_REPLICATETOALLSIDES) == (flags2 & FT2_REPLICATETOALLSIDES))
            {
              if (!(flags2 & FT2_CHECKFORALLSEQUENCES))
              {
                ti->m_NextTxt = NULL;
                return ti;
              }
              else
              {
                if (ti->m_NextTxt)
                {
                  if (pPrevTex)
                    pPrevTex->m_NextTxt = ti;
                  return ti;
                }
              }
            }
            else
              m_CurCubemapBind = ti->m_Bind;
          }
          /*else
          {
            if (ti->m_Flags2 & FT2_WASUNLOADED)
              return ti;
          }*/
          tx = ti;
          if (nNumSeq < 0)
          {
            tx->m_Flags2 &= ~FT2_REPLICATETOALLSIDES;
            tx->m_Flags2 |= (flags2 & FT2_REPLICATETOALLSIDES);
            pTX = tx;
          }
          ti->m_Flags2 |= FT2_CUBEASSINGLETEXTURE;
          if (ti->m_Flags & FT_DXT)
          {
            flags2 |= FT2_FORCEDXT;
          }
          if (flags2 & FT2_REPLICATETOALLSIDES)
          {
            tx->m_Flags2 |= FT2_REPLICATETOALLSIDES;
            if (!(tx->m_Flags2 & (FT2_WASUNLOADED | FT2_PARTIALLYLOADED)))
            {
              STexPic *pPrevTex = tx;
              for (int j=1; j<6; j++)
              {
                sprintf(cube, "%s_%s", name, cubefaces[j]);
                ti = gRenDev->m_TexMan->CopyTexture(cube, tx, j);
                if (ti)
                {
                  ti->m_ETF = tx->m_ETF;
                  pPrevTex->m_NextCMSide = ti;
                  pPrevTex = ti;
                }
              }
              sRemoveGarbage(tx);
            }
          }
          else
          {
            if (!(tx->m_Flags2 & (FT2_WASUNLOADED | FT2_PARTIALLYLOADED)))
            {
              int size = ti->m_Width * ti->m_Height * 4;
              byte *data = new byte [size];
              memset(data, 0, size);
              if (ti->m_ETF == eTF_DXT3 || ti->m_ETF == eTF_DXT5)
                flags |= FT_HASALPHA;
              for (int j=1; j<6; j++)
              {
                sprintf(cube, "%s_%s", name, cubefaces[j]);
                ti = gRenDev->m_TexMan->CreateTexture(cube, ti->m_Width, ti->m_Height, 1, flags, flags2, data, eTT_Cubemap, fAmount);
                ti->m_ETF = tx->m_ETF;
              }
              delete [] data;
            }
          }
          if (!(flags2 & FT2_CHECKFORALLSEQUENCES))
          {
            sRemoveGarbage(pTX);
            return pTX;
          }
          char name01[256];
          if (nNumSeq < 0)
          {
            int len = strlen(name);
            n = 1;
            while (isdigit(name[len-n]))
            {
              n++;
            }
            nNumSeq = atoi(&name[len-n+1]);
            name[len-n+1] = 0;
            strcpy(name01, name);
          }
          if (pPrevTex)
            pPrevTex->m_NextTxt = tx;
          pPrevTex = tx;
          nNumSeq++;
          if (n == 2)
            sprintf(name, "%s%01d", name01, nNumSeq);
          else
            sprintf(name, "%s%02d", name01, nNumSeq);
        }
        sRemoveGarbage(pTX);
        return pTX;
      }
      sRemoveGarbage(ti);
      return ti;
    }
    if (!i)
    {
      if (ti->m_Flags2 & FT2_WASFOUND)
        return ti;
      //if (!(flags2 & FT2_NODXT) && !(ti->m_Flags & FT_DXT))
      //  flags2 |= FT2_GENERATEDDS;
      if (!(ti->m_Flags & FT_DXT))
        flags2 |= FT2_NODXT;
      tx = ti;
    }
    if (flags2 & FT2_RELOAD)
    {
      tId = -1;
      tbId = 0;
    }
  }
  //tx->SaveJPG("BugCube", false);
  sRemoveGarbage(tx);
  return tx;
}

#ifdef WIN64
#pragma warning( pop )							//AMD Port
#endif


STexPic *CTexMan::LoadTexture(const char* nameTex, uint flags, uint flags2, byte eTT, float fAmount1, float fAmount2, int bind, int numT)  
{
  int num = 0;
  int i;
  byte *src, *dst, *src1;
  char nametex[256];

  src1 = NULL;
  src = dst = NULL;

  STexPic *ti = TextureInfoForName(nameTex, numT, eTT, flags, flags2, bind);
  /*if (gRenDev->m_TexMan->m_Text_NoTexture)
  {
    char name[256];
    int Tgt = ti->m_TargetType;

    ti->m_bBusy = true;
    ti->m_LoadedSize = 0;
    i = ti->m_Id;
    strcpy(name, *ti->m_SearchName);
    STexLoaded *tl = ti->m_TL;
    if (m_Textures[0])
      *ti = *(m_Textures[0]);
    if (!bind)
      ti->m_Bind = TX_FIRSTBIND + i;
    else
      ti->m_Bind = bind;
    ti->m_eTT = (ETexType)eTT;
    ti->m_TL = tl;
    ti->m_Name = nametex;
    ti->m_SearchName = name;
    ti->m_SourceName = name;
    ti->m_TargetType = Tgt;
    ti->m_nRefCounter = 1;
    ti->m_Flags &= ~FT_NOSTREAM;
    ti->m_Flags2 |= FT2_WASLOADED;
    ti->m_Id = i;
    ti->m_Next = NULL;
    ti->m_Prev = NULL;
    AddToHash(ti->m_Bind, ti);
    m_LastTex = ti;
    return ti;
  }*/

  /*if (ti->m_bBusy && (ti->m_fAmount1 != fAmount1 || ti->m_fAmount2 != fAmount2) && !strstr(nameTex, "_ddn"))
  {
    iLog->Log("Warning: Changed amount in '%s' (Count: %d) from %.3f:%.3f to %.3f:%.3f", nameTex, ti->m_nRefCounter, ti->m_fAmount1, ti->m_fAmount2, fAmount1, fAmount2);
    ti->m_fAmount1 = fAmount1;
    ti->m_fAmount2 = fAmount2;
    flags2 |= FT2_RELOAD;
  }*/
  m_LastTex = ti;
  if (ti->m_bBusy && !(flags2 & FT2_RELOAD))
  {
    ti->m_Flags2 |= FT2_WASFOUND;
    if (ti->m_eTT == eTT_Cubemap && ti->m_CubeSide == 0)
      m_pCurCubeTexture = ti->m_RefTex.m_VidTex;
    return ti;
  }

  ti->m_bBusy = true;
  ti->m_LoadedSize = 0;
  i = ti->m_Id;

  ti->m_Flags = flags;
  ti->m_Flags2 = flags2 & ~(FT2_RELOAD | FT2_WASFOUND);
  if (!bind)
    ti->m_Bind = TX_FIRSTBIND + i;
  else
    ti->m_Bind = bind;

  if (m_Streamed & 1)
  {
    if (LoadFromCache(ti, flags, flags2, NULL, NULL, (ETexType)eTT))
      return ti;
  }

  strcpy(nametex, ti->m_Name.c_str());

  char name[256];

  strcpy(name, *ti->m_SearchName);
  STexLoaded *tl = ti->m_TL;
  STexPic *tmp;
  if (name[0] != '$')
  {
    tmp = LoadFromImage(name, ti->m_Flags, ti->m_Flags2, eTT, bind, ti, fAmount1, fAmount2);
    if (tmp)
      tmp->m_Flags2 |= FT2_WASLOADED;
  }
  else
  {
    tmp = ti;
    AddToHash(ti->m_Bind, ti);
    ti->m_RefTex.m_VidTex = NULL;
    ti->m_ETF = eTF_8888;
  }
  if (tmp)
  {
    m_LastTex = tmp;
    if (CRenderer::CV_r_logusedtextures)
    {
      sLogTexture(*ti->m_SearchName, ti->m_Size);
    }
    return tmp;
  }
  if (!i)
  {
    ti->m_bBusy = true;
    ti->m_nRefCounter = 1;
    ti->m_Flags |= FT_NOTFOUND;
    ti->m_Id = i;
    return ti;
  }
  int Tgt = ti->m_TargetType;
  if (m_Textures[0])
    *ti = *(m_Textures[0]);
  else
    iConsole->Exit("Couldn't find default texture '%s\n", nameTex);
  ti->m_eTT = (ETexType)eTT;
  ti->m_TL = tl;
  ti->m_Name = nametex;
  ti->m_SearchName = name;
  ti->m_SourceName = name;
  ti->m_TargetType = Tgt;
  ti->m_nRefCounter = 1;
  if (i)
    ti->m_Flags &= ~(FT_NOREMOVE | FT_NORESIZE);
  ti->m_Flags |= FT_NOTFOUND;
  ti->m_Flags |= flags;
  //ti->m_Flags &= ~FT_NOSTREAM;
  ti->m_Flags2 &= ~FT2_WASLOADED;
  ti->m_Size = 0;
  ti->m_Id = i;
  ti->m_Next = NULL;
  ti->m_Prev = NULL;
  if (bind)
  {
    ti->m_eTT = (ETexType)eTT;
    ti->m_Bind = bind;
    AddToHash(ti->m_Bind, ti);
    ti->m_RefTex.m_VidTex = NULL;
  }
  m_LastTex = ti;

  return ti;
}

STexPic *CTexMan::UploadImage(CImageFile* im, const char *name, uint flags, uint flags2, byte eTT, int bind, STexPic *ti, float fAmount1, float fAmount2)
{
  byte *dst;
  int i, m;
  byte pal[256][4];
  dst = NULL;
  int DXTSize = 0;
  byte *pix;

  ti->m_bBusy = true;
  if (!bind)
    ti->m_Bind = TX_FIRSTBIND + ti->m_Id;
  else
    ti->m_Bind = bind;
  AddToHash(ti->m_Bind, ti);

  ETEX_Format eTF = eTF_8888;
  ETEX_Format txf = eTF_Unknown;

  ti->m_Width = im->mfGet_width();
  ti->m_Height = im->mfGet_height();
  ti->m_Depth = im->mfGet_depth();
  ti->m_nMips = im->mfGet_numMips();

  ti->m_Flags = flags;
  ti->m_Flags2 = flags2;
  if (im->mfGet_Flags() & FIM_NORMALMAP)
    ti->m_Flags |= FT_HASNORMALMAP | FT_HASMIPS;
  else
  if (im->mfGet_Flags() & FIM_DSDT)
    ti->m_Flags |= FT_HASDSDT | FT_HASMIPS;
  if (ti->m_nMips > 1)
    ti->m_Flags |= FT_HASMIPS;
  else
  if (ti->m_nMips == 1)
    ti->m_Flags |= FT_NOMIPS;

  switch (im->mfGetFormat())
  {
    case eIF_Gif:
    case eIF_Pcx:
      txf = eTF_Index;
      break;

    case eIF_Bmp:
      if (im->mfGet_bps() == 32)
        txf = eTF_8888;
      else
      if (im->mfGet_bps() == 24)
        txf = eTF_0888;
      else
      if (im->mfGet_bps() == 8)
        txf = eTF_Index;
      break;

    case eIF_Jpg:
    case eIF_Tga:
      if (im->mfGet_bps() == 32)
        txf = eTF_8888;
      else
        txf = eTF_0888;
      break;

    case eIF_DXT1:
      txf = eTF_DXT1;
      break;

    case eIF_DXT3:
      txf = eTF_DXT3;
      break;

    case eIF_DXT5:
      txf = eTF_DXT5;
      break;

    case eIF_DDS_RGB8:
      txf = eTF_8888;
      break;

    case eIF_DDS_RGBA8:
      txf = eTF_8888;
      break;

    case eIF_DDS_RGBA4:
      txf = eTF_4444;
      ti->m_Flags |= FT_HASALPHA;
      break;

    case eIF_DDS_DSDT:
      if (im->mfGet_Flags() & FIM_DSDT)
        txf = eTF_DSDT_MAG;
      else
        assert(0);
      break;

    case eIF_DDS_LUMINANCE:
      txf = eTF_8000;
      break;

    case eIF_DDS_SIGNED_RGB8:
      if (im->mfGet_Flags() & FIM_NORMALMAP)
        txf = eTF_SIGNED_RGB8;
      else
        assert(0);
      break;

    case eIF_DDS_SIGNED_HILO8:
      if (im->mfGet_Flags() & FIM_NORMALMAP)
        txf = eTF_SIGNED_HILO8;
      else
        assert(0);
      break;

    case eIF_DDS_SIGNED_HILO16:
      if (im->mfGet_Flags() & FIM_NORMALMAP)
        txf = eTF_SIGNED_HILO16;
      else
        assert(0);
      break;
  }

  if (txf == -1)
    return NULL;

  ti->m_ETF = txf;
  pix = im->mfGet_image();

  switch(txf)
  {
    case eTF_Index:
      {
        SRGBPixel *pPal = im->m_pPal;
        for (i=0; i<256; i++)
        {
          pal[i][0] = pPal[i].red;
          pal[i][1] = pPal[i].green;
          pal[i][2] = pPal[i].blue;
          pal[i][3] = pPal[i].alpha;
        }

        if (ti->m_Flags & FT_DYNAMIC)
        {
          ti->m_pPalette = new SRGBPixel[256];
          byte a = 255;
          if (m_bRGBA)
          {
            for (m=0; m<256; m++)
            {
              *(uint *)(&ti->m_pPalette[m]) = *(uint *)(&pal[m][0]);
              a &= pal[m][3];
            }
          }
          else
          {
            for (m=0; m<256; m++)
            {
              ti->m_pPalette[m].red   = pal[m][2];
              ti->m_pPalette[m].green = pal[m][1];
              ti->m_pPalette[m].blue  = pal[m][0];
              ti->m_pPalette[m].alpha = pal[m][3];
              a &= pal[m][3];
            }
          }
          if (a != 255)
            ti->m_Flags |= FT_HASALPHA;
          ti->m_pData = new byte[ti->m_Width*ti->m_Height];
          cryMemcpy(ti->m_pData, pix, ti->m_Width*ti->m_Height);
          ti->m_Flags |= FT_PALETTED;
        }
        dst = new byte [ti->m_Width*ti->m_Height*4];
        FillBGRA_8to32(dst, pix, pal, ti->m_Width, ti->m_Height, ti);
      }
      break;

    case eTF_0888:
      {
        if (ti->m_Flags & FT_HASMIPS)
        {
          int Size = im->mfGet_ImageSize();
          dst = new byte [Size/3*4];
          FillBGRA_24to32(dst, pix, Size);
        }
        else
          dst = pix;
        if (ti->m_Flags & FT_DYNAMIC)
        {
          int i = 256;
          ti->m_pPalette = new SRGBPixel[i];
          ti->m_pData = new byte[ti->m_Width*ti->m_Height];
          shQuantizeRGB((SRGBPixel *)dst, ti->m_Width*ti->m_Height, ti->m_Width, ti->m_pData, ti->m_pPalette, i, CRenderer::CV_r_texquantizedither ? 1 : 0);
          eTF = eTF_Index;
        }
      }
      break;

    case eTF_8888:
      {
        dst = pix;
        byte aMin = 255;
        byte aMax = 0;
        for (i=0; i<ti->m_Width*ti->m_Height; i++)
        {
          byte a = dst[i*4+3];
          aMin = min(a, aMin);
          aMax = max(a, aMax);
        }
        if (aMax != aMin)
          ti->m_Flags |= FT_HASALPHA;
        if (ti->m_Flags & FT_DYNAMIC)
        {
          int i = 256;
          ti->m_pPalette = new SRGBPixel[i];
          ti->m_pData = new byte[ti->m_Width*ti->m_Height];
          shQuantizeRGB((SRGBPixel *)dst, ti->m_Width*ti->m_Height, ti->m_Width, ti->m_pData, ti->m_pPalette, i, CRenderer::CV_r_texquantizedither ? 1 : 0);
          eTF = eTF_Index;
        }
      }
      break;

    case eTF_4444:
      {
        ti->m_Flags |= FT_HASALPHA;
        dst = pix;
        eTF = eTF_4444;
      }
      break;

    case eTF_RGB8:
    case eTF_SIGNED_RGB8:
    case eTF_SIGNED_HILO8:
    case eTF_SIGNED_HILO16:
      {
        dst = pix;
        if (im->mfGet_numMips() == 1)
          ti->m_Flags |= FT_NOMIPS;
        eTF = eTF_0888;
      }
      break;

    case eTF_DSDT_MAG:
      ti->m_eTT = eTT_DSDTBump;
      eTF = eTF_DSDT_MAG;
      dst = pix;
      break;
    case eTF_8000:
      dst = pix;
      eTF = eTF_8000;
      break;
    case eTF_DXT1:
      DXTSize = im->mfGet_ImageSize();
      dst = pix;
      ti->m_Flags |= FT_DXT1;
      eTF = eTF_DXT1;
      break;
    case eTF_DXT3:
      DXTSize = im->mfGet_ImageSize();
      dst = pix;
      ti->m_Flags |= FT_DXT3;
      eTF = eTF_DXT3;
      break;
    case eTF_DXT5:
      DXTSize = im->mfGet_ImageSize();
      dst = pix;
      ti->m_Flags |= FT_DXT5;
      eTF = eTF_DXT5;
      break;

    default:
      return NULL;
      break;
  }
  TArray<byte> Dst;
#ifdef USE_3DC
  if (txf == eTF_0888 || txf == eTF_8888)
  {
#ifndef NULL_RENDERER
    if (eTT == eTT_Bumpmap && (ti->m_Flags & FT_3DC) && gRenDev->m_bDeviceSupportsComprNormalmaps)
    {
      int i, nMip;
      int wdt = ti->m_Width;
      int hgt = ti->m_Height;
      bool bAllow = true;
      if (gRenDev->m_bDeviceSupportsComprNormalmaps == 1)
      {
        if (CRenderer::CV_r_texnormalmapcompressed == 1 && wdt != hgt)
          bAllow = false;
        if (!CompressTextureATI)
          bAllow = false;
      }
      if (bAllow)
      {
        int Offs = 0;
        for (nMip=0; nMip<ti->m_nMips; nMip++)
        {
          if (wdt <= 0)
            wdt = 1;
          if (hgt <= 0)
            hgt = 1;
          byte *d = &dst[Offs];
          if (gRenDev->m_bDeviceSupportsComprNormalmaps == 1)
          {
            if (!(ti->m_Flags & FT_3DC_A))
            {
              for (i=0; i<wdt*hgt; i++)
              {
                Exchange(d[i*4+1], d[i*4+2]);
              }
            }
            else
            {
              for (i=0; i<wdt*hgt; i++)
              {
                d[i*4+1] = d[i*4+3];
                d[i*4+2] = d[i*4+0];
                d[i*4+3] = 255;
                d[i*4+0] = 255;
              }
            }
            void *outData = NULL;
            DWORD outSize = 0;
            COMPRESSOR_ERROR err = CompressTextureATI(wdt, hgt, FORMAT_ARGB_8888, FORMAT_COMP_ATI2N, d, &outData, &outSize);
            if (err == COMPRESSOR_ERROR_NONE)
            {
              int nDst = Dst.Num();
              Dst.Grow(outSize);
              memcpy(&Dst[nDst], outData, outSize);
              DeleteDataATI(outData);
            }
            else
            {
              ti->m_Flags &= ~FT_3DC;
              break;
            }
          }
          else
          if (gRenDev->m_bDeviceSupportsComprNormalmaps > 1)
          {
            int nDst = Dst.Num();
            Dst.Grow(wdt*hgt*2);
            byte *dst = &Dst[nDst];
            if (!(ti->m_Flags & FT_3DC_A))
            {
              for (i=0; i<wdt*hgt; i++)
              {
                dst[i*2+1] = d[i*4+1] - 128;
                dst[i*2+0] = d[i*4+2] - 128;
              }
            }
            else
            {
              for (i=0; i<wdt*hgt; i++)
              {
                dst[i*2+0] = d[i*4+3] - 128;
                dst[i*2+1] = 0;
              }
            }
          }
          Offs += wdt * hgt * 4;
          wdt >>= 1;
          hgt >>= 1;
        }
        if (Dst.Num())
        {
          dst = &Dst[0];
        }
      }
      else
        ti->m_Flags &= ~FT_3DC;
    }
#endif
  }
#endif

  CreateTexture(NULL, ti->m_Width, ti->m_Height, ti->m_Depth, ti->m_Flags, ti->m_Flags2, dst, ti->m_eTT, fAmount1, fAmount2, DXTSize, ti, ti->m_Bind, eTF);
  //ti->Release(false);

  if (dst && ti->m_pData32 != dst && dst != pix && dst != Dst.Data())
    delete [] dst;

  return ti;
}

ETEX_Format sImageFormat2TexFormat(EImFormat eImF)
{
  switch(eImF)
  {
    case eIF_DDS_RGBA8:
    case eIF_Jpg:
    case eIF_Tga:
    case eIF_Bmp:
      return eTF_8888;
    case eIF_DDS_RGB8:
      return eTF_0888;
    case eIF_DXT1:
      return eTF_DXT1;
    case eIF_DXT3:
      return eTF_DXT3;
    case eIF_DXT5:
      return eTF_DXT5;
  default:
    assert(0);
    return eTF_Unknown;
  }

  return eTF_Unknown;
}

char *gImgExt[][16] = 
{
  {
    ".dds",
    ".tga",
    ".jpg",
    NULL
  },
  {
    ".dds",
    ".tga",
    ".jpg",
    NULL
  },
  {
    ".dds",
    NULL
  }
  ,
  {
    ".pcx",
    ".tga",
    ".jpg",
    NULL
  },
};


STexPic *CTexMan::LoadFromImage (const char *name, uint flags, uint flags2, byte eTT, int bind, STexPic *ti, float fAmount1, float fAmount2)
{
  char nm[2][256];
  char nam[2][256];
  CImageFile* im[2];
  STexPic *tx = NULL;
  char wasloaded[2][256];
  int i;

  wasloaded[0][0] = 0;
  wasloaded[1][0] = 0;
  im[0] = im[1] = NULL;
  nam[0][0] = nam[1][0] = 0;

  strcpy(nm[0], name);
  strlwr(nm[0]);

  ConvertDOSToUnixName(nm[0], nm[0]);

  int nI;
  if (flags & FT_DYNAMIC)
    nI = 3;
  else
  if ((eTT == eTT_Base || eTT == eTT_Cubemap) && (flags2 & FT2_NODXT))
    nI = 1;
  else
    nI = 0;
  char *str;
  int nT = 1;
  char suffix[128];
  if (str=strchr(nm[0], '+'))
  {
    nT++;
    *str = 0;
    str++;
    suffix[0] = '+';
    int ns = 1;
    while (*str != '_')
    {
      suffix[ns++] = *str;
      str++;
    }
    suffix[ns] = '_';
    suffix[ns+1] = 0;
    strcpy(nm[1], &str[1]);
    StripExtension(nm[1], nm[1]);
    StripExtension(nm[0], nm[0]);
  }
  for (i=0; i<nT; i++)
  {
    for (int n=0; gImgExt[nI][n]; n++)
    {
      strcpy(nam[i], nm[i]);
      strcat(nam[i], gImgExt[nI][n]);

      im[i] = NULL;
      im[i] = CImageFile::mfLoad_file(nam[i]);
      if ( !im[i] )
        continue;
      if (n)
        Warning(VALIDATOR_FLAG_TEXTURE, nam[i], "Texture format '%s' is deprecated ('%s') (use .dds instead)", gImgExt[nI][n], nam[i]);
      if (im[i]->mfGet_error() != eIFE_OK || im[i]->mfGetFormat() == eIF_Unknown)
      {
        delete im[i];
        continue;
      }
      if ((im[i]->mfGet_Flags() & FIM_NORMALMAP) && eTT != eTT_Bumpmap)
      {
        delete im[i];
        continue;
      }
      if ((im[i]->mfGet_Flags() & FIM_DSDT) && eTT != eTT_DSDTBump)
      {
        delete im[i];
        continue;
      }
      if (im[i]->mfGetFormat() == eIF_DDS_RGB8 || im[i]->mfGetFormat() == eIF_DDS_DSDT)
      {
        im[i]->m_eFormat = eIF_DDS_RGBA8;
      }


      bool bComp = ((gRenDev->GetFeatures() & RFT_COMPRESSTEXTURE) != 0);
      if ((!bComp && !(flags2 & FT2_FORCEDXT)) || eTT == eTT_Bumpmap || eTT == eTT_DSDTBump)
      {
        if (!(im[i]->mfGet_Flags() & (FIM_NORMALMAP | FIM_DSDT)) && (im[i]->mfGetFormat() == eIF_DXT1 || im[i]->mfGetFormat() == eIF_DXT3 || im[i]->mfGetFormat() == eIF_DXT5))
        {
          strcpy(wasloaded[i], nam[i]);
          delete im[i];

          //Warning( 0,0,"Warning: CTexMan::LoadFromImage: Texture skipped because it's compressed: %s [Ext=%s]", name, gImgExt[nI][n]);

          continue;
        }
      }
      wasloaded[i][0] = 0;
      break;
    }
  }
  for (i=0; i<nT; i++)
  {
    if (wasloaded[i][0])
		{
#if !defined(NULL_RENDERER)
#ifdef WIN64
			Warning( VALIDATOR_FLAG_TEXTURE,wasloaded[i],"Error: CTexMan::LoadFromImage: Generate normal map from compressed texture: '%s', "
				"Loading of compressed bump-maps is not supported. "
				"If you want the engine to compress it please use _ddn_cct postfix in texture name", 
				wasloaded[i]);
			return LoadTexture("Textures/white_ddn", 0,0); 
#endif
#endif
      STexPic tp;
      im[i] = CImageFile::mfLoad_file(wasloaded[i]);
      if ( !im[i] )
        return NULL;
      strcpy(nam[i], wasloaded[i]);
      tp.m_Width = im[i]->mfGet_width();
      tp.m_Height = im[i]->mfGet_height();
      tp.m_Flags = 0;
      switch (im[i]->mfGetFormat())
      {
        case eIF_DXT1:
          tp.m_Flags |= FT_DXT1;
          break;
        case eIF_DXT3:
          tp.m_Flags |= FT_DXT3;
          break;
        case eIF_DXT5:
          tp.m_Flags |= FT_DXT5;
          break;
      }
#if !defined(NULL_RENDERER)
      Warning( VALIDATOR_FLAG_TEXTURE,wasloaded[i],"Error: CTexMan::LoadFromImage: Generate normal map from compressed texture: '%s', "
				"Loading of compressed bump-maps is not supported. "
				"If you want the engine to compress it please use _ddn_cct postfix in texture name", 
				wasloaded[i]);
#endif
      byte *data_ = ImgConvertDXT_RGBA(im[i]->mfGet_image(), &tp, im[i]->mfGet_ImageSize());
      SAFE_DELETE_ARRAY (im[i]->m_pByteImage);

      //Watch out this is a BIG problem on PS2
      //During new/delete the data MUST be the same.
      //That's the case where mspImage "is casted" from byte to
      //SRGBPixel causing a memory bug on PS2
      //This is a workaround... maybe there's a better elegant solution      
      SRGBPixel *pNewImage = new SRGBPixel[tp.m_Width*tp.m_Height];
      for(int j=0; j<tp.m_Width*tp.m_Height; j++)          
      {
        pNewImage[j].red=data_[j*4+0];
        pNewImage[j].green=data_[j*4+1];
        pNewImage[j].blue=data_[j*4+2];
        pNewImage[j].alpha=data_[j*4+3];
      }
      im[i]->m_pPixImage = pNewImage;

      delete [] data_;
      
      im[i]->m_eFormat = eIF_Tga;
			im[i]->mfSet_ImageSize(tp.m_Width*tp.m_Height*4);
			im[i]->mfSet_numMips(1);
      im[i]->m_Bps = 32;
    }
  }
  if (im[0] && im[1])
  {
    char str[256];
    sprintf(str, "%s%s%s", nam[0], suffix, nam[1]);
    ti->m_SourceName = str;
  }
  else
  if (im[0])
    ti->m_SourceName = nam[0];
  else
  if (im[1])
  {
    ti->m_SourceName = nam[1];
    im[0] = im[1];
    im[1] = NULL;
  }
  
  if (im[0] && (eTT == eTT_Bumpmap || eTT == eTT_DSDTBump))
  {
    flags &= ~FT_NOMIPS;
    GenerateNormalMap(im, flags, flags2, eTT, fAmount1, fAmount2, ti);
  }

  if (!(flags & FT_DYNAMIC))
    ImagePreprocessing(im[0], flags, flags2, eTT, ti);

#ifndef NULL_RENDERER
  if (im[0] && !ti->m_pSH && eTT == eTT_Bumpmap && im[0]->mfGetFormat()!=eIF_DDS_RGB8 && (strstr(ti->m_SourceName.c_str(), "_shadow") || CRenderer::CV_r_bumpselfshadow==2))
  {
    ti->m_pSH = new STexShadow;
    ti->m_pSH->Init(im[0]->mfGet_image(), im[0]->mfGet_width(), im[0]->mfGet_height(), false);
  }
#endif

  if (im[0])
    tx = UploadImage(im[0], name, flags, flags2, eTT, bind, ti, fAmount1, fAmount2);
  delete im[0];
  delete im[1];
  if (!tx)
    return NULL;

  return tx;
}

//===============================================================================

void CTexMan::ImagePreprocessing(CImageFile* im, uint flags, uint flags2, byte eTT, STexPic *ti)
{
  if (!im)
    return;
  EImFormat imf = im->m_eFormat;
  int nMips = im->mfGet_numMips();
  int width = im->mfGet_width();
  int height = im->mfGet_height();
  ti->m_WidthOriginal = width;
  ti->m_HeightOriginal = height;
  int nWidth = ilog2(width);
  int nHeight = ilog2(height);
  bool bPowerOfTwo = ((nWidth == width) && (nHeight == height));
  int nComps;
  switch (imf)
  {
    case eIF_DDS_RGB8:
    case eIF_DDS_SIGNED_RGB8:
    case eIF_DDS_DSDT:
      nComps = 3;
      break;
    case eIF_DDS_RGBA8:
      nComps = 4;
      break;
    case eIF_Gif:
    case eIF_Pcx:
    case eIF_DDS_LUMINANCE:
      nComps = 1;
      break;
    case eIF_DDS_RGBA4:
      nComps = 2;
      break;
    default:
      nComps = 4;
  }
  if (!(m_Streamed & 1) && !(flags & FT_NORESIZE))
  {
    int minSize = max(CRenderer::CV_r_texminsize, 16);
    if (eTT == eTT_Bumpmap)
    {
      if (CRenderer::CV_r_texbumpresolution > 0)
      {
        if (nWidth >= minSize || nHeight >= minSize)
        {
          int nRes = min(CRenderer::CV_r_texbumpresolution, 4);
          nWidth = max(nWidth>>nRes, 1);
          nHeight = max(nHeight>>nRes, 1);
        }
      }
    }
    else
    {
      if (flags & FT_SKY)
      {
        if (CRenderer::CV_r_texskyresolution > 0)
        {
          if (nWidth >= minSize || nHeight >= minSize)
          {
            int nRes = min(CRenderer::CV_r_texskyresolution, 4);
            nWidth = max(nWidth>>nRes, 1);
            nHeight = max(nHeight>>nRes, 1);
          }
        }
      }
      else
      if (flags & FT_LM)
      {
        if (CRenderer::CV_r_texlmresolution > 0)
        {
          if (nWidth >= minSize || nHeight >= minSize)
          {
            int nRes = min(CRenderer::CV_r_texlmresolution, 4);
            nWidth = max(nWidth>>nRes, 1);
            nHeight = max(nHeight>>nRes, 1);
          }
        }
      }
      else
      {
        if (CRenderer::CV_r_texresolution > 0)
        {
          // Do no resize TGA textures and DSDT textures
          if (ti->m_eTT != eTT_DSDTBump && im->m_eFormat != eIF_Tga)
          {
            if (nWidth >= minSize || nHeight >= minSize)
            {
              int nRes = min(CRenderer::CV_r_texresolution, 4);
              nWidth = max(nWidth>>nRes, 1);
              nHeight = max(nHeight>>nRes, 1);
            }
          }
        }
      }
    }
    if (nWidth != nHeight && CRenderer::CV_r_texforcesquare)
    {
      nWidth = max(nWidth, nHeight);
      nHeight = nWidth;
    }
    // Hardware limitation check
    if (gRenDev->m_MaxTextureSize)
    {
      if (nWidth > gRenDev->m_MaxTextureSize)
        nWidth = gRenDev->m_MaxTextureSize;
      if (nHeight > gRenDev->m_MaxTextureSize)
        nHeight = gRenDev->m_MaxTextureSize;
    }
    if (nWidth <= 0)
      nWidth = 1;
    if (nHeight <= 0)
      nHeight = 1;
    // User limitation check
    if (CRenderer::CV_r_texmaxsize > 1 && im->m_eFormat != eIF_Tga)
    {
      CRenderer::CV_r_texmaxsize = ilog2(CRenderer::CV_r_texmaxsize);

      if (nWidth > CRenderer::CV_r_texmaxsize)
        nWidth = CRenderer::CV_r_texmaxsize;
      if (nHeight > CRenderer::CV_r_texmaxsize)
        nHeight = CRenderer::CV_r_texmaxsize;
    }
  }

  if (!bPowerOfTwo)
  {
    if (imf == eIF_DXT1 || imf == eIF_DXT3 || imf == eIF_DXT5)
      Warning( VALIDATOR_FLAG_TEXTURE,im->m_FileName,"Error: CTexMan::ImagePreprocessing: Attempt to load of non-power-of-two compressed texture '%s' (%dx%d)", im->m_FileName, width, height);
    else
      Warning( VALIDATOR_FLAG_TEXTURE,im->m_FileName,"Warning: CTexMan::ImagePreprocessing: Attempt to load of non-power-of-two texture '%s' (%dx%d)", im->m_FileName, width, height);
  }
  else
  if ((flags & FT_NORESIZE))
    return;
  if (nWidth != width || nHeight != height)
  {
    if (imf == eIF_DXT1 || imf == eIF_DXT3 || imf == eIF_DXT5)
    {
      bool bResample = false;
      if (bPowerOfTwo)
      {
        if (nMips > 1)
        {
          int nLodDW = 0;
          int nLodDH = 0;
          int nOffs = 0;
          int blockSize = imf == eIF_DXT1 ? 8 : 16;
          int wdt = width;
          int hgt = height;
          int n = 0;
          while (wdt || hgt)
          {
            if (!wdt)
              wdt = 1;
            if (!hgt)
              hgt = 1;
            if (wdt == nWidth)
              nLodDW = n;
            if (hgt == nHeight)
              nLodDH = n;
            if (nLodDH && nLodDW)
              break;
            nOffs += ((wdt+3)/4)*((hgt+3)/4)*blockSize;
            wdt >>= 1;
            hgt >>= 1;
            n++;
          }
          if (nLodDH != nLodDW)
          {
            Warning( VALIDATOR_FLAG_TEXTURE,im->m_FileName,"Error: CTexMan::ImagePreprocessing: Scaling of '%s' compressed texture is dangerous (non-proportional scaling)", im->m_FileName);
          }
          else
          if (n)
          {
            byte *dst = im->m_pByteImage;
            int nSize = im->mfGet_ImageSize();
            memmove(dst, &dst[nOffs], nSize-nOffs);
            im->mfSet_ImageSize(nSize-nOffs);
            im->mfSet_numMips(nMips-n);
            im->mfSet_dimensions(nWidth, nHeight);
            bResample = true;
          }
        }
        else
          Warning( VALIDATOR_FLAG_TEXTURE,im->m_FileName,"Error: CTexMan::ImagePreprocessing: Scaling of '%s' compressed texture is dangerous (only one mip)", im->m_FileName);
      }
#ifndef WIN64
      if (!bResample)
      {
        switch (imf)
        {
          case eIF_DXT1:
            ti->m_Flags |= FT_DXT1;
        	  break;
          case eIF_DXT3:
            ti->m_Flags |= FT_DXT3;
        	  break;
          case eIF_DXT5:
            ti->m_Flags |= FT_DXT5;
        	  break;
        }
        ti->m_Width = width;
        ti->m_Height = height;
        byte *data_ = ImgConvertDXT_RGBA(im->mfGet_image(), ti, im->mfGet_ImageSize());
        //::WriteTGA(data_, width, height, "bug.tga", 32);
        byte *dst = new byte[nWidth * nHeight * 4];
        byte *src = data_;
        ImgResample((uint *)dst, nWidth, nHeight, (uint *)src, width, height);
        //::WriteTGA(dst, nWidth, nHeight, "bug1.tga", 32);
        SAFE_DELETE_ARRAY(im->m_pByteImage);
        SAFE_DELETE_ARRAY(data_);
        ti->m_Width = nWidth;
        ti->m_Height = nHeight;
        bool bUseMips = true;
        if (nMips > 1)
        {
          nMips = 0;
          bUseMips = false;
        }
        int DXTSize = 0;
        byte *dstDXT = ImgConvertRGBA_DXT(dst, ti, DXTSize, nMips, 32, bUseMips);
        //WriteDDS(dstDXT, nWidth, nHeight, DXTSize, "bugDXT.dds", eIF_DXT1, nMips);
        int wdt = nWidth;
        int hgt = nHeight;
        int n = 0;
        int blockSize = imf == eIF_DXT1 ? 8 : 16;
        int nSize = 0;
        while (wdt || hgt)
        {
          if (!wdt)
            wdt = 1;
          if (!hgt)
            hgt = 1;
          nSize += ((wdt+3)/4)*((hgt+3)/4)*blockSize;
          wdt >>= 1;
          hgt >>= 1;
          n++;
        }
        SAFE_DELETE_ARRAY(dst);
        im->m_pByteImage = dstDXT;
        im->mfSet_ImageSize(DXTSize);
        im->mfSet_numMips(nMips);
        im->mfSet_dimensions(nWidth, nHeight);
      }
#endif
    }
    else
    {
      if (bPowerOfTwo)
      {
        bool bResampled = false;
        if (nMips > 1)
        {
          int nLodDW = 0;
          int nLodDH = 0;
          int nOffs = 0;
          int wdt = width;
          int hgt = height;
          int n = 0;
          while (wdt || hgt)
          {
            if (!wdt)
              wdt = 1;
            if (!hgt)
              hgt = 1;

            if (wdt == nWidth)
              nLodDW = n;
            if (hgt == nHeight)
              nLodDH = n;

            if (nLodDH && nLodDW)
              break;
            nOffs += wdt*hgt*nComps;
            wdt >>= 1;
            hgt >>= 1;
            n++;
          }
          if (nLodDH == nLodDW && n)
          {
            byte *dst = im->m_pByteImage;
            int nSize = im->mfGet_ImageSize();
            memmove(dst, &dst[nOffs], nSize-nOffs);
            im->mfSet_ImageSize(nSize-nOffs);
            im->mfSet_numMips(nMips-n);
            im->mfSet_dimensions(nWidth, nHeight);
            bResampled = true;
          }
        }
        if (!bResampled)
        {
          if (nComps == 1)
          {
            byte *dst = new byte[nWidth * nHeight];
            byte *src = (byte *)im->m_pByteImage;
            ImgResample8(dst, nWidth, nHeight, src, width, height);
            SAFE_DELETE_ARRAY(im->m_pByteImage);
            im->m_pByteImage = dst;
            im->mfSet_ImageSize(nWidth*nHeight);
            im->mfSet_numMips(0);
            im->mfSet_dimensions(nWidth, nHeight);
          }
          else
          {
            byte *dst = new byte[nWidth * nHeight * 4];
            byte *src = im->m_pByteImage;
            ImgResample((uint *)dst, nWidth, nHeight, (uint *)src, width, height);
            SAFE_DELETE_ARRAY(im->m_pByteImage);
            im->m_pByteImage = dst;
            im->mfSet_ImageSize(nWidth*nHeight*4);
            im->mfSet_numMips(0);
            im->mfSet_dimensions(nWidth, nHeight);
          }
        }
      }
      else
      {
        if (nMips > 1)
        {
          byte *dst = new byte[nWidth * nHeight * 4];
          byte *src = im->m_pByteImage;
          byte *src1 = NULL;
          if (nComps == 3)
          {
            src1 = new byte[width * height * 4];
            for (int i=0; i<width*height; i++)
            {
              src1[i*4+0] = src[i*3+0];
              src1[i*4+1] = src[i*3+1];
              src1[i*4+2] = src[i*3+2];
              src1[i*4+3] = 255;
            }
            src = src1;
          }
          ImgResample((uint *)dst, nWidth, nHeight, (uint *)src, width, height);
          SAFE_DELETE_ARRAY(src1);
          SAFE_DELETE_ARRAY(im->m_pByteImage);
          im->m_pByteImage = dst;
          im->mfSet_ImageSize(nWidth*nHeight*4);
          im->mfSet_numMips(0);
          im->mfSet_dimensions(nWidth, nHeight);
        }
        else
        {
          byte *dst = new byte[nWidth * nHeight * 4];
          byte *src = im->m_pByteImage;
          ImgResample((uint *)dst, nWidth, nHeight, (uint *)src, width, height);
          SAFE_DELETE_ARRAY(im->m_pByteImage);
          im->m_pByteImage = dst;
          im->mfSet_ImageSize(nWidth*nHeight*4);
          im->mfSet_numMips(0);
          im->mfSet_dimensions(nWidth, nHeight);
        }
      }
    }
  }
}

//===============================================================================

byte *CTexMan::ConvertRGB_Gray(byte *src, STexPic *ti, int flags, ETEX_Format eTF)
{
  byte *dst = new byte [ti->m_Width*ti->m_Height];
  int size = ti->m_Width*ti->m_Height;
  int nComps = (eTF == eTF_0888) ? 3 : 4;
  byte *src1 = src;
  byte *dst1 = dst;
  if (flags & FT_BUMP_DETRED)
  while (size)
  {
    *dst1 = src1[0];
    dst1++;
    src1 += 4;
    size--;
  }
  else
  if (flags & FT_BUMP_DETBLUE)
  while (size)
  {
    *dst1 = src1[2];
    dst1++;
    src1 += 4;
    size--;
  }
  else
  if (flags & FT_BUMP_DETALPHA)
  while (size)
  {
    *dst1 = src1[3];
    dst1++;
    src1 += 4;
    size--;
  }
  else
  if (CRenderer::CV_r_texgrayoverage)
  {
    while (size)
    {
      *dst1 = (src1[0] + src1[1] + src1[2]) / 3;
      dst1++;
      src1 += nComps;
      size--;
    }
  }
  else
  {
    while (size)
    {
      float fRed, fGreen, fBlue;
      if (m_bRGBA)
      {
        fRed =   src1[0] / 255.0f;
        fGreen = src1[1] / 255.0f;
        fBlue =  src1[2] / 255.0f;
      }
      else
      {
        fRed =   src1[2] / 255.0f;
        fGreen = src1[1] / 255.0f;
        fBlue =  src1[0] / 255.0f;
      }
      float fLuminance = ((fRed * 0.3f) + (fGreen * 0.59f) + (fBlue * 0.11f));
      *dst1 = (byte)(fLuminance * 255.0f);
      dst1++;
      src1 += nComps;
      size--;
    }
  }
  return dst;
}

byte *CTexMan::GenerateNormalMap(byte *src, int width, int height, uint flags, uint flags2, byte eTT, float fAmount, STexPic *ti, int& nMips, int& nSize, ETEX_Format eTF)
{
  byte *dst;
  int i, j, l;

  ti->m_Width = width;
  ti->m_Height = height;
  byte *gray = ConvertRGB_Gray(src, ti, flags, eTF);
  
  /*if (!ti->m_pSH)
  {
    ti->m_pSH = new STexShadow;
    ti->m_pSH->Init(gray, width, height, true);
  }*/

  bool bInv = (flags2 & FT2_BUMPINVERTED) ? true : false;

  float fScale;
  if (fAmount >= 0)
    fScale = fAmount / 10.0f;
  else
    fScale = 4.0f;
  if (flags & FT_NOMIPS)
    nMips = 1;
  else
  {
    // Compute log base 2 of width.
    int bits = width;
    int nlevelsw = 0;
    for ( true; bits != 0; )
    {
      bits = bits >> 1;
      nlevelsw++;
    }
    // Compute log base 2 of height.
    bits = height;
    int nlevelsh = 0;
    for ( true; bits != 0; )
    {
      bits = bits >> 1;
      nlevelsh++;
    }
    nMips = max(nlevelsw, nlevelsh);
  }
  int mx = width-1;
  int my = height-1;
  /*if (eTT == eTT_DSDTBump)
  {
    nMips = 1;
    nSize = width*height*3;
    dst = new byte[nSize];
    BYTE* pDstT  = (BYTE*)dst;
    BYTE* pSrcB0 = (BYTE*)gray;
    BYTE* pSrcB1 = ( pSrcB0 + width );
    BYTE* pSrcB2 = ( pSrcB0 - width );

    for(int y=0; y<height; y++ )
    {
      if( y == height-1 )  // Don't go past the last line
        pSrcB1 = pSrcB0;
      if( y == 0 )               // Don't go before first line
        pSrcB2 = pSrcB0;

      for( int x=0; x<width; x++ )
      {
        LONG v00 = *(pSrcB0+0); // Get the current pixel
        LONG v01 = *(pSrcB0+1); // and the pixel to the right
        LONG vM1 = *(pSrcB0-1); // and the pixel to the left
        LONG v10 = *(pSrcB1+0); // and the pixel one line below.
        LONG v1M = *(pSrcB2+0); // and the pixel one line above.

        LONG iDu = (vM1-v01); // The delta-u bump value
        LONG iDv = (v1M-v10); // The delta-v bump value

        if( (v00 < vM1) && (v00 < v01) )  // If we are at valley
        {
          iDu = vM1-v00;                 // Choose greater of 1st order dif
          if( iDu < v00-v01 )
            iDu = v00-v01;
        }

        // The luminance bump value (land masses are less shiny)
        WORD uL = ( v00>1 ) ? 63 : 127;

        *pDstT++ = (BYTE)iDu;
        *pDstT++ = (BYTE)iDv;
        *pDstT++ = (BYTE)uL;

        // Move one pixel to the left (src is 32-bpp)
        pSrcB0+=1;
        pSrcB1+=1;
        pSrcB2+=1;
      }
    }
    return dst;
  }*/

  TArray<Vec3d> *Normals = new TArray<Vec3d> [nMips];
  TArray<byte> *Heights = new TArray<byte> [nMips];
  nSize = width * height * 4;
  if (!CRenderer::CV_r_texnormalmaptype)
  {
    Normals[0].Grow(width*height);
    Heights[0].Grow(width*height);
    Vec3d *vDst = &Normals[0][0];
    byte *bDst = &Heights[0][0];
    for(j=0; j<height; j++)
    {
      for(i=0; i<width; i++)
      {
        Vec3d vN;
        vN.x = ((float)gray[j*width+((i-1)&mx)] - (float)gray[j*width+((i+1)&mx)]) / 255.0f;
        vN.y = ((float)gray[((j-1)&my)*width+i] - (float)gray[((j+1)&my)*width+i]) / 255.0f;
        if (bInv)
        {
          vN[0] = -vN[0];
          vN[1] = -vN[1];
        }
        vN.x *= fScale;
        vN.y *= fScale;
        vN.z = 1.0f;
        vN.NormalizeFast();
        *vDst = vN;
        vDst++;
        *bDst = gray[j*width+i];
        bDst++;
      }
    }
  }
  else
  {
    Normals[0].Grow(width*height);
    Vec3d *vDst = &Normals[0][0];
    Heights[0].Grow(width*height);
    byte *bDst = &Heights[0][0];
    for(j=0; j<height; j++)
    {
      for(i=0; i<width; i++)
      {
        Vec3d vN1, vN2;
        vN1.x = ((float)gray[j*width+i] - (float)gray[j*width+((i+1)&mx)]) / 255.0f;
        vN1.y = ((float)gray[j*width+i] - (float)gray[((j+1)&my)*width+i]) / 255.0f;
        if (bInv)
        {
          vN1.x = -vN1.x;
          vN1.y = -vN1.y;
        }
        vN1.x *= fScale;
        vN1.y *= fScale;
        float f = vN1.y;
        vN1.z = 1.0f;
        vN1.NormalizeFast();

        vN2.x = ((float)gray[((j+1)&my)*width+i] - (float)gray[((j+1)&my)*width+((i+1)&mx)]) / 255.0f;
        vN2.y = f;
        if (bInv)
        {
          vN2.x = -vN2.x;
          vN2.y = -vN2.y;
        }
        vN2.x *= fScale;
        vN2.z = 1.0f;
        vN2.NormalizeFast();

        *vDst = vN1 + vN2;
        vDst->NormalizeFast();
        vDst++;

        *bDst = gray[j*width+i];
        bDst++;
      }
    }
  }

  int reswp = width;
  int reshp = height;
  for (l=1; l<nMips; l++)
  {
    int resw = width  >> l;
    int resh = height >> l;
    if (!resw)
      resw = 1;
    if (!resh)
      resh = 1;
    nSize += resw * resh * 4;
    Normals[l].Grow(resw*resh);
    Heights[l].Grow(resw*resh);
    Vec3d *curr = &Normals[l][0];
    Vec3d *prev = &Normals[l-1][0];
    byte *bcurr = &Heights[l][0];
    byte *bprev = &Heights[l-1][0];
    int wmul = (reswp == 1) ? 1 : 2;
    int hmul = (reshp == 1) ? 1 : 2;
    for (j=0; j<resh; j++)
    {
      for (i=0; i<resw; i++)
      {
        Vec3d avg;
        int bavg;
        if (wmul == 1)
        {
          avg = prev[wmul*i+hmul*j*reswp] +
                prev[wmul*i+(hmul*j+1)*reswp];
          bavg = bprev[wmul*i+hmul*j*reswp] +
                 bprev[wmul*i+(hmul*j+1)*reswp];
          bavg /= 2;
        }
        else
        if (hmul == 1)
        {
          avg = prev[wmul*i+hmul*j*reswp] +
                prev[wmul*i+1+hmul*j*reswp];
          bavg = bprev[wmul*i+hmul*j*reswp] +
                 bprev[wmul*i+1+hmul*j*reswp];
          bavg /= 2;
        }
        else
        {
          avg = prev[wmul*i+hmul*j*reswp] +
                prev[wmul*i+1+hmul*j*reswp] +
                prev[wmul*i+1+(hmul*j+1)*reswp] +
                prev[wmul*i+(hmul*j+1)*reswp];
          bavg = bprev[wmul*i+hmul*j*reswp] +
                 bprev[wmul*i+1+hmul*j*reswp] +
                 bprev[wmul*i+1+(hmul*j+1)*reswp] +
                 bprev[wmul*i+(hmul*j+1)*reswp];
          bavg /= 4;
        }
        avg.NormalizeFast();
        *curr = avg;
        curr++;
        *bcurr = bavg;
        bcurr++;
      }
    }
    reswp = resw;
    reshp = resh;
  }
  dst = new byte[nSize];
  int n = 0;
  if (eTT == eTT_Bumpmap)
  {
    for (l=0; l<nMips; l++)
    {
      int resw = width  >> l;
      int resh = height >> l;
      if (!resw)
        resw = 1;
      if (!resh)
        resh = 1;
      for (i=0; i<resw*resh; i++)
      {
        Vec3d vN = Normals[l][i];
        byte bH = Heights[l][i];
        dst[n*4+2] = (byte)(vN.x * 127.0f + 128.0f);
        dst[n*4+1] = (byte)(vN.y * 127.0f + 128.0f);
        dst[n*4+0] = (byte)(vN.z * 127.0f + 128.0f);
        dst[n*4+3] = bH;
        n++;
      }
    }
  }
  else
  if (eTT == eTT_DSDTBump)
  {
    for (l=0; l<nMips; l++)
    {
      int resw = width  >> l;
      int resh = height >> l;
      if (!resw)
        resw = 1;
      if (!resh)
        resh = 1;
      for (i=0; i<resw*resh; i++)
      {
        Vec3d vN = Normals[l][i];
        float f = vN.x * 127.5f;
        dst[n*4+0] = (byte)(f);
        f = vN.y * 127.5f;
        dst[n*4+1] = (byte)(f);
        dst[n*4+2] = (byte)(63.0f);
        //dst[n*3+0] = byte(   (vN.x>=0.0f) ? (vN.x* 127.5f) : (256.0f + vN.x * 127.5f));
        //dst[n*3+1] = byte(   (vN.y>=0.0f) ? (vN.y* 127.5f) : (256.0f + vN.y * 127.5f));
        n++;
      }
    }
  }

  delete [] Heights;
  delete [] Normals;
  delete [] gray;

  return dst;
}

void CTexMan::MergeNormalMaps(byte *src[2], CImageFile *im[2], int nMips[2])
{
  int i, j, l, n;

  int Width0 = im[0]->mfGet_width();
  int Height0 = im[0]->mfGet_height();
  int Width1 = im[1]->mfGet_width();
  int Height1 = im[1]->mfGet_height();
  n = 0;
  int nIndexNM = (im[0]->mfGet_Flags() & FIM_NORMALMAP) ? 0 : 1;

  if (Width0 == Width1 && Height0 == Height1)
  {
    for (l=0; l<nMips[0]; l++)
    {
      int wdt = Width0 >> l;
      int hgt = Height0 >> l;
      if (!wdt)
        wdt = 1;
      if (!hgt)
        hgt = 1;
      for (j=0; j<hgt; j++)
      {
        for (i=0; i<wdt; i++)
        {
          Vec3d vN[2];
          vN[0].x = (src[0][n*4+2]/255.0f-0.5f)*2.0f;
          vN[0].y = (src[0][n*4+1]/255.0f-0.5f)*2.0f;
          vN[0].z = (src[0][n*4+0]/255.0f-0.5f)*2.0f;

          vN[1].x = (src[1][n*4+2]/255.0f-0.5f)*2.0f;
          vN[1].y = (src[1][n*4+1]/255.0f-0.5f)*2.0f;
          vN[1].z = (src[1][n*4+0]/255.0f-0.5f)*2.0f;

          float fLen = min(vN[nIndexNM].Length(), 1.0f);

          vN[0].x /= vN[0].z;
          vN[0].y /= vN[0].z;
          vN[0].z = 1.0f;

          vN[1].x /= vN[1].z;
          vN[1].y /= vN[1].z;
          vN[1].z = 1.0f;

          vN[0] = vN[0] + vN[1];
          vN[0].x *= 2.0f;
          vN[0].y *= 2.0f;
          vN[0].NormalizeFast();
          vN[0] *= fLen;
          vN[0].CheckMax(Vec3(-1,-1,-1));
          vN[0].CheckMin(Vec3(1,1,1));

          src[0][n*4+2] = (byte)(vN[0].x * 127.0f + 128.0f);
          src[0][n*4+1] = (byte)(vN[0].y * 127.0f + 128.0f);
          src[0][n*4+0] = (byte)(vN[0].z * 127.0f + 128.0f);
          src[0][n*4+3] = (src[0][n*4+3] + src[1][n*4+3]) >> 1;
          n++;
        }
      }
    }
  }
  else
  {
    for (l=0; l<nMips[0]; l++)
    {
      int wdt = Width0 >> l;
      int hgt = Height0 >> l;
      if (!wdt)
        wdt = 1;
      if (!hgt)
        hgt = 1;
      int l1 = min(l, nMips[1]-1);
      int wdt1 = Width1 >> l1;
      int hgt1 = Height1 >> l1;
      if (!wdt1)
        wdt1 = 1;
      if (!hgt1)
        hgt1 = 1;
      int mwdt = wdt1-1;
      int mhgt = hgt1-1;
      for (j=0; j<hgt; j++)
      {
        for (i=0; i<wdt; i++)
        {
          Vec3d vN[2];
          vN[0].x = (src[0][n*4+2]/255.0f-0.5f)*2.0f;
          vN[0].y = (src[0][n*4+1]/255.0f-0.5f)*2.0f;
          vN[0].z = (src[0][n*4+0]/255.0f-0.5f)*2.0f;

          vN[1].x = (src[1][(i&mwdt)*4+(j&mhgt)*wdt1*4+2]/255.0f-0.5f)*2.0f;
          vN[1].y = (src[1][(i&mwdt)*4+(j&mhgt)*wdt1*4+1]/255.0f-0.5f)*2.0f;
          vN[1].z = (src[1][(i&mwdt)*4+(j&mhgt)*wdt1*4+0]/255.0f-0.5f)*2.0f;

          float fLen = min(vN[nIndexNM].Length(), 1.0f);

          vN[0].x /= vN[0].z;
          vN[0].y /= vN[0].z;
          vN[0].z = 1.0f;

          vN[1].x /= vN[1].z;
          vN[1].y /= vN[1].z;
          vN[1].z = 1.0f;

          vN[0] = vN[0] + vN[1];
          vN[0].x *= 2.0f;
          vN[0].y *= 2.0f;
          vN[0].NormalizeFast();
          vN[0] *= fLen;
          vN[0].CheckMax(Vec3(-1,-1,-1));
          vN[0].CheckMin(Vec3(1,1,1));

          src[0][n*4+2] = (byte)(vN[0].x * 127.0f + 128.0f);
          src[0][n*4+1] = (byte)(vN[0].y * 127.0f + 128.0f);
          src[0][n*4+0] = (byte)(vN[0].z * 127.0f + 128.0f);
          src[0][n*4+3] = (src[0][n*4+3] + src[1][(i&mwdt)*4+(j&mhgt)*wdt1*4+3]) >> 1;
          n++;
        }
      }
    }
  }
}

void CTexMan::GenerateNormalMap(CImageFile** im, uint flags, uint flags2, byte eTT, float fAmount1, float fAmount2, STexPic *ti)
{
  byte *dst[2];
  int nMips[2];
  int nSizeWithMips[2];
  dst[0] = dst[1] = NULL;
  nMips[0] = nMips[1] = 0;
  nSizeWithMips[0] = nSizeWithMips[1] = 0;

  if ((im[0]->mfGet_Flags() & FIM_NORMALMAP) || (im[0]->mfGet_Flags() & FIM_DSDT))
  {
    dst[0] = im[0]->mfGet_image();
    nMips[0] = im[0]->mfGet_numMips();
    nSizeWithMips[0] = im[0]->mfGet_ImageSize();
  }
  else
  {
    ETEX_Format eTF = sImageFormat2TexFormat(im[0]->m_eFormat);
    dst[0] = GenerateNormalMap((byte *)im[0]->mfGet_image(), im[0]->mfGet_width(), im[0]->mfGet_height(), flags, flags2, eTT, fAmount1, ti, nMips[0], nSizeWithMips[0], eTF);
    ti->m_ETF = eTF_8888;
  }

  if (im[1])
  {
    if ((im[1]->mfGet_Flags() & FIM_NORMALMAP) || (im[1]->mfGet_Flags() & FIM_DSDT))
    {
      dst[1] = im[1]->mfGet_image();
      nMips[1] = im[1]->mfGet_numMips();
    }
    else
    {
      ETEX_Format eTF = sImageFormat2TexFormat(im[1]->m_eFormat);
      dst[1] = GenerateNormalMap((byte *)im[1]->mfGet_image(), im[1]->mfGet_width(), im[1]->mfGet_height(), flags, flags2, eTT, fAmount2, ti, nMips[1], nSizeWithMips[1], eTF);
      ti->m_ETF = eTF_8888;
    }
  }
  if (flags & FT_NOMIPS)
    nMips[0] = nMips[1] = 1;

  if (dst[1])
  {
    MergeNormalMaps(dst, im, nMips);
    if (!(im[1]->mfGet_Flags() & FIM_NORMALMAP) && !(im[1]->mfGet_Flags() & FIM_DSDT))
      delete [] dst[1];
    delete im[1];
    im[1] = NULL;
    if (nMips[0])
      im[0]->mfSet_numMips(nMips[0]);
    if (im[0]->m_pByteImage != dst[0])
    {
      SAFE_DELETE_ARRAY (im[0]->m_pByteImage);
      im[0]->m_pByteImage = dst[0];
    }
  }
  else
  if ((im[0]->mfGet_Flags() & FIM_NORMALMAP) || (im[0]->mfGet_Flags() & FIM_DSDT))
    return;
  else
  {
    if (im[0]->m_pByteImage != dst[0])
    {
      SAFE_DELETE_ARRAY (im[0]->m_pByteImage);
      im[0]->m_pByteImage = dst[0];
    }
  }
  if (nMips[0])
    im[0]->mfSet_numMips(nMips[0]);
  if (eTT == eTT_Bumpmap)
  {
    im[0]->mfSet_Flags(FIM_NORMALMAP);
    im[0]->m_eFormat = eIF_DDS_RGBA8;
  }
  else
  if (eTT == eTT_DSDTBump)
  {
    im[0]->mfSet_Flags(FIM_DSDT);
    im[0]->m_eFormat = eIF_DDS_DSDT;
  }
  im[0]->mfSet_ImageSize(nSizeWithMips[0]);
}

void CTexMan::BuildImageGamma(int wdt, int hgt, byte *dst, bool bHasAlpha)
{
}

void CTexMan::ImgResample(uint *uout, int ox, int oy, uint *uin, int ix, int iy)
{
  int   i, j;
  uint  *inrow;
  uint  ifrac, fracstep;

  fracstep = ix*0x10000/ox;
  if (!(ox & 3))
  {
    for (i=0 ; i<oy ; i++, uout += ox)
    {
      inrow = uin + ix*(i*iy/oy);
      ifrac = fracstep >> 1;
      for (j=0 ; j<ox ; j+=4)
      {
        uout[j] = inrow[ifrac>>16];
        ifrac += fracstep;
        uout[j+1] = inrow[ifrac>>16];
        ifrac += fracstep;
        uout[j+2] = inrow[ifrac>>16];
        ifrac += fracstep;
        uout[j+3] = inrow[ifrac>>16];
        ifrac += fracstep;
      }
    }
  }
  else
  {
    for (i=0 ; i<oy ; i++, uout += ox)
    {
      inrow = uin + ix*(i*iy/oy);
      ifrac = fracstep >> 1;
      for (j=0 ; j<ox ; j++)
      {
        uout[j] = inrow[ifrac>>16];
        ifrac += fracstep;
      }
    }
  }
}

void CTexMan::ImgResample8(byte *uout, int ox, int oy, byte *uin, int ix, int iy)
{
  int   i, j;
  byte  *inrow;
  uint  ifrac, fracstep;

  fracstep = ix*0x10000/ox;
  if (!(ox & 3))
  {
    for (i=0 ; i<oy ; i++, uout += ox)
    {
      inrow = uin + ix*(i*iy/oy);
      ifrac = fracstep >> 1;
      for (j=0 ; j<ox ; j+=4)
      {
        uout[j] = inrow[ifrac>>16];
        ifrac += fracstep;
        uout[j+1] = inrow[ifrac>>16];
        ifrac += fracstep;
        uout[j+2] = inrow[ifrac>>16];
        ifrac += fracstep;
        uout[j+3] = inrow[ifrac>>16];
        ifrac += fracstep;
      }
    }
  }
  else
  {
    for (i=0 ; i<oy ; i++, uout += ox)
    {
      inrow = uin + ix*(i*iy/oy);
      ifrac = fracstep >> 1;
      for (j=0 ; j<ox ; j++)
      {
        uout[j] = inrow[ifrac>>16];
        ifrac += fracstep;
      }
    }
  }
}

//============================================================================

byte *sData;
static TArray<byte> sAData;
void WriteDTXnFile (DWORD count, void *buffer, void * userData)
{
  int n = sAData.Num();
  sAData.Grow(count);
  cryMemcpy(&sAData[n], buffer, count);
}

void ReadDTXnFile (DWORD count, void *buffer, void * userData)
{
  cryMemcpy(buffer, sData, count);
  sData += count;
}

#if !defined(_XBOX) && !defined(PS2) && !defined(LINUX)
#include <ddraw.h>
#else

#define DDSD_CAPS		0x00000001l	// default
#define DDSD_PIXELFORMAT	0x00001000l
#define DDSD_WIDTH		0x00000004l
#define DDSD_HEIGHT		0x00000002l
#define DDSD_LINEARSIZE		0x00080000l

#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

#endif

#include "Image/dds.h"
#include "dxtlib.h"

bool CTexMan::m_bRGBA = true;

byte *CTexMan::ImgConvertDXT_RGBA(byte *dst, STexPic *ti, int DXTSize)
{
  int width;
  int height;
  int planes;
  int lTotalWidth; 
  int rowBytes;
  DDS_HEADER *ddsh;

  byte *dd = new byte [DXTSize + sizeof(DDS_HEADER) + sizeof(DWORD)];

  DWORD dwMagic = MAKEFOURCC('D','D','S',' ');
  *(DWORD *)dd = dwMagic;
  ddsh = (DDS_HEADER *)&dd[sizeof(DWORD)];
  memset(ddsh, 0, sizeof(DDS_HEADER));
  cryMemcpy(&dd[sizeof(DWORD)+sizeof(DDS_HEADER)], dst, DXTSize);

  ddsh->dwSize = sizeof(DDS_HEADER);
  ddsh->dwWidth = ti->m_Width;
  ddsh->dwHeight = ti->m_Height;
  ddsh->dwHeaderFlags = DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_LINEARSIZE;
  int blockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
  ddsh->dwPitchOrLinearSize = ti->m_Width*ti->m_Height*4/blockSize;
  if (ti->m_Flags & FT_DXT1)
    ddsh->ddspf.dwFourCC = FOURCC_DXT1;
  else
  if (ti->m_Flags & FT_DXT3)
    ddsh->ddspf.dwFourCC = FOURCC_DXT3;
  else
  if (ti->m_Flags & FT_DXT5)
    ddsh->ddspf.dwFourCC = FOURCC_DXT5;
  ddsh->ddspf.dwSize = sizeof(ddsh->ddspf);
  ddsh->ddspf.dwFlags = DDS_FOURCC;
  ddsh->dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;

  sData = dd;
      
#if defined(WIN64) || defined(LINUX)
	// NOTE: AMD64 port: implement
	dd = new byte [ti->m_Width*ti->m_Height*4];
#else
  int src_format;
  byte *_data = nvDXTdecompress(width, height, planes, lTotalWidth, rowBytes, src_format);
  delete [] dd;
  dd = new byte [ti->m_Width*ti->m_Height*4];
  byte *dd1 = dd;
  byte *data1 = _data;

  if (planes == 3)
  {
    int n = width*height;
    if (m_bRGBA)
    for (int i=0; i<n; i++)
    {
      dd1[2] = data1[0];
      dd1[1] = data1[1];
      dd1[0] = data1[2];
      dd1[3] = 255;
      dd1   += 4;
      data1 += 3;
    }
    else
    for (int i=0; i<n; i++)
    {
      dd1[0] = data1[0];
      dd1[1] = data1[1];
      dd1[2] = data1[2];
      dd1[3] = 255;
      dd1   += 4;
      data1 += 3;
    }
  }
  else
  {
    int n = width*height;
    if (m_bRGBA)
    for (int i=0; i<n; i++)
    {
      dd1[2] = data1[0];
      dd1[1] = data1[1];
      dd1[0] = data1[2];
      dd1[3] = data1[3];
      dd1   += 4;
      data1 += 4;
    }
    else
    for (int i=0; i<n; i++)
    {
      dd1[0] = data1[0];
      dd1[1] = data1[1];
      dd1[2] = data1[2];
      dd1[3] = data1[3];
      dd1   += 4;
      data1 += 4;
    }
  }

  CRTDeleteArray(_data);
#endif
  return dd;
}
  
byte *CTexMan::ImgConvertRGBA_DXT(byte *dst, STexPic *ti, int& DXTSize, int& nMips, int bits, bool bUseExistingMips)
{
#if !defined(PS2) && !defined(WIN64) && !defined(LINUX)
	// NOTE: AMD64 port: implement

  assert (bits == 24 || bits == 32);

  CompressionOptions opt;
  int i, j;
  if (ti->m_Flags & FT_DXT1)
    opt.TextureFormat = kDXT1;
  else
  if (ti->m_Flags & FT_DXT3)
    opt.TextureFormat = kDXT3;
  else
  if (ti->m_Flags & FT_DXT5)
    opt.TextureFormat = kDXT5;

  int width = ti->m_Width;
  int height = ti->m_Height;
  byte *d;

  if (bUseExistingMips)
  {
    int w = width;
    int h = height;
    TArray<byte> Data;
    for (i=0; i<nMips; i++)
    {
      if (!w)
        w = 1;
      if (!h)
        h = 1;
      sAData.Free();
      if (m_bRGBA)
      {
        if (bits == 24)
        {
          for (j=0; j<w*h; j++)
          {
            Exchange(dst[j*3+0], dst[j*3+2]);
          }
        }
        else
        {
          for (j=0; j<w*h; j++)
          {
            Exchange(dst[j*4+0], dst[j*4+2]);
          }
        }
      }
      opt.MipMapType = dNoMipMaps;
      nvDXTcompress(dst, w, h, w, &opt, bits/8, NULL);
      if (bits == 24)
        dst += w*h*3;
      else
        dst += w*h*4;
      int Offs = sizeof(DWORD)+sizeof(DDS_HEADER);
      int n = Data.Num();
      int Size = sAData.Num()-Offs;
      int blockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
      int mipsize = ((w+3)/4)*((h+3)/4)*blockSize;
      assert(mipsize == Size);
      Data.Grow(Size);
      cryMemcpy(&Data[n], &sAData[Offs], Size);
      w >>= 1;
      h >>= 1;
    }
    d = new byte[Data.Num()];
    cryMemcpy(d, &Data[0], Data.Num());
    DXTSize = Data.Num();
  }
  else
  {
    sAData.Free();
    opt.MipMapType = dGenerateMipMaps;
    nvDXTcompress(dst, width, height, width, &opt, bits/8, NULL);

    int Offs = sizeof(DWORD)+sizeof(DDS_HEADER);
    d = new byte[sAData.Num()-Offs];
    cryMemcpy(d, &sAData[Offs], sAData.Num()-Offs);
    DXTSize = sAData.Num()-Offs;
    sAData.Free();
  }

  if (!bUseExistingMips)
  {
    nMips = 0;
    while (width || height)
    {
      if (!width)
        width = 1;
      if (!height)
        height = 1;
      nMips++;

      width >>= 1;
      height >>= 1;
    }
  }

  return d;

#else
  return NULL;
#endif
}


void CTexMan::MipMap8Bit (STexPic *ti, byte *in, byte *out, int width, int height)
{
  int    i, j;
  uint r,g,b;
  byte  *at1, *at2, *at3, *at4;

  height >>= 1;
  uint *tabsrc = ti->m_p8to24table;
  byte *tabdst = ti->m_p15to8table;
  for (i=0; i<height; i++,in+=width)
  {
    for (j=0; j<width; j+=2,out+=1,in+=2)
    {
      at1 = (byte *) (tabsrc + in[0]);
      at2 = (byte *) (tabsrc + in[1]);
      at3 = (byte *) (tabsrc + in[width+0]);
      at4 = (byte *) (tabsrc + in[width+1]);

      r = (at1[0]+at2[0]+at3[0]+at4[0]); r>>=5;
      g = (at1[1]+at2[1]+at3[1]+at4[1]); g>>=5;
      b = (at1[2]+at2[2]+at3[2]+at4[2]); b>>=5;

      out[0] = tabdst[(r<<0) + (g<<5) + (b<<10)];
    }
  }
}

void CTexMan::MipMap32Bit (STexPic *ti, byte *in, byte *out, int width, int height)
{
  int    i, j;

  byte *src1 = in;
  byte *dst1 = out;
  int wd = width<<3;
  for (i=0; i<height; i++,in+=width)
  {
    byte *src2 = src1;
    for (j=0; j<width; j++)
    {
      dst1[0] = (src2[0]+src2[4]+src2[wd+0]+src2[wd+4])>>2;
      dst1[1] = (src2[1]+src2[5]+src2[wd+1]+src2[wd+5])>>2;
      dst1[2] = (src2[2]+src2[6]+src2[wd+2]+src2[wd+6])>>2;
      dst1[3] = (src2[3]+src2[7]+src2[wd+3]+src2[wd+7])>>2;
      dst1 += 4;
      src2 += 8;
    }
    src1 += wd<<1;
  }
}

void CTexMan::ClearAll(int nFlags)
{
  if (nFlags == FRR_ALL)
    Shutdown();

  for (int i=0; i<m_Textures.Num(); i++)
  {
    STexPic *tp = m_Textures[i];
    if (!tp)
      continue;
    
    if ((nFlags & FRR_RESTORE) && (tp->m_Flags & FT_NOREMOVE) && !(nFlags & FRR_SYSTEM))
      continue;

    if (!(tp->m_Flags & FT_NOREMOVE))
    {
      if (CRenderer::CV_r_printmemoryleaks)
        iLog->Log("Warning: CTexMan::ClearAll: Texture %s (Id: %d) was not deleted (%d)", tp->m_SourceName.c_str(), i, tp->m_nRefCounter);
    }
    continue;
    
    if (nFlags != FRR_ALL && (nFlags & FRR_REINITHW))
    {
      tp->ReleaseDriverTexture();
      continue;
    }

    tp->Release(nFlags == FRR_ALL ? 2 : 1);
  }
  if (nFlags == FRR_ALL)
  {
    gRenDev->m_TexMan->m_Textures.Free();
    gRenDev->m_TexMan->m_FreeSlots.Free();
  }
}

#ifdef WIN64
#pragma warning( push )							//AMD Port
#pragma warning( disable : 4267 )				// conversion from 'size_t' to 'int', possible loss of data
#endif

void CTexMan::ReloadAll(int nFlags)
{
  static char* cubefaces[5] = {"negx","posy","negy","posz","negz"};

  int i, j;

  for (i=0; i<m_Textures.Num(); i++)
  {
    STexPic *tp = m_Textures[i];
    if (!tp || !tp->m_bBusy)
      continue;
    if (tp->m_LoadFrame == gRenDev->GetFrameID())
      continue;
    if (tp->m_Flags2 & FT2_RENDERTARGET)
      continue;
    tp->m_LoadFrame = gRenDev->GetFrameID();
    if (tp->m_eTT == eTT_Cubemap)
    {
      char name[256];
      StripExtension(*tp->m_SearchName, name);
      int len = strlen(name);
      if (len > 5)
      {
        for (j=0; j<5; j++)
        {
          if (!stricmp(&name[len-4], cubefaces[j]))
            break;
		    }
        if (j != 5)
          continue;
	    }
    }

    if (tp->m_pData32)
      CreateTexture(*tp->m_SearchName, tp->m_Width, tp->m_Height, tp->m_Depth, tp->m_Flags, tp->m_Flags2, tp->m_pData32, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_DXTSize, tp, tp->m_Bind, tp->m_ETF);
    else
      gRenDev->EF_LoadTexture(*tp->m_SearchName, tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id, tp->m_Bind);    
  }
}

#ifdef WIN64
#pragma warning( pop )							//AMD Port
#endif

bool CTexMan::ReloadFile(const char *fileName, int nFlags)
{
  bool bRes = false;
  int i;
  char name[256];
  strcpy(name, fileName);
  strlwr(name);
  ConvertDOSToUnixName(name, name);

  for (i=0; i<m_Textures.Num(); i++)
  {
    STexPic *tp = m_Textures[i];
    if (!tp || !tp->m_bBusy)
      continue;
    if (tp->m_LoadFrame == gRenDev->GetFrameID())
      continue;
    if (!strcmp(name, tp->m_SourceName.c_str()) && (tp->m_eTT != eTT_Cubemap || !tp->m_CubeSide))
    {
      tp->m_LoadFrame = gRenDev->GetFrameID();
      iLog->Log("Reload texture '%s'", name);
      gRenDev->EF_LoadTexture(*tp->m_SearchName, tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id, tp->m_Bind);    
      bRes = true;
    }
  }

  return bRes;
}

const char *STexPic::GetName()
{	
	return (m_Name.c_str());
}

int STexPic::GetWidth() 
{ 
  return m_Width; 
}
int STexPic::GetHeight() 
{ 
  return m_Height; 
}
int STexPic::GetTextureID() 
{ 
  return m_Bind; 
}
bool STexPic::IsTextureLoaded() 
{ 
  assert(gRenDev);
  return GetTextureID() != gRenDev->m_TexMan->m_Text_NoTexture->GetTextureID();
}

typedef struct
{
  unsigned char  id_length, colormap_type, image_type;
  unsigned short colormap_index, colormap_length;
  unsigned char  colormap_size;
  unsigned short x_origin, y_origin, width, height;
  unsigned char  pixel_size, attributes;
} TargaHeader_t;

void WriteTGA32(byte *dat, int wdt, int hgt, char *name);


///////////////////////////////////////////////////
bool STexPic::SaveTga(unsigned char *sourcedata,int sourceformat,int w,int h,const char *filename,bool flip)
{
/*	if (flip)
	{
		int size=w*(sourceformat/8);
		unsigned char *tempw=new unsigned char [size];
		unsigned char *src1=sourcedata;		
		unsigned char *src2=sourcedata+(w*(sourceformat/8))*(h-1);
		for (int k=0;k<h/2;k++)
		{
			memcpy(tempw,src1,size);
			memcpy(src1,src2,size);
			memcpy(src2,tempw,size);
			src1+=size;
			src2-=size;
		}
		delete [] tempw;
	}*/

	unsigned char * oldsourcedata = sourcedata;

	if (sourceformat==FORMAT_8_BIT)
	{
		unsigned char *desttemp=new unsigned char [w*h*4];
		memset(desttemp,0,w*h*3);

		unsigned char *destptr=desttemp;
		unsigned char *srcptr=sourcedata;

		unsigned char col;

		for (int k=0;k<w*h;k++)
		{			
			col=*srcptr++;
			*destptr++=col;		
			*destptr++=col;	
			*destptr++=col;		
			*destptr++=255;		
		}
		
		sourcedata=desttemp;

		sourceformat=FORMAT_32_BIT;
	}

  WriteTGA32(sourcedata, w, h, (char*)filename);

	if (sourcedata!=oldsourcedata)
		delete [] sourcedata;

	return (true);
}

void CTexMan::LoadDefaultTextures()
{
  char str[256];

  m_Text_NoTexture = (STexPic*)gRenDev->EF_LoadTexture("Textures/red", FT_NOREMOVE | FT_NOSTREAM | FT_NORESIZE, 0, eTT_Base);
  m_Text_White = (STexPic*)gRenDev->EF_LoadTexture("Textures/white", FT_NOREMOVE, FT2_NOANISO, eTT_Base);
  m_Text_WhiteBump = (STexPic*)gRenDev->EF_LoadTexture("Textures/white_ddn", FT_NOREMOVE | FT_NORESIZE, FT2_NOANISO, eTT_Bumpmap);
  m_Text_Atten2D = (STexPic*)gRenDev->EF_LoadTexture("Textures/Defaults/Pointlight2D", FT_NOREMOVE | FT_CLAMP | FT_NOSTREAM, FT2_NOANISO, eTT_Base);
  m_Text_Edge = (STexPic*)gRenDev->EF_LoadTexture("Grad14", FT_NOREMOVE, FT2_NODXT, eTT_Base); 
  m_Text_Gray = (STexPic*)gRenDev->EF_LoadTexture("Grey", FT_NOREMOVE, FT2_NODXT, eTT_Base);   
  m_Text_FlashBangFlash = (STexPic*)gRenDev->EF_LoadTexture("Textures/flashbangflash", FT_NOREMOVE |  FT_CLAMP| FT_NOSTREAM, 0, eTT_Base);  
  m_Text_ScreenNoise = (STexPic*)gRenDev->EF_LoadTexture("Textures/ScreenNoise", FT_NOREMOVE, 0, eTT_Base);  
  m_Text_HeatPalete= (STexPic*)gRenDev->EF_LoadTexture("Textures/Defaults/palletteHeat", FT_NORESIZE|FT_NOREMOVE | FT_CLAMP, FT2_NODXT, eTT_Base);  

  // Default Template textures

  m_Text_ScreenMap = LoadTexture("$ScreenTexMap", FT_NOREMOVE | FT_NOSTREAM| FT_CLAMP, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_SCREENMAP);
  m_Text_PrevScreenMap = LoadTexture("$PrevScreenTexMap", FT_NOREMOVE | FT_NOSTREAM| FT_CLAMP, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_PREVSCREENMAP);  
  m_Text_Glare   = LoadTexture("$Glare", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_GLARE);
  m_Text_ScreenLuminosityMap = LoadTexture("$ScreenLuminosityMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_SCREENLUMINOSITYMAP);
  m_Text_ScreenCurrLuminosityMap = LoadTexture("$ScreenCurrLuminosityMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_SCREENCURRLUMINOSITYMAP);
  m_Text_ScreenLowMap = LoadTexture("$ScreenLowMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_SCREENLOWMAP);
  m_Text_ScreenAvg1x1 = LoadTexture("$ScreenAvg1x1", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_SCREENAVGMAP);      
  m_Text_FlashBangMap = LoadTexture("$FlashBangMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_FLASHBANGMAP);
  m_Text_DofMap = LoadTexture("$DofTexMap", FT_NOREMOVE | FT_NOSTREAM| FT_CLAMP, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_DOFMAP);  
  
  m_Text_NormalizeCMap = LoadTexture("$NormalizeCMap", FT_NOREMOVE | FT_NOMIPS, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_NORMALIZE_CUBE_MAP);
  m_Text_LightCMap     = LoadTexture("$LightCMap", FT_NOREMOVE | FT_NOMIPS, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_LIGHT_CUBE_MAP);

  m_Text_EnvLCMap = LoadTexture("$EnvironmentLightCMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_ENVIRONMENT_LIGHTCUBE_MAP);
  m_Text_EnvCMap = LoadTexture("$EnvironmentCMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_ENVIRONMENT_CUBE_MAP);
  m_Text_EnvTex  = LoadTexture("$EnvironmentTex", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_ENVIRONMENT_TEX);
  m_Text_EnvScr  = LoadTexture("$EnvironmentScr", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Rectangle, -1.0f, -1.0f, TO_ENVIRONMENT_SCR);
  m_Text_RefractMap = LoadTexture("$RefractMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_REFRACTMAP);
  m_Text_RainMap = LoadTexture("$RainMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_RAINMAP);
  m_Text_Ghost = LoadTexture("$Ghost", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_GHOST);

  m_Text_WaterMap = LoadTexture("$WaterMap", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_TEXTURE_WATERMAP);
  m_Text_WaterMap->m_Width = 512;
  m_Text_WaterMap->m_Height = 512;

  gRenDev->m_TexMan->m_Text_FromLight = LoadTexture("$FromLightCM", FT_NOREMOVE, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_FROMLIGHT);

  int i;

  for (i=0; i<8; i++)
  {
    sprintf(str, "$FromRE_%d", i);
    m_Text_FromRE[i] = LoadTexture(str, FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_FROMRE0+i);
  }

  m_Text_FromObj = LoadTexture("$FromObj_0", FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_FROMOBJ);

  for (i=0; i<MAX_ENVLIGHTCUBEMAPS; i++)
  {
    sprintf(str, "$EnvLCMap_%d", i);
    m_EnvLCMaps[i].m_Id = i;
    m_EnvLCMaps[i].m_Tex = LoadTexture(str, FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_ENVIRONMENT_LIGHTCUBE_MAP_REAL + i);
  }  
  for (i=0; i<MAX_ENVCUBEMAPS; i++)
  {
    sprintf(str, "$EnvCMap_%d", i);
    m_EnvCMaps[i].m_Id = i;
    m_EnvCMaps[i].m_Tex = LoadTexture(str, FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_ENVIRONMENT_CUBE_MAP_REAL + i);
  }  
  for (i=0; i<MAX_ENVTEXTURES; i++)
  {
    sprintf(str, "$EnvTex_%d", i);
    m_EnvTexts[i].m_Id = i;
    m_EnvTexts[i].m_Tex = LoadTexture(str, FT_PROJECTED | FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_ENVIRONMENT_TEX_MAP_REAL + i);
  }  

  for (i=0; i<TO_CUSTOM_CUBE_MAP_LAST-TO_CUSTOM_CUBE_MAP_FIRST+1; i++)
  {
    sprintf(str, "$CustomCMap_%d", i);
    m_CustomCMaps[i].m_Id = i;
    m_CustomCMaps[i].m_Tex = LoadTexture(str, FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_CUSTOM_CUBE_MAP_FIRST + i);
  }  
  for (i=0; i<TO_CUSTOM_TEXTURE_LAST-TO_CUSTOM_TEXTURE_FIRST+1; i++)
  {
    sprintf(str, "$CustomTexture_%d", i);
    m_CustomTextures[i].m_Id = i;
    m_CustomTextures[i].m_Tex = LoadTexture(str, FT_NOREMOVE | FT_NOSTREAM, FT2_NODXT, eTT_Base, -1.0f, -1.0f, TO_CUSTOM_TEXTURE_FIRST + i);
  }  
  for (i=0; i<EFTT_MAX; i++)
  {
    m_Templates[i].m_Bind = EFTT_DIFFUSE + i;
    m_Templates[i].m_Flags |= FT_NOREMOVE;
  }

  GenerateNMPalette();
  GenerateFuncTextures();
}

void CTexMan::GenerateNMPalette()
{
  int i, j, n;
  int iInterpVal[16];

  n = (int)((1.0f - cry_sqrtf(0.986159145832f)) * 127.0f + 0.5f);
  iInterpVal[7] = 127 - n;
  iInterpVal[8] = n + 128;

  n = (int)((1.0f - cry_sqrtf(0.944636702538f)) * 127.0f + 0.5f);
  iInterpVal[6] = 127 - n;
  iInterpVal[9] = n + 128;

  n = (int)((1.0f - cry_sqrtf(0.875432550907f)) * 127.0f + 0.5f);
  iInterpVal[5]  = 127 - n;
  iInterpVal[10] = n + 128;

  n = (int)((1.0f - cry_sqrtf(0.778546690941f)) * 127.0f + 0.5f);
  iInterpVal[4]  = 127 - n;
  iInterpVal[11] = n + 128;

  n = (int)((1.0f - cry_sqrtf(0.653979241848f)) * 127.0f + 0.5f);
  iInterpVal[3]  = 127 - n;
  iInterpVal[12] = n + 128;

  n = (int)((1.0f - cry_sqrtf(0.501730084419f)) * 127.0f + 0.5f);
  iInterpVal[2]  = 127 - n;
  iInterpVal[13] = n + 128;

  n = (int)((1.0f - cry_sqrtf(0.321799308062f)) * 127.0f + 0.5f);
  iInterpVal[1]  = 127 - n;
  iInterpVal[14] = n + 128;

  n = (int)((1.0f - cry_sqrtf(0.11418685317f)) * 127.0f + 0.5f);
  iInterpVal[0]  = 127 - n;
  iInterpVal[15] = n + 128;

  for (i=0; i<256; i++)
  {
    int n;
    if (i <= iInterpVal[0])
    {
      m_NMPaletteLookup[i] = 0;
      continue;
    }
    else
    if (i >= iInterpVal[15])
    {
      m_NMPaletteLookup[i] = 0xf;
      continue;
    }
    else
    {
      n = 0;
      while (true)
      {
        if (i <= iInterpVal[n+1])
        {
          break;
        }
        else
        if (i <= iInterpVal[n+2])
        {
          n++;
          break;
        }
        else
        if (i <= iInterpVal[n+3])
        {
          n += 2;
          break;
        }
        else
        if (i <= iInterpVal[n+4])
        {
          n += 3;
          break;
        }
        else
        if (i <= iInterpVal[n+5])
        {
          n += 4;
          break;
        }
        else
        if (i <= iInterpVal[n+6])
        {
          n += 5;
          break;
        }
        else
        if (i <= iInterpVal[n+7])
        {
          n += 6;
          break;
        }
        else
        {
          n += 7;
          if (n >= 14)
            break;
        }
      }
      if (i-iInterpVal[n] >= iInterpVal[n+1]-i)
        n++;
      m_NMPaletteLookup[i] = n;
    }
  }

  Vec3d v;
  for (i=0; i<16; i++)
  {
    v.x = (iInterpVal[i] - 127.5f) / 128.0f;
    for (j=0; j<16; j++)
    {
      v.y = (iInterpVal[j] - 127.5f) / 128.0f;
      float f = 1.0f - (v.x * v.x + v.y * v.y);
      if (f < 0)
        f = 0;
      v.z = cry_sqrtf(f);
      m_NMPalette[i][j].red = (byte)(cry_floorf(v.x * 127.0f + 0.5f) + 128.0f);
      m_NMPalette[i][j].green = (byte)(cry_floorf(v.y * 127.0f + 0.5f) + 128.0f);
      m_NMPalette[i][j].blue = (byte)(cry_floorf(v.z * 127.0f + 0.5f) + 128.0f);
      m_NMPalette[i][j].alpha = 255;
    }
  }
  m_NMPalette[15][15].red = 128;
  m_NMPalette[15][15].green = 128;
  m_NMPalette[15][15].blue = 128;
  m_NMPalette[15][15].alpha = 255;
}

byte *CTexMan::ConvertNMToPalettedFormat(byte *src, STexPic *ti, ETEX_Format eTF)
{
  int nMips = ti->m_nMips;
  TArray<byte> IdxTex;
  int w = ti->m_Width;
  int h = ti->m_Height;
  int l, i;
  if (!nMips)
    nMips = 1;

  for (l=0; l<nMips; l++)
  {
    if (!w)
      w = 1;
    if (!h)
      h = 1;
    int nCur = IdxTex.Num();
    int nSize = w*h;
    IdxTex.Grow(nSize);
    byte *dst = &IdxTex[nCur];
    for (i=0; i<nSize; i++)
    {
      int j;
      if (eTF == eTF_8888)
        j = i*4;
      else
        j = i*3;
      byte b;
      if (src[j+0] == 0x80 && src[j+1] == 0x80 && src[j+2] == 0x80)
        b = 255;
      else
      {
        b = (m_NMPaletteLookup[src[j+0]]<<4) | m_NMPaletteLookup[src[j+1]];
        if (b == 255)
          b = 254;
      }
      dst[i] = b;
    }
    w >>= 1;
    h >>= 1;
    if (eTF == eTF_8888)
      src += nSize * 4;
    else
      src += nSize * 3;
  }
  byte *dst = new byte [IdxTex.Num()];
  cryMemcpy(dst, &IdxTex[0], IdxTex.Num());

  return dst;
}

void CTexMan::SetGridTexture(STexPic *tp)
{
  int i, j;
  if (tp->m_Width < 8 && tp->m_Height < 8)
  {
    gRenDev->SetTexture(TX_FIRSTBIND, eTT_Base);
    return;
  }
  for (i=0; i<m_TGrids.Num(); i++)
  {
    STexGrid *tg = &m_TGrids[i];
    if (tg->m_Width == tp->m_Width && tg->m_Height == tp->m_Height && tg->m_TP->m_eTT == tp->m_eTT)
      break;
  }
  if (i != m_TGrids.Num())
  {
    m_TGrids[i].m_TP->Set();
    return;
  }
  STexGrid tg;
  tg.m_Width = tp->m_Width;
  tg.m_Height = tp->m_Height;
  STexPic *tpg = LoadTexture("Textures/world_texel", 0, 0, tp->m_eTT);
  if (tpg->m_Width == tp->m_Height && tpg->m_Height == tp->m_Height && tpg->m_eTT == tp->m_eTT)
  {
    tg.m_TP = tpg;
    tg.m_TP->Set();
    m_TGrids.AddElem(tg);
    return;
  }
  byte *src = tpg->GetData32();
  if (!src)
  {
    gRenDev->SetTexture(TX_FIRSTBIND, eTT_Base);
    return;
  }
  byte *dst = new byte[tg.m_Width*tg.m_Height*4];

  int wdt = tg.m_Width;
  int hgt = tg.m_Height;
  int wdt1 = tpg->m_Width;
  int hgt1 = tpg->m_Height;
  int mwdt1 = wdt1-1;
  int mhgt1 = hgt1-1;
  for (j=0; j<hgt; j++)
  {
    for (i=0; i<wdt; i++)
    {
      dst[j*wdt*4+i*4+0] = src[(i&mwdt1)*4+(j&mhgt1)*wdt1*4+0];
      dst[j*wdt*4+i*4+1] = src[(i&mwdt1)*4+(j&mhgt1)*wdt1*4+1];
      dst[j*wdt*4+i*4+2] = src[(i&mwdt1)*4+(j&mhgt1)*wdt1*4+2];
      dst[j*wdt*4+i*4+3] = src[(i&mwdt1)*4+(j&mhgt1)*wdt1*4+3];
    }
  }
  char name[128];
  sprintf(name, "TexGrid_%d_%d", wdt, hgt);
  tg.m_TP = CreateTexture(name, wdt, hgt, 1, 0, 0, dst, tp->m_eTT);
  delete [] dst;
  delete [] src;
  tg.m_TP->Set();
  m_TGrids.AddElem(tg);
}

void CTexMan::PreloadTextures (int Flags)
{
  int i;
  for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
  {
    STexPic *tp = gRenDev->m_TexMan->m_Textures[i];
    if (!tp || tp->m_Bind == TX_FIRSTBIND)
      continue;
    if (tp->m_Flags2 & FT2_RENDERTARGET)
      continue;
    tp->Preload(Flags);
  }
}

bool CTexMan::GetCubeColor(Vec3 *Pos, CFColor *cols)
{
  CRendElement tre;
  CRendElement *re = gRenDev->m_RP.m_pRE;
  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  gRenDev->m_RP.m_pRE = &tre;
  gRenDev->m_RP.m_pCurObject = gRenDev->m_RP.m_TempObjects[0];
  SEnvTexture *cm = &gRenDev->m_TexMan->m_EnvLCMaps[0];
  cm->m_ObjPos = Vec3(0,0,0);
  cm->m_CamPos = *Pos;
  cm->m_bReady = false;
  ScanEnvironmentCube(cm, -1, 16, true);
  for (int n=0; n<6; n++)
  {
    GetAverageColor(cm, n);
    cols[n].r = cm->m_EnvColors[n].bcolor[0] / 255.0f;
    cols[n].g = cm->m_EnvColors[n].bcolor[1] / 255.0f;
    cols[n].b = cm->m_EnvColors[n].bcolor[2] / 255.0f;
    cols[n].a = cm->m_EnvColors[n].bcolor[3] / 255.0f;
  }
  return true;
}
