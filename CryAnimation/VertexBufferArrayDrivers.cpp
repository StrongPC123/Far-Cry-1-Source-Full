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

#include "stdafx.h"
#include "VertexBufferArrayDrivers.h"

CVertexBufferPosArrayDriver::CVertexBufferPosArrayDriver (CLeafBuffer* pVertexBuffer, int nId, bool bSystemMemory)
{
	m_pData = pVertexBuffer->GetPosPtr (m_nStride, nId, bSystemMemory);
}
CVertexBufferTangentArrayDriver::CVertexBufferTangentArrayDriver (CLeafBuffer* pVertexBuffer, int nId, bool bSystemMemory)
{
	m_pData = pVertexBuffer->GetTangentPtr (m_nStride, nId, bSystemMemory);
}
CVertexBufferBinormalArrayDriver::CVertexBufferBinormalArrayDriver (CLeafBuffer* pVertexBuffer, int nId, bool bSystemMemory)
{
	m_pData = pVertexBuffer->GetBinormalPtr (m_nStride, nId, bSystemMemory);
}

CVertexBufferTNormalArrayDriver::CVertexBufferTNormalArrayDriver (CLeafBuffer* pVertexBuffer, int nId, bool bSystemMemory)
{
	m_pData = pVertexBuffer->GetTNormalPtr (m_nStride, nId, bSystemMemory);
}

CVertexBufferUVArrayDriver::CVertexBufferUVArrayDriver (CLeafBuffer* pVertexBuffer, int nId, bool bSystemMemory)
{
	m_pData = pVertexBuffer->GetUVPtr (m_nStride, nId, bSystemMemory);
}

CVertexBufferColorArrayDriver::CVertexBufferColorArrayDriver (CLeafBuffer* pVertexBuffer, int nId, bool bSystemMemory)
{
	m_pData = pVertexBuffer->GetColorPtr (m_nStride, nId, bSystemMemory);
}