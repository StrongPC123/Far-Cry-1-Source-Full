#ifndef _LIGHTMAN_H_
#define _LIGHTMAN_H_

#include "Array2d.h"

#define LIGHT_GRID_SIZE 16

class CLightManager : public Cry3DEngineBase
{
  typedef std::set<class CDLight*> LightsSet;
  typedef std::map<int, LightsSet*> LightsMap;

  LightsMap * m_arrLights;

public:
  CLightManager();
  ~CLightManager();
  void AddLight(CDLight*pLight);
  void DeleteLight(CDLight*pLight);
  void GetLightBounds(CDLight * pLight, int &min_x, int &min_y, int &max_x, int &max_y);
  
  void GetLightsAffectingBBox(const Vec3d & vBoxMin, const Vec3d & vBoxMax, LightsSet*pOutputList);
  int MakeLMaskFromPositionAndActivateLights( const Vec3d vObjPos, const float fObjRadius, 
    IEntityRender * pEntityRender, int nMaxLightBitsNum, 
    CDLight ** pSelectedLights, int nMaxSelectedLights);

  struct DLightAmount{ CDLight * pDLight; float fAmount; };
  list2<DLightAmount> m_lstTmpDLA_MMFP;
  LightsSet m_setTmpDL_MMFP;
  LightsSet m_setActiveLights;

  void ClearFrameLights();
};

#endif // _LIGHTMAN_H_