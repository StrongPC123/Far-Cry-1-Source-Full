////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   aimanager.cpp
//  Version:     v1.00
//  Created:     11/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "AIManager.h"

#include "AIGoalLibrary.h"
#include "AiGoal.h"
#include "AiBehaviorLibrary.h"

#include "IAISystem.h"

#include <IScriptSystem.h>


//////////////////////////////////////////////////////////////////////////
// CAI Manager.
//////////////////////////////////////////////////////////////////////////
CAIManager::CAIManager()
{
	m_goalLibrary = new CAIGoalLibrary;
	m_behaviorLibrary = new CAIBehaviorLibrary;
}

CAIManager::~CAIManager()
{
	delete m_behaviorLibrary;
	m_behaviorLibrary = 0;
	delete m_goalLibrary;
	m_goalLibrary = 0;
}

void CAIManager::Init( ISystem *system )
{
	m_aiSystem = system->GetAISystem();
	if (!m_aiSystem)
		return;
	
	//m_goalLibrary->InitAtomicGoals();
	m_behaviorLibrary->LoadBehaviors( "Scripts\\AI\\Behaviors\\" );

	//enumerate Anchor actions.
	EnumAnchorActions();
}

IAISystem*	CAIManager::GetAISystem()
{
	return GetIEditor()->GetSystem()->GetAISystem();
}

//////////////////////////////////////////////////////////////////////////
void CAIManager::ReloadScripts()
{
	GetBehaviorLibrary()->ReloadScripts();
	EnumAnchorActions();
}

//////////////////////////////////////////////////////////////////////////
void CAIManager::GetAnchorActions( std::vector<CString> &actions ) const
{
	actions.clear();
	for (AnchorActions::const_iterator it = m_anchorActions.begin(); it != m_anchorActions.end(); it++)
	{
		actions.push_back( it->first );
	}
}

//////////////////////////////////////////////////////////////////////////
int CAIManager::AnchorActionToId( const char *sAction ) const
{
	AnchorActions::const_iterator it = m_anchorActions.find(sAction);
	if (it != m_anchorActions.end())
		return it->second;
	return -1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
struct CAIAnchorDump : public IScriptObjectDumpSink
{
	CAIManager::AnchorActions actions;

	CAIAnchorDump( IScriptObject *obj )
	{
		m_pScriptObject = obj;
	}

	virtual void OnElementFound(int nIdx,ScriptVarType type) {}
	virtual void OnElementFound(const char *sName,ScriptVarType type)
	{
		if (type == svtNumber)
		{
			// New behavior.
			int val;
			if (m_pScriptObject->GetValue(sName,val))
			{
				actions[sName] = val;
			}
		}
	}
private:
	IScriptObject *m_pScriptObject;
};

//////////////////////////////////////////////////////////////////////////
void CAIManager::EnumAnchorActions()
{
	IScriptSystem *pScriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
	
	_SmartScriptObject pAIAnchorTable( pScriptSystem,true );
	if (pScriptSystem->GetGlobalValue( "AIAnchor",pAIAnchorTable ))
	{
		CAIAnchorDump anchorDump(pAIAnchorTable);
		pAIAnchorTable->Dump( &anchorDump );
		m_anchorActions = anchorDump.actions;
	}
}