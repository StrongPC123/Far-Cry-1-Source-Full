#ifndef _CRY_ANIMATION_CRY_SKIN_TYPES_HDR_
#define _CRY_ANIMATION_CRY_SKIN_TYPES_HDR_

#include "MathUtils.h"
//////////////////////////////////////////////////////////////////////////
// the packed skin vertex structure: rigid or smooth, 16 byte
// the point is the original point in the bone's CS
struct CrySkinVertexAligned
{
	Vec3 pt;

	union
	{
		unsigned nDest; // the destination vertex index to store the transformed point
		float fWeight; // the weight of the point
	};
};

// the way the packed indices/group headers are kept: short or int
#define CRY_SKIN_AUX_INT_SIZE 2

#if CRY_SKIN_AUX_INT_SIZE==2
typedef unsigned short CrySkinAuxInt;
#elif CRY_SKIN_AUX_INT_SIZE==4
typedef unsigned CrySkinAuxInt;
#if defined _CPU_AMD64
#error You will need to modify the file CrySkinAMD64.ASM to make this work!
#endif	 // _CPU_AMD64
#else
#error CRY_SKIN_AUX_INT_SIZE must be defined and be 2 or 4
#endif

//////////////////////////////////////////////////////////////////////////
// Unpacked rigid vertex structure
struct CrySkinRigidVertex
{
	Vec3 pt;
	unsigned nDest; // the destination vertex index

	CrySkinRigidVertex(){}

	// initializes the structure given the offset from the bone and the destination vertex index
	CrySkinRigidVertex (const Vec3& _ptOffset, unsigned _nDest):
		pt (_ptOffset),
		nDest (_nDest)
		{
		}

	// constructs the packed rigid vertex data from this structure
	void build (CrySkinVertexAligned& vDst)const
	{
		vDst.pt = pt;
		vDst.nDest = nDest;
	}
};

// aligned by 16-byte boundary tangent vectors
// if you don't want the padding to be used, just define it as
// typedef SPipTangents SPipTangentsA16;
struct SPipTangentsA16
{
  Vec3 m_Tangent;
	unsigned int m_Pad0;
  Vec3 m_Binormal;
	unsigned int m_Pad1;
  Vec3 m_TNormal;
	unsigned int m_Pad2;

	inline SPipTangentsA16& operator = (const SPipTangents& right)
	{
		m_Tangent  = right.m_Tangent;
		m_Binormal = right.m_Binormal;
		m_TNormal  = right.m_TNormal;
		return *this;
	}

	inline void copyTo (SPipTangents* right)
	{/*
#ifdef DO_ASM
    _asm
		{
			mov EBX, this
			mov EDX, right
			
			mov EAX, [EBX]
			mov [EDX], EAX
			mov EAX, [EBX+4]
			mov [EDX+4], EAX
			mov EAX, [EBX+8]
			mov [EDX+8], EAX

			mov EAX, [EBX+0x10]
			mov [EDX+0xC], EAX
			mov EAX, [EBX+0x14]
			mov [EDX+0x10], EAX
			mov EAX, [EBX+0x18]
			mov [EDX+0x14], EAX

			mov EAX, [EBX+0x20]
			mov [EDX+0x18], EAX
			mov EAX, [EBX+0x24]
			mov [EDX+0x1C], EAX
			mov EAX, [EBX+0x28]
			mov [EBX+0x20], EAX
		}
#else
		*/
		right->m_Tangent  = m_Tangent;
		right->m_Binormal = m_Binormal;
		right->m_TNormal  = m_TNormal;
//#endif
	}
};


// the size of the SPipTangentsA: can be either 0x30 for aligned structure or
// 0x24 for non-aligned
#define sizeofSPipTangentsA 0x24

#if sizeofSPipTangentsA == 0x24
// this is the actual structure that's to be used to store tangents
typedef SPipTangents SPipTangentsA;
#elif sizeofSPipTangentsA == 0x30
typedef SPipTangentsA16 SPipTangentsA;
#else
#error 
#endif
//////////////////////////////////////////////////////////////////////////
// Unpacked rigid basis structure
struct CrySkinRigidBaseInfo
{
	Vec3 ptTangent;
	Vec3 ptBinormal;
	unsigned nDest;

	CrySkinRigidBaseInfo (){}
	CrySkinRigidBaseInfo (const Matrix44& matInvDef, const TangData& rBasis, unsigned _nDest):
	ptTangent (matInvDef.TransformVectorOLD(rBasis.tangent)),
	ptBinormal(matInvDef.TransformVectorOLD(rBasis.binormal)),
	nDest (_nDest)
	{

	}

	// constructs the packed representation of the base: uses pDst[0] for tangent and destination
	// vertex, and pDst[1] for binormal. Does not set the additional field of the pDst[1]
	void build (CrySkinVertexAligned* pDst)const
	{
		pDst[0].pt = ptTangent;
		// we multiply to get the actual offset (to avoid multiplication in assembly)
		// and add the dummy bits that avoid denormalization assists in SSE
		pDst[0].nDest = nDest * sizeof(SPipTangentsA) + 0x40000000;
		pDst[1].pt = ptBinormal;
		pDst[1].fWeight = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
// Unpacked smooth vertex structure
struct CrySkinSmoothVertex: public CrySkinRigidVertex
{
	float fWeight;// the weight of the point

	CrySkinSmoothVertex(){}

	// initializes this smooth vertex structure
	CrySkinSmoothVertex (const CryLink& rLink, unsigned _nDest):
		CrySkinRigidVertex(rLink.offset, _nDest),
		fWeight(rLink.Blending)
	{
	}

	// constructs the packed rigid vertex data from this structure
	void build (CrySkinVertexAligned& vDst)const
	{
		vDst.pt = pt;
		vDst.fWeight = fWeight;
	}
};

#endif