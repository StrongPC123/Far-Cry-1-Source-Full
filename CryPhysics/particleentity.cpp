//////////////////////////////////////////////////////////////////////
//
//	Particle Entity
//	
//	File: particleentity.cpp
//	Description : CParticleEntity class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "singleboxtree.h"
#include "raybv.h"
#include "raygeom.h"
#include "intersectionchecks.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "geoman.h"
#include "physicalworld.h"
#include "particleentity.h"


CParticleEntity::CParticleEntity(CPhysicalWorld *pWorld) : CPhysicalEntity(pWorld)
{
	m_gravity = pWorld->m_vars.gravity; m_mass=0.2f; m_dim=m_dimLying=0.05f; m_rdim=20.0f;
	m_heading.Set(1,0,0); m_vel.zero(); m_wspin.zero();
	m_waterGravity = m_gravity*0.8f;
	m_kAirResistance = 0;
	m_kWaterResistance = 0.5f;
	m_accThrust = 0;
	m_kAccLift = 0;
	m_qspin.SetIdentity();
	m_surface_idx = 0;
	m_dirdown.Set(0,0,-1);
	m_normal.Set(0,0,1);
	m_flags = 0;
	m_iSimClass = 4;
	for(int i=0;i<sizeof(m_CollHistory)/sizeof(m_CollHistory[0]);i++)
		m_CollHistory[i].age = 1E10;
	m_iCollHistory = 0;
	m_bSliding = 0;
	m_slide_normal.Set(0,0,1);
	m_minBounceVel = 1.5f;
	m_pColliderToIgnore = 0;
	m_iPierceability = sf_max_pierceable;
	m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = -3;
	m_timeSurplus = 0;

	m_defpart.flags = 0;
	m_defpart.id = 0;
	m_defpart.pos.zero();
	m_defpart.q.SetIdentity();
	m_defpart.scale = 1.0f;
	m_defpart.mass = 0;
	m_defpart.minContactDist = 0;
	m_waterPlane.origin.zero();
	m_waterPlane.n.Set(0,0,1);
	m_waterDensity = 1000.0f;
	m_bForceAwake = -1;
	m_timeForceAwake = 0;
	m_sleepTime = 0;
}

CParticleEntity::~CParticleEntity()
{
	if (m_nParts>0 && m_pWorld)
		m_pWorld->GetGeomManager()->UnregisterGeometry(m_parts[0].pPhysGeom);
	m_nParts = 0;
}


int CParticleEntity::SetParams(pe_params *_params)
{
	int res;
	if (res = CPhysicalEntity::SetParams(_params)) {
		if (_params->type==pe_params_flags::type_id) {
			pe_params_particle pp;
			SetParams(&pp);
		}
		return res;
	}

	if (_params->type==pe_params_particle::type_id) {
		pe_params_particle *params = (pe_params_particle*)_params;
		ENTITY_VALIDATE("CParticleEntity:SetParams(pe_params_particle)",params);
		if (!is_unused(params->mass)) m_mass = params->mass; 
		if (!is_unused(params->size)) {
			m_rdim = 1.0f/(m_dim = params->size*0.5f);
			m_dimLying = params->size*0.5f;
		}
		if (!is_unused(params->thickness)) m_dimLying = params->thickness*0.5f;
		if (!is_unused(params->kAirResistance)) m_kAirResistance = params->kAirResistance;
		if (!is_unused(params->kWaterResistance)) m_kWaterResistance = params->kWaterResistance;
		if (!is_unused(params->accThrust)) m_accThrust = params->accThrust;
		if (!is_unused(params->accLift) && !is_unused(params->velocity))
			m_kAccLift = params->velocity!=0 ? params->accLift/fabs_tpl(params->velocity):0;
		if (!is_unused(params->heading)) m_heading = params->heading;
		if (!is_unused(params->velocity)) m_vel = m_heading*params->velocity;
		if (!is_unused(params->wspin)) m_wspin = params->wspin;
		if (!is_unused(params->gravity)) {
			m_gravity = params->gravity;
			if (m_gravity.len2()>0)
				(m_dirdown=m_gravity).normalize();
			else m_dirdown.Set(0,0,-1);
		}
		if (!is_unused(params->waterGravity)) m_waterGravity = params->waterGravity;
		if (!is_unused(params->normal)) m_normal = params->normal;
		if (!is_unused(params->q0)) m_qspin = params->q0;
		if (!is_unused(params->minBounceVel)) m_minBounceVel = params->minBounceVel;
		if (!is_unused(params->surface_idx)) m_surface_idx = params->surface_idx;
		if (!is_unused(params->flags)) m_flags = params->flags;

		if (m_flags & particle_traceable) {
			if (m_ig[0].x==-3)
				m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = -2;
			if (m_pos.len2()>0)
				m_pWorld->RepositionEntity(this,1);
		}	else {
			if (m_ig[0].x!=-3) {
				m_pWorld->DetachEntityGridThunks(this);
				m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = -3;
			}
		}
		
		if (!is_unused(params->pColliderToIgnore))
			m_pColliderToIgnore = (CPhysicalEntity*)params->pColliderToIgnore;
		if (!is_unused(params->iPierceability))
			m_iPierceability = params->iPierceability;

		if (!(m_flags & particle_constant_orientation)) {
			if (!(m_flags & particle_no_path_alignment)) {
				vectorf dirbuf[3]; 
				dirbuf[0] = m_dirdown^m_heading; dirbuf[1] = m_heading;
			//	if (dirbuf[0].len2()<0.01f) dirbuf[0] = m_heading.orthogonal();
				if (dirbuf[0].len2()<0.01f) dirbuf[0].SetOrthogonal(m_heading);
				dirbuf[0].normalize(); dirbuf[2] = dirbuf[0]^dirbuf[1];
				m_qrot = quaternionf((matrix3x3CMf&)dirbuf[0])*m_qspin;
			} else
				m_qrot = m_qspin;
		}

		m_BBox[0] = m_pos-vectorf(m_dim,m_dim,m_dim);
		m_BBox[1] = m_pos+vectorf(m_dim,m_dim,m_dim);
		return 1;
	}

	if (_params->type==pe_params_buoyancy::type_id) {
		pe_params_buoyancy *params = (pe_params_buoyancy*)_params;
		if (!is_unused(params->waterDensity)) m_waterDensity = params->waterDensity;
		if (!is_unused(params->waterPlane.n)) m_waterPlane.n = params->waterPlane.n;
		if (!is_unused(params->waterPlane.origin)) m_waterPlane.origin = params->waterPlane.origin;
		return 1;
	}

	return 0;
}


int CParticleEntity::GetParams(pe_params *_params)
{
	if (CPhysicalEntity::GetParams(_params))
		return 1;

	if (_params->type==pe_params_particle::type_id) {
		pe_params_particle *params = (pe_params_particle*)_params;
		params->mass = m_mass; 
		params->size = m_dim*2.0f;
		params->thickness = m_dimLying*2.0f;
		params->heading = m_heading;
		params->velocity = m_vel.len();
		params->wspin = m_wspin;
		params->gravity = m_gravity;
		params->normal = m_normal;
		params->kAirResistance = m_kAirResistance;
		params->accThrust = m_accThrust;
		params->accLift = params->velocity*m_kAccLift;
		params->q0 = m_qspin;
		params->surface_idx = m_surface_idx;
		params->flags = m_flags;
		params->pColliderToIgnore = m_pColliderToIgnore;
		params->iPierceability = m_iPierceability;
		return 1;
	}

	if (_params->type==pe_params_buoyancy::type_id) {
		pe_params_buoyancy *params = (pe_params_buoyancy*)_params;
		params->waterDensity = m_waterDensity;
		params->waterDamping = 0;
		params->waterPlane = m_waterPlane;
		params->waterFlow.zero();
		params->waterResistance = 0;
		params->waterEmin = 0;
		return 1;
	}

	return 0;
}


int CParticleEntity::GetStateSnapshot(CStream &stm, float time_back, int flags)
{
	stm.WriteNumberInBits(SNAPSHOT_VERSION, 4);
	if (m_pWorld->m_vars.bMultiplayer) {
		if (!IsAwake()) {
			if (m_sleepTime>5.0f)
				stm.Write(false);
			else {
				stm.Write(true);
				stm.Write(m_pos);
				stm.Write(false);
			}
		} else {
			stm.Write(true); stm.Write(m_pos);
			stm.Write(true);
			stm.Write(m_vel);
			if (!m_bSliding) stm.Write(false);
			else {
				stm.Write(true);
				stm.Write(asin_tpl(m_slide_normal.z));
				stm.Write(atan2_tpl(m_slide_normal.y,m_slide_normal.x));
			}
		}
	} else {
		stm.Write(m_pos);
		stm.Write(m_vel);
		if (!m_bSliding) stm.Write(false);
		else {
			stm.Write(true);
			stm.Write(asin_tpl(m_slide_normal.z));
			stm.Write(atan2_tpl(m_slide_normal.y,m_slide_normal.x));
		}
	}
	/*if (m_qspin.v.len2()<0.01*0.01) stm.Write(false);
	else {
		stm.Write(true); 
		//CHANGED_BY_IVO (NOTE: order of angles is flipped!!!!)
		//float angles[3]; m_qspin.get_Euler_angles_xyz(angles[0],angles[1],angles[2]);
		//EULER_IVO
		//Vec3 TempAng; m_qspin.GetEulerAngles_XYZ( TempAng );	
		vectorf angles = Ang3::GetAnglesXYZ(matrix3x3f(m_qspin));	
		for(int i=0;i<3;i++) stm.Write((unsigned short)float2int((angles[i]+pi)*(65535.0/2/pi)));
	}
	if (m_wspin.len2()==0) stm.Write(false);
	else {
		stm.Write(true); stm.Write(m_wspin);
	}*/

	return 1;
}

int CParticleEntity::SetStateFromSnapshot(CStream &stm, int flags)
{
	bool bnz;	int ver=0;
	stm.ReadNumberInBits(ver,4);
	if (ver!=SNAPSHOT_VERSION)
		return 0;

	if (!(flags & ssf_no_update)) {
		if (m_pWorld->m_vars.bMultiplayer) {
			stm.Read(bnz); if (bnz) {
				stm.Read(m_pos);
				stm.Read(bnz); if (bnz) {
					stm.Read(m_vel);
					stm.Read(bnz); if (bnz) {
						m_bSliding = 1;
						float yaw,pitch;
						stm.Read(pitch); stm.Read(yaw);
						m_slide_normal(cos_tpl(yaw)*cos_tpl(pitch),sin_tpl(yaw)*cos_tpl(pitch),sin_tpl(pitch));
					}	else m_bSliding = 0;
				}
			}
		}	else {
			stm.Read(m_pos);
			stm.Read(m_vel);
			stm.Read(bnz); if (bnz) {
				m_bSliding = 1;
				float yaw,pitch;
				stm.Read(pitch); stm.Read(yaw);
				m_slide_normal(cos_tpl(yaw)*cos_tpl(pitch),sin_tpl(yaw)*cos_tpl(pitch),sin_tpl(pitch));
			}	else m_bSliding = 0;
		}
		if (m_bForceAwake!=0)
			m_bForceAwake = -1;
	}	else {
		if (m_pWorld->m_vars.bMultiplayer) {
			stm.Read(bnz); if (bnz) {
				stm.Seek(stm.GetReadPos()+sizeof(vectorf)*8);
				stm.Read(bnz); if (bnz) {
					stm.Seek(stm.GetReadPos()+sizeof(vectorf)*8);
					stm.Read(bnz); if (bnz)
						stm.Seek(stm.GetReadPos()+2*sizeof(float)*8);
				}
			}
		} else {
			stm.Seek(stm.GetReadPos()+2*sizeof(vectorf)*8);
			stm.Read(bnz); if (bnz)
				stm.Seek(stm.GetReadPos()+2*sizeof(float)*8);
		}
	}
	/*stm.Read(bnz); if (bnz) {
		unsigned short tmp; int i; vectorf axis(zero);
		for(i=0,m_qspin.SetIdentity(); i<3; i++) {
			axis[i] = 1;
			//stm.Read(tmp); m_qspin*=quaternionf(tmp*(2*pi/65535.0)-pi, axis);
			stm.Read(tmp); m_qspin = GetRotationAA((float)(tmp*(2*pi/65535.0)-pi), axis)*m_qspin;
			axis[i] = 0;
		}
	}
	stm.Read(bnz); if (bnz)
		stm.Read(m_wspin);
	else m_wspin.zero();*/

	return 1;
}


int CParticleEntity::GetStatus(pe_status* _status)
{
	if (CPhysicalEntity::GetStatus(_status))
		return 1;

	if (_status->type==pe_status_collisions::type_id) {
		pe_status_collisions *status = (pe_status_collisions*)_status;
		int i,n,nmax = min(status->len, sizeof(m_CollHistory)/sizeof(m_CollHistory[0]));
		for(i=m_iCollHistory,n=0; n<nmax && m_CollHistory[i].age <= status->age; i=i-1&sizeof(m_CollHistory)/sizeof(m_CollHistory[0])-1,n++)
			status->pHistory[n] = m_CollHistory[i];
		if (status->bClearHistory) for(i=0;i<sizeof(m_CollHistory)/sizeof(m_CollHistory[0]);i++)
			m_CollHistory[i].age = 1E10;
		return status->len = n;
	}

	if (_status->type==pe_status_dynamics::type_id) {
		pe_status_dynamics *status = (pe_status_dynamics*)_status;
		vectorf gravity; float kAirResistance;
		if (m_waterDensity>0 && (m_pos-m_waterPlane.origin)*m_waterPlane.n<0) {
			gravity = m_waterGravity; kAirResistance = m_kWaterResistance;
		} else {
			gravity = m_gravity; kAirResistance = m_kAirResistance;
		}
		status->v = m_vel;
		status->a = gravity+m_heading*m_accThrust-m_vel*kAirResistance+(m_heading^(m_heading^m_dirdown)).normalize()*m_kAccLift*m_vel.len();
		status->w = m_wspin;
		status->centerOfMass = m_pos;
		return 1;
	}

	return 0;
}


int CParticleEntity::Action(pe_action* _action)
{
	int res;
	if (res = CPhysicalEntity::Action(_action))
		return res;

	if (_action->type==pe_action_impulse::type_id) {
		pe_action_impulse *action = (pe_action_impulse*)_action;
		vectorf P=action->impulse, L(zero);
		if (!is_unused(action->momentum))
			L = action->momentum;
		else if (!is_unused(action->point))
			L = action->point-m_pos^action->impulse;
		m_vel += P/m_mass;
		if (!(m_flags & particle_constant_orientation))
			m_wspin += L/(0.4f*m_mass*m_dim);
		return 1;
	}

	if (_action->type==pe_action_reset::type_id) {
		m_vel.zero(); m_wspin.zero();
		return 1;
	}
	return 0;
}

int CParticleEntity::Awake(int bAwake,int iSource) 
{ 
	if (bAwake)
		m_bForceAwake = 1; 
	else {
		m_bForceAwake = 0;
		m_vel.zero();
	}
	return m_iSimClass;
}

int CParticleEntity::IsAwake(int ipart)
{
	vectorf gravity = m_waterDensity>0 && (m_pos-m_waterPlane.origin)*m_waterPlane.n<0 ? m_waterGravity : m_gravity;
	return m_vel.len2()<sqr(500.0f) && m_pos.z>-1000.0f && m_bForceAwake!=0 && 
				 (m_bForceAwake==1 || (m_vel.len2()>sqr(m_pWorld->m_vars.minSeparationSpeed) || 
				 m_CollHistory[m_iCollHistory].age>1E9 && !m_bSliding && gravity.len2()>0));
}


void CParticleEntity::StartStep(float time_interval)
{
	m_timeStepPerformed = 0;
	m_timeStepFull = time_interval;
}
float CParticleEntity::GetMaxTimeStep(float time_interval)
{
	if (m_timeStepPerformed > m_timeStepFull-0.001f)
		return time_interval;
	return min_safe(m_timeStepFull-m_timeStepPerformed,time_interval);
}


int CParticleEntity::Step(float time_interval)
{
	ray_hit hits[8];
	pe_action_impulse ai;
	pe_action_register_coll_event arce;
	pe_status_dynamics sd;
	vectorf pos0,vel0,heading0,vtang,vel_next;
	float vn,vtang_len,rvtang_len,e,k,friction,vn0;
	int i,nhits,bHit,flags;
	vectorf gravity; float kAirResistance;
	if (m_waterDensity>0 && (m_pos-m_waterPlane.origin)*m_waterPlane.n<0) {
		gravity = m_waterGravity; kAirResistance = m_kWaterResistance;
	} else {
		gravity = m_gravity; kAirResistance = m_kAirResistance;
	}
	m_timeStepPerformed += time_interval;

	if (m_pColliderToIgnore && m_pColliderToIgnore->m_iSimClass==7)
		m_pColliderToIgnore = 0;

	for(i=0;i<sizeof(m_CollHistory)/sizeof(m_CollHistory[0]);i++)
		m_CollHistory[i].age += time_interval;

	if (IsAwake()) {
		FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );
		PHYS_ENTITY_PROFILER

		pos0 = m_pos; vel0 = m_vel;
		if (!m_bSliding)
			vel0 += gravity*time_interval*0.5f;
		flags = m_flags;
		m_pos += vel0*time_interval;

		if (m_bSliding) {
			if (m_pWorld->RayWorldIntersection(m_pos,m_slide_normal*(m_dim*-1.1f), ent_all,
					m_iPierceability|(geom_colltype0|geom_colltype_ray)<<rwi_colltype_bit, hits,1, m_pColliderToIgnore,this)) 
			{
				m_slide_normal = hits[0].n;	
				m_pos = hits[0].pt+m_slide_normal*m_dimLying;
				//if (m_dimLying!=m_dim)
				//	pos0 = m_pos;
				if (m_flags&particle_no_roll || m_slide_normal.z<0.5f) { // always slide if the slope is steep enough
					friction = max(0.0f, (m_pWorld->m_FrictionTable[m_surface_idx&NSURFACETYPES-1]+
						m_pWorld->m_FrictionTable[hits[0].surface_idx&NSURFACETYPES-1])*0.5f);
					if (m_slide_normal.z<0.5f)
						friction = min(1.0f,friction); // limit sliding friction on slopes
					vn = hits[0].n*vel0; vtang = vel0-hits[0].n*vn; vtang_len = vtang.len(); rvtang_len = vtang_len>1e-4 ? 1.0f/vtang_len:0;
					if (((CPhysicalEntity*)hits[0].pCollider)->m_iSimClass>1) {
						hits[0].pCollider->GetStatus(&sd);
						vn0 = sd.v*hits[0].n;
					} else vn0 = 0;
					m_vel = vel0 = hits[0].n*max(vn0,vn) + 
						vtang*(max(0.0f,vtang_len-max(0.0f,-(vn+(m_gravity*hits[0].n)*time_interval))*friction)*rvtang_len);
					m_wspin.zero(); m_qspin.SetIdentity();
					if (!(m_flags & particle_constant_orientation))
						(m_qrot = GetRotationV0V1(m_qrot*m_normal,hits[0].n)*m_qrot).Normalize();
						//m_qrot.SetRotationV0V1(m_normal,hits[0].n); //m_qrot = quaternionf(m_normal,hits[0].n);
					flags |= particle_constant_orientation;
				} else {
					friction = m_pWorld->m_FrictionTable[m_surface_idx&NSURFACETYPES-1];
					vel0 = m_vel = (m_vel-m_slide_normal*(m_vel*m_slide_normal))*max(0.0f,1.0f-time_interval*friction);
					m_wspin = m_slide_normal^m_vel*m_rdim;
				}
				if (m_flags & particle_single_contact)
					gravity.zero();
				else
					gravity -= m_slide_normal*(m_slide_normal*gravity);
				m_bForceAwake = ((CPhysicalEntity*)hits[0].pCollider)->m_iSimClass<=2 || m_timeForceAwake>40.0f ? -1:1;
			} else {
				m_bSliding = 0;
				if (!(m_flags & particle_constant_orientation))
					m_wspin = (m_heading^m_gravity).normalized()*((m_gravity*m_dirdown)*0.5f*m_rdim);
			}
		}
		
		m_vel += (gravity + m_heading*m_accThrust - m_vel*kAirResistance +
			(m_heading^(m_heading^m_dirdown)).normalize()*(m_kAccLift*m_vel.len()))*time_interval;
		(m_heading=m_vel).normalize();

		if (!(flags & particle_constant_orientation)) {
			if (!(m_flags & particle_no_spin)) {
				if (m_wspin.len2()*sqr(time_interval)<0.1f*0.1f) {
					m_qspin += quaternionf(0,m_wspin*0.5f)*m_qspin*time_interval;
					m_qspin.Normalize();
				} else {
					float wlen = m_wspin.len();
					//m_qspin = quaternionf(wlen*time_interval,m_wspin/wlen)*m_qspin;
					m_qspin = GetRotationAA(wlen*time_interval,m_wspin/wlen)*m_qspin;
				}
			} else
				m_wspin.zero();
			if (!(m_flags & particle_no_path_alignment)) {
				vectorf dirbuf[3]; 
				dirbuf[0] = m_dirdown^m_heading; dirbuf[1] = m_heading;
				if (dirbuf[0].len2()<0.01f) dirbuf[0].SetOrthogonal(m_heading);
				dirbuf[0].normalize(); dirbuf[2] = dirbuf[0]^dirbuf[1];
				m_qrot = m_qspin*quaternionf((matrix3x3CMf&)dirbuf[0]);
			} else
				m_qrot = m_qspin;
		}

		bHit = 0;
		if (m_pWorld->m_bWorldStep==2) {
			CPhysicalEntity **pentlist;
			pe_status_pos sp; sp.timeBack = 1;//time_interval;
			vectorf posFixed;
			int nents = m_pWorld->GetEntitiesAround(pos0-vectorf(m_dim,m_dim,m_dim),pos0+vectorf(m_dim,m_dim,m_dim),pentlist,ent_rigid);

			hits[0].dist = hits[1].dist = 1E10f;
			for(i=0;i<nents;i++) {
				pentlist[i]->GetStatus(&sp);
				posFixed = (pentlist[i]->m_qrot*!sp.q)*(pos0-sp.pos)+pentlist[i]->m_pos;
				if (bHit = (m_pWorld->RayTraceEntity(pentlist[i],posFixed,pos0-posFixed+(pos0-posFixed).normalized()*(m_dim*0.97f),hits+1) && 
					pentlist[i]->m_parts[hits[1].ipart].flags&geom_colltype0 && hits[1].dist<hits[0].dist))
					hits[0] = hits[1];
			}
			if (bHit)	{ // ignore collisions with moving bodies if they push us through statics
				heading0 = (hits[0].pt-pos0).normalized();
				bHit ^= m_pWorld->RayWorldIntersection(pos0,hits[0].pt-pos0+heading0*m_dim,ent_terrain|ent_static,
				 m_iPierceability|(geom_colltype0|geom_colltype_ray)<<rwi_colltype_bit|rwi_ignore_back_faces, hits,1, m_pColliderToIgnore,this);
			}
			if (nhits = bHit)
				pos0 = posFixed;
		}

		if (!bHit) {
			heading0 = (m_pos-pos0).normalized();
			nhits = m_pWorld->RayWorldIntersection(pos0,m_pos-pos0+heading0*m_dim, ent_all,
				m_iPierceability|(geom_colltype0|geom_colltype_ray)<<rwi_colltype_bit|rwi_ignore_back_faces, hits,8, m_pColliderToIgnore,this);
			bHit = isneg(-nhits) & (isneg(hits[0].dist+0.5f)^1);

			for(i=nhits-1+(bHit^1);i>=(bHit^1);i--) {	// register all hits in history 
				m_iCollHistory = m_iCollHistory+1 & sizeof(m_CollHistory)/sizeof(m_CollHistory[0])-1;
				RigidBody *pbody = ((CPhysicalEntity*)hits[i].pCollider)->GetRigidBody(hits[i].ipart);
				m_CollHistory[m_iCollHistory].pt = hits[i].pt-heading0*m_dim;	// store not contact, but position of particle center at the time of contact
				m_CollHistory[m_iCollHistory].n = hits[i].n;									// it's better for explosions to be created at some distance from the wall
				m_CollHistory[m_iCollHistory].v[0] = vel0;
				m_CollHistory[m_iCollHistory].v[1] = pbody->v+(pbody->w^m_CollHistory[m_iCollHistory].pt-pbody->pos);
				m_CollHistory[m_iCollHistory].mass[0] = m_mass;
				m_CollHistory[m_iCollHistory].mass[1] = pbody->M;
				m_CollHistory[m_iCollHistory].idCollider = m_pWorld->GetPhysicalEntityId(hits[i].pCollider);
				m_CollHistory[m_iCollHistory].partid[0] = 0;
				m_CollHistory[m_iCollHistory].partid[1] = hits[i].partid;
				m_CollHistory[m_iCollHistory].idmat[0] = m_surface_idx;
				m_CollHistory[m_iCollHistory].idmat[1] = hits[i].surface_idx;
				m_CollHistory[m_iCollHistory].age = 0;
			}
		}

		if (bHit) {
			e = max(min((m_pWorld->m_BouncinessTable[m_surface_idx] + m_pWorld->m_BouncinessTable[hits[0].surface_idx])*0.5f, 1.0f), 0.0f);
			float minv = ((CPhysicalEntity*)hits[0].pCollider)->GetMassInv();
			k = m_mass*minv/(1+m_mass*minv);
			if (((CPhysicalEntity*)hits[0].pCollider)->m_iSimClass>0) {
				RigidBody *pbody = ((CPhysicalEntity*)hits[0].pCollider)->GetRigidBody(hits[0].ipart);
				vectorf vrel = vel0-pbody->v-(pbody->w^hits[0].pt-pbody->pos);
				if (vel0.len2()>sqr(m_minBounceVel)) {
					ai.point = hits[0].pt;
					ai.impulse = vrel*(m_mass*pbody->M/(m_mass+pbody->M)*(1+e));
					ai.ipart = hits[0].ipart;
					hits[0].pCollider->Action(&ai);

					arce.pt = hits[0].pt;
					arce.n = hits[0].n;
					arce.v = m_vel;
					arce.collMass = m_mass;
					arce.pCollider = this;
					arce.partid[0] = hits[0].partid;
					arce.partid[1] = 0;
					arce.idmat[0] = hits[0].surface_idx;
					arce.idmat[1] = m_surface_idx;
					hits[0].pCollider->Action(&arce);
				}
			}

			vn = hits[0].n*vel0; vtang = vel0-hits[0].n*vn;
			if (vn>-m_minBounceVel || m_dimLying<m_dim*0.3f)
				e = 0;
			m_vel = vel0-hits[0].n*(vn*(1-k)*(1+e));
			if (sqr(m_dim)<(hits[0].pt-pos0).len2())
				m_pos = hits[0].pt - heading0*m_dim;
			else 
				m_pos = pos0;

			if (m_vel.len2()<sqr(m_pWorld->m_vars.minSeparationSpeed) || m_flags & particle_single_contact) {
				m_vel.zero(); m_wspin.zero(); m_qspin.SetIdentity();
				if (m_dim!=m_dimLying)
					m_pos = hits[0].pt+hits[0].n*m_dimLying;
				m_bSliding = 1; m_slide_normal = hits[0].n;
				if (!(flags & particle_constant_orientation))
					m_qrot = GetRotationV0V1(m_qrot*m_normal,hits[0].n)*m_qrot;
					//m_qrot = quaternionf(m_normal,hits[0].n);
			} else {
				vel_next = m_vel + (gravity + m_heading*m_accThrust - m_vel*kAirResistance +
					(m_heading^(m_heading^m_dirdown)).normalize()*(m_kAccLift*m_vel.len()))*time_interval;
				if (vel_next*hits[0].n<m_pWorld->m_vars.minSeparationSpeed) {
					if (m_dim!=m_dimLying)
						m_pos = hits[0].pt+hits[0].n*m_dimLying;
					m_bSliding = 1; m_slide_normal = hits[0].n;
					if (m_flags & particle_no_roll && !(m_flags & particle_constant_orientation))
						m_qrot = GetRotationV0V1(m_qrot*m_normal,hits[0].n)*m_qrot;
						//m_qrot = quaternionf(m_normal,hits[0].n);
				}

				if (m_flags & particle_no_roll) m_wspin.zero();
				else m_wspin = (hits[0].n^vtang)*m_rdim;
			}

			if (((CPhysicalEntity*)hits[0].pCollider)->m_iSimClass>1) {
				hits[0].pCollider->GetStatus(&sd);
				m_vel += hits[0].n*(sd.v*hits[0].n);
			}

			m_bForceAwake = ((CPhysicalEntity*)hits[0].pCollider)->m_iSimClass<=2 || m_timeForceAwake>40.0f ? -1:1;
		}
		i = (m_bForceAwake ^ m_bForceAwake>>31) & 1;
		m_timeForceAwake = m_timeForceAwake*i + time_interval*i;
		m_sleepTime = 0;

		m_BBox[0] = m_pos-vectorf(m_dim,m_dim,m_dim);
		m_BBox[1] = m_pos+vectorf(m_dim,m_dim,m_dim);
		if (m_flags & particle_traceable)
			m_pWorld->RepositionEntity(this, 1);
	} else
		m_sleepTime += time_interval;
	
	return 1;
}


int CParticleEntity::RayTrace(CRayGeom *pRay,geom_contact *&pcontacts)
{
	static geom_contact g_ParticleContact;
	prim_inters inters;
	box abox;
	abox.Basis.SetRow(2,m_qrot*m_normal);
	abox.Basis.SetRow(0,m_qrot*GetOrthogonal(m_normal).normalized());
	abox.Basis.SetRow(1,abox.Basis.GetRow(2)^abox.Basis.GetRow(0));
	abox.size(m_dim,m_dim,m_dimLying);
	abox.center = m_pos;

	if (box_ray_intersection(&abox,&pRay->m_ray,&inters)) {
		pcontacts = &g_ParticleContact;
		pcontacts->pt = inters.pt[0];
		pcontacts->t = (inters.pt[0]-pRay->m_ray.origin)*pRay->m_dirn;
		pcontacts->id[0] = m_surface_idx;
		pcontacts->iNode[0] = 0;
		pcontacts->n = inters.n;
		return 1;
	}
	return 0;
}


void CParticleEntity::DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags)
{
	CPhysicalEntity::DrawHelperInformation(DrawLineFunc,flags);

	if (flags & pe_helper_geometry) {
		box abox;
		abox.Basis.SetRow(2,m_qrot*m_normal);
		abox.Basis.SetRow(0,m_qrot*GetOrthogonal(m_normal));
		abox.Basis.SetRow(1,abox.Basis.GetRow(2)^abox.Basis.GetRow(0));
		abox.center = m_pos;
		int i,j;
		vectorf pt[8];
		for(i=0;i<8;i++)
			pt[i] = vectorf(m_dim*((i<<1&2)-1),m_dim*((i&2)-1),m_dimLying*((i>>1&2)-1))*abox.Basis+abox.center;
		for(i=0;i<8;i++) for(j=0;j<3;j++) if (i&1<<j)
			DrawLineFunc(pt[i],pt[i^1<<j]);
	}
}


void CParticleEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	CPhysicalEntity::GetMemoryStatistics(pSizer);
	if (GetType()==PE_PARTICLE)
		pSizer->AddObject(this, sizeof(CParticleEntity));
}