#include "stdafx.h"
#include "CryModel.h"
#include "CryModelState.h"
#include "CVars.h"

///////////////////////////////////////////// physics stuff /////////////////////////////////////////////////

#define FULL_LOD_LEVEL 0


//////////////////////////////////////////////////////////////////////////
// finds the first physicalized parent of the given bone (bone given by index)
int CryModelState::getBonePhysParentIndex(int iBoneIndex, int nLod)
{
	int iPrevBoneIndex;
	do {
		iBoneIndex = getBoneParentIndex(iPrevBoneIndex = iBoneIndex);
	} while (iBoneIndex!=iPrevBoneIndex && !getBoneInfo(iBoneIndex)->m_PhysInfo[nLod].pPhysGeom);
	return iBoneIndex==iPrevBoneIndex ? -1 : iBoneIndex;
}


//////////////////////////////////////////////////////////////////////////
// finds the first physicalized child (or itself) of the given bone (bone given by index)
// returns -1 if it's not physicalized
int CryModelState::getBonePhysChildIndex (int iBoneIndex, int nLod)
{
	CryBoneInfo* pBoneInfo = getBoneInfo (iBoneIndex);
	if (pBoneInfo->m_PhysInfo[nLod].pPhysGeom)
		return iBoneIndex;
	unsigned numChildren = pBoneInfo->numChildren();
	unsigned nFirstChild = pBoneInfo->getFirstChildIndexOffset() + iBoneIndex;
	for (unsigned nChild = 0; nChild < numChildren; ++nChild)
	{
		int nResult = getBonePhysChildIndex(nFirstChild + nChild, nLod);
		if (nResult >= 0)
			return nResult;
	}
	return -1;
}

void CryModelState::BuildPhysicalEntity(IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale, float scale,Vec3d offset, int nLod)
{
	pe_type pentype = pent->GetType();
	int i,j;
	pe_geomparams gp;
	pe_articgeomparams agp;
	pe_geomparams *pgp = pentype==PE_ARTICULATED ? &agp:&gp;
	pgp->flags = pentype==PE_LIVING ? 0:geom_collides|geom_floats;
	pgp->bRecalcBBox = 0;
	float volume;
	Matrix44 mtx;
	for (i=0, volume=0; i<(int)numBones(); ++i)
		if (getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom)
			volume += getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom->V;
	pgp->density = mass/volume;
	
	if (surface_idx>=0)
		pgp->surface_idx = surface_idx;
	
	pent->Action(&pe_action_remove_all_parts());
	
	for(i=0;i<(int)numBones();i++) if (getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom) {
		mtx = getBoneMatrixGlobal(i);
		*(Vec3d*)mtx[3] *= scale;
		*(Vec3d*)mtx[3] += offset;
		pgp->pMtx4x4T = mtx[0];
		pgp->flags = /*strstr(getBoneInfo(i)->getNameCStr(),"Hand") ? geom_no_raytrace :*/ geom_collides|geom_floats;
		agp.idbody = i;
		pent->AddGeometry(getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom,pgp,i);
		getBoneInfo(i)->m_fMass = pgp->density*getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom->V;
	}

	if (pentype==PE_ARTICULATED) {
		CryBone *pBone;
		CryBoneInfo* pBoneInfo;
		matrix3x3bf mtx0;
		for(i=0;i<(int)numBones();i++) if (getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom)	{
			pe_params_joint pj;
			int iParts[8]; const char *ptr,*ptr1;
			pBone = &getBone(i);
			pBoneInfo = getBoneInfo(i);
			Matrix44& matBoneGlobal = getBoneMatrixGlobal(i);

			pj.pSelfCollidingParts = iParts;
			if ((pj.flags = pBoneInfo->m_PhysInfo[nLod].flags)!=-1)
				pj.flags |= angle0_auto_kd*7;
			else pj.flags = angle0_locked;
 			pj.op[0] = getBonePhysParentIndex(i,nLod);
			pj.op[1] = i;
			pj.pivot = matBoneGlobal.GetTranslationOLD();
      pj.pivot = (pj.pivot+offset)*scale;
			pj.nSelfCollidingParts = 0;
			if ((ptr=strstr(pBoneInfo->getNameCStr(),"Forearm"))) {
				for(j=0;j<(int)numBones();j++) if (getBoneInfo(j)->m_PhysInfo[nLod].pPhysGeom && 
					(strstr(getBoneInfo(j)->getNameCStr(),"Pelvis") || strstr(getBoneInfo(j)->getNameCStr(),"Head") || 
					 strstr(getBoneInfo(j)->getNameCStr(),"Spine") ||	
					 (ptr1=strstr(getBoneInfo(j)->getNameCStr(),"Thigh")) && ptr[-2]==ptr1[-2] || 
					 strstr(getBoneInfo(j)->getNameCStr(),"Forearm") && i>j))
					pj.pSelfCollidingParts[pj.nSelfCollidingParts++] = j;
			}
			if (pBoneInfo->m_PhysInfo[nLod].flags!=-1) for(j=0;j<3;j++) {
				pj.limits[0][j] = pBoneInfo->m_PhysInfo[nLod].min[j];
				pj.limits[1][j] = pBoneInfo->m_PhysInfo[nLod].max[j];
				pj.bounciness[j] = 0;
				pj.ks[j] = pBoneInfo->m_PhysInfo[nLod].spring_tension[j]*stiffness_scale;
				pj.kd[j] = pBoneInfo->m_PhysInfo[nLod].damping[j];
				if (fabsf(pj.limits[0][j])<3) {
					pj.qdashpot[j] = 0.2f; pj.kdashpot[j] = 40.0f;
				} else pj.qdashpot[j] = pj.kdashpot[j] = 0;
			} else for(j=0;j<3;j++) {
				pj.limits[0][j]=-1E10f; pj.limits[1][j]=1E10f;
				pj.bounciness[j]=0; pj.ks[j]=0; pj.kd[j]=stiffness_scale;
			}
			pj.pMtx0 = pj.pMtx0T = 0;
			if (pBoneInfo->m_PhysInfo[nLod].framemtx[0][0]<10 && pBoneInfo->m_PhysInfo[nLod].flags!=-1)
				pj.pMtx0 = pBoneInfo->m_PhysInfo[nLod].framemtx[0];
			pent->SetParams(&pj);
		}
	}

	//m_vOffset = offset;
	//m_fScale = scale;
	m_fMass = mass;
	m_iSurfaceIdx = surface_idx;
}

IPhysicalEntity *CryModelState::CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, 
								  																		 float scale,Vec3d offset, int nLod)
{
	if (m_pCharPhysics) {
		GetPhysicalWorld()->DestroyPhysicalEntity(m_pCharPhysics);
		m_pCharPhysics = 0;
	}

	if (m_bHasPhysics) {
		pe_params_pos pp;
		pp.iSimClass = 4;
		m_pCharPhysics = GetPhysicalWorld()->CreatePhysicalEntity(PE_ARTICULATED,5.0f,&pp,0);

		pe_params_articulated_body pab;
		pab.bGrounded = 1;
		pab.scaleBounceResponse = 0.6f;
		pab.bAwake = 0;
		pab.pivot = vectorf(getBoneMatrixGlobal(getBonePhysChildIndex(0))[3])*scale;
		m_pCharPhysics->SetParams(&pab);

		BuildPhysicalEntity(m_pCharPhysics, mass,surface_idx,stiffness_scale, scale,vectorf(zero), 0);

		if (pHost) {
			pe_params_foreign_data pfd;
			pHost->GetParams(&pfd);
			MARK_UNUSED pfd.iForeignFlags;
			m_pCharPhysics->SetParams(&pfd);

			pe_params_articulated_body pab1;
			pab1.pivot.zero();
			pab1.pHost = pHost;
			pab1.posHostPivot = vectorf(getBoneMatrixGlobal(getBonePhysChildIndex(0))[3])*scale+offset;
			m_pCharPhysics->SetParams(&pab1);
		}

		pe_params_joint pj;
		pj.op[0] = -1;
		pj.op[1] = getBonePhysChildIndex(0);
		pj.flags = all_angles_locked | joint_no_gravity | joint_isolated_accelerations;
		m_pCharPhysics->SetParams(&pj);
	}

	m_vOffset = offset;
	m_fScale = scale;
	m_fMass = mass;
	m_iSurfaceIdx = surface_idx;	

	return m_pCharPhysics;
}

int CryModelState::CreateAuxilaryPhysics(IPhysicalEntity *pHost, float scale,Vec3d offset, int nLod)
{
	int i,j,k;
	char strbuf[256],*pspace;

	// Delete aux physics

	for(i=0;i<m_nAuxPhys;i++)
	{
		GetPhysicalWorld()->DestroyPhysicalEntity(m_auxPhys[i].pPhysEnt);
		delete[] m_auxPhys[i].pauxBoneInfo;
	}

	m_nAuxPhys=0;

	for (i = 0; i<(int)numBones(); i++)
	{
		CryBoneInfo* pBoneInfo = getBoneInfo(i);
		const char* szBoneName = pBoneInfo->getNameCStr();
#if defined(LINUX)
		if (!strnicmp(szBoneName,"rope",4))
#else
		if (!_strnicoll(szBoneName,"rope",4))
#endif
		{
			strcpy(strbuf,szBoneName);
			if (pspace = strchr(strbuf,' '))
			{
				*pspace = 0;
				pBoneInfo->setName(strbuf);
			}
			for(j = 0; j < m_nAuxPhys && strcmp(m_auxPhys[j].strName,szBoneName); ++j)
				continue;
			if (j == m_nAuxPhys)
			{
				if (m_nAuxPhys==SIZEOF_ARRAY(m_auxPhys))
					break;
				m_auxPhys[m_nAuxPhys].pPhysEnt = GetPhysicalWorld()->CreatePhysicalEntity(PE_ROPE);
				m_auxPhys[m_nAuxPhys++].strName = szBoneName;
			}
			pBoneInfo->m_nLimbId = 100+j;
		}
	}

	for (j = 0; j < m_nAuxPhys; ++j)
	{
		pe_status_pos sp;
		sp.pos.zero(); 
		sp.q.SetIdentity();
		if (pHost)
			pHost->GetStatus(&sp);

		pe_params_rope pr;
		int iLastBone = 0;
		pr.nSegments = 0;
		for(i=0;i<(int)numBones();i++) if (!strcmp(m_auxPhys[j].strName,getBoneInfo(i)->getNameCStr()))
			pr.nSegments++;
		pr.pPoints = new vectorf[pr.nSegments+1];
		m_auxPhys[j].pauxBoneInfo = new aux_bone_info[m_auxPhys[j].nBones = pr.nSegments];
		pr.length = 0;

		for(i=k=0;i<(int)numBones();i++) if (!strcmp(m_auxPhys[j].strName,getBoneInfo(i)->getNameCStr()))	{
			if (k==0 && getBonePhysParentIndex(i,nLod)>=0)
				pr.idPartTiedTo[0] = getBonePhysParentIndex(i,nLod);
			pr.pPoints[k] = *(Vec3d*)getBoneMatrixGlobal(i)[3]*scale + offset;
			pr.pPoints[k+1] = *(Vec3d*)getBoneMatrixGlobal(getBoneChildIndex(i,0))[3]*scale + offset;
			m_auxPhys[j].pauxBoneInfo[k].iBone = i;
			m_auxPhys[j].pauxBoneInfo[k].dir0 = pr.pPoints[k+1]-pr.pPoints[k];
			if (m_auxPhys[j].pauxBoneInfo[k].dir0.len2()>0)
				m_auxPhys[j].pauxBoneInfo[k].dir0 *= 
					(m_auxPhys[j].pauxBoneInfo[k].rlen0 = 1.0f/m_auxPhys[j].pauxBoneInfo[k].dir0.len());
			else {
				m_auxPhys[j].pauxBoneInfo[k].dir0(0,0,1);
				m_auxPhys[j].pauxBoneInfo[k].rlen0 = 1.0f;
			}
			m_auxPhys[j].pauxBoneInfo[k].quat0 = quaternionf(*(const matrix3x3in4x4Tf*)&getBoneMatrixGlobal(i));
			pr.pPoints[k] = sp.q*pr.pPoints[k]+sp.pos;
			pr.pPoints[k+1] = sp.q*pr.pPoints[k+1]+sp.pos;
			pr.length += (pr.pPoints[k+1]-pr.pPoints[k]).len();
			iLastBone = ++k;
		}
		if (!is_unused(pr.idPartTiedTo[0])) {
			pr.pEntTiedTo[0] = pHost;
			pr.ptTiedTo[0] = pr.pPoints[0];
		}
		if ((i = getBonePhysChildIndex(iLastBone))>=0) {
			pr.pEntTiedTo[1] = pHost;
			pr.idPartTiedTo[1] = i;
			pr.ptTiedTo[1] = pr.pPoints[pr.nSegments];
		}


		m_auxPhys[j].pPhysEnt->SetParams(&pr);
		delete[] pr.pPoints;
	}

	m_vOffset = offset;
	m_fScale = scale;

	return m_nAuxPhys;
}

//////////////////////////////////////////////////////////////////////////
// Replaces each bone global matrix with the relative matrix.
// The root matrix is relative to the world, which is its parent, so it's
// obviously both the global and relative (they coincide), so it doesn't change
//
// ASSUMES: the matrices are unitary and orthogonal, so that Transponse(M) == Inverse(M)
// ASSUMES: in each bone global matrix, upon entry, there's the actual global matrix
// RETURNS: in each m_matRelativeToParent matrix, there's a matrix relative to the parent
void CryModelState::ConvertBoneGlobalToRelativeMatrices()
{
  unsigned i, numBones = this->numBones();

	// scan through all bones from the bottom up (from children to parents),
	// excluding the root (i>0)
	for (i = numBones-1; i > 0; --i)
	{
		Matrix44 &Mtxrel = getBone(i).m_matRelativeToParent;
		Matrix44 &mtxBoneGlobal = getBoneMatrixGlobal (i);
		Matrix44 &mtxParentGlobal = getBoneMatrixGlobal (getBoneParentIndex(i));
		matrix3x3in4x4Tf &mtxrel((matrix3x3in4x4Tf&)Mtxrel),
			&mtxparent ((matrix3x3in4x4Tf&)mtxParentGlobal),
			&mtxchild ((matrix3x3in4x4Tf&)mtxBoneGlobal);

		vectorf dx = mtxBoneGlobal.GetTranslationOLD()-mtxParentGlobal.GetTranslationOLD();
		(mtxrel = mtxparent.T()) *= mtxchild;
		Mtxrel.SetTranslationOLD(dx*mtxparent);
		Mtxrel[0][3] = Mtxrel[1][3] = Mtxrel[2][3] = 0.0f;
		Mtxrel[3][3] = 1.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
// Multiplies each bone global matrix with the parent global matrix,
// and calculates the relative-to-default-pos matrix. This is essentially
// the process opposite to conversion of global matrices to relative form
// performed by ConvertBoneGlobalToRelativeMatrices()
// NOTE:
//   The root matrix is relative to the world, which is its parent, so it's
//   obviously both the global and relative (they coincide), so it doesn't change
//
// PARAMETERS:
//   bNonphysicalOnly - if set to true, only those bones that have no physical geometry are affected
//
// ASSUMES: in each m_matRelativeToParent matrix, upon entry, there's a matrix relative to the parent
// RETURNS: in each bone global matrix, there's the actual global matrix
void CryModelState::UnconvertBoneGlobalFromRelativeForm(bool bNonphysicalOnlyArg, int nLod)
{
	unsigned numBones = this->numBones();	
	bool bNonphysicalOnly = true;
	// start from 1, since we don't affect the root, which is #0
	for (unsigned i = 1; i < numBones; ++i)
	{
		CryBoneInfo* pBoneInfo = getBoneInfo(i);
		bool bPhysical = pBoneInfo->getPhysInfo(nLod).pPhysGeom || pBoneInfo->getLimbId()>=100;
		if (!bNonphysicalOnly || !bPhysical)
		{
			assert (pBoneInfo->hasParent());
			Matrix44& matBoneGlobal = getBoneMatrixGlobal(i);
			matBoneGlobal = getBone(i).m_matRelativeToParent*getBoneMatrixGlobal(getBoneParentIndex(i));
		}
		if (bPhysical) // don't update the 1st physical bone (pelvis) even if bNonphysicalOnlyArg is false
			bNonphysicalOnly = bNonphysicalOnlyArg;
	}
}


void CryModelState::ResetNonphysicalBoneRotations(int nLod, float fBlend)
{
	// set non-physical bones to their default position wrt parent for LODs>0
	// do it for every bone except the root, parents first
	unsigned numBones = this->numBones();
	for (unsigned nBone = 1; nBone < numBones; ++nBone)
	{
		CryBoneInfo* pInfo = getBoneInfo(nBone);
		if (!pInfo->getPhysInfo(nLod).pPhysGeom)
		{
			CryBone* pBone = &getBone(nBone);
			Matrix44& matBoneGlobal = getBoneMatrixGlobal(nBone);
			if (fBlend>=1.0f)
				pInfo->getDefRelTransform().buildMatrix(pBone->m_matRelativeToParent);
			else
			{
				IController::PQLog pq;
				pq.blendPQ(pBone->m_pqTransform, pInfo->getDefRelTransform(), fBlend);
				pq.buildMatrix(pBone->m_matRelativeToParent);
			}
		}
	}
}


void CryModelState::SynchronizeWithPhysicalEntity(IPhysicalEntity *pent, const Vec3& posMaster,const Quat& qMaster, Vec3 offset, int iDir)
{
	if (pent && (iDir==0 || iDir<0 && pent->GetType()!=PE_ARTICULATED && pent->GetType()!=PE_RIGID))
	{	// copy state _to_ physical entity
		//m_vOffset = offset;
		//m_fScale = scale;

		if (!m_bHasPhysics) return;
		pe_params_part partpos;
		int i,j;
		for(i=j=0;i<(int)numBones();i++) if (getBoneInfo(i)->m_PhysInfo[0].pPhysGeom)
			j=i;
		if (pent->GetType()==PE_ARTICULATED)
			offset = -*(Vec3*)getBoneMatrixGlobal(getBonePhysChildIndex(0))[3];//*scale;
		
		for(i=0;i<(int)numBones();i++) if (getBoneInfo(i)->m_PhysInfo[0].pPhysGeom)
		{
			partpos.partid = i;
			partpos.bRecalcBBox = i==j;
			Matrix44 mtx = getBoneMatrixGlobal(i);
			//*(vectorf*)mtx[3] *= scale;
			*(Vec3d*)mtx[3] += offset;
			partpos.pMtx4x4T = (float*)&mtx;
			if (!pent->SetParams(&partpos))
				break;
		}
	}	else
	{	// copy state _from_ physical entity
		if (!pent && !m_nAuxPhys) 
			return;
		int i,j,nLod = min(m_pCharPhysics!=pent?1:0,(int)numLODs()-1);
		float rScale = 1.0f;///scale;
		pe_status_pos sp;
		pe_status_joint sj;
		m_bPhysicsAwake = 0;
		if (pent)
			m_bPhysicsAwake = pent->GetStatus(&pe_status_awake());
		for(j=0;j<m_nAuxPhys;j++)
			m_bPhysicsAwake |= m_auxPhys[j].pPhysEnt->GetStatus(&pe_status_awake());

		if (!m_bPhysicsAwake && !m_bPhysicsWasAwake)
			return;

		if (!m_bPhysicsAwake)
			m_fPhysBlendTime = m_fPhysBlendMaxTime+0.1f;
		ResetNonphysicalBoneRotations(nLod, m_fPhysBlendTime*m_frPhysBlendMaxTime);

		if (pent)
		{
			pe_status_pos partpos;
			partpos.flags = status_local;
			for(i=0;i<(int)numBones();i++) if (getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom) 
			{
				partpos.partid = i;
				getBoneMatrixGlobal(i).SetIdentity();
				partpos.pMtx4x4T = (float*)&getBoneMatrixGlobal(i);
				pent->GetStatus(&partpos);
				getBoneMatrixGlobal(i).SetTranslationOLD((getBoneMatrixGlobal(i).GetTranslationOLD()-offset)*rScale);
				if (m_fPhysBlendTime < m_fPhysBlendMaxTime)
					break; // if in blending stage, do it only for the root
			}

			//Matrix33 mtxRel;
			for(;i<(int)numBones();i++) if (getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom) 
			{
				//mtxRel.SetIdentity33();
				// build the physparent->immediate parent matrix
				//for(j=getBoneParentIndex(i); j>0 && !getBoneInfo(j)->getPhysInfo(nLod).pPhysGeom; j=getBoneParentIndex(j))
				//	mtxRel = (matrix3x3in4x4Tf&)getBone(j).m_matRelativeToParent * mtxRel; 
				sj.idChildBody = i;
				pent->GetStatus(&sj);
				//(matrix3x3in4x4Tf&)getBone(i).m_matRelativeToParent = mtxRel.T()*Matrix33(sj.quat0*GetRotationXYZ<float>(sj.q+sj.qext));
				(matrix3x3in4x4Tf&)getBone(i).m_matRelativeToParent = 
					Matrix33(getBoneInfo(i)->getqRelPhysParent(nLod) * sj.quat0 * GetRotationXYZ<float>(sj.q+sj.qext));
				getBone(i).m_matRelativeToParent.SetTranslationOLD(getBoneInfo(i)->getDefRelTransform().vPos);
			}

			pent->GetStatus(&sp);
		}
		else
		{
			sp.pos = posMaster;
			sp.q = qMaster;
		}

		// additionally copy state from all ropes present in this character
		pe_params_rope pr;
		for(j=0;j<m_nAuxPhys;j++)
		{
			m_auxPhys[j].pPhysEnt->GetParams(&pr);
			for(i=0;i<m_auxPhys[j].nBones;i++) {
				vectorf &pt0 = *(vectorf*)((char*)pr.pPoints+i*pr.iStride),
								&pt1 = *(vectorf*)((char*)pr.pPoints+(i+1)*pr.iStride),
								dir = (pt1-pt0)*sp.q;
				float len = dir.len();
				if (len>1E-4f) {
					int nBone = m_auxPhys[j].pauxBoneInfo[i].iBone;
					CryBone &bone = getBone(nBone);
					Matrix44& matBoneGlobal = getBoneMatrixGlobal(nBone);

					//(quaternionf(m_auxPhys[j].pauxBoneInfo[i].dir0, dir/len)*m_auxPhys[j].pauxBoneInfo[i].quat0).
					//	getmatrix(matrix3x3in4x4Tf(matBoneGlobal[0])) *= len*m_auxPhys[j].pauxBoneInfo[i].rlen0;
					//Q2M_IVO			
					//(GetRotationV0V1(m_auxPhys[j].pauxBoneInfo[i].dir0, dir/len)*m_auxPhys[j].pauxBoneInfo[i].quat0).
						//getmatrix((matrix3x3in4x4Tf&)matBoneGlobal) *= len*m_auxPhys[j].pauxBoneInfo[i].rlen0;
					*(matrix3x3in4x4Tf*)&matBoneGlobal = (
						matrix3x3f(
								GetRotationV0V1(m_auxPhys[j].pauxBoneInfo[i].dir0, dir/len) * m_auxPhys[j].pauxBoneInfo[i].quat0
							) * len*m_auxPhys[j].pauxBoneInfo[i].rlen0
						);
					matBoneGlobal.SetTranslationOLD(((pt0-sp.pos)*sp.q-offset)*rScale);
				}
			}
		}

		UnconvertBoneGlobalFromRelativeForm(m_fPhysBlendTime>=m_fPhysBlendMaxTime, nLod);
		if(m_bPhysicsAwake)
			ForceReskin();
		m_bPhysicsWasAwake = m_bPhysicsAwake;
	}
}


IPhysicalEntity *CryModelState::RelinquishCharacterPhysics()
{
	if (m_bHasPhysics) {
		int i, nLod=min(numLODs()-1,1), iRoot=getBonePhysChildIndex(0,0);
		DestroyCharacterPhysics();

		ConvertBoneGlobalToRelativeMatrices();	// takes into accout all post-animation layers
		// store death pose (current) matRelative orientation in bone's m_pqTransform
		for(i=0;i<(int)numBones();i++) 
		{	// '-' since pqTransform.buildMatrix assumes flipped quaternion
			getBone(i).m_pqTransform.vRotLog = -log(CryQuat((matrix3x3in4x4Tf&)getBone(i).m_matRelativeToParent)).v;
			//Vec3 vb4 = getBone(i).m_pqTransform.vRotLog;
			AdjustLogRotationTo(getBoneInfo(i)->getDefRelTransform().vRotLog, getBone(i).m_pqTransform.vRotLog);
			/*Vec3 vaft = getBone(i).m_pqTransform.vRotLog;
			if (fabsf(exp(CryQuat(0,vb4.x,vb4.y,vb4.z))|exp(CryQuat(0,vaft.x,vaft.y,vaft.z)))<0.98f)
				getBone(i).m_pqTransform.vRotLog=vb4;*/
			getBone(i).m_pqTransform.vPos = getBoneInfo(i)->getDefRelTransform().vPos;
		}
		ResetNonphysicalBoneRotations(nLod,1.0f); // reset nonphysical bones matRel to default pose matRel
		UnconvertBoneGlobalFromRelativeForm(false,nLod); // build matGlobals from matRelativeToParents 

		pe_params_articulated_body pab;
		pab.bGrounded = 0;
		pab.scaleBounceResponse = 1;
		pab.bCheckCollisions = 1;
		pab.bCollisionResp = 1;

		IPhysicalEntity *res = GetPhysicalWorld()->CreatePhysicalEntity(PE_ARTICULATED,&pab);//,&pp,0);
		BuildPhysicalEntity(res, m_fMass,m_iSurfaceIdx,0, m_fScale,m_vOffset, nLod);

		pe_params_joint pj;
		pj.bNoUpdate = 1;
		iRoot = getBonePhysChildIndex(0,nLod);
		for(i=0;i<3;i++) { pj.ks[i]=0; pj.kd[i]=0.2f; }
		for(i=numBones()-1;i>=0;i--)	if (getBoneInfo(i)->m_PhysInfo[nLod].pPhysGeom) {
			pj.op[0] = getBonePhysParentIndex(i,nLod);
			pj.op[1] = i;
			pj.flags = getBoneInfo(i)->m_PhysInfo[nLod].flags & (all_angles_locked | angle0_limit_reached*7);
			if (i==iRoot)
				pj.flags = 0;
			res->SetParams(&pj);
		}

		m_fPhysBlendTime = 0.0f;
		m_fPhysBlendMaxTime = g_GetCVars()->ca_DeathBlendTime();
		if (m_fPhysBlendMaxTime>0.001f)
			m_frPhysBlendMaxTime = 1.0f/m_fPhysBlendMaxTime;
		else
		{
			m_fPhysBlendMaxTime = 0.0f;
			m_frPhysBlendMaxTime = 1.0f;
		}
			
		ResetNonphysicalBoneRotations(nLod,0.0f); // restore death pose matRel from m_pqTransform
		UnconvertBoneGlobalFromRelativeForm(false,nLod); // build matGlobals from matRelativeToParents

		pe_simulation_params sp; 
		sp.gravity.Set(0,0,-7.5f);
		sp.maxTimeStep = 0.025f;
		sp.damping = 0.3f;
		sp.iSimClass = 2;
		res->SetParams(&sp);

		m_vOffset.zero();
		m_bPhysicsAwake=m_bPhysicsWasAwake = 1; 
		m_uFlags |= nFlagsNeedReskinAllLODs; // in order to force skinning update during the first death sequence frame
		return res;
	}
	return 0;
}

bool CryModelState::AddImpact(const int partid,vectorf point,vectorf impact,float scale)
{
	if (m_pCharPhysics)
	{
		if ((unsigned int)partid>=(unsigned int)numBones())
		{
			//GetLog()->LogToFile("*ERROR* CryModelState::AddImpact: part id %u is out of range [0..%u]", partid, numBones());
			// sometimes out of range value (like -1) is passed to indicate that no impulse should be added to character
			return false;
		}
		int i;

		CryBone* pImpactBone = &getBone(partid);
		const char* szImpactBoneName = getBoneInfo(partid)->getNameCStr();

		for(i=0; i<4 && !(m_pIKEffectors[i] && m_pIKEffectors[i]->AffectsBone(partid)); i++);
		const char *ptr = strstr(szImpactBoneName,"Spine");
		if (i<4 || strstr(szImpactBoneName,"Pelvis") || ptr && !ptr[5])	
		{
			if (i<4)
			{
				pe_params_articulated_body pab;
				m_pCharPhysics->GetParams(&pab);
				if (!pab.pHost)
					return false;
				//pe_action_impulse crouch_impulse;
				//crouch_impulse.impulse(0,0,-impact.len()*10.0f);
				//pab.pHost->Action(&crouch_impulse);
			}
			return false;	// don't add impulse to spine and pelvis
		}
		if (strstr(szImpactBoneName,"UpperArm") || strstr(szImpactBoneName,"Forearm"))
			impact *= 0.35f;
		else if (strstr(szImpactBoneName,"Hand"))
			impact *= 0.2f;
		else if (strstr(szImpactBoneName,"Spine1"))
			impact *= 0.2f;
		
		pe_action_impulse impulse;
		impulse.partid = partid;
		impulse.impulse = impact;
		impulse.point = point;
		m_pCharPhysics->Action(&impulse);

		return true;
	}
	else 
	{
		pe_action_impulse impulse;
		impulse.impulse = impact;
		impulse.point = point;
		for(int i=0;i<m_nAuxPhys;i++)
			m_auxPhys[i].pPhysEnt->Action(&impulse);
	}

	return false;
}

int CryModelState::TranslatePartIdToDeadBody(int partid)
{
	if ((unsigned int)partid>=(unsigned int)numBones())
		return -1;

	int nLod = min(numLODs()-1,1);
	if (getBoneInfo(partid)->m_PhysInfo[nLod].pPhysGeom)
		return partid;

	return getBonePhysParentIndex(partid,nLod);
}

void CryModelState::SetLimbIKGoal(int limbid, float scale, vectorf ptgoal, int ik_flags, float addlen, vectorf goal_normal)
{
	if (ptgoal.x==1E10) {
		if (m_pIKEffectors[limbid]) {
			delete m_pIKEffectors[limbid];
			m_pIKEffectors[limbid] = 0;
		}
		return;
	}
	int i;
	if (!m_pIKEffectors[limbid]) {
		for(i=0;i<(int)numBones() && getBoneInfo(i)->m_nLimbId!=limbid;i++);
		if (i==numBones()) return;
		m_pIKEffectors[limbid] = new CCryModEffIKSolver(this);
		m_pIKEffectors[limbid]->SetPrimaryBone(i);
	}
	for(i=0;i<3;i++) if (ptgoal[i]!=IK_NOT_USED) ptgoal[i]/=scale;
	if (limbid==LIMB_LEFT_LEG || limbid==LIMB_RIGHT_LEG)
		ptgoal.z += m_IKpos0[limbid].z;
	m_pIKEffectors[limbid]->SetGoal(ptgoal,ik_flags,addlen,goal_normal,m_IKpos0[limbid].z);
}

vectorf CryModelState::GetLimbEndPos(int limbid, float scale)
{
  int i;
	for(i=0;i<(int)numBones() && getBoneInfo(i)->m_nLimbId!=limbid;i++);
	if (i==numBones())
		return vectorf(zero);
	int limbend = getBoneGrandChildIndex(i,0,0);
	vectorf res = *(vectorf*)getBoneMatrixGlobal(limbend)[3]*scale;
	if (limbid==LIMB_LEFT_LEG || limbid==LIMB_RIGHT_LEG)
		res.z -= m_IKpos0[limbid].z;
	return res;
}

void CryModelState::ProcessPhysics(float fDeltaTimePhys, int nNeff)
{
	if(g_GetCVars()->ca_NoPhys())
		return;

	m_uFlags |= nFlagsNeedReskinAllLODs;

	pe_params_joint pj;
	pe_status_joint sj;
	pj.bNoUpdate = 1;
	if (fDeltaTimePhys>0)
		pj.ranimationTimeStep = 1.0f/(pj.animationTimeStep = fDeltaTimePhys);
	int i,j,iParent,iRoot;

	if (nNeff>=0)
		for(i=0;i<4;i++) if (m_pIKEffectors[i])
			m_pIKEffectors[i]->Tick (fDeltaTimePhys);

	if (m_pCharPhysics && (m_bPhysicsAwake = m_pCharPhysics->GetStatus(&pe_status_awake())))
	{
		if (nNeff==0) 
		{	// if there's no animation atm, just read the state from physics verbatim
			SynchronizeWithPhysicalEntity(m_pCharPhysics, Vec3(0,0,0),Quat(1,0,0,0),m_vOffset);
		} else 
		{
			matrix3x3bf mtxid;
			mtxid.SetIdentity();
			
			// read angles deltas from physics and set current angles from animation as qext to physics
			for(i=0; i<(int)numBones(); i++)
				if (getBoneInfo(i)->getPhysInfo(0).pPhysGeom && (iParent=getBonePhysParentIndex(i)) >= 0)
			{
				for (j = 0; j < 4 && !(m_pIKEffectors[j] && m_pIKEffectors[j]->AffectsBone(i)); ++j);
				if (j<4)
					continue;
				sj.idChildBody = i;
				if (!m_pCharPhysics->GetStatus(&sj))
					continue;
				matrix3x3in4x4Tf &matrel((matrix3x3in4x4Tf&)getBone(i).m_matRelativeToParent);
				matrix3x3f matparent,mtx; matparent.SetIdentity();
				for(int nCurParent=getBoneParentIndex(i); nCurParent != iParent; nCurParent=getBoneParentIndex(nCurParent))
					matparent.T() *= ((matrix3x3in4x4Tf&)getBone(nCurParent).m_matRelativeToParent).T();
				matrix3x3f &mtx0(getBoneInfo(i)->getPhysInfo(0).framemtx[0][0]<10 ? (matrix3x3RMf&)*(&getBoneInfo(i)->getPhysInfo(0).framemtx[0][0]) : mtxid);

				//EULER_IVO
				//(((mtx=mtx0.T())*=matparent)*=matrel).GetEulerAngles_XYZ(sj.qext);
			   mtx=mtx0.T();
			   mtx*=matparent;
			   mtx*=matrel;
				 sj.qext=Ang3::GetAnglesXYZ(mtx);
				 

				//quaternionf(sj.q+sj.qext).getmatrix(matrel);
 
				//Q2M_IVO 
				//GetRotationXYZ(sj.q+sj.qext).getmatrix(matrel);
				matrel=matrix3x3f(GetRotationXYZ<float>(sj.q+sj.qext));

				(matrel.T() *= mtx0.T()) *= matparent;

				pj.op[0] = iParent;
				pj.op[1] = i;
				pj.qext = sj.qext;
				m_pCharPhysics->SetParams(&pj);
			}

			UnconvertBoneGlobalFromRelativeForm(false);
		}
	}

	if (nNeff>=0)
		for(int i=0;i<4;i++) if (m_pIKEffectors[i])
			m_pIKEffectors[i]->ApplyToBone (nNeff++);

	if (m_pCharPhysics)
	{
		iRoot = getBonePhysChildIndex(0);
		
		if (m_bPhysicsAwake) 
		{	
			matrix3x3bf matrel;
			pj.q.zero();

			for(i=0; i<(int)numBones(); i++) if (getBoneInfo(i)->m_PhysInfo[0].pPhysGeom && (iParent=getBonePhysParentIndex(i))>=0)
			{	// now force positions of IK-controlled bones to physics
				for(j=0; j<4 && !(m_pIKEffectors[j] && m_pIKEffectors[j]->AffectsBone(i)); j++);
				if (j==4)
					continue;
				(matrel = ((matrix3x3in4x4Tf&)getBoneMatrixGlobal(iParent)).T()) *= (matrix3x3in4x4Tf&)getBoneMatrixGlobal(i);
				if (getBoneInfo(i)->m_PhysInfo[0].framemtx[0][0]<10)
					matrel.T() *= (matrix3x3RMf&)*(&getBoneInfo(i)->m_PhysInfo[0].framemtx[0][0]);

				//EULER_IVO
				//matrel.GetEulerAngles_XYZ(pj.qext);
				pj.qext = Ang3::GetAnglesXYZ(matrel);

				pj.op[0] = iParent;
				pj.op[1] = i;
				m_pCharPhysics->SetParams(&pj);
			}

			//EULER_IVO
			//((matrix3x3in4x4Tf&)getBoneMatrixGlobal(iRoot)).GetEulerAngles_XYZ(pj.qext);
			pj.qext = Ang3::GetAnglesXYZ( (matrix3x3in4x4Tf&)getBoneMatrixGlobal(iRoot) );
                      
			pj.op[0] = -1;
			pj.op[1] = iRoot;
			m_pCharPhysics->SetParams(&pj);
		}	else 
			SynchronizeWithPhysicalEntity(m_pCharPhysics, Vec3(zero),Quat(1,0,0,0),Vec3(zero), 0);

		pe_params_articulated_body pab;
		pab.pivot.zero();
		pab.posHostPivot = m_vOffset+vectorf(getBoneMatrixGlobal(iRoot)[3])*m_fScale;
		pab.bRecalcJoints = m_bPhysicsAwake;
		m_pCharPhysics->SetParams(&pab);
	}

	for(i=0;i<m_nAuxPhys;i++)
		m_bPhysicsAwake |= m_auxPhys[i].pPhysEnt->GetStatus(&pe_status_awake());

	if (m_bPhysicsAwake)
		m_uFlags |= nFlagsNeedReskinAllLODs;
	m_bPhysicsWasAwake = m_bPhysicsAwake;
}

IPhysicalEntity *CryModelState::GetCharacterPhysics(const char *pRootBoneName)
{ 
	if (!pRootBoneName)
		return m_pCharPhysics; 

	for(int i=0;i<m_nAuxPhys;i++) 
	{
#if defined(LINUX)
		if (!stricmp(m_auxPhys[i].strName,pRootBoneName))
#else
		if (!_stricoll(m_auxPhys[i].strName,pRootBoneName))
#endif
			return m_auxPhys[i].pPhysEnt;
	}

	return m_pCharPhysics;
}

IPhysicalEntity *CryModelState::GetCharacterPhysics(int iAuxPhys)
{ 
	if (iAuxPhys<0)
		return m_pCharPhysics;
	if (iAuxPhys>=m_nAuxPhys)
		return 0;
	return m_auxPhys[iAuxPhys].pPhysEnt;
}


void CryModelState::DestroyCharacterPhysics(int iMode)
{
	if (m_pCharPhysics)
		GetPhysicalWorld()->DestroyPhysicalEntity(m_pCharPhysics,iMode);
	if (iMode==0)
		m_pCharPhysics = 0;

	int i;
	for(i=0;i<m_nAuxPhys;i++) {
		GetPhysicalWorld()->DestroyPhysicalEntity(m_auxPhys[i].pPhysEnt,iMode);
		if (iMode==0)
			delete[] m_auxPhys[i].pauxBoneInfo;
	}
	if (iMode==0)
		m_nAuxPhys = 0;
}