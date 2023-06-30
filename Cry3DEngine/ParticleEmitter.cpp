////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particleemitter.cpp
//  Version:     v1.00
//  Created:     18/7/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ParticleEmitter.h"
#include "partman.h"

//////////////////////////////////////////////////////////////////////////
CParticleEmitter::~CParticleEmitter()
{
	if (m_bActive)
		OnActivate(false);
	ReleaseParams();
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::ReleaseParams()
{
	if (m_pParams && m_pParams->pStatObj)
		m_pParams->pStatObj->UnregisterUser();
	if (m_pChildParams && m_pChildParams->pStatObj)
		m_pChildParams->pStatObj->UnregisterUser();

	if (m_pParams && m_bOwnParams)
	{
		delete m_pParams;
	}
	if (m_pChildParams && m_bOwnParams)
	{
		delete m_pChildParams;
	}

	m_pParams = 0;
	m_pChildParams = 0;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetParams( const ParticleParams &params )
{
	ReleaseParams();

	m_bOwnParams = true;
	m_pParams = new ParticleParams;

	float fCurTime = m_pPartManager->GetParticlesTime();
	m_startTime = fCurTime + params.fSpawnDelay.GetVariantValue();
	SetLifeTime(params.fEmitterLifeTime.GetVariantValue());
	m_spawnPeriod = params.fSpawnPeriod;
	m_lastActiveTime = m_startTime;
	m_lastSpawnTime = -100000.0f; // Force to spawn first time.

	*m_pParams = params;
	m_pParams->pChild = 0;
	if (params.pChild != NULL && params.pChild->nCount > 0)
	{
		m_pChildParams = new ParticleParams;
		m_pParams->pChild = m_pChildParams;
		*m_pChildParams = *params.pChild;
	}
	m_pMaterial = params.pMaterial;
	m_pSpawnerEntity = params.pEntity;
	m_pos = params.vPosition;
	m_dir = params.vDirection;
	m_bbox.min = m_bbox.max = m_pos;

	//////////////////////////////////////////////////////////////////////////
	// Keep object from suddenly deleting.
	//////////////////////////////////////////////////////////////////////////
	if (m_pParams && m_pParams->pStatObj)
		m_pParams->pStatObj->RegisterUser();
	if (m_pChildParams && m_pChildParams->pStatObj)
		m_pChildParams->pStatObj->RegisterUser();
	//////////////////////////////////////////////////////////////////////////

	InitTexture( m_pParams );
	if (m_pChildParams)
		InitTexture( m_pChildParams );

	CalculateWaterLevel();
}

void CParticleEmitter::InitTexture( ParticleParams *pParams )
{
	//////////////////////////////////////////////////////////////////////////
	// Initialize texure ids.
	//////////////////////////////////////////////////////////////////////////
	if (pParams->nTexAnimFramesCount>1 && pParams->nTexId)
	{
		AnimTexInfo *p = Cry3DEngineBase::GetRenderer()->GetAnimTexInfoFromId(pParams->nTexId);
		if(!p)
		{
#if !defined(LINUX)
			Cry3DEngineBase::Warning( 0,0,"Invalid Animated Texture Id %d for Particles",pParams->nTexId );
#endif
			return;
		}
		pParams->pAnimTex = p;
	}
	else if(!pParams->nTexId && !pParams->pStatObj)
	{
		pParams->nTexId = m_pPartManager->GetGlowTexID();
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetEffect( IParticleEffect *pEffect )
{
	AssignEffect( pEffect,true );
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::AssignEffect( IParticleEffect *pEffect,bool bChildEffects )
{
	ReleaseParams();
	m_bOwnParams = false;

	m_pEffect = pEffect;
	m_pParams = &m_pEffect->GetParticleParams(0);
	ParticleParams& childParams = m_pEffect->GetParticleParams(1);

	float fCurTime = m_pPartManager->GetParticlesTime();
	m_startTime = fCurTime + m_pParams->fSpawnDelay.GetVariantValue();
	SetLifeTime(m_pParams->fEmitterLifeTime.GetVariantValue());
	m_spawnPeriod = m_pParams->fSpawnPeriod;
	m_lastActiveTime = m_startTime;
	m_lastSpawnTime = -100000.0f; // Force to spawn first time.

	if (m_pParams->pChild)
	{
		m_pChildParams = m_pParams->pChild;
	}
	else if (childParams.nCount > 0)
	{
		m_pChildParams = &childParams;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keep object from suddenly deleting.
	//////////////////////////////////////////////////////////////////////////
	if (m_pParams && m_pParams->pStatObj)
		m_pParams->pStatObj->RegisterUser();
	if (m_pChildParams && m_pChildParams->pStatObj)
		m_pChildParams->pStatObj->RegisterUser();
	//////////////////////////////////////////////////////////////////////////

	//Not needed. m_pMaterial = m_pParams->pMaterial;
	m_pSpawnerEntity = m_pParams->pEntity;
	m_pos = m_pParams->vPosition;
	m_dir = m_pParams->vDirection;
	m_bbox.min = m_bbox.max = m_pos;

	//////////////////////////////////////////////////////////////////////////
	// Play looped sound for this particle emitter.
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create child effects
	//////////////////////////////////////////////////////////////////////////
	if (bChildEffects && pEffect->GetChildCount() > 0)
	{
		for (int i = 0; i < pEffect->GetChildCount(); i++)
		{
			// Create child emitter.
			CParticleEmitter *pEmitter = new CParticleEmitter(m_pPartManager);
			pEmitter->m_bChildEmitter = true;
			pEmitter->m_bPermament = true;
			m_childEmitters.push_back( pEmitter );
			CParticleEffect *pChildEffect = (CParticleEffect*)pEffect->GetChild(i);
			pEmitter->SetEffect(pChildEffect);
			pEmitter->m_bbox.min = pEmitter->m_bbox.max = m_pos;
			// If uncommented Sounds of child emitters not playing then.. 
			// pEmitter->m_bActive = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::UpdateChildSpawnTimes( float fCurrTime )
{
	for (int i = 0; i < (int)m_childEmitters.size(); i++)
	{
		CParticleEmitter *pEmitter = m_childEmitters[i];
		if (pEmitter && pEmitter->m_pParams)
		{
			pEmitter->m_bActiveChild = true;
			pEmitter->m_startTime = fCurrTime + pEmitter->m_pParams->fSpawnDelay.GetVariantValue();
			pEmitter->SetLifeTime(pEmitter->m_pParams->fEmitterLifeTime.GetVariantValue());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
const ParticleParams& CParticleEmitter::GetParams() const
{
	return *m_pParams;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetPos( const Vec3 &vPos,const Vec3 &vDir,float fScale )
{
	bool bPosChanged = false;
	if (m_pos != vPos)
		bPosChanged = true;
	m_pos = vPos;
	m_dir = vDir;
	m_fScale = fScale;
	m_lastActiveTime = m_pPartManager->GetParticlesTime();
	
	if (bPosChanged)
	{
		if (!m_bActive)
		{
			m_bbox.min = vPos;
			m_bbox.max = vPos;
		}

		if (m_pSound)
		{
			// Update sound position.
			m_pSound->SetPosition( m_pos + m_pParams->vPositionOffset );
		}

		for (int i = 0; i < (int)m_childEmitters.size(); i++)
		{
			CParticleEmitter *pEmitter = m_childEmitters[i];
			if (pEmitter)
			{
				pEmitter->SetPos(vPos,vDir,fScale);
			}
		}

		CalculateWaterLevel();
	}

	if (!m_bChildEmitter)
	{
		// Make sure this emitter is active.
		m_pPartManager->ActivateEmitter( this );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetSpawnPeriod( float fSpawnPeriod )
{
	m_spawnPeriod = fSpawnPeriod;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetLifeTime( float fLifeTime )
{
	m_endTime = m_startTime + max(fLifeTime,0);
	m_bUseEndTime=true;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetUnlimitedLife()
{
	m_bUseEndTime=false;		// Unlimited life time.
}
//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetEntity( IEntityRender *pEntity )
{
	m_pSpawnerEntity = pEntity;

	CalculateWaterLevel();
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::SetMaterial( IMatInfo *pMaterial )
{
	m_pMaterial = pMaterial;
	m_pShader = NULL;
	if (m_pMaterial)
	{
		IShader *pContainerShader = m_pMaterial->GetShaderItem().m_pShader;
		m_pShader = pContainerShader->GetTemplate(-1);
	}

	if(m_pMaterial)
		m_pMaterial->SetFlags(m_pMaterial->GetFlags()|MIF_WASUSED);
}

void CParticleEmitter::CalculateWaterLevel()
{
	// remember water level to avoid calculating it for each particle
	if(m_pSpawnerEntity)
		m_fWaterLevel = Cry3DEngineBase::Get3DEngine()->GetWaterLevel(m_pSpawnerEntity);
	else
		m_fWaterLevel = Cry3DEngineBase::Get3DEngine()->GetWaterLevel(&m_pos);
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::OnActivate( bool bActive )
{
	if (bActive == m_bActive)
		return;

	m_bActive = bActive;
	if (bActive)
	{
		// Effect is activated.
		// Play sound if have.
		ISoundSystem *pISoundSystem = GetISystem()->GetISoundSystem();
#if !defined(LINUX64)
		if (pISoundSystem != NULL && m_pEffect != NULL)
#else
		if (pISoundSystem != 0 && m_pEffect != 0)
#endif
		{
			// Check if effect needs to play sounds.
			IParticleEffect::SoundParams soundParams;
			m_pEffect->GetSoundParams( soundParams );
			if (strlen(soundParams.szSound) > 0)
			{
				int nSndFlags = FLAG_SOUND_3D | FLAG_SOUND_RADIUS | FLAG_SOUND_OCCLUSION;
				if (soundParams.bLoop && m_bPermament)
				{
					m_bLoopSound = true;
					nSndFlags |= FLAG_SOUND_LOOP;
				}
				else
					m_bLoopSound = false;

				m_pSound = pISoundSystem->LoadSound( soundParams.szSound,nSndFlags );
#if !defined(LINUX64)
				if (m_pSound != NULL && !soundParams.bOnEverySpawn)
#else
				if (m_pSound != 0 && !soundParams.bOnEverySpawn)
#endif
				{
					PlaySound();
				}
			}
		}
	}
	else
	{
		// Effect is deactivated.
		if (m_pSound != 0 && m_bLoopSound)
		{
			m_pSound->Stop();
			m_pSound = NULL;
		}
	}

	// Activate/deactivate childs.
	for (int i = 0; i < (int)m_childEmitters.size(); i++)
	{
		CParticleEmitter *pEmitter = m_childEmitters[i];
		if (pEmitter)
		{
			pEmitter->OnActivate( bActive );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::PlaySound()
{
	if (!m_pEffect || !m_pSound)
		return;
	
	IParticleEffect::SoundParams soundParams;
	m_pEffect->GetSoundParams( soundParams );

	m_pSound->SetVolume( (int)soundParams.volume );
	m_pSound->SetMinMaxDistance( soundParams.minRadius,soundParams.maxRadius );
	Vec3 vOffset = m_pParams->vPositionOffset;
	m_pSound->SetPosition( m_pos + vOffset );
	m_pSound->Play();
}

//////////////////////////////////////////////////////////////////////////
// Called when particles are spawned.
//////////////////////////////////////////////////////////////////////////
void CParticleEmitter::OnSpawnParticles( bool bChildProcess )
{
#if !defined(LINUX64)
	if (!bChildProcess && m_pSound != NULL && m_pEffect != NULL)
#else
	if (!bChildProcess && m_pSound != 0 && m_pEffect != 0)
#endif
	{
		IParticleEffect::SoundParams soundParams;
		m_pEffect->GetSoundParams( soundParams );
		if (soundParams.bOnEverySpawn)
		{
			PlaySound();
		}
	}
}