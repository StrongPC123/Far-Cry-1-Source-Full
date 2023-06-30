/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created 12/05/2002 by Sergiy Migdalskiy
//
//  This is the class that's used to implement support for multiple
//  shadow volumes per character, and the policy of double- or single-buffered
//  shadow volumes for performance optimization
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_CHAR_RE_SHADOW_MANAGER_HDR_
#define _CRY_CHAR_RE_SHADOW_MANAGER_HDR_

class CryCharReShadowVolume;

class CryCharReShadowManager
{
public:
	CryCharReShadowManager ();
	~CryCharReShadowManager ();
	
	// creates a new shadow volume object or retrieves an old one from the pool
	// May return NULL, if insufficient resources; in this case, no further action on
	// creating shadow volumes must be attempted
	CryCharReShadowVolume* newShadow ();

	// cleans up the shadow volume resources that weren't used lately
	// (collect the garbage)
	void shrink();

	void GetMemoryUsage (ICrySizer* pSizer);
protected:
	// the pool of shadow volume objects
	std::vector<CryCharReShadowVolume*> m_arrPool;
};

#endif