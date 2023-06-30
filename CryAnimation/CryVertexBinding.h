/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	11/05/2002 - Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//  Contains:
//  Declaration of CryVertexBinding, a class incapsulating the array of links of a vertex to bone.
//  This class is only used during construction of the geometry, and shouldn't be used for 
//  calculating the actual skin in the run time.
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_VERTEX_BINDING_HDR_
#define _CRY_VERTEX_BINDING_HDR_

// array of crylinks for one vertex
class CryVertexBinding: public std::vector<CryLink>
{
public:
	CryVertexBinding ();
	// scales all the link offsets multiplying the offset by the given scale
	void scaleOffsets(float fScale);
	// sorts the links by the blending factor, descending order
	void sortByBlendingDescending();
	// normalizes the weights of links so that they sum up to 1
	void normalizeBlendWeights();
	// prunes the weights that are less than the specified minimal blending factor.
	// Leaves (unpruned) at least the first numMinLinks links
	// ASSUMES:that the links are already sorted by the blending factors in descending order
	void pruneSmallWeights(float fMinBlending, unsigned numMinLinks = 1);
	// remaps the bone ids
	void remapBoneIds (const unsigned* arrBoneIdMap, unsigned numBoneIds);

	// returns the maximum BoneID in the array of links
	unsigned maxBoneID ()const;
	// returns the minimal BoneID in the array of links
	unsigned minBoneID () const;

	// returns the link weight to the given bone
	float getBoneWeight (int nBoneID) const;

	// returns true if there is such bone weight
	bool hasBoneWeight (int nBoneID, float fWeight) const;
};


#endif