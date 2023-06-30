//////////////////////////////////////////////////////////////////////
//
//	Rigid Body header
//	
//	File: rigidbody.cpp
//	Description : RigidBody class declaration
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef rigidbody_h
#define rigidbody_h
#pragma once

struct entity_contact;

const int MAX_CONTACTS = 4096;
enum solver_events { solver_initialize, solver_end_iter, solver_end };

class IRigidBodyOwner {
public:
	virtual void AddImpulseAtContact(entity_contact *pcontact, int iop, const vectorf &dP) = 0;
	virtual vectorf GetVelocityAtContact(entity_contact *pcontact, int iop) = 0;
	virtual int OnRegisterContact(entity_contact *pcontact, int iop) = 0;
	virtual void OnSolverEvent(int iEvent) = 0;
};

class RigidBody : public IRigidBodyOwner {
public:
	RigidBody();
	void Create(const vectorf &center,const vectorf &Ibody0,const quaternionf &q0, float volume,float mass, 
							const quaternionf &qframe,const vectorf &posframe);
	void Add(const vectorf &center,const vectorf Ibodyop,const quaternionf &qop, float volume,float mass);
	void zero();

	void Step(float dt);
	void UpdateState();
	void GetContactMatrix(const vectorf &r, matrix3x3f &K);

	virtual void AddImpulseAtContact(entity_contact *pcontact, int iop, const vectorf &dP);
	virtual vectorf GetVelocityAtContact(entity_contact *pcontact, int iop);
	virtual int OnRegisterContact(entity_contact *pcontact, int iop) { return 1; }
	virtual void OnSolverEvent(int iEvent) {}

	vectorf pos;
	quaternionf q;
	vectorf P,L;
	vectorf w,v;

	float M,Minv; // mass, 1.0/mass (0 for static objects)
	float V; // volume
	matrix3x3diagf Ibody; // diagonalized inertia tensor (aligned with body's axes of inertia)
	matrix3x3diagf Ibody_inv; // { 1/Ibody.ii }
	quaternionf qfb; // frame->body rotation
	vectorf offsfb; // frame->body offset

	matrix3x3f Iinv; // I^-1(t)

	vectorf Fcollision,Tcollision;
	int bProcessed; // used internally
	float Eunproj;
	float softness[2];

	IRigidBodyOwner *pOwner;
};

enum contactflags { contact_count_mask=0x3F, contact_new=0x40, contact_2b_verified=0x80, contact_2b_verified_log2=7, 
										contact_angular=0x100, contact_constraint_3dof=0x200, contact_constraint_2dof=0x400, 
										contact_constraint_1dof=0x800, contact_solve_for=0x1000,
										contact_constraint=contact_constraint_3dof|contact_constraint_2dof|contact_constraint_1dof,
										contact_angular_log2=8,contact_bidx=0x2000,contact_bidx_log2=13, contact_maintain_count=0x4000,
										contact_wheel=0x8000, contact_use_C=0x10000, contact_inexact=0x20000 };

class CPhysicalEntity;

struct entity_contact {
	vectorf pt[2];
	vectorf n;
	vectorf dir;
	vectorf ptloc[2];
	CPhysicalEntity *pent[2];
	int ipart[2];
	RigidBody *pbody[2];
	vectorf nloc;
	float friction;
	int id[2];
	int flags;
	vectorf vrel;
	vectorf vreq;
	//float vsep;
	float Pspare;
	float penetration;
	matrix3x3f K,Kinv;
	matrix3x3f C;
	int iNormal;
	int iPrim[2];
	int iFeature[2];
	int bProcessed;
	int iCount,*pBounceCount;

	vectorf r0,r;
	vectorf dP,P;
	float dPn;
};

extern bool g_bUsePreCG;
extern int g_nContacts,g_nBodies;
void InitContactSolver(float time_interval);
void RegisterContact(entity_contact *pcontact);
void InvokeContactSolver(float time_interval, SolverSettings *pss);
char *AllocSolverTmpBuf(int size);

#endif