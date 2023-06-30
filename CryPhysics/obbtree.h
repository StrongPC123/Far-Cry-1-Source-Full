#ifndef obbtree_h
#define obbtree_h
#pragma once

struct OBBnode {
	vectorf axes[3];
	vectorf center;
	vectorf size;
	int iparent;
	int ichild;
	int ntris;
};

class CTriMesh;

class COBBTree : public CBVTree {
public:
	COBBTree() { m_nMinTrisPerNode=2; m_nMaxTrisPerNode=4; m_maxSkipDim=0; m_pNodes=0; m_pTri2Node=0; }
	virtual ~COBBTree() { 
		if (m_pNodes) delete[] m_pNodes; m_pNodes=0; 
		if (m_pTri2Node) delete[] m_pTri2Node; m_pTri2Node=0; 
	}
	virtual int GetType() { return BVT_OBB; }

	virtual float Build(CGeometry *pMesh);
	virtual void SetGeomConvex();

	void SetParams(int nMinTrisPerNode,int nMaxTrisPerNode, float skipdim);
	float BuildNode(int iNode, int iTriStart,int nTris, int nDepth);

	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest);
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest);
	virtual void GetBBox(box *pbox); 
	virtual int MaxPrimsInNode() { return m_nMaxTrisInNode; }
	virtual void GetNodeBV(BV *&pBV, int iNode=0);
	virtual void GetNodeBV(BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0);
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, int iNode=0);
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0) {}
	virtual float SplitPriority(const BV* pBV);
	virtual void GetNodeChildrenBVs(const matrix3x3f &Rw,const vectorf &offsw,float scalew, const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2);
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2);
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, const vectorf &sweepdir,float sweepstep, BV *&pBV_child1,BV *&pBV_child2);
	virtual void ReleaseLastBVs();
	virtual void ReleaseLastSweptBVs();
	virtual int GetNodeContents(int iNode, BV *pBVCollider,int bColliderUsed,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);
	virtual int GetNodeContentsIdx(int iNode, int &iStartPrim) { iStartPrim = m_pNodes[iNode].ichild; return m_pNodes[iNode].ntris; }
	virtual void MarkUsedTriangle(int itri, geometry_under_test *pGTest);

	virtual void GetMemoryStatistics(ICrySizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm, CGeometry *pGeom);

	CTriMesh *m_pMesh;
	OBBnode *m_pNodes;
	int m_nNodes;
	int m_nNodesAlloc;
	index_t *m_pTri2Node;
	int m_nMaxTrisInNode;

	int m_nMinTrisPerNode;
	int m_nMaxTrisPerNode;
	float m_maxSkipDim;
	int *m_pMapVtxUsed;
	vectorf *m_pVtxUsed;
};

#endif