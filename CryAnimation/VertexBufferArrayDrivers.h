/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	10/07/2002 - Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//  Contains:
//	Utility classes derived from sparse array driver that are used to index/access easily into
//     the leaf(vertex) buffer internal structure of vertex buffer.
//  Essentially, these are just sparse array drivers (each sparse array is just the array of positions,
//     normals, tangents, whatever in the video memory), and these classes are just used to initialize them.
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_ANIMATION_VERTEX_BUFFER_ARRAY_DRIVERS_HDR_
#define _CRY_ANIMATION_VERTEX_BUFFER_ARRAY_DRIVERS_HDR_

#include "SparseArrayDriver.h"

// position array in the video buffer.
// the positions in the vertex buffer are kept within some structure with some stride
// upon initialization, this array driver fetches those values from the vertex buffer structure
// and you can address the positions as a normal array then
class CVertexBufferPosArrayDriver: public TSparseArrayDriver<Vec3>
{
public:
	// this constructor mimics parameters of GetPosPtr
	CVertexBufferPosArrayDriver (CLeafBuffer* pVertexBuffer, int nId=0, bool bSystemMemory=true);
};

// Tangent array in the video buffer
class CVertexBufferTangentArrayDriver: public TSparseArrayDriver<Vec3>
{
public:
	// this constructor mimics parameters of GetTangentPtr
	CVertexBufferTangentArrayDriver (CLeafBuffer* pVertexBuffer, int nId=0, bool bSystemMemory=true);
};

// Tangent array in the video buffer
class CVertexBufferBinormalArrayDriver: public TSparseArrayDriver<Vec3>
{
public:
	// this constructor mimics parameters of GetTangentPtr
	CVertexBufferBinormalArrayDriver (CLeafBuffer* pVertexBuffer, int nId=0, bool bSystemMemory=true);
};


// TNormal array in the video buffer
class CVertexBufferTNormalArrayDriver: public TSparseArrayDriver<Vec3>
{
public:
	// this constructor mimisc parameters of GetTNormalPtr
	CVertexBufferTNormalArrayDriver (CLeafBuffer* pVertexBuffer, int nId=0, bool bSystemMemory=true);
};


// UV array in the video buffer
class CVertexBufferUVArrayDriver: public TSparseArrayDriver<CryUV>
{
public:
	// this constructor mimisc parameters of GetUVPtr
	CVertexBufferUVArrayDriver (CLeafBuffer* pVertexBuffer, int nId=0, bool bSystemMemory=true);
};

// Color array in the video buffer. Each DWORD is the color in the format AABBGGRR
class CVertexBufferColorArrayDriver: public TSparseArrayDriver<DWORD>
{
public:
	// this constructor mimisc parameters of GetUVPtr
	CVertexBufferColorArrayDriver (CLeafBuffer* pVertexBuffer, int nId=0, bool bSystemMemory=true);
};

#endif