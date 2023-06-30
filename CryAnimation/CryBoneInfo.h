//////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//
//  File:CryBoneInfo.h
// 
//	History:
//	Created by Sergiy Migdalskiy 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_BONE_INFO_HDR_
#define _CRY_BONE_INFO_HDR_

#include "Controller.h"
#include "CryAnimationInfo.h"
#include "CryBoneDesc.h"

// this class contains the information that's common to all instances of bones
// in the given model: like bone name, and misc. shared properties
class CryBoneInfo: public CryBoneDesc
{
public:
	CryBoneInfo ();
	~CryBoneInfo ();

	CryBoneInfo* getChild (unsigned i) {assert(i < numChildren()); return this + m_nOffsetChildren + i;}
	const CryBoneInfo* getChild (unsigned i) const {assert(i < numChildren()); return this + m_nOffsetChildren + i;}
	CryBoneInfo* getParent () {return m_nOffsetParent ? this + m_nOffsetParent : NULL;}
	const CryBoneInfo* getParent () const {return m_nOffsetParent ? this + m_nOffsetParent : NULL;}

	// binds this bone to a controller from the specified animation, using the controller manager
	class IController* BindController (GlobalAnimation& GlobalAnim, unsigned nAnimID);
	// unbinds the bone from the given animation's controller
	void UnbindController (unsigned nAnimID);

	CryBoneInfo& operator = (const CryBoneDesc& rThat);
	unsigned sizeofThis()const;

	void PostInitialize();
	IController::PQLog &getDefRelTransform() { return m_pqDefRelTransform; }
	CryQuat &getqRelPhysParent(int nLod) { return m_qRelPhysParent[nLod]; }

protected:
	// updates the given lod level bone physics info from the bones found in the given chunk
	void UpdateHierarchyPhysics (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize, int nLodLevel);

	typedef std::map<unsigned, CryBoneInfo*> UnsignedToCryBoneMap;
	// adds this bone and all its children to the given map controller id-> bone ptr
	void AddHierarchyToControllerIdMap (UnsignedToCryBoneMap& mapControllerIdToCryBone);

	//! Performs post-initialization. This step is requred to initialize the pPhysGeom of the bones
	//! After the bone has been loaded but before it is first used. When the bone is first loaded, pPhysGeom
	//! is set to the value equal to the chunk id in the file where the physical geometry (BoneMesh) chunk is kept.
	//! After those chunks are loaded, and chunk ids are mapped to the registered physical geometry objects,
	//! call this function to replace pPhysGeom chunk ids with the actual physical geometry object pointers.
	//!	NOTE:
	//!	The entries of the map that were used are deleted
	typedef std::map<int, struct phys_geometry*> ChunkIdToPhysGeomMap;
	bool PostInitPhysGeom (ChunkIdToPhysGeomMap& mapChunkIdToPhysGeom, int nLodLevel);

	friend class CryBone;
	friend class CryModelState;
	friend class CryModelAnimationContainer;
	friend class CryModelGeometryLoader;

protected:
	// the array of controllers is formed only once at the load time, so we can actually use FixedArray here
	typedef std::vector<IController_AutoPtr>ControllerArray;
	ControllerArray m_arrControllers;
	IController::PQLog m_pqDefRelTransform;
	CryQuat m_qRelPhysParent[2];
};

#endif