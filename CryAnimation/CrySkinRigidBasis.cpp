#include "stdafx.h"
//#include "CryAnimationBase.h"
#include "CrySkinRigidBasis.h"

#define FOR_TEST 0

#if FOR_TEST
#include "CryAnimation.h"
#include "CVars.h"
#endif

// returns the size of the skin, the number of bases being calculated
// by this skin. The bases are calculated into a 0-base continuous array
// tangents may be divided into subskins, each having different number of bases
// to skin, based on the performance consideration (strip mining)
unsigned CrySkinRigidBasis::size()const
{
	return m_numDestBases;
}

// does the same as the base class init() but also remembers the number of bases (numVerts/2)
// for future reference
void CrySkinRigidBasis::init (unsigned numVerts, unsigned numAux, unsigned numSkipBones, unsigned numBones)
{
	m_numDestBases = numVerts >> 1;
	CrySkinBase::init (numVerts, numAux, numSkipBones, numBones);
}


void CrySkinRigidBasis::CStatistics::initSetDests (const CrySkinRigidBasis* pSkin)
{
	const CrySkinAuxInt* pAux = &pSkin->m_arrAux[0];
	const Vertex* pVertex = &pSkin->m_arrVertices[0];
	setDests.clear();
	arrNumLinks.clear();

	for (unsigned nBone = pSkin->m_numSkipBones; nBone < pSkin->m_numBones; ++nBone)
	{
		// each bone has a group of (always rigid) vertices

		// this is to take into account two groups: non-flipped and flipped tangents
		for (int t = 0; t < 2; ++t)
		{
			// for each actual basis, we have two vertices
			const Vertex* pGroupEnd = pVertex + (*pAux++<<1);
			for (;pVertex < pGroupEnd; pVertex+=2)
			{
				unsigned nDestOffset = pVertex[0].nDest & 0xFFFFFF;
				assert (nDestOffset < pSkin->m_numDestBases*sizeof(SPipTangentsA) && nDestOffset % sizeof(SPipTangentsA) == 0);
				unsigned nDest = nDestOffset / sizeof(SPipTangentsA);
				addDest(nDest);
			}
		}
	}
}

void CrySkinRigidBasis::CStatistics::addDest(unsigned nDest)
{
	if (arrNumLinks.size() < nDest+1)
		arrNumLinks.resize (nDest+1,0);
	++arrNumLinks[nDest];
	setDests.insert (nDest);
}


// does the skinning out of the given array of global matrices:
// calculates the bases and fills the PipVertices in
void CrySkinRigidBasis::skin (const Matrix44* pBones, SPipTangentsA* pDest)const
{
#ifdef DEFINE_PROFILER_FUNCTION
	DEFINE_PROFILER_FUNCTION();
#endif

#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif
	{
		const Matrix44* pBone = pBones + m_numSkipBones, *pBonesEnd = pBones + m_numBones;
		const CrySkinAuxInt* pAux = &m_arrAux[0];
		const Vertex* pVertex = &m_arrVertices[0];

		for (; pBone!= pBonesEnd; ++pBone)
		{
			// each bone has a group of (always rigid) vertices

			// for each actual basis, we have two vertices
			const Vertex* pGroupEnd = pVertex + (*pAux++<<1);
			for (;pVertex < pGroupEnd; pVertex+=2)
			{
				unsigned nDestOffset = pVertex[0].nDest & 0xFFFFFF;
				assert (nDestOffset < m_numDestBases*sizeof(SPipTangentsA));
				SPipTangentsA& rDest = *(SPipTangentsA*)(UINT_PTR(pDest) + nDestOffset);
				Vec3d vTang = pBone->TransformVectorOLD(pVertex[0].pt);
				Vec3d vBinorm = pBone->TransformVectorOLD(pVertex[1].pt);
				rDest.m_Tangent	 = vTang;
				rDest.m_Binormal = vBinorm;
				rDest.m_TNormal  = vTang^vBinorm;
			}

			// the flipped version
			pGroupEnd = pVertex + (*pAux++<<1);
			for (;pVertex < pGroupEnd; pVertex+=2)
			{
				unsigned nDestOffset = pVertex[0].nDest & 0xFFFFFF;
				assert (nDestOffset < m_numDestBases*sizeof(SPipTangentsA));
				SPipTangentsA& rDest = *(SPipTangentsA*)(UINT_PTR(pDest) + nDestOffset);
				Vec3d vTang = pBone->TransformVectorOLD(pVertex[0].pt);
				Vec3d vBinorm = pBone->TransformVectorOLD(pVertex[1].pt);

				rDest.m_Tangent	 = vTang;
				rDest.m_Binormal = vBinorm;
				rDest.m_TNormal  = vBinorm^vTang;
			}
		}
	}
}




#if defined(_CPU_X86) && !defined(LINUX)
// uses SSE for skinning; NOTE: EVERYTHING must be 16-aligned:
// destination, bones, and the data in this object
void CrySkinRigidBasis::skinSSE (const Matrix44* pBones, SPipTangentsA* pDest)const
{
#ifdef DEFINE_PROFILER_FUNCTION
	DEFINE_PROFILER_FUNCTION();
#endif

#if defined(_DEBUG) && FOR_TEST
	TElementaryArray<SPipTangentsA> arrTest ("CrySkinRigidBasis::skinSSE");
	arrTest.reinit(size());
  skin (pBones, &arrTest[0]);
#endif

#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif
	{
		const Matrix44* pBone = pBones + m_numSkipBones, *pBoneEnd = pBones + m_numBones;
		const CrySkinAuxInt* pAux = &m_arrAux[0];
		const Vertex* pVertex = &m_arrVertices[0];

		_asm
		{	
			mov EDX, pAux
			mov EBX, pVertex
			mov EDI, pDest
			mov ESI, pBone

			// load the current matrix; we don't need the move component
	startLoop:
			cmp ESI, pBoneEnd
			jz endLoop
			movaps xmm0, [ESI]
			movaps xmm1, [ESI+0x10]
			movaps xmm2, [ESI+0x20]
			add ESI, 0x40

			// load the counter for the number of non-flipped tangets for this bone
	#if CRY_SKIN_AUX_INT_SIZE==2
			xor ECX,ECX
			mov CX, word ptr [EDX]
			add EDX, 2
	#else
			mov ECX, dword ptr [EDX]
			add EDX, 4
	#endif
			test ECX, ECX
			jz endLoopNonflipped

	startLoopNonflipped:
			// load the tangent vector
			movaps xmm7, [EBX]
			// load the binormal
			// calculate the destination pointer
			mov EAX, [EBX+0xC]
			and EAX, 0xFFFFFF
			add EAX, EDI
			// EAX points to the destination triplet of vectors now
			movaps xmm6, [EBX+0x10]
			add EBX, 0x20
			//prefetchnta [EBX]

			// calculate the transformed tangent and binormal
			movss xmm5, xmm7
			shufps xmm5, xmm5, 0 // xmm5 = 4 copies of tangent.x
			mulps xmm5, xmm0
			movaps xmm4, xmm7
			shufps xmm4, xmm4, 0x55 // xmm4 = 4 copies of tangent.y
			mulps xmm4, xmm1
			shufps xmm7, xmm7, 0xAA // xmm7 = 4 copies of tangent.z
			mulps xmm7, xmm2
			addps xmm7, xmm4
			addps xmm7, xmm5
			// xmm7 = transformed tangent
#if sizeofSPipTangentsA == 0x30
			movaps [EAX], xmm7
#else
			//SSE_MOVSS(EAX,xmm7)
			movss [EAX], xmm7       
			shufps xmm7,xmm7, 0x39  // roll right
			movss [EAX+4], xmm7		 
			shufps xmm7,xmm7, 0x39  // roll right
			movss [EAX+8], xmm7
			shufps xmm7, xmm7, 0x4E  // roll left twice
#endif

			// transform the binormal
			movss xmm5, xmm6
			shufps xmm5, xmm5, 0
			mulps xmm5, xmm0
			movaps xmm4, xmm6
			shufps xmm4, xmm4, 0x55
			mulps xmm4, xmm1
			shufps xmm6, xmm6, 0xAA
			mulps xmm6, xmm2
			addps xmm6, xmm4
			addps xmm6, xmm5
			// xmm6 = transformed binormal
#if sizeofSPipTangentsA == 0x30
			movaps [EAX+0x10], xmm6
#else
			//SSE_MOVSS(EAX+0xC,xmm7)
			movss [EAX+0xC], xmm6       
			shufps xmm6,xmm6, 0x39	 
			movss [EAX+0xC+4], xmm6		 
			shufps xmm6,xmm6, 0x39
			movss [EAX+0xC+8], xmm6		 
			shufps xmm6, xmm6, 0x4E  // roll left twice
#endif

			// calculate the cross product tangent (xmm7)^binormal (xmm6)
			movaps xmm5, xmm7
			shufps xmm5, xmm5, 0x09 // roll right 3-base
			movaps xmm4, xmm6
			shufps xmm4, xmm4, 0x12 // roll left 3-base
			//shufps xmm4, xmm4, 0x09 // roll right 3-base
			mulps xmm5, xmm4

			shufps xmm7, xmm7, 0x12 // roll left 3-base
			shufps xmm6, xmm6, 0x09 // roll right 3-base
			//shufps xmm7, xmm7, 0x09 // roll right 3-base
			mulps xmm7, xmm6
			subps xmm5, xmm7
			//shufps xmm5,xmm5, 0x09
#if sizeofSPipTangentsA == 0x30
			movaps [EAX+0x20], xmm5
#else
			//SSE_MOVSS(EAX+0x18,xmm5)
			movss [EAX+0x18], xmm5       
			shufps xmm5,xmm5, 0x39	 
			movss [EAX+0x18+4], xmm5		 
			shufps xmm5,xmm5, 0x39
			movss [EAX+0x18+8], xmm5		 
			shufps xmm5, xmm5, 0x4E  // roll left twice
#endif

			dec ECX
			jnz startLoopNonflipped
			//loop startLoopNonflipped
	endLoopNonflipped:

			//////////////////////////////////////////////////////////
			// Flipped loop

			// load the counter for the number of flipped tangets for this bone
	#if CRY_SKIN_AUX_INT_SIZE==2
			xor ECX,ECX
			mov CX, word ptr [EDX]
			add EDX, 2
	#else
			mov ECX, dword ptr [EDX]
			add EDX, 4
	#endif
			test ECX, ECX
			jz endLoopFlipped

	startLoopFlipped:
			// load the tangent vector
			movaps xmm7, [EBX]
			// load the binormal
			movaps xmm6, [EBX+0x10]
			// calculate the destination pointer
			mov EAX, [EBX+0xC]
			and EAX, 0xFFFFFF
			add EAX, EDI
			// EAX points to the destination triplet of vectors now
			add EBX, 0x20
			//prefetchnta [EBX]

			// calculate the transformed tangent and binormal
			movss xmm5, xmm7
			shufps xmm5, xmm5, 0 // xmm5 = 4 copies of tangent.x
			mulps xmm5, xmm0
			movaps xmm4, xmm7
			shufps xmm4, xmm4, 0x55 // xmm4 = 4 copies of tangent.y
			mulps xmm4, xmm1
			shufps xmm7, xmm7, 0xAA // xmm7 = 4 copies of tangent.z
			mulps xmm7, xmm2
			addps xmm7, xmm4
			addps xmm7, xmm5
			// xmm7 = transformed tangent
#if sizeofSPipTangentsA == 0x30
			movaps [EAX], xmm7
#else
			//SSE_MOVSS(EAX,xmm7)
			movss [EAX], xmm7       
			shufps xmm7,xmm7, 0x39	 
			movss [EAX+4], xmm7		 
			shufps xmm7,xmm7, 0x39
			movss [EAX+8], xmm7		 
			shufps xmm7, xmm7, 0x4E  // roll left twice
#endif

			// transform the binormal
			movss xmm5, xmm6
			shufps xmm5, xmm5, 0
			mulps xmm5, xmm0
			movaps xmm4, xmm6
			shufps xmm4, xmm4, 0x55
			mulps xmm4, xmm1
			shufps xmm6, xmm6, 0xAA
			mulps xmm6, xmm2
			addps xmm6, xmm4
			addps xmm6, xmm5
			// xmm6 = transformed binormal
#if sizeofSPipTangentsA == 0x30
			movaps [EAX+0x10], xmm6
#else
			//SSE_MOVSS(EAX+0xC,xmm6)
			movss [EAX+0xC], xmm6       
			shufps xmm6,xmm6, 0x39
			movss [EAX+0xC+4], xmm6		 
			shufps xmm6,xmm6, 0x39   
			movss [EAX+0xC+8], xmm6
			shufps xmm6, xmm6, 0x4E  // roll left twice
#endif

			// calculate the cross product binormal (xmm6)^tangent (xmm7)
			movaps xmm5, xmm7
			shufps xmm5, xmm5, 0x09 // roll right 3-base
			movaps xmm4, xmm6
			shufps xmm4, xmm4, 0x12 // roll left 3-base
			//shufps xmm4, xmm4, 0x09 // roll right 3-base
			mulps xmm5, xmm4

			shufps xmm7, xmm7, 0x12 // roll left 3-base
			shufps xmm6, xmm6, 0x09 // roll right 3-base
			//shufps xmm7, xmm7, 0x09 // roll right 3-base
			mulps xmm7, xmm6
			subps xmm7, xmm5
			//shufps xmm7,xmm7, 9
#if sizeofSPipTangentsA == 0x30
			movaps [EAX+0x20], xmm7
#else
			//SSE_MOVSS(EAX+0x18,xmm7)
			movss [EAX+0x18], xmm7       
			shufps xmm7,xmm7, 0x39
			movss [EAX+0x18+4], xmm7		 
			shufps xmm7,xmm7, 0x39   
			movss [EAX+0x18+8], xmm7		 
			shufps xmm7, xmm7, 0x4E  // roll left twice
#endif

			dec ECX
			jnz startLoopFlipped
			//loop startLoopFlipped
	endLoopFlipped:
			jmp startLoop
	endLoop:
		}
	}
#if defined(_DEBUG) && FOR_TEST
	unsigned numBases = 0;

	for (unsigned nBone = m_numSkipBones; nBone < m_numBones; ++nBone)
	{
		assert (numBases < size());
		const CrySkinAuxInt* pAux = &m_arrAux[(nBone-m_numSkipBones)*2];
		SPipTangentsA* pBTest;
		SPipTangentsA* pBDest;
		const Vertex*pVertex = &m_arrVertices[numBases*2];

		// check the non-flipped bases
		unsigned i, j;
		float dT, dB, dN;
		for (i = 0; i < pAux[0]; ++i, ++numBases, pVertex+=2)
		{
			pBTest = (SPipTangentsA*)(((unsigned)&arrTest[0])+(pVertex->nDest&0xFFFFFF));
			pBDest = (SPipTangentsA*)(((unsigned)pDest)+(pVertex->nDest&0xFFFFFF));
			dT = Distance2(pBTest->m_Tangent,pBDest->m_Tangent);
			dB = Distance2(pBTest->m_Binormal,pBDest->m_Binormal);
			dN = Distance2(pBTest->m_TNormal,pBDest->m_TNormal);
			assert (dT < 1e-6 && dB < 1e-6 && dN < 1e-6);
		}
		for (j = 0; j < pAux[1]; ++j, ++numBases, pVertex+=2)
		{
			pBTest = (SPipTangentsA*)(((unsigned)&arrTest[0])+(pVertex->nDest&0xFFFFFF));
			pBDest = (SPipTangentsA*)(((unsigned)pDest)+(pVertex->nDest&0xFFFFFF));
			dT = Distance2(pBTest->m_Tangent,pBDest->m_Tangent);
			dB = Distance2(pBTest->m_Binormal,pBDest->m_Binormal);
			dN = Distance2(pBTest->m_TNormal,pBDest->m_TNormal);
			assert (dT < 1e-6 && dB < 1e-6 && dN < 1e-6);
		}
	}
	assert (numBases == size());
#endif
}
#endif

// returns the number of bytes occupied by this structure and all its contained objects
unsigned CrySkinRigidBasis::sizeofThis()const
{
	return CrySkinBase::sizeofThis() + sizeof(CrySkinRigidBasis) - sizeof(CrySkinBase);
}

unsigned CrySkinRigidBasis::Serialize (bool bSave, void* pBuffer, unsigned nBufSize)
{
	if (bSave)
	{
		unsigned nWrittenBytes = CrySkinBase::Serialize_PC(true, pBuffer, nBufSize);
		if (nWrittenBytes)
		{
			if (pBuffer)
        *(unsigned*)(((char*)pBuffer)+nWrittenBytes) = m_numDestBases;

			return sizeof(unsigned) + nWrittenBytes;
		}
		else
		{
			// error
			return 0;
		}
	}
	else
	{
		unsigned nReadBytes = CrySkinBase::Serialize_PC(false, pBuffer, nBufSize);
		if (nReadBytes)
		{
			if (nBufSize - nReadBytes >= sizeof(unsigned))
			{
				m_numDestBases = *(unsigned*)(((char*)pBuffer)+nReadBytes);
				return nReadBytes + sizeof(unsigned);
			}
			else
			{
				//error - perhaps not the tang stream
				m_numDestBases = 0;
				clear();
				return 0;
			}
		}
		else
		{
			//error
			return 0;
		}
	}
}
