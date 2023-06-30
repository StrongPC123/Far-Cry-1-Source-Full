////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   partman.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: manage particles and sprites
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "partman.h"
#include "objman.h"
#include "visareas.h"
#include "ParticleEffect.h"
#include "3dEngine.h"

#define PARTICLES_FILE_TYPE 2
#define PARTICLES_FILE_VERSION 4
#define PARTICLES_FILE_SIGNATURE "CRY"

#define ACTIVE_TIME 1

#define EFFECTS_FOLDER "CRY"

//////////////////////////////////////////////////////////////////////////
#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
struct SExportedParticlesHeader
{
	char signature[3];	// File signature.
	int filetype;				// File type.
	int	version;				// File version.
};

struct SExportParticleSound
{
	enum {
		LOOP = 0x01,
		EVERY_SPAWN = 0x02,
	};
	char soundfile[64];
	float volume;
	float minRadius;
	float maxRadius;
	char nSoundFlags;
};

enum EParticleExportFlags
{
	PARTICLE_EFFECT_DISABLED = 0x01
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Particle system parameters
struct SExportParticleParams
{
	Vec3 vPosition; // spawn position
	Vec3 vDirection; // initial direction  (normalization not important)
	float fFocus; // if 0 - particles go in all directions, if more than 20 - particles go mostly in vDirection
	Vec3 vColorStart; // initial color
	Vec3 vColorEnd; // final color
	FloatVariant fSpeed; // initial speed ( +- 25% random factor applyed, m/sec )
	float fSpeedFadeOut; // Time in which before end of life time speed decreases from normal to 0.
	float fSpeedAccel;	// Constant speed acceleration along particle heading.
	float fAirResistance; // Air resistance.
	Vec3Variant vRotation; // rotation speed (degree/sec)
	Vec3Variant vInitAngles; // initial rotation
	int   nCount; // number of particles to spawn
	FloatVariant fSize; // initial size of particles
	float fSizeSpeed; // particles will grow with this speed
	float fSizeFadeIn; // Time in which at the begning of life time size goes from 0 to fSize.
	float fSizeFadeOut; // Time in which at the end of life time size goes from fSize to 0.
	float fThickness;	// lying thickness - for physicalized particles only
	FloatVariant fLifeTime; // time of life of particle
	float fFadeInTime; // particle will fade in slowly during this time
	int   nTexAnimFramesCount; // number of frames in animated texture ( 0 if no animation )
	ParticleBlendType eBlendType; // see ParticleBlendType
	float fTailLenght; // delay of tail ( 0 - no tail, 1 meter if speed is 1 meter/sec )
	float fStretch; // Stretch particles into moving direction.
	int   nParticleFlags; // see particle system flags
	bool  bRealPhysics; // use physics engine to control particles
	float fChildSpawnPeriod; // if more than 0 - run child process every x seconds, if 0 - run it at collision
	float fChildSpawnTime; // if more then 0, Spawn child process for max this ammount of time.
	int   nDrawLast; // add this element into second list and draw this list last
	float fBouncenes; // if 0 - particle will not bounce from the ground, 0.5 is good in most cases
	float  fTurbulenceSize; // radius of turbulence
	float  fTurbulenceSpeed; // speed of rotation
	float fDirVecScale; //the game need to store this(Alberto)
	float fPosRandomOffset; // maximum distance of random offset from original position

	//////////////////////////////////////////////////////////////////////////
	// New parameters, used by Particle effects.
	//////////////////////////////////////////////////////////////////////////
	//! Spawn Position offset from effect spawn position.
	Vec3 vPositionOffset;
	//! Random offset of particle relative to spawn position.
	Vec3 vRandomPositionOffset;
	//! Delay actual spawn time by this ammount.
	FloatVariant fSpawnDelay;
	//! Life time of emitter.
	FloatVariant fEmitterLifeTime;
	//! When using emitter, spawn time between between 2 particle bursts.
	float fSpawnPeriod;

	//! Global effect scale. (0 ignored)
	float fScale;
	//! Object scale, multiplied with fSize to give scale adjustment between object and texture.
	//! 0 not affect fSize.
	float fObjectScale;

	Vec3 vNormal; // lying normal - for physicalized particles only
	int iPhysMat; // material for physicalized particles
	Vec3 vGravity; // gravity(wind) vector

	//////////////////////////////////////////////////////////////////////////
	// Added.
	//////////////////////////////////////////////////////////////////////////
	unsigned short nTailSteps;
	//////////////////////////////////////////////////////////////////////////
	// Reserve space for new members.
	//////////////////////////////////////////////////////////////////////////
	char reserve[126];
};

struct SExportParticleEffect
{
	char name[64];
	char texture[2][64];
	char geometry[2][64];
	char material[2][64];
	SExportParticleParams params[2]; // Primary and Child params.
	SExportParticleSound sound;
	int parent;	// Index of parent particle.
	int flags; // General flags.
};
#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CPartManager::CPartManager( ) 
{
	/*
  // fill list of free particles
  for(int i=0; i<MAX_PARTICLES_COUNT; i++)
  {
    m_arrParts[i].m_nId = i;
    m_FreeParticles.Add(&m_arrParts[i]);
  }
	*/
	m_currParticlesTime = 0;
	m_currParticlesFrameTime = 0;
	m_smoothTimeCount = 0;
	memset(m_smoothTimes,0,sizeof(m_smoothTimes));

  m_pSpriteMan = new CSpriteManager(this);

  m_nGlowTexID = GetRenderer()->GenerateAlphaGlowTexture(1);

  m_pPartLightShader = GetRenderer()->EF_LoadShader("ParticleLight", eSH_World, EF_SYSTEM);

//  CPartSpray::InitVertBuffers(GetRenderer());
}

CPartManager::~CPartManager() 
{ 
/*  for(int i=0; i<m_lstpPartEmitters.Count(); i++)
  {
    m_lstpPartEmitters[i]->Shutdown();
    delete m_lstpPartEmitters[i];
    m_lstpPartEmitters.Delete(i);
    i--;
  }
	*/
	/*
  while(m_FreeParticles.GetFirst())
    m_FreeParticles.Remove(m_FreeParticles.GetFirst());
		*/

	Reset();
  delete m_pSpriteMan;

  //CPartSpray::DeleteVertBuffers(GetRenderer());
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::Spawn( CParticleEmitter *pEmitter,bool bChildProcess )
{
	if(!GetCVars()->e_particles)
		return;

	if(GetCVars()->e_particles_debug==2)
		return;

	//GetISystem()->VTuneResume();
	if(pEmitter->m_pParams && pEmitter->m_pParams->nParticleFlags & PART_FLAG_NO_DRAW_UNDERWATER)
	{
		pEmitter->CalculateWaterLevel();
		if(pEmitter->m_fWaterLevel>pEmitter->m_pos.z)
			return;
	}

	//////////////////////////////////////////////////////////////////////////
	Vec3d vCamPos = GetViewCamera().GetPos();
	float fMaxViewDist = Get3DEngine()->GetMaxViewDist();
	
	ParticleParams &Params = *pEmitter->m_pParams;
	if (!bChildProcess)
	{
		Params.vPosition = pEmitter->m_pos;
		Params.vDirection = pEmitter->m_dir;
		//Params.pMaterial = pEmitter->m_pMaterial;
		Params.pEntity = pEmitter->m_pSpawnerEntity;

		// Check distance to camera with MaxViewDistance.
		if (GetLengthSquared(Params.vPosition-vCamPos) > fMaxViewDist*fMaxViewDist)
			return;
	}
	else
	{
		if (!pEmitter->m_pChildParams)
			return;
		// Check distance to camera with MaxViewDistance.
		if (GetLengthSquared(pEmitter->m_pChildParams->vPosition-vCamPos) > fMaxViewDist*fMaxViewDist)
			return;
	}

	// make sprites
	pEmitter->OnSpawnParticles(bChildProcess);
	m_pSpriteMan->Spawn( *pEmitter,bChildProcess );

	//GetISystem()->VTunePause();
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::Spawn( const ParticleParams &Params,float fMaxViewDist,CObjManager * pObjManager,bool bNoEmitter ) 
{
//	if(Params.fLifeTime<0.5 && Params.pStatObj)
	//	return;

//	Params.pStatObj = pObjManager->MakeObject("objects/box.cgf");
	//Params.vInitAngles = Params.pEntity->GetAngles();
 /*
	if((Params.nParticleFlags & PART_FLAG_HORIZONTAL) && fabs(Params.vPosition.z - Get3DEngine()->GetWaterLevel())<1.f)
	{
		GetRenderer()->EF_AddSplash(Params.vPosition, EST_Water, Params.fSize+Params.fSizeSpeed);
		return;
	}
	 */

  if(!GetCVars()->e_particles)
    return;

  if(GetCVars()->e_particles_debug==2)
    return;

	IParticleEmitter *pEmitter = new CParticleEmitter( this );
	pEmitter->SetParams( Params );
	ActivateEmitter( (CParticleEmitter*)pEmitter );
}

void CPartManager::UpdateMan(CObjManager * pObjManager, CTerrain * pTerrain, int nRecursionLevel)
{ 
	// function not used
	assert(0);
//  if(!GetCVars()->e_particles || nRecursionLevel)
    return;

//  GetRenderer()->ResetToDefault();

  { // update/render particle emitters
/*    for(int i=0; i<m_lstpPartEmitters.Count(); i++)
      if(!m_lstpPartEmitters[i]->Update(pObjManager, pTerrain, nRecursionLevel, &m_FreeParticles))
      { // remove if not used
        m_lstpPartEmitters[i]->Shutdown();
        delete m_lstpPartEmitters[i];
        m_lstpPartEmitters.Delete(i);
        i--;
      }*/
  }

//  GetRenderer()->ResetToDefault();

  // Draw sprites
  // get orientation for billboard particles
  Matrix44 mat;
  GetRenderer()->GetModelViewMatrix(mat.GetData());
	Vec3d
		vRight = mat.GetColumn(0),
		vUp    = mat.GetColumn(1),
		vFront = mat.GetColumn(2);

	//CELL_CHANGED_BY_IVO
	//vRight(mat.cell(0), mat.cell(4), mat.cell(8));
	//vUp   (mat.cell(1), mat.cell(5), mat.cell(9)); 
	//vFront(mat.cell(2), mat.cell(6), mat.cell(10)); 

	
	/*
  // set rendering state
  GetRenderer()->SetDepthFunc(R_LEQUAL);
  GetRenderer()->EnableBlend(true);
  GetRenderer()->EnableDepthWrites(false);
  GetRenderer()->SetEnviMode(R_MODE_MODULATE);
  GetRenderer()->SetCullMode(R_CULL_DISABLE);

	// set world color
  Vec3d vColor = GetSystem()->GetI3DEngine()->GetWorldColor();
  GetRenderer()->SetMaterialColor(vColor.x,vColor.y,vColor.z,1);
*/
//  float fTime = (float)GetCurTimeSec();
  Vec3d vCamPos = GetViewCamera().GetPos();

  	//////////////////////////////////////////////////////////////////////////
  //CCamera * pCamera = &GetViewCamera();
  //bool bTestVis = (nRecursionLevel==0) && ((GetFrameID()&3)==0);
  //CVars * pCVars = GetCVars();

  m_pSpriteMan->Render(pObjManager, pTerrain, nRecursionLevel, this, m_pPartLightShader);

  GetRenderer()->ResetToDefault();
}

int CPartManager::Count(int * pCurSpritesCount, int * pCurFreeCount, int * pCurEmitCount) 
{ 
  int nSumm=0;

//  for(int i=0; i<m_lstpPartEmitters.Count(); i++)
  //  nSumm += m_lstpPartEmitters[i]->Count(); 

  if(pCurSpritesCount)
    *pCurSpritesCount = m_pSpriteMan->m_nCurSpritesCount;

	/*
  if(pCurFreeCount)
    *pCurFreeCount = m_FreeParticles.Count();
	*/

  if(pCurEmitCount)
    *pCurEmitCount = 0;//m_lstpPartEmitters.Count();

  return nSumm;
}

void CPartManager::Render(CObjManager * pObjManager, CTerrain * pTerrain) 
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	int nRecursionLevel = pObjManager->m_nRenderStackLevel;

	float fFrameTime = nRecursionLevel ? 0 : GetTimer()->GetFrameTime();
	if(fFrameTime>0.2f)
		fFrameTime=0.2f;
	if(fFrameTime<0)
		fFrameTime=0;

	//////////////////////////////////////////////////////////////////////////
	// Smooth frame time if not rendered in recursion.
	//////////////////////////////////////////////////////////////////////////
	if (nRecursionLevel == 0)
	{
		m_smoothTimes[m_smoothTimeCount] = fFrameTime;

		m_smoothTimeCount++;
		if(m_smoothTimeCount >= PARTICLES_SMOOTH_FRAMES)
			m_smoothTimeCount = 0;

		// average multiple frames together to smooth changes out a bit
		float fTotalTime = 0;
		for (int i = 0 ; i < PARTICLES_SMOOTH_FRAMES; i++ ) 
			fTotalTime += m_smoothTimes[i];	

		fFrameTime = fTotalTime/PARTICLES_SMOOTH_FRAMES;
	}

	if (GetCVars()->e_particles_debug==2)
		fFrameTime = 0;

	if (nRecursionLevel == 0)
	{
		m_currParticlesTime += fFrameTime;
	}
	m_currParticlesFrameTime = fFrameTime;

	//////////////////////////////////////////////////////////////////////////
	// Update all emitters.
	//////////////////////////////////////////////////////////////////////////
	if (pObjManager->m_nRenderStackLevel == 0)
	{
		UpdateEmitters();
	}

	//GetISystem()->VTuneResume();
  if(m_pSpriteMan)
		m_pSpriteMan->Render(pObjManager, pTerrain, nRecursionLevel, this, m_pPartLightShader);
	//GetISystem()->VTunePause();
}
/*
C_PartEmitter * CPartManager::CreateParticleEmitter(ParticleParams Params, float fSpawnPeriod, float fLifeTime)
{
  if(!Params.nTexId)
    Params.nTexId = m_nGlowTexID;

  C_PartEmitter * pTmp = 0;//new C_PartEmitter(m_pSystem,this,Params, fSpawnPeriod, fLifeTime);
//  m_lstpPartEmitters.Add(pTmp);
  
  return pTmp;
}	*/
/*
void CPartManager::DeleteParticleEmitter(C_PartEmitter * pPartEmitter)
{
//  delete pPartEmitter;
  //m_lstpPartEmitters.Delete(pPartEmitter);
}	*/

void CPartManager::GetMemoryUsage(ICrySizer*pSizer)const
{
	pSizer->Add (*this);

	if(m_pSpriteMan)
		pSizer->AddObject(m_pSpriteMan, sizeof(*m_pSpriteMan));
}

void CPartManager::Reset()
{
	int i;
	bool bActive = true;
	IPhysicalWorld *pPhysWorld = Cry3DEngineBase::GetPhysicalWorld();
	for (i = 0; i < m_pSpriteMan->m_nCurSpritesCount && i<m_pSpriteMan->m_nMaxSpritesCount; i++)
		m_pSpriteMan->m_arrSprites[i].DeActivateParticle( pPhysWorld );
	m_pSpriteMan->m_nCurSpritesCount=0;

	// Clear all emitters.
	for (ActiveEmitters::iterator it = m_activeEmitters.begin(); it != m_activeEmitters.end(); ++it)
	{
		CParticleEmitter *pEmitter = *it;
		pEmitter->OnActivate( false );
	}
	m_activeEmitters.clear();
	
	//////////////////////////////////////////////////////////////////////////
	// Unload resources for all effects.
	//////////////////////////////////////////////////////////////////////////
	for (i = 0; i < (int)m_effects.size(); i++)
	{
		IParticleEffect *pEffect = m_effects[i];
		((CParticleEffect*)pEffect)->UnloadResources(false);
	}
}

void CPartManager::OnEntityDeleted(IEntityRender * pEntityRender)
{
  if(m_pSpriteMan)
    m_pSpriteMan->OnEntityDeleted(pEntityRender);

	{ // delete active emitters of this entity
		ActiveEmitters::iterator next;
		for (ActiveEmitters::iterator it = m_activeEmitters.begin(); it != m_activeEmitters.end(); it = next)
		{
			next = it;
			next++;
			CParticleEmitter *pEmitter = *it;
			if(pEmitter->m_pSpawnerEntity == pEntityRender)
			{
				pEmitter->OnActivate( false );
				m_activeEmitters.erase(it);
			}
		}
	}

	{ // delete global emitters of this entity
		ParticleEmitters::iterator next;
		for (ParticleEmitters::iterator it = m_allEmitters.begin(); it != m_allEmitters.end(); it = next)
		{
			next = it;
			next++;
			CParticleEmitter *pEmitter = *it;
			if(pEmitter->m_pSpawnerEntity == pEntityRender)
			{
				pEmitter->OnActivate( false );
				m_allEmitters.erase( it );
			}
		}
	}
}

void CSpriteManager::OnEntityDeleted(IEntityRender * pEntityRender)
{
  for( int i=0; i<m_nCurSpritesCount && i<m_nMaxSpritesCount; i++)
  {
    if(m_arrSprites[i].m_pSpawnerEntity == pEntityRender)
    { // remove
			m_arrSprites[i].DeActivateParticle( Cry3DEngineBase::GetPhysicalWorld());
      m_arrSprites[i].m_pSpawnerEntity=0;

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

//////////////////////////////////////////////////////////////////////////
// Particle Effects.
//////////////////////////////////////////////////////////////////////////
IParticleEffect* CPartManager::CreateEffect()
{
	IParticleEffect_AutoPtr pEffect = new CParticleEffect( this );
	m_effects.push_back( pEffect );
	return pEffect;
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::RenameEffect( IParticleEffect *pEffect,const char *sNewName )
{
	assert( pEffect );
	const char *sOldName = pEffect->GetName();
	if (strlen(sOldName) > 0)
	{
		// Delete old name.
		m_effectsMap[sOldName] = 0;
	}
	if (strlen(sNewName) > 0)
	{
		// Add new name.
		m_effectsMap[sNewName] = pEffect;
	}
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::RemoveEffect( IParticleEffect *pEffect )
{
	assert( pEffect );
	const char *sOldName = pEffect->GetName();
	if (strlen(sOldName) > 0)
	{
		// Delete old name.
		m_effectsMap[sOldName] = 0;
	}
	stl::find_and_erase( m_effects,pEffect );
}

//////////////////////////////////////////////////////////////////////////
IParticleEffect* CPartManager::FindEffect( const char *sEffectName )
{
	IParticleEffect *pEffect = stl::find_in_map(m_effectsMap,sEffectName,(IParticleEffect*)NULL);
	return pEffect;
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::UpdateEmitters()
{
	float fTime = GetParticlesTime();
	CCamera *pCamera = &GetViewCamera();

	ActiveEmitters::iterator next;
	for (ActiveEmitters::iterator it = m_activeEmitters.begin(); it != m_activeEmitters.end(); it = next)
	{
		next = it;
		next++;
		
		_smart_ptr<CParticleEmitter> pEmitter = (*it);
		CParticleEmitter &emitter = *pEmitter;

		/*
		// Check if emitter is visible.
		if (pCamera->IsAABBVisibleFast(emitter.m_bbox))
			emitter.m_bVisible = true;
		else
			emitter.m_bVisible = false;
		// Reset bounding box.
		emitter.m_bbox.Reset();
				*/

		float fSpawnPeriod = emitter.m_spawnPeriod/max(0.01f,GetCVars()->e_particles_lod) + (1.f-GetCVars()->e_particles_lod)*0.1f;
		if (fTime > emitter.m_startTime || emitter.m_startTime == 0)
		{
			if (fTime - emitter.m_lastSpawnTime > fSpawnPeriod || fSpawnPeriod == 0)
			{
				// Remember last spawn time.
				emitter.m_lastSpawnTime = fTime;

				if (!pEmitter->m_childEmitters.empty())
					pEmitter->UpdateChildSpawnTimes(fTime);

				// Spawn particles from this emitter.
				if (emitter.m_pEffect)
				{
					CParticleEffect* pEffect = (CParticleEffect*)((IParticleEffect*)emitter.m_pEffect);
					if (!pEffect->PrepareSpawn( pEmitter->m_pos ))
						continue;
				}
				Spawn( pEmitter );
			}
		}
		if (!pEmitter->m_childEmitters.empty())
		{
			for (int i = 0; i < (int)pEmitter->m_childEmitters.size(); i++)
			{
				CParticleEmitter *pChildEmitter = pEmitter->m_childEmitters[i];
			
				/*
				// Check if emitter is visible.
				if (pCamera->IsAABBVisibleFast(pChildEmitter->m_bbox))
					pChildEmitter->m_bVisible = true;
				else
					pChildEmitter->m_bVisible = false;
				pChildEmitter->m_bbox.Reset();
				*/

				if (!pChildEmitter->m_bActive || !pChildEmitter->m_bActiveChild)
					continue;

				if (fTime < pChildEmitter->m_startTime && emitter.m_startTime != 0)
					continue;

				// Check if needs to Deactivate emitter, on next update, spawn particles on this update anyway.
				if (fTime > pChildEmitter->m_endTime)
					pChildEmitter->m_bActiveChild = false;

				float spawnPeriod = pChildEmitter->m_spawnPeriod/max(0.01f,GetCVars()->e_particles_lod) + (1.f-GetCVars()->e_particles_lod)*0.1f;
				if (pChildEmitter->m_pParams->fEmitterLifeTime <= 0 && pChildEmitter->m_pParams->fSpawnDelay <= 0)
				{
					float spawnPeriodMul = pChildEmitter->m_pParams->fSpawnPeriod;
					if (spawnPeriodMul == 0)
						spawnPeriodMul = 1;
					spawnPeriod = fSpawnPeriod * spawnPeriodMul;
				}

				if (fTime - pChildEmitter->m_lastSpawnTime > spawnPeriod)
				{
					// Remember last spawn time.
					pChildEmitter->m_lastSpawnTime = fTime;
					pChildEmitter->m_pos = pEmitter->m_pos;
					pChildEmitter->m_dir = pEmitter->m_dir;
					pChildEmitter->m_fScale = pEmitter->m_fScale;
					// Spawn particles from this emitter.
					if (emitter.m_pEffect)
					{
						CParticleEffect* pEffect = (CParticleEffect*)((IParticleEffect*)pChildEmitter->m_pEffect);
						if (!pEffect->PrepareSpawn( pEmitter->m_pos ))
							continue;
					}
					Spawn( pChildEmitter );
				}
			}
		}

		// If emitter end time == 0 do not kill this emitter.
		if (fTime > emitter.m_endTime && emitter.m_bUseEndTime)
		{
			pEmitter->OnActivate( false );
			// Times up, delete this emitter.
			m_activeEmitters.erase( it );
			// Also delete it from total emitters.
			DeleteEmitter( pEmitter );
			continue;
		}
		else
		{
			if (emitter.m_bPermament && fTime > emitter.m_lastActiveTime+ACTIVE_TIME)
			{
				// This emitter should get deactivated.
				pEmitter->OnActivate( false );
				m_activeEmitters.erase( it );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
#define PATICLEPARAMS_COPY_HELPER( inparam,outparam,name ) outparam.name = inparam.name;

//////////////////////////////////////////////////////////////////////////
static void ExportDataToParticleParams( const SExportParticleParams &inp,ParticleParams &outp )
{
	PATICLEPARAMS_COPY_HELPER( inp,outp,vPosition );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vDirection );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fFocus );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vColorStart );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vColorEnd );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpeed );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpeedFadeOut );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpeedAccel );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fAirResistance );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vRotation );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vInitAngles );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nCount );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSize );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSizeSpeed );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSizeFadeIn );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSizeFadeOut );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fThickness );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fLifeTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fFadeInTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nTexAnimFramesCount );
	PATICLEPARAMS_COPY_HELPER( inp,outp,eBlendType );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fTailLenght );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nTailSteps );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fStretch );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nParticleFlags );
	PATICLEPARAMS_COPY_HELPER( inp,outp,bRealPhysics );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fChildSpawnPeriod );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fChildSpawnTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nDrawLast );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fBouncenes );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fTurbulenceSize );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fTurbulenceSpeed );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fDirVecScale );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fPosRandomOffset );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vPositionOffset );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vRandomPositionOffset );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpawnDelay );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fEmitterLifeTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpawnPeriod );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fScale );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fObjectScale );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vNormal );
	PATICLEPARAMS_COPY_HELPER( inp,outp,iPhysMat );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vGravity );
}

//////////////////////////////////////////////////////////////////////////
bool CPartManager::LoadParticles( CCryFile &file )
{
	//UpdateLoadingScreen( "\003Loading Particle Effects..." );

	SExportedParticlesHeader header;
	file.Read( &header,sizeof(header) );
	if (strncmp(header.signature,PARTICLES_FILE_SIGNATURE,sizeof(header.signature)) != 0)
	{
		// Bad signature.
		Warning( 0,file.GetFilename(),"Cannot Load Particles,Wrong File Signature %s",file.GetFilename() );
		return false;
	}
	if (header.filetype != PARTICLES_FILE_TYPE)
	{
		// Bad signature.
		Warning( 0,file.GetFilename(),"Cannot Load Particles,Wrong File Type %s",file.GetFilename() );
		return false;
	}
	if (header.version != PARTICLES_FILE_VERSION)
	{
		// Bad signature.
		Warning( 0,file.GetFilename(),"Cannot Load Particles,Wrong File Version %s",file.GetFilename() );
		return false;
	}

	int numItems;
	file.Read( &numItems,sizeof(numItems) );

	std::vector<SExportParticleEffect> exportedEffects;
	std::vector<IParticleEffect*> libEffects;

	exportedEffects.resize( numItems );
	libEffects.resize( numItems );

	int fileSize = file.GetLength();
	if (numItems*sizeof(SExportParticleEffect) != fileSize-sizeof(header)-sizeof(numItems))
	{
		Warning( 0,file.GetFilename(),"Cannot Load Particles,Corrupted File %s",file.GetFilename() );
		return false;
	}
	if (numItems > 0)
	{
		file.Read( &exportedEffects[0],numItems*sizeof(SExportParticleEffect) );
	}

	//UpdateLoadingScreen( "Loading Particles %s",file.GetFilename() );
	int i;
	// Initialize effects.
	for (i = 0; i < numItems; i++)
	{
		IParticleEffect_AutoPtr pEffect = CreateEffect();
		libEffects[i] = pEffect;
		for (int p = 0; p < IParticleEffect::NUM_PARTICLE_PROCESSES; p++)
		{
			ParticleParams &params = pEffect->GetParticleParams(p);
			ExportDataToParticleParams( exportedEffects[i].params[p],params );
			// Reset data that must be loaded.
			params.nTexId = 0;
			params.pStatObj = 0;
			params.pMaterial = 0;
			params.pChild = 0;
			params.pAnimTex = 0;
			params.pEntity = 0;
			params.pShader = 0;

			pEffect->SetTexture( p,exportedEffects[i].texture[p] );
			pEffect->SetGeometry( p,exportedEffects[i].geometry[p] );
			pEffect->SetMaterialName( p,exportedEffects[i].material[p] );

			// Load Resources at this point.
		}
		if (exportedEffects[i].flags & PARTICLE_EFFECT_DISABLED)
		{
			pEffect->SetEnabled(false);
		}
		
		IParticleEffect::SoundParams sndParams;
		sndParams.szSound = exportedEffects[i].sound.soundfile;
		sndParams.volume = exportedEffects[i].sound.volume;
		sndParams.minRadius = exportedEffects[i].sound.minRadius;
		sndParams.maxRadius = exportedEffects[i].sound.maxRadius;
		sndParams.bLoop = (exportedEffects[i].sound.nSoundFlags & SExportParticleSound::LOOP) != 0;
		sndParams.bOnEverySpawn = (exportedEffects[i].sound.nSoundFlags & SExportParticleSound::EVERY_SPAWN) != 0;
		pEffect->SetSoundParams(sndParams);

		pEffect->SetName( exportedEffects[i].name );
		pEffect->LoadResources();
	}
	// Link to parents.
	for (i = 0; i < (int)exportedEffects.size(); i++)
	{
		int parent = exportedEffects[i].parent;
		if (parent >= 0)
		{
			IParticleEffect* pEffect = libEffects[i];
			IParticleEffect* pParentEffect = libEffects[parent];
			pParentEffect->AddChild( pEffect );
		}
	}

	UpdateLoadingScreen( "\003Loaded %d Particle Effects from %s.",numItems,file.GetFilename() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
IParticleEmitter* CPartManager::CreateEmitter()
{
	CParticleEmitter *pEmitter =  new CParticleEmitter( this );
	pEmitter->m_bPermament = true;
	m_allEmitters.insert( pEmitter );
	return pEmitter;
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::DeleteEmitter( IParticleEmitter *pEmitter )
{
	assert( pEmitter );

	CParticleEmitter *pEmt = (CParticleEmitter*)pEmitter;
	//////////////////////////////////////////////////////////////////////////
	if (pEmt->m_bPermament)
		m_allEmitters.erase( pEmt );
	// delete emitter from list.
	if (pEmt->m_bActive)
		stl::find_and_erase( m_activeEmitters,pEmt );
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::ActivateEmitter( CParticleEmitter *pEmitter )
{
	if (pEmitter!=NULL && !pEmitter->m_bActive)
	{
		pEmitter->OnActivate( true );
		m_activeEmitters.push_back( pEmitter );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::LoadSharedParticleLibrary( const char *sEffectsFolder,const char *sLibName )
{
	if (m_loadedLibs.find(sLibName) != m_loadedLibs.end())
	{
		// Already loaded.
		return;
	}

	char filename[_MAX_PATH];
	_makepath( filename,NULL,sEffectsFolder,sLibName,"prt" );

	CCryFile file;
	if (file.Open( filename,"rb" ))
	{
		LoadParticles( file );
		m_loadedLibs.insert( sLibName );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::LoadParticlesLibs( const char *sEffectsFolder,XmlNodeRef &levelDataRoot )
{
	XmlNodeRef libs = levelDataRoot->findChild( "ParticlesLibrary" );
	if (!libs)
		return;

	// Enmerate material libraries.
	for (int i = 0; i < libs->getChildCount(); i++)
	{
		XmlNodeRef libNode = libs->getChild(i);
		XmlString libraryName = libNode->getAttr( "Name");
		LoadSharedParticleLibrary( sEffectsFolder,libraryName );
	}
}