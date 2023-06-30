////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   3denginelight.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Light sources manager
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "3dEngine.h"
#include "objman.h"
#include "visareas.h"
#include "AABBSV.h"

#include "partman.h"
#include <LMCompStructures.h>
#ifndef PI
#define PI 3.14159f
#endif

void C3DEngine::RegisterLightSourceInSectors(CDLight * pDynLight)
{
	if(!m_pTerrain)
		return;
	/*
	pDynLight->m_Color.r = min(pDynLight->m_Color.r,1.f);
	pDynLight->m_Color.g = min(pDynLight->m_Color.g,1.f);
	pDynLight->m_Color.b = min(pDynLight->m_Color.b,1.f);
	pDynLight->m_Color.a = min(pDynLight->m_Color.a,1.f);*/
	//pDynLight->m_SpecColor.r = min(pDynLight->m_SpecColor.r,1.f);
	//pDynLight->m_SpecColor.g = min(pDynLight->m_SpecColor.g,1.f);
	//pDynLight->m_SpecColor.b = min(pDynLight->m_SpecColor.b,1.f);
	//pDynLight->m_SpecColor.a = min(pDynLight->m_SpecColor.a,1.f);
	/*
	if(pDynLight->m_Flags & (DLF_LIGHTSOURCE|DLF_HEATSOURCE))
	pDynLight->m_Flags &= ~DLF_FAKE;
	else
	pDynLight->m_Flags |=  DLF_FAKE;
	*/
	// Register in renderer and set lsource id
	//	GetRenderer()->EF_ADDDlight(pDynLight);

	if(pDynLight->m_Id == -1) // ignored by renderer
		return; // fake
	else
		if (pDynLight->m_pShader!=0 && (pDynLight->m_pShader->GetLFlags() & LMF_DISABLE))
			return; // fake


	// Register in sectors
	// find 2d bounds in sectors array
	int min_x = (int)(((pDynLight->m_Origin.x - pDynLight->m_fRadius - TERRAIN_SECTORS_MAX_OVERLAPPING)/CTerrain::GetSectorSize()));
	int min_y = (int)(((pDynLight->m_Origin.y - pDynLight->m_fRadius - TERRAIN_SECTORS_MAX_OVERLAPPING)/CTerrain::GetSectorSize()));
	int max_x = (int)(((pDynLight->m_Origin.x + pDynLight->m_fRadius + TERRAIN_SECTORS_MAX_OVERLAPPING)/CTerrain::GetSectorSize()));
	int max_y = (int)(((pDynLight->m_Origin.y + pDynLight->m_fRadius + TERRAIN_SECTORS_MAX_OVERLAPPING)/CTerrain::GetSectorSize()));

	if( min_x<0 ) min_x = 0; else if( min_x>=CTerrain::GetSectorsTableSize() ) min_x = CTerrain::GetSectorsTableSize()-1;
	if( min_y<0 ) min_y = 0; else if( min_y>=CTerrain::GetSectorsTableSize() ) min_y = CTerrain::GetSectorsTableSize()-1;
	if( max_x<0 ) max_x = 0; else if( max_x>=CTerrain::GetSectorsTableSize() ) max_x = CTerrain::GetSectorsTableSize()-1;
	if( max_y<0 ) max_y = 0; else if( max_y>=CTerrain::GetSectorsTableSize() ) max_y = CTerrain::GetSectorsTableSize()-1;

	// set lmask in all affected sectors
	if(pDynLight->m_Id>=0)
		for(int x=min_x; x<=max_x; x++)
			for(int y=min_y; y<=max_y; y++)
			{
				CSectorInfo * pSecInfo = m_pTerrain->m_arrSecInfoTable[x][y];

				assert(pSecInfo->m_nDynLightMask>=0);
				assert(pSecInfo->m_nDynLightMaskNoSun>=0);

				pSecInfo->m_nDynLightMask |= (1<<pDynLight->m_Id);
				if(!(pDynLight->m_Flags & DLF_SUN)) // skip sun for static world
					pSecInfo->m_nDynLightMaskNoSun |= (1<<pDynLight->m_Id);

				assert(pSecInfo->m_nDynLightMask>0);
				assert(pSecInfo->m_nDynLightMaskNoSun>=0);
			}
}

// render light pass on terrain
void CTerrain::RenderDLightOnHeightMap(CDLight * pDLight)
{
	/*	if( (!(pDLight->m_Flags & DLF_DIRECTIONAL)) &&
	!Get3DEngine()->IsSphereVisibleOnTheScreen(pDLight->m_Origin, pDLight->m_fRadius, 0))
	return; // invisible
	*/
	// skip indoor lightsource
	if( pDLight->m_pOwner && pDLight->m_pOwner->GetEntityVisArea() )
		return;

	if(pDLight->m_Flags & DLF_IGNORE_TERRAIN)
		return;

	if ((pDLight->m_Flags & DLF_ONLY_FOR_HIGHSPEC) && (m_LightConfigSpec < CONFIG_HIGH_SPEC))
		return; // Skip high spec only light on medium and low spec configs.

	if(!(pDLight->m_Flags & DLF_DIRECTIONAL))
	{ // draw light on the ground
		Vec3d vPos = pDLight->m_Origin;
		float fRadius = pDLight->m_fRadius;

		// check terrain bounds
		if(vPos.x < -fRadius || vPos.y < -fRadius)
			return;
		if(vPos.x >= CTerrain::GetTerrainSize() + fRadius || vPos.y >= CTerrain::GetTerrainSize() + fRadius)
			return;

		const int nUsintSize = CTerrain::GetHeightMapUnitSize();
		fRadius += nUsintSize;
		vPos.x  = float(int(vPos.x  + 0.5f*nUsintSize)/nUsintSize*nUsintSize);
		vPos.y  = float(int(vPos.y  + 0.5f*nUsintSize)/nUsintSize*nUsintSize);
		vPos.z  = float(int(vPos.z  + 0.5f*nUsintSize)/nUsintSize*nUsintSize);
		fRadius = float(int(fRadius + 0.5f*nUsintSize)/nUsintSize*nUsintSize);

		if( fabs(vPos.z - GetZSafe(int(vPos.x),int(vPos.y))) > fRadius )
			return; // too far from ground surface

		IShader * pEffStencilTest = pDLight->m_Flags & DLF_CASTSHADOW_VOLUME ? GetRenderer()->EF_LoadShader("StencilState_Terrain",eSH_Misc, EF_SYSTEM) : 0;

		//		if(m_pVisAreaManager->IsOutdoorAreasVisible())
		RenderAreaLeafBuffers(
			vPos,
			fRadius,
			1<<pDLight->m_Id,
			pDLight->m_arrLightLeafBuffers,
			sizeof(pDLight->m_arrLightLeafBuffers)/sizeof(pDLight->m_arrLightLeafBuffers[0]),
			GetIdentityCCObject(), m_pTerrainLightPassEf, true, pDLight->m_sDebugName,
			pEffStencilTest); // todo: do not recalculate always
	}
}

INT_PTR C3DEngine::AddStaticLightSource(const class CDLight & LSource, IEntityRender *__pCreator, ICryCharInstance * pCryCharInstance, const char * szBoneName)	//AMD Port
{
	// try to delete source if it's already present
	/*	for(int i=0; i<m_lstStaticLights.Count(); i++)
	{
	if( m_lstStaticLights[i]->m_pLight->m_pCharInstance == pCryCharInstance )//&& m_lstStaticLights[i] == pCreator )
	{
	delete m_lstStaticLights[i];
	m_lstStaticLights.Delete(i);
	i--;
	}
	}
	*/
	// construct new object
	CDLight * pNewLight = new CDLight();
	*pNewLight = LSource;
	CLightEntity * pLightEntity = new CLightEntity( );
	pNewLight->m_pOwner = pLightEntity; 
	pLightEntity->m_pLight = pNewLight;
	pLightEntity->m_vPos = LSource.m_Origin;

	if(pCryCharInstance) // character instance will update position
	{
		if (pCryCharInstance->AttachLight(pNewLight,pCryCharInstance->GetModel()->GetBoneByName(szBoneName)))
			pNewLight->m_pCharInstance = pCryCharInstance;
	}

	Get3DEngine()->RegisterEntity(pLightEntity);

	m_lstStaticLights.Add(pLightEntity);

	return (INT_PTR)pLightEntity;						//AMD Port
}

bool C3DEngine::DeleteStaticLightSource(INT_PTR nLightId)	//AMD Port
{
	CLightEntity * pLightEntity = (CLightEntity*)nLightId;
	if(m_lstStaticLights.Delete(pLightEntity))
	{
		delete pLightEntity;
		return true;
	}

	return false; // not found
}

const list2<CDLight*> * C3DEngine::GetStaticLightSources()
{
	// tmp solution since .h files are checked out
	static list2<CDLight*> lstLights;
	lstLights.Reset();

	for(int i=0; i<m_lstStaticLights.Count(); i++)
		lstLights.Add(m_lstStaticLights[i]->m_pLight);

	return &lstLights;
}

void C3DEngine::UpdateStaticLightSources()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	for(int i=0; i<m_lstStaticLights.Count(); i++)
	{
		//    if(i==94)
		//    i=i;

		CLightEntity * pLightEntity = m_lstStaticLights[i];
		CDLight * pLight = pLightEntity->m_pLight;
		//		pLight->m_nStaticLightId = i;
		/*
		if(pLight->m_pCharInstance)
		{
		//Matrix44 EntityMatrix;
		//EntityMatrix.Identity();
		//EntityMatrix = GetTranslationMat(pLightEntity->GetPos(true))*EntityMatrix;
		//EntityMatrix = GetRotationZYX44(-gf_DEGTORAD*pLightEntity->GetAngles(0))*EntityMatrix;
		//EntityMatrix = GetScale33( Vec3d(pLight->m_pOwner->GetScale(),pLight->m_pOwner->GetScale(),pLight->m_pOwner->GetScale()))*EntityMatrix;

		//OPTIMISED_BY_IVO  
		Matrix33diag diag				=	Vec3(pLight->m_pOwner->GetScale(),pLight->m_pOwner->GetScale(),pLight->m_pOwner->GetScale() );		//use diag-matrix for scaling
		Matrix34 rt34						=	Matrix34::GetRotationXYZ34(gf_DEGTORAD*pLightEntity->GetAngles(0),pLightEntity->GetPos(true) );	//set scaling and translation in one function call
		Matrix44 EntityMatrix		=	rt34*diag;	//optimised concatenation: m34*diag
		EntityMatrix.Transpose();							//TODO: remove this after E3 and use Matrix34 instead of Matrix44

		AddDynamicLightSource(*pLight,pLight->m_pOwner, 0, &EntityMatrix );
		}
		else*/
		AddDynamicLightSource(*pLight,pLight->m_pOwner);
	}
}

void C3DEngine::DeleteAllStaticLightSources()
{
	for(int i=0; i<m_lstStaticLights.Count(); i++)
		delete m_lstStaticLights[i];
	m_lstStaticLights.Reset();
}

Vec3d ConvertProjAngles(const Vec3d & vIn)
{
	Vec3d vOut;
	vOut.x = vIn.x;
	vOut.y = vIn.y;
	vOut.z = vIn.z;
	return vOut;
}

void C3DEngine::AddDynamicLightSource(const class CDLight & LSource, IEntityRender *pEnt, int nEntityLightId, const Matrix44 * pMatrix)
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Check errors
	////////////////////////////////////////////////////////////////////////////////////////////////

	assert(pEnt);

	if(!pEnt)
	{ 
		Warning(0,0,"C3DEngine::AddDynamicLightSource: Entity not specified"); 
		return; 
	}

	if(!_finite(LSource.m_Origin.x) || !_finite(LSource.m_Origin.y) || !_finite(LSource.m_fRadius))
	{
		Warning(0,0,"C3DEngine::AddDynamicLightSource: undefined light source position: %s", LSource.m_sDebugName);
		return;
	}

	if ((LSource.m_Flags & DLF_ONLY_FOR_HIGHSPEC) && (m_LightConfigSpec < CONFIG_HIGH_SPEC))
		return; // Skip high spec only light on medium and low spec configs.

	if(m_lstDynLights.Count()==128)
		Warning(0,0,"C3DEngine::AddDynamicLightSource: more than 128 dynamic light sources created");
	else if(m_lstDynLights.Count()>128)
		return;

	if(LSource.m_Flags & DLF_LOCAL)
		return; // this lsource affects only owner

	if(LSource.m_Flags & DLF_TEMP)
		return; // this lsource is used only during preprocess

	if(	!GetCVars()->e_dynamic_light_debug && GetRenderer()->EF_GetHeatVision() && !(LSource.m_Flags & DLF_HEATSOURCE) )
		return;

	if(	!GetRenderer()->EF_GetHeatVision() && !(LSource.m_Flags & DLF_LIGHTSOURCE ) )
		return;

	if(!GetCVars()->e_dynamic_light)
		return;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Detect sun case
	////////////////////////////////////////////////////////////////////////////////////////////////

	if(pEnt == (IEntityRender*)-1)
	{ // sun
		if(LSource.m_Color.r + LSource.m_Color.g + LSource.m_Color.b == 0)
			return; // sun disabled

		pEnt = 0;
	}
	else	if(GetCVars()->e_dynamic_light_exact_vis_test)
	{
		if(GetFrameID() - pEnt->GetDrawFrame(0) > MAX_FRAME_ID_STEP_PER_FRAME)
			return;
	}

	Vec3d vWSOrigin = LSource.m_Origin;
	if(pMatrix)
		vWSOrigin = pMatrix->TransformPointOLD(LSource.m_vObjectSpacePos);

	if(GetCVars()->e_dynamic_light_exact_vis_test)
		if(!GetViewCamera().IsSphereVisibleFast( Sphere(vWSOrigin,LSource.m_fRadius) ))
			return; // invisible

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Make lsource name for debugging
	////////////////////////////////////////////////////////////////////////////////////////////////

	char sName[sizeof(LSource.m_sDebugName)]="";
	if (pEnt)
	{
		// crash test
		if(pEnt->GetEntityRS() && pEnt->m_pVisArea)
		{
			int nCount = pEnt->m_pVisArea->m_lstConnections.Count();
			if(nCount==1233)
				return;
		}

		const char * pName = pEnt->GetEntityClassName();
		if(!pName || !pName[0])
			pName = pEnt->GetName();

		strncpy(sName, pName ? pName : "???", sizeof(sName));
		sName[sizeof(sName)-1]=0;

		if(GetCVars()->e_dynamic_light_exact_vis_test)
			if(!m_pObjManager || !m_pVisAreaManager->IsEntityVisAreaVisible(pEnt,!(LSource.m_Flags & DLF_THIS_AREA_ONLY)))
			{
				bool bFirstPersonOwner = pEnt && pEnt->GetRndFlags() & ERF_FIRST_PERSON_CAMERA_OWNER;
				if(!bFirstPersonOwner)
					return;			
			}
	}
	else
		strncpy(sName, "Sun", sizeof(sName));

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Try to update present lsource
	////////////////////////////////////////////////////////////////////////////////////////////////

	for (int i=0; i<m_lstDynLights.Count(); i++)
	{
		if( m_lstDynLights[i].m_pOwner == pEnt && m_lstDynLights[i].m_nEntityLightId == nEntityLightId )
		{
			// copy lsource (do not owerwrite m_arrLightLeafBuffers, pEntityRS and nEntityLightId)
			CLeafBuffer * TmpLightLeafBuffers[8];
			assert(sizeof(TmpLightLeafBuffers) == sizeof(m_lstDynLights[i].m_arrLightLeafBuffers));

			memcpy(TmpLightLeafBuffers,m_lstDynLights[i].m_arrLightLeafBuffers,sizeof(TmpLightLeafBuffers));

			CCObject *pObj[4][4];
			memcpy(&pObj[0][0], &m_lstDynLights[i].m_pObject[0][0], sizeof(pObj));

			m_lstDynLights[i] = LSource;
			// !HACK: Needs to decrement refcounter of shader because m_lstDynLights never release light sources
			if (LSource.m_pShader)
				LSource.m_pShader->Release();

			memcpy(&m_lstDynLights[i].m_pObject[0][0], &pObj[0][0], sizeof(pObj));

			memcpy(m_lstDynLights[i].m_arrLightLeafBuffers,TmpLightLeafBuffers,sizeof(TmpLightLeafBuffers));
			m_lstDynLights[i].m_pOwner = pEnt;
			m_lstDynLights[i].m_nEntityLightId = nEntityLightId;
			m_lstDynLights[i].m_pCharInstance= 0;

			// reinit start time
			m_lstDynLights[i].m_fStartTime = GetTimer()->GetCurrTime();
			m_lstDynLights[i].m_fLifeTime = LSource.m_fLifeTime;

			// transform if needed
			if(pMatrix)
			{	
				m_lstDynLights[i].m_Origin		 = vWSOrigin;//pMatrix->TransformPoint(LSource.m_vObjectSpacePos);
				//CHANGED_BY_IVO
				//m_lstDynLights[i].m_ProjAngles = pMatrix->TransformVector(LSource.m_ProjAngles);
				m_lstDynLights[i].m_ProjAngles = GetTransposed44(*pMatrix)*(LSource.m_ProjAngles);
			}

			strncpy(m_lstDynLights[i].m_sDebugName,sName,8);
			/*
			Vec3d Angles(pDLight->m_ProjAngles[1], 0, pDLight->m_ProjAngles[2]+90.0f);
			cam.SetAngle(Angles);
			cam.Init(1, 1, (pDLight->m_fLightFrustumAngle*2)/180.0f*PI, pDLight->m_fRadius, 1.0f, 0.1f);
			cam.Update();                       */

			m_lstDynLights[i].m_ProjAngles = ConvertProjAngles(LSource.m_ProjAngles);

			// set base params
			m_lstDynLights[i].m_BaseOrigin = m_lstDynLights[i].m_Origin;
			m_lstDynLights[i].m_BaseProjAngles = m_lstDynLights[i].m_ProjAngles;

			if ((m_lstDynLights[i].m_Flags & DLF_SPECULAR_ONLY_FOR_HIGHSPEC) && (m_LightConfigSpec < CONFIG_HIGH_SPEC))
			{
				m_lstDynLights[i].m_SpecColor = Col_Black;
			}

			return;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Add new lsource into list and set some parameters
	////////////////////////////////////////////////////////////////////////////////////////////////

	m_lstDynLights.Add(LSource);

	// add ref to avoid shader deleting
	if (m_lstDynLights.Last().m_pShader)
		m_lstDynLights.Last().m_pShader->AddRef();
	m_lstDynLights.Last().m_Flags |= DLF_COPY;

	m_lstDynLights.Last().m_fStartTime = GetTimer()->GetCurrTime();
	m_lstDynLights.Last().m_fLifeTime = LSource.m_fLifeTime;

	if (!LSource.m_fEndRadius)
		m_lstDynLights.Last().m_fStartRadius = LSource.m_fRadius;

	m_lstDynLights.Last().m_pOwner = pEnt;
	m_lstDynLights.Last().m_nEntityLightId = nEntityLightId;

	// transform if needed
	if(pMatrix)
	{	
		m_lstDynLights.Last().m_Origin		 = vWSOrigin;//pMatrix->TransformPoint(LSource.m_vObjectSpacePos);
		//CHANGED_BY_IVO
		//m_lstDynLights.Last().m_ProjAngles = pMatrix->TransformVector(LSource.m_ProjAngles);
		m_lstDynLights.Last().m_ProjAngles = GetTransposed44(*pMatrix)*(LSource.m_ProjAngles);
	}

	strncpy(m_lstDynLights.Last().m_sDebugName,sName,8);
	m_lstDynLights.Last().m_ProjAngles = ConvertProjAngles(LSource.m_ProjAngles);

	// set base params
	m_lstDynLights.Last().m_BaseOrigin = m_lstDynLights.Last().m_Origin;
	m_lstDynLights.Last().m_BaseProjAngles = m_lstDynLights.Last().m_ProjAngles;

	if ((m_lstDynLights.Last().m_Flags & DLF_SPECULAR_ONLY_FOR_HIGHSPEC) && (m_LightConfigSpec < CONFIG_HIGH_SPEC))
	{
		m_lstDynLights.Last().m_SpecColor = Col_Black;
	}
}

void C3DEngine::PrepareLightSourcesForRendering()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	// reset lists of lsource pointers in sectors
	if(m_pTerrain)
		m_pTerrain->ResetDLightMaskInSectors();

	//	GetRenderer()->EF_ClearLightsList();
	m_lstDynLightsNoLight.Clear();

	// update lmasks in terrain sectors
	if(m_pObjManager->m_nRenderStackLevel)
	{ // do not delete lsources during reqursion, becasue hmap lpasses are shared between levels
		for (int i=0; i<m_nRealLightsNum/*m_lstDynLights.Count()*/; i++)    
		{
			m_lstDynLights[i].m_Id = -1;
			GetRenderer()->EF_ADDDlight(&m_lstDynLights[i]);
			assert(m_lstDynLights[i].m_Id == i);
			if(m_lstDynLights[i].m_Id != -1)
				RegisterLightSourceInSectors(&m_lstDynLights[i]);
		}
	}
	else
	{
		for (int i=0; i<m_lstDynLights.Count(); i++)    
		{
			m_lstDynLights[i].m_Id = -1;
			/*
			if(i >= MAX_LIGHTS_NUM)
			{ // no more sources can be accepted by renderer
			Warning( 0,0,"C3DEngine::PrepareLightSourcesForRendering: No more than %d lsources allowed on the screen", (int)MAX_LIGHTS_NUM);
			FreeLightSourceComponents(&m_lstDynLights[i]);
			m_lstDynLights.Delete(i); i--;
			continue; 
			}
			*/
			if( m_lstDynLights[i].m_pOwner && m_lstDynLights[i].m_pOwner->GetEntityVisArea())
			{ // vis area lsource

				if(!GetViewCamera().IsSphereVisibleFast( Sphere(m_lstDynLights[i].m_Origin,m_lstDynLights[i].m_fRadius) ))
				{
					FreeLightSourceComponents(&m_lstDynLights[i]);
					m_lstDynLights.Delete(i); i--;
					continue; // invisible
				}

				// check if lsource is in visible area
				if(GetCVars()->e_dynamic_light_exact_vis_test)
					if(!m_lstDynLights[i].m_pOwner->IsEntityAreasVisible())
					{
						if(m_lstDynLights[i].m_Flags & DLF_THIS_AREA_ONLY)
						{
							IEntityRender * pEnt = m_lstDynLights[i].m_pOwner;
							if(pEnt->m_pVisArea)
							{
								int nRndFrameId = pEnt->m_pVisArea->m_nRndFrameId;
								if(GetFrameID() - pEnt->GetDrawFrame(0) > MAX_FRAME_ID_STEP_PER_FRAME)
								{
									FreeLightSourceComponents(&m_lstDynLights[i]);
									m_lstDynLights.Delete(i); i--;
									continue; // area invisible
								}
							}
							else
							{
								FreeLightSourceComponents(&m_lstDynLights[i]);
								m_lstDynLights.Delete(i); i--;
								continue; // area invisible
							}
						}
					}
			}
			else
			{ // outdoor lsource
				if( (!(m_lstDynLights[i].m_Flags & DLF_DIRECTIONAL)) &&
					!IsSphereVisibleOnTheScreen(m_lstDynLights[i].m_Origin, m_lstDynLights[i].m_fRadius, 0))
				{
					FreeLightSourceComponents(&m_lstDynLights[i]);
					m_lstDynLights.Delete(i); i--;
					continue; // outdoor invisible
				}
			}

			if(GetRenderer()->EF_IsFakeDLight(&m_lstDynLights[i]))
			{ // ignored by renderer
				m_lstDynLightsNoLight.Add(m_lstDynLights[i]);
				m_lstDynLights.Delete(i); i--;
				continue; 
			}

			GetRenderer()->EF_ADDDlight(&m_lstDynLights[i]);

			if(m_lstDynLights[i].m_Id == -1)
			{ // ignored by renderer

				assert(i >= MAX_LIGHTS_NUM);

				if(i >= MAX_LIGHTS_NUM)
				{ // no more sources can be accepted by renderer
					Warning( 0,0,"C3DEngine::PrepareLightSourcesForRendering: No more than %d real lsources allowed on the screen", (int)MAX_LIGHTS_NUM);
				}

				FreeLightSourceComponents(&m_lstDynLights[i]);
				m_lstDynLights.Delete(i); i--;
				continue; 
			}

			if (m_lstDynLights[i].m_pShader!=0 && (m_lstDynLights[i].m_pShader->GetLFlags() & LMF_DISABLE))
			{ // fake
				assert(0); // should not be called, but no problem
				FreeLightSourceComponents(&m_lstDynLights[i]);
				m_lstDynLights.Delete(i); i--;
				continue; 
			}

			if(i != m_lstDynLights[i].m_Id)
				Warning( 0,0,"C3DEngine::PrepareLightSourcesForRendering: Invalid light source id");

			if(m_lstDynLights[i].m_Flags & DLF_PROJECT 
				&& m_lstDynLights[i].m_fLightFrustumAngle<90 // actual fov is twice bigger
				&& m_lstDynLights[i].m_pLightImage
				&& !(m_lstDynLights[i].m_pLightImage->GetFlags2() & FT2_REPLICATETOALLSIDES)
				&& (m_lstDynLights[i].m_pLightImage->GetFlags2() & FT2_CUBEASSINGLETEXTURE))
			{ // prepare projector camera for frustum test
				CCamera & cam = m_arrCameraProjectors[i];
				cam.SetPos(m_lstDynLights[i].m_Origin);
				Vec3d Angles(m_lstDynLights[i].m_ProjAngles[1], 0, m_lstDynLights[i].m_ProjAngles[2]+90.0f);
				cam.SetAngle(Angles);
				cam.Init(1, 1, (m_lstDynLights[i].m_fLightFrustumAngle*2)/180.0f*PI, m_lstDynLights[i].m_fRadius, 1.0f, 0.1f);
				cam.Update();
			}

			if(m_lstDynLights[i].m_fRadius >= 0.5f)
			{
				assert(m_lstDynLights[i].m_fRadius >= 0.5f && !(m_lstDynLights[i].m_Flags & DLF_FAKE));
				RegisterLightSourceInSectors(&m_lstDynLights[i]);
			}
		}

		m_nRealLightsNum = m_lstDynLights.Count();
		m_lstDynLights.AddList(m_lstDynLightsNoLight);

		for(int i=m_nRealLightsNum; i<m_lstDynLights.Count(); i++)
		{ // ignored by renderer
			m_lstDynLights[i].m_Id = -1;
			GetRenderer()->EF_ADDDlight(&m_lstDynLights[i]);
			assert(m_lstDynLights[i].m_Id == -1);
		}

		m_lstDynLightsNoLight.Clear();
	}
}

void C3DEngine::FreeLightSourceComponents(CDLight *pLight)
{
	if(pLight->m_pCharInstance)
		pLight->m_pCharInstance->DetachLight(pLight);
	pLight->m_pCharInstance=0;

	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			if (pLight->m_pObject[i][j])
				pLight->m_pObject[i][j]->RemovePermanent();
			pLight->m_pObject[i][j]=0;
		}
	}

	// free leafbuffers
	for(int nLeafId=0; nLeafId<(sizeof(pLight->m_arrLightLeafBuffers)/sizeof(pLight->m_arrLightLeafBuffers[0])); nLeafId++)
		if(pLight->m_arrLightLeafBuffers[nLeafId])
		{
			GetRenderer()->DeleteLeafBuffer(pLight->m_arrLightLeafBuffers[nLeafId]);
			pLight->m_arrLightLeafBuffers[nLeafId] = 0;
		}

		//  delete pLight->m_pProjCamera;
		//pLight->m_pProjCamera=0;

		//if(pLight->m_pShader)
		//		SAFE_RELEASE(pLight->m_pShader);
}

void C3DEngine::UpdateLightSources()
{
	// process lsources
	float fCurrTime = GetTimer()->GetCurrTime();
	for(int i=0; i<m_lstDynLights.Count(); i++)
	{
		CDLight *pLight = &m_lstDynLights[i];

		if(pLight->m_fStartTime + pLight->m_fLifeTime+0.001f < fCurrTime)
		{
			// time is come to delete this lsource
			FreeLightSourceComponents(pLight);
			m_lstDynLights.Delete(i); i--;
			continue;
		}

		bool bDeleteNow = GetRenderer()->EF_UpdateDLight(pLight);
		//pLight->m_Color.Clamp();
		//pLight->m_SpecColor.Clamp();

		if(pLight->m_fLifeTime && !pLight->m_pShader)
		{ // evaluate radius
			float fAtten = 1.f - (fCurrTime - pLight->m_fStartTime) / pLight->m_fLifeTime;
			if(fAtten<0)
			{
				fAtten = 0;
				bDeleteNow = true;
			}
			else if(fAtten>1.f)
				fAtten = 1.f;
			pLight->m_fRadius = pLight->m_fBaseRadius*fAtten;
		}

		if(bDeleteNow)
		{
			// time is come to delete this lsource
			FreeLightSourceComponents(pLight);
			m_lstDynLights.Delete(i); i--;
			continue;
		}

		SetupLightScissors(&m_lstDynLights[i]);
	}
}

void C3DEngine::SetupLightScissors(CDLight * pLight)
{
	pLight->m_sX = 0;
	pLight->m_sY = 0;
	pLight->m_sWidth  = 0;
	pLight->m_sHeight = 0;
	return;

	Vec3d vLeft  = GetViewCamera().GetVCMatrixD3D9().GetOrtX();
	Vec3d vUp		 = GetViewCamera().GetVCMatrixD3D9().GetOrtY();
	Vec3d vFront = GetViewCamera().GetVCMatrixD3D9().GetOrtZ();

	float color[] = {1,1,1,1};
	float fRadius = (GetCVars()->e_scissor_debug>1) ? pLight->m_fRadius/GetCVars()->e_scissor_debug : pLight->m_fRadius;

	// get TopLeft2d
	Vec3d vTopLeft2d, vTopLeft = pLight->m_Origin + vUp*fRadius - vLeft*fRadius;
	GetRenderer()->ProjectToScreen(vTopLeft.x, vTopLeft.y, vTopLeft.z, &vTopLeft2d.x, &vTopLeft2d.y, &vTopLeft2d.z);
	vTopLeft2d.x = vTopLeft2d.x*GetRenderer()->GetWidth()/100;
	vTopLeft2d.y = vTopLeft2d.y*GetRenderer()->GetHeight()/100;
	if(vTopLeft2d.z<0 || vTopLeft2d.z>1.f)
		return;

	// get vBotRight2d
	Vec3d vBotRight2d, vBotRight = pLight->m_Origin - vUp*fRadius + vLeft*fRadius;
	GetRenderer()->ProjectToScreen(vBotRight.x, vBotRight.y, vBotRight.z, &vBotRight2d.x, &vBotRight2d.y, &vBotRight2d.z);
	vBotRight2d.x = vBotRight2d.x*GetRenderer()->GetWidth()/100;
	vBotRight2d.y = vBotRight2d.y*GetRenderer()->GetHeight()/100;
	if(vBotRight2d.z<0 || vBotRight2d.z>1.f)
		return;

	if(GetCVars()->e_scissor_debug)
	{
		/*		GetRenderer()->Draw2dLabel(vBotRight2d.x, vBotRight2d.y, 2 , color, false, "br");
		GetRenderer()->Draw2dLabel(vTopLeft2d.x,	vBotRight2d.y, 2 , color, false, "bl");
		GetRenderer()->Draw2dLabel(vBotRight2d.x, vTopLeft2d.y,	 2 , color, false, "tr");
		GetRenderer()->Draw2dLabel(vTopLeft2d.x,  vTopLeft2d.y,  2 , color, false, "tl");*/
	}

	// get vCenter2d
	/*if(GetCVars()->e_scissor_debug)
	{
	Vec3d vCenter2d, vCenter = pLight->m_Origin;
	GetRenderer()->ProjectToScreen(vCenter.x, vCenter.y, vCenter.z, &vCenter2d.x, &vCenter2d.y, &vCenter2d.z);
	vCenter2d.x = vCenter2d.x*GetRenderer()->GetWidth()/100;
	vCenter2d.y = vCenter2d.y*GetRenderer()->GetHeight()/100;
	GetRenderer()->Draw2dLabel(vCenter2d.x, vCenter2d.y, 2 , color, false, "x");
	}	*/

	// set 2d bounds
	/*pLight->m_sX = max(0,int(vTopLeft2d.x));
	pLight->m_sY = max(0,int(vTopLeft2d.y));
	pLight->m_sWidth  = min(GetRenderer()->GetWidth(), int(vBotRight2d.x - vTopLeft2d.x));
	pLight->m_sHeight = min(GetRenderer()->GetHeight(), int(vBotRight2d.y - vTopLeft2d.y));

	// set depth bounds
	float fDist = GetViewCamera().GetPlane(FR_PLANE_NEAR)->DistFromPlane(pLight->m_Origin);
	pLight->m_fNear = fDist - fRadius;
	pLight->m_fFar  = fDist + fRadius;
	*/

	pLight->m_sX = 0;
	pLight->m_sY = 0;
	pLight->m_sWidth  = 0;
	pLight->m_sHeight = 0;
}

void C3DEngine::LightSourcesDebug()
{
	if(!GetCVars()->e_dynamic_light_debug)
		return;

#ifndef PS2  
#ifndef _XBOX

	CCamera vCam = GetViewCamera();

	// add test dynamic source
	// don't panic, it's just for test
	static CDLight Light;
	static bool bFreeze = false;
	static Vec3d sAngs;
	static float sRad = 10.0f;
	if (true)
	{
#if !defined(LINUX)
		if ((GetAsyncKeyState('F') & 0x8000))
			bFreeze = 1;
		else
			if ((GetAsyncKeyState('U') & 0x8000))
				bFreeze = 0;
#endif      
		//Light.m_Orientation.m_vForward = Vec3d(1,0,0);
		//Light.m_Orientation.m_vUp = Vec3d(0,0,1);
		//Light.m_Orientation.m_vRight = Vec3d(0,1,0);

		if (!Light.m_pShader)
			Light.m_pShader = GetRenderer()->EF_LoadShader("GlowingMonkeyEyes", eSH_World, EF_SYSTEM);
#if !defined(LINUX)    
		if (GetAsyncKeyState(VK_DELETE) & 0x8000)
			sAngs[2] += 1.0f;
		if (GetAsyncKeyState(VK_NEXT) & 0x8000)
			sAngs[2] -= 1.0f;

		if (GetAsyncKeyState(VK_END) & 0x8000)
			sAngs[1] += 1.0f;
		if (GetAsyncKeyState(VK_HOME) & 0x8000)
			sAngs[1] -= 1.0f;

		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
			sAngs[0] -= 1.0f;
		if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
			sAngs[0] += 1.0f;

		if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
			sRad += 0.25f;
		if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
			sRad -= 0.25f;
#endif
		if (sRad < 0)
			sRad = 0;
		if (sRad > 100)
			sRad = 100;
#if !defined(LINUX)    
		if (GetAsyncKeyState(VK_INSERT) & 0x8000)
			Light.m_fLightFrustumAngle -= 1.0f;
		if (GetAsyncKeyState(VK_PRIOR) & 0x8000)
			Light.m_fLightFrustumAngle += 1.0f;
#endif    
		if (Light.m_fLightFrustumAngle < 1.0f)
			Light.m_fLightFrustumAngle = 1.0f;
		if (Light.m_fLightFrustumAngle > 88.0f)
			Light.m_fLightFrustumAngle = 88.0f;

		Light.m_ProjAngles = sAngs;
		//Light.m_Orientation.rotate(Vec3d(1,0,0), sAngs[0]);
		//Light.m_Orientation.rotate(Vec3d(0,1,0), sAngs[1]);
		//Light.m_Orientation.rotate(Vec3d(0,0,1), sAngs[2]);

		Light.m_Color = CFColor(1.0f,1.0f,1.0f, 1.0f);
		Light.m_SpecColor = CFColor(1.0f,1.0f,1.0f);
		//Light.m_AmbColor = CFColor(0.6f,0.6f,0.6f);
		if (!bFreeze)
		{
			Light.m_Origin = vCam.GetPos();
			//Light.m_Origin += Vec3d(1, 0, 0.0f);
		}
#if !defined(LINUX)
		if (GetAsyncKeyState('6') & 0x8000)
			Light.m_Flags = 0;
		else
			if (GetAsyncKeyState('7') & 0x8000)
				Light.m_Flags = DLF_DIRECTIONAL;
			else
				if (GetAsyncKeyState('P') & 0x8000)
					Light.m_Flags = DLF_PROJECT;
				else
					if (GetAsyncKeyState('O') & 0x8000)
						Light.m_Flags = DLF_POINT;
		Light.m_Flags |= DLF_HEATSOURCE | DLF_LIGHTSOURCE;
#endif      
		if (!(Light.m_Flags & DLF_LIGHTTYPE_MASK))
			return;

		if (Light.m_Flags & DLF_DIRECTIONAL)
			Light.m_fRadius = 10000.0f;
		else
			Light.m_fRadius = sRad;

		if (!Light.m_pLightImage)
		{
			//Light.m_pLightImage = m_IndInterface.m_pRenderer->EF_LoadTexture("FlashLightCube", 0, FT2_FORCECUBEMAP, eTT_Cubemap);
			Light.m_pLightImage = GetRenderer()->EF_LoadTexture("Textures/Lights/gk_spotlight_sm", 0, FT2_FORCECUBEMAP, eTT_Cubemap);
			Light.m_fAnimSpeed = 0.0f;
		}

		GetRenderer()->EF_UpdateDLight(&Light);

		AddDynamicLightSource(Light, (IEntityRender *)-1, 0);
	}
#endif      
#endif      
}

int __cdecl C3DEngine__Cmp_LightAmount(const void* v1, const void* v2)
{
	DLightAmount * p1 = ((DLightAmount*)v1);
	DLightAmount * p2 = ((DLightAmount*)v2);

	if(!p1 || !p2)
		return 0;

	if(p1->fAmount > p2->fAmount)
		return -1;
	else if(p1->fAmount < p2->fAmount)
		return 1;

	return 0;
}

uint C3DEngine::GetFullLightMask()
{
	uint nRes=0;
	for(int n=0; n<m_nRealLightsNum/*m_lstDynLights.Count()*/; n++)
	{
		const int nId = m_lstDynLights[n].m_Id;
		assert(nId>=0);
		nRes |= (1<<nId);
	}

	return nRes;
}

CDLight * C3DEngine::CheckDistancesToLightSources(uint & nDLightMask, 
																									const Vec3d vObjPos, const float fObjRadius, 
																									IEntityRender * pEntityRender, int nMaxLightBitsNum, 
																									CDLight ** pSelectedLights, int nMaxSelectedLights, 
																									Vec3d * pvSummLightAmmount )
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	m_lstDL_CDTLS.Clear();

	CVisArea * pEntityVisArea = (CVisArea *)(pEntityRender ? pEntityRender->GetEntityVisArea() : 0);

	// make list of all affecting light sources
	Vec3d vSummLightAmmount(0,0,0);
	int nProjectorsNum=0;
	for(int n=0; n<32 && n<m_nRealLightsNum/*m_lstDynLights.Count()*/; n++)
	{
		const int nId = m_lstDynLights[n].m_Id;

		// [marco] commented out 'cos it happens every frame 
		assert(nId == n);

		if(nId>=0 && nDLightMask & (1<<nId) && nId==n)
		{
			CDLight * pDLight = &m_lstDynLights[n];

			assert(!pEntityRender || pDLight == (CDLight*)GetRenderer()->EF_Query(EFQ_LightSource, nId));
			assert(pDLight->m_Id == n);
			assert(!pDLight->m_pShader || !(pDLight->m_pShader->GetLFlags() & LMF_DISABLE));

			// todo: remove another similar check 
			if(pEntityRender && pDLight->m_Flags & DLF_THIS_AREA_ONLY)
				if(pDLight->m_pOwner && pDLight->m_pOwner->GetEntityVisArea())
					if(pEntityRender->GetEntityVisArea() != pDLight->m_pOwner->GetEntityVisArea())
						if(!pEntityRender->GetEntityVisArea() || !pEntityRender->GetEntityVisArea()->IsPortal())
							//							if(!(pEntityRender->GetRndFlags() & ERF_DONOTCHECKVIS)) // not for 1p weapons
							continue; // different areas

			if(pDLight->m_Flags & DLF_IGNORE_OWNER && pEntityRender && pEntityRender == pDLight->m_pOwner)
				continue; // skip this object lights (vehicle lights)

			if ((pDLight->m_Flags & DLF_ONLY_FOR_HIGHSPEC) && (m_LightConfigSpec < CONFIG_HIGH_SPEC))
				continue; // Skip high spec only light on medium and low spec configs.

			if(pEntityRender && 
				pDLight->m_Flags & DLF_LM && 
				!(pDLight->m_Flags & DLF_CASTSHADOW_VOLUME) &&
				pEntityRender->GetRndFlags() & ERF_USELIGHTMAPS && 
				pEntityRender->HasLightmap(0))
			{ // in case of lightmaps
				if(pDLight->m_Flags & DLF_PROJECT)
					continue; // ignore specular only lights if projector since specular projectors not supported

				if (!(pDLight->m_Flags & DLF_CASTSHADOW_MAPS))
				{
					if ((pDLight->m_SpecColor == Col_Black) ||
						((pDLight->m_Flags & DLF_SPECULAR_ONLY_FOR_HIGHSPEC) && (m_LightConfigSpec < CONFIG_HIGH_SPEC)))
						continue; // ignore specular only lights if specular disabled
				}
			}

			if(!pEntityRender && (pDLight->m_Flags & DLF_THIS_AREA_ONLY))
				if(	pDLight->m_pOwner && pDLight->m_pOwner->GetEntityVisArea())
					continue; // indoor lsource do not affect on vegetations

			float fProjectorAmmountPlus = 0;
			if(pDLight->m_Flags & DLF_PROJECT && pEntityRender 
				&& pDLight->m_fLightFrustumAngle<90 // actual fov is twice bigger
				&& !(pDLight->m_pLightImage!=0 && pDLight->m_pLightImage->GetFlags2() & FT2_REPLICATETOALLSIDES))
			{ // check projector frustum
				// use pDLight->m_TextureMatrix to construct Plane
				/*GetRenderer()->Draw3dBBox(pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtX(), DPRIM_LINE);
				GetRenderer()->DrawLabel(pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtX(),1,"x");
				GetRenderer()->Draw3dBBox(pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtY(), DPRIM_LINE);
				GetRenderer()->DrawLabel(pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtY(),1,"y");
				GetRenderer()->Draw3dBBox(pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtZ(), DPRIM_LINE);
				GetRenderer()->DrawLabel(pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtZ(),1,"z");*/					

				if(GetCVars()->e_projector_exact_test)
				{ 
					CCamera & cam = m_arrCameraProjectors[n];
					Vec3d vBoxMin,vBoxMax;
					pEntityRender->GetRenderBBox(vBoxMin,vBoxMax);
					if (!cam.IsAABBVisibleFast( AABB(vBoxMin,vBoxMax) ))
						continue;
				}
				else
				{
					Plane p;
					p.CalcPlane( pDLight->m_Origin, pDLight->m_Origin+pDLight->m_TextureMatrix.GetOrtY(), pDLight->m_Origin-pDLight->m_TextureMatrix.GetOrtZ() );
					if( p.DistFromPlane(vObjPos) + fObjRadius < 0 )
						continue;
				}
				fProjectorAmmountPlus = 10.f;
				nProjectorsNum++;
			}
			else if(pEntityRender)
			{ // check sphere/bbox intersection
				Vec3d vBoxMin,vBoxMax;
				pEntityRender->GetBBox(vBoxMin,vBoxMax);
				if(!Overlap::Sphere_AABB(Sphere(pDLight->m_Origin, pDLight->m_fRadius), AABB(vBoxMin,vBoxMax)))
					continue;
			}

			// find amount of light
			float fDist = GetDistance(pDLight->m_Origin,vObjPos);
			float fLightAttenuation = (pDLight->m_Flags & DLF_DIRECTIONAL) ? 1.f : 1.f - (fDist-fObjRadius) / (pDLight->m_fRadius);
			if(fLightAttenuation<0)
				fLightAttenuation=0;

			float fLightAmount = 
				(pDLight->m_Color.r+pDLight->m_Color.g+pDLight->m_Color.b)*0.233f + 
				(pDLight->m_SpecColor.r+pDLight->m_SpecColor.g+pDLight->m_SpecColor.b)*0.1f;
			fLightAmount = fLightAmount*fLightAttenuation + fProjectorAmmountPlus;

			if(fLightAmount>0.05f)
			{
				// if entity is inside some area - allow lightsources only from this area
				if(pEntityRender)
				{
					if(pEntityVisArea)
					{
						if(	pDLight->m_pOwner && pDLight->m_pOwner->GetEntityRS() && 
							pDLight->m_pOwner->m_pVisArea)
						{
							CVisArea * pLightArea = pDLight->m_pOwner->m_pVisArea;
							if(pEntityVisArea != pLightArea && !(pEntityRender->GetRndFlags() & ERF_DONOTCHECKVIS))
							{	// try also neighbor volumes
								int nSearchDepth;
								if(pDLight->m_Flags & DLF_PROJECT && !(pDLight->m_Flags & DLF_THIS_AREA_ONLY))
									nSearchDepth = 4 + int(pLightArea->IsPortal()); // allow projector to go depther
								else
									nSearchDepth = 1 + int((pDLight->m_Flags & DLF_THIS_AREA_ONLY)==0) + int(pLightArea->IsPortal());

								bool bNearFound = pEntityVisArea->FindVisArea(pLightArea, nSearchDepth, true);
								if(!bNearFound)
									continue; // areas do not much

								// construct frustum from light pos and portal, check that object is visible from light position
								if(!(pDLight->m_Flags & DLF_THIS_AREA_ONLY) && pEntityVisArea && !pEntityVisArea->IsPortal())
								{
									Vec3d vBoxMin,vBoxMax;
									pEntityRender->GetBBox(vBoxMin,vBoxMax);
									AABB aabbReceiver(vBoxMin,vBoxMax);
									Shadowvolume sv;
									int p=0;
									for(; p<pEntityVisArea->m_lstConnections.Count(); p++)
									{
										CVisArea * pArea = pEntityVisArea->m_lstConnections[p];
										if(pArea->IsPortal())
										{
											AABB aabbCaster(pArea->m_vBoxMin, pArea->m_vBoxMax);
											NAABB_SV::AABB_ShadowVolume(pDLight->m_Origin, aabbCaster, sv, pDLight->m_fRadius);
											bool bIntersect = NAABB_SV::Is_AABB_In_ShadowVolume(sv, aabbReceiver);
											if(bIntersect)
												break;
										}
									}
									if(p==pEntityVisArea->m_lstConnections.Count())
										continue; // object is not visible from light position
								}
							}
						}
						else if(!pEntityVisArea->m_bAfectedByOutLights)
							continue; // outdoor lsource
					}
					else // entity is outside
						if(	pDLight->m_pOwner && pDLight->m_pOwner->GetEntityVisArea())
							continue; // indoor lsource should not affect outdoor entity
				}

				DLightAmount la;
				la.pDLight = pDLight;
				la.fAmount = fLightAmount;
				vSummLightAmmount += Vec3d(pDLight->m_Color.r,pDLight->m_Color.g,pDLight->m_Color.b)*fLightAttenuation;
				m_lstDL_CDTLS.Add(la);
			}
		}
	}

	nDLightMask = 0;

	if(!m_lstDL_CDTLS.Count())
		return 0; // no lsources found

	// sort by light amount
	qsort(&m_lstDL_CDTLS[0], m_lstDL_CDTLS.Count(), sizeof(m_lstDL_CDTLS[0]), C3DEngine__Cmp_LightAmount);

	nMaxLightBitsNum = min(nMaxLightBitsNum,GetCVars()->e_max_entity_lights+nProjectorsNum);
	//	if(pEntityRender && pEntityRender->GetRndFlags() & ERF_USELIGHTMAPS && pEntityRender->HasLightmap(0))
	//	nMaxLightBitsNum--;

	// increate max lights num for big objects
	if(pEntityRender && pEntityRender->m_fWSRadius>8 && nMaxLightBitsNum)
		nMaxLightBitsNum++;

	// limit number of effective light sources
	CDLight * pStrongestShadowCaster = NULL;
	for(int n=0; n<nMaxLightBitsNum && n<m_lstDL_CDTLS.Count(); n++)
	{
		const int nId = m_lstDL_CDTLS[n].pDLight->m_Id;

		nDLightMask |= (1<<nId);

		if(pSelectedLights && n<nMaxSelectedLights)
			pSelectedLights[n] = m_lstDL_CDTLS[n].pDLight;

		// always search for shadow map caster even if shadow volume caster is already selected
		if((!pStrongestShadowCaster || pStrongestShadowCaster->m_Flags & DLF_CASTSHADOW_VOLUME) && m_lstDL_CDTLS[n].pDLight->m_Flags & DLF_CASTSHADOW_MAPS)
			pStrongestShadowCaster = m_lstDL_CDTLS[n].pDLight;
		else if(!pStrongestShadowCaster && m_lstDL_CDTLS[n].pDLight->m_Flags & DLF_CASTSHADOW_VOLUME)
			pStrongestShadowCaster = m_lstDL_CDTLS[n].pDLight;
	}

	if(pvSummLightAmmount)
		*pvSummLightAmmount = vSummLightAmmount;

	return pStrongestShadowCaster ? 
pStrongestShadowCaster : ((nMaxLightBitsNum>0) ? m_lstDL_CDTLS[0].pDLight : 0); // return strongest
}

void C3DEngine::RemoveEntityLightSources(IEntityRender * pEntity)
{
	for (int i=0; i<m_lstDynLights.Count(); i++)    
	{
		if(m_lstDynLights[i].m_pOwner == pEntity)
		{
			FreeLightSourceComponents(&m_lstDynLights[i]);
			m_lstDynLights.Delete(i);
			i--;
		}
	}
	/*
	for (int i=0; i<m_lstStaticLights.Count(); i++)    
	{
	if(m_lstStaticLights[i] == pEntity && !pEntity->IsStatic())
	{
	CLightEntity * pLightEntity = m_lstStaticLights[i];
	m_lstStaticLights.Delete(i);
	delete pLightEntity;
	i--;
	}
	}*/
}

typedef LMStatLightFileHeader LightFileHeader;

bool ReadString(FILE * hFile, char * szBuff, int nMaxChars, ICryPak * pPak)
{
	UINT iStrLen = 0;
	if (pPak->FRead(&iStrLen, sizeof(UINT), 1, hFile) != 1)
		return false;

	if((int)iStrLen >= nMaxChars)
		return false;

	if (pPak->FRead(szBuff, 1, iStrLen, hFile) != iStrLen)
		return false;

	szBuff[iStrLen]=0;

	return true;
}

#ifdef WIN64
#pragma pack(push,4)
// dummy structure that's binary compatible with the 32-bit version of CDLight
struct CDLightDummy32
{
	typedef int ptr32;

	int															m_Id;
	Vec3														m_Origin;					 //world space position
	Vec3														m_BaseOrigin;					 //world space position
	CFColor													m_Color;									//!< clampled diffuse light color
	CFColor													m_BaseColor;									//!< clampled diffuse light color
	CFColor 												m_SpecColor;
	CFColor 												m_BaseSpecColor;
	Vec3														m_vObjectSpacePos;		 //Object space position
	float														m_fRadius;
	float														m_fBaseRadius;
	float   												m_fDirectFactor;
	float														m_fStartRadius;
	float														m_fEndRadius;
	float														m_fLastTime;
	int     												m_NumCM;

	// Scissor parameters (2d extent)
	short   					m_sX;
	short   					m_sY;
	short   					m_sWidth;
	short   					m_sHeight;
	// Far/near planes
	float							m_fNear;
	float							m_fFar;

	ptr32					m_pOwner;

	//for static spot light sources casting volumetric shadows	
	int m_nReserved; // compensates for the vtbl
	COrthoNormalBasis								m_Orientation;
	int															m_CustomTextureId;
	Matrix44												m_TextureMatrix;

	ptr32											m_pObject[4][4];								//!< Object for light coronas and light flares

	//the light image
	ptr32												m_pLightImage;
	float														m_fLightFrustumAngle;
	float														m_fBaseLightFrustumAngle;
	float                           m_fAnimSpeed;

	ptr32												m_pShader;
	Vec3														m_ProjAngles;
	Vec3														m_BaseProjAngles;

	uint														m_Flags;									//!< flags from above (prefix DLF_)

	char														m_Name[64];
	int                             m_nLightStyle;
	float                           m_fCoronaScale;

	float														m_fStartTime;
	float														m_fLifeTime;							//!< lsource will be removed after this number of seconds

	char														m_sDebugName[8];					//!< name of light creator (for debuging, pointer can't be used since entity may be deleted)

	ptr32 m_pShadowMapLightSource;	//!<

	ptr32										m_arrLightLeafBuffers[8]; //!< array of leafbuffers used for heightmap lighting pass

	int															m_nEntityLightId;					//!<
	int															m_nFrameID;								//!<

	ptr32 m_pCharInstance; // pointer to character this source is attached to

	void copyTo(CDLight& dl)
	{
		memset(&dl, 0, sizeof(dl));
#define COPY(x) dl.x=x
#define MEMCOPY(x) memcpy (dl.x,x,sizeof(x))
		COPY(m_Id);
		COPY(m_Origin);					 //world space position
		COPY(m_BaseOrigin);					 //world space position
		COPY(m_Color);									//!< clampled diffuse light color
		COPY(m_BaseColor);									//!< clampled diffuse light color
		COPY(m_SpecColor);
		COPY(m_BaseSpecColor);
		COPY(m_vObjectSpacePos);		 //Object space position
		COPY(m_fRadius);
		COPY(m_fBaseRadius);
		COPY(m_fDirectFactor);
		COPY(m_fStartRadius);
		COPY(m_fEndRadius);
		COPY(m_fLastTime);
		COPY(m_NumCM);

		// Scissor parameters (2d extent)
		COPY(m_sX);
		COPY(m_sY);
		COPY(m_sWidth);
		COPY(m_sHeight);
		// Far/near planes
		COPY(m_fNear);
		COPY(m_fFar);

		COPY(m_Orientation);
		COPY(m_CustomTextureId);
		COPY(m_TextureMatrix);

		COPY(m_fLightFrustumAngle);
		COPY(m_fBaseLightFrustumAngle);
		COPY(m_fAnimSpeed);

		COPY(m_ProjAngles);
		COPY(m_BaseProjAngles);

		COPY(m_Flags);									//!< flags from above (prefix DLF_)

		MEMCOPY(m_Name);
		COPY(m_nLightStyle);
		COPY(m_fCoronaScale);

		COPY(m_fStartTime);
		COPY(m_fLifeTime);							//!< lsource will be removed after this number of seconds

		MEMCOPY(m_sDebugName);					//!< name of light creator (for debuging, pointer can't be used since entity may be deleted)

		COPY(m_nEntityLightId);
		COPY(m_nFrameID);
#undef MEMCOPY
#undef COPY
	}
};

bool C3DEngine::LoadStaticLightSources (const char *pszFileName)
{
	LightFileHeader sHeader, sHdrCmp;

	FILE * hFile = GetSystem()->GetIPak()->FOpen(pszFileName, "rb");

	if (hFile == NULL)
		return false;

	if (GetSystem()->GetIPak()->FRead(&sHeader, sizeof(LightFileHeader), 1, hFile) != 1)
	{
		GetSystem()->GetIPak()->FClose(hFile);
		return false;
	}

	CDLightDummy32 dummy;
	if (sHeader.iNumDLights == 0 || 
		sHeader.iVersion != 0 || 
		sHeader.iSizeOfDLight != sizeof(CDLightDummy32))
	{
		GetConsole()->Exit ("C3DEngine::LoadStaticLightSources: StatLights.dat version error (%d,%d). Please reexport using latest version of 32-bit editor.",
			sHeader.iSizeOfDLight, sizeof(dummy));
		return false;
	}

	m_lstStaticLights.Reset();
	m_lstStaticLights.PreAllocate(sHeader.iNumDLights);

	for (UINT iCurLight=0; iCurLight<sHeader.iNumDLights; iCurLight++)
	{
		CDLight newLight;
		CDLightDummy32 dummy32Light;
		if (GetSystem()->GetIPak()->FRead(&dummy32Light, sizeof(dummy32Light), 1, hFile) != 1)
		{
			GetSystem()->GetIPak()->FClose(hFile);
			return false;
		}
		dummy32Light.copyTo(newLight);

		newLight.m_pLightImage = 0;
		newLight.m_pCharInstance = 0;
		newLight.m_pOwner = 0;
		newLight.m_pShader = 0;

		int nTextureFlags2 = 0;
		if (GetSystem()->GetIPak()->FRead(&nTextureFlags2, sizeof(int), 1, hFile) != 1)
		{
			GetSystem()->GetIPak()->FClose(hFile);
			return false;
		}

		char szTextureName[MAX_PATH_LENGTH]="";
		bool b0 = ReadString(hFile, szTextureName, MAX_PATH_LENGTH, GetSystem()->GetIPak());

		char szShaderName[MAX_PATH_LENGTH]="";
		bool b1 = ReadString(hFile, szShaderName, MAX_PATH_LENGTH, GetSystem()->GetIPak());

		//    bool b(nTextureFlags2&FT2_FORCECUBEMAP);

		if(szTextureName[0])
			newLight.m_pLightImage = GetRenderer()->EF_LoadTexture(szTextureName, 0, nTextureFlags2/*FT2_FORCECUBEMAP*/, eTT_Cubemap);

		if(szShaderName[0])
			newLight.m_pShader = GetRenderer()->EF_LoadShader(szShaderName, eSH_World, EF_SYSTEM);

		AddStaticLightSource(newLight,0,0,0);
	}

	GetSystem()->GetIPak()->FClose(hFile);

	return true;
}

#else

bool C3DEngine::LoadStaticLightSources(const char *pszFileName)
{
	LightFileHeader sHeader, sHdrCmp;

	FILE * hFile = GetSystem()->GetIPak()->FOpen(pszFileName, "rb");

	if (hFile == NULL)
		return false;

	if (GetSystem()->GetIPak()->FRead(&sHeader, sizeof(LightFileHeader), 1, hFile) != 1)
	{
		GetSystem()->GetIPak()->FClose(hFile);
		return false;
	}

	CDLight dummy;

	if (sHeader.iNumDLights == 0 || 
		sHeader.iVersion != sHdrCmp.iVersion || 
		sHeader.iSizeOfDLight != sHdrCmp.iSizeOfDLight)
	{
		GetConsole()->Exit("C3DEngine::LoadStaticLightSources: StatLights.dat version error, please reexport using latest version of editor.");
		return false;
	}

	m_lstStaticLights.Reset();
	m_lstStaticLights.PreAllocate(sHeader.iNumDLights);

	for (UINT iCurLight=0; iCurLight<sHeader.iNumDLights; iCurLight++)
	{
		CDLight newLight;
		if (GetSystem()->GetIPak()->FRead(&newLight, sizeof(CDLight), 1, hFile) != 1)
		{
			GetSystem()->GetIPak()->FClose(hFile);
			return false;
		}

		newLight.m_pLightImage = 0;
		newLight.m_pCharInstance = 0;
		newLight.m_pOwner = 0;
		newLight.m_pShader = 0;

		int nTextureFlags2 = 0;
		if (GetSystem()->GetIPak()->FRead(&nTextureFlags2, sizeof(int), 1, hFile) != 1)
		{
			GetSystem()->GetIPak()->FClose(hFile);
			return false;
		}

		char szTextureName[MAX_PATH_LENGTH]="";
		bool b0 = ReadString(hFile, szTextureName, MAX_PATH_LENGTH, GetSystem()->GetIPak());

		char szShaderName[MAX_PATH_LENGTH]="";
		bool b1 = ReadString(hFile, szShaderName, MAX_PATH_LENGTH, GetSystem()->GetIPak());

		//    bool b(nTextureFlags2&FT2_FORCECUBEMAP);

		if(szTextureName[0])
			newLight.m_pLightImage = GetRenderer()->EF_LoadTexture(szTextureName, 0, nTextureFlags2/*FT2_FORCECUBEMAP*/, eTT_Cubemap);

		if(szShaderName[0])
			newLight.m_pShader = GetRenderer()->EF_LoadShader(szShaderName, eSH_World, EF_SYSTEM);

		AddStaticLightSource(newLight,0,0,0);
	}

	GetSystem()->GetIPak()->FClose(hFile);

	return true;
}
#endif
