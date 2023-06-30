////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2003.
// -------------------------------------------------------------------------
//  File name:   CryChunkedFile.h
//  Version:     v1.00
//  Created:     13.01.2003 by Sergiy.
//  Compilers:   Visual Studio.NET
//  Description: CryChunkedFile structure with substructures. It contains
//               everything about c?f file, in ready parsed form
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __crychunkedfile_h__
#define __crychunkedfile_h__

#include "ChunkFileReader.h"
#include "CryVertexBinding.h"
#include "CryBoneHierarchyLoader.h"


// The mesh bone links contain bone indices (not ids)
// The CryBoneDesc array contains the physics in LOD0 only
// 
struct CryChunkedFile:public _reference_target_t
{
	class Error
	{
	public:
		Error (const char* szFormat, ...);
		string strDesc;
	};

	// parses the given chunked file into the internal structure;
	// outputs warnings if some of the chunks are incomplete or otherwise broken
	CryChunkedFile(CChunkFileReader* pFile);

	// ChunkType_Timing
	// NULL if no timing chunk was found
	const TIMING_CHUNK_DESC* pTiming;
	// the range entities following the timing chunk. There are (pTiming->nSubRanges) of them
	const RANGE_ENTITY* pRangeEntities;

	// the file header
	const FILE_HEADER* pFileHeader;

	// the file type
	int GetFileType ()const {return /*(FileTypes)*/pFileHeader->FileType;}

	// the array of nodes
	struct NodeDesc
	{
		NodeDesc ():pDesc(NULL), pChildren (NULL){}
		NodeDesc (const NODE_CHUNK_DESC*);

		const NODE_CHUNK_DESC* pDesc;
		// array of chunk ids; there are pDesc->nChildren of them
		const int* pChildren;
		// the property string
		string strProps;

		// array of children nodes (superfluous)
		std::vector <NodeDesc*> arrChildren;
		// pointer to the parent node (superfluous)
		NodeDesc* pParent;

		Matrix44 getWorldTransform()const 
		{
			if (pParent)
				return pDesc->tm * pParent->getWorldTransform();
			else
				return pDesc->tm;
		}
	};
	std::vector<NodeDesc> arrNodes;
	
	// map: chunk id (node chunk id) -> index in arrNodes array
	typedef std::map<unsigned, unsigned> UintUintMap;
	typedef UintUintMap NodeIdxMap;
	NodeIdxMap mapNodeIdx,mapObjectNodeIdx;
	// map of light chunks
	typedef std::map<unsigned, const LIGHT_CHUNK_DESC*> LightMap;
	LightMap mapLights;

	// returns node pointer by the node chunk id; if chunkid is not node , returns NULL
	virtual NodeDesc* GetNodeDesc (unsigned nChunkId);
	// returns node pointer by the object (to which the node refers, and which should refer back to node) id
	// if can't find it, returns NULL
	virtual NodeDesc* GetObjectNodeDesc (unsigned nObjectId);

	// returns light pointer by the light chunk id.
	// returns NULL on failure
	virtual const LIGHT_CHUNK_DESC* GetLightDesc (unsigned nChunkId);

	// given a correct array of links and array of bones (with indices synchronized)
	// calculates for each bone description a bounding box
	void computeBoneBBoxes();


	// the structure describing the morph target
	struct MorphTargetDesc
	{
		unsigned numMorphVertices;
		const SMeshMorphTargetVertex* pMorphVertices;
		string strName;
	};

	// NOTE: the bone ids in the CryLinks are the bone indices after this all has been loaded (remapped
	// from within adjust() function)
	struct MeshDesc
	{
		MeshDesc (): pDesc (NULL), pNode (NULL) {}

		MeshDesc (const MESH_CHUNK_DESC* pChunk, unsigned nSize);

		// back-reference to the node referencing to this mesh (superfluous)
		const NodeDesc* pNode;

		const MESH_CHUNK_DESC* pDesc;

		// array of pDesc->nVerts vertices
		unsigned numVertices() const {return (unsigned)pDesc->nVerts;}
		const CryVertex* pVertices;

		// array of pDesc->nFaces faces
		unsigned numFaces() const {return (unsigned)pDesc->nFaces;}
		const CryFace* pFaces;

		// array of pDesc->nTVerts
		unsigned numUVs() const {return (unsigned)pDesc->nTVerts;}
		const CryUV* pUVs;

		// array of texture faces; present only when there are UVs
		unsigned numTexFaces () const {return (unsigned)(pDesc->nTVerts?pDesc->nFaces:0);}
		const CryTexFace* pTexFaces;

		// answers the question: has this mesh chunk bone info (links with offsets and weights for each vertex)?
		// if yes, arrVertBinds will contain them
		bool hasBoneInfo()const {return this->pDesc->HasBoneInfo;}
		// the actual binding to bones
		typedef std::vector<CryVertexBinding> VertBindArray;
		VertBindArray arrVertBinds;
		CryVertexBinding& getLink(unsigned i) {return arrVertBinds[i];}

		// answers the question: has this mesh vertex colors?
		bool hasVertColors()const {return this->pDesc->HasVertexCol;}
		const CryIRGB* pVColors;

		// remaps the bone ids using the given transmutation from old to new
		void remapBoneIds(const int* pPermutation, unsigned numBones);

		// validates the indices of the mesh. if there are some indices out of range,
		// throws an appropriate exception
		void validateIndices();

		// recalculates the normals of the mesh and returns
		// the pointer tot he array of recalculated normals
		void buildReconstructedNormals();

		// this is the array of normals calculated on demand
		std::vector<Vec3d> arrNormals;

		std::vector<MorphTargetDesc>arrMorphTargets;
	};
	std::vector<MeshDesc> arrMeshes;
	std::vector<MeshDesc> arrBoneMeshes;

	struct MorphTargetChunk
	{
		const MESHMORPHTARGET_CHUNK_DESC_0001* pData;
		unsigned nSize;
	};
	std::vector<MorphTargetChunk> m_arrMorphTargetChunks;

	// map: from mesh ChunkId to index in arrMeshes array (or arrBoneMeshes for mapBoneMeshIdx)
	typedef UintUintMap MeshIdxMap;
	MeshIdxMap mapMeshIdx, mapBoneMeshIdx;

	// returns mesh pointer by the mesh chunk id
	// returns NULL if error
	virtual MeshDesc* GetMeshDesc (unsigned nChunkId);

	// returns mesh pointer by the mesh chunk id
	// returns NULL if error
	virtual MeshDesc* GetBoneMeshDesc (unsigned nChunkId);

	std::vector<MAT_ENTITY> arrMtls;

	virtual bool DoesMtlCastShadow(int nMtl);

	// the array of bone names from the corresponding chunk ; these get assigned to the CryBoneDesc's
	// in the final step (adjust())
	std::vector<const char*> arrNames;

	// the bone loader, providing info about the bones
	CryBoneHierarchyLoader Bones;

	// bone-light bindings
	unsigned m_numBoneLightBinds;
	const SBoneLightBind* m_pBoneLightBind;

	bool IsBoneInitialPosPresent() {return Bones.hasInitPos();}

	unsigned numSceneProps;
	const SCENEPROP_ENTITY* pSceneProps;

	// chunk parsers, each parses its chunk into the internal structures of this object
protected:
	void addChunkTiming (const CHUNK_HEADER& chunkHeader, const TIMING_CHUNK_DESC* pChunkData, unsigned nChunkSize);
	void addChunkNode (const CHUNK_HEADER& chunkHeader, const NODE_CHUNK_DESC* pChunkData, unsigned nChunkSize);
	void addChunkLight (const CHUNK_HEADER& chunkHeader, const LIGHT_CHUNK_DESC* pChunkData, unsigned nChunkSize);
	void addChunkMesh (const CHUNK_HEADER& chunkHeader, const MESH_CHUNK_DESC* pChunkData, unsigned nChunkSize);
	void addChunkBoneMesh (const CHUNK_HEADER& chunkHeader, const MESH_CHUNK_DESC* pChunkData, unsigned nChunkSize);
	void addChunkMaterial (const CHUNK_HEADER& chunkHeader, const void* pChunkData, unsigned nChunkSize);
	void addChunkBoneNameList (const CHUNK_HEADER& chunkHeader, const void* pChunkData, unsigned nChunkSize);
	void addChunkBoneAnim (const CHUNK_HEADER& chunkHeader, const BONEANIM_CHUNK_DESC* pChunkData, unsigned nChunkSize);
	void addChunkBoneInitialPos (const CHUNK_HEADER& chunkHeader, const BONEINITIALPOS_CHUNK_DESC_0001* pChunkData, unsigned nChunkSize);
	void addChunkMeshMorphTarget (const CHUNK_HEADER& chunkHeader, const MESHMORPHTARGET_CHUNK_DESC_0001* pChunkData, unsigned nChunkSize);
	void addChunkBoneLightBinding (const CHUNK_HEADER& chunkHeader, const BONELIGHTBINDING_CHUNK_DESC_0001* pChunkData, unsigned nChunkSize);
	void addChunkSceneProps(const CHUNK_HEADER& chunkHeader, const SCENEPROPS_CHUNK_DESC*pChunkData, unsigned nChunkSize);

	// this is postprocess: after all structures are in place, this function
	// sets the superfuous pointers/etc
	void adjust();
protected:
	// releases all the internally referenced structures
	// the clients of this class should release it via Release() method
	virtual ~CryChunkedFile();

	CChunkFileReader_AutoPtr m_pFile;
};

TYPEDEF_AUTOPTR (CryChunkedFile);

#endif