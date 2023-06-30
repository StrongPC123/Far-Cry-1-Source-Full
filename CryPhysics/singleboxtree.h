#ifndef singleboxtree_h
#define singleboxtree_h

class CSingleBoxTree : public CBVTree {
public:
	CSingleBoxTree() { m_nPrims = 1; }
	virtual int GetType() { return BVT_SINGLEBOX; }
	virtual float Build(CGeometry *pGeom);
	void SetBox(box *pbox);
	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest);
	virtual void GetBBox(box *pbox);
	virtual int MaxPrimsInNode() { return m_nPrims; }
	virtual void GetNodeBV(BV *&pBV, int iNode=0);
	virtual void GetNodeBV(BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0);
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, int iNode=0);
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0);
	virtual int GetNodeContents(int iNode, BV *pBVCollider,int bColliderUsed,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);
	virtual int GetNodeContentsIdx(int iNode, int &iStartPrim) { iStartPrim=0; return m_nPrims; }
	virtual void MarkUsedTriangle(int itri, geometry_under_test *pGTest);

	virtual void GetMemoryStatistics(ICrySizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm, CGeometry *pGeom);

	CGeometry *m_pGeom;
	box m_Box;
	int m_nPrims;
};

#endif