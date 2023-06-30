/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	  Sep 25 2002 :- Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_ANIMATION_CRY_CHAR_DECAL_COMMON_HDR_
#define _CRY_ANIMATION_CRY_CHAR_DECAL_COMMON_HDR_

#include "TFace.h"
// common structures, funcitons and declarations used by different parts of the character decal system

// the simple decal face - each vertex index refers to the 0-based vertex index
// in the new vertex array
typedef TFace<unsigned short> CryCharDecalFace;
// this is the simple decal vertex - the index of the original vertex and UV coordinates
struct CryCharDecalVertex
{
	// the original vertex of the mesh
	//  (external vertex index in GeomInfo;
	//   explicit vertex index in the character
	//   vertex buffer sparse array)
	unsigned nVertex;
	// the new UV coordinates
	CryUV uvNew;

	CryCharDecalVertex () {}
	CryCharDecalVertex (unsigned nOrigVertex, float u, float v):
		nVertex (nOrigVertex)
	{
		uvNew.u = u;
		uvNew.v = v;
	}
};


#endif