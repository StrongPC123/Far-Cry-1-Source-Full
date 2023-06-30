#include "stdafx.h"
#include "StencilShadowConnectivity.h"
#include "StencilShadowEdgeDetector.h"

CStencilShadowEdgeDetector::CStencilShadowEdgeDetector():
	m_pConnectivity(NULL),
	m_pModelVertices(NULL)
{
	m_bBitFieldIsSet=false;
}

// Re-construct the whole object
// Use this object to reuse the EdgeDetector object multiple times for different
// connectivities and vertices. This can save on reallocating internal arrays this object uses at run time
void CStencilShadowEdgeDetector::reinit (
	const IStencilShadowConnectivity* pConnectivity,
	const Vec3d* pDeformedVertices
)
{
	assert(pConnectivity);

	m_pConnectivity = pConnectivity->GetInternalRepresentation();
	m_pModelVertices = pDeformedVertices;

	if (!m_pModelVertices || m_pConnectivity->IsStandalone())
		m_pModelVertices = m_pConnectivity->getVertices();

	assert (pDeformedVertices);

	m_arrShadowEdges.clear();
	m_arrShadowFaces.clear();
	m_numShadowEdgeVertices = 0;
}

CStencilShadowEdgeDetector::~CStencilShadowEdgeDetector(void)
{
}




// fills in the internal array of faces and vertices
// - detected shadow faces for the given light source and model
void CStencilShadowEdgeDetector::detectShadowFaces ()
{
	assert(m_pConnectivity);
	if(!m_pConnectivity)
	{
#if !defined(LINUX)
		Warning(0,0,"CStencilShadowEdgeDetector::detectShadowFaces: !m_pConnectivity");
#endif
		return;
	}

	if(m_arrFaceOrientations.empty())
	{
#if !defined(LINUX)
		Warning(0,0,"CStencilShadowEdgeDetector::detectShadowFaces: m_arrFaceOrientations.empty()");
#endif
		return;
	}

	assert(!m_arrFaceOrientations.empty());

	// scan all faces and fill the backfacing ones
	unsigned nNumFaces = m_pConnectivity->numFaces();

	unsigned* pFaceOrientBits = m_arrFaceOrientations.begin();
	for (unsigned nFace = 0; nFace < nNumFaces; ++pFaceOrientBits)
	{
		unsigned nOrientBit = 1;
		do
		{
			if ((*pFaceOrientBits & nOrientBit) == 0)
			{
				// the face is back-facing the light
				const CStencilShadowConnectivity::Face& face = m_pConnectivity->getFace(nFace);
				// revert orientation of the face 'cause the volumizer expects light-facing capping faces
				AddFace(face.getVertex(0), face.getVertex(2), face.getVertex(1));
			}
		}
		while (((++nFace) < nNumFaces) && (nOrientBit <<= 1) != 0);
	}

	m_bBitFieldIsSet=true;
}


// returns true if the given face faces the light, and false otherwise
bool CStencilShadowEdgeDetector::IsFaceTurnedToLight (unsigned nFace, const Vec3d& vLight)
{
	assert(m_pModelVertices);
	assert(m_pConnectivity);
	assert(nFace>=0 && nFace<m_pConnectivity->numFaces());


	if (m_pConnectivity->hasPlanes())
		return m_pConnectivity->getPlane(nFace).apply (vLight) > 0;


	const CStencilShadowConnectivity::Face& face = m_pConnectivity->getFace(nFace);

	DWORD dwIndex[3]={ face.getVertex(0),
										 face.getVertex(1),
										 face.getVertex(2) };

	const Vec3d& vFaceV0 = m_pModelVertices[dwIndex[0]];

	Vec3d vNormal = (m_pModelVertices[dwIndex[1]] - vFaceV0) ^ (m_pModelVertices[dwIndex[2]] - vFaceV0);

	return vNormal * (vLight - vFaceV0) > 0;
}
//////////////////////////////////////////////////////////////////////
// Calculates all face orientations into the bit array
// PARAMETERS:
//   vLight - position of the light source to detect the edges for
//////////////////////////////////////////////////////////////////////
void CStencilShadowEdgeDetector::computeFaceOrientations (Vec3d vLight)
{
	unsigned nModelNumFaces = m_pConnectivity->numFaces();

	// mask that masks out the bits that address the bit in an unsigned
	//const nBitIndexMask = (sizeof(m_arrFaceOrientations[0])*8) - 1;

	// number of unsigneds to allocate in the bit array
	unsigned nBitarraySize = (nModelNumFaces + 31u) >> 5;
	if (m_arrFaceOrientations.size() < nBitarraySize)
		m_arrFaceOrientations.reinit(nBitarraySize);

	memset (m_arrFaceOrientations.begin(), 0, nBitarraySize * 4);		// neccessary because we set only the 1 bits

	// scan through all faces, in packets of 32 faces, and pack the orientations into
	// bit array with 32-bit portions
	unsigned nNumFaces = m_pConnectivity->numFaces();
	unsigned* pFaceOrientBits = m_arrFaceOrientations.begin();
	for (unsigned nFace = 0; nFace < nNumFaces; ++pFaceOrientBits)
	{
		unsigned nOrientBit = 1;
		do
		{
			if (IsFaceTurnedToLight (nFace, vLight))
				*pFaceOrientBits |= nOrientBit;
		}
		while (((++nFace) < nNumFaces) && (nOrientBit <<= 1) != 0);
	}

	m_bBitFieldIsSet=true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fills in the internal array of edges and vertices with appropriate data
// call	computeFaceOrientations(vLight) before or create the triangle orientation bitfield yourself
// - detected shadow edges for the given light source and model
// ALGORITHM:
//  scans each edge and determines signs of volumes formed with this edge and light source
//  on one hand and the opposite vertex on the other hand
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStencilShadowEdgeDetector::detectShadowEdges( void )
{
	assert(m_bBitFieldIsSet);		// call	computeFaceOrientations(vLight) before or create the triangle orientation bitfield yourself

	// number of vertices in the original model
	unsigned nModelNumVertices = m_pConnectivity->numVertices();

	//
	// for each edge, insert it into m_arrShadowEdges if it's boundary;
	// insert it in reverse order if it's reverse-boundary
	// insert the vertex/-ices into the m_arrVertices array if necessary
	//
	// TODO: perhaps an optimization is possible, if we detect adjacent edges
	// and then render triangle strips without indices
	//
	
	unsigned nEdge;
	unsigned nEdgeCount = m_pConnectivity->numEdges();
	for (nEdge = 0; nEdge < nEdgeCount; ++nEdge)
	{
		// this is the edge to check against being boundary
		const CStencilShadowConnectivity::Edge& rEdge = m_pConnectivity->getEdge(nEdge);
		
		// check the edge
		switch (CheckEdgeType (
			rEdge.getFace(0).getFaceIndex(),
			rEdge.getFace(1).getFaceIndex()
		))
		{
		case nET_Boundary:
			AddEdge (rEdge[0], rEdge[1]);
			break;
		case nET_ReverseBoundary:
			AddEdge (rEdge[1], rEdge[0]);
			break;
		}
	}

	// now check the orphan edges
	nEdgeCount = m_pConnectivity->numOrphanEdges();
	for (nEdge = 0; nEdge < nEdgeCount; ++nEdge)
	{
		// this is the edge to check against being boundary
		const CStencilShadowConnectivity::OrphanEdge& rEdge = m_pConnectivity->getOrphanEdge(nEdge);
		
		// check the edge
		switch (CheckOrphanEdgeType (
			rEdge.getFace().getFaceIndex()
		))
		{
		case nET_Boundary:
			AddEdge (rEdge[0], rEdge[1]);
			break;
		case nET_ReverseBoundary:
			AddEdge (rEdge[1], rEdge[0]);
			break;
		}
	}

	m_bBitFieldIsSet=false;

	m_numShadowEdgeVertices = m_pConnectivity->numVertices();
}


////////////////////////////////////////////////////////////////////////
// Adds the given shadow edge to the list of boundary edges
// PARAMETERS:
//   nVertex[2] - indices of the edge vertices in the m_pModelVertices
//                array (and in g_arrModelVtxIdxMap).
////////////////////////////////////////////////////////////////////////
void CStencilShadowEdgeDetector::AddEdge( vindex nVertex0, vindex nVertex1 )
{
	// add to the list of edges
	m_arrShadowEdges.push_back(nVertex0);
	m_arrShadowEdges.push_back(nVertex1);
}


// adds the given shadow face, in the order that is passed
void CStencilShadowEdgeDetector::AddFace( vindex nVertex0, vindex nVertex1, vindex nVertex2 )
{
	// add to the list of faces
	m_arrShadowFaces.push_back(nVertex0);
	m_arrShadowFaces.push_back(nVertex1);
	m_arrShadowFaces.push_back(nVertex2);
}





// checks the edge type by the face indices: assumes that the face orientation bits have been calculated already
CStencilShadowEdgeDetector::EdgeTypeEnum 
	CStencilShadowEdgeDetector::CheckEdgeType (unsigned nFace0, unsigned nFace1)
{
	assert(m_arrFaceOrientations.size());

	if ((m_arrFaceOrientations[nFace0>>5]&(1<<(nFace0&0x1F))) == 0)
	{
		// the primary edge face is culled against the light
		if ((m_arrFaceOrientations[nFace1>>5]&(1<<(nFace1&0x1F))) == 0)
		{
			// the opposite face is culled against the light,
			// both are back-facing the light
			return nET_Interior;
		}
		else
		{
			// the opposite face is facing the light
			// we've got the rev-boundary edge
			return nET_ReverseBoundary;
		}
	}
	else
	{
		// the primary edge face is facing the light
		if ((m_arrFaceOrientations[nFace1>>5]&(1<<(nFace1&0x1F))) == 0)
		{
			// the opposite face is culled against the light,
			// we've got the boundary edge
			return nET_Boundary;
		}
		else
		{
			// both are facing the light
			return nET_Exterior;
		}
	}
}

CStencilShadowEdgeDetector::EdgeTypeEnum 
	CStencilShadowEdgeDetector::CheckOrphanEdgeType (unsigned nFace0)
{
	assert(m_arrFaceOrientations.size());

	if ((m_arrFaceOrientations[nFace0>>5]&(1<<(nFace0&0x1F))) == 0)
	{
		// the primary edge face is culled against the light

		// we can either ignore the face and return nET_Interior
		// or we can pretend we have a complementary face - so that the face casts
		// the shadow with both its sides. then, return nET_ReverseBoundary

		return nET_Interior;
		//return nET_ReverseBoundary;
	}
	else
	{
		// the primary edge face is facing the light
		// since we don't have any neighbor to check, pretend as if the neighbor were back-facing the light
		return nET_Boundary;
	}
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

const CStencilShadowEdgeDetector::vindex *CStencilShadowEdgeDetector::getShadowEdgeArray( unsigned &outiNumEdges ) const
{
	outiNumEdges=m_arrShadowEdges.size()/2;

	if(!outiNumEdges)return(0);

	return(&m_arrShadowEdges[0]);
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

void CStencilShadowEdgeDetector::BuildSilhuetteFromPos( const IStencilShadowConnectivity* pConnectivity, const Vec3d &invLightPos, 
																										 const Vec3d* inpDeformedVertices )
{
	assert(pConnectivity);

	m_pConnectivity = pConnectivity->GetInternalRepresentation();
	m_pModelVertices = m_pConnectivity->getVertices();
	if ( !pConnectivity->IsStandalone())
	{
		if (!m_pModelVertices)
			m_pModelVertices = inpDeformedVertices;
	}

	assert(m_pModelVertices);

	m_arrShadowEdges.clear();
	m_arrShadowFaces.clear();
	m_numShadowEdgeVertices = 0;	

	computeFaceOrientations(invLightPos);

	detectShadowEdges();		// call computeFaceOrientations(vLight) before or create the triangle orientation bitfield yourself
	detectShadowFaces();

//	assert(m_bBitFieldIsSet);
	if(!m_bBitFieldIsSet)
		Warning(0,0,"CStencilShadowEdgeDetector::BuildSilhuetteFromPos: !m_bBitFieldIsSet");
}


void CStencilShadowEdgeDetector::BuildSilhuetteFromBitfield( const IStencilShadowConnectivity* pConnectivity, const Vec3d* inpVertices )
{
	assert(pConnectivity);

	assert(m_bBitFieldIsSet);		// call	computeFaceOrientations(vLight) before or create the triangle orientation bitfield yourself

	m_pConnectivity = pConnectivity->GetInternalRepresentation();
	m_pModelVertices = m_pConnectivity->getVertices();
	if (!m_pModelVertices)
		m_pModelVertices = inpVertices;

	assert(m_pModelVertices);

	m_arrShadowEdges.clear();
	m_arrShadowFaces.clear();
	m_numShadowEdgeVertices = 0;	

	// triangle orientation bitfield has to be provided by calling getOrientationBitfield
	detectShadowEdges();	// call computeFaceOrientations(vLight) before or create the triangle orientation bitfield yourself
	detectShadowFaces();

	assert(m_bBitFieldIsSet);		// call	computeFaceOrientations(vLight) before or create the triangle orientation bitfield yourself
}


//!
unsigned *CStencilShadowEdgeDetector::getOrientationBitfield( int iniNumTriangles )
{
	assert(iniNumTriangles);

	// mask that masks out the bits that address the bit in an unsigned
	//const nBitIndexMask = (sizeof(m_arrFaceOrientations[0])*8) - 1;

	// number of unsigneds to allocate in the bit array
	unsigned nBitarraySize = (iniNumTriangles + 31u) >> 5;
	if (m_arrFaceOrientations.size() < nBitarraySize)
		m_arrFaceOrientations.reinit(nBitarraySize);

	m_bBitFieldIsSet=true;

	memset (m_arrFaceOrientations.begin(), 0, nBitarraySize * 4);		// neccessary because we set only the 1 bits

	return(m_arrFaceOrientations.begin());
}



//////////////////////////////////////////////////////////////////////////////
// make up the shadow volume
// constructs the shadow volume mesh, and puts the mesh definition into the 
// vertex buffer (vertices that define the mesh) and index buffer (triple
// integers defining the triangular faces, counterclockwise order)
// The size of the vertex buffer must be at least numVertices()
// The size of the index buffer must be at least numIndices()
//////////////////////////////////////////////////////////////////////////////
void CStencilShadowEdgeDetector::meshShadowVolume( Vec3d vLight, float fFactor, Vec3d* outpVertexBuf, unsigned short* pIndexBuf )
{
	assert(outpVertexBuf);

	// The Algorithm
	// We have the edges and their vertex array in m_pShadowEdges
	// For each edge, we generate a quad (2 trianges), out of the pair of vertices defining the edge:
	//    take the vertex, move it away from the light (this makes up the far vertex), this will define an extruded edge -- our actual quad

	// 1. scan through all shadow edges, insert corresponding generated vertex indices into the index buffer;
	// 2. scan through all vertices, generate far vertices (for each near vertex, there's a far vertex), fill in the vertex buffer
	unsigned nNumVerts;
	unsigned i;

	nNumVerts = numShadowVolumeVertices()/2;
	const Vec3d* pVertices = m_pModelVertices;

	// go thougth all compressed vertices
	{
		Vec3d vLightFac=(1.0f-fFactor)*vLight;		// optimized a little

		for(i=0;i<nNumVerts;++i)
		{
			const Vec3d& vPos = pVertices[i];
			outpVertexBuf[i] = vPos;
			outpVertexBuf[nNumVerts+i] = vPos*fFactor + vLightFac; // == (vPos - vLight) * fFactor + vLight
		}
	}
	

	unsigned short* pIndex = pIndexBuf;
	unsigned nNumEdges;
	const unsigned short* pEdgeArray = getShadowEdgeArray(nNumEdges);
	
	// fill in the shadow volume quads (extruded vertices)
	for (i = 0; i < nNumEdges; ++i)
	{
		unsigned short Edge1=*pEdgeArray++,Edge2=*pEdgeArray++;

		assert (Edge1 < nNumVerts);
		assert (Edge2 < nNumVerts);

		*(pIndex++) = Edge2;
		*(pIndex++) = Edge1;
		*(pIndex++) = Edge1 + nNumVerts;

		*(pIndex++) = Edge1 + nNumVerts;
		*(pIndex++) = Edge2 + nNumVerts;
		*(pIndex++) = Edge2;
	}

	if(m_arrShadowFaces.empty())
		return; // vlad: otherwise crash

	// fill in the shadow volume caps 
	unsigned nFaceIndices;
	const unsigned short* pFaceIndex = getShadowFaceIndices(nFaceIndices);
	const unsigned short* pFaceIndexEnd = pFaceIndex + nFaceIndices;
	// fill the near cap
	memcpy (pIndex, pFaceIndex, sizeof(*pFaceIndex)*nFaceIndices);

	pIndex += nFaceIndices;

	// fill the far cap

	for (; pFaceIndex < pFaceIndexEnd; pFaceIndex += 3)
	{
		// for each face..
		assert (pFaceIndex[0] < nNumVerts);
		assert (pFaceIndex[1] < nNumVerts);
		assert (pFaceIndex[2] < nNumVerts);

		*(pIndex++) = pFaceIndex[0] + nNumVerts;
		*(pIndex++) = pFaceIndex[2] + nNumVerts;
		*(pIndex++) = pFaceIndex[1] + nNumVerts;
	}

	assert(numShadowVolumeVertices()==nNumVerts*2);
	assert(numShadowVolumeIndices()==(int)(pIndex-pIndexBuf));
}
