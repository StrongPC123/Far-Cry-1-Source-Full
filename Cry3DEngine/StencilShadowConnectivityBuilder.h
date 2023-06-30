//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:StencilShadowConnectivityBuilder.h
//  Declaration of class CStencilShadowConnectivityBuilder
//
//	History:
//	-:Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////

// class CStencilShadowConnectivityBuilder
//
// This class is used to pre-compute the data for a model that's gonna cast stencil
// shadows, and whose vertices may change position, but the topology
// cannot change.
//
// Ok, to put it clear: for each model, you need one instance of
// class CStencilShadowConnectivity. To create that instance, use
// this class: create an object, push the mesh of the model into it,
// then ask this class to give out the CStencilShadowConnectivity.
//
// CStencilShadowConnectivity is build for maximum run-time performance and
// minimal memory footprint, whilst this class is build for convenient
// pre-calculations and flexibility.
// 
// The model is allowed to deform, but is not
// allowed to change topology (add or remove faces as well as re-assign
// face vertices or even change face orientation).
/////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION NOTES should be in
// StencilShadowConnectivityBuilder.cpp
//

#pragma once
#ifndef _STENCIL_SHADOW_CONNECTIVITY_BUILDER_HDR_
#define _STENCIL_SHADOW_CONNECTIVITY_BUILDER_HDR_

// the connectivity class is required for the declarations
// of the Face, Edge, EdgeFace and BasicEdge structures which are common for both
// the builder and the built class
#include "IEdgeConnectivityBuilder.h"										// IEdgeConnectivityBuilder
#include "StencilShadowConnectivity.h"									// CStencilShadowConnectivity

#include <map>																					// STL map<>
#include <functional>
#include <algorithm>


class CStencilShadowConnectivityBuilder : public IEdgeConnectivityBuilder
{
public:
	typedef IStencilShadowConnectivity::vindex vindex;
	//! default constructor
	//! creates an empty mesh connectivity
	CStencilShadowConnectivityBuilder(void);
	
	//! destructor
	virtual ~CStencilShadowConnectivityBuilder(void);

	//! return to the state right after construction
	virtual void Reinit( void );

  // sets the maximum distance between two points that are allowed to be welded
	//virtual void SetWeldTolerance (float fTolerance) {m_fWeldTolerance = fTolerance*fTolerance;}

	// reserves space for the given number of triangles that are to be added
	//! /param nNumTriangles 0..
	//! /param innNumVertices 0..
	virtual void ReserveForTriangles( unsigned nNumTriangles, unsigned innNumVertices );

	// adds a single triangle to the mesh
	// the triangle is defined by three vertices, in counter-clockwise order
	// these vertex indices will be used later when accessing the array of
	// deformed character/model vertices to determine the shadow volume boundary
	//! /param nV0 vertex index one 0..0xffff
	//! /param nV1 vertex index two 0..0xffff
	//! /param nV2 vertex index three 0..0xffff
	//! /param inpVertexPos pointer to the vertex array (to remove vertices on same position)
	virtual void AddTriangle( vindex nV0, vindex nV1, vindex nV2 );

	//!
	//! /param nV0 vertex index one 0..0xffff
	//! /param nV1 vertex index two 0..0xffff
	//! /param nV2 vertex index three 0..0xffff
	//! /param vV0 original vertex one position
	//! /param vV1 original vertex two position 
	//! /param vV2 original vertex three position 
	//! slower but with the auto weld feature (if there are vertices with the same position your result is smaller and therefore faster)
	virtual void AddTriangleWelded( vindex nV0, vindex nV1, vindex nV2, const Vec3d &vV0, const Vec3d &vV1, const Vec3d &vV2 );

	// constructs/compiles the optimum representation of the connectivity
	// to be used in run-time
	// WARNING: use Release method to dispose the connectivity object
	//! /param inpVertexBuf vertex position buffer to check for solvable open edges (2 vertices with same position)
	//! /return interface pointer, could be 0
	virtual class IStencilShadowConnectivity *ConstructConnectivity( void );

	// returns the number of single (with no pair faces found) or orphaned edges
	// /return 0..
	virtual unsigned numOrphanedEdges ()const;

	//! Returns the list of faces for orphaned edges into the given buffer;
	//! For each orphaned edge, one face will be returned; some faces may be duplicated
	virtual void getOrphanedEdgeFaces (unsigned* pBuffer);


protected:

	// for the descriptions of the following shared structures, refer to
	// File:StencilShadowConnectivity.h
	typedef CStencilShadowConnectivity::BasicEdge BasicEdge;
	typedef CStencilShadowConnectivity::Edge Edge;
	typedef CStencilShadowConnectivity::EdgeFace EdgeFace;
	typedef CStencilShadowConnectivity::Face Face;

	// map of single edges - edge with only one face attached
	typedef std::map<BasicEdge, EdgeFace> SingleEdgeMap;
	SingleEdgeMap m_mapSingleEdges;																			//!<

	// array of double-edges - edges with both faces attached/found
	typedef std::vector<Edge> DoubleEdgeArray;
	DoubleEdgeArray m_vDoubleEdges;																			//!<

	// triangles added (used to extract index to reference edge to faces, and keep the topology)
	typedef std::vector<Face> FaceArray;
	FaceArray m_vFaces;																					//!<
	DWORD	m_dwNumUncompressedVertices;																	//!<

	// helper to get order for Vec3d
	struct CVec3dOrder: public std::binary_function< Vec3d, Vec3d, bool>
	{
		bool operator() ( const Vec3d &a, const Vec3d &b ) const
		{
			// first sort by x
			if(a.x<b.x)return(true);
			if(a.x>b.x)return(false);

			// then by y
			if(a.y<b.y)return(true);
			if(a.y>b.y)return(false);

			// then by z
			if(a.z<b.z)return(true);
			if(a.z>b.z)return(false);

			return(false);
		}
	};

	typedef std::map<Vec3d,unsigned,CVec3dOrder> VertexWelderMap;
	VertexWelderMap m_mVertexWelder;						//!< used for AddTriangleWelded

	// this will try to find a close match to the given vertex, and
	// if found, return its index (the actual index of the vertex that's very close or
	// coincide with v in space). Otherwise, creates a new vertex reference in the map
	// and returns the index nNewVertex
	unsigned WeldVertex (const Vec3d& v, unsigned nNewVertex);

	//float m_fWeldTolerance;
	// this is 1 + the max index of the vertex in the original mesh vertex array, that
	// is used by the currently being built connectivity
	//unsigned m_numOrigMeshVerticesUsed; // this is unneeded because connectivity calculates this itself
protected:

	// adds a new single edge, or finds an adjacent edge and puts the double-edge on record
	// add a new edge, if there is no complementary single edge;
	// otherwise, withdraw the edge from the list of single edges and add to double edges
	//! /param eEdge
	//! /param efFace
	void AddNewEdge (BasicEdge eEdge, EdgeFace efFace);
};

// this is the builder of connectivity that can be used for static objects
// (with non-changing face normals)
class CStencilShadowStaticConnectivityBuilder :public CStencilShadowConnectivityBuilder
{
public:
	CStencilShadowStaticConnectivityBuilder();
	virtual ~CStencilShadowStaticConnectivityBuilder();
	void AddTriangleWelded( vindex nV0, vindex nV1, vindex nV2, const Vec3d &vV0, const Vec3d &vV1, const Vec3d &vV2 );
	//! return to the state right after construction
	virtual void Reinit( void );

	// reserves space for the given number of triangles that are to be added
	//! /param nNumTriangles 0..
	//! /param innNumVertices 0..
	virtual void ReserveForTriangles( unsigned nNumTriangles, unsigned innNumVertices );

	// constructs/compiles the optimum representation of the connectivity
	// to be used in run-time
	// WARNING: use Release method to dispose the connectivity object
	//! /param inpVertexBuf vertex position buffer to check for solvable open edges (2 vertices with same position)
	//! /return interface pointer, could be 0
	virtual class IStencilShadowConnectivity *ConstructConnectivity( void );
protected:
	// WARNING: This is only to be called when the whole construction process
	//   is finished, to modify already created connectivity
	// Goes through all vertices used by the connectivity and constructs a continuous
	// array out of them; reindexes the vertices in the connectivity object
	// to use these vertices; puts the new vertex array into the connectivity object
	void SetVertices (CStencilShadowConnectivity * pConnectivity);

	typedef CStencilShadowConnectivity::Plane Plane;
	std::vector<Plane> m_vPlanes;
};

#endif // _STENCIL_SHADOW_CONNECTIVITY_BUILDER_HDR_