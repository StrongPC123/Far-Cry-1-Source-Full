//////////////////////////////////////////////////////////////////////
//
//	Wheeled Vehicle Entity header
//	
//	File: wheeledvehicleentity.h
//	Description : CWheeledVehicleEntity class declaration
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef wheeledvehicleentity_h
#define wheeledvehicleentity_h
#pragma once

struct suspension_point {
	int bDriving; // if the corresponding wheel a driving wheel
	int iAxle;
	vectorf pt; // the uppermost suspension point in car frame
	float fullen; // unconstrained length
	float kStiffness; // stiffness coefficient
	float kDamping,kDamping0; // damping coefficient
	float len0; // initial length in model
	float Mpt; // hull "mass" at suspension upper point along suspension direction
	quaternionf q0;	// used to calculate geometry transformation from wheel transformation
	vectorf pos0,ptc0; // ...
	float Iinv;
	float minFriction,maxFriction;
	int flags0,flagsCollider0;
	int bCanBrake;
	int iBuddy;
	float r,rinv; // wheel radius, 1.0/radius
	float width;

	float curlen; // current length
	float steer; // steering angle
	float rot; // current wheel rotation angle
	float w; // current rotation speed
	float wa; // current angular acceleration
	float T; // wheel's net torque
	float prevTdt;
	float prevw;

	vectorf ncontact,ptcontact; // filled in RegisterPendingCollisions
	int bSlip,bSlipPull;
	int bContact;
	int surface_idx[2];
	vectorf vrel;
	vectorf rworld;
	float vworld;
	float PN;
	RigidBody *pbody;
	CPhysicalEntity *pent;
	int ipart;
};

class CWheeledVehicleEntity : public CRigidEntity, public IRigidBodyOwner {
 public:
	CWheeledVehicleEntity(CPhysicalWorld *pworld);
	virtual pe_type GetType() { return PE_WHEELEDVEHICLE; }

	virtual int SetParams(pe_params*);
	virtual int GetParams(pe_params*);
	virtual int Action(pe_action*);
	virtual int GetStatus(pe_status*);

	enum snapver { SNAPSHOT_VERSION = 1 };
	virtual int GetSnapshotVersion() { return SNAPSHOT_VERSION; }
	virtual int GetStateSnapshot(class CStream &stm, float time_back=0, int flags=0);
	virtual int SetStateFromSnapshot(class CStream &stm, int flags=0);

	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1);
	virtual void RemoveGeometry(int id);

	virtual float GetMaxTimeStep(float time_interval);
	virtual float GetDamping(float time_interval);
	virtual void CheckAdditionalGeometry(float time_interval, masktype &contact_mask);
	virtual int HasContactsWith(CPhysicalEntity *pent);
	virtual void AddAdditionalImpulses(float time_interval);
	virtual int Update(float time_interval, float damping);
	virtual void ComputeBBox();

	//virtual RigidBody *GetRigidBody(int ipart=-1) { return &m_bodyStatic; }
	virtual void AddImpulseAtContact(entity_contact *pcontact, int iop, const vectorf &dP) {};
	virtual vectorf GetVelocityAtContact(entity_contact *pcontact, int iop) { return vectorf(zero); };
	virtual int OnRegisterContact(entity_contact *pcontact, int iop);
	virtual void OnSolverEvent(int iEvent) {};

	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	void UpdateWheelsGeoms();
	void RecalcSuspStiffness();
	float ComputeDrivingTorque(float time_interval);

	suspension_point m_susp[8];
	float m_enginePower,m_maxSteer;
	float m_engineMaxw,m_engineMinw,m_engineIdlew,m_engineShiftUpw,m_engineShiftDownw,m_gearDirSwitchw,m_engineStartw;
	float m_axleFriction,m_brakeTorque,m_clutchSpeed,m_maxBrakingFriction,m_kDynFriction,m_slipThreshold;
	float m_kStabilizer;
	float m_enginePedal,m_steer,m_clutch,m_wengine;
	float m_gears[8];
	int m_nGears,m_iCurGear;
	int m_bHandBrake;
	int m_nHullParts;
	int m_iIntegrationType;
	float m_EminRigid,m_EminVehicle;
	float m_maxAllowedStepVehicle,m_maxAllowedStepRigid;
	float m_dampingVehicle;
	vectorf m_Ffriction,m_Tfriction;
	float m_timeNoContacts;
	int m_nContacts,m_bHasContacts;
};

#endif