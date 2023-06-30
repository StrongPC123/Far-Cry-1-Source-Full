//////////////////////////////////////////////////////////////////////
//
//	Living Entity header
//	
//	File: livingentity.h
//	Description : CLivingEntity class header
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef livingentity_h
#define livingentity_h
#pragma once

const int SZ_ACTIONS = 128;
const int SZ_HISTORY = 128;

struct le_history_item {
	vectorf pos;
	quaternionf q;
	vectorf v;
	int bFlying;
	vectorf nslope;
	float timeFlying;
	float minFlyTime;
	float timeUseLowCap;
	int idCollider;
	int iColliderPart;
	vectorf posColl;
	float dt;
};

struct le_contact {
	CPhysicalEntity *pent;
	int ipart;
	vectorf pt,ptloc;
	vectorf n;
	float penetration;
	vectorf center;
};

class CLivingEntity : public CPhysicalEntity {
public:
	CLivingEntity(CPhysicalWorld *pWorld);
	virtual ~CLivingEntity();
	virtual pe_type GetType() { return PE_LIVING; }

	virtual int SetParams(pe_params*);
	virtual int GetParams(pe_params*);
	virtual int GetStatus(pe_status*);
	virtual int Action(pe_action*);
	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual int Step(float time_interval);
	void StepBackEx(float time_interval,bool bRollbackHistory=true);
	virtual void StepBack(float time_interval) { StepBackEx(time_interval); }
	virtual float CalcEnergy(float time_interval);
	virtual int RegisterContacts(float time_interval,int nMaxPlaneContacts);
	virtual int Update(float time_interval, float damping);
	virtual int Awake(int bAwake=1,int iSource=0);
	virtual void AlertNeighbourhoodND() { ReleaseGroundCollider(); CPhysicalEntity::AlertNeighbourhoodND(); }
	virtual void ComputeBBox() { 
		CPhysicalEntity::ComputeBBox(); m_BBox[0].z = min(m_BBox[0].z,m_pos.z-m_hPivot); 
		m_BBox[1].z = max(m_BBox[1].z,m_pos.z-m_hPivot+(m_hHead+m_HeadGeom.m_sphere.r)*isneg(0.001f-m_HeadGeom.m_sphere.r));
	}

	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1);
	virtual void RemoveGeometry(int id);

	virtual void DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags);

	enum snapver { SNAPSHOT_VERSION = 2 };
	virtual int GetStateSnapshot(class CStream &stm, float time_back=0, int flags=0);
	virtual int SetStateFromSnapshot(class CStream &stm, int flags=0);

	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	virtual float GetMassInv() { return m_massinv; }
	virtual void GetContactMatrix(const vectorf &pt, int ipart, matrix3x3f &K) {
		K(0,0)+=m_massinv; K(1,1)+=m_massinv; K(2,2)+=m_massinv;
	}
	virtual void GetSpatialContactMatrix(const vectorf &pt, int ipart, float Ibuf[][6]) {
		Ibuf[3][0]+=m_massinv; Ibuf[4][1]+=m_massinv; Ibuf[5][2]+=m_massinv;
	}
	float ShootRayDown(CPhysicalEntity **pentlist,int nents, const vectorf &pos,vectorf &nslope, float time_interval=0, 
		bool bUseRotation=false,bool bUpdateGroundCollider=false,bool bIgnoreSmallObjects=true);
	void AddLegsImpulse(const vectorf &vel, const vectorf &nslope, bool bInstantChange);
	void ReleaseGroundCollider();
	void SetGroundCollider(CPhysicalEntity *pCollider);
	void SyncWithGroundCollider(float time_interval);
	void RegisterContact(const vectorf& pt,const vectorf& n, CPhysicalEntity *pCollider, int ipart,int idmat);
	void RegisterUnprojContact(const le_contact &unproj);
	int IsPositionFree(const vectorf *BBox,float newh,const vectorf &newdim);

	void AllocateExtendedHistory();

	vectorf m_vel,m_velRequested,m_gravity,m_nslope;
	float m_kInertia,m_kAirControl,m_kAirResistance, m_hCyl,m_hEye,m_hPivot,m_hHead;
	vectorf m_size;
	float m_dh,m_dhSpeed,m_dhAcc,m_stablehTime,m_hLatest,m_nodSpeed;
	float m_mass,m_massinv;
	int m_bFlying,m_bJumpRequested,m_bSwimming, m_surface_idx, m_lastGroundSurfaceIdx;
	float m_timeFlying,m_minFlyTime,m_timeForceInertia;
	float m_slopeSlide,m_slopeClimb,m_slopeJump,m_slopeFall;
	float m_maxVelGround;
	CCylinderGeom m_CylinderGeom;
	CSphereGeom m_SphereGeom,m_HeadGeom;
	phys_geometry m_CylinderGeomPhys;
	int m_bIgnoreCommands;
	int m_bStateReading;
	int m_bActive;
	float m_timeUseLowCap;
	float m_timeSinceStanceChange;
	float m_timeSinceImpulseContact;
	int m_bActiveEnvironment;
	int m_bStuck;
	float m_dhHist[2],m_timeOnStairs;
	float m_timeStepFull,m_timeStepPerformed;
	int m_iSnapshot;
	int m_iTimeLastSend;

	CPhysicalEntity *m_pLastGroundCollider;
	int m_iLastGroundColliderPart;
	vectorf m_posLastGroundColl;
	vectorf m_velGround;
	vectorf m_deltaPos,m_posLocal;
	float m_timeSmooth;
	int m_bUseSphere;

	le_history_item *m_history,m_history_buf[4];
	int m_szHistory,m_iHist;
	pe_action_move *m_actions,m_actions_buf[16];
	int m_szActions,m_iAction;
	coll_history_item m_collision;

	le_contact *m_pContacts;
	int m_nContacts,m_nContactsAlloc;

	int m_nSensors;
	vectorf *m_pSensors,*m_pSensorsPoints,*m_pSensorsSlopes;
	int m_iSensorsActive;
};

#endif