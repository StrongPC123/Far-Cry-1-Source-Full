//////////////////////////////////////////////////////////////////////
//
//	Physical World
//	
//	File: physicalworld.cpp
//	Description : PhysicalWorld class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "overlapchecks.h"
#include "raybv.h"
#include "raygeom.h"
#include "geoman.h"
#include "singleboxtree.h"
#include "cylindergeom.h"
#include "spheregeom.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "rigidentity.h"
#include "particleentity.h"
#include "livingentity.h"
#include "wheeledvehicleentity.h"
#include "articulatedentity.h"
#include "ropeentity.h"
#include "softentity.h"
#include "physicalworld.h"


CPhysicalWorld *g_pPhysWorlds[64];
int g_nPhysWorlds;


CPhysicalWorld::CPhysicalWorld(ILog *pLog)
{ 
	m_pLog = pLog; 
	Init(); 
	g_pPhysWorlds[g_nPhysWorlds] = this;
	g_nPhysWorlds = min(g_nPhysWorlds+1,sizeof(g_pPhysWorlds)/sizeof(g_pPhysWorlds[0]));
	m_pEntBeingDeleted = 0;
	m_bGridThunksChanged = 0;
	m_bUpdateOnlyFlagged = 0;
}

CPhysicalWorld::~CPhysicalWorld()
{
	Shutdown();
	int i;
	for(i=0; i<g_nPhysWorlds && g_pPhysWorlds[i]!=this; i++);
	if (i<g_nPhysWorlds)
		g_nPhysWorlds--;
	for(; i<g_nPhysWorlds; i++) g_pPhysWorlds[i] = g_pPhysWorlds[i+1];
}


void CPhysicalWorld::Init()
{
	InitGeoman();
	m_pTmpEntList=0; m_pTmpEntList1=0; m_pGroupMass=0; m_pMassList = 0; m_pGroupIds = 0; m_pGroupNums = 0;
	m_nEnts = 0; m_nEntsAlloc = 0;
	m_pEntGrid = 0;
	m_timePhysics = m_timeSurplus = 0;
	m_timeSnapshot[0]=m_timeSnapshot[1]=m_timeSnapshot[2]=m_timeSnapshot[3] = 0;
	m_iTimeSnapshot[0]=m_iTimeSnapshot[1]=m_iTimeSnapshot[2]=m_iTimeSnapshot[3] = 0;
	m_iTimePhysics = 0;
	m_pHeightfield = 0;
	int i; for(i=0;i<8;i++) { m_pTypedEnts[i]=m_pTypedEntsPerm[i]=0; m_updateTimes[i]=0; }
	m_vars.nMaxSubsteps = 10;
	for(i=0;i<NSURFACETYPES;i++) {
		m_BouncinessTable[i] = 0;
		m_FrictionTable[i] = 1.2f;
		m_DynFrictionTable[i] = 1.2f/1.5f;
		m_SurfaceFlagsTable[i] = 0;
	}
	m_vars.nMaxStackSizeMC = 8;
	m_vars.maxMassRatioMC = 50.0f;
	m_vars.nMaxMCiters = 6000;
	m_vars.nMaxMCitersHopeless = 6000;
	m_vars.accuracyMC = 0.005f;
	m_vars.accuracyLCPCG = 0.005f;
	m_vars.nMaxContacts = 150;
	m_vars.nMaxPlaneContacts = 8;
	m_vars.nMaxPlaneContactsDistress = 4;
	m_vars.nMaxLCPCGsubiters = 120;
	m_vars.nMaxLCPCGsubitersFinal = 250;
	m_vars.nMaxLCPCGmicroiters = 12000;
	m_vars.nMaxLCPCGmicroitersFinal = 25000;
	m_vars.nMaxLCPCGiters = 5;
	m_vars.minLCPCGimprovement = 0.05f;
	m_vars.nMaxLCPCGFruitlessIters = 4;
	m_vars.accuracyLCPCGnoimprovement = 0.05f;
	m_vars.minSeparationSpeed = 0.02f;
	m_vars.maxwCG = 500.0f;
	m_vars.maxvCG = 500.0f;
	m_vars.maxvUnproj = 10.0f;
	m_vars.bFlyMode = 0;
	m_vars.iCollisionMode = 0;
	m_vars.bSingleStepMode = 0;
	m_vars.bDoStep = 0;
	m_vars.fixedTimestep = 0;
	m_vars.timeGranularity = 0.0001f;
	m_vars.maxWorldStep = 0.2f;
	m_vars.iDrawHelpers = 0;
	m_vars.nMaxSubsteps = 5;
	m_vars.nMaxSurfaces = NSURFACETYPES;
	m_vars.maxContactGap = 0.01f;
	m_vars.maxContactGapPlayer = 0.01f;
	m_vars.bProhibitUnprojection = 1;//2;
	m_vars.bUseDistanceContacts = 0;
	m_vars.unprojVelScale = 10.0f;
	m_vars.maxUnprojVel = 1.2f;
	m_vars.gravity.Set(0,0,-9.8f);
	m_vars.nGroupDamping = 8;
	m_vars.groupDamping = 0.5f;
	m_vars.bEnforceContacts = 1;//1;
	m_vars.bBreakOnValidation = 0;
	m_vars.bLogActiveObjects = 0;
	m_vars.bMultiplayer = 0;
	m_vars.bProfileEntities = 0;
	m_vars.minBounceSpeed = 6;
	m_vars.nGEBMaxCells = 500;
	m_vars.maxVel = 100.0f;
	m_vars.maxVelPlayers = 30.0f;
	m_vars.bSkipRedundantColldet = 1;
	m_vars.penaltyScale = 0.3f;
	m_vars.maxContactGapSimple = 0.03f;
	m_vars.bLimitSimpleSolverEnergy = 1;
	m_iNextId = 1;
	m_pEntsById = 0;
	m_nIdsAlloc = 0;
	m_nOccRes = 0;
	m_nExplVictims = m_nExplVictimsAlloc = 0;
	m_pPlaceholders = 0; m_pPlaceholderMap = 0;
	m_nPlaceholders = m_nPlaceholderChunks = 0;
	m_pCurEntityHost = 0;
	m_iLastPlaceholder = -1; 
	m_pPhysicsStreamer = 0;
	m_pEventClient = 0;
	m_nProfiledEnts = 0;
	m_iSubstep = 0;
	m_bWorldStep = 0;
	m_nDynamicEntitiesDeleted = 0;
}


void CPhysicalWorld::Shutdown(int bDeleteGeometries)
{
	int i; CPhysicalEntity *pent,*pent_next;
	for(i=0;i<8;i++) {
		for(pent=m_pTypedEnts[i]; pent; pent=pent_next) 
		{ pent_next=pent->m_next; delete pent; }
		m_pTypedEnts[i] = 0; m_pTypedEntsPerm[i] = 0;
	}
	m_nEnts = m_nEntsAlloc = 0;
	for(i=0;i<m_nPlaceholderChunks;i++) if (m_pPlaceholders[i])
		delete[] m_pPlaceholders[i];
	if (m_pPlaceholders) delete[] m_pPlaceholders;
	if (m_pPlaceholderMap) delete[] m_pPlaceholderMap;
	m_nPlaceholderChunks = m_nPlaceholders = 0;
	m_iLastPlaceholder = -1;
	if (m_pEntGrid) delete[] m_pEntGrid;
	m_pEntGrid=0;
	if (m_pTmpEntList) delete[] m_pTmpEntList; m_pTmpEntList = 0;
	if (m_pTmpEntList1) delete[] m_pTmpEntList1; m_pTmpEntList1 = 0;
	if (m_pGroupMass) delete[] m_pGroupMass; m_pGroupMass = 0;
	if (m_pMassList) delete[] m_pMassList; m_pMassList = 0;
	if (m_pGroupIds) delete[] m_pGroupIds; m_pGroupIds = 0;
	if (m_pGroupNums) delete[] m_pGroupNums; m_pGroupNums = 0;
	if (m_pEntsById) delete[] m_pEntsById; m_pEntsById = 0;
	if (m_nOccRes) for(i=0;i<6;i++) {
		delete[] m_pGridStat[i]; delete[] m_pGridDyn[i];
	}
	if (m_nExplVictimsAlloc) {
		delete[] m_pExplVictims; delete[] m_pExplVictimsFrac;
	}

	if (bDeleteGeometries) {
		SetHeightfieldData(0);
		ShutDownGeoman();
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////


void CPhysicalWorld::SetHeightfieldData(const heightfield *phf)
{
	if (!phf) {
		if (m_pHeightfield) {
			m_pHeightfield->m_parts[0].pPhysGeom->pGeom->Release();
			delete m_pHeightfield->m_parts[0].pPhysGeom;
			m_pHeightfield->m_parts[0].pPhysGeom = 0;
			delete m_pHeightfield;
		}
		m_pHeightfield = 0;
		return;
	}
	CGeometry *pGeom = (CGeometry*)CreatePrimitive(heightfield::type, phf);
	if (m_pHeightfield)
		m_pHeightfield->m_parts[0].pPhysGeom->pGeom->Release();
	else {
		m_pHeightfield = new CPhysicalEntity(this);
		m_pHeightfield->m_parts[0].pPhysGeom = m_pHeightfield->m_parts[0].pPhysGeomProxy = new phys_geometry;
		m_pHeightfield->m_parts[0].id = 0;
		m_pHeightfield->m_parts[0].scale = 1.0;
		m_pHeightfield->m_parts[0].mass = 0;
		m_pHeightfield->m_parts[0].flags = geom_collides|geom_has_thin_parts;
		m_pHeightfield->m_parts[0].minContactDist = phf->step.x;
		m_pHeightfield->m_nParts = 1;
		m_pHeightfield->m_id = -1;
	}
	m_HeightfieldBasis = phf->Basis;
	m_HeightfieldOrigin = phf->origin;
	m_pHeightfield->m_parts[0].pPhysGeom->pGeom = pGeom;
	m_pHeightfield->m_parts[0].pos = phf->origin;
	m_pHeightfield->m_parts[0].q = !quaternionf(phf->Basis);
}

int CPhysicalWorld::GetHeightfieldData(heightfield *phf)
{
	if (!m_pHeightfield)
		return 0;
	m_pHeightfield->m_parts[0].pPhysGeom->pGeom->GetPrimitive(0,phf);
	phf->Basis = m_HeightfieldBasis;
	phf->origin = m_HeightfieldOrigin;
	return 1;
}


void CPhysicalWorld::SetupEntityGrid(int axisz, vectorf org, int nx,int ny, float stepx,float stepy)
{
	if (m_pEntGrid) delete[] m_pEntGrid;
	m_iEntAxisz = axisz;
	m_entgrid.size.set(nx,ny);
	m_entgrid.stride.set(1,nx);
	m_entgrid.step.set(stepx,stepy);
	m_entgrid.stepr.set(1.0f/stepx,1.0f/stepy);
	m_entgrid.origin = org;
	m_pEntGrid = new pe_gridthunk*[nx*ny+1];
	for(int i=nx*ny;i>=0;i--) m_pEntGrid[i]=0;
}


int CPhysicalWorld::SetSurfaceParameters(int surface_idx, float bounciness,float friction, unsigned int flags)
{
	if ((unsigned int)surface_idx>=(unsigned int)NSURFACETYPES)
		return 0;
	m_BouncinessTable[surface_idx] = bounciness;
	m_FrictionTable[surface_idx] = friction;
	m_DynFrictionTable[surface_idx] = friction*(1.0/1.5);
	m_SurfaceFlagsTable[surface_idx] = flags;
	return 1;
}
int CPhysicalWorld::GetSurfaceParameters(int surface_idx, float &bounciness,float &friction, unsigned int &flags)
{
	if ((unsigned int)surface_idx>=(unsigned int)NSURFACETYPES)
		return 0;
	bounciness = m_BouncinessTable[surface_idx];
	friction = m_FrictionTable[surface_idx];
	flags = m_SurfaceFlagsTable[surface_idx];
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

template<class dtype> void ReallocateList(dtype *&plist, int szold,int sznew,bool bZero=false)
{
	dtype *newlist = new dtype[sznew];
	if (bZero)
		memset(newlist+szold,0,sizeof(dtype)*max(0,sznew-szold));
	memcpy(newlist,plist,min(szold,sznew)*sizeof(dtype));
	if (plist)
		delete[] plist;
	plist = newlist;
}


IPhysicalEntity* CPhysicalWorld::CreatePhysicalEntity(pe_type type, float lifeTime, pe_params* params, void *pForeignData,int iForeignData, 
																											int id, IPhysicalEntity *pHostPlaceholder)
{
	CPhysicalEntity *res=0;
	CPhysicalPlaceholder *pPrevHost = m_pCurEntityHost;
	if (pHostPlaceholder)
		m_pCurEntityHost = (CPhysicalPlaceholder*)pHostPlaceholder;

	switch (type) {
		case PE_STATIC: res = new CPhysicalEntity(this); break;
		case PE_RIGID : res = new CRigidEntity(this); break;
		case PE_LIVING: res = new CLivingEntity(this); break;
		case PE_WHEELEDVEHICLE: res = new CWheeledVehicleEntity(this); break;
		case PE_PARTICLE: res = new CParticleEntity(this); break;
		case PE_ARTICULATED: res = new CArticulatedEntity(this); break;
		case PE_ROPE: res = new CRopeEntity(this); break;
		case PE_SOFT: res = new CSoftEntity(this); break;
	}

	if (res) {
		if (type!=PE_STATIC)
			m_nDynamicEntitiesDeleted = 0;
		if (m_pCurEntityHost && lifeTime>0) {
			res->m_pForeignData = m_pCurEntityHost->m_pForeignData;
			res->m_iForeignData = m_pCurEntityHost->m_iForeignData;
			res->m_iForeignFlags = m_pCurEntityHost->m_iForeignFlags;
			res->m_id = m_pCurEntityHost->m_id;
			res->m_pEntBuddy = m_pCurEntityHost;
			m_pCurEntityHost->m_pEntBuddy = res;
			res->m_maxTimeIdle = lifeTime;
			res->m_bPermanent = 0;
			res->m_nGridThunks = m_pCurEntityHost->m_nGridThunks;
			res->m_nGridThunks = m_pCurEntityHost->m_nGridThunks;
			res->m_nGridThunksAlloc = m_pCurEntityHost->m_nGridThunksAlloc;
			res->m_pGridThunks = m_pCurEntityHost->m_pGridThunks;
			res->m_ig[0].x=m_pCurEntityHost->m_ig[0].x; res->m_ig[1].x=m_pCurEntityHost->m_ig[1].x;
			res->m_ig[0].y=m_pCurEntityHost->m_ig[0].y; res->m_ig[1].y=m_pCurEntityHost->m_ig[1].y;
		} else {
			res->m_bPermanent = 1;
			res->m_pForeignData = pForeignData;
			res->m_iForeignData = iForeignData;
			SetPhysicalEntityId(res, id>=0 ? id:m_iNextId++);
		}
		if (params)
			res->SetParams(params);
		RepositionEntity(res,2);
		if (++m_nEnts > m_nEntsAlloc-1) {
			m_nEntsAlloc += 256;
			ReallocateList(m_pTmpEntList,m_nEnts-1,m_nEntsAlloc);
			ReallocateList(m_pTmpEntList1,m_nEnts-1,m_nEntsAlloc);
			ReallocateList(m_pGroupMass,m_nEnts-1,m_nEntsAlloc);
			ReallocateList(m_pMassList,m_nEnts-1,m_nEntsAlloc);
			ReallocateList(m_pGroupIds,m_nEnts-1,m_nEntsAlloc);
			ReallocateList(m_pGroupNums,m_nEnts-1,m_nEntsAlloc);
		}
	}

	m_pCurEntityHost = pPrevHost;
	return res;
}


IPhysicalEntity *CPhysicalWorld::CreatePhysicalPlaceholder(pe_type type, pe_params* params, void *pForeignData,int iForeignData, int id)
{
	int i,j,iChunk;
	if (m_nPlaceholders*10<m_iLastPlaceholder*7) {
		for(i=m_iLastPlaceholder>>5; i>=0 && m_pPlaceholderMap[i]==-1; i--);
		if (i>=0) {
			for(j=0;j<32 && m_pPlaceholderMap[i]&1<<j;j++);
			i = i<<5|j;
		}
		i = i-(i>>31) | m_iLastPlaceholder+1&i>>31;
	} else
		i = m_iLastPlaceholder+1;

	iChunk = i>>PLACEHOLDER_CHUNK_SZLG2;
	if (iChunk==m_nPlaceholderChunks) {
		m_nPlaceholderChunks++;
		ReallocateList(m_pPlaceholders, m_nPlaceholderChunks-1,m_nPlaceholderChunks,true);
		ReallocateList(m_pPlaceholderMap, (m_iLastPlaceholder>>5)+1,m_nPlaceholderChunks<<PLACEHOLDER_CHUNK_SZLG2-5,true);
	}
	if (!m_pPlaceholders[iChunk])
		m_pPlaceholders[iChunk] = new CPhysicalPlaceholder[PLACEHOLDER_CHUNK_SZ];
	CPhysicalPlaceholder *res = m_pPlaceholders[iChunk]+(i & PLACEHOLDER_CHUNK_SZ-1);
	
	res->m_pForeignData = pForeignData;
	res->m_iForeignData = iForeignData;
	res->m_iForeignFlags = 0;
	res->m_nGridThunks = res->m_nGridThunksAlloc = 0;
	res->m_pGridThunks = 0;
	res->m_ig[0].x=res->m_ig[0].y=res->m_ig[1].x=res->m_ig[1].y = -2;
	res->m_pEntBuddy = 0;
	res->m_id = -1;
	res->m_bProcessed = 0;
	switch (type) {
		case PE_STATIC: res->m_iSimClass = 0; break;
		case PE_RIGID: case PE_WHEELEDVEHICLE: res->m_iSimClass = 1; break;
		case PE_LIVING: res->m_iSimClass = 3; break;
		case PE_PARTICLE: case PE_ROPE: case PE_ARTICULATED: case PE_SOFT: res->m_iSimClass = 4;
	}
	m_pPlaceholderMap[i>>5] |= 1<<(i&31);

	SetPhysicalEntityId(res, id>=0 ? id:m_iNextId++);
	if (params)
		res->SetParams(params);
	m_nPlaceholders++;
	m_iLastPlaceholder = max(m_iLastPlaceholder,i);

	return res;
}


int CPhysicalWorld::DestroyPhysicalEntity(IPhysicalEntity* _pent,int mode)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	int idx;
	CPhysicalPlaceholder *ppc = (CPhysicalPlaceholder*)_pent;
	if (ppc->m_pEntBuddy && IsPlaceholder(ppc->m_pEntBuddy) && mode!=0 || m_nDynamicEntitiesDeleted && ppc->m_iSimClass>0)
		return 0;

	if (idx=IsPlaceholder(ppc)) {
		if (mode!=0)
			return 0;
		if (ppc->m_pEntBuddy)
			DestroyPhysicalEntity(ppc->m_pEntBuddy,mode);
		SetPhysicalEntityId(ppc,-1);
		DetachEntityGridThunks(ppc);
		if (ppc->m_pGridThunks) delete[] ppc->m_pGridThunks;
		--idx;
		m_pPlaceholderMap[idx>>5] &= ~(1<<(idx&31));
		m_nPlaceholders--;

		int i,j,iChunk = idx>>PLACEHOLDER_CHUNK_SZLG2;
		// if entire iChunk is empty, deallocate it
		for(i=j=0;i<PLACEHOLDER_CHUNK_SZ>>5;i++) j |= m_pPlaceholderMap[(iChunk<<PLACEHOLDER_CHUNK_SZLG2-5)+i];
		if (!j) {
			delete[] m_pPlaceholders[iChunk]; m_pPlaceholders[iChunk] = 0;
		}
		j = m_nPlaceholderChunks;
		// make sure that m_iLastPlaceholder points to the last used placeholder slot
		for(; m_iLastPlaceholder>=0 && !(m_pPlaceholderMap[m_iLastPlaceholder>>5] & 1<<(m_iLastPlaceholder&31)); m_iLastPlaceholder--)
		if ((m_iLastPlaceholder^m_iLastPlaceholder-1)+1>>1 == PLACEHOLDER_CHUNK_SZ) {	
			// if m_iLastPlaceholder points to the 1st chunk element, entire chunk is free and can be deallocated
			iChunk = m_iLastPlaceholder>>PLACEHOLDER_CHUNK_SZLG2;
			if (m_pPlaceholders[iChunk]) {
				delete[] m_pPlaceholders[iChunk]; m_pPlaceholders[iChunk] = 0;
			}
			m_nPlaceholderChunks = iChunk;
		}
		if (m_nPlaceholderChunks<j)
			ReallocateList(m_pPlaceholderMap,j<<PLACEHOLDER_CHUNK_SZLG2-5,m_nPlaceholderChunks<<PLACEHOLDER_CHUNK_SZLG2-5,true);

		return 1;
	}

	CPhysicalEntity *pent = (CPhysicalEntity*)_pent;
	if (pent->m_iSimClass==7)
		return 0;

	if (mode==2) {
		if (pent->m_iSimClass==-1 && pent->m_iPrevSimClass>=0) {
			pent->m_ig[0].x=pent->m_ig[1].x=pent->m_ig[0].y=pent->m_ig[1].y = -2;
			pent->m_iSimClass = pent->m_iPrevSimClass & 0x0F; pent->m_iPrevSimClass=-1;
			RepositionEntity(pent);
		}
		return 1;
	}

	if (m_pEntBeingDeleted==pent)
		return 1;
	m_pEntBeingDeleted = pent;
	if (mode==0 && !pent->m_bPermanent && m_pPhysicsStreamer)
		m_pPhysicsStreamer->DestroyPhysicalEntity(pent);
	m_pEntBeingDeleted = 0;

	pent->AlertNeighbourhoodND();
	if ((unsigned int)pent->m_iPrevSimClass<8u && pent->m_iSimClass>=0) {
		if (pent->m_next) pent->m_next->m_prev = pent->m_prev;
		(pent->m_prev ? pent->m_prev->m_next : m_pTypedEnts[pent->m_iPrevSimClass]) = pent->m_next;
		if (pent==m_pTypedEntsPerm[pent->m_iPrevSimClass])
			m_pTypedEntsPerm[pent->m_iPrevSimClass] = pent->m_next;
	}
	pent->m_next=pent->m_prev = 0;
/*#ifdef _DEBUG
CPhysicalEntity *ptmp = m_pTypedEnts[1];
for(;ptmp && ptmp!=m_pTypedEntsPerm[1]; ptmp=ptmp->m_next);
if (ptmp!=m_pTypedEntsPerm[1])
DEBUG_BREAK;
#endif*/

	if (!pent->m_pEntBuddy) {
		DetachEntityGridThunks(pent);
		if (pent->m_pGridThunks) delete[] pent->m_pGridThunks;
	}
	pent->m_nGridThunks = pent->m_nGridThunksAlloc = 0;
	pent->m_pGridThunks = 0;

	if (mode==0) {
		pent->m_iPrevSimClass = -1; pent->m_iSimClass = 7;
		if (pent->m_next = m_pTypedEnts[7]) 
			pent->m_next->m_prev=pent;
		if (pent->m_pEntBuddy)
			pent->m_pEntBuddy->m_pEntBuddy = 0;
		else
			SetPhysicalEntityId(pent,-1);
		m_pTypedEnts[7] = pent;	
		if (--m_nEnts < m_nEntsAlloc-512) {
			ReallocateList(m_pTmpEntList,m_nEntsAlloc,m_nEntsAlloc-512);
			ReallocateList(m_pTmpEntList1,m_nEntsAlloc,m_nEntsAlloc-512);
			ReallocateList(m_pGroupMass,0,m_nEntsAlloc-512);
			ReallocateList(m_pMassList,0,m_nEntsAlloc-512);
			ReallocateList(m_pGroupIds,0,m_nEntsAlloc-512);
			ReallocateList(m_pGroupNums,0,m_nEntsAlloc-512);
			m_nEntsAlloc -= 512;
		}
	} else if (pent->m_iSimClass>=0) {
		pe_action_reset reset;
		pent->Action(&reset);
		pent->m_iPrevSimClass = pent->m_iSimClass | 0x100;
		pent->m_iSimClass = -1;
	}
	
	return 1;
}


int CPhysicalWorld::SetPhysicalEntityId(IPhysicalEntity *_pent, int id, int bReplace)
{
	CPhysicalPlaceholder *pent = (CPhysicalPlaceholder*)_pent;
	unsigned int previd = (unsigned int)pent->m_id;
	if (previd<(unsigned int)m_nIdsAlloc) {
		m_pEntsById[previd] = 0;
		if (previd==m_iNextId-1)
			for(;m_iNextId>0 && m_pEntsById[m_iNextId-1]==0;m_iNextId--);
	}
	m_iNextId = max(m_iNextId,id+1);

	if (id>=0) { 
		if (id>=m_nIdsAlloc) {
			int nAllocPrev = m_nIdsAlloc;
			ReallocateList(m_pEntsById, nAllocPrev,m_nIdsAlloc=(id&~255)+256, true);
		}
		if (m_pEntsById[id]) {
			if (bReplace)
				SetPhysicalEntityId(m_pEntsById[id],m_iNextId++);
			else 
				return 0;
		}
		if (IsPlaceholder(pent->m_pEntBuddy))
			pent = pent->m_pEntBuddy;
		(m_pEntsById[id] = pent)->m_id = id;
		if (pent->m_pEntBuddy)
			pent->m_pEntBuddy->m_id = id;
		return 1;
	}
	return 0;
}

int CPhysicalWorld::GetPhysicalEntityId(IPhysicalEntity *pent)
{
	return pent ? ((CPhysicalEntity*)pent)->m_id : -1;
}

IPhysicalEntity* CPhysicalWorld::GetPhysicalEntityById(int id)
{
	if ((unsigned int)id<(unsigned int)m_nIdsAlloc)
		return m_pEntsById[id] ? m_pEntsById[id]->GetEntity() : 0;
	else
		return id==-1 ? m_pHeightfield : 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////


static inline void swap(CPhysicalEntity **pentlist,float *pmass,int *pids, int i1,int i2) {	
	CPhysicalEntity *pent = pentlist[i1]; pentlist[i1] = pentlist[i2]; pentlist[i2] = pent;
	float m = pmass[i1]; pmass[i1] = pmass[i2]; pmass[i2] = m;
	if (pids) {
		int id = pids[i1]; pids[i1] = pids[i2]; pids[i2] = id;
	}
}
static void qsort(CPhysicalEntity **pentlist,float *pmass,int *pids, int ileft,int iright)
{
	if (ileft>=iright) return;
	int i,ilast; 
	swap(pentlist,pmass,pids, ileft,ileft+iright>>1);
	for(ilast=ileft,i=ileft+1; i<=iright; i++)
	if (pmass[i] > pmass[ileft])
		swap(pentlist,pmass,pids, ++ilast,i);
	swap(pentlist,pmass,pids, ileft,ilast);

	qsort(pentlist,pmass,pids, ileft,ilast-1);
	qsort(pentlist,pmass,pids, ilast+1,iright);
}


int CPhysicalWorld::GetEntitiesAround(const vectorf &ptmin,const vectorf &ptmax, CPhysicalEntity **&pList, int objtypes, 
																			CPhysicalEntity *pPetitioner)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	if (!m_pEntGrid) return 0;
	int i,j,igx[2],igy[2],ix,iy,nout=0,itype,bSortRequired=0,bContact,nGridEnts=0,nEntsChecked=0;
	vectorf bbox[2],bbox1[2];
	pe_gridthunk *thunk,*thunk_next; 
	itype = pPetitioner ? 1<<pPetitioner->m_iSimClass & -iszero((int)pPetitioner->m_flags&pef_never_affect_triggers): 0;

	bbox[0]=ptmin; bbox[1]=ptmax;
	for(i=0;i<2;i++) {
		igx[i] = max(-1,min(m_entgrid.size.x,float2int((bbox[i][inc_mod3[m_iEntAxisz]]-m_entgrid.origin[inc_mod3[m_iEntAxisz]])*m_entgrid.stepr.x-0.5f)));
		igy[i] = max(-1,min(m_entgrid.size.y,float2int((bbox[i][dec_mod3[m_iEntAxisz]]-m_entgrid.origin[dec_mod3[m_iEntAxisz]])*m_entgrid.stepr.y-0.5f)));
	}

	if ((igx[1]-igx[0]+1)*(igy[1]-igy[0]+1)>m_vars.nGEBMaxCells) {
		m_pLog->Log("\003GetEntitiesInBox: too many cells requested by %s (%d, (%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f))",
			pPetitioner ? m_pPhysicsStreamer->GetForeignName(pPetitioner->m_pForeignData,pPetitioner->m_iForeignData,pPetitioner->m_iForeignFlags):"Game",
			(igx[1]-igx[0]+1)*(igy[1]-igy[0]+1), bbox[0].x,bbox[0].y,bbox[0].z, bbox[1].x,bbox[1].y,bbox[1].z);
		if (m_vars.bBreakOnValidation) DoBreak
	}
	
	for(ix=igx[0];ix<=igx[1];ix++) for(iy=igy[0];iy<=igy[1];iy++) {
		for(thunk=m_pEntGrid[m_entgrid.getcell_safe(ix,iy)]; thunk; thunk=thunk_next,nGridEnts++) {
			thunk_next = thunk->next;
			if ((thunk->pent->m_bProcessed^1) & (objtypes>>thunk->pent->m_iSimClass)&1) {
				CPhysicalPlaceholder *pGridEnt = thunk->pent;
				bbox1[0] = pGridEnt->m_BBox[0]; bbox1[1] = pGridEnt->m_BBox[1];
				bContact = isneg(fabsf(bbox[0].x+bbox[1].x-bbox1[0].x-bbox1[1].x) - (bbox[1].x-bbox[0].x)-(bbox1[1].x-bbox1[0].x)) & 
									 isneg(fabsf(bbox[0].y+bbox[1].y-bbox1[0].y-bbox1[1].y) - (bbox[1].y-bbox[0].y)-(bbox1[1].y-bbox1[0].y)) &
									 isneg(fabsf(bbox[0].z+bbox[1].z-bbox1[0].z-bbox1[1].z) - (bbox[1].z-bbox[0].z)-(bbox1[1].z-bbox1[0].z));

				if (bContact) {
					if (pGridEnt->m_iSimClass!=6) {
						m_bGridThunksChanged = 0;
						CPhysicalEntity *pent = thunk->pent->GetEntity();
						if (m_bGridThunksChanged)
							thunk_next = m_pEntGrid[m_entgrid.getcell_safe(ix,iy)];
						m_bGridThunksChanged = 0;
						if (objtypes & ent_ignore_noncolliding) {
							for(i=0;i<pent->m_nParts && !(pent->m_parts[i].flags & (geom_collides&~geom_colltype_ray));i++);
							if (i==pent->m_nParts) continue;
						}
						m_pTmpEntList[nout] = pent;
						nout += (pGridEnt->m_bProcessed = iszero(m_bUpdateOnlyFlagged & ((int)pent->m_flags^pef_update)) | iszero(pent->m_iSimClass));
						bSortRequired += pent->m_pOuterEntity!=0;
					} else if (pGridEnt->m_iForeignFlags & itype && m_pEventClient)	{
						/*bContact = 
							isneg(fabsf(pPetitioner->m_BBox[0].x+pPetitioner->m_BBox[1].x-bbox1[0].x-bbox1[1].x) - 
										(pPetitioner->m_BBox[1].x-pPetitioner->m_BBox[0].x)-(bbox1[1].x-bbox1[0].x)) & 
							isneg(fabsf(pPetitioner->m_BBox[0].y+pPetitioner->m_BBox[1].y-bbox1[0].y-bbox1[1].y) - 
										(pPetitioner->m_BBox[1].y-pPetitioner->m_BBox[0].y)-(bbox1[1].y-bbox1[0].y)) &
							isneg(fabsf(pPetitioner->m_BBox[0].z+pPetitioner->m_BBox[1].z-bbox1[0].z-bbox1[1].z) - 
										(pPetitioner->m_BBox[1].z-pPetitioner->m_BBox[0].z)-(bbox1[1].z-bbox1[0].z));
						if (bContact)*/ 
						m_pEventClient->OnBBoxOverlap(pGridEnt,pGridEnt->m_pForeignData,pGridEnt->m_iForeignData,
							pPetitioner,pPetitioner->m_pForeignData,pPetitioner->m_iForeignData);
					}
				}
				nEntsChecked++;
			}
		}
	}
	for(i=0;i<nout;i++)	{
		m_pTmpEntList[i]->m_bProcessed = 0;
		if (m_pTmpEntList[i]->m_pEntBuddy)
			m_pTmpEntList[i]->m_pEntBuddy->m_bProcessed = 0;
	}

	if (bSortRequired) {
		CPhysicalEntity *pent,*pents,*pstart;
		for(i=0;i<nout;i++) m_pTmpEntList[i]->m_bProcessed_aux = 1;
		for(i=0,pent=0;i<nout-1;i++) {
			m_pTmpEntList[i]->m_prev_aux = pent;
			m_pTmpEntList[i]->m_next_aux = m_pTmpEntList[i+1];
			pent = m_pTmpEntList[i];
		}
		pstart = m_pTmpEntList[0];
		m_pTmpEntList[nout-1]->m_prev_aux = pent;
		m_pTmpEntList[nout-1]->m_next_aux = 0;
		for(i=0;i<nout;i++) {
			if ((pent=m_pTmpEntList[i])->m_pOuterEntity && pent->m_pOuterEntity->m_bProcessed_aux>0) {
				// if entity has an outer entity, move it together with its children right before this outer entity
				for(pents=pent,j=pent->m_bProcessed_aux-1; j>0; pents=pents->m_prev_aux);	// count back the number of pent children
				(pents->m_prev_aux ? pent->m_prev_aux->m_next_aux : pstart) = pent->m_next_aux;	// cut pents-pent stripe from list ...
				if (pent->m_next_aux) pent->m_next_aux->m_prev_aux = pents->m_prev_aux;
				pent->m_next_aux = pent->m_pOuterEntity; // ... and insert if before pent
				pents->m_prev_aux = pent->m_pOuterEntity->m_prev_aux;
				(pent->m_pOuterEntity->m_prev_aux ? pent->m_pOuterEntity->m_prev_aux->m_next_aux : pstart) = pents;
				pent->m_pOuterEntity->m_prev_aux = pent;
				pent->m_pOuterEntity->m_bProcessed_aux += pent->m_bProcessed_aux;
			}
		}
		vectorf ptc = (ptmin+ptmax)*0.5f;
		for(i=0;i<nout;i++) m_pTmpEntList[i]->m_bProcessed_aux = 0;
		for(pent=pstart,nout=0; pent; pent=pent->m_next_aux) if (!pent->m_bProcessed_aux) {
			m_pTmpEntList[nout] = pent;
			if (pent->m_pOuterEntity && pent->IsPointInside(ptc))
				for(pent=pent->m_pOuterEntity; pent; pent=pent->m_pOuterEntity) pent->m_bProcessed_aux=-1;
			pent = m_pTmpEntList[nout++];
		}
	}

	if (m_pHeightfield && objtypes & ent_terrain)
		m_pTmpEntList[nout++] = m_pHeightfield;

	pList = m_pTmpEntList;
	if (objtypes & ent_sort_by_mass) {
		for(i=0;i<nout;i++) m_pMassList[i] = m_pTmpEntList[i]->GetMassInv();
		// manually put all static (0-massinv) object to the end of the list, since qsort doesn't
		// perform very well on lists of same numbers
		int ilast;
		for(i=ilast=nout-1; i>0; i--) if (m_pMassList[i]==0) {
			if (i!=ilast) 
				swap(m_pTmpEntList,m_pMassList,0, i,ilast);
			--ilast;
		}
		qsort(m_pTmpEntList,m_pMassList,0, 0,ilast);
	}

	if (objtypes&ent_allocate_list) {
		if (nout>0) {
			pList = new CPhysicalEntity*[nout];	
			for(i=0;i<nout;i++) pList[i] = m_pTmpEntList[i];
		}	else
			pList = 0; //  don't allocate 0-elements arrays
	}

	return nout;
}


void CPhysicalWorld::ScheduleForStep(CPhysicalEntity *pent)
{
	if (!(pent->m_flags & pef_step_requested)) {
		pent->m_flags |= pef_step_requested;
		pent->m_next_coll = m_pAuxStepEnt;
		m_pAuxStepEnt = pent;
	}
}

int __curstep = 0; // debug

void CPhysicalWorld::TimeStep(float time_interval, int flags)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	float m,max_time_step,time_interval_org = time_interval, Ebefore,Eafter,damping;
	CPhysicalEntity *pent,*phead,*ptail,**pentlist,*pent_next,*pent1,*pentmax;
	int i,j,n,iter,ipass,nGroups,bHeadAdded,bGroupFinished,bAllGroupsFinished,bStepValid,nAnimatedObjects,nEnts,bSkipFlagged;

	if (time_interval<0)
		return;
	if (time_interval > m_vars.maxWorldStep)
		time_interval = time_interval_org = m_vars.maxWorldStep;
	
	if (m_vars.timeGranularity>0) {
		i = float2int(time_interval_org/m_vars.timeGranularity);
		time_interval_org = time_interval = i*m_vars.timeGranularity;
		m_iTimePhysics += i;
		m_timePhysics = m_iTimePhysics*m_vars.timeGranularity;
	}	else
		m_timePhysics += time_interval;
	if (m_vars.fixedTimestep>0)
		time_interval = m_vars.fixedTimestep;
	m_bUpdateOnlyFlagged = flags & ent_flagged_only;
	bSkipFlagged = flags>>1 & pef_update;
	m_bWorldStep = 1;
	m_vars.bUseDistanceContacts &= m_vars.bMultiplayer^1;

	if (flags & ent_living) {
		for(pent=m_pTypedEnts[3]; pent; pent=pent->m_next) if (!(m_bUpdateOnlyFlagged&(pent->m_flags^pef_update) | bSkipFlagged&pent->m_flags))
			pent->StartStep(time_interval_org);	// prepare to advance living entities
	}

	if (!m_vars.bSingleStepMode || m_vars.bDoStep) {
		m_nProfiledEnts = 0;
		iter = 0;	__curstep++;
		if (flags & ent_independent) {
			for(pent=m_pTypedEnts[4]; pent; pent=pent->m_next) if (!(m_bUpdateOnlyFlagged&(pent->m_flags^pef_update) | bSkipFlagged&pent->m_flags))
				pent->StartStep(time_interval);
		}

		if (flags & ent_rigid) {
			if (m_pTypedEnts[2]) do { // make as many substeps as required
				bAllGroupsFinished = 1;
				m_pGroupNums[m_nEntsAlloc-1] = -1; // special group for rigid bodies w/ infinite mass
				m_iSubstep++;

				for(ipass=0; ipass<2; ipass++) {
					// build lists of intercolliding groups of entities
					for(pent=m_pTypedEnts[2],nGroups=0; pent; pent=pent->m_next) 
					if (!(pent->m_bMoved | m_bUpdateOnlyFlagged&(pent->m_flags^pef_update) | bSkipFlagged&pent->m_flags)) {
						if (pent->GetMassInv()<=0) { 
							if ((iter|ipass)==0) { // just make isolated step for rigids with infinite mass
								pent->StartStep(time_interval); 
								pent->m_iGroup = m_nEntsAlloc-1;
							}
							if (ipass==0)	{
								pent->Step(pent->GetMaxTimeStep(time_interval));
								bAllGroupsFinished &= pent->Update(time_interval,1);
							}
						} else {
							pent->m_iGroup = nGroups; pent->m_bMoved = 1;	m_pGroupIds[nGroups] = 0;
							m_pGroupMass[nGroups] = 1.0f/pent->GetMassInv();
							if ((iter | ipass)==0) pent->StartStep(time_interval);
							pent->m_next_coll1 = pent->m_next_coll = 0;
							m_pTmpEntList1[nGroups] = 0;
							// initially m_pTmpEntList1 points to group entities that collide with statics (sorted by mass) - linked via m_next_coll
							// m_next_coll1 maintains a queue of current intercolliding objects

							for(phead=ptail=pentmax=pent; phead; phead=phead->m_next_coll1) {
								for(i=bHeadAdded=0,n=phead->GetColliders(pentlist); i<n; i++) if (pentlist[i]->GetMassInv()<=0) {
									if (!bHeadAdded) {
										for(pent1=m_pTmpEntList1[nGroups]; pent1 && pent1->m_next_coll && pent1->m_next_coll->GetMassInv()<=phead->GetMassInv(); 
												pent1=pent1->m_next_coll);
										if (!pent1 || pent1->GetMassInv()>phead->GetMassInv()) {
											phead->m_next_coll = pent1; m_pTmpEntList1[nGroups] = phead;
										} else {
											phead->m_next_coll = pent1->m_next_coll; pent1->m_next_coll = phead;
										}
										bHeadAdded = 1;
									}
									m_pGroupIds[nGroups] = 1; // tells that group has static entities
								} else if (!(pentlist[i]->m_bMoved | m_bUpdateOnlyFlagged & (pentlist[i]->m_flags^pef_update))) {
									pentlist[i]->m_flags &= ~bSkipFlagged;
									ptail->m_next_coll1 = pentlist[i]; ptail = pentlist[i]; ptail->m_next_coll1 = 0;
									ptail->m_next_coll = 0;
									ptail->m_iGroup = nGroups; ptail->m_bMoved = 1;
									if ((iter | ipass)==0) ptail->StartStep(time_interval);
									m_pGroupMass[nGroups] += 1.0f/(m=ptail->GetMassInv());
									if (pentmax->GetMassInv()>m)
										pentmax = ptail;
								}
							}
							if (!m_pTmpEntList1[nGroups])
								m_pTmpEntList1[nGroups] = pentmax;
							nGroups++;
						}
					}

					// add maximum group mass to all groups that contain static entities
					for(i=1,m=m_pGroupMass[0]; i<nGroups; i++) m = max(m,m_pGroupMass[i]);
					for(m*=1.01f,i=0; i<nGroups; i++) m_pGroupMass[i] += m*m_pGroupIds[i];
					for(i=0;i<nGroups;i++) m_pGroupIds[i] = i;

					// sort groups by decsending group mass
					qsort(m_pTmpEntList1,m_pGroupMass,m_pGroupIds, 0,nGroups-1);
					for(i=0;i<nGroups;i++) m_pGroupNums[m_pGroupIds[i]] = i;

					for(i=0;i<nGroups;i++) {
						m_iCurGroup = m_pGroupIds[i];
						max_time_step = time_interval; nAnimatedObjects = 0;
						for(ptail=m_pTmpEntList1[i]; ptail->m_next_coll; ptail=ptail->m_next_coll) ptail->m_bMoved = 0;
						ptail->m_bMoved = 0;
						for(phead=m_pTmpEntList1[i]; phead; phead=phead->m_next_coll)
							for(j=0,n=phead->GetColliders(pentlist); j<n; j++) if (pentlist[j]->GetMassInv()>0) {
								if (!(pentlist[j]->m_bMoved^1 | m_bUpdateOnlyFlagged & (pentlist[j]->m_flags^pef_update))) {
									ptail->m_next_coll = pentlist[j]; ptail = pentlist[j]; ptail->m_next_coll = 0; ptail->m_bMoved = 0;
								} 
							} else if (pentlist[j]->m_iSimClass>1) {
								max_time_step = min(max_time_step,pentlist[j]->GetLastTimeStep(time_interval));
								nAnimatedObjects++;
							}
						for(pent=m_pTmpEntList1[i]; pent; pent=pent->m_next_coll)
							max_time_step = min(max_time_step, pent->GetMaxTimeStep(time_interval));

						if (ipass==0) {
							m_pAuxStepEnt = 0;
							for(pent=m_pTmpEntList1[i],bStepValid=1; pent; pent=pent->m_next_coll) {
								bStepValid &= pent->Step(max_time_step); pent->m_bMoved = 1;
							}
							if (!bStepValid) {
								for(pent=m_pTmpEntList1[i]; pent; pent=pent->m_next_coll)
									pent->StepBack(max_time_step);
								for(pent=m_pAuxStepEnt; pent; pent=pent->m_next_coll) 
									pent->m_flags &= ~pef_step_requested;
							} else {
								m_bWorldStep = 2;
								for(pent=m_pAuxStepEnt; pent; pent=pent_next) {
									pent_next = pent->m_next_coll;
									pent->m_flags &= ~pef_step_requested;
									pent->Step(pent->GetMaxTimeStep(max_time_step));
								}
								m_bWorldStep = 1;
							}
							for(pent=m_pTmpEntList1[i]; pent; pent=pent->m_next_coll) pent->m_bMoved = 0;
						} else {
							InitContactSolver(max_time_step);
							Ebefore = Eafter = 0.0f; 

							if (m_vars.nMaxPlaneContactsDistress!=m_vars.nMaxPlaneContacts) {
								for(pent=m_pTmpEntList1[i],j=nEnts=0; pent; pent=pent->m_next_coll,nEnts++)	{
									j += pent->GetContactCount(m_vars.nMaxPlaneContacts);
									Ebefore += pent->CalcEnergy(max_time_step);
								}
								n = j>m_vars.nMaxContacts ? m_vars.nMaxPlaneContactsDistress : m_vars.nMaxPlaneContacts;
								for(pent=m_pTmpEntList1[i]; pent; pent=pent->m_next_coll)
									pent->RegisterContacts(max_time_step,n);
							} else for(pent=m_pTmpEntList1[i],nEnts=0; pent; pent=pent->m_next_coll,nEnts++) {
								pent->RegisterContacts(max_time_step, m_vars.nMaxPlaneContacts);
								Ebefore += pent->CalcEnergy(max_time_step);
							}

							Ebefore = max(m_pGroupMass[i]*sqr(0.005f),Ebefore);
							
							InvokeContactSolver(max_time_step, &m_vars);

							//if (nAnimatedObjects==0) 
							damping = 1.0f-max_time_step*m_vars.groupDamping*isneg(m_vars.nGroupDamping-1-max(nEnts,g_nBodies));
							for(pent=m_pTmpEntList1[i],bGroupFinished=1; pent; pent=pent->m_next_coll) {
								Eafter += pent->CalcEnergy(0);	
								if (!(pent->m_flags & pef_fixed_damping))
									damping = min(damping,pent->GetDamping(max_time_step));
								else {
									damping = pent->GetDamping(max_time_step);
									break;
								}
							}
							Ebefore *= isneg(-nAnimatedObjects)+1; // increase energy growth limit if we have animated bodies involved
							if (Eafter>Ebefore*(1.0f+0.1f*isneg(g_nBodies-15)))
								damping = min(damping, sqrt_tpl(Ebefore/Eafter));
							for(pent=m_pTmpEntList1[i],bGroupFinished=1; pent; pent=pent->m_next_coll)
								bGroupFinished &= pent->Update(max_time_step, damping);
							for(pent=m_pTmpEntList1[i]; pent; pent=pent->m_next_coll)
								pent->m_bMoved = bGroupFinished;
							bAllGroupsFinished &= bGroupFinished;
						}
					}
				}
			} while (!bAllGroupsFinished && ++iter<m_vars.nMaxSubsteps);

			for(pent=m_pTypedEnts[1]; pent; pent=pent->m_next) {
				pent->m_bMoved=0; pent->m_iGroup=-1;
			}
			for(pent=m_pTypedEnts[2]; pent; pent=pent->m_next) pent->m_bMoved=0;
			m_updateTimes[1] = m_updateTimes[2] = m_timePhysics;
		}
	}
	m_iSubstep++;

	if (flags & ent_living) {
		//for(pent=m_pTypedEnts[3]; pent; pent=pent->m_next) if (!(m_bUpdateOnlyFlagged & (pent->m_flags^pef_update)))
		//	pent->StartStep(time_interval_org);	// prepare to advance living entities
		for(pent=m_pTypedEnts[3]; pent; pent=pent_next) {
			pent_next = pent->m_next;
			if (!(m_bUpdateOnlyFlagged&(pent->m_flags^pef_update) | bSkipFlagged&pent->m_flags))
				pent->Step(pent->GetMaxTimeStep(time_interval_org)); // advance living entities
		}
		m_updateTimes[3] = m_timePhysics;
	}

	if (!m_vars.bSingleStepMode || m_vars.bDoStep) {
		if (flags & ent_independent) {
			for(pent=m_pTypedEnts[4]; pent; pent=pent->m_next) if (!(m_bUpdateOnlyFlagged&(pent->m_flags^pef_update) | bSkipFlagged&pent->m_flags))
				for(/*pent->StartStep(time_interval),*/iter=0; !pent->Step(pent->GetMaxTimeStep(time_interval)) && ++iter<m_vars.nMaxSubsteps; );
			m_updateTimes[4] = m_timePhysics;
		}
	}

	if (flags & ent_deleted) {
		if (!m_vars.bSingleStepMode || m_vars.bDoStep) {
			for(pent=m_pTypedEnts[7]; pent; pent=pent_next) { // purge deletion requests
				pent_next = pent->m_next; delete pent;
			}
			m_pTypedEnts[7] = 0;
		}

		// flush static and sleeping physical objects that have timeouted
		for(i=0;i<2;i++) for(pent=m_pTypedEnts[i]; pent!=m_pTypedEntsPerm[i]; pent=pent_next) {
			pent_next = pent->m_next;
			if (pent->m_nRefCount==0 && (pent->m_timeIdle+=time_interval_org)>pent->m_maxTimeIdle)
				DestroyPhysicalEntity(pent);
		}

		for(pent=m_pTypedEnts[2]; pent!=m_pTypedEntsPerm[2]; pent=pent->m_next)
			pent->m_timeIdle = 0;	// reset idle count for active physical entities 

		for(pent=m_pTypedEnts[4]; pent!=m_pTypedEntsPerm[4]; pent=pent_next) {
			pent_next = pent->m_next;
			if (pent->IsAwake())
				pent->m_timeIdle = 0;	// reset idle count for active detached entities 
			else if (pent->m_nRefCount==0 && (pent->m_timeIdle+=time_interval_org)>pent->m_maxTimeIdle)
				DestroyPhysicalEntity(pent);
		}

		m_updateTimes[7] = m_timePhysics;
		m_vars.bDoStep = 0;
	}
	m_bUpdateOnlyFlagged = 0;
	m_bWorldStep = 0;
}


void CPhysicalWorld::DetachEntityGridThunks(CPhysicalPlaceholder *pobj)
{
	for(int i=0;i<pobj->m_nGridThunks;i++) {
		if (pobj->m_pGridThunks[i].next) pobj->m_pGridThunks[i].next->prev = pobj->m_pGridThunks[i].prev;
		if (pobj->m_pGridThunks[i].prev) pobj->m_pGridThunks[i].prev->next = pobj->m_pGridThunks[i].next;
		pobj->m_pGridThunks[i].next = pobj->m_pGridThunks[i].prev = 0;
		/*else for(ix=pobj->m_ig[0].x;ix<=pobj->m_ig[1].x;ix++) for(iy=pobj->m_ig[0].y;iy<=pobj->m_ig[1].y;iy++) {
			j = m_entgrid.getcell_safe(ix,iy);
			if (m_pEntGrid[j]==pobj->m_pGridThunks+i)
				m_pEntGrid[j] = pobj->m_pGridThunks[i].next;
		}*/
	}
	pobj->m_nGridThunks = 0;
}


void CPhysicalWorld::RepositionEntity(CPhysicalPlaceholder *pobj, int flags)
{
	int i,j,igx[2],igy[2],n,ix,iy;
	if ((unsigned int)pobj->m_iSimClass>=7u) return; // entity is frozen

	if (flags&1 && m_pEntGrid) {
		for(i=0;i<2;i++) {
			igx[i] = max(-1,min(m_entgrid.size.x,
				float2int((pobj->m_BBox[i][inc_mod3[m_iEntAxisz]] - m_entgrid.origin[inc_mod3[m_iEntAxisz]])*m_entgrid.stepr.x-0.5f)));
			igy[i] = max(-1,min(m_entgrid.size.y,
				float2int((pobj->m_BBox[i][dec_mod3[m_iEntAxisz]] - m_entgrid.origin[dec_mod3[m_iEntAxisz]])*m_entgrid.stepr.y-0.5f)));
		}
		if ((igx[0]-pobj->m_ig[0].x | igy[0]-pobj->m_ig[0].y | igx[1]-pobj->m_ig[1].x | igy[1]-pobj->m_ig[1].y) &&
				pobj->m_ig[0].x!=-3) // if m_igx[0] is -3, the entity should not be registered in grid at all
		{
			m_bGridThunksChanged = 1;
			DetachEntityGridThunks(pobj);
			CPhysicalPlaceholder *pcurobj = pobj;
			if (IsPlaceholder(pobj->m_pEntBuddy))
				pcurobj = pobj->m_pEntBuddy;
			n = (igx[1]-igx[0]+1)*(igy[1]-igy[0]+1);
			if (n<=0 || n>1024) {
				vectorf pos = (pcurobj->m_BBox[0]+pcurobj->m_BBox[1])*0.5f;
				char buf[256]; sprintf(buf,"\002Error: %s @ %.1f,%.1f,%.1f is too large or invalid",
					m_pPhysicsStreamer->GetForeignName(pcurobj->m_pForeignData,pcurobj->m_iForeignData,pcurobj->m_iForeignFlags), pos.x,pos.y,pos.z);
				VALIDATOR_LOG(m_pLog,buf);
				if (m_vars.bBreakOnValidation) DoBreak
				pobj->m_ig[0].x=pobj->m_ig[1].x=pobj->m_ig[0].y=pobj->m_ig[1].y = -2;
				goto skiprepos;
			}
			if (n > pcurobj->m_nGridThunksAlloc) {
				// check the heap for integrity
				//assert (IsHeapValid());
				if (pcurobj->m_pGridThunks) delete[] pcurobj->m_pGridThunks;
				pcurobj->m_pGridThunks = new pe_gridthunk[pcurobj->m_nGridThunksAlloc=n];
				for(i=0;i<n;i++) pcurobj->m_pGridThunks[i].pent = pcurobj;
			}
			for(ix=igx[0],i=0;ix<=igx[1];ix++) for(iy=igy[0];iy<=igy[1];iy++,i++) {
				j = m_entgrid.getcell_safe(ix,iy);
				pcurobj->m_pGridThunks[i].next = m_pEntGrid[j];
				pcurobj->m_pGridThunks[i].prev = (pe_gridthunk*)&m_pEntGrid[j];
				//pcurobj->m_pGridThunks[i].prev = 0;
				if (m_pEntGrid[j]) m_pEntGrid[j]->prev = pcurobj->m_pGridThunks+i;
				m_pEntGrid[j] = pcurobj->m_pGridThunks+i;
			}
			pcurobj->m_nGridThunks = i;
			pcurobj->m_ig[0].x=igx[0]; pcurobj->m_ig[1].x=igx[1]; 
			pcurobj->m_ig[0].y=igy[0]; pcurobj->m_ig[1].y=igy[1];
			if (pcurobj->m_pEntBuddy) {
				pcurobj->m_pEntBuddy->m_nGridThunks = pcurobj->m_nGridThunks;
				pcurobj->m_pEntBuddy->m_nGridThunksAlloc = pcurobj->m_nGridThunksAlloc;
				pcurobj->m_pEntBuddy->m_pGridThunks = pcurobj->m_pGridThunks;
				pcurobj->m_pEntBuddy->m_ig[0].x=igx[0]; pcurobj->m_pEntBuddy->m_ig[1].x=igx[1];
				pcurobj->m_pEntBuddy->m_ig[0].y=igy[0]; pcurobj->m_pEntBuddy->m_ig[1].y=igy[1];
			}
			skiprepos:;
		}
	}

	if (flags&2) {
		CPhysicalEntity *pent = (CPhysicalEntity*)pobj;
		if (pent->m_iPrevSimClass!=pent->m_iSimClass) {
			if ((unsigned int)pent->m_iPrevSimClass<8u) {
				if (pent->m_next) pent->m_next->m_prev = pent->m_prev;
				(pent->m_prev ? pent->m_prev->m_next : m_pTypedEnts[pent->m_iPrevSimClass]) = pent->m_next;
				if (pent==m_pTypedEntsPerm[pent->m_iPrevSimClass])
					m_pTypedEntsPerm[pent->m_iPrevSimClass] = pent->m_next;
			}

			if (!pent->m_bPermanent) {
				pent->m_next = m_pTypedEnts[pent->m_iSimClass]; 
				pent->m_prev = 0;
				if (pent->m_next) pent->m_next->m_prev = pent;
				m_pTypedEnts[pent->m_iSimClass] = pent;
			} else {
				pent->m_next = m_pTypedEntsPerm[pent->m_iSimClass];
				if (m_pTypedEntsPerm[pent->m_iSimClass]) {
					if (pent->m_prev = m_pTypedEntsPerm[pent->m_iSimClass]->m_prev)
						pent->m_prev->m_next = pent;
					pent->m_next->m_prev = pent;
				} else if (m_pTypedEnts[pent->m_iSimClass]) {
					for(pent->m_prev=m_pTypedEnts[pent->m_iSimClass]; pent->m_prev && pent->m_prev->m_next; 
						pent->m_prev=pent->m_prev->m_next);
					pent->m_prev->m_next = pent;
				} else
					pent->m_prev = 0;
				if (m_pTypedEntsPerm[pent->m_iSimClass]==m_pTypedEnts[pent->m_iSimClass])
					m_pTypedEnts[pent->m_iSimClass] = pent;
				m_pTypedEntsPerm[pent->m_iSimClass] = pent;
			}
			i = pent->m_iPrevSimClass;
			pent->m_iPrevSimClass = pent->m_iSimClass;

			if (pent->m_flags & pef_monitor_state_changes && m_pEventClient)
				m_pEventClient->OnStateChange(pent,pent->m_pForeignData,pent->m_iForeignData,i,pent->m_iSimClass);

/*#ifdef _DEBUG
CPhysicalEntity *ptmp = m_pTypedEnts[1];
for(;ptmp && ptmp!=m_pTypedEntsPerm[1]; ptmp=ptmp->m_next);
if (ptmp!=m_pTypedEntsPerm[1])
DEBUG_BREAK;
#endif*/
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////


struct entity_grid_checker {
	geom_world_data gwd;
	intersection_params ip;
	int nMaxHits,nThroughHits,nThroughHitsAux,objtypes,nEnts,bCallbackUsed;
	unsigned int flags,flagsCollider;
	vector2df org2d,dir2d;
	float dir2d_len,maxt;
	ray_hit *phits;
	CPhysicalWorld *pWorld;
	CPhysicalPlaceholder *pGridEnt;
	void *pSkipForeignData;
	CRayGeom aray;
	entity_grid_checker() {}

	int check_cell(const vector2di &icell, int &ilastcell) {
		quotientf t((org2d+icell)*dir2d, dir2d_len*dir2d_len);
		if (t.x>maxt && (icell.x&icell.y)!=-1)
			return 1;

		box bbox;
		bbox.Basis.SetIdentity();
		bbox.bOriented = 0;
		geom_contact *pcontacts;
		pe_gridthunk *thunk,*thunk_next;
		int i,j,ihit,nCellEnts=0,nEntsChecked=0;

		for(thunk=pWorld->m_pEntGrid[pWorld->m_entgrid.getcell_safe(icell.x,icell.y)]; thunk; thunk=thunk_next) {
			thunk_next = thunk->next;
			if (!thunk->pent->m_bProcessed && objtypes & 1u<<thunk->pent->m_iSimClass && 
					(!pSkipForeignData || thunk->pent->GetForeignData()!=pSkipForeignData)) 
			{
				bbox.center = (thunk->pent->m_BBox[0]+thunk->pent->m_BBox[1])*0.5f;
				bbox.size = (thunk->pent->m_BBox[1]-thunk->pent->m_BBox[0])*0.5f;
				nCellEnts++;

				if ((bbox.center-aray.m_ray.origin-aray.m_dirn*((bbox.center-aray.m_ray.origin)*aray.m_dirn)).len2() > bbox.size.len2())
					continue; // skip objects that lie to far from the ray

				//if ((ptc-aray.m_ray.origin)*aray.m_dirn-fabsf(size.x*aray.m_dirn.x)-fabs(size.y*aray.m_dirn.y)-fabs(size.z*aray.m_dirn.z) > phits[0].dist)
				//	continue;	// skip objects that lie farther than the last ray hit point
				if (!box_ray_overlap_check(&bbox,&aray.m_ray))
					continue;
				nEntsChecked++;	
				pWorld->m_bGridThunksChanged = 0;
				CPhysicalEntity *pent = (pGridEnt=thunk->pent)->GetEntity();
				if (pWorld->m_bGridThunksChanged)
					thunk_next = pWorld->m_pEntGrid[pWorld->m_entgrid.getcell_safe(icell.x,icell.y)];
				pWorld->m_bGridThunksChanged = 0;

				bCallbackUsed = 0;
				if (pent->m_nParts==0 || pent->m_flags&pef_use_geom_callbacks) {
					j = pent->RayTrace(&aray,pcontacts);
					i=0; bCallbackUsed=1; goto gotcontacts;
				}

				for(i=0;i<pent->m_nParts;i++) 
				if ((pent->m_parts[i].flags & flagsCollider)==flagsCollider) {
					if (pent->m_nParts>1) {
						bbox.center = (pent->m_parts[i].BBox[0]+pent->m_parts[i].BBox[1])*0.5f;
						bbox.size = (pent->m_parts[i].BBox[1]-pent->m_parts[i].BBox[0])*0.5f;
						if (!box_ray_overlap_check(&bbox,&aray.m_ray))
							continue;
					}
					gwd.offset = pent->m_pos + pent->m_qrot*pent->m_parts[i].pos;
					//(pent->m_qrot*pent->m_parts[i].q).getmatrix(gwd.R);	//Q2M_IVO 
					gwd.R = matrix3x3f(pent->m_qrot*pent->m_parts[i].q);
					gwd.scale = pent->m_parts[i].scale;
					j = pent->m_parts[i].pPhysGeom->pGeom->Intersect(&aray, &gwd,0, &ip, pcontacts);
					gotcontacts:

					for(j--; j>=0; j--) 
					if (pcontacts[j].t<phits[0].dist && (!(flags & rwi_ignore_back_faces) || pcontacts[j].n*aray.m_dirn<0)) {
						ihit = 0;
						if (pcontacts[j].id[0]<0) pcontacts[j].id[0] = pent->m_parts[i].surface_idx;
						if ((flags & rwi_pierceability_mask) < 
								(pWorld->m_SurfaceFlagsTable[pcontacts[j].id[0]&NSURFACETYPES-1] & sf_pierceable_mask))
						{
							if ((pWorld->m_SurfaceFlagsTable[pcontacts[j].id[0]&NSURFACETYPES-1]|flags) & sf_important) {
								for(ihit=1; ihit<=nThroughHits && phits[ihit].dist<pcontacts[j].t; ihit++);
								if (ihit<=nThroughHits)
									memmove(phits+ihit+1, phits+ihit, (min(nThroughHits+1,nMaxHits-1)-ihit)*sizeof(ray_hit));
								else if (nThroughHits+1==nMaxHits)
									continue;
								nThroughHits = min(nThroughHits+1, nMaxHits-1);
								nThroughHitsAux = min(nThroughHitsAux, nMaxHits-1-nThroughHits);
							}	else {
								for(ihit=nMaxHits-1; ihit>=nMaxHits-nThroughHitsAux && phits[ihit].dist<pcontacts[j].t; ihit--);
								if (ihit>=nMaxHits-nThroughHitsAux) {
									int istart = max(nMaxHits-nThroughHitsAux-1,nThroughHits+1);
									memmove(phits+istart, phits+istart+1, (ihit-istart)*sizeof(ray_hit));
								} else if (nThroughHits+nThroughHitsAux>=nMaxHits-1)
									continue;
								nThroughHitsAux = min(nThroughHitsAux+1, nMaxHits-1-nThroughHits);
							}
						} else {
							ilastcell = 
								float2int((pcontacts[j].pt[inc_mod3[pWorld->m_iEntAxisz]]-pWorld->m_entgrid.origin[inc_mod3[pWorld->m_iEntAxisz]])*
									pWorld->m_entgrid.stepr.x-0.5f) |
								float2int((pcontacts[j].pt[dec_mod3[pWorld->m_iEntAxisz]]-pWorld->m_entgrid.origin[dec_mod3[pWorld->m_iEntAxisz]])*
									pWorld->m_entgrid.stepr.y-0.5f)<<16;
							aray.m_ray.dir = pcontacts[j].pt-aray.m_ray.origin;
						}
						phits[ihit].dist = pcontacts[j].t;
						phits[ihit].pCollider = pent; 
						if (!bCallbackUsed)
							phits[ihit].partid = pent->m_parts[phits[ihit].ipart=i].id;
						else
							phits[ihit].partid = phits[ihit].ipart = pcontacts[j].iNode[0];
						phits[ihit].surface_idx = pcontacts[j].id[0];
						phits[ihit].pt = pcontacts[j].pt; 
						phits[ihit].n = pcontacts[j].n;
						phits[ihit].bTerrain = 0;
					}
				}

				pWorld->m_pTmpEntList[nEnts++] = pent;
				pGridEnt->m_bProcessed = 1;
			}
		}
		return (sgn((icell.y<<16|icell.x)-ilastcell)&1)^1;
	}
};

int CPhysicalWorld::RayWorldIntersection(vectorf org,vectorf dir, int objtypes, unsigned int flags, ray_hit *hits,int nMaxHits, 
																				 IPhysicalEntity *pSkipEnt, IPhysicalEntity *pSkipEntAux)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	if (!(dir.len2()<25E6f && org.len2()<4E8f)) {
		VALIDATOR_LOG(m_pLog,"RayWorldIntersection: ray is out of bounds");
		if (m_vars.bBreakOnValidation) DoBreak
		return 0;
	}
	if (dir.len2()==0)
		return 0;
	int i,nHits; for(i=0;i<nMaxHits;i++) { hits[i].dist=1E10; hits[i].bTerrain=0; }
	if (pSkipEnt) {
		((CPhysicalPlaceholder*)pSkipEnt)->m_bProcessed = 1;
		if (((CPhysicalPlaceholder*)pSkipEnt)->m_pEntBuddy)
			((CPhysicalPlaceholder*)pSkipEnt)->m_pEntBuddy->m_bProcessed = 1;
	}
	if (pSkipEntAux) {
		((CPhysicalPlaceholder*)pSkipEntAux)->m_bProcessed = 1;
		if (((CPhysicalPlaceholder*)pSkipEntAux)->m_pEntBuddy)
			((CPhysicalPlaceholder*)pSkipEntAux)->m_pEntBuddy->m_bProcessed = 1;
	}

	if ((objtypes & ent_terrain) && m_pHeightfield) {
		geom_world_data gwd;
		geom_contact *pcontacts;
		CRayGeom aray(org,dir);
		gwd.R = m_HeightfieldBasis.T();
		gwd.offset = m_HeightfieldOrigin;
		if (m_pHeightfield->m_parts[0].pPhysGeom->pGeom->Intersect(&aray, &gwd,0, 0, pcontacts) && 
				(pcontacts->id[0]>=0 || flags&rwi_ignore_terrain_holes)) 
		{
			dir = pcontacts->pt-org;
			hits[0].dist = pcontacts->t;
			hits[0].pCollider = m_pHeightfield; 
			hits[0].partid = hits[0].ipart = 0;
			hits[0].surface_idx = pcontacts->id[0];
			hits[0].pt = pcontacts->pt; 
			hits[0].n = pcontacts->n;
			hits[0].bTerrain = 1;
		}
	}

	entity_grid_checker egc;
	egc.phits = hits;
	egc.pWorld = this;
	egc.objtypes = objtypes;
	egc.flags = flags^rwi_separate_important_hits;
	if (!(egc.flagsCollider = flags>>rwi_colltype_bit))
		egc.flagsCollider = geom_colltype_ray;
	if (flags & rwi_ignore_noncolliding)
		egc.flagsCollider |= geom_colltype0;
	egc.nMaxHits = nMaxHits;
	egc.nThroughHits = egc.nThroughHitsAux = 0;
	egc.pSkipForeignData = pSkipEnt ? pSkipEnt->GetForeignData() : 0;
	egc.ip.bStopAtFirstTri = (flags & rwi_any_hit)!=0;

	vectorf origin_grid = (org-m_entgrid.origin).permutated(m_iEntAxisz), dir_grid = dir.permutated(m_iEntAxisz);
	egc.aray.CreateRay(org,dir);
	egc.org2d.set(0.5f-origin_grid.x*m_entgrid.stepr.x, 0.5f-origin_grid.y*m_entgrid.stepr.y);
	egc.dir2d.set(dir_grid.x*m_entgrid.stepr.x, dir_grid.y*m_entgrid.stepr.y);
	egc.dir2d_len = egc.dir2d.len();
	egc.maxt = egc.dir2d_len*(egc.dir2d_len+sqrt2);
	egc.nEnts = 0;

	if (fabsf(origin_grid.x*m_entgrid.stepr.x*2-m_entgrid.size.x)>m_entgrid.size.x || 
			fabsf(origin_grid.y*m_entgrid.stepr.y*2-m_entgrid.size.y)>m_entgrid.size.y || 
			fabsf((origin_grid.x+dir_grid.x)*m_entgrid.stepr.x*2-m_entgrid.size.x)>m_entgrid.size.x || 
			fabsf((origin_grid.y+dir_grid.y)*m_entgrid.stepr.y*2-m_entgrid.size.y)>m_entgrid.size.y)
		egc.check_cell(vector2di(-1,-1),i);

	DrawRayOnGrid(&m_entgrid, origin_grid,dir_grid, egc);
	for(i=0;i<egc.nEnts;i++) { 
		m_pTmpEntList[i]->m_bProcessed = 0;
		if (m_pTmpEntList[i]->m_pEntBuddy)
			m_pTmpEntList[i]->m_pEntBuddy->m_bProcessed = 0;
	}
	if (pSkipEnt) {
		((CPhysicalPlaceholder*)pSkipEnt)->m_bProcessed = 0;
		if (((CPhysicalPlaceholder*)pSkipEnt)->m_pEntBuddy)
			((CPhysicalPlaceholder*)pSkipEnt)->m_pEntBuddy->m_bProcessed = 0;
	}
	if (pSkipEntAux) {
		((CPhysicalPlaceholder*)pSkipEntAux)->m_bProcessed = 0;
		if (((CPhysicalPlaceholder*)pSkipEntAux)->m_pEntBuddy)
			((CPhysicalPlaceholder*)pSkipEntAux)->m_pEntBuddy->m_bProcessed = 0;
	}

	if (flags & rwi_separate_important_hits) {
		int j,idx[2]; ray_hit thit;
		for(idx[0]=1,idx[1]=nMaxHits-1,i=1; idx[0]+nMaxHits-idx[1]-2<egc.nThroughHits+egc.nThroughHitsAux; i++) {
			j = isneg(hits[idx[1]].dist-hits[idx[0]].dist);	// j = hits[1].dist<hits[0].dist ? 1:0;
			j |= isneg(egc.nThroughHits-idx[0]);						// if (idx[0]>nThroughHits) j = 1; 
			j &= nMaxHits-egc.nThroughHitsAux-1-idx[1]>>31; // if (idx[1]<=nMaxHits-nThroughHits1-1) j=0;	
			hits[idx[j]].bTerrain = i;
			idx[j] += 1-j*2;
		}
		for(i=egc.nThroughHits+1; i<nMaxHits-egc.nThroughHitsAux; i++)
			hits[i].bTerrain = nMaxHits+1;
		for(i=1;i<nMaxHits;) if (hits[i].bTerrain!=i && hits[i].bTerrain<nMaxHits) {
			thit=hits[hits[i].bTerrain]; hits[hits[i].bTerrain]=hits[i]; hits[i]=thit;
		}	else i++;
	}

	nHits = 0;
	if (hits[0].dist>1E9f) hits[0].dist = -1;
	else nHits++;
	for(i=1;i<nMaxHits;i++) if (hits[i].dist>1E9f || hits[0].dist>0 && hits[i].dist>hits[0].dist)
		hits[i].dist = -1;
	else { hits[i].bTerrain=0; nHits++; }

	return nHits;
}


float CPhysicalWorld::IsAffectedByExplosion(IPhysicalEntity *pobj)
{
	int i;
	CPhysicalEntity *pent = ((CPhysicalPlaceholder*)pobj)->GetEntityFast();
	for(i=0;i<m_nExplVictims && m_pExplVictims[i]!=pent;i++);
	return i<m_nExplVictims ? m_pExplVictimsFrac[i] : 0.0f;
}


void CPhysicalWorld::SimulateExplosion(vectorf epicenter,vectorf epicenterImp, float rmin,float rmax, float r,float impulsive_pressure_at_r, 
																			 int nOccRes,int nGrow,float rmin_occ, IPhysicalEntity **pSkipEnts,int nSkipEnts, int iTypes)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );
	
	CPhysicalEntity **pents;
	int nents,nents1,i,j;
	RigidBody *pbody;
	float kr=impulsive_pressure_at_r*(r*r),maxspeed=15,E,frac=1.0f,sumFrac,sumV;
	geom_world_data gwd;
	CPhysicalPlaceholder **pSkipPcs = (CPhysicalPlaceholder**)pSkipEnts;
	pe_action_impulse shockwave;
	shockwave.iApplyTime = 2;
	shockwave.iSource = 2;

	for(i=0;i<nSkipEnts;i++)
		((!pSkipPcs[i]->m_pEntBuddy || IsPlaceholder(pSkipPcs[i])) ? pSkipPcs[i] : pSkipPcs[i]->m_pEntBuddy)->m_bProcessed = 1;

	if (nOccRes>0) {
//if (m_vars.iDrawHelpers & 1)
//epicenter = m_lastEpicenter;
		if (nOccRes>m_nOccRes) for(i=0;i<6;i++) {
			if (m_nOccRes) {
				delete[] m_pGridStat[i]; delete[] m_pGridDyn[i];
			}
			m_pGridStat[i] = new int[nOccRes*nOccRes];
			m_pGridDyn[i] = new int[nOccRes*nOccRes];
		}
		for(i=0;i<6;i++) for(j=nOccRes*nOccRes-1;j>=0;j--) 
			m_pGridStat[i][j] = (1u<<31)-1;
		m_lastEpicenter = epicenter;
		m_lastRmax = rmax;
		
		for(nents=GetEntitiesAround(epicenter-vectorf(rmax,rmax,rmax),epicenter+vectorf(rmax,rmax,rmax),pents,ent_terrain|ent_static)-1; nents>=0; nents--)
		for(i=0;i<pents[nents]->m_nParts;i++) if (pents[nents]->m_parts[i].flags & geom_colltype_explosion) {
			//(pents[nents]->m_qrot*pents[nents]->m_parts[i].q).getmatrix(gwd.R);	//Q2M_IVO 
			gwd.R = matrix3x3f(pents[nents]->m_qrot*pents[nents]->m_parts[i].q);
			gwd.offset = pents[nents]->m_pos + pents[nents]->m_qrot*pents[nents]->m_parts[i].pos - epicenter;
			gwd.scale = pents[nents]->m_parts[i].scale;
			pents[nents]->m_parts[i].pPhysGeomProxy->pGeom->BuildOcclusionCubemap(&gwd, 0, m_pGridStat,m_pGridDyn,nOccRes, rmin_occ,rmax,nGrow);
		}
	}
	if (nOccRes>=0)
		m_nOccRes = nOccRes;

	nents = GetEntitiesAround(epicenter-vectorf(rmax,rmax,rmax),epicenter+vectorf(rmax,rmax,rmax),pents, iTypes);
	if (nOccRes<0 && m_nOccRes>=0) {
		// special case: reuse the previous m_pGridStat and process only entities that were not affected by the previous call
		for(i=nents1=0;i<nents;i++) {
			for(j=0;j<m_nExplVictims && m_pExplVictims[j]!=pents[i];j++);
			if (j==m_nExplVictims)
				pents[nents1++] = pents[i];
		}
		nOccRes=m_nOccRes; nents=nents1;
	}
	if (m_nExplVictimsAlloc<nents) {
		if (m_nExplVictimsAlloc) {
			delete[] m_pExplVictims; delete[] m_pExplVictimsFrac;
		}
		m_pExplVictims = new CPhysicalEntity*[m_nExplVictimsAlloc=nents];
		m_pExplVictimsFrac = new float[m_nExplVictimsAlloc];
	}

	for(nents--,m_nExplVictims=0; nents>=0; nents--) {
		for(i=0,sumFrac=sumV=0.0f; i<pents[nents]->m_nParts; i++) 
		if (pents[nents]->m_parts[i].flags & geom_colltype_explosion || pents[nents]->m_flags & pef_use_geom_callbacks) {
			//(pents[nents]->m_qrot*pents[nents]->m_parts[i].q).getmatrix(gwd.R);	//Q2M_IVO 
			gwd.R = matrix3x3f(pents[nents]->m_qrot*pents[nents]->m_parts[i].q);
			gwd.offset = pents[nents]->m_pos + pents[nents]->m_qrot*pents[nents]->m_parts[i].pos;
			gwd.scale = pents[nents]->m_parts[i].scale;

			if (nOccRes>0) {
				gwd.offset -= epicenter;
				frac = pents[nents]->m_parts[i].pPhysGeomProxy->pGeom->BuildOcclusionCubemap(&gwd, 1, m_pGridStat,m_pGridDyn,nOccRes, rmin_occ,rmax,nGrow);
				gwd.offset += epicenter;
				sumFrac += pents[nents]->m_parts[i].pPhysGeomProxy->V*frac;
				sumV += pents[nents]->m_parts[i].pPhysGeomProxy->V;
			}

			if (kr>0) {
				if (!(pents[nents]->m_flags & pef_use_geom_callbacks)) {
					shockwave.impulse.zero(); shockwave.momentum.zero();
					pbody = pents[nents]->GetRigidBody(i);
					pents[nents]->m_parts[i].pPhysGeomProxy->pGeom->CalcVolumetricPressure(&gwd, epicenterImp,kr,rmin, pbody->pos, 
						shockwave.impulse,shockwave.momentum);
					shockwave.impulse *= frac; shockwave.momentum *= frac;
					shockwave.ipart = i;
					if ((E=shockwave.impulse.len2()*sqr(pbody->Minv))>sqr(maxspeed))
						shockwave.impulse *= sqrt_tpl(sqr(maxspeed)/E);
					if ((E=shockwave.momentum*(pbody->Iinv*shockwave.momentum)*pbody->Minv)>sqr(maxspeed))
						shockwave.momentum *= sqrt_tpl(sqr(maxspeed)/E);
					pents[nents]->Action(&shockwave);
				} else
					pents[nents]->ApplyVolumetricPressure(epicenterImp,kr*frac,rmin);
			}
		}

		m_pExplVictims[m_nExplVictims] = pents[nents];
		m_pExplVictimsFrac[m_nExplVictims++] = sumV>0 ? sumFrac/sumV : 1.0f;
	}

	for(i=0;i<nSkipEnts;i++)
		((!pSkipPcs[i]->m_pEntBuddy || IsPlaceholder(pSkipPcs[i])) ? pSkipPcs[i] : pSkipPcs[i]->m_pEntBuddy)->m_bProcessed = 0;
}


void CPhysicalWorld::ResetDynamicEntities()
{
	int i; CPhysicalEntity *pent;
	pe_action_reset reset;
	for(i=1;i<=4;i++) for(pent=m_pTypedEnts[i]; pent; pent=pent->m_next)
		pent->Action(&reset);
}


void CPhysicalWorld::DestroyDynamicEntities()
{
	int i; CPhysicalEntity *pent,*pent_next;

	m_nDynamicEntitiesDeleted = 0;
	for(i=1;i<=4;i++) {
		for(pent=m_pTypedEnts[i]; pent; pent=pent_next) {
			pent_next = pent->m_next;
			if (pent->m_pEntBuddy) {
				pent->m_pEntBuddy->m_pEntBuddy = 0;
				pent->m_pEntBuddy->m_nGridThunks = 0;
				DestroyPhysicalEntity(pent->m_pEntBuddy);
			}	else
				SetPhysicalEntityId(pent,-1);
			DetachEntityGridThunks(pent);
			pent->m_nGridThunks = pent->m_nGridThunksAlloc = 0;
			if (pent->m_next = m_pTypedEnts[7]) 
				pent->m_next->m_prev = pent;
			m_pTypedEnts[7] = pent;	
			pent->m_iPrevSimClass = -1; pent->m_iSimClass = 7;
			m_nDynamicEntitiesDeleted++;
		}
		m_pTypedEnts[i] = m_pTypedEntsPerm[i] = 0;
	}

	m_nEnts -= m_nDynamicEntitiesDeleted;
	if (m_nEnts < m_nEntsAlloc-512) {
		int nEntsAlloc = (m_nEnts-1&~511)+512;
		ReallocateList(m_pTmpEntList,m_nEntsAlloc,nEntsAlloc);
		ReallocateList(m_pTmpEntList1,m_nEntsAlloc,nEntsAlloc);
		ReallocateList(m_pGroupMass,0,nEntsAlloc);
		ReallocateList(m_pMassList,0,nEntsAlloc);
		ReallocateList(m_pGroupIds,0,nEntsAlloc);
		ReallocateList(m_pGroupNums,0,nEntsAlloc);
		m_nEntsAlloc = nEntsAlloc;
	}
}

void CPhysicalWorld::PurgeDeletedEntities()
{
	CPhysicalEntity *pent,*pent_next;
	for(pent=m_pTypedEnts[7]; pent; pent=pent_next) { // purge deletion requests
		pent_next = pent->m_next; delete pent;
	}
	m_pTypedEnts[7] = 0;
}


void CPhysicalWorld::DrawPhysicsHelperInformation(void (*DrawLineFunc)(float*,float*))
{
	int entype; CPhysicalEntity *pent;
	if (m_vars.iDrawHelpers) {
		for(entype=0;entype<=4;entype++) if (m_vars.iDrawHelpers & 0x100<<entype)
		for(pent=m_pTypedEnts[entype]; pent; pent=pent->m_next)
			pent->DrawHelperInformation(DrawLineFunc, m_vars.iDrawHelpers);
	}

	if (m_vars.iDrawHelpers & 8192 && m_nOccRes) {
		float zscale,xscale,xoffs,z;
		int i,ix,iy,cx,cy,cz;
		vectorf pt0,pt1;
		zscale = m_lastRmax*(1.0/65535.0f);
		xscale = 2.0f/m_nOccRes;
		xoffs = 1.0f-xscale;
		for(i=0;i<6;i++) {
			cz=i>>1; cx=inc_mod3[cz]; cy=dec_mod3[cz];
			for(iy=0;iy<m_nOccRes;iy++) for(ix=0;ix<m_nOccRes;ix++) if (m_pGridStat[i][iy*m_nOccRes+ix]<(1u<<31)-1) {
				pt0[cz] = (z=m_pGridStat[i][iy*m_nOccRes+ix]*zscale)*((i&1)*2-1);
				pt0[cx] = ((ix+0.5f)*xscale-1.0f)*z;
				pt0[cy] = ((iy+0.5f)*xscale-1.0f)*z;
				pt0 += m_lastEpicenter;
				DrawLineFunc(m_lastEpicenter,pt0);
				pt0[cx] -= z*xscale*0.5f; pt0[cy] -= z*xscale*0.5f;
				pt1=pt0; pt1[cx] += z*xscale; pt1[cy] += z*xscale;
				DrawLineFunc(pt0,pt1);
				pt0[cy] += z*xscale; pt1[cy] -= z*xscale;
				DrawLineFunc(pt0,pt1);
			}
		}
	}

	if (m_vars.bLogActiveObjects) {
		m_vars.bLogActiveObjects = 0;
		int i,nPrims,nCount=0;
		RigidBody *pbody;
		for(pent=m_pTypedEnts[2]; pent; pent=pent->m_next,nCount++) {
			for(i=nPrims=0;i<pent->m_nParts;i++) nPrims += ((CGeometry*)pent->m_parts[i].pPhysGeomProxy->pGeom)->GetPrimitiveCount();
			pbody = pent->GetRigidBody();
			m_pLog->Log("\001%s @ %7.2f,%7.2f,%7.2f, mass %.2f, v %.1f, w %.1f, #polies %d, id %d",
				m_pPhysicsStreamer->GetForeignName(pent->m_pForeignData,pent->m_iForeignData,pent->m_iForeignFlags),
				pent->m_pos.x,pent->m_pos.y,pent->m_pos.z, pbody->M,pbody->v.len(),pbody->w.len(),nPrims,pent->m_id);
		}
		m_pLog->Log("\001%d active object(s)",nCount);
	}
}


int CPhysicalWorld::CollideEntityWithBeam(IPhysicalEntity *_pent, vectorf org,vectorf dir,float r, ray_hit *phit)
{
	CPhysicalEntity *pent = (CPhysicalEntity*)_pent;
	CSphereGeom SweptSph;
	geom_contact *pcontacts;
	geom_world_data gwd[2];
	sphere asph;
	asph.r = r;
	asph.center.zero();
	SweptSph.CreateSphere(&asph);
	intersection_params ip;
	ip.bSweepTest = 1;
	gwd[0].R.SetIdentity();
	gwd[0].offset = org;
	gwd[0].v = dir;
	ip.time_interval = 1.0f;
	phit->dist = 1E10;

	for(int i=0;i<pent->m_nParts;i++) if (pent->m_parts[i].flags & geom_collides) {
		gwd[1].offset = pent->m_pos + pent->m_qrot*pent->m_parts[i].pos;
		//(pent->m_qrot*pent->m_parts[i].q).getmatrix(gwd[1].R); //Q2M_IVO 
		gwd[1].R = matrix3x3f(pent->m_qrot*pent->m_parts[i].q);
		gwd[1].scale = pent->m_parts[i].scale;
		if (SweptSph.Intersect(pent->m_parts[i].pPhysGeom->pGeom, gwd,gwd+1, &ip, pcontacts) && pcontacts->t<phit->dist) {
			phit->dist = pcontacts->t;
			phit->pCollider = pent;
			phit->partid = pent->m_parts[phit->ipart=i].id;
			phit->surface_idx = pcontacts->id[1]<0 ? pent->m_parts[i].surface_idx : pcontacts->id[1];
			phit->pt = pcontacts->pt;
			phit->n = -pcontacts->n;
		}
	}

	return isneg(phit->dist-1E9f);
}


int CPhysicalWorld::RayTraceEntity(IPhysicalEntity *pient, vectorf origin,vectorf dir, ray_hit *pHit, pe_params_pos *pp)
{
	if (!(dir.len2()>0 && origin.len2()>=0))
		return 0;

	CPhysicalEntity *pent = ((CPhysicalPlaceholder*)pient)->GetEntity();
	int i,ncont;
	vectorf pos;
	quaternionf qrot;
	float scale = 1.0f;
	CRayGeom aray(origin,dir);
	geom_world_data gwd;
	geom_contact *pcontacts;
	pHit->dist = 1E10;
	if (pp) {
		pos = pp->pos; qrot = pp->q; 
		if (!is_unused(pp->scale)) scale = pp->scale;
		get_xqs_from_matrices(pp->pMtx3x3,pp->pMtx3x3T,pp->pMtx4x4,pp->pMtx4x4T, pos,qrot,scale);
	}	else {
		pos = pent->m_pos; qrot = pent->m_qrot;
	}

	for(i=0;i<pent->m_nParts;i++) {
		//(pent->m_qrot*pent->m_parts[i].q).getmatrix(gwd.R);	//Q2M_IVO 
		gwd.R = matrix3x3f(qrot*pent->m_parts[i].q);
		gwd.offset = pos + qrot*pent->m_parts[i].pos;
		gwd.scale = scale*pent->m_parts[i].scale;
		for(ncont=pent->m_parts[i].pPhysGeom->pGeom->Intersect(&aray,&gwd,0,0,pcontacts); 
				ncont>0 && pcontacts[ncont-1].t<pHit->dist && pcontacts[ncont-1].n*dir>0; ncont--);
		if (ncont>0) {
			pHit->dist = pcontacts[ncont-1].t;
			pHit->pCollider = pent; pHit->partid = pent->m_parts[pHit->ipart=i].id;
			pHit->surface_idx = pcontacts[ncont-1].id[0]&~(pcontacts[ncont-1].id[0]>>31) | pent->m_parts[i].surface_idx&pcontacts[ncont-1].id[0]>>31;
			pHit->pt = pcontacts[ncont-1].pt; 
			pHit->n = pcontacts[ncont-1].n;
		}
	}

	return isneg(pHit->dist-1E9);
}


CPhysicalEntity *CPhysicalWorld::CheckColliderListsIntegrity()
{
	int i,j,k;
	CPhysicalEntity *pent;
	for(i=1;i<=2;i++) for(pent=m_pTypedEnts[i];pent;pent=pent->m_next)
		for(j=0;j<pent->m_nColliders;j++) if (pent->m_pColliders[j]->m_iSimClass>0) {
			for(k=0;k<pent->m_pColliders[j]->m_nColliders && pent->m_pColliders[j]->m_pColliders[k]!=pent;k++);
			if (k==pent->m_pColliders[j]->m_nColliders)
				return pent;
		}
	return 0;
}


void CPhysicalWorld::GetMemoryStatistics(ICrySizer *pSizer)
{
	static char *entnames[] = { "static entities", "physical entities", "physical entities", "living entities", "detached entities", 
		"","", "deleted entities" };
	int i,j,n;
	CPhysicalEntity *pent;

/*#ifdef WIN32
	static char *sec_ids[] = { ".text",".textbss",".data",".idata" };
	static char *sec_names[] = { "code section","code section","data section","data section" };
	_IMAGE_DOS_HEADER *pMZ = (_IMAGE_DOS_HEADER*)GetModuleHandle("CryPhysics.dll");
	_IMAGE_NT_HEADERS *pPE = (_IMAGE_NT_HEADERS*)((char*)pMZ+pMZ->e_lfanew);
	IMAGE_SECTION_HEADER *sections = IMAGE_FIRST_SECTION(pPE);
	for(i=0;i<pPE->FileHeader.NumberOfSections;i++) for(j=0;j<sizeof(sec_ids)/sizeof(sec_ids[0]);j++)
	if (!strncmp((char*)sections[i].Name, sec_ids[j], min(8,strlen(sec_ids[j])+1))) {
		SIZER_COMPONENT_NAME(pSizer, sec_names[j]);
		pSizer->AddObject((void*)sections[i].VirtualAddress, sections[i].Misc.VirtualSize);
	}
#endif*/

	{ SIZER_COMPONENT_NAME(pSizer,"world structures");
		pSizer->AddObject(this, sizeof(CPhysicalWorld));
		pSizer->AddObject(m_pTmpEntList, m_nEntsAlloc*sizeof(m_pTmpEntList[0]));
		pSizer->AddObject(m_pTmpEntList1, m_nEntsAlloc*sizeof(m_pTmpEntList1[0]));
		pSizer->AddObject(m_pGroupMass, m_nEntsAlloc*sizeof(m_pGroupMass[0]));
		pSizer->AddObject(m_pMassList, m_nEntsAlloc*sizeof(m_pMassList[0]));
		pSizer->AddObject(m_pGroupIds, m_nEntsAlloc*sizeof(m_pGroupIds[0]));
		pSizer->AddObject(m_pGroupNums, m_nEntsAlloc*sizeof(m_pGroupNums[0]));
		pSizer->AddObject(m_pEntsById, m_nEntsAlloc*sizeof(m_pEntsById[0]));
		pSizer->AddObject(m_pEntGrid, (m_entgrid.size.x*m_entgrid.size.y+1)*sizeof(m_pEntGrid[0]));
		pSizer->AddObject(m_pGridStat, m_nOccRes*6*2*sizeof(m_pGridStat[0][0]));
		pSizer->AddObject(m_pExplVictims, m_nExplVictimsAlloc*(sizeof(m_pExplVictims[0]+sizeof(m_pExplVictimsFrac[0]))));
	}

	{	SIZER_COMPONENT_NAME(pSizer,"placeholders");
		for(i=n=0;i<m_nPlaceholderChunks;i++) for(j=0;j<PLACEHOLDER_CHUNK_SZ;j++) 
		if (m_pPlaceholderMap[(i<<PLACEHOLDER_CHUNK_SZLG2)+j>>5] & 1<<(i<<PLACEHOLDER_CHUNK_SZLG2)+j&31 && !m_pPlaceholders[i][j].m_pEntBuddy)
			n += m_pPlaceholders[i][j].m_nGridThunksAlloc*sizeof(pe_gridthunk);
		pSizer->AddObject(m_pPlaceholders, m_nPlaceholderChunks*(sizeof(CPhysicalPlaceholder)*PLACEHOLDER_CHUNK_SZ+
			sizeof(CPhysicalPlaceholder*))+n*sizeof(pe_gridthunk));
		pSizer->AddObject(m_pPlaceholderMap, (m_nPlaceholderChunks<<PLACEHOLDER_CHUNK_SZLG2-5)*sizeof(int));
	}

	{ SIZER_COMPONENT_NAME(pSizer,"entities");
		for(i=0;i<=7;i++) {
			SIZER_COMPONENT_NAME(pSizer,entnames[i]);
			for(pent=m_pTypedEnts[i]; pent; pent=pent->m_next)
				pent->GetMemoryStatistics(pSizer);
		}
	}

	{ SIZER_COMPONENT_NAME(pSizer,"geometries");
		if (m_pHeightfield)
			m_pHeightfield->m_parts[0].pPhysGeom->pGeom->GetMemoryStatistics(pSizer);
		pSizer->AddObject(m_pGeoms, m_nGeomChunks*sizeof(m_pGeoms[0]));
		for(i=0;i<m_nGeomChunks;i++) {
			n = GEOM_CHUNK_SZ&i-m_nGeomChunks+1>>31 | m_nGeomsInLastChunk&m_nGeomChunks-2-i>>31;
			pSizer->AddObject(m_pGeoms[i], n*sizeof(m_pGeoms[i][0]));
			for(j=0;j<n;j++) if (m_pGeoms[i][j].pGeom)
				m_pGeoms[i][j].pGeom->GetMemoryStatistics(pSizer);
		}
	}
}


void CPhysicalWorld::AddEntityProfileInfo(CPhysicalEntity *pent,int nTicks)
{
	if (m_nProfiledEnts==sizeof(m_pEntProfileData)/sizeof(m_pEntProfileData[0]) && 
			nTicks<=m_pEntProfileData[m_nProfiledEnts-1].nTicks)
		return;

	int i,nCalls=1,iBound[2]={ 0,m_nProfiledEnts };
	for(i=0;i<m_nProfiledEnts;i++) if (m_pEntProfileData[i].pEntity==pent) {
		nTicks += m_pEntProfileData[i].nTicks; nCalls += m_pEntProfileData[i].nCalls;
		memmove(m_pEntProfileData+i,m_pEntProfileData+i+1, (--m_nProfiledEnts-i)*sizeof(m_pEntProfileData[0]));
		break;
	}
	do {
		i = iBound[0]+iBound[1]>>1;
		iBound[isneg(m_pEntProfileData[i].nTicks-nTicks)] = i;
	} while(iBound[1]>iBound[0]+1);
	i = iBound[0]+(isneg(-m_nProfiledEnts)&isneg(nTicks-m_pEntProfileData[iBound[0]].nTicks));
	m_nProfiledEnts = min(m_nProfiledEnts+1, sizeof(m_pEntProfileData)/sizeof(m_pEntProfileData[0]));
	memmove(m_pEntProfileData+i+1,m_pEntProfileData+i, (m_nProfiledEnts-1-i)*sizeof(m_pEntProfileData[0]));
	m_pEntProfileData[i].pEntity = pent;
	m_pEntProfileData[i].nTicks = nTicks;
	m_pEntProfileData[i].nCalls = nCalls;
	m_pEntProfileData[i].pName = m_pPhysicsStreamer ? m_pPhysicsStreamer->GetForeignName(pent->m_pForeignData,
			pent->m_iForeignData,pent->m_iForeignFlags) : "noname";
	m_pEntProfileData[i].id = pent->m_id;
}

int CPhysicalWorld::GetEntityProfileInfo(phys_profile_info *&pList)
{
	pList = m_pEntProfileData;
	return m_nProfiledEnts;
}


#ifdef PHYSWORLD_SERIALIZATION
void SerializeGeometries(CPhysicalWorld *pWorld, const char *fname,int bSave);
void SerializeWorld(CPhysicalWorld *pWorld, const char *fname,int bSave);

int CPhysicalWorld::SerializeWorld(const char *fname, int bSave) 
{
	::SerializeWorld(this,fname,bSave);
	return 1;
}
int CPhysicalWorld::SerializeGeometries(const char *fname, int bSave)
{
	::SerializeGeometries(this,fname,bSave);
	return 1;
}
#else
int CPhysicalWorld::SerializeWorld(const char *fname, int bSave) { return 0; }
int CPhysicalWorld::SerializeGeometries(const char *fname, int bSave) { return 0; }
#endif