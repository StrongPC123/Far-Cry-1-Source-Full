/*=============================================================================
  D3DTexturesStreaming.cpp : Direct3D9 specific texture streaming technology.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"

//===============================================================================

//#define TEXUSAGE D3DUSAGE_DYNAMIC
//#define TEXPOOL  D3DPOOL_DEFAULT

#define TEXUSAGE 0
#define TEXPOOL  D3DPOOL_MANAGED

void STexPic::BuildMips()
{
}

void STexPicD3D::BuildMips()
{
  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  IDirect3DTexture9 *pID3DTexture = NULL;
  IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  D3DSURFACE_DESC ddsdDescDest;

  if (!m_Mips[0])
    CreateMips();

  if (m_eTT != eTT_Cubemap)
  {
    pID3DTexture = (IDirect3DTexture9*)m_RefTex.m_VidTex;
    for (int i=0; i<m_nMips; i++)
    {
      pID3DTexture->GetLevelDesc(i, &ddsdDescDest);
      assert (ddsdDescDest.Width && ddsdDescDest.Height);
      int size = CD3D9TexMan::TexSize(ddsdDescDest.Width, ddsdDescDest.Height, ddsdDescDest.Format);
      SAFE_DELETE(m_Mips[0][i]);
      SMipmap *mp = new SMipmap(ddsdDescDest.Width, ddsdDescDest.Height, size);
      m_Mips[0][i] = mp;
      mp->m_bUploaded = true;
      h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
      cryMemcpy(&mp->DataArray[0], d3dlr.pBits, size);
      pID3DTexture->UnlockRect(i);
    }
  }
  else
  {
    pID3DCubeTexture = (IDirect3DCubeTexture9*)m_RefTex.m_VidTex;
    for (int n=0; n<6; n++)
    {
      for (int i=0; i<m_nMips; i++)
      {
        pID3DCubeTexture->GetLevelDesc(i, &ddsdDescDest);
        assert (ddsdDescDest.Width && ddsdDescDest.Height);
        int size = CD3D9TexMan::TexSize(ddsdDescDest.Width, ddsdDescDest.Height, ddsdDescDest.Format);
        SAFE_DELETE(m_Mips[n][i]);
        SMipmap *mp = new SMipmap(ddsdDescDest.Width, ddsdDescDest.Height, size);
        m_Mips[n][i] = mp;
        mp->m_bUploaded = true;
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)n, i, &d3dlr, NULL, 0);
        cryMemcpy(&mp->DataArray[0], d3dlr.pBits, size);
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)n, i);
      }
    }
  }
}

bool STexPic::UploadMips(int nStartMip, int nEndMip)
{
  return false;
}

void STexPic::FakeUploadMips(int nStartMip, int nEndMip)
{
  int i;

  if (CRenderer::CV_r_texturesstreamingonlyvideo)
    m_LoadedSize = 0;
  for (i=nStartMip; i<=nEndMip; i++)
  {
    int nLod = i-m_nBaseMip;
    SMipmap *mp = m_Mips[0][i];
    if (mp->m_bUploaded)
    {
#ifdef _DEBUG
      int nI = i+1;
      while (nI <= nEndMip)
      {
        SMipmap *mp = m_Mips[0][nI];
        assert (mp && mp->m_bUploaded);
        nI++;
      }
#endif
      break;
    }
    m_LoadedSize += m_pFileTexMips[i].m_Size;
    gRenDev->m_TexMan->m_StatsCurTexMem += m_pFileTexMips[i].m_Size;
  }
}

bool STexPicD3D::UploadMips(int nStartMip, int nEndMip)
{
  double time0 = 0;
  ticks(time0);

  CD3D9Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
  IDirect3DTexture9 *pID3DTexture = NULL;
  IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  int i;

  if (CRenderer::CV_r_texturesstreamingonlyvideo)
  {
    if (!AddToPool(nStartMip, m_nMips-nStartMip))
      return true;
    if (!m_pPoolItem)
      return true;
  }

  if (m_eTT != eTT_Cubemap)
  {
    pID3DTexture = (IDirect3DTexture9*)m_RefTex.m_VidTex;
    if (!pID3DTexture)
    {
      if( FAILED( h = dv->CreateTexture(m_Width>>m_nBaseMip, m_Height>>m_nBaseMip, m_nMips-m_nBaseMip, TEXUSAGE, (D3DFORMAT)m_DstFormat, TEXPOOL, &pID3DTexture, NULL)))
        return true;
      m_RefTex.m_VidTex = pID3DTexture;
    }
    int SizeToLoad = 0;
    if (m_pPoolItem)
    {
      for (i=nStartMip; i<=nEndMip; i++)
      {
        int nLod = i-nStartMip;
        SMipmap *mp = m_Mips[0][i];

        gRenDev->m_TexMan->m_StatsCurTexMem += m_pFileTexMips[i].m_Size;
        gRenDev->m_TexMan->m_UpLoadBytes += m_pFileTexMips[i].m_Size;
        LPDIRECT3DSURFACE9 pDestSurf, pSrcSurf;
        h = pID3DTexture->GetSurfaceLevel(nLod, &pDestSurf); 
        pSrcSurf = (LPDIRECT3DSURFACE9)m_pPoolItem->m_pOwner->m_SysSurfaces[nLod];
        h = pSrcSurf->LockRect(&d3dlr, NULL, 0);
#ifdef _DEBUG
        int nD3DSize;
        if (m_DstFormat == D3DFMT_DXT1 || m_DstFormat == D3DFMT_DXT3 || m_DstFormat == D3DFMT_DXT5)
          nD3DSize = (d3dlr.Pitch/4) * ((mp->VSize+3)&~3);
        else
          nD3DSize = d3dlr.Pitch * mp->VSize;
        assert(nD3DSize == m_pFileTexMips[i].m_Size);
#endif
        SizeToLoad += m_pFileTexMips[i].m_Size;
        // Copy data to system texture 
        cryMemcpy((byte *)d3dlr.pBits, &mp->DataArray[0], m_pFileTexMips[i].m_Size);
        // Unlock the system texture
        pSrcSurf->UnlockRect();
        h = dv->UpdateSurface(pSrcSurf, 0, pDestSurf, 0);
        SAFE_RELEASE(pDestSurf);
        mp->m_bUploaded = true;
      }
    }
    else
    {
      pID3DTexture->SetLOD(nStartMip-m_nBaseMip);
      for (i=nStartMip; i<=nEndMip; i++)
      {
        int nLod = i-m_nBaseMip;
        SMipmap *mp = m_Mips[0][i];
        if (mp->m_bUploaded)
        {
#ifdef _DEBUG
          int nI = i+1;
          while (nI <= nEndMip)
          {
            SMipmap *mp = m_Mips[0][nI];
            assert (mp && mp->m_bUploaded);
            nI++;
          }
#endif
          break;
        }
        m_LoadedSize += m_pFileTexMips[i].m_Size;
        gRenDev->m_TexMan->m_StatsCurTexMem += m_pFileTexMips[i].m_Size;
        gRenDev->m_TexMan->m_UpLoadBytes += m_pFileTexMips[i].m_Size;
        h = pID3DTexture->LockRect(nLod, &d3dlr, NULL, 0);
#ifdef _DEBUG
        int nD3DSize;
        if (m_DstFormat == D3DFMT_DXT1 || m_DstFormat == D3DFMT_DXT3 || m_DstFormat == D3DFMT_DXT5)
          nD3DSize = (d3dlr.Pitch/4) * ((mp->VSize+3)&~3);
        else
          nD3DSize = d3dlr.Pitch * mp->VSize;
        assert(nD3DSize == m_pFileTexMips[i].m_Size);
#endif
        SizeToLoad += m_pFileTexMips[i].m_Size;
        // Copy data to video texture 
        cryMemcpy((byte *)d3dlr.pBits, &mp->DataArray[0], m_pFileTexMips[i].m_Size);
        // Unlock the video texture
        pID3DTexture->UnlockRect(nLod);
        mp->m_bUploaded = true;
      }
    }
    if (gRenDev->m_LogFileStr)
      gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Uploading mips '%s'. (Side: %d, %d-%d[%d]), Size: %d, Time: %.3f\n", m_SourceName.c_str(), -1, nStartMip, i-1, m_nMips, SizeToLoad, iTimer->GetAsyncCurTime());
  }
  else
  {
    pID3DCubeTexture = (IDirect3DCubeTexture9*)m_RefTex.m_VidTex;
    if (!pID3DCubeTexture)
    {
      if( FAILED( h = dv->CreateCubeTexture(m_Width>>m_nBaseMip, m_nMips-m_nBaseMip, TEXUSAGE, (D3DFORMAT)m_DstFormat, TEXPOOL, &pID3DCubeTexture, NULL)))
        return true;
      m_RefTex.m_VidTex = pID3DCubeTexture;
    }
    if (!m_pPoolItem)
      pID3DCubeTexture->SetLOD(nStartMip-m_nBaseMip);
    for (int n=0; n<6; n++)
    {
      int SizeToLoad = 0;
      if (m_pPoolItem)
      {
        for (i=nStartMip; i<=nEndMip; i++)
        {
          int nLod = i-nStartMip;
          SMipmap *mp = m_Mips[n][i];

          gRenDev->m_TexMan->m_StatsCurTexMem += m_pFileTexMips[i].m_Size;
          gRenDev->m_TexMan->m_UpLoadBytes += m_pFileTexMips[i].m_Size;
          LPDIRECT3DSURFACE9 pDestSurf, pSrcSurf;
          h = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)n, nLod, &pDestSurf);
          pSrcSurf = (LPDIRECT3DSURFACE9)m_pPoolItem->m_pOwner->m_SysSurfaces[nLod];
          h = pSrcSurf->LockRect(&d3dlr, NULL, 0);
#ifdef _DEBUG
          int nD3DSize;
          if (m_DstFormat == D3DFMT_DXT1 || m_DstFormat == D3DFMT_DXT3 || m_DstFormat == D3DFMT_DXT5)
            nD3DSize = (d3dlr.Pitch/4) * ((mp->VSize+3)&~3);
          else
            nD3DSize = d3dlr.Pitch * mp->VSize;
          assert(nD3DSize == m_pFileTexMips[i].m_Size);
#endif
          SizeToLoad += m_pFileTexMips[i].m_Size;
          // Copy data to system texture 
          cryMemcpy((byte *)d3dlr.pBits, &mp->DataArray[0], m_pFileTexMips[i].m_Size);
          // Unlock the system texture
          pSrcSurf->UnlockRect();
          h = dv->UpdateSurface(pSrcSurf, 0, pDestSurf, 0);
          SAFE_RELEASE(pDestSurf);
          mp->m_bUploaded = true;
        }
      }
      else
      {
        for (i=nStartMip; i<=nEndMip; i++)
        {
          int nLod = i-m_nBaseMip;
          SMipmap *mp = m_Mips[n][i];
          if (mp->m_bUploaded)
          {
#ifdef _DEBUG
            int nI = i+1;
            while (nI <= nEndMip)
            {
              SMipmap *mp = m_Mips[n][nI];
              assert (mp && mp->m_bUploaded);
              nI++;
            }
#endif
            break;
          }
          if (!mp->DataArray.GetSize())
            continue;
          m_LoadedSize += m_pFileTexMips[i].m_Size;
          gRenDev->m_TexMan->m_StatsCurTexMem += m_pFileTexMips[i].m_Size;
          gRenDev->m_TexMan->m_UpLoadBytes += m_pFileTexMips[i].m_Size;
          h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)n, nLod, &d3dlr, NULL, 0);
#ifdef _DEBUG
          int nD3DSize;
          if (m_DstFormat == D3DFMT_DXT1 || m_DstFormat == D3DFMT_DXT3 || m_DstFormat == D3DFMT_DXT5)
            nD3DSize = (d3dlr.Pitch/4) * ((mp->VSize+3)&~3);
          else
            nD3DSize = d3dlr.Pitch * mp->VSize;
          assert(nD3DSize == m_pFileTexMips[i].m_Size);
#endif
          SizeToLoad += m_pFileTexMips[i].m_Size;
          // Copy data to video texture 
          cryMemcpy((byte *)d3dlr.pBits, &mp->DataArray[0], m_pFileTexMips[i].m_Size);
          // Unlock the video texture
          pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)n, nLod);
          mp->m_bUploaded = true;
        }
      }
      if (gRenDev->m_LogFileStr)
        gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Uploading mips '%s'. (Side: %d, %d-%d[%d]), Size: %d, Time: %.3f\n", m_SourceName.c_str(), n, nStartMip, i-1, m_nMips, SizeToLoad, iTimer->GetAsyncCurTime());
    }
  }
  unticks(time0);
  r->m_RP.m_PS.m_fTexUploadTime += (float)(time0*1000.0*g_SecondsPerCycle);

  return true;
}

// Just remove item from the texture object and keep Item in Pool list for future use
// This function doesn't release API texture
void STexPic::RemoveFromPool()
{
  if (!m_pPoolItem)
    return;
  STexPoolItem *pIT = m_pPoolItem;
  m_pPoolItem = NULL;
  pIT->m_pTex = NULL;
  pIT->m_fLastTimeUsed = gRenDev->m_RP.m_RealTime;
  m_LoadedSize = -1;
  m_RefTex.m_VidTex = NULL;
}

bool STexPic::AddToPool(int nStartMip, int nMips)
{
  assert (m_Mips && nStartMip<m_nMips && m_Mips[0][nStartMip]);
  SMipmap * mp = m_Mips[0][nStartMip];
  STexPool *pPool = NULL;
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  IDirect3DTexture9 *pID3DTexture = NULL;
  HRESULT h;

  if (m_pPoolItem)
  {
    if (m_pPoolItem->m_pOwner->m_nMips == nMips && m_pPoolItem->m_pOwner->m_Width == mp->USize && m_pPoolItem->m_pOwner->m_Height == mp->VSize)
      return false;
    STexPoolItem *pIT = m_pPoolItem;
    RemoveFromPool();
    pIT->LinkFree(&gRenDev->m_TexMan->m_FreeTexPoolItems);
    int nSides = 1;
    if (m_eTT == eTT_Cubemap)
      nSides = 6;
    for (int j=0; j<nSides; j++)
    {
      if (!m_Mips[j])
        continue;
      for (int i=0; i<m_nMips; i++)
      {
        if (m_Mips[j][i])
          m_Mips[j][i]->m_bUploaded = false;
      }
    }
    /*IDirect3DBaseTexture9 *pTex = (IDirect3DBaseTexture9*)pIT->m_pAPITexture;
    SAFE_RELEASE(pTex);
    pIT->Unlink();
    gRenDev->m_TexMan->m_StatsCurTexMem -= pIT->m_pOwner->m_Size;
    delete pIT;*/
  }
  pPool = gRenDev->m_TexMan->CreatePool(mp->USize, mp->VSize, nMips, m_DstFormat, m_eTT);
  if (!pPool)
    return false;

  // Try to find empty item in the pool
  STexPoolItem *pIT = pPool->m_ItemsList.m_Next;
  while (pIT != &pPool->m_ItemsList)
  {
    if (!pIT->m_pTex)
      break;
    pIT = pIT->m_Next;
  }
  if (pIT != &pPool->m_ItemsList)
  {
    pIT->UnlinkFree();
    gRenDev->m_TexMan->m_StatsCurTexMem -= pPool->m_Size;
  }
  else
  {
    pIT = new STexPoolItem;
    pIT->m_pOwner = pPool;
    pIT->Link(&pPool->m_ItemsList);

    // Create API texture for the item in DEFAULT pool
    IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
    pID3DTexture = NULL;
    if (m_eTT != eTT_Cubemap)
    {
      if( FAILED( h = dv->CreateTexture(mp->USize, mp->VSize, nMips, TEXUSAGE, (D3DFORMAT)m_DstFormat, D3DPOOL_DEFAULT, &pID3DTexture, NULL)))
      {
        assert(0);
        return false;
      }
      pIT->m_pAPITexture = pID3DTexture;
    }
    else
    {
      if( FAILED( h = dv->CreateCubeTexture(mp->USize, nMips, TEXUSAGE, (D3DFORMAT)m_DstFormat, D3DPOOL_DEFAULT, &pID3DCubeTexture, NULL)))
      {
        assert(0);
        return false;
      }
      pIT->m_pAPITexture = pID3DCubeTexture;
    }
  }
  m_LoadedSize = pPool->m_Size;
  m_pPoolItem = pIT;
  pIT->m_pTex = this;
  m_RefTex.m_VidTex = pIT->m_pAPITexture;

  return true;
}

STexPool *CTexMan::CreatePool(int nWidth, int nHeight, int nMips, int nFormat, ETexType eTT)
{
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  HRESULT h;
  int i;
  IDirect3DTexture9 *pID3DTexture = NULL;

  STexPool *pPool = NULL;

  for (i=0; i<m_TexPools.Num(); i++)
  {
    pPool = m_TexPools[i];
    if (pPool->m_nMips == nMips && pPool->m_Width == nWidth && pPool->m_Height == nHeight && pPool->m_Format == nFormat && pPool->m_eTT == eTT)
      break;
  }
  // Create new pool
  if (i == m_TexPools.Num())
  {
    pPool = new STexPool;
    pPool->m_eTT = eTT;
    pPool->m_Format = nFormat;
    pPool->m_Width = nWidth;
    pPool->m_Height = nHeight;
    pPool->m_nMips = nMips;
    pPool->m_Size = 0;
    int wdt = nWidth;
    int hgt = nHeight;
    for (i=0; i<nMips; i++)
    {
      if (!wdt)
        wdt = 1;
      if (!hgt)
        hgt = 1;
      pPool->m_Size += CD3D9TexMan::TexSize(wdt, hgt, nFormat);
      wdt >>= 1;
      hgt >>= 1;
    }
    if (eTT == eTT_Cubemap)
      pPool->m_Size *= 6;

    if( FAILED( h = dv->CreateTexture(nWidth, nHeight, nMips, TEXUSAGE, (D3DFORMAT)nFormat, D3DPOOL_SYSTEMMEM, &pID3DTexture, NULL)))
      return NULL;
    pPool->m_pSysTexture = pID3DTexture;
    for (i=0; i<nMips; i++)
    {
      LPDIRECT3DSURFACE9 pDestSurf;
      h = pID3DTexture->GetSurfaceLevel(i, &pDestSurf);
      pPool->m_SysSurfaces.AddElem(pDestSurf);
    }
    pPool->m_SysSurfaces.Shrink();

    pPool->m_ItemsList.m_Next = &pPool->m_ItemsList;
    pPool->m_ItemsList.m_Prev = &pPool->m_ItemsList;
    gRenDev->m_TexMan->m_TexPools.AddElem(pPool);
  }

  return pPool;
}

void CTexMan::CreatePools()
{
  int nMaxTexSize = gcpRendD3D->m_pd3dDevice->GetAvailableTextureMem();
  int i = 10;
  int j, n;

  while (i)
  {
    int wdt = (1<<i);
    int hgt = (1<<i);
    int nTexSize = 0;
    nTexSize += CD3D9TexMan::TexSize(wdt, hgt, D3DFMT_DXT1);
    nTexSize += CD3D9TexMan::TexSize(wdt, hgt, D3DFMT_DXT3);
    nTexSize += CD3D9TexMan::TexSize(wdt, hgt, D3DFMT_A8R8G8B8);
    int nH = i-1;
    if (nH < 0)
      nH = 0;
    hgt = (1<<nH);
    nTexSize += CD3D9TexMan::TexSize(wdt, hgt, D3DFMT_DXT1);
    nTexSize += CD3D9TexMan::TexSize(wdt, hgt, D3DFMT_DXT3);
    nTexSize += CD3D9TexMan::TexSize(wdt, hgt, D3DFMT_A8R8G8B8);
    n = nMaxTexSize / nTexSize / i / 2;
    for (j=0; j<n; j++)
    {
      CreatePool(1<<i, 1<<i, i, D3DFMT_DXT1, eTT_Base);
      CreatePool(1<<i, 1<<i, i, D3DFMT_DXT3, eTT_Base);
      CreatePool(1<<i, 1<<i, i, D3DFMT_A8R8G8B8, eTT_Base);

      CreatePool(1<<i, 1<<nH, i, D3DFMT_DXT1, eTT_Base);
      CreatePool(1<<i, 1<<nH, i, D3DFMT_DXT3, eTT_Base);
      CreatePool(1<<i, 1<<nH, i, D3DFMT_A8R8G8B8, eTT_Base);
    }

    i--;
  }
}

void CTexMan::UnloadOldTextures(STexPic *pExclude)
{
  if (!CRenderer::CV_r_texturesstreampoolsize)
    return;
  //ValidateTexSize();

  // First try to release freed texture pool items
  STexPoolItem *pIT = gRenDev->m_TexMan->m_FreeTexPoolItems.m_PrevFree;
  while (pIT != &gRenDev->m_TexMan->m_FreeTexPoolItems)
  {
    assert (!pIT->m_pTex);
    STexPoolItem *pITNext = pIT->m_PrevFree;
    IDirect3DBaseTexture9 *pTex = (IDirect3DBaseTexture9*)pIT->m_pAPITexture;
    SAFE_RELEASE(pTex);
    pIT->Unlink();
    pIT->UnlinkFree();
    m_StatsCurTexMem -= pIT->m_pOwner->m_Size;
    delete pIT;

    pIT = pITNext;
    if (m_StatsCurTexMem < CRenderer::CV_r_texturesstreampoolsize*1024*1024)
      break;
  }

  // Second release old texture objects
  STexPic *pTP = STexPic::m_Root.m_Prev;
  while (m_StatsCurTexMem >= CRenderer::CV_r_texturesstreampoolsize*1024*1024)
  {
    if (pTP == &STexPic::m_Root)
    {
      ICVar *var = iConsole->GetCVar("r_TexturesStreamPoolSize");
      var->Set(m_StatsCurTexMem/(1024*1024)+30);
      iLog->Log("WARNING: Texture pool was changed to %d Mb", CRenderer::CV_r_texturesstreampoolsize);
      return;
    }

    STexPic *Next = pTP->m_Prev;
    if (pTP != pExclude)
      pTP->Unload();
    pTP = Next;
  }
}


void STexPic::PrecacheAsynchronously(float fDist, int Flags)
{
  if (gRenDev->m_bDeviceLost)
    return;

  // Ignore predicted load if we near to texture thrashing
  if (m_nCustomMip)
    return;

  PROFILE_FRAME(Texture_Precache);
  if (gRenDev->m_TexMan->m_Streamed & 1)
  {
    if (m_Flags2 & (FT2_WASUNLOADED | FT2_PARTIALLYLOADED))
    {
      if (!IsStreamed())
        return;
      //if (!(Flags & FPR_NEEDLIGHT) && m_eTT == eTT_Bumpmap)
      //  return;
      if (CRenderer::CV_r_logTexStreaming >= 2)
        gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Precache '%s' (Stream)\n", m_SourceName.c_str());
      LoadFromCache(Flags, fDist);
    }
    else
    {
      if (CRenderer::CV_r_logTexStreaming >= 2)
        gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Precache '%s' (Skip)\n", m_SourceName.c_str());
    }
  }
  if (gRenDev->m_TexMan->m_Streamed & 2)
  {
    if (CRenderer::CV_r_logTexStreaming >= 2)
      gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Precache '%s' (Draw)\n", m_SourceName.c_str());
    gcpRendD3D->EF_SetState(GS_NOCOLMASK | GS_NODEPTHTEST);
    Set();

    int nOffs;
    struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *vTri = gcpRendD3D->GetVBPtr2D(3, nOffs);
    DWORD col = -1;

    // Define the triangle
    vTri[0].x = 0;
    vTri[0].y = 0;
    vTri[0].z = 1;
    vTri[0].rhw = 1.0f;
    vTri[0].color.dcolor = col;
    vTri[0].st[0] = 0;
    vTri[0].st[1] = 0;

    vTri[1].x = 2;
    vTri[1].y = 0;
    vTri[1].z = 1;
    vTri[1].rhw = 1.0f;
    vTri[1].color.dcolor = col;
    vTri[1].st[0] = 1;
    vTri[1].st[1] = 0;

    vTri[2].x = 2;
    vTri[2].y = 2;
    vTri[2].z = 1;
    vTri[2].rhw = 1.0f;
    vTri[2].color.dcolor = col;
    vTri[2].st[0] = 1;
    vTri[2].st[1] = 1;

    gcpRendD3D->UnlockVB2D();

    gcpRendD3D->m_pd3dDevice->SetStreamSource(0, gcpRendD3D->m_pVB2D, 0, sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F));
    gcpRendD3D->EF_SetVertexDeclaration(0, VERTEX_FORMAT_TRP3F_COL4UB_TEX2F);
    gcpRendD3D->m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, nOffs, 1);
    gcpRendD3D->EF_SetState(GS_NODEPTHTEST);
  }
}

void CTexMan::CheckTexLimits(STexPic *pExclude)
{
  ValidateTexSize();
  if (!m_Streamed)
    return;

  // To avoid textures thrashing we will try to predict it using some statistics
  // and using 3-Phase old textures releasing scheme

  m_TexSizeHistory[m_nTexSizeHistory & 7] = gRenDev->m_TexMan->m_StatsCurTexMem;
  m_nTexSizeHistory++;
  bool bUp = true;
  int n = m_nTexSizeHistory-8;
  if (n < 0)
    n = 0;
  int nMinTexSize = 0;
  int nPrev;
  for (int i=n; i<m_nTexSizeHistory; i++)
  {
    int m = i&7;
    if (i == n)
      nMinTexSize = m_TexSizeHistory[m];
    else
      nMinTexSize = min(m_TexSizeHistory[m], nMinTexSize);
    if (bUp)
    {
      if (i == n || m_TexSizeHistory[m] > nPrev)
        nPrev = m_TexSizeHistory[m];
      else
        bUp = false;
    }
  }
  int nPoolSize = CRenderer::CV_r_texturesstreampoolsize*1024*1024;
  if (nMinTexSize < (int)(nPoolSize*0.8f))
  {
    if (m_nCustomMip > 0)
      m_nCustomMip--;
  }
  else
  if (nMinTexSize > (int)(nPoolSize*0.98f))
  {
    if (m_nCustomMip < 2)
      m_nCustomMip++;
  }
  if (!m_nPhaseProcessingTextures)
    m_nPhaseProcessingTextures = 1;

  // Phase1: delete old freed pool items
  if (m_nPhaseProcessingTextures == 1)
  {
    float fTime = gRenDev->m_RP.m_RealTime;
    int n = 20;
    STexPoolItem *pIT = gRenDev->m_TexMan->m_FreeTexPoolItems.m_PrevFree;
    while (pIT != &gRenDev->m_TexMan->m_FreeTexPoolItems)
    {
      assert (!pIT->m_pTex);
      STexPoolItem *pITNext = pIT->m_PrevFree;
      if (fTime-pIT->m_fLastTimeUsed > 2.0f)
      {
        IDirect3DBaseTexture9 *pTex = (IDirect3DBaseTexture9*)pIT->m_pAPITexture;
        SAFE_RELEASE(pTex);
        pIT->Unlink();
        pIT->UnlinkFree();
        m_StatsCurTexMem -= pIT->m_pOwner->m_Size;
        nMinTexSize -= pIT->m_pOwner->m_Size;
        delete pIT;
        n--;
      }
      pIT = pITNext;
      if (!n || nMinTexSize <= (int)(nPoolSize*0.88f))
        break;
    }
    if (nMinTexSize <= (int)(nPoolSize*0.88f))
      m_nPhaseProcessingTextures = 0;
    else
    if (pIT == &gRenDev->m_TexMan->m_FreeTexPoolItems)
    {
      m_nProcessedTextureID1 = 0;
      m_pProcessedTexture1 = NULL;
      m_nPhaseProcessingTextures = 2;
    }
  }

  // Phase2: unload old textures (which was used at least 2 secs ago)
  if (m_nPhaseProcessingTextures == 2)
  {
    STexPic *tp;
    if (!m_nProcessedTextureID1 || !(tp=m_Textures[m_nProcessedTextureID1]) || !tp->m_Prev || tp != m_pProcessedTexture1)
      tp = STexPic::m_Root.m_Prev;
    int nFrame = gRenDev->GetFrameID();
    int nWeight = 0;
    while (tp != &STexPic::m_Root)
    {
      STexPic *Next = tp->m_Prev;
      if (tp->m_AccessFrame && nFrame-tp->m_AccessFrame > 100 && !(tp->m_Flags & FT_NOSTREAM))
      {
        nWeight += 10;
        tp->Unload();
      }
      tp = Next;
      nWeight++;
      if (nWeight > 100)
        break;
    }
    if (tp == &STexPic::m_Root)
      m_nPhaseProcessingTextures = 0;
    else
    {
      m_nProcessedTextureID1 = tp->m_Id;
      m_pProcessedTexture1 = tp;
    }
  }

  {
    STexPic *tp;
    if (!m_nProcessedTextureID2 || !(tp=m_Textures[m_nProcessedTextureID2]) || !tp->m_Prev || tp != m_pProcessedTexture2)
      tp = STexPic::m_Root.m_Prev;
    else
      tp = m_Textures[m_nProcessedTextureID2];
    int nFrame = gRenDev->GetFrameID();
    int nWeight = 0;
    while (tp != &STexPic::m_Root)
    {
      STexPic *Next = tp->m_Prev;
      if (tp->m_fMinDistance)
      {
        if ((tp->m_Width >64 || tp->m_Height > 64) && tp->m_nCustomMip != m_nCustomMip)
        {
          nWeight += 5;
          tp->m_nCustomMip = m_nCustomMip;
          tp->m_Flags2 |= FT2_NEEDTORELOAD;
        }
      }
      tp = Next;
      nWeight++;
      if (nWeight > 100)
        break;
    }
    if (tp == &STexPic::m_Root)
    {
      m_nProcessedTextureID2 = 0;
      m_pProcessedTexture2 = NULL;
    }
    else
    {
      m_nProcessedTextureID2 = tp->m_Id;
      m_pProcessedTexture2 = tp;
    }
  }

  if (!(gRenDev->m_TexMan->m_Streamed & 1))
    return;
  if (CRenderer::CV_r_texturesstreampoolsize < 10)
    CRenderer::CV_r_texturesstreampoolsize = 10;

  ValidateTexSize();

  // Usually we do it in case of textures thrashing
  if (gRenDev->m_TexMan->m_StatsCurTexMem >= CRenderer::CV_r_texturesstreampoolsize*1024*1024)
    UnloadOldTextures(pExclude);
}


