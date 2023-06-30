////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   partpolygon.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: sprite particles, big independent polygons
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "partman.h"

#include "objman.h"


#include "3dEngine.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CSprite::Render( const PartProcessParams &PPP,IShader *pShader )
{
	float fCurTime = PPP.fCurTime;

	uint nDynLightMask = 0;

	if (m_pEmitter->m_pShader)
	{
		pShader = m_pEmitter->m_pShader;
	}
	else
	{
		if (m_pParams->pShader)
			pShader = m_pParams->pShader;
		else
		{
			if (m_pMaterial)
			{
				IShader *pShaderContainer = m_pMaterial->GetShaderItem().m_pShader;
				if (pShaderContainer)
					pShader = pShaderContainer->GetTemplate(-1);
			}
		}
	}

	//	pRenderer->DrawLabel(m_vPos,2,"%d",nDynLightMask);

	int dwCCObjFlags = 0;
	if (m_pParams->nParticleFlags & PART_FLAG_DRAW_NEAR)
		dwCCObjFlags = FOB_NEAREST;

	// do not draw fire in the water
	if(m_pParams->nParticleFlags & PART_FLAG_NO_DRAW_UNDERWATER && m_pEmitter->m_fWaterLevel>WATER_LEVEL_UNKNOWN)
		if(m_vPos.z < m_pEmitter->m_fWaterLevel)
			return;

	// Render 3d object
	if(m_pParams->pStatObj)
	{
		float fObjScale = m_fSize;
		if (m_pParams->fObjectScale)
			fObjScale *= m_pParams->fObjectScale;

		nDynLightMask = PPP.p3DEngine->GetLightMaskFromPosition(m_vPos, fObjScale);
		PPP.p3DEngine->CheckDistancesToLightSources(nDynLightMask,m_vPos,fObjScale,m_pSpawnerEntity,1);

		//Matrix44 matPart2=GetTranslationMat(m_vPos);

		Matrix44 matPart2=Matrix34::CreateTranslationMat(m_vPos);
		matPart2=GetTransposed44(matPart2);

		matPart2=Matrix33::CreateScale( Vec3d(fObjScale,fObjScale,fObjScale) )*matPart2;

		if (m_pParams->nParticleFlags & PART_FLAG_SWAP_XY)
		{
			matPart2=Matrix33::CreateRotationAA( -m_vAngles.z*gf_DEGTORAD, Vec3d(0,0,1) )*matPart2; //NOTE: angles needs to be negated
			matPart2=Matrix33::CreateRotationAA( -m_vAngles.x*gf_DEGTORAD, Vec3d(1,0,0) )*matPart2; //NOTE: angles needs to be negated
			matPart2=Matrix33::CreateRotationAA( -m_vAngles.y*gf_DEGTORAD, Vec3d(0,1,0) )*matPart2; //NOTE: angles needs to be negated
		}
		else

			matPart2=Matrix44::CreateRotationZYX(-gf_DEGTORAD*m_vAngles)*matPart2; //NOTE: angles in radians and negated 

		//-------------------------------------------------------------------

		if(!(m_pParams->nParticleFlags & PART_FLAG_NO_OFFSET))
		{
			Vec3d vCenter = (m_pParams->pStatObj->GetBoxMax() + m_pParams->pStatObj->GetBoxMin())*0.5f;

			//Matrix44 matOffSet=GetTranslationMat(-vCenter*fObjScale);
			Matrix44 matOffSet=Matrix34::CreateTranslationMat(-vCenter*fObjScale);
			matOffSet=GetTransposed44(matOffSet);

			matPart2 = matOffSet * matPart2;
		}

		//-----------------------------------------------------------------------------------------------

		SRendParams rParms;
		rParms.pMatrix				= &matPart2;
		rParms.nDLightMask		= nDynLightMask;
		rParms.nStrongestDLightMask = nDynLightMask;
		rParms.vAmbientColor = Color2Vec(m_cAmbientColor);
		rParms.dwFObjFlags		= FOB_IGNOREMATERIALAMBIENT | dwCCObjFlags;
		//		rParms.nFogVolumeID = m_nFogVolumeId;
		rParms.pMaterial = m_pMaterial;
    rParms.dwFObjFlags |= FOB_TRANS_MASK;

		int nSortId = max(-4,min(4,m_pParams->nDrawLast));
		nSortId = FtoI(PPP.pObjManager->GetSortOffset(m_vPos,PPP.vCamPos,m_pEmitter->m_fWaterLevel)) - nSortId;
		rParms.fCustomSortOffset = (float)nSortId;

		if(dwCCObjFlags & FOB_NEAREST)
			rParms.nSortValue = EFSLIST_LAST;

		float fAge = fCurTime-m_fSpawnTime;
		float fAlpha = (1.f - (fAge - m_pParams->fFadeInTime)/(m_fLifeTime - m_pParams->fFadeInTime));//*2.f;
		//if(fAlpha<0) fAlpha=0; else if(fAlpha>1) fAlpha=1;

		// fadde in
		if (fAge < m_pParams->fFadeInTime && fCurTime)
			fAlpha *= fAge / m_pParams->fFadeInTime;

		fAlpha*=1.5f;
		if(fAlpha<0) fAlpha=0; else if(fAlpha>1) fAlpha=1;

		rParms.fAlpha = fAlpha;
		m_pParams->pStatObj->Render(rParms,Vec3(zero),0);
	}

	// render sprite
	if (m_pParams->nTexId)
	{
		if (m_pParams->eBlendType != ParticleBlendType_Additive)
		{
			nDynLightMask = PPP.p3DEngine->GetLightMaskFromPosition(m_vPos, 0);
			PPP.p3DEngine->CheckDistancesToLightSources(nDynLightMask,m_vPos,m_fSize,m_pSpawnerEntity,1);
		}

		// find vertex offsets from origin
		Vec3d vFront, vRight, vUp;
		Vec3d vParticlePos = m_vPos;
		{
			if(m_pParams->nParticleFlags & PART_FLAG_HORIZONTAL)
			{
				vRight = Vec3d(m_fSize,0,0);
				vUp    = Vec3d(0,m_fSize,0);
				vFront = Vec3d(0,0,1);
			}
			else
			{
				vFront = PPP.vFront;
				vRight = -PPP.vRight*m_fSize;
				vUp    = -PPP.vUp*m_fSize;
			}

			if(m_vAngles.z)
			{
				Matrix33 mat;
				mat.SetRotationAA( DEG2RAD(m_vAngles.z), vFront ); //NOTE: angles needs to be negated
				vRight	= mat * vRight;	
				vUp			= mat * vUp;
			}

			// Stretching to speed direction.
			if (m_pParams->fStretch != 0)
			{
				float fSpeed = m_vDelta.GetLength();
				if (fSpeed < 0.01f)
					fSpeed = 0.01f;
				
				vFront = PPP.pCamera->GetPos() - m_vPos;
				vFront.Normalize();

				Vec3d vVec1 = -vFront.Cross(m_vDelta/fSpeed);
				Vec3d vVec2 = vVec1.Cross(vFront);
				vRight = m_fSize * vVec1;
				vUp = m_fSize * vVec2;
				vUp = vUp +  vUp*fSpeed*m_pParams->fStretch;
				vParticlePos -= vUp; // Offset partcile.
			}
		}

		int nSortId = max(-4,min(4,m_pParams->nDrawLast));
		nSortId = FtoI(PPP.pObjManager->GetSortOffset(m_vPos,PPP.vCamPos,m_pEmitter->m_fWaterLevel)) - nSortId;
		/*
		if((vCamPos.z-pTerrain->Ge tWaterLevel())*(m_vPos.z-pTerrain->GetWaterLev el())>0)
		nSortId += 10000;
		else
		nSortId -= 10000;
		*/

		////////////////////////////////////////////////////////////////////////////////////////////////
		// Find color, texture and alpha
		////////////////////////////////////////////////////////////////////////////////////////////////

		int nTexBindId = m_pParams->nTexId;

		float fAlpha = 1.f;

		float t=0; // from 0 to 1

		float fAge = fCurTime-m_fSpawnTime;
		if ((m_nParticleFlags & PARTICLE_ANIMATED_TEXTURE) && m_pParams->pAnimTex)
		{ // animated
			t = fAge/m_fLifeTime;
			if (t<0) t=0; else if(t>1) t=1;

			float anim_time = t*(m_pParams->nTexAnimFramesCount-1);
			AnimTexInfo *pAnimTexInfo = m_pParams->pAnimTex;
			int cur_tid = 0;
			cur_tid = ((int)anim_time) % pAnimTexInfo->nFramesCount;
			nTexBindId = pAnimTexInfo->pBindIds[cur_tid];

			//t = float(cur_tid)/(pAnimTexInfo->nFramesCount);
		}
		else if(m_pParams->nTexId>=0)
		{ // one frame
			nTexBindId = m_pParams->nTexId;

			t = fAge/m_fLifeTime;
			if (t<0) t=0; else if(t>1) t=1;
		}

		fAlpha = (1.f - (fAge - m_pParams->fFadeInTime)/(m_fLifeTime - m_pParams->fFadeInTime));//*2.f;
		//if(fAlpha<0) fAlpha=0; else if(fAlpha>1) fAlpha=1;

		if(!m_fLifeTime)
		{
			fAlpha = 1.f;
			t = 0;
		}

		// fadde in
		if (fAge < m_pParams->fFadeInTime && fCurTime)
			fAlpha *= fAge / m_pParams->fFadeInTime;

		fAlpha*=1.5f;
		if(fAlpha<0) fAlpha=0; else if(fAlpha>1) fAlpha=1;

		Vec3d vResColor;
		vResColor = m_pParams->vColorStart*(1.f-t) + m_pParams->vColorEnd*t;

		// find result color
		if(nDynLightMask==0 && (!(m_nParticleFlags & CParticle::PARTICLE_ADDITIVE)))
		{
			vResColor.Set(0,0,0);
		}
		else
		{
			if (m_nParticleFlags & CParticle::PARTICLE_COLOR_BASED)
				vResColor *= fAlpha*255.f;
			else
				vResColor *= 0.5f*255.f;
		}

		Vec3 vAmbientColor;
		if (m_nParticleFlags & CParticle::PARTICLE_COLOR_BASED)
		{
			vAmbientColor.Set(0,0,0);
		}
		else
		{
			if(PPP.p3DEngine->GetFogEnd()>PPP.p3DEngine->GetFogStart())
			{
				float fDist = (L1Distance2D(m_vPos, PPP.vCamPos)-PPP.p3DEngine->GetFogStart())
														/(PPP.p3DEngine->GetFogEnd()-PPP.p3DEngine->GetFogStart());
				if(fDist>1.f)
					fDist=1.f;
				else if(fDist<0)
					fDist=0;
				fAlpha *= (1.f-fDist);
			}

			vAmbientColor = Color2Vec(m_cAmbientColor);
		}

		// fade if near the space bounds
		float fAlphaSpaceLoopRatio = 1.f;
		if(m_pParams->nParticleFlags & PART_FLAG_SPACELOOP && m_pEmitter)
		{
			float fDistFromCenterX = m_pParams->vSpaceLoopBoxSize.x - fabs(m_pEmitter->m_pos.x-m_vPos.x);
			float fDistFromCenterY = m_pParams->vSpaceLoopBoxSize.y - fabs(m_pEmitter->m_pos.y-m_vPos.y);
			float fDistFromCenterZ = m_pParams->vSpaceLoopBoxSize.z - fabs(m_pEmitter->m_pos.z-m_vPos.z);
			float fMinDistToTheBorder = min(min(fDistFromCenterX,fDistFromCenterY),fDistFromCenterZ);
			fAlphaSpaceLoopRatio = max(0,min(1.f,fMinDistToTheBorder*0.5f));
		}

		// make particle color
		UCol ucResCol;
		ucResCol.bcolor[0] = fastftol_positive(vResColor.x);
		ucResCol.bcolor[1] = fastftol_positive(vResColor.y);
		ucResCol.bcolor[2] = fastftol_positive(vResColor.z);
		ucResCol.bcolor[3] = fastftol_positive(255.f*fAlpha*fAlphaSpaceLoopRatio);

		if(m_pParams->nParticleFlags & PART_FLAG_LINEPARTICLE)
		{ // line bullet trail
			Vec3d vCamVec = PPP.pCamera->GetPos()-m_vPos;

			vCamVec.Normalize();

			Vec3d vSideStep = vCamVec.Cross(m_vDelta.normalized());
			vSideStep.Normalize();

			vSideStep.SetLen(m_fSize);

			PPP.pObjManager->AddPolygonToRenderer( nTexBindId, pShader, nDynLightMask, vSideStep, m_vDelta*0.5f, 
				ucResCol,
				m_pParams->eBlendType,
				vAmbientColor,
				vParticlePos+m_vDelta*0.5f, 0,0,0,0, (float)nSortId, dwCCObjFlags, m_pMaterial );
		}
		else if (m_pParams->fTailLenght)
		{	// render with tail
			SColorVert arrTailVerts[PART_MAX_HISTORY_ELEMENTS*2];
			byte arrTailIndices[PART_MAX_HISTORY_ELEMENTS*6];
			int nTailVertsNum=0, nTailIndsNum=0;

			nTailVertsNum = FillTailVertBuffer(	arrTailVerts, vFront, ucResCol );
			if(nTailVertsNum>2)
			{ // fill tail indices
				nTailIndsNum = (nTailVertsNum/2-1)*6;
				int nIdxId = 0;
				for(int i=0; i<(nTailVertsNum/2-1); i++)
				{
					arrTailIndices[nIdxId++] = i*2+0;
					arrTailIndices[nIdxId++] = i*2+1;
					arrTailIndices[nIdxId++] = i*2+2;

					arrTailIndices[nIdxId++] = i*2+1;
					arrTailIndices[nIdxId++] = i*2+2;
					arrTailIndices[nIdxId++] = i*2+3;

					assert(i*2+3 < m_nTailSteps*2);
				}
				assert(nIdxId == nTailIndsNum);
			}

			//			nTailIndsNum=0;
			PPP.pObjManager->AddPolygonToRenderer( nTexBindId, pShader, nDynLightMask, vRight, vUp, 
				ucResCol,
				m_pParams->eBlendType, 
				vAmbientColor,
				vParticlePos, arrTailVerts, nTailVertsNum, arrTailIndices, nTailIndsNum, (float)nSortId, dwCCObjFlags, m_pMaterial);

			if (m_pArrvPosHistory)
			{
				float fTailLength = (m_pParams->fTailLenght/m_nTailSteps) /  m_fScale; // Here we must divide by scale because speed is scaled.
				m_fTrailCurPos += min(1.f, PPP.fFrameTime/fTailLength);
				m_pArrvPosHistory[FtoI(m_fTrailCurPos)%m_nTailSteps] = m_vPos;
			}
		}
		else
		{ 
			list2<struct ShadowMapLightSourceInstance> * pShadowsList = NULL;
			if(PPP.pObjManager->GetCVars()->e_particles_receive_shadows && 
				m_pSpawnerEntity && m_pSpawnerEntity->GetEntityRS() && m_pSpawnerEntity->GetEntityRS()->pShadowMapInfo)
				pShadowsList = m_pSpawnerEntity->GetEntityRS()->pShadowMapInfo->pShadowMapCasters;

			// no tail
			PPP.pObjManager->AddPolygonToRenderer( nTexBindId, pShader, nDynLightMask, vRight, vUp, 
				ucResCol,
				m_pParams->eBlendType,
				vAmbientColor,
				vParticlePos, 0,0,0,0, (float)nSortId, dwCCObjFlags, m_pMaterial, NULL, pShadowsList );
		}
	}
}
int CSprite::FillTailVertBuffer(	SColorVert * pTailVerts, 
																	const Vec3d & vCamVec,
																	const UCol & ucColor )
{
	int nVertCount=0;

  if (m_pArrvPosHistory && m_pParams->fTailLenght && (m_vDelta!=Vec3(0.0f,0.0f,0.0f))  )
  {
    int nPos = FtoI(m_fTrailCurPos);

//		if(nPos==1)
//			return 0;

//    Vec3d vCamVec = PPP.pCamera->GetPos()-m_vPos;
//    vCamVec.Normalize();

    Vec3d vSideStep,vSideStepPrev,vDelta,vSideStepReal;
		//Vec3d vSideStepPrev,vSideStepReal;

    //float fMaxItSteps = min( (float)(PART_HISTORY_ELEMENTS-1), m_fTrailCurPos-1);
		float fInvMaxItHalfSteps = 0.5f/m_nTailSteps;

		float fTC0=0.5f;

		Vec3 vPrev = m_vPos;
    // fill vert buffer
    for(int it=0; it < m_nTailSteps && nPos >= 0; it++)
    {
      Vec3 vPos = m_pArrvPosHistory[ nPos % m_nTailSteps ];

			vDelta = vPrev-vPos;
			if (vDelta.IsZero())
				vDelta = m_vDelta;
			vSideStep = m_fSize*vCamVec.Cross(GetNormalized(vDelta));
			
			//if (it > 0)
				//vSideStepReal = (vSideStep + vSideStepPrev)*0.5f; // Average.
			//else
				//vSideStepReal = vSideStep;
			
			//vSideStepPrev = vSideStep;
			vPrev = vPos;

			pTailVerts[nVertCount].vert = vPos + vSideStep;
      pTailVerts[nVertCount].dTC[0] = fTC0;
      pTailVerts[nVertCount].dTC[1] = 0;
			pTailVerts[nVertCount].color = ucColor;
      nVertCount++;
      
      pTailVerts[nVertCount].vert = vPos - vSideStep;
      pTailVerts[nVertCount].dTC[0] = fTC0;
      pTailVerts[nVertCount].dTC[1] = 1;
			pTailVerts[nVertCount].color = ucColor;
      nVertCount++;

      nPos--;
			fTC0+=fInvMaxItHalfSteps;
    }
  }

	return nVertCount;
}