
#include "stdafx.h"
#include "CryBone.h"
#include "CryBoneInfo.h"
#include "CryModelState.h"
#include "CryModEffector.h"
#include "CryModEffIKSolver.h"

CCryModEffIKSolver::CCryModEffIKSolver(CryModelState* pParent):
	m_pParent (pParent),
	m_nPrimaryBone (-1)
{
	m_flags = 0;
	m_ptGoal.zero();
	m_GoalNormal.zero();
	m_bActive = true;
	m_fFeetAngle = m_fDeltaTime = 0;
}


void CCryModEffIKSolver::Tick(float deltatime)
{
	const Matrix44& rBoneMtx = m_pParent->getBoneMatrixGlobal (m_pParent->getBoneGrandChildIndex(m_nPrimaryBone,0,0));
	vectorf pt = rBoneMtx.GetTranslationOLD();
	for(int i=0;i<3;i++) if (m_ptGoal[i]==IK_NOT_USED)
		m_ptGoal[i] = pt[i];
	m_bActive = !((m_flags & ik_leg) && (m_flags & ik_avoid_stretching) && m_ptGoal.z+0.01f<pt.z);
	m_fDeltaTime = deltatime;
}

void RotateMatrix(Matrix44 &mtx, vectorf axis,float cosa,float sina,vectorf pivot)
{
	vectorf curaxis; int i;
	Matrix44 rotmtx; rotmtx.SetIdentity();

	for(i=0,curaxis.zero();i<3;i++) {
		curaxis[i] = 1;
		*(vectorf*)&rotmtx(i,0) = curaxis.rotated(axis,cosa,sina);
		curaxis[i] = 0;
	}
	rotmtx.SetTranslationOLD(pivot-pivot.rotated(axis,cosa,sina));

	mtx *= rotmtx;
}

void CCryModEffIKSolver::UpdateBoneChildren(int iBone)
{
	int i,iChild;
	for(i=m_pParent->numBoneChildren(iBone)-1;i>=0;i--)
	{
		iChild = m_pParent->getBoneChildIndex(iBone,i);
		m_pParent->getBoneMatrixGlobal(iChild) = 
			m_pParent->getBoneChild(iBone,i)->m_matRelativeToParent * m_pParent->getBoneMatrixGlobal(iBone);
		UpdateBoneChildren(iChild);
	}
}

// Only one joint version
void CCryModEffIKSolver::ApplyToBone(int nLayer)
{
	if (!m_bActive)	{
		m_fFeetAngle = 0;
		return;
	}

  int i,nbones;
	int arrBones[4] =
	{
		m_nPrimaryBone,
		m_pParent->getBoneChildIndex(m_nPrimaryBone,0),
		m_pParent->getBoneGrandChildIndex(m_nPrimaryBone,0,0),
		-1
	};
	vectorf pt[4],a,b,c,n,goal;
	float alen,blen,clen,nlen,beta,cosa,sina;

	if (m_pParent->numBoneChildren(arrBones[2]) && m_flags & ik_leg) {
		arrBones[3] = m_pParent->getBoneChildIndex (arrBones[2],0);
		nbones = 4;
	} else
		nbones = 3;

	for (i = 0; i < nbones; ++i)
		pt[i] = m_pParent->getBoneMatrixGlobal(arrBones[i]).GetTranslationOLD();
	goal = m_ptGoal;

	a = pt[1]-pt[0]; b = pt[2]-pt[1]; c = goal-pt[0];
	n = (a^b).normalized();
	alen = a.len(); blen = b.len()+m_additLen; clen = c.len();
	if (alen*clen<1E-6f)
		return;

	beta = (alen*alen+clen*clen-blen*blen)/(2*alen*clen);
	beta = min(0.9999f,max(-0.9999f,beta));
	a = a.rotated(n,beta,cry_sqrtf(1-beta*beta)).normalized();
	c/=clen; n=a^c; nlen=n.len(); 
	if (nlen<1E-6)
		return;
	n/=nlen;
	for(i=0;i<nbones;++i) {
		RotateMatrix(m_pParent->getBoneMatrixGlobal(arrBones[i]), n,a*c,nlen, pt[0]);
		pt[i] = m_pParent->getBoneMatrixGlobal(arrBones[i]).GetTranslationOLD();
	}

	c = goal-pt[1]; b = pt[2]-pt[1];
	n = b^c; nlen = n.len(); 
	if (nlen>0.001) {
		n/=nlen; beta = cry_atan2f(nlen,b*c);
		struct {float cosa, sina;}acs;
		cry_sincosf(beta, &acs.cosa);
		cosa = acs.cosa; //(float)cry_cosf(beta);
		sina = acs.sina; //(float)cry_sinf(beta);
		for(i=1;i<nbones;i++) {
			RotateMatrix(m_pParent->getBoneMatrixGlobal(arrBones[i]), n,cosa,sina,pt[1]);
			pt[i] = m_pParent->getBoneMatrixGlobal(arrBones[i]).GetTranslationOLD();
		}
	}

	if (m_GoalNormal.len2()>0) {
		c = m_pParent->getBoneMatrixGlobal(arrBones[nbones-1]).GetRow(1);
		n = c^m_GoalNormal; nlen = n.len();
		if (nlen>0.001) {
			n/=nlen; beta = cry_atan2f(nlen,c*m_GoalNormal);
			if (beta<0.95f) {
				if (fabsf(beta)>0.7f)
					beta = sgnnz(beta)*0.7f;
				m_fFeetAngle = m_fFeetAngle*(1-m_fDeltaTime*2)+beta*m_fDeltaTime*2;
				struct {float cosa, sina;}acs;
				cry_sincosf(m_fFeetAngle, &acs.cosa);
				cosa = acs.cosa; //(float)cos(m_fFeetAngle);
				sina = acs.sina; //(float)sin(m_fFeetAngle);
				for(i=2;i<nbones;i++)
					RotateMatrix(m_pParent->getBoneMatrixGlobal(arrBones[i]), n,cosa,sina,pt[2]);
			}
		}
	}
	UpdateBoneChildren(arrBones[nbones-1]);
}


void CCryModEffIKSolver::Reset()
{
}

void CCryModEffIKSolver::SetGoal(vectorf ptgoal,int flags,float addlen,vectorf goal_normal,float goal_height)
{
	m_ptGoal = ptgoal;
	m_flags = flags;
	m_additLen = addlen;
	m_GoalNormal = goal_normal;
	m_GoalHeight = goal_height;
}

bool CCryModEffIKSolver::AffectsBone(int nBone)
{
	return m_bActive &&
		(  nBone == m_nPrimaryBone
		|| nBone == m_pParent->getBoneChildIndex(m_nPrimaryBone,0)
		|| nBone == m_pParent->getBoneGrandChildIndex(m_nPrimaryBone,0,0)
		);
}