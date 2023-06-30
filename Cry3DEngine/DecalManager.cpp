////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   decals.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: draw, create decals on the world
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "DecalManager.h"
#include "3dengine.h"
#include <IStatObj.h>
#include "ObjMan.h"

#define COMPLEX_DECAL_MIN_SIZE 0.1f

CDecalManager::CDecalManager(C3DEngine * p3DEngine) 
{ 
  m_nCurDecal = 0; 
  memset(m_arrbActiveDecals,0,sizeof(m_arrbActiveDecals)); 
  m_p3DEngine = p3DEngine;
  m_pShader_ParticleLight = GetRenderer()->EF_LoadShader("ParticleLight", eSH_World, EF_SYSTEM);
	m_pShader_Decal_VP = GetRenderer()->EF_LoadShader("Decal_VP", eSH_World, EF_SYSTEM);
	m_pShader_Decal_2D_VP = GetRenderer()->EF_LoadShader("Decal_2D_VP", eSH_World, EF_SYSTEM);
}

// called from the renderer, draws immideately
void CDecalManager::DrawBigDecalsOnTerrain()
{ 
  if(!GetCVars()->e_decals)
    return;

  //GetRenderer()->ResetToDefault();

  GetRenderer()->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
  GetRenderer()->SetColorOp(eCO_MODULATE, eCO_MODULATE, eCA_Texture|(eCA_Constant<<3), eCA_Texture|(eCA_Constant<<3));
  GetRenderer()->SetCullMode(R_CULL_DISABLE);

  Vec3d vColor = GetSystem()->GetI3DEngine()->GetWorldColor();//*

  GetRenderer()->SetMaterialColor(vColor.x,vColor.y,vColor.z,1);

	float fCurrTime = GetTimer()->GetCurrTime();

  for(int i=0; i<DECAL_COUNT; i++)
  if(m_arrbActiveDecals[i])
  {
    if(!m_arrDecals[i].m_pStatObj && m_arrDecals[i].m_bOnTheGround)
    if( GetViewCamera().IsSphereVisibleFast( Sphere(m_arrDecals[i].m_vPos, m_arrDecals[i].m_fSize)) )
      m_arrDecals[i].DrawBigDecalOnTerrain(m_p3DEngine, GetRenderer(), fCurrTime);
  }

  //GetRenderer()->ResetToDefault();
}

bool CDecalManager::AdjustDecalPosition( CryEngineDecalInfo & DecalInfo, bool bMakeFatTest )
{
	Matrix44 objMat, objMatInv;
	int nPartID = DecalInfo.nPartID;
	IStatObj * pEntObject = DecalInfo.pDecalOwner->GetEntityStatObj(nPartID, &objMat, true);	
	while(!pEntObject && nPartID<16)
	{ // find first visible entity component
		pEntObject = DecalInfo.pDecalOwner->GetEntityStatObj(++nPartID, &objMat, true);
	}
	
	if(!pEntObject || !pEntObject->GetLeafBuffer()|| nPartID>=16)
		return false;

	// transform decal into object space 
	objMatInv = objMat;
	objMatInv.Invert44();

	// put into normal object space hit direction of projection
	Vec3d vObjSpaceHitDirection = objMatInv.TransformVectorOLD(DecalInfo.vHitDirection);			

	// put into position object space hit position
	Vec3d vObjSpacePos = objMatInv.TransformPointOLD(DecalInfo.vPos);

	// get decal directions
	Vec3d vRi(0,0,0), vUp(0,0,0);
	if(fabs(vObjSpaceHitDirection.Dot(Vec3d(0,0,1)))>0.999f)
	{ // horiz surface
		vRi = Vec3d(0,1,0);
		vUp = Vec3d(1,0,0);
	}
	else
	{
		vRi = vObjSpaceHitDirection.Cross(Vec3d(0,0,1));
		vRi.Normalize();
		vUp    = vObjSpaceHitDirection.Cross(vRi);
		vUp.Normalize();
	}

	vRi*=DecalInfo.fSize;
	vUp*=DecalInfo.fSize;


	Vec3d vObjSpaceOutPos(0,0,0), vObjSpaceOutNormal(0,0,0), vTmp;
	CLeafBuffer * pLB = pEntObject->GetLeafBuffer();
	if(	pLB && pLB->m_pSecVertBuffer && pLB->m_SecVertCount &&
		RayLeafBufferIntersection(pLB, vObjSpacePos, vObjSpaceHitDirection*0.1f, vObjSpaceOutPos, vObjSpaceOutNormal) )
	{ // now check that none of decal sides will not fly in the air
		Vec3d vDecalNormal = vObjSpaceOutNormal*DecalInfo.fSize;
		if(	!bMakeFatTest || (
				RayLeafBufferIntersection(pLB, vObjSpaceOutPos+vUp, -vDecalNormal, vTmp, vTmp) &&
				RayLeafBufferIntersection(pLB, vObjSpaceOutPos-vUp, -vDecalNormal, vTmp, vTmp) &&
				RayLeafBufferIntersection(pLB, vObjSpaceOutPos+vRi, -vDecalNormal, vTmp, vTmp) &&
				RayLeafBufferIntersection(pLB, vObjSpaceOutPos-vRi, -vDecalNormal, vTmp, vTmp) ))
		{
			DecalInfo.vPos = objMat.TransformPointOLD(vObjSpaceOutPos);
			DecalInfo.vNormal = objMat.TransformVectorOLD(vObjSpaceOutNormal);
			return true;
		}
	}

	return false;
}

struct HitPosInfo {
	HitPosInfo() { memset(this,0,sizeof(HitPosInfo)); }
	Vec3d vPos,vNormal;
	float fDistance;
};

int __cdecl CDecalManager__CmpHitPos(const void* v1, const void* v2)
{
	HitPosInfo * p1 = (HitPosInfo*)v1;
	HitPosInfo * p2 = (HitPosInfo*)v2;

	if(p1->fDistance > p2->fDistance)
		return 1;
	else if(p1->fDistance < p2->fDistance)
		return -1;

	return 0;
}

bool CDecalManager::RayLeafBufferIntersection(CLeafBuffer * pLeafBuffer, const Vec3d & vInPos, const Vec3d & vInDir, Vec3d & vOutPos, Vec3d & vOutNormal)
{
	// get position offset and stride
	int nPosStride = 0;
	byte * pPos  = pLeafBuffer->GetPosPtr(nPosStride, 0, true);

	// get indices
	ushort *pInds = pLeafBuffer->GetIndices(0);
	int nInds = pLeafBuffer->m_Indices.m_nItems;
	assert(nInds%3 == 0);

	HitPosInfo arrResult[16];
	memset(arrResult,0,sizeof(arrResult));
	int nResultCount=0;

	// find faces
	for(int i=0; i<nInds && nResultCount<16; i+=3)
	{
		// get tri vertices
		Vec3d v0 = *((Vec3d*)&pPos[nPosStride*pInds[i+0]]);
		Vec3d v1 = *((Vec3d*)&pPos[nPosStride*pInds[i+1]]);
		Vec3d v2 = *((Vec3d*)&pPos[nPosStride*pInds[i+2]]);

		// make line triangle intersection
		Vec3d vResPos(0,0,0);
		if(	Intersect::Lineseg_Triangle(Lineseg(vInPos+vInDir, vInPos-vInDir), v0, v1, v2, vResPos) ||
				Intersect::Lineseg_Triangle(Lineseg(vInPos-vInDir, vInPos+vInDir), v0, v1, v2, vResPos) )
		{
			arrResult[nResultCount].vPos = vResPos;
			arrResult[nResultCount].vNormal = (v1-v0).Cross(v2-v0);
			arrResult[nResultCount].vNormal.Normalize();
			arrResult[nResultCount].fDistance = (vInPos-vInDir).GetDistance(vResPos);
			nResultCount++;
		}
	}

	if(nResultCount)
	{ // return closest to the shooter
		qsort(&arrResult, nResultCount, sizeof(arrResult[0]), CDecalManager__CmpHitPos);
		vOutPos = arrResult[0].vPos;
		vOutNormal = arrResult[0].vNormal;
	}

	return nResultCount!=0;
}

void CDecalManager::Spawn( CryEngineDecalInfo DecalInfo, float fMaxViewDistance, CTerrain * pTerrain )
{
  Vec3d vCamPos = GetViewCamera().GetPos();

//	DecalInfo.fLifeTime = 1000000000;
//	DecalInfo.fSize = .5f;
//	DecalInfo.m_fGrowTime = 4.f;
//	DecalInfo.nPartID = -1;

	// do not spawn if too far
	float fZoom = m_p3DEngine->GetObjManager() ? m_p3DEngine->GetObjManager()->m_fZoomFactor : 1.f;
	float fDecalDistance = L1Distance2D(DecalInfo.vPos, vCamPos);
  if( fDecalDistance>fMaxViewDistance || fDecalDistance*fZoom>DecalInfo.fSize*ENTITY_DECAL_DIST_FACTOR*3.f )
    return;

/*	if(DecalInfo.pDecalOwner && DecalInfo.pDecalOwner->GetEntityRenderType() == eERType_Vegetation)
	{
		CStatObjInst * pStatObjInst = (CStatObjInst*)DecalInfo.pDecalOwner;
		if(pStatObjInst->m_fFinalBending)
			return; // do not spawn on bendable vegetations
	}*/

	// project decal to visible geoemtry
	if(DecalInfo.bAdjustPos && DecalInfo.nPartID>=0 && DecalInfo.pDecalOwner && !AdjustDecalPosition( DecalInfo, DecalInfo.fSize<=COMPLEX_DECAL_MIN_SIZE ))
		return;

	// clamp all decal textures
	GetRenderer()->SetTexture(DecalInfo.nTid);
	GetRenderer()->SetTexClampMode(true);

	// don't go crazy
	if(DecalInfo.fSize > 4)
		DecalInfo.fSize = 4;

	if(DecalInfo.fSize > 1 && GetCVars()->e_decals_neighbor_max_life_time)
	{ // force near decals to fade faster
		float fCurrTime = GetTimer()->GetCurrTime();
		for(int i=0; i<DECAL_COUNT; i++)
		if(m_arrbActiveDecals[i])
		{
			if(m_arrDecals[i].m_vWSPos.GetDistance(DecalInfo.vPos) < m_arrDecals[i].m_fWSSize+DecalInfo.fSize)
				if((m_arrDecals[i]).m_fLifeBeginTime < fCurrTime-0.1f)
					if(m_arrDecals[i].m_fLifeEndTime > fCurrTime+GetCVars()->e_decals_neighbor_max_life_time)
						m_arrDecals[i].m_fLifeEndTime = fCurrTime+GetCVars()->e_decals_neighbor_max_life_time;
		}
	}

	// loop position in array
  if(m_nCurDecal>=DECAL_COUNT)
    m_nCurDecal=0;

	// free old LB
	GetRenderer()->DeleteLeafBuffer(m_arrDecals[m_nCurDecal].m_pBigDecalLeafBuffer);
	m_arrDecals[m_nCurDecal].m_pBigDecalLeafBuffer=0;

	// just in case
  DecalInfo.vNormal.Normalize();

	bool bBigDecalOnTheGround = false;

	// remember object we need to follow
	m_arrDecals[m_nCurDecal].m_nDecalOwnerComponentId = DecalInfo.nPartID;

	m_arrDecals[m_nCurDecal].m_vWSPos = DecalInfo.vPos;
	m_arrDecals[m_nCurDecal].m_fWSSize = DecalInfo.fSize;

	// If owner entity and object is specified - make decal use entity geometry
	float fObjScale = 1.f;
	if(DecalInfo.pDecalOwner && DecalInfo.nPartID>=0 && DecalInfo.fSize>COMPLEX_DECAL_MIN_SIZE)
	{ // get object pointer
		Matrix44 objMat;
		IStatObj * pEntObject = DecalInfo.pDecalOwner->GetEntityStatObj(DecalInfo.nPartID, &objMat);
		if(pEntObject)
		{
			CLeafBuffer * pSourceLeafBuffer = pEntObject->GetLeafBuffer();

			// transform decal into object space 
			objMat.Invert44();

			// put into normal object space hit direction of projection
			DecalInfo.vNormal = -objMat.TransformVectorOLD(DecalInfo.vHitDirection);			

			// put into position object space hit position
			DecalInfo.vPos = objMat.TransformPointOLD(DecalInfo.vPos);

			// find object scale
			if(DecalInfo.pDecalOwner->GetEntityRenderType()==eERType_Vegetation)
				fObjScale = DecalInfo.pDecalOwner->GetScale();
			else
			{
				Vec3d vTest(0,0,1.f);
				vTest = objMat.TransformVectorOLD(vTest);
				fObjScale = 1.f/vTest.len();
			}

			if(fObjScale<0.01f)
				return;

			// transform size into object space
			DecalInfo.fSize/=fObjScale;

			// make decal geometry
			m_arrDecals[m_nCurDecal].m_pBigDecalLeafBuffer = MakeBigDecalLeafBuffer(pSourceLeafBuffer, DecalInfo.vPos, 
				DecalInfo.fSize, DecalInfo.vNormal, DecalInfo.nTid);

			if(!m_arrDecals[m_nCurDecal].m_pBigDecalLeafBuffer)
				return; // no geometry found

			ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(DecalInfo.nTid);
			if(pTexPic)
			{
				pTexPic->SetFilter(FILTER_LINEAR);
				pTexPic->SetClamp(true);
			}
		}		 
		else
		{
			Warning( 0,0,"CDecalManager::Spawn: nPartID points to empty object"); 
			return;
		}
	}	
	else if(DecalInfo.pDecalOwner && 
		(!DecalInfo.pDecalOwner->IsStatic() || DecalInfo.pDecalOwner->GetEntityRenderType() == eERType_Vegetation) && 
		DecalInfo.nPartID>=0)
	{ // transform decal from world space into entity space
		Matrix44 objMat;
		IStatObj * pEntObject = DecalInfo.pDecalOwner->GetEntityStatObj(DecalInfo.nPartID, &objMat);
		assert(pEntObject);
		objMat.Invert44();

		DecalInfo.vNormal = objMat.TransformVectorOLD(DecalInfo.vNormal);
		DecalInfo.vNormal.Normalize();
		DecalInfo.vPos = objMat.TransformPointOLD(DecalInfo.vPos);

		// find object scale
		if(DecalInfo.pDecalOwner->GetEntityRenderType()==eERType_Vegetation)
			fObjScale = DecalInfo.pDecalOwner->GetScale();
		else
		{
			Vec3d vTest(0,0,1.f);
			vTest = objMat.TransformVectorOLD(vTest);
			fObjScale = 1.f/vTest.len();
		}

		ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(DecalInfo.nTid);
		if(pTexPic)
		{
			pTexPic->SetFilter(FILTER_LINEAR);
			pTexPic->SetClamp(true);
		}
	}
	else 
	{
		if(DecalInfo.fSize>COMPLEX_DECAL_MIN_SIZE && !DecalInfo.pDecalOwner && 
			(DecalInfo.vPos.z-pTerrain->GetZApr(DecalInfo.vPos.x,DecalInfo.vPos.y))<DecalInfo.fSize)
		{
			int nUnitSize = CTerrain::GetHeightMapUnitSize();
			int x1 = int(DecalInfo.vPos.x-DecalInfo.fSize)/nUnitSize*nUnitSize-nUnitSize;
			int x2 = int(DecalInfo.vPos.x+DecalInfo.fSize)/nUnitSize*nUnitSize+nUnitSize;
			int y1 = int(DecalInfo.vPos.y-DecalInfo.fSize)/nUnitSize*nUnitSize-nUnitSize;
			int y2 = int(DecalInfo.vPos.y+DecalInfo.fSize)/nUnitSize*nUnitSize+nUnitSize;

			for(int x=x1; x<=x2; x+=CTerrain::GetHeightMapUnitSize())
			for(int y=y1; y<=y2; y+=CTerrain::GetHeightMapUnitSize())
				if(pTerrain->GetHoleSafe(x,y))
					return;

			bBigDecalOnTheGround = true;

			ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(DecalInfo.nTid);
			if(pTexPic)
			{
				pTexPic->SetFilter(FILTER_LINEAR);
				pTexPic->SetClamp(true);
			}
		}
		
		DecalInfo.pDecalOwner = NULL;
	}

	DecalInfo.vNormal.Normalize();

  // spawn
  m_arrDecals[m_nCurDecal].m_vPos = DecalInfo.vPos;

	m_arrDecals[m_nCurDecal].m_vPos += DecalInfo.vNormal*0.01f/fObjScale;

	m_arrDecals[m_nCurDecal].m_bOnTheGround = bBigDecalOnTheGround;

  if(fabs(DecalInfo.vNormal.Dot(Vec3d(0,0,1)))>0.999f)
  { // horiz surface
    m_arrDecals[m_nCurDecal].m_vRight = Vec3d(0,1,0);
    m_arrDecals[m_nCurDecal].m_vUp    = Vec3d(1,0,0);
  }
  else
  {
    m_arrDecals[m_nCurDecal].m_vRight = DecalInfo.vNormal.Cross(Vec3d(0,0,1));
    m_arrDecals[m_nCurDecal].m_vRight.Normalize();
    m_arrDecals[m_nCurDecal].m_vUp    = DecalInfo.vNormal.Cross(m_arrDecals[m_nCurDecal].m_vRight);
    m_arrDecals[m_nCurDecal].m_vUp.Normalize();
  }
/*
	// check that decal will look ok here
	if(!DecalInfo.pDecalOwner && DecalInfo.fSize<=COMPLEX_DECAL_MIN_SIZE)
	{
		ray_hit hit;
		float fSize = DecalInfo.fSize*2;

		if(GetPhysicalWorld()->RayWorldIntersection(
			DecalInfo.vPos - DecalInfo.vNormal*fSize + m_arrDecals[m_nCurDecal].m_vUp*fSize, DecalInfo.vNormal*fSize*2,
			ent_all, rwi_stop_at_pierceable,&hit,1)
		&& GetPhysicalWorld()->RayWorldIntersection(
			DecalInfo.vPos - DecalInfo.vNormal*fSize - m_arrDecals[m_nCurDecal].m_vUp*fSize, DecalInfo.vNormal*fSize*2,
			ent_all, rwi_stop_at_pierceable,&hit,1)
		&& GetPhysicalWorld()->RayWorldIntersection(
			DecalInfo.vPos - DecalInfo.vNormal*fSize + m_arrDecals[m_nCurDecal].m_vRight*fSize, DecalInfo.vNormal*fSize*2,
			ent_all, rwi_stop_at_pierceable,&hit,1)
		&& GetPhysicalWorld()->RayWorldIntersection(
			DecalInfo.vPos - DecalInfo.vNormal*fSize - m_arrDecals[m_nCurDecal].m_vRight*fSize, DecalInfo.vNormal*fSize*2,
			ent_all, rwi_stop_at_pierceable,&hit,1))
		{
			int t=0; // ok
		}
		else
			return; // position is bad
	}
*/
	// rotate vectors
	AngleAxis rotation(DecalInfo.fAngle,DecalInfo.vNormal);
	m_arrDecals[m_nCurDecal].m_vRight = rotation*m_arrDecals[m_nCurDecal].m_vRight;
	m_arrDecals[m_nCurDecal].m_vUp		= rotation*m_arrDecals[m_nCurDecal].m_vUp;

  m_arrDecals[m_nCurDecal].m_vFront		= DecalInfo.vNormal;
  m_arrDecals[m_nCurDecal].m_fSize		= DecalInfo.fSize;
  m_arrDecals[m_nCurDecal].m_fLifeEndTime = GetTimer()->GetCurrTime() + DecalInfo.fLifeTime*GetCVars()->e_decals_life_time_scale;
  m_arrDecals[m_nCurDecal].m_nTexId		= DecalInfo.nTid;
  m_arrDecals[m_nCurDecal].m_fAngle		= DecalInfo.fAngle;
  m_arrDecals[m_nCurDecal].m_pStatObj = DecalInfo.pStatObj;

	m_arrDecals[m_nCurDecal].m_vAmbient = Get3DEngine()->GetAmbientColorFromPosition(DecalInfo.vPos);

	m_arrDecals[m_nCurDecal].m_pDecalOwner = DecalInfo.pDecalOwner;

	m_arrDecals[m_nCurDecal].m_fGrowTime = DecalInfo.m_fGrowTime;
	m_arrDecals[m_nCurDecal].m_fLifeBeginTime = GetTimer()->GetCurrTime();

  if(DecalInfo.pStatObj)
  {
    DecalInfo.pStatObj->RegisterUser();
    CDecal tmp;
    tmp.m_vPos = DecalInfo.vPos;
    tmp.m_vFront = DecalInfo.vNormal;
    tmp.m_nTexId = DecalInfo.nTid;
    tmp.m_fSize = DecalInfo.fSize;
    m_LightHoles.Add(tmp);
  }

	m_arrbActiveDecals[m_nCurDecal] = true;

  m_nCurDecal++;
}

void CDecalManager::SubmitLightHolesToRenderer()
{
  if(!this)
    return;

  for(int i=0; i<m_LightHoles.Count(); i++)
    GetRenderer()->EF_SetLightHole(m_LightHoles[i].m_vPos, 
    m_LightHoles[i].m_vFront, m_LightHoles[i].m_nTexId, m_LightHoles[i].m_fSize);
  
  m_LightHoles.Clear();
}

void CDecalManager::Render()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

  if(!GetCVars()->e_decals || !m_p3DEngine->GetObjManager() || m_p3DEngine->GetCurrentLightSpec() < 1)
    return;

	CCamera * pCamera = &GetViewCamera();
	Vec3d vCamPos = pCamera->GetPos();
	float fCurrTime = GetTimer()->GetCurrTime();
	IRenderer * pIRenderer = GetRenderer();
	CObjManager * pObjManager =	m_p3DEngine->GetObjManager();
	float fZoom = m_p3DEngine->GetObjManager()->m_fZoomFactor;
	float fWaterLevel = m_p3DEngine->GetWaterLevel();

	// draw
  for(int i=0; i<DECAL_COUNT; i++)
  if(m_arrbActiveDecals[i])
  {
		CDecal * pDecal = &m_arrDecals[i];
		if(L1Distance2D(vCamPos, pDecal->m_vWSPos)*fZoom < pDecal->m_fWSSize*ENTITY_DECAL_DIST_FACTOR)
		{
			float fSortOffset = pObjManager->GetSortOffset(pDecal->m_vWSPos,vCamPos,fWaterLevel);
			pDecal->Process(m_arrbActiveDecals[i], pIRenderer, fCurrTime, m_p3DEngine, m_pShader_ParticleLight, pCamera, fSortOffset);
		}
  }
}

void CDecalManager::OnEntityDeleted(IEntityRender * pEntityRender)
{
/*	if(pEntityRender && pEntityRender->GetEntityRenderType()!=eERType_Unknown)
	{ // remove all decals, can happend only during editing or level unloading
		memset(m_arrbActiveDecals,0,sizeof(m_arrbActiveDecals)); 
		return;
	}
*/
  // remove decals of this entity
  for(int i=0; i<DECAL_COUNT; i++)
  if(m_arrbActiveDecals[i])
  {
    if(m_arrDecals[i].m_pDecalOwner == pEntityRender)
		{
      assert(pEntityRender->GetEntityRS());
			m_arrbActiveDecals[i] = false;
			if(m_arrDecals[i].m_pBigDecalLeafBuffer)
			{
				GetRenderer()->DeleteLeafBuffer(m_arrDecals[i].m_pBigDecalLeafBuffer);
				m_arrDecals[i].m_pBigDecalLeafBuffer=0;
			}
      m_arrDecals[i].m_pDecalOwner = 0;
		}
  }
}

void CDecalManager::FillBigDecalIndices(CLeafBuffer * pLeafBuffer, Vec3d vPos, float fRadius, Vec3d vProjDir, list2<ushort> * plstIndices)
{
  // get position offset and stride
	int nPosStride = 0, nNormStride = 0;
	byte * pPos  = pLeafBuffer->GetPosPtr(nPosStride, 0, true);
	byte * pNorm = pLeafBuffer->GetNormalPtr(nNormStride, 0, true);

  ushort *pInds = pLeafBuffer->GetIndices(0);
  int nInds = pLeafBuffer->m_Indices.m_nItems;

	assert(nInds%3 == 0);

	plstIndices->Clear();
	vProjDir.Normalize();

//	Plane plane = GetPlane(vProjDir,vPos);

	// fill decal indices
	for(int i=0; i<nInds; i+=3)
	{
		// get tri vertices
		Vec3d v0 = *((Vec3d*)&pPos[nPosStride*pInds[i+0]]);
		Vec3d v1 = *((Vec3d*)&pPos[nPosStride*pInds[i+1]]);
		Vec3d v2 = *((Vec3d*)&pPos[nPosStride*pInds[i+2]]);

		// get tri normals
		Vec3d n0 = *((Vec3d*)&pNorm[nNormStride*pInds[i+0]]);
		Vec3d n1 = *((Vec3d*)&pNorm[nNormStride*pInds[i+1]]);
		Vec3d n2 = *((Vec3d*)&pNorm[nNormStride*pInds[i+2]]);

		// find bbox
		Vec3d vBoxMin = v0; vBoxMin.CheckMin(v1); vBoxMin.CheckMin(v2);
		Vec3d vBoxMax = v0; vBoxMax.CheckMax(v1); vBoxMax.CheckMax(v2);

		if(vProjDir.IsZero())
		{ // explo mode
			// get dir to triangle
//			Vec3d vCenter = (v0+v1+v2)*0.33333f;
			
			// test the face
			float fDot0 = (vPos-v0).Dot(n0);
			float fDot1 = (vPos-v1).Dot(n1);
			float fDot2 = (vPos-v2).Dot(n2);
			float fTest = -0.15f;
			if(fDot0 < fTest && fDot1 < fTest && fDot2 < fTest)
				continue;
		}
		else
		{
			// get triangle normal
			Vec3d vNormal = (v1-v0).Cross(v2-v0);
			vNormal.Normalize();

			// test the face
			if(vNormal.Dot(vProjDir)<0)
				continue;
		}


		if(Overlap::Sphere_AABB(Sphere(vPos, fRadius), AABB(vBoxMin,vBoxMax)))
		{
			plstIndices->Add(pInds[i+0]);
			plstIndices->Add(pInds[i+1]);
			plstIndices->Add(pInds[i+2]);
		}
	}
}

CLeafBuffer * CDecalManager::MakeBigDecalLeafBuffer(CLeafBuffer * pSourceLeafBuffer, Vec3d vPos, float fRadius, Vec3d vProjDir, int nTexID)
{
  if(!pSourceLeafBuffer || pSourceLeafBuffer->m_SecVertCount==0)
    return 0;

	// make indices of this decal
	list2<ushort> lstIndices;		
	
	if(pSourceLeafBuffer && pSourceLeafBuffer->m_pSecVertBuffer && pSourceLeafBuffer->m_SecVertCount)
		FillBigDecalIndices(pSourceLeafBuffer, vPos, fRadius, vProjDir, &lstIndices);

	if(!lstIndices.Count())
		return 0;

	// make fake vert buffer with one vertex // todo: remove this
  list2<struct_VERTEX_FORMAT_P3F_COL4UB> EmptyVertBuffer;
  EmptyVertBuffer.Add(struct_VERTEX_FORMAT_P3F_COL4UB());

	CLeafBuffer * pLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
    EmptyVertBuffer.GetElements(), EmptyVertBuffer.Count(), VERTEX_FORMAT_P3F_COL4UB,
    lstIndices.GetElements(), lstIndices.Count(),
    R_PRIMV_TRIANGLES, "BigDecal", eBT_Static, 1, nTexID);

  pLeafBuffer->SetVertexContainer(pSourceLeafBuffer);

	IShader * pShader = vProjDir.IsZero() ? m_pShader_Decal_VP : m_pShader_Decal_2D_VP;
  pLeafBuffer->SetChunk(pShader, 0, pSourceLeafBuffer->m_SecVertCount, 0, lstIndices.Count());
  pLeafBuffer->SetShader(pShader,nTexID);

  return pLeafBuffer;
}

/*
CLeafBuffer * CDecalManager::MakeBigDecalLeafBuffer(CLeafBuffer * pSourceLeafBuffer, Vec3d vPos, float fRadius, Vec3d vProjDir, int nTexID)
{
if(!pSourceLeafBuffer || pSourceLeafBuffer->m_SecVertCount==0)
return 0;

// make indices of this decal
list2<ushort> lstIndices;		

if(pSourceLeafBuffer && pSourceLeafBuffer->m_pSecVertBuffer && pSourceLeafBuffer->m_SecVertCount)
FillBigDecalIndices(pSourceLeafBuffer, vPos, fRadius, vProjDir, &lstIndices);

if(!lstIndices.Count())
return 0;

// make fake vert buffer with one vertex // todo: remove this
list2<struct_VERTEX_FORMAT_P3F_N> VertBuffer;
VertBuffer.PreAllocate(pSourceLeafBuffer->m_SecVertCount);
int nPosStride=0, nNormStride=0;
void * pPos = pSourceLeafBuffer->GetPosPtr(nPosStride);
void * pNorm = pSourceLeafBuffer->GetPosPtr(nNormStride);
for(int i=0; i<pSourceLeafBuffer->m_SecVertCount; i++)
{
struct_VERTEX_FORMAT_P3F_N vert;
vert.xyz = *(Vec3d*)((uchar*)pPos+nPosStride*i);
vert.normal = *(Vec3d*)((uchar*)pNorm+nNormStride*i);
vert.normal = vert.normal*5;
VertBuffer.Add(vert);
}

CLeafBuffer * pLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
VertBuffer.GetElements(), VertBuffer.Count(), VERTEX_FORMAT_P3F_N,
lstIndices.GetElements(), lstIndices.Count(),
R_PRIMV_TRIANGLES, "BigDecal", eBT_Static, 1, nTexID);

pLeafBuffer->SetChunk(m_pShader_Decal_VP, 0, pSourceLeafBuffer->m_SecVertCount, 0, lstIndices.Count());
pLeafBuffer->SetShader(m_pShader_Decal_VP,nTexID);

return pLeafBuffer;
}
*/
void CDecalManager::GetMemoryUsage(ICrySizer*pSizer)
{
	pSizer->Add (*this);
	pSizer->AddContainer (m_LightHoles);
}

void CDecalManager::DeleteDecalsInRange( Vec3d vBoxMin, Vec3d vBoxMax, bool bDeleteBigTerrainDecals )
{
	for(int i=0; i<DECAL_COUNT; i++)
		if(m_arrbActiveDecals[i] && !m_arrDecals[i].m_bOnTheGround)
		{
			if(!m_arrDecals[i].m_pDecalOwner && 
				m_arrDecals[i].m_vPos.x<vBoxMax.x && 
				m_arrDecals[i].m_vPos.y<vBoxMax.y && 
				m_arrDecals[i].m_vPos.z<vBoxMax.z && 
				m_arrDecals[i].m_vPos.x>vBoxMin.x && 
				m_arrDecals[i].m_vPos.y>vBoxMin.y && 
				m_arrDecals[i].m_vPos.z>vBoxMin.z )
			{
				m_arrbActiveDecals[i] = false;
				GetRenderer()->DeleteLeafBuffer(m_arrDecals[i].m_pBigDecalLeafBuffer);
				m_arrDecals[i].m_pBigDecalLeafBuffer=0;
			}
		}
}
/*
struct CDebugLine
{
	Vec3d v0,v1;
	int nFrameId;
};

list2<CDebugLine> lstDebugLines;

void AddDebugLine(Vec3d v0, Vec3d v1, int nFrameId)
{
	CDebugLine l;
	l.v0 = v0;
	l.v1 = v1;
	l.nFrameId = nFrameId;
	lstDebugLines.Add(l);
}

void DrawDebugLines(IRenderer*pRenderer)
{
	for(int i=0; i<lstDebugLines.Count(); i++)
	{
		pRenderer->Draw3dPrim(lstDebugLines[i].v0,lstDebugLines[i].v1,DPRIM_LINE);
		if(pRenderer->GetFrameID()>lstDebugLines[i].nFrameId+1000)
		{
			lstDebugLines.Delete(i);
			i--;
		}
	}
}*/