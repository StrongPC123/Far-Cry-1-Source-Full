//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:CryCharManager.h
//  Implementation of CryCharManager class
//
//	History:
//	August 16, 2002: Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRY_CHAR_MANAGER_HEADER_
#define _CRY_CHAR_MANAGER_HEADER_

#include <ICryAnimation.h>
#include "AnimObjectManager.h"

class CControllerManager;
class CryCharInstance;
class CAnimObjectManager;

#include "CryCharBody.h"

extern float g_YLine;

//////////////////////////////////////////////////////////////////////
// This class contains a list of character bodies and list of character instances.
// On attempt to create same character second time only new instance will be created.
// Only this class can create actual characters.
class CryCharManager : public ICryCharManager
{
public:
	CryCharManager (ISystem * pSystem);
	~CryCharManager ();

	// Loads a cgf and the corresponding caf file and creates an animated object,
	// or returns an existing object.
	virtual ICryCharInstance * MakeCharacter(const char * szFilename, unsigned nFlags);

	// loads the character model (which is ref-counted, so you must assign it to an autopointer)
	virtual ICryCharModel* LoadModel(const char* szFileName, unsigned nFlags = 0);

	// Reduces reference counter for object and deletes object if counter is 0
	virtual void RemoveCharacter (ICryCharInstance * pCryCharInstance, unsigned nFlags);  

	// Deletes itself
	virtual void Release();

	// returns the controller manager used by this character manager
	CControllerManager * GetControllerManager();

	void RegisterBody (CryCharBody*);
	void UnregisterBody (CryCharBody*);

	// puts the size of the whole subsystem into this sizer object, classified,
	// according to the flags set in the sizer
	void GetMemoryUsage(class ICrySizer* pSizer)const;

	//! Executes a script command
	//! Returns true if the command was executed, or false if not
	//! All the possible values for nCommand are in the CryAnimationScriptCommands.h
	//! file in the CryAnimationScriptCommandEnum enumeration. All the parameter/result
	//! structures are also there.
	bool ExecScriptCommand (int nCommand, void* pParams = NULL, void* pResult = NULL);

	// should be called every frame
	void Update();

	//! Cleans up all resources - currently deletes all bodies and characters (even if there are references on them)
	virtual void ClearResources();

	//! The specified animation will be unloaded from memory; it will be loaded back upon the first invokation (via StartAnimation())
	void UnloadAnimation (const char* szFileName);

	//! Starts loading the specified animation. fWhenRequired is the timeout, in seconds, from the current moment,
	//! when the animation data will actually be needed
	void StartLoadAnimation (const char* szFileName, float fWhenRequired);

	//! Unloads animations older than the given number of frames
	void UnloadOldAnimations (int numFrames);

	// returns statistics about this instance of character animation manager
	// don't call this too frequently
	virtual void GetStatistics(Statistics& rStats);

	//! Locks all models in memory
	void LockResources();

	//! Unlocks all models in memory
	void UnlockResources();
protected:

	// destroys all characters
	void CleanupInstances();

	// asserts the cache validity
	void ValidateBodyCache();

	// deletes all registered bodies; the character instances got deleted even if they're still referenced
	void CleanupBodyCache();

	// Finds a cached or creates a new CryCharBody instance and returns it
	// returns NULL if the construction failed
	CryCharBody* FetchBody (const string& strFileName);

	// locks or unlocks the given body if needed, depending on the hint and console variables
	void DecideModelLockStatus(CryCharBody* pBody, unsigned nHints);
	void ClearDecals();
private:
	CControllerManager * m_pControllerManager;
	// manager of animated objects.
	CAnimObjectManager * m_pAnimObjectManager;

	class OrderByFileName
	{
	public:
		bool operator () (const CryCharBody* pLeft, const CryCharBody* pRight);
		bool operator () (const string& strLeft, const CryCharBody* pRight);
		bool operator () (const CryCharBody* pLeft, const string& strRight);
	};
	// this is the set of all existing instances of crycharbody
	typedef std::vector<CryCharBody*> CryCharBodyCache;
	CryCharBodyCache m_arrBodyCache;

	// temporary locked objects with LockResources
	CryCharBody_AutoArray m_arrTempLock;

	//typedef std::vector<CryCharBody_AutoPtr> CryCharBodyAutoArray;
	// this is the array of bodies that are kept locked even if the number of characters drops to zero
	CryCharBody_AutoSet m_setLockedBodies;
};


#endif