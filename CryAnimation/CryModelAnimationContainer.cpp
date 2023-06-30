#include "stdafx.h"
#include <CryCompiledFile.h>
#include <stlutils.h>
//#include "CryAnimation.h"
#include "ControllerManager.h"
#include "CryModelAnimationContainer.h"
#include "CVars.h"
#include "CryBoneInfo.h"
#include "StringUtils.h"
#include "CryAnimationInfo.h"
#include "CryGeomMorphTarget.h"
#include "CrySkinMorph.h"
using namespace CryStringUtils;

CryModelAnimationContainer::CryModelAnimationContainer (CControllerManager * pControllerManager):
	m_pControllerManager(pControllerManager),
	m_arrBones ("CryModelAnimationContainer.BoneInfo"),
	m_arrMorphTargets ("CryModelAnimationContainer.MorphTargets")
{
	m_pControllerManager->Register(this);
}

CryModelAnimationContainer::~CryModelAnimationContainer()
{
	// before releasing the animations, we should release the controllers from bones
	m_arrBones.clear();
	// now there are no controllers referred to by this object, we can release the animations
	for (AnimationArray::iterator it = m_arrAnimations.begin(); it != m_arrAnimations.end(); ++it)
		m_pControllerManager->AnimationRelease(it->nGlobalAnimId, this);
	m_pControllerManager->Unregister(this);
}

void CryModelAnimationContainer::OnAnimationGlobalUnload(int nGlobalAnimId)
{
	std::vector<LocalAnimId>::iterator it = std::lower_bound (m_arrAnimByGlobalId.begin(), m_arrAnimByGlobalId.end(), nGlobalAnimId, AnimationGlobIdPred(m_arrAnimations));
	
	assert (it == m_arrAnimByGlobalId.begin() || m_arrAnimations[(it-1)->nAnimId].nGlobalAnimId < nGlobalAnimId);

	for (;it != m_arrAnimByGlobalId.end() && m_arrAnimations[it->nAnimId].nGlobalAnimId == nGlobalAnimId; ++it)
	{
		AUTO_PROFILE_SECTION(g_dTimeAnimBindControllers);
		// bind loaded controllers to the bones
		for (unsigned nBone = 0; nBone < numBoneInfos(); ++nBone)
		{
			CryBoneInfo& rBoneInfo = getBoneInfo(nBone);
			rBoneInfo.UnbindController (it->nAnimId);
		}
	}
}

void CryModelAnimationContainer::OnAnimationGlobalLoad (int nGlobalAnimId)
{
	std::vector<LocalAnimId>::iterator it = std::lower_bound (m_arrAnimByGlobalId.begin(), m_arrAnimByGlobalId.end(), nGlobalAnimId, AnimationGlobIdPred(m_arrAnimations));

	assert (it == m_arrAnimByGlobalId.begin() || m_arrAnimations[(it-1)->nAnimId].nGlobalAnimId < nGlobalAnimId);

	// scan through the sequence of animations with the given Global AnimId
	// and bind each of them to the bones
	if (it != m_arrAnimByGlobalId.end() && m_arrAnimations[it->nAnimId].nGlobalAnimId == nGlobalAnimId)
	{
		AUTO_PROFILE_SECTION(g_dTimeAnimBindControllers);
		GlobalAnimation& GlobalAnim = m_pControllerManager->GetAnimation (nGlobalAnimId);

		m_fTicksPerFrame  = (float)GlobalAnim.nTicksPerFrame;
		m_fSecsPerTick  = (float)GlobalAnim.fSecsPerTick;
		float fSecsPerFrame = m_fSecsPerTick * m_fTicksPerFrame;
		float fStart = GlobalAnim.rangeGlobal.start * fSecsPerFrame;
		float fStop = GlobalAnim.rangeGlobal.end * fSecsPerFrame;

		do {
			AnimData &LocalAnim = m_arrAnimations[it->nAnimId];  
			assert (LocalAnim.nGlobalAnimId == nGlobalAnimId);
			LocalAnim.fStart = fStart;
			LocalAnim.fStop  = fStop;

			if (GlobalAnim.IsLoaded())
			// bind loaded controllers (if any) to the bones
			for (unsigned nBone = 0; nBone < numBoneInfos(); ++nBone)
			{
				CryBoneInfo& rBoneInfo = getBoneInfo(nBone);
				rBoneInfo.BindController (GlobalAnim, it->nAnimId);
			}
		} while(++it != m_arrAnimByGlobalId.end() && m_arrAnimations[it->nAnimId].nGlobalAnimId == nGlobalAnimId);
	}
}

// adds an animation record to the animation set
void CryModelAnimationContainer::RegisterAnimation(const char * szFileName, int nGlobalAnimId, const char* szAnimName)
{
	const CControllerManager::Animation& GlobalAnim = m_pControllerManager->GetAnimation (nGlobalAnimId);
	m_fTicksPerFrame  = (float)GlobalAnim.nTicksPerFrame;
	m_fSecsPerTick  = (float)GlobalAnim.fSecsPerTick;
	float fSecsPerFrame = m_fSecsPerTick * m_fTicksPerFrame;
	float fStart = GlobalAnim.rangeGlobal.start * fSecsPerFrame;
	float fStop = GlobalAnim.rangeGlobal.end * fSecsPerFrame;
	
	AnimData LocalAnim; 
  LocalAnim.fStart = fStart;
  LocalAnim.fStop  = fStop;

  LocalAnim.bLoop = stristr(szFileName, "loop") != NULL;
	LocalAnim.nGlobalAnimId = nGlobalAnimId;
	LocalAnim.strName = szAnimName;
  
	// hack to fix random goto default pose problem
  if(LocalAnim.fStop <= LocalAnim.fStart) 
    LocalAnim.fStop = LocalAnim.fStart+1.0f/30.0f;

  if(LocalAnim.fStart > LocalAnim.fStop)
  {
		g_GetLog()->LogToFile ("  Invalid start(%g) > stop(%g) values in animation: %s", LocalAnim.fStart, LocalAnim.fStop, LocalAnim.strName.c_str());
		return;
	}
	int nLocalAnimId = m_arrAnimations.size();
  m_arrAnimations.push_back(LocalAnim);
	m_arrAnimByGlobalId.insert (std::lower_bound(m_arrAnimByGlobalId.begin(), m_arrAnimByGlobalId.end(), nGlobalAnimId, AnimationGlobIdPred(m_arrAnimations)), LocalAnimId(nLocalAnimId));
	m_arrAnimByLocalName.insert (std::lower_bound(m_arrAnimByLocalName.begin(), m_arrAnimByLocalName.end(), szAnimName, AnimationNamePred(m_arrAnimations)), nLocalAnimId);
	selfValidate();
 
	// The caller MUST TAKE CARE to bind the animation if it's already loaded before it has registered itself within this manager
	m_pControllerManager->AnimationAddRef(nGlobalAnimId, this);
}

//////////////////////////////////////////////////////////////////////////
// Loads animation file. Returns the global anim id of the file, or -1 if error
// SIDE EFFECT NOTES:
//  THis function does not put up a warning in the case the animation couldn't be loaded.
//  It returns an error (false) and the caller must process it.
int CryModelAnimationContainer::LoadCAF(const char * szFileName, float fScale, int nAnimID, const char * szAnimName, unsigned nGlobalAnimFlags)
{
	int nGlobalAnimID;
 
	int nLocalAnimId = findAnimation (szAnimName);
 
	if (nLocalAnimId == -1) 
	{
		AUTO_PROFILE_SECTION(g_dTimeAnimLoadFile);
		// find already loaded controllers
		nGlobalAnimID = m_pControllerManager->StartLoadAnimation (szFileName, fScale, nGlobalAnimFlags);

		if (nGlobalAnimID < 0)
			return nGlobalAnimID;

		m_pControllerManager->GetAnimation(nGlobalAnimID).nFlags |= nGlobalAnimFlags;

		RegisterAnimation(szFileName, nGlobalAnimID, szAnimName);

		if (!g_GetCVars()->ca_AnimationDeferredLoad() || (nGlobalAnimFlags&GlobalAnimation::FLAGS_DISABLE_DELAY_LOAD))
		{
			if (g_GetCVars()->ca_Debug() && (g_GetCVars()->ca_AnimationUnloadDelay()>0 || !(nGlobalAnimFlags&GlobalAnimation::FLAGS_DISABLE_AUTO_UNLOAD)))
				g_GetLog()->LogWarning ("\003Performance penalty: animation deferred load is disabled, but automatic animation unload is enabled. This will most surely lead to poor performance upon loading the level (because everything is loaded, more memory is used and massive precalculations are needed) and even some time after (because of animation controllers trashing) (animation %s)", szAnimName);
			// deferred load is disabled, try to load this animation immediately

			m_pControllerManager->LoadAnimation (nGlobalAnimID);
		}
	}
	else
	{
		nGlobalAnimID = m_arrAnimations[nLocalAnimId].nGlobalAnimId;
		g_GetLog()->LogError ("\003Trying to load animation with alias \"%s\" from file \"%s\" into the animation container. Such animation alias already exists and uses file \"%s\". Please use another animation alias.",
			szAnimName,
			szFileName,
			m_pControllerManager->GetAnimation(nGlobalAnimID).strFileName.c_str());
	}

  return nGlobalAnimID;
}

// Returns the index of the animation in the set, -1 if there's no such animation
int CryModelAnimationContainer::Find (const char* szAnimationName)
{
	if (szAnimationName[0] == '#')
		return int(m_arrAnimations.size() + findMorphTarget (szAnimationName));
	else
		return findAnimation(szAnimationName);
}


//! Returns the index of the morph target in the set, -1 if there's no such morph target
int CryModelAnimationContainer::FindMorphTarget (const char* szMorphTarget)
{
	return findMorphTarget(szMorphTarget);
}

// returns the index of the animation
int CryModelAnimationContainer::findAnimation (const char*szAnimationName)
{
	int nResult = -1;
	std::vector<int>::iterator it = std::lower_bound(m_arrAnimByLocalName.begin(), m_arrAnimByLocalName.end(), szAnimationName, AnimationNamePred(m_arrAnimations));
	if (it != m_arrAnimByLocalName.end() && !stricmp(m_arrAnimations[*it].strName.c_str(),szAnimationName))
		nResult = *it;

#ifdef _DEBUG
	int nTestResult = -1;
	AnimationArray::iterator itTest = m_arrAnimations.begin(), itTestEnd = m_arrAnimations.end();

  for (; itTest != itTestEnd; ++itTest)
    if(!stricmp(szAnimationName, itTest->strName.c_str()))
		{
      nTestResult = itTest - m_arrAnimations.begin();
			break;
		}

	assert (nTestResult == nResult);
#endif

  return nResult;
}

// returns the index of the morph target, in the indexation of the array of morph targets
int CryModelAnimationContainer::findMorphTarget (const char* szMorphTargetName)
{
	for (int i = 0; i < (int)m_arrMorphTargets.size(); ++i)
		if (!stricmp(m_arrMorphTargets[i].getNameCStr(), szMorphTargetName))
			return i;
	return -1;
}


// Returns the number of animations in this set
int CryModelAnimationContainer::Count()
{
	return (int)(m_arrAnimations.size() + m_arrMorphTargets.size());
}

//! Returns the number of morph targets in the set
int CryModelAnimationContainer::CountMorphTargets()
{
	return (int)m_arrMorphTargets.size();
}


// Returns the given animation length, in seconds
float CryModelAnimationContainer::GetLength (int nAnimationId)
{
	if ((unsigned)nAnimationId < numAnimations())
	{
		m_pControllerManager->LoadAnimationInfo(m_arrAnimations[nAnimationId].nGlobalAnimId);
		return m_arrAnimations[nAnimationId].getLength();
	}
	else
		return 0;
}

//! Returns the given animation's start, in seconds; 0 if the id is invalid
float CryModelAnimationContainer::GetStart (int nAnimationId)
{
	if ((unsigned)nAnimationId < numAnimations())
	{
		m_pControllerManager->LoadAnimationInfo(m_arrAnimations[nAnimationId].nGlobalAnimId);
		return m_arrAnimations[nAnimationId].fStart;
	}
	else
		return 0;
}


// Returns the given animation name
const char* CryModelAnimationContainer::GetName (int nAnimationId)
{
	if (nAnimationId >=0)
	{
		if (nAnimationId < (int)m_arrAnimations.size())
			return m_arrAnimations[nAnimationId].strName.c_str();
		nAnimationId -= (int)m_arrAnimations.size();
		if (nAnimationId < (int)m_arrMorphTargets.size())
			return m_arrMorphTargets[nAnimationId].getNameCStr();
		return "!ANIMATION ID OUT OF RANGE!";
	}
	else
		return "!NEGATIVE ANIMATION ID!";
}

//! Returns the name of the morph target
const char* CryModelAnimationContainer::GetNameMorphTarget (int nMorphTargetId)
{
	if (nMorphTargetId< 0)
		return "!NEGATIVE MORPH TARGET ID!";
	if ((unsigned)nMorphTargetId >= m_arrMorphTargets.size())
		return "!MORPH TARGET ID OUT OF RANGE!";
	return m_arrMorphTargets[nMorphTargetId].getNameCStr();
}


// Retrieves the animation loop flag
bool CryModelAnimationContainer::IsLoop (int nAnimationId)
{
	if ((unsigned)nAnimationId < numAnimations())
	{
		m_pControllerManager->LoadAnimationInfo(m_arrAnimations[nAnimationId].nGlobalAnimId);
		return m_arrAnimations[nAnimationId].bLoop;
	}
	else
		return false;
}


// updates the physics info of the given lod from the given bone animation descriptor
void CryModelAnimationContainer::UpdateRootBonePhysics (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize, int nLodLevel)
{
	if (numBoneInfos())
		getRootBoneInfo().UpdateHierarchyPhysics (pChunk, nChunkSize, nLodLevel);
}

// finds the bone by its name; returns the index of the bone, or -1 if not found
// TODO: for performance, we could once build a map name->index
int CryModelAnimationContainer::findBone (const char* szName)const
{
#ifdef _DEBUG
	int nTestResult = -1;
	TFixedArray<CryBoneInfo>::const_iterator it = m_arrBones.begin(), itEnd = it + m_arrBones.size();
	for (; it != itEnd; ++it)
	{
		if (!strcmp(it->getNameCStr(), szName))
		{
			nTestResult = it - m_arrBones.begin();
			break;
		}
	}
#endif

	int nResult = find_in_map(m_mapBoneNameIndex, szName, -1);
#ifdef _DEBUG
	assert (nResult == nTestResult);
#endif
	return nResult;
}

void CryModelAnimationContainer::onBonesChanged()
{
	m_mapBoneNameIndex.clear();
	for (unsigned i = 0; i < m_arrBones.size(); ++i)
		m_mapBoneNameIndex[m_arrBones[i].getNameCStr()] = i;
}

void CryModelAnimationContainer::onBonePhysicsChanged()
{
	for (unsigned i = 0; i < m_arrBones.size(); ++i)
		getBoneInfo(i).PostInitialize();
}


//! Performs post-initialization. This step is requred to initialize the pPhysGeom of the bones
//! After the bone has been loaded but before it is first used. When the bone is first loaded, pPhysGeom
//! is set to the value equal to the chunk id in the file where the physical geometry (BoneMesh) chunk is kept.
//! After those chunks are loaded, and chunk ids are mapped to the registered physical geometry objects,
//! call this function to replace pPhysGeom chunk ids with the actual physical geometry object pointers.
//!	NOTE:
//!	The entries of the map that were used are deleted upon exit
bool CryModelAnimationContainer::PostInitBonePhysGeom (ChunkIdToPhysGeomMap& mapChunkIdToPhysGeom, int nLodLevel)
{
	// map each bone,
	// and set the hasphysics to 1 if at least one bone has recognized and mapped its physical geometry
	bool bHasPhysics = false;
	for (unsigned i=0; i < numBoneInfos(); ++i)
	{
		if (getBoneInfo(i).PostInitPhysGeom (mapChunkIdToPhysGeom, nLodLevel))
			// yes, this model has the physics
			bHasPhysics = true;
	}
	return bHasPhysics;
}


//! Modifies the animation loop flag
void CryModelAnimationContainer::SetLoop (int nAnimationId, bool bIsLooped)
{
	if ((unsigned)nAnimationId < numAnimations())
	{
		m_pControllerManager->LoadAnimationInfo(m_arrAnimations[nAnimationId].nGlobalAnimId);
		m_arrAnimations[nAnimationId].bLoop = bIsLooped;
	}
}

// returns the reference to the given animation , or the default animation, if the animation
// id is out of range
const AnimData &CryModelAnimationContainer::getAnimation (int nAnimationId) const
{
	if ((size_t)nAnimationId < m_arrAnimations.size())
		return m_arrAnimations[nAnimationId];
	else
	{
		static const AnimData DefaultAnimation;
		return DefaultAnimation;
	}
}

// records statistics about the given animation (the id is the same as for getAnimation()):
// should be called upon ticking the animation
void CryModelAnimationContainer::OnAnimationTick(int nAnimationId)
{
	m_pControllerManager->OnTickAnimation(getAnimation(nAnimationId).nGlobalAnimId);
}
void CryModelAnimationContainer::OnAnimationApply(int nAnimationId)
{
	m_pControllerManager->OnApplyAnimation(getAnimation(nAnimationId).nGlobalAnimId);
}
// records statistics about the given animation (the id is the same as for getAnimation()):
// should be called upon starting the animation
void CryModelAnimationContainer::OnAnimationStart(int nAnimationId)
{
	m_pControllerManager->OnStartAnimation(getAnimation(nAnimationId).nGlobalAnimId);
}


// returns the reference to the given morph target, or the default morph target if the
// morph target id is out of range
const CryGeomMorphTarget& CryModelAnimationContainer::getMorphTarget (int nMorphTargetId) const
{
	if ((size_t)nMorphTargetId < m_arrMorphTargets.size())
		return m_arrMorphTargets[nMorphTargetId];
	else
	{
		assert (0);// this will lead to a memory leak, if it is called
		static const CryGeomMorphTarget DefaultMorphTarget;
		return DefaultMorphTarget;
	}
}

// returns the number of morph targets
unsigned CryModelAnimationContainer::numMorphTargets() const
{
	return (unsigned)m_arrMorphTargets.size(); 
}

// resets the morph target array, and allocates the given number of targets, with the given number of LODs each
void CryModelAnimationContainer::reinitMorphTargets (unsigned numMorphTargets, unsigned numLODs)
{
	m_arrMorphTargets.reinit(numMorphTargets);
}

// prepares to load the specified number of CAFs by reserving the space for the controller pointers
void CryModelAnimationContainer::prepareLoadCAFs (unsigned nReserveAnimations)
{
	nReserveAnimations += (unsigned)m_arrAnimations.size();
	for (unsigned i = 0; i < numBoneInfos(); ++i)
	{
		CryBoneInfo::ControllerArray& arrControllers = getBoneInfo(i).m_arrControllers;
		arrControllers.reserve (nReserveAnimations);
	}
	m_arrAnimations.reserve (nReserveAnimations);
	m_arrAnimByGlobalId.reserve (nReserveAnimations);
	m_arrAnimByLocalName.reserve (nReserveAnimations);
}

//! Deserializes the bones from the CCF chunk using serialization function from CryBoneInfo
//! THe serialized data follows the given header; the total size (including the header) is passed
bool CryModelAnimationContainer::loadCCGBones (const CCFBoneDescArrayHeader*pHeader, unsigned nSize)
{
	unsigned nBone, numBones = pHeader->numBones;
	if (numBones > 1000)
		return false; // too many bones

	m_arrBones.reinit (numBones);

	const char* pBoneData = (const char*)(pHeader+1);
	const char* pDataEnd = (const char*)pHeader+ nSize;

	for (nBone = 0; nBone < numBones; ++nBone)
	{
		if (pBoneData >= pDataEnd)
			return false;

		CryBoneInfo& rBone = m_arrBones[nBone];
		unsigned nReadBytes = rBone.Serialize(false, (void*)pBoneData, pDataEnd - pBoneData);
		// we don't use the information from these pointers right now, because we bind physical geometry using bone id
		rBone.m_PhysInfo[0].pPhysGeom = rBone.m_PhysInfo[1].pPhysGeom = NULL;
		// we need the bone name to be in the lookup map
		m_mapBoneNameIndex[rBone.getNameCStr()] = nBone;

		if (!nReadBytes)
			return false;

		pBoneData += nReadBytes;

		if (pBoneData > pDataEnd)
			return false;
	}

	if (pBoneData != pDataEnd)
		g_GetLog()->LogWarning ("\003CryModelAnimationContainer::deserializeBones: %d bytes left unread in the bone chunk", pDataEnd - pBoneData);

	return true;
}


// scales the skeleton (its initial pose)
void CryModelAnimationContainer::scaleBones (float fScale)
{
	for (unsigned nBone = 0; nBone < numBoneInfos(); ++nBone)
		m_arrBones[nBone].scale (fScale);
}


void CryModelAnimationContainer::selfValidate()
{
#ifdef _DEBUG
	assert (m_arrAnimByGlobalId.size() == m_arrAnimations.size());
	assert (m_arrAnimByLocalName.size() == m_arrAnimations.size());
	for (unsigned i = 0; i < m_arrAnimByGlobalId.size()-1; ++i)
	{
		assert (m_arrAnimations[m_arrAnimByGlobalId[i].nAnimId].nGlobalAnimId <= m_arrAnimations[m_arrAnimByGlobalId[i+1].nAnimId].nGlobalAnimId);
		assert (stricmp(m_arrAnimations[m_arrAnimByLocalName[i]].strName.c_str(), m_arrAnimations[m_arrAnimByLocalName[i+1]].strName.c_str())<0);
	}
#endif
}

size_t CryModelAnimationContainer::sizeofThis()const
{
	size_t nSize = sizeof(*this);
	for (AnimationArray::const_iterator itAnim = m_arrAnimations.begin(); itAnim != m_arrAnimations.end(); ++itAnim)
		nSize += itAnim->sizeofThis();
	nSize += sizeofArray(m_arrAnimByGlobalId);
	nSize += sizeofArray(m_arrAnimByLocalName);

	for (unsigned nBone = 0; nBone < m_arrBones.size(); ++nBone)
		nSize += m_arrBones[nBone].sizeofThis();

	for (unsigned nMorphSkin = 0; nMorphSkin < m_arrMorphSkins.size(); ++nMorphSkin)
		nSize += m_arrMorphSkins[nMorphSkin].sizeofThis();

	for (unsigned nMorphTarget = 0; nMorphTarget < m_arrMorphTargets.size(); ++nMorphTarget)
		nSize += m_arrMorphTargets[nMorphTarget].sizeofThis();

	nSize += sizeofArray(m_arrTempBoneIdToIndex);
	for (std::set<string>::const_iterator it = m_setDummyAnimations.begin(); it != m_setDummyAnimations.end(); ++it)
		nSize += it->capacity() + sizeof(*it);
	return nSize;
}

// returns the number of animations that aren't shared
unsigned CryModelAnimationContainer::numUniqueAnimations()
{
	unsigned numResult =0;
	for (AnimationArray::const_iterator it = m_arrAnimations.begin(); it != m_arrAnimations.end(); ++it)
		if (m_pControllerManager->GetAnimation(it->nGlobalAnimId).nRefCount == 1)
			++numResult;

	return numResult;
}

const AnimData* CryModelAnimationContainer::getAnimationInfo(unsigned i)
{
	if(m_pControllerManager->LoadAnimationInfo(m_arrAnimations[i].nGlobalAnimId))
		return &m_arrAnimations[i];
	else
		return NULL;
}

//! Unloads animation from memory
//! The client must take into account that the animation can be shared and, if unloaded, the other
//! character models (animation sets) will have to load it back to use.
void CryModelAnimationContainer::UnloadAnimation (int nAnimId)
{
	if ((unsigned)nAnimId < m_arrAnimations.size())
	{
		m_pControllerManager->UnloadAnimation(m_arrAnimations[nAnimId].nGlobalAnimId);
	}
}

//! Loads the animation data in memory. fWhenRequired is the timeout in seconds from current moment when
//! the animation data will actually be required
void CryModelAnimationContainer::StartLoadAnimation (int nAnimId, float fWhenRequired)
{
	if ((unsigned)nAnimId < m_arrAnimations.size())
	{
		m_pControllerManager->LoadAnimation(m_arrAnimations[nAnimId].nGlobalAnimId);
	}
}
