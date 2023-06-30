#ifndef raygeom_h
#define raygeom_h
#pragma once

class CRayGeom : public CGeometry {
public:
	CRayGeom() { m_iCollPriority=0; m_Tree.Build(this); m_Tree.SetRay(&m_ray); m_minVtxDist=1.0f; }
	CRayGeom(const ray *pray) {
		m_iCollPriority=0; m_ray.origin = pray->origin; m_ray.dir = pray->dir; m_dirn = pray->dir.normalized();
		m_Tree.Build(this); m_Tree.SetRay(&m_ray); m_minVtxDist=1.0f;
	}
	CRayGeom(const vectorf &origin, const vectorf &dir) { 
		m_iCollPriority=0; m_ray.origin = origin; m_ray.dir = dir; m_dirn = dir.normalized();
		m_Tree.Build(this); m_Tree.SetRay(&m_ray); m_minVtxDist=1.0f;
	}
	CRayGeom *CreateRay(const vectorf &origin, const vectorf &dir, const vectorf *pdirn=0) {
		m_ray.origin = origin; m_ray.dir = dir; m_dirn = pdirn ? *pdirn : dir.normalized(); return this;
	}

	virtual int GetType() { return GEOM_RAY; }
	virtual int IsAPrimitive() { return 1; }
	virtual void GetBBox(box *pbox);
	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts);
	virtual int RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2, 
		prim_inters *pinters);
	virtual int GetPrimitiveList(int iStart,int nPrims, int typeCollider,primitive *pCollider,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,short *pResId);
	virtual int PreparePrimitive(geom_world_data *pgwd,primitive *&pprim);
	virtual CBVTree *GetBVTree() { return &m_Tree; }
	virtual int GetPrimitive(int iPrim, primitive *pprim) { *(ray*)pprim = m_ray; return sizeof(ray); }

	ray m_ray;
	vectorf m_dirn;
	CRayBV m_Tree;
};

#endif