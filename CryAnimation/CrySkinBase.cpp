#include "stdafx.h"
#include <StlUtils.h>
#include "CrySkinBase.h"

CrySkinBase::CrySkinBase():
	m_numBones(0),
	m_arrVertices ("CrySkin*.Vertices"),
	m_arrAux ("CrySkin*.Aux"),
	m_numSkipBones (0),
	m_numDests(0)
{
}

CrySkinBase::~CrySkinBase ()
{
}

void CrySkinBase::clear()
{
	m_numBones = 0;
	m_numSkipBones = 0;
	m_arrVertices.clear();
	m_arrAux.clear();
}

bool CrySkinBase::empty()const
{
	return m_numBones == 0 || m_arrVertices.empty();
}

void CrySkinBase::init (unsigned numVerts, unsigned numAux, unsigned numSkipBones, unsigned numBones)
{
	m_numBones = numBones;
	m_numSkipBones = numSkipBones;
	// (re)allocate the memory
	m_arrVertices.reinit (numVerts);
	m_arrAux.reinit (numAux);
}

// transforms the given smooth point into the destination with the matrix
void CrySkinBase::transformWPoint (Vec3d& pDest, const Matrix44& matBone, const Vertex& rVtx)
{
	//pDest = matBone.TransformPoint(rVtx) * rVtx.fWeight;
	
  pDest.x = ((matBone[0][0] * rVtx.pt.x) + (matBone[1][0] * rVtx.pt.y) + (matBone[2][0] * rVtx.pt.z) + matBone[3][0]) * rVtx.fWeight;
  pDest.y = ((matBone[0][1] * rVtx.pt.x) + (matBone[1][1] * rVtx.pt.y) + (matBone[2][1] * rVtx.pt.z) + matBone[3][1]) * rVtx.fWeight;
  pDest.z = ((matBone[0][2] * rVtx.pt.x) + (matBone[1][2] * rVtx.pt.y) + (matBone[2][2] * rVtx.pt.z) + matBone[3][2]) * rVtx.fWeight;
}


// adds the given smooth point into the destination with the matrix
void CrySkinBase::addWPoint (Vec3d& pDest, const Matrix44& matBone, const Vertex& rVtx)
{
	//pDest += matBone.TransformPoint(rVtx) * rVtx.fWeight;

  pDest.x += ((matBone[0][0] * rVtx.pt.x) + (matBone[1][0] * rVtx.pt.y) + (matBone[2][0] * rVtx.pt.z) + matBone[3][0]) * rVtx.fWeight;
  pDest.y += ((matBone[0][1] * rVtx.pt.x) + (matBone[1][1] * rVtx.pt.y) + (matBone[2][1] * rVtx.pt.z) + matBone[3][1]) * rVtx.fWeight;
  pDest.z += ((matBone[0][2] * rVtx.pt.x) + (matBone[1][2] * rVtx.pt.y) + (matBone[2][2] * rVtx.pt.z) + matBone[3][2]) * rVtx.fWeight;
}

// transforms the given _vector_ without applying the transitional part
/*
void CrySkinBase::transformVectorNoTrans (Vec3d& pDest, const Vec3d& pSrc, const Matrix& matBone)
{
	pDest.x = matBone[0][0] * pSrc.x + matBone[1][0] * pSrc.y + matBone[2][0] * pSrc.z;
	pDest.y = matBone[0][1] * pSrc.x + matBone[1][1] * pSrc.y + matBone[2][1] * pSrc.z;
	pDest.z = matBone[0][2] * pSrc.x + matBone[1][2] * pSrc.y + matBone[2][2] * pSrc.z;
}
*/

// transforms the given smooth point into the destination with the matrix
void CrySkinBase::transformWVector (Vec3d& pDest, const Matrix44& matBone, const Vertex& rVtx)
{
	//pDest = matBone.TransformPoint(rVtx) * rVtx.fWeight;
	
  pDest.x = ((matBone[0][0] * rVtx.pt.x) + (matBone[1][0] * rVtx.pt.y) + (matBone[2][0] * rVtx.pt.z)) * rVtx.fWeight;
  pDest.y = ((matBone[0][1] * rVtx.pt.x) + (matBone[1][1] * rVtx.pt.y) + (matBone[2][1] * rVtx.pt.z)) * rVtx.fWeight;
  pDest.z = ((matBone[0][2] * rVtx.pt.x) + (matBone[1][2] * rVtx.pt.y) + (matBone[2][2] * rVtx.pt.z)) * rVtx.fWeight;
}


// adds the given smooth point into the destination with the matrix
void CrySkinBase::addWVector (Vec3d& pDest, const Matrix44& matBone, const Vertex& rVtx)
{
	//pDest += matBone.TransformPoint(rVtx) * rVtx.fWeight;

  pDest.x += ((matBone[0][0] * rVtx.pt.x) + (matBone[1][0] * rVtx.pt.y) + (matBone[2][0] * rVtx.pt.z)) * rVtx.fWeight;
  pDest.y += ((matBone[0][1] * rVtx.pt.x) + (matBone[1][1] * rVtx.pt.y) + (matBone[2][1] * rVtx.pt.z)) * rVtx.fWeight;
  pDest.z += ((matBone[0][2] * rVtx.pt.x) + (matBone[1][2] * rVtx.pt.y) + (matBone[2][2] * rVtx.pt.z)) * rVtx.fWeight;
}

// returns the number of bytes occupied by this structure and all its contained objects
unsigned CrySkinBase::sizeofThis()const
{
	return sizeof(*this) + sizeofArray (m_arrAux) + sizeofArray(m_arrVertices);
}



//------------------------------------------------------------------------------
//-----  this is the serialize version for little-endian CPUs              -----
//------------------------------------------------------------------------------
unsigned CrySkinBase::Serialize_PC (bool bSave, void* pStream, unsigned nBufSize)
{
	SerialHeader Header;
	if (bSave)
	{
		Header.initFromSkin(this);
	}
	else
	{
		if (!pStream)
			return 0;
		if (nBufSize < sizeof(Header))
			return 0;
		Header = *((SerialHeader*)pStream);
	}

	unsigned nSizeAuxInts = ((sizeof(CrySkinAuxInt)*Header.numAuxInts+3)&~3);
	unsigned nSizeRequired = 
		// the m_numBones, m_numSkipBones, m_numDests
		// m_arrAux.size(), m_arrVertices.size()
		sizeof(SerialHeader) +
		nSizeAuxInts +
		sizeof(Vertex) * Header.numVertices;									 

	if (!pStream)
		return bSave?nSizeRequired:0;

	if (nBufSize < nSizeRequired)
		return 0;

	CrySkinAuxInt* pAuxInts = (CrySkinAuxInt*)(((SerialHeader*)pStream) + 1);
	Vertex* pVertices = (Vertex*)(((char*)pAuxInts) + nSizeAuxInts);

	if (bSave)
	{
		//saving
		*((SerialHeader*)pStream) = Header;
		memcpy (pAuxInts, &m_arrAux[0], sizeof(CrySkinAuxInt)*m_arrAux.size());
		memcpy (pVertices, &m_arrVertices[0], sizeof(Vertex)*m_arrVertices.size());
	}
	else
	{
		// loading
		m_arrAux.clear();
		m_arrAux.resize (Header.numAuxInts);
		m_arrVertices.clear();
		m_arrVertices.resize (Header.numVertices);
		m_numBones = Header.numBones;
		m_numSkipBones = Header.numSkipBones;
		m_numDests = Header.numDests;
		memcpy (&m_arrAux[0], pAuxInts, sizeof(CrySkinAuxInt)*m_arrAux.size());
		memcpy (&m_arrVertices[0], pVertices, sizeof(Vertex)*m_arrVertices.size());
	}
	return nSizeRequired;

}


//------------------------------------------------------------------------------
//-----     this is the serialize version for big-endian CPUs              -----
//------------------------------------------------------------------------------
unsigned CrySkinBase::Serialize_GC (bool bSave, void* pStream, unsigned nBufSize)
{
	SerialHeader Header;
	if (bSave)
	{
		Header.initFromSkin(this);
	}
	else
	{
		if (!pStream)
			return 0;
		if (nBufSize < sizeof(Header))
			return 0;
		Header = *((SerialHeader*)pStream);
	}

	unsigned nSizeAuxInts = ((sizeof(CrySkinAuxInt)*Header.numAuxInts+3)&~3);
	unsigned nSizeRequired = 
		// the m_numBones, m_numSkipBones, m_numDests
		// m_arrAux.size(), m_arrVertices.size()
		sizeof(SerialHeader) +
		nSizeAuxInts +
		sizeof(Vertex) * Header.numVertices;									 

	if (!pStream)
		return bSave?nSizeRequired:0;

	if (nBufSize < nSizeRequired)
		return 0;

	CrySkinAuxInt* pAuxInts = (CrySkinAuxInt*)(((SerialHeader*)pStream) + 1);
	Vertex* pVertices				= (Vertex*)(((char*)pAuxInts) + nSizeAuxInts);

	if (bSave) 
	{
		//saving
		Header.numBones			=	SWAP32(Header.numBones);
		Header.numSkipBones	=	SWAP32(Header.numSkipBones);
		Header.numDests			=	SWAP32(Header.numDests);
		Header.numAuxInts		=	SWAP32(Header.numAuxInts);
		Header.numVertices	=	SWAP32(Header.numVertices);
		*((SerialHeader*)pStream) = Header;

		//swap data into big-endian-format
		unsigned sa	= sizeof(CrySkinAuxInt);
		unsigned a		=	(unsigned)m_arrAux.size();
		unsigned x;
		for (x=0; x<a; x++) {
			pAuxInts[x]=SWAP16(pAuxInts[x]);
		}
		unsigned va	=	sizeof(m_arrVertices.size());
		unsigned v		=	(unsigned)m_arrVertices.size();
		for (x=0; x<(v); x++) {
			pVertices[x].nDest		=	SWAP32(pVertices[x].nDest);
			pVertices[x].pt.x			=	FSWAP32(pVertices[x].pt.x);
			pVertices[x].pt.y			=	FSWAP32(pVertices[x].pt.y);
			pVertices[x].pt.z			=	FSWAP32(pVertices[x].pt.z);
		}

		//copy into stream 
		memcpy (pAuxInts,		&m_arrAux[0],				sizeof(CrySkinAuxInt)*m_arrAux.size());
		memcpy (pVertices,	&m_arrVertices[0],	sizeof(Vertex)*m_arrVertices.size());
   
		//..and new swap-back to little-endian,
		//because we need the original format for futher calculation on PC
		for (x=0; x<a; x++) {
			pAuxInts[x]=SWAP16(pAuxInts[x]);
		}
		for (x=0; x<(v); x++) {
			pVertices[x].nDest		=	SWAP32(pVertices[x].nDest);
			pVertices[x].pt.x			=	FSWAP32(pVertices[x].pt.x);
			pVertices[x].pt.y			=	FSWAP32(pVertices[x].pt.y);
			pVertices[x].pt.z			=	FSWAP32(pVertices[x].pt.z);
		}

	}
	else
	{
		// loading
		m_arrAux.clear();
		m_arrAux.resize (Header.numAuxInts);
		m_arrVertices.clear();
		m_arrVertices.resize (Header.numVertices);
		m_numBones			= Header.numBones;
		m_numSkipBones	= Header.numSkipBones;
		m_numDests			= Header.numDests;
		memcpy (&m_arrAux[0], pAuxInts, sizeof(CrySkinAuxInt)*m_arrAux.size());
		memcpy (&m_arrVertices[0], pVertices, sizeof(Vertex)*m_arrVertices.size());
	}
	return nSizeRequired;
}



CrySkinBase::CStatistics::CStatistics (const CrySkinBase*pSkin)
{
	this->numBones     = pSkin->m_numBones;
	this->numSkipBones = pSkin->m_numSkipBones;
	this->numAuxInts   = (unsigned)pSkin->m_arrAux.size();
	this->numVertices  = (unsigned)pSkin->m_arrVertices.size();

	// construct the set of destination vertices
	for (unsigned nBone = numSkipBones; nBone < numBones; ++nBone)
	{
	}
}

void CrySkinBase::scaleVertices (float fScale)
{
	// all we have to do is to scale all the offsets
	for (VertexArray::iterator it = m_arrVertices.begin(), itEnd = m_arrVertices.end(); it!= itEnd; ++it)
		it->pt *= fScale;
}
