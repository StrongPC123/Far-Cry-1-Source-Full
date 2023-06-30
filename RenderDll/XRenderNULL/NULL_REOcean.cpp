/*=============================================================================
  PS2_REOcean.cpp : implementation of the Ocean Rendering.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "NULL_Renderer.h"
#include "I3dengine.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//=======================================================================

void CREOcean::mfReset()
{
}

bool CREOcean::mfPreDraw(SShaderPass *sl)
{
  return true;
}

void CREOcean::UpdateTexture()
{
}

void CREOcean::DrawOceanSector(SOceanIndicies *oi)
{
}

static _inline int Compare(SOceanSector *& p1, SOceanSector *& p2)
{
  if(p1->m_Flags > p2->m_Flags)
    return 1;
  else
  if(p1->m_Flags < p2->m_Flags)
    return -1;
  
  return 0;
}

static _inline float sCalcSplash(SSplash *spl, float fX, float fY)
{
  CNULLRenderer *r = gcpNULL;

  float fDeltaTime = r->m_RP.m_RealTime - spl->m_fStartTime;
  float fScaleFactor = 1.0f / (r->m_RP.m_RealTime - spl->m_fLastTime + 1.0f);

  float vDelt[2];

  // Calculate 2D distance
  vDelt[0] = spl->m_Pos[0] - fX; vDelt[1] = spl->m_Pos[1] - fY;
  float fSqDist = vDelt[0]*vDelt[0] + vDelt[1]*vDelt[1];

  // Inverse square root
  unsigned int *n1 = (unsigned int *)&fSqDist;
  unsigned int nn = 0x5f3759df - (*n1 >> 1);
  float *n2 = (float *)&nn;
  float fDistSplash = 1.0f / ((1.5f - (fSqDist * 0.5f) * *n2 * *n2) * *n2);

  // Emulate sin waves
  float fDistFactor = fDeltaTime*10.0f - fDistSplash + 4.0f;
  fDistFactor = CLAMP(fDistFactor, 0.0f, 1.0f);
  float fRad = (fDistSplash - fDeltaTime*10) * 0.4f / 3.1416f * 1024.0f;
  float fSin = gRenDev->m_RP.m_tSinTable[QRound(fRad)&0x3ff] * fDistFactor;

  return fSin * fScaleFactor * spl->m_fForce;
}

float *CREOcean::mfFillAdditionalBuffer(SOceanSector *os, int nSplashes, SSplash *pSplashes[], int& nCurSize, int nLod, float fSize)
{
  return NULL;
}

void CREOcean::mfDrawOceanSectors()
{ 
}


void CREOcean::mfDrawOceanScreenLod()
{ 
}

bool CREOcean::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  double time0 = 0;
  ticks(time0);

  if (CRenderer::CV_r_oceanrendtype == 0)
    mfDrawOceanSectors();
  else
    mfDrawOceanScreenLod();
  unticks(time0);
  m_RS.m_StatsTimeRendOcean = (float)(time0*1000.0*g_SecondsPerCycle);

  return true;
}


