//////////////////////////////////////////////////////////////////////
//
//	Soft Entity
//	
//	File: softentity.cpp
//	Description : CSoftEntity class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "overlapchecks.h"
#include "intersectionchecks.h"
#include "raybv.h"
#include "raygeom.h"
#include "singleboxtree.h"
#include "trimesh.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "geoman.h"
#include "physicalworld.h"
#include "softentity.h"


CSoftEntity::CSoftEntity(CPhysicalWorld *pworld) : CPhysicalEntity(pworld)
{
	m_vtx=0; m_edges=0; m_pVtxEdges=0;
	m_nVtx=m_nEdges=0;
	m_maxAllowedStep = 0.1f;
	m_ks = 200.0f;
	m_thickness = 0.04f;
	m_maxSafeStep = 0.2f;
	m_kdRatio = 0.9f;
	m_airResistance = 0;//10.0f;
	m_wind.zero();
	m_waterDensity = 0;
	m_waterPlane.n.Set(0,0,1);
	m_waterPlane.origin.Set(0,0,0);
	m_waterFlow.zero();
	m_waterResistance = 0;
	m_waterDamping = 1.5f;
	m_gravity.Set(0,0,-9.8f);
	m_damping = 0;
	m_Emin = sqr(0.01f);
	m_iSimClass = 4;
	m_accuracy = 0.01f;
	m_nMaxIters = 1024;
	m_prevTimeInterval = 0;
	m_bAwake = 1;
	m_nSlowFrames = 0;
	m_friction = 0;
	m_impulseScale = 0.05f;
	m_explosionScale = 0.001f;
	m_qrot0.SetIdentity();
	m_collImpulseScale = 1.0f;
	m_bMeshUpdated = 0;
	m_collTypes = /*ent_terrain |*/ ent_static|ent_sleeping_rigid|ent_rigid|ent_living;
	m_maxCollImpulse = 3000;
}

CSoftEntity::~CSoftEntity()
{
	AlertNeighbourhoodND();	
}

void CSoftEntity::AlertNeighbourhoodND()
{
	if (m_vtx) { delete[] m_vtx; m_vtx=0; }
	if (m_edges) { delete[] m_edges; m_edges=0; }
	if (m_pVtxEdges) { delete[] m_pVtxEdges; m_pVtxEdges=0; }
	m_nVtx=m_nEdges = 0; 
}


int CSoftEntity::AddGeometry(phys_geometry *pgeom, pe_geomparams* params, int id)
{
	if (!pgeom || pgeom->pGeom->GetType()!=GEOM_TRIMESH || m_nParts>0)
		return -1;

	int res = CPhysicalEntity::AddGeometry(pgeom,params,id);
	int i,j,i0,i1,bDegen,iedge,itri,itri0,ivtx,itrinew,nVtxEdges,(*pInfo)[3];
	float rvtxmass,vtxvol,len[3];
	
	CTriMesh *pMesh = (CTriMesh*)pgeom->pGeom;
	rvtxmass = pMesh->m_nVertices/params->mass;
	vtxvol = 1.0f/(rvtxmass*params->density);
	m_density = params->density;

	m_vtx = new se_vertex[m_nVtx = pMesh->m_nVertices];
	for(i=0;i<m_nVtx;i++) {
		m_vtx[i].posorg = pMesh->m_pVertices[i];
		m_vtx[i].pos = m_qrot0*(params->q*pMesh->m_pVertices[i]*params->scale+params->pos);
		m_vtx[i].massinv = rvtxmass; m_vtx[i].volume = vtxvol;
		m_vtx[i].vel.zero(); m_vtx[i].bSeparating=m_vtx[i].iSorted=m_vtx[i].bAttached = 0;
		m_vtx[i].pContactEnt = 0;	m_vtx[i].iContactNode = 0; m_vtx[i].ncontact.zero(); m_vtx[i].n.zero();
	}
	m_offs0 = m_vtx[0].pos;
	for(i=1;i<m_nVtx;i++) m_vtx[i].pos -= m_vtx[0].pos;
	m_vtx[0].pos.zero();
	memset(pInfo=new int[pMesh->m_nTris][3],-1,pMesh->m_nTris*sizeof(pInfo[0]));

	// count the number of edges - for each tri, mark each edge, unless buddy already did it
	for(i=m_nEdges=0;i<pMesh->m_nTris;i++) {
		for(j=bDegen=0;j<3;j++) {
			len[j] = (pMesh->m_pVertices[pMesh->m_pIndices[i*3+j]]-pMesh->m_pVertices[pMesh->m_pIndices[i*3+inc_mod3[j]]]).len2();
			bDegen |= iszero(len[j]);
		}
		iedge = idxmax3(len); j = iedge&-bDegen;
		do {
			if (pInfo[i][j]<0 && !(m_flags&se_skip_longest_edges && j==iedge && !bDegen)) {
				if (pMesh->m_pTopology[i].ibuddy[j]>=0)
					pInfo[ pMesh->m_pTopology[i].ibuddy[j] ][ pMesh->GetEdgeByBuddy(pMesh->m_pTopology[i].ibuddy[j],i) ] = m_nEdges;
				pInfo[i][j] = m_nEdges++;
			}
		} while(++j<3 && !bDegen);
	}
	m_edges = new se_edge[m_nEdges];
	m_pVtxEdges = new int[m_nEdges*2];
	for(i=0;i<pMesh->m_nTris;i++) for(j=0;j<3;j++) if ((iedge=pInfo[i][j])>=0) {
		i0 = m_edges[iedge].ivtx[0] = pMesh->m_pIndices[i*3+j];
		i1 = m_edges[iedge].ivtx[1] = pMesh->m_pIndices[i*3+inc_mod3[j]];
		m_edges[iedge].len0 = (pMesh->m_pVertices[i0]-pMesh->m_pVertices[i1]).len()*params->scale;
		m_edges[pInfo[i][j]].kd = m_vtx[i0].massinv+m_vtx[i1].massinv>0 ? 
			sqrt_tpl(m_ks/(m_vtx[i0].massinv+m_vtx[i1].massinv))*2.0f*m_kdRatio : 0;
	}

	// for each vertex, trace ccw fan around it and store in m_pVtxEdges
	m_BBox[0].zero(); m_BBox[1].zero();
	for(i=nVtxEdges=0; i<pMesh->m_nTris; i++) 
	for(j=0;j<3;j++) if (!m_vtx[ivtx=pMesh->m_pIndices[i*3+j]].iSorted) {
		itri=i; iedge=j;
		m_vtx[ivtx].iStartEdge = nVtxEdges;	m_vtx[ivtx].bFullFan = 1;
		do { // first, trace cw fan until we find an open edge (if any)
			if ((itrinew = pMesh->m_pTopology[itri].ibuddy[iedge])<=0)
				break;
			iedge = inc_mod3[pMesh->GetEdgeByBuddy(itrinew,itri)];
		}	while((itri=itrinew)!=i);
		itri0 = itri;
		do { // now trace ccw fan
			if (pInfo[itri][iedge]>=0)
				m_pVtxEdges[nVtxEdges++] = pInfo[itri][iedge];
			if ((itrinew = pMesh->m_pTopology[itri].ibuddy[dec_mod3[iedge]])<0) {
				if (pInfo[itri][dec_mod3[iedge]]>=0)
					m_pVtxEdges[nVtxEdges++] = pInfo[itri][dec_mod3[iedge]];
				m_vtx[ivtx].bFullFan = 0; break;
			}
			iedge = pMesh->GetEdgeByBuddy(itrinew,itri);
		} while ((itri=itrinew)!=itri0);
		m_vtx[ivtx].iEndEdge = nVtxEdges-1;
		m_vtx[ivtx].rnEdges = 1.0f/(nVtxEdges-m_vtx[ivtx].iStartEdge);
		m_vtx[ivtx].iSorted = 1;
		m_vtx[ivtx].surface_idx[0] = pMesh->m_pIds ? pMesh->m_pIds[i]:-1;

		m_BBox[0].x=min(m_BBox[0].x,m_vtx[ivtx].pos.x); m_BBox[1].x=max(m_BBox[1].x,m_vtx[ivtx].pos.x); 
		m_BBox[0].y=min(m_BBox[0].y,m_vtx[ivtx].pos.y); m_BBox[1].y=max(m_BBox[1].y,m_vtx[ivtx].pos.y); 
		m_BBox[0].z=min(m_BBox[0].z,m_vtx[ivtx].pos.z); m_BBox[1].z=max(m_BBox[1].z,m_vtx[ivtx].pos.z); 
	}
	delete[] pInfo;
	m_coverage = m_flags&se_skip_longest_edges ? 0.5f: 1.0f/3;
	m_BBox[0] += m_pos+m_offs0; m_BBox[1] += m_pos+m_offs0;

	box bbox;
	bbox.Basis.SetIdentity();
	bbox.bOriented = 0;
	bbox.center = (m_BBox[0]+m_BBox[1])*0.5f-m_pos-m_parts[0].pos;
	bbox.size = (m_BBox[1]-m_BBox[0])*(0.5f/params->scale);
	if (pMesh->m_pTree)
		delete pMesh->m_pTree;
	CSingleBoxTree *pTree = new CSingleBoxTree;
	pTree->SetBox(&bbox);
	pTree->Build(pMesh);
	pTree->m_nPrims = pMesh->m_nTris;
	pMesh->m_pTree = pTree;
	m_flags |= pef_use_geom_callbacks;
	m_bMeshUpdated = 0;

	return res;
}


void CSoftEntity::RemoveGeometry(int id)
{
	int i;
	for(i=0;i<m_nParts && m_parts[i].id!=id;i++);
	if (i==m_nParts) return;

	if (m_vtx) { delete[] m_vtx; m_vtx=0; }
	if (m_edges) { delete[] m_edges; m_edges=0; }
	if (m_pVtxEdges) { delete[] m_pVtxEdges; m_pVtxEdges=0; }
	m_nVtx=m_nEdges = 0; 

	CPhysicalEntity::RemoveGeometry(id);
}


int CSoftEntity::SetParams(pe_params *_params)
{
	int res = CPhysicalEntity::SetParams(_params);
	if (res) {
		if (_params->type==pe_params_pos::type_id && !is_unused(((pe_params_pos*)_params)->q)) {
			if (m_nVtx>0 && (m_qrot0|m_qrot)<0.998f) {
				CTriMesh *pMesh = (CTriMesh*)m_parts[0].pPhysGeom->pGeom;
				int i; quaternionf qrot = m_bMeshUpdated ? m_qrot*!m_qrot0 : m_qrot;
				for(i=0;i<m_nVtx;i++) {
					m_vtx[i].pos = qrot*(m_parts[0].q*pMesh->m_pVertices[i]*m_parts[0].scale+m_parts[0].pos);
					if (m_vtx[i].bAttached==2 && m_vtx[i].pContactEnt) {
						RigidBody *pbody = m_vtx[i].pContactEnt->GetRigidBody(m_vtx[i].iContactPart);
						m_vtx[i].ptAttach = (m_vtx[i].pos+m_pos-pbody->pos)*pbody->q;
					}
				}
				m_offs0 = m_vtx[0].pos;
				for(i=1;i<m_nVtx;i++) m_vtx[i].pos -= m_vtx[0].pos;
				m_vtx[0].pos.zero();
			}
			m_qrot0 = m_qrot; m_qrot.SetIdentity();
		}
		return res;
	}

	if (_params->type==pe_simulation_params::type_id) {
		pe_simulation_params *params = (pe_simulation_params*)_params;
		if (!is_unused(params->gravity)) m_gravity = params->gravity;
		if (!is_unused(params->maxTimeStep)) m_maxAllowedStep = params->maxTimeStep;
		if (!is_unused(params->minEnergy)) m_Emin = params->minEnergy;
		if (!is_unused(params->damping)) m_damping = params->damping;
		if (!is_unused(params->density) && params->density>=0 && m_nParts>0) {
			for(int i=0;i<m_nVtx;i++) if (m_vtx[i].massinv>0)
				m_vtx[i].volume = 1.0f/(m_vtx[i].massinv*params->density);
			m_density = params->density;
		}
		if (!is_unused(params->mass) && params->mass>=0 && m_nParts>0) {
			float rvtxmass = m_nVtx/params->mass;
			for(int i=0;i<m_nVtx;i++) if (m_vtx[i].massinv>0)
				m_vtx[i].massinv = rvtxmass;
			m_parts[0].mass = params->mass;
		}
		if (!is_unused(params->iSimClass))	{
			m_iSimClass = params->iSimClass;
			m_pWorld->RepositionEntity(this,2);
		}
		return 1;
	}

	if (_params->type==pe_params_buoyancy::type_id) {
		pe_params_buoyancy *params = (pe_params_buoyancy*)_params;
		if (!is_unused(params->waterDensity)) { 
			if (m_waterDensity!=params->waterDensity)
				m_bAwake = 1;
			m_waterDensity = params->waterDensity;
		}
		if (!is_unused(params->waterDamping)) m_waterDamping = params->waterDamping;
		if (!is_unused(params->waterPlane.n)) {
			if ((m_waterPlane.n-params->waterPlane.n).len2()>0)
				m_bAwake = 1;
			m_waterPlane.n = params->waterPlane.n;
		}
		if (!is_unused(params->waterPlane.origin)) {
			if ((m_waterPlane.origin-params->waterPlane.origin).len2()>0)
				m_bAwake = 1;
			m_waterPlane.origin = params->waterPlane.origin;
		}
		if (!is_unused(params->waterFlow)) {
			if ((m_waterFlow-params->waterFlow).len2()>0)
				m_bAwake = 1;
			m_waterFlow = params->waterFlow;
		}
		if (!is_unused(params->waterResistance)) m_waterResistance = params->waterResistance;
		return 1;
	}

	if (_params->type==pe_params_softbody::type_id) {
		pe_params_softbody *params = (pe_params_softbody*)_params;
		if (!is_unused(params->thickness)) m_thickness = params->thickness;
		if (!is_unused(params->friction)) m_friction = params->friction;
		if (!is_unused(params->ks)) m_ks = params->ks;
		if (!is_unused(params->kdRatio)) m_kdRatio = params->kdRatio;
		if (!is_unused(params->ks) || !is_unused(params->kdRatio)) {
			for(int i=0;i<m_nEdges;i++) if (m_vtx[m_edges[i].ivtx[0]].massinv+m_vtx[m_edges[i].ivtx[1]].massinv>0)
				m_edges[i].kd = sqrt_tpl(m_ks/(m_vtx[m_edges[i].ivtx[0]].massinv+m_vtx[m_edges[i].ivtx[1]].massinv))*2.0f*m_kdRatio;
		}
		if (!is_unused(params->airResistance)) m_airResistance = params->airResistance;
		if (!is_unused(params->wind)) m_wind = params->wind;
		if (!is_unused(params->accuracy)) m_accuracy = params->accuracy;
		if (!is_unused(params->nMaxIters)) m_nMaxIters = params->nMaxIters;
		if (!is_unused(params->maxSafeStep)) m_maxSafeStep = params->maxSafeStep;
		if (!is_unused(params->impulseScale)) m_impulseScale = params->impulseScale;
		if (!is_unused(params->explosionScale)) m_explosionScale = params->explosionScale;
		if (!is_unused(params->collisionImpulseScale)) m_collImpulseScale = params->collisionImpulseScale;
		if (!is_unused(params->maxCollisionImpulse)) m_maxCollImpulse = params->maxCollisionImpulse;
		if (!is_unused(params->collTypes)) m_collTypes = params->collTypes;
		return 1;
	}

	return CPhysicalEntity::SetParams(_params);
}


int CSoftEntity::GetParams(pe_params *_params)
{
	if (_params->type==pe_simulation_params::type_id) {
		pe_simulation_params *params = (pe_simulation_params*)_params;
		params->gravity=params->gravityFreefall = m_gravity;
		params->maxTimeStep = m_maxAllowedStep;
		params->minEnergy = m_Emin;
		params->damping=params->dampingFreefall = m_damping;
		params->mass = m_parts[0].mass;
		params->density = m_density;
		params->iSimClass = m_iSimClass;
		return 1;
	}

	if (_params->type==pe_params_buoyancy::type_id) {
		pe_params_buoyancy *params = (pe_params_buoyancy*)_params;
		params->waterDensity = m_waterDensity;
		params->waterDamping = m_waterDamping;
		params->waterPlane = m_waterPlane;
		params->waterFlow = m_waterFlow;
		params->waterResistance = m_waterResistance;
		params->waterEmin = m_Emin;
		return 1;
	}

	if (_params->type==pe_params_softbody::type_id) {
		pe_params_softbody *params = (pe_params_softbody*)_params;
		params->thickness = m_thickness;
		params->friction = m_friction;
		params->ks = m_ks;
		params->kdRatio = m_kdRatio;
		params->airResistance = m_airResistance;
		params->wind = m_wind;
		params->accuracy = m_accuracy;
		params->nMaxIters = m_nMaxIters;
		params->maxSafeStep = m_maxSafeStep;
		params->impulseScale = m_impulseScale;
		params->explosionScale = m_explosionScale;
		params->collisionImpulseScale = m_collImpulseScale;
		params->maxCollisionImpulse = m_maxCollImpulse;
		params->collTypes = m_collTypes;
		return 1;
	}

	return CPhysicalEntity::GetParams(_params);
}


int CSoftEntity::GetStatus(pe_status *_status)
{
	int res;
	if (res = CPhysicalEntity::GetStatus(_status)) {
		if (_status->type==pe_status_caps::type_id) {
			pe_status_caps *status = (pe_status_caps*)_status;
			status->bCanAlterOrientation = 1;
		}
		return res;
	}

	if (_status->type==pe_status_softvtx::type_id) {
		if (m_nVtx<=0)
			return 0;
		pe_status_softvtx *status = (pe_status_softvtx*)_status;
		status->nVtx = m_nVtx;
		status->pVtx = ((CTriMesh*)m_parts[0].pPhysGeomProxy->pGeom)->m_pVertices;
		status->pNormals.data = &m_vtx[0].n;
		status->pNormals.iStride = sizeof(m_vtx[0]);
		return 1;
	}

	if (_status->type==pe_status_collisions::type_id) {
		pe_status_collisions *status = (pe_status_collisions*)_status;
		int i,j,n;
		if (status->len<=0)
			return 0;

		for(i=n=0;i<m_nVtx;i++) if (!m_vtx[i].bAttached && m_vtx[i].pContactEnt) {
			if (n==status->len) {
				for(n=1,j=0; n<status->len; n++) 
					if ((status->pHistory[n].v[0]-status->pHistory[n].v[1]).len2() < (status->pHistory[j].v[0]-status->pHistory[j].v[1]).len2())
						j = n;
				if ((status->pHistory[j].v[0]-status->pHistory[j].v[1]).len2() > (m_vtx[i].vel-m_vtx[i].vcontact).len2())
					continue;
			} else j=n++;
			status->pHistory[j].pt = m_vtx[i].pos;
			status->pHistory[j].n = m_vtx[i].ncontact;
			status->pHistory[j].v[0] = m_vtx[i].vel;
			status->pHistory[j].v[1] = m_vtx[i].vcontact;
			status->pHistory[j].mass[0] = m_parts[0].mass;
			status->pHistory[j].mass[1] = m_vtx[i].pContactEnt->GetMassInv();
			if (status->pHistory[j].mass[1]>0)
				status->pHistory[j].mass[1] = 1.0f/status->pHistory[j].mass[1];
			status->pHistory[j].age = 0;
			status->pHistory[j].idCollider = m_pWorld->GetPhysicalEntityId(m_vtx[i].pContactEnt);
			status->pHistory[j].partid[0] = 0;
			status->pHistory[j].partid[1] = m_vtx[i].iContactPart;
			status->pHistory[j].idmat[0] = m_vtx[i].surface_idx[0];
			status->pHistory[j].idmat[1] = m_vtx[i].surface_idx[1];
		}

		return status->len = n;
	}

	return 0;
}


int CSoftEntity::Action(pe_action *_action)
{
	if (_action->type==pe_action_impulse::type_id) {
		pe_action_impulse *action = (pe_action_impulse*)_action;
		ENTITY_VALIDATE("CSoftEntity:Action(action_impulse)",action);

		if (m_nVtx>0 && !is_unused(action->point) && !is_unused(action->impulse)) {
			CTriMesh *pMesh = (CTriMesh*)m_parts[0].pPhysGeom->pGeom;
			vectorf pt=action->point-(m_pos+m_offs0), impulse=action->impulse*m_impulseScale;
			int i,j,bBest;
			if (!is_unused(action->ipart)) i = action->ipart;
			else i = action->partid;
			m_bAwake = 1;

			if ((unsigned int)i < (unsigned int)pMesh->m_nTris) {
				float rarea,k; int idx[3];
				for(j=0;j<3;j++) idx[j] = pMesh->m_pIndices[i*3+j];
				rarea = (m_vtx[idx[1]].pos-m_vtx[idx[0]].pos ^ m_vtx[idx[2]].pos-m_vtx[idx[0]].pos).len();
				if (rarea>1E-4f) {
					rarea = 1.0f/rarea;
					for(j=0;j<3;j++) {
						k = (m_vtx[idx[inc_mod3[j]]].pos-pt ^ m_vtx[idx[j]].pos-pt).len()*rarea;
						m_vtx[idx[dec_mod3[j]]].vel += impulse*(m_vtx[idx[dec_mod3[j]]].massinv*k);
					}
				} else
					m_vtx[idx[0]].vel += impulse*m_vtx[idx[0]].massinv;
			} else {
				for(i=1,j=0;i<m_nVtx;i++) {
					bBest = -isneg((m_vtx[i].pos-pt).len2()-(m_vtx[j].pos-pt).len2());
					j = i&bBest | j&~bBest;
				}
				m_vtx[j].vel += impulse*m_vtx[j].massinv;
			}
		}
		return 1;
	}

	if (_action->type==pe_action_attach_points::type_id) {
		pe_action_attach_points *action = (pe_action_attach_points*)_action;
		CPhysicalEntity* pent = action->pEntity==WORLD_ENTITY ? &g_StaticPhysicalEntity : 
			action->pEntity ? ((CPhysicalPlaceholder*)action->pEntity)->GetEntity() : 0;
		int ipart=0, bAttached=iszero((intptr_t)pent)^1;
		if (bAttached && is_unused(action->points))
			bAttached = 2;
		float rvtxmass = pent ? 0 : m_nVtx/m_parts[0].mass;

		if (!is_unused(action->partid)) {
			for(ipart=0; ipart<pent->m_nParts && pent->m_parts[ipart].id!=action->partid; ipart++);
			if (ipart>=pent->m_nParts)
				return 0;
		}
		RigidBody *pbody;
		if (bAttached)
			pbody = pent->GetRigidBody(ipart);

		for(int i=0;i<action->nPoints;i++) {
			if (m_vtx[action->piVtx[i]].pContactEnt)	m_vtx[action->piVtx[i]].pContactEnt->Release();
			if (m_vtx[action->piVtx[i]].pContactEnt = pent)
				pent->AddRef();
			m_vtx[action->piVtx[i]].massinv = rvtxmass;
			m_vtx[action->piVtx[i]].iContactPart = ipart;
			if (m_vtx[action->piVtx[i]].bAttached = bAttached) {
				if (!is_unused(action->points))
					m_vtx[action->piVtx[i]].ptAttach = action->points[i];
				else
					m_vtx[action->piVtx[i]].ptAttach = m_vtx[action->piVtx[i]].pos+m_pos+m_offs0;
				m_vtx[action->piVtx[i]].ptAttach = (m_vtx[action->piVtx[i]].ptAttach-pbody->pos)*pbody->q;
			}
		}

		return 1;
	}

	if (_action->type==pe_action_reset::type_id) {
		if (m_nVtx>0) {
			CTriMesh *pMesh = (CTriMesh*)m_parts[0].pPhysGeomProxy->pGeom;
			int i;
			for(i=0;i<m_nVtx;i++) {
				if (m_vtx[i].pContactEnt)	m_vtx[i].pContactEnt->Release();
				m_vtx[i].pContactEnt = 0;
				m_vtx[i].vel.zero();
				pMesh->m_pVertices[i] = m_vtx[i].posorg;
			}
			for(i=0;i<pMesh->m_nTris;i++)
				pMesh->m_pNormals[i] = (pMesh->m_pVertices[pMesh->m_pIndices[i*3+1]]-pMesh->m_pVertices[pMesh->m_pIndices[i*3]] ^
																pMesh->m_pVertices[pMesh->m_pIndices[i*3+2]]-pMesh->m_pVertices[pMesh->m_pIndices[i*3]]).normalized();
		}
		return 1;
	}

	return CPhysicalEntity::Action(_action);
}


void CSoftEntity::StartStep(float time_interval)
{
	m_timeStepPerformed = 0;
	m_timeStepFull = time_interval;
}


float CSoftEntity::GetMaxTimeStep(float time_interval)
{
	if (m_timeStepPerformed > m_timeStepFull-0.001f)
		return time_interval;
	return min(min(m_timeStepFull-m_timeStepPerformed,m_maxAllowedStep),time_interval);
}


inline int GetCheckPart(CPhysicalEntity *pent, int ipart)
{
	int i = (pent->m_bProcessed_aux&0xFFFFFF) & (1<<ipart)-1;
	return (pent->m_bProcessed_aux>>24) + g_bitcount[i&0xFF] + g_bitcount[i>>8&0xFF] + g_bitcount[i>>16];
}


int CSoftEntity::Step(float time_interval)
{
	if (m_nVtx<=0 || m_timeStepPerformed>m_timeStepFull-0.001f)
		return 1;

	int i,j,j1,nEnts,i0,i1,nContactVtx,nContactVtx0,nCheckParts,iter,imask,bUnstable,iFullIter=4;
	vectorf d,l,llTd,v,F,w,center,BBox0[2];
	float rl,l0,rmax,windage,kr,rsep,friction=max(0.5f,1.0f-m_friction*time_interval),ks=m_ks,kdScale=1.0f;
	real r2,r2new,a,b,dAd;
	struct check_part {
		vectorf offset;
		matrix3x3f R;
		box bbox;
		CPhysicalEntity *pent;
		int ipart;
		RigidBody *pbody;
		CGeometry *pGeom;
		int bPrimitive;
		int surface_idx;
		vectorf P,L;
	};
	box boxent;
	check_part checkParts[20];
	CRayGeom aray;
	intersection_params ip;
	geom_contact *pcontacts;
	CPhysicalEntity **pentlist,*pent;
	geom_world_data gwd;
	RigidBody *pbody;

	g_Overlapper.Init();
	boxent.size = (m_BBox[1]-m_BBox[0])*0.5f+vectorf(m_thickness,m_thickness,m_thickness)*2;
	center = (m_BBox[0]+m_BBox[1])*0.5f;
	boxent.bOriented = 1;
	ip.bStopAtFirstTri = true;

	for(i=0;i<m_nVtx;i++) if (m_vtx[i].pContactEnt && m_vtx[i].pContactEnt->m_iSimClass==7)	{
		m_vtx[i].pContactEnt = 0; m_vtx[i].bAttached = 0;
	}
	if (!m_bAwake)
		return 1;

	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );
	PHYS_ENTITY_PROFILER

	nEnts = m_pWorld->GetEntitiesAround(m_BBox[0]-vectorf(m_thickness,m_thickness,m_thickness)*2,
		m_BBox[1]+vectorf(m_thickness,m_thickness,m_thickness)*2, pentlist, 
			m_collTypes|ent_sort_by_mass|ent_ignore_noncolliding|ent_triggers, this);

	for(i=j=nCheckParts=0;i<nEnts;i++) if (pentlist[i]!=this)
	for(j=0,pentlist[i]->m_bProcessed_aux=nCheckParts<<24; j<pentlist[i]->m_nParts; j++) {
		pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom->GetBBox(&checkParts[nCheckParts].bbox);
		checkParts[nCheckParts].bbox.center *= pentlist[i]->m_parts[j].scale;
		checkParts[nCheckParts].bbox.size *= pentlist[i]->m_parts[j].scale;
		//(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q).getmatrix(boxrope.Basis);	//Q2M_IVO
		boxent.Basis = matrix3x3f(pentlist[i]->m_qrot*pentlist[i]->m_parts[j].q);
		checkParts[nCheckParts].offset = pentlist[i]->m_pos+pentlist[i]->m_qrot*pentlist[i]->m_parts[j].pos;
		boxent.center = (center-checkParts[nCheckParts].offset)*boxent.Basis;
		boxent.bOriented++;
		if (box_box_overlap_check(&boxent,&checkParts[nCheckParts].bbox)) {
			checkParts[nCheckParts].pent = pentlist[i];
			checkParts[nCheckParts].ipart = j;
			checkParts[nCheckParts].R = boxent.Basis;
			pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom->PrepareForRayTest(
				pentlist[i]->m_parts[j].scale==1.0f ? m_thickness*2 : m_thickness*2/pentlist[i]->m_parts[j].scale);
			checkParts[nCheckParts].offset -= m_pos+m_offs0;
			checkParts[nCheckParts].pbody = pentlist[i]->GetRigidBody(j);
			checkParts[nCheckParts].pGeom = (CGeometry*)pentlist[i]->m_parts[j].pPhysGeomProxy->pGeom;
			checkParts[nCheckParts].bPrimitive = isneg(checkParts[nCheckParts].pGeom->GetPrimitiveCount()-2);
			checkParts[nCheckParts].surface_idx = pentlist[i]->m_parts[j].surface_idx;
			pentlist[i]->m_bProcessed_aux |= 1u<<j;
			pentlist[i]->m_bProcessed = 1;
			if (++nCheckParts==sizeof(checkParts)/sizeof(checkParts[0]))
				goto enoughgeoms;
		}
	} enoughgeoms:

	for(i=nContactVtx=0; i<m_nVtx; i++) if (!m_vtx[i].bAttached) { // detect collisions for free vertices
		// calculate normal
		for(j=m_vtx[i].iStartEdge,m_vtx[i].n.zero(); j<m_vtx[i].iEndEdge+m_vtx[i].bFullFan; j++) {
			imask = j-m_vtx[i].iEndEdge>>31; j1 = j+1&imask | m_vtx[i].iStartEdge&~imask;
			m_vtx[i].n += 
				(m_vtx[m_edges[m_pVtxEdges[j]].ivtx[1]].pos-m_vtx[m_edges[m_pVtxEdges[j]].ivtx[0]].pos)*(iszero(i^m_edges[m_pVtxEdges[j]].ivtx[0])*2-1) ^
				(m_vtx[m_edges[m_pVtxEdges[j1]].ivtx[1]].pos-m_vtx[m_edges[m_pVtxEdges[j1]].ivtx[0]].pos)*(iszero(i^m_edges[m_pVtxEdges[j1]].ivtx[0])*2-1);
		}
		m_vtx[i].area = m_vtx[i].n.len();
		m_vtx[i].n /= m_vtx[i].area; m_vtx[i].area *= m_coverage*0.5f;
		m_vtx[i].pos0 = m_vtx[i].pos;

		rsep = m_thickness;
		if (pent = m_vtx[i].pContactEnt) {
			pbody = m_vtx[i].pContactEnt->GetRigidBody(m_vtx[i].iContactPart);
			m_vtx[i].vcontact = pbody->v+(pbody->w^m_vtx[i].pos+m_pos+m_offs0-pbody->pos);
			if (pent->m_bProcessed && (!m_vtx[i].bSeparating || (m_vtx[i].vel-m_vtx[i].vcontact)*m_vtx[i].ncontact<0.1f)) {
				if (checkParts[GetCheckPart(m_vtx[i].pContactEnt,m_vtx[i].iContactPart)].bPrimitive) {
					rsep = m_thickness*1.5f; m_vtx[i].pContactEnt = 0;
				}	else {
					j = m_vtx[i].iContactPart;
					gwd.offset = pent->m_pos-m_pos-m_offs0+pent->m_qrot*pent->m_parts[j].pos;
					gwd.R = matrix3x3f(pent->m_qrot*pent->m_parts[j].q);
					gwd.scale = pent->m_parts[j].scale;
					aray.m_dirn = -m_vtx[i].ncontact;
					aray.m_ray.origin = m_vtx[i].pos;
					aray.m_ray.dir = aray.m_dirn*(m_thickness*1.5f);
					gwd.iStartNode = m_vtx[i].iContactNode;
					if (pent->m_parts[j].pPhysGeomProxy->pGeom->Intersect(&aray,&gwd,0,&ip,pcontacts)) {
						m_vtx[i].pos = pcontacts->pt+pcontacts->n*m_thickness;
						m_vtx[i].ncontact = pcontacts->n;
						m_vtx[i].iContactNode = pcontacts->iNode[0];
					} else
						m_vtx[i].pContactEnt = 0;
				}
			}	else
				m_vtx[i].pContactEnt = 0;
		}

		for(j=0; j<nCheckParts; j++) {
			v = checkParts[j].pbody->v+(checkParts[j].pbody->w^m_vtx[i].pos+m_pos+m_offs0-checkParts[j].pbody->pos);
			if (checkParts[j].bPrimitive) {
				contact acontact;
				if (checkParts[j].pGeom->UnprojectSphere((m_vtx[i].pos-checkParts[j].offset)*checkParts[j].R, m_thickness,rsep, &acontact)) {
					m_vtx[i].pos = checkParts[j].R*(acontact.pt+acontact.n*m_thickness)+checkParts[j].offset;
					m_vtx[i].ncontact = checkParts[j].R*acontact.n;
					m_vtx[i].pContactEnt = checkParts[j].pent;
					m_vtx[i].iContactPart = checkParts[j].ipart;
					m_vtx[i].vcontact = v;					
					m_vtx[i].surface_idx[1] = checkParts[j].surface_idx;
				}
			} else {
				aray.m_ray.dir = ((v-m_vtx[i].vel)*checkParts[j].R)*m_prevTimeInterval;
				aray.m_dirn = aray.m_ray.dir.normalized();
				aray.m_ray.origin = (m_vtx[i].pos-checkParts[j].offset)*checkParts[j].R - aray.m_dirn*m_thickness;
				aray.m_ray.dir += aray.m_dirn*m_thickness;
				if (box_ray_overlap_check(&checkParts[j].bbox,&aray.m_ray)) {
					gwd.scale = checkParts[j].pent->m_parts[checkParts[j].ipart].scale;
					gwd.offset.zero(); gwd.R.SetIdentity();
					if (checkParts[j].pent->m_parts[checkParts[j].ipart].pPhysGeomProxy->pGeom->Intersect(&aray,&gwd,0,&ip,pcontacts) &&
							pcontacts->n*aray.m_dirn>0) 
					{
						m_vtx[i].pos = checkParts[j].R*(pcontacts->pt+aray.m_dirn*m_thickness)+checkParts[j].offset;
						m_vtx[i].ncontact = checkParts[j].R*pcontacts->n;
						m_vtx[i].pContactEnt = checkParts[j].pent;
						m_vtx[i].iContactPart = checkParts[j].ipart;
						m_vtx[i].iContactNode = pcontacts->iNode[0];
						m_vtx[i].vcontact = v;
						imask = pcontacts->id[0]>>31;
						m_vtx[i].surface_idx[1] = checkParts[j].surface_idx&imask | pcontacts->id[0]&~imask;
					}
				}
			}
		}

		if (pent!=m_vtx[i].pContactEnt) {
			if (pent) pent->Release();
			if (m_vtx[i].pContactEnt) m_vtx[i].pContactEnt->AddRef();
		} 
		if (m_vtx[i].pContactEnt) {
			m_vtx[nContactVtx++].iSorted = i;
			if ((m_vtx[i].pos0-m_vtx[i].pos).len2()>sqr(m_maxSafeStep*0.75f))
				m_vtx[i].pos = m_vtx[i].pos0+(m_vtx[i].pos-m_vtx[i].pos0).normalized()*(m_maxSafeStep*0.75f);
		}
	} else { // synchronize attached vertices w/ hosts
		pbody = m_vtx[i].pContactEnt->GetRigidBody(m_vtx[i].iContactPart);
		m_vtx[i].pos = pbody->pos+pbody->q*m_vtx[i].ptAttach;
		m_vtx[i].vcontact = m_vtx[i].vel = pbody->v+(pbody->w^m_vtx[i].pos-pbody->pos);
		m_vtx[i].pos -= m_pos+m_offs0;
	}
	for(i=0; i<nCheckParts; i++)
		checkParts[i].pent->m_bProcessed = 0;

	for(i=0; i<m_nEdges; i++) {	// calculate edge lengths
		l = m_vtx[m_edges[i].ivtx[0]].pos - m_vtx[m_edges[i].ivtx[1]].pos;
		m_edges[i].rlen = 1.0f/max(1E-4f,m_edges[i].len = sqrt_tpl(l.len2()));
	}

	for(i=0; i<m_nVtx; i++) {	// apply gravity, buoyancy, wind (or water resistance)
		if (!m_vtx[i].bAttached) {
			if ((m_vtx[i].pos+m_pos-m_waterPlane.origin)*m_waterPlane.n<0) {
				m_vtx[i].vel -= m_gravity*(m_waterDensity*m_vtx[i].volume*time_interval);
				kr = m_waterResistance;	w = m_waterFlow;
			}	else {
				kr = m_airResistance;	w = m_wind;
			}
			for(j=m_vtx[i].iStartEdge,windage=0; j<=m_vtx[i].iEndEdge; j++)
				windage += ((m_vtx[m_edges[m_pVtxEdges[j]].ivtx[1]].pos-m_vtx[m_edges[m_pVtxEdges[j]].ivtx[0]].pos)*m_vtx[i].n)*
					(iszero(i^m_edges[m_pVtxEdges[j]].ivtx[0])*2-1)*m_edges[m_pVtxEdges[j]].rlen;
			m_vtx[i].vel += m_vtx[i].n*((m_vtx[i].n*(w-m_vtx[i].vel))*m_vtx[i].area*(windage*m_vtx[i].rnEdges+1)*kr*time_interval);
			m_vtx[i].vel += m_gravity*time_interval;
		}
		if (m_vtx[i].pContactEnt)
			m_vtx[i].vel -= m_vtx[i].ncontact*(m_vtx[i].ncontact*(m_vtx[i].vel-m_vtx[i].vcontact));
		m_vtx[i].r.zero(); m_vtx[i].bSeparating = 0;
		m_vtx[i].pos0=m_vtx[i].pos; m_vtx[i].vel0=m_vtx[i].vel; m_vtx[i].iSorted0=m_vtx[i].iSorted;
	}
	nContactVtx0 = nContactVtx;
	BBox0[0] = m_BBox[0]; BBox0[1] = m_BBox[1];

	reiterate:
	for(i=0;i<nCheckParts;i++) checkParts[i].P=checkParts[i].L.zero();

	for(i=0; i<m_nEdges; i++) { // calculate residual for the solver
		i0 = m_edges[i].ivtx[0]; i1 = m_edges[i].ivtx[1];
		l0 = m_edges[i].len0; rl = m_edges[i].rlen;	
		l = m_vtx[i0].pos - m_vtx[i1].pos;
		v = m_vtx[i0].vel - m_vtx[i1].vel;
		d = v*time_interval;//+m_vtx[i0].n*m_vtx[i0].shift-m_vtx[i1].n*m_vtx[i1].shift;
		llTd = l*((l*d)*sqr(rl));
		F  = l*(rl*(l0-m_edges[i].len)*ks); // Felastic
		F -= l*((v*l)*sqr(rl)*m_edges[i].kd*kdScale);	// Fviscous
		F += (d*(l0*rl-1)-llTd*(l0*rl))*ks; // dFelastic/dx * shift
		F -= ((l*v)*(d-llTd)+l*(v*(d-llTd)))*(m_edges[i].kd*kdScale*sqr(rl)); // dFviscous/dx * shift
		F *= time_interval;
		m_vtx[i0].r += F*m_vtx[i0].massinv;
		m_vtx[i1].r -= F*m_vtx[i1].massinv;
	}
	for(i=j=0; i<nContactVtx; i++) { // remove vertices that have 'separating' residuals from the contacting vertices list
		i1 = m_vtx[i].iSorted;
		float rn = m_vtx[i1].ncontact*m_vtx[i1].r;
		if (rn>0)
			m_vtx[i1].bSeparating = 1;
		else {
			j1 = GetCheckPart(m_vtx[i1].pContactEnt,m_vtx[i1].iContactPart);
			checkParts[j1].P += m_vtx[i1].ncontact*rn;
			checkParts[j1].L += m_vtx[i1].pos+m_pos-m_offs0-checkParts[j1].pbody->pos ^ m_vtx[i1].ncontact*rn;
			/*imask = m_vtx[i].surface_idx[0]>>31;
			float friction = max(0.0f, (m_pWorld->m_DynFrictionTable[(m_parts[0].surface_idx&imask|m_vtx[i1].surface_idx[0]&~imask)&NSURFACETYPES-1] + 
				m_pWorld->m_DynFrictionTable[m_vtx[i1].surface_idx[1]&NSURFACETYPES-1])*0.5f);
			if (rn*friction<0) {
				if ((m_vtx[i1].vel-m_vtx[i1].vcontact).len2()>sqr(rn*friction))
					m_vtx[i1].vel += (m_vtx[i1].vel-m_vtx[i1].vcontact).normalized()*(rn*friction);
				else
					m_vtx[i1].vel = m_vtx[i1].vcontact;
			}*/
			m_vtx[i1].vel = m_vtx[i1].vel*friction+m_vtx[i1].vcontact*(1.0f-friction);
			m_vtx[i1].r -= m_vtx[i1].ncontact*rn;
			m_vtx[j++].iSorted = i1;
		}
	}
	nContactVtx = j;
	for(i=0,r2=0; i<m_nVtx; i++) {
		r2+=m_vtx[i].r.len2(); m_vtx[i].d=m_vtx[i].r; m_vtx[i].P.zero();
	}
	iter = min(m_nMaxIters,m_nVtx); 

	do { // conjugate gradient solver for implicit Euler step
		for(i=0; i<m_nVtx; i++)	
			m_vtx[i].dv	= m_vtx[i].d*m_vtx[i].massinv;
		for(i=0; i<m_nEdges; i++) {
			i0 = m_edges[i].ivtx[0]; i1 = m_edges[i].ivtx[1];
			d = m_vtx[i0].d*m_vtx[i0].massinv-m_vtx[i1].d*m_vtx[i1].massinv; 
			l0 = m_edges[i].len0; rl = m_edges[i].rlen;	
			l = m_vtx[i0].pos - m_vtx[i1].pos;
			v = m_vtx[i0].vel - m_vtx[i1].vel;
			llTd = l*((l*d)*sqr(rl));
			F = (d*(l0*rl-1)-llTd*(l0*rl))*ks; // dFelastic/dx
			F -= ((l*v)*(d-llTd)+l*(v*(d-llTd)))*(m_edges[i].kd*kdScale*sqr(rl)); // dFviscous/dx - probably not worth the calculation amount
			F *= sqr(time_interval);
			F += llTd*(-m_edges[i].kd*kdScale*time_interval); // dFviscous/dv
			m_vtx[i0].dv -= F*m_vtx[i0].massinv;
			m_vtx[i1].dv += F*m_vtx[i1].massinv;
		}
		for(i=0; i<nContactVtx; i++) // filter away constrained components of contacting vertices
			m_vtx[m_vtx[i].iSorted].dv -= m_vtx[m_vtx[i].iSorted].ncontact*(m_vtx[m_vtx[i].iSorted].ncontact*m_vtx[m_vtx[i].iSorted].dv);
		for(i=0,dAd=0; i<m_nVtx; i++)
			dAd += m_vtx[i].d*m_vtx[i].dv;

		if (dAd<m_accuracy*0.01f)
			break;
		a = min((real)100.0,r2/dAd);	
		for(i=0,r2new=rmax=0; i<m_nVtx; i++) {
			r2new += (m_vtx[i].r -= m_vtx[i].dv*a).len2();
			m_vtx[i].P += m_vtx[i].d*a;
			rmax = max(rmax,m_vtx[i].r.len2());
		}
		b = r2new/r2; r2 = r2new;
		for(i=0;i<m_nVtx;i++)
			(m_vtx[i].d*=b) += m_vtx[i].r;
	} while(--iter && rmax>sqr(m_accuracy));

	m_vtx[0].pos += (m_vtx[0].vel+=m_vtx[0].P*m_vtx[0].massinv)*time_interval;
	bUnstable = isneg(sqr(m_maxSafeStep)-m_vtx[0].pos.len2());
	kr = 1.0f-m_damping*time_interval;
	m_BBox[0].zero(); m_BBox[1].zero();	rmax=0;
	for(i=1; i<m_nVtx; i++) {
		m_vtx[i].pos += (m_vtx[i].vel+=m_vtx[i].P*m_vtx[i].massinv)*time_interval - m_vtx[0].pos;
		bUnstable += isneg(sqr(m_maxSafeStep)-(m_vtx[i].pos-m_vtx[i].pos0).len2());
		rmax = max(rmax,m_vtx[i].vel.len2());
		m_vtx[i].vel *= kr;
		m_BBox[0].x=min(m_BBox[0].x,m_vtx[i].pos.x); m_BBox[1].x=max(m_BBox[1].x,m_vtx[i].pos.x); 
		m_BBox[0].y=min(m_BBox[0].y,m_vtx[i].pos.y); m_BBox[1].y=max(m_BBox[1].y,m_vtx[i].pos.y); 
		m_BBox[0].z=min(m_BBox[0].z,m_vtx[i].pos.z); m_BBox[1].z=max(m_BBox[1].z,m_vtx[i].pos.z); 
	}

	if (bUnstable) {
		for(i=0;i<m_nVtx;i++) {
			m_vtx[i].pos=m_vtx[i].pos0; m_vtx[i].vel=m_vtx[i].vel0; m_vtx[i].iSorted=m_vtx[i].iSorted0;
		}
		nContactVtx = nContactVtx0;
		time_interval *= 0.5f; ks *= 0.49f; kdScale *= 0.7f;
		if (--iFullIter>0)
			goto reiterate;
		else for(i=0;i<m_nVtx;i++) {
			m_vtx[i].vel.zero();
			m_BBox[0] = BBox0[0]-(m_pos+m_offs0);
			m_BBox[1] = BBox0[1]-(m_pos+m_offs0);
		}
	}

	pe_action_impulse ai;
	ai.iApplyTime = 0;
	if (m_collImpulseScale>0) 
		for(i=0;i<nCheckParts;i++) if (checkParts[i].P.len2()+checkParts[i].L.len2()>0 && checkParts[i].pent->GetType()==PE_LIVING) {
			if (checkParts[i].P.len2()>sqr(m_maxCollImpulse)) {
				a = m_maxCollImpulse/checkParts[i].P.len();
				checkParts[i].P *= a; checkParts[i].L *= a;
			}
			ai.impulse = checkParts[i].P*m_collImpulseScale;
			ai.momentum = checkParts[i].L*m_collImpulseScale;
			ai.ipart = checkParts[i].ipart;
			checkParts[i].pent->Action(&ai);
		}

	m_pos += m_vtx[0].pos; m_vtx[0].pos.zero();
	m_BBox[0] += m_pos+m_offs0; m_BBox[1] += m_pos+m_offs0;
	m_prevTimeInterval = time_interval;
	if (m_wind.len2()*m_airResistance>0 || rmax>m_Emin) {
		m_nSlowFrames = 0; m_bAwake = 1;
	} else if (++m_nSlowFrames>=3) 
		m_bAwake = 0;

	float rscale = m_parts[0].scale==1.0f ? 1.0f:1.0f/m_parts[0].scale;
	box bbox;
	bbox.Basis = matrix3x3f(!m_parts[0].q);
	bbox.center = (m_BBox[0]+m_BBox[1])*0.5f-m_pos-m_parts[0].pos;
	bbox.size = (m_BBox[1]-m_BBox[0])*(0.5f*rscale);
	CTriMesh *pMesh = (CTriMesh*)m_parts[0].pPhysGeomProxy->pGeom;
	((CSingleBoxTree*)pMesh->m_pTree)->SetBox(&bbox);
	d = m_offs0-m_parts[0].pos;
	if (m_parts[0].scale==1.0f && m_parts[0].q.w==1.0f)
		for(i=0;i<m_nVtx;i++) pMesh->m_pVertices[i] = m_vtx[i].pos+d;
	else 
		for(i=0;i<m_nVtx;i++) pMesh->m_pVertices[i] = ((m_vtx[i].pos+d)*rscale)*m_parts[0].q;
	m_bMeshUpdated = 1;
	//for(i=0;i<pMesh->m_nTris;i++)
	//	MARK_UNUSED pMesh->m_pNormals[i];

	if (m_flags & pef_traceable)
		m_pWorld->RepositionEntity(this, 1);

	return 1;
}


int CSoftEntity::RayTrace(CRayGeom *pRay,geom_contact *&pcontacts)
{
	static geom_contact g_SoftContact;

	if (m_nVtx>0) {
		CTriMesh *pMesh = (CTriMesh*)m_parts[0].pPhysGeom->pGeom;
		prim_inters inters;
		triangle atri;
		int i,j;

		for(i=0;i<pMesh->m_nTris;i++) {
			for(j=0;j<3;j++) atri.pt[j] = m_vtx[pMesh->m_pIndices[i*3+j]].pos+m_pos+m_offs0;
			atri.n = atri.pt[1]-atri.pt[0] ^ atri.pt[2]-atri.pt[0];
			if (ray_tri_intersection(&pRay->m_ray,&atri,&inters)) {
				pcontacts = &g_SoftContact;
				pcontacts->pt = inters.pt[0];
				pcontacts->t = (inters.pt[0]-pRay->m_ray.origin)*pRay->m_dirn;
				pcontacts->id[0] = pMesh->m_pIds ? pMesh->m_pIds[i] : m_parts[0].surface_idx;
				pcontacts->iNode[0] = i;
				pcontacts->n = atri.n.normalized()*-sgnnz(atri.n*pRay->m_dirn);
				return 1;
			}
		}
	}

	return 0;
}


void CSoftEntity::ApplyVolumetricPressure(const vectorf &epicenter, float kr, float rmin)
{
	if (m_nVtx>0) {
		CTriMesh *pMesh = (CTriMesh*)m_parts[0].pPhysGeom->pGeom;
		vectorf ptc,r,n,pt[3],dP;
		int i,j,idx[3];
		float r2,rmin2=sqr(rmin);
		kr *= m_explosionScale;

		for(i=0;i<pMesh->m_nTris;i++) {
			for(j=0;j<3;j++) pt[j]=m_vtx[idx[j]=pMesh->m_pIndices[i*3+j]].pos+m_pos+m_offs0;
			ptc = (pt[0]+pt[1]+pt[2])*(1.0f/3); r = ptc-epicenter; r2 = r.len2();
			n = pt[1]-pt[0]^pt[2]-pt[0]; n *= sgnnz(r*n);
			dP = n*((n*r)*0.5f*kr/(sqrt_tpl(n.len2()*r2)*max(rmin2,r2)));
			for(j=0;j<3;j++) 
				m_vtx[idx[j]].vel += dP*m_vtx[idx[j]].massinv;
		}
		m_bAwake = 1;
	}
}


int CSoftEntity::GetStateSnapshot(CStream &stm,float time_back,int flags)
{
	stm.WriteNumberInBits(SNAPSHOT_VERSION,4);
	stm.WriteBits((BYTE*)&m_qrot0,sizeof(m_qrot0)*8);
	stm.Write(m_bAwake!=0);
	return 1;
}

int CSoftEntity::SetStateFromSnapshot(CStream &stm, int flags)
{
	int ver=0; 
	bool bnz;

	stm.ReadNumberInBits(ver,4);
	if (ver!=SNAPSHOT_VERSION)
		return 0;

	if (!(flags & ssf_no_update)) {
		pe_params_pos pp;
		stm.ReadBits((BYTE*)&pp.q,sizeof(pp.q)*8);
		SetParams(&pp);
		stm.Read(bnz);
		m_bAwake = bnz? 1:0;
	} else
		stm.Seek(stm.GetReadPos()+sizeof(quaternionf)*8+1);

	return 1;
}


void CSoftEntity::DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags)
{
	CPhysicalEntity::DrawHelperInformation(DrawLineFunc,flags);

	int i;
	vectorf offs = m_pos+m_offs0;
	if (flags & pe_helper_geometry) for(i=0;i<m_nEdges;i++)
		DrawLineFunc(m_vtx[m_edges[i].ivtx[0]].pos+offs, m_vtx[m_edges[i].ivtx[1]].pos+offs);
	if (flags & pe_helper_collisions) for(i=0;i<m_nVtx;i++) if (m_vtx[i].pContactEnt && !m_vtx[i].bSeparating)
		DrawLineFunc(m_vtx[i].pos+offs-m_vtx[i].ncontact*m_thickness, m_vtx[i].pos+offs);
}


void CSoftEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	CPhysicalEntity::GetMemoryStatistics(pSizer);
	if (GetType()==PE_SOFT)
		pSizer->AddObject(this, sizeof(CSoftEntity));
	pSizer->AddObject(m_vtx, m_nVtx*sizeof(m_vtx[0]));
	pSizer->AddObject(m_edges, m_nEdges*sizeof(m_edges[0]));
	pSizer->AddObject(m_pVtxEdges, m_nEdges*2*sizeof(m_pVtxEdges[0]));
}