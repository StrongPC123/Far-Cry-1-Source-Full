#include "stdafx.h"
#include "ScriptObjectAnimation.h"
#include "CryAnimationScriptCommands.h"
#include <ISystem.h>
#include <I3DEngine.h>
#include <ICryAnimation.h>

#define GET_ANIM_MANAGER(pAnimationManager) 	ICryCharManager* pAnimationManager = getAnimationManager(); \
	if (!pAnimationManager)																																									\
	{																																																				\
		m_pScriptSystem->RaiseError("System. No Animation System available", pH->GetParamCount());						\
		return 0;																																															\
	}

#undef REG_FUNC
#define REG_FUNC(_func) CScriptObjectAnimation::RegisterFunction(pSS,#_func,&CScriptObjectAnimation::_func);

_DECLARE_SCRIPTABLEEX(CScriptObjectAnimation)

CScriptObjectAnimation::CScriptObjectAnimation(void)
{
}

CScriptObjectAnimation::~CScriptObjectAnimation(void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Initializes the script-object and makes it available for the scripts.
		@param pScriptSystem Pointer to the ScriptSystem-interface
		@param pGame Pointer to the Game
		@param pSystem Pointer to the System-interface
*/
void CScriptObjectAnimation::Init(IScriptSystem *pScriptSystem,  ISystem *pSystem)
{
	m_pSystem = pSystem;

	m_pScriptSystem = pScriptSystem;
	InitGlobal(pScriptSystem,"Animation",this);
}

void CScriptObjectAnimation::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectAnimation>::InitializeTemplate(pSS);
	REG_FUNC(DumpAnims);
	REG_FUNC(DumpModels);
	REG_FUNC(TestParticles);
	REG_FUNC(StopParticles);
	REG_FUNC(TrashAnims);
	REG_FUNC(UnloadAnim);
	REG_FUNC(ClearDecals);
	REG_FUNC(DumpDecals);
	REG_FUNC(Start2Anims);
	REG_FUNC(DumpStates);
	REG_FUNC(ExportModels);
}

ICryCharManager* CScriptObjectAnimation::getAnimationManager()
{
	return m_pSystem->GetIAnimationSystem();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Dumps memory usage by animations
		@param nSort first operand (int) 0: sort by memory size 1: sort by name (not implemented)
		@return result of the operation (int)
*/
int CScriptObjectAnimation::DumpAnims(IFunctionHandler *pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	switch (pH->GetParamCount())
	{
	case 0:
		{
			pAnimationManager->ExecScriptCommand(CASCMD_DUMP_ANIMATIONS);
		}
		break;
	case 1:
		{
			int nSort;
			pH->GetParam(1,nSort);
		}
		break;
	default:
		m_pScriptSystem->RaiseError("System.DumpAnims wrong number of arguments (%d)", pH->GetParamCount());
		return pH->EndFunctionNull();
	}
	return pH->EndFunction(0);
}


int CScriptObjectAnimation::TrashAnims(IFunctionHandler *pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	switch (pH->GetParamCount())
	{
	case 0:
		{
			pAnimationManager->ExecScriptCommand(CASCMD_TRASH_ANIMATIONS);
		}
		break;

	case 1:
		{
			int numFrames;
			if (pH->GetParam(1,numFrames))
				pAnimationManager->ExecScriptCommand(CASCMD_TRASH_ANIMATIONS, &numFrames);
			else
				pAnimationManager->ExecScriptCommand(CASCMD_TRASH_ANIMATIONS);
		}

	default:
		m_pScriptSystem->RaiseError("System.TrashAnims wrong number of arguments (%d)", pH->GetParamCount());
		return pH->EndFunctionNull();
	}
	return pH->EndFunction(0);
}

int CScriptObjectAnimation::ClearDecals(IFunctionHandler* pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	switch (pH->GetParamCount())
	{
	case 0:
		pAnimationManager->ExecScriptCommand(CASCMD_CLEAR_DECALS);
		break;
	default:
		m_pScriptSystem->RaiseError("System.ClearDecals wrong number of arguments (%d)", pH->GetParamCount());
		return pH->EndFunctionNull();
	}
	return pH->EndFunction(0);
}

int CScriptObjectAnimation::DumpDecals(IFunctionHandler* pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	switch (pH->GetParamCount())
	{
	case 0:
		pAnimationManager->ExecScriptCommand(CASCMD_DUMP_DECALS);
		break;
	default:
		m_pScriptSystem->RaiseError("System.DumpDecals wrong number of arguments (%d)", pH->GetParamCount());
		return pH->EndFunctionNull();
	}
	return pH->EndFunction(0);
}

int CScriptObjectAnimation::Start2Anims(IFunctionHandler* pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	switch (pH->GetParamCount())
	{
	case 5:
		{
			CASCmdStartAnim sa[2];
			CASCmdStartMultiAnims ma;
			if (!pH->GetParam (1, sa[0].nLayer)
				||!pH->GetParam (2, sa[0].szAnimName)
				||!pH->GetParam (3, sa[1].nLayer)
				||!pH->GetParam (4, sa[1].szAnimName)
				||!pH->GetParam (5, ma.fBlendTime))
				m_pScriptSystem->RaiseError("System.UnloadAnim wrong argument type");
			ma.numAnims = 2;
			ma.pAnims = sa;
			pAnimationManager->ExecScriptCommand(CASCMD_START_MANY_ANIMS, &ma);
		}
		break;
	default:
		m_pScriptSystem->RaiseError("System.Start2Anims wrong number of arguments (%d), use: Start2Anims(layer,anim,layer,anim,blendin)", pH->GetParamCount());
		return pH->EndFunctionNull();
	}
	return pH->EndFunction(0);
}


int CScriptObjectAnimation::DumpStates(IFunctionHandler* pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	switch (pH->GetParamCount())
	{
	case 0:
		pAnimationManager->ExecScriptCommand(CASCMD_DUMP_STATES);
		break;
	default:
		m_pScriptSystem->RaiseError("System.DumpStates wrong number of arguments (%d)", pH->GetParamCount());
		return pH->EndFunctionNull();
	}
	return pH->EndFunction(0);
}

int CScriptObjectAnimation::ExportModels(IFunctionHandler* pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	pAnimationManager->ExecScriptCommand(CASCMD_EXPORT_MODELS_ASCII);
	return pH->EndFunction(0);
}


int CScriptObjectAnimation::UnloadAnim(IFunctionHandler *pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	switch (pH->GetParamCount())
	{
	case 1:
		{
			const char* pName;
			if (pH->GetParam(1,pName))
				pAnimationManager->ExecScriptCommand(CASCMD_UNLOAD_ANIMATION, (void*)pName);
			else
			{
				m_pScriptSystem->RaiseError("System.UnloadAnim wrong argument type (%d) - must be a string", pH->GetParamType(1));
				return pH->EndFunctionNull();
			}
		}

	default:
		m_pScriptSystem->RaiseError("System.UnloadAnim wrong number of arguments (%d)", pH->GetParamCount());
		return pH->EndFunctionNull();
	}
	return pH->EndFunction(0);
}


int CScriptObjectAnimation::DumpModels(IFunctionHandler *pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	pAnimationManager->ExecScriptCommand(CASCMD_DUMP_MODELS);
	return pH->EndFunction(0);
}

/*! Starts spawning particles
		@param fSpawnRate first operand (float) rate of particle spawn
		@return result of the operation (int)
*/
int CScriptObjectAnimation::TestParticles(IFunctionHandler *pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	pAnimationManager->ExecScriptCommand (CASCMD_TEST_PARTICLES);
	return pH->EndFunction(0);
}

/*! Starts spawning particles
		@param fSpawnRate first operand (float) rate of particle spawn
		@return result of the operation (int)
*/
int CScriptObjectAnimation::StopParticles(IFunctionHandler *pH)
{
	GET_ANIM_MANAGER(pAnimationManager);
	pAnimationManager->ExecScriptCommand (CASCMD_STOP_PARTICLES);
	return pH->EndFunction(0);
}

