#include "stdafx.h"
#include <StlUtils.h>
#include <I3DEngine.h>

#include "CryModel.h"
#include "CryGeomMorphTarget.h"
#include "CryCharBody.h"
#include "CryModelState.h"
#include "ChunkFileReader.h"
#include "CryModelGeometryLoader.h"
#include "CryBoneHierarchyLoader.h"
#include "StringUtils.h"
#include "CVars.h"
#include "CgfUtils.h"

CryModelGeometryLoader::CryModelGeometryLoader ():
#ifdef DEBUG_STD_CONTAINERS
	m_arrGeomBoneNameTable ("CryModelGeometryLoader.m_arrGeomBoneNameTable"),
	m_mapLights ("CryModelGeometryLoader.mapLights"),
  m_mapObjectNodes ("CryModelGeometryLoader.mapObjectNodes"),
	m_arrTempBoneIdToIndex("TempBoneIdToIndex")
	m_mapLimbPhysGeoms("CryModelGeometryLoader.mapLimbPhysGeoms"),
#endif
	m_numBoneLightBinds (0),
	m_pBoneLightBind(NULL),
	m_pModel (NULL),
	m_nLOD (-1),
	m_numMorphTargets (0)
{
}

CryModelGeometryLoader::~CryModelGeometryLoader ()
{
	clear();
}

void CryModelGeometryLoader::clear()
{
	m_pReader = NULL;
	m_arrGeomBoneNameTable.clear();
	m_numBoneLightBinds = 0;
	m_pBoneLightBind = NULL;
	m_mapLimbPhysGeoms.clear();
	m_pModel = NULL;
	m_nLOD = -1;
	m_mapLights.clear();
	m_mapObjectNodes.clear();
	m_numMorphTargets = 0;
	m_arrTempBoneIdToIndex.clear();

	m_bFirstTimeWarningBoneGeometryMtl = true;
  m_bMeshFound     = false;
  m_bGeometryFound = false;
	m_bBonesFound    = false;
	m_nGeometryChunkID = 0;

	m_pPhysicalGeometryManager = GetPhysicalWorld() ? GetPhysicalWorld()->GetGeomManager() : NULL;
}

bool CryModelGeometryLoader::load (CryModel* pModel, CChunkFileReader* pReader, unsigned nLOD, float fScale)
{
	//CAutoClear<CryModelGeometryLoader> _autoClean(this);
	clear();

	// lock the reader - we'll use the reader's memory to represent some of the structures
	m_pReader = pReader;
	m_pModel  = pModel;
	m_nLOD    = nLOD;
	m_fScale  = fScale;
	
	if (!prepare())
		return false;

	if (!loadStage1())
		return false;
	indicateProgress();

	if (!loadMaterials())
		return false;

	// we load bones, mesh and bone mesh here
	// in LOD 0 we initialize the bones, in LOD 1 we update their physics; lod 2 has no effect
	if (!loadStage2())
		return false;

	if (!loadMorphTargets())
		return false;

	if (!rebindBoneIndices())
		return false;

	if (!finalize())
		return false;

	return true;
}


bool CryModelGeometryLoader::prepare()
{
	//read the file header
  const FILE_HEADER& fh = m_pReader->getFileHeader();

	if(fh.Version != GeomFileVersion || fh.FileType != FileType_Geom) 
		// invalid file version or type
    return false;

	if(!m_pModel->m_pDefaultModelState) // load bones only for 0 lod
    m_pModel->m_pDefaultModelState = new CryModelState(m_pModel);

	return true;
}

// loads the materials; returns true if succeeds
bool CryModelGeometryLoader::loadMaterials()
{
	m_nFirstMat = (unsigned)m_pModel->m_arrMaterials.size(); // the base number of materials that was before loading this geometry (used to shift the material references)
	unsigned numLoadedMaterials = m_nFirstMat; // the number of actually loaded objects
	m_pModel->m_arrMaterials.resize (numLoadedMaterials + m_pReader->numChunksOfType(ChunkType_Mtl));

  int i;
  for (i = 0; i < m_pReader->numChunks(); i++)
  {
		const CHUNK_HEADER& chunkHeader = m_pReader->getChunkHeader(i);
    if (chunkHeader.ChunkType == ChunkType_Mtl)
    {
			const void* pChunk = m_pReader->getChunkData(i);
			unsigned nChunkSize = m_pReader->getChunkSize(i);
			CMatEntityNameTokenizer mt;
      if (loadMaterial ((CHUNK_HEADER*)pChunk, nChunkSize, m_pModel->m_arrMaterials[numLoadedMaterials], mt))
				++numLoadedMaterials;
			else
			{
				// some materials are MULTI, and they're ignored during the load, and that's normal
				//g_GetLog()->LogToFile ("*WARNING* Cannot load standard material (chunk id 0x%08X) from file %s", chunkHeader.ChunkID, getFilePathCStr());
			}
		}
  }	

	m_pModel->m_arrMaterials.resize (numLoadedMaterials);
	return true;
}



//////////////////////////////////////////////////////////////////////////
// Attempts to load the material from the given material chunk
// If the chunk is MULTi material or too old version, returns an error
// If successful, the read material is copied to the rMaterial
// PARAMETERS:
//  pMaterialChunk - the chunk to load from
//  nChunkSize     - the size of the chunk
//  rMateria - [OUT] - upon success, contains the material data upon exit
// RETURNS:
//   true  - the material was loaded successfully
//   false - the material was not loaded
bool CryModelGeometryLoader::loadMaterial (CHUNK_HEADER* pMaterialChunk, unsigned nChunkSize, MAT_ENTITY& rMaterial, CMatEntityNameTokenizer& mt)
{
	IPhysMaterialEnumerator *pMatEnum = Get3DEngine()->GetPhysMaterialEnumerator();
	MatChunkLoadErrorEnum nError = LoadMatEntity(*pMaterialChunk, pMaterialChunk, nChunkSize, rMaterial);
	if (nError == MCLE_Success)
	{
		mt.tokenize (rMaterial.name);
		if (pMaterialChunk->ChunkVersion == MTL_CHUNK_DESC_0746::VERSION)
		{
      if (*mt.szPhysMtl)
  			rMaterial.iGameID = pMatEnum ? pMatEnum->EnumPhysMaterial(mt.szPhysMtl) : 0;
      else
        rMaterial.iGameID = 0;
		}
		rMaterial = rMaterial;
		return true;
	}
	else
	{
		if (nError!= MCLE_IgnoredType)
			g_GetLog()->LogError ("cannot read material chunk 0x%X", pMaterialChunk->ChunkID);
		return false;
	}
}


// loads the bone name list into m_arrGeomBoneNameTable
// m_arrGeomBoneNameTable points to the chunk data places where the actual names are; so no need to deallocate the strings
bool CryModelGeometryLoader::loadBoneNameList (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize)
{
	bool bOk = LoadBoneNameList (chunkHeader, pChunk, nChunkSize, m_arrGeomBoneNameTable);
	if (!bOk)
		return false;

	if (m_nLOD> 0)
	{
		if (m_arrGeomBoneNameTable.size() != m_pModel->numBoneInfos())
		{
			g_GetLog()->LogError ("lod %d file %s has inconsistent number of bones (%d, expected %d). Please re-export the lod", m_nLOD, getFilePathCStr(), m_arrGeomBoneNameTable.size(), m_pModel->numBoneInfos());
		
			char str[256];

			if(g_GetISystem()->GetSSFileInfo(m_pModel->m_pBody->GetFilePathCStr(),str,256))
				g_GetLog()->LogToFile ("      %s",str);

			//return true; // we only load the table of bone names for lod 0
		}
	}

	return true;
}


// loads the bone light binding array
bool CryModelGeometryLoader::loadBoneLightBinding (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize)
{
	const BONELIGHTBINDING_CHUNK_DESC_0001* pBLBChunk = (const BONELIGHTBINDING_CHUNK_DESC_0001*)pChunk;
	m_pBoneLightBind = (const SBoneLightBind*)(pBLBChunk+1);
	m_numBoneLightBinds = pBLBChunk->numBindings;

	if (((const char*)pBLBChunk) + nChunkSize > (const char*)(m_pBoneLightBind + m_numBoneLightBinds))
	{
		g_GetLog()->LogError ("\003BoneLightBinding chunk in %s is truncated", m_pModel->m_pBody->GetNameCStr());
		m_numBoneLightBinds = 0;
		m_pBoneLightBind = NULL;
	}
	return true;
}


bool CryModelGeometryLoader::loadBoneInitialPos (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize)
{
	const BONEINITIALPOS_CHUNK_DESC_0001* pBIPChunk = (const BONEINITIALPOS_CHUNK_DESC_0001*)pChunk;
	if (chunkHeader.ChunkVersion != pBIPChunk->VERSION)
	{
		g_GetLog()->LogError ("\003BoneInitialPos chunk is of unknown version %d", chunkHeader.ChunkVersion);
		return false; // unrecognized version
	}

  unsigned numBytesLoaded = m_BoneLoader.load (pBIPChunk, nChunkSize);
	if (!numBytesLoaded)
		return false;
	return true;
}

// returns true if the bone infos have the initial position set
bool CryModelGeometryLoader::hasBoneInfoInitPos()
{
	return m_BoneLoader.hasInitPos();
}

///////////////////////////////////////////////////////////////////////
// for LODs > 0, builds the m_arrTempBoneIdToIndex by matching the bone names
// against the bone names in the already loaded LOD 0
void CryModelGeometryLoader::buildBoneIndicesByNames()
{
	// We don't keep the bone id map in the model anymore - so we map ids by names anyway
	assert (m_nLOD > 0 && m_arrTempBoneIdToIndex.empty());

	// since the bone map is not native to this file, map the bone indices by names:
	// take the names of bones in the LOD file and try to match them against the bone names
	// in the master file (lod0). For those without corresponding names, set the default of 0
	// This is used to map the LOD-bone-id to the Master-bone-index
	m_arrTempBoneIdToIndex.reinit((unsigned)m_arrGeomBoneNameTable.size(), 0);
	// TODO: for performance, we could once build a map name->index
	for (unsigned i = 0; i < m_arrGeomBoneNameTable.size(); ++i)
	{
		int nBone = m_pModel->findBone (m_arrGeomBoneNameTable[i]);
		if (nBone >= 0)
			m_arrTempBoneIdToIndex[i] = nBone;
	}
}


//////////////////////////////////////////////////////////////////////////
// load the stuff that doesn't need anything else to be already loaded:
//   + load bone name table,
//		 + for LOD>0, associate the bones with bone indices
//   + load bone-light binding table,
//   + load light and node maps,
//   + calculate the number of morph targets
bool CryModelGeometryLoader::loadStage1()
{
  int i;
  for (i = 0; i < m_pReader->numChunks(); i++)
  {
		const CHUNK_HEADER& chunkHeader = m_pReader->getChunkHeader(i);
		const void* pChunk = m_pReader->getChunkData(i);
		unsigned nChunkSize = m_pReader->getChunkSize(i);
    switch (chunkHeader.ChunkType)
    {
      case ChunkType_BoneNameList:
				if (!loadBoneNameList(chunkHeader, pChunk, nChunkSize))
					return false;
				if (m_nLOD > 0)
					// if the LOD0 already was loaded, then we should find the index of each existing bone by name
					buildBoneIndicesByNames();
        break;

			case ChunkType_BoneLightBinding:
				if (!loadBoneLightBinding(chunkHeader, pChunk, nChunkSize))
					return false;
				break;

			case ChunkType_BoneInitialPos:
				if (!loadBoneInitialPos(chunkHeader, pChunk, nChunkSize))
					return false;
				// we should scale the initial position in case we use it for bone rest pose initialization
				m_BoneLoader.scale (m_fScale);
				break;

			case ChunkType_MeshMorphTarget:
				if (chunkHeader.ChunkVersion == MESHMORPHTARGET_CHUNK_DESC_0001::VERSION)
					++m_numMorphTargets;
				break;

			case ChunkType_Light:
				{
					const LIGHT_CHUNK_DESC* pLight = (const LIGHT_CHUNK_DESC*)pChunk;
					m_mapLights[chunkHeader.ChunkID] = pLight;
				}
				break;

			case ChunkType_Node:
				{
					const NODE_CHUNK_DESC* pNode = (const NODE_CHUNK_DESC*)pChunk;
					m_mapObjectNodes[pNode->ObjectID] = pNode;
				}
				break;
    }
  }

	return true;
}


// loads the stuff that needs the stuff loaded at stage 1
bool CryModelGeometryLoader::loadStage2()
{
  //check chunks
  for (int i = 0; i < m_pReader->numChunks(); i++)
  {
		const CHUNK_HEADER& chunkHeader = m_pReader->getChunkHeader(i);
		const void* pChunk = m_pReader->getChunkData(i);
		unsigned nChunkSize = m_pReader->getChunkSize(i);

    switch (chunkHeader.ChunkType)
    {
      case ChunkType_Mesh:
				if (!loadMesh (chunkHeader, pChunk, nChunkSize))
					return false;
        break;

      case ChunkType_BoneAnim:
				if (!loadBoneAnim (chunkHeader, pChunk, nChunkSize))
					return false;
        break;

      case ChunkType_BoneMesh:
				if (!loadBoneMesh (chunkHeader, pChunk, nChunkSize))
					return false;
        break;        
    }
  }
	if (!m_bGeometryFound)
	{
    g_GetLog()->LogToFile("File %s contains no geometry", getFilePathCStr());		
		return false;
	}

	return true;
}


// loads the morph targets - after the other things are known
bool CryModelGeometryLoader::loadMorphTargets()
{
	if (m_nLOD > 0) // we don't support the LOD>0 morph targets yet
		return true;

	m_pModel->reinitMorphTargets (m_numMorphTargets, m_pModel->numLODs());
	// prepare to load the morph targets
	m_numMorphTargets = 0; // the morph target currently being loaded

	// load the morph targets
  for (int i = 0; i < m_pReader->numChunks(); i++)
  {
		const CHUNK_HEADER& chunkHeader = m_pReader->getChunkHeader(i);
		const void* pChunk = m_pReader->getChunkData(i);
		unsigned nChunkSize = m_pReader->getChunkSize(i);
		switch (chunkHeader.ChunkType)
		{
			case ChunkType_MeshMorphTarget:
				if (chunkHeader.ChunkVersion == MESHMORPHTARGET_CHUNK_DESC_0001::VERSION)
				{
					const MESHMORPHTARGET_CHUNK_DESC_0001* pMeshMorphTarget = (const MESHMORPHTARGET_CHUNK_DESC_0001*)pChunk;
					if (pMeshMorphTarget->nChunkIdMesh == m_nGeometryChunkID)
					{
						CryGeomMorphTarget& rMorphTarget = m_pModel->m_arrMorphTargets[m_numMorphTargets];
						if (rMorphTarget.load (m_nLOD,pMeshMorphTarget, nChunkSize) == nChunkSize)
						{
							// rotate the object in the world coordinates for the rest
							const NODE_CHUNK_DESC* pNodeChunk = find_in_map(m_mapObjectNodes, m_nGeometryChunkID, (const NODE_CHUNK_DESC *)NULL);
							if (pNodeChunk)
							{
								Matrix44 tm = pNodeChunk->tm;
								tm *= m_fScale;
								rMorphTarget.transform (m_nLOD, tm);
								//rMorphTarget.scale(m_nLOD, scale);
							}
							else
								rMorphTarget.scale (m_nLOD, m_fScale);

							++m_numMorphTargets;
						}
					}
				}
			break;
		}
	}
	
	// clean up the unused morph targets
	if (m_pModel->m_arrMorphTargets.size() != m_numMorphTargets)
		m_pModel->m_arrMorphTargets.resize (m_numMorphTargets); // impossible situation in the current conditions of one mesh per file

  if (!m_bBonesFound && !m_numMorphTargets && m_nLOD == 0)
  {
		// if there's no bones, then it's an error: the mesh without bones should not go through the Animation module
		g_GetLog()->LogWarning ("\003CryModel::LoadGeomChunks: Neither bones nor morph targets found in %s", m_pModel->m_pBody->GetNameCStr());
    return false;
  }

	return true;
}



// corrects the bone ids in the geometry info according to the new numeration of the bones
bool CryModelGeometryLoader::rebindBoneIndices()
{
	if (!m_arrGeomBoneNameTable.empty())
		m_pModel->getGeometryInfo(m_nLOD)->remapBoneIds(&m_arrTempBoneIdToIndex[0], (unsigned)m_arrTempBoneIdToIndex.size());

	return true;
}


//-------------------------------------------------------------

bool CryModelGeometryLoader::finalize()
{
	if (!m_nLOD)
	{
		m_pModel->m_pDefaultModelState->m_bHasPhysics = false;
		// build the bone index by name and call PostInitialize for bone infos
		m_pModel->onBonesChanged();
	}		
	if (m_nLOD < 2)
	{
		// change the pPhysGeom from indices (chunk ids) to the actual physical geometry pointers
		// This modifies the given map by deleting or zeroing elements of the map that were used during the mapping
		if (m_pModel->PostInitBonePhysGeom (m_mapLimbPhysGeoms, m_nLOD))
		{
			m_pModel->m_pDefaultModelState->m_bHasPhysics = true;
			m_pModel->onBonePhysicsChanged();
		}
	}

	// clean up the physical geometry objects that were not used
	for (ChunkIdToPhysGeomMap::iterator it = m_mapLimbPhysGeoms.begin(); it != m_mapLimbPhysGeoms.end(); ++it)
	{
		phys_geometry* pPhysGeom = it->second;
		if (pPhysGeom)
			GetPhysicalWorld()->GetGeomManager()->UnregisterGeometry(pPhysGeom);
	}
	m_mapLimbPhysGeoms.clear();

  if(m_pModel->m_arrDamageTable.empty() && m_nLOD == 0 && !m_arrGeomBoneNameTable.empty())
  {
    m_pModel->m_arrDamageTable.reinit(m_arrGeomBoneNameTable.size());
    m_pModel->InitDamageTable();
  }

	// there must be at least one shader for the object; create a default material/shader if there are no materials
  if(m_pModel->m_arrMaterials.empty())
  { // make some default material
    g_GetLog()->LogWarning ("\002CryModel::LoadGeomChunks: Materials not found in %s", getFilePathCStr());
    MAT_ENTITY me;
    memset(&me, 0, sizeof(MAT_ENTITY));    
    strcpy(me.map_d.name, "default");
    m_pModel->m_arrMaterials.push_back(me);
  }

  // fix the wrong mat id's
  m_pModel->getGeometryInfo(m_nLOD)->limitMaterialIDs((unsigned)m_pModel->m_arrMaterials.size());

	// add the light bindings to the internal array
	std::vector<CBoneLightBindInfo> arrBoneLights;
	arrBoneLights.reserve (m_numBoneLightBinds);
	for (unsigned i = 0; i < m_numBoneLightBinds; ++i)
	{
		SBoneLightBind binding = m_pBoneLightBind[i];

		if (binding.nBoneId >= m_arrTempBoneIdToIndex.size())
		{
			g_GetLog()->LogError ("\002CryModel::LoadGeomChunks: invalid light-bone binding in the file for bone id %d, light 0x%08X. Bone id is out of range.", binding.nBoneId, binding.nLightChunkId);
			continue;
		}
		binding.nBoneId = m_arrTempBoneIdToIndex[binding.nBoneId];

		// find the light chunk and the chunk of the node of that light
		const LIGHT_CHUNK_DESC* pLightChunk = find_in_map (m_mapLights, binding.nLightChunkId, (const LIGHT_CHUNK_DESC *)NULL);
		// the node chunk is required to determine the light's transformation
		// (it's stored in the light's node)
		const NODE_CHUNK_DESC* pNodeChunk = find_in_map(m_mapObjectNodes, binding.nLightChunkId, (const NODE_CHUNK_DESC *)NULL);

		if (!pLightChunk)
		{
			g_GetLog()->LogError ("\002invalid light-bone binding in the file for bone id %d, light 0x%08X", binding.nBoneId, binding.nLightChunkId);
			continue;
		}

		if (!pNodeChunk)
		{
			g_GetLog()->LogWarning ("\002there is bone light in the file, but there is no Node for it. Unknown light name. Assuming a heat source. Bone id %d, light 0x%08X", binding.nBoneId, binding.nLightChunkId);
		}

		arrBoneLights.resize (arrBoneLights.size()+1);
		arrBoneLights.back().load (binding, *pLightChunk, pNodeChunk?pNodeChunk->name:"_hs_Unknown", m_fScale);
	}

	m_pModel->addBoneLights(arrBoneLights);

	return true;
}


bool CryModelGeometryLoader::loadMesh (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize)
{
	m_bGeometryFound = true;
	m_nGeometryChunkID = chunkHeader.ChunkID;
	if(m_bMeshFound)
  {
		g_GetLog()->LogToFile ("\003CryModel::LoadGeomChunks: more than one geom found (not supported) in %s", m_pModel->m_pBody->GetNameCStr());
		return false;
  }

	const MESH_CHUNK_DESC_0744* pMeshChunk = (const MESH_CHUNK_DESC_0744*)pChunk;
	if (pMeshChunk->chdr.ChunkVersion == MESH_CHUNK_DESC_0744::VERSION)
	{
		// read the geometry info into the corresponding lod
		CryGeometryInfo* pGeomInfo = m_pModel->getGeometryInfo(m_nLOD);
		unsigned nReadData = pGeomInfo->Load (m_nLOD, pMeshChunk, nChunkSize);
		if (!pGeomInfo->numLinks() || m_arrGeomBoneNameTable.empty())
		{
			// rotate the object in the world coordinates for the rest
			const NODE_CHUNK_DESC* pNodeChunk = find_in_map(m_mapObjectNodes, m_nGeometryChunkID,(const NODE_CHUNK_DESC *)NULL);
			if (pNodeChunk)
			{
				Matrix44 tm = pNodeChunk->tm;
				tm *= m_fScale;
				pGeomInfo->transform (tm);
				//pGeomInfo->Scale (scale);
			}
			else
				pGeomInfo->Scale (m_fScale);
		}
		else
			pGeomInfo->Scale (m_fScale);


		// check if we read the whole chunk
		if (nReadData != nChunkSize)
			g_GetLog()->LogToFile ("\003CryModel::LoadGeomChunks: unexpected geometry chunk size: read %d bytes of %d", nReadData, nChunkSize);
		if (nReadData == 0)
			return false;

		// check if we read the bone info
		if (pGeomInfo->numLinks() == 0 && m_numMorphTargets == 0)
		{
			// Mesh without bone info should not go through the Animation module
			g_GetLog()->LogWarning ("\003CryModel::LoadGeomChunks: the mesh doesn't have any bone information.");
			return false;
		}
	}
	else
	{
		g_GetLog()->LogError ("\003CryModel::LoadGeomChunks: old file format, mesh chunk version 0x%08X (expected 0x%08X)",pMeshChunk->chdr.ChunkVersion, MESH_CHUNK_DESC_0744::VERSION);
		return false;
	}
	if (g_GetCVars()->ca_Debug())
		g_GetLog()->LogToFile ("\005  %d faces in lod %d", m_pModel->getGeometryInfo(m_nLOD)->numFaces(), m_nLOD);

	m_bMeshFound = true;

	return true;
}



bool CryModelGeometryLoader::loadBoneAnim (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize)
{
	if (!m_bBonesFound)
	{
		switch(m_nLOD)
		{
		case 0:// load bones structure
			if (!loadBones (static_cast<const BONEANIM_CHUNK_DESC*>(pChunk), nChunkSize))
				return false; // failed to load bones
			m_pModel->m_pDefaultModelState->CreateBones();
			m_bBonesFound = true;
			break;
		case 1:
			m_pModel->UpdateRootBonePhysics (static_cast<const BONEANIM_CHUNK_DESC*>(pChunk), nChunkSize, m_nLOD);
			break;
		}
	}
	else
	{
		g_GetLog()->LogWarning ("\003multiple bone animation chunks found; ignoring");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
// loads the root bone (and the hierarchy) and returns true if loaded successfully
// this gets called only for LOD 0
bool CryModelGeometryLoader::loadBones (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize)
{
	if (pChunk->nBones != m_arrGeomBoneNameTable.size())
	{
		g_GetLog()->LogToFile ("\002Number of bones does not match in the bone hierarchy chunk (%d) and the bone name chunk (%d)", pChunk->nBones, m_arrGeomBoneNameTable.size());
		return false;
	}

	unsigned numBytesRead = m_BoneLoader.load(pChunk, nChunkSize);
	if (!numBytesRead)
	{
		g_GetLog()->LogError ("\002CryModelGeometryLoader::loadBones: cannot load the boneanimation chunk");
		return false;
	}

	if (numBytesRead != nChunkSize)
		g_GetLog()->LogWarning ("\002CryModelGeometryLoader::loadBones: bone animation chunk is too big (%u, expected %u). Ignoring the extra bytes.", numBytesRead, nChunkSize);

	m_pModel->m_arrBones.resize (pChunk->nBones);
	for (int nBone = 0; nBone < pChunk->nBones; ++nBone)
		m_pModel->m_arrBones[nBone] = m_BoneLoader.getBoneByIndex(nBone);

	unsigned numBones = m_pModel->numBoneInfos();
	m_arrTempBoneIdToIndex.reinit (numBones, -1);
	// fill in names of the bones, (limb ids?)
	for (unsigned nBone = 0; nBone < numBones; ++nBone)
	{
		unsigned nBoneId = (unsigned)m_BoneLoader.mapIndexToId(nBone);
		m_pModel->getBoneInfo(nBone).setName(m_arrGeomBoneNameTable[nBoneId]);
		m_arrTempBoneIdToIndex[nBoneId] = nBone;
	}

	return true;
}


bool CryModelGeometryLoader::loadBoneMesh (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize)
{ // load bones geometry

	const MESH_CHUNK_DESC_0744* pMeshChunk = (const MESH_CHUNK_DESC_0744*)pChunk;
	if (pMeshChunk->chdr.ChunkVersion != MESH_CHUNK_DESC_0744::VERSION)
	{
		g_GetLog()->LogError ("\002CryModel::LoadGeomChunks: old file format, mesh chunk version 0x%08X (expected 0x%08X)",pMeshChunk->chdr.ChunkVersion, MESH_CHUNK_DESC_0744::VERSION);
		return false;
	}

	// load the geometry from the chunk
	CryGeometryInfo GeomInfo;
	unsigned nReadData  = GeomInfo.Load (2, pMeshChunk, nChunkSize);
	if (nReadData != nChunkSize)
		g_GetLog()->LogError ("\002CryModel::LoadGeomChunks: unexpected bone mesh chunk size: read %d bytes of %d", nReadData, nChunkSize);
	if (nReadData != 0) // to account for a lot of broken mesh chunks; <<TODO>> change this line to "else" when all cgf are not broken
	{
		GeomInfo.Scale(m_fScale);
		TElementaryArray<vectorf> points ("CryModelGeometryLoader::loadBoneMesh.points");
		points.reinit(GeomInfo.numVertices());
		TElementaryArray<int> indices ("CryModelGeometryLoader::loadBoneMesh.indices");
		indices.reinit(GeomInfo.numFaces()*3);
		TElementaryArray<unsigned char> pmats ("CryModelGeometryLoader::loadBoneMesh.pmats");
		pmats.reinit(GeomInfo.numFaces());

		unsigned j, k;
		for (j = 0; j < GeomInfo.numVertices(); ++j)
			points[j] = GeomInfo.getVertex(j);

		for (j = 0; j < GeomInfo.numFaces(); ++j) 
		{
			GeomMtlID nMtlID = GeomInfo.getFaceMtl(j);
			const GeomFace& face = GeomInfo.getFace(j);
			for (k = 0; k < 3; ++k)
				indices[j*3+k] = face[k];
			
      if( nMtlID >= 0 &&
          (unsigned)nMtlID + m_nFirstMat < m_pModel->numMaterials())
        pmats[j] = m_pModel->getMaterial(nMtlID+m_nFirstMat).iGameID;
      else
      {
				if(m_bFirstTimeWarningBoneGeometryMtl && nMtlID != 255 && nMtlID != -1) // 255/-1 are the no-materials
				{
      		g_GetLog()->LogWarning ("\004CryModel::LoadGeomChunks: Bone geometry material id is invalid: %s", getFilePathCStr());
					m_bFirstTimeWarningBoneGeometryMtl = false;
				}

        pmats[j] = 0;
      }
		}

		if(m_pPhysicalGeometryManager) 
		{
			// add a new entry to the map chunk->limb geometry, so that later it can be found by the bones
			// during the post-initialization mapping of pPhysGeom from Chunk id to the actual geometry pointer
			IGeometry* pPhysicalGeometry = m_pPhysicalGeometryManager->CreateMesh (
				// hack: the interface method has short* parameter, that should actually be const void*, because it can accept both char* and short* arrays, depending on the flags
					&points[0], &indices[0], (short*)&pmats[0],
					(unsigned)GeomInfo.numFaces(),
					(GeomInfo.numFaces()<=20 ? mesh_SingleBB : mesh_OBB|mesh_AABB)
						| mesh_multicontact0 | mesh_uchar_ids | mesh_approx_box | mesh_approx_sphere | mesh_approx_cylinder
				);
			assert (pPhysicalGeometry);
			m_mapLimbPhysGeoms[chunkHeader.ChunkID] =
				m_pPhysicalGeometryManager->RegisterGeometry (pPhysicalGeometry, pmats.empty() ? 0 : pmats[0]);
		}
	}

	return true;
}

// returns the file path tot he file being currently loaded
const char* CryModelGeometryLoader::getFilePathCStr()const
{
	return m_pModel->m_pBody->GetNameCStr();
}

void CryModelGeometryLoader::indicateProgress(const char*szMsg)
{
	//g_GetLog()->UpdateLoadingScreenPlus(/*szMsg?szMsg:*/"\003.");
}
