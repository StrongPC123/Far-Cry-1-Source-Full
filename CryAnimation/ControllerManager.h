/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//	
//  Notes:
//    CControllerManager class declaration extracted from File:CryModelState.h
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRYTEK_CONTROLLER_MANAGER_HEADER_
#define _CRYTEK_CONTROLLER_MANAGER_HEADER_

#include "Controller.h"
#include "CryAnimationInfo.h"
#include "FileMapping.h"
#include "IStreamEngine.h"

class CryModelAnimationContainer;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// class CControllerManager 
// Responsible for creation of multiple animations, subsequently bind to the character bones
// There is one instance of this class per a game.
/////////////////////////////////////////////////////////////////////////////////////////////////////
class CControllerManager : public IStreamCallback
{
public:
	// array of animations, each animation having its name and id
	// Animation is an array of controllers. Animation id is an index in the array
	
	typedef GlobalAnimation Animation;

	struct AnimationIdPred
	{
		const std::vector<Animation>& m_arrAnims;
		AnimationIdPred(const std::vector<Animation>& arrAnims):
			m_arrAnims(arrAnims)
			{
			}
			
		bool operator () (int left, int right)const 
		{
			return stricmp(m_arrAnims[left].strFileName.c_str(), m_arrAnims[right].strFileName.c_str()) < 0;
		}
		bool operator () (int left, const char* right)const 
		{
			return stricmp(m_arrAnims[left].strFileName.c_str(), right)<0;
		}
		bool operator () (const char* left, int right)const 
		{
			return stricmp(left, m_arrAnims[right].strFileName.c_str())<0;
		}
	};

	// returns the structure describing the animation data, given the global anim id
	Animation& GetAnimation (int nAnimID);

	CControllerManager();

	// loads the animation with the specified name; if the animation is already loaded,
	// then just returns its id
	// The caller MUST TAKE CARE to bind the animation if it's already loaded before it has registered itself within this manager
	int StartLoadAnimation (const string& strFileName, float fScale, unsigned nFlags = Animation::FLAGS_DEFAULT_FLAGS);

	// unreferences the controllers and makes the animation unloaded
	// before this operation, all bones must be unbound
	bool UnloadAnimation (int nGlobAnimId);
	
	// loads existing animation record, returns false on error
	bool LoadAnimation(int nGlobalAnimId);

	// loads the animation info, if not loaded yet
	bool LoadAnimationInfo(int nGlobalAnimId);

	// updates the animation from the chunk of AnimInfo
	bool UpdateAnimation (int nGlobalAnimId, const struct CCFAnimInfo* pAnimInfo);

	// notifies the controller manager that another client uses the given animation.
	// these calls must be balanced with AnimationRelease() calls
	void AnimationAddRef (int nGlobalAnimId, CryModelAnimationContainer* pClient);


	// notifies the controller manager that this client doesn't use the given animation any more.
	// these calls must be balanced with AnimationAddRef() calls
	void AnimationRelease (int nGlobalAnimId, CryModelAnimationContainer* pClient);

	void OnStartAnimation (int nGlobalAnimId);
	void OnTickAnimation(int nGlobalAnimId);
	void OnApplyAnimation(int nGlobalAnimId);

	// returns the total number of animations hold in memory (for statistics)
	unsigned NumAnimations();
  
	// finds controller with the given nControllerID among controller in the animation
	// identified by nGlobalAnimID
	IController* GetController (int nGlobalAnimID, unsigned nControllerID);

  ~CControllerManager();

	// logs controller usage statistics
	void LogUsageStats();

	// puts the size of the whole subsystem into this sizer object, classified,
	// according to the flags set in the sizer
	void GetSize(class ICrySizer* pSizer);

	// dumps the used animations
	void DumpAnims();

	void Update();

	void Register (CryModelAnimationContainer* pClient)
	{
		m_arrClients.insert (std::lower_bound(m_arrClients.begin(), m_arrClients.end(), pClient), pClient);
	}
	void Unregister (CryModelAnimationContainer* pClient)
	{
		std::vector<CryModelAnimationContainer*>::iterator it = std::lower_bound(m_arrClients.begin(), m_arrClients.end(), pClient);
		if(it != m_arrClients.end() && *it == pClient)
			m_arrClients.erase (it);
		else
			assert (0); // the unregistered client tries to unregister
	}

	// finds the animation by name. Returns -1 if no animation was found
	// Returns the animation ID if it was found
	int FindAnimationByFile(const string& sAnimFileName);

protected:
	// notifies all clients that the animation or its info has been loaded
	void FireAnimationGlobalLoad (int nAnimId);

	// immediately load the given animation from the already opened reader
	bool LoadAnimation(int nAnimId, class CChunkFileReader* pReader);

	// these two are called back when an asynchronous IO operation has finished
	void StreamOnComplete (IReadStream* pStream, unsigned nError);

	// this structure describes the pending animation load request: it exists as long as the animation
	// is being loaded (asynchronously)
	struct PendingAnimLoad:public _reference_target_t
	{
	public:
		PendingAnimLoad (int animId){nAnimId = animId;}
		PendingAnimLoad (){}
		// the data that's being loaded
		CFileMapping_AutoPtr pFile;
		// the animation for which the data is loaded
		int nAnimId;
		// the frame at which the animation load was started
		int nFrameId;
		// the stream that loads the data
		IReadStreamPtr pStream;
	};
	TYPEDEF_AUTOPTR(PendingAnimLoad);

	// the set of pending animation load requests; they get deleted as soon as the IO operation is completed
	std::set<PendingAnimLoad_AutoPtr> m_setPendingAnimLoads;

	// Loads the controller from the chunk, returns the result in the autopointer
	// It is important that the return type is autoptr: it lets the procedure gracefully destruct 
	// half-constructed animation in case of an error.
  IController_AutoPtr LoadController(float fScale, const CONTROLLER_CHUNK_DESC_0826*pChunk, int nSize);

	// array of animations
	// since the number of animations dynamically changes and STL vector has good balanced
	// reallocation properties, we use it here
	typedef std::vector<Animation> AnimationArray;
	AnimationArray m_arrAnims;
	
	// index array for the animation indices sorted by file name
	std::vector<int>m_arrAnimByFile;

	void selfValidate();

	size_t m_nCachedSizeofThis;

	// the sorted list of pointers to the clients that use the services of this manager
	std::vector<CryModelAnimationContainer*> m_arrClients;

	// this scans through all animations cyclicly each frame and checks which ones should be unloaded
	// due to not being used
	unsigned m_nLastCheckedUnloadCandidate;
};													 

#endif