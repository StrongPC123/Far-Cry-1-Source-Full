#include "stdafx.h"

#include "utils.h"
#include "primitives.h"
#include "bvtree.h"
#include "geometry.h"
#include "singleboxtree.h"

float CSingleBoxTree::Build(CGeometry *pGeom) 
{ 
	m_pGeom = pGeom; 
	return m_Box.size.volume()*8; 
}

void CSingleBoxTree::SetBox(box *pbox)
{
	m_Box.Basis = pbox->Basis;
	m_Box.bOriented = m_Box.Basis.IsIdentity()^1;
	m_Box.center = pbox->center;
	m_Box.size = pbox->size;
}

void CSingleBoxTree::GetNodeBV(BV *&pBV,int iNode)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	((BBox*)pBV)->abox.Basis = m_Box.Basis;
	((BBox*)pBV)->abox.bOriented = m_Box.bOriented;
	((BBox*)pBV)->abox.center = m_Box.center;
	((BBox*)pBV)->abox.size = m_Box.size;
}
void CSingleBoxTree::GetNodeBV(BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	box boxstatic;
	boxstatic.Basis = m_Box.Basis;
	boxstatic.bOriented = m_Box.bOriented;
	boxstatic.center = m_Box.center;
	boxstatic.size = m_Box.size;
	ExtrudeBox(&boxstatic, sweepdir,sweepstep, &((BBox*)pBV)->abox);
}
void CSingleBoxTree::GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, int iNode)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	((BBox*)pBV)->abox.Basis = m_Box.Basis*Rw.T();
	((BBox*)pBV)->abox.bOriented = 1;
	((BBox*)pBV)->abox.center = Rw*m_Box.center*scalew + offsw;
	((BBox*)pBV)->abox.size = m_Box.size*scalew;
}
void CSingleBoxTree::GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, 
															 const vectorf &sweepdir,float sweepstep, int iNode)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	box boxstatic;
	boxstatic.Basis = m_Box.Basis*Rw.T();
	boxstatic.bOriented = 1;
	boxstatic.center = Rw*m_Box.center*scalew + offsw;
	boxstatic.size = m_Box.size*scalew;
	ExtrudeBox(&boxstatic, sweepdir,sweepstep, &((BBox*)pBV)->abox);
}

int CSingleBoxTree::GetNodeContents(int iNode, BV *pBVCollider,int bColliderUsed,int bColliderLocal, 
																		geometry_under_test *pGTest,geometry_under_test *pGTestOp)
{
	return m_pGeom->GetPrimitiveList(0,m_nPrims, pBVCollider->type,*pBVCollider,bColliderLocal, pGTest,pGTestOp, pGTest->primbuf,pGTest->idbuf);
}

int CSingleBoxTree::PrepareForIntersectionTest(geometry_under_test *pGTest)
{
	pGTest->pUsedNodesMap = &pGTest->nUsedNodes;
	pGTest->nUsedNodes = 0;
	return 1;
}

void CSingleBoxTree::MarkUsedTriangle(int itri, geometry_under_test *pGTest)
{
	pGTest->nUsedNodes = 1;
}

void CSingleBoxTree::GetBBox(box *pbox)
{
	pbox->Basis = m_Box.Basis;
	pbox->bOriented = m_Box.bOriented;
	pbox->center = m_Box.center;
	pbox->size = m_Box.size;
}


void CSingleBoxTree::GetMemoryStatistics(ICrySizer *pSizer)
{
	SIZER_COMPONENT_NAME(pSizer, "SingleBox trees");
	pSizer->AddObject(this, sizeof(CSingleBoxTree));
}


void CSingleBoxTree::Save(CMemStream &stm)
{
	stm.Write(m_Box);
	stm.Write(m_nPrims);
}

void CSingleBoxTree::Load(CMemStream &stm, CGeometry *pGeom)
{
	m_pGeom = pGeom;
	stm.Read(m_Box);
	stm.Read(m_nPrims);
}