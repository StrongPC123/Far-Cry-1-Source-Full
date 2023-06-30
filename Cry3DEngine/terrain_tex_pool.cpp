////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_tex_pool.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: terrain texture cahce
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_sector.h"

CTexturePool::CTexturePool()
{
  /*
  int nMaxTexSize = 128;
  int nMaxTexNum  = 64; // max hires textures num

  // alloc tmp buff
  uchar * pTmpBuff = new uchar[nMaxTexSize*nMaxTexSize*4];
  memset(pTmpBuff,0,nMaxTexSize*nMaxTexSize*4);

  // alloc texture pool for lods
  for(int nLod=0; nLod<2; nLod++)
  {
    for(int i=0; i<nMaxTexNum; i++)
    {
      int nTexID = GetRenderer()->DownLoadToVideoMemory(pTmpBuff,
        nMaxTexSize, nMaxTexSize, eTF_DXT1, eTF_DXT1, 0, false, FILTER_LINEAR);

      assert(nTexID);

      TexInfo info;
      info.nTexId = nTexID;
      info.pSectorInfo = 0;
      m_TexturePool[nLod].Add(info);
    }
    nMaxTexSize/=2;
    nMaxTexNum=1024; // max lod1 textures num
  }

  delete [] pTmpBuff;*/
}

int CTexturePool::MakeTexture(uchar * pData, int nTexSize, CSectorInfo * pNewSectorInfo, bool bMakeUncompressedForEditing)
{
  if(!GetCVars()->e_terrain_texture_pool)
  { // if pool not used
    return GetRenderer()->DownLoadToVideoMemory(pData, nTexSize, nTexSize, 
			eTF_DXT1, bMakeUncompressedForEditing ? eTF_8888 : eTF_DXT1, 0, false, FILTER_LINEAR);
  }

  int nLod = -1;
  if(nTexSize == 128)
    nLod = 0;
  else if(nTexSize == 64)
    nLod = 1;
  else
    GetConsole()->Exit("CTexturePool::MakeTexture: invalid nSize");

  list2<TexInfo> & TexInfos = m_TexturePool[nLod];

  // find first free slot
  for(int i=0; i<TexInfos.Count(); i++)
  if(!TexInfos[i].pSectorInfo)
  { 
    // update slot texture
    GetRenderer()->UpdateTextureInVideoMemory(TexInfos[i].nTexId,pData,0,0,nTexSize,nTexSize,eTF_DXT1);
    TexInfos[i].pSectorInfo = pNewSectorInfo;
    return TexInfos[i].nTexId;
  }

  GetLog()->Log("CTexturePool::MakeTexture: Terrain texture pool overflow");

  // find oldest sector
  uint nMinLastUsageTime = (uint)-1;
  int nTexInfoId = -1;
  for(int i=0; i<TexInfos.Count(); i++)
  if(TexInfos[i].pSectorInfo && nMinLastUsageTime > (uint)TexInfos[i].pSectorInfo->m_cLastTimeUsed)
  {
    nMinLastUsageTime = TexInfos[i].pSectorInfo->m_cLastTimeUsed;
    nTexInfoId = i;
  }

  // free owning sector textures
  assert(TexInfos[nTexInfoId].pSectorInfo);
  TexInfos[nTexInfoId].pSectorInfo->RemoveSectorTextures(nLod>0);
  assert(!TexInfos[nTexInfoId].pSectorInfo);

  // update slot texture with new data
  TexInfos[nTexInfoId].pSectorInfo = pNewSectorInfo;
  GetRenderer()->UpdateTextureInVideoMemory(TexInfos[nTexInfoId].nTexId,pData,0,0,nTexSize,nTexSize,eTF_DXT1);
  return TexInfos[nTexInfoId].nTexId;
}

void CTexturePool::RemoveTexture(int nId)
{
  if(!GetCVars()->e_terrain_texture_pool)
  { // if pool not used
    GetRenderer()->RemoveTexture(nId);
    return;
  }

  // find texture id and mark it as free
  int bFound=0;
  for(int nLod=0; nLod<2; nLod++)
  {
    for(int i=0; i<m_TexturePool[nLod].Count(); i++)
    {
      if(m_TexturePool[nLod][i].pSectorInfo && m_TexturePool[nLod][i].nTexId == nId)
      {
        assert(!bFound);
        if(bFound)
          GetLog()->Log("CTexturePool::RemoveTexture: texture id found twice");

        m_TexturePool[nLod][i].pSectorInfo = 0;
        bFound = true;
      }
    }
  }
}

const char * CTexturePool::GetStatusText(CTerrain * pTerrain)
{
  int nInUseTexNum[2] = {0,0};
  for(int nLod=0; nLod<2; nLod++)
  for(int i=0; i<m_TexturePool[nLod].Count(); i++)
  if(m_TexturePool[nLod][i].pSectorInfo)
    nInUseTexNum[nLod]++;

  int arrTexMem[2] = 
  { 
    nInUseTexNum[0]*128*128*3/6,
    nInUseTexNum[1]*64*64*3/6
  };

  static char szText[256]="";
  if(!GetCVars()->e_terrain_texture_pool)
    snprintf(szText, sizeof(szText), "TerrTexStats %d/%d", pTerrain->m_arrLoadedTexturesNum[0], pTerrain->m_arrLoadedTexturesNum[1]);
  else
    snprintf(szText, sizeof(szText), "TerrTexPoolStats %d/%d (%d/%d) / %d/%d (%d/%d)", 
      nInUseTexNum[0], m_TexturePool[0].Count(), arrTexMem[0]/1024, m_TexturePool[0].Count()*128*128*3/6/1024,
      nInUseTexNum[1], m_TexturePool[1].Count(), arrTexMem[1]/1024, m_TexturePool[1].Count()* 64* 64*3/6/1024);

  return szText;
}
