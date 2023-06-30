
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef _SCRIPTOBJECTADVCAMSYSTEM_H_
#define _SCRIPTOBJECTADVCAMSYSTEM_H_

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
class CAdvCamSystem;

/*! In this class are all AdvCamSystem-related script-functions implemented.

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectAdvCamSystem : public _ScriptableEx<CScriptObjectAdvCamSystem>, public IScriptObjectSink
{
public:
	//! constructor
	CScriptObjectAdvCamSystem( void ) {}
	//! destructor
	virtual ~CScriptObjectAdvCamSystem( void ) {}

	//!
	bool Create(IScriptSystem *pScriptSystem, CAdvCamSystem *pAdvCamSystem);

	//!
	void OnRelease()
	{
		m_pScriptThis=NULL;
		delete this;
	}

	// is called from CXGame::Reset() to add the scripting 
	static void InitializeTemplate(IScriptSystem *pSS);

	// script functions -----------------------------------------------

	int SetPlayerA( IFunctionHandler *pH );
	int GetPlayerA( IFunctionHandler *pH );
	int SetPlayerB( IFunctionHandler *pH );
	int GetPlayerB( IFunctionHandler *pH );

	int SetMaxRadius( IFunctionHandler *pH );
	int SetMinRadius( IFunctionHandler *pH );

private:
	CAdvCamSystem *m_pAdvCamSystem;
};

#endif //_SCRIPTOBJECTADVCAMSYSTEM_H_