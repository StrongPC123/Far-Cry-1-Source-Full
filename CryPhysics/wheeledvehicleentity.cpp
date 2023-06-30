//////////////////////////////////////////////////////////////////////
//
//	Vehicle Entity
//	
//	File: wheeledvehicleentity.cpp
//	Description : CWheeledVehicleEntity class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "geoman.h"
#include "physicalworld.h"
#include "rigidentity.h"
#include "wheeledvehicleentity.h"


CWheeledVehicleEntity::CWheeledVehicleEntity(CPhysicalWorld *pworld) : CRigidEntity(pworld)
{
	m_axleFriction = 0;
	m_engineMaxw = 120;
	m_enginePower = 10000;
	m_enginePedal = 0;
	m_bHandBrake = 1;
	m_nHullParts = 0;
  m_maxAllowedStepRigid = m_maxAllowedStep;
	m_maxAllowedStepVehicle = 0.01f;
	m_steer = 0;
	m_maxSteer = pi/4;
	m_iIntegrationType = 1;
	m_EminVehicle = sqr(0.05f);
	m_EminRigid = m_Emin;
	m_Ffriction.zero();
	m_Tfriction.zero();
	m_dampingVehicle = 0.2f;
	m_bCollisionCulling = 1;
	m_body.pOwner = this;
	m_nContacts = m_bHasContacts = 0;
	m_flags |= pef_fixed_damping;
	m_timeNoContacts = 10.0f;
	m_maxBrakingFriction = 1.0f;
	m_clutch = 0;
	m_clutchSpeed = 1.0f;
	m_nGears = 2;
	m_iCurGear = 1;
	m_gears[0] = -1.0f;
	m_gears[1] = 1.0f;
	m_engineShiftUpw = m_engineMaxw*0.5f;
	m_engineShiftDownw = m_engineMaxw*0.2f;
	m_wengine = m_engineIdlew = m_engineMaxw*0.1f;
	m_engineMinw = m_engineMaxw*0.05f;
	m_gearDirSwitchw = 1.0f;
	m_kDynFriction = 1.0f;
	m_slipThreshold = 0.05f;
	m_engineStartw = 40;
	m_brakeTorque = 4000.0f;
}


int CWheeledVehicleEntity::SetParams(pe_params *_params)
{
	int res;
	if (res = CRigidEntity::SetParams(_params)) {
		if (_params->type==pe_simulation_params::type_id) {
			pe_simulation_params *params = (pe_simulation_params*)_params;
			if (!is_unused(params->maxTimeStep)) m_maxAllowedStepRigid = params->maxTimeStep;
			if (!is_unused(params->minEnergy)) m_EminRigid = params->minEnergy;
		}
		return res;
	}
		
	if (_params->type==pe_params_car::type_id) {
		pe_params_car *params = (pe_params_car*)_params;
		if (!is_unused(params->axleFriction)) m_axleFriction = params->axleFriction;
		if (!is_unused(params->enginePower)) m_enginePower = params->enginePower;
		if (!is_unused(params->engineMaxRPM)) m_engineMaxw = params->engineMaxRPM*(2*pi/60.0);
		if (!is_unused(params->maxSteer)) m_maxSteer = params->maxSteer;
		if (!is_unused(params->iIntegrationType)) m_iIntegrationType = params->iIntegrationType;
		if (!is_unused(params->maxTimeStep)) m_maxAllowedStepVehicle = params->maxTimeStep;
		if (!is_unused(params->minEnergy)) m_EminVehicle = params->minEnergy;
		if (!is_unused(params->damping)) m_dampingVehicle = params->damping;
		if (!is_unused(params->maxBrakingFriction)) m_maxBrakingFriction = params->maxBrakingFriction;
		if (!is_unused(params->kStabilizer)) m_kStabilizer = params->kStabilizer;
		if (!is_unused(params->engineMinRPM)) m_engineMinw = params->engineMinRPM*(2*pi/60.0);
		if (!is_unused(params->engineIdleRPM))
			m_wengine = m_engineIdlew = params->engineIdleRPM*(2*pi/60.0);
		if (!is_unused(params->engineStartRPM)) m_engineStartw = params->engineStartRPM*(2*pi/60.0);
		if (!is_unused(params->engineShiftUpRPM)) m_engineShiftUpw = params->engineShiftUpRPM*(2*pi/60.0);
		if (!is_unused(params->engineShiftDownRPM)) m_engineShiftDownw = params->engineShiftDownRPM*(2*pi/60.0);
		if (!is_unused(params->engineIdleRPM)) m_engineIdlew = params->engineIdleRPM*(2*pi/60.0);
		if (!is_unused(params->clutchSpeed)) m_clutchSpeed = params->clutchSpeed;
		if (!is_unused(params->nGears)) m_nGears = params->nGears;
		if (!is_unused(params->gearRatios)) for(int i=0;i<m_nGears;i++) m_gears[i] = params->gearRatios[i];
		if (!is_unused(params->kDynFriction)) m_kDynFriction = params->kDynFriction;
		if (!is_unused(params->gearDirSwitchRPM)) m_gearDirSwitchw = params->gearDirSwitchRPM*(2*pi/60.0);
		if (!is_unused(params->slipThreshold)) m_slipThreshold = params->slipThreshold;
		if (!is_unused(params->brakeTorque)) m_brakeTorque = params->brakeTorque;
		return 1;
	}

	if (_params->type==pe_params_wheel::type_id) {
		pe_params_wheel *params = (pe_params_wheel*)_params;
        int iWheel = max(0,min(m_nParts-m_nHullParts-1,params->iWheel));
		if (!is_unused(params->bDriving)) m_susp[iWheel].bDriving = params->bDriving;
		if (!is_unused(params->iAxle)) m_susp[iWheel].iAxle = params->iAxle;
		if (!is_unused(params->suspLenMax)) m_susp[iWheel].fullen = params->suspLenMax;
		if (!is_unused(params->minFriction)) m_susp[iWheel].minFriction = params->minFriction;
		if (!is_unused(params->maxFriction)) m_susp[iWheel].maxFriction = params->maxFriction;
		if (!is_unused(params->surface_idx)) m_parts[m_nHullParts+iWheel].surface_idx = params->surface_idx;
		return 1;
	}

	return 0;
}


int CWheeledVehicleEntity::GetParams(pe_params *_params)
{
	if (_params->type==pe_params_car::type_id) {
		pe_params_car *params = (pe_params_car*)_params;
		params->axleFriction = m_axleFriction;
		params->enginePower = m_enginePower;
		params->engineMaxRPM = float2int(m_engineMaxw*(30.0/pi));
		params->maxSteer = m_maxSteer;
		params->iIntegrationType = m_iIntegrationType;
		params->maxTimeStep = m_maxAllowedStepVehicle;
		params->minEnergy = m_EminVehicle;
		params->damping = m_dampingVehicle;
		params->maxBrakingFriction = m_maxBrakingFriction;
		params->kStabilizer = m_kStabilizer;
		params->engineMinRPM = m_engineMinw*(60.0/(2*pi));
		params->engineIdleRPM = m_engineIdlew*(60.0/(2*pi));
		params->engineShiftUpRPM = m_engineShiftUpw*(60.0/(2*pi));
		params->engineShiftDownRPM = m_engineShiftDownw*(60.0/(2*pi));
		params->engineIdleRPM = m_engineIdlew*(60.0/(2*pi));
		params->engineStartRPM = m_engineStartw*(60.0/(2*pi));
		params->clutchSpeed = m_clutchSpeed;
		params->nGears = m_nGears;
		if (!is_unused(params->gearRatios))
			for(int i=0;i<m_nGears;i++) params->gearRatios[i] = m_gears[i];
		params->kDynFriction = m_kDynFriction;
		params->gearDirSwitchRPM = m_gearDirSwitchw*(60.0/(2*pi));
		params->slipThreshold = m_slipThreshold;
		params->brakeTorque = m_brakeTorque;
		params->nWheels = m_nParts-m_nHullParts;
		return 1;
	}

	if (_params->type==pe_params_wheel::type_id) {
		pe_params_wheel *params = (pe_params_wheel*)_params;
		int iWheel = min(0,max(m_nParts-m_nHullParts-1,params->iWheel));
		params->bDriving = m_susp[iWheel].bDriving;
		params->iAxle = m_susp[iWheel].iAxle;
		params->suspLenMax = m_susp[iWheel].fullen;
		params->minFriction = m_susp[iWheel].minFriction;
		params->maxFriction = m_susp[iWheel].maxFriction;
		params->surface_idx = m_parts[m_nHullParts+iWheel].surface_idx;
		return 1;
	}

	return CRigidEntity::GetParams(_params);
}


int CWheeledVehicleEntity::Action(pe_action *_action)
{
	int res;
	if (res = CRigidEntity::Action(_action)) {
		if (_action->type==pe_action_impulse::type_id && ((pe_action_impulse*)_action)->iSource!=1)
			m_minAwakeTime = max(m_minAwakeTime,3.0f);
		else if (_action->type==pe_action_reset::type_id) {
			for(int i=0;i<m_nParts-m_nHullParts;i++) 
				m_susp[i].w=m_susp[i].wa=m_susp[i].T=0;
			m_enginePedal = 0; m_timeNoContacts = 10.0f;
		}
		return res;
	}

	if (_action->type==pe_action_drive::type_id) {
		pe_action_drive *action = (pe_action_drive*)_action;
		if (!is_unused(action->dpedal)) m_enginePedal = min_safe(1.0f,max_safe(-1.0f,m_enginePedal+action->dpedal));
		if (!is_unused(action->pedal)) m_enginePedal = action->pedal;
		if (!is_unused(action->clutch)) m_clutch = action->clutch;
		if (!is_unused(action->iGear)) m_iCurGear = action->iGear;
		if (!is_unused(action->steer) || !is_unused(action->dsteer)) {
			int i,j;
			if (!is_unused(action->steer)) m_steer = action->steer;
			if (!is_unused(action->dsteer)) m_steer += action->dsteer;
			m_steer = min_safe(m_maxSteer,max_safe(-m_maxSteer,m_steer));
			if (m_pWorld->m_vars.bMultiplayer)
				m_steer = CompressFloat(m_steer,1.0f,12);
			if (m_steer!=0) {	// apply Ackerman steering to front wheels
				vectorf pt[2],rarax,cm=(m_body.pos-m_pos)*m_qrot;
				float ha,la,hb,lb,tgsteer=tan_tpl(m_steer);
				pt[0]=pt[1] = cm;
				for(i=0;i<m_nParts-m_nHullParts;i++) if (m_susp[i].pt.y < pt[j=isneg(cm.x-m_susp[i].pt.x)].y) 
					pt[j] = m_susp[i].pt;
				rarax = (pt[1]-pt[0]).normalized(); // rear wheels axis (pt[0,1] - rear wheels positions
				for(i=0;i<m_nParts-m_nHullParts;i++) if (m_susp[i].pt.y>cm.y && (m_susp[i].pt.x-cm.x)*m_steer>0) {
					m_susp[i].steer = m_steer; 
					ha = (m_susp[i].pt-pt[0]^rarax).len();
					la = (m_susp[i].pt-pt[0])*rarax;
					for(j=0;j<m_nParts-m_nHullParts;j++) if (fabsf(m_susp[j].pt.y-m_susp[i].pt.y)<ha*0.05f && (m_susp[j].pt.x-cm.x)*m_steer<0) {
						hb = (m_susp[j].pt-pt[0]^rarax).len();
						lb = (m_susp[j].pt-pt[0])*rarax;
						m_susp[j].steer = atan_tpl(hb*tgsteer/(ha+fabs((lb-la)*tgsteer)));
					}
				}
			} else for(i=0;i<m_nParts-m_nHullParts;i++) m_susp[i].steer=0;
			UpdateWheelsGeoms();
		}
		int bPrevHandBrake = m_bHandBrake;
		if (!is_unused(action->bHandBrake))
			m_bHandBrake = action->bHandBrake;
		if (m_bHandBrake)
			m_enginePedal = 0;
		if ((m_enginePedal!=0 || !m_bHandBrake && bPrevHandBrake) && !m_bAwake) {
			m_bAwake=1;	m_minAwakeTime = max(m_minAwakeTime,1.0f);
			if (m_iSimClass<2) {
				m_iSimClass = 2; m_pWorld->RepositionEntity(this, 2);
			}
		}
	}

	return 0;
}


int CWheeledVehicleEntity::GetStatus(pe_status* _status)
{
	int res;
	if (_status->type==pe_status_pos::type_id) {
		pe_status_pos *status = (pe_status_pos*)_status;
		int iwheel,i;
		if (status->ipart==-1 && status->partid>=0) {
			for(i=0;i<m_nParts && m_parts[i].id!=status->partid;i++);
			if (i==m_nParts) return 0;
		} else
			i = status->ipart;

		vectorf prevpos,ptc1; 
		quaternionf prevq;
		iwheel = i-m_nHullParts;
		if ((unsigned int)iwheel<(unsigned int)m_nParts-m_nHullParts) {
			prevpos = m_parts[i].pos; prevq = m_parts[i].q;
			m_parts[i].q = 
				GetRotationAA(m_susp[iwheel].steer,vectorf(0,0,-1))*GetRotationAA(m_susp[iwheel].rot,vectorf(-1,0,0))*m_susp[iwheel].q0;
			ptc1 = m_parts[i].q*m_parts[i].pPhysGeomProxy->origin + m_susp[iwheel].pos0;
			(m_parts[i].pos = m_susp[iwheel].pos0+m_susp[iwheel].ptc0-ptc1).z -= m_susp[iwheel].curlen-m_susp[iwheel].len0;
		}

		res = CRigidEntity::GetStatus(_status);

		if ((unsigned int)iwheel<(unsigned int)m_nParts-m_nHullParts) {
			m_parts[i].pos = prevpos;	m_parts[i].q = prevq;
		}
		return res;
	}

	if (res = CRigidEntity::GetStatus(_status))
		return res;

	if (_status->type==pe_status_vehicle::type_id) {
		pe_status_vehicle *status = (pe_status_vehicle*)_status;
		status->steer = m_steer;
		status->pedal = m_enginePedal;
		status->bHandBrake = m_bHandBrake;
		status->footbrake = m_enginePedal*sgn(m_iCurGear-1)<=0 ? fabs_tpl(m_enginePedal) : 0;
		status->vel = m_body.v;
		int i; for(i=status->bWheelContact=0; i<m_nParts-m_nHullParts; i++)
			status->bWheelContact += m_susp[i].bContact;
		status->engineRPM = m_wengine*(60.0/(2*pi/60.0));
		status->iCurGear = m_iCurGear;
		status->clutch = m_clutch;
		for(i=status->nActiveColliders=0; i<m_nColliders; i++) 
			if (m_pColliders[i]->m_iSimClass>0 && m_pColliders[i]->GetType()!=PE_ARTICULATED)
				status->nActiveColliders++;
		return 1;
	}

	if (_status->type==pe_status_wheel::type_id) {
		pe_status_wheel *status = (pe_status_wheel*)_status;
		status->bContact = m_susp[status->iWheel].bContact;
		status->ptContact = m_susp[status->iWheel].ptcontact;
		status->w = m_susp[status->iWheel].w;
		status->bSlip = m_susp[status->iWheel].bSlip;
		status->velSlip = m_susp[status->iWheel].vrel;
		vectorf axis(cos_tpl(m_susp[status->iWheel].steer),-sin_tpl(m_susp[status->iWheel].steer),0), pulldir, ncontact;
		pulldir = m_qrot*(m_susp[status->iWheel].ncontact^axis).normalized();
		status->velSlip -= pulldir*(m_susp[status->iWheel].w*m_susp[status->iWheel].r);
		ncontact = m_qrot*m_susp[status->iWheel].ncontact;
		status->velSlip -= ncontact*(status->velSlip*ncontact);
		if (!m_bAwake) {
			status->w = 0;
			status->velSlip.zero();
		}
		status->contactSurfaceIdx = m_susp[status->iWheel].surface_idx[1];
		status->suspLen = m_susp[status->iWheel].curlen;
		status->suspLenFull = m_susp[status->iWheel].fullen;
		status->suspLen0 = m_susp[status->iWheel].len0;
		return 1;
	}

	if (_status->type==pe_status_vehicle_abilities::type_id) {
		pe_status_vehicle_abilities *status = (pe_status_vehicle_abilities*)_status;
		int i,nd;
		if (!is_unused(status->steer)) {
			vectorf pt[2],cm=(m_body.pos-m_pos)*m_qrot;
			for(i=0;i<m_nParts-m_nHullParts;i++) if ((m_susp[i].pt.x-cm.x)*status->steer>0)
				pt[isneg(cm.y-m_susp[i].pt.y)] = m_susp[i].pt;
			pt[0].x = pt[1].x+(pt[1].y-pt[0].y)/tan_tpl(status->steer)*sgn(status->steer);
			status->rotPivot = m_qrot*pt[0]+m_pos;
		}

		float k,wlim[2],w;
		wlim[0]=m_engineMaxw*0.01f; wlim[1]=m_engineMaxw;
		for(i=nd=0;i<m_nParts-m_nHullParts;i++) nd += m_susp[i].bDriving;
		if (nd>0) {
			k=pi/m_engineMaxw; i=0;
			do {
				w = (wlim[0]+wlim[1])*0.5f;
				wlim[isneg((sin_tpl(w*k)*m_enginePower-m_axleFriction*w)*nd - sqr(w*m_susp[0].r)*m_dampingVehicle*m_body.M)] = w;
			} while((wlim[1]-wlim[0])>m_engineMaxw*0.005f && ++i<256);
			status->maxVelocity = w*m_susp[0].r;
		} else
			status->maxVelocity = 0;
		return 1;
	}

	return 0;
}


void CWheeledVehicleEntity::RecalcSuspStiffness()
{
	vectorf cm=(m_body.pos-m_pos)*m_qrot, sz=(m_BBox[1]-m_BBox[0]);
	int i,j,idx,nl[2]={0,0}; float kdl[2],l[2]={0,0};
	float e = max(max(sz.x,sz.y),sz.z)*0.02f;

	for(i=0;i<m_nParts-m_nHullParts;i++) m_susp[i].iBuddy = -1;
	// first, force nearly symmetrical wheels to be symmetrical (if they are on the same axle)
	for(i=0;i<m_nParts-m_nHullParts;i++) for(j=i+1;j<m_nParts-m_nHullParts;j++) 
	if (m_susp[j].iAxle==m_susp[i].iAxle) {
		if (fabs_tpl(m_susp[i].pt.y-m_susp[j].pt.y)<e && fabs_tpl(m_susp[i].pt.x+m_susp[j].pt.x)<e) {
			m_susp[j].pt(-m_susp[i].pt.x, m_susp[i].pt.y, m_susp[i].pt.z);
			m_susp[j].ptc0(-m_susp[i].ptc0.x, m_susp[i].ptc0.y, m_susp[i].ptc0.z);
		}
		m_susp[i].iBuddy = j;	m_susp[j].iBuddy = i;
		break;
	}

	for(i=0;i<m_nParts-m_nHullParts;i++) {
		idx = isneg(m_susp[i].pt.y-cm.y);
		l[idx] += m_susp[i].pt.y-cm.y; nl[idx]++;
	}

	if (l[0]-l[1]>0 && nl[0]*nl[1]>0) {
		kdl[0] = l[1]*m_body.M*m_gravity.z/(l[0]*nl[1]-l[1]*nl[0]);
		kdl[1] = (m_body.M*-m_gravity.z-kdl[0]*nl[0])/nl[1];
		for(i=0;i<m_nParts-m_nHullParts;i++)
			m_susp[i].kStiffness = kdl[isneg(m_susp[i].pt.y-cm.y)];
	} else for(i=0;i<m_nParts-m_nHullParts;i++)
		m_susp[i].kStiffness = m_susp[i].Mpt*-m_gravity.z;			

	for(i=0;i<m_nParts-m_nHullParts;i++) {
		m_susp[i].kStiffness /= m_susp[i].fullen-m_susp[i].len0;
		if (m_susp[i].kDamping0<=0)
			m_susp[i].kDamping = -m_susp[i].kDamping0*sqrt_tpl(4*m_susp[i].kStiffness*m_susp[i].Mpt);
	}
}


int CWheeledVehicleEntity::AddGeometry(phys_geometry *pgeom, pe_geomparams *_params, int id)
{
	if (!pgeom)
		return 0;
	int res;
	if (_params->type!=pe_cargeomparams::type_id) {
		pe_cargeomparams params = *(pe_geomparams*)_params;
		return AddGeometry(pgeom,&params,id);
	}

	pe_cargeomparams *params = (pe_cargeomparams*)_params;
	float density=params->mass!=0 ? params->mass/pgeom->V : params->density;
	int flags0,flagsCollider0;
	if (!is_unused(params->bDriving)) {
		flags0 = params->flags; flagsCollider0 = params->flagsCollider;
		params->flags = geom_colltype_player|geom_colltype_ray;
		params->flagsCollider = 0;
		params->density = params->mass = 0;
	}

	int nPartsOld=m_nParts;
	if ((res = CRigidEntity::AddGeometry(pgeom,_params,id))<0)
		return res;

	if (params->bDriving<0) {
		if (m_nParts > nPartsOld) {
			if (m_nHullParts < m_nParts-1) {
				memcpy(&m_defpart, m_parts+m_nParts-1, sizeof(geom));
				memmove(m_parts+m_nHullParts+1, m_parts+m_nHullParts, sizeof(geom)*(m_nParts-m_nHullParts-1));
				memcpy(m_parts+m_nHullParts, &m_defpart, sizeof(geom));
			}
			m_nHullParts++;
		}
	} else {
		int idx = m_nParts-1-m_nHullParts;
		m_susp[idx].bDriving = params->bDriving;
		m_susp[idx].iAxle = params->iAxle;
		m_susp[idx].bCanBrake = params->bCanBrake;
		m_susp[idx].pt = params->pivot;
		m_susp[idx].fullen = params->lenMax;
		m_susp[idx].curlen = m_susp[idx].len0 = params->lenInitial;
		m_susp[idx].steer = 0;
		m_susp[idx].T=m_susp[idx].w=m_susp[idx].wa=m_susp[idx].rot=m_susp[idx].prevTdt=m_susp[idx].prevw=0;
		m_susp[idx].q0 = params->q;
		m_susp[idx].pos0 = params->pos;
		m_susp[idx].ptc0 = params->q*pgeom->origin + params->pos;
		m_susp[idx].bSlip = m_susp[idx].bSlipPull = 0;
		m_susp[idx].minFriction = is_unused(params->minFriction) ? 0.0f : params->minFriction;
		m_susp[idx].maxFriction = is_unused(params->maxFriction) ? 1.0f : params->maxFriction;
		m_susp[idx].pent = 0;
		m_susp[idx].bContact = 0;
		m_susp[idx].flags0 = flags0;
		m_susp[idx].flagsCollider0 = flagsCollider0;
		box bbox; pgeom->pGeom->GetBBox(&bbox);
		float diff,maxdiff=0;
		for(int i=0;i<3;i++) 
		if ((diff = min(fabs_tpl(bbox.size[i]-bbox.size[inc_mod3[i]]),fabs_tpl(bbox.size[i]-bbox.size[dec_mod3[i]])))>maxdiff)
		{ maxdiff=diff; m_susp[idx].r=bbox.size[inc_mod3[i]]; m_susp[idx].width=bbox.size[i]; }
		m_susp[idx].Iinv = 1.0f/(pgeom->Ibody.z*density);
		m_susp[idx].rinv = 1.0f/m_susp[idx].r;

		vectorf r = m_susp[idx].pt-m_body.pos+m_pos;
		matrix3x3f R,Iinv; 
		//(!m_qrot*m_body.q).getmatrix(R); //Q2M_IVO
		R = matrix3x3f(!m_qrot*m_body.q);
		Iinv = R*m_body.Ibody_inv*R.T();
		m_susp[idx].Mpt = 1.0f/(m_body.Minv+(Iinv*vectorf(r.y,-r.x,0)^r).z);

		m_susp[idx].kDamping0 = params->kDamping;
		if (params->kStiffness==0)
			RecalcSuspStiffness();
		else {
			m_susp[idx].kStiffness = params->kStiffness;
			m_susp[idx].kDamping = params->kDamping;
		}
		m_susp[idx].bSlip = 0;
	}

	return res;
}


void CWheeledVehicleEntity::RemoveGeometry(int id)
{
	int i; for(i=0;i<m_nParts && m_parts[i].id!=id;i++);
	if (i<m_nParts) {
		if (i<m_nHullParts) m_nHullParts--;
		else memmove(m_susp+i-m_nHullParts, m_susp+i-m_nHullParts+1, m_nParts-i-1);
	}

	CRigidEntity::RemoveGeometry(id);
}


void CWheeledVehicleEntity::ComputeBBox() 
{ 
	CPhysicalEntity::ComputeBBox(); // inflate the box a bit, because wheels attempt to maintain a safety gap from the ground
	vectorf infl; infl.x=infl.y=infl.z = m_pWorld->m_vars.maxContactGap*2;
	m_BBox[0] -= infl; m_BBox[1] += infl;
}


void CWheeledVehicleEntity::CheckAdditionalGeometry(float time_interval, masktype &contact_mask)
{
	int i,j,nents,ient,iwheel,ncontacts,icont,icoll;
	float tmax,tcur,N,vreq,newlen;
	vectorf r,ptc1,geompos,axis,ptcw;// axisx=m_qrot*vectorf(1,0,0);
	quaternionf qsteer;
	geom_world_data gwd[2];
	intersection_params ip;
	CPhysicalEntity **pentlist;
	geom_contact *pcontacts;
	pe_action_impulse ai;
	ai.iSource = 3;

	nents = m_pWorld->GetEntitiesAround(m_BBox[0],m_BBox[1], pentlist, ent_terrain|ent_static|ent_sleeping_rigid|ent_rigid|ent_triggers, this);
	for(i=j=0;i<nents;i++) 
	if (pentlist[i]!=this && (pentlist[i]->m_iSimClass<2 || pentlist[i]->GetMassInv()<m_body.Minv*10 || 
			pentlist[i]->m_nParts==1 && pentlist[i]->GetMassInv()<m_body.Minv*100))
		pentlist[j++] = pentlist[i];
		// && (pentlist[i]->m_iGroup==m_iGroup && pentlist[i]->m_bMoved || !pentlist[i]->IsAwake() ||
			//m_pWorld->m_pGroupNums[pentlist[i]->m_iGroup]<m_pWorld->m_pGroupNums[m_iGroup]))
		
	nents = j;
	gwd[0].v = m_qrot*vectorf(0,0,-1);
	ip.bStopAtFirstTri = true;
	ip.bNoBorder = true;
	ip.maxSurfaceGapAngle = 4.0f/180*pi;

	for(i=m_nHullParts;i<m_nParts;i++) {
		m_parts[i].flags = geom_colltype_player|geom_colltype_ray;	
		m_parts[i].flagsCollider = 0;
		iwheel = i-m_nHullParts;
		ip.time_interval = m_susp[iwheel].fullen*2.0f;
		//(m_qrot*m_parts[i].q).getmatrix(gwd[0].R); //Q2M_IVO
		//gwd[0].R = matrix3x3f(m_qrot*m_parts[i].q);
		qsteer = GetRotationAA(m_susp[iwheel].steer,vectorf(0,0,-1));
		axis = (m_qrot*qsteer)*vectorf(sgnnz(m_susp[iwheel].pt.x),0,0); // outward wheel rotation axis in world cs
		m_parts[i].q = qsteer*m_susp[iwheel].q0;
		gwd[0].R = matrix3x3f(m_qrot*m_parts[i].q);
		ptc1 = m_parts[i].q*m_parts[i].pPhysGeomProxy->origin + m_susp[iwheel].pos0;
		ptcw = m_pos + m_qrot*ptc1;
		(m_parts[i].pos = m_susp[iwheel].pos0+m_susp[iwheel].ptc0-ptc1).z -= m_susp[iwheel].fullen-m_susp[iwheel].len0;
		geompos = m_pos + m_qrot*m_parts[i].pos;

		gwd[0].scale = m_parts[i].scale;
		tmax = 0;	m_susp[iwheel].ptcontact = m_pos;
		m_susp[iwheel].pbody = 0;
		m_susp[iwheel].bContact = 0;

		for(ient=0;ient<nents;ient++) for(j=0; j<pentlist[ient]->m_nParts; j++) 
		if (pentlist[ient]->m_parts[j].flags & m_susp[iwheel].flagsCollider0) {
			gwd[0].offset = geompos;//m_pos + m_qrot*m_parts[i].pos + gwd[0].v*(m_susp[iwheel].fullen-m_susp[iwheel].curlen);
			if (ip.bSweepTest = true)//(pentlist[ient]->m_parts[j].flags & geom_has_thin_parts)!=0)
				gwd[0].offset -= gwd[0].v*ip.time_interval;

			gwd[1].offset = pentlist[ient]->m_pos + pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].pos;
			//(pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].q).getmatrix(gwd[1].R); //Q2M_IVO
			gwd[1].R = matrix3x3f(pentlist[ient]->m_qrot*pentlist[ient]->m_parts[j].q);
			gwd[1].scale = pentlist[ient]->m_parts[j].scale;

			ncontacts = m_parts[i].pPhysGeomProxy->pGeom->Intersect(pentlist[ient]->m_parts[j].pPhysGeomProxy->pGeom, gwd+0,gwd+1, &ip, pcontacts);
			for(icont=0; icont<ncontacts; icont++) if (pcontacts[icont].n*gwd[0].v>0.5f) {// || pcontacts[icont].n*gwd[0].v<-0.01f) {
				tcur = ip.bSweepTest ? ip.time_interval-pcontacts[icont].t : pcontacts[icont].t;
				if ((ip.bSweepTest || pcontacts[icont].vel>0) && tcur>tmax)	{
					tmax = tcur; m_susp[iwheel].bContact = 1;
					m_susp[iwheel].pent = pentlist[ient]; m_susp[iwheel].ipart = j;
					m_susp[iwheel].pbody = pentlist[ient]->GetRigidBody(j);
					m_susp[iwheel].surface_idx[0] = m_parts[i].surface_idx&pcontacts[icont].id[0]>>31 | pcontacts[icont].id[0]&~(pcontacts[icont].id[0]>>31);
					m_susp[iwheel].surface_idx[1] = pentlist[ient]->m_parts[j].surface_idx&pcontacts[icont].id[1]>>31 | 
						pcontacts[icont].id[1]&~(pcontacts[icont].id[1]>>31);
					// always project contact point to the outer wheel edge
					m_susp[iwheel].ptcontact = pcontacts[icont].pt + axis*(m_susp[iwheel].width-axis*(pcontacts[icont].pt-ptcw)); 
					m_susp[iwheel].ncontact = -pcontacts[icont].n;
					/*if (pcontacts[icont].parea && pcontacts[icont].parea->npt>1) {
						m_susp[iwheel].ptcontact = m_pos;	// find the contact point that is farthest from center along vehicle's x axis
						for(ipt=0;ipt<pcontacts[icont].parea->npt;ipt++)
						if (fabs_tpl((pcontacts[icont].parea->pt[ipt]-m_pos)*axisx) > fabs_tpl((m_susp[iwheel].ptcontact-m_pos)*axisx)) {
							m_susp[iwheel].ptcontact = pcontacts[icont].parea->pt[ipt]; 
							m_susp[iwheel].ncontact = pcontacts[icont].parea->n1;
						}
					} else {
						m_susp[iwheel].ptcontact = pcontacts[icont].pt; 
						m_susp[iwheel].ncontact = -pcontacts[icont].n;
					}*/
				}
				AddCollider(pentlist[ient]);
				pentlist[ient]->AddCollider(this);
			} else if (pcontacts[icont].n*gwd[0].v>-0.01f && // if contact normal is too steep (but not negative), 
								 pentlist[ient]->GetRigidBody(j)->Minv<m_body.Minv*10.f) // ..and colliding body is not loo light (too avoid degeneracy)
			{ 
				m_parts[i].flags = m_susp[iwheel].flags0; m_parts[i].flagsCollider = m_susp[iwheel].flagsCollider0;
				goto endwheel;	// treat wheel as part of vehicle's rigid geometry
			}	
		}

		endwheel:
		if (!m_parts[i].flagsCollider) {
			if (tmax>0) tmax += m_pWorld->m_vars.maxContactGap*0.5f;
			newlen = max(0.0f,m_susp[iwheel].fullen-tmax);
			if (m_susp[iwheel].bContact || newlen<m_susp[iwheel].curlen)
				m_susp[iwheel].curlen = newlen;
			else
				m_susp[iwheel].curlen = min(newlen, m_susp[iwheel].curlen+(newlen-m_susp[iwheel].curlen)*(time_interval*7));
			m_susp[iwheel].vrel = m_body.v+(m_body.w^m_susp[iwheel].ptcontact-m_body.pos);
			if (m_susp[iwheel].pbody)
				m_susp[iwheel].vrel -= m_susp[iwheel].pbody->v + (m_susp[iwheel].pbody->w^m_susp[iwheel].ptcontact-m_susp[iwheel].pbody->pos);

			for(j=0;j<NMASKBITS && getmask(j)<=contact_mask;j++) 
			if (contact_mask&getmask(j) && m_pContacts[j].ipart[0]==i && m_pContacts[j].pent[1]==m_susp[iwheel].pent) {
				// remove all rigid contacts with the wheel's collider if the wheel is in "suspension" contact state
				contact_mask &= ~getmask(j);
				for(icoll=0; icoll<m_nColliders && m_pColliders[icoll]!=m_susp[iwheel].pent; icoll++);
				if (icoll<m_nColliders && !((m_pColliderContacts[icoll]&=~getmask(j)) | m_pColliderConstraints[icoll]) && 
						tmax==0 && !m_pColliders[icoll]->HasContactsWith(this)) 
				{
					CPhysicalEntity *pCollider = m_pColliders[icoll]; 
					pCollider->RemoveCollider(this); RemoveCollider(pCollider);
				}
				break;
			}

			if (tmax>m_susp[iwheel].fullen) { // spring is fully compressed, but we still have penetration
				tmax -= m_susp[iwheel].fullen;
				if (tmax>m_susp[iwheel].fullen) {
					vreq = tmax*0.3f; tmax = 0;
				} else vreq = 0;

				r = m_susp[iwheel].ptcontact-m_body.pos; // apply impulse to stop relative velocity at the contact
				N = m_body.Minv+m_susp[iwheel].pbody->Minv + m_susp[iwheel].ncontact*((m_body.Iinv*(r^m_susp[iwheel].ncontact))^r);
				r = m_susp[iwheel].ptcontact-m_susp[iwheel].pbody->pos;
				N += m_susp[iwheel].ncontact*((m_susp[iwheel].pbody->Iinv*(r^m_susp[iwheel].ncontact))^r);
				ai.impulse = m_susp[iwheel].ncontact*((vreq-(m_susp[iwheel].ncontact*m_susp[iwheel].vrel))/N);
				ai.point = m_susp[iwheel].ptcontact;
				ai.ipart = i;	ai.iApplyTime = 2;
				Action(&ai);
				if (m_susp[iwheel].pent->GetRigidBody(m_susp[iwheel].ipart)->Minv<m_body.Minv*10.0f) {
					ai.impulse.Flip(); ai.ipart = m_susp[iwheel].ipart;
					m_susp[iwheel].pent->Action(&ai);
				}

				m_pos -= gwd[0].v*tmax;
			}
		}
		m_parts[i].pos.z += m_susp[iwheel].fullen-m_susp[iwheel].curlen;
	}

	m_qrot.Normalize();
	m_body.pos = m_pos+m_qrot*m_body.offsfb;
	m_body.q = m_qrot*!m_body.qfb;
	m_body.UpdateState();
}


void calc_lateral_limits(float cosa,float sina, float fspring,float mue, quotient &frmin,quotient &frmax)
{
	frmin.set(-mue*cosa-sina,cosa-mue*sina).fixsign();
	frmax = min(quotientf(cosa,sina), quotientf(mue*cosa-sina,cosa+mue*sina));
	frmin.x *= fspring; frmax.x *= fspring;
}


float CWheeledVehicleEntity::ComputeDrivingTorque(float time_interval)
{
/*
#ifdef _DEBUG
	static float __g_log_speed_timeout=0;
	if ((__g_log_speed_timeout+=time_interval)>1) {
		__g_log_speed_timeout=0;
		m_pWorld->m_pLog->LogToConsole("%.2f m/s",m_body.v.len());
	}
#endif
*/
	float wwheel=0,T=0,power;
	int i,iNewGear,sg=sgnnz(m_gears[m_iCurGear]);
	for(i=0;i<m_nParts-m_nHullParts;i++)
		wwheel = max(wwheel,m_susp[i].w*m_susp[i].bDriving*sg);
	wwheel *= sg;

	if (m_iCurGear!=1) {
		if (m_clutch>0)
			m_wengine += (wwheel*m_gears[m_iCurGear]-m_wengine)*(m_clutchSpeed*2*time_interval);
		if (m_wengine>m_engineMinw) {//m_enginePedal!=0) {
			m_clutch += time_interval*m_clutchSpeed;
			if (m_clutch>1.0f) {
/*
#ifdef _DEBUG
				if (m_clutch-time_interval*m_clutchSpeed<0.9999f)
					m_pWorld->m_pLog->LogToConsole("full clutch");
#endif
*/
				m_clutch = 1.0f; m_wengine = wwheel*m_gears[m_iCurGear];
			}
		}
	}

	// if the engine goes below min RPM - set neutral gear
	if (m_clutch>0 && m_wengine<m_engineMinw) {
		m_iCurGear = 1;
		m_clutch = 0; m_wengine = m_engineIdlew;
/*
#ifdef _DEBUG
		m_pWorld->m_pLog->LogToConsole("engine RPM's too low. switching to gear %d. clutch disengaged", m_iCurGear);
#endif
*/
	}
	
	// if pedal is on, but gear is neutral - set gear 0/2 and rev engine m_engineStartw, disengage clutch 
	iNewGear = m_iCurGear;
	if (m_enginePedal!=0 && m_iCurGear==1 && wwheel*-sgnnz(m_enginePedal)<m_gearDirSwitchw) {
		iNewGear = isneg(m_enginePedal)*2^2;
		m_wengine = m_engineStartw;
	} else if (m_clutch>0.5f && m_iCurGear>1) {
		if (m_wengine>m_engineShiftUpw)
			iNewGear = min(iNewGear+1,m_nGears-1);
		else if (m_wengine<m_engineShiftDownw && (m_enginePedal<=0 || m_iCurGear>2))
			iNewGear = max(iNewGear-1,1);
	}
	if (iNewGear!=m_iCurGear)	{
/*
#ifdef _DEBUG
		m_pWorld->m_pLog->LogToConsole("switching to gear %d. clutch disengaged",iNewGear);
#endif
*/
		m_clutch = 0;
	}
	m_iCurGear = iNewGear;

	if (m_enginePedal*sgn(m_iCurGear-1)>0) {
		if (m_wengine>0.1f) {
			power = sin_tpl(m_wengine/m_engineMaxw*pi)*m_enginePower*fabs_tpl(m_enginePedal);
			T = power/m_wengine;
		} else {
			T = fabs_tpl(m_enginePedal)*m_enginePower*(pi/m_engineMaxw);
		}
		T *= m_gears[m_iCurGear]*m_clutch;
	}

	return T;
}


/*const int NH=256;
CWheeledVehicleEntity *g_whist[NH],*g_whist2[NH];
CStream g_wsnap[NH];
entity_contact *g_conthist[NH],*g_conthist2[NH];
masktype *g_collhist[NH],*g_collhist2[NH];
int g_iwhist=0,g_checksum[NH],g_ncompare=0,g_bstartcompare=0,g_histinit=0,g_iwhist0=-1;
int g_forcepedal = 0;*/


void CWheeledVehicleEntity::AddAdditionalImpulses(float time_interval)
{
	int i,j,i1,j1,nfr,nfr1,idx[16],slide[16],nContacts,bAllSlip=1;
	float fN,friction,Npull,N,fpull,driving_torque=0,cosa,sina,maxfric,wmax=0,wground;
	quotient frmin[16],frmax[16];
	real buf[624];
	vectorf dP,Pexp,Lexp,ncontact,vdir,pulldir,ptc,axis,axisz,axisx,frdir[16],r,vel_slip;
	matrix3x3f R = matrix3x3f(m_qrot);
	pe_action_impulse ai;
	ai.iSource = 3;
	//m_qrot.getmatrix(R); //Q2M_IVO
	axisz = R*vectorf(0,0,1); axisx = R*vectorf(1,0,0);
//	cur_power = m_enginePedal>0 ? m_enginePower : m_enginePowerBack;
//if (g_forcepedal) m_enginePedal = 1;

	m_nContacts = 0;
	driving_torque = ComputeDrivingTorque(time_interval);

	// evolve wheels
	for(i=0;i<m_nParts-m_nHullParts;i++) {
		/*for(j=0,wengine=0;j<m_nParts-m_nHullParts;j++) if (m_susp[j].bDriving && m_susp[j].iAxle==m_susp[i].iAxle) 
			wengine = max(fabs_tpl(wengine), fabs_tpl(m_susp[j].w))*sgnnz(m_susp[j].w);
		if (wengine*m_enginePedal<0) wengine=0;
		wengine = min(fabs_tpl(wengine), m_engineMaxw);
		power = sin_tpl(wengine/m_engineMaxw*pi)*cur_power*m_enginePedal;
		driving_torque = wengine>0.1f ? power/wengine : m_enginePedal*cur_power*(pi/m_engineMaxw);*/

		m_susp[i].T = -sgn(m_susp[i].w)*m_axleFriction;
		if (m_enginePedal*sgn(m_iCurGear-1)<=0)
			m_susp[i].T += m_brakeTorque*m_enginePedal;
		if (driving_torque*m_susp[i].bDriving==0) {
			quotientf T0(-m_susp[i].w*m_susp[i].prevTdt*0.7f,(m_susp[i].w-m_susp[i].prevw)*time_interval);
			if (T0.x*T0.y*m_susp[i].T>0 && fabsf(m_susp[i].T*T0.y)>fabsf(T0.x))
				m_susp[i].T = T0.val();
		}
		m_susp[i].prevTdt = m_susp[i].T*time_interval;
		m_susp[i].prevw = m_susp[i].w;
		m_susp[i].T += driving_torque*m_susp[i].bDriving;

		if (m_bHandBrake & m_susp[i].bCanBrake) 
			m_susp[i].w = 0;
		if (fabs_tpl(m_susp[i].w*m_susp[i].bDriving) > fabs_tpl(wmax))
			wmax = m_susp[i].w*m_susp[i].bDriving;
		bAllSlip &= m_susp[i].bSlipPull|m_susp[i].bDriving^1;
		m_susp[i].bSlipPull = 0;
		m_susp[i].rot += m_susp[i].w*time_interval;
		if (m_susp[i].rot>2*pi) m_susp[i].rot-=2*pi;
		if (m_susp[i].rot<-2*pi) m_susp[i].rot+=2*pi;
	}

	Pexp.zero(); Lexp.zero(); nContacts=0;
	for(i=0;i<m_nParts-m_nHullParts;i++) {
		m_susp[i].rworld = R*m_susp[i].pt+m_pos-m_body.pos;
		m_susp[i].vworld = (m_body.v+(m_body.w^m_susp[i].rworld))*axisz;
		if (m_susp[i].bContact) {
			fN = (m_susp[i].fullen-m_susp[i].curlen)*m_susp[i].kStiffness;
			fN -= m_susp[i].vworld*m_susp[i].kDamping;
			if (m_susp[i].iBuddy>=0 && m_kStabilizer>0)
				fN -= (m_susp[i].curlen-m_susp[m_susp[i].iBuddy].curlen)*m_susp[i].kStiffness*m_kStabilizer;
			m_susp[i].PN = max(0.0f,fN*time_interval);
			dP = axisz*m_susp[i].PN; Pexp += dP; Lexp += m_susp[i].rworld^dP;
			idx[nContacts++] = i;
		}
	}

	if (m_iIntegrationType) { // solve for (delta)v = (I-AK)^-1*(A*(Pexplicit - ks*v*dt^2 + Pexternal))
		matrix I_AK(nContacts,nContacts,0,buf+nContacts*2);
		vectorn delta_v_src(nContacts,buf),delta_v_dst(nContacts,buf+nContacts);
		for(i=0;i<nContacts;i++) {
			dP = axisz*(-m_susp[idx[i]].vworld*m_susp[idx[i]].kStiffness*sqr(time_interval));
			if (m_susp[idx[i]].iBuddy && m_kStabilizer>0)
				dP += axisz*((m_susp[m_susp[idx[i]].iBuddy].vworld-m_susp[idx[i]].vworld)*m_susp[idx[i]].kStiffness*m_kStabilizer*sqr(time_interval));
			Pexp += dP; Lexp += m_susp[idx[i]].rworld^dP;
		}

		Pexp += (m_Ffriction+m_body.Fcollision)*time_interval; 
		Pexp += (m_nColliders ? m_gravity : m_gravityFreefall)*(time_interval*m_body.M);
		Lexp += (m_Tfriction+m_body.Tcollision-(m_body.w^m_body.L))*time_interval;
		Pexp = Pexp*m_body.Minv+m_gravity*time_interval; Lexp = m_body.Iinv*Lexp;	// Pexp = body delta v, Lexp = body delta w

		for(i=0;i<nContacts;i++) {
			for(j=0;j<nContacts;j++) { // build I - A*(-ks*dt^2-kd*dt) matrix
				float kStabilizer = m_kStabilizer*(isneg(m_susp[idx[j]].iBuddy)^1);
				I_AK[i][j] = iszero(i-j) + (m_body.Minv - (m_susp[idx[i]].rworld^(m_body.Iinv*(m_susp[idx[j]].rworld^axisz)))*axisz)*
					(m_susp[idx[j]].kStiffness*(1.0f+kStabilizer)*time_interval+m_susp[idx[j]].kDamping)*time_interval;
				if (m_susp[idx[j]].iBuddy>=0 && m_kStabilizer>0)
					I_AK[i][j] -= (m_body.Minv - (m_susp[idx[i]].rworld^(m_body.Iinv*(m_susp[m_susp[idx[j]].iBuddy].rworld^axisz)))*axisz)*
						m_susp[idx[j]].kStiffness*m_kStabilizer*sqr(time_interval);
			}
			delta_v_src[i] = (Pexp+(Lexp^m_susp[idx[i]].rworld))*axisz; // compute delta v along each spring
		}

		I_AK.invert();
		mul_vector_by_matrix(I_AK,delta_v_src.data,delta_v_dst.data);

		// now compute normal impulses that correspond to the computed delta_v
		for(i=0;i<nContacts;i++) {
			m_susp[idx[i]].vworld += delta_v_dst[i];
			fN = (m_susp[idx[i]].fullen-m_susp[idx[i]].curlen-m_susp[idx[i]].vworld*time_interval)*m_susp[i].kStiffness;
			fN -= m_susp[idx[i]].vworld*m_susp[idx[i]].kDamping;
			if (m_susp[i].iBuddy>=0 && m_kStabilizer>0)
				fN -= (m_susp[i].curlen-m_susp[m_susp[i].iBuddy].curlen+(m_susp[idx[i]].vworld-m_susp[m_susp[idx[i]].iBuddy].vworld)*time_interval)*
							m_susp[i].kStiffness*m_kStabilizer;
			m_susp[idx[i]].PN = fN*time_interval;
		}
	}

	Pexp.zero(); Lexp.zero();
	for(i=nfr=0;i<m_nParts-m_nHullParts;i++) if (m_susp[i].bContact) {
		dP = axisz*m_susp[i].PN; Pexp += dP; Lexp += m_susp[i].rworld^dP;
		j = m_bHandBrake & m_susp[i].bCanBrake;
		maxfric =  m_maxBrakingFriction*j + (j^1)*10.0f;
		friction = min(m_susp[i].maxFriction, max(m_susp[i].minFriction,
			(m_pWorld->m_FrictionTable[m_susp[i].surface_idx[0]&NSURFACETYPES-1]+m_pWorld->m_FrictionTable[m_susp[i].surface_idx[1]&NSURFACETYPES-1])*0.5f));
		friction = min(maxfric,friction);

		axis = R*vectorf(cos_tpl(m_susp[i].steer),-sin_tpl(m_susp[i].steer),0);
		pulldir = (m_susp[i].ncontact^axis).normalize();
		vel_slip = m_susp[i].vrel-pulldir*(m_susp[i].w*m_susp[i].r);
		if (vel_slip.len2()>sqr(m_slipThreshold))
			friction *= m_kDynFriction;
		Npull = N = m_susp[i].PN*friction;

		if (m_susp[i].pent->GetRigidBody(m_susp[i].ipart)->Minv<m_body.Minv*10.0f) {
			ai.point = m_susp[i].ptcontact;
			ai.impulse = -dP;
			ai.ipart = m_susp[i].ipart;
			m_susp[i].pent->Action(&ai);
		}

		if (m_bHandBrake & m_susp[i].bCanBrake) {
			/*if (m_susp[i].bSlip) { // the wheel slips, use rel. velocity direction as primary friction axis
				vdir = m_susp[i].vrel - m_susp[i].ncontact*(m_susp[i].vrel*m_susp[i].ncontact);
				frdir[nfr] = -vdir.normalized(); frmax[nfr] = N; idx[nfr++] = i;
				frdir[nfr] = frdir[nfr-1]^m_susp[i].ncontact; frmax[nfr] = N*0.3f; idx[nfr++] = i;
			} else {*/
			frdir[nfr] = axisz^axis; frdir[nfr] *= -sgnnz(frdir[nfr]*m_susp[i].ncontact);
			sina = pulldir*axisz;
			cosa = sqrt_tpl(max(0.0f,1.0f-sina*sina));
			calc_lateral_limits(cosa,sina, N*=0.85f,friction, frmin[nfr],frmax[nfr]);
			idx[nfr] = i;
			nfr += isneg(m_body.M*1E-6f*frmax[nfr].y-frmax[nfr].x);
		} else {
			if (vel_slip.len2()>sqr(m_slipThreshold))
				Npull *= fabs_tpl(vel_slip.normalized()*pulldir);
			ptc = R*(m_susp[i].ptc0-vectorf(0,0,m_susp[i].curlen-m_susp[i].len0))+m_pos - m_body.pos;
			//ptc = R*(m_parts[i+m_nHullParts].q*m_parts[i+m_nHullParts].pPhysGeomProxy->origin + m_parts[i+m_nHullParts].pos) +	m_pos - m_body.pos;
			fpull = m_susp[i].T*m_susp[i].rinv*time_interval;
			wground = (m_susp[i].vrel*pulldir)*m_susp[i].rinv;
			if (isneg(fabs_tpl(Npull)-fabs_tpl(fpull)) && m_susp[i].bDriving)	{
				m_susp[i].bSlipPull = 1;
				m_susp[i].w = wmax;
				if (bAllSlip)
					m_susp[i].w += (fpull-sgn(fpull)*Npull)*m_susp[i].r*m_susp[i].Iinv;
				if (m_susp[i].w*wground<0 || m_susp[i].w*m_susp[i].T<0 || fabs_tpl(wground)>fabs_tpl(m_susp[i].w))
					m_susp[i].w = wground;
				if (fabs_tpl(m_susp[i].w)*m_gears[m_iCurGear] > m_engineMaxw)
          m_susp[i].w = sgnnz(m_susp[i].w)*m_engineMaxw/m_gears[m_iCurGear];
				fpull = sgn(fpull)*Npull;
			} else
				m_susp[i].w = wground;
			dP = axisz^axis; dP *= (pulldir*dP)*fpull; Pexp += dP; Lexp += ptc^dP;
			N = sqrt_tpl(max(0.0f,sqr(N)-sqr(fpull)));
		}
		frdir[nfr] = axis; frdir[nfr] *= -sgnnz(frdir[nfr]*m_susp[i].ncontact);
		cosa = axisz*m_susp[i].ncontact;
		sina = sqrt_tpl(max(0.0f,1.0f-cosa*cosa));
		calc_lateral_limits(cosa,sina, N,friction, frmin[nfr],frmax[nfr]);
		idx[nfr] = i;
		nfr += isneg(m_body.M*1E-6f*frmax[nfr].y-frmax[nfr].x);
		m_susp[i].bSlip = 0;
	}
	m_body.v = (m_body.P+=Pexp)*m_body.Minv; 
	m_body.w = m_body.Iinv*(m_body.L+=Lexp);
	m_Ffriction.zero(); m_Tfriction.zero();

	if (nfr>0) {
		matrix A(nfr,nfr,mtx_PSD | mtx_symmetric,buf);
		vectorn v(nfr,buf+nfr*nfr),p(nfr,buf+nfr*(nfr+1)),v1(nfr,buf+nfr*(nfr+2)),p1(nfr,buf+nfr*(nfr+3)),
			dv(nfr,buf+nfr*(nfr+5)),dp(nfr,buf+nfr*(nfr+6));
		float sign; int d,prev_j,bSliding=0;
		quotient s,s1,sd;	real sval;
		float Ebefore=CalcEnergy(0),Eafter,damping;
		vectorf Pbefore,Lbefore;
		int iter;

		for(i=0;i<nfr;i++) {
			r = m_susp[idx[i]].ptcontact-m_body.pos; 
			v[i] = (m_body.v + (m_body.w^r) - m_susp[idx[i]].pbody->v - 
				(m_susp[idx[i]].pbody->w^m_susp[idx[i]].ptcontact-m_susp[idx[i]].pbody->pos))*frdir[i];
			v[i] += ((m_body.Fcollision*m_body.Minv+(m_body.Iinv*m_body.Tcollision^r))*frdir[i])*time_interval;
			for(j=i;j<nfr;j++)
				A[i][j] = A[j][i] = frdir[i]*(frdir[j]*m_body.Minv + (m_body.Iinv*(m_susp[idx[j]].ptcontact-m_body.pos^frdir[j])^r));
			v1[i] = -v[i]; slide[i] = 0; 
			if (frmax[i].x*frmin[i].x>0 && frmax[i].y*frmin[i].y>1E-6) {
				p[i] = (frmax[i]+frmin[i]).val()*0.5; bSliding = 1;
			} else p[i] = 0;
		}
		if (bSliding)
			v += A*p;

		// first, try to drive all relative velocities to zero and see if we exceed friction limits
		p1.zero(); A.conjugate_gradient(p1,v1,0,0.0001);
		for(i=bSliding=0;i<nfr;i++) bSliding += isneg(frmax[i]-fabs_tpl(p1[i]));
		if (!bSliding) {
			for(i=0;i<nfr;i++) p[i] = p1[i];
			d = nfr;
		} else // .. invoke LCP solver otherwise
		for(d=iter=0;d<nfr;d++) if (!slide[d]) {
			prev_j=-1; do {
			for(i=nfr1=0;i<d;i++) nfr1 += iszero(slide[i]);
			sign = -sgn(v[d]); dp.zero();
			if (nfr1>0) {
				matrix A1(nfr1,nfr1,mtx_PSD | mtx_symmetric,buf+nfr*(nfr+7));
				for(i=i1=0;i<d;i++) if (!slide[i]) {
					v1[i1] = -sign*A[i][d];
					for(j=j1=0;j<d;j++) if (!slide[j])
						A1[i1][j1++] = A[i][j];
					i1++;
				} p1.zero();
				A1.conjugate_gradient(p1,v1);
				for(i=i1=0;i<d;i++) if (!slide[i])
					dp[i] = p1[i1++];
			}
			dp[d] = sign;
			dv = A*dp;
			s.set(1,1E-20); j=-1;
			if (v[d]*dv[d]<0) 
			{	j=d; sd = s.set(-v[d],dv[d]).fixsign(); }
			for(i=0;i<=d;i++) if (
				slide[i]==0 && (s1.set(frmax[i]-p[i],dp[i]).fixsign()>=0 && s1<s || s1.set(frmin[i]-p[i],dp[i]).fixsign()>=0 && s1<s) ||
				slide[i]==1 && (v[i]!=0 || p[i]*dv[i]<0) && (s1.set(-v[i],dv[i]).fixsign()>=0 && s1<s))
			{ s=s1; j=i; }
			if (j==-1)
				break;
			sval = s.val(); 
#if defined(LINUX)
			p = ::operator+=(p, (const vectorn&)(dp*sval)); 
			v = (const vectorn&)(dv.operator*(sval));
#else
			p += dp*sval; 
			v += dv*sval;
#endif
			if (j==d) {
				if (s!=sd) 
					slide[d]=1; 
				break;
			}
			v[j]=0;	slide[j] = slide[j]^1;
			if (slide[j] && isneg(s.x-1E-8*s.y) && prev_j==j)
				break;
			prev_j=j;
		} while(++iter<1000); }

		for(i=0;i<d;i++) {
			dP = frdir[i]*p[i]; m_Ffriction += dP; m_Tfriction += m_susp[idx[i]].ptcontact-m_body.pos^dP;
		}
		Eafter = (m_body.P+m_Ffriction).len2()*m_body.Minv + (m_body.Iinv*(m_body.L+m_Tfriction))*(m_body.L+m_Tfriction);
		if (Eafter>Ebefore*1.1f)
			damping = sqrt_tpl(Ebefore/Eafter);
		else 
			damping = 1.0f;

		for(i=0;i<d;i++) {
			m_susp[idx[i]].bSlip = slide[i];
			//dP = frdir[i]*p[i]; m_Ffriction += dP; m_Tfriction += m_susp[idx[i]].ptcontact-m_body.pos^dP;
			if (m_susp[idx[i]].pent->GetRigidBody(m_susp[idx[i]].ipart)->Minv<m_body.Minv*10.0f) {
				ai.point = m_susp[idx[i]].ptcontact;
				ai.impulse = -frdir[i]*(p[i]*damping);
				ai.ipart = m_susp[idx[i]].ipart;
				m_susp[idx[i]].pent->Action(&ai);
			}
		}
		Pbefore = m_body.P; Lbefore = m_body.L;
		(m_body.P+=m_Ffriction)*=damping; (m_body.L+=m_Tfriction)*=damping;
		m_Ffriction = m_body.P-Pbefore; m_Tfriction = m_body.L-Lbefore;

#ifdef _DEBUG
#ifdef WIN64
		assert(m_Ffriction.len2()>=0);
#else
		if (!(m_Ffriction.len2()>=0))
			DEBUG_BREAK;
#endif
		m_body.UpdateState();
		for(i=0;i<d;i++) {
			r = m_susp[idx[i]].ptcontact-m_body.pos;
			v[i] = (m_body.v + (m_body.w^r) - m_susp[idx[i]].pbody->v - 
				(m_susp[idx[i]].pbody->w^m_susp[idx[i]].ptcontact-m_susp[idx[i]].pbody->pos))*frdir[i];
			v[i] += ((m_body.Fcollision*m_body.Minv+(m_body.Iinv*m_body.Tcollision^r))*frdir[i])*time_interval;
		}
#endif

		time_interval = 1.0f/time_interval;
		m_Ffriction*=time_interval; m_Tfriction*=time_interval;
	}

	/*m_bodyStatic.pos = m_body.pos;
	m_bodyStatic.q = m_body.q;
	m_bodyStatic.v = m_body.v;
	m_bodyStatic.w = m_body.w;*/
}


void CWheeledVehicleEntity::UpdateWheelsGeoms()
{
	vectorf ptc1;
	for(int i=0;i<m_nParts-m_nHullParts;i++) {
		//CHANGED_BY_IVO
	  //	m_parts[i+m_nHullParts].q =	quaternionf(m_susp[i].steer,vectorf(0,0,-1))*quaternionf(m_susp[i].rot,vectorf(-1,0,0))*m_susp[i].q0;
		m_parts[i+m_nHullParts].q = 
			//GetRotationAA(m_susp[i].steer,vectorf(0,0,-1))*GetRotationAA(m_susp[i].rot,vectorf(-1,0,0))*m_susp[i].q0;
			GetRotationAA(m_susp[i].steer,vectorf(0,0,-1))*m_susp[i].q0;
		ptc1 = m_parts[i+m_nHullParts].q*m_parts[i+m_nHullParts].pPhysGeomProxy->origin + m_susp[i].pos0;
		(m_parts[i+m_nHullParts].pos = m_susp[i].pos0+m_susp[i].ptc0-ptc1).z -= m_susp[i].curlen-m_susp[i].len0;
	}
}


int CWheeledVehicleEntity::OnRegisterContact(entity_contact *pcontact, int iop)
{
	if (pcontact->pent[iop^1]!=this)
		m_nContacts++;
	return 1;
}


float CWheeledVehicleEntity::GetMaxTimeStep(float time_interval) 
{
	m_maxAllowedStep = m_bHasContacts ? m_maxAllowedStepRigid : m_maxAllowedStepVehicle;
	return CRigidEntity::GetMaxTimeStep(time_interval);
}


float CWheeledVehicleEntity::GetDamping(float time_interval)
{
	return m_bHasContacts ? CRigidEntity::GetDamping(time_interval) : max(0.0f,1.0f-m_dampingVehicle*time_interval);
}


int CWheeledVehicleEntity::HasContactsWith(CPhysicalEntity *pent)
{
	int i; 
	for(i=0;i<m_nParts-m_nHullParts && m_susp[i].pent!=pent;i++);
	if (i<m_nParts-m_nHullParts)
		return 1;
	for(i=0;i<m_nColliders && m_pColliders[i]!=pent;i++);
	return i==m_nColliders ? 0 : iszero((int)(m_pColliderContacts[i] | m_pColliderConstraints[i]))^1;
}


int CWheeledVehicleEntity::Update(float time_interval, float damping)
{
	if (m_nContacts)
		m_timeNoContacts = 0;
	else 
		m_timeNoContacts += time_interval;
	m_bHasContacts = isneg(m_timeNoContacts-0.5f);
	m_Emin = m_bHasContacts ? m_EminRigid : m_EminVehicle;

	CRigidEntity::Update(time_interval, damping);
	
	/*m_simTime += time_interval;
	if (!m_bAwake && m_simTime<m_minSimTime) {
		m_bAwake = 1;
		if (m_iSimClass<2) {
			m_iSimClass = 2; m_pWorld->RepositionEntity(this,2);	
		}
	}*/

	if (m_pWorld->m_vars.bMultiplayer) {
		for(int i=0;i<m_nParts-m_nHullParts;i++)
			m_susp[i].w = CompressFloat(m_susp[i].w,200.0f,14);
		m_enginePedal = CompressFloat(m_enginePedal,1.0f,12);
		m_clutch = CompressFloat(m_clutch,1.0f,12);
		m_wengine = CompressFloat(m_wengine,200.0f,14);
	}

	/*if (!g_histinit) {
		g_histinit=1; int i;
		for(i=0;i<NH;i++) { g_whist[i] = new CWheeledVehicleEntity(m_pWorld);	g_whist2[i] = new CWheeledVehicleEntity(m_pWorld); }
		for(i=0;i<NH;i++) { g_conthist[i] = new entity_contact[64];	g_conthist2[i] = new entity_contact[64]; }
		for(i=0;i<NH;i++) { g_collhist[i] = new masktype[64];	g_collhist2[i] = new masktype[64]; }
	}
	if (g_ncompare) {
		if (g_checksum[g_iwhist]!=GetStateChecksum())
		g_iwhist = g_iwhist;
	} else {
		memcpy(g_whist[g_iwhist],this,sizeof(*this));
		memcpy(g_conthist[g_iwhist],m_pContacts,m_nContactsAlloc*sizeof(entity_contact));
		memcpy(g_collhist[g_iwhist],m_pColliderContacts,m_nCollidersAlloc*sizeof(masktype));
		g_wsnap[g_iwhist].Reset();
		GetStateSnapshot(g_wsnap[g_iwhist]);
		g_checksum[g_iwhist] = GetStateChecksum();
	}
	memcpy(g_whist2[g_iwhist],this,sizeof(*this));
	memcpy(g_conthist2[g_iwhist],m_pContacts,m_nContactsAlloc*sizeof(entity_contact));
	memcpy(g_collhist2[g_iwhist],m_pColliderContacts,m_nCollidersAlloc*sizeof(masktype));
	g_iwhist = g_iwhist+1 & NH-1;
	if (g_ncompare && ++g_ncompare>NH-2) g_ncompare=0;
	if (g_bstartcompare) {
		if (g_iwhist0>=0)	g_iwhist=g_iwhist0;
		else g_iwhist0=g_iwhist;
		g_ncompare = 1; g_bstartcompare=0;
		g_wsnap[g_iwhist].Seek(0);
		SetStateFromSnapshot(g_wsnap[g_iwhist]);
		PostSetStateFromSnapshot();
		g_iwhist = g_iwhist+1 & NH-1;
	}*/

	return (m_bAwake^1) | isneg(m_timeStepFull-m_timeStepPerformed-0.001f);
}


int CWheeledVehicleEntity::GetStateSnapshot(CStream &stm, float time_back, int flags)
{
	CRigidEntity::GetStateSnapshot(stm,time_back,flags);

	if (!(flags & ssf_checksum_only)) {
		if (m_pWorld->m_vars.bMultiplayer) {
			for(int i=0;i<m_nParts-m_nHullParts;i++)
				WriteCompressedFloat(stm, m_susp[i].w, 200.0f,14);
			WriteCompressedFloat(stm, m_enginePedal, 1.0f,12);
			WriteCompressedFloat(stm, m_steer, 1.0f,12);
			WriteCompressedFloat(stm, m_clutch, 1.0f,12);
			WriteCompressedFloat(stm, m_wengine, 200.0f,14);
		} else {
			for(int i=0;i<m_nParts-m_nHullParts;i++)
				stm.Write(m_susp[i].w);
			stm.Write(m_enginePedal);
			stm.Write(m_steer);
			stm.Write(m_clutch);
			stm.Write(m_wengine);
		}
		stm.Write(m_bHandBrake!=0);
		if (m_iIntegrationType) {
			stm.Write(true);
			stm.Write(m_Ffriction);
			stm.Write(m_Tfriction);
		} else stm.Write(false);
		if (m_body.Fcollision.len2()+m_body.Tcollision.len2()>0) {
			stm.Write(true);
			stm.Write(m_body.Fcollision);
			stm.Write(m_body.Tcollision);
		}	else stm.Write(false);
		stm.Write(m_bHasContacts!=0);
		stm.WriteNumberInBits(m_iCurGear,3);
	}

	return 1;
}

int CWheeledVehicleEntity::SetStateFromSnapshot(CStream &stm, int flags)
{
	int res = CRigidEntity::SetStateFromSnapshot(stm,flags);
	if (res && res!=2) {
		bool bnz;
		if (!(flags & ssf_no_update)) {
			pe_action_drive ad;
			if (m_pWorld->m_vars.bMultiplayer) {
				for(int i=0;i<m_nParts-m_nHullParts;i++)
					ReadCompressedFloat(stm, m_susp[i].w, 200.0f,14);
				ReadCompressedFloat(stm, ad.pedal, 1.0f,12);
				ReadCompressedFloat(stm, ad.steer, 1.0f,12);
				ReadCompressedFloat(stm, m_clutch, 1.0f,12);
				ReadCompressedFloat(stm, m_wengine, 200.0f,14);
			} else {
				for(int i=0;i<m_nParts-m_nHullParts;i++)
					stm.Read(m_susp[i].w);
				stm.Read(ad.pedal);
				stm.Read(ad.steer);
				stm.Read(m_clutch);
				stm.Read(m_wengine);
			}
			stm.Read(bnz); ad.bHandBrake = bnz ? 1:0;
			Action(&ad);

			stm.Read(bnz); if (bnz) {
				stm.Read(m_Ffriction);
				stm.Read(m_Tfriction);
			}
			stm.Read(bnz); if (bnz) {
				stm.Read(m_body.Fcollision);
				stm.Read(m_body.Tcollision);
			}	else {
				m_body.Fcollision.zero();
				m_body.Tcollision.zero();
			}
			stm.Read(bnz); m_bHasContacts = bnz ? 1:0;
			stm.ReadNumberInBits(m_iCurGear,3);
		} else {
			stm.Seek(stm.GetReadPos()+(m_pWorld->m_vars.bMultiplayer ? 
				(m_nParts-m_nHullParts)*14+12*3+14+1 : (m_nParts-m_nHullParts)*sizeof(float)*8+4*sizeof(float)*8+1));
			stm.Read(bnz); if (bnz)
				stm.Seek(stm.GetReadPos()+2*sizeof(vectorf)*8);
			stm.Read(bnz); if (bnz)
				stm.Seek(stm.GetReadPos()+2*sizeof(vectorf)*8);
			stm.Seek(stm.GetReadPos()+4);
		}
	}

	return res;
}



void CWheeledVehicleEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	CRigidEntity::GetMemoryStatistics(pSizer);
	if (GetType()==PE_WHEELEDVEHICLE)
		pSizer->AddObject(this, sizeof(CWheeledVehicleEntity));
}