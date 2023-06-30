//////////////////////////////////////////////////////////////////////
//
//	Physical Entity header
//	
//	File: physicalentity.h
//	Description : PhysicalEntity and PhysicalWorld classes declarations
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef physicalworld_h
#define physicalworld_h
#pragma once


const int NSURFACETYPES = 2048;
const int PLACEHOLDER_CHUNK_SZLG2 = 8;
const int PLACEHOLDER_CHUNK_SZ = 1<<PLACEHOLDER_CHUNK_SZLG2;

class CPhysicalPlaceholder;
class CPhysicalEntity;
struct pe_gridthunk;
enum { pef_step_requested = 0x10000000 };

class CPhysicalWorld : public IPhysicalWorld, public IPhysUtils, public CGeomManager {
public:
	CPhysicalWorld(ILog *pLog);
	~CPhysicalWorld();

	virtual void Init();
	virtual void Shutdown(int bDeleteEntities = 1);
	virtual void Release() { delete this; }
	virtual IGeomManager* GetGeomManager() { return this; }
	virtual IPhysUtils* GetPhysUtils() { return this; }

	virtual void SetupEntityGrid(int axisz, vectorf org, int nx,int ny, float stepx,float stepy);
	virtual void SetHeightfieldData(const heightfield *phf);
	virtual int GetHeightfieldData(heightfield *phf);
	virtual int SetSurfaceParameters(int surface_idx, float bounciness,float friction, unsigned int flags=0);
	virtual int GetSurfaceParameters(int surface_idx, float &bounciness,float &friction, unsigned int &flags);
	virtual PhysicsVars *GetPhysVars() { return &m_vars; }

	virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, pe_params* params=0, void *pForeignData=0,int iForeignData=0, int id=-1)
	{ return CreatePhysicalEntity(type,0.0f,params,pForeignData,iForeignData,id); }
	virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, float lifeTime, pe_params* params=0, void *pForeignData=0,int iForeignData=0, 
		int id=-1,IPhysicalEntity *pHostPlaceholder=0);
	virtual IPhysicalEntity *CreatePhysicalPlaceholder(pe_type type, pe_params* params=0, void *pForeignData=0,int iForeignData=0, int id=-1);
	virtual int DestroyPhysicalEntity(IPhysicalEntity *pent, int mode=0);
	virtual int SetPhysicalEntityId(IPhysicalEntity *pent, int id, int bReplace=1);
	virtual int GetPhysicalEntityId(IPhysicalEntity *pent);
	virtual IPhysicalEntity* GetPhysicalEntityById(int id);
	int IsPlaceholder(CPhysicalPlaceholder *pent) {
		if (!pent) return 0;
		int iChunk; for(iChunk=0; iChunk<m_nPlaceholderChunks && (unsigned int)(pent-m_pPlaceholders[iChunk])>=(unsigned int)PLACEHOLDER_CHUNK_SZ; iChunk++);
		return iChunk<m_nPlaceholderChunks ? (iChunk<<PLACEHOLDER_CHUNK_SZLG2 | pent-m_pPlaceholders[iChunk])+1 : 0;
	}
	void SetCurrentEntityHost(CPhysicalPlaceholder *pHost) { m_pCurEntityHost=pHost; }

	virtual void TimeStep(float time_interval, int flags=ent_all|ent_deleted);
	virtual float GetPhysicsTime() { return m_timePhysics; }
	virtual int GetiPhysicsTime() { return m_iTimePhysics; }
	virtual void SetPhysicsTime(float time) { 
		m_timePhysics = time; 
		if (m_vars.timeGranularity>0) 
			m_iTimePhysics = (int)(m_timePhysics/m_vars.timeGranularity+0.5f); 
	}
	virtual void SetiPhysicsTime(int itime) { m_timePhysics = (m_iTimePhysics=itime)*m_vars.timeGranularity; }
	virtual void SetSnapshotTime(float time_snapshot,int iType=0) { 
		m_timeSnapshot[iType] = time_snapshot; 
		if (m_vars.timeGranularity>0) 
			m_iTimeSnapshot[iType] = (int)(time_snapshot/m_vars.timeGranularity+0.5f); 
	}
	virtual void SetiSnapshotTime(int itime_snapshot,int iType=0) { 
		m_iTimeSnapshot[iType] = itime_snapshot; m_timeSnapshot[iType] = itime_snapshot*m_vars.timeGranularity; 
	}

	virtual int RayWorldIntersection(vectorf org,vectorf dir, int objtypes, unsigned int flags, ray_hit *hits,int nmaxhits, 
		IPhysicalEntity *pSkipEnt=0,IPhysicalEntity *pSkipEntAux=0);

	virtual void SimulateExplosion(vectorf epicenter,vectorf epicenterImp, float rmin,float rmax, float r,float impulsive_pressure_at_r, 
		int nOccRes=0,int nGrow=0,float rmin_occ=0.1f, IPhysicalEntity **pSkipEnts=0,int nSkipEnts=0,
		int iTypes=ent_rigid|ent_sleeping_rigid|ent_living|ent_independent);
	virtual float IsAffectedByExplosion(IPhysicalEntity *pent);
	virtual void ResetDynamicEntities();
	virtual void DestroyDynamicEntities();
	virtual void PurgeDeletedEntities();

	virtual void DrawPhysicsHelperInformation(void (*DrawLineFunc)(float*,float*));

	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	virtual int CollideEntityWithBeam(IPhysicalEntity *_pent, vectorf org,vectorf dir,float r, ray_hit *phit);
	virtual int RayTraceEntity(IPhysicalEntity *pient, vectorf origin,vectorf dir, ray_hit *pHit, pe_params_pos *pp=0);

	virtual int GetEntitiesInBox(vectorf ptmin,vectorf ptmax, IPhysicalEntity **&pList, int objtypes) {
		return GetEntitiesAround(ptmin,ptmax, (CPhysicalEntity**&)pList, objtypes);
	}
	int GetEntitiesAround(const vectorf &ptmin,const vectorf &ptmax, CPhysicalEntity **&pList, int objtypes, CPhysicalEntity *pPetitioner=0);
	void RepositionEntity(CPhysicalPlaceholder *pobj, int flags=3);
	void DetachEntityGridThunks(CPhysicalPlaceholder *pobj);
	void ScheduleForStep(CPhysicalEntity *pent);
	CPhysicalEntity *CheckColliderListsIntegrity();

	virtual int BreakPolygon(vector2df *ptSrc,int nPt, int nCellx,int nCelly, int maxPatchTris, vector2df *&ptout,int *&nPtOut, 
													 float jointhresh=0.5f,int seed=-1) 
	{ return ::BreakPolygon(ptSrc,nPt, nCellx,nCelly, maxPatchTris, ptout,nPtOut, jointhresh,seed); };
	virtual int CoverPolygonWithCircles(strided_pointer<vector2df> pt,int npt,bool bConsecutive, const vector2df &center, 
		vector2df *&centers,float *&radii, float minCircleRadius) 
	{ return ::CoverPolygonWithCircles(pt,npt,bConsecutive, center, centers,radii, minCircleRadius); }
	virtual void DeletePointer(void *pdata) { if (pdata) delete[] pdata; }
	virtual void SetPhysicsStreamer(IPhysicsStreamer *pStreamer) { m_pPhysicsStreamer=pStreamer; }
	virtual void SetPhysicsEventClient(IPhysicsEventClient *pEventClient) { m_pEventClient=pEventClient; }
	virtual float GetLastEntityUpdateTime(IPhysicalEntity *pent) { return m_updateTimes[((CPhysicalPlaceholder*)pent)->m_iSimClass & 7]; }

	void AddEntityProfileInfo(CPhysicalEntity *pent,int nTicks);
	virtual int GetEntityProfileInfo(phys_profile_info *&pList);

	virtual int SerializeWorld(const char *fname, int bSave);
	virtual int SerializeGeometries(const char *fname, int bSave);

	PhysicsVars m_vars;
	ILog *m_pLog;
	IPhysicsStreamer *m_pPhysicsStreamer;
	IPhysicsEventClient *m_pEventClient;

	CPhysicalEntity *m_pTypedEnts[8],*m_pTypedEntsPerm[8];
	CPhysicalEntity **m_pTmpEntList,**m_pTmpEntList1;
	float *m_pGroupMass,*m_pMassList;
	int *m_pGroupIds,*m_pGroupNums;
	grid m_entgrid;
	int m_iEntAxisz;
	pe_gridthunk **m_pEntGrid;
	int m_nEnts,m_nEntsAlloc;
	int m_nDynamicEntitiesDeleted;
	CPhysicalPlaceholder **m_pEntsById;
	int m_nIdsAlloc, m_iNextId;
	int m_bGridThunksChanged;
	int m_bUpdateOnlyFlagged;

	int m_nPlaceholders,m_nPlaceholderChunks,m_iLastPlaceholder;
	CPhysicalPlaceholder **m_pPlaceholders;
	int *m_pPlaceholderMap;
	CPhysicalPlaceholder *m_pCurEntityHost;
	CPhysicalEntity *m_pEntBeingDeleted;

	int *m_pGridStat[6],*m_pGridDyn[6];
	int m_nOccRes;
	vectorf m_lastEpicenter;
	float m_lastRmax;
	CPhysicalEntity **m_pExplVictims;
	float *m_pExplVictimsFrac;
	int m_nExplVictims,m_nExplVictimsAlloc;

	CPhysicalEntity *m_pHeightfield;
	matrix3x3f m_HeightfieldBasis;
	vectorf m_HeightfieldOrigin;

	float m_timePhysics,m_timeSurplus,m_timeSnapshot[4];
	int m_iTimePhysics,m_iTimeSnapshot[4];
	float m_updateTimes[8];
	int m_iSubstep,m_bWorldStep,m_iCurGroup;
	CPhysicalEntity *m_pAuxStepEnt;
	phys_profile_info m_pEntProfileData[16];
	int m_nProfiledEnts;

	float m_BouncinessTable[NSURFACETYPES];
	float m_FrictionTable[NSURFACETYPES];
	float m_DynFrictionTable[NSURFACETYPES];
	unsigned int m_SurfaceFlagsTable[NSURFACETYPES];
};

extern int g_nPhysWorlds;
extern CPhysicalWorld *g_pPhysWorlds[];

#ifdef ENTITY_PROFILER_ENABLED
#define PHYS_ENTITY_PROFILER CPhysEntityProfiler ent_profiler(this);
#else
#define PHYS_ENTITY_PROFILER
#endif

struct CPhysEntityProfiler {
	int64 m_iStartTime;
	CPhysicalEntity *m_pEntity;

	CPhysEntityProfiler(CPhysicalEntity *pent) {
		m_pEntity = pent;
		m_iStartTime = GetTicks();
	}
	~CPhysEntityProfiler() {
		if (m_pEntity->m_pWorld->m_vars.bProfileEntities)
			m_pEntity->m_pWorld->AddEntityProfileInfo(m_pEntity,GetTicks()-m_iStartTime);
	}
};

#endif