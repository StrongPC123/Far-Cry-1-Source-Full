// ScriptObjectScript.cpp: implementation of the CScriptObjectScript class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectScript.h"
#include <ILog.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectScript)

CScriptObjectScript::CScriptObjectScript()
{

}

CScriptObjectScript::~CScriptObjectScript()
{

}

void CScriptObjectScript::Init(IScriptSystem *pScriptSystem)
{
	InitGlobal(pScriptSystem,"Script",this);
}




void CScriptObjectScript::Debug_Full_recursive( IScriptObject *pCurrent, string &sPath, std::set<const void *> &setVisited )
{
	assert(pCurrent);

	pCurrent->BeginIteration();

	while(pCurrent->MoveNext())
	{
		char *szKeyName;

		if(!pCurrent->GetCurrentKey(szKeyName))
			szKeyName="NO";

		ScriptVarType type=pCurrent->GetCurrentType();

		if(type==svtObject)		// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			const void *pVis;
			
			pCurrent->GetCurrentPtr(pVis);

			GetISystem()->GetILog()->Log("  table '%s/%s'",sPath.c_str(),szKeyName);				

			if(setVisited.count(pVis)!=0)
			{
				GetISystem()->GetILog()->Log("    .. already processed ..");				
				continue;
			}

			setVisited.insert(pVis);

			{
				IScriptObject *pNewObject = m_pScriptSystem->CreateEmptyObject();

				pCurrent->GetCurrent(pNewObject);

#if defined(LINUX)
				string s = sPath+string("/")+szKeyName;
				Debug_Full_recursive(pNewObject,s,setVisited);
#else
				Debug_Full_recursive(pNewObject,sPath+string("/")+szKeyName,setVisited);
#endif
				pNewObject->Release();
			}
		}
		else if(type==svtFunction)		// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			unsigned int *pCode=0;
			int iSize=0;
			
			if(pCurrent->GetCurrentFuncData(pCode,iSize))
			{
				// lua function
				if(pCode) 
					GetISystem()->GetILog()->Log("         lua function '%s' size=%d",szKeyName,(DWORD)iSize);				
				 else
					GetISystem()->GetILog()->Log("         cpp function '%s'",szKeyName);				
			}
		}
		else													// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			GetISystem()->GetILog()->Log("         data '%s'",szKeyName);				
		}
	}
	
	pCurrent->EndIteration();
}


DWORD CScriptObjectScript::Debug_Buckets_recursive( IScriptObject *pCurrent, string &sPath, std::set<const void *> &setVisited, 
	const DWORD dwMinBucket )
{
	assert(pCurrent);

	DWORD dwTableElementCount=0;

	pCurrent->BeginIteration();

	while(pCurrent->MoveNext())
	{
		char *szKeyName;

		dwTableElementCount++;

		if(!pCurrent->GetCurrentKey(szKeyName))
			szKeyName="NO";

		ScriptVarType type=pCurrent->GetCurrentType();

		if(type==svtObject)		// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			const void *pVis;
			
			pCurrent->GetCurrentPtr(pVis);

			if(setVisited.count(pVis)!=0)
				continue;

			setVisited.insert(pVis);

			{
				IScriptObject *pNewObject = m_pScriptSystem->CreateEmptyObject();

				pCurrent->GetCurrent(pNewObject);

#if defined(LINUX)
				string s = sPath+string("/")+szKeyName;
				DWORD dwSubTableCount = Debug_Buckets_recursive(pNewObject,s,setVisited,dwMinBucket);
#else
				DWORD dwSubTableCount = Debug_Buckets_recursive(pNewObject,sPath+string("/")+szKeyName,setVisited,dwMinBucket);
#endif
				pNewObject->Release();

				if(dwSubTableCount>=dwMinBucket)
				{
					GetISystem()->GetILog()->Log("  %8d '%s/%s'\n",dwSubTableCount,sPath.c_str(),szKeyName);				
				}
				else dwTableElementCount+=dwSubTableCount;
			}
		}
/*		else if(type==svtFunction)		// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			unsigned int *pCode=0;
			int iSize=0;
			
			if(pCurrent->GetCurrentFuncData(pCode,iSize))
			if(pCode)
			{
				// lua function

				char str[256];

				sprintf(str,"%s lua  '%s' size=%d\n",sPath.c_str(),szKeyName,(DWORD)iSize);

				OutputDebugString(str);
			}
		}
*/
	}
	
	pCurrent->EndIteration();

	return dwTableElementCount;
}


void CScriptObjectScript::Debug_Elements( IScriptObject *pCurrent, string &sPath, std::set<const void *> &setVisited )
{
	assert(pCurrent);

	pCurrent->BeginIteration();

	while(pCurrent->MoveNext())
	{
		char *szKeyName;

		if(!pCurrent->GetCurrentKey(szKeyName))
			szKeyName="NO";

		ScriptVarType type=pCurrent->GetCurrentType();

		if(type==svtObject)		// svtNull,svtFunction,svtString,svtNumber,svtUserData,svtObject
		{
			const void *pVis;
			
			pCurrent->GetCurrentPtr(pVis);

			if(setVisited.count(pVis)!=0)
				continue;

			setVisited.insert(pVis);

			{
				IScriptObject *pNewObject = m_pScriptSystem->CreateEmptyObject();

				pCurrent->GetCurrent(pNewObject);

#if defined(LINUX)
				string s = sPath+string("/")+szKeyName;
				DWORD dwSubTableCount = Debug_Buckets_recursive(pNewObject,s,setVisited,0xffffffff);
#else
				DWORD dwSubTableCount = Debug_Buckets_recursive(pNewObject,sPath+string("/")+szKeyName,setVisited,0xffffffff);
#endif

				pNewObject->Release();

				GetISystem()->GetILog()->Log("  %8d '%s' '%s'\n",dwSubTableCount,sPath.c_str(),szKeyName);				
			}
		}
	}
	
	pCurrent->EndIteration();
}



int CScriptObjectScript::Debug(IFunctionHandler *pH)
{
/*  deactivate because it can be used to hack

	CHECK_PARAMETERS(0);

	IScriptObject *pGlobals=m_pScriptSystem->GetGlobalObject();


	{
		std::set<const void *> setVisited;
	
		GetISystem()->GetILog()->Log("CScriptObjectScript::Debug globals recursive minbuckets");

		DWORD dwCount=Debug_Buckets_recursive(pGlobals,string(""),setVisited,50);

		GetISystem()->GetILog()->Log("  %8d '' ''\n",dwCount);			
	}

	{
		std::set<const void *> setVisited;

		GetISystem()->GetILog()->Log("CScriptObjectScript::Debug globals");
		Debug_Elements(pGlobals,string(""),setVisited);
	}

	pGlobals->Release();
*/
	return pH->EndFunction();
}


int CScriptObjectScript::DebugFull(IFunctionHandler *pH)
{
/* deactivate because it can be used to hack
	CHECK_PARAMETERS(0);

	IScriptObject *pGlobals=m_pScriptSystem->GetGlobalObject();

	{
		std::set<const void *> setVisited;
	
		GetISystem()->GetILog()->Log("CScriptObjectScript::Debug full");

		Debug_Full_recursive(pGlobals,string(""),setVisited);
	}

	pGlobals->Release();
*/

	return pH->EndFunction();
}

void CScriptObjectScript::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectScript>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectScript,ReloadScripts);
	REG_FUNC(CScriptObjectScript,ReloadScript);
	REG_FUNC(CScriptObjectScript,LoadScript);
	REG_FUNC(CScriptObjectScript,UnloadScript);
	REG_FUNC(CScriptObjectScript,DumpLoadedScripts);
	REG_FUNC(CScriptObjectScript,Debug);
	REG_FUNC(CScriptObjectScript,DebugFull);
}

/*!reload all previosly loaded scripts
*/
int CScriptObjectScript::ReloadScripts(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pScriptSystem->ReloadScripts();
	return pH->EndFunction();
}

/*!reload a specified script. If the script wasn't loaded at least once before the function will fail
	@param sFileName path of the script that has to be reloaded
*/
int CScriptObjectScript::ReloadScript(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sFileName;
	pH->GetParam(1,sFileName);
		
	if(!sFileName)
		return pH->EndFunction();			// ReloadScript filename is nil

	m_pScriptSystem->ExecuteFile(sFileName,true,true);
	//m_pScriptSystem->ReloadScript(sFileName);
	return pH->EndFunction();
}

/*!load a specified script
	@param sFileName path of the script that has to be loaded
*/
int CScriptObjectScript::LoadScript(IFunctionHandler *pH)
{
	bool bReload = false;
	bool bRaiseError = true;

	if (pH->GetParamCount() >= 3)
	{
		pH->GetParam(3, bRaiseError);
	}
	if (pH->GetParamCount() >= 2)
	{
		if (pH->GetParamType(2) == svtNumber)
		{
			int iReload;

			if (pH->GetParam(2, iReload))
			{
				bReload = (iReload != 0);
			}
		}
	}
	
	const char *sScriptFile;
	pH->GetParam(1,sScriptFile);

	if (m_pScriptSystem->ExecuteFile(sScriptFile, bRaiseError, bReload))
		return pH->EndFunction(1);
	else
		return pH->EndFunctionNull();
}

/*!unload script from the "loaded scripts map" so if this script is loaded again
	the Script system will reloadit. this function doesn't
	involve the LUA VM so the resources allocated by the script will not be released
	unloading the script
	@param sFileName path of the script that has to be loaded
*/
int CScriptObjectScript::UnloadScript(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sScriptFile;
	pH->GetParam(1,sScriptFile);
	m_pScriptSystem->UnloadScript(sScriptFile);
	return pH->EndFunction();
}

/*!Dump all loaded scripts path calling IScriptSystemSink::OnLoadedScriptDump
	@see IScriptSystemSink
*/
int CScriptObjectScript::DumpLoadedScripts(IFunctionHandler *pH)
{
	m_pScriptSystem->DumpLoadedScripts();
	return pH->EndFunction();
}