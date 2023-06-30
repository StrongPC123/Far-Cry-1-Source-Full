/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	08/28/2002 - Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//  Contains:
//  Declaration of CryGeometryInfo, a structure holding geometry in the form that is  to
//  the data structure of CGF
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_GEOMETRY_INFO_HEADER_
#define _CRY_GEOMETRY_INFO_HEADER_

class CIndexedMesh;
class IStencilShadowConnectivity;

#include "CryVertexBinding.h"
#include "CrySkinFull.h"
#include "CrySkinRigidBasis.h"
#include "CrySkinBuilderBase.h"
#include "GeomCommon.h"

struct CCFMaterialGroup;

struct CCFIntFace;

// This class holds the geometry information (presumably read from a CGF file)
// Including: vertices, faces, tangents, vertex mapping(s), bone bindings, texture coordinates
// TODO: normal constructor/destructor,
// maybe some loading operators,
// normal TFixedArray<> arrays
// normal getters/setters
class CryGeometryInfo
{
public:
	CryGeometryInfo();
	~CryGeometryInfo();

	// number of elements in the m_rDupVertToNorVert array
	//unsigned int m_nNumDupVerts;

	void PrepareLinks (int numVertices);

	// after constructing the skins etc. some data is not ever more used
	// this data can be cleared here
	void clearConstructionData();

	// this is the number of vertices that are really used by the faces
	unsigned numUsedVertices()const;

	bool hasSkin()const;

	void GetSize (ICrySizer* pSizer)const;

	// the primitive groups and the indices, as they appear in the vertex buffer
	std::vector<CCFMaterialGroup> m_arrPrimGroups;
	std::vector<unsigned short> m_arrIndices;
	// the triples for each face, internal indexation
	
	typedef std::vector<GeomFace> FaceArray;
	FaceArray m_arrFaces;
	std::vector<GeomMtlID> m_arrFaceMtl;
	GeomMtlID getFaceMtl (unsigned i)const {return m_arrFaceMtl[i];}

	size_t numFaces()const {return m_arrFaces.size();}
	GeomFace getFace (unsigned i)const {return m_arrFaces[i];}
	const GeomFace* getFaces ()const {return &m_arrFaces[0];}

	// creates a fake array of faces; need to be deleted[]
	CryFace* newCryFaces()const;

	void setNumUsedVertices(unsigned num) {m_numUsedVertices = num;}

	void exportASC (FILE* f);
protected:
	// connectivity object that gets pre-computed for each model once and then
	// used to extract edge topology by the stencil shadowing module
	IStencilShadowConnectivity* m_pStencilShadowConnectivity;

	// the skinner for the vertices
	CrySkinFull m_SkinGeom, m_SkinNormal;


	// the skinner for the tangent bases
	CrySkinRigidBasis m_TangSkin;

	// array of vertices
	typedef TFixedArray<Vec3> Vec3dArray;
	typedef TElementaryArray<Vec3> Vec3dElementaryArray;
	Vec3dElementaryArray m_arrVertices, m_arrNormals;
	// tangent data - it's always 4 * numVertices()
	TFixedArray<TangData> m_arrExtTangents;
	// geometry faces with indices in the m_arrVertices array
	unsigned m_numUsedVertices;
	// texture faces. This array is either empty, or of the same size as m_arrFaces
	typedef TElementaryArray<CryTexFace> TexFaceArray;
	TexFaceArray m_arrTexFaces;
	// UVs of the model. If there are texture faces, the indices in them refer to this array
	TFixedArray<CryUV> m_arrUVs;
	// UVs of the vertex buffer - external indexation
	TElementaryArray<CryUV> m_arrExtUVs;

	// internal indexation vertex colors
	std::vector<DWORD> m_arrVColors;

	// array of vertices: for each vertex, there is an array of bone links
	// there may be 0 or numVertices() links
	typedef TElementaryArray<CryVertexBinding> VertexBindingArray;
	VertexBindingArray m_arrLinks;

	// array - mapping from the "duplicated" vertices (that the renderer generates) to the "normal" vertices (that getVertices() returns)
	TFixedArray<unsigned> m_arrExtToIntMap;

	// back mapping (inverse of ExtToInt)
	TElementaryArray<unsigned> m_arrIntToExtMap;

	// format of the vertex buffer in system memory now (m_arrVertBuf)
	int m_nVertBufFormat;
	TElementaryArray<char> m_arrVertBuf;
public:
	// returns the vertex buffer of the given format; in this buffer, UVs will
	// already be set up properly and extra fields like color will be NULLed
	char* getVertBuf (int nVertFormat);
	/*
	void selfValidate()
#ifndef _DEBUG
	{}
#endif
	;
	*/

	// returns the cached connectivity object for stencil shadows
	class IStencilShadowConnectivity* getStencilShadowConnectivity(const std::vector<MAT_ENTITY>&arrMaterials);

	//////////////////////////////////////////////////////////////////////////
	// builds the connectivity object for stencil shadows
	// PARAMETERS:
	//  iEdgeBuilder - the builder to use to create the connectivity info
	//  pbCastShadow - the array of flags, 1 flag per 1 material, if the flag is true, then this material casts shadow, otherwise not
	//  numMaterials - number of items in the pbCastShadow array
	void buildStencilShadowConnectivity (const std::vector<MAT_ENTITY>&arrMaterials); // 

	void OutputOrphanedEdgeWarning(class IEdgeConnectivityBuilder *iEdgeBuilder, const std::vector<MAT_ENTITY>&arrMaterials);

	// deserializes the stencil shadow connectivity object
	void setStencilShadowConnectivity (IStencilShadowConnectivity* pConnectivity);

	bool hasGeomSkin()const;
	class CrySkinFull* getGeomSkin();
	class CrySkinFull* getNormalSkin();
	void buildGeomSkins(const class CryBoneInfo* pBoneInfo, unsigned numBoneInfos);
	bool loadVertexSkin (const void* pData, unsigned nSize);
	bool loadNormalSkin (const void* pData, unsigned nSize);
	bool loadTangSkin (const void* pData, unsigned nSize);	

	// the tangent bases are calculated in pieces, each piece calculating some number of vertices
	// the piece
	CrySkinRigidBasis* getTangSkin();
	
	// builds the tangent base skins
	void buildTangSkins (const class CryBoneInfo* pBoneInfo, unsigned numBoneInfos);

	// sorts the faces by the material indices.
	// in the given array, sets elements corresponding to used materials to true; doesn't touch the other elements
	// (i.e. the array elements must be set to false before calling this function)
	// IMPORTANT: Keeps the original order of faces in each material group
	void sortFacesByMaterial (std::vector<bool>& arrUsedMtls);

	// creates an instance of the CIndexedMesh with the info from this geometry info.
	// this is needed to create the leaf buffers (while GenerateRenderArrays() execution)
	CIndexedMesh* new3DEngineIndexedMesh()const;

	// Allocates the array that keeps the vertices
	void PrepareVertices (unsigned numVertices);
	// Allocates the face array
	void MakeIntFaces(const CryFace* pCryFaces, unsigned numFaces);
	// Allocattes the texture coordinate array
	void PrepareUVs (int numUVs);
	// Allocates texture face array
	void PrepareTexFaces (int numTexFaces);
	// Allocates the external to internal map entries
	void PrepareExtToIntMap (int numExtVertices);
	// Create the Internal-to-external map
	void CreateIntToExtMap ();

	// loads the geometry from the chunk
	// returns the number of bytes read from the chunk (normally must be the whole chunk size; 0 means failure)
	unsigned Load (unsigned nLOD, const MESH_CHUNK_DESC_0744* pChunk, unsigned nChunkSize);

	// optimize the vertex order for skinning
	void sortVertices();

	// scales the model. Multiplies all vertex coordinates by the given factor
	void Scale (float fScale);

	// rotates the model; transforms each vector with the specified matrix, using only its
	// ROTATIONAL components (no translation)
	void rotate (const Matrix44& tm);

	// transforms the model with the given transform matrix
	void transform (const Matrix44& tm);

	// Checks the face indices, if they are in range (0..numVertices())
	// returns true upon successful validation, false means there are indices out of range
	bool ValidateFaceIndices ();

	// Checks the texture face indices, if they are in range (0..numUVs())
	// returns true upon successful validation, false means there are indices out of range
	bool ValidateTexFaceIndices ();

	// calculates the bounding box for the object
	// if the bbox can't be calculated (e.g. the skin hasn't been yet loaded), returns false
	bool computeBBox (CryAABB& bb);

	// forces the material indices to be in the range [0..numMaterials]
	void limitMaterialIDs(unsigned numMaterials);

	// recalculates all the normals, assuming smooth geometry (non-smooth vertices were already split in the exporter)
	void recalculateNormals();

	// remaps the ids of the bones in the CryLink structures
	void remapBoneIds (const unsigned* arrBoneIdMap, unsigned numBoneIds);


	// remaps the ids of materials. arrMtlIdMap[nOldMtlId] == nNewMtlId
	void remapMtlIds (const unsigned* arrMtlIdMap, unsigned numMtlIds);

	// returns true if the geometry is degraded (e.g. no vertices or no faces)
	bool empty();

	// creates the array of UVs that can be directly copied into videobuffer (no index mapping required, external indexation)
	// this relies on the pExtToInt mapping for the number of entries
	void initExtUVs (const class CVertexBufferUVArrayDriver& pUVs);

	// creates and copies the data into the new face array
	void initFaces (unsigned numFaces, const CCFIntFace* pFaces, const void* pMtls);

	// creates and initializes the array of extern UVs
	void initExtUVs (const CryUV* pUVs, unsigned numUVs);

	// initializes the ext-to-int map
	void initExtToIntMap (const unsigned short* pExtToIntMap, unsigned numEntries);


	// after strip-generation of the mesh, rearranges the geometry info to match the sequence of vertices
	// in the stripified mesh
	void remapVerticesToExt ();

	// creates (allocates with new[]) an array of bindings for each normal
	// there are numVertices() elements in this array
	CryVertexBinding* newNormalLinks (const CryBoneInfo* pBoneInfo);

	// Getter methods for vertices
	DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(Vec3, Vertex, Vertices, m_arrVertices)
	unsigned numVertices() const {return m_numUsedVertices;}
	DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(Vec3, Normal, Normals, m_arrNormals)
	unsigned numNormals() const {return m_arrNormals.empty()?0:m_numUsedVertices;}
	// Getter methods for tangent basis
	DECLARE_ARRAY_GETTER_METHODS(TangData, ExtTangent, ExtTangents, m_arrExtTangents)
	// Getter methods for faces
	DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(CryTexFace, TexFace, TexFaces, m_arrTexFaces)
	unsigned numTexFaces() const;
	// destroys the texture faces
	void removeTexFaces();
	// Getter methods for the UVs
	DECLARE_ARRAY_GETTER_METHODS(CryUV, UV, UVs, m_arrUVs);
	// destroys the UV array
	void removeUVs();

	// Getter methods for the external indexed UVs
	DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(CryUV, ExtUV, ExtUVs, m_arrExtUVs);
	unsigned numExtUVs() const {return m_arrExtUVs.empty()?0:numExtToIntMapEntries();}
	unsigned numExtVertices() const {return numExtToIntMapEntries();}

	// Getter methods for the links
	DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(CryVertexBinding, Link, Links, m_arrLinks);
	unsigned numLinks() const;

	// Getter/setter methods for the dup->nor mapping
	DECLARE_ARRAY_GETTER_METHODS(unsigned, ExtToIntMapEntry, ExtToIntMapEntries, m_arrExtToIntMap);
	// Getter/setter methods for the nor->dup mapping
	//unsigned numIntToExtMapEntries()const {return numVertices();}
	//DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(unsigned, IntToExtMapEntry, IntToExtMapEntries, m_arrIntToExtMap);
};



//////////////////////////////////////////////////////////////////////////
//
//  Declaration and implementation of skin sources, implementations
//  of an interface used to supply skin builder with vertex info.
//
//////////////////////////////////////////////////////////////////////////
// this class will just pass the number of vertices and links from the geometry through
class CrySkinVertexSource: public ICrySkinSource
{
public:
	CrySkinVertexSource (CryGeometryInfo* pGeometry):
		ICrySkinSource (
			pGeometry->getLinks(),
			pGeometry->numLinks(),
			pGeometry->getVertices(),
			pGeometry->numVertices(),
			pGeometry->getExtTangents(),
			pGeometry->numExtTangents(),
			pGeometry->getExtToIntMapEntries()
			)
	{}

	~CrySkinVertexSource ()
	{	/* no need to deallocate any stuff*/ }
};

//////////////////////////////////////////////////////////////////////////
// this class will create and hold the links for actual normals
class CrySkinNormalSource: public ICrySkinSource
{
public:	
	CrySkinNormalSource (CryGeometryInfo* pGeometry, const CryBoneInfo* pBoneInfo):
		ICrySkinSource (
			pGeometry->newNormalLinks(pBoneInfo),
			pGeometry->numLinks(),
			pGeometry->getVertices(),
			pGeometry->numVertices(),
			pGeometry->getExtTangents(),
			pGeometry->numExtTangents(),
			pGeometry->getExtToIntMapEntries()
			)
	{
	}

	~CrySkinNormalSource()
	{
		delete [] m_pLinks;
		m_pLinks = NULL;
	}
};


#endif
