
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef _SCRIPTOBJECTSPECTATOR_H_
#define _SCRIPTOBJECTSPECTATOR_H_

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
class CSpectator;

/*! In this class are all spectator-related script-functions implemented.

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectSpectator :
public _ScriptableEx<CScriptObjectSpectator>,
public IScriptObjectSink
{
public:
	CScriptObjectSpectator(void);
	virtual ~CScriptObjectSpectator(void);
	bool Create(IScriptSystem *pScriptSystem, CSpectator *pSpectator);
	void OnRelease()
	{
		m_pScriptThis=NULL;
		delete this;
	}
	static void InitializeTemplate(IScriptSystem *pSS);
	int SetHost(IFunctionHandler *pH);
	int GetHost(IFunctionHandler *pH);
private:
	CSpectator *m_pSpectator;
};

#endif //_SCRIPTOBJECTSPECTATOR_H_