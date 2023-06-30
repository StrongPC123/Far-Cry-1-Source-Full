//////////////////////////////////////////////////////////////////////
//
//	Rigid Entity
//	
//	File: rigidentity.cpp
//	Description : RigidEntity class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "bvtree.h"
#include "singleboxtree.h"
#include "boxgeom.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "geoman.h"
#include "physicalworld.h"
#include "rigidentity.h"


CRigidEntity::CRigidEntity(CPhysicalWorld *pWorld) : CPhysicalEntity(pWorld)
{
	m_iSimClass=2; 
	if (pWorld) 
		m_gravity = pWorld->m_vars.gravity;
	else m_gravity.Set(0,0,-9.81f);
	m_gravityFreefall = m_gravity;
	m_maxAllowedStep = 0.01f;
	m_Pext[0].zero(); m_Lext[0].zero();
	m_Pext[1].zero(); m_Lext[1].zero();
	int i;
	for(i=0;i<sizeof(m_CollHistory)/sizeof(m_CollHistory[0]);i++)
		m_CollHistory[i].age = 1E10;
	m_iCollHistory = 0;
	for(i=0;i<sizeof(m_vhist)/sizeof(m_vhist[0]);i++) {
		m_vhist[i].zero(); m_whist[i].zero(); m_Lhist[i].zero();
	}
	m_iDynHist = 0;
	m_timeStepPerformed = 0;
	m_timeStepFull = 0.01f;
	m_Emin = sqr(0.04f);
	m_pColliderContacts = 0;
	m_pColliderConstraints = 0;
	m_pContacts = 0;
	m_pConstraints = 0;
	m_pConstraintInfos = 0;
	m_nContactsAlloc = 0;
	m_nConstraintsAlloc = 0;
	m_bAwake = 1;
	m_nSleepFrames = 0;
	//m_nStickyContacts = m_nSlidingContacts = 0;
	m_prevUnprojDir.zero();
	m_bProhibitUnproj = 0;
	m_damping = 0.2f;
	m_dampingFreefall = 0.03f;
	m_dampingEx = 0;
	m_waterDensity = 0;
	m_waterPlane.n.Set(0,0,1);
	m_waterPlane.origin.Set(0,0,0);
	m_waterFlow.zero();
	m_waterResistance = 1000.0f;
	m_waterDamping = 0;
	m_EminWater = sqr(0.005f);
	m_bFloating = 0;
	m_bProhibitUnprojection = m_bEnforceContacts = -1;
	m_bJustLoaded = 0;
	m_bCollisionCulling = 0;
	m_maxw = 20.0f;
	m_maxWaterResistance2 = 0;
	m_iLastChecksum = 0;
	m_softness[0] = 0.00015f;
	m_softness[1] = 0.001f;
	m_bStable = 0;
	m_bHadSeverePenetration = 0;
	m_nRestMask = 0;
	m_nPrevColliders = 0;
	m_lastTimeStep = 0;
	m_nStepBackCount = m_bSteppedBack = 0;
	m_bCanSweep = 1;
	m_minAwakeTime = 0;
	//m_flags |= ref_use_simple_solver;
	m_E0 = m_Estep = 0;
	m_impulseTime = 0;
}

CRigidEntity::~CRigidEntity()
{
	if (m_pColliderContacts) delete[] m_pColliderContacts;
	if (m_pContacts) delete[] m_pContacts;
	if (m_pColliderConstraints) delete[] m_pColliderConstraints;
	if (m_pConstraints) delete[] m_pConstraints;
	if (m_pConstraintInfos) delete[] m_pConstraintInfos;
}


int CRigidEntity::AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id)
{
	if (pgeom && params->mass>0 && (pgeom->V<0 || pgeom->Ibody.x<0 || pgeom->Ibody.y<0 || pgeom->Ibody.z<0))	{
		char errmsg[128];
		sprintf(errmsg,"\002CRigidEntity::AddGeometry: (at %.1f,%.1f,%.1f) Trying to add bad geometry",m_pos.x,m_pos.y,m_pos.z);
		VALIDATOR_LOG(m_pWorld->m_pLog,errmsg);
		return -1;
	}

	int res;
	if ((res=CPhysicalEntity::AddGeometry(pgeom,params,id))<0)
		return res;

	float V=pgeom->V*cube(params->scale), M=params->mass>0 ? params->mass : params->density*V;
	vectorf bodypos = m_pos + m_qrot*(params->pos+params->q*pgeom->origin*params->scale); 
	quaternionf bodyq = m_qrot*params->q*pgeom->q;
	int i; for(i=0; i<m_nParts && m_parts[i].id!=res; i++);
	m_parts[i].mass = M;

	if (M>0) {
		if (m_body.M==0 || m_nParts==1)	{
			m_body.Create(bodypos,pgeom->Ibody*sqr(params->scale)*cube(params->scale),bodyq, V,M, m_qrot,m_pos);
			m_body.softness[0] = m_softness[0]; m_body.softness[1] = m_softness[1];
		} else
			m_body.Add(bodypos,pgeom->Ibody*sqr(params->scale)*cube(params->scale),bodyq, V,M);
	}
	
	return res;
}

void CRigidEntity::RemoveGeometry(int id)
{
	int i;
	for(i=0;i<m_nParts && m_parts[i].id!=id;i++);
	if (i==m_nParts) return;
	phys_geometry *pgeom = m_parts[i].pPhysGeomProxy;

	if (m_parts[i].mass>0) {
		vectorf bodypos = m_pos + m_qrot*(m_parts[i].pos+m_parts[i].q*pgeom->origin*m_parts[i].scale); 
		quaternionf bodyq = m_qrot*m_parts[i].q*pgeom->q;

		if (m_nParts>0)
			m_body.Add(bodypos,-pgeom->Ibody,bodyq,-pgeom->V,-m_parts[i].mass);
		else
			m_body.zero();
	}

	CPhysicalEntity::RemoveGeometry(id);
}

void CRigidEntity::RecomputeMassDistribution(int ipart,int bMassChanged)
{
	float V;
	vectorf bodypos;
	quaternionf bodyq;

	m_body.zero();
	for(int i=0; i<m_nParts; i++) {
		V = m_parts[i].pPhysGeom->V*cube(m_parts[i].scale);
		bodypos = m_pos + m_qrot*(m_parts[i].pos+m_parts[i].q*m_parts[i].pPhysGeom->origin*m_parts[i].scale); 
		bodyq = m_qrot*m_parts[i].q*m_parts[i].pPhysGeom->q;
		if (m_parts[i].mass>0) {
			if (i==0)	{
				m_body.Create(bodypos,m_parts[i].pPhysGeom->Ibody*cube(m_parts[i].scale)*sqr(m_parts[i].scale),bodyq, V,m_parts[i].mass, m_qrot,m_pos);
				m_body.softness[0] = m_softness[0]; m_body.softness[1] = m_softness[1];
			} else
				m_body.Add(bodypos,m_parts[i].pPhysGeom->Ibody*sqr(m_parts[i].scale)*cube(m_parts[i].scale),bodyq, V,m_parts[i].mass);
		}
	}
}


int CRigidEntity::SetParams(pe_params *_params)
{
	int res,i;
	float scale=m_parts[0].scale;
	
	if (res = CPhysicalEntity::SetParams(_params)) {
		if (_params->type==pe_params_pos::type_id) {
			pe_params_pos *params = (pe_params_pos*)_params;
			bool bPosChanged;
			if (!is_unused(params->pos) || params->pMtx4x4 || params->pMtx4x4T)	{
				m_body.pos = m_pos+m_qrot*m_body.offsfb; bPosChanged = true;
			}
			if (!is_unused(params->q) || params->pMtx3x3 || params->pMtx3x3T || params->pMtx4x4 || params->pMtx4x4T) {
				m_body.pos = m_pos+m_qrot*m_body.offsfb;
				m_body.q = m_qrot*!m_body.qfb; bPosChanged = true;
			}
			if (!is_unused(params->iSimClass))
				m_bAwake = isneg(1-m_iSimClass);
			if (!is_unused(params->scale) && fabsf(params->scale-scale)>0.001f && params->bRecalcBounds)
				RecomputeMassDistribution();
			if (m_body.Minv==0) {
				// for animated 'static' rigid bodies, awake the environment
				CPhysicalEntity **pentlist;
				int nEnts=m_pWorld->GetEntitiesAround(m_BBox[0],m_BBox[1],pentlist,ent_sleeping_rigid|ent_living|ent_independent|ent_triggers,this);
				for(--nEnts;nEnts>=0;nEnts--) if (pentlist[nEnts]!=this)
					pentlist[nEnts]->Awake();
			}
		} else if (_params->type==pe_params_part::type_id) {
			pe_params_part *params = (pe_params_part*)_params;
			if (params->bRecalcBBox)
				RecomputeMassDistribution(res-1,(is_unused(params->mass)&is_unused(params->density))^1);
		}
		return res;
	}

	if (_params->type==pe_simulation_params::type_id) {
		pe_simulation_params *params = (pe_simulation_params*)_params;
		bool bRecompute = false;
		if (!is_unused(params->gravity)) {
			m_gravity = params->gravity;
			if (is_unused(params->gravityFreefall)) m_gravityFreefall = params->gravity;
		}
		if (!is_unused(params->maxTimeStep)) m_maxAllowedStep = params->maxTimeStep;
		if (!is_unused(params->minEnergy)) m_Emin = params->minEnergy;
		if (!is_unused(params->damping)) m_damping = params->damping;
		if (!is_unused(params->gravityFreefall)) m_gravityFreefall = params->gravityFreefall;
		if (!is_unused(params->dampingFreefall)) m_dampingFreefall = params->dampingFreefall;
		if (!is_unused(params->density) && params->density>=0 && m_nParts>0) {
			for(i=0;i<m_nParts;i++) m_parts[i].mass = m_parts[i].pPhysGeom->V*cube(m_parts[i].scale)*params->density;
			bRecompute = true;
		}
		if (!is_unused(params->mass) && params->mass>=0 && m_nParts>0) {
			if (m_body.M==0) {
				float density,V=0;
				for(i=0;i<m_nParts;i++) V+=m_parts[i].pPhysGeom->V*cube(m_parts[i].scale);
				if (V>0) {
					density = params->mass/V;
					for(i=0;i<m_nParts;i++) m_parts[i].mass = m_parts[i].pPhysGeom->V*cube(m_parts[i].scale)*density;
				}
			} else {
				float scaleM = params->mass/m_body.M;
				for(i=0;i<m_nParts;i++) m_parts[i].mass *= scaleM;
			}
			bRecompute = true;
		}
		if (!is_unused(params->softness)) { m_softness[0]=m_body.softness[0]=params->softness; bRecompute=true; }
		if (!is_unused(params->softnessAngular)) { m_softness[1]=m_body.softness[1]=params->softnessAngular; bRecompute=true; }
		if (bRecompute)
			RecomputeMassDistribution();
		if (!is_unused(params->iSimClass))	{
			m_bAwake = isneg(1-(m_iSimClass = params->iSimClass));
			m_pWorld->RepositionEntity(this,2);
		}
		return 1;
	}

	if (_params->type==pe_params_buoyancy::type_id) {
		pe_params_buoyancy *params = (pe_params_buoyancy*)_params;
		int bAwake = 0;
		if (!is_unused(params->waterDensity)) { 
			if (m_waterDensity!=params->waterDensity)
				bAwake = 1;
			m_waterDensity = params->waterDensity;
		}
		if (!is_unused(params->waterDamping)) m_waterDamping = params->waterDamping;
		if (!is_unused(params->waterPlane.n)) {
			if ((m_waterPlane.n-params->waterPlane.n).len2()>0)
				bAwake = 1;
			m_waterPlane.n = params->waterPlane.n;
		}
		if (!is_unused(params->waterPlane.origin)) {
			vectorf center=(m_BBox[1]+m_BBox[0])*0.5f, sz=(m_BBox[1]-m_BBox[0])*0.5f;
			if ((m_waterPlane.origin-params->waterPlane.origin).len2()>0 && 
					center*m_waterPlane.n-min(params->waterPlane.origin*m_waterPlane.n,m_waterPlane.origin*m_waterPlane.n) > -fabsf(sz*m_waterPlane.n))
				bAwake = 1;	// but don't awake body if in both old and new cases if was fully under water
			m_waterPlane.origin = params->waterPlane.origin;
		}
		if (!is_unused(params->waterFlow)) {
			if ((m_waterFlow-params->waterFlow).len2()>0)
				bAwake = 1;
			m_waterFlow = params->waterFlow;
		}
		if (!is_unused(params->waterResistance)) m_waterResistance = params->waterResistance;
		if (!is_unused(params->waterEmin))
			m_EminWater = params->waterEmin;
		if (m_body.Minv>0 && bAwake && !m_bAwake && m_iSimClass<=2) {
			m_bAwake = 1;
			if (m_iSimClass==1) {
				m_iSimClass = 2; m_pWorld->RepositionEntity(this,2);
			}
		}
		return 1;
	}

	return res;
}

int CRigidEntity::GetParams(pe_params *_params)
{
	if (_params->type==pe_simulation_params::type_id) {
		pe_simulation_params *params = (pe_simulation_params*)_params;
		params->gravity = m_gravity;
		params->maxTimeStep = m_maxAllowedStep;
		params->minEnergy = m_Emin;
		params->damping = m_damping;
		params->dampingFreefall = m_dampingFreefall;
		params->gravityFreefall = m_gravityFreefall;
		params->iSimClass = m_iSimClass;
		params->density = m_body.M/m_body.V;
		params->mass = m_body.M;
		return 1;
	}

	if (_params->type==pe_params_buoyancy::type_id) {
		pe_params_buoyancy *params = (pe_params_buoyancy*)_params;
		params->waterDensity = m_waterDensity;
		params->waterDamping = m_waterDamping;
		params->waterPlane = m_waterPlane;
		params->waterFlow = m_waterFlow;
		params->waterResistance = m_waterResistance;
		params->waterEmin = m_EminWater;
		return 1;
	}

	return CPhysicalEntity::GetParams(_params);
}


int CRigidEntity::GetStatus(pe_status *_status)
{
	int res;
	if (res = CPhysicalEntity::GetStatus(_status)) {
		if (_status->type==pe_status_pos::type_id) {
			pe_status_pos *status = (pe_status_pos*)_status;
			if (status->timeBack>0) {
				status->q = m_prevq*m_body.qfb;
				status->pos = m_prevPos-status->q*m_body.offsfb;
			}
		}
		return res;
	}

	if (_status->type==pe_status_dynamics::type_id) {
		pe_status_dynamics *status = (pe_status_dynamics*)_status;
		status->v = m_body.v;
		status->w = m_body.w;
		status->a = m_gravity + m_body.Fcollision*m_body.Minv;
		status->wa = m_body.Iinv*(m_body.Tcollision - (m_body.w^m_body.L));
		status->centerOfMass = m_body.pos;
		status->submergedFraction = m_submergedFraction;
		status->waterResistance = sqrt_tpl(m_maxWaterResistance2);
		m_maxWaterResistance2 = 0;
		status->mass = m_body.M;
		return 1;
	}

	if (_status->type==pe_status_collisions::type_id) {
		pe_status_collisions *status = (pe_status_collisions*)_status;
		int i,n,nmax = min(status->len, sizeof(m_CollHistory)/sizeof(m_CollHistory[0]));
		for(i=m_iCollHistory,n=0; n<nmax && m_CollHistory[i].age <= status->age; i=i-1&sizeof(m_CollHistory)/sizeof(m_CollHistory[0])-1,n++)
			status->pHistory[n] = m_CollHistory[i];
		if (status->bClearHistory) for(i=0;i<sizeof(m_CollHistory)/sizeof(m_CollHistory[0]);i++)
			m_CollHistory[i].age = 1E10;
		return status->len = n;
	}

	if (_status->type==pe_status_sample_contact_area::type_id) {
		pe_status_sample_contact_area *status = (pe_status_sample_contact_area*)_status;
		vectorf sz = m_BBox[1]-m_BBox[0];
		float dist,tol = (sz.x+sz.y+sz.z)*0.1f;
		masktype contact_mask = 0;
		int i,nContacts;
		for(i=0; i<m_nColliders; i++)	contact_mask |= m_pColliderContacts[i];
		return CompactContactBlock(contact_mask,tol,0,nContacts, sz,dist, status->ptTest,status->dirTest);
	}

	return 0;
}


int CRigidEntity::Action(pe_action *_action)
{
	if (_action->type==pe_action_impulse::type_id) {
		pe_action_impulse *action = (pe_action_impulse*)_action;
		ENTITY_VALIDATE("CRigidEntity:Action(action_impulse)",action);
		if (m_flags&pef_monitor_impulses && action->iSource==0 && m_pWorld->m_pEventClient)
			if (!m_pWorld->m_pEventClient->OnImpulse(this,m_pForeignData,m_iForeignData, action))
				return 1;
		if (m_body.Minv==0)
			return 1;

		vectorf P=action->impulse, L(zero);
		if (!is_unused(action->momentum))
			L = action->momentum;
		else if (!is_unused(action->point))
			L = action->point-m_body.pos^action->impulse;

		if (action->iSource!=1) {
			m_bAwake = 1;
			if (m_iSimClass==1) {
				m_iSimClass = 2;	m_pWorld->RepositionEntity(this, 2);
			}
			m_impulseTime = 0.2f;
		}	else {
			float vres;
			if ((vres=P.len2()*sqr(m_body.Minv)) > sqr(5.0f))
				P *= 5.0f/sqrt_tpl(vres);
			if ((vres=(m_body.Iinv*L).len2()) > sqr(5.0f))
				L *= 5.0f/sqrt_tpl(vres);
		}

		if (action->iApplyTime==0) {
			m_body.P+=P;m_body.L+=L; m_body.v=m_body.P*m_body.Minv; m_body.w=m_body.Iinv*m_body.L;
		} else {
			m_Pext[action->iApplyTime-1]+=P; m_Lext[action->iApplyTime-1]+=L;
		}

		return 1;
	}

	if (_action->type==pe_action_reset::type_id) {
		m_body.v.zero(); m_body.w.zero(); m_body.P.zero(); m_body.L.zero();
		m_Pext[0].zero(); m_Lext[0].zero(); m_Pext[1].zero(); m_Lext[1].zero();
		for(int i=m_nColliders-1;i>=0;i--) if (!m_pColliderConstraints[i]) {
			CPhysicalEntity *pCollider = m_pColliders[i]; 
			pCollider->RemoveCollider(this); RemoveCollider(pCollider);
		}
		if (m_pWorld->m_vars.bMultiplayer) {
			m_qrot = CompressQuat(m_qrot);
			matrix3x3f R = matrix3x3f(m_body.q = m_qrot*!m_body.qfb);
			m_body.Iinv = R*m_body.Ibody_inv*R.T();
			ComputeBBox();
			m_pWorld->RepositionEntity(this,1);
		}
		//m_nStickyContacts = m_nSlidingContacts = 0;
		m_nRestMask = 0;
		return 1;
	}

	if (_action->type==pe_action_add_constraint::type_id) {
		pe_action_add_constraint *action = (pe_action_add_constraint*)_action;
		CPhysicalEntity *pBuddy = (CPhysicalEntity*)action->pBuddy;
		if (pBuddy==WORLD_ENTITY)
			pBuddy = &g_StaticPhysicalEntity;
		int i,res,ipart[2];
		vectorf nloc,pt1;
		quaternionf qframe[2];
		if (is_unused(action->pt[0]))
			return 0;
		pt1 = is_unused(action->pt[1]) ? action->pt[0] : action->pt[1];

		if (!is_unused(action->partid[0])) {
			for(ipart[0]=0;ipart[0]<m_nParts && m_parts[ipart[0]].id!=action->partid[0];ipart[0]++);
			if (ipart[0]>=m_nParts)
				return 0;
		} else
			ipart[0] = 0;
		if (!is_unused(action->partid[1])) {
			for(ipart[1]=0;ipart[1]<pBuddy->m_nParts && pBuddy->m_parts[ipart[1]].id!=action->partid[1];ipart[1]++);
			if (pBuddy->m_nParts>0 && ipart[1]>=pBuddy->m_nParts)
				return 0;
		} else
			ipart[1] = 0;

		res = i = RegisterConstraint(action->pt[0],pt1,ipart[0], pBuddy,ipart[1], contact_constraint_3dof);

		nloc = !m_pConstraints[i].pbody[0]->q*qframe[0]*vectorf(1,0,0);
		if (is_unused(qframe[0] = action->qframe[0])) qframe[0].SetIdentity();
		if (is_unused(qframe[1] = action->qframe[1])) qframe[1].SetIdentity();
		if (action->flags & local_frames) {
			qframe[0] = m_qrot*qframe[0];
			qframe[1] = pBuddy->m_qrot*qframe[1];
		}
		qframe[0] = !m_pConstraints[i].pbody[0]->q*qframe[0];
		qframe[1] = !m_pConstraints[i].pbody[1]->q*qframe[1];

		if (!is_unused(action->pConstraintEntity)) {
			m_pConstraintInfos[i].pConstraintEnt = (CPhysicalEntity*)action->pConstraintEntity;
			if (action->pConstraintEntity && action->pConstraintEntity->GetType()==PE_ROPE) {
				m_pConstraints[i].flags = 0; // act like frictionless contact when rope is strained
				m_pConstraintInfos[i].flags = constraint_rope;
			}
		}

		if (!is_unused(action->xlimits[0]) && action->xlimits[0]>=action->xlimits[1]) {
			i = RegisterConstraint(action->pt[0],pt1,ipart[0], pBuddy,ipart[1], contact_angular|contact_constraint_2dof);
			m_pConstraints[i].nloc=nloc; m_pConstraintInfos[i].qframe_rel[0]=qframe[0];	m_pConstraintInfos[i].qframe_rel[1]=qframe[1];
		} else if (!is_unused(action->yzlimits[0]) && action->yzlimits[0]>=action->yzlimits[1]) {
			i = RegisterConstraint(action->pt[0],pt1,ipart[0], pBuddy,ipart[1], contact_angular|contact_constraint_1dof);
			m_pConstraints[i].nloc=nloc; m_pConstraintInfos[i].qframe_rel[0]=qframe[0];	m_pConstraintInfos[i].qframe_rel[1]=qframe[1];
		}

		if (!is_unused(action->xlimits[0]) && action->xlimits[0]<action->xlimits[1]) {
			i = RegisterConstraint(action->pt[0],pt1,ipart[0], pBuddy,ipart[1], contact_angular);
			m_pConstraints[i].nloc=nloc; m_pConstraintInfos[i].qframe_rel[0]=qframe[0];	m_pConstraintInfos[i].qframe_rel[1]=qframe[1];
			m_pConstraintInfos[i].limits[0]=action->xlimits[0]; m_pConstraintInfos[i].limits[1]=action->xlimits[1];
			m_pConstraintInfos[i].flags = constraint_limited_1axis;
		}
		if (!is_unused(action->yzlimits[0]) && action->yzlimits[0]>=action->yzlimits[1]) {
			i = RegisterConstraint(action->pt[0],pt1,ipart[0], pBuddy,ipart[1], contact_angular);
			m_pConstraints[i].nloc=nloc; m_pConstraintInfos[i].qframe_rel[0]=qframe[0];	m_pConstraintInfos[i].qframe_rel[1]=qframe[1];
			m_pConstraintInfos[i].limits[0]=action->yzlimits[0]; m_pConstraintInfos[i].flags = constraint_limited_2axes;
		}

		return res+1;
	}

	if (_action->type==pe_action_remove_constraint::type_id) {
		pe_action_remove_constraint *action = (pe_action_remove_constraint*)_action;
		if (is_unused(action->idConstraint)) {
			for(int i=m_nColliders; i>=0 ;i--) if (m_pColliderConstraints[i]) {
				m_pColliderConstraints[i] = 0;
				if (!m_pColliderContacts[i] && !m_pColliders[i]->HasContactsWith(this)) {
					CPhysicalEntity *pCollider = m_pColliders[i]; 
					pCollider->RemoveCollider(this); RemoveCollider(pCollider);
				}
			}
			return 1;
		} else
			return RemoveConstraint(action->idConstraint-1);
	}

	if (_action->type==pe_action_set_velocity::type_id) {
		pe_action_set_velocity *action = (pe_action_set_velocity*)_action;
		int ipart = 0;
		if (!is_unused(action->ipart))
			ipart = action->ipart;
		else if (!is_unused(action->partid))
			for(;ipart<m_nParts && m_parts[ipart].id!=action->partid;ipart++);
		RigidBody *pbody = GetRigidBody(ipart);
		if (!is_unused(action->v)) {
			pbody->v = action->v;
			pbody->P = action->v*pbody->M;
		}
		if (!is_unused(action->w)) {
			pbody->w = action->w;
			pbody->L = pbody->q*(pbody->Ibody*(!pbody->q*pbody->w));
		}
		if (pbody->v.len2()+pbody->w.len2()>0) {
			if (!m_bAwake)
				Awake();
		} else if (m_body.Minv==0)
			Awake(0);

		return 1;
	}

	return CPhysicalEntity::Action(_action);
}


int CRigidEntity::RemoveCollider(CPhysicalEntity *pCollider, bool bRemoveAlways)
{
	int i; 
	if (!bRemoveAlways) {
		for(i=0;i<m_nColliders && m_pColliders[i]!=pCollider; i++);
		if (i<m_nColliders && m_pColliderContacts[i] | m_pColliderConstraints[i])
			return i;
	}
	i = CPhysicalEntity::RemoveCollider(pCollider,bRemoveAlways);
	if (i>=0) 
		for(;i<m_nColliders;i++) {
			m_pColliderContacts[i] = m_pColliderContacts[i+1];
			m_pColliderConstraints[i] = m_pColliderConstraints[i+1];
		}
	return i;
}


int CRigidEntity::AddCollider(CPhysicalEntity *pCollider)
{
	int nColliders=m_nColliders, nCollidersAlloc=m_nCollidersAlloc, i=CPhysicalEntity::AddCollider(pCollider);

	if (m_nCollidersAlloc>nCollidersAlloc) {
		masktype *pColliderContacts = m_pColliderContacts;
		memcpy(m_pColliderContacts = new masktype[m_nCollidersAlloc], pColliderContacts, nColliders*sizeof(masktype));
		if (pColliderContacts) delete[] pColliderContacts;
		masktype *pColliderConstraints = m_pColliderConstraints;
		memcpy(m_pColliderConstraints = new masktype[m_nCollidersAlloc], pColliderConstraints, nColliders*sizeof(masktype));
		if (pColliderConstraints) delete[] pColliderConstraints;
	}

	if (m_nColliders>nColliders) {
		for(int j=nColliders-1;j>=i;j--) {
			m_pColliderContacts[j+1] = m_pColliderContacts[j];
			m_pColliderConstraints[j+1] = m_pColliderConstraints[j];
		}
		m_pColliderContacts[i] = 0;
		m_pColliderConstraints[i] = 0;
	}
	m_bJustLoaded = 0;

	return i;
}


int CRigidEntity::HasContactsWith(CPhysicalEntity *pent)
{
	int i; for(i=0;i<m_nColliders && m_pColliders[i]!=pent;i++);
	return i==m_nColliders ? 0 : iszero((int)(m_pColliderContacts[i] | m_pColliderConstraints[i]))^1;
}

int CRigidEntity::HasCollisionContactsWith(CPhysicalEntity *pent)
{
	int i; for(i=0;i<m_nColliders && m_pColliders[i]!=pent;i++);
	return i==m_nColliders ? 0 : iszero((int)m_pColliderContacts[i])^1;
}


int CRigidEntity::Awake(int bAwake,int iSource)
{
	if ((unsigned int)m_iSimClass>6u) {
		m_pWorld->m_pLog->Log("\002Error: trying to awake deleted rigid entity");
		return -1;
	}
	int i;
	if (m_iSimClass<=2) {
		for(i=m_nColliders-1; i>=0; i--) if (m_pColliders[i]->m_iSimClass==7)
			RemoveCollider(m_pColliders[i]);
		if (m_iSimClass!=bAwake+1 && (m_body.Minv>0 || m_body.v.len2()+m_body.w.len2()>0 || bAwake==0 || iSource==1)) {
			m_nSleepFrames = 0;	m_bAwake = bAwake;
			m_iSimClass = m_bAwake+1; m_pWorld->RepositionEntity(this,2);
		}
		if (m_body.Minv==0) for(i=0;i<m_nColliders;i++)	if (m_pColliders[i]!=this && m_pColliders[i]->GetMassInv()>0)
			m_pColliders[i]->Awake();
	}	else
		m_bAwake = bAwake;
	return m_iSimClass;
}


void CRigidEntity::AlertNeighbourhoodND() 
{ 
	int i,iSimClass=m_iSimClass;
	masktype constraint_mask;
	m_iSimClass = 7; // notifies the others that we are being deleted

	for(i=0,constraint_mask=0; i<m_nColliders; constraint_mask|=m_pColliderConstraints[i++]);
	for(i=0;i<NMASKBITS && getmask(i)<=constraint_mask;i++) 
		if (constraint_mask & getmask(i) && m_pConstraintInfos[i].pConstraintEnt && (unsigned int)m_pConstraintInfos[i].pConstraintEnt->m_iSimClass<7u) 
			m_pConstraintInfos[i].pConstraintEnt->Awake();
	m_iSimClass = iSimClass;

	CPhysicalEntity::AlertNeighbourhoodND(); 
}


CPhysicalEntity *g_CurColliders[128];
int g_CurCollParts[128][2];

int CRigidEntity::GetPotentialColliders(CPhysicalEntity **&pentlist)
{
	int i,j,nents,bSameGroup;
	if (m_body.Minv<=0)
		return 0;
	vectorf BBox[2]={m_BBox[0],m_BBox[1]}, move=m_body.v*m_timeStepFull;
	BBox[isneg(-move.x)].x += move.x;	BBox[isneg(-move.y)].y += move.y;	BBox[isneg(-move.z)].z += move.z;
	nents = m_pWorld->GetEntitiesAround(BBox[0],BBox[1], pentlist, 
		ent_terrain|ent_static|ent_sleeping_rigid|ent_rigid|ent_living|ent_independent|ent_sort_by_mass|ent_triggers, this);

	if (m_flags & ref_use_simple_solver) for(i=0;i<m_nColliders;i++)
		m_pColliders[i]->m_bProcessed =	m_pWorld->m_vars.bSkipRedundantColldet &
			iszero(m_pColliders[i]->m_nParts*2+m_nParts-3) &
			iszero(((CGeometry*)m_pColliders[i]->m_parts[0].pPhysGeomProxy->pGeom)->m_bIsConvex+((CGeometry*)m_parts[0].pPhysGeomProxy->pGeom)->m_bIsConvex-2) &
			isneg(3-g_bitcount[m_pColliderContacts[i]&0xFF]-g_bitcount[m_pColliderContacts[i]>>8&0xFF]-
						g_bitcount[m_pColliderContacts[i]>>16&0xFF]-g_bitcount[m_pColliderContacts[i]>>24&0xFF]);

	for(i=j=0;i<nents;i++) if (!pentlist[i]->m_bProcessed) {
		if (pentlist[i]->m_iSimClass>2) {
			if (pentlist[i]->GetType()!=PE_ARTICULATED) {
				pentlist[i]->Awake();
				if (!m_pWorld->m_vars.bMultiplayer || pentlist[i]->m_iSimClass!=3)
					m_pWorld->ScheduleForStep(pentlist[i]);
			}
		} else if (((pentlist[i]->m_BBox[1]-pentlist[i]->m_BBox[0]).len2()==0 || AABB_overlap(m_BBox,pentlist[i]->m_BBox)) && 
			pentlist[i]!=this && ((bSameGroup=iszero(pentlist[i]->m_iGroup-m_iGroup)) & pentlist[i]->m_bMoved || 
			pentlist[i]->m_iGroup==-1 || 
			//!pentlist[i]->IsAwake() || 
			!(pentlist[i]->IsAwake() | (m_flags&ref_use_simple_solver | pentlist[i]->m_flags&ref_use_simple_solver)&-bSameGroup) ||
			m_pWorld->m_pGroupNums[pentlist[i]->m_iGroup]<m_pWorld->m_pGroupNums[m_iGroup]))
			pentlist[j++] = pentlist[i];
	}

	if (m_flags & ref_use_simple_solver) for(i=0;i<m_nColliders;i++)
		m_pColliders[i]->m_bProcessed = 0;

	return j;
}

int CRigidEntity::CheckForNewContacts(geom_world_data *pgwd0,intersection_params *pip, int &itmax, int iStartPart,int nParts)
{
	CPhysicalEntity **pentlist;
	geom_world_data gwd1;
	int ient,nents,i,j,icont,ncontacts,ipt,imask,nTotContacts=0,nAreas=0,nAreaPt=0;
	RigidBody *pbody;
	geom_contact *pcontacts;
	float tsg=!pip->bSweepTest ? 1.0f:-1.0f, tmax=-tsg;
	bool bStopAtFirstTri = pip->bStopAtFirstTri, bCheckBBox;
	vectorf sz,BBox[2],prevOutsidePivot=pip->ptOutsidePivot[0];
	int bPivotFilled = isneg(pip->ptOutsidePivot[0].x-1E9f);
	box bbox;
	itmax = -1;
	nParts = m_nParts&nParts>>31 | max(nParts,0);

	nents = GetPotentialColliders(pentlist);
	pip->bKeepPrevContacts = false;

	for(i=iStartPart; i<iStartPart+nParts; i++) if (m_parts[i].flagsCollider) {
		pgwd0->offset = m_pos + m_qrot*m_parts[i].pos;
		//(m_qrot*m_parts[i].q).getmatrix(pgwd0->R); //Q2M_IVO
		pgwd0->R = matrix3x3f(m_qrot*m_parts[i].q);
		pgwd0->scale = m_parts[i].scale;

		m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&bbox);
		bbox.Basis *= pgwd0->R.T();
		sz = (bbox.size*bbox.Basis.Fabs())*m_parts[i].scale;
		BBox[0] = BBox[1] = m_pos+m_qrot*(m_parts[i].pos+m_parts[i].q*bbox.center*m_parts[i].scale);
		BBox[0] -= sz; BBox[1] += sz;

		for(ient=0; ient<nents; ient++) 
		for(j=0,bCheckBBox=(pentlist[ient]->m_BBox[1]-pentlist[ient]->m_BBox[0]).len2()>0; j<pentlist[ient]->m_nParts; j++) 
		if ((pentlist[ient]->m_parts[j].flags & m_parts[i].flagsCollider) && !(pentlist[ient]==this && !CheckSelfCollision(i,j)) &&
				(m_nParts+pentlist[ient]->m_nParts==2 || 
				 (IsAwake(i) || pentlist[ient]->IsAwake(j)) && (!bCheckBBox || AABB_overlap(BBox,pentlist[ient]->m_parts[j].BBox))))
		{
			gwd1.offset = pentlist[ient]->m_pos + pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].pos;
			//(pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].q).getmatrix(gwd1.R); //Q2M_IVO
			gwd1.R = matrix3x3f(pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].q);
			gwd1.scale = pentlist[ient]->m_parts[j].scale;
			pbody = pentlist[ient]->GetRigidBody(j);
			gwd1.v = pbody->v;
			gwd1.w = pbody->w;
			gwd1.centerOfMass = pbody->pos;
			pip->ptOutsidePivot[0] = m_parts[i].maxdim<pentlist[ient]->m_parts[j].maxdim ?
				prevOutsidePivot : vectorf(1E11f,1E11f,1E11f);
			/*if (!bPivotFilled && (m_bCollisionCulling || m_parts[i].pPhysGeomProxy->pGeom->IsConvex(0.1f))) {
				m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&bbox);
				pip->ptOutsidePivot[0] = pgwd0->R*(bbox.center*pgwd0->scale)+pgwd0->offset;
			}*/
			//pip->bStopAtFirstTri = pentlist[ient]==this || bStopAtFirstTri || 
			//	m_parts[i].pPhysGeomProxy->pGeom->IsConvex(0.02f) && pentlist[ient]->m_parts[j].pPhysGeomProxy->pGeom->IsConvex(0.02f);

			ncontacts = m_parts[i].pPhysGeomProxy->pGeom->Intersect(pentlist[ient]->m_parts[j].pPhysGeomProxy->pGeom, pgwd0,&gwd1, pip, pcontacts);
			for(icont=0; icont<ncontacts; icont++) {
				pcontacts[icont].id[0] = m_parts[i].surface_idx&pcontacts[icont].id[0]>>31 | max(pcontacts[icont].id[0],0);
				pcontacts[icont].id[1] = pentlist[ient]->m_parts[j].surface_idx&pcontacts[icont].id[1]>>31 | max(pcontacts[icont].id[1],0);
				g_CurColliders[nTotContacts] = pentlist[ient];
				g_CurCollParts[nTotContacts][0] = i;
				g_CurCollParts[nTotContacts][1] = j;
				if (pcontacts[icont].parea) for(ipt=0;ipt<pcontacts[icont].parea->npt;ipt++) {
					imask = pcontacts[icont].parea->piPrim[0][ipt]>>31;
					(pcontacts[icont].parea->piPrim[0][ipt] &= ~imask) |= pcontacts[icont].iPrim[0] & imask;
					(pcontacts[icont].parea->piFeature[0][ipt] &= ~imask) |= (pcontacts[icont].iFeature[0] & imask) | imask&1<<31;
					imask = pcontacts[icont].parea->piPrim[1][ipt]>>31;
					(pcontacts[icont].parea->piPrim[1][ipt] &= ~imask) |= pcontacts[icont].iPrim[1] & imask;
					(pcontacts[icont].parea->piFeature[1][ipt] &= ~imask) |= (pcontacts[icont].iFeature[1] & imask) | imask&1<<31;
				}
				if (pcontacts[icont].vel>0 && pcontacts[icont].t*tsg>tmax*tsg) {
					tmax = pcontacts[icont].t; itmax = nTotContacts;
				}
				if (++nTotContacts==sizeof(g_CurColliders)/sizeof(g_CurColliders[0]))
					goto CollidersNoMore;
			}
			pip->bKeepPrevContacts = true;
		}
	}
	CollidersNoMore:
	pip->bStopAtFirstTri = bStopAtFirstTri;
	pip->ptOutsidePivot[0] = prevOutsidePivot;

	return nTotContacts;
}


int CRigidEntity::RemoveContactPoint(CPhysicalEntity *pCollider, const vectorf &pt, float mindist2)
{
	int i,j;
	for(i=0;i<m_nColliders && m_pColliders[i]!=pCollider;i++);
	if (i<m_nColliders) {
		for(j=0; j<NMASKBITS && getmask(j)<=m_pColliderContacts[i] && !(m_pColliderContacts[i]&getmask(j) && 
				isneg((m_pContacts[j].pt[0]-pt).len2()-mindist2)); j++);
		if (m_pColliderContacts[i] & getmask(j)) {
			/*for(k=0; k<m_nStickyContacts && m_iStickyContacts[k]!=j; k++);
			for(m_nStickyContacts+=k-m_nStickyContacts>>31; k<m_nStickyContacts; k++) m_iStickyContacts[k] = m_iStickyContacts[k+1];
			for(k=0; k<m_nSlidingContacts && m_iSlidingContacts[k]!=j; k++);
			for(m_nSlidingContacts+=k-m_nSlidingContacts>>31; k<m_nSlidingContacts; k++) m_iSlidingContacts[k] = m_iSlidingContacts[k+1];*/
			if (!(m_flags & ref_use_simple_solver))
				if (!((m_pColliderContacts[i] &= ~getmask(j)) | m_pColliderConstraints[i]) && !pCollider->HasContactsWith(this)) {
					RemoveCollider(pCollider); pCollider->RemoveCollider(this);
				}
			return j;
		}
	}
	return -1;
}


int CRigidEntity::RegisterContactPoint(masktype &contact_mask, int idx, const vectorf &pt, const geom_contact *pcontacts, int iPrim0,int iFeature0, 
																			 int iPrim1,int iFeature1, int flags, float penetration)
{
	if (!(m_pWorld->m_vars.bUseDistanceContacts | m_flags&ref_use_simple_solver) && penetration==0 && GetType()!=PE_ARTICULATED)
		return -1;
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	float min_dist2 = sqr(min(m_parts[g_CurCollParts[idx][0]].minContactDist,
		g_CurColliders[idx]->m_parts[g_CurCollParts[idx][1]].minContactDist));// * (penetration>0 ? 3.0f:1.0f));
	int i,j,bUseSimpleSolver=iszero((int)m_flags&ref_use_simple_solver)^1;
	RigidBody *pbody1 = g_CurColliders[idx]->GetRigidBody(g_CurCollParts[idx][1]);
	if (bUseSimpleSolver) {
		if (g_CurColliders[idx]->RemoveContactPoint(this,pt,min_dist2)>=0)
			return -1;
	}	else if (m_pWorld->m_vars.bUseDistanceContacts)
		g_CurColliders[idx]->RemoveContactPoint(this,pt,min_dist2);

	for(i=0;i<NMASKBITS && getmask(i)<=contact_mask && 
			!(contact_mask&getmask(i) && (m_pContacts[i].flags&contact_new || m_pContacts[i].penetration==0) && 
			m_pContacts[i].pent[1]==g_CurColliders[idx] && m_pContacts[i].pbody[1]==pbody1 &&
			((m_pContacts[i].iPrim[0]-iPrim0|m_pContacts[i].iFeature[0]-iFeature0|
			 m_pContacts[i].iPrim[1]-iPrim1|m_pContacts[i].iFeature[1]-iFeature1|bUseSimpleSolver^1)==0 ||
			((m_pContacts[i].flags&contact_new) ? 
				(m_pContacts[i].pt[0]-pt).len2() : 
			  (m_pContacts[i].pbody[0]->q*m_pContacts[i].ptloc[0]+m_pContacts[i].pbody[0]->pos-pt).len2()) < min_dist2));
			i++);
	if (i==NMASKBITS || getmask(i)>contact_mask) { // no existing point that is close enough to this one
		for(i=0;i<NMASKBITS && contact_mask&getmask(i) && (m_pContacts[i].flags&contact_new || m_pContacts[i].penetration==0);i++);
		if (i==NMASKBITS)	{
			m_timeContactOverflow = 0.5f; return -1;
		}
		if (i>=m_nContactsAlloc) {
			entity_contact *pcontacts = m_pContacts;
			int ncontacts = m_nContactsAlloc;
			memcpy(m_pContacts = new entity_contact[(m_nContactsAlloc=(i&~7)+8)], pcontacts, ncontacts*sizeof(entity_contact));
			if (pcontacts) delete[] pcontacts;
		}
		for(j=m_nColliders-1;j>=0;j--) // detach contact slot we are going to use from all other colliders
		if (!((m_pColliderContacts[j]&=~getmask(i)) | m_pColliderConstraints[j]) && !m_pColliders[j]->HasContactsWith(this)) {
			CPhysicalEntity *pCollider = m_pColliders[j]; 
			pCollider->RemoveCollider(this); RemoveCollider(pCollider);
		}
		m_pColliderContacts[AddCollider(g_CurColliders[idx])] |= getmask(i);
		g_CurColliders[idx]->AddCollider(this);
		contact_mask |= getmask(i);
	} else if (bUseSimpleSolver || 
		(m_pContacts[i].penetration==0 || m_pContacts[i].flags&contact_new) && 
		(m_pContacts[i].penetration>=penetration ||	flags & contact_2b_verified))
		return -1;
		//(!m_pWorld->m_vars.bUseDistanceContacts ? 	
		//(m_pContacts[i].penetration==0 || m_pContacts[i].flags&contact_new) :
		//(flags & contact_2b_verified && m_pContacts[i].penetration==0))

	m_pContacts[i].pt[0] = m_pContacts[i].pt[1] = pt;
	m_pContacts[i].n = -pcontacts[idx].n;
	m_pContacts[i].dir = pcontacts[idx].dir;
	m_pContacts[i].pent[0] = this;
	m_pContacts[i].pent[1] = g_CurColliders[idx];
	m_pContacts[i].ipart[0] = g_CurCollParts[idx][0];
	m_pContacts[i].ipart[1] = g_CurCollParts[idx][1];
	m_pContacts[i].pbody[0] = GetRigidBody(g_CurCollParts[idx][0]);
	m_pContacts[i].pbody[1] = pbody1;
	m_pContacts[i].iPrim[0] = iPrim0; m_pContacts[i].iFeature[0] = iFeature0;
	m_pContacts[i].iPrim[1] = iPrim1; m_pContacts[i].iFeature[1] = iFeature1;
	m_pContacts[i].penetration = penetration;

	m_pContacts[i].vrel = m_pContacts[i].pbody[0]->v+(m_pContacts[i].pbody[0]->w^m_pContacts[i].pt[0]-m_pContacts[i].pbody[0]->pos) - 
		m_pContacts[i].pbody[1]->v-(m_pContacts[i].pbody[1]->w^m_pContacts[i].pt[0]-m_pContacts[i].pbody[1]->pos);
	m_pContacts[i].id[0] = min(max(0,pcontacts[idx].id[0]),m_pWorld->m_vars.nMaxSurfaces-1);
	m_pContacts[i].id[1] = min(max(0,pcontacts[idx].id[1]),m_pWorld->m_vars.nMaxSurfaces-1);
	m_pContacts[i].friction = 0.5f*max(0.0f, m_pContacts[i].vrel.len2()<sqr(m_pWorld->m_vars.maxContactGap*5) ? 
		m_pWorld->m_FrictionTable[m_pContacts[i].id[0]&NSURFACETYPES-1] + m_pWorld->m_FrictionTable[m_pContacts[i].id[1]&NSURFACETYPES-1] :
		m_pWorld->m_DynFrictionTable[m_pContacts[i].id[0]&NSURFACETYPES-1] + m_pWorld->m_DynFrictionTable[m_pContacts[i].id[1]&NSURFACETYPES-1]);
	m_pContacts[i].flags = flags;

	if (bUseSimpleSolver) { 
		vectorf ptloc[2],unproj=pcontacts[idx].dir*pcontacts[idx].t;
		m_pContacts[i].iNormal = isneg((m_pContacts[i].iFeature[0]&0xE0)-(m_pContacts[i].iFeature[1]&0xE0));
		m_pContacts[i].nloc = m_pContacts[i].n*m_pContacts[i].pbody[m_pContacts[i].iNormal]->q;
		ptloc[0] = (pt-m_pContacts[i].pbody[0]->pos-unproj)*m_pContacts[i].pbody[0]->q;
		ptloc[1] = (pt-m_pContacts[i].pbody[1]->pos)*m_pContacts[i].pbody[1]->q;
		if (!(flags & contact_inexact)) {
			m_pContacts[i].ptloc[0]=ptloc[0]; m_pContacts[i].ptloc[1]=ptloc[1];
		}	else {
			vectorf ptfeat[4];
			if (!(m_pContacts[i].iFeature[0]&m_pContacts[i].iFeature[1]&0xE0)) {
				j = m_pContacts[i].iNormal;
				geom *ppart = m_pContacts[i].pent[j]->m_parts+m_pContacts[i].ipart[j];
				// get geometry-CS coordinates of features in ptloc
				ppart->pPhysGeomProxy->pGeom->GetFeature(m_pContacts[i].iPrim[j],m_pContacts[i].iFeature[j], ptfeat); 
				// store world-CS coords in m_pContacts[i].ptloc
				m_pContacts[i].ptloc[j] = m_pContacts[i].pent[j]->m_qrot*(ppart->q*ptfeat[0]*ppart->scale+ppart->pos)+m_pContacts[i].pent[j]->m_pos; 
				vectorf nfeat = m_pContacts[i].pent[j]->m_qrot*ppart->q*(ptfeat[1]-ptfeat[0] ^ ptfeat[2]-ptfeat[0]);
				ppart = m_pContacts[i].pent[j^1]->m_parts+m_pContacts[i].ipart[j^1];
				ppart->pPhysGeomProxy->pGeom->GetFeature(m_pContacts[i].iPrim[j^1],m_pContacts[i].iFeature[j^1], ptfeat); 
				m_pContacts[i].ptloc[j^1] = m_pContacts[i].pent[j^1]->m_qrot*(ppart->q*ptfeat[0]*ppart->scale+ppart->pos)+m_pContacts[i].pent[j^1]->m_pos; 
				m_pContacts[i].ptloc[0] += unproj;
				m_pContacts[i].ptloc[j] = pt-nfeat*((nfeat*(pt-m_pContacts[i].ptloc[j]))/nfeat.len2());
				m_pContacts[i].ptloc[0] = (m_pContacts[i].ptloc[0]-m_pContacts[i].pbody[0]->pos-unproj)*m_pContacts[i].pbody[0]->q;
				m_pContacts[i].ptloc[1] = (m_pContacts[i].ptloc[1]-m_pContacts[i].pbody[1]->pos)*m_pContacts[i].pbody[1]->q;
			}	else {
				for(j=0;j<2;j++) {
					geom *ppart = m_pContacts[i].pent[j]->m_parts+m_pContacts[i].ipart[j];
					float rscale = ppart->scale==1.0f ? 1.0f : 1.0f/ppart->scale;
					// get edge in geom CS to ptloc[1]-ptloc[2]
					ppart->pPhysGeomProxy->pGeom->GetFeature(m_pContacts[i].iPrim[j],m_pContacts[i].iFeature[j], ptfeat+1);	
					ptfeat[0] = (((pt-m_pContacts[i].pent[j]->m_pos-unproj*(j^1))*m_pContacts[i].pent[j]->m_qrot-	// ptloc[0] <- contact point in geom CS
						ppart->pos)*rscale)*ppart->q;
					ptfeat[0] = ptfeat[1] + (ptfeat[2]-ptfeat[1])*(((ptfeat[0]-ptfeat[1])*(ptfeat[2]-ptfeat[1]))/(ptfeat[2]-ptfeat[1]).len2());
					// transform geom CS->world CS
					ptfeat[0] = m_pContacts[i].pent[j]->m_qrot*(ppart->q*ptfeat[0]*ppart->scale+ppart->pos)+m_pContacts[i].pent[j]->m_pos; 
					m_pContacts[i].ptloc[j] = (ptfeat[0]-m_pContacts[i].pbody[j]->pos)*m_pContacts[i].pbody[j]->q;// world CS->body CS
				}
			}
			for(j=0;j<2 && (m_pContacts[i].ptloc[j]-ptloc[j]).len2()<sqr(m_pWorld->m_vars.maxContactGapSimple);j++);
			if (j<2)
				m_pContacts[i].ptloc[0]=ptloc[0], m_pContacts[i].ptloc[1]=ptloc[1];
		}
		m_pContacts[i].vreq.zero();
		m_pContacts[i].r.zero(); m_pContacts[i].dP.zero();
		UpdatePenaltyContact(i,m_lastTimeStep);
	} else {
		m_pContacts[i].Pspare = 0;
		m_pContacts[i].K.SetZero();
		GetContactMatrix(m_pContacts[i].pt[0], m_pContacts[i].ipart[0], m_pContacts[i].K);
		g_CurColliders[idx]->GetContactMatrix(m_pContacts[i].pt[1], m_pContacts[i].ipart[1], m_pContacts[i].K);

		float e = (m_pWorld->m_BouncinessTable[pcontacts[idx].id[0]&NSURFACETYPES-1] + 
							m_pWorld->m_BouncinessTable[pcontacts[idx].id[1]&NSURFACETYPES-1])*0.5f;
		if (//m_body.M<2 &&	// bounce only smalll objects
				//m_nParts+g_CurColliders[idx]->m_nParts<=4 && // bounce only simple objects (bouncing is dangerous)
				!bUseSimpleSolver && 
				e>0 && m_pContacts[i].vrel*m_pContacts[i].n<-m_pWorld->m_vars.minBounceSpeed) 
		{ // apply bounce impulse if needed
			pe_action_impulse ai;
			ai.impulse = m_pContacts[i].n*((m_pContacts[i].vrel*m_pContacts[i].n)*-(1+e)/(m_pContacts[i].n*m_pContacts[i].K*m_pContacts[i].n));
			ai.point = m_pContacts[i].pt[0];
			ai.partid = m_parts[m_pContacts[i].ipart[0]].id;
			ai.iApplyTime = 0; ai.iSource = 3;
			Action(&ai);
			ai.impulse.Flip(); ai.partid = g_CurColliders[idx]->m_parts[m_pContacts[i].ipart[1]].id;
			g_CurColliders[idx]->Action(&ai);
		}

		if (penetration>0) {
			vectorf sz = (m_BBox[1]-m_BBox[0]);
			if (penetration>(sz.x+sz.y+sz.z)*0.06f)
				m_pContacts[i].n = pcontacts[idx].dir;
			m_pContacts[i].vreq = penetration>m_pWorld->m_vars.maxContactGap ? 
				m_pContacts[i].n*min(m_pWorld->m_vars.maxUnprojVel,
					(penetration-m_pWorld->m_vars.maxContactGap/*0.5f*/)*m_pWorld->m_vars.unprojVelScale) : vectorf(zero);
			//m_pContacts[i].vsep = penetration>m_pWorld->m_vars.maxContactGap*10 ? penetration*3.0f : 0;
		} else {
			m_pContacts[i].ptloc[0] = (pt-m_pContacts[i].pbody[0]->pos)*m_pContacts[i].pbody[0]->q;
			m_pContacts[i].ptloc[1] = (pt-m_pContacts[i].pbody[1]->pos)*m_pContacts[i].pbody[1]->q;
			m_pContacts[i].iNormal = isneg((m_pContacts[i].iFeature[0]&0xE0)-(m_pContacts[i].iFeature[1]&0xE0));
			m_pContacts[i].nloc = m_pContacts[i].n*m_pContacts[i].pbody[m_pContacts[i].iNormal]->q;
			m_pContacts[i].vreq.zero();
			//m_pContacts[i].vsep = 0;//m_pContacts[i].friction>0.8f ? m_pWorld->m_vars.minSeparationSpeed*0.1f : 0;
		}
	}

	return i;
}


void CRigidEntity::UpdatePenaltyContacts(float time_interval)
{
	int i,nContacts;//,bResolveInstantly;
	masktype contact_mask=0;
	//entity_contact *pContacts[16];

	for(i=0;i<m_nColliders;i++) contact_mask |= m_pColliderContacts[i];
	nContacts = g_bitcount[contact_mask&0xFF]+g_bitcount[contact_mask>>8&0xFF]+g_bitcount[contact_mask>>16&0xFF]+g_bitcount[contact_mask>>24&0xFF];
	//bResolveInstantly = /isneg((int)(sizeof(pContacts)/sizeof(pContacts[0]))-nContacts);
	m_bStable = max(m_bStable, isneg(2-nContacts)*2);
	for(i=nContacts=0;i<NMASKBITS && getmask(i)<=contact_mask;i++) if (contact_mask & getmask(i))	{
		UpdatePenaltyContact(i,time_interval);//, bResolveInstantly,pContacts,nContacts);
		if (m_bStable && m_pContacts[i].pent[1]->m_flags&ref_use_simple_solver)
			((CRigidEntity*)m_pContacts[i].pent[1])->m_bStable = 2;
	}

	/*if ((bResolveInstantly^1) & -nContacts>>31) {
		int j,nBodies,iter;
		vectorf r,dP,n,dP0;
		RigidBody *pBodies[17],*pbody;
		real pAp,a,b,r2,r2new;
		float dpn,dptang2,dPn,dPtang2;

		for(i=nBodies=0,r2=0;i<nContacts;i++) {
			for(j=0;j<2;j++) if (!pContacts[i]->pbody[j]->bProcessed)	{
				pBodies[nBodies++] = pContacts[i]->pbody[j]; pContacts[i]->pbody[j]->bProcessed = 1;
			}
			pContacts[i]->vreq = pContacts[i]->dP;
			(pContacts[i]->Kinv = pContacts[i]->K).Invert33();
			pContacts[i]->dP = pContacts[i]->Kinv*pContacts[i]->r0;
			r2 += pContacts[i]->dP*pContacts[i]->r0;
			pContacts[i]->P.zero();
		}
		for(i=0;i<nBodies;i++)
			pBodies[i]->bProcessed = 0;
		iter = nContacts;

		do {
			for(i=0; i<nBodies; i++) {
				pBodies[i]->Fcollision.zero(); pBodies[i]->Tcollision.zero();
			} for(i=0; i<nContacts; i++) for(j=0;j<2;j++) {
				r = pContacts[i]->pt[j]-pContacts[i]->pbody[j]->pos;
				pContacts[i]->pbody[j]->Fcollision += pContacts[i]->dP*(1-j*2); 
				pContacts[i]->pbody[j]->Tcollision += r^pContacts[i]->dP*(1-j*2);
			} for(i=0; i<nContacts; i++) for(pContacts[i]->vrel.zero(),j=0;j<2;j++) {
				pbody = pContacts[i]->pbody[j]; r = pContacts[i]->pt[j]-pbody->pos;
				pContacts[i]->vrel += (pbody->Fcollision*pbody->Minv + (pbody->Iinv*pbody->Tcollision^r))*(1-j*2);
			} for(i=0,pAp=0; i<nContacts; i++)
				pAp += pContacts[i]->vrel*pContacts[i]->dP;
			a = min((real)20.0,r2/max((real)1E-10,pAp));
			for(i=0,r2new=0; i<nContacts; i++) {
				pContacts[i]->vrel = pContacts[i]->Kinv*(pContacts[i]->r0 -= pContacts[i]->vrel*a);
				r2new += pContacts[i]->vrel*pContacts[i]->r0;
				pContacts[i]->P += pContacts[i]->dP*a;
			}
			b = min((real)1.0,r2new/r2); r2=r2new;
			for(i=0;i<nContacts;i++)
				(pContacts[i]->dP*=b) += pContacts[i]->vrel;
		} while(--iter && r2>sqr(0.03f));//m_Emin);

		for(i=0;i<nContacts;i++) {
			pContacts[i]->dP = pContacts[i]->vreq; pContacts[i]->vreq.zero();
			n = pContacts[i]->n;
			dpn = n*pContacts[i]->r; dptang2 = (pContacts[i]->r-n*dpn).len2();
			dP = pContacts[i]->P;
			//dP0 = (pContacts[i]->r*m_pWorld->m_vars.maxVel+pContacts[i]->vrel)*pContacts[i]->Kinv(0,0);
			//if (dP.len2()>dP0.len2()*sqr(1.5f))
			//	dP = dP0;
			dPn = n*dP; dPtang2 = (dP-n*dPn).len2();
			if (dptang2>sqr_signed(-dpn*pContacts[i]->friction) && dPtang2>sqr_signed(dPn*pContacts[i]->friction)) {
				dP = (dP-n*dPn).normalized()*(max(dPn,0.0f)*pContacts[i]->friction)+pContacts[i]->n*dPn;
				if (sqr(dpn*pContacts[i]->friction*2.5f) < dptang2) for(j=m_nColliders-1;j>=0;j--)
				if (!((m_pColliderContacts[j]&=~getmask(pContacts[i]->bProcessed)) | m_pColliderConstraints[j]) && !m_pColliders[j]->HasContactsWith(this)) {
					CPhysicalEntity *pCollider = m_pColliders[j]; 
					pCollider->RemoveCollider(this); RemoveCollider(pCollider);
				}
			}
			if (dP*n > min(0.0f,(pContacts[i]->dP*n)*-2.5f)) {
				pContacts[i]->dP = dP;
				for(j=0;j<2;j++,dP.flip()) {
					r = pContacts[i]->pt[j] - pContacts[i]->pbody[j]->pos;
					pContacts[i]->pbody[j]->P += dP;	pContacts[i]->pbody[j]->L += r^dP;
					pContacts[i]->pbody[j]->v = pContacts[i]->pbody[j]->P*pContacts[i]->pbody[j]->Minv, 
					pContacts[i]->pbody[j]->w = pContacts[i]->pbody[j]->Iinv*pContacts[i]->pbody[j]->L;
				}
			} else
				pContacts[i]->dP.zero();
		}
#ifdef _DEBUG
		for(i=0;i<nContacts;i++) {
			dP = pContacts[i]->pbody[0]->v+(pContacts[i]->pbody[0]->w^pContacts[i]->pt[0]-pContacts[i]->pbody[0]->pos);
			dP-= pContacts[i]->pbody[1]->v+(pContacts[i]->pbody[1]->w^pContacts[i]->pt[1]-pContacts[i]->pbody[1]->pos);
			r = pContacts[i]->r*m_pWorld->m_vars.maxVel; 
			a = r*dP; b = r.len2();	r2 = pContacts[i]->r0*r;
			b = a;
		}
#endif
	}*/
}

static float g_timeInterval=0.01f,g_rtimeInterval=100.0f;

int CRigidEntity::UpdatePenaltyContact(int i, float time_interval)//, int bResolveInstantly,entity_contact **pContacts,int &nContacts)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	int j,bRemoveContact,bCanPull;
	vectorf dp,n,vrel,dP,r;
	float dpn,dptang2,dPn,dPtang2;
	matrix3x3f rmtx,rmtx1;
	
	if (g_timeInterval!=time_interval)
		g_rtimeInterval = 1.0f/(g_timeInterval=time_interval);

	for(j=0; j<2; j++)
		m_pContacts[i].pt[j] = m_pContacts[i].pbody[j]->q*m_pContacts[i].ptloc[j]+m_pContacts[i].pbody[j]->pos;
	m_pContacts[i].n=n = m_pContacts[i].pbody[m_pContacts[i].iNormal]->q*m_pContacts[i].nloc;
	dp = m_pContacts[i].pt[0]-m_pContacts[i].pt[1];
	dpn = dp*n; dptang2 = (dp-n*dpn).len2();
	bRemoveContact = 0;
	bCanPull = isneg(m_impulseTime-1E-6f);

	if (dpn<m_pWorld->m_vars.maxContactGapSimple) {//,(m_pContacts[i].r*n)*-2)) {
		m_pContacts[i].r = dp;
		dp *= g_rtimeInterval*m_pWorld->m_vars.penaltyScale;
		vrel = m_pContacts[i].pbody[0]->v+(m_pContacts[i].pbody[0]->w^m_pContacts[i].pt[0]-m_pContacts[i].pbody[0]->pos);
		vrel-= m_pContacts[i].pbody[1]->v+(m_pContacts[i].pbody[1]->w^m_pContacts[i].pt[1]-m_pContacts[i].pbody[1]->pos);
		m_pContacts[i].vrel = vrel;
		dp += vrel;

		m_pContacts[i].K.SetZero();
		for(j=0;j<2;j++) {
			r = m_pContacts[i].pt[j] - m_pContacts[i].pbody[j]->pos;
			((crossproduct_matrix(r,rmtx))*=m_pContacts[i].pbody[j]->Iinv)*=crossproduct_matrix(r,rmtx1);
			m_pContacts[i].K -= rmtx; 
			for(int idx=0;idx<3;idx++)
				m_pContacts[i].K(idx,idx) += m_pContacts[i].pbody[j]->Minv;
		}

		/*if (!bResolveInstantly) {
			m_pContacts[i].r0 = -dp;
			m_pContacts[i].bProcessed = i;
			pContacts[nContacts++] = m_pContacts+i;
			return 0;
		}*/

		//(m_pContacts[i].Kinv = m_pContacts[i].K).Invert33();
		//dP = m_pContacts[i].Kinv*-dp;
		dP = dp*(-dp.len2()/(dp*m_pContacts[i].K*dp));
		dPn = dP*n; dPtang2 = (dP-n*dPn).len2();
		if (dptang2>sqr_signed(-dpn*m_pContacts[i].friction)*bCanPull && dPtang2>sqr_signed(dPn*m_pContacts[i].friction)) {
			dP = (dP-n*dPn).normalized()*(max(dPn,0.0f)*m_pContacts[i].friction)+n*dPn;
			bRemoveContact = isneg(sqr(m_pWorld->m_vars.maxContactGapSimple)-dptang2);
		}
		if (dP*n > min(0.0f,(m_pContacts[i].dP*n)*-2.1f*bCanPull)) {
			m_pContacts[i].dP = dP;
			for(j=0;j<2;j++,dP.flip()) {
				r = m_pContacts[i].pt[j] - m_pContacts[i].pbody[j]->pos;
				m_pContacts[i].pbody[j]->P += dP;	m_pContacts[i].pbody[j]->L += r^dP;
				m_pContacts[i].pbody[j]->v = m_pContacts[i].pbody[j]->P*m_pContacts[i].pbody[j]->Minv; 
				m_pContacts[i].pbody[j]->w = m_pContacts[i].pbody[j]->Iinv*m_pContacts[i].pbody[j]->L;
			}
		} else
			m_pContacts[i].dP.zero();
	}	else 
		bRemoveContact = 1;
	
	if (bRemoveContact) for(j=m_nColliders-1;j>=0;j--)
	if (!((m_pColliderContacts[j]&=~getmask(i)) | m_pColliderConstraints[j]) && !m_pColliders[j]->HasContactsWith(this)) {
		CPhysicalEntity *pCollider = m_pColliders[j]; 
		pCollider->RemoveCollider(this); RemoveCollider(pCollider);
	}

	return bRemoveContact;
}


int CRigidEntity::RegisterConstraint(const vectorf &pt0,const vectorf &pt1, int ipart0, CPhysicalEntity *pBuddy,int ipart1, int flags)
{
	int i;
	masktype constraint_mask;

	for(i=0,constraint_mask=0; i<m_nColliders; constraint_mask|=m_pColliderConstraints[i++]);
	for(i=0; i<NMASKBITS && constraint_mask & getmask(i); i++);
	if (i==NMASKBITS) return -1;
	if (!pBuddy) pBuddy = &g_StaticPhysicalEntity;

	if (i>=m_nConstraintsAlloc) {
		entity_contact *pConstraints = m_pConstraints;
		constraint_info *pInfos = m_pConstraintInfos;
		int nConstraints = m_nConstraintsAlloc;
		memcpy(m_pConstraints = new entity_contact[(m_nConstraintsAlloc=(i&~7)+8)], pConstraints, nConstraints*sizeof(entity_contact));
		memcpy(m_pConstraintInfos = new constraint_info[m_nConstraintsAlloc], pInfos, nConstraints*sizeof(constraint_info));
		if (pConstraints) delete[] pConstraints;
		if (pInfos) delete[] pInfos;
	}
	m_pColliderConstraints[AddCollider(pBuddy)] |= getmask(i);
	pBuddy->AddCollider(this);
	
	m_pConstraints[i].pt[0] = pt0;
	m_pConstraints[i].pt[1] = pt1;
	m_pConstraints[i].n.Set(0,0,1);
	m_pConstraints[i].iNormal = 0;
	m_pConstraints[i].dir.zero();
	m_pConstraints[i].pent[0] = this;
	m_pConstraints[i].pent[1] = pBuddy;
	m_pConstraints[i].ipart[0] = ipart0;
	m_pConstraints[i].ipart[1] = ipart1;
	m_pConstraints[i].pbody[0] = GetRigidBody(ipart0);
	m_pConstraints[i].pbody[1] = pBuddy->GetRigidBody(ipart1);

	m_pConstraints[i].ptloc[0] = (m_pConstraints[i].pt[0]-m_pConstraints[i].pbody[0]->pos)*m_pConstraints[i].pbody[0]->q;
	m_pConstraints[i].ptloc[1] = (m_pConstraints[i].pt[1]-m_pConstraints[i].pbody[1]->pos)*m_pConstraints[i].pbody[1]->q;

	m_pConstraints[i].vrel.zero();
	m_pConstraints[i].friction = 0;
	m_pConstraints[i].flags = flags;

	m_pConstraints[i].Pspare = 0;
	m_pConstraints[i].K.SetZero();
	GetContactMatrix(m_pConstraints[i].pt[0], m_pConstraints[i].ipart[0], m_pConstraints[i].K);
	pBuddy->GetContactMatrix(m_pConstraints[i].pt[1], m_pConstraints[i].ipart[1], m_pConstraints[i].K);
	m_pConstraints[i].vreq.zero();
	//m_pConstraints[i].vsep = 0;

	m_pConstraintInfos[i].flags = 0;
	m_pConstraintInfos[i].pConstraintEnt = 0;
	m_pConstraintInfos[i].bActive = 0;

	return i;
}


int CRigidEntity::RemoveConstraint(int iConstraint)
{
	int i;
	for(i=0;i<m_nColliders && !(m_pColliderConstraints[i] & getmask(iConstraint));i++);
	if (i==m_nColliders)
		return 0;
	CPhysicalEntity *pBuddy = m_pConstraints[iConstraint].pent[1];
	if (!((m_pColliderConstraints[i]&=~getmask(iConstraint)) | m_pColliderContacts[i]) && !m_pColliders[i]->HasContactsWith(this)) {
		pBuddy->RemoveCollider(this); RemoveCollider(pBuddy);
	}
	return 1;
}


int CRigidEntity::VerifyExistingContacts(float maxdist)
{
	int i,j,j1,ipart,bConfirmed,contact_mask,icode0,icode1;
	geom_world_data gwd;
	vectorf ptres[2],n,ptclosest[2];
	RigidBody *pbody;
	CPhysicalEntity *pent;

	// verify all existing contacts with dynamic entities
	for(i=0;i<m_nColliders;i++) //if (m_bAwake+m_pColliders[i]->IsAwake()>0)
	for(j=0;j<NMASKBITS && getmask(j)<=m_pColliderContacts[i];j++) 
	if (m_pColliderContacts[i] & getmask(j) && IsAwake(m_pContacts[j].ipart[0])+m_pColliders[i]->IsAwake(m_pContacts[j].ipart[1])>0) {
		if (!(m_pContacts[j].flags & contact_new)) {
			bConfirmed = 0;
			if (m_pContacts[j].penetration==0) {
				ptres[0].Set(1E9f,1E9f,1E9f); ptres[1].Set(1E11f,1E11f,1E11f);

				pent = m_pContacts[j].pent[0]; ipart = m_pContacts[j].ipart[0];
				//(pent->m_qrot*pent->m_parts[ipart].q).getmatrix(gwd.R);	//Q2M_IVO
				gwd.R = matrix3x3f(pent->m_qrot*pent->m_parts[ipart].q);
				gwd.offset = pent->m_pos + pent->m_qrot*pent->m_parts[ipart].pos;
				gwd.scale = pent->m_parts[ipart].scale;
				if ((m_pContacts[j].iFeature[0]&0xFFFFFF60)!=0) {
					pbody = m_pContacts[j].pbody[1]; 
					ptres[1] = pbody->q*m_pContacts[j].ptloc[1]+pbody->pos;
					if (pent->m_parts[ipart].pPhysGeomProxy->pGeom->FindClosestPoint(&gwd, m_pContacts[j].iPrim[0],m_pContacts[j].iFeature[0], 
						ptres[1],ptres[1], ptclosest)<0)
						goto endcheck;
					ptres[0] = ptclosest[0];
				} else {
					pent->m_parts[ipart].pPhysGeomProxy->pGeom->GetFeature(m_pContacts[j].iPrim[0],m_pContacts[j].iFeature[0], ptres);
					ptres[0] = gwd.R*(ptres[0]*gwd.scale)+gwd.offset;
				}
				pbody = m_pContacts[j].pbody[0]; m_pContacts[j].ptloc[0] = (ptres[0]-pbody->pos)*pbody->q;
				m_pContacts[j].pt[0] = m_pContacts[j].pt[1] = ptres[0];

				pent = m_pContacts[j].pent[1]; ipart = m_pContacts[j].ipart[1];
				//(pent->m_qrot*pent->m_parts[ipart].q).getmatrix(gwd.R);	//Q2M_IVO
				gwd.R = matrix3x3f(pent->m_qrot*pent->m_parts[ipart].q);
				gwd.offset = pent->m_pos + pent->m_qrot*pent->m_parts[ipart].pos;
				gwd.scale = pent->m_parts[ipart].scale;
				if ((m_pContacts[j].iFeature[1]&0xFFFFFF60)!=0) {
					if (pent->m_parts[ipart].pPhysGeomProxy->pGeom->FindClosestPoint(&gwd, m_pContacts[j].iPrim[1],m_pContacts[j].iFeature[1], 
						ptres[0],ptres[0], ptclosest)<0)
						goto endcheck;
					ptres[1] = ptclosest[0];
				} else {
					pent->m_parts[ipart].pPhysGeomProxy->pGeom->GetFeature(m_pContacts[j].iPrim[1],m_pContacts[j].iFeature[1], ptres+1);
					ptres[1] = gwd.R*(ptres[1]*gwd.scale)+gwd.offset;
				}
				pbody = m_pContacts[j].pbody[1]; m_pContacts[j].ptloc[1] = (ptres[1]-pbody->pos)*pbody->q;

				n = (ptres[0]-ptres[1]).normalized();
				if (m_pContacts[j].n*n>0.98f) {
					m_pContacts[j].n = n; bConfirmed = 1;
				} else 
					bConfirmed = iszero(m_pContacts[j].flags & contact_2b_verified);
				m_pContacts[j].iNormal = isneg((m_pContacts[j].iFeature[0]&0x60)-(m_pContacts[j].iFeature[1]&0x60));
				m_pContacts[j].nloc = m_pContacts[j].n*m_pContacts[j].pbody[m_pContacts[j].iNormal]->q;
				m_pContacts[j].K.SetZero();
				m_pContacts[j].pent[0]->GetContactMatrix(m_pContacts[j].pt[0], m_pContacts[j].ipart[0], m_pContacts[j].K);
				m_pContacts[j].pent[1]->GetContactMatrix(m_pContacts[j].pt[1], m_pContacts[j].ipart[1], m_pContacts[j].K);

				m_pContacts[j].vrel = m_pContacts[j].pbody[0]->v+(m_pContacts[j].pbody[0]->w^m_pContacts[j].pt[0]-m_pContacts[j].pbody[0]->pos) - 
					m_pContacts[j].pbody[1]->v-(m_pContacts[j].pbody[1]->w^m_pContacts[j].pt[0]-m_pContacts[j].pbody[1]->pos);
				m_pContacts[j].friction = 0.5f*max(0.0f, m_pContacts[j].vrel.len2()<sqr(m_pWorld->m_vars.maxContactGap*5) ? 
					m_pWorld->m_FrictionTable[m_pContacts[j].id[0]&NSURFACETYPES-1] + m_pWorld->m_FrictionTable[m_pContacts[j].id[1]&NSURFACETYPES-1] :
					m_pWorld->m_DynFrictionTable[m_pContacts[j].id[0]&NSURFACETYPES-1] + m_pWorld->m_DynFrictionTable[m_pContacts[j].id[1]&NSURFACETYPES-1]);

				bConfirmed &= isneg((ptres[0]-ptres[1]).len2()-sqr(maxdist));
				if (iszero(m_pContacts[j].iFeature[0]&0x60) | iszero(m_pContacts[j].iFeature[0]&0x60)) {
					// if at least one contact point is at geometry vertex, make sure there are no other contacts for the same combination of features
					icode0 = m_pContacts[j].ipart[0]<<24 | m_pContacts[j].iPrim[0]<<7 | m_pContacts[j].iFeature[0]&0x7F;
					icode1 = m_pContacts[j].ipart[1]<<24 | m_pContacts[j].iPrim[1]<<7 | m_pContacts[j].iFeature[1]&0x7F;
					for(j1=0;j1<j;j1++) if (m_pColliderContacts[i] & getmask(j1) &&
						icode0 == m_pContacts[j1].ipart[0]<<24 | m_pContacts[j1].iPrim[0]<<7 | m_pContacts[j1].iFeature[0]&0x7F &&
						icode1 == m_pContacts[j1].ipart[1]<<24 | m_pContacts[j1].iPrim[1]<<7 | m_pContacts[j1].iFeature[1]&0x7F)
					{ bConfirmed = 0; break; }
				}
			}

			endcheck:
			if (!bConfirmed)
				m_pColliderContacts[i] &= ~getmask(j);
			/*if (!bConfirmed && // remove unconfirmed contact
					!((m_pColliderContacts[i] &= ~getmask(j)) | m_pColliderConstraints[i]) && !m_pColliders[i]->HasContactsWith(this)) 
			{ // remove collider if it has no active contacts
				m_pColliders[i]->RemoveCollider(this);
				RemoveCollider(m_pColliders[i]);	
			}*/
		}
		m_pContacts[j].flags &= ~(contact_new | contact_2b_verified);
	}

	for(i=m_nColliders-1;i>=0;i--) if ((m_pColliderContacts[i] | m_pColliderConstraints[i])==0 && 
			!HasContactsWith(m_pColliders[i]) && !m_pColliders[i]->HasContactsWith(this)) 
	{
		CPhysicalEntity *pCollider = m_pColliders[i]; 
		pCollider->RemoveCollider(this); RemoveCollider(pCollider);
	}
	for(i=contact_mask=0;i<m_nColliders;contact_mask|=m_pColliderContacts[i++]);
	return contact_mask;
}


void CRigidEntity::UpdateConstraints()
{
	int i; masktype constraint_mask=0;
	vectorf drift,angles;
	quaternionf qframe0,qframe1;
	for(i=0;i<m_nColliders;constraint_mask|=m_pColliderConstraints[i++]);

	for(i=0;i<NMASKBITS && getmask(i)<=constraint_mask;i++) if (constraint_mask & getmask(i)) {
		m_pConstraints[i].pt[0] = m_pConstraints[i].pbody[0]->q*m_pConstraints[i].ptloc[0]+m_pConstraints[i].pbody[0]->pos;
		m_pConstraints[i].pt[1] = m_pConstraints[i].pbody[1]->q*m_pConstraints[i].ptloc[1]+m_pConstraints[i].pbody[1]->pos;
		drift = m_pConstraints[i].pt[1]-m_pConstraints[i].pt[0];
		m_pConstraintInfos[i].bActive = 1;

		if (m_pConstraintInfos[i].flags & constraint_rope) {
			pe_params_rope pr;
			m_pConstraintInfos[i].pConstraintEnt->GetParams(&pr);
			if (drift.len2() > sqr(pr.length*0.99f)) {
				m_pConstraints[i].n = drift.normalized();
				if (drift.len2() > sqr(pr.length))
					m_pConstraints[i].vreq = (drift-m_pConstraints[i].n*pr.length)*5.0f;
				else m_pConstraints[i].vreq.zero();
			} else
				m_pConstraintInfos[i].bActive = 0;
			pe_simulation_params sp;
			m_pConstraintInfos[i].pConstraintEnt->GetParams(&sp);
			m_dampingEx = max(m_dampingEx, sp.damping);
			m_pConstraintInfos[i].pConstraintEnt->Awake();
		}	else if (m_pConstraints[i].flags & contact_constraint_3dof) {
			m_pConstraints[i].n = drift.normalized();
			m_pConstraints[i].vreq = drift*10.0f;
			//m_pConstraints[i].vsep = (m_pConstraints[i].vreq*m_pConstraints[i].n)*0.5f;
		} else if (m_pConstraints[i].flags & contact_angular) {
			m_pConstraints[i].n = m_pConstraints[i].pbody[0]->q*m_pConstraints[i].nloc;
			qframe0 = m_pConstraints[i].pbody[0]->q*m_pConstraintInfos[i].qframe_rel[0];
			qframe1 = m_pConstraints[i].pbody[1]->q*m_pConstraintInfos[i].qframe_rel[1];

			if (m_pConstraints[i].flags & contact_constraint_2dof | m_pConstraintInfos[i].flags & constraint_limited_1axis) {
				//(qframe1*qframe0).GetEulerAngles_XYZ(angles);	//EULER_IVO
				angles = Ang3::GetAnglesXYZ(matrix3x3f(qframe1*qframe0));
				if (!m_pConstraintInfos[i].flags)
					m_pConstraints[i].vreq = m_pConstraints[i].n*(-angles.x*10.0f);
				else {
					if (angles.x < m_pConstraintInfos[i].limits[0])
						m_pConstraints[i].vreq = m_pConstraints[i].n*min(5.0f,(m_pConstraintInfos[i].limits[0]-angles.x)*10.0f);
					else if (angles.x > m_pConstraintInfos[i].limits[1]) {
						m_pConstraints[i].n.Flip();
						m_pConstraints[i].vreq = m_pConstraints[i].n*min(5.0f,(angles.x-m_pConstraintInfos[i].limits[1])*10.0f);
					} else
						m_pConstraintInfos[i].bActive = 0;
				}
			} else if (m_pConstraints[i].flags & contact_constraint_1dof | m_pConstraintInfos[i].flags & constraint_limited_2axes) {
				drift = m_pConstraints[i].n ^ qframe1*vectorf(1,0,0);
				if (!m_pConstraintInfos[i].flags)	{
					if (drift.len2()>1.0f)
						drift.normalize();
					m_pConstraints[i].vreq = drift*10.0f;
				} else if (drift.len2()>m_pConstraintInfos[i].limits[0])
					m_pConstraints[i].vreq = drift.normalized()*min(10.0f,drift.len()-m_pConstraintInfos[i].limits[0]);
				else
					m_pConstraintInfos[i].bActive = 0;
			}
		}

		m_pConstraints[i].K.SetZero();
		if (!(m_pConstraints[i].flags & contact_angular)) {
			m_pConstraints[i].pent[0]->GetContactMatrix(m_pConstraints[i].pt[0], m_pConstraints[i].ipart[0], m_pConstraints[i].K);
			m_pConstraints[i].pent[1]->GetContactMatrix(m_pConstraints[i].pt[1], m_pConstraints[i].ipart[1], m_pConstraints[i].K);
		} else {
			m_pConstraints[i].K += m_pConstraints[i].pbody[0]->Iinv;
			m_pConstraints[i].K += m_pConstraints[i].pbody[1]->Iinv;
		}
	}
}


float CRigidEntity::CalcEnergy(float time_interval)
{
	vectorf v(zero);
	float vmax=0,Emax=m_body.M*sqr(m_pWorld->m_vars.maxVel)*2;
	int i; masktype contact_mask,constraint_mask;

	if (time_interval>0) {
		if (m_flags & ref_use_simple_solver & -m_pWorld->m_vars.bLimitSimpleSolverEnergy)
			return m_E0;

		for(i=0,contact_mask=constraint_mask=0; i<m_nColliders; i++) {
			contact_mask |= m_pColliderContacts[i];
			constraint_mask |= m_pColliderConstraints[i];
		}
		if (!(m_flags & ref_use_simple_solver))
			for(i=0; i<NMASKBITS && getmask(i)<=contact_mask; i++) if (contact_mask & getmask(i))	{
				v += m_pContacts[i].n*max(0.0f,max(0.0f,m_pContacts[i].vreq*m_pContacts[i].n-max(0.0f,m_pContacts[i].vrel*m_pContacts[i].n))-m_pContacts[i].n*v);
				if (m_pContacts[i].pbody[1]->Minv==0 && m_pContacts[i].pent[1]->m_iSimClass>1) {
					RigidBody *pbody = m_pContacts[i].pbody[1];
					vmax = max(vmax,(pbody->v + (pbody->w^pbody->pos-m_pContacts[i].pt[1])).len2());
					Emax = m_body.M*sqr(10.0f)*2; // since will allow some extra energy, make sure we restrict the upper limit
				}
			}
		for(i=0; i<NMASKBITS && getmask(i)<=constraint_mask; i++) 
		if (constraint_mask & getmask(i) && (m_pConstraints[i].flags & contact_constraint_3dof || m_pConstraintInfos[i].flags & constraint_rope))	{
			v += m_pConstraints[i].n*max(0.0f,max(0.0f,m_pConstraints[i].vreq*m_pConstraints[i].n-max(0.0f,m_pConstraints[i].vrel*m_pConstraints[i].n))-
				m_pConstraints[i].n*v);
			if (m_pConstraints[i].pbody[1]->Minv==0 && m_pConstraints[i].pent[1]->m_iSimClass>1) {
				RigidBody *pbody = m_pConstraints[i].pbody[1];
				vmax = max(vmax,(pbody->v + (pbody->w^pbody->pos-m_pConstraints[i].pt[1])).len2());
			}
		}
		return min_safe(m_body.M*(m_body.v+v).len2() + m_body.L*m_body.w, Emax);
	}

	return m_body.M*(m_body.v+v).len2() + m_body.L*m_body.w;
}


void CRigidEntity::StartStep(float time_interval)
{
	m_timeStepPerformed = 0;
	m_timeStepFull = time_interval;
}

int __bstop = 0;
extern int __curstep;

int CRigidEntity::Step(float time_interval)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );
	PHYS_ENTITY_PROFILER

	geom_contact *pcontacts;
	int i,j,itmax,bRecheckPenetrations,ncontacts,nAwakeColliders,bHasContacts=0,bSeverePenetration=0;
	int bUseSimpleSolver = isneg(-((int)m_flags&ref_use_simple_solver)), 
			bUseDistanceContacts = m_pWorld->m_vars.bUseDistanceContacts & (bUseSimpleSolver^1);
	//nSlidingContacts,bHasPenetrations;
	//int bEnforceContacts = m_bEnforceContacts&~(m_bEnforceContacts>>31) | m_pWorld->m_vars.bEnforceContacts&(m_bEnforceContacts>>31), 
	//		bProhibitUnprojection = m_bProhibitUnprojection&~(m_bProhibitUnprojection>>31) | 
	//														m_pWorld->m_vars.bProhibitUnprojection&(m_bProhibitUnprojection>>31);
	masktype contact_mask=0;
	real tmax,e;//,angle; 
	float r;
	//vector dp,wrel,nsrc,ndst,dirsrc,dirdst,offset,axis;//,ptsrc_st[3],ptdst_st[3],ptsrc_sl[4],ptdst_sl[4],n_st[3],n_sl[4];
	quaternionf qrot,qrot0;
	vectorf axis,pos0,pos;
	geom_world_data gwd;
	intersection_params ip;
	//RigidBody *pbody;

	for(i=m_nColliders-1,nAwakeColliders=0; i>=0; i--) {
		if (m_pColliders[i]->m_iSimClass==7)
			RemoveCollider(m_pColliders[i]);
		else {
			contact_mask |= m_pColliderContacts[i];
			bHasContacts |= (iszero((int)m_pColliderContacts[i])^1) | m_pColliders[i]->HasCollisionContactsWith(this);
			nAwakeColliders += isneg(-m_pColliders[i]->m_iSimClass);
		}
	}
	m_prevPos = m_body.pos;
	m_prevq = m_body.q;
	m_prevv = m_body.v; m_prevw = m_body.w;

	m_dampingEx = 0;
	if (m_timeStepPerformed>m_timeStepFull-0.001f || m_nParts==0)
		return 1;
	m_timeStepPerformed += time_interval;
	for(i=0;i<sizeof(m_CollHistory)/sizeof(m_CollHistory[0]);i++)
		m_CollHistory[i].age += time_interval;
	m_lastTimeStep = time_interval;
	m_impulseTime = max(0.0f,m_impulseTime-time_interval);
	int bContactOverflow = isnonneg(m_timeContactOverflow -= time_interval);
	m_timeContactOverflow *= bContactOverflow;
	m_flags = m_flags&~ref_contact_overflow | ref_contact_overflow&-bContactOverflow;

	// if not sleeping, check for new contacts with all surrounding entities
	if (m_bAwake | bUseSimpleSolver) {
		m_body.P += m_Pext[0]; m_Pext[0].zero();
		m_body.L += m_Lext[0]; m_Lext[0].zero();
		m_body.Step(time_interval);
		if (m_body.v.len2() > sqr(m_pWorld->m_vars.maxVel)) {
			m_body.v.normalize() *= m_pWorld->m_vars.maxVel;
			m_body.P = m_body.v*m_body.M;
		}
		if (m_body.w.len2() > sqr(m_maxw)) {
			m_body.w.normalize() *= m_maxw;
			m_body.L = m_body.q*(m_body.Ibody*(!m_body.q*m_body.w));
		}
		pos0 = m_pos;	qrot0 = m_qrot;
		m_qrot = m_body.q*m_body.qfb;
		m_pos = m_body.pos-m_qrot*m_body.offsfb;

		ip.maxSurfaceGapAngle = 1.0f*(pi/180);
		ip.axisContactNormal = -m_gravity.normalized();
		//ip.bNoAreaContacts = !m_pWorld->m_vars.bUseDistanceContacts;
		gwd.v = m_body.v;
		gwd.w = m_body.w;
		gwd.centerOfMass = m_body.pos;
		
		/*for(i=0; i<min(m_nStickyContacts,3); i++) {
			ptsrc_st[i] = m_pContacts[m_iStickyContacts[i]].pbody[0]->q*m_pContacts[m_iStickyContacts[i]].ptloc[0] +
										m_pContacts[m_iStickyContacts[i]].pbody[0]->pos;
			ptdst_st[i] = m_pContacts[m_iStickyContacts[i]].pbody[1]->q*m_pContacts[m_iStickyContacts[i]].ptloc[1] +
										m_pContacts[m_iStickyContacts[i]].pbody[1]->pos;
			pbody = m_pContacts[m_iStickyContacts[i]].pbody[m_pContacts[m_iStickyContacts[i]].iNormal];
			n_st[i] = pbody->q*m_pContacts[m_iStickyContacts[i]].nloc;
		}
		nSlidingContacts = min(m_nSlidingContacts,3);
		for(i=0; i<nSlidingContacts; i++) {
			ptsrc_sl[i] = m_pContacts[m_iSlidingContacts[i]].pbody[0]->q*m_pContacts[m_iSlidingContacts[i]].ptloc[0] +
										m_pContacts[m_iSlidingContacts[i]].pbody[0]->pos;
			ptdst_sl[i] = m_pContacts[m_iSlidingContacts[i]].pbody[1]->q*m_pContacts[m_iSlidingContacts[i]].ptloc[1] +
										m_pContacts[m_iSlidingContacts[i]].pbody[1]->pos;
			pbody = m_pContacts[m_iSlidingContacts[i]].pbody[m_pContacts[m_iSlidingContacts[i]].iNormal];
			n_sl[i] = pbody->q*m_pContacts[m_iSlidingContacts[i]].nloc;
		}
		if (m_nStickyContacts>0)
			wrel = m_body.w-m_pContacts[m_iStickyContacts[0]].pbody[1]->w;
		for(i=bHasPenetrations=0;i<NMASKBITS && getmask(i)<=contact_mask;i++) bHasPenetrations += iszero(m_pContacts[i].penetration)^1;
		e = m_pWorld->m_vars.maxContactGap*0.5f;
		ip.time_interval = time_interval*3;

		if (nAwakeColliders<bEnforceContacts) {
			if (m_nStickyContacts>=3) {
				// match transformed 3 points, use linear v+w unprojection constrained to contact plane
				nsrc = (ptsrc_st[1]-ptsrc_st[0] ^ ptsrc_st[2]-ptsrc_st[0]).normalized();
				ndst = (ptdst_st[1]-ptdst_st[0] ^ ptdst_st[2]-ptdst_st[0]).normalized();
				qrot.SetRotationV0V1(nsrc,ndst); //qrot = quaternionf(nsrc,ndst); 
				if (qrot.w>0.999f) {
					m_qrot = qrot*m_qrot;
					for(i=0;i<3;i++) ptsrc_st[i] = m_pos+qrot*(ptsrc_st[i]-m_pos);
					dirsrc = (ptsrc_st[0]*(2.0f/3)-(ptsrc_st[1]+ptsrc_st[2])*(1.0f/3)).normalized();
					dirdst = (ptdst_st[0]*(2.0f/3)-(ptdst_st[1]+ptdst_st[2])*(1.0f/3)).normalized();
					qrot.SetRotationV0V1(dirsrc,dirdst); //qrot = quaternionf(dirsrc,dirdst);
					for(i=0,offset.zero();i<3;i++)
						offset += (ptdst_st[i]-m_pos-qrot*(ptsrc_st[i]-m_pos))*(1.0f/3);
					m_qrot = qrot*m_qrot;
					ndst *= sgnnz(ndst*n_st[0]);
					m_pos += (offset += ndst*e);
					ip.iUnprojectionMode = 0;
					ip.unprojectionPlaneNormal = ndst;
				}
			} else if (m_nStickyContacts==2) {
				// match transformed 2 points, use rotational unprojection around contact line
				ip.time_interval = 0.15f; // ca 8.5 degrees
				dirsrc = (ptsrc_st[1]-ptsrc_st[0]).normalized();
				dirdst = (ptdst_st[1]-ptdst_st[0]).normalized();
				qrot.SetRotationV0V1(dirsrc,dirdst); //qrot = quaternionf(dirsrc,dirdst);
				for(i=0,offset.zero(),ndst.zero();i<2;i++) {
					ptsrc_st[i] = m_pos+qrot*(ptsrc_st[i]-m_pos);
					offset += (ptdst_st[i]-ptsrc_st[i])*0.5f;
					pbody = m_pContacts[m_iStickyContacts[i]].pbody[m_pContacts[m_iStickyContacts[i]].iNormal];
					ndst += pbody->q*m_pContacts[m_iStickyContacts[i]].nloc + pbody->pos;
				}
				ndst.normalize();
				if (qrot.w>0.999f) {
					m_qrot = qrot*m_qrot;
					m_pos += (offset += ndst*e);
				}
				ip.iUnprojectionMode = 1;
				ip.axisOfRotation = dirdst;
				ip.centerOfRotation = ptsrc_st[0]+offset;
				ip.unprojectionPlaneNormal = ndst; // constrain LS unprojection to contact plane
			} else if (m_nStickyContacts+nSlidingContacts) {
				if (m_nStickyContacts==1) {
					offset = ptdst_st[0]-ptsrc_st[0]+n_st[0]*e;
					m_pos += offset;
					for(i=nSlidingContacts;i>0;i--) {
						ptsrc_sl[i] = ptsrc_sl[i-1]+offset; ptdst_sl[i] = ptdst_sl[i-1]; n_sl[i] = n_sl[i-1];
					}
					ptsrc_sl[0] = ptsrc_st[0]+offset;	n_sl[0] = n_st[0];
					nSlidingContacts++;
				} else if (nSlidingContacts) {
					offset = n_sl[0]*(n_sl[0]*(ptdst_sl[0]-ptsrc_sl[0])+e);	// consrain the 1st point to contact plane
					m_pos += offset; for(i=0;i<nSlidingContacts;i++) ptsrc_sl[i] += offset;
				}
				ip.iUnprojectionMode = 1;
				ip.centerOfRotation = ptsrc_sl[0];
				ip.axisOfRotation.zero();// = wrel.normalized();
				ip.unprojectionPlaneNormal = n_sl[0];
				if (nSlidingContacts==1) // if unproj. around one point we are not very confident about rot. axis, so limit unprojection more
					ip.time_interval = 0.1f; 
				else ip.time_interval = 0.15f;

				if (nSlidingContacts>1) { // constrain the 2nd point to contact plane while keeping the 1st point fixed, rot. unproj. around contact line
					axis = (n_sl[1]^ptsrc_sl[1]-ptsrc_sl[0]).normalized();
					angle = RotatePointToPlane(ptsrc_sl[1], axis,ptsrc_sl[0], n_sl[1],ptdst_sl[1]+n_sl[1]*e);
					if (fabs_tpl(angle)<0.05f) {
						qrot.SetRotationAA(angle, axis); //qrot = quaternionf(angle, axis);
						m_qrot = qrot*m_qrot; m_pos = ptsrc_sl[0]+qrot*(m_pos-ptsrc_sl[0]);
						for(i=1;i<nSlidingContacts;i++) ptsrc_sl[i] = ptsrc_sl[0]+qrot*(ptsrc_sl[i]-ptsrc_sl[0]);
					}
					ip.axisOfRotation = (ptsrc_sl[1]-ptsrc_sl[0]).normalized();
					if ((n_sl[0]+n_sl[1]).len2()>sqr(0.99f*2))
						ip.unprojectionPlaneNormal = n_sl[0];
					else ip.unprojectionPlaneNormal.zero();
				}

				if (nSlidingContacts>2) {	// constrain the 3d point to contact plane while keeping the 1st two points fixed
					angle = RotatePointToPlane(ptsrc_sl[2], ip.axisOfRotation,ptsrc_sl[0], n_sl[2],ptdst_sl[2]+n_sl[2]*e);
					if (fabs_tpl(angle)<0.05f) {
						qrot.SetRotationAA(angle, ip.axisOfRotation);	//qrot = quaternionf(angle, ip.axisOfRotation);
						m_qrot = qrot*m_qrot; m_pos = ptsrc_sl[0]+qrot*(m_pos-ptsrc_sl[0]);
					}
					if (m_nStickyContacts==1) // rot. unproj. around sticky contact normal
						ip.axisOfRotation = n_st[0];
					else {
						ip.iUnprojectionMode = 0;	// LS unproj.
						ip.vrel_min = 1E10;
						if ((n_sl[0]+n_sl[1]).len2()>sqr(0.99f*2) && (n_sl[1]+n_sl[2]).len2()>sqr(0.99f*2))
							ip.unprojectionPlaneNormal = n_sl[0];
						else ip.unprojectionPlaneNormal.zero();
					}
				}
			} else {
				if (bHasPenetrations) { // use LS unprojections if we already have penetrating contacts
					ip.iUnprojectionMode = 0; ip.vrel_min = 1E10; 
				}
			}
		}
		if (contact_mask && (m_bProhibitUnproj || bProhibitUnprojection && nAwakeColliders>=bProhibitUnprojection-1)) {
			ip.iUnprojectionMode = 0; ip.vrel_min = 1E10;
		}

		if (ip.iUnprojectionMode==1) {
			box abox; vectorf ort,vec,wax;	// calc rotation radius 
			r = (m_BBox[1]-m_BBox[0]).len()*0.1f; wax = gwd.w.normalized();
			for(i=0;i<m_nParts;i++) if (m_parts[i].flags & geom_collides) {
				m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&abox);
				axis = abox.Basis*(wax*(m_qrot*m_parts[i].q));
				for(j=0,ort.zero(); j<3; ort[j++]=0) {
					ort[j] = 1; vec = (axis^(abox.size^ort)).normalized();
					if (fabsf(abox.size[j]*vec[inc_mod3[j]]) > fabsf(abox.size[inc_mod3[j]]*vec[j]))
						r = max(r,fabs_tpl(abox.size[inc_mod3[j]]/vec[inc_mod3[j]]));
				}
			}
			ip.minAxisDist = r*0.1f;
		} else
			ip.time_interval = time_interval*3;*/

		if (m_bCollisionCulling || m_parts[0].pPhysGeomProxy->pGeom->IsConvex(0.02f)) 
			ip.ptOutsidePivot[0] = (m_BBox[0]+m_BBox[1])*0.5f;
		vectorf sz = m_BBox[1]-m_BBox[0];
		ip.maxUnproj = max(max(sz.x,sz.y),sz.z);
		e = m_pWorld->m_vars.maxContactGap*(0.5f-0.4f*bUseSimpleSolver);
		ip.iUnprojectionMode = 0;
		ip.bNoIntersection = bUseSimpleSolver;
		m_qrot.Normalize();

		CheckAdditionalGeometry(time_interval, contact_mask);
		
		if (m_velFastDir*time_interval > m_sizeFastDir*0.71f) {
			pos = m_pos; m_pos = pos0;
			qrot = m_qrot; m_qrot = qrot0;
			ComputeBBox();
			ip.bSweepTest = true; 
			ip.time_interval = time_interval;
			vectorf delta=gwd.v*ip.time_interval;
			m_BBox[isneg(-delta.x)].x += delta.x;
			m_BBox[isneg(-delta.y)].y += delta.y;
			m_BBox[isneg(-delta.z)].z += delta.z;
			ncontacts = CheckForNewContacts(&gwd,&ip, itmax);
			pcontacts = ip.pGlobalContacts;
			if (itmax>=0)	{
				m_pos -= pcontacts[itmax].dir*(pcontacts[itmax].t+e);
				m_body.pos = m_pos+m_qrot*m_body.offsfb;
				m_pos = m_body.pos-qrot*m_body.offsfb;
			} else
				m_pos = pos;
			m_qrot = qrot;
			ip.bSweepTest = false;
		}
		ComputeBBox(); 
		ip.time_interval = time_interval*3;
		ip.vrel_min = ip.maxUnproj/ip.time_interval*(bHasContacts ? 0.2f : 0.05f);
		ip.vrel_min += 1E10f*bUseSimpleSolver;
		ncontacts = CheckForNewContacts(&gwd,&ip, itmax);
		pcontacts = ip.pGlobalContacts;

		if (ncontacts<6) for(i=0;i<ncontacts-1;i++) for(j=i+1;j<ncontacts;j++) 
		if (g_CurColliders[i]==g_CurColliders[j] && g_CurCollParts[i][1]==g_CurCollParts[j][1] &&
				fabs_tpl(pcontacts[i].vel)+fabs_tpl(pcontacts[j].vel)==0 &&
				(pcontacts[i].center-pcontacts[j].center)*pcontacts[i].n<0 &&	// **||**, but not |****|
				(pcontacts[j].center-pcontacts[i].center)*pcontacts[j].n<0) 
		{
			vectorf dci=pcontacts[i].center-m_body.pos, dcj=pcontacts[j].center-m_body.pos;
			if (pcontacts[i].dir*pcontacts[j].dir<-0.93f && sqr_signed(dci*dcj)>sqr(0.5f)*dci.len2()*dcj.len2())
				pcontacts[dci*pcontacts[i].dir>0 ? i:j].t = -1;
		}

		tmax = itmax>=0 ? pcontacts[itmax].t : 0;
		i = itmax; //if (ncontacts==1) i = 0;
		bRecheckPenetrations = 0;
		if (i>=0) { // first unproject tmax contact to exact collision time (to get ptloc fields in entity_contact filled correctly)
			if (pcontacts[i].iUnprojMode==0) // check if unprojection conflicts with existing contacts
				for(j=0; j<NMASKBITS && getmask(j)<=contact_mask && !(contact_mask&getmask(j) && m_pContacts[j].n*pcontacts[i].dir<-1E-3f); j++);
			else for(j=0; j<NMASKBITS && getmask(j)<=contact_mask; j++) if (contact_mask&getmask(j)) {
				axis = pcontacts[i].dir ^ m_pContacts[j].pt[0]-ip.centerOfRotation; r = axis.len2();
				if (r>sqr(ip.minAxisDist) && sqr_signed(axis*m_pContacts[j].n)<-1E-3f*r)
					break;
			}
			if (j<NMASKBITS && contact_mask&getmask(j)) {
				e=0; bRecheckPenetrations=1;
			} else if (pcontacts[i].iUnprojMode==0 && (!bHasContacts || pcontacts[i].t>ip.maxUnproj*0.1f)) {
				m_pos += pcontacts[i].dir*pcontacts[i].t;
				m_body.pos += pcontacts[i].dir*pcontacts[i].t;
			}
			/*else if (pcontacts[i].t<0.3f) {
				e /= (pcontacts[i].pt-ip.centerOfRotation).len();
				//qrot = quaternionf(pcontacts[i].t,pcontacts[i].dir);
				qrot.SetRotationAA(pcontacts[i].t,pcontacts[i].dir);
				m_qrot = qrot*m_qrot; 
				m_pos = ip.centerOfRotation + qrot*(m_pos-ip.centerOfRotation);
			}*/ else {
				e=0; bRecheckPenetrations=1;
			}
			if (pcontacts[i].dir*m_prevUnprojDir<-0.5f)
				m_bProhibitUnproj += 2;
			m_prevUnprojDir = pcontacts[i].dir;
		}

		if (bUseSimpleSolver) {
			m_body.P += m_Pext[1]; m_Pext[1].zero();
			m_body.L += m_Lext[1]; m_Lext[1].zero();
			m_Estep = (m_body.v.len2() + (m_body.L*m_body.w)*m_body.Minv)*0.5f;
			m_body.P += m_gravity*(m_body.M*time_interval);
			m_body.v = m_body.P*m_body.Minv;
			m_body.w = m_body.Iinv*m_body.L;
			m_E0 = m_body.M*m_body.v.len2() + m_body.L*m_body.w;
			UpdatePenaltyContacts(time_interval);
		}

		if (!bRecheckPenetrations) {
			if (bUseDistanceContacts) for(i=0;i<ncontacts;i++) 
				if (pcontacts[i].vel>0 && pcontacts[i].t>tmax-e) { // timed contact in unprojection gap range
					if (pcontacts[i].parea && pcontacts[i].parea->npt>1) {
						/*if (ncontacts==1) {	// align contacting area surfaces
							qrot = quaternionf(pcontacts[i].n,-pcontacts[i].parea->n1);
							m_pos += pcontacts[i].pt-m_pos-qrot*(pcontacts[i].pt-m_pos);
							m_qrot = qrot*m_qrot;
						}*/
						for(j=0;j<pcontacts[i].parea->npt;j++) // do not set contact_new flag for areas since contacts are not precise and should be verified
							RegisterContactPoint(contact_mask,i, pcontacts[i].parea->pt[j], pcontacts, pcontacts[i].parea->piPrim[0][j],
								pcontacts[i].parea->piFeature[0][j], pcontacts[i].parea->piPrim[1][j],pcontacts[i].parea->piFeature[1][j],contact_inexact);
					} else
						RegisterContactPoint(contact_mask,i, pcontacts[i].pt, pcontacts, pcontacts[i].iPrim[0],pcontacts[i].iFeature[0], 
							pcontacts[i].iPrim[1],pcontacts[i].iFeature[1]);
				} else
					bRecheckPenetrations |= pcontacts[i].vel<=0 ? 1:0;//iszero(pcontacts[i].vel)|isneg(pcontacts[i].vel);

			// if we don't generate distance contacts, leave some penetration during unprojection instead of a gap
			e *= bUseDistanceContacts*2-1; 
			if (itmax>=0) { // now additionally unproject tmax contact to create safety gap
				/*if (pcontacts[itmax].parea && pcontacts[itmax].parea->npt>1) 
					m_pos += pcontacts[itmax].parea->n1*(m_pWorld->m_vars.maxContactGap*0.5f);
					else*/ 
				if (pcontacts[itmax].iUnprojMode==0) {
					m_pos += pcontacts[itmax].dir*e;
					m_body.pos += pcontacts[itmax].dir*e;
				} else {
					qrot.SetRotationAA(e,pcontacts[itmax].dir);	//qrot = quaternionf(e,pcontacts[itmax].dir);
					m_qrot = qrot*m_qrot; 
					m_pos = ip.centerOfRotation + qrot*(m_pos-ip.centerOfRotation);
					m_body.pos = m_pos+m_qrot*m_body.offsfb;
					m_body.q = m_qrot*!m_body.qfb;
					m_body.UpdateState();
				}
			}
		}
		ComputeBBox();
		bRecheckPenetrations |= bUseDistanceContacts^1;

		if (bRecheckPenetrations) { // retest for non-unprojected LS contacts
			if (itmax>=0) {
				ip.iUnprojectionMode = 0;
				ip.vrel_min = 1E10;
				ip.maxSurfaceGapAngle = 0;
				ncontacts = CheckForNewContacts(&gwd,&ip, itmax);
			}
			for(i=0;i<ncontacts;i++) if (pcontacts[i].t>=0) { // penetration contacts - register points and add additional penalty impulse in solver
				vectorf ntilt(zero);
				axis.zero(); r=0;
				if (pcontacts[i].parea && pcontacts[i].parea->npt>2) {
					for(j=0;j<pcontacts[i].parea->npt;j++)
						RegisterContactPoint(contact_mask,i, pcontacts[i].parea->pt[j], pcontacts, pcontacts[i].parea->piPrim[0][j],
							pcontacts[i].parea->piFeature[0][j], pcontacts[i].parea->piPrim[1][j],pcontacts[i].parea->piFeature[1][j], 
							contact_2b_verified|contact_new|contact_inexact);
					if (pcontacts[i].parea->type==geom_contact_area::polygon) {
						axis = (ntilt = pcontacts[i].parea->n1^pcontacts[i].n)^pcontacts[i].n;
						for(j=1,r=pcontacts[i].ptborder[0]*axis; j<pcontacts[i].nborderpt; j++)
							r = max(r,pcontacts[i].ptborder[j]*axis);
					}
				} else RegisterContactPoint(contact_mask,i, pcontacts[i].pt, pcontacts, pcontacts[i].iPrim[0],
						pcontacts[i].iFeature[0], pcontacts[i].iPrim[1],pcontacts[i].iFeature[1], contact_2b_verified | contact_new);
				if (!bUseSimpleSolver) for(j=0;j<pcontacts[i].nborderpt;j++)
					if ((itmax = RegisterContactPoint(contact_mask,i, pcontacts[i].ptborder[j], pcontacts, pcontacts[i].iPrim[0],pcontacts[i].iFeature[0], 
						pcontacts[i].iPrim[1],pcontacts[i].iFeature[1], contact_new, max(0.001f,(float)pcontacts[i].t+axis*pcontacts[i].ptborder[j]-r)))>=0)
						m_pContacts[itmax].nloc = ntilt;
				bSeverePenetration |= isneg(ip.maxUnproj*0.4f-pcontacts[i].t);
			}
		}	else
			m_bProhibitUnproj = max(m_bProhibitUnproj-1,0);

		//m_qrot.Normalize();
		//m_body.pos = m_pos+m_qrot*m_body.offsfb;
		//m_body.q = m_qrot*!m_body.qfb;
		//m_body.UpdateState();
	}	else
		CheckAdditionalGeometry(time_interval, contact_mask);	

	if (!bUseSimpleSolver)
		VerifyExistingContacts(m_pWorld->m_vars.maxContactGap);

	if (m_flags & pef_custom_poststep)
		m_pWorld->m_pEventClient->OnPostStep(this,m_pForeignData,m_iForeignData,time_interval);
	vectorf gravity = m_nColliders ? m_gravity : m_gravityFreefall;
	ApplyBuoyancy(time_interval,gravity);
	if (!bUseSimpleSolver) {
		m_body.P += m_Pext[1]; m_Pext[1].zero();
		m_body.L += m_Lext[1]; m_Lext[1].zero();
		m_body.P += gravity*(m_body.M*time_interval);
	}
	AddAdditionalImpulses(time_interval);
	m_body.Fcollision.zero(); m_body.Tcollision.zero();

	m_body.UpdateState();
  m_pWorld->RepositionEntity(this,1);

	if (bSeverePenetration & m_bHadSeverePenetration) {
		if ((m_body.v.len2()+m_body.w.len2()*sqr(ip.maxUnproj*0.5f))*sqr(time_interval) < sqr(ip.maxUnproj*0.1f))
			bSeverePenetration = 0; // apparently we cannot resolve the situation by stepping back
		else {
			m_body.P.zero(); m_body.L.zero(); m_body.v.zero();m_body.w.zero();
		}
	}
	m_bHadSeverePenetration = bSeverePenetration;
	m_bSteppedBack = 0;

	return (m_bHadSeverePenetration^1) | isneg(3-m_nStepBackCount);//1;
}


int CRigidEntity::IsFast(float time_interval)
{
	box bbox;	
	float bestvol=0;
	int i,iBest=-1;
	for(i=0;i<m_nParts;i++) if (m_parts[i].flags & geom_collides) {
		m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&bbox);
		if (bbox.size.volume()>bestvol) {
			bestvol = bbox.size.volume(); iBest = i;
		}
	}
	if (iBest==-1) 
		return 0;
	m_parts[iBest].pPhysGeomProxy->pGeom->GetBBox(&bbox);
	vectorf vloc,vsz,size;
	vloc = bbox.Basis*(m_body.v*(m_qrot*m_parts[iBest].q));
	size = bbox.size*m_parts[iBest].scale;
	vsz(fabsf(vloc.x)*size.y*size.z, fabsf(vloc.y)*size.x*size.z, fabsf(vloc.z)*size.x*size.y);
	i = idxmax3((float*)&vsz);

	return isneg(size[i]-time_interval*fabsf(vloc[i]));
}


float CRigidEntity::GetMaxTimeStep(float time_interval)
{
	if (m_timeStepPerformed > m_timeStepFull-0.001f)
		return time_interval;

	box bbox;	float bestvol=0;
	int i,iBest=-1;
	unsigned int flagsCollider=0;
	for(i=0;i<m_nParts;i++) if (m_parts[i].flags & geom_collides) {
		m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&bbox);
		if (bbox.size.volume()>bestvol) {
			bestvol = bbox.size.volume(); iBest = i;
		}
		flagsCollider |= m_parts[i].flagsCollider;
	}
	if (iBest==-1) 
		return time_interval;
	m_parts[iBest].pPhysGeomProxy->pGeom->GetBBox(&bbox);
	vectorf vloc,vsz,size;
	vloc = bbox.Basis*(m_body.v*(m_qrot*m_parts[iBest].q));
	size = bbox.size*m_parts[iBest].scale;
	vsz(fabsf(vloc.x)*size.y*size.z, fabsf(vloc.y)*size.x*size.z, fabsf(vloc.z)*size.x*size.y);
	i = idxmax3((float*)&vsz);
	m_velFastDir = fabsf(vloc[i]);
	m_sizeFastDir = size[i];

	bool bSkipSpeedCheck = false;
	if (!m_nColliders || m_nColliders==1 && m_pColliders[0]==this) {
		if (m_bCanSweep)
			bSkipSpeedCheck = true;
		else if (min(m_maxAllowedStep,time_interval)*m_velFastDir > m_sizeFastDir*0.7f) {
			int ient,j,nents;
			vectorf BBox[2]={m_BBox[0],m_BBox[1]}, delta=m_body.v*m_maxAllowedStep;
			geom_contact *pcontacts;
			intersection_params ip;
			geom_world_data gwd0,gwd1;
			ip.bStopAtFirstTri = true;
			ip.time_interval = m_maxAllowedStep;

			BBox[isneg(-delta.x)].x += delta.x;
			BBox[isneg(-delta.y)].y += delta.y;
			BBox[isneg(-delta.z)].z += delta.z;
			CPhysicalEntity **pentlist;
			nents = m_pWorld->GetEntitiesAround(BBox[0],BBox[1], pentlist, ent_terrain|ent_static|ent_sleeping_rigid|ent_rigid, this);

			CBoxGeom boxgeom;
			box abox;
			abox.Basis.SetIdentity();
			abox.center.zero();
			abox.size = (m_BBox[1]-m_BBox[0])*0.5f;
			boxgeom.CreateBox(&abox);
			gwd0.offset = (m_BBox[0]+m_BBox[1])*0.5f;
			//(m_qrot*m_parts[i].q).getmatrix(pgwd0->R); //Q2M_IVO
			gwd0.R.SetIdentity();
			gwd0.v = m_body.v;

			for(ient=0;ient<nents;ient++) if (pentlist[ient]!=this) for(j=0;j<pentlist[ient]->m_nParts;j++) 
			if (pentlist[ient]->m_parts[j].flags & flagsCollider) {
				gwd1.offset = pentlist[ient]->m_pos + pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].pos;
				//(pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].q).getmatrix(gwd1.R); //Q2M_IVO
				gwd1.R = matrix3x3f(pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].q);
				gwd1.scale = pentlist[ient]->m_parts[j].scale;
				ip.bSweepTest = true;
				if (boxgeom.Intersect(pentlist[ient]->m_parts[j].pPhysGeomProxy->pGeom, &gwd0,&gwd1, &ip, pcontacts))
					goto hitsmth;
				// note that sweep check can return nothing of bbox is initially intersecting the obstacle
				ip.bSweepTest = false;
				if (boxgeom.Intersect(pentlist[ient]->m_parts[j].pPhysGeomProxy->pGeom, &gwd0,&gwd1, &ip, pcontacts))
					goto hitsmth;
			}
			bSkipSpeedCheck = true;
			hitsmth:;
		}
	}

	if (!bSkipSpeedCheck && time_interval*fabsf(vloc[i])>size[i]*0.7f)
		time_interval = size[i]*0.7f/fabsf(vloc[i]);

	return min(min(m_timeStepFull-m_timeStepPerformed,m_maxAllowedStep),time_interval);
}


void CRigidEntity::ArchiveContact(int idx)
{
	int i,iHist;
	vectorf v[2];
	for(i=0;i<2;i++)
		v[i] = m_pContacts[idx].pbody[i]->v+(m_pContacts[idx].pbody[i]->w^m_pContacts[idx].pt[0]-m_pContacts[idx].pbody[i]->pos);
	iHist = m_iCollHistory+1 & sizeof(m_CollHistory)/sizeof(m_CollHistory[0])-1;
	float vrel2 = (m_CollHistory[iHist].v[0]-m_CollHistory[iHist].v[1]).len2();

	if (m_CollHistory[iHist].age>0.3f || vrel2<sqr(2.0f) || (v[0]-v[1]).len2()>vrel2) {
		m_iCollHistory = iHist;
		m_CollHistory[m_iCollHistory].pt = m_pContacts[idx].pt[0];
		m_CollHistory[m_iCollHistory].n = m_pContacts[idx].n;
		for(i=0;i<2;i++) {
			m_CollHistory[m_iCollHistory].v[i] = v[i];
			m_CollHistory[m_iCollHistory].mass[i] = m_pContacts[idx].pbody[i]->M;
			m_CollHistory[m_iCollHistory].partid[i] = m_pContacts[idx].pent[i]->m_parts[m_pContacts[idx].ipart[i]].id;
			m_CollHistory[m_iCollHistory].idmat[i] = m_pContacts[idx].id[i];
		}
		m_CollHistory[m_iCollHistory].idCollider = m_pWorld->GetPhysicalEntityId(m_pContacts[idx].pent[1]);
		m_CollHistory[m_iCollHistory].age = 0;
	}
}


int CRigidEntity::GetContactCount(int nMaxPlaneContacts)
{
	if (m_flags & ref_use_simple_solver)
		return 0;
	int i,j,nContacts,nTotContacts;
	for(i=nTotContacts=0; i<m_nColliders; i++,nTotContacts+=min(nContacts,nMaxPlaneContacts))
		for(j=nContacts=0; j<NMASKBITS && getmask(j)<=m_pColliderContacts[i]; j++)
			nContacts += m_pColliderContacts[i]>>i & (m_pContacts[i].flags>>contact_2b_verified_log2^1) & 1;
	return nTotContacts;
}


int CRigidEntity::RegisterContacts(float time_interval,int nMaxPlaneContacts)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	int i,j,j0,bComplexCollider,bStable,nContacts,bUseAreaContact;
	vectorf sz = m_BBox[1]-m_BBox[0],n;
	float mindim=min(sz.x,min(sz.y,sz.z)), mindim1,dist,friction,penetration;
	masktype contact_mask;
	entity_contact *pContactLin,*pContactAng;
	RigidBody *pbody;
	m_body.Fcollision.zero();
	m_body.Tcollision.zero();
	m_bStable &= -((int)m_flags & ref_use_simple_solver)>>31;
	if (m_nPrevColliders!=m_nColliders)
		m_nRestMask = 0;
	m_nPrevColliders = m_nColliders;

	UpdateConstraints();

	for(i=0;i<m_nColliders;i++) {
		if (!(m_flags & ref_use_simple_solver)) {
			contact_mask = m_pColliderContacts[i];
			pbody=0; bComplexCollider=bStable=bUseAreaContact=0; friction=penetration=0;
			for(j=0;j<NMASKBITS && getmask(j)<=contact_mask;j++) if (contact_mask & getmask(j))
				if (m_pContacts[j].flags & contact_2b_verified)
					contact_mask &= ~getmask(j);
				else {
					if (pbody && m_pContacts[j].pbody[1]!=pbody)
						bComplexCollider = 1;
					j0 = j;
					pbody = m_pContacts[j].pbody[1];
					m_pContacts[j].flags &= ~contact_maintain_count;
					friction = max(friction, m_pContacts[j].friction);
					penetration = max(penetration, m_pContacts[j].penetration);
				}
			if (!bComplexCollider) {
				sz = m_pColliders[i]->m_BBox[1]-m_pColliders[i]->m_BBox[0];
				mindim1 = min(sz.x,min(sz.y,sz.z));
				bStable = CompactContactBlock(contact_mask, min(mindim,mindim1)*0.15f, nMaxPlaneContacts,nContacts, n,dist, m_body.pos,m_gravity);
				if (bStable && dist<min(mindim,mindim1)*0.05f) {
					pContactLin = (entity_contact*)AllocSolverTmpBuf(sizeof(entity_contact));
					pContactAng = (entity_contact*)AllocSolverTmpBuf(sizeof(entity_contact));
					if (pContactLin && pContactAng) {
						bUseAreaContact = 1;
						pContactLin->pent[0]=pContactAng->pent[0] = this;
						pContactLin->pent[1]=pContactAng->pent[1] = m_pColliders[i];
						pContactLin->pbody[0]=pContactAng->pbody[0] = &m_body;
						pContactLin->pbody[1]=pContactAng->pbody[1] = pbody;
						pContactLin->flags = contact_maintain_count+3;//max(3,(nContacts>>1)+1);
						pContactLin->pBounceCount = &pContactAng->iCount;
						pContactAng->flags = contact_angular|contact_constraint_1dof|1;
						pContactLin->friction = friction;
						pContactLin->pt[0] = m_body.pos;
						pContactLin->pt[1] = pbody->pos;
						pContactLin->n = pContactAng->n = m_pContacts[j0].n;
						pContactLin->vreq = pContactLin->n*min(m_pWorld->m_vars.maxUnprojVel, 
							max(0.0f,(penetration-m_pWorld->m_vars.maxContactGap))*m_pWorld->m_vars.unprojVelScale);
						pContactAng->vreq = m_pContacts[j0].penetration>0 ? m_pContacts[j0].nloc*10.0f : vectorf(zero);
						pContactLin->K.SetZero();
						pContactLin->K(0,0)=pContactLin->K(1,1)=pContactLin->K(2,2) = m_body.Minv+pbody->Minv;
						pContactAng->K = m_body.Iinv+pbody->Iinv;
					}
				}
				bStable += bStable & (m_pColliders[i]->IsAwake()^1);
			}
			m_bStable = max(m_bStable,bStable);
			for(j=0;j<NMASKBITS && getmask(j)<=contact_mask;j++) if (contact_mask & getmask(j)) {
				m_pContacts[j].pBounceCount = &pContactLin->iCount;
				m_pContacts[j].flags |= contact_maintain_count&-bUseAreaContact;
				RegisterContact(m_pContacts+j);
				ArchiveContact(j);
			}
			if (bUseAreaContact) {
				RegisterContact(pContactLin);
				RegisterContact(pContactAng);
			}
		}
		for(j=0;j<NMASKBITS && getmask(j)<=m_pColliderConstraints[i];j++) if (m_pColliderConstraints[i] & getmask(j) && m_pConstraintInfos[j].bActive)
			RegisterContact(m_pConstraints+j);
	}
	if (m_submergedFraction>0)
		g_bUsePreCG = false;
	return 1;
}


void CRigidEntity::UpdateContactsAfterStepBack(float time_interval)
{
	int i;
	masktype contact_mask;
	vectorf diff;

	for(i=0,contact_mask=0; i<m_nColliders; contact_mask|=m_pColliderContacts[i++]);
	for(i=0;i<NMASKBITS && getmask(i)<=contact_mask;i++) if (contact_mask & getmask(i)) {
		diff = m_pContacts[i].pbody[0]->q*m_pContacts[i].ptloc[0]+m_pContacts[i].pbody[0]->pos-
					 m_pContacts[i].pbody[1]->q*m_pContacts[i].ptloc[1]-m_pContacts[i].pbody[1]->pos;
		// leave contact point and ptloc unchanged, but update penetration and vreq
		//m_pContacts[i].ptloc[0] = (m_pContacts[i].pt[0]-m_pContacts[i].pbody[0]->pos)*m_pContacts[i].pbody[0]->q;
		//m_pContacts[i].ptloc[1] = (m_pContacts[i].pt[0]-m_pContacts[i].pbody[1]->pos)*m_pContacts[i].pbody[1]->q;
		if (m_pContacts[i].penetration>0) {
			m_pContacts[i].penetration = max(0.0f,m_pContacts[i].penetration-min(0.0f,m_pContacts[i].n*diff));
			m_pContacts[i].vreq = m_pContacts[i].n*min(1.2f,m_pContacts[i].penetration*10.0f);
		}
	}
}


void CRigidEntity::StepBack(float time_interval)
{
	m_body.pos = m_prevPos;
	m_body.q = m_prevq;
	m_body.v = m_prevv;
	m_body.w = m_prevw;

	matrix3x3f R = matrix3x3f(m_body.q);
	m_body.Iinv = R*m_body.Ibody_inv*R.T();
	m_body.P = m_body.v*m_body.M;
	m_body.L = m_body.q*(m_body.Ibody*(!m_body.q*m_body.w));
	m_qrot = m_body.q*m_body.qfb;
	m_pos = m_body.pos-m_qrot*m_body.offsfb;
	m_bSteppedBack = 1;
}


float CRigidEntity::GetDamping(float time_interval)
{
	float damping = max(m_nColliders ? m_damping : m_dampingFreefall,m_dampingEx);
	//if (!(m_flags & pef_fixed_damping) && m_nGroupSize>=m_pWorld->m_vars.nGroupDamping) 
	//	damping = max(damping,m_pWorld->m_vars.groupDamping);

	return max(0.0f,1.0f-damping*time_interval);
}


int CRigidEntity::Update(float time_interval, float damping)
{
	int i;
	masktype contacts_mask;
	float E,E_accum, Emin = m_bFloating && m_nColliders==0 ? m_EminWater : m_Emin;
	vectorf v_accum,w_accum,L_accum,pt[4];
	unsigned int collFlags;
	//m_nStickyContacts = m_nSlidingContacts = 0;
	m_nStepBackCount = (m_nStepBackCount&-m_bSteppedBack)+m_bSteppedBack;
	if (m_flags & ref_use_simple_solver)
		damping = max(0.9f,damping);

	for(i=0,contacts_mask=0,collFlags=0; i<m_nColliders; i++)	{
		contacts_mask |= m_pColliderContacts[i]; collFlags |= m_pColliders[i]->m_flags;
	}
	m_body.v*=damping; m_body.w*=damping; m_body.P*=damping; m_body.L*=damping;

	if (m_body.Eunproj>0) { // if the solver changed body position
		m_qrot = m_body.q*m_body.qfb;
		m_pos = m_body.pos-m_qrot*m_body.offsfb;
		ComputeBBox();
		m_pWorld->RepositionEntity(this,1);
	}
//m_Emin = 0;

	/*for(j=0;j<NMASKBITS && getmask(j)<=contacts_mask;j++) 
	if (contacts_mask & getmask(j) && m_pContacts[j].penetration==0 && !(m_pContacts[j].flags & contact_2b_verified)) {
		vrel = m_pContacts[j].vrel*m_pContacts[j].n;
		if (vrel<m_pWorld->m_vars.minSeparationSpeed && m_pContacts[j].penetration==0) {
			if (m_pContacts[j].Pspare>0 && (m_pContacts[j].vrel-m_pContacts[j].n*vrel).len2()<sqr(m_pWorld->m_vars.minSeparationSpeed)) {
				m_iStickyContacts[(m_nStickyContacts = m_nStickyContacts+1&7)-1&7] = j;
				pt[nRestContacts&3] = m_pContacts[j].pt[0];
				nRestContacts += m_pContacts[j].pent[1]->IsAwake()^1;
			} else
				m_iSlidingContacts[(m_nSlidingContacts = m_nSlidingContacts+1&7)-1&7] = j;
		}
	}

	if (nRestContacts>=3) {
		m_bAwake = 0; float sg = (pt[1]-pt[0]^pt[2]-pt[0])*m_gravity;
		if (isneg(((m_body.pos-pt[0]^pt[1]-pt[0])*m_gravity)*sg) & isneg(((m_body.pos-pt[1]^pt[2]-pt[1])*m_gravity)*sg) & 
				isneg(((m_body.pos-pt[2]^pt[0]-pt[2])*m_gravity)*sg)) 
		{	// check if center of mass projects into stable contacts triangle 
			m_iSimClass=1; m_pWorld->RepositionEntity(this,2);
			goto doneupdate;
		}
	} */
	
	m_vhist[m_iDynHist] = m_body.v;
	m_whist[m_iDynHist] = m_body.w;
	m_Lhist[m_iDynHist] = m_body.L;
	m_iDynHist = m_iDynHist+1 & sizeof(m_vhist)/sizeof(m_vhist[0])-1;

	m_minAwakeTime = max(m_minAwakeTime,0.0f)-time_interval;
	if (m_body.Minv>0) {
		for(i=0,v_accum.zero(),w_accum.zero(),L_accum.zero(); i<sizeof(m_vhist)/sizeof(m_vhist[0]); i++) {
			v_accum+=m_vhist[i]; w_accum+=m_whist[i]; L_accum+=m_Lhist[i];
		}
		E = (m_body.v.len2() + (m_body.L*m_body.w)*m_body.Minv)*0.5f + m_body.Eunproj*1000.0f;
		if (m_flags & ref_use_simple_solver)
			E = min(E,m_Estep);
		if (m_bStable<2 && g_bitcount[m_nRestMask&0xFF]+g_bitcount[m_nRestMask>>8&0xFF]+
				g_bitcount[m_nRestMask>>16&0xFF]+g_bitcount[m_nRestMask>>24&0xFF]<20)
			E_accum = (v_accum.len2() + (w_accum*L_accum)*m_body.Minv)*0.5f + m_body.Eunproj*1000.0f;
		else E_accum = E;

		if (m_bAwake) {
			if ((E<Emin || E_accum<Emin) && (m_nColliders || m_bFloating) && m_minAwakeTime<=0 && !(collFlags&ref_contact_overflow)) {
				/*if (!m_bFloating) {
					m_body.P.zero();m_body.L.zero(); m_body.v.zero();m_body.w.zero();
				}*/
				m_bAwake = 0;
			} else 
				m_nSleepFrames=0;
		} else {
			if (E_accum>Emin || collFlags&ref_contact_overflow) {
				m_bAwake = 1;	m_nSleepFrames = 0;
				if (m_iSimClass==1) {
					m_iSimClass=2; m_pWorld->RepositionEntity(this,2);
				}
			} else {
				if (!(m_bFloating | m_flags&ref_use_simple_solver)) {
					m_body.P.zero();m_body.L.zero(); m_body.v.zero();m_body.w.zero();
				}
				if (++m_nSleepFrames>=4 && m_iSimClass==2) {
					m_iSimClass=1; m_pWorld->RepositionEntity(this,2);
					m_nSleepFrames = 0;
				}
			}
		}
		m_nRestMask = (m_nRestMask<<1) | (m_bAwake^1);
	}
	m_bStable = 0;

	//doneupdate:
	if (m_pWorld->m_vars.bMultiplayer) {
		//m_pos = CompressPos(m_pos);
		//m_body.pos = m_pos+m_qrot*m_body.offsfb;
		m_qrot = CompressQuat(m_qrot);
		matrix3x3f R = matrix3x3f(m_body.q = m_qrot*!m_body.qfb);
		m_body.v = CompressVel(m_body.v,50.0f);
		m_body.w = CompressVel(m_body.w,20.0f);
		m_body.P = m_body.v*m_body.M;
		m_body.L = m_body.q*(m_body.Ibody*(m_body.w*m_body.q));
		m_body.Iinv = R*m_body.Ibody_inv*R.T();
		ComputeBBox();
		m_pWorld->RepositionEntity(this,1);

		m_iLastChecksum = m_iLastChecksum+1 & NCHECKSUMS-1;
		m_checksums[m_iLastChecksum].iPhysTime = m_pWorld->m_iTimePhysics;
		m_checksums[m_iLastChecksum].checksum = GetStateChecksum();
	}

	return (m_bAwake^1) | isneg(m_timeStepFull-m_timeStepPerformed-0.001f);
}


int CRigidEntity::GetStateSnapshot(CStream &stm, float time_back, int flags)
{
	if (flags & ssf_checksum_only) {
		stm.Write(false);
		stm.Write(GetStateChecksum());
	}	else {
		stm.Write(true);
		stm.WriteNumberInBits(GetSnapshotVersion(),4);
		stm.Write(m_pos);
		if (m_pWorld->m_vars.bMultiplayer) {
			WriteCompressedQuat(stm,m_qrot);
			WriteCompressedVel(stm,m_body.v,50.0f);
			WriteCompressedVel(stm,m_body.w,20.0f);
		} else {
			stm.WriteBits((BYTE*)&m_qrot,sizeof(m_qrot)*8);
			if (m_body.Minv>0) {
				stm.Write(m_body.P+m_Pext[0]);
				stm.Write(m_body.L+m_Lext[0]);	
			} else {
				stm.Write(m_body.v);
				stm.Write(m_body.w);
			}
		}
		if (m_Pext[1].len2()+m_Lext[1].len2()>0) {
			stm.Write(true);
			stm.Write(m_Pext[1]);
			stm.Write(m_Lext[1]);
		} else stm.Write(false);
		stm.Write(m_bAwake!=0);
		stm.Write(m_iSimClass>1);
		WriteContacts(stm,flags);
	}

	return 1;
}

int CRigidEntity::WriteContacts(CStream &stm,int flags)
{
	int i,j,imax;
	masktype contacts;
	for(i=0,contacts=0;i<m_nColliders;i++) contacts |= m_pColliderContacts[i];
	for(imax=j=0; imax<NMASKBITS && getmask(imax)<=contacts; imax++) 
		if (contacts&getmask(imax) && m_pContacts[imax].penetration==0 && ++j==11) // allow for maximum 10 contacts
			break;

	WritePacked(stm, m_nColliders);
	WritePacked(stm, m_nContactsAlloc);
	for(i=0;i<m_nColliders;i++) {
		WritePacked(stm, m_pWorld->GetPhysicalEntityId(m_pColliders[i])+1);
		for(j=0,contacts=0; j<imax; j++) if (m_pColliderContacts[i]&getmask(j) && m_pContacts[j].penetration==0)
			contacts |= getmask(j);
		//contacts = m_pColliderContacts[i];
		WritePacked(stm, contacts);
		WritePacked(stm, m_pColliderConstraints[i]);
		for(j=0;j<NMASKBITS;j++) if (contacts & getmask(j)) {
			stm.Write(m_pContacts[j].ptloc[0]);
			stm.Write(m_pContacts[j].ptloc[1]);
			stm.Write(m_pContacts[j].nloc);
			WritePacked(stm, m_pContacts[j].ipart[0]);
			WritePacked(stm, m_pContacts[j].ipart[1]);
			WritePacked(stm, m_pContacts[j].iPrim[0]);
			WritePacked(stm, m_pContacts[j].iPrim[1]);
			WritePacked(stm, m_pContacts[j].iFeature[0]);
			WritePacked(stm, m_pContacts[j].iFeature[1]);
			WritePacked(stm, m_pContacts[j].iNormal);
			stm.Write((m_pContacts[j].flags & contact_2b_verified)!=0);
			stm.Write(false);//m_pContacts[j].vsep>0);
			stm.Write(m_pContacts[j].penetration);
		}
	}

	return 1;
}

int CRigidEntity::SetStateFromSnapshot(CStream &stm, int flags)
{
	bool bnz;
	int ver=0;
	int iMiddle,iBound[2]={ m_iLastChecksum+1-NCHECKSUMS,m_iLastChecksum+1 };
	unsigned int checksum,checksum_hist;
	if (m_pWorld->m_iTimeSnapshot[2]>=m_checksums[iBound[0]&NCHECKSUMS-1].iPhysTime &&
			m_pWorld->m_iTimeSnapshot[2]<=m_checksums[iBound[1]-1&NCHECKSUMS-1].iPhysTime)
	{
		do {
			iMiddle = iBound[0]+iBound[1]>>1;
			iBound[isneg(m_pWorld->m_iTimeSnapshot[2]-m_checksums[iMiddle&NCHECKSUMS-1].iPhysTime)] = iMiddle;
		} while(iBound[1]>iBound[0]+1);
		checksum_hist = m_checksums[iBound[0]&NCHECKSUMS-1].checksum;
	} else
		checksum_hist = GetStateChecksum();

#ifdef _DEBUG
	if (m_pWorld->m_iTimeSnapshot[2]!=0) {
		if (m_bAwake && m_checksums[iBound[0]&NCHECKSUMS-1].iPhysTime!=m_pWorld->m_iTimeSnapshot[2])
			m_pWorld->m_pLog->Log("Rigid Entity: time not in list (%d, bounds %d-%d) (id %d)", m_pWorld->m_iTimeSnapshot[2],
				m_checksums[iBound[0]&NCHECKSUMS-1].iPhysTime,m_checksums[iBound[1]&NCHECKSUMS-1].iPhysTime,m_id);
		if(m_bAwake && (m_checksums[iBound[0]-1&NCHECKSUMS-1].iPhysTime==m_checksums[iBound[0]&NCHECKSUMS-1].iPhysTime ||
			 m_checksums[iBound[0]+1&NCHECKSUMS-1].iPhysTime==m_checksums[iBound[0]&NCHECKSUMS-1].iPhysTime))
			m_pWorld->m_pLog->Log("Rigid Entity: 2 same times in history (id %d)",m_id);
	}
#endif

	stm.Read(bnz);
	if (!bnz) {
		stm.Read(checksum);
		m_flags |= pef_checksum_received;
		if (!(flags & ssf_no_update)) {
			m_flags &= ~pef_checksum_outofsync;
			m_flags |= pef_checksum_outofsync & ~-iszero((int)(checksum-checksum_hist));
			//if (m_flags & pef_checksum_outofsync)
			//	m_pWorld->m_pLog->Log("\005Rigid Entity %s out of sync (id %d)",
			//		m_pWorld->m_pPhysicsStreamer->GetForeignName(m_pForeignData,m_iForeignData,m_iForeignFlags), m_id);
		}
		return 2;
	}	else {
		m_flags = m_flags & ~pef_checksum_received;
		stm.ReadNumberInBits(ver,4);
		if (ver!=GetSnapshotVersion())
			return 0;

		if (!(flags & ssf_no_update)) {
			m_flags &= ~pef_checksum_outofsync;
			stm.Read(m_pos);
			if (m_pWorld->m_vars.bMultiplayer) {
				ReadCompressedQuat(stm,m_qrot);
				m_body.pos = m_pos+m_qrot*m_body.offsfb;
				matrix3x3f R = matrix3x3f(m_body.q = m_qrot*!m_body.qfb);
				ReadCompressedVel(stm,m_body.v,50.0f);
				ReadCompressedVel(stm,m_body.w,20.0f);
				m_body.P = m_body.v*m_body.M;
				m_body.L = m_body.q*(m_body.Ibody*(m_body.w*m_body.q));
				m_body.Iinv = R*m_body.Ibody_inv*R.T();
			} else {
				stm.ReadBits((BYTE*)&m_qrot,sizeof(m_qrot)*8);
				if (m_body.Minv>0) {
					stm.Read(m_body.P);
					stm.Read(m_body.L);
				} else {
					stm.Read(m_body.v);
					stm.Read(m_body.w);
				}
				m_body.pos = m_pos+m_qrot*m_body.offsfb;
				m_body.q = m_qrot*!m_body.qfb;
				m_body.UpdateState();
			}
			m_Pext[0].zero(); m_Lext[0].zero();
			stm.Read(bnz); if (bnz) {
				stm.Read(m_Pext[1]);
				stm.Read(m_Lext[1]);
			}
			stm.Read(bnz); m_bAwake = bnz ? 1:0;
			stm.Read(bnz); m_iSimClass = bnz ? 2:1;
			ComputeBBox();
			m_pWorld->RepositionEntity(this);

			m_iLastChecksum = iBound[0]+sgn(m_pWorld->m_iTimeSnapshot[2]-m_checksums[iBound[0]&NCHECKSUMS-1].iPhysTime) & NCHECKSUMS-1;
			m_checksums[m_iLastChecksum].iPhysTime = m_pWorld->m_iTimeSnapshot[2];
			m_checksums[m_iLastChecksum].checksum = GetStateChecksum();
		} else {
			stm.Seek(stm.GetReadPos()+sizeof(vectorf)*8 + 
				(m_pWorld->m_vars.bMultiplayer ? CMP_QUAT_SZ+CMP_VEL_SZ*2 : (sizeof(quaternionf)+2*sizeof(vectorf))*8));
			stm.Read(bnz); if (bnz)
				stm.Seek(stm.GetReadPos()+2*sizeof(vectorf)*8);
			stm.Seek(stm.GetReadPos()+2);
		}

		ReadContacts(stm, flags);
	}

	return 1;
}

int CRigidEntity::ReadContacts(CStream &stm, int flags)
{
	int i,j,id; bool bnz;
	pe_status_id si;
	int idPrevColliders[16],nPrevColliders=0;
	masktype iPrevConstraints[16],dummy;

	if (!(flags & ssf_no_update)) {
		for(i=0;i<m_nColliders;i++) {
			if (m_pColliderConstraints[i] && nPrevColliders<16) {
				idPrevColliders[nPrevColliders] = m_pWorld->GetPhysicalEntityId(m_pColliders[i]);
				iPrevConstraints[nPrevColliders++] = m_pColliderConstraints[i];
			}
			if (m_pColliders[i]!=this) {
				m_pColliders[i]->RemoveCollider(this);
				m_pColliders[i]->Release();
			}
		}

		ReadPacked(stm,m_nColliders);
		if (m_nCollidersAlloc<m_nColliders) {
			if (m_pColliders) delete[] m_pColliders;
			if (m_pColliderContacts) delete[] m_pColliderContacts;
			m_nCollidersAlloc = (m_nColliders-1&~7)+8;
			m_pColliders = new CPhysicalEntity*[m_nCollidersAlloc];
			m_pColliderContacts = new masktype[m_nCollidersAlloc];
			memset(m_pColliderConstraints = new masktype[m_nCollidersAlloc],0,m_nCollidersAlloc*sizeof(m_pColliderConstraints[0]));
		}
		int nContactsAlloc;
		ReadPacked(stm, nContactsAlloc);
		if (m_nContactsAlloc<nContactsAlloc) {
			if (m_pContacts) delete[] m_pContacts;
			m_pContacts = new entity_contact[nContactsAlloc];
		}
		m_nContactsAlloc = nContactsAlloc;

		for(i=0;i<m_nColliders;i++) {
			ReadPacked(stm,id); --id;
			m_pColliders[i] = (CPhysicalEntity*)id;
			ReadPacked(stm, m_pColliderContacts[i]);
			ReadPacked(stm, dummy);// m_pColliderConstraints[i]);
			m_pColliderConstraints[i] = 0;
			for(j=0;j<NMASKBITS;j++) if (m_pColliderContacts[i]&getmask(j)) {
				stm.Read(m_pContacts[j].ptloc[0]);
				stm.Read(m_pContacts[j].ptloc[1]);
				stm.Read(m_pContacts[j].nloc);
				ReadPacked(stm, m_pContacts[j].ipart[0]);
				ReadPacked(stm, m_pContacts[j].ipart[1]);
				ReadPacked(stm, m_pContacts[j].iPrim[0]);
				ReadPacked(stm, m_pContacts[j].iPrim[1]);
				ReadPacked(stm, m_pContacts[j].iFeature[0]);
				ReadPacked(stm, m_pContacts[j].iFeature[1]);
				ReadPacked(stm, m_pContacts[j].iNormal);
				stm.Read(bnz); m_pContacts[j].flags = bnz ? contact_2b_verified : 0;
				stm.Read(bnz); //m_pContacts[j].vsep = bnz ? m_pWorld->m_vars.minSeparationSpeed : 0;
				stm.Read(m_pContacts[j].penetration);

				m_pContacts[j].pent[0] = this;
				m_pContacts[j].pent[1] = (CPhysicalEntity*)id;
				m_pContacts[j].pbody[0] = m_pContacts[j].pent[0]->GetRigidBody(m_pContacts[j].ipart[0]);
				si.ipart = m_pContacts[j].ipart[0];
				si.iPrim = m_pContacts[j].iPrim[0];
				si.iFeature = m_pContacts[j].iFeature[0];
				m_pContacts[j].pent[0]->GetStatus(&si);
				m_pContacts[j].id[0] = si.id;
				//m_pContacts[j].penetration = 0;
				m_pContacts[j].pt[0] = m_pContacts[j].pt[1] = 
					m_pContacts[j].pbody[0]->q*m_pContacts[j].ptloc[0]+m_pContacts[j].pbody[0]->pos;
			}
		}
		for(i=0;i<nPrevColliders;i++)	{
			for(j=0;j<m_nColliders && (int)m_pColliders[j]!=idPrevColliders[i];j++);
			if (j<m_nColliders)
				m_pColliderConstraints[j] = iPrevConstraints[i];
		}
		m_bJustLoaded = m_nColliders;

		m_nColliders = 0; // so that no1 thinks we have colliders until postload is called
		if (!m_nParts && m_bJustLoaded) {
			m_pWorld->m_pLog->Log("\002Error: Rigid Body (@%.1f,%.1f,%.1f; id %d) lost its geometry after loading",m_pos.x,m_pos.y,m_pos.z,m_id);
			m_bJustLoaded = 0;
		}
	} else {
		int nColliders;
		masktype iContacts;
		ReadPacked(stm,nColliders); ReadPacked(stm,id);
		for(i=0;i<nColliders;i++) {
			ReadPacked(stm,id); ReadPacked(stm,iContacts); ReadPacked(stm,dummy);
			for(j=0;j<NMASKBITS;j++) if (iContacts&getmask(j)) {
				stm.Seek(stm.GetReadPos()+3*sizeof(vectorf)*8);
				ReadPacked(stm,id);ReadPacked(stm,id);ReadPacked(stm,id);ReadPacked(stm,id);
				ReadPacked(stm,id);ReadPacked(stm,id);ReadPacked(stm,id);
				stm.Seek(stm.GetReadPos()+2+sizeof(float)*8);
			}
		}
	}

	return 1;
}


int CRigidEntity::PostSetStateFromSnapshot()
{
	if (m_bJustLoaded) {
		pe_status_id si;
		int i,j;
		masktype contact_mask;
		m_nColliders = m_bJustLoaded;
		m_bJustLoaded = 0;

		for(i=contact_mask=0;i<m_nColliders;i++) contact_mask|=m_pColliderContacts[i];
		for(i=0;i<NMASKBITS;i++) if (contact_mask & getmask(i)) {
			m_pContacts[i].pent[1] = (CPhysicalEntity*)m_pWorld->GetPhysicalEntityById((int)m_pContacts[i].pent[1]);
			if (m_pContacts[i].pent[1] && (unsigned int)m_pContacts[i].ipart[0]<(unsigned int)m_pContacts[i].pent[0]->m_nParts &&
					(unsigned int)m_pContacts[i].ipart[1]<(unsigned int)m_pContacts[i].pent[1]->m_nParts) 
			{
				m_pContacts[i].pbody[1] = m_pContacts[i].pent[1]->GetRigidBody(m_pContacts[i].ipart[1]);
				si.ipart = m_pContacts[i].ipart[1];
				si.iPrim = m_pContacts[i].iPrim[1];
				si.iFeature = m_pContacts[i].iFeature[1];
				if (m_pContacts[i].pent[1]->GetStatus(&si)) {
					m_pContacts[i].id[1] = si.id;
					m_pContacts[i].n = m_pContacts[i].pbody[m_pContacts[i].iNormal]->q*m_pContacts[i].nloc;
					m_pContacts[i].vreq = m_pContacts[i].n*min(1.2f,m_pContacts[i].penetration*10.0f);
					m_pContacts[i].K.SetZero();
					m_pContacts[i].pent[0]->GetContactMatrix(m_pContacts[i].pt[0], m_pContacts[i].ipart[0], m_pContacts[i].K);
					m_pContacts[i].pent[1]->GetContactMatrix(m_pContacts[i].pt[1], m_pContacts[i].ipart[1], m_pContacts[i].K);
					// assume that the contact is static here, for if it's dynamic, VerifyContacts will update it before passing to the solver
					m_pContacts[i].vrel.zero();
					m_pContacts[i].friction = 0.5f*max(0.0f, 
						m_pWorld->m_FrictionTable[m_pContacts[i].id[0]&NSURFACETYPES-1] + m_pWorld->m_FrictionTable[m_pContacts[i].id[1]&NSURFACETYPES-1]);
				}	else
					contact_mask &= ~getmask(i);
			} else
				contact_mask &= ~getmask(i);
		}

		for(i=m_nColliders-1;i>=0;i--) {
			m_pColliderContacts[i] &= contact_mask;
			if (!(m_pColliders[i] = (CPhysicalEntity*)m_pWorld->GetPhysicalEntityById((int)m_pColliders[i])) || 
					!(m_pColliderContacts[i]|m_pColliderConstraints[i])) 
			{
				for(j=i;j<m_nColliders;j++) {
					m_pColliders[j] = m_pColliders[j+1];
					m_pColliderContacts[j] = m_pColliderContacts[j+1];
					m_pColliderConstraints[j] = m_pColliderConstraints[j+1];
				}
				m_nColliders--;
			}
		}
		for(i=0;i<m_nColliders;i++)
			m_pColliders[i]->AddRef();
	}
	return 1;
}


unsigned int CRigidEntity::GetStateChecksum()
{
	return //*(unsigned int*)&m_testval;
		float2int(m_pos.x*1024)^float2int(m_pos.y*1024)^float2int(m_pos.z*1024)^
		float2int(m_qrot.v.x*1024)^float2int(m_qrot.v.y*1024)^float2int(m_qrot.v.z*1024)^
		float2int(m_body.v.x*1024)^float2int(m_body.v.y*1024)^float2int(m_body.v.z*1024)^
		float2int(m_body.w.x*1024)^float2int(m_body.w.y*1024)^float2int(m_body.w.z*1024);
}


void CRigidEntity::ApplyBuoyancy(float time_interval,const vectorf& gravity)
{
	float maxWaterResistance2 = m_maxWaterResistance2;
	m_maxWaterResistance2 = 0;
	m_submergedFraction = 0;
	if (m_waterDensity==0 || m_body.Minv==0)
		return;
	vectorf sz,center,dP; float r,dist;
	RigidBody *pbody;
	sz = (m_BBox[1]-m_BBox[0])*0.5f; center = (m_BBox[1]+m_BBox[0])*0.5f;
	r = fabsf(m_waterPlane.n.x*sz.x)+fabsf(m_waterPlane.n.y*sz.y)+fabsf(m_waterPlane.n.z*sz.z);
	dist = (center-m_waterPlane.origin)*m_waterPlane.n;
	if (dist>r)
		return;

	pe_action_impulse ai;
	float V,Vsubmerged=0,Vfull=0;
	geom_world_data gwd;
	ai.iSource = 1;
	ai.iApplyTime = 0;
	m_bFloating = 0;

	for(int i=0;i<m_nParts;i++) if (m_parts[i].flags & geom_floats) {
		//(m_qrot*m_parts[i].q).getmatrix(gwd.R);	//Q2M_IVO
		gwd.R = matrix3x3f(m_qrot*m_parts[i].q);
		gwd.offset = m_pos + m_qrot*m_parts[i].pos;
		gwd.scale = m_parts[i].scale;
		pbody = GetRigidBody(i);
		gwd.v = pbody->v-m_waterFlow; gwd.w = pbody->w; gwd.centerOfMass = pbody->pos;

		if (m_waterResistance>0) {
			m_parts[i].pPhysGeomProxy->pGeom->CalculateMediumResistance(&m_waterPlane,&gwd,ai.impulse,ai.momentum);
			ai.impulse *= m_waterResistance*time_interval; ai.momentum *= m_waterResistance*time_interval;
			m_maxWaterResistance2 = max(maxWaterResistance2, ai.impulse.len2());
		} else {
			if (m_waterFlow.len2()>0)
				ai.impulse = (m_waterFlow-pbody->v)*((sz.x*sz.y+sz.x*sz.z+sz.y*sz.z)*m_waterDensity);
			else
				ai.impulse.zero();
			ai.momentum.zero();
		}

		if (dist>-r) {
			V = m_parts[i].pPhysGeomProxy->pGeom->CalculateBuoyancy(&m_waterPlane,&gwd,center);
			dP = gravity*(-m_waterDensity*V*time_interval);
			ai.impulse += dP; ai.momentum += center-pbody->pos^dP;
			m_bFloating = 1;
		} else {
			V = m_parts[i].pPhysGeomProxy->V*cube(m_parts[i].scale);
			ai.impulse -= gravity*(m_waterDensity*V*time_interval);
		}

		ai.ipart = i;
		Action(&ai);
		Vsubmerged += V; Vfull += m_parts[i].pPhysGeomProxy->V;
	}
	if (Vfull>0) {
		m_submergedFraction = Vsubmerged/Vfull;
		m_dampingEx = max(m_dampingEx, m_damping*(1-m_submergedFraction)+m_waterDamping*m_submergedFraction);
		if (Vsubmerged>0 && m_body.M<m_waterDensity*m_body.V)
			m_bFloating = 1;
	}
}


static inline void swap(edgeitem *plist, int i1,int i2) {
	int ti=plist[i1].idx; plist[i1].idx=plist[i2].idx; plist[i2].idx=ti;
}
static void qsort(edgeitem *plist, int left,int right)
{
	if (left>=right) return;
	int i,last; 
	swap(plist, left, left+right>>1);
	for(last=left,i=left+1; i<=right; i++)
	if (plist[plist[i].idx].area < plist[plist[left].idx].area)
		swap(plist, ++last, i);
	swap(plist, left,last);

	qsort(plist, left,last-1);
	qsort(plist, last+1,right);
}

int CRigidEntity::CompactContactBlock(masktype &contact_mask, float maxPlaneDist, int nMaxContacts,int &nContacts, 
																			vectorf &n,float &maxDist, const vectorf &ptTest, const vectorf &dirTest)
{
	int i,nEdges;
	matrix3x3f C,Ctmp;
	vectorf pt,p_avg,center,axes[2];
	float detabs[3],det;
	ptitem2d pts[NMASKBITS];
	edgeitem edges[NMASKBITS],*pedge,*pminedge,*pnewedge;

	for(i=nContacts=0,p_avg.zero(); i<NMASKBITS && getmask(i)<=contact_mask; i++) if (contact_mask & getmask(i)) {
		p_avg += m_pContacts[i].pt[0]; pts[nContacts++].iContact = i;
	}
	if (nContacts<=nMaxContacts && dirTest.len2()==0)
		return 0;

	center = p_avg/nContacts;
	for(i=0,C.SetZero(); i<nContacts; i++) // while it's possible to calculate C and center in one pass, it will lose precision
		C += dotproduct_matrix(m_pContacts[pts[i].iContact].pt[0]-center,m_pContacts[pts[i].iContact].pt[0]-center,Ctmp);
	detabs[0] = fabsf(C(1,1)*C(2,2)-C(2,1)*C(1,2));
	detabs[1] = fabsf(C(2,2)*C(0,0)-C(0,2)*C(2,0));
	detabs[2] = fabsf(C(0,0)*C(1,1)-C(1,0)*C(0,1));
	i = idxmax3(detabs);
	if (detabs[i]<1E-8f)
		return 0;
	det = 1.0f/(C(inc_mod3[i],inc_mod3[i])*C(dec_mod3[i],dec_mod3[i])-C(dec_mod3[i],inc_mod3[i])*C(inc_mod3[i],dec_mod3[i]));
	n[i] = 1;
	n[inc_mod3[i]] = -(C(inc_mod3[i],i)*C(dec_mod3[i],dec_mod3[i]) - C(dec_mod3[i],i)*C(inc_mod3[i],dec_mod3[i]))*det;
	n[dec_mod3[i]] = -(C(inc_mod3[i],inc_mod3[i])*C(dec_mod3[i],i) - C(dec_mod3[i],inc_mod3[i])*C(inc_mod3[i],i))*det;
	n.normalize();
	//axes[0]=n.orthogonal(); axes[1]=n^axes[0];
	axes[0].SetOrthogonal(n); axes[1]=n^axes[0];
	for(i=0,maxDist=0; i<nContacts; i++) {
		pt = m_pContacts[pts[i].iContact].pt[0]-center;
		maxDist = max(maxDist, fabsf(pt*n));
		pts[i].pt.set(pt*axes[0],pt*axes[1]);
	}
	if (maxDist>maxPlaneDist)
		return 0;
	
	nEdges = qhull2d(pts,nContacts,edges);

	if (nMaxContacts) { // chop off vertices until we have only the required amount left
		vector2df edge[2];
		for(i=0;i<nEdges;i++)	{
			edge[0] = edges[i].prev->pvtx->pt-edges[i].pvtx->pt; edge[1] = edges[i].next->pvtx->pt-edges[i].pvtx->pt;
			edges[i].area = edge[1] ^ edge[0]; edges[i].areanorm2 = edge[0].len2()*edge[1].len2();
			edges[i].idx = i;
		}
		// sort edges by the area of triangles
		qsort(edges,0,nEdges-1);
		// transform sorted array into a linked list
		pminedge = edges+edges[0].idx;
		pminedge->next1=pminedge->prev1 = edges+edges[0].idx; 
		for(pedge=pminedge,i=1; i<nEdges; i++) {
			edges[edges[i].idx].prev1 = pedge; edges[edges[i].idx].next1 = pedge->next1;
			pedge->next1->prev1 = edges+edges[i].idx; pedge = pedge->next1 = edges+edges[i].idx;
		}
		
		while(nEdges>2 && (nEdges>nMaxContacts || sqr(pminedge->area)<sqr(0.15f)*pminedge->areanorm2)) {
			edgeitem *pedge[2];
			// delete the ith vertex with the minimum area triangle
			pminedge->next->prev = pminedge->prev; pminedge->prev->next = pminedge->next;
			pminedge->next1->prev1 = pminedge->prev1; pminedge->prev1->next1 = pminedge->next1;
			pedge[0] = pminedge->prev; pedge[1] = pminedge->next; pminedge = pminedge->next1;
			nEdges--;
			for(i=0;i<2;i++) {
				edge[0] = pedge[i]->prev->pvtx->pt-pedge[i]->pvtx->pt; edge[1] = pedge[i]->next->pvtx->pt-pedge[i]->pvtx->pt;
				pedge[i]->area = edge[1]^edge[0]; pedge[i]->areanorm2 = edge[0].len2()*edge[1].len2(); // update area
				for(pnewedge=pedge[i]->next1; pnewedge!=pminedge && pnewedge->area<pedge[i]->area; pnewedge=pnewedge->next1);
				if (pedge[i]->next1!=pnewedge) { // re-insert pedge[i] before pnewedge
					if (pedge[i]==pminedge) pminedge = pedge[i]->next1;
					if (pedge[i]!=pnewedge) {
						pedge[i]->next1->prev1 = pedge[i]->prev1; pedge[i]->prev1->next1 = pedge[i]->next1;	
						pedge[i]->prev1 = pnewedge->prev1; pedge[i]->next1 = pnewedge;
						pnewedge->prev1->next1 = pedge[i]; pnewedge->prev1 = pedge[i];
					}
				}	else {
					for(pnewedge=pedge[i]->prev1; pnewedge->next1!=pminedge && pnewedge->area>pedge[i]->area; pnewedge=pnewedge->prev1);
					if (pedge[i]->prev1!=pnewedge) { // re-insert pedge[i] after pnewedge
						if (pnewedge->next1==pminedge) pminedge = pedge[i];
						if (pedge[i]!=pnewedge) {
							pedge[i]->next1->prev1 = pedge[i]->prev1; pedge[i]->prev1->next1 = pedge[i]->next1;	
							pedge[i]->prev1 = pnewedge; pedge[i]->next1 = pnewedge->next1;
							pnewedge->next1->prev1 = pedge[i]; pnewedge->next1 = pedge[i];
						}
					}
				}
			}
		}
	} else
		pminedge = edges;

	for(i=nContacts=0,pedge=pminedge,contact_mask=0; i<nEdges; i++,nContacts++,pedge=pedge->next)
		contact_mask |= getmask(pedge->pvtx->iContact);

	if (dirTest.len2()>0) {
		if (nEdges<3 || sqr(det = dirTest*n)<dirTest.len2()*0.001f)
			return 0;
		pt = ptTest-center+dirTest*(((center-ptTest)*n)/det);
		vector2df pt2d(axes[0]*pt,axes[1]*pt);
		for(i=0; i<nEdges; i++,pminedge=pminedge->next) 
		if ((pt2d-pminedge->pvtx->pt ^ pminedge->next->pvtx->pt-pminedge->pvtx->pt)>0)
			return 0;
	}

	return 1;
}


void CRigidEntity::DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags)
{
	CPhysicalEntity::DrawHelperInformation(DrawLineFunc, flags);

	if (flags & pe_helper_collisions) {
		int i;
		masktype contacts_mask=0;
		for(i=0;i<m_nColliders;i++) contacts_mask |= m_pColliderContacts[i];
		for(i=0;i<NMASKBITS;i++) if (contacts_mask & getmask(i))
			DrawLineFunc(m_pContacts[i].pt[0], m_pContacts[i].pt[0] + m_pContacts[i].n*m_pWorld->m_vars.maxContactGap*30);
	}
}


void CRigidEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	CPhysicalEntity::GetMemoryStatistics(pSizer);
	if (GetType()==PE_RIGID)
		pSizer->AddObject(this, sizeof(CRigidEntity));
	pSizer->AddObject(m_pColliderContacts, m_nCollidersAlloc*sizeof(m_pColliderContacts[0]));
	pSizer->AddObject(m_pColliderConstraints, m_nCollidersAlloc*sizeof(m_pColliderConstraints[0]));
	pSizer->AddObject(m_pContacts, m_nContactsAlloc*sizeof(m_pContacts[0]));
	pSizer->AddObject(m_pConstraints, m_nConstraintsAlloc*sizeof(m_pConstraints[0]));
	pSizer->AddObject(m_pConstraintInfos, m_nConstraintsAlloc*sizeof(m_pConstraintInfos[0]));
}