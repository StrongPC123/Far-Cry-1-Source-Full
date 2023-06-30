// ScriptObjectScript.h: interface for the CScriptObjectScript class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTSCRIPT_H__2432459C_19FC_4AC4_8EAA_D73967BC4B37__INCLUDED_)
#define AFX_SCRIPTOBJECTSCRIPT_H__2432459C_19FC_4AC4_8EAA_D73967BC4B37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
#include <set>												// STL set<>

/*! This class implements script-functions for exposing the scripting system functionalities

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "Script".
	
	Example:
		Script.LoadScript("scripts/common.lua")

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/

class CScriptObjectScript :
public _ScriptableEx<CScriptObjectScript> 
{
public:
	CScriptObjectScript();
	virtual ~CScriptObjectScript();
	void Init(IScriptSystem *pScriptSystem); 
	int LoadScript(IFunctionHandler *pH);
	int ReloadScripts(IFunctionHandler *pH);
	int ReloadScript(IFunctionHandler *pH);
	int UnloadScript(IFunctionHandler *pH);
	int DumpLoadedScripts(IFunctionHandler *pH);
	int Debug(IFunctionHandler *pH);
	int DebugFull(IFunctionHandler *pH);
	static void InitializeTemplate(IScriptSystem *pSS);

private: // -------------------------------------------------------------------------

	//! recursive
	//! /return amount of table elements (recursive)
	DWORD Debug_Buckets_recursive( IScriptObject *pCurrent, string &sPath, std::set<const void *> &setVisited, const DWORD dwMinBucket );

	//! not recursive
	void Debug_Elements( IScriptObject *pCurrent, string &sPath, std::set<const void *> &setVisited );

	//! recursive
	void Debug_Full_recursive( IScriptObject *pCurrent, string &sPath, std::set<const void *> &setVisited );
};

#endif // !defined(AFX_SCRIPTOBJECTSCRIPT_H__2432459C_19FC_4AC4_8EAA_D73967BC4B37__INCLUDED_)
