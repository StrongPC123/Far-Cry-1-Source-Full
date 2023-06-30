
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptTimerMgr.cpp: implementation of the CScriptTimerMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EntityDesc.h"
#include "ScriptTimerMgr.h"
#include <IEntitySystem.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScriptTimerMgr::CScriptTimerMgr(IScriptSystem *pScriptSystem,IEntitySystem *pS,IGame *pGame )
{
	m_nLastTimerID=0;
	m_pScriptSystem=pScriptSystem;
	m_pEntitySystem=pS;
	m_pGame=pGame;
	m_bPause=false;
}

CScriptTimerMgr::~CScriptTimerMgr()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
// Create a new timer and put it in the list of managed timers.
int CScriptTimerMgr::AddTimer(IScriptObject *pTable,int64 nStartTimer,int64 nTimer,IScriptObject *pUserData,bool bUpdateDuringPause)
{
	m_nLastTimerID++;
	m_mapTempTimers.insert(ScriptTimerMapItor::value_type(m_nLastTimerID,new ScriptTimer(pTable,nStartTimer,nTimer,pUserData,bUpdateDuringPause)));
	return m_nLastTimerID;
}

//////////////////////////////////////////////////////////////////////////
// Delete a timer from the list.
void CScriptTimerMgr::RemoveTimer(int nTimerID)
{
	ScriptTimerMapItor itor;
	// find timer
	itor=m_mapTimers.find(nTimerID);
	if(itor!=m_mapTimers.end())
	{
		// remove it
		ScriptTimer *pST=itor->second;
		delete pST;
		m_mapTimers.erase(itor);
	}
}

//////////////////////////////////////////////////////////////////////////
void	CScriptTimerMgr::Pause(bool bPause)
{ 
	m_bPause=bPause; 
	if (!m_pGame->GetModuleState(EGameMultiplayer))
		m_pEntitySystem->PauseTimers(bPause);
}

//////////////////////////////////////////////////////////////////////////
// Remove all timers.
void CScriptTimerMgr::Reset()
{
	ScriptTimerMapItor itor;
	itor=m_mapTimers.begin();
	while(itor!=m_mapTimers.end())
	{
		if(itor->second)
			delete itor->second;
		++itor;
	}
	m_mapTimers.clear();

	itor=m_mapTempTimers.begin();
	while(itor!=m_mapTempTimers.end())
	{
		if(itor->second)
			delete itor->second;
		++itor;
	}
	m_mapTempTimers.clear();


}

//////////////////////////////////////////////////////////////////////////
// Update all managed timers.
void CScriptTimerMgr::Update(int64 nCurrentTime)
{
	ScriptTimerMapItor itor;
	// loop through all timers.
	itor=m_mapTimers.begin();
	
	while(itor!=m_mapTimers.end())
	{
		ScriptTimer *pST=itor->second;

		if (m_bPause && !pST->bUpdateDuringPause)
		{
			++itor;
			continue;
		}

    // if it is time send a timer-event
		if(((nCurrentTime-pST->nStartTime)>=pST->nTimer))
		{
			int id=0;
			if(pST->pTable->GetValue("id",id))
			{
				IEntity *pEntity=m_pEntitySystem->GetEntity(id);
				if(pEntity)
				{
					pEntity->SendScriptEvent(ScriptEvent_Timer,pST->pUserData?pST->pUserData:0);
				}
			}
			else
			{
				HSCRIPTFUNCTION funcOnEvent;
				if(pST->pTable->GetValue("OnEvent",funcOnEvent))
				{
					m_pScriptSystem->BeginCall(funcOnEvent);
					m_pScriptSystem->PushFuncParam(pST->pTable);//self
					m_pScriptSystem->PushFuncParam((int)ScriptEvent_Timer);
					if(pST->pUserData)
						m_pScriptSystem->PushFuncParam(pST->pUserData);
					else
						m_pScriptSystem->PushFuncParam(false);
					m_pScriptSystem->EndCall();
				}
			}
			// after sending the event we can remove the timer.
			ScriptTimerMapItor tempItor=itor;
			++itor;
			m_mapTimers.erase(tempItor);
			delete pST;

		}
		else
		{
			++itor;
		}
		
	}
	// lets move all new created timers in the map. this is done at this point to avoid recursion, while trying to create a timer on a timer-event.
	if(!m_mapTempTimers.empty())
	{
		ScriptTimerMapItor itor;
		itor=m_mapTempTimers.begin();
		while(itor!=m_mapTempTimers.end())
		{
			m_mapTimers.insert(ScriptTimerMapItor::value_type(itor->first,itor->second));
			++itor;
		}
		m_mapTempTimers.clear();
	}
}