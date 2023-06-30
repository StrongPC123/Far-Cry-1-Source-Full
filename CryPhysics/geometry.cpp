#include "stdafx.h"

#include "utils.h"
#include "primitives.h"
#include "overlapchecks.h"
#include "intersectionchecks.h"
#include "unprojectionchecks.h"
#include "bvtree.h"
#include "geometry.h"
#include "singleboxtree.h"
#include "spheregeom.h"


indexed_triangle g_IdxTriBuf[256];
int g_IdxTriBufPos;
cylinder g_CylBuf[2];
int g_CylBufPos;
sphere g_SphBuf[2];
int g_SphBufPos;
box g_BoxBuf[2];
int g_BoxBufPos;

BBox g_BBoxBuf[128];
int g_BBoxBufPos;

surface_desc g_SurfaceDescBuf[64];
int g_SurfaceDescBufPos;

edge_desc g_EdgeDescBuf[64];
int g_EdgeDescBufPos;

int g_iFeatureBuf[64];
int g_iFeatureBufPos;

short g_IdBuf[256];
int g_IdBufPos;

int g_UsedNodesMap[8192];
int g_UsedNodesMapPos;
int g_UsedNodesIdx[64];
int g_UsedNodesIdxPos;

geom_contact g_Contacts[64];
int g_nTotContacts;
geom_contact_area g_AreaBuf[32];
int g_nAreas;
vectorf g_AreaPtBuf[256];
int g_AreaPrimBuf0[256],g_AreaFeatureBuf0[256],g_AreaPrimBuf1[256],g_AreaFeatureBuf1[256];
int g_nAreaPt;
extern vectorf g_BrdPtBuf[2048]; // from trimesh.cpp
extern int g_BrdPtBufPos;				 // 

struct InitGeometryGlobals {
	InitGeometryGlobals() {
		memset(g_UsedNodesMap, 0, sizeof(g_UsedNodesMap));
		g_EdgeDescBufPos = 0;	g_IdBufPos = 0;	g_IdxTriBufPos = 0;
		g_SurfaceDescBufPos = 0; g_UsedNodesMapPos = 0;	g_UsedNodesIdxPos = 0;
		g_iFeatureBufPos = 0;
		g_nAreas = 0;	g_nAreaPt = 0; g_nTotContacts = 0;
	}
};
static InitGeometryGlobals now;

#include "raybv.h"
#include "raygeom.h"

int IntersectBVs(geometry_under_test *pGTest, BV *pBV1,BV *pBV2) 
{
	intptr_t mask;
	int iNode[2],*pUsedNodesMap[2],res=0,nullmap=0,bNodeUsed[2];
	mask = iszero_mask(pGTest[0].pUsedNodesMap); // mask = -(pUsedNodeMap==0)
	pUsedNodesMap[0] = (int*)((intptr_t)pGTest[0].pUsedNodesMap&~mask | (intptr_t)&nullmap&mask);
	iNode[0] = pBV1->iNode&~mask;
	mask = iszero_mask(pGTest[1].pUsedNodesMap); // mask = -(pUsedNodeMap==0)
	pUsedNodesMap[1] = (int*)((intptr_t)pGTest[1].pUsedNodesMap&~mask | (intptr_t)&nullmap&mask);
	iNode[1] = pBV2->iNode&~mask;
	bNodeUsed[0] = pUsedNodesMap[0][iNode[0]>>5]>>(iNode[0]&31) & 1;
	bNodeUsed[1] = pUsedNodesMap[1][iNode[1]>>5]>>(iNode[1]&31) & 1;

	if (bNodeUsed[0] & bNodeUsed[1])
		return 0; // skip check only if both nodes are marked used

	//if (pUsedNodesMap[0][iNode[0]>>5] & 1<<(iNode[0]&31) | pUsedNodesMap[1][iNode[1]>>5] & 1<<(iNode[1]&31))
	//	return 0; // skip check if at least one BV node is marked checked
	
	if (!g_Overlapper.Check(pBV1->type,pBV2->type, *pBV1,*pBV2))
		return 0;

	float split1,split2;
	split1 = pGTest[0].pBVtree->SplitPriority(pBV1);
	split2 = pGTest[1].pBVtree->SplitPriority(pBV2);

	BV *pBV_split1,*pBV_split2;

	if (split1>split2 && split1>0) { // split the first BV
		pGTest[0].pBVtree->GetNodeChildrenBVs(pBV1, pBV_split1,pBV_split2);
		res = IntersectBVs(pGTest, pBV_split1,pBV2);
		if (pGTest[0].bStopIntersection + pGTest[1].bStopIntersection > 0)
			return res;
		res += IntersectBVs(pGTest, pBV_split2,pBV2);
		pGTest[0].pBVtree->ReleaseLastBVs();
	} else 
	if (split2>0) { // split the second BV
		pGTest[1].pBVtree->GetNodeChildrenBVs(pGTest[1].R_rel,pGTest[1].offset_rel,pGTest[1].scale_rel, pBV2, pBV_split1,pBV_split2);
		res = IntersectBVs(pGTest, pBV1,pBV_split1);
		if (pGTest[0].bStopIntersection + pGTest[1].bStopIntersection > 0)
			return res;
		res += IntersectBVs(pGTest, pBV1,pBV_split2);
		pGTest[1].pBVtree->ReleaseLastBVs();
	} else {
		int nPrims1,nPrims2,iClient,i,j;
		primitive *ptr[2];
		prim_inters inters;
		static vectorf ptborder[16];
		inters.minPtDist2 = sqr(min(pGTest[0].pGeometry->m_minVtxDist, pGTest[1].pGeometry->m_minVtxDist));
		iClient = -(pGTest[1].pGeometry->m_iCollPriority-pGTest[0].pGeometry->m_iCollPriority>>31);
		inters.pt[1].Set(0,0,0);
		inters.ptborder = ptborder;
		inters.nborderpt = inters.nBestPtVal = 0;
		inters.nbordersz = sizeof(ptborder)/sizeof(ptborder[0]);

		nPrims1 = pGTest[0].pBVtree->GetNodeContents(pBV1->iNode, pBV2,bNodeUsed[1],1, pGTest+0,pGTest+1);
		nPrims2 = pGTest[1].pBVtree->GetNodeContents(pBV2->iNode, pBV1,bNodeUsed[0],0, pGTest+1,pGTest+0);

		for(i=0,ptr[0]=pGTest[0].primbuf; i<nPrims1; i++,ptr[0]=(primitive*)((char*)ptr[0]+pGTest[0].szprim)) 
			for(j=0,ptr[1]=pGTest[1].primbuf; j<nPrims2; j++,ptr[1]=(primitive*)((char*)ptr[1]+pGTest[1].szprim))
				if (g_Intersector.Check(pGTest[iClient].typeprim,pGTest[iClient^1].typeprim, ptr[iClient],ptr[iClient^1], &inters)) 
		{
			inters.id[0] = pGTest[iClient].idbuf[i&-(iClient^1)|j&-iClient]; 
			inters.id[1] = pGTest[iClient^1].idbuf[i&-iClient|j&-(iClient^1)];
			inters.iNode[0] = pBV1->iNode; 
			inters.iNode[1] = pBV2->iNode;
			pGTest[0].bCurNodeUsed = pGTest[1].bCurNodeUsed = 0;
			res += pGTest[iClient].pGeometry->RegisterIntersection(ptr[iClient],ptr[iClient^1], pGTest+iClient,pGTest+(iClient^1), &inters);
			if (pGTest[0].bStopIntersection+pGTest[1].bStopIntersection + pGTest[0].bCurNodeUsed+pGTest[1].bCurNodeUsed> 0)
				return res;
			inters.nborderpt = inters.nBestPtVal = 0;
			inters.nbordersz = sizeof(ptborder)/sizeof(ptborder[0]);
		}
	}

	return res;
}

int SweepTestBVs(geometry_under_test *pGTest, BV *pBV1,BV *pBV2) 
{
	if (!g_Overlapper.Check(pBV1->type,pBV2->type, *pBV1,*pBV2))
		return 0;
	
	int res = 0;
	float split1,split2;
	split1 = pGTest[0].pBVtree->SplitPriority(pBV1);
	split2 = pGTest[1].pBVtree->SplitPriority(pBV2);

	BV *pBV_split1,*pBV_split2;

	if (split1>split2 && split1>0) { // split the first BV
		pGTest[0].pBVtree->GetNodeChildrenBVs(pBV1, pGTest[0].sweepdir_loc,pGTest[0].sweepstep_loc, pBV_split1,pBV_split2);
		res = SweepTestBVs(pGTest, pBV_split1,pBV2) + SweepTestBVs(pGTest, pBV_split2,pBV2);
		pGTest[0].pBVtree->ReleaseLastSweptBVs();
	} else 
	if (split2>0) { // split the second BV
		pGTest[1].pBVtree->GetNodeChildrenBVs(pGTest[1].R_rel,pGTest[1].offset_rel,pGTest[1].scale_rel, pBV2, pBV_split1,pBV_split2);
		res = SweepTestBVs(pGTest, pBV1,pBV_split1) + SweepTestBVs(pGTest, pBV1,pBV_split2);
		pGTest[1].pBVtree->ReleaseLastBVs();
	} else {
		int nPrims1,nPrims2,i,j;
		primitive *ptr[2];
		contact contact_cur;
		geom_contact *pcontact = pGTest[0].contacts;
		unprojection_mode unproj;
		vectorf startoffs = pGTest[0].offset;
		pGTest[0].offset += pGTest[0].sweepdir*pGTest[0].sweepstep;
		unproj.imode = 0;
		unproj.dir = -pGTest[0].sweepdir;
		unproj.tmax = pGTest[0].sweepstep;
		unproj.minPtDist = min(pGTest[0].pGeometry->m_minVtxDist, pGTest[1].pGeometry->m_minVtxDist);

		nPrims1 = pGTest[0].pBVtree->GetNodeContents(pBV1->iNode, pBV2,0,-1, pGTest+0,pGTest+1);
		nPrims2 = pGTest[1].pBVtree->GetNodeContents(pBV2->iNode, pBV1,0,0, pGTest+1,pGTest+0);

		for(i=0,ptr[0]=pGTest[0].primbuf; i<nPrims1; i++,ptr[0]=(primitive*)((char*)ptr[0]+pGTest[0].szprim)) 
			for(j=0,ptr[1]=pGTest[1].primbuf; j<nPrims2; j++,ptr[1]=(primitive*)((char*)ptr[1]+pGTest[1].szprim))
				if (g_Unprojector.Check(&unproj, pGTest[0].typeprim,pGTest[1].typeprim, ptr[0],-1,ptr[1],-1, &contact_cur) && 
						contact_cur.n*pGTest[0].sweepdir>0 && contact_cur.t<=pGTest[0].sweepstep && pGTest[0].sweepstep-contact_cur.t<pcontact->t) 
		{
			pcontact->t = pGTest[0].sweepstep-contact_cur.t;
			pcontact->pt = contact_cur.pt;
			pcontact->n = contact_cur.n.normalized();
			pcontact->iFeature[0] = contact_cur.iFeature[0];
			pcontact->iFeature[1] = contact_cur.iFeature[1];
			pcontact->id[0] = pGTest[0].idbuf[i];
			pcontact->id[1] = pGTest[1].idbuf[j];
			pcontact->iNode[0] = pBV1->iNode; 
			pcontact->iNode[1] = pBV2->iNode;
			res++; *(pGTest[0].pnContacts) = 1;
		}
		pGTest[0].offset = startoffs;
	}

	return res;
}


int CGeometry::Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	geometry_under_test gtest[2];
	geom_world_data *pdata[2] = { pdata1,pdata2 };
	int i,j,jmax,mask,nContacts=0,iStartNode[2];
	intersection_params defip;
	vectorr offsetWorld;
	if (!pparams)
		pparams = &defip;

	// zero all global buffer pointers
	ResetGlobalPrimsBuffers();
	g_Overlapper.Init();
	if (!pparams->bKeepPrevContacts) {
		g_nAreas = 0; g_nAreaPt = 0; g_nTotContacts = 0;
	}
	if (g_nTotContacts>=sizeof(g_Contacts)/sizeof(g_Contacts[0]))
		return 0;
	g_SurfaceDescBufPos = 0;
	g_EdgeDescBufPos = 0;
	g_IdBufPos = 0;
	g_iFeatureBufPos = 0;
	g_UsedNodesMapPos = 0;
	g_UsedNodesIdxPos = 0;
	pcontacts = g_Contacts+g_nTotContacts;
	pparams->pGlobalContacts = g_Contacts;

	if (!pparams->bNoAreaContacts && g_nAreas<sizeof(g_AreaBuf)/sizeof(g_AreaBuf[0]) && g_nAreaPt<sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])) {
		pcontacts[0].parea = g_AreaBuf+g_nAreas;
		pcontacts[0].parea->pt = g_AreaPtBuf+g_nAreaPt;
		pcontacts[0].parea->piPrim[0] = g_AreaPrimBuf0+g_nAreaPt; 
		pcontacts[0].parea->piFeature[0] = g_AreaFeatureBuf0+g_nAreaPt;
		pcontacts[0].parea->piPrim[1] = g_AreaPrimBuf1+g_nAreaPt; 
		pcontacts[0].parea->piFeature[1] = g_AreaFeatureBuf1+g_nAreaPt;
		pcontacts[0].parea->npt = 0; pcontacts[0].parea->nmaxpt = sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])-g_nAreaPt;
	/*for(i=0;i<pcontacts[0].parea->nmaxpt;i++)	{
	pcontacts[0].parea->piPrim[0][i]=pcontacts[0].parea->piFeature[0][i]=-2;
	pcontacts[0].parea->pt[i](777.0f,777.0f,777.0f); }*/
		pcontacts[0].parea->minedge = min(gtest[0].minAreaEdge, gtest[1].minAreaEdge);
	} else
		pcontacts[0].parea = 0;
	if (pparams->bSweepTest && pdata1->v.len2()==0) {
		pparams->bSweepTest = 0; pdata1->v.Set(0,0,1); pparams->vrel_min = 0; 
	}

	for(i=0;i<2;i++) {
		if (pdata[i]) {
			gtest[i].offset = pdata[i]->offset;
			gtest[i].R = pdata[i]->R;
			gtest[i].scale = pdata[i]->scale;
			gtest[i].rscale = 1.0f/pdata[i]->scale;
			gtest[i].v = pdata[i]->v;
			gtest[i].w = pdata[i]->w;
			gtest[i].centerOfMass = pdata[i]->centerOfMass;
			iStartNode[i] = pdata[i]->iStartNode;
		} else {
			gtest[i].offset.Set(0,0,0);
			gtest[i].R.SetIdentity();
			gtest[i].scale = gtest[i].rscale = 1.0f;
			gtest[i].v.Set(0,0,0);
			gtest[i].w.Set(0,0,0);
			iStartNode[i] = 0;
		}
		gtest[i].centerOfRotation = pparams->centerOfRotation;
		gtest[i].axisContactNormal = pparams->axisContactNormal*(1-(i<<1));
		gtest[i].contacts = pcontacts;
		gtest[i].pnContacts = &nContacts;
		gtest[i].nMaxContacts = sizeof(g_Contacts)/sizeof(g_Contacts[0])-g_nTotContacts;
		gtest[i].bStopIntersection = 0;
		gtest[i].sweepstep = 0;
		gtest[i].ptOutsidePivot = pparams->ptOutsidePivot[i];
		gtest[i].pParams = pparams;
	}
	for(i=0;i<2;i++) {
		gtest[i].offset_rel = ((gtest[i].offset-gtest[i^1].offset)*gtest[i^1].R)*gtest[i^1].rscale;
		(gtest[i].R_rel = gtest[i^1].R.T()) *= gtest[i].R;
		gtest[i].scale_rel = gtest[i].scale*gtest[i^1].rscale;
		gtest[i].rscale_rel = gtest[i^1].scale*gtest[i].rscale;
	}
	pparams->bBothConvex = m_bIsConvex & ((CGeometry*)pCollider)->m_bIsConvex;
	
	gtest[0].offset -= (offsetWorld = gtest[1].offset);
	gtest[0].centerOfMass -= offsetWorld; gtest[0].centerOfRotation -= offsetWorld;	gtest[0].ptOutsidePivot -= offsetWorld;
	gtest[1].centerOfMass -= offsetWorld;	gtest[1].centerOfRotation -= offsetWorld;	gtest[1].ptOutsidePivot -= offsetWorld;
	gtest[1].offset.zero(); // use relative offset in calculations to improve accuracy

	BV *pBV1,*pBV2;

	if (!pparams->bSweepTest) {
		if (!PrepareForIntersectionTest(gtest+0, (CGeometry*)pCollider,gtest+1, pparams->bKeepPrevContacts) || 
			!((CGeometry*)pCollider)->PrepareForIntersectionTest(gtest+1, this,gtest+0, pparams->bKeepPrevContacts))
			return 0;

		if (iStartNode[0]+iStartNode[1]) {
			gtest[0].pBVtree->GetNodeBV(pBV1, iStartNode[0]);
			gtest[1].pBVtree->GetNodeBV(gtest[1].R_rel,gtest[1].offset_rel,gtest[1].scale_rel, pBV2, iStartNode[1]);
			IntersectBVs(gtest, pBV1,pBV2);
		}
		if (nContacts==0) {
			gtest[0].pBVtree->GetNodeBV(pBV1);
			gtest[1].pBVtree->GetNodeBV(gtest[1].R_rel,gtest[1].offset_rel,gtest[1].scale_rel, pBV2);
			IntersectBVs(gtest, pBV1,pBV2);
		}

		int iClient = -(gtest[1].pGeometry->m_iCollPriority-gtest[0].pGeometry->m_iCollPriority>>31);
		if (iClient) for(i=0;i<nContacts;i++) {
			if (pcontacts[i].parea && pcontacts[i].parea->npt) {
				vectorf ntmp=pcontacts[i].n; pcontacts[i].n=pcontacts[i].parea->n1; pcontacts[i].parea->n1=ntmp;
				int *pprim=pcontacts[i].parea->piPrim[0]; pcontacts[i].parea->piPrim[0]=pcontacts[i].parea->piPrim[1]; pcontacts[i].parea->piPrim[1]=pprim;
				int *pfeat=pcontacts[i].parea->piFeature[0]; pcontacts[i].parea->piFeature[0]=pcontacts[i].parea->piFeature[1]; 
				pcontacts[i].parea->piFeature[1]=pfeat;
				if (pcontacts[i].iUnprojMode) {
					pcontacts[i].n = pcontacts[i].n.rotated(pcontacts[i].dir,-pcontacts[i].t);
					pcontacts[i].parea->n1 = pcontacts[i].parea->n1.rotated(pcontacts[i].dir,-pcontacts[i].t);
				}
				if (pcontacts[i].iUnprojMode)	for(j=0;j<pcontacts[i].parea->npt;j++)
					pcontacts[i].parea->pt[j] = pcontacts[i].parea->pt[j].rotated(gtest[0].centerOfRotation,pcontacts[i].dir,-pcontacts[i].t);
				else for(j=0;j<pcontacts[i].parea->npt;j++)
					pcontacts[i].parea->pt[j] -= pcontacts[i].dir*pcontacts[i].t;
			} else {
				if (pcontacts[i].iUnprojMode)
					pcontacts[i].n = pcontacts[i].n.rotated(pcontacts[i].dir,-pcontacts[i].t);
				pcontacts[i].n.Flip();
			}
			if (pcontacts[i].iUnprojMode)
				pcontacts[i].pt = pcontacts[i].pt.rotated(gtest[0].centerOfRotation,pcontacts[i].dir,-pcontacts[i].t);
			else
				pcontacts[i].pt -= pcontacts[i].dir*pcontacts[i].t;
			pcontacts[i].dir.Flip();
			short id=pcontacts[i].id[0]; pcontacts[i].id[0]=pcontacts[i].id[1]; pcontacts[i].id[1]=id;
			int iprim=pcontacts[i].iPrim[0]; pcontacts[i].iPrim[0]=pcontacts[i].iPrim[1]; pcontacts[i].iPrim[1]=iprim;
			int ifeature=pcontacts[i].iFeature[0]; pcontacts[i].iFeature[0]=pcontacts[i].iFeature[1]; pcontacts[i].iFeature[1]=ifeature;
		}

		// sort contacts in descending t order
		geom_contact tmpcontact;
		for(i=0;i<nContacts-1;i++) {
			for(jmax=i,j=i+1; j<nContacts; j++) {
				mask = -isneg(pcontacts[jmax].t-pcontacts[j].t);
				jmax = jmax&~mask | j&mask;
			}
			if (jmax!=i) {
				if (pcontacts[i].ptborder==&pcontacts[i].center) pcontacts[i].ptborder = &pcontacts[jmax].center;
				if (pcontacts[i].ptborder==&pcontacts[i].pt) pcontacts[i].ptborder = &pcontacts[jmax].pt;
				if (pcontacts[jmax].ptborder==&pcontacts[jmax].center) pcontacts[jmax].ptborder = &pcontacts[i].center;
				if (pcontacts[jmax].ptborder==&pcontacts[jmax].pt) pcontacts[jmax].ptborder = &pcontacts[i].pt;
				tmpcontact=pcontacts[i]; pcontacts[i]=pcontacts[jmax]; pcontacts[jmax]=tmpcontact;
			}
		}
	}	else {
		gtest[0].sweepstep = pcontacts[0].vel = gtest[0].v.len();
		gtest[0].sweepdir = gtest[0].v/gtest[0].sweepstep;
		gtest[0].sweepstep *= pparams->time_interval;
		gtest[0].sweepdir_loc = gtest[0].sweepdir*gtest[0].R;
		gtest[0].sweepstep_loc = gtest[0].sweepstep*gtest[0].rscale;

		if (!PrepareForIntersectionTest(gtest+0, (CGeometry*)pCollider,gtest+1, pparams->bKeepPrevContacts) || 
			!((CGeometry*)pCollider)->PrepareForIntersectionTest(gtest+1, this,gtest+0, pparams->bKeepPrevContacts))
			return 0;

		pcontacts[0].ptborder = &g_Contacts[0].pt;
		pcontacts[0].nborderpt = 1;
		pcontacts[0].parea = 0;
		pcontacts[0].dir = -gtest[0].sweepdir;
		pcontacts[0].t = gtest[0].sweepstep;

		gtest[0].pBVtree->GetNodeBV(pBV1, gtest[0].sweepdir_loc,gtest[0].sweepstep_loc);
		gtest[1].pBVtree->GetNodeBV(gtest[1].R_rel,gtest[1].offset_rel,gtest[1].scale_rel, pBV2);
		SweepTestBVs(gtest, pBV1,pBV2);
	}

	for(i=0;i<nContacts;i++) {
		pcontacts[i].pt += offsetWorld;
		pcontacts[i].center += offsetWorld;
		if (pcontacts[i].ptborder!=&pcontacts[i].pt && pcontacts[i].ptborder!=&pcontacts[i].center)
			for(j=0;j<pcontacts[i].nborderpt;j++)
				pcontacts[i].ptborder[j] += offsetWorld;
		if (pcontacts[i].parea) for(j=0;j<pcontacts[i].parea->npt;j++)
			pcontacts[i].parea->pt[j] += offsetWorld;
	}

	CleanupAfterIntersectionTest(gtest+0);
	((CGeometry*)pCollider)->CleanupAfterIntersectionTest(gtest+1);

	g_nTotContacts += nContacts;
	return nContacts;
}


int CGeometry::RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2, 
																		prim_inters *pinters)
{
	unprojection_mode unproj;
	contact contact_cur;
	geom_contact *pres = pGTest1->contacts + *pGTest1->pnContacts;
	primitive *pprims[2] = { pprim1, pprim2 };
	pres->ptborder = &pres->pt;
	pres->nborderpt = 1;

	if (pGTest1->pParams->iUnprojectionMode==0) {
		// for linear unprojection, find point with maximum relative normal velocity
		unproj.imode = 0;
		unproj.dir = pGTest2->v+(pGTest2->w^pinters->pt[1]-pGTest2->centerOfMass) - 
			pGTest1->v-(pGTest1->w^pinters->pt[1]-pGTest1->centerOfMass);
		unproj.vel = unproj.dir.len();
		if (unproj.vel < pGTest1->pParams->vrel_min)
			goto use_normal;
		unproj.dir /= unproj.vel;
	} else if (pGTest1->pParams->iUnprojectionMode==1) {
		unproj.imode = 1;
		unproj.dir = pGTest2->w-pGTest1->w;
		unproj.center = pGTest1->centerOfRotation.len2()>pGTest2->centerOfRotation.len2() ? pGTest1->centerOfRotation : pGTest2->centerOfRotation;
		unproj.vel = unproj.dir.len();
		unproj.dir /= unproj.vel;
	}
	unproj.tmax = pGTest1->pParams->time_interval*unproj.vel;

	if (!g_Unprojector.Check(&unproj, pGTest1->typeprim,pGTest2->typeprim, pprims[0],-1,pprims[1],-1, &contact_cur, pres->parea))
		return 0;

	if (contact_cur.t > pGTest1->pParams->time_interval*unproj.vel) {
		use_normal:
		unproj.imode = 0;
		unproj.dir = pinters->n.normalize();
		unproj.vel = 0;	

		if (!g_Unprojector.Check(&unproj, pGTest1->typeprim,pGTest2->typeprim, pprims[0],-1,pprims[1],-1, &contact_cur, pres->parea))
			return 0;
	}

	pres->t = contact_cur.t;
	pres->pt = contact_cur.pt;
	pres->n = contact_cur.n;
	pres->dir = unproj.dir;
	pres->vel = unproj.vel;
	pres->iUnprojMode = unproj.imode;
	pres->id[0] = pinters->id[0];
	pres->id[1] = pinters->id[1];
	pres->iNode[0] = pinters->iNode[0];
	pres->iNode[1] = pinters->iNode[1];

	if (pres->parea->npt>0) {
		g_nAreaPt += pres->parea->npt;
		g_nAreas++;
	}	else 
		pres->parea = 0;

	// allocate slots for the next intersection
	(*pGTest1->pnContacts)++;
	pres = pGTest1->contacts + *pGTest1->pnContacts;
	if (g_nAreas<sizeof(g_AreaBuf)/sizeof(g_AreaBuf[0]) && g_nAreaPt<sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])) {
		pres->parea = g_AreaBuf+g_nAreas;
		pres->parea->pt = g_AreaPtBuf+g_nAreaPt;
		pres->parea->piPrim[0] = g_AreaPrimBuf0+g_nAreaPt; pres->parea->piFeature[0] = g_AreaFeatureBuf0+g_nAreaPt;
		pres->parea->piPrim[1] = g_AreaPrimBuf1+g_nAreaPt; pres->parea->piFeature[1] = g_AreaFeatureBuf1+g_nAreaPt;
		pres->parea->npt = 0; pres->parea->nmaxpt = sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])-g_nAreaPt;
		pres->parea->minedge = min(pGTest1->minAreaEdge, pGTest2->minAreaEdge);
	} else
		pres->parea = 0;

	if (*pGTest1->pnContacts >= pGTest1->nMaxContacts)
		pGTest1->bStopIntersection = 1;

	return 0;
}

struct radius_check_data {
	CGeometry *pGeom;
	CBVTree *pBVtree;
	geom_world_data *pgwd;
	sphere sph;
	float rmin,rmax;
	float zscale;
	int **pGrid;
	int nRes;
	int nGrow;
	int iPass;
};

int RadiusCheckBVs(radius_check_data *prcd, BV *pBV)
{
	if (!g_Overlapper.Check(pBV->type,sphere::type, *pBV,&prcd->sph))
		return 0;

	if (prcd->pBVtree->SplitPriority(pBV)>0) {
		BV *pBV_split1,*pBV_split2;
		prcd->pBVtree->GetNodeChildrenBVs(pBV, pBV_split1,pBV_split2);
		int res = RadiusCheckBVs(prcd, pBV_split1)+RadiusCheckBVs(prcd, pBV_split2);
		prcd->pBVtree->ReleaseLastBVs();
		return res;
	} else {
		int iStartPrim,nPrims;
		nPrims = prcd->pBVtree->GetNodeContentsIdx(pBV->iNode, iStartPrim);
		return prcd->pGeom->DrawToOcclusionCubemap(prcd->pgwd, iStartPrim,nPrims, prcd->iPass, 
			prcd->pGrid,prcd->nRes, prcd->rmin,prcd->rmax,prcd->zscale);
	}
}


float CGeometry::BuildOcclusionCubemap(geom_world_data *pgwd, int iMode, int *pGrid0[6],int *pGrid1[6],int nRes, float rmin,float rmax, int nGrow)
{
	radius_check_data rcd;
	float rscale = pgwd->scale==1.0f ? 1.0f:1.0f/pgwd->scale;
	int i,j,irmin;

	ResetGlobalPrimsBuffers();
	rcd.pgwd = pgwd;
	rcd.sph.center = (-pgwd->offset*rscale)*pgwd->R;
	rcd.sph.r = rmax*rscale;
	rcd.rmin = rmin;
	rcd.rmax = rmax;
	rcd.zscale = 65535.0f/rmax;
	rcd.nRes = nRes;
	rcd.nGrow = nGrow;
	rcd.iPass = 0;
	if (iMode==0)
		rcd.pGrid = pGrid0;
	else {
		rcd.pGrid = pGrid1;
		for(i=0;i<6;i++) for(j=nRes*nRes-1;j>=0;j--)
			pGrid1[i][j] = (1u<<31)-1;
	}
	rcd.pGeom = this;
	rcd.pBVtree = GetBVTree();

	BV *pBV;
	rcd.pBVtree->GetNodeBV(pBV);

	if (RadiusCheckBVs(&rcd,pBV)) {// && iMode==0) {
		rcd.iPass = 1;
		ResetGlobalPrimsBuffers();
		RadiusCheckBVs(&rcd,pBV);
	}

	irmin = float2int(rmin*rcd.zscale);
	for(i=0;i<6;i++) for(j=nRes*nRes-1;j>=0;j--) // if the highest bit is set, change cell z to irmin
		rcd.pGrid[i][j] = irmin&rcd.pGrid[i][j]>>31 | rcd.pGrid[i][j]&~(rcd.pGrid[i][j]>>31);
	
	if (iMode) {
		int nCells,nOccludedCells;
		GrowAndCompareCubemaps(pGrid0,pGrid1,nRes, nGrow, nCells,nOccludedCells);
		return nCells>0 ? (float)(nCells-nOccludedCells)/nCells : 0.0f;
	}
	return 0.0f;
}


void DrawBBox(void (*DrawLineFunc)(float*,float*), geom_world_data *gwd, CBVTree *pTree,BBox *pbbox,int maxlevel,int level)
{
	if (level<maxlevel && pTree->SplitPriority(pbbox)>0) {
		BV *pbbox1,*pbbox2;
		pTree->GetNodeChildrenBVs(gwd->R,gwd->offset,gwd->scale, pbbox, pbbox1,pbbox2);
		DrawBBox(DrawLineFunc,gwd, pTree,(BBox*)pbbox1,maxlevel,level+1);
		DrawBBox(DrawLineFunc,gwd, pTree,(BBox*)pbbox2,maxlevel,level+1);
		pTree->ReleaseLastBVs();
		return;
	}

	vectorf pts[8],sz;
	int i,j;

	for(i=0;i<8;i++) {
		for(j=0;j<3;j++) sz[j]=pbbox->abox.size[j]*(((i>>j&1)<<1)-1);
		pts[i] = pbbox->abox.Basis.T()*sz + pbbox->abox.center;
	}
	for(i=0;i<8;i++) for(j=0;j<3;j++) if (i&1<<j)
		DrawLineFunc(pts[i],pts[i^1<<j]);
}



int CPrimitive::Intersect(IGeometry *_pCollider, geom_world_data *pdata1,geom_world_data *pdata2, 
													intersection_params *pparams, geom_contact *&pcontacts)
{
	CGeometry *pCollider = (CGeometry*)_pCollider;
	if (!pCollider->IsAPrimitive())
		return CGeometry::Intersect(pCollider,pdata1,pdata2,pparams,pcontacts);

	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	static geom_world_data defgwd;
	static intersection_params defip;
	pdata1 = (geom_world_data*)((intptr_t)pdata1 | -iszero((intptr_t)pdata1) & (intptr_t)&defgwd);
	pdata2 = (geom_world_data*)((intptr_t)pdata2 | -iszero((intptr_t)pdata2) & (intptr_t)&defgwd);
	pparams = (intersection_params*)((intptr_t)pparams | -iszero((intptr_t)pparams) & (intptr_t)&defip);
	if (!pparams->bKeepPrevContacts)
		g_nAreas = g_nAreaPt = g_nTotContacts = g_BrdPtBufPos = 0;
	if (g_nTotContacts>=sizeof(g_Contacts)/sizeof(g_Contacts[0]))
		return 0;
	pcontacts = g_Contacts+g_nTotContacts;
	pcontacts->ptborder = g_BrdPtBuf+g_BrdPtBufPos;
	pcontacts->nborderpt = 0;
	pparams->pGlobalContacts = g_Contacts;

	BV *pBV1,*pBV2;
	prim_inters inters;
	vectorf sweepdir(zero); float sweepstep=0;
	ResetGlobalPrimsBuffers();
	g_Overlapper.Init();
	if (!pparams->bSweepTest) 
		GetBVTree()->GetNodeBV(pdata1->R,pdata1->offset,pdata1->scale, pBV1);
	else {
		sweepstep = pdata1->v.len();
		sweepdir = pdata1->v/sweepstep;
		sweepstep *= pparams->time_interval;
		GetBVTree()->GetNodeBV(pdata1->R,pdata1->offset,pdata1->scale, pBV1, sweepdir,sweepstep);
	}
	pCollider->GetBVTree()->GetNodeBV(pdata2->R,pdata2->offset,pdata2->scale, pBV2);
	if (!g_Overlapper.Check(pBV1->type,pBV2->type, *pBV1,*pBV2))
		return 0;

	// get primitives in world space
	int i,itype[2],bUnprojected=0;
	primitive *pprim[2];
	pdata1->offset += sweepdir*sweepstep;
	itype[0] = PreparePrimitive(pdata1,pprim[0]);
	itype[1] = pCollider->PreparePrimitive(pdata2,pprim[1]);
	pdata1->offset -= sweepdir*sweepstep;

	if (pCollider->m_iCollPriority==0) { // probably the other geometry is a ray
		if (!g_Intersector.Check(itype[0],itype[1], pprim[0],pprim[1], &inters)) 
			return 0;
		geometry_under_test gtest;
		gtest.contacts = g_Contacts+g_nTotContacts;
		gtest.pnContacts = &g_nTotContacts;
		gtest.pParams = pparams;
		pCollider->RegisterIntersection(pprim[0],pprim[1],&gtest,0,&inters);
		pcontacts->n.Flip();
		return 1;
	}

	unprojection_mode unproj;
	contact contact_best;
	contact_best.t = 0;
	geom_contact_area *parea;
	unproj.minPtDist = min(m_minVtxDist,pCollider->m_minVtxDist);

	if (!pparams->bNoAreaContacts && g_nAreas<sizeof(g_AreaBuf)/sizeof(g_AreaBuf[0]) && g_nAreaPt<sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])) {
		parea = g_AreaBuf+g_nAreas;
		parea->pt = g_AreaPtBuf+g_nAreaPt;
		parea->piPrim[0] = g_AreaPrimBuf0+g_nAreaPt; 
		parea->piFeature[0] = g_AreaFeatureBuf0+g_nAreaPt;
		parea->piPrim[1] = g_AreaPrimBuf1+g_nAreaPt; 
		parea->piFeature[1] = g_AreaFeatureBuf1+g_nAreaPt;
		parea->npt = 0; parea->nmaxpt = sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])-g_nAreaPt;
		parea->minedge = 0;
	} else
		parea = 0;

	if (!pparams->bSweepTest) {
		vectorf ptm;
		if (!pparams->bNoIntersection) {
			if (g_Intersector.CheckExists(itype[0],itype[1]))	{
				inters.ptborder = pcontacts->ptborder;
				inters.nborderpt = 0;
				if (!g_Intersector.Check(itype[0],itype[1], pprim[0],pprim[1], &inters))
					return 0;
				ptm = inters.ptborder[0];
				pcontacts->nborderpt = inters.nborderpt;
				for(i=0,pcontacts->center.zero();i<inters.nborderpt;i++) pcontacts->center += pcontacts->ptborder[i];
				pcontacts->center /= inters.nborderpt;
			}
		} else if (!g_Overlapper.Check(itype[0],itype[1], pprim[0],pprim[1]))
			return 0;

		if (pparams->iUnprojectionMode==0) {
			if (pparams->vrel_min<1E9f) {
				vectorf vrel;
				if (!pcontacts->nborderpt) {
					vectorf center[2],pt[3]; box bbox;
					int iPrim,iFeature; // dummy parameters
					if (pBV1->type==box::type) center[0] = ((box*)(primitive*)*pBV1)->center;
					else { GetBBox(&bbox); center[0] = pdata1->R*bbox.center+pdata1->offset; }
					if (pBV1->type==box::type) center[1] = ((box*)(primitive*)*pBV2)->center;
					else { pCollider->GetBBox(&bbox); center[1] = pdata2->R*bbox.center+pdata2->offset; }
					FindClosestPoint(pdata1,iPrim,iFeature,center[1],center[1],pt+0);
					pCollider->FindClosestPoint(pdata2,iPrim,iFeature,center[0],center[0],pt+1);
					if (pCollider->PointInsideStatus(((pt[0]-pdata2->offset)*pdata2->R)*(pdata2->scale==1.0f ? 1.0f:1.0f/pdata2->scale)))
						ptm = pt[0];
					else if (PointInsideStatus(((pt[1]-pdata1->offset)*pdata1->R)*(pdata1->scale==1.0f ? 1.0f:1.0f/pdata1->scale)))
						ptm = pt[1];
					else
						ptm = (pt[0]+pt[1])*0.5f;
				}
				vrel = pdata1->v+(pdata1->w^ptm-pdata1->centerOfMass)-pdata2->v-(pdata2->w^ptm-pdata2->centerOfMass);
				if (vrel.len2()>sqr(pparams->vrel_min)) {
					unproj.imode = 0;	// unproject along vrel
					(unproj.dir=-vrel) /= (unproj.vel=vrel.len());
					unproj.tmax = pparams->time_interval*unproj.vel;
					bUnprojected = g_Unprojector.Check(&unproj, itype[0],itype[1], pprim[0],-1,pprim[1],-1, &contact_best, parea);
					bUnprojected &= isneg(contact_best.t-unproj.tmax);
				}
			}
		} else {
			unproj.imode = 1;
			unproj.center = pparams->centerOfRotation;
			if (pparams->axisOfRotation.len2()==0) {
				unproj.dir = inters.n^inters.pt[0]-unproj.center;
				if (unproj.dir.len2()<1E-6f)
					unproj.dir = GetOrthogonal(inters.n);
				unproj.dir.normalize();
			} else
				unproj.dir = pparams->axisOfRotation;
			bUnprojected = g_Unprojector.Check(&unproj, itype[0],itype[1], pprim[0],-1,pprim[1],-1, &contact_best, parea);
			if (bUnprojected)
				contact_best.t = atan2(contact_best.t,contact_best.taux);
		}

		if (!bUnprojected) {
			unproj.imode = 0;
			unproj.dir.zero(); // zero requested direction - means minimum direction will be found
			unproj.vel = 0; unproj.tmax = pparams->maxUnproj;
			bUnprojected = g_Unprojector.Check(&unproj, itype[0],itype[1], pprim[0],-1,pprim[1],-1, &contact_best, parea);
		}
	}	else {
		unproj.imode = 0;
		unproj.dir = -sweepdir;
		unproj.tmax = sweepstep;
		unproj.bCheckContact = 1;
		contact_best.t = 0;
		bUnprojected = g_Unprojector.Check(&unproj, itype[0],itype[1], pprim[0],-1,pprim[1],-1, &contact_best, parea);
		bUnprojected &= isneg(contact_best.t-unproj.tmax);
		if (bUnprojected && contact_best.n*unproj.dir>0) {
			// if we hit something with the back side, 1st primitive probably just passed through the 2nd one,
			// so move a little deeper than the contact and unproject again (some primitives check for minimal separating
			// unprojection, and some - for maximum contacting; the former can cause this)
			real t = contact_best.t;
			box bbox; pCollider->GetBBox(&bbox); 
			vectorf dirloc,dirsz;
			dirloc = bbox.Basis*(sweepdir*pdata2->R);
			dirsz(fabs_tpl(dirloc.x)*bbox.size.y*bbox.size.z, fabs_tpl(dirloc.y)*bbox.size.x*bbox.size.z,	fabs_tpl(dirloc.z)*bbox.size.x*bbox.size.y);
			i = idxmax3((float*)&dirsz);
			t += bbox.size[i]/fabs_tpl(dirloc[i])*0.1f;
			unproj.tmax = sweepstep-t;
			pdata1->offset += sweepdir*unproj.tmax;
			itype[0] = PreparePrimitive(pdata1,pprim[0]);
			pdata1->offset -= sweepdir*unproj.tmax;
			bUnprojected = g_Unprojector.Check(&unproj, itype[0],itype[1], pprim[0],-1,pprim[1],-1, &contact_best, parea);
			bUnprojected &= isneg(contact_best.t-unproj.tmax) & isneg(contact_best.n*unproj.dir);
			contact_best.t += t;
			unproj.tmax = sweepstep;
		}
	}

	if (bUnprojected) {
		pcontacts->t = contact_best.t;
		if (pparams->bSweepTest)
			pcontacts->t = unproj.tmax-pcontacts->t;
		pcontacts->pt = contact_best.pt;
		pcontacts->n = contact_best.n.normalized();
		pcontacts->dir = unproj.dir;
		pcontacts->iUnprojMode = unproj.imode;
		pcontacts->vel = unproj.vel;
		pcontacts->id[0] = pcontacts->id[1] = -1;
		pcontacts->iPrim[0] = pcontacts->iPrim[1] = 0;
		pcontacts->iFeature[0] = contact_best.iFeature[0];
		pcontacts->iFeature[1] = contact_best.iFeature[1];
		pcontacts->iNode[0] = pcontacts->iNode[1] = 0;
		if (!parea || parea->npt==0)
			pcontacts->parea = 0;
		else {
			pcontacts->parea = parea;
			g_nAreas++; g_nAreaPt += parea->npt;
			if (pcontacts->nborderpt<parea->npt && (pcontacts->nborderpt==0 || (iszero(itype[0]-cylinder::type) | iszero(itype[1]-cylinder::type)))) {
				pcontacts->ptborder = parea->pt;
				pcontacts->nborderpt = parea->npt;
				for(i=0,pcontacts->center.zero(); i<parea->npt; i++) pcontacts->center += parea->pt[i];
				pcontacts->center /= parea->npt;
			}
		}
		if (pcontacts->nborderpt==0) {
			pcontacts->ptborder[0]=pcontacts->center = pcontacts->pt;
			pcontacts->nborderpt = 1;
		}
		pcontacts->bBorderConsecutive = false;
		g_nTotContacts++;
		g_BrdPtBufPos += pcontacts->nborderpt;
	}

	return bUnprojected;
}

