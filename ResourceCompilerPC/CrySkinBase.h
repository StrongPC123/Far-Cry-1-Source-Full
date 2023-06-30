#ifndef _CRY_ANIMATION_CRY_SKIN_BASE_HDR_
#define _CRY_ANIMATION_CRY_SKIN_BASE_HDR_

#include "SSEUtils.h"
#include "CrySkinTypes.h"

// this is the base for the actual skins
// the actual skin does the validation and calculates the skin
class CrySkinBase
{
public:
	void clear();
	bool empty()const;

	void scaleVertices (float fScale);

	//version for little-endian
	unsigned Serialize_PC (bool bSave, void* pBuffer, unsigned nBufSize);

	//version for big-endian
	unsigned Serialize_GC (bool bSave, void* pBuffer, unsigned nBufSize);
	
	// returns the number of bytes occupied by this structure and all its contained objects
	unsigned sizeofThis()const;

	// this structure contains the statistical information about this skin; its calculation
	// may take significant time and should not be used in game run time (only for debugging purposes
	// and to output statistics in the tools)
  class CStatistics
	{
	public:
		CStatistics (const CrySkinBase*pSkin);
		
		unsigned numBones;
		unsigned numSkipBones;
		unsigned numAuxInts;
		unsigned numVertices;
	};
	friend class CStatistics;

	unsigned numBones() const {return m_numBones;}
	unsigned numSkipBones() const {return m_numSkipBones;}
protected:
	CrySkinBase();
	~CrySkinBase ();
	void init (unsigned numVerts, unsigned numAux, unsigned numSkipBones, unsigned numBones);

	typedef CrySkinVertexAligned Vertex;

	// transforms the given _vector_ without applying the transitional part
	//void transformVectorNoTrans (Vec3& pDest, const Vec3& pSrc, const Matrix& matBone);


	// transforms the given smooth point into the destination with the matrix
	static void transformWPoint (Vec3& pDest, const Matrix44& matBone, const Vertex& rVtx);

	// adds the given smooth point into the destination with the matrix
	static void addWPoint (Vec3& pDest, const Matrix44& matBone, const Vertex& rVtx);

	// transforms the given smooth point into the destination with the matrix
	static void transformWVector (Vec3& pDest, const Matrix44& matBone, const Vertex& rVtx);

	// adds the given smooth point into the destination with the matrix
	static void addWVector (Vec3& pDest, const Matrix44& matBone, const Vertex& rVtx);

	//typedef TIncContAllocator<Vertex, &g_VectorAllocator> VertexAllocator;
	//typedef TIncContAllocator<CrySkinAuxInt, &g_VectorAllocator> AuxIntAllocator;
	typedef TAllocator16<Vertex> VertexAllocator;

	// the vertex stream: contains the rigid and smooth vertex structures
	typedef TFixedArray<Vertex, VertexAllocator> VertexArray;
	VertexArray m_arrVertices;

	// the auxiliary stream: contains the number of vertices in each group, and the group vertices' destination idices (for smooth vertices only)
	TFixedArray<CrySkinAuxInt> m_arrAux;
	// the total number of bones, NOT taking the m_numSkipBones into account
	unsigned m_numBones;
	// additional flexibility: skip m_numSkipBones bones before performing skinning;
	// useful for optimizing the skin which has only one or a few bones' influences
	unsigned m_numSkipBones;

	// this is the header structure for serialization
	struct SerialHeader
	{
		void initFromSkin(const CrySkinBase* pSkin)
		{
			numBones = pSkin->m_numBones;
			numSkipBones = pSkin->m_numSkipBones;
			numDests = pSkin->m_numDests;
			numAuxInts = (unsigned)pSkin->m_arrAux.size();
			numVertices = (unsigned)pSkin->m_arrVertices.size();
		}

		unsigned numBones;
		unsigned numSkipBones;
		unsigned numDests;
		unsigned numAuxInts;
		unsigned numVertices;
	};

	// the total number of destination vertices
	unsigned m_numDests;
};

#endif