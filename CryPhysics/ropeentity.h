#ifndef ropeentity_h
#define ropeentity_h

struct rope_segment {
	~rope_segment() { if (pContactEnt) pContactEnt->Release(); }
	vectorf pt;
	vectorf vel;
	vectorf dir;
	vectorf vel_ext;

	CPhysicalEntity *pContactEnt;
	int iContactPart;
	int bRecheckContact;
	float tcontact;
	vectorf ncontact;
	vectorf vcontact;
	float vreq;
	float friction[2];
	int iPrim,iFeature;
	int bRecalcDir;

	float r,P;
	float dP,dv;
	float cosnext,kdP;
};

class CRopeEntity : public CPhysicalEntity {
 public:
	CRopeEntity(CPhysicalWorld *pworld);
	virtual ~CRopeEntity();
	virtual pe_type GetType() { return PE_ROPE; }

	virtual int SetParams(pe_params*);
	virtual int GetParams(pe_params*);
	virtual int GetStatus(pe_status*);
	virtual int Action(pe_action*);

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual int Step(float time_interval);
	virtual int Awake(int bAwake=1,int iSource=0);
	virtual int IsAwake(int ipart=-1) { return m_bAwake; }
	virtual void AlertNeighbourhoodND();
	virtual int RayTrace(CRayGeom *pRay, geom_contact *&pcontacts);

	enum snapver { SNAPSHOT_VERSION = 8 };
	virtual int GetStateSnapshot(CStream &stm, float time_back=0,int flags=0);
	virtual int SetStateFromSnapshot(CStream &stm, int flags);

	virtual void DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags);
	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	vectorf m_gravity;
	float m_damping;
	float m_maxAllowedStep;
	float m_Emin;
	int m_bAwake;
	float m_timeStepPerformed,m_timeStepFull;
	int m_nSlowFrames;

	float m_length;
	int m_nSegs;
	float m_mass;
	float m_collDist;
	int m_surface_idx;
	float m_friction;
	rope_segment *m_segs;

	CPhysicalEntity *m_pTiedTo[2];
	vectorf m_ptTiedLoc[2];
	int m_iTiedPart[2];
	int m_idConstraint;
	int m_iConstraintClient;
};

#endif