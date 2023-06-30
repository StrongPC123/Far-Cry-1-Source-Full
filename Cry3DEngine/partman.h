////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   partman.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef PART_MANAGER
#define PART_MANAGER

#define MAX_PARTICLES_IN_VIDEO_BUFFER 16
#define MAX_VIDEO_BUFFERS_PER_SPRAY   4

#define PARTICLES_SMOOTH_FRAMES 20

#define PART_MAX_HISTORY_ELEMENTS 256

#define rn() ((((float)rand())/RAND_MAX)-0.5f)

#include "ParticleEffect.h"
#include "ParticleEmitter.h"

// particle proc params
struct PartProcessParams
{
  PartProcessParams() { memset(this, 0, sizeof(PartProcessParams)); }
  float fCurTime, fFrameTime;
  Vec3d vRight, vUp, vFront;
  IRenderer * pIRenderer;
  CObjManager * pObjManager;
  CTerrain * pTerrain;
  IPhysicalWorld * pPhysicalWorld;
	class C3DEngine *p3DEngine;
  CCamera * pCamera;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pVertBufChunk;
  struct ISystem * pSystem;
	class CPartManager * pPartManager;
	Vec3 vCamPos;
	Vec3d vSpaceFocusPos;
};

class CPartManager;

// dynamic particle data
class CParticle 
{
public:
	enum EAdvancedParticleFlags
	{
		PARTICLE_COLOR_BASED = 0x10000,
		PARTICLE_ADDITIVE = 0x20000,
		PARTICLE_ANIMATED_TEXTURE = 0x40000
	};

	//int m_nId; // debug
	//CParticle *m_pNext, *m_pPrev;

  // cur state
	int m_nParticleFlags;
  Vec3 m_vPos;
  Vec3 m_vDelta;
  float m_fSize;
	float m_fSizeOriginal;
	float m_fScale;
  float m_fSpawnTime;
	float m_fLifeTime;
	float m_fChildSpawnLastTime;
	float m_fTrailCurPos;
	Vec3  m_vAngles;
	Vec3  m_vRotation;
	//int m_nDynLightMask;
	//int m_nFogVolumeId;
	unsigned int m_cAmbientColor;
	//! Override material for this emitter.
	IMatInfo *m_pMaterial;
	//! Entity who controls this emitter.
	IEntityRender *m_pSpawnerEntity;

	//IVisArea * m_pVisArea;
	IPhysicalEntity * m_pPhysEnt;
	ParticleParams* m_pParams;

	//! Emitter who spawned this entity.
	CParticleEmitter* m_pEmitter;

	// For patricles with tail, keeps history of previous positions.
	Vec3 *m_pArrvPosHistory;
	unsigned char m_nTailSteps;
	char m_reserved[3]; // Fill to 32 byte.

  CParticle() 
  {
		//m_nId=0;
		m_nParticleFlags = 0;
    m_vPos(0,0,0);
    m_vDelta(0,0,0);
    m_fSize=0;
		m_fSizeOriginal=0;
    //m_pNext=m_pPrev=0;
    m_fSpawnTime=0;
		m_pPhysEnt=0;
		m_fScale = 1;
		m_fChildSpawnLastTime = 0;
		//m_fTrailCurPos = 0;
		m_vAngles(0,0,0);
		m_vRotation(0,0,0);
		m_cAmbientColor = 0;
		//m_pVisArea = 0;
		//m_nDynLightMask = 0;
		//m_nFogVolumeId = 0;
		m_pMaterial = 0;
		m_pSpawnerEntity = 0;
		m_pArrvPosHistory = 0;
		m_fTrailCurPos = 0;
		m_nTailSteps = 8;
		m_reserved[0] = 0;
		m_reserved[1] = 0;
		m_reserved[2] = 0;
  }

  bool Update(const PartProcessParams & PPP);
  void FillBuffer(const PartProcessParams & PPP);
  void DeActivateParticle(IPhysicalWorld * pPhysicalWorld);
	void Physicalize( ParticleParams &Params,IPhysicalWorld * pPhysicalWorld);
	unsigned int Vec2Color( const Vec3 &v )
	{
		unsigned int r = (FtoI(v.x*255.0f)), g = FtoI(v.y*255.0f), b = FtoI(v.z*255.0f);
		return r|(g<<8)|(b<<16);
	}
	Vec3 Color2Vec( unsigned int c )
	{
		return Vec3( 
			((unsigned char)(c))*0.00392156f, 
			((unsigned char)(c>>8))*0.00392156f, 
			((unsigned char)(c>>16))*0.00392156f );
	}
};


/*
// linked list of particles
class CPartList
{
  CParticle *m_pFirst, *m_pLast;
  int m_nCount;

public:

  CPartList() { memset(this,0,sizeof(CPartList)); }
  ~CPartList() { assert(!m_nCount && !m_pFirst); }
  int Count() { return m_nCount; }
  CParticle * GetFirst() { return m_pFirst; }
  CParticle * GetLast() { return m_pLast; }

  void Add(CParticle * pElem) 
  {
    assert(!pElem->m_pNext && !pElem->m_pPrev);

    if(!m_pFirst)
    { // insert first element
      m_pFirst = m_pLast = pElem;
      pElem->m_pNext = pElem->m_pPrev = 0;
      
      assert(!m_nCount);
    }
    else
    {
      assert(!m_pLast->m_pNext && !m_pFirst->m_pPrev);

      pElem->m_pPrev = m_pLast;
      pElem->m_pNext = 0;
      m_pLast->m_pNext = pElem;
      m_pLast = pElem;

      assert(m_nCount);
    }
    m_nCount++;
  }

  void Remove(CParticle * pElem)
  {
    if(m_pLast == pElem)
      m_pLast = pElem->m_pPrev;

    if(m_pFirst == pElem)
      m_pFirst = pElem->m_pNext;

    if(pElem->m_pNext)
    {
      assert(pElem->m_pNext->m_pPrev == pElem);
      pElem->m_pNext->m_pPrev = pElem->m_pPrev;
    }

    if(pElem->m_pPrev)
    {
      assert(pElem->m_pPrev->m_pNext == pElem);
      pElem->m_pPrev->m_pNext = pElem->m_pNext;
    }

    pElem->m_pPrev = pElem->m_pNext = 0;
    m_nCount--;
  }
};
*/

// sprite contain dynamic data and life style
class CSprite : public CParticle
{
public:
  void Render( const PartProcessParams &PPP, IShader *pShader );
	int FillTailVertBuffer(	SColorVert * pTailVerts, 
													const Vec3d & vCamVec,
													const UCol & ucColor );
};

// manager contain all sprites(in pool) and emitters
class CSpriteManager : public Cry3DEngineBase
{
public:
	CSpriteManager(class CPartManager *pPartManager);
	~CSpriteManager();
  void Spawn( CParticleEmitter &emitter,bool bChildProcess );
  void Render( CObjManager * pObjManager, CTerrain * pTerrain, int nRecursionLevel, CPartManager * pPartManager, IShader * pPartLightShader);
  void OnEntityDeleted(IEntityRender * pEntityRender);

  CSprite * m_arrSprites;//[MAX_SPRITES_COUNT];
  int m_nCurSpritesCount;  
	int m_nMaxSpritesCount;  

private:
	void SpawnParticle( CParticleEmitter &emitter,bool bChildProcess,float fCurrTime,CParticle *pParticle );

	ISystem* m_pSystem;
	I3DEngine* m_p3DEngine;
	CVisAreaManager *m_pVisAreaManager;
	CObjManager *m_pObjManager;
	CPartManager *m_pPartManager;
};

// contain part spray and reference counter
/*class C_PartEmitter : public Cry3DEngineBase, public IPartEmitter
{
  CPartManager * m_pPartManager;
  ParticleParams m_PartParams;
  float m_fSpawnPeriod, m_fLastSpawnTime, m_fLifeEndTime;

public:
  CPartSpray * m_pSpray;
  int m_nUsers;

  C_PartEmitter(CPartManager * pPartManager, const ParticleParams & Params, const float fSpawnPeriod, const float fLifeTime);
  bool  Update(CObjManager * pObjManager, CTerrain * pTerrain, int nRecursionLevel, CPartList * pFreeParticles);
  virtual void Spawn(const Vec3d & vPos);
  void  Shutdown();
  int   Count() { return m_pSpray->Count(); }
};*/

// top class of particle system
class CPartManager : public Cry3DEngineBase
{ 
public:
  CPartManager( );
  ~CPartManager();
  void Spawn( const ParticleParams &Params,float fMaxViewDist, CObjManager * pObjManager,bool bNoEmitter=false );
	void Spawn( CParticleEmitter *pEmitter,bool bChildProcess=false );
  void UpdateMan(CObjManager * pObjManager, CTerrain * pTerrain, int nRecursionLevel);
  void Render(CObjManager * pObjManager, CTerrain * pTerrain);
  int  Count(int * pCurSpritesCount, int * pCurFreeCount, int * pCurEmitCount);

//  C_PartEmitter * CreateParticleEmitter(ParticleParams Params, float fSpawnPeriod = 1000000, float fLifeTime = 1000000);
  //void DeleteParticleEmitter(C_PartEmitter * pPartEmitter);
	void GetMemoryUsage(ICrySizer* pSizer)const;
	void Reset();
  void OnEntityDeleted(IEntityRender * pEntityRender);
	int GetGlowTexID() const { return m_nGlowTexID; }

	//////////////////////////////////////////////////////////////////////////
	// Particle effects interface.
	//////////////////////////////////////////////////////////////////////////
	IParticleEffect* CreateEffect();
	void RenameEffect( IParticleEffect *pEffect,const char *sNewName );
	void RemoveEffect( IParticleEffect *pEffect );
	IParticleEffect* FindEffect( const char *sEffectName );

	//! Load particle effects from file.
	//! @return true if succesfully loaded.
	bool LoadParticles( CCryFile &file );
	void LoadSharedParticleLibrary( const char *sEffectsFolder,const char *sLibName );
	void LoadParticlesLibs( const char *sEffectsFolder,XmlNodeRef &levelDataRoot );

	//////////////////////////////////////////////////////////////////////////
	// Emitters.
	//////////////////////////////////////////////////////////////////////////
	IParticleEmitter* CreateEmitter();
	void DeleteEmitter( IParticleEmitter *pEmitter );
	void ActivateEmitter( CParticleEmitter *pEmitter );

	//////////////////////////////////////////////////////////////////////////
	// Particles timing.
	//////////////////////////////////////////////////////////////////////////
	float GetParticlesTime() const { return m_currParticlesTime; };
	float GetParticlesFrameTime() const { return m_currParticlesFrameTime; };

	//////////////////////////////////////////////////////////////////////////
	void PlaySound( ISound *pSound );

protected:
	void UpdateEmitters();
	void UpdateSounds();

private:
	//CPartList m_FreeParticles;
	//CParticle             m_arrParts[MAX_PARTICLES_COUNT];
	CSpriteManager *      m_pSpriteMan;
	int                   m_nGlowTexID;
	//  list2<CPartEmitter*>  m_lstpPartEmitters;
	IShader *             m_pPartLightShader;

	//////////////////////////////////////////////////////////////////////////
	//[Timur] Particle Effects.
	//////////////////////////////////////////////////////////////////////////
	//! Array of all registered particle effects.
	std::vector<IParticleEffect_AutoPtr> m_effects;
	//! Map of particle effect case insensetive name to interface pointer.
	typedef std::map<String,IParticleEffect_AutoPtr,stl::less_stricmp<String> > EffectsMap;
	EffectsMap m_effectsMap;

	//////////////////////////////////////////////////////////////////////////
	// Loaded particle libs.
	std::set<String,stl::less_stricmp<String> > m_loadedLibs;

	//////////////////////////////////////////////////////////////////////////
	// Particle effects emitters.
	//////////////////////////////////////////////////////////////////////////
	typedef std::set<_smart_ptr<CParticleEmitter> > ParticleEmitters;
	typedef std::list<_smart_ptr<CParticleEmitter> > ActiveEmitters;
	ParticleEmitters m_allEmitters;
	ActiveEmitters m_activeEmitters;

	//////////////////////////////////////////////////////////////////////////
	// Particle time smoothing.
	//////////////////////////////////////////////////////////////////////////
	float m_currParticlesTime;
	float m_currParticlesFrameTime;
	int m_smoothTimeCount;
	float m_smoothTimes[PARTICLES_SMOOTH_FRAMES];
};

#endif // PART_MANAGER
	