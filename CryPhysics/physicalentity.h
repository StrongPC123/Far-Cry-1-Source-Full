//////////////////////////////////////////////////////////////////////
//
//	Physical Entity header
//	
//	File: physicalentity.h
//	Description : PhysicalEntity class declarations
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef physicalentity_h
#define physicalentity_h
#pragma once

enum phentity_flags_int { 
	pef_use_geom_callbacks = 0x2000000
};

struct geom {
	phys_geometry *pPhysGeom,*pPhysGeomProxy;
	int id;
	vectorf pos;
	quaternionf q;
	float scale;
	float mass;
	int surface_idx;
	unsigned int flags,flagsCollider;
	float maxdim;
	float minContactDist;
	vectorf BBox[2];
};

class CPhysicalWorld;
class CRayGeom;

class CPhysicalEntity : public CPhysicalPlaceholder {
public:
	CPhysicalEntity(CPhysicalWorld *pworld);
	virtual ~CPhysicalEntity();
	virtual pe_type GetType() { return PE_STATIC; }

	int AddRef() { return ++m_nRefCount; }
	int Release() { return --m_nRefCount; }

	virtual int SetParams(pe_params*);
	virtual int GetParams(pe_params*);
	virtual int GetStatus(pe_status*);
	virtual int Action(pe_action*);
	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1);
	virtual void RemoveGeometry(int id);
	virtual void *GetForeignData(int itype=0) { return itype==m_iForeignData ? m_pForeignData:0; }
	virtual int GetiForeignData() { return m_iForeignData; }
	virtual IPhysicalWorld *GetWorld() { return (IPhysicalWorld*)m_pWorld; }
	virtual CPhysicalEntity *GetEntity() { return this; }
	virtual CPhysicalEntity *GetEntityFast() { return this; }

	virtual void StartStep(float time_interval) {}
	virtual float GetMaxTimeStep(float time_interval) { return time_interval; }
	virtual float GetLastTimeStep(float time_interval) { return time_interval; }
	virtual int Step(float time_interval) { return 1; }
	virtual void StepBack(float time_interval) {} 
	virtual int GetContactCount(int nMaxPlaneContacts) { return 0; }
	virtual int RegisterContacts(float time_interval,int nMaxPlaneContacts) { return 0; }
	virtual int Update(float time_interval, float damping) { return 1; }
	virtual float CalcEnergy(float time_interval) { return 0; }
	virtual float GetDamping(float time_interval) { return 1.0f; } 

	virtual int AddCollider(CPhysicalEntity *pCollider);
	virtual int RemoveCollider(CPhysicalEntity *pCollider, bool bAlwaysRemove=true);
	virtual int RemoveContactPoint(CPhysicalEntity *pCollider, const vectorf &pt, float mindist2) { return -1; }
	virtual int HasContactsWith(CPhysicalEntity *pent) { return 0; }
	virtual int HasCollisionContactsWith(CPhysicalEntity *pent) { return 0; }
	virtual void AlertNeighbourhoodND();
	virtual int Awake(int bAwake=1,int iSource=0) { return 0; }
	virtual int IsAwake(int ipart=-1) { return 0; }
	int GetColliders(CPhysicalEntity **&pentlist) { pentlist=m_pColliders; return m_nColliders; }
	virtual int RayTrace(CRayGeom *pRay, geom_contact *&pcontacts) { return 0; }
	virtual void ApplyVolumetricPressure(const vectorf &epicenter, float kr, float rmin) {}

	virtual RigidBody *GetRigidBody(int ipart=-1);
	virtual void GetContactMatrix(const vectorf &pt, int ipart, matrix3x3f &K) {}
	virtual void GetSpatialContactMatrix(const vectorf &pt, int ipart, float Ibuf[][6]) {}
	virtual float GetMassInv() { return 0; }
	virtual int IsPointInside(vectorf pt);

	virtual void DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags);
	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	virtual int GetStateSnapshot(class CStream &stm, float time_back=0,	int flags=0) { return 0; }
	virtual int SetStateFromSnapshot(class CStream &stm, int flags=0) { return 0; }
	virtual int PostSetStateFromSnapshot() { return 1; }
	virtual unsigned int GetStateChecksum() { return 0; }
	virtual int GetStateSnapshotTxt(char *txtbuf,int szbuf, float time_back=0);
	virtual void SetStateFromSnapshotTxt(const char *txtbuf,int szbuf);

	virtual void ComputeBBox();

	int m_nRefCount;
	unsigned int m_flags;
	CPhysicalEntity *m_next,*m_prev;
	CPhysicalWorld *m_pWorld;

	int m_iPrevSimClass;
	int m_iGroup,m_bMoved;
	CPhysicalEntity *m_next_coll,*m_next_coll1;

	vectorf m_pos;
	quaternionf m_qrot;

	CPhysicalEntity **m_pColliders;
	int m_nColliders,m_nCollidersAlloc;

	CPhysicalEntity *m_next_aux,*m_prev_aux;
	CPhysicalEntity *m_pOuterEntity;
	CGeometry *m_pBoundingGeometry;
	int m_bProcessed_aux;

	float m_timeIdle,m_maxTimeIdle;
	int m_bPermanent;

	geom *m_parts,m_defpart;
	int m_nParts,m_nPartsAlloc;
	int m_iLastIdx;
};

extern RigidBody g_StaticRigidBody;
extern CPhysicalEntity g_StaticPhysicalEntity;

#endif