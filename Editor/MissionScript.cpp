#include "StdAfx.h"
#include "missionscript.h"
#include <IScriptSystem.h>

#define EVENT_PREFIX "Event_"

struct CMissionScriptMethodsDump : public IScriptObjectDumpSink
{
	std::vector<CString> methods;
	std::vector<CString> events;
	void OnElementFound(int nIdx,ScriptVarType type){/*ignore non string indexed values*/};
	virtual void OnElementFound(const char *sName, ScriptVarType type)
	{
		if (type == svtFunction)
		{
			if (strncmp(sName,EVENT_PREFIX,6) == 0)
				events.push_back( sName+6 );
			else
				methods.push_back( sName );
		}
	}
};

CMissionScript::CMissionScript()
{
	m_sFilename="";
}

CMissionScript::~CMissionScript()
{
}

bool CMissionScript::Load()
{
	if (m_sFilename.IsEmpty())
		return true;

	// Parse .lua file.
	IScriptSystem *script = GetIEditor()->GetSystem()->GetIScriptSystem();
	if (!script->ExecuteFile(CString(GetIEditor()->GetMasterCDFolder())+m_sFilename, false, true))
	{
		AfxMessageBox(CString("Unable to execute script '")+CString(GetIEditor()->GetMasterCDFolder())+m_sFilename+"'. Check syntax ! Script not loaded.", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	_SmartScriptObject pMission(script, true);
	if (!script->GetGlobalValue("Mission",*pMission))
	{
		AfxMessageBox("Unable to find script-table 'Mission'. Check script ! Script not loaded.", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	CMissionScriptMethodsDump dump;
	pMission->Dump( &dump );
	m_methods = dump.methods;
	m_events = dump.events;

	// Sort methods and events alphabetically.
	std::sort( m_methods.begin(),m_methods.end() );
	std::sort( m_events.begin(),m_events.end() );
	return true;
}

void CMissionScript::Edit()
{
	if (m_sFilename.IsEmpty())
		return;

	CFileUtil::EditTextFile( m_sFilename );
}

//////////////////////////////////////////////////////////////////////////
void CMissionScript::OnReset()
{
	IScriptSystem *pScriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();

	_SmartScriptObject pMission( pScriptSystem, true);
	if (!pScriptSystem->GetGlobalValue("Mission",pMission))
	{
		return;
	}
	
	HSCRIPTFUNCTION pf;
	if (pMission->GetValue( "OnReset",pf ))
	{
		pScriptSystem->BeginCall(pf);
		pScriptSystem->EndCall();
		//Alberto
		pScriptSystem->ReleaseFunc(pf);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMissionScript::SetScriptFile( const CString &file )
{
	m_sFilename = file;
}