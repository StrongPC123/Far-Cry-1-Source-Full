////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   scriptsink.h
//  Version:     v1.00
//  Created:     30/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __scriptsink_h__
#define __scriptsink_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include <IScriptSystem.h>
class CSystem;
class CXConsole;
/** Implements callback of script system.
*/
class CScriptSink :
	public IScriptSystemSink,
	public IScriptDebugSink
{
public:
	CScriptSink( CSystem *pSystem,CXConsole *pConsole );

	void Init();

	//! Called every frame to handle ScriptSystem needs..
	void Update( bool bNoLuaGC=false );

	void OnLoadSource(const char *sSourceName,unsigned char *sSource,long nSourceSize);
	void OnExecuteLine(ScriptDebugInfo &sdiDebugInfo);
///////////////////////////////////////////////////////////////////////////
	//! @name IScriptSytemSink implementation
	//@{ 
	void OnScriptError(const char *sSourceFile,const char *sFuncName,int nLineNum,const char *sErrorDesc);
	void OnSetGlobal(const char *sVarName);
	bool CanSetGlobal(const char* sVarName);
	void OnLoadedScriptDump(const char *sScriptPath);
	void OnCollectUserData(INT_PTR nValue,int nCookie);
	//@}

	//! \param freq time in seconds
	void SetGCFreq( const float fFreq ) { m_fGCFreq = fFreq; };

private: // ----------------------------------------------------------------

	float						m_fGCFreq;				//!< relative time in seconds
	float						m_lastGCTime;			//!< absolute time in seconds
	int							m_nLastGCCount;		//!<
	//there's no need to use interface inside the same module
	CSystem *				m_pSystem;				//!<
	CXConsole *			m_pConsole;				//!<
};


#endif // __scriptsink_h__
