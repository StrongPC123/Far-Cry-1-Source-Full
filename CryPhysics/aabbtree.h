#ifndef aabbtree_h
#define aabbtree_h

struct AABBnode {
	unsigned int minx : 7;
	unsigned int maxx : 7;
	unsigned int miny : 7;
	unsigned int maxy : 7;
	unsigned int minz : 7;
	unsigned int maxz : 7;
	unsigned int ichild : 15;
	unsigned int ntris : 6;
	unsigned int bSingleColl : 1;
};

class CTriMesh;

class CAABBTree : public CBVTree {
public:
	CAABBTree() {}
	virtual ~CAABBTree() { 
		if (m_pNodes) delete[] m_pNodes; m_pNodes=0; 
		if (m_pTri2Node) delete[] m_pTri2Node; m_pTri2Node=0; 
	}
	virtual int GetType()  { return BVT_AABB; }

	float Build(CGeometry *pMesh);
	virtual void SetGeomConvex();

	void SetParams(int nMinTrisPerNode,int nMaxTrisPerNode, float skipdim, const matrix3x3f &Basis);
	float BuildNode(int iNode, int iTriStart,int nTris, vectorf center,vectorf size, int nDepth);

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
	AABBnode *m_pNodes;
	int m_nNodes,m_nNodesAlloc;
	vectorf m_center;
	vectorf m_size;
	matrix3x3f m_Basis;
	int m_bOriented;
	int *m_pTri2Node,m_nBitsLog;
	int m_nMaxTrisPerNode,m_nMinTrisPerNode;
	int m_nMaxTrisInNode;
	float m_maxSkipDim;
};

#endif