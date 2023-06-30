#ifndef _CRY_ANIMATION_CRY_SKIN_BUILDER_BASE_HDR_
#define _CRY_ANIMATION_CRY_SKIN_BUILDER_BASE_HDR_

#include "CrySkinTypes.h"
#include "CryVertexBinding.h"

// This interface presents all the necessary information to the skin builders
// the actual skinned info can actually be vertices or normals - doesn't really matter
class ICrySkinSource
{
public:
	// this structure is initialized through the constructor
	ICrySkinSource (
			const CryVertexBinding* pLinks,
			unsigned numLinks,
			const Vec3* pVertices,
			unsigned numVertices,
			const TangData* pExtTangents,
			unsigned numExtTangents,
			const unsigned* pExtToIntMapping
		):
		m_pLinks (pLinks),
		m_numLinks (numLinks),
		m_pVertices (pVertices),
		m_numVertices (numVertices),
		m_pExtTangents (pExtTangents),
		m_numExtTangents (numExtTangents),
		m_pExtToIntMapping (pExtToIntMapping)
	{}
	unsigned numVertices()const {return m_numVertices;}
	const Vec3& getVertex (unsigned i)const  {return m_pVertices[i];}
	unsigned numLinks() const {return m_numLinks;}
	const CryVertexBinding& getLink (unsigned i) const {return m_pLinks[i];}
	unsigned numExtTangents() const {return m_numExtTangents;}
	const unsigned *getExtToIntMapEntries()const {return m_pExtToIntMapping;}
	const TangData& getExtTangent(unsigned i)const {return m_pExtTangents[i];}
protected:
	// this needs to be filled in by the derived class
	unsigned m_numLinks;
	const CryVertexBinding* m_pLinks;
	unsigned m_numVertices;
	const Vec3* m_pVertices;
	const TangData* m_pExtTangents;
	unsigned m_numExtTangents;
	const unsigned* m_pExtToIntMapping;
};

class CrySkinBuilderBase0
{
public:
	CrySkinBuilderBase0(const class ICrySkinSource* pGeometry):
		m_pGeometry (pGeometry)
		{}

	typedef CrySkinVertexAligned Vertex;

	struct CrySkinStreams
	{
		CrySkinAuxInt* pAux;
		Vertex* pVert;
	};
protected:
	const ICrySkinSource* m_pGeometry;
};

class CrySkinBuilderBase: public CrySkinBuilderBase0
{
public:
protected:
	CrySkinBuilderBase(const class ICrySkinSource* pGeometry);

	// Calculates the total number of links per all vertices. If the whole object is rigid,
	// the number of links is the same as the number of vertices.
	// Also calculates the sets of vertices belonging to each bone
	// calculates the max number of bone affecting a vertex + 1
	// and the total number of links to smooth vertices - the sum of numbers of links of all smooth vertices
	void preprocess();

	// calculates the vertex list of each bone, for all vertices present in the geometry
	void makeFullBoneVertexArrays();

	// fills in the group of aux ints for the given bone (the rigid vertex group)
	// returns the pointer to the next available auxint after the group
	void fillRigidGroup (CrySkinStreams& streams, unsigned nBone);

protected:

	// precalculated numbers
	unsigned m_numLinks; // total number of links (number of aligned vertex structures)
	unsigned m_numBones; // number of bones affecting the skin, in the original array (max bone id + 1)
	unsigned m_numSmoothLinks; // number of smooth links

	// the sets of vertices belonging to each bone
	typedef std::vector< CrySkinRigidVertex > CrySkinRigidVertexArray;
	typedef std::vector< CrySkinSmoothVertex > CrySkinSmoothVertexArray;
	struct BoneVertexGroup
	{
		CrySkinRigidVertexArray arrRigid;
		CrySkinSmoothVertexArray arrSmooth;
		bool empty() const{return arrRigid.empty() && arrSmooth.empty();}
		void reserve (unsigned numReserve)
		{
			arrRigid.reserve (numReserve/2);
			arrSmooth.reserve (numReserve/2);
		}
	};
	typedef std::vector< BoneVertexGroup > BoneVertexArray;
	BoneVertexArray m_arrBoneVerts;
};

#endif