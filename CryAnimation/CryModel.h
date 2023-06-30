/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Oscar Blasco
//	Taken over by Vladimir Kajalin, Andrey Honich
//  Taken over by Sergiy Migdalskiy
//
//  Contains the geometry management code for multi-LOD models, shader/material, bone info management
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_MODEL_HEADER_
#define _CRY_MODEL_HEADER_

#include "IPhysics.h"
#include "CryHeaders.h"
#include "CryBone.h"
#include "CryModEffector.h"
#include "CryModEffIKSolver.h"
#include "CryModEffAnimation.h"
#include "CryGeometryInfo.h"
#include "BoneLightBindInfo.h"
#include "CryAnimationInfo.h"

#include "CryModelAnimationContainer.h"

//----------------------------------------------------------------------
// CryModel class
//----------------------------------------------------------------------

class CryModelState;
class CryModelSubmesh;
class IStencilShadowConnectivity;

struct CCFAnimGeomInfo;
struct CCFBoneGeometry;
struct CCFBGBone;
struct CCFMorphTargetSet;
struct CCFCharLightDesc;

#define ENABLE_BONE_BBOXES 0

////////////////////////////////////////////////////////////////////////////////
// This class contain data which can be shared between different several models of same type.
// It loads and contains all geometry, materials, and also damage table.
// Damage table contains amount of damage for each bone.
// Also it contains default model state.
class CryModel :
	public CryModelAnimationContainer
{
friend class CryModelState;
friend class CryModelLoader;
friend class CryModelGeometryLoader;
friend class CryModelSubmesh;
friend class CryModelSubmeshGeometry;
public:  

	// returns the geometry of the given lod (0 is the highest detail lod)
	CryGeometryInfo* getGeometryInfo (unsigned  nLodLevel = 0);
	const CryGeometryInfo* getGeometryInfo (unsigned  nLodLevel = 0) const;
	
	// returns the number of levels of details.
	// 1 means there's only 0th (basic) level of details, or there are effectively no additional LODs
	unsigned numLODs ()const {return (unsigned)m_arrGeomInfo.size();}

	// this enum contains the 
	enum DamageAreaEnum
	{
		g_nDamageAreaDefault = 0,
		g_nDamageAreaHead    = 1,
		g_nDamageAreaTorso   = 2,
		g_nDamageAreaArmL    = 3,
		g_nDamageAreaArmR    = 4,
		g_nDamageAreaLegL    = 5,
		g_nDamageAreaLegR    = 6
	};

	// fills the array m_arrDamageTable, using the bone names
	void InitDamageTable();

	enum {g_nMaxMaterialCount = 128};

	// retrieves the pointer to the static array of shadow volume vertices
	//static float* getShadowVolumeVerts ();
	
	// expands the size of the shadow volume vertex array to the specified size
	// the size defines the number of floats the array must have (at least)
	//static void expandShadowVolumeVerts(unsigned nSize);

	// frees the array of shadow volume vertices; this should be normally called
	// in the destructor of CryModelManager, but can safely be called whenever you wish
	// since expand will always restore the array
	//static void freeShadowVolumeVerts( );

  void ComputeStaticBoundingBox();
	
	// Loads it from geom file; returns the number of bytes used from the chunk
  ///unsigned LoadFromGeom (const MESH_CHUNK_DESC_0744* pChunk, unsigned nChunkSize, float scale, const int nLodLevel, bool bTestForBoneInfo=true);
  
	bool LoadGeomChunks (const string& filename, float scale, int nLodLevel);

	//void LoadTextures(const string& );

	// computes, caches and returns the connectivity object for stencil shadows
	// (or just returns, if the object is already cached)
	IStencilShadowConnectivity* getStencilShadowConnectivity(unsigned nLOD);

	unsigned numLocalBoneLights() const {return (unsigned)m_arrLocalBoneLights.size();}
	unsigned numGlobalBoneLights() const {return (unsigned)m_arrGlobalBoneLights.size();}
	unsigned numBoneLights() const {return numLocalBoneLights()+numGlobalBoneLights();}
	CBoneLightBindInfo& getBoneLight(unsigned i)
	{
		if (i < numLocalBoneLights())
			return m_arrLocalBoneLights[i];
		else
		{
			assert(i < numGlobalBoneLights());
			return m_arrGlobalBoneLights[i-numLocalBoneLights()];
		}
	}
	CBoneLightBindInfo& getLocalBoneLight(unsigned i) {return m_arrLocalBoneLights[i];}
	CBoneLightBindInfo& getGlobalBoneLight(unsigned i) {return m_arrGlobalBoneLights[i];}

	void clearConstructionData();

	const Vec3& getModelOffset()const;
public:

  //////////////////////////////////////////////////////////////////////////////////////////
  // ----------------------------- GAME INTERFACE FUNCTIONS ----------------------------- //
  //////////////////////////////////////////////////////////////////////////////////////////
  CryModel (class CryCharBody* pBody, class CControllerManager* pControllerManager);
  virtual ~CryModel();

	// puts the size of the whole subsystem into this sizer object, classified,
	// according to the flags set in the sizer
	void GetSize(class ICrySizer* pSizer)const;

	const char* getFilePathCStr();

  void DrawStaticGeom();                    // Draws static geometry, useful to check the Geom
  //virtual void GetStaticBoundingBox(Vec3 * pvMin, Vec3 * pvMax);
	
	// returns the radius of the bounding sphere of the object
	float getStaticBSphereRadius() {return m_fStaticBSphereRadius;}

  void InvertMarkedTangentBasises();

	void ExportModelsASC();
	//DECLARE_ARRAY_GETTER_METHODS(MAT_ENTITY, Material, Materials, m_arrMaterials);
	size_t numMaterials ()const {return m_arrMaterials.size();}
	const MAT_ENTITY& getMaterial (unsigned i)const {return m_arrMaterials[i];}
	MAT_ENTITY& getMaterial (unsigned i) {return m_arrMaterials[i];}
	
	// Returns the interface for animations applicable to this model
	virtual ICryAnimationSet* GetAnimationSet ();

	// builds the skins out of morph targets
	void buildMorphSkins ();

	const CrySkinMorph& getMorphSkin (unsigned nLOD, int nMorphTargetId);

	// builds the skins for tangent base and geometry skin calculation for each LOD
	void buildGeomSkins();

	// builds all stencil shadow connectivities
	void buildStencilShadowInfos();

	// deletes all unused materials in the material array
	void deleteUnusedMaterials ();

	CryModelState * m_pDefaultModelState;

	// 1. completely initializes the CryModel from the given CCG file
	// 2. Loads textures
	// 3. Generates render arrays
	// returns true if successfully initialized
	bool initFromCCG (const string& strTextureDir, const string& strAnimationDir, class CCFMemReader& Reader, float fScale);

	// loads the LOD: geometry info and leaf buffers
	// nSize is the size of the buffer including header; the raw data follows the header immediately
	bool loadCCGLOD (unsigned nLOD, const CCFAnimGeomInfo* pHeader, unsigned nSize);

	// loads the bone geometry and initializes the physical geometry for corresponding LOD for bones
	bool loadCCGBoneGeomLOD (unsigned nLOD, float fScale, const CCFBoneGeometry* pHeader, unsigned nSize);

	// loads the morph target set from CCG; scales all morph targets
	bool loadCCGMorphTargetSet (const CCFMorphTargetSet* pData, unsigned nSize, float fScale);

	// loads the light array
	bool loadCCGLights (const CCFCharLightDesc* pData, unsigned nSize,float fScale);

	// loads the bone geometry for a particular bone
	bool loadCCGBoneGeom (IGeomManager* pGeomManager, unsigned nLOD, float fScale, const CCFBGBone* pHeader, unsigned nSize);

	// loads the animation list from the chunk
	bool loadCCGAnimScript (float fScale, string strAnimDir, const void* pData, unsigned nSize);

	// given the pointers to the chunk datas of the anim info chunks, loads those animations
	bool loadAnimations (float fScale, const std::vector<const struct CCFAnimInfo*>& arrAnimInfos);

	void loadCCGUserProperties (const char* pData, unsigned nSize);

	// fills the iGameID for each material, using its name to enumerate the physical materials in 3D Engine
	void fillMaterialGameId();

	CryBBoxA16* getBoneBBoxes ()
	{
#if ENABLE_BONE_BBOXES
		return m_arrBoneBBoxes.begin();
#else
		return NULL;
#endif
	}
	void computeBoneBBoxes();

	// returns the extra data attached to the character by the artist during exporting
	// as the scene user properties. if there's no such data, returns NULL
	const char* GetProperty(const char* szName);

protected:

	// finds all lod file names (starting 
	//void 

	// performs post-initialization tasks one-time after loading the model
	void LoadPostInitialize(bool bBoneInfoDefPoseNeedInitialization);

	void LoadPostInitializeCCG();

	CryCharBody* m_pBody;
protected:
	// the set of geometries for each lod level
	TFixedArray<CryGeometryInfo> m_arrGeomInfo;

	// these are the special user-defined scene properties exported from Max
	typedef std::map<string,string> UserPropMap;
	UserPropMap m_mapUserProps;

	// the radius of the bounding sphere of the object
	float   m_fStaticBSphereRadius;

	// this is the mapping BoneNumber->DamageExtermity
	// this is initialized only for the LOD 0
	// There are as many elements in this array, as there are bones at all
	// the values are from the enum DamageAreaEnum
	// Numbering by BoneID
	TFixedArray<unsigned char> m_arrDamageTable;

#if ENABLE_BONE_BBOXES
	typedef TAllocator16<CryBBoxA16> BBoxAllocatorA16;
	// the vertex stream: contains the rigid and smooth vertex structures
	typedef TFixedArray<CryBBoxA16, BBoxAllocatorA16> BBoxArrayA16;
	BBoxArrayA16 m_arrBoneBBoxes;
#endif

	// array of materials used to render this model
	// this is a fixed array because reallocations occur only on load and then the array is only read from
	std::vector <MAT_ENTITY> m_arrMaterials;  

	// local and global bone light and heat source bindings. the locals go first
	std::vector<CBoneLightBindInfo> m_arrLocalBoneLights, m_arrGlobalBoneLights;
	void addBoneLights (const std::vector<CBoneLightBindInfo>& arrLights);
	unsigned m_numLocalBoneLights;

	// offset of the model LCS - by this value, the whole model is offset
	Vec3 m_vModelOffset;

	bool m_bDeleteLBMats;
	bool m_bFromCCG;
	// the material physical game id that will be used as default for this character
	int m_nDefaultGameID;
protected: // static data

	// the following string tables are for recognition of different parts of the body
	// by the bone name. depending on this recognition, the damage will be calculated.
	// each array must end with the NULL string, to mark its end
	static const char* g_szDamageBonesHead[];
	static const char* g_szDamageBonesTorso[];
	static const char* g_szDamageBonesArmL[];
	static const char* g_szDamageBonesArmR[];
	static const char* g_szDamageBonesLegL[];
	static const char* g_szDamageBonesLegR[];

	// the static array of shadow volume vertices
	// this holds the vertices of a deformed characters between calls to Deform()
	// and functions that use its output; this avoids necessity to reallocate the array many times each frame
	//static TFixedArray<float> g_arrShadowVolumeVerts;
};

#endif // _CryModel_H_
