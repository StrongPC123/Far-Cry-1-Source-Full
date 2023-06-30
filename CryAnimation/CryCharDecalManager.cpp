// NOTES:
// CRenderer::CreateLeafBufferInitialized creates a buffer with reserve of vertices and no indices.
// it can accept a fake pointer (like something on stack) for vertex array pointer.
// UpdateIndices() actually just copies the indices into the vertex buffer structure

#include "stdafx.h"
#include <StlUtils.h>
#include "CryGeometryInfo.h"
#include "CryEngineDecalInfo.h"
#include "VertexBufferArrayDrivers.h"
#include "CryCharDecalManager.h"
#include "CryCharDecalBuilder.h"
#include "CryCharDecal.h"
#include "CVars.h"
#include "MathUtils.h"

// nVertexAllocStep == 2^nVertexAllocStepLog2 is the step with which the vertex buffer will be enlarged when needed
enum {nVertexAllocStepLog2 = 8};
enum {nVertexAllocStep = (1 << nVertexAllocStepLog2)};
enum {nMaterialAllocStep = 1};
// the max number of vertices used for the character until the decals start to disappear
// note: this must be a multiple of nVertexAllocStep
enum {nMaxDecalVertices = (nVertexAllocStep * 6u)};

CryCharDecalManager::CStatistics CryCharDecalManager::g_Statistics;


CryCharDecalManager::CryCharDecalManager (class CryGeometryInfo* pGeomInfo):
	m_pGeometry (pGeomInfo),
	m_bNeedUpdateIndices (false)
{
	m_pShader = g_GetIRenderer()->EF_LoadShader("DecalCharacter", eSH_World, EF_SYSTEM);
}

CryCharDecalManager::~CryCharDecalManager ()
{
	DeleteLeafBuffer();
	DeleteOldRenderElements();
	
	if (!m_arrOldRE.empty())
	{
		if (!g_GetISystem()->IsQuitting())
			g_LogToFile ("Warning: ~CryCharDecalManager: There are still %d render elements present that may be in rendering queue. But since destruction was requested, attempting to destruct those elements", m_arrOldRE.size());
		for (unsigned i = 0; i < m_arrOldRE.size(); ++i)
			m_arrOldRE[i].destruct();
	}
}

// cleans up the old leaf buffers
void CryCharDecalManager::DeleteOldRenderElements()
{
	for (RenderElementArray::iterator it = m_arrOldRE.begin(); it != m_arrOldRE.end();)
		if (it->canDestruct())
		{
			it->destruct();
			it = m_arrOldRE.erase(it);
		}
		else
			++it;
}


//////////////////////////////////////////////////////////////////////////
// Calculates the required number of vertices for the current decal array
// If needed, recreates the current leaf buffer so that it contains the given
// number of vertices and reserved indices, both uninitialized.
// There are 0 used indices initially
void CryCharDecalManager::ReserveVertexBufferVertices (const Vec3d* pInPositions)
{
	// first, calculate how many vertices we'll need
	unsigned i, numDecals = (unsigned)m_arrDecals.size(), numVertices = 0;
	for (i = 0; i < numDecals; ++i)
	{
		numVertices += m_arrDecals[i].numVertices();
#if DECAL_USE_HELPERS
		numVertices += m_arrDecals[i].numHelperVertices();
#endif
	}

	while (numVertices > nMaxDecalVertices && !m_arrDecals.empty())
	{
		unsigned nDecalToDelete = (unsigned)(rand()%m_arrDecals.size());
		numVertices -= m_arrDecals[nDecalToDelete].numVertices();
#if DECAL_USE_HELPERS
		numVertices -= m_arrDecals[nDecalToDelete].numHelperVertices();
#endif
    m_arrDecals.erase (m_arrDecals.begin()+nDecalToDelete);
	}

	unsigned numMaterials = groupMaterials();

	// make sure the vertex buffer contains enough space for all vertices

	if (m_RE.numVertices() >= g_MeshInfo.numVertices && m_RE.numMaterials() >= numMaterials)
		return;// no need to reallocate

	DeleteLeafBuffer();

	// construct the data to initialize the new system buffer
	unsigned numVerticesToReserve = (numVertices + (nVertexAllocStep-1)) & ~(nVertexAllocStep-1);
	TElementaryArray<struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F> arrSourceVerts (numVerticesToReserve);
	RefreshVertexBufferVertices (pInPositions, &arrSourceVerts[0]);

	unsigned numMaterialsToReserve = numMaterials + nMaterialAllocStep;

	if (numVerticesToReserve > numVertices)
		memset (&arrSourceVerts[numVertices], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F) * (numVerticesToReserve - numVertices));
	
	m_RE.create (numVerticesToReserve, &arrSourceVerts[0], "AnimDecals", numMaterialsToReserve);
}


// sets up the given material to default state: just clean decal material
/*
void CryCharDecalManager::initDefaultMaterial (CMatInfo& rMat)
{
	rMat.pRE = (CREOcLeaf*)GetRenderer()->EF_CreateRE(eDATA_OcLeaf);
	rMat.pRE->m_pBuffer = m_pLeafBuffer;
	rMat.pRE->m_pChunk = &rMat;
  rMat.pRE->m_CustomTexBind[0] = 0x1000;
  rMat.pShader = GetRenderer()->EF_LoadShader("DecalCharacter", -1, eSH_World, EF_SYSTEM);
}
*/


// Request (add) a new decal to the character
void CryCharDecalManager::Add (CryEngineDecalInfo& Decal)
{
	//if (m_arrDecalRequests.empty())
		// only 1 decal per frame is supported
		m_arrDecalRequests.push_back(Decal);
}

// discards the decal request queue (not yet realized decals added through Add())
void CryCharDecalManager::DiscardRequests()
{
	m_arrDecalRequests.clear();
}


// cleans up all decals, destroys the vertex buffer
void CryCharDecalManager::clear()
{
	m_arrDecalRequests.clear();
	m_arrDecals.clear();
	DeleteLeafBuffer();
}


// deletes the leaf buffer
void CryCharDecalManager::DeleteLeafBuffer ()
{
	if (m_RE.canDestruct())
		m_RE.destruct();
	else
	{
		m_arrOldRE.push_back(m_RE);
		m_RE.detach();
	}
}


// cleans up the dead decals
// updates indices if needed; doesn't update vertices
// sets m_bNeedUpdateIndices to true if it has added something (and therefore refresh of indices is required)
void CryCharDecalManager::DeleteOldDecals()
{
	// if we delete one of the decals, we need to update indices;
	// updating vertices is not our business

	for (CDecalArray::iterator it = m_arrDecals.begin(); it != m_arrDecals.end();)
		if (it->isDead())
		{
			m_bNeedUpdateIndices = true;
			it = m_arrDecals.erase (it);
		}
		else
			++it;
}


//////////////////////////////////////////////////////////////////////////
// realizes (creates geometry for) unrealized(requested) decals
// NOTE: this also fills  the UVs in for the vertex stream
void CryCharDecalManager::Realize (const Vec3d* pPositions)
{
	DeleteOldRenderElements();

	// write the deformed vertices into the videobuffer
	// NOTE: 
	// we write to the old videobuffer; in the case Realization reallocates the current buffer,
	// the old videobuffer will still be rendered on this frame, so it must be updated now

	// in case the following assert works, it means that the sequence of drawing/skinning
	// has been changed; in this case remove this assert and switch the order of the following
	// Refresh and Realize calls in order to avoid decal flickers.
	//assert (!m_RE.getLeafBuffer() || GetRenderer()->GetFrameID() == m_RE.getLastRenderFrameID());

	if (!m_arrDecalRequests.empty())
  {
    int nnn = 0;
  }
	RefreshVertexBufferVertices (pPositions);
	if (m_bNeedUpdateIndices)
		RefreshVertexBufferIndices ();

	DeleteOldDecals();
	RealizeNewDecalRequests (pPositions);
}


// if there are decal requests, then converts them into the decal objects
// reserves the vertex/updates the index buffers, if need to be
// sets m_bNeedUpdateIndices to true if it has added something (and therefore refresh of indices is required)
void CryCharDecalManager::RealizeNewDecalRequests (const Vec3d* pPositions)
{
	if (m_arrDecalRequests.empty())
		return; // nothing to udpate

	// realize each unrealized decal, then clean up the array of unrealized decals
	for (unsigned nDecal = 0; nDecal < m_arrDecalRequests.size(); ++nDecal)
	{
		CryEngineDecalInfo& Decal = m_arrDecalRequests[nDecal];
		CryCharDecalBuilder builder (Decal, m_pGeometry, pPositions);
		if (!builder.numDecalFaces())
			continue; // we're not interested in this decal: we don't have any decals

		// starts fading out all decals that are very close to this new one
		//fadeOutCloseDecals (builder.getSourceLCS(), sqr(Decal.fSize/4));

		g_Statistics.onDecalAdd (builder.numDecalVertices(), builder.numDecalFaces());

		CryCharDecal NewDecal;
		NewDecal.buildFrom (builder);
		m_arrDecals.insert (std::lower_bound(m_arrDecals.begin(), m_arrDecals.end(), NewDecal), NewDecal);
		//m_arrDecals.resize(m_arrDecals.size()+1);
		//m_arrDecals.back().buildFrom (builder);
	}

	// after we realized the decal request, we don't need it anymore
	m_arrDecalRequests.clear();

	// make sure we have enough vertices
	ReserveVertexBufferVertices(pPositions);

	// need to recreate indices
	m_bNeedUpdateIndices = true;
}


// starts fading out all decals that are close enough to the given point
// NOTE: the radius is m^2 - it's the square of the radius of the sphere
void CryCharDecalManager::fadeOutCloseDecals (const Vec3d& ptCenter, float fRadius2)
{
	for (unsigned i = 0; i < m_arrDecals.size(); ++i)
	{
		if ( GetLengthSquared((m_arrDecals[i].getSourceLCS()-ptCenter)) < fRadius2)
			m_arrDecals[i].startFadeOut (2);
	}
}


// returns true if the Realize() needs to be called
// Since the Realize() updates the vertex buffers, creates decals from requests,
// it's always needed when there are decals
bool CryCharDecalManager::NeedRealize () const
{
	return !m_arrDecals.empty() || !m_arrDecalRequests.empty()
		// we don't actually need the vertices that Realize() receives in this case,
		// but we need it to be called in order to free these buffers
		|| !m_arrOldRE.empty();
}

void CopyVertex (Vec3d& vDst, const Vec3d& vSrc)
{
	struct XYZ {unsigned x,y,z;};
	assert (sizeof(XYZ) == 12 && sizeof(Vec3d) == 12);
	(XYZ&)vDst = (XYZ&)vSrc;
}


// put the deformed vertices into the videobuffer of the given format,
// using the deformed character skin
void CryCharDecalManager::RefreshVertexBufferVertices (const Vec3d* pInPositions, struct_VERTEX_FORMAT_P3F_TEX2F* pDst)
{
	// scan through all decals
	CDecalArray::const_iterator itDecal = m_arrDecals.begin(), itDecalEnd = itDecal + m_arrDecals.size();

	for (; itDecal != itDecalEnd; ++itDecal)
	{
		// for each decal, there are a number of vertices to fill in; so we fill them in.
		// this is the number of vertices for the current decal
		unsigned numDecalVertices = itDecal->numVertices();
		for (unsigned nDecalVertex = 0; nDecalVertex < numDecalVertices; ++nDecalVertex)
		{
			const CryCharDecalVertex& rDecalVertex = itDecal->getVertex(nDecalVertex);
			const Vec3d& vCharPosition = pInPositions[rDecalVertex.nVertex];
			pDst->xyz = vCharPosition;
			pDst->st[0] = rDecalVertex.uvNew.u;
			pDst->st[1] = rDecalVertex.uvNew.v;
			++pDst;
		}

#if DECAL_USE_HELPERS
		unsigned numHelperVertices = itDecal->numHelperVertices();
		for (unsigned nHelperVertex = 0; nHelperVertex < numHelperVertices; ++nHelperVertex)
		{
			Vec3d vCharPosition = itDecal->getHelperVertex (nHelperVertex);
			CryUV uvCharUV = itDecal->getHelperUV (nHelperVertex);
			pDst->xyz = vCharPosition;
			pDst->st[0] = uvCharUV.u;
			pDst->st[1] = uvCharUV.v;
			++pDst;
		}

#endif
	}
}

// put the deformed vertices into the videobuffer of the given format
void CryCharDecalManager::RefreshVertexBufferVertices (const Vec3d* pInPositions, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F* pDst)
{
	// scan through all decals
	CDecalArray::const_iterator itDecal = m_arrDecals.begin(), itDecalEnd = itDecal + m_arrDecals.size();

	for (; itDecal != itDecalEnd; ++itDecal)
	{
		float fIntensity = itDecal->getIntensity() * 0.7f + 0.3f;
		int nIntensity = (int)((float)0xFF * fIntensity);
		DWORD dwColor = nIntensity | (nIntensity << 8) | (nIntensity << 16) | (nIntensity << 24);

		// for each decal, there are a number of vertices to fill in; so we fill them in.
		// this is the number of vertices for the current decal
		unsigned numDecalVertices = itDecal->numVertices();
		for (unsigned nDecalVertex = 0; nDecalVertex < numDecalVertices; ++nDecalVertex)
		{
			const CryCharDecalVertex& rDecalVertex = itDecal->getVertex(nDecalVertex);
			const Vec3d& vCharPosition = pInPositions[rDecalVertex.nVertex];
			pDst->xyz = vCharPosition;
			pDst->color.dcolor = dwColor;
			pDst->st[0] = (rDecalVertex.uvNew.u-0.5f) / fIntensity + 0.5f;
			pDst->st[1] = (rDecalVertex.uvNew.v-0.5f) / fIntensity + 0.5f;
			++pDst;
		}

#if DECAL_USE_HELPERS
		unsigned numHelperVertices = itDecal->numHelperVertices();
		for (unsigned nHelperVertex = 0; nHelperVertex < numHelperVertices; ++nHelperVertex)
		{
			Vec3d vCharPosition = itDecal->getHelperVertex (nHelperVertex);
			CryUV uvCharUV = itDecal->getHelperUV (nHelperVertex);
			pDst->xyz = vCharPosition;
			pDst->color.decolor = 0xFFFFFFFF;
			pDst->st[0] = uvCharUV.u;
			pDst->st[1] = uvCharUV.v;
			++pDst;
		}

#endif
	}
}


// put the vertices out of the vertex/normal array (that belongs to the character) to the
// decal vertex array. Decal vertices are a bit shifted toward the vertex normals (extruded)
// to ensure the decals are above the character skin
void CryCharDecalManager::RefreshVertexBufferVertices (const Vec3d* pInPositions)
{
	CLeafBuffer* pLB = m_RE.getLeafBuffer();
	if (!pLB)
		return;
	CLeafBuffer* pVertexContainer = pLB->GetVertexContainer();
	if (!pVertexContainer)
		return;

	if (pVertexContainer->m_pVertexBuffer == NULL)
		m_RE.recreate();

	m_RE.lock (true);

	bool bNeedCopy = true;

	{
		// choose an optimized for the vertex format routine, if there is one
		switch (pVertexContainer->m_pVertexBuffer->m_vertexformat)
		{
		case VERTEX_FORMAT_P3F_TEX2F:
			RefreshVertexBufferVertices(pInPositions, (struct_VERTEX_FORMAT_P3F_TEX2F*)pVertexContainer->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData);
			bNeedCopy = false;
			break;
		case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
			RefreshVertexBufferVertices(pInPositions, (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)pVertexContainer->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData);
			bNeedCopy = false;
			break;
		default:
			assert (0);
			break;
		}
	}

	if (bNeedCopy)
	{
		// scan through all decals
		CDecalArray::const_iterator itDecal = m_arrDecals.begin(), itDecalEnd = itDecal + m_arrDecals.size();

		// fill out the following sparse arrays found inthe leaf buffer
		CVertexBufferPosArrayDriver pOutPosition (m_RE.getLeafBuffer(), 0, false);
		CVertexBufferUVArrayDriver pOutUV (m_RE.getLeafBuffer(), 0, false);
		//CVertexBufferColorArrayDriver pOutColor (m_RE.getLeafBuffer(), 0, false);

		for (; itDecal != itDecalEnd; ++itDecal)
		{
			// for each decal, there are a number of vertices to fill in; so we fill them in.
			// this is the number of vertices for the current decal
			unsigned numDecalVertices = itDecal->numVertices();
			for (unsigned nDecalVertex = 0; nDecalVertex < numDecalVertices; ++nDecalVertex)
			{
				const CryCharDecalVertex& rDecalVertex = itDecal->getVertex(nDecalVertex);
				const Vec3d& vCharPosition = pInPositions[rDecalVertex.nVertex];
				//Vec3d vCharNormal   = arrInNormals[rDecalVertex.nVertex];
				// the decal mesh is slightly extruded and has the same normals
				*pOutPosition = vCharPosition;/* + vCharNormal * 0.02f*/;
				++pOutPosition;
				//*pOutColor = 0xFFFFFFFF;
				//++pOutColor;
				*pOutUV = rDecalVertex.uvNew;
				++pOutUV;
			}

	#if DECAL_USE_HELPERS
			unsigned numHelperVertices = itDecal->numHelperVertices();
			for (unsigned nHelperVertex = 0; nHelperVertex < numHelperVertices; ++nHelperVertex)
			{
				Vec3d vCharPosition = itDecal->getHelperVertex (nHelperVertex);
				CryUV uvCharUV = itDecal->getHelperUV (nHelperVertex);
				*pOutPosition = vCharPosition;
				++pOutPosition;
				//*pOutColor = 0xFFFFFFFF;
				//++pOutColor;
				*pOutUV = uvCharUV;
				++pOutUV;
			}

	#endif
		}
	}

	m_RE.unlock(true);
}


//////////////////////////////////////////////////////////////////////////
// recalculates the index array for the vertex buffer and replaces it (so that the vertex buffer is prepared for rendering)
// also makes sure the vertex buffer contains enough vertices to draw all the current decals (from m_arrDecals)
void CryCharDecalManager::RefreshVertexBufferIndices ()
{
	m_bNeedUpdateIndices = false;

	if (!m_RE.getLeafBuffer())
		return; // nothing to update - no vertex buffer

	if (m_RE.getLeafBuffer()->GetVertexContainer()->m_pVertexBuffer == NULL)
		m_RE.recreate();

	// calculate the number of required indices in the arrIndices array
	unsigned numDecals = (unsigned)m_arrDecals.size();
	unsigned numMaterials = groupMaterials ();

	// now we know the number of indices required
	if (g_MeshInfo.numIndices)
	{
		TElementaryArray<unsigned short> arrIndices;
		arrIndices.reinit (g_MeshInfo.numIndices);
		unsigned short* pIndex = &arrIndices[0];

		// for each face, put the 3 indices referring to the vertex in the char decal manager
		// vertex array
		unsigned short nBaseVertexIndex = 0;

		// scan through decals and add necessary indices (faces) to the index array
		for (unsigned i = 0; i < m_arrDecals.size(); ++i)
		{
			const CryCharDecal& rDecal = m_arrDecals[i];
			const CryCharDecalFace* pDecalFace = rDecal.getFaces(), *pDecalFaceEnd = pDecalFace + rDecal.numFaces();
			
			// scan through decal faces, and add 3 indices for each face
			for (; pDecalFace != pDecalFaceEnd; ++pDecalFace)
			{
				for (unsigned nVertexIndex = 0; nVertexIndex < 3; ++nVertexIndex)
					*(pIndex++) = nBaseVertexIndex + (*pDecalFace)[nVertexIndex];
			}
			
			// after scanning the decal faces, update the vertex base - we'll keep vertices in sequence:
			// 0th decal vertices first, then 1st decal vertices, etc. to the last decal.
			nBaseVertexIndex += rDecal.numVertices();

#if DECAL_USE_HELPERS

			// the same way - with the helper faces/vertices
			// scan through all helper faces
			for (unsigned nHelperFace = 0; nHelperFace < rDecal.numHelperFaces(); ++nHelperFace)
			{
				CryCharDecalFace faceHelper = rDecal.getHelperFace(nHelperFace);
				for (unsigned nVertexIndex = 0; nVertexIndex < 3; ++nVertexIndex)
					*(pIndex++) = nBaseVertexIndex + faceHelper[nVertexIndex];
			}

			nBaseVertexIndex += rDecal.numHelperVertices();
#endif
		}

		// we scanned through all decals, and vertex indices must be in the range and coinside
		// with the number that we calculated beforehand
		assert (g_MeshInfo.numVertices == nBaseVertexIndex);
		m_RE.updateIndices(&arrIndices[0], g_MeshInfo.numIndices);
	}

	// now assign the materials from submesh infos
	m_RE.resizeMaterials(numMaterials, m_pShader);
	for (unsigned nMaterial = 0; nMaterial < numMaterials; ++nMaterial)
		assignMaterial (nMaterial, g_SubmeshInfo[nMaterial].nTextureId, g_SubmeshInfo[nMaterial].nFirstIndex, g_SubmeshInfo[nMaterial].numIndices, g_SubmeshInfo[nMaterial].nFirstVertex, g_SubmeshInfo[nMaterial].numVertices);
}

// temporary locations for groupMaterials results
// the information about the decal mesh currently (after groupMaterials)
CryCharDecalManager::MeshInfo CryCharDecalManager::g_MeshInfo;
// the material groups
CryCharDecalManager::SubmeshInfo CryCharDecalManager::g_SubmeshInfo[CryCharDecalManager::g_numSubmeshInfos];


// returns the number of materials in g_SubmeshInfo
unsigned CryCharDecalManager::groupMaterials ()
{
	// scan through all decals, grouping them by texture ids and
	// watching the max number of types not reaching  g_numSubmeshInfos

	g_MeshInfo.numIndices = g_MeshInfo.numVertices = 0;
	unsigned numDecals = (unsigned)m_arrDecals.size();
	SubmeshInfo* pNext = g_SubmeshInfo; // the next available slot in the submesh info
	SubmeshInfo* pEnd = g_SubmeshInfo + g_numSubmeshInfos;
	for (unsigned i = 0; i < numDecals; ++i)
	{
		CryCharDecal& rDecal = m_arrDecals[i];

		if ((pNext == g_SubmeshInfo || pNext[-1].nTextureId != rDecal.getTextureId())
			&& pNext < pEnd) // we don't support more than the given amount of decal types
		{
			// add a new material group
			pNext->nFirstIndex  = g_MeshInfo.numIndices;
			pNext->numIndices   = 0;
			pNext->nFirstVertex = g_MeshInfo.numVertices;
			pNext->numVertices  = 0;
			pNext->nTextureId   = rDecal.getTextureId();
			++pNext;
		}

		g_MeshInfo.numIndices  += rDecal.numFaces() * 3;
		g_MeshInfo.numVertices += rDecal.numVertices();
#if DECAL_USE_HELPERS
		g_MeshInfo.numIndices  += rDecal.numHelperFaces() * 3;
		g_MeshInfo.numVertices += rDecal.numHelperVertices();
#endif
		pNext[-1].numIndices  = g_MeshInfo.numIndices  - pNext[-1].nFirstIndex;
		pNext[-1].numVertices = g_MeshInfo.numVertices - pNext[-1].nFirstVertex;
	}
	return pNext - g_SubmeshInfo;
}

// assigns the given material to the given range of indices/vertices
void CryCharDecalManager::assignMaterial (unsigned nMaterial, int nTextureId, int nFirstIndex, int numIndices, int nFirstVertex, int numVertices)
{
	m_RE.assignMaterial (nMaterial, m_pShader, g_GetCVars()->ca_DefaultDecalTexture()?0x1000:nTextureId, nFirstIndex, numIndices, nFirstVertex, numVertices);
	/*if (!m_RE.getLeafBuffer())
		return;
	CMatInfo& rMatInfo = (*m_RE.getLeafBuffer()->m_pMats)[nMaterial];
	rMatInfo.pRE->m_CustomTexBind[0] = nTextureId;
	m_RE.getLeafBuffer()->SetChunk(m_pShader, nFirstVertex, numVertices, nFirstIndex, numIndices, nMaterial);
	(*m_RE.getLeafBuffer()->m_pMats)[nMaterial].pRE->m_CustomTexBind[0] = GetCVars()->ca_DefaultDecalTexture()?0x1000 : nTextureId;*/
}

// adds the render data to the renderer, so that the current decals can be rendered
void CryCharDecalManager::AddRenderData (CCObject *pObj, const SRendParams & rRendParams)
{
	if(!m_RE.getLeafBuffer() || !m_RE.getLeafBuffer()->m_pVertexBuffer || m_arrDecals.empty())
		return; // we don't add render data if there's no vertex buffer or no decals
	m_RE.render (pObj);
#if DECAL_USE_HELPERS
	Vec3d vPos = rRendParams.vPos;
	Vec3d vAngles = rRendParams.vAngles;
	Matrix matTranRotMatrix;
	if (rRendParams.pMatrix)
		matTranRotMatrix = *rRendParams.pMatrix;
	else
	{
		//matTranRotMatrix.Identity();
		//matTranRotMatrix = GetTranslationMat(vPos)*matTranRotMatrix;
		//matTranRotMatrix = GetRotationZYX44(-gf_DEGTORAD*vAngles )*matTranRotMatrix; //NOTE: angles in radians and negated 

		//OPTIMIZED_BY_IVO
		matTranRotMatrix = Matrix34::GetRotationXYZ34( Deg2Rad(vAngles), vPos );
		matTranRotMatrix = GetTransposed44(matTranRotMatrix); //TODO: remove this after E3 and use Matrix34 instead of Matrix44
	}
	for (CDecalArray::iterator it = m_arrDecals.begin(); it != m_arrDecals.end(); ++it)
	{
		it->debugDraw(matTranRotMatrix);
	}
#endif
}

void CryCharDecalManager::LogStatistics()
{
	if (!g_Statistics.empty())
		g_GetLog()->LogToFile ("%d decals created, %d total vertices, %d faces, %.1f vers/decal, %.1f faces/decal",
		g_Statistics.numDecals, g_Statistics.numDecalVertices, g_Statistics.numDecalFaces,
		g_Statistics.getAveVertsPerDecal(), g_Statistics.getAveFacesPerDecal());
}

void CryCharDecalManager::CStatistics::onDecalAdd (unsigned numVertices, unsigned numFaces)
{
	++numDecals;
	numDecalVertices += numVertices;
	numDecalFaces += numFaces;
#ifdef _DEBUG
	if (g_GetCVars()->ca_Debug())
	{
		char szBuf[1024];
		sprintf (szBuf, "Decal added (%d vert, %d faces)\n", numVertices, numFaces);

		#ifdef WIN32
		OutputDebugString (szBuf);
		#endif

		#ifdef GAMECUBE
			OSReport(szBuf);
		#endif

	}
#endif
}

// returns the memory usage by this object into the sizer
void CryCharDecalManager::GetMemoryUsage (ICrySizer* pSizer)
{
	unsigned nSize = sizeof(*this);
	nSize += capacityofArray (m_arrDecalRequests);
	nSize += capacityofArray (m_arrDecals);
	nSize += capacityofArray (m_arrOldRE);
	pSizer->AddObject (this, nSize);
}

void CryCharDecalManager::debugDump()
{
	unsigned numMaterials = groupMaterials ();
	g_GetLog()->Log ("\001   %d decals: %d chunks used, mesh is %d verts %d indices", m_arrDecals.size(), numMaterials, g_MeshInfo.numVertices, g_MeshInfo.numIndices);
	unsigned i;
	for (i = 0; i < m_arrDecals.size(); ++i)
	{
		CryCharDecal& rDecal = m_arrDecals[i];
		g_GetLog()->Log("\001      decal %3d: %d verts, %d faces, \"%s\" (texId=%d)",i, rDecal.numVertices(), rDecal.numFaces(), g_GetIRenderer()->EF_GetTextureByID(rDecal.getTextureId())->GetName(), rDecal.getTextureId());
	}
	for (i = 0; i < numMaterials; ++i)
	{
    SubmeshInfo &rSubmesh = g_SubmeshInfo[i];
		g_GetLog()->Log ("\001      chunk %d: %d verts @%d, %d indices @%d, texture %d \"%s\"", i, rSubmesh.numVertices, rSubmesh.nFirstVertex, rSubmesh.numIndices, rSubmesh.nFirstIndex, rSubmesh.nTextureId, g_GetIRenderer()->EF_GetTextureByID(rSubmesh.nTextureId)->GetName());
	}
}