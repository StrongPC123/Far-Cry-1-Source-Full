/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	22 Sep 2002 :- Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//  Contains:
//  class - loading context - responsible for loading the linearized bone hierarchy from the BONEANIM
//  chunk 
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_BONE_HIERARCHY_LOADER_HDR_
#define _CRY_BONE_HIERARCHY_LOADER_HDR_

#include "CryBoneDesc.h"

// this class is responsible for building the bone hierarchy array out of the chunk in file
// it does recursive building so that the hierarchy is continuous, each bone has continuous child array
class CryBoneHierarchyLoader
{
public:
	CryBoneHierarchyLoader();

	// loads and creates the array of bones;
	// returns number of bytes read if successful, 0 if not
	// updates the CryBoneDesc inv def matrices, if the initial pose matrices are already available
	unsigned load(const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize);

	// loads the default positions of each bone; if the bone chunk is loaded,
	// updates the bone inverse default pose matrices
	// returns number of bytes read if successful, 0 if not
	unsigned load (const BONEINITIALPOS_CHUNK_DESC_0001*pChunk, unsigned nChunkSize);

	void scale (float fScale);

	// maps the given index of the bone into the id with which the bone is identified in the file
	int mapIndexToId(int nIndex) const {return m_arrIndexToId[nIndex];}
	const int* getIndexToIdMap ()const {return &m_arrIndexToId[0];}
	int mapIdToIndex (int nId)const {return m_arrIdToIndex[nId];}
	const int* getIdToIndexMap ()const {return &m_arrIdToIndex[0];}
	
	// initializes the map id->index (inverse of mapIndexToId)
	void getIdToIndexMap (unsigned* pMap)const;

	const CryBoneDesc& getBoneByIndex (unsigned nIndex) const {return m_arrBones[nIndex];}

	// compares the two bone structures. Returns true if they're equal
	// (e.g. for validation of different lods)
	bool isEqual(const CryBoneHierarchyLoader& right)const;

	// returns the number of loaded bones, or 0 if no bones were loaded yet
	unsigned numBones() const {return m_pChunkBoneAnim ? m_pChunkBoneAnim->nBones : 0;}

	bool hasInitPos () const {return !m_arrInitPose.empty();}
	const Matrix44& getInitPosMatrixByIndex(int nBoneIndex){return m_arrInitPose[mapIndexToId(nBoneIndex)];}

	const char* getLastError() const {return m_szLastError;}

	// array of bones that's initialized in the constructor
	// this array is allocated and is not reallocated afterwards; it gives the number of entries in the id<->index map tables
	// The bones are in the internal (Index) indexation
	typedef std::vector<CryBoneDesc> CryBoneDescArray;
	CryBoneDescArray m_arrBones;

	// the array of default bone positions; it's empty if no 
	// init pos chunk was loaded; it doesn't matter in which
	// order the bones and init pose chunk are loaded:
	// when both are loaded, the bone descriptors receive the
	// inverse of these matrices, when they're available.
	// NOTE:
	//    This is in BoneID indexation
	std::vector<Matrix44> m_arrInitPose;
protected:
	// loads the whole hierarchy of bones, using the state machine
	bool load (int nBoneParentIndex, int nBoneIndex);

	// allocates the required number of bones in the plain hierarchy array, starting at the next available place
	int allocateBones(int numBones);

	// updates the bone InvDefGlobal matrices, if the default pose matrices
	// are available
	void updateInvDefGlobalMatrices();
protected:
	const BONEANIM_CHUNK_DESC* m_pChunkBoneAnim;
	unsigned m_pChunkBoneAnimSize;

	// the current raw data pointer
	const void* m_pBoneAnimRawData, *m_pBoneAnimRawDataEnd;

	// the currently free position in the array of bones
	int m_nNextBone;

	// the mapping bone index->bone id and vice versa
	// have the same size as m_arrBones; -1 means no entry
	std::vector<int> m_arrIndexToId, m_arrIdToIndex;

	const char* m_szLastError;
private:
};

#endif