/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	  Sep 25 2002 :- Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_CHAR_DECAL_MANAGER_HDR_
#define _CRY_CHAR_DECAL_MANAGER_HDR_

#include "CryCharDecal.h"
#include "SparseArrayDriver.h"
#include "CryCharRenderElement.h"

#ifndef DECAL_USE_HELPERS
#error DECAL_USE_HELPERS must be defined in this header. Please include "CryCharDecal.h"
#endif

// This class manages decals on characters.
// It memorizes requests for decals, then realizes them using the character skin
// geometry (constructs the decal face strip, UV-maps it)
//
// The workflow is as follows:
//   Request(Add) the decal. At this stage, its info will just be memorized, it won't be present in the decal list
//   Realize the decal. At this stage, the deformable geometry will be created for the decal.
//   Render the decal. After realization, the decals have their own geometry that bases on the deformed vertices
//   and normals of the character skin.
class CryCharDecalManager
{
public:

	static void LogStatistics();

	CryCharDecalManager (class CryGeometryInfo* pGeomInfo);

	~CryCharDecalManager ();

	// Request (add) a new decal to the character
	void Add (CryEngineDecalInfo& Decal);

	// discards the decal request queue (not yet realized decals added through Add())
	void DiscardRequests();

	// realizes (creates geometry for) unrealized(requested) decals
	void Realize (const Vec3* pPositions);

	// returns true if the Realize() needs to be called
	bool NeedRealize () const;

	// this is the number of additional faces introduced by the decals
	unsigned numFaces() const;
	// this is the number of additional vertices introduced by the decals
	unsigned numVertices() const;

	// adds the render data to the renderer, so that the current decals can be rendered
	void AddRenderData (CCObject *pObj, const SRendParams & rRendParams);

	// cleans up all decals, destroys the vertex buffer
	void clear();

	void debugDump();

	// returns the memory usage by this object into the sizer
	void GetMemoryUsage (ICrySizer* pSizer);
protected:
	// sets up the given material to default state: just clean decal material
  void initDefaultMaterial (CMatInfo& rMat);

	// assigns the given material to the given range of indices/vertices
	void assignMaterial (unsigned nMaterial, int nTextureId, int nFirstIndex, int numIndices, int nFirstVertex, int numVertices);

	// recreates the current leaf buffer so that it contains the given number of vertices and
	// reserved indices, both uninitialized. There are 0 used indices initially
	void ReserveVertexBufferVertices (const Vec3* pInPositions);

	// recalculates the index array for the vertex buffer and replaces it (so that the vertex buffer is prepared for rendering)
	// also makes sure the vertex buffer contains enough vertices to draw all the current decals (from m_arrDecals)
	void RefreshVertexBufferIndices ();

	// put the deformed vertices into the videomemory
	void RefreshVertexBufferVertices (const Vec3* pPositions);

	// put the deformed vertices into the videobuffer of the given format
	void RefreshVertexBufferVertices (const Vec3* pPositions, struct_VERTEX_FORMAT_P3F_TEX2F* pDst);

	// put the deformed vertices into the videobuffer of the given format
	void RefreshVertexBufferVertices (const Vec3* pPositions, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F* pDst);

	// cleans up the old leaf buffers
	void DeleteOldRenderElements();

	// cleans up the dead decals
	// sets m_bNeedUpdateIndices to true if it has added something (and therefore refresh of indices is required)
	void DeleteOldDecals();

	// if there are decal requests, then converts them into the decal objects
	// reserves the vertex/updates the index buffers, if need to be
	// sets m_bNeedUpdateIndices to true if it has added something (and therefore refresh of indices is required)
	void RealizeNewDecalRequests (const Vec3* pPositions);

	// starts fading out all decals that are close enough to the given point
	// NOTE: the radius is m^2 - it's the square of the radius of the sphere
	void fadeOutCloseDecals (const Vec3& ptCenter, float fRadius2);
protected:
	// deletes the leaf buffer
	void DeleteLeafBuffer ();

	// this represents the contiguous subarray of vertices and indices
	struct MeshInfo {
		unsigned short numIndices, numVertices;
	};
	struct SubmeshInfo: public MeshInfo {
		unsigned short nFirstIndex, nFirstVertex;
		int nTextureId;
	};

	// temporary locations for groupMaterials results
	// the information about the decal mesh currently (after groupMaterials)
	static MeshInfo g_MeshInfo;
	// the maximum number of decal types per character
	enum {g_numSubmeshInfos = 32};
	// the material groups
	static SubmeshInfo g_SubmeshInfo[g_numSubmeshInfos];
	// groups the decals and returns the information in gTmpMeshInfo and gTmpSubmeshInfo
	// returns the number of materials in g_SubmeshInfo
	unsigned groupMaterials ();


	// the array of requested (unrealized) decals
	std::vector<CryEngineDecalInfo> m_arrDecalRequests;

	// the array of realized decals
	typedef std::vector<CryCharDecal> CDecalArray;
	CDecalArray m_arrDecals;

	// the geometry of the character
	CryGeometryInfo* m_pGeometry;

	// this shader is used by all decals
	IShader* m_pShader;

	// if this is true, then the indices will be updated upon the next render method invokation
	bool m_bNeedUpdateIndices;

	//
	// The following members are only dealt with by the Reserve method, constructor and destructor
	//

	CryCharRenderElement m_RE;


	// this is the collection of leaf buffers that are to be deleted - each with its own last render frame
	typedef std::vector<CryCharRenderElement> RenderElementArray;
	RenderElementArray m_arrOldRE;


	struct CStatistics
	{
		unsigned numDecals;
		unsigned numDecalVertices;
		unsigned numDecalFaces;
		float getAveVertsPerDecal() {return float (numDecalVertices) / numDecals;}
		float getAveFacesPerDecal() {return float (numDecalFaces) / numDecals;}
		bool empty() const {return numDecals == 0;}
		void onDecalAdd (unsigned numVertices, unsigned numFaces);
		CStatistics():
			numDecalVertices(0),
			numDecals(0),
			numDecalFaces (0)
		{
		}
	};
	static CStatistics g_Statistics;

};

#endif