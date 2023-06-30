////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   scriptsink.cpp
//  Version:     v1.00
//  Created:     30/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ScriptSink.h"

#include "System.h"
#include <IRenderer.h>
#include <I3DEngine.h>
#include <ISound.h>
#include <ILog.h>
#include <IConsole.h>
#include <ITimer.h>
#include <IGame.h>
#include <IAISystem.h>					// AISystem

#include "XConsole.h"

//////////////////////////////////////////////////////////////////////////
CScriptSink::CScriptSink( CSystem *pSystem,CXConsole *pConsole)
{
	assert( pSystem );
	m_pSystem = pSystem;
	m_pConsole = pConsole;
	m_lastGCTime = 0;
	m_fGCFreq = 10;
}

//////////////////////////////////////////////////////////////////////////
void CScriptSink::Init()
{
	IScriptSystem *pScriptSystem = m_pSystem->GetIScriptSystem();
	if (pScriptSystem)
	{
		// Set global time variable into the script.
		pScriptSystem->SetGlobalValue( "_time", 0 );
		pScriptSystem->SetGlobalValue( "_frametime",0 );
		pScriptSystem->SetGlobalValue( "_aitick",0 );

		m_nLastGCCount=pScriptSystem->GetCGCount();
	}
	else
		m_nLastGCCount=0;
}

//////////////////////////////////////////////////////////////////////////
void CScriptSink::OnLoadSource(const char *sSourceName,unsigned char *sSource,long nSourceSize)
{
}

//////////////////////////////////////////////////////////////////////
void CScriptSink::OnScriptError(const char *sSourceFile,const char *sFuncName,int nLineNum,const char *sErrorDesc)
{
	string sTemp;
	int n=0;

	if(nLineNum!=-1)			// line no info?
	{
		m_pSystem->Warning( VALIDATOR_MODULE_SCRIPTSYSTEM,VALIDATOR_ERROR,VALIDATOR_FLAG_SCRIPT,sSourceFile,
			"$3#SCRIPT ERROR File: %s, Line [%03d], Function: %s,\n%s",sSourceFile,nLineNum,sFuncName,sErrorDesc );
	}
	else
	{
		m_pSystem->Warning( VALIDATOR_MODULE_SCRIPTSYSTEM,VALIDATOR_ERROR,VALIDATOR_FLAG_SCRIPT,sSourceFile,
			"$3#SCRIPT ERROR File: %s, Function: %s,\n%s",sSourceFile,sFuncName,sErrorDesc );
	}


	m_pSystem->GetILog()->Log("\001$6#Function %s ",sFuncName);
	while(sErrorDesc[n]!=0)
	{
		if(sErrorDesc[n]=='\n')
		{
			m_pSystem->GetILog()->Log("\001$6# %s",sTemp.c_str());
			sTemp="";
			while(sErrorDesc[n]=='\n')n++;
		}else{
			sTemp+=sErrorDesc[n];
			n++;
		}
	
	}
	if(!sTemp.empty())
		m_pSystem->GetILog()->LogError("\001$6# %s ",sTemp.c_str());

	if (m_pSystem->GetLuaDebugger())
	{
		if (sSourceFile != NULL &&
 			_stricmp(sSourceFile, "__script_buffer__") != 0 && 
 			_stricmp(sSourceFile, "=C") != 0 &&
 			_stricmp(sSourceFile, "undefined") != 0 &&
			nLineNum!=-1 &&
 			sSourceFile[0] != '\0')
 		{
			// char szMessage[2048];
			// sprintf(szMessage, "Lua runtime error found in '%s' line %03d function '%s' - open debugger ?", 
			// 	sSourceFile, nLineNum, sFuncName);
			// if (MessageBox(NULL, szMessage, "Lua Runtime Error", MB_YESNO | MB_ICONERROR) == IDYES)

			m_pSystem->ShowDebugger(sSourceFile, nLineNum, sErrorDesc);
 		}
	}
}

void CScriptSink::OnLoadedScriptDump(const char *sScriptPath)
{
	m_pSystem->GetILog()->Log(sScriptPath);
}

//////////////////////////////////////////////////////////////////////
bool CScriptSink::CanSetGlobal(const char *sVarName)
{
	if (!m_pConsole)
	{
		m_pSystem->GetILog()->LogWarning("\001Attempt to change variable %s denied: no console system found", sVarName);
		return false; // we can't change globals unless we have console
	}

	IGame*pGame = m_pSystem->GetIGame();
	if (!pGame)
		return true; // we may change whatever we want until we're not in the game

	ICVar* pVar = m_pConsole->GetCVar(sVarName);
	if (!pVar)
	{
		m_pSystem->GetILog()->Log("\001Attempt to change variable %s denied: no such variable found", sVarName);
		return false; // we aren't allowed to change global that doesn't exist
	}

	int nFlags = pVar->GetFlags();

	if((nFlags & VF_REQUIRE_NET_SYNC) && !pGame->GetModuleState(EGameServer) && pGame->GetModuleState(EGameClient))
	{
		m_pSystem->GetILog()->Log("\001Attempt to change variable %s on client denied:", sVarName);
		m_pSystem->GetILog()->Log("\001The variable is Server-synchronized and the game is not server.");
		return false;
	}
		
	if ((nFlags & VF_CHEAT) && (!m_pSystem->IsDevMode()))
	{		
		// No Cheat Log. m_pSystem->GetILog()->Log("\001Variable %s is cheat protected.", sVarName);
		return false;
	}

	if (nFlags & VF_READONLY)
	{
		m_pSystem->GetILog()->Log("\001Variable %s is read only.", sVarName);
		return false;
	}

	return true; 
}
//////////////////////////////////////////////////////////////////////
void CScriptSink::OnSetGlobal(const char *sVarName)
{
	if (m_pConsole)
		m_pConsole->RefreshVariable(sVarName);
}
//////////////////////////////////////////////////////////////////////
void CScriptSink::OnCollectUserData(INT_PTR nValue,int nCookie)
{
	//m_Log.Log("OnCollectUserData %d", nValue);
	
	switch(nCookie)
	{
	case USER_DATA_SOUND:
		{
			ISound *m_pISound;
			m_pISound=(ISound *)nValue;
			//m_pSystem->GetILog()->Log("collecting %s",m_pISound->GetName());
			//::OutputDebugString(m_pISound->GetName());
			//::OutputDebugString(" OnCollect USER_DATA_SOUND\n");
			if(m_pISound)
				m_pISound->Release();
			
		}
		break;
	case USER_DATA_TEXTURE:
		{
			//::OutputDebugString("OnCollect USER_DATA_TEXTURE\n");
			INT_PTR nTid=nValue;
			IRenderer *pRenderer = m_pSystem->GetIRenderer();
			if (pRenderer)
				pRenderer->RemoveTexture(nTid);
		}
		break;
	case USER_DATA_OBJECT:
		{
			//::OutputDebugString("OnCollect USER_DATA_OBJECT\n");
			I3DEngine *p3DEngine = m_pSystem->GetI3DEngine();
			IStatObj *pObj=(IStatObj *)nValue;

			// if you have crash here - pObj is invalid pointer 
			// ( maybe attempt to use IStatObj pointer after I3DEngine::ClearRenderResources() )
			// char chTest = pObj->GetFileName()[0];

			if (p3DEngine)
				p3DEngine->ReleaseObject(pObj);
		}
		break;
	case USER_DATA_LIGHT:
		{
			//::OutputDebugString("OnCollect USER_DATA_LIGHT\n");
			INT_PTR nLightId=nValue;
			I3DEngine *p3DEngine = m_pSystem->GetI3DEngine();			
			if (p3DEngine)
				p3DEngine->DeleteStaticLightSource(nLightId);
		}
		break;
	default:
		{
			//m_Log.Log("OnCollectUserData WARINING unknown user data type %d [cookie=%d]", nValue,nCookie);
			//if(m_pSystem->m_pGame)
			//	m_pSystem->m_pGame->OnCollectUserData(nValue,nCookie);
		}
	}
	
}

/////////////////////////////////////////////////////////////
//LUA DEBUGGER
/////////////////////////////////////////////////////////////
class ___Temp : public IScriptObjectDumpSink
{
	void OnElementFound(int nIdx,ScriptVarType type){}
	void OnElementFound(const char *sName,ScriptVarType type)
	{
		switch(type)
		{
		case svtNull:
			//GetILog()->LogToConsole("<<NULL>> %s",sName);
			break;
		case svtString:
			//GetILog()->LogToConsole("<<STRING>> %s",sName);
				break;
		case svtNumber:
			//GetILog()->LogToConsole("<<NUMBER>> %s",sName);
				break;
		case svtFunction:
			//GetILog()->LogToConsole("<<FUNCTION>> %s",sName);
				break;
		case svtObject:
			//GetILog()->LogToConsole("<<OBJECT>> %s",sName);
				break;
		case svtUserData:
			//GetILog()->LogToConsole("<<USERDATA>> %s",sName);
			break;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
void CScriptSink::OnExecuteLine(ScriptDebugInfo &sdiDebugInfo)
{
	/*
	___Temp temp;
	IScriptObject *pLocals = m_pSystem->GetIScriptSystem()->GetLocalVariables();
	m_pSystem->GetILog()->LogToConsole("START DUMP");
	pLocals->Dump(&temp);
	pLocals->Release();
	*/

	if (m_pSystem->GetLuaDebugger())
	{
		m_pSystem->ShowDebugger(sdiDebugInfo.sSourceName, 
			sdiDebugInfo.nCurrentLine, "Breakpoint Hit");
	}
}

//////////////////////////////////////////////////////////////////////////
void CScriptSink::Update( bool bNoLuaGC )
{
	ITimer *pTimer=m_pSystem->GetITimer();

	float currTime=pTimer->GetCurrTime();
	float frameTime=pTimer->GetFrameTime();

	IScriptSystem *pScriptSystem = m_pSystem->GetIScriptSystem();

	// Set global time variable into the script.
	pScriptSystem->SetGlobalValue( "_time", currTime );
	pScriptSystem->SetGlobalValue( "_frametime",frameTime );

	{
		int aiTicks = 0;
	
		IAISystem *pAISystem=m_pSystem->GetAISystem();

		if(pAISystem)
			aiTicks = pAISystem->GetAITickCount();
		pScriptSystem->SetGlobalValue( "_aitick",aiTicks );
	}

	// garbage-collect script-variables
	pTimer->MeasureTime("PreLuaGC");

	//TRACE("GC DELTA %d",m_pScriptSystem->GetCGCount()-nStartGC);
	//int nStartGC = pScriptSystem->GetCGCount();

	bool bKickIn=false;															// Invoke Gargabe Collector

	if(currTime-m_lastGCTime>m_fGCFreq)	 // g_GC_Frequence->GetIVal())
		bKickIn=true;

	int nGCCount=pScriptSystem->GetCGCount();

	if(nGCCount-m_nLastGCCount>2000 && !bNoLuaGC)		//
		bKickIn=true;

	if(bKickIn)
	{
		FRAME_PROFILER( "Lua GC",m_pSystem,PROFILE_SCRIPT );
		
		//float fTimeBefore=pTimer->GetAsyncCurTime()*1000;
		pScriptSystem->ForceGarbageCollection();
		m_nLastGCCount=pScriptSystem->GetCGCount();
		m_lastGCTime = currTime;
		//float fTimeAfter=pTimer->GetAsyncCurTime()*1000;
		//CryLog("--[after coll]GC DELTA %d ",pScriptSystem->GetCGCount()-nGCCount);
		//TRACE("--[after coll]GC DELTA %d [time =%f]",m_pScriptSystem->GetCGCount()-nStartGC,fTimeAfter-fTimeBefore);
	}
	
  pTimer->MeasureTime("LuaGC");
}