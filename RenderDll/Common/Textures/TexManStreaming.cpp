/*=============================================================================
  TexManStreaming.cpp : Common Texture manager implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/


#include "RenderPCH.h"
#include "../CommonRender.h"
#include "Image/DDSImage.h"
#include "Image/dds.h"

// DXT compressed block for black image (DXT1, DXT3, DXT5)
static byte sDXTData[3][16] = 
{
  {0,0,0,0,0,0,0,0},
  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0,0,0,0,0,0,0,0},
  {0xff,0xff,0,0,0,0,0,0, 0,0,0,0,0,0,0,0}
};

void STexPic::CreateMips()
{
  int nSides = (m_eTT == eTT_Cubemap) ? 6 : 1;
  int nMips = m_nMips;
  for (int i=0; i<nSides; i++)
  {
    if (!m_Mips[i])
    {
      m_Mips[i] = new SMipmap*[nMips];
      memset(m_Mips[i], 0, sizeof(SMipmap *)*nMips);
    }
  }
}

void STexPic::RemoveMips(int nFromEnd)
{
  int nSides = (m_eTT == eTT_Cubemap) ? 6 : 1;
  int nMips = m_nMips;
  bool bRelease;
  if (!nFromEnd)
    bRelease = true;
  else
    bRelease = false;
  if (nMips <= 1)
    nFromEnd = 0;
  int nEnd = nMips-nFromEnd;
  {
    for (int i=0; i<nSides; i++)
    {
      if (!m_Mips[i])
        continue;
      for (int j=0; j<nEnd; j++)
      {
        if (bRelease)
        {
          SAFE_DELETE(m_Mips[i][j]);
        }
        else
        if (m_Mips[i][j])
          m_Mips[i][j]->DataArray.Free();
      }
      if (bRelease)
        SAFE_DELETE_ARRAY(m_Mips[i]);
    }
  }
}

void STexPic::Unload()
{
  if (m_Flags2 & FT2_WASUNLOADED)
    return;
  if (!IsStreamed())
    return;

//  Set();
  if (gRenDev->m_TexMan->m_Streamed & 1)
  {
    if (gRenDev->m_LogFileStr)
      gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Unload '%s', Time: %.3f\n", m_SourceName.c_str(), iTimer->GetAsyncCurTime());
    SaveToCache();
    ReleaseDriverTexture();
    m_Flags2 |= FT2_WASUNLOADED;
    gRenDev->SetTexture(0, eTT_Base);
    return;
  }
}

void STexPic::Restore()
{
  if (gRenDev->m_TexMan->m_Streamed & 1)
  {
    // Check the distance
    float fDist = -1.0f;
    if (gRenDev->m_RP.m_pRE)
    {
#ifndef PIPE_USE_INSTANCING
      fDist = gRenDev->m_RP.m_pRE->mfDistanceToCameraSquared(*gRenDev->m_RP.m_pCurObject);
#else
      fDist = 999999.0f;
      int nObj = 0;
      CCObject *pSaveObj = gRenDev->m_RP.m_pCurObject;
      CCObject *pObj = pSaveObj;
      while (true)
      {      	
        if (nObj)
        {
          gRenDev->m_RP.m_pCurObject = gRenDev->m_RP.m_MergedObjects[nObj];
          gRenDev->m_RP.m_FrameObject++;
          pObj = gRenDev->m_RP.m_pCurObject;
        }
        fDist = min(fDist, gRenDev->m_RP.m_pRE->mfDistanceToCameraSquared(*pObj));
        nObj++;
        if (nObj >= gRenDev->m_RP.m_MergedObjects.Num())
          break;
      }
      if (pSaveObj != gRenDev->m_RP.m_pCurObject)
      {
        gRenDev->m_RP.m_pCurObject = pSaveObj;
        gRenDev->m_RP.m_FrameObject++;
      }
#endif
      fDist = crySqrtf(fDist);
      if (m_Flags2 & FT2_NEEDTORELOAD)
        fDist = min(m_fMinDistance, fDist);
    }
    //gRenDev->m_TexMan->CheckTexLimits(this);
    LoadFromCache(FPR_IMMEDIATELLY, fDist);
    gRenDev->SetTexture(0, eTT_Base);
    return;
  }
}

static char *sETFToStr(ETEX_Format ETF)
{
  char *sETF;
  switch (ETF)
  {
    case eTF_Index:
      sETF = "Index";
      break;
    case eTF_0888:
      sETF = "0888";
      break;
    case eTF_8888:
      sETF = "8888";
      break;
    case eTF_8000:
      sETF = "8000";
      break;
    case eTF_0088:
      sETF = "0088";
      break;
    case eTF_DXT1:
      sETF = "DXT1";
      break;
    case eTF_DXT3:
      sETF = "DXT3";
      break;
    case eTF_DXT5:
      sETF = "DXT5";
      break;
    case eTF_SIGNED_HILO16:
      sETF = "SIGNED_HILO16";
      break;
    case eTF_DSDT_MAG:
      sETF = "DSDT_MAG";
      break;
    case eTF_DSDT:
      sETF = "DSDT";
      break;
    case eTF_RGB8:
      sETF = "RGB8";
      break;
    case eTF_RGBA:
      sETF = "RGBA";
      break;
    default:
      assert(0);
      sETF = "Unknown";		// for better behaviour in non debug
      break;
  }
  return sETF;
}

void STexPic::GetCacheName(char *name)
{
  char *sETT;
  switch (m_eTT)
  {
    case eTT_Base:
      sETT = "Base";
      break;
    case eTT_DSDTBump:
      sETT = "DSDTBump";
      break;
    case eTT_Bumpmap:
      sETT = "Normalmap";
      break;
    case eTT_Cubemap:
      sETT = "Cubemap";
      break;
    case eTT_Rectangle:
      sETT = "Rectangle";
      break;
  }
  char nam[512];
  if (!strncmp("Spr_$", m_SourceName.c_str(), 5))
    strcpy(nam, m_SourceName.c_str());
  else
    StripExtension(m_SourceName.c_str(), nam);
  sprintf(name, "%s[%s]", nam, sETT); 
}

void STexPic::SaveToCache()
{
  int i, j;

  if (m_CacheID >= 0 || (m_Flags2 & FT2_STREAMFROMDDS))
    return;

  CResFile *rf;
  rf = gRenDev->m_TexMan->m_TexCache;
  SDirEntry de;
  char name[512];
  GetCacheName(name);
  if (!rf)
    gRenDev->m_TexMan->CreateCacheFile();
  rf = gRenDev->m_TexMan->m_TexCache;
  bool bKeepPlace = false;
  if (rf->mfFileExist(name)>=0)
  {
    if (m_Flags2 & FT2_VERSIONWASCHECKED)
      return;
    m_Flags2 |= FT2_VERSIONWASCHECKED;

    if (m_CacheID < 0)
      m_CacheID = rf->mfFileGetNum(name);
    if (m_Flags2 & FT2_DISCARDINCACHE)
    {
      STexCacheFileHeader fh;
      STexCacheMipHeader mh;
      rf->mfFileSeek(m_CacheID, 0, SEEK_SET);
      rf->mfFileRead2(m_CacheID, sizeof(STexCacheFileHeader), &fh);
      rf->mfFileRead2(m_CacheID, sizeof(STexCacheMipHeader), &mh);
      if (m_Width == mh.m_USize && m_Height == mh.m_VSize)
        bKeepPlace = true;
    }
    if (!bKeepPlace)
    {
      STexCacheFileHeader fh;
      rf->mfFileSeek(m_CacheID, 0, SEEK_SET);
      rf->mfFileRead2(m_CacheID, sizeof(STexCacheFileHeader), &fh);
      if (fh.m_sExt[0] == 0)
      {
        if (fh.m_nMips == m_nMips)
          return;
        else
        {
          rf->mfFileDelete(m_CacheID);
          m_CacheID = -1;
        }
      }
      if (m_CacheID >= 0)
      {
        char nameInRes[512];
        char nameFile[2][512];
        int nFiles = GetFileNames(nameFile[0], nameFile[1], 128);
        int n = 0;
        while (name[n] != '[')
        {
          nameInRes[n] = name[n];
          nameInRes[n+1] = 0;
          n++;
        }
        strcat(nameInRes, fh.m_sExt);
        if (!stricmp(nameInRes, m_SourceName.c_str()))
        {
          int nCompares = 0;
          for (i=0; i<nFiles; i++)
          {
            HANDLE status2 = CreateFile(nameFile[i],GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            if (status2 != INVALID_HANDLE_VALUE)
            {
              FILETIME writetime1,writetime2;
              writetime1 = fh.m_WriteTime[i];
              
              GetFileTime(status2,NULL,NULL,&writetime2);
              
              CloseHandle(status2);
              if (CompareFileTime(&writetime1,&writetime2)==0)
                nCompares++;
            }
          }
          if (nCompares == nFiles)
            return;
        }
        rf->mfFileDelete(m_CacheID);
        m_CacheID = -1;
      }
    }
  }

  STexCacheMipHeader mh;
  STexCacheFileHeader fh;
  TArray<byte> Data;

  memset(&fh, 0, sizeof(fh));

  BuildMips();

  int nSides;
  if (m_Mips[5])
    nSides = 6;
  else
    nSides = 1;

  fh.m_SizeOf = sizeof(fh);
  fh.m_Version = TEXCACHE_VERSION;
  fh.m_nSides = nSides;
  fh.m_nMips = m_nMips;
  fh.m_DstFormat = m_DstFormat;
  fh.m_bPolyBump = false;
  fh.m_bCloneSpace = false;
  const char *pExt;
  if (m_SourceName.c_str()[0] && (pExt=GetExtension(m_SourceName.c_str())))
  {
    if (strlen(pExt) < 6)
      strncpy(fh.m_sExt, pExt, 6);
    else
      fh.m_sExt[0] = 0;
  }
  else
    fh.m_sExt[0] = 0;
  strcpy(fh.m_sETF, sETFToStr(m_ETF));
  char nameFile[2][128];
  int nFiles = GetFileNames(nameFile[0], nameFile[1], 128);
  for (i=0; i<nFiles; i++)
  {
    HANDLE status = CreateFile(nameFile[i],GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);  
    if (status != INVALID_HANDLE_VALUE)
    {
      FILETIME writetime;  
      GetFileTime(status,NULL,NULL,&writetime);
      CloseHandle(status);
      fh.m_WriteTime[i] = writetime;
    }
    else
    {
      fh.m_WriteTime[i].dwHighDateTime = 0;
      fh.m_WriteTime[i].dwLowDateTime = 0;
    }
  }
  m_Flags2 |= FT2_VERSIONWASCHECKED;
  int n = Data.Num();
  Data.Grow(sizeof(STexCacheFileHeader));
  memcpy(&Data[n], &fh, sizeof(STexCacheFileHeader));

  for (i=0; i<m_nMips; i++)
  {
    SMipmap *mp = m_Mips[0][i];
    mh.m_SizeOf = sizeof(mh);
    mh.m_USize = mp->USize;
    mh.m_VSize = mp->VSize;
    mh.m_Size = mp->DataArray.Num();
    mh.m_SizeWithMips = 0;
    for (j=i; j<m_nMips; j++)
    {
      mh.m_SizeWithMips += m_Mips[0][j]->DataArray.Num();
    }
    int n = Data.Num();
    Data.Grow(sizeof(STexCacheMipHeader));
    memcpy(&Data[n], &mh, sizeof(STexCacheMipHeader));
  }

  for (j=0; j<nSides; j++)
  {
    for (i=0; i<m_nMips; i++)
    {
      SMipmap *mp = m_Mips[j][i];
      int n = Data.Num();
      Data.Grow(mp->DataArray.Num());
      memcpy(&Data[n], &mp->DataArray[0], mp->DataArray.Num());
    }
  }

  SDirEntry *pde = rf->mfGetEntry(m_CacheID);
  if (bKeepPlace && Data.Num() == pde->size)
  {
    iSystem->GetIPak()->FSeek(rf->mfGetHandle(), pde->offset, SEEK_SET);
    iSystem->GetIPak()->FWrite(&Data[0], 1, Data.Num(), rf->mfGetHandle());
  }
  else
  {
    memset(&de, 0, sizeof(de));
    de.ID = CName(name, eFN_Add).GetIndex();
    de.size = Data.Num();
    de.eid = eRI_BIN;
    de.user.data = &Data[0];
    de.earc = eARC_NONE;
    rf->mfFileAdd(&de);
    rf->mfFlush();
  }
  RemoveMips(4);
}

bool STexPic::ValidateMipsData(int nStartMip, int nEndMip)
{
  int i, j;
  bool bOk = true;
  if (m_eTT == eTT_Cubemap)
  {
    for (j=0; j<6; j++)
    {
      for (i=nStartMip; i<=nEndMip; i++)
      {
        SMipmap *mp = m_Mips[j][i];
        if (!CRenderer::CV_r_texturesstreamingonlyvideo)
        {
          if (mp && mp->m_bUploaded)
            continue;
        }
        if (!mp || !mp->DataArray.GetSize())
        {
          bOk = false;
          break;
        }
      }
    }
  }
  else
  {
    for (i=nStartMip; i<=nEndMip; i++)
    {
      SMipmap *mp = m_Mips[0][i];
      if (!CRenderer::CV_r_texturesstreamingonlyvideo)
      {
        if (mp && mp->m_bUploaded)
          continue;
      }
      if (!mp || !mp->DataArray.GetSize())
      {
        bOk = false;
        break;
      }
    }
  }
  return bOk;
}

void CTextureStreamCallback::StreamOnComplete (IReadStream* pStream, unsigned nError)
{
  PROFILE_FRAME(Texture_AsyncUpload);

  int i, j;
  STexCacheInfo *pTexCacheFileInfo = (STexCacheInfo *)pStream->GetUserData();
  if (gRenDev->m_TexMan->m_Textures.Num() <= pTexCacheFileInfo->m_TexId)
    return;
  STexPic *tp = gRenDev->m_TexMan->m_Textures[pTexCacheFileInfo->m_TexId];
  if (!tp || !tp->m_pFileTexMips)
  {
    //Warning( VALIDATOR_FLAG_TEXTURE,tp->GetName(),"CTextureStreamCallback::StreamOnComplete texture is missing %s",tp->GetName() );
    return;
  }
  int n = 0;
  byte *Src = (byte *)pStream->GetBuffer();
  STexCacheFileHeader *fh = &tp->m_CacheFileHeader;
  STexCacheMipHeader *mh = tp->m_pFileTexMips;
  int nMips = fh->m_nMips;
  int nSide = pTexCacheFileInfo->m_nCubeSide;
  int nS = nSide < 0 ? 0 : nSide;
  if (gRenDev->m_LogFileStr)
    gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Async Finish Load '%s' (Side: %d, %d-%d[%d]), Size: %d, Time: %.3f\n", tp->m_SourceName.c_str(), nS, pTexCacheFileInfo->m_nStartLoadMip, pTexCacheFileInfo->m_nEndLoadMip, tp->m_nMips, pTexCacheFileInfo->m_nSizeToLoad, iTimer->GetAsyncCurTime());
  for (i=pTexCacheFileInfo->m_nStartLoadMip; i<=pTexCacheFileInfo->m_nEndLoadMip; i++)
  {
    SMipmap *mp = tp->m_Mips[nS][i];
    assert(mp);

    mp->m_bLoading = false;
    if (!mp->m_bUploaded || (CRenderer::CV_r_texturesstreamingonlyvideo && !mp->DataArray.GetSize())) // Already uploaded synchronously
    {
      double time0 = 0;
      ticks(time0);

      if (!mp->DataArray.GetSize())
        mp->DataArray.Alloc(mh[i].m_Size);
      if (tp->m_ETF == eTF_0888 && (tp->m_Flags2 & FT2_STREAMFROMDDS))
      {
        byte *pTemp = new byte[mp->USize*mp->VSize*3];
        cryMemcpy(pTemp, &Src[n], mp->USize*mp->VSize*3);
        for (int n=0; n<mp->USize*mp->VSize; n++)
        {
          mp->DataArray[n*4+0] = pTemp[n*3+0];
          mp->DataArray[n*4+1] = pTemp[n*3+1];
          mp->DataArray[n*4+2] = pTemp[n*3+2];
          mp->DataArray[n*4+3] = 255;
        }
        delete [] pTemp;
      }
      else
        cryMemcpy(&mp->DataArray[0], &Src[n], mh[i].m_Size);

      unticks(time0);
      gRenDev->m_RP.m_PS.m_fTexUploadTime += (float)(time0*1000.0*g_SecondsPerCycle);
    }
    if (tp->m_ETF == eTF_0888 && (tp->m_Flags2 & FT2_STREAMFROMDDS))
      n += mh[i].m_USize*mh[i].m_VSize*3;
    else
      n += mh[i].m_Size;
  }
  gRenDev->m_TexMan->ValidateTexSize();
  bool bNeedToWait = false;
  if (nSide >= 0)
  {
    if (tp->m_Flags2 & (FT2_REPLICATETOALLSIDES | FT2_FORCECUBEMAP))
    {
      for (j=1; j<6; j++)
      {
        for (int i=pTexCacheFileInfo->m_nStartLoadMip; i<=pTexCacheFileInfo->m_nEndLoadMip; i++)
        {
          if (!tp->m_Mips[j][i])
            tp->m_Mips[j][i] = new SMipmap(mh[i].m_USize, mh[i].m_VSize);
          SMipmap *mp = tp->m_Mips[j][i];
          if (!mp->DataArray.GetSize())
            mp->DataArray.Alloc(mh[i].m_Size);
          if (tp->m_Flags2 & FT2_REPLICATETOALLSIDES)
            cryMemcpy(&mp->DataArray[0], &tp->m_Mips[0][i]->DataArray[0], mp->DataArray.GetSize());
          else
          if (tp->m_Flags2 & FT2_FORCECUBEMAP)
          {
            if (tp->m_ETF == eTF_DXT1 || tp->m_ETF == eTF_DXT3 || tp->m_ETF == eTF_DXT5)
            {
              int nBlocks = ((mp->USize+3)/4)*((mp->VSize+3)/4);
              int blockSize = tp->m_ETF == eTF_DXT1 ? 8 : 16;
              int nOffsData = tp->m_ETF - eTF_DXT1;
              int nCur = 0;
              for (int n=0; n<nBlocks; n++)
              {
                cryMemcpy(&mp->DataArray[nCur], sDXTData[nOffsData], blockSize);
                nCur += blockSize;
              }
            }
            else
              memset(&mp->DataArray[0], 0, mp->DataArray.GetSize());
          }
        }
      }
    }

    for (j=0; j<6; j++)
    {
      if (!tp->m_Mips[j])
      {
        bNeedToWait = true;
        break;
      }
      for (i=pTexCacheFileInfo->m_nStartLoadMip; i<=pTexCacheFileInfo->m_nEndLoadMip; i++)
      {
        SMipmap *mp = tp->m_Mips[j][i];
        if (!mp || !mp->DataArray.GetSize())
        {
          bNeedToWait = true;
          break;
        }
      }
      if (i <= pTexCacheFileInfo->m_nEndLoadMip)
        break;
    }
  }
  if (!bNeedToWait)
  {
    bool bOk = tp->ValidateMipsData(pTexCacheFileInfo->m_nStartLoadMip, nMips-1);
    if (!bOk)
    {
      tp->RemoveMips(4);
      tp->m_Flags2 |= FT2_WASUNLOADED;
    }
    else
    {
      bool bRes = tp->UploadMips(pTexCacheFileInfo->m_nStartLoadMip, nMips-1);
      if (!pTexCacheFileInfo->m_nStartLoadMip && bRes)
        tp->RemoveMips(4);
      tp->m_Flags2 &= ~FT2_WASUNLOADED;

      if (!tp->m_Mips[0][0] || !tp->m_Mips[0][0]->m_bUploaded)
        tp->m_Flags2 |= FT2_PARTIALLYLOADED;
      else
        tp->m_Flags2 &= ~FT2_PARTIALLYLOADED;
    }
    tp->Relink(&STexPic::m_Root);
    gRenDev->m_TexMan->CheckTexLimits(NULL);
  }
  SAFE_DELETE(pTexCacheFileInfo);
  tp->m_Flags2 &= ~FT2_STREAMINGINPROGRESS;

  gRenDev->m_TexMan->ValidateTexSize();
}

int STexPic::UpdateMip(float fDist)
{
  int nMip = 0;
  if (fDist)
  {
    fDist *= gRenDev->m_TexMan->m_fStreamDistFactor;
    nMip = min(m_nMips-1, (int)(fDist / (float)CRenderer::CV_r_texturespixelsize));
  }
  if ((m_Width > 64 || m_Height > 64) && !CRenderer::CV_r_texturesstreamingsync)
  {
    int nResMip = CRenderer::CV_r_texturesbasemip;
    if (m_eTT == eTT_Bumpmap)
      nResMip += CRenderer::CV_r_texbumpresolution + m_nCustomMip;
    else
      nResMip += CRenderer::CV_r_texresolution + m_nCustomMip;
    nResMip = min(m_nMips-1, nResMip);
    m_nBaseMip = nResMip;
    nMip = min(m_nMips-1, nMip+nResMip);
  }
  else
    m_nBaseMip = 0;

  return nMip;
}

void STexPic::LoadFromCache(int Flags, float fDist)
{
  int i, j;

  PROFILE_FRAME(Texture_LoadFromCache);

  int nStartMip = 0;
  if (fDist >= 0)
  {
    //float fTSize = (float)Max(m_Width, m_Height);
    if (m_nFrameCache != gRenDev->GetFrameID())
    {
      m_nFrameCache = gRenDev->GetFrameID();
      m_fMinDistance = fDist;
    }
    else
    if (fDist < m_fMinDistance)
      m_fMinDistance = fDist;

    nStartMip = UpdateMip(m_fMinDistance);
  }
  else
  if (m_nFrameCache != gRenDev->GetFrameID())
    nStartMip = UpdateMip(0);
  else
    nStartMip = UpdateMip(m_fMinDistance);

  if (m_Mips[0] && m_Mips[0][nStartMip] && m_Mips[0][nStartMip]->m_bUploaded)
  {
    if ((m_Flags2 & FT2_NEEDTORELOAD))
    {
      if (m_Mips[0][nStartMip]->DataArray.GetSize())
      {
        gRenDev->m_TexMan->ValidateTexSize();
        //STexPic ttt;
        //memcpy (&ttt, this, sizeof(ttt));
        //memset (this, &ttt, sizeof(ttt));
        if (ValidateMipsData(nStartMip, m_nMips-1))
          UploadMips(nStartMip, m_nMips-1);
        m_Flags2 &= ~FT2_NEEDTORELOAD;
        for (i=0; i<nStartMip; i++)
        {
          if (m_Mips[0][i])
            m_Mips[0][i]->m_bUploaded = false;
        }
        if (!m_Mips[0][0] || !m_Mips[0][0]->m_bUploaded)
          m_Flags2 |= FT2_PARTIALLYLOADED;
        else
          m_Flags2 &= ~FT2_PARTIALLYLOADED;
        gRenDev->m_TexMan->ValidateTexSize();
        return;
      }
      else
      {
        for (i=0; i<m_nMips; i++)
        {
          if (m_Mips[0][i])
            m_Mips[0][i]->m_bUploaded = false;
        }
      }
    }
    else
      return;
  }
  gRenDev->m_TexMan->ValidateTexSize();

  m_Flags2 &= ~FT2_NEEDTORELOAD;

  CResFile *rf;
  rf = gRenDev->m_TexMan->m_TexCache;

  int nMips, nSides;
  if (!m_pFileTexMips)
  {
    if (m_CacheID < 0)
    {
      char name[512];
      GetCacheName(name);
      if (rf->mfFileExist(name)<0)
        return;  // Should never happen
      m_CacheID = rf->mfFileGetNum(name);
    }
    assert(m_CacheID >= 0);
    rf->mfFileSeek(m_CacheID, 0, SEEK_SET);
    rf->mfFileRead2(m_CacheID, sizeof(STexCacheFileHeader), &m_CacheFileHeader);
    nSides = m_CacheFileHeader.m_nSides;
    nMips = m_CacheFileHeader.m_nMips;
    m_pFileTexMips = new STexCacheMipHeader[nMips];
    rf->mfFileRead2(m_CacheID, sizeof(STexCacheMipHeader)*nMips, m_pFileTexMips);
  }

  STexCacheFileHeader *fh = &m_CacheFileHeader;
  STexCacheMipHeader *mh = m_pFileTexMips;
  nSides = fh->m_nSides;
  nMips = fh->m_nMips;

  assert(nMips == m_nMips);

  if (!m_Mips[0])
    CreateMips();

  int Size;
  int nEndMip = m_nMips-1;
  int n = nEndMip;
  int nSyncStartMip = -1;
  int nSyncEndMip = -1;
  int nASyncStartMip = -1;
  int nASyncEndMip = -1;
  if (nMips == 1 || CRenderer::CV_r_texturesstreamingsync)
  {
    nSyncStartMip = nStartMip;
    nSyncEndMip = nEndMip;
  }
  else
  {
    // Always stream lowest 4 mips synchronously
    int nStartLowestM = max(nStartMip, nEndMip-3);
    for (i=nStartLowestM; i<=nEndMip; i++)
    {
      if (!m_Mips[0][i] || !m_Mips[0][i]->m_bUploaded)
      {
        nSyncStartMip = i;
        for (j=i+1; j<=nEndMip; j++)
        {
          if (m_Mips[0][j] && m_Mips[0][j]->m_bUploaded)
            break;
        }
        nSyncEndMip = j-1;
        break;
      }
    }
    // Let's see which part of the texture not loaded yet and stream it asynhronously
    if (nStartLowestM > nStartMip)
    {
      for (i=nStartMip; i<=nStartLowestM-1; i++)
      {
        if (!m_Mips[0][i] || (!m_Mips[0][i]->m_bLoading && !m_Mips[0][i]->m_bUploaded))
        {
          nASyncStartMip = i;
          break;
        }
      }
      if (nASyncStartMip >= 0 && nASyncStartMip <= nStartLowestM-1)
      {
        for (i=nASyncStartMip; i<=nStartLowestM-1; i++)
        {
          if (m_Mips[0][i] && (m_Mips[0][i]->m_bUploaded || m_Mips[0][i]->m_bLoading))
            break;
        }
        nASyncEndMip = i-1;
      }
    }
  }
  if (nASyncStartMip < 0 && nSyncStartMip < 0)
    return;

  if (!(Flags & FPR_IMMEDIATELLY))
  {
    int nSize = 0;
    for (i=nSyncStartMip; i<=nSyncEndMip; i++)
    {
      nSize += mh[i].m_Size;
    }
    for (i=nASyncStartMip; i<=nASyncEndMip; i++)
    {
      nSize += mh[i].m_Size;
    }
    int n10Perc = CRenderer::CV_r_texturesstreampoolsize*1024*1024/100*10;
    if (gRenDev->m_TexMan->m_StatsCurTexMem+nSize+n10Perc >= CRenderer::CV_r_texturesstreampoolsize*1024*1024)
      return;
  }

  Relink(&STexPic::m_Root);

  IStreamEngine *pSE = iSystem->GetStreamEngine();
  FILE *fp = NULL;

  // Synchronous loading
  static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
  if (nSyncStartMip >= 0)
  {
    PROFILE_FRAME(Texture_LoadFromCacheSync);

    assert(nSyncStartMip <= nSyncEndMip);
    int nSeekFromStart = 0;
    for (i=0; i<nSyncStartMip; i++)
    {
      if (m_ETF == eTF_0888 && (m_Flags2 & FT2_STREAMFROMDDS))
        nSeekFromStart += mh[i].m_USize*mh[i].m_VSize*3;
      else
        nSeekFromStart += mh[i].m_Size;
    }
    if (nSides > 1)
      Size = m_Size/6;
    else
      Size = m_Size;

    for (j=0; j<nSides; j++)
    {
      if (m_CacheID < 0)
      {
        if (j)
        {
          if (fp)
            iSystem->GetIPak()->FClose(fp);
          char name[512];
          StripExtension(m_SourceName.c_str(), name);
          size_t len = strlen(name);
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
          char cube[512];
          sprintf(cube, "%s_%s.dds", name, cubefaces[j]);
          fp = iSystem->GetIPak()->FOpen(cube, "rb");
        }
        if (!j || !(m_Flags2 & FT2_FORCECUBEMAP | FT2_REPLICATETOALLSIDES))
        {
          if (!fp)
            fp = iSystem->GetIPak()->FOpen(m_SourceName.c_str(), "rb");
        }
        if (fp)
          iSystem->GetIPak()->FSeek(fp, sizeof(DWORD)+sizeof(DDS_HEADER)+nSeekFromStart, SEEK_SET);
      }
      else
        rf->mfFileSeek(m_CacheID, sizeof(STexCacheFileHeader)+nMips*sizeof(STexCacheMipHeader)+Size*j+nSeekFromStart, SEEK_SET);
      int SizeToLoad = 0;
      for (i=nSyncStartMip; i<=nSyncEndMip; i++)
      {
        if (m_Mips[j][i] && (m_Mips[j][i]->DataArray.GetSize() || m_Mips[j][i]->m_bUploaded))
        {
          i++;
          break;
        }
        SMipmap *mp;
        if (!m_Mips[j][i])
          m_Mips[j][i] = new SMipmap(mh[i].m_USize, mh[i].m_VSize);
        mp = m_Mips[j][i];
        if (!mp->DataArray.GetSize())
          mp->DataArray.Alloc(mh[i].m_Size);
        assert(!mp->m_bUploaded);
        gRenDev->m_TexMan->m_LoadBytes += mh[i].m_Size;
        SizeToLoad += mh[i].m_Size;
        if (fp)
        {
          if (m_ETF == eTF_0888 && (m_Flags2 & FT2_STREAMFROMDDS))
          {
            byte *pTemp = new byte[mp->USize*mp->VSize*3];
            iSystem->GetIPak()->FRead(pTemp, mp->USize*mp->VSize*3, 1, fp);
            for (int n=0; n<mp->USize*mp->VSize; n++)
            {
              mp->DataArray[n*4+0] = pTemp[n*3+0];
              mp->DataArray[n*4+1] = pTemp[n*3+1];
              mp->DataArray[n*4+2] = pTemp[n*3+2];
              mp->DataArray[n*4+3] = 255;
            }
            delete [] pTemp;
          }
          else
          if (fp)
            iSystem->GetIPak()->FRead(&mp->DataArray[0], mh[i].m_Size, 1, fp);
          else
          if (j)
          {
            if (m_Flags2 & FT2_REPLICATETOALLSIDES)
              memcpy(&mp->DataArray[0], &m_Mips[0][i]->DataArray[0], mp->DataArray.GetSize());
            else
            if (m_Flags2 & FT2_FORCECUBEMAP)
            {
              if (m_ETF == eTF_DXT1 || m_ETF == eTF_DXT3 || m_ETF == eTF_DXT5)
              {
                int nBlocks = ((mp->USize+3)/4)*((mp->VSize+3)/4);
                int blockSize = m_ETF == eTF_DXT1 ? 8 : 16;
                int nOffsData = m_ETF - eTF_DXT1;
                int nCur = 0;
                for (int n=0; n<nBlocks; n++)
                {
                  memcpy(&mp->DataArray[nCur], sDXTData[nOffsData], blockSize);
                  nCur += blockSize;
                }
              }
              else
                memset(&mp->DataArray[0], 0, mp->DataArray.GetSize());
            }
          }
        }
        else
        if (m_CacheID >= 0)
          rf->mfFileRead2(m_CacheID, mh[i].m_Size, &mp->DataArray[0]);
      }
      if (gRenDev->m_LogFileStr && SizeToLoad > 0)
        gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Sync Load '%s' (Side: %d, %d-%d[%d]), Size: %d, Time: %.3f\n", m_SourceName.c_str(), j, nSyncStartMip, i-1, nMips, SizeToLoad, iTimer->GetAsyncCurTime());
    }
    if (fp)
      iSystem->GetIPak()->FClose(fp);
    bool bRes;
    { 
      PROFILE_FRAME(Texture_LoadFromCache_UploadSync);
      bRes = UploadMips(nSyncStartMip, nMips-1);
    }
    if (!nSyncStartMip && bRes)
      RemoveMips(4);
    m_Flags2 &= ~FT2_WASUNLOADED;
    if (!m_Mips[0][0] || !m_Mips[0][0]->m_bUploaded)
      m_Flags2 |= FT2_PARTIALLYLOADED;
    else
      m_Flags2 &= ~FT2_PARTIALLYLOADED;
    //gRenDev->m_TexMan->CheckTexLimits(this);
  }
  // Asynchronous loading
  if (nASyncStartMip >= 0)
  {
    PROFILE_FRAME(Texture_LoadFromCacheASync);

    assert(nASyncStartMip <= nASyncEndMip);
    int nSeekFromStart = 0;
    for (i=0; i<nASyncStartMip; i++)
    {
      if (m_ETF == eTF_0888 && (m_Flags2 & FT2_STREAMFROMDDS))
        nSeekFromStart += mh[i].m_USize*mh[i].m_VSize*3;
      else
        nSeekFromStart += mh[i].m_Size;
    }
    if (nSides > 1)
      Size = m_Size/6;
    else
      Size = m_Size;
    for (j=0; j<nSides; j++)
    {
      int SizeToLoad = 0;
      for (i=nASyncStartMip; i<=nASyncEndMip; i++)
      {
        if (m_Mips[j][i] && (m_Mips[j][i]->m_bLoading || m_Mips[j][i]->m_bUploaded))
          break;
        if (!m_Mips[j][i])
          m_Mips[j][i] = new SMipmap(mh[i].m_USize, mh[i].m_VSize);
        m_Mips[j][i]->m_bLoading = true;
        if (m_ETF == eTF_0888 && (m_Flags2 & FT2_STREAMFROMDDS))
          SizeToLoad += mh[i].m_USize*mh[i].m_VSize*3;
        else
          SizeToLoad += mh[i].m_Size;
      }
      if (j && (m_Flags2 & (FT2_FORCECUBEMAP | FT2_REPLICATETOALLSIDES)))
        continue;

      if (SizeToLoad)
      {
        i--;
        if (gRenDev->m_LogFileStr)
          gRenDev->LogStrv(SRendItem::m_RecurseLevel, "Async Start Load '%s' (Side: %d, %d-%d[%d]), Size: %d, Time: %.3f\n", m_SourceName.c_str(), j, nASyncStartMip, i, nMips, SizeToLoad, iTimer->GetAsyncCurTime());
        gRenDev->m_TexMan->m_LoadBytes += SizeToLoad;
        STexCacheInfo *pTexCacheFileInfo = new STexCacheInfo;
        pTexCacheFileInfo->m_TexId = m_Id;
        pTexCacheFileInfo->m_pTempBufferToStream = new byte[SizeToLoad];
        byte *buf = (byte *)pTexCacheFileInfo->m_pTempBufferToStream;
        pTexCacheFileInfo->m_nStartLoadMip = nASyncStartMip;
        pTexCacheFileInfo->m_nEndLoadMip = i;
        pTexCacheFileInfo->m_nSizeToLoad = SizeToLoad;
        pTexCacheFileInfo->m_nCubeSide = nSides > 1 ? j : -1;
        pTexCacheFileInfo->m_fStartTime = iTimer->GetCurrTime();
        m_Flags2 |= FT2_STREAMINGINPROGRESS;
        StreamReadParams StrParams;
        if (m_CacheID >= 0)
        {
          SDirEntry *de = rf->mfGetEntry(m_CacheID);
          StrParams.nOffset = de->offset+sizeof(STexCacheFileHeader)+nMips*sizeof(STexCacheMipHeader)+Size*j+nSeekFromStart;
        }
        else
          StrParams.nOffset = sizeof(DWORD)+sizeof(DDS_HEADER)+nSeekFromStart;
        StrParams.dwUserData = (DWORD_PTR)pTexCacheFileInfo;
        StrParams.nLoadTime = 1;
        StrParams.nMaxLoadTime = 4;
        StrParams.nPriority = 0;
        StrParams.pBuffer = pTexCacheFileInfo->m_pTempBufferToStream;
        StrParams.nSize = SizeToLoad;
        if (m_CacheID >= 0)
        {
          StrParams.nFlags |= SRP_FLAGS_MAKE_PERMANENT;
          IReadStreamPtr pStream = pSE->StartRead(CName(m_CacheID).c_str(), rf->mfGetFileName(), &pTexCacheFileInfo->m_Callback, &StrParams);
        }
        else
        {
          if (!j)
            IReadStreamPtr pStream = pSE->StartRead(m_SourceName.c_str(), m_SourceName.c_str(), &pTexCacheFileInfo->m_Callback, &StrParams);
          else
          {
            char name[512];
            StripExtension(m_SourceName.c_str(), name);
            size_t len = strlen(name);
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
            char cube[512];
            sprintf(cube, "%s_%s.dds", name, cubefaces[j]);
            IReadStreamPtr pStream = pSE->StartRead(cube, cube, &pTexCacheFileInfo->m_Callback, &StrParams);
          }
        }
      }
    }
  }
}

static ETEX_Format sStrToETF(const char *sETF)
{
  if (!stricmp(sETF, "Index"))
    return eTF_Index;
  if (!stricmp(sETF, "0888"))
    return eTF_0888;
  if (!stricmp(sETF, "RGB8"))
    return eTF_RGB8;
  if (!stricmp(sETF, "8888"))
    return eTF_8888;
  if (!stricmp(sETF, "8000"))
    return eTF_8000;
  if (!stricmp(sETF, "0088"))
    return eTF_0088;
  if (!stricmp(sETF, "DXT1"))
    return eTF_DXT1;
  if (!stricmp(sETF, "DXT3"))
    return eTF_DXT3;
  if (!stricmp(sETF, "DXT5"))
    return eTF_DXT5;
  if (!stricmp(sETF, "SIGNED_HILO16"))
    return eTF_SIGNED_HILO16;
  if (!stricmp(sETF, "DSDT_MAG"))
    return eTF_DSDT_MAG;
  if (!stricmp(sETF, "DSDT"))
    return eTF_DSDT;
  assert (0);
  return eTF_8888;
}

static ETexType sStrToETT(const char *sETT)
{
  if (!stricmp(sETT, "Base"))
    return eTT_Base;
  if (!stricmp(sETT, "Rectangle"))
    return eTT_Rectangle;
  if (!stricmp(sETT, "Cubemap"))
    return eTT_Cubemap;
  assert(0);
  return eTT_Base;
}

#ifdef WIN64
#pragma warning( push )							//AMD Port
#pragma warning( disable : 4267 )				// conversion from 'size_t' to 'int', possible loss of data
#endif

STexPic *CTexMan::LoadFromCache(STexPic *ti, int flags, int flags2, char *name, const char *szModelName, ETexType eTT)
{
  if (!(m_Streamed & 1))
    return NULL;

  if (flags & FT_NOSTREAM)
    return NULL;

  int i, n;
  FILE *fp = NULL;
  CResFile *rf = gRenDev->m_TexMan->m_TexCache;
  char szName[512];
  if (ti && !(flags2 & FT2_DISCARDINCACHE))
  {
    strcpy(szName, ti->m_SourceName.c_str());
    StripExtension(szName, szName);
    AddExtension(szName, ".dds");
    fp = iSystem->GetIPak()->FOpen(szName, "rb");
    if (ti->m_eTT == eTT_Bumpmap && !strstr(ti->m_SourceName.c_str(), "_ddn"))
    {
      if (fp)
      {
        iSystem->GetIPak()->FClose(fp);
        fp = NULL;
      }
      iLog->Log("Warning: streaming of heightmap texture %s.dds are not supported (texture name should contain _ddn substring)", ti->m_SourceName.c_str());
    }
    if (ti->m_eTT == eTT_DSDTBump && !strstr(ti->m_SourceName.c_str(), "_ddt"))
    {
      if (fp)
      {
        iSystem->GetIPak()->FClose(fp);
        fp = NULL;
      }
      iLog->Log("Warning: streaming of heightmap texture %s.dds are not supported (texture name should contain _ddt substring)", ti->m_SourceName.c_str());
    }
  }

  if (!fp && !rf)
  {
    if (!CreateCacheFile())
      return NULL;
    rf = gRenDev->m_TexMan->m_TexCache;
  }
  bool bSprite = false;
  if (ti && !(flags2 & FT2_DISCARDINCACHE))
    ti->GetCacheName(szName);
  else
  if (!ti)
    sprintf(szName, "%s[Base]", name);
  else
  {
    strcpy(szName, ti->m_Name.c_str());
    szModelName = &ti->m_SourceName.c_str()[0];
  }
  ConvertDOSToUnixName(szName, szName);

  name = szName;
  if (eTT == eTT_Cubemap && m_LastCMStreamed)
  {
    char SourceName[512];
    n = 0;
    while (name[n] != '[')
    {
      SourceName[n] = name[n];
      n++;
      SourceName[n] = 0;
    }
    strcat(SourceName, m_LastCMStreamed->m_CacheFileHeader.m_sExt);

    ti->m_Width = m_LastCMStreamed->m_Width;
    ti->m_Height = m_LastCMStreamed->m_Height;
    ti->m_SourceName = SourceName;
    ti->m_nMips = m_LastCMStreamed->m_nMips;
    ti->m_Size = 0;
    if (ti->m_nMips <= 1)
      ti->m_Flags |= FT_NOMIPS;
    ti->m_eTT = eTT;
    int CubeSide;
    int tgt = TEXTGT_CUBEMAP;
    n = strlen(ti->m_SearchName.c_str()) - 4;
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
    if (m_LastCMStreamed->m_CubeSide >= CubeSide)
    {
      m_LastCMStreamed = NULL;
      return NULL;
    }
    ti->m_DstFormat = m_LastCMStreamed->m_DstFormat;
    if (CubeSide == 5)
      m_LastCMStreamed = NULL;
    ti->m_CubeSide = CubeSide;

    ti->m_Flags2 |= FT2_WASUNLOADED;

    ti->m_Bind = TX_FIRSTBIND + ti->m_Id;

    return ti;
  }

  if (!fp && rf->mfFileExist(name)<0)
    return NULL;
  if (!ti)
  {
    bSprite = true;
    ti = TextureInfoForName(name, -1, eTT_Base, flags, flags2, -1);
    if (ti->m_bBusy)
      return ti;
    ti->m_bBusy = true;
    ti->m_Flags = flags;
    ti->m_Flags2 = flags2 & ~FT2_RELOAD;
    ti->m_Bind = TX_FIRSTBIND + ti->m_Id;
  }
  else
  if (flags2 & FT2_DISCARDINCACHE)
    bSprite = true;

  STexCacheFileHeader fh;
  int nSides, nMips;
  if (fp)
  {
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
    int size = iSystem->GetIPak()->FTell(fp);
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
    byte *buf = new byte [size+1];
    size = iSystem->GetIPak()->FRead(buf, 1, size, fp);
    iSystem->GetIPak()->FClose(fp);
    CImageFile::m_CurFileName[0] = 0;
    CImageDDSFile im(buf, size);
    delete [] buf;
    if (im.mfGet_error() == eIFE_OK && im.mfGetFormat() != eIF_Unknown)
    {
      ti->m_Flags2 |= FT2_STREAMFROMDDS;
      int i, j;
      fh.m_SizeOf = sizeof(STexCacheFileHeader);
      fh.m_bPolyBump = false;
      fh.m_bCloneSpace = false;
      fh.m_nMips = im.mfGet_numMips();
      ETEX_Format eTF;
      if (ti->m_eTT == eTT_DSDTBump)
        eTF = eTF_DSDT_MAG;
      else
        eTF = sImageFormat2TexFormat(im.mfGetFormat());
      strcpy(fh.m_sETF, sETFToStr(eTF));
      strcpy(fh.m_sExt, ".dds");
      fh.m_nSides = ti->m_eTT == eTT_Cubemap ? 6 : 1;
      fh.m_DstFormat = ti->DstFormatFromTexFormat(eTF);
      memcpy(&ti->m_CacheFileHeader, &fh, sizeof(fh));
      nSides = ti->m_CacheFileHeader.m_nSides;
      nMips = ti->m_CacheFileHeader.m_nMips;
      ti->m_pFileTexMips = new STexCacheMipHeader[nMips];
      int wdt = im.mfGet_width();
      int hgt = im.mfGet_height();
      for (i=0; i<nMips; i++)
      {
        assert(wdt || hgt);
        if (!wdt)
          wdt = 1;
        if (!hgt)
          hgt = 1;
        ti->m_pFileTexMips[i].m_USize = wdt;
        ti->m_pFileTexMips[i].m_VSize = hgt;
        if (eTF == eTF_DXT1 || eTF == eTF_DXT3 || eTF == eTF_DXT5)
        {
          ti->m_pFileTexMips[i].m_USize = (ti->m_pFileTexMips[i].m_USize + 3) & ~3;
          ti->m_pFileTexMips[i].m_VSize = (ti->m_pFileTexMips[i].m_VSize + 3) & ~3;
        }
        ti->m_pFileTexMips[i].m_Size = ti->TexSize(wdt, hgt, fh.m_DstFormat);
        ti->m_pFileTexMips[i].m_SizeOf = sizeof(STexCacheMipHeader);
        wdt >>= 1;
        hgt >>= 1;
      }
      for (i=0; i<nMips; i++)
      {
        ti->m_pFileTexMips[i].m_SizeWithMips = 0;
        for (j=i; j<nMips; j++)
        {
          ti->m_pFileTexMips[i].m_SizeWithMips += ti->m_pFileTexMips[j].m_Size;
        }
      }
    }
    else
      fp = NULL;
  }
  if (!fp)
  {
    if (ti->m_CacheID < 0)
      ti->m_CacheID = rf->mfFileGetNum(name);
    rf->mfFileSeek(ti->m_CacheID, 0, SEEK_SET);
    rf->mfFileRead2(ti->m_CacheID, sizeof(STexCacheFileHeader), &fh);
    if (fh.m_sExt[0] == 0 && !bSprite)
      return NULL;
    memcpy(&ti->m_CacheFileHeader, &fh, sizeof(fh));
    nSides = ti->m_CacheFileHeader.m_nSides;
    nMips = ti->m_CacheFileHeader.m_nMips;
    ti->m_pFileTexMips = new STexCacheMipHeader[nMips];
    rf->mfFileRead2(ti->m_CacheID, sizeof(STexCacheMipHeader)*nMips, ti->m_pFileTexMips);
  }
  char SourceName[512];
  n = 0;
  char sETT[32];
  if (bSprite)
  {
    strcpy(SourceName, szModelName);
    ti->m_ETF = eTF_8888;
  }
  else
  {
    while (name[n] != '[')
    {
      SourceName[n] = name[n];
      n++;
      SourceName[n] = 0;
    }
    n++;
    int m = 0;
    while (name[n] != ']')
    {
      sETT[m] = name[n];
      n++;
      m++;
    }
    sETT[m] = 0;
    strcat(SourceName, fh.m_sExt);
    ti->m_ETF = sStrToETF(fh.m_sETF);
    if (ti->m_eTT == eTT_DSDTBump)
      ti->m_ETF = eTF_0888;
  }

  char nameFile[2][128];
  int nFiles = ti->GetFileNames(nameFile[0], nameFile[1], 128);
  if (nFiles > 1)
    strcat(nameFile[1], fh.m_sExt);
  else
  {
    StripExtension(nameFile[0], nameFile[0]);
    strcat(nameFile[0], fh.m_sExt);
  }
  if (!bSprite && !fp)
  {
    int nComp = 0;
    for (i=0; i<nFiles; i++)
    {
      HANDLE status2 = CreateFile(nameFile[i],GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
      if (status2 != INVALID_HANDLE_VALUE)
      {
        FILETIME writetime1,writetime2;
        writetime1 = fh.m_WriteTime[i];

        GetFileTime(status2,NULL,NULL,&writetime2);
        if (CompareFileTime(&writetime1,&writetime2)==0)
          nComp++;

        CloseHandle(status2);
      }
    }
    if (nComp != nFiles)
    {
      ti->m_CacheID = -1;
      return NULL;
    }
  }
  else
  if (bSprite && !(flags2 & FT2_RELOAD))
  {
    HANDLE status2 = CreateFile(szModelName,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (status2 != INVALID_HANDLE_VALUE)
    {
      FILETIME writetime1,writetime2;
      writetime1 = fh.m_WriteTime[0];

      GetFileTime(status2,NULL,NULL,&writetime2);
      CloseHandle(status2);
      if (CompareFileTime(&writetime1,&writetime2)!=0)
      {
        ti->m_CacheID = -1;
        return NULL;
      }
    }
  }

  STexCacheMipHeader *mh = ti->m_pFileTexMips;

  ti->m_eTT = eTT;
  ti->m_Width = ti->m_pFileTexMips[0].m_USize;
  ti->m_Height = ti->m_pFileTexMips[0].m_VSize;
  ti->m_SourceName = SourceName;
  ti->m_nMips = nMips;
  ti->m_Size = ti->m_pFileTexMips[0].m_SizeWithMips * nSides;
  if (ti->m_nMips <= 1)
    ti->m_Flags |= FT_NOMIPS;
  int CubeSide = -1;
  int tgt = TEXTGT_2D;
  if (ti->m_eTT == eTT_Cubemap)
  {
    tgt = TEXTGT_CUBEMAP;
    int n = strlen(ti->m_SearchName.c_str()) - 4;
		assert (n > 0);
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
  }
  if (CubeSide < 0 || CubeSide == 5)
    m_LastCMStreamed = NULL;
  else
    m_LastCMStreamed = ti;
  ti->m_CubeSide = CubeSide;

  ti->Unlink();
  if (ti->m_eTT != eTT_Cubemap || !ti->m_CubeSide)
    ti->m_Flags2 |= FT2_WASUNLOADED;

  ti->m_Flags2 |= FT2_WASLOADED;
  ti->m_Bind = TX_FIRSTBIND + ti->m_Id;
  AddToHash(ti->m_Bind, ti);
  ti->m_RefTex.m_MipFilter = nMips > 1 ? m_MipFilter : 0;
  ti->m_DstFormat = fh.m_DstFormat;
  if (ti->m_eTT == eTT_Cubemap)
  {
    ti->m_RefTex.bRepeats = false;
    ti->m_RefTex.m_MinFilter = GetMinFilter();
    ti->m_RefTex.m_MagFilter = GetMagFilter();
    ti->m_RefTex.m_AnisLevel = 1;
  }
  else
  {
    if (ti->m_Flags & FT_CLAMP)
      ti->m_RefTex.bRepeats = false;
    else
      ti->m_RefTex.bRepeats = true;
    ti->m_RefTex.m_MinFilter = GetMinFilter();
    ti->m_RefTex.m_MagFilter = GetMagFilter();
    ti->m_RefTex.m_AnisLevel = 1;
  }
  ti->m_RefTex.m_Type = tgt;
  ti->m_RefTex.bProjected = (ti->m_Flags & FT_PROJECTED) ? true : false;

  // Always load lowest 4 mips synchronously
  fp = NULL;
  if (ti->m_nMips > 1)
  {
    int i, j;
    int Size;

    if (!ti->m_Mips[0])
      ti->CreateMips();
    STexCacheFileHeader *fh = &ti->m_CacheFileHeader;
    STexCacheMipHeader *mh = ti->m_pFileTexMips;
    nSides = fh->m_nSides;
    nMips = fh->m_nMips;
    int nSyncStartMip = -1;
    int nSyncEndMip = -1;
    int nEndMip = ti->m_nMips-1;
    int nStartLowestM = max(0, nEndMip-3);
    for (i=nStartLowestM; i<=nEndMip; i++)
    {
      if (!ti->m_Mips[0][i] || !ti->m_Mips[0][i]->m_bUploaded)
      {
        nSyncStartMip = i;
        for (j=i+1; j<=nEndMip; j++)
        {
          if (ti->m_Mips[0][j] && ti->m_Mips[0][j]->m_bUploaded)
            break;
        }
        nSyncEndMip = j-1;
        break;
      }
    }

    if (nSyncStartMip >= 0)
    {
      assert(nSyncStartMip <= nSyncEndMip);
      int nSeekFromStart = 0;
      for (i=0; i<nSyncStartMip; i++)
      {
        if (ti->m_ETF == eTF_0888 && (ti->m_Flags2 & FT2_STREAMFROMDDS))
          nSeekFromStart += mh[i].m_USize*mh[i].m_VSize*3;
        else
          nSeekFromStart += mh[i].m_Size;
      }
      if (nSides > 1)
        Size = ti->m_Size/6;
      else
        Size = ti->m_Size;
      static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
      for (j=0; j<nSides; j++)
      {
        if (ti->m_CacheID < 0)
        {
          if (j)
          {
            if (fp)
              iSystem->GetIPak()->FClose(fp);
            char name[512];
            StripExtension(ti->m_SourceName.c_str(), name);
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
            char cube[512];
            sprintf(cube, "%s_%s.dds", name, cubefaces[j]);
            fp = iSystem->GetIPak()->FOpen(cube, "rb");
          }
          if (!fp)
          {
            if (!j)
              fp = iSystem->GetIPak()->FOpen(ti->m_SourceName.c_str(), "rb");
          }
          if (fp)
            iSystem->GetIPak()->FSeek(fp, sizeof(DWORD)+sizeof(DDS_HEADER)+nSeekFromStart, SEEK_SET);
        }
        else
          rf->mfFileSeek(ti->m_CacheID, sizeof(STexCacheFileHeader)+nMips*sizeof(STexCacheMipHeader)+Size*j+nSeekFromStart, SEEK_SET);
        int SizeToLoad = 0;
        for (i=nSyncStartMip; i<=nSyncEndMip; i++)
        {
          if (ti->m_Mips[j][i] && (ti->m_Mips[j][i]->DataArray.GetSize() || ti->m_Mips[j][i]->m_bUploaded))
          {
            i++;
            break;
          }
          SMipmap *mp;
          if (!ti->m_Mips[j][i])
            ti->m_Mips[j][i] = new SMipmap(mh[i].m_USize, mh[i].m_VSize);
          mp = ti->m_Mips[j][i];
          if (!mp->DataArray.GetSize())
            mp->DataArray.Alloc(mh[i].m_Size);
          assert(!mp->m_bUploaded);
          gRenDev->m_TexMan->m_LoadBytes += mh[i].m_Size;
          SizeToLoad += mh[i].m_Size;
          if (fp)
          {
            if (ti->m_ETF == eTF_0888)
            {
              byte *pTemp = new byte[mh[i].m_USize*mh[i].m_VSize*3];
              iSystem->GetIPak()->FRead(pTemp, mh[i].m_USize*mh[i].m_VSize*3, 1, fp);
              for (int n=0; n<mh[i].m_USize*mh[i].m_VSize; n++)
              {
                mp->DataArray[n*4+0] = pTemp[n*3+0];
                mp->DataArray[n*4+1] = pTemp[n*3+1];
                mp->DataArray[n*4+2] = pTemp[n*3+2];
                mp->DataArray[n*4+3] = 255;
              }
              delete [] pTemp;
            }
            else
              iSystem->GetIPak()->FRead(&mp->DataArray[0], mh[i].m_Size, 1, fp);
          }
          else
          if (ti->m_CacheID >= 0)
            rf->mfFileRead2(ti->m_CacheID, mh[i].m_Size, &mp->DataArray[0]);
          else
          if (j)
          {
            if (ti->m_Flags2 & FT2_REPLICATETOALLSIDES)
              memcpy(&mp->DataArray[0], &ti->m_Mips[0][i]->DataArray[0], mp->DataArray.GetSize());
            else
            if (ti->m_Flags2 & FT2_FORCECUBEMAP)
            {
              if (ti->m_ETF == eTF_DXT1 || ti->m_ETF == eTF_DXT3 || ti->m_ETF == eTF_DXT5)
              {
                int nBlocks = ((mp->USize+3)/4)*((mp->VSize+3)/4);
                int blockSize = ti->m_ETF == eTF_DXT1 ? 8 : 16;
                int nOffsData = ti->m_ETF - eTF_DXT1;
                int nCur = 0;
                for (int n=0; n<nBlocks; n++)
                {
                  memcpy(&mp->DataArray[nCur], sDXTData[nOffsData], blockSize);
                  nCur += blockSize;
                }
              }
              else
                memset(&mp->DataArray[0], 0, mp->DataArray.GetSize());
            }
          }
        }
      }
      if (fp)
      {
        iSystem->GetIPak()->FClose(fp);
        fp = NULL;
      }
    }
  }

  return ti;
}

#ifdef WIN64
#pragma warning( pop )							//AMD Port
#endif

//=========================================================================

bool CTexMan::CreateCacheFile()
{
  STexCacheFileHeader fh;
  STexCacheMipHeader mh;
  bool bValid = true;

#ifdef DIRECT3D8
  CResFile *rf = new CResFile("Textures\\TexturesD3D8.cache", eFSD_name);
#elif DIRECT3D9
  CResFile *rf = new CResFile("Textures\\TexturesD3D9.cache", eFSD_name);
#elif OPENGL
  CResFile *rf = new CResFile("Textures\\TexturesOGL.cache", eFSD_name);
#else
  CResFile *rf = new CResFile("Textures\\Textures.cache", eFSD_name);
#endif
  rf->mfOpen(RA_READ);
  if (rf->mfGetError())
  {
    rf->mfClose();
    rf->mfOpen(RA_CREATE);
    bValid = false;
  }
  else
  {
    rf->mfFileSeek(0, 0, SEEK_SET);
    rf->mfFileRead2(0, sizeof(STexCacheFileHeader), &fh);
    if (fh.m_SizeOf != sizeof(STexCacheFileHeader) || fh.m_Version != TEXCACHE_VERSION)
      bValid = false;
    else
    {
      rf->mfFileRead2(0, sizeof(STexCacheMipHeader), &mh);
      if (mh.m_SizeOf != sizeof(STexCacheMipHeader))
        bValid = false;
      else
      if (rf->mfGetHolesSize()/rf->mfGetResourceSize()*100 > 20)
        bValid = false;
    }
    if (bValid)
    {
      rf->mfClose();
      rf->mfOpen(RA_READ|RA_WRITE);
    }
    else
    {
      iLog->LogToConsole("Texture cache file '%s' isn't valid (creating new one...)\n", "Textures\\TexturesD3D8.cache");
      rf->mfClose();
      rf->mfOpen(RA_CREATE);
    }
  }
  gRenDev->m_TexMan->m_TexCache = rf;

  return bValid;
}

