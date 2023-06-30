//////////////////////////////////////////////////////////////////////
//
//	Soft Entity header
//	
//	File: softentity.h
//	Description : SoftEntity class declaration
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef softentity_h
#define softentity_h
#pragma once

struct se_vertex {
	~se_vertex() { if (pContactEnt) pContactEnt->Release(); }
	vectorf pos,pos0,posorg;
	vectorf vel,vel0;
	float massinv;
	float volume;
	vectorf n,ncontact;
	int bSeparating;
	int iSorted,iSorted0;
	float area;
	int iStartEdge,iEndEdge,bFullFan;
	float rnEdges;
	CPhysicalEntity *pContactEnt;
	int iContactPart;
	int iContactNode;
	vectorf vcontact;
	int surface_idx[2];
	int bAttached;
	vectorf ptAttach;
	vectorf P,dv,r,d;
};

struct se_edge {
	int ivtx[2];
	float len0;
	float len,rlen;
	float kd;
};


class CSoftEntity : public CPhysicalEntity {
 public:
	CSoftEntity(CPhysicalWorld *pworld);
	virtual ~CSoftEntity();
	virtual pe_type GetType() { return PE_SOFT; }

	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1);
	virtual void RemoveGeometry(int id);
	virtual int SetParams(pe_params *_params);
	virtual int GetParams(pe_params *_params);
	virtual int Action(pe_action*);
	virtual int GetStatus(pe_status*);

	virtual int Awake(int bAwake=1,int iSource=0) { if (m_bAwake=bAwake) m_nSlowFrames=0; return 1; }
	virtual int IsAwake(int ipart=-1) { return m_bAwake; }
	virtual void AlertNeighbourhoodND();

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual int Step(float time_interval);
	virtual int RayTrace(CRayGeom *pRay, geom_contact *&pcontacts);
	virtual void ApplyVolumetricPressure(const vectorf &epicenter, float kr, float rmin);

	enum snapver { SNAPSHOT_VERSION = 10 };
	virtual int GetStateSnapshot(CStream &stm, float time_back=0,int flags=0);
	virtual int SetStateFromSnapshot(CStream &stm, int flags);

	virtual void DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags);
	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	se_vertex *m_vtx;
	se_edge *m_edges;
	int *m_pVtxEdges;
	int m_nVtx,m_nEdges;
	vectorf m_offs0;
	quaternionf m_qrot0;
	int m_bMeshUpdated;

	float m_timeStepFull;
	float m_timeStepPerformed;

	vectorf m_gravity;
	float m_Emin;
	float m_maxAllowedStep;
	int m_bAwake,m_nSlowFrames;
	float m_damping;
	float m_accuracy;
	int m_nMaxIters;
	float m_prevTimeInterval;

	float m_thickness;
	float m_ks,m_kdRatio;
	float m_maxSafeStep;
	float m_density;
	float m_coverage;
	float m_friction;
	float m_impulseScale;
	float m_explosionScale;
	float m_collImpulseScale;
	float m_maxCollImpulse;
	int m_collTypes;

	plane m_waterPlane;
	float m_waterDensity;
	float m_waterDamping;
	float m_waterResistance;
	vectorf m_waterFlow;

	float m_airResistance;
	vectorf m_wind;
};

#endif