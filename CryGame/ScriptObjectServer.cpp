
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IXSystem.h"
#include "ScriptObjectServer.h"
#include "ScriptObjectVector.h"
//#include "TeamMgr.h"

_DECLARE_SCRIPTABLEEX(CScriptObjectServer)

CScriptObjectServer::CScriptObjectServer()
{
	m_pServer=NULL;
	m_pSlotMap=NULL;
}

CScriptObjectServer::~CScriptObjectServer()
{
	if(m_pSlotMap)
		m_pSlotMap->Release();
	m_pSlotMap=NULL;
}
//! create the object into the LUA VM
bool CScriptObjectServer::Create(IScriptSystem *pScriptSystem,IXSystem *pXSystem,CXGame *pGame)
{
	m_pXSystem=pXSystem;
	m_pGame=pGame;
	InitGlobal(pScriptSystem,"Server",this);
	m_pSlotMap=pScriptSystem->CreateObject();

	
	return true;
}

void CScriptObjectServer::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectServer>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectServer,Unban);
	REG_FUNC(CScriptObjectServer,ListBans);
	REG_FUNC(CScriptObjectServer,GetServerSlotMap);
//	REG_FUNC(CScriptObjectServer,IsValidServerSlotId);
	REG_FUNC(CScriptObjectServer,GetServerSlotByEntityId);
	REG_FUNC(CScriptObjectServer,GetServerSlotBySSId);
//	REG_FUNC(CScriptObjectServer,MultiCastObituary);
	REG_FUNC(CScriptObjectServer,GetNumPlayers);
	REG_FUNC(CScriptObjectServer,BroadcastText);
	REG_FUNC(CScriptObjectServer,BroadcastCommand);
	REG_FUNC(CScriptObjectServer,SpawnEntity);
	REG_FUNC(CScriptObjectServer,RemoveEntity);

	REG_FUNC(CScriptObjectServer,AddTeam);
	REG_FUNC(CScriptObjectServer,RemoveTeam);
	REG_FUNC(CScriptObjectServer,AddToTeam);
	REG_FUNC(CScriptObjectServer,RemoveFromTeam);
	REG_FUNC(CScriptObjectServer,SetTeamScoreByEntity);
	REG_FUNC(CScriptObjectServer,GetTeamMemberCount);
	REG_FUNC(CScriptObjectServer,SetTeamScore);
	REG_FUNC(CScriptObjectServer,SetTeamFlags);

	REG_FUNC(CScriptObjectServer,GetRespawnPoint);
	REG_FUNC(CScriptObjectServer,GetRandomRespawnPoint);
	REG_FUNC(CScriptObjectServer,GetNextRespawnPoint);
	REG_FUNC(CScriptObjectServer,GetPrevRespawnPoint);
	REG_FUNC(CScriptObjectServer,GetFirstRespawnPoint);
	REG_FUNC(CScriptObjectServer,GetName);
	REG_FUNC(CScriptObjectServer,DebugTest);
}

/////////////////////////////////////////////////////////
/*! return the number of players 
		@return the number of players 
*/
int CScriptObjectServer::GetNumPlayers(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0)
	return pH->EndFunction(m_pServer->GetNumPlayers());
}

/////////////////////////////////////////////////////////
//! return the serverslot that map on the given id
int CScriptObjectServer::GetServerSlotBySSId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1)
	int nId;
	if(!pH->GetParam(1,nId))
		return pH->EndFunction(false);

	CXServer::XSlotMap::iterator itor;

	itor=m_pServer->GetSlotsMap().find(nId);

	if(itor!=m_pServer->GetSlotsMap().end())
		return pH->EndFunction(itor->second->GetScriptObject());

	return pH->EndFunction(false);
}

int CScriptObjectServer::Unban(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	if (pH->GetParamType(1) != svtNumber)
	{
		return pH->EndFunctionNull();
	}

	int iBanNumber = -1;

	pH->GetParam(1, iBanNumber);

	if (iBanNumber >= 0)
	{
		if (iBanNumber < m_pServer->m_vBannedIDList.size())
		{
			BannedID Banned = m_pServer->m_vBannedIDList[iBanNumber];

			m_pServer->UnbanID(Banned);

			GetISystem()->GetILog()->Log("\001removed ban on player: %s", Banned.szName.c_str());
		}
		else if (iBanNumber < m_pServer->m_vBannedIDList.size() + m_pServer->m_vBannedIPList.size())
		{
			unsigned int Banned = m_pServer->m_vBannedIPList[iBanNumber-m_pServer->m_vBannedIDList.size()];

			m_pServer->UnbanIP(Banned);

			CIPAddress ip;
			ip.m_Address.ADDR = Banned;

			GetISystem()->GetILog()->Log("\001removed ban on ip: %s", ip.GetAsString());
		}
	}

	return pH->EndFunctionNull();
}

int CScriptObjectServer::ListBans(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	if (!m_pServer)
	{
		return pH->EndFunctionNull();
	}

	IConsole *pConsole = GetISystem()->GetIConsole();

	pConsole->PrintLine("\t#       name/ip");

	int i = 0;
	for (BannedIDList::iterator it = m_pServer->m_vBannedIDList.begin(); it != m_pServer->m_vBannedIDList.end(); ++it)
	{
		char szLine[512] = {0};
		sprintf(szLine, "\t%d   %s", i, it->szName.c_str());

		pConsole->PrintLine(szLine);
		++i;
	}
	for (BannedIPList::iterator it = m_pServer->m_vBannedIPList.begin(); it != m_pServer->m_vBannedIPList.end(); ++it)
	{
		char szLine[512] = {0};

		CIPAddress ip;
		ip.m_Address.ADDR = *it;

		sprintf(szLine, "\t%d   %s", i, ip.GetAsString());

		pConsole->PrintLine(szLine);
		++i;
	}

	return pH->EndFunctionNull();
}

/////////////////////////////////////////////////////////
/*! return a script table filled with the couple [serverslot id/serverslot script object]
	@return the table containing all serverslot objects
*/
int CScriptObjectServer::GetServerSlotMap(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0)

	CXServer::XSlotMap::iterator itor=m_pServer->GetSlotsMap().begin();
	CXServer::XSlotMap::iterator end=m_pServer->GetSlotsMap().end();

	m_pSlotMap->Clear();
	while(itor!=end)
	{
		m_pSlotMap->SetAt(itor->second->GetID(),itor->second->GetScriptObject());
		++itor;
	}
	return pH->EndFunction(m_pSlotMap);
}

/////////////////////////////////////////////////////////
// return if a creatin id is mapped to avalid serverslot
//		@param nId server slot id
//		@return !=nil(true) nil(false)
/*
int CScriptObjectServer::IsValidServerSlotId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1)
	int nId;
	if(!pH->GetParam(1,nId))
		return pH->EndFunction(false);

	CXServer::XSlotMap::iterator itor;

	itor=m_pServer->GetSlotsMap().find(nId);

	bool bRet=(itor!=m_pServer->GetSlotsMap().end());

	return pH->EndFunction(bRet);
}
*/

/////////////////////////////////////////////////////////
/*! retreive the serverslot passing the entity that identify his player
		@param nEntityId id of the entity
*/
int CScriptObjectServer::GetServerSlotByEntityId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1)
	int val;

	if(!pH->GetParam(1, val))
		return pH->EndFunction(false);

	EntityId nEntityId = val;

	CXServerSlot *pRet=m_pServer->GetServerSlotByEntityId(nEntityId);

	if(pRet)
			return pH->EndFunction(pRet->GetScriptObject());

	return pH->EndFunction(false);			// not found
}

/////////////////////////////////////////////////////////
/*
int CScriptObjectServer::MultiCastObituary(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	int targid, attid, situation;
	pH->GetParam(1, targid);
	pH->GetParam(2, attid);
	pH->GetParam(3, situation);
	CStream stm;
	stm.Write((EntityId)targid);
	stm.Write((EntityId)attid);
	stm.Write((char)situation);
	m_pServer->BroadcastReliable(XSERVERMSG_OBITUARY, stm,true);
	return pH->EndFunction();
}
*/

/////////////////////////////////////////////////////////
/*! Send text to all connected clients
		@param sText the text that must be sent
*/
int CScriptObjectServer::BroadcastText(IFunctionHandler *pH)
{
	const char *sText=NULL;
	if(pH->GetParam(1,sText) && m_pServer && sText)
	{
		float fLifeTime = DEFAULT_TEXT_LIFETIME;

		pH->GetParam(2, fLifeTime);
		m_pServer->BroadcastText(sText, fLifeTime);
	}

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////
/*! Destroys an entity
		@param A reference to the entity that is to be destroyed
		If the entity reference is invalid, the call is ignored
		@see RemoveEntity
*/
int CScriptObjectServer::RemoveEntity(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(1);
	ASSERT(pH && (pH->GetParamCount() == 1 || pH->GetParamCount() == 2));

	int iEntity=0;
	bool	bRemoveNow=false;
	//if(pH->GetParam(1, iEntity))
	pH->GetParam(1, iEntity);

	IEntity* pEntity=m_pXSystem->GetEntity(iEntity);
	if(!pEntity || pEntity->IsGarbage())
		return pH->EndFunction();

	if(pH->GetParamCount() == 2)
		bRemoveNow = true;
	if (iEntity)
		m_pXSystem->RemoveEntity((EntityId) iEntity, bRemoveNow);
	
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////
/*! Creates an entity
		@param The class id or class name of the entity that is to be created
		@param Optional world position of where the entity will be created
		@return If successful it returns a reference to the new entity that is created
		Otherwise it returns NULL
		@see RemoveEntity
*/
int CScriptObjectServer::SpawnEntity(IFunctionHandler *pH)
{
	// Ensure that there is only one parameter
//	CHECK_PARAMETERS(1);
	//static EntityId nEntityID=20000;
	// Added optional position parameter so that position is valid during the objects OnInit()
	ASSERT(pH && (pH->GetParamCount() == 1 || pH->GetParamCount() == 2));
	
	int iEntityClassID;
	EntityClassId ClassId;
	const char* sEntityClassName;
	const char *sName="";
	const char *sModel="";
	EntityClass *pClass;
	CEntityDesc ed;
	Vec3 pos(0,0,0);
	Vec3 angles(0,0,0);
	int requestedid=0;
	_SmartScriptObject pED(m_pScriptSystem,true);
	CScriptObjectVector o(m_pScriptSystem,true);
	_SmartScriptObject pProperties(m_pScriptSystem,true);
	bool bproperties=false;
	//if the first parameter is a table(is the entity description)
	if(pH->GetParam(1, pED))
	{
		if(!pED->GetValue("classid",iEntityClassID))
			return pH->EndFunctionNull();

		ClassId=(EntityClassId)iEntityClassID;
		pClass=m_pGame->GetClassRegistry()->GetByClassId(ClassId);
		if(!pClass)
			return pH->EndFunctionNull();

		pED->GetValue("name",sName);
		pED->GetValue("model",sModel);
		if(pED->GetValue("properties",pProperties))
		{
			bproperties=true;
		}
		if(pED->GetValue("pos",o))
		{
			pos=o.Get();
		}
		if(pED->GetValue("angles",o))
		{
			angles=o.Get();
		}
		pED->GetValue("id",requestedid);

		// color used for team identification
		CScriptObjectColor oCol(m_pScriptSystem,true); 

		if (pED->GetValue( "color",oCol ))
			ed.vColor = oCol.Get();
	}
	else
	{
		// First try reading the parameter as class ID (int)
		if(!pH->GetParam(1, iEntityClassID))
		{
			// Parameter is not an int so now try reading as a string
			if(!pH->GetParam(1, sEntityClassName))

				// Parameter appears to be invalid so bail
				return pH->EndFunctionNull();

			// Convert string to class id

			pClass=m_pGame->GetClassRegistry()->GetByClass(sEntityClassName);

			if(!pClass)
			{
				m_pGame->GetSystem()->GetILog()->LogError("Server:SpawnEntity failed, class name '%s' not recognized", sEntityClassName);
				return pH->EndFunctionNull();
			}

			ClassId = pClass->ClassId;
		}
		else
		{
			ClassId=(EntityClassId)iEntityClassID;

			pClass=m_pGame->GetClassRegistry()->GetByClassId(ClassId);
			if(!pClass)
				return pH->EndFunctionNull();
		}

		// We now have a valid class ID

		//Vec3 StartPosition(0.0, 0.0, 0.0);

		if(pH->GetParamCount() == 2)
		{
			CScriptObjectVector oVec(m_pScriptSystem,true);

			// Read the optional position parameter
			pH->GetParam(2,*oVec);

			pos = oVec.Get();
		}
	}
	
	ed.ClassId = ClassId;
	ed.className = pClass->strClassName.c_str();
	ed.pos = pos;
	ed.angles=angles;
	ed.name=sName;
	ed.sModel=sModel;
	ed.id=requestedid;
	if(bproperties)
	{
		ed.pProperties=pProperties;
	}

	IEntity* pEntity;

	ILog *pLog=m_pGame->GetSystem()->GetILog();
	if (pLog->GetVerbosityLevel()>5)
	{		
		// Timur, excessive logging.
		//pLog->Log("Attempt to spawn entity classname=%s,name=%s,type=%d,id=%d (%.2f %.2f %.2f)",ed.className.c_str(),ed.name.c_str(),(int)ed.ClassId,ed.id,pos.x,pos.y,pos.z);
	}

	// Attempt to spawn the object using the class ID
	if((pEntity = m_pXSystem->SpawnEntity(ed)) == NULL)
		// Spawn failed
		return pH->EndFunctionNull();

	// Returns a pointer to the IScriptObject
	return pH->EndFunction(pEntity->GetScriptObject());
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/*!add a team into the game
	@param pszName name of the team
*/
int CScriptObjectServer::AddTeam(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *pszName;
	pH->GetParam(1, pszName);
	//m_pGame->GetTeamManager()->AddTeam(pszName);
	m_pServer->AddTeam(pszName);
	//m_pServer->m_pISystem->AddTeam(pszName,-1);
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/*!remove a team from the game
	@param pszName name of the team
*/
int CScriptObjectServer::RemoveTeam(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *pszName;
	pH->GetParam(1, pszName);
	//m_pGame->GetTeamManager()->RemoveTeam(m_pGame->GetTeamManager()->NameToId(pszName));
	//int nTID=m_pServer->m_pISystem->GetTeamId(pszName);
	m_pServer->RemoveTeam(pszName);
	//m_pServer->m_pISystem->RemoveTeam(nTID);
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/*!add an entity into a team
	@param pszName name of the theam
	@param id the entity's id
*/
int CScriptObjectServer::AddToTeam(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *pszName;
	int id=0;
	if(pH->GetParam(1, pszName))
	{
		pH->GetParam(2, id);
		int nTID=m_pServer->m_pISystem->GetEntityTeam(id);
		if(nTID!=-1)
		{
			m_pServer->RemoveFromTeam(id);
		}
//		CTeamMgr *pTM=m_pGame->GetTeamManager();
//		CTeam *pTeam=pTM->GetTeam(pTM->NameToId(pszName));
//		if(pTeam)
//		{
//			pTeam->AddEntity(id);
//		}
		m_pServer->AddToTeam(pszName,id);
	}
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/*!remove an entity from a team
	@param id the entity's id
*/
int CScriptObjectServer::RemoveFromTeam(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int id=0;
	if(pH->GetParam(1, id))
	{
		//CTeamMgr *pTM=m_pGame->GetTeamManager();
		//if(pTM)
		//{
			//CTeam *pTeam=pTM->GetEntityTeam(id);
			//if(pTeam)
			//{
				//pTeam->RemoveEntity(id);
			//}
		//}
		m_pServer->RemoveFromTeam(id);
	}
	return pH->EndFunction();
}

////////////////////////////////////////////////////////////////////
//void (return vector)
////////////////////////////////////////////////////////////////////
/*!return a respawn point from the respawn points specified in levedata.xml by a given name
	@param name name of the respawn point
	@return if succeded a table with the fields x,y,z containing the pos of the respawn point if failed return nil
*/
int CScriptObjectServer::GetRespawnPoint(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	char *name=NULL;
	pH->GetParam(1,name);

	if(!name)
		return pH->EndFunction();
		
	m_pGame->GetSystem()->GetILog()->Log("GetRespawnPoint '%s'",name?name:"<nil>");

	ITagPoint *tag = m_pServer->GetRespawnPoint(name);

	if(tag != NULL)
	{
		_SmartScriptObject pTagPoint(m_pScriptSystem);
		MakeTagScriptObject(tag, pTagPoint);

		return pH->EndFunction(*pTagPoint);	
	}
	
	return pH->EndFunction();
}

////////////////////////////////////////////////////////////////////
//void (return vector)
////////////////////////////////////////////////////////////////////
/*!return a respawn first point from the respawn points specified in levedata.xml
	@return a table with the fields x,y,z containing the pos of the respawn point
*/
int CScriptObjectServer::GetFirstRespawnPoint(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	ITagPoint *tag = m_pServer->GetFirstRespawnPoint();

	if(tag != NULL)
	{
		_SmartScriptObject pTagPoint(m_pScriptSystem);
		MakeTagScriptObject(tag, pTagPoint);

		return pH->EndFunction(*pTagPoint);	
	}

	return pH->EndFunction();
}


////////////////////////////////////////////////////////////////////
//void (return vector)
////////////////////////////////////////////////////////////////////
/*!return a respawn next point from the respawn points specified in levedata.xml
	@return a table with the fields x,y,z containing the pos of the respawn point
*/
int CScriptObjectServer::GetNextRespawnPoint(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	ITagPoint *tag = m_pServer->GetNextRespawnPoint();

	if(tag != NULL)
	{
		_SmartScriptObject pTagPoint(m_pScriptSystem);
		MakeTagScriptObject(tag, pTagPoint);

		return pH->EndFunction(*pTagPoint);	
	}

	return pH->EndFunctionNull();
}


////////////////////////////////////////////////////////////////////
//void (return vector)
////////////////////////////////////////////////////////////////////
/*!return a respawn next point from the respawn points specified in levedata.xml
	@return a table with the fields x,y,z containing the pos of the respawn point
*/
int CScriptObjectServer::GetPrevRespawnPoint(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	ITagPoint *tag = m_pServer->GetPrevRespawnPoint();

	if(tag != NULL)
	{
		_SmartScriptObject pTagPoint(m_pScriptSystem);
		MakeTagScriptObject(tag, pTagPoint);

		return pH->EndFunction(*pTagPoint);	
	}

	return pH->EndFunctionNull();
}



////////////////////////////////////////////////////////////////////
//void (return vector)
////////////////////////////////////////////////////////////////////
/*!return a respawn random point from the respawn points specified in levedata.xml
	@return a table with the fields x,y,z containing the pos of the respawn point
*/
int CScriptObjectServer::GetRandomRespawnPoint(IFunctionHandler *pH)
{
	const char *sFilter=NULL;
	
	if(pH->GetParamCount())
		pH->GetParam(1, sFilter);
	
// 	m_pGame->GetSystem()->GetILog()->Log("GetRandomRespawnPoint '%s'",sFilter?sFilter:"<nil>");

	ITagPoint *tag = m_pServer->GetRandomRespawnPoint(sFilter);

	if(tag != NULL)
	{
		_SmartScriptObject pTagPoint(m_pScriptSystem);
		MakeTagScriptObject(tag, pTagPoint);

		return pH->EndFunction(*pTagPoint);	
	}

	return pH->EndFunction();
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/*! set the score of a team selecting the team by the entity id of an entity of the team
	@param EntityId the entity's id
	@param nScore the score that has to be set
*/
int CScriptObjectServer::SetTeamScoreByEntity(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int val;

	if(!pH->GetParam(1, val))
		return pH->EndFunction(false);

	EntityId nEntityId = val;

	int nScore;

	pH->GetParam(2, nScore);
	int nTID=m_pServer->m_pISystem->GetEntityTeam(nEntityId);
	if(nTID!=-1)
		m_pServer->m_pISystem->SetTeamScore(nTID,nScore);

	return pH->EndFunction();
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/*!set the score of a team 
@param teamname name of the team
@param score the score that has to be set
*/
int CScriptObjectServer::SetTeamScore(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *team;
	int score;
	if(pH->GetParam(1,team))
	{
		pH->GetParam(2,score);
		m_pServer->SetTeamScore(team,score);
	}
	return pH->EndFunctionNull();
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/*!set the score of a team 
@param teamname name of the team
@param flags the flags that has to be set
*/
int CScriptObjectServer::SetTeamFlags(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *team;
	int flags;
	if(pH->GetParam(1,team))
	{
		pH->GetParam(2,flags);
		m_pServer->SetTeamFlags(team,flags);
	}
	return pH->EndFunctionNull();
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/*!return the number of members of a certain team
	@param pszName name of the team
	@return if succeded the number of members of the team if failed return nil
*/
int CScriptObjectServer::GetTeamMemberCount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *pszName;
	if(pH->GetParam(1, pszName))
	{
		int nTID=m_pServer->m_pISystem->GetTeamId(pszName);

		if(nTID!=-1)
			return pH->EndFunction(m_pServer->m_pISystem->GetTeamMembersCount(nTID));
//		CTeamMgr *pTM=m_pGame->GetTeamManager();
//		CTeam *pTeam=pTM->GetTeam(pTM->NameToId(pszName));
	//	if(pTeam)
	//		return pH->EndFunction(pTeam->GetMemberCount());
	}
	return pH->EndFunction(0);
}

int CScriptObjectServer::GetName(IFunctionHandler *pH)
{	
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pServer->GetName());
}

// should be removed in release - pure debugging purpose
int CScriptObjectServer::DebugTest(IFunctionHandler *pH)
{	
/*  deactivate because it can be used to hack

	if(pH->GetParamCount()==0)
	{ 
		IEntitySystem *pEntitySystem=m_pGame->GetSystem()->GetIEntitySystem();

		DWORD dwCount=pEntitySystem->GetNumEntities();

		EntityId idChosen=(EntityId)(rand()%dwCount);

		IEntity *pEnt=pEntitySystem->GetEntity(idChosen);

		if(pEnt)
		{
			EntityClassId ClassId=pEnt->GetClassId();

			EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClassId(ClassId,false);
			assert(pClass);

			// debugging
			m_pGame->GetSystem()->GetILog()->Log("Cheat protection: %d/%d %p class=%s\n",(int)idChosen,dwCount,pEnt,pClass->strClassName.c_str());

//			if(pClass)
			m_pServer->SendRequestScriptHash(idChosen,"","");
		}
	} 
	else
	{
		CHECK_PARAMETERS(2);

		const char *path;
		const char *key;

		if(pH->GetParam(1,path))
			if(pH->GetParam(2,key))
				m_pServer->SendRequestScriptHash(0,path,key);
	}
*/
	return pH->EndFunctionNull();
}

int CScriptObjectServer::BroadcastCommand(IFunctionHandler *pH)
{
	if(pH->GetParamCount()==1)
	{
		const char *cmd;
		if(pH->GetParam(1,cmd))
		{
			m_pServer->BroadcastCommand(cmd);
		}
	}
	else
	{
		CHECK_PARAMETERS(5);

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

		m_pServer->BroadcastCommand(sString,vPos,vNormal,(EntityId)Id,(unsigned char)iUserByte);
	}
	return pH->EndFunction();
}

void CScriptObjectServer::MakeTagScriptObject(ITagPoint* pInTagPoint, _SmartScriptObject& rOut)
{
	Vec3 spawnpt, spawnangles;
	pInTagPoint->GetPos(spawnpt);
	pInTagPoint->GetAngles(spawnangles);
	char* spawnname = pInTagPoint->GetName();

	rOut->SetValue("x",spawnpt.x);
	rOut->SetValue("y",spawnpt.y);
	rOut->SetValue("z",spawnpt.z);
	rOut->SetValue("xA",spawnangles.x);
	rOut->SetValue("yA",spawnangles.y);
	rOut->SetValue("zA",spawnangles.z);
	rOut->SetValue("name",spawnname);
}