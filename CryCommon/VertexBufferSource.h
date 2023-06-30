// Using the structure defined in this header, the leaf buffer is created
// from serialized data in the compiled CCG file

#ifndef _VERTEX_BUFFER_SOURCE_HDR_
#define _VERTEX_BUFFER_SOURCE_HDR_

struct VertexBufferSource
{
	VertexBufferSource():
		numMaterials (0),
		pMaterials (NULL),
		pShaders (NULL),
		pMats (NULL),
		pOrFlags(NULL),
		pAndFlags(NULL),
		numPrimGroups(0),
		pPrimGroups(NULL),
		numIndices(0),
		pIndices(NULL),
		numVertices (0),
		pVertices(NULL),
		pUVs(NULL),
		nREFlags (0)
	{
	}

	// number of materials in the corresponding array
	unsigned numMaterials;
	// the pointer to numMaterials materials
	const struct MAT_ENTITY* pMaterials;
	// the pointer to the shaders, there must be numMaterials elements
	const SShaderItem* pShaders;
	// if there are no shaders, the client may specify the array of already loaded CMatInfo's
	// this array is then moved to the leaf buffer and this member is NULL'ed (if no error occurs)
	list2<CMatInfo>* pMats;
	// for each material, there can be OR and/or AND flags. 
	// they are ORed and ANDed with m_Flags in CMatInfo
	// these parameters may be NULL
	const unsigned* pOrFlags;
	const unsigned* pAndFlags;
	
	// the number of Primitive Groups (sections)
	unsigned numPrimGroups;
	// the pointer to the array of sections
	const struct CCFMaterialGroup* pPrimGroups;
	
	// the number of indices in the index buffer
	unsigned numIndices;
	// the indices
	const unsigned short* pIndices;

  // the number of vertices
	unsigned numVertices;
	// the vertices
	const Vec3d* pVertices;
	// UVs
	const CryUV* pUVs;

	// these flags will be ORed to the RE m_Flags (this is for FCEF_DYNAMIC flag in animation)
	unsigned nREFlags;
};

#endif
