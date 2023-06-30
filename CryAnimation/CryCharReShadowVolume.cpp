#include "stdafx.h"
//#include "CryAnimation.h"
#include "CVars.h"
#include "CryCharReShadowVolume.h"


CryCharReShadowVolume::CryCharReShadowVolume():
	m_pLeafBuffer (NULL),
	m_pMesh (NULL),
	m_nUsedMeshVertices(0),
	m_arrIndices ("CryCharReShadowVolume.Indices"),
	m_arrVertices ("CryCharReShadowVolume.Vertices"),
	m_nLastFrameSubmitted(0)
{
}

CryCharReShadowVolume::~CryCharReShadowVolume()
{
	clear();
}

void CryCharReShadowVolume::clear()
{
	if (m_pLeafBuffer)
	{
		g_GetIRenderer()->DeleteLeafBuffer (m_pLeafBuffer);
		m_pLeafBuffer = NULL;
	}
  
	if (m_pMesh)
	{
		m_pMesh->m_arrLBuffers[0].pVB = NULL;
		m_pMesh->Release();
		m_pMesh = NULL;
	}
}	


// prepares for calculating the shadow volume	
// numVertices is the minimal length of the vertex buffer
// numIndices is the minimal length of the index buffer
void CryCharReShadowVolume::prepare (unsigned numIndices, unsigned numVertices)
{
	bool bRecreate = false;

	m_nUsedMeshVertices = numVertices;

	// Realloc index in-mem buffer
	if (m_arrIndices.size() < numIndices)
	{
		m_arrIndices.reinit(numIndices);
	}
	
	// Realloc vertex in-mem buffer
	if (m_arrVertices.size() < numVertices)
	{
		m_arrVertices.reinit(numVertices);
	}

	if (m_pLeafBuffer && m_pLeafBuffer->m_SecVertCount < (int)numVertices)
	{
		g_GetIRenderer()->DeleteLeafBuffer (m_pLeafBuffer);
		m_pLeafBuffer = NULL;
	}

	if (!m_pMesh)
  {
		m_pMesh = (CRETriMeshShadow *)g_GetIRenderer()->EF_CreateRE (eDATA_TriMeshShadow);
    m_pMesh->m_bAnimatedObject = true;
  }
	
	if (!m_pMesh)
	{
		assert (0);
		g_GetLog()->LogError ("Cannot get the render element for the stencil shadow volume");
		return;
	}

	if (!m_pLeafBuffer)
	{
    assert(!m_arrVertices.empty());
    m_pLeafBuffer = g_GetIRenderer()->CreateLeafBufferInitialized(&m_arrVertices[0], numVertices, VERTEX_FORMAT_P3F, NULL, 0, R_PRIMV_TRIANGLES, "Character ShadowVolume");
    m_pLeafBuffer->SetChunk(0,0,numVertices,0,-1); // setup fake shunk to allow mfCheckUpdate
		m_pMesh->m_arrLBuffers[0].pVB = m_pLeafBuffer; // use always slot 0
	}
  m_pMesh->m_nRendIndices = numIndices;
}


// assuming the calculation of the shadow volume is finished, submits it to the renderer
void CryCharReShadowVolume::submit (const SRendParams *rParams, IShader* pShadowCull)
{
#ifdef _DEBUG
	{
		for (int i = 0; i < m_pLeafBuffer->m_Indices.m_nItems; ++i)
			assert (m_arrIndices[i] < m_pLeafBuffer->m_SecVertCount);
	}
#endif

	//get the effect object from the renderer
	CCObject *pObj = g_GetIRenderer()->EF_GetObject (true);

//  pObj->m_nScissorX1 = rParams->nScissorX1;
//  pObj->m_nScissorY1 = rParams->nScissorY1;
//  pObj->m_nScissorX2 = rParams->nScissorX2;
//  pObj->m_nScissorY2 = rParams->nScissorY2;

	pObj->m_DynLMMask = rParams->nDLightMask; // used for scissor test
  Matrix34 t				= Matrix34::CreateTranslationMat(rParams->vPos);
  Matrix33 r33			=	Matrix33::CreateRotationXYZ( Deg2Rad(Ang3(rParams->vAngles[0],-rParams->vAngles[1],rParams->vAngles[2])));	
  pObj->m_Matrix    = GetTransposed44(t*r33);
  pObj->m_ObjFlags |= FOB_TRANS_MASK;

//  m_pMesh->mfCheckUpdate(0);

//	GetRenderer()->UpdateBuffer(m_pLeafBuffer->m_pVertexBuffer,&m_arrVertices[0],numMeshVertices(),true);
	//GetRenderer()->UpdateIndices(m_pLeafBuffer->m_pVertexBuffer,&m_arrIndices[0],numMeshIndices());		
  
  // update verts in system buffer
  // it's better to make call back function and write directly into video memory
  // indices are passed to CRETriMeshShadow by pointer as before
  m_pLeafBuffer->UpdateSysVertices(&m_arrVertices[0],numMeshVertices());
  m_pLeafBuffer->UpdateSysIndices(&m_arrIndices[0],m_pMesh->m_nRendIndices);

	IShader * pSHStencil = g_GetIRenderer()->EF_LoadShader("<Stencil>", eSH_World, EF_SYSTEM);
	g_GetIRenderer()->EF_AddEf(0, (CRendElement *)m_pMesh , pSHStencil, NULL, pObj, -1, pShadowCull, rParams->nSortValue);		

	m_nLastFrameSubmitted = g_GetIRenderer()->GetFrameID();
	m_fLastTimeSubmitted = g_GetTimer()->GetCurrTime();
}


void CryCharReShadowVolume::GetMemoryUsage (ICrySizer* pSizer)
{
	pSizer->Add(*this);
	pSizer->AddContainer(m_arrIndices);
	pSizer->AddContainer(m_arrVertices);
}


// returns the age of this shadow volume: the current frame - the frame it was submitted last
unsigned CryCharReShadowVolume::getAgeFrames()
{
	return g_GetIRenderer()->GetFrameID() - m_nLastFrameSubmitted;
}

// returns the timeout from the last call to submit()
float CryCharReShadowVolume::getAgeSeconds()
{
	return g_GetTimer()->GetCurrTime() - m_fLastTimeSubmitted;
}
