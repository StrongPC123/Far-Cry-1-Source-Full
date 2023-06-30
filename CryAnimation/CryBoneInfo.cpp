#include "stdafx.h"
#include "CryBoneInfo.h"
#include <StlUtils.h>
#include "ControllerManager.h"
#include "StringUtils.h"

CryBoneInfo::CryBoneInfo()
	//:m_arrControllers ("CryBoneInfo.Controllers")
{
}

CryBoneInfo::~CryBoneInfo()
{
	IPhysicalWorld *pIPhysicalWorld = GetPhysicalWorld();
	IGeomManager* pPhysGeomManager = pIPhysicalWorld?pIPhysicalWorld->GetGeomManager():NULL;

	for(int nLod=0; nLod<2; nLod++)
	if (m_PhysInfo[nLod].pPhysGeom && (INT_PTR)m_PhysInfo[nLod].pPhysGeom!=-1) 
	{
		if ((INT_PTR)m_PhysInfo[nLod].pPhysGeom<0x400) 
		{
			TRACE("Error: trying to release wrong bone phys geometry");
		} 
		else if (pPhysGeomManager)
		{
			phys_geometry* pPhysGeom = m_PhysInfo[nLod].pPhysGeom;
			pPhysGeomManager->UnregisterGeometry(pPhysGeom);
		}
		else
			TRACE("todo: delete bones phys");
	}
}

//////////////////////////////////////////////////////////////////////////
// binds this bone to a controller from the specified animation, using
// the controller manager
// NOT RECURSIVE anymore
// PARAMETERS:
//  nAnimID       - local for this bone animation ID (will be used when
//                  subsequently referring to the animation)
//  nGlobalAnimID - global animation id, identifies the animation within
//                  the controller manager context
// RETURNS:
//  The binded controller, if the bind was successful, or NULL if there's
//  no such controller (animation is missing)
//////////////////////////////////////////////////////////////////////////
IController* CryBoneInfo::BindController(CControllerManager::Animation& GlobalAnim, unsigned nAnimID)
{
	if (m_arrControllers.size() <= nAnimID)
		// this is an autopointer array, so there's no need to re-initialize the autopointers after construction
		m_arrControllers.resize (nAnimID+1/*, NULL*/); 

	IController* pController = m_arrControllers[nAnimID] = GlobalAnim.GetController (m_nControllerID);

	return pController;
}

// unbinds the bone from the given animation's controller
void CryBoneInfo::UnbindController (unsigned nAnimID)
{
	if (m_arrControllers.size() > nAnimID)
		m_arrControllers[nAnimID] = NULL;
}

unsigned CryBoneInfo::sizeofThis()const
{
	unsigned nSize = sizeof(*this);
	nSize += sizeofVector(m_arrControllers);
	return nSize;
}


//////////////////////////////////////////////////////////////////////////
// Updates the given lod level bone physics info from the bones found in
// the given chunk.
// THis is required to update the dead body physics info from the lod1 geometry
// without rebuilding the new bone structure. If the lod1 has another bone structure,
// then the bones are mapped to the lod0 ones using controller ids. Matching bones'
// physics info is updated
void CryBoneInfo::UpdateHierarchyPhysics (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize, int nLodLevel)
{
	UnsignedToCryBoneMap mapCtrlId;
	AddHierarchyToControllerIdMap(mapCtrlId);

	// the first bone entity
	const BONE_ENTITY* pBoneEntity = (const BONE_ENTITY*)(pChunk+1);
	// the actual end of the chunk
	const BONE_ENTITY* pBoneEntityEnd = (const BONE_ENTITY*)(((const char*)pChunk)+nChunkSize);
	
	// if you get this assert, it means the lod 1 file is corrupted
	if (pBoneEntity + pChunk->nBones > pBoneEntityEnd)
	{
		assert (0);
		return;
	}

	// update physics for each bone entity
	for (; pBoneEntity < pBoneEntityEnd; ++pBoneEntity)
	{
		CryBoneInfo* pBone = find_in_map(mapCtrlId, pBoneEntity->ControllerID, (CryBoneInfo*)NULL);
		if (pBone)
			pBone->UpdatePhysics(*pBoneEntity, nLodLevel);
	}
}

//////////////////////////////////////////////////////////////////////////
// adds this bone and all its children to the given map controller id-> bone ptr
void CryBoneInfo::AddHierarchyToControllerIdMap (UnsignedToCryBoneMap& mapControllerIdToCryBone)
{
	mapControllerIdToCryBone.insert (UnsignedToCryBoneMap::value_type(m_nControllerID, this));
	for (unsigned nChild = 0; nChild < numChildren(); ++nChild)
	{
		getChild(nChild)->AddHierarchyToControllerIdMap(mapControllerIdToCryBone);
	}
}

//! Performs post-initialization. This step is requred to initialize the pPhysGeom of the bones
//! After the bone has been loaded but before it is first used. When the bone is first loaded, pPhysGeom
//!   is set to the value equal to the chunk id in the file where the physical geometry (BoneMesh) chunk is kept.
//! After those chunks are loaded, and chunk ids are mapped to the registered physical geometry objects,
//!   call this function to replace pPhysGeom chunk ids with the actual physical geometry object pointers.
//! RETURNS:
//    true if the corresponding physical geometry object has been found
//!	NOTE:
//!	The entries of the map that were used are deleted
bool CryBoneInfo::PostInitPhysGeom (ChunkIdToPhysGeomMap& mapChunkIdToPhysGeom, int nLodLevel)
{
	phys_geometry*& pPhysGeom = m_PhysInfo[nLodLevel].pPhysGeom;
	ChunkIdToPhysGeomMap::iterator it = mapChunkIdToPhysGeom.find ((INT_PTR)pPhysGeom);
	if (it != mapChunkIdToPhysGeom.end()) 
	{
		// remap the chunk id to the actual pointer to the geometry
		pPhysGeom = it->second;
		mapChunkIdToPhysGeom.erase (it);
		return true;
	} 
	else
	{
		pPhysGeom = NULL;
		return false;
	}
}

void CryBoneInfo::PostInitialize()
{
	if (getParent())
	{
		CryQuat qrel = CryQuat((matrix3x3in4x4Tf&)getParent()->getInvDefGlobal() * ((matrix3x3in4x4Tf&)getInvDefGlobal()).T());
		m_pqDefRelTransform.vRotLog = -log(qrel).v; // '-' since pqTransform.buildMtarix assumes flipped quaternion
		m_pqDefRelTransform.vPos = getParent()->getInvDefGlobal().TransformPointOLD(
			(matrix3x3in4x4f&)getInvDefGlobal() * -getInvDefGlobal().GetTranslationOLD());

		CryBoneInfo *pParent,*pPhysParent;
		for(int nLod=0; nLod<2; nLod++)
		{
			for(pParent=pPhysParent=getParent(); pPhysParent && !pPhysParent->m_PhysInfo[nLod].pPhysGeom; pPhysParent=pPhysParent->getParent());
			if (pPhysParent)
				m_qRelPhysParent[nLod] = CryQuat((matrix3x3in4x4Tf&)pParent->getInvDefGlobal() * (matrix3x3in4x4f&)pPhysParent->getInvDefGlobal());
			else
				m_qRelPhysParent[nLod].SetIdentity();
		}
	}
	else
	{
		m_pqDefRelTransform.vRotLog.Set(0,0,0);
		m_pqDefRelTransform.vPos.Set(0,0,0);
	}
}

CryBoneInfo& CryBoneInfo::operator = (const CryBoneDesc& rThat)
{
	*static_cast<CryBoneDesc*>(this) = rThat;
	m_arrControllers.clear();
	return *this;
}