#include "stdafx.h"
//#include "CryAnimation.h"
#include "CVars.h"
#include "RenderUtils.h"


// copies the vertex info to the videomemory, given the source (vertices and UVs),
// target format and pointer to the videobuffer
void CopyPosUVsToVideomemory (
	const Vec3* pSrcPos,     // Source UVs, in its own indexation (needs to unreference with the pVertMap)
	const CryUV* pDstUV,      // UVs, in the destimation indexation (no need to unreference)
	unsigned numVerts,        // number of Target vertices
	const unsigned *pVertMap, // vertex map from Target to Source vertex indices
	Vec3* pDstPos,						// destination vertex buffer (always has vertices as the first element)
	int nDstVertexFormat      // the vertex format of the pDstPos vertex buffer
	)
{
	DEFINE_PROFILER_FUNCTION();
	if (nDstVertexFormat == VERTEX_FORMAT_P3F_TEX2F && pDstUV)
	{
		// should be the primary vertex format for characters
#if 0//DO_ASM
		// OPTIMIZATION NOTE: the assembly version runs ~7-10% faster in release mode
		_asm
		{
			mov ESI, pVertMap
			mov EDI, pDstPos
			mov ECX, numVerts
			mov EDX, pDstUV
			// start copying
			copy_loop:
			
			// pDst[0..2] = pSrc[pVertMap][0..2] 
			mov EBX, pSrcPos
			mov EAX, [ESI]  // EAX = pVertMap[sn]
			add EAX, EAX
			add EAX, EAX    // EAX = pVertMap[sn]*4
			add EBX, EAX
			add EAX, EAX    // EBX = EAX*3
			add EBX, EAX    // EBX = &pSrcPos[pVertMap[sn]] = pVertMap[sn]*12
			
			// [EDI][0..2] = [EBX][0..2]
			mov EAX, [EBX]
			mov [EDI], EAX
			mov EAX, [EBX+4]
			mov [EDI+4], EAX
			mov EAX, [EBX+8]
			mov [EDI+8], EAX

			mov EAX, [EDX]
			mov [EDI+12], EAX
			mov EAX, [EDX+4]
			mov [EDI+16], EAX

			// pDst[3..4] = pDstUV[0..2]

			add ESI, 4
			add EDI, 20
			add EDX, 8
			loop copy_loop
		}

#else
		struct_VERTEX_FORMAT_P3F_TEX2F *pDst = (struct_VERTEX_FORMAT_P3F_TEX2F*)pDstPos;
		for (unsigned sn = 0; sn < numVerts; ++sn, ++pDst)
		{
			unsigned i = pVertMap[sn];
      pDst->xyz = pSrcPos[i];
			*(CryUV*)(&pDst->st[0]) = *(pDstUV++);
		}
#endif
	}
	else
	{
		unsigned nStride = m_VertexSize[nDstVertexFormat];
		for(unsigned sn = 0;sn < numVerts; ++sn, ((byte*&)pDstPos) += nStride)
		{
			unsigned i = pVertMap[sn];
			*pDstPos = pSrcPos[i];
		}
	}
}

// copies the vertex info to the videomemory, given the source (vertices and UVs),
// target format and pointer to the videobuffer
void CopyPosUVsToVideomemory (
	const Vec3* pSrcPos,     // Source UVs, in its own indexation (needs to unreference with the pVertMap)
	const CryUV* pDstUV,      // UVs, in the destimation indexation (no need to unreference)
	unsigned numVerts,        // number of Target vertices
	Vec3* pDstPos,						// destination vertex buffer (always has vertices as the first element)
	int nDstVertexFormat      // the vertex format of the pDstPos vertex buffer
	)
{
	if (nDstVertexFormat == VERTEX_FORMAT_P3F_TEX2F && pDstUV)
	{
		// should be the primary vertex format for characters
		struct_VERTEX_FORMAT_P3F_TEX2F *pDst = (struct_VERTEX_FORMAT_P3F_TEX2F*)pDstPos;
		for (unsigned sn = 0; sn < numVerts; ++sn, ++pDst)
		{
      pDst->xyz = *(pSrcPos++);
			*(CryUV*)(&pDst->st[0]) = *(pDstUV++);
		}
	}
	else
	{
		unsigned nStride = m_VertexSize[nDstVertexFormat];
		for(unsigned sn = 0;sn < numVerts; ++sn, ((byte*&)pDstPos) += nStride)
		{
			*pDstPos = *(pSrcPos++);
		}
	}
}

IShader* GetShaderFrontCull()
{
	return g_GetIRenderer()->EF_LoadShader("FrontCull", eSH_World, EF_SYSTEM);
}
