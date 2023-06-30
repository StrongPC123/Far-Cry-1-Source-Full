#pragma once
#include <_ScriptableEx.h>
#include "IScriptSystem.h"
struct IEntity;
class CScriptObjectMovie : 
public _ScriptableEx<CScriptObjectMovie>
{
public:
	CScriptObjectMovie();
	virtual ~CScriptObjectMovie();
	static void InitializeTemplate(IScriptSystem *pSS);
	void Init(IScriptSystem *pScriptSystem, ISystem *pSystem);
	int PlaySequence(IFunctionHandler *pH);
	int StopSequence(IFunctionHandler *pH);
	int StopAllSequences(IFunctionHandler *pH);
	int StopAllCutScenes(IFunctionHandler *pH);
	int PauseSequences(IFunctionHandler *pH);
	int ResumeSequences(IFunctionHandler *pH);
private:
	ISystem *m_pSystem;
	IMovieSystem *m_pMovieSystem;
};
