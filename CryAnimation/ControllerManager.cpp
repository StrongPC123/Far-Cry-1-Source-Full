// method definitions extracted from CryModel.*

#include "stdafx.h"
#include <StlUtils.h>
#include <CryCompiledFile.h>
#include "ControllerManager.h"
#include "Controller.h"
#include "ControllerCryBone.h"
#include "ControllerPackedBSpline.h"
#include "FileMapping.h"
#include "ChunkFileReader.h"
#include "CVars.h"
#include "CryModelAnimationContainer.h"

#if defined(LINUX)
#	include "CryAnimationInfo.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CControllerManager::CControllerManager():
#ifdef DEBUG_STD_CONTAINERS
	m_arrAnims ("CControllerManager.Anims"),
	m_arrAnimByFile ("CControllerManager.AnimByFile"),
#endif
	m_nCachedSizeofThis (0),
	m_nLastCheckedUnloadCandidate(0)
{ 
	// we reserve the place for future animations. The best performance will be
	// archived when this number is >= actual number of animations that can be used,
	// and not much greater
	m_arrAnims.reserve (0x100);
}

/////////////////////////////////////////////////////////////////////
// finds the animation by name. Returns -1 if no animation was found
// Returns the animation ID if it was found
int CControllerManager::FindAnimationByFile (const string& sAnimFileName)
{
	std::vector<int>::iterator it = std::lower_bound(m_arrAnimByFile.begin(), m_arrAnimByFile.end(), sAnimFileName.c_str(), AnimationIdPred(m_arrAnims));
	if (it != m_arrAnimByFile.end() && !stricmp(m_arrAnims[*it].strFileName.c_str(),sAnimFileName.c_str()))
		return *it;
	else
		return -1;
}


CControllerManager::~CControllerManager()
{
#ifdef _DEBUG
	for (AnimationArray::iterator it = m_arrAnims.begin(); it!= m_arrAnims.end(); ++it)
		assert (it->nRefCount == 0);
#endif
	for(int nAttempt = 0; nAttempt < 40 && !m_setPendingAnimLoads.empty(); ++nAttempt)
	{
		g_GetLog()->LogToFile("\003%d asynch loading are pending, waiting...", m_setPendingAnimLoads.size());
		g_GetStreamEngine()->Wait(200,IStreamEngine::FLAGS_DISABLE_CALLBACK_TIME_QUOTA);
	}
	if (!m_setPendingAnimLoads.empty())
		g_GetLog()->LogWarning ("\002%d asynchronous operations are left; destructing the controller manager anyway, because they seem to be dangling.If a crash happens after this, perhaps this is because some async operation took _very_ long to complete.", m_setPendingAnimLoads.size());
}

////////////////////////////////////////////////////////////////////////////////////
// loads the animation with the specified name; if the animation is already loaded,
// then just returns its id
// The caller MUST TAKE CARE to bind the animation if it's already loaded before it has registered itself within this manager
// RETURNS:
//   The global animation id.
//   -1 if the animation couldn't be loaded 
// SIDE EFFECT NOTES:
//   This function does not put up a warning in the case the file was not found.
//   The caller must do so. But if the file was found and was corrupted, more detailed
//   error information (like where the error occured etc.) may be put into the log
int CControllerManager::StartLoadAnimation (const string& strFileName, float fScale, unsigned nFlags)
{
	int nAnimId = FindAnimationByFile (strFileName);
	bool bRecordExists = (nAnimId >= 0);
	if (bRecordExists && m_arrAnims[nAnimId].IsLoaded())
	{
		// we have already such file loaded
		return nAnimId;
	}
	else
	{
		selfValidate();
		if (!bRecordExists)
		{
			// add a new animation structure that will hold the info about the new animation.
			// the new animation id is the index of this structure in the array
			nAnimId = (int)m_arrAnims.size();
			m_arrAnims.resize (nAnimId + 1);
		}

		Animation& Anim = m_arrAnims[nAnimId];
		Anim.nFlags = nFlags;

		if (!bRecordExists)
			Anim.strFileName = strFileName;

		Anim.fScale = fScale;
		
		if (!bRecordExists)// no need to insert if we reload the file
		{
			m_arrAnimByFile.insert (std::lower_bound(m_arrAnimByFile.begin(), m_arrAnimByFile.end(), nAnimId, AnimationIdPred(m_arrAnims)), nAnimId);
		}
		selfValidate();
	}

	return nAnimId;
}
 
// loads the animation info, if not loaded yet
bool CControllerManager::LoadAnimationInfo(int nGlobalAnimId)
{
	if ((unsigned)nGlobalAnimId < m_arrAnims.size())
	{
		if (m_arrAnims[nGlobalAnimId].IsInfoLoaded())
			return true;
		return LoadAnimation(nGlobalAnimId);
	}
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
// Loads the animation controllers into the existing animation record.
// May be used to load the animation the first time, or reload it upon request.
// Does nothing if there are already controls in the existing animation record.
bool CControllerManager::LoadAnimation(int nAnimId)
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );

	Animation& Anim = m_arrAnims[nAnimId];
	if (Anim.IsLoaded())
		return true;

	if (Anim.nFlags & Animation::FLAGS_LOAD_PENDING)
	{
		// we already have this animation being loaded
		return true;
	}

	unsigned nFileSize = g_GetStreamEngine()->GetFileSize(Anim.strFileName.c_str());

	if (!nFileSize) // no such file
	{
		if (!(Anim.nFlags & GlobalAnimation::FLAGS_DISABLE_LOAD_ERROR_LOG))
			g_GetLog()->LogError ("\003CControllerManager::LoadAnimation: file loading %s file not found", Anim.strFileName.c_str());
		return false;
	}

	if (Anim.IsInfoLoaded())
	{	// we can start loading asynchronously
		// this structure will keep the info until the completion routine is called
		PendingAnimLoad_AutoPtr pAnimLoad = new PendingAnimLoad;
		pAnimLoad->nAnimId = nAnimId;
		pAnimLoad->pFile = new CFileMapping;
		pAnimLoad->pFile->attach (malloc (nFileSize), nFileSize);
		pAnimLoad->nFrameId = g_nFrameID;
		
		// these parameters tell that the file must be read directly into the file mapping object
		// the file mapping object will be passed to the chunk reader that will parse the data
		StreamReadParams params;
		params.dwUserData = (DWORD_PTR)(PendingAnimLoad*)pAnimLoad;
		params.nSize = nFileSize;
		params.pBuffer = pAnimLoad->pFile->getData();

		// now we're prepared to read, register the pending operation and start reading
		m_setPendingAnimLoads.insert (pAnimLoad);
		// mark the animation as being loaded
		Anim.nFlags |= Animation::FLAGS_LOAD_PENDING;
		// we're not interested in the pointer to the read stream - we only need to know when it's finished
		pAnimLoad->pStream = g_GetStreamEngine()->StartRead("Animation", Anim.strFileName.c_str(), this, &params);
		return true;
	}
	else
	{
		// if the info hasn't still been loaded, we need to load synchronously
		// try to read the file
		CChunkFileReader Reader;
		if (!Reader.open (Anim.strFileName))
		{
			if (!(Anim.nFlags & GlobalAnimation::FLAGS_DISABLE_LOAD_ERROR_LOG))
				g_GetLog()->LogError ("\003CControllerManager::LoadAnimation: file loading %s, last error is: %s", Anim.strFileName.c_str(), Reader.getLastError());
			return false;
		}
		return LoadAnimation(nAnimId, &Reader);
	}
}

void CControllerManager::StreamOnComplete (IReadStream* pStream, unsigned nError)
{
	PendingAnimLoad_AutoPtr pAnimLoad = (PendingAnimLoad*)pStream->GetUserData();
	assert (m_setPendingAnimLoads.find (pAnimLoad) != m_setPendingAnimLoads.end());
	Animation& Anim = m_arrAnims[pAnimLoad->nAnimId];
	m_setPendingAnimLoads.erase(pAnimLoad);
	// flag the animation as being loaded
	Anim.nFlags &= ~Animation::FLAGS_LOAD_PENDING;
	if (nError)
	{
#if defined(LINUX)
		g_GetLog()->LogError ("\003Asynchronous load of file %s has failed, error code 0x%X", Anim.strFileName.c_str(), nError);
#else
		g_GetLog()->LogError ("\003Asynchronous load of file %s has failed (%d of %d bytes read), error code 0x%X", Anim.strFileName.c_str(), pStream->GetBytesRead(), pAnimLoad->pFile->getSize(), nError);
#endif
	}
	else
	{
		CChunkFileReader Reader;
		if (!Reader.open (pAnimLoad->pFile))
#if defined(LINUX)
			g_GetLog()->LogError ("\003Error: Asynchronous load of file %s has completed successfully, but the data can't be parsed. The last \"%s\".", Anim.strFileName.c_str(), Reader.getLastError());
#else
			g_GetLog()->LogError ("\003Error: Asynchronous load of file %s has completed successfully (%d of expected %d bytes read), but the data can't be parsed. The last \"%s\".", Anim.strFileName.c_str(), pStream->GetBytesRead(), pAnimLoad->pFile->getSize(), Reader.getLastError());
#endif
		else
		{
			if (LoadAnimation (pAnimLoad->nAnimId, &Reader))
			{
				++g_nAsyncAnimCounter;
				g_nAsyncAnimFrameDelays += g_nFrameID - pAnimLoad->nFrameId;
			}
		}
	}
}

// immediately load the given animation from the already opened reader
bool CControllerManager::LoadAnimation (int nAnimId, CChunkFileReader* pReader)
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );

	Animation& Anim = m_arrAnims[nAnimId];

	// check the file header for validity
	const FILE_HEADER& fh = pReader->getFileHeader();

	if(fh.Version != AnimFileVersion || fh.FileType != FileType_Anim) 
	{
		g_GetLog()->LogError ("\003CControllerManager::LoadAnimation: file version error or not an animation file: %s", Anim.strFileName.c_str());
		return false;
	}

	// prepare the array of controllers and the counter for them
	Anim.arrCtrls.resize(pReader->numChunksOfType (ChunkType_Controller));
	unsigned nController = 0;

	m_nCachedSizeofThis = 0;
	// scan the chunks and load all controllers and time data into the animation structure
	for (int nChunk = 0; nChunk < pReader->numChunks (); ++nChunk)
	{
		// this is the chunk header in the chunk table at the end of the file
		const CHUNK_HEADER& chunkHeader = pReader->getChunkHeader(nChunk);
		// this is the chunk raw data, starts with the chunk header/descriptor structure
		const void* pChunk = pReader->getChunkData (nChunk);
		unsigned nChunkSize = pReader->getChunkSize(nChunk);

		switch (chunkHeader.ChunkType)
		{
		case ChunkType_Controller:
			switch (chunkHeader.ChunkVersion)
			{
			case CONTROLLER_CHUNK_DESC_0826::VERSION:
				{
					// load and add a controller constructed from the controller chunk
					const CONTROLLER_CHUNK_DESC_0826* pCtrlChunk = static_cast<const CONTROLLER_CHUNK_DESC_0826*>(pChunk);
					IController_AutoPtr pController = LoadController (Anim.fScale, pCtrlChunk, nChunkSize);
					if (!pController)
					{
						g_GetLog()->LogError ("\002CControllerManager::LoadAnimation: error loading v826 controller: %s", Anim.strFileName.c_str());
						return false;
					}

					assert (nController < Anim.arrCtrls.size());
					Anim.arrCtrls[nController++] = pController;
				}
				break;

			case CONTROLLER_CHUNK_DESC_0827::VERSION:
				{
					const CONTROLLER_CHUNK_DESC_0827* pCtrlChunk = static_cast<const CONTROLLER_CHUNK_DESC_0827*>(pChunk);
					CControllerCryBone_AutoPtr pController = new CControllerCryBone ();
					if (pController->Load (pCtrlChunk, nChunkSize, Anim.fScale))
					{
						assert (nController < Anim.arrCtrls.size());
						Anim.arrCtrls[nController++] = static_cast<IController*>(pController);
					}
					else
					{
						g_GetLog()->LogError ("\002CControllerManager::LoadAnimation: error loading v827 controller: %s", Anim.strFileName.c_str());
						return false;
					}
				}
				break;
			default:
				g_GetLog()->LogError ("\003Unsupported controller chunk 0x%08X version 0x%08X in file %s. Please re-export the file.", chunkHeader.ChunkID, chunkHeader.ChunkVersion, Anim.strFileName.c_str());
				return false;
				break;
			}
			break;

		case ChunkType_Timing:
			{
				// memorize the timing info
				const TIMING_CHUNK_DESC* pTimingChunk = static_cast<const TIMING_CHUNK_DESC*> (pChunk);
				Anim.nTicksPerFrame  = pTimingChunk->TicksPerFrame;
				Anim.fSecsPerTick    = pTimingChunk->SecsPerTick;
				Anim.rangeGlobal      = pTimingChunk->global_range;
			}
			break;
		}
	}

	if (nController != Anim.arrCtrls.size())
	{
		g_GetLog()->LogError ("\003%d controllers (%d expected) loaded from file %s. Please re-export the file. The animations will be discarded.",
			nController, Anim.arrCtrls.size(), Anim.strFileName.c_str());
	}

	std::sort(
		Anim.arrCtrls.begin(), 
		Anim.arrCtrls.end(), 
		AnimCtrlSortPred()
		);

	Anim.OnInfoLoaded();
	FireAnimationGlobalLoad(nAnimId);

	return true;
}


// updates the animation from the chunk of AnimInfo
bool CControllerManager::UpdateAnimation (int nGlobalAnimId, const CCFAnimInfo* pAnimInfo)
{
	if ((unsigned)nGlobalAnimId >= m_arrAnims.size())
		return false;

	GlobalAnimation& Anim = m_arrAnims[nGlobalAnimId];

#if !defined(LINUX)
/*	if (Anim.IsInfoLoaded())
	{
		assert (Anim.fSecsPerTick      == pAnimInfo->fSecsPerTick);
		assert (Anim.nTicksPerFrame    == pAnimInfo->nTicksPerFrame);
		assert (Anim.rangeGlobal.start == pAnimInfo->nRangeStart);
		assert (Anim.rangeGlobal.end   == pAnimInfo->nRangeEnd);
		return true;
	}*/
#endif

	Anim.fSecsPerTick      = pAnimInfo->fSecsPerTick;
	Anim.nTicksPerFrame    = pAnimInfo->nTicksPerFrame;
	Anim.rangeGlobal.start = pAnimInfo->nRangeStart;
	Anim.rangeGlobal.end   = pAnimInfo->nRangeEnd;
	Anim.nFlags |= pAnimInfo->nAnimFlags;
	Anim.OnInfoLoaded();
	
	FireAnimationGlobalLoad(nGlobalAnimId);
	return true;
}

// notify everybody that the given animation OR its info has been just loaded
void CControllerManager::FireAnimationGlobalLoad (int nAnimId)
{
	for (std::vector<CryModelAnimationContainer*>::iterator it = m_arrClients.begin(); it != m_arrClients.end(); ++it)
		(*it)->OnAnimationGlobalLoad(nAnimId);
} 

// notifies the controller manager that another client uses the given animation.
// these calls must be balanced with AnimationRelease() calls
void CControllerManager::AnimationAddRef (int nGlobalAnimId, CryModelAnimationContainer* pClient)
{
	assert ((unsigned)nGlobalAnimId< m_arrAnims.size());
	// the new client, though it might have received "OnLoad" events from this global animation,
	// might have missed it because it wasn't interested in them; now we re-send this event if needed
	m_arrAnims[nGlobalAnimId].AddRef();
	if (m_arrAnims[nGlobalAnimId].IsLoaded())
		pClient->OnAnimationGlobalLoad(nGlobalAnimId);
}

// notifies the controller manager that this client doesn't use the given animation any more.
// these calls must be balanced with AnimationAddRef() calls
void CControllerManager::AnimationRelease (int nGlobalAnimId, CryModelAnimationContainer* pClient)
{
	assert ((unsigned)nGlobalAnimId< m_arrAnims.size());
	// if the given client is still interested in unload events, it will receive them all anyway,
	// so we don't force anything but pure release. Normally during this call the client doesn't need
	// any information about the animation being released
	m_arrAnims[nGlobalAnimId].Release();
}


////////////////////////////////////////////////////////////////////////////////////
// returns the total number of animations hold in memory (for statistics)
unsigned CControllerManager::NumAnimations()
{
	return (unsigned)m_arrAnims.size();
}


////////////////////////////////////////////////////////////////////////////////////
// Loads the controller from the chunk, returns the result in the autopointer
// It is important that the return type is autoptr: it lets the procedure gracefully destruct 
// half-constructed animation in case of an error.
IController_AutoPtr CControllerManager::LoadController (float fScale, const CONTROLLER_CHUNK_DESC_0826* pChunk, int nSize)
{
	// different controllers are constructed depending on the representation of the raw data:
	// spline or original crybone
	IController_AutoPtr pController;
	switch (pChunk->type)
	{
	case CTRL_BSPLINE_2O:
	case CTRL_BSPLINE_1O:
	case CTRL_BSPLINE_2C:
	case CTRL_BSPLINE_1C:
		{
			// one of the fixed-point spline formats
			CControllerPackedBSpline_AutoPtr pCtrl = new CControllerPackedBSpline();
			if (pCtrl->Load(pChunk, nSize, fScale))
				pController = static_cast<IController*>(static_cast<CControllerPackedBSpline*>(pCtrl));
		}
		break;

	case CTRL_CRYBONE:
		{
			// the old bone format
		  CControllerCryBone_AutoPtr pCtrl = new CControllerCryBone ();
			if (pCtrl->Load(pChunk, fScale))
				pController = static_cast<IController*>(static_cast<CControllerCryBone*>(pCtrl));
		}
		break;
	}

	return pController;
}

/////////////////////////////////////////////////////////////////////////////////////
// finds controller with the given nControllerID among controller in the animation
// identified by nGlobalAnimID
IController* CControllerManager::GetController (int nGlobalAnimID, unsigned nControllerID)
{
	Animation& Anim = m_arrAnims[nGlobalAnimID];
	return Anim.GetController(nControllerID);
}

//////////////////////////////////////////////////////////////////////////
// logs controller usage statistics. Used for debugging only 
void CControllerManager::LogUsageStats()
{
	size_t nCtrlUnused = 0;
	size_t nCtrlTotal = 0;
	size_t nCtrlUsage = 0;
	AnimationArray::const_iterator it = m_arrAnims.begin(), itEnd = it + m_arrAnims.size();
	for (; it != itEnd; ++it)
	{
		const Animation& Anim = *it;
		nCtrlTotal += Anim.arrCtrls.size();
		Animation::ControllerArray::const_iterator itCtrl = Anim.arrCtrls.begin(), itCtrlEnd = itCtrl + Anim.arrCtrls.size();
		for (; itCtrl != itCtrlEnd; ++itCtrl)
		{
			IController* pCtrl = *itCtrl;
			if (pCtrl->NumRefs() == 1)
				++nCtrlUnused;
			else
				nCtrlUsage += pCtrl->NumRefs()-1;
		}
	}

	g_GetLog()->LogToFile ("%u controllers, %u (%.1f percent) unused, %.2f average refs per used controller",
		nCtrlTotal, nCtrlUnused, (nCtrlUnused*100.0f/nCtrlTotal),
		float(nCtrlUsage)/(nCtrlTotal-nCtrlUnused));
}


// returns the structure describing the animation data, given the global anim id
CControllerManager::Animation& CControllerManager::GetAnimation (int nAnimID)
{
	if ((unsigned)nAnimID < m_arrAnims.size())
		return m_arrAnims[nAnimID];
	else
	{
		assert (0);
		static Animation dummy;
		return dummy;
	}
}

void CControllerManager::DumpAnims()
{
	// approximately calculating the size of the map
	std::vector<int>::const_iterator it;
	const std::vector<int>::const_iterator itBegin = m_arrAnimByFile.begin(), itEnd = m_arrAnimByFile.end();
	unsigned nMaxNameLength = 0;
	for (it=itBegin; it != itEnd; ++it)
	{
		unsigned nNameLength = (unsigned)m_arrAnims[*it].strFileName.length();
		if (nNameLength > nMaxNameLength)
			nMaxNameLength = nNameLength;
	}

	nMaxNameLength += 2;

	g_GetLog()->LogToFile    ("\001%*s   kbytes    started    ticks", nMaxNameLength, "Animation Memory Usage Dump");
	g_GetLog()->LogToConsole ("\001%*s   kbytes    started    ticks", nMaxNameLength, "Animation Memory Usage Dump");

	// the size of the array of controllers
	size_t nSize = 0;

	typedef std::multimap<size_t, int> SizeMap;
	SizeMap mapSizes;
	for (int nAnim = 0; nAnim < (int)m_arrAnims.size(); ++nAnim)
	{
		size_t nSizeAnimation = m_arrAnims[nAnim].sizeofThis();
		nSize += nSizeAnimation;
		mapSizes.insert (SizeMap::value_type(nSizeAnimation, nAnim));
	}

	for (SizeMap::reverse_iterator itSize = mapSizes.rbegin(); itSize != mapSizes.rend(); ++itSize)
	{
		int nAnim = itSize->second;
		Animation& anim = m_arrAnims[nAnim];

		g_GetLog()->LogToFile    ("\001%*s : %6.1f%10u%10u%s%s %d refs", nMaxNameLength, anim.strFileName.c_str(), itSize->first/1024.0f, anim.nTickCount, anim.nStartCount, anim.IsLoaded()?"":anim.IsInfoLoaded()?",Unloaded":",Not Loaded",anim.nRefCount?"":",Not Used:", anim.nRefCount);
		g_GetLog()->LogToConsole ("\001%*s : %6.1f%10u%10u%s%s %d refs", nMaxNameLength, anim.strFileName.c_str(), itSize->first/1024.0f, anim.nTickCount, anim.nStartCount, anim.IsLoaded()?"":anim.IsInfoLoaded()?",Unloaded":",Not Loaded",anim.nRefCount?"":",Not Used:", anim.nRefCount);
	}

	g_GetLog()->LogToFile    ("\001%*s : %6.1f", nMaxNameLength, "TOTAL", nSize / 1024.0f);
	g_GetLog()->LogToConsole ("\001%*s : %6.1f", nMaxNameLength, "TOTAL", nSize / 1024.0f);
}

// unreferences the controllers and makes the animation unloaded
// before this operation, all bones must be unbound
// returns true if the animation was really unloaded (if it had been unloaded before, returns false)
bool CControllerManager::UnloadAnimation (int nGlobAnimId)
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );

	if ((unsigned)nGlobAnimId>= m_arrAnims.size())
		return false;

	if (m_arrAnims[nGlobAnimId].arrCtrls.empty())
		return false;
	else
	{
		m_nCachedSizeofThis = 0;
		for (std::vector<CryModelAnimationContainer*>::iterator it = m_arrClients.begin(); it!= m_arrClients.end(); ++it)
			(*it)->OnAnimationGlobalUnload (nGlobAnimId);
#if !defined(LINUX)
		assert (m_arrAnims[nGlobAnimId].MaxControllerRefCount()==1);
#endif
		m_arrAnims[nGlobAnimId].arrCtrls.clear();
		return true;
	}
}


void CControllerManager::OnStartAnimation (int nGlobalAnimId)
{
	assert ((unsigned)nGlobalAnimId < m_arrAnims.size());
	LoadAnimation(nGlobalAnimId);
	m_arrAnims[nGlobalAnimId].OnStart();
}

void CControllerManager::OnTickAnimation(int nGlobalAnimId)
{
	assert ((unsigned)nGlobalAnimId < m_arrAnims.size());
	LoadAnimation(nGlobalAnimId);
	m_arrAnims[nGlobalAnimId].OnTick();
}

void CControllerManager::OnApplyAnimation(int nGlobalAnimId)
{
	assert ((unsigned)nGlobalAnimId < m_arrAnims.size());
	LoadAnimation(nGlobalAnimId);
	m_arrAnims[nGlobalAnimId].OnApply();
}

// puts the size of the whole subsystem into this sizer object, classified,
// according to the flags set in the sizer
void CControllerManager::GetSize(class ICrySizer* pSizer)
{
#if ENABLE_GET_MEMORY_USAGE
	SIZER_SUBCOMPONENT_NAME(pSizer, "Keys");

	size_t nSize = sizeof(*this);
	if (m_nCachedSizeofThis)
		nSize = m_nCachedSizeofThis;
	else
	{
		// approximately calculating the size of the map
		unsigned nMaxNameLength = 0;
		nSize += sizeofArray (m_arrAnimByFile);
		for (AnimationArray::const_iterator it = m_arrAnims.begin(); it != m_arrAnims.end(); ++it)
			nSize += it->sizeofThis();

		m_nCachedSizeofThis = nSize;
	}
	pSizer->AddObject(this, nSize);

#endif
}

size_t CControllerManager::Animation::sizeofThis ()const
{
	size_t nSize = sizeof(*this) + sizeofArray (arrCtrls)+ strFileName.capacity() + 1;
	ControllerArray::const_iterator it = arrCtrls.begin(), itEnd = it + arrCtrls.size();
	for (; it != itEnd; ++it)
		nSize += (*it)->sizeofThis();
	return nSize;
}

void CControllerManager::selfValidate()
{
#ifdef _DEBUG
	assert (m_arrAnimByFile.size()==m_arrAnims.size());
	for (int i = 0; i < (int)m_arrAnimByFile.size()-1; ++i)
		assert (stricmp(m_arrAnims[m_arrAnimByFile[i]].strFileName.c_str(), m_arrAnims[m_arrAnimByFile[i+1]].strFileName.c_str()) < 0);
#endif
}


void CControllerManager::Update()
{
	if (g_GetCVars()->ca_AnimationUnloadDelay() < 30)
		return;

	if (m_nLastCheckedUnloadCandidate < m_arrAnims.size())
	{
		Animation &GlobalAnim = m_arrAnims[m_nLastCheckedUnloadCandidate];
		if (
			GlobalAnim.IsAutoUnload() &&
			GlobalAnim.nLastAccessFrameId + g_GetCVars()->ca_AnimationUnloadDelay() < g_nFrameID
			)
		{
			if (UnloadAnimation(m_nLastCheckedUnloadCandidate))
				if (g_GetCVars()->ca_Debug())
					g_GetLog()->LogToFile ("\004Unloaded animation %s", GlobalAnim.strFileName.c_str());
		}
		++m_nLastCheckedUnloadCandidate;
	}
	else
		m_nLastCheckedUnloadCandidate = 0;
}