#include "stdafx.h"

// New version of light sources management - not used yet

#include "3dEngine.h"
#include "objman.h"
#include "visareas.h"

#include "lightman.h"

#ifndef PI
#define PI 3.14159f
#endif

CLightManager::CLightManager()
{
  m_arrLights = new std::map<int, LightsSet*>;
}

CLightManager::~CLightManager()
{
  for (LightsMap::iterator it = m_arrLights->begin(); it != m_arrLights->end(); ++it)
  { // free lights lists
    LightsSet * pCellLights = (*it).second;
    delete pCellLights;
  }

  m_arrLights->clear();
  delete m_arrLights;
}

void CLightManager::GetLightBounds(CDLight * pLight, int &min_x, int &min_y, int &max_x, int &max_y)
{
  // find lights 2d bounds
  Vec3d vRadius(pLight->m_fRadius,pLight->m_fRadius,pLight->m_fRadius);
  Vec3d vBoxMin = pLight->m_Origin - vRadius;
  Vec3d vBoxMax = pLight->m_Origin + vRadius;

  // get 2d bounds in sectors array
  min_x = (int)(((vBoxMin.x - 1.f)/LIGHT_GRID_SIZE));
  min_y = (int)(((vBoxMin.y - 1.f)/LIGHT_GRID_SIZE));
  max_x = (int)(((vBoxMax.x + 1.f)/LIGHT_GRID_SIZE));
  max_y = (int)(((vBoxMax.y + 1.f)/LIGHT_GRID_SIZE));

  int nTableSize = CTerrain::GetTerrainSize()/LIGHT_GRID_SIZE;

  if( min_x<0 ) min_x = 0; else if( min_x>=nTableSize ) min_x = nTableSize-1;
  if( min_y<0 ) min_y = 0; else if( min_y>=nTableSize ) min_y = nTableSize-1;
  if( max_x<0 ) max_x = 0; else if( max_x>=nTableSize ) max_x = nTableSize-1;
  if( max_y<0 ) max_y = 0; else if( max_y>=nTableSize ) max_y = nTableSize-1;
}

void CLightManager::AddLight(CDLight * pLight)
{
  int min_x, min_y, max_x, max_y;
  GetLightBounds(pLight, min_x, min_y, max_x, max_y);

  int nTableSize = CTerrain::GetTerrainSize()/LIGHT_GRID_SIZE;

  for(int x=min_x; x<=max_x; x++)
    for(int y=min_y; y<=max_y; y++)
    {
      // find cell from position
      int nXY = x+y*nTableSize;
      LightsMap::iterator itTable = m_arrLights->find(nXY);
      LightsSet * pLightsSet = 0;
      if(itTable!=m_arrLights->end())
        pLightsSet = (*itTable).second;
      else
      { // allocate new set if needed
        pLightsSet = new LightsSet;
        m_arrLights->insert(LightsMap::value_type(nXY,pLightsSet));
      }

      // add light if not found
      LightsSet::iterator itLightsSet = pLightsSet->find(pLight);
      if(itLightsSet==pLightsSet->end())
        pLightsSet->insert(pLight);
    }
}

void CLightManager::DeleteLight(CDLight*pLight)
{
  int min_x, min_y, max_x, max_y;
  GetLightBounds(pLight, min_x, min_y, max_x, max_y);

  int nTableSize = CTerrain::GetTerrainSize()/LIGHT_GRID_SIZE;

  for(int x=min_x; x<=max_x; x++)
    for(int y=min_y; y<=max_y; y++)
    {
      // find cell from position
      int nXY = x+y*nTableSize;
      LightsMap::iterator itTable = m_arrLights->find(nXY);
      LightsSet * pLightsSet = 0;
      if(itTable!=m_arrLights->end())
      {
        pLightsSet = (*itTable).second;

        // delete light if found
        LightsSet::iterator itLightsSet = pLightsSet->find(pLight);
        if(itLightsSet!=pLightsSet->end())
        {
          pLightsSet->erase(itLightsSet);
          if(pLightsSet->empty())
          {
            delete pLightsSet;
            m_arrLights->erase(itTable);
          }
        }
      }
    }
}

void CLightManager::GetLightsAffectingBBox(const Vec3d & vBoxMin, const Vec3d & vBoxMax, LightsSet * pOutputList)
{
  pOutputList->clear();

  // get 2d bounds in sectors array
  int min_x = (int)(((vBoxMin.x - 1.f)/LIGHT_GRID_SIZE));
  int min_y = (int)(((vBoxMin.y - 1.f)/LIGHT_GRID_SIZE));
  int max_x = (int)(((vBoxMax.x + 1.f)/LIGHT_GRID_SIZE));
  int max_y = (int)(((vBoxMax.y + 1.f)/LIGHT_GRID_SIZE));

  int nTableSize = CTerrain::GetTerrainSize()/LIGHT_GRID_SIZE;

  if( min_x<0 ) min_x = 0; else if( min_x>=nTableSize ) min_x = nTableSize-1;
  if( min_y<0 ) min_y = 0; else if( min_y>=nTableSize ) min_y = nTableSize-1;
  if( max_x<0 ) max_x = 0; else if( max_x>=nTableSize ) max_x = nTableSize-1;
  if( max_y<0 ) max_y = 0; else if( max_y>=nTableSize ) max_y = nTableSize-1;

  for(int x=min_x; x<=max_x; x++)
    for(int y=min_y; y<=max_y; y++)
    {
      // find cell from position
      int nXY = x+y*nTableSize;
      LightsMap::iterator itTable = m_arrLights->find(nXY);
      LightsSet * pLightsSet = 0;
      if(itTable!=m_arrLights->end())
      {
        pLightsSet = (*itTable).second;

        // add lights into output list if found
        for (LightsSet::iterator it = pLightsSet->begin(); it != pLightsSet->end(); ++it)
        {
          if(pOutputList->find(*it) == pOutputList->end())
            pOutputList->insert(*it);
        }
      }
    }
}

extern int __cdecl C3DEngine__Cmp_LightAmount(const void* v1, const void* v2);

int CLightManager::MakeLMaskFromPositionAndActivateLights( const Vec3d vObjPos, const float fObjRadius, 
  IEntityRender * pEntityRender, int nMaxLightBitsNum, CDLight ** pSelectedLights, int nMaxSelectedLights)
{
  Vec3d vBoxMin, vBoxMax;
  pEntityRender->GetRenderBBox(vBoxMin, vBoxMax);
  GetLightsAffectingBBox(vBoxMin, vBoxMax, &m_setTmpDL_MMFP);

	// make list of really affecting light sources
  m_lstTmpDLA_MMFP.Clear();
  for(LightsSet::iterator itLightsSet = m_setTmpDL_MMFP.begin(); itLightsSet != m_setTmpDL_MMFP.end(); ++itLightsSet)
  {
    CDLight * pDLight = (*itLightsSet);
		if(!pDLight || pDLight->m_fRadius < 0.5f || pDLight->m_Flags & DLF_FAKE)
			continue;

		if (pDLight->m_pShader!=0 && (pDLight->m_pShader->GetLFlags() & LMF_DISABLE))
			continue; // fake

    if(pEntityRender && pDLight->m_Flags & DLF_LM && pEntityRender->GetRndFlags() & ERF_USELIGHTMAPS && pEntityRender->HasLightmap(0))
    { // in case of lightmaps
      if(pDLight->m_SpecColor == Col_Black)
        continue; // ignore specular only lights if specular disabled

      if(pDLight->m_Flags & DLF_PROJECT)
        continue; // ignore specular only lights if projector since specular projectors not supported
    }

    if(pDLight->m_Flags & DLF_PROJECT && pEntityRender)
    { // check projector frustum
      // use pDLight->m_TextureMatrix to construct Plane
      /*GetRenderer()->Draw3dBBox(pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtX(), DPRIM_LINE);
      GetRenderer()->DrawLabel(pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtX(),1,"x");
      GetRenderer()->Draw3dBBox(pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtY(), DPRIM_LINE);
      GetRenderer()->DrawLabel(pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtY(),1,"y");
      GetRenderer()->Draw3dBBox(pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtZ(), DPRIM_LINE);
      GetRenderer()->DrawLabel(pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtZ(),1,"z");*/					

      if(GetCVars()->e_projector_exact_test)
      { // test
        CCamera cam; // construct light camera
        cam.SetPos(pDLight->m_Origin);
        Vec3d Angles(pDLight->m_ProjAngles[1], 0, pDLight->m_ProjAngles[2]+90.0f);
        cam.SetAngle(Angles);
        cam.Init(1, 1, (pDLight->m_fLightFrustumAngle*2)/180.0f*PI, pDLight->m_fRadius, 1.0f, 0.1f);
        cam.Update();

        Vec3d vBoxMin,vBoxMax;
        pEntityRender->GetRenderBBox(vBoxMin,vBoxMax);
        if (!cam.IsAABBVisibleFast(AABB(vBoxMin,vBoxMax)))
          continue;
      }
      else
      {
        Plane p;
        p.CalcPlane( pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtY(), pDLight->m_Origin-pDLight->m_TextureMatrix.GetOrtZ() );
        if( p.DistFromPlane(vObjPos) + fObjRadius < 0 )
          continue;
      }
    }

		// find amount of light
		float fDist = GetDistance(pDLight->m_Origin,vObjPos);
		float fLightAmount = 1.f - (fDist-fObjRadius) / (pDLight->m_fRadius);
		fLightAmount *= 
      (pDLight->m_Color.r+pDLight->m_Color.g+pDLight->m_Color.b)*0.333f + 
      (pDLight->m_SpecColor.r+pDLight->m_SpecColor.g+pDLight->m_SpecColor.b)*0.333f;

    if(fLightAmount>0.05f)
    {
      // if entity is inside some area - allow lightsources only from this area
      if(pEntityRender)
        if((pDLight->m_Flags & DLF_THIS_AREA_ONLY) /*|| (pDLight->m_Flags & DLF_SUN)*/)
        {
          if(pEntityRender->GetEntityVisArea() && pDLight->m_pOwner!=(IEntityRender*)-1)
          {
            if(	pDLight->m_pOwner && pDLight->m_pOwner->GetEntityRS() && pDLight->m_pOwner->m_pVisArea)
            {
              CVisArea * pLightArea = pDLight->m_pOwner->m_pVisArea;
              if(pEntityRender->GetEntityVisArea() != pLightArea)
              {	// try also portal volumes
                bool bNearFound = pEntityRender->m_pVisArea->FindVisArea(pLightArea, 1, true);
                if(!bNearFound)
                  continue; // areas do not much
              }
            }
            else
              continue; // outdoor lsource
          }
          else // entity is outside
            if(	pDLight->m_pOwner  && pDLight->m_pOwner!=(IEntityRender*)-1 && pDLight->m_pOwner->GetEntityVisArea() && pDLight->m_pOwner!=(IEntityRender*)-1)
              continue; // indoor lsource should not affect outdoor entity
        }

        DLightAmount la;
        la.pDLight = pDLight;
        la.fAmount = fLightAmount;
        m_lstTmpDLA_MMFP.Add(la);
    }
  }

	int nDLightMask = 0;

	if(!m_lstTmpDLA_MMFP.Count())
		return 0; // no lsources found

	// sort by light amount
	qsort(&m_lstTmpDLA_MMFP[0], m_lstTmpDLA_MMFP.Count(), sizeof(m_lstTmpDLA_MMFP[0]), C3DEngine__Cmp_LightAmount);

	// limit number of affective light sources
  for(int n=0; n<nMaxLightBitsNum && n<GetCVars()->e_max_entity_lights && n<m_lstTmpDLA_MMFP.Count(); n++)
  {
    LightsSet::iterator itLightsSet = m_setActiveLights.find(m_lstTmpDLA_MMFP[n].pDLight);
    if(itLightsSet==m_setActiveLights.end())
    {
      GetRenderer()->EF_ADDDlight(m_lstTmpDLA_MMFP[n].pDLight);
      m_setActiveLights.insert(m_lstTmpDLA_MMFP[n].pDLight);
    }

    const int nId = m_lstTmpDLA_MMFP[n].pDLight->m_Id;

    nDLightMask |= (1<<nId);

    if(pSelectedLights && n<nMaxSelectedLights)
      pSelectedLights[n] = m_lstTmpDLA_MMFP[n].pDLight;
  }

  return nDLightMask;
}

void CLightManager::ClearFrameLights()
{
  m_setActiveLights.clear();
}

const Vec3d &CLightEntity::GetPos(bool) const
{
  return m_vPos;//m_pLight->m_Origin; 
}

const Vec3d &CLightEntity::GetAngles(int) const
{
  return m_pLight->m_ProjAngles;
}

void CLightEntity::GetRenderBBox(Vec3d &vMin,Vec3d &vMax)
{
  vMin = m_pLight->m_Origin - Vec3d(m_pLight->m_fRadius,m_pLight->m_fRadius,m_pLight->m_fRadius);
  vMax = m_pLight->m_Origin + Vec3d(m_pLight->m_fRadius,m_pLight->m_fRadius,m_pLight->m_fRadius);
}

float CLightEntity::GetRenderRadius(void) const
{
  return m_pLight->m_fRadius;
}

float CLightEntity::GetMaxViewDist()
{
	return GetRenderRadius()*GetCVars()->e_obj_view_dist_ratio*GetViewDistRatioNormilized();
}

CLightEntity::CLightEntity()
{
  m_pEntityRenderState = Get3DEngine()->MakeEntityRenderState();
  m_vPos.Set(0,0,0);
}

CLightEntity::~CLightEntity()
{
	Get3DEngine()->FreeEntityRenderState(this);
  ((C3DEngine*)Get3DEngine())->FreeLightSourceComponents(m_pLight);
  Get3DEngine()->UnRegisterEntity(this);
	((C3DEngine*)Get3DEngine())->RemoveEntityLightSources(this);
	delete m_pLight; m_pLight = NULL;
}
