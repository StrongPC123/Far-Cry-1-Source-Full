//////////////////////////////////////////////////////////////////////
//
//	Articulated Entity
//	
//	File: articulatedentity.cpp
//	Description : ArticulatedEntity class implementation
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
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h" 
#include "geoman.h"
#include "physicalworld.h"
#include "rigidentity.h"
#include "articulatedentity.h"

int __ae_step=0; // for debugging

CArticulatedEntity::CArticulatedEntity(CPhysicalWorld *pWorld) : CRigidEntity(pWorld)
{
	m_nJoints = 0;
	m_bGrounded = 0;
	m_acc.Set(0,0,0); 
	m_wacc.Set(0,0,0);
	m_joints = new ae_joint[m_nJointsAlloc=4];
	m_nJoints = 0;
	m_nContactsAlloc = 0;
	m_offsPivot.Set(0,0,0);
	m_bAwake = 0;
	m_simTime = 0;
	m_simTimeAux = 10.0f;
	m_scaleBounceResponse = 1;
	m_posPivot.Set(0,0,0);
	m_bIaReady = 0;
	m_nPartsAlloc = 4;
	m_parts = new geom[m_nPartsAlloc];
	m_infos = new ae_part_info[m_nPartsAlloc];
	m_bCheckCollisions = 0;
	m_bCollisionResp = 0;
	m_maxAllowedStep = 0.02f;
	m_bInheritVel = 0;
	m_pHost = 0;
	m_posHostPivot.Set(0,0,0);
	m_bPartPosForced = 0;
	m_bExertImpulse = 0;
	m_Ya_vec[0].Set(0,0,0);
	m_Ya_vec[0].Set(0,0,0);
	m_nContacts = m_nDynContacts = m_bInGroup = 0;
	m_nCollLyingMode = 5;
	m_gravityLyingMode.Set(0,0,-5.0f);
	m_dampingLyingMode = 1.0f;
	m_EminLyingMode = 0.04f;
	m_maxPenetrationCur = 0;
	m_iSimType = 0;
	m_iSimTypeLyingMode = 1;
	m_iSimTypeCur = 0;
	m_iSimTypeOverride = 0;
	m_bFastLimbs = 0;
	m_bExpandHinges = 0;
	m_bUsingUnproj = 0;
	m_softness[0] = 0.01f;
	m_softness[1] = 0.05f;
	m_softness[2] = 0.001f;
	m_softness[3] = 0.01f;
	m_bUpdateBodies = 1;
	m_bCanSweep = 0;
}

CArticulatedEntity::~CArticulatedEntity()
{
	if (m_joints) delete[] m_joints;
	if (m_infos) delete[] m_infos;
}

void CArticulatedEntity::AlertNeighbourhoodND()
{
	if (m_pHost)
		m_pHost->Release();
	m_pHost = 0;
	CRigidEntity::AlertNeighbourhoodND();
}


int CArticulatedEntity::AddGeometry(phys_geometry *pgeom, pe_geomparams* _params,int id)
{
	if (_params->type!=pe_articgeomparams::type_id) {
		pe_articgeomparams params = *(pe_geomparams*)_params;
		return AddGeometry(pgeom,&params,id);
	}
	pe_articgeomparams *params = (pe_articgeomparams*)_params;

	int res,i,nPartsAlloc=m_nPartsAlloc,nParts=m_nParts,idx;
	if ((res = CPhysicalEntity::AddGeometry(pgeom,params,id))<0 || m_nParts==nParts && params->flags & geom_proxy)
		return res;

	idx = m_nParts-1;
	float V=pgeom->V*cube(params->scale), M=params->mass ? params->mass : params->density*V;
	vectorf bodypos = m_pos+m_qrot*(params->pos+params->q*pgeom->origin); 
	quaternionf bodyq = m_qrot*params->q*pgeom->q;
	if (M<=0) M=0.1f;
	m_parts[idx].mass = M;
	if (nPartsAlloc < m_nPartsAlloc) {
		ae_part_info *pInfos = m_infos;
		memcpy(m_infos = new ae_part_info[m_nPartsAlloc], pInfos, sizeof(ae_part_info)*nPartsAlloc);
		if (pInfos) delete[] pInfos;
	}

	for(i=0;i<m_nJoints && m_joints[i].idbody!=params->idbody;i++);
	if (i==m_nJoints) {
		if (m_nJoints==m_nJointsAlloc) {
			ae_joint *pJoints = m_joints;
			memcpy(m_joints=new ae_joint[m_nJointsAlloc+=4], pJoints, sizeof(ae_joint)*m_nJoints);
			if (pJoints) delete[] pJoints;
		}
		m_joints[m_nJoints].iStartPart = idx;
		m_joints[m_nJoints].nParts = 0;
		m_joints[m_nJoints++].idbody = params->idbody;
	}
	if (m_joints[i].iStartPart+m_joints[i].nParts<idx) {
		geom tmppart = m_parts[idx];
		for(; idx>m_joints[i].iStartPart+m_joints[i].nParts; idx--) {
			m_parts[idx] = m_parts[idx-1]; m_infos[idx] = m_infos[idx-1];
		}
		m_parts[idx] = tmppart;
	}
	m_joints[i].nParts++;

	if (M>0) {
		// pivot positions are not recalculated
		if (m_body.M==0)
			m_body.Create(bodypos,pgeom->Ibody*sqr(params->scale)*cube(params->scale),bodyq, V,M, m_qrot,m_pos);
		else
			m_body.Add(bodypos,pgeom->Ibody*sqr(params->scale)*cube(params->scale),bodyq,V,M);	

		if (m_joints[i].body.M==0) {
			m_joints[i].body.Create(bodypos,pgeom->Ibody*sqr(params->scale)*cube(params->scale),bodyq, V,M, m_qrot*params->q,bodypos);
			m_joints[i].body.pOwner = this;
			m_joints[i].body.softness[0] = m_softness[0];
			m_joints[i].body.softness[1] = m_softness[1];
		} else
			m_joints[i].body.Add(bodypos,pgeom->Ibody*sqr(params->scale)*cube(params->scale),bodyq,V,M);	
	}
	m_infos[idx].iJoint = i;
	m_infos[idx].idbody = params->idbody;
	m_joints[i].quat = m_joints[i].body.q*m_joints[i].body.qfb;
	m_infos[idx].q0 = !m_joints[i].quat*(m_qrot*m_parts[idx].q);
	m_infos[idx].pos0 = (m_pos+m_qrot*m_parts[idx].pos-m_joints[i].body.pos)*m_joints[i].quat;
	
	return res;
}

void CArticulatedEntity::RemoveGeometry(int id)
{
	int i,j;
	for(i=0;i<m_nParts && m_parts[i].id!=id;i++);
	if (i==m_nParts) return;
	phys_geometry *pgeom = m_parts[i].pPhysGeom;

	if (m_parts[i].mass>0) {
		vectorf bodypos = m_pos + m_qrot*(m_parts[i].pos+pgeom->origin); 
		quaternionf bodyq = m_qrot*m_parts[i].q*pgeom->q;
		m_body.Add(bodypos,-pgeom->Ibody,bodyq,-pgeom->V,-m_parts[i].mass);
		m_joints[m_infos[i].iJoint].body.Add(bodypos,-pgeom->Ibody,bodyq,-pgeom->V,-m_parts[i].mass);
		if (m_body.M<1E-8) m_body.M = 0;
		if (m_joints[m_infos[i].iJoint].body.M<1E-8) m_joints[m_infos[i].iJoint].body.M = 0;
		m_joints[m_infos[i].iJoint].nParts--;
		for(j=0;j<m_nJoints;j++) if (m_joints[j].iStartPart>i)
			m_joints[j].iStartPart--;
	}

	CPhysicalEntity::RemoveGeometry(id);
}

void CArticulatedEntity::RecomputeMassDistribution(int ipart,int bMassChanged)
{
	int i,iStart,iEnd;
	float V;
	vectorf bodypos;
	quaternionf bodyq;

	if (ipart>=0) {
		if (!bMassChanged)
			return;
		i = m_infos[ipart].iJoint; m_joints[i].body.zero();
		iStart = m_joints[i].iStartPart; iEnd = m_joints[i].iStartPart+m_joints[i].nParts-1;
	}	else {
		m_body.zero();
		for(i=0;i<m_nJoints;i++) m_joints[i].body.zero();
		iStart = 0; iEnd = m_nParts-1;
	}

	for(i=iStart; i<=iEnd; i++) {
		V = m_parts[i].pPhysGeom->V*cube(m_parts[i].scale);
		bodypos = m_pos + m_qrot*(m_parts[i].pos+m_parts[i].q*m_parts[i].pPhysGeom->origin); 
		bodyq = m_qrot*m_parts[i].q*m_parts[i].pPhysGeom->q;

		if (m_parts[i].mass>0) {
			if (ipart<0) {
				if (m_body.M==0)
					m_body.Create(bodypos,m_parts[i].pPhysGeom->Ibody*sqr(m_parts[i].scale)*cube(m_parts[i].scale),bodyq, V,m_parts[i].mass, m_qrot,m_pos);
				else
					m_body.Add(bodypos,m_parts[i].pPhysGeom->Ibody*sqr(m_parts[i].scale)*cube(m_parts[i].scale),bodyq, V,m_parts[i].mass);
			}

			if (m_joints[m_infos[i].iJoint].body.M==0) {
				m_joints[m_infos[i].iJoint].body.Create(bodypos,m_parts[i].pPhysGeom->Ibody*sqr(m_parts[i].scale)*cube(m_parts[i].scale),bodyq, 
					V,m_parts[i].mass, m_qrot,m_pos);
				m_joints[i].body.pOwner = this;
				m_joints[i].body.softness[0] = m_softness[0];
				m_joints[i].body.softness[1] = m_softness[1];
			} else
				m_joints[m_infos[i].iJoint].body.Add(bodypos,m_parts[i].pPhysGeom->Ibody*sqr(m_parts[i].scale)*cube(m_parts[i].scale),bodyq, 
					V,m_parts[i].mass);	
		}
	}
}


int CArticulatedEntity::SetParams(pe_params *_params)
{
	vectorf prevpos = m_pos;
	quaternionf prevq = m_qrot;

	if (CRigidEntity::SetParams(_params)) {
		if (_params->type==pe_params_pos::type_id) {
			pe_params_pos *params = (pe_params_pos*)_params;
			int i=-1;
			if ((prevq.v-m_qrot.v).len2()>0) {
				if (m_nJoints>0) {
					/*if (m_bGrounded) {
						if ((m_joints[0].flags & all_angles_locked)!=all_angles_locked)	{
							vectorf dangle; int j;
							quaternionf qalpha(m_joints[0].q+m_joints[0].qext); 
							(!m_qrot*m_joints[0].quat0*qalpha).get_Euler_angles_xyz(dangle);
							for(j=0;j<3;j++) if (!(m_joints[0].flags & angle0_locked<<j))
								m_joints[0].q[j] = dangle[i]-m_joints[0].qext[j];
						}
						m_joints[0].quat0 = m_qrot;
					} else {
						m_qrot.get_Euler_angles_xyz(m_joints[0].q);
						m_joints[0].qext.zero();
					}*/
					m_offsPivot = (m_qrot*!prevq)*m_offsPivot;
					m_posPivot = m_pos + m_offsPivot;
					m_joints[0].quat0 = m_qrot;
					i = 0;
				}
			}
			if (!m_bPartPosForced)
				m_qrot.SetIdentity();
			if ((prevpos-m_pos).len2()>0) {
				m_posPivot = m_pos + m_offsPivot; i = 0;
			}
			if (i>=0 && params->bRecalcBounds)
				for(;i<m_nJoints;i++) SyncBodyWithJoint(i);
		}
		if (_params->type==pe_params_part::type_id)
			m_bPartPosForced = 1;
		return 1;
	}

	if (_params->type==pe_params_joint::type_id) {
		pe_params_joint *params = (pe_params_joint*)_params;
		m_bPartPosForced = 0;
		int i,j,op[2]={-1,-1},nChanges=0;
		for(i=0;i<m_nJoints;i++) {
			if (m_joints[i].idbody==params->op[0]) op[0]=i;
			if (m_joints[i].idbody==params->op[1]) op[1]=i;
		}
		if (op[0]<0 && params->op[0]>=0 || op[1]<0) 
			return 0;

		quaternionf qparent, qchild = m_joints[op[1]].body.q*m_joints[op[1]].body.qfb;
		vectorf posparent, poschild = m_joints[op[1]].body.pos;
		if (op[0]>=0) {
			qparent = m_joints[op[0]].body.q*m_joints[op[0]].body.qfb;
			posparent = m_joints[op[0]].body.pos;
		} else {
			qparent.SetIdentity(); 
			posparent = m_posPivot;
		}

		if (!is_unused(params->pivot)) {
			vectorf pivot = m_qrot*params->pivot + m_pos;
			if (params->op[0]!=-1)
				m_joints[op[1]].pivot[0] = (pivot-posparent)*qparent;
			else m_joints[op[1]].pivot[0].Set(0,0,0);
			if (m_bGrounded || params->op[0]!=-1)
				m_joints[op[1]].pivot[1] = (pivot-poschild)*qchild;
			else m_joints[op[1]].pivot[1].Set(0,0,0);
		}
		if (!is_unused(params->flags)) {
			m_joints[op[1]].flags = params->flags | m_joints[op[1]].flags & params->flags & (angle0_limit_reached|angle0_gimbal_locked)*7;
			for(i=0;i<3;i++) if (params->flags & angle0_locked<<i)
				m_joints[op[1]].dq[i] = 0;
		}
		if (!is_unused(params->nSelfCollidingParts)) {
			for(i=0;i<m_nParts;i++) for(j=0;j<params->nSelfCollidingParts;j++) if (m_parts[i].id==params->pSelfCollidingParts[j])
				m_joints[op[1]].selfCollMask |= getmask(i);
		}

		float rdt = !is_unused(params->ranimationTimeStep) ? params->ranimationTimeStep : 1.0f/params->animationTimeStep;
		for(i=0;i<3;i++) {
			if (!is_unused(params->limits[0][i])) m_joints[op[1]].limits[0][i] = params->limits[0][i];
			if (!is_unused(params->limits[1][i])) m_joints[op[1]].limits[1][i] = params->limits[1][i];
			if (!is_unused(params->bounciness[i])) m_joints[op[1]].bounciness[i] = params->bounciness[i];
			if (!is_unused(params->ks[i])) m_joints[op[1]].ks[i] = params->ks[i];
			if (!is_unused(params->kd[i])) 
				m_joints[op[1]].kd[i] = ((m_joints[op[1]].flags & angle0_auto_kd<<i) ? 2.0f*sqrt_tpl(m_joints[op[1]].ks[i]) : 1.0f)*params->kd[i];
			if (!is_unused(params->qdashpot[i])) m_joints[op[1]].qdashpot[i] = params->qdashpot[i];
			if (!is_unused(params->kdashpot[i])) m_joints[op[1]].kdashpot[i] = params->kdashpot[i];
			if (!is_unused(params->q[i])) m_joints[op[1]].q[i] = params->q[i];
			if (!is_unused(params->qext[i])) { 
				m_joints[op[1]].dqext[i] = (params->qext[i]-m_joints[op[1]].qext[i])*rdt;
				m_joints[op[1]].qext[i] = params->qext[i]; nChanges++; 
				if (!(m_joints[op[1]].flags & angle0_locked<<i) && 
						isneg(m_joints[op[1]].limits[0][i]-m_joints[op[1]].qext[i]) + isneg(m_joints[op[1]].qext[i]-m_joints[op[1]].limits[1][i]) + 
						isneg(m_joints[op[1]].limits[1][i]-m_joints[op[1]].limits[1][i]) < 2) 
				{	// qext violates limits; adjust the limits
					float diff[2];
					diff[0] = m_joints[op[1]].limits[0][i]-m_joints[op[1]].qext[i];
					diff[1] = m_joints[op[1]].limits[1][i]-m_joints[op[1]].qext[i];
					diff[0] -= (int)(diff[0]/(2*pi))*2*pi;
					diff[1] -= (int)(diff[1]/(2*pi))*2*pi;
					m_joints[op[1]].limits[isneg(fabs_tpl(diff[1])-fabs_tpl(diff[0]))][i] = m_joints[op[1]].qext[i];
				}
			}
		}
		if (params->pMtx0T) 
			params->q0 = quaternionf((matrix3x3Tf&)(*params->pMtx0T)/((matrix3x3Tf&)(*params->pMtx0T)).GetRow(0).len());
		if (params->pMtx0) 
			params->q0 = quaternionf((matrix3x3f&)(*params->pMtx0)/((matrix3x3f&)(*params->pMtx0)).GetRow(0).len());
		if (!is_unused(params->q0)) { 
			m_joints[op[1]].quat0 = params->q0; m_joints[op[1]].bQuat0Changed = 0; nChanges++; 
		}
		ENTITY_VALIDATE("CArticulatedEntity:SetParams(params_joint)",params);

		if (m_joints[op[1]].iParent!=op[0]) {
			if (!is_unused(params->qext[0])) {
				//(!(qparent*m_joints[op[1]].quat0)*qchild).GetEulerAngles_XYZ(m_joints[op[1]].q); //EULER_IVO 
				m_joints[op[1]].q = Ang3::GetAnglesXYZ( matrix3x3f(!(qparent*m_joints[op[1]].quat0)*qchild) );
				m_joints[op[1]].q -= m_joints[op[1]].qext;
			} else 
				//(!(qparent*m_joints[op[1]].quat0)*qchild).GetEulerAngles_XYZ(m_joints[op[1]].qext);	//EULER_IVO 
				m_joints[op[1]].qext = Ang3::GetAnglesXYZ( matrix3x3f(!(qparent*m_joints[op[1]].quat0)*qchild) );

			if (op[1]!=op[0]+1) {
				int nChildren;
				for(nChildren=m_joints[op[1]].nChildren+1,i=1; i<nChildren; nChildren+=m_joints[op[1]+i++].nChildren);
				ae_joint jbuf,*pjbuf=nChildren>1 ? new ae_joint[nChildren] : &jbuf;
				memcpy(pjbuf, m_joints+op[1], nChildren*sizeof(ae_joint));
				if (op[1]<op[0]) {
					for(i=1; i<nChildren; i++) pjbuf[i].iParent += op[0]-op[1]+1-nChildren;
					for(i=op[1]+nChildren; i<m_nJoints; i++) if (m_joints[i].iParent>op[1] && m_joints[i].iParent<=op[0]) 
						m_joints[i].iParent -= nChildren;
					memmove(m_joints+op[1], m_joints+op[1]+nChildren, (op[0]-op[1]-nChildren+1)*sizeof(ae_joint)); 
					op[0] -= nChildren;
				} else {
					for(i=1;i<nChildren;i++) pjbuf[i].iParent -= op[1]-op[0]-1;
					for(i=op[0]+1; i<op[1]; i++) if (m_joints[i].iParent>op[0]) m_joints[i].iParent += nChildren;
					memmove(m_joints+op[0]+1+nChildren, m_joints+op[0]+1, (op[1]-op[0]-1)*sizeof(ae_joint));
				}
				memcpy(m_joints+op[0]+1, pjbuf, nChildren*sizeof(ae_joint));
				if (pjbuf!=&jbuf) delete[] pjbuf;
				JointListUpdated();
			}
			op[1] = op[0]+1;
			m_joints[op[1]].iParent = op[0];
			m_joints[op[1]].q0 = m_joints[op[1]].q;
			if (op[0]>=0) {
				m_joints[op[0]].nChildren++;
				m_joints[op[0]+1].iLevel = m_joints[op[0]].iLevel+1;
			} else m_joints[op[0]+1].iLevel = 0;
			for(i=op[0]; i>=0; i=m_joints[i].iParent) 
				m_joints[i].nChildrenTree += m_joints[op[0]+1].nChildrenTree+1;
			if (params->op[0]==-1 && !m_bGrounded) {
				m_posPivot = m_joints[op[1]].body.pos;
				m_offsPivot = m_posPivot-m_pos;
			}
			(m_joints[op[1]].flags &= ~joint_expand_hinge) |= joint_expand_hinge & -m_bExpandHinges;
			MARK_UNUSED(m_joints[op[1]].hingePivot[0]);
			nChanges++;
		}	else {
			for(i=0;i<3;i++) if (!is_unused(params->q[i])) {
				m_joints[op[1]].q[i] = params->q[i]; nChanges++;
			}
			if (nChanges) {
				m_joints[op[1]].q0 = m_joints[op[1]].q;
				if (!params->bNoUpdate)
					for(i=op[1];i<=op[1]+m_joints[op[1]].nChildrenTree;i++) SyncBodyWithJoint(i);
			}
		}
		m_bUpdateBodies = 1;
		return 1;
	}

	if (_params->type==pe_params_articulated_body::type_id) {
		pe_params_articulated_body *params = (pe_params_articulated_body*)_params;
		if (!is_unused(params->pivot)) {
			vectorf offs = m_posPivot;
			m_offsPivot = params->pivot; m_posPivot = m_pos+m_offsPivot; offs = m_posPivot-offs;
			for(int i=0;i<m_nJoints;i++) m_joints[i].body.pos += offs;
		}
		if (!is_unused(params->bGrounded)) {
			m_bGrounded = params->bGrounded;
			if (m_nJoints>0 && m_joints[0].iParent==-1) {
				quaternionf qroot = m_joints[0].body.q*m_joints[0].body.qfb;
				if (m_bGrounded) {
					if (!is_unused(params->pivot)) {
						m_joints[0].pivot[1] = params->pivot-m_joints[0].body.pos;
						m_offsPivot -= m_joints[0].pivot[1]; m_posPivot += m_joints[0].pivot[1];
						m_joints[0].pivot[1] = m_joints[0].pivot[1]*qroot;
					}
				} else {
					m_joints[0].pivot[1] = qroot*m_joints[0].pivot[1];
					m_offsPivot -= m_joints[0].pivot[1]; m_posPivot -= m_joints[0].pivot[1];
					m_joints[0].pivot[1].Set(0,0,0);
					m_joints[0].flags &= ~all_angles_locked;
					m_joints[0].limits[0].Set(-1E15,-1E15,-1E15);
					m_joints[0].limits[1].Set(1E15,1E15,1E15);
					m_joints[0].ks.Set(0,0,0); m_joints[0].kd.Set(0,0,0);
					m_acc.Set(0,0,0); m_body.w.Set(0,0,0); m_wacc.Set(0,0,0);
				}
			}
			m_bIaReady = 0; m_simTime = 0; m_bAwake = 1;
		}
		if (!is_unused(params->v) && (params->v-m_body.v).len2()>0) { m_body.v = params->v; m_simTime = 0; m_bAwake = 1; }
		if (!is_unused(params->a) && (params->a-m_acc).len2()>0) { m_acc = params->a; m_simTime = 0; m_bAwake = 1; }
		if (!is_unused(params->w) && (params->w-m_body.w).len2()>0) { 
			if (m_bGrounded)
				m_body.w = params->w; 
			else if (m_bIaReady) {
				matrix3x3f basis_inv = (matrix3x3RMf&)m_joints[0].rotaxes[0];
				//m_joints[0].dq = basis_inv.invert()*m_body.w;
				basis_inv.Invert(); m_joints[0].dq = basis_inv*m_body.w;
			}
			m_simTime = 0; m_bAwake = 1;
		}
		if (!is_unused(params->wa) && (params->wa-m_wacc).len2()>0) { m_wacc = params->wa; m_simTime = 0; }
		if (!is_unused(params->scaleBounceResponse)) m_scaleBounceResponse = params->scaleBounceResponse;
		if (params->bApply_dqext) {
			for(int i=0;i<m_nJoints;i++)
				m_joints[i].dq += m_joints[i].dqext;
		}
		if (!is_unused(params->bAwake))
			if (!(m_bAwake = params->bAwake))
				m_simTime = 10.0f;
		int bRecalcPos = 0;
		if (!is_unused(params->pHost)) { 
			if (m_pHost) {
				m_pHost->Release(); m_pHost = 0;
			}
			if (params->pHost) {
				(m_pHost = ((CPhysicalPlaceholder*)params->pHost)->GetEntity())->AddRef(); 
				bRecalcPos = 1; 
			}
		}
		if (!is_unused(params->bInheritVel)) { m_bInheritVel = params->bInheritVel; bRecalcPos = 1; }
		if (!is_unused(params->posHostPivot)) { m_posHostPivot = params->posHostPivot; bRecalcPos = 1; }
		if (bRecalcPos)
			SyncWithHost(params->bRecalcJoints,0);
		if (!is_unused(params->bCheckCollisions)) m_bCheckCollisions = params->bCheckCollisions;
		if (!is_unused(params->bCollisionResp)) m_bCollisionResp = params->bCollisionResp;

		if (!is_unused(params->nCollLyingMode)) m_nCollLyingMode = params->nCollLyingMode;
		if (!is_unused(params->gravityLyingMode)) m_gravityLyingMode = params->gravityLyingMode;
		if (!is_unused(params->dampingLyingMode)) m_dampingLyingMode = params->dampingLyingMode;
		if (!is_unused(params->minEnergyLyingMode)) m_EminLyingMode = params->minEnergyLyingMode;
		if (!is_unused(params->iSimType)) m_iSimType = params->iSimType;
		if (!is_unused(params->iSimTypeLyingMode)) m_iSimTypeLyingMode = params->iSimTypeLyingMode;
		if (!is_unused(params->bExpandHinges)) m_bExpandHinges = params->bExpandHinges;

		return 1;
	}

	return 0;
}


int CArticulatedEntity::GetParams(pe_params *_params)
{
	int res = CRigidEntity::GetParams(_params);
	if (res)
		return res;

	if (_params->type==pe_params_joint::type_id) {
		pe_params_joint *params = (pe_params_joint*)_params;
		int i; for(i=0;i<m_nJoints && m_joints[i].idbody!=params->op[1];i++);
		if (i>=m_nJoints)
			return 0;
		params->flags = m_joints[i].flags;
		params->pivot = (m_joints[i].body.pos + (m_joints[i].body.q*m_joints[i].body.qfb)*m_joints[i].pivot[1] - m_pos)*m_qrot;
		params->q0 = m_joints[i].quat0;
		//if (params->pMtx0) m_joints[i].quat0.getmatrix( (matrix3x3f&)*params->pMtx0  );	//Q2M_IVO
		if (params->pMtx0) ((matrix3x3f&)*params->pMtx0) = matrix3x3f(m_joints[i].quat0);
		//if (params->pMtx0T) m_joints[i].quat0.getmatrix((matrix3x3Tf&)*params->pMtx0T);	//Q2M_IVO
		if (params->pMtx0T) ((matrix3x3Tf&)*params->pMtx0T) = matrix3x3f(m_joints[i].quat0);
		params->limits[0] = m_joints[i].limits[0];
		params->limits[1] = m_joints[i].limits[1];
		params->bounciness = m_joints[i].bounciness;
		params->ks = m_joints[i].ks;
		params->kd = m_joints[i].kd;
		params->q = m_joints[i].q;
		params->qext = m_joints[i].qext;
		params->op[0] = m_joints[i].iParent>=0 ? m_joints[m_joints[i].iParent].idbody : -1;
		return 1;
	}

	if (_params->type==pe_params_articulated_body::type_id) {
		pe_params_articulated_body *params = (pe_params_articulated_body*)_params;
		params->bGrounded = m_bGrounded;
		params->pivot = m_offsPivot;
		params->a = m_acc;
		params->wa = m_wacc;
		params->w = m_body.w;
		params->v = m_body.v;
		params->scaleBounceResponse = m_scaleBounceResponse;
		params->pHost = m_pHost;
		params->bInheritVel = m_bInheritVel;
		params->posHostPivot = m_posHostPivot;
		params->bCheckCollisions = m_bCheckCollisions;
		params->bCollisionResp = m_bCollisionResp;
		params->nCollLyingMode = m_nCollLyingMode;
		params->gravityLyingMode = m_gravityLyingMode;
		params->dampingLyingMode = m_dampingLyingMode;
		params->minEnergyLyingMode = m_EminLyingMode;
		params->iSimType = m_iSimType;
		params->iSimTypeLyingMode = m_iSimTypeLyingMode;
		params->bExpandHinges = m_bExpandHinges;
		return 1;
	}

	return 0;
}


int CArticulatedEntity::GetStatus(pe_status* _status)
{
	if (_status->type==pe_status_joint::type_id) {
		pe_status_joint *status = (pe_status_joint*)_status;
		int i; for(i=0;i<m_nJoints && m_joints[i].idbody!=status->idChildBody;i++);
		if (i>=m_nJoints) return 0;
		status->flags = m_joints[i].flags;
		status->q = m_joints[i].q;
		status->qext = m_joints[i].qext;
		status->dq = m_joints[i].dq;
		status->quat0 = m_joints[i].quat0;
		return 1;
	}

	if (_status->type==pe_status_dynamics::type_id) {
		int i; pe_status_dynamics *status = (pe_status_dynamics*)_status;
		if (!is_unused(status->partid))
			for(i=0; i<m_nParts && m_parts[i].id!=status->partid; i++);
		else i = status->ipart;
		if ((unsigned int)i>=(unsigned int)m_nParts) {
			m_body.pos.Set(0,0,0); m_body.M = 0;
			for(i=0;i<m_nJoints;i++) {
				m_body.pos += m_joints[i].body.pos*m_joints[i].body.M;
				m_body.M += m_joints[i].body.M;
			}
			m_body.pos /= m_body.M;
			return CRigidEntity::GetStatus(_status);
		}

		RigidBody *pbody = &m_joints[m_infos[i].iJoint].body;
		status->v = pbody->v;
		status->w = pbody->w;
		status->a = m_gravity + pbody->Fcollision*pbody->Minv;
		status->wa = pbody->Iinv*(pbody->Tcollision - (pbody->w^pbody->L));
		status->centerOfMass = pbody->pos;
		status->submergedFraction = m_submergedFraction;
		status->mass = pbody->M;
		return 1;
	}

	int res = CRigidEntity::GetStatus(_status);
	if (res && _status->type==pe_status_caps::type_id) {
		pe_status_caps *status = (pe_status_caps*)_status;
		status->bCanAlterOrientation = 1;
	}
	return res;
}


int CArticulatedEntity::Action(pe_action *_action)
{
	if (_action->type==pe_action_impulse::type_id) {
		pe_action_impulse *action = (pe_action_impulse*)_action;
		int i,j;
		if (m_bCollisionResp && action->impulse.len2()<sqr(m_body.M*0.04f) && action->iSource==0)
			return 1;
		if (is_unused(action->ipart))
			for(i=0; i<m_nParts && m_parts[i].id!=action->partid; i++);
		else i = action->ipart;
		ENTITY_VALIDATE("CArticulatedEntity:Action(action_impulse)",action);

		if (m_flags&pef_monitor_impulses && action->iSource==0 && m_pWorld->m_pEventClient)
			if (!m_pWorld->m_pEventClient->OnImpulse(this,m_pForeignData,m_iForeignData, action))
				return 1;

		box bbox; vectorf posloc;
		if ((unsigned int)i>=(unsigned int)m_nParts) {
			if (is_unused(action->point)) return 0;
			for(i=0;i<m_nParts;i++) {
				m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&bbox);
				posloc = (m_qrot*m_parts[i].q)*bbox.center + m_qrot*m_parts[i].pos + m_pos;
				posloc = bbox.Basis*((action->point-posloc)*(m_qrot*m_parts[i].q));
				for(j=0;j<3 && posloc[j]<bbox.size[j];j++);
				if (j==3) break;
			}
			if ((unsigned int)i>=(unsigned int)m_nParts) return 0;
		}

		j = m_infos[i].iJoint;
		vectorf P(zero),L(zero);
		if (!is_unused(action->impulse))
			P += action->impulse;
		if (!is_unused(action->momentum)) 
			L += action->momentum;
		else if (!is_unused(action->point)) {
			// make sure that the point is not too far from the part's bounding box
			m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&bbox);
			posloc = (m_qrot*m_parts[i].q)*bbox.center + m_qrot*m_parts[i].pos + m_pos;
			posloc = bbox.Basis*((action->point-posloc)*(m_qrot*m_parts[i].q));
			if (posloc.x<bbox.size.x*1.3f && posloc.y<bbox.size.y*1.3f && posloc.z<bbox.size.z*1.3f) {
				// recompute body's com, since sometimes only parts are in sync with animation, not joint bodies
				vectorf bodypos = m_parts[i].pos+m_pos-(m_parts[i].q*!m_infos[i].q0)*m_infos[i].pos0;
				L += action->point-bodypos ^ action->impulse;
			}
		}
		m_joints[j].Pimpact += P; m_joints[j].Limpact += L;

		if (action->iSource!=1) {
			m_bAwake = 1; m_simTime = 0;
			for(i=0;i<m_nJoints;i++) 
			if (m_iSimClass==1) {
				m_iSimClass = 2;	m_pWorld->RepositionEntity(this, 2);
			}
		}

		return 1;
	}

	if (_action->type==pe_action_reset::type_id) {
		for(int idx=0;idx<m_nJoints;idx++) {
			m_joints[idx].q.zero();
			if (m_joints[idx].bQuat0Changed)
				m_joints[idx].quat0.SetIdentity();
			m_joints[idx].dq.Set(0,0,0);
			m_joints[idx].body.v.Set(0,0,0); m_joints[idx].body.w.Set(0,0,0);
			m_joints[idx].body.P.Set(0,0,0); m_joints[idx].body.L.Set(0,0,0);
			SyncBodyWithJoint(idx);
		}
		ComputeBBox();
		m_pWorld->RepositionEntity(this,1);
		m_body.v.Set(0,0,0); m_body.w.Set(0,0,0);
		m_body.P.Set(0,0,0); m_body.L.Set(0,0,0);
		return CRigidEntity::Action(_action);
	}

	if (_action->type==pe_action_register_coll_event::type_id) {
		pe_action_register_coll_event *action = (pe_action_register_coll_event*)_action;
		m_iCollHistory = m_iCollHistory+1 & sizeof(m_CollHistory)/sizeof(m_CollHistory[0])-1;
		m_CollHistory[m_iCollHistory].pt = action->pt;
		m_CollHistory[m_iCollHistory].n = action->n;
		int i; for(i=0;i<m_nParts && m_parts[i].id!=action->partid[0];i++);
		RigidBody *pbody = &m_joints[m_infos[i].iJoint].body;
		m_CollHistory[m_iCollHistory].v[0] = pbody->v+(pbody->w^pbody->pos-action->pt);
		m_CollHistory[m_iCollHistory].v[1] = action->v;
		m_CollHistory[m_iCollHistory].mass[0] = pbody->M;
		m_CollHistory[m_iCollHistory].mass[1] = action->collMass;
		m_CollHistory[m_iCollHistory].idCollider = m_pWorld->GetPhysicalEntityId(action->pCollider);
		m_CollHistory[m_iCollHistory].partid[0] = action->partid[0];
		m_CollHistory[m_iCollHistory].partid[1] = action->partid[1];
		m_CollHistory[m_iCollHistory].idmat[0] = action->idmat[0];
		m_CollHistory[m_iCollHistory].idmat[1] = action->idmat[1];
		m_CollHistory[m_iCollHistory].age = 0;
		return 1;
	}

	if (_action->type==pe_action_set_velocity::type_id) {
		pe_action_set_velocity *action = (pe_action_set_velocity*)_action;
		if (m_nJoints<=0)
			return 0;

		if (!is_unused(action->v)) {
			pe_status_dynamics sd;
			GetStatus(&sd);

			pe_action_impulse ai;
			ai.ipart = m_joints[0].iStartPart;
			ai.point = sd.centerOfMass;
			ai.impulse = action->v*m_body.M;
			Action(&ai);
		}
		return 1;
	}

	return CRigidEntity::Action(_action);
}


int CArticulatedEntity::GetPotentialColliders(CPhysicalEntity **&pentlist)
{
	pentlist = m_pCollEntList; return m_nCollEnts;
}

int CArticulatedEntity::CheckSelfCollision(int ipart0,int ipart1)
{
	return m_joints[m_infos[ipart0].iJoint].selfCollMask & getmask(ipart1);
}

RigidBody *CArticulatedEntity::GetRigidBody(int ipart)
{
	return (unsigned int)ipart<(unsigned int)m_nParts ? &m_joints[m_infos[ipart].iJoint].body : &m_body;
}


void CArticulatedEntity::SyncWithHost(int bRecalcJoints, float time_interval)
{
	if (m_pHost) {
		if (m_pHost->m_iSimClass==7) {
			m_pHost = 0; return;
		}
		pe_status_pos sp;
		m_pHost->GetStatus(&sp);
		m_posPivot = sp.q*m_posHostPivot + sp.pos;
		m_pos = m_posPivot - m_offsPivot;
		pe_params_pos pp;
		pp.q = m_pHost->m_qrot;
		pp.bRecalcBounds = 0;
		SetParams(&pp);
		if (m_bInheritVel && time_interval>0) {
			pe_status_dynamics sd;
			m_pHost->GetStatus(&sd);
			float rdt = 1.0f/time_interval;
			m_acc = (sd.v-m_body.v)*rdt;
			m_wacc = (sd.w-m_body.w)*rdt;
			m_body.v = sd.v;
			m_body.w = sd.w;
		}
		if (bRecalcJoints)
			for(int i=0;i<m_nJoints;i++) SyncBodyWithJoint(i);
		m_pWorld->RepositionEntity(this,1);
	}
}


void CArticulatedEntity::UpdateJointRotationAxes(int idx)
{
	quaternionf q_parent = m_joints[idx].iParent>=0 ? m_joints[m_joints[idx].iParent].quat : m_qrot;
	//(q_parent*m_joints[idx].quat0).getmatrix((matrix3x3CMf&)m_joints[idx].rotaxes[0]); //Q2M_IVO 
	((matrix3x3CMf&)m_joints[idx].rotaxes[0]) = matrix3x3f(q_parent*m_joints[idx].quat0);

	float angle,cosa,sina;
	angle = m_joints[idx].q[1]+m_joints[idx].qext[1]; cosa = cos_tpl(angle); sina = sin_tpl(angle);
	m_joints[idx].rotaxes[0] = m_joints[idx].rotaxes[0].rotated(m_joints[idx].rotaxes[1], cosa,sina); // rot x around y - pitch
	angle = m_joints[idx].q[2]+m_joints[idx].qext[2]; cosa = cos_tpl(angle); sina = sin_tpl(angle);
	m_joints[idx].rotaxes[0] = m_joints[idx].rotaxes[0].rotated(m_joints[idx].rotaxes[2], cosa,sina); // rot x around z - yaw
	m_joints[idx].rotaxes[1] = m_joints[idx].rotaxes[1].rotated(m_joints[idx].rotaxes[2], cosa,sina); // rot y around z
}

void CArticulatedEntity::CheckForGimbalLock(int idx)
{
	int i;
	m_joints[idx].flags &= ~(angle0_gimbal_locked*7);
	if (!(m_joints[idx].flags & angle0_locked*5) && sqr(m_joints[idx].rotaxes[0]*m_joints[idx].rotaxes[2])>0.999f*0.999f) {
		if (!(m_joints[idx].flags & angle0_locked*7) && cry_fabsf(m_joints[idx].limits[1][0]-m_joints[idx].limits[0][0])>10 &&
				fabsf(m_joints[idx].limits[1][1]-m_joints[idx].limits[0][1])>10 && fabsf(m_joints[idx].limits[1][2]-m_joints[idx].limits[0][2])>10)
		{ // if joint is 3dof w/o limits, just rotate its quat0 to avoid gimbal lock
			//m_joints[idx].quat0 *= quaternionf(pi/6,m_joints[idx].rotaxes[1]);
			m_joints[idx].quat0 *= GetRotationAA((float)pi/6,vectorf(0,1,0));//(m_joints[idx].rotaxes[0]+m_joints[idx].rotaxes[1])*(1.0f/sqrt2));
			m_joints[idx].bQuat0Changed = 1;
			SyncJointWithBody(idx,3);
		} else {
			i = iszero((int)m_joints[idx].flags & angle0_limit_reached) << 1; // gimbal lock angle that already reached its limit, if this is the case
			m_joints[idx].flags |= angle0_gimbal_locked<<i;
			m_joints[idx].dq[i^2] += m_joints[idx].dq[i];
			m_joints[idx].dq[i] = 0;
			m_joints[idx].rotaxes[i] = m_joints[idx].rotaxes[i^2]*0.99f+m_joints[idx].rotaxes[1]*0.01f;
		}
	}
}

void CArticulatedEntity::SyncBodyWithJoint(int idx, int flags)
{
	vectorf pos_parent,pivot,wpivot;
	quaternionf q_parent,q;
	matrix3x3f R;
	int i;
	if (m_joints[idx].iParent>=0) {
		q_parent = m_joints[m_joints[idx].iParent].quat;
		pos_parent = m_joints[m_joints[idx].iParent].body.pos;
		pivot = pos_parent + q_parent*m_joints[idx].pivot[0];
	} else {
		q_parent = m_qrot; pos_parent = pivot = m_posPivot;
	}

	if (flags & 1) { // sync geometry
		UpdateJointRotationAxes(idx);

	//	m_joints[idx].quat = q_parent*m_joints[idx].quat0*quaternionf(m_joints[idx].q+m_joints[idx].qext);
		m_joints[idx].quat = q_parent*m_joints[idx].quat0*Quat::GetRotationXYZ(m_joints[idx].q+m_joints[idx].qext);
		m_joints[idx].body.q = m_joints[idx].quat*!m_joints[idx].body.qfb;
		m_joints[idx].body.pos = pos_parent + q_parent*m_joints[idx].pivot[0] - m_joints[idx].quat*m_joints[idx].pivot[1];
		//m_joints[idx].body.q.getmatrix(R); //Q2M_IVO
		R = matrix3x3f(m_joints[idx].body.q);
		m_joints[idx].body.Iinv = R*m_joints[idx].body.Ibody_inv*R.T();
		m_joints[idx].I = R*m_joints[idx].body.Ibody*R.T();
		//((m_joints[idx].body.Iinv=R)*=m_joints[idx].body.Ibody_inv)*=R.T();
		//((I=R)*=m_joints[idx].body.Ibody)*=R.T();

		for(i=m_joints[idx].iStartPart; i<m_joints[idx].iStartPart+m_joints[idx].nParts; i++) {
			m_parts[i].q = m_joints[idx].quat*m_infos[i].q0;
			m_parts[i].pos = m_joints[idx].quat*m_infos[i].pos0+m_joints[idx].body.pos-m_pos;
		}
	}

	if (flags & 2) { // sync velocities
		if (m_joints[idx].iParent>=0) {
			m_joints[idx].body.w = m_joints[m_joints[idx].iParent].body.w;
			m_joints[idx].body.v = m_joints[m_joints[idx].iParent].body.v + 
				(m_joints[m_joints[idx].iParent].body.w^m_joints[idx].body.pos-m_joints[m_joints[idx].iParent].body.pos);
		} else {
			m_joints[idx].body.v = m_body.v;
			if (m_bGrounded) {
				m_joints[idx].body.w = m_body.w;
				m_joints[idx].body.v += m_body.w^m_joints[idx].body.pos-m_posPivot;
			}	else m_joints[idx].body.w.Set(0,0,0);
		}

		for(i=0,wpivot.Set(0,0,0); i<3; i++)
			wpivot += m_joints[idx].rotaxes[i]*m_joints[idx].dq[i];
		m_joints[idx].body.v += wpivot^m_joints[idx].body.pos-pivot;
		m_joints[idx].body.w += wpivot;
		m_joints[idx].body.P = m_joints[idx].body.v*m_joints[idx].body.M;
		m_joints[idx].body.L = m_joints[idx].I*m_joints[idx].body.w;
	}
}

void CArticulatedEntity::SyncJointWithBody(int idx, int flags)
{
	if (flags & 1) { // sync angles
		quaternionf qparent = m_joints[idx].iParent>=0 ? m_joints[m_joints[idx].iParent].quat : m_qrot;
		m_joints[idx].quat = m_joints[idx].body.q*m_joints[idx].body.qfb;
		//(!(qparent*m_joints[idx].quat0)*m_joints[idx].quat).GetEulerAngles_XYZ(m_joints[idx].q); //EULER_IVO
		m_joints[idx].q = Ang3::GetAnglesXYZ( matrix3x3f(!(qparent*m_joints[idx].quat0)*m_joints[idx].quat) );
		m_joints[idx].q -= m_joints[idx].qext;
		UpdateJointRotationAxes(idx);
		CheckForGimbalLock(idx);
	}

	if (flags & 2) { // sync velocities
		vectorf wrel = m_joints[idx].iParent>=0 ? m_joints[m_joints[idx].iParent].body.w : m_body.w;
		wrel = m_joints[idx].body.w - wrel;
		if (!(m_joints[idx].flags & (angle0_locked|angle0_gimbal_locked)*7)) {
			matrix3x3f Basis_inv = (matrix3x3RMf&)m_joints[idx].rotaxes[0];
			//m_joints[idx].dq = wrel*Basis_inv.invert();	// IVO
			Basis_inv.Invert(); m_joints[idx].dq = wrel*Basis_inv;
		} else {
			int i,j,iAxes[3];
			for(i=j=0;i<3;i++) if (!(m_joints[idx].flags & (angle0_locked|angle0_gimbal_locked)<<i))
				iAxes[j++] = i;
			m_joints[idx].dq.Set(0,0,0);
			if (j==1)
				m_joints[idx].dq[iAxes[0]] = m_joints[idx].rotaxes[iAxes[0]]*wrel;
			else if (j==2) {
				if (!(m_joints[idx].flags & (angle0_locked|angle0_gimbal_locked)<<1)) {
					// only x and z axes can be nonorthogonal, so just project wrel on active axes in other cases
					m_joints[idx].dq[iAxes[0]] = m_joints[idx].rotaxes[iAxes[0]]*wrel;
					m_joints[idx].dq[iAxes[1]] = m_joints[idx].rotaxes[iAxes[1]]*wrel;
				} else {
					vectorf axisz,axisy;
					axisz = (m_joints[idx].rotaxes[iAxes[0]]^m_joints[idx].rotaxes[iAxes[1]]).normalized();
					axisy = axisz ^ m_joints[idx].rotaxes[iAxes[0]];
					m_joints[idx].dq[iAxes[1]] = (axisy*wrel)/(axisy*m_joints[idx].rotaxes[iAxes[1]]);
					m_joints[idx].dq[iAxes[0]] = wrel*m_joints[idx].rotaxes[iAxes[0]] - 
						(m_joints[idx].rotaxes[iAxes[0]]*m_joints[idx].rotaxes[iAxes[1]])*m_joints[idx].dq[iAxes[1]];
				}
			}
		}
	}
}


float CArticulatedEntity::CalcEnergy(float time_interval)
{
	float E=0,vmax=0,Emax=m_body.M*sqr(m_pWorld->m_vars.maxVel)*2; 
	int i,j; vectorf v;
	for(i=0; i<m_nJoints; i++) {
		v.zero();
		if (time_interval>0) for(j=0; j<NMASKBITS && getmask(j)<=m_joints[i].iContacts; j++) if (m_joints[i].iContacts & getmask(j)) {
			v += m_pContacts[j].n*max(0.0f,max(0.0f,m_pContacts[j].vreq*m_pContacts[j].n-max(0.0f,m_pContacts[j].vrel*m_pContacts[j].n))-v*m_pContacts[j].n);
			if (m_pContacts[j].pbody[1]->Minv==0 && m_pContacts[j].pent[1]->m_iSimClass>1) {
				RigidBody *pbody = m_pContacts[j].pbody[1];
				vmax = max(vmax,(pbody->v + (pbody->w^pbody->pos-m_pContacts[j].pt[1])).len2());
				Emax = m_body.M*sqr(10.0f)*2; // since will allow some extra energy, make sure we restrict the upper limit
			}
		}
		E += m_joints[i].body.M*(m_joints[i].body.v+v).len2() + m_joints[i].body.L*m_joints[i].body.w;
	}

	if (time_interval>0) {
		masktype constraint_mask;
		v.Set(0,0,0);
		for(i=0,constraint_mask=0; i<m_nColliders; constraint_mask |= m_pColliderConstraints[i++]);
		for(i=0; i<NMASKBITS && getmask(i)<=constraint_mask; i++) 
		if (constraint_mask & getmask(i) && (m_pConstraints[i].flags & contact_constraint_3dof || m_pConstraintInfos[i].flags & constraint_rope))	{
			v += m_pConstraints[i].n*max(0.0f,max(0.0f,m_pConstraints[i].vreq*m_pConstraints[i].n-max(0.0f,m_pConstraints[i].vrel*m_pConstraints[i].n))-
				m_pConstraints[i].n*v);
			if (m_pConstraints[i].pbody[1]->Minv==0 && m_pConstraints[i].pent[1]->m_iSimClass>1) {
				RigidBody *pbody = m_pConstraints[i].pbody[1];
				vmax = max(vmax,(pbody->v + (pbody->w^pbody->pos-m_pConstraints[i].pt[1])).len2());
			}
		}
		E += m_body.M*(v.len2()+vmax);
		return min_safe(E,Emax);
	}

	return E;
}


float CArticulatedEntity::GetDamping(float time_interval)
{
	float damping = CRigidEntity::GetDamping(time_interval);
	if (m_nContacts>=m_nCollLyingMode)
		damping = min(1.0f-m_dampingLyingMode*time_interval, damping);
	return damping;
}


int CArticulatedEntity::IsAwake(int ipart)
{
	return (unsigned int)ipart<(unsigned int)m_nParts ? m_joints[m_infos[ipart].iJoint].bAwake : m_bAwake;
}


int CArticulatedEntity::GetUnprojAxis(int idx, vectorf &axis)
{
	axis.Set(0,0,0);
	if (!(m_joints[idx].flags & (angle0_limit_reached|angle0_locked|angle0_gimbal_locked)*6)) // y and z axes are not locked
		return 1;
	else {
		int i; for(i=2; i>=0 && m_joints[idx].flags & (angle0_limit_reached|angle0_locked|angle0_gimbal_locked)<<i; i--);
		if (i>=0)	{
			axis = m_joints[idx].rotaxes[i]; // free axis from list: z y x
			return 1;
		}
	}
	return 0;
}


inline masktype getContactMask(int iContact) { return iContact>=0 ? getmask(iContact) : 0; }

int CArticulatedEntity::StepJoint(int idx, float time_interval,masktype &contact_mask,int &bBounced,int bFlying)
{
	int i,j,idxpivot,ncont,itmax,curidx,sgq,bSelfColl;
	float qlim[2],dq,curq,e,diff[2],tdiff, minEnergy = m_nContacts>=m_nCollLyingMode ? m_EminLyingMode : m_Emin;
	vectorf q,n,ptsrc,ptdst,axis,axis0,pivot,prevpt;
	quaternionf qrot;
	matrix3x3f R;
	geom_contact *pcontacts;
	geom_world_data gwd;
	intersection_params ip;
	box bbox;

	m_joints[idx].prev_q = m_joints[idx].q;
	m_joints[idx].prev_dq = m_joints[idx].dq;
	m_joints[idx].prev_pos = m_joints[idx].body.pos;
	m_joints[idx].prev_qrot = m_joints[idx].body.q;
	m_joints[idx].prev_v = m_joints[idx].body.v;
	m_joints[idx].prev_w = m_joints[idx].body.w;
	m_joints[idx].bHasExtContacts = 0;

	if (m_iSimTypeCur && m_bCollisionResp) {
		e = (m_joints[idx].body.v.len2() + (m_joints[idx].body.L*m_joints[idx].body.w)*m_joints[idx].body.Minv)*0.5f + m_joints[idx].body.Eunproj;
		m_bAwake += (m_joints[idx].bAwake = isneg(minEnergy-e));
		m_joints[idx].bAwake |= m_bFloating|m_bUsingUnproj;
		if (!m_joints[idx].bAwake) {
			for(i=m_joints[idx].iParent; i>=0; i=m_joints[i].iParent)
				m_joints[idx].bAwake |= m_joints[i].bAwake;
		}

		if (m_joints[idx].bAwake) {
			m_joints[idx].body.Step(time_interval);
			SyncJointWithBody(idx,3);
		}
		//m_joints[idx].body.q.getmatrix(R); //Q2M_IVO
		R = matrix3x3f(m_joints[idx].body.q);
		m_joints[idx].I = R*m_joints[idx].body.Ibody*R.T();
		m_joints[idx].dv_body.Set(0,0,0); m_joints[idx].dw_body.Set(0,0,0);

		m_joints[idx].flags &= ~(angle0_limit_reached*7);
		q = m_joints[idx].q + m_joints[idx].qext;
		for(i=0;i<3;i++) if (!(m_joints[idx].flags & (angle0_locked|angle0_gimbal_locked)<<i)) {
			qlim[0] = m_joints[idx].limits[0][i]; qlim[1] = m_joints[idx].limits[1][i];
			if (isneg(qlim[0]-q[i]) + isneg(q[i]-qlim[1]) + isneg(qlim[1]-qlim[0]) < 2) {
				m_joints[idx].flags |= angle0_limit_reached<<i;
				diff[0] = q[i]-qlim[0]; tdiff = diff[0]-sgn(diff[0])*(2*pi);
				if (fabsf(tdiff)<fabsf(diff[0])) 
					diff[0] = tdiff;
				diff[1] = q[i]-qlim[1]; tdiff = diff[1]-sgn(diff[1])*(2*pi);
				if (fabsf(tdiff)<fabsf(diff[1]))
					diff[1] = tdiff;
				m_joints[idx].dq_limit[i] = diff[isneg(fabsf(diff[1])-fabsf(diff[0]))];
			}
		}

		for(i=m_joints[idx].iStartPart; i<m_joints[idx].iStartPart+m_joints[idx].nParts; i++) {
			(m_parts[i].q = m_joints[idx].quat*m_infos[i].q0).Normalize();
			m_parts[i].pos = m_joints[idx].quat*m_infos[i].pos0+m_joints[idx].body.pos-m_pos;
		}
	}	
	else 
	{
		for(i=0;i<3;i++) if (!(m_joints[idx].flags & (angle0_locked|angle0_gimbal_locked)<<i)) {
			if (fabsf(m_joints[idx].dq[i])<1E-5f && fabsf(m_joints[idx].q[i])<0.01f) 
				m_joints[idx].dq[i] = 0;
			qlim[0] = m_joints[idx].limits[0][i]; qlim[1] = m_joints[idx].limits[1][i];
			//qlim[1] += 2*pi*isneg(qlim[1]-qlim[0]);
			if (m_joints[idx].limits[0][i] > m_joints[idx].limits[1][i]) {
				sgq = sgn(m_joints[idx].q[i]+m_joints[idx].qext[i]);
				qlim[sgq+1>>1] += (sgq<<1)*pi;
			}
			dq = m_joints[idx].dq[i]*time_interval;	sgq = sgn(dq);
			curq = m_joints[idx].q[i]+m_joints[idx].qext[i];
			if ((curq + dq - qlim[sgq+1>>1])*sgq > 0)	{	// we'll breach the limit that lies ahead along movement (ignore the other one)
				dq = qlim[sgq+1>>1]-curq+sgq*0.01f;
				dq = sgnnz(dq)*min(fabsf(dq),0.1f+(m_bCollisionResp^1)); // limit angle snapping in full simulation mode
				m_joints[idx].dq_limit[i] = m_joints[idx].dq[i];
				m_joints[idx].flags |= angle0_limit_reached<<i;
				m_joints[idx].dq_req[i] = -m_joints[idx].dq[i]*m_joints[idx].bounciness[i];
				bBounced++;
			} else if (sgq!=0) {
				m_joints[idx].flags &= ~(angle0_limit_reached<<i);
				// if angle passes through 0 during this step, make it snap to 0 and also clamp dq if it's small enough
				if (m_joints[idx].ks[i]!=0 && (m_joints[idx].q[i]+dq)*m_joints[idx].q[i]<0) {
					dq = -m_joints[idx].q[i]; 
					if (fabs_tpl(m_joints[idx].dq[i])<0.1f)
						m_joints[idx].dq[i] = 0;
				}
			}
			m_joints[idx].q[i] += min(fabsf(dq),1.2f)*sgn(dq);
			//while(m_joints[idx].q[i]>pi) m_joints[idx].q[i]-=2*pi;
			//while(m_joints[idx].q[i]<-pi) m_joints[idx].q[i]+=2*pi;
		}	else 
			m_joints[idx].dq[i] = 0;

		m_joints[idx].body.Step(time_interval);
		m_joints[idx].dv_body = m_joints[idx].body.v;
		m_joints[idx].dw_body = m_joints[idx].body.w;
		SyncBodyWithJoint(idx);
		m_joints[idx].dv_body = m_joints[idx].body.v-m_joints[idx].dv_body;
		m_joints[idx].dw_body = m_joints[idx].body.w-m_joints[idx].dw_body;
		m_joints[idx].ddq.Set(0,0,0);

		e = (m_joints[idx].body.v.len2() + (m_joints[idx].body.L*m_joints[idx].body.w)*m_joints[idx].body.Minv)*0.5f;
		m_bAwake += (m_joints[idx].bAwake = isneg(minEnergy-e));
		m_joints[idx].bAwake |= m_bFloating;

		if (!m_joints[idx].bAwake && m_bCollisionResp) {
			for(i=m_joints[idx].iParent; i>=0; i=m_joints[i].iParent)
				m_joints[idx].bAwake |= m_joints[i].bAwake;
			if (!m_joints[idx].bAwake) {
				m_joints[idx].q = m_joints[idx].prev_q;
				m_joints[idx].dq.Set(0,0,0);
				SyncBodyWithJoint(idx);
				m_joints[idx].body.P.Set(0,0,0); m_joints[idx].body.L.Set(0,0,0);
				m_joints[idx].body.v.Set(0,0,0); m_joints[idx].body.w.Set(0,0,0);
				if (idx==0) {
					m_pos = m_prev_pos;	m_posPivot = m_pos+m_offsPivot;
					m_body.v.zero();
				}
			}
		}

		CheckForGimbalLock(idx);
	}

	// force persistant contacts to lie in contact plane
	pivot = m_joints[idx].iParent<0 ? m_posPivot : 
		m_joints[m_joints[idx].iParent].body.pos + m_joints[m_joints[idx].iParent].quat*m_joints[idx].pivot[0];
	/*if (m_joints[idx].iContacts) {
		for(i=0; !(m_joints[idx].iContacts & getmask(i)); i++);
		n = m_pContacts[i].pbody[m_pContacts[i].iNormal]->q*m_pContacts[i].nloc;
		e = m_pWorld->m_vars.maxContactGap*0.5f;
		ptsrc = m_pContacts[i].pbody[0]->q*m_pContacts[i].ptloc[0] + m_pContacts[i].pbody[0]->pos;
		ptdst = m_pContacts[i].pbody[1]->q*m_pContacts[i].ptloc[1] + m_pContacts[i].pbody[1]->pos + n*e;
		if (idx==0)
			m_posPivot += n*(n*(ptdst-ptsrc)+e);
		else {
			axis = ptsrc-pivot ^ n*(n*(ptdst-ptsrc));
			for(i=0;i<3;i++) if (m_joints[idx].flags & (angle0_limit_reached|angle0_locked|angle0_gimbal_locked)<<i)
				axis -= m_joints[idx].rotaxes[i]*(axis*m_joints[idx].rotaxes[i]);
			axis.normalize();
			angle = RotatePointToPlane(ptsrc, axis,pivot, n,ptdst+n*e);
			if (angle<0.1f) {
				qrot = quaternionf(angle, axis);
				m_joints[idx].body.q = qrot*m_joints[idx].body.q; 
				m_joints[idx].body.pos = pivot + qrot*(m_joints[idx].body.pos-pivot);
			}
		}
	}*/

	if (m_bCheckCollisions) {// && m_joints[idx].bAwake) {
		// check for new contacts; unproject if necessary; register new contacts
		if (/*m_iSimTypeCur==0 &&*/ bFlying) {
			ip.iUnprojectionMode = 0; ip.time_interval = time_interval*2;
			gwd.v = m_body.v; gwd.w.zero();
		} else if (m_iSimTypeCur==0 && m_joints[idx].nChildren==0 && (m_joints[idx].body.w.len2()>sqr(6) || m_joints[idx].body.v.len2()>sqr(4)) &&
			m_joints[idx].body.Minv>m_body.Minv*10.0f) 
		{
			ip.iUnprojectionMode = 1; ip.centerOfRotation = pivot;
			ip.time_interval = 0.6f;
			if (!GetUnprojAxis(idx,ip.axisOfRotation)) {
				ip.iUnprojectionMode = 0; ip.vrel_min = 1E10;
			}
		}	else {
			ip.iUnprojectionMode = 0; ip.vrel_min = 1E10; ip.bNoAreaContacts = true;
		}
		idxpivot = idx;	itmax = -1; 
		ip.bStopAtFirstTri = /*m_timeStepFull>m_maxAllowedStep ||*/ m_joints[idx].body.M<m_body.M*0.2f && m_joints[idx].nChildren>0; 
		//if (m_joints[idx].iParent>=0)
		ip.ptOutsidePivot[0] = m_joints[idx].iParent>=0 ? pivot : m_joints[idx].body.pos;
		ncont = CheckForNewContacts(&gwd,&ip, itmax, m_joints[idx].iStartPart,m_joints[idx].nParts);
		pcontacts = ip.pGlobalContacts;
		/*if (ncont>0 && itmax<0 && ip.iUnprojectionMode==1) {
			// find parent that has good free axes
			axis0 = (pcontacts[0].center-ip.centerOfRotation^pcontacts[0].dir).normalized();
			for(i=3,idxpivot=m_joints[idxpivot].iParent; i>0 && idxpivot>0; i--,idxpivot=m_joints[idxpivot].iParent) {
				for(i=0,axis=axis0;i<3;i++) if (m_joints[idxpivot].flags & (angle0_limit_reached|angle0_locked|angle0_gimbal_locked)<<i)
					axis -= m_joints[idxpivot].rotaxes[i]*(axis*m_joints[idx].rotaxes[i]);
				if (axis.len2()>0.5f*0.5f) {
					ip.centerOfRotation = m_joints[idxpivot].body.pos + m_joints[idxpivot].quat*m_joints[idxpivot].pivot[1];
					ip.axisOfRotation = GetUnprojAxis(idxpivot);
					break;
				}
			}
			if (i*idxpivot>0)
				ncont = CheckForNewContacts(&gwd,&ip, itmax, m_joints[idx].iStartPart,m_joints[idx].nParts);
		}*/
		if (itmax>=0) {
			if (pcontacts[itmax].iUnprojMode) {
				//qrot = quaternionf(pcontacts[itmax].t, pcontacts[itmax].dir);
				qrot.SetRotationAA(pcontacts[itmax].t, pcontacts[itmax].dir);
				for(i=idxpivot;i<=idx;i++) { // note: update of parent will possibly invalidate its registered contacts, is it crucial?
					m_joints[i].body.q = qrot*m_joints[i].body.q; 
					m_joints[i].body.pos = ip.centerOfRotation + qrot*(m_joints[i].body.pos-ip.centerOfRotation);
					SyncJointWithBody(i,3);
				}
				if (!m_bCollisionResp) {
					for(i=0;i<3;i++) 
					if (!(m_joints[idxpivot].flags & (angle0_locked|angle0_gimbal_locked|angle0_limit_reached)<<i) && 
								m_joints[idxpivot].rotaxes[i]*pcontacts[itmax].dir*sgnnz(m_joints[idxpivot].dq[i])<-0.5f)
						m_joints[idxpivot].dq[i] = 0;
				}
			} else {
				axis = pcontacts[itmax].dir*pcontacts[itmax].t;
				for(i=0;i<=idx;i++) m_joints[i].body.pos += axis;
				m_posPivot += axis; m_pos += axis;
			}
		}/*	else if (m_joints[idx].iContacts) {
			SyncJointWithBody(idx); SyncBodyWithJoint(idx,2);
		}*/


		if (m_bCollisionResp) {
			// register new contacts
			j = m_joints[idx].iStartPart;
			m_parts[j].pPhysGeomProxy->pGeom->GetBBox(&bbox);
			if (m_nParts>3)
				m_parts[j].minContactDist = max(max(bbox.size.x,bbox.size.y),bbox.size.z)*m_parts[j].scale*0.4f;

			for(i=0;i<ncont;i++) if (pcontacts[i].vel==0) { 
				bSelfColl = g_CurColliders[i]==this;
				e = sqr(m_parts[m_joints[idx].iStartPart].minContactDist);
				if (bSelfColl)
					e *= sqr(1.5f);
				j = 0;
				if (idx>0)
					for(;j<pcontacts[i].nborderpt && (pcontacts[i].ptborder[j]-pcontacts[i].ptborder[0]).len2()<e; j++);
				if (j==pcontacts[i].nborderpt/*idx>0*/) // register border center only for penetration contacts in marginal joints
					contact_mask |= m_joints[idx].iContacts |= getContactMask(RegisterContactPoint(contact_mask,i, pcontacts[i].center, pcontacts, 
						pcontacts[i].iPrim[0],pcontacts[i].iFeature[0], pcontacts[i].iPrim[1],pcontacts[i].iFeature[1], contact_new, pcontacts[i].t));
				else {
					//m_parts[m_joints[idx].iStartPart].pPhysGeomProxy->pGeom->GetBBox(&bbox);
					//e = sqr(max(max(bbox.size.x,bbox.size.y),bbox.size.z)*0.2f);
					for(j=0,prevpt.zero(); j<pcontacts[i].nborderpt; j++)	if ((pcontacts[i].ptborder[j]-prevpt).len2()>e) {
						contact_mask |= m_joints[idx].iContacts |= getContactMask(RegisterContactPoint(contact_mask,i, pcontacts[i].ptborder[j], pcontacts, 
							pcontacts[i].iPrim[0],pcontacts[i].iFeature[0], pcontacts[i].iPrim[1],pcontacts[i].iFeature[1], contact_new, pcontacts[i].t));
						prevpt = pcontacts[i].ptborder[j];
					}
				}
				if (!bSelfColl)
					m_maxPenetrationCur = max(m_maxPenetrationCur,(float)pcontacts[i].t);
			} else if (pcontacts[i].parea) { 
				// register only points that are sufficiently far away from each other
				m_parts[m_joints[idx].iStartPart].pPhysGeomProxy->pGeom->GetBBox(&bbox);
				e = sqr(max(max(bbox.size.x,bbox.size.y),bbox.size.z)*0.8f*(iszero(idx)^1)); prevpt.zero();
				for(j=0;j<pcontacts[i].parea->npt;j++) if ((pcontacts[i].parea->pt[j]-prevpt).len2()>e) {
					contact_mask |= m_joints[idx].iContacts |= getContactMask(RegisterContactPoint(contact_mask,i, pcontacts[i].parea->pt[j], pcontacts, 
						pcontacts[i].parea->piPrim[0][j], pcontacts[i].parea->piFeature[0][j], 
						pcontacts[i].parea->piPrim[1][j],pcontacts[i].parea->piFeature[1][j]));
					prevpt = pcontacts[i].parea->pt[j];
				}
			} else
				contact_mask |= m_joints[idx].iContacts |= getContactMask(RegisterContactPoint(contact_mask,i, pcontacts[i].pt, pcontacts, 
					pcontacts[i].iPrim[0],pcontacts[i].iFeature[0], pcontacts[i].iPrim[1],pcontacts[i].iFeature[1]));
		}
	}

	// iterate children
	for(i=0,curidx=idx+1; i<m_joints[idx].nChildren; i++)
		curidx = StepJoint(curidx, time_interval,contact_mask,bBounced,bFlying);
	return curidx;
}


float CArticulatedEntity::GetMaxTimeStep(float time_interval)
{
	return CRigidEntity::GetMaxTimeStep(time_interval);
}

int CArticulatedEntity::Step(float time_interval)
{
	int i,j,bBounced=0;
	masktype contact_mask=0;
	vectorf Y_vec[2]; vectornf Y(6,Y_vec[0]);
	float Iabuf[39]; matrixf Ia(6,6,0,_align16(Iabuf)); 
	vectorf Za_vec[2]; vectornf Za(6,Za_vec[0]);
	vectorf dv,dw;
	vectorf gravity;
	int bWasAwake = m_bAwake;
	float maxPenetrationPrev = m_maxPenetrationCur;
	box bbox;
	if (m_nContacts>=m_nCollLyingMode) {
		gravity = m_gravityLyingMode;
		m_iSimTypeCur = m_iSimTypeLyingMode;
	} else {
		gravity = m_nContacts ? m_gravity : m_gravityFreefall;
		m_iSimTypeCur = m_iSimType | m_bFloating;
	}

	for(i=m_nColliders-1;i>=0;i--) {
		if (m_pColliders[i]->m_iSimClass==7)
			RemoveCollider(m_pColliders[i]);
		else
			contact_mask |= m_pColliderContacts[i];
	}
	if (m_pHost && m_pHost->m_iSimClass==7)
		m_pHost = 0;
	if (m_bUpdateBodies & m_iSimTypeCur)
		for(i=0;i<m_nJoints;i++)
			SyncBodyWithJoint(i);
	m_bUpdateBodies = 0;

	if (m_timeStepPerformed > m_timeStepFull-0.001f)
		return 1;
	m_timeStepPerformed += time_interval;
	for(i=0;i<sizeof(m_CollHistory)/sizeof(m_CollHistory[0]);i++)
		m_CollHistory[i].age += time_interval;
	m_dampingEx = 0;

	SyncWithHost(0,time_interval);

	if (!m_bAwake && !m_bCheckCollisions)
		return 1;

	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );
	PHYS_ENTITY_PROFILER

	ComputeBBox();
	if (m_bCheckCollisions) {
		vectorf sz = m_BBox[1]-m_BBox[0];
		float szmax = max(max(sz.x,sz.y),sz.z)*0.3f;
		if (m_body.v.len2()*sqr(time_interval) > szmax)
			szmax = m_body.v.len()*time_interval;
		sz.Set(szmax,szmax,szmax);
		m_nCollEnts = m_pWorld->GetEntitiesAround(m_BBox[0]-sz,m_BBox[1]+sz, m_pCollEntList, 
			ent_terrain|ent_static|ent_sleeping_rigid|ent_rigid|ent_living|ent_independent|ent_sort_by_mass|ent_triggers, this);

		for(i=0;i<m_nCollEnts;i++) if (m_pCollEntList[i]->m_iSimClass>2 && m_pCollEntList[i]->GetType()!=PE_ARTICULATED)
			m_pCollEntList[i]->Awake();

		if (m_iSimClass<=2) {
			for(i=j=0;i<m_nCollEnts;i++) 
			if (m_pCollEntList[i]==this || m_pCollEntList[i]->m_iSimClass<=2 && (m_pCollEntList[i]->m_iGroup==m_iGroup && m_pCollEntList[i]->m_bMoved || 
					!m_pCollEntList[i]->IsAwake() || m_pWorld->m_pGroupNums[m_pCollEntList[i]->m_iGroup]<m_pWorld->m_pGroupNums[m_iGroup]))
				m_pCollEntList[j++] = m_pCollEntList[i];
			m_nCollEnts = j;
		} else
			m_nCollEnts = 0;
	} else 
		m_nCollEnts = 0;

	for(i=m_bFastLimbs=0; i<m_nJoints; i++) 
		m_bFastLimbs |= isneg(sqr(0.4f)-m_joints[i].body.w.len2()*sqr(time_interval));
	/*{
		m_parts[m_joints[i].iStartPart].pPhysGeomProxy->pGeom->GetBBox(&bbox);
		r = sqr(max(max(bbox.size.x,bbox.size.y),bbox.size.z)*m_parts[m_joints[i].iStartPart].scale);
		m_bFastLimbs |= isneg(r-(m_joints[i].body.v.len2()+m_joints[i].body.w.len2()*r)*sqr(time_interval));
	}*/
	m_iSimTypeCur &= m_bFastLimbs^1;
	m_iSimTypeCur |= m_iSimTypeOverride;

	m_qrot.SetIdentity();
	m_bPartPosForced = 0;
	m_prev_pos = m_pos;
	m_prev_vel = m_body.v;
	if (!m_bGrounded)
		if (m_iSimTypeCur==0)
			m_posPivot += m_body.v*time_interval;
		else
			m_posPivot = m_joints[0].body.pos;
	m_pos = m_posPivot - m_offsPivot;
	m_bAwake = isneg(m_simTime-2.5f) | (iszero(m_nContacts) & (m_bGrounded^1) & (m_bFloating^1));
	m_simTime += time_interval;
	m_simTimeAux += time_interval;
	m_maxPenetrationCur = 0;
	StepJoint(0, time_interval, contact_mask,bBounced, iszero(m_nContacts));
	m_bAwake = isneg(-m_bAwake);
	m_nContacts = m_nDynContacts = 0;

	if (m_bCollisionResp)	{
		contact_mask = VerifyExistingContacts(m_pWorld->m_vars.maxContactGap);
		ApplyBuoyancy(time_interval,gravity);
		for(i=0;i<m_nJoints;i++) {
			m_joints[i].body.w -= m_joints[i].dw_body;
			m_joints[i].body.v -= m_joints[i].dv_body-gravity*time_interval;
			m_joints[i].body.P = m_joints[i].body.v*m_joints[i].body.M;
			m_joints[i].body.L = m_joints[i].I*m_joints[i].body.w;
			if (m_joints[i].Pimpact.len2()>0 || m_joints[i].Limpact.len2()>0) {
				m_joints[i].body.P += m_joints[i].Pimpact; 
				m_joints[i].body.L += m_joints[i].Limpact; 
				m_joints[i].body.v = m_joints[i].body.P*m_joints[i].body.Minv;
				m_joints[i].body.w = m_joints[i].body.Iinv*m_joints[i].body.L;
				m_joints[i].Pimpact.zero(); m_joints[i].Limpact.zero();
			}
			m_joints[i].iContacts &= contact_mask;
			for(j=0;j<NMASKBITS && getmask(j)<=m_joints[i].iContacts; j++) 
			if (getmask(j) & m_joints[i].iContacts && m_infos[m_pContacts[j].ipart[0]].iJoint!=i)
				m_joints[i].iContacts &= ~getmask(j);
		}

		ComputeBBox();
		m_pWorld->RepositionEntity(this,1);
		m_bAwake = bWasAwake;
		m_bSteppedBack = 0;

		// do not request step back if we were in deep penetration state initially
		i = m_iSimTypeCur | isneg(m_maxPenetrationCur-0.07f) | isneg(0.07f-maxPenetrationPrev);
		if (!i)
			m_simTimeAux = 0;
		return i | isneg(3-m_nStepBackCount);
	}

	if (!m_joints[0].fs)
		for(i=0;i<m_nJoints;i++) {
			m_joints[i].fs = (featherstone_data*)_align16(m_joints[i].fsbuf = new char[sizeof(featherstone_data)+16]);
			m_joints[i].fs->Q.zero(); m_joints[i].fs->Ya_vec[0].zero(); m_joints[i].fs->Ya_vec[1].zero();
#if defined(LINUX)
			((matrix3x3RMf&)(*reinterpret_cast<matrix3x3RMf*>(&(m_joints[i].fs->qinv)))).SetIdentity();
#else
			((matrix3x3RMf&)m_joints[i].fs->qinv).SetIdentity();
#endif
		}

	if (bBounced && m_bIaReady) {	// apply bounce impulses at joints that reached their limits
		bBounced=0; CollectPendingImpulses(0,bBounced);
		if (bBounced) {
			PropagateImpulses(dv = m_M0inv*-m_Ya_vec[0]);
			if (!m_bGrounded)
				m_body.v += dv;
		}
	}

	if (m_simTime>10) {
		for(i=0;i<m_nJoints;i++) {
			m_joints[i].q.zero(); m_joints[i].dq.zero();
		}
		m_bAwake = 0;
	}

	if (!m_bAwake)
		return 1;

	CalcBodyIa(0, Ia);
	m_M0inv = (matrix3x3in6x6f&)Ia.data[3];
	matrix3x3f M0host; M0host.SetZero();
	if (m_bGrounded) {
		if (m_pHost && m_bExertImpulse) {
			float Ihost_inv[6][6]; // TODO: support full spatial matrices if the host provides them
			memset(Ihost_inv, 0, sizeof(float)*36);
			m_pHost->GetSpatialContactMatrix(m_posPivot, -1, Ihost_inv);
			M0host = (matrix3x3in6x6f&)Ihost_inv[3][0];
		}
		if (M0host.IsZero())
			m_M0inv.SetZero();
		else {
			//(m_M0inv += M0host.invert()).invert();
			M0host.Invert(); (m_M0inv+=M0host).Invert();
		} 
	} else 
		m_M0inv.Invert();

	m_bIaReady = 1;
	Za.zero(); CalcBodyZa(0, time_interval,Za);
	CalcBodiesIinv(m_bCollisionResp);

	for(i=0;i<m_nJoints;i++) { // apply external impulses
		m_joints[i].Pext += m_joints[i].Pimpact;	m_joints[i].Lext += m_joints[i].Limpact;
		m_joints[i].Pimpact.zero();	m_joints[i].Limpact.zero();
	}
	CollectPendingImpulses(0,bBounced);

	dv = m_M0inv*(-Za_vec[0]-m_Ya_vec[0]); dw.zero(); 
	if (m_bGrounded)
		dv += m_acc*time_interval; dw += m_wacc*time_interval;
	CalcVelocityChanges(time_interval, dv,dw);
	if (!m_bGrounded)	{
		m_body.v += dv; 
		m_body.w.zero();
	}
	for(i=0;i<m_nJoints;i++)
		SyncBodyWithJoint(i,3);	// synchronize body geometry as well as dynamics to accomodate potential changes in joint limits

	if (m_bGrounded && m_pHost && m_bExertImpulse) {
		pe_action_impulse ai;
		ai.impulse = -Za_vec[0]-m_Ya_vec[0];
		ai.point = m_posPivot;
		m_pHost->Action(&ai);
	}

	ComputeBBox();
	m_pWorld->RepositionEntity(this,1);

	return isneg(m_timeStepFull-m_timeStepPerformed-0.001f);
}


void CArticulatedEntity::StepBack(float time_interval)
{
	if (m_simTime>0) {
		matrix3x3f R;
		m_pos = m_prev_pos;
		m_posPivot = m_pos+m_offsPivot;
		m_body.P = (m_body.v=m_prev_vel)*m_body.M;

		for(int i=0;i<m_nJoints;i++) {
			m_joints[i].q = m_joints[i].prev_q;
			m_joints[i].dq = m_joints[i].prev_dq;
			m_joints[i].body.pos = m_joints[i].prev_pos;
			m_joints[i].body.q = m_joints[i].prev_qrot;
			m_joints[i].body.v = m_joints[i].prev_v;
			m_joints[i].body.w = m_joints[i].prev_w;

			m_joints[i].quat = m_joints[i].body.q*m_joints[i].body.qfb;
			R = matrix3x3f(m_joints[i].body.q);
			m_joints[i].I = R*m_joints[i].body.Ibody*R.T();
			m_joints[i].body.Iinv = R*m_joints[i].body.Ibody_inv*R.T();
			m_joints[i].body.P = m_joints[i].body.v*m_joints[i].body.M; 
			m_joints[i].body.L = m_joints[i].I*m_joints[i].body.w;

			UpdateJointRotationAxes(i);
			for(int j=m_joints[i].iStartPart; j<m_joints[i].iStartPart+m_joints[i].nParts; j++) {
				m_parts[j].q = m_joints[i].quat*m_infos[j].q0;
				m_parts[j].pos = m_joints[i].quat*m_infos[j].pos0+m_joints[i].body.pos-m_pos;
			}
			//SyncBodyWithJoint(i);
		}
		m_bSteppedBack = 1;

		//UpdateContactsAfterStepBack(time_interval);
	}
}


void CArticulatedEntity::JointListUpdated()
{
	int i,j;
	for(i=0;i<m_nParts;i++) {
		for(j=0;j<m_nJoints && m_joints[j].idbody!=m_infos[i].idbody;j++);
		m_infos[i].iJoint = j;
	}
	for(i=0;i<m_nJoints;i++)
		m_joints[i].iLevel = m_joints[i].iParent>=0 ? m_joints[m_joints[i].iParent].iLevel+1 : 0;
}


int CArticulatedEntity::GetStateSnapshot(CStream &stm,float time_back,int flags)
{
	stm.WriteNumberInBits(SNAPSHOT_VERSION,4);
	stm.Write((unsigned char)m_nJoints);
	stm.Write(m_pos);
	stm.Write(m_body.v);
	stm.Write(m_bAwake!=0);
	int i = m_iSimClass-1;
	stm.WriteNumberInBits(i,2);

#ifdef DEBUG_BONES_SYNC
	stm.WriteBits((BYTE*)&m_qrot,sizeof(m_qrot)*8);
	for(i=0;i<m_nParts;i++) {
		stm.Write(m_parts[i].pos);
		stm.WriteBits((BYTE*)&m_parts[i].q,sizeof(m_parts[i].q)*8);
	}
	return 1;
#endif

	if (m_bPartPosForced) for(i=0;i<m_nJoints;i++) {
		int j = m_joints[i].iStartPart;
		m_joints[i].quat = m_qrot*m_parts[j].q*!m_infos[j].q0;
		m_joints[i].body.pos = m_parts[j].pos-m_joints[i].quat*m_infos[j].pos0+m_pos;
		m_joints[i].body.q = m_joints[i].quat*!m_joints[i].body.qfb;
	}

	for(i=0;i<m_nJoints;i++) {
		stm.Write(m_joints[i].bAwake!=0);
		stm.Write(m_joints[i].q);
		stm.Write(m_joints[i].dq);

		stm.Write(m_joints[i].qext);
		stm.Write(m_joints[i].body.pos);
		stm.WriteBits((BYTE*)&m_joints[i].body.q,sizeof(m_joints[i].body.q)*8);
		if (m_joints[i].bQuat0Changed) {
			stm.Write(true);
			stm.WriteBits((BYTE*)&m_joints[i].quat0, sizeof(m_joints[i].quat0)*8);
		} else stm.Write(false);
		if (m_joints[i].body.P.len2()+m_joints[i].body.L.len2()>0) {
			stm.Write(true); 
			stm.Write(m_joints[i].body.P);
			stm.Write(m_joints[i].body.L);
		}	else stm.Write(false);
		WritePacked(stm,m_joints[i].iContacts);
	}

	WriteContacts(stm,flags);

	return 1;
}

int CArticulatedEntity::SetStateFromSnapshot(CStream &stm, int flags)
{
	int i=0,j,ver=0; 
	bool bnz;
	matrix3x3f R;
	unsigned char nJoints;

	stm.ReadNumberInBits(ver,4);
	if (ver!=SNAPSHOT_VERSION)
		return 0;
	stm.Read(nJoints);
	if (nJoints!=m_nJoints)
		return 0;

	if (!(flags & ssf_no_update)) {
		stm.Read(m_pos);
		stm.Read(m_body.v);
		stm.Read(bnz); m_bAwake = bnz ? 1:0;
		stm.ReadNumberInBits(i, 2); m_iSimClass=i+1;

#ifdef DEBUG_BONES_SYNC
stm.ReadBits((BYTE*)&m_qrot,sizeof(m_qrot)*8);
for(i=0;i<m_nParts;i++) {
	stm.Read(m_parts[i].pos);
	stm.ReadBits((BYTE*)&m_parts[i].q,sizeof(m_parts[i].q)*8);
}
#else
		for(i=0;i<m_nJoints;i++) {
			stm.Read(bnz);
			m_joints[i].bAwake = bnz ? 1:0;
			stm.Read(m_joints[i].q);
			stm.Read(m_joints[i].dq);
			//SyncBodyWithJoint(i);

			stm.Read(m_joints[i].qext);
			stm.Read(m_joints[i].body.pos);
			stm.ReadBits((BYTE*)&m_joints[i].body.q,sizeof(m_joints[i].body.q)*8);
			stm.Read(bnz); if (bnz)	{
				stm.ReadBits((BYTE*)&m_joints[i].quat0,sizeof(m_joints[i].quat0)*8);
				m_joints[i].bQuat0Changed = 1;
			} else m_joints[i].bQuat0Changed = 0;
			stm.Read(bnz); if (bnz)	{
				stm.Read(m_joints[i].body.P);
				stm.Read(m_joints[i].body.L);
			}	else {
				m_joints[i].body.P.zero();
				m_joints[i].body.L.zero();
			}
			ReadPacked(stm,m_joints[i].iContacts);
			m_body.UpdateState();

			UpdateJointRotationAxes(i);
			m_joints[i].quat = m_joints[i].body.q*m_joints[i].body.qfb;
			//m_joints[i].body.q.getmatrix(R); //Q2M_IVO
			R = matrix3x3f(m_joints[i].body.q);
			m_joints[i].I = R*m_joints[i].body.Ibody*R.T();

			for(j=m_joints[i].iStartPart; j<m_joints[i].iStartPart+m_joints[i].nParts; j++) {
				m_parts[j].q = m_joints[i].quat*m_infos[j].q0;
				m_parts[j].pos = m_joints[i].quat*m_infos[j].pos0+m_joints[i].body.pos-m_pos;
			}
		}
#endif

		m_bUpdateBodies = 0;
		ComputeBBox();
		m_pWorld->RepositionEntity(this);
	} else {
		masktype dummy;
		stm.Seek(stm.GetReadPos()+2*sizeof(vectorf)*8+3);
#ifdef DEBUG_BONES_SYNC
stm.Seek(stm.GetReadPos()+sizeof(quaternionf)*8+m_nParts*(sizeof(quaternionf)+sizeof(vectorf))*8);
#else
		for(i=0;i<m_nJoints;i++) {
			stm.Seek(stm.GetReadPos()+1+4*sizeof(vectorf)*8+sizeof(quaternionf)*8);
			stm.Read(bnz); if (bnz) 
				stm.Seek(stm.GetReadPos()+sizeof(quaternionf)*8);
			stm.Read(bnz); if (bnz) 
				stm.Seek(stm.GetReadPos()+2*sizeof(vectorf)*8);
			ReadPacked(stm,dummy);
		}
#endif
	}

#ifndef DEBUG_BONES_SYNC
	ReadContacts(stm,flags);
	masktype contact_mask = 0;
	for(i=0;i<m_nColliders;i++) contact_mask |= m_pColliderContacts[i];
	for(i=0;i<m_nJoints;i++) m_joints[i].iContacts &= contact_mask;
#endif

	return 1;
}


int CArticulatedEntity::CalcBodyZa(int idx, float time_interval, vectornf &Za_change)
{
	int i,j,curidx,nextidx,idir;
	float kd,tlim,qdashpot,dq_dashpot,k,qbuf[3]; 
	vectornf Q(3,m_joints[idx].fs->Q),Qtmp(3,qbuf);
	vectorf d,r,pos_parent,Qscale;
	quaternionf q_parent;
	vectorf Za_vec[2]; vectornf Za(6,Za_vec[0]);

	if (m_joints[idx].iParent>=0) {
		pos_parent = m_joints[m_joints[idx].iParent].body.pos;
		q_parent = m_joints[m_joints[idx].iParent].quat;
	} else {
		pos_parent = m_posPivot; q_parent = m_qrot;
	}
	d = m_joints[idx].body.pos - (pos_parent + q_parent*m_joints[idx].pivot[0]);
	r = m_joints[idx].body.pos - pos_parent;

	if (m_joints[idx].flags & joint_no_gravity)
		Za_vec[0].zero();
	else
		Za_vec[0] = m_gravity*-m_joints[idx].body.M*time_interval;
	Za_vec[1].zero() = (m_joints[idx].body.w^m_joints[idx].I*m_joints[idx].body.w)*time_interval;
	Za_vec[0] += m_joints[idx].dv_body*m_joints[idx].body.M;
	Za_vec[1] += m_joints[idx].I*m_joints[idx].dw_body;
	m_joints[idx].fs->Q.zero();
	m_joints[idx].ddq.zero();

	if (!(m_joints[idx].flags & joint_isolated_accelerations)) for(j=0;j<m_joints[idx].nPotentialAngles;j++) 
		Qscale[j] = 1.0f/m_joints[idx].fs->qinv[j*(m_joints[idx].nPotentialAngles+1)];
	else Qscale.Set(1.0f,1.0f,1.0f);

	// calculate torque vector and spatial joint axes
	for(j=0;j<m_joints[idx].nPotentialAngles;j++) {
		i = m_joints[idx].fs->axidx2qidx[j];
		kd = fabsf(m_joints[idx].dq[i])>15.0f ? 0 : m_joints[idx].kd[i];
		k = time_interval*Qscale[j];
		Q[j] = (m_joints[idx].ks[i]*m_joints[idx].q[i] + kd*m_joints[idx].dq[i])*-k;
		// if the joint is approaching limit (but haven't breached it yet) and is in dashpot area, damp the velocity
		if (fabsf(m_joints[idx].dq[i])>0.5f) {
			idir = isneg(m_joints[idx].dq[i])^1;
			tlim = m_joints[idx].q[i]-m_joints[idx].limits[idir][i];
			if (tlim<-pi) tlim += 2*pi;
			if (tlim>pi) tlim -= 2*pi;
			tlim *= idir*2-1;	qdashpot = m_joints[idx].qdashpot[i];
			dq_dashpot = min(fabsf(m_joints[idx].dq[i])*Qscale[j], (qdashpot-tlim)*m_joints[idx].kdashpot[i]*isneg(fabsf(tlim-2*qdashpot)-qdashpot)*k);
			Q[j] += dq_dashpot*(1-idir*2);
		}
	}
	if (!(m_joints[idx].flags & (angle0_locked|angle0_gimbal_locked)*5)) { // take into accout that x and z axes influence one another
		Q[m_joints[idx].fs->qidx2axidx[0]] += Q[m_joints[idx].fs->qidx2axidx[2]]*(m_joints[idx].rotaxes[0]*m_joints[idx].rotaxes[2]);
		Q[m_joints[idx].fs->qidx2axidx[2]] += Q[m_joints[idx].fs->qidx2axidx[0]]*(m_joints[idx].rotaxes[0]*m_joints[idx].rotaxes[2]);
	}
	if (m_joints[idx].flags & joint_isolated_accelerations)
		m_joints[idx].ddq = m_joints[idx].fs->Q;

	vectorf Za_child_vec[2]; vectornf Za_child(6,Za_child_vec[0]);
	// iterate all children, accumulate Za
	for(i=0,curidx=idx+1; i<m_joints[idx].nChildren; i++) {
		nextidx = CalcBodyZa(curidx, time_interval, Za_child);
		Za += Za_child;	Za_vec[1] += Za_child_vec[0] ^ m_joints[idx].body.pos-m_joints[curidx].body.pos;
		curidx = nextidx;
	}

	// calculate Za changes for the parent
	Za_change = Za;
	vectornf ddq(m_joints[idx].nActiveAngles, m_joints[idx].ddq);
	if (m_joints[idx].nActiveAngles>0) {
		matrixf sT(Q.len = m_joints[idx].nActiveAngles,6,0, m_joints[idx].fs->s_vec[0][0]);
		matrixf qinv_down(m_joints[idx].nActiveAngles,m_joints[idx].nActiveAngles,0, m_joints[idx].fs->qinv_down);
		matrixf Ia_s(6,m_joints[idx].nActiveAngles,0, m_joints[idx].fs->Ia_s);
		if (!(m_joints[idx].flags & joint_isolated_accelerations))
			ddq = qinv_down*((Qtmp=Q)-=sT*Za);
		Za_change += Ia_s*ddq;
	}

	if (m_joints[idx].nActiveAngles<m_joints[idx].nPotentialAngles && !(m_joints[idx].flags & joint_isolated_accelerations)) {
		matrixf sT(Q.len = m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->s_vec[0][0]);
		matrixf qinv(m_joints[idx].nPotentialAngles,m_joints[idx].nPotentialAngles,0, m_joints[idx].fs->qinv);
		matrixf qinv_sT(m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->qinv_sT[0]);
		ddq = qinv*((Qtmp=Q)-=sT*Za);
	}

	return curidx;
}


int CArticulatedEntity::CalcBodyIa(int idx, matrixf& Ia_change)
{
	int i,j,curidx,nextidx;
	vectorf d,r,pos_parent;
	quaternionf q_parent;

	if (m_joints[idx].iParent>=0) {
		pos_parent = m_joints[m_joints[idx].iParent].body.pos;
		q_parent = m_joints[m_joints[idx].iParent].quat;
	} else {
		pos_parent = m_posPivot; q_parent = m_qrot;
	}
	d = m_joints[idx].body.pos - (pos_parent + q_parent*m_joints[idx].pivot[0]);
	r = m_joints[idx].body.pos - pos_parent;

	// fill initial Ia
	matrixf Ia(6,6,0,m_joints[idx].fs->Ia[0]); Ia.zero();
	(matrix3x3in6x6f&)m_joints[idx].fs->Ia[0][3] = vectorf(m_joints[idx].body.M,m_joints[idx].body.M,m_joints[idx].body.M);
	(matrix3x3in6x6f&)m_joints[idx].fs->Ia[3][0] = m_joints[idx].I;

	// sort axes so that active angles' axes come first
	for (i=j=0;i<3;i++) if (!(m_joints[idx].flags & (angle0_locked|angle0_limit_reached|angle0_gimbal_locked)<<i)) {
		m_joints[idx].fs->qidx2axidx[i] = j; m_joints[idx].fs->axidx2qidx[j++] = i;
	} m_joints[idx].nActiveAngles = j;
	for (i=0;i<3;i++) if ((m_joints[idx].flags & (angle0_locked|angle0_limit_reached)<<i) == angle0_limit_reached<<i) {
		m_joints[idx].fs->qidx2axidx[i] = j; m_joints[idx].fs->axidx2qidx[j++] = i;
	} m_joints[idx].nPotentialAngles = j;

	for(j=0;j<m_joints[idx].nPotentialAngles;j++) {
		i = m_joints[idx].fs->axidx2qidx[j];
		m_joints[idx].fs->s_vec[j][0] = m_joints[idx].rotaxes[i]^d; 
		m_joints[idx].fs->s_vec[j][1] = m_joints[idx].rotaxes[i];
	}
	
	float buf[39]; matrixf Ia_child(6,6,0,_align16(buf));
	// iterate all children, accumulate Ia
	for(i=0,curidx=idx+1; i<m_joints[idx].nChildren; i++) {
		nextidx = CalcBodyIa(curidx, Ia_child);
		r = m_joints[idx].body.pos-m_joints[curidx].body.pos;
		LeftOffsetSpatialMatrix(Ia_child, -r);
		RightOffsetSpatialMatrix(Ia_child, r);
		Ia += Ia_child;
		curidx = nextidx;
	}

	// calculate Ia changes for the parent
	Ia_change = Ia;
	matrixf sT(m_joints[idx].nActiveAngles,6,0, m_joints[idx].fs->s_vec[0][0]);
	matrixf s(6,m_joints[idx].nActiveAngles,0, m_joints[idx].fs->s);
	matrixf Ia_s_qinv_sT(6,6,0, m_joints[idx].fs->Ia_s_qinv_sT[0]);
	matrixf qinv(m_joints[idx].nActiveAngles,m_joints[idx].nActiveAngles,0, m_joints[idx].fs->qinv);
	if (m_joints[idx].nActiveAngles>0) {
		SpatialTranspose(sT, s);
		matrixf Ia_s(6,m_joints[idx].nActiveAngles,0, m_joints[idx].fs->Ia_s);
		matrixf qinv_down(m_joints[idx].nActiveAngles,m_joints[idx].nActiveAngles,0, m_joints[idx].fs->qinv_down);
		matrixf Ia_s_qinv(6,m_joints[idx].nActiveAngles,0, buf);
		Ia_s = Ia*s; qinv = (qinv_down = sT*Ia_s).invert();
		Ia_s_qinv_sT = (Ia_s_qinv = Ia_s*qinv)*sT;
		Ia_change -= Ia_s_qinv_sT*Ia;
		Ia_s_qinv_sT *= -1.0f;
	} else Ia_s_qinv_sT.zero();
	for(i=0;i<6;i++) m_joints[idx].fs->Ia_s_qinv_sT[i][i] += 1;
	r = m_joints[idx].body.pos - pos_parent;
	LeftOffsetSpatialMatrix(Ia_s_qinv_sT, r);

	if (m_joints[idx].nPotentialAngles>0) {
		// precalculate matrices for velocity propagation up - use limited angles also 
		matrixf qinv_sT(m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->qinv_sT[0]);
		matrixf qinv_sT_Ia(m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->qinv_sT_Ia[0]);
		matrixf sT_Ia(m_joints[idx].nPotentialAngles,6,0, buf);
		if (m_joints[idx].nActiveAngles < m_joints[idx].nPotentialAngles) {
			sT.nRows = m_joints[idx].nPotentialAngles; SpatialTranspose(sT, s);
			(qinv = (sT_Ia = sT*Ia)*s).invert();
		}
		(qinv_sT = qinv*sT)*=-1.0f; qinv_sT_Ia = qinv_sT*Ia;
		//matrixf(6,6,0,m_joints[idx].s_qinv_sT_Ia[0]) = s*qinv_sT_Ia;
	}

	return curidx;
}


int CArticulatedEntity::CollectPendingImpulses(int idx,int &bNotZero)
{
	int i,j,curidx,newidx,bChildNotZero;
	bNotZero = 0;

	for(i=0;i<3;i++) if (!is_unused(m_joints[idx].dq_req[i])) {
		// note that articulation matrices from the previous frame are used, while rotaxes are already new; this introduces slight inconsistency
		//matrix3x3bf K; GetJointTorqueResponseMatrix(idx, K);
		//float resp = m_joints[idx].rotaxes[i]*K*m_joints[idx].rotaxes[i];
		float buf[39],buf1[39]; matrixf Mresp(6,6,0,_align16(buf)),MrespLoc(6,6,0,_align16(buf1));
		MrespLoc = matrixf(6,6,0,m_joints[idx].fs->Ia_s_qinv_sT[0]);
		for(j=0;j<6;j++) MrespLoc[j][j] -= 1.0f;
		Mresp = matrixf(6,6,0,m_joints[m_joints[idx].iParent].fs->Iinv[0])*MrespLoc;
		LeftOffsetSpatialMatrix(Mresp,m_joints[m_joints[idx].iParent].body.pos-m_joints[idx].body.pos);
		j = m_joints[idx].fs->qidx2axidx[i];
		vectorf axis = m_joints[idx].rotaxes[i], dw = (matrix3x3in6x6f&)Mresp[0][3]*axis, dv = (matrix3x3in6x6f&)Mresp[3][3]*axis;
		float resp = -(vectorf(m_joints[idx].fs->qinv_sT[j]+3)*axis);
		resp -= vectorf(m_joints[idx].fs->qinv_sT_Ia[j])*dw + vectorf(m_joints[idx].fs->qinv_sT_Ia[j]+3)*dv;

		if (resp>0.1f*m_body.Minv) {
			resp = (m_joints[idx].dq_req[i]-m_joints[idx].dq[i])/resp*m_scaleBounceResponse;
			m_joints[idx].fs->Ya_vec[1] -= m_joints[idx].rotaxes[i]*resp;
			if (m_joints[idx].iParent>=0)
				m_joints[m_joints[idx].iParent].fs->Ya_vec[1] += m_joints[idx].rotaxes[i]*resp;
			bNotZero++;
		}
	}

	if (bNotZero += isneg(1E-6-m_joints[idx].Pext.len2()-m_joints[idx].Lext.len2())) {
		m_joints[idx].fs->Ya_vec[0] -= m_joints[idx].Pext; m_joints[idx].Pext.zero();
		m_joints[idx].fs->Ya_vec[1] -= m_joints[idx].Lext; m_joints[idx].Lext.zero();
	}

	for(i=0,curidx=idx+1; i<m_joints[idx].nChildren; i++) {
		newidx = CollectPendingImpulses(curidx, bChildNotZero);
		if (bChildNotZero) {
			matrixf Ia_s_qinv_sT(6,6,0,m_joints[curidx].fs->Ia_s_qinv_sT[0]);
			vectornf Ya(6,m_joints[idx].fs->Ya_vec[0]);
			vectornf Ya_child(6,m_joints[curidx].fs->Ya_vec[0]);
			Ya += Ia_s_qinv_sT*Ya_child;
			bNotZero += bChildNotZero;
		}
		curidx = newidx;
	}
	if (idx==0)
	{
#if defined(LINUX)
		vectornf(6,(float*)(m_Ya_vec[0])) = ::operator*((const matrix_tpl<float> &)matrixf(6,6,0,m_joints[0].fs->Ia_s_qinv_sT[0]), (const vectorn_tpl<float>&)vectornf(6,m_joints[0].fs->Ya_vec[0]));
#else
		vectornf(6,m_Ya_vec[0]) = matrixf(6,6,0,m_joints[0].fs->Ia_s_qinv_sT[0])*vectornf(6,m_joints[0].fs->Ya_vec[0]);
#endif
	}
	return curidx;
}


void CArticulatedEntity::PropagateImpulses(const vectorf &dv, int bLockLimits)
{
	int idx,i,j,bHitsLimit;
	vectorf pos_parent;

	for(idx=0; idx<m_nJoints; idx++) {
		vectornf dv_spat(6, m_joints[idx].fs->dv_vec[0]);
		if (m_joints[idx].iParent>=0) {
			m_joints[idx].fs->dv_vec[0] = m_joints[m_joints[idx].iParent].fs->dv_vec[0];
			m_joints[idx].fs->dv_vec[1] = m_joints[m_joints[idx].iParent].fs->dv_vec[1];
			pos_parent = m_joints[m_joints[idx].iParent].body.pos;
		} else {
			m_joints[idx].fs->dv_vec[0].zero(); m_joints[idx].fs->dv_vec[1] = dv; pos_parent = m_posPivot;
		}
		m_joints[idx].fs->dv_vec[1] += m_joints[idx].fs->dv_vec[0] ^ m_joints[idx].body.pos-pos_parent;

		if (m_joints[idx].nPotentialAngles>0) {
			vectornf ddq(m_joints[idx].nPotentialAngles, m_joints[idx].ddq);
			matrixf qinv_sT_Ia(m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->qinv_sT_Ia[0]);
			matrixf qinv_sT(m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->qinv_sT[0]);
			vectornf Ya(6,m_joints[idx].fs->Ya_vec[0]);
			matrixf s(6,m_joints[idx].nPotentialAngles,0, m_joints[idx].fs->s);
			ddq = qinv_sT_Ia*dv_spat;
			ddq += qinv_sT*Ya;

			for(i=0; i<m_joints[idx].nPotentialAngles; i++) {
				j = m_joints[idx].fs->axidx2qidx[i]; 
				if (!is_unused(m_joints[idx].dq_req[j])) {
					m_joints[idx].ddq[i] = m_joints[idx].dq_req[j]-m_joints[idx].dq[j];
					MARK_UNUSED m_joints[idx].dq_req[j];
				} else if (m_joints[idx].flags & angle0_limit_reached<<j) {
					bHitsLimit = isneg(-m_joints[idx].ddq[i]*m_joints[idx].dq_limit[j]);
					if (bHitsLimit || bLockLimits) 
						m_joints[idx].ddq[i] = 0;	// do not accelerate angle that reached its limit
					if (!bHitsLimit && bLockLimits)
						m_joints[idx].flags &= ~(angle0_limit_reached<<j);
				}
				m_joints[idx].dq[j] += m_joints[idx].ddq[i];
			}
			
			dv_spat += s*ddq;
		}
		m_joints[idx].fs->Ya_vec[0].zero(); m_joints[idx].fs->Ya_vec[1].zero();
	}
}


void CArticulatedEntity::CalcVelocityChanges(float time_interval, const vectorf &dv,const vectorf &dw)
{
	int idx,i,j;
	vectorf pos_parent;

	for(idx=0; idx<m_nJoints; idx++) {
		vectornf dv_spat(6, m_joints[idx].fs->dv_vec[0]);
		if (m_joints[idx].iParent>=0) {
			m_joints[idx].fs->dv_vec[0] = m_joints[m_joints[idx].iParent].fs->dv_vec[0];
			m_joints[idx].fs->dv_vec[1] = m_joints[m_joints[idx].iParent].fs->dv_vec[1];
			pos_parent = m_joints[m_joints[idx].iParent].body.pos;
		} else {
			m_joints[idx].fs->dv_vec[0] = dw; m_joints[idx].fs->dv_vec[1] = dv; pos_parent = m_posPivot;
		}
		m_joints[idx].fs->dv_vec[1] += m_joints[idx].fs->dv_vec[0] ^ m_joints[idx].body.pos-pos_parent;

		if (m_joints[idx].nPotentialAngles>0) {
			vectornf ddq(m_joints[idx].nPotentialAngles, m_joints[idx].ddq);
			matrixf qinv_sT(m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->qinv_sT[0]);
			vectornf Ya(6,m_joints[idx].fs->Ya_vec[0]);
			matrixf qinv_sT_Ia(m_joints[idx].nPotentialAngles,6,0, m_joints[idx].fs->qinv_sT_Ia[0]);
			matrixf s(6,m_joints[idx].nPotentialAngles,0, m_joints[idx].fs->s);

			ddq += qinv_sT_Ia*dv_spat;
			ddq += qinv_sT*Ya;

			for(i=0; i<m_joints[idx].nPotentialAngles; i++) {
				j = m_joints[idx].fs->axidx2qidx[i]; 
				if (m_joints[idx].flags & angle0_limit_reached<<j && m_joints[idx].ddq[i]*m_joints[idx].dq_limit[j]>0)
					m_joints[idx].ddq[i] = 0;	// do not accelerate angle that reached its limit
				m_joints[idx].dq[j] += m_joints[idx].ddq[i];
			}
			
			dv_spat += s*ddq;
		}
		m_joints[idx].fs->Ya_vec[0].zero(); m_joints[idx].fs->Ya_vec[1].zero();
	}
}


int CArticulatedEntity::OnRegisterContact(entity_contact *pcontact, int iop)
{
	if (pcontact->pent[iop^1]!=this) {
		m_nContacts++;
		m_nDynContacts -= -pcontact->pent[iop^1]->m_iSimClass >> 31;
		m_joints[m_infos[pcontact->ipart[iop]].iJoint].bHasExtContacts = 1;
	}
	// force simtype 1 in case of character-vehicle interaction
	//m_iSimTypeOverride |= inrange(pcontact->pbody[iop^1]->Minv,1E-8f,m_body.Minv*0.1f);
	return 1;
}


void CArticulatedEntity::CalcBodiesIinv(int bLockLimits)
{
	int idx,i,nAngles;
	float Iinv_buf[39],aux_buf[39],Mright_buf[39],s_buf[18],qinv_sT_buf[18];
	vectorf r,pos_parent;
	matrixf Iinv_parent(6,6,0,_align16(Iinv_buf)),Iaux(6,6,0,_align16(aux_buf));
	memset(Iinv_buf,0,sizeof(Iinv_buf));
	((matrix3x3in6x6f&)Iinv_parent.data[18] = m_M0inv) *=- 1.0f;

	for(idx=0; idx<m_nJoints; idx++) {
		if (m_joints[idx].iParent>=0) {
			Iinv_parent.data = m_joints[m_joints[idx].iParent].fs->Iinv[0];
			pos_parent = m_joints[m_joints[idx].iParent].body.pos;
		} else {
			Iinv_parent.data = _align16(Iinv_buf);
			pos_parent = m_posPivot;
		}
		matrixf Iinv(6,6,0,m_joints[idx].fs->Iinv[0]);
		matrixf s_qinv_sT_Ia(6,6,0, m_joints[idx].fs->s_qinv_sT_Ia[0]);
		r = m_joints[idx].body.pos-pos_parent;
		nAngles = bLockLimits ? m_joints[idx].nActiveAngles : m_joints[idx].nPotentialAngles;

		if (nAngles>0) {
			matrixf Mright(6,6,0,_align16(Mright_buf)),s(6,nAngles,0,m_joints[idx].fs->s);
			matrixf Ia(6,6,0, m_joints[idx].fs->Ia[0]);
			matrixf s_qinv_sT(6,6,0,m_joints[idx].fs->s_qinv_sT[0]);
			matrixf qinv_sT(nAngles,6,0,m_joints[idx].fs->qinv_sT[0]);

			if (nAngles!=m_joints[idx].nPotentialAngles) {
				matrixf sT(6,nAngles,0, m_joints[idx].fs->s_vec[0][0]);
				s.data = s_buf; SpatialTranspose(sT,s);
				qinv_sT.data = qinv_sT_buf;
				qinv_sT = matrixf(nAngles,nAngles,0,m_joints[idx].fs->qinv_down)*sT;
			}

			s_qinv_sT = s*qinv_sT; // already with '-' sign
			Iinv = s_qinv_sT*Ia; 
			for(i=0;i<6;i++) Iinv.data[i*7] += 1.0f;
			RightOffsetSpatialMatrix(Iinv, -r);
			s_qinv_sT_Ia = Iinv;

			//Mright = Iinv_parent;
			//LeftOffsetSpatialMatrix(Mright, -r);
			//RightOffsetSpatialMatrix(Mright, r);
			Iinv = (Iaux=Iinv)*Iinv_parent; //Mright;

			if (nAngles == m_joints[idx].nActiveAngles)
				Iinv = (Iaux=Iinv)*matrixf(6,6,0,m_joints[idx].fs->Ia_s_qinv_sT[0]);
			else {
				Mright = Ia*s_qinv_sT;
				for(i=0;i<6;i++) Mright.data[i*7] += 1.0f;
				LeftOffsetSpatialMatrix(Mright, r);
				Iinv = (Iaux=Iinv)*Mright;
			}
			Iinv += s_qinv_sT;
		} else {
			Iinv = Iinv_parent;
			LeftOffsetSpatialMatrix(Iinv, -r);
			RightOffsetSpatialMatrix(Iinv, r);
			s_qinv_sT_Ia.identity();
			RightOffsetSpatialMatrix(s_qinv_sT_Ia, -r);
		}
	}
}


void CArticulatedEntity::GetContactMatrix(const vectorf &pt, int ipart, matrix3x3f &K)
{
	m_joints[m_infos[ipart].iJoint].body.GetContactMatrix(pt-m_joints[m_infos[ipart].iJoint].body.pos,K);
}


void CArticulatedEntity::GetJointTorqueResponseMatrix(int idx, matrix3x3f &K)
{
	matrix3x3in6x6f& Lw((matrix3x3in6x6f&)m_joints[idx].fs->Iinv[0][3]);
	(K = Lw) *=- 1.0;
	if (m_joints[idx].iParent>=0) {
		matrix3x3in6x6f &ww_up((matrix3x3in6x6f&)*(matrix3x3in6x6f*)m_joints[idx].fs->s_qinv_sT_Ia[0]), 
										&vw_up((matrix3x3in6x6f&)m_joints[idx].fs->s_qinv_sT_Ia[0][3]),
										&LL_down((matrix3x3in6x6f&)m_joints[idx].fs->Ia_s_qinv_sT[3][3]), 
										&LP_down((matrix3x3in6x6f&)m_joints[idx].fs->Ia_s_qinv_sT[0][3]),
										&Lv_parent((matrix3x3in6x6f&)m_joints[m_joints[idx].iParent].fs->Iinv[3][3]), 
										&Lw_parent((matrix3x3in6x6f&)m_joints[m_joints[idx].iParent].fs->Iinv[0][3]),
										&Pw_parent((matrix3x3in6x6f&)m_joints[m_joints[idx].iParent].fs->Iinv[0][0]);
		//matrix3x3bf rmtx;
		//(m_joints[idx].body.pos-m_joints[m_joints[idx].iParent].body.pos).crossproduct_matrix(rmtx);
		K += vw_up*Lv_parent;//-rmtx*Lw_parent);
		K += ww_up*Lw_parent;
		K += Pw_parent*LP_down;
		K += Lw_parent*LL_down;
		K -= Lw_parent;
	}
}


entity_contact *CArticulatedEntity::CreateConstraintContact(int idx)
{
	entity_contact *pcontact = (entity_contact*)AllocSolverTmpBuf(sizeof(entity_contact));
	if (!pcontact) 
		return 0;
	pcontact->flags = 0;
	pcontact->pent[0] = this;
	pcontact->pbody[0] = &m_joints[idx].body;
	pcontact->ipart[0] = m_joints[idx].iStartPart;
	if (m_joints[idx].iParent>=0) {
		pcontact->pent[1] = this;
		pcontact->pbody[1] = &m_joints[m_joints[idx].iParent].body;
		pcontact->ipart[1] = m_joints[m_joints[idx].iParent].iStartPart;
	} else {
		pcontact->pent[1] = &g_StaticPhysicalEntity;
		pcontact->pbody[1] = &g_StaticRigidBody;
		pcontact->ipart[1] = 0;
	}
	pcontact->friction = 0;
	pcontact->vreq.zero();
	pcontact->pt[0] = pcontact->pt[1] = m_joints[idx].body.pos + m_joints[idx].quat*m_joints[idx].pivot[1];
	pcontact->K.SetZero();

	return pcontact;
}


int CArticulatedEntity::RegisterContacts(float time_interval,int nMaxPlaneContacts)
{
	int idx,i,j,iAxes[3];
	masktype contacts_mask,constraints_mask;
	entity_contact *pcontact;
__ae_step++;

	UpdateConstraints();

	for(i=0,contacts_mask=constraints_mask=0;i<m_nColliders;i++) {
		contacts_mask |= m_pColliderContacts[i];
		constraints_mask |= m_pColliderConstraints[i];
	}

	for(idx=0;idx<m_nJoints;idx++) {
		m_joints[idx].body.softness[0] = m_softness[m_bInGroup*2+0];
		m_joints[idx].body.softness[1] = m_softness[m_bInGroup*2+1];
		m_joints[idx].iContacts &= contacts_mask;
		for(i=0; i<NMASKBITS && getmask(i)<=m_joints[idx].iContacts; i++) 
		if (m_joints[idx].iContacts & getmask(i) && !(m_pContacts[i].flags & contact_2b_verified)) {
			RegisterContact(m_pContacts+i);
			if (i==31 || getmask(i+1)>m_joints[idx].iContacts) // register only the last contact in history
				ArchiveContact(i);
		}

		for(i=0;i<NMASKBITS && getmask(i)<=constraints_mask;i++) 
		if (constraints_mask&getmask(i) && m_pConstraintInfos[i].bActive && 
				(unsigned int)(m_pConstraints[i].ipart[0]-m_joints[idx].iStartPart) < (unsigned int)m_joints[idx].nParts)
			RegisterContact(m_pConstraints+i);

		if (m_bGrounded || m_joints[idx].iParent>=0) {
			vectorf pivot[2],axisDrift(zero);
			if (!(pcontact = CreateConstraintContact(idx)))
				break;
			pcontact->flags = contact_constraint_3dof;
			GetContactMatrix(pcontact->pt[0], pcontact->ipart[0], pcontact->K);
			if (m_joints[idx].iParent>=0) {
				GetContactMatrix(pcontact->pt[0], pcontact->ipart[1], pcontact->K);
				pivot[0] = m_joints[m_joints[idx].iParent].body.pos + m_joints[m_joints[idx].iParent].quat*m_joints[idx].pivot[0];
			} else 
				pivot[0] = m_posPivot;
			pivot[1] = m_joints[idx].body.pos + m_joints[idx].quat*m_joints[idx].pivot[1];
			if (m_iSimTypeCur && (pivot[0]-pivot[1]).len2()>sqr(0.003f)) {
				pcontact->vreq = (pivot[0]-pivot[1])*10.0f;
				pcontact->n = pcontact->vreq.normalized();
				if (pcontact->vreq.len2()>sqr(10.0f))
					pcontact->vreq = pcontact->n*10.0f;
				pcontact->pt[1] = pivot[0];
			} else
				pcontact->n.Set(0,0,1);
			RegisterContact(pcontact);

			for(i=j=0;i<3;i++) if (!(m_joints[idx].flags & (angle0_locked|angle0_gimbal_locked)<<i))
				iAxes[j++] = i;
			else
				axisDrift += m_joints[idx].rotaxes[i]*min(1.0f,m_joints[idx].q0[i]-m_joints[idx].q[i]);
			if ((unsigned int)j-1u<2u) {
				if (!(pcontact = CreateConstraintContact(idx)))
					break;

				if (m_iSimTypeCur && j==1 && m_joints[idx].flags&joint_expand_hinge) {
					if (is_unused(m_joints[idx].hingePivot[0])) {
						CCylinderGeom cylgeom;
						cylinder cyl;
						box bbox;
						geom_world_data gwd[2];
						intersection_params ip;
						geom_contact *punproj;
						i = m_joints[idx].iStartPart;

						m_parts[i].pPhysGeomProxy->pGeom->GetBBox(&bbox);
						cyl.axis = m_joints[idx].rotaxes[iAxes[0]];
						cyl.center.zero();
						cyl.r = (bbox.size.x+bbox.size.y+bbox.size.z)*m_parts[i].scale*0.05f; cyl.hh = cyl.r*0.2f;
						cylgeom.CreateCylinder(&cyl);

						gwd[0].offset = m_pos + m_qrot*m_parts[i].pos;
						//(m_qrot*m_parts[i].q).getmatrix(gwd[0].R); //Q2M_IVO
						gwd[0].R = matrix3x3f(m_qrot*m_parts[i].q);
						gwd[0].scale = m_parts[i].scale;
						gwd[1].offset = pcontact->pt[0];
						gwd[1].v = -m_joints[idx].rotaxes[iAxes[0]];
						ip.iUnprojectionMode = 0;
						ip.time_interval = 1E10;
						if (m_parts[i].pPhysGeomProxy->pGeom->Intersect(&cylgeom, gwd+1,gwd, &ip, punproj)) {
							punproj->t -= cyl.hh;
							m_joints[idx].hingePivot[1] = m_joints[idx].pivot[1] + (m_joints[idx].rotaxes[iAxes[0]]*m_joints[idx].quat)*punproj->t;
							m_joints[idx].hingePivot[0] = m_joints[idx].iParent>=0 ?
								m_joints[idx].pivot[0] + (m_joints[idx].rotaxes[iAxes[0]]*m_joints[m_joints[idx].iParent].quat)*punproj->t :
								m_posPivot + m_joints[idx].rotaxes[iAxes[0]]*punproj->t;
						} else
							m_joints[idx].flags &= ~joint_expand_hinge;
					}

					if (m_joints[idx].flags & joint_expand_hinge) {
						pcontact->flags = contact_constraint_3dof;
						pcontact->pt[0] = m_joints[idx].body.pos + m_joints[idx].quat*m_joints[idx].hingePivot[1];
						GetContactMatrix(pcontact->pt[0], pcontact->ipart[0], pcontact->K);
						if (m_joints[idx].iParent>=0) {
							GetContactMatrix(pcontact->pt[0], pcontact->ipart[1], pcontact->K);
							pcontact->pt[1] = m_joints[m_joints[idx].iParent].body.pos + m_joints[m_joints[idx].iParent].quat*m_joints[idx].hingePivot[0];
						} else 
							pcontact->pt[1] = m_joints[idx].hingePivot[0];
						if ((pcontact->pt[0]-pcontact->pt[1]).len2()>sqr(0.003f)) {
							pcontact->vreq = (pivot[0]-pivot[1])*10.0f;
							pcontact->n = pcontact->vreq.normalized();
							if (pcontact->vreq.len2()>sqr(10.0f))
								pcontact->vreq = pcontact->n*10.0f;
						} else
							pcontact->n.Set(0,0,1);
					}
				}

				pcontact->K = m_joints[idx].body.Iinv;
				if (m_joints[idx].iParent>=0)
					pcontact->K += m_joints[m_joints[idx].iParent].body.Iinv;
				if (j==2) { // 2 free axes, 1 axis locked
					pcontact->flags = contact_constraint_2dof | contact_angular;
					pcontact->n = (m_joints[idx].rotaxes[iAxes[0]]^m_joints[idx].rotaxes[iAxes[1]]).normalized();
					pcontact->vreq = pcontact->n*min(5.0f,(axisDrift*pcontact->n)*6.0f);
				} else { // 1 free axis, 2 axes locked
					pcontact->flags = contact_constraint_1dof | contact_angular;
					pcontact->n = m_joints[idx].rotaxes[iAxes[0]];
					pcontact->vreq = (axisDrift-pcontact->n*(pcontact->n*axisDrift))*6.0f;
					if (pcontact->vreq.len2()>sqr(5.0f))
						pcontact->vreq.normalize() *= 5.0f;
				}
				RegisterContact(pcontact);
			}

			for(i=0;i<3;i++) if (m_joints[idx].flags & angle0_limit_reached<<i) {
				if (!(pcontact = CreateConstraintContact(idx)))
					break;
				pcontact->K = m_joints[idx].body.Iinv;
				if (m_joints[idx].iParent>=0)
					pcontact->K += m_joints[m_joints[idx].iParent].body.Iinv;
				pcontact->flags = contact_angular;
				pcontact->n = m_joints[idx].rotaxes[i]*-sgnnz(m_joints[idx].dq_limit[i]);
				if (m_iSimTypeCur)
					pcontact->vreq = pcontact->n*min(5.0f,fabsf(m_joints[idx].dq_limit[i])*4.0f);
				//pcontact->vsep = -0.01f;
				RegisterContact(pcontact);
			}
		}
	}

	return 0;
}

float __maxdiff = 0;

int CArticulatedEntity::Update(float time_interval, float damping)
{
	int i,j,nCollJoints=0; 
	float e,minEnergy = m_nContacts>=m_nCollLyingMode ? m_EminLyingMode : m_Emin;
	m_bAwake = (iszero(m_nContacts) & (m_bFloating^1)) | isneg(m_simTimeAux-0.5f);
	m_bUsingUnproj = 0;
	m_nStepBackCount = (m_nStepBackCount&-m_bSteppedBack)+m_bSteppedBack;

	for(i=0;i<m_nJoints;i++) {
		m_joints[i].dq *= damping;
		m_joints[i].body.v*=damping; m_joints[i].body.w*=damping;
		m_joints[i].body.P*=damping; m_joints[i].body.L*=damping;

		e = (m_joints[i].body.v.len2() + (m_joints[i].body.L*m_joints[i].body.w)*m_joints[i].body.Minv)*0.5f + m_joints[i].body.Eunproj;
		m_bAwake |= (m_joints[i].bAwake = isneg(minEnergy-e));
		m_bUsingUnproj |= isneg(1E-8f-m_joints[i].body.Eunproj);
		m_joints[i].bAwake |= m_bFloating|m_bUsingUnproj;
		for(j=m_joints[i].iParent; j>=0; j=m_joints[j].iParent)
			m_joints[i].bAwake |= m_joints[j].bAwake;
		nCollJoints += m_joints[i].bHasExtContacts;

		if (m_iSimTypeCur && m_joints[i].body.Eunproj>0) {
			SyncJointWithBody(i,3);
			for(j=m_joints[i].iStartPart; j<m_joints[i].iStartPart+m_joints[i].nParts; j++) {
				(m_parts[j].q = m_joints[i].quat*m_infos[j].q0).Normalize();
				m_parts[j].pos = m_joints[i].quat*m_infos[j].pos0+m_joints[i].body.pos-m_pos;
			}
		}
	}
	m_nSleepFrames = (m_nSleepFrames&~-m_bAwake) + (m_bAwake^1);
	m_bAwake |= isneg(m_nSleepFrames-4) & isneg(nCollJoints-3);
	if (!m_bGrounded)
		m_body.v = m_joints[0].body.v;
	m_body.P = m_body.v*m_body.M;
	m_body.w.zero();
	m_bInGroup = isneg(-m_nDynContacts);

	for(i=0; i<m_nJoints; i++) {
#ifdef _DEBUG
vectorf v0=m_joints[i].body.v,w0=m_joints[i].body.w;
#endif
		SyncJointWithBody(i,2);
		if (m_iSimTypeCur==0)
			SyncBodyWithJoint(i,2);
#ifdef _DEBUG
float diff = (m_joints[i].body.v-v0).len()+(m_joints[i].body.w-w0).len();
__maxdiff = max(diff,__maxdiff);
if (m_joints[i].iParent>=0)	{
	vectorf pivot[2],dpivot,dvel; int j = m_joints[i].iParent;
	pivot[0] = m_joints[j].body.pos + m_joints[j].quat*m_joints[i].pivot[0];
	pivot[1] = m_joints[i].body.pos + m_joints[i].quat*m_joints[i].pivot[1];
	dpivot = pivot[0]-pivot[1];
	dvel = m_joints[i].body.v + (m_joints[i].body.w^pivot[1]-m_joints[i].body.pos) -
				 m_joints[j].body.v - (m_joints[j].body.w^pivot[0]-m_joints[j].body.pos);
	float diff1 = (dvel-dpivot*10.0f).len();
	pivot[0].zero();
}
#endif 
		/*for(j=iPersistentContacts=0; j<NMASKBITS && getmask(j)<=m_joints[i].iContacts; j++) 
		if (m_joints[i].iContacts & getmask(j)) {
			pbody = m_pContacts[j].pbody[0]; vrel = pbody->v+(pbody->w ^ m_pContacts[j].pt-pbody->pos);
			pbody = m_pContacts[j].pbody[1]; vrel -= pbody->v+(pbody->w ^ m_pContacts[j].pt-pbody->pos);
			if (vrel*m_pContacts[j].n<m_pWorld->m_vars.minSeparationSpeed && m_pContacts[j].penetration==0)
				iPersistentContacts |= getmask(j);
			
		}
		m_joints[i].iContacts = iPersistentContacts;*/
	}

	if (m_iSimClass-1!=m_bAwake) {
		m_iSimTypeOverride &= m_bAwake;
		m_iSimClass = 1+m_bAwake;
		m_pWorld->RepositionEntity(this,2);
	}

	if (!m_bAwake) {
		for(i=0;i<m_nJoints;i++) {
			m_joints[i].dq.zero(); m_joints[i].body.P.zero(); m_joints[i].body.L.zero();
			m_joints[i].body.v.zero(); m_joints[i].body.w.zero();
		}
		m_body.P.zero(); m_body.L.zero(); m_body.v.zero(); m_body.w.zero();
	}

	return (m_bAwake^1) | isneg(m_timeStepFull-m_timeStepPerformed-0.001f);
}


void CArticulatedEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	CRigidEntity::GetMemoryStatistics(pSizer);
	if (GetType()==PE_ARTICULATED)
		pSizer->AddObject(this, sizeof(CArticulatedEntity));
	pSizer->AddObject(m_joints, m_nJointsAlloc*sizeof(m_joints[0]));
	pSizer->AddObject(m_infos, m_nPartsAlloc*sizeof(m_infos[0]));
}