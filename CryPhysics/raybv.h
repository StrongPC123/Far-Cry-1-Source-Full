#ifndef raybv_h
#define raybv_h
#pragma once

class CRayBV : public CBVTree {
public:
	CRayBV() {}
	virtual int GetType() { return BVT_RAY; }
	virtual float Build(CGeometry *pGeom) { m_pGeom=pGeom; return 0.0f; }
	void SetRay(ray *pray) { m_pray = pray; }
	virtual void GetNodeBV(BV *&pBV, int iNode=0);
	virtual void GetNodeBV(BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0) {}
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, int iNode=0);
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0) {}
	virtual int GetNodeContents(int iNode, BV *pBVCollider,int bColliderUsed,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);

	CGeometry *m_pGeom;
	ray *m_pray;
	static BVray g_BVray;
};

#endif

