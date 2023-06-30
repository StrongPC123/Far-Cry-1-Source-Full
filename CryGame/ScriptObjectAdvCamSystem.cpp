
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectAdvCamSystem.cpp
//
//  Description:  
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scriptobjectAdvCamSystem.h"
#include "AdvCamSystem.h"

//////////////////////////////////////////////////////////////////////////
_DECLARE_SCRIPTABLEEX(CScriptObjectAdvCamSystem)

//////////////////////////////////////////////////////////////////////////
void CScriptObjectAdvCamSystem::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectAdvCamSystem>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectAdvCamSystem,SetPlayerA);
	REG_FUNC(CScriptObjectAdvCamSystem,GetPlayerA);
	REG_FUNC(CScriptObjectAdvCamSystem,SetPlayerB);
	REG_FUNC(CScriptObjectAdvCamSystem,GetPlayerB);
	REG_FUNC(CScriptObjectAdvCamSystem,SetMaxRadius);
	REG_FUNC(CScriptObjectAdvCamSystem,SetMinRadius);
}

bool CScriptObjectAdvCamSystem::Create(IScriptSystem *pScriptSystem, CAdvCamSystem *pAdvCamSystem)
{
	m_pAdvCamSystem=pAdvCamSystem;
	Init(pScriptSystem, this);
	m_pScriptThis->RegisterParent(this);
	// Function-Registration
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectAdvCamSystem::SetPlayerA(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int host=0;

	if(pH->GetParam(1,host))
		m_pAdvCamSystem->m_eiPlayerA=host;

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectAdvCamSystem::GetPlayerA(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pAdvCamSystem->m_eiPlayerA);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectAdvCamSystem::SetPlayerB(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int host=0;

	if(pH->GetParam(1,host))
		m_pAdvCamSystem->m_eiPlayerB=host;

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectAdvCamSystem::GetPlayerB(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pAdvCamSystem->m_eiPlayerB);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectAdvCamSystem::SetMaxRadius(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float radius = 0;

	if(pH->GetParam(1, radius))
		m_pAdvCamSystem->SetMaxRadius(radius);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectAdvCamSystem::SetMinRadius(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float radius = 0;

	if(pH->GetParam(1, radius))
		m_pAdvCamSystem->SetMinRadius(radius);

	return pH->EndFunction();
}
