
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectInput.h: interface for the CScriptObjectInput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTINPUT_H__18286CA7_21F2_45E0_9DFF_9D67F6AE3BE8__INCLUDED_)
#define AFX_SCRIPTOBJECTINPUT_H__18286CA7_21F2_45E0_9DFF_9D67F6AE3BE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
class CXGame;

/*! This class implements all input-related script-functions.

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "Input".
	
	Example:
		Input:BindAction("MOVE_LEFT","a");

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectInput :
public _ScriptableEx<CScriptObjectInput> 
{
public:
	CScriptObjectInput();
	virtual ~CScriptObjectInput();
	void Init(IScriptSystem *pScriptSystem,CXGame *pGame,ISystem *pSystem);
	static void InitializeTemplate(IScriptSystem *pSS);
	int ResetToDefaults(IFunctionHandler *pH);
	int ResetAllBindings(IFunctionHandler *pH);
	int BindCommandToKey(IFunctionHandler *pH);
	int BindAction(IFunctionHandler *pH);
	int BindActionMultipleMaps(IFunctionHandler *pH);
	//int CheckForAction(IFunctionHandler *pH);
	int ClearAction(IFunctionHandler *pH);
	int GetActionMaps(IFunctionHandler *pH);
	int ResetBinding(IFunctionHandler *pH);
	int GetBinding(IFunctionHandler *pH);
	int SetActionMap(IFunctionHandler *pH);
	int SetMouseSensitivity(IFunctionHandler *pH);
	int GetMouseSensitivity(IFunctionHandler *pH);
	int SetMouseSensitivityScale(IFunctionHandler *pH);
	int GetMouseSensitivityScale(IFunctionHandler *pH);
	int GetXKeyPressedName(IFunctionHandler *pH);
	int GetXKeyDownName(IFunctionHandler *pH);
	int ResetKeyState(IFunctionHandler *pH);
	int SetInvertedMouse(IFunctionHandler *pH);
	int GetInvertedMouse(IFunctionHandler *pH);
	
private:
	CXGame *m_pGame;
	ISystem *m_pSystem;
	IConsole *m_pConsole;
	IInput *m_pInput;
};

#endif // !defined(AFX_SCRIPTOBJECTINPUT_H__18286CA7_21F2_45E0_9DFF_9D67F6AE3BE8__INCLUDED_)
