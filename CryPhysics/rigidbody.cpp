//////////////////////////////////////////////////////////////////////
//
//	Rigid Body
//	
//	File: rigidbody.cpp
//	Description : RigidBody class implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "rigidbody.h"

RigidBody::RigidBody()
{
	P.zero(); L.zero(); 
	v.zero(); w.zero(); 
	q.SetIdentity(); 
	pos.zero();
	Ibody.zero();
	bProcessed = 0; M=Minv=V = 0;
	Iinv.SetZero(); 
	Ibody.zero(); 
	Ibody_inv.zero(); 
	qfb.SetIdentity(); 
	offsfb.zero();
	Fcollision.zero(); Tcollision.zero();
	pOwner = this;
	Eunproj = 0;
	softness[0] = 0.00015f;
	softness[1] = 0.001f;
}

void RigidBody::Create(const vectorf &center,const vectorf &Ibody0,const quaternionf &q0, float volume,float mass, 
											 const quaternionf &qframe,const vectorf &posframe)
{
	float density=mass/volume;

	V = volume;M = mass; q = q0; pos = center;
	if (M>0) {
		(Ibody_inv = Ibody = Ibody0*density).invert();
		Minv = 1.0f/M;
	} else {
		Ibody_inv = Ibody.zero();
		Minv = 0;
	}
	qfb = !q*qframe;
	offsfb = (pos-posframe)*qframe;

	UpdateState();
}

void RigidBody::Add(const vectorf &center,const vectorf Ibodyop,const quaternionf &qop, float volume,float mass)
{
	if (mass==0.0f) return;
	float density=mass/volume; int i;
	matrix3x3RMf Rop,Ibodyop_mtx,Iop,Rbody2newbody,Ibody_mtx;
	Ibodyop_mtx.SetIdentity(); for(i=0;i<3;i++) Ibodyop_mtx(i,i)=Ibodyop[i]*density;

	//(!q*qop).getmatrix(Rop); //Q2M_IVO
	Rop = matrix3x3f(!q*qop);
	Iop = Rop*Ibodyop_mtx*Rop.T();

	vectorf posnew = (pos*M+center*mass)/(M+mass); 
	Ibody_mtx = Ibody;
	OffsetInertiaTensor(Ibody_mtx,(posnew-pos)*q,M);
	OffsetInertiaTensor(Iop,(posnew-center)*q,mass);
	M+=mass; V+=volume;	Ibody_mtx+=Iop; Minv=1.0f/M;

	quaternionf qframe = q*qfb;
	offsfb += (posnew-pos)*qframe;

	matrixf eigenBasis(3,3,0,Rbody2newbody.GetData());
	matrixf(3,3,mtx_symmetric,Ibody_mtx.GetData()).jacobi_transformation(eigenBasis,&Ibody.x);
	(Ibody_inv = Ibody).invert();
	quaternion qb2nb = (quaternion)Rbody2newbody;
	q *= !qb2nb; qfb = qb2nb*qfb;	pos = posnew;

	UpdateState();
}

void RigidBody::zero()
{
	M = Minv = 0;
	Ibody.zero(); Ibody_inv.zero();
}

void RigidBody::UpdateState()
{
	//q.getmatrix(R);	//Q2M_IVO
	matrix3x3f R = matrix3x3f(q);
	Iinv = R*Ibody_inv*R.T();
	if (Minv>0) {
		v = P*Minv;
		w = Iinv*L;
	}
}

void RigidBody::Step(float dt)
{
	UpdateState();	

	pos += v*dt;
	if (w.len2()*sqr(dt)<sqr(0.003f))
		q += quaternionf(0,w*0.5f)*q*dt;
	else {
		float wlen = w.len();
		//q = quaternionf(wlen*dt,w/wlen)*q;
		q = GetRotationAA(wlen*dt,w/wlen)*q;
	}

	q.Normalize();
	//q.getmatrix(R);	//Q2M_IVO
	matrix3x3f R = matrix3x3f(q);
	Iinv = R*Ibody_inv*R.T();
	w = Iinv*L;
}

void RigidBody::GetContactMatrix(const vectorf &r, matrix3x3f &K)
{
	matrix3x3f rmtx,rmtx1;
	//temporary change: crossproduct_matrix() is new a global function
	((crossproduct_matrix(r,rmtx))*=Iinv) *= crossproduct_matrix(r,rmtx1);
	K -= rmtx;
	K(0,0)+=Minv; K(1,1)+=Minv; K(2,2)+=Minv;
}

void RigidBody::AddImpulseAtContact(entity_contact *pcontact, int iop, const vectorf &dP)
{
	P += dP; L += pcontact->pt[iop]-pos^dP; 
	v = P*Minv; w = Iinv*L;
}

vectorf RigidBody::GetVelocityAtContact(entity_contact *pcontact, int iop)
{
	return v + (w^pcontact->pt[iop]-pos);
}


////////////////////////////////////////////////////////////////////////
///////////////////// Global Multibody Solver //////////////////////////
////////////////////////////////////////////////////////////////////////

struct contact_helper {
	vectorf r0,r1;
	matrix3x3f K;
	vectorf n;
	vectorf vreq;
	float Pspare;
	float friction;
	int flags;
	int iBody[2];
	int iCount;
	int iCountDst;
};
struct body_helper {
	vectorf v,w;
	float Minv,M;
	matrix3x3f Iinv;
	vectorf L;
};
struct contact_sandwich {
	int iMiddle;
	int iBread[2];
	contact_sandwich *next;
	int bProcessed;
};
struct buddy_info {
	int iBody;
	vectorf vreq;
	int flags;
	buddy_info *next;
};
struct follower_thunk {
	int iBody;
	follower_thunk *next;
};
struct body_info {
	buddy_info *pbuddy;
	contact_sandwich *psandwich;
	follower_thunk *pfollower;
	float Minv;
	int iLevel;
	int idUpdate;
	int idx;
	vectorf v_unproj,w_unproj;
	vectorf Fcollision,Tcollision;
};

int g_nContacts,g_nBodies;
static entity_contact *g_pContacts[MAX_CONTACTS];
static RigidBody *g_pBodies[MAX_CONTACTS];
static contact_helper g_Contacts[MAX_CONTACTS];
static body_helper g_Bodies[MAX_CONTACTS];
static body_info g_infos[MAX_CONTACTS];
static contact_sandwich g_sandwichbuf[MAX_CONTACTS];
static buddy_info g_buddybuf[MAX_CONTACTS];
static follower_thunk g_followersbuf[MAX_CONTACTS];
static int g_nFollowers,g_idLastUpdate,g_nUnprojLoops;
static char g_SolverBuf[262144];
static int g_iSolverBufPos;
bool g_bUsePreCG = true;
int __solver_step=0;


bool should_swap(entity_contact **pContacts,int i1,int i2) { // sub-sorts contacts by pbody[1]
	return pContacts[i1]->pbody[1]->bProcessed < pContacts[i2]->pbody[1]->bProcessed;
}
bool should_swap(RigidBody **pBodies, int i1,int i2) { // sorts bodies by level
	return g_infos[pBodies[i1]->bProcessed].iLevel < g_infos[pBodies[i2]->bProcessed].iLevel;
}
struct entity_contact_unproj : entity_contact {};
bool should_swap(entity_contact_unproj **pContacts,int i1,int i2) { // sorts contacts by newly sorted bodies 
	int iop1 = isneg(g_infos[pContacts[i1]->pbody[0]->bProcessed].iLevel-g_infos[pContacts[i1]->pbody[1]->bProcessed].iLevel);
	int iop2 = isneg(g_infos[pContacts[i2]->pbody[0]->bProcessed].iLevel-g_infos[pContacts[i2]->pbody[1]->bProcessed].iLevel);
	return g_infos[pContacts[i1]->pbody[iop1]->bProcessed].idx < g_infos[pContacts[i2]->pbody[iop2]->bProcessed].idx;
}

template<class dtype> static void qsort(dtype *pArray,int left,int right) {
	if (left>=right) return;
	int i,last; 
	swap(pArray, left,left+right>>1);
	for(last=left,i=left+1; i<=right; i++)
	if (should_swap(pArray,i,left))
		swap(pArray, ++last, i);
	swap(pArray, left,last);

	qsort(pArray, left,last-1);
	qsort(pArray, last+1,right);
}
#define bidx0(i) (g_pContacts[i]->pbody[0]->bProcessed)
#define bidx1(i) (g_pContacts[i]->pbody[1]->bProcessed)

void update_followers(int iBody,int idUpdate) {
	if (g_infos[iBody].idUpdate==idUpdate) {
		g_nUnprojLoops++; return;
	}
	g_infos[iBody].idUpdate = idUpdate;
	for(follower_thunk *pfollower=g_infos[iBody].pfollower; pfollower; pfollower=pfollower->next)	
	if (g_infos[pfollower->iBody].iLevel<=g_infos[iBody].iLevel) {
		g_infos[pfollower->iBody].iLevel = g_infos[iBody].iLevel+1;
		update_followers(pfollower->iBody,idUpdate);
	}
}

// trace_unproj_route recursive func: have 2 bodies (1 inside, 1 outside), find all possible 2nd outsides and call recurecively for each of them
//   (maintain a list of "followers" for each such body); assign level to each processed body
void trace_unproj_route(int iMiddle,int iBread);

void add_route_follower(int iBody,int iFollower) {
	follower_thunk* pfollower;
	for(pfollower=g_infos[iBody].pfollower; pfollower && pfollower->iBody!=iFollower; pfollower=pfollower->next);
	if (!pfollower && g_nFollowers<MAX_CONTACTS) {
		g_followersbuf[g_nFollowers].iBody = iFollower;
		g_followersbuf[g_nFollowers].next = g_infos[iBody].pfollower;
		g_infos[iBody].pfollower = g_followersbuf+g_nFollowers++;
		trace_unproj_route(iFollower, iBody);
	}
}

void update_level(int iBody, int iNewLevel) {
	if ((unsigned int)g_infos[iBody].iLevel>=(unsigned int)iNewLevel)
		g_infos[iBody].iLevel = max(g_infos[iBody].iLevel, iNewLevel); // -1 or >=new level
	else {
		g_infos[iBody].iLevel = iNewLevel;
		update_followers(iBody,++g_idLastUpdate);
	}
}

void trace_unproj_route(int iMiddle,int iBread) {
	int iop;
	for(contact_sandwich *psandwich=g_infos[iMiddle].psandwich; psandwich; psandwich=psandwich->next)
	if (iszero(psandwich->iMiddle-iMiddle) & ((iop=iszero(psandwich->iBread[0]-iBread)) | iszero(psandwich->iBread[1]-iBread))) {
		psandwich->bProcessed = 1;
		update_level(psandwich->iBread[iop], g_infos[iMiddle].iLevel+1);
		add_route_follower(iMiddle,psandwich->iBread[iop]);
	}
}

real ComputeRc(RigidBody *body0, entity_contact **pContacts, int nAngContacts,int nContacts) // for MINRES unprojection solver
{
	int i;
	real rT_x_rc=0;
	vectorf dP,r0;

	body0->Fcollision.zero(); body0->Tcollision.zero();
	for(i=0;i<nAngContacts;i++) // angular contacts
		body0->Tcollision += (pContacts[i]->dP*pContacts[i]->r.x)*pContacts[i]->Kinv(0,0); 
	for(;i<nContacts;i++) { // positional contacts
		r0 = pContacts[i]->pt[pContacts[i]->flags>>contact_bidx_log2&1]-body0->pos;
		dP = (pContacts[i]->dP*pContacts[i]->r.x)*pContacts[i]->Kinv(0,0);
		body0->Fcollision += dP; body0->Tcollision += r0^dP;
	}
	for(i=0;i<nAngContacts;i++)	// angular contacts
		pContacts[i]->vrel = body0->Iinv*body0->Tcollision;
	for(;i<nContacts;i++) { // positional contacts
		r0 = pContacts[i]->pt[pContacts[i]->flags>>contact_bidx_log2&1]-body0->pos;
		pContacts[i]->vrel = body0->Fcollision*body0->Minv + (body0->Iinv*body0->Tcollision^r0);
	}
	for(i=0;i<nContacts;i++)
		rT_x_rc += (pContacts[i]->dP*pContacts[i]->vrel)*pContacts[i]->r.x*pContacts[i]->Kinv(0,0);

	return rT_x_rc;
}


void InitContactSolver(float time_interval)
{
	g_nContacts = g_nBodies = 0;
	g_iSolverBufPos = 0;
	g_bUsePreCG = true;
}

char *AllocSolverTmpBuf(int size)
{
	if (g_iSolverBufPos+size<sizeof(g_SolverBuf)) {
		g_iSolverBufPos += size;
		return g_SolverBuf+g_iSolverBufPos-size;
	}
	return 0;
}

void RegisterContact(entity_contact *pcontact)
{
	if (!pcontact->pbody[0]->pOwner->OnRegisterContact(pcontact,0) || !pcontact->pbody[1]->pOwner->OnRegisterContact(pcontact,1))
		return;
	if (!(pcontact->flags & contact_maintain_count))
		pcontact->pBounceCount = &pcontact->iCount;
	g_pContacts[g_nContacts++] = pcontact;
	g_nContacts = min(g_nContacts,(int)(sizeof(g_pContacts)/sizeof(g_pContacts[0]))-1);
}

void InvokeContactSolver(float time_interval, SolverSettings *pss)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	if (g_nContacts==0) return;
	int i,j,iop,nBodies,istart,iend,istep,iter,bBounced,nBounces,bContactBounced,nConstraints,nMaxIters;
	RigidBody *body0,*body1;
	body_helper *hbody0,*hbody1;
	vectorf r0,r1,n,dp,dP,Kdp;
	float t,vrel,Ebefore,Eafter,dPn,dPtang,rtime_interval=1/time_interval;
	float e = pss->accuracyMC;
__solver_step++;

	for(i=nBodies=nConstraints=0; i<g_nContacts; i++) {
		for(iop=0;iop<2;iop++) {
			if (!g_pContacts[i]->pbody[iop]->bProcessed) {
				g_pBodies[nBodies++] = g_pContacts[i]->pbody[iop];
				g_pContacts[i]->pbody[iop]->bProcessed = nBodies;
			}
			g_Contacts[i].iBody[iop] = g_pContacts[i]->pbody[iop]->bProcessed-1;
		}
		if (!(g_pContacts[i]->flags & contact_wheel))
			g_pContacts[i]->Pspare = 0;
		if (!(g_pContacts[i]->flags & contact_use_C))
			g_pContacts[i]->C.SetIdentity();
		nConstraints -= -(g_pContacts[i]->flags & contact_constraint)>>31;
		g_pContacts[i]->P.zero();
		g_pContacts[i]->iCount = 0;
		g_pContacts[i]->bProcessed = i;

		if (g_pContacts[i]->flags & contact_constraint_3dof)
			(g_pContacts[i]->Kinv=g_pContacts[i]->K).Invert();
		else if (g_pContacts[i]->flags & contact_constraint_1dof) {
			vectorf axes[2]; int j,k; float mtx[2][2]; matrix3x3f mtx1;
			axes[0] = g_pContacts[i]->n.orthogonal().normalized();
			axes[1] = g_pContacts[i]->n ^ axes[0];
			for(j=0;j<2;j++) for(k=0;k<2;k++) mtx[j][k] = axes[j]*g_pContacts[i]->K*axes[k];
			matrixf(2,2,0,mtx[0]).invert();
			g_pContacts[i]->Kinv.SetZero();
			for(j=0;j<2;j++) for(k=0;k<2;k++) 
				g_pContacts[i]->Kinv += dotproduct_matrix(axes[j],axes[k],mtx1)*mtx[j][k];
			dotproduct_matrix(g_pContacts[i]->n, g_pContacts[i]->n, g_pContacts[i]->C) *= -1.0f;
			g_pContacts[i]->C(0,0)+=1.0f; g_pContacts[i]->C(1,1)+=1.0f; g_pContacts[i]->C(2,2)+=1.0f; 
		}	else if (g_pContacts[i]->flags & contact_constraint_2dof) {
			t = g_pContacts[i]->n*g_pContacts[i]->K*g_pContacts[i]->n;
			dotproduct_matrix(g_pContacts[i]->n, g_pContacts[i]->n, g_pContacts[i]->C);
			(g_pContacts[i]->Kinv = g_pContacts[i]->C) /= t;
		} else if (g_pContacts[i]->friction<0.01f)
			dotproduct_matrix(g_pContacts[i]->n, g_pContacts[i]->n, g_pContacts[i]->C);
	}

	g_nBodies = nBodies;
	for(i=0,Ebefore=0;i<nBodies;i++) {
		Ebefore += g_pBodies[i]->P*g_pBodies[i]->v + g_pBodies[i]->L*g_pBodies[i]->w;
		g_pBodies[i]->bProcessed = 0; g_pBodies[i]->Eunproj = 0;
	}
	if (Ebefore < nBodies*sqr(pss->minSeparationSpeed))
		Ebefore = nBodies*sqr(pss->minSeparationSpeed);

	nMaxIters = pss->nMaxMCiters;
	/*if (pss->nMaxMCiters!=pss->nMaxMCitersHopeless) {
		// check if any body contacts with at least 2 bodies that are more than 50 times heavier than it
		for(i=0;i<g_nContacts;i++) {
			iop = isneg(g_pContacts[i]->pbody[0]->Minv-g_pContacts[i]->pbody[1]->Minv);
			if (g_pContacts[i]->pbody[iop]->Minv > g_pContacts[i]->pbody[iop^1]->Minv*pss->maxMassRatioMC) {
				if (!g_pContacts[i]->pbody[iop]->bProcessed)
					g_pContacts[i]->pbody[iop]->bProcessed = (int)g_pContacts[i]->pbody[iop^1];
				else if (g_pContacts[i]->pbody[iop]->bProcessed != (int)g_pContacts[i]->pbody[iop^1])
					break;
			}
		}
		if (i<g_nContacts)
			nMaxIters = pss->nMaxMCitersHopeless;
		else { // calculate maximum body 'level' in the contact graph
			float minMinv = 1E10f;
			for(i=0;i<nBodies;i++) minMinv = min(minMinv,g_pBodies[i]->Minv);
			minMinv = minMinv*1.001f+0.001f;
			for(i=iend=0;i<nBodies;i++) g_pBodies[i]->bProcessed = -isneg(g_pBodies[i]->Minv-minMinv);
			do {
				for(i=bBounced=0;i<g_nContacts;i++) {
					if ((g_pContacts[i]->pbody[0]->bProcessed & g_pContacts[i]->pbody[0]->bProcessed)==-1)
						bBounced = 1;
					else if ((g_pContacts[i]->pbody[0]->bProcessed | g_pContacts[i]->pbody[0]->bProcessed)==-1) {
						iop = g_pContacts[i]->pbody[0]->bProcessed>>31 & 1;
						iend = max(iend, g_pContacts[i]->pbody[iop^1]->bProcessed = g_pContacts[i]->pbody[iop]->bProcessed+1);
					}
				}
			} while (bBounced);
			if (iend>pss->nMaxStackSizeMC)
				nMaxIters = pss->nMaxMCitersHopeless;
		}
	}*/
	for(i=0;i<nBodies;i++) g_pBodies[i]->bProcessed = 0;

	if (g_bUsePreCG && g_nContacts<20) {
		FRAME_PROFILER( "PreCG",GetISystem(),PROFILE_PHYSICS );

		real a,b,r2,r2new,pAp,vmax,vdiff;

		// try to drive all contact velocities to zero by using conjugate gradient
		for(i=0,r2=0,vmax=0;i<g_nContacts;i++) {
			body0 = g_pContacts[i]->pbody[0]; body1 = g_pContacts[i]->pbody[1];
			if (!(g_pContacts[i]->flags & contact_angular)) {
				r0 = g_pContacts[i]->pt[0]-body0->pos; r1 = g_pContacts[i]->pt[1]-body1->pos;
				dp = body0->v+(body0->w^r0) - body1->v-(body1->w^r1); 
			} else dp = body0->w-body1->w;
			g_pContacts[i]->dP = g_pContacts[i]->r = g_pContacts[i]->vreq - g_pContacts[i]->C*dp;
			g_pContacts[i]->P.zero(); r2 += g_pContacts[i]->r.len2(); vmax = max((float)vmax,g_pContacts[i]->r.len2());
		}
		iter = g_nContacts*6;
		
		do {
			for(i=0;i<nBodies;i++) { g_pBodies[i]->Fcollision.zero(); g_pBodies[i]->Tcollision.zero(); }
			for(i=0;i<g_nContacts;i++) {
				body0 = g_pContacts[i]->pbody[0]; body1 = g_pContacts[i]->pbody[1]; 
				if (!(g_pContacts[i]->flags & contact_angular)) {
					r0 = g_pContacts[i]->pt[0]-body0->pos; r1 = g_pContacts[i]->pt[1]-body1->pos;
					body0->Fcollision += g_pContacts[i]->dP; body0->Tcollision += r0^g_pContacts[i]->dP;
					body1->Fcollision -= g_pContacts[i]->dP; body1->Tcollision -= r1^g_pContacts[i]->dP;
				} else {
					body0->Tcollision += g_pContacts[i]->dP; body1->Tcollision -= g_pContacts[i]->dP;
				}
			}
			for(i=0;i<g_nContacts;i++) {
				body0 = g_pContacts[i]->pbody[0]; body1 = g_pContacts[i]->pbody[1];
				if (!(g_pContacts[i]->flags & contact_angular)) {
					r0 = g_pContacts[i]->pt[0]-body0->pos; r1 = g_pContacts[i]->pt[1]-body1->pos;
					dp = body0->Fcollision*body0->Minv + (body0->Iinv*body0->Tcollision^r0);
					dp -= body1->Fcollision*body1->Minv + (body1->Iinv*body1->Tcollision^r1);
				} else
					dp = body0->Iinv*body0->Tcollision - body1->Iinv*body1->Tcollision;
				g_pContacts[i]->vrel = g_pContacts[i]->C*dp;
			}
			
			for(i=0,pAp=0;i<g_nContacts;i++)
				pAp += g_pContacts[i]->vrel*g_pContacts[i]->dP;
			if (sqr(pAp)<1E-30) break;
			a = r2/pAp;	
			for(i=0,r2new=0;i<g_nContacts;i++) {
				r2new += (g_pContacts[i]->r -= g_pContacts[i]->vrel*a).len2();
				g_pContacts[i]->P += g_pContacts[i]->dP*a;
			}
			if (r2new>r2*500)
				break;
			b = r2new/r2; r2=r2new;
			for(i=0,vmax=0;i<g_nContacts;i++) {
				(g_pContacts[i]->dP*=b)+=g_pContacts[i]->r;
				vmax = max((float)vmax,g_pContacts[i]->r.len2());
			}
		} while (--iter && vmax>sqr(e));

		for(i=0;i<nBodies;i++) { g_pBodies[i]->Fcollision.zero(); g_pBodies[i]->Tcollision.zero(); }
		for(i=0;i<g_nContacts;i++) {
			body0 = g_pContacts[i]->pbody[0];	body1 = g_pContacts[i]->pbody[1];
			if (!(g_pContacts[i]->flags & contact_angular)) {
				body0->Fcollision += g_pContacts[i]->P; body0->Tcollision += g_pContacts[i]->pt[0]-body0->pos ^ g_pContacts[i]->P;
				body1->Fcollision -= g_pContacts[i]->P; body1->Tcollision -= g_pContacts[i]->pt[1]-body1->pos ^ g_pContacts[i]->P;
			}	else {
				body0->Tcollision += g_pContacts[i]->P;	body1->Tcollision -= g_pContacts[i]->P;
			}
		}
		for(i=0,Eafter=0;i<nBodies;i++)
			Eafter += (g_pBodies[i]->P+g_pBodies[i]->Fcollision)*(g_pBodies[i]->v+g_pBodies[i]->Fcollision*g_pBodies[i]->Minv) + 
				(g_pBodies[i]->L+g_pBodies[i]->Tcollision)*(g_pBodies[i]->w+g_pBodies[i]->Iinv*g_pBodies[i]->Tcollision);
		for(i=0;i<g_nContacts;i++) {
			n = g_pContacts[i]->n; dPn = g_pContacts[i]->P*n; dPtang = (g_pContacts[i]->P-n*dPn).len2();
			if (!(g_pContacts[i]->flags & contact_angular)) {
				dp = g_pContacts[i]->P*(g_pContacts[i]->pbody[0]->Minv+g_pContacts[i]->pbody[1]->Minv);
				vdiff = -0.004f;
			} else {
				dp = g_pContacts[i]->pbody[0]->Iinv*g_pContacts[i]->P + g_pContacts[i]->pbody[1]->Iinv*g_pContacts[i]->P;
				vdiff = -0.015f;
			}
			if (!(g_pContacts[i]->flags & contact_constraint) &&
					(dp*n<-0.05f || // allow to pull bodies at contacts slightly
					dPtang>sqr(dPn*g_pContacts[i]->friction)+0.001f || 
					(g_pContacts[i]->r+g_pContacts[i]->vreq)*n<vdiff))
				break;
		}

		if (i==g_nContacts && Eafter<Ebefore*1.5f && vmax<sqr(0.01)) { // conjugate gradient yielded acceptable results, apply these impulses and quit
			for(i=0;i<nBodies;i++) {
				g_pBodies[i]->P += g_pBodies[i]->Fcollision; g_pBodies[i]->L += g_pBodies[i]->Tcollision;
				if (g_pBodies[i]->M>0) {
					g_pBodies[i]->v = g_pBodies[i]->P*g_pBodies[i]->Minv; g_pBodies[i]->w = g_pBodies[i]->Iinv*g_pBodies[i]->L;
				}
				g_pBodies[i]->Fcollision *= rtime_interval; g_pBodies[i]->Tcollision *= rtime_interval;
			}
			for(i=0;i<g_nContacts;i++) {
				g_pContacts[i]->vrel = g_pContacts[i]->vreq-g_pContacts[i]->r;
				g_pContacts[i]->Pspare = 1.0f; // indicates that contact is sticky
			}
			return;
		}
	}

	for(i=0; i<g_nContacts; i++) {
		g_Contacts[i].r0 = g_pContacts[i]->pt[0]-g_pContacts[i]->pbody[0]->pos;
		g_Contacts[i].r1 = g_pContacts[i]->pt[1]-g_pContacts[i]->pbody[1]->pos;
		g_Contacts[i].K = g_pContacts[i]->K;
		g_Contacts[i].n = g_pContacts[i]->n;
		g_Contacts[i].vreq = g_pContacts[i]->vreq;
		g_Contacts[i].Pspare = g_pContacts[i]->Pspare;
		g_Contacts[i].flags = g_pContacts[i]->flags;
		g_Contacts[i].friction = g_pContacts[i]->friction;
		g_Contacts[i].iCount = g_pContacts[i]->iCount;
		g_Contacts[i].iCountDst = ((entity_contact*)((char*)g_pContacts[i]->pBounceCount-
			((char*)&g_pContacts[i]->iCount-(char*)g_pContacts[i])))->bProcessed;
	}
	for(i=0;i<nBodies;i++) {
		g_pBodies[i]->Fcollision = g_pBodies[i]->P; g_pBodies[i]->Tcollision = g_Bodies[i].L = g_pBodies[i]->L;
		g_Bodies[i].v = g_pBodies[i]->v; g_Bodies[i].w = g_pBodies[i]->w;
		g_Bodies[i].Minv = g_pBodies[i]->Minv; g_Bodies[i].Iinv = g_pBodies[i]->Iinv; g_Bodies[i].M = g_pBodies[i]->M;
	}

	iter=nBounces=0; Eafter = 0;

//bBounced=1; if (false)
	{
		FRAME_PROFILER( "LCPMC",GetISystem(),PROFILE_PHYSICS );

		do {
			bBounced = 0;
			//istep = ((iter&1)<<1)-1;
			//istart = g_nContacts-1 & -(iter&1^1);
			//iend = (g_nContacts+1 & -(iter&1))-1;
			istart=0; iend=g_nContacts; istep=1;

			for(i=istart; i!=iend; i+=istep) {
				if (g_Contacts[i].iCount >= (g_Contacts[i].flags & contact_count_mask))	{
					//body0 = g_Contacts[i]->pbody[0]; body1 = g_pContacts[i]->pbody[1];
					hbody0 = g_Bodies+g_Contacts[i].iBody[0]; hbody1 = g_Bodies+g_Contacts[i].iBody[1];
					n = g_Contacts[i].n; 
					if (!(g_Contacts[i].flags & contact_angular)) {
						//r0 = g_pContacts[i]->pt[0]-body0->pos; r1 = g_pContacts[i]->pt[1]-body1->pos;
						r0 = g_Contacts[i].r0; r1 = g_Contacts[i].r1;
						dp = hbody0->v+(hbody0->w^r0) - hbody1->v-(hbody1->w^r1);
						//g_pContacts[i]->vrel = 
					} else
						dp = hbody0->w-hbody1->w;
					dp -= g_Contacts[i].vreq;
					if (g_Contacts[i].flags & contact_use_C)
						dp = g_pContacts[i]->C*dp;
					bContactBounced = 0;

					if (g_Contacts[i].flags & contact_constraint) {
						if ((g_pContacts[i]->C*dp).len2()>sqr(e)) {
							dP = g_pContacts[i]->Kinv*-dp;
							bContactBounced = 1; 
						}
					} else if (!(g_Contacts[i].flags & contact_wheel)) {
						if ((vrel=dp*n)<0 &&
								(isneg(e-fabs_tpl(vrel)) | isneg(0.0001f-g_Contacts[i].Pspare) & isneg(sqr(pss->minSeparationSpeed)-(dp-n*vrel).len2()))) 
						//if ((vrel=dp*n)<0 && (vrel<-0.003 || g_pContacts[i]->Pspare>0.0001f && (dp-n*vrel).len2()>sqr(pss->minSeparationSpeed)))
						{
							if (g_Contacts[i].friction>0.01f) {
								dP = dp*(-dp.len2()/(dp*g_Contacts[i].K*dp));
								g_Contacts[i].Pspare += dPn=(dP*n)*g_Contacts[i].friction;
								dPtang = sqrt_tpl(max(0.0f,dP.len2()-sqr(dP*n)));
								g_Contacts[i].Pspare -= dPtang;
								if (g_Contacts[i].Pspare<0) {	// friction cannot stop sliding
									dp += (dp-n*vrel)*((g_Contacts[i].Pspare)/dPtang); // remove part of dp that friction cannot stop
									Kdp = g_Contacts[i].K*dp;
									if (sqr(Kdp*n) < Kdp.len2()*0.04f)	// switch to frictionless contact in dangerous cases
										dP = n*-vrel/(n*g_Contacts[i].K*n);
									else
										dP = dp*-vrel/(n*Kdp); // apply impulse along dp so that it stops normal component
									g_Contacts[i].Pspare = 0;
								}
							} else
								dP = n*-vrel/(n*g_Contacts[i].K*n);
							bContactBounced = 1; 
						}
					} else if (dp.len2()>sqr(pss->minSeparationSpeed) && g_Contacts[i].Pspare>g_pContacts[i]->pbody[0]->M*0.01f) {
						dP = dp*(-dp.len2()/(dp*g_Contacts[i].K*dp));
						dPn = (dP*n)*g_Contacts[i].friction;
						dPtang = sqrt_tpl(max(0.0f,dP.len2()-sqr(dP*n)));
						if (dPtang > dPn*1.01f)	{
							if (g_Contacts[i].Pspare*0.5f < dPtang-dPn)	{
								dP *= g_Contacts[i].Pspare*0.5f/(dPtang-dPn);
								g_Contacts[i].Pspare *= 0.5f;
							} else
								g_Contacts[i].Pspare -= dPtang-dPn;
							dP -= n*min(0.0f,dP*n);
						}
						bContactBounced = 1; 
					}

					if (bContactBounced) {
						if (g_Contacts[i].flags & contact_use_C)
							dP = g_pContacts[i]->C*dP;
						if (!(g_Contacts[i].flags & contact_angular)) {
							hbody0->v += dP*hbody0->Minv; hbody0->w += hbody0->Iinv*(dp=r0^dP); hbody0->L += dp;
							hbody1->v -= dP*hbody1->Minv; hbody1->w -= hbody1->Iinv*(dp=r1^dP);	hbody1->L -= dp;
						}	else {
							hbody0->w += hbody0->Iinv*dP; hbody0->L += dP;
							hbody1->w -= hbody1->Iinv*dP; hbody1->L -= dP;
						}
						g_Contacts[g_Contacts[i].iCountDst].iCount++;
						/*if (!(g_Contacts[i].flags & contact_angular)) {
							body0->P+=dP; body0->L+=r0^dP; body1->P-=dP; body1->L-=r1^dP;
						}	else {
							body0->L+=dP; body1->L-=dP;
						}
						if (body0->M>0) { body0->v=body0->P*body0->Minv; body0->w=body0->Iinv*body0->L; }
						if (body1->M>0) { body1->v=body1->P*body1->Minv; body1->w=body1->Iinv*body1->L; }
						g_pContacts[i]->P += dP;
						(*g_pContacts[i]->pBounceCount)++;*/
						bBounced++;	nBounces++;
					}
				}
				g_Contacts[i].iCount = 0;
			} 

			for(i=0,Eafter=0; i<nBodies; i++)
				Eafter += g_Bodies[i].v.len2()*g_Bodies[i].M + g_Bodies[i].L*g_Bodies[i].w;
			nBounces += g_nContacts-bBounced >> 4;

		} while (bBounced && nBounces<nMaxIters && Eafter<Ebefore*3.0f);

		for(i=0; i<nBodies; i++) {
			g_pBodies[i]->P = (g_pBodies[i]->v=g_Bodies[i].v)*g_pBodies[i]->M; 
			g_pBodies[i]->L = g_pBodies[i]->q*(g_pBodies[i]->Ibody*(!g_pBodies[i]->q*(g_pBodies[i]->w = g_Bodies[i].w)));
		}
		for(i=0; i<g_nContacts; i++)
			g_pContacts[i]->Pspare = g_Contacts[i].Pspare;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (bBounced && pss->nMaxLCPCGiters>0) { // use several iterations of CG solver, solving only for non-separating contacts
		unsigned int iClass;
		int cgiter,bStateChanged,n1dofContacts,n2dofContacts,nAngContacts,nFric0Contacts,nFricInfContacts,
			nContacts,iSortedContacts[6],flags,bNoImprovement;
		entity_contact *pContacts[sizeof(g_pContacts)/sizeof(g_pContacts[0])];
		real a,b,r2,r2new,pAp;
		float vmax=0;
		vectorf vreq;
		e = pss->accuracyLCPCG;

		// prepare for CG solver: calculate target residuals (they will be used as starting residuals during each iteration) and vrel's
		for(i=bBounced=0; i<g_nContacts; i++) {
			body0 = g_pContacts[i]->pbody[0]; body1 = g_pContacts[i]->pbody[1];
//g_pContacts[i]->Pspare = 0;//g_pContacts[i]->flags&contact_angular ? 0:1;
			if (!(g_pContacts[i]->flags & contact_angular)) {
				r0 = g_pContacts[i]->pt[0]-body0->pos; r1 = g_pContacts[i]->pt[1]-body1->pos;
				g_pContacts[i]->vrel = body0->v+(body0->w^r0) - body1->v-(body1->w^r1); 
			} else
				g_pContacts[i]->vrel = body0->w-body1->w;
			vreq = g_pContacts[i]->vreq;
			if (g_pContacts[i]->flags & contact_use_C)
				g_pContacts[i]->vrel  = g_pContacts[i]->C*g_pContacts[i]->vrel;

			if (g_pContacts[i]->flags & contact_wheel) {
				if (g_pContacts[i]->Pspare>g_pContacts[i]->pbody[0]->M*0.01f)
					g_pContacts[i]->flags = (g_pContacts[i]->flags & contact_use_C) | contact_constraint_3dof;
				else
					g_pContacts[i]->flags = contact_count_mask;	// don't solve for this contact
			}

			// remove constraint flags from contacts with counters, since constraints cannot to removed from solving process
			g_pContacts[i]->flags &= contact_count_mask | (g_pContacts[i]->flags&contact_count_mask)-1>>31;
			if (g_pContacts[i]->flags & contact_constraint_1dof)
				g_pContacts[i]->r0 = (g_pContacts[i]->vrel -= g_pContacts[i]->n*(g_pContacts[i]->vrel*g_pContacts[i]->n));
			else if (g_pContacts[i]->flags&contact_constraint_2dof || !(g_pContacts[i]->flags&contact_constraint_3dof || g_pContacts[i]->Pspare>0)) {
				g_pContacts[i]->r0(g_pContacts[i]->n*g_pContacts[i]->vrel,0,0); vreq(g_pContacts[i]->n*vreq,0,0);
				g_pContacts[i]->vrel = g_pContacts[i]->n*g_pContacts[i]->r0.x;
				g_pContacts[i]->Kinv.SetZero();
				g_pContacts[i]->Kinv(0,0) = 1.0f/(g_pContacts[i]->n*g_pContacts[i]->K*g_pContacts[i]->n);
			} else {
				g_pContacts[i]->r0 = g_pContacts[i]->vrel;
				if (!(g_pContacts[i]->flags & contact_constraint))
					(g_pContacts[i]->Kinv = g_pContacts[i]->K).Invert(); // for constraints it's computed earlier
			}
			g_pContacts[i]->r0.Flip();
			
			if (g_pContacts[i]->flags & contact_constraint || g_pContacts[i]->vrel*g_pContacts[i]->n<0)	{
				iop = g_pContacts[i]->flags>>contact_angular_log2&1;
				bBounced += isneg(sqr(body0->softness[iop]+body1->softness[iop])*0.25f - (g_pContacts[i]->r0+vreq).len2()*sqr(time_interval));
			}
			g_pContacts[i]->flags &= ~contact_solve_for;
		}
		if (bBounced) {
			{FRAME_PROFILER( "LCPCG",GetISystem(),PROFILE_PHYSICS );

			cgiter = pss->nMaxLCPCGiters;
			for(i=0;i<nBodies;i++) {
				g_infos[i].Fcollision = g_pBodies[i]->Fcollision;
				g_infos[i].Tcollision = g_pBodies[i]->Tcollision;
			}

			do {
				// sort contacts for the current iteration in the following way:
				// 1dof angular (always remove v-n*(v*n))					|	angular
				// 2dof angular constraints (always remove n*v)		|	angular
				// angular limits (remove n*v if <0)							|	angular
				// frictionless contacts (remove n*v if <0)				|	positional
				// infinite friction contacts (remove v if n*v<0)	|	positional
				// 3dof constraints (always remove v)							|	positional
				for(i=0;i<6;i++) iSortedContacts[i]=0;
				for(i=bStateChanged=0; i<g_nContacts; i++) {
					flags = g_pContacts[i]->flags;
					if (!(g_pContacts[i]->flags&contact_count_mask)) {
						g_pContacts[i]->flags |= contact_solve_for;
						if (g_pContacts[i]->flags & contact_constraint_1dof) iSortedContacts[0]++;
						else if (g_pContacts[i]->flags & contact_constraint_2dof) iSortedContacts[1]++;
						else if (g_pContacts[i]->flags & contact_constraint_3dof) iSortedContacts[5]++;
						else {
							if (g_pContacts[i]->vrel*g_pContacts[i]->n<0) {
								if (g_pContacts[i]->flags & contact_angular) iSortedContacts[2]++;
								else if (g_pContacts[i]->Pspare>0) iSortedContacts[4]++;
								else iSortedContacts[3]++;
							}	else
								g_pContacts[i]->flags &= ~contact_solve_for;
							bStateChanged += iszero((flags^g_pContacts[i]->flags) & contact_solve_for)^1;
						}
					}
				}
				if (!bStateChanged && vmax<sqr(e))
					break;
				iter = iSortedContacts[0]*2 + iSortedContacts[1]+iSortedContacts[2]+iSortedContacts[3] + (iSortedContacts[4]+iSortedContacts[5])*3;
				for(i=1;i<6;i++) iSortedContacts[i]+=iSortedContacts[i-1];
				n1dofContacts=iSortedContacts[0]; n2dofContacts=iSortedContacts[1]; nAngContacts=iSortedContacts[2]; 
				nFric0Contacts=iSortedContacts[3];nFricInfContacts=iSortedContacts[4]; nContacts=iSortedContacts[5];
				for(i=0,r2=0; i<g_nContacts; i++) {
					if (g_pContacts[i]->flags & contact_constraint_1dof) iClass = 0;
					else if (g_pContacts[i]->flags & contact_constraint_2dof) iClass = 1;
					else if (g_pContacts[i]->flags & contact_constraint_3dof) iClass = 5;
					else if (g_pContacts[i]->flags & contact_solve_for) {
						if (g_pContacts[i]->flags & contact_angular) iClass = 2;
						else if (g_pContacts[i]->Pspare>0) iClass = 4;
						else iClass = 3;
					} else
						continue;
					pContacts[--iSortedContacts[iClass]] = g_pContacts[i];
					g_pContacts[i]->vrel = g_pContacts[i]->Kinv*(g_pContacts[i]->r = g_pContacts[i]->r0);
					if (g_pContacts[i]->flags & contact_use_C)
						g_pContacts[i]->vrel = g_pContacts[i]->C*g_pContacts[i]->vrel;
					r2 += g_pContacts[i]->vrel*g_pContacts[i]->r0;
					g_pContacts[i]->dP = iClass-1u<3u ? g_pContacts[i]->n*(g_pContacts[i]->dPn=g_pContacts[i]->vrel.x) : g_pContacts[i]->vrel;
					g_pContacts[i]->P.zero();
				}
				if (!(cgiter==1 || !bStateChanged))	{
					iter = min(iter,pss->nMaxLCPCGsubiters);
					if (iter*nContacts > pss->nMaxLCPCGmicroiters)
						iter = max(5,pss->nMaxLCPCGmicroiters/nContacts);
				} else {
					iter = min(iter*2,pss->nMaxLCPCGsubitersFinal);
					if (iter*nContacts > pss->nMaxLCPCGmicroitersFinal)
						iter = max(10,pss->nMaxLCPCGmicroitersFinal/nContacts);
				}
				bNoImprovement = 0;
				
				do {
					for(i=0;i<nBodies;i++) { g_pBodies[i]->Fcollision.zero(); g_pBodies[i]->Tcollision.zero(); }
					for(i=0;i<nAngContacts;i++) { // angular contacts
						pContacts[i]->pbody[0]->Tcollision += pContacts[i]->dP; 
						pContacts[i]->pbody[1]->Tcollision -= pContacts[i]->dP;
					} for(;i<nContacts;i++) { // positional contacts
						body0 = pContacts[i]->pbody[0]; body1 = pContacts[i]->pbody[1]; 
						r0 = pContacts[i]->pt[0]-body0->pos; r1 = pContacts[i]->pt[1]-body1->pos;
						body0->Fcollision += pContacts[i]->dP; body0->Tcollision += r0^pContacts[i]->dP;
						body1->Fcollision -= pContacts[i]->dP; body1->Tcollision -= r1^pContacts[i]->dP;
					}

					for(i=0;i<nAngContacts;i++) {	// angular contacts
						body0 = pContacts[i]->pbody[0]; body1 = pContacts[i]->pbody[1];
						pContacts[i]->vrel = body0->Iinv*body0->Tcollision - body1->Iinv*body1->Tcollision;
					} for(;i<nContacts;i++) { // positional contacts
						body0 = pContacts[i]->pbody[0]; body1 = pContacts[i]->pbody[1];
						r0 = pContacts[i]->pt[0]-body0->pos; r1 = pContacts[i]->pt[1]-body1->pos;
						pContacts[i]->vrel  = body0->Fcollision*body0->Minv + (body0->Iinv*body0->Tcollision^r0);
						pContacts[i]->vrel -= body1->Fcollision*body1->Minv + (body1->Iinv*body1->Tcollision^r1);
					}

					pAp = 0;
					for(i=0;i<n1dofContacts;i++) { // 1-dof contacts (remove 1 axis from vrel)
						pContacts[i]->vrel -= pContacts[i]->n*(pContacts[i]->vrel*pContacts[i]->n);
						pAp += pContacts[i]->vrel*pContacts[i]->dP;
					} for(;i<nFric0Contacts;i++) { // general 2-dof contacts (remove 2 axes from vrel - leave 1)
						pContacts[i]->vrel.x = pContacts[i]->n*pContacts[i]->vrel;
						pAp += pContacts[i]->vrel.x*pContacts[i]->dPn;
					} for(;i<nContacts;i++)	// general 3-dof contacts (leave vrel untouched - all directions need to be 0ed)
						pAp += pContacts[i]->vrel*pContacts[i]->dP;

					a = min((real)50.0,r2/max((real)1E-10,pAp)); r2new=0;
					i = nFric0Contacts&nFric0Contacts-nContacts>>31;
					if (nFric0Contacts-n1dofContacts<nContacts) do { // contacts with 3(and 2)-components vel
						pContacts[i]->vrel = pContacts[i]->Kinv*(pContacts[i]->r -= pContacts[i]->vrel*a);
						if (pContacts[i]->flags & contact_use_C)
							pContacts[i]->vrel = pContacts[i]->C*pContacts[i]->vrel;
						r2new += pContacts[i]->vrel*pContacts[i]->r;
						pContacts[i]->P += pContacts[i]->dP*a;
						i = i+1&i+1-nContacts>>31;
					} while(i!=n1dofContacts);
					for(;i<nFric0Contacts;i++) { // contacts with 1-component vel
						pContacts[i]->vrel.x = pContacts[i]->Kinv(0,0)*(pContacts[i]->r.x -= pContacts[i]->vrel.x*a);
						r2new += pContacts[i]->vrel.x*pContacts[i]->r.x;
						pContacts[i]->P.x += pContacts[i]->dPn*a;
					}

					bNoImprovement = max(0,bNoImprovement-sgnnz(r2-r2new - r2*pss->minLCPCGimprovement));
					b = r2new/r2; r2=r2new;	vmax=0;
					i = nFric0Contacts&nFric0Contacts-nContacts>>31;
					if (nFric0Contacts-n1dofContacts<nContacts) do { // contacts with 3(and 2)-components vel
						(pContacts[i]->dP*=b) += pContacts[i]->vrel;
						vmax = max(vmax,pContacts[i]->r.len2());
						i = i+1&i+1-nContacts>>31;
					} while(i!=n1dofContacts);
					for(;i<nFric0Contacts;i++) { // contacts with 1-component vel
						(pContacts[i]->dPn*=b) += pContacts[i]->vrel.x;
						pContacts[i]->dP = pContacts[i]->n*pContacts[i]->dPn;
						vmax = max(vmax,sqr(pContacts[i]->r.x));
					}

				} while (--iter && vmax>sqr(e) && !(bNoImprovement>pss->nMaxLCPCGFruitlessIters && vmax<sqr(pss->accuracyLCPCGnoimprovement)));

				for(i=n1dofContacts;i<nFric0Contacts;i++) pContacts[i]->P = pContacts[i]->n*pContacts[i]->P.x;
				if (!bStateChanged)
					break;

				// calculate how the impulses affect the contacts excluded from this iteration
				for(i=0;i<nBodies;i++) { g_pBodies[i]->Fcollision.zero(); g_pBodies[i]->Tcollision.zero(); }
				for(i=0;i<nAngContacts;i++) { // angular contacts
					pContacts[i]->pbody[0]->Tcollision += pContacts[i]->P; 
					pContacts[i]->pbody[1]->Tcollision -= pContacts[i]->P;
				} for(;i<nContacts;i++) { // positional contacts
					body0 = pContacts[i]->pbody[0]; body1 = pContacts[i]->pbody[1]; 
					r0 = pContacts[i]->pt[0]-body0->pos; r1 = pContacts[i]->pt[1]-body1->pos;
					body0->Fcollision += pContacts[i]->P; body0->Tcollision += r0^pContacts[i]->P;
					body1->Fcollision -= pContacts[i]->P; body1->Tcollision -= r1^pContacts[i]->P;
				}
				for(i=0;i<g_nContacts;i++) if (!(g_pContacts[i]->flags & (contact_solve_for|contact_count_mask))) {
					body0 = g_pContacts[i]->pbody[0]; body1 = g_pContacts[i]->pbody[1];
					if (!(g_pContacts[i]->flags & contact_angular)) {
						r0 = g_pContacts[i]->pt[0]-body0->pos; r1 = g_pContacts[i]->pt[1]-body1->pos;
						g_pContacts[i]->vrel  = body0->v + body0->Fcollision*body0->Minv + (body0->w+body0->Iinv*body0->Tcollision ^ r0);
						g_pContacts[i]->vrel -= body1->v + body1->Fcollision*body1->Minv + (body1->w+body1->Iinv*body1->Tcollision ^ r1);
					} else
						g_pContacts[i]->vrel = body0->w+body0->Iinv*body0->Tcollision - body1->w-body1->Iinv*body1->Tcollision;
					if (g_pContacts[i]->flags & contact_use_C)
						g_pContacts[i]->vrel = g_pContacts[i]->C*g_pContacts[i]->vrel;
				}
				if (cgiter>2) for(i=n2dofContacts; i<nFricInfContacts; i++)
					pContacts[i]->vrel = -pContacts[i]->P*max(pContacts[i]->pbody[0]->Minv,pContacts[i]->pbody[1]->Minv)-pContacts[i]->n*(e*3);
				else for(i=n2dofContacts; i<nFricInfContacts; i++) // if we reached the last iteration, keep the current 'solve for' contacts unconditionally
					pContacts[i]->vrel = -pContacts[i]->n;
			} while(--cgiter>0);


			bBounced = 1;
			if (cgiter<pss->nMaxLCPCGiters) {
				for(i=0,vmax=0;i<nAngContacts;i++) {
					vmax = max(vmax,(pContacts[i]->pbody[0]->Ibody*pContacts[i]->P).len2());
					vmax = max(vmax,(pContacts[i]->pbody[1]->Ibody*pContacts[i]->P).len2());
				}	if (_isnan(vmax) || vmax>sqr(pss->maxwCG))
					bBounced = 0;
				for(vmax=0;i<nContacts;i++) {
					vmax = max(vmax,sqr(pContacts[i]->pbody[0]->Minv)*pContacts[i]->P.len2());
					vmax = max(vmax,sqr(pContacts[i]->pbody[1]->Minv)*pContacts[i]->P.len2());
				}	if (_isnan(vmax) || vmax>sqr(pss->maxvCG))
					bBounced = 0;

				if (bBounced) {
					for(;i<nContacts;i++) { // positional contacts
						body0 = pContacts[i]->pbody[0]; body1 = pContacts[i]->pbody[1]; 
						body0->P += pContacts[i]->P; body0->L += r0^pContacts[i]->P;
						body1->P -= pContacts[i]->P;
					}
					// apply P from the last CG iteration
					for(i=0;i<nAngContacts;i++) { // angular contacts
						pContacts[i]->pbody[0]->L += pContacts[i]->P; 
						pContacts[i]->pbody[1]->L -= pContacts[i]->P;
					} for(;i<nContacts;i++) { // positional contacts
						body0 = pContacts[i]->pbody[0]; body1 = pContacts[i]->pbody[1]; 
						r0 = pContacts[i]->pt[0]-body0->pos; r1 = pContacts[i]->pt[1]-body1->pos;
						body0->P += pContacts[i]->P; body0->L += r0^pContacts[i]->P;
						body1->P -= pContacts[i]->P; body1->L -= r1^pContacts[i]->P;
					}
					for(i=0;i<nBodies;i++) if (g_pBodies[i]->M>0) {
						g_pBodies[i]->v = g_pBodies[i]->P*g_pBodies[i]->Minv; g_pBodies[i]->w = g_pBodies[i]->Iinv*g_pBodies[i]->L;
					}
				} else for(i=0; i<nBodies; i++) {
					j = g_pBodies[i]->bProcessed;
					g_pBodies[i]->Fcollision = g_infos[j].Fcollision;
					g_pBodies[i]->Tcollision = g_infos[j].Tcollision;
				}
			}

			}//"LCPCG"

			if (bBounced) {
				FRAME_PROFILER( "LCPCG-unproj",GetISystem(),PROFILE_PHYSICS );

				///////////////////////////////////////////////////////////////////////////////////
				// now, use a separate solver for unprojections (unproject each body independently)
				int nBuddies,nSandwiches,iMinLevel,iMaxLevel,iLevel;
				buddy_info *pbuddy0,*pbuddy1;
				vectorf vreq;
				float minMinv;

				for(i=0;i<nBodies;i++) {
					g_pBodies[i]->bProcessed=i; g_infos[i].pbuddy=0; g_infos[i].iLevel=-1; 
					g_infos[i].psandwich=0; g_infos[i].pfollower=0; g_infos[i].idUpdate=0;
				}
				// require that the all contacts are grouped by pbody[0]
				for(istart=nBuddies=0; istart<g_nContacts; istart=iend) {
					// sub-group contacts relating to each pbody[0] by pbody[1] (using quick sort)
					for(iend=istart+1; iend<g_nContacts && g_pContacts[iend]->pbody[0]==g_pContacts[istart]->pbody[0]; iend++);
					qsort(g_pContacts, istart,iend-1);
					// for each body: find all its contacting bodies and for each such body get integral vreq (but ignore separating contacts)
					for(i=istart; i<iend; i=j) {
						for(j=i,vreq.zero(),flags=0; j<iend && g_pContacts[j]->pbody[1]==g_pContacts[i]->pbody[1]; j++)
							if (g_pContacts[j]->flags&contact_solve_for && g_pContacts[j]->vreq.len2()>0) {
								vreq += g_pContacts[j]->vreq; flags |= g_pContacts[j]->flags;
							}
						if (flags) { 
							g_buddybuf[nBuddies].next = g_infos[bidx0(i)].pbuddy;
							g_buddybuf[nBuddies].iBody = bidx1(i);
							g_buddybuf[nBuddies].vreq = vreq; g_buddybuf[nBuddies].flags = flags;
							g_infos[bidx0(i)].pbuddy = g_buddybuf+nBuddies++;
							g_buddybuf[nBuddies].next = g_infos[bidx1(i)].pbuddy;
							g_buddybuf[nBuddies].iBody = bidx0(i);
							g_buddybuf[nBuddies].vreq = -vreq; g_buddybuf[nBuddies].flags = flags;
							g_infos[bidx1(i)].pbuddy = g_buddybuf+nBuddies++;
						}
					}
				}

				// for every 2 contacting bodies of each body check if vreqs are conflicting, register sandwich triplet if so (1 inside, 2 outside)
				for(i=nSandwiches=0; i<nBodies; i++)
					for(pbuddy0=g_infos[i].pbuddy; pbuddy0; pbuddy0=pbuddy0->next)
						for(pbuddy1=pbuddy0->next; pbuddy1; pbuddy1=pbuddy1->next)
							if (pbuddy0->vreq*pbuddy1->vreq<0 && !((pbuddy0->flags^pbuddy1->flags)&contact_angular)) {
								g_sandwichbuf[nSandwiches].iMiddle = i;
								g_sandwichbuf[nSandwiches].iBread[0] = pbuddy0->iBody;
								g_sandwichbuf[nSandwiches].iBread[1] = pbuddy1->iBody;
								g_sandwichbuf[nSandwiches].next = g_infos[i].psandwich;
								g_sandwichbuf[nSandwiches].bProcessed = 0;
								g_infos[i].psandwich = g_sandwichbuf+nSandwiches++;
							}

				// call tracepath for each sandwich with static	as 'bread' (if no static - assign a 'pseudo-static')
				for(i=0;i<nBodies;i++) g_infos[i].Minv = g_pBodies[i]->Minv;
				// find the heaviest body participating as bread in any sandwich
				for(i=j=0,minMinv=1E10f; i<nSandwiches; i++) {
					if (g_infos[g_sandwichbuf[i].iBread[0]].Minv<minMinv) minMinv = g_infos[j=g_sandwichbuf[i].iBread[0]].Minv;
					if (g_infos[g_sandwichbuf[i].iBread[1]].Minv<minMinv) minMinv = g_infos[j=g_sandwichbuf[i].iBread[1]].Minv;
				}
				if (minMinv>0) { // if no static participates in sandwich, assign bodies contacting with statics Minv 0 and repeat the procedure
					for(i=0; i<g_nContacts; i++) if (g_pContacts[i]->pbody[0]->Minv*g_pContacts[i]->pbody[1]->Minv==0)
						g_infos[g_pContacts[i]->pbody[g_pContacts[i]->pbody[0]->Minv==0]->bProcessed].Minv = 0;
					for(i=0,minMinv=1E10f; i<nSandwiches; i++) {
						if (g_infos[g_sandwichbuf[i].iBread[0]].Minv<minMinv) minMinv = g_infos[j=g_sandwichbuf[i].iBread[0]].Minv;
						if (g_infos[g_sandwichbuf[i].iBread[1]].Minv<minMinv) minMinv = g_infos[j=g_sandwichbuf[i].iBread[1]].Minv;
					}
				}
				for(i=g_nFollowers=g_nUnprojLoops=0; i<nSandwiches; i++) {
					if (j==g_sandwichbuf[i].iBread[0] || g_infos[g_sandwichbuf[i].iBread[0]].Minv==0) iop=0;
					else if (j==g_sandwichbuf[i].iBread[1] || g_infos[g_sandwichbuf[i].iBread[1]].Minv==0) iop=1;
					else continue;
					g_infos[g_sandwichbuf[i].iBread[iop]].iLevel = 0;
					g_infos[g_sandwichbuf[i].iMiddle].iLevel = max(1,g_infos[g_sandwichbuf[i].iMiddle].iLevel);
					trace_unproj_route(g_sandwichbuf[i].iMiddle, g_sandwichbuf[i].iBread[iop]);
				}

				// option: since after this moment we are starting to get 'heuristic', maybe we should randomize the order of sandwiches?
				iter = 0;
				do {
					// iteratively select an unprocessed sandwich with the min level body (bread or middle), call trace_unproj_route for it; 
					// when the route reaches initialized branches, they will automatically update all previously traced followers
					do {
						for(i=0,j=-1,iMinLevel=0x10000; i<nSandwiches; i++) if (!g_sandwichbuf[i].bProcessed) {
							iLevel = min(g_infos[g_sandwichbuf[i].iMiddle].iLevel&0xFFFF,
								min(g_infos[g_sandwichbuf[i].iBread[0]].iLevel&0xFFFF,g_infos[g_sandwichbuf[i].iBread[1]].iLevel&0xFFFF));
							iop = iLevel-iMinLevel>>31;	j = i&iop | j&~iop;
							iMinLevel = min(iMinLevel,iLevel);
						}
						if (j<0) break;
						g_sandwichbuf[j].bProcessed = 1;
						i = (g_infos[g_sandwichbuf[j].iBread[0]].iLevel>>31&1 | g_infos[g_sandwichbuf[j].iBread[1]].iLevel>>31&2 |
							g_infos[g_sandwichbuf[j].iMiddle].iLevel>>31&4) ^ 7;
						if (i==7) {	// all participating bodies already have levels
							iop = isneg(g_infos[g_sandwichbuf[j].iBread[1]].iLevel-g_infos[g_sandwichbuf[j].iBread[0]].iLevel);	// min level bread
							if (g_infos[g_sandwichbuf[j].iMiddle].iLevel >= g_infos[g_sandwichbuf[j].iBread[iop]].iLevel)
								add_route_follower(g_sandwichbuf[j].iBread[iop], g_sandwichbuf[j].iMiddle);	// breadmin -> middle -> breadmax(will be updated)
						} else if (i==3) { // both breads have levels
							g_infos[g_sandwichbuf[j].iMiddle].iLevel = 1;
							if (iMinLevel>1) { // bread0 <- middle=1 -> bread1
								add_route_follower(g_sandwichbuf[j].iMiddle, g_sandwichbuf[j].iBread[0]);
								add_route_follower(g_sandwichbuf[j].iMiddle, g_sandwichbuf[j].iBread[1]);
							} else { // breadmin -> middle=1 -> breadmax
								iop = isneg(g_infos[g_sandwichbuf[j].iBread[1]].iLevel-g_infos[g_sandwichbuf[j].iBread[0]].iLevel);	// min level bread
								add_route_follower(g_sandwichbuf[j].iBread[iop], g_sandwichbuf[j].iMiddle);
							}
						} else if (i==4) { // only the middle has level
							g_infos[g_sandwichbuf[j].iBread[0]].iLevel = g_infos[g_sandwichbuf[j].iBread[1]].iLevel = 
								g_infos[g_sandwichbuf[j].iMiddle].iLevel+1;
							add_route_follower(g_sandwichbuf[j].iMiddle, g_sandwichbuf[j].iBread[0]);
							add_route_follower(g_sandwichbuf[j].iMiddle, g_sandwichbuf[j].iBread[1]);
						} else if (i!=0) { // either one bread or one bread and the middle have levels
							iop = i>>1&1; // the bread that has level
							g_infos[g_sandwichbuf[j].iMiddle].iLevel = min(g_infos[g_sandwichbuf[j].iMiddle].iLevel&0xFFFF, iMinLevel+1);
							if (i&2) // the middle had level
								add_route_follower(g_sandwichbuf[j].iMiddle, g_sandwichbuf[j].iBread[iop^1]);
							else
								add_route_follower(g_sandwichbuf[j].iBread[iop], g_sandwichbuf[j].iMiddle);
						}
					} while(true);
					if (iter>0) break;

					// now we have only fully clear sandwiches in isolated branches; as we progress, however, some unprocessed sandwiches might
					// become partially initialized; ignore them - the previous loop will pick them up during the next iteration
					for(i=0;i<nSandwiches;i++) 
					if ((g_infos[g_sandwichbuf[i].iMiddle].iLevel & g_infos[g_sandwichbuf[i].iBread[0]].iLevel & g_infos[g_sandwichbuf[i].iBread[1]].iLevel)==-1) {
						g_sandwichbuf[i].bProcessed = 1;
						g_infos[g_sandwichbuf[i].iMiddle].iLevel = 0;
						g_infos[g_sandwichbuf[i].iBread[0]].iLevel = g_infos[g_sandwichbuf[i].iBread[1]].iLevel = 1;
						add_route_follower(g_sandwichbuf[i].iMiddle, g_sandwichbuf[i].iBread[0]);
						add_route_follower(g_sandwichbuf[i].iMiddle, g_sandwichbuf[i].iBread[1]);
					}
					++iter;
				} while(true);

				// set level to maxlevel+1 for all bodies that still don't have a level assigned
				// (they either didn't form sandwiches, or belonged to sandwiches that had only middles initialized, and didn't belong to routes
				for(i=iMaxLevel=0; i<nBodies; i++) iMaxLevel = max(iMaxLevel,g_infos[i].iLevel);
				for(i=0,iMaxLevel++; i<nBodies; i++) {
					g_infos[i].iLevel = iMaxLevel&g_infos[i].iLevel>>31 | max(g_infos[i].iLevel,0);
					if (g_pBodies[i]->Minv==0)
						g_infos[i].iLevel = 0; // force level 0 to statics (some can accidentally get non-0 level if they don't participate in sandwiches)
				}

	#ifdef _DEBUG
	for(i=j=0;i<nSandwiches;i++) 
	if (g_infos[g_sandwichbuf[i].iMiddle].iLevel>=max(g_infos[g_sandwichbuf[i].iBread[0]].iLevel,g_infos[g_sandwichbuf[i].iBread[1]].iLevel))
		j++;
	#endif

				// sort body list according to level
				qsort(g_pBodies,0,nBodies-1);
				for(i=0;i<nBodies;i++) {
					g_infos[g_pBodies[i]->bProcessed].idx = i;
					g_infos[g_pBodies[i]->bProcessed].v_unproj.zero(); g_infos[g_pBodies[i]->bProcessed].w_unproj.zero();  
				}
				// sort contact list so that each contact gets assigned to the body with the higher level (among the two participating in the contact)
				qsort((entity_contact_unproj**)g_pContacts,0,g_nContacts-1);

				for(istart=0; istart<g_nContacts; istart=j) {
					// among contacts assigned to a body, select only non-separating with non-zero vreq and use cg to enforce vreq (along its direction only)
					// don't enforce contacts with 0 vreq, since if we violate them, they'll be reported to have vreq during the next step
					body0 = g_pContacts[istart]->pbody[isneg(g_infos[bidx0(istart)].iLevel-g_infos[bidx1(istart)].iLevel)];
					iSortedContacts[0] = iSortedContacts[1] = 0;
					for(j=istart; j<g_nContacts && g_pContacts[j]->pbody[iop = isneg(g_infos[bidx0(j)].iLevel-g_infos[bidx1(j)].iLevel)]==body0; j++)
					if (g_pContacts[j]->flags & contact_solve_for && g_pContacts[j]->vreq.len2()>sqr(0.01f)) {
						g_pContacts[j]->flags = g_pContacts[j]->flags&~contact_bidx | iop<<contact_bidx_log2;
						g_pContacts[j]->vrel = g_pContacts[j]->vreq*(iop*2-1);
						// from now on use dP to store normalized unprojection direction
						if (fabsf(sqr(g_pContacts[j]->vreq*g_pContacts[j]->n)-g_pContacts[j]->vreq.len2()) < g_pContacts[j]->vreq.len2()*0.01f)
							g_pContacts[j]->dP = g_pContacts[j]->n;
						else 
							g_pContacts[j]->dP = g_pContacts[j]->vreq.normalized();
						body1 = g_pContacts[j]->pbody[iop^1];
						if (!(g_pContacts[j]->flags & contact_angular)) {
							r0 = g_pContacts[j]->pt[iop]-body0->pos; r1 = g_pContacts[j]->pt[iop^1]-body1->pos;
							g_pContacts[j]->Kinv(0,0) = body0->Minv+g_pContacts[j]->dP*(body0->Iinv*(r0^g_pContacts[j]->dP)^r0);
							if (g_pContacts[j]->Kinv(0,0) < body0->Minv*0.1f) { // contact has near-degenerate Kinv, skip it
								g_pContacts[j]->flags &= ~contact_solve_for; continue;
							}
							g_pContacts[j]->vrel += body0->v+g_infos[body0->bProcessed].v_unproj+(body0->w+g_infos[body0->bProcessed].w_unproj^r0);
							g_pContacts[j]->vrel -= body1->v+g_infos[body1->bProcessed].v_unproj+(body1->w+g_infos[body1->bProcessed].w_unproj^r1);
							iSortedContacts[1]++;
						}	else {
							g_pContacts[j]->Kinv(0,0) = g_pContacts[j]->dP*(body0->Iinv*g_pContacts[j]->dP);
							g_pContacts[j]->vrel += body0->w+g_infos[body0->bProcessed].w_unproj - body1->w-g_infos[body1->bProcessed].w_unproj;
							iSortedContacts[0]++;
						}
					}	else
						g_pContacts[j]->flags &= ~contact_solve_for;
					nContacts=iSortedContacts[0]+iSortedContacts[1]; nAngContacts=iSortedContacts[1]=iSortedContacts[0]; iSortedContacts[0]=0;
					if (body0->Minv==0)
						continue;

					for(j=istart; j<g_nContacts && g_pContacts[j]->pbody[iop = isneg(g_infos[bidx0(j)].iLevel-g_infos[bidx1(j)].iLevel)]==body0; j++)
					if (g_pContacts[j]->flags & contact_solve_for) {
						g_pContacts[j]->Kinv(0,0) = 1.0f/g_pContacts[j]->Kinv(0,0);
						g_pContacts[j]->r0.x = g_pContacts[j]->r.x = -(g_pContacts[j]->vrel*g_pContacts[j]->dP);
						g_pContacts[j]->dPn = g_pContacts[j]->r.x*g_pContacts[j]->Kinv(0,0);
						g_pContacts[j]->P.x = 0;
						pContacts[iSortedContacts[g_pContacts[j]->flags>>contact_angular_log2&1^1]++] = g_pContacts[j];
					}
					r2 = ComputeRc(body0,pContacts,nAngContacts,nContacts);

					// use MINRES to enforce vreq
					iter = nContacts;
					if (iter=nContacts) {
						do {
							body0->Fcollision.zero(); body0->Tcollision.zero();
							for(i=0;i<nAngContacts;i++) // angular contacts
								body0->Tcollision += pContacts[i]->dP*pContacts[i]->dPn; 
							for(;i<nContacts;i++) { // positional contacts
								r0 = pContacts[i]->pt[pContacts[i]->flags>>contact_bidx_log2&1]-body0->pos;
								body0->Fcollision += pContacts[i]->dP*pContacts[i]->dPn; body0->Tcollision += r0^pContacts[i]->dP*pContacts[i]->dPn;
							}
							for(i=0;i<nAngContacts;i++)	// angular contacts
								pContacts[i]->vrel = body0->Iinv*body0->Tcollision;
							for(;i<nContacts;i++) { // positional contacts
								r0 = pContacts[i]->pt[pContacts[i]->flags>>contact_bidx_log2&1]-body0->pos;
								pContacts[i]->vrel = body0->Fcollision*body0->Minv + (body0->Iinv*body0->Tcollision^r0);
							}
							for(i=0,pAp=0;i<nContacts;i++) { 
								pContacts[i]->vrel.x = pContacts[i]->dP*pContacts[i]->vrel;
								pAp += sqr(pContacts[i]->vrel.x)*pContacts[i]->Kinv(0,0);
							}
							a = min(20.0,r2/max((real)1E-10,pAp));
							for(i=0;i<nContacts;i++) {
								pContacts[i]->r.x -= pContacts[i]->vrel.x*a;
								pContacts[i]->P.x += pContacts[i]->dPn*a;
							}
							r2new = ComputeRc(body0,pContacts,nAngContacts,nContacts);
							b = r2new/r2; r2=r2new;
							for(i=0;i<nContacts;i++)
								(pContacts[i]->dPn*=b) += pContacts[i]->r.x*pContacts[i]->Kinv(0,0);
						} while(--iter && r2>sqr(pss->minSeparationSpeed));

						// store velocities in v_unproj,w_unproj
						body0->Fcollision.zero(); body0->Tcollision.zero();
						for(i=0;i<nAngContacts;i++) // angular contacts
							body0->Tcollision += pContacts[i]->dP*pContacts[i]->P.x; 
						for(;i<nContacts;i++) { // positional contacts
							r0 = pContacts[i]->pt[pContacts[i]->flags>>contact_bidx_log2&1]-body0->pos;
							body0->Fcollision += pContacts[i]->dP*pContacts[i]->P.x; body0->Tcollision += r0^pContacts[i]->dP*pContacts[i]->P.x;
						}
						g_infos[body0->bProcessed].w_unproj = body0->Iinv*body0->Tcollision;
						g_infos[body0->bProcessed].v_unproj = body0->Fcollision*body0->Minv;

	#ifdef _DEBUG
	for(i=0,r2=r2new=0;i<nAngContacts;i++) {
		r2 += sqr(pContacts[i]->r0.x);
		r2new += sqr(g_infos[body0->bProcessed].w_unproj*pContacts[i]->dP-pContacts[i]->r0.x);
	} for(;i<nContacts;i++) {
		iop = pContacts[i]->flags>>contact_bidx_log2;
		r2 += sqr(pContacts[i]->r0.x);
		r2new += sqr((g_infos[body0->bProcessed].v_unproj+(g_infos[body0->bProcessed].w_unproj^pContacts[i]->pt[iop]-body0->pos))*
			pContacts[i]->dP-pContacts[i]->r0.x);
	}
	iop = 0;
	iop = 1;
	#endif
						float r2max,v2max,v2unproj;
						vmax = 2.0f;
						if (nAngContacts==nContacts)
							r2max = 1;
						else for(i=nAngContacts,r2max=0;i<nContacts;i++) {
							r2max = max(r2max,(pContacts[i]->pt[pContacts[i]->flags>>contact_bidx_log2]-body0->pos).len2());
							vmax = 1.0f;
						}
						for(i=0,v2max=0;i<nAngContacts;i++)
							v2max = max(v2max,sqr(pContacts[i]->r0.x)*r2max);
						for(;i<nContacts;i++)
							v2max = max(v2max,sqr(pContacts[i]->r0.x));
						v2max = min(v2max,sqr(pss->maxvUnproj));
						v2unproj = max(g_infos[body0->bProcessed].v_unproj.len2(), g_infos[body0->bProcessed].w_unproj.len2()*r2max);
						if (v2unproj > v2max*vmax) {
							v2unproj = sqrt_tpl(v2max/v2unproj);
							g_infos[body0->bProcessed].v_unproj *= v2unproj;
							g_infos[body0->bProcessed].w_unproj *= v2unproj;
						}
					}
				}

				// update bodies' positions with v_unproj*dt,w_unproj*dt
				for(i=0; i<nBodies; i++) {
					j = g_pBodies[i]->bProcessed;
					g_pBodies[i]->bProcessed = 0;
					vectorf L = g_pBodies[i]->q*(g_pBodies[i]->Ibody*(g_infos[j].w_unproj*g_pBodies[i]->q));
					g_pBodies[i]->Eunproj = (g_infos[j].v_unproj.len2()+(g_infos[j].w_unproj*L)*g_pBodies[i]->Minv)*0.5f;
					if (g_pBodies[i]->Eunproj>0) {
						g_pBodies[i]->pos += g_infos[j].v_unproj*time_interval;
						if (g_infos[j].w_unproj.len2()*sqr(time_interval)<sqr(0.003f))
							g_pBodies[i]->q += quaternionf(0,g_infos[j].w_unproj*0.5f)*g_pBodies[i]->q*time_interval;
						else {
							float wlen = g_infos[j].w_unproj.len();
							//q = quaternionf(wlen*dt,w/wlen)*q;
							g_pBodies[i]->q = GetRotationAA(wlen*time_interval,g_infos[j].w_unproj/wlen)*g_pBodies[i]->q;
						}
						g_pBodies[i]->q.Normalize();
						// don't unpdate Iinv and w, no1 will appreciate it after this point
					}
					g_pBodies[i]->Fcollision = g_infos[j].Fcollision;
					g_pBodies[i]->Tcollision = g_infos[j].Tcollision;
				}
			}//"LCPCG-unproj"
		}
	}


	for(i=0;i<nBodies;i++) {
		g_pBodies[i]->Fcollision = (g_pBodies[i]->P-g_pBodies[i]->Fcollision)*rtime_interval; 
		g_pBodies[i]->Tcollision = (g_pBodies[i]->L-g_pBodies[i]->Tcollision)*rtime_interval;
	}
}
