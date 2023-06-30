#ifndef _CRY_SKIN_BUIDLER_HDR_
#define _CRY_SKIN_BUIDLER_HDR_

class CrySkinFull;
#include "CrySkinBuilderBase.h"

//////////////////////////////////////////////////////////////////////////
// Capable of building an optimized CrySkin class out of geometry info
class CrySkinBuilder: public CrySkinBuilderBase
{
public:
	// initializes the builder for usage
	CrySkinBuilder (const ICrySkinSource* pGeometry);
	// creates a new skin, capable of skinning into a uninited mem, to be deleted by Release
  void initSkinFull(CrySkinFull*);

protected:

	// fills in 2 groups of aux ints for the given bone (the smooth vertex groups)
	// returns the pointer to the next available auxint after the groups
	void fillSmoothGroups (CrySkinStreams& streams, unsigned nBone);

	// validates the created skin
	void validate (CrySkinFull *pSkin);
protected:
	unsigned m_numAuxInts; // number of aux ints required for the skin

	// the number of times a vertex has been already met in the auxiliary stream
	TElementaryArray<unsigned> m_arrSmoothVertHitCount;

	// number of bones to skip when skinning
	unsigned m_numSkipBones;
};

#endif