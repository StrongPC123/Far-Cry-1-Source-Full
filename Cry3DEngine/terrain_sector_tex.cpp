////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_sector_tex.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: terrain texture management
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "terrain_sector.h"
#include "terrain.h"
#include "objman.h"

// set texture for sector
void CSectorInfo::SetTextures(bool bMakeUncompressedForEditing)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	if(m_bLockTexture) // always keep full texture detail during editing
		m_cNewTextMML = 0;

  { // required actions
    if(!m_nLowLodTextureID)
      m_nLowLodTextureID = MakeSectorTextureDDS( GetSecIndex(), MAX_TEX_MML_LEVEL, bMakeUncompressedForEditing );

    if(!m_nTextureID)
    {
      m_nTextureID = m_nLowLodTextureID;
      m_cTextureMML = MAX_TEX_MML_LEVEL; 
    }
  }

  // desired actions
  if(m_cTextureMML > m_cNewTextMML)
  { // increase tex resolution
		if(!m_bLockTexture)
		{
			if(m_pTerrain->m_nUploadsInFrame>1)
				return; // no more than 1 upload per frame

			// remove hi res texture if it's not needed
			if(m_cGeometryMML>=MAX_MML_LEVEL) // NOTE: in testing
			{
				if(m_nTextureID != m_nLowLodTextureID)
				{
					m_pTerrain->m_pTexturePool->RemoveTexture(m_nTextureID);
					assert(m_nTextureID);
					assert(m_nTextureID != m_nLowLodTextureID);
					m_nTextureID = m_nLowLodTextureID;
					m_cTextureMML = MAX_TEX_MML_LEVEL; 
				}
				return;
			}

			m_pTerrain->m_nUploadsInFrame++;

			if(m_nTextureID != m_nLowLodTextureID)
				GetLog()->Log("CSectorInfo::SetTextureAndLOD: m_nTextureID != m_nLowLodTextureID");
		}

    m_cTextureMML = m_cNewTextMML; 
    m_nTextureID = MakeSectorTextureDDS( GetSecIndex(), m_cTextureMML, bMakeUncompressedForEditing );
  
    if(m_pTerrain->GetCVars()->e_terrain_log) 
      GetLog()->Log("tex loaded %d(%d)", GetSecIndex(), m_cTextureMML);
  }
  else if(m_cTextureMML < m_cNewTextMML)
  { // reduce tex resolution
    if(m_nTextureID == m_nLowLodTextureID)
      GetLog()->Log("CSectorInfo::SetTextureAndLOD: m_nTextureID == m_nLowLodTextureID");

    m_pTerrain->m_pTexturePool->RemoveTexture(m_nTextureID);
    m_nTextureID = m_nLowLodTextureID;
    m_cTextureMML = MAX_TEX_MML_LEVEL; 
  }
}

// load compresssed texture from disk
int CSectorInfo::MakeSectorTextureDDS( int sec_id, int nMipMapLevelToLoad, bool bMakeUncompressedForEditing )
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	nMipMapLevelToLoad+=GetCVars()->e_terrain_texture_mip_offset;

  // open file once
  if(!m_pTerrain->m_fpTerrainTextureFile)
  { 
    m_pTerrain->m_fpTerrainTextureFile = GetSystem()->GetIPak()->FOpen(Get3DEngine()->GetLevelFilePath("terrain\\cover.ctc"), "rb");

    if(!m_pTerrain->m_fpTerrainTextureFile) 
      return 0;

    GetSystem()->GetIPak()->FRead(&m_pTerrain->m_nSectorTextureReadedSize, 1, 4, m_pTerrain->m_fpTerrainTextureFile);
    GetLog()->Log("  TerrainSectorTextureSize %dx%d", m_pTerrain->m_nSectorTextureReadedSize, m_pTerrain->m_nSectorTextureReadedSize);

    GetSystem()->GetIPak()->FSeek( m_pTerrain->m_fpTerrainTextureFile, 0, SEEK_END);
    int nFileSize = GetSystem()->GetIPak()->FTell(m_pTerrain->m_fpTerrainTextureFile);

    m_pTerrain->m_nSectorTextureDataSizeBytes = (nFileSize-4)/(m_pTerrain->GetSectorsTableSize()*m_pTerrain->GetSectorsTableSize());
    GetLog()->Log("  SectorTextureDataSizeBytes = %d", m_pTerrain->m_nSectorTextureDataSizeBytes);

    m_pTerrain->m_ucpTmpTexBuffer = new uchar [m_pTerrain->m_nSectorTextureDataSizeBytes];
  }

  if(!m_pTerrain->m_fpTerrainTextureFile)
  { Warning(0,0,"MakeSectorTextureDDS: !m_pTerrain->m_fpTerrainTextureFile"); return 0; }
  
  // count mm levels
  int nMipLevels=0;
  int w = m_pTerrain->m_nSectorTextureReadedSize;
  while(w>0)
  { w/=2; nMipLevels++; }

  int nDataSize = m_pTerrain->m_nSectorTextureDataSizeBytes;
  int nTexSize  = m_pTerrain->m_nSectorTextureReadedSize;

  // calculate texture offset in file
  int file_offset = 4+sec_id*m_pTerrain->m_nSectorTextureDataSizeBytes;

  // if not zero mml specified
  for(int m=0; m<nMipMapLevelToLoad; m++)
  {
    file_offset = file_offset + nTexSize*nTexSize/2;
    nDataSize -= nTexSize*nTexSize/2;
    nMipLevels--;
    nTexSize = nTexSize/2;
  }

	assert(m_pTerrain->m_nSectorTextureDataSizeBytes >= (GetCVars()->e_terrain_texture_mipmaps ? nDataSize : nTexSize*nTexSize/2));

  // read texture
  GetSystem()->GetIPak()->FSeek( m_pTerrain->m_fpTerrainTextureFile, file_offset, SEEK_SET );
  INT_PTR readed = GetSystem()->GetIPak()->FRead(m_pTerrain->m_ucpTmpTexBuffer, 1,		//AMD Port
    GetCVars()->e_terrain_texture_mipmaps ? nDataSize : nTexSize*nTexSize/2, 
    m_pTerrain->m_fpTerrainTextureFile);

  // no reason to use update texture instead create since size is always diferent
/*  int nTexID = GetRenderer()->DownLoadToVideo Memory(m_pTerrain->m_ucpTmpTexBuffer,
    nTexSize, nTexSize, eTF_DXT1, eTF_DXT1,
    GetCVars()->e_terrain_texture_mipmaps ? nMipLevels : 0, false,
    GetCVars()->e_terrain_texture_mipmaps ? FILTER_BILINEAR : FILTER_LINEAR);*/

  int nTexID = m_pTerrain->m_pTexturePool->MakeTexture(m_pTerrain->m_ucpTmpTexBuffer, nTexSize, this, bMakeUncompressedForEditing);

  return (nTexID);
}
/*
void CSectorInfo::UpdateSectorTexture(unsigned char * pTexData, int nSizeOffTexData)
{
  int nTexSize = m_pTerrain->m_nSectorTextureReadedSize;

  if(nSizeOffTexData != nTexSize*nTexSize*3)
  { 
    GetLog()->Log("Error: CSectorInfo::UpdateSectorTexture: nSizeOffTexData error"); 
    return; 
  }

  if(m_nTextureID)
  {
    GetRenderer()->UpdateTextureInVideoMemory(m_nTextureID,pTexData,0,0,nTexSize,nTexSize,eTF_0888);
  }
  else
  {
    assert(0);
//    m_nTextureID = GetRenderer()->DownLoadToVideo Memory(pTexData,
  //    nTexSize,nTexSize,eTF_0888,eTF_DXT1,0,false);
  }
}	*/

int CSectorInfo::LockSectorTexture(int & nTexDim)
{
	m_bLockTexture = true;
	nTexDim = m_pTerrain->m_nSectorTextureReadedSize;
	// force texture reloading as uncompressed
	m_cNewTextMML = 0;
	m_cTextureMML = 1;
	SetTextures(true);
	return m_nTextureID;
}

void CSectorInfo::RemoveSectorTextures(bool bRemoveLowLod)
{
  // remove high
  if(m_nTextureID)
  {
    m_pTerrain->m_pTexturePool->RemoveTexture(m_nTextureID);
    assert(m_nLowLodTextureID);
    m_nTextureID = m_nLowLodTextureID;
    m_cTextureMML = MAX_TEX_MML_LEVEL; 
  }

  // remove low
  if(bRemoveLowLod)
  {
    m_pTerrain->m_pTexturePool->RemoveTexture(m_nLowLodTextureID);
    m_nTextureID = m_nLowLodTextureID = 0;
  }
}

void CSectorInfo::UnloadHeighFieldTexture(float fDistanse, float fMaxViewDist)
{
	if(m_nTextureID && m_cTextureMML == 0 && m_nTextureID!=m_nLowLodTextureID)
	{ // unload if to far or not in use int time
		if(m_nTextureID == m_nLowLodTextureID)
			GetLog()->Log("unload old secs error");

		// set low lod tex
		//glDeleteTextures(1, &(m_nTextureID) );
		m_pTerrain->m_pTexturePool->RemoveTexture(m_nTextureID);
		m_nTextureID = m_nLowLodTextureID;
		m_cTextureMML = MAX_TEX_MML_LEVEL; 

		if(GetCVars()->e_terrain_log)
			GetLog()->Log("lod0 tex unloaded");
	}
	else if(m_nTextureID && m_nTextureID == m_nLowLodTextureID)
	{ // only low lod       
		if(fDistanse > (1.5f*fMaxViewDist))
		{
			m_pTerrain->m_pTexturePool->RemoveTexture(m_nTextureID);
			m_nTextureID = m_nLowLodTextureID = 0;
			m_cTextureMML = 0;

			if(GetCVars()->e_terrain_log)
				GetLog()->Log("lod1 tex unloaded");
		}
	}
	else if(!m_nTextureID && !m_nLowLodTextureID)
	{ // no textures
		m_nTextureID = m_nLowLodTextureID;
	}
	else
	{
		Warning(0,0,"CTerrain::UnloadOldSectors: tex management error");
	}
}