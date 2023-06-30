#include "stdafx.h"
#include "CrySkinMorphBuilder.h"
#include "CrySkinMorph.h"

CrySkinMorphBuilder::CrySkinMorphBuilder(const ICrySkinSource* pGeometry, const Matrix44* pMatInvDef, unsigned numBones):
	CrySkinBuilderBase (pGeometry),
	m_pMatInvDef(pMatInvDef),
	m_numBones(numBones)
{
}


//////////////////////////////////////////////////////////////////////////
// initializes the given skin out of the given morph target
void CrySkinMorphBuilder::initSkinMorph (const SMeshMorphTargetVertex* pMorphVerts, unsigned numMorphVerts, class CrySkinMorph* pSkin)
{
	m_pMorphVerts = pMorphVerts;
	m_numMorphVerts = numMorphVerts;
	m_pSkinTarget = pSkin;
	pSkin->m_numDests = m_pGeometry->numVertices();

	if (!numMorphVerts)
	{
		pSkin->clear();
		return;
	}

	findAffectingBoneRange();

	makeMorphBoneVertexArray();

	calculateNumMorphLinks();

	validate();

	// for aux ints, we need, for each bone:
	// 2 ints for group headers
	// for each smooth link,
	// 1 int for index of the target vertex
	unsigned numAuxInts = 2 * (m_numAffectingBones - m_nFirstAffectingBone) + m_numMorphSmoothLinks;

	pSkin->init (m_numMorphRigidLinks + m_numMorphSmoothLinks,
		numAuxInts,
		m_nFirstAffectingBone,
		m_numAffectingBones);

	CrySkinStreams stream, streamBegin, streamEnd;
	streamBegin.pAux  = pSkin->m_arrAux.begin();
	streamBegin.pVert = pSkin->m_arrVertices.begin();
	stream = streamBegin;
	streamEnd.pAux  = streamBegin.pAux  + numAuxInts;
	streamEnd.pVert = streamBegin.pVert	+ m_numMorphRigidLinks + m_numMorphSmoothLinks;

	for (unsigned nBone = m_nFirstAffectingBone; nBone < m_numAffectingBones; ++nBone)
	{
		// for each bone, fill the three groups
		// we start from the rigid vertices
		fillRigidGroup (stream, nBone);
		assert (stream.pAux <= streamEnd.pAux);
		assert (stream.pVert <= streamEnd.pVert);
		fillSmoothGroup (stream, nBone);
		// only when we processed the last bone, we should have the pAux pointing to the end
		assert (stream.pAux <= streamEnd.pAux);
		assert (stream.pVert <= streamEnd.pVert);
	}
}


//////////////////////////////////////////////////////////////////////////
// for the given morph target(source) finds and initializes the
// m_nFirstAffectingBone and m_numAffectingBones
void CrySkinMorphBuilder::findAffectingBoneRange()
{
	// init - find the first vertex binding info
	const CryVertexBinding* pLinks = &m_pGeometry->getLink(m_pMorphVerts[0].nVertexId);
	m_nFirstAffectingBone = pLinks->minBoneID();
	m_numAffectingBones = pLinks->maxBoneID()+1;

	// find all the other vertex binding info and init the bone range
	for (unsigned nMorphVert = 1; nMorphVert < m_numMorphVerts; ++nMorphVert)
	{
		unsigned nGeomVert = m_pMorphVerts[nMorphVert].nVertexId;
		pLinks = &m_pGeometry->getLink(nGeomVert);
		m_nFirstAffectingBone = min (m_nFirstAffectingBone, pLinks->minBoneID());
		m_numAffectingBones = max(m_numAffectingBones, pLinks->maxBoneID()+1);
	}

	assert (m_nFirstAffectingBone < m_numAffectingBones);
	assert (m_numAffectingBones <= m_numBones);
}


//////////////////////////////////////////////////////////////////////////
// calculates the number of rigid and smooth links
// m_numMorphRigidLinks and m_numMorphSmoothLinks
void CrySkinMorphBuilder::calculateNumMorphLinks()
{
	m_numMorphRigidLinks  = 0;
	m_numMorphSmoothLinks = 0;
	for (unsigned nMorphVert = 0; nMorphVert < m_numMorphVerts; ++nMorphVert)
	{
		unsigned nGeomVert = m_pMorphVerts[nMorphVert].nVertexId;
		const CryVertexBinding* pLinks = &m_pGeometry->getLink(nGeomVert);

		if (pLinks->size() == 1)
			++m_numMorphRigidLinks;
		else
			m_numMorphSmoothLinks += (unsigned)pLinks->size();
	}
}


//////////////////////////////////////////////////////////////////////////
// calculates the vertex list of each bone, taking only those vertices 
// present in the morph vertex array
void CrySkinMorphBuilder::makeMorphBoneVertexArray()
{
	m_arrBoneVerts.clear();
	m_arrBoneVerts.resize (m_numAffectingBones);
	for (unsigned nMorphVert = 0; nMorphVert < m_numMorphVerts; ++nMorphVert)
	{
		unsigned nGeomVert = m_pMorphVerts[nMorphVert].nVertexId;
		const CryVertexBinding* pLinks = &m_pGeometry->getLink(nGeomVert);
		// the morph vertex target in the object coordinates
		Vec3d ptMorphOffset = m_pMorphVerts[nMorphVert].ptVertex - m_pGeometry->getVertex(nGeomVert);
		
		if (pLinks->size() == 1)
		{
			unsigned nBone = (*pLinks)[0].BoneID;
			CrySkinRigidVertex v;
			v.nDest = nGeomVert;
			// transform the point into the bone coordinates

			//CHANGED_BY_IVO - INVALID CHANGE, PLEASE REVISE
			v.pt = m_pMatInvDef[nBone].TransformVectorOLD (ptMorphOffset);
			//v.pt = GetTransposed44(m_pMatInvDef[nBone])*(ptMorphOffset);

			m_arrBoneVerts[nBone].arrRigid.push_back(v);
		}
		else
		{
			for (unsigned nLink = 0; nLink < pLinks->size(); ++nLink)
			{
				unsigned nBone = (*pLinks)[nLink].BoneID;
				CrySkinSmoothVertex v;
				v.fWeight = (*pLinks)[nLink].Blending;
				v.nDest = nGeomVert;

				v.pt = m_pMatInvDef[nBone].TransformVectorOLD (ptMorphOffset);

				m_arrBoneVerts[nBone].arrSmooth.push_back(v);
			}
		}
	}
}


void CrySkinMorphBuilder::validate()
{
#ifdef _DEBUG
	unsigned nBone;
	for (nBone = 0; nBone < m_nFirstAffectingBone;++nBone)
		assert (m_arrBoneVerts[nBone].empty());
	assert (!m_arrBoneVerts[m_nFirstAffectingBone].empty());
#endif
}


// fills in the group of aux ints for the given bone (the smooth vertex group)
// returns the pointer to the next available auxint after the group
void CrySkinMorphBuilder::fillSmoothGroup (CrySkinStreams& streams, unsigned nBone)
{
	CrySkinSmoothVertexArray& arrSmooth = m_arrBoneVerts[nBone].arrSmooth;

	// the group starts with the number of rigid vertices belonging to this bone
	*streams.pAux++ = (CrySkinAuxInt)arrSmooth.size();

	CrySkinSmoothVertexArray::const_iterator it = arrSmooth.begin(), itEnd = it + arrSmooth.size();

  for (; it != itEnd; ++it)
	{
		it->build (*streams.pVert++);
		*streams.pAux++ = it->nDest;
	}
}
