///////////////////////////////////////////////////3///////////////////
//
//  Game Source Code
//
//  File: XClient.cpp
//  Description: XClient implemetation.
//
//  History:
//  - August 3, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XClient.h"
#include "XPlayer.h"

#include "UIHud.h"
#include "XSystemClient.h"
#include "XSystemDummy.h"
#include "IngameDialog.h"

#include <I3dengine.h>

#include <IEntitySystem.h>
#include <IMovieSystem.h>
#include <ISound.h>
#include <IAISystem.h>

#include "XPlayer.h"
#include "Spectator.h"									// CSpectator
#include "AdvCamSystem.h"								// CAdvCamSystem
#include "XVehicle.h"
#include "PlayerSystem.h"
#include "XVehicleSystem.h"

#include "ScriptObjectVector.h"
#include "ScriptObjectPlayer.h"
//#include "ScriptObjectEntity.h"
#include "ScriptObjectVehicle.h"
#include "ScriptObjectSpectator.h"			// CScriptObjectSpectator
#include "ScriptObjectAdvCamSystem.h"		// CScriptObjectAdvCamSystem
#include "StreamData.h"									// CStreamData_WorldPos
#include <map>													// STL map<>

#include "Game.h"



//////////////////////////////////////////////////////////////////////////////////////////////

void CXClient::OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity )
{
	m_pISystem->OnSpawnContainer(ed,pEntity);
}

void CXClient::OnSpawn(IEntity *ent, CEntityDesc &ed) 
{
	m_pISystem->OnSpawn(ent,ed);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CXClient::OnRemove(IEntity *ent)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
CXClient::CXClient() 
{
	m_bConnected=0;
	m_CameraParams=0;
	m_pScriptObjectClient=0;
	m_pIActionMapManager=0;
	m_pEntitySystem=0;
	m_pISystem=0;
	m_pIClient=0;
	m_pISystem=0;
	m_wPlayerID=INVALID_WID;
	m_pGame=0;

	cl_explShakeDCoef = 0.07f;
	cl_explShakeAmplH = 0.001;
	cl_explShakeAmplV = 0.001;
	cl_explShakeFreq = 11.73f;
	cl_explShakeTime = 1.73f;

	m_fFrontSound=0;
	m_fBackSound=0;
	m_fLeftSound=0;
	m_fRightSound=0;
	m_pClientStuff=0;
	bDoSwitch=false;
	m_pTimer=0;
	m_bSelfDestruct=false;			//  to make sure the client is only released in one place
	m_pSavedConsoleVars=0;
	m_bLazyChannelState=false;	// start with false on client and serverslot side
}

bool CXClient::Init(CXGame *pGame,bool bLocal) 
{
	m_fLastClientStringTime=0;
	m_bDisplayHud=true;
	m_bMapConnecting=false;
	m_bRecordingDemo=false;
	m_bPlaybackDemo=false;
//	m_iaLastSeaponSwitch = 0;
	m_wPlayerID = INVALID_WID;
	m_pISystem = NULL;
	m_bLocalHost = false;	
	m_bLinkListenerToCamera =true;
	m_pGame = pGame;
	m_pScriptSystem = m_pGame->GetScriptSystem();
	m_pEntitySystem = m_pGame->GetSystem()->GetIEntitySystem();
	m_pLog=m_pGame->m_pLog;

	m_sopMsgNormal.Create( m_pScriptSystem );
	m_sopMsgPos.Create( m_pScriptSystem );

	m_pTimer	=m_pGame->GetSystem()->GetITimer();
	// Create the client
	m_pIClient = m_pGame->CreateClient(this,bLocal);

	m_PrevPlayerProcessingCmd = m_PlayerProcessingCmd;

	// Set the first System interface.
	UpdateISystem();

	m_pIActionMapManager = m_pGame->GetActionMapManager();
	if(m_pIActionMapManager)
		m_pIActionMapManager->SetSink(this);

	// Create the console variables
	CreateConsoleVariables();

	m_nGameState = CGS_INTERMISSION;   // until we get first update from the mod		
	m_nGameLastTime = 0;	
	m_fGameLastTimeReceived = 0;
	m_pScriptObjectClient=new CScriptObjectClient;
	m_pScriptObjectClient->Create(pGame->GetScriptSystem(),pGame,this);

	m_CameraParams = new SCameraParams;
	m_pClientStuff=m_pScriptSystem->CreateEmptyObject();
	m_iPhysicalWorldTime = 0;
	m_bIgnoreSnapshot = false;

	return true;
}

///////////////////////////////////////////////
CXClient::~CXClient()
{
	assert(m_bSelfDestruct);		//  to make sure the client is only released in one place

	if (m_pSavedConsoleVars)
	{
		RestoreServerSyncedVars();	// restore VF_REQUIRE_NET_SYNC marked console vars)
	}

	// delete the player
	if (m_pISystem)
		m_pISystem->RemoveEntity(m_wPlayerID);
	m_wPlayerID = INVALID_WID;

	// Release the System interface
	SAFE_RELEASE(m_pISystem); 
	SAFE_DELETE(m_CameraParams);
	SAFE_DELETE(m_pScriptObjectClient);

	if(m_pIActionMapManager)
		m_pIActionMapManager->SetSink(NULL);

	if (m_pEntitySystem)
		m_pEntitySystem->RemoveSink(this);
	// Call disconnect to be sure that it has been done
	//Disconnect("@ClientHasQuit");
	
	SAFE_RELEASE(m_pIClient);
/*
	if (cl_explShakeDCoef)
		cl_explShakeDCoef->Release();
	if (cl_explShakeAmplH)
		cl_explShakeAmplH->Release();
	if (cl_explShakeAmplV)
		cl_explShakeAmplV->Release();
	if (cl_explShakeFreq)
		cl_explShakeFreq->Release();
	if (cl_explShakeTime)
		cl_explShakeTime->Release();
*/
	m_pGame = NULL;

	m_fFrontSound=0;
	m_fBackSound=0;
	m_fLeftSound=0;
	m_fRightSound=0;

	SAFE_RELEASE(m_pClientStuff);

	if(m_pScriptSystem)
	{
		m_pScriptSystem->SetGlobalToNull("ClientStuff");
		m_pScriptSystem->SetGlobalToNull("Client");

		//Never force Lua GC, m_pScriptSystem->ForceGarbageCollection();
	}
}

void CXClient::LoadPlayerDesc()
{
	IScriptSystem *pSS = m_pGame->GetScriptSystem();
	
	pSS->ExecuteFile("playercfg.lua",false);
}

///////////////////////////////////////////////
bool CXClient::CreateConsoleVariables()
{
	IConsole *pConsole=m_pGame->GetSystem()->GetIConsole();
	cl_runroll = pConsole->CreateVariable("cl_runroll","0",0,
		"\n"
		"Usage: cl_runroll ?\n"
		"Default is 0.");
	cl_runpitch = pConsole->CreateVariable("cl_runpitch","0.4",0,
		"\n"
		"Usage: cl_runpitch 0.4\n"
		"Default is 0.4.");
	cl_runheight = pConsole->CreateVariable("cl_runheight","0.03",0,
		"\n"
		"Usage: cl_runheight 0.03\n"
		"Default is 0.03.");
	cl_runheightspeed = pConsole->CreateVariable("cl_runheightspeed","1.5",0,
		"\n"
		"Usage: cl_runheightspeed 1.5\n"
		"Default is 1.5.");
	cl_playerclassid = pConsole->CreateVariable("cl_playerclassid","1",0,
		"Sets the player class.\n"
		"Usage: cl_playerclassid #\n"
		"Default is 1.");
	cl_netstats = pConsole->CreateVariable("cl_netstats","0",0,
		"Toggles client network statistics.\n"
		"Usage: cl_netstats [0/1]\n"
		"Default is 0 (off). Set to 1 to display network statistics.");
	cl_cmdrate = pConsole->CreateVariable("cl_cmdrate","40",0,
		"Specify the max client network command rate\n"
		"(less is better for bandwidth, more is better for response,\n"
		"the actual rate is limited by frame rate as well)\n"
		"Usage: cl_cmdrate [5..100]\n"
		"Default is 40");
/*
	cl_explShakeDCoef = pConsole->CreateVariable("cl_ExplShakeD",".07",0,
		"Sets the damping co-efficient of the explosion shake effect.\n"
		"Usage: cl_ExplShakeD .07\n"
		"Default is 0.07. Higher values suppress the effect more quickly.");
	cl_explShakeAmplH = pConsole->CreateVariable("cl_ExplShakeAmplH",".001",0,
		"Sets the horizontal amplitude of the explosion shake effect.\n"
		"Usage: cl_ExplShakeAmplH .001\n"
		"Default is 0.001 metres. The view is horizontally displaced\n"
		"by this amount when the player is near an explosion.");
	cl_explShakeAmplV = pConsole->CreateVariable("cl_ExplShakeAmplV",".001",0,
		"Sets the vertical amplitude of the explosion shake effect.\n"
		"Usage: cl_ExplShakeAmplV .001\n"
		"Default is 0.001 metres. The view is vertically displaced\n"
		"by this amount when the player is near an explosion.");
	cl_explShakeFreq = pConsole->CreateVariable("cl_ExplShakeFreq","11.73",0,
		"Sets the frequency of the explosion shake effect.\n"
		"Usage: cl_ExplShakeFreq 11.73\n"
		"When the player is near an explosion, the view\n"
		"shakes with a frequency of 11.73 Hz by default.");
	cl_explShakeTime = pConsole->CreateVariable("cl_ExplShakeTime","1.73",0,
		"Sets the duration of the explosion shake effect.\n"
		"Usage: cl_ExplShakeTime 1.73\n"
		"Default is 1.73 seconds. When the player is near an explosion,\n"
		"the view shakes for this time.");
*/
	cl_sound_detection_max_distance = pConsole->CreateVariable("cl_sound_detection_max_distance","50",0);
	cl_sound_detection_min_distance = pConsole->CreateVariable("cl_sound_detection_min_distance","2",0);
	cl_sound_event_radius = pConsole->CreateVariable("cl_sound_event_radius","50",0);
	cl_sound_event_timeout = pConsole->CreateVariable("cl_sound_event_timeout","1",0);
	
	pConsole->AddCommand("say","Client:Say(%line)",VF_NOHELP,"");
	pConsole->AddCommand("sayteam","Client:SayTeam(%line)",VF_NOHELP,"");
	pConsole->AddCommand("sayone","Client:SayOne(%%)",VF_NOHELP, "");
	pConsole->AddCommand("tell","Client:SayOne(%%)",VF_NOHELP, "");
	pConsole->AddCommand("team","Client:JoinTeamRequest(%%)",0,
		"Sends a request to join a team.\n"
		"Usage: team teamname\n");
	pConsole->AddCommand("ready","Client:CallVote(\"ready\")",0,
		"Asks if other players are ready.\n"
		"Usage: ready\n"
		"Works by sending a request to players.\n"
		"Players respond y or n.\n");
	pConsole->AddCommand("callvote","Client:CallVote(%%)",0,
		"Asks players to vote on a command.\n"
		"Usage: callvote commandname arg\n"
		"Sends a request to players to vote on 'commandname arg'.\n"
		"Players respond y or n.\n");
	pConsole->AddCommand("vote","Client:Vote(%1)",0,
		"Used to vote on suggestions from other players.\n"
		"Usage: vote [y/n]\n"
		"Vote y for yes or n for no.");
	pConsole->AddCommand("name","Client:SetName(%line)",VF_NOHELP, "");
	pConsole->AddCommand("kill","Client:Kill()",0,
		"Kills the player.\n"
		"Usage: kill\n"
		"Player respawns as normal.");
	pConsole->AddCommand("cl_maxrate","Client:SetBitsPerSecond(%1)",0,
		"Sets client maximum download bandwidth\n"
		"(the actual rate is limited by server setting as well)\n"
		"Usage: cl_maxrate 28800\n"
		"Sets bits per second the server is allowed to send to you (this client).");
	pConsole->AddCommand("cl_updaterate","Client:SetUpdateRate(%1)",0,
		"Specify the max server network update rate\n"
		"(less is better for bandwidth, more is better for response,\n"
		"the actual rate is limited by frame/update rate and the server setting as well)\n"
		"Usage: cl_updaterate [5..100]\n"
		"Default is 20");
	return true;
}


///////////////////////////////////////////////
void CXClient::OnXConnect()
{
	//sound sources
	m_fFrontSound=0;
	m_fBackSound=0;
	m_fLeftSound=0;
	m_fRightSound=0;
	m_lstSounds.clear();
	m_nDiscardedPackets=0;
	m_fLastRemoteAsyncCurrTime=0;
	m_fLastScoreBoardTime = 0;
	TRACE("CXClient::OnXConnect");	
	LoadPlayerDesc();

/*	if (bDoSwitch)
	{
		// TODO
		// Check if this works
		m_pGame->GetSystem()->GetIConsole()->SetScrollMax(600);
		m_pGame->GetSystem()->GetIConsole()->ShowConsole(true);
		//m_pGame->SendMessage("Switch");
		bDoSwitch=false;
	}
*/
}

///////////////////////////////////////////////
void CXClient::MarkForDestruct()
{
//	m_pGame->GetSystem()->GetILog()->Log("CXClient MarkForDestruct");
	m_bSelfDestruct=true;
}

///////////////////////////////////////////////
bool CXClient::DestructIfMarked()
{
	if(m_bSelfDestruct)
	{
//		m_pGame->GetSystem()->GetILog()->Log("CXClient DestructIfMarked true");

		delete this;
		return true;			// was deleted
	}
	return false;		// was not deleted
}


///////////////////////////////////////////////
void CXClient::RestoreServerSyncedVars()
{
	if(m_pSavedConsoleVars)
	{
		string varname,val;

		if(m_bLocalHost)			// only client have to restore their VF_REQUIRE_NET_SYNC console variables
		{
			while(!m_pSavedConsoleVars->EOS() && m_pSavedConsoleVars->Read(varname))
			{
				m_pSavedConsoleVars->Read(val);

	//			m_pGame->GetSystem()->GetILog()->Log("Restored console variable %s to %s",varname.c_str(),val.c_str());

				m_pISystem->SetVariable(varname.c_str(),val.c_str());
			}
		}

		delete m_pSavedConsoleVars;m_pSavedConsoleVars=0;
	}
}

///////////////////////////////////////////////
void CXClient::OnXClientDisconnect(const char *szCause)
{
	_SmartScriptObject pClientStuff(m_pScriptSystem,true);
	if(m_pScriptSystem->GetGlobalValue("ClientStuff",pClientStuff))				// call ClientStuff:OnShutdown()
	{
		m_pScriptSystem->BeginCall("ClientStuff","OnShutdown");
		m_pScriptSystem->PushFuncParam(pClientStuff);
		m_pScriptSystem->EndCall();
	}

	TRACE(szCause);
	SetPlayerID(0);
	// <<FIXME>> Should cleanup the stuff with a new function of the IXSystem interface

	m_bConnected=0;

	if(m_pISystem)
		m_pISystem->Disconnected(szCause);

	m_pScriptSystem->BeginCall("ClientOnDisconnect");
	m_pScriptSystem->PushFuncParam(szCause);
	m_pScriptSystem->EndCall();

	m_pGame->MarkClientForDestruct();		// to make sure the client is only released in one place

	// hide console and reset progress bar after a disconnection
	GetISystem()->GetIConsole()->ResetProgressBar(0);
	GetISystem()->GetIConsole()->SetScrollMax(600/2);
	GetISystem()->GetIConsole()->ShowConsole(0);
}


///////////////////////////////////////////////
void CXClient::Reset()
{
	m_pScriptSystem->GetGlobalValue("ClientStuff",m_pClientStuff);
	m_pScriptSystem->BeginCall("ClientStuff","OnReset");
	m_pScriptSystem->PushFuncParam(m_pClientStuff);
	m_pScriptSystem->EndCall();
}

///////////////////////////////////////////////
void CXClient::OnXContextSetup(CStream &stm)
{
	GetISystem()->GetILog()->Log("CXClient::OnXContextSetup");

	SetPlayerID(INVALID_WID);
	UpdateISystem();

	m_GameContext.Read(stm);		// Read the sended game context

	if (m_pGame->IsMultiplayer())
	{
		if(m_GameContext.bInternetServer)
			if(!GetISystem()->GetINetwork()->VerifyMultiplayerOverInternet())
				return;
	}
	
	if (!m_pGame->m_bEditor)
	{
		HSCRIPTFUNCTION pfnOnConnectEstablished = m_pScriptSystem->GetFunctionPtr("Game", "OnConnectEstablished");

		if (pfnOnConnectEstablished)
		{
			m_pScriptSystem->BeginCall(pfnOnConnectEstablished);
			m_pScriptSystem->PushFuncParam(m_pGame->GetScriptObject());
			m_pScriptSystem->EndCall();

			m_pScriptSystem->ReleaseFunc(pfnOnConnectEstablished);
		}
	}

	IGameMods *pModInterface=m_pGame->GetModsInterface();

	assert(pModInterface);		// otherwise the Game::Init failed

	//if(m_GameContext.strMod!=string(pModInterface->GetCurrentMod()))
	if (stricmp(m_GameContext.strMod.c_str(),pModInterface->GetCurrentMod())!=0)
	{
		m_pLog->LogError("Wrong Mod: CurrentMod='%s' RequestedMod='%s'",pModInterface->GetCurrentMod(),m_GameContext.strMod.c_str());

		XDisconnect("@GameVersionError");
		return;
	}

	if(!m_GameContext.IsVersionOk())
	{
		m_pLog->LogError("CXClient::OnXContextSetup - Versions do not match.  Server version: %i.%d  Client version: %i.%d",
			m_GameContext.dwNetworkVersion,(int)m_GameContext.ucServerInfoVersion,NETWORK_FORMAT_VERSION,(int)SERVERINFO_FORMAT_VERSION);

		XDisconnect("@GameVersionError");
		return;
	}

	{
		IConsole *pConsole=m_pGame->GetSystem()->GetIConsole();
		static CDefaultStreamAllocator sa;

		RestoreServerSyncedVars();	// restore VF_REQUIRE_NET_SYNC marked console vars

		if(!stm.EOS())
		{
			m_pSavedConsoleVars = new CStream(1024, &sa); // saved console variable state (to restore the VF_REQUIRE_NET_SYNC marked vars)

			string varname,val;
			while(!stm.EOS() && stm.Read(varname))
			{
				stm.Read(val);

				ICVar *pVar=pConsole->GetCVar(varname.c_str());

				if(pVar)
				{
					m_pSavedConsoleVars->Write(varname.c_str());
					m_pSavedConsoleVars->Write(pVar->GetString());
				}

	//			m_pGame->GetSystem()->GetILog()->Log("Got Server synced console variable %s to %s (was %s)",varname.c_str(),val.c_str(),pVar->GetString());

				m_pISystem->SetVariable(varname.c_str(),val.c_str());
			}
		}
	}

	m_pGame->g_GameType->Set(m_GameContext.strGameType.c_str());
	m_Snapshot.Reset();

	m_pLog->Log("CXClient::OnXContextSetup - map : %s\n", m_GameContext.strMapFolder.c_str());
	
	if(!m_pISystem->LoadLevel(m_GameContext.strMapFolder.c_str(), m_GameContext.strMission.c_str(), false))
	{
		m_pLog->LogError("CXClient::OnXContextSetup ERROR LOADING LEVEL: %s\n", m_GameContext.strMapFolder.c_str());
		
		LoadingError("@LoadLevelError");

		return;
	}

	if((!m_pGame->m_bEditor) 
		&& (!m_pGame->IsServer()) 
		&& (m_pISystem->GetLevelDataCheckSum()!=m_GameContext.wLevelDataCheckSum))
	{
		m_pLog->LogError("CXClient::OnXContextSetup ERROR LOADING LEVEL: %s [INVALID CHECKSUM]\n", m_GameContext.strMapFolder.c_str());

		LoadingError("@LevelVersionError");

		return;
	}
	

	_SmartScriptObject pClientStuff(m_pScriptSystem,true);
	if(m_pScriptSystem->GetGlobalValue("ClientStuff",pClientStuff))				// call ClientStuff:OnShutdown()
	{
		m_pScriptSystem->BeginCall("ClientStuff","OnShutdown");
		m_pScriptSystem->PushFuncParam(pClientStuff);
		m_pScriptSystem->EndCall();
	}

	if(!m_pGame->m_bDedicatedServer)								// don't load ClientStuff on dedicated server
	{
		if(!m_pGame->ExecuteScript("scripts/$GT$/ClientStuff.lua",true))
		{
			DebugBreak();	
		}
		m_pScriptSystem->GetGlobalValue("ClientStuff",m_pClientStuff);
		m_pScriptSystem->BeginCall("ClientStuff","OnInit");
		m_pScriptSystem->PushFuncParam(m_pClientStuff);
		m_pScriptSystem->EndCall();
	}
	
	// Write the stream to send to ContextReady
	{
//		m_pLog->Log("Write the stream to send to ContextReady");
		stm.Reset();

		// bLocalHost
		stm.Write(m_bLocalHost);

		// p_name
		m_pLog->Log("p_name=%s",m_pGame->p_name->GetString());
		stm.Write(m_pGame->p_name->GetString());

		// p_model, mp_model
		{
			ICVar *model;

			if (m_pGame->IsMultiplayer())
				model = m_pGame->mp_model;		// multiplayer model
			else
				model = m_pGame->p_model;			// single player model

			if(!model->GetString())
				stm.Write("");
			else
			{
				m_pLog->Log("p_model=%s",model->GetString());
				stm.Write(model->GetString());
			}	
		}

		// p_color
		{
			ICVar *color = m_pGame->p_color;			// player's color in non team base multiplayer mods

			if(!color->GetString())
				stm.Write("");
			else
			{
				m_pLog->Log("p_color=%s",color->GetString());
				stm.Write(color->GetString());
			}	
		}

		// player classid
		stm.Write(cl_playerclassid->GetIVal());
		
		m_pLog->Log("SEND ContextReady");
		m_pIClient->ContextReady(stm);
	}

	// fade in when loading a new map
	if (!m_pGame->m_bEditor)
	{	
		m_pGame->m_p3DEngine->SetScreenFx("ScreenFade",1);
		float fFadeTime=-2.5f;
		m_pGame->m_p3DEngine->SetScreenFxParam("ScreenFade","ScreenFadeTime", &fFadeTime);
		float fPreFade=5.0f;
		m_pGame->m_p3DEngine->SetScreenFxParam("ScreenFade","ScreenPreFadeTime", &fPreFade);
	}

	if(!m_pGame->m_bEditor)
	{
		m_pGame->m_pSystem->SetIProcess(m_pGame->m_p3DEngine);
		m_pGame->m_pSystem->GetIProcess()->SetFlags(PROC_3DENGINE);
	}

	m_pGame->m_pUIHud->Reset();
	
	// We have to tell Ubisoft that the client has successfully connected
	// If ubisoft is not running this won't do anything.
	GetISystem()->GetINetwork()->Client_ReJoinGameServer();

	m_bConnected = 1;

	m_pGame->GetSystem()->SetForceNonDevMode(m_GameContext.bForceNonDevMode);

	// clean up all the sounds that might have been started before the first
	// frame to avoid problems with sloppy/bogus vis areas
	// this calls RecomputeSoundOcclusion

	if(m_pGame->GetSystem()->GetISoundSystem())
		m_pGame->GetSystem()->GetISoundSystem()->Silence();
	if(m_pGame->m_pSystem->GetIMusicSystem())
		m_pGame->m_pSystem->GetIMusicSystem()->Silence();

	if (!m_pGame->m_bIsLoadingLevelFromFile)
	{
		if (m_pGame->IsMultiplayer())
		{
			m_pGame->GotoMenu(true);
		}
		else
		{
			m_pGame->GotoGame(true);
		}
	}

	if (!m_pGame->m_bIsLoadingLevelFromFile)
	{
		//		m_pLog->Log("HIDE CONSOLE");
		m_pGame->GetSystem()->GetIConsole()->ResetProgressBar(0);
		m_pGame->m_pSystem->GetIConsole()->ShowConsole(false);
		m_pGame->m_pSystem->GetIConsole()->SetScrollMax(600/2);

		//if (m_pGame->IsMultiplayer())
		m_pGame->GetSystem()->GetIRenderer()->ClearColorBuffer(Vec3(0,0,0));
	}
}

///////////////////////////////////////////////
void CXClient::UpdateClientNetwork()
{
	assert(this);
	if(m_bPlaybackDemo)
	{
		m_pGame->PlaybackChunk();
	}
	else
	{
		if(m_pIClient)
		{
			bool bThisExists=m_pIClient->Update(GetCurrentTime());// this pointer might be destroyed after this call

			if(!bThisExists)
				return;
		}
	}

	assert(m_pTimer);

	m_NetStats.Update(m_pTimer->GetCurrTimePrecise());		// keep statistics for one sec
}

///////////////////////////////////////////////
void CXClient::OnXData(CStream &stm)
{
	if(stm.GetReadPos()!=0)
	{
		CryError( "<CryGame> (CXClient::OnXData) Stream read position is zero" );
	}
	if(m_bRecordingDemo){
		m_pGame->AddDemoChunk(stm);
	}
	// this is an incoming message for the client - it should not be processed on a server
	ParseIncomingStream(stm);
}

//------------------------------------------------------------------------------------------------- 
void CXClient::OnXServerTimeout()
{
	m_pScriptSystem->BeginCall("ClientOnServerTimeout");
	m_pScriptSystem->EndCall();
}

//------------------------------------------------------------------------------------------------- 
void CXClient::OnXServerRessurect()
{
	m_pScriptSystem->BeginCall("ClientOnServerRessurect");
	m_pScriptSystem->EndCall();
}



///////////////////////////////////////////////
void CXClient::XConnect( const char *szAddr, bool _bDoLateSwitch, const bool inbCDAuthorization )
{
	m_pIClient->SetServerIP(szAddr);
	bDoSwitch=_bDoLateSwitch;

	m_pIClient->InitiateCDKeyAuthorization(inbCDAuthorization);

	m_pGame->m_szLastAddress = szAddr;
	m_pGame->m_bLastDoLateSwitch = _bDoLateSwitch;
	m_pGame->m_bLastCDAuthentication = inbCDAuthorization;
}



///////////////////////////////////////////////
void CXClient::DemoConnect()
{
	m_bPlaybackDemo = true;
};

///////////////////////////////////////////////
void CXClient::XDisconnect(const char *szCause)
{
	if (m_pIClient)
		m_pIClient->Disconnect(szCause);
}

void CXClient::UpdateSound( const float fFrameTime )
{
	if(m_fFrontSound>0)
	{
		m_fFrontSound-=fFrameTime;
		if(m_fFrontSound<0)
			m_fFrontSound=0;
	}
	if(m_fBackSound>0)
	{
		m_fBackSound-=fFrameTime;
		if(m_fBackSound<0)
			m_fBackSound=0;
	}
	if(m_fLeftSound>0)
	{
		m_fLeftSound-=fFrameTime;
		if(m_fLeftSound<0)
			m_fLeftSound=0;
	}
	if(m_fRightSound>0)
	{
		m_fRightSound-=fFrameTime;
		if(m_fRightSound<0)
			m_fRightSound=0;
	}
	// remove old sounds from list
	for (TSoundListIt It=m_lstSounds.begin();It!=m_lstSounds.end();)
	{
		SSoundInfo &SoundInfo=(*It);
		SoundInfo.fTimeout-=fFrameTime;
		if (SoundInfo.fTimeout<=0.0f)
		{
			It=m_lstSounds.erase(It);
		}
		else
		{
			++It;
		}
	}
}


///////////////////////////////////////////////
void CXClient::Update()
{
	CPlayer *pPlayer=NULL;
	CSpectator *pSpectator=NULL;
	CAdvCamSystem *pAdvCamSystem=NULL;

	ITimer *pTimer=m_pGame->m_pSystem->GetITimer();
	float time = pTimer->GetCurrTime();
	float fFrameTime=pTimer->GetFrameTime();

	if(time-m_fLastClientStringTime>1)
		m_sClientString="";

	UpdateSound(fFrameTime);	

	IEntity *en=NULL;
	IEntityContainer *pCnt=NULL;

	if(m_wPlayerID != INVALID_WID)
	{
		en = m_pISystem->GetEntity(m_wPlayerID);

		if(en && (pCnt=en->GetContainer()))
		{
			if(pCnt->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayer))
			{
				pPlayer->m_stats.concentration=false;
			}
////////SPECTATOR CAMERA STUFF
			if(pCnt->QueryContainerInterface(CIT_ISPECTATOR,(void **) &pSpectator))
			{
				EntityId idHost=pSpectator->GetHostId();

				IEntity *pHost= m_pISystem->GetEntity(idHost);
				if(pHost)
				{
					IEntityContainer *pHostCnt=pHost->GetContainer();

					if(pHostCnt)
					{
						CPlayer *pPlayerHost;

						if(pHostCnt->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayerHost))
						{
							pPlayerHost->SetViewMode(true);
							pPlayerHost->UpdateCamera();
							en=pHost;
						}
					}
				}
			}

		}

		if((!m_pGame->m_pSystem->GetIConsole()->IsOpened()) && (!m_pGame->m_bMenuOverlay) && m_pIActionMapManager)
			m_pIActionMapManager->Update((unsigned int)(time*1000.f));

		if(en==NULL)
			return;
	}

	//ASSIGN THE CAMERA
	IEntityCamera *pEntCam=NULL;
	if (m_CameraParams->nCameraId)
	{
		IEntity *pEnt=m_pEntitySystem->GetEntity(m_CameraParams->nCameraId);
		if (pEnt)
		{
			pEntCam=pEnt->GetCamera();
			if (!pEntCam)
			{
				pEntCam=m_pGame->GetSystem()->GetIEntitySystem()->CreateEntityCamera();
				pEnt->SetCamera(pEntCam);
				pEntCam->GetCamera().Init(m_pGame->GetSystem()->GetIRenderer()->GetWidth(), m_pGame->GetSystem()->GetIRenderer()->GetHeight());
			}
			pEntCam->SetPos(pEnt->GetPos());
			pEntCam->SetAngles(pEnt->GetAngles());
			pEntCam->SetFov(m_CameraParams->fFOV, m_pGame->GetSystem()->GetIRenderer()->GetWidth(), m_pGame->GetSystem()->GetIRenderer()->GetHeight());
			pEntCam->Update();
		}
		else
		{
			GameWarning( "Camera entity with Id %d not found",m_CameraParams->nCameraId );
		}
	}

	if (!pEntCam)
	{
		if(en)
			pEntCam=en->GetCamera();
	}
		
	{
		bool bTimeToSend=(m_Snapshot.IsTimeToSend(fFrameTime) && m_pIClient->IsReady());

		SendInputToServer(bTimeToSend);
	}

	if((m_wPlayerID != INVALID_WID))
	{
		if (m_nGameState != CGS_INTERMISSION)
		{
			if(m_fLastScoreBoardTime+0.5f<time)
			{
				m_fLastScoreBoardTime = time+10000;
				m_pScriptSystem->BeginCall("ClientStuff", "ShowScoreBoard");
				m_pScriptSystem->PushFuncParam(m_pClientStuff);
				m_pScriptSystem->PushFuncParam(0);
				m_pScriptSystem->EndCall();
			};
		}
	}

	if (m_pGame->UseFixedStep() && !m_lstUpdatedEntities.empty())
	{
		// send the list of off-sync entities			
		CStream stm;
		unsigned char nEnts = min(255,m_lstUpdatedEntities.size());
		stm.Write(nEnts);
		for(int i=0;i<nEnts;i++)
			stm.WritePkd(m_lstUpdatedEntities[i]->GetId());
		m_lstUpdatedEntities.clear();

		SendUnreliableMsg(XCLIENTMSG_ENTSOFFSYNC,stm);
	}
	
	if (pEntCam)
	{
		CCamera cam=pEntCam->GetCamera();
		if(pPlayer)
			cam.SetAngle(cam.GetAngles()+pPlayer->m_vShake);
			//pEntCam->GetCamera().GetAngles()+m_vSh
		m_pGame->m_pSystem->SetViewCamera(cam);
		if(m_bLinkListenerToCamera && m_pGame->m_pSystem->GetISoundSystem())
			m_pGame->m_pSystem->GetISoundSystem()->SetListener(cam,Vec3(0,0,0));
	}

	if (m_wPlayerID != INVALID_WID)
	{
		m_pScriptSystem->BeginCall("ClientStuff","OnUpdate");
		m_pScriptSystem->PushFuncParam(m_pClientStuff);
		m_pScriptSystem->EndCall();
	}

	
	// adjust commands per second (less is better for bandwidth, more is better for response)
	{
		int iCmdRate=cl_cmdrate->GetIVal();

		iCmdRate=CLAMP(iCmdRate,5,100);

		bool bInDrivingAVehicle=false;	// reduce the command rate when in vehicles - to reduce upstream

		if(pPlayer)
			if(pPlayer->m_stats.inVehicleState==CPlayer::PVS_DRIVER)
				bInDrivingAVehicle=true;

		if(bInDrivingAVehicle && m_pGame->IsMultiplayer())	// only in MP we need the bandwidth
			iCmdRate=(iCmdRate+1)/2;

		if(m_Snapshot.GetSendPerSecond()!=iCmdRate)
			m_Snapshot.SetSendPerSecond(iCmdRate);
	}
}

//////////////////////////////////////////////
// XCLIENTMSG_RETURNSCRIPTHASH
void CXClient::SendScriptHashResponse( const unsigned int dwHash )
{
	CStream stm;

	IBitStream *pBitStream = m_pGame->GetIBitStream();

	pBitStream->WriteBitStream(stm,(uint32)dwHash,eDoNotCompress);			// returned hash
	SendReliableMsg(XCLIENTMSG_RETURNSCRIPTHASH,stm);

	// debug
//	m_pLog->Log("SendScriptHashResponse %p",dwHash);
}


void CXClient::SendInputToServer( const bool bTimeToSend )
{
	if(m_wPlayerID==INVALID_WID)
		return;

	IEntity *en = m_pISystem->GetEntity(m_wPlayerID);
	IBitStream *pBitStream=m_pGame->GetIBitStream();		// compressed or uncompressed

	if(!en)
		return;

	IEntityContainer *pCnt=en->GetContainer();

	if(!pCnt)
		return;

	CPlayer *pPlayer=NULL;
	CSpectator *pSpectator=NULL;
	CAdvCamSystem *pAdvCamSystem=NULL;

	CStream &stm = m_Snapshot.GetReliableStream();
	assert(!stm.GetSize());

	//////////////////////////////////////////////////////////////////////////////////////
	//HANDLE A PLAYER CONTAINER
	//////////////////////////////////////////////////////////////////////////////////////
	if(pCnt->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayer))
	{
		// Send the snapshot if it's time to do so

		//when the game is in intermission state it will skip all client inputs
		if(m_nGameState==CGS_INTERMISSION)
			m_PlayerProcessingCmd.Reset();

		// to clamp the angles (up/down)
		pPlayer->ProcessAngles(m_PlayerProcessingCmd);

		bool bSendToServer = false;
		if (m_pGame->IsMultiplayer() || !pPlayer->IsAlive() || m_PlayerProcessingCmd.CheckAction(ACTION_SCORE_BOARD))
		{
			bSendToServer = true;
		}

		//////////////////////////////////////////////////////////////////////////
		if (!bSendToServer)
		{
			// Perform player processing locally.
			pPlayer->ProcessCmd( 0,m_PlayerProcessingCmd );
			m_PrevPlayerProcessingCmd = m_PlayerProcessingCmd;
			m_PlayerProcessingCmd.Reset();
		}
		else if(bTimeToSend)
		{
			CVehicle *pVehicle = pPlayer->GetVehicle();
			pe_status_timeslices stc;
			float slices[32];
			stc.pTimeSlices = slices;
			stc.sz = 32;

			if (en->GetPhysics()) 
				m_PrevPlayerProcessingCmd.AddTimeSlice(slices,en->GetPhysics()->GetStatus(&stc));

			CXEntityProcessingCmd &epc=(m_pGame->IsServer() ? m_PlayerProcessingCmd : m_PrevPlayerProcessingCmd);

			//AIMING STUFF
			if(pPlayer->m_stats.aiming)
				epc.AddAction(ACTION_ZOOM_TOGGLE);
			else
				epc.RemoveAction(ACTION_ZOOM_TOGGLE);

			CPlayer::SWalkParams wp=pPlayer->GetWalkParams();
			epc.SetLeaning(wp.fCurrLean);
			if (pVehicle)
				epc.ResetTimeSlices();

			// write the player processing cmd			
			m_PrevPlayerProcessingCmd.SetPhysicalTime(m_pGame->GetSystem()->GetIPhysicalWorld()->GetiPhysicsTime());
      
			// used for sending ordered reliable data over the unreliable connection (slow but never stalls, used for scoreboard)
			stm.Write(m_bLazyChannelState);

			{
				// sync random seed (to the server)
				bool bSyncToServer=pPlayer->m_SynchedRandomSeed.IsTimeToSyncToServerC();

				stm.Write(bSyncToServer);
				if(bSyncToServer)
				{
					stm.Write(pPlayer->m_SynchedRandomSeed.GetStartRandomSeedC());
//					GetISystem()->GetILog()->Log(">> ClientCmd Write %d %d",(int)pPlayer->GetEntity()->GetId(),(int)pPlayer->m_SynchedRandomSeed.GetStartRandomSeedC());			// debug
				}
			}

			epc.Write(stm,pBitStream,true);
		
			if(pVehicle && pPlayer->m_stats.inVehicleState==CPlayer::PVS_DRIVER && !m_pGame->IsServer())
			{
				stm.Write(true);
				pBitStream->WriteBitStream(stm,pVehicle->GetEntity()->GetId(),eEntityId);
//				stm.Write(pVehicle->GetEntity()->GetId());
				m_stmVehicle.Reset();
				pVehicle->GetEntity()->Write(m_stmVehicle);
				stm.Write((short)m_stmVehicle.GetSize());
				stm.Write(m_stmVehicle);
			}
			else
				stm.Write(false);
			stm.Write(false); // client pos (used for spectators)

			// send the main streams
			SendUnreliableMsg(XCLIENTMSG_PLAYERPROCESSINGCMD,m_Snapshot.GetReliableStream(),true);		// with size

			m_Snapshot.Reset();

			if(pPlayer && !m_pGame->IsServer())
			{
				pPlayer->ProcessMovements(m_PlayerProcessingCmd);

				// special fire processing for the client in MP
				// it will always perform the fire on the local event
				if (m_pGame->IsMultiplayer() && pPlayer->IsMyPlayer())
				{
					CXEntityProcessingCmd tempPC;
					if (m_PlayerProcessingCmd.CheckAction(ACTION_FIRE0))
						tempPC.AddAction(ACTION_FIRE0);
					if (m_PlayerProcessingCmd.CheckAction(ACTION_FIRE_GRENADE))
						tempPC.AddAction(ACTION_FIRE_GRENADE);
					pPlayer->ProcessWeapons(tempPC);
				}
			}

			m_PrevPlayerProcessingCmd = m_PlayerProcessingCmd;

			m_PlayerProcessingCmd.Reset();
		}	

	}
	else if(pCnt->QueryContainerInterface(CIT_ISPECTATOR,(void **) &pSpectator))
	{
		//////////////////////////////////////////////////////////////////////////////////////
		//HANDLE A SPECTATOR
		//////////////////////////////////////////////////////////////////////////////////////
		
		pSpectator->ProcessKeys(m_PlayerProcessingCmd);

		if(bTimeToSend)
		{
			// write the player processing cmd			
			m_PlayerProcessingCmd.SetPhysicalTime(m_pGame->GetSystem()->GetIPhysicalWorld()->GetiPhysicsTime());

			// used for sending ordered reliable data over the unreliable connection (slow but never stalls, used for scoreboard)
			stm.Write(m_bLazyChannelState);

			stm.Write(false);			// random seed (do not sync)

			m_PlayerProcessingCmd.Write(stm,pBitStream,false);

			stm.Write(false);		// no vehicle data
			stm.Write(true);
			Vec3 pos = en->GetPos();
			if (inrange(pos.x,0.0f,4095.0f)&inrange(pos.y,0.0f,4095.f)&inrange(pos.z,0.0f,511.0f))
			{
				stm.Write(true);
				stm.WriteNumberInBits((int)(pos.x*16+0.5f),16);
				stm.WriteNumberInBits((int)(pos.y*16+0.5f),16);
				stm.WriteNumberInBits((int)(pos.z*16+0.5f),13);
			}
			else
			{
				stm.Write(false);
				stm.Write(pos);
			}

			SendUnreliableMsg(XCLIENTMSG_PLAYERPROCESSINGCMD,m_Snapshot.GetReliableStream(),true);		// with size

			m_Snapshot.Reset();
		}
		m_PlayerProcessingCmd.Reset();
	}
	else if(pCnt->QueryContainerInterface(CIT_IADVCAMSYSTEM,(void **) &pAdvCamSystem))
	{
		//////////////////////////////////////////////////////////////////////////////////////
		//HANDLE A AdvancedCameraSystem
		//////////////////////////////////////////////////////////////////////////////////////

		if(bTimeToSend)
		{
			// to clamp the angles (up/down)
			pAdvCamSystem->ProcessKeys(m_PlayerProcessingCmd);

			// used for sending ordered reliable data over the unreliable connection (slow but never stalls, used for scoreboard)
			stm.Write(m_bLazyChannelState);

			stm.Write(false);			// random seed (do not sync)

			// write the player processing cmd			
			m_PlayerProcessingCmd.Write(stm,pBitStream,true);
			stm.Write(false);	// no vehicle data
			stm.Write(false); // client pos (used for spectators)

			SendUnreliableMsg(XCLIENTMSG_PLAYERPROCESSINGCMD,m_Snapshot.GetReliableStream(),true);		// with size

			m_Snapshot.Reset();
		}
		m_PlayerProcessingCmd.Reset();
	}
}

const char *CXClient::GetMsgName( XCLIENTMSG inValue )
{
	switch(inValue)
	{
#define ADDNAME(name) case XCLIENTMSG_##name:	return(#name);
		ADDNAME(UNKNOWN)
		ADDNAME(PLAYERPROCESSINGCMD)
		ADDNAME(TEXT)
		ADDNAME(JOINTEAMREQUEST)
		ADDNAME(CALLVOTE)
		ADDNAME(VOTE)
		ADDNAME(KILL)
		ADDNAME(CMD)
		ADDNAME(RATE)
		ADDNAME(ENTSOFFSYNC)
#undef ADDNAME
		default: assert(0);
	}
	return 0;
}


void CXClient::DrawNetStats()
{
	int iRoundtripInMS = m_pIClient->GetPing()*2;
	float fIncomingKbPerSec,fOutgoingKbPerSec;
	DWORD nIncomingPacketsPerSec,nOutgoingPacketsPerSec;
	m_pIClient->GetBandwidth(fIncomingKbPerSec,fOutgoingKbPerSec,nIncomingPacketsPerSec,nOutgoingPacketsPerSec);
	unsigned int lost = m_pIClient->GetPacketsLostCount();
	unsigned int ulost = m_pIClient->GetUnreliablePacketsLostCount();
	IRenderer *pRen=m_pGame->m_pSystem->GetIRenderer();
	pRen->TextToScreen(10,70,"Roundtrip [%dms] LOST [%d] ULOST [%d] DISCARDED[%d]",iRoundtripInMS,lost,ulost,m_nDiscardedPackets);

	float fPacketSize=(20+8)*8.0f/1024.0f;		// 20bytes IP + 8bytes UDP in kBit

	pRen->TextToScreen(10,75,"BANDWIDTH IN=%.2f/%.2f Kbits/sec (%d) OUT=%.2f/%.2f Kbits/sec (%d)",
		fIncomingKbPerSec, fIncomingKbPerSec+fPacketSize*nIncomingPacketsPerSec, nIncomingPacketsPerSec,
		fOutgoingKbPerSec, fOutgoingKbPerSec+fPacketSize*nOutgoingPacketsPerSec, nOutgoingPacketsPerSec);

#ifndef REDUCED_FOR_PUBLIC_RELEASE
	pRen->TextToScreen(0,78,"Packets produced but might not be sent:");

	static DWORD sResetCount=0;

	bool bNewDataArrived = m_NetStats.GetResetCount()!=sResetCount;

	sResetCount=m_NetStats.GetResetCount();

	for(int i=0;;i++)
	{
		DWORD dwRelCount,dwUnrelCount,dwItem;
		size_t nBitSize;

		if(!m_NetStats.GetStats(i,dwItem,dwRelCount,dwUnrelCount,nBitSize))
			break;

		pRen->TextToScreen(10,78+(i+1)*3.0f,"(R:%d U:%d Bits:%d) %s",dwRelCount,dwUnrelCount,nBitSize,GetMsgName((XSERVERMSG)dwItem));

		if(bNewDataArrived)
			m_pLog->Log("Client (R:%d U:%d Bits:%d) %s",dwRelCount,dwUnrelCount,nBitSize,GetMsgName((XSERVERMSG)dwItem));
	}

	if(bNewDataArrived)
		m_pLog->Log("-------<netstats end>n");
#endif
}

//////////////////////////////////////////////////////////////////////////
void CXClient::OnMapChanged()   
{
	m_bMapConnecting = true;
};

//////////////////////////////////////////////////////////////////////////
void CXClient::OnMapChangedReally()   
{
  m_bMapConnecting = false;

	// [marcio] reseting the movie system here stop any cutscene the begins imediately
	// after the game starts.
	// loading first checkpoint in:
	//			Factory
	//			Carrier
	//
	//m_pGame->GetSystem()->GetIMovieSystem()->Reset();

	// [marco] save the first checkpoint in single player game
	// after the client has been connected, ID set etc. etc.
	// do NOT save if this map was already loaded from a checkpoint saved game!
	if (!m_pGame->IsMultiplayer() && !m_pGame->m_bMapLoadedFromCheckpoint)
	{	
		m_pLog->Log("\001 Saving first checkpoint");
		IAISystem *pAISystem = m_pGame->GetSystem()->GetAISystem();
		if (pAISystem)
		{
			if (pAISystem->GetAutoBalanceInterface())
			{
				pAISystem->GetAutoBalanceInterface()->Checkpoint();
				pAISystem->GetAutoBalanceInterface()->SetAllowedDeathCount(2);
			}
		}

		ITagPoint *pRespawn;
		pRespawn=m_pGame->GetTagPoint("Respawn0");
		if (pRespawn)	
		{
			Vec3 vPos,vAngles;
			pRespawn->GetPos(vPos);
			pRespawn->GetAngles(vAngles);
			char buf[1024];
			sprintf(buf, "checkpoint_%s_%s_1", m_pGame->g_LevelName->GetString(), m_pGame->m_pServer->m_GameContext.strMission.c_str());
			m_pGame->Save( buf, &vPos,&vAngles,true );
		}
		IInput *pInput=m_pGame->GetSystem()->GetIInput();
		if(pInput)
			pInput->GetIKeyboard()->ClearKeyState();
	}

	m_pScriptSystem->BeginCall("ClientStuff", "OnMapChange");
	m_pScriptSystem->PushFuncParam(m_pClientStuff);
	m_pScriptSystem->EndCall();

	if (!m_pGame->IsMultiplayer() && !m_pGame->m_bMapLoadedFromCheckpoint)
	{
		_SmartScriptObject pMissionScript(m_pScriptSystem);
		m_pScriptSystem->GetGlobalValue("Mission", pMissionScript);
 
		if (((IScriptObject *)pMissionScript) != 0)
		{
			HSCRIPTFUNCTION temp=0;
			bool bRes=pMissionScript->GetValue( "OnMapChange",temp );			
			temp=0;
			if (bRes)
			{			
				m_pScriptSystem->BeginCall("Mission", "OnMapChange");
				m_pScriptSystem->PushFuncParam((IScriptObject*)pMissionScript);
				m_pScriptSystem->EndCall();
			}
		}
	}

}

///////////////////////////////////////////////
void CXClient::SendReliable(CStream &stm)
{
	m_NetStats.AddPacket(XCLIENTMSG_UNIDENTIFIED,stm.GetSize(),true);

	m_pIClient->SendReliable(stm);

	
#ifdef NET_PACKET_LOGGING
	// debugging
	{
		FILE *out=fopen("c:/temp/ClientOutPackets.txt","a");

		if(out)
		{
			size_t size=stm.GetSize();
			BYTE *p=stm.GetPtr();

			fprintf(out,"Rel   Ptr:%p Bits:%4d %s %s ",this,(int)size,"UNIDENTIFIED","");

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
}

///////////////////////////////////////////////
void CXClient::SendUnreliable(CStream &stm)
{
	m_NetStats.AddPacket(XCLIENTMSG_UNIDENTIFIED,stm.GetSize(),false);

	m_pIClient->SendUnreliable(stm);


#ifdef NET_PACKET_LOGGING
	// debugging
	{
		FILE *out=fopen("c:/temp/ClientOutPackets.txt","a");

		if(out)
		{
			size_t size=stm.GetSize();
			BYTE *p=stm.GetPtr();

			fprintf(out,"Unrel Ptr:%p Bits:%4d %s %s ",this,(int)size,"UNIDENTIFIED","");

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
}

///////////////////////////////////////////////
size_t CXClient::SendReliableMsg(XCLIENTMSG msg, CStream &stm)
{
	CStream istm;
	istm.Write(msg);
	istm.Write(stm);
	m_pIClient->SendReliable(istm);

	m_NetStats.AddPacket(msg,istm.GetSize(),true);

	
#ifdef NET_PACKET_LOGGING
	// debugging
	{
		FILE *out=fopen("c:/temp/ClientOutPackets.txt","a");

		if(out)
		{
			size_t size=istm.GetSize();
			BYTE *p=istm.GetPtr();

			fprintf(out,"Rel   Ptr:%p Bits:%4d %s %s ",this,(int)size,CXClient::GetMsgName(msg),"");

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
size_t CXClient::SendUnreliableMsg(XCLIENTMSG msg, CStream &stm, const bool bWithSize )
{
	CStream istm;
	istm.Write(msg);

	if(bWithSize)
		istm.WritePkd((short)stm.GetSize());				// sub packet size (without packet id and size itself)

	istm.Write(stm);
	m_pIClient->SendUnreliable(istm);

	m_NetStats.AddPacket(msg,istm.GetSize(),false);


#ifdef NET_PACKET_LOGGING
	// debugging
	{
		FILE *out=fopen("c:/temp/ClientOutPackets.txt","a");

		if(out)
		{
			size_t size=istm.GetSize();
			BYTE *p=istm.GetPtr();

			fprintf(out,"Unrel Ptr:%p Bits:%4d %s %s ",this,(int)size,CXClient::GetMsgName(msg),"");

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
bool CXClient::IsReady()
{
	return m_pIClient->IsReady();
}



EntityId CXClient::GetPlayerId() const
{
	return m_wPlayerID;
}

///////////////////////////////////////////////
void CXClient::SetPlayerID( EntityId wPlayerID )
{
	m_wPlayerID=wPlayerID;
	IEntity *pPlayer = 0;
	
	if(m_pISystem)
		pPlayer=m_pISystem->GetEntity(wPlayerID);

	if(pPlayer)
		m_pScriptSystem->SetGlobalValue("_localplayer",pPlayer->GetScriptObject());
	 else
		m_pScriptSystem->SetGlobalToNull("_localplayer");

	if(pPlayer)
		m_pGame->SetCurrentUI(m_pGame->m_pUIHud);
}

///////////////////////////////////////////////
void CXClient::UpdateISystem()
{
	SAFE_RELEASE(m_pISystem);

	// create the system interface
	if(m_pGame->IsServer())
	{
		m_pISystem = new CXSystemDummy(m_pGame,m_pGame->m_pLog);	// Dummy interface if this game is client/server
		TRACE("DUMMY SYSTEM");
		m_bLocalHost = true;		
	}
	else
	{
		m_pISystem = new CXSystemClient(m_pGame,m_pGame->m_pLog);	// Client interface if this game is only client	
		TRACE("CLIENT SYSTEM");
		m_bLocalHost = false;
		m_pGame->GetSystem()->GetIEntitySystem()->SetSink(this);
	}
}


///////////////////////////////////////////////
void CXClient::TriggerMoveLeft(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_MOVE_LEFT);
}

///////////////////////////////////////////////
void CXClient::TriggerMoveRight(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_MOVE_RIGHT);
}

///////////////////////////////////////////////
void CXClient::TriggerMoveForward(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_MOVE_FORWARD);
}

///////////////////////////////////////////////
void CXClient::TriggerMoveBackward(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_MOVE_BACKWARD);
}

///////////////////////////////////////////////
void CXClient::TriggerJump(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_JUMP);
}

///////////////////////////////////////////////
void CXClient::TriggerMoveMode(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_MOVEMODE);
}

///////////////////////////////////////////////
void CXClient::TriggerMoveModeToggle(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_MOVEMODE_TOGGLE);
}

///////////////////////////////////////////////
void CXClient::TriggerAimToggle(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);

	if (pEntity)
	{
		IEntityContainer *pContainer=pEntity->GetContainer();
		
		if(pContainer)
		{
			CPlayer *pPlayer;
			
			pContainer->QueryContainerInterface(CIT_IPLAYER, (void**) &pPlayer);
			
			if (pPlayer)
				pEntity->SendScriptEvent(ScriptEvent_ZoomToggle, (!pPlayer->m_stats.aiming)?1:2);
		}
	}
}

///////////////////////////////////////////////
// move mode double click
void CXClient::TriggerMoveMode2(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_MOVEMODE2);
}

///////////////////////////////////////////////
void CXClient::TriggerLeanLeft(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_LEANLEFT);
}

///////////////////////////////////////////////
void CXClient::TriggerLeanRight(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_LEANRIGHT);
}

///////////////////////////////////////////////
void CXClient::TriggerHoldBreath(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_HOLDBREATH);
}

///////////////////////////////////////////////
void CXClient::TriggerFireMode(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_FIREMODE);
}

///////////////////////////////////////////////
void CXClient::TriggerFire0(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_FIRE0);
}

///////////////////////////////////////////////
void CXClient::TriggerFireCancel(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_FIRECANCEL);
}

void CXClient::TriggerFireGrenade(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_FIRE_GRENADE);
}

void CXClient::TriggerFlashlight(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_FLASHLIGHT);
}

//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerChangeView(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_CHANGE_VIEW);
}

///////////////////////////////////////////////
void CXClient::TriggerVehicleBoost(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_VEHICLE_BOOST);
}

///////////////////////////////////////////////
void CXClient::TriggerReload(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_RELOAD);
}

///////////////////////////////////////////////
void CXClient::TriggerUse(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_USE);
}
 
///////////////////////////////////////////////
void CXClient::TriggerTurnLR(float fValue,XActivationEvent ae)
{
	float fFovMul = 1.0f;
	IEntity *pPlayerEnt = m_pISystem->GetEntity( m_wPlayerID );
	if (pPlayerEnt)
	{

		IEntityCamera *pCam = pPlayerEnt->GetCamera();
		if (pCam)
		{
			fFovMul = (float) (pCam->GetFov() / 1.5707963267948966192313216916398);
		}
	}
	m_PlayerProcessingCmd.GetDeltaAngles()[ROLL] -= fValue*fFovMul;
	m_PlayerProcessingCmd.AddAction(ACTION_TURNLR);
}

///////////////////////////////////////////////
void CXClient::TriggerTurnUD(float fValue,XActivationEvent ae)
{
	float fFovMul = 1.0f;
	IEntity *pPlayerEnt = m_pISystem->GetEntity( m_wPlayerID );
	if (pPlayerEnt)
	{

		IEntityCamera *pCam = pPlayerEnt->GetCamera();
		if (pCam)
		{
			fFovMul = float(pCam->GetFov() / 1.5707963267948966192313216916398);
		}
		//RESET RECOIL RETURN
		IEntityContainer *pCnt=pPlayerEnt->GetContainer();
		if(pCnt){
			CPlayer *pP=NULL;
			pCnt->QueryContainerInterface(CIT_IPLAYER,(void **)&pP);
			if(pP){
				pP->m_fRecoilXDelta=0;
			}
		}
	}
	m_PlayerProcessingCmd.GetDeltaAngles()[YAW] += fValue*fFovMul;
	m_PlayerProcessingCmd.AddAction(ACTION_TURNUD);
}

///////////////////////////////////////////////
void CXClient::TriggerWalk(float fValue,XActivationEvent ae)
{
		m_PlayerProcessingCmd.AddAction(ACTION_WALK);
}

///////////////////////////////////////////////
void CXClient::TriggerRunSprint(float fValue,XActivationEvent ae)
{
		m_PlayerProcessingCmd.AddAction(ACTION_RUNSPRINT);
}

//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerNextWeapon(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_NEXT_WEAPON);
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerPrevWeapon(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction(ACTION_PREV_WEAPON);
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon0(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_0 );
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon1(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_1 );
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon2(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_2 );
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon3(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_3 );
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon4(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_4 );
}
//////////////////////////////////////////////////////////////////////////
/*void CXClient::TriggerWeapon5(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_5 );
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon6(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_6 );
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon7(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_7 );	
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerWeapon8(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_WEAPON_8 );
}
*/
//////////////////////////////////////////////////////////////////////////
void CXClient::CycleGrenade(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_CYCLE_GRENADE );
}
//////////////////////////////////////////////////////////////////////////
void CXClient::TriggerDropWeapon(float fValue,XActivationEvent ae)
{
	m_PlayerProcessingCmd.AddAction( ACTION_DROPWEAPON );
}


void CXClient::TriggerItem0(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(pEntity)
		pEntity->SendScriptEvent(ScriptEvent_ItemActivated, 0);
}

void CXClient::TriggerItem1(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(pEntity)
		pEntity->SendScriptEvent(ScriptEvent_ItemActivated, 1);
}

void CXClient::TriggerItem2(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(pEntity)
		pEntity->SendScriptEvent(ScriptEvent_ItemActivated, 2);
}

void CXClient::TriggerItem3(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(pEntity)
		pEntity->SendScriptEvent(ScriptEvent_ItemActivated, 3);
}

void CXClient::TriggerZoomToggle(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(pEntity)
	{
		
		pEntity->SendScriptEvent(ScriptEvent_ZoomToggle, (ae==etPressing?1:0));
	}
}

void CXClient::TriggerZoomIn(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(pEntity)
		pEntity->SendScriptEvent(ScriptEvent_ZoomIn, 0);
}

void CXClient::TriggerZoomOut(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(pEntity)
		pEntity->SendScriptEvent(ScriptEvent_ZoomOut, 0);
}

void CXClient::TriggerConcentration(float fValue,XActivationEvent ae)
{
	IEntity *pEntity=m_pEntitySystem->GetEntity(m_wPlayerID);
	IEntityContainer *pContainer=pEntity->GetContainer();
	if(pContainer)
	{
		CPlayer *pPlayer;
		pContainer->QueryContainerInterface(CIT_IPLAYER, (void**) &pPlayer);
		if (pPlayer)
			pPlayer->m_stats.concentration=true;
	}
}

void CXClient::TriggerQuickLoad(float fValue,XActivationEvent ae)
{
	if (m_pGame->IsQuicksaveAllowed())
		m_pGame->SendMessage("LoadGame");
}

void CXClient::TriggerQuickSave(float fValue,XActivationEvent ae)
{
	ICVar *g_LevelStated = GetISystem()->GetIConsole()->GetCVar("g_LevelStated");
	if (!g_LevelStated->GetIVal())
	{
		if (m_pGame->IsQuicksaveAllowed())
			m_pGame->SendMessage("SaveGame");
	}
}

void CXClient::TriggerMessageMode(float fValue,XActivationEvent ae)
{
	m_pGame->m_pSystem->GetIConsole()->ExecuteString("messagemode", 0);
}

void CXClient::TriggerMessageMode2(float fValue,XActivationEvent ae)
{
	m_pGame->m_pSystem->GetIConsole()->ExecuteString("messagemode2", 0);
}

void CXClient::TriggerScreenshot(float fValue, XActivationEvent ae)
{
	m_pGame->m_pSystem->GetIConsole()->ExecuteString("r_GetScreenShot 1", 0);
}

///////////////////////////////////////////////
// Client parser. 

//#define DEBUG_SNAPSHOT
bool CXClient::ParseIncomingStream(CStream &stm)
{
	// this is an incoming message for the client - it should not be processed on a server

	bool bRet = false;
	static int lastSuccessfulPacketID=-1;
	XSERVERMSG msg=0;
	//m_lstUpdatedEntities.clear();
	
	m_lstGarbageEntities.clear();
		//TRACE("--BEGIN---ParseIncomingStream--------------");
	
	do
	{
		if(!stm.ReadPkd(msg))
			return false;
		#ifdef DEBUG_SNAPSHOT		
		TRACE("SERVER MESSAGE\n");
		TRACE(">> STREAM size=%04d readpos=%04d",stm.GetSize(),stm.GetReadPos());
		#endif
		switch(msg)
		{
////////////////////////////////////////////
//RELIABLE MESSAGES
			case XSERVERMSG_SETTEAM:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_SETTEAM\n");
				#endif
				OnServerMsgSetTeam(stm);
			break;
			case XSERVERMSG_ADDTEAM:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_ADDTEAM\n");
				#endif
				OnServerMsgAddTeam(stm);
			break;
			case XSERVERMSG_REMOVETEAM:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_REMOVETEAM\n");
				#endif
				OnServerMsgRemoveTeam(stm);
			break;
			case XSERVERMSG_REQUESTSCRIPTHASH:
				OnServerMsgRequestScriptHash(stm);
			break;
			case XSERVERMSG_SETTEAMSCORE:
				OnServerMsgSetTeamScore(stm);
			break;
			case XSERVERMSG_SETTEAMFLAGS:
				OnServerMsgSetTeamFlags(stm);
				break;
			case XSERVERMSG_TEXT:				
				#ifdef DEBUG_SNAPSHOT		
				TRACE("XSERVERMSG_TEXT\n");
				#endif
				OnServerMsgText(stm);
			break;
			case XSERVERMSG_SETPLAYER:		
				//#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_SETPLAYER\n");
				//#endif
				OnServerMsgSetPlayer(stm);	
				break;
			case XSERVERMSG_ADDENTITY:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_ADDENTITY\n");
				#endif
				OnServerMsgAddEntity(stm);		
				break;
			case XSERVERMSG_REMOVEENTITY:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_REMOVEENTITY\n");
				#endif
				OnServerMsgRemoveEntity(stm);	
				break;
			case XSERVERMSG_SETENTITYNAME:
				//#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_SETENTITYNAME");
				//#endif
				OnServerMsgSetEntityName(stm);		
				break;
			case XSERVERMSG_SETPLAYERSCORE:
				NET_TRACE("<<NET>>XSERVERMSG_SETPLAYERSCORE");
				OnServerMsgSetPlayerScore(stm);
				break;
			case XSERVERMSG_SETENTITYSTATE:
				NET_TRACE("<<NET>>XSERVERMSG_SETENTITYSTATE");
				OnServerMsgSetEntityState(stm);
				break;
			case XSERVERMSG_BINDENTITY:
				NET_TRACE("<<NET>>XSERVERMSG_BINDENTITY");
				OnServerMsgBindEntity(stm);
				break;
			case XSERVERMSG_SCOREBOARD:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_SCOREBOARD\n");
				#endif
				{
					short size;

					if(!stm.ReadPkd(size))			// sub packet size
						return false;

					size_t readpos=stm.GetReadPos();

					OnServerMsgScoreBoard(stm);

					assert(stm.GetReadPos()==readpos+size);		// just for testing

					stm.Seek(readpos+size);			// jump over it in the case the packet wasn't processable
				}
				break;
			case XSERVERMSG_SETGAMESTATE:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_SETGAMESTATE\n");
				#endif
				OnServerMsgGameState(stm);		
				break;
/*			case XSERVERMSG_OBITUARY:
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_OBITUARY\n");
				#endif
				OnServerMsgObituary(stm);		
				break;
*/			case XSERVERMSG_TEAMS:
				NET_TRACE("<<NET>>XSERVERMSG_TEAMS\n");
				m_pISystem->ReadTeams(stm);
				break;
			case XSERVERMSG_CMD:
				OnServerMsgCmd(stm);
				break;
			case XSERVERMSG_SYNCVAR:
				OnServerMsgSyncVar(stm);
				break;
			case XSERVERMSG_AISTATE:
				OnServerMsgSyncAIState(stm);
				break;
////////////////////////////////////////////
//UNRELIABLE MESSAGES(are discarded by the local host)
			case XSERVERMSG_CLIENTSTRING:
				if(!OnServerMsgClientString(stm))
					return false;	
			case XSERVERMSG_UPDATEENTITY:		
				if(m_bLocalHost)
					return true;
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_UPDATEENTITY\n");
				#endif
				if(!OnServerMsgUpdateEntity(stm))
					return false;	
				break;
			case XSERVERMSG_TIMESTAMP:			
				if(m_bLocalHost)
					return true;
				#ifdef DEBUG_SNAPSHOT		
				NET_TRACE("<<NET>>XSERVERMSG_TIMESTAMP\n");
				#endif
				if(!OnServerMsgTimeStamp(stm))
				{
					NET_TRACE("<<NET>>OLD PACKET DISCARDED");
					m_nDiscardedPackets++;
					return false;
				}
        break;
			case XSERVERMSG_EVENTSCHEDULE:
				if(m_bLocalHost)
					return true;
				if (!OnServerMsgEventSchedule(stm))
					return false;
				break;

			
			default:
				m_pLog->LogError("lastSuccessfulPacketID=%i currentPacketID=%i - wrong data chunk.", (int)lastSuccessfulPacketID, (int)msg);
				assert(0);
				return false;
				break;
		}
#ifdef DEBUG_SNAPSHOT		
		NET_TRACE("<<NET>><< STREAM size=%04d readpos=%04d",stm.GetSize(),stm.GetReadPos());		
#endif
		
	} while(!stm.EOS());

	// only needed in SP (cause problems in MP)
	if(!m_lstGarbageEntities.empty())
	{
		EntityIdListItor itor=m_lstGarbageEntities.begin();
			while(itor!=m_lstGarbageEntities.end())
			{
				m_pISystem->RemoveEntity(*itor);
				++itor;
			}
			m_lstGarbageEntities.clear();
	}

	if(!m_lstUpdatedEntities.empty())
	{
		EntityListItor itor=m_lstUpdatedEntities.begin();
		for(; itor!=m_lstUpdatedEntities.end(); ++itor)
			if ((*itor)->GetPhysics())
				(*itor)->GetPhysics()->PostSetStateFromSnapshot();

		// update the world here
		if (m_iPhysicalWorldTime)
			m_pGame->AdvanceReceivedEntities(m_iPhysicalWorldTime);
		//m_iPhysicalWorldTime = 0;

		pe_params_flags pf;
		pf.flagsAND = ~pef_update;
		for(itor=m_lstUpdatedEntities.begin(); itor!=m_lstUpdatedEntities.end(); )
		{
			if ((*itor)->GetPhysics())
			{
				(*itor)->GetPhysics()->GetParams(&pf);
				(*itor)->GetPhysics()->SetParams(&pf);
				if (!(pf.flags & pef_checksum_outofsync))
				{
					itor = m_lstUpdatedEntities.erase(itor);
					continue;
				}
			} 
			++itor;
		}
	}

#ifdef DEBUG_SNAPSHOT		
		NET_TRACE("<<NET>>--END-----ParseIncomingStream--------------");
#endif
		lastSuccessfulPacketID=msg;
	return bRet;
}

///////////////////////////////////////////////
// XSERVERMSG_SETPLAYER
bool CXClient::OnServerMsgSetPlayer(CStream &stm)
{
	m_wPlayerID = INVALID_WID;

	EntityId id;
	Vec3 v3Angles;
	VERIFY_COOKIE(stm);

	if(!stm.Read(id))
		return false;
	if(!stm.Read(v3Angles))
		return false;

	VERIFY_COOKIE(stm);
	IEntity *pEntity = m_pISystem->GetEntity(id);

	if(pEntity)
	{
		IEntityContainer *pCnt;
		CPlayer *pPlayer = NULL;

		SetPlayerID(id);
		pEntity->SetAngles(v3Angles);
		pCnt=pEntity->GetContainer();

		if(pCnt)
		{
			pCnt->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayer);

			if(pPlayer)
			{
				m_pLog->Log("SET PLAYER [%d]",id);

				//m_bFirstPerson = true;
				m_pGame->SetViewMode(!pPlayer->m_bFirstPersonLoaded);

				// make sure that the key state of the input is cleared at this point to prevent the
				// player from shooting because he clicked to respawn
				//				IInput *pInput=m_pGame->GetSystem()->GetIInput();
				//			if(pInput)
				//				pInput->GetIKeyboard()->ClearKeyState();

				//Initialize various stuff like Hud etc...

				//				m_pLog->Log("CLIENT SET` PLAYER : %d - %s - Pos : \n", pEntity->GetId(), pEntity->GetName());

				int tempweapon = pPlayer->m_stats.weapon;
				pPlayer->SelectWeapon(-1);
				pPlayer->SelectWeapon(tempweapon);

				// call OnSetPlayer whenever the localplayer is spawned and set
				m_pScriptSystem->BeginCall("ClientStuff", "OnSetPlayer");
				m_pScriptSystem->PushFuncParam(m_pClientStuff);
				m_pScriptSystem->EndCall();

				if (m_bMapConnecting) 
					OnMapChangedReally();				
			}
			else m_pLog->Log("SET PLAYER [%d]b",id);
		}
		else m_pLog->Log("SET PLAYER [%d] failed1",id);
	}
	else m_pLog->Log("SET PLAYER [%d] failed2",id);

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_SETTEAM
bool CXClient::OnServerMsgSetTeam(CStream &stm)
{
	EntityId EntId;
	BYTE nTeamId;
	VERIFY_COOKIE(stm);
	stm.Read(EntId);
	stm.Read(nTeamId);
	VERIFY_COOKIE(stm);
	m_pISystem->SetTeam(EntId, (int)nTeamId);
	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_ADDTEAM
bool CXClient::OnServerMsgAddTeam(CStream &stm)
{
	string sTeamName;
	BYTE nTeamId;
	VERIFY_COOKIE(stm);
	stm.Read(sTeamName);
	stm.Read(nTeamId);
	VERIFY_COOKIE(stm);
	m_pISystem->AddTeam(sTeamName.c_str(), (int)nTeamId);
	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_REMOVETEAM
bool CXClient::OnServerMsgRemoveTeam(CStream &stm)
{
	BYTE nTeamId;
	stm.Read(nTeamId);
	m_pISystem->RemoveTeam(nTeamId);
	return true;
}



inline void CalcNCombineHash( const unsigned int indwValue, unsigned int &inoutHash )
{
	inoutHash^=(inoutHash%600011) + (inoutHash/600011) + indwValue;
}


///////////////////////////////////////////////
// XSERVERMSG_REQUESTSCRIPTHASH
bool CXClient::OnServerMsgRequestScriptHash(CStream &stm)
{
	IBitStream *pBitStream = m_pGame->GetIBitStream();

	char szPath[256];
	char szKey[256];

	u32 dwHash;
	EntityId entity=INVALID_WID;

	if(!pBitStream->ReadBitStream(stm,entity,eEntityId))							// e.g. INVALID_WID for globals, otherwise it's and entity
		return false;
	if(!pBitStream->ReadBitStream(stm,szPath,255,eASCIIText))					// e.g. "cnt.myTable"
		return false;
	if(!pBitStream->ReadBitStream(stm,szKey,255,eASCIIText))					// e.g. "luaFunc1" or "" 
		return false;
	if(!pBitStream->ReadBitStream(stm,dwHash,eDoNotCompress))					// start hash
		return false;

	// debug
//	m_pLog->Log("OnServerMsgRequestScriptHash '%s' '%s'",szPath,szKey);

	// find the specified root -------------------
/*
	IScriptSystem *pScriptSystem=m_pGame->GetScriptSystem();
	IScriptObject *pRoot;

	if(entity==INVALID_WID)
	{
		// global
		pRoot=pScriptSystem->GetGlobalObject();
	}
	else
	{
		// relative to a entity
		IEntity *pEntity=m_pISystem->GetEntity(entity);
		
		pRoot=pEntity->GetScriptObject();
	}

	if(!pRoot)
		return false;

	// apply the path -----------------------------

	IScriptObject *pPath=m_pScriptSystem->CreateEmptyObject();

	if(!pRoot->GetValueRecursive(szPath,pPath))						// is modifying the given Hash
	{
		pRoot->Release();
		return false;
	}

	// --------------------------------------------

	if(*szKey)
	{
		unsigned int *pCode;
		int iSize;

		if(pPath->GetFuncData(szKey,pCode,iSize))		// get function data
		if(pCode)																		// it's a lua function
		{
			for(int i=0;i<iSize;i++)
				CalcNCombineHash(*pCode++,dwHash);
		}
	}
	else
	{
		std::map<string,unsigned int> Sorter;

todo: Sorter is needed because lua table don't have a fixed order

		// the whole table
		pPath->BeginIteration();
		while(pPath->MoveNext())
		{
			unsigned int *pCode;
			int iSize;

			if(pPath->GetCurrentFuncData(pCode,iSize))		// get function data
			if(pCode)																		// it's a lua function
			{
				unsigned int dwLocalHash=0;

				for(int i=0;i<iSize;i++)
					CalcNCombineHash(*pCode++,dwLocalHash);

				// debugging
				{
//					char *szKey;
//					pPath->GetCurrentKey(szKey);			
					
//					assert(szKey);

//					m_pGame->m_pSystem->GetILog()->Log("GetCurrentFuncData hash=%p '%s' size=%d",dwLocalHash,szKey,iSize);
				}

				CalcNCombineHash(dwLocalHash,dwHash);
			}
		}
		pPath->EndIteration();
	}

//	pPath->Release();   todo?
//	pRoot->Release();		todo?
*/

	// send back the information -----------------
//	SendScriptHashResponse(dwHash);

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_SETTEAMSCORE
bool CXClient::OnServerMsgSetTeamScore(CStream &stm)
{
	BYTE cTeamId;
	short nScore;
	VERIFY_COOKIE(stm);
	stm.Read(cTeamId);
	stm.Read(nScore);
	VERIFY_COOKIE(stm);
	m_pISystem->SetTeamScore(cTeamId,nScore);
	
	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_SETTEAMSCORE
bool CXClient::OnServerMsgSetTeamFlags(CStream &stm)
{
	BYTE cTeamId;
	int nFlags;
	VERIFY_COOKIE(stm);
	stm.Read(cTeamId);
	stm.ReadPkd(nFlags);
	VERIFY_COOKIE(stm);
	m_pISystem->SetTeamFlags(cTeamId,nFlags);

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_SETENTITYSTATE
bool CXClient::OnServerMsgSetEntityState(CStream &stm)
{
	IBitStream *pBitStream = m_pGame->GetIBitStream();			// compression helper

	EntityId id;
	unsigned char cState;
	IEntity *pEntity;
	VERIFY_COOKIE(stm);
	pBitStream->ReadBitStream(stm,id,eEntityId);
	stm.Read(cState);
	VERIFY_COOKIE(stm);

	// in SP or if you play on the server we don't need to set the state
	if(m_pGame->IsMultiplayer() && !m_bLocalHost)	
	{
		pEntity=m_pISystem->GetEntity(id);

		if(pEntity && (pEntity->GetStateIdx()!=cState))
			pEntity->GotoState(cState);
	}

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_BINDENTITY
bool CXClient::OnServerMsgBindEntity(CStream &stm)
{
	// this is an incoming message for the client - it should not be processed on a server

	EntityId idParent;
	EntityId idChild;
	unsigned char cParam;
	IEntity *pParent,*pChild;
	bool bBindUnbind;
	Vec3 vParent,vChild;
	stm.Read(idParent);
	stm.Read(idChild);
	stm.Read(cParam);
	stm.Read(bBindUnbind);
	stm.Read(vParent);
	stm.Read(vChild);

	if(m_pGame->IsMultiplayer())
	{
		pParent=m_pISystem->GetEntity(idParent);
		pChild=m_pISystem->GetEntity(idChild);
		if(pParent && pChild)
		{
			if(bBindUnbind)
				pParent->Bind(idChild,cParam,true);	// bClientOnly=true
			else
				pParent->Unbind(idChild,cParam,true);	// bClientOnly=true
		}
	}

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_SETPLAYERSCORE
bool CXClient::OnServerMsgSetPlayerScore(CStream &stm)
{
	short nScore=0;
	//	short nFlags=0;
	EntityId id=0;
	VERIFY_COOKIE(stm);
	stm.Read(id);
	stm.Read(nScore);
	//	stm.Read(nFlags);
	VERIFY_COOKIE(stm);
	IEntity *pEntity=m_pISystem->GetEntity(id);

	if(pEntity)
	{
		IEntityContainer *pC=pEntity->GetContainer();	
		if(pC)
		{
			CPlayer *pPlayer=NULL;
			pC->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayer);
			if(pPlayer)
			{
				pPlayer->m_stats.score=nScore;
				//				pPlayer->m_stats.pflags=nFlags;
				m_pScriptSystem->GetGlobalValue("ClientStuff",m_pClientStuff);
				_HScriptFunction pFunc(m_pScriptSystem);
				if(pFunc=m_pScriptSystem->GetFunctionPtr("ClientStuff","OnSetPlayerScore")){
					m_pScriptSystem->BeginCall(pFunc);
					m_pScriptSystem->PushFuncParam(m_pClientStuff);
					m_pScriptSystem->PushFuncParam(pEntity->GetScriptObject());
					m_pScriptSystem->EndCall();
				}

			}
		}
	}

	return true;
}
///////////////////////////////////////////////
// XSERVERMSG_SETGAMESTATE
bool CXClient::OnServerMsgGameState(CStream &stm)
{
	stm.Read(m_nGameState);
	stm.Read(m_nGameLastTime);

	m_fGameLastTimeReceived = m_pGame->m_pSystem->GetITimer()->GetCurrTime();

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_SCOREBOARD
bool CXClient::OnServerMsgScoreBoard(CStream &stmScoreBoard)
{
	CScriptObjectStream stmScript;

	stmScript.Create(m_pScriptSystem);
	stmScript.Attach(&stmScoreBoard);

	bool bContinue;

	m_pScriptSystem->BeginCall("ClientStuff", "ResetScores");
	m_pScriptSystem->PushFuncParam(m_pClientStuff);
	m_pScriptSystem->EndCall();
	
	while(stmScoreBoard.Read(bContinue) && bContinue)
	{
		m_pScriptSystem->BeginCall("ClientStuff", "SetPlayerScore");
		m_pScriptSystem->PushFuncParam(m_pClientStuff);
		m_pScriptSystem->PushFuncParam(stmScript.GetScriptObject());
		m_pScriptSystem->EndCall();
	};
	
	m_pScriptSystem->BeginCall("ClientStuff", "ShowScoreBoard");
	m_pScriptSystem->PushFuncParam(m_pClientStuff);
	m_pScriptSystem->PushFuncParam(1);
	m_pScriptSystem->EndCall();

	m_fLastScoreBoardTime = m_pGame->m_pSystem->GetITimer()->GetCurrTime();

	return true;
};

///////////////////////////////////////////////
// XSERVERMSG_COMMAND
bool CXClient::OnServerMsgText(CStream &stm)
{
	TextMessage pTextMessage;
	pTextMessage.Read(stm);

	IEntitySystem *pEntitySystem = m_pGame->GetSystem()->GetIEntitySystem();

	string szText = pTextMessage.m_sText;
	string szCmdName;
	string szSenderName;

//	if(pTextMessage.stmPayload.Read(szText))
	{
		// if uiSender is 0, it's a message from the server
		IEntity *pSender=m_pISystem->GetEntity(pTextMessage.uiSender);

		if(pSender)
		{
			szSenderName = pSender->GetName();

			// public message
			if (pTextMessage.cMessageType == CMD_SAY)
			{
				szCmdName = "say";
			}
			// team-only message
			else if (pTextMessage.cMessageType == CMD_SAY_TEAM)
			{
				szCmdName = "sayteam";
			}
			// private player-to-player message
			else if (pTextMessage.cMessageType == CMD_SAY_ONE)
			{
				szCmdName = "sayone";
			}
			else
			{
				szCmdName = "";
			}
		}

		if (!szText.size())
		{
			return true;
		}

		HSCRIPTFUNCTION pScriptFunction = m_pScriptSystem->GetFunctionPtr("ClientStuff", "OnTextMessage");

		if (pScriptFunction)
		{
			m_pScriptSystem->ReleaseFunc(pScriptFunction);

			_SmartScriptObject pClientStuff(m_pScriptSystem, true);

			m_pScriptSystem->GetGlobalValue("ClientStuff", *pClientStuff);

			m_pScriptSystem->BeginCall("ClientStuff", "OnTextMessage");
			m_pScriptSystem->PushFuncParam(pClientStuff);
			m_pScriptSystem->PushFuncParam(szCmdName.c_str());
			m_pScriptSystem->PushFuncParam(szSenderName.c_str());
			m_pScriptSystem->PushFuncParam(szText.c_str());
			m_pScriptSystem->EndCall();
		}

		string szHudMessage;
	
		if (szCmdName == "sayone")
		{
			szHudMessage = "$4[$1" + szSenderName + "$4]$9 " + szText;
		}
		else if (szCmdName == "sayteam")
		{
			szHudMessage = "$4[$1" + szSenderName + "$4]$3 " + szText;
		}
		else if (szCmdName == "say")
		{
			szHudMessage = "$4[$1" + szSenderName + "$4]$1 " + szText;
		}
		else
		{
			szHudMessage = szText;
		}

		{
			wstring szEnglishLocalized;

			m_pGame->m_StringTableMgr.Localize(szHudMessage,szEnglishLocalized,true);		// localize to english
						
			std::vector<char> tmp;
			tmp.resize(szEnglishLocalized.size()+1);

			sprintf (&tmp[0], "%S", szEnglishLocalized.c_str());

			m_pGame->m_pLog->LogToConsole("%s",&tmp[0]);			// needed for console printout of \callvote response
		}

		AddHudMessage(szHudMessage.c_str(), pTextMessage.fLifeTime);
	}

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_ADDENTITY
bool CXClient::OnServerMsgAddEntity(CStream &stm)
{
	CEntityDesc ed;

	ed.Read(m_pGame->GetIBitStream(),stm);

	IEntity *pEntity = m_pISystem->SpawnEntity(ed);
	NET_TRACE("<<NET>> XSERVERMSG_ADDENTITY %d",ed.id);

	if(m_pGame->IsMultiplayer())
	{
		if(pEntity)
		{
			IEntityContainer *pEntCont=pEntity->GetContainer();

			if(pEntCont)
				pEntCont->Update();
		}
		else
		{
			m_pLog->Log("OnServerMsgAddEntity entity was 0");			// remove soon
		}
	}

	// Tell the client in lua that an entity has spawned
	if (pEntity && pEntity->GetScriptObject())
	{
		//Timur, Look ups by name in lua must be replaced with table reference.
		IScriptSystem *pSS = m_pGame->GetScriptSystem();
		_SmartScriptObject pClientStuff(pSS,true);
		if(pSS->GetGlobalValue("ClientStuff",pClientStuff))
		{
			pSS->BeginCall("ClientStuff","OnSpawnEntity");
			pSS->PushFuncParam(pClientStuff);
			m_pScriptSystem->PushFuncParam(pEntity->GetScriptObject());
			pSS->EndCall();
		}
	}

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_REMOVEENTITY
bool CXClient::OnServerMsgRemoveEntity(CStream &stm)
{
	EntityId id;
	stm.Read(id);
#ifdef _DEBUG
  NET_TRACE("<<NET>>CXClient::OnServerMsgRemoveEntity %04d",id);
#endif // _DEBUG
  NET_TRACE("<<NET>> XSERVERMSG_REMOVEENTITY %d",id);
	if(id==m_wPlayerID)
	{
		SetPlayerID(INVALID_WID);
	}

	if(m_pGame->IsMultiplayer())
		m_pISystem->RemoveEntity(id);					// MP: remove it right now
	 else
		m_lstGarbageEntities.push_back(id);		// SP: remove it later

	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_UPDATEENTITY
bool CXClient::OnServerMsgUpdateEntity(CStream &stm)
{
	IBitStream *pBitStream = m_pGame->GetIBitStream();			// compression helper

	EntityId id;

	pBitStream->ReadBitStream(stm,id,eEntityId);
//	stm.Read(id);

	//TRACE("XSERVERMSG_UPDATEENTITY [%d]",id);
	IEntity *ent = m_pISystem->GetEntity(id);
	
	if(ent)
	{
		if(!ent->Read(stm,m_bIgnoreSnapshot))
		{
			m_pLog->Log("WARNING ENTITY [%d] error reading from the snapshot!!",id);
			return false;
		}

		if (!m_bIgnoreSnapshot)
		{
			//if the entity is the player of this client 
			//the localhost override the angles
 			if(ent->GetId()==m_wPlayerID && !ent->IsBound())
			{
				ent->SetAngles(m_PlayerProcessingCmd.GetDeltaAngles(),false,false);
			}
			else if (ent->GetPhysics())
			{	// the player will update her physics herself (to preserve perfect client time-slice based simuklation)
				// the rest will be updated to match physics time later in ParseIncomingStream
				pe_params_flags pf;
				pf.flagsOR = pef_update;
				m_lstUpdatedEntities.push_back(ent);
				ent->GetPhysics()->GetParams(&pf);
				if (!(pf.flags & pef_checksum_received))
					ent->GetPhysics()->SetParams(&pf);
			}
		}
	}
	else
	{
		NET_TRACE("<<NET>> ENTITY [%d] UPDATED BUT BOT SPAWNED",id);
		GameWarning( "ENTITY [%d] Updated but not spawned!!",id );
		return false;
	}
	VERIFY_COOKIE_NO(stm,28);
	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_TIMESTAMP
bool CXClient::OnServerMsgTimeStamp(CStream &stm)
{
	int iPhysicalTime,iPrevWorldTime=m_iPhysicalWorldTime;
	IPhysicalWorld *pWorld = m_pGame->GetSystem()->GetIPhysicalWorld();
	stm.Read(iPhysicalTime);
	stm.Read(m_iPhysicalWorldTime);

	if(stm.GetStreamVersion()<PATCH1_SAVEVERSION)					// to be backward compatible with old samegames
	{
		BYTE seed;
		stm.Read(seed);
//		m_pISystem->SetRandomSeed(seed);
	}

	float fDelta = (m_iPhysicalWorldTime-iPrevWorldTime)*pWorld->GetPhysVars()->timeGranularity;
	if (fDelta>-2.0f && fDelta<0)
	{
		m_bIgnoreSnapshot = true;
		m_iPhysicalWorldTime = iPrevWorldTime;
	}
	else
	{
		m_bIgnoreSnapshot = false;
		if (!m_bLocalHost) 
		{
			pWorld->SetiSnapshotTime(iPhysicalTime,0);	
			pWorld->SetiSnapshotTime(m_iPhysicalWorldTime,1);
			pWorld->SetiSnapshotTime(m_pGame->SnapTime(m_iPhysicalWorldTime),2);
		}
	}
	return true;
}


///////////////////////////////////////////////
// XSERVERMSG_EVENTSCHEDULE
bool CXClient::OnServerMsgEventSchedule(CStream &stm)
{
	m_pGame->ReadScheduledEvents(stm);
	return true;
}


///////////////////////////////////////////////
// XSERVERMSG_SETENTITYNAME
bool CXClient::OnServerMsgSetEntityName(CStream &stm)
{
	EntityId id;
	string sName;
	VERIFY_COOKIE(stm);
	if(!stm.Read(id))return false;
	if(!stm.Read(sName))return false;
	VERIFY_COOKIE(stm);
	IEntity *pEntity=m_pISystem->GetEntity(id);
	if(pEntity)
	{
		pEntity->SetName(sName.c_str());
		NET_TRACE("<<NET>>SET Entity NAME =\"%s\"",sName.c_str());
	}
	return true;
}

///////////////////////////////////////////////
// XSERVERMSG_CMD
bool CXClient::OnServerMsgCmd(CStream &stm)
{
	if(!m_pClientStuff)
		return true;

	string cmd;
	bool bExtra;
	unsigned char cUserByte;
	Vec3 vNormal,vPos;
	EntityId id;

	stm.Read(cmd);

	stm.Read(bExtra);
	if(bExtra)
	{
		{
			bool bPos;

			stm.Read(bPos);
			
			if(bPos)
			{
				CStreamData_WorldPos tmp(vPos);
				stm.ReadPkd(tmp);
			}
			 else
				vPos=Vec3(0,0,0);
		}

		{
			bool bNormal;

			stm.Read(bNormal);
			
			if(bNormal)
			{
				CStreamData_Normal tmp(vNormal);
				stm.ReadPkd(tmp);
			}
			 else
				vNormal=Vec3(0,0,0);
		}
		stm.ReadPkd(id);
		stm.ReadPkd(cUserByte);
	}

	m_pScriptSystem->BeginCall("ClientStuff","OnServerCmd");
	m_pScriptSystem->PushFuncParam(m_pClientStuff);
	m_pScriptSystem->PushFuncParam(cmd.c_str());
	if(bExtra)
	{
		m_sopMsgPos.Set(vPos);
		m_sopMsgNormal.Set(vNormal);
		m_pScriptSystem->PushFuncParam(m_sopMsgPos);
		m_pScriptSystem->PushFuncParam(m_sopMsgNormal);
		m_pScriptSystem->PushFuncParam(id);
		m_pScriptSystem->PushFuncParam(cUserByte);
	}
//		NET_TRACE("<<NET>>COMMAND RECEIVED [%s]",cmd.c_str());
	m_pScriptSystem->EndCall();

	return true;
}


bool CXClient::OnServerMsgClientString(CStream &stm)
{
	m_fLastClientStringTime=m_pTimer->GetCurrTime();
	return stm.Read(m_sClientString);

}

///////////////////////////////////////////////
// XSERVERMSG_SYNCVAR
bool CXClient::OnServerMsgSyncVar(CStream &stm)
{
	string name,val;
	stm.Read(name);
	stm.Read(val);
	m_pISystem->SetVariable(name.c_str(),val.c_str());
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CXClient::OnServerMsgSyncAIState(CStream &stm)
{
	int nDummy;
	stm.Read(nDummy);

	// [marco] petar do the stuff here after you read the packet
	// from the stream

	return (true);
}

//////////////////////////////////////////////////////////////////////////
void CXClient::SendTextMessage(TextMessage &tm)
{
	CStream stm;
	tm.uiSender=m_wPlayerID;
	tm.Write(stm);
	SendReliableMsg(XCLIENTMSG_TEXT,stm);
}

void CXClient::SetBitsPerSecond( const unsigned int dwBitsPerSecond )
{
	CStream stm;

	unsigned char cVar=0;

	stm.Write(cVar);
	stm.Write(dwBitsPerSecond);								// cVar=0

	SendReliableMsg(XCLIENTMSG_RATE,stm);
}


void CXClient::SetUpdateRate( const unsigned int dwUpdatesPerSec )
{
	CStream stm;

	unsigned char cVar=1;

	stm.Write(cVar);
	stm.Write(dwUpdatesPerSec);								// cVar=1

	SendReliableMsg(XCLIENTMSG_RATE,stm);
}


void CXClient::SendCommand(const char *sCmd)
{
	CStream stm;
	stm.Write(sCmd);
	SendReliableMsg(XCLIENTMSG_CMD,stm);
}



//true
bool FrontOfLine(float Ax,float Ay,float Bx,float By,float xt,float yt)
{
	float s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));

	if (s>=0) 
	{ 
		//front
		return true;
	}
	else 
	{ 
		//back
		return false; 
	}
}

//-------------------------------
//		LINE A    LINE B
//	x1,y1\       /x2,y2
//         \   /
//				   O
//         /   \
//	x3,y3/       \x4,y4
//----------------------------------
void CXClient::SoundEvent(EntityId idSrc,Vec3 &pos,float fRadius,float fThreat)
{
	if(!m_wPlayerID)
		return;
	IEntity *pPlayer=m_pEntitySystem->GetEntity(m_wPlayerID);
	if(!pPlayer)
		return;
	Vec3 ppos=pPlayer->GetPos();
	float fDistance2=GetSquaredDistance(pos,ppos)-(fRadius*fRadius);
	//the player isn't bothered by his own sounds, but we wanna show them in the radar
	//if (m_wPlayerID==idSrc)
	{
		float fSoundEventRadius=cl_sound_event_radius->GetFVal();
		if (fDistance2<(fSoundEventRadius*fSoundEventRadius))
		{
			SSoundInfo SoundInfo;
			SoundInfo.nEntityId=idSrc;
			SoundInfo.Pos=pos;
			SoundInfo.fRadius=fRadius;
			SoundInfo.fThread=fThreat;
			SoundInfo.fDistance2=fDistance2;
			SoundInfo.fTimeout=cl_sound_event_timeout->GetFVal();
			m_lstSounds.push_back(SoundInfo);
		}
		//return;
	}
	float x1=-1000+ppos.x,y1=-1000+ppos.y;
	float x2=1000+ppos.x,y2=-1000+ppos.y;
	float x3=-1000+ppos.x,y3=1000+ppos.y;
	float x4=1000+ppos.x,y4=1000+ppos.y;
	bool bLineA;
	bool bLineB;
	float fMaxDistance=cl_sound_detection_max_distance->GetFVal();
	float fMinDistance=cl_sound_detection_min_distance->GetFVal();
	if((fDistance2<(fMaxDistance*fMaxDistance)) 
		&& (fDistance2>(fMinDistance*fMinDistance)))
	{
		Vec3 vAng=pPlayer->GetAngles();
		float fCos=(float)cos_tpl(-DEG2RAD(vAng.z));
		float fSin=(float)sin_tpl(-DEG2RAD(vAng.z));
		pos-=ppos;
		float fSoundX=(pos.x*fCos) - (pos.y*fSin);
		float fSoundY=(pos.x*fSin) + (pos.y*fCos);
		fSoundX+=ppos.x;
		fSoundY+=ppos.y;
		bLineA=FrontOfLine(x1,y1,x4,y4,fSoundX,fSoundY);
		bLineB=FrontOfLine(x2,y2,x3,y3,fSoundX,fSoundY);
		if(bLineA)
		{
			if(bLineB)
			{
				//LEFT
				//left
				m_fLeftSound+=1;
				//::OutputDebugString("left\n");
				if(m_fLeftSound>1)
					m_fLeftSound=1;
				
			}
			else
			{

				//FRONT!!
				//front
				m_fFrontSound+=1;
				//::OutputDebugString("front\n");
				if(m_fFrontSound>1)
					m_fFrontSound=1;
				
			}
		}
		else
		{
			if(bLineB)
			{
				//BACK
				//back
				m_fBackSound+=1;
				//::OutputDebugString("back\n");
				if(m_fBackSound>1)
					m_fBackSound=1;
				
			}
			else
			{
				//RIGHT!!
				m_fRightSound+=1;
				//right
				//::OutputDebugString("right\n");
				if(m_fRightSound>1)
					m_fRightSound=1;
				
			}
		}
	}
}

void CXClient::SetEntityCamera( const SCameraParams &CameraParams )
{
	if (m_CameraParams)
		*m_CameraParams = CameraParams;
}

void CXClient::AddHudMessage(const char *sMessage,float lifetime,bool bHighPriority)
{
	_SmartScriptObject pHud(m_pScriptSystem,true);
	const char *sfunc="AddMessage";

	if(m_pScriptSystem->GetGlobalValue("Hud",pHud))
	{
		if(bHighPriority){
			sfunc="AddCenterMessage";
		}
		m_pScriptSystem->BeginCall("Hud",sfunc);
		m_pScriptSystem->PushFuncParam(pHud);
		m_pScriptSystem->PushFuncParam(sMessage);
		m_pScriptSystem->PushFuncParam(lifetime);
		m_pScriptSystem->EndCall();
	}
}

void CXClient:: AddHudSubtitle(const char *sMessage, float lifetime)
{
  _SmartScriptObject pHud(m_pScriptSystem,true);
  const char *sfunc="AddSubtitle";

  if(m_pScriptSystem->GetGlobalValue("Hud",pHud))
  {
    m_pScriptSystem->BeginCall("Hud",sfunc);
    m_pScriptSystem->PushFuncParam(pHud);
    m_pScriptSystem->PushFuncParam(sMessage);   
    m_pScriptSystem->PushFuncParam(lifetime);
    m_pScriptSystem->EndCall();
  }
}

void CXClient:: ResetSubtitles(void)
{
  _SmartScriptObject pHud(m_pScriptSystem,true);
  const char *sfunc="ResetSubtitles";

  if(m_pScriptSystem->GetGlobalValue("Hud",pHud))
  {
    m_pScriptSystem->BeginCall("Hud",sfunc);
    m_pScriptSystem->PushFuncParam(pHud);
    m_pScriptSystem->EndCall();
  }
}

void CXClient::LoadingError(const char *szError)
{
//	m_pLog->Log("HIDE CONSOLE");
	if (m_pGame->IsMultiplayer())
		m_pGame->GetSystem()->GetIRenderer()->ClearColorBuffer(Vec3(0,0,0));
	m_pGame->GetSystem()->GetIConsole()->ResetProgressBar(0);
	m_pGame->m_pSystem->GetIConsole()->ShowConsole(false);
	m_pGame->m_pSystem->GetIConsole()->SetScrollMax(600/2);

	m_pScriptSystem->BeginCall("Game", "OnLoadingError");
	m_pScriptSystem->PushFuncParam(m_pGame->GetScriptObject());
	m_pScriptSystem->PushFuncParam(szError);
	m_pScriptSystem->EndCall();
}

unsigned int CXClient::GetTimeoutCompensation()
{
	if (m_nGameState == CGS_INTERMISSION)
	{
		if (m_pGame->cl_loadtimeout->GetIVal() < 5)
		{
			return 5000;
		}

		return (unsigned int)(m_pGame->cl_loadtimeout->GetFVal() * 1000.0f);
	}

	return 0;
}

void CXClient::LazyChannelAcknowledge()
{
	m_bLazyChannelState=!m_bLazyChannelState;
}

bool CXClient::GetLazyChannelState()
{
	return m_bLazyChannelState;
}
