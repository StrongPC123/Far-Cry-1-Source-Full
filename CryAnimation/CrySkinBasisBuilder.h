#ifndef _CRY_ANIMATION_CRY_SKIN_BASIS_BUILDER_HDR_
#define _CRY_ANIMATION_CRY_SKIN_BASIS_BUILDER_HDR_

#include "CrySkinBuilderBase.h"

class CrySkinBasisBuilder: public CrySkinBuilderBase0
{
public:
	CrySkinBasisBuilder (const ICrySkinSource* pGeometry, const Matrix44* pMatInvDef, unsigned numBones);

	// sets the destination vertex interval to operate on. Initially, this is infinity.
	// The destination vertex interval is the interval within which the destination vertex
	// index must lie in order to be skinned. The base of the interval is considered
	// vertex 0 in the produced skinner
	void setDestinationInterval (unsigned nBegin, unsigned nEnd);

	// initializes the given rigid basis builder
	void initRigidBasisSkin (class CrySkinRigidBasis* pSkin);
protected:

	// calculate the number of used bones and the skip-bone
	void preprocess();

	// makes up the basis array
	void makeBoneBases ();

	// fills the given bases to the simple stream
	typedef std::vector< CrySkinRigidBaseInfo > CrySkinRigidBasisArray;
  void fillGroup (CrySkinStreams& stream, const CrySkinRigidBasisArray& arrBases);
protected:
	// the bone info, from which we'll extract the default position
	const Matrix44* m_pMatInvDef;
	unsigned m_numBoneInfos;

	// this structure describes the flipped and unflipped bases belonging to the bone
	struct BoneBasisGroup
	{
		CrySkinRigidBasisArray arrRight;// unflipped, Normal  = Tang ^ Binorm
		CrySkinRigidBasisArray arrLeft; // flipped, Normal = Binorm ^ Tang
		void reserve (unsigned numReserve)
		{
			arrRight.reserve (numReserve/2);
			arrLeft.reserve (numReserve/2);
		}
	};
	typedef std::vector<BoneBasisGroup> BoneBasisGroupArray;
	BoneBasisGroupArray m_arrBoneBases;

	// total number of bones used (max bone + 1)
	unsigned m_numBones;
	unsigned m_nFirstBone;

	// the destination interval: if the destination vertex is within this interval,
	// it is put into the skinner; the actual destination vertex index is dubtracted the interval begin
	unsigned m_nDestIntervalBegin, m_nDestIntervalEnd;

	/*
	unsigned m_nFlags;
	
	enum
	{
		flagForSSE = 1// if this is set, then the skin is constructed for SSE usage
	}
	*/
};

#endif