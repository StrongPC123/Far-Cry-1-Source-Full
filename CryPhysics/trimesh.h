#ifndef trimesh_h
#define trimesh_h

struct trinfo {
	index_t ibuddy[3];
};
struct border_trace {
	vectorf *pt;
	int (*itri)[2];
	float *seglen;
	int npt,szbuf;

	vectorf pt_end;
	int itri_end;
	int iedge_end;
	float end_dist2;

	int iMark,nLoop;

	vectorr n_sum[2];
	vectorf n_best;
	int ntris[2];
};

class CTriMesh : public CGeometry {
public:
	CTriMesh();
	virtual ~CTriMesh();

	CTriMesh* CreateTriMesh(strided_pointer<const vectorf> pVertices,index_t *pIndices,const short *pIds,int nTris, int flags, 
		bool bCanSortTriangles=false,bool bCopyVertices=true, int nMinTrisPerNode=2,int nMaxTrisPerNode=4, float favorAABB=1.0f);

	virtual int GetType() { return GEOM_TRIMESH; }
	virtual int Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual void GetBBox(box *pbox) { m_pTree->GetBBox(pbox); }
	virtual int FindClosestPoint(geom_world_data *pgwd, int &iPrim,int &iFeature, const vectorf &ptdst0,const vectorf &ptdst1,
		vectorf *ptres, int nMaxIters=10);
	virtual int CalcPhysicalProperties(phys_geometry *pgeom);
	virtual int PointInsideStatus(const vectorf &pt);
	virtual void DrawWireframe(void (*DrawLineFunc)(float*,float*),geom_world_data *gwd, int iLevel);
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const vectorf &epicenter,float k,float rmin, 
		const vectorf &centerOfMass, vectorf &P,vectorf &L);
	virtual float CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, vectorf &massCenter);
	virtual void CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, vectorf &dPres,vectorf &dLres);
	virtual int GetPrimitiveId(int iPrim,int iFeature) { return m_pIds ? m_pIds[iPrim]:-1; }
	virtual int IsConvex(float tolerance);
	virtual void PrepareForRayTest(float raylen);
	virtual int DrawToOcclusionCubemap(const geom_world_data *pgwd, int iStartPrim,int nPrims, int iPass, int *pGrid[6],int nRes, 
		float rmin,float rmax,float zscale);
	virtual CBVTree *GetBVTree() { return m_pTree; }
	virtual int GetPrimitiveCount() { return m_nTris; }
	virtual int GetPrimitive(int iPrim, primitive *pprim) { 
		if ((unsigned int)iPrim>=(unsigned int)m_nTris) 
			return 0;
		((triangle*)pprim)->pt[0] = m_pVertices[m_pIndices[iPrim*3+0]];
		((triangle*)pprim)->pt[1] = m_pVertices[m_pIndices[iPrim*3+1]];
		((triangle*)pprim)->pt[2] = m_pVertices[m_pIndices[iPrim*3+2]];
		((triangle*)pprim)->n = m_pNormals[iPrim];
		return sizeof(triangle); 
	}
	virtual int GetFeature(int iPrim,int iFeature, vectorf *pt);
	virtual int PreparePrimitive(geom_world_data *pgwd,primitive *&pprim);
	virtual void RemapFaceIds(short *pMap,int sz) { if (m_pIds) for(int i=0;i<m_nTris;i++) m_pIds[i]=pMap[min(m_pIds[i],sz-1)]; }

	virtual void GetMemoryStatistics(ICrySizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm);

	virtual int PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false);
	virtual int RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2, 
		prim_inters *pinters);
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest);

	virtual int GetPrimitiveList(int iStart,int nPrims, int typeCollider,primitive *pCollider,int bColliderLocal, 
		geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,short *pResId);
	virtual int GetUnprojectionCandidates(int iop,const contact *pcontact, primitive *&pprim,int *&piFeature, geometry_under_test *pGTest);
	virtual int PreparePolygon(coord_plane *psurface, int iPrim,int iFeature, geometry_under_test *pGTest, vector2df *&ptbuf,
		int *&pVtxIdBuf,int *&pEdgeIdBuf);
	virtual int PreparePolyline(coord_plane *psurface, int iPrim,int iFeature, geometry_under_test *pGTest, vector2df *&ptbuf,
		int *&pVtxIdBuf,int *&pEdgeIdBuf);

	int GetEdgeByBuddy(int itri,int itri_buddy) {
		int iedge=0,imask;
		imask = m_pTopology[itri].ibuddy[1]-itri_buddy; imask = imask-1>>31 ^ imask>>31; iedge = 1&imask;
		imask = m_pTopology[itri].ibuddy[2]-itri_buddy; imask = imask-1>>31 ^ imask>>31; iedge = iedge&~imask | 2&imask;
		return iedge;
	}
	int GetNeighbouringEdgeId(int vtxid, int ivtx);
	void PrepareTriangle(int itri,triangle *ptri, const geometry_under_test *pGTest);
	int TraceTriangleInters(int iop, primitive *pprims[], int idx_buddy,int type_buddy, prim_inters *pinters, 
													geometry_under_test *pGTest, border_trace *pborder);
	void HashTrianglesToPlane(const coord_plane &hashplane, const vector2df &hashsize, grid &hashgrid,index_t *&pHashGrid,index_t *&pHashData,float cellsize=0);

	CBVTree *m_pTree;
	trinfo *m_pTopology;
	index_t *m_pIndices;
	short *m_pIds;
	strided_pointer<vectorf> m_pVertices;
	vectorf *m_pNormals;
	int m_nTris,m_nVertices;
	int m_nMaxVertexValency;
	int m_flags;
	index_t *m_pHashGrid[3],*m_pHashData[3];
	grid m_hashgrid[3];
	int m_nHashPlanes;
	int m_bConvex[4];
	float m_ConvexityTolerance[4];
	int m_bMultipart;
};

#endif