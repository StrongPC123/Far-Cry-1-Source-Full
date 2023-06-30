////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ParticleEffect.h
//  Version:     v1.00
//  Created:     10/7/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particleeffect_h__
#define __particleeffect_h__
#pragma once

#include "I3DEngine.h"

class CPartManager;
class CParticleEmitter;

/*!	CParticleEffect implements IParticleEffect interface and contain all components necesarry to 
		to create particluar effect with particles.
 */
class CParticleEffect : public IParticleEffect, public Cry3DEngineBase
{
public:
	CParticleEffect( CPartManager *pPartManager );
	~CParticleEffect();

	virtual void Spawn( const Vec3 &pos,const Vec3 &dir,float fScale=1.0f );

	virtual void SetName( const char *sName );
	virtual const char* GetName() { return m_name.c_str(); };

	virtual void SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; };
	virtual bool IsEnabled() const { return m_bEnabled; };

	//////////////////////////////////////////////////////////////////////////
	// Child particle systems.
	//////////////////////////////////////////////////////////////////////////
	virtual int GetChildCount() const;
	//! Get sub Particles child by index.
	virtual IParticleEffect* GetChild( int index ) const;
	//! Adds a new sub Particles.
	virtual void AddChild( IParticleEffect *pEffect );
	//! Remove specific sub Particles
	virtual void RemoveChild( IParticleEffect *pEffect );
	//! Remove all sub Particles.
	virtual void ClearChilds();
	//! Insert sub particles in between other child particles.
	virtual void InsertChild( int slot,IParticleEffect *pEffect );
	//! Find slot where sub Particles stored.
	//! @retun slot index if Particles found, -1 if Particles not found.
	virtual int FindChild( IParticleEffect *pEffect ) const;


	//////////////////////////////////////////////////////////////////////////
	ParticleParams& GetParticleParams( int process ) { return m_particleParams[process]; };

	//////////////////////////////////////////////////////////////////////////
	//virtual EffectParams& GetEffectParams() { return m_effectParams; };

	//////////////////////////////////////////////////////////////////////////
	// Texture and Geometry.
	//////////////////////////////////////////////////////////////////////////
	const char* GetTexture( int process ) const { return m_texture[process].c_str(); };
	const char* GetGeometry( int process ) const { return m_geometry[process].c_str(); };
	void SetTexture( int process,const char *s );
	void SetGeometry( int process,const char *s );

	//////////////////////////////////////////////////////////////////////////
	// Materials
	//////////////////////////////////////////////////////////////////////////
	void SetMaterial( int process,IMatInfo *pMaterial );
	IMatInfo* GetMaterial( int process ) const { return m_pMaterials[process]; };
	virtual void SetMaterialName( int process,const char *sMtlName );
	virtual const char* GetMaterialName( int process ) const;

	//////////////////////////////////////////////////////////////////////////
	//! Load resources, required by this particle effect (Textures and geometry).
	void LoadResources( bool bRecursive = true );
	//! Unload resources, required by this particle effect (Textures and geometry).
	void UnloadResources( bool bRecursive = true );
	//! Check if resources are loaded.
	bool IsResourcesLoaded();

	//////////////////////////////////////////////////////////////////////////
	// Sound parameters.
	//! Set Sound parameters for this particle effect.
	virtual void SetSoundParams( const SoundParams &params );
	//! Get Sound parameters for this particle effect.
	virtual void GetSoundParams( SoundParams &params ) const;
	//////////////////////////////////////////////////////////////////////////

	//! Called to initialize effect just before particles spawned..
	//! @return true if can spawn, false if cannot spawn this effect.
	bool PrepareSpawn( const Vec3 &pos );

private:
	void AssignMaterial( int process );
	//////////////////////////////////////////////////////////////////////////
	String m_name;

	//! When enabled this particle system will be spawned.
	bool m_bEnabled;
	//! Indicated whenever this effect was already loaded.
	mutable bool m_bLoaded;

	//! Particle primary and child system spawning parameters.
	ParticleParams m_particleParams[NUM_PARTICLE_PROCESSES];
	String m_texture[NUM_PARTICLE_PROCESSES];
	String m_geometry[NUM_PARTICLE_PROCESSES];
	String m_material[NUM_PARTICLE_PROCESSES];

	//////////////////////////////////////////////////////////////////////////
	// Materials.
	//////////////////////////////////////////////////////////////////////////
	_smart_ptr<IMatInfo> m_pMaterials[NUM_PARTICLE_PROCESSES];

	bool m_bAnimatedTexture[NUM_PARTICLE_PROCESSES];

	//EffectParams m_effectParams;

	//! Array of sub materials.
	std::vector<IParticleEffect_AutoPtr> m_childs;

	//! Creating particle manager.
	CPartManager* m_pPartManager;

	//! Effect's sound parameters.
	String m_sound;
	float m_soundVolume;
	float m_soundMinRadius;
	float m_soundMaxRadius;
	bool m_bSoundLoop;
	bool m_bOnEverySpawn;
};

#endif //__particleeffect_h__
