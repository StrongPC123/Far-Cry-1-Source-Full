/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CryAnimationBase.h"
#include "CryCharRenderElement.h"
#include "CVars.h"

CryCharRenderElement::CryCharRenderElement ():
	m_pLeafBuffer (NULL),
	m_nLeafBufferLastRenderFrame (0),
	m_numVertBufVertices (0)
{
}

CryCharRenderElement::~CryCharRenderElement ()
{
}

// returns true if the leaf buffer can be deleted immediately
bool CryCharRenderElement::canDestruct()
{
	// we can't delete the leaf buffer if we rendered it in this frame, and the rendering is still pending
	return !m_pLeafBuffer || g_GetIRenderer()->GetFrameID(false) != m_nLeafBufferLastRenderFrame;
}

#if 0
// creates the buffer with the given number of vertices and vertex format
// can only be called for a clean (without initialized leaf buffer) object
void CryCharRenderElement::create (int nVertCount, int nVertFormat, const char *szSource, unsigned numMaterials, bool bOnlyVideoBuffer)
{
	assert (!m_pLeafBuffer);
	m_numVertBufVertices = nVertCount;
  
	m_pLeafBuffer = GetIRenderer()->CreateLeafBufferInitialized (NULL, 0, nVertFormat, NULL, 0, R_PRIMV_TRIANGLES, szSource, eBT_Dynamic, numMaterials, 0, NULL, NULL, true);
  m_pLeafBuffer->m_bOnlyVideoBuffer = bOnlyVideoBuffer;
	// I suppose we can delete it just afterwards, but just incase let's pretend we've already rendered i in this frame
	m_nLeafBufferLastRenderFrame = GetIRenderer()->GetFrameID(false);
}
#endif

// creates the buffer with the given number of vertices and vertex format
// can only be called for a clean (without initialized leaf buffer) object
void CryCharRenderElement::create (int nVertCount, const struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F* pSourceData, const char *szSource, unsigned numMaterials, bool bOnlyVideoBuffer)
{
	assert (!m_pLeafBuffer);
	m_numVertBufVertices = nVertCount;

	m_pLeafBuffer = g_GetIRenderer()->CreateLeafBufferInitialized ((void *)pSourceData, nVertCount, VERTEX_FORMAT_P3F_COL4UB_TEX2F, NULL, 0, R_PRIMV_TRIANGLES, szSource, eBT_Dynamic, numMaterials, 0, NULL, NULL, bOnlyVideoBuffer);

	// I suppose we can delete it just afterwards, but just incase let's pretend we've already rendered i in this frame
	m_nLeafBufferLastRenderFrame = g_GetIRenderer()->GetFrameID(false);
}


// recreates videobuffer
void CryCharRenderElement::recreate()
{
	m_pLeafBuffer->CreateVidVertices(m_pLeafBuffer->m_SecVertCount, m_pLeafBuffer->m_nVertexFormat);
}

// returns the number of vertices allocated in the current buffer, if any is allocated
unsigned CryCharRenderElement::numVertices()const
{
	return m_numVertBufVertices;
}

// returns the number of materials in the leaf buffer
unsigned CryCharRenderElement::numMaterials()const
{
	return m_pLeafBuffer?m_pLeafBuffer->m_pMats->size():0;
}

// sets the number of used materials in the leaf buffer
void CryCharRenderElement::resizeMaterials (unsigned numMaterials, IShader* pShader)
{
	if (m_pLeafBuffer)
	{
		unsigned numInitialized = (unsigned)m_pLeafBuffer->m_pMats->Count();
		m_pLeafBuffer->m_pMats->PreAllocate (numMaterials, numMaterials);

		for (; numInitialized < numMaterials; ++numInitialized)
		{
			// uninitialized material
			CMatInfo& rMatInfo = (*m_pLeafBuffer->m_pMats)[numInitialized];
			rMatInfo = CMatInfo();
			rMatInfo.shaderItem.m_pShader = pShader; //gRenDev->EF_LoadShader((char*)szEfName, -1, eEF_World, 0);
			rMatInfo.pRE                  = (CREOcLeaf*)g_GetIRenderer()->EF_CreateRE(eDATA_OcLeaf);
		}
	}
}


// deletes the current buffer, cleans up the object; can only be called when canDestruct() == true
void CryCharRenderElement::destruct ()
{
	// when we exit from the game, we can destruct this 
	//assert (canDestruct());
	if (m_pLeafBuffer)
	{
		g_GetIRenderer()->DeleteLeafBuffer(m_pLeafBuffer);
		detach();
	}
}

// detaches the leaf buffer from this object (forgets it)
void CryCharRenderElement::detach()
{
	m_numVertBufVertices = 0;
	m_pLeafBuffer = NULL;
	m_nLeafBufferLastRenderFrame = 0;
}


void CryCharRenderElement::update(bool bLock, bool bCopyToVideoBuffer)
{
	if (m_pLeafBuffer->m_pVertexBuffer)
	{
		// now we filled in all the components of the vertex buffer - we need to synchronize it with the hardware
		if (!bCopyToVideoBuffer)
			g_GetIRenderer()->UpdateBuffer (m_pLeafBuffer->m_pVertexBuffer, m_pLeafBuffer->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, m_pLeafBuffer->m_SecVertCount, !bLock, 0);
		else // otherwise unlock buffers
			g_GetIRenderer()->UpdateBuffer (m_pLeafBuffer->m_pVertexBuffer, 0, 0, !bLock, 0, 3);
	}
}


void CryCharRenderElement::render (CCObject* pObj)
{
	// we might not have the buffer now - nothing to render
	if (!m_pLeafBuffer)
		return;

	m_nLeafBufferLastRenderFrame = g_GetIRenderer()->GetFrameID(false);

	// add the render element to the renderer, using the local transformations & stuff of the 
	// same object as the character's and default sort order (with default shader)
	m_pLeafBuffer->AddRenderElements(pObj);
}

void CryCharRenderElement::assignMaterial (unsigned nMaterial, IShader* pShader, int nTextureId, int nFirstIndex, int numIndices, int nFirstVertex, int numVertices)
{
	if (m_pLeafBuffer)
	{
		CMatInfo& rMatInfo = (*m_pLeafBuffer->m_pMats)[nMaterial];
		rMatInfo.pRE->m_CustomTexBind[0] = nTextureId;
		m_pLeafBuffer->SetChunk(pShader, nFirstVertex, numVertices, nFirstIndex, numIndices, nMaterial);
		(*m_pLeafBuffer->m_pMats)[nMaterial].pRE->m_CustomTexBind[0] = nTextureId;
	}
}

void CryCharRenderElement::updateIndices (const unsigned short* pIndices, unsigned numIndices)
{
	getLeafBuffer()->UpdateVidIndices(pIndices, (int)numIndices);
	getLeafBuffer()->m_NumIndices = numIndices;
}
