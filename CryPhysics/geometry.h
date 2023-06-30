#ifndef geometry_h
#define geometry_h
#pragma once


extern surface_desc g_SurfaceDescBuf[64];
extern int g_SurfaceDescBufPos;

extern edge_desc g_EdgeDescBuf[64];
extern int g_EdgeDescBufPos;

extern int g_iFeatureBuf[64];
extern int g_iFeatureBufPos;

extern short g_IdBuf[256];
extern int g_IdBufPos;

extern int g_UsedNodesMap[8192];
extern int g_UsedNodesMapPos;
extern int g_UsedNodesIdx[64];
extern int g_UsedNodesIdxPos;

extern geom_contact g_Contacts[64];
extern int g_nTotContacts;

extern geom_contact_area g_AreaBuf[32];
extern int g_nAreas;
extern vectorf g_AreaPtBuf[256];
extern int g_AreaPrimBuf0[256],g_AreaFeatureBuf0[256],g_AreaPrimBuf1[256],g_AreaFeatureBuf1[256];
extern int g_nAreaPt;

class CGeometry : public IGeometry {
public:
	CGeometry() { m_bIsConvex=0; }
	virtual ~CGeometry() {}
	virtual int GetType() = 0;
	virtual void Release() { delete this; }
	virtual void GetBBox(box *pbox) = 0;
	virtual int Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual int CalcPhysicalProperties(phys_geometry *pgeom) { return 0; }
	virtual int FindClosestPoint(geom_world_data *pgwd, int &iPrim,int &iFeature, const vectorf &ptdst0,const vectorf &ptdst1, 
		vectorf *ptres, int nMaxIters=10) { return 0; }
	virtual int PointInsideStatus(const vectorf &pt) { return -1; }
	virtual void DrawWireframe(void (*DrawLineFunc)(float*,float*), geom_world_data *gwd, int iLevel) {}
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const vectorf &epicenter,float k,float rmin, 
		const vectorf &centerOfMass, vectorf &P,vectorf &L) {}
	virtual float CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, vectorf &massCenter) { massCenter.zero(); return 0; }
	virtual void CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, vectorf &dPres,vectorf &dLres) { dPres.zero(); dLres.zero(); }
	virtual int GetPrimitiveId(int iPrim,int iFeature) { return -1; }
	virtual int IsConvex(float tolerance) { return 1; }
	virtual void PrepareForRayTest(float raylen) {}
	virtual CBVTree *GetBVTree() { return 0; }
	virtual int GetPrimitiveCount() { return 1; }
	virtual int IsAPrimitive() { return 0; }
	virtual int PreparePrimitive(geom_world_data *pgwd,primitive *&pprim) { return -1; }
	virtual int GetFeature(int iPrim,int iFeature, vectorf *pt) { return 0; }
	virtual void RemapFaceIds(short *pMap,int sz) {}
	virtual int UnprojectSphere(vectorf center,float r,float rsep, contact *pcontact) { return 0; }

	virtual float BuildOcclusionCubemap(geom_world_data *pgwd, int iMode, int *pGrid0[6],int *pGrid1[6],int nRes, float rmin,float rmax, int nGrow);
	virtual int DrawToOcclusionCubemap(const geom_world_data *pgwd, int iStartPrim,int nPrims, int iPass, int *pGrid[6],int nRes, 
		float rmin,float rmax,float zscale) { return 0; }

	virtual void GetMemoryStatistics(ICrySizer *pSizer) {}
	virtual void Save(CMemStream &stm) {}
	virtual void Load(CMemStream &stm) {}

	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false) 
	{ return 1; };
	virtual int RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2, 
		prim_inters *pinters);
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest) {}

	virtual int GetPrimitiveList(int iStart,int nPrims, int typeCollider,primitive *pCollider,int bColliderLocal, geometry_under_test *pGTest,
		geometry_under_test *pGTestOp, primitive *pRes,short *pResId) { return 0; }
	virtual int GetUnprojectionCandidates(int iop,const contact *pcontact, primitive *&pprim,int *&piFeature, geometry_under_test *pGTest) { return 0; }
	virtual int PreparePolygon(coord_plane *psurface, int iPrim,int iFeature, geometry_under_test *pGTest, vector2df *&ptbuf,
		int *&pVtxIdBuf,int *&pEdgeIdBuf) { return 0; }
	virtual int PreparePolyline(coord_plane *psurface, int iPrim,int iFeature, geometry_under_test *pGTest, vector2df *&ptbuf,
		int *&pVtxIdBuf,int *&pEdgeIdBuf) { return 0; }

	float m_minVtxDist;
	int m_iCollPriority;
	int m_bIsConvex;
};

class CPrimitive : public CGeometry {
public:
	CPrimitive() { m_bIsConvex=1; }
	virtual int Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual int IsAPrimitive() { return 1; }
};

void DrawBBox(void (*DrawLineFunc)(float*,float*), geom_world_data *gwd, CBVTree *pTree,BBox *pbbox,int maxlevel,int level=0);

#endif
