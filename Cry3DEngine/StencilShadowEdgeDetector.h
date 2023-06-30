//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:StencilShadowEdgeDetector.h
//  Declaration of class CStencilShadowEdgeDetector
//
//	History:
//	-:Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////

// class CStencilShadowEdgeDetector
// AUTHOR: Sergiy Migdalskiy <sergiy@crytek.de>
//
// This is run-time (temporary, non-persistent) class that's used to construct
// the optimized shadow volume (optimized means only with the necessary boundary
// edges) for a concrete deformed model/character instance
//
// It ACCEPTS a Connectivity instance that must be pre-computed on per-model
// basis, and an array of deformed vertices.
// It OUTPUTS (constructs, prepares) a set of edges in the form prepared for
// fast construction of efficient shadow volume render data (vertex buffer)
//

#pragma once
#ifndef _STENCIL_SHADOW_EDGE_DETECTOR_HDR_
#define _STENCIL_SHADOW_EDGE_DETECTOR_HDR_

#include "StencilShadowConnectivity.h"																	// CStencilShadowConnectivity
#include "IEdgeConnectivityBuilder.h"																		// IEdgeDetector
#include "Cry3DEngineBase.h"

class CStencilShadowEdgeDetector : public IEdgeDetector, public Cry3DEngineBase
{
public:

	//! default constructor
	CStencilShadowEdgeDetector();

	//! destructor
	~CStencilShadowEdgeDetector(void);

	// Re-construct the whole object
	// Use this object to reuse the EdgeDetector object multiple times for different
	// connectivities and vertices. This can save on reallocating internal arrays this object uses at run time
	// /param pConnectivity, must not be 0 - don't use if there is nothing to do
	// /param pDeformedVertices, must not be 0 - don't use if there is nothing to do
	void reinit ( const IStencilShadowConnectivity* pConnectivity, const Vec3d* pDeformedVertices	);

	//! /param pConnectivity must not be 0
	//! /param invLightPos in ???-space (World or Object)
	//! /param pDeformedVertices must not be 0
	virtual void BuildSilhuetteFromPos( const IStencilShadowConnectivity* pConnectivity, const Vec3d &invLightPos, const Vec3d* pDeformedVertices );

	//! /param iniNumTriangles the size it should have, must not be 0 - don't use if there is nothing to do
	//! /return pointer to the cleared (set to zero) bitfield where each bit represents the orientation of one triangle
	virtual unsigned *getOrientationBitfield( int iniNumTriangles );

	//!
	//! /param pConnectivity must not be 0 - don't use if there is nothing to do
	//! /param inpVertices must not be 0 - don't use if there is nothing to do
	virtual void BuildSilhuetteFromBitfield( const IStencilShadowConnectivity* pConnectivity, const Vec3d* inpVertices );

	//! returns a pointer to the the given shadow edges (array of 2 integers, defining the vertex indices)
	//! /param
	//! /return
	virtual const vindex *getShadowEdgeArray( unsigned &outiNumEdges ) const;



	//! number of vertices used to define the detected shadow edges
	//! (at least nSE+1, at most nSE*2, where nSE is the number of shadow edges)
	//! This may be less than total number of vertices since the edges may use less
	//! vertices than faces
	//! /return
	_inline unsigned numShadowEdgeVertices() const
	{
		return m_numShadowEdgeVertices;
	}


	// number of vertices required to define the shadow volume,
	// use this to determine the size of vertex buffer passed to meshShadowVolume (2*used vertices because of capping)
	virtual unsigned numShadowVolumeVertices( void ) const
	{
		assert(m_pConnectivity);

		return(m_pConnectivity->numVertices()*2);
	}

#ifdef WIN64
#pragma warning( push )								//AMD Port
#pragma warning( disable : 4267 )
#endif

	// pointer to the triplets defining shadow faces
	virtual const vindex* getShadowFaceIndices( unsigned &outCount ) const
	{
		outCount=m_arrShadowFaces.size();

		return &m_arrShadowFaces[0];
	}

	

	// number of indices required to define the shadow volume,
	// use this to determine the size of index buffer passed to meshShadowVolume
	// this is always a dividend of 3
	virtual unsigned numShadowVolumeIndices()const
	{	
		// each shadow edge requires 6 indices to define the shadow volume:
		// it defines one quad => 2 triangles => 6 vertex references
		unsigned numShadowEdges=m_arrShadowEdges.size()/2;
		unsigned numShadowFaceIndices =m_arrShadowFaces.size();

		return(numShadowEdges*6 + numShadowFaceIndices*2);
	}

#ifdef WIN64
#pragma warning( pop )								//AMD Port
#endif

	//! fills in the internal array of edges and vertices with appropriate data
	//! call	computeFaceOrientations(vLight) before or create the triangle orientation bitfield yourself
	//! - detected shadow edges for the given light source and model
	void detectShadowEdges();

	//! fills in the internal array of faces and vertices
	//! - detected shadow faces for the given light source and model
	void detectShadowFaces ();


	//! calculates all face orientations into the bit array
	//! /param invLight position of the light source in ???-space (World or Object)
	void computeFaceOrientations (Vec3d invLight);



	// make up the shadow volume
	// constructs the shadow volume mesh, and puts the mesh definition into the 
	// vertex buffer (vertices that define the mesh) and index buffer (triple
	// integers defining the triangular faces, counterclockwise order)
	// The size of the vertex buffer must be at least numVertices()
	// The size of the index buffer must be at least numIndices()
	virtual void meshShadowVolume (Vec3d vLight, float fFactor, Vec3d* outpVertexBuf, unsigned short* pIndexBuf );

protected:
	const CStencilShadowConnectivity* m_pConnectivity;						//!<
	const Vec3d* m_pModelVertices;																//!<


	// number of vertices used to define edges
	unsigned m_numShadowEdgeVertices;

	// array of shadow edges, referring to the vertices in m_vVertices
	// Now, edges are defined by pairs of integers. This may change in the future
	std::vector <vindex> m_arrShadowEdges;

	// array of shadow faces, referring to the vertices in m_vVertices
	std::vector <vindex> m_arrShadowFaces;

	// returns true if the given face faces the light, and false otherwise
	bool IsFaceTurnedToLight( unsigned nFace, const Vec3d& vLight );

	// adds the given shadow edge to the list of boundary edges
	//! /param nVertex0 0..
	//! /param nVertex1 0..
	void AddEdge (vindex nVertex0, vindex nVertex1);

	// adds the given shadow face, in the order that is passed
	//! /param nVertex0 0..
	//! /param nVertex1 0..
	//! /param nVertex2 0..
	void AddFace (vindex nVertex0, vindex nVertex1, vindex nVertex2);

	// checks whether the given edge is boundary or reverse boundary
	enum EdgeTypeEnum {
		nET_Boundary,             // normal boundary edge
		nET_ReverseBoundary,      // internal boundary edge, needs reversing before insertion into the edge list
		nET_Exterior,             // visible to the light surface interior edge
		nET_Interior              // invisible to the light surface interior edge
	};

	//! checks the edge type by the face indices: assumes that the face orientation bits have been calculated already
	//! /param nFace0
	//! /param nFace1
	EdgeTypeEnum CheckEdgeType (unsigned nFace0, unsigned nFace1);

	//! /param nFace0
	EdgeTypeEnum CheckOrphanEdgeType (unsigned nFace0);

	// face orientation bit array: for each face, there's a bit, which is 1 if it's facing the light and 0 otherwise
	TFixedArray<unsigned> m_arrFaceOrientations;

	bool m_bBitFieldIsSet;					//! for debugging purpose
};

#endif