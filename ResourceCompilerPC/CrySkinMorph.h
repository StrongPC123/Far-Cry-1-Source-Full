#ifndef _CRY_ANIMATION_CRY_SKIN_MORPH_HDR_
#define _CRY_ANIMATION_CRY_SKIN_MORPH_HDR_

#include "CrySkinBase.h"

//////////////////////////////////////////////////////////////////////////
// This skinner is capable only of morphing a few vertices and adding their
// displacements to the destination vertex array
class CrySkinMorph: public CrySkinBase
{
public:
	// does the skinning out of the given array of global matrices:
	// adds the corresponding displacements with the given weight
	void skin (const Matrix44* pBones, float fWeight, Vec3d* pDest)const;

	// does the skinning out of the given array of global matrices:
	// adds the corresponding displacements with the given weight
	// also tries to estimate the changes in normals
	void skin (const Matrix44* pBones, float fWeight, Vec3d* pDest, Vec3dA16* pDestNormalsA16, float fAmplify = 1) const;

	void scale (float fScale)
	{
		// to scale it, we just need to proportionally scale each vertex x,y,z
		scaleVertices(fScale);
	}

	friend class CrySkinMorphBuilder;

	class CStatistics: public CrySkinBase::CStatistics
	{
	public:
		CStatistics (const CrySkinMorph* pSkin):
			CrySkinBase::CStatistics(pSkin)
			{
				init(pSkin);
			}
		void init(const CrySkinMorph* pSkin);
		void addOffset (const Vec3d& v);

		// destination vertex set
		std::set<unsigned> setDests;
		// number of rigid and smooth vertices
		unsigned numRigid, numSmooth;
		// minimum and maximum offset length
		float fMinOffset, fMaxOffset;
	};
};

#endif