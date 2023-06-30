
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectAI.h: interface for the CScriptObjectAI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTAI_H__3D4BC3E5_B60C_40DC_A819_17EE0F04C00A__INCLUDED_)
#define AFX_SCRIPTOBJECTAI_H__3D4BC3E5_B60C_40DC_A819_17EE0F04C00A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <IEntitySystem.h>
#include <IAISystem.h>
#include <ILog.h>
#include <map>

#include <_ScriptableEx.h>

struct IGoalPipe;

/*! This class implements script-functions for manipulating the AI system

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "AI".
	
	
	Example:
		AI.CreateGoalPipe("some_pipe");

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/

class CScriptObjectAI :
public _ScriptableEx<CScriptObjectAI>  
{
public:
	int SoundEvent(IFunctionHandler *pH);
	int PushGoal(IFunctionHandler *pH);
	int CreateGoalPipe( IFunctionHandler *pH);
	void Init(IScriptSystem *, ISystem *, CXGame *);
	CScriptObjectAI();
	virtual ~CScriptObjectAI();
	static void InitializeTemplate(IScriptSystem *pSS);
private:
	IAISystem *m_pAISystem;
	ILog *m_pLog;
	IEntitySystem *m_pEntitySystem;
	ISoundSystem *m_pSoundSystem;
	CXGame *m_pGame;
public:
	//! logs into special AI file :) 
	int Log(IFunctionHandler * pH);
	//! sends signal to ai objects
	int Signal(IFunctionHandler * pH);
	//! gets how many agents are in the specified group
	int GetGroupCount(IFunctionHandler * pH);
	int GetAttentionTargetOf(IFunctionHandler * pH);
	int ReloadAll(IFunctionHandler * pH);
	int MakePuppetIgnorant(IFunctionHandler * pH);
	int FreeSignal(IFunctionHandler * pH);
	int SetAssesmentMultiplier(IFunctionHandler * pH);
	int FindObjectOfType(IFunctionHandler * pH);
	int GetGroupOf(IFunctionHandler * pH);
	int GetAnchor(IFunctionHandler * pH);
	int GetPerception(IFunctionHandler * pH);
	int RegisterWithAI(IFunctionHandler * pH);
	int AIBind(IFunctionHandler * pH);
	int CreateBoundObject(IFunctionHandler *pH);
	int Cloak(IFunctionHandler * pH);
	int DeCloak(IFunctionHandler * pH);
	int ProjectileShoot(IFunctionHandler * pH);
	int SetTheSkip(IFunctionHandler * pH);
	int SetAllowedDeathCount(IFunctionHandler * pH);
	int Checkpoint(IFunctionHandler * pH);
	int RegisterPlayerHit(IFunctionHandler * pH);
	int FireOverride(IFunctionHandler * pH);
	int SetSpeciesThreatMultiplier(IFunctionHandler * pH);
	int EnablePuppetMovement(IFunctionHandler * pH);
	int IsMoving(IFunctionHandler * pH);
	int EnableNodesInSphere(IFunctionHandler * pH);

	int GetStats(IFunctionHandler * pH);
};

#endif // !defined(AFX_SCRIPTOBJECTAI_H__3D4BC3E5_B60C_40DC_A819_17EE0F04C00A__INCLUDED_)
