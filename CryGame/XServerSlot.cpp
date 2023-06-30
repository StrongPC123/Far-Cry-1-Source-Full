//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XServerSlot.cpp 
//  Description: XServerSlot implementation.
//
//  History: 
//  - August 3, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XServerSlot.h"
#include "XSystemServer.h"

#include "XPlayer.h"
#include "XVehicle.h"
#include "Spectator.h"								// CSpectator
#include "AdvCamSystem.h"							// CAdvCamSystem
#include "StreamData.h"								// CStreamData_WorldPos
#include "PlayerSystem.h"
#include <functional>
#include "ScriptObjectServerSlot.h"



  
///////////////////////////////////////////////
CXServerSlot::CXServerSlot(CXServer *pParent, IServerSlot *pSlot)
{
	m_fLastClientStringTime=0;
	m_wPlayerId = INVALID_WID;	
	m_bXServerSlotGarbage = false;
	m_bCanSpawn = false;
	m_bWaitingForContextReady=false;
	m_bLocalHost = false;
	m_pLog=pParent->m_pGame->m_pLog;
	// init the interface with the given pointer
	m_pISSlot = pSlot;
	m_pISSlot->Advise(this);
	// set the parent server
	m_pParent = pParent;
	m_pTimer=pParent->m_pTimer;
	m_Snapshot.Init(pParent,this);
	m_Snapshot.SetSendPerSecond(20);
	m_bForceScoreBoard = false;
	m_fLastScoreBoardTime = 0;
	m_ScriptObjectServerSlot.Create(pParent->m_pGame->GetScriptSystem());
	m_ScriptObjectServerSlot.SetServerSlot(this);
	m_pPhysicalWorld=pParent->m_pGame->m_pSystem->GetIPhysicalWorld();
	m_bReady=false;
	m_nDesyncFrames = 0;
	m_iLastEventSent = -1;
	m_bContextIsReady=false;
	m_ClassId=0;
	m_idClientVehicle = 0;
	m_fClientVehicleSimTime = -1.0f;

	memset(m_vGlobalID, 0, 64);
	m_bGlobalIDSize = 0;
	memset(m_vAuthID, 0, 64);
	m_bAuthIDSize = 0;
	m_bServerLazyChannelState=false;	// start with false on client and serverslot side
	m_bClientLazyChannelState=false;	// start with false on client and serverslot side
	m_dwUpdatesSinceLastLazySend=0;
}

///////////////////////////////////////////////
CXServerSlot::~CXServerSlot()
{
	//m_pParent->m_pISystem->RemoveEntity(m_wPlayerID);
	m_wPlayerId = INVALID_WID;
	SAFE_RELEASE(m_pISSlot);
	m_pParent = NULL;
}


bool CXServerSlot::IsEntityOffSync(EntityId id) 
{ 
	bool bOffSync = true;

	if (m_pParent->m_pGame->IsMultiplayer() && m_pParent->m_pGame->UseFixedStep())
	{
		std::map<EntityId,int>::iterator itor = m_mapOffSyncEnts.find(id);
		if (itor!=m_mapOffSyncEnts.end())
			m_mapOffSyncEnts.erase(itor);
		else
		{
			// always send full snapshots of driven vehicles, since with absense of scheduling they'll go off-sync anyway, 
			// but it will take longer to update them in this case (=more perceived lag)
			IEntity *pEnt = m_pParent->m_pISystem->GetEntity(id);
			CVehicle *pVehicle;
			IEntityContainer *pContainer;
			if (!(pEnt && (pContainer = pEnt->GetContainer()) && 
					pContainer->QueryContainerInterface(CIT_IVEHICLE,(void**)&pVehicle) && pVehicle->HasDriver()))
				bOffSync = false;
		}
	}
	return bOffSync;
}


///////////////////////////////////////////////
void CXServerSlot::OnXServerSlotConnect(const BYTE *pbAuthorizationID, unsigned int uiAuthorizationSize)
{
	NET_TRACE("<<NET>>CXServerSlot::OnConnect");

	memcpy(m_vAuthID, pbAuthorizationID, min(64, uiAuthorizationSize));
	m_bAuthIDSize = min(64, uiAuthorizationSize);
}

///////////////////////////////////////////////
void CXServerSlot::OnXPlayerAuthorization( bool bAllow, const char *szError, const BYTE *pGlobalID,
	unsigned int uiGlobalIDSize )
{
	//TODO: save the GlobalID
	if (!bAllow)
	{
		Disconnect(szError);

		return;
	}

	// this should only be false in LAN games
	if (pGlobalID && uiGlobalIDSize)
	{
		m_bGlobalIDSize = uiGlobalIDSize;
		memcpy(m_vGlobalID, pGlobalID, min(64, uiGlobalIDSize));

		// check if it is banned or not
		BannedID ID(pGlobalID, uiGlobalIDSize, "");

		if (m_pParent->IsIDBanned(ID))
		{
			Disconnect("@Banned");

			return;
		}
	}
	else
	{
		memset(m_vGlobalID, 0, 64);
		m_bGlobalIDSize = 0;
	}

	ContextSetup();
	ResetPlayTime();
}


///////////////////////////////////////////////
void CXServerSlot::Disconnect(const char *sCause)
{
	if(m_pISSlot)
		m_pISSlot->Disconnect(sCause);
}

///////////////////////////////////////////////
void CXServerSlot::OnXServerSlotDisconnect(const char *szCause)
{
	// if the player is not fully connected,
	// no action should be taken
	if (m_bContextIsReady)
	{
		m_pParent->GetRules()->OnClientDisconnect(m_ScriptObjectServerSlot.GetScriptObject());
	}

	m_wPlayerId = INVALID_WID;
	// Unregister this slot
	m_pParent->UnregisterXSlot(GetID());

	// set as a garbage
	m_bXServerSlotGarbage = true;
}

void CXServerSlot::OnSpawnEntity(CEntityDesc &ed,IEntity *pEntity,bool bSend)
{
	if(!m_bCanSpawn)
	{
		NET_TRACE("<<NET>>CXServerSlot::OnSpawnEntity[%s] [%d] NOT READY TO SPAWN!",GetName() ,pEntity->GetId());
		return;
	}
	if(bSend)
	{
		CStream stm;

		ed.Write(m_pParent->m_pGame->GetIBitStream(),stm);
		SendReliableMsg(XSERVERMSG_ADDENTITY,stm,false,(const char *)ed.className);
	}
	NET_TRACE("<<NET>>CXServerSlot::OnSpawnEntity[%s] [%d]",GetName(),pEntity->GetId());
	m_Snapshot.AddEntity(pEntity);

	if (pEntity->GetPhysics())
	{
		pe_params_flags pf;
		pf.flagsOR = pef_monitor_impulses;
		pEntity->GetPhysics()->SetParams(&pf);
	}
}

void CXServerSlot::BanByID()
{
	BannedID Banned(m_vGlobalID, m_bGlobalIDSize, GetName());

	m_pParent->BanID(Banned);
}

void CXServerSlot::BanByIP()
{
  unsigned int dwIP = GetIServerSlot()->GetClientIP();

	m_pParent->BanIP(dwIP);
}

BYTE CXServerSlot::GetID()
{
	return m_pISSlot->GetID(); 
}

bool CXServerSlot::IsXServerSlotGarbage()
{ 
	return m_bXServerSlotGarbage; 
}

bool CXServerSlot::IsLocalHost()
{ 
	return m_bLocalHost; 
}

unsigned int CXServerSlot::GetPing()
{ 
	return m_pISSlot->GetPing(); 
}


CXServer *CXServerSlot::GetServer() 
{ 
	return m_pParent; 
}


void CXServerSlot::OnRemoveEntity(IEntity *pEntity)
{
	if(!m_bCanSpawn)
	{
		NET_TRACE("<<NET>>CXServerSlot::OnRemoveEntity[%s] [%d] NOT READY TO SPAWN!",GetName() ,pEntity->GetId());
		return;
	}
	if(m_Snapshot.RemoveEntity(pEntity))
	{
		CStream stm;
		stm.Write(pEntity->GetId());
		SendReliableMsg(XSERVERMSG_REMOVEENTITY,stm,false,pEntity->GetName());
		NET_TRACE("<<NET>> CXServerSlot::OnRemoveEntity[%s] [%d]",GetName(),pEntity->GetId());
	}
	else
	{
		NET_TRACE("<<NET>> CXServerSlot::OnRemoveEntity[%s] [%d] ##SKIPPED## (WAS NOT IN THE SNAPSHOT)",GetName(),pEntity->GetId());
	}		
}

//////////////////////////////////////////////////////////////////////////
void CXServerSlot::ConvertToValidPlayerName( const char *szName, char* outName, size_t sizeOfOutName )
{
	assert(szName);
	assert(sizeOfOutName);

	outName[0]=0;

	int len = strlen(szName);

	int iVisibleCharacters=0;
	int i=0;
	bool bail( false );
	for (; (i < len) && (i < sizeOfOutName-1) && (iVisibleCharacters<20) && !bail ; i++)
	{
		switch(szName[i])
		{
		case '%':
		case '@':
		case '#':
			outName[i] = '_';
			iVisibleCharacters++;
			break;

		case '$':		// color encoding
			if(szName[i+1]>='0' && szName[i+1]<='9')
			{
				if( i+1 < sizeOfOutName-1 )
				{
					outName[i] = szName[i];i++;
					outName[i] = szName[i];
				}
				else
					bail = true;
			}
			break;

		default:
			outName[i] = szName[i];
			iVisibleCharacters++;
		}
	}

	outName[i] = 0;		// 0 termination
}

///////////////////////////////////////////////
void CXServerSlot::OnContextReady(CStream &stm)
{
	m_pLog->Log("CXServerSlot::OnContextReady");

	m_bContextIsReady=true;

	string sNewPlayerName;

	stm.Read(m_bLocalHost);								//
	stm.Read(sNewPlayerName);						// client requested player name
	stm.Read(m_strPlayerModel);						// client requested player model
	stm.Read(m_strClientColor);						// client requested player color in non team base multiplayer mods
	stm.Read(m_ClientRequestedClassId);		//


	char sTemp[65];
	//CXServerSlot::ConvertToValidPlayerName(sNewPlayerName.c_str(),sTemp);
	CXServerSlot::ConvertToValidPlayerName(sNewPlayerName.c_str(),sTemp,sizeof(sTemp));

	m_strPlayerName=sTemp;

//	m_pLog->Log("RECEIVE p_color=%s",m_strClientColor.c_str());

	ValidateName();

	if(m_pParent->m_bIsLoadingLevel)
	{
		m_bWaitingForContextReady=true;
		return;
	}

	stm.Reset();

	FinishOnContextReady();
}

///////////////////////////////////////////////
void CXServerSlot::FinishOnContextReady()
{
	CPlayerSystem *pPlayerSystem = m_pParent->m_pGame->GetPlayerSystem();

	IEntityItPtr pEntities=m_pParent->m_pISystem->GetEntities();
	CStream relstm;
	m_pParent->m_pISystem->WriteTeams(relstm);
	SendReliableMsg(XSERVERMSG_TEAMS,relstm,false);
	m_bCanSpawn=true;	



	// send all entities to the client in 2 passes
	//
	// first pass ensures all entities are created (only the ones that are not in the map after loading)
	// second pass updates the properties of all entities
	IEntity *pEnt=NULL;

	pEntities->MoveFirst();
	while(pEnt=pEntities->Next())
	{
		NET_TRACE("<<NET>>ENTITY NAME %s ENTITY NET PRESENCE %d",pEnt->GetName(),pEnt->GetNetPresence() );
		if(pEnt->IsGarbage())
			continue;

		if(m_pParent->m_pISystem->IsLevelEntity(pEnt->GetId()))
			continue;

		pEnt->GetEntityDesc(m_ed);
//		m_pLog->LogToFile(">> Send Entity A %d - %s (%f %f %f)", m_ed.id, m_ed.name.c_str(),m_ed.pos.x,m_ed.pos.y,m_ed.pos.z);
//		NET_TRACE("<<NET>>SENDING entity id=%08d class=%03d NetPresence=%s name=%s",m_ed.id,(int)m_ed.ClassId,m_ed.netPresence?"true":"false",m_ed.name.c_str());
		OnSpawnEntity(m_ed,pEnt,true);
	}

	//send all other entities
	pEnt=NULL;
	pEntities->MoveFirst();
	while(pEnt=pEntities->Next())
	{
		if(pEnt->IsGarbage())
			continue;
		NET_TRACE("<<NET>>ENTITY CLASS %s NAME %s ENTITY NET PRESENCE %d",pEnt->GetEntityClassName(),pEnt->GetName(),pEnt->GetNetPresence() );
		pEnt->GetEntityDesc(m_ed);

		OnSpawnEntity(m_ed,pEnt,!m_pParent->m_pISystem->IsLevelEntity(pEnt->GetId()));
		
//		m_pLog->LogToFile(">> Send Entity B %d - %s (%f %f %f)", m_ed.id, m_ed.name.c_str(),m_ed.pos.x,m_ed.pos.y,m_ed.pos.z);
//		NET_TRACE("<<NET>>SENDING entity id=%08d class=%03d NetPresence=%s name=%s",m_ed.id,(int)m_ed.ClassId,m_ed.netPresence?"true":"false",m_ed.name.c_str());
	}




	//check if the level is loaded from file
	if(m_bLocalHost)
	{
		if(m_pParent->m_pGame->IsLoadingLevelFromFile())
			return;
	}
	// create the player and register it
	int nClassID=m_pParent->GetRules()->OnClientConnect(m_ScriptObjectServerSlot.GetScriptObject(),m_ClientRequestedClassId);
	
	//reset prediction stuff
	m_iLastCommandServerPhysTime = 0;
	m_iLastCommandClientPhysTime = 0;
	m_iClientWorldPhysTimeDelta = 0;
	m_idClientVehicle = 0;
	m_fClientVehicleSimTime = -1.0f;
	NET_TRACE("<<NET>>END CXServerSlot::OnContextReady");
}

///////////////////////////////////////////////
void CXServerSlot::OnData(CStream &stm)
{
	if(stm.GetReadPos()!=0) 
	{
		CryError( "<CryGame> (CXServerSlot::OnData) Stream read position is zero" );
		return;
	}
	m_PlayerProcessingCmd.Reset();
	ParseIncomingStream(stm);
}

///////////////////////////////////////////////
void CXServerSlot::SendReliable(CStream &stm,bool bSecondaryChannel)
{
	assert(m_pParent);
	m_pParent->m_NetStats.AddPacket(XSERVERMSG_UNIDENTIFIED,stm.GetSize(),true);

	m_pISSlot->SendReliable(stm,bSecondaryChannel);
}

///////////////////////////////////////////////
void CXServerSlot::SendUnreliable(CStream &stm)
{
	assert(m_pParent);
	m_pParent->m_NetStats.AddPacket(XSERVERMSG_UNIDENTIFIED,stm.GetSize(),false);

	m_pISSlot->SendUnreliable(stm);
}

void CXServerSlot::SendText(const char *sText,float fLifeTime)
{
	TextMessage tm;
	tm.cMessageType=CMD_SAY;
	tm.uiSender=0;
	tm.fLifeTime=fLifeTime;
//	tm.stmPayload.Write(sText);
	tm.m_sText=sText;
	SendTextMessage(tm,true);
	//m_pISSlot->SendUnreliable(stm);
}

void CXServerSlot::SendCommand(const char *sCmd)
{
	CStream stm;
	stm.Write(sCmd);
	stm.Write(false);		// no extra
	SendReliableMsg(XSERVERMSG_CMD,stm,false);
}

void CXServerSlot::SendCommand(const char *sCmd, const Vec3 &_invPos, const Vec3 &_invNormal, 
	const EntityId inId, const unsigned char incUserByte )
{
	Vec3 invPos=_invPos;
	Vec3 invNormal=_invNormal;
	CStream stm;
	stm.Write(sCmd);
	stm.Write(true);		// extra

	{
		bool bPos = invPos!=Vec3(0,0,0);

		stm.Write(bPos);

		if(bPos)
			stm.WritePkd(CStreamData_WorldPos(invPos));
	}

	{
		bool bNormal = invNormal!=Vec3(0,0,0);

		stm.Write(bNormal);

		if(bNormal)
			stm.WritePkd(CStreamData_Normal(invNormal));
	}

	stm.WritePkd(inId);
	stm.WritePkd(incUserByte);

	SendReliableMsg(XSERVERMSG_CMD,stm,false);
}


///////////////////////////////////////////////
size_t CXServerSlot::SendReliableMsg( XSERVERMSG msg, CStream &stm,bool bSecondaryChannel, const char *inszName )
{
	assert(m_pParent);

	CStream istm;
	istm.WritePkd(msg);
	istm.Write(stm);
	m_pISSlot->SendReliable(istm,bSecondaryChannel);

	m_pParent->m_NetStats.AddPacket(msg,istm.GetSize(),true);

#ifdef NET_PACKET_LOGGING
	// debugging
	{
		FILE *out=fopen("c:/temp/ServerOutPackets.txt","a");

		if(out)
		{
			size_t size=istm.GetSize();
			BYTE *p=istm.GetPtr();

			fprintf(out,"Rel   Ptr:%p Bits:%4d %s %s ",this,(int)size,CXServer::GetMsgName(msg),inszName);

			for(DWORD i=0;i<(size+7)/8;i++)
			{
				int iH=p[i]>>4;
				int iL=p[i]&0xf;

				char cH = iH<10?iH+'0':iH-10+'A';
				char cL = iL<10?iL+'0':iL-10+'A';

				fputc(cH,out);
				fputc(cL,out);
			}
		
			fprintf(out,"\n");

			fclose(out);	
		}
	}
#endif

	return istm.GetSize();
}

///////////////////////////////////////////////
size_t CXServerSlot::SendUnreliableMsg(XSERVERMSG msg, CStream &stm, const char *inszName, const bool bWithSize )
{
	assert(m_pParent);

	CStream istm;
	istm.WritePkd(msg);

	if(bWithSize)
		istm.WritePkd((short)stm.GetSize());				// sub packet size (without packet id and size itself)

	istm.Write(stm);
	m_pISSlot->SendUnreliable(istm);

	m_pParent->m_NetStats.AddPacket(msg,istm.GetSize(),false);

#ifdef NET_PACKET_LOGGING
	// debugging
	{
		FILE *out=fopen("c:/temp/ServerOutPackets.txt","a");

		if(out)
		{
			size_t size=istm.GetSize();
			BYTE *p=istm.GetPtr();

			fprintf(out,"Unrel Ptr:%p Bits:%4d %s %s ",this,(int)size,CXServer::GetMsgName(msg),inszName);

			for(DWORD i=0;i<(size+7)/8;i++)
			{
				int iH=p[i]>>4;
				int iL=p[i]&0xf;

				char cH = iH<10?iH+'0':iH-10+'A';
				char cL = iL<10?iL+'0':iL-10+'A';

				fputc(cH,out);
				fputc(cL,out);
			}
		
			fprintf(out,"\n");

			fclose(out);	
		}
	}
#endif

	return istm.GetSize();
}

///////////////////////////////////////////////
bool CXServerSlot::IsReady()
{
	return m_pISSlot->IsReady();
}

///////////////////////////////////////////////
void CXServerSlot::Update(bool send_snap, bool send_events)
{
	if(m_pTimer->GetCurrTime()-m_fLastClientStringTime>1)
	{
		m_sClientString="";
	}

	if(m_bWaitingForContextReady && !m_pParent->m_bIsLoadingLevel)
	{
		FinishOnContextReady();
		m_bWaitingForContextReady=false;
	}

	

	if(m_pISSlot->IsReady())
	{
		if (send_snap)
			m_Snapshot.BuildAndSendSnapshot();

		if (send_events)
		{
			CStream stm;
			m_pParent->m_pGame->WriteScheduledEvents(stm, m_iLastEventSent, GetClientTimeDelta());
			SendReliableMsg(XSERVERMSG_EVENTSCHEDULE, stm, true);
		}
	}

	m_fClientVehicleSimTime -= m_pTimer->GetFrameTime();
}

class CCVarSerialize : public ICVarDumpSink
{
private:
	CStream *m_pStm;
public:
	CCVarSerialize(CStream *pStm)
	{
		m_pStm=pStm;
	}
	void OnElementFound(ICVar *pCVar)
	{
		m_pStm->Write(pCVar->GetName());
		m_pStm->Write(pCVar->GetString());
	}
};

///////////////////////////////////////////////
void CXServerSlot::ContextSetup()
{
	CStream stm;
	SXGameContext ctx;
	IConsole *pCon=m_pParent->m_pGame->GetSystem()->GetIConsole();
	m_pParent->GetContext(ctx);
	
	m_pLog->Log("ContextSetup strMapFolder=%s",ctx.strMapFolder.c_str());		// debug

	ctx.Write(stm);
	CCVarSerialize t(&stm);
	pCon->DumpCVars(&t,VF_REQUIRE_NET_SYNC);

	m_pISSlot->ContextSetup(stm);
}

///////////////////////////////////////////////
void CXServerSlot::SetPlayerID(EntityId idPlayer)
{
  if(idPlayer==0)
  {
    m_wPlayerId=idPlayer;
    return;
  }

  IEntity *pPlayer = m_pParent->m_pISystem->GetEntity(idPlayer);
  ASSERT(pPlayer);
  if(pPlayer)
  {
    m_wPlayerId=idPlayer;
    m_ClassId = pPlayer->GetClassId();

    Vec3 ang=pPlayer->GetAngles();
    CStream outstm;
		WRITE_COOKIE(outstm);
    outstm.Write(pPlayer->GetId());
    outstm.Write(ang);
		WRITE_COOKIE(outstm);
    NET_TRACE("<<NET>>Set player sent angles [%f,%f,%f]",ang.x,ang.y,ang.z);
    SendReliableMsg(XSERVERMSG_SETPLAYER,outstm,false,pPlayer->GetName());
  }
}

///////////////////////////////////////////////
EntityId CXServerSlot::GetPlayerId() const
{
	return m_wPlayerId;
}

//////////////////////////////////////////////
const char *CXServerSlot::GetName()
{
	return m_strPlayerName.c_str();
}

const char *CXServerSlot::GetModel()
{
	return m_strPlayerModel.c_str();
}

const char *CXServerSlot::GetColor()
{
	return m_strClientColor.c_str();
}

//////////////////////////////////////////////
bool CXServerSlot::ParseIncomingStream(CStream &stm)
{
	bool bRet = false;
	
	XCLIENTMSG lastMsg=XCLIENTMSG_UNKNOWN;

	do
	{
		XCLIENTMSG Msg;

		if(!stm.Read(Msg))
			return false;
		
		switch(Msg)
		{
			case XCLIENTMSG_CMD:
				OnClientMsgCmd(stm);
				break;
			case XCLIENTMSG_PLAYERPROCESSINGCMD:
				{
					short size;

					if(!stm.ReadPkd(size))			// sub packet size
						return false;

					size_t readpos=stm.GetReadPos();

					OnClientMsgPlayerProcessingCmd(stm);

//					assert(stm.GetReadPos()==readpos+size);		// just for testing

					stm.Seek(readpos+size);
				}
				break;
			case XCLIENTMSG_TEXT:
				m_pParent->OnClientMsgText(m_wPlayerId,stm);
				break;
			case XCLIENTMSG_JOINTEAMREQUEST:
				OnClientMsgJoinTeamRequest(stm);
				break;
			case XCLIENTMSG_CALLVOTE:
		    OnClientMsgCallVote(stm);
		    break;
			case XCLIENTMSG_VOTE:
		    OnClientMsgVote(stm);
		    break;
			case XCLIENTMSG_KILL:
		    OnClientMsgKill(stm);
		    break;
			case XCLIENTMSG_NAME:
				OnClientMsgName(stm);
				break;
			case XCLIENTMSG_RATE:
				OnClientMsgRate(stm);
				break;
			case XCLIENTMSG_ENTSOFFSYNC:
				OnClientOffSyncEntityList(stm);
				break;
			case XCLIENTMSG_RETURNSCRIPTHASH:
				OnClientReturnScriptHash(stm);
				break;
			case XCLIENTMSG_AISTATE:
				OnClientMsgAIState(stm);
				break;

			default:				
			case XCLIENTMSG_UNKNOWN:
				m_pLog->LogError("SS lastSuccessfulPacketID=%i currentPacketID=%i - wrong data chunk.", lastMsg, (int)Msg);
				break;
		}

		lastMsg=Msg;

	} while(!stm.EOS());
	
	return bRet;
}

//////////////////////////////////////////////
void CXServerSlot::OnClientMsgCallVote(CStream &stm)
{
    string command, arg1;
    stm.Read(command);
    stm.Read(arg1);
    m_pParent->m_ServerRules.CallVote(m_ScriptObjectServerSlot, (char *)command.c_str(), (char *)arg1.c_str());
};

//////////////////////////////////////////////
void CXServerSlot::OnClientMsgVote(CStream &stm)
{
    int vote;
    stm.Read(vote);
    m_pParent->m_ServerRules.Vote(m_ScriptObjectServerSlot, vote);
};

//////////////////////////////////////////////
void CXServerSlot::OnClientMsgKill(CStream &stm)
{
    m_pParent->m_ServerRules.Kill(m_ScriptObjectServerSlot);    
};

void CXServerSlot::OnClientMsgRate(CStream &stm)
{
	unsigned char cVar;

	stm.Read(cVar);

	switch(cVar)
	{
		case 0:
			{
				unsigned int dwBitsPerSecond;

				stm.Read(dwBitsPerSecond);

				m_Snapshot.SetClientBitsPerSecond(dwBitsPerSecond);
			}
			break;

		case 1:
			{
				unsigned int dwUpdatesPerSec;
				stm.Read(dwUpdatesPerSec);

				m_Snapshot.SetSendPerSecond(dwUpdatesPerSec);
			}
			break;

		default:
			assert(0);
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CXServerSlot::OnClientMsgName(CStream &stm)
{
	string sName;
	stm.Read(sName);

	m_pLog->Log("Receive SetName '%s'",sName.c_str());

	IEntity *pThis=m_pParent->m_pISystem->GetEntity(GetPlayerId());
	if(pThis)
	{
		string sOldName = m_strPlayerName;

		m_strPlayerName=sName;
		ValidateName();						// validate the name
		sName = m_strPlayerName;	// read it back

		if (sName == sOldName)
		{
			return;
		}

		//SET THE ENTITY NAME
		CStream nstm;
		WRITE_COOKIE(nstm);
		nstm.Write(pThis->GetId());
		nstm.Write(sName);
		WRITE_COOKIE(nstm);
		m_pParent->BroadcastReliable(XSERVERMSG_SETENTITYNAME,nstm,true);
		
		if (m_pParent->m_pGame->IsMultiplayer())
		{
			string sTemp;
			sTemp+=pThis->GetName();
			sTemp+=" @PlayerRenamed ";
			sTemp+=sName;
			m_pParent->BroadcastText(sTemp.c_str());
			pThis->SetName(sName.c_str());
		}
	}

	NET_TRACE("<<NET>>CXServerSlot::OnClientMsgName(%s)\n",sName.c_str());
}

//////////////////////////////////////////////////////////////////////////
void CXServerSlot::OnClientMsgAIState(CStream &stm)
{
	int nDummy;
	stm.Read(nDummy);

	IEntity *pThis=m_pParent->m_pISystem->GetEntity(GetPlayerId());
	if (pThis)
	{		

	}	
}


//////////////////////////////////////////////
bool CXServerSlot::IsContextReady()
{
	return m_bContextIsReady;
}

//////////////////////////////////////////////
void CXServerSlot::SetGameState(int state, int time)
{
	CStream stm;
	m_nState = state;
	stm.Write((char)state);
	stm.Write((short)time);
	
	SendReliableMsg(XSERVERMSG_SETGAMESTATE, stm,false);	
};

//////////////////////////////////////////////
void CXServerSlot::SendScoreBoard()
{
	// don't send anything if we are loading
	if (m_pParent->m_pGame->IsLoadingLevelFromFile())
	{
		return;
	}

	float fTime = GetISystem()->GetITimer()->GetCurrTime();

	// check for timer reset
	if (fTime < m_fLastScoreBoardTime)
	{
		m_fLastScoreBoardTime = 0.0f;
	}

	if (fTime - m_fLastScoreBoardTime <= 0.200f) // send scoreboard every 200ms only
	{
		return;
	}

	m_fLastScoreBoardTime = fTime;

	IScriptSystem *pScriptSystem = GetISystem()->GetIScriptSystem();
	IScriptObject *pGameRules = m_pParent->m_ServerRules.GetScriptObject();

	// prepare the streams
	CStream stmScoreBoard;
	CScriptObjectStream stmScript;

	stmScoreBoard.Reset();
	stmScript.Create(pScriptSystem);
	stmScript.Attach(&stmScoreBoard);

	CXServer::XSlotMap &Slots = m_pParent->GetSlotsMap();

	for (CXServer::XSlotMap::iterator it = Slots.begin(); it != Slots.end(); ++it)
	{
		CXServerSlot *Slot = it->second;

		if (Slot->GetPlayerId() != INVALID_WID)
		{
			stmScoreBoard.Write((bool)1);

			pScriptSystem->BeginCall("GameRules", "GetPlayerScoreInfo");
			pScriptSystem->PushFuncParam(pGameRules);
			pScriptSystem->PushFuncParam(Slot->GetScriptObject());
			pScriptSystem->PushFuncParam(stmScript.GetScriptObject());
			pScriptSystem->EndCall();
		}
	}

	stmScoreBoard.Write((bool)0);

	SendUnreliableMsg(XSERVERMSG_SCOREBOARD, stmScoreBoard,"",true);		// true=send with size

	if ((stmScoreBoard.GetSize()>>3) >= 768)
	{
		GetISystem()->GetILog()->Log("sent scoreboard to %s at %gsecs with %d bytes", GetName(), fTime, stmScoreBoard.GetSize()>>3);
	}
};

//////////////////////////////////////////////
void CXServerSlot::ValidateName()
{
	if (m_strPlayerName.empty())
	{
		m_strPlayerName = "Jack Carver";
	}

	CXServer::XSlotMap &slots = m_pParent->GetSlotsMap();

	for (int i = 0; i < (int)m_strPlayerName.size(); i++)
	{
		switch(m_strPlayerName[i])
		{
		case '@':
		case '%':
		case '\"':
		case '\'':
			m_strPlayerName[i] = '_';
		}
	}
	
	for(CXServer::XSlotMap::iterator i = slots.begin(); i != slots.end(); ++i)
	{
		CXServerSlot *slot = i->second;

		if(slot!=this && strcmp(slot->GetName(), GetName())==0)
		{
		    m_strPlayerName += "_"; // better renaming scheme?
		    ValidateName(); // keep renaming until no more clashes

		    return;
		}
	}
}


///////////////////////////////////////////////
bool CXServerSlot::OccupyLazyChannel()
{
	if(m_bClientLazyChannelState!=m_bServerLazyChannelState)
		return false;

	// todo: remove
//	GetISystem()->GetILog()->Log("OccupyLazyChannel %s -> %s",
//		m_bServerLazyChannelState?"true":"false", !m_bServerLazyChannelState?"true":"false");			// debug

	m_bServerLazyChannelState=!m_bServerLazyChannelState;
	m_dwUpdatesSinceLastLazySend=0;		// 0=it wasn't set at all

	return true;
}

///////////////////////////////////////////////
bool CXServerSlot::GetServerLazyChannelState()
{
	return m_bServerLazyChannelState;
}

///////////////////////////////////////////////
bool CXServerSlot::ShouldSendOverLazyChannel()
{
	m_dwUpdatesSinceLastLazySend++;

	// if we should send it the first time or it's time resend it
	bool bRet = m_dwUpdatesSinceLastLazySend==1 || m_dwUpdatesSinceLastLazySend>20;

	if(bRet)
		m_dwUpdatesSinceLastLazySend=1;		// 1=resend again

	return bRet;
}


///////////////////////////////////////////////
void CXServerSlot::OnClientMsgPlayerProcessingCmd(CStream &stm)
{
	IBitStream *pBitStream = m_pParent->m_pGame->GetIBitStream();

	stm.Read(m_bClientLazyChannelState);

	// sync random seed (from the client)
	{
		CPlayer *pPlayer=0;

		if(m_wPlayerId!=INVALID_WID) 
		if(IEntity *pSlotPlayerEntity = m_pParent->m_pISystem->GetEntity(m_wPlayerId))
			pSlotPlayerEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer);

		bool bSyncSeed;

		stm.Read(bSyncSeed);

		if(bSyncSeed)
		{
			uint8 ucStartRandomSeed;

			stm.Read(ucStartRandomSeed);

			if(pPlayer)
				pPlayer->m_SynchedRandomSeed.EnableSyncRandomSeedS(ucStartRandomSeed);
//			GetISystem()->GetILog()->Log(">> ClientCmd Read %d %d",(int)m_wPlayerId,(int)ucStartRandomSeed);			// debug
		}
		else
		{
			if(pPlayer)
				pPlayer->m_SynchedRandomSeed.DisableSyncRandomSeedS();
//			GetISystem()->GetILog()->Log(">> ClientCmd Read %d off",(int)m_wPlayerId);			// debug
		}
	}

	m_PlayerProcessingCmd.Read(stm,pBitStream);

	//<<FIXME>> implement
	//if no player is assigned to this serverslot I ignore this chunk
	if(m_wPlayerId == INVALID_WID) 
		return;

	IEntity *pSlotPlayerEntity = m_pParent->m_pISystem->GetEntity(m_wPlayerId);
	if(!pSlotPlayerEntity)
	{
		m_pLog->LogToConsole("m_wPlayerId=%d entity not found",m_wPlayerId);
		return;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PREDICTION STUFF
	IPhysicalEntity *pPhysEnt=(IPhysicalEntity *)pSlotPlayerEntity->GetPhysics();
	if(pPhysEnt)
	{
		//SERVER
		float fServerDelta=0.0f,fPing=m_pISSlot->GetPing()*0.001f,timeGran = m_pPhysicalWorld->GetPhysVars()->timeGranularity;
		int iPing = m_pParent->m_pGame->QuantizeTime(fPing);
		int iSrvTime = m_pPhysicalWorld->GetiPhysicsTime();
		fPing = iPing*timeGran;

		if (iPing>0) 
		{
			float *pfSlices,fSliceTotal=0,fClientDelta;
			int nSlices = m_PlayerProcessingCmd.GetTimeSlices(pfSlices);
			for(int i=0;i<nSlices;i++)
				fSliceTotal += pfSlices[i];
			fClientDelta = (m_PlayerProcessingCmd.GetPhysicalTime()-m_iLastCommandClientPhysTime)*timeGran;
			if (m_iClientWorldPhysTimeDelta && fClientDelta>-2.0f && fClientDelta<0)
				return; // ignore older client commands that arrived out of order
			// if some previous client command(s) didn't arrive, insert a time slice corresponding to its duration
			if (fClientDelta<2.0f && (fClientDelta-fSliceTotal)>0.00001 && fClientDelta>fSliceTotal)
				m_PlayerProcessingCmd.InsertTimeSlice(fClientDelta-fSliceTotal,0);
		}

		if(m_iLastCommandServerPhysTime!=0)
			fServerDelta=(iSrvTime-m_iLastCommandServerPhysTime)*timeGran;
		m_PlayerProcessingCmd.SetPhysDelta(fServerDelta);
		m_iLastCommandClientPhysTime = m_PlayerProcessingCmd.GetPhysicalTime();

		float fCurTime=m_pTimer->GetAsyncCurTime(), fTimeDiff, fTimeThresh;
		fTimeDiff = fabsf(timeGran*(m_iLastCommandClientPhysTime+iPing-m_iClientWorldPhysTimeDelta)-fCurTime);
		fTimeThresh = max(fPing,m_pTimer->GetFrameTime())*1.4f;
		if (fTimeDiff > fTimeThresh)
			m_nDesyncFrames++;
		else
			m_nDesyncFrames = 0;
		if (m_nDesyncFrames>4 || fTimeDiff>fTimeThresh*8 || !m_iClientWorldPhysTimeDelta)
		{
			int iNewTimeDelta = m_iLastCommandClientPhysTime+iPing-m_pParent->m_pGame->QuantizeTime(fCurTime);
			iNewTimeDelta = m_pParent->m_pGame->SnapTime(iNewTimeDelta);
			if (iPing>0 && fabs_tpl((iNewTimeDelta-m_iClientWorldPhysTimeDelta)*timeGran)<3.0f)
				m_pLog->LogToConsole("Adjusting client clock by %+.4f, new delta %.4f (ping %.3f)", 
					(iNewTimeDelta-m_iClientWorldPhysTimeDelta)*timeGran, iNewTimeDelta*timeGran, fPing);
			m_iClientWorldPhysTimeDelta = iNewTimeDelta;
		}
		//
		//m_iLastCommandServerPhysTime = m_pPhysicalWorld->GetiPhysicsTime()-iPing;
		m_iLastCommandServerPhysTime = min(iSrvTime,m_iLastCommandClientPhysTime-m_iClientWorldPhysTimeDelta);

		if (fPing>0)
		{
			fPing = (iSrvTime-m_iLastCommandServerPhysTime)*timeGran;
			m_PlayerProcessingCmd.AddTimeSlice(&fPing);
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_idClientVehicle = 0;
	bool bHasVehicle;
	stm.Read(bHasVehicle);
	if (bHasVehicle)
	{
		CPlayer *pPlayer=0;
		CVehicle *pVehicle=0;
		short nSize;
		long nPos;
		pe_status_vehicle sv;
		
		EntityId id;

		pBitStream->ReadBitStream(stm,id,eEntityId);
//		stm.Read(id);

		IEntity *pEnt = m_pParent->m_pISystem->GetEntity(id);
		stm.Read(nSize);
		nPos = stm.GetReadPos()+nSize;

		if (!m_bLocalHost &&
				pEnt && pEnt->GetContainer() && 
				pEnt->GetContainer()->QueryContainerInterface(CIT_IVEHICLE, (void**)&pVehicle) &&
				pSlotPlayerEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer) &&
				pVehicle==pPlayer->GetVehicle() &&
				pEnt->GetPhysics()->GetStatus(&sv) && sv.nActiveColliders==0)
		{
			pEnt->Read(stm);

			pe_params_flags pf;
			pf.flagsOR = pef_update;
			pEnt->GetPhysics()->SetParams(&pf);
			m_pParent->m_pGame->AdvanceReceivedEntities(m_iLastCommandServerPhysTime);
			pf.flagsOR = 0;
			pf.flagsAND = ~pef_update;
			pEnt->GetPhysics()->SetParams(&pf);

			m_idClientVehicle = id;
			m_fClientVehicleSimTime = 2.0f;
		}
		stm.Seek(nPos);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Vec3 posClient; bool bClientPos,bInRange;
	stm.Read(bClientPos);
	if (bClientPos)
	{
		stm.Read(bInRange);
		if (bInRange)
		{
			unsigned int num;
			stm.ReadNumberInBits(num,16); posClient.x = num*(1.0f/16);
			stm.ReadNumberInBits(num,16); posClient.y = num*(1.0f/16);
			stm.ReadNumberInBits(num,13); posClient.z = num*(1.0f/16);
		}
		else
			stm.Read(posClient);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//if the player is asking the scoreboard I send it
	if(m_PlayerProcessingCmd.CheckAction(ACTION_SCORE_BOARD) || m_bForceScoreBoard)
	{
		SendScoreBoard();
	}
	else
	{
		m_fLastScoreBoardTime = 0.0f;
	}

	//when the game is in itermission state it will skip all client inputs
	if(m_nState==CGS_INTERMISSION)
	{
	    m_PlayerProcessingCmd.Reset();
	};

	//if the client entity is a CPlayer
	if (m_pParent->m_pGame->GetPlayerSystem()->IsPlayerClass(pSlotPlayerEntity->GetClassId()))
	{
		CPlayer *pPlayer=NULL;

		if (pSlotPlayerEntity->GetContainer() && pSlotPlayerEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer))
		{
			//<<FIXME>> move the respawn time in the serverslot
			if( (!pPlayer->IsAlive()))
			{

				if((m_PlayerProcessingCmd.CheckAction(ACTION_FIRE0)) != 0)
				{
					m_pParent->GetRules()->OnClientRequestRespawn(m_ScriptObjectServerSlot.GetScriptObject(),m_ClassId);
				}
				else
				{
					if (!m_pParent->m_pGame->IsMultiplayer())
					{
						// [marco] for single player, after the fade in is finished,
						// show the last checkpoint menu
						// try to click button always, so it will show the checkpoint
						// menu as soon as the client respawn time is passed
						m_pParent->GetRules()->OnClientRequestRespawn(m_ScriptObjectServerSlot.GetScriptObject(),m_ClassId);	
					}
				}

				return;
			}
			pPlayer->ProcessCmd(m_pISSlot->GetPing(),m_PlayerProcessingCmd);
			// My player entity, process animations in Client.
			if(!m_bLocalHost) //avoid to process the angles(and recoil) twice
				pPlayer->ProcessAngles(m_PlayerProcessingCmd);

			/*if (m_pParent->m_pGame->IsMultiplayer() && m_pParent->m_pGame->UseFixedStep() && pPlayer->GetVehicle())
			{
				m_PlayerProcessingCmd.SetPhysicalTime(m_pParent->m_pGame->SnapTime(m_pPhysicalWorld->GetiPhysicsTime())+m_pParent->GetSchedulingDelay());
				m_pParent->m_pGame->ScheduleEvent(pSlotPlayerEntity, m_PlayerProcessingCmd);
			}*/
		}
	}
	else if (pSlotPlayerEntity->GetClassId()==SPECTATOR_CLASS_ID)
	{
//		assert(!bHasVehicle);

		pe_player_dynamics pd;
		if (!m_bLocalHost && bClientPos)
		{
			pSlotPlayerEntity->SetPos(posClient);
			m_idClientVehicle = m_wPlayerId;
			m_fClientVehicleSimTime = 10.0f;
			pd.bActive = 0;
		}
		else
			pd.bActive = 1;
		if (pPhysEnt)
			pPhysEnt->SetParams(&pd);

		CSpectator *pSpectator=NULL;
		pSlotPlayerEntity->GetContainer()->QueryContainerInterface(CIT_ISPECTATOR,(void**) &pSpectator);
		if(pSpectator)
			pSpectator->ProcessKeys(m_PlayerProcessingCmd);
	}
	else if (pSlotPlayerEntity->GetClassId()==ADVCAMSYSTEM_CLASS_ID)
	{
		CAdvCamSystem *pAdvCamSystem=NULL;
		pSlotPlayerEntity->GetContainer()->QueryContainerInterface(CIT_IADVCAMSYSTEM,(void**) &pAdvCamSystem);
		if(pAdvCamSystem)
			pAdvCamSystem->ProcessKeys(m_PlayerProcessingCmd);
	}
}

bool CXServerSlot::IsClientSideEntity(IEntity *pEnt) 
{ 
	IEntityContainer *pIContainer=pEnt->GetContainer();

	if(!pIContainer)
		return false;

	CVehicle *pVehicle;

	// only vehicles are client side simulated
	if(pIContainer->QueryContainerInterface(CIT_IVEHICLE,(void**)&pVehicle))
		return m_fClientVehicleSimTime>0 && pEnt->GetId()==m_idClientVehicle; 

	return false;
}


//////////////////////////////////////////////
void CXServerSlot::OnClientMsgJoinTeamRequest(CStream &stm)
{
	//check if the team switch is allowed via game rules script
	//if yes,respawn the player/spectator/commander
	BYTE nTeamId;
	string sClass;
	stm.Read(nTeamId);
	stm.Read(sClass);

  if(m_wPlayerId!=INVALID_WID)
	{
		m_pParent->m_ServerRules.OnClientMsgJoinTeamRequest(this,nTeamId,sClass.c_str());
	}

}

void CXServerSlot::OnClientMsgCmd(CStream &stm)
{
	string cmd;
	stm.Read(cmd);
	if(m_wPlayerId!=INVALID_WID)
	{
		m_pParent->m_ServerRules.OnClientCmd(this,cmd.c_str());
	}
}

//////////////////////////////////////////////
void CXServerSlot::OnClientOffSyncEntityList(CStream &stm)
{
	unsigned char nEnts;
	EntityId id;

	stm.Read(nEnts);
	for(;nEnts;nEnts--)
	{
		stm.ReadPkd(id);
		MarkEntityOffSync(id);
	}
}

//////////////////////////////////////////////
// XCLIENTMSG_RETURNSCRIPTHASH
void CXServerSlot::OnClientReturnScriptHash(CStream &stm)
{
	IBitStream *pBitStream = m_pParent->m_pGame->GetIBitStream();

	u32 dwHash;

	pBitStream->ReadBitStream(stm,dwHash,eDoNotCompress);			// returned hash

	// todo

	// debug
//	m_pLog->Log("OnClientReturnScriptHash %p",dwHash);
}

void CXServerSlot::MarkEntityOffSync(EntityId id) 
{ 
	IEntity *pEnt = m_pParent->m_pGame->m_pSystem->GetIEntitySystem()->GetEntity(id);
	if (!pEnt)
		return;

	Vec3 vBBox[2],sz;
	m_mapOffSyncEnts.insert(std::pair<EntityId,int>(id,0)); 
	pEnt->GetBBox(vBBox[0],vBBox[1]);
	sz = vBBox[1]-vBBox[0];

	pe_params_foreign_data pfd;
	IPhysicalEntity **ppEnts;
	int i = m_pPhysicalWorld->GetEntitiesInBox(vBBox[0]-sz*0.3f,vBBox[1]+sz*0.3f,ppEnts,ent_sleeping_rigid|ent_rigid)-1;
	for(; i>=0; i--)
	{
		pEnt = (IEntity*)ppEnts[i]->GetForeignData();
		if (pEnt && pEnt->GetNetPresence())
			m_mapOffSyncEnts.insert(std::pair<EntityId,int>(pEnt->GetId(),0)); 
	}
}


//////////////////////////////////////////////
void CXServerSlot::GetBandwidthStats( SServerSlotBandwidthStats &out )
{
	if(m_pISSlot)
		m_pISSlot->GetBandwidthStats(out);
	 else 
		out.Reset();
}


//////////////////////////////////////////////
void CXServerSlot::ResetBandwidthStats()
{
	if(m_pISSlot)
		m_pISSlot->ResetBandwidthStats();
}

//////////////////////////////////////////////
void CXServerSlot::SendTextMessage(TextMessage &tm,bool bSecondaryChannel)
{
	CStream stm;
	tm.Write(stm);
	SendReliableMsg(XSERVERMSG_TEXT,stm,bSecondaryChannel);
}

void CXServerSlot::ClientString(const char *s)
{
	m_fLastClientStringTime=m_pTimer->GetCurrTime();
	m_sClientString=s;
}

void CXServerSlot::GetNetStats(SlotNetStats &ss)
{
	ss.ping=m_pISSlot->GetPing()*2;
	ss.packetslost=m_pISSlot->GetPacketsLostCount();
	ss.upacketslost=m_pISSlot->GetUnreliablePacketsLostCount();
	ss.name=m_strPlayerName;
	ss.lastsnapshotbitsize=m_Snapshot.m_nLastSnapshotBitSize;
	ss.maxsnapshotbitsize=m_Snapshot.m_nMaxSnapshotBitSize;
}


