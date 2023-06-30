////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_hmap.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: highmap
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

float CHighMap::GetZApr(float x1, float y1)
{
  float dDownLandZ;

  if( x1<1 || y1<1 || x1>=CTerrain::GetTerrainSize() || y1>=CTerrain::GetTerrainSize()  )
    dDownLandZ = BOTTOM_LEVEL;
  else
  {
    // convert into hmap space
    x1 /= CTerrain::GetHeightMapUnitSize();
    y1 /= CTerrain::GetHeightMapUnitSize();

    int nX = fastftol_positive(x1);
    int nY = fastftol_positive(y1);

    float dx1 = x1 - nX;
    float dy1 = y1 - nY;

    float dDownLandZ0 = 
      (1.f-dx1) * (m_arrusHightMapData[nX  ][nY  ] & (~INFO_BITS_MASK)) + 
      (    dx1) * (m_arrusHightMapData[nX+1][nY  ] & (~INFO_BITS_MASK));
    
    float dDownLandZ1 = 
      (1.f-dx1) * (m_arrusHightMapData[nX  ][nY+1] & (~INFO_BITS_MASK)) + 
      (    dx1) * (m_arrusHightMapData[nX+1][nY+1] & (~INFO_BITS_MASK));

    dDownLandZ = (1-dy1) * dDownLandZ0 + (  dy1) * dDownLandZ1;

    dDownLandZ *= TERRAIN_Z_RATIO;

    if(dDownLandZ < BOTTOM_LEVEL)
      dDownLandZ = BOTTOM_LEVEL;
  } 

  return dDownLandZ;
}

#if defined(WIN32) && defined (_CPU_X86)
inline int fastround_positive(float f) // note: only positive numbers works correct
{
	int i;
	__asm fld [f]
	__asm fistp [i]
	return i;
}
#else
inline int fastround_positive(float f) { int i; i=(int)(f+0.5f); return i; } // note: only positive numbers works correct
#endif

bool CHighMap::IntersectWithSector(Vec3d vStartPoint, Vec3d vStopPoint, float fDist, int nMaxTestsToScip)
{
  // convert into hmap space
	float fInvUnitSize = CTerrain::GetInvUnitSize();
  vStopPoint.x *= fInvUnitSize;
  vStopPoint.y *= fInvUnitSize;
  vStopPoint.z *= INV_TERRAIN_Z_RATIO;
  vStartPoint.x *= fInvUnitSize;
  vStartPoint.y *= fInvUnitSize;
  vStartPoint.z *= INV_TERRAIN_Z_RATIO;

  // scan hmap in sector
  Vec3d vDir = (vStopPoint - vStartPoint);
  int nSteps = fastftol_positive(fDist/(CTerrain::GetHeightMapUnitSize()*4)); // every 4 units

  //int nSteps = int(vDir.Length()/CTerrain::GetHeightMapUnitSize()/8); 

	switch(Cry3DEngineBase::m_pCVars->e_terrain_occlusion_culling)
	{
		case 4:											// far objects are culled less precise but with far hills as well (same culling speed)
			if(nSteps>50)
				nSteps=50;
			vDir /= (float)nSteps;
			break;
		default:											// far hills are not culling
			vDir /= (float)nSteps;
			if(nSteps>50)
				nSteps=50;
			break;
	}

  Vec3d vPos = vStartPoint;
  vPos.z+=(INFO_BITS_MASK);

	int nTest=0;
  for(nTest=0; nTest<nSteps && nTest<nMaxTestsToScip; nTest++)
  {
    if(vPos.z < m_arrusHightMapData[fastround_positive(vPos.x)][fastround_positive(vPos.y)])
      vPos += vDir;
    else
      break;
  }
  
	nMaxTestsToScip = min(nMaxTestsToScip,4);
  for(; nTest<nSteps-nMaxTestsToScip; nTest++)
  {
    vPos += vDir;
    if(vPos.z < m_arrusHightMapData[fastround_positive(vPos.x)][fastround_positive(vPos.y)])
      return true;
  }
  
  return false;
}

bool CHighMap::IsPointOccludedByTerrain(const Vec3d & _vPoint, float fDist, const Vec3d & vCamPos, int nMaxTestsToScip)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

  // test works only inside map
  if( _vPoint.x<0 || _vPoint.y<0 || _vPoint.x>CTerrain::GetTerrainSize() || _vPoint.y>CTerrain::GetTerrainSize() )
    return false;
  if( vCamPos.x<0 || vCamPos.y<0 || vCamPos.x>CTerrain::GetTerrainSize() || vCamPos.y>CTerrain::GetTerrainSize() )
    return false;

  return IntersectWithSector(vCamPos, _vPoint, fDist, nMaxTestsToScip);
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

bool CHighMap::LoadHighMap(const char* file_name, ICryPak * pCryPak)
{ // Terrain Land dimensions = file dimensions + 1
  int nArraySize = (CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize())*(CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize());
  unsigned short * pTmpBuff = new unsigned short[nArraySize];
  if(!pTmpBuff)
    return 0;

  FILE * f = pCryPak->FOpen(file_name, "rb");
  if(!f)
    return 0; 

  int nBytesReaded = pCryPak->FRead(pTmpBuff, sizeof(pTmpBuff[0]), (CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize())*(CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize()), f);

  if(nBytesReaded != nArraySize)
	{
		pCryPak->FClose(f);
    return 0;
	}

  pCryPak->FClose(f);
  m_arrusHightMapData.Allocate(CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize()+1);

  int nHolesCount=0;
  int nLightCount=0;
  int nDarkCount=0;
  for (int t = 1; t < CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize(); t++) 
  for (int s = 1; s < CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize(); s++) 
  {
    m_arrusHightMapData[t][s] = pTmpBuff[(t)*(CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize())+(s)];

    if((m_arrusHightMapData[t][s] & STYPE_BIT_MASK) == STYPE_HOLE)
      nHolesCount++;

    if(m_arrusHightMapData[t][s] & LIGHT_BIT_MASK)
      nLightCount++;
    else
      nDarkCount++;

    m_arrusHightMapData[t][s] &= ~EXPLO_BIT_MASK; // reset explo state
  }

  delete [] pTmpBuff;

  m_bHightMapModified = false;

  return f!=0;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

bool CHighMap::GetHoleSafe(const int & x, const int & y) 
{
  if(x>=0 && y>=0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize())
    return (m_arrusHightMapData[(x+1)>>HMAP_BIT_SHIFT][(y+1)>>HMAP_BIT_SHIFT] & STYPE_BIT_MASK) == STYPE_HOLE;
  return false;
}

float CHighMap::GetZSafe(int x, int y)
{ 
  if(x>=0 && y>=0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize() && GetZ(x,y) > BOTTOM_LEVEL)
    return  GetZ(x,y); 
  return BOTTOM_LEVEL; 
}

float CHighMap::GetZSafe(float x, float y)
{ 
  if(x>=0 && y>=0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize() && GetZ((int)x,(int)y) > BOTTOM_LEVEL)
    return  GetZ((int)x,(int)y); 
  return BOTTOM_LEVEL; 
}

/*
float CHighMap::GetZ(const int x, const int y) 
{ 
  return  TERRAIN_Z_RATIO*(m_arrusHightMapData[x>>HMAP_BIT_SHIFT][y>>HMAP_BIT_SHIFT] & (~INFO_BITS_MASK)); 
}
*/

float CHighMap::GetZF(const float x, const float y) 
{ 
  return  TERRAIN_Z_RATIO*(m_arrusHightMapData[int(x)>>HMAP_BIT_SHIFT][int(y)>>HMAP_BIT_SHIFT] & (~INFO_BITS_MASK)); 
}

bool CHighMap::IsOnTheLight(const int x, const int y) 
{
  if(x>=0 && y>=0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize())
    return (m_arrusHightMapData[(x+1)>>HMAP_BIT_SHIFT][(y+1)>>HMAP_BIT_SHIFT] & LIGHT_BIT_MASK)!=0; 
  return  true;   
}

uchar CHighMap::GetSurfaceTypeID(int x, int y)
{
  if(x>=0 && y>=0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize())
    return m_arrusHightMapData[x>>HMAP_BIT_SHIFT][y>>HMAP_BIT_SHIFT] & STYPE_BIT_MASK;
  return 0;
}

int CHighMap::GetSurfaceType(int x, int y)
{
  assert(CTerrain::GetHeightMapUnitSize()==2);
  if(x>0 && y>0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize())
  {
    if((m_arrusHightMapData[x>>HMAP_BIT_SHIFT][y>>HMAP_BIT_SHIFT] & STYPE_BIT_MASK) == STYPE_HOLE)
      return -1; // there is hole here

    return m_arrusHightMapData[x>>HMAP_BIT_SHIFT][y>>HMAP_BIT_SHIFT] & STYPE_BIT_MASK;
  }

  return -1; // out of the map
}

void CHighMap::SetLightValue(const int x, const int y, bool bValue) 
{     
  if(bValue)
    m_arrusHightMapData[(x+1)>>HMAP_BIT_SHIFT][(y+1)>>HMAP_BIT_SHIFT] |=  LIGHT_BIT_MASK; 
  else
    m_arrusHightMapData[(x+1)>>HMAP_BIT_SHIFT][(y+1)>>HMAP_BIT_SHIFT] &= ~LIGHT_BIT_MASK; 
}

void CHighMap::SetBurnedOut(int x, int y, bool bBurnedOut) 
{ 
  if(bBurnedOut)
    m_arrusHightMapData[(x+1)>>HMAP_BIT_SHIFT][(y+1)>>HMAP_BIT_SHIFT] |=  EXPLO_BIT_MASK; 
  else
    m_arrusHightMapData[(x+1)>>HMAP_BIT_SHIFT][(y+1)>>HMAP_BIT_SHIFT] &= ~EXPLO_BIT_MASK; 
}

bool CHighMap::IsBurnedOut(int x, int y) 
{
  if(x>=0 && y>=0 && x<CTerrain::GetTerrainSize() && y<CTerrain::GetTerrainSize())
    return (EXPLO_BIT_MASK & m_arrusHightMapData[(x+HMAP_BIT_SHIFT)>>HMAP_BIT_SHIFT][(y+HMAP_BIT_SHIFT)>>HMAP_BIT_SHIFT]) != 0;
  
  return 0;
}
