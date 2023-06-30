//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:3DEngine.cpp
//  Description: Implementation of C3DEngine class
//  Level loading, implementation of I3DEngine interface methods
//
//	History:
//	-:Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CryAnimationBase.h"
#include "cvars.h"

CryAnimVars::CryAnimVars ()
{
	IConsole* pConsole = g_GetConsole();
	// if you get an assertion here, it means some variable couldn't be registered
#define DECLARE_INT_VARIABLE_IMMEDIATE(_var, _default,_comment) m_##_var = _default; m_p_##_var = attach((#_var), &(m_##_var), _comment, VF_CHEAT|CVF_CHANGE_SOURCE);
#define DECLARE_INT_VARIABLE_IMMEDIATE_DUMP(_var, _default,_comment) m_##_var = _default; m_p_##_var = attach((#_var), &(m_##_var), _comment, VF_DUMPTODISK);
#define DECLARE_STRING_VARIABLE(_var, _default) m_##_var = pConsole->CreateVariable(#_var, (_default), VF_CHEAT); assert(m_##_var)
#define DECLARE_INT_VARIABLE(_var, _default) m_##_var = declare (#_var,_default,VF_CHEAT);
#define DECLARE_FLOAT_VARIABLE(_var, _default,_comment) m_##_var = pConsole->CreateVariable(#_var, (#_default), VF_CHEAT, _comment); assert(m_##_var)
#define DECLARE_FLOAT_VARIABLE_DUMP(_var, _default,_comment) m_##_var = pConsole->CreateVariable(#_var, (#_default), VF_DUMPTODISK, _comment); assert(m_##_var)
#include "CVars-list.h"
#undef DECLARE_STRING_VARIABLE
#undef DECLARE_INT_VARIABLE
#undef DECLARE_FLOAT_VARIABLE
#undef DECLARE_FLOAT_VARIABLE_DUMP
#undef DECLARE_INT_VARIABLE_IMMEDIATE
#undef DECLARE_INT_VARIABLE_IMMEDIATE_DUMP
}

CryAnimVars::~CryAnimVars ()
{
#define DECLARE_INT_VARIABLE_IMMEDIATE(_var, _default, _comment) detach((#_var),&(m_##_var));
#define DECLARE_INT_VARIABLE_IMMEDIATE_DUMP(_var, _default, _comment) DECLARE_INT_VARIABLE_IMMEDIATE(_var, _default, _comment)
#define DECLARE_STRING_VARIABLE(_var, _default) //m_##_var->Release()
#define DECLARE_INT_VARIABLE(_var, _default) //m_##_var->Release()
#define DECLARE_FLOAT_VARIABLE(_var, _default,_comment) //m_##_var->Release()
#define DECLARE_FLOAT_VARIABLE_DUMP(_var, _default,_comment) //m_##_var->Release()
#include "CVars-list.h"
#undef DECLARE_STRING_VARIABLE
#undef DECLARE_INT_VARIABLE
#undef DECLARE_FLOAT_VARIABLE
#undef DECLARE_FLOAT_VARIABLE_DUMP
#undef DECLARE_INT_VARIABLE_IMMEDIATE
#undef DECLARE_INT_VARIABLE_IMMEDIATE_DUMP
}

float CryAnimVars::ca_MinVertexWeightLOD (unsigned nLOD)
{
	switch (nLOD)
	{
		case 0:
			return ca_MinVertexWeightLOD0();
		case 1:
			return ca_MinVertexWeightLOD1();
		default:
			return ca_MinVertexWeightLOD2();
	}	
}

float CryAnimVars::ca_UpdateSpeed (unsigned nLayer)
{
	switch (nLayer)
	{
	case 0:
		return ca_UpdateSpeed0();
	case 1:
		return ca_UpdateSpeed1();
	case 2:
		return ca_UpdateSpeed2();
	case 3:
		return ca_UpdateSpeed3();
	case 4:
		return ca_UpdateSpeed4();
	default:
		return 1;
	}
}


//////////////////////////////////////////////////////////////////////////
// declares or retrieves the pointer to the console variable
// with the given default value (used only in case the variable doesn't exist now)
// returns the pointer to the preexisting or newly created variable.
ICVar* CryAnimVars::declare (const char* szVarName, int nDefault,int nFlags)
{
	IConsole* pConsole = g_GetConsole();
	ICVar* pVar;
	{
		char szDefault[32];
		sprintf (szDefault, "%d", nDefault);
		pVar = pConsole->CreateVariable(szVarName, szDefault, nFlags);
	}
	assert (pVar);
	return pVar;
}


//////////////////////////////////////////////////////////////////////////
// attaches the given variable to the given container;
// recreates the variable if necessary
ICVar* CryAnimVars::attach (const char* szVarName, int* pContainer, const char*szComment, int nFlags)
{
#ifndef WIN32
	// to economize memory ... :(
	szComment = "";
#endif

	IConsole* pConsole = g_GetConsole();
	

	ICVar* pOldVar = pConsole->GetCVar (szVarName);
	int nDefault;
	if (pOldVar)
	{
		nDefault = pOldVar->GetIVal();
		pConsole->UnregisterVariable(szVarName, true);
	}

	// NOTE: maybe we should preserve the actual value of the variable across the registration,
	// because of the strange architecture of IConsole that converts int->float->int

	pConsole->Register(szVarName, pContainer, float(*pContainer), nFlags, szComment);

	ICVar* pVar = pConsole->GetCVar(szVarName);

#ifdef _DEBUG
	// test if the variable really has this container
	assert (*pContainer == pVar->GetIVal());
	++*pContainer;
	assert (*pContainer == pVar->GetIVal());	
	--*pContainer;
#endif

	if (pOldVar && pVar && !(pVar->GetFlags()&VF_CHEAT))
	{
		// carry on the default value from the old variable anyway
		pVar->Set(nDefault);
	}

	return pVar;
}


//////////////////////////////////////////////////////////////////////////
// detaches the given variable from the container;
// recreates the variable with new value
void CryAnimVars::detach (const char* szVarName, int* pContainer)
{
	IConsole* pConsole = g_GetConsole();
	ICVar* pVar = pConsole->GetCVar (szVarName);
	if (pVar)
	{
#ifdef _DEBUG
		// test if the variable really has this container
		assert (*pContainer == pVar->GetIVal());
		++*pContainer;
		assert (*pContainer == pVar->GetIVal());
		--*pContainer;
#endif
		*pContainer = pVar->GetIVal();
		pConsole->UnregisterVariable(szVarName, true);
		char szDefault[32];
		sprintf (szDefault, "%d", *pContainer);
		pVar = pConsole->CreateVariable(szVarName, szDefault, 0);
	}
}

