#include "stdafx.h"
#include "scriptobjectmovie.h"
#include <ISystem.h>
#include <IMovieSystem.h>
#include <IGame.h>

_DECLARE_SCRIPTABLEEX(CScriptObjectMovie)

//////////////////////////////////////////////////////////////////////////
CScriptObjectMovie::CScriptObjectMovie()
{
}

//////////////////////////////////////////////////////////////////////////
CScriptObjectMovie::~CScriptObjectMovie()
{
}

/*! Initializes the script-object and makes it available for the scripts.
		@param pScriptSystem Pointer to the ScriptSystem-interface
		@param pGame Pointer to the Game
*/
//////////////////////////////////////////////////////////////////////////
void CScriptObjectMovie::Init(IScriptSystem *pScriptSystem, ISystem *pSystem)
{
	m_pSystem=pSystem;
	m_pMovieSystem=m_pSystem->GetIMovieSystem();	
	InitGlobal(pScriptSystem, "Movie", this);
}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectMovie::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectMovie>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectMovie,PlaySequence);
	REG_FUNC(CScriptObjectMovie,StopSequence);
	REG_FUNC(CScriptObjectMovie,StopAllSequences);
	REG_FUNC(CScriptObjectMovie,StopAllCutScenes);
	REG_FUNC(CScriptObjectMovie,PauseSequences);
	REG_FUNC(CScriptObjectMovie,ResumeSequences);
}

/*! Start a sequence
		@param pszName Name of sequence
*/
//////////////////////////////////////////////////////////////////////////
int CScriptObjectMovie::PlaySequence(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(1);
	if (pH->GetParamCount()<1)
		return pH->EndFunction();

	const char *pszName;
	pH->GetParam(1, pszName);

	IGame *pGame=m_pSystem->GetIGame();
	if (!pGame) 
	{
		// can this happen?
		return pH->EndFunction();
	}

	bool bResetFx=true;
	if (pH->GetParamCount()==2)
		pH->GetParam(2, bResetFx);

	m_pSystem->GetIMovieSystem()->PlaySequence(pszName,bResetFx);

	return pH->EndFunction();
}

/*! Stop a sequence
		@param pszName Name of sequence
*/
//////////////////////////////////////////////////////////////////////////
int CScriptObjectMovie::StopSequence(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *pszName;
	pH->GetParam(1, pszName);
	m_pMovieSystem->StopSequence(pszName);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectMovie::StopAllSequences(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pMovieSystem->StopAllSequences();
	return pH->EndFunction();
}
 
//////////////////////////////////////////////////////////////////////////
int CScriptObjectMovie::StopAllCutScenes(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);	
	IGame *pGame=m_pSystem->GetIGame();
	if (!pGame) 
	{
		// can this happen?
		return pH->EndFunction();		
	}

	//pGame->StopCurrentCutscene();
	//m_pMovieSystem->StopAllCutScenes();
	m_pSystem->GetIMovieSystem()->StopAllCutScenes();

	return pH->EndFunction();
}

int CScriptObjectMovie::PauseSequences(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);	

	m_pMovieSystem->Pause();
	return pH->EndFunction();
}

int CScriptObjectMovie::ResumeSequences(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pMovieSystem->Resume();
	return pH->EndFunction();
}
