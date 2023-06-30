////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjman.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: objects container, streaming, common part for indoor and outdoor sectors
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_sector.h"
#include "cbuffer.h"
#include "3DEngine.h"
#include "meshidx.h"
#include "watervolumes.h"
#include "brush.h"
#include "LMCompStructures.h"

void CBasicArea::SerializeArea(bool bSave)
{
  char szFileName[256]="";                        
  sprintf(szFileName,"visarea_objects_%.1f_%.1f_%.1f_%.1f.cache", m_vBoxMin.x, m_vBoxMin.y, m_vBoxMax.x, m_vBoxMax.y);

  FILE * f = GetPak()->FOpen(Get3DEngine()->GetFilePath(szFileName), bSave ? "wb" : "rb");
  if(!f)
    return;

  if(bSave)
  {
    for(int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
    {
      EERType eType = m_lstEntities[STATIC_ENTITIES].GetAt(i)->GetEntityRenderType();

      if(eType == eERType_Brush || eType == eERType_Vegetation)
      {
        GetPak()->FWrite(&eType,sizeof(eType),1,f);        

//        if(m_lstEntities[STATIC_ENTITIES].GetAt(i)->GetEntityRS())
        {
          m_lstEntities[STATIC_ENTITIES].GetAt(i)->m_pVisArea = 0;
          m_lstEntities[STATIC_ENTITIES].GetAt(i)->m_pSector = 0;
        }

        m_lstEntities[STATIC_ENTITIES].GetAt(i)->Dephysicalize();
        m_lstEntities[STATIC_ENTITIES].GetAt(i)->Dematerialize();

        m_lstEntities[STATIC_ENTITIES].GetAt(i)->Serialize(bSave,GetPak(),f);
      }
    }
    m_lstEntities[STATIC_ENTITIES].Reset();
  }
  else
  {
    assert(m_lstEntities[STATIC_ENTITIES].Count()==0);
    m_lstEntities[STATIC_ENTITIES].Reset();

    m_eSStatus = eSStatus_Ready;

    while(1)
    {
      EERType eType = eERType_Unknown;
      if(GetPak()->FRead(&eType,sizeof(eType),1,f)!=1)
        break;

      IEntityRender * pEntityRender = 0;
      if(eType == eERType_Brush)
        pEntityRender = new CBrush();      
      else if(eType == eERType_Vegetation)
        pEntityRender = new CStatObjInst();      

      if(pEntityRender)
      {
        pEntityRender->Serialize(bSave,GetPak(),f);
        pEntityRender->GetEntityRS() = new IEntityRenderState;
        pEntityRender->Physicalize();
        Get3DEngine()->RegisterEntity(pEntityRender);
      }
    }
  }

  GetPak()->FClose(f);
}

void CBasicArea::UnmakeAreaBrush()
{
	for( int i=0; i<m_lstAreaBrush.Count(); i++ )
		FreeAreaBrush(m_lstAreaBrush[i]);

	// mark all objects as unmerged
	if(m_lstAreaBrush.Count())
	for( int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++ )
	{
		IEntityRender * pEntityRender =	m_lstEntities[STATIC_ENTITIES].GetAt(i);
		pEntityRender->m_dwRndFlags &= ~ERF_MERGED;
		assert(!(pEntityRender->m_dwRndFlags & ERF_MERGED));
	}

	m_lstAreaBrush.Clear();
}

void CBasicArea::DrawEntities( int nFogVolumeID, int nDLightMask,
                              bool bLMapGeneration, const CCamera & EntViewCamera, Vec3d * pvAmbColor, Vec3d * pvDynAmbColor,
                              VolumeInfo * pFogVolume, bool bNotAllInFrustum, float fSectorMinDist,
                              CObjManager * pObjManager, bool bAllowBrushMerging, char*fake, uint nStatics)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	if(!GetCVars()->e_objects)
		return;

  if(GetCVars()->e_stream_areas && GetCVars()->e_stream_for_visuals && CStatObj::m_fStreamingTimePerFrame<CGF_STREAMING_MAX_TIME_PER_FRAME)
  {
    m_nLastUsedFrameId = GetFrameID();

    if(m_eSStatus != eSStatus_Ready)
    {
      CStatObj::m_fStreamingTimePerFrame -= GetTimer()->GetAsyncCurTime();
      SerializeArea(false);
      CStatObj::m_fStreamingTimePerFrame += GetTimer()->GetAsyncCurTime();
    }
  }

	// apply zoom factor
	fSectorMinDist *= pObjManager->m_fZoomFactor;

	// make lmask for vegetations
	int nDLightMaskNoSun = nDLightMask;
	list2<CDLight> * pSources = ((C3DEngine*)m_p3DEngine)->GetDynamicLightSources();
	for(int i=0; i<pSources->Count(); i++)
	{
		CDLight * pDynLight = pSources->Get(i);
		assert(pDynLight->m_Id == i || pDynLight->m_Id == -1);
		if(pDynLight->m_Flags & DLF_SUN)
		{
			nDLightMaskNoSun &= ~(1<<pDynLight->m_Id);
			break;
		}
	}

	if(nStatics && m_StaticEntitiesSorted && GetCVars()->e_optimized_render_object && 
		!bLMapGeneration && pObjManager->m_fZoomFactor >= 0.99f)
	{ // render statics compiled
		const Vec3d vCamPos = EntViewCamera.GetPos();

		const Plane PlaneR = *EntViewCamera.GetFrustumPlane(FR_PLANE_RIGHT);
		const Plane PlaneL = *EntViewCamera.GetFrustumPlane(FR_PLANE_LEFT);

		static list2<IEntityRenderInfo*> TmpEntList; TmpEntList.Clear();
		list2<struct IEntityRender*> & SrcEntList = m_lstEntities[STATIC_ENTITIES];

		if(GetCVars()->e_vegetation)
		{	// fill simple vegetations
//		FRAME_PROFILER( "*fill simple vegetations", GetSystem(), PROFILE_3DENGINE );
			for( int i=0; i<m_lstStatEntInfoVegetNoCastersNoVolFog.Count(); i++ )
			{
				IEntityRenderInfo & inf = m_lstStatEntInfoVegetNoCastersNoVolFog[i];

				if(fSectorMinDist >= inf.m_fWSMaxViewDist)
					break;

				// check max view distance sq
				const Vec3d vCamPos = EntViewCamera.GetPos();
				const float dx = vCamPos.x-inf.m_vWSCenter.x;
				const float dy = vCamPos.y-inf.m_vWSCenter.y;
				const float fEntDistanceSQ = (dx*dx+dy*dy); // must be 2d for sprites
				if(fEntDistanceSQ > inf.m_fWSMaxViewDistSQ)
					continue;

				// early sphere test agains left and right camera planes
				if( bNotAllInFrustum && 
						PlaneR.DistFromPlane(inf.m_vWSCenter) > inf.m_fWSRadius ||
						PlaneR.DistFromPlane(inf.m_vWSCenter) > inf.m_fWSRadius )
					continue;

				// get view distance
				inf.m_fEntDistance = cry_sqrtf(fEntDistanceSQ);
				assert(inf.m_fEntDistance>=0 && _finite(inf.m_fEntDistance));
				assert(inf.m_fEntDistance <= inf.m_fWSMaxViewDist);

				TmpEntList.Add(&inf);
			}

			// render simple vegetations
//		FRAME_PROFILER( "*render simple vegetations", GetSystem(), PROFILE_3DENGINE );
			for( int i=0; i<TmpEntList.Count(); i++ )
			{
				IEntityRender * pEntityRender =	TmpEntList.GetAt(i)->m_pEntityRender;
				
				if (i+1 < TmpEntList.Count())
				{ // prefech next element
					IEntityRender * pNext = TmpEntList.GetAt(i+1)->m_pEntityRender;
					cryPrefetchT0SSE(pNext);
				}

				assert(fSectorMinDist < pEntityRender->m_fWSMaxViewDist);

				pObjManager->RenderObjectVegetationNonCastersNoFogVolume( pEntityRender, 
					nDLightMaskNoSun, EntViewCamera, bNotAllInFrustum,
					pEntityRender->m_fWSMaxViewDist, TmpEntList.GetAt(i));
			}
		}

		TmpEntList.Clear();

		if(GetCVars()->e_vegetation || GetCVars()->e_brushes)
		{ // fill complex objects
//		FRAME_PROFILER( "*fill complex objects", GetSystem(), PROFILE_3DENGINE );
			for( int i=0; i<m_lstStatEntInfoOthers.Count(); i++ )
			{
				IEntityRenderInfo & inf = m_lstStatEntInfoOthers[i];

				if(fSectorMinDist >= inf.m_fWSMaxViewDist)
					break;

				// check max view distance sq
				const Vec3d vCamPos = EntViewCamera.GetPos();
				const float dx = vCamPos.x-inf.m_vWSCenter.x;
				const float dy = vCamPos.y-inf.m_vWSCenter.y;
				const float fEntDistanceSQ = (dx*dx+dy*dy); // must be 2d for sprites
				if(fEntDistanceSQ > inf.m_fWSMaxViewDistSQ)
					continue;

				// early sphere test agains left and right camera planes
				if( bNotAllInFrustum &&
						PlaneR.DistFromPlane(inf.m_vWSCenter) > inf.m_fWSRadius+TERRAIN_SECTORS_MAX_OVERLAPPING ||
						PlaneR.DistFromPlane(inf.m_vWSCenter) > inf.m_fWSRadius+TERRAIN_SECTORS_MAX_OVERLAPPING )
					continue;

				// get view distance
				inf.m_fEntDistance = cry_sqrtf(fEntDistanceSQ);
				assert(inf.m_fEntDistance>=0 && _finite(inf.m_fEntDistance));
				assert(inf.m_fEntDistance <= inf.m_fWSMaxViewDist);

				TmpEntList.Add(&inf);
			}

			// render complex objects
//		FRAME_PROFILER( "*render complex objects", GetSystem(), PROFILE_3DENGINE );
			for( int i=0; i<TmpEntList.Count(); i++ )
			{
				IEntityRender * pEntityRender =	TmpEntList.GetAt(i)->m_pEntityRender;

				if (i+1 < TmpEntList.Count())
				{ // prefech next element
					IEntityRender * pNext = TmpEntList.GetAt(i+1)->m_pEntityRender;
					cryPrefetchT0SSE(pNext);
				}

				assert(fSectorMinDist < pEntityRender->m_fWSMaxViewDist);

				pObjManager->RenderObject( pEntityRender, nFogVolumeID, 
					nDLightMask, bLMapGeneration, EntViewCamera, pvAmbColor, pvDynAmbColor, pFogVolume, bNotAllInFrustum,
					pEntityRender->m_fWSMaxViewDist, TmpEntList.GetAt(i));
			}
		}
	}
	else if(nStatics && m_StaticEntitiesSorted && !bLMapGeneration)
	{

#ifdef VEGETATION_MEM_STATS
		static int64 t0=0,t1=0,t2=0;
#endif // VEGETATION_MEM_STATS

		list2<struct IEntityRender*> & EntList = m_lstEntities[STATIC_ENTITIES];
		for( int i=0; i<EntList.Count(); i++ )
		{
			IEntityRender * pEntityRender =	EntList.GetAt(i);

			if (i+1 < EntList.Count())
			{ // prefech next element
				IEntityRender * pNext = EntList.GetAt(i+1);
				cryPrefetchT0SSE(pNext);
			}

#ifdef VEGETATION_MEM_STATS
			if (i+1 < EntList.Count())
			{
				IEntityRender * pNext = EntList.GetAt(i+1);
				IEntityRender * pThis = EntList.GetAt(i);

				if(pNext>pThis)
				{
					if(((int64)pNext-(int64)pThis)<=128)
						t0++;
					else
						t1++;
				}
				else
					t2++;
			}
#endif // VEGETATION_MEM_STATS

			if(fSectorMinDist > pEntityRender->m_fWSMaxViewDist)
				break;

			pObjManager->RenderObject( pEntityRender, nFogVolumeID, 
				nDLightMask, bLMapGeneration, EntViewCamera, pvAmbColor, pvDynAmbColor, pFogVolume, bNotAllInFrustum,
				pEntityRender->m_fWSMaxViewDist);
		}

#ifdef VEGETATION_MEM_STATS
		static int fr=0;
		if((GetFrameID()&63)==0 && fr!=GetFrameID())
		{
			fr=GetFrameID();
			if(t0+t1+t2)
				GetLog()->Log(
				"<128 = %.1f, >128 = %.1f, NEG = %.1f",
				0.1f*float((t0*1000/(t0+t1+t2))),
				0.1f*float((t1*1000/(t0+t1+t2))), 
				0.1f*float((t2*1000/(t0+t1+t2))));
		}
#endif // VEGETATION_MEM_STATS
	}
	else if(nStatics || GetCVars()->e_entities)
	{ // render dynamics or statics uncompiled
		assert(!m_StaticEntitiesSorted || !nStatics);
		list2<struct IEntityRender*> & EntList = m_lstEntities[nStatics];
		for( int i=0; i<EntList.Count(); i++ )
		{
			IEntityRender * pEntityRender =	EntList.GetAt(i);
			pEntityRender->m_fWSMaxViewDist = pEntityRender->GetMaxViewDist();
			if(fSectorMinDist > pEntityRender->m_fWSMaxViewDist)
				continue;

			pObjManager->RenderObject( pEntityRender, nFogVolumeID, 
				nDLightMask, bLMapGeneration, EntViewCamera, pvAmbColor, pvDynAmbColor, pFogVolume, bNotAllInFrustum,
				pEntityRender->m_fWSMaxViewDist);
		}
	}
}

void CBasicArea::Unload(bool bUnloadOnlyVegetations, const Vec3d & vVegetPos)
{
  if(bUnloadOnlyVegetations)
  {
    for(int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
    {
      EERType eType = m_lstEntities[STATIC_ENTITIES].GetAt(i)->GetEntityRenderType();
      if(eType == eERType_Vegetation)
      {
        Vec3d vEntPos = m_lstEntities[STATIC_ENTITIES].GetAt(i)->GetPos();
        float fDist = (vVegetPos==Vec3d(0,0,0)) ? 0 : GetDist2D(vVegetPos.x,vVegetPos.y,vEntPos.x,vEntPos.y);
        if(fDist<0.01f)
        {
          int nCountBefore = m_lstEntities[STATIC_ENTITIES].Count();
          delete m_lstEntities[STATIC_ENTITIES].GetAt(i); // will also remove it from this list
          assert(m_lstEntities[STATIC_ENTITIES].Count() == (nCountBefore-1));
          i--;
        }
      }
    }
  }
  else
  {
    while(m_lstEntities[STATIC_ENTITIES].Count())
    {
      EERType eType = m_lstEntities[STATIC_ENTITIES].GetAt(0)->GetEntityRenderType();
      assert(eType == eERType_Brush || eType == eERType_Vegetation);
      int nCountBefore = m_lstEntities[STATIC_ENTITIES].Count();
			
			if(eType != eERType_Vegetation)
				Get3DEngine()->ReleaseObject(m_lstEntities[STATIC_ENTITIES].GetAt(0)->GetEntityStatObj(0));
			m_lstEntities[STATIC_ENTITIES].GetAt(0)->SetEntityStatObj(0,0);

			IEntityRender * pEntityRender =	m_lstEntities[STATIC_ENTITIES].GetAt(0); // will also remove it from this list
      delete pEntityRender; // will also remove it from this list

      assert(m_lstEntities[STATIC_ENTITIES].Count() == (nCountBefore-1));
    }
    assert(m_lstEntities[STATIC_ENTITIES].Count()==0);
    m_eSStatus = eSStatus_Unloaded;
  }

	UnmakeAreaBrush();
}

bool CBasicArea::CheckUnload()
{
  if(CStatObj::m_fStreamingTimePerFrame>=CGF_STREAMING_MAX_TIME_PER_FRAME)
    return (m_eSStatus == eSStatus_Ready);

  CStatObj::m_fStreamingTimePerFrame -= GetTimer()->GetAsyncCurTime();

  if(m_nLastUsedFrameId < GetFrameID() - 100)// && m_lstEntities[STATIC_ENTITIES].Count())
  { // unload
    Unload();
  }

  CStatObj::m_fStreamingTimePerFrame += GetTimer()->GetAsyncCurTime();

  return (m_eSStatus == eSStatus_Ready);
}

void CBasicArea::CheckPhysicalized()
{
  if(m_eSStatus != eSStatus_Ready)
    SerializeArea(false);

  for(int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
    m_lstEntities[STATIC_ENTITIES][i]->CheckPhysicalized();

  m_nLastUsedFrameId = GetFrameID();
}

void CBasicArea::PreloadResources(Vec3d vPrevPortalPos, float fPrevPortalDistance)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	int nFrameId = GetFrameID();
	for(int nStatic=0; nStatic<2; nStatic++)
	for( int i=0; i<m_lstEntities[nStatic].Count() && GetCurTimeSec()<(m_fPreloadStartTime+0.010f); i++ )
	{
		if((nFrameId%8) == (i%8))
			m_lstEntities[nStatic].GetAt(i)->PreloadInstanceResources(vPrevPortalPos, fPrevPortalDistance, 1.f);	
	}
}

void CBasicArea::UnregisterDynamicEntities()
{
	while(m_lstEntities[DYNAMIC_ENTITIES].Count())
	{
		EERType eType = m_lstEntities[DYNAMIC_ENTITIES].GetAt(0)->GetEntityRenderType();
		assert(eType != eERType_Brush && eType != eERType_Vegetation);
		int nCountBefore = m_lstEntities[DYNAMIC_ENTITIES].Count();
		//		delete m_lstEntities[DYNAMIC_ENTITIES].GetAt(0); // will also remove it from this list
		Get3DEngine()->UnRegisterEntity(m_lstEntities[DYNAMIC_ENTITIES].GetAt(0));
		assert(m_lstEntities[DYNAMIC_ENTITIES].Count() == (nCountBefore-1));
	}
	assert(m_lstEntities[DYNAMIC_ENTITIES].Count()==0);
}

int __cdecl CObjManager__Cmp_EntTmpDistance(const void* v1, const void* v2);

void CBasicArea::SortStaticInstancesBySize(VolumeInfo * pFogVolume)
{
	// sort lists instances in sector by size ( for rendering speed up )
	for( int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
	{
		m_lstEntities[STATIC_ENTITIES][i]->GetEntityRS()->fTmpDistance = 
			-m_lstEntities[STATIC_ENTITIES][i]->GetMaxViewDist();

		m_lstEntities[STATIC_ENTITIES][i]->m_fWSMaxViewDist = m_lstEntities[STATIC_ENTITIES][i]->GetMaxViewDist();
	}

	// sort
	if(m_lstEntities[STATIC_ENTITIES].Count())
		qsort(m_lstEntities[STATIC_ENTITIES].GetElements(), m_lstEntities[STATIC_ENTITIES].Count(), 
		sizeof(m_lstEntities[STATIC_ENTITIES][0]), CObjManager__Cmp_EntTmpDistance);

	m_lstStaticShadowMapCasters.Clear();
	for( int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
	{
		IEntityRender * pEntityRender =	m_lstEntities[STATIC_ENTITIES][i];
		if(pEntityRender->GetRndFlags()&ERF_CASTSHADOWMAPS)
			m_lstStaticShadowMapCasters.Add(pEntityRender);
	}

	m_lstStatEntInfoVegetNoCastersNoVolFog.Clear();
	m_lstStatEntInfoOthers.Clear();
	for( int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
	{
		IEntityRender * pEntityRender =	m_lstEntities[STATIC_ENTITIES][i];
		IEntityRenderInfo inf(pEntityRender);

		bool bInFogVolume = false;
		if(pFogVolume)
		{ // fog is set only for outdoors
			Vec3d vBoxMin,vBoxMax;
			assert(pEntityRender->m_pSector);
			pEntityRender->GetBBox(vBoxMin,vBoxMax);
			if(pFogVolume->IntersectBBox(vBoxMin,vBoxMax))
				bInFogVolume = true;
		}

		if(	!(pEntityRender->GetRndFlags() & (ERF_CASTSHADOWMAPS|ERF_CASTSHADOWVOLUME|ERF_RECVSHADOWMAPS|ERF_SELFSHADOW)) && 
				pEntityRender->GetEntityRenderType() ==	eERType_Vegetation && 
				!bInFogVolume)
			m_lstStatEntInfoVegetNoCastersNoVolFog.Add(inf);
		else
			m_lstStatEntInfoOthers.Add(inf);
	}

	m_StaticEntitiesSorted = true;

	// swap to disk
	if(GetCVars()->e_stream_areas)
		SerializeArea(true);
}

int __cdecl CBasicArea__Cmp_MatChunks(const void* v1, const void* v2)
{
	CMatInfo * pMat1 = (CMatInfo*)v1;
	CMatInfo * pMat2 = (CMatInfo*)v2;

	// shader
	if(pMat1->shaderItem.m_pShader->GetTemplate(-1) > pMat2->shaderItem.m_pShader->GetTemplate(-1))
		return  1;
	else if(pMat1->shaderItem.m_pShader->GetTemplate(-1) < pMat2->shaderItem.m_pShader->GetTemplate(-1))
		return -1;

	// shader resources
	if(pMat1->shaderItem.m_pShaderResources > pMat2->shaderItem.m_pShaderResources)
		return  1;
	else if(pMat1->shaderItem.m_pShaderResources < pMat2->shaderItem.m_pShaderResources)
		return -1;

	// lm tex id
	if(pMat1->m_Id > pMat2->m_Id)
		return  1;
	else if(pMat1->m_Id < pMat2->m_Id)
		return -1;

	return 0;

/*
	const char * pName1 = ((*(((*(pMat1)).shaderItem).m_pShaderResources)).m_Textures)[0] ? (*(((*(((*(pMat1)).shaderItem).m_pShaderResources)).m_Textures)[0])).m_Name.c_str() : "";
	const char * pName2 = ((*(((*(pMat2)).shaderItem).m_pShaderResources)).m_Textures)[0] ? (*(((*(((*(pMat2)).shaderItem).m_pShaderResources)).m_Textures)[0])).m_Name.c_str() : "";

	return strcmp(pName1,pName2);*/
}

struct LMTexCoord
{
	float s,t;
};

void CBasicArea::MakeAreaBrush()
{
	const Vec3d vCamPos = GetViewCamera().GetPos();

	int nEntityId=0;
	int nNextStartId=-1;
	while(nEntityId<m_lstEntities[STATIC_ENTITIES].Count() || nNextStartId>=0)
	{
		static list2<struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F> lstVerts;lstVerts.Clear();
		static list2<LMTexCoord> lstLMTexCoords; lstLMTexCoords.Clear();
		static list2<ushort> lstIndices; lstIndices.Clear();
		static list2<ushort> lstIndicesSorted; lstIndicesSorted.Clear();
		static list2<CMatInfo> lstChunks; lstChunks.Clear();
		static list2<CMatInfo> lstChunksMerged; lstChunksMerged.Clear();
		static list2<SPipTangents> lstTangBasises; lstTangBasises.Clear();

		Vec3d vBoxMax(-10000,-10000,-10000);
		Vec3d vBoxMin( 10000, 10000, 10000);

		int nLMTexId=-1;
		int nHDRLMTexId=-1;
		int nLMDirTexId=-1;
		int nCurVertsNum=0;
		
		if(nNextStartId>=0)
			nEntityId = nNextStartId;
		nNextStartId=-1;

		for( ; nEntityId<m_lstEntities[STATIC_ENTITIES].Count(); nEntityId++)
		{
			IEntityRender * pEntityRender	= m_lstEntities[STATIC_ENTITIES][nEntityId];

			if(	pEntityRender->m_dwRndFlags & ERF_MERGED ||
					pEntityRender->m_dwRndFlags & ERF_CASTSHADOWVOLUME ||
					pEntityRender->m_dwRndFlags & ERF_CASTSHADOWMAPS)
				continue;

//			Vec3d vCenter = (pEntityRender->m_vWSBoxMin+pEntityRender->m_vWSBoxMax)*0.5f;
//			float fEntDistance = GetDist2D( vCamPos.x, vCamPos.y, vCenter.x, vCenter.y );
//			if(fEntDistance > pEntityRender->GetMaxViewDist())
//				continue;

//			if(fEntDistance > GetCVars()->e_area_merging_distance)
	//			continue;

			if(nLMTexId<0)
			{
				nLMTexId = pEntityRender->GetLightmap(0) ? pEntityRender->GetLightmap(0)->GetColorLerpTex() : 0;
				nHDRLMTexId = pEntityRender->GetLightmap(0) ? pEntityRender->GetLightmap(0)->GetHDRColorLerpTex() : 0;
				nLMDirTexId = pEntityRender->GetLightmap(0) ? pEntityRender->GetLightmap(0)->GetDomDirectionTex() : 0;
			}
			else
			{
				if(nLMTexId != (pEntityRender->GetLightmap(0) ? pEntityRender->GetLightmap(0)->GetColorLerpTex() : 0) ||
					nLMDirTexId != (pEntityRender->GetLightmap(0) ? pEntityRender->GetLightmap(0)->GetDomDirectionTex() : 0) ||
					nHDRLMTexId != (pEntityRender->GetLightmap(0) ? pEntityRender->GetLightmap(0)->GetHDRColorLerpTex() : 0))
				{
					if(nNextStartId<0)
						nNextStartId = nEntityId;
					continue;
				}
			}

			Matrix44 mat;
			mat.SetIdentity();
			mat*=0;
			IStatObj * pStatObj = pEntityRender->GetEntityStatObj(0,&mat);
			if(!pStatObj)
				continue;

			EERType eType = pEntityRender->GetEntityRenderType();
			if(eType == eERType_Vegetation)
			{
				Matrix33diag diag	=	Vec3(pEntityRender->GetScale(),pEntityRender->GetScale(),pEntityRender->GetScale());	//use diag-matrix for scaling
				mathCalcMatrix(mat, pEntityRender->GetPos(), Vec3d(0,0,0), 
					Vec3d(pEntityRender->GetScale(),pEntityRender->GetScale(),pEntityRender->GetScale()),
					Cry3DEngineBase::m_CpuFlags);
			}

			if(!CBrush::IsMatrixValid(mat))
				continue;

			CLeafBuffer * pLMLB = pEntityRender->GetLightmapTexCoord(0);

			CLeafBuffer * pLB = pStatObj->GetLeafBuffer();
			if(!pLB->m_SecVertCount)
				continue;

			if(nCurVertsNum + pLB->m_SecVertCount>65000)
				break;
 
			int nIndCount=0;
			pLB->GetIndices(&nIndCount);
			if(nIndCount > GetCVars()->e_area_merging_max_tris_in_input_brush*3)
				continue;

			int nInitVertCout = lstVerts.Count();
			for(int m=0; m<pLB->m_pMats->Count(); m++)
			{
				CMatInfo newMatInfo = *pLB->m_pMats->Get(m);

				if(GetCVars()->e_materials)
				{ // Override default material
					CMatInfo * pCustMat = (CMatInfo *)pEntityRender->GetMaterial();
					if (pCustMat)
					{
						int nMatId = newMatInfo.m_nCGFMaterialID;
						if(nMatId<0)
							continue;

						if (nMatId == 0)
							pCustMat = (CMatInfo*)pCustMat;
						else if (nMatId-1 < pCustMat->GetSubMtlCount())
							pCustMat = (CMatInfo*)pCustMat->GetSubMtl(nMatId-1);

						newMatInfo.shaderItem = pCustMat->shaderItem;
					}
				}

				// copy indices
				for(int i=newMatInfo.nFirstIndexId; i<newMatInfo.nFirstIndexId+newMatInfo.nNumIndices; i++)
					lstIndices.Add(pLB->GetIndices(0)[i]+nInitVertCout);
				newMatInfo.nFirstIndexId = lstIndices.Count() - newMatInfo.nNumIndices;

				// copy verts
				int nPosStride=0;
				const byte * pPos = pLB->GetPosPtr(nPosStride,0,true);
				int nTexStride=0;
				const byte * pTex = pLB->GetUVPtr(nTexStride,0,true);

				// get tengent basis
				int nTangStride=0;
				const byte * pTang = pLB->GetTangentPtr(nTangStride,0,true);
				int nTnormStride=0;
				const byte * pTNorm = pLB->GetTNormalPtr(nTnormStride,0,true);
				int nBNormStride=0;
				const byte * pBNorm = pLB->GetBinormalPtr(nBNormStride,0,true);
				int nColorStride=0;
				const byte * pColor = pLB->GetColorPtr(nColorStride,0,true);

				// get LM TexCoords
				int nLMStride=0;
				const byte * pLMTexCoords = pLMLB ? pLMLB->GetPosPtr(nLMStride,0,true) : 0;

				for(int v=newMatInfo.nFirstVertId; v<newMatInfo.nFirstVertId+newMatInfo.nNumVerts; v++)
				{
					struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F vert;

					// set pos
					Vec3d vPos = *(Vec3d*)&pPos[nPosStride*v];				
					vPos = mat.TransformPointOLD(vPos);
          vert.xyz = vPos;

          // set uv
          float * pUV = (float*)&pTex[nTexStride*v];				
          vert.st[0] = pUV[0];
          vert.st[1] = pUV[1];

          vert.color.dcolor = *(DWORD*)&pColor[nColorStride*v+0];

					// calc bbox
					vBoxMin.CheckMin(vPos);
					vBoxMax.CheckMax(vPos);

					lstVerts.Add(vert);

					// add tbasis
					SPipTangents basis;
					basis.m_Tangent = mat.TransformVectorOLD(*(Vec3d*)&pTang[nTangStride*v]);
					basis.m_TNormal = mat.TransformVectorOLD(*(Vec3d*)&pTNorm[nTnormStride*v]);
					basis.m_Binormal= mat.TransformVectorOLD(*(Vec3d*)&pBNorm[nBNormStride*v]);
					lstTangBasises.Add(basis);

					// add LM texcoords
					LMTexCoord vLMTC;
					if(pLMTexCoords)
						vLMTC = *(LMTexCoord*)&pLMTexCoords[nLMStride*v];
					else
						vLMTC.s = vLMTC.t = 0;

					lstLMTexCoords.Add(vLMTC);
				}

				// set vert range
				newMatInfo.nFirstVertId = lstVerts.Count() - newMatInfo.nNumVerts;

				newMatInfo.pRE = 0;
				newMatInfo.m_Id = pEntityRender->GetLightmap(0) ? pEntityRender->GetLightmap(0)->GetColorLerpTex() : 0;

				if(newMatInfo.nNumIndices)
				{
					lstChunks.Add(newMatInfo);
				}
				else
					assert(!newMatInfo.nNumVerts);
			}
			nCurVertsNum += pLB->m_SecVertCount;
			
			pEntityRender->m_dwRndFlags |= ERF_MERGED;
		}

		CBrush * pAreaBrush = 0;
		if(!lstVerts.Count())
		{ // make empty brush - no geometry in sector
			pAreaBrush = new CBrush;
			m_lstAreaBrush.Add(pAreaBrush);
			return;
		}

		// sort
		if(lstChunks.Count())
			qsort(lstChunks.GetElements(), lstChunks.Count(), 
			sizeof(lstChunks[0]), CBasicArea__Cmp_MatChunks);

		// merge chunks
		for(int nChunk=0; nChunk<lstChunks.Count(); nChunk++)
		{
			if(!nChunk || CBasicArea__Cmp_MatChunks(&lstChunks[nChunk], &lstChunks[nChunk-1]))
			{ // not equal materials - add new chunk
				lstChunksMerged.Add(lstChunks[nChunk]);
				lstChunksMerged.Last().nFirstIndexId = lstIndicesSorted.Count();
				lstChunksMerged.Last().nNumIndices = 0;

				lstChunksMerged.Last().nFirstVertId = 0;
				lstChunksMerged.Last().nNumVerts = lstVerts.Count();
			}

			// add indices
			for(int nId=lstChunks[nChunk].nFirstIndexId; nId<lstChunks[nChunk].nFirstIndexId+lstChunks[nChunk].nNumIndices; nId++)
				lstIndicesSorted.Add(lstIndices[nId]);

			// update start/stop pos
			lstChunksMerged.Last().nNumIndices += lstChunks[nChunk].nNumIndices;
		}

		lstChunks = lstChunksMerged;
		lstIndices = lstIndicesSorted;

		// make leaf buffer
		CLeafBuffer * pAreaLB = GetRenderer()->CreateLeafBufferInitialized(
			lstVerts.GetElements(), lstVerts.Count(), VERTEX_FORMAT_P3F_COL4UB_TEX2F, 
			lstIndices.GetElements(), lstIndices.Count(), R_PRIMV_TRIANGLES,
			"AreaLB", eBT_Static, lstChunks.Count());

		pAreaLB->UpdateTangBuffer(lstTangBasises.GetElements());

		for(int i=0; i<lstChunks.Count(); i++)
		{
			pAreaLB->SetChunk(lstChunks[i].GetShaderItem().m_pShader,
				lstChunks[i].nFirstVertId, lstChunks[i].nNumVerts,
				lstChunks[i].nFirstIndexId, lstChunks[i].nNumIndices, i, true);

			assert(lstChunks[i].GetShaderItem().m_pShaderResources);
			assert(lstChunks[i].GetShaderItem().m_pShader);
			pAreaLB->m_pMats->Get(i)->shaderItem = lstChunks[i].GetShaderItem();
			
			pAreaLB->m_pMats->Get(i)->shaderItem.m_pShader->AddRef();
			pAreaLB->m_pMats->Get(i)->shaderItem.m_pShaderResources->AddRef();
		}

		// make statobj
		CStatObj * pAreaStatObj = new CStatObj();
		pAreaStatObj->m_nLoadedTrisCount = lstIndices.Count()/3;
		pAreaStatObj->SetLeafBuffer(pAreaLB);
		pAreaStatObj->SetBBoxMin(vBoxMin);
		pAreaStatObj->SetBBoxMax(vBoxMax);
		pAreaStatObj->RegisterUser();

		// make brush
		pAreaBrush = new CBrush();

	//	if(m_pAreaBrush == (CBrush*)0x0e968358)
		//	int b=0;

		Matrix44 mat;
		mat.SetIdentity();
		pAreaBrush->SetEntityStatObj(0,pAreaStatObj,&mat);
		pAreaBrush->m_vWSBoxMin = vBoxMin;
		pAreaBrush->m_vWSBoxMax = vBoxMax;
		pAreaBrush->m_fWSRadius = vBoxMin.GetDistance(vBoxMax)*0.5f;

		// Make leafbuffer and fill it with texture coordinates
		if(nLMTexId && (nLMDirTexId || (GetCVars()->e_light_maps_quality==0)))
		{
			RenderLMData * pLMData = new RenderLMData(GetRenderer(), nLMTexId, nHDRLMTexId, nLMDirTexId);
			pAreaBrush->SetLightmap(pLMData, (float*)lstLMTexCoords.GetElements(), lstLMTexCoords.Count(), 0);
			pLMData->AddRef();
			pAreaBrush->SetRndFlags(ERF_USELIGHTMAPS,true);
		}

		Get3DEngine()->UnRegisterEntity(pAreaBrush);
		Get3DEngine()->RegisterEntity(pAreaBrush);

		// find distance to the camera
		const Vec3d vCamPos = GetViewCamera().GetPos();
		Vec3d vCenter = (pAreaBrush->m_vWSBoxMin+pAreaBrush->m_vWSBoxMax)*0.5f;
		float fEntDistance = GetDist2D( vCamPos.x, vCamPos.y, vCenter.x, vCenter.y );

		assert(fEntDistance>=0);
		assert(_finite(fEntDistance));

		m_lstAreaBrush.Add(pAreaBrush);
	}
}

void CBasicArea::FreeAreaBrush(CBrush * pAreaBrush)
{
	if(!pAreaBrush)
		return;

	Get3DEngine()->UnRegisterEntity(pAreaBrush);

//	if(m_pAreaBrush == (CBrush*)0x0e968358)
	//	int b=0;

	CStatObj * pAreaStatObj = (CStatObj *)pAreaBrush->GetEntityStatObj(0);
	if(pAreaStatObj)
	{
		pAreaBrush->SetEntityStatObj(0,0);
		pAreaStatObj->UnregisterUser();
		CLeafBuffer * pAreaLB = pAreaStatObj->GetLeafBuffer();
		pAreaStatObj->SetLeafBuffer(0);
		
		GetRenderer()->DeleteLeafBuffer(pAreaLB);
		delete pAreaStatObj;
	}
	delete pAreaBrush;
	pAreaBrush=0;
}

CBasicArea::~CBasicArea()
{
	for( int i=0; i<m_lstAreaBrush.Count(); i++ )
		FreeAreaBrush(m_lstAreaBrush[i]);
	m_lstAreaBrush.Clear();
}


int CBasicArea::GetLastStaticElementIdWithInMaxViewDist(float fMaxViewDist)
{ // use binary search to find range of elements visible on fMaxViewDist
	int nCount = m_lstEntities[STATIC_ENTITIES].Count();
	int nCurrId = m_lstEntities[STATIC_ENTITIES].Count()/2;
	for(int nJump=2; (nCount>>nJump); nJump++)
	{
		IEntityRender * pCaster = m_lstEntities[STATIC_ENTITIES][nCurrId];
		if(pCaster->m_fWSMaxViewDist<fMaxViewDist)
			nCurrId -= nCount>>nJump;
		else
			nCurrId += nCount>>nJump;
	}

	return min(nCurrId+2,m_lstEntities[STATIC_ENTITIES].Count());
}
