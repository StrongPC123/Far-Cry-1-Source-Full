//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:CryCharManager.cpp
//  Implementation of CryCharManager class
//
//	History:
//	August 16, 2002: Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <IGame.h> // to examine whether we're in dedicated server mode
#include "CryCharDecal.h"
#include "CryCharInstance.h"
#include "CryCharManager.h"
#include "CryModelState.h"
#include "ControllerManager.h"
#include "cvars.h"
#include "CryCharBody.h"
#include "StringUtils.h"
#include "CryModEffAnimation.h"
#include "CryAnimationScriptCommands.h"
#include "IncContHeap.h"

#include "AnimObjectManager.h"
// for freeing resources

#include "CryCharDecalManager.h"
#include "SSEUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace CryStringUtils;


// init memory pool usage
//#if defined(WIN32)
_ACCESS_POOL;
//#endif 

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call,	LPVOID lpReserved )
{
	return TRUE;
}
#endif

float g_YLine=0.0f;

//////////////////////////////////////////////////////////////////////////
// The main symbol exported from CryAnimation: creator of the main (root) interface/object
ICryCharManager * CreateCharManager(ISystem	* pSystem, const char * szInterfaceVersion)
{
	// check the interface timestamp
#if !defined(LINUX)
	if(strcmp(szInterfaceVersion,gAnimInterfaceVersion))
		pSystem->GetILog()->LogError("CreateCharManager(): CryAnimation interface version error");
#endif
	CryCharManager * pCryCharManager = new CryCharManager(pSystem);

	g_CpuFlags = pSystem->GetCPUFlags();
	g_SecondsPerCycle = pSystem->GetSecondsPerCycle();

	return static_cast<ICryCharManager*>(pCryCharManager);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////
// CryCharManager
///////////////////////////////////////////////////////////////////////////////////////////////////////
CryCharManager::CryCharManager (ISystem * pSystem)
#ifdef DEBUG_STD_CONTAINERS
:m_arrBodyCache ("CryCharManager.BodyCache")
#endif
{
	cpu::detect();

	g_Temp.init();
	g_InitInterfaces(pSystem);

#if ENABLE_FRAME_PROFILER	
	if (!cpu::hasRDTSC())
		pSystem->GetILog()->LogToFile ("\002Error: (Critical) this is development version of animation system, with the profiling enabled. Built-in profiler uses RDTSC function for precise time profiling. This machine doesn't seem to have RDTSC instruction implemented. Please recompile with ENABLE_FRAME_PROFILER 0. Don't use ca_Profile 1");
#endif

	//g_ProfilerTimer.init();
	CryModelState::initClass();

	m_pControllerManager = new CControllerManager();
	m_pAnimObjectManager = new CAnimObjectManager;
#if !defined(LINUX)
	cpu::logCaps();
#endif

#define REGISTER_COMMAND_SHORTCUT(name) g_GetConsole()->AddCommand("ca" #name, "Animation:" #name "()",VF_CHEAT)
	REGISTER_COMMAND_SHORTCUT(DumpAnims);
	REGISTER_COMMAND_SHORTCUT(DumpModels);
	REGISTER_COMMAND_SHORTCUT(TrashAnims);
	REGISTER_COMMAND_SHORTCUT(ClearDecals);
	REGISTER_COMMAND_SHORTCUT(DumpDecals);
	REGISTER_COMMAND_SHORTCUT(DumpStates);
#undef REGISTER_COMMAND_SHORTCUT
}

// returns the controller manager used by this character manager
CControllerManager * CryCharManager::GetControllerManager()
{
	return m_pControllerManager;
}


CryCharManager::~CryCharManager()
{
	m_setLockedBodies.clear();
	if (!m_arrBodyCache.empty())
	{
		g_GetLog()->LogToFile("*ERROR* CryCharManager: %u body instances still not deleted. Forcing deletion (though some instances may still refer to them)", m_arrBodyCache.size());
		CleanupBodyCache();
	}

	//CryModel::freeShadowVolumeVerts();
	CryModelState::deinitClass();

	delete m_pAnimObjectManager;
	delete m_pControllerManager;

	g_GetLog()->LogToFile ("\004CryAnimation profile statistics:");
	g_GetLog()->LogToFile ("\004load.model         (Loading character models)       : %4.2f sec (this is the TOTAL)", g_dTimeGeomLoad);
	g_GetLog()->LogToFile ("\004load.model.grarr   (Generating Render Arrays)         : %4.2f sec", g_dTimeGenRenderArrays);
	g_GetLog()->LogToFile ("\004load.model.shaders (Shader loading)                     : %4.2f sec", g_dTimeShaderLoad);
	g_GetLog()->LogToFile ("\004load.model.geom_prepare(Geometry (cgf) Preparing)     : %4.2f sec", g_dTimeGeomChunkLoad);
	g_GetLog()->LogToFile ("\004load.model.geom_fileio                                : %4.2f sec", g_dTimeGeomChunkLoadFileIO);
	g_GetLog()->LogToFile ("\004load.model.anim    (Animation (caf) Loading/Binding)  : %4.2f sec", g_dTimeAnimLoadBind);
	g_GetLog()->LogToFile ("\004load.model.anim.nocal    (scan for animation files)     : %4.2f sec", g_dTimeAnimLoadBindNoCal);
	g_GetLog()->LogToFile ("\004load.model.anim.withcal  (parse CAL file)               : %4.2f sec", g_dTimeAnimLoadBindWithCal);
	g_GetLog()->LogToFile ("\004load.model.anim.prepare (preallocating)                 : %4.2f sec", g_dTimeAnimLoadBindPreallocate);
	g_GetLog()->LogToFile ("\004load.model.anim.fileio  (Animation (caf) File i/o)      : %4.2f sec",g_dTimeAnimLoadFile);
	g_GetLog()->LogToFile ("\004load.model.anim.bind    (Animation (caf) Binding)       : %4.2f sec", g_dTimeAnimBindControllers);
	g_GetLog()->LogToFile ("\004load.model.anim.postinit (Post-initialization)          : %4.2f sec", g_dTimeGeomPostInit);
	g_GetLog()->LogToFile ("\004test.1: %4.2f sec", g_dTimeTest1);
	g_GetLog()->LogToFile ("\004test.2: %4.2f sec", g_dTimeTest2);
	g_GetLog()->LogToFile ("\004test.3: %4.2f sec", g_dTimeTest3);
	g_GetLog()->LogToFile ("\004test.4: %4.2f sec", g_dTimeTest4);
	g_dTimeGeomLoad        = 0;
	g_dTimeAnimLoadBind    = 0;
	g_dTimeAnimLoadBindNoCal    = 0;
	g_dTimeAnimLoadBindWithCal    = 0;
	g_dTimeAnimLoadBindPreallocate    = 0;
	g_dTimeShaderLoad      = 0;
	g_dTimeGeomChunkLoad   = 0;
	g_dTimeGeomChunkLoadFileIO = 0;
	g_dTimeGenRenderArrays = 0;
	g_dTimeAnimBindControllers = 0;
	g_dTimeAnimLoadFile    = 0;
	g_dTimeGeomPostInit    = 0;
	g_dTimeTest1 = g_dTimeTest2 = g_dTimeTest3 = g_dTimeTest4 = 0;

	if (g_nAsyncAnimCounter)
		g_GetLog()->LogToFile ("\003%d async anim loads; %d (ave %.2f) frame-skips", g_nAsyncAnimCounter, g_nAsyncAnimFrameDelays, double(g_nAsyncAnimFrameDelays)/g_nAsyncAnimCounter);
	g_nAsyncAnimFrameDelays = 0;
	g_nAsyncAnimCounter = 0;

	g_GetLog()->LogToFile ("Memory usage dump:");
	g_GetLog()->LogToFile ("Scratchpad (temporary storage): %d kbytes", g_Temp.size()/1024);

	CryCharDecalManager::LogStatistics();

	g_DeleteInterfaces();
	CCryModEffAnimation::deinitClass();
	
	g_Temp.done();
}


//////////////////////////////////////////////////////////////////////////
// Loads a cgf and the corresponding caf file and creates an animated object,
// or returns an existing object.
ICryCharInstance * CryCharManager::MakeCharacter (const char * szCharacterFileName, unsigned nFlags)
{

	g_pIRenderer			= g_pISystem->GetIRenderer();
	g_pIPhysicalWorld	= g_pISystem->GetIPhysicalWorld();
	g_pI3DEngine			=	g_pISystem->GetI3DEngine();

	if (!szCharacterFileName)
		return (NULL);	// to prevent a crash in the frequent case the designers will mess 
										// around with the entity parameters in the editor

	string strPath = szCharacterFileName;
	UnifyFilePath(strPath);

	if (strstr(strPath.c_str(),".cga") || (nFlags & nMakeAnimObject))
	{
		// Loading cga file.
		return m_pAnimObjectManager->MakeAnimObject( szCharacterFileName );
	}

	// try to find already loaded model, or load a new one
	CryCharBody* pCryCharBody = FetchBody (strPath);

	if(!pCryCharBody)
	{
		// the model has not been loaded
		return NULL;
	}

	CryCharInstance * pCryCharInstance = new CryCharInstance (pCryCharBody);

	DecideModelLockStatus(pCryCharBody, nFlags);

	return pCryCharInstance;
}

// loads the character model (which is ref-counted, so you must assign it to an autopointer)
ICryCharModel* CryCharManager::LoadModel(const char* szFileName, unsigned nFlags)
{
	if (!szFileName)
		return NULL;

	string strPath = szFileName;
	UnifyFilePath(strPath);

	if (strstr(strPath.c_str(),".cga") || (nFlags & nMakeAnimObject))
	{
		// Loading cga file: not supported at this time
		return NULL;
	}

	// try to find already loaded model, or load a new one
	CryCharBody* pCryCharBody = FetchBody (strPath);

	return pCryCharBody;
}

// locks or unlocks the given body if needed, depending on the hint and console variables
void CryCharManager::DecideModelLockStatus(CryCharBody* pCryCharBody, unsigned nHints)
{
	// there's already a character, so we can safely remove the body from the lock pool
	// basic rules:
	//  transient hint takes precedence over everything else
	//  any hint takes precedence over the console settings
	//  if there are no hints, console variable ca_KeepModels is used to determine
	//    if the model should be kept in memory all the time (ca_KeepModels 1) or not (0)
	//  Note: changing ca_KeepModels won't immediately lock or unlock all models. It will only affect
	//  models which are actively used to create new characters or destroy old ones.
	if (nHints & nHintModelTransient) 
		m_setLockedBodies.erase (pCryCharBody);
	else
	if (nHints & nHintModelPersistent) 
		m_setLockedBodies.insert (pCryCharBody);
	else
	if (g_GetCVars()->ca_KeepModels())
		m_setLockedBodies.insert (pCryCharBody);
	else
		m_setLockedBodies.erase (pCryCharBody);
}


/////////////////////////////////////////////////////////////////////////////
// Reduces reference counter for object and deletes object if counter is 0
void CryCharManager::RemoveCharacter (ICryCharInstance * pCryCharInstance, unsigned nFlags)
{
	if (pCryCharInstance && !m_pAnimObjectManager->RemoveCharacter( pCryCharInstance ))
	{
		// check if the character exists (maybe it's a dangling pointer)
		bool bFound = false;
		for (CryCharBodyCache::iterator it = m_arrBodyCache.begin(); !bFound && it != m_arrBodyCache.end(); ++it)
		{
			bFound = (*it)->DoesInstanceExist (static_cast<CryCharInstance*>(pCryCharInstance));
		}

		if (!bFound)
		{
			g_GetLog()->LogError ("\001attempt to delete invalid character pointer 0x%08X. Perhaps it was auto-deleted during Forced Resource Cleanup upon level reloading, see the log for details.", pCryCharInstance);
			return;
		}

		CryCharInstance* pCharacter = static_cast<CryCharInstance*> (pCryCharInstance);
		DecideModelLockStatus(pCharacter->GetBody(), nFlags);
		// this will delete the character and possibly the corresponding Body instance
		pCharacter->Release();
	}
}


// Deletes itself
void CryCharManager::Release()
{
	delete this;
}


// adds the body to the cache from which it can be reused if another instance of that model is to be created
void CryCharManager::RegisterBody (CryCharBody* pBody)
{
	m_arrBodyCache.insert (std::lower_bound(m_arrBodyCache.begin(), m_arrBodyCache.end(), pBody->GetFilePath(), OrderByFileName()), pBody);
}


// Deletes the given body from the model cache. This is done when there are no instances using this body info.
void CryCharManager::UnregisterBody (CryCharBody* pBody)
{
	ValidateBodyCache();
	CryCharBodyCache::iterator itCache = std::lower_bound (m_arrBodyCache.begin(), m_arrBodyCache.end(), pBody, OrderByFileName());
	if (itCache != m_arrBodyCache.end())
	{
		if (*itCache == pBody)
			m_arrBodyCache.erase (itCache);
		else
		{
			assert (false); // there must be no duplicate name pointers here
			while (++itCache != m_arrBodyCache.end())
				if (*itCache == pBody)
				{
					m_arrBodyCache.erase(itCache);
					break;
				}
		}
	}
	else
		assert (false); // this pointer must always be in the cache
	ValidateBodyCache();
}


//////////////////////////////////////////////////////////////////////////
// Finds a cached or creates a new CryCharBody instance and returns it
// returns NULL if the construction failed
CryCharBody* CryCharManager::FetchBody (const string& strFileName)
{
	ValidateBodyCache();

	{
		CryCharBodyCache::iterator it = std::lower_bound (m_arrBodyCache.begin(), m_arrBodyCache.end(), strFileName, OrderByFileName());

		if (it != m_arrBodyCache.end())
		{
			const string& strBodyFilePath = (*it)->GetFilePath();
			if (strBodyFilePath == strFileName)
				return *it;
		}
	}

	CryCharBody* pBody = new CryCharBody (this, strFileName);
	if (pBody->GetModel())
		return pBody;
	else
	{
		delete pBody;
		return NULL;
	}
}

// returns statistics about this instance of character animation manager
// don't call this too frequently
void CryCharManager::GetStatistics(Statistics& rStats)
{
	memset (&rStats, 0, sizeof(rStats));
	rStats.numCharModels = (unsigned)m_arrBodyCache.size();
	for (CryCharBodyCache::const_iterator it = m_arrBodyCache.begin(); it != m_arrBodyCache.end(); ++it)
		rStats.numCharacters += (*it)->NumInstances();

	rStats.numAnimObjectModels = rStats.numAnimObjects = m_pAnimObjectManager->NumObjects();
}

// Validates the cache
void CryCharManager::ValidateBodyCache()
{
#ifdef _DEBUG
	CryCharBodyCache::iterator itPrev = m_arrBodyCache.end();
	for (CryCharBodyCache::iterator it = m_arrBodyCache.begin(); it != m_arrBodyCache.end(); ++it)
	{
		if (itPrev != m_arrBodyCache.end())
			assert ((*itPrev)->GetFilePath() < (*it)->GetFilePath());
	}
#endif
}



//////////////////////////////////////////////////////////////////////////
// Deletes all the cached bodies and their associated character instances
void CryCharManager::CleanupBodyCache()
{
	m_setLockedBodies.clear();
	while (!m_arrBodyCache.empty())
	{
		CryCharBody* pBody = m_arrBodyCache.back();
		// cleaning up the instances referring to this body actually releases it and 
		// deletes from the cache
		pBody->CleanupInstances();

		if (m_arrBodyCache.empty())
			break;

		// if the body still stays in the cache, it means someone (who?!) still refers to it
		if (m_arrBodyCache.back() == pBody)
		{
			assert(0); // actually ,after deleting all instances referring to the body, the body must be auto-deleted and deregistered.
			// if this doesn't happen, something is very wrong
			//delete pBody; // still, we'll delete the body
		}
	}
}


bool CryCharManager::OrderByFileName::operator () (const CryCharBody* pLeft, const CryCharBody* pRight)
{
	return pLeft->GetFilePath() < pRight->GetFilePath();
}
bool CryCharManager::OrderByFileName::operator () (const string& strLeft, const CryCharBody* pRight)
{
	return strLeft < pRight->GetFilePath();
}
bool CryCharManager::OrderByFileName::operator () (const CryCharBody* pLeft, const string& strRight)
{
	return pLeft->GetFilePath() < strRight;
}

// puts the size of the whole subsystem into this sizer object, classified,
// according to the flags set in the sizer
void CryCharManager::GetMemoryUsage(class ICrySizer* pSizer)const
{
#if ENABLE_GET_MEMORY_USAGE
	if (!pSizer->Add(*this))
		return;
	if (!m_arrBodyCache.empty())
	{
		for (CryCharBodyCache::const_iterator it = m_arrBodyCache.begin(); it != m_arrBodyCache.end(); ++it)
			(*it)->GetSize (pSizer);
		pSizer->Add (&m_arrBodyCache[0], m_arrBodyCache.size());
	}
	m_pControllerManager->GetSize (pSizer);
	m_pAnimObjectManager->GetMemoryUsage (pSizer);
	/*
	{
		SIZER_COMPONENT_NAME(pSizer, "Test1M");
		{
			SIZER_COMPONENT_NAME (pSizer, "Test1M");
			pSizer->AddObject((void*)1, 0x10000-1);
		}
		{
			SIZER_COMPONENT_NAME (pSizer, "Test1M");
			pSizer->AddObject((void*)2, 0x10000-1);
			{
				SIZER_COMPONENT_NAME (pSizer, "Test1M");
				pSizer->AddObject((void*)3, 0x10000-1);
			}
		}
		{
			SIZER_COMPONENT_NAME(pSizer, "Test2M");
			{
				SIZER_COMPONENT_NAME (pSizer, "Test1M");
				pSizer->AddObject((void*)4, 0x10000-1);
			}
			{
				SIZER_COMPONENT_NAME (pSizer, "Test1M");
				pSizer->AddObject((void*)5, 0x10000-1);
			}
		}
	}
	*/
#endif
}

//! Executes a script command
//! Returns true if the command was executed, or false if not
//! All the possible values for nCommand are in the CryAnimationScriptCommands.h
//! file in the CryAnimationScriptCommandEnum enumeration. All the parameter/result
//! structures are also there.
bool CryCharManager::ExecScriptCommand (int nCommand, void* pParams, void* pResult)
{
	switch (nCommand)
	{
	case CASCMD_TEST_PARTICLES:
	case CASCMD_STOP_PARTICLES:
		{
			for (CryCharBodyCache::iterator it = m_arrBodyCache.begin(); it != m_arrBodyCache.end(); ++it)
				(*it)->SpawnTestParticles (nCommand == CASCMD_TEST_PARTICLES);
		}
		break;

	case CASCMD_DUMP_ANIMATIONS:
		m_pControllerManager->DumpAnims();
		break;

	case CASCMD_TRASH_ANIMATIONS:
		UnloadOldAnimations(pParams?*(int*)pParams:100);
		break;

	case CASCMD_UNLOAD_ANIMATION:
		UnloadAnimation ((char*)pParams);
		break;

	case CASCMD_DUMP_MODELS:
	case CASCMD_EXPORT_MODELS_ASCII:
	case CASCMD_CLEAR_DECALS:
	case CASCMD_DUMP_DECALS:
	case CASCMD_START_MANY_ANIMS:
	case CASCMD_DEBUG_DRAW:
	case CASCMD_DUMP_STATES:
		{
			for (CryCharBodyCache::iterator it = m_arrBodyCache.begin(); it != m_arrBodyCache.end(); ++it)
				(*it)->ExecScriptCommand(nCommand, pParams, pResult);
		}
		break;

	default:
		return false; // unknown command
	}
	return true;
}

void CryCharManager::ClearDecals()
{
	ExecScriptCommand(CASCMD_CLEAR_DECALS);
}

//! Cleans up all resources - currently deletes all bodies and characters (even if there are references on them)
void CryCharManager::ClearResources(void)
{
	CleanupBodyCache();
}


// should be called every frame
void CryCharManager::Update()
{
	//update interfaces every frame
	g_YLine=16.0f;
	g_pIRenderer			= g_pISystem->GetIRenderer();
	g_pIPhysicalWorld	= g_pISystem->GetIPhysicalWorld();
	g_pI3DEngine			=	g_pISystem->GetI3DEngine();

	g_nFrameID = g_GetIRenderer()->GetFrameID();
	g_bProfilerOn = g_GetISystem()->GetIProfileSystem()->IsProfiling();
	
	IGame* pGame = g_GetISystem()->GetIGame();
	g_bUpdateBonesAlways = pGame? (pGame->GetModuleState(EGameServer) && pGame->GetModuleState(EGameMultiplayer)) : false;

	m_pControllerManager->Update();
	CryCharDecal::setGlobalTime(g_GetTimer()->GetCurrTime());

	if (g_GetCVars()->ca_DrawBones() > 1) 
		ExecScriptCommand(CASCMD_DEBUG_DRAW);
}

//! The specified animation will be unloaded from memory; it will be loaded back upon the first invokation (via StartAnimation())
void CryCharManager::UnloadAnimation (const char* szFileName)
{
	int nAnimId = m_pControllerManager->FindAnimationByFile(szFileName);
	if (nAnimId < 0)
		g_GetLog()->LogWarning ("\004CryCharManager::UnloadAnimation(%s): animation not loaded", szFileName);
	else
		m_pControllerManager->UnloadAnimation(nAnimId);
}

//! Starts loading the specified animation. fWhenRequired is the timeout, in seconds, from the current moment,
//! when the animation data will actually be needed
void CryCharManager::StartLoadAnimation (const char* szFileName, float fWhenRequired)
{
	m_pControllerManager->StartLoadAnimation(szFileName, g_fDefaultAnimationScale);
}

void CryCharManager::UnloadOldAnimations(int numFrames)
{
	unsigned numAnims = m_pControllerManager->NumAnimations(), numUnloaded = 0, numFound = 0;
	for (unsigned i = 0; i < numAnims; ++i)
	{
		if (m_pControllerManager->GetAnimation(i).nLastAccessFrameId + numFrames < g_nFrameID)
		{
			++numFound;
			if (m_pControllerManager->UnloadAnimation(i))
				++numUnloaded;
		}
	}
	if (numFound)
		g_GetLog()->LogToConsole("\001%d Animations were found, %d were unloaded", numFound, numUnloaded);
	else
		g_GetLog()->LogToConsole("\001No Animation were found/unloaded");
}

//! Locks all models in memory
void CryCharManager::LockResources()
{
	m_arrTempLock.clear();
	m_arrTempLock.resize (m_arrBodyCache.size());
	for (size_t i = 0; i < m_arrBodyCache.size(); ++i)
		m_arrTempLock[i] = m_arrBodyCache[i];

	m_pAnimObjectManager->LockResources();
}

//! Unlocks all models in memory
void CryCharManager::UnlockResources()
{
	m_arrTempLock.clear();
	m_pAnimObjectManager->UnlockResources();
}
