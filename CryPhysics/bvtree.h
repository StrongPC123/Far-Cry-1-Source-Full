#ifndef bvtree_h
#define bvtree_h

extern indexed_triangle g_IdxTriBuf[256];
extern int g_IdxTriBufPos;
extern cylinder g_CylBuf[2];
extern int g_CylBufPos;
extern sphere g_SphBuf[2];
extern int g_SphBufPos;
extern box g_BoxBuf[2];
extern int g_BoxBufPos;

////////////////////////// bounding volumes ////////////////////////

struct BV {
	int type;
	int iNode;
	operator primitive*() { return (primitive*)((char*)this+sizeof(type)+sizeof(iNode)); }
};

struct BBox : BV {
	box abox;
};
extern BBox g_BBoxBuf[128];
extern int g_BBoxBufPos;

struct BVheightfield : BV {
	heightfield hf;
};

struct BVray : BV {
	ray aray;
};

inline void ResetGlobalPrimsBuffers()
{
	g_BBoxBufPos = 0;
	g_IdxTriBufPos = 0;
	g_CylBufPos = 0;
	g_BoxBufPos = 0;
	g_SphBufPos = 0;
}

class CGeometry;
class CBVTree;

struct surface_desc {
	vectorf n;
	int idx;
	int iFeature;
};
struct edge_desc {
	vectorf dir;
	vectorf n[2];
	int idx;
	int iFeature;
};

struct geometry_under_test {
	CGeometry *pGeometry;
	CBVTree *pBVtree;
	int *pUsedNodesMap;
	int *pUsedNodesIdx;
	int nUsedNodes;
	int nMaxUsedNodes;
	int bStopIntersection;
	int bCurNodeUsed;

	matrix3x3f R,R_rel;
	vectorf offset,offset_rel;
	float scale,rscale, scale_rel,rscale_rel;
	int bTransformUpdated;

	vectorf v;
	vectorf w,centerOfMass;
	vectorf centerOfRotation;
	intersection_params *pParams;

	vectorf axisContactNormal;
	vectorf sweepdir,sweepdir_loc;
	float sweepstep,sweepstep_loc;
	vectorf ptOutsidePivot;

	int typeprim;
	primitive *primbuf; // used to get node contents
	primitive *primbuf1;// used to get unprojection candidates
	int szprimbuf,szprimbuf1;
	int *iFeature_buf; // feature that led to this primitive
	short *idbuf; // ids of unprojection candidates
	int szprim;

	surface_desc *surfaces;	// the last potentially surfaces 
	edge_desc *edges;	// the last potentially contacting edges
	int nSurfaces,nEdges;
	float minAreaEdge;

	geom_contact *contacts;
	int *pnContacts;
	int nMaxContacts;
};

enum BVtreetypes { BVT_OBB=0, BVT_AABB=1, BVT_SINGLEBOX=2, BVT_RAY=3, BVT_HEIGHTFIELD=4 };

class CBVTree {
public:
	virtual ~CBVTree() {}
	virtual int GetType() = 0;
	virtual void GetBBox(box *pbox) {}
	virtual int MaxPrimsInNode() { return 1; }
	virtual float Build(CGeometry *pGeom) = 0;
	virtual void SetGeomConvex() {}

	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest) {
		pGTest->pUsedNodesMap = 0;
		pGTest->pUsedNodesIdx = 0;
		pGTest->nMaxUsedNodes = 0;
		pGTest->nUsedNodes = -1;
		return 1;
	}
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest) {}
	virtual void GetNodeBV(BV *&pBV, int iNode=0) = 0;
	virtual void GetNodeBV(BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0) = 0;
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, int iNode=0) = 0;
	virtual void GetNodeBV(const matrix3x3f &Rw,const vectorf &offsw,float scalew, BV *&pBV, const vectorf &sweepdir,float sweepstep, int iNode=0) = 0;
	virtual float SplitPriority(const BV* pBV) { return 0.0f; }
	virtual void GetNodeChildrenBVs(const matrix3x3f &Rw,const vectorf &offsw,float scalew, const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2) {}
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2) {}
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, const vectorf &sweepdir,float sweepstep, BV *&pBV_child1,BV *&pBV_child2) {}
	virtual void ReleaseLastBVs() {}
	virtual void ReleaseLastSweptBVs() {}

	virtual void GetMemoryStatistics(ICrySizer *pSizer) {}
	virtual void Save(CMemStream &stm) {}
	virtual void Load(CMemStream &stm, CGeometry *pGeom) {}

	virtual int GetNodeContents(int iNode, BV *pBVCollider,int bColliderUsed,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp) = 0;
	virtual int GetNodeContentsIdx(int iNode, int &iStartPrim) { iStartPrim=0; return 1; }
	virtual void MarkUsedTriangle(int itri, geometry_under_test *pGTest) {}
};

#endif
