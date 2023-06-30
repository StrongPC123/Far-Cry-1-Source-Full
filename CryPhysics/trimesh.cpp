#include "stdafx.h"

#include "utils.h"
#include "primitives.h"
#include "overlapchecks.h"
#include "intersectionchecks.h"
#include "unprojectionchecks.h"
#include "bvtree.h"
#include "aabbtree.h"
#include "obbtree.h"
#include "singleboxtree.h"
#include "geometry.h"
#include "trimesh.h"
#include "raybv.h"
#include "raygeom.h"

vectorf g_BrdPtBuf[2048];
int g_BrdPtBufPos,g_BrdPtBufStart;
int g_BrdiTriBuf[2048][2];
float g_BrdSeglenBuf[2048];
int g_UsedVtxMap[4096];
int g_UsedTriMap[4096];
vector2df g_PolyPtBuf[1024];
int g_PolyVtxIdBuf[1024];
int g_PolyEdgeIdBuf[1024];
int g_PolyPtBufPos;

struct tritem {
	int itri;
	int itri_parent;
	int ivtx0;
};
struct vtxitem {
	int ivtx;
	int id;
	int ibuddy[2];
};
tritem g_TriQueue[512];
vtxitem g_VtxList[512];

struct InitTriMeshGlobals {
	InitTriMeshGlobals() {
		memset(g_UsedVtxMap, 0, sizeof(g_UsedVtxMap));
		memset(g_UsedTriMap, 0, sizeof(g_UsedTriMap));
		g_BrdPtBufPos = 0; g_PolyPtBufPos = 0;
	}
};
static InitTriMeshGlobals now;


CTriMesh::CTriMesh() 
{ 
	m_iCollPriority = 1; m_pTree=0;m_pTopology=0;m_pIndices=0;m_pIds=0;m_pVertices=0;m_pNormals=0; m_flags=0;
	for(int i=0;i<sizeof(m_bConvex)/sizeof(m_bConvex[0]);i++) m_ConvexityTolerance[i]=-1;
	m_nHashPlanes = 0; m_bMultipart = 0;
}


CTriMesh::~CTriMesh()
{
	if (m_pTree) delete m_pTree;
	if (m_pTopology) delete[] m_pTopology;
	if (m_flags & 2) {
		if (m_pIndices) delete[] m_pIndices;
		if (m_pIds) delete[] m_pIds;
	}
	if (m_flags & 1)
		if (m_pVertices) delete[] m_pVertices;
	if (m_pNormals && !(m_flags&4)) delete[] m_pNormals;
	for(int i=0;i<m_nHashPlanes;i++) {
		delete[] m_pHashGrid[i]; delete[] m_pHashData[i];
	}
}

inline void swap(int *v, int i1,int i2)
{	int ti=v[i1]; v[i1]=v[i2]; v[i2]=ti; }
void qsort(int *v,index_t *idx, int left,int right)
{
	if (left>=right) return;
	int i,last; 
	swap(v, left, left+right>>1);
	for(last=left,i=left+1; i<=right; i++)
	if (idx[v[i]*3] < idx[v[left]*3])
		swap(v, ++last, i);
	swap(v, left,last);

	qsort(v,idx, left,last-1);
	qsort(v,idx, last+1,right);
}
int bin_search(int *v,index_t *idx,int n,int refidx)
{
	int left=0,right=n,m;
	do {
		m = left+right>>1;
		if (idx[v[m]*3]==refidx) return m;
		if (idx[v[m]*3]<refidx) left=m;
		else right=m;
	} while (left<right-1);
	return left;
}

template<class ftype> void qsort(int *v,strided_pointer<ftype> pkey, int left,int right)
{
	if (left>=right) return;
	int i,last; 
	swap(v, left, left+right>>1);
	for(last=left,i=left+1; i<=right; i++)
	if (pkey[v[i]] < pkey[v[left]])
		swap(v, ++last, i);
	swap(v, left,last);

	qsort(v,pkey, left,last-1);
	qsort(v,pkey, last+1,right);
}
void qsort(int *v, int left,int right)
{
	if (left>=right) return;
	int i,last; 
	swap(v, left, left+right>>1);
	for(last=left,i=left+1; i<=right; i++)
	if (v[i] < v[left])
		swap(v, ++last, i);
	swap(v, left,last);

	qsort(v, left,last-1);
	qsort(v, last+1,right);
}


int __gtrimesh = 0;

CTriMesh* CTriMesh::CreateTriMesh(strided_pointer<const vectorf> pVertices,index_t *pIndices,const short *pIds,int nTris, int flags, 
																	bool bCopyTriangles,bool bCopyVertices, int nMinTrisPerNode,int nMaxTrisPerNode, float favorAABB)
{
	__gtrimesh++;
	int i;
	m_nTris = nTris;
	m_pNormals = new vectorf[m_nTris];
	m_nVertices = 0;
	for(i=0;i<m_nTris;i++)
		m_nVertices = max(m_nVertices, max(max(pIndices[i*3],pIndices[i*3+1]),pIndices[i*3+2]));
	m_nVertices++;
	if (bCopyVertices) {
		memcpy(m_pVertices = new vectorf[m_nVertices], pVertices, m_nVertices*sizeof(vectorf));
		m_flags |= 1;
	} else
		m_pVertices = strided_pointer<vectorf>((vectorf*)pVertices.data,pVertices.iStride); // removes const modifier

	unsigned char *pValency = new unsigned char[m_nVertices];
	memset(pValency, 0, m_nVertices);
	for(i=nTris*3-1;i>=0;i--) pValency[pIndices[i]]++;
	for(m_nMaxVertexValency=i=0;i<m_nVertices;i++)
		m_nMaxVertexValency = max(m_nMaxVertexValency, (int)pValency[i]);
	delete[] pValency;

	if (bCopyTriangles) {
		memcpy(m_pIndices = new index_t[m_nTris*3], pIndices, m_nTris*3*sizeof(index_t));
		if (pIds) {
			m_pIds = new short[m_nTris];
			if (flags & mesh_uchar_ids)
				for(i=0;i<nTris;i++) m_pIds[i] = ((unsigned char*)pIds)[i];
			else
				memcpy(m_pIds, pIds, m_nTris*sizeof(short));
		}
		m_flags |= 2;
	} else {
		m_pIndices=pIndices; m_pIds=(short*)pIds; 
	}
	
	int *vsort[3],j,k,ibuddy,*pList[4],nList[4];
	struct vtxinfo {
		int id;
		vectori isorted;
	};
	vtxinfo *pVtx;
	float coord;

	// for mesh connectivity calculations, temporarily assign same ids to the vertices that have the same coordinates
	for(i=0;i<3;i++) vsort[i] = new int[max(m_nTris,m_nVertices)];
	for(i=0;i<4;i++) pList[i] = new int[m_nVertices];
	pVtx = new vtxinfo[m_nVertices];
	for(i=0;i<m_nVertices;i++) vsort[0][i]=vsort[1][i]=vsort[2][i]=i;
	for(j=0;j<3;j++) {
		qsort(vsort[j],strided_pointer<float>((float*)m_pVertices.data+j,m_pVertices.iStride),0,m_nVertices-1);
		for(i=0;i<m_nVertices;i++) pVtx[vsort[j][i]].isorted[j] = i;
	}
	for(i=0;i<m_nVertices;i++) pVtx[i].id=-1;
	for(i=0;i<m_nVertices;i++) if (pVtx[i].id<0) {
		for(j=0;j<3;j++) {
			for(k=pVtx[i].isorted[j],coord=m_pVertices[i][j]; k>0 && m_pVertices[vsort[j][k-1]][j]==coord; k--);
			for(nList[j]=0; k<m_nVertices && m_pVertices[vsort[j][k]][j]==coord; k++)
				pList[j][nList[j]++] = vsort[j][k];
			qsort(pList[j],0,nList[j]-1);
		}
		nList[3] = intersect_lists(pList[0],nList[0], pList[1],nList[1], pList[3]);
		nList[0] = intersect_lists(pList[3],nList[3], pList[2],nList[2], pList[0]);
		for(k=0;k<nList[0];k++)
			pVtx[pList[0][k]].id = i;
	}
	for(i=0;i<4;i++) delete[] pList[i];
	// move degenerate triangles to the end of the list
	for(i=0,j=m_nTris; i<j;) 
	if (iszero(pVtx[m_pIndices[i*3]].id^pVtx[m_pIndices[i*3+1]].id) | iszero(pVtx[m_pIndices[i*3+1]].id^pVtx[m_pIndices[i*3+2]].id) |
			iszero(pVtx[m_pIndices[i*3+2]].id^pVtx[m_pIndices[i*3]].id)) 
	{
		j--;
		for(k=0;k<3;k++) { int itmp=m_pIndices[j*3+k]; m_pIndices[j*3+k]=m_pIndices[i*3+k]; m_pIndices[i*3+k]=itmp; }
		if (m_pIds) { short stmp=m_pIds[j]; m_pIds[j]=m_pIds[i]; m_pIds[i]=stmp; }
	}	else i++;
	m_nTris = i;

	box bbox;
	vectorr axes[3],center;
	if (flags & (mesh_AABB | mesh_SingleBB)) {
		index_t *pHullTris;
		int nHullTris;
		if (m_nTris<=20 || !(nHullTris = qhull(m_pVertices,m_nVertices, pHullTris))) {
			nHullTris=m_nTris; pHullTris=m_pIndices;
		}
		ComputeMeshEigenBasis(m_pVertices,pHullTris,nHullTris, axes,center);
		if (pHullTris!=m_pIndices)
			delete[] pHullTris;
	}

	if (flags & mesh_SingleBB) {
		CSingleBoxTree *pTree = new CSingleBoxTree;
		bbox.Basis = (matrix3x3RM&)axes[0];
		vectorf pt,ptmin(MAX),ptmax(MIN);
		for(i=0;i<m_nVertices;i++) {
			pt = bbox.Basis*m_pVertices[i];
			ptmin.x=min_safe(ptmin.x,pt.x); ptmin.y=min_safe(ptmin.y,pt.y); ptmin.z=min_safe(ptmin.z,pt.z);
			ptmax.x=max_safe(ptmax.x,pt.x); ptmax.y=max_safe(ptmax.y,pt.y); ptmax.z=max_safe(ptmax.z,pt.z);
		}
		bbox.center = ((ptmin+ptmax)*bbox.Basis)*0.5f;
		bbox.bOriented = 1;
		bbox.size = (ptmax-ptmin)*0.5f;
		float mindim = max(max(bbox.size.x,bbox.size.y),bbox.size.z)*0.001f;
		bbox.size.x=max(bbox.size.x,mindim); bbox.size.y=max(bbox.size.y,mindim);	bbox.size.z=max(bbox.size.z,mindim);
		pTree->SetBox(&bbox);
		pTree->Build(this);
		pTree->m_nPrims = m_nTris;
		m_pTree = pTree;
	} else {
		CBVTree *pTrees[3];
		index_t *pTreeIndices[3];
		short *pTreeIds[3];
		float volumes[3],skipdim;
		int nTrees=0,iTreeBest;
		skipdim = flags & mesh_multicontact2 ? 0.0f : (flags & mesh_multicontact0 ? 10.0f : 0.1f);

		if (flags & mesh_AABB) {
			matrix3x3f Basis; Basis.SetIdentity();
			CAABBTree *pTree = new CAABBTree;
			pTree->SetParams(nMinTrisPerNode,nMaxTrisPerNode,skipdim,Basis);
			volumes[nTrees++] = (pTrees[nTrees]=pTree)->Build(this);

			memcpy(pTreeIndices[nTrees-1] = new index_t[m_nTris*3], m_pIndices, sizeof(m_pIndices[0])*m_nTris*3);
			if (m_pIds) memcpy(pTreeIds[nTrees-1] = new short[m_nTris], m_pIds, sizeof(m_pIds[0])*m_nTris);
			pTree = new CAABBTree;
			Basis = (matrix3x3RM&)axes[0];
			pTree->SetParams(nMinTrisPerNode,nMaxTrisPerNode,skipdim,Basis);
			volumes[nTrees++] = (pTrees[nTrees]=pTree)->Build(this)*1.01f; // favor non-oriented AABBs slightly
		} 
		if (flags & mesh_OBB) {
			if (nTrees>0) {
				memcpy(pTreeIndices[nTrees-1] = new index_t[m_nTris*3], m_pIndices, sizeof(m_pIndices[0])*m_nTris*3);
				if (m_pIds) memcpy(pTreeIds[nTrees-1] = new short[m_nTris], m_pIds, sizeof(m_pIds[0])*m_nTris);
			}
			COBBTree *pTree = new COBBTree;
			pTree->SetParams(nMinTrisPerNode,nMaxTrisPerNode,skipdim);
			volumes[nTrees++] = (pTrees[nTrees]=pTree)->Build(this)*favorAABB;
		}
		for(iTreeBest=0,i=1;i<nTrees;i++) if (volumes[i]<volumes[iTreeBest])
			iTreeBest = i;
		m_pTree = pTrees[iTreeBest];
		if (iTreeBest!=nTrees-1) {
			memcpy(m_pIndices, pTreeIndices[iTreeBest], sizeof(m_pIndices[0])*m_nTris*3);
			if (m_pIds) memcpy(m_pIds, pTreeIds[iTreeBest], sizeof(m_pIds[0])*m_nTris);
		}
		for(i=0;i<nTrees;i++) if (i!=iTreeBest)
			delete pTrees[i];
		for(i=0;i<nTrees-1;i++) { 
			delete[] pTreeIndices[i];
			if (m_pIds) delete[] pTreeIds[i];
		}
	}
	m_flags &= ~(mesh_OBB | mesh_AABB | mesh_SingleBB);
	switch (m_pTree->GetType()) {
		case BVT_AABB: m_flags |= mesh_AABB; break;
		case BVT_OBB:	m_flags |= mesh_OBB; break;
		case BVT_SINGLEBOX:	m_flags |= mesh_SingleBB; break;
	}

	pIndices = new index_t[m_nTris*3]; // BVtree may have sorted indices, so calc normals and topology after the tree
	for(i=0;i<m_nTris*3;i++) pIndices[i] = pVtx[m_pIndices[i]].id;
	delete[] pVtx;

	for(i=0;i<m_nTris;i++) // precalculate normals
		m_pNormals[i] = (m_pVertices[pIndices[i*3+1]]-m_pVertices[pIndices[i*3]] ^ 
			m_pVertices[pIndices[i*3+2]]-m_pVertices[pIndices[i*3]]).normalized();

	// fill topology information - find 3 edge neighbours for every triangle
	m_pTopology = new trinfo[m_nTris];
	for(i=0;i<m_nTris;i++)
		m_pTopology[i].ibuddy[0]=m_pTopology[i].ibuddy[1]=m_pTopology[i].ibuddy[2] = -1;

	for(i=0;i<3;i++) {
		for(j=0;j<m_nTris;j++) vsort[i][j]=j;
		qsort(vsort[i],strided_pointer<index_t>(pIndices+i,sizeof(index_t)*3), 0,m_nTris-1);
	}

	for(i=0;i<m_nTris;i++) for(j=0;j<3;j++) {
		for(k=0;k<3;k++) {
			ibuddy = bin_search(vsort[k],pIndices+k,m_nTris, pIndices[i*3+j]); // find triangle that has its k.th vertex same as j.th of this one
			for(; ibuddy>0 && pIndices[vsort[k][ibuddy-1]*3+k]==pIndices[i*3+j]; ibuddy--); // find first such triangle if there are several
			for(; ibuddy<m_nTris && pIndices[vsort[k][ibuddy]*3+k]==pIndices[i*3+j]; ibuddy++)
			if (vsort[k][ibuddy]!=i && pIndices[i*3+inc_mod3[j]]==pIndices[vsort[k][ibuddy]*3+dec_mod3[k]]) { 
				m_pTopology[i].ibuddy[j] = vsort[k][ibuddy]; goto nextedge;	// find triangle that shares this edge with the current one
			}
		}	nextedge:;
	}
	// detect if we have unconnected regions (floodfill starting from the 1st tri)
	if (m_nTris>0) {
		for(i=0;i<m_nTris;i++) vsort[0][i]=0;
		vsort[0][pIndices[0]=0] = 1;
		for(i=0,j=1; i<j; i++) {
			for(k=0;k<3;k++) if (m_pTopology[pIndices[i]].ibuddy[k]>=0 && vsort[0][m_pTopology[pIndices[i]].ibuddy[k]]==0)
				if (j<m_nTris)
					vsort[0][pIndices[j++] = m_pTopology[pIndices[i]].ibuddy[k]] = 1;
				else 
					break; // guard against broken topology
		}
		m_bMultipart = isneg(i-m_nTris);
	}
	for(i=0;i<3;i++) delete[] vsort[i];
	delete[] pIndices;
	if (m_bIsConvex = IsConvex(0.02f))
		m_pTree->SetGeomConvex();

	m_pTree->GetBBox(&bbox);
	m_minVtxDist = max(max(bbox.size.x,bbox.size.y),bbox.size.z)*1E-3f;
	return this;
}


int CTriMesh::IsConvex(float tolerance)
{
	int idx,i,j;
	for(idx=0; idx<sizeof(m_bConvex)/sizeof(m_bConvex[0])-1 && m_ConvexityTolerance[idx]>=0 && m_ConvexityTolerance[idx]!=tolerance; idx++);
	if (m_ConvexityTolerance[idx]==tolerance)	{
		m_ConvexityTolerance[idx]=m_ConvexityTolerance[0]; m_ConvexityTolerance[0]=tolerance;
		i=m_bConvex[idx]; m_bConvex[idx]=m_bConvex[0]; m_bConvex[0]=i;
		return i;
	}

	if (m_bMultipart)
		return m_bConvex[idx]=0;

	m_ConvexityTolerance[idx] = tolerance;
	tolerance = sqr(tolerance);
	for(i=0;i<m_nTris;i++) for(j=0;j<3;j++) if (m_pTopology[i].ibuddy[j]>=0) {
		vectorf cross = m_pNormals[i]^m_pNormals[m_pTopology[i].ibuddy[j]];
		if (cross.len2()>tolerance && cross*(m_pVertices[m_pIndices[i*3+inc_mod3[j]]]-m_pVertices[m_pIndices[i*3+j]])<0)
			return m_bConvex[idx]=0;
	} else
		return m_bConvex[idx]=0;
	return m_bConvex[idx]=1;
}


int CTriMesh::CalcPhysicalProperties(phys_geometry *pgeom)
{
	pgeom->pGeom = this;

	vectorr center,Ibody;
	matrix3x3 I; matrix3x3RM Irm,Rframe2body;
	pgeom->V = ComputeMassProperties(m_pVertices,m_pIndices,m_nTris, center,I);

	matrix eigenBasis(3,3,0,Rframe2body.GetData());
	matrix(3,3,mtx_symmetric,(Irm=I).GetData()).jacobi_transformation(eigenBasis, Ibody);
	pgeom->origin = center; 
	pgeom->Ibody = Ibody;
	pgeom->q = quaternionf(Rframe2body.T());

	return 1;
}


int CTriMesh::PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts)
{
	pGTest->pGeometry = this;
	pGTest->pBVtree = m_pTree;
	m_pTree->PrepareForIntersectionTest(pGTest);

	pGTest->typeprim = indexed_triangle::type;
	int nNodeTris = m_pTree->MaxPrimsInNode();
	if (nNodeTris<=(int)(sizeof(g_IdxTriBuf)/sizeof(g_IdxTriBuf[0]))-g_IdxTriBufPos) {
		pGTest->primbuf = g_IdxTriBuf+g_IdxTriBufPos; g_IdxTriBufPos+=nNodeTris;
		pGTest->szprimbuf = nNodeTris;
	} else 
		pGTest->primbuf = new indexed_triangle[nNodeTris];
	if (m_nMaxVertexValency<=(int)(sizeof(g_IdxTriBuf)/sizeof(g_IdxTriBuf[0]))-g_IdxTriBufPos) {
		pGTest->primbuf1 = g_IdxTriBuf+g_IdxTriBufPos; g_IdxTriBufPos+=m_nMaxVertexValency;
		pGTest->szprimbuf1 = m_nMaxVertexValency;
	} else 
		pGTest->primbuf1 = new indexed_triangle[m_nMaxVertexValency];
	if (m_nMaxVertexValency<=(int)(sizeof(g_iFeatureBuf)/sizeof(g_iFeatureBuf[0]))-g_iFeatureBufPos) {
		pGTest->iFeature_buf = g_iFeatureBuf+g_iFeatureBufPos; g_iFeatureBufPos+=m_nMaxVertexValency;
	} else 
		pGTest->iFeature_buf = new int[m_nMaxVertexValency];
	int szbuf = max(nNodeTris,m_nMaxVertexValency);
	if (szbuf<=(int)(sizeof(g_IdBuf)/sizeof(g_IdBuf[0]))-g_IdBufPos) {
		pGTest->idbuf = g_IdBuf+g_IdBufPos; g_IdBufPos+=szbuf;
	} else 
		pGTest->idbuf = new short[szbuf];
	pGTest->szprim = sizeof(indexed_triangle);

	if (m_nMaxVertexValency<=(int)(sizeof(g_SurfaceDescBuf)/sizeof(g_SurfaceDescBuf[0]))-g_SurfaceDescBufPos) {
		pGTest->surfaces = g_SurfaceDescBuf+g_SurfaceDescBufPos; g_SurfaceDescBufPos+=m_nMaxVertexValency;
	} else 
		pGTest->surfaces = new surface_desc[m_nMaxVertexValency];
	if (m_nMaxVertexValency<=(int)(sizeof(g_EdgeDescBuf)/sizeof(g_EdgeDescBuf[0]))-g_EdgeDescBufPos) {
		pGTest->edges = g_EdgeDescBuf+g_EdgeDescBufPos; g_EdgeDescBufPos+=m_nMaxVertexValency;
	} else 
		pGTest->edges = new edge_desc[m_nMaxVertexValency];

	pGTest->minAreaEdge = 0;
	if (!bKeepPrevContacts)
		g_BrdPtBufPos = g_BrdPtBufStart = 0;
	g_BrdPtBufStart = g_BrdPtBufPos;

	return 1;
}

void CTriMesh::CleanupAfterIntersectionTest(geometry_under_test *pGTest)
{
	m_pTree->CleanupAfterIntersectionTest(pGTest);
	if ((unsigned int)((indexed_triangle*)pGTest->primbuf-g_IdxTriBuf) > (unsigned int)(sizeof(g_IdxTriBuf)/sizeof(g_IdxTriBuf[0])))
		delete[] pGTest->primbuf;
	if ((unsigned int)((indexed_triangle*)pGTest->primbuf1-g_IdxTriBuf) > (unsigned int)(sizeof(g_IdxTriBuf)/sizeof(g_IdxTriBuf[0])))
		delete[] pGTest->primbuf1;
	if ((unsigned int)(pGTest->iFeature_buf-g_iFeatureBuf) > (unsigned int)(sizeof(g_iFeatureBuf)/sizeof(g_iFeatureBuf[0])))
		delete[] pGTest->iFeature_buf;
	if ((unsigned int)(pGTest->idbuf-g_IdBuf) > (unsigned int)(sizeof(g_IdBuf)/sizeof(g_IdBuf[0])))
		delete[] pGTest->idbuf;
	if ((unsigned int)(pGTest->surfaces-g_SurfaceDescBuf) > (unsigned int)(sizeof(g_SurfaceDescBuf)/sizeof(g_SurfaceDescBuf[0])))
		delete[] pGTest->surfaces;
	if ((unsigned int)(pGTest->edges-g_EdgeDescBuf) > (unsigned int)(sizeof(g_EdgeDescBuf)/sizeof(g_EdgeDescBuf[0])))
		delete[] pGTest->edges;
}


int CTriMesh::GetFeature(int iPrim,int iFeature, vectorf *pt)
{ 
	int npt=0;
	switch (iFeature & 0x60) {
		case 0x40: pt[2]=m_pVertices[m_pIndices[iPrim*3+dec_mod3[(iFeature&0x1F)]]]; npt++;
		case 0x20: pt[1]=m_pVertices[m_pIndices[iPrim*3+inc_mod3[(iFeature&0x1F)]]]; npt++;
		case 0   : pt[0]=m_pVertices[m_pIndices[iPrim*3+(iFeature&0x1F)]]; npt++;
	}
	return npt;
}


void CTriMesh::PrepareTriangle(int itri,triangle *ptri, const geometry_under_test *pGTest)
{
	int idx = itri*3;
	ptri->pt[0] = pGTest->R*m_pVertices[m_pIndices[idx  ]]*pGTest->scale + pGTest->offset;
	ptri->pt[1] = pGTest->R*m_pVertices[m_pIndices[idx+1]]*pGTest->scale + pGTest->offset;
	ptri->pt[2] = pGTest->R*m_pVertices[m_pIndices[idx+2]]*pGTest->scale + pGTest->offset;
	ptri->n = pGTest->R*m_pNormals[itri];
}

int CTriMesh::PreparePrimitive(geom_world_data *pgwd, primitive *&pprim)
{
	indexed_triangle *ptri;
	pprim = ptri=g_IdxTriBuf;
	ptri->pt[0] = pgwd->R*m_pVertices[m_pIndices[0]]*pgwd->scale + pgwd->offset;
	ptri->pt[1] = pgwd->R*m_pVertices[m_pIndices[1]]*pgwd->scale + pgwd->offset;
	ptri->pt[2] = pgwd->R*m_pVertices[m_pIndices[2]]*pgwd->scale + pgwd->offset;
	ptri->n = pgwd->R*m_pNormals[0];
	return indexed_triangle::type;
}

const int MAX_LOOP = 20;

int CTriMesh::TraceTriangleInters(int iop, primitive *pprims[], int idx_buddy,int type_buddy, prim_inters *pinters, 
																	geometry_under_test *pGTest, border_trace *pborder)
{
	indexed_triangle *ptri = (indexed_triangle*)pprims[iop];
	const int MAX_LOOP = 10;
	int itri,itri0=ptri->idx,itri_end,itri_prev,itri_cur,iedge,itypes[2],iter;
	vectorf dir0,dir1;
	itypes[iop] = triangle::type;
	itypes[iop^1] = type_buddy;

	do {
		itri = m_pTopology[itri0].ibuddy[pinters->iFeature[1][iop] & 0x1F];
		if ((itri|iop<<31)==pborder->itri_end && GetEdgeByBuddy(itri,itri0)==pborder->iedge_end || (pborder->npt>3 && 
				(pinters->pt[1]-pborder->pt_end).len2()<pborder->end_dist2) || itri<0 || pborder->npt>=pborder->szbuf)
			return 2;
		PrepareTriangle(itri,ptri, pGTest);
		//m_pTree->MarkUsedTriangle(itri, pGTest);

		if (!g_Intersector.Check(itypes[0],itypes[1], pprims[0],pprims[1], pinters)) {
			// if we start from a point close to vertex, check a triangle fan around the vertex rather than just edge owner
			// return 0 if we fail to close the intersection region
			iedge = GetEdgeByBuddy(itri, itri0);
			itri_end = itri;
			if ((pinters->pt[1]-ptri->pt[iedge]).len2() < sqr(m_minVtxDist)) { // start edge vertex
				itri_end = itri0; itri_prev = itri;
				itri = m_pTopology[itri].ibuddy[dec_mod3[iedge]];
			} else if ((pinters->pt[1]-ptri->pt[inc_mod3[iedge]]).len2() < sqr(m_minVtxDist)) { // end edge
				itri_end = itri; itri_prev = itri0;
				itri = m_pTopology[itri0].ibuddy[dec_mod3[pinters->iFeature[1][iop] & 0x1F]];
			}	else
				return 0;
			if (itri<0)
				return 0;

			iter=0; do {
				PrepareTriangle(itri,ptri, pGTest);
				//m_pTree->MarkUsedTriangle(itri, pGTest);
				if (g_Intersector.Check(itypes[0],itypes[1], pprims[0],pprims[1], pinters))
					goto have_inters;
				itri_cur = itri;
				itri = m_pTopology[itri].ibuddy[dec_mod3[GetEdgeByBuddy(itri, itri_prev)]];
				itri_prev = itri_cur;
			} while(itri>=0 && itri!=itri_end && ++iter<30);

			return 0;
		}
		have_inters:
		// store information about the last intersection segment
		pborder->itri[pborder->npt][iop] = itri;
		pborder->itri[pborder->npt][iop^1] = idx_buddy;
		pborder->pt[pborder->npt] = pinters->pt[1];
		pborder->n_sum[iop] += ptri->n;
		if (fabs_tpl(pborder->n_best.z)<fabs_tpl(ptri->n.z))
			pborder->n_best = ptri->n*(1-iop*2);
		pborder->ntris[iop]++;
		// do checks against looping in degenerate cases
		if ((pborder->pt[pborder->npt]-pborder->pt[pborder->iMark]).len2() < pborder->end_dist2*0.01f)
			return 0;
		--pborder->nLoop; pborder->iMark = pborder->iMark&~(pborder->nLoop>>31) | pborder->npt&pborder->nLoop>>31;
		pborder->nLoop = pborder->nLoop-(pborder->nLoop>>31) | MAX_LOOP&pborder->nLoop>>31;
		pborder->npt++;

		ptri->idx = itri0 = itri;
	} while(!(pinters->iFeature[1][iop^1] & 0x80)); // iterate while triangle from another mesh is not ended

	return 1;
}


int CTriMesh::GetUnprojectionCandidates(int iop,const contact *pcontact, primitive *&pprim,int *&piFeature, geometry_under_test *pGTest)
{
	int i,itri,iFeature = pcontact->iFeature[iop];
	indexed_triangle *ptri = (indexed_triangle*)pGTest->primbuf1;
	intptr_t idmask = ~iszero_mask(m_pIds);
	short idnull=-1, *pIds = (short*)((intptr_t)m_pIds&idmask | (intptr_t)&idnull&~idmask);
	itri = ((indexed_triangle*)pprim)->idx;
	PrepareTriangle(itri,ptri,pGTest); ptri->idx = itri;
	pprim = ptri;
	piFeature = pGTest->iFeature_buf;
	pGTest->bTransformUpdated = 0;

	if ((iFeature & 0x60)!=0) { // if feature is not vertex, but is very close to one, change it to vertex
		for(i=0;i<3 && (ptri->pt[i]-pcontact->pt).len2()>sqr(m_minVtxDist);i++);
		if (i<3) 
			iFeature = 0x80 | i;
	}

	switch (iFeature & 0x60) {
		case 0x00: { // vertex - return triangle fan around it (except the previous contacting triangle)
				int itri_prev,itri0,iedge,ntris=0;
				itri_prev = itri0 = itri;
				
				pGTest->surfaces[0].n = ptri->n;
				pGTest->surfaces[0].idx = itri;
				pGTest->surfaces[0].iFeature = 0x40;

				iedge = iFeature & 0x1F;
				pGTest->edges[0].dir = ptri->pt[inc_mod3[iedge]]-ptri->pt[iedge];
				pGTest->edges[0].n[0] = ptri->n ^ pGTest->edges[0].dir;
				pGTest->edges[0].idx = itri;
				pGTest->edges[0].iFeature = 0x20 | iedge;

				itri = m_pTopology[itri].ibuddy[dec_mod3[iFeature & 0x1F]];
				if (itri>=0) do {
					PrepareTriangle(itri,ptri+ntris,pGTest);
					ptri[ntris].idx = itri;
					pGTest->idbuf[ntris] = pIds[itri&idmask];

					iedge = GetEdgeByBuddy(itri,itri_prev);
					piFeature[ntris] = 0x80 | iedge;
					pGTest->edges[ntris+1].dir = ptri[ntris].pt[inc_mod3[iedge]]-ptri[ntris].pt[iedge];
					pGTest->edges[ntris+1].n[0] = pGTest->edges[ntris+1].dir ^ pGTest->surfaces[ntris].n;
					pGTest->edges[ntris+1].n[1] = ptri[ntris].n ^ pGTest->edges[ntris+1].dir;
					pGTest->edges[ntris+1].idx = itri;
					pGTest->edges[ntris+1].iFeature = 0x20 | iedge;

					pGTest->surfaces[ntris+1].n = ptri[ntris].n;
					pGTest->surfaces[ntris+1].idx = itri;
					pGTest->surfaces[ntris+1].iFeature = 0x40;

					itri_prev = itri;
					itri = m_pTopology[itri].ibuddy[dec_mod3[iedge]];
					ntris++;
				} while(itri>=0 && itri!=itri0 && ntris<pGTest->szprimbuf1-1);

				pGTest->edges[0].n[1] = pGTest->edges[0].dir ^ ptri[ntris-1].n;
				pGTest->nSurfaces = ntris+1;
				pGTest->nEdges = ntris+1;
				pprim = ptri;
				return ntris;
			}

		case 0x20: // edge - switch to the corresponding buddy
			pGTest->edges[0].dir = ptri->pt[inc_mod3[iFeature & 0x1F]] - ptri->pt[iFeature & 0x1F];
			pGTest->edges[0].n[0] = ptri->n^pGTest->edges[0].dir;
			pGTest->edges[0].idx = itri;
			pGTest->edges[0].iFeature = iFeature;
			pGTest->nEdges = 1;

			pGTest->surfaces[0].n = ptri->n;
			pGTest->surfaces[0].idx = itri;
			pGTest->surfaces[0].iFeature = 0x40;

			piFeature[0] = GetEdgeByBuddy(m_pTopology[itri].ibuddy[iFeature & 0x1F], itri) | 0xA0;
			itri = m_pTopology[itri].ibuddy[iFeature & 0x1F];
			pGTest->nSurfaces = 1;
			if (itri<0) {
				pGTest->edges[0].n[1] = pGTest->edges[0].n[0];
				return 0;
			}

			PrepareTriangle(itri,ptri,pGTest);
			pGTest->idbuf[0] = pIds[itri&idmask];
			ptri->idx = itri;

			pGTest->edges[0].n[1] = pGTest->edges[0].dir^ptri->n;
			pGTest->surfaces[1].n = ptri->n;
			pGTest->surfaces[1].idx = itri;
			pGTest->surfaces[1].iFeature = 0x40;
			pGTest->nSurfaces = 2;
			return 1;

		case 0x40: // face - keep the previous contacting triangle
			pGTest->surfaces[0].n = ptri->n;
			pGTest->surfaces[0].idx = itri;
			pGTest->surfaces[0].iFeature = 0x40;
			pGTest->nSurfaces = 1;
			pGTest->nEdges = 0;
			piFeature[0] = 0x40;
			return 1;
	}
	return 0;
}


int CTriMesh::GetPrimitiveList(int iStart,int nPrims, int typeCollider,primitive *pCollider,int bColliderLocal, 
															 geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,short *pResId)
{
	intptr_t mask;
	int i,j,iEnd=min(m_nTris,iStart+nPrims),bContact,bPivotCulling;
	short *pIds,noid=-2;
	mask = ~iszero_mask(m_pIds);
	pIds = (short*)((intptr_t)m_pIds&mask | (intptr_t)&noid&~mask);
	bPivotCulling = isneg(pGTestOp->ptOutsidePivot.x-1E10);

	if (bColliderLocal==1) // collider is in local coordinate frame
	for(i=iStart,j=0; i<iEnd; i++,j+=bContact) {
		indexed_triangle *ptri = (indexed_triangle*)pRes+j;
		ptri->pt[0] = m_pVertices[m_pIndices[i*3+0]]; ptri->pt[1] = m_pVertices[m_pIndices[i*3+1]];
		ptri->pt[2] = m_pVertices[m_pIndices[i*3+2]]; ptri->n = m_pNormals[i];		
		pResId[j] = pIds[i&mask]; // material -1 signals that the triangle should be skipped
		bContact = g_Overlapper.Check(typeCollider,triangle::type, pCollider,ptri) & (iszero(pResId[j]+1)^1); 
		PrepareTriangle(i,ptri, pGTest); 
		if (bPivotCulling)
			bContact &= isneg((ptri->pt[0]-pGTestOp->ptOutsidePivot)*ptri->n);
		ptri->idx = i;
	}	else if (bColliderLocal==0)	// collider is in his own coordinate frame
	for(i=iStart,j=0; i<iEnd; i++,j+=bContact) {
		indexed_triangle *ptri = (indexed_triangle*)pRes+j;
		ptri->pt[0] = pGTest->R_rel*m_pVertices[m_pIndices[i*3+0]]*pGTest->scale_rel + pGTest->offset_rel;
		ptri->pt[1] = pGTest->R_rel*m_pVertices[m_pIndices[i*3+1]]*pGTest->scale_rel + pGTest->offset_rel;
		ptri->pt[2] = pGTest->R_rel*m_pVertices[m_pIndices[i*3+2]]*pGTest->scale_rel + pGTest->offset_rel;	
		ptri->n = pGTest->R_rel*m_pNormals[i];
		pResId[j] = pIds[i&mask];
		bContact = g_Overlapper.Check(typeCollider,triangle::type, pCollider,ptri) & (iszero(pResId[j]+1)^1);
		PrepareTriangle(i,ptri, pGTest); 
		if (bPivotCulling)
			bContact &= isneg((ptri->pt[0]-pGTestOp->ptOutsidePivot)*ptri->n);
		ptri->idx = i;
	}	else // no checks for collider, get all tris unconditionally (used in sweep check)
	for(i=iStart,j=0; i<iEnd; i++,j++) {
		indexed_triangle *ptri = (indexed_triangle*)pRes+j;
		PrepareTriangle(i,ptri, pGTest); 
		ptri->idx = i;
	}
	pGTest->bTransformUpdated = 0;
 	return j;
}


void update_unprojection(real t, unprojection_mode *pmode, geometry_under_test *pGTest)
{
	if (pmode->imode==0) 
		pGTest->offset = pmode->offset0 + pmode->dir*t;
	else {
		matrix3x3f Rw;
		real cosa=cos_tpl(t), sina=sin_tpl(t);
		Rw.SetRotationAA(cosa,sina, pmode->dir);
		(pGTest->R = Rw) *= pmode->R0;
		pGTest->offset = Rw*(pmode->offset0-pmode->center) + pmode->center;
	}
	pGTest->bTransformUpdated = 1;
}


int CTriMesh::RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2, 
																	 prim_inters *pinters)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_PHYSICS );

	geometry_under_test *pGTest[2] = { pGTest1,pGTest2 };
	indexed_triangle tri = *(indexed_triangle*)pprim1, tri1;
	primitive *pprims[2] = { &tri, pprim2 }, *ptr[2];
	int iFeature_dummy = -1, *piFeature[2] = { &iFeature_dummy,&iFeature_dummy };
	int idx_prim[2] = { ((indexed_triangle*)pprim1)->idx, -1 };
	int bNoUnprojection=0,bSurfaceSurfaceContact,bSurfaceEdgeContact,bUseLSNormal=0;
	int i,j,res,ipt,ibest,jbest,iop,nprims1,nprims2,nSmallSteps;
	int indexed_triangle::*pidxoffs=0;
	float len,maxlen;
	real t,tmax;
	border_trace border;
	geom_contact *pres = pGTest[0]->contacts + *pGTest[0]->pnContacts;
	vectorr n_avg;
	vectorf unprojPlaneNorm;
	if (pGTest[1]->typeprim==indexed_triangle::type) {
		idx_prim[1] = ((indexed_triangle*)pprims[1])->idx;
		pidxoffs = &indexed_triangle::idx;
		/*if (pGTest[0]->ptOutsidePivot.x<1E10) {
			vectorf inters_dir = (pinters->pt[0]+pinters->pt[1])*0.5f-pGTest[0]->ptOutsidePivot;
			if (sqr_signed(inters_dir*((indexed_triangle*)pprims[1])->n) > inters_dir.len2()*sqr(0.4f))
				return 0;
		}*/
	}
	pres->iPrim[0] = ((indexed_triangle*)pprims[0])->idx;
	pres->iPrim[1] = ((indexed_triangle*)pprims[1])->*pidxoffs;
	pres->bBorderConsecutive = true;

	float maxcos=1.0f,mincos2=0;
	if (pres->parea) {
		if (pGTest2->pParams->maxSurfaceGapAngle<0.2f) {
			maxcos = 1.0f-sqr(pGTest2->pParams->maxSurfaceGapAngle)*2;
			mincos2 = sqr(pGTest2->pParams->maxSurfaceGapAngle);
		} else {
			maxcos = cos_tpl(pGTest2->pParams->maxSurfaceGapAngle);
			mincos2 = sqr(cos_tpl(pi*0.5f-pGTest2->pParams->maxSurfaceGapAngle));
		}
	}
	pres->id[0] = pinters->id[0];
	pres->id[1] = pinters->id[1];
	pres->iNode[0] = pinters->iNode[0];
	pres->iNode[1] = pinters->iNode[2];

	border.itri = g_BrdiTriBuf;
	border.seglen = g_BrdSeglenBuf;
	border.pt = g_BrdPtBuf + g_BrdPtBufPos;
	border.szbuf = min(100, sizeof(g_BrdPtBuf)/sizeof(g_BrdPtBuf[0]) - g_BrdPtBufPos-10);
	if (border.szbuf<0)
		return 0;
	pres->ptborder = border.pt;
	border.n_sum[0].zero(); border.n_sum[1].zero();
	border.ntris[0] = border.ntris[1] = 0;
	border.n_best.zero();

	iop = pinters->iFeature[0][1]>>7; // index of mesh whose primitive has boundary on intersection start
	border.pt_end = pinters->pt[0];
	border.itri_end = ((indexed_triangle*)pprims[iop])->idx | iop<<31;
	border.iedge_end = pinters->iFeature[0][iop] & 0x1F;
	border.end_dist2 = sqr(m_minVtxDist*10);

	for(i=g_BrdPtBufStart;i<g_BrdPtBufPos && (g_BrdPtBuf[i]-pinters->pt[1]).len2()>border.end_dist2;i++);
	if (i<g_BrdPtBufPos)
		return 0;

	pres->center = border.pt[0] = pinters->pt[0];
	border.pt[1] = pinters->pt[1];
	border.itri[0][iop] = border.itri[1][iop] = idx_prim[iop];
	border.itri[0][iop^1] = border.itri[1][iop^1] = idx_prim[iop^1];
	border.npt = 2;
	border.iMark = 1;
	border.nLoop = MAX_LOOP;
	iop = pinters->iFeature[1][1]>>7; // index of mesh whose primitive has boundary on intersection end
	if (pGTest[iop^1]->typeprim==indexed_triangle::type) {
		border.n_sum[iop^1] = ((indexed_triangle*)pprims[iop^1])->n;
		border.n_best = border.n_sum[iop^1]*(iop*2-1);
		border.ntris[iop^1] = 1;
	}

	if ((pinters->iFeature[1][0] | pinters->iFeature[1][1]) & 0x80
			&& !pGTest[0]->pParams->bNoBorder) { // if at least one triangle ended
			// && (!pGTest1->bStopAtFirstTri || (pinters->pt[0]-pinters->pt[1]).len2()<sqr(m_minVtxDist*10))) { 
		do {
			if (iop==1 && pprims[1]==pprim2) { // since we are about to change triangle being traced, use a local copy
				tri1 = *(indexed_triangle*)pprim2; pprims[1] = &tri1;
			}
			border.n_sum[iop] += ((indexed_triangle*)pprims[iop])->n; border.ntris[iop]++;
			if (fabs_tpl(border.n_best.z) < fabs_tpl(((indexed_triangle*)pprims[iop])->n.z))
				border.n_best = ((indexed_triangle*)pprims[iop])->n*(1-iop*2);
			res = ((CTriMesh*)pGTest[iop]->pGeometry)->TraceTriangleInters(iop, pprims, idx_prim[iop^1],pGTest[iop^1]->typeprim, 
				pinters, pGTest[iop], &border);
			pGTest1->pParams->bBothConvex &= res>>1; // if failed to close a contour - don't use optiomizations for convex objects
			idx_prim[iop] = ((indexed_triangle*)pprims[iop])->idx; iop ^= 1;
		} while(res==1);

		if (res==2) for(i=1;i<border.npt;i++) {
			pGTest[0]->pBVtree->MarkUsedTriangle(border.itri[i][0],pGTest[0]);
			pGTest[1]->pBVtree->MarkUsedTriangle(border.itri[i][1],pGTest[1]);
		}
	} else {
		border.ntris[0]=1; border.n_sum[0]=tri.n;
	};
	pres->nborderpt = border.npt;
	g_BrdPtBufPos += border.npt;
	border.n_sum[1].Flip();
	n_avg = (border.ntris[0]<border.ntris[1] && border.ntris[0] || !border.ntris[1] ? 
		border.n_sum[0]:border.n_sum[1]).normalized();
	if (fabs_tpl(border.n_best.z) < 0.95f)
		border.n_best = n_avg;

	unprojection_mode unproj;
	contact contact_cur, contact_best;

	vectorf vrel_loc, vrel=pGTest[0]->v-pGTest[1]->v, pthit=border.pt[0], pthit1=border.pt[0];
	float vrel_loc2,vrel2=0,dist,maxdist=0,maxdist1=0;
	for(i=0;i<border.npt;i++) {
		vrel_loc = pGTest[0]->v-pGTest[1]->v + (pGTest[0]->w^border.pt[i]-pGTest[0]->centerOfMass) - 
			(pGTest[1]->w^border.pt[i]-pGTest[1]->centerOfMass);
		if (vrel_loc*n_avg>0 && (vrel_loc2=vrel_loc.len2()) > vrel2) {
			vrel2=vrel_loc2; vrel=vrel_loc; 
		}
		if ((dist=(border.pt[i]-pGTest[0]->centerOfRotation^pGTest[0]->pParams->axisOfRotation).len2())>maxdist) {
			maxdist = dist; pthit = border.pt[i];
		}
		if ((dist=(border.pt[i]-pGTest[0]->centerOfRotation).len2())>maxdist1) {
			maxdist1 = dist; pthit1 = border.pt[i];
		}
	}

	if (pGTest[0]->pParams->iUnprojectionMode==0) {
		unproj.imode = 0;
		unproj.dir = -vrel;
		unproj.vel = unproj.dir.len();
		if (unproj.vel < pGTest[0]->pParams->vrel_min) {
			unproj.R0 = pGTest[0]->R; unproj.offset0 = pGTest[0]->offset;
			bUseLSNormal=1; goto end_unproj;
		}
		unproj.dir /= unproj.vel;
		tmax = unproj.tmax = pGTest[0]->pParams->time_interval*unproj.vel;
	} else {
		if (pGTest[0]->pParams->axisOfRotation.len2()==0) {
			maxdist = maxdist1; pthit = pthit1;
		}
		if (maxdist < sqr(pGTest[0]->pParams->minAxisDist)) {
			unproj.R0 = pGTest[0]->R; unproj.offset0 = pGTest[0]->offset;
			bUseLSNormal=1; goto end_unproj;
		}
		unproj.imode = 1;
		unproj.center = pGTest[0]->centerOfRotation;
		unproj.dir = pGTest[0]->pParams->axisOfRotation;
		if (unproj.dir.len2()>0)
			unproj.dir *= -sgnnz(n_avg*(unproj.dir^pthit-unproj.center));
		else {
			//unproj.dir = (pthit-unproj.center^vrel).normalized();
			//unproj.dir *= -sgnnz(n_avg*(unproj.dir^pthit-unproj.center));
			unproj.dir = (n_avg^pthit-unproj.center).normalized();
			pGTest[0]->pParams->axisOfRotation = unproj.dir;
		}
		unproj.vel = unproj.dir.len();
		unproj.dir /= unproj.vel;
		tmax = pGTest[0]->pParams->time_interval*unproj.vel;
		unproj.tmax = sin_tpl(tmax);
	}
	unproj.minPtDist = min(m_minVtxDist,pGTest2->pGeometry->m_minVtxDist);
	unprojPlaneNorm = pGTest[0]->pParams->unprojectionPlaneNormal;

	do {
		unproj.R0 = pGTest[0]->R;
		unproj.offset0 = pGTest[0]->offset;
		bSurfaceSurfaceContact = 0;
		bSurfaceEdgeContact = 0;
		if (unproj.imode==0 && unprojPlaneNorm.len2()>0 && fabs_tpl(unproj.dir*unprojPlaneNorm)<0.707f) 
			(unproj.dir -= unprojPlaneNorm*(unproj.dir*unprojPlaneNorm)).normalize();	// do not restrict direction to requested plane if angle>pi/4

		// select the intersection segment with maximum length as unprojection starting point
		ibest = border.npt-1;
		maxlen = (border.pt[0]-border.pt[border.npt-1]).len2();
		for(i=0;i<border.npt-1;i++) if ((len=(border.pt[i]-border.pt[i+1]).len2()) > maxlen) {
			maxlen=len; ibest=i;
		}

		PrepareTriangle(border.itri[ibest][0],(triangle*)pprims[0],pGTest[0]);
		((indexed_triangle*)pprims[0])->idx = border.itri[ibest][0];
		if (border.itri[ibest][1]>=0) {
			((CTriMesh*)pGTest[1]->pGeometry)->PrepareTriangle(border.itri[ibest][1],(triangle*)pprims[1],pGTest[1]);
			((indexed_triangle*)pprims[1])->idx = border.itri[ibest][1];
		}

		t = 0;
		contact_best.pt = unproj.center;
		contact_best.n = pGTest[isneg(pGTest[1]->w.len2()-pGTest[0]->w.len2())]->axisContactNormal;
		contact_best.taux = 1;
		if (!g_Unprojector.Check(&unproj, pGTest[0]->typeprim,pGTest[1]->typeprim, pprims[0],-1,pprims[1],-1, &contact_best)) {
			if ((border.npt>3 || pinters->nBestPtVal || !g_Unprojector.CheckExists(unproj.imode,pGTest[0]->typeprim,pGTest[1]->typeprim)) && bUseLSNormal==0) 
				bUseLSNormal = 1; 
			else 
				bNoUnprojection = 1;
			goto end_unproj;
		}
		if (unproj.imode)
			contact_best.t = atan2(contact_best.t,contact_best.taux);

		for(nSmallSteps=0; contact_best.t>0 && nSmallSteps<3; ) {
			// modify pGTest[0] in accordance with contact_best.t
			t += contact_best.t;
			update_unprojection(t, &unproj, pGTest[0]);
			if (t>unproj.tmax) {
				bNoUnprojection = 1; goto end_unproj;
			}

			pres->iPrim[0] = ((indexed_triangle*)pprims[0])->idx;
			pres->iPrim[1] = ((indexed_triangle*)pprims[1])->*pidxoffs;
			nprims1 = pGTest[0]->pGeometry->GetUnprojectionCandidates(0,&contact_best, pprims[0],piFeature[0], pGTest[0]);
			nprims2 = pGTest[1]->pGeometry->GetUnprojectionCandidates(1,&contact_best, pprims[1],piFeature[1], pGTest[1]);

			ibest=jbest=0; contact_best.t=0;
			for(i=0,ptr[0]=pprims[0]; i<nprims1; i++,ptr[0]=(primitive*)((char*)ptr[0]+pGTest[0]->szprim)) 
				for(j=0,ptr[1]=pprims[1]; j<nprims2; j++,ptr[1]=(primitive*)((char*)ptr[1]+pGTest[1]->szprim))
					if (g_Unprojector.Check(&unproj, pGTest[0]->typeprim,pGTest[1]->typeprim, ptr[0],piFeature[0][i],ptr[1],piFeature[1][j], &contact_cur) && 
							contact_cur.t>contact_best.t) 
					{	
						ibest=i; jbest=j; contact_best=contact_cur;	
						pres->id[0]=pGTest[0]->idbuf[i]; pres->id[1]=pGTest[1]->idbuf[j];
					}
			pprims[0] = (primitive*)((char*)pprims[0] + pGTest[0]->szprim*ibest);
			pprims[1] = (primitive*)((char*)pprims[1] + pGTest[1]->szprim*jbest);
			
			if (unproj.imode) {
				contact_best.t = atan2(contact_best.t,contact_best.taux);
				i = isneg(contact_best.t-(real)1E-3);
			} else
				i = isneg(contact_best.t-m_minVtxDist);
			nSmallSteps = (nSmallSteps & -i)+i;
		}

		// check if we have surface-surface or edge-surface contact (in the latter case test if both edge buddies are outside the surface)
		if (pres->parea) {
			for(i=0;i<pGTest[0]->nSurfaces;i++) for(j=0;j<pGTest[1]->nSurfaces;j++)
				if (pGTest[0]->surfaces[i].n*pGTest[1]->surfaces[j].n < -maxcos)
				{ bSurfaceSurfaceContact=1; goto end_unproj; } // surface - surface contact
			for(iop=0;iop<2;iop++) for(i=0;i<pGTest[iop]->nSurfaces;i++) for(j=0;j<pGTest[iop^1]->nEdges;j++)
				if (sqr(pGTest[iop]->surfaces[i].n*pGTest[iop^1]->edges[j].dir) < mincos2*pGTest[iop^1]->edges[j].dir.len2() &&
						sqr_signed(pGTest[iop]->surfaces[i].n*pGTest[iop^1]->edges[j].n[0]) > -mincos2*pGTest[iop^1]->edges[j].n[0].len2() && 
						sqr_signed(pGTest[iop]->surfaces[i].n*pGTest[iop^1]->edges[j].n[1]) > -mincos2*pGTest[iop^1]->edges[j].n[1].len2())
				{ bSurfaceEdgeContact=1; goto end_unproj; } // surface - edge contact
		}
		end_unproj:

		if (!bUseLSNormal && unproj.imode==1 && (contact_best.pt-unproj.center ^ unproj.dir).len2()<sqr(pGTest[0]->pParams->minAxisDist)) {
			unproj.imode = 0;
			unproj.dir = -contact_best.n;
			unproj.vel = unproj.dir*(pGTest[1]->v+(pGTest[1]->w^contact_best.pt-pGTest[1]->centerOfMass) - 
				pGTest[0]->v-(pGTest[0]->w^contact_best.pt-pGTest[0]->centerOfMass));
			tmax = unproj.tmax = pGTest[0]->pParams->maxUnproj;
			pGTest[0]->R = unproj.R0;
			pGTest[0]->offset = unproj.offset0;
			bUseLSNormal = -1;
			bNoUnprojection = 0;
		} else if ((bUseLSNormal==1 || t>tmax) && bUseLSNormal!=-1) {	// if length is out of bounds (relative to vel)
			//   calc least squares normal for border
			//   request calculation of linear unprojection along this normal
			vectorr inters_center,p0,p1,ntest;
			matrix3x3 inters_C;
			real inters_len,det,maxdet;
			vectorf n;

			pGTest[0]->R = unproj.R0;
			pGTest[0]->offset = unproj.offset0;

			if (pinters->nBestPtVal > border.npt) {
				inters_center = pinters->ptbest;
				n = n_avg = pinters->n;
			} else if (border.npt < 3)	{ // not enough points to calc least squares normal, use average normal
				n = border.n_best; inters_center = (border.pt[0]+border.pt[1])*0.5f;
			} else {
				border.pt[border.npt++] = border.pt[0];
				for(ipt=0,inters_len=0,inters_center.zero(); ipt<border.npt-1; ipt++) {
					inters_len += border.seglen[ipt] = (border.pt[ipt+1]-border.pt[ipt]).len();
					inters_center += (border.pt[ipt+1]+border.pt[ipt])*border.seglen[ipt];
				}
				inters_center /= inters_len*2;

				for(ipt=0,inters_C.SetZero(),ntest.zero(); ipt<border.npt-1; ipt++) { // form inters_C matrix for least squares normal calculation
					p0 = border.pt[ipt]-inters_center; p1 = border.pt[ipt+1]-inters_center;
					ntest += p1+p0 ^ p1-p0;
					for(i=0;i<3;i++) for(j=0;j<3;j++)
						inters_C(i,j) += border.seglen[ipt]*(2*(p0[i]*p0[j]+p1[i]*p1[j]) + p0[i]*p1[j]+p0[j]*p1[i]);
				}
				if (ntest.len2()<sqr(m_minVtxDist))
					n = border.n_best;
				else {
					// since inter_C matrix is (it least it should be) singular, we try to set one coordinate to 1
					// and find which one provides most stable solution
					for(i=0,maxdet=0;i<3;i++) {
						det = inters_C(inc_mod3[i],inc_mod3[i])*inters_C(dec_mod3[i],dec_mod3[i]) - 
									inters_C(dec_mod3[i],inc_mod3[i])*inters_C(inc_mod3[i],dec_mod3[i]);
						if (fabs(det)>fabs(maxdet)) 
						{ maxdet=det; j=i; }
					}
					if (fabs(maxdet) < 1E-20) // normal is ill defined
						n = n_avg;
					else {
						det = 1.0/maxdet;
						n[j] = 1;
						n[inc_mod3[j]] = -(inters_C(inc_mod3[j],j)*inters_C(dec_mod3[j],dec_mod3[j]) - 
							inters_C(dec_mod3[j],j)*inters_C(inc_mod3[j],dec_mod3[j]))*det;
						n[dec_mod3[j]] = -(inters_C(inc_mod3[j],inc_mod3[j])*inters_C(dec_mod3[j],j) - 
							inters_C(dec_mod3[j],inc_mod3[j])*inters_C(inc_mod3[j],j))*det;
						n.normalize();
						if (ntest.len2()<sqr(m_minVtxDist*100))
							ntest = n_avg;
						if (n*ntest<0)
							n.Flip();
					}
				}
			}

			unproj.imode = 0;
			unproj.dir = -n;
			unproj.vel = 0;
			tmax = unproj.tmax = pGTest[0]->pParams->maxUnproj;
			bUseLSNormal = -1;
			bNoUnprojection = 0;
			unprojPlaneNorm.zero();
			pres->center = inters_center;
		} else break;
	} while (true);

	//pres->iPrim[0] = pGTest[0]->typeprim==indexed_triangle::type ? ((indexed_triangle*)pprims[0])->idx : 0;
	//pres->iPrim[1] = pGTest[1]->typeprim==indexed_triangle::type ? ((indexed_triangle*)pprims[1])->idx : 0;
	if (pGTest[1]->typeprim!=indexed_triangle::type)
		pres->iPrim[1] = 0;

	if (bNoUnprojection) {
		pres->t = 0.0001;
		pres->pt = border.pt[0];
		pres->n = n_avg;
		pres->iFeature[0] = pinters->iFeature[0][0];
		pres->iFeature[1] = pinters->iFeature[0][1];
		pres->dir = -n_avg;
		pres->vel = -1;
		pres->parea = 0;
		pres->iUnprojMode = 0;
	} else {
		pres->t = t;
		pres->pt = contact_best.pt;
		pres->iFeature[0] = contact_best.iFeature[0];
		pres->iFeature[1] = contact_best.iFeature[1];
		pres->n = contact_best.n.normalized();
		pres->dir = unproj.dir;
		pres->iUnprojMode = unproj.imode;
		pres->vel = unproj.vel;

		if (bSurfaceSurfaceContact || bSurfaceEdgeContact) {
			coord_plane surface; surface.n.zero();
			vector2df *ptbuf1,*ptbuf2,*ptbuf;
			int *idbuf1[2],*idbuf2[2],*idbuf,id,idmask,bEdgeEdge;
			int npt1,npt2,npt;
			g_PolyPtBufPos = 0;

			if (bSurfaceSurfaceContact) {
				pres->parea->type = geom_contact_area::polygon;
				pres->n = pGTest[0]->surfaces[i].n;	iop = 0;
				pres->parea->n1 = pGTest[1]->surfaces[j].n;
				npt1 = pGTest[0]->pGeometry->PreparePolygon(&surface, pGTest[0]->surfaces[i].idx,pGTest[0]->surfaces[i].iFeature, pGTest[0], 
					ptbuf1,idbuf1[0],idbuf1[1]);
				npt2 = pGTest[1]->pGeometry->PreparePolygon(&surface, pGTest[1]->surfaces[j].idx,pGTest[1]->surfaces[j].iFeature, pGTest[1], 
					ptbuf2,idbuf2[0],idbuf2[1]);
				npt = boolean2d(bool_intersect, ptbuf1,npt1, ptbuf2,npt2,1, ptbuf,idbuf);
			} else {
				pres->parea->type = geom_contact_area::polyline;
				pres->n = pGTest[iop]->surfaces[i].n;
				pres->parea->n1 = (pGTest[iop^1]->edges[j].dir^(pGTest[iop]->surfaces[i].n^pGTest[iop^1]->edges[j].dir)).normalize();
				pres->parea->n1 *= -sgnnz(pres->parea->n1*pres->n);
				if (iop) {
					vectorf ntmp=pres->n; pres->n=pres->parea->n1; pres->parea->n1=ntmp;
				}
				npt1 = pGTest[iop]->pGeometry->PreparePolygon(&surface, pGTest[iop]->surfaces[i].idx,pGTest[iop]->surfaces[i].iFeature, pGTest[iop], 
					ptbuf1,idbuf1[0],idbuf1[1]);
				npt2 = pGTest[iop^1]->pGeometry->PreparePolyline(&surface, pGTest[iop^1]->edges[j].idx,pGTest[iop^1]->edges[j].iFeature, pGTest[iop^1], 
					ptbuf2,idbuf2[0],idbuf2[1]);
				npt = boolean2d(bool_intersect, ptbuf1,npt1, ptbuf2,npt2,0, ptbuf,idbuf);
			}

			if (npt) {
				for(i=0;i<min(npt,pres->parea->nmaxpt);i++) {
					pres->parea->pt[i] = surface.origin + surface.axes[0]*ptbuf[i].x + surface.axes[1]*ptbuf[i].y;
					bEdgeEdge = -((idbuf[i]>>16)-1>>31 | (idbuf[i]&0xFFFF)-1>>31)^1;
					id = (idbuf[i]&0xFFFF)-1;	idmask = id>>31; id -= idmask;
					pres->parea->piPrim[iop][i] = idbuf1[bEdgeEdge][id]>>8 | idmask; 
					pres->parea->piFeature[iop][i] = idbuf1[bEdgeEdge][id]&0xFF | idmask; 
					id = (idbuf[i]>>16)-1; idmask = id>>31; id -= idmask;
					pres->parea->piPrim[iop^1][i] = idbuf2[bEdgeEdge][id]>>8 | idmask; 
					pres->parea->piFeature[iop^1][i] = idbuf2[bEdgeEdge][id]&0xFF | idmask; 
				}
				pres->parea->npt = i;
			} else pres->parea->npt = 0;

			g_nAreaPt += pres->parea->npt;
			g_nAreas++;

		} else
			pres->parea = 0;
	}

	if (pinters->nborderpt) {
		for(i=0;i<pinters->nborderpt;i++)
			pres->ptborder[pres->nborderpt++] = pinters->ptborder[i];
		g_BrdPtBufPos += pinters->nborderpt;
		pres->bBorderConsecutive = false;
	} else if (pinters->nBestPtVal>100) {
		pres->nborderpt = 1;
		pres->ptborder = &pres->center;
	}

	pGTest[0]->R = unproj.R0;
	pGTest[0]->offset = unproj.offset0;

	// allocate slots for the next intersection
	(*pGTest[0]->pnContacts)++;
	if (*pGTest[0]->pnContacts>=pGTest[0]->nMaxContacts || pGTest[0]->pParams->bStopAtFirstTri || pGTest[0]->pParams->bBothConvex)
		pGTest[0]->bStopIntersection = 1;
	else {
		pres = pGTest[0]->contacts + *pGTest[0]->pnContacts;
		if (!pGTest[0]->pParams->bNoAreaContacts && 
				g_nAreas<sizeof(g_AreaBuf)/sizeof(g_AreaBuf[0]) && g_nAreaPt<sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])) 
		{
			pres->parea = g_AreaBuf+g_nAreas;
			pres->parea->pt = g_AreaPtBuf+g_nAreaPt;
			pres->parea->piPrim[0] = g_AreaPrimBuf0+g_nAreaPt; pres->parea->piFeature[0] = g_AreaFeatureBuf0+g_nAreaPt;
			pres->parea->piPrim[1] = g_AreaPrimBuf1+g_nAreaPt; pres->parea->piFeature[1] = g_AreaFeatureBuf1+g_nAreaPt;
			pres->parea->npt = 0; pres->parea->nmaxpt = sizeof(g_AreaPtBuf)/sizeof(g_AreaPtBuf[0])-g_nAreaPt;
			pres->parea->minedge = min(pGTest[0]->minAreaEdge, pGTest[1]->minAreaEdge);
		} else
			pres->parea = 0;
	}

	return 1;
}


int CTriMesh::GetNeighbouringEdgeId(int vtxid, int ivtx)
{
	int iedge,itri,itrinew,itri0,iter=50;
	itri0=itri = vtxid>>8; iedge = dec_mod3[vtxid&31];
	do {
		if (m_pIndices[itri*3+iedge]==ivtx) // check leaving edge
			return itri<<8 | iedge | 0x20;
		if ((itrinew=m_pTopology[itri].ibuddy[iedge])<0)
			return -1;
		iedge = GetEdgeByBuddy(itrinew, itri); itri = itrinew;
		if (m_pIndices[itri*3+inc_mod3[iedge]]==ivtx) // check entering edge
			return itri<<8 | iedge | 0x20;
		iedge = dec_mod3[iedge];
	} while(itri!=itri0 && --iter>0);
	return -1;
}


int CTriMesh::PreparePolygon(coord_plane *psurface, int iPrim,int iFeature, geometry_under_test *pGTest, vector2df *&ptbuf,
														 int *&pVtxIdBuf,int *&pEdgeIdBuf)
{
	int *pUsedVtxMap,UsedVtxIdx[16],nUsedVtx, *pUsedTriMap,UsedTriIdx[16],nUsedTri;
	int i,ihead,itail,ntri,nvtx,itri,itri_parent,ivtx0,ivtx,itri_new,iedge,iedge_open,ivtx0_new,ivtx_start,ivtx_end,ivtx_end1,
		ivtx_head,iorder,nSides;
	vectorf n0,n,edge,pt0;

	if (psurface->n.len2()==0) {
		psurface->n = pGTest->R*m_pNormals[iPrim];
		psurface->axes[0] = pGTest->R*(m_pVertices[m_pIndices[iPrim*3+1]]-m_pVertices[m_pIndices[iPrim*3]]).normalized();
		psurface->axes[1] = psurface->n^psurface->axes[0];
		psurface->origin = pGTest->R*m_pVertices[m_pIndices[iPrim*3]]*pGTest->scale + pGTest->offset;
	}
	if ((m_nVertices-1>>5)+1<sizeof(g_UsedVtxMap)/sizeof(g_UsedVtxMap[0]))
		pUsedVtxMap = g_UsedVtxMap;
	else memset(pUsedVtxMap = new int[(m_nVertices-1>>5)+1], 0, (m_nVertices-1>>5)+1<<2);
	if ((m_nTris-1>>5)+1<sizeof(g_UsedTriMap)/sizeof(g_UsedTriMap[0]))
		pUsedTriMap = g_UsedTriMap;
	else memset(pUsedTriMap = new int[(m_nTris-1>>5)+1], 0, (m_nTris-1>>5)+1<<2);
	nUsedVtx = nUsedTri = 0;

	#define ENQUEUE_TRI \
	if (itri_new>=0 && !(pUsedTriMap[itri_new>>5] & 1<<(itri_new&31)) && m_pNormals[itri_new]*n0>1.0f-2E-4f) {	\
		ihead = ihead+1&(int)(sizeof(g_TriQueue)/sizeof(g_TriQueue[0]))-1; \
		ntri = min(ntri+1,(int)(sizeof(g_TriQueue)/sizeof(g_TriQueue[0]))); \
		g_TriQueue[ihead].itri = itri_new; \
		g_TriQueue[ihead].itri_parent = itri;	\
		g_TriQueue[ihead].ivtx0 = ivtx0_new; \
		pUsedTriMap[itri_new>>5] |= 1<<(itri_new&31);	\
		UsedTriIdx[nUsedTri] = itri_new; nUsedTri = min(nUsedTri+1,15); \
		m_pTree->MarkUsedTriangle(itri_new, pGTest); \
	}

	ihead=-1; itail=0; n0=m_pNormals[iPrim]; itri=iPrim; ntri=0; nvtx=3;
	g_VtxList[0].ivtx=m_pIndices[itri*3+0]; g_VtxList[0].ibuddy[1]=1; g_VtxList[0].ibuddy[0]=2; g_VtxList[0].id=iPrim<<8;
	g_VtxList[1].ivtx=m_pIndices[itri*3+1]; g_VtxList[1].ibuddy[1]=2;	g_VtxList[1].ibuddy[0]=0;	g_VtxList[1].id=iPrim<<8|1;
	g_VtxList[2].ivtx=m_pIndices[itri*3+2]; g_VtxList[2].ibuddy[1]=0;	g_VtxList[2].ibuddy[0]=1;	g_VtxList[2].id=iPrim<<8|2;
	itri_new=m_pTopology[itri].ibuddy[0]; ivtx0_new=0; ENQUEUE_TRI
	itri_new=m_pTopology[itri].ibuddy[1]; ivtx0_new=1; ENQUEUE_TRI
	itri_new=m_pTopology[itri].ibuddy[2]; ivtx0_new=2; ENQUEUE_TRI
	ivtx_head = 0;
	
	while(ntri>0) {
		itri = g_TriQueue[itail].itri; itri_parent = g_TriQueue[itail].itri_parent; 
		ivtx_head = ivtx0 = g_TriQueue[itail].ivtx0; itail = itail+1&sizeof(g_TriQueue)/sizeof(g_TriQueue[0])-1; ntri--;
		iedge = GetEdgeByBuddy(itri,itri_parent);
		ivtx = m_pIndices[itri*3+dec_mod3[iedge]];
		nSides = 0;

		if (pUsedVtxMap[ivtx>>5] & 1<<(ivtx&31)) {
			// triangle's free vertex is already in the list
			if (g_VtxList[ivtx_start = g_VtxList[ivtx0].ibuddy[0]].ivtx==ivtx) {
				ivtx_end = g_VtxList[g_VtxList[ivtx_start].ibuddy[1]].ibuddy[1]; // tringle touches previous buddy
				iedge_open = dec_mod3[iedge];	ivtx0_new = ivtx_head = ivtx_start; nSides++;
			} 
			if (g_VtxList[ivtx_end1 = g_VtxList[g_VtxList[ivtx0].ibuddy[1]].ibuddy[1]].ivtx==ivtx) {
				ivtx_end = ivtx_end1; ivtx_start = ivtx0_new = ivtx0; // triangle touches next buddy
				iedge_open = inc_mod3[iedge]; nSides++;
			}
			if (nSides==2) { // if triangle touches both neighbours, it must have filled interior hole, trace it and "glue" sides
				ivtx_start = g_VtxList[ivtx0].ibuddy[0];
				ivtx_end = g_VtxList[g_VtxList[ivtx0].ibuddy[1]].ibuddy[1];
				for(; g_VtxList[ivtx_start].ivtx==g_VtxList[ivtx_end].ivtx; 
						ivtx_start=g_VtxList[ivtx_start].ibuddy[0],ivtx_end=g_VtxList[ivtx_end].ibuddy[1]);
				ivtx_start = g_VtxList[ivtx_start].ibuddy[1];
			}
			if (nSides>0) { // remove one vertex if triangle touches only one neighbour
				g_VtxList[ivtx_start].ibuddy[1] = ivtx_end;
				g_VtxList[ivtx_end].ibuddy[0] = ivtx_start;
			}
		}
		if (nSides==0) { // add new vertex to the list if triangle does not touch neighbours
			if (nvtx==sizeof(g_VtxList)/sizeof(g_VtxList[0]))
				break;
			g_VtxList[nvtx].ivtx = ivtx;
			g_VtxList[nvtx].id = itri<<8|dec_mod3[iedge];
			g_VtxList[nvtx].ibuddy[1] = g_VtxList[ivtx0].ibuddy[1];
			g_VtxList[nvtx].ibuddy[0] = ivtx0;
			g_VtxList[ivtx0].ibuddy[1] = g_VtxList[g_VtxList[ivtx0].ibuddy[1]].ibuddy[0] = nvtx++;
			pUsedVtxMap[ivtx>>5] |= 1<<(ivtx&31);
			UsedVtxIdx[nUsedVtx] = ivtx; nUsedVtx = min(nUsedVtx+1,15);

			itri_new = m_pTopology[itri].ibuddy[inc_mod3[iedge]]; ivtx0_new = ivtx0;
			ENQUEUE_TRI
			iedge_open = dec_mod3[iedge];	
			ivtx0_new = g_VtxList[ivtx0].ibuddy[1];
		}
		if (nSides<2) { 
			itri_new = m_pTopology[itri].ibuddy[iedge_open];
			ENQUEUE_TRI
		}
	}
	#undef ENQUEUE_TRI

	if (pUsedVtxMap!=g_UsedVtxMap)
		delete[] pUsedVtxMap;
	else if (nUsedVtx>=15)
		memset(g_UsedVtxMap,0,(m_nVertices-1>>5)+1<<2);
	else for(i=0;i<nUsedVtx;i++)
		g_UsedVtxMap[UsedVtxIdx[i]>>5] &= ~(1<<(UsedVtxIdx[i]&31));
	if (pUsedTriMap!=g_UsedTriMap)
		delete[] pUsedTriMap;
	else if (nUsedTri>=15)
		memset(g_UsedTriMap,0,(m_nTris-1>>5)+1<<2);
	else for(i=0;i<nUsedTri;i++)
		g_UsedTriMap[UsedTriIdx[i]>>5] &= ~(1<<(UsedTriIdx[i]&31));

	ivtx_start = ivtx_head; nvtx = 0;
	ptbuf = g_PolyPtBuf + g_PolyPtBufPos;
	pVtxIdBuf = g_PolyVtxIdBuf + g_PolyPtBufPos;
	pEdgeIdBuf = g_PolyEdgeIdBuf + g_PolyPtBufPos;
	iorder = isnonneg((pGTest->R*n0)*psurface->n);
	do {
		pt0 = pGTest->R*m_pVertices[g_VtxList[ivtx_head].ivtx]*pGTest->scale + pGTest->offset - psurface->origin;
		ptbuf[nvtx].set(psurface->axes[0]*pt0, psurface->axes[1]*pt0); pVtxIdBuf[nvtx]=g_VtxList[ivtx_head].id;
		pEdgeIdBuf[nvtx++] = GetNeighbouringEdgeId(g_VtxList[ivtx_head].id, g_VtxList[g_VtxList[ivtx_head].ibuddy[iorder]].ivtx);
		ivtx_head = g_VtxList[ivtx_head].ibuddy[iorder];
	} while(ivtx_head!=ivtx_start && nvtx<sizeof(g_VtxList)/sizeof(g_VtxList[0]));
	ptbuf[nvtx] = ptbuf[0];	pVtxIdBuf[nvtx] = pVtxIdBuf[0]; pEdgeIdBuf[nvtx] = pEdgeIdBuf[0];
	g_PolyPtBufPos += nvtx+1;

	return nvtx;
}

int CTriMesh::PreparePolyline(coord_plane *psurface, int iPrim,int iFeature, geometry_under_test *pGTest, vector2df *&ptbuf,
															int *&pVtxIdBuf,int *&pEdgeIdBuf)
{
	int *pUsedVtxMap,UsedVtxIdx[16],nUsedVtx=0;
	int nvtx,iedge,itri,itri_prev,ivtx,itri0,iorder,i,iprev,iter=0;
	vectorf n0,edge,pt;

	if ((m_nVertices-1>>5)+1<sizeof(g_UsedVtxMap)/sizeof(g_UsedVtxMap[0]))
		pUsedVtxMap = g_UsedVtxMap;
	else memset(pUsedVtxMap = new int[(m_nVertices-1>>5)+1], 0, (m_nVertices-1>>5)+1<<2);

	iedge = iFeature & 0x1F;
	itri_prev = itri0 = iPrim;
	nvtx = 2;
	g_VtxList[0].ivtx = m_pIndices[itri0*3+iedge];
	g_VtxList[1].ivtx = m_pIndices[itri0*3+inc_mod3[iedge]];
	g_VtxList[0].ibuddy[0] = g_VtxList[1].ibuddy[1] = -1;	
	g_VtxList[0].ibuddy[1] = 1; g_VtxList[1].ibuddy[0] = 0;	
	g_VtxList[0].id = itri0<<8|iedge;	g_VtxList[1].id = itri0<<8|inc_mod3[iedge];
	edge = m_pVertices[m_pIndices[itri0*3+inc_mod3[iedge]]] - m_pVertices[m_pIndices[itri0*3+iedge]];
	ivtx = m_pIndices[itri0*3+iedge]; pUsedVtxMap[ivtx>>5] |= 1<<(ivtx&31);	UsedVtxIdx[nUsedVtx++] = ivtx;
	ivtx = m_pIndices[itri0*3+inc_mod3[iedge]]; pUsedVtxMap[ivtx>>5] |= 1<<(ivtx&31);	UsedVtxIdx[nUsedVtx++] = ivtx;
	n0 = psurface->n*pGTest->R;
	n0 = (edge^(edge^n0)).normalized();

	for(iorder=1,iprev=nvtx-1;iorder>=0;iorder--) {
		do {
			itri = m_pTopology[itri_prev].ibuddy[iedge];
			if (itri<0 || itri==itri0)
				break;
			iedge = dec_mod3[GetEdgeByBuddy(itri,itri_prev)];
			edge = m_pVertices[m_pIndices[itri*3+inc_mod3[iedge]]] - m_pVertices[m_pIndices[itri*3+iedge]];
			if (fabsf(edge*n0)<1E-2f && sqr((m_pVertices[m_pIndices[itri*3+iedge]]+edge*0.5f-psurface->origin)*psurface->n) < edge.len2()*sqr(0.02f)) {
				itri0=itri; iedge=dec_mod3[iedge]; ivtx=m_pIndices[itri*3+inc_mod3[iedge]];
				if (pUsedVtxMap[ivtx>>5] & 1<<(ivtx&31))
					break;
				// add edge to list
				if (nvtx==sizeof(g_VtxList)/sizeof(g_VtxList[0]))
					break;
				g_VtxList[nvtx].ivtx = ivtx;
				g_VtxList[nvtx].id = itri<<8 | inc_mod3[iedge];
				g_VtxList[nvtx].ibuddy[iorder] = -1; g_VtxList[nvtx].ibuddy[iorder^1] = iprev;
				g_VtxList[iprev].ibuddy[iorder] = nvtx; iprev = nvtx++;
				pUsedVtxMap[ivtx>>5] |= 1<<(ivtx&31);
				UsedVtxIdx[nUsedVtx] = ivtx; nUsedVtx = min(nUsedVtx+1,15);
			}
			itri_prev = itri;
		} while(++iter<100);
		itri_prev = itri0 = iPrim;
		iedge = dec_mod3[iFeature & 0x1F];
		iprev = 0;
	}

	if (pUsedVtxMap!=g_UsedVtxMap)
		delete[] pUsedVtxMap;
	else if (nUsedVtx>=15)
		memset(g_UsedVtxMap,0,(m_nVertices-1>>5)+1<<2);
	else for(i=0;i<nUsedVtx;i++)
		g_UsedVtxMap[UsedVtxIdx[i]>>5] &= ~(1<<(UsedVtxIdx[i]&31));

	ptbuf = g_PolyPtBuf + g_PolyPtBufPos;
	pVtxIdBuf = g_PolyVtxIdBuf + g_PolyPtBufPos;
	pEdgeIdBuf = g_PolyEdgeIdBuf + g_PolyPtBufPos;
	for(nvtx=0,ivtx=iprev; ivtx>=0 && nvtx<sizeof(g_VtxList)/sizeof(g_VtxList[0]); ivtx=g_VtxList[ivtx].ibuddy[1],nvtx++) {
		pt = pGTest->R*m_pVertices[g_VtxList[ivtx].ivtx]*pGTest->scale + pGTest->offset - psurface->origin;
		ptbuf[nvtx].set(psurface->axes[0]*pt, psurface->axes[1]*pt); pVtxIdBuf[nvtx]=g_VtxList[ivtx].id;
		pEdgeIdBuf[nvtx] = g_VtxList[ivtx].ibuddy[1]>=0 ? 
			GetNeighbouringEdgeId(g_VtxList[ivtx].id, g_VtxList[g_VtxList[ivtx].ibuddy[1]].ivtx) : -1;
	}
	ptbuf[nvtx] = ptbuf[0]; pVtxIdBuf[nvtx] = pVtxIdBuf[0];	pEdgeIdBuf[nvtx] = pEdgeIdBuf[0];
	g_PolyPtBufPos += nvtx+1;

	return nvtx;
}

template <class ftype> class vector2_w_enforcer {
public:
  vector2_w_enforcer(Vec3_tpl<ftype> &_vec0,Vec3_tpl<ftype> &_vec1,ftype &_w) : vec0(_vec0),vec1(_vec1),w(_w) {}
	~vector2_w_enforcer() {
		if (w!=(ftype)1.0 && fabs_tpl(w)>1E-20) {
			float rw = 1.0f/w;
			vec0*=rw; vec1*=rw;
		}
	}
	Vec3_tpl<ftype> &vec0,&vec1;
	ftype &w;
};

int CTriMesh::FindClosestPoint(geom_world_data *pgwd, int &iPrim,int &iFeature, const vectorf &ptdst0,const vectorf &ptdst1,
															 vectorf *ptres, int nMaxIters)
{
	vectorf pt[3],n,nprev,edge,dp,seg,cross,ptdst,ptseg[3];
	int i,itri,ivtx,iedge,bEdgeOut[4],bOut,itri0,inewtri,itrimax,bBest,iter;
	float t,tmax,dist[3],ptdstw=1.0f;
	int iPrim_hist[4]={-1,-1,-1,-1},iFeature_hist[4]={-1,-1,-1,-1},ihist=0,bLooped;
	int bLine = isneg(1E-4f-(seg=ptdst1-ptdst0).len2());
	ptseg[0] = ptdst0; ptseg[1] = ptdst1;
	ptres[1] = ptdst0;
	ptres[0].Set(1E10f,1E10f,1E10f);
	vector2_w_enforcer<float> wdiv(ptres[1],ptres[0],ptdstw);

	if ((unsigned int)iPrim<(unsigned int)m_nTris) do {
		if ((iFeature&0x60)==0x40) { // face
			pt[0] = pgwd->R*m_pVertices[m_pIndices[iPrim*3+0]]*pgwd->scale + pgwd->offset;
			pt[1] = pgwd->R*m_pVertices[m_pIndices[iPrim*3+1]]*pgwd->scale + pgwd->offset;
			pt[2] = pgwd->R*m_pVertices[m_pIndices[iPrim*3+2]]*pgwd->scale + pgwd->offset;
			n = pgwd->R*m_pNormals[iPrim];

			if (bLine) {
				dist[0]=(ptseg[0]-pt[0])*n; dist[1]=(ptseg[1]-pt[0])*n;
				if (dist[0]*dist[1]<0 && fabsf(ptdstw=seg*n)>1E-4f) {
					ptres[1] = (ptseg[0]*ptdstw + seg*((pt[0]-ptseg[0])*n))*sgnnz(ptdstw);
					ptdstw = fabsf(ptdstw);
					pt[0]*=ptdstw; pt[1]*=ptdstw; pt[2]*=ptdstw;
				} else {
					ptres[1] = ptseg[isneg(dist[1]-dist[0])];
					ptdstw = 1.0f;
				}
			}	else {
				ptres[1] = ptdst0; ptdstw = 1.0f;
			}

			bEdgeOut[0] = isneg((pt[1]-pt[0]^ptres[1]-pt[0])*n);
			bEdgeOut[1] = isneg((pt[2]-pt[1]^ptres[1]-pt[1])*n);
			bEdgeOut[2] = isneg((pt[0]-pt[2]^ptres[1]-pt[2])*n);
			ptres[0] = ptres[1]-n*((ptres[1]-pt[0])*n);
			if (bEdgeOut[0]+bEdgeOut[1]+bEdgeOut[2]==0)
				return nMaxIters; // ptres[1] is in triangle's voronoi region

			i = (bEdgeOut[0]^1)+((bEdgeOut[0]|bEdgeOut[1])^1); // i = index of the 1st non-zero (unity) bEdgeOut
			if (m_pTopology[iPrim].ibuddy[i]>=0) {
				iFeature = GetEdgeByBuddy(m_pTopology[iPrim].ibuddy[i], iPrim) | 0x20;
				iPrim = m_pTopology[iPrim].ibuddy[i];
			} else {
				edge = pt[inc_mod3[i]]-pt[i]; t = min(ptdstw=edge.len2(),max(0.0f,(ptdst0-pt[i])*edge));
				ptres[0] = pt[i]*ptdstw+edge*t; ptres[1] *= ptdstw;
				iFeature = 0x20 | i; return nMaxIters;
			}
			
			/*for(i=0;i<3;i++) if (bEdgeOut[i]) {
				if ((ptres[1]-pt[i])*(pt[inc_mod3[i]]-pt[i])>0 && (ptres[1]-pt[inc_mod3[i]])*(pt[i]-pt[inc_mod3[i]])>0) {
					// ptres[1] is possibly in edge's voronoi region
					if (m_pTopology[iPrim].ibuddy[i]<0) {
						iFeature = 0x20 | i; return nMaxIters;
					}
					iFeature = GetEdgeByBuddy(m_pTopology[iPrim].ibuddy[i], iPrim) | 0x20;
					iPrim = m_pTopology[iPrim].ibuddy[i];
					n = pgwd->R*m_pNormals[iPrim];
					if ((pt[i]-pt[inc_mod3[i]]^ptres[1]-pt[i])*n<0) {
						edge = pt[inc_mod3[i]]-pt[i]; // ptres[1] is in edge's voronoi region
						//ptres[0] = pt[i]+edge*((edge*(ptres[1]-pt[i]))/edge.len2());
						ptres[0] = pt[i]*(t=edge.len2()) + edge*((edge*(ptres[1]-pt[i])));
						ptdstw *= t; ptres[1] *= t;
						return nMaxIters;
					} else {
						iFeature = 0x40; break;
					}
				}
			}
			if (i==3) {	// switch to vertex that is closest to the point
				i = isneg((pt[1]-ptres[1]).len2()-(pt[0]-ptres[1]).len2());
				i |= isneg((pt[2]-ptres[1]).len2()-(pt[i]-ptres[1]).len2())<<1; i &= 2|(i>>1^1);
				iFeature = i;
			}*/
		} else if ((iFeature&0x60)==0x20) {	// edge
			iedge = iFeature & 0x1F;
			n = pgwd->R*m_pNormals[iPrim];
			pt[0] = pgwd->R*m_pVertices[m_pIndices[iPrim*3+iedge]]*pgwd->scale + pgwd->offset;
			pt[1] = pgwd->R*m_pVertices[m_pIndices[iPrim*3+inc_mod3[iedge]]]*pgwd->scale + pgwd->offset;
			edge = pt[1]-pt[0]; 
			
			if (bLine) {
				cross = seg^edge;
				t = (pt[0]-ptseg[0]^edge)*cross; ptdstw = cross.len2(); 
				i = isneg(1E-6f-ptdstw); ptdstw = ptdstw*i + (i^1); // set ptdstw to 1 if it's too small - point is ill defined anyway
				t = min(ptdstw,max(0.0f,t));
				dist[0] = (ptseg[0]-pt[0]^edge).len2()*ptdstw; 
				dist[1] = (ptseg[1]-pt[0]^edge).len2()*ptdstw;
				ptseg[2] = ptseg[0]*ptdstw + seg*t; dist[2] = sqr((ptseg[0]-pt[0])*cross)*edge.len2();
				i = idxmin3(dist);
				ptres[1] = ptseg[i]; ptdstw = ptdstw*(i>>1)+(i>>1^1); // reset ptdstw to 1.0 if i is not 2
				pt[0] *= ptdstw; pt[1] *= ptdstw; edge *= ptdstw;
			}	else {
				ptres[1] = ptdst0; ptdstw = 1.0f;
			}

			if (m_pTopology[iPrim].ibuddy[iedge]<0)
				iFeature = 0x40;
			else {
				nprev = pgwd->R*m_pNormals[m_pTopology[iPrim].ibuddy[iedge]];
				dp = ptres[1]-pt[0];
				bEdgeOut[0] = isnonneg((n^edge)*dp); // point leans to the 1st triangle
				bEdgeOut[1] = isneg((nprev^edge)*dp); // point leans to the 2nd triangle
				bEdgeOut[2] = isneg((ptres[1]-pt[0])*edge); // point leans to start vertex
				bEdgeOut[3] = isnonneg((ptres[1]-pt[1])*edge); // point leans to end vertex
				//ptres[0] = pt[0]+edge*((edge*dp)/edge.len2());
				ptres[0] = pt[0]*(t=edge.len2()) + edge*((edge*(ptres[1]-pt[0])));
				ptdstw *= t; ptres[1] *= t;

				if (bEdgeOut[0]+bEdgeOut[1]+bEdgeOut[2]+bEdgeOut[3]==0)
					return nMaxIters;
				if (bEdgeOut[0]+bEdgeOut[1]) {
					if (bEdgeOut[1]) iPrim = m_pTopology[iPrim].ibuddy[iedge];
					iFeature = 0x40;
				} else
					iFeature = inc_mod3[iedge]&-bEdgeOut[3] | iedge&~-bEdgeOut[3];
			}
		}	else if ((iFeature&0x60)==0) { // vertex
			pt[0] = pgwd->R*m_pVertices[m_pIndices[iPrim*3+(iFeature&0x1F)]]*pgwd->scale + pgwd->offset;

			if (bLine) {
				dist[0] = (ptseg[0]-pt[0]).len2();
				dist[1] = (ptseg[1]-pt[0]).len2();
				dist[2] = (pt[0]-ptseg[0]^seg).len2(); ptdstw=seg.len2(); dist[0]=sqr(dist[0])*ptdstw; dist[1]=sqr(dist[1])*ptdstw;
				ptseg[2] = ptseg[0]*ptdstw + seg*(seg*(pt[0]-ptseg[0]));
				i = idxmin3(dist);
				ptres[1] = ptseg[i];	ptdstw = ptdstw*(i>>1)+(i>>1^1); // reset ptdstw to 1.0 if i is not 2
				pt[0] *= ptdstw;
			} else {
				ptres[1] = ptdst0; ptdstw = 1.0f;
			}

			dp=ptres[1]-pt[0]; itri=itrimax=itri0=iPrim; ivtx=iFeature&0x1F; tmax=0; bOut=0;
			if (m_pTopology[itri].ibuddy[ivtx]>=0)
				nprev = pgwd->R*m_pNormals[m_pTopology[itri].ibuddy[ivtx]];
			else nprev.zero();
			iter=35; do {
				n = pgwd->R*m_pNormals[itri];
				bOut |= isneg((nprev^n)*dp+1E-7f);
				bBest = isneg(tmax-(t=n*dp));
				tmax = tmax*(bBest^1)+t*bBest;
				itrimax = itrimax&~-bBest | itri&-bBest;
				if ((inewtri = m_pTopology[itri].ibuddy[dec_mod3[ivtx]])<0)
					break;
				ivtx = GetEdgeByBuddy(inewtri,itri); itri = inewtri; nprev = n;
			} while(itri!=itri0 && --iter);
			ptres[0] = pt[0];
			if (!bOut)
				return nMaxIters;
			iPrim = itrimax; iFeature = 0x40;
		} else
			return -1;

		bLooped = iszero(iPrim-iPrim_hist[0] | iFeature-iFeature_hist[0]) | iszero(iPrim-iPrim_hist[1] | iFeature-iFeature_hist[1]) |
			iszero(iPrim-iPrim_hist[2] | iFeature-iFeature_hist[2]) | iszero(iPrim-iPrim_hist[3] | iFeature-iFeature_hist[3]);
		iPrim_hist[ihist] = iPrim; iFeature_hist[ihist] = iFeature; ihist = ihist+1&3;
		// if line mode and looped, don't allow final iteration to be face one (face doesn't compute ptres[1] properly)
	} while(!bLooped && --nMaxIters>0 || bLine && iFeature_hist[ihist-2&3]==0x40);
	else 
		return -1;

	return max(nMaxIters,0);
}


int CTriMesh::PointInsideStatus(const vectorf &pt)
{
	int res=0;
	float mindist=1E20f;
	triangle atri;
	ray aray;
	int icell,i;
	vectorf ptgrid;
	prim_inters inters;

	if (m_nTris<4)
		return 0;

	ptgrid = m_hashgrid[0].Basis*(pt-m_hashgrid[0].origin);
	icell = float2int(m_hashgrid[0].stepr.x*ptgrid.x-0.5f)*m_hashgrid[0].stride.x + float2int(m_hashgrid[0].stepr.y*ptgrid.y-0.5f)*m_hashgrid[0].stride.y;
	aray.origin = pt;
	aray.dir = m_hashgrid[0].Basis.GetRow(2)*(m_hashgrid[0].step.x*m_hashgrid[0].size.x + m_hashgrid[0].step.y*m_hashgrid[0].size.y);
	for(i=m_pHashGrid[0][icell]; i<m_pHashGrid[0][icell+1]; i++) {
		atri.pt[0] = m_pVertices[m_pIndices[m_pHashData[0][i]*3+0]];
		atri.pt[1] = m_pVertices[m_pIndices[m_pHashData[0][i]*3+1]];
		atri.pt[2] = m_pVertices[m_pIndices[m_pHashData[0][i]*3+2]];
		atri.n = m_pNormals[i];
		if (ray_tri_intersection(&aray,&atri, &inters) && (inters.pt[0]-aray.origin)*aray.dir<mindist) {
			mindist = (inters.pt[0]-aray.origin)*aray.dir;
			res = isneg(aray.dir*atri.n);
		}
	}

	return res;
}


void CTriMesh::PrepareForRayTest(float raylen)
{
	if (m_nHashPlanes==0) {
		coord_plane hashplane;
		box bbox; m_pTree->GetBBox(&bbox);
		float rcellsize=0;
		vectori isz;
		int i,iPlane;
		bbox.size.x = max(bbox.size.x, raylen*0.5f);
		bbox.size.y = max(bbox.size.y, raylen*0.5f);
		bbox.size.z = max(bbox.size.z, raylen*0.5f);

		if (raylen>0) {
			rcellsize = 1.0f/raylen;
			for(i=0;i<3;i++)
				isz[i] = float2int(bbox.size[i]*2*rcellsize+0.5f);
			for(i=0;i<3;i++) if (isz[inc_mod3[i]]*isz[dec_mod3[i]]>4096) {
				isz[inc_mod3[i]] = min(64,isz[inc_mod3[i]]); 
				isz[dec_mod3[i]] = min(64,isz[dec_mod3[i]]);
			}
			if (m_nTris > (isz.x*isz.y+isz.x*isz.z+isz.y*isz.z)*6)
				return; // hash will be too inefficient
		}

		iPlane = isneg(bbox.size.y*bbox.size.z-bbox.size.x*bbox.size.z); // initially set iPlane correspond to the bbox axis w/ max. face area
		iPlane |= isneg(bbox.size[inc_mod3[iPlane]]*bbox.size[dec_mod3[iPlane]]-bbox.size.x*bbox.size.y)<<1; iPlane&=~(iPlane>>1);
		for(i=0; i<3; i++,iPlane=inc_mod3[iPlane]) {
			hashplane.n = bbox.Basis.GetRow(iPlane);
			hashplane.axes[0] = bbox.Basis.GetRow(inc_mod3[iPlane]);
			hashplane.axes[1] = bbox.Basis.GetRow(dec_mod3[iPlane]);
			hashplane.origin = bbox.center;
			HashTrianglesToPlane(hashplane,vector2df(bbox.size[inc_mod3[iPlane]]*2,bbox.size[dec_mod3[iPlane]]*2), 
				m_hashgrid[i],m_pHashGrid[i],m_pHashData[i], rcellsize);
		}
		m_nHashPlanes = i;
	}
}


void CTriMesh::HashTrianglesToPlane(const coord_plane &hashplane, const vector2df &hashsize, grid &hashgrid,index_t *&pHashGrid,index_t *&pHashData, 
																		float rcellsize)
{
	float maxsz,edge2[3],tsx,tex,tsy,tey,ts,te;
	vector2df sz,pt[3],edge[3],step,rstep,ptc,sg;
	vector2di isz,irect[2],ipt;
	int i,itri,ipass,bFilled,imax,ix,iy,idx;
	index_t dummy,*pgrid,iHashMax=0;
	vectorf ptcur,origin;

	maxsz = max(hashsize.x,hashsize.y)*1E-5f;
	sz.set(hashsize.x+maxsz, hashsize.y+maxsz);
	imax = isneg(sz.x-sz.y);
	if (rcellsize==0) {
		isz[imax] = float2int(sqrt_tpl(m_nTris*2*sz[imax]/sz[imax^1]));
		isz[imax^1] = max(1,m_nTris*2/isz[imax]);
	} else {
		isz.x = float2int(sz.x*rcellsize+0.5f);
		isz.y = float2int(sz.y*rcellsize+0.5f);
	}
	if ((unsigned int)(isz.x*isz.y)>4096u) {
		isz.x = min(64,max(1,isz.x)); isz.y = min(64,max(1,isz.y));
	} else if (isz.x*isz.y==0)
		return;
	memset(pHashGrid = new index_t[isz.x*isz.y+1], 0, (isz.x*isz.y+1)*sizeof(index_t));
	step.set(sz.x/isz.x, sz.y/isz.y);
	rstep.set(isz.x/sz.x, isz.y/sz.y);
	origin = hashplane.origin - hashplane.axes[0]*(step.x*isz.x*0.5f)-hashplane.axes[1]*(step.y*isz.y*0.5f);
	pHashData = &dummy;

	for(ipass=0;ipass<2;ipass++) {
		for(itri=m_nTris-1;itri>=0;itri--) { // iterate tris in reversed order to get them in accending order in grid (since the algorithm reverses order)
			i = isneg(m_pNormals[itri]*hashplane.n)<<1;
			ptcur = m_pVertices[m_pIndices[itri*3+i]]-origin; pt[0].set(ptcur*hashplane.axes[0],ptcur*hashplane.axes[1]);
			ptcur = m_pVertices[m_pIndices[itri*3+1]]-origin; pt[1].set(ptcur*hashplane.axes[0],ptcur*hashplane.axes[1]);
			ptcur = m_pVertices[m_pIndices[itri*3+(i^2)]]-origin; pt[2].set(ptcur*hashplane.axes[0],ptcur*hashplane.axes[1]);
			edge2[0] = (edge[0] = pt[1]-pt[0]).len2(); 
			edge2[1] = (edge[1] = pt[2]-pt[1]).len2(); 
			edge2[2] = (edge[2] = pt[0]-pt[2]).len2();
			irect[1] = irect[0].set(float2int(pt[0].x*rstep.x-0.5f), float2int(pt[0].y*rstep.y-0.5f));
			ipt.set(float2int(pt[1].x*rstep.x-0.5f), float2int(pt[1].y*rstep.y-0.5f));
			irect[0].set(min(irect[0].x,ipt.x), min(irect[0].y,ipt.y));
			irect[1].set(max(irect[1].x,ipt.x), max(irect[1].y,ipt.y));
			ipt.set(float2int(pt[2].x*rstep.x-0.5f), float2int(pt[2].y*rstep.y-0.5f));
			irect[0].set(max(0,min(irect[0].x,ipt.x)), max(0,min(irect[0].y,ipt.y)));
			irect[1].set(min(isz.x-1,max(irect[1].x,ipt.x)), min(isz.y-1,max(irect[1].y,ipt.y)));

			for(iy=irect[0].y;iy<=irect[1].y;iy++) 
			for(pgrid=pHashGrid+iy*isz.x+(ix=irect[0].x),ptc.set((ix+0.5f)*step.x,(iy+0.5f)*step.y);ix<=irect[1].x;ix++,pgrid++,ptc.x+=step.x) {
				bFilled = isneg(ptc-pt[0]^edge[0]) & isneg(ptc-pt[1]^edge[1]) & isneg(ptc-pt[2]^edge[2]); // check if cell center is inside triangle
				for(i=0;i<3;i++) { // for each edge, find intersections with cell x and y bounds, then check intersection of x and y ranges
					sg.x = sgn(edge[i].x); sg.y = sgn(edge[i].y);
					tsx = max(0.0f, (ptc.x-step.x*sg.x*0.5f-pt[i].x)*sg.x);
					tex = min(fabsf(edge[i].x), (ptc.x+step.x*sg.x*0.5f-pt[i].x)*sg.x);
					tsy = max(0.0f, (ptc.y-step.y*sg.y*0.5f-pt[i].y)*sg.y);
					tey = min(fabsf(edge[i].y), (ptc.y+step.y*sg.y*0.5f-pt[i].y)*sg.y);
					ts = tsx*fabsf(edge[i].y)+tsy*fabsf(edge[i].x)+fabsf(tsx*fabsf(edge[i].y)-tsy*fabsf(edge[i].x));
					te = tex*fabsf(edge[i].y)+tey*fabsf(edge[i].x)-fabsf(tex*fabsf(edge[i].y)-tey*fabsf(edge[i].x));
					// test tsx,tex and tsy,tey as well, since ts,te might be indefinite (0,0)
					bFilled |= isneg(tsx-tex-1E-10f) & isneg(tsy-tey-1E-10f) & isneg(ts-te-1E-10f); 
				}
				*pgrid -= ipass & -bFilled;	// during the 2nd pass if bFilled, decrement *pgrid prior to storing
				idx = max(0,min(iHashMax,*pgrid));
				(pHashData[idx] &= ~-bFilled) |= itri&-bFilled; // store itri if bFilled is 1
				*pgrid += (ipass^1) & bFilled; // during the 1st pass if bFilled, increment *pgrid after storing
			}
		}
		if (ipass==0) {
			for(i=1;i<=isz.x*isz.y;i++) pHashGrid[i]+=pHashGrid[i-1];
			pHashData = new index_t[iHashMax = pHashGrid[isz.x*isz.y]];
			memset(pHashData, 0,iHashMax*sizeof(index_t));
			iHashMax--;
		}
	}

	hashgrid.Basis.SetRow(0,hashplane.axes[0]);
	hashgrid.Basis.SetRow(1,hashplane.axes[1]);
	hashgrid.Basis.SetRow(2,hashplane.n);
	hashgrid.origin = origin;
	hashgrid.step = step;
	hashgrid.stepr = rstep;
	hashgrid.size = isz;
	hashgrid.stride.set(1,isz.x);
}


int CTriMesh::Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts)
{
	if (pCollider->GetType()==GEOM_RAY && m_nHashPlanes>0) {
		ray aray,arayrot;
		int trilist[3][1024],nTris[3];
		int iPlane,iListPlane,iListRes,iListTmp,ix,iy,iCell;
		vector2df pt[2];
		vector2di ipt[2],irect[2];
		float rscale = pdata1->scale==1.0f ? 1.0f:1.0f/pdata1->scale;
		indexed_triangle atri;
		prim_inters inters;
		unprojection_mode unproj;
		int i,j,jmax,nSmallSteps,iEdge, nContacts=0;
		intptr_t idmask = ~iszero_mask(m_pIds);
		short idnull=-1, *pIds=(short*)((intptr_t)m_pIds&idmask|(intptr_t)&idnull&~idmask);
		bool bStopAtFirstTri;
		real t;
		contact contact_best,contact_cur;
		CRayGeom *pRay = (CRayGeom*)pCollider;
		ray *pray = (ray*)&pRay->m_ray;
		if (!pparams || !pparams->bKeepPrevContacts)
			g_nTotContacts = 0;
		pcontacts = g_Contacts+g_nTotContacts;
		arayrot.origin = aray.origin = ((pray->origin-pdata1->offset)*pdata1->R)*rscale;
		arayrot.dir = aray.dir = pray->dir*pdata1->R;
		unproj.dir.zero();

		if (pparams) {
			if (((CGeometry*)pCollider)->m_iCollPriority<m_iCollPriority)
				unproj.imode = -1;
			else {
				unproj.imode = pparams->iUnprojectionMode;
				unproj.center = ((pparams->centerOfRotation-pdata1->offset)*pdata1->R)*rscale;
				unproj.dir = (unproj.imode ? pparams->axisOfRotation : -pdata1->v)*pdata1->R;
				unproj.tmax = pi*0.5; // not used for linear unprojection
				unproj.minPtDist = m_minVtxDist;
			}
			bStopAtFirstTri = pparams->bStopAtFirstTri;
			pparams->pGlobalContacts = g_Contacts;
		}	else {
			unproj.imode = -1;
			bStopAtFirstTri = false;
		}

		if ((unsigned int)pdata1->iStartNode-1u<(unsigned int)m_nTris) {
			atri.n = m_pNormals[pdata1->iStartNode-1];
			atri.pt[0] = m_pVertices[m_pIndices[(pdata1->iStartNode-1)*3+0]];
			atri.pt[1] = m_pVertices[m_pIndices[(pdata1->iStartNode-1)*3+1]];
			atri.pt[2] = m_pVertices[m_pIndices[(pdata1->iStartNode-1)*3+2]];
			atri.idx = pdata1->iStartNode-1;
			if (ray_tri_intersection(&aray,&atri,&inters)) {
				bStopAtFirstTri = true; goto cached_inters;
			}
		}

		for(iPlane=iListRes=0; iPlane<m_nHashPlanes; iPlane++) {
			pt[0] = m_hashgrid[iPlane].Basis*(aray.origin-m_hashgrid[iPlane].origin);
			pt[1] = pt[0]+vector2df(m_hashgrid[iPlane].Basis*aray.dir);
			ipt[0].set(float2int(pt[0].x*m_hashgrid[iPlane].stepr.x-0.5f), float2int(pt[0].y*m_hashgrid[iPlane].stepr.y-0.5f));
			ipt[1].set(float2int(pt[1].x*m_hashgrid[iPlane].stepr.x-0.5f), float2int(pt[1].y*m_hashgrid[iPlane].stepr.y-0.5f));
			irect[0].set(max(0,min(ipt[0].x,ipt[1].x)), max(0,min(ipt[0].y,ipt[1].y)));
			irect[1].set(min(m_hashgrid[iPlane].size.x-1,max(ipt[0].x,ipt[1].x)), min(m_hashgrid[iPlane].size.y-1,max(ipt[0].y,ipt[1].y)));
			if (irect[0].x+1-irect[1].x>>31 | irect[0].x+1-irect[1].x>>31) // can be ineffective for long rays
				goto skiphashes;

			nTris[iListPlane = inc_mod3[iListRes]] = 0; iListTmp = inc_mod3[iListPlane];
			for(iy=irect[0].y; iy<=irect[1].y; iy++) 
			for(iCell=iy*m_hashgrid[iPlane].stride.y+(ix=irect[0].x)*m_hashgrid[iPlane].stride.x; ix<=irect[1].x; ix++,iCell+=m_hashgrid[iPlane].stride.x) {
				iListPlane^=iListTmp; iListTmp^=iListPlane; iListPlane^=iListTmp;
				nTris[iListPlane] = unite_lists(m_pHashData[iPlane]+m_pHashGrid[iPlane][iCell],m_pHashGrid[iPlane][iCell+1]-m_pHashGrid[iPlane][iCell], 
					trilist[iListTmp],nTris[iListTmp], trilist[iListPlane],sizeof(trilist[0])/sizeof(trilist[0][0]));
			}
			if (iPlane>0) {
				nTris[iListTmp] = intersect_lists(trilist[iListRes],nTris[iListRes], trilist[iListPlane],nTris[iListPlane], trilist[iListTmp]);
				iListRes = iListTmp;
			}	else
				iListRes = iListPlane;
		}

		if (nTris[iListRes]) {
			for(i=0;i<nTris[iListRes] && g_nTotContacts+nContacts<sizeof(g_Contacts)/sizeof(g_Contacts[0]);i++) {
				atri.n = m_pNormals[trilist[iListRes][i]];
				atri.pt[0] = m_pVertices[m_pIndices[trilist[iListRes][i]*3+0]];
				atri.pt[1] = m_pVertices[m_pIndices[trilist[iListRes][i]*3+1]];
				atri.pt[2] = m_pVertices[m_pIndices[trilist[iListRes][i]*3+2]];
				atri.idx = trilist[iListRes][i];
				if (ray_tri_intersection(&aray,&atri,&inters)) {
					cached_inters:
					if (unproj.imode>=0) {	
						// here goes a simplified version of RegisterIntersection
						if (unproj.imode && unproj.dir.len2()==0) {
							unproj.dir = atri.n^aray.dir;
							//if (unproj.dir.len2()<0.002) unproj.dir = aray.dir.orthogonal();
							if (unproj.dir.len2()<0.002) unproj.dir.SetOrthogonal(aray.dir);
							unproj.dir.normalize();
						}

						if (!g_Unprojector.Check(&unproj, triangle::type,ray::type, &atri,-1,&aray,-1, &contact_best))
							continue;
						if (unproj.imode)
							contact_best.t = atan2(contact_best.t,contact_best.taux);
						pcontacts[nContacts].iPrim[0] = atri.idx; 

						for(nSmallSteps=0,t=0; contact_best.t>0 && nSmallSteps<3 && (contact_best.iFeature[1] & 0x80); ) {
							t += contact_best.t;
							if (unproj.imode==1) {
								real cosa=cos_tpl(t),sina=sin_tpl(t);
								arayrot.origin = aray.origin.rotated(unproj.center,-unproj.dir, cosa,sina);
								arayrot.dir = aray.dir.rotated(-unproj.dir, cosa,sina);
							} else
								arayrot.origin = aray.origin - unproj.dir*t;
							if (t>unproj.tmax)
								break;

							if ((j = m_pTopology[atri.idx].ibuddy[contact_best.iFeature[1]&0x1F])<0)
								break;
							atri.n = m_pNormals[j];
							atri.pt[0] = m_pVertices[m_pIndices[j*3+0]];
							atri.pt[1] = m_pVertices[m_pIndices[j*3+1]];
							atri.pt[2] = m_pVertices[m_pIndices[j*3+2]];
							iEdge = GetEdgeByBuddy(j,atri.idx);
							atri.idx = j;

							contact_best.t = 0;
							if (g_Unprojector.Check(&unproj, triangle::type,ray::type, &atri,iEdge|0xA0,&arayrot,0xA0, &contact_cur)) {
								contact_best = contact_cur;
								pcontacts[nContacts].iPrim[0] = atri.idx; 
								if (unproj.imode) {
									contact_best.t = atan2(contact_best.t,contact_best.taux);
									j = isneg(contact_best.t-(real)1E-3);
								} else
									j = isneg(contact_best.t-m_minVtxDist);
								nSmallSteps = (nSmallSteps & -j)+j;
							}
						}

						pcontacts[nContacts].t = t+contact_best.t;
						pcontacts[nContacts].pt = pdata1->R*contact_best.pt*pdata1->scale+pdata1->offset;
						pcontacts[nContacts].n = pdata1->R*contact_best.n; // not normalized!
						pcontacts[nContacts].dir = unproj.dir;
						pcontacts[nContacts].iFeature[0] = contact_best.iFeature[1];
						pcontacts[nContacts].iUnprojMode = unproj.imode;
					} else {
						pcontacts[nContacts].pt = pdata1->R*inters.pt[0]*pdata1->scale+pdata1->offset;
						pcontacts[nContacts].t = (pcontacts[nContacts].pt-pray->origin)*pRay->m_dirn;
						pcontacts[nContacts].n = pdata1->R*inters.n;
						pcontacts[nContacts].dir.zero();
						pcontacts[nContacts].iPrim[0] = atri.idx; 
						pcontacts[nContacts].iFeature[0] = 0x40;
						pcontacts[nContacts].iUnprojMode = 0;
					}

					pcontacts[nContacts].vel = 0;
					pcontacts[nContacts].ptborder = &pcontacts[nContacts].pt;
					pcontacts[nContacts].nborderpt = 1;
					pcontacts[nContacts].parea = 0;
					pcontacts[nContacts].id[0] = pIds[atri.idx&idmask];
					pcontacts[nContacts].id[1] = -1;
					pcontacts[nContacts].iPrim[1] = 0; pcontacts[nContacts].iFeature[1] = 0x20;
					pcontacts[nContacts].iNode[0] = pcontacts[nContacts].iPrim[0]+1;
					pcontacts[nContacts].iNode[1] = 0;
					nContacts++;
					if (bStopAtFirstTri)
						break;
				}
			}

			// sort contacts in descending t order
			geom_contact tmpcontact;
			for(i=0;i<nContacts-1;i++) {
				for(jmax=i,j=i+1; j<nContacts; j++) {
					idmask = -isneg(pcontacts[jmax].t-pcontacts[j].t);
					jmax = jmax&~idmask | j&idmask;
				}
				if (jmax!=i) {
					tmpcontact=pcontacts[i]; pcontacts[i]=pcontacts[jmax]; pcontacts[jmax]=tmpcontact;
				}
			}

			return nContacts;
		}
		return 0;
	}
skiphashes:
	return CGeometry::Intersect(pCollider,pdata1,pdata2,pparams,pcontacts);
}


void CTriMesh::CalcVolumetricPressure(geom_world_data *gwd, const vectorf &epicenter,float k,float rmin, 
																			const vectorf &centerOfMass, vectorf &P,vectorf &L)
{
	int i,j,npt,iter;
	vectorf pt[64],ptc,dP,Pres(zero),Lres(zero),c,cm;
	float r2,t1,t2,rmin2,rscale;
	c = (epicenter-gwd->offset)*gwd->R;
	cm = (centerOfMass-gwd->offset)*gwd->R;
	rscale = 1.0f/gwd->scale;
	rmin2 = sqr(rmin*rscale);
	k *= rscale*rscale;

	for(i=0;i<m_nTris;i++) if (m_pNormals[i]*(m_pVertices[m_pIndices[i*3]]-c)>0) {
		pt[0]=m_pVertices[m_pIndices[i*3]]; pt[1]=m_pVertices[m_pIndices[i*3+1]]; pt[2]=m_pVertices[m_pIndices[i*3+2]];
		iter=0; npt=3; do {
			npt -= 3;
			for(j=0; j<3 && (pt[npt+inc_mod3[j]]-pt[npt+j] ^ pt[npt+inc_mod3[j]]-c)*m_pNormals[i]>1E-4; j++);
			if (j==3 && npt<=sizeof(pt)/sizeof(vectorf)-9) { // closest to the epicenter point lies inside triangle, split on this point
				ptc = c-m_pNormals[i]*(m_pNormals[i]*(c-pt[npt]));
				pt[npt+3]=pt[npt];pt[npt+5]=pt[npt+2]; pt[npt+6]=pt[npt];pt[npt+7]=pt[npt+1];
				for(j=0;j<3;j++) pt[npt+j*4] = ptc;
				npt += 9;
			} else {
				for(j=0;j<3;j++) { 
					t1 = (pt[npt+j]-c).len2(); t2 = (pt[npt+inc_mod3[j]]-c).len2();
					if ((t1>rmin || t2>rmin) && (t1<(0.7f*0.7f)*t2 || t1>(1.0f/0.7f/0.7f)*t2)) 
						break;
				}
				if (j<3 && npt<=sizeof(pt)/sizeof(vectorf)-12) {
					for(j=0;j<3;j++) { // split triangle on into 4 if difference between distances from any 2 vertices to epicenter is too big
						pt[npt+j*3+3] = pt[npt+j];
						pt[npt+j*3+4] = (pt[npt+j]+pt[npt+inc_mod3[j]])*0.5f;
						pt[npt+j*3+5] = (pt[npt+j]+pt[npt+dec_mod3[j]])*0.5f;
					}
					for(j=0;j<3;j++)
						pt[npt+j] = (pt[npt+j*3+3]+pt[npt+inc_mod3[j]*3+3])*0.5f;
					npt += 12;
				}	else {
					ptc = (pt[npt]+pt[npt+1]+pt[npt+2])*(1.0f/3); r2 = (ptc-c).len2();
					dP = m_pNormals[i]*(((ptc-c)*m_pNormals[i])*(pt[npt+1]-pt[npt]^pt[npt+2]-pt[npt]).len()*0.5f*k/(sqrt_tpl(r2)*max(r2,rmin2)));
					Pres += dP; Lres += ptc-cm^dP;
				}
			}
		} while(npt>0 && ++iter<300);
	}
	P += gwd->R*Pres;
	L += gwd->R*Lres;
}


float CalcPyramidVolume(const vectorf &pt0,const vectorf &pt1,const vectorf &pt2,const vectorf &pt3, vectorf &com)
{
	com = (pt0+pt1+pt2+pt3)*0.25f;
	return fabsf((pt1-pt0^pt2-pt0)*(pt3-pt0))*(1.0f/6);
}

float CTriMesh::CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, vectorf &massCenter)
{
	geometry_under_test gtest;
	triangle tri,tri0;

	//GetRotationV0V1(vectorf(0,0,1),pplane->n).getmatrix(gtest.R.T());	//Q2M_IVO
	gtest.R = matrix3x3f(GetRotationV0V1(vectorf(0,0,1),pplane->n)).T();
	gtest.R *= pgwd->R;
	gtest.offset = pgwd->offset-pplane->origin;
	gtest.scale = pgwd->scale;
	int itri,i,j,imask,nAbove,iLow,iHigh,sign[3],nPieces;
	float t,V[4],Vaccum=0;
	vectorf com[4],com_accum(zero),pt[3];

	for(itri=0;itri<m_nTris;itri++) {
		PrepareTriangle(itri,&tri, &gtest);
		for(i=iLow=iHigh=0,nAbove=3; i<3; i++) {
			nAbove -= (sign[i] = isneg(tri.pt[i].z));
			imask = -isneg(tri.pt[i].z-tri.pt[iLow].z); iLow = i&imask | iLow&~imask;
			imask = -isneg(tri.pt[iHigh].z-tri.pt[i].z); iHigh = i&imask | iHigh&~imask;
		}
		for(i=j=0;i<3;i++) if (sign[i]^sign[inc_mod3[i]]) {
			t = -tri.pt[i].z/(tri.pt[inc_mod3[i]].z-tri.pt[i].z);
			pt[j++] = tri.pt[i]*(1.0f-t)+tri.pt[inc_mod3[i]]*t;
		}

		if (nAbove<2) {
			(tri0.pt[0] = tri.pt[0]).z = tri.pt[iHigh].z;
			(tri0.pt[1] = tri.pt[1]).z = tri.pt[iHigh].z;
			(tri0.pt[2] = tri.pt[2]).z = tri.pt[iHigh].z;
			V[0] = CalcPyramidVolume(tri.pt[iHigh],tri0.pt[inc_mod3[iHigh]],tri.pt[inc_mod3[iHigh]],tri0.pt[dec_mod3[iHigh]], com[0]);
			V[1] = CalcPyramidVolume(tri.pt[iHigh],tri0.pt[dec_mod3[iHigh]],tri.pt[dec_mod3[iHigh]],tri.pt[inc_mod3[iHigh]], com[1]);
			V[2] = fabsf((tri0.pt[1]-tri0.pt[0]^tri0.pt[2]-tri0.pt[0]).z*0.5f*tri0.pt[0].z);
			(com[2] = (tri0.pt[0]+tri0.pt[1]+tri0.pt[2])*(1.0f/3)).z = tri0.pt[0].z*0.5f;
			if (nAbove==1) {
				V[2] *= -1.0f; tri0.pt[iHigh].z = 0; nPieces = 4;
				V[3] = CalcPyramidVolume(tri.pt[iHigh],tri0.pt[iHigh],pt[0],pt[1], com[3]);
			}	else nPieces = 3;
		} else if (nAbove==2) {
			(tri0.pt[0] = tri.pt[iLow]).z = 0; nPieces = 1;
			V[0] = CalcPyramidVolume(tri.pt[iLow],tri0.pt[0],pt[0],pt[1], com[0]);
		} else nPieces = 0;

		sign[0] = -sgn(tri.n.z);
		for(i=0;i<nPieces;i++) {
			V[i] *= sign[0]; Vaccum += V[i]; com_accum += com[i]*V[i];
		}
	}

	if (Vaccum>0)
		massCenter = com_accum/Vaccum+pplane->origin;
	else massCenter.zero();

	return Vaccum;
}


void CTriMesh::CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, vectorf &dPres,vectorf &dLres)
{
	geometry_under_test gtest;
	triangle tri;
	gtest.R = pgwd->R;
	gtest.offset = pgwd->offset;
	gtest.scale = pgwd->scale;
	int itri;

	dPres.zero(); dLres.zero();
	for(itri=0;itri<m_nTris;itri++) {
		PrepareTriangle(itri,&tri, &gtest);
		CalcMediumResistance(tri.pt,3,tri.n, *pplane, pgwd->v,pgwd->w,pgwd->centerOfMass, dPres,dLres);
	}
}


int CTriMesh::DrawToOcclusionCubemap(const geom_world_data *pgwd, int iStartPrim,int nPrims, int iPass, int *pGrid[6],int nRes, 
																		 float rmin,float rmax,float zscale)
{
	vectorf pt[3];
	for(int i=iStartPrim;i<iStartPrim+nPrims;i++) {
		pt[0] = pgwd->R*m_pVertices[m_pIndices[i*3  ]]*pgwd->scale + pgwd->offset;
		pt[1] = pgwd->R*m_pVertices[m_pIndices[i*3+1]]*pgwd->scale + pgwd->offset;
		pt[2] = pgwd->R*m_pVertices[m_pIndices[i*3+2]]*pgwd->scale + pgwd->offset;
		RasterizePolygonIntoCubemap(pt,3, iPass, pGrid,nRes, rmin,rmax,zscale);
	}
	return nPrims;
}


void CTriMesh::DrawWireframe(void (*DrawLineFunc)(float*,float*), geom_world_data *gwd, int iLevel)
{
	if (iLevel==0) {
		vectorf pt[3]; int i,j;
		for(i=0;i<m_nTris;i++) {
			pt[0] = gwd->offset+gwd->R*m_pVertices[m_pIndices[i*3+0]]*gwd->scale;
			pt[1] = gwd->offset+gwd->R*m_pVertices[m_pIndices[i*3+1]]*gwd->scale;
			pt[2] = gwd->offset+gwd->R*m_pVertices[m_pIndices[i*3+2]]*gwd->scale;
			for(j=0;j<3;j++) if (pt[j].y<=pt[inc_mod3[j]].y)
				DrawLineFunc(pt[j],pt[inc_mod3[j]]);
		}
	} else {
		BV *pbbox;
		ResetGlobalPrimsBuffers();
		m_pTree->GetNodeBV(gwd->R,gwd->offset,gwd->scale, pbbox);
		DrawBBox(DrawLineFunc,gwd, m_pTree,(BBox*)pbbox,iLevel-1);
	}
}


void CTriMesh::GetMemoryStatistics(ICrySizer *pSizer)
{
	if (GetType()==GEOM_TRIMESH)
		pSizer->AddObject(this, sizeof(CTriMesh));
	m_pTree->GetMemoryStatistics(pSizer);

	{ SIZER_COMPONENT_NAME(pSizer, "mesh data");
		if (m_flags & 2) {
			pSizer->AddObject(m_pIndices, m_nTris*3*sizeof(m_pIndices[0]));
			if (m_pIds) pSizer->AddObject(m_pIds, m_nTris*sizeof(m_pIds[0]));
			pSizer->AddObject(m_pNormals, m_nTris*sizeof(m_pNormals[0]));
		}
		if (m_flags & 1)
			pSizer->AddObject(m_pVertices, m_nVertices*sizeof(m_pVertices[0]));
	}

	{ SIZER_COMPONENT_NAME(pSizer, "auxilary data");
		pSizer->AddObject(m_pTopology, m_nTris*sizeof(m_pTopology[0]));
		for(int i=0;i<m_nHashPlanes;i++) {
			pSizer->AddObject(m_pHashGrid[i], (m_hashgrid[i].size.x*m_hashgrid[i].size.y+1)*sizeof(m_pHashGrid[i][0]));
			pSizer->AddObject(m_pHashData[i], m_pHashGrid[i][m_hashgrid[i].size.x*m_hashgrid[i].size.y]*sizeof(m_pHashData[i][0]));
		}
	}
}

void CTriMesh::Save(CMemStream &stm)
{
	stm.Write(m_nVertices);
	stm.Write(m_nTris);
	stm.Write(m_nMaxVertexValency);
	stm.Write(m_flags);

	if (m_flags & 1) 
		stm.Write(m_pVertices, sizeof(m_pVertices[0])*m_nVertices);
	if (m_flags & 2) {
		stm.Write(m_pIndices, sizeof(m_pIndices[0])*m_nTris*3);
		if (m_pIds) {
			stm.Write(true);
			stm.Write(m_pIds, sizeof(m_pIds[0])*m_nTris);
		} else
			stm.Write(false);
	}
	stm.Write(m_pNormals, sizeof(m_pNormals)*m_nTris);

	int i;
	stm.Write(m_nHashPlanes);
	for(i=0;i<m_nHashPlanes;i++) {
		stm.Write(m_pHashGrid[i], (m_hashgrid[i].size.x*m_hashgrid[i].size.y+1)*sizeof(m_pHashGrid[i][0]));
		stm.Write(m_pHashData[i], m_pHashGrid[i][m_hashgrid[i].size.x*m_hashgrid[i].size.y]*sizeof(m_pHashData[i][0]));
	}

	for(i=0;i<4;i++) {
		stm.Write(m_bConvex[i]);
		stm.Write(m_ConvexityTolerance[i]);
	}

	stm.Write(m_pTree->GetType());
	stm.Write(m_pTopology, sizeof(m_pTopology[0])*m_nTris);
	m_pTree->Save(stm);
}

void CTriMesh::Load(CMemStream &stm)
{
	stm.Read(m_nVertices);
	stm.Read(m_nTris);
	stm.Read(m_nMaxVertexValency);
	stm.Read(m_flags);

	if (m_flags & 1) {
		m_pVertices = new vectorf[m_nVertices];
		stm.Read(m_pVertices, sizeof(m_pVertices[0])*m_nVertices);
	}
	if (m_flags & 2) {
		m_pIndices = new index_t[m_nTris*3];
		stm.Read(m_pIndices, sizeof(m_pIndices[0])*m_nTris*3);
		bool bIds; stm.Read(bIds);
		if (bIds) {
			m_pIds = new short[m_nTris];
			stm.Read(m_pIds, sizeof(m_pIds[0])*m_nTris);
		} else
			m_pIds = 0;
	}
	stm.Read(m_pNormals, sizeof(m_pNormals)*m_nTris);
	m_pTopology = new trinfo[m_nTris];
	stm.Read(m_pTopology, sizeof(m_pTopology[0])*m_nTris);

	int i;
	stm.Read(m_nHashPlanes);
	for(i=0;i<m_nHashPlanes;i++) {
		m_pHashGrid[i] = new index_t[m_hashgrid[i].size.x*m_hashgrid[i].size.y+1];
		stm.Read(m_pHashGrid[i], (m_hashgrid[i].size.x*m_hashgrid[i].size.y+1)*sizeof(m_pHashGrid[i][0]));
		m_pHashData[i] = new index_t[m_pHashGrid[i][m_hashgrid[i].size.x*m_hashgrid[i].size.y]];
		stm.Read(m_pHashData[i], m_pHashGrid[i][m_hashgrid[i].size.x*m_hashgrid[i].size.y]*sizeof(m_pHashData[i][0]));
	}

	for(i=0;i<4;i++) {
		stm.Read(m_bConvex[i]);
		stm.Read(m_ConvexityTolerance[i]);
	}

	int itype; stm.Read(itype);
	switch (itype) {
		case BVT_OBB: m_pTree = new COBBTree; break;
		case BVT_AABB: m_pTree = new CAABBTree; break;
		case BVT_SINGLEBOX: m_pTree = new CSingleBoxTree; break;
	}
	m_pTree->Load(stm,this);
}