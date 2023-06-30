#ifndef _CRY_ANIMATION_CRY_SKIN_MORPH_BUILDER_HDR_
#define _CRY_ANIMATION_CRY_SKIN_MORPH_BUILDER_HDR_

#include "CrySkinBuilderBase.h"
class CrySkinMorph;

// builds the skin morph
class CrySkinMorphBuilder: public CrySkinBuilderBase
{
public:
	// Receives the geometry and the array of inverse-default-global matrices
	// 1 matrix per 1 bone
	CrySkinMorphBuilder(const ICrySkinSource* pGeometry, const Matrix44* pMatInvDef, unsigned numBones);

	// initializes the given skin out of the given morph target
	void initSkinMorph (const SMeshMorphTargetVertex* pMorphVerts, unsigned numMorphVerts, class CrySkinMorph* pSkin);

protected:
	// for the given morph target(source) finds and initializes the m_nFirstAffectingBone and m_numAffectingBones
	void findAffectingBoneRange();

	// calculates the number of rigid and smooth links
	void calculateNumMorphLinks();

	// calculates the vertex list of each bone, taking only those vertices 
	// present in the morph vertex array
	void makeMorphBoneVertexArray();

	void validate();


	// fills in the group of aux ints for the given bone (the smooth vertex group)
	// returns the pointer to the next available auxint after the group
	void fillSmoothGroup (CrySkinStreams& streams, unsigned nBone);
protected:
	// the bone info that's used to translate the 
	// the matrices are from getInvDefGlobal() of each corresponding CryBoneInfo
	const Matrix44* m_pMatInvDef;
	unsigned m_numBones;

	// this is set temporarily during initialization of the skin out of the morph target structure
	const SMeshMorphTargetVertex* m_pMorphVerts;
	unsigned m_numMorphVerts;
	CrySkinMorph* m_pSkinTarget;

	// the first bone affecting the morph target
	unsigned m_nFirstAffectingBone;
	// the number of bones that are to be taken into account, does NOT depend on m_numFirstAffectingBone
	unsigned m_numAffectingBones;

	// the total number of rigid links for the current morph target
	unsigned m_numMorphRigidLinks;
	// the total number of smooth links for the current morph target
	unsigned m_numMorphSmoothLinks;
};

#endif