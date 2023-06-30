#include "stdafx.h"																				// precompiled header
#include "StencilShadowConnectivity.h"
#include "StencilShadowConnectivityBuilder.h"

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

// creates an empty mesh connectivity
CStencilShadowConnectivityBuilder::CStencilShadowConnectivityBuilder( void )
#ifdef DEBUG_STD_CONTAINERS
:m_mapSingleEdges("CStencilShadowConnectivityBuilder.SingleEdges")
,m_vDoubleEdges ("CStencilShadowConnectivityBuilder.DoubleEdges")
,m_vFaces("CStencilShadowConnectivityBuilder.Faces")
,m_mVertexWelder("CStencilShadowConnectivityBuilder.VertexWelder")
#endif
{
	Reinit();
}

CStencilShadowConnectivityBuilder::~CStencilShadowConnectivityBuilder( void )
{
}

void CStencilShadowConnectivityBuilder::Reinit( void )
{
	m_mapSingleEdges.clear();
	m_vDoubleEdges.clear();
	m_vFaces.clear();
	m_mVertexWelder.clear();
	m_dwNumUncompressedVertices=0;
//	m_fWeldTolerance = 0;
}

///////////////////////////////////////////////////////////////////////////////////
// adds a single triangle to the mesh
// the tirangle is defined by three vertices, in counter-clockwise order
// these vertex indices will be used later when accessing the array of
// deformed character/model vertices to determine the shadow volume boundary
///////////////////////////////////////////////////////////////////////////////////
void CStencilShadowConnectivityBuilder::AddTriangle( unsigned short nV0, unsigned short nV1, unsigned short nV2 )
{
	unsigned nNumFaces = m_vFaces.size();
	AddNewEdge (BasicEdge(nV0, nV1), EdgeFace(nNumFaces, nV2));
	AddNewEdge (BasicEdge(nV1, nV2), EdgeFace(nNumFaces, nV0));
	AddNewEdge (BasicEdge(nV2, nV0), EdgeFace(nNumFaces, nV1));
	m_vFaces.push_back(Face (nV0, nV1, nV2));
}


// this will try to find a close match to the given vertex, and
// if found, return its index (the actual index of the vertex that's very close or
// coincide with v in space). Otherwise, creates a new vertex reference in the map
// and returns the index nNewVertex
unsigned CStencilShadowConnectivityBuilder::WeldVertex (const Vec3d& v, unsigned nNewVertex)
{
	// The easiest way: just find it directly
	VertexWelderMap::iterator it;
	it = m_mVertexWelder.find(v);
	if (it != m_mVertexWelder.end())
		return it->second;
	else
	{
		/*
		// scan and find some very close vertex
		if (m_fWeldTolerance > 0)
			for (it = m_mVertexWelder.begin(); it != m_mVertexWelder.end(); ++it)
			{
				if ((it->first-v).len2() < m_fWeldTolerance)
					return it->second;
			}
		*/
		m_mVertexWelder.insert (VertexWelderMap::value_type(v, nNewVertex));
		return nNewVertex;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Adds the triangle with the given vertex indices and vertex coordinates
// to the list of triangles that will cast shadows. Welds the vertices based on their
// coordinates
void CStencilShadowConnectivityBuilder::AddTriangleWelded(
	unsigned short nV0, unsigned short nV1, unsigned short nV2, 
	const Vec3d &vV0, const Vec3d &vV1, const Vec3d &vV2 )
{
	nV0=WeldVertex(vV0,nV0);
	nV1=WeldVertex(vV1,nV1);
	nV2=WeldVertex(vV2,nV2);

	AddTriangle(nV0,nV1,nV2);
}


//////////////////////////////////////////////////////////////////////////////////////
// add a new edge, if there is no complementary single edge;
// otherwise, withdraw the edge from the list of single edges and add to double edges
// PARAMETERS:
//   edge - start and end vertices of the edge going CCW along the face
//   efFace - the opposite vertex/face the edge belongs to
//////////////////////////////////////////////////////////////////////////////////////
void CStencilShadowConnectivityBuilder::AddNewEdge (BasicEdge eEdge, EdgeFace efFace)
{
	// first, try to find the complementary
	BasicEdge eComplementary (eEdge[1], eEdge[0]);

	SingleEdgeMap::iterator itComplementary = m_mapSingleEdges.find (eComplementary);

	if (itComplementary == m_mapSingleEdges.end())
	{
		// complementeary edge not found. Add a new single edge

		m_mapSingleEdges.insert (SingleEdgeMap::value_type(eEdge, efFace));

		// nVEdgeG for nFaceVertex[1] is unknown and doesn't matter at the moment
	}
	else
	{
		// we found the complementary edge
		Edge edgeNewDouble (eComplementary, itComplementary->second, efFace);

		m_vDoubleEdges.push_back (edgeNewDouble);

		m_mapSingleEdges.erase (itComplementary);
	}
}


///////////////////////////////////////////////////////////////////////////////////
// constructs/compiles the optimum representation of the connectivity
// to be used in run-time
// WARNING: use Release method to dispose the connectivity object
///////////////////////////////////////////////////////////////////////////////////
IStencilShadowConnectivity *CStencilShadowConnectivityBuilder::ConstructConnectivity( void )
{
	CStencilShadowConnectivity* pConnectivity = new CStencilShadowConnectivity(m_vDoubleEdges);

	assert(pConnectivity);		// low memeory?

	pConnectivity->SetOrphanEdges(m_mapSingleEdges);
	pConnectivity->SetFaces(m_vFaces);

	// find all the used vertices
	FaceArray::const_iterator pFace = m_vFaces.begin();
	DWORD nFaceCount = m_vFaces.size();

#if 0
	// test for serialization function
	unsigned nBufSize = pConnectivity->Serialize(true, NULL, 0);
	void* pBuf = malloc (nBufSize);
	unsigned nSaved = pConnectivity->Serialize(true, pBuf, nBufSize);
	assert (nSaved == nBufSize);
	pConnectivity->Release();
	pConnectivity = new CStencilShadowConnectivity ();
	unsigned nLoaded = pConnectivity->Serialize(false, pBuf, nBufSize);
	assert (nLoaded == nBufSize);
	free (pBuf);
#endif

	return((IStencilShadowConnectivity *)pConnectivity);
}



// returns the number of single (with no pair faces found) or orphaned edges
unsigned CStencilShadowConnectivityBuilder::numOrphanedEdges ()const
{
	return(m_mapSingleEdges.size());
}

//! Returns the list of faces for orphaned edges into the given buffer;
//! For each orphaned edge, one face will be returned; some faces may be duplicated
void CStencilShadowConnectivityBuilder::getOrphanedEdgeFaces (unsigned* pBuffer)
{
	for (SingleEdgeMap::iterator it = m_mapSingleEdges.begin(); it != m_mapSingleEdges.end(); ++it)
		*(pBuffer++) = it->second.m_nFace;
}



// reserves space for the given number of triangles that are to be added
void CStencilShadowConnectivityBuilder::ReserveForTriangles( unsigned nNumTriangles, unsigned innNumVertices )
{
	m_vDoubleEdges.reserve(nNumTriangles*3/2);
	m_vFaces.reserve(nNumTriangles);

	m_dwNumUncompressedVertices=innNumVertices;
}


CStencilShadowStaticConnectivityBuilder::CStencilShadowStaticConnectivityBuilder()
#ifdef DEBUG_STD_CONTAINERS
: m_vPlanes("CStencilShadowStaticConnectivityBuilder.Planes")
#endif
{
}

CStencilShadowStaticConnectivityBuilder::~CStencilShadowStaticConnectivityBuilder()
{
}

// reserves space for the given number of triangles that are to be added
void CStencilShadowStaticConnectivityBuilder::ReserveForTriangles( unsigned nNumTriangles, unsigned innNumVertices )
{
	CStencilShadowConnectivityBuilder::ReserveForTriangles (nNumTriangles, innNumVertices);
	m_vPlanes.reserve (nNumTriangles);
}

//! return to the state right after construction
void CStencilShadowStaticConnectivityBuilder::Reinit( void )
{
	CStencilShadowConnectivityBuilder::Reinit();
	m_vPlanes.clear();
}

//////////////////////////////////////////////////////////////////////////
// Adds the triangle with the given vertex indices and vertex coordinates
// to the list of triangles that will cast shadows. Welds the vertices based on their
// coordinates. Calculates and remembers the normals for each triangle
void CStencilShadowStaticConnectivityBuilder::AddTriangleWelded(
	vindex nV0, vindex nV1, vindex nV2, 
	const Vec3d &vV0, const Vec3d &vV1, const Vec3d &vV2 )
{
	CStencilShadowConnectivityBuilder::AddTriangleWelded (nV0, nV1, nV2, vV0, vV1, vV2);
	m_vPlanes.push_back(Plane (vV0,vV1,vV2));
}

// constructs/compiles the optimum representation of the connectivity
// to be used in run-time
// WARNING: use Release method to dispose the connectivity object
//! /param inpVertexBuf vertex position buffer to check for solvable open edges (2 vertices with same position)
//! /return interface pointer, could be 0
class IStencilShadowConnectivity *CStencilShadowStaticConnectivityBuilder::ConstructConnectivity ()
{
	if (m_vPlanes.empty())
		return new CStencilShadowConnectivity();

	CStencilShadowConnectivity * pConnectivity = (CStencilShadowConnectivity *)CStencilShadowConnectivityBuilder::ConstructConnectivity()->GetInternalRepresentation();
	pConnectivity->SetPlanes(&m_vPlanes[0], m_vPlanes.size());

	SetVertices (pConnectivity);
	return pConnectivity;
};

//////////////////////////////////////////////////////////////////////////
// WARNING: This is only to be called when the whole construction process
//   is finished, to modify already created connectivity
// Goes through all vertices used by the connectivity and constructs a continuous
// array out of them; reindexes the vertices in the connectivity object
// to use these vertices; puts the new vertex array into the connectivity object
void CStencilShadowStaticConnectivityBuilder::SetVertices (CStencilShadowConnectivity * pConnectivity)
{
	// scan through all used vertices and put them into continuous array
	std::vector<Vec3d> arrNewVertices; // the vertices array in new indexation
	std::vector<CStencilShadowConnectivity::vindex> arrOldToNewMap; // the mapping old->new indexation
	arrNewVertices.reserve (pConnectivity->numVertices());
	arrOldToNewMap.resize (pConnectivity->numVertices(),-1);
	for (VertexWelderMap::const_iterator it = m_mVertexWelder.begin(); it != m_mVertexWelder.end(); ++it)
	{
#if 1
		// this will remap the vertex indices
		arrOldToNewMap[it->second] = arrNewVertices.size();
		arrNewVertices.push_back(it->first);
#else
		// this will not change indexation
		arrOldToNewMap[it->second] = it->second;
		if (arrNewVertices.size() <= it->second)
			arrNewVertices.resize (it->second+1, Vec3d(0,0,0));
		arrNewVertices[it->second] = it->first;
#endif
	}

	// now renumber the vertices
	pConnectivity->SetRemapVertices(&arrOldToNewMap[0], arrOldToNewMap.size(), &arrNewVertices[0], arrNewVertices.size());
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif
