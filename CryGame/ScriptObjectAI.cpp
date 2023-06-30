
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectAI.cpp
//
//  Description: 
//		ScriptObjectAI.cpp: implementation of the CScriptObjectAI class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectAI.h"
#include "ScriptObjectVector.h"
#include <IAISystem.h>
#include <IAgent.h>
#include <ISound.h>
#include <physinterface.h>


#include "XObjectProxy.h"
#include "XPuppetProxy.h"
#include "XVehicleProxy.h"
#include "XPlayer.h"
#include "XVehicle.h"
#include "ScriptObjectAI.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction 
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectAI)

//////////////////////////////////////////////////////////////////////
CScriptObjectAI::CScriptObjectAI()
{

}

//////////////////////////////////////////////////////////////////////
CScriptObjectAI::~CScriptObjectAI()
{

}

//////////////////////////////////////////////////////////////////////
void CScriptObjectAI::Init(IScriptSystem *pScriptSystem, ISystem *pSystem, CXGame *pGame)
{
	m_pScriptSystem = pScriptSystem;
	m_pEntitySystem = pSystem->GetIEntitySystem();
	m_pAISystem = pSystem->GetAISystem();
	m_pSoundSystem = pSystem->GetISoundSystem();
	m_pLog = pSystem->GetILog();
	m_pGame = pGame;
	InitGlobal(pScriptSystem,"AI",this);

	//! Set some defines
	m_pScriptSystem->SetGlobalValue("AIPARAM_SIGHTRANGE",1);
	m_pScriptSystem->SetGlobalValue("AIPARAM_ATTACKRANGE",2);
	m_pScriptSystem->SetGlobalValue("AIPARAM_ACCURACY",3);
	m_pScriptSystem->SetGlobalValue("AIPARAM_AGGRESION",4);
	m_pScriptSystem->SetGlobalValue("AIPARAM_GROUPID",5);
	m_pScriptSystem->SetGlobalValue("AIPARAM_SOUNDRANGE",6);
	m_pScriptSystem->SetGlobalValue("AIPARAM_FOV",7);
	m_pScriptSystem->SetGlobalValue("AIPARAM_COMMRANGE",8);
	m_pScriptSystem->SetGlobalValue("AIPARAM_FWDSPEED",	9);
	m_pScriptSystem->SetGlobalValue("AIPARAM_RESPONSIVENESS",	10);
	m_pScriptSystem->SetGlobalValue("AIPARAM_SPECIES",	11);
	

	pScriptSystem->SetGlobalValue("AIOBJECT_PUPPET",				AIOBJECT_PUPPET);
//	pScriptSystem->SetGlobalValue("AIOBJECT_VEHICLE",			AIOBJECT_VEHICLE);
	pScriptSystem->SetGlobalValue("AIOBJECT_CAR",						AIOBJECT_CAR);
	pScriptSystem->SetGlobalValue("AIOBJECT_BOAT",					AIOBJECT_BOAT);
	pScriptSystem->SetGlobalValue("AIOBJECT_HELICOPTER",		AIOBJECT_HELICOPTER);
	pScriptSystem->SetGlobalValue("AIOBJECT_ATTRIBUTE",					AIOBJECT_ATTRIBUTE);
	pScriptSystem->SetGlobalValue("AIOBJECT_WAYPOINT",			AIOBJECT_WAYPOINT);
	pScriptSystem->SetGlobalValue("AIOBJECT_SNDSUPRESSOR",	AIOBJECT_SNDSUPRESSOR);
	pScriptSystem->SetGlobalValue("AIOBJECT_MOUNTEDWEAPON",	AIOBJECT_MOUNTEDWEAPON);
	pScriptSystem->SetGlobalValue("AIOBJECT_PLAYER",				AIOBJECT_PLAYER);
	pScriptSystem->SetGlobalValue("AIOBJECT_DUMMY",				AIOBJECT_DUMMY);
	pScriptSystem->SetGlobalValue("AIOBJECT_NONE",				AIOBJECT_NONE);

	pScriptSystem->SetGlobalValue("AIEVENT_ONBODYSENSOR",			AIEVENT_ONBODYSENSOR);
	pScriptSystem->SetGlobalValue("AIEVENT_ONVISUALSTIMULUS",	AIEVENT_ONVISUALSTIMULUS);
	pScriptSystem->SetGlobalValue("AIEVENT_AGENTDIED",				AIEVENT_AGENTDIED);
	pScriptSystem->SetGlobalValue("AIEVENT_SLEEP",					AIEVENT_SLEEP);
	pScriptSystem->SetGlobalValue("AIEVENT_WAKEUP",					AIEVENT_WAKEUP);
	pScriptSystem->SetGlobalValue("AIEVENT_ENABLE",					AIEVENT_ENABLE);
	pScriptSystem->SetGlobalValue("AIEVENT_DISABLE",				AIEVENT_DISABLE);
	pScriptSystem->SetGlobalValue("AIEVENT_REJECT",					AIEVENT_REJECT);
	pScriptSystem->SetGlobalValue("AIEVENT_PATHFINDON",				AIEVENT_PATHFINDON);
	pScriptSystem->SetGlobalValue("AIEVENT_PATHFINDOFF",			AIEVENT_PATHFINDOFF);
	pScriptSystem->SetGlobalValue("AIEVENT_CLEAR",		AIEVENT_CLEAR);
	pScriptSystem->SetGlobalValue("AIEVENT_DROPBEACON",		AIEVENT_DROPBEACON);
	

	pScriptSystem->SetGlobalValue("AIREADIBILITY_NORMAL",AIREADIBILITY_NORMAL);
	pScriptSystem->SetGlobalValue("AIREADIBILITY_NOPRORITY",AIREADIBILITY_NOPRIORITY);
	pScriptSystem->SetGlobalValue("AIREADIBILITY_SEEN",AIREADIBILITY_SEEN);
	pScriptSystem->SetGlobalValue("AIREADIBILITY_LOST",AIREADIBILITY_LOST);
	pScriptSystem->SetGlobalValue("AIREADIBILITY_INTERESTING",AIREADIBILITY_INTERESTING);

	pScriptSystem->SetGlobalValue("SIGNALID_THROWGRENADE",-10);
	pScriptSystem->SetGlobalValue("SIGNALID_READIBILITY",100);

	pScriptSystem->SetGlobalValue("SIGNALFILTER_LASTOP",			1);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_GROUPONLY",		2);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_SPECIESONLY",	3);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_ANYONEINCOMM",4);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_TARGET",			5);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_SUPERGROUP",	6);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_SUPERSPECIES",7);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_SUPERTARGET",	8);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_NEARESTGROUP",9);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_NEARESTINCOMM",SIGNALFILTER_NEARESTINCOMM);
	pScriptSystem->SetGlobalValue("SIGNALFILTER_HALFOFGROUP",12);

	pScriptSystem->SetGlobalValue("HM_NEAREST",0);
	pScriptSystem->SetGlobalValue("HM_FARTHEST_FROM_TARGET",1);
	pScriptSystem->SetGlobalValue("HM_NEAREST_TO_TARGET",2);
	pScriptSystem->SetGlobalValue("HM_FARTHEST_FROM_GROUP",3);
	pScriptSystem->SetGlobalValue("HM_NEAREST_TO_GROUP",4);
	pScriptSystem->SetGlobalValue("HM_LEFTMOST_FROM_TARGET",5);
	pScriptSystem->SetGlobalValue("HM_RIGHTMOST_FROM_TARGET",6);
	pScriptSystem->SetGlobalValue("HM_RANDOM",7);
	pScriptSystem->SetGlobalValue("HM_FRONTLEFTMOST_FROM_TARGET",8);
	pScriptSystem->SetGlobalValue("HM_FRONTRIGHTMOST_FROM_TARGET",9);
	pScriptSystem->SetGlobalValue("HM_NEAREST_TO_FORMATION",10);
	pScriptSystem->SetGlobalValue("HM_FARTHEST_FROM_FORMATION",11);
	pScriptSystem->SetGlobalValue("HM_NEAREST_TO_LASTOPRESULT",20);
	pScriptSystem->SetGlobalValue("HM_FARTHEST_FROM_LASTOPRESULT",21);
	pScriptSystem->SetGlobalValue("HM_NEAREST_TO_LASTOPRESULT_NOSAME",22);
	
	pScriptSystem->SetGlobalValue("BODYPOS_STAND",BODYPOS_STAND);
	pScriptSystem->SetGlobalValue("BODYPOS_CROUCH",BODYPOS_CROUCH);
	pScriptSystem->SetGlobalValue("BODYPOS_PRONE",BODYPOS_PRONE);
	pScriptSystem->SetGlobalValue("BODYPOS_RELAX",BODYPOS_RELAX);
	pScriptSystem->SetGlobalValue("BODYPOS_STEALTH",BODYPOS_STEALTH);
}

//////////////////////////////////////////////////////////////////////
void CScriptObjectAI::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectAI>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectAI,CreateGoalPipe);
	REG_FUNC(CScriptObjectAI,PushGoal);
	REG_FUNC(CScriptObjectAI,Signal);
	REG_FUNC(CScriptObjectAI,FreeSignal);
	REG_FUNC(CScriptObjectAI,SoundEvent);
	REG_FUNC(CScriptObjectAI,GetGroupCount);
	REG_FUNC(CScriptObjectAI,Log);
	REG_FUNC(CScriptObjectAI,MakePuppetIgnorant);
	REG_FUNC(CScriptObjectAI,GetAttentionTargetOf);
	REG_FUNC(CScriptObjectAI,SetAssesmentMultiplier);
	REG_FUNC(CScriptObjectAI,FindObjectOfType);
	REG_FUNC(CScriptObjectAI,GetGroupOf);
	REG_FUNC(CScriptObjectAI,GetAnchor);
	REG_FUNC(CScriptObjectAI,GetPerception);
	REG_FUNC(CScriptObjectAI,RegisterWithAI);
	REG_FUNC(CScriptObjectAI,AIBind);
	REG_FUNC(CScriptObjectAI,CreateBoundObject);
	REG_FUNC(CScriptObjectAI,Cloak);
	REG_FUNC(CScriptObjectAI,DeCloak);
	REG_FUNC(CScriptObjectAI,ProjectileShoot);
	REG_FUNC(CScriptObjectAI,SetTheSkip);
	REG_FUNC(CScriptObjectAI,SetAllowedDeathCount);
	REG_FUNC(CScriptObjectAI,Checkpoint);
	REG_FUNC(CScriptObjectAI,RegisterPlayerHit);
	REG_FUNC(CScriptObjectAI,FireOverride);
	REG_FUNC(CScriptObjectAI,SetSpeciesThreatMultiplier);
	REG_FUNC(CScriptObjectAI,EnablePuppetMovement);
	REG_FUNC(CScriptObjectAI,IsMoving);
	REG_FUNC(CScriptObjectAI,EnableNodesInSphere);
	REG_FUNC(CScriptObjectAI,GetStats);
	
}

/*!Create a sequence of AI atomic commands (a goal pipe)
	@param desired name of the goal pipe
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::CreateGoalPipe(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *name;

	pH->GetParam(1,name);

	IGoalPipe *pPipe = m_pAISystem->CreateGoalPipe(name);

//	m_mapGoals.insert(goalmap::iterator::value_type(name,pPipe));

	return pH->EndFunctionNull();

}

/*!Push a goal into a goal pipe. The goal is appended at the end of the goal pipe. Pipe of given name has to be previously created.
	@param name of the goal pipe in which the goal will be pushed.
	@param name of atomic goal that needs to be pushed into the pipe
	@param 1 if the goal should block the pipe execution, 0 if the goal should not block the goal execution
	@see CScriptObjectAI::CreateGoalPipe
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::PushGoal(IFunctionHandler *pH)
{

	const char *pipename;
	const char *temp;
	string goalname;
//	int id;
	GoalParameters params;
	bool blocking;

	pH->GetParam(1,pipename);
	pH->GetParam(2,temp);
	pH->GetParam(3,blocking);

	goalname = temp;

	IGoalPipe *pPipe=0;
	
	if (!(pPipe = m_pAISystem->OpenGoalPipe(pipename))) 
		return pH->EndFunctionNull();

	if (goalname == AIOP_ACQUIRETARGET)
	{
			pH->GetParam(4,temp);
			IAIObject *pObject = m_pAISystem->GetAIObjectByName(AIOBJECT_WAYPOINT,temp);
			params.m_pTarget = pObject;
			pPipe->PushGoal(AIOP_ACQUIRETARGET,blocking,params);
	}
	else if (goalname == AIOP_FORM)
	{
			pH->GetParam(4,temp);
			params.szString = temp;
			pPipe->PushGoal(AIOP_FORM,blocking,params);
	}
	else if (goalname == AIOP_PATHFIND)
	{
			pH->GetParam(4,temp);
			IAIObject *pObject = 0;
			if (strlen(temp) > 0) 
				pObject = m_pAISystem->GetAIObjectByName(0,temp);
			params.m_pTarget = pObject;
			params.szString = temp;
			pPipe->PushGoal(AIOP_PATHFIND,blocking,params);
	}
	else if (goalname == AIOP_LOCATE)
	{
		if (pH->GetParamType(4) == svtString)
		{
			const char *temp;
			pH->GetParam(4,temp);
			params.szString = temp;
			params.nValue = 0;
		}
		else if (pH->GetParamType(4) == svtNumber)
		{
			params.szString.clear();
			pH->GetParam(4,params.nValue);
		}

		pPipe->PushGoal(AIOP_LOCATE,blocking,params);
	}
	else if (goalname == AIOP_HIDE)
	{
			pH->GetParam(4,params.fValue);
			pH->GetParam(5,params.nValue);
			if (pH->GetParamCount() > 5 ) 
				pH->GetParam(6,params.bValue);
			else
				params.bValue = false;
			pPipe->PushGoal(AIOP_HIDE,blocking,params);
	}
	else if (goalname == AIOP_JUMP)
	{
			pH->GetParam(4,params.fValue);
			pH->GetParam(5,params.nValue);
			if (pH->GetParamCount() > 5 ) 
				pH->GetParam(6,params.bValue);
			else
				params.bValue = false;
			if (pH->GetParamCount()>6)
				pH->GetParam(7,params.fValueAux);
			else
				params.fValueAux = 20.f;	// default degrees for jump
			pPipe->PushGoal(AIOP_JUMP,blocking,params);
	}
	else if (goalname == AIOP_TRACE)
	{
			if (pH->GetParamCount()>3)
			{
				pH->GetParam(4,params.nValue);
				if (pH->GetParamCount() > 4)
					pH->GetParam(5,params.fValue);
				else
					params.fValue = 0;
			}
			else
			{
				params.fValue = 0;
				params.nValue = 0;
			}
			pPipe->PushGoal(AIOP_TRACE,blocking,params);
	}
	else if (goalname == AIOP_LOOKAT)
	{
			pH->GetParam(4,params.fValue);
			pH->GetParam(5,params.nValue);
			pPipe->PushGoal(AIOP_LOOKAT,blocking,params);
	}
	else if (goalname == AIOP_SIGNAL)
	{
			pH->GetParam(4,params.fValue);	// get the signal id
			params.nValue = 0;

			if (pH->GetParamCount()>4)
			{
				const char *sTemp;
				pH->GetParam(5,sTemp);	// get the signal text
				params.szString = sTemp;
			}

			if (pH->GetParamCount()>5)
			{
				pH->GetParam(6,params.nValue);	// get the desired filter
			}

			pPipe->PushGoal(AIOP_SIGNAL,blocking,params);
	}
	else if (goalname == AIOP_APPROACH)
	{
		pH->GetParam(4,params.fValue);
		if (params.fValue < 1)
			params.nValue = 1;
		else
			params.nValue = 0;
		if (pH->GetParamCount() > 4)
		{
			int useLastOpRes=0;
			pH->GetParam(5,useLastOpRes);
			params.bValue = (useLastOpRes!=0);
		}
		else 
			params.bValue = false;
		pPipe->PushGoal(goalname,blocking,params);
	}
	else if (goalname == AIOP_BACKOFF)
	{
		pH->GetParam(4,params.fValue);
		if (params.fValue < 1)
			params.nValue = 1;
		else
			params.nValue = 0;
		pPipe->PushGoal(goalname,blocking,params);
	}
	else if (goalname == AIOP_TIMEOUT)
	{
		pH->GetParam(4,params.fValue);
		if (pH->GetParamCount() > 4)
			pH->GetParam(5,params.fValueAux);
		else 
			params.fValueAux = 0;
		pPipe->PushGoal(goalname,blocking,params);
	}
	else if (goalname == AIOP_LOOP)
	{
		pH->GetParam(4,params.fValue);
		if (pH->GetParamCount() > 4)
			pH->GetParam(5,params.nValue);
		else 
			params.nValue = 0;
		pPipe->PushGoal(goalname,blocking,params);
	}
	else if ( pH->GetParamCount() > 3 )
	{
		// with float parametar one
		pH->GetParam(4,params.fValue);
		pPipe->PushGoal(goalname,blocking,params);
	}
	else 
	{
		// without parameters
		pPipe->PushGoal(goalname,blocking,params);
	}

	return pH->EndFunctionNull();	
}


/*!Generates a sound event in the AI system with the given parameters.
	@param id of the sound that is played for this event
	@param position of the origin of the sound event
	@param radius in which this sound event should be heard
	@param threat value of the sound event 
	@param interest value of the sound event
	@param id of the entity who generated this sound event
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::SoundEvent(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(6);
	float fThreat,fInterest,fRadius;
	int nID;
	USER_DATA val;
	int cookie;
	
	pH->GetParamUDVal(1,val,cookie);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(2,*oVec);
	pH->GetParam(3,fRadius);
	pH->GetParam(4,fThreat);
	pH->GetParam(5,fInterest);
	pH->GetParam(6,nID);
	Vec3d pos = oVec.Get();

	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (pEntity)
	{
		if (pEntity->GetAI())
		{
			m_pAISystem->SoundEvent(val,pos,fRadius,fThreat,fInterest,pEntity->GetAI());
			//if (stricmp("Player",pEntity->GetName())!=0)
			//	m_pLog->LogToConsole("SoundEvent,%s, position=%f,%f,%f",pEntity->GetName(),pos.x,pos.y,pos.z);
		}
	}
	
	return pH->EndFunction();
}

/*!Log into special AI log file
	@param string to log
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::Log(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	const char *pLogText;
	pH->GetParam(1,pLogText);

	/*
	m_pLog->SetFileName("AILOG.txt");
	m_pLog->LogToFile(pLogText);
	m_pLog->SetFileName("Log.txt");
	*/

	return pH->EndFunction();
}

/*!Sends a signal to ai objects defined by the parameters given
	@param filter which defines to which subset of the ai object this signal will be delivered. One of the SIGNALFILTER_ defines
	@param integer defining the signal (value bigger than zero). If you are not sure, put 1
	@param text which will be sent using the signal
	@param entity id of the entity sending this signal
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::Signal(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(4);
	int cFilter;
	int nSignalID;
	const char *szSignalText;
	int EntityID;

	pH->GetParam(1,cFilter);
	pH->GetParam(2,nSignalID);
	pH->GetParam(3,szSignalText);
	pH->GetParam(4,EntityID);

	IEntity *pEntity = m_pEntitySystem->GetEntity(EntityID);

	if ((pEntity) && (pEntity->GetAI()))
			m_pAISystem->SendSignal(cFilter,nSignalID,szSignalText,pEntity->GetAI());
	
	return pH->EndFunction();
}

/*!Returns number of objects in a given group
	@param the id of the desired group
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::GetGroupCount(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1)
	int nEntityID;

	pH->GetParam(1,nEntityID);

	IEntity *pEntity = m_pEntitySystem->GetEntity(nEntityID);
	if (pEntity)
	{
		IAIObject *pObject = pEntity->GetAI();
		if (pObject)
		{
			IPuppet *pPuppet=0;
			if (pObject->CanBeConvertedTo(AIOBJECT_PUPPET,(void**)&pPuppet))
			{
				AgentParameters ap = pPuppet->GetPuppetParameters();
				return pH->EndFunction(m_pAISystem->GetGroupCount(ap.m_nGroup));
			}
		}
	}

	return pH->EndFunction(0);
	
}

/*! Retrieves the attention target of a given entity
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::GetAttentionTargetOf(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int nID;

	pH->GetParam(1,nID);
	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (pEntity && pEntity->GetAI())
	{
		IAIObject *pObject = pEntity->GetAI();
		IPipeUser *pPipeUser = 0;
		if (pObject->CanBeConvertedTo(AIOBJECT_PIPEUSER,(void**) &pPipeUser))
		{
			IAIObject *pTheTarget = pPipeUser->GetAttentionTarget();
			if (pTheTarget)
			{
				IEntity *pTargetEntity = 0;
				if (pTheTarget->GetType()==AIOBJECT_PLAYER || pTheTarget->GetType()==AIOBJECT_PUPPET)
					pTargetEntity = (IEntity *) pTheTarget->GetAssociation();

				if (pTargetEntity)
					return pH->EndFunction(pTargetEntity->GetScriptObject());
				else
				{
					if (pTheTarget->GetType()==AIOBJECT_DUMMY)
						return pH->EndFunction(AIOBJECT_DUMMY);
					else 
						return pH->EndFunction(AIOBJECT_NONE);
				}
			}
		}
	}

	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::ReloadAll(IFunctionHandler * pH)
{
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::MakePuppetIgnorant(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2);
	int nID;
	bool bIgnorant;

	pH->GetParam(1,nID);
	pH->GetParam(2,bIgnorant);
	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (pEntity && pEntity->GetAI())
	{
		IAIObject *pObject = pEntity->GetAI();
		IPuppet *pPuppet = 0;
		if (pObject->CanBeConvertedTo(AIOBJECT_PUPPET,(void**) &pPuppet))
		{
			AgentParameters params = pPuppet->GetPuppetParameters();
			params.m_bIgnoreTargets = bIgnorant;
			pPuppet->SetPuppetParameters(params);
		}
	}

	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::FreeSignal(IFunctionHandler * pH)
{
	//CHECK_PARAMETERS(4);
	float fRadius;
	int nSignalID,nID = 0;
	const char *szSignalText;
	CScriptObjectVector vPos(m_pScriptSystem,true);
	
	pH->GetParam(1,nSignalID);
	pH->GetParam(2,szSignalText);
	pH->GetParam(3,vPos);
	pH->GetParam(4,fRadius);
	if (pH->GetParamCount()>4)
		pH->GetParam(5,nID);

	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	IAIObject *pObject=0;
	if (pEntity)
	{
		pObject = pEntity->GetAI();
	}

	m_pAISystem->SendAnonimousSignal(nSignalID,szSignalText,vPos.Get(),fRadius,pObject);
		
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::SetAssesmentMultiplier(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2)
	int type;
	float fMultiplier;

	pH->GetParam(1,type);
	pH->GetParam(2,fMultiplier);

	if (type < 0)
	{
		m_pLog->Log("\002[AIWARNING] Tried to set assesment multiplier to a negative type. Not allowed.");
		return pH->EndFunction();
	}

	m_pAISystem->SetAssesmentMultiplier((unsigned short)type, fMultiplier);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::SetSpeciesThreatMultiplier(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2)
	int species;
	float fMultiplier;

	pH->GetParam(1,species);
	pH->GetParam(2,fMultiplier);

	m_pAISystem->SetSpeciesThreatMultiplier(species, fMultiplier);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::FindObjectOfType(IFunctionHandler * pH)
{	
	int type;
	Vec3d pos;
	float fRadius;
	int nFlags = 0;
	CScriptObjectVector vPos(m_pScriptSystem,true);

	if (pH->GetParamType(1) == svtNumber)
	{
		int nID;
		
		pH->GetParam(1,nID);
		pH->GetParam(2,fRadius);
		pH->GetParam(3,type);
		if (pH->GetParamCount()>3)
			pH->GetParam(4,nFlags);


		IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
		if (pEntity)
		{
			if (pEntity->GetAI())
			{
				IAIObject *pObject = m_pAISystem->GetNearestObjectOfType(pEntity->GetAI(),type, fRadius,nFlags);
				if (pObject)
				{
					if (type==6)
					{
						m_pLog->Log("\001 Entity %s requested a waypoint anchor! Here is a dump of its state:");
						m_pAISystem->DumpStateOf(pObject);
						return pH->EndFunctionNull();
					}
					
					IEntity *pEntity = (IEntity*)pObject->GetAssociation();
					if (pEntity)
					{
						pEntity->SendScriptEvent(ScriptEvent_Activate,0);

						if (pEntity->GetAI() != pObject)
							return pH->EndFunctionNull();
						else
							return pH->EndFunction(pObject->GetName()); 
					}
					else
						return pH->EndFunction(pObject->GetName()); 
				} 
			}
		}
	}
	else
	{
		CScriptObjectVector vReturnPos(m_pScriptSystem,true);
		pH->GetParam(1,vPos);
		pH->GetParam(2,fRadius);
		pH->GetParam(3,type);
		if (pH->GetParamCount()>3)
			pH->GetParam(4,vReturnPos);

		IAIObject *pObject = m_pAISystem->GetNearestObjectOfType(vPos.Get(),type, fRadius);
		if (pObject)
		{
			IEntity *pEntity = (IEntity*)pObject->GetAssociation();
			if (pEntity)
				pEntity->SendScriptEvent(ScriptEvent_Activate,0);
			vReturnPos.Set(pObject->GetPos());
			return pH->EndFunction(pObject->GetName());
		}
	}

	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::GetGroupOf(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);

	int nID;

	pH->GetParam(1,nID);

	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	IAIObject *pObject=0;
	if (pEntity)
	{
		pObject = pEntity->GetAI();
		if (pObject)
		{
			IPuppet *pPuppet = 0;
			if (pObject->CanBeConvertedTo(AIOBJECT_PUPPET,(void**)&pPuppet))
			{
				return pH->EndFunction(pPuppet->GetPuppetParameters().m_nGroup);
			}
		}
	}
	return pH->EndFunction(0);
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::GetAnchor(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(3);

	int nAnchor;
	int nID;
	float fRadius;

	pH->GetParam(1,nID);
	pH->GetParam(2,nAnchor);
	pH->GetParam(3,fRadius);

	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	IAIObject *pObject = 0;
	if (pEntity)
	{
		pObject = pEntity->GetAI();
		if (pObject)
		{
			IAIObject *pAnchor = m_pAISystem->GetNearestToObject(pObject,nAnchor,fRadius);
			if (pAnchor)
				return pH->EndFunction(pAnchor->GetName());
		}
	}

	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::GetPerception(IFunctionHandler * pH)
{
	if (m_pAISystem && m_pGame)
	{
		if (m_pGame->GetMyPlayer())
		{
			if (m_pGame->GetMyPlayer()->GetAI())
				return pH->EndFunction(m_pAISystem->GetPerceptionValue(m_pGame->GetMyPlayer()->GetAI()));
		}
	}
	return pH->EndFunction(0);
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::RegisterWithAI(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(3);
	AIObjectParameters params;
	_SmartScriptObject pTable(m_pScriptSystem,true);
	_SmartScriptObject pTableInstance(m_pScriptSystem,true);
	int type;
	_SmartScriptObject pObj(m_pScriptSystem,true);
	_SmartScriptObject pTempObj(m_pScriptSystem,true);

	IEntity *pEntity;
	int nID;
	float fRadius = 0;

	params.bUsePathfindOutdoors = true;

	pH->GetParam(1,nID);
	pEntity=m_pEntitySystem->GetEntity(nID);
	if(pEntity==NULL)
	{
		TRACE("CScriptObjectAI::RegisterWithAI() entity in nil");
		return pH->EndFunctionNull();
	}

	pH->GetParam(2,type);
	
	switch (type)
	{
		case AIOBJECT_PUPPET:
			{
				float fwdspeed,bckspeed;
				CXPuppetProxy *pProxy;
				pProxy = new CXPuppetProxy(pEntity,m_pScriptSystem,m_pGame);

				if (pH->GetParamCount() > 2)
					pH->GetParam(3,*pTable);
				else
					return pH->EndFunction();

				if (pH->GetParamCount() > 3)
					pH->GetParam(4,*pTableInstance);
				else
					return pH->EndFunction();

				if (!pTable->GetValue("special",params.m_sParamStruct.m_bSpecial))
						params.m_sParamStruct.m_bSpecial = false;

				if (!pTable->GetValue("aggression",params.m_sParamStruct.m_fAggression))
					if (!pTableInstance->GetValue("aggression",params.m_sParamStruct.m_fAggression))
						return pH->EndFunction();
				if (!pTable->GetValue("sightrange",params.m_sParamStruct.m_fSightRange))
					if (!pTableInstance->GetValue("sightrange",params.m_sParamStruct.m_fSightRange))
							return pH->EndFunction();
				pTable->GetValue("fSpeciesHostility",params.m_sParamStruct.m_fSpeciesHostility);
				pTable->GetValue("bAffectSOM",params.m_sParamStruct.m_bPerceivePlayer);
				pTable->GetValue("bAwareOfPlayerTargeting",params.m_sParamStruct.m_bAwareOfPlayerTargeting);
				pTable->GetValue("fMeleeDistance",params.m_sParamStruct.m_fMeleeDistance);
				pTable->GetValue("fGroupHostility",params.m_sParamStruct.m_fGroupHostility);
				pTable->GetValue("fPersistence",params.m_sParamStruct.m_fPersistence);

				if (!pTable->GetValue("bSmartMelee",params.m_sParamStruct.m_bSmartMelee))
					if (!pTableInstance->GetValue("bSmartMelee",params.m_sParamStruct.m_bSmartMelee))
						params.m_sParamStruct.m_bSmartMelee = true;


				if (!pTable->GetValue("soundrange",params.m_sParamStruct.m_fSoundRange))
					if (!pTableInstance->GetValue("soundrange",params.m_sParamStruct.m_fSoundRange))
							return pH->EndFunction();
				if (!pTable->GetValue("attackrange",params.m_sParamStruct.m_fAttackRange))
					if (!pTableInstance->GetValue("attackrange",params.m_sParamStruct.m_fAttackRange))
							return pH->EndFunction();
				if (!pTable->GetValue("commrange",params.m_sParamStruct.m_fCommRange))
					if (!pTableInstance->GetValue("commrange",params.m_sParamStruct.m_fCommRange))
							return pH->EndFunction();
				if (!pTable->GetValue("horizontal_fov",params.m_sParamStruct.m_fHorizontalFov))
					if (!pTableInstance->GetValue("horizontal_fov",params.m_sParamStruct.m_fHorizontalFov))
							return pH->EndFunction();
				if (!pTable->GetValue("gravity_multiplier",params.m_sParamStruct.m_fGravityMultiplier))
					params.m_sParamStruct.m_fGravityMultiplier = 1.f;
				pTable->GetValue("eye_height",params.fEyeHeight);
				pTable->GetValue("max_health",params.m_sParamStruct.m_fMaxHealth);
                if (!pTable->GetValue("accuracy",params.m_sParamStruct.m_fAccuracy))
					if (!pTableInstance->GetValue("accuracy",params.m_sParamStruct.m_fAccuracy))
							return pH->EndFunction();
				pTable->GetValue("responsiveness",params.m_sParamStruct.m_fResponsiveness);
				params.m_sParamStruct.m_fResponsiveness*=400.f/7.5f;
				if (!pTable->GetValue("groupid",params.m_sParamStruct.m_nGroup))
					if (!pTableInstance->GetValue("groupid",params.m_sParamStruct.m_nGroup))
							return pH->EndFunction();

				pTable->GetValue("species",params.m_sParamStruct.m_nSpecies);       
				pTable->GetValue("forward_speed",fwdspeed);
				pTable->GetValue("back_speed",bckspeed);

				float gameAggression = params.m_sParamStruct.m_fAggression * m_pGame->cv_game_Aggression->GetFVal();
				if (gameAggression > 1)
					gameAggression = 1;
				else if (gameAggression < 0)
					gameAggression = 0;

				params.m_sParamStruct.m_fAggression = gameAggression;
				params.m_sParamStruct.m_fOriginalAggression = gameAggression;
				params.m_sParamStruct.m_fAggression = 1.f -  params.m_sParamStruct.m_fAggression;

				float gameAccuracy = params.m_sParamStruct.m_fAccuracy * m_pGame->cv_game_Accuracy->GetFVal();
				if (gameAccuracy > 1)
					gameAccuracy = 1;
				else if (gameAccuracy < 0)
					gameAccuracy = 0;

				params.m_sParamStruct.m_fAccuracy = gameAccuracy;
				params.m_sParamStruct.m_fOriginalAccuracy = gameAccuracy;
				params.m_sParamStruct.m_fAccuracy = 1.f - params.m_sParamStruct.m_fAccuracy;

				float gameHealth = params.m_sParamStruct.m_fMaxHealth * m_pGame->cv_game_Health->GetFVal();

				params.m_sParamStruct.m_fMaxHealth = gameHealth;

				if (type != AIOBJECT_PUPPET)
					params.m_sParamStruct.m_bIgnoreTargets = true;				
				else
					params.m_sParamStruct.m_bIgnoreTargets = false;				

				pe_player_dimensions dims;

				dims.heightEye = params.fEyeHeight;
				dims.heightCollider = params.fEyeHeight;
				dims.sizeCollider.x = params.fEyeHeight;

				pProxy->SetPuppetDimensions(dims.heightEye+0.05f,dims.heightEye,dims.heightCollider,dims.sizeCollider.x);
				//pProxy->SetRootBone(rootbone);
				pProxy->SetSpeeds(fwdspeed,bckspeed);

				params.pProxy = (IUnknownProxy *) pProxy;
			}
			break;
		case AIOBJECT_CAR:
		case AIOBJECT_BOAT:
		case AIOBJECT_HELICOPTER:
		case AIOBJECT_AIRPLANE:
			{
				const char *rootbone;
				float fwdspeed,bckspeed;
				float	minAlt;
				CXVehicleProxy *pProxy;
//				pProxy = new CXPuppetProxy(m_pEntity,m_pScriptSystem,m_pGame);
				pProxy = new CXVehicleProxy(pEntity,m_pScriptSystem,m_pGame);

				if (pH->GetParamCount() > 2)
					pH->GetParam(3,*pTable);
				else
					return pH->EndFunction();
				if (pH->GetParamCount() > 3)
					pH->GetParam(4,*pTableInstance);
				else
					return pH->EndFunction();


				pTable->GetValue("aggression",params.m_sParamStruct.m_fAggression);
				if (!pTable->GetValue("sightrange",params.m_sParamStruct.m_fSightRange))
					if (!pTableInstance->GetValue("sightrange",params.m_sParamStruct.m_fSightRange))
							return pH->EndFunction();
				pTable->GetValue("fSpeciesHostility",params.m_sParamStruct.m_fSpeciesHostility);
				pTable->GetValue("fGroupHostility",params.m_sParamStruct.m_fGroupHostility);
				pTable->GetValue("fPersistence",params.m_sParamStruct.m_fPersistence);
				if (!pTable->GetValue("soundrange",params.m_sParamStruct.m_fSoundRange))
					if (!pTableInstance->GetValue("soundrange",params.m_sParamStruct.m_fSoundRange))
							return pH->EndFunction();
				pTable->GetValue("attackrange",params.m_sParamStruct.m_fAttackRange);
				pTable->GetValue("commrange",params.m_sParamStruct.m_fCommRange);
				pTable->GetValue("horizontal_fov",params.m_sParamStruct.m_fHorizontalFov);
//				pTable->GetValue("vertical_fov",params.m_sParamStruct.m_fVerticalFov);
				pTable->GetValue("eye_height",params.fEyeHeight);
				pTable->GetValue("max_health",params.m_sParamStruct.m_fMaxHealth);
				pTable->GetValue("accuracy",params.m_sParamStruct.m_fAccuracy);
				params.m_sParamStruct.m_fAccuracy = 1.f - params.m_sParamStruct.m_fAccuracy;
				pTable->GetValue("responsiveness",params.m_sParamStruct.m_fResponsiveness);
				if (!pTable->GetValue("groupid",params.m_sParamStruct.m_nGroup))
					if (!pTableInstance->GetValue("groupid",params.m_sParamStruct.m_nGroup))
							return pH->EndFunction();
				pTable->GetValue("species",params.m_sParamStruct.m_nSpecies);       
				pTable->GetValue("cohesion",params.m_sParamStruct.m_fCohesion);       
				pTable->GetValue("bUsePathfind",params.bUsePathfindOutdoors);


				pTable->GetValue("root_bone",rootbone);
				pTable->GetValue("forward_speed",fwdspeed);
				pTable->GetValue("back_speed",bckspeed);
				pTable->GetValue("fFlightAltitudeMin",minAlt);
				

//				if (type != AIOBJECT_PUPPET)
//					params.m_sParamStruct.m_bIgnoreTargets = true;				
//				else
				params.m_sParamStruct.m_bIgnoreTargets = false;				

				pe_player_dimensions dims;

				pProxy->SetMinAltitude( minAlt );
				pProxy->SetSpeeds(fwdspeed,bckspeed);

				pProxy->SetType( type );

//			pProxy->m_nSpecialMovement = type;
				params.pProxy = (IUnknownProxy *) pProxy;
			}
			break;
		case AIOBJECT_PLAYER:
			params.pProxy = new CXObjectProxy(pEntity,m_pScriptSystem);

			if (pH->GetParamCount() > 1)
					pH->GetParam(3,*pTable);
			else
					return pH->EndFunction();

			pTable->GetValue("eye_height",params.fEyeHeight);
			pTable->GetValue("groupid",params.m_sParamStruct.m_nGroup);       
			pTable->GetValue("species",params.m_sParamStruct.m_nSpecies);       
			break;
		case AIOBJECT_AWARE:
			{
				CXObjectProxy *pProxy = new CXObjectProxy(pEntity, m_pScriptSystem);
				const char *szSignalFunction;
			
				if (pH->GetParamCount() > 1)
						pH->GetParam(2,szSignalFunction);
				else
						return pH->EndFunction();
			
				pProxy->SetSignalFuncName(szSignalFunction);
				params.pProxy = pProxy;
			}
			break;
		case AIOBJECT_ATTRIBUTE:
			{
				if (!pEntity->RegisterInAISystem(type,params))
				{
					m_pLog->Log("Could not register entity with AI");
				}
				else if (pEntity->GetAI())
				{
					if (pH->GetParamCount() > 2)
					{
						int ownerID;
						IEntity *pOwnerEntity;
						pH->GetParam(3,ownerID);
						pOwnerEntity=m_pEntitySystem->GetEntity(ownerID);
						if(pOwnerEntity==NULL)
						{
							m_pLog->Log("\002 CScriptObjectAI::RegisterWithAI() Owner entity of attribute AI %s cannot be found",pEntity->GetName());
							return pH->EndFunction();
						}

						pEntity->GetAI()->Bind(pOwnerEntity->GetAI());
					}
					else
						m_pLog->Log("\002 Entity %s registered as attribute AI without specifying owner",pEntity->GetName());
				}

			}
			return pH->EndFunction();
		default:
			{
				if (pH->GetParamCount() > 2)
					pH->GetParam(3,fRadius);
			}
			break;
	}

	if (!pEntity->RegisterInAISystem(type,params))
	{
		m_pLog->Log("Could not register entity with AI");
	}
	else
	{
		if (pEntity->GetAI())
		{
			pEntity->GetAI()->SetName(pEntity->GetName());
			pEntity->GetAI()->SetRadius(fRadius);
		}
	}
	
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::AIBind(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	IEntity *pParent;
	IEntity *pChild;
	int nID;

	pH->GetParam(1,nID);
	pParent=m_pEntitySystem->GetEntity(nID);
	if(pParent==NULL)
	{
		TRACE("CScriptObjectAI::AIBind() parent in nil");
		return pH->EndFunctionNull();
	}
	pH->GetParam(2,nID);
	pChild=m_pEntitySystem->GetEntity(nID);
	if(pChild==NULL)
	{
		TRACE("CScriptObjectAI::AIBind() chilt in nil");
		return pH->EndFunctionNull();
	}

	IAIObject *pParentAI = pParent->GetAI();
	IAIObject *pChildAI	= pChild->GetAI();

	if( pParentAI && pChildAI )
		pParentAI->Bind(pChildAI);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::CreateBoundObject(IFunctionHandler *pH)
{
CHECK_PARAMETERS(4);
	IEntity *pParent;
	int		type;
	int		nID;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,nID);
	pH->GetParam(2,type);
	pH->GetParam(3,*oVec);
	Vec3d pos = oVec.Get();
	pH->GetParam(4,*oVec);
	Vec3d angl = oVec.Get();

	pParent=m_pEntitySystem->GetEntity(nID);
	if(pParent==NULL)
	{
		TRACE("CScriptObjectAI::CreateBoundObject() entity in nil");
		return pH->EndFunctionNull();
	}

	IAIObject *pParentAI = pParent->GetAI();
	if( pParentAI  )
	{
		pParentAI->CreateBoundObject( type, pos, angl );
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::Cloak(IFunctionHandler * pH)
{
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::DeCloak(IFunctionHandler * pH)
{
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::ProjectileShoot(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2);
	int nID;
	_SmartScriptObject pParamsTable(m_pScriptSystem,true);

	pH->GetParam(1,nID);
	pH->GetParam(2,pParamsTable);

	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (pEntity)
	{
		IAIObject *pAIObject = pEntity->GetAI();
		if (pAIObject)
		{
			IPipeUser *pPipeUser = 0;
			if (pAIObject->CanBeConvertedTo(AIOBJECT_PIPEUSER,(void**)&pPipeUser))
			{
				IAIObject *pTarget = pPipeUser->GetAttentionTarget();
				if (!pTarget)
					return pH->EndFunction(0.f);

				Vec3d vTargetPos = pTarget->GetPos();
				if (pTarget->GetAssociation() && (pTarget->GetType()!=AIOBJECT_WAYPOINT))
				{
					IEntity *pTargetEntity = (IEntity*)pTarget->GetAssociation();
					if (pTargetEntity)
					{
							if (pTargetEntity->GetContainer())
							{
								CPlayer* pPlayer = 0;
								if (pTargetEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
								{
									if (pPlayer->GetVehicle())
										pTargetEntity = pPlayer->GetVehicle()->GetEntity();
								}
							}

							// we have a real target - calculate
							vTargetPos = pTargetEntity->GetPos();

							IPhysicalEntity *pPhysicalEntity = pTargetEntity->GetPhysics();
							if (pPhysicalEntity)
							{ 
                                pe_status_dynamics pdyn;
								pPhysicalEntity->GetStatus(&pdyn);

								vTargetPos += pdyn.v*1.5f;
								//vTargetPos.z = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation(vTargetPos.x,vTargetPos.y);
							}
					}
				}
			//	else
			//	{
					// randomize end position
					vTargetPos.x+= float((rand()%20))-10.f;
					vTargetPos.y+= float((rand()%20))-10.f;
			//	}

				Vec3d vShot = vTargetPos - pEntity->GetPos();
				float d = vShot.GetLength();
				

				Vec3d vShotProjected = vShot;
				vShotProjected.z = 0;
				float dx = vShotProjected.GetLength();
				float dy = vShot.z;

				//  velocity of shot
				float vel;
				// calculate angles
				float beta = asin(dy/d);
				float gamma = 3.14f/4.f+beta;

				float sbeta,cbeta,sgamma,cgamma,tgamma;
				sbeta = sin(beta);
				cbeta = cos(beta);
				sgamma = sin(gamma);
				cgamma = cos(gamma);
				tgamma = sgamma/cgamma;
				float A = 4.f*9.81f * dx* dx;
				float B = dx*sin(2*gamma) - dy*cgamma*cgamma;

				vel = sqrt(A/B);									

				vShot/=d;
				Vec3 axis = vShot.Cross(Vec3d(0,0,1));
				Vec3 vLaunchDir = vShot.rotated(axis,3.14f/4.f);

				CScriptObjectVector vHeading(m_pScriptSystem);
				vHeading = GetNormalized(vLaunchDir);
				pParamsTable->SetValue("heading",vHeading);
				// now this is returned
				//pParamsTable->SetValue("initial_velocity",fVelocity);
				return pH->EndFunction(vel);
			}
		}
	}

	return pH->EndFunction(0.f);
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::SetTheSkip(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int nID;

	pH->GetParam(1,nID);

	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (!pEntity)
	{
		m_pAISystem->SetTheSkip( NULL );
		return pH->EndFunction();
	}

	IPhysicalEntity *pPhys = pEntity->GetPhysics();

	if(pPhys)
		m_pAISystem->SetTheSkip( pPhys );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::SetAllowedDeathCount(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int nDeaths;

	pH->GetParam(1,nDeaths);
	if (m_pAISystem->GetAutoBalanceInterface())
		m_pAISystem->GetAutoBalanceInterface()->SetAllowedDeathCount(nDeaths);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::Checkpoint(IFunctionHandler * pH)
{
	if (m_pAISystem->GetAutoBalanceInterface())
		m_pAISystem->GetAutoBalanceInterface()->Checkpoint();
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::RegisterPlayerHit(IFunctionHandler * pH)
{
	if (m_pAISystem->GetAutoBalanceInterface())
		m_pAISystem->GetAutoBalanceInterface()->RegisterPlayerHit();
	return pH->EndFunction();
	
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::FireOverride(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int nID;

	pH->GetParam(1,nID);
	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (pEntity)
	{
		IAIObject *pAIObject = pEntity->GetAI();
		if (pAIObject)
		{
			IUnknownProxy *pProxy = pAIObject->GetProxy();
			if (pProxy)
			{
				CXPuppetProxy *pPuppetProxy =  0;
				if (pProxy->QueryProxy(AIPROXY_PUPPET,(void**) &pPuppetProxy))
					pPuppetProxy->SetFireOverride();
			}
		}
	}

	return pH->EndFunction();
	
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::EnablePuppetMovement(IFunctionHandler * pH)
{
	//CHECK_PARAMETERS(2);
	int nID;
	int nEnable;
	float fDuration=-1;

	
	pH->GetParam(1,nID);
	pH->GetParam(2,nEnable);
	if (pH->GetParamCount()>2) 
		pH->GetParam(3,fDuration);
	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (pEntity)
	{
		IAIObject *pAIObject = pEntity->GetAI();
		if (pAIObject)
		{
			IUnknownProxy *pProxy = pAIObject->GetProxy();
			if (pProxy)
			{
				IPuppetProxy *pPuppetProxy =  0;
				if (pProxy->QueryProxy(AIPROXY_PUPPET,(void**) &pPuppetProxy))
					pPuppetProxy->MovementControl(nEnable==1, fDuration);
			}
		}
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::IsMoving(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int nID;

	pH->GetParam(1,nID);

	IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
	if (pEntity)
	{
		IAIObject *pAIObject = pEntity->GetAI();
		if (pAIObject)
		{
			if (pAIObject->IsMoving())
				return pH->EndFunction(true);
		}
	}

	return pH->EndFunction(false);
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::EnableNodesInSphere(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(3);
	float fRadius;
	bool bEnable;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	
	pH->GetParam(1,*oVec);
	pH->GetParam(2,fRadius);
	pH->GetParam(3,bEnable);

	
	if (bEnable)
		m_pAISystem->GetNodeGraph()->EnableInSphere(oVec.Get(),fRadius);
	else
		m_pAISystem->GetNodeGraph()->DisableInSphere(oVec.Get(),fRadius);

	return pH->EndFunction(false);
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectAI::GetStats(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem,true);

	pH->GetParam(1,*pTable);
	
	if (m_pAISystem)
	{
		if (m_pAISystem->GetAutoBalanceInterface())
		{
			AIBalanceStats stats;
			stats.bFinal = true;
			m_pAISystem->GetAutoBalanceInterface()->GetAutobalanceStats(stats);

			pTable->SetValue("AVGEnemyLifetime",stats.fAVGEnemyLifetime);
			pTable->SetValue("AVGPlayerLifetime",stats.fAVGPlayerLifetime);
			pTable->SetValue("CheckpointsHit",stats.nCheckpointsHit);
			pTable->SetValue("EnemiesKilled",stats.nEnemiesKilled);
			pTable->SetValue("ShotsFired",stats.nShotsFires);
			pTable->SetValue("ShotsHit",stats.nShotsHit);
			pTable->SetValue("SilentKills",stats.nSilentKills);
			pTable->SetValue("TotalEnemiesInLevel",stats.nTotalEnemiesInLevel);
			pTable->SetValue("TotalPlayerDeaths",stats.nTotalPlayerDeaths);
			pTable->SetValue("VehiclesDestroyed",stats.nVehiclesDestroyed);
			pTable->SetValue("TotalTimeSecs",stats.fTotalTimeSeconds);

			int sec,min,hr;
			sec = (int)stats.fTotalTimeSeconds;
			sec %=60;
			min = stats.fTotalTimeSeconds/60;
			hr = stats.fTotalTimeSeconds/3600;
			char str[256];
			sprintf(str,"%d:%d:%d",hr,min,sec);
			pTable->SetValue("TotalTime",str);
		}
	}
	return pH->EndFunction();
}
