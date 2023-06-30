#include "stdafx.h"
#include "CryVertexBinding.h"


CryVertexBinding::CryVertexBinding()
#ifdef DEBUG_STD_CONTAINERS
	:std::vector<CryLink>("CryVertexBinding")
#endif
{
}

// normalizes the weights of links so that they sum up to 1
void CryVertexBinding::normalizeBlendWeights()
{
	// renormalize blending
	float fBlendSumm = 0;
	unsigned j;
	for (j = 0; j < size(); j++)
		fBlendSumm += (*this)[j].Blending;

	assert (fBlendSumm > 0.1f && fBlendSumm <=1.001f);

	for (j=0; j<size(); j++)
		(*this)[j].Blending /= fBlendSumm;
}


// prunes the weights that are less than the specified minimal blending factor
// ASSUMES: that the links are already sorted by the blending factors in descending order
void CryVertexBinding::pruneSmallWeights(float fMinBlending, unsigned numMinLinks)
{
  // remove 0 blending links and merge the links to the same bones
	unsigned j;
  for (j = numMinLinks; j < size(); j++)
	{
		assert (j == 0 || (*this)[j].Blending <= (*this)[j-1].Blending);
		if((*this)[j].Blending <= fMinBlending)
		{
			resize(j);
			assert(j);
			break;
		}
	}
	/*
	// the links to delete
	std::set<unsigned> setToDel;
	for (i = 0; i < size()-1; ++i)
		for (j = i+1; j < size(); ++j)
			if ((*this)[i].BoneID == (*this)[j].BoneID)
				setToDel.insert (j);

	// delete
	for (std::set<unsigned>::reverse_iterator it = setToDel.rbegin(); it != setToDel.rend(); ++it)
		this->erase (*it);
	*/
}


// remaps the bone ids
void CryVertexBinding::remapBoneIds (const unsigned* arrBoneIdMap, unsigned numBoneIds)
{
	for (iterator it = begin(); it != end(); ++it)
	{
		// if you get this assert, most probably there is dissynchronization between different LODs of the same model
		// - all of them must be exported with exactly the same skeletons.
		if(it->BoneID >= 0 && it->BoneID < (int)numBoneIds)
			it->BoneID = arrBoneIdMap[it->BoneID];
		else
		{
#ifdef _CRY_ANIMATION_BASE_HEADER_
			g_GetLog()->LogError ("\001bone index is out of range");
#endif
			it->BoneID = 0;
		}
	}
}


// scales all the link offsets multiplying the offset by the given scale
void CryVertexBinding::scaleOffsets(float fScale)
{
	for (iterator itLink = begin(); itLink != end(); ++itLink)
		itLink->offset *= fScale;
}


// sorts the links by the blending factor, descending order
void CryVertexBinding::sortByBlendingDescending()
{
	// sort the links by blend factor to allow skip unimportant ones
	std::sort (begin(), end(), CryLinkOrderByBlending());
}

// returns the maximum BoneID in the array of links
unsigned CryVertexBinding::maxBoneID ()const
{
	unsigned nResult = 0;
	for (unsigned i = 0; i < this->size(); ++i)
		nResult = max((unsigned)(*this)[i].BoneID, nResult);
	return nResult;
}

// returns the minimal BoneID in the array of links
unsigned CryVertexBinding::minBoneID () const
{
	unsigned nResult = (unsigned)(*this)[0].BoneID;
	for (unsigned i = 1; i < this->size(); ++i)
		nResult = min((unsigned)(*this)[i].BoneID, nResult);
	return nResult;
}

// returns the link weight to the given bone
float CryVertexBinding::getBoneWeight (int nBoneID)const
{
	for (unsigned i = 0; i < this->size(); ++i)
		if ((*this)[i].BoneID == nBoneID)
			return (*this)[i].Blending;
	return 0;
}

// returns true if there is such bone weight
bool CryVertexBinding::hasBoneWeight (int nBoneID, float fWeight) const
{
	for (unsigned i = 0; i < this->size(); ++i)
		if ((*this)[i].BoneID == nBoneID && (*this)[i].Blending == fWeight)
			return true;
	return false;
}
