////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjmandraw.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Render all entities in the sector together with shadows
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_sector.h"
#include "3dengine.h"
#include "cbuffer.h"
#include "3dengine.h"
#include "cryparticlespawninfo.h"
#include "lightman.h"
#include <utility>

#define MAX_SHADOW_VOLUME_LEN 8.f

void CObjManager::ProcessActiveShadowReceiving(IEntityRender * pEnt, float fEntDistance, CDLight * pLight, bool bLMapGeneration)
{
	ShadowMapLightSource * & pLsource = pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainerPassiveCasters;

	if(!pLsource)
	{ // make lsource
		pLsource = new ShadowMapLightSource;
		ShadowMapFrustum lof;
		lof.pOwner = pEnt;
		pLsource->m_LightFrustums.Add(lof);
	}

	// get needed size of shadow map
	int nTexSize = GetCVars()->e_max_shadow_map_size/2*GetCVars()->e_active_shadow_maps_receving;
	float fDistToTheCamera = pEnt->m_arrfDistance[m_nRenderStackLevel];
	Vec3d vBoxMin,vBoxMax;
	pEnt->GetBBox(vBoxMin,vBoxMax);
	float fCasterRadius = (vBoxMax-vBoxMin).len()*0.5f;
	while( nTexSize*fDistToTheCamera > fCasterRadius*GetCVars()->e_shadow_maps_size_ratio )
		nTexSize /= 2;

	// get obj space light pos
	Vec3d vObjSpaceLightPos; Matrix44 objMatrix;
	IStatObj * pStatObj = pEnt->GetEntityStatObj(0, &objMatrix);
	if(pStatObj)
	{
		objMatrix.Invert44();
		vObjSpaceLightPos = objMatrix.TransformVectorOLD(pLight ? pLight->m_Origin  : m_p3DEngine->GetSunPosition());
	}
	else
		vObjSpaceLightPos = (pLight ? pLight->m_Origin  : m_p3DEngine->GetSunPosition()) - pEnt->GetPos();

	// make shadow map frustum for receiving (include all objects into frustum)
	assert(pLsource->GetShadowMapFrustum());
	if(pLsource->GetShadowMapFrustum())
	{
		if( nTexSize != pLsource->GetShadowMapFrustum()->nTexSize ||
			!IsEquivalent(pLsource->vObjSpaceSrcPos, vObjSpaceLightPos, 0.001f) || 
			pEnt->HasChanged() || pLsource->GetShadowMapFrustum()->depth_tex_id==0 )
		{
			pLsource->GetShadowMapFrustum()->bUpdateRequested = true;
			pLsource->vSrcPos = (pLight ? pLight->m_Origin  : m_p3DEngine->GetSunPosition()) - pEnt->GetPos();
			pLsource->vObjSpaceSrcPos = vObjSpaceLightPos;
			pLsource->fRadius = pLight ? pLight->m_fRadius : 5000000;
			ShadowMapFrustum * lof = MakeEntityShadowFrustum(pLsource->GetShadowMapFrustum(), pLsource, pEnt, EST_DEPTH_BUFFER, SMC_DYNAMICS | SMC_STATICS | SMC_ALLOW_PASSIVE_SHADOWMAP_CASTERS);
			pLsource->GetShadowMapFrustum()->nTexSize = nTexSize;
			lof->dwFlags = SMFF_ACTIVE_SHADOW_MAP;
		}

		pLsource->GetShadowMapFrustum()->nDLightId = pLsource->nDLightId = pLight ? pLight->m_Id : -1;
	}

	// add frustum to the list of shadow casters
	assert(pLsource);
	{
		ShadowMapLightSourceInstance LightSourceInfo;
		LightSourceInfo.m_pLS              = pLsource;
		LightSourceInfo.m_vProjTranslation = pEnt->GetPos();
		LightSourceInfo.m_fProjScale       = 1.f;
		Vec3d vThisEntityPos = pEnt->GetPos();
		LightSourceInfo.m_fDistance        = 0;
		LightSourceInfo.m_pReceiver        = pEnt;
		if(LightSourceInfo.m_pLS->m_LightFrustums.Count() && 
			LightSourceInfo.m_pLS->m_LightFrustums[0].pEntityList &&
			LightSourceInfo.m_pLS->m_LightFrustums[0].pEntityList->Count())
		{
			pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters->Add(LightSourceInfo);
			RequestEntityShadowMapGeneration(pEnt);
		}
	}
}

void CObjManager::RequestEntityShadowMapGeneration(IEntityRender * pEntityRnd)
{
	CCObject * pObj = GetIdentityCCObject();
	pObj->m_pShadowCasters = pEntityRnd->GetShadowMapCasters();

	GetRenderer()->EF_AddEf(0, m_p3DEngine->m_pREShadowMapGenerator,
		m_p3DEngine->m_pSHShadowMapGen, NULL, pObj, 0);

	if(GetCVars()->e_shadow_maps_frustums && pEntityRnd->GetShadowMapFrustumContainer())
		pEntityRnd->GetShadowMapFrustumContainer()->m_LightFrustums.Get(0)->DrawFrustum(GetRenderer(), 
		pEntityRnd->GetPos(), 1.f);

	if(GetCVars()->e_shadow_maps_frustums && pEntityRnd->GetShadowMapFrustumContainerPassiveCasters())
		pEntityRnd->GetShadowMapFrustumContainerPassiveCasters()->m_LightFrustums.Get(0)->DrawFrustum(GetRenderer(), 
		pEntityRnd->GetPos(), 1.f);
}


char BoxSides[0x40*8] = {
	0,0,0,0, 0,0,0,0, //00
		0,4,6,2, 0,0,0,4, //01
		7,5,1,3, 0,0,0,4, //02
		0,0,0,0, 0,0,0,0, //03
		0,1,5,4, 0,0,0,4, //04
		0,1,5,4, 6,2,0,6, //05
		7,5,4,0, 1,3,0,6, //06
		0,0,0,0, 0,0,0,0, //07
		7,3,2,6, 0,0,0,4, //08
		0,4,6,7, 3,2,0,6, //09
		7,5,1,3, 2,6,0,6, //0a
		0,0,0,0, 0,0,0,0, //0b
		0,0,0,0, 0,0,0,0, //0c
		0,0,0,0, 0,0,0,0, //0d
		0,0,0,0, 0,0,0,0, //0e
		0,0,0,0, 0,0,0,0, //0f
		0,2,3,1, 0,0,0,4, //10
		0,4,6,2, 3,1,0,6, //11
		7,5,1,0, 2,3,0,6, //12
		0,0,0,0, 0,0,0,0, //13
		0,2,3,1, 5,4,0,6, //14
		1,5,4,6, 2,3,0,6, //15
		7,5,4,0, 2,3,0,6, //16
		0,0,0,0, 0,0,0,0, //17
		0,2,6,7, 3,1,0,6, //18
		0,4,6,7, 3,1,0,6, //19
		7,5,1,0, 2,6,0,6, //1a
		0,0,0,0, 0,0,0,0, //1b
		0,0,0,0, 0,0,0,0, //1c
		0,0,0,0, 0,0,0,0, //1d
		0,0,0,0, 0,0,0,0, //1e
		0,0,0,0, 0,0,0,0, //1f
		7,6,4,5, 0,0,0,4, //20
		0,4,5,7, 6,2,0,6, //21
		7,6,4,5, 1,3,0,6, //22
		0,0,0,0, 0,0,0,0, //23
		7,6,4,0, 1,5,0,6, //24
		0,1,5,7, 6,2,0,6, //25
		7,6,4,0, 1,3,0,6, //26
		0,0,0,0, 0,0,0,0, //27
		7,3,2,6, 4,5,0,6, //28
		0,4,5,7, 3,2,0,6, //29
		6,4,5,1, 3,2,0,6, //2a
		0,0,0,0, 0,0,0,0, //2b
		0,0,0,0, 0,0,0,0, //2c
		0,0,0,0, 0,0,0,0, //2d
		0,0,0,0, 0,0,0,0, //2e
		0,0,0,0, 0,0,0,0, //2f
		0,0,0,0, 0,0,0,0, //30
		0,0,0,0, 0,0,0,0, //31
		0,0,0,0, 0,0,0,0, //32
		0,0,0,0, 0,0,0,0, //33
		0,0,0,0, 0,0,0,0, //34
		0,0,0,0, 0,0,0,0, //35
		0,0,0,0, 0,0,0,0, //36
		0,0,0,0, 0,0,0,0, //37
		0,0,0,0, 0,0,0,0, //38
		0,0,0,0, 0,0,0,0, //39
		0,0,0,0, 0,0,0,0, //3a
		0,0,0,0, 0,0,0,0, //3b
		0,0,0,0, 0,0,0,0, //3c
		0,0,0,0, 0,0,0,0, //3d
		0,0,0,0, 0,0,0,0, //3e
		0,0,0,0, 0,0,0,0, //3f
};

float CObjManager::GetSortOffset( const Vec3d & vPos, const Vec3d & vCamPos, float fUserWaterLevel )
{
	float fWaterLevel = fUserWaterLevel>WATER_LEVEL_UNKNOWN ? fUserWaterLevel : m_pTerrain->GetWaterLevel();
	if ((0.5f-m_nRenderStackLevel)*(vCamPos.z - fWaterLevel)*(vPos.z - fWaterLevel)>0)
		return -1000000;
	else
		return  1000000;
}


void CObjManager::RenderObjectVegetationNonCastersNoFogVolume( IEntityRender * pEntityRS,uint nDLightMask, 
	const CCamera & EntViewCamera, bool bNotAllInFrustum, float fMaxViewDist, IEntityRenderInfo * pEntInfo)
{
	assert(pEntInfo);

	Vec3d vCenter = pEntInfo->m_vWSCenter;
	float fEntDistance = pEntInfo->m_fEntDistance;

	// do not draw if marked to be not drawn
	unsigned int nRenderFlags = pEntityRS->GetRndFlags();

	// set only x strongest light bits and do frustum test
	Vec3d vLightIntensity(0,0,0);
	Vec3d vBoxMin = pEntityRS->m_vWSBoxMin, vBoxMax = pEntityRS->m_vWSBoxMax;
	float fEntRadius = pEntityRS->m_fWSRadius;
	IRenderer * pRend = GetRenderer();
	CVars * pCVars = GetCVars();

	CDLight * pStrongestLightForTranspGeom = NULL;
	// check only original bbox
	if(bNotAllInFrustum && !EntViewCamera.IsAABBVisibleFast( AABB( vBoxMin, vBoxMax )))
		return;

	// for big objects (registered in sector 00) - get light mask from 00 sector
	if(pEntityRS->m_pSector == m_pTerrain->m_arrSecInfoTable[0][0])
	{
		CSectorInfo * pSectorInfo = m_pTerrain->GetSecInfo(vCenter);
		if(pSectorInfo)
			nDLightMask = pSectorInfo->m_nDynLightMask;
	}

	list2<CDLight> * pSources = m_p3DEngine->GetDynamicLightSources();
	if(nDLightMask==1 && pSources->Count() && pSources->GetAt(0).m_Flags & DLF_SUN)
		pStrongestLightForTranspGeom = pSources->Get(0);
	else if(nDLightMask)
		m_p3DEngine->CheckDistancesToLightSources(nDLightMask, vCenter, fEntRadius, pEntityRS, 8, &pStrongestLightForTranspGeom, 1, &vLightIntensity);

	// check cvars
	assert(pEntityRS->GetEntityRenderType() == eERType_Vegetation);

	// check all possible occlusions for outdoor objects
	if(fEntRadius && pCVars->e_portals!=3)
	{
		// test occlusion of outdoor objects by mountains
		if(m_fZoomFactor && fEntDistance/m_fZoomFactor > 48 && !pEntityRS->m_pVisArea)
			if(IsBoxOccluded(vBoxMin, vBoxMax, fEntDistance/m_fZoomFactor, &pEntityRS->OcclState))
				return;

		// test occl by antiportals
		if(GetVisAreaManager()->IsOccludedByOcclVolumes(vBoxMin,vBoxMax, pEntityRS->m_pVisArea!=NULL))
			return;
	}

	// store for later use (like tree sprites rendering)
	pEntityRS->m_arrfDistance[m_nRenderStackLevel] = fEntDistance;

	// mark as rendered in this frame
	pEntityRS->SetDrawFrame( GetFrameID(), m_nRenderStackLevel );

	// process object particles (rain drops)
	if(GetCVars()->e_rain_amount)
		ProcessEntityParticles(pEntityRS,fEntDistance);

	// set render params
	SRendParams DrawParams;  
//	DrawParams.fSQDistance = fEntDistance*fEntDistance;

	DrawParams.nDLightMask  = nDLightMask;
	DrawParams.nFogVolumeID = 0;
	DrawParams.fDistance = fEntDistance;
	DrawParams.vAmbientColor = m_vOutdoorAmbientColor;
	if(GetCVars()->e_objects_fade_on_distance)
		DrawParams.fAlpha = min(1.f,(1.f - fEntDistance / fMaxViewDist)*6);

	// modulate ambient color by envir color
	Vec3d vWorldColor = Get3DEngine()->GetWorldColor();
	DrawParams.vAmbientColor.x *= vWorldColor.x;
	DrawParams.vAmbientColor.y *= vWorldColor.y;
	DrawParams.vAmbientColor.z *= vWorldColor.z;

	const Vec3d vCamPos = EntViewCamera.GetPos();
	DrawParams.fCustomSortOffset = GetSortOffset(vCenter,vCamPos);

	// ignore ambient color set by artist
	DrawParams.dwFObjFlags = FOB_IGNOREMATERIALAMBIENT;

	if(pRend->EF_GetHeatVision())
		DrawParams.nShaderTemplate = EFT_HEATVISION;

	// draw bbox
	if (pCVars->e_bboxes)			
		GetRenderer()->Draw3dBBox(pEntityRS->m_vWSBoxMin, pEntityRS->m_vWSBoxMax);

	// set light mask for transparent geometry
	if(pStrongestLightForTranspGeom)
	{
		DrawParams.nStrongestDLightMask = 1<<pStrongestLightForTranspGeom->m_Id;
		if(DrawParams.fAlpha<1.f) // set it for entire object if entire object is transparent
			DrawParams.nDLightMask = DrawParams.nStrongestDLightMask & DrawParams.nDLightMask;
	}

	pEntityRS->DrawEntity(DrawParams);
}

void CObjManager::RenderObject( IEntityRender * pEntityRS,
															 int nFogVolumeID, uint nDLightMask, bool bLMapGeneration,
															 const CCamera & EntViewCamera, Vec3d * pvAmbColor, Vec3d * pvDynAmbColor,
															 VolumeInfo * pFogVolume,
															 bool bNotAllInFrustum, float fMaxViewDist, IEntityRenderInfo * pEntInfo)
{
//	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

#ifdef _DEBUG
	const char * szName = pEntityRS->GetName();
	const char * szClassName = pEntityRS->GetEntityClassName();

	if(strstr(szName, "Player"))
	{
		pEntityRS->SetRndFlags(ERF_HIDDEN, !GetCVars()->e_player);
		if(!GetCVars()->e_player)
			return;
	}

	// be sure fps weapon is not rendered this way
	assert(!pEntityRS->GetEntityCharacter(0) || !(pEntityRS->GetEntityCharacter(0)->GetFlags() & CS_FLAG_DRAW_NEAR));

	// is position valid
	if(!_finite(pEntityRS->GetPos().x) || !_finite(pEntityRS->GetPos().y))
	{
		Warning(0,0,"Warning: CObjManager::RenderObject: Entity position undefined: %s", szName);
		return;
	}
#endif // _DEBUG

	float fEntDistance;
	Vec3d vCenter;
	const Vec3d vCamPos = EntViewCamera.GetPos();
	if(pEntInfo)
	{
		vCenter = pEntInfo->m_vWSCenter;
		fEntDistance = pEntInfo->m_fEntDistance;
	}
	else
	{
		// find distance to the camera
		vCenter.x = (pEntityRS->m_vWSBoxMin.x+pEntityRS->m_vWSBoxMax.x)*0.5f;
		vCenter.y = (pEntityRS->m_vWSBoxMin.y+pEntityRS->m_vWSBoxMax.y)*0.5f;
		vCenter.z = (pEntityRS->m_vWSBoxMin.z+pEntityRS->m_vWSBoxMax.z)*0.5f;

		// check max view distance sq
		const float dx = vCamPos.x-vCenter.x;
		const float dy = vCamPos.y-vCenter.y;
		const float fEntDistanceSQ = (dx*dx+dy*dy); // must be 2d for sprites

	#ifdef _DEBUG
		if(fEntDistanceSQ<0 || !_finite(fEntDistanceSQ))
		{
			Warning(0,0,"Warning: CObjManager::RenderObject: Entity bbox undefined: %s, fEntDistanceSQ=%.2f", szName, fEntDistanceSQ);
			return;
		}
	#endif // _DEBUG

		if(fEntDistanceSQ > fMaxViewDist*fMaxViewDist && m_fZoomFactor >= 0.99f && !bLMapGeneration)
			return;

		// check max view distance
		fEntDistance = cry_sqrtf(fEntDistanceSQ)*m_fZoomFactor;
		assert(fEntDistance>=0 && _finite(fEntDistance));
		if(fEntDistance > fMaxViewDist)
			if(!bLMapGeneration)
				return;

		// do not cull objects during lightmap generation
		if(bLMapGeneration)
			fEntDistance = 0;

		// early sphere test agains left and right camera planes
		if( bNotAllInFrustum )
		if( EntViewCamera.GetFrustumPlane(FR_PLANE_RIGHT)->DistFromPlane(vCenter) > (TERRAIN_SECTORS_MAX_OVERLAPPING+pEntityRS->m_fWSRadius) ||
				EntViewCamera.GetFrustumPlane(FR_PLANE_LEFT)->DistFromPlane(vCenter) > (TERRAIN_SECTORS_MAX_OVERLAPPING+pEntityRS->m_fWSRadius) )
				if(!bLMapGeneration)
					return;
	}

	// do not draw if already rendered in this frame
	if(pEntityRS->GetDrawFrame(m_nRenderStackLevel) == GetFrameID())
	{
/*		if(pEntityRS->GetEntityVisArea())
		{ // process object scissor settings
			if(pEntityRS->GetShadowFrame(m_nRenderStackLevel) == GetFrameID())
			{ // merge with previous scissor from this frame (needed when object is visible thru 2 portals)
				pEntityRS->GetEntityRS()->nScissorX1 = min(pEntityRS->GetEntityRS()->nScissorX1,EntViewCamera.m_ScissorInfo.x1);
				pEntityRS->GetEntityRS()->nScissorY1 = min(pEntityRS->GetEntityRS()->nScissorY1,EntViewCamera.m_ScissorInfo.y1);
				pEntityRS->GetEntityRS()->nScissorX2 = max(pEntityRS->GetEntityRS()->nScissorX2,EntViewCamera.m_ScissorInfo.x2);
				pEntityRS->GetEntityRS()->nScissorY2 = max(pEntityRS->GetEntityRS()->nScissorY2,EntViewCamera.m_ScissorInfo.y2);
			}
			else
			{ // set new
				pEntityRS->GetEntityRS()->nScissorX1 = EntViewCamera.m_ScissorInfo.x1;
				pEntityRS->GetEntityRS()->nScissorY1 = EntViewCamera.m_ScissorInfo.y1;
				pEntityRS->GetEntityRS()->nScissorX2 = EntViewCamera.m_ScissorInfo.x2;
				pEntityRS->GetEntityRS()->nScissorY2 = EntViewCamera.m_ScissorInfo.y2;
			}
		}*/
		return; // already drawn
	}


	// do not draw if marked to be not drawn
	unsigned int nRenderFlags = pEntityRS->GetRndFlags();
	if(nRenderFlags&ERF_HIDDEN || pEntityRS->m_dwRndFlags&ERF_MERGED)
		return;

	// for big objects (registered in sector 00) - get light mask from 00 sector
	if(pEntityRS->m_pSector == m_pTerrain->m_arrSecInfoTable[0][0])
	{
		CSectorInfo * pSectorInfo = m_pTerrain->GetSecInfo(vCenter);
		if(pSectorInfo)
			nDLightMask = pSectorInfo->m_nDynLightMask;
	}

	// set only x strongest light bits and do frustum test
	Vec3d vLightIntensity(0,0,0);
	Vec3d vBoxMin = pEntityRS->m_vWSBoxMin, vBoxMax = pEntityRS->m_vWSBoxMax;
	bool bEntityBodyVisible = true;
	CDLight * pStrongestShadowLight = 0;
	float fEntRadius = pEntityRS->m_fWSRadius;
	IRenderer * pRend = GetRenderer();
	CVars * pCVars = GetCVars();

	CDLight * pStrongestLightForTranspGeom = NULL;
	if(pCVars->e_stencil_shadows && (nRenderFlags&ERF_CASTSHADOWVOLUME || nRenderFlags&ERF_CASTSHADOWMAPS) && fEntRadius)
	{ // adjust bbox by shadow before frustum check
		list2<CDLight> * pSources = m_p3DEngine->GetDynamicLightSources();
		if(nDLightMask==1 && pSources->Count() && pSources->GetAt(0).m_Flags & DLF_SUN)
			pStrongestShadowLight = pStrongestLightForTranspGeom = pSources->Get(0);
		else if(nDLightMask)
			pStrongestShadowLight = m_p3DEngine->CheckDistancesToLightSources(nDLightMask, vCenter, fEntRadius, pEntityRS, 8, &pStrongestLightForTranspGeom, 1, &vLightIntensity);

		bool bAdjustBBoxByShadowSize = (nRenderFlags&ERF_CASTSHADOWVOLUME) && 
			pStrongestShadowLight && (pStrongestShadowLight->m_Flags & DLF_CASTSHADOW_VOLUME);
		bAdjustBBoxByShadowSize |= (nRenderFlags&ERF_CASTSHADOWMAPS) && 
			pStrongestShadowLight && (pStrongestShadowLight->m_Flags & DLF_CASTSHADOW_MAPS);

		bool bShadowIsVisible = false;
		if(pStrongestShadowLight)
		{ // shadow bbox frustum test
			float fShadowVolumeExtent = CalculateEntityShadowVolumeExtent(pEntityRS, pStrongestShadowLight);
			pEntityRS->GetBBox(vBoxMin, vBoxMax); // project geometry bbox
			MakeShadowBBox(vBoxMin, vBoxMax, pStrongestShadowLight->m_Origin, pStrongestShadowLight->m_fRadius, fShadowVolumeExtent);

			bShadowIsVisible = true;
			if(!(nRenderFlags&ERF_DONOTCHECKVIS))
				if( bNotAllInFrustum )
					if(!EntViewCamera.IsAABBVisible_exact(AABB( vBoxMin, vBoxMax )) && !bLMapGeneration)
						if(!pEntityRS->GetLight() || pEntityRS->GetLight()->m_pOwner!=pEntityRS)
							bShadowIsVisible = false; // shadow is not visible

			if(pCVars->e_stencil_shadows==2)
				pRend->Draw3dBBox(vBoxMin,vBoxMax);

			pEntityRS->GetRenderBBox(vBoxMin, vBoxMax); // restore bbox
		}

		bEntityBodyVisible = pEntityRS->m_bForceBBox || !bNotAllInFrustum || (nRenderFlags&ERF_DONOTCHECKVIS) ||
			EntViewCamera.IsAABBVisible_exact( AABB( vBoxMin, vBoxMax ));
		if(!bEntityBodyVisible && !pStrongestShadowLight && !bLMapGeneration && !bShadowIsVisible)
			return;
	}
	else 
	{ // check only original bbox
		bEntityBodyVisible = pEntityRS->m_bForceBBox || !bNotAllInFrustum || (nRenderFlags&ERF_DONOTCHECKVIS) || EntViewCamera.IsAABBVisible_exact( AABB( vBoxMin, vBoxMax ));
		if(!bEntityBodyVisible && !bLMapGeneration)
			return;

		list2<CDLight> * pSources = m_p3DEngine->GetDynamicLightSources();
		if(nDLightMask==1 && pSources->Count() && pSources->GetAt(0).m_Flags & DLF_SUN)
			pStrongestShadowLight = pStrongestLightForTranspGeom = pSources->Get(0);
		else if(nDLightMask)
			pStrongestShadowLight = m_p3DEngine->CheckDistancesToLightSources(nDLightMask, vCenter, fEntRadius, pEntityRS, 8, &pStrongestLightForTranspGeom, 1, &vLightIntensity);
	}

	if(fEntDistance > 2000.f && !bLMapGeneration)
	{
/*		Warning(0,0,
			"Warning: CObjManager::RenderObject: Invalid object skipped: %s, fEntDistance=%.2f, GetPos() = (%.2f,%.2f,%.2f)",
			pEntityRS->GetName(), fEntDistance,
			pEntityRS->GetPos().x,pEntityRS->GetPos().y,pEntityRS->GetPos().z);*/
		return; // skip invalid objects - usually only objects with invalid very big scale will reach this point
	}

	// check cvars
	EERType eERType = pEntityRS->GetEntityRenderType();
	switch(eERType)
	{
	case eERType_Brush:
		if(!pCVars->e_brushes)
			return;
		break;

	case eERType_Vegetation:
		if(!pCVars->e_vegetation)
			return;
		break;

	case eERType_Unknown:
		if(!pCVars->e_entities)
			return;

		if(nRenderFlags&ERF_FIRST_PERSON_CAMERA_OWNER)
			if(!GetCVars()->e_player)
				return;

		if(!m_nRenderStackLevel)
			if(nRenderFlags&ERF_CASTSHADOWMAPS)
				if(GetCVars()->e_shadow_spots && m_lstEntitiesShadowSpots.Count()<32 && fEntDistance<32)
					m_lstEntitiesShadowSpots.Add(pEntityRS);

		break;
	}

	// check all possible occlusions for outdoor objects
	if(fEntRadius && !bLMapGeneration && pCVars->e_portals!=3)
	{
		// test occlusion of outdoor objects by mountains
		if(m_fZoomFactor && fEntDistance/m_fZoomFactor > 48 && !pEntityRS->m_pVisArea)
			if(IsBoxOccluded(vBoxMin, vBoxMax, fEntDistance/m_fZoomFactor, &pEntityRS->OcclState))
				return;

		// test occl by antiportals
		if(GetVisAreaManager()->IsOccludedByOcclVolumes(vBoxMin,vBoxMax, pEntityRS->m_pVisArea!=NULL))
			return;
	}

	// store for later use (like tree sprites rendering)
	pEntityRS->m_arrfDistance[m_nRenderStackLevel] = fEntDistance;

	// mark as rendered in this frame
	if(bEntityBodyVisible)
		pEntityRS->SetDrawFrame( GetFrameID(), m_nRenderStackLevel );

	// process object particles (rain drops)
	if(GetCVars()->e_rain_amount)
		ProcessEntityParticles(pEntityRS,fEntDistance);

	// update scissor
	SRendParams DrawParams;  
//	GetRenderer()->SetGlobalShaderTemplateId(EFT_WHITESHADOW);
	DrawParams.nShaderTemplate = GetRenderer()->GetGlobalShaderTemplateId();

//	DrawParams.fSQDistance = fEntDistance*fEntDistance;
/*	if(pEntityRS->GetShadowFrame(m_nRenderStackLevel) == GetFrameID())
	{ // merge, this case maybe not needed
		pEntityRS->GetEntityRS()->nScissorX1 = min(pEntityRS->GetEntityRS()->nScissorX1,EntViewCamera.m_ScissorInfo.x1);
		pEntityRS->GetEntityRS()->nScissorY1 = min(pEntityRS->GetEntityRS()->nScissorY1,EntViewCamera.m_ScissorInfo.y1);
		pEntityRS->GetEntityRS()->nScissorX2 = max(pEntityRS->GetEntityRS()->nScissorX2,EntViewCamera.m_ScissorInfo.x2);
		pEntityRS->GetEntityRS()->nScissorY2 = max(pEntityRS->GetEntityRS()->nScissorY2,EntViewCamera.m_ScissorInfo.y2);
	}
	else
	{ // set
		pEntityRS->GetEntityRS()->nScissorX1 = EntViewCamera.m_ScissorInfo.x1;
		pEntityRS->GetEntityRS()->nScissorY1 = EntViewCamera.m_ScissorInfo.y1;
		pEntityRS->GetEntityRS()->nScissorX2 = EntViewCamera.m_ScissorInfo.x2;
		pEntityRS->GetEntityRS()->nScissorY2 = EntViewCamera.m_ScissorInfo.y2;
	}*/

	int nDLightMaskBeforeLightPassesSeparation = nDLightMask;
	DrawParams.nDLightMask  = nDLightMask;
	DrawParams.nFogVolumeID = 0;
	DrawParams.fDistance = fEntDistance;
	DrawParams.vAmbientColor = (pvAmbColor && pCVars->e_portals!=4) ? (*pvAmbColor) : m_vOutdoorAmbientColor;
	if(GetCVars()->e_objects_fade_on_distance)
		DrawParams.fAlpha = min(1.f,(1.f - fEntDistance / fMaxViewDist)*6);

	// adjust ambient level depending on lsources around for not lightmapped objects
	if(!(nRenderFlags & ERF_USELIGHTMAPS && pEntityRS->HasLightmap(0)) && pvAmbColor)
		if((vLightIntensity.x || vLightIntensity.y || vLightIntensity.z) && GetCVars()->e_dynamic_ambient_ratio)
		{
			vLightIntensity.x *= GetCVars()->e_dynamic_ambient_ratio;
			vLightIntensity.y *= GetCVars()->e_dynamic_ambient_ratio;
			vLightIntensity.z *= GetCVars()->e_dynamic_ambient_ratio;

			if(pvDynAmbColor)
			{
				vLightIntensity.x *= pvDynAmbColor->x;
				vLightIntensity.y *= pvDynAmbColor->y;
				vLightIntensity.z *= pvDynAmbColor->z;
			}

			DrawParams.vAmbientColor += vLightIntensity;

			DrawParams.vAmbientColor.CheckMin(Vec3d(1.f,1.f,1.f));
		}

		// modulate ambient color by envir color
		Vec3d vWorldColor = Get3DEngine()->GetWorldColor();
		DrawParams.vAmbientColor.x *= vWorldColor.x;
		DrawParams.vAmbientColor.y *= vWorldColor.y;
		DrawParams.vAmbientColor.z *= vWorldColor.z;

		DrawParams.fCustomSortOffset = GetSortOffset(vCenter,vCamPos);

		// ignore ambient color set by artist
		DrawParams.dwFObjFlags = FOB_IGNOREMATERIALAMBIENT;

    if(nRenderFlags&ERF_SELECTED)
    {
      DrawParams.vAmbientColor += Vec3d(0.2f,0.3f,0) + Vec3d(0.1f,0,0)*(int(GetTimer()->GetCurrTime()*12)%3==0);
			DrawParams.vAmbientColor.CheckMin(Vec3d(1.f,1.f,1.f));
      DrawParams.dwFObjFlags |= FOB_SELECTED;
    }

		assert(DrawParams.vAmbientColor.x>=0 && DrawParams.vAmbientColor.x<=1.f);
		assert(DrawParams.vAmbientColor.y>=0 && DrawParams.vAmbientColor.y<=1.f);
		assert(DrawParams.vAmbientColor.z>=0 && DrawParams.vAmbientColor.z<=1.f);

		if(pRend->EF_GetHeatVision())
			DrawParams.nShaderTemplate = EFT_HEATVISION;

		// process shadow maps
		CStatObj * pEntStatObj = NULL;
		if( pCVars->e_shadow_maps && m_nRenderStackLevel==0 && 
			(nRenderFlags&ERF_RECVSHADOWMAPS || nRenderFlags&ERF_CASTSHADOWMAPS) && 
			pStrongestShadowLight &&
			(pStrongestShadowLight->m_Flags & DLF_SUN || (pStrongestShadowLight->m_pOwner && pStrongestShadowLight->m_pOwner->GetRndFlags()&ERF_CASTSHADOWMAPS)) &&
			fEntDistance<(pEntityRS->GetRenderRadius()*GetCVars()->e_shadow_maps_view_dist_ratio) &&
			(!pEntityRS->GetLight() || pEntityRS->GetContainer()) && (pEntityRS->GetLight()!=pStrongestShadowLight) &&
			(pStrongestShadowLight->m_pOwner != pEntityRS) && // do not cast from it own light
			(!(pEntStatObj = (CStatObj*)pEntityRS->GetEntityStatObj(0,NULL,true)) || pEntStatObj->GetRenderTrisCount()) &&
			pEntityRS->IsEntityHasSomethingToRender())
		{ SetupEntityShadowMapping( pEntityRS, &DrawParams, bLMapGeneration, fEntDistance, pStrongestShadowLight ); }
		else
		{ 
			DrawParams.pShadowMapCasters = 0;
/*			if(pEntityRS->GetEntityRS() && pEntityRS->GetEntityRS()->pShadowMapInfo && m_nRenderStackLevel==0)
			{
				delete pEntityRS->GetEntityRS()->pShadowMapInfo;
				pEntityRS->GetEntityRS()->pShadowMapInfo=0;
			}*/
		}

		// draw bbox
		if (pCVars->e_bboxes)			
			GetRenderer()->Draw3dBBox(pEntityRS->m_vWSBoxMin, pEntityRS->m_vWSBoxMax);

		// only shadow is needed during terrain light maps calculation
		if(bLMapGeneration) 
			return; 

		// find cases when lighting need to be rendered in separate pass
		if(pEntityRS->GetShadowFrame(m_nRenderStackLevel) !=  (unsigned short)GetFrameID())
			if(pCVars->e_stencil_shadows && pStrongestShadowLight && fEntRadius && !GetCVars()->e_debug_lights)
			{ // make list of entities for each light casting shadow volume
				uint nNewMask = DrawParams.nDLightMask;
				for(int i=0; DrawParams.nDLightMask; i++)
				{
					if(DrawParams.nDLightMask & 1)
					{ // todo: remove EF_Query
						assert(i<m_p3DEngine->GetDynamicLightSources()->Count());
						CDLight * pDLight = m_p3DEngine->GetDynamicLightSources()->Get(i);
						assert(pDLight == (CDLight*)pRend->EF_Query(EFQ_LightSource, i));
						if(pDLight && (pDLight->m_Flags & DLF_CASTSHADOW_VOLUME))
						{
							nNewMask &= ~(1<<i); // remove this source from final mask

							if(!bEntityBodyVisible && EntViewCamera.IsSphereVisibleFast( Sphere(pDLight->m_Origin,0.1f) ))
							{
								DrawParams.nDLightMask = DrawParams.nDLightMask>>1;
								continue; // light is inside but caster is outside
							}

							assert(m_lstLightEntities[i].Find(pEntityRS)<0);
							if(pEntityRS->IsEntityHasSomethingToRender())
								m_lstLightEntities[i].Add(pEntityRS);
						}
					}

					DrawParams.nDLightMask = DrawParams.nDLightMask>>1;
				}

				DrawParams.nDLightMask = nNewMask;
				DrawParams.nSortValue = EFSLIST_STENCIL;

				pEntityRS->GetEntityRS()->nStrongestLightId = pStrongestShadowLight->m_Id;
			}

			// mark shadow as processed
			pEntityRS->SetShadowFrame( GetFrameID(), m_nRenderStackLevel );

			// set light mask for transparent geometry
			if(pStrongestLightForTranspGeom)
			{
				DrawParams.nStrongestDLightMask = 1<<pStrongestLightForTranspGeom->m_Id;
				if(DrawParams.fAlpha<1.f) // set it for entire object if entire object is transparent
					DrawParams.nDLightMask = DrawParams.nStrongestDLightMask & DrawParams.nDLightMask;
			}

			if(bEntityBodyVisible)
			{ // draw entity components (ambient(z-pass) with lights without shadowvolumes and fog) 
				bool bFogNeeded = nFogVolumeID>0;
				if(pFogVolume && !pFogVolume->InsideBBox((vCamPos.z>pFogVolume->vBoxMax.z) ? vCenter : Vec3d(vCenter.x,vCenter.y,vBoxMin.z)))
					bFogNeeded = false;
				else if(fEntDistance>256)
					bFogNeeded = false;
				else
					pEntityRS->m_nFogVolumeID = nFogVolumeID;

				// allows to sort correctly underwater fog passes
				if(bFogNeeded && 
					nDLightMaskBeforeLightPassesSeparation == DrawParams.nDLightMask &&
					!pEntityRS->GetEntityVisArea()) // can make overlays double fogged 
				{ 
					DrawParams.nFogVolumeID = nFogVolumeID; // include fog now if no lights was separated
					bFogNeeded = false;
				}

				// skip fog and detail passes if this is not last pass
				if(nDLightMaskBeforeLightPassesSeparation != DrawParams.nDLightMask)
					DrawParams.dwFObjFlags |= FOB_ZPASS;
        DrawParams.dwFObjFlags |= (~pEntityRS->m_dwRndFlags) & FOB_TRANS_MASK;

				if(!pEntityRS->DrawEntity(DrawParams))
					return;

				DrawParams.dwFObjFlags &= ~FOB_ZPASS;

				if(bFogNeeded && nFogVolumeID && !DrawParams.nFogVolumeID)
				{ // render separate fog pass if still was not rendered
					//    assert(nDLightMaskBewforeSeparatingShadowCasters != DrawParams.nDLightMask);
					DrawParams.nSortValue = eS_FogShader; // add fog to be rendered later, after light passes
					DrawParams.nDLightMask = 0;
					DrawParams.dwFObjFlags |= FOB_FOGPASS;
					DrawParams.nFogVolumeID = nFogVolumeID;
					if(!pEntityRS->DrawEntity(DrawParams))
						return;
				}
			}

			// add occluders into c-buffer
			if(	!m_nRenderStackLevel && 
				fEntDistance<COVERAGEBUFFER_OCCLUDERS_MAX_DISTANCE && 
				pCVars->e_cbuffer && 
				!bLMapGeneration && bEntityBodyVisible)
			{ // only use occlusion volume of first entity object 
				Matrix44 objMatrix;
				IStatObj * pStatObj = pEntityRS->GetEntityStatObj(0, &objMatrix);
				list2<Vec3d> * plstOcclVolVerts=0;
				list2<int> * plstOcclVolInds=0;

				if( pStatObj && pStatObj->GetOcclusionVolume(plstOcclVolVerts, plstOcclVolInds) && pCVars->e_cbuffer )
				{
					m_pCoverageBuffer->AddMesh(
						&(*plstOcclVolVerts)[0], (*plstOcclVolVerts).Count(),
						&(*plstOcclVolInds)[0],  (*plstOcclVolInds).Count(),
						&objMatrix);
				}
			}
}

int __cdecl CObjManager__Cmp_EntTmpDistance(const void* v1, const void* v2)
{
	IEntityRender * p1 = *((IEntityRender**)v1);
	IEntityRender * p2 = *((IEntityRender**)v2);

	if(p1==(IEntityRender*)-1)
		return 1;
	if(p2==(IEntityRender*)-1)
		return -1;

	if(p1->GetEntityRS()->fTmpDistance > p2->GetEntityRS()->fTmpDistance)
		return 1;
	else if(p1->GetEntityRS()->fTmpDistance < p2->GetEntityRS()->fTmpDistance)
		return -1;

	return 0;
}

void CObjManager::DrawEntitiesLightPass()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	Vec3d vWorldColor = Get3DEngine()->GetWorldColor();

	for(int nLightId=0; nLightId<MAX_LIGHTS_NUM; nLightId++)
	{
		CDLight * pDLight = (CDLight*)GetRenderer()->EF_Query(EFQ_LightSource, nLightId);
		if(!pDLight || !m_lstLightEntities[nLightId].Count())
			continue;

		assert(pDLight->m_Flags & DLF_CASTSHADOW_VOLUME);

		bool bUseStencilStateTest = false;
		// clear stencil
		GetRenderer()->EF_AddEf(0, m_pREClearStencil, m_p3DEngine->m_pSHClearStencil, NULL, GetIdentityCCObject(), 0, NULL, EFSLIST_STENCIL);

		// sort entities by distance to light source
		const Vec3d & vLightPos = pDLight->m_Origin;
		for(int nEntId=0; nEntId<m_lstLightEntities[nLightId].Count(); nEntId++)
		{
			IEntityRender * pEntityRS = m_lstLightEntities[nLightId][nEntId];
			if(pEntityRS==(IEntityRender*)-1)
				continue; // terrain light pass

			Vec3d vBoxMin,vBoxMax;
			pEntityRS->GetRenderBBox(vBoxMin,vBoxMax);
			pEntityRS->GetEntityRS()->fTmpDistance = GetDistance(vLightPos,(vBoxMin+vBoxMax)*0.5f);
		}

		qsort(&m_lstLightEntities[nLightId][0], m_lstLightEntities[nLightId].Count(), sizeof(m_lstLightEntities[nLightId][0]), CObjManager__Cmp_EntTmpDistance);

		// draw SS entity shadow volumes into stencil
		for(int nEntId=0; nEntId<m_lstLightEntities[nLightId].Count(); nEntId++)
		{
			IEntityRender * pEntityRS = m_lstLightEntities[nLightId][nEntId];
			if(pEntityRS==(IEntityRender*)-1)
				continue; // terrain light pass
			if(!(pEntityRS->GetRndFlags()&ERF_SELFSHADOW))
				continue; 
			if(!(pEntityRS->GetRndFlags()&ERF_CASTSHADOWVOLUME))
				continue; 

			if(GetCVars()->e_stencil_shadows_only_from_strongest_light)
				if(pEntityRS->GetEntityRS()->nStrongestLightId != pDLight->m_Id)
					continue;

			SRendParams DrawParams;     		
			DrawParams.pShadowVolumeLightSource = pDLight;
			// set scissor
//			DrawParams.nScissorX1 = pEntityRS->GetEntityRS()->nScissorX1;
	//		DrawParams.nScissorY1 = pEntityRS->GetEntityRS()->nScissorY1;
		//	DrawParams.nScissorX2 = pEntityRS->GetEntityRS()->nScissorX2;
			//DrawParams.nScissorY2 = pEntityRS->GetEntityRS()->nScissorY2;

			DrawParams.pCaller = pEntityRS;
			DrawParams.nDLightMask = 1<<pDLight->m_Id;
			DrawParams.nSortValue=EFSLIST_STENCIL; 
			DrawParams.fDistance = pEntityRS->m_arrfDistance[m_nRenderStackLevel];
			DrawParams.fShadowVolumeExtent = CalculateEntityShadowVolumeExtent(pEntityRS, pDLight);
      DrawParams.dwFObjFlags |= (~pEntityRS->m_dwRndFlags) & FOB_TRANS_MASK;
			if(DrawParams.fShadowVolumeExtent>1.f)
			{
				DrawParams.dwFObjFlags |= (pEntityRS->m_dwRndFlags&ERF_SELECTED) ? FOB_SELECTED : 0;
				pEntityRS->DrawEntity(DrawParams);
				bUseStencilStateTest = true;
			}
		}

		// draw NSS entity light pass and THEN shadow volumes into stencil
		for(int nEntId=0; nEntId<m_lstLightEntities[nLightId].Count(); nEntId++)
		{
			IEntityRender * pEntityRS = m_lstLightEntities[nLightId][nEntId];
			if(pEntityRS==(IEntityRender*)-1)
				continue; // terrain light pass
			if(pEntityRS->GetRndFlags()&ERF_SELFSHADOW)
				continue; 

			if(!(pDLight->m_Flags & DLF_LM && pEntityRS->GetRndFlags()&ERF_USELIGHTMAPS && pEntityRS->HasLightmap(0)) 
				|| pDLight->m_SpecColor != Col_Black)
			{ // light pass
				SRendParams DrawParams;     		
				DrawParams.nDLightMask = (1<<nLightId);
				DrawParams.nSortValue=EFSLIST_STENCIL;
				DrawParams.dwFObjFlags = FOB_LIGHTPASS;
				if(bUseStencilStateTest)
				{
					assert(pDLight->m_Flags & DLF_CASTSHADOW_VOLUME);
					assert(m_lstLightEntities[nLightId].Count());
					DrawParams.pStateShader = m_p3DEngine->m_pSHStencilState;
				}
				DrawParams.fDistance = pEntityRS->m_arrfDistance[m_nRenderStackLevel];
        DrawParams.dwFObjFlags |= (~pEntityRS->m_dwRndFlags) & FOB_TRANS_MASK;
				DrawParams.dwFObjFlags |= (pEntityRS->m_dwRndFlags&ERF_SELECTED) ? FOB_SELECTED : 0;
				pEntityRS->DrawEntity(DrawParams);
			}

			// shadow volumes
			if(pEntityRS->GetRndFlags()&ERF_CASTSHADOWVOLUME)
			{
				if(GetCVars()->e_stencil_shadows_only_from_strongest_light)
					if(pEntityRS->GetEntityRS()->nStrongestLightId != pDLight->m_Id)
						continue;

				SRendParams DrawParams;     		
				DrawParams.pShadowVolumeLightSource = pDLight;
				// set scissor
//				DrawParams.nScissorX1 = pEntityRS->GetEntityRS()->nScissorX1;
	//			DrawParams.nScissorY1 = pEntityRS->GetEntityRS()->nScissorY1;
		//		DrawParams.nScissorX2 = pEntityRS->GetEntityRS()->nScissorX2;
			//	DrawParams.nScissorY2 = pEntityRS->GetEntityRS()->nScissorY2;

				DrawParams.pCaller = pEntityRS;
				DrawParams.nDLightMask = 1<<pDLight->m_Id;
				DrawParams.nSortValue=EFSLIST_STENCIL; 
				DrawParams.fDistance = pEntityRS->m_arrfDistance[m_nRenderStackLevel];
				DrawParams.fShadowVolumeExtent = CalculateEntityShadowVolumeExtent(pEntityRS, pDLight);
				if(DrawParams.fShadowVolumeExtent>1.f)
				{
          DrawParams.dwFObjFlags |= (~pEntityRS->m_dwRndFlags) & FOB_TRANS_MASK;
					DrawParams.dwFObjFlags |= (pEntityRS->m_dwRndFlags&ERF_SELECTED) ? FOB_SELECTED : 0;
					pEntityRS->DrawEntity(DrawParams);
					bUseStencilStateTest = true;
				}
			}
		}

		// draw SS light pass with stencil test
		for(int nEntId=0; nEntId<m_lstLightEntities[nLightId].Count(); nEntId++)
		{
			IEntityRender * pEntityRS = m_lstLightEntities[nLightId][nEntId];
			if(pEntityRS == (IEntityRender*)-1)
			{
				if(GetVisAreaManager()->IsOutdoorAreasVisible())
					m_pTerrain->RenderDLightOnHeightMap(pDLight); // light on terrain
			}
			else if(!(pDLight->m_Flags & DLF_LM && pEntityRS->GetRndFlags()&ERF_USELIGHTMAPS && pEntityRS->HasLightmap(0))
				|| pDLight->m_SpecColor != Col_Black)
			{ // entity light pass
				if(!(pEntityRS->GetRndFlags()&ERF_SELFSHADOW))
					continue; 

				SRendParams DrawParams;     		
				DrawParams.nDLightMask = (1<<nLightId);
				DrawParams.nSortValue=EFSLIST_STENCIL;
				DrawParams.dwFObjFlags = FOB_LIGHTPASS;
				if(bUseStencilStateTest)
				{
					assert(pDLight->m_Flags & DLF_CASTSHADOW_VOLUME);
					assert(m_lstLightEntities[nLightId].Count());
					DrawParams.pStateShader = m_p3DEngine->m_pSHStencilState;
				}
				DrawParams.fDistance = pEntityRS->m_arrfDistance[m_nRenderStackLevel];
        DrawParams.dwFObjFlags |= (~pEntityRS->m_dwRndFlags) & FOB_TRANS_MASK;
				DrawParams.dwFObjFlags |= (pEntityRS->m_dwRndFlags&ERF_SELECTED) ? FOB_SELECTED : 0;
				pEntityRS->DrawEntity(DrawParams);
			}
		}

		// draw shadow pass with stencil test for lightmapped objects
		if(bUseStencilStateTest) // todo: add shadow-object bbox test
			for(int nEntId=0; nEntId<m_lstLightEntities[nLightId].Count(); nEntId++)
			{
				IEntityRender * pEntityRS = m_lstLightEntities[nLightId][nEntId];
				if(pEntityRS != (IEntityRender*)-1)
					if( pEntityRS->GetRndFlags()&ERF_USELIGHTMAPS && pEntityRS->HasLightmap(0) && 
						pDLight->m_Flags & DLF_LM &&
						pDLight->m_Flags & DLF_CASTSHADOW_VOLUME)
					{ 
						if(pDLight->m_Flags & DLF_DIRECTIONAL)
						{ // aditionall ambient pass with stencil test for outdoor brushes
							if(pEntityRS->GetEntityVisArea())
								continue; // scip indoor objects

							// detect if object recives shadow
							bool bDraw = false;
							for(int nCasterId=0; nCasterId<m_lstLightEntities[nLightId].Count(); nCasterId++)
							{
								if(	m_lstLightEntities[nLightId][nCasterId]->GetRndFlags()&ERF_CASTSHADOWVOLUME &&
									pEntityRS != m_lstLightEntities[nLightId][nCasterId] && 
									!m_lstLightEntities[nLightId][nCasterId]->GetEntityVisArea())
								{
									if(	IsSphereAffectedByShadow( m_lstLightEntities[nLightId][nCasterId], pEntityRS, pDLight))
									{
										IEntityRender * pCaster = m_lstLightEntities[nLightId][nCasterId];
										float fShadowVolumeExtent = CalculateEntityShadowVolumeExtent(pCaster, pDLight);
										Vec3d vShadowBoxMin=pCaster->m_vWSBoxMin, vShadowBoxMax=pCaster->m_vWSBoxMax;
										MakeShadowBBox(vShadowBoxMin, vShadowBoxMax, pDLight->m_Origin, pDLight->m_fRadius, fShadowVolumeExtent);
										if(AABB(vShadowBoxMin, vShadowBoxMax).IsIntersectBox(AABB(pEntityRS->m_vWSBoxMin,pEntityRS->m_vWSBoxMax)))
										{
											bDraw = true;
											break;
										}
									}
								}
							}

							if(!bDraw)
								continue;

							SRendParams DrawParams;     		
							DrawParams.vAmbientColor = m_vOutdoorAmbientColor;
							DrawParams.vAmbientColor.x *= vWorldColor.x;
							DrawParams.vAmbientColor.y *= vWorldColor.y;
							DrawParams.vAmbientColor.z *= vWorldColor.z;
							DrawParams.dwFObjFlags  = FOB_IGNOREMATERIALAMBIENT;
              DrawParams.dwFObjFlags |= (~pEntityRS->m_dwRndFlags) & FOB_TRANS_MASK;

							DrawParams.nDLightMask = 0;//(1<<nLightId);
							DrawParams.nSortValue=EFSLIST_STENCIL;
							if(bUseStencilStateTest)
							{
								assert(pDLight->m_Flags & DLF_CASTSHADOW_VOLUME);
								assert(m_lstLightEntities[nLightId].Count());
								DrawParams.pStateShader = m_p3DEngine->m_pSHStencilStateInv;
							}
							DrawParams.fDistance = pEntityRS->m_arrfDistance[m_nRenderStackLevel];

							// turn of lightmaps and render
							uint nOrigFlags = pEntityRS->GetRndFlags();
							pEntityRS->SetRndFlags(ERF_USELIGHTMAPS, false);
							DrawParams.dwFObjFlags |= (pEntityRS->m_dwRndFlags&ERF_SELECTED) ? FOB_SELECTED : 0;
							pEntityRS->DrawEntity(DrawParams);
							pEntityRS->SetRndFlags(nOrigFlags);
						}
						else
						{ // inverted light pass
							SRendParams DrawParams;     		
							DrawParams.vAmbientColor = Vec3d(0,0,0);
							// (pvAmbColor && pCVars->e_portals!=4) ? (*pvAmbColor) : m_vOutdoorAmbientColor;
							DrawParams.nDLightMask = (1<<nLightId);
							DrawParams.nSortValue=EFSLIST_STENCIL;
							DrawParams.nShaderTemplate = EFT_INVLIGHT;
							if(bUseStencilStateTest)
							{
								assert(pDLight->m_Flags & DLF_CASTSHADOW_VOLUME);
								assert(m_lstLightEntities[nLightId].Count());
								DrawParams.pStateShader = m_p3DEngine->m_pSHStencilStateInv;
							}
							DrawParams.fDistance = pEntityRS->m_arrfDistance[m_nRenderStackLevel];
              DrawParams.dwFObjFlags |= (~pEntityRS->m_dwRndFlags) & FOB_TRANS_MASK;
							DrawParams.dwFObjFlags |= (pEntityRS->m_dwRndFlags&ERF_SELECTED) ? FOB_SELECTED : 0;
							pEntityRS->DrawEntity(DrawParams);
						}
					}
			} 

			m_lstLightEntities[nLightId].Clear();
	}
}

void CObjManager::SetupEntityShadowMapping( IEntityRender * pEnt, SRendParams * pDrawParams, bool bLMapGeneration, float fEntDistance, CDLight * pDLight )
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

//	bool bPassiveShadowReseiver = 
	//	GetCVars()->e_active_shadow_maps_receving ? (pEnt->GetEntityRenderType()!=eERType_Unknown) : true;

	// Init ShadowMapInfo structure
	if(!pEnt->GetEntityRS()->pShadowMapInfo)
		pEnt->GetEntityRS()->pShadowMapInfo = new IEntityRenderState::ShadowMapInfo(); // leak

	// this list will contain shadow map casters
	if(!pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters)
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters = new list2<ShadowMapLightSourceInstance>;
	else
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters->Clear();

	if(pEnt->GetRndFlags()&ERF_CASTSHADOWMAPS)
	{ // make shadow frustum to project shadow to the world (and this entity)
		ProcessShadowMapCasting(pEnt, pDLight);
		if(GetCVars()->e_entities_debug)
			m_lstDebugEntityList.Add(pEnt);
	}
	else if(pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer && !bLMapGeneration)
	{ // unmake if not needed
		delete pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pEntityList;
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pEntityList=0;
		delete pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pModelsList;
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer->m_LightFrustums.Get(0)->pModelsList=0;

		if(pEnt->GetEntityRenderType()!=eERType_Vegetation) // vegetations share containers
			delete pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer;
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer=0;
	}
	
	if(pEnt->GetRndFlags()&ERF_RECVSHADOWMAPS_ACTIVE && GetCVars()->e_active_shadow_maps_receving)
	{ // make frustum containing all potential shadow casters (even not marked for shadow casting)
		ProcessActiveShadowReceiving(pEnt, fEntDistance, pDLight, bLMapGeneration);
	}
	else if(pEnt->GetRndFlags()&ERF_RECVSHADOWMAPS && GetCVars()->e_shadow_maps_receiving)
	{ // make list of potential active shadow map casters to cast to this entity
		MakeShadowMapInstancesList(pEnt, fEntDistance, pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters, 
			SMC_STATICS | SMC_DYNAMICS, pDLight);
	}
	else if(pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters && !bLMapGeneration && !(pEnt->GetRndFlags()&ERF_CASTSHADOWMAPS))
	{
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters->Clear();
		delete pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters;
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters = 0;
	}

	// pass result to output
	list2<struct ShadowMapLightSourceInstance> * pLsList = pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters;
	if(pLsList && pLsList->Count() && pLsList->Get(0)->m_pLS && pLsList->Get(0)->m_pLS->GetShadowMapFrustum())
	{
		assert(pLsList->Get(0)->m_pReceiver == pEnt || !pLsList->Get(0)->m_pReceiver);
		if(	pLsList->Count() == 1 &&
				pLsList->Get(0)->m_pLS->GetShadowMapFrustum()->pOwner == pLsList->Get(0)->m_pReceiver && 
			!(pLsList->Get(0)->m_pLS->GetShadowMapFrustum()->dwFlags & SMFF_ACTIVE_SHADOW_MAP))
			assert(pDrawParams->pShadowMapCasters == NULL); // skip single self shadowing pass
		else 
			pDrawParams->pShadowMapCasters = pLsList;
	}

	// if entity do not receive shadow - no more than this entity should be in the casters list
	assert(pEnt->GetRndFlags()&ERF_RECVSHADOWMAPS || 
		((pDrawParams->pShadowMapCasters == 0) || (pDrawParams->pShadowMapCasters->Count() == 1)));

	// add to list of outdoor shadow maps
	if(!m_nRenderStackLevel && (pEnt->GetRndFlags()&ERF_CASTSHADOWMAPS))
		if(m_lstStatEntitiesShadowMaps.Count()<64) // don't go crazy
			if(GetCVars()->e_shadow_maps_from_static_objects || (pEnt->GetEntityRenderType()==eERType_Unknown))
				m_lstStatEntitiesShadowMaps.Add(pEnt);
}
/*
int __cdecl CObjManager__Cmp_EntRadius(const void* v1, const void* v2)
{
	IEntityRender * p1 = *((IEntityRender**)v1);
	IEntityRender * p2 = *((IEntityRender**)v2);

	if(p1->GetRenderRadius() > p2->GetRenderRadius())
		return 1;
	else if(p1->GetRenderRadius() < p2->GetRenderRadius())
		return -1;

	return 0;
}
*/
void CObjManager::RenderEntitiesShadowMapsOnTerrain(bool bLMapGeneration, CREShadowMapGen * pREShadowMapGenerator)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	if(GetCVars()->e_shadow_maps && m_lstStatEntitiesShadowMaps.Count())
	{
		// Sort entities by size to avoid problems with overlaping
//		qsort(&m_lstStatEntitiesShadowMaps[0], m_lstStatEntitiesShadowMaps.Count(), sizeof(m_lstStatEntitiesShadowMaps[0]), CObjManager__Cmp_EntRadius);

		for(int i=0; i<m_lstStatEntitiesShadowMaps.Count(); i++)
			RenderEntityShadowOnTerrain(m_lstStatEntitiesShadowMaps[i], bLMapGeneration, pREShadowMapGenerator);
	}

	m_lstStatEntitiesShadowMaps.Clear();
}

void CObjManager::DrawEntitiesShadowSpotsOnTerrain()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	for(int i=0; GetCVars()->e_shadow_spots && i<m_lstEntitiesShadowSpots.Count(); i++)
	{
		Vec3d vEntBoxMin,vEntBoxMax;
		m_lstEntitiesShadowSpots.GetAt(i)->GetRenderBBox(vEntBoxMin,vEntBoxMax);
		Vec3d vEntCenter = (vEntBoxMin+vEntBoxMax)*0.5f;
		float fRadius = (vEntBoxMax-vEntBoxMin).len();
		((C3DEngine*)Get3DEngine())->DrawShadowSpotOnTerrain(vEntCenter, fRadius*0.5f);
	}

	m_lstEntitiesShadowSpots.Clear();
}

CFColor CObjManager::CalcShadowOnTerrainColor(float fAlpha, bool bLMapGeneration)
{ // calc shadow color
	//  float fAmbLevel = (m_vOutdoorAmbientColor.x+m_vOutdoorAmbientColor.y+m_vOutdoorAmbientColor.z)*0.33f*0.25f;
	//if(fAmbLevel<0.01f)
	//fAmbLevel=0.01f;

	//  if(fAlpha < fAmbLevel)
	//  fAlpha = fAmbLevel;

	if(bLMapGeneration)
		return CFColor(0,0,0);

	Vec3d vAverColor = m_vOutdoorAmbientColor*(1.f-fAlpha)*0.5f + Vec3d(1,1,1)*fAlpha;

	return CFColor (
		min(255.f,vAverColor.x), 
		min(255.f,vAverColor.y), 
		min(255.f,vAverColor.z));
}

void CObjManager::RenderEntityShadowOnTerrain(IEntityRender * pEntityRnd, bool bLMapGeneration,	CREShadowMapGen * pREShadowMapGenerator)
{
	float fEntRenderRadius = pEntityRnd->GetRenderRadius();
	if(fEntRenderRadius<2.f)
		fEntRenderRadius=2.f;

	Vec3d vShadowOffset = GetNormalized(m_p3DEngine->GetSunPosition())*fEntRenderRadius;

	IEntityRenderState * pRendState = pEntityRnd->GetEntityRS();

	if( pEntityRnd->GetShadowMapFrustumContainer() && 
		pEntityRnd->GetShadowMapFrustumContainer()->m_LightFrustums.Count() && 
		pRendState->pShadowMapInfo->pShadowMapCasters && fEntRenderRadius)
	{
		Vec3d vEntBoxMin,vEntBoxMax;
		pEntityRnd->GetBBox(vEntBoxMin,vEntBoxMax);
		Vec3d vEntCenter = (vEntBoxMin+vEntBoxMax)*0.5f;

		float fDistance = bLMapGeneration ? 0 : GetDistance(vEntCenter, GetViewCamera().GetPos());
		float fAlpha = (1.f - fDistance/(pEntityRnd->GetRenderRadius()*GetCVars()->e_shadow_maps_view_dist_ratio))*3.f;

		CCObject * pObj = GetIdentityCCObject();
		pObj->m_ObjFlags |= FOB_IGNOREREPOINTER;

		if(fAlpha<0)
			fAlpha=0; 

		pObj->m_SortId = 100.f*fAlpha;

		if(fAlpha>1.f)
			fAlpha = 1.f;

		if(GetCVars()->e_shadow_maps_fade_from_light_bit && !bLMapGeneration)
		{
			int nAreaSize = int(pEntityRnd->GetRenderRadius())/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize()+CTerrain::GetHeightMapUnitSize();
			if(nAreaSize>8)
				nAreaSize=8;
			int x1 = int(vEntCenter.x + nAreaSize/2)/nAreaSize*nAreaSize;
			int y1 = int(vEntCenter.y + nAreaSize/2)/nAreaSize*nAreaSize;
			for(int x=(x1-nAreaSize); x<=(x1+nAreaSize); x+=CTerrain::GetHeightMapUnitSize())
			for(int y=(y1-nAreaSize); y<=(y1+nAreaSize); y+=CTerrain::GetHeightMapUnitSize())
			{
				bool bLight = m_pTerrain->IsOnTheLight(x,y);
				if(!bLight)
					fAlpha -= (0.15f * CTerrain::GetHeightMapUnitSize()/nAreaSize * CTerrain::GetHeightMapUnitSize()/nAreaSize);
			}
			if(fAlpha<0)
				fAlpha=0;
		}

		if(fAlpha<0)
			fAlpha=0;

		ShadowMapLightSource * pShadowMapFrustumContainer = pEntityRnd->GetShadowMapFrustumContainer();
//		pObj->m_pShadowFrustum = pShadowMapFrustumContainer ? pShadowMapFrustumContainer->GetShadowMapFrustum() : 0;

		pObj->m_pShadowCasters = pEntityRnd->GetShadowMapCasters();

		pObj->m_Color = CalcShadowOnTerrainColor(1.f-fAlpha, bLMapGeneration);

		Vec3d vPos = vEntCenter - vShadowOffset;
		float fQuadRadius = fEntRenderRadius*2.f;
		if(fQuadRadius>100)
			fQuadRadius=100;

		// setup fading out
		pObj->m_TempVars[0] = vEntCenter.x;
		pObj->m_TempVars[1] = vEntCenter.y;
		pObj->m_TempVars[2] = vEntBoxMin.z;
		pObj->m_Color.a = bLMapGeneration ? 0 : (0.75f/fQuadRadius);

		// allign by unit size
		int nUnitSize = CTerrain::GetHeightMapUnitSize();
		vPos.x  = float(int(vPos.x  + 0.5f*nUnitSize)/nUnitSize*nUnitSize);
		vPos.y  = float(int(vPos.y  + 0.5f*nUnitSize)/nUnitSize*nUnitSize);
		vPos.z  = float(int(vPos.z  + 0.5f*nUnitSize)/nUnitSize*nUnitSize);
		fQuadRadius = float(int(fQuadRadius + 0.5f*nUnitSize)/nUnitSize*nUnitSize);

		bool bREAdded = false;
		if(!pEntityRnd->GetEntityVisArea() && GetVisAreaManager()->IsOutdoorAreasVisible() &&
			(!(pEntityRnd->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP) || bLMapGeneration) && fAlpha>0)
		{ // generate shadow map and render terrain shadow pass
			if( vPos.x<0 || vPos.x>=CTerrain::GetTerrainSize() || vPos.y<0 || vPos.y>=CTerrain::GetTerrainSize() )
				return;

			// bool bRecalcLeafBuffers = (bLMapGeneration || pRendState->vPrevTerShadowPos != vPos || pRendState->fPrevTerShadowRadius != fQuadRadius);
			bool bRecalcLeafBuffers = (bLMapGeneration || (!IsEquivalent(pRendState->pShadowMapInfo->vPrevTerShadowPos,vPos)) || pRendState->pShadowMapInfo->fPrevTerShadowRadius != fQuadRadius);


			pRendState->pShadowMapInfo->vPrevTerShadowPos = vPos;
			pRendState->pShadowMapInfo->fPrevTerShadowRadius = fQuadRadius;

			if(!pRendState->pShadowMapInfo->pShadowMapLeafBuffersList)
				pRendState->pShadowMapInfo->pShadowMapLeafBuffersList = new list2<struct CLeafBuffer *>;
			pRendState->pShadowMapInfo->pShadowMapLeafBuffersList->PreAllocate(16,16);

			bREAdded = m_pTerrain->RenderAreaLeafBuffers(vPos, fQuadRadius, 0, 
				pRendState->pShadowMapInfo->pShadowMapLeafBuffersList->GetElements(), 
				pRendState->pShadowMapInfo->pShadowMapLeafBuffersList->Count(),
				pObj, m_pTerrain->m_pTerrainShadowPassEf, bRecalcLeafBuffers, "EntityShadowOnTerrain",0,0,
				pRendState->pShadowMapInfo->pShadowMapFrustumContainer->GetShadowMapFrustum(),
				&Vec3d(pEntityRnd->GetPos()),
				(pEntityRnd->GetEntityRenderType() == eERType_Vegetation) ? pEntityRnd->GetScale() : 1.f);
		}

		if(pREShadowMapGenerator && !bREAdded)
		{ // just generate shadow map to use in indoors
			GetRenderer()->EF_AddEf(0, pREShadowMapGenerator, m_p3DEngine->m_pSHShadowMapGen, NULL, pObj, 0);
		}

		if(GetCVars()->e_shadow_maps_frustums)
			pEntityRnd->GetShadowMapFrustumContainer()->m_LightFrustums.Get(0)->DrawFrustum(GetRenderer(), 
			pEntityRnd->GetPos(), 1.f);
	}
}

float CObjManager::CalculateEntityShadowVolumeExtent(IEntityRender * pEntity, CDLight * pDLight)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	// calculate optimal extent for outdoor entity
	Vec3d vMinObjBBox, vMaxObjBBox;
	pEntity->GetRenderBBox(vMinObjBBox, vMaxObjBBox);

	Vec3d vObjCenter = (vMinObjBBox+vMaxObjBBox)*0.5;
	float fObjRadius = (vMaxObjBBox-vMinObjBBox).Length()*0.5f;
	float fObjLightDist = GetDistance(pDLight->m_Origin,vObjCenter) - fObjRadius;

	if(fObjLightDist<0.01f)
		fObjLightDist=0.01f;		

	float fRadiusForShadow = pDLight->m_fRadius;
	CVisArea * pArea = (CVisArea*)pEntity->GetEntityVisArea();
	if(pArea && pDLight->m_Flags & DLF_THIS_AREA_ONLY)
	{ // limit light radius by area box
		Vec3d vLightDir = pDLight->m_Origin - vObjCenter;
		Vec3d vLightDirOrig = vLightDir;
		vLightDir.SetLen(pDLight->m_fRadius);
		vLightDir.x = fabsf(vLightDir.x);
		vLightDir.y = fabsf(vLightDir.y);
		vLightDir.z = fabsf(vLightDir.z);

		for(int a=0; a<3; a++)
		{
			float fMaxDistToBoxFace = vLightDirOrig[a]<0 ? pArea->m_vBoxMax[a]-pDLight->m_Origin[a] : pDLight->m_Origin[a]-pArea->m_vBoxMin[a];
			if(vLightDir[a] > (fMaxDistToBoxFace))
				vLightDir /= vLightDir[a] / (fMaxDistToBoxFace);
		}

		fRadiusForShadow = vLightDir.Length();
	}

	if(fRadiusForShadow>(fObjLightDist+MAX_SHADOW_VOLUME_LEN))
		fRadiusForShadow=fObjLightDist+MAX_SHADOW_VOLUME_LEN;

	float fShadowVolumeExtent = fRadiusForShadow/fObjLightDist;

	if(fShadowVolumeExtent<1.f)
		fShadowVolumeExtent=1.f;
	else if(fShadowVolumeExtent>20.f)
		fShadowVolumeExtent=20.f;

	return fShadowVolumeExtent;
}

void CObjManager::ProcessEntityParticles(IEntityRender * pEnt, float fEntDistance)
{
	const float fMaxRainDropsDist = 12.f;

	if(!GetCVars()->e_rain_amount || fEntDistance>fMaxRainDropsDist)
		return;

	ParticleParams PartParams;
	PartParams.fFocus = 1.5f;
	PartParams.fLifeTime = 0.5f;
	PartParams.fSize = 0.02f;
	PartParams.fSizeSpeed = 0;
	PartParams.fSpeed = 1.f;
	PartParams.vGravity(0,0,-5.f);
	PartParams.nCount = 15;
	PartParams.eBlendType = ParticleBlendType_ColorBased;
	PartParams.nTexId = GetRenderer()->LoadTexture("cloud");
	float fAlpha = GetCVars()->e_rain_amount*min(1.f,(1.f-fEntDistance/fMaxRainDropsDist)*2.f);
	PartParams.vColorStart = Get3DEngine()->GetFogColor()*fAlpha;
	PartParams.vColorEnd = Get3DEngine()->GetFogColor()*fAlpha;
	PartParams.vDirection(0,0,1);

	for(int i=0; i<2; i++)
	{ // anim objects
		ICryCharInstance * pChar = pEnt->GetEntityCharacter(i);

		if(pChar)
			pChar->RemoveParticleEmitter(-1);

		if(pChar && (pChar->GetFlags() & CS_FLAG_DRAW_MODEL))
		{
			CryParticleSpawnInfo SpawnParams;
			SpawnParams.fSpawnRate = GetCVars()->e_rain_amount*120.f;
			SpawnParams.nFlags = CryParticleSpawnInfo::FLAGS_RAIN_MODE | CryParticleSpawnInfo::FLAGS_ONE_TIME_SPAWN;
			if(pChar->GetFlags() & CS_FLAG_DRAW_NEAR)
				PartParams.nParticleFlags |= PART_FLAG_DRAW_NEAR;
			else
				PartParams.nParticleFlags &= ~PART_FLAG_DRAW_NEAR;
			pChar->AddParticleEmitter(PartParams,SpawnParams);
		}
	}

	/*	{
	pEnt->GetEntityRS()->fParticleTimer += GetTimer()->GetFrameTime();

	Vec3d vBoxMin, vBoxMax;
	pEnt->GetRenderBBox(vBoxMin, vBoxMax);
	float fArea = (vBoxMax.x-vBoxMin.x)*(vBoxMax.y-vBoxMin.y);
	if(fArea)
	{
	const float fDropPeriod = max(0.002f, (1.f-GetCVars()->e_rain_amount)/45.f/fArea);

	while(pEnt->GetEntityRS()->fParticleTimer > fDropPeriod)
	{
	pEnt->GetEntityRS()->fParticleTimer -= fDropPeriod;

	Matrix44 StatObjMat;
	CStatObj * pStatObj = (CStatObj*)pEnt->GetEntityStatObj(0,&StatObjMat);
	if(pStatObj)
	pStatObj->SpawnParticles(PartParams,StatObjMat,true);
	}
	}
	}*/
}

void CObjManager::MakeShadowBBox(Vec3d & vBoxMin, Vec3d & vBoxMax, const Vec3d & vLightPos, float fLightRadius, float fShadowVolumeExtent)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	Vec3d arrVerts3d[8] = 
	{
		Vec3d(vBoxMin.x,vBoxMin.y,vBoxMin.z),
			Vec3d(vBoxMin.x,vBoxMax.y,vBoxMin.z),
			Vec3d(vBoxMax.x,vBoxMin.y,vBoxMin.z),
			Vec3d(vBoxMax.x,vBoxMax.y,vBoxMin.z),
			Vec3d(vBoxMin.x,vBoxMin.y,vBoxMax.z),
			Vec3d(vBoxMin.x,vBoxMax.y,vBoxMax.z),
			Vec3d(vBoxMax.x,vBoxMin.y,vBoxMax.z),
			Vec3d(vBoxMax.x,vBoxMax.y,vBoxMax.z)
	};

	for(int i=0; i<8; i++)
	{
		Vec3d vLightDir = arrVerts3d[i] - vLightPos;
		/*    float fDistToLightPos = max(0.1f,vLightDir.Length());
		float fDistToLightRadius = fLightRadius - fDistToLightPos;
		float t = max(0,fDistToLightRadius/fDistToLightPos);
		arrVerts3d[i] = arrVerts3d[i] + vLightDir * t;
		*/
		arrVerts3d[i] = vLightPos+vLightDir*fShadowVolumeExtent;
		vBoxMin.CheckMin(arrVerts3d[i]);
		vBoxMax.CheckMax(arrVerts3d[i]);
	}
}

bool CObjManager::ProcessShadowMapCasting(IEntityRender * pEnt, CDLight * pDLight)
{
	if(pEnt->GetEntityRenderType() == eERType_Vegetation)
	{ // shadow casting already prepared at loading time, todo: move it in real-time
		pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer = pEnt->GetShadowMapFrustumContainer();
		pEnt->GetShadowMapCasters();
	}
	else
	{
		ShadowMapLightSource * & pLsource = pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer;

		if(!pLsource)
		{ // make lsource
			pLsource = new ShadowMapLightSource;
			ShadowMapFrustum lof;
			lof.pOwner = pEnt;
			pLsource->m_LightFrustums.Add(lof);
		}

		// get needed size of shadow map
		int nTexSize = GetCVars()->e_max_shadow_map_size;
		float fDistToTheCamera = pEnt->m_arrfDistance[m_nRenderStackLevel];
		Vec3d vBoxMin,vBoxMax;
		pEnt->GetBBox(vBoxMin,vBoxMax);
		float fCasterRadius = (vBoxMax-vBoxMin).len()*0.5f;
		while( nTexSize*fDistToTheCamera > fCasterRadius*GetCVars()->e_shadow_maps_size_ratio )
			nTexSize /= 2;

		// get obj space light pos
		Vec3d vObjSpaceLightPos; Matrix44 objMatrix;
		IStatObj * pStatObj = pEnt->GetEntityStatObj(0, &objMatrix);
		if(pStatObj)
		{
			objMatrix.Invert44();
			vObjSpaceLightPos = objMatrix.TransformVectorOLD(pDLight ? pDLight->m_Origin  : m_p3DEngine->GetSunPosition());
		}
		else
			vObjSpaceLightPos = (pDLight ? pDLight->m_Origin  : m_p3DEngine->GetSunPosition()) - pEnt->GetPos();

		// request shadow map update if needed
		assert(pDLight);
		if(!pDLight)
			return true;

		// process shadow map for casting
		if( nTexSize != pLsource->GetShadowMapFrustum()->nTexSize ||
			!IsEquivalent(pLsource->vObjSpaceSrcPos, vObjSpaceLightPos, 0.01f) || 
			pEnt->HasChanged() || pLsource->GetShadowMapFrustum()->depth_tex_id==0 )
		{
			pLsource->GetShadowMapFrustum()->bUpdateRequested = true;
			pLsource->vSrcPos = (pDLight ? pDLight->m_Origin  : m_p3DEngine->GetSunPosition()) - pEnt->GetPos();
			pLsource->vObjSpaceSrcPos = vObjSpaceLightPos;
			pLsource->fRadius = pDLight ? pDLight->m_fRadius : 5000000;
			MakeEntityShadowFrustum(pLsource->GetShadowMapFrustum(), pLsource, pEnt, EST_DEPTH_BUFFER, 0);
			pLsource->GetShadowMapFrustum()->nTexSize = nTexSize;
		}

		if(pLsource->GetShadowMapFrustum())
		{
			float fAlpha = (1.f-fDistToTheCamera/(pEnt->GetRenderRadius()*GetCVars()->e_shadow_maps_view_dist_ratio))*3.f;
			pLsource->GetShadowMapFrustum()->fAlpha = min(1.f,fAlpha);

			pLsource->GetShadowMapFrustum()->nDLightId = pLsource->nDLightId = pDLight ? pDLight->m_Id : -1;
		}

		// add this frustum to the list of shadow casters for self shadowing and shadow on terrain
		if(pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer)
		{
			ShadowMapLightSourceInstance LightSourceInfo;
			LightSourceInfo.m_pLS              = pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer;
			LightSourceInfo.m_vProjTranslation = pEnt->GetPos();
			LightSourceInfo.m_fProjScale       = 1.f;
			Vec3d vThisEntityPos = pEnt->GetPos();
			LightSourceInfo.m_fDistance        = 0;
			LightSourceInfo.m_pReceiver        = pEnt;
			if(LightSourceInfo.m_pLS->m_LightFrustums.Count())
				pEnt->GetEntityRS()->pShadowMapInfo->pShadowMapCasters->Add(LightSourceInfo);
		}
	}

	return true;
}

bool CObjManager::IsSphereAffectedByShadow(IEntityRender * pCaster, IEntityRender * pReceiver, CDLight * pLight)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	assert(pLight->m_Flags & DLF_DIRECTIONAL);

	// get spheres
	Vec3d vBoxMin,vBoxMax;
	pCaster->GetBBox(vBoxMin,vBoxMax);
	Vec3d vCasterCenter = (vBoxMin+vBoxMax)*0.5f;
	float fCasterRadius = (vBoxMax-vBoxMin).len()*0.5f;
	//	GetRenderer()->Draw3dBBox(vBoxMin,vBoxMax,DPRIM_SOLID_SPHERE);

	pReceiver->GetBBox(vBoxMin,vBoxMax);
	Vec3d vReceiverCenter = (vBoxMin+vBoxMax)*0.5f;
	float fReceiverRadius = (vBoxMax-vBoxMin).len()*0.5f;
	//GetRenderer()->Draw3dBBox(vBoxMin,vBoxMax,DPRIM_SOLID_SPHERE);

	// define rays and distances
	Vec3d vCasterDir = vCasterCenter - pLight->m_Origin;
	float fDistToCaster = vCasterDir.len();
	Vec3d vReceiverDir = vReceiverCenter - pLight->m_Origin;
	float fDistToReceiver = vReceiverDir.len();

	if((fDistToReceiver+fReceiverRadius) < (fDistToCaster-fCasterRadius))
		return false;

	if((fDistToReceiver-fReceiverRadius) > (fDistToCaster+fCasterRadius+MAX_SHADOW_VOLUME_LEN))
		return false;

	// move caster to receiver
	vCasterDir *= (fDistToReceiver/fDistToCaster);

	float fDist = vCasterDir.GetDistance(vReceiverDir);

	return fDist < (fCasterRadius+fReceiverRadius);
}