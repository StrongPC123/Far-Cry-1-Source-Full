#ifndef _CRY_GEOM_MORPH_TARGET_HDR_
#define _CRY_GEOM_MORPH_TARGET_HDR_

// this is a container for the information about morphing a mesh
class CryGeomMorphTarget
{
public:
	// create an identity morph target (doesn't morph anything)
	CryGeomMorphTarget();

	// load the morph target from the chunk/size. Returns 0 is the chunk is completely wrong,
	// or the number of read bytes if it's ok (returns nSize if everything is fine)
	unsigned load (unsigned nLOD, const MESHMORPHTARGET_CHUNK_DESC_0001* pChunk, unsigned nSize);

	// scales the target vertices by the given factor
	void scale (unsigned nLOD, float fScale);

	// rotates the target morph vertices; transforms each vertex with the specified matrix, using only its
	// ROTATIONAL components (no translation)
#if !defined(LINUX)
	__declspec(deprecated)
#endif
	void rotate (unsigned nLOD, const Matrix44& tm);

	// transforms the target morph vertices with the given matrix
	void transform (unsigned nLOD, const Matrix44& tm);

	// given the source morph object, morphs it with the given weight to this morph target;
	// 1 means the morph target will replace the target with itself; 0 means it will just copy the source
	// into the destination (or leave it alone, if the two coincide)
#if !defined(LINUX)
	__declspec(deprecated)
#endif
	void morph (unsigned nLOD, const Vec3d* pSrc, Vec3d* pDst, unsigned numVertices, float fWeight)const;

	// returns the name of the morph target
	const char* getNameCStr()const {return m_strName.c_str();}
	void setName (const char* szName, unsigned nMaxSize = 0xFFFFFFFF);

	const SMeshMorphTargetVertex* getMorphVertices (unsigned nLOD) const;
	unsigned numMorphVertices (unsigned nLOD) const;
	
	size_t sizeofThis()const;
protected:
	// morphed vertices
	class MorphVertexArray: public TFixedArray<SMeshMorphTargetVertex>
	{public:
		MorphVertexArray():  TFixedArray<SMeshMorphTargetVertex>("CryGeomMorphTarget.MorphVertexArray") {}
	};
	// the max supported LODs for the morph targets
	enum {g_nMaxLodLevels = 1};
	MorphVertexArray m_arrVertex[g_nMaxLodLevels]; 

	// the name
	string m_strName;
};

#endif