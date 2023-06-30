#include "stdafx.h"
#include "CrySkinBuilderBase.h"

CrySkinBuilderBase::CrySkinBuilderBase(const ICrySkinSource* pGeometry):
	CrySkinBuilderBase0 (pGeometry)
#ifdef DEBUG_STD_CONTAINERS
	,m_arrBoneVerts ("CrySkinBuilder.arrBoneVerts")
#endif
{
	preprocess();
}


//////////////////////////////////////////////////////////////////////////
// fills in the group of aux ints for the given bone (the rigid vertex group)
// returns the pointer to the next available auxint after the group
// IMPLEMENTATION NOTE:
//  actually, the rigid group is empty in the auxiliary stream, so we just set the number of vertices
void CrySkinBuilderBase::fillRigidGroup (CrySkinStreams& streams, unsigned nBone)
{
	CrySkinRigidVertexArray& arrRigid = m_arrBoneVerts[nBone].arrRigid;

	// the group starts with the number of rigid vertices belonging to this bone
	*streams.pAux++ = (CrySkinAuxInt)arrRigid.size();

	CrySkinRigidVertexArray::const_iterator it = arrRigid.begin(), itEnd = it + arrRigid.size();

  for (; it != itEnd; ++it)
		it->build (*streams.pVert++);
}

//////////////////////////////////////////////////////////////////////////
// computes the max number of bone affecting a vertex + 1
// and the total number of links to smooth vertices - the sum of numbers of links of all smooth vertices
// the total number of links per all vertices. If the whole object is rigid,
// this is the same as the number of vertices
void CrySkinBuilderBase::preprocess()
{
	// we need for each bone:
	// Vert:
	//  RigidVertex for each rigid vertex, and SmoothVertex for each smooth vertex
	// Aux:
	//  3 counts of vertices in each group
	//  index of target for each smooth vertex

	m_numBones = 0;
	m_numSmoothLinks = 0;
	m_numLinks = 0;
	unsigned numVertices = m_pGeometry->numVertices();
	for (unsigned nVert = 0; nVert < numVertices; ++nVert)
	{
		// the number of links for this vertex
		unsigned numVertexLinks = (unsigned)m_pGeometry->getLink(nVert).size();
		if (numVertexLinks > 1)
			// if the vertex is smooth, we'll need to keep the weights and destination indices
			m_numSmoothLinks += numVertexLinks;

		m_numBones = max(m_pGeometry->getLink(nVert).maxBoneID(), m_numBones);
		
		m_numLinks += numVertexLinks;
	}
	++m_numBones;
}


//////////////////////////////////////////////////////////////////////////
// calculates the vertex list of each bone
void CrySkinBuilderBase::makeFullBoneVertexArrays()
{
	m_arrBoneVerts.clear();
	m_arrBoneVerts.resize (m_numBones);

	unsigned numVertices = m_pGeometry->numVertices();
	// preallocate memory for the vertices in bones
	for (unsigned i = 0; i < m_numBones; ++i)
		// assume approximately uniform distribution
		m_arrBoneVerts[i].reserve (numVertices / m_numBones);

	for (unsigned nVert = 0; nVert < numVertices; ++nVert)
	{
		// the number of links for this vertex
		const CryVertexBinding& rLinks = m_pGeometry->getLink (nVert);
		if (rLinks.size() == 1)
			m_arrBoneVerts[rLinks[0].BoneID].arrRigid.push_back(CrySkinRigidVertex (rLinks[0].offset, nVert));
		else
		for (unsigned i = 0; i < rLinks.size(); ++i)
		{
			m_arrBoneVerts[rLinks[i].BoneID].arrSmooth.push_back(CrySkinSmoothVertex (rLinks[i], nVert));
		}
	}
}
