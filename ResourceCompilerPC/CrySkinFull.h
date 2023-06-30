#ifndef _CRY_SKIN_HDR_
#define _CRY_SKIN_HDR_

#include "CrySkinTypes.h"
#include "CrySkinBase.h"
#include "platform.h"

//////////////////////////////////////////////////////////////////////////
// the optimized skinner; built with the CrySkinBuilder class instance,
// destroyed with the Release()
// This is the full skinner: it skins into a memory with garbage in it
class CrySkinFull: public CrySkinBase
{
public:
	friend class CrySkinBuilder;

	// does the skinning out of the given array of global matrices
	void skin (const Matrix44* pBones, Vec3* pDest);
	// Skins skipping the translation components of bone matrices
	void skinAsVec3d16 (const Matrix44* pBones, Vec3dA16* pDest);
#if ( defined (_CPU_X86) || defined (_CPU_AMD64) ) & !defined(LINUX)
	// skins using the given bone matrices, into the given destination array,
	// SIDE EFFECT: calculates the bounding box into the g_BBox
	void skinSSE (const Matrix44* pBones, Vec3dA16* pDest);	
	DEFINE_ALIGNED_DATA_STATIC( CryBBoxA16, g_BBox, 32 ); // align by cache line boundaries
#endif

	// takes each offset and includes it into the bbox of corresponding bone
	void computeBoneBBoxes(CryBBoxA16* pBBox);

	void scale (float fScale)
	{
		scaleVertices(fScale);
	}

	// validates the skin against the given geom info
	void validate (const class ICrySkinSource* pGeometry);

	// this structure contains the statistical information about this skin; its calculation
	// may take significant time and should not be used in game run time (only for debugging purposes
	// and to output statistics in the tools)
	class CStatistics: public CrySkinBase::CStatistics
	{
	public:
		CStatistics (const CrySkinFull* pSkin):
			CrySkinBase::CStatistics(pSkin)
			{
				initSetDests (pSkin);
			}

		void initSetDests (const CrySkinFull* pSkin);
		void addDest(unsigned nDest);

		// destination vertex set
		std::set<unsigned> setDests;
		// the number of links per each vertex
		std::vector<unsigned> arrNumLinks;
	};

	friend class CStatistics;
};

#endif