#ifndef _CRY_ANIMATION_CRY_SKIN_RIGID_BASIS_HDR_
#define _CRY_ANIMATION_CRY_SKIN_RIGID_BASIS_HDR_

#include "CrySkinBase.h"


//////////////////////////////////////////////////////////////////////////
// This is the skin that calculates the tangent bases
// the skin is assumed to be rigid.
// The basises can be flipped, but may not be non-normalized or non-orthogonal
//
// IMPLEMENTATION NOTES:
//  each base is represented by the first 2 vectors in the vertex array.
//  the 3rd one is the cross-product, optionally negated with the 0x80000000
//  flag in the 2nd vertex's nDest field. The 1st vertex contains the destination
//  vertex index itself
class CrySkinRigidBasis: public CrySkinBase
{
public:
	// returns the size of the skin, the number of bases being calculated
	// by this skin. The bases are calculated into a 0-base continuous array
	// tangents may be divided into subskins, each having different number of bases
	// to skin, based on the performance consideration (strip mining)
	unsigned size()const;

	// does the skinning out of the given array of global matrices:
	// calculates the bases and fills the PipVertices in
	void skin (const Matrix44* pBones, SPipTangentsA* pDest)const;
#if defined(_CPU_X86) && !defined(LINUX)
	// uses SSE for skinning; NOTE: EVERYTHING must be 16-aligned:
	// destination, bones, and the data in this object
	void skinSSE (const Matrix44* pBones, SPipTangentsA* pDest)const;
#endif
	friend class CrySkinBasisBuilder;

	// does the same as the base class init() but also remembers the number of bases (numVerts/2)
	// for future reference
	void init (unsigned numVerts, unsigned numAux, unsigned numSkipBones, unsigned numBones);

	// returns the number of bytes occupied by this structure and all its contained objects
	unsigned sizeofThis()const;

	friend class CStatistics;
	// this structure contains the statistical information about this skin; its calculation
	// may take significant time and should not be used in game run time (only for debugging purposes
	// and to output statistics in the tools)
	class CStatistics: public CrySkinBase::CStatistics
	{
	public:
		CStatistics (const CrySkinRigidBasis* pSkin):
			CrySkinBase::CStatistics(pSkin)
		{
			initSetDests (pSkin);
		}

		void initSetDests (const CrySkinRigidBasis* pSkin);
		void addDest(unsigned nDest);

		// destination vertex set
		std::set<unsigned> setDests;
		// the number of links per each vertex
		std::vector<unsigned> arrNumLinks;
	};

	unsigned Serialize (bool bSave, void* pBuffer, unsigned nBufSize);
protected:
	// The size of the skin, the number of bases being calculated
	// by this skin. The bases are calculated into a 0-base continuous array
	unsigned m_numDestBases;
};

#endif