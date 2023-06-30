// copies the vertex info to the videomemory, given the source (vertices and UVs),
// target format and pointer to the videobuffer
extern void CopyPosUVsToVideomemory (
	const Vec3* pSrcPos,
	const CryUV* pSrcUV,
	unsigned numVerts,  // number of Target vertices
	const unsigned *pVertMap, // vertex map from Target to Source vertex indices
	Vec3* pDstPos,
	int nDstVertexFormat
	);

// copies the vertex info to the videomemory, given the source (vertices and UVs),
// target format and pointer to the videobuffer
extern void CopyPosUVsToVideomemory (
	const Vec3* pSrcPos,
	const CryUV* pSrcUV,
	unsigned numVerts,  // number of Target vertices
	Vec3* pDstPos,
	int nDstVertexFormat
	);

extern IShader* GetShaderStencilStateFrontCull();
extern IShader* GetShaderFrontCull();