/*=============================================================================
  D3DTexturesStreaming.cpp : Direct3D8 specific texture streaming technology.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"

//===============================================================================

void STexPic::BuildMips(TArray<SMipmap *>* Mips)
{
}

void STexPicD3D::BuildMips(TArray<SMipmap *>* Mips)
{
  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
  IDirect3DTexture8 *pID3DTexture = NULL;
  IDirect3DCubeTexture8 *pID3DCubeTexture = NULL;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;
  D3DSURFACE_DESC ddsdDescDest;

  if (m_eTT != eTT_Cubemap)
  {
    pID3DTexture = (IDirect3DTexture8*)m_RefTex->m_VidTex;
    Mips[0].Free();
    for (int i=0; i<m_nMips; i++)
    {
      pID3DTexture->GetLevelDesc(i, &ddsdDescDest);
      assert (ddsdDescDest.Width && ddsdDescDest.Height);
      int size = CD3D8TexMan::TexSize(ddsdDescDest.Width, ddsdDescDest.Height, ddsdDescDest.Format);
      SMipmap *mp = new SMipmap(ddsdDescDest.Width, ddsdDescDest.Height, size);
      h = pID3DTexture->LockRect(i, &d3dlr, NULL, 0);
      memcpy(&mp->DataArray[0], d3dlr.pBits, size);
      pID3DTexture->UnlockRect(i);
      Mips[0].AddElem(mp);
    }
  }
  else
  {
    pID3DCubeTexture = (IDirect3DCubeTexture8*)m_RefTex->m_VidTex;
    for (int n=0; n<6; n++)
    {
      Mips[n].Free();
      for (int i=0; i<m_nMips; i++)
      {
        pID3DCubeTexture->GetLevelDesc(i, &ddsdDescDest);
        assert (ddsdDescDest.Width && ddsdDescDest.Height);
        int size = CD3D8TexMan::TexSize(ddsdDescDest.Width, ddsdDescDest.Height, ddsdDescDest.Format);
        SMipmap *mp = new SMipmap(ddsdDescDest.Width, ddsdDescDest.Height, size);
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)n, i, &d3dlr, NULL, 0);
        memcpy(&mp->DataArray[0], d3dlr.pBits, size);
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)n, i);
        Mips[n].AddElem(mp);
      }
    }
  }
}

void STexPic::DownloadMips(TArray<SMipmap *>* Mips, int nStartMip, int nEndMip, int SizeFirst)
{
}

void STexPicD3D::DownloadMips(TArray<SMipmap *>* Mips, int nStartMip, int nEndMip, int SizeFirst)
{
  CD3D8Renderer *r = gcpRendD3D;
  LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
  IDirect3DTexture8 *pID3DTexture = NULL;
  IDirect3DCubeTexture8 *pID3DCubeTexture = NULL;
  HRESULT h;
  D3DLOCKED_RECT d3dlr;

  if (m_eTT != eTT_Cubemap)
  {
    pID3DTexture = (IDirect3DTexture8*)m_RefTex->m_VidTex;
    if (!pID3DTexture)
    {
      if( FAILED( h = D3DXCreateTexture(dv, m_Width, m_Height, m_nMips, 0, (D3DFORMAT)m_RefTex->m_DstFormat, D3DPOOL_MANAGED, &pID3DTexture ) ) )
        return;
      m_RefTex->m_VidTex = pID3DTexture;
    }
#ifndef _XBOX
    pID3DTexture->SetLOD(nStartMip);
#endif
    for (int i=0; i<Mips[0].Num(); i++)
    {
      int nLod = i+nStartMip;
      SMipmap *mp = Mips[0][i];
      int size = CD3D8TexMan::TexSize(mp->USize, mp->VSize, m_RefTex->m_DstFormat);
      h = pID3DTexture->LockRect(nLod, &d3dlr, NULL, 0);
      // Copy data to video texture 
      memcpy((byte *)d3dlr.pBits, &mp->DataArray[0], size);
      // Unlock the video texture
      pID3DTexture->UnlockRect(nLod);
    }
    Mips[0].Free();
  }
  else
  {
    pID3DCubeTexture = (IDirect3DCubeTexture8*)m_RefTex->m_VidTex;
    if (!pID3DCubeTexture)
    {
      if( FAILED( h = D3DXCreateCubeTexture(dv, m_Width, m_nMips, 0, (D3DFORMAT)m_RefTex->m_DstFormat, D3DPOOL_MANAGED, &pID3DCubeTexture ) ) )
        return;
      m_RefTex->m_VidTex = pID3DCubeTexture;
    }
#ifndef _XBOX
    pID3DCubeTexture->SetLOD(nStartMip);
#endif
    for (int n=0; n<6; n++)
    {
      for (int i=0; i<Mips[n].Num(); i++)
      {
        int nLod = i+nStartMip;
        SMipmap *mp = Mips[n][i];
        int size = CD3D8TexMan::TexSize(mp->USize, mp->VSize, m_RefTex->m_DstFormat);
        h = pID3DCubeTexture->LockRect((D3DCUBEMAP_FACES)n, nLod, &d3dlr, NULL, 0);
        // Copy data to video texture 
        memcpy((byte *)d3dlr.pBits, &mp->DataArray[0], size);
        // Unlock the video texture
        pID3DCubeTexture->UnlockRect((D3DCUBEMAP_FACES)n, nLod);
      }
      Mips[n].Free();
    }
  }
}

