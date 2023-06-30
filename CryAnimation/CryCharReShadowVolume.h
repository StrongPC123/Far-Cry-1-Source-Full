/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created 12/05/2002 by Sergiy Migdalskiy
//
//  This is the class that's used by the model state (or cry char instance)
//  to render the shadow volumes. Generally, it's former part of the CryModelState
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_CHAR_RE_SHADOW_VOLUME_HDR_
#define _CRY_CHAR_RE_SHADOW_VOLUME_HDR_

// This is the utility/helper class for rendering the shadow volumes,
// all the dirty work (of double buffering and managing the memory)
// should be in this class
//
// USAGE:
// 1. get the vertex and index buffers and fill them in by the declared
//    number of vertices and indices referring to them
// 2. submit that with submit() function that will create the renderer object and
//    update the buffers and do everything required.
//
// This sequence is to be done once and only once per frame per light per character
class CryCharReShadowVolume
{
public:
	CryCharReShadowVolume();
	~CryCharReShadowVolume();

	void clear();

	// prepares for calculating the shadow volume	
	// numVertices is the minimal length of the vertex buffer
	// numIndices is the minimal length of the index buffer
	void prepare (unsigned numIndices, unsigned numVertices);
	
	// returns the pointer to the array of vertices to fill in
	// the vertices are used to form the shadow volume mesh/geometry
	Vec3d* getVertexBuffer () {return &m_arrVertices[0];}

	// returns the pointer to the indices in the index buffer to fill in
	// the indices refer to the vertices in the vertex buffer returned by getVertexBuffer ()
	unsigned short* getIndexBuffer () {return &m_arrIndices[0];}

	// assuming the calculation of the shadow volume is finished, submits it to the renderer
	void submit (const SRendParams *rParams, IShader* pShadowCull );

	// returns the age of this shadow volume: the current frame - the frame it was submitted last
	unsigned getAgeFrames();

	// returns the timeout from the last call to submit()
	float getAgeSeconds();

	void GetMemoryUsage (ICrySizer* pSizer);
protected:
	unsigned numMeshIndices() {return m_nUsedMeshVertices;}
	unsigned numMeshVertices() {return m_pLeafBuffer->m_SecVertCount;}

protected:
	//! shadow volume renderer managed vertex buffer
	CLeafBuffer* m_pLeafBuffer;

	// this is the number of vertices actually used in the leaf buffer
	// (referred to by the index array)
	unsigned m_nUsedMeshVertices;

	//! Shader RenderElements for stencil
	CRETriMeshShadow* m_pMesh;

	//! shadow volume indices
	TFixedArray<unsigned short> m_arrIndices;

	//! shadow volume memory vertex buffer
	//! TODO: get rid of it; rewrite the shadow code so that we don't keep it in every character
	TFixedArray<Vec3d> m_arrVertices;

	// the synchronous time when the last submit() was called
	float m_fLastTimeSubmitted;
	
	// the last frame submit() function was called
	int m_nLastFrameSubmitted;
};

#endif