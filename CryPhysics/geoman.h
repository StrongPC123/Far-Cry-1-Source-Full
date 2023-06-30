#ifndef geoman_h
#define geoman_h
#pragma once

const int GEOM_CHUNK_SZ = 64;

class CGeomManager : public IGeomManager {
public:
	CGeomManager() { InitGeoman(); }
	~CGeomManager() { ShutDownGeoman(); }

	void InitGeoman();
	void ShutDownGeoman();

	virtual IGeometry *CreateMesh(strided_pointer<const vectorf> pVertices,index_t *pIndices,const short *pIds,int nTris, int flags,bool bCopyTriangles=true,
		bool bCopyVertices=true, float approx_tolerance=0.05f, int nMinTrisPerNode=2,int nMaxTrisPerNode=4, float favorAABB=1.0f);
	virtual IGeometry *CreatePrimitive(int type, const primitive *pprim);
	virtual void DestroyGeometry(IGeometry *pGeom);

	virtual phys_geometry *RegisterGeometry(IGeometry *pGeom,int defSurfaceIdx=0);
	virtual int AddRefGeometry(phys_geometry *pgeom);
	virtual int UnregisterGeometry(phys_geometry *pgeom);

	virtual void SaveGeometry(CMemStream &stm, IGeometry *pGeom);
	virtual IGeometry *LoadGeometry(CMemStream &stm);
	virtual void SavePhysGeometry(CMemStream &stm, phys_geometry *pgeom);
	virtual phys_geometry *LoadPhysGeometry(CMemStream &stm);
	virtual void RemapPhysGeometryFaceIds(phys_geometry *pgeom,short *pMap,int sz) { pgeom->pGeom->RemapFaceIds(pMap,sz); }

	phys_geometry *GetFreeGeomSlot();

	phys_geometry **m_pGeoms;
	int m_nGeomChunks,m_nGeomsInLastChunk;
};

#endif
