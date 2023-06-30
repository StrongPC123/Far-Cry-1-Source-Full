////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   spritemanager.cpp
//  Version:     v1.00
//  Created:     18/7/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Created by Vladimir
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "partman.h"
#include "objman.h"
#include "visareas.h"
//#include "ParticleEffect.h"
#include "3dEngine.h"

CSpriteManager::CSpriteManager( CPartManager *pPartManager )
{
	m_nMaxSpritesCount = min(16384, max(16,GetCVars()->e_particles_max_count));
	m_arrSprites = new CSprite[m_nMaxSpritesCount];

	m_nCurSpritesCount=0;
	memset(m_arrSprites,0,sizeof(CSprite)*m_nMaxSpritesCount);

	m_pSystem = GetSystem();
	m_p3DEngine = Get3DEngine();
	m_pVisAreaManager = GetVisAreaManager();
	m_pObjManager = ((C3DEngine*)m_p3DEngine)->GetObjManager();
	m_pPartManager = pPartManager;
}

CSpriteManager::~CSpriteManager()
{
	delete [] m_arrSprites;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSpriteManager::Spawn( CParticleEmitter &emitter,bool bChildProcess ) 
{ 
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

	// make sprites  
	ParticleParams &Params = (!bChildProcess) ? *emitter.m_pParams : *emitter.m_pParams->pChild;

	if(!_finite(Params.vPosition.x) || !_finite(Params.vPosition.y) || !_finite(Params.vPosition.z))
	{
//		GetLog()->Log("Warning: CSpriteManager::Spawn: Particle emitter position is undefined");
		return;
	}



	//////////////////////////////////////////////////////////////////////////
/*	if(GetCVars()->e_particles_debug==1)
	{
		Params.fSpeed = Params.fSpeed / 10;
		Params.fLifeTime = Params.fLifeTime*10;
		Params.vGravity/=10;
	}*/

	/*
	if (Params.fScale != 1.0f && Params.fScale > 0)
	{
		float fScale = Params.fScale;
		// Adjust particle parameters by scale.
		Params.fSize = Params.fSize*fScale;
		Params.fPosRandomOffset *= fScale;
		Params.vGravity *= fScale;
		Params.vPositionOffset *= fScale;
		Params.vRandomPositionOffset *= fScale;
		Params.fSizeSpeed *= fScale;
		Params.fSpeed = Params.fSpeed*fScale;
		Params.fTailLenght *= fScale;
		Params.fTurbulenceSize *= fScale;
		//Params.fTurbulenceSpeed *= fScale;
		if (Params.pChild)
		{
			Params.pChild->fSize = Params.pChild->fSize*fScale;
			Params.pChild->fPosRandomOffset *= fScale;
			Params.pChild->vGravity *= fScale;
			Params.pChild->vPositionOffset *= fScale;
			Params.pChild->vRandomPositionOffset *= fScale;
			Params.pChild->fSizeSpeed *= fScale;
			Params.pChild->fSpeed = Params.pChild->fSpeed*fScale;
			Params.pChild->fTailLenght *= fScale;
			Params.pChild->fTurbulenceSize *= fScale;
		}
	}
	*/


	float fCurrTime = m_pPartManager->GetParticlesTime();
	int nCount = max(1,int(Params.nCount*GetCVars()->e_particles_lod));

	// pass Params structure to CPartSpray::Spawn() nCount times
	for(int i=0; i < nCount; i++)
	{
		if(m_nCurSpritesCount>=m_nMaxSpritesCount)
			break;

		CSprite * pSprite = &m_arrSprites[m_nCurSpritesCount];

		// spawn one sprite
		SpawnParticle( emitter,bChildProcess,fCurrTime,pSprite );    
		m_nCurSpritesCount++;
	}
}

//////////////////////////////////////////////////////////////////////////
void CSpriteManager::SpawnParticle( CParticleEmitter &emitter,bool bChildProcess,float fCurrTime,CParticle *pPart )
{
	if (!bChildProcess)
		pPart->m_pParams = emitter.m_pParams;
	else
		pPart->m_pParams = emitter.m_pParams->pChild;

	ParticleParams &Params = *pPart->m_pParams;

//	Params.nParticleFlags |= PART_FLAG_LINEPARTICLE;
	//Params.vDirection = Vec3d(0,100,0);

	//pPart->m_pVisArea = m_pVisAreaManager->GetVisAreaFromPos(Params.vPosition);

	pPart->m_pEmitter = &emitter;
	pPart->m_pEmitter->AddRef();

	// Get scale for this particle.
	float fScale = emitter.m_fScale;
	pPart->m_fScale = fScale;

	pPart->m_nParticleFlags = Params.nParticleFlags;
	if (Params.eBlendType == ParticleBlendType_Additive)
	{
		pPart->m_nParticleFlags |= CParticle::PARTICLE_COLOR_BASED | CParticle::PARTICLE_ADDITIVE;
	}
	else if (Params.eBlendType == ParticleBlendType_ColorBased)
	{
		pPart->m_nParticleFlags |= CParticle::PARTICLE_COLOR_BASED;
	}
	if (Params.nParticleFlags & PART_FLAG_RIGIDBODY)
	{
		// Also set no offset flag.
		Params.nParticleFlags |= PART_FLAG_NO_OFFSET;
	}

	if (Params.nTexAnimFramesCount > 1)
		pPart->m_nParticleFlags |= CParticle::PARTICLE_ANIMATED_TEXTURE;
	
	// calculate focus and dir
	if(Params.nParticleFlags & PART_FLAG_LINEPARTICLE)
		pPart->m_vDelta = Params.vDirection;
	else if(Params.nParticleFlags & PART_FLAG_FOCUS_PLANE)
	{
		// Distribute particles on plane.
		Vec3 dir = Params.vDirection;
		if(Params.nParticleFlags & PART_FLAG_SPEED_IN_GRAVITY_DIRECTION)
		{
			dir = -Params.vGravity;
			if (dir.IsZero()) dir = Vec3(0,0,1);
			dir.Normalize();
		}
		Vec3 n1 = Vec3(0,1,0);
		if (n1 == dir)
			n1 = Vec3(1,0,0);
		n1 = dir.Cross(n1);
		Vec3 n2 = dir.Cross(n1);
		Matrix33 tm;
		tm.SetMatFromVectors( dir,n1,n2 );

		Vec3d vPartDir(2.0f*rnd()-1.0f,2.0f*rnd()-1.0f,2.0f*rnd()-1.0f);
		float focus = max(0,min(1,Params.fFocus));
		vPartDir.x *= focus;
		vPartDir.Normalize();
		vPartDir = tm * vPartDir;
		vPartDir *= Params.fSpeed.GetVariantValue();

		pPart->m_vDelta = vPartDir*fScale;
	}
	else if(Params.nParticleFlags & PART_FLAG_SPEED_IN_GRAVITY_DIRECTION)
	{
		Vec3d vGravityDir = -Params.vGravity;
		Vec3d vPartDir(rnd()-0.5f,rnd()-0.5f,rnd()-0.5f);
		if (vPartDir.IsZero()) vPartDir = Vec3(0,0,1);
		if (vGravityDir.IsZero())
			vGravityDir = Vec3(0,0,-1);
		else
			vGravityDir.Normalize();
		vPartDir.Normalize();
		vPartDir += vGravityDir*Params.fFocus;
		vPartDir.Normalize();
		vPartDir *= Params.fSpeed.GetVariantValue();/* * (0.75f+0.5f*rnd());*/
		pPart->m_vDelta = vPartDir*fScale;
	}
	else
	{
		Vec3d vPartDir(rnd()-0.5f,rnd()-0.5f,rnd()-0.5f);
		vPartDir.Normalize();
		vPartDir += Params.vDirection*Params.fFocus;
		vPartDir.Normalize();
		vPartDir *= Params.fSpeed.GetVariantValue();/* * (0.75f+0.5f*rnd());*/
		pPart->m_vDelta = vPartDir*fScale;
	}

	// do not move water waves in vertical direction
	if(Params.nParticleFlags & PART_FLAG_HORIZONTAL)
		pPart->m_vDelta.z = 0;

	// set params
	if(Params.nParticleFlags & PART_FLAG_SPACELOOP)
	{
		pPart->m_vPos = Params.vPosition + Vec3d(rnd()-0.5f,rnd()-0.5f,rnd()-0.5f)*32;
/*
		float fModX = fabs(Params.vDirection.x);
		float fModY = fabs(Params.vDirection.y);
		float fModZ = fabs(Params.vDirection.z);

		if(fModX>fModY && fModX>fModZ)
			pPart->m_vPos.x = Params.vPosition.x + (Params.vDirection.x>0 ? -Params.vSpaceLoopBoxSize.x : Params.vSpaceLoopBoxSize.x);
		else if(fModY>fModX && fModY>fModZ)
			pPart->m_vPos.y = Params.vPosition.y + (Params.vDirection.y>0 ? -Params.vSpaceLoopBoxSize.y : Params.vSpaceLoopBoxSize.y);
		else
			pPart->m_vPos.z = Params.vPosition.z + (Params.vDirection.z>0 ? -Params.vSpaceLoopBoxSize.z : Params.vSpaceLoopBoxSize.z);*/
	}
	else if(Params.fPosRandomOffset)
	{ // random offset
		Vec3d vOffset(rnd()-0.5f,rnd()-0.5f,rnd()-0.5f);
		vOffset *= (Params.fPosRandomOffset*2)*fScale;
		pPart->m_vPos = Params.vPosition + vOffset;
	}
	else
	{
		pPart->m_vPos = Params.vPosition;
	}

	if (!Params.vRandomPositionOffset.IsZero())
	{
		Vec3 vrnd;
		vrnd.x = (2.0f*rnd()-1.0f) * Params.vRandomPositionOffset.x;
		vrnd.y = (2.0f*rnd()-1.0f) * Params.vRandomPositionOffset.y;
		vrnd.z = (2.0f*rnd()-1.0f) * Params.vRandomPositionOffset.z;
		pPart->m_vPos += vrnd*fScale;
	}
	pPart->m_vPos += Params.vPositionOffset*fScale;

	pPart->m_fSizeOriginal = Params.fSize.GetVariantValue()*fScale;
	pPart->m_fSize      = pPart->m_fSizeOriginal;
	pPart->m_vAngles    = Params.vInitAngles.GetVariantValue();
	pPart->m_vRotation  = Params.vRotation.GetVariantValue();

	pPart->m_fLifeTime  = Params.fLifeTime.GetVariantValue();
	pPart->m_fSpawnTime = fCurrTime;

	if (Params.fTailLenght)
	{
		pPart->m_nTailSteps = Params.nTailSteps;
		if (pPart->m_nTailSteps <= 0)
			pPart->m_nTailSteps = 8; // Default steps.
		if (pPart->m_nTailSteps > 16)
			pPart->m_nTailSteps = 16; // Max steps (Renderer cant do more).
		// init history stack
		pPart->m_fTrailCurPos = 1.0f;
		pPart->m_pArrvPosHistory = new Vec3[pPart->m_nTailSteps];
		for(int t=0; t < pPart->m_nTailSteps; t++)
			pPart->m_pArrvPosHistory[t] = Params.vPosition;
	}

	// init physical particle if requested
	if(Params.bRealPhysics /* && GetCVars()->e_particles_allow_physics */)
		pPart->Physicalize(  Params,m_pSystem->GetIPhysicalWorld() );

	pPart->m_fChildSpawnLastTime = fCurrTime;

	// set statobj
//	if(Params.pStatObj)
	//	Params.pStatObj->RegisterUser(); // prevent object deleting

	// get ambient color
	Vec3d vAmbientColor = m_p3DEngine->GetAmbientColorFromPosition(Params.vPosition);
	Vec3d vWorldColor = Get3DEngine()->GetWorldColor();
	vAmbientColor.x *= vWorldColor.x;
	vAmbientColor.y *= vWorldColor.y;
	vAmbientColor.z *= vWorldColor.z;
  vAmbientColor.CheckMin(Vec3d(1,1,1));

	pPart->m_cAmbientColor = pPart->Vec2Color(vAmbientColor);

	//pPart->m_nDynLightMask = 0;//pSystem->GetI3DEngine()->GetLightMaskFromPosition(Params.vPosition);

	//float fSize = pPart->m_fSize;
	//pPart->m_nFogVolumeId  = m_p3DEngine->GetFogVolumeIdFromBBox(
		//Params.vPosition-Vec3d(fSize,fSize,fSize),
		//Params.vPosition+Vec3d(fSize,fSize,fSize));
	if (Params.pMaterial)
	{
		pPart->m_pMaterial = Params.pMaterial;
	}
	else
		pPart->m_pMaterial = emitter.m_pMaterial;
	pPart->m_pSpawnerEntity = emitter.m_pSpawnerEntity;
}

//////////////////////////////////////////////////////////////////////////
void CSpriteManager::Render(CObjManager * pObjManager, CTerrain * pTerrain, int nRecursionLevel, CPartManager * pPartManager, IShader * pPartLightShader)
{
	// update max sprites count if console variable changed
	if(m_nMaxSpritesCount != min(16384, max(16,GetCVars()->e_particles_max_count)))
	{
		m_nMaxSpritesCount = min(16384, max(16,GetCVars()->e_particles_max_count));
		delete [] m_arrSprites;
		m_arrSprites = new CSprite[m_nMaxSpritesCount];
		memset(m_arrSprites,0,sizeof(CSprite)*m_nMaxSpritesCount);
		m_nCurSpritesCount=0;
	}

	// get orientation for billboard particles
	Matrix44 mat;
	mat.SetIdentity();
	GetRenderer()->GetModelViewMatrix(mat.GetData());
	Vec3d vRight,vUp,vFront;	

	//CELL_CHANGED_BY_IVO
	//vRight(mat.cell(0), mat.cell(4), mat.cell(8));
	//vUp   (mat.cell(1), mat.cell(5), mat.cell(9)); 
	//vFront(mat.cell(2), mat.cell(6), mat.cell(10)); 
	vRight = GetNormalized(mat.GetColumn(0));
	vUp		 = GetNormalized(mat.GetColumn(1));
	vFront = GetNormalized(mat.GetColumn(2));


	Vec3d vCamPos = GetViewCamera().GetPos();

	CCamera * pCamera = &GetViewCamera();
	CVars * pCVars = GetCVars();

	PartProcessParams ProcParams;

	ProcParams.fCurTime = pPartManager->GetParticlesTime();;
	ProcParams.fFrameTime = pPartManager->GetParticlesFrameTime();
	ProcParams.vRight=vRight;
	ProcParams.vUp=vUp;
	ProcParams.vFront=vFront;
	ProcParams.pIRenderer=GetRenderer();
	ProcParams.pObjManager=pObjManager;
	ProcParams.pTerrain=pTerrain;
	ProcParams.pPhysicalWorld=GetPhysicalWorld();
	ProcParams.pCamera=pCamera;
	ProcParams.pVertBufChunk=0;
	ProcParams.pSystem = GetSystem();
	ProcParams.p3DEngine = (C3DEngine*)m_p3DEngine;
	ProcParams.pPartManager = pPartManager;
	ProcParams.vCamPos = pCamera->GetPos();

	for( int i=0; i<m_nCurSpritesCount && i<m_nMaxSpritesCount; i++)
	{
		bool bActive = true;
		
		CSprite *pSprite = &m_arrSprites[i];

		if(!nRecursionLevel)
		{
			bActive = pSprite->Update(ProcParams);
			//Vec3 vsz = Vec3(pSprite->m_fSize,pSprite->m_fSize,pSprite->m_fSize);
			//pSprite->m_pEmitter->m_bbox.Add( pSprite->m_vPos - vsz );
			//pSprite->m_pEmitter->m_bbox.Add( pSprite->m_vPos + vsz );
		}

		if (!nRecursionLevel || (!(pSprite->m_nParticleFlags & PART_FLAG_HORIZONTAL)) )
		{
			bool bVisible = false;
			if(pSprite->m_nParticleFlags & PART_FLAG_LINEPARTICLE)
			{ // find bbox
				Vec3d vBoxMin = pSprite->m_vPos;
				Vec3d vBoxMax = pSprite->m_vPos;
				vBoxMin.CheckMin(pSprite->m_vPos + pSprite->m_vDelta);
				vBoxMax.CheckMax(pSprite->m_vPos + pSprite->m_vDelta);
				vBoxMin -= Vec3d(pSprite->m_fSize,pSprite->m_fSize,pSprite->m_fSize);
				vBoxMax += Vec3d(pSprite->m_fSize,pSprite->m_fSize,pSprite->m_fSize);
				bVisible = pCamera->IsAABBVisibleFast( AABB(vBoxMin,vBoxMax) );
			}
			else
				bVisible = pCamera->IsSphereVisibleFast( Sphere(pSprite->m_vPos,pSprite->m_fSize) );

			if ( bVisible && (!(pSprite->m_nParticleFlags & PART_FLAG_SPACELIMIT) || bActive))
				pSprite->Render( ProcParams,pPartLightShader );
		}

		if (!bActive)
		{ // remove
			pSprite->DeActivateParticle(GetPhysicalWorld());

			if(i < m_nCurSpritesCount-1)
			{
				m_arrSprites[i] = m_arrSprites[m_nCurSpritesCount-1];
				memset(&m_arrSprites[m_nCurSpritesCount-1],0,sizeof(m_arrSprites[m_nCurSpritesCount-1]));
			}
			m_nCurSpritesCount--;
			i--;
		}
	}
}