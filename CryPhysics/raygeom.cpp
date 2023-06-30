#include "stdafx.h"

#include "utils.h"
#include "primitives.h"
#include "bvtree.h"
#include "geometry.h"
#include "raybv.h"
#include "raygeom.h"

void CRayGeom::GetBBox(box *pbox) 
{
	pbox->Basis.SetRow(2,m_dirn);
	//pbox->Basis.SetRow(1,m_dirn.orthogonal().normalized());
	pbox->Basis.SetRow(1,GetOrthogonal(m_dirn).normalized());
	pbox->Basis.SetRow(0,pbox->Basis.GetRow(1)^m_dirn);
}

int CRayGeom::PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts)
{
	static short g_id = -1;
	m_Tree.PrepareForIntersectionTest(pGTest);
	pGTest->pGeometry = this;
	pGTest->pBVtree = &m_Tree;
	pGTest->primbuf = pGTest->primbuf1 = &m_ray;
	pGTest->typeprim = ray::type;
	pGTest->szprim = sizeof(ray);
	pGTest->idbuf = &g_id;
	pGTest->surfaces = 0;
	pGTest->edges = 0;
	pGTest->minAreaEdge = 0;
	return 1;
}

int CRayGeom::PreparePrimitive(geom_world_data *pgwd, primitive *&pprim)
{
	pprim = &m_ray;
	return ray::type;
}

int CRayGeom::GetPrimitiveList(int iStart,int nPrims, int typeCollider,primitive *pCollider,int bColliderLocal, 
													 geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,short *pResId)
{
	return 1;
}

int CRayGeom::RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2, prim_inters *pinters)
{
	geom_contact *pres = pGTest1->contacts + *pGTest1->pnContacts;
	(*pGTest1->pnContacts)++;
	pres->ptborder = &pres->pt;
	pres->nborderpt = 1;
	pres->parea = 0;

	pres->t = (pinters->pt[0]-m_ray.origin)*m_dirn;
	pres->pt = pinters->pt[0];
	pres->n = -pinters->n;
	pres->dir.zero();
	pres->vel = 0;
	pres->iUnprojMode = 0;
	pres->id[0] = -1;
	pres->id[1] = pinters->id[1];
	pres->iPrim[0] = 0; pres->iFeature[0] = 0x20;
	pres->iPrim[1] = pGTest2 && pGTest2->typeprim==indexed_triangle::type ? ((indexed_triangle*)pprim2)->idx : 0;
	pres->iFeature[1] = pinters->iFeature[0][1];
	pres->iNode[0] = pinters->iNode[0];
	pres->iNode[1] = pinters->iNode[1];
	if (*pGTest1->pnContacts>=pGTest1->nMaxContacts || pGTest1->pParams->bStopAtFirstTri)
		pGTest1->bStopIntersection = 1;
	return 1;
}