#include "stdafx.h"
#include "cvars.h"
#include <MakMatInfoFromMAT_ENTITY.h>
#include <MeshIdx.h>
#include <CryCompiledFile.h>
#include <IEdgeConnectivityBuilder.h>
#include "CryCharReShadowVolume.h"
#include "CryModelState.h"
#include "VertexBufferArrayDrivers.h"
#include "CryModel.h"
#include "CryCharBody.h"
#include "CryCharDecalManager.h"
#include "CryModelSubmesh.h"
#include <VertexBufferSource.h>
#include "IncContHeap.h"
#include "CryModEffMorph.h"
#include "CrySkinMorph.h"
#include "CrySkinFull.h"
#include "CryCharInstance.h"
#include "DebugUtils.h"
#include "Cry_Camera.h"

// initializes and binds the submesh to the given model
// there's no way to change the model at runtime
CryModelSubmesh::CryModelSubmesh (CryModelState* pParent, CryModel* pMesh):
#ifdef DEBUG_STD_CONTAINERS
	m_arrMorphEffectors ("CryModelState.MorphTargets"),
#endif
	m_pParent(pParent),
	m_pDecalManager (NULL),
	m_nLastSkinBBoxUpdateFrameId(0),
	m_nLastTangentsUpdatedFrameId(0),
	m_nLastTangentsUpdatedLOD(-1),
	m_pMesh(pMesh),
	m_nFlags (FLAG_DEFAULT_FLAGS)
{
	// the submeshes that are not the default model submeshes lock their corresponding
	// CryModels. The default model can't lock its parent CryModel, because it will
	// be circular dependency. Owner ship is as follows: States->CryModel->Default State
	
	// It's for sure that the default model state is being created if there's none in the mesh yet.
	if (pParent != pMesh->m_pDefaultModelState && NULL != pMesh->m_pDefaultModelState)
		pMesh->m_pBody->AddRef();

	for (unsigned i = 0; i < SIZEOF_ARRAY(m_pLeafBuffers); ++i)
		m_pLeafBuffers[i] = NULL;
	if (pMesh->m_pDefaultModelState != pParent && pMesh->m_pDefaultModelState)
		CopyLeafBuffers(pMesh->m_pDefaultModelState->GetCryModelSubmesh(0)->m_pLeafBuffers);

	list2<CMatInfo>* pMats = getLeafBufferMaterials();

	if (pMats)
	{
		getShaderTemplates(0).reinit(pMats->Count(), -1);
		getShaderTemplates(1).reinit(pMats->Count(), -1);
	}

	m_nLastUpdatedLodLevel     = -1;
	for (int i = 0; i < g_nMaxLods; ++i)
	{
		m_nLastSkinnedFrameID[i]      = -1;
		m_nLastTangedFrameID[i]       = -1;
	}
}

// this is the array that's returned from the LeafBuffer
list2<CMatInfo>* CryModelSubmesh::getLeafBufferMaterials()
{
	if (m_pLeafBuffers[0])
		return m_pLeafBuffers[0]->m_pMats;
	else
		return NULL;
}


CryModelSubmesh::~CryModelSubmesh()
{
#ifdef UNIQUE_VERT_BUFF_PER_INSTANCE
	DeleteLeafBuffers();
#endif
	// if the vertex buffers are not unique per modelstate, then they're deleted by the CryModel destructor
	if (m_pDecalManager)
	{
		delete m_pDecalManager;
		m_pDecalManager = NULL;
	}

	// the submeshes that are not the default model submeshes lock their corresponding
	// CryModels. The default model can't lock its parent CryModel, because it will
	// be circular dependency. Owner ship is as follows: States->CryModel->Default State
	if (m_pParent != m_pMesh->m_pDefaultModelState)
		m_pMesh->m_pBody->Release();
}

void CryModelSubmesh::CopyLeafBuffers (CLeafBuffer** pLeafBuffers)
{
	// copy leaf buffers
	for(unsigned nLod=0; nLod < m_pMesh->numLODs(); nLod++)
	{
		assert(pLeafBuffers[nLod]);
#ifdef UNIQUE_VERT_BUFF_PER_INSTANCE
		CLeafBuffer* pSrcLeafBuffer = pLeafBuffers[nLod];
		CLeafBuffer* pNewLeafBuffer = g_GetIRenderer()->CreateLeafBuffer(eBT_Dynamic);//new CLeafBuffer;
		pSrcLeafBuffer->CopyTo(pNewLeafBuffer, false);
		m_pLeafBuffers[nLod] = pNewLeafBuffer;
#else
		m_pLeafBuffers[nLod] = pLeafBuffers[nLod];
#endif
	}
}


// returns the model of the submesh, or NULL in case of failure
ICryCharModel* CryModelSubmesh::GetModel ()
{
	return m_pMesh->m_pBody;
}

// Disposes the leaf buffers allocated for 
void CryModelSubmesh::DeleteLeafBuffers()
{
	for(unsigned i=0; i<m_pMesh->numLODs(); i++)
	{
		if(!m_pLeafBuffers[i])
			return;

		if(list2<CMatInfo>* &pMats = m_pLeafBuffers[i]->m_pMats)
		{
			for (int m=0; m<pMats->Count(); m++)
			{
				CMatInfo *tmp = pMats->Get(m);
				SAFE_RELEASE(tmp->pRE);
				SShaderItem Sh = tmp->GetShaderItem();
				SAFE_RELEASE(Sh.m_pShader);
				SAFE_RELEASE(Sh.m_pShaderResources);
			}

			// during GenerateRenderArrays, we allocate this
			if (m_pParent == m_pMesh->m_pDefaultModelState && m_pMesh->m_bDeleteLBMats)
			{
				assert (m_pParent->GetCryModelSubmesh(0) == this);
				delete pMats;
				pMats = NULL;
			}
		}

		g_GetIRenderer()->DeleteLeafBuffer(m_pLeafBuffers[i]);
	}
}

//////////////////////////////////////////////////////////////////////
// Generation of the leaf buffers for the default model state.
// Done only once - when the default model state is initialized.
// szFileName is the full path to the file
void CryModelSubmesh::GenerateRenderArrays(const char * szFileName)
{
	AUTO_PROFILE_SECTION(g_dTimeGenRenderArrays);
	for(unsigned nLod = 0; nLod < m_pMesh->numLODs(); nLod++) 
	{
		CryGeometryInfo * pGeomInfo = m_pMesh->getGeometryInfo(nLod);

		if(!pGeomInfo->numVertices())
			break;


		// LeafBuffer generation:
		//  Allocate (CreateLeafBuffer) the leaf buffer from the renderer;
		//  Initialize (CreateBuffer) it from IndexedMesh generated out of GeomInfo
		//  Initialize the material list for the leaf buffer, and copy some material properties from
		//    the CryModel
		//  Sort the faces in the GeomInfo to synchronize their indices with those in the leaf buffer
		{ 
			CLeafBuffer * pVertBuffer =
				m_pLeafBuffers[nLod] =
				g_GetIRenderer()->CreateLeafBuffer(eBT_Dynamic, "CryModelArray");

			CIndexedMesh * pIndexedMesh = pGeomInfo->new3DEngineIndexedMesh();
			pGeomInfo->removeUVs(); // we won't need them any more

			std::vector<bool> arrMtlUsage;
			arrMtlUsage.resize (m_pMesh->numMaterials(), false);
			// [Sergiy] HACK Sort faces by material to synchronize with the actual leaf buffer
			// mark the used materials
			pGeomInfo->sortFacesByMaterial(arrMtlUsage);
			{
				AUTO_PROFILE_SECTION(g_dTimeShaderLoad);

				CMatInfo defaultMaterial;

				//calculate the index of max actually used material
				unsigned nMaterial, numUsedMtls = 0;
				for (nMaterial = 0; nMaterial < arrMtlUsage.size(); ++nMaterial)
					if (arrMtlUsage[nMaterial])
						numUsedMtls = nMaterial + 1;

				//g_GetLog()->LogPlus("\005%d of %d mtls used", numUsedMtls, arrMtlUsage.size());

				// make material table with clean elements
				list2<CMatInfo>& arrMats = *(pVertBuffer->m_pMats = new list2<CMatInfo>);
				arrMats.resize(numUsedMtls);

				for (nMaterial = 0; nMaterial < numUsedMtls; ++nMaterial)
				{
					CMatInfo& rMaterial = arrMats[nMaterial];
					// this is the hack that's required because list2 doesn't construct the elements: to copy the vtable
					*(unsigned int*)&rMaterial = *(unsigned int*)&defaultMaterial;

					if (arrMtlUsage[nMaterial])
					{
						rMaterial.m_Id = nMaterial;

						CIndexedMesh__LoadMaterial (NULL, CryStringUtils::GetParentDirectory(string(szFileName)).c_str(), rMaterial, g_GetIRenderer(), &m_pMesh->getMaterial(nMaterial));

						if (rMaterial.pRE)
							rMaterial.pRE->m_Flags |= FCEF_DYNAMIC;
					}
					else
						rMaterial = defaultMaterial;
				}
			}
			pVertBuffer->CreateBuffer (pIndexedMesh, g_GetCVars()->ca_StripifyGeometry()?true:false, 
				false); // do not remove any geometry because it will destroy mapping of animation data to leaf buffer data
			delete pIndexedMesh;

		}

		//assert (m_pLeafBuffers[nLod]->m_pMats->size() == m_pMesh->numMaterials());

		{
			// this is for the new skinning pass
			pGeomInfo->PrepareExtToIntMap (m_pLeafBuffers[nLod]->m_SecVertCount);

			pGeomInfo->initExtUVs (CVertexBufferUVArrayDriver (m_pLeafBuffers[nLod]));

			ushort * pInds = m_pLeafBuffers[nLod]->m_pIndicesPreStrip->Get(0);
			uint * pVertReMaping = m_pLeafBuffers[nLod]->m_arrVertStripMap;

			for (unsigned i=0; i<pGeomInfo->numFaces(); i++)
			{		
				if( pInds[0] >= m_pLeafBuffers[nLod]->m_SecVertCount ||
					pInds[1] >= m_pLeafBuffers[nLod]->m_SecVertCount ||
					pInds[2] >= m_pLeafBuffers[nLod]->m_SecVertCount )
				{
					g_GetConsole()->Exit("CryModelState::GenerateRenderArrays: indices out of range: %s", szFileName);
					break;
				}

				GeomFace face = pGeomInfo->getFace(i);

				if(pVertReMaping)
				{
					for (int v = 0; v < 3; ++v)
					{
						assert(pVertReMaping[pInds[v]]<pGeomInfo->numExtToIntMapEntries());//flip
						pGeomInfo->getExtToIntMapEntry(pVertReMaping[pInds[v]]) = face[v];//flip
					}
				}
				else
				{
					for (int v = 0; v < 3; ++v)
						pGeomInfo->getExtToIntMapEntry(pInds[v]) = face[v];//flip
				}

				pInds += 3;
			}

			pGeomInfo->CreateIntToExtMap();
		}
	}
}


// Creates Leaf buffers for all LODs
// szTextureDir is the texture directory used to create the shaders
void CryModelSubmesh::GenerateRenderArraysCCG(const char * szTextureDir)
{
	AUTO_PROFILE_SECTION(g_dTimeGenRenderArrays);

	for(unsigned nLod = 0; nLod < m_pMesh->numLODs(); nLod++) 
	{
		CryGeometryInfo * pGeomInfo = m_pMesh->getGeometryInfo(nLod);

		///////////
		// construct the vertices for the renderer to calculate its stuff
		//
		CrySkinFull* pSkin = pGeomInfo->getGeomSkin();
		g_Temp.reserve (sizeof(Vec3d)*(pGeomInfo->numUsedVertices()+pGeomInfo->numExtToIntMapEntries()));
		Vec3d* pVertices = (Vec3d*)g_Temp.data();
		pSkin->skin (m_pParent->getBoneGlobalMatrices(), pVertices);
		Vec3d* pExtVertices = pVertices + pGeomInfo->numUsedVertices();

		const unsigned* pExtToIntMap = pGeomInfo->getExtToIntMapEntries();
		for (unsigned i = 0; i < pGeomInfo->numExtToIntMapEntries(); ++i)	
			pExtVertices[i] = pVertices[pExtToIntMap[i]];

		CMatInfo defaultMaterial;
		list2<CMatInfo>* pMats = new list2<CMatInfo>;
		pMats->resize(m_pMesh->numMaterials());

		{
			AUTO_PROFILE_SECTION(g_dTimeShaderLoad);

			for (unsigned nMaterial = 0; nMaterial < pMats->size(); ++nMaterial)
			{
				CMatInfo& rMaterial = (*pMats)[nMaterial];
				// this is the hack that's required because list2 doesn't construct the elements: to copy the vtable
				*(unsigned int*)&rMaterial = *(unsigned int*)&defaultMaterial;
				rMaterial.m_Id = nMaterial;

				CIndexedMesh__LoadMaterial (NULL, szTextureDir, rMaterial, g_GetIRenderer(), &m_pMesh->getMaterial(nMaterial));

				if (rMaterial.pRE)
					rMaterial.pRE->m_Flags |= FCEF_DYNAMIC;
			}
		}

		/////////////////////////////////////////////////////////////////////////////////
		// Construct the Leaf Buffer
		//assert (m_pMesh->m_arrShaders.size() == m_pMesh->m_arrMaterials.size());
		m_pLeafBuffers[nLod] = g_GetIRenderer()->CreateLeafBuffer(eBT_Dynamic, "ModelState");

		VertexBufferSource vbs;
		vbs.numMaterials = (unsigned)m_pMesh->m_arrMaterials.size();
		vbs.pMaterials   = &m_pMesh->m_arrMaterials[0];
		vbs.pShaders     = NULL;
		vbs.pMats        = pMats;
		vbs.numPrimGroups = (unsigned)pGeomInfo->m_arrPrimGroups.size();
		vbs.pPrimGroups  = &pGeomInfo->m_arrPrimGroups[0];
		vbs.numIndices   = (unsigned)pGeomInfo->m_arrIndices.size();
		vbs.pIndices     = &pGeomInfo->m_arrIndices[0];
		vbs.numVertices  = pGeomInfo->numExtToIntMapEntries();
		vbs.pUVs         = pGeomInfo->getExtUVs();
		vbs.pVertices    = pExtVertices;
		vbs.nREFlags     = FCEF_DYNAMIC;
		vbs.pAndFlags    = NULL;
		vbs.pOrFlags     = NULL;

		m_pLeafBuffers[nLod]->CreateBuffer (&vbs);
	}
}

void CryModelSubmesh::AddCurrentRenderData(CCObject *obj, CCObject *obj1, const SRendParams & rParams)
{
	//g_GetLog()->LogToFile("\003 %d.rendering %x %s", g_nFrameID, this, m_pMesh->getFilePathCStr());
	int nSort = rParams.nSortValue;

	// prevent from render list changing
	// todo: use define here
	if (obj->m_ObjFlags & FOB_NEAREST)
		nSort = eS_Nearest | (nSort & EFSLIST_MASK); 

	int nLod = m_pParent->m_nLodLevel;
	obj->m_nLod = nLod;
	if (obj1)
	{
		obj1->m_nLod = nLod;
		//obj->m_ObjFlags |= FOB_REFRACTED; // moved from Game01
	}
	CLeafBuffer* pLeafBuffer = m_pLeafBuffers[nLod];
	pLeafBuffer->m_vBoxMin = m_SubBBox.vMin;
	pLeafBuffer->m_vBoxMax = m_SubBBox.vMax;

	//  m_pLeafBuffers[nLod]->SetRECustomData(&(m_arrHeatSourcePos[0].x));

	int nTempl1;

	CryGeometryInfo* pGeomInfo = m_pMesh->getGeometryInfo(nLod);
	if (pGeomInfo->m_arrPrimGroups.empty())
	{
		// the old version : we don't have mapping between materials in mesh and materials in the Leaf Buffers
		for(unsigned i=0; i<pLeafBuffer->m_pMats->size(); i++)
		{ 
			SShaderItem si = (*pLeafBuffer->m_pMats)[i].shaderItem;
			// Override object material.
			if (rParams.pMaterial)
			{
				rParams.pMaterial->OverrideShaderItem( i,si );
			}
			CREOcLeaf * pREOcLeaf  = (*pLeafBuffer->m_pMats)[i].pRE;

			if (si.m_pShader && i<(unsigned)pLeafBuffer->m_pMats->Count() && pREOcLeaf)
			{
				nTempl1 = -1;
				int nTempl = rParams.nShaderTemplate;
				if (nTempl == -2)
				{
					nTempl = getShaderTemplates(0)[i];
					nTempl1 = getShaderTemplates(1)[i];
				}
				else
					if (rParams.nShaderTemplate > 0)
					{
						int nTempl = rParams.nShaderTemplate;
						si.m_pShader->AddTemplate(si.m_pShaderResources, nTempl, NULL);
					}
					if (nTempl > 0)
						obj->m_nTemplId = nTempl;
					g_GetIRenderer()->EF_AddEf(rParams.nFogVolumeID, pREOcLeaf, si.m_pShader, si.m_pShaderResources, obj, nTempl, m_pParent->m_pShaderStateCull?m_pParent->m_pShaderStateCull: rParams.pStateShader , nSort);
					if (nTempl1 > 0)
						g_GetIRenderer()->EF_AddEf(rParams.nFogVolumeID, pREOcLeaf, si.m_pShader, si.m_pShaderResources, obj1, nTempl1, m_pParent->m_pShaderStateCull?m_pParent->m_pShaderStateCull:rParams.pStateShader , nSort);
			}
		}
	}
	else
	{
		// the new version : we do have mapping between materials in mesh and materials in the Leaf Buffers
		for (unsigned nPrimGroup=0; nPrimGroup<pGeomInfo->m_arrPrimGroups.size(); nPrimGroup++)
		{ 
			unsigned nMaterial = pGeomInfo->m_arrPrimGroups[nPrimGroup].nMaterial;
			CMatInfo& mi = (*pLeafBuffer->m_pMats)[nMaterial];
			CREOcLeaf * pREOcLeaf  = mi.pRE;
			SShaderItem si = mi.shaderItem;//m_pMesh->getShader(nMaterial);

			// Override object material.
			if (rParams.pMaterial)
			{
				// Assume that the root material is the first material, sub materials start from index 1.
				if (nPrimGroup == 0)
					si = rParams.pMaterial->GetShaderItem();
				else if (nPrimGroup-1 < unsigned(rParams.pMaterial->GetSubMtlCount()))
				{
					si = rParams.pMaterial->GetSubMtl(nPrimGroup-1)->GetShaderItem();
				}
			}

			assert (nPrimGroup<(unsigned)pLeafBuffer->m_pMats->size() && pREOcLeaf);

			if (si.m_pShader)
			{
				nTempl1 = -1;
				int nTempl = rParams.nShaderTemplate;
				if (nTempl == -2)
				{
					nTempl  = getShaderTemplates(0)[nPrimGroup];
					nTempl1 = getShaderTemplates(1)[nPrimGroup];
				}
				else
					if (rParams.nShaderTemplate > 0)
					{
						int nTempl = rParams.nShaderTemplate;
						si.m_pShader->AddTemplate(si.m_pShaderResources, nTempl, NULL);
					}
					if (nTempl > 0)
						obj->m_nTemplId = nTempl;
					if (nTempl1 > 0)
						obj1->m_nTemplId = nTempl1;

					// vlad: set default sorting in case of cryvision when set as custom teplate
					if(rParams.nShaderTemplate && si.m_pShader->GetTemplate(nTempl)->GetSort() == eS_HeatVision)
					{
						g_GetIRenderer()->EF_AddEf(rParams.nFogVolumeID, pREOcLeaf, si.m_pShader, si.m_pShaderResources, obj, nTempl, m_pParent->m_pShaderStateCull?m_pParent->m_pShaderStateCull:rParams.pStateShader);
						continue;
					}

					g_GetIRenderer()->EF_AddEf(rParams.nFogVolumeID, pREOcLeaf, si.m_pShader, si.m_pShaderResources, obj, nTempl, m_pParent->m_pShaderStateCull?m_pParent->m_pShaderStateCull:rParams.pStateShader , nSort);
					if (nTempl1 > 0)
						g_GetIRenderer()->EF_AddEf(rParams.nFogVolumeID, pREOcLeaf, si.m_pShader, si.m_pShaderResources, obj1, nTempl1, m_pParent->m_pShaderStateCull?m_pParent->m_pShaderStateCull:rParams.pStateShader , nSort);			}
		}
	}
}


// renders the decals - adds the render element to the renderer
void CryModelSubmesh::AddDecalRenderData (CCObject *pObj, const SRendParams & rRendParams)
{
	if (m_pDecalManager) // we don't render the character decals in lod!=0
		m_pDecalManager->AddRenderData (pObj, rRendParams);
}

// returns true if calling Morph () is really necessary (there are any pending morphs)
bool CryModelSubmesh::NeedMorph ()
{
	return !m_arrMorphEffectors.empty();
}

// morphs the LOD 0 into the given destination (already skinned) buffer
void CryModelSubmesh::MorphWithSkin (Vec3d* pDst, Vec3dA16* pDstNormalsA16)
{
	for (unsigned nMorphEffector = 0; nMorphEffector < m_arrMorphEffectors.size(); ++nMorphEffector)
	{
		const CryModEffMorph& rMorphEffector = m_arrMorphEffectors[nMorphEffector];
		int nMorphTargetId = rMorphEffector.getMorphTargetId ();
		if (nMorphTargetId < 0)
			continue;
		const CrySkinMorph& rMorphSkin = m_pMesh->getMorphSkin (0, nMorphTargetId);
		float fBlending = rMorphEffector.getBlending();

		if (pDstNormalsA16 && g_GetCVars()->ca_MorphNormals())
			rMorphSkin.skin (m_pParent->getBoneGlobalMatrices(), fBlending, pDst, pDstNormalsA16, 1.0f * g_GetCVars()->ca_MorphNormals());
		else
			rMorphSkin.skin (m_pParent->getBoneGlobalMatrices(), fBlending, pDst);
	}
}



// Software skinning: calculate positions and normals
//////////////////////////////////////////////////////////////////////
void CryModelSubmesh::Deform( int nLodToDeform, unsigned nDeformFlags)
{
	DEFINE_PROFILER_FUNCTION();
#ifdef _DEBUG
	if (m_pParent->m_bOriginalPose && CryStringUtils::stristr(m_pMesh->getFilePathCStr(),"weapon"))
		g_GetLog()->LogWarning ("\001%d. Deforming (lod %d, model %s) without any animations applied!", g_nFrameID, nLodToDeform, m_pMesh->getFilePathCStr());
#endif
	if ((unsigned)nLodToDeform>=SIZEOF_ARRAY(m_pLeafBuffers))
	{
		g_GetLog()->LogError ("\001CryModelState::Deform(lod=%d,%s,%s,%s):invalid lod", nLodToDeform, (nDeformFlags&FLAG_DEFORM_UPDATE_TANGENTS)?"Update Tangents":"", (nDeformFlags&FLAG_DEFORM_UPDATE_NORMALS)?"Update Normals":"", (nDeformFlags&FLAG_DEFORM_FORCE_UPDATE)?"Force Update":"");
		return;
	}

	CLeafBuffer *lb = m_pLeafBuffers[nLodToDeform];

	if (!lb)
	{
		g_GetLog()->LogError ("\001CryModelState::Deform(lod=%d,%s,%s,%s):no lod leaf buffer", nLodToDeform, (nDeformFlags&FLAG_DEFORM_UPDATE_TANGENTS)?"Update Tangents":"", (nDeformFlags&FLAG_DEFORM_UPDATE_NORMALS)?"Update Normals":"", (nDeformFlags&FLAG_DEFORM_FORCE_UPDATE)?"Force Update":"");
		return;
	}

	CVertexBuffer* pRenderVertexBuffer = lb->GetVertexContainer()->m_pVertexBuffer;
	if (!pRenderVertexBuffer)
	{
		g_GetLog()->LogError ("\001CryModelState::Deform(lod=%d,%s,%s,%s):no vertex buffer in leaf buffer %x", nLodToDeform, (nDeformFlags&FLAG_DEFORM_UPDATE_TANGENTS)?"Update Tangents":"", (nDeformFlags&FLAG_DEFORM_UPDATE_NORMALS)?"Update Normals":"", (nDeformFlags&FLAG_DEFORM_FORCE_UPDATE)?"Force Update":"", lb);
		return;
	}

	int nVertexFormat = pRenderVertexBuffer->m_vertexformat;
	int nNormalsOffset = g_VertFormatNormalOffsets[nVertexFormat];

	if (nNormalsOffset >= 0)
		nDeformFlags |= FLAG_DEFORM_UPDATE_NORMALS;

#ifdef _DEBUG
	static bool bContinue[3] = {true,true,true};
	if (!bContinue[nLodToDeform])
		return;
#endif

	if(!lb)
		return;
	// if we write directly into video memory - wait for fences first
	{
		if (!lb->m_pVertexBuffer)
		{
			// Strange: we are to copy to the videobuffer, but there's no videobuffer.
			assert(0);
			return;
		}
	}

	bool bRealizeDecals = false;

	// we only currently support decals for LOD 0
	if (NeedRealizeDecals())
	{
		if (nLodToDeform == 0)
			bRealizeDecals = true;
		else
			DiscardDecalRequests();
	}

	int nFrameId = g_GetIRenderer()->GetFrameID();

	// get vertices of selected lod
	CryGeometryInfo * pGeomInfo = m_pMesh->getGeometryInfo(nLodToDeform);

	const CryUV* pExtUVs = pGeomInfo->getExtUVs();

	TangData * pExtTangents = pGeomInfo->getExtTangents();
	unsigned numExtTangents = pGeomInfo->numExtTangents();

	Vec3d vNorm, vTang, vBinorm;
	CrySkinRigidBasis* pTangSkin = NULL;

	bool bSpawnParticles = !m_pParent->m_ParticleManager.empty() && m_nLastSkinnedFrameID[nLodToDeform] < nFrameId;

	// the tangents will be skinned here; if it remains NULL, they won't be skinned at all
	SPipTangentsA* pTangentBases = NULL;

	// these are the sizes of buffers needed in the temporary buffer, in bytes
	unsigned
		sizeVertices = pGeomInfo->numUsedVertices() * sizeof(Vec3dA16),
		sizeNormals  = (nDeformFlags&FLAG_DEFORM_UPDATE_NORMALS)?sizeVertices:0,
		sizeTangents = 0;

	if ((nDeformFlags&FLAG_DEFORM_UPDATE_TANGENTS) && m_pMesh->numBoneInfos())
	{
		if (
			(nDeformFlags&FLAG_DEFORM_FORCE_UPDATE) || // we calculate tangents if the videobuffer has changed and we're forced to recalculate all buffers
			!m_nLastTangentsUpdatedFrameId ||             // we calculate tangents if they haven't been calculated yet
			m_nLastTangentsUpdatedLOD != nLodToDeform ||
			(nFrameId >= m_nLastTangentsUpdatedFrameId + g_GetCVars()->ca_TangentBasisCalcPeriod()))
		{
			// we need to recalculate and update the tangents
			pTangSkin = pGeomInfo->getTangSkin ();
			// we need to update (possibly without recalculation) the tangent bases
			// we won't need the tangent bases and vertices simultaneously
			sizeTangents  = pTangSkin->size()*sizeof(SPipTangentsA);
		}
		else
			nDeformFlags &= ~FLAG_DEFORM_UPDATE_TANGENTS;
	}
	// we won't need tangents simultaneously with the vertices/normals
	g_Temp.reserve (max(sizeTangents, sizeVertices+sizeNormals));

	const unsigned* pExtToIntMap = pGeomInfo->getExtToIntMapEntries();


	//----------------------------------------------------------------
	//----------------------------------------------------------------
	//----------------------------------------------------------------

	Vec3dA16* pNormals = NULL;
	if ((nDeformFlags&FLAG_DEFORM_UPDATE_NORMALS))
		pNormals = SelfNormalSkin (nLodToDeform, (((Vec3dA16*)g_Temp.data())+pGeomInfo->numUsedVertices()));

	// most probably we'll skin here; sometimes we don't skin and just change the pointer tothe original geometry data
	// so, get the pointer to the skin. SelfSkin can also modify normals according to morph targets
	const Vec3d* pVertices = NULL;

	if ((nDeformFlags&FLAG_DEFORM_UPDATE_VERTICES) || bRealizeDecals || bSpawnParticles)
		pVertices = SelfSkin(nLodToDeform, (Vec3d*)g_Temp.data(), pNormals);

	if (bRealizeDecals)
		RealizeDecals (pVertices);

	if (bSpawnParticles)
	{
		CryCharParticleManager::SpawnParams params;
		params.setVertices (pVertices, pGeomInfo->numUsedVertices());
		params.setFaces (pGeomInfo->getFaces(), (unsigned)pGeomInfo->numFaces());
		params.pModelMatrix = &m_pParent->m_ModelMatrix44;
		params.pNormalsA16 = pNormals;
		if (params.numBoneMatrices = m_pMesh->numBoneInfos())
			params.pBoneGlobalMatrices = m_pParent->getBoneGlobalMatrices();

		//spawn particles now, if they don't need normals, because in this case
		// the vertices will be destroyed by the normals
		m_pParent->m_ParticleManager.spawn (params);
	}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

	Vec3d* pVideobuffer = (Vec3d*)pRenderVertexBuffer->m_VS[VSF_GENERAL].m_VData;
	
	unsigned numVerts = (unsigned)lb->m_SecVertCount;
	assert (numVerts == pGeomInfo->numExtToIntMapEntries());
	if ((nDeformFlags&FLAG_DEFORM_UPDATE_VERTICES))
	{
		DEFINE_PROFILER_SECTION("ModelDeformVideoMemCopy1");
		//CopyPosUVsToVideomemory (pVertices, pExtUVs, numVerts, pExtToIntMap, pVideobuffer, nVertexFormat);

		char* pVertBuf = pGeomInfo->getVertBuf(nVertexFormat);
		int nVertexFormatSize = m_VertexSize[nVertexFormat];


//----------------------------------------------------------

{
		DEFINE_PROFILER_SECTION("Reconstruct-VertexBuffer-PUV");
		
		u32 max=0;
		if (!pNormals || nNormalsOffset < 0)
		{
			for (unsigned i = 0; i < numVerts; ++i){
				u32 index=pExtToIntMap[i];	
				//_mm_prefetch( (char*)&pVertices[index], _MM_HINT_NTA );
				*(Vec3d*)(pVertBuf + i * nVertexFormatSize) = pVertices[index];
				if (max<index) max=index;
			}

		} 	else {
						for (unsigned i = 0; i < numVerts; ++i)
						{
								*(Vec3d*)(pVertBuf + i * nVertexFormatSize) = pVertices[pExtToIntMap[i]];
								*(Vec3d*)(pVertBuf + i * nVertexFormatSize + nNormalsOffset) = pNormals[pExtToIntMap[i]].v;
						}
		}

}

//----------------------------------------------------------

			g_GetIRenderer()->UpdateBuffer (pRenderVertexBuffer, pVertBuf, numVerts, true, 0, VSF_GENERAL);

	}





static int CTang=0;
CTang=(CTang+1) & 0x00;


if((nDeformFlags&FLAG_DEFORM_UPDATE_TANGENTS)/* && pTangents*/)
{

	if (CTang==0) 
	{

		unsigned numTangents;
		SPipTangents* pSrc;
		if(pTangSkin && g_GetCVars()->ca_EnableTangentSkinning())
		{
			// this is the number of bases and the bases themselves, as
			// they are to be copied to the videomemory (directly)

			pTangentBases = (SPipTangentsA*)g_Temp.data(); // use the same mem for the tangents
			assert(m_pMesh->numBoneInfos());
			numTangents = pTangSkin->size();
			#if defined(_CPU_X86) && !defined(LINUX)
				if (g_GetCVars()->ca_SSEEnable() && cpu::hasSSE())
					pTangSkin->skinSSE (m_pParent->getBoneGlobalMatrices(), pTangentBases);
				else
			#endif
				pTangSkin->skin (m_pParent->getBoneGlobalMatrices(), pTangentBases);

			#if 0 && defined(_DEBUG)
				for (unsigned nTang = 0; nTang < numTangents; ++nTang)
				{
					SPipTangentsA& t = pTangentBases[nTang];
					assert (t.m_Binormal.Length2() >0.4);
					assert (t.m_Tangent.Length2() >0.4);
					assert (t.m_TNormal.Length2() >0.4);
				}
			#endif


			#if sizeofSPipTangentsA == 0x30
				packVec3d16 (pTangentBases, pTangSkin->size()*3);
			#endif

			// we've calculated the tangents that are exactly in the same format as in videomem
			// (packed 3-float vectors) so we may copy them directly
			pSrc = (SPipTangents*)pTangentBases;
			assert (pTangSkin->size() == pGeomInfo->numExtToIntMapEntries());
		}
		else
		{
			numTangents = numExtTangents;
			pSrc = (SPipTangents*)pExtTangents;
		}



		// copy it from the calculated array
		{
			DEFINE_PROFILER_SECTION ("ModelDeformVideoMemCopy2");
			g_GetIRenderer()->UpdateBuffer (pRenderVertexBuffer, pSrc, numTangents, true, 0, VSF_TANGENTS);
		}
	}



		//memcpy (pTangents, pSrc, sizeof(SPipTangents) * numTangents );
		m_nLastTangentsUpdatedFrameId = nFrameId;
		m_nLastTangentsUpdatedLOD     = nLodToDeform;
 }

}



// returns true if the decals need realization (otherwise RealizeDecals() might not be called)
bool CryModelSubmesh::NeedRealizeDecals ()
{
	return m_pDecalManager && m_pDecalManager->NeedRealize();
}

// the modelstate gives itself a chance to process decals (realize)
// in this call. The decal realization means creation of geometry that carries the decal
void CryModelSubmesh::RealizeDecals (const Vec3d* pPositions)
{
	if (m_pDecalManager)
		m_pDecalManager->Realize (pPositions);
}



// discards all outstanding decal requests - the decals that have not been meshed (realized) yet
// and have no chance to be correctly meshed in the future
void CryModelSubmesh::DiscardDecalRequests()
{
	if (m_pDecalManager)
		m_pDecalManager->DiscardRequests();
}


// do not calculate normals and do not copy into video buffers
// Returns: number of vertices
const Vec3d* CryModelSubmesh::DeformForShadowVolume( int nLodToDeform )
{
	return SelfSkin(nLodToDeform);
}

// skins this model (into the given temporary storage, if needed)
// if no storage is provided, then allocates its own storage (g_Temp.data())
// can return the model pre-skinned data (i.e. the returned value is not guaranteed to 
// be the same as the buffer passed
// The normals, if not NULL, are modified according to the vertex movements
const Vec3d* CryModelSubmesh::SelfSkin(int nLOD, Vec3d*pVertices, Vec3dA16* pNormalsA16)
{
	// if we have links to bones, use them to skin the model
	CryGeometryInfo * pGeomInfo = m_pMesh->getGeometryInfo(nLOD);
	if (m_pMesh->numBoneInfos())
	{
		unsigned numVertices = pGeomInfo->numUsedVertices();
#if ( defined(_CPU_X86) || defined (_AMD64_) ) && !defined(LINUX)
		if (g_GetCVars()->ca_SSEEnable() && cpu::hasSSE())
		{
			if (!pVertices)
			{
				g_Temp.reserve (sizeof(Vec3dA16)*numVertices);
				pVertices = (Vec3d*)g_Temp.data();
			}
			CrySkinFull* pSkin = pGeomInfo->getGeomSkin();
			pSkin->skinSSE (m_pParent->getBoneGlobalMatrices(), (Vec3dA16*)pVertices);
			packVec3d16 (pVertices, numVertices);

			CryAABB caabb;
			caabb.vMin=pSkin->g_BBox.vMin.v;
			caabb.vMax=pSkin->g_BBox.vMax.v;
			m_SubBBox = caabb;
			m_nLastSkinBBoxUpdateFrameId = g_nFrameID;
		}
		else
#endif
		{
			if (!pVertices)
			{
				g_Temp.reserve (sizeof(Vec3d)*numVertices);
				pVertices = (Vec3d*)g_Temp.data();
			}
			pGeomInfo->getGeomSkin()->skin (m_pParent->getBoneGlobalMatrices(), pVertices);
		}

		if (nLOD == 0 && NeedMorph() && !g_GetCVars()->ca_NoMorph())
		{
			// morph the object into the given vertex array to use subsequently in skinning
			MorphWithSkin (pVertices, pNormalsA16);
		}
		return pVertices;
	}
	else
	{
		return pGeomInfo->getVertices();
	}
}


// Normal-skins this model (into the given storage)
Vec3dA16* CryModelSubmesh::SelfNormalSkin (int nLOD, Vec3dA16* pNormals)
{
	// if we have links to bones, use them to skin the model
	CryGeometryInfo * pGeomInfo = m_pMesh->getGeometryInfo(nLOD);
	if (m_pMesh->numBoneInfos())
	{
		CrySkinFull* pNormalSkin = pGeomInfo->getNormalSkin();
		if (!pNormalSkin->empty())
		{
			//reserve for the normals in the internal indexation and for the normals in outer indexation
			pNormalSkin->skinAsVec3d16 (m_pParent->getBoneGlobalMatrices(), pNormals);
			return pNormals;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////
//this is called only at the beginning once for each LOD
void CryModelSubmesh::DeformFirst()
{
	// get current LOD geometry
	int nLod = m_pParent->m_nLodLevel;
	CryGeometryInfo * pGeomInfo = m_pMesh->getGeometryInfo(nLod);
	unsigned numVertices = pGeomInfo->numVertices();
	unsigned i;

	// deform positions into pGeomInfo->m_pVertices
	for(i = 0; i < numVertices; i++)
	{
		Vec3d & p = pGeomInfo->getVertex(i);

		if (pGeomInfo->numLinks())
		{
			p.x = p.y = p.z = 0;
			const CryVertexBinding& arrLinks = pGeomInfo->getLink(i);

			for (CryVertexBinding::const_iterator itLink = arrLinks.begin(); itLink != arrLinks.end(); ++itLink)
			{
				Matrix44& matBoneGlobal = m_pParent->getBoneMatrixGlobal(itLink->BoneID);      
				p += matBoneGlobal.TransformPointOLD(itLink->offset) * itLink->Blending;
			}
		}
	}

	// calc nomals for default pose for this LOD into pGeomInfo->m_pVertices
	pGeomInfo->recalculateNormals();

	// at this point in pGeomInfo there are positions and normals for zero frame

	if(!m_pLeafBuffers[nLod])
		return;

	// now put pos and normals into leaf secondary buffer
	CLeafBuffer * lb = m_pLeafBuffers[nLod];
	const unsigned* pExtToIntMap = pGeomInfo->getExtToIntMapEntries();
	int StrPos;
	byte *pPos = lb->GetPosPtr(StrPos, 0, true);
	int StrNor;
	byte *pNor = lb->GetNormalPtr(StrNor, 0, true);
	for (i=0; i<(unsigned)lb->m_SecVertCount; i++, pPos+=StrPos, pNor+=StrNor)
	{
		unsigned sn = pExtToIntMap[i];    
		assert(sn<pGeomInfo->numVertices());
		if (pPos)
		{
			Vec3d *v = (Vec3d *)pPos;
			v->x = pGeomInfo->getVertex(sn).x;
			v->y = pGeomInfo->getVertex(sn).y;
			v->z = pGeomInfo->getVertex(sn).z;
		}
		assert ( GetLengthSquared(pGeomInfo->getNormal(sn)) > 0.4);
		if (pNor)
		{
			Vec3d *v = (Vec3d *)pNor;
			v->x = pGeomInfo->getNormal(sn).x;
			v->y = pGeomInfo->getNormal(sn).y;
			v->z = pGeomInfo->getNormal(sn).z;
		}
	}

	// calculate tangent basises
	lb->CreateTangBuffer();

	if((unsigned)lb->m_SecVertCount > pGeomInfo->numExtTangents())
		g_GetConsole()->Exit("Error: CryModelState::DeformFirst: lb->m_SecVertCount > pGeomInfo->m_nAllocatedTangNum (%d)", lb->m_SecVertCount);

	// copy tangent basises into pGeom
	//pSecBuff = (PipVertex *)lb->m_pSecVertBuffer->m_data;
	SPipTangents *pTangBuff = (SPipTangents *)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
	TangData* pExtTangents = pGeomInfo->getExtTangents();
	for (i = 0; i < (unsigned)lb->m_SecVertCount; ++i)
	{
		assert(i<pGeomInfo->numExtTangents());

		// For Debug:
		// This is the current vertex:
		// (((*pGeomInfo).m_arrVertices).m_pData)[((*pGeomInfo).m_arrExtToIntMap).m_pData[i]]
		// This is its normal (may be not from the initial position):
		// (((*pGeomInfo).m_arrNormals).m_pData)[((*pGeomInfo).m_arrExtToIntMap).m_pData[i]]
		// 
		/*
		assert (pTangBuff[i].m_Binormal.Length2() > 0.4);
		assert (pTangBuff[i].m_Tangent.Length2() > 0.4);
		assert (pTangBuff[i].m_TNormal.Length2() > 0.4);
		*/

		pExtTangents[i].binormal = pTangBuff[i].m_Binormal;
		pExtTangents[i].tangent  = pTangBuff[i].m_Tangent;    
		pExtTangents[i].tnormal  = pTangBuff[i].m_TNormal;    
	}
	//lb->FreeSystemBuffer();
}

//////////////////////////////////////////////////////////////////////
//Calculate shadow volumes,fill buffers and render shadow volumes into the stencil buffer
//TODO: Optimize everything
void CryModelSubmesh::RenderShadowVolumes (const SRendParams *rParams, int nLimitLOD)
{
	DEFINE_PROFILER_FUNCTION();

#ifdef _DEBUG	
	if (g_GetCVars()->ca_DebugRebuildShadowVolumes())
		m_pMesh->buildStencilShadowInfos();
#endif

	// all of the operations here (except when another PROFILE_FRAME_* is used) are related to deformations, extrusions etc.
	// for the shadow volume

	int nShadowLOD = max (m_pParent->m_nLodLevel, min2((int)m_pMesh->numLODs()-1, max2(g_GetCVars()->ca_LimitShadowLOD(), nLimitLOD)));

	// detect the shadow edges
	IStencilShadowConnectivity *pConnectivity = m_pMesh->getStencilShadowConnectivity(nShadowLOD);
	DWORD dwVertexCount, dwTriangleCount;
	pConnectivity->GetStats(dwVertexCount, dwTriangleCount);
	if (dwVertexCount == 0 || dwTriangleCount == 0)
		return; // empty shadow volume, don't do anything

	IEdgeDetector *iEdgeDetector = Get3DEngine()->GetEdgeDetector();
	CryCharReShadowVolume* pReShadowVolume = m_ReShadowManager.newShadow();
	if (!pReShadowVolume)
		return;

	unsigned numVertices = m_pMesh->getGeometryInfo(nShadowLOD)->numUsedVertices();
	// compute the deformed character vertices; the character will keep the allocated buffer itself (it won't be allocated)
	const Vec3d* pModelVerts = DeformForShadowVolume(nShadowLOD);

//	Vec3d vLightPos = rParams->lSource->m_Origin;
	Vec3d vLightPos = rParams->pShadowVolumeLightSource->m_Origin;

	float fShadowVolumeExtent = g_GetCVars()->ca_ShadowVolumeExtent();
	if (fShadowVolumeExtent == 0)
	{
		if(rParams->fShadowVolumeExtent)
			fShadowVolumeExtent = rParams->fShadowVolumeExtent;
		else
			fShadowVolumeExtent = 20.0f; // the default extent
	}


	//translate the light pos with respect to the character
	Vec3d vLightTrans = vLightPos - rParams->vPos; 
	//rotate the light pos to compensate the rotation of the shadow volume
	//these operations are done to avoid to translate and rotate all shadow volume edges

	float fCos = cry_cosf(DEG2RAD(-rParams->vAngles.z));
	float fSin = cry_sinf(DEG2RAD(-rParams->vAngles.z));
	Vec3d vLSourcePos (
		vLightTrans.x*fCos-vLightTrans.y*fSin,
		vLightTrans.x*fSin+vLightTrans.y*fCos,
		vLightTrans.z);

	if(!pConnectivity)return;		// nothing more to do

	// deformed model vertices
	iEdgeDetector->BuildSilhuetteFromPos (pConnectivity, vLSourcePos, pModelVerts);

	unsigned numShadowIndices = iEdgeDetector->numShadowVolumeIndices();
	unsigned numShadowVertices = iEdgeDetector->numShadowVolumeVertices();

	if (numShadowVertices > 1 && numShadowIndices > 1)
	{
		{
			DEFINE_PROFILER_SECTION("ModelShadowVideoAlloc");
			pReShadowVolume->prepare (numShadowIndices, numShadowVertices);
		}

		iEdgeDetector->meshShadowVolume (vLSourcePos, fShadowVolumeExtent, pReShadowVolume->getVertexBuffer(), pReShadowVolume->getIndexBuffer());

		{
			DEFINE_PROFILER_SECTION("ModelShadowVideoUpdate");
			pReShadowVolume->submit (rParams, m_pParent->m_pShaderStateShadowCull);
		}
	}
	else
		g_GetLog () -> Log ("WARNING: shadow volume with %d indices and %d vertices generated", numShadowIndices, numShadowVertices);
}


//////////////////////////////////////////////////////////////////////////
// Compute all effectors and remove those whose life time has ended,
// then apply effectors to the bones
void CryModelSubmesh::ProcessSkinning(const Vec3& t, const Matrix44& mtxModel, int nTemplate, int nLod, bool bForceUpdate)
{
	if (nLod < 0)
		nLod = m_pParent->m_nLodLevel;
	m_pParent->m_ModelMatrix44 = mtxModel;

	bool bNeedVertices = true;

	CLeafBuffer *lb = m_pLeafBuffers[nLod];

	SShaderItem si;
	for (int i = 0; i < lb->m_pMats->Count(); ++i)
	{
		si = lb->m_pMats->Get(i)->shaderItem;
		if (si.m_pShader)
			break;
	}
	IShader *ef;
	assert (si.m_pShader);
	ef = si.m_pShader->GetTemplate(nTemplate);
	assert (ef);

#ifdef _DEBUG
	bool bAllowToCopyIntoVideoBufferDirectly = ((ef->GetFlags3() & EF3_NEEDSYSBUF) == 0);
#endif

	bool bShowNormals  = g_GetCVars()->r_ShowNormals() != 0;
	bool bShowTangents = g_GetCVars()->r_ShowTangents() != 0;

	bool bNeedTangents = ef && (ef->GetFlags() & EF_NEEDTANGENTS);
	bool bNeedNormals = ef && (ef->GetFlags() & EF_NEEDNORMALS);

	int nCurrentFrameID = g_GetIRenderer()->GetFrameID();
#ifdef UNIQUE_VERT_BUFF_PER_INSTANCE
	// check if already skinned in this frame

	bNeedVertices = bNeedVertices && m_nLastSkinnedFrameID[nLod] != nCurrentFrameID;
	bNeedTangents = bNeedTangents && m_nLastTangedFrameID[nLod]  != nCurrentFrameID;

	//const unsigned nInternalForceUpdatePow = 6;

	if(!bNeedVertices && !bForceUpdate && !bNeedTangents &&
		(m_pParent->m_uFlags&(CryModelState::nFlagNeedReskinLOD<<nLod)) == 0 &&
		m_pLeafBuffers[nLod] && 
		m_pLeafBuffers[nLod]->m_pVertexBuffer 
		// force update every 2^nInternalForceUpdatePow frames: this comparison
		// produces false and doesn't let this conditional return
		//&&(!g_GetCVars()->ca_SafeReskin() /*|| ((g_GetIRenderer()->GetFrameID() & nInternalForceUpdatePow) != (m_nInstanceNumber & nInternalForceUpdatePow))*/)
		)
	{
		return;
	}
#endif


	CryAABB bbox = m_pParent->getBBox();
	//Vec3 trans=mtxModel.GetTranslationOLD();
	//Matrix44 m44 = m_pParent->getBoneMatrixGlobal(0);
 //	Vec3 trans=m_pParent->m_vOffset;


	AABB aabb;
	aabb.min=bbox.vMin+t;
	aabb.max=bbox.vMax+t;

	Vec3 WorldMiddle=(aabb.min+aabb.max)*0.5f;
	bool NearCam=0;
	f32 dist = GetLength( GetViewCamera().GetPos()-WorldMiddle );
	if (dist<2.0f) NearCam=1;



	Matrix34 m34=Matrix34::CreateIdentity();
	m34 = Matrix34( GetTransposed44(mtxModel) );
	m34.SetTranslation(t);

/*{
	//debug - this draws the bounding box around the character
	static float fColorBBox[4] = {1,1,1,1};
	fColorBBox[1]+=0.01f; 	if (fColorBBox[1]>1.0f) fColorBBox[1]=fColorBBox[1]-1.0f;
	debugDrawBBox (m34, CryAABB(bbox.vMin,bbox.vMax), 2, fColorBBox);
// 	g_pIRenderer->Draw3dBBox(aabb.min,aabb.max);
	float fColor[4] = {0,1,0,1};
	extern float g_YLine;
//	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"trans: %15.10f %15.10f %15.10f  bForceUpdate:%d",t.x,t.y,t.z, bForceUpdate );	g_YLine+=16.0f;
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"aabbsize: %15.10f %15.10f %15.10f",(aabb.max.x-aabb.min.x),(aabb.max.y-aabb.min.y),(aabb.max.z-aabb.min.z) );	g_YLine+=16.0f;
	//g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"trans: %15.10f %15.10f %15.10f  bForceUpdate:%d",trans.x,trans.y,trans.z,bForceUpdate );	g_YLine+=16.0f;
	//g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"min: %15.10f %15.10f %15.10f",aabb.min.x,aabb.min.y,aabb.min.z );	g_YLine+=16.0f;
	//g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"max: %15.10f %15.10f %15.10f",aabb.max.x,aabb.max.y,aabb.max.z );	g_YLine+=16.0f;
	//g_YLine+=16.0f;
}*/


//if ( !(GetAsyncKeyState('I') & 0x8000) )

//	if ( GetViewCamera().IsAABBVisibleFast(aabb) || bForceUpdate || NearCam ) 
	{

		// Deform current LOD
		if (  !g_GetCVars()->ca_NoDeform() || bForceUpdate)
		{
			unsigned nDeformFlags = 0;

			if (bNeedTangents||bShowTangents)
				nDeformFlags |= FLAG_DEFORM_UPDATE_TANGENTS;
			if (bNeedNormals||bShowNormals)
				nDeformFlags |= FLAG_DEFORM_UPDATE_NORMALS;
			if (bNeedVertices)
				nDeformFlags |= FLAG_DEFORM_UPDATE_VERTICES;
			if (bForceUpdate)
				nDeformFlags |= FLAG_DEFORM_FORCE_UPDATE;

			Deform(nLod, nDeformFlags);

			m_pParent->m_uFlags &= ~(CryModelState::nFlagNeedReskinLOD<<nLod);
			m_nLastUpdatedLodLevel = nLod;

			if (bNeedVertices)
				m_nLastSkinnedFrameID[nLod] = nCurrentFrameID;
			if (bNeedTangents)
				m_nLastTangedFrameID[nLod] = nCurrentFrameID;
			m_nLastUpdatedLodLevel = nLod;
		}

	}


}





bool CryModelSubmesh::SetShaderTemplateName (const char *TemplName, int Id, const char *ShaderName,IMatInfo *pCustomMaterial,unsigned nFlags)
{

	bool bRes = false;
	for (unsigned nLOD = 0; nLOD < m_pMesh->numLODs(); ++nLOD)
	{
		CLeafBuffer *lb = m_pLeafBuffers[nLOD];
		if (lb->m_pMats->Count() > (int)getShaderTemplates(Id).size())
			return false;
		int nTempl = -1;
		if (!ShaderName)
		{
			int numMaterials = lb->m_pMats->Count();
			assert (numMaterials >= 0);
			for (unsigned i=0; i < (unsigned)numMaterials; i++)
			{
				//assert (m_pModelState->m_pMesh == m_pCryCharBody->GetModel());
				SShaderItem si = lb->m_pMats->Get(i)->shaderItem;
				// Override object material.
				if (pCustomMaterial)
				{
					pCustomMaterial->OverrideShaderItem( i,si );
				}
		    
				if (si.m_pShader && (*lb->m_pMats)[i].pRE && (*lb->m_pMats)[i].nNumIndices)
				{
					if (TemplName && TemplName[0])
					{
						si.m_pShader->AddTemplate(si.m_pShaderResources, nTempl, TemplName, false);
					}
					getShaderTemplates(Id)[i] = nTempl;
				}
			}
		}
		else
		for (unsigned i=0; i < getShaderTemplates(Id).size(); ++i)
		{
			//assert (m_pMesh == m_pModel );
			SShaderItem si = lb->m_pMats->Get(i)->shaderItem;

			if (!si.m_pShader)
				continue;

			if (!stricmp(ShaderName, si.m_pShader->GetName()))
			{
				bRes = true;
				if (TemplName && TemplName[0])
					si.m_pShader->AddTemplate(si.m_pShaderResources, nTempl, TemplName);
				getShaderTemplates(Id)[i] = nTempl;
			}
		}
	}



	return bRes;
}

CLeafBuffer* CryModelSubmesh::GetLeafBuffer ()
{
	return m_pLeafBuffers[m_pParent->m_nLodLevel];
}


void CryModelSubmesh::SetShaderFloat (const char *Name, float Val, const char *ShaderName)
{

  char name[128];
  int i;

  strcpy(name, Name);
  strlwr(name);

  int nMat = -1;
  if (ShaderName)
  {
		unsigned nLOD, numLODs = m_pMesh->numLODs();
		for (nLOD = 0; nLOD < numLODs; ++nLOD)
		{
			CLeafBuffer *lb = m_pLeafBuffers[nLOD];
			int numMaterials = lb->m_pMats->Count();
			assert (numMaterials >= 0);
			for (i=0; i<numMaterials; i++)
			{
				CMatInfo *mi = lb->m_pMats->Get(i);
				if (!mi->pRE)
					continue;
  			SShaderItem si = mi->shaderItem;
				if (!stricmp(si.m_pShader->GetName(), ShaderName))
				{
					nMat = i;
					break;
				}
			}
		}
  }

	unsigned j;
	for (j = 0; j < m_ShaderParams.size(); ++j)
  {
    if (!strcmp(name, m_ShaderParams[j].m_Name) && m_ShaderParams[j].m_nMaterial == nMat)
      break;
  }

  if (j == m_ShaderParams.size())
  {
    SShaderParam pr;
    strncpy(pr.m_Name, name, 32);
    pr.m_nMaterial = nMat;
    m_ShaderParams.push_back(pr);
		//g_GetLog()->LogToFile("SetShaderFloat realloc: %d", m_ShaderParams.size());
  }
  m_ShaderParams[j].m_Type = eType_FLOAT;
  m_ShaderParams[j].m_Value.m_Float = Val;
}


void CryModelSubmesh::Render(const struct SRendParams & RendParams, Matrix44& mtxObjMatrix, CryCharInstanceRenderParams& rCharParams, const Vec3& translation)
{
	
	if ( translation.GetLength() == 0.0f )
	{
		int i=0;
	}

/*
	Vec3 trans=RendParams.vPos;
	float fColor[4] = {0,1,0,1};
	extern float g_YLine;
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"render trans: %15.10f %15.10f %15.10f",trans.x,trans.y,trans.z );	g_YLine+=16.0f;
	g_YLine+=16.0f;
*/

	if (0 == (m_nFlags&FLAG_VISIBLE))
		return;

	CCObject * pObj = g_GetIRenderer()->EF_GetObject(true);

	//pObj->m_Translation=translation;

	pObj->m_SortId = RendParams.fCustomSortOffset;

	pObj->m_ObjFlags |= FOB_TRANS_MASK;

  // set scissor
 // pObj->m_nScissorX1 = RendParams.nScissorX1;
 // pObj->m_nScissorY1 = RendParams.nScissorY1;
 // pObj->m_nScissorX2 = RendParams.nScissorX2;
 // pObj->m_nScissorY2 = RendParams.nScissorY2;

	//checl if it should be drawn close to the player
	if ((RendParams.dwFObjFlags & FOB_NEAREST) || (rCharParams.m_nFlags & CS_FLAG_DRAW_NEAR) )	{ 	pObj->m_ObjFlags|=FOB_NEAREST;	}
	else pObj->m_ObjFlags&=~FOB_NEAREST;

  pObj->m_Color = rCharParams.m_Color;
	pObj->m_Color.a *= RendParams.fAlpha;

	//some indoor flags
	//if   (RendParams.dwFlags & RPF_ZPASS)	pObj->m_ObjFlags |= FOB_ZPASS;
	//else { if (RendParams.dwFlags & RPF_LIGHTPASS)  pObj->m_ObjFlags |= FOB_LIGHTPASS; }

	//if (RendParams.dwFObjFlags & RPF_INDOOR) pObj->m_ObjFlags |= FOB_INDOOR;

	pObj->m_AmbColor = RendParams.vAmbientColor;

	if (g_GetIRenderer()->EF_GetHeatVision())	pObj->m_ObjFlags |= FOB_HOTAMBIENT;

	pObj->m_ObjFlags |= RendParams.dwFObjFlags;

	pObj->m_Matrix = mtxObjMatrix;

	int nTemplID = RendParams.nShaderTemplate;

	pObj->m_pShadowCasters = RendParams.pShadowMapCasters;

	if (!m_ShaderParams.empty())	pObj->m_ShaderParams = &m_ShaderParams;

	if(RendParams.pShadowMapCasters && RendParams.pShadowMapCasters->size()) pObj->m_ObjFlags |= FOB_INSHADOW;
	else pObj->m_ObjFlags &= ~FOB_INSHADOW;

	pObj->m_pCharInstance = this;

	pObj->m_nTemplId = nTemplID;

	pObj->m_DynLMMask = RendParams.nDLightMask;

	if (g_GetCVars()->ca_ambient_light_range() > 0.05f && !(RendParams.dwFObjFlags & FOB_FOGPASS) && !(RendParams.dwFObjFlags & FOB_LIGHTPASS))
	{
		float col = (1.0f - RendParams.fDistance/g_GetCVars()->ca_ambient_light_range())*g_GetCVars()->ca_ambient_light_intensity();

		if (col >= 0.0f)
		{
			Vec3 ambientColor(pObj->m_AmbColor.x, pObj->m_AmbColor.y, pObj->m_AmbColor.z);
			if (ambientColor.GetLengthSquared()>0.001)
			{
				rCharParams.m_ambientLight.m_Color    = CFColor(ambientColor * col);

//				pObj->m_AmbColor -= Vec3d(rCharParams.m_ambientLight.m_Color.r*2.0f, rCharParams.m_ambientLight.m_Color.g*2.0f, rCharParams.m_ambientLight.m_Color.b*2.0f);

				rCharParams.m_ambientLight.m_Origin			= Vec3d(0, 0, 5000.0f);
				rCharParams.m_ambientLight.m_fRadius		= 100000000;
				rCharParams.m_ambientLight.m_Color			= Vec3(col, col, col);
				rCharParams.m_ambientLight.m_SpecColor	= Vec3(0.0f, 0.0f, 0.0f);
				rCharParams.m_ambientLight.m_Flags		 |= DLF_DIRECTIONAL | DLF_AMBIENT_LIGHT;
				g_GetIRenderer()->EF_ADDDlight (&rCharParams.m_ambientLight);

				if(rCharParams.m_ambientLight.m_Id>=0)
					pObj->m_DynLMMask |= (1<<rCharParams.m_ambientLight.m_Id);
			}
		}
	}

  CCObject * pObj1 = NULL;
  if (getShaderTemplates(1)[0] > 0)
  {
    pObj1 = g_GetIRenderer()->EF_GetObject(true);
    pObj1->CloneObject(pObj);
    pObj1->m_ObjFlags |= FOB_TRANS_MASK;
    pObj->m_SortId += 10;
  }

  // HACK HACK HACK
  // If we have less then 10 bones it's not the character
	if (m_pMesh->numBoneInfos() > 10)
    pObj->m_ObjFlags |= FOB_ENVLIGHTING;

	if (g_GetCVars()->ca_EnableLightUpdate() && this == m_pParent->GetSubmesh(0))
	{
		m_pParent->UpdateHeatSources (pObj, RendParams);
		m_pParent->UpdateDynBoundLights (pObj, RendParams);
	}

	if (!g_GetCVars()->ca_NoDraw())
		AddCurrentRenderData (pObj,pObj1,RendParams);

//	if (g_GetCVars()->ca_EnableDecals()  && !RendParams.bRenderIntoShadowMap)
	if (g_GetCVars()->ca_EnableDecals()  && !(RendParams.dwFObjFlags & FOB_RENDER_INTO_SHADOWMAP) )
		AddDecalRenderData (pObj,RendParams);


//---------------------------------------------------------------

/*
	CCObject * pObj = g_GetIRenderer()->EF_GetObject(true);

	pObj->m_ObjFlags |= FOB_TRANS_MASK;

	// set scissor
	pObj->m_nScissorX1 = RendParams.nScissorX1;
	pObj->m_nScissorY1 = RendParams.nScissorY1;
	pObj->m_nScissorX2 = RendParams.nScissorX2;
	pObj->m_nScissorY2 = RendParams.nScissorY2;

	//checl if it should be drawn close to the player
	if ((RendParams.dwFlags & RPF_DRAWNEAR) || (rCharParams.m_nFlags & CS_FLAG_DRAW_NEAR))	{		pObj->m_ObjFlags|=FOB_NEAREST;	}
	else	pObj->m_ObjFlags&=~FOB_NEAREST;

	pObj->m_Color = rCharParams.m_Color;
	pObj->m_Color.a *= RendParams.fAlpha;

	//some indoor flags
	if (RendParams.dwFlags & RPF_ZPASS)	pObj->m_ObjFlags |= FOB_ZPASS;
	else { if (RendParams.dwFlags & RPF_LIGHTPASS) pObj->m_ObjFlags |= FOB_LIGHTPASS; }

	if (RendParams.dwFlags & RPF_INDOOR) pObj->m_ObjFlags |= FOB_INDOOR;

	pObj->m_AmbColor = RendParams.vAmbientColor;

	if (g_GetIRenderer()->EF_GetHeatVision())	pObj->m_ObjFlags |= FOB_HOTAMBIENT;

	pObj->m_ObjFlags |= RendParams.dwFObjFlags;

	pObj->m_Matrix = mtxObjMatrix;

	int nTemplID = RendParams.nShaderTemplate;

	pObj->m_pShadowCasters = RendParams.pShadowMapCasters;

	if (!m_ShaderParams.empty()) pObj->m_ShaderParams = &m_ShaderParams;

	if(RendParams.pShadowMapCasters && RendParams.pShadowMapCasters->size())	pObj->m_ObjFlags |= FOB_INSHADOW;
	else	pObj->m_ObjFlags &= ~FOB_INSHADOW;

	pObj->m_pCharInstance = this;

	pObj->m_nTemplId = nTemplID;

	pObj->m_DynLMMask = RendParams.nDLightMask;

	if (g_GetCVars()->ca_ambient_light_range() > 0.05f &&	!(RendParams.dwFObjFlags & FOB_FOGPASS) && !(RendParams.dwFObjFlags & FOB_LIGHTPASS))
	{
		float col = (1.0f - RendParams.fDistance/g_GetCVars()->ca_ambient_light_range())*g_GetCVars()->ca_ambient_light_intensity();

		if (col >= 0.0f)
		{
			Vec3 ambientColor(pObj->m_AmbColor.x, pObj->m_AmbColor.y, pObj->m_AmbColor.z);
			if (ambientColor.GetLengthSquared()>0.001)
			{
				rCharParams.m_ambientLight.m_Color    = CFColor(ambientColor * col);

				pObj->m_AmbColor -= Vec3d(rCharParams.m_ambientLight.m_Color.r*2.0f, rCharParams.m_ambientLight.m_Color.g*2.0f, rCharParams.m_ambientLight.m_Color.b*2.0f);

				rCharParams.m_ambientLight.m_Origin			= Vec3d(0, 0, 5000.0f);
				rCharParams.m_ambientLight.m_fRadius		= 100000000;
				rCharParams.m_ambientLight.m_Color			= Vec3(col, col, col);
				rCharParams.m_ambientLight.m_SpecColor	= Vec3(0.0f, 0.0f, 0.0f);
				rCharParams.m_ambientLight.m_Flags     |= DLF_DIRECTIONAL | DLF_AMBIENT_LIGHT;
				g_GetIRenderer()->EF_ADDDlight (&rCharParams.m_ambientLight);

				if(rCharParams.m_ambientLight.m_Id>=0)	pObj->m_DynLMMask |= (1<<rCharParams.m_ambientLight.m_Id);
			}
		}
	}

	CCObject * pObj1 = NULL;
	if (getShaderTemplates(1)[0] > 0)
	{
		pObj1 = g_GetIRenderer()->EF_GetObject(true);
		pObj1->CloneObject(pObj);
    pObj1->m_ObjFlags |= FOB_TRANS_MASK;
		pObj->m_SortId += 10;
	}

	// HACK HACK HACK
	// If we have less then 10 bones it's not the character
	if (m_pMesh->numBoneInfos() > 10)
		pObj->m_ObjFlags |= FOB_ENVLIGHTING;

	if (g_GetCVars()->ca_EnableLightUpdate() && this == m_pParent->GetSubmesh(0))
	{
		m_pParent->UpdateHeatSources (pObj, RendParams);
		m_pParent->UpdateDynBoundLights (pObj, RendParams);
	}

	if (!g_GetCVars()->ca_NoDraw())
		AddCurrentRenderData (pObj,pObj1,RendParams);

	if (g_GetCVars()->ca_EnableDecals()  && !RendParams.bRenderIntoShadowMap)
		AddDecalRenderData (pObj,RendParams);
*/
}

void CryModelSubmesh::UpdateMorphEffectors (float fDeltaTimeSec)
{
	unsigned numMorphTargets = (unsigned)m_arrMorphEffectors.size();
	unsigned nMorphTarget;
	for (nMorphTarget = 0; nMorphTarget < numMorphTargets; ++nMorphTarget)
	{
		CryModEffMorph& rMorph = m_arrMorphEffectors[nMorphTarget];
		rMorph.Tick(fDeltaTimeSec);
	}
	// clean up the array of morph targets from unused ones
	while (!m_arrMorphEffectors.empty() && !m_arrMorphEffectors.back().isActive())
		m_arrMorphEffectors.pop_back();
}

void CryModelSubmesh::RunMorph (int nMorphTargetId, const CryCharMorphParams&Params)
{
	// find the first free slot in the array of morph target, or create a new one and start morphing there
	unsigned nMorphEffector;
	for (nMorphEffector = 0; nMorphEffector < m_arrMorphEffectors.size(); ++nMorphEffector)
	{
		if (!m_arrMorphEffectors[nMorphEffector].isActive())
		{
			m_arrMorphEffectors[nMorphEffector].StartMorph (nMorphTargetId, Params);
			return;
		}
	}
	m_arrMorphEffectors.resize (m_arrMorphEffectors.size()+1);
	m_arrMorphEffectors.back().StartMorph (nMorphTargetId, Params);
}

bool CryModelSubmesh::RunMorph (const char* szMorphTarget,const CryCharMorphParams&Params,bool bShowNotFoundWarning)
{
	assert(IsValidString(szMorphTarget));

	int nMorphTargetId = m_pMesh->findMorphTarget(szMorphTarget);
	if (nMorphTargetId < 0)
	{
		if(g_GetCVars()->ca_AnimWarningLevel() > 0 && bShowNotFoundWarning)
			g_GetLog()->LogWarning ("\002CryModelSubmesh::RunMorph: Morph Target \"%s\" Not Found for character \"%s\"", szMorphTarget, m_pMesh->getFilePathCStr()); 
		return false;
	}
	else
	{
		RunMorph (nMorphTargetId, Params);
		return true;
	}
}

bool CryModelSubmesh::StopMorph (int nMorphTargetId)
{
	unsigned numStopped = 0;
	for (;;)
	{
		CryModEffMorph* pEffector = getMorphTarget(nMorphTargetId);
		if (!pEffector)
			break;
		pEffector->stop();
		++numStopped;
	}
	return numStopped > 0;
}

bool CryModelSubmesh::SetMorphTime (int nMorphTargetId, float fTime)
{
	CryModEffMorph* pEffector = getMorphTarget(nMorphTargetId);
	if (pEffector)
	{
		pEffector->setTime (fTime);
		return true;
	}
	return false;
}


bool CryModelSubmesh::SetMorphSpeed(int nMorphTargetId, float fSpeed)
{
	CryModEffMorph* pEffector = getMorphTarget(nMorphTargetId);
	if (pEffector)
	{
		pEffector->setSpeed (fSpeed);
		return true;
	}
	return false;
}

void CryModelSubmesh::StopAllMorphs()
{
	m_arrMorphEffectors.clear();
}

void CryModelSubmesh::FreezeAllMorphs()
{
	for (MorphEffectorArray::iterator it = m_arrMorphEffectors.begin(); it != m_arrMorphEffectors.end(); ++it)
		it->freeze();
}

//returns the morph target effector, or NULL if no such effector found
CryModEffMorph* CryModelSubmesh::getMorphTarget (int nMorphTargetId)
{
	unsigned nMorphEffector;
	for (nMorphEffector = 0; nMorphEffector < m_arrMorphEffectors.size(); ++nMorphEffector)
		if (m_arrMorphEffectors[nMorphEffector].getMorphTargetId() == nMorphTargetId)
			return &m_arrMorphEffectors[nMorphEffector];
	return NULL;
}

void CryModelSubmesh::AddDecal (CryEngineDecalInfo& Decal)
{
	if (!m_pDecalManager)
		m_pDecalManager = new CryCharDecalManager (m_pMesh->getGeometryInfo(0));

	m_pDecalManager->Add (Decal);
}


// returns true if the given submesh is visible
bool CryModelSubmesh::IsVisible()
{
	return (m_nFlags & FLAG_VISIBLE) != 0;
}

// depending on bVisible, either makes the submesh visible or invisible
void CryModelSubmesh::SetVisible (bool bVisible)
{
	if (bVisible)
		m_nFlags |= FLAG_VISIBLE;
	else
		m_nFlags &= ~FLAG_VISIBLE;
}

void CryModelSubmesh::ClearDecals()
{
	if (m_pDecalManager)
		m_pDecalManager->clear();
}

void CryModelSubmesh::PreloadResources(float fDistance, float fTime, int nFlags)
{
	for (unsigned i = 0; i < m_pMesh->numLODs(); ++i)
		if (m_pLeafBuffers[i])
			g_GetIRenderer()->EF_PrecacheResource(m_pLeafBuffers[i], fDistance, fTime, nFlags);
}
