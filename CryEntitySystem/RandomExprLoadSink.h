
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <IScriptSystem.h>
#include "LipSync.h"

#define EXPRLOAD_MODE_BASE	1

class CRandomExprLoadSink : public IScriptObjectDumpSink
{
private:
	bool m_bRaiseError;
	IScriptSystem *m_pScriptSystem;
	_SmartScriptObject *m_pObj;
	IAnimationSet *m_pAnimSet;
	TExprPatternVec *m_pvecExprPatterns;
	int m_nMode;
public:
	CRandomExprLoadSink(bool bRaiseError, IScriptSystem *pScriptSystem, _SmartScriptObject *pObj, IAnimationSet *pAnimSet, TExprPatternVec *pvecExprPatterns, int nMode=EXPRLOAD_MODE_BASE);
	~CRandomExprLoadSink();
	void OnElementFound(const char *sName, ScriptVarType type);
	void OnElementFound(int nIdx, ScriptVarType type) {}
};
