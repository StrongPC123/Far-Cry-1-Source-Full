//////////////////////////////////////////////////////////////////////
//
//	Rope Entity
//	
//	File: ropeentity.cpp
//	Description : CRopeEntity class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "overlapchecks.h"
#include "raybv.h"
#include "raygeom.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "geoman.h"
#include "physicalworld.h"
#include "ropeentity.h"


CRopeEntity::CRopeEntity(CPhysicalWorld *pWorld) : CPhysicalEntity(pWorld)
{
	m_nSegs = -1;
	m_segs = 0;
	m_length = 0;
	m_pTiedTo[0] = m_pTiedTo[1] = 0;
	m_iTiedPart[0] = m_iTiedPart[1] = 0;
	m_idConstraint = 0;
	m_iConstraintClient = 0;
	m_gravity = pWorld->m_vars.gravity;
	m_bAwake = m_nSlowFrames = 0;
	m_damping = 0.5f;
	m_iSimClass = 4;
	m_Emin = sqr(0.003f);
	m_maxAllowedStep = 0.01f;
	m_mass = 1.0f;
	m_collDist = 0.01f;
	m_flags = 0;
	m_surface_idx = 0;
	m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = -3;
	m_defpart.id = 0;
	m_friction = 0.2f;
}

CRopeEntity::~CRopeEntity()
{
	if (m_segs)
		delete[] m_segs;
}

void CRopeEntity::AlertNeighbourhoodND()
{
	pe_params_rope pr;
	pr.pEntTiedTo[0] = pr.pEntTiedTo[1] = 0;
	SetParams(&pr);
	if (m_segs)
		delete[] m_segs;
	m_segs = 0; m_nSegs = 0;
}

int CRopeEntity::Awake(int bAwake,int iSource)
{ 
	m_bAwake=bAwake; m_nSlowFrames=0; 
	int i;
	if (m_nSegs>0) {
		for(i=0;i<=m_nSegs;i++)	if (m_segs[i].pContactEnt && m_segs[i].pContactEnt->m_iSimClass==7)
			m_segs[i].pContactEnt = 0;
		for(i=0;i<2;i++) if (m_pTiedTo[i] && m_pTiedTo[i]->m_iSimClass==7)
			m_pTiedTo[i] = 0;
	}
	return 1; 
}


int CRopeEntity::SetParams(pe_params *_params)
{
	int res;
	vectorf prevpos = m_pos;
	if (res = CPhysicalEntity::SetParams(_params)) {
		if (_params->type==pe_params_pos::type_id) {
			//matrix3x3f R; m_qrot.getmatrix(R); //Q2M_IVO
			matrix3x3f R = matrix3x3f(m_qrot);
			for(int i=0;i<=m_nSegs;i++) {
				m_segs[i].pt = R*(m_segs[i].pt-prevpos)+m_pos;
				m_segs[i].vel = R*m_segs[i].vel;
				m_segs[i].dir = R*m_segs[i].dir;
			}
			m_qrot.SetIdentity();
		}
		return res;
	}

	if (_params->type==pe_simulation_params::type_id) {
		pe_simulation_params *params = (pe_simulation_params*)_params;
		if (!is_unused(params->gravity)) m_gravity = params->gravity;
		if (!is_unused(params->damping)) m_damping = params->damping;
		if (!is_unused(params->maxTimeStep)) m_maxAllowedStep = params->maxTimeStep;
		if (!is_unused(params->minEnergy)) m_Emin = params->minEnergy;
		return 1;
	}

	if (_params->type==pe_params_rope::type_id) {
		pe_params_rope *params = (pe_params_rope*)_params;
		int i;
		if (!is_unused(params->length)) m_length = params->length;
		if (!is_unused(params->mass)) m_mass = params->mass;
		if (!is_unused(params->collDist)) m_collDist = params->collDist;
		if (!is_unused(params->surface_idx)) m_surface_idx = params->surface_idx;
		if (!is_unused(params->friction)) m_friction = params->friction;
		if (!is_unused(params->nSegments)) {
			if (params->nSegments>m_nSegs) {
				if (m_segs) delete[] m_segs;
				m_segs = new rope_segment[params->nSegments+1];
				memset(m_segs, 0, (params->nSegments+1)*sizeof(rope_segment));
				float seglen = m_length/params->nSegments;
				for(i=0;i<=params->nSegments;i++)
					(m_segs[i].pt = m_pos).z -= seglen*i;
			}
			m_nSegs = params->nSegments;
		}
		if (!is_unused(params->pPoints) && params->pPoints) 
			for(i=0;i<=m_nSegs;i++) m_segs[i].pt = params->pPoints[i];
		if (!is_unused(params->pVelocities) && params->pVelocities)
			for(i=0;i<=m_nSegs;i++) m_segs[i].vel = params->pVelocities[i];

		if (m_idConstraint && m_pTiedTo[m_iConstraintClient]) {
			pe_action_remove_constraint arc;
			arc.idConstraint = m_idConstraint;
			m_pTiedTo[m_iConstraintClient]->Action(&arc);
			m_idConstraint = 0;
		}
		for(i=0;i<2;i++) if (m_pTiedTo[i]) {
			if (m_pTiedTo[i]->m_iSimClass==7)
				m_pTiedTo[i] = 0;
			else
				m_pTiedTo[i]->Awake();
		}

		for(i=0;i<2;i++) if (!is_unused(params->pEntTiedTo[i])) {
			if (m_pTiedTo[i]) m_pTiedTo[i]->Release();
			m_pTiedTo[i] = params->pEntTiedTo[i]==WORLD_ENTITY ? &g_StaticPhysicalEntity : 
										 (params->pEntTiedTo[i] ? ((CPhysicalPlaceholder*)params->pEntTiedTo[i])->GetEntity() : 0);
			if (!is_unused(params->idPartTiedTo[i])) {
				for(m_iTiedPart[i]=0; m_iTiedPart[i]<m_pTiedTo[i]->m_nParts && 
						params->idPartTiedTo[i]!=m_pTiedTo[i]->m_parts[m_iTiedPart[i]].id; m_iTiedPart[i]++);
				if (m_iTiedPart[i]>=m_pTiedTo[i]->m_nParts)
					m_iTiedPart[i] = 0;
			}
			if (m_pTiedTo[i]) {
				RigidBody *pbody = m_pTiedTo[i]->GetRigidBody(m_iTiedPart[i]);
				if (!is_unused(params->ptTiedTo[i]))
					m_ptTiedLoc[i] = (params->ptTiedTo[i]-pbody->pos)*pbody->q;
				else if (!is_unused(params->pEntTiedTo[i]) || !is_unused(params->idPartTiedTo[i]))
					m_ptTiedLoc[i] = (m_segs[m_nSegs*i].pt-pbody->pos)*pbody->q;
				m_pTiedTo[i]->AddRef();
			}
		}
		m_bAwake = 1;
		m_nSlowFrames = 0;

		if (m_pTiedTo[0] && (m_pTiedTo[0]==this || m_pTiedTo[0]->m_iSimClass==3 || m_pTiedTo[0]->GetType()==PE_ROPE || m_pTiedTo[0]->GetType()==PE_SOFT) ||
				m_pTiedTo[1] && (m_pTiedTo[1]==this || m_pTiedTo[1]->m_iSimClass==3 || m_pTiedTo[1]->GetType()==PE_ROPE || m_pTiedTo[1]->GetType()==PE_SOFT))
		{	// rope cannot be attached to such objects
			if (m_pTiedTo[0]) m_pTiedTo[0]->Release();
			if (m_pTiedTo[1]) m_pTiedTo[1]->Release();
			m_pTiedTo[0] = 0; m_pTiedTo[1] = 0;
			return 0;
		}

		if (m_pTiedTo[0] && m_pTiedTo[1]) {
			pe_action_add_constraint aac;
			m_iConstraintClient = isneg(m_pTiedTo[0]->GetMassInv()-m_pTiedTo[1]->GetMassInv());
			for(i=0;i<2;i++) {
				RigidBody *pbody = m_pTiedTo[m_iConstraintClient^i]->GetRigidBody(m_iTiedPart[m_iConstraintClient^i]);
				aac.pt[i] = pbody->pos + pbody->q*m_ptTiedLoc[m_iConstraintClient^i];
				aac.partid[i] = m_pTiedTo[m_iConstraintClient^i]->m_parts[m_iTiedPart[m_iConstraintClient^i]].id;
			}
			aac.pBuddy = m_pTiedTo[m_iConstraintClient^1];
			aac.pConstraintEntity = this;
			m_idConstraint = m_pTiedTo[m_iConstraintClient]->Action(&aac);
		}

		return 1;
	}

	return 0;
}


int CRopeEntity::GetParams(pe_params *_params)
{
	int res;
	if (res = CPhysicalEntity::GetParams(_params))
		return res;

	if (_params->type==pe_simulation_params::type_id) {
		pe_simulation_params *params = (pe_simulation_params*)_params;
		params->gravity = m_gravity;
		params->damping = m_damping;
		params->maxTimeStep = m_maxAllowedStep;
		params->minEnergy = m_Emin;
		return 1;
	}

	if (_params->type==pe_params_rope::type_id) {
		pe_params_rope *params = (pe_params_rope*)_params;
		params->length = m_length;
		params->mass = m_mass;
		params->nSegments = m_nSegs;
		params->collDist = m_collDist;
		params->surface_idx = m_surface_idx;
		params->friction = m_friction;
		params->pPoints = &m_segs[0].pt;
		params->pVelocities = &m_segs[0].vel;
		params->iStride = sizeof(rope_segment);
		for(int i=0;i<2;i++) if (params->pEntTiedTo[i] = m_pTiedTo[i]) {
			if (m_pTiedTo[i]==&g_StaticPhysicalEntity)
				params->pEntTiedTo[i] = WORLD_ENTITY;
			params->idPartTiedTo[i] = m_pTiedTo[i]->m_parts[m_iTiedPart[i]].id;
			RigidBody *pbody = m_pTiedTo[i]->GetRigidBody(m_iTiedPart[i]);
			params->ptTiedTo[i] = pbody->pos + pbody->q*m_ptTiedLoc[i];
		}
		return 1;
	}

	return 0;
}


int CRopeEntity::GetStatus(pe_status *_status)
{
	int res;
	if (res = CPhysicalEntity::GetStatus(_status)) {
		if (_status->type==pe_status_caps::type_id) {
			pe_status_caps *status = (pe_status_caps*)_status;
			status->bCanAlterOrientation = 1;
		}
		return res;
	}

	if (_status->type==pe_status_rope::type_id) {
		pe_status_rope *status = (pe_status_rope*)_status;
		status->nSegments = m_nSegs;
		int i;
		if (!is_unused(status->pPoints) && status->pPoints)
			for(i=0;i<=m_nSegs;i++) status->pPoints[i] = m_segs[i].pt;
		if (!is_unused(status->pVelocities) && status->pVelocities)
			for(i=0;i<=m_nSegs;i++) status->pVelocities[i] = m_segs[i].vel;
		return 1;
	}

	return 0;
}


int CRopeEntity::Action(pe_action *_action)
{
	int res,i,j;
	if (res = CPhysicalEntity::Action(_action))
		return res;

	if (_action->type==pe_action_impulse::type_id) {
		pe_action_impulse *action = (pe_action_impulse*)_action;
		if (!is_unused(action->ipart)) i = action->ipart;
		else if (!is_unused(action->partid)) i = action->partid;
		else if (!is_unused(action->point)) {
			for(j=1,i=0;j<=m_nSegs;j++) if ((m_segs[j].pt-action->point).len2()<(m_segs[i].pt-action->point).len2())
				i=j;
			if ((m_segs[i].pt-action->point).len2()>sqr(m_collDist*3) && 
					(i==0 || (m_segs[i].pt-m_segs[i-1].pt^action->point-m_segs[i-1].pt).len2() > sqr(m_collDist)*(m_segs[i].pt-m_segs[i-1].pt).len2()))
				return 0;
		} else
			return 0;
		if ((unsigned int)i>(unsigned int)m_nSegs)
			return 0;
		m_segs[i].vel_ext += action->impulse*((m_nSegs+1)/m_mass);
		m_bAwake = 1; m_nSlowFrames = 0;
		return 1;
	}

	if (_action->type==pe_action_reset::type_id) {
		for(i=0;i<=m_nSegs;i++) {
			m_segs[i].vel.zero();
			if (m_segs[i].pContactEnt) {
				m_segs[i].pContactEnt->Release();
				m_segs[i].pContactEnt = 0;
			}
		}
		return 1;
	}

	return 0;
}


void CRopeEntity::StartStep(float time_interval)
{
	m_timeStepPerformed = 0;
	m_timeStepFull = time_interval;
}


float CRopeEntity::GetMaxTimeStep(float time_interval)
{
	if (m_timeStepPerformed > m_timeStepFull-0.001f)
		return time_interval;
	return min(min(m_timeStepFull-m_timeStepPerformed,m_maxAllowedStep),time_interval);
}

int __ropeframe = 0;


int CRopeEntity::Step(float time_interval)
{
	if (m_nSegs<=0 || !m_bAwake)
		return 1;

	int i;
	for(i=0;i<=m_nSegs;i++)	if (m_segs[i].pContactEnt && m_segs[i].pContactEnt->m_iSimClass==7)
		m_segs[i].pContactEnt = 0;
	for(i=0;i<2;i++) if (m_pTiedTo[i] && m_pTiedTo[i]->m_iSimClass==7)
		m_pTiedTo[i] = 0;

	if (m_timeStepPerformed > m_timeStepFull-0.001f)
		return 1;
	m_timeStepPerformed += time_interval;

	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );
	PHYS_ENTITY_PROFILER

__ropeframe++;

	float seglen=m_length/m_nSegs,seglen2=sqr(seglen), rseglen=m_nSegs/m_length,rseglen2=sqr(rseglen),Ebefore; 
	int iDir,iStart,iEnd,iter,bStretched,bHasContacts=0;
	vectorf dir,ptend[2],sz;
	float len2,diff,a,b,r2,r2new,pAp,vmax,k,E,damping=max(0.0f,1.0f-m_damping*time_interval);

	for(i=0;i<=m_nSegs;i++)	{
		m_segs[i].pt += m_segs[i].vel*time_interval;
		m_segs[i].vel += m_gravity*time_interval;
	}
	for(i=0;i<2;i++) if (m_pTiedTo[i]) {
		RigidBody *pbody = m_pTiedTo[i]->GetRigidBody(m_iTiedPart[i]);
		m_segs[m_nSegs*i].pt = ptend[i] = pbody->pos + pbody->q*m_ptTiedLoc[i];
		m_segs[m_nSegs*i].vel = pbody->v + (pbody->w^ptend[i]-pbody->pos);
	}

	if (m_pTiedTo[0] && m_pTiedTo[1] && (ptend[1]-ptend[0]).len2()>sqr(m_length*0.99f)) {
		float newlen=(ptend[1]-ptend[0]).len(), newseglen=newlen/m_nSegs;
		dir = (ptend[1]-ptend[0]).normalized();
		for(i=1;i<m_nSegs;i++) {
			m_segs[i].dir = dir;
			m_segs[i].pt = ptend[0] + m_segs[i].dir*(newseglen*i);
		}
		m_bAwake = m_pTiedTo[0]->GetStatus(&pe_status_awake()) | m_pTiedTo[1]->GetStatus(&pe_status_awake());
		m_BBox[0].x = min(m_segs[0].pt.x,m_segs[m_nSegs].pt.x); m_BBox[1].x = max(m_segs[0].pt.x,m_segs[m_nSegs].pt.x); 
		m_BBox[0].y = min(m_segs[0].pt.y,m_segs[m_nSegs].pt.y); m_BBox[1].y = max(m_segs[0].pt.y,m_segs[m_nSegs].pt.y); 
		m_BBox[0].z = min(m_segs[0].pt.z,m_segs[m_nSegs].pt.z); m_BBox[1].z = max(m_segs[0].pt.z,m_segs[m_nSegs].pt.z); 
		a = max(max(m_BBox[1].x-m_BBox[0].x,m_BBox[1].y-m_BBox[0].y),m_BBox[1].z-m_BBox[0].z)*0.01f+m_collDist*2;
		m_BBox[0] -= vectorf(a,a,a); m_BBox[1] += vectorf(a,a,a);
		if (m_flags & pef_traceable)
			m_pWorld->RepositionEntity(this,1);
		return 1;
	}

	// first, ensure that all vertices have distance between them equal to seglen
	iter=300;
	iDir = m_pTiedTo[1] && !m_pTiedTo[0] ? 1:-1;
	do {
		iDir = -iDir;
		iStart = m_nSegs & iDir>>31;
		iEnd = m_nSegs & -iDir>>31;

		for(i=iStart;i!=iEnd;i+=iDir)	{
			dir = m_segs[i+iDir].pt-m_segs[i].pt; len2 = dir.len2(); 
			diff = fabs_tpl(len2-seglen2);
			if (bStretched = (diff>seglen2*0.01f)) {
				if (diff<seglen2*0.2f) { // use 3 terms of 1/sqrt(x) Taylor series expansion
					k = len2*rseglen2; k = 1.875f-(1.25f-0.375f*k)*k;
				} else
					k = seglen/sqrt_tpl(len2);
				m_segs[i+iDir].pt = m_segs[i].pt + dir*k;
				iter--;
			}	else k = 1.0f;
			m_segs[i+(iDir>>31)].dir = dir*(k*rseglen);
		} 

		if (m_pTiedTo[0]) m_segs[0].pt = ptend[0];
		if (m_pTiedTo[1]) m_segs[m_nSegs].pt = ptend[1];
	} while(m_pTiedTo[iDir+1>>1] && bStretched && iter>0);

	m_BBox[0] = m_BBox[1] = m_segs[0].pt;
	for(i=0;i<=m_nSegs;i++) {
		(m_segs[i].vel += m_segs[i].vel_ext) *= damping;
		m_segs[i].vel_ext.zero();
		m_segs[i].bRecalcDir = 0;
		m_BBox[0].x = min(m_BBox[0].x, m_segs[i].pt.x); m_BBox[1].x = max(m_BBox[1].x, m_segs[i].pt.x); 
		m_BBox[0].y = min(m_BBox[0].y, m_segs[i].pt.y); m_BBox[1].y = max(m_BBox[1].y, m_segs[i].pt.y); 
		m_BBox[0].z = min(m_BBox[0].z, m_segs[i].pt.z);	m_BBox[1].z = max(m_BBox[1].z, m_segs[i].pt.z);
	}
	a = max(max(m_BBox[1].x-m_BBox[0].x,m_BBox[1].y-m_BBox[0].y),m_BBox[1].z-m_BBox[0].z)*0.01f+m_collDist*2;
	m_BBox[0] -= vectorf(a,a,a); m_BBox[1] += vectorf(a,a,a);

	if (m_flags & rope_collides) {
		CPhysicalEntity **pentlist;
		int j,iseg,nEnts,nCheckParts,nLocChecks,iend;
		box boxrope,boxpart;
		vectorf center,ptres[2],rotax,n;
		float angle,dist2,t,cost,sint;
		struct check_part {
			vectorf offset;
			matrix3x3f R;
			float scale;
			box bbox;
			CPhysicalEntity *pent;
			int ipart;
		};
		check_part checkParts[20];
		CRayGeom aray; aray.m_iCollPriority = 10;
		intersection_params ip;
		geom_contact *pcontact;
		CPhysicalEntity *pent;
		geom_world_data gwd;

		g_Overlapper.Init();
		boxrope.size = (m_BBox[1]-m_BBox[0])*0.5f;
		center = (m_BBox[0]+m_BBox[1])*0.5f;
		ip.bStopAtFirstTri = true;
		ip.iUnprojectionMode = 1;

		nEnts = m_pWorld->GetEntitiesAround(m_BBox[0],m_BBox[1], pentlist, 
			(m_flags & rope_collides_with_terrain ? ent_terrain:0) | 
			ent_static|ent_sleeping_rigid|ent_rigid|ent_living|ent_sort_by_mass|ent_ignore_noncolliding|ent_triggers, this);

		for(i=j=nCheckParts=0;i<nEnts;i++) if (pentlist[i]!=this)
		for(j=0;j<pentlist[i]->m_nParts;j++) {
			checkParts[nCheckParts].offset = pentlist[i]->m_pos+pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
			//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(boxrope.Basis);	//Q2M_IVO
			boxrope.Basis = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
			boxrope.center = (center-checkParts[nCheckParts].offset)*boxrope.Basis;
			if (pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom->GetType()!=GEOM_HEIGHTFIELD) {
				pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom->GetBBox(&checkParts[nCheckParts].bbox);
				checkParts[nCheckParts].bbox.center *= pentlist[i]->m_parts[j].scale;
				checkParts[nCheckParts].bbox.size *= pentlist[i]->m_parts[j].scale;
			} else
				checkParts[nCheckParts].bbox = boxrope;
			boxrope.bOriented++;
			if (box_box_overlap_check(&boxrope,&checkParts[nCheckParts].bbox)) {
				checkParts[nCheckParts].pent = pentlist[i];
				checkParts[nCheckParts].ipart = j;
				checkParts[nCheckParts].R = boxrope.Basis;
				checkParts[nCheckParts].scale = pentlist[i]->m_parts[j].scale;
				pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom->PrepareForRayTest(
					pentlist[i]->m_parts[j].scale==1.0f ? seglen:seglen/pentlist[i]->m_parts[j].scale);
				if (++nCheckParts==sizeof(checkParts)/sizeof(checkParts[0]))
					goto enoughgeoms;
			}
		} enoughgeoms:

		if (m_pTiedTo[0] && m_pTiedTo[1])
			iDir = isneg(m_pTiedTo[1]->GetRigidBody(m_iTiedPart[1])->Minv-m_pTiedTo[0]->GetRigidBody(m_iTiedPart[0])->Minv);
		else 
			iDir = iszero((intptr_t)m_pTiedTo[0]);
		iDir = 1-iDir*2;
		iStart = m_nSegs & iDir>>31;
		iEnd = m_nSegs & -iDir>>31;
		for(i=iStart;i!=iEnd;i+=iDir)	{
			iseg = i+(iDir>>31);
			if (pent=m_segs[iseg].pContactEnt) {
				//(pent->m_qrot*pent->m_parts[m_segs[iseg].iContactPart].q).getmatrix(gwd.R);	//Q2M_IVO 
				gwd.R = matrix3x3f(pent->m_qrot*pent->m_parts[m_segs[iseg].iContactPart].q);
				gwd.offset = pent->m_pos + pent->m_qrot*pent->m_parts[m_segs[iseg].iContactPart].pos;
				gwd.scale = pent->m_parts[m_segs[iseg].iContactPart].scale;
				if (!m_segs[iseg].bRecheckContact &&
						pent->m_parts[m_segs[iseg].iContactPart].pPhysGeomProxy->pGeom->FindClosestPoint(&gwd, m_segs[iseg].iPrim,m_segs[iseg].iFeature, 
							m_segs[i+iDir].pt,m_segs[i].pt, ptres)>=0 && 
						(dist2=(n=ptres[1]-ptres[0]).len2())<sqr(m_collDist*2) && // drop contact when gap becomes too big
						sqr_signed(m_segs[iseg].vreq=n*m_segs[iseg].ncontact)>n.len2()*sqr(0.75f)) // drop contact if normal changes abruptly
				{
					n.normalize();
					m_segs[i].tcontact = (ptres[1]-m_segs[iseg].pt).len()*rseglen;
					if (dist2<sqr(m_collDist*0.85f)) { // reinforce the distance
						t = (iDir>>31&1) + m_segs[i].tcontact*iDir; // 1.0-t for iDir==-1
						j = isneg(t-0.1f); t += j; j = -j & iDir;
						if ((unsigned int)i-j<=(unsigned int)m_nSegs) {
							rotax = (m_segs[i+iDir-j].pt-m_segs[i-j].pt^n).normalized();
							angle = (m_collDist-sqrt_tpl(dist2))/(t*seglen);
							m_segs[i+iDir-j].pt = m_segs[i+iDir-j].pt.rotated(m_segs[i-j].pt, rotax, cos_tpl(angle),sin_tpl(angle));
							m_segs[i+iDir-j].bRecalcDir = m_segs[max(0,i+iDir-j-1)].bRecalcDir = 1;
						}
						m_segs[iseg].vreq = 0;	 
					} else
						m_segs[iseg].vreq = (m_collDist-m_segs[iseg].vreq)*10.0f;
					m_segs[i].ncontact = n;
					RigidBody *pbody = m_segs[iseg].pContactEnt->GetRigidBody(m_segs[iseg].iContactPart);
					m_segs[iseg].vcontact = pbody->v+(pbody->w^ptres[0]-pbody->pos);
					bHasContacts = 1;
				}	else {
					m_segs[iseg].pContactEnt->Release();
					m_segs[iseg].pContactEnt = 0;
				}
			}

			if (!m_segs[iseg].pContactEnt) for(j=nLocChecks=0;j<nCheckParts && nLocChecks<6;j++) {
				aray.m_ray.origin = (m_segs[i].pt-checkParts[j].offset)*checkParts[j].R;
				aray.m_ray.dir = (m_segs[i+iDir].pt-checkParts[j].offset)*checkParts[j].R-aray.m_ray.origin;
				if (box_ray_overlap_check(&checkParts[j].bbox,&aray.m_ray)) {
					ip.centerOfRotation = aray.m_ray.origin;
					gwd.offset.zero(); gwd.R.SetIdentity(); gwd.scale=checkParts[j].scale;
					if (checkParts[j].pent->m_parts[checkParts[j].ipart].pPhysGeomProxy->pGeom->Intersect(&aray,&gwd,0,&ip,pcontact) && 
							pcontact->iUnprojMode==1) 
					{
						if (pcontact->t>(real)0.5) {
							pcontact->t=(real)0.5; m_segs[iseg].bRecheckContact=1;
						} else
							m_segs[iseg].bRecheckContact=0;
						if (m_segs[iseg].pContactEnt)
							m_segs[iseg].pContactEnt->Release();
						m_segs[iseg].pContactEnt = 0;
						diff = m_segs[iseg].tcontact = (pcontact->pt-aray.m_ray.origin).len();
						m_segs[iseg].tcontact = m_segs[iseg].tcontact*iDir-seglen*(iDir>>31); // flip tcontact when iDir is -1
						iend = -iseg>>31 & 1;
						if ((unsigned int)(iseg-1)>=(unsigned int)(m_nSegs-2) && m_pTiedTo[iend] && isneg(m_segs[iseg].tcontact*rseglen-0.5f)^iend)
							continue; // ignore collisions too close to tied ends
						(m_segs[iseg].pContactEnt = checkParts[j].pent)->AddRef();
						m_segs[iseg].iContactPart = checkParts[j].ipart;
						rotax = checkParts[j].R*-pcontact->dir;
						if (m_collDist > diff*0.25f) {
							if ((unsigned int)(i-iDir)>(unsigned int)m_nSegs)
								continue;
							// the contact is too close to the segment start (requires >15 deg. gap unproj), rotate start point to get the safe gap
							float tgap = m_collDist/seglen;
							cost = cos_tpl(tgap); sint = sin_tpl(tgap);
							m_segs[i].pt = m_segs[i].pt.rotated(m_segs[i-iDir].pt, rotax, cost,sint);
							m_segs[i+iDir].pt = m_segs[i+iDir].pt.rotated(m_segs[i-iDir].pt, rotax, cost,sint);
							m_segs[i].bRecalcDir = m_segs[max(0,i-1)].bRecalcDir = 1;
						}	else
							pcontact->t += m_collDist/diff;
						cost = cos_tpl(pcontact->t); sint = sin_tpl(pcontact->t);
						m_segs[iseg].ncontact = checkParts[j].R*(pcontact->n.normalized().rotated(pcontact->dir,cost,-sint));
						m_segs[iseg].tcontact	*= rseglen;
						m_segs[i+iDir].pt = m_segs[i+iDir].pt.rotated(m_segs[i].pt, rotax, cost,sint);
						aray.m_ray.dir = (m_segs[i+iDir].pt-checkParts[j].offset)*checkParts[j].R-aray.m_ray.origin;
						m_segs[iseg].iPrim = pcontact->iPrim[0];
						m_segs[iseg].iFeature = pcontact->iFeature[0];
						/*m_segs[iseg].friction[0] = 0.5f*max(0.0f, m_pWorld->m_FrictionTable[m_surface_idx&NSURFACETYPES-1] + 
							m_pWorld->m_FrictionTable[pcontact->id[0]&NSURFACETYPES-1]);
						m_segs[iseg].friction[1] = 0.5f*max(0.0f,	m_pWorld->m_DynFrictionTable[m_surface_idx&NSURFACETYPES-1] + 
							m_pWorld->m_DynFrictionTable[pcontact->id[0]&NSURFACETYPES-1]);*/
						m_segs[iseg].friction[0] = m_segs[iseg].friction[1] = m_friction;
						RigidBody *pbody = m_segs[iseg].pContactEnt->GetRigidBody(m_segs[iseg].iContactPart);
						m_segs[iseg].vcontact = pbody->v+(pbody->w^pcontact->pt-pbody->pos);
						m_segs[i+iDir].bRecalcDir = m_segs[max(0,i+iDir-1)].bRecalcDir = 1;
						m_segs[iseg].vreq = 0;
						bHasContacts = 1;
						//break;
					}
					//nLocChecks++;
				}
			}
		}

		for(i=0;i<m_nSegs;i++) if (m_segs[i].bRecalcDir)
			m_segs[i].dir = (m_segs[i+1].pt-m_segs[i].pt).normalized();
	}

	for(i=0,Ebefore=0;i<=m_nSegs;i++)
		Ebefore += m_segs[i].vel.len2();

	if (bHasContacts) {
		int iter,bBounced;
		float vrel,dPtang;
		vectorf dp;

		for(i=0;i<m_nSegs;i++) 
			m_segs[i].kdP = 0.5f;
		if (m_pTiedTo[0]) m_segs[0].kdP=0.0f;
		if (m_pTiedTo[1]) m_segs[m_nSegs-1].kdP=1.0f;

		// solve for velocities using relaxation solver with friction
		iter = 200;
		do {
			for(i=bBounced=0;i<m_nSegs;i++) {
				if (fabsf(vrel=(m_segs[i+1].vel-m_segs[i].vel)*m_segs[i].dir) > seglen*0.005f) {
					m_segs[i].vel += m_segs[i].dir*(vrel*m_segs[i].kdP);
					m_segs[i+1].vel -= m_segs[i].dir*(vrel*(1.0f-m_segs[i].kdP));
					bBounced++;
				}
				if (m_segs[i].pContactEnt) {
					dp = m_segs[i].vel*(1.0f-m_segs[i].tcontact)+m_segs[i+1].vel*m_segs[i].tcontact-m_segs[i].vcontact;
					if ((vrel=dp*m_segs[i].ncontact-m_segs[i].vreq) < -seglen*0.005f) {
						if (m_segs[i].friction[0]>0.01f) {
							m_segs[i].dP -= dPtang=sqrt_tpl(max(0.0f,dp.len2()-sqr(vrel)));
							if (m_segs[i].dP-vrel*m_segs[i].friction[0] < 0) {	// friction cannot stop sliding
								dp += (dp-m_segs[i].ncontact*vrel)*((m_segs[i].dP-vrel*m_segs[i].friction[1])/dPtang); // remove part of dp that friction cannot stop
								dp *= vrel/(m_segs[i].ncontact*dp); // apply impulse along dp so that it stops normal component
								m_segs[i].dP = 0;
							} else 
								m_segs[i].dP -= vrel*m_segs[i].friction[0];
						} else
							dp = m_segs[i].ncontact*vrel;
						m_segs[i].vel -= dp*((1.0f-m_segs[i].tcontact)*m_segs[i].kdP);
						m_segs[i+1].vel -= dp*(m_segs[i].tcontact*(1.0f-m_segs[i].kdP));
						bBounced++;
					}
				}
			}
		} while (bBounced && --iter);
	}	else {
		// solve for velocities using conjugate gradient
		for(i=0,r2=0;i<m_nSegs;i++) {
			r2 += sqr(m_segs[i].dP = m_segs[i].r = (m_segs[i+1].vel-m_segs[i].vel)*m_segs[i].dir);
			m_segs[i].cosnext = m_segs[i].dir*m_segs[i+1].dir;
			m_segs[i].kdP = 2.0f;
			m_segs[i].P = 0;
		}
		if (m_pTiedTo[0]) m_segs[0].kdP=1.0f;
		if (m_pTiedTo[1]) m_segs[m_nSegs-1].kdP=1.0f;
		iter = m_nSegs*3;

		do {
			m_segs[0].dv = m_segs[0].dP*m_segs[0].kdP;
			m_segs[1].dv = -m_segs[0].cosnext*m_segs[0].dP;
			for(i=1;i<m_nSegs;i++) {
				m_segs[i-1].dv -= m_segs[i-1].cosnext*m_segs[i].dP;
				m_segs[i].dv += m_segs[i].dP*m_segs[i].kdP;
				m_segs[i+1].dv = -m_segs[i].cosnext*m_segs[i].dP;
			}

			for(i=0,pAp=0;i<m_nSegs;i++)
				pAp += m_segs[i].dv*m_segs[i].dP;
			if (fabs_tpl(pAp)<1E-10) break;
			a = r2/pAp;	
			for(i=0,r2new=0;i<m_nSegs;i++) {
				r2new += sqr(m_segs[i].r -= m_segs[i].dv*a);
				m_segs[i].P += m_segs[i].dP*a;
			}
			if (r2new>r2*500)
				break;
			b = r2new/r2; r2 = r2new;
			for(i=0,vmax=0;i<m_nSegs;i++) {
				(m_segs[i].dP*=b) += m_segs[i].r;
				vmax = max(vmax,sqr(m_segs[i].r));
			}
		} while(--iter && vmax>sqr(0.003f));

		if (!m_pTiedTo[0]) m_segs[0].vel += m_segs[0].dir*m_segs[0].P;
		if (!m_pTiedTo[1]) m_segs[m_nSegs].vel -= m_segs[m_nSegs-1].dir*m_segs[m_nSegs-1].P;
		m_segs[1].vel -= m_segs[0].dir*m_segs[0].P;
		m_segs[m_nSegs-1].vel += m_segs[m_nSegs-1].dir*m_segs[m_nSegs-1].P;
		for(i=1;i<m_nSegs-1;i++) {
			m_segs[i].vel += m_segs[i].dir*m_segs[i].P;
			m_segs[i+1].vel -= m_segs[i].dir*m_segs[i].P;
		}
	}

	for(i=0,E=0;i<=m_nSegs;i++) E += m_segs[i].vel.len2();
	if (E>Ebefore && E>m_Emin) {
		k = sqrt_tpl(Ebefore/E);
		for(i=0;i<=m_nSegs;i++) m_segs[i].vel*=k;
		E = Ebefore;
	}
	i = -isneg(E-m_Emin*(m_nSegs+1));
	m_nSlowFrames = (m_nSlowFrames&i)-i;
	if (!(m_bAwake = isneg(m_nSlowFrames-4)))
		for(i=iszero((intptr_t)m_pTiedTo[0])^1; i<m_nSegs+iszero((intptr_t)m_pTiedTo[1]); i++)
			m_segs[i].vel.zero();

	m_pos = m_segs[0].pt;
	if (m_flags & pef_traceable)
		m_pWorld->RepositionEntity(this,1);

	return isneg(m_timeStepFull-m_timeStepPerformed-0.001f);
}


int CRopeEntity::RayTrace(CRayGeom *pRay,geom_contact *&pcontacts)
{
	static geom_contact g_RopeContact;
	vectorf dp,dir,l,pt;
	float t,t1,llen2,raylen=pRay->m_ray.dir*pRay->m_dirn; 
	int i;

	for(i=0;i<m_nSegs;i++) {
		dp = pRay->m_ray.origin-m_segs[i].pt;
		if ((dp^pRay->m_dirn).len2()<sqr(m_collDist) && inrange((m_segs[i].pt-pRay->m_ray.origin)*pRay->m_dirn, 0.0f,raylen)) {
			pt = m_segs[i].pt; break;
		}
		dir = m_segs[i+1].pt-m_segs[i].pt;
		l = dir^pRay->m_dirn; llen2 = l.len2();
		t = (dp^pRay->m_dirn)*l; t1 = (dp^dir)*l;
		if (sqr(dp*l)<sqr(m_collDist)*llen2 && inrange(t, 0.0f,llen2) && inrange(t1, 0.0f,llen2*raylen)) {
			pt = m_segs[i].pt+dir*(t/llen2); break;
		}
	}

	if (i<m_nSegs) {
		pcontacts = &g_RopeContact;
		pcontacts->pt = pt;
		pcontacts->t = (pt-pRay->m_ray.origin)*pRay->m_dirn;
		pcontacts->id[0] = m_surface_idx;
		pcontacts->iNode[0] = 0;
		pcontacts->n = -pRay->m_dirn;
		return 1;
	}
	return 0;
}


int CRopeEntity::GetStateSnapshot(CStream &stm,float time_back,int flags)
{
	stm.WriteNumberInBits(SNAPSHOT_VERSION,4);
	WritePacked(stm,m_nSegs);

	if (m_nSegs>0) for(int i=0; i<=m_nSegs; i++) {
		stm.Write(m_segs[i].pt);
		if (m_segs[i].vel.len2()>0) {
			stm.Write(true);
			stm.Write(m_segs[i].vel);
		} else stm.Write(false);
	}
	stm.Write(m_bAwake!=0);

	return 1;
}


int CRopeEntity::SetStateFromSnapshot(CStream &stm, int flags)
{
	int i,ver=0; 
	bool bnz;

	stm.ReadNumberInBits(ver,4);
	if (ver!=SNAPSHOT_VERSION)
		return 0;
	ReadPacked(stm,i);
	if (i!=m_nSegs)
		return 0;

	if (!(flags & ssf_no_update)) {
		if (m_nSegs>0) for(i=0; i<=m_nSegs; i++) {
			stm.Read(m_segs[i].pt);
			stm.Read(bnz); if (bnz) 
				stm.Read(m_segs[i].vel);
			else
				m_segs[i].vel.zero();
			if (m_segs[i].pContactEnt) {
				m_segs[i].pContactEnt->Release();
				m_segs[i].pContactEnt = 0;
			}
		}
		stm.Read(bnz);
		m_bAwake = bnz ? 1:0;

		for(i=0;i<m_nSegs;i++)
			m_segs[i].dir = (m_segs[i+1].pt-m_segs[i].pt).normalized();
		m_pos = m_segs[0].pt;
		if (m_flags & pef_traceable)
			m_pWorld->RepositionEntity(this,1);
	} else {
		for(i=0;i<=m_nSegs;i++) {
			stm.Seek(stm.GetReadPos()+sizeof(vectorf)*8);
			stm.Read(bnz); if (bnz)
				stm.Seek(stm.GetReadPos()+sizeof(vectorf)*8);
		}
		stm.Read(bnz);
	}

	return 1;
}



void CRopeEntity::DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags)
{
	CPhysicalEntity::DrawHelperInformation(DrawLineFunc,flags);

	int i;
	if (flags & pe_helper_geometry) for(i=0;i<m_nSegs;i++)
		DrawLineFunc(m_segs[i].pt,m_segs[i+1].pt);
	if (flags & pe_helper_collisions) for(i=0;i<m_nSegs;i++) if (m_segs[i].pContactEnt) {
		vectorf pt = m_segs[i].pt+(m_segs[i+1].pt-m_segs[i].pt)*m_segs[i].tcontact;
		DrawLineFunc(pt,pt+m_segs[i].ncontact*m_pWorld->m_vars.maxContactGap*30);
	}
}


void CRopeEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	CPhysicalEntity::GetMemoryStatistics(pSizer);
	if (GetType()==PE_ROPE)
		pSizer->AddObject(this, sizeof(CRopeEntity));
	pSizer->AddObject(m_segs, m_nSegs*sizeof(m_segs[0]));
}