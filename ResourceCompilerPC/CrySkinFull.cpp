#include "stdafx.h"
#include "MathUtils.h"
#include "CrySkinBuilderBase.h"
#include "CrySkinFull.h"
#include "platform.h"

#define FOR_TEST 0

// takes each offset and includes it into the bbox of corresponding bone
/*void CrySkinFull::computeBoneBBoxes(CryBBoxA16* pBBoxes)
{
	CrySkinAuxInt* pAux = &m_arrAux[0];
	Vertex* pVertex = &m_arrVertices[0];
	CryBBoxA16* pBBox = pBBoxes + m_numSkipBones, *pBBoxEnd = pBBoxes + m_numBones;

	for (; pBBox!= pBBoxEnd; ++pBBox)
	{
		// each bone has a group of vertices

		// first process the rigid vertices
		Vertex* pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex)
			pBBox->include(pVertex->pt);

		// process the smooth1 vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
			pBBox->include(pVertex->pt);

		// process the smooth vertices that were the second time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
			pBBox->include(pVertex->pt);
	}
}*/

//////////////////////////////////////////////////////////////////////////
// does the skinning out of the given array of global matrices
void CrySkinFull::skin (const Matrix44* pBones, Vec3d* pDest)
{
#ifdef DEFINE_PROFILER_FUNCTION
	DEFINE_PROFILER_FUNCTION();
#endif

	//PROFILE_FRAME_SELF(PureSkin);
#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif
	{
	
	const Matrix44* pBone			= pBones + m_numSkipBones;
	const Matrix44* pBonesEnd = pBones + m_numBones;

	u32 s = 0;
	u32 t = 0;

#ifdef _DEBUG
	TFixedArray<float> arrW;
	arrW.reinit(m_numDests, 0);
#endif

	for (; pBone!= pBonesEnd; ++pBone)
	{

		Matrix34 m34 = Matrix34( GetTransposed44(*pBone) );

		// first process the rigid vertices
		u32 a0=m_arrAux[t];
		for (u32 i=0; i<a0; i++ )
		{
			//_mm_prefetch( (char*)&m_arrVertices[s+20].pt, _MM_HINT_T0 );
			pDest[m_arrVertices[s].nDest] = m34 * m_arrVertices[s].pt;

			#ifdef _DEBUG
				assert (arrW[m_arrVertices[s].nDest] == 0);
				arrW[m_arrVertices[s].nDest] = 1;
			#endif
			s++;
		}
		t++;

		// process the smooth1 vertices that were the first time met
		u32 a1=m_arrAux[t]; t++;
		for (u32 i=0; i<a1; i++ )
		{
			//_mm_prefetch( (char*)&m_arrVertices[s+20].pt, _MM_HINT_T0 );
			pDest[m_arrAux[t]]= (m34*m_arrVertices[s].pt) * m_arrVertices[s].fWeight;
			
			#ifdef _DEBUG
				assert (arrW[m_arrAux[t]] == 0);
				arrW[m_arrAux[t]] = m_arrVertices[s].fWeight;
			#endif
			s++;
			t++;
		}

		// process the smooth vertices that were the first time met
		u32 a2=m_arrAux[t]; t++;
		for (u32 i=0; i<a2; i++)
		{
			//_mm_prefetch( (char*)&m_arrVertices[s+20].pt, _MM_HINT_T0 );
			pDest[m_arrAux[t]] += (m34*m_arrVertices[s].pt) * m_arrVertices[s].fWeight;
			
			#ifdef _DEBUG
				assert (arrW[m_arrAux[t]] > 0 && arrW[m_arrAux[t]] < 1.005f);
				arrW[m_arrAux[t]] += m_arrVertices[s].fWeight;
				assert (arrW[m_arrAux[t]] > 0 && arrW[m_arrAux[t]] < 1.005f);
			#endif
		 s++;
		 t++;
		}
	}
	
	/*#ifdef _DEBUG
	for (unsigned i = 0; i < m_numDests; ++i)
		assert (arrW[i] > 0.995f && arrW[i] < 1.005f);
	#endif
	*/

	}
}

//////////////////////////////////////////////////////////////////////////
// does the skinning out of the given array of global matrices
void CrySkinFull::skinAsVec3d16 (const Matrix44* pBones, Vec3dA16* pDest)
{
	//PROFILE_FRAME_SELF(PureSkin);
#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif
	{
	const Matrix44* pBone = pBones + m_numSkipBones, *pBonesEnd = pBones + m_numBones;
	CrySkinAuxInt* pAux = &m_arrAux[0];
	Vertex* pVertex = &m_arrVertices[0];


#ifdef _DEBUG
	TFixedArray<float> arrW;
	arrW.reinit(m_numDests, 0);
#endif

	for (; pBone!= pBonesEnd; ++pBone)
	{
		// each bone has a group of vertices

		// first process the rigid vertices
		Vertex* pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex)
		{
			//CHANGED_BY_IVO  - INVALID CHANGE, PLEASE REVISE
			pDest[pVertex->nDest].v = pBone->TransformVectorOLD(pVertex->pt);
			// Temporary fixed by Sergiy. A new operation in the Matrix must be made
			//pDest[pVertex->nDest].v = GetTransposed44(*pBone) * (pVertex->pt);
			//transformVectorNoTrans (pDest[pVertex->nDest].v, pVertex->pt, *pBone);
#ifdef _DEBUG
			assert (arrW[pVertex->nDest] == 0);
			arrW[pVertex->nDest] = 1;
#endif
		}

		// process the smooth1 vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			transformWVector (pDest[*pAux].v, *pBone, *pVertex);
#ifdef _DEBUG
			assert (arrW[*pAux] == 0);
			arrW[*pAux] = pVertex->fWeight;
#endif
		}

		// process the smooth vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			addWVector (pDest[*pAux].v, *pBone, *pVertex);
#ifdef _DEBUG
			assert (arrW[*pAux] > 0 && arrW[*pAux] < 1.005f);
			arrW[*pAux] += pVertex->fWeight;
			assert (arrW[*pAux] > 0 && arrW[*pAux] < 1.005f);
#endif
		}
	}
#ifdef _DEBUG
	for (unsigned i = 0; i < m_numDests; ++i)
		assert (arrW[i] > 0.995f && arrW[i] < 1.005f);
#endif
	}
}

void CrySkinFull::CStatistics::addDest(unsigned nDest)
{
	if (arrNumLinks.size() < nDest+1)
		arrNumLinks.resize (nDest+1,0);
	++arrNumLinks[nDest];
	setDests.insert (nDest);
}

void CrySkinFull::CStatistics::initSetDests (const CrySkinFull* pSkin)
{
	const CrySkinAuxInt* pAux = &pSkin->m_arrAux[0];
	const Vertex* pVertex = &pSkin->m_arrVertices[0];

	arrNumLinks.clear();

	for (unsigned nBone = pSkin->m_numSkipBones; nBone < pSkin->m_numBones; ++nBone)
	{
		// each bone has a group of vertices

		// first process the rigid vertices
		const Vertex* pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex)
		{
			unsigned nDest = pVertex->nDest;
			addDest (nDest);
			assert (arrNumLinks[nDest] == 1);
		}

		// process the smooth1 vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			addDest (nDest);
			assert (arrNumLinks[nDest] == 1);
			//pVertex->fWeight is the weight of the vertex
		}

		// process the smooth vertices that were the second/etc time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			addDest (nDest);
			assert (arrNumLinks[nDest] > 1);
			// pVertex->fWeight contains the weight of the vertex
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
// validates the skin against the given geom info
#if defined (_DEBUG)
void CrySkinFull::validate (const ICrySkinSource* pGeometry)
{
	TElementaryArray<unsigned> arrNumLinks ("CrySkinFull::validate.arrNumLinks");
	arrNumLinks.reinit (pGeometry->numVertices(), 0);

	CrySkinAuxInt* pAux = &m_arrAux[0];
	Vertex* pVertex = &m_arrVertices[0];

	for (unsigned nBone = m_numSkipBones; nBone < m_numBones; ++nBone)
	{
		// each bone has a group of vertices

		// first process the rigid vertices
		Vertex* pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex)
		{
			unsigned nDest = pVertex->nDest;
			const CryVertexBinding& rLink = pGeometry->getLink(nDest);
			assert (arrNumLinks[nDest] == 0);
			arrNumLinks[nDest] = 1;
			assert (rLink.size()==1);
			assert (rLink[0].Blending == 1);
			assert (rLink[0].BoneID == nBone);
		}

		// process the smooth1 vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			const CryVertexBinding& rLink = pGeometry->getLink(nDest);
			assert (arrNumLinks[nDest]++ == 0);
			assert (rLink.size()>1);
			float fLegacyWeight = rLink.getBoneWeight(nBone);
			assert (pVertex->fWeight == fLegacyWeight);
		}

		// process the smooth vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (;pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			const CryVertexBinding& rLink = pGeometry->getLink(nDest);
			assert (arrNumLinks[nDest]++ > 0);
			assert (arrNumLinks[nDest] <= rLink.size());
			assert (rLink.size()>1);
			float fLegacyWeight = rLink.getBoneWeight(nBone);
			assert (rLink.hasBoneWeight(nBone,pVertex->fWeight));
		}
	}

	for (unsigned nVert = 0; nVert < pGeometry->numVertices(); ++nVert)
		assert (arrNumLinks[nVert] == pGeometry->getLink(nVert).size());
}
#endif

#if ( defined (_CPU_X86) || defined (_CPU_AMD64) ) & !defined(LINUX)

DEFINE_ALIGNED_DATA( CryBBoxA16, CrySkinFull::g_BBox, 32 ); // align by cache line boundaries

#if defined (_CPU_AMD64)
extern "C" void Amd64Skinner(CrySkinAuxInt* pAux, CrySkinVertexAligned* pVertex, Vec3dA16* pDest, const Matrix44* pBone, Vec3dA16* pvMin,const Matrix44* pBoneEnd);
#endif


void CrySkinFull::skinSSE (const Matrix44* pBones, Vec3dA16* pDest)
{	

#ifdef DEFINE_PROFILER_FUNCTION
	DEFINE_PROFILER_FUNCTION();
#endif

	//PROFILE_FRAME_SELF(PureSkin);
	const Matrix44* pBone = pBones + m_numSkipBones, *pBoneEnd = pBones + m_numBones;
	CrySkinAuxInt* pAux = &m_arrAux[0];
	Vertex* pVertex = &m_arrVertices[0];

	// set the bbox to the negative volume to make sure the bbox will calculate starting from the first vertex
	g_BBox.vMin.v = Vec3d(1e6,1e6,1e6);// = pBone->GetTranslation();
	g_BBox.vMax.v = Vec3d(-1e6,-1e6,-1e6);// = pBone->GetTranslation();

#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif

#if defined(_CPU_AMD64)
	Amd64Skinner(pAux, pVertex, pDest, pBone, &g_BBox.vMin, pBoneEnd);
#else
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
			movaps xmm3, [ESI+0x30]
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
			jz endLoopRigid

	startLoopRigid:
			// load the offset
			movaps xmm7, [EBX]
			// calculate the destination pointer
			mov EAX, [EBX+0xC]
			and EAX, 0xFFFFFF
			add EAX, EAX
			// EDI+EAX*8 points to the destination vector now
			add EBX, 0x10

			// transform the vertex
			movss xmm6, xmm7
			shufps xmm6, xmm6, 0 // xmm6 = 4 copies of offset.x
			mulps xmm6, xmm0
			movaps xmm5, xmm7
			shufps xmm5, xmm5, 0x55 // xmm5 = 4 copies of offset.y
			mulps xmm5, xmm1
			shufps xmm7, xmm7, 0xAA // xmm7 = 4 copies of offset.z
			mulps xmm7, xmm2
			addps xmm7, xmm5
			addps xmm7, xmm6
			addps xmm7, xmm3        // xmm7 = fully transformed vertex, store it
			// xmm7 = transformed vertex
			movaps [EDI+EAX*8], xmm7

			//----------------------
			// Calculation of BBox
			// xmm5 will be the min, xmm6 will be the max of bbox
			movaps xmm5, xmm7
			movaps xmm6, xmm7
			minps xmm5, g_BBox.vMin
			maxps xmm6, g_BBox.vMax
			movaps g_BBox.vMin, xmm5
			movaps g_BBox.vMax, xmm6

			loop startLoopRigid
	endLoopRigid:


//////////////////////////////////////////////////////////
// Smooth-1 loop
// load the counter for the number of smooth vertices met for the first time
//////////////////////////////////////////////////////////
	#if CRY_SKIN_AUX_INT_SIZE==2
			xor ECX,ECX
			mov CX, word ptr [EDX]
			add EDX, 2
	#else
			mov ECX, dword ptr [EDX]
			add EDX, 4
	#endif
			test ECX, ECX
			jz endLoopSmooth1

	startLoopSmooth1:
			// load the offset & blending
			movaps xmm7, [EBX]
			// calculate the destination pointer
	#if CRY_SKIN_AUX_INT_SIZE==2
			xor EAX,EAX
			mov AX, word ptr [EDX]
			add EDX, 2
	#else
			mov EAX, dword ptr [EDX]
			add EDX, 4
	#endif
			add EAX, EAX
			// EDI+EAX*8 points to the destination vector now
			add EBX, 0x10

			// transform the vertex
			movss xmm6, xmm7
			shufps xmm6, xmm6, 0 // xmm6 = 4 copies of offset.x
			mulps xmm6, xmm0
			movaps xmm5, xmm7
			shufps xmm5, xmm5, 0x55 // xmm5 = 4 copies of offset.y
			mulps xmm5, xmm1
			movaps xmm4, xmm7
			shufps xmm4, xmm4, 0xAA // xmm4 = 4 copies of offset.z
			mulps xmm4, xmm2
			addps xmm4, xmm5
			addps xmm4, xmm6
			addps xmm4, xmm3        // xmm4 = fully transformed vertex, blend it
			shufps xmm7, xmm7, 0xFF // xmm7 = 4 copies of blending
			mulps xmm7, xmm4
			// xmm7 = transformed and blended vertex
			movaps [EDI+EAX*8], xmm7

			loop startLoopSmooth1
			//loop startLoopNonflipped
	endLoopSmooth1:



//////////////////////////////////////////////////////////
// Smooth-2 loop
// load the counter for the number of smooth vertices met for the second time
//////////////////////////////////////////////////////////
	#if CRY_SKIN_AUX_INT_SIZE==2
			xor ECX,ECX
			mov CX, word ptr [EDX]
			add EDX, 2
	#else
			mov ECX, dword ptr [EDX]
			add EDX, 4
	#endif
			test ECX, ECX
			jz endLoopSmooth2

	startLoopSmooth2:
			// load the offset & blending
			movaps xmm7, [EBX]
			// calculate the destination pointer
	#if CRY_SKIN_AUX_INT_SIZE==2
			xor EAX,EAX
			mov AX, word ptr [EDX]
			add EDX, 2
	#else
			mov EAX, dword ptr [EDX]
			add EDX, 4
	#endif
			shl EAX, 4
			add EAX, EDI
			// EAX points to the destination vector now
			add EBX, 0x10

			// transform the vertex
			movss xmm6, xmm7
			shufps xmm6, xmm6, 0 // xmm6 = 4 copies of offset.x
			mulps xmm6, xmm0
			movaps xmm5, xmm7
			shufps xmm5, xmm5, 0x55 // xmm5 = 4 copies of offset.y
			mulps xmm5, xmm1
			movaps xmm4, xmm7
			shufps xmm4, xmm4, 0xAA // xmm4 = 4 copies of offset.z
			mulps xmm4, xmm2
			addps xmm4, xmm5
			addps xmm4, xmm6
			addps xmm4, xmm3        // xmm4 = fully transformed vertex, blend it
			shufps xmm7, xmm7, 0xFF // xmm7 = 4 copies of blending
			mulps xmm7, xmm4
			// xmm7 = transformed and blended vertex
			addps xmm7, [EAX]
			movaps [EAX], xmm7

			loop startLoopSmooth2
			//loop startLoopNonflipped
	endLoopSmooth2:

			jmp startLoop
	endLoop:
		}
#endif		// _CPU_AMD64
}
#endif
