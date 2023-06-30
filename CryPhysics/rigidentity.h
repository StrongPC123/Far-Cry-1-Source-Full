//////////////////////////////////////////////////////////////////////
//
//	Rigid Entity header
//	
//	File: rigidentity.h
//	Description : RigidEntity class declaration
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef rigidentity_h
#define rigidentity_h
#pragma once

typedef uint64 masktype;
#define getmask(i) ((uint64)1<<(i))
const int NMASKBITS = 64;

enum rentity_flags_int { 
	ref_contact_overflow = 0x4000000
};

//typedef unsigned int masktype;
//#define getmask(i) (1u<<(i))
//const int NMASKBITS = 32;

enum constr_info_flags { constraint_limited_1axis=1, constraint_limited_2axes=2, constraint_rope=4 };

struct constraint_info {
	quaternionf qframe_rel[2];
	float limits[2];
	unsigned int flags;
	CPhysicalEntity *pConstraintEnt;
	int bActive;
};

struct checksum_item {
	int iPhysTime;
	unsigned int checksum;
};
const int NCHECKSUMS = 32;

class CRigidEntity : public CPhysicalEntity {
 public:
	CRigidEntity(CPhysicalWorld *pworld);
	virtual ~CRigidEntity();
	virtual pe_type GetType() { return PE_RIGID; }

	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1);
	virtual void RemoveGeometry(int id);
	virtual int SetParams(pe_params *_params);
	virtual int GetParams(pe_params *_params);
	virtual int GetStatus(pe_status*);
	virtual int Action(pe_action*);

	virtual int AddCollider(CPhysicalEntity *pCollider);
	virtual int RemoveCollider(CPhysicalEntity *pCollider, bool bRemoveAlways=true);
	virtual int RemoveContactPoint(CPhysicalEntity *pCollider, const vectorf &pt, float mindist2);
	virtual int HasContactsWith(CPhysicalEntity *pent);
	virtual int HasCollisionContactsWith(CPhysicalEntity *pent);
	virtual int Awake(int bAwake=1,int iSource=0);
	virtual int IsAwake(int ipart=-1) { return m_bAwake; }
	virtual void AlertNeighbourhoodND();

	virtual RigidBody *GetRigidBody(int ipart=-1) { return &m_body; }
	virtual void GetContactMatrix(const vectorf &pt, int ipart, matrix3x3f &K) { m_body.GetContactMatrix(pt-m_body.pos,K); }
	virtual float GetMassInv() { return m_body.Minv; }

	enum snapver { SNAPSHOT_VERSION = 9 };
	virtual int GetSnapshotVersion() { return SNAPSHOT_VERSION; }
	virtual int GetStateSnapshot(class CStream &stm, float time_back=0, int flags=0);
	virtual int SetStateFromSnapshot(class CStream &stm, int flags=0);
	virtual int PostSetStateFromSnapshot();
	virtual unsigned int GetStateChecksum();
	int WriteContacts(CStream &stm,int flags);
	int ReadContacts(CStream &stm,int flags);

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual float GetLastTimeStep(float time_interval) { return m_lastTimeStep; }
	virtual int Step(float time_interval);
	virtual void StepBack(float time_interval);
	virtual int GetContactCount(int nMaxPlaneContacts);
	virtual int RegisterContacts(float time_interval,int nMaxPlaneContacts);
	virtual int Update(float time_interval, float damping);
	virtual float CalcEnergy(float time_interval);
	virtual float GetDamping(float time_interval);

	virtual void CheckAdditionalGeometry(float time_interval, masktype &contact_mask) {}
	virtual void AddAdditionalImpulses(float time_interval) {}
	virtual void RecomputeMassDistribution(int ipart=-1,int bMassChanged=1);

	virtual void DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags);
	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	int RegisterConstraint(const vectorf &pt0,const vectorf &pt1, int ipart0, CPhysicalEntity *pBuddy,int ipart1, int flags);
	int RemoveConstraint(int iConstraint);
	int RegisterContactPoint(masktype &contact_mask, int idx, const vectorf &pt, const geom_contact *pcontacts, int iPrim0,int iFeature0, 
		int iPrim1,int iFeature1, int flags=contact_new, float penetration=0);
	int CheckForNewContacts(geom_world_data *pgwd0,intersection_params *pip, int &itmax, int iStartPart=0,int nParts=-1);
	virtual int GetPotentialColliders(CPhysicalEntity **&pentlist);
	virtual int CheckSelfCollision(int ipart0,int ipart1) { return 0; }
	void UpdatePenaltyContacts(float time_interval);
	int UpdatePenaltyContact(int i, float time_interval);
	int VerifyExistingContacts(float maxdist);
	void UpdateConstraints();
	void UpdateContactsAfterStepBack(float time_interval);
	void ApplyBuoyancy(float time_interval,const vectorf &gravity);
	void ArchiveContact(int idx);
	int CompactContactBlock(masktype &contact_mask, float maxPlaneDist, int nMaxContacts,int &nContacts, vectorf &n, float &maxDist, 
		const vectorf& ptTest = vectorf(zero), const vectorf& dirTest = vectorf(zero));
	int IsFast(float time_interval);

	masktype *m_pColliderContacts,*m_pColliderConstraints;
	entity_contact *m_pContacts,*m_pConstraints;
	constraint_info *m_pConstraintInfos;
	int m_nContactsAlloc,m_nConstraintsAlloc;
	//int m_iStickyContacts[8],m_nStickyContacts;
	//int m_iSlidingContacts[8],m_nSlidingContacts;
	int m_bProhibitUnproj;
	int m_bProhibitUnprojection,m_bEnforceContacts;
	vectorf m_prevUnprojDir;
	int m_bCollisionCulling;
	int m_bJustLoaded;
	int m_bStable;
	int m_bHadSeverePenetration;
	unsigned int m_nRestMask;
	int m_nPrevColliders;
	int m_bSteppedBack,m_nStepBackCount;
	float m_velFastDir,m_sizeFastDir;
	int m_bCanSweep;

	float m_timeStepFull;
	float m_timeStepPerformed;
	float m_lastTimeStep;
	float m_minAwakeTime;

	vectorf m_gravity,m_gravityFreefall;
	float m_Emin;
	float m_maxAllowedStep;
	vectorf m_vhist[4],m_whist[4],m_Lhist[4];
	int m_iDynHist;
	int m_bAwake;
	int m_nSleepFrames;
	float m_damping,m_dampingFreefall;
	float m_dampingEx;
	float m_maxw;
	float m_softness[4];

	RigidBody m_body;
	vectorf m_Pext[2],m_Lext[2];
	vectorf m_prevPos,m_prevv,m_prevw;
	quaternionf m_prevq;
	float m_E0,m_Estep;
	float m_impulseTime;
	float m_timeContactOverflow;

	coll_history_item m_CollHistory[8];
	int m_iCollHistory;

	plane m_waterPlane;
	float m_waterDensity;
	float m_waterDamping;
	float m_waterResistance;
	vectorf m_waterFlow;
	float m_EminWater;
	int m_bFloating;
	float m_submergedFraction;
	float m_maxWaterResistance2;

	checksum_item m_checksums[NCHECKSUMS];
	int m_iLastChecksum;
};

extern CPhysicalEntity *g_CurColliders[128];
extern int g_CurCollParts[128][2];

#endif