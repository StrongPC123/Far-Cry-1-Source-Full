////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particleemitter.h
//  Version:     v1.00
//  Created:     18/7/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particleemitter_h__
#define __particleemitter_h__
#pragma once

#include <ISound.h>

/*! Temporary emitter position for ParticleEffects.
*/
class CParticleEmitter : public IParticleEmitter
{
public:
	uint m_bOwnParams : 1;		//!< True if particle created its own particle params.
	uint m_bActive : 1;				//!< True when this emitter is active.
	uint m_bActiveChild : 1;	//!< True when child emitter is active.
	uint m_bPermament : 1;		//!< True when this emitter is permament (temporary emitters not added to particle manager).
	uint m_bLoopSound : 1;		//!< True when playing looped sound.
	uint m_bVisible : 1;			//!< True if emitter is visible.
	uint m_bUseEndTime : 1;		//!< True if m_endTime is used
	uint m_bChildEmitter : 1;	//!< True if this child emitter (Only with permament parent emitter).

	//! Position of the emitter.
	Vec3 m_pos;
	//! Direction of the emitter.
	Vec3 m_dir;
	//! Scale for particles at this emitter.
	float m_fScale;
	//! When this emitter must be activated.
	float m_startTime;
	//! When this emitter must be killed. (only used when m_bUseEndTime is true)
	float m_endTime;
	//! How often to spawn particles, time in seconds between 2 successing spawns.
	float m_spawnPeriod;
	// Last time when particles where spawned.
	float m_lastSpawnTime;
	//! Pointer to particle params. (ParticleEffect).
	ParticleParams *m_pParams;
	//! Pointer to child params of this emitter.
	ParticleParams *m_pChildParams;
	//! Notify time when position was last set on this emitter.
	float m_lastActiveTime;
	//! Particle effect used for this emitter.
	IParticleEffect_AutoPtr m_pEffect;
	//! Owner particle manager.
	class CPartManager *m_pPartManager;
	//! Override material for this emitter.
	_smart_ptr<IMatInfo> m_pMaterial;
	//! Entity who controls this emitter.
	IEntityRender *m_pSpawnerEntity;
	//! Custom shader from material.
	_smart_ptr<IShader> m_pShader;
	//! Bounding box.
	AABB m_bbox;
	//! Water level in position of emitter. Used to avoid calculating it for each particle.
	float m_fWaterLevel; 

	//////////////////////////////////////////////////////////////////////////
	std::vector<_smart_ptr<CParticleEmitter> > m_childEmitters;

	//////////////////////////////////////////////////////////////////////////
	// Particle emitter looped sound.
	_smart_ptr<ISound> m_pSound;

	//////////////////////////////////////////////////////////////////////////
	// Methods.
	//////////////////////////////////////////////////////////////////////////
	CParticleEmitter( CPartManager *pPartManager ) : m_pos(0,0,0),m_dir(0,0,0)
	{
		m_pPartManager = pPartManager;
		m_pSpawnerEntity = NULL;
		m_startTime = m_endTime = m_lastActiveTime = m_spawnPeriod = m_lastSpawnTime = 0;
		m_pParams = 0;
		m_pChildParams = 0;
		m_bOwnParams = false;
		m_fScale = 1;
		m_bActive = false;
		m_bActiveChild = false;
		m_bPermament = false;
		m_bVisible = true;
		m_fWaterLevel = WATER_LEVEL_UNKNOWN;
		m_bLoopSound = false;
		m_bUseEndTime=false;
		m_bChildEmitter = false;
	}
	~CParticleEmitter();
	void ReleaseParams();
	void UpdateChildSpawnTimes( float fCurrTime );
	// Only when not effect.
	void InitTexture( ParticleParams *pParams );

	void AssignEffect( IParticleEffect *pEffect,bool bChildEffects );

	void CalculateWaterLevel();
	void OnActivate( bool bActive );
	void OnSpawnParticles( bool bChildProcess );
	void PlaySound();

	//////////////////////////////////////////////////////////////////////////
	// IParticleEmitter interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetParams( const ParticleParams &params );
	virtual void SetEffect( IParticleEffect *pEffect );
	virtual const ParticleParams& GetParams() const;
	virtual void SetPos( const Vec3 &vPos,const Vec3 &vDir,float fScale );
	virtual void SetSpawnPeriod( float fSpawnPeriod );
	virtual void SetLifeTime( const float fLifeTime );
	virtual void SetUnlimitedLife();
	virtual void SetEntity( IEntityRender *pEntity );
	virtual void SetMaterial( IMatInfo *pMaterial );
};

#endif // __particleemitter_h__

