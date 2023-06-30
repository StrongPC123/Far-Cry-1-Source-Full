//////////////////////////////////////////////////////////////////////
//
//	Particle Entity header
//	
//	File: particleentity.h
//	Description : CParticleEntity class declaration
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef particleentity_h
#define particleentity_h
#pragma once

class CParticleEntity : public CPhysicalEntity {
 public:
	CParticleEntity(CPhysicalWorld *pworld);
	virtual ~CParticleEntity();
	virtual pe_type GetType() { return PE_PARTICLE; }

	virtual int SetParams(pe_params*);
	virtual int GetParams(pe_params*);
	virtual int GetStatus(pe_status*);
	virtual int Action(pe_action*);
	virtual int Awake(int bAwake=1,int iSource=0);
	virtual int IsAwake(int ipart=-1);
	virtual int RayTrace(CRayGeom *pRay, geom_contact *&pcontacts);
	virtual void ComputeBBox() { vectorf sz(m_dim,m_dim,m_dim); m_BBox[0]=m_pos-sz; m_BBox[1]=m_pos+sz; }

	enum snapver { SNAPSHOT_VERSION = 3 };
	virtual int GetStateSnapshot(class CStream &stm,float time_back=0,int flags=0);
	virtual int SetStateFromSnapshot(class CStream &stm, int flags);

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual int Step(float time_interval);

	virtual void DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags);
	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	float m_mass,m_dim,m_rdim,m_dimLying;
	float m_kAirResistance,m_kWaterResistance, m_accThrust, m_kAccLift;
	vectorf m_gravity,m_waterGravity,m_dirdown,m_normal;
	vectorf m_heading,m_vel,m_wspin;
	quaternionf m_qspin;
	float m_minBounceVel;
	int m_surface_idx;
	CPhysicalEntity *m_pColliderToIgnore;
	int m_iPierceability;
	int m_bSliding;
	vectorf m_slide_normal;
	float m_timeSurplus;
	plane m_waterPlane;
	float m_waterDensity;
	int m_bForceAwake;
	float m_timeStepPerformed,m_timeStepFull;
	float m_timeForceAwake;
	float m_sleepTime;

	coll_history_item m_CollHistory[4];
	int m_iCollHistory;
};

#endif