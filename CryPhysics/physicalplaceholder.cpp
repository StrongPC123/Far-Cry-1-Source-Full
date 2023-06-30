//////////////////////////////////////////////////////////////////////
//
//	Physical Placeholder
//	
//	File: physicalplaceholder.cpp
//	Description : PhysicalPlaceholder class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bvtree.h"
#include "geometry.h"
#include "geoman.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "physicalworld.h"


IPhysicalWorld *CPhysicalPlaceholder::GetWorld()
{
	CPhysicalWorld *pWorld;
	if (g_nPhysWorlds==1)
		pWorld = g_pPhysWorlds[0];
	else 
		for(int i=0;i<g_nPhysWorlds && !(pWorld=g_pPhysWorlds[i])->IsPlaceholder(this);i++);
	return pWorld;
}


CPhysicalEntity *CPhysicalPlaceholder::GetEntity()
{
	CPhysicalEntity *pEntBuddy;
	if (!m_pEntBuddy) {
		CPhysicalWorld *pWorld;
		if (g_nPhysWorlds==1)
			pWorld = g_pPhysWorlds[0];
		else 
			for(int i=0;i<g_nPhysWorlds && !(pWorld=g_pPhysWorlds[i])->IsPlaceholder(this);i++);
		if (pWorld->m_pPhysicsStreamer) {
			pWorld->SetCurrentEntityHost(this);
			pWorld->m_pPhysicsStreamer->CreatePhysicalEntity(m_pForeignData,m_iForeignData,m_iForeignFlags);
			pWorld->SetCurrentEntityHost(0);
			if (!m_pEntBuddy) {
				pWorld->m_pLog->Log("\002Error: physical entity on-demand creation failed (type: %d, id: %d)",m_iForeignData,m_id);
				pEntBuddy = pWorld->m_pHeightfield;
			} else
				pEntBuddy = (CPhysicalEntity*)m_pEntBuddy;
		}
	}	else
		pEntBuddy = (CPhysicalEntity*)m_pEntBuddy;
	pEntBuddy->m_timeIdle = 0;
	return pEntBuddy;
}


pe_type CPhysicalPlaceholder::GetType()
{
	switch (m_iSimClass) {
		case 0: return PE_STATIC;
		case 1:case 2: return PE_RIGID;
		case 3: return PE_LIVING;
		case 4: return PE_PARTICLE;
		default: return PE_NONE;
	}
}


int CPhysicalPlaceholder::SetParams(pe_params *_params)
{
	if (_params->type==pe_params_bbox::type_id) {
		pe_params_bbox *params = (pe_params_bbox*)_params;
		if (!is_unused(params->BBox[0])) m_BBox[0] = params->BBox[0];
		if (!is_unused(params->BBox[1])) m_BBox[1] = params->BBox[1];
		if (m_pEntBuddy) { 
			m_pEntBuddy->m_BBox[0] = m_BBox[0];
			m_pEntBuddy->m_BBox[1] = m_BBox[1];
		}
		((CPhysicalWorld*)GetWorld())->RepositionEntity(this,1);
		return 1;
	}

	if (_params->type==pe_params_pos::type_id) {
		pe_params_pos *params = (pe_params_pos*)_params;
		if (!is_unused(params->pos) | !is_unused(params->q) | !is_unused(params->scale) | 
				(intptr_t)params->pMtx3x3 | (intptr_t)params->pMtx3x3T | (intptr_t)params->pMtx4x4 | (intptr_t)params->pMtx4x4T)
			return GetEntity()->SetParams(params);
		if (!is_unused(params->iSimClass))
			m_iSimClass = params->iSimClass;
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

	if (m_pEntBuddy)
		return m_pEntBuddy->SetParams(_params);
	return 0;//GetEntity()->SetParams(_params);
}

int CPhysicalPlaceholder::GetParams(pe_params *_params)
{
	if (_params->type==pe_params_bbox::type_id) {
		pe_params_bbox *params = (pe_params_bbox*)_params;
		params->BBox[0] = m_BBox[0];
		params->BBox[1] = m_BBox[1];
		return 1;
	}

	if (_params->type==pe_params_foreign_data::type_id) {
		pe_params_foreign_data *params = (pe_params_foreign_data*)_params;
		params->iForeignData = m_iForeignData;
		params->pForeignData = m_pForeignData;
		params->iForeignFlags = m_iForeignFlags;
		return 1;
	}

	return GetEntity()->GetParams(_params);
}

int CPhysicalPlaceholder::GetStatus(pe_status* _status) 
{ 
	if (_status->type==pe_status_placeholder::type_id) {
		((pe_status_placeholder*)_status)->pFullEntity = m_pEntBuddy;
		return 1;
	}

	if (_status->type==pe_status_awake::type_id)
		return 0;

	return GetEntity()->GetStatus(_status); 
}
int CPhysicalPlaceholder::Action(pe_action* action) { 
	if (action->type==pe_action_awake::type_id && ((pe_action_awake*)action)->bAwake==0 && !m_pEntBuddy) {
		if (m_iSimClass==2)
			m_iSimClass = 1;
		return 1;
	}
	return GetEntity()->Action(action); 
}

int CPhysicalPlaceholder::AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id) { 
	return GetEntity()->AddGeometry(pgeom,params,id); 
}
void CPhysicalPlaceholder::RemoveGeometry(int id) { 
	return GetEntity()->RemoveGeometry(id); 
}

int CPhysicalPlaceholder::GetStateSnapshot(class CStream &stm, float time_back, int flags) { 
	return GetEntity()->GetStateSnapshot(stm,time_back,flags); 
}
int CPhysicalPlaceholder::SetStateFromSnapshot(class CStream &stm, int flags) { 
	return GetEntity()->SetStateFromSnapshot(stm,flags); 
}
int CPhysicalPlaceholder::PostSetStateFromSnapshot() { 
	return GetEntity()->PostSetStateFromSnapshot(); 
}
int CPhysicalPlaceholder::GetStateSnapshotTxt(char *txtbuf,int szbuf, float time_back) {
	return GetEntity()->GetStateSnapshotTxt(txtbuf,szbuf,time_back);
}
void CPhysicalPlaceholder::SetStateFromSnapshotTxt(const char *txtbuf,int szbuf) {
	GetEntity()->SetStateFromSnapshotTxt(txtbuf,szbuf);
}
unsigned int CPhysicalPlaceholder::GetStateChecksum() {
	return GetEntity()->GetStateChecksum();
}

int CPhysicalPlaceholder::Step(float time_interval) { 
	return GetEntity()->Step(time_interval); 
}
void CPhysicalPlaceholder::StartStep(float time_interval) { 
	GetEntity()->StartStep(time_interval); 
}
void CPhysicalPlaceholder::StepBack(float time_interval) { 
	return GetEntity()->StepBack(time_interval); 
}