////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   bflyes.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
/*
#ifndef _BFLIES_H
#define _BFLIES_H



#define MAX_BF_COUNT 256
#define BF_RANGE 16
const int BF_RANGEx2 = BF_RANGE*2;
const int BF_CAMERA_SHIFT = BF_RANGE-1;

#define rn() ((((float)rand())/RAND_MAX)-0.5f)
#define fBFPerformanceFactor (pITimer->GetFrameTime()*40)

#define GROUND_LEVEL 0.1f

class Cry_Butterfly
{
  float m_fWingPos,m_fAngle,m_fAngleDelta,m_fHigh,m_fHightDelta,m_fLifeSpeed,m_fSize;
  Vec3d m_vCurWingPos;
  bool  m_bMoveDir;

  inline void Limit(float * val, float min, float max)
  {
    if(*val > max)      *val = max;
    else if(*val < min) *val = min;
  }

  bool IsPointInvalid(const Vec3d & pos);

public:
  Cry_Butterfly();
  void Render(ITimer * pITimer, IRenderer * pIRenderer, const Vec3d & vCamPos, const bool & bEven, const Vec3d & vColor, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pVertBufChunk, CTerrain * pTerrain );

  Vec3d m_vPos;
};

class CBFManager : public Cry3DEngineBase
{
  Cry_Butterfly * m_pBFArray;
  CVertexBuffer * m_pVideoBuffer;
  SVertexStream   m_Indixes;
  int m_nTexID;
  bool m_bEven;
	Vec3d m_vForward;	
  int m_nCurrentObjectsCount;

public:
  CBFManager();
  void Render(CTerrain * pTerrain);
  ~CBFManager();
  void KillBF(const Vec3d & vExploPos, const float fRadius);
  void SetCount(int nCount)  {  m_nCurrentObjectsCount = min(nCount, MAX_BF_COUNT); }
  int  GetCount()  {  return m_nCurrentObjectsCount; }
	int GetMemoryUsage() { return sizeof(Cry_Butterfly)*MAX_BF_COUNT + sizeof(ushort)*MAX_BF_COUNT; }
};


#endif 
*/