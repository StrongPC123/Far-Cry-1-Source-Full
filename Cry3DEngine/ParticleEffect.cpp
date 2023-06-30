////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particleeffect.cpp
//  Version:     v1.00
//  Created:     10/7/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ParticleEffect.h"
#include "ParticleEmitter.h"
#include "PartMan.h"
#include "3DEngine.h"
#include "ISound.h"

//////////////////////////////////////////////////////////////////////////
CParticleEffect::CParticleEffect( CPartManager *pPartManager )
{
	assert( pPartManager );
	m_pPartManager = pPartManager;
	m_bLoaded = false;
	m_bEnabled = true;

	m_soundVolume = 100;
	m_soundMinRadius = 1;
	m_soundMaxRadius = 10;
	m_bSoundLoop = true;
	m_bOnEverySpawn = false;

	m_bAnimatedTexture[0] = false;
	m_bAnimatedTexture[1] = false;
}

//////////////////////////////////////////////////////////////////////////
CParticleEffect::~CParticleEffect()
{
	UnloadResources();
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::SetName( const char *sName )
{
	m_pPartManager->RenameEffect( this,sName );

	m_name = sName;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::SetTexture( int process,const char *s )
{
	if (m_texture[process] != s)
	{
		UnloadResources(false);
		m_texture[process] = s;
	}
};

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::SetGeometry( int process,const char *s )
{
	if (m_geometry[process] != s)
	{
		UnloadResources(false);
		m_geometry[process] = s;
	}
};

//////////////////////////////////////////////////////////////////////////
int CParticleEffect::GetChildCount() const
{
	return (int)m_childs.size();
}

//////////////////////////////////////////////////////////////////////////
IParticleEffect* CParticleEffect::GetChild( int index ) const
{
	assert( index >= 0 && index < (int)m_childs.size() );
	return m_childs[index];
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::AddChild( IParticleEffect *pEffect )	
{
	assert( pEffect );
	m_childs.push_back(pEffect);
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::RemoveChild( IParticleEffect *pEffect )
{
	assert( pEffect );
	stl::find_and_erase( m_childs,pEffect );
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::ClearChilds()
{
	m_childs.clear();
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::InsertChild( int slot,IParticleEffect *pEffect )
{
	if (slot < 0)
		slot = 0;
	if (slot > (int)m_childs.size())
		slot = (int)m_childs.size();

	assert( pEffect );
	m_childs.insert( m_childs.begin() + slot,pEffect );
}

//////////////////////////////////////////////////////////////////////////
int CParticleEffect::FindChild( IParticleEffect *pEffect ) const
{
	for (int i = 0; i < (int)m_childs.size(); i++)
	{
		if (m_childs[i] == pEffect)
		{
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::LoadResources( bool bRecursive )
{
	if (m_bLoaded)
		return;
	m_bLoaded = true;
	
	for (int i = 0; i < NUM_PARTICLE_PROCESSES; i++)
	{
		if (!m_material[i].empty() && !m_pMaterials[i])
			AssignMaterial( i );

		bool bNeedAnimatedTex = m_particleParams[i].nTexAnimFramesCount > 0;
		// First unload what is loaded.
		if (m_particleParams[i].nTexId != 0)
		{
			GetRenderer()->RemoveTexture( m_particleParams[i].nTexId );
			m_particleParams[i].nTexId = 0;
			if(m_particleParams[i].pAnimTex)
				GetRenderer()->RemoveAnimatedTexture(m_particleParams[i].pAnimTex);
			m_particleParams[i].pAnimTex = 0;
		}
		if (m_particleParams[i].pStatObj)
		{
			Get3DEngine()->ReleaseObject( m_particleParams[i].pStatObj );
			m_particleParams[i].pStatObj = 0;
		}

		// Load textures.
		if (!m_texture[i].empty())
		{
			if (bNeedAnimatedTex)
			{
				int texid = GetRenderer()->LoadAnimatedTexture( m_texture[i].c_str(),m_particleParams[i].nTexAnimFramesCount );
				m_particleParams[i].nTexId = texid;
				m_particleParams[i].pAnimTex = GetRenderer()->GetAnimTexInfoFromId(texid);
				if(!m_particleParams[i].pAnimTex)
				{
#if !defined(LINUX)
					Warning( 0,0,"ParticleEffect %s, Use Invalid Animated Texture Id %d",m_name.c_str(),texid );
#endif
					return;
				}
			}
			else
			{
#if defined(NULL_RENDERER)
				m_particleParams[i].nTexId = 0;
#else
      	m_particleParams[i].nTexId = GetRenderer()->LoadTexture( m_texture[i].c_str() );
#endif
				m_particleParams[i].pAnimTex = 0;
			}
			m_bAnimatedTexture[i] = bNeedAnimatedTex;
		}
		// Load geometry.
		if (!m_geometry[i].empty())
		{
			m_particleParams[i].pStatObj = Get3DEngine()->MakeObject( m_geometry[i].c_str() );
		}
	}
	if (bRecursive)
	{
		for (int i = 0; i < (int)m_childs.size(); i++)
		{
			IParticleEffect *pChild = m_childs[i];
			((CParticleEffect*)pChild)->LoadResources( bRecursive );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::UnloadResources( bool bRecursive )
{
	if (!m_bLoaded)
		return;
	m_bLoaded = false;

	for (int i = 0; i < NUM_PARTICLE_PROCESSES; i++)
	{
		if (m_particleParams[i].nTexId != 0)
		{
			GetRenderer()->RemoveTexture( m_particleParams[i].nTexId );
			m_particleParams[i].nTexId = 0;
			if(m_particleParams[i].pAnimTex)
				GetRenderer()->RemoveAnimatedTexture(m_particleParams[i].pAnimTex);
			m_particleParams[i].pAnimTex = 0;
		}
		if (m_particleParams[i].pStatObj)
		{
			Get3DEngine()->ReleaseObject( m_particleParams[i].pStatObj );
			m_particleParams[i].pStatObj = 0;
		}
	}
	if (bRecursive)
	{
		for (int i = 0; i < (int)m_childs.size(); i++)
		{
			IParticleEffect *pChild = m_childs[i];
			((CParticleEffect*)pChild)->UnloadResources( bRecursive );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::SetSoundParams( const SoundParams &params )
{
	m_sound = params.szSound;
	m_soundVolume = params.volume;
	m_soundMinRadius = params.minRadius;
	m_soundMaxRadius = params.maxRadius;
	m_bSoundLoop = params.bLoop;
	m_bOnEverySpawn = params.bOnEverySpawn;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::GetSoundParams( SoundParams &params ) const
{
	params.szSound = m_sound.c_str();
	params.volume = m_soundVolume;
	params.minRadius = m_soundMinRadius;
	params.maxRadius = m_soundMaxRadius;
	params.bLoop = m_bSoundLoop;
	params.bOnEverySpawn = m_bOnEverySpawn;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::Spawn( const Vec3 &pos,const Vec3 &dir,float fScale )
{
	if (m_bEnabled)
	{
		// Spawn emitter for this particle.
		if (!IsResourcesLoaded())
		{
			LoadResources();
		}
		if (m_particleParams[1].nCount > 0)
		{
			m_particleParams[0].pChild = &m_particleParams[1];
		}
		else
			m_particleParams[0].pChild = 0;

		m_particleParams[0].pEntity = 0;
		m_particleParams[1].pEntity = 0;
	
		// Spawn particle system emitter.
		CParticleEmitter *pEmitter = new CParticleEmitter(m_pPartManager);
		pEmitter->m_bPermament = false;
		pEmitter->AssignEffect( this,false );
		pEmitter->SetPos( pos,dir,fScale );
	}
	// Spawn child effects.
	for (int i = 0; i < (int)m_childs.size(); i++)
	{
		m_childs[i]->Spawn( pos,dir,fScale );
	}
}


//////////////////////////////////////////////////////////////////////////
bool CParticleEffect::PrepareSpawn( const Vec3 &pos )
{
	if (!m_bEnabled)
		return false;

	if (!IsResourcesLoaded())
	{
		LoadResources();
	}

/*
	// Play sound if not looped.
	if (!m_bSoundLoop && !m_sound.empty())
	{
		ISound *pSound = GetSystem()->GetISoundSystem()->LoadSound( m_sound.c_str(),FLAG_SOUND_3D );
		if (pSound)
		{
			pSound->SetVolume( (int)m_soundVolume );
			pSound->SetMinMaxDistance( m_soundMinRadius,m_soundMaxRadius );
			pSound->SetPosition(pos);
			pSound->Play();
		}
	}
*/

	// Init child process pointer.
	if (m_particleParams[1].nCount > 0)
		m_particleParams[0].pChild = &m_particleParams[1];
	else
		m_particleParams[0].pChild = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CParticleEffect::IsResourcesLoaded()
{
	bool bAnimatedTex0 = m_particleParams[0].nTexAnimFramesCount > 0;
	bool bAnimatedTex1 = m_particleParams[1].nTexAnimFramesCount > 0;
	if (bAnimatedTex0 != m_bAnimatedTexture[0] || bAnimatedTex1 != m_bAnimatedTexture[1])
	{
		UnloadResources(false);
	}

	return m_bLoaded;
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::SetMaterial( int process,IMatInfo *pMaterial )
{
	assert( process >= 0 && process < NUM_PARTICLE_PROCESSES );
	m_pMaterials[process] = pMaterial;
	if (pMaterial)
		pMaterial->SetFlags(pMaterial->GetFlags()|MIF_WASUSED);
	m_particleParams[process].pMaterial = pMaterial;
	if (pMaterial)
	{
		m_material[process] = pMaterial->GetName();
	}
	else
		m_material[process] = "";
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::SetMaterialName( int process,const char *sMtlName )
{
	assert( process >= 0 && process < NUM_PARTICLE_PROCESSES );
	m_material[process] = sMtlName;
	AssignMaterial( process );
}

//////////////////////////////////////////////////////////////////////////
const char* CParticleEffect::GetMaterialName( int process ) const
{
	assert( process >= 0 && process < NUM_PARTICLE_PROCESSES );
	return m_material[process].c_str();
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffect::AssignMaterial( int process )
{
	assert( process >= 0 && process < NUM_PARTICLE_PROCESSES );
	IMatInfo *pMtl = 0;
	if (m_material[process].empty())
	{
		pMtl = 0;
	}
	else
	{
		pMtl = Get3DEngine()->FindMaterial( m_material[process].c_str() );
		if (!pMtl)
		{
#if !defined(LINUX)
			Warning( 0,0,"ParticleEffect %s material assign failed, Material %s not found",m_name.c_str(),m_material[process].c_str() );
#endif
		}
	}
	if(pMtl)
		pMtl->SetFlags(pMtl->GetFlags()|MIF_WASUSED);
	m_pMaterials[process] = pMtl;
	m_particleParams[process].pMaterial = pMtl;
}
