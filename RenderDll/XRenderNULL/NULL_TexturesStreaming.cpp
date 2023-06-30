/*=============================================================================
  PS2_TexturesStreaming.cpp : PS2 specific texture streaming technology.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "NULL_Renderer.h"

//===============================================================================

void STexPic::BuildMips()
{
}

bool STexPic::UploadMips(int nStartMip, int nEndMip)
{
  return true;
}

void STexPic::RemoveFromPool()
{
}

void CTexMan::UnloadOldTextures(STexPic *pExclude)
{
}

void CTexMan::CheckTexLimits(STexPic *pExclude)
{
}

void STexPic::PrecacheAsynchronously(float fDist, int Flags)
{
}

