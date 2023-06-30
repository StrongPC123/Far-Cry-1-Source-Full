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

#include "stdafx.h"
#include "MathUtils.h"
#include "CryBoneHierarchyLoader.h"
#include "ChunkFileReader.h"
#include "CgfUtils.h"

CryBoneHierarchyLoader::CryBoneHierarchyLoader ():
#ifdef DEBUG_STD_CONTAINERS
	m_arrBones("CryBoneHierarchyLoader.Bones"),
	m_arrIdToIndex ("CryBoneHierarchyLoader.indexidmaps"),
	m_arrIndexToId ("CryBoneHierarchyLoader.indexidmaps"),
#endif
	m_pChunkBoneAnim(NULL),
	m_pChunkBoneAnimSize(0),

	m_pBoneAnimRawData (NULL),
	m_pBoneAnimRawDataEnd (NULL),
	m_szLastError ("")
{
}

unsigned CryBoneHierarchyLoader::load (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize)
{
	m_arrBones.clear();
	m_arrIndexToId.clear();
	m_arrIdToIndex.clear();

	m_pChunkBoneAnim = pChunk;
	m_pChunkBoneAnimSize = nChunkSize;

	m_pBoneAnimRawData = pChunk+1;
	m_pBoneAnimRawDataEnd = ((const char*) pChunk) + nChunkSize;

	if (nChunkSize < sizeof(*pChunk))
	{
		m_szLastError = "Couldn't read the data."	;
		m_pBoneAnimRawData = pChunk;
		return 0;
	}

	if (pChunk->nBones <= 0)
	{
		m_szLastError = "There must be at least one bone.";
		m_pBoneAnimRawData = pChunk;
		return 0;
	}

	const BONE_ENTITY* pBones = (const BONE_ENTITY*) (pChunk+1);
	if (m_pBoneAnimRawDataEnd < (const byte*)(pBones + pChunk->nBones))
	{
		m_szLastError = "Premature end of data.";
		return 0;
	}

	// check for one-and-only-one root rule
	if (pBones[0].ParentID != -1)
	{
		m_szLastError = "The first bone in the hierarchy has a parent, but it is expected to be the root bone.";
		return 0;
	}

	for (int i = 1; i < pChunk->nBones; ++i)
	{
		if (pBones[i].ParentID == -1)
		{
			m_szLastError = "The skeleton has multiple roots. Only single-rooted skeletons are supported in this version.";
			return 0;
		}
	}

	m_arrBones.resize (numBones());
	m_arrIndexToId.resize (numBones(), -1);
	m_arrIdToIndex.resize (numBones(), -1);
	m_nNextBone = 0;

	int nRootBoneIndex = (int)allocateBones (1);

	if (nRootBoneIndex < 0 || !load(nRootBoneIndex, nRootBoneIndex))
		m_pBoneAnimRawData = pChunk;

	updateInvDefGlobalMatrices();
	return (const byte*)m_pBoneAnimRawData - (const byte*)m_pChunkBoneAnim;
}

// updates the bone InvDefGlobal matrices, if the default pose matrices
// are available
void CryBoneHierarchyLoader::updateInvDefGlobalMatrices()
{
	if (m_arrInitPose.empty() || m_arrBones.empty())
		return;

	if (m_arrInitPose.size() != m_arrBones.size())
	{
#ifdef _CRY_ANIMATION_BASE_HEADER_
		g_GetLog()->LogError ("\003there are %d bones and %d initial pose matrices. ignoring matrices.", m_arrBones.size(), m_arrInitPose.size());
#endif
		return;
	}

	for (unsigned nBone = 0; nBone < m_arrBones.size(); ++nBone)
	{
		unsigned nBoneID = mapIndexToId(nBone);
		m_arrBones[nBone].setDefaultGlobal (m_arrInitPose[nBoneID]);
	}
}


// loads the default positions of each bone; if the bone chunk is loaded,
// updates the bone inverse default pose matrices
// returns number of bytes read if successful, 0 if not
unsigned CryBoneHierarchyLoader::load (const BONEINITIALPOS_CHUNK_DESC_0001*pChunk, unsigned nChunkSize)
{
	if (nChunkSize < sizeof(*pChunk) || pChunk->numBones < 0 || pChunk->numBones > 0x10000)
		return 0;
	const char* pChunkEnd = ((const char*)pChunk) + nChunkSize;
	const SBoneInitPosMatrix* pDefMatrix = (const SBoneInitPosMatrix*)(pChunk+1);

	// the end of utilized data in the chunk
	const char* pUtilizedEnd = (const char*)(pDefMatrix + pChunk->numBones);
	if (pUtilizedEnd > pChunkEnd)
		return 0;

	m_arrInitPose.resize (pChunk->numBones);
	for (unsigned nBone = 0; nBone < pChunk->numBones; ++nBone)
	{
		Matrix44& matBone = m_arrInitPose[nBone];
		copyMatrix (matBone,pDefMatrix[nBone]);
		// for some reason Max supplies unnormalized matrices.
		matBone.NoScale();
	}

	updateInvDefGlobalMatrices();
	return pUtilizedEnd - (const char*)pChunk;
}

void CryBoneHierarchyLoader::scale (float fScale)
{
	unsigned i;
	for (i = 0; i < m_arrInitPose.size(); ++i)
		m_arrInitPose[i].ScaleTranslationOLD (fScale);
	
	updateInvDefGlobalMatrices();
}

// allocates the required number of bones in the plain hierarchy array, starting at the next available place
// -1 if couldn't allocate
int CryBoneHierarchyLoader::allocateBones(int numBones)
{
	if (m_nNextBone + numBones > (int)this->numBones())
		return -1; // the request is for too many bones

	int nResult = m_nNextBone;
	m_nNextBone += numBones;
	assert (m_nNextBone <= (int)this->numBones());
	return nResult;
}


// loads the whole hierarchy of bones, using the state machine
// when this funciton is called, the bone is already allocated
bool CryBoneHierarchyLoader::load (int nBoneParentIndex, int nBoneIndex)
{
	const BONE_ENTITY* pEntity;
	if (!EatRawDataPtr(pEntity, 1, m_pBoneAnimRawData, m_pBoneAnimRawDataEnd))
		return false;

	// initialize the next bone
	CryBoneDesc& rBoneDesc = m_arrBones[nBoneIndex];
	if (!rBoneDesc.LoadRaw (pEntity))
		return false;

	// set the mapping entries
	m_arrIndexToId[nBoneIndex] = pEntity->BoneID;
	m_arrIdToIndex[pEntity->BoneID] = nBoneIndex;

	rBoneDesc.m_nOffsetParent = nBoneParentIndex - nBoneIndex;

	// load children
	if (pEntity->nChildren)
	{
		int nChildrenIndexBase = allocateBones(pEntity->nChildren);
		if (nChildrenIndexBase < 0)
			return false;

		// load the children
		rBoneDesc.m_numChildren = pEntity->nChildren;
		rBoneDesc.m_nOffsetChildren = nChildrenIndexBase - nBoneIndex;

		for (int nChild = 0; nChild < pEntity->nChildren; ++nChild)
		{
			if (!load(nBoneIndex, nChildrenIndexBase + nChild))
				return false;
		}
	}
	else
	{
		rBoneDesc.m_numChildren = 0;
		rBoneDesc.m_nOffsetChildren = 0;
	}
	return true;
}


// compares the two bone structures. Returns true if they're equal
// (e.g. for validation of different lods)
bool CryBoneHierarchyLoader::isEqual (const CryBoneHierarchyLoader& right)const
{
	if (m_arrBones.size() != right.m_arrBones.size())
		return false;
	for (unsigned nBone = 0; nBone < m_arrBones.size(); ++nBone)
	{
		if (!m_arrBones[nBone].isEqual(right.m_arrBones[nBone]))
			return false;
	}
	return true;
}
