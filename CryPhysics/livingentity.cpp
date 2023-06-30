//////////////////////////////////////////////////////////////////////
//
//	Living Entity
//	
//	File: livingentity.cpp
//	Description : CLivingEntity class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "singleboxtree.h"
#include "cylindergeom.h"
#include "spheregeom.h"
#include "raybv.h"
#include "raygeom.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "geoman.h"
#include "physicalworld.h"
#include "livingentity.h"


inline Vec3_tpl<short> EncodeVec6b(const vectorf &vec) {
	return Vec3_tpl<short>(
		float2int(min_safe(100.0f,max_safe(-100.0f,vec.x))*(32767/100.0f)),
		float2int(min_safe(100.0f,max_safe(-100.0f,vec.y))*(32767/100.0f)),
		float2int(min_safe(100.0f,max_safe(-100.0f,vec.z))*(32767/100.0f)));
}
inline vectorf DecodeVec6b(const Vec3_tpl<short> &vec) {
	return vectorf(vec.x,vec.y,vec.z)*(100.0f/32767);
}

struct vec4b {
	short yaw;
	char pitch;
	unsigned char len;
};
inline vec4b EncodeVec4b(const vectorf &vec) {
	vec4b res = { 0,0,0 };
	if (vec.len2()>sqr(1.0f/512)) {
		float len = vec.len();
		res.yaw = float2int(atan2_tpl(vec.y,vec.x)*(32767/pi));
		res.pitch = float2int(asin_tpl(vec.z/len)*(127/pi));
		res.len = float2int(min(20.0f,len)*(255/20.0f));
	}
	return res;
}
inline vectorf DecodeVec4b(const vec4b &vec) {
	float yaw=vec.yaw*(pi/32767), pitch=vec.pitch*(pi/127), len=vec.len*(20.0f/255);
	float t = cos_tpl(pitch);
	return vectorf(t*cos_tpl(yaw),t*sin_tpl(yaw),sin_tpl(pitch))*len;
}


CLivingEntity::CLivingEntity(CPhysicalWorld *pWorld) : CPhysicalEntity(pWorld)
{
	m_vel.zero(); m_velRequested.zero(); 
	if (pWorld) 
		m_gravity = pWorld->m_vars.gravity; 
	else m_gravity.Set(0,0,-9.81f);
	m_bFlying = 1; m_bJumpRequested = 0; 
	m_iSimClass = 3;
	m_dh=m_dhSpeed=m_dhAcc=0; m_stablehTime=1; 
	m_hLatest=0;
	m_timeFlying=m_minFlyTime = 0;
	m_nslope.Set(0,0,1);
	m_mass = 80; m_massinv = 2.0f/80;
	m_surface_idx = 0;
	m_slopeSlide = cos_tpl(pi*0.2f);
	m_slopeJump = m_slopeClimb = cos_tpl(pi*0.3f);
	m_slopeFall = cos_tpl(pi*0.39f);
	m_nodSpeed = 60.0f;

	m_size.Set(0.4f,0.4f,0.6f);
	m_hCyl = 1.1f;
	m_hEye = 1.7f;
	m_hPivot = 0;

	cylinder cyl;
	cyl.axis.Set(0,0,1); cyl.center.zero();
	cyl.hh = m_size.z; cyl.r = m_size.x;
	m_CylinderGeom.CreateCylinder(&cyl);
	m_CylinderGeom.CalcPhysicalProperties(&m_CylinderGeomPhys);
	m_CylinderGeomPhys.nRefCount = -1;
	sphere sph;
	sph.center.Set(0,0,-m_size.z);
	sph.r = m_size.x;
	m_SphereGeom.CreateSphere(&sph);
	sph.center.zero();
	sph.r = 0.2f;
	m_HeadGeom.CreateSphere(&sph);
	m_hHead = 1.9f;
	m_parts[0].pPhysGeom = m_parts[0].pPhysGeomProxy = &m_CylinderGeomPhys;
	m_parts[0].id = 100;
	m_parts[0].pos.Set(0,0,m_hCyl-m_hPivot);
	m_parts[0].q.SetIdentity();
	m_parts[0].scale = 1.0f;
	m_parts[0].mass = m_mass;
	m_parts[0].surface_idx = m_surface_idx;
	m_parts[0].flags = geom_collides;
	m_parts[0].flagsCollider = geom_colltype_player;
	m_parts[0].minContactDist = m_size.z*0.1;
	m_nParts = 1;

	m_kInertia = 8;
	m_kAirControl = 0.1f;
	m_kAirResistance = 0;
	m_bSwimming = 0;
	m_nSensors = 0;
	m_pSensors = 0;
	m_pSensorsPoints = 0;
	m_pSensorsSlopes = 0;
	m_iSensorsActive = 0;
	m_pLastGroundCollider = 0;

	m_iHist=0;
	int i;
	m_history = m_history_buf; m_szHistory = sizeof(m_history_buf)/sizeof(m_history_buf[0]);
	m_actions = m_actions_buf; m_szActions = sizeof(m_actions_buf)/sizeof(m_actions_buf[0]);
	for(i=0;i<m_szHistory;i++) {
		m_history[i].dt = 1E10;
		m_history[i].pos.zero(); m_history[i].v.zero(); m_history[i].bFlying=0;
		m_history[i].q.SetIdentity(); 
		m_history[i].timeFlying = 0;
		m_history[i].idCollider = -2; m_history[i].posColl.zero();
	}
	for(i=0;i<m_szActions;i++) {
		m_actions[i].dt = 1E10;
		MARK_UNUSED m_actions[i].dir; m_actions[i].iJump=0;
	}
	m_iAction = 0;
	m_bIgnoreCommands = 0;
	m_bStateReading = 0;
	m_flags |= lef_push_players | lef_push_objects;
	m_velGround.zero();
	m_timeUseLowCap = -1.0f;
	m_bActive = 1;
	m_timeSinceStanceChange = 0;
	m_bActiveEnvironment = 0;
	m_collision.age = 1E10f;
	m_dhHist[0] = m_dhHist[1] = 0;
	m_timeOnStairs = 0;
	m_timeForceInertia = 0;
	m_deltaPos.zero();
	m_timeSmooth = 0;
	m_bUseSphere = 0;
	m_timeSinceImpulseContact = 10.0f;
	m_maxVelGround = 8.0f;
	m_bStuck = 0;
	m_nContacts = m_nContactsAlloc = 0;
	m_pContacts = 0;
	m_iSnapshot = 0;
	m_iTimeLastSend = -1;
}

CLivingEntity::~CLivingEntity()
{
	m_parts[0].pPhysGeom = m_parts[0].pPhysGeomProxy = 0;
	if (m_pSensors) delete[] m_pSensors;
	if (m_pSensorsPoints) delete[] m_pSensorsPoints;
	if (m_pSensorsSlopes) delete[] m_pSensorsSlopes;
	if (m_history!=m_history_buf) delete[] m_history;
	if (m_actions!=m_actions_buf) delete[] m_actions;
	if (m_pContacts) delete[] m_pContacts;
}																					


void CLivingEntity::ReleaseGroundCollider()
{
	if (m_pLastGroundCollider) {
		m_pLastGroundCollider->Release();
		m_pLastGroundCollider = 0;
	}
}

void CLivingEntity::SetGroundCollider(CPhysicalEntity *pCollider)
{
	ReleaseGroundCollider();
	if (pCollider && pCollider->m_iSimClass>0) {
		pCollider->AddRef();
		m_pLastGroundCollider = pCollider;
	}
}

int CLivingEntity::Awake(int bAwake,int iSource)
{ 
	if (m_pLastGroundCollider && m_pLastGroundCollider->m_iSimClass==7)
		ReleaseGroundCollider();
	m_bActiveEnvironment = bAwake; 
	return 1; 
}


int CLivingEntity::IsPositionFree(const vectorf *BBox,float newh,const vectorf &newdim)
{
	int i,j,nents;
	geom_world_data gwd[2];
	intersection_params ip;
	CPhysicalEntity **pentlist;
	geom_contact *pcontacts;

	nents = m_pWorld->GetEntitiesAround(BBox[0],BBox[1], pentlist, ent_terrain|ent_static|ent_sleeping_rigid|ent_rigid|ent_living);
	//m_qrot.getmatrix(gwd[0].R); //gwd[0].R.identity(); //Q2M_IVO 
	gwd[0].R = matrix3x3f(m_qrot); 
	gwd[0].offset = m_pos + gwd[0].R*vectorf(0,0,newh); 
	//(gwd[0].offset = m_pos).z += params->heightCollider-params->heightPivot;
	ip.vrel_min = 1E10;
	for(i=0;i<nents;i++) if (pentlist[i]!=this) {
		if (pentlist[i]->GetType()!=PE_LIVING) {
			for(j=0;j<pentlist[i]->m_nParts;j++) if (pentlist[i]->m_parts[j].flags & geom_colltype_player) {
				//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(gwd[1].R); //Q2M_IVO 
				gwd[1].R = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
				gwd[1].offset = pentlist[i]->m_pos + pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
				gwd[1].scale = pentlist[i]->m_parts[j].scale;
				if (m_CylinderGeom.Intersect(pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom, gwd,gwd+1, &ip, pcontacts))
					return 0;
			}
		} else {
			CLivingEntity *pent = (CLivingEntity*)pentlist[i];
			if (fabs_tpl((m_pos.z+newh)-(pent->m_pos.z+pent->m_hCyl-pent->m_hPivot)) < newdim.z+pent->m_size.z &&
					(vector2df(m_pos)-vector2df(pent->m_pos)).len2() < sqr(newdim.x+pent->m_size.x))
				return 0;
		}
	}
	return 1;
}


int CLivingEntity::SetParams(pe_params *_params)
{
	int res;
	vectorf prevpos=m_pos;
	quaternionf prevq = m_qrot;
	if (res = CPhysicalEntity::SetParams(_params)) {
		if (_params->type==pe_params_pos::type_id) {
			if ((prevpos-m_pos).len2()>sqr(m_size.z*0.01)) {
				ReleaseGroundCollider(); m_bFlying = 1;
				m_vel.zero(); m_dh=0;
				for(int i=0;i<m_szHistory;i++) m_history[i].pos = m_pos;
			}
			if (sqr(m_qrot.v*prevq.v)<m_qrot.v.len2()*prevq.v.len2()*sqr(0.998f) && ((pe_params_pos*)_params)->bRecalcBounds && 
					m_bActive && !IsPositionFree(m_BBox,m_hCyl-m_hPivot,m_size)) 
			{
				m_qrot = prevq; ComputeBBox();
				return 0;
			}
			/*if (m_qrot.s!=prevq.s || m_qrot.v!=prevq.v) {
				vectorf angles;
				m_qrot.get_Euler_angles_xyz(angles);
				m_qrot = quaternionf(angles.z,vectorf(0,0,1));
			}*/
			//m_BBox[0].z = min(m_BBox[0].z,m_pos.z-m_hPivot);
		}
		return res;
	}
	
	if (_params->type==pe_player_dimensions::type_id) {
		pe_player_dimensions *params = (pe_player_dimensions*)_params;
		ENTITY_VALIDATE("CLivingEntity:SetParams(player_dimensions)",params);
		if (is_unused(params->sizeCollider)) params->sizeCollider = m_size;
		if (is_unused(params->heightCollider)) params->heightCollider = m_hCyl;
		if (is_unused(params->heightPivot)) params->heightPivot = m_hPivot;

		if ((params->heightCollider!=m_hCyl || params->sizeCollider.x!=m_size.x || params->sizeCollider.z!=m_size.z) && m_pWorld) {
			cylinder newdim,prevdim = m_CylinderGeom.m_cyl;
			newdim.r = params->sizeCollider.x;
			newdim.hh = params->sizeCollider.z;
			newdim.center.zero();
			newdim.axis.Set(0,0,1);
			m_CylinderGeom.CreateCylinder(&newdim);
			vectorf BBox[2];
			BBox[0].Set(-params->sizeCollider.x,-params->sizeCollider.x,-params->heightPivot) += m_pos;
			BBox[1].Set(params->sizeCollider.x,params->sizeCollider.x,params->sizeCollider.z+params->heightCollider-params->heightPivot) += m_pos;

			if (m_bActive && !IsPositionFree(BBox,params->heightCollider-params->heightPivot,params->sizeCollider)) {
				m_CylinderGeom.CreateCylinder(&prevdim); 
				return 0;
			}
			m_hCyl = params->heightCollider;
			m_parts[0].pos.Set(0,0,m_hCyl-m_hPivot);
			m_size.Set(params->sizeCollider.x, params->sizeCollider.x, params->sizeCollider.z);
			m_CylinderGeom.m_Tree.m_Box.size = m_size; 
			sphere sph;
			sph.center.Set(0,0,-m_size.z);
			sph.r = m_size.x;
			m_SphereGeom.CreateSphere(&sph);
			m_SphereGeom.m_Tree.m_Box.center = sph.center;
			m_SphereGeom.m_Tree.m_Box.size = vectorf(sph.r,sph.r,sph.r); 
		}

		if (!is_unused(params->heightEye)) {
			if (m_hEye!=params->heightEye) {
				m_dh += params->heightEye-m_hEye;
				m_dhSpeed = 4*(params->heightEye-m_hEye);
			}
			m_hEye = params->heightEye;
			m_timeSinceStanceChange = 0;
		}

		if (!is_unused(params->heightHead)) m_hHead = params->heightHead;
		if (!is_unused(params->headRadius)) {
			sphere sph;
			sph.center.zero(); sph.r = params->headRadius;
			m_HeadGeom.CreateSphere(&sph);
		}
		m_BBox[0].z = min(m_BBox[0].z,m_pos.z-m_hPivot);
		if (m_HeadGeom.m_sphere.r>0)
			m_BBox[1].z = max(m_BBox[1].z,m_pos.z-m_hPivot+m_hHead+m_HeadGeom.m_sphere.r);

		return 1;
	}

	if (_params->type==pe_player_dynamics::type_id) {
		pe_player_dynamics *params = (pe_player_dynamics*)_params;
		if (!is_unused(params->kInertia)) m_kInertia = params->kInertia;
		if (!is_unused(params->kAirControl)) m_kAirControl = params->kAirControl;
		if (!is_unused(params->kAirResistance)) m_kAirResistance = params->kAirResistance;
		if (!is_unused(params->gravity)) m_gravity.Set(0,0,-params->gravity);
		if (!is_unused(params->nodSpeed)) m_nodSpeed = params->nodSpeed;
		if (!is_unused(params->bSwimming)) m_bSwimming = params->bSwimming;
		if (!is_unused(params->mass)) { m_mass = params->mass; m_massinv = 2.0f/m_mass; }
		if (!is_unused(params->surface_idx))
			m_surface_idx = m_parts[0].surface_idx = params->surface_idx;
		if (!is_unused(params->minSlideAngle)) m_slopeSlide = cos_tpl(params->minSlideAngle*(pi/180.0f));
		if (!is_unused(params->maxClimbAngle)) m_slopeClimb = cos_tpl(params->maxClimbAngle*(pi/180.0f));
		if (!is_unused(params->maxJumpAngle)) m_slopeJump = cos_tpl(params->maxJumpAngle*(pi/180.0f));
		if (!is_unused(params->minFallAngle))	m_slopeFall = cos_tpl(params->minFallAngle*(pi/180.0f));
		if (!is_unused(params->maxVelGround))	m_maxVelGround = params->maxVelGround;
		if (!is_unused(params->bActive)) {
			if (m_bActive+params->bActive*2==2)
				m_bFlying = 1;
			m_bActive = params->bActive;
		}
		if (!is_unused(params->bNetwork) && params->bNetwork)
			AllocateExtendedHistory();
		return 1;
	}

	if (_params->type==pe_params_sensors::type_id) {
		pe_params_sensors *params = (pe_params_sensors*)_params;
		if (m_nSensors!=params->nSensors)
			m_bActiveEnvironment = 1;
		if (m_pSensors && m_nSensors!=params->nSensors) {
			delete[] m_pSensors; m_pSensors=0;
			delete[] m_pSensorsPoints; m_pSensorsPoints=0;
			delete[] m_pSensorsSlopes; m_pSensorsSlopes=0;
		}
		m_nSensors = params->nSensors;
		if (!m_pSensors && m_nSensors>0) {
			memset(m_pSensors = new vectorf[m_nSensors],0,m_nSensors*sizeof(vectorf));
			m_pSensorsPoints = new vectorf[m_nSensors];
			m_pSensorsSlopes = new vectorf[m_nSensors];
		}
		for(int i=0; i<m_nSensors; i++)	{
			if ((m_pSensors[i]-params->pOrigins[i]).len2()>sqr(0.01f)) {
				m_bActiveEnvironment = 1;
				(m_pSensorsPoints[i] = m_pos+m_qrot*m_pSensors[i]).z = m_pos.z;
				m_pSensorsSlopes[i].Set(0,0,1);
				m_iSensorsActive &= ~(1<<i);
			}
			m_pSensors[i] = params->pOrigins[i];
		}
		return 1;
	}

	return 0;
}


int CLivingEntity::GetParams(pe_params *_params)
{
	int res;
	if (res = CPhysicalEntity::GetParams(_params))
		return res;
	
	if (_params->type==pe_player_dimensions::type_id) {
		pe_player_dimensions *params = (pe_player_dimensions*)_params;
		params->heightPivot = m_hPivot;
		params->sizeCollider = m_size;
		params->heightCollider = m_hCyl;
		params->heightEye = m_hEye; 
		params->headRadius = m_HeadGeom.m_sphere.r;
		params->heightHead = m_hHead;
		return 1;
	}

	if (_params->type==pe_player_dynamics::type_id) {
		pe_player_dynamics *params = (pe_player_dynamics*)_params;
		params->kInertia = m_kInertia;
		params->kAirControl = m_kAirControl;
		params->gravity = -m_gravity.z;
		params->nodSpeed = m_nodSpeed;
		params->bSwimming = m_bSwimming;
		params->mass = m_mass;
		params->surface_idx = m_surface_idx;
		params->minSlideAngle = cry_acosf(m_slopeSlide)*(180.0f/pi);
		params->maxClimbAngle = cry_acosf(m_slopeClimb)*(180.0f/pi);
		params->maxJumpAngle = cry_acosf(m_slopeJump)*(180.0f/pi);
		params->minFallAngle = cry_acosf(m_slopeFall)*(180.0f/pi);
		params->maxVelGround = m_maxVelGround;
		params->bActive = m_bActive;
		params->bNetwork = m_szHistory==SZ_HISTORY;
		return 1;
	}

	if (_params->type==pe_params_sensors::type_id) {
		pe_params_sensors *params = (pe_params_sensors*)_params;
		if (m_pSensors && m_nSensors!=params->nSensors) {
			delete[] m_pSensors; m_pSensors=0;
			delete[] m_pSensorsPoints; m_pSensorsPoints=0;
			delete[] m_pSensorsSlopes; m_pSensorsSlopes=0;
		}
		m_nSensors = params->nSensors;
		if (!m_pSensors && m_nSensors>0) {
			m_pSensors = new vectorf[m_nSensors];
			m_pSensorsPoints = new vectorf[m_nSensors];
			m_pSensorsSlopes = new vectorf[m_nSensors];
		}
		for(int i=0; i<m_nSensors; i++)	m_pSensors[i]=params->pOrigins[i];
		return 1;
	}
	return 0;
}


void CLivingEntity::AllocateExtendedHistory()
{
	int i;
	if (m_history==m_history_buf) {
		m_history = new le_history_item[SZ_HISTORY];
		for(i=0;i<m_szHistory;i++) m_history[i] = m_history_buf[i];
		m_szHistory = SZ_HISTORY;
		for(;i<m_szHistory;i++) {
			m_history[i].dt = 1E10;
			m_history[i].pos.zero(); m_history[i].v.zero(); m_history[i].bFlying=0;
			m_history[i].q.SetIdentity(); 
			m_history[i].timeFlying = 0;
			m_history[i].idCollider = -2; m_history[i].posColl.zero();
		}
	}

	if (m_actions==m_actions_buf) {
		m_actions = new pe_action_move[SZ_ACTIONS];
		for(i=0;i<m_szActions;i++) m_actions[i] = m_actions_buf[i];
		m_szActions = SZ_ACTIONS;
		for(;i<m_szActions;i++) {
			m_actions[i].dt = 1E10;
			MARK_UNUSED m_actions[i].dir; m_actions[i].iJump=0;
		}
	}
}


int CLivingEntity::AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id)
{
	m_parts[0].flags &= ~geom_colltype_ray;
	return CPhysicalEntity::AddGeometry(pgeom,params,id);
}


void CLivingEntity::RemoveGeometry(int id)
{
	CPhysicalEntity::RemoveGeometry(id);
	if (m_nParts==1)
		m_parts[0].flags |= geom_colltype_ray;
}


int CLivingEntity::GetStatus(pe_status *_status)
{
	if (_status->type==pe_status_pos::type_id) {
		vectorf prevPos = m_pos;
		if (m_bActive)
			m_pos += m_deltaPos*(m_timeSmooth*(1/0.3f));
		int res = CPhysicalEntity::GetStatus(_status);
		m_pos = prevPos;
		return res;
	}

	if (CPhysicalEntity::GetStatus(_status))
		return 1;

	if (_status->type==pe_status_living::type_id) {
		pe_status_living *status = (pe_status_living*)_status;
		status->bFlying = m_bFlying & m_bActive;
		status->timeFlying = m_timeFlying;
		status->camOffset = m_qrot*vectorf(0,0,-m_hPivot+m_hEye-m_dh);

		int idx,len; float dt;
		for(idx=m_iHist,len=0,dt=0; len<3 && m_history[idx-1&m_szHistory-1].dt<1E9; idx=idx-1&m_szHistory-1,len++)
			dt += m_history[idx].dt;
		if (idx==m_iHist || (status->vel = (m_history[m_iHist].pos-m_history[idx].pos)/dt).len2()>sqr(6.0f)) {
			status->vel = m_vel;
			if (m_pLastGroundCollider)
				status->vel += m_velGround;
		}

		status->velUnconstrained = m_vel;
		status->velRequested = m_velRequested;
		status->velGround = m_velGround;
		status->groundHeight = m_hLatest;
		status->groundSlope = m_nslope;
		status->groundSurfaceIdx = m_lastGroundSurfaceIdx;
		status->pGroundCollider = m_pLastGroundCollider;
		status->iGroundColliderPart = m_iLastGroundColliderPart;
		status->timeSinceStanceChange = m_timeSinceStanceChange;
		status->bOnStairs = isneg(0.1f-m_timeOnStairs);
		return 1;
	} else

	if (_status->type==pe_status_sensors::type_id) {
		pe_status_sensors *status = (pe_status_sensors*)_status;
		status->pPoints = m_pSensorsPoints;
		status->pNormals = m_pSensorsSlopes;
		status->flags = m_iSensorsActive;
		return m_nSensors;
	}

	if (_status->type==pe_status_dynamics::type_id) {
		pe_status_dynamics *status = (pe_status_dynamics*)_status;
		status->v = m_vel;
		if (m_pLastGroundCollider)
			status->v += m_velGround;
		float rdt = 1.0f/m_history[m_iHist].dt;
		status->a = (m_history[m_iHist].v-m_history[m_iHist-1&m_szHistory-1].v)*rdt;
		status->w = (m_history[m_iHist].q.v-m_history[m_iHist-1&m_szHistory-1].q.v)*2*rdt;
		status->centerOfMass = (m_BBox[0]+m_BBox[1])*0.5f;
		status->mass = m_mass;
		/*for(idx=idx-1&m_szHistory-1; dt<status->time_interval; idx=idx-1&m_szHistory-1) {
			rdt = 1.0f/m_history[idx].dt; dt += m_history[idx].dt;
			status->a += (m_history[idx].v-m_history[idx-1&m_szHistory-1].v)*rdt; 
			status->w += (m_history[idx].q.v-m_history[idx-1&m_szHistory-1].q.v)*2*rdt;
			status->v += m_history[idx].v;
		}
		if (dt>0) {
			rdt = 1.0f/dt;
			status->v *= dt;
			status->a *= dt;
			status->w *= dt;
		}*/
		return 1;
	}

	if (_status->type==pe_status_timeslices::type_id) {
		pe_status_timeslices *status = (pe_status_timeslices*)_status;
		if (!status->pTimeSlices) return 0;
		int i,nSlices=0; float dt;
		if (is_unused(status->time_interval)) {
			if (m_actions[m_iAction].dt>1E9)
				return 0;
			status->time_interval = m_actions[m_iAction].dt;
		}

		for(i=m_iHist,dt=status->time_interval; dt>status->precision; dt-=m_history[i].dt,i=i-1&m_szHistory-1);
		for(i=i+1&m_szHistory-1; i!=(m_iHist+1&m_szHistory-1) && nSlices<status->sz; i=i+1&m_szHistory-1)
			status->pTimeSlices[nSlices++] = m_history[i].dt;
		return nSlices;
	}

	if (_status->type==pe_status_collisions::type_id) {
		pe_status_collisions *status = (pe_status_collisions*)_status;
		if (m_collision.age<status->age) {
			status->pHistory[0] = m_collision;
			if (status->bClearHistory)
				m_collision.age = 1E10f;
			return 1;
		}
		return 0;
	}

	return 0;
}


int CLivingEntity::Action(pe_action* _action)
{
	int res;
	if (res = CPhysicalEntity::Action(_action))
		return res; 

	if (_action->type==pe_action_move::type_id) {
		if (!m_bIgnoreCommands) {
			pe_action_move *action = (pe_action_move*)_action;
			bool bForceHistory = false;
			ENTITY_VALIDATE("CLivingEntity:Action(action_move)",action);
			if (!is_unused(action->dir)) {
				m_velRequested.zero();
				if (m_kAirControl==1) {
					m_velRequested = m_vel = action->dir;
					m_bJumpRequested = action->iJump;
				} else if (!action->iJump) {
					m_velRequested = action->dir;
					//if (m_bFlying && m_vel.len2()>0 && !m_bSwimming && !m_pWorld->m_vars.bFlyMode) 
					//	m_velRequested -= m_vel*((action->dir*m_vel)/m_vel.len2());
				} else {
					if (action->iJump==2) {
						m_vel += action->dir; bForceHistory = true;
						if (action->dir.z>1.0f) {
							m_minFlyTime=0.2f; m_timeFlying=0;
							m_bJumpRequested = 1;
						}
					}	else if (m_bFlying) {
						if (m_pWorld->m_vars.bFlyMode) 
							m_velRequested.Set(0,0,2);
					} else if (m_bStuck || !(m_nslope.z<m_slopeJump && (m_nslope^(m_nslope^action->dir)).z<0)) {
						m_vel = action->dir; bForceHistory = true;
						if (!m_bStuck && m_nslope.z<m_slopeJump)
							m_vel.z = min(m_vel.z,0.0f);
						m_minFlyTime=0.2f; m_timeFlying=0;
						m_bJumpRequested = 1;
					}
				}
				if (m_flags & lef_snap_velocities) {
					m_velRequested = DecodeVec4b(EncodeVec4b(m_velRequested));
					m_vel = DecodeVec6b(EncodeVec6b(m_vel));
				}
				if (bForceHistory)
					m_history[m_iHist].v = m_vel;
			} 
			m_actions[++m_iAction&=m_szActions-1] = *action;
			return 1;
		}
		return 0;
	}	
	
	if (_action->type==pe_action_impulse::type_id) {
		pe_action_impulse *action = (pe_action_impulse*)_action;
		ENTITY_VALIDATE("CLivingEntity:Action(action_impulse)",action);
		vectorf impulse = action->impulse;
		if (action->iSource==2) // explosion
			impulse *= 0.3f;
		if (action->iApplyTime==0) {
			//if (!m_bFlying) m_velRequested.zero();
			m_vel += impulse*(m_massinv*0.5f);
			if (m_flags & lef_snap_velocities)
				m_vel = DecodeVec6b(EncodeVec6b(m_vel));
			if (impulse.z*m_massinv*0.5f>1.0f) {
				m_bFlying=1; m_minFlyTime=0.2f; m_timeFlying=0;
			}
		} else {
			pe_action_move am;
			am.dir = impulse*(m_massinv*0.5f);
			am.iJump = 2;
			Action(&am);
		}
		/*if (!m_bFlying && action->impulse.z<0 && m_dh<m_size.z*0.1f) {
			m_dhSpeed = action->impulse.z*m_massinv;
			m_dhAcc = max(sqr(m_dhSpeed)/(m_size.z*2), m_nodSpeed*0.15f);
		}*/
		if (m_kInertia==0)
			m_timeForceInertia = 5.0f;
		return 1;
	}
	
	if (_action->type==pe_action_reset::type_id) {
		m_vel.zero(); m_velRequested.zero();
		m_dh = m_dhSpeed = m_stablehTime = 0;
		m_bFlying = 1;
		return 1;
	}

	return 0;
}


int CLivingEntity::GetStateSnapshot(CStream &stm,float time_back,int flags)
{
	vectorf pos_prev=m_pos, vel_prev=m_vel, nslope_prev=m_nslope;
	int bFlying_prev=m_bFlying;
	float timeFlying_prev=m_timeFlying, minFlyTime_prev=m_minFlyTime, timeUseLowCap_prev=m_timeUseLowCap;
	quaternionf qrot_prev=m_qrot;
	CPhysicalEntity *pLastGroundCollider=m_pLastGroundCollider;
	int iLastGroundColliderPart=m_iLastGroundColliderPart;
	vectorf posLastGroundColl=m_posLastGroundColl;
	ReleaseGroundCollider();

	if (time_back>0) {
		AllocateExtendedHistory();
		StepBackEx(time_back,false);
	}
	stm.WriteNumberInBits(SNAPSHOT_VERSION,4);
	if (m_pWorld->m_vars.bMultiplayer) {
		WriteCompressedPos(stm,m_pos,(m_id+m_iSnapshot&31)!=0);
		if (m_pWorld->m_iTimePhysics!=m_iTimeLastSend) {
			m_iSnapshot++; m_iTimeLastSend = m_pWorld->m_iTimePhysics;
		}
	} else
		stm.Write(m_pos);

	if (m_flags & lef_snap_velocities) {
		if (m_vel.len2()>0) {
			stm.Write(true);
			Vec3_tpl<short> v = EncodeVec6b(m_vel);
			stm.Write(v.x); stm.Write(v.y); stm.Write(v.z);
		} else stm.Write(false);
		if (m_velRequested.len2()>0) {
			stm.Write(true);
			vec4b vr = EncodeVec4b(m_velRequested);
			stm.Write(vr.yaw); stm.Write(vr.pitch);	stm.Write(vr.len);
		} else stm.Write(false);
	} else {
		if (m_vel.len2()>0) {
			stm.Write(true);
			stm.Write(m_vel);
		} else stm.Write(false);
		if (m_velRequested.len2()>0) {
			stm.Write(true);
			stm.Write(m_velRequested);
		} else stm.Write(false);
	}
	if (m_timeFlying>0) {
		stm.Write(true);
		stm.Write((unsigned short)float2int(m_timeFlying*6553.6f));
	} else stm.Write(false);
	unsigned int imft = isneg(0.1f-m_minFlyTime)+isneg(0.35f-m_minFlyTime)+isneg(0.75f-m_minFlyTime); // snap to 0..0.2..0.5..1
	stm.WriteNumberInBits(imft,2);
	stm.Write((bool)(m_bFlying!=0));

	/*if (m_pLastGroundCollider) {
		stm.Write(true);
		WritePacked(stm, m_pWorld->GetPhysicalEntityId(m_pLastGroundCollider)+1);
		WritePacked(stm, m_iLastGroundColliderPart);
		stm.Write((Vec3&)m_posLastGroundColl);
	} else
		stm.Write(false);*/

	m_pos=pos_prev; m_vel=vel_prev; m_bFlying=bFlying_prev; m_qrot=qrot_prev; 
	m_timeFlying=timeFlying_prev; m_minFlyTime=minFlyTime_prev; m_timeUseLowCap = timeUseLowCap_prev;
	m_nslope=nslope_prev;
	SetGroundCollider(pLastGroundCollider);
	m_iLastGroundColliderPart=iLastGroundColliderPart;
	m_posLastGroundColl=posLastGroundColl;
	
	return 1;
}


int CLivingEntity::SetStateFromSnapshot(CStream &stm, int flags)
{
	bool bnz,bCompressed;
	unsigned short tmp;
	int ver=0;
	stm.ReadNumberInBits(ver,4);
	if (ver!=SNAPSHOT_VERSION)
		return 0;

	if (flags & ssf_no_update) {
		vectorf dummy;
		if (m_pWorld->m_vars.bMultiplayer)
			ReadCompressedPos(stm,dummy,bCompressed);
		else
			stm.Seek(stm.GetReadPos()+sizeof(vectorf)*8);
		stm.Read(bnz); if (bnz) stm.Seek(stm.GetReadPos()+((m_flags & lef_snap_velocities) ? 6:sizeof(vectorf))*8);
		stm.Read(bnz); if (bnz) stm.Seek(stm.GetReadPos()+((m_flags & lef_snap_velocities) ? 4:sizeof(vectorf))*8);
		stm.Read(bnz); if (bnz) stm.Seek(stm.GetReadPos()+sizeof(tmp)*8);
		stm.Seek(stm.GetReadPos()+3);
		return 1;
	}

	m_posLocal = m_pos + m_deltaPos*(m_timeSmooth*(1/0.3f));
	AllocateExtendedHistory();
	if (flags & ssf_compensate_time_diff)
		StepBackEx((m_pWorld->m_iTimePhysics-m_pWorld->m_iTimeSnapshot[0])*m_pWorld->m_vars.timeGranularity,false);
	else
		StepBackEx((m_pWorld->m_iTimePhysics-m_pWorld->m_iTimeSnapshot[1])*m_pWorld->m_vars.timeGranularity,true);

	vectorf pos0=m_pos,vel0=m_vel,velRequested0=m_velRequested;
	int bFlying0=m_bFlying;
	
	if (m_pWorld->m_vars.bMultiplayer) {
		ReadCompressedPos(stm,m_pos,bCompressed);
		// if we received compressed pos, and our pos0 compressed is equal to it, assume the server had pos equal to our uncompressed pos0
		if (bCompressed && (CompressPos(pos0)-m_pos).len2()<sqr(0.0001f))
			m_pos = pos0;
	} else
		stm.Read(m_pos);

	if (m_flags & lef_snap_velocities) {
		stm.Read(bnz); if (bnz) {
			Vec3_tpl<short> v; stm.Read(v.x); stm.Read(v.y); stm.Read(v.z);
			m_vel = DecodeVec6b(v);
		} else m_vel.zero();
		stm.Read(bnz); if (bnz) {
			vec4b vr; stm.Read(vr.yaw); stm.Read(vr.pitch);	stm.Read(vr.len);
			m_velRequested = DecodeVec4b(vr);
		} else m_velRequested.zero();
	} else {
		stm.Read(bnz); if (bnz)
			stm.Read(m_vel);
		else m_vel.zero();
		stm.Read(bnz); if (bnz)
			stm.Read(m_velRequested);
		else m_velRequested.zero();
	}
	stm.Read(bnz); if (bnz) {
		stm.Read(tmp); m_timeFlying = tmp*(10.0f/65536);
	} else m_timeFlying = 0;
	unsigned int imft; stm.ReadNumberInBits(imft,2);
	static float flytable[] = {0.0f, 0.2f, 0.5f, 1.0f};
	m_minFlyTime = flytable[imft];
	stm.Read(bnz);
	m_bFlying = bnz ? 1:0;
	ReleaseGroundCollider();
	m_bJumpRequested = 0;

	/*stm.Read(bnz);
	if (bnz) {
		SetGroundCollider((CPhysicalEntity*)m_pWorld->GetPhysicalEntityById(ReadPackedInt(stm)-1));
		m_iLastGroundColliderPart = ReadPackedInt(stm);
		stm.Read((Vec3&)m_posLastGroundColl);
	}*/

#ifdef _DEBUG
	float diff = (m_pos-pos0).len2();
	if (diff>sqr(0.0001f) && flags&ssf_compensate_time_diff && m_bActive)
		m_pWorld->m_pLog->Log("\002Local client desync: %.4f",sqrt_tpl(diff));
#endif
	if (m_pos.len2()>1E18) {
		m_pos = pos0;
		return 1;
	}

	float dt_state = (m_pWorld->m_iTimePhysics-m_pWorld->m_iTimeSnapshot[0])*m_pWorld->m_vars.timeGranularity;
	if (flags & ssf_compensate_time_diff && dt_state>0 && dt_state<3.0f && m_bActive) {
#ifdef _DEBUG
int iaction0=m_iAction,ihist0=m_iHist;
#endif
		int iaction,nActions,nSteps,bChange;
		float dt,dt_action; 
		for(dt_action=dt_state,nActions=0; dt_action>0.00001f && nActions<m_szActions; 
			dt_action-=m_actions[m_iAction].dt,--m_iAction&=m_szActions-1,nActions++);
		for(dt=dt_state,nSteps=0; dt>0.00001f && nSteps<m_szHistory && m_history[m_iHist].dt<1E9; 
				dt-=m_history[m_iHist].dt,--m_iHist&=m_szHistory-1,nSteps++);
		m_bStateReading = 1;

		do {
			bChange = 0;
			for(; dt_action<0.00001f && nActions>0; nActions--) {
				iaction = m_iAction+1&m_szActions-1;
				if (fabsf(dt_action)<0.00001f) dt_action = 0;
				dt_action += m_actions[iaction].dt; 
				m_actions[iaction].dt = 0;
				Action(m_actions+iaction); 
				bChange = 1;
			}
			for(; dt_action>=0.00001f && nSteps>0; nSteps--) {
				if (fabsf(dt)<0.00001f) dt = 0;
				dt += m_history[m_iHist+1&m_szHistory-1].dt;
				Step(dt); dt_action -= dt; dt = 0;
				bChange = 1;
			}
		} while (nActions+nSteps>0 && bChange);
		m_bStateReading = 0;
	}	

	return 1;
}


float CLivingEntity::ShootRayDown(CPhysicalEntity **pentlist,int nents, const vectorf &pos,vectorf &nslope, float time_interval,
																	bool bUseRotation,bool bUpdateGroundCollider,bool bIgnoreSmallObjects)
{
	int i,j,ibest=-1,jbest,ncont,idbest;
	//matrix3x3f R; m_qrot.getmatrix(R); //Q2M_IVO 
	matrix3x3f R;
	box bbox;
	if (bUseRotation)
		R = matrix3x3f(m_qrot);
	else
		R.SetIdentity();
	CRayGeom aray; aray.CreateRay(pos+R*vectorf(0,0,m_hCyl-m_hPivot),R*vectorf(0,0,-m_hCyl-m_size.x),&(R*vectorf(0,0,-1)));
	geom_world_data gwd;
	geom_contact *pcontacts;
	CPhysicalEntity *pPrevCollider=m_pLastGroundCollider;
	float h=-1E10,maxdim,maxarea;
	nslope = m_nslope;

	for(i=nents-1;i>=0;i--) if (pentlist[i]!=this) 
	for(j=0;j<pentlist[i]->m_nParts;j++) if (pentlist[i]->m_parts[j].flags & geom_colltype_player) {
		if (bIgnoreSmallObjects && (ibest>=0 || pentlist[i]->GetRigidBody(j)->v.len2()>sqr(m_maxVelGround*0.2f))) {
			pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom->GetBBox(&bbox);
			maxarea = max(max(bbox.size.x*bbox.size.y, bbox.size.x*bbox.size.z), bbox.size.y*bbox.size.z)*sqr(pentlist[i]->m_parts[j].scale)*4;
			maxdim = max(max(bbox.size.x,bbox.size.y),bbox.size.z)*pentlist[i]->m_parts[j].scale*2;
			if (maxarea<sqr(m_size.x)*pi*0.25f && maxdim<m_size.z*1.4f)
				continue;
		}
		//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(gwd.R);	//Q2M_IVO 
		gwd.R = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
		gwd.offset = pentlist[i]->m_pos + pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
		gwd.scale = pentlist[i]->m_parts[j].scale;
		if ((ncont=pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom->Intersect(&aray, &gwd,0, 0, pcontacts)) && pcontacts[ncont-1].pt.z>h) {
			h = pcontacts[ncont-1].pt.z; ibest=i; jbest=j; nslope = pcontacts[ncont-1].n;
			if (pentlist[i]->m_iSimClass==3) {
				(nslope = pcontacts[ncont-1].pt-pentlist[i]->m_pos).z = 0;
				if (nslope.len2()>1E-4f)
					nslope.normalize();
				else nslope.Set(1,0,0);
				nslope*=0.965925826f; nslope.z=0.2588190451f; // make it act like a 75 degrees slope
			}
			idbest = pcontacts[ncont-1].id[0]&~pcontacts[ncont-1].id[0]>>31 | pentlist[i]->m_parts[j].surface_idx&pcontacts[ncont-1].id[0]>>31;
		}
	}

	if (bUpdateGroundCollider) {
		ReleaseGroundCollider();
		m_lastGroundSurfaceIdx = -1;
		if (ibest>=0) {
			SetGroundCollider(pentlist[ibest]);
			m_iLastGroundColliderPart = jbest;
			vectorf coll_origin = pentlist[ibest]->m_pos+pentlist[ibest]->m_qrot*pentlist[ibest]->m_parts[jbest].pos;
			quaternionf coll_q = pentlist[ibest]->m_qrot*pentlist[ibest]->m_parts[jbest].q;
			float coll_scale = pentlist[ibest]->m_parts[jbest].scale;
			m_posLastGroundColl = ((m_pos-coll_origin)*coll_q)/coll_scale;
			m_lastGroundSurfaceIdx = idbest;

			if (pPrevCollider!=pentlist[ibest])
				AddLegsImpulse(m_gravity*time_interval+m_vel,nslope,true);
			else
				AddLegsImpulse(m_gravity*time_interval,nslope,false);
		}
	}
	return h;
}


void CLivingEntity::AddLegsImpulse(const vectorf &vel, const vectorf &nslope, bool bInstantChange)
{
	int ncoll;
	CPhysicalEntity **pColliders,*pPrevCollider=m_pLastGroundCollider;
	pe_status_sample_contact_area ssca;
	RigidBody *pbody;
	pe_action_impulse ai;
	(ssca.ptTest = m_pos).z -= m_hPivot; ssca.dirTest = m_gravity;

	if (m_pLastGroundCollider && m_flags&lef_push_objects &&
			(unsigned int)m_pLastGroundCollider->m_iSimClass-1u<2u && m_pLastGroundCollider->m_flags & pef_pushable_by_players && 
			((pbody=m_pLastGroundCollider->GetRigidBody(m_iLastGroundColliderPart))->Minv<m_massinv*2 ||
			 (ncoll=m_pLastGroundCollider->GetColliders(pColliders))==0 || (ncoll==1 && pColliders[0]==m_pLastGroundCollider)) &&
			!m_pLastGroundCollider->GetStatus(&ssca))
	{
		vectorf vrel = vel;
		ai.point = ssca.ptTest;
		if (bInstantChange)
			vrel -= pbody->v+(pbody->w^ai.point-pbody->pos);
		matrix3x3f K; K.SetZero();
		m_pLastGroundCollider->GetContactMatrix(ai.point,m_iLastGroundColliderPart,K);
		K(0,0)+=m_massinv*0.5f; K(1,1)+=m_massinv*0.5f; K(2,2)+=m_massinv*0.5f;
		ai.impulse = nslope*(min(0.0f,nslope*vrel)/(nslope*K*nslope));
		m_pLastGroundCollider->Action(&ai);
	}
}


void CLivingEntity::RegisterContact(const vectorf& pt,const vectorf& n, CPhysicalEntity *pCollider, int ipart,int idmat)
{
	RigidBody *pbody = pCollider->GetRigidBody(ipart);
	vectorf v = pbody->v+(pbody->w^pt-pbody->pos);
	if (m_collision.age<0.5f && v.len2()<pbody->Minv*m_collision.mass[1]*m_collision.v[1].len2())
		return;

	m_collision.pt = pt;
	m_collision.n = n;
	m_collision.v[0] = m_vel;
	m_collision.v[1] = v;
	m_collision.mass[0] = m_mass;
	m_collision.mass[1] = pbody->M;
	m_collision.age = 0;
	m_collision.idCollider = m_pWorld->GetPhysicalEntityId(pCollider);
	m_collision.partid[0] = 100;
	m_collision.partid[1] = pCollider->m_parts[ipart].id;
	m_collision.idmat[0] = m_surface_idx;
	m_collision.idmat[1] = idmat>>31 & pCollider->m_parts[ipart].surface_idx | idmat & ~(idmat>>31);
}

void CLivingEntity::RegisterUnprojContact(const le_contact &unproj)
{
	if (m_nContacts==m_nContactsAlloc) {
		le_contact *pContacts = m_pContacts;
		m_pContacts = new le_contact[m_nContactsAlloc+=4];
		memcpy(m_pContacts, pContacts, m_nContacts*sizeof(le_contact));
		if (pContacts) delete[] pContacts;
	}
	m_pContacts[m_nContacts++] = unproj;
	AddCollider(unproj.pent);
	unproj.pent->AddCollider(this);
}


float CLivingEntity::GetMaxTimeStep(float time_interval)
{
	if (m_timeStepPerformed > m_timeStepFull-0.001f)
		return time_interval;
	return min_safe(m_timeStepFull-m_timeStepPerformed,time_interval);
}

void CLivingEntity::SyncWithGroundCollider(float time_interval)
{
	int i; vectorf newpos;
	if (m_pLastGroundCollider && m_pLastGroundCollider->m_iSimClass==7)
		ReleaseGroundCollider();
	m_velGround.zero();

	if (m_pLastGroundCollider) {
		i = m_iLastGroundColliderPart;
		vectorf coll_origin = m_pLastGroundCollider->m_pos+m_pLastGroundCollider->m_qrot*m_pLastGroundCollider->m_parts[i].pos;
		quaternionf coll_q = m_pLastGroundCollider->m_qrot*m_pLastGroundCollider->m_parts[i].q;
		float coll_scale = m_pLastGroundCollider->m_parts[i].scale;
		newpos = coll_q*m_posLastGroundColl*coll_scale+coll_origin;
		if ((newpos-m_pos).len2() > sqr(time_interval*0.01f))	{
			if ((newpos-m_pos).len2() > sqr(time_interval*m_maxVelGround))
				newpos = m_pos+(newpos-m_pos).normalized()*(m_maxVelGround*time_interval);
			m_BBox[0]+=(newpos-m_pos); m_BBox[1]+=(newpos-m_pos);
			m_pos = newpos; 
		}
		pe_status_dynamics sd;
		m_pLastGroundCollider->GetStatus(&sd);
		m_velGround = sd.v;
		if (m_velGround.len2()>sqr(m_maxVelGround))
			m_velGround.normalize() *= m_maxVelGround;
	}
}

void CLivingEntity::StartStep(float time_interval)
{
	m_timeStepPerformed = 0;
	m_timeStepFull = time_interval;
	m_nContacts = 0;
	//SyncWithGroundCollider(time_interval);
}


int CLivingEntity::Step(float time_interval)
{
	if (time_interval<=0)
		return 1;
	if (m_timeStepPerformed>m_timeStepFull-0.001f && m_pWorld->m_vars.bMultiplayer+m_bStateReading==0)
		time_interval = 0.001f;
	int i,j,imin,jmin,iter,nents,ncont,bWasFlying,bPushOther,bUnprojected,idmat,bFastPhys,bHasFastPhys,iCyl,icnt,nUnproj,bStaticUnproj,bDynUnproj;
	vectorf pos0,newpos,move(zero),nslope,ncontact,ptcontact,ncontactHist[4],ncontactSum,BBoxInner[2],BBoxOuter[2],velGround;
	float movelen,tmin,h,hcur,dh,tfirst,vrel,move0,movesum,kInertia=m_timeForceInertia>0 ? 6.0f:m_kInertia;
	le_contact unproj[8];
	CCylinderGeom CylinderGeomOuter,*pCyl[2];
	geom_world_data gwd[2];
	intersection_params ip;
	CPhysicalEntity **pentlist;
	geom_contact *pcontacts;
	pe_player_dimensions pd;
	pe_action_impulse ai;
	pe_status_dynamics sd;
	matrix3x3f K;
	ip.bNoAreaContacts = true;

	SyncWithGroundCollider(time_interval);
	m_actions[m_iAction].dt += time_interval;
	bWasFlying = m_bFlying;
	if (m_vel.z<-m_size.z*200) {
		m_vel.z = 0; m_pos.z = 100;
	}
	m_timeUseLowCap -= time_interval;
	m_timeSinceStanceChange += time_interval;
	m_timeSinceImpulseContact += time_interval;
	m_timeForceInertia = max(0.0f,m_timeForceInertia-time_interval);
	m_timeStepPerformed += time_interval;
	m_bUseSphere = 0;

	if (m_bActive && 
			(!(m_vel.len2()==0 && m_velRequested.len2()==0 && (!m_bFlying || m_gravity.len2()==0) && m_dhSpeed==0 && m_dhAcc==0 && 
			m_qrot.w==m_history[m_iHist-1&m_szHistory-1].q.w && (m_qrot.v-m_history[m_iHist-1&m_szHistory-1].q.v).len2()==0) || 
			m_nslope.z<m_slopeSlide || m_velGround.len2()>0 || m_bActiveEnvironment))	
	{
		FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );
		PHYS_ENTITY_PROFILER

		m_bActiveEnvironment = 0;
		m_nslope.Set(0,0,1);
		if (kInertia==0 && !m_bFlying && !m_bJumpRequested || m_pWorld->m_vars.bFlyMode) {
			m_vel = m_velRequested;
			m_vel -= m_nslope*(m_nslope*m_vel);
		}
		if (!m_pWorld->m_vars.bFlyMode && m_bFlying)
			//move += m_velRequested*(m_kAirControl*time_interval);
			if (m_vel*m_velRequested < m_velRequested.len2()*m_kAirControl)
				m_vel += m_velRequested*(m_kAirControl*time_interval*2);
		if (m_vel.len2() > sqr(m_pWorld->m_vars.maxVelPlayers))
			m_vel.normalize() *= m_pWorld->m_vars.maxVelPlayers;
		move += m_vel*time_interval;
		if (m_bFlying && !m_bSwimming && !m_pWorld->m_vars.bFlyMode) 
			move += m_gravity*sqr(time_interval)*0.5f;
		bUnprojected = 0;

		if (m_pWorld->m_vars.iCollisionMode!=0 && m_pWorld->m_vars.bFlyMode) {
			m_pos+=move; m_bFlying=1; m_hLatest=0; ReleaseGroundCollider();
		} else {
			movelen = move.len(); h = (movelen+m_pWorld->m_vars.maxContactGapPlayer)*1.5f;
			BBoxInner[0] = m_BBox[0]-vectorf(h,h,h+m_size.z*0.1f);
			BBoxInner[1] = m_BBox[1]+vectorf(h,h,h);
			h = max(10.0f*time_interval,h); // adds a safety margin of m_size.x width
			BBoxOuter[0] = m_BBox[0]-vectorf(h,h,h);
			BBoxOuter[1] = m_BBox[1]+vectorf(h,h,h);
			nents = m_pWorld->GetEntitiesAround(BBoxOuter[0],BBoxOuter[1], 
				pentlist, ent_terrain|ent_static|ent_sleeping_rigid|ent_rigid|ent_living|ent_independent|ent_triggers|ent_sort_by_mass, this);

			for(i=j=bHasFastPhys=0,vrel=0; i<nents; i++) {
				vectorf sz = pentlist[i]->m_BBox[1]-pentlist[i]->m_BBox[0];
				if (pentlist[i]->m_iSimClass==2 && pentlist[i]->GetMassInv()*0.4f<m_massinv) {
					pentlist[i]->GetStatus(&sd);
					vrel = max(vrel,sd.v.len()+sd.w.len()*max(max(sz.x,sz.y),sz.z));;
					bHasFastPhys |= (bFastPhys = isneg(m_size.x*0.2f-vrel*time_interval));
				}	else
					bFastPhys = 0;
				if (!bFastPhys && !AABB_overlap(pentlist[i]->m_BBox,BBoxInner) && sz.len2()>0)
					continue;
				idmat = pentlist[i]->GetType();
				if (idmat==PE_SOFT || idmat==PE_ROPE)
					pentlist[i]->Awake();
				else if (pentlist[i]->m_iSimClass<4 && 
					(idmat!=PE_LIVING || /*m_parts[0].flags&*/pentlist[i]->m_parts[0].flags&geom_colltype_player && 
					 pentlist[i]!=this))// && pentlist[i]->GetParams(&pdyn) && pdyn.bActive))
				{
					if (pentlist[i]->m_iSimClass==1 && m_timeSinceImpulseContact<0.2f && pentlist[i]->GetMassInv()>0)	{
						int ipart; unsigned int flags; 
						for(ipart=0,flags=0; ipart<pentlist[i]->m_nParts; ipart++) 
							flags |= pentlist[i]->m_parts[ipart].flags;
						if (flags & geom_colltype_player)
							pentlist[i]->Awake();
					}
					pentlist[j++] = pentlist[i];
					if (idmat==PE_LIVING)
						((CLivingEntity*)pentlist[i])->SyncWithGroundCollider(time_interval);
				}
			}
			nents = j;
			pos0 = m_pos;
			bStaticUnproj = bDynUnproj = 0;
			newpos = m_pos+move;
			h = ShootRayDown(pentlist,nents, newpos,nslope);

			// first, check if we need unprojection in the initial position
			//m_qrot.getmatrix(gwd[0].R); //gwd[0].R.identity(); //Q2M_IVO 
			if (sqr(m_qrot.v.x)+sqr(m_qrot.v.y)<sqr(0.05f))
				gwd[0].R.SetIdentity(); // = matrix3x3f(m_qrot);
			else
				gwd[0].R = matrix3x3f(m_qrot);
			gwd[0].centerOfMass = gwd[0].offset = m_pos + gwd[0].R*m_parts[0].pos; //(gwd[0].offset = m_pos).z += m_parts[0].pos.z;
			gwd[0].v.zero(); gwd[0].w.zero(); // since we check a static character against potentially moving environment here
			ip.vrel_min = m_size.x;
			ip.time_interval = time_interval*2;
			ip.maxUnproj = m_size.x*1.5f;
			ip.ptOutsidePivot[0] = gwd[0].offset;
			bFastPhys = 0;
			pCyl[0] = &m_CylinderGeom;
			if (bHasFastPhys) {
				cylinder cylOuter;
				cylOuter.r = m_size.x+min(m_size.x*1.5f,vrel*time_interval);
				cylOuter.hh = m_size.z+min(m_size.z,vrel*time_interval);
				cylOuter.center.zero();
				cylOuter.axis.Set(0,0,1);
				CylinderGeomOuter.CreateCylinder(&cylOuter);
				pCyl[1] = &CylinderGeomOuter;
			}

			for(i=nUnproj=0;i<nents;i++) if (pentlist[i]->GetType()!=PE_LIVING || pentlist[i]->m_nParts>1) {
				int bHeavy = isneg(pentlist[i]->GetMassInv()*0.4f-m_massinv);
				if (bHasFastPhys && pentlist[i]->m_iSimClass==2 && bHeavy) {
					pentlist[i]->GetStatus(&sd);
					vectorf sz = pentlist[i]->m_BBox[1]-pentlist[i]->m_BBox[0];
					vrel = max(vrel,sd.v.len()+sd.w.len()*max(max(sz.x,sz.y),sz.z));
					bFastPhys = isneg(m_size.x*0.2f-vrel*time_interval);
					gwd[1].v = sd.v; gwd[1].w = sd.w;
					gwd[1].centerOfMass = sd.centerOfMass;
				}	else {
					gwd[1].v.zero(); gwd[1].w.zero();
					bFastPhys = 0;
				}

				for(iCyl=0; iCyl<=bFastPhys; iCyl++)
				for(j=0;j<pentlist[i]->m_nParts;j++) if (pentlist[i]->m_parts[j].flags & geom_colltype_player) {
					//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(gwd[1].R); //Q2M_IVO 
					gwd[1].R = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
					gwd[1].offset = pentlist[i]->m_pos + pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
					gwd[1].scale = pentlist[i]->m_parts[j].scale;
					
					if (pCyl[iCyl]->Intersect(pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom, gwd,gwd+1, &ip, pcontacts)) {
						if (iCyl==0 && bHeavy) {
							if (m_pWorld->m_bWorldStep==2) { // this means step induced by rigid bodies moving around
								// if the entity is rigid, store the contact
								if (bPushOther = pentlist[i]->m_iSimClass>0 && pentlist[i]->GetMassInv()>0 && pentlist[i]->m_iGroup==m_pWorld->m_iCurGroup) {
									nUnproj = min(nUnproj+1,sizeof(unproj)/sizeof(unproj[0]));
									unproj[nUnproj-1].pent = pentlist[i];
									unproj[nUnproj-1].ipart = j;
									unproj[nUnproj-1].pt = pcontacts[0].pt;
									unproj[nUnproj-1].n = pcontacts[0].n;
									unproj[nUnproj-1].center = gwd[0].offset;
									unproj[nUnproj-1].penetration = pcontacts[0].t;
								}
								// check previous contacts from this frame, register in entity if there are conflicts
								for(icnt=0; icnt<nUnproj-bPushOther; icnt++) if (unproj[icnt].n*pcontacts[0].n<0) {
									RegisterUnprojContact(unproj[icnt]);
									if (bPushOther)
										RegisterUnprojContact(unproj[nUnproj-1]);
								}
							}
							(pentlist[i]->GetMassInv()==0 ? bStaticUnproj:bDynUnproj)++;

							vectorf offs = pcontacts[0].dir*(pcontacts[0].t+m_pWorld->m_vars.maxContactGapPlayer);
							offs.z = max(offs.z, h-(m_pos.z-m_hPivot+m_hCyl-m_size.z)); // don't allow unprojection to push cylinder into the ground
							m_pos += offs; gwd[0].offset += offs;
							bUnprojected = 1;
						}

						if (pentlist[i]->m_iSimClass==2) {
							K.SetZero(); GetContactMatrix(pcontacts[0].center, 0, K);
							bPushOther = (m_flags & lef_push_objects) && (pentlist[i]->m_flags & pef_pushable_by_players);
							if (bPushOther)
								pentlist[i]->GetContactMatrix(pcontacts[0].center, j, K);
							ncontact = (gwd[0].offset-pcontacts[0].center).normalized();
							if (fabs_tpl(ncontact.z)<0.5f) {
								ncontact.z=0; ncontact.normalize();
							}
							RigidBody *pbody = pentlist[i]->GetRigidBody(j);
							vrel = ncontact*(pbody->v+(pbody->w^pcontacts[0].center-pbody->pos));
							if (iCyl==0 || vrel*time_interval>m_size.x*0.2f) {
								vrel -= ncontact*m_vel;
								ai.impulse = ncontact*(vrel/(ncontact*K*ncontact));
								ai.point = pcontacts[0].center;	ai.ipart = 0;
								if (ai.impulse.len2()*sqr(m_massinv) > max(1.0f,sqr(vrel)))
									ai.impulse.normalize() *= fabs_tpl(vrel)*m_mass*0.5f;
								m_vel += ai.impulse*m_massinv; 
								/*if (m_vel.z>-5) {
									m_vel.z = max(0.0f, m_vel.z); m_vel.z += ai.impulse.len()*m_massinv*0.1f; 
								}*/
								m_bFlying = 1; m_minFlyTime = 1.0f; m_timeFlying = 0;
								if (m_kInertia==0)
									m_timeForceInertia = 5.0f;
								if (bPushOther) {
									if (fabs_tpl(pcontacts[0].dir.z)<0.7f)
										ai.impulse.z = 0; // ignore vertical component - might be semi-random depending on cylinder intersection
									ai.impulse.Flip(); ai.ipart = j;
									pentlist[i]->Action(&ai);
									m_timeSinceImpulseContact = 0;
								}
								idmat = pentlist[i]->m_parts[j].surface_idx&pcontacts[0].id[1]>>31 | max(pcontacts[0].id[1],0);
								RegisterContact(pcontacts[0].pt,ncontact,pentlist[i],j,idmat);
							}
						}
						break;
					}
				}
			}
			if (bStaticUnproj && bDynUnproj) {
				m_pos = pos0; // disable unprojection if we are being squashed
				bStaticUnproj = bDynUnproj = 0;
			}

			if (bStaticUnproj+bDynUnproj>0) {	// need to recalculate h in case of unprojection
				newpos = m_pos+move;
				h = ShootRayDown(pentlist,nents, newpos,nslope);
			}

			hcur = newpos.z-m_hPivot;
			if (h>hcur && h<hcur+(m_hCyl-m_size.z)*1.01f && nslope.z>m_slopeClimb || 
					h<hcur && h>hcur-m_size.z*0.06f && !m_bFlying && !m_bJumpRequested && !m_bSwimming) 
			{
				newpos.z = h+m_hPivot; move = newpos-m_pos; movelen = move.len();
			}
			pos0 = m_pos;
			if (m_bJumpRequested)
				AddLegsImpulse(-m_vel,m_nslope,true);
			m_bJumpRequested = 0;
			m_bStuck = 0; ncontactSum.zero();

			if (movelen>m_size.x*1E-4f) {
				ip.bSweepTest = true;
				gwd[0].v = move/movelen;
				//m_pos -= gwd[0].v*m_pWorld->m_vars.maxContactGapPlayer; movelen += m_pWorld->m_vars.maxContactGapPlayer;
				iter = 0;	move0 = movelen; movesum = 0;
				m_bUseSphere = m_bFlying && !m_pWorld->m_vars.bFlyMode && m_hCyl-m_hPivot>m_size.x*1.05f && h<hcur-m_size.x && m_gravity.len2()>0;

				do {
					gwd[0].offset = m_pos + gwd[0].R*m_parts[0].pos; //(gwd[0].offset = m_pos).z += m_parts[0].pos.z;
					ip.time_interval = movelen+m_pWorld->m_vars.maxContactGapPlayer; tmin = ip.time_interval*2; imin = -1;
					for(i=0;i<nents;i++) if (pentlist[i]->GetType()!=PE_LIVING || pentlist[i]->m_nParts>1) {
						for(j=0;j<pentlist[i]->m_nParts;j++) if (pentlist[i]->m_parts[j].flags & geom_colltype_player) {
							//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(gwd[1].R); //Q2M_IVO 
							gwd[1].R = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
							gwd[1].offset = pentlist[i]->m_pos + pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
							gwd[1].scale = pentlist[i]->m_parts[j].scale; 
							if (m_bUseSphere && (ncont = m_SphereGeom.Intersect(pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom, gwd,gwd+1, &ip, pcontacts)) &&
									pcontacts[ncont-1].t<tmin && pcontacts[ncont-1].n*gwd[0].v>0)
							{ tmin = pcontacts[ncont-1].t; ncontact = -pcontacts[ncont-1].n; ptcontact = pcontacts[ncont-1].pt; 
								imin=i; jmin=j; idmat=pentlist[i]->m_parts[j].surface_idx&pcontacts[ncont-1].id[1]>>31 | max(pcontacts[ncont-1].id[1],0);
							}
							if((ncont = m_CylinderGeom.Intersect(pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom, gwd,gwd+1, &ip, pcontacts)) &&
									pcontacts[ncont-1].t<tmin && pcontacts[ncont-1].n*gwd[0].v>0 && 
									(!m_bUseSphere || m_timeUseLowCap>0 || pcontacts[ncont-1].n.z>-0.5f || 
									 pcontacts[ncont-1].iFeature[0]!=0x20 && pcontacts[ncont-1].iFeature[0]!=0x41))
							{ tmin = pcontacts[ncont-1].t; ncontact = -pcontacts[ncont-1].n; ptcontact = pcontacts[ncont-1].pt; 
								imin=i; jmin=j; idmat=pentlist[i]->m_parts[j].surface_idx&pcontacts[ncont-1].id[1]>>31 | max(pcontacts[ncont-1].id[1],0);
							}
						}
					}	else {
						pentlist[i]->GetParams(&pd);
						vector2df dorigin,ncoll,move2d=(vector2df)gwd[0].v;
						dorigin = newpos-pentlist[i]->m_pos;
						float kb=dorigin*move2d, kc=dorigin.len2()-sqr(m_size.x+pd.sizeCollider.x), kd=kb*kb-kc, zup0,zdown0,zup1,zdown1;
						if (kd>=0) {
							zup0 = (zdown0 = m_pos.z-m_hPivot)+m_hCyl+m_size.z; 
							zup1 = (zdown1 = pentlist[i]->m_pos.z-pd.heightPivot)+pd.heightCollider+pd.sizeCollider.z;
							kd=sqrt_tpl(kd); tfirst=-kb+kd; ncoll = m_pos+gwd[0].v*tfirst-pentlist[i]->m_pos;
							if (tfirst>-m_size.x && tfirst<tmin && ncoll*move2d>=0 && min(zup0+gwd[0].v.z*tfirst,zup1)<max(zdown0+gwd[0].v.z*tfirst,zdown1))
								continue;	// if entities separate during this timestep, ignore other collisions
							tfirst=-kb-kd; ncoll = m_pos+gwd[0].v*tfirst-pentlist[i]->m_pos;
							if (tfirst<-m_size.x || min(zup0+gwd[0].v.z*tfirst,zup1)<max(zdown0+gwd[0].v.z*tfirst,zdown1) || ncoll*move2d>=0) { 
								tfirst=-kb+kd; ncoll = m_pos+gwd[0].v*tfirst-pentlist[i]->m_pos; 
							}
							if (tfirst>-m_size.x && tfirst<tmin && ncoll*move2d<0 && min(zup0+gwd[0].v.z*tfirst,zup1)>max(zdown0+gwd[0].v.z*tfirst,zdown1)) { 
								tmin = tfirst; ncontact.Set(ncoll.x,ncoll.y,0).normalize(); ptcontact = m_pos+gwd[0].v*tfirst-ncontact*m_size.x; 
								imin=i; jmin=0; idmat=m_surface_idx; 
							}
							// also check for cap-cap contact
							if (fabs_tpl(gwd[0].v.z)>m_size.z*1E-5f) {
								j = sgnnz(gwd[0].v.z);
								zup0 = m_pos.z-m_hPivot+m_hCyl+m_size.z*j;
								zdown1 = pentlist[i]->m_pos.z-pd.heightPivot+pd.heightCollider-pd.sizeCollider.z*j;
								tfirst = zdown1-zup0;
								if (inrange(tfirst,-m_size.x*gwd[0].v.z,tmin*gwd[0].v.z) && 
										(dorigin*gwd[0].v.z+move2d*tfirst).len2()<sqr((m_size.x+pd.sizeCollider.x)*gwd[0].v.z)) 
								{
									tmin = tfirst/gwd[0].v.z; ncontact.Set(0,0,-j); (ptcontact = m_pos+gwd[0].v*tfirst).z += m_size.z*j; 
									imin=i; jmin=0; idmat=m_surface_idx; 
								}
							}
						}
					}

					if (tmin<=ip.time_interval) {
						tmin = max(0.0f,tmin-m_pWorld->m_vars.maxContactGapPlayer);
						m_pos += gwd[0].v*tmin; 
						if (pentlist[imin]->m_iSimClass>0 || ncontact.z>m_slopeClimb || ncontact.z<-m_slopeFall || m_vel.z<-1.0f) {
							K.SetZero(); GetContactMatrix(ptcontact, 0, K);
							bPushOther = (m_flags & (pentlist[imin]->GetType()==PE_LIVING ? lef_push_players : lef_push_objects)) && 
								(pentlist[imin]->m_flags & pef_pushable_by_players);
							if (bPushOther)
								pentlist[imin]->GetContactMatrix(ptcontact, jmin, K);
							vrel = ncontact*m_vel; //(ncontact*gwd[0].v)*m_vel.len();
							ai.impulse = ncontact;
							if (pentlist[imin]->GetType()==PE_LIVING) {
								// make the player slide off when standing on other players
								vrel -= ncontact*((CLivingEntity*)pentlist[imin])->m_vel;
								if (ncontact.z>0.95f) {
									*(vector2df*)&ai.impulse += vector2df(m_pos-pentlist[imin]->m_pos).normalized();
									if (inrange(vrel,-1.0f,1.0f))
										vrel = -1.0f;
								}
							} else {
								RigidBody *pbody = pentlist[imin]->GetRigidBody(jmin);
								vrel -= ncontact*(pbody->v+(pbody->w^ptcontact-pbody->pos));
							}
							RegisterContact(ptcontact,ncontact,pentlist[imin],jmin,idmat);
							ai.impulse *= -vrel*1.01f/(ncontact*K*ncontact);
							ai.point = ptcontact;	ai.ipart = 0;
							if (ai.impulse.len2()*sqr(m_massinv) > max(1.0f,sqr(vrel)))
								ai.impulse.normalize() *= fabs_tpl(vrel)*m_mass*0.5f;
							m_vel += ai.impulse*m_massinv;
							if (m_kInertia==0)
								m_timeForceInertia = 5.0f;
							if (bPushOther) {
								ai.impulse.Flip(); ai.ipart = jmin;
								if (fabs_tpl(ncontact.z)<0.7f)
									ai.impulse.z = 0; // ignore vertical component - might be semi-random depending on cylinder intersection
								ai.iApplyTime = isneg(pentlist[imin]->m_iSimClass-3)<<1;
								pentlist[imin]->Action(&ai);
								if (pentlist[imin]->m_iSimClass<3)
									m_timeSinceImpulseContact = 0;
							}
						}
						movelen -= tmin; movesum += tmin;
						gwd[0].v -= ncontact*((gwd[0].v*ncontact)-0.1f);
						tmin = gwd[0].v.len(); movelen*=tmin; gwd[0].v/=tmin;
						if (gwd[0].v*move<0)
							movelen = 0;
						for(i=0;i<iter && ncontactHist[i]*ncontact<0.95f;i++);
						if (i==iter)
							ncontactSum += ncontact;
						ncontactHist[iter] = ncontact;
					} else {
						m_pos += gwd[0].v*movelen; movesum += movelen; 
						break;
					}
				} while(movelen>m_pWorld->m_vars.maxContactGapPlayer*0.1f && ++iter<3);
				if (movesum<move0*0.001f && (sqr_signed(ncontactSum.z)>sqr(0.4f)*ncontactSum.len2() || ncontactSum.len2()<0.6f))
					m_bStuck = 1;

				if (bUnprojected || !(m_flags & lef_loosen_stuck_checks)) {
					ip.bSweepTest = false;
					gwd[0].offset = m_pos + gwd[0].R*m_parts[0].pos; //(gwd[0].offset = m_pos).z += m_parts[0].pos.z;
					gwd[0].v.Set(0,0,-1); 
					ip.bStopAtFirstTri = true; ip.bNoBorder = true; ip.time_interval = m_size.z*10;
					for(i=0;i<nents;i++) if (pentlist[i]->GetType()!=PE_LIVING && pentlist[i]->GetMassInv()*0.4f<m_massinv) {
						for(j=0;j<pentlist[i]->m_nParts;j++) if (pentlist[i]->m_parts[j].flags & geom_colltype_player) {
							//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(gwd[1].R); //Q2M_IVO 
							gwd[1].R = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
							gwd[1].offset = pentlist[i]->m_pos + pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
							gwd[1].scale = pentlist[i]->m_parts[j].scale;
							if(m_CylinderGeom.Intersect(pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom, gwd,gwd+1, &ip, pcontacts)) { 
								m_pos = pos0; 
								m_vel.zero(); m_timeUseLowCap=1.0f; m_bStuck=1; goto nomove; 
							}
						}
					} nomove:;
				}
			}

			if (!m_pLastGroundCollider || m_pLastGroundCollider->GetMassInv()>m_massinv)
				velGround.zero();
			else
				velGround = m_velGround;
			m_hLatest = h = ShootRayDown(pentlist,nents, m_pos,nslope, time_interval,false,true);
			if (nslope.z<0.087f)
				nslope = m_nslope;
			else
				m_nslope = nslope;

			if (m_bFlying)
				m_timeFlying += time_interval;
			int bGroundContact = isneg(m_pos.z-m_hPivot-(h+m_size.z*0.01));
			m_bFlying = m_pWorld->m_vars.bFlyMode || m_gravity.z>0 ||	m_bSwimming || ((bGroundContact^1) &&
									!(m_bStuck || ncontactSum.len2()>0.01f && sqr_signed(ncontactSum.z)>sqr(0.7f)*ncontactSum.len2()));
			m_bActiveEnvironment = m_bStuck;

			vectorf last_force(zero);
			if (m_bFlying) {
				if (!bWasFlying)
					m_vel += velGround;
				if (!m_pWorld->m_vars.bFlyMode) {
					last_force = m_gravity;
					if (m_bSwimming) 
						last_force += (m_velRequested-m_vel)*kInertia;
					if ((m_vel+last_force*time_interval)*m_vel<0)
						m_vel.zero();
					else
						m_vel += last_force*time_interval;
					last_force = -m_vel*m_kAirResistance*time_interval;
					if (last_force.len2()<m_vel.len2()*4.0f)
						m_vel += last_force;
				}
				if (m_vel.len2()<1E-10f) m_vel.zero();
			} else {
				if (bWasFlying && !m_bSwimming) {
					if (m_vel.z<-4.0f && !m_bStateReading) {
						m_dhSpeed = m_vel.z;
						m_dhAcc = max(sqr(m_dhSpeed)/(m_size.z*2), m_nodSpeed);
					}
					if (m_nslope.z<m_slopeFall && (m_vel.z<-4) && !m_bStuck){// || m_timeFlying<m_minFlyTime)) {
						m_vel -= m_nslope*(m_vel*m_nslope)*1.4f;
						if (m_vel.z>0) m_vel.z=0;
						m_bFlying = 1;
					} else if ((!bGroundContact || m_velRequested.len2()>0) && (m_nslope.z>m_slopeClimb || m_vel.z<0))
						m_vel -= m_nslope*(m_vel*m_nslope);
					else
						m_vel.zero();
				}
				vectorf vel = m_velRequested,g;
				if (!m_bSwimming) vel -= m_nslope*(vel*m_nslope);
				last_force = (vel-m_vel)*kInertia;
				if (m_nslope.z<m_slopeSlide && !m_bSwimming)	{
					if (m_velRequested.len2()>0)
						g.Set(0,0,-vel.len()*kInertia/sqrt_tpl(1-sqr(m_slopeClimb)));
					else g = m_gravity;
					last_force += g-m_nslope*(g*m_nslope);
				}
				if ((m_vel+last_force*time_interval)*m_vel<0 && (m_vel+last_force*time_interval)*m_velRequested<=0)
					m_vel.zero();
				else
					m_vel += last_force*time_interval;
				if (m_nslope.z<m_slopeClimb) {
					if (m_vel.z>0 && last_force.z>0)
						m_vel.z = 0;
					if (m_pos.z>pos0.z+m_size.z*0.001f) {
						/*m_pos=pos0;*/ m_vel.z=0;
					}
				}
				if (m_nslope.z<m_slopeFall && !m_bStuck) {
					m_bFlying=1; m_minFlyTime=0.5f; m_vel += vectorf(m_nslope.x,m_nslope.y,0);
				}
				if (m_velRequested.len2()==0 && m_vel.len2()<0.001f || m_vel.len2()<0.0001f) 
					m_vel.zero();
			}

			if (!m_bFlying)
				m_timeFlying = m_minFlyTime = 0;
			else
				m_timeFlying = float2int(min(m_timeFlying,9.9f)*6553.6f)*(10.0f/65536); // quantize time flying
			if (m_flags & lef_snap_velocities)
				m_vel = DecodeVec6b(EncodeVec6b(m_vel));

			if (!m_bStateReading) {
				if (!m_bFlying && (dh=m_pos.z-pos0.z)>m_size.z*0.01f) {
					m_dhSpeed = max(m_dhSpeed, dh/m_stablehTime);
					m_dh += dh; if (m_dh>m_size.z) m_dh=m_size.z;
					m_stablehTime = 0;
					if (dh>(m_dhHist[0]+m_dhHist[1])*3)
						m_timeOnStairs = 0.5f;
				}	else if (m_bFlying)
					dh = m_dh = m_dhSpeed = 0;
				m_dhHist[0]=m_dhHist[1]; m_dhHist[1]=dh;
				m_stablehTime = min(m_stablehTime+time_interval,1.0f);
				m_timeOnStairs = max(m_timeOnStairs-time_interval,0.0f);

				m_dhSpeed += m_dhAcc*time_interval;
				if (m_dhAcc==0 && m_dh*m_dhSpeed<0 || m_dh*m_dhAcc<0 || m_dh*(m_dh-m_dhSpeed*time_interval)<0)
					m_dh = m_dhSpeed = m_dhAcc = 0;
				else
					m_dh -= m_dhSpeed*time_interval;
			}

			if (m_HeadGeom.m_sphere.r>0) {
				ip.bSweepTest = true;
				gwd[0].offset = m_pos + gwd[0].R*m_parts[0].pos; //(gwd[0].offset = m_pos).z += m_parts[0].pos.z;
				gwd[0].v = gwd[0].R*vectorf(0,0,1); 
				tmin = ip.time_interval = m_hHead-m_hCyl-min(m_dh,0.0f);
				for(i=0;i<nents;i++) if (pentlist[i]->m_iSimClass==0) {//pentlist[i]->GetType()!=PE_LIVING && pentlist[i]->GetMassInv()*0.4f<m_massinv) {
					for(j=0;j<pentlist[i]->m_nParts;j++) if (pentlist[i]->m_parts[j].flags & geom_colltype_player) {
						//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(gwd[1].R); //Q2M_IVO 
						gwd[1].R = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
						gwd[1].offset = pentlist[i]->m_pos + pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
						gwd[1].scale = pentlist[i]->m_parts[j].scale;
						if(m_HeadGeom.Intersect(pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom, gwd,gwd+1, &ip, pcontacts))
							tmin = min(tmin,(float)pcontacts[0].t);
					}
				}
				if (m_dh<ip.time_interval+min(m_dh,0.0f)-tmin || fabs_tpl(m_dhSpeed)+fabs_tpl(m_dhAcc)==0)
					m_dh = ip.time_interval+min(m_dh,0.0f)-tmin;
			}

			m_iSensorsActive = 0;
			for(i=0;i<m_nSensors;i++) {
				vectorf pt = m_pos+m_qrot*m_pSensors[i];
				m_pSensorsPoints[i] = pt;	m_pSensorsPoints[i].z = m_pos.z;
				m_pSensorsPoints[i].z = ShootRayDown(pentlist,nents, pt,m_pSensorsSlopes[i],0,true,false,false);
				int bActive = isneg(m_pSensors[i].z+m_pos.z-m_pSensorsPoints[i].z);
				if (!bActive)
					m_pSensorsPoints[i].z = m_pSensors[i].z+m_pos.z;
				m_iSensorsActive |= bActive<<i;
				m_pSensorsPoints[i] = (m_pSensorsPoints[i]-m_pos)*m_qrot;
				m_pSensorsSlopes[i] = m_pSensorsSlopes[i]*m_qrot;
			}
		}

		ComputeBBox(); 
		m_pWorld->RepositionEntity(this,1);
	} else if (!m_bActive) {
		if (m_velRequested.len2()>0) {
			m_pos += m_velRequested*time_interval;
			m_BBox[0] += m_velRequested*time_interval; m_BBox[1] += m_velRequested*time_interval;
			m_pWorld->RepositionEntity(this,1);
		}
		ReleaseGroundCollider();
		m_iSensorsActive = 0;
	}
	
	//if (m_pWorld->m_vars.bMultiplayer)
	//	m_pos = CompressPos(m_pos);
	m_iHist = m_iHist+1 & m_szHistory-1;
	m_history[m_iHist].dt = time_interval;
	m_history[m_iHist].v = m_vel;
	m_history[m_iHist].pos = m_pos;
	m_history[m_iHist].q = m_qrot;
	m_history[m_iHist].bFlying = m_bFlying;
	m_history[m_iHist].timeFlying = m_timeFlying;
	m_history[m_iHist].minFlyTime = m_minFlyTime;
	m_history[m_iHist].timeUseLowCap = m_timeUseLowCap;
	m_history[m_iHist].nslope = m_nslope;
	m_history[m_iHist].idCollider = m_pLastGroundCollider ? m_pWorld->GetPhysicalEntityId(m_pLastGroundCollider) : -2;
	m_history[m_iHist].iColliderPart = m_iLastGroundColliderPart;
	m_history[m_iHist].posColl = m_posLastGroundColl;

	m_timeSmooth = max(0.0f,m_timeSmooth-time_interval);
	if (m_pWorld->m_bUpdateOnlyFlagged) {
		m_deltaPos = m_posLocal-m_pos;
		if (m_deltaPos.len2()<sqr(0.01f) || m_deltaPos.len2()>sqr(2.0f))
			m_deltaPos.zero();
		m_timeSmooth = 0.3f;
	}

	if (!m_bStateReading && m_flags & pef_custom_poststep)
		m_pWorld->m_pEventClient->OnPostStep(this,m_pForeignData,m_iForeignData,time_interval);
	
	return 1;
}


void CLivingEntity::StepBackEx(float time_interval, bool bRollbackHistory)
{
	float dt; int idx,nSteps;
	for(dt=0,idx=m_iHist,nSteps=0; dt+0.00001f<time_interval && nSteps<m_szHistory; 
			dt+=m_history[idx].dt,idx=idx-1&m_szHistory-1,nSteps++);
	if (nSteps>=m_szHistory || dt>1E9)
		return;

	float a1=dt-time_interval, a0=m_history[idx+1&m_szHistory-1].dt-a1, rdt;
	if (fabsf(a1)<0.00001f || m_history[idx+1&m_szHistory-1].dt==0) {
		a1=0; a0=1.0f;
	} else {
		rdt = 1.0f/m_history[idx+1&m_szHistory-1].dt; a0*=rdt; a1*=rdt;
	}

	m_pos = m_history[idx].pos*a0 + m_history[idx+1&m_szHistory-1].pos*a1;
	m_vel = m_history[idx].v*a0 + m_history[idx+1&m_szHistory-1].v*a1;
	//(m_qrot = m_history[idx].q*a0 + m_history[idx+1&m_szHistory-1].q*a1).Normalize();
	m_bFlying = m_history[idx].bFlying;
	m_nslope = m_history[idx].nslope;
	m_timeFlying = m_history[idx].timeFlying*a0 + m_history[idx+1&m_szHistory-1].timeFlying*a1;
	m_minFlyTime = m_history[idx].minFlyTime*a0 + m_history[idx+1&m_szHistory-1].minFlyTime*a1;
	m_timeUseLowCap = m_history[idx].timeUseLowCap*a0 + m_history[idx+1&m_szHistory-1].timeUseLowCap*a1;
	SetGroundCollider(
		m_history[idx].idCollider>-2 ? (CPhysicalEntity*)m_pWorld->GetPhysicalEntityById(m_history[idx].idCollider) : 0);
	m_iLastGroundColliderPart = m_history[idx].iColliderPart;
	m_posLastGroundColl = m_history[idx].posColl;

	if (bRollbackHistory) {
		idx = idx+1&m_szHistory-1;
		m_history[idx].dt = dt-time_interval;
		m_history[idx].pos = m_pos;
		m_history[idx].v = m_vel;
		//m_history[idx].q = m_qrot;
		m_history[idx].bFlying = m_bFlying;
		m_history[idx].nslope = m_nslope;
		m_history[idx].timeFlying = m_timeFlying;
		m_history[idx].minFlyTime = m_minFlyTime;
		m_history[idx].timeUseLowCap = m_timeUseLowCap;
		m_iHist = idx;
	}
}


float CLivingEntity::CalcEnergy(float time_interval)
{
	float E=0;
	for(int i=0;i<m_nContacts;i++) // account for extra energy we are going to add by enforcing penertation unprojections
		E += sqr(min(3.0f,(m_pContacts[i].penetration*10.0f)))*m_pContacts[i].pent->GetRigidBody(m_pContacts[i].ipart)->M;
	return E; 
}

int CLivingEntity::RegisterContacts(float time_interval,int nMaxPlaneContacts)
{
	int i,j;
	vectorf pt[2];
	float h;
	entity_contact *pcontact;

	if (m_iSimClass!=7) for(i=0;i<m_nContacts;i++) {
		h = m_pContacts[i].pt.z-m_pContacts[i].center.z;
		pt[0] = pt[1] = m_pContacts[i].pt;
		if (fabs_tpl(m_pContacts[i].n.z)>0.7f && fabs_tpl(h)>m_size.z*0.9f) {	// contact with cap
			pt[1] += (m_pContacts[i].center-pt[0])*2;	pt[1].z = pt[0].z;
		} else { // contact with side
			pt[0].z -= h-m_size.z; pt[1].z -= h+m_size.z;
		} 

		for(j=0;j<2;j++) {
			if (!(pcontact=(entity_contact*)AllocSolverTmpBuf(sizeof(entity_contact)))) 
				return 0;
			pcontact->flags = 0;
			pcontact->pent[0] = m_pContacts[i].pent;
			pcontact->ipart[0] = m_pContacts[i].ipart;
			pcontact->pbody[0] = m_pContacts[i].pent->GetRigidBody(m_pContacts[i].ipart);
			pcontact->pent[1] = this;
			pcontact->ipart[1] = 0;
			pcontact->pbody[1] = GetRigidBody();
			pcontact->friction = 0;
			pcontact->vreq = m_pContacts[i].n*min(3.0f,(m_pContacts[i].penetration*10.0f));
			pcontact->pt[0] = pcontact->pt[1] = pt[j];
			pcontact->n = m_pContacts[i].n;
			pcontact->K.SetZero();
			m_pContacts[i].pent->GetContactMatrix(m_pContacts[i].pt, m_pContacts[i].ipart, pcontact->K);
			::RegisterContact(pcontact);
		}
	}

	return 1;
}

int CLivingEntity::Update(float time_interval, float damping)
{
	for(int i=0;i<m_nColliders;i++) {
		m_pColliders[i]->RemoveCollider(this);
		m_pColliders[i]->Release();
	}
	m_nColliders = 0;
	m_nContacts = 0;	
	return 1;
}


void CLivingEntity::DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags)
{
	if (m_bActive) {
		CPhysicalEntity::DrawHelperInformation(DrawLineFunc, flags);

		geom_world_data gwd;
		gwd.R = matrix3x3f(m_qrot);
		gwd.offset = m_pos + gwd.R*m_parts[0].pos;
		if (m_bUseSphere)
			m_SphereGeom.DrawWireframe(DrawLineFunc,&gwd,0);
		if (m_HeadGeom.m_sphere.r>0) {
			gwd.offset += gwd.R*vectorf(0,0,m_hHead-m_hCyl-m_hPivot-m_dh);
			m_HeadGeom.DrawWireframe(DrawLineFunc,&gwd,0);
		}
	}
}


void CLivingEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	CPhysicalEntity::GetMemoryStatistics(pSizer);
	if (GetType()==PE_LIVING)
		pSizer->AddObject(this, sizeof(CLivingEntity));
	if (m_nSensors>0) {
		if (m_pSensors) pSizer->AddObject(m_pSensors, m_nSensors*sizeof(m_pSensors[0]));
		if (m_pSensorsPoints) pSizer->AddObject(m_pSensorsPoints, m_nSensors*sizeof(m_pSensorsPoints[0]));
		if (m_pSensorsSlopes) pSizer->AddObject(m_pSensorsSlopes, m_nSensors*sizeof(m_pSensorsSlopes[0]));
	}
}