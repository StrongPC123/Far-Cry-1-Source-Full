#ifndef heightfieldgeom_h
#define heightfieldgeom_h
#pragma once

class CHeightfield : public CTriMesh {
public:
	CHeightfield() {}
	virtual ~CHeightfield() { m_pTree=0; }

	CHeightfield* CreateHeightfield(heightfield *phf);
	virtual int GetType() { return GEOM_HEIGHTFIELD; }
	virtual int Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false);
	virtual int PointInsideStatus(const vectorf &pt) { return -1; }
	virtual int FindClosestPoint(geom_world_data *pgwd, int &iPrim,int &iFeature, const vectorf &ptdst0,const vectorf &ptdst1,
		vectorf *ptres, int nMaxIters=10);
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const vectorf &epicenter,float k,float rmin, 
		const vectorf &centerOfMass, vectorf &P,vectorf &L);
	virtual int IsConvex(float tolerance) { return 0; }
	virtual int DrawToOcclusionCubemap(const geom_world_data *pgwd, int iStartPrim,int nPrims, int iPass, int *pGrid[6],int nRes, 
		float rmin,float rmax,float zscale);
	virtual void PrepareForRayTest(float raylen) {}
	virtual CBVTree *GetBVTree() { return &m_Tree; }

	virtual int GetPrimitive(int iPrim, primitive *pprim) { *(heightfield*)pprim = m_hf; return sizeof(heightfield); }
	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	heightfield m_hf;
	CHeightfieldBV m_Tree;
	float m_minHeight,m_maxHeight;
	int m_nVerticesAlloc,m_nTrisAlloc;
};

#endif
