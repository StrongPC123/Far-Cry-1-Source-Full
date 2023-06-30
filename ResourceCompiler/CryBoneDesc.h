//////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//
//  File:CryBoneDesc.h
// 
//	History:
//	Created by Sergiy Migdalskiy 01/14/2003
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_BONE_DESC_HDR_
#define _CRY_BONE_DESC_HDR_

// this class contains the information that's common to all instances of bones
// in the given model: like bone name, and misc. shared properties
// We use a base structure to separate the bunch of parameters that get serialized
// together
#pragma pack(push,4)
template <class TBonePhysics>
struct TCryBoneDescData
{
	unsigned int m_nControllerID; // unic id of bone (generated from bone name in the max)

	// [Sergiy] physics info for different lods
	// lod 0 is the physics of alive body, lod 1 is the physics of a dead body
	TBonePhysics m_PhysInfo[2]; 
	float m_fMass;

	Matrix44 m_matInvDefGlobal; // allows to get difference to def pose matrices 

	int	m_nLimbId; // set by model state class

	// this bone parent is this[m_nOffsetParent], 0 if the bone is root. Normally this is <= 0
	int m_nOffsetParent;

	// The whole hierarchy of bones is kept in one big array that belongs to the ModelState
	// Each bone that has children has its own range of bone objects in that array,
	// and this points to the beginning of that range and defines the number of bones.
	unsigned m_numChildren;
	// the beginning of the subarray of children is at this[m_nOffsetChildren]
	// this is 0 if there are no children
	int m_nOffsetChildren;
};

typedef TCryBoneDescData<BONE_PHYSICS> CryBoneDescData;
// compatible structure
typedef TCryBoneDescData<BONE_PHYSICS_COMP> CryBoneDescData_Comp;

#define __copy(x) left.x = right.x

inline void CopyCryBone (CryBoneDescData_Comp& left, const CryBoneDescData& right)
{
	__copy(m_nControllerID);
	CopyPhysInfo (left.m_PhysInfo[0], right.m_PhysInfo[0]);
	CopyPhysInfo (left.m_PhysInfo[1], right.m_PhysInfo[1]);
	__copy(m_fMass);
	__copy(m_matInvDefGlobal);
	__copy(m_nLimbId);
	__copy(m_nOffsetParent);
	__copy(m_numChildren);
	__copy(m_nOffsetChildren);
}

inline void CopyCryBone (CryBoneDescData& left, const CryBoneDescData_Comp& right)
{
	__copy(m_nControllerID);
	CopyPhysInfo (left.m_PhysInfo[0], right.m_PhysInfo[0]);
	CopyPhysInfo (left.m_PhysInfo[1], right.m_PhysInfo[1]);
	__copy(m_fMass);
	__copy(m_matInvDefGlobal);
	__copy(m_nLimbId);
	__copy(m_nOffsetParent);
	__copy(m_numChildren);
	__copy(m_nOffsetChildren);
}

#undef __copy

#pragma pack(pop)



class CryBoneDesc: public CryBoneDescData
{
public:
	CryBoneDesc ();
	~CryBoneDesc ();

	// returns the bone name, if available
	const char* getNameCStr()const;
	const string& getName()const;

	unsigned getControllerId()const {return m_nControllerID;}

	// sets the name of the bone out of the given buffer of the given max size
	void setName (const char* szName);

	unsigned numChildren ()const {return m_numChildren;}
	bool hasParent() const {return m_nOffsetParent != 0;}
	int getParentIndexOffset()const {return m_nOffsetParent;}
	int getFirstChildIndexOffset() const {return m_nOffsetChildren;}

	const Matrix44& getInvDefGlobal() const {return m_matInvDefGlobal;}
	void setDefaultGlobal(const Matrix44& mxDefault);
	int getLimbId () const {return m_nLimbId;}

	BONE_PHYSICS& getPhysInfo (int nLod) {return m_PhysInfo[nLod];}

	// updates this bone physics, from the given entity descriptor, and of the given lod
	void UpdatePhysics (const BONE_ENTITY& entity, int nLod);
	void setPhysics (int nLod, const BONE_PHYSICS& BonePhysics)
	{
		assert (nLod >= 0 && nLod < sizeof(m_PhysInfo)/sizeof(m_PhysInfo[0]));
		m_PhysInfo[nLod] = BonePhysics;
	}
	// the physics for the given LOD is not available
	void resetPhysics (int nLod)
	{
		assert (nLod >= 0 && nLod < sizeof(m_PhysInfo)/sizeof(m_PhysInfo[0]));
		memset (&m_PhysInfo[nLod], 0, sizeof(m_PhysInfo[nLod]));
	}
	const BONE_PHYSICS& getPhysics (int nLod)const
	{
		assert (nLod >= 0 && nLod < sizeof(m_PhysInfo)/sizeof(m_PhysInfo[0]));
		return m_PhysInfo[nLod];
	}

	// Returns the id of the bone mesh chunk from which the bone physical geometry
	// should be taken. The caller should take this id, find the corresponding chunk in the CCG/CGF file
	// and construct physical geometry using IGeomManager::CreateMesh.
	// Then, register it with RegisterGeometry(). It will return phys_geometry* that the caller
	// should put into the corresponding LOD m_PhysInfo.pPhysGeom
	// CryBoneInfo::PostInitPhysGeom uses this same id to find the physical geometry in the map
	INT_PTR getPhysGeomId (unsigned nLOD) {return (INT_PTR)m_PhysInfo[nLOD].pPhysGeom;}

	// compares two bone descriptions and returns true if they're the same bone
	// (the same name and the same position in the hierarchy)
	bool isEqual(const CryBoneDesc& desc)const;

	// Serializes the description:
	// returns the number of required bytes for serialization, if the data pointer is NULL
	// returns 0 (if the buffer size is not enough) or the number of bytes written, if the data pointer is given
	unsigned Serialize (bool bSave, void *pStream, unsigned nSize);

	// scales the bone with the given multiplier
	void scale (float fScale);

protected:

	// loads the bone from a raw chunk data (no header)
	// returns false if the bone couldn't be loaded
	bool LoadRaw (const BONE_ENTITY* pEntity);

	//! Performs post-initialization. This step is requred to initialize the pPhysGeom of the bones
	//! After the bone has been loaded but before it is first used. When the bone is first loaded, pPhysGeom
	//! is set to the value equal to the chunk id in the file where the physical geometry (BoneMesh) chunk is kept.
	//! After those chunks are loaded, and chunk ids are mapped to the registered physical geometry objects,
	//! call this function to replace pPhysGeom chunk ids with the actual physical geometry object pointers.
	//!	NOTE:
	//!	The entries of the map that were used are deleted
	typedef std::map<INT_PTR, struct phys_geometry*> ChunkIdToPhysGeomMap;
	bool PostInitPhysGeom (ChunkIdToPhysGeomMap& mapChunkIdToPhysGeom, int nLodLevel);

	friend class CryBoneHierarchyLoader;

protected:
	string m_strName;
};

#endif