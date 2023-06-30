#include "stdafx.h"
#include "CryCharFxTrail.h"
#include "VertexBufferArrayDrivers.h"
#include "CryCharInstanceRenderParams.h"
#include "CryModelState.h"

CryCharFxTrail::CryCharFxTrail (class CryModelState* pState, const CryCharFxTrailParams& params):
	m_Params(params), m_bVisible(true), m_numEntries(0), m_nHeadEntry(0), m_nLastFrameDeform(0), m_pParent(pState)
{
	m_pTimes.reinit(numMaxEntries());

	// I need to create a dummy array because otherwise the system buffer doesn't get created
	struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pDummy = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[numVertices()];
	memset (pDummy, 0, sizeof(pDummy[0]) * numVertices());
	m_RE.create (numVertices(), pDummy, "AnimFxTrail", 1, false);
	delete[]pDummy;

	m_pShader = g_GetIRenderer()->EF_LoadShader("TemplDecalAdd", eSH_World, EF_SYSTEM);

	m_RE.resizeMaterials(1, m_pShader);
}

CryCharFxTrail::~CryCharFxTrail()
{
	m_RE.destruct();
}


// gets called upon Deform() of the model state
void CryCharFxTrail::Deform (const Matrix44* pBones)
{
	int nFrame = g_GetIRenderer()->GetFrameID(false);
	if (m_nLastFrameDeform != nFrame)
	{
		UpdateEntries(pBones);
		m_nLastFrameDeform = nFrame;
	}
}

void CryCharFxTrail::UpdateEntries (const Matrix44* pBones)
{
	float fTime = g_GetTimer()->GetCurrTime();

	// add a new entry to the history
	if (m_numEntries)
	{
		// shrink the queue if the tail is too old
		if (fTime - m_pTimes[getTailEntry()] > m_Params.fLength)
			--m_numEntries;
	}

	// grow the entry queue
	m_numEntries = min(numMaxEntries(), m_numEntries + 1);

	// increment the head entry to fill the new data in
	m_nHeadEntry = (m_nHeadEntry + numMaxEntries() - 1) % numMaxEntries();

	m_pTimes[m_nHeadEntry] = fTime;

	CVertexBufferPosArrayDriver pPos(m_RE.getLeafBuffer());
	CVertexBufferColorArrayDriver pColor(m_RE.getLeafBuffer());
	CVertexBufferUVArrayDriver pUV(m_RE.getLeafBuffer());

	unsigned i, nEntry = m_nHeadEntry;
	for (i = 0; i < m_Params.numVerts; ++i)
	{
		pPos[nEntry*m_Params.numVerts+i] = pBones[m_Params.nBone].TransformPointOLD(m_Params.vLine[i]);
		pUV[nEntry*m_Params.numVerts+i].u = float(i)/(m_Params.numVerts-1);
		pUV[nEntry*m_Params.numVerts+i].v = 0;
		pColor[nEntry*m_Params.numVerts+i] = 0xFFFFFFFF;
	}

	// fill all the old entries with the corresponding UVs
	for (i = 1; i < m_numEntries; ++i)
	{
		nEntry = (m_nHeadEntry + i) % numMaxEntries();
		float fAge = (fTime - m_pTimes[nEntry]) / m_Params.fLength;
		for (unsigned j = 0; j < m_Params.numVerts; ++j)
		{
			pUV[nEntry*m_Params.numVerts+j].u = float(j)/(m_Params.numVerts-1);
			pUV[nEntry*m_Params.numVerts+j].v = fAge;
			pColor[nEntry*m_Params.numVerts+j] = ((unsigned)(0xFF * fAge) << 24) + 0xFFFFFF;
		}
	}

	m_RE.getLeafBuffer()->InvalidateVideoBuffer();

	if (m_numEntries > 1)
		UpdateIndices();
	else
		m_RE.updateIndices(NULL, 0);
}

void CryCharFxTrail::UpdateIndices()
{
	std::vector<unsigned short> arrIndices;
	// 6 indices per quad; numVerts-1 quads per strip; numEntries-1 strips
	arrIndices.resize ((m_Params.numVerts-1) * 6 * (m_numEntries - 1));
	
	unsigned nStrip, nQuad, nBase = 0;
	for (nStrip = 1; nStrip < m_numEntries; ++nStrip)
	{
		unsigned nEntry = (m_nHeadEntry + nStrip)%numMaxEntries();
		unsigned nPrevEntry = (m_nHeadEntry + nStrip - 1)%numMaxEntries();
		for (nQuad = 1; nQuad < m_Params.numVerts; ++nQuad)
		{
			arrIndices[nBase++] = nEntry*m_Params.numVerts + nQuad - 1;
			arrIndices[nBase++] = nEntry*m_Params.numVerts + nQuad;
			arrIndices[nBase++] = nPrevEntry*m_Params.numVerts + nQuad;
			arrIndices[nBase++] = nEntry*m_Params.numVerts + nQuad - 1;
			arrIndices[nBase++] = nPrevEntry*m_Params.numVerts + nQuad;
			arrIndices[nBase++] = nPrevEntry*m_Params.numVerts + nQuad - 1;
		}
	}

	m_RE.updateIndices(&arrIndices[0], arrIndices.size());

	// start with the whole diapason of vertices in the vertex buffer. If the circular queue
	// consists of only one piece (doesn't wrap around the end), it'll be this piece
	int nFirstVertex = 0;
	int numVertices = this->numVertices();

	if (m_nHeadEntry + m_numEntries <= numMaxEntries())
	{
		// doesn't wrap around
		nFirstVertex = m_nHeadEntry * m_Params.numVerts;
		numVertices  = m_numEntries * m_Params.numVerts;
	}

	m_RE.assignMaterial(0, m_pShader, m_Params.nTextureId, 0, m_numEntries * m_Params.numVerts, nFirstVertex, numVertices);
}


void CryCharFxTrail::Render (const struct SRendParams & RendParams, Matrix44& mtxObjMatrix, CryCharInstanceRenderParams& rCharParams)
{
	return;
	CCObject * pObj = rCharParams.NewCryCharCCObject(RendParams, mtxObjMatrix, NULL);

	AddRenderData(pObj, RendParams);
}


// adds the render data to the renderer, so that the current decals can be rendered
void CryCharFxTrail::AddRenderData (CCObject *pObj, const SRendParams & rRendParams)
{
	// we must have the leaf buffer AND at least 2 history entries to form a quad
	if(m_bVisible && m_RE.getLeafBuffer() && m_numEntries >= 2)
		m_RE.render(pObj);
}


//! Renderer calls this function to allow update the video vertex buffers right before the rendering
void CryCharFxTrail::ProcessSkinning (const Vec3& t, const Matrix44& mtxModel, int nTemplate, int nLod, bool bForceUpdate)
{
	//Deform (m_pParent->getBoneGlobalMatrices());
}

// returns the memory usage by this object into the sizer
// TODO: use
void CryCharFxTrail::GetMemoryUsage (ICrySizer* pSizer)
{
	pSizer->AddObject(this, sizeof(*this) + sizeof(m_pTimes[0]) * numMaxEntries());
}
