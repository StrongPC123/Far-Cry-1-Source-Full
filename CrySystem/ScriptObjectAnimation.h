// ScriptObjectAnimation.h: interface for the CScriptObjectAnimation class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __CRY_GAME_SCRIPT_OBJECT_ANIMATION_HDR__
#define __CRY_GAME_SCRIPT_OBJECT_ANIMATION_HDR__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
struct ISystem;
struct ICryCharManager;
/*! This class implements all animation-related (currently only test) script functions.

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "Animation".
	
	Example:
		Animation:DumpAnims();

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/

class CScriptObjectAnimation:
	public _ScriptableEx<CScriptObjectAnimation>
{
public:
	CScriptObjectAnimation(void);
	virtual ~CScriptObjectAnimation(void);
	void Init(IScriptSystem *pScriptSystem, ISystem *pSystem);
	static void InitializeTemplate(IScriptSystem *pSS);

public:
	int DumpAnims(IFunctionHandler *pH);
	int DumpModels (IFunctionHandler *pH);
	int TestParticles (IFunctionHandler *pH);
	int StopParticles (IFunctionHandler *pH);
	int TrashAnims(IFunctionHandler* pH);
	int UnloadAnim(IFunctionHandler* pH);
	int ClearDecals(IFunctionHandler* pH);
	int DumpDecals(IFunctionHandler* pH);
	int Start2Anims(IFunctionHandler* pH);
	int DumpStates (IFunctionHandler* pH);
	int ExportModels(IFunctionHandler* pH);

	ICryCharManager* getAnimationManager();

private:
	ISystem *m_pSystem;
};

#endif