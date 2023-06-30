
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectServerSlot.cpp
//
//  Description: 
//		ScriptObjectServerSlot.cpp: implementation of the ScriptObjectServerSlot class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scriptobjectserverslot.h"
#include "XSystemBase.h"
#include "XServerslot.h"

_DECLARE_SCRIPTABLEEX(CScriptObjectServerSlot);

CScriptObjectServerSlot::CScriptObjectServerSlot(void)
{
	m_pSS=NULL;
}

CScriptObjectServerSlot::~CScriptObjectServerSlot(void)
{
}

void CScriptObjectServerSlot::Create(IScriptSystem *pScriptSystem)
{
	m_pSS=NULL;
	Init(pScriptSystem,this);
}

void CScriptObjectServerSlot::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectServerSlot>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectServerSlot,BanByID);
	REG_FUNC(CScriptObjectServerSlot,BanByIP);
	REG_FUNC(CScriptObjectServerSlot,SetPlayerId);
	REG_FUNC(CScriptObjectServerSlot,GetName);
	REG_FUNC(CScriptObjectServerSlot,GetModel);
	REG_FUNC(CScriptObjectServerSlot,GetColor);
	REG_FUNC(CScriptObjectServerSlot,SetGameState);
	REG_FUNC(CScriptObjectServerSlot,SendText);
	REG_FUNC(CScriptObjectServerSlot,Ready);
	REG_FUNC(CScriptObjectServerSlot,IsReady);
	REG_FUNC(CScriptObjectServerSlot,IsContextReady);
	REG_FUNC(CScriptObjectServerSlot,ResetPlayTime);
	REG_FUNC(CScriptObjectServerSlot,SendCommand);
	REG_FUNC(CScriptObjectServerSlot,Disconnect);
	REG_FUNC(CScriptObjectServerSlot,GetId);
	REG_FUNC(CScriptObjectServerSlot,GetPlayerId);
	REG_FUNC(CScriptObjectServerSlot,GetPing);
	REG_FUNC(CScriptObjectServerSlot,GetPlayTime);
}

int CScriptObjectServerSlot::BanByID(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pSS->BanByID();

	return pH->EndFunctionNull();
}

int CScriptObjectServerSlot::BanByIP(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pSS->BanByIP();

	return pH->EndFunctionNull();
}

/*!Send a string from the Serverslot(Server) to the Client (there OnServerCommand is invoked) 
*/
int CScriptObjectServerSlot::SendCommand(IFunctionHandler *pH)
{
	if(pH->GetParamCount()==1)
	{
		const char *sString;
		if(pH->GetParam(1,sString))
			m_pSS->SendCommand(sString);
	}
	else		
	{
		if(pH->GetParamCount()!=5)
		{
			m_pScriptSystem->RaiseError( "CScriptObjectServerSlot::SendCommand : %d arguments passed, 1 or 4 expected)", pH->GetParamCount());
			return pH->EndFunction();
		}

		const char *sString;
		Vec3d vPos,vNormal;
		int Id;
		int iUserByte;

		if(!pH->GetParam(1,sString))
			return pH->EndFunction();

		{
			_SmartScriptObject tpos(m_pScriptSystem, true);
			_VERIFY(pH->GetParam(2, tpos));
			float x,y,z;
			_VERIFY(tpos->GetValue("x", x));
			_VERIFY(tpos->GetValue("y", y));
			_VERIFY(tpos->GetValue("z", z));
			vPos=Vec3d(x,y,z);
//			m_pScriptSystem->PushFuncParam(vPos);
		}
		{
			_SmartScriptObject tnormal(m_pScriptSystem, true);
			_VERIFY(pH->GetParam(3, tnormal));
			float x,y,z;
			_VERIFY(tnormal->GetValue("x", x));
			_VERIFY(tnormal->GetValue("y", y));
			_VERIFY(tnormal->GetValue("z", z));
			vNormal=Vec3d(x,y,z);
//			m_pScriptSystem->PushFuncParam(vNormal);
		}
		if(!pH->GetParam(4,Id))
			return pH->EndFunction();
		if(!pH->GetParam(5,iUserByte))
			return pH->EndFunction();

		m_pSS->SendCommand(sString,vPos,vNormal,(EntityId)Id,(unsigned char)iUserByte);
	}


	return pH->EndFunction();
}

int CScriptObjectServerSlot::Disconnect(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	char *szCause = "";

	pH->GetParam(1, szCause);

	m_pSS->Disconnect(szCause);

	return pH->EndFunctionNull();
}

/*!Set the state of the game
	@param state state of the game (like intermission, prewar etc..)
	@param time current time
*/
int CScriptObjectServerSlot::SetGameState(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int state, time;
	pH->GetParam(1,state);
	pH->GetParam(2,time);
	if(m_pSS)
	{
		m_pSS->SetGameState(state, time);
		m_pSS->m_bForceScoreBoard = state==CGS_INTERMISSION;
	}
	return pH->EndFunction();
}

/*!assign the player entity to the serverslot	this entity from now will be considered
	the player controlled by the remote client assigned to this server slot
	@param nID the entity id
*/
int CScriptObjectServerSlot::SetPlayerId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nID;

	if(pH->GetParam(1,nID) && m_pSS)
		m_pSS->SetPlayerID(nID);

	return pH->EndFunction();
}

/*!retrieve the id of the entity(player)
	the player controlled by the remote client assigned to this server slot
	@return the entity id
*/
int CScriptObjectServerSlot::GetPlayerId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	int nID=0;

	if(m_pSS)
		nID=m_pSS->GetPlayerId();

	return pH->EndFunction(nID);
}

/*!retrieve the name of the player assigned to this serverslot
	@return the name of the player assigned to this serverslot
*/
int CScriptObjectServerSlot::GetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	const char *sName="";

	if(m_pSS)
		sName=m_pSS->GetName();

	return pH->EndFunction(sName);
}


/*!retrieve the name of the player assigned to this serverslot
@return the name of the player assigned to this serverslot
*/
int CScriptObjectServerSlot::GetModel(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	const char *sModel="";

	if(m_pSS)
		sModel=m_pSS->GetModel();

	return pH->EndFunction(sModel);
}


/*!retrieve the color of the player assigned to this serverslot
@return string which was specified on the client 
*/
int CScriptObjectServerSlot::GetColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	const char *sColor="";

	if(m_pSS)
		sColor=m_pSS->GetColor();

	return pH->EndFunction(sColor);
}


/*!send a tring of text to the client assigned to this serverslot
	@param sText the text that as to be sent
*/
int CScriptObjectServerSlot::SendText(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(1);
	const char *sText=NULL;
	float lifetime=DEFAULT_TEXT_LIFETIME;
	if(pH->GetParamCount()){
		if(pH->GetParam(1,sText) && m_pSS && sText)
		{
			if(pH->GetParamCount()>1){
					pH->GetParam(2,lifetime);
			}
			m_pSS->SendText(sText,lifetime);
		}
	}
	return pH->EndFunction();
}

/*!set the readyness state
	@param bReady !=nil is true nil is false
*/
int CScriptObjectServerSlot::Ready(IFunctionHandler *pH)
{
	bool bReady;
	pH->GetParam(1,bReady);
	m_pSS->m_bReady=bReady;
	return pH->EndFunction();
}

/*!quesry the readyness state
*/
int CScriptObjectServerSlot::IsReady(IFunctionHandler *pH)
{
	return pH->EndFunction(m_pSS->m_bReady);
}

int CScriptObjectServerSlot::IsContextReady(IFunctionHandler *pH)
{
	return pH->EndFunction(m_pSS->IsContextReady());
}

int CScriptObjectServerSlot::ResetPlayTime(IFunctionHandler *pH)
{
	m_pSS->ResetPlayTime();
	return pH->EndFunction();
}

int CScriptObjectServerSlot::GetPing(IFunctionHandler *pH)
{
	return pH->EndFunction((int)m_pSS->GetPing());
}


int CScriptObjectServerSlot::GetPlayTime(IFunctionHandler *pH)
{
	return pH->EndFunction(m_pSS->GetPlayTime());
}

int CScriptObjectServerSlot::GetId(IFunctionHandler *pH)
{
	return pH->EndFunction((int)m_pSS->GetID());
}