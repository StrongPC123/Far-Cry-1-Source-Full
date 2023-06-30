////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjmanshadows.cpp
//  Version:     v1.00
//  Created:     2/6/2002 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Shadow casters/reseivers relations
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "objman.h"
#include "visareas.h"
#include "3dengine.h"
#include <AABBSV.h>

#define MIN_SHADOW_CASTER_VIEW_DIST 16

void CObjManager::MakeShadowCastersListInArea(CBasicArea*pArea, IEntityRender * pReceiver, list2<IEntityRender*> * pEntList, int dwAllowedTypes, Vec3d vLightPos, float fLightRadius)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	bool bCmpCasterReceiverDistances = !(GetRenderer()->GetFeatures() & (RFT_DEPTHMAPS | RFT_SHADOWMAP_SELFSHADOW));

	Shadowvolume ResShadowVolume;
	{
		Vec3d vBoxMin,vBoxMax;
		pReceiver->GetBBox(vBoxMin,vBoxMax);
		AABB aabbReceiver(vBoxMin,vBoxMax+Vec3d(0.01f,0.01f,0.01f));
		NAABB_SV::AABB_ReceiverShadowVolume(vLightPos, aabbReceiver, ResShadowVolume);//, min(fDistFromLsToCaster+pCaster->GetRenderRadius()*4, fLightRadius));
	}

	float fDistFromLsToReceiver = GetDistance(vLightPos, pReceiver->GetPos());

	int nStaticsAllowed = int(GetCVars()->e_shadow_maps_from_static_objects>0);
	for(int nStatic=0; nStatic<=nStaticsAllowed; nStatic++)
	{
		list2<struct IEntityRender*> * pList = &pArea->m_lstEntities[nStatic];
		if(nStatic && pArea->m_StaticEntitiesSorted && !(dwAllowedTypes&SMC_ALLOW_PASSIVE_SHADOWMAP_CASTERS))
			pList = &pArea->m_lstStaticShadowMapCasters;

		for(int e=0; e<pList->Count(); e++)
		{
			IEntityRender * pCaster = (*pList)[e];

			if (e+1 < pList->Count())
			{
				IEntityRender * pNext = (*pList)[e+1];
				cryPrefetchT0SSE(pNext);
			}

			if(pCaster->m_fWSMaxViewDist<MIN_SHADOW_CASTER_VIEW_DIST)
			{
				if(!nStatic || !pArea->m_StaticEntitiesSorted)
					continue;
				else
					break; // break in sorted list
			}

	#ifdef _DEBUG
			const char * szClass = pCaster->GetEntityClassName();
			const char * szName  = pCaster->GetName();
	#endif // _DEBUG

			int dwRndFlags = pCaster->GetRndFlags();

			bool bTakeThisOne = false;
			{ // take only allowed active shadow casters
				if((pCaster->IsStatic() && (dwAllowedTypes & SMC_STATICS )) ||
					(!pCaster->IsStatic() && (dwAllowedTypes & SMC_DYNAMICS)) )
				{
					if(dwAllowedTypes&SMC_ALLOW_PASSIVE_SHADOWMAP_CASTERS)
						bTakeThisOne = true;
					else
					if(dwRndFlags & ERF_CASTSHADOWMAPS && pCaster->GetEntityRS()->pShadowMapInfo)
					{
						ShadowMapLightSource * pFrustumContainer = pCaster->GetShadowMapFrustumContainer();
						if(	pFrustumContainer && pFrustumContainer->m_LightFrustums.Count() && pFrustumContainer->m_LightFrustums.Get(0)->pLs )
							bTakeThisOne = true;
					}
				}
			}

			if(bTakeThisOne && pCaster != pReceiver && !(dwRndFlags&ERF_HIDDEN) && (!pCaster->GetLight() || pCaster->GetContainer()))
			{
				float fDistFromLsToCaster = GetDistance(vLightPos, pCaster->GetPos());
				if(bCmpCasterReceiverDistances)
					if(fDistFromLsToReceiver < fDistFromLsToCaster)
						continue;

				if(fDistFromLsToReceiver - fDistFromLsToCaster > pCaster->GetRenderRadius()*4+pReceiver->GetRenderRadius())
					continue;

				// check if caster is in receiver frustum 
				Vec3d vBoxMin,vBoxMax;
				pCaster->GetBBox(vBoxMin,vBoxMax);
				AABB aabbCaster(vBoxMin,vBoxMax+Vec3d(0.01f,0.01f,0.01f));
				bool bIntersect = NAABB_SV::Is_AABB_In_ShadowVolume(ResShadowVolume, aabbCaster);
				if(bIntersect && pEntList->Find(pCaster)<0)
					pEntList->Add(pCaster);
			}
		}
	}
}

void CObjManager::MakeShadowCastersList(IEntityRender * pReceiver, list2<IEntityRender*> * pEntList, int dwAllowedTypes, Vec3d vLightPos, float fLightRadius)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	assert(vLightPos.len()>1); // world space pos required

	pEntList->Clear();

	if(pReceiver->m_pVisArea && pReceiver->IsEntityAreasVisible())
		MakeShadowCastersListInArea(pReceiver->m_pVisArea, pReceiver, pEntList, dwAllowedTypes, vLightPos, fLightRadius);
	else
	{	// make list of sectors around
		// find 2d bounds in sectors array
		Vec3d vBoxMin,vBoxMax;
		pReceiver->GetBBox(vBoxMin,vBoxMax);

		// get 2d bounds in sectors array
		int min_x = (int)(((vBoxMin.x - 16.f)/CTerrain::GetSectorSize()));
		int min_y = (int)(((vBoxMin.y - 16.f)/CTerrain::GetSectorSize()));
		int max_x = (int)(((vBoxMax.x + 16.f)/CTerrain::GetSectorSize()));
		int max_y = (int)(((vBoxMax.y + 16.f)/CTerrain::GetSectorSize()));

		// limit bounds
		if(min_x<0) min_x=0; else if(min_x>=CTerrain::GetSectorsTableSize()) min_x=CTerrain::GetSectorsTableSize()-1;
		if(min_y<0) min_y=0; else if(min_y>=CTerrain::GetSectorsTableSize()) min_y=CTerrain::GetSectorsTableSize()-1;
		if(max_x<0) max_x=0; else if(max_x>=CTerrain::GetSectorsTableSize()) max_x=CTerrain::GetSectorsTableSize()-1;
		if(max_y<0) max_y=0; else if(max_y>=CTerrain::GetSectorsTableSize()) max_y=CTerrain::GetSectorsTableSize()-1;

		m_lstTmpSectors_MELFP.Clear();
		m_lstTmpSectors_MELFP.Add(m_pTerrain->m_arrSecInfoTable[0][0]);
		for(int x=min_x; x<=max_x && x>=0 && x<=CTerrain::GetTerrainSize(); x++)
		for(int y=min_y; y<=max_y && y>=0 && y<=CTerrain::GetTerrainSize(); y++)
		{
			CSectorInfo * pSectorInfo = m_pTerrain->m_arrSecInfoTable[x][y];

			// check if sector cast shadow to the receiver
			if(m_lstTmpSectors_MELFP.Find(pSectorInfo)<0 && pSectorInfo)
			{
				Shadowvolume sv;
				Vec3d vBoxMin,vBoxMax;
				pReceiver->GetBBox(vBoxMin,vBoxMax);
				AABB aabbReceiver(vBoxMin,vBoxMax);
				AABB aabbCaster(pSectorInfo->m_vBoxMin,pSectorInfo->m_vBoxMax);
				aabbCaster.max.z += 0.01f;
				NAABB_SV::AABB_ShadowVolume(vLightPos, aabbCaster, sv, fLightRadius);
				bool bIntersect = NAABB_SV::Is_AABB_In_ShadowVolume(sv, aabbReceiver);
				if(bIntersect)
					m_lstTmpSectors_MELFP.Add(pSectorInfo);
			}
		}

		// make list of entities
		for(int s=0; s<m_lstTmpSectors_MELFP.Count(); s++)
			MakeShadowCastersListInArea(m_lstTmpSectors_MELFP[s], pReceiver, pEntList, dwAllowedTypes, vLightPos, fLightRadius);
	}
}

void CObjManager::MakeShadowMapInstancesList( IEntityRender * pReceiver, float obj_distance,
  list2<ShadowMapLightSourceInstance> * pLSourceInstances, int dwAllowedTypes, CDLight * pLight)
{
  if(!GetCVars()->e_shadow_maps || obj_distance > pReceiver->GetRenderRadius()*32)
    return;
 
  // get entities in area
  lstEntList_MLSMCIA.Clear();
  if(dwAllowedTypes)
    MakeShadowCastersList(pReceiver, &lstEntList_MLSMCIA, dwAllowedTypes, pLight->m_Origin, pLight->m_fRadius);
	lstEntList_MLSMCIA.Delete(pReceiver);

	// add this entity if self shadowing allowed
	if(	GetCVars()->e_shadow_maps_self_shadowing &&
			pReceiver->GetRndFlags() & ERF_SELFSHADOW && 
			GetRenderer()->GetFeatures() & (RFT_DEPTHMAPS | RFT_SHADOWMAP_SELFSHADOW))
		lstEntList_MLSMCIA.InsertBefore(pReceiver,0);

  // make list of shadow casters
  for(int e=0; e<lstEntList_MLSMCIA.Count(); e++)
  {
    const char * szClass = lstEntList_MLSMCIA[e]->GetEntityClassName();
    const char * szName = lstEntList_MLSMCIA[e]->GetName();

    if( !lstEntList_MLSMCIA[e]->GetShadowMapFrustumContainer() || 
        !lstEntList_MLSMCIA[e]->GetShadowMapFrustumContainer()->m_LightFrustums.Count() ||
        !(lstEntList_MLSMCIA[e]->GetRndFlags()&ERF_CASTSHADOWMAPS) )
      continue;

    ShadowMapLightSourceInstance LightSourceInfo;
    LightSourceInfo.m_pLS              = lstEntList_MLSMCIA[e]->GetShadowMapFrustumContainer();
    LightSourceInfo.m_vProjTranslation = lstEntList_MLSMCIA[e]->GetPos();
    LightSourceInfo.m_fProjScale       = lstEntList_MLSMCIA[e]->GetScale();
    Vec3d vThisEntityPos = pReceiver->GetPos();
    LightSourceInfo.m_fDistance        = GetDistance(vThisEntityPos,lstEntList_MLSMCIA[e]->GetPos()) - lstEntList_MLSMCIA[e]->GetRenderRadius()/3;
    LightSourceInfo.m_pReceiver        = pReceiver;
    if(!LightSourceInfo.m_pLS->m_LightFrustums.Count())// || !LightSourceInfo.m_pLS->m_LightFrustums[0].depth_tex_id)
      continue;
    pLSourceInstances->Add(LightSourceInfo);
  }

  // select only closest
  pLSourceInstances->SortByDistanceMember(true,1);
  while(pLSourceInstances->Count()>GetCVars()->e_shadow_maps_max_casters_per_object)
    pLSourceInstances->DeleteLast();
}

ShadowMapFrustum * CObjManager::MakeEntityShadowFrustum(ShadowMapFrustum * pFrustum,  
	ShadowMapLightSource * pLs, IEntityRender * pEnt, EShadowType nShadowType, int dwAllowedTypes)
{  
  Vec3d vMin,vMax;
  pEnt->GetBBox(vMin,vMax);

  Vec3d visual_center = (vMin+vMax)*0.5f - pEnt->GetPos();
  float model_radius = GetDistance(vMax,vMin) * 0.5f;

	float model_radiusXY = GetDistance(Vec3d(vMax.x,vMax.y,0),Vec3d(vMin.x,vMin.y,0)) * 0.5f;

	pFrustum->target = visual_center;
	Vec3d dir = pFrustum->target - pLs->vSrcPos;

	float dist = dir.Length();

	pFrustum->ProjRatio = model_radiusXY/model_radius;
	if(pFrustum->ProjRatio>1.f)
		pFrustum->ProjRatio=1.f;

  pFrustum->FOV = (float)RAD2DEG(cry_atanf((model_radius+0.25f)/dist))*1.9f;
  if(pFrustum->FOV>120.f)
    pFrustum->FOV=120.f;

  pFrustum->min_dist = dist - (model_radius + ((dwAllowedTypes&SMC_ALLOW_PASSIVE_SHADOWMAP_CASTERS) ? 32 : 0));

  if(pFrustum->min_dist<0.25)
    pFrustum->min_dist=0.25;
  
	pFrustum->max_dist = dist + ((dwAllowedTypes&SMC_ALLOW_PASSIVE_SHADOWMAP_CASTERS) ? -0.75f : model_radius);

  pFrustum->pLs = pLs;

  pFrustum->shadow_type = nShadowType;

	//  make entities list
  if(pFrustum->pEntityList)
    pFrustum->pEntityList->Clear();
  else
    pFrustum->pEntityList = new list2<IEntityRender*>;
  
  if(dwAllowedTypes)
    MakeShadowCastersList(pEnt, pFrustum->pEntityList, dwAllowedTypes, pLs->vSrcPos+pEnt->GetPos(), pLs->fRadius);
  else
    pFrustum->pEntityList->Add(pEnt);

  return pFrustum;
}

static int Cmp_Shadow_Size(const void* v1, const void* v2)
{
  CStatObjInst* p1 = *((CStatObjInst**)(v1));
  CStatObjInst* p2 = *((CStatObjInst**)(v2));

  if(p1->m_fScale > p2->m_fScale)
    return -1;
  else if(p1->m_fScale < p2->m_fScale)
    return 1;

  return 0;
}

static int __cdecl CObjManager_Cmp_EntSize(const void* v1, const void* v2)
{
  IEntityRender* p1 = *((IEntityRender**)(v1));
  IEntityRender* p2 = *((IEntityRender**)(v2));

  if(p1->GetRenderRadius() > p2->GetRenderRadius())
    return -1;
  else if(p1->GetRenderRadius() < p2->GetRenderRadius())
    return 1;

  return (p1>p2) ? 1 : -1;
}

void CObjManager::DrawAllShadowsOnTheGroundInSector(list2<IEntityRender*> * pEntList)
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw entity shadows
///////////////////////////////////////////////////////////////////////////////////////////////////////////
  GetRenderer()->ResetToDefault();
  GetRenderer()->ClearDepthBuffer();
  GetRenderer()->ClearColorBuffer(Vec3d(1,1,1));   
	GetRenderer()->SetClearColor(Vec3d(1,1,1));

  // sort by size to draw small shadows last or skip them
  if(pEntList && pEntList->Count())
    qsort(&(*pEntList)[0], (*pEntList).Count(), sizeof((*pEntList)[0]), CObjManager_Cmp_EntSize);
	
  GetRenderer()->EF_StartEf();  
  int nRealCout=0;
  for( int i=0; i<pEntList->Count(); i++ )
  {
    IEntityRender * pEnt = (*pEntList)[i];

		if(!(pEnt->GetRndFlags() & ERF_HIDDEN))
    if(pEnt->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP)
    if(!pEnt->GetEntityVisArea())
    {
      assert(m_nRenderStackLevel == 0);
      pEnt->SetDrawFrame(-100,m_nRenderStackLevel);

			int nFlags = pEnt->GetRndFlags();
      pEnt->SetRndFlags(nFlags|ERF_CASTSHADOWMAPS|ERF_RECVSHADOWMAPS);
			pEnt->SetRndFlags(ERF_CASTSHADOWINTOLIGHTMAP,false);

			int nRealLightsNum = ((C3DEngine*)Get3DEngine())->GetRealLightsNum();
			assert(nRealLightsNum==1);
      RenderObject(pEnt,0,1,true,GetViewCamera(),NULL,0,0,false,pEnt->GetMaxViewDist());
			RenderEntitiesShadowMapsOnTerrain(true, 0);

      // increase frame id to help shadow map menager in renderer
      unsigned short * pPtr2FrameID = (unsigned short *)GetRenderer()->EF_Query(EFQ_Pointer2FrameID);
      if(pPtr2FrameID)
        (*pPtr2FrameID)++;

      pEnt->SetRndFlags(nFlags);
      
      nRealCout++;
      if((GetRenderer()->GetType() == R_GL_RENDERER) && nRealCout>=64) // maximum number of shadow maps in frame is limited in ogl to MAX_DYNAMIC_SHADOW_MAPS_COUNT
        break;
    }
  }	
	GetRenderer()->EF_EndEf3D(SHDF_SORT);

	// free shadow pass leafbuffers
	for( int i=0; i<pEntList->Count(); i++ )
	{
		IEntityRender * pEnt = (*pEntList)[i];
		IEntityRenderState * pEntRendState = pEnt->m_pEntityRenderState;
		if(pEntRendState && pEntRendState->pShadowMapInfo && pEntRendState->pShadowMapInfo->pShadowMapLeafBuffersList)
		{
			for(int i=0; i<pEntRendState->pShadowMapInfo->pShadowMapLeafBuffersList->Count(); i++)
			{
				if(pEntRendState->pShadowMapInfo->pShadowMapLeafBuffersList->GetAt(i))
				{
					GetRenderer()->DeleteLeafBuffer(pEntRendState->pShadowMapInfo->pShadowMapLeafBuffersList->GetAt(i));
					pEntRendState->pShadowMapInfo->pShadowMapLeafBuffersList->GetAt(i)=0;
				}
			}
			delete pEntRendState->pShadowMapInfo->pShadowMapLeafBuffersList;
			pEntRendState->pShadowMapInfo->pShadowMapLeafBuffersList=0;
		}
	}
}
