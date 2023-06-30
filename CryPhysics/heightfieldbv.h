#ifndef heightfieldbv_h
#define heightfieldbv_h
#pragma once

class CHeightfieldBV : public CBVTree {
public:
	CHeightfieldBV() { m_pUsedTriMap=0; }
	virtual ~CHeightfieldBV() { if (m_pUsedTriMap) delete[] m_pUsedTriMap; }
	virtual int GetType() { return BVT_HEIGHTFIELD; }

	virtual float Build(CGeometry *pGeom);
	void SetHeightfield(heightfield *phf);
	virtual void GetBBox(box *pbox);
	virtual int MaxPrimsInNode() { return m_PatchSize.x*m_PatchSize.y*2; }

	virtual void GetNodeBV(BV *&pBV, int iNode=0);
	virtual void GetNodeBV(BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0) {}
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, int iNode=0);
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0) {}
	virtual int GetNodeContents(int iNode, BV *pBVCollider,int bColliderUsed,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);
	virtual void MarkUsedTriangle(int itri, geometry_under_test *pGTest);

	CTriMesh *m_pMesh;
	heightfield *m_phf;
	vector2di m_PatchStart;
	vector2di m_PatchSize;
	float m_minHeight,m_maxHeight;
	unsigned int *m_pUsedTriMap;
	
	static BVheightfield g_BVhf;
};

void project_box_on_grid(box *pbox,grid *pgrid, geometry_under_test *pGTest, int &ix,int &iy,int &sx,int &sy,float &minz);

#endif
