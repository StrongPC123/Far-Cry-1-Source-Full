#include "stdafx.h"

#include "utils.h"
#include "primitives.h"
#include "bvtree.h"
#include "geometry.h"
#include "raybv.h"

BVray CRayBV::g_BVray;

int CRayBV::GetNodeContents(int iNode, BV *pBVCollider,int bColliderUsed,int bColliderLocal, 
														geometry_under_test *pGTest,geometry_under_test *pGTestOp)
{
	return 1;	// ray should already be in the buffer
}

void CRayBV::GetNodeBV(BV *&pBV, int iNode)
{
	pBV = &g_BVray;
	g_BVray.iNode = 0;
	g_BVray.type = ray::type;
	g_BVray.aray.origin = m_pray->origin;
	g_BVray.aray.dir = m_pray->dir;
}

void CRayBV::GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, int iNode)
{
	pBV = &g_BVray;
	g_BVray.iNode = 0;
	g_BVray.type = ray::type;
	g_BVray.aray.origin = Rw*m_pray->origin*scalew + offsw;
	g_BVray.aray.dir = Rw*m_pray->dir*scalew;
}