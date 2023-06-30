#include "stdafx.h"
#include "CrySkinFull.h"
#include "CrySkinBuilder.h"


// initializes the builder for usage
CrySkinBuilder::CrySkinBuilder (const ICrySkinSource*pGeometry):
	m_arrSmoothVertHitCount ("CrySkinBuilder.arrSmoothVertHitCount"),
	CrySkinBuilderBase (pGeometry)
{
	makeFullBoneVertexArrays();

	// skip the non-infuencing bones
	for (m_numSkipBones = 0; m_numSkipBones < m_numBones && m_arrBoneVerts[m_numSkipBones].empty(); ++m_numSkipBones)
		continue;

	// we need the aux ints for:
	//  3 number per each bone to keep the number of groups
	//  0 numbers for the rigid vertices
	//  1 number for each smooth vertex
	// we don't count the bones that are skipped
	m_numAuxInts = 3 * (m_numBones-m_numSkipBones) + m_numSmoothLinks;
}

// initializes a new skin
void CrySkinBuilder::initSkinFull(CrySkinFull* pSkin)
{
	pSkin->init(m_numLinks, m_numAuxInts, m_numSkipBones, m_numBones);
	pSkin->m_numDests = m_pGeometry->numVertices();

	// this will hold the number of times a smooth vertex is hit
	m_arrSmoothVertHitCount.reinit (m_pGeometry->numVertices(), 0);

	CrySkinStreams stream, streamBegin, streamEnd;
	streamBegin.pAux  = pSkin->m_arrAux.begin();
	streamBegin.pVert = pSkin->m_arrVertices.begin();
	stream = streamBegin;
	streamEnd.pAux  = streamBegin.pAux  + m_numAuxInts;
	streamEnd.pVert = streamBegin.pVert	+ m_numLinks;

	for (unsigned nBone = m_numSkipBones; nBone < m_numBones; ++nBone)
	{
		// for each bone, fill the three groups
		// we start from the rigid vertices
		fillRigidGroup (stream, nBone);
		assert (stream.pAux <= streamEnd.pAux);
		assert (stream.pVert <= streamEnd.pVert);
		fillSmoothGroups (stream, nBone);
		// only when we processed the last bone, we should have the pAux pointing to the end
		assert (stream.pAux <= streamEnd.pAux);
		assert (stream.pVert <= streamEnd.pVert);
	}
	m_arrSmoothVertHitCount.clear();
	assert (stream.pAux == streamEnd.pAux);
	assert (stream.pVert == streamEnd.pVert);
#ifdef _DEBUG
	validate (pSkin);
	pSkin->validate (m_pGeometry);
#endif
}



//////////////////////////////////////////////////////////////////////////
// fills in 2 groups of aux ints for the given bone (the smooth vertex groups)
// returns the pointer to the next available auxint after the groups
// IMPLEMENTATION NOTE:
//  there are TWO groups filled in here
void CrySkinBuilder::fillSmoothGroups (CrySkinStreams& streams, unsigned nBone)
{
	// the group starts with the number of smooth vertices met the first time, belonging to this bone
	CrySkinAuxInt& numSmooth1Verts = *streams.pAux++;
	numSmooth1Verts = 0;

	// the list of smooth vertices belonging to the bone
	CrySkinSmoothVertexArray& arrSmooth = m_arrBoneVerts[nBone].arrSmooth;
	CrySkinSmoothVertexArray::const_iterator itBegin = arrSmooth.begin(), itEnd = itBegin + arrSmooth.size(), it = itBegin;

	// this will be the array of vertices met not for the first time -
	// we'll fill it in during processing of the Smooth1 group
	std::vector<CrySkinSmoothVertexArray::const_iterator> arrSmooth2Verts;
	arrSmooth2Verts.reserve (arrSmooth.size());

	// iterate through the smooth vertices, picking up Smooth1 vertices and memorizing Smooth2 vertices
	// hit each vertex, either smooth1 or smooth2
	for (; it != itEnd; ++it)
	{
		unsigned& nVertHitCount = m_arrSmoothVertHitCount[it->nDest];
#ifdef _DEBUG
		const CryVertexBinding& rLink = m_pGeometry->getLink(it->nDest);
		float fLegacyWeight = rLink.getBoneWeight(nBone);
		assert (rLink.hasBoneWeight(nBone, it->fWeight));
#endif
		// check which time the vertex is met
		if (nVertHitCount == 0)
		{
			// this vertex is met for the first time
			*streams.pAux++ = it->nDest;
			it->build (*streams.pVert++);
			++numSmooth1Verts;
		}
		else
			// this vertex is met not for the first time  - to be gone to the group Smooth2
			arrSmooth2Verts.push_back(it);
		
		// hit the vertex anyway
		++nVertHitCount;
	}

	CrySkinAuxInt& numSmooth2Verts = *streams.pAux++;
	// we already know the number of smooth2 group vertices
	numSmooth2Verts = (CrySkinAuxInt)arrSmooth2Verts.size();

	// now iterate through the smooth2 vertices
	for (unsigned i = 0; i < arrSmooth2Verts.size(); ++i)
	{
		arrSmooth2Verts[i]->build (*streams.pVert++);
		*streams.pAux++ = arrSmooth2Verts[i]->nDest;
	}
}


// validates the created skin
void CrySkinBuilder::validate (CrySkinFull *pSkin)
{
	CrySkinStreams stream;
	stream.pAux  = &pSkin->m_arrAux[0];
	stream.pVert = &pSkin->m_arrVertices[0];

	for (unsigned nBone = m_numSkipBones; nBone < pSkin->m_numBones; ++nBone)
	{
		/*
		BoneVertexGroup& rGroup = m_arrBoneVerts[nBone];
		assert (*stream.pAux++ == rGroup.arrRigid.size());

		stream.pVert += rGroup.arrRigid.size();
		*/
	}
}
