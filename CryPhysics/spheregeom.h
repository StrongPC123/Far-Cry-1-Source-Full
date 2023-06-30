#ifndef spheregeom_h
#define spheregeom_h
#pragma once

class CSphereGeom : public CPrimitive {
public:
	CSphereGeom() { m_iCollPriority = 3; m_minVtxDist = 0; }

	CSphereGeom* CreateSphere(sphere *pcyl);

	virtual int GetType() { return GEOM_SPHERE; }
	virtual void GetBBox(box *pbox) { m_Tree.GetBBox(pbox); }
	virtual int CalcPhysicalProperties(phys_geometry *pgeom);
	virtual int FindClosestPoint(geom_world_data *pgwd, int &iPrim,int &iFeature, const vectorf &ptdst0,const vectorf &ptdst1,
		vectorf *ptres, int nMaxIters);
	virtual int PointInsideStatus(const vectorf &pt);
	virtual float CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, vectorf &massCenter);
	virtual void CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, vectorf &dPres,vectorf &dLres);
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const vectorf &epicenter,float k,float rmin, 
		const vectorf &centerOfMass, vectorf &P,vectorf &L);
	virtual int DrawToOcclusionCubemap(const geom_world_data *pgwd, int iStartPrim,int nPrims, int iPass, int *pGrid[6],int nRes, 
		float rmin,float rmax,float zscale);
	virtual CBVTree *GetBVTree() { return &m_Tree; }
	virtual int UnprojectSphere(vectorf center,float r,float rsep, contact *pcontact);
	virtual int GetPrimitive(int iPrim, primitive *pprim) { *(sphere*)pprim = m_sphere; return sizeof(sphere); }
	virtual void DrawWireframe(void (*DrawLineFunc)(float*,float*), geom_world_data *gwd, int iLevel);

	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false);
	virtual int PreparePrimitive(geom_world_data *pgwd,primitive *&pprim);

	virtual int GetPrimitiveList(int iStart,int nPrims, int typeCollider,primitive *pCollider,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,short *pResId);
	virtual int GetUnprojectionCandidates(int iop,const contact *pcontact, primitive *&pprim,int *&piFeature, geometry_under_test *pGTest);

	virtual void GetMemoryStatistics(ICrySizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm);

	sphere m_sphere;
	CSingleBoxTree m_Tree;
};

#endif