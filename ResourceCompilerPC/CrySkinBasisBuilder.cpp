#include "stdafx.h"
#include "CrySkinRigidBasis.h"
#include "CrySkinBasisBuilder.h"

CrySkinBasisBuilder::CrySkinBasisBuilder (const ICrySkinSource* pGeometry, const Matrix44* pMatInvDef, unsigned numBoneInfos):
	CrySkinBuilderBase0 (pGeometry),
	m_pMatInvDef(pMatInvDef),
	m_numBoneInfos (numBoneInfos),
	m_nDestIntervalBegin (0),
	m_nDestIntervalEnd (0),
	m_numBones (0),
	m_nFirstBone (0)
{
}


//////////////////////////////////////////////////////////////////////////
// sets the destination vertex interval to operate on. Initially, this is infinity.
// The destination vertex interval is the interval within which the destination vertex
// index must lie in order to be skinned. The base of the interval is considered
// vertex 0 in the produced skinner
void CrySkinBasisBuilder::setDestinationInterval (unsigned nBegin, unsigned nEnd)
{
	if (nEnd > m_pGeometry->numExtTangents())
		nEnd = m_pGeometry->numExtTangents();

	if (nBegin < nEnd)
	{
		m_nDestIntervalBegin = nBegin;
		m_nDestIntervalEnd   = nEnd;

		preprocess();
		makeBoneBases ();
	}
	else
	{
		m_nDestIntervalBegin = 0;
		m_nDestIntervalEnd   = 0;
		m_numBones           = 0;
		m_nFirstBone         = 0;
	}
}


// initializes the given rigid basis builder
void CrySkinBasisBuilder::initRigidBasisSkin (CrySkinRigidBasis* pSkin)
{
	assert (sizeof(SPipTangentsA16)==0x30);
	unsigned numAuxInts = 2 * (m_numBones-m_nFirstBone);
	unsigned numBases = m_nDestIntervalEnd-m_nDestIntervalBegin;
	pSkin->init (2 * numBases, numAuxInts, m_nFirstBone, m_numBones);

	CrySkinStreams stream, streamBegin, streamEnd;
	streamBegin.pAux  = pSkin->m_arrAux.begin();
	streamBegin.pVert = pSkin->m_arrVertices.begin();
	stream = streamBegin;
	streamEnd.pAux  = streamBegin.pAux  + numAuxInts;
	streamEnd.pVert = streamBegin.pVert	+ 2 * numBases;

	for (unsigned nBone = m_nFirstBone; nBone < m_numBones; ++nBone)
	{
		// for each bone, fill the three groups
		// we fill the left and right vertices
		fillGroup (stream, m_arrBoneBases[nBone].arrRight);
		assert (stream.pAux <= streamEnd.pAux);
		assert (stream.pVert <= streamEnd.pVert);
		fillGroup (stream, m_arrBoneBases[nBone].arrLeft);
		// only when we processed the last bone, we should have the pAux pointing to the end
		assert (stream.pAux <= streamEnd.pAux);
		assert (stream.pVert <= streamEnd.pVert);
	}
}

void clipDenormal (float& x)
{
	if (x > -1e-4 && x < 1e-4)
		x = 0;
}

void clipDenormals(Vec3d& v)
{
	clipDenormal(v.x);
	clipDenormal(v.y);
	clipDenormal(v.z);
}

// makes up the basis array
void CrySkinBasisBuilder::makeBoneBases ()
{
	const unsigned* pExtToInt = m_pGeometry->getExtToIntMapEntries();

	m_arrBoneBases.clear();
	m_arrBoneBases.resize (m_numBones);

	unsigned i;
	// preallocate for push_back
	for (i = 0; i < m_numBones; ++i)
		m_arrBoneBases[i].reserve (m_nDestIntervalEnd / m_numBones);

	for (i = m_nDestIntervalBegin; i < m_nDestIntervalEnd; ++i)
	{
		unsigned nGeomVert = pExtToInt[i];

		TangData rBasis = m_pGeometry->getExtTangent (i);
		clipDenormals(rBasis.binormal);
		clipDenormals(rBasis.tangent);
		const CryVertexBinding& rLink = m_pGeometry->getLink(nGeomVert);
		unsigned nBone = rLink[0].BoneID;
		assert (nBone < m_numBoneInfos);

		CrySkinRigidBasisArray* pTargetArray;
		if ((rBasis.tangent ^ rBasis.binormal)*rBasis.tnormal > 0)
			pTargetArray = &m_arrBoneBases[nBone].arrRight;
		else
			pTargetArray = &m_arrBoneBases[nBone].arrLeft;

		const Matrix44& matInvDef = m_pMatInvDef[nBone];

		pTargetArray->push_back(CrySkinRigidBaseInfo (matInvDef,rBasis,i-m_nDestIntervalBegin));
	}
}


//////////////////////////////////////////////////////////////////////////
// calculate the number of used bones and the skip-bone
void CrySkinBasisBuilder::preprocess()
{
	unsigned numTangents = m_pGeometry->numExtTangents();
	const unsigned* pExtToInt = m_pGeometry->getExtToIntMapEntries();

	m_numBones = 0;
	m_nFirstBone = 0;
	bool bNotInited = true;

	for (unsigned i = m_nDestIntervalBegin; i < min(m_nDestIntervalEnd,numTangents); ++i)
	{
		unsigned nGeomVert = pExtToInt[i];
		const CryVertexBinding& rLink = m_pGeometry->getLink(nGeomVert);
		unsigned nBone = rLink[0].BoneID;
		
		if (bNotInited)
		{
			m_nFirstBone = nBone;
			m_numBones = nBone+1;
			bNotInited = false;
		}
		else
		{
			m_nFirstBone = min (m_nFirstBone, nBone);
			m_numBones = max (nBone+1, m_numBones);
		}
	}
}

// fills the given bases to the simple stream
typedef std::vector< CrySkinRigidBaseInfo > CrySkinRigidBasisArray;
void CrySkinBasisBuilder::fillGroup (CrySkinStreams& streams, const CrySkinRigidBasisArray& arrBases)
{
	// the group header is the number of vertices in the group
	*streams.pAux++ = (CrySkinAuxInt)arrBases.size();

	CrySkinRigidBasisArray::const_iterator it = arrBases.begin(), itEnd = it + arrBases.size();
	for (; it != itEnd; ++it)
	{
		it->build(streams.pVert);
		streams.pVert += 2;
	}
}
