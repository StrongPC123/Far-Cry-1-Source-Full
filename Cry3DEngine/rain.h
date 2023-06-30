////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   rain.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef RAIN_MANAGER
#define RAIN_MANAGER

#define RAIN_COUNT 512
const float RAIN_RANGE = 8;
const float RAIN_RANGEx2 = RAIN_RANGE*2;

class CPartManager;

class CRainItem
{
public:

  // cur state
  Vec3d  m_vPos;
  float  m_fSize;

  CRainItem();
  void Process(Vec3d &right, Vec3d &up, Vec3d &front, const int & nTexID, const Vec3d & delta,
         IRenderer * pIRenderer, ITimer * pITimer, const Vec3d & vFocusPos, 
				 CPartManager * pPartManager, CTerrain * pTerrain, class CObjManager * pObjManager, const Vec3d & vCamPos );

  bool IsPointInvalid(const Vec3d & pos);
};

class CRainManager : public Cry3DEngineBase
{
  CRainItem m_arrItems[RAIN_COUNT];
  int m_nCurItem;
  int m_nRainTexID;
	float m_fDropTime;
public:
  
  CRainManager() 
  { 
    m_nCurItem = 0; 
    m_nRainTexID = 0;
		m_fDropTime = 0;
  }
  void Render(class CTerrain * pTerrain, const Vec3d & vColor, class CObjManager * pObjManager, class CPartManager * pPartManager, const Vec3d & vWindDir);
};

#endif // RAIN_MANAGER
