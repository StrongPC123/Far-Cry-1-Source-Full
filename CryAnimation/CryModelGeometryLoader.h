/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//
//  Contains the geometry load code for CryModel class
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_MODEL_GEOMETRY_LOADER_HDR_
#define _CRY_MODEL_GEOMETRY_LOADER_HDR_

#include "CryBoneInfo.h"
class CryModel;
#include "CryBoneHierarchyLoader.h"

//////////////////////////////////////////////////////////////////////////
// this is just a helper class for CryModelLoader.
// it parses the CGF file and extracts the useful information in the useful form
//
// NOTE:
//  to load different LODs, use the same instance of this class. Between LOD
//  loading, it may preserve important information
class CryModelGeometryLoader
{
	// class CryModel is the only client of this class for now
public:
	CryModelGeometryLoader ();
	~CryModelGeometryLoader ();

	// frees all resources allocated within this class
	void clear();
	
	// parses the given file, returns true if successful
	bool load (CryModel* pModel, CChunkFileReader* pReader, unsigned nLOD, float fScale);

	// returns true if the bone infos have the initial position set
	bool hasBoneInfoInitPos();
protected:
	// loads the materials; returns true if succeeds
	bool loadMaterials();

	bool prepare();
	bool finalize();

	// load the stuff that doesn't need anything else to be already loaded:
	// Bone name table, bone-light binding table, light and node maps,
	// calculate the number of morph targets
	bool loadStage1();

	// loads the stuff that needs the stuff loaded at stage 1
	bool loadStage2();

	// loads the morph targets - after the other things are known
	bool loadMorphTargets();

	// corrects the bone ids in the geometry info according to the new numeration of the bones
	bool rebindBoneIndices();

	// loads the bone name list into m_arrGeomBoneNameTable
	// m_arrGeomBoneNameTable points to the chunk data places where the actual names are; so no need to deallocate the strings
	bool loadBoneNameList (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize);

	// loads the bone light binding array
	bool loadBoneLightBinding (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize);

	// loads the bone light binding array
	bool loadBoneInitialPos (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize);

	// Attempts to load the material from the given material chunk
	// loads the material from the material chunk to the given structure.
	// returns true if the chunk was recognized and the material was loaded
	bool loadMaterial (CHUNK_HEADER* pMaterialChunk, unsigned nChunkSize, MAT_ENTITY& rMaterial, class CMatEntityNameTokenizer& mt);

	bool loadBoneMesh (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize);
	bool loadBoneAnim (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize);
	bool loadBones (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize);
	// for LODs > 0, builds the m_arrTempBoneIdToIndex by matching the bone names
	// against the bone names in the already loaded LOD 0
	void buildBoneIndicesByNames();
	// loads the mesh
	bool loadMesh(const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize);

	// returns the file path tot he file being currently loaded
	const char* getFilePathCStr()const;

	void indicateProgress (const char* szMsg = NULL);
protected:
	// this will contain the information about the loaded bones, if any
	CryBoneHierarchyLoader m_BoneLoader;

	// the number of elements in the arrGeomBoneNameTable is the number of bone name entities
	// this will be the array of bone names, extracted right from the file (the pointers will point into the mapped file)
	std::vector<const char*> m_arrGeomBoneNameTable;

	// this is the array of bone-light binding
	unsigned m_numBoneLightBinds;
	const SBoneLightBind* m_pBoneLightBind;

	// this map contains the bone geometry. The mapping is : [Chunk ID]->[Physical geometry read from that chunk]
	// the chunks from which the physical geometry is read are the bone mesh chunks.
	typedef CryBoneInfo::ChunkIdToPhysGeomMap ChunkIdToPhysGeomMap;
	ChunkIdToPhysGeomMap m_mapLimbPhysGeoms;

	// the map of lights
	std::map <unsigned, const LIGHT_CHUNK_DESC*> m_mapLights;
	// the map of object nodes - pointers to the node chunks for ObjectIDs
	std::map <unsigned, const NODE_CHUNK_DESC*> m_mapObjectNodes;

	TFixedArray<unsigned> m_arrTempBoneIdToIndex;

	// the number of morph target chunks
	unsigned m_numMorphTargets;

	// the basis number of materials: by this value, the material ids are shifted in the mesh
	int m_nFirstMat;

	bool m_bFirstTimeWarningBoneGeometryMtl; // if set to true, we can issue this warning and set it to false; when it's false, we don't issue this warning to keep the extra warning number low
  bool m_bMeshFound;     // mesh chunk found in the file
  bool m_bGeometryFound; // mesh chunk found in the file
	bool m_bBonesFound;    // bone animation chunk found in the file
	unsigned m_nGeometryChunkID; // the id used to bind the morph targets

	IGeomManager *m_pPhysicalGeometryManager;

	CChunkFileReader_AutoPtr m_pReader;
	CryModel* m_pModel;
	unsigned m_nLOD;
	float m_fScale;
};

#endif