#include "stdafx.h"
#include "StlUtils.h"
#include "CryGeomMorphTarget.h"


// create an identity morph target (doesn't morph anything)
CryGeomMorphTarget::CryGeomMorphTarget()
{
}


//////////////////////////////////////////////////////////////////////////
// load the morph target from the chunk/size. Returns 0 is the chunk is completely wrong,
// or the number of read bytes if it's ok (returns nSize if everything is fine)
unsigned CryGeomMorphTarget::load (unsigned nLOD, const MESHMORPHTARGET_CHUNK_DESC_0001* pChunk, unsigned nSize)
{
	if (nLOD >= g_nMaxLodLevels)
		return nSize; // pretend we have loaded everything, but ignore it

	// the chunk must at least contain its header and the name (min 2 bytes)
	if (nSize < sizeof(*pChunk) + 2)
		return 0;
	if (nSize < sizeof(*pChunk) + sizeof(SMeshMorphTargetVertex)*pChunk->numMorphVertices + 2)
		return 0; // we don't want truncated chunks

	m_arrVertex[nLOD].reinit (pChunk->numMorphVertices);
	const SMeshMorphTargetVertex* pSrcVertices = (const SMeshMorphTargetVertex*)(pChunk+1);
	if (pChunk->numMorphVertices)
		memcpy (&m_arrVertex[nLOD][0], pSrcVertices, sizeof(SMeshMorphTargetVertex)*pChunk->numMorphVertices);

	const char* pName = (const char*)(pSrcVertices+pChunk->numMorphVertices);
	const char* pNameEnd = ((const char*)pChunk)+nSize;

	setName (pName, pNameEnd-pNameEnd);

	return nSize;
}


// scales the target vertices by the given factor
void CryGeomMorphTarget::scale (unsigned nLOD,float fScale)
{
	if (nLOD >= g_nMaxLodLevels)
		return;

	MorphVertexArray::iterator it = m_arrVertex[nLOD].begin(), itEnd = it + m_arrVertex[nLOD].size();
	for (; it != itEnd; ++it)
		it->ptVertex *= fScale;
}


//////////////////////////////////////////////////////////////////////////
// given the source morph object, morphs it with the given weight to this morph target;
// 1 means the morph target will replace the target with itself; 0 means it will just copy the source
// into the destination (or leave it alone, if the two coincide)
// PARAMETERS:
//   numVertices -  the number of vertices in the source and destination buffers (to update)
//   fWeight     -  0..1, 0 means no morph, 1 means full morphed
void CryGeomMorphTarget::morph (unsigned nLOD, const Vec3d* pSrc, Vec3d* pDst, unsigned numVertices, float fWeight)const
{
	if (nLOD >= g_nMaxLodLevels)
	{
		// pretend we did everything
		if (pSrc != pDst)
			memcpy (pDst, pSrc, sizeof(Vec3d) * numVertices);
		return;
	}

	MorphVertexArray::const_iterator itMorphVertex = m_arrVertex[nLOD].begin(), itMorphVertexEnd = itMorphVertex + m_arrVertex[nLOD].size();
	if (pSrc == pDst)
	{
		// there's no need to copy them
		for (; itMorphVertex != itMorphVertexEnd; ++itMorphVertex)
			pDst[itMorphVertex->nVertexId] = pDst[itMorphVertex->nVertexId] * (1-fWeight) + itMorphVertex->ptVertex * fWeight;
	}
	else
	{
		// we must copy everything
		for (unsigned nVertex = 0; nVertex < numVertices; )
		{
			// copy until the next morphed vertex
			while (nVertex < itMorphVertex->nVertexId)
			{
				pDst[nVertex] = pSrc[nVertex];
				++nVertex;
			}
			// morph the current vertex
			pDst[itMorphVertex->nVertexId] = pSrc[itMorphVertex->nVertexId] * (1-fWeight) + itMorphVertex->ptVertex * fWeight;
			// go to the next vertex in the for loop
			++nVertex;

			if (++itMorphVertex == itMorphVertexEnd)
			{
				// finish the tail of array after the last morphed vertex
				for (; nVertex < numVertices; ++nVertex)
					pDst[nVertex] = pSrc[nVertex];
				break;
			}
		}
	}
}

// rotates the target morph vertices; transforms each vertex with the specified matrix, using only its
// ROTATIONAL components (no translation)
void CryGeomMorphTarget::rotate (unsigned nLOD, const Matrix44& tm)
{
	if (nLOD >= g_nMaxLodLevels)
		return; // everything is ok, we just assume this LOD is identity
	MorphVertexArray::iterator it = m_arrVertex[nLOD].begin(), itEnd = it + m_arrVertex[nLOD].size();
	for (; it != itEnd; ++it)

		//CHANGED_BY_IVO - INVALID CHANGE
		it->ptVertex = tm.TransformVectorOLD(it->ptVertex);
		//it->ptVertex = GetTransposed44(tm) * (it->ptVertex);
}

// transforms the target morph vertices with the given matrix
void CryGeomMorphTarget::transform (unsigned nLOD, const Matrix44& tm)
{
	if (nLOD >= g_nMaxLodLevels)
		return; // everything is ok, we just assume this LOD is identity
	MorphVertexArray::iterator it = m_arrVertex[nLOD].begin(), itEnd = it + m_arrVertex[nLOD].size();
	for (; it != itEnd; ++it)
		it->ptVertex = tm.TransformPointOLD(it->ptVertex);
}

const SMeshMorphTargetVertex* CryGeomMorphTarget::getMorphVertices (unsigned nLOD) const
{
	if (nLOD < g_nMaxLodLevels)
		return m_arrVertex[nLOD].begin();
	else
		return NULL;
}

unsigned CryGeomMorphTarget::numMorphVertices (unsigned nLOD) const
{
	if (nLOD < g_nMaxLodLevels)
		return (unsigned)m_arrVertex[nLOD].size();
	else
		return 0;
}

void CryGeomMorphTarget::setName (const char* szName, unsigned nMaxSize)
{
	// this is for the artists to distinguish between morph targets and animations
	// this symbol is also used for speeding up CryModelAnimationContainer::Find()
	m_strName = "#" + string(szName);
}
size_t CryGeomMorphTarget::sizeofThis()const
{
	unsigned nSize = sizeof(*this);
	nSize += m_strName.capacity();
	for (unsigned i = 0; i < g_nMaxLodLevels; ++i)
		nSize += sizeofArray (m_arrVertex[i]);
	return nSize;
}