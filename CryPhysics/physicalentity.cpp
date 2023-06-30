//////////////////////////////////////////////////////////////////////
//
//	Physical Entity
//	
//	File: physicalentity.cpp
//	Description : PhysicalEntity class implementation
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

RigidBody g_StaticRigidBody;
CPhysicalEntity g_StaticPhysicalEntity(0);

CPhysicalEntity::CPhysicalEntity(CPhysicalWorld *pworld) 
{ 
	m_pos.zero(); m_qrot.SetIdentity(); 
	m_BBox[0].zero(); m_BBox[1].zero(); 
	m_iSimClass = 0; m_iPrevSimClass = -1;
	m_nGridThunks = m_nGridThunksAlloc = 0;
	m_pGridThunks = 0;
	m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = -2;
	m_prev = m_next = 0; m_bProcessed = 0;
	m_nRefCount = 0;//1; 0 means that initially no other physical entity references this one
	m_nParts = 0;
	m_nPartsAlloc = 1;
	m_parts = &m_defpart;
	m_iLastIdx = 0;
	m_pWorld = pworld;
	m_pOuterEntity = 0;
	m_pBoundingGeometry = 0;
	m_bProcessed_aux = 0;
	m_nColliders = m_nCollidersAlloc = 0;
	m_pColliders = 0;
	m_iGroup = -1;
	m_bMoved = 0;
	m_id = -1;
	m_pForeignData = 0;
	m_iForeignData = m_iForeignFlags = 0;
	m_timeIdle = m_maxTimeIdle = 0;
	m_bPermanent = 1;
	m_pEntBuddy = 0;
	m_flags = pef_pushable_by_players|pef_traceable;
}

CPhysicalEntity::~CPhysicalEntity()
{
	for(int i=0;i<m_nParts;i++) if (m_parts[i].pPhysGeom) {
		m_pWorld->GetGeomManager()->UnregisterGeometry(m_parts[i].pPhysGeom);
		if (m_parts[i].pPhysGeomProxy!=m_parts[i].pPhysGeom)
			m_pWorld->GetGeomManager()->UnregisterGeometry(m_parts[i].pPhysGeomProxy);
	}
	if (m_pGridThunks) delete[] m_pGridThunks;
	if (m_parts!=&m_defpart) delete[] m_parts;
	if (m_pColliders) delete[] m_pColliders;
}


void CPhysicalEntity::ComputeBBox()
{
	if (m_nParts==0)
		m_BBox[0]=m_BBox[1]=m_pos; 
	else {
		IGeometry *pGeom[3];
		matrix3x3f R;
		int i,j; box abox; vectorf sz,pos;
		m_BBox[0]=vectorf(MAX); m_BBox[1]=vectorf(MIN);

		for(i=0;i<m_nParts;i++) {
			pGeom[0] = m_parts[i].pPhysGeomProxy->pGeom;
			pGeom[1]=pGeom[2] = m_parts[i].pPhysGeom->pGeom; 
			j=0; do {
				pGeom[j]->GetBBox(&abox);
				//(m_qrot*m_parts[i].q).getmatrix(R);	//Q2M_IVO
				R = matrix3x3f(m_qrot*m_parts[i].q);
				abox.Basis *= R.T();
				sz = (abox.size*abox.Basis.Fabs())*m_parts[i].scale;
				pos = m_pos+m_qrot*(m_parts[i].pos+m_parts[i].q*abox.center*m_parts[i].scale);
				m_parts[i].BBox[0] = pos-sz;
				m_parts[i].BBox[1] = pos+sz;
				m_BBox[0].x = min_safe(m_BBox[0].x, m_parts[i].BBox[0].x);
				m_BBox[0].y = min_safe(m_BBox[0].y, m_parts[i].BBox[0].y);
				m_BBox[0].z = min_safe(m_BBox[0].z, m_parts[i].BBox[0].z);
				m_BBox[1].x = max_safe(m_BBox[1].x, m_parts[i].BBox[1].x);
				m_BBox[1].y = max_safe(m_BBox[1].y, m_parts[i].BBox[1].y);
				m_BBox[1].z = max_safe(m_BBox[1].z, m_parts[i].BBox[1].z);
				j++;
			} while(pGeom[j]!=pGeom[j-1]);
		}
	}

	if (m_pEntBuddy) { 
		m_pEntBuddy->m_BBox[0] = m_BBox[0];
		m_pEntBuddy->m_BBox[1] = m_BBox[1];
	}
}


int CPhysicalEntity::SetParams(pe_params *_params)
{
	if (_params->type==pe_params_pos::type_id) {
		pe_params_pos *params = (pe_params_pos*)_params;
		get_xqs_from_matrices(params->pMtx3x3,params->pMtx3x3T,params->pMtx4x4,params->pMtx4x4T, params->pos,params->q,params->scale);
		ENTITY_VALIDATE("CPhysicalEntity:SetParams(pe_params_pos)",params);
		int bPosChanged = 0;
		if (!is_unused(params->pos)) { m_pos = params->pos; bPosChanged=1; }
		if (!is_unused(params->q)) { m_qrot = params->q; bPosChanged=1; }
		if (!is_unused(params->iSimClass) && m_iSimClass>=0 && m_iSimClass<7) {
			m_iSimClass = params->iSimClass;
			m_pWorld->RepositionEntity(this,2);
		}
		if (!is_unused(params->scale)) {
			for(int i=0;i<m_nParts;i++) {
				m_parts[i].pos *= params->scale/m_parts[i].scale;
				m_parts[i].scale = params->scale;
			}	bPosChanged=1; 
		}
		if (params->bRecalcBounds && bPosChanged) {
			CPhysicalEntity **pentlist;
			// make triggers aware of the object's movements
			if (!(m_flags & pef_never_affect_triggers))
				m_pWorld->GetEntitiesAround(m_BBox[0],m_BBox[1],pentlist,ent_triggers,this);
			ComputeBBox();
			m_pWorld->RepositionEntity(this);
			if (!(m_flags & pef_never_affect_triggers))
				m_pWorld->GetEntitiesAround(m_BBox[0],m_BBox[1],pentlist,ent_triggers,this); 
		}
		return 1;
	} 

	if (_params->type==pe_params_bbox::type_id) {
		pe_params_bbox *params = (pe_params_bbox*)_params;
		ENTITY_VALIDATE("CPhysicalEntity::SetParams(pe_params_bbox)",params);
		if (!is_unused(params->BBox[0])) m_BBox[0] = params->BBox[0];
		if (!is_unused(params->BBox[1])) m_BBox[1] = params->BBox[1];
		if (m_pEntBuddy) { 
			m_pEntBuddy->m_BBox[0] = m_BBox[0];
			m_pEntBuddy->m_BBox[1] = m_BBox[1];
		}
		m_pWorld->RepositionEntity(this,1);
		return 1;
	}
	
	if (_params->type==pe_params_part::type_id) {
		pe_params_part *params = (pe_params_part*)_params;
		int i = params->ipart;
		if (i<0)
			for(i=0;i<m_nParts && m_parts[i].id!=params->partid;i++);
		if (i>=m_nParts)
			return 0;
		get_xqs_from_matrices(params->pMtx3x3,params->pMtx3x3T,params->pMtx4x4,params->pMtx4x4T, params->pos,params->q,params->scale);
		ENTITY_VALIDATE("CPhysicalEntity:SetParams(pe_params_part)",params);
		if (!is_unused(params->pos.x)) m_parts[i].pos = params->pos;
		if (!is_unused(params->q)) m_parts[i].q = params->q;
		if (!is_unused(params->scale)) m_parts[i].scale = params->scale;
		if (!is_unused(params->mass)) m_parts[i].mass = params->mass;
		if (!is_unused(params->density)) m_parts[i].mass = m_parts[i].pPhysGeom->V*cube(m_parts[i].scale)*params->density;
		if (!is_unused(params->pPhysGeom) && params->pPhysGeom!=m_parts[i].pPhysGeom) {
			if (m_parts[i].pPhysGeom==m_parts[i].pPhysGeomProxy)
				m_parts[i].pPhysGeomProxy = params->pPhysGeom;
			m_pWorld->GetGeomManager()->UnregisterGeometry(m_parts[i].pPhysGeom);
			m_parts[i].pPhysGeom = params->pPhysGeom;
			m_pWorld->GetGeomManager()->AddRefGeometry(params->pPhysGeom);
		}
		if (!is_unused(params->pPhysGeomProxy) && params->pPhysGeomProxy!=m_parts[i].pPhysGeomProxy) {
			m_pWorld->GetGeomManager()->UnregisterGeometry(m_parts[i].pPhysGeomProxy);
			m_parts[i].pPhysGeomProxy = params->pPhysGeomProxy;
			m_pWorld->GetGeomManager()->AddRefGeometry(params->pPhysGeomProxy);
		}
		m_parts[i].flags = m_parts[i].flags & params->flagsAND | params->flagsOR;
		m_parts[i].flagsCollider = m_parts[i].flagsCollider & params->flagsColliderAND | params->flagsColliderOR;
		if (params->bRecalcBBox)
			ComputeBBox();
		return i+1;
	} 
	
	if (_params->type==pe_params_outer_entity::type_id) {
		m_pOuterEntity = (CPhysicalEntity*)((pe_params_outer_entity*)_params)->pOuterEntity;
		m_pBoundingGeometry = (CGeometry*)((pe_params_outer_entity*)_params)->pBoundingGeometry;
		return 1;
	}
	
	if (_params->type==pe_params_foreign_data::type_id) {
		pe_params_foreign_data *params = (pe_params_foreign_data*)_params;
		if (!is_unused(params->iForeignData)) m_iForeignData = params->iForeignData;
		if (!is_unused(params->pForeignData)) m_pForeignData = params->pForeignData;
		if (!is_unused(params->iForeignFlags)) m_iForeignFlags = params->iForeignFlags;
		if (m_pEntBuddy) {
			m_pEntBuddy->m_pForeignData = m_pForeignData;
			m_pEntBuddy->m_iForeignData = m_iForeignData;
			m_pEntBuddy->m_iForeignFlags = m_iForeignFlags;
		}
		return 1;
	}

	if (_params->type==pe_params_flags::type_id) {
		pe_params_flags *params = (pe_params_flags*)_params;
		int flags = m_flags;
		if (!is_unused(params->flags)) m_flags = params->flags;
		if (!is_unused(params->flagsAND)) m_flags &= params->flagsAND;
		if (!is_unused(params->flagsOR)) m_flags |= params->flagsOR;

		if (m_flags&pef_traceable && m_ig[0].x==-3) {
			m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = -2;
			if (m_pos.len2()>0)
				m_pWorld->RepositionEntity(this,1);
		}
		if (!(m_flags&pef_traceable) && m_ig[0].x!=-3) {
			m_pWorld->DetachEntityGridThunks(this);
			m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = -3;
		}

		return 1;
	}

	return 0;
}


int CPhysicalEntity::GetParams(pe_params *_params)
{
	if (_params->type==pe_params_bbox::type_id) {
		pe_params_bbox *params = (pe_params_bbox*)_params;
		params->BBox[0] = m_BBox[0];
		params->BBox[1] = m_BBox[1];
		return 1;
	}

	if (_params->type==pe_params_outer_entity::type_id) {
		((pe_params_outer_entity*)_params)->pOuterEntity = m_pOuterEntity;
		((pe_params_outer_entity*)_params)->pBoundingGeometry = m_pBoundingGeometry;
		return 1;
	}
	
	if (_params->type==pe_params_foreign_data::type_id) {
		pe_params_foreign_data *params = (pe_params_foreign_data*)_params;
		params->iForeignData = m_iForeignData;
		params->pForeignData = m_pForeignData;
		params->iForeignFlags = m_iForeignFlags;
		return 1;
	}

	if (_params->type==pe_params_part::type_id) {
		pe_params_part *params = (pe_params_part*)_params;
		int i;
		if (params->ipart==-1 && params->partid>=0) {
			for(i=0;i<m_nParts && m_parts[i].id!=params->partid;i++);
			if (i==m_nParts) return 0;
		} else
			i = params->ipart;
		params->partid = m_parts[params->ipart = i].id;
		params->pos = m_parts[i].pos;
		params->q = m_parts[i].q;
		params->scale = m_parts[i].scale;
		if (params->pMtx3x3) //resq.getmatrix((matrix3x3f&)*status->pMtx3x3) *= resscale; //Q2M_IVO
			(matrix3x3f&)*params->pMtx3x3 = (matrix3x3f(m_parts[i].q)*m_parts[i].scale);
		if (params->pMtx3x3T)	//resq.getmatrix((matrix3x3Tf&)*params->pMtx3x3T) *= resscale; //Q2M_IVO
			(matrix3x3Tf&)*params->pMtx3x3T=(matrix3x3f(m_parts[i].q)*m_parts[i].scale);
		if (params->pMtx4x4) //(resq.getmatrix((matrix3x3in4x4f&)*params->pMtx4x4)*=resscale).SetColumn(3,respos);	//Q2M_IVO
			((matrix3x3in4x4f&)*params->pMtx4x4 = (matrix3x3f(m_parts[i].q)*m_parts[i].scale)).SetColumn(3,m_parts[i].pos);
		if (params->pMtx4x4T)	//(resq.getmatrix((matrix3x3in4x4Tf&)*params->pMtx4x4T)*=resscale).SetColumn(3,respos);	//Q2M_IVO
			((matrix3x3in4x4Tf&)*params->pMtx4x4T = (matrix3x3f(m_parts[i].q)*m_parts[i].scale)).SetColumn(3,m_parts[i].pos);
		params->flagsOR=params->flagsAND = m_parts[i].flags;
		params->flagsColliderOR=params->flagsColliderAND = m_parts[i].flagsCollider;
		params->mass = m_parts[i].mass;
		params->density = m_parts[i].pPhysGeomProxy->V>0 ?
			m_parts[i].mass/(m_parts[i].pPhysGeomProxy->V*cube(m_parts[i].scale)) : 0;
		params->pPhysGeom = m_parts[i].pPhysGeom;
		params->pPhysGeomProxy = m_parts[i].pPhysGeomProxy;
	}

	if (_params->type==pe_params_flags::type_id) {
		((pe_params_flags*)_params)->flags = m_flags;
		return 1;
	}

	return 0;
}


int CPhysicalEntity::GetStatus(pe_status *_status)
{
	if (_status->type==pe_status_pos::type_id) {
		pe_status_pos *status = (pe_status_pos*)_status;
		vectorf respos;
		quaternionf resq;
		float resscale;
		int i;

		if (status->ipart==-1 && status->partid>=0) {
			for(i=0;i<m_nParts && m_parts[i].id!=status->partid;i++);
			if (i==m_nParts) return 0;
		} else
			i = status->ipart;

		if (i==-1) {
			respos=m_pos; resq=m_qrot; resscale=1.0f;
			for(i=0,status->flagsOR=0,status->flagsAND=-1;i<m_nParts;i++) 
				status->flagsOR|=m_parts[i].flags, status->flagsAND&=m_parts[i].flags;
			if (m_nParts>0) {
				status->pGeom = m_parts[0].pPhysGeom->pGeom;
				status->pGeomProxy = m_parts[0].pPhysGeomProxy->pGeom;
			} else
				status->pGeom = status->pGeomProxy = 0;
		} else {
			if (status->flags & status_local) {
				respos = m_parts[i].pos; resq = m_parts[i].q;
			} else {
				respos = m_pos+m_qrot*m_parts[i].pos;
				resq = m_qrot*m_parts[i].q;
			}
			resscale = m_parts[i].scale;
			status->flagsOR = status->flagsAND = m_parts[i].flags;
			status->pGeom = m_parts[i].pPhysGeom->pGeom;
			status->pGeomProxy = m_parts[i].pPhysGeomProxy->pGeom;
		}

		status->pos = respos;
		status->q = resq;
		status->scale = resscale;
		status->iSimClass = m_iSimClass;
		if (status->pMtx3x3) //resq.getmatrix((matrix3x3f&)*status->pMtx3x3) *= resscale; //Q2M_IVO
			(matrix3x3f&)*status->pMtx3x3 = (matrix3x3f(resq)*resscale);
		if (status->pMtx3x3T)	//resq.getmatrix((matrix3x3Tf&)*status->pMtx3x3T) *= resscale; //Q2M_IVO
			(matrix3x3Tf&)*status->pMtx3x3T=(matrix3x3f(resq)*resscale);
		if (status->pMtx4x4) //(resq.getmatrix((matrix3x3in4x4f&)*status->pMtx4x4)*=resscale).SetColumn(3,respos);	//Q2M_IVO
			((matrix3x3in4x4f&)*status->pMtx4x4 = (matrix3x3f(resq)*resscale)).SetColumn(3,respos);
		if (status->pMtx4x4T)	//(resq.getmatrix((matrix3x3in4x4Tf&)*status->pMtx4x4T)*=resscale).SetColumn(3,respos);	//Q2M_IVO
			((matrix3x3in4x4Tf&)*status->pMtx4x4T = (matrix3x3f(resq)*resscale)).SetColumn(3,respos);
		status->BBox[0] = m_BBox[0]-m_pos;
		status->BBox[1] = m_BBox[1]-m_pos;
		return 1;
	}

	if (_status->type==pe_status_id::type_id) {
		pe_status_id *status = (pe_status_id*)_status;
		int ipart = status->ipart;
		if (ipart<0) 
			for(ipart=0;ipart<m_nParts-1 && m_parts[ipart].id!=status->partid;ipart++);
		if (ipart>=m_nParts)
			return 0;
		phys_geometry *pgeom = status->bUseProxy ? m_parts[ipart].pPhysGeomProxy : m_parts[ipart].pPhysGeom;
		if ((unsigned int)status->iPrim >= (unsigned int)pgeom->pGeom->GetPrimitiveCount() ||
				pgeom->pGeom->GetType()==GEOM_TRIMESH && status->iFeature>2)
			return 0;
		status->id = pgeom->pGeom->GetPrimitiveId(status->iPrim, status->iFeature);
		status->id = status->id&~(status->id>>31) | m_parts[ipart].surface_idx&status->id>>31;
		return 1;
	}

	if (_status->type==pe_status_nparts::type_id)
		return m_nParts;

	if (_status->type==pe_status_awake::type_id)
		return IsAwake();

	if (_status->type==pe_status_contains_point::type_id)
		return IsPointInside(((pe_status_contains_point*)_status)->pt);

	if (_status->type==pe_status_caps::type_id) {
		pe_status_caps *status = (pe_status_caps*)_status;
		status->bCanAlterOrientation = 0;
		return 1;
	}

	return 0;
};


int CPhysicalEntity::Action(pe_action *_action)
{
	if (_action->type==pe_action_awake::type_id && m_iSimClass>=0 && m_iSimClass<7) {
		Awake(((pe_action_awake*)_action)->bAwake,1);	
		return 1;
	}
	if (_action->type==pe_action_remove_all_parts::type_id) {
		for(int i=m_nParts-1;i>=0;i--)
			RemoveGeometry(m_parts[i].id);
		return 1;
	}
	return 0;
}


int CPhysicalEntity::AddGeometry(phys_geometry *pgeom, pe_geomparams* params, int id)
{
	if (!pgeom)
		return -1;
	box abox; pgeom->pGeom->GetBBox(&abox);
	float mindim=min(min(abox.size.x,abox.size.y),abox.size.z), mindims=mindim*params->scale;
	float maxdim=max(max(abox.size.x,abox.size.y),abox.size.z), maxdims=maxdim*params->scale;

	if (id>=0) {
		int i; for(i=0;i<m_nParts && m_parts[i].id!=id;i++);
		if (i<m_nParts) {
			if (params->flags & geom_proxy) {
				if (pgeom) {
					if (m_parts[i].pPhysGeomProxy!=pgeom)
						m_pWorld->GetGeomManager()->AddRefGeometry(pgeom);
					m_parts[i].pPhysGeomProxy = pgeom;
					if (mindims<0.15f)
						m_parts[i].flags |= geom_has_thin_parts;
				}
			} else {
				m_parts[i].pos = params->pos;
				m_parts[i].q = params->q;
			}
			return id;
		}
	}
	get_xqs_from_matrices(params->pMtx3x3,params->pMtx3x3T,params->pMtx4x4,params->pMtx4x4T, params->pos,params->q,params->scale);
	ENTITY_VALIDATE_ERRCODE("CPhysicalEntity:AddGeometry",params,-1);
	if (m_nParts==m_nPartsAlloc) {
		geom *pparts = m_parts;
		memcpy(m_parts = new geom[m_nPartsAlloc=m_nPartsAlloc+4&~3], pparts, sizeof(geom)*m_nParts);
		if (pparts!=&m_defpart) delete[] pparts;
	}
	m_parts[m_nParts].id = id<0 ? m_iLastIdx++:id;
	m_parts[m_nParts].pPhysGeom = m_parts[m_nParts].pPhysGeomProxy = pgeom;
	m_pWorld->GetGeomManager()->AddRefGeometry(pgeom);
	m_parts[m_nParts].surface_idx = pgeom->surface_idx;
	if (!is_unused(params->surface_idx)) m_parts[m_nParts].surface_idx = params->surface_idx;
	m_parts[m_nParts].flags = params->flags & ~geom_proxy;
	m_parts[m_nParts].flagsCollider = params->flagsCollider;
	m_parts[m_nParts].pos = params->pos;
	m_parts[m_nParts].q = params->q;
	m_parts[m_nParts].scale = params->scale;
	if (is_unused(params->minContactDist)) {
		m_parts[m_nParts].minContactDist = max(maxdims*0.03f,mindims*(mindims<maxdims*0.3f ? 0.4f:0.1f));
		if (mindims<0.15f)
			m_parts[m_nParts].flags |= geom_has_thin_parts;
	} else
		m_parts[m_nParts].minContactDist = params->minContactDist;
	m_parts[m_nParts].maxdim = maxdim;
	m_nParts++;

	if (params->bRecalcBBox) {
		ComputeBBox();
		m_pWorld->RepositionEntity(this,1);
	}

	return m_parts[m_nParts-1].id;
}


void CPhysicalEntity::RemoveGeometry(int id)
{
	for(int i=0;i<m_nParts;i++) if (m_parts[i].id==id) {
		m_pWorld->GetGeomManager()->UnregisterGeometry(m_parts[i].pPhysGeom);
		if (m_parts[i].pPhysGeomProxy!=m_parts[i].pPhysGeom)
			m_pWorld->GetGeomManager()->UnregisterGeometry(m_parts[i].pPhysGeomProxy);

		for(;i<m_nParts-1;i++) m_parts[i]=m_parts[i+1];
		m_nParts--;
		ComputeBBox();
		m_pWorld->RepositionEntity(this,1);
		return;
	}
}


RigidBody *CPhysicalEntity::GetRigidBody(int ipart)
{
	g_StaticRigidBody.P.zero(); g_StaticRigidBody.v.zero();
	g_StaticRigidBody.L.zero(); g_StaticRigidBody.w.zero();
	return &g_StaticRigidBody;
}


int CPhysicalEntity::IsPointInside(vectorf pt)
{
	pt = (pt-m_pos)*m_qrot;
	if (m_pBoundingGeometry)
		return m_pBoundingGeometry->PointInsideStatus(pt);
	for(int i=0;i<m_nParts;i++) if ((m_parts[i].flags & geom_collides) && 
			m_parts[i].pPhysGeom->pGeom->PointInsideStatus((pt-m_parts[i].pos)*m_parts[i].q)) 
		return 1;
	return 0;
}	


void CPhysicalEntity::AlertNeighbourhoodND()
{
	int i;
	if (m_iSimClass>3)
		return;

	for(i=0;i<m_nColliders;i++)	if (m_pColliders[i]!=this) {
		m_pColliders[i]->RemoveCollider(this);
		m_pColliders[i]->Awake();
		m_pColliders[i]->Release();
		m_nRefCount--;
	}
	m_nColliders = 0;

	if (m_nRefCount>0 || m_flags&pef_always_notify_on_deletion) {
		CPhysicalEntity **pentlist;
		vectorf inflator = (m_BBox[1]-m_BBox[0])*1E-3f+vectorf(4,4,4)*m_pWorld->m_vars.maxContactGap;
		for(i=m_pWorld->GetEntitiesAround(m_BBox[0]-inflator,m_BBox[1]+inflator, pentlist, 
			ent_sleeping_rigid|ent_rigid|ent_living|ent_independent|ent_triggers)-1; i>=0; i--)
			pentlist[i]->Awake();
	}
}


int CPhysicalEntity::RemoveCollider(CPhysicalEntity *pCollider, bool bRemoveAlways)
{
	if (m_pColliders && m_iSimClass>0) {
		int i,islot; for(i=0;i<m_nColliders && m_pColliders[i]!=pCollider;i++);
		if (i<m_nColliders) {
			for(islot=i;i<m_nColliders-1;i++) m_pColliders[i] = m_pColliders[i+1];
			if (pCollider!=this)
				pCollider->Release();
			m_nColliders--; return islot;
		}
	}
	return -1;
}

int CPhysicalEntity::AddCollider(CPhysicalEntity *pCollider)
{
	if (m_iSimClass==0)
		return 1;
	int i,j;
	for(i=0;i<m_nColliders && m_pColliders[i]!=pCollider;i++);
	if (i==m_nColliders) {
		if (m_nColliders==m_nCollidersAlloc) {
			CPhysicalEntity **pColliders = m_pColliders;
			memcpy(m_pColliders = new (CPhysicalEntity*[m_nCollidersAlloc+=8]), pColliders, sizeof(CPhysicalEntity*)*m_nColliders);
			if (pColliders) delete[] pColliders;
		}
		for(i=0;i<m_nColliders && pCollider->GetMassInv()>m_pColliders[i]->GetMassInv();i++);
		for(j=m_nColliders-1; j>=i; j--) m_pColliders[j+1] = m_pColliders[j];
		m_pColliders[i] = pCollider; m_nColliders++;
		if (pCollider!=this)
			pCollider->AddRef();
	}
	return i;
}


void CPhysicalEntity::DrawHelperInformation(void (*DrawLineFunc)(float*,float*), int flags)
{
	if (flags & pe_helper_bbox) {
		int i,j;
		vectorf sz,center,pt[8];
		center = (m_BBox[1]+m_BBox[0])*0.5f;
		sz = (m_BBox[1]-m_BBox[0])*0.5f;
		for(i=0;i<8;i++)
			pt[i] = vectorf(sz.x*((i<<1&2)-1),sz.y*((i&2)-1),sz.z*((i>>1&2)-1))+center;
		for(i=0;i<8;i++) for(j=0;j<3;j++) if (i&1<<j)
			DrawLineFunc(pt[i],pt[i^1<<j]);
	}

	if (flags & pe_helper_geometry) {
		int iLevel = flags>>16;
		geom_world_data gwd;
		for(int i=0;i<m_nParts;i++) {
			//(m_qrot*m_parts[i].q).getmatrix(gwd.R);	//Q2M_IVO
			gwd.R = matrix3x3f(m_qrot*m_parts[i].q);
			gwd.offset = m_pos + m_qrot*m_parts[i].pos;
			gwd.scale = m_parts[i].scale;
			m_parts[i].pPhysGeom->pGeom->DrawWireframe(DrawLineFunc,&gwd, iLevel);
			if (m_parts[i].pPhysGeomProxy!=m_parts[i].pPhysGeom)
				m_parts[i].pPhysGeomProxy->pGeom->DrawWireframe(DrawLineFunc,&gwd, iLevel);
		}
	}
}


void CPhysicalEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	if (GetType()==PE_STATIC)
		pSizer->AddObject(this, sizeof(CPhysicalEntity));
	if (m_parts!=&m_defpart)
		pSizer->AddObject(m_parts, m_nParts*sizeof(m_parts[0]));
	pSizer->AddObject(m_pGridThunks, m_nGridThunksAlloc*sizeof(m_pGridThunks[0]));
	pSizer->AddObject(m_pColliders, m_nCollidersAlloc*sizeof(m_pColliders[0]));
}


int CPhysicalEntity::GetStateSnapshotTxt(char *txtbuf,int szbuf, float time_back) 
{
	CStream stm;
	GetStateSnapshot(stm,time_back);
	int size=bin2ascii(stm.GetPtr(),(stm.GetSize()-1>>3)+1,(unsigned char*)txtbuf);
/*
	// debugging
	static char test[1024*16];
	int testsize=ascii2bin((const unsigned char*)txtbuf,size,(unsigned char*)test);
	if(memcmp(test,stm.GetPtr(),testsize)!=0)
	{
		char str[256];
		
		sprintf(str,"%d %d\n",size,testsize);
		
		OutputDebugString(str);
	}
*/

	return size;
}

void CPhysicalEntity::SetStateFromSnapshotTxt(const char *txtbuf,int szbuf)
{
	CStream stm; 
	stm.SetSize(ascii2bin((const unsigned char*)txtbuf,szbuf,stm.GetPtr())*8);
	SetStateFromSnapshot(stm);
}
