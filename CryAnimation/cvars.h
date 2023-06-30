/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	  Sep 25 2002 :- Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

#ifndef _3DENGINE_CVARS_H_
#define _3DENGINE_CVARS_H_

//////////////////////////////////////////////////////////////////////////
// console variable interfaces.
// In an instance of this class (singleton for now), there are all the interfaces
// of all the variables needed for the Animation subsystem.
// In case sometimes it is required to maintain several sets of console variables
// (e.g. in the unlikely case several consoles will be introduced), several
// instances of this class (for each console) are to be created
struct CryAnimVars
{
	enum {nDefaultAnimWarningLevel = 2};
	enum {
#if defined(WIN64) || defined(LINUX64)
		nDefaultAnimUnloadDelay = 0, // don't need to unload animation of Win64 architecture
		nDefaultAnimDeferredLoad = 0, // don't need to defer anim loading
#else
		nDefaultAnimUnloadDelay = 300,
		nDefaultAnimDeferredLoad = 1
#endif
	};
//#define DECLARE_STRING_VARIABLE(_var, _default) ICVar* _var
#define DECLARE_INT_VARIABLE_IMMEDIATE(_var, _default,_comment) ICVar* m_p_##_var; int m_##_var; int _var () const {return m_##_var;} \
	int _var##_Clamp (int nMin, int nMax)const \
	{																				   \
		if (m_##_var < nMin) { 	                 \
			m_p_##_var->Set (nMin);                \
			return nMin;                           \
		}                                        \
		if (m_##_var > nMax) {				           \
			m_p_##_var->Set (nMax);                \
			return nMax;										       \
		}																         \
		return m_##_var;								         \
	}
#define DECLARE_INT_VARIABLE_IMMEDIATE_DUMP(_var, _default,_comment) DECLARE_INT_VARIABLE_IMMEDIATE(_var, _default,_comment)
#define DECLARE_FLOAT_VARIABLE_IMMEDIATE(_var, _default) float m_##_var; float _var () const {return m_##_var;}
#define DECLARE_STRING_VARIABLE(_var, _default) ICVar* m_##_var; const char* _var () {return m_##_var ? m_##_var->GetString () : _default;}
#define DECLARE_INT_VARIABLE(_var, _default) ICVar* m_##_var; int _var() {return m_##_var->GetIVal();}
#define DECLARE_FLOAT_VARIABLE(_var, _default,_comment) ICVar* m_##_var; float _var() {return m_##_var->GetFVal();}
#define DECLARE_FLOAT_VARIABLE_DUMP(_var, _default,_comment) DECLARE_FLOAT_VARIABLE(_var, _default,_comment)
#include "CVars-list.h"
#undef DECLARE_STRING_VARIABLE
#undef DECLARE_INT_VARIABLE
#undef DECLARE_FLOAT_VARIABLE
#undef DECLARE_FLOAT_VARIABLE_DUMP
#undef DECLARE_INT_VARIABLE_IMMEDIATE
#undef DECLARE_INT_VARIABLE_IMMEDIATE_DUMP
#undef DECLARE_FLOAT_VARIABLE_IMMEDIATE

	float ca_MinVertexWeightLOD (unsigned nLOD);

	float ca_UpdateSpeed (unsigned nLayer);

	int ca_RainPower_Clamped();

	CryAnimVars ();

	~CryAnimVars ();

	// declares or retrieves the pointer to the console variable
	// with the given default value (used only in case the variable doesn't exist now)
	// returns the pointer to the preexisting or newly created variable.
	ICVar* declare (const char* szVarName, int nDefault, int nFlags=0 );

	// attaches the given variable to the given container;
	// recreates the variable if necessary
	ICVar* attach (const char* szVarName, int* pContainer, const char* szComment, int nFlags = 0);

	// detaches the given variable from the container;
	// recreates the variable with new value
	void detach (const char* szVarName, int* pContainer);
};
  
#endif // _3DENGINE_CVARS_H_
