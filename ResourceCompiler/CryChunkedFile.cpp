////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   CryChunkedFile.cpp
//  Version:     v1.00
//  Created:     13.01.2003 by Sergiy
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <stdarg.h>
#include "CryVertexBinding.h"
#include "CgfUtils.h"
#include "CryChunkedFile.h"
#include "CryBoneDesc.h"
#include "CryBoneHierarchyLoader.h"

#pragma warning(default:4018)

//E:\GAME01\STLPORT\stlport

// parses the given chunked file into the internal structure;
// outputs warnings if some of the chunks are incomplete or otherwise broken
CryChunkedFile::CryChunkedFile (CChunkFileReader* pFile):
	m_pFile (pFile),
	pTiming (NULL),
	pRangeEntities (NULL),
	pFileHeader (NULL),
	m_numBoneLightBinds (0),
	m_pBoneLightBind (NULL),
	numSceneProps(0),
	pSceneProps(NULL)
{

	unsigned numChunks = pFile->numChunks();
	this->pFileHeader = &pFile->getFileHeader();

	for (unsigned nChunk = 0; nChunk < numChunks; ++nChunk)
	{
		const void* pChunkData = pFile->getChunkData(nChunk);
		unsigned nChunkSize = pFile->getChunkSize(nChunk);
		const CHUNK_HEADER& chunkHeader = pFile->getChunkHeader(nChunk);

		switch (chunkHeader.ChunkType)
		{
		case ChunkType_Timing:
			addChunkTiming (chunkHeader, (const TIMING_CHUNK_DESC*)pChunkData, nChunkSize);
			break;
		case ChunkType_Node:
			addChunkNode (chunkHeader, (const NODE_CHUNK_DESC*)pChunkData, nChunkSize);
			break;
		case ChunkType_Light:
			addChunkLight (chunkHeader, (const LIGHT_CHUNK_DESC*)pChunkData, nChunkSize);
			break;
		case ChunkType_Mesh:
			addChunkMesh (chunkHeader, (const MESH_CHUNK_DESC*)pChunkData, nChunkSize);
			break;
		case ChunkType_BoneMesh:
			addChunkBoneMesh (chunkHeader, (const MESH_CHUNK_DESC*)pChunkData, nChunkSize);
			break;
		case ChunkType_Mtl:
			addChunkMaterial(chunkHeader, pChunkData, nChunkSize);
			break;
		case ChunkType_BoneAnim:
			addChunkBoneAnim (chunkHeader, (const BONEANIM_CHUNK_DESC*)pChunkData, nChunkSize);
			break;
		case ChunkType_BoneInitialPos:
			addChunkBoneInitialPos (chunkHeader, (const BONEINITIALPOS_CHUNK_DESC_0001*)pChunkData, nChunkSize);
			break;
		case ChunkType_BoneNameList:
			addChunkBoneNameList (chunkHeader, pChunkData, nChunkSize);
			break;
		case ChunkType_MeshMorphTarget:
			addChunkMeshMorphTarget (chunkHeader, (const MESHMORPHTARGET_CHUNK_DESC_0001*)pChunkData, nChunkSize);
			break;
		case ChunkType_BoneLightBinding:
			addChunkBoneLightBinding (chunkHeader, (const BONELIGHTBINDING_CHUNK_DESC_0001*)pChunkData, nChunkSize);
			break;
		case ChunkType_SceneProps:
			addChunkSceneProps(chunkHeader, (const SCENEPROPS_CHUNK_DESC*)pChunkData, nChunkSize);
			break;
		}
	}

	// necessary for superfluous content (misc. back-pointers that can be deduced without any additional info) post-step:
	adjust();
}


CryChunkedFile::~CryChunkedFile()
{
}

void CheckChunk (const CHUNK_HEADER& chunkHeader, unsigned nChunkSize, unsigned nExpectedVersion, unsigned nExpectedSize)
{
	if (chunkHeader.ChunkVersion != nExpectedVersion)
		throw CryChunkedFile::Error ("Unexpected timing chunk 0x%X version 0x%X", chunkHeader.ChunkID, chunkHeader.ChunkVersion);
	if (nChunkSize < nExpectedSize)
		throw CryChunkedFile::Error ("Truncated node chunk 0x%X: %d bytes (at least %d bytes expected)", chunkHeader.ChunkID, nChunkSize, nExpectedSize);
}


void CryChunkedFile::addChunkTiming (const CHUNK_HEADER& chunkHeader, const TIMING_CHUNK_DESC* pChunkData, unsigned nChunkSize)
{
	CheckChunk(chunkHeader, nChunkSize, pChunkData->VERSION, sizeof(TIMING_CHUNK_DESC) + sizeof(RANGE_ENTITY)*pChunkData->nSubRanges);

	this->pTiming = pChunkData;
	this->pRangeEntities = (const RANGE_ENTITY*)(pChunkData+1);
}


// 
void CryChunkedFile::addChunkNode (const CHUNK_HEADER& chunkHeader, const NODE_CHUNK_DESC* pChunkData, unsigned nChunkSize)
{
	CheckChunk(chunkHeader, nChunkSize, NODE_CHUNK_DESC::VERSION, sizeof(NODE_CHUNK_DESC) + pChunkData->PropStrLen + sizeof(int)*pChunkData->nChildren);

	unsigned nNodeIdx = this->arrNodes.size();
	this->mapObjectNodeIdx.insert (NodeIdxMap::value_type(pChunkData->ObjectID, nNodeIdx));
	this->mapNodeIdx.insert (NodeIdxMap::value_type(chunkHeader.ChunkID, nNodeIdx));
	this->arrNodes.push_back (NodeDesc(pChunkData));
}


void CryChunkedFile::addChunkLight (const CHUNK_HEADER& chunkHeader, const LIGHT_CHUNK_DESC* pChunkData, unsigned nChunkSize)
{
	CheckChunk(chunkHeader, nChunkSize, LIGHT_CHUNK_DESC::VERSION, sizeof(*pChunkData));

	this->mapLights[chunkHeader.ChunkID] = pChunkData;
}


void CryChunkedFile::addChunkMesh (const CHUNK_HEADER& chunkHeader, const MESH_CHUNK_DESC* pChunkData, unsigned nChunkSize)
{
	CheckChunk (chunkHeader, nChunkSize, MESH_CHUNK_DESC::VERSION, sizeof(MESH_CHUNK_DESC));
	this->mapMeshIdx.insert (MeshIdxMap::value_type (chunkHeader.ChunkID, this->arrMeshes.size()));
	this->arrMeshes.push_back(MeshDesc(pChunkData, nChunkSize));
}

void CryChunkedFile::addChunkBoneMesh (const CHUNK_HEADER& chunkHeader, const MESH_CHUNK_DESC* pChunkData, unsigned nChunkSize)
{
	CheckChunk (chunkHeader, nChunkSize, MESH_CHUNK_DESC::VERSION, sizeof(MESH_CHUNK_DESC));
	this->mapBoneMeshIdx.insert (MeshIdxMap::value_type (chunkHeader.ChunkID, this->arrBoneMeshes.size()));
	this->arrBoneMeshes.push_back(MeshDesc(pChunkData, nChunkSize));
}


// this is postprocess: after all structures are in place, this function
// sets the superfuous pointers/etc
void CryChunkedFile::adjust()
{
	unsigned numBones = this->Bones.numBones(), nBone, i;

	// for each mesh, try to find and set the node
	// also remap the bone ids to bone indices
	// also recalculate
	for (i = 0; i < arrMeshes.size(); ++i)
	{
		MeshDesc& Mesh = arrMeshes[i];
		Mesh.pNode = NULL;
		if (numBones > 0)
			Mesh.remapBoneIds (this->Bones.getIdToIndexMap(),numBones);
	}

	if (numBones != this->arrNames.size())
	{
		throw Error ("bones count (%d) is differs from names count (%d); file is corrupted.", this->Bones.numBones(), this->arrNames.size());
	}

	// Assign a name to every bone
  for (nBone = 0; nBone < numBones; ++nBone)
	{
		unsigned nBoneId = this->Bones.mapIndexToId(nBone);
		this->Bones.m_arrBones[nBone].setName (this->arrNames[nBoneId]);
	}

	//if (numBones)
	//	computeBoneBBoxes();

	// fill in the back-references from mesh to nodes,
	// and references from nodes to their parent and children nodes
	for (i = 0; i < arrNodes.size(); ++i)
	{
		NodeDesc* pNode = &arrNodes[i];
		MeshDesc* pMesh = GetMeshDesc (pNode->pDesc->ObjectID);
		if (pMesh)
			pMesh->pNode = pNode;

		pNode->pParent = GetNodeDesc (pNode->pDesc->ParentID);
		pNode->arrChildren.clear();
		for (int j = 0; j < pNode->pDesc->nChildren; ++j)
			pNode->arrChildren.push_back(GetNodeDesc(pNode->pChildren[j]));
	}

	if (!arrMeshes.empty())
		arrMeshes[0].arrMorphTargets.reserve (m_arrMorphTargetChunks.size());

	// fill each mesh morph target array
	for (unsigned nMT = 0; nMT < m_arrMorphTargetChunks.size(); ++nMT)
	{
		const MorphTargetChunk& rChunk = m_arrMorphTargetChunks[nMT];
		MeshDesc* pMesh = GetMeshDesc(rChunk.pData->nChunkIdMesh);
		if (pMesh)
		{
			MorphTargetDesc desc;
			desc.numMorphVertices = rChunk.pData->numMorphVertices;
			desc.pMorphVertices = (const SMeshMorphTargetVertex*)(rChunk.pData+1);
			const char* pName = (const char*)(desc.pMorphVertices + desc.numMorphVertices);
			const char* pDataEnd = ((const char*)rChunk.pData) + rChunk.nSize;
			desc.strName.assign (pName, pDataEnd);
			pMesh->arrMorphTargets.push_back(desc);
		}
	}
}

// given a correct array of links and array of bones (with indices synchronized)
// calculates for each bone description a bounding box
/*
void CryChunkedFile::computeBoneBBoxes()
{
	if (this->Bones.numBones() == 0 || this->arrMeshes.empty())
		return;


	MeshDesc::VertBindArray::const_iterator it = arrMeshes[0].arrVertBinds.begin(), itEnd = arrMeshes[0].arrVertBinds.end();
	for (; it != itEnd; ++it)
	{
		for (CryVertexBinding::const_iterator itLink = it->begin(); itLink != it->end(); ++itLink)
		{
			CryBoneDesc& rBone = this->Bones.m_arrBones[itLink->BoneID];
			AddToBounds(itLink->offset, rBone.m_vBBMin, rBone.m_vBBMax);
		}
	}
}
*/


CryChunkedFile::NodeDesc::NodeDesc (const NODE_CHUNK_DESC* pDesc)
{
  this->pDesc = pDesc;
	
	if (pDesc->nChildren)
		this->pChildren = (const int*)((const char*)(pDesc+1) + pDesc->PropStrLen);
	else
		this->pChildren = NULL;
	
	//if ((pDesc->PropStrLen&3)!=0 && pDesc->nChildren)
	//	throw (Error("Node chunk contains unaligned data"));

	if (pDesc->PropStrLen)
		this->strProps.assign ((const char*)(pDesc+1), pDesc->PropStrLen);
	else
		this->strProps.erase();
}


CryChunkedFile::MeshDesc::MeshDesc (const MESH_CHUNK_DESC* pChunk, unsigned nSize):
	pVColors(NULL)
{
	this->pNode = NULL;
	this->pDesc = pChunk; // from now on, we can use num****s() functions

	const char* pChunkEnd = ((const char*)pChunk) + nSize;

	this->pVertices = (const CryVertex*)(pChunk+1);
	this->pFaces = (const CryFace*)(this->pVertices + numVertices());

	if ((const char*)(this->pFaces) > pChunkEnd)
		throw Error ("Mesh Chunk Truncated at vertex array (%d vertices expected)", numVertices());

	this->pUVs = (const CryUV*)(this->pFaces+numFaces());

	if ((const char*)this->pUVs > pChunkEnd)
		throw Error ("Mesh Chunk Truncated at face array (%d faces expected)", numFaces());

	this->pTexFaces = (const CryTexFace*)(this->pUVs + numUVs());

	if ((const char*)this->pTexFaces > pChunkEnd)
		throw Error ("Mesh Chunk Truncated at UV array (%d UVs expected)", numUVs());

	const void* pRawData = this->pTexFaces + numTexFaces();

	this->arrVertBinds.clear();
	if (hasBoneInfo())
	{
		this->arrVertBinds.resize (numVertices());

    for (int i=0; i < pChunk->nVerts; i++)
    {
			// the links for the current vertex in the geometry info structure
			CryVertexBinding& arrLinks = getLink(i);
			
			// read the number of links and initialize the link array
			{
				// the number of links for the current vertex
				unsigned numLinks; 
				if (!EatRawData (&numLinks, 1, pRawData, nSize))
					throw Error("Truncated vertex link array");

				if(numLinks > 32u)
					throw Error("Number of links for vertex (%u) is invalid", numLinks);

				arrLinks.resize(numLinks);
			}

			if (arrLinks.empty())
			{
				//throw("File contains unbound vertices");
				// if we choose not to throw, bind the unbound vertices to the root bone
				arrLinks.resize(1);
				arrLinks[0].Blending = 1;
				arrLinks[0].BoneID = 0;
				arrLinks[0].offset = Vec3d (0,0,0);
				continue;
			}
			else {

				u32 size	=	arrLinks.size();

				if (!EatRawData(&arrLinks[0], arrLinks.size(), pRawData, nSize))
					throw Error("Truncated vertex link array");

				//---------------------------------------------------
				//changes to optimize skinning added by ivo
				//---------------------------------------------------
				for (u32 x=0; x<size; x++) {
					for (u32 y=x+1; y<size; y++) {
						if (arrLinks[x].BoneID==arrLinks[y].BoneID) {
							//add blending of same bone to 1st found bone in list;			
							arrLinks[x].Blending+=arrLinks[y].Blending;					
							//remove link from the list;			
							for (u32 z=y; z<size; z++) arrLinks[z]=arrLinks[z+1];
							arrLinks.pop_back();
							size	=	arrLinks.size();
							y--;  
						}
					}
				}

				//---------------------------------------------------------------

				if (size==1) { arrLinks[0].Blending=1.0f;	}
				else 
				{
					//loop over all vertices and check for "minimal" blending vlaues			
					for (u32 i=0; i<size; i++)
					{
						float minval=0.12f;
						float f=arrLinks[i].Blending;
						if (f<(0.0f+minval)) arrLinks[i].Blending=0.0f;
						if (f>(1.0f-minval)) arrLinks[i].Blending=1.0f;
					}

					for (u32 x=0; x<size; x++) 
					{
						if (arrLinks[x].Blending==0.0f) 
						{
							//remove link from the list;			
							for (u32 z=x; z<size; z++) arrLinks[z]=arrLinks[z+1];
							arrLinks.pop_back();
							size	=	arrLinks.size();
							x--;  
						}
					}

					float t=0;
					//sum up all blending values
					for (u32 i=0; i<size; i++)	t+=arrLinks[i].Blending;
					//normalized blending
					for (u32 i=0; i<size; i++)	arrLinks[i].Blending/=t;
				}

				//-------------------------------------------------------------------------

				//check if summed blending of all bones is 1.0f
				float Blending=0;
				for (u32 i=0; i<size; i++)	Blending+=arrLinks[i].Blending;

				assert( fabsf(Blending-1.0f)<0.005f );

			}

    }
	}

	if (pChunk->HasVertexCol)
	{
		this->pVColors = (const CryIRGB*)pRawData;
		if (nSize < pChunk->nVerts * sizeof(CryIRGB))
			throw Error ("Vertex Color chunk is truncated: %d bytes expected, %d bytes available", pChunk->nVerts * sizeof(CryIRGB), nSize);
	}
	else
		this->pVColors = NULL;

	validateIndices();
	buildReconstructedNormals();
}


// returns node pointer by the node chunk id; if chunkid is not node , returns NULL
CryChunkedFile::NodeDesc* CryChunkedFile::GetNodeDesc (unsigned nChunkId)
{
	NodeIdxMap::const_iterator it = this->mapNodeIdx.find (nChunkId);
	if (it == this->mapNodeIdx.end())
		return NULL;
	return &this->arrNodes[it->second];
}

// returns node pointer by the object (to which the node refers, and which should refer back to node) id
// if can't find it, returns NULL
CryChunkedFile::NodeDesc* CryChunkedFile::GetObjectNodeDesc (unsigned nObjectId)
{
	NodeIdxMap::const_iterator it = this->mapObjectNodeIdx.find (nObjectId);
	if (it == this->mapObjectNodeIdx.end())
		return NULL;
	return &this->arrNodes[it->second];
}

// returns light pointer by the light chunk id.
// returns NULL on failure
const LIGHT_CHUNK_DESC* CryChunkedFile::GetLightDesc (unsigned nChunkId)
{
	LightMap::iterator it = this->mapLights.find (nChunkId);
	if (it == this->mapLights.end())
		return NULL;
	return it->second;
}


// returns mesh pointer by the mesh chunk id
CryChunkedFile::MeshDesc* CryChunkedFile::GetMeshDesc (unsigned nChunkId)
{
	MeshIdxMap::const_iterator it = this->mapMeshIdx.find (nChunkId);
	if (it == this->mapMeshIdx.end())
		return NULL;
	return &this->arrMeshes[it->second];
}

// returns mesh pointer by the mesh chunk id
CryChunkedFile::MeshDesc* CryChunkedFile::GetBoneMeshDesc (unsigned nChunkId)
{
	MeshIdxMap::const_iterator it = this->mapBoneMeshIdx.find (nChunkId);
	if (it == this->mapBoneMeshIdx.end())
		return NULL;
	return &this->arrBoneMeshes[it->second];
}

bool CryChunkedFile::DoesMtlCastShadow(int nMtl)
{
	if (nMtl < 0 || (unsigned)nMtl >= this->arrMtls.size())
		return true;

	if (this->arrMtls[nMtl].Dyn_StaticFriction == 1)
		return false; //doesn't cast shadow
	return true;
}


void CryChunkedFile::addChunkMaterial (const CHUNK_HEADER& chunkHeader, const void* pChunkData, unsigned nChunkSize)
{
	MAT_ENTITY me;

	MatChunkLoadErrorEnum nError = LoadMatEntity (chunkHeader, pChunkData, nChunkSize, me);

	switch(nError)
	{
	case MCLE_Success:
		this->arrMtls.push_back(me);
		break;
	case MCLE_IgnoredType:
		break;
	case MCLE_Truncated:
		throw Error ("Material chunk 0x%X is truncated (%d bytes)", chunkHeader.ChunkID, nChunkSize);
		break;
	case MCLE_UnknownVersion:
		throw Error ("Material chunk 0x%X is unknown version 0x%X", chunkHeader.ChunkID, chunkHeader.ChunkVersion);
		break;
	}	
}


void CryChunkedFile::addChunkBoneNameList (const CHUNK_HEADER& chunkHeader, const void* pChunkData, unsigned nChunkSize)
{
	if (!LoadBoneNameList(chunkHeader, pChunkData, nChunkSize, this->arrNames))
		throw Error ("Cannot load bone name list");
}


void CryChunkedFile::addChunkBoneInitialPos (const CHUNK_HEADER& chunkHeader, const BONEINITIALPOS_CHUNK_DESC_0001* pChunkData, unsigned nChunkSize)
{
	if (chunkHeader.ChunkVersion != pChunkData->VERSION)
		throw Error ("Unexpected BoneInitialPos chunk version 0x%X", chunkHeader.ChunkVersion);

	unsigned numBytesRead = this->Bones.load (pChunkData, nChunkSize);
	if (numBytesRead != nChunkSize)
		throw Error ("Can't read BoneInitialPos chunk: %d bytes parsed instead of %d", numBytesRead, nChunkSize);
}

void CryChunkedFile::addChunkMeshMorphTarget (const CHUNK_HEADER& chunkHeader, const MESHMORPHTARGET_CHUNK_DESC_0001* pChunkData, unsigned nChunkSize)
{
	if (chunkHeader.ChunkVersion != pChunkData->VERSION)
		throw Error ("Unexpected MeshMorphTarget chunk version 0x%X", chunkHeader.ChunkVersion);

	if (nChunkSize < sizeof(*pChunkData))
		throw Error ("Truncated MeshMorphTarget chunk header");

	if (nChunkSize < sizeof(*pChunkData) + pChunkData->numMorphVertices*sizeof(SMeshMorphTargetVertex))
		throw Error ("Truncated MeshMorphTarget chunk data");

	MorphTargetChunk Chunk;
	Chunk.nSize = nChunkSize;
	Chunk.pData = pChunkData;
	m_arrMorphTargetChunks.push_back (Chunk);
}


void CryChunkedFile::addChunkBoneAnim (const CHUNK_HEADER& chunkHeader, const BONEANIM_CHUNK_DESC* pChunkData, unsigned nChunkSize)
{
	if (nChunkSize < sizeof(*pChunkData))
		throw Error ("Truncated BoneAnim chunk header: %d bytes", nChunkSize);
	if (pChunkData->VERSION != pChunkData->chdr.ChunkVersion)
		throw Error ("Unexpected BoneAnim chunk version 0x%X", pChunkData->chdr.ChunkVersion);
	if (nChunkSize < sizeof(*pChunkData) + sizeof(BONE_ENTITY)*pChunkData->nBones)
		throw Error ("Truncated BoneAnim chunk: %d bytes, %d bones", nChunkSize, pChunkData->nBones);

	unsigned numBytesRead = this->Bones.load(pChunkData, nChunkSize);
	if (numBytesRead != nChunkSize)
		throw Error ("Can't read BoneAnim chunk: %d bytes parsed (%d total bytes)", numBytesRead, nChunkSize);
	if (this->Bones.m_arrBones.size() != pChunkData->nBones)
		throw Error ("Unexpected number of bones in BoneAnim chunk: %d claimed, %d loaded", pChunkData->nBones, this->Bones.m_arrBones.size());

	// update the bone physics as if it were LOD 0
	BONE_ENTITY* pBoneEntities = (BONE_ENTITY*)(pChunkData+1);
	for (int nId = 0; nId < pChunkData->nBones; ++nId)
	{
		int nIndex = this->Bones.mapIdToIndex(nId);
		this->Bones.m_arrBones[nIndex].UpdatePhysics(pBoneEntities[nId], 0);
	}
}


void CryChunkedFile::addChunkBoneLightBinding (
	const CHUNK_HEADER& chunkHeader,
	const BONELIGHTBINDING_CHUNK_DESC_0001* pChunkData,
	unsigned nChunkSize)
{
	if (m_pBoneLightBind)
		throw Error ("There are multiple BoneLightBinding chunks in the file. This is not supported at the moment.");

	if (pChunkData->numBindings > (unsigned)m_pFile->numChunks())
		throw Error ("BoneLightBinding chunk has invalid number of bindings declared (%d)", pChunkData->numBindings);

	unsigned numExpectedBytes = pChunkData->numBindings * sizeof(SBoneLightBind) + sizeof(*pChunkData);
	if (numExpectedBytes != nChunkSize)
		throw Error ("BoneLightBinding chunk has unexpected length (%d instead of %d)", nChunkSize, numExpectedBytes);

	m_pBoneLightBind = (const SBoneLightBind*)(pChunkData + 1);
	m_numBoneLightBinds = pChunkData->numBindings;
}

void CryChunkedFile::addChunkSceneProps (const CHUNK_HEADER& chunkHeader, const SCENEPROPS_CHUNK_DESC*pChunkData, unsigned nChunkSize)
{
	if (nChunkSize < sizeof(*pChunkData) )
		throw Error ("Truncated SCENEPROPS_CHUNK_DESC chunk header (%d bytes, at least %d expected)", nChunkSize, sizeof(*pChunkData));
	if (nChunkSize < sizeof(*pChunkData) + sizeof(SCENEPROP_ENTITY) * pChunkData->nProps)
		throw Error ("Truncated SCENEPROPS_CHUNK_DESC chunk, %d props, %d bytes, %d expected", pChunkData->nProps, nChunkSize , sizeof(*pChunkData) + sizeof(SCENEPROP_ENTITY) * pChunkData->nProps);
	this->numSceneProps = pChunkData->nProps;
	this->pSceneProps = (const SCENEPROP_ENTITY*)(pChunkData+1);
}


CryChunkedFile::Error::Error(const char* szFormat, ...)
{
	va_list args;
	va_start (args,szFormat);
	char szBuf[0x1000];
	_vsnprintf (szBuf, sizeof(szBuf), szFormat, args);
	va_end (args);
	this->strDesc = szBuf;
}

// remaps the bone ids using the given transmutation from old to new
void CryChunkedFile::MeshDesc::remapBoneIds(const int* pPermutation, unsigned numBones)
{
	VertBindArray::iterator it;
	for (it = this->arrVertBinds.begin(); it != this->arrVertBinds.end(); ++it)
		it->remapBoneIds((unsigned*)pPermutation, numBones);
}

// recalculates (if necessary) the normals of the mesh and returns
// the pointer tot he array of recalculated normals
void CryChunkedFile::MeshDesc::buildReconstructedNormals()
{
	// recalculate the normals
	this->arrNormals.resize (numVertices());
	memset (&this->arrNormals[0], 0, sizeof(Vec3d)*numVertices());
	for (unsigned nFace = 0; nFace < numFaces(); ++nFace)
	{
		const CryFace& Face = this->pFaces[nFace];
		Vec3d v1, v2, vTmpNormal;
		v1 = this->pVertices[Face.v0].p - this->pVertices[Face.v1].p;
		v2 = this->pVertices[Face.v0].p - this->pVertices[Face.v2].p;
		vTmpNormal = v1 ^ v2;
		this->arrNormals[Face.v0] += vTmpNormal;
		this->arrNormals[Face.v1] += vTmpNormal;
		this->arrNormals[Face.v2] += vTmpNormal;
	}
	for (unsigned nVertex=0; nVertex < numVertices(); ++nVertex)
	{
		Vec3d& vN = this->arrNormals[nVertex];
		float fLength = vN.Length();
		if (fLength < 1e-3)
			vN = Vec3d(0,1,0);
		else
			vN /= fLength;
	}
}


//////////////////////////////////////////////////////////////////////////
// validates the indices of the mesh. if there are some indices out of range,
// throws an appropriate exception
void CryChunkedFile::MeshDesc::validateIndices()
{
	for (unsigned nFace = 0; nFace < numFaces(); ++nFace)
	{
		const CryFace& Face = this->pFaces[nFace];
		if (Face.v0 < 0 || (unsigned)Face.v0 >= numVertices()
			||Face.v1 < 0 || (unsigned)Face.v1 >= numVertices()
			||Face.v2 < 0 || (unsigned)Face.v2 >= numVertices())
			throw Error("Face %d (v={%d,%d,%d}) has one or more indices out of range (%d vertices in the mesh)",
				nFace, Face.v0, Face.v1, Face.v2, numVertices());
	}


}
