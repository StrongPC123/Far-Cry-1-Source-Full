#include "stdafx.h"
#include "MathUtils.h"
#include "CryBoneDesc.h"
#include "StringUtils.h"

using namespace CryStringUtils;

CryBoneDesc::CryBoneDesc()
{
	m_PhysInfo[0].pPhysGeom = m_PhysInfo[1].pPhysGeom = 0;
	m_matInvDefGlobal.SetIdentity();; // allows to get difference to def pose matrices 
}

CryBoneDesc::~CryBoneDesc()
{
}

//////////////////////////////////////////////////////////////////////////
// loads the bone from a raw chunk data (no header)
// PARAMETERS:
//   pEntity - the chunk data to load the bone info from
// RETURNS:
//   false if the bone couldn't be loaded
bool CryBoneDesc::LoadRaw (const BONE_ENTITY* pEntity)
{
  //read  bone info
	assert(pEntity->nChildren<200);

	// update the lod 0 physics of this bone
	UpdatePhysics (*pEntity, 0);

	//get bone info
	m_nControllerID = pEntity->ControllerID;

	return true;
}


//////////////////////////////////////////////////////////////////////////
// updates this bone physics, from the given entity descriptor, and of the given lod
void CryBoneDesc::UpdatePhysics (const BONE_ENTITY& entity, int nLod)
{
	assert (nLod >= 0 && nLod < SIZEOF_ARRAY(m_PhysInfo));
	CopyPhysInfo(m_PhysInfo[nLod], entity.phys);

	int nFlags = 0;
	if (entity.prop[0])
	{
		nFlags = joint_no_gravity|joint_isolated_accelerations;
	}
	else
	{
		if (!strnstr(entity.prop,"gravity", sizeof(entity.prop)))
			nFlags |= joint_no_gravity;
		
		if (!strnstr(entity.prop,"physical",sizeof(entity.prop)))
			nFlags |= joint_isolated_accelerations;
	}

	m_PhysInfo[nLod].flags |= nFlags;
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
bool CryBoneDesc::PostInitPhysGeom (ChunkIdToPhysGeomMap& mapChunkIdToPhysGeom, int nLodLevel)
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

// sets the name of the bone out of the given buffer of the given max size
void CryBoneDesc::setName (const char* szName)
{
	m_strName.assign (szName);

	static const char *g_arrLimbNames[4] = { "L UpperArm","R UpperArm","L Thigh","R Thigh" };
	m_nLimbId = -1;
	for (int j=0; j < SIZEOF_ARRAY(g_arrLimbNames) ;j++)
		if (strstr(szName,g_arrLimbNames[j]))
		{
			m_nLimbId = j;
			break;
		}
}

// returns the bone name, if available
const char* CryBoneDesc::getNameCStr()const
{
	return m_strName.c_str();
}

const string& CryBoneDesc::getName()const
{
	return m_strName;
}

// compares two bone descriptions and returns true if they're the same bone
// (the same name and the same position in the hierarchy)
bool CryBoneDesc::isEqual (const CryBoneDesc& desc)const
{
	return m_strName == desc.m_strName
		&& m_nControllerID == desc.m_nControllerID
		&& m_nOffsetParent == desc.m_nOffsetParent
		&& m_numChildren == desc.m_numChildren
		&& m_nOffsetChildren == desc.m_nOffsetChildren;
}


inline size_t align4 (size_t x)
{
	return (x + 3)&~3;
}

// Serializes the description:
// returns the number of required bytes for serialization, if the data pointer is NULL
// returns 0 (if the buffer size is not enough) or the number of bytes written, if the data pointer is given
unsigned CryBoneDesc::Serialize (bool bSave, void *pStream, unsigned nSize)
{
	if (bSave)
	{
		unsigned nSizeRequired = (unsigned)(sizeof(CryBoneDescData_Comp) + align4 (m_strName.length()+1));
		if (!pStream)
			return nSizeRequired;
		if (nSize < nSizeRequired)
			return 0;

		CopyCryBone(*(CryBoneDescData_Comp*)pStream, *this);
		memcpy ((CryBoneDescData_Comp*)pStream+1,m_strName.c_str(),m_strName.length()+1);
		return nSizeRequired;
	}
	else
	{
		if (!pStream)
			return 0;
		if (nSize < sizeof(CryBoneDescData_Comp)+1)
			return 0;
		if (nSize & 3)
			return 0; // alignment error
		CopyCryBone(*this, *(CryBoneDescData_Comp*)pStream);
		// end of the stream
		const char* pEnd = (const char*)pStream + nSize;
		// the start byte of the bone name
		const char* pName = (const char*)((CryBoneDescData_Comp*)pStream+1);

		// scan until the end of the name (\0 char) or the stream (pEnd address)
		const char *pNameEnd;
		for (pNameEnd = pName; pNameEnd < pEnd && *pNameEnd; ++pNameEnd);
		m_strName.assign (pName, pNameEnd);
		// return aligned size of the chunk including the string 0 terminator, but
		// no more than the declared size of the stream
		return min(nSize, (unsigned)align4 (pNameEnd+1-(const char*)pStream));
	}
}

void CryBoneDesc::setDefaultGlobal(const Matrix44& matDefault)
{
	m_matInvDefGlobal=OrthoUniformGetInverted (matDefault);
}


// scales the bone with the given multiplier
void CryBoneDesc::scale (float fScale)
{
	m_matInvDefGlobal.SetTranslationOLD(m_matInvDefGlobal.GetTranslationOLD()*fScale);
}
