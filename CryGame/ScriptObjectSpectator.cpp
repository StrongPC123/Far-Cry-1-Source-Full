
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectSpectator.cpp
//
//  Description: 
//		ScriptObjectSpectator.cpp: implementation of the ScriptObjectSpectator class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scriptobjectspectator.h"
#include "Spectator.h"

_DECLARE_SCRIPTABLEEX(CScriptObjectSpectator)

//////////////////////////////////////////////////////////////////////////
CScriptObjectSpectator::CScriptObjectSpectator(void)
{
}

//////////////////////////////////////////////////////////////////////////
CScriptObjectSpectator::~CScriptObjectSpectator(void)
{
}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectSpectator::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectSpectator>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectSpectator,SetHost);
	REG_FUNC(CScriptObjectSpectator,GetHost);
}

//////////////////////////////////////////////////////////////////////////
bool CScriptObjectSpectator::Create(IScriptSystem *pScriptSystem, CSpectator *pSpectator)
{
	m_pSpectator=pSpectator;
	Init(pScriptSystem, this);
	m_pScriptThis->RegisterParent(this);
	// Function-Registration
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSpectator::SetHost(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int host=0;
	if(pH->GetParam(1,host))
	{
		m_pSpectator->m_eiHost=host;
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSpectator::GetHost(IFunctionHandler *pH)
{
	return pH->EndFunction(m_pSpectator->m_eiHost);
}