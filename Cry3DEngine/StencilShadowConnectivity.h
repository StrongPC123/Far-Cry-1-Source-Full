//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:StencilShadowConnectivity.h
//  Declaration of class CStencilShadowConnectivity
//
//	History:
//	           -:Created by Sergiy Migdalskiy <sergiy@crytek.de>
//  08/30/2002 include .cpp in header files, move to CryCommon, use in Cry3DEngine (Martin Mittring)
//
//////////////////////////////////////////////////////////////////////

// class CStencilShadowConnectivity
//
// This is pre-computed data for a model that's gonna cast stencil
// shadows, and whose vertices may change position ****, but the topology
// cannot change. 
//
// Ok, to put it clear: for each model, you need one instance of this
// class initialized once. The model is allowed to deform, but is not
// allowed to change topology (add or remove faces as well as re-assign
// face vertices or even change face orientation).
//
// NOTE: The class is optimized for use with manifold geometry.
// This means you have to provide a model with unified normals and non-
// exploded triangles.
//
//
// IMPLEMENTATION NOTES should be in
// StencilShadowConnectivity.cpp
//

#pragma once
#ifndef _STENCIL_SHADOW_CONNECTIVITY_HDR_
#define _STENCIL_SHADOW_CONNECTIVITY_HDR_

#include <IEdgeConnectivityBuilder.h>				// IStencilShadowConnectivity

// the empty connectivity info:
class CStencilShadowConnectivityNull:public IStencilShadowConnectivity
{
public:

	//! don't forget to call Release for freeing the memory resources
	virtual void Release( void ) {delete this;}

	//! to keep the interface small and the access fast
	//! /return pointer to the internal memory representation (only used within module 3DEngine)
	const virtual CStencilShadowConnectivity *GetInternalRepresentation( void ) const {return NULL;}

	//! for debugging and profiling
	//! /param outVertexCount
	//! /param outTriangleCount
	virtual void GetStats( DWORD &outVertexCount, DWORD &outTriangleCount ) {outVertexCount = outTriangleCount = 0;}

	//! Memorize the vertex buffer in this connectivity object. This is needed if a static object doesn't need this info
	virtual void SetVertices(const Vec3d* pVertices, unsigned numVertices) {assert(0);}

	//! Serializes this object to the given memory block; if it's NULL, only returns
	//! the number of bytes required. If it's not NULL, returns the number of bytes written
	//! (0 means error, insufficient space)
	virtual unsigned Serialize (bool bSave, void* pStream, unsigned nSize, IMiniLog* pWarningLog = NULL)
	{
		return 0;
	}

	//! Calculates the size of this object
	virtual void GetMemoryUsage (ICrySizer* pSizer) {}

#ifdef WIN32
	//! /param szFilename filename with path (or relative) and extension
	//! /return true=success, false otherwise
	virtual bool DebugConnectivityInfo( const char *szFilename ){}
#endif
};

#ifdef WIN64
#pragma warning( push )								//AMD Port
#pragma warning( disable : 4267 )
#endif

class CStencilShadowConnectivity : public IStencilShadowConnectivity
{
public:

	//! number of vertices in the array referenced by the mesh
	//! /return vertex count
	unsigned numVertices() const
	{
		return m_numVertices;
	}

	//! number of edges in the array of edges
	//! /return edge count
	unsigned numEdges() const
	{
		return m_numEdges;
	}

	//! number of orphaned (open) edges
	//! /return orphaned (open) count
	virtual unsigned numOrphanEdges() const
	{
		return m_numOrphanEdges;
	}

	//! total number of faces
	//! /return face count
	unsigned numFaces ()const
	{
		return m_numFaces;
	}

	// -----------------

	// This is the basic edge representation: two vertex indices.
	// This is capable of being sorted.
	struct BasicEdge
	{
		//! default constructor
		BasicEdge () {}
		
		//! constructor
		//! /param nV0 start vertex index
		//! /param nV1 end vertex index 
		BasicEdge (vindex nV0, vindex nV1)
		{
			m_nVertex[0] = nV0;
			m_nVertex[1] = nV1;
		}

		//! start and end vertex index of the edge
		vindex m_nVertex[2];

		// alphabet sorting implementation
		//! /param rEdge right side of the operator term
		inline bool operator < (const BasicEdge& rEdge)const
		{
			if (m_nVertex[0] < rEdge.m_nVertex[0])
				return true;
			else
			if (m_nVertex[0] == rEdge.m_nVertex[0])
				return m_nVertex[1] < rEdge.m_nVertex[1];
			else
				return false;
		}

		//! get teh start or end vertex index
		//! /param nIndex 0/1
		inline unsigned operator [] (int nIndex)const
		{
			assert(nIndex==0 || nIndex==1);
			return m_nVertex[nIndex];
		}

		//! copy the basic edge, designed for ancestors
		//! /param beEdge
		void setBasicEdge (const BasicEdge& beEdge)
		{
			m_nVertex[0] = beEdge.m_nVertex[0];
			m_nVertex[1] = beEdge.m_nVertex[1];
		}
	};

	// -----------------

	// connection of the edge to the face: may contain, depending on the implementation,
	// the face index, pointer to it, vertex, pointer to it
	struct EdgeFace
	{
		//! default constructor
		EdgeFace (){}

		//! constructor
		//! /param nFace face index
		//! /param nVertex vertex index
		EdgeFace (vindex nFace, vindex nVertex): m_nFace(nFace), m_nVertex(nVertex) {}

		//! /return face index
		inline vindex getFaceIndex ()const {return m_nFace;}

		//! /return vertex index
		inline vindex getVertexIndex() const {return m_nVertex;}

		vindex m_nFace;   //!< face index
		vindex m_nVertex; //!< face vertex
	};

	// -----------------

	// This is a primary connectivity edge data structure:
	// the basic edge (start and end vertices, inherited from BasicEdge)
	// and the opposite face edges
	class Edge: public BasicEdge
	{
		//! this and opposite face 3rd vertex.
		//! in "this" face, the edge goes CCW (natural) direction [0],
		//! and in "opposite" face it goes CW (reverse) direction [1]
		EdgeFace m_Face[2];

	public:

		//! default constructor
		Edge () {}

		//! constructor
		//! /param be edge datastructure
		//! /param ef0 in this triangle the edge goes CCW (natural) direction
		//! /param ef1 in this triangle the edge goes CW (reverse) direction
		Edge (const BasicEdge& be, const EdgeFace& ef0, const EdgeFace& ef1):
				BasicEdge(be)
		{
			m_Face[0] = ef0;
			m_Face[1] = ef1;
		}

		//! /param nFace 0/1
		//! /return reference to the face data structure
		const EdgeFace& getFace(int nFace) const
		{
			assert(nFace==0 || nFace==1);

			return m_Face[nFace];
		}

		//! /param nFace 0/1
		//! /param ef reference to the edge datastructure
		void setFace (int nFace, const EdgeFace& ef) 
		{
			assert(nFace==0 || nFace==1);

			m_Face[nFace] = ef;
		}
	};

	// -----------------

	// orphan edge - an edge with only one face attached to it (boundary)
	class OrphanEdge: public BasicEdge
	{
		EdgeFace m_Face;				//!<

	public:
		//! default constructor
		OrphanEdge () {}

		// constructor
		OrphanEdge (const BasicEdge& be, const EdgeFace& ef):
			BasicEdge (be),
			m_Face (ef)
		{}

		const EdgeFace& getFace()const {return m_Face;}
		void setFace (const EdgeFace& ef) {m_Face = ef;}
	};

	// -----------------

	// face - 3 vertices
	class Face
	{
	public:
		vindex m_nVertex[3];					//!<

		//! default constructur
		Face () {}

		//! constructor
		//! /param nV0 first vertex index
		//! /param nV1 second vertex index
		//! /param nV2 thired vertex index
		Face (vindex nV0, vindex nV1, vindex nV2)
		{
			m_nVertex[0] = nV0;
			m_nVertex[1] = nV1;
			m_nVertex[2] = nV2;
		}

		//! get the index of one triangle vertex
		//! /param nVtx 0/1/2
		//! /return index of that triangle vertex
		const vindex getVertex (int nVtx) const
		{
			assert(nVtx==0 || nVtx==1 || nVtx==2);

			return m_nVertex[nVtx];
		}

		//! set the index of one triangle vertex
		//! /param nVtx 0/1/2
		//! /param nIndex index of the triangle vertex no <nVtx>
		void setVertex (int nVtx, vindex nIndex) 
		{
			assert(nVtx==0 || nVtx==1 || nVtx==2);

			m_nVertex[nVtx] = nIndex;
		}
	};

	// -----------------

	//! /aparam nEdge 0..m_numEdges-1
	//! /return the nEdge'th edge
	const Edge& getEdge (int nEdge)const
	{
		assert(m_pEdges);
		assert(nEdge>=0);
		assert((unsigned)nEdge<m_numEdges);

		return m_pEdges[nEdge];
	}

	// returns the nEdge's orphaned edge
	//! /param nEdge 0..m_numOrphanEdges-1
	//! /return
	const OrphanEdge& getOrphanEdge(int nEdge)const
	{
		assert(m_pOrphanEdges);
		assert(nEdge>=0);
		assert((unsigned)nEdge<m_numOrphanEdges);
		
		return m_pOrphanEdges[nEdge];
	}

	//! returns the nFace's face
	//! /param 0..m_numFaces-1
	//! /return reference to the face
	const Face& getFace (int nFace) const
	{
		assert(m_pFaces);
		assert(nFace>=0);
		assert((unsigned)nFace<m_numFaces);

		return m_pFaces[nFace];
	}

	// copies the array of referred faces into internal array
	// this is specially optimized for faster cooperation with the Builder
	// The number of faces is determined from the earlier given edges, so
	// NOTE: becareful to call the SetOrphanEdges BEFORE THIS FUNCTION
	// DO NOT call twice, this is only designed to be called once after construction
	void SetFaces (const std::vector<Face>& arrFaces)
	{
		assert (!m_pFaces && m_numFaces <= arrFaces.size());
		if (!arrFaces.empty())
		{
			m_pFaces = new Face[m_numFaces = arrFaces.size()];
			for (unsigned i = 0; i < m_numFaces; ++i)
				m_pFaces[i] = arrFaces[i];
		}
	}

	// destruct the object (it's only constructed by the Builder)
	// the virtual ensures that the function releases the object in the same module where it was allocated:
	// because the VTable was also allocated in the same module
	virtual void Release ()
	{
		assert(this);
		// this is a counterpart to the new that's called within the
		// CStencilShadowConnectivityBuilder::ConstructConnectivity() method
		delete this;
	}

	const virtual CStencilShadowConnectivity *GetInternalRepresentation( void ) const
	{
		return(this);
	}


	// from IStencilShadowConnectivity
	virtual bool DebugConnectivityInfo( const char *szFilename )
	{
		// used only for debugging
		FILE *out=fopen(szFilename,"w");
		if(!out)return(false);

		fprintf(out,"%d Edges:\n",m_numEdges);
		for(unsigned i=0;i<m_numEdges;i++)
		{
			fprintf(out,"   face={%d,%d}, vertex={%d,%d}\n",
				(m_pEdges[i].getFace(0)).getFaceIndex(),
				(m_pEdges[i].getFace(1)).getFaceIndex(),
				m_pEdges[i].m_nVertex[0],
				m_pEdges[i].m_nVertex[1]);
		}

		fprintf(out,"%d OrphanEdges:\n",m_numOrphanEdges);
		for(unsigned i=0;i<m_numOrphanEdges;i++)
#if !defined(LINUX)
			fprintf(out,"   face={%d}, vertex={%d,%d}\n",m_pOrphanEdges[i].getFace(),m_pOrphanEdges[i].m_nVertex[0],m_pOrphanEdges[i].m_nVertex[1]);
#else
		fprintf(out,", vertex={%d,%d}\n",m_pOrphanEdges[i].m_nVertex[0],m_pOrphanEdges[i].m_nVertex[1]);//face info does not compile, no time to make it more correct here
#endif//LINUX

		fprintf(out,"%d Vertices:\n",m_numVertices);

		fprintf(out,"Faces:\n",m_numFaces);
		for(unsigned i=0;i<m_numFaces;i++)
			fprintf(out,"   vertex indices={%d,%d,%d}\n",m_pFaces[i].getVertex(0),m_pFaces[i].getVertex(1),m_pFaces[i].getVertex(2));

		fclose(out);

		return(true);
	}

	//! constructor, only makes an empty object that's good for deserializing only
	CStencilShadowConnectivity ()
	{
		m_numEdges = m_numOrphanEdges = m_numVertices = m_numFaces = 0;
		m_pOrphanEdges = NULL;
		m_pFaces       = NULL;
		m_pEdges       = NULL;
		m_pPlanes      = NULL;
		m_pVertices    = NULL;
	}

	struct Plane
	{
		Vec3d vNormal;
		float fDistance;

		Plane(){}
		Plane (const Vec3d& v0, const Vec3d& v1, const Vec3d& v2)
		{
			vNormal   = (v1-v0)^(v2-v0);
		  fDistance = (v0*vNormal);
		}
		float apply (const Vec3d& v)const
		{
			return vNormal * v - fDistance;
		}
	};

	//! Calculates the size of this object
	void GetMemoryUsage(ICrySizer* pSizer)
	{
		pSizer->AddObject(this,Serialize (true, NULL,0));
	}

protected:

	//! constructor is only called within 3DEngine
	//! /param pEdges    - array of nNumEdges edges
	//! /param nNumEdges
	CStencilShadowConnectivity( const std::vector<Edge>& arrEdges)
	{
		m_pOrphanEdges=NULL;
		m_pFaces=NULL;
		m_numOrphanEdges=0;

		// find the max vertex index to put it into m_nVertices
		m_numVertices = 0;
		m_numFaces = 0;
		for (std::vector<Edge>::const_iterator it = arrEdges.begin(); it != arrEdges.end(); ++it)
		{
			UseBasicEdge (*it);
			UseEdgeFace (it->getFace(0));
			UseEdgeFace (it->getFace(1));
		}

		m_numEdges = arrEdges.size();
		if(m_numEdges)
		{
			m_pEdges = new Edge[m_numEdges];
			for (unsigned i = 0; i < m_numEdges; ++i)
				m_pEdges[i] = arrEdges[i];
		}
		else
			m_pEdges=0;

		m_pPlanes = NULL;
		m_pVertices = NULL;
	}

	void SetPlanes (const Plane* pPlanes, unsigned numPlanes)
	{
		if (m_pPlanes)
			delete [] m_pPlanes;
		if (pPlanes)
		{
			assert (numPlanes >= m_numFaces);
			m_pPlanes = new Plane[m_numFaces];
			memcpy (m_pPlanes, pPlanes, sizeof(Plane) * m_numFaces);
		}
		else
			m_pPlanes = NULL;
	}

	void SetVertices(const Vec3d* pVertices, unsigned numVertices)
	{
		if (m_pVertices)
			delete[]m_pVertices;
		if (pVertices)
		{
			assert (numVertices >= m_numVertices);
			m_pVertices = new Vec3d[m_numVertices];
			memcpy (m_pVertices, pVertices, sizeof(Vec3d)*m_numVertices);
		}
		else
			m_pVertices = NULL;
	}

	// remaps all vertex indices and memorizes the passed "new" vertex array (in new indexation)
	void SetRemapVertices (const vindex*pMap, unsigned numMapEntries, const Vec3d* pNewVertices, unsigned numNewVertices)
	{
		if (m_pVertices)
			delete[]m_pVertices;
		assert (pNewVertices);
		// we collected information about the vertices before . The number of used vertices is in
		// m_numVertices. We assume the passed new vertex map can not make this number larger, because
		// its sole purpose is optimization of number of vertex indices used (for storing the vertex info inside)
		assert (numNewVertices <= m_numVertices);
		m_numVertices = numNewVertices;
		m_pVertices = new Vec3d[numNewVertices];
		memcpy (m_pVertices, pNewVertices, sizeof(Vec3d)*numNewVertices);

		remapVertexIndices (pMap, numMapEntries);
	}


	// can be destructed only from within Release()
	virtual ~CStencilShadowConnectivity(void)
	{
		if (m_pEdges)				delete[] m_pEdges;
		if (m_pOrphanEdges)	delete[] m_pOrphanEdges;
		if (m_pFaces)				delete[] m_pFaces;
		if (m_pPlanes)      delete[] m_pPlanes;
		if (m_pVertices)    delete[] m_pVertices;
	}


	// makes sure that the given vertex index fits into the range 0..m_nVertices
	// by expanding (increasing) m_nVertices
	void UseVertex(vindex nVertex)
	{
		if (nVertex >= m_numVertices)
			m_numVertices = nVertex + 1;
	}

	// makes sure that the given face index fits into the range 0..m_nFaces
	// by expanding (increasing) m_nFaces
	void UseFace (vindex nFace)
	{
		if (nFace >= m_numFaces)
			m_numFaces = nFace + 1;
	}

	// copies the list of orphaned edges into internal array
	// this is specially optimized for faster cooperation with the Builder
	// DO NOT call twice, this is only designed to be called once after construction
	typedef std::map<BasicEdge, EdgeFace> OrphanEdgeMap;


	// copies the list of orphaned edges into internal array
	// this is specially optimized for faster cooperation with the Builder
	// DO NOT call twice, this is only designed to be called once after construction
	void SetOrphanEdges (const OrphanEdgeMap& mapOrphanEdge)
	{
		m_numOrphanEdges = mapOrphanEdge.size();

		m_pOrphanEdges = new OrphanEdge[m_numOrphanEdges];

		OrphanEdge* pEdge = m_pOrphanEdges;

		// copy the content of the orphan edge map into the array
		for (OrphanEdgeMap::const_iterator it = mapOrphanEdge.begin(); it != mapOrphanEdge.end(); ++it)
		{
			pEdge->setBasicEdge (it->first);
			pEdge->setFace (it->second);

			UseBasicEdge (*pEdge);
			UseEdgeFace (pEdge->getFace());

			++pEdge;
		}
	}

	// makes sure that the vertices/faces referenced by the edge are in account by the m_numFaces and m_numVertices
	void UseBasicEdge (const BasicEdge& beEdge)
	{
		UseVertex(beEdge[0]);
		UseVertex(beEdge[1]);
	}

	// makes sure that the vertices/faces referenced by the edge are in account by the m_numFaces and m_numVertices
	void UseEdgeFace (const EdgeFace& efEdge)
	{
		UseFace (efEdge.getFaceIndex());
		UseVertex(efEdge.getVertexIndex());
	}



	//! for debugging and profiling
	//! /param outVertexCount
	//! /param outTriangleCount
	virtual void GetStats( DWORD &outVertexCount, DWORD &outTriangleCount )
	{
		outVertexCount=m_numVertices;
		outTriangleCount=m_numFaces;
	}

public:

	// the version of the stream
	enum {g_nSerialVersion = 2};

	//! Deserializes this object. Returns the number of bytes read. 0 means error
	//! pWarningLog is used to put warnings about deserialized connectivity
	unsigned Deserialize (void *pStream, unsigned nSize, IMiniLog* pWarningLog = NULL)
	{
		unsigned* pHeader = (unsigned*)pStream;

		if (nSize < 5 * sizeof(unsigned))
			return 0; // we should have the header in place

		if (*(pHeader++) != g_nSerialVersion) // version is incompatible
			return 0;

		m_numEdges       = *(pHeader++);
		m_numOrphanEdges = *(pHeader++);
		m_numVertices    = *(pHeader++);
    m_numFaces       = *(pHeader++);
		unsigned numPlanes = *(pHeader++);
		assert (numPlanes == 0 || numPlanes == m_numFaces);
		unsigned numVertices = *(pHeader++);
		assert (numVertices == 0 || numVertices == m_numVertices);

		unsigned nRequiredSize =
			sizeof(unsigned) +
			sizeof(m_numEdges) + m_numEdges * sizeof(Edge) +
			sizeof(m_numOrphanEdges) + m_numOrphanEdges * sizeof(OrphanEdge) +
			sizeof(m_numVertices) + 
			sizeof(m_numFaces) + m_numFaces * sizeof(Face) +
			sizeof(numPlanes) + numPlanes * sizeof(Plane)+
			sizeof(numVertices) + numVertices * sizeof(Vec3d);

		if(m_pEdges)				delete[] m_pEdges;
		if(m_pOrphanEdges)	delete[] m_pOrphanEdges;
		if(m_pFaces)				delete[] m_pFaces;
		if(m_pPlanes)      delete[] m_pPlanes;
		if (m_pVertices)   delete []m_pVertices;

		if (nSize < nRequiredSize)
		{
			m_pEdges = NULL;
			m_pOrphanEdges = NULL;
			m_pFaces = NULL;

			m_numEdges       = 0;
			m_numOrphanEdges = 0;
			m_numVertices    = 0;
			m_numFaces       = 0;

			// incompatible stream
			return 0;
		}

		m_pEdges = new Edge [m_numEdges];
		m_pOrphanEdges = new OrphanEdge [m_numOrphanEdges];
		m_pFaces = new Face [m_numFaces];
		m_pPlanes = numPlanes?new Plane[numPlanes]:NULL;
		m_pVertices = numVertices?new Vec3d[numVertices]:NULL;

		Edge* pEdgeData = (Edge*)pHeader;
		memcpy (m_pEdges, pEdgeData, m_numEdges*sizeof(Edge));
		OrphanEdge* pOrphanEdgeData = (OrphanEdge*)(pEdgeData + m_numEdges);
		memcpy (m_pOrphanEdges, pOrphanEdgeData, m_numOrphanEdges*sizeof(OrphanEdge));
		Face* pFaceData = (Face*)(pOrphanEdgeData + m_numOrphanEdges);
		memcpy (m_pFaces, pFaceData, m_numFaces * sizeof(Face));

		Plane* pPlanes = (Plane*)(pFaceData+m_numFaces);
		if (numPlanes)
			memcpy (m_pPlanes, pPlanes, numPlanes* sizeof(Plane));

		Vec3d* pVertices = (Vec3d*)(pPlanes + numPlanes);
		if (numVertices)
			memcpy (m_pVertices, pVertices, sizeof(Vec3d)*numVertices);
	
#if !defined(LINUX)
		if (m_numOrphanEdges && pWarningLog)
			pWarningLog->LogWarning("Connectivity warning: there are %d open edges (%d closed, %d faces, %d planes, %d vertices)", m_numOrphanEdges, m_numEdges, m_numFaces, numPlanes, numVertices);
#endif
		return nRequiredSize;
	}


	//! Serializes this object to the given memory block; if it's NULL, only returns
	//! the number of bytes required. If it's not NULL, returns the number of bytes written
	//! (0 means error, insufficient space)
  unsigned Serialize (bool bSave, void* pStream, unsigned nSize, IMiniLog* pWarningLog = NULL)
	{
		if (!bSave)
			return Deserialize (pStream, nSize, pWarningLog);

		// calculate the required size of the buffer
		unsigned
			numPlanes = m_pPlanes?m_numFaces:0,
			numVertices = m_pVertices?m_numVertices:0,
			nRequiredSize =
			sizeof(unsigned) +
			sizeof(m_numEdges) + m_numEdges * sizeof(Edge) +
			sizeof(m_numOrphanEdges) + m_numOrphanEdges * sizeof(OrphanEdge) +
			sizeof(m_numVertices) + 
			sizeof(m_numFaces) + m_numFaces * sizeof(Face) +
			sizeof(numPlanes) + numPlanes * sizeof(Plane)+
			sizeof(numVertices) + numVertices * sizeof(Vec3d);

		if (pStream)
		{
			if (nSize < nRequiredSize)
				return 0; // insufficient space in the pStream buffer

			unsigned* pHeader = (unsigned*)pStream;
			*(pHeader++) = g_nSerialVersion; // version of the stream
			*(pHeader++) = m_numEdges;
			*(pHeader++) = m_numOrphanEdges;
			*(pHeader++) = m_numVertices;
      *(pHeader++) = m_numFaces;
			*(pHeader++) = numPlanes;
			*(pHeader++) = numVertices;

			Edge* pEdgeData = (Edge*)pHeader;
			memcpy (pEdgeData, m_pEdges, m_numEdges*sizeof(Edge));
			OrphanEdge* pOrphanEdgeData = (OrphanEdge*)(pEdgeData + m_numEdges);
			memcpy (pOrphanEdgeData, m_pOrphanEdges, m_numOrphanEdges*sizeof(OrphanEdge));
			Face* pFaceData = (Face*)(pOrphanEdgeData + m_numOrphanEdges);
			memcpy (pFaceData, m_pFaces, m_numFaces * sizeof(Face));
			Plane* pPlanes = (Plane*)(pFaceData+m_numFaces);
			if (m_pPlanes)
				memcpy (pPlanes, m_pPlanes, numPlanes * sizeof(Plane));
			Vec3d* pVertices = (Vec3d*)(pPlanes+numPlanes);
			if (m_pVertices)
				memcpy (pVertices, m_pVertices, numVertices * sizeof(Vec3d));
		}

		return nRequiredSize;
	}

	// the plane equation, x*n+d == 0
	bool hasPlanes() const {return m_pPlanes != NULL;}
	const Plane&getPlane (unsigned i)const {assert(i<m_numFaces && m_pPlanes);return m_pPlanes[i];}

	const Vec3d* getVertices() const {return m_pVertices;}

	bool IsStandalone() const {return m_pPlanes != NULL && m_pVertices!= NULL;}

	void remapVertexIndices(const vindex*pMap, unsigned nMapSize)
	{
		unsigned i;
#define REMAP(X) assert((X) < nMapSize);\
		(X) = pMap[X];\
		assert ((vindex)(X) != (vindex)-1 && (X) < m_numVertices)

		for(i=0;i<m_numEdges;i++)
		{
			REMAP(m_pEdges[i].m_nVertex[0]);
			REMAP(m_pEdges[i].m_nVertex[1]);
		}
		for(i=0;i<m_numOrphanEdges;i++)
		{
			REMAP(m_pOrphanEdges[i].m_nVertex[0]);
			REMAP(m_pOrphanEdges[i].m_nVertex[1]);
		}
		for(i=0;i<m_numFaces;i++)
		{
			REMAP(m_pFaces[i].m_nVertex[0]);
			REMAP(m_pFaces[i].m_nVertex[1]);
			REMAP(m_pFaces[i].m_nVertex[2]);
		}
#undef REMAP
	}
private:


	friend class CStencilShadowConnectivityBuilder;
	friend class CStencilShadowStaticConnectivityBuilder;
	
	// the array of normal edges (with two adjacent faces)
	unsigned m_numEdges;																		//!<
	Edge* m_pEdges;																					//!<

	// the array of orphaned edges (with only one face attached)
	unsigned m_numOrphanEdges;															//!<
	OrphanEdge* m_pOrphanEdges;															//!<


private:
	// number of vertices in the array referenced by the mesh
	unsigned m_numVertices;																//!<

	// the faces
	unsigned m_numFaces;																		//!< number of faces in the array referenced by the mesh
	Face* m_pFaces;																					//!<
	Plane* m_pPlanes;                                      //!< if not NULL, then use these normals instead of computing them on the fly; 1 face - 1 normal
	Vec3d* m_pVertices;
};

#ifdef WIN64
#pragma warning( pop )								//AMD Port
#endif


#endif // _STENCIL_SHADOW_CONNECTIVITY_HDR_