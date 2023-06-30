/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Oscar Blasco
//	Taken over by Vladimir Kajalin, Andrey Honich
//  Taken over by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <I3DEngine.h>
#include <StlUtils.h>
#include <CryCompiledFile.h>

#include "CryHeaders.h"
#include "CryModel.h"
#include "CryModelState.h"
#include "FileMapping.h"
#include "ChunkFileReader.h"
#include "StringUtils.h"
#include "CryCharBody.h"
#include "CryGeomMorphTarget.h"
#ifdef PS2
#include "File.h"
#endif

#include <IEdgeConnectivityBuilder.h>									// IEdgeConnectivityBuilder
#include "ControllerManager.h"
#include "CVars.h"
#include "CrySkinMorph.h"
#include "CrySkinMorphBuilder.h"
#include "CrySkinRigidBasis.h"
#include "CrySkinBasisBuilder.h"
#include "CryModelSubmesh.h"
#include "CgfUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace CryStringUtils;
static int GetDefaultPhysMaterial()
{
	I3DEngine* pEngine = Get3DEngine();
	if (pEngine)
	{
		IPhysMaterialEnumerator *pMatEnum = pEngine->GetPhysMaterialEnumerator();
		if (pMatEnum)
			return pMatEnum->EnumPhysMaterial("mat_default");
	}
	return 0; // the default in case there's no physics yet
}

CryModel::CryModel (CryCharBody* pBody, CControllerManager* pControllerManager):
	CryModelAnimationContainer (pControllerManager),
	m_pBody(pBody),
	m_arrDamageTable ("CryModel.DamageTable"),
	m_arrGeomInfo("CryModel.GeomInfo"),
	m_vModelOffset (0,0,0),
	m_bDeleteLBMats (true),
	m_bFromCCG(false),
	m_nDefaultGameID (GetDefaultPhysMaterial())

{
  m_pDefaultModelState=0;
	// the material physical game id that will be used as default for this character
}

CryModel::~CryModel()
{
  if(m_pDefaultModelState)
	{ 
#ifndef UNIQUE_VERT_BUFF_PER_INSTANCE
		// the default model state is the only one where the leaf buffers must be deleted.
		// if the vertex buffers are unique per modelstate, then they're deleted by the modelstate destructor
		m_pDefaultModelState->DeleteLeafBuffers();
#endif
		delete m_pDefaultModelState;
		m_pDefaultModelState=0;
	}
}

//////////////////////////////////////////////////////////////////////////
// Performs computational tasks once after loading the model
//  - Initializes the bones
//  - Calculates the default pose
//  - Computes the static bounding box
void CryModel::LoadPostInitialize (bool bBoneInfoDefPoseNeedInitialization)
{
	//m_pControllerManager->OptimizeKeys(m_pSystem);
	AUTO_PROFILE_SECTION(g_dTimeGeomPostInit);
	if (getBoneInfos())
	{
		m_pDefaultModelState->InitBones(bBoneInfoDefPoseNeedInitialization);

		// Calculate normals for the first frame. TODO: move into DeformFirst
		if(m_pDefaultModelState->getRootBone())
		{
			for (unsigned nLodLevel = 0; nLodLevel < numLODs() && !getGeometryInfo (nLodLevel)->empty(); ++nLodLevel)
			{       
				m_pDefaultModelState->m_nLodLevel=nLodLevel;
				m_pDefaultModelState->GetCryModelSubmesh(0)->DeformFirst();
			}
		}
	}
	m_pDefaultModelState->m_nLodLevel=0;

	g_GetLog()->UpdateLoadingScreenPlus ("\003.");
	buildGeomSkins();
	g_GetLog()->UpdateLoadingScreenPlus ("\003.");
	buildMorphSkins ();
	g_GetLog()->UpdateLoadingScreenPlus ("\003.");

	if (g_GetCVars()->ca_PrebuildShadowConnectivity())
	{
		buildStencilShadowInfos();
		g_GetLog()->UpdateLoadingScreenPlus ("\003.");
	}

	computeBoneBBoxes();
	m_pDefaultModelState->InitBBox();

	// calculate the rough bounding sphere radius

	m_fStaticBSphereRadius = m_pDefaultModelState->getBBox().getSize().Length()*0.5f;
}

void CryModel::computeBoneBBoxes()
{
#if ENABLE_BONE_BBOXES
	if (numBoneInfos())
	{
		m_arrBoneBBoxes.reinit (numBoneInfos(), CryBBoxA16(Vec3d(0,0,0)));
		getGeometryInfo(0)->getGeomSkin()->computeBoneBBoxes(&m_arrBoneBBoxes[0]);
	}
#endif
}


// builds all stencil shadow connectivities
void CryModel::buildStencilShadowInfos()
{
	for (unsigned nLOD = 0; nLOD < numLODs(); ++nLOD)
	{
		CryGeometryInfo* pGeometry = getGeometryInfo(nLOD);
		pGeometry->buildStencilShadowConnectivity (m_arrMaterials);
	}
}


void CryModel::clearConstructionData()
{
	for (unsigned nLOD = 0; nLOD < numLODs(); ++nLOD)
	{
		CryGeometryInfo* pGeometry = getGeometryInfo(nLOD);
		pGeometry->clearConstructionData();
	}
}


// builds the skins for tangent base calculation for each LOD
// WARNING:
//   This also destructs the original external tangents etc.
void CryModel::buildGeomSkins()
{
	if (numBoneInfos())
	for (unsigned nLOD = 0; nLOD < numLODs(); ++nLOD)
	{
		CryGeometryInfo* pGeometry = getGeometryInfo(nLOD);
		pGeometry->buildTangSkins(getBoneInfos(), numBoneInfos());
		pGeometry->buildGeomSkins(getBoneInfos(), numBoneInfos());
	}
}


inline void RemoveFileName(char*path)
{
  while(path[0])
  { 
    if(path[strlen(path)-1]=='/' || path[strlen(path)-1]=='\\') 
      break; 
    else 
      path[strlen(path)-1]=0; 
  }
}


static _inline ETexType sSetType(TextureMap3 *tm)
{
  if (tm->type == TEXMAP_CUBIC)
    return eTT_Cubemap;
  else
  if (tm->type == TEXMAP_AUTOCUBIC)
    return eTT_AutoCubemap;
  return eTT_Base;
}


//////////////////////////////////////////////////////////////////////////
// fills the array m_arrDamageTable, using the bone names
void CryModel::InitDamageTable()
{
	if (m_arrDamageTable.empty())
		return;

  MEMSET_VECTOR (m_arrDamageTable, g_nDamageAreaDefault);

	for( unsigned idx = 0; idx < numBoneInfos(); ++idx )
	{
		const char* szBoneName = getBoneInfo(idx).getNameCStr();
		if (findString (szBoneName, g_szDamageBonesHead) >= 0)
			m_arrDamageTable[idx] = g_nDamageAreaHead;
		else
		if (findString (szBoneName, g_szDamageBonesTorso) >= 0)
			m_arrDamageTable[idx] = g_nDamageAreaTorso;
		else
		if (findString (szBoneName, g_szDamageBonesArmL) >= 0)
			m_arrDamageTable[idx] = g_nDamageAreaArmL;
		else
		if (findString (szBoneName, g_szDamageBonesArmR) >= 0)
			m_arrDamageTable[idx] = g_nDamageAreaArmR;
		else
		if (findString (szBoneName, g_szDamageBonesLegL) >= 0)
			m_arrDamageTable[idx] = g_nDamageAreaLegL;
		else
		if (findString (szBoneName, g_szDamageBonesLegR) >= 0)
			m_arrDamageTable[idx] = g_nDamageAreaLegR;
		else
			m_arrDamageTable[idx] = g_nDamageAreaDefault;
	}

}


//////////////////////////////////////////////////////////////////////////
// for mirrored bumpmap
void CryModel::InvertMarkedTangentBasises()
{
  CLeafBuffer ** pLeafBuffers = m_pDefaultModelState->GetCryModelSubmesh(0)->m_pLeafBuffers;

  for(unsigned nLodLevel=0; nLodLevel<numLODs(); nLodLevel++)
  {
		assert(pLeafBuffers[nLodLevel]);
    //pLeafBuffers[nLodLevel]->CorrectTangentBasisesForPolyBump(getGeometryInfo(nLodLevel)->getExtTangents());
  }
}


// computes, caches and returns the connectivity object for stencil shadows
// (or just returns, if the object is already cached)
IStencilShadowConnectivity* CryModel::getStencilShadowConnectivity(unsigned nLOD)
{
	IStencilShadowConnectivity *ret = getGeometryInfo(nLOD)->getStencilShadowConnectivity(m_arrMaterials);
	return(ret);
}


// retrieves the pointer to the static array of shadow volume vertices
///*static*/ float* CryModel::getShadowVolumeVerts ()
//{
//	return &g_arrShadowVolumeVerts[0];
//}

// expands the size of the shadow volume vertex array to the specified size
// the size defines the number of floats the array must have (at least)
///*static*/ void CryModel::expandShadowVolumeVerts(unsigned nSize)
//{
//	if (g_arrShadowVolumeVerts.size() < nSize)
//		g_arrShadowVolumeVerts.reinit(nSize);
//}


// frees the array of shadow volume vertices; this should be normally called
// in the destructor of CryModelManager, but can safely be called whenever you wish
// since expand will always restore the array
//void CryModel::freeShadowVolumeVerts( )
//{
//	g_arrShadowVolumeVerts.clear();
//}

// returns the geometry of the given lod (0 is the highest detail lod)
CryGeometryInfo* CryModel::getGeometryInfo (unsigned  nLodLevel)
{
	assert (nLodLevel >= 0 && nLodLevel < m_arrGeomInfo.size());
	return &m_arrGeomInfo[nLodLevel];
}
// returns the geometry of the given lod (0 is the highest detail lod)
const CryGeometryInfo* CryModel::getGeometryInfo (unsigned nLodLevel)const
{
	assert (nLodLevel >= 0 && nLodLevel < m_arrGeomInfo.size());
	return &m_arrGeomInfo[nLodLevel];
}

const char* CryModel::getFilePathCStr()
{
	return m_pBody->GetFilePathCStr();
}

// Returns the interface for animations applicable to this model
ICryAnimationSet* CryModel::GetAnimationSet ()
{
	return this;
}

// builds the skins out of morph targets
void CryModel::buildMorphSkins ()
{
	m_arrMorphSkins.reinit (m_arrMorphTargets.size());
	CrySkinVertexSource VertexSource (getGeometryInfo(0));

	// build the array of inverse-default-global matrices from the bone infos
	// These matrices are required by the morph skin builder to determine the offsets
	// of morphed vertices relatively to the skin
	unsigned numBones = numBoneInfos();
	std::vector<Matrix44> arrMatInvDef;
	arrMatInvDef.resize (numBones);
	for (unsigned nBone = 0; nBone < numBones; ++nBone)
		arrMatInvDef[nBone] = getBoneInfo(nBone).getInvDefGlobal();

	CrySkinMorphBuilder builder (&VertexSource, &arrMatInvDef[0], numBones);
	unsigned numMorphTargets = (unsigned)m_arrMorphTargets.size();
	for (unsigned nMorphTarget = 0; nMorphTarget < numMorphTargets; ++nMorphTarget)
	{
		CryGeomMorphTarget& rMorphTarget = m_arrMorphTargets[nMorphTarget];
		builder.initSkinMorph (rMorphTarget.getMorphVertices(0), rMorphTarget.numMorphVertices(0), &m_arrMorphSkins[nMorphTarget]);
	}
}

const CrySkinMorph& CryModel::getMorphSkin (unsigned nLOD, int nMorphTargetId)
{
	if (nLOD == 0 && nMorphTargetId >= 0 && nMorphTargetId < (int)m_arrMorphSkins.size())
		return m_arrMorphSkins[nMorphTargetId];
	else
	{
		assert(0); // memory leaks possible
		static const CrySkinMorph DefaultMorphSkin;
		return DefaultMorphSkin;
	}
}


void CryModel::addBoneLights (const std::vector<CBoneLightBindInfo>& arrLights)
{
	m_arrLocalBoneLights.clear();
	m_arrGlobalBoneLights.clear();

	unsigned numLocal = 0, numGlobal = 0;
	std::vector<CBoneLightBindInfo>::const_iterator it, itEnd = arrLights.end();

	for ( it = arrLights.begin(); it != itEnd; ++it)
		if (it->isLocal())
			++numLocal;
		else
			++numGlobal;

	m_arrLocalBoneLights.reserve (numLocal);
	m_arrGlobalBoneLights.reserve (numGlobal);

	for ( it = arrLights.begin(); it != arrLights.end(); ++it)
		if (it->isLocal())
			m_arrLocalBoneLights.push_back(*it);
		else
			m_arrGlobalBoneLights.push_back(*it);

	std::sort (m_arrGlobalBoneLights.begin(), m_arrGlobalBoneLights.end());
	std::sort (m_arrLocalBoneLights.begin(), m_arrLocalBoneLights.end());
}

// puts the size of the whole subsystem into this sizer object, classified,
// according to the flags set in the sizer
void CryModel::GetSize(class ICrySizer* pSizer)const
{
#if ENABLE_GET_MEMORY_USAGE 
	pSizer->AddContainer (m_arrLocalBoneLights);
	pSizer->AddContainer (m_arrGlobalBoneLights);
	pSizer->AddContainer (m_arrDamageTable);

	for (unsigned i = 0; i < m_arrGeomInfo.size(); ++i)
		m_arrGeomInfo[i].GetSize(pSizer);

	pSizer->AddContainer (m_arrMaterials);

	pSizer->AddObject (this, CryModelAnimationContainer::sizeofThis()+sizeof(*this)-sizeof(CryModelAnimationContainer));

	m_pDefaultModelState->GetSize(pSizer);
#endif
}

const Vec3d& CryModel::getModelOffset()const
{
	return m_vModelOffset;
}


// deletes all unused materials in the material array
void CryModel::deleteUnusedMaterials ()
{
	// first, find out which materials aren't used
	char* pUsed = new char [m_arrMaterials.size()];
	memset (pUsed, 0, m_arrMaterials.size());

	unsigned nLOD;
	for (nLOD = 0; nLOD < numLODs(); ++nLOD)
	{
		CryGeometryInfo* pLOD = getGeometryInfo(nLOD);
		assert (pLOD->numFaces());
		for (unsigned nFace = 0; nFace < pLOD->numFaces(); ++nFace)
		{
			pUsed[pLOD->getFaceMtl(nFace)] |= 1 << nLOD;
		}
	}

	// now shrink the unused material records
	unsigned nNewId = 0, nOldId = 0;
	std::vector<unsigned> arrMapMtlsNew;
	arrMapMtlsNew.resize (m_arrMaterials.size(),0);

	for (; nOldId < m_arrMaterials.size(); ++nOldId)
	{
		if (pUsed[nOldId])
		{
			m_arrMaterials[nNewId] = m_arrMaterials[nOldId];
			arrMapMtlsNew[nOldId] = nNewId;
			++nNewId;
		}
	}

	for (nLOD = 0; nLOD < numLODs(); ++nLOD)
		getGeometryInfo(nLOD)->remapMtlIds(&arrMapMtlsNew[0], (unsigned)arrMapMtlsNew.size());

	m_arrMaterials.resize (nNewId);

	delete[]pUsed;
}


void CryModel::LoadPostInitializeCCG()
{
	m_pDefaultModelState->m_nLodLevel=0;

	computeBoneBBoxes();
	// calculate the rough bounding sphere radius
	m_pDefaultModelState->InitBBox();

	m_fStaticBSphereRadius = m_pDefaultModelState->getBBox().getSize().Length()*0.5f;

	m_arrDamageTable.reinit(numBoneInfos());
	InitDamageTable();
}


// 1. completely initializes the CryModel from the given CCG file
// 2. Loads textures
// 3. Generates render arrays
// returns true if successfully initialized
bool CryModel::initFromCCG (const string& strTextureDir, const string& strAnimationDir, class CCFMemReader& Reader, float fScale)
{
	//////////////////////////////////////////////////////////////////////////
	// Load the header, prepare GeometryInfo structures
	if (Reader.GetChunkType() != CCF_HEADER_CCG)
		return false;

	m_bFromCCG = true;

	const CCFCCGHeader* pHeader = (const CCFCCGHeader*)Reader.GetData();
	unsigned numLODs = pHeader->numLODs;
	if (numLODs > 3)
	{
		g_GetLog()->LogError ("unsupported number of lods (%d)", numLODs);
		return false;
	}

	m_arrGeomInfo.reinit (numLODs);

	if (!Reader.Skip())
		return false;

	if(!m_pDefaultModelState) // load bones only for 0 lod
		m_pDefaultModelState = new CryModelState(this);

	//////////////////////////////////////////////////////////////////////////
	// Load the bones
	if (Reader.GetChunkType() != CCF_BONE_DESC_ARRAY)
	{
		g_GetLog()->LogError("Unexpected chunk type %d size %d while loading ccg", Reader.GetChunkType(), Reader.GetDataSize());
		return false;
	}

	if (!loadCCGBones((const CCFBoneDescArrayHeader*)Reader.GetData(), Reader.GetDataSize()))
		return false;

	if (numBoneInfos() == 0)
	{
		g_GetLog()->LogError("No bones have been loaded. Such animated objects are not supported");
		return false;
	}

	scaleBones (fScale);



	m_pDefaultModelState->CreateBones();
	
	unsigned numStateBones = m_pDefaultModelState->m_arrBones.empty() ? 0 : m_pDefaultModelState->numBones();
	if (numStateBones != numBoneInfos())
	{
		g_GetLog()->LogError("FATAL: could not create bones for the default model state: %d instead of %d were created. %d reported.", numStateBones, numBoneInfos(), m_pDefaultModelState->numBones());
		return false;
	}
	
	m_pDefaultModelState->InitDefaultPose (false, true);			
	m_pDefaultModelState->UpdateBBox();

	LoadPostInitializeCCG();

	if (!Reader.Skip())
		return false;

	//////////////////////////////////////////////////////////////////////////
	// Load the materials
	if (Reader.GetChunkType() != CCF_MATERIALS)
		return false;
	unsigned numMaterials = Reader.GetChunkSize()/sizeof(MAT_ENTITY_COMP);
	m_arrMaterials.clear();
	if (numMaterials)
	{
		m_arrMaterials.resize (numMaterials);
		//memset (&m_arrMaterials[0], 0, sizeof(MAT_ENTITY)*numMaterials);
		for (unsigned i = 0; i< numMaterials; ++i)
			CopyMatEntity (m_arrMaterials[i], ((MAT_ENTITY_COMP*)Reader.GetData())[i]);
	}

	fillMaterialGameId();

	unsigned nBoneGeomLOD = 0; // the LOD of the next BoneGeometry chunk
	unsigned nLOD = 0; // the next GeometryInfo chunk LOD
	unsigned numMorphTargetSets = 0; // the number of already loaded morph target sets ( up to 1 is currently supported)

	//////////////////////////////////////////////////////////////////////////
	// Load each LOD
	while (Reader.Skip())
	{
		switch (Reader.GetChunkType())
		{
		case CCF_GEOMETRY_INFO:
			if (!loadCCGLOD (nLOD, (const CCFAnimGeomInfo*)Reader.GetData(), Reader.GetDataSize()))
				return false;
			getGeometryInfo(nLOD)->getGeomSkin()->scale (fScale);
			++nLOD;
			break;

		case CCF_BONE_GEOMETRY:
			if (!loadCCGBoneGeomLOD (nBoneGeomLOD, fScale, (const CCFBoneGeometry*)Reader.GetData(), Reader.GetDataSize()))
				return false;
			++nBoneGeomLOD;
			// we loaded the physics geometry; therefore, we have physics and should tell this to the default model state
			m_pDefaultModelState->m_bHasPhysics = true;
			break;

		case CCF_MORPH_TARGET_SET:
			if (!numMorphTargetSets)
				if (!loadCCGMorphTargetSet ((const CCFMorphTargetSet*)Reader.GetData(), Reader.GetDataSize(), fScale))
					return false;
			break;

		case CCF_CHAR_LIGHT_DESC:
			if (!loadCCGLights ((const CCFCharLightDesc*)Reader.GetData(), Reader.GetDataSize(), fScale))
				g_GetLog()->LogError("\003cannot load chunk CCF_CHAR_LIGHT_DESC. No lights will be loaded into the character model %s", m_pBody->GetFilePathCStr());
			break;

		case CCF_ANIM_SCRIPT:
			if (!loadCCGAnimScript (fScale, strAnimationDir, Reader.GetData(), Reader.GetDataSize()))
			{
				g_GetLog()->LogError ("\003cannot load chunk CCF_ANIM_SCRIPT in model %s.", m_pBody->GetFilePathCStr());
				return false;
			}
			break;

			// these are reserved for future use
		case CCF_USER_PROPERTIES:
		case CCF_USER_PROPERTIES+1:
		case CCF_USER_PROPERTIES+2:
		case CCF_USER_PROPERTIES+3:
		case CCF_USER_PROPERTIES+4:
		case CCF_USER_PROPERTIES+5:
			break;

		default:
			g_GetLog()->LogWarning("\004Ignoring unexpected CCG chunk (CCF_?) %u (%u bytes)", Reader.GetChunkType(), Reader.GetChunkSize());
			break;
		}

	}
	if (nBoneGeomLOD)
		onBonePhysicsChanged();

	if (nLOD != numLODs)
		return false;

	// there must always be 0th submesh in the default model state
	m_pDefaultModelState->GetCryModelSubmesh(0)->GenerateRenderArraysCCG (strTextureDir.c_str());


	return true;
}


//////////////////////////////////////////////////////////////////////////
// loads the bone geometry for a particular bone
bool CryModel::loadCCGBoneGeom (IGeomManager* pGeomManager, unsigned nLOD, float fScale, const CCFBGBone* pHeader, unsigned nSize)
{
	if (pHeader->nBone >= numBoneInfos())
	{
		g_GetLog()->LogError ("\003CryModel::loadCCGBoneGeom:Cannot load bone #%d, bone id is out of range [0..%d)", pHeader->nBone, numBoneInfos());
		return false;
	}

	if (pHeader->numFaces > 10000 || pHeader->numVertices > 10000)
	{
		g_GetLog()->LogError ("\003CryModel::loadCCGBoneGeom:Cannot load bone #%d, too many faces and/or vertices (%d faces, %d vertices)", pHeader->numFaces, pHeader->numVertices);
		return false;
	}

	const char* pDataEnd = (const char*)pHeader + nSize;
	const Vec3d* pVertices = (const Vec3d*)(pHeader+1);
	const CCFIntFace* pFaces = (const CCFIntFace*)(pVertices+pHeader->numVertices);
	const unsigned char* pMaterials = (const unsigned char*)(pFaces+pHeader->numFaces);

	const char* pRequiredDataEnd = (const char*)(pMaterials + pHeader->numFaces);

	if (pRequiredDataEnd > pDataEnd)
	{
		g_GetLog()->LogToFile("\003: CryModel::loadCCGBoneGeom:Truncated chunk");
		return false;
	}

	std::vector<int> arrIndices;
	unsigned nFace;
	arrIndices.resize (pHeader->numFaces*3);
	for (nFace =0; nFace < pHeader->numFaces; ++nFace)
		for (unsigned j = 0; j < 3; ++j)
			arrIndices[nFace*3+j] = pFaces[nFace].v[j];

	std::vector<short>arrMtls;
	arrMtls.resize (pHeader->numFaces);
	size_t numMaterials = this->numMaterials();
	for (nFace = 0; nFace < pHeader->numFaces; ++nFace)
		arrMtls[nFace] = pMaterials[nFace] < numMaterials? getMaterial(pMaterials[nFace]).iGameID : m_nDefaultGameID;

	std::vector<Vec3d> arrVertices;
	arrVertices.resize (pHeader->numVertices);
	for (unsigned nVertex = 0; nVertex < pHeader->numVertices; ++nVertex)
		arrVertices[nVertex] = pVertices[nVertex] * fScale;

	IGeometry* pPhysicalGeometry = pGeomManager->CreateMesh(&arrVertices[0], &arrIndices[0], &arrMtls[0], pHeader->numFaces,
		(pHeader->numFaces <= 20 ?	mesh_SingleBB : mesh_OBB|mesh_AABB) | mesh_multicontact0 | mesh_approx_box | mesh_approx_sphere | mesh_approx_cylinder);

	assert (pPhysicalGeometry);

	phys_geometry* pRegisteredGeometry = pGeomManager->RegisterGeometry (pPhysicalGeometry, arrMtls.empty() ? 0 : arrMtls[0]);
	assert (pRegisteredGeometry);

	getBoneInfo(pHeader->nBone).getPhysInfo(nLOD).pPhysGeom = pRegisteredGeometry;
	
	return true;
}




//////////////////////////////////////////////////////////////////////////
// loads the bone geometry and initializes the physical geometry for
// the corresponding LOD for bones
bool CryModel::loadCCGBoneGeomLOD (unsigned nLOD, float fScale, const CCFBoneGeometry* pHeader, unsigned nSize)
{
	// reset the pointers
	for (unsigned nBone = 0; nBone < numBoneInfos(); ++nBone)
		getBoneInfo(nBone).getPhysInfo(nLOD).pPhysGeom = NULL;

	IPhysicalWorld* pPhysicalWorld = GetPhysicalWorld();
	if (!pPhysicalWorld)
	{
		// we're not initializing physics, then
		return true;
	}
	IGeomManager* pGeomManager = GetPhysicalWorld()->GetGeomManager();
	if (!pGeomManager)
		return true;

	for (CCFMemReader Reader (pHeader+1, nSize - sizeof(*pHeader)); !Reader.IsEnd(); Reader.Skip())
		switch (Reader.GetChunkType())
	{
		case CCF_BG_BONE:
			loadCCGBoneGeom (pGeomManager, nLOD,fScale, (const CCFBGBone*)Reader.GetData(), Reader.GetDataSize());
			break;
		default:
			g_GetLog()->LogError ("\003Cannot load the physics, unexpected subchunk %d", Reader.GetChunkType());
			return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
// loads the light array
bool CryModel::loadCCGLights (const CCFCharLightDesc* pData, unsigned nSize, float fScale)
{
	if (nSize < sizeof(*pData))
		return false;

	m_numLocalBoneLights = pData->numLocalLights;
	std::vector<CBoneLightBindInfo> arrBoneLights;
	arrBoneLights.resize (pData->numLights);
	const char* pRawData = (const char*)(pData+1);
	const char* pDataEnd = ((const char*)pData) + nSize;

	for (unsigned nLight = 0; nLight < pData->numLights; ++ nLight)
	{
		unsigned numReadBytes = arrBoneLights[nLight].Serialize (false, (void*)pRawData, pDataEnd-pRawData);
		if (!numReadBytes)
			return false;
		arrBoneLights[nLight].scale (fScale);
		pRawData += (numReadBytes+3)&~3;
		assert (pRawData <= pDataEnd);
	}

	addBoneLights(arrBoneLights);
	return true;
}


//////////////////////////////////////////////////////////////////////////
// loads the morph target set from CCG
bool CryModel::loadCCGMorphTargetSet (const CCFMorphTargetSet* pData, unsigned nSize, float fScale)
{
	m_arrMorphTargets.resize (pData->numMorphTargets);
	m_arrMorphSkins.resize (pData->numMorphTargets);
	unsigned numMorphSkins = 0; // the number of morph skins already loaded
	for (CCFMemReader Reader(pData+1, nSize-sizeof(*pData)); !Reader.IsEnd() && numMorphSkins < pData->numMorphTargets; Reader.Skip())
	{
		switch (Reader.GetChunkType())
		{
		case CCF_MORPH_TARGET:
			{
			const CCFMorphTarget* pHeader  = (CCFMorphTarget*)Reader.GetData();
			const char* pDataEnd = ((const char*)pHeader) + Reader.GetDataSize();
			const char* pData = (const char*)(pHeader+1);
			CrySkinMorph& rSkin = m_arrMorphSkins[numMorphSkins];
			unsigned numReadBytes = rSkin.Serialize_PC (false, (void*)pData, pDataEnd - pData);
			if (!numReadBytes || numReadBytes > Reader.GetDataSize())
				return false; // something is wrong
			rSkin.scale (fScale);
      pData += (numReadBytes + 3)&~3; // serialization data should be padded to end on 4-byte boundary
			m_arrMorphTargets[numMorphSkins].setName (pData, pDataEnd-pData);
			}
			++numMorphSkins;
			break;
		default:
			break;
		}
	}
	return numMorphSkins == pData->numMorphTargets;
}


//////////////////////////////////////////////////////////////////////////
// loads the LOD: geometry info and leaf buffers
// nSize is the size of the buffer including header; the raw data follows the header immediately
bool CryModel::loadCCGLOD (unsigned nLOD, const CCFAnimGeomInfo* pHeader, unsigned nSize)
{
	// These will get initialized from the corresponding subchunks:
	CryGeometryInfo* pLOD = getGeometryInfo(nLOD);
	pLOD->setNumUsedVertices(pHeader->numVertices);

	for (CCFMemReader Reader (pHeader+1, nSize-sizeof(*pHeader)); !Reader.IsEnd(); Reader.Skip())
		switch (Reader.GetChunkType())
	{
		case CCF_GI_PRIMITIVE_GROUPS:
			if (Reader.GetDataSize() != sizeof(CCFMaterialGroup)*pHeader->numPrimGroups)
				return false;
			pLOD->m_arrPrimGroups.resize (pHeader->numPrimGroups);
			memcpy (&pLOD->m_arrPrimGroups[0], Reader.GetData(), sizeof(CCFMaterialGroup)*pHeader->numPrimGroups);
			break;
		
		case CCF_GI_INDEX_BUFFER:
			if (Reader.GetDataSize() < pHeader->numIndices * sizeof(unsigned short))
				return false;
			pLOD->m_arrIndices.resize (pHeader->numIndices);
			memcpy (&pLOD->m_arrIndices[0],Reader.GetData(),pHeader->numIndices * sizeof(unsigned short));
			break;

		case CCF_GI_EXT_TO_INT_MAP:
			if (Reader.GetDataSize() < pHeader->numExtTangents*sizeof(unsigned short))
				return false;
			pLOD->initExtToIntMap ((const unsigned short*)Reader.GetData(), pHeader->numExtTangents);
			break;

		case CCF_GI_EXT_UVS:
			if (Reader.GetDataSize() < pHeader->numExtTangents * sizeof(CryUV))
				return false;
			pLOD->initExtUVs ((const CryUV*)Reader.GetData(), Reader.GetDataSize());
			break;

		case CCF_GI_INT_FACES:
			if (Reader.GetDataSize() < pHeader->numFaces * (sizeof(GeomFace)+sizeof(GeomMtlID)))
				return false;
			pLOD->initFaces (pHeader->numFaces, (const CCFIntFace*)Reader.GetData(), (const CCFIntFaceMtlID*)(((const CCFIntFace*)Reader.GetData())+pHeader->numFaces));
			break;

		case CCF_STENCIL_SHADOW_CONNECTIVITY:
			{
				IStencilShadowConnectivity* pConn = Get3DEngine()->NewConnectivity();
				unsigned nReadBytes = pConn->Serialize (false, (void*)Reader.GetData(), Reader.GetDataSize(), g_GetLog());
				if (!nReadBytes)
				{
					pConn->Release();
					return false;
				}
				pLOD->setStencilShadowConnectivity(pConn);
			}
			break;

		case CCF_SKIN_VERTICES:
			if (!pLOD->loadVertexSkin(Reader.GetData(), Reader.GetDataSize()))
				return false;
			break;

		case CCF_SKIN_NORMALS:
			if (!pLOD->loadNormalSkin(Reader.GetData(), Reader.GetDataSize()))
				return false;
			break;

		case CCF_SKIN_TANGENTS:
			if (!pLOD->loadTangSkin(Reader.GetData(), Reader.GetDataSize()))
				return false;
			break;
	}

	if (pLOD->numExtToIntMapEntries() != pHeader->numExtTangents)
		return false;
	if (pLOD->numExtUVs() != pHeader->numExtTangents)
		return false;

	return true;
}


// loads the animation list from the chunk
bool CryModel::loadCCGAnimScript (float fScale, string strAnimDir, const void* pAnimScriptData, unsigned nAnimScriptSize)
{
	// the animinfo pointers in the chunks carrying animinfos
	std::vector<const CCFAnimInfo*> arrAnimInfos;
	arrAnimInfos.reserve (128);

	for (CCFMemReader Reader (pAnimScriptData, nAnimScriptSize); !Reader.IsEnd(); Reader.Skip())
		switch (Reader.GetChunkType())
	{
		case CCF_ANIM_SCRIPT_DUMMYANIM:
			RegisterDummyAnimation((const char*)Reader.GetData());
			break;

		case CCF_ANIM_SCRIPT_MODELOFFSET:
			if (Reader.GetDataSize() != sizeof(Vec3d))
				g_GetLog()->LogWarning ("\003CryModel::loadCCGAnimScript:ModelOffset chunk has invalid size %d", Reader.GetDataSize());
			else
				m_vModelOffset = *(const Vec3d*)Reader.GetData();
			break;

		case CCF_ANIM_SCRIPT_ANIMINFO:
			if (Reader.GetDataSize() < sizeof(CCFAnimInfo) + 2)
			{
				g_GetLog()->LogWarning("\003CryModel::loadCCGAnimScript:Truncated chunk AnimInfo (%d bytes)", Reader.GetDataSize());
			}
			else
			{
				arrAnimInfos.push_back((const CCFAnimInfo*)Reader.GetData());
			}
			break;

		case CCF_ANIM_SCRIPT_ANIMDIR:
			strAnimDir += '\\';
			strAnimDir += (const char*)Reader.GetData();
			break;

		default:
			g_GetLog()->LogError("\003Invalid subchunk %d of AnimScript chunk", Reader.GetChunkType());
			return false;
	}

	return loadAnimations (fScale, arrAnimInfos);
}


// given the pointers to the chunk datas of the anim info chunks, loads those animations
bool CryModel::loadAnimations (float fScale, const std::vector<const CCFAnimInfo*>& arrAnimInfos)
{
	{
		AUTO_PROFILE_SECTION(g_dTimeAnimLoadBindPreallocate);
		prepareLoadCAFs ((unsigned)arrAnimInfos.size());
	}
	unsigned nAnimId = 0;
	for (std::vector<const CCFAnimInfo*>::const_iterator it = arrAnimInfos.begin(); it != arrAnimInfos.end(); ++it)
	{
		const CCFAnimInfo* pAnimInfo = *it;
		const char* pName = (const char*)(pAnimInfo+1);
		const char* pPath = pName + strlen(pName) + 1;
		int nGlobalAnimId = LoadCAF (pPath, fScale, nAnimId, pName, pAnimInfo->nAnimFlags);
		if (nGlobalAnimId < 0)
		{
			g_GetLog()->LogWarning("\003loadAnimations:can't load animation %s = %s", pName, pPath);
			continue;
		}

		m_pControllerManager->UpdateAnimation(nGlobalAnimId, pAnimInfo);
		++nAnimId;
	}
	return nAnimId > 0;
}

/*
void CryModel::loadCCGUserProperties (const char* pData, unsigned nSize)
{
	for (const char* pEnd = pData + nSize; pData < pEnd; pData += 1 + strlen(pData))
	{
		const char* pValue = pData + 1 + strlen(pData);
		if (pValue > pEnd || (!*pData && !*pValue)) // the pairs "name" "value" end with \0\0 or with one of the pair members not fitting into the chunk
			break;
		m_mapUserProps.insert (UserPropMap::value_type(pData, pValue));
		pData = pValue;
	}
}*/

// returns the extra data attached to the character by the artist during exporting
// as the scene user properties. if there's no such data, returns NULL
const char* CryModel::GetProperty(const char* szName)
{
	UserPropMap::iterator it = m_mapUserProps.find (szName);
	if (it == m_mapUserProps.end())
		return NULL;
	return it->second.c_str();
}


// fills the iGameID for each material, using its name to enumerate the physical materials in 3D Engine
void CryModel::fillMaterialGameId()
{
	IPhysMaterialEnumerator *pMatEnum = Get3DEngine()->GetPhysMaterialEnumerator();
	if (!pMatEnum)
		return;

	CMatEntityNameTokenizer mt;
	int nDefaultGameID = m_nDefaultGameID;

	for (unsigned nMaterial = 0; nMaterial < numMaterials(); ++nMaterial)
	{
		MAT_ENTITY& rMaterial = getMaterial (nMaterial);
		mt.tokenize (rMaterial.name);
		if (*mt.szPhysMtl && pMatEnum)
			rMaterial.iGameID = pMatEnum->EnumPhysMaterial(mt.szPhysMtl);
		else
			rMaterial.iGameID = nDefaultGameID;
	}
}

void CryModel::ExportModelsASC()
{
	FILE *f = fopen ("AniModelExport.txt", "at");
	if (!f)
		return;

	fprintf (f, "===================================================================================================\n");
	for (unsigned n = 0; n < numLODs(); ++n)
	{

		if (n)
			fprintf (f, "-----------------------------------------------------------------------------------------------\n");
		fprintf (f, "Exporting %s %d/%d lod\n", getFilePathCStr(), n, numLODs());
		getGeometryInfo(n)->exportASC(f);
		fprintf (f, "Index buffer:\n");
		list2<unsigned short>* pIndices = m_pDefaultModelState->GetCryModelSubmesh(0)->m_pLeafBuffers[n]->m_pIndicesPreStrip;
		for (unsigned nIndex = 0; nIndex<pIndices->size();  nIndex+=3)
		{
			fprintf (f, "0x%04x, 0x%04x, 0x%04x, //0x%04x  //0x%04x\n\t", (*pIndices)[nIndex], (*pIndices)[nIndex+1], (*pIndices)[nIndex+2], nIndex, nIndex/3);
		}


		fprintf (f, "Materials:\n");
		list2<CMatInfo>*pMats = m_pDefaultModelState->getLeafBufferMaterials();
		for (unsigned nMat = 0; nMat < pMats->size(); ++nMat)
		{
			const CMatInfo& m = (*pMats)[nMat];
			fprintf (f, "%s: indices[0x%04x..0x%04x], vertices[0x%04x..0x%04x]\n", m.sMaterialName, m.nFirstIndexId, m.nFirstIndexId + m.nNumIndices, m.nFirstVertId, m.nFirstVertId+m.nNumVerts);
		}
	}
	fprintf (f, "\n");
	fclose(f);
}
