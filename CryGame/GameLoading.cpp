//////////////////////////////////////////////////////////////////////
//
//	Game source code
//	
//	File: GameLoading.cpp
//  
//	History: 
//	-December 11,2001:  
//		Created by Alberto and Petar 
//	
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"

#include "UIHud.h"

#include "XPlayer.h"
#include "XVehicle.h"


#include "XVehicleSystem.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"

#include "EntityClassRegistry.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include "ScriptObjectLanguage.h"
#include "ScriptObjectStream.h"
#include "ScriptObjectRenderer.h"
#include "ScriptObjectAI.h"
#include <IEntitySystem.h>
#include <IMovieSystem.h>


#include "UISystem.h"
#include "ScriptObjectUI.h"
#include "TagPoint.h"
//#include "XPath.h"
#include <ISound.h>
#include <IAgent.h>

#if !defined(LINUX)
#	include <dbghelp.h>
#	pragma comment(lib, "dbghelp.lib")
#else
#	include <stdio.h>
#endif

#if defined(LINUX)
	#include "ILog.h"
#endif

//////////////////////////////////////////////////////////////////////
#define CHUNK_ENTITY 0x01
#define CHUNK_PLAYER 0x02
#define CHUNK_AI	 0x03
#define CHUNK_INGAME_SEQUENCE	 0x04
#define CHUNK_HUD	 0x05

//#define _INTERNET_SIMULATOR

#define SAVEGAME_THUMBNAIL_SIZEX 128
#define SAVEGAME_THUMBNAIL_SIZEY 96

//////////////////////////////////////////////////////////////////////
// load level batch process
bool CXGame::LoadLevelForEditor(const char *pszLevelDirectory, const char *pszMissionName)
{
	if (pszMissionName)
		m_currentMission = pszMissionName;

	// start local server
	if(!StartupServer(false,"__editor__"))
		return false;

	//////////////////////////////////////////////////////////////////////////
	// Init common stuff.
	//////////////////////////////////////////////////////////////////////////
	//init the entity registry
	m_pServer->m_pISystem->LoadLevel( pszLevelDirectory,m_currentMission.c_str(),true );

	// start local client and connect
	if(!StartupLocalClient())
		return false;

	m_pClient->XConnect("localhost");
	m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
	return true;
}

#define TABLE_END (-1)      // a value not used in enum ScriptVarType
//////////////////////////////////////////////////////////////////////////
struct PropertyWriter : IScriptObjectDumpSink
{
	IScriptObject *table;
	CStream &stm;
	IScriptSystem *m_pScriptSystem;

	PropertyWriter(IScriptObject *p, CStream &s, IScriptSystem *ss) : table(p), stm(s), m_pScriptSystem(ss) {};

	void Element(const char *sName, int nIdx, ScriptVarType type, bool iskey)
	{
		switch(type)
		{
		case svtUserData:
		case svtFunction: TRACE("WARNING: userdata or function found in properties table (%s)", sName);
 		case svtNull:     break;
		case svtString:   { const char *s = ""; _VERIFY(iskey ? table->GetValue(sName, s) : table->GetAt(nIdx, s)); stm.Write(s); }; break;
		case svtNumber:   { float f = 0;  _VERIFY(iskey ? table->GetValue(sName, f) : table->GetAt(nIdx, f)); stm.Write(f); }; break;
		case svtObject: 
			{
				_SmartScriptObject t(m_pScriptSystem, true);
				_VERIFY(iskey ? table->GetValue(sName, t) : table->GetAt(nIdx, t));
				t->Dump(&PropertyWriter(t, stm, m_pScriptSystem));
				stm.Write((char)TABLE_END);
				break;
			};
		};
	};

	//////////////////////////////////////////////////////////////////////////	
	void OnElementFound(const char *sName, ScriptVarType type)
	{
		stm.Write((char)type);
		stm.Write((char)true);
		stm.Write(sName);

		Element(sName, 0, type, true);
	};

	void OnElementFound(int nIdx, ScriptVarType type)
	{
		stm.Write((char)type);
		stm.Write((char)false);
		stm.Write(nIdx);
		Element("", nIdx, type, false);
	};
};

//////////////////////////////////////////////////////////////////////////
void LoadProperties(IScriptObject *table, CStream &stm, IScriptSystem *ss, char *parent)
{
	ASSERT(table);
	ASSERT(ss);  // martin made me do it
	for(;;)
	{
		char what;
		stm.Read(what);
		if(what==TABLE_END) return;     

		char iskey;
		stm.Read(iskey);
		int idx = 0;
		string key;
		iskey ? stm.Read(key) : stm.Read(idx);

		switch(what)
		{
		case svtNull:   {                             iskey ? table->SetToNull(key.c_str())           : table->SetNullAt(idx);        break; };
		case svtString: { string s; stm.Read(s); iskey ? table->SetValue(key.c_str(), s.c_str()) : table->SetAt(idx, s.c_str()); break; };
		case svtNumber: { float f = 0;   stm.Read(f); iskey ? table->SetValue(key.c_str(), f)         : table->SetAt(idx, f);         break; }; 
		case svtObject: { _SmartScriptObject t(ss);   iskey ? table->SetValue(key.c_str(), *t)         : table->SetAt(idx, *t);  LoadProperties(t, stm, ss, (char *)key.c_str()); break; }; 
		case svtUserData:
		case svtFunction: TRACE("WARNING: can't restore userdata or function in properties table (%s.%s)", parent, key.c_str()); 
		};
	}; 
};

//////////////////////////////////////////////////////////////////////////
class CCVarSerializeGameSave : public ICVarDumpSink
{
public:
	CCVarSerializeGameSave(CStream *pStm, bool bSave)
	{
		m_pStm=pStm;
		m_bSave=bSave;
		m_nCount=0;
	}

	void OnElementFound(ICVar *pCVar)
	{
		if (m_bSave)
		{		
			m_pStm->Write(pCVar->GetName());
			m_pStm->Write(pCVar->GetString());
		}
		
		m_nCount++;
	}

	int	GetCount() { return(m_nCount); }
	
private:
	CStream *m_pStm;
	bool m_bSave;
	int	m_nCount;
};

// IMPORTANT: DONT FORGET TO UPDATE "SAVEVERSION" DEFINE IN GAME.H IF THE FORMAT CHANGES
//
//////////////////////////////////////////////////////////////////////////
void SaveName(string &s, string &prof)
{
	if(!s[0]) s = "quicksave";
	for(unsigned int i = 0; i<s.size(); i++) if(!isalnum(s[i])) s[i] = '_';
	s = "profiles/player/" + prof + "/savegames/" + s + ".sav";
};

//////////////////////////////////////////////////////////////////////////
bool CXGame::SaveToStream(CStream &stm, Vec3d *pos, Vec3d *angles,string sFilename)
{
	IBitStream *pBitStream = GetIBitStream();

	if(m_bEditor)				 
	{
		m_pLog->Log("Skipping savegame in editor...");
		return false;
	};
 
	IEntitySystem *pEntitySystem=m_pSystem->GetIEntitySystem();

	IEntity *pPlayerEnt=NULL;
	if (m_pClient)
		pPlayerEnt=pEntitySystem->GetEntity(m_pClient->GetPlayerId());
	//ASSERT(pPlayerEnt);
	if (!pPlayerEnt)
		//CryError("Saving to checkpoint - Current player not set or invalid - data error - possible reasons: \n - the first respawn point might be inside a checkpoint \n - something else other than shapes is used to trigger game events \n the trigger used to trigger checkpoint is not trigger once \n solution: check out the map and fix");
		CryError("A checkpoint has been triggered to save data when the player is not existing yet, generally right after respawning. \n This is a data error, and must be fixed by the designer working on this map. \n Possible data errors are: \n - the first respawn point might be inside a checkpoint \n - something else other than shapes is used to trigger game events; \n - the area trigger used to trigger checkpoint is not trigger once; \n how to proceed: get the designer working on this map to fix this");

	CPlayer *pPlayer=NULL;
	if(pPlayerEnt->GetContainer()) pPlayerEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
	//ASSERT(pPlayer);
	if (!pPlayer)
		CryError("Cannot get player container");

	if(pPlayer->m_stats.health<=0)
	{
		m_pLog->Log("Cannot save while player is dead");
		return false;		
	};

	CScriptObjectStream scriptStream;
	scriptStream.Create(m_pScriptSystem);
	scriptStream.Attach(&stm);

	//m_p3DEngine->UnRegisterAllEntities();
	stm.Reset();
 
	// save header
	stm.Write(SAVEMAGIC);
	stm.Write((int)PATCH2_SAVEVERSION);

	// get lowercase versions of levelname and missionname
	char szLowerCaseStr[256] = {0};

	strncpy(szLowerCaseStr, g_LevelName->GetString(), 255);
	strlwr(szLowerCaseStr);
	
	// save levelname
	stm.Write(szLowerCaseStr);

	strncpy(szLowerCaseStr, m_pServer->m_GameContext.strMission.c_str(), 255);
	strlwr(szLowerCaseStr);

	// save mission name
	stm.Write(szLowerCaseStr);

	// write current time and date
	SYSTEMTIME pSystemTime;		
	GetLocalTime(&pSystemTime);	// FIX: this should be moved to crysystem

	stm.Write((unsigned char)pSystemTime.wHour);	// hour
	stm.Write((unsigned char)pSystemTime.wMinute);// minute
	stm.Write((unsigned char)pSystemTime.wSecond);// second
	stm.Write((unsigned char)pSystemTime.wDay);		// day
	stm.Write((unsigned char)pSystemTime.wMonth);	// month
	stm.Write((unsigned short)pSystemTime.wYear);	// year

	// save savegame name
	stm.Write(sFilename);

	// save the number of entities, for the loading screen bar
	int nEnt=pEntitySystem->GetNumEntities();
	stm.Write(nEnt);

	WRITE_COOKIE_NO(stm,0x22);

	// save current EAX preset
	int nPreset;
	CS_REVERB_PROPERTIES tProps;
	m_pSystem->GetISoundSystem()->GetCurrentEaxEnvironment(nPreset,tProps);
	stm.Write(nPreset);
	if (nPreset==-1)
	{	
		stm.WriteBits((BYTE *)&tProps,sizeof(CS_REVERB_PROPERTIES));
	}

	WRITE_COOKIE_NO(stm,0x12);

	// save saveable cvars
	CCVarSerializeGameSave tCount(&stm,false);
	m_pSystem->GetIConsole()->DumpCVars(&tCount,VF_SAVEGAME);
	int nCount=tCount.GetCount(); // get the number of cvars to save
	stm.Write(nCount);  
	CCVarSerializeGameSave t(&stm,true); 
	m_pSystem->GetIConsole()->DumpCVars(&t,VF_SAVEGAME); // save them
	ASSERT(t.GetCount()==nCount);

	//[petar] save autobalancing stuff
	IAISystem *pAISystem = m_pSystem->GetAISystem();
	AIBalanceStats stats;
	pAISystem->GetAutoBalanceInterface()->GetAutobalanceStats(stats);
	stm.Write(stats.nAllowedDeaths);
	/*
	WRITE_COOKIE_NO(stm,0x11);
	// save played cutscenes
	nCount=m_lstPlayedCutScenes.size();
	stm.Write(nCount);  
	for (std::set<string>::iterator it=m_lstPlayedCutScenes.begin();it!=m_lstPlayedCutScenes.end();it++)
	{
		string sName=*it;
		stm.Write(sName);
	} //it
	*/

	IEntityClassRegistry *pECR=GetClassRegistry();
	IEntityItPtr pEntities=pEntitySystem->GetEntityIterator();
	pEntities->MoveFirst();
	
	IEntity *pEnt=NULL; 

	WRITE_COOKIE_NO(stm,0x3c);

	// [kirill] savig IDs of all dynamicaly spawn BUT saved entities
	// so would be able to preserve IDs from being taken by other dynamic entities on loadind
	std::vector<int>	dynSaveableEntities;	
	while((pEnt=pEntities->Next())!=NULL)
	{
		if (!pEnt->IsSaveEnabled()) 
			continue;

		if (pEntitySystem->IsDynamicEntityId( pEnt->GetId() ))
			dynSaveableEntities.push_back( pEnt->GetId() );
	}
	stm.Write((int)dynSaveableEntities.size());
	for(int cntr=0; cntr<(int)(dynSaveableEntities.size()); ++cntr )
	stm.Write((int)dynSaveableEntities[cntr]);

	WRITE_COOKIE_NO(stm,61);

	pEntities=pEntitySystem->GetEntityIterator();
  while((pEnt=pEntities->Next())!=NULL)
	{
		if (!pEnt->IsSaveEnabled()) 
			continue;

		EntityClass *pClass = pECR->GetByClass(pEnt->GetEntityClassName());
		CPlayer *pPlayer = NULL;

		if(pEnt->GetContainer()) pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
		if(pPlayer && pPlayer->m_stats.health<=0) continue;

		//[kirill] 
		// fixme somtimes health of dead players somehow not 0 so let's make sure we don't save dead guy
		if(pPlayer && pEnt->GetPhysics() && pEnt->GetPhysics()->GetType()!=PE_LIVING) 
		{
			m_pSystem->GetILog()->Log("\001 WARNING, dead player [%s] has health %d ", pEnt->GetName(), pPlayer->m_stats.health );
			continue;
		}

		stm.Write((BYTE)CHUNK_ENTITY);

		pBitStream->WriteBitStream(stm,pClass->ClassId,eEntityClassId);

		if(pPlayer && pPlayer->IsMyPlayer()) 
			stm.Write((EntityId)0);		// don't save id for local player - generate it on spawn, to avoid ID's collision
		else
			stm.Write(pEnt->GetId());
		//	the position is saved later
		// with	pEnt->Save(

		//////////////////////////////////////////////////////////////////////////		
		// [marco] the position must be saved before / must be done the same way to
		// be consistent with load level!

		// save relative position if bound
		Vec3d vPos = pEnt->GetPos(!pEnt->IsBound());		
		Vec3d vAng = pEnt->GetAngles(pEnt->IsBound());

		if (!stm.Write(vPos))
			CryError("Error while saving entity %s, id=%d",pEnt->GetName(),(int)pClass->ClassId);
		if (!stm.Write(vAng))
			CryError("Error while saving entity %s, id=%d",pEnt->GetName(),(int)pClass->ClassId);
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// rendering stuff
		
		stm.Write(pEnt->GetScale());
		
		stm.Write(pEnt->GetRndFlags());

		unsigned char uViewDistRatio=(int)(pEnt->GetViewDistRatioNormilized()*100.0f);
		unsigned char uLodRatio=(int)(pEnt->GetLodRatioNormilized()*100.0f);

		stm.Write(uViewDistRatio);
		stm.Write(uLodRatio);

		IMatInfo *pMat=pEnt->GetMaterial();
		string sMat;
		if (pMat)
			sMat=pMat->GetName();			
		stm.Write(sMat);

		stm.Write(pEnt->IsHidden());				
		//////////////////////////////////////////////////////////////////////////

		WRITE_COOKIE_NO(stm,76);

		stm.Write(pEnt->GetName());

		if (m_pLog->GetVerbosityLevel()>5)
			m_pLog->Log("SAVED entity name %s classname %s classid=%d id=%d",pEnt->GetName(),pClass->strClassName.c_str(),(int)pClass->ClassId,pEnt->GetId());

		IScriptObject *so = pEnt->GetScriptObject();
		ASSERT(so);

		WRITE_COOKIE_NO(stm,77);

		stm.AlignWrite();

		_SmartScriptObject props(m_pScriptSystem, true);
		if(so->GetValue("Properties", props)) props->Dump(&PropertyWriter(props, stm, m_pScriptSystem));
		stm.Write((char)TABLE_END);

		_SmartScriptObject propsi(m_pScriptSystem, true);
		if(so->GetValue("PropertiesInstance", propsi)) propsi->Dump(&PropertyWriter(propsi, stm, m_pScriptSystem));
		stm.Write((char)TABLE_END);

		_SmartScriptObject events(m_pScriptSystem, true);
		if(so->GetValue("Events", events)) events->Dump(&PropertyWriter(events, stm, m_pScriptSystem));
		stm.Write((char)TABLE_END);

		WRITE_COOKIE_NO(stm,78);

		WRITE_COOKIE_NO(stm,45);

		if(pPlayer)
			stm.Write(pPlayer->m_stats.health);		// int
		 else
			stm.Write((int)0);										// int

		WRITE_COOKIE_NO(stm,46);

		pEnt->Save(stm,scriptStream.GetScriptObject());

		if(pPlayer) 
		{
			pPlayer->SaveGame(stm);
			if (pEnt->GetAI())
				stm.Write(pEnt->GetAI()->GetName());
		}
		
	}

	stm.Write((BYTE)CHUNK_PLAYER);

	if(pPlayer->IsMyPlayer()) 
		stm.Write((EntityId)0);		// don't save id for local player - generate it on spawn, to avoid ID's collision
	else
		stm.Write(m_pClient->GetPlayerId());

	stm.Write(pPlayer->m_bFirstPerson);

	IEntityCamera *pCam=pPlayerEnt->GetCamera();
	ASSERT(pCam);

	if(pos && angles)
	{
		stm.Write(*pos);
		stm.Write(*angles);        
	}
	else
	{
		//[PETAR] please do  not touch anything that is not clear to you
		//the next lines serialize THE CAMERA POSITION, not the player entity position
		//the entity position is serialized in a normal entity chunk
		stm.Write(pCam->GetPos());
		stm.Write(pCam->GetAngles());
	};



	// now loop through entities again to save their AI state
	// it has to be done here because all entities need to be spawned when AI state is recreated
	pEntities->MoveFirst();
	while((pEnt=pEntities->Next())!=NULL)
	{
		CPlayer *pPlayer=NULL;
		CVehicle *pVehicle=NULL;
		IAIObject *pAIObject = pEnt->GetAI();
		if(pEnt->GetContainer())
		{
			pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
			pEnt->GetContainer()->QueryContainerInterface(CIT_IVEHICLE,(void**) &pVehicle);
		}

		if (!pEnt->IsSaveEnabled())
			continue;

		if (!pPlayer && !pVehicle && !pAIObject)
			continue;

		if (pPlayer)
		{
			if (!pPlayer->IsAlive())
				continue;

//			if (pPlayer->IsMyPlayer())
//				continue;  

			stm.Write((BYTE)CHUNK_AI);		

			// [kirill]
			// for local player ID will be different - it will be spawn
			if(pPlayer->IsMyPlayer()) 
				stm.Write((int)0);			
			else
				stm.Write((int)pEnt->GetId());	// for which player is this
			pPlayer->SaveAIState(stm, scriptStream);
		}

		if (pVehicle)
		{
			stm.Write((BYTE)CHUNK_AI);		
			stm.Write((int)pEnt->GetId());	// for which player is this
			pVehicle->SaveAIState(stm, scriptStream);
		}

		if (!pPlayer && !pVehicle)
		{
			stm.Write((BYTE)CHUNK_AI);		
			stm.Write((int)pEnt->GetId());	// for which player is this
			pAIObject->Save(stm);

			IScriptSystem *pScriptSystem = GetSystem()->GetIScriptSystem();
			HSCRIPTFUNCTION	saveOverallFunction=NULL;
			if( pEnt->GetScriptObject() && pEnt->GetScriptObject()->GetValue("OnSaveOverall", saveOverallFunction) )
			{
				pScriptSystem->BeginCall(saveOverallFunction);
				pScriptSystem->PushFuncParam(pEnt->GetScriptObject());
				pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
				pScriptSystem->EndCall();
			}
		}
		WRITE_COOKIE_NO(stm,13);
	}

	// serialize any playing cutscenes
	
	IMovieSystem *pMovies = m_pSystem->GetIMovieSystem();
	ISequenceIt *pIt = pMovies->GetSequences();
	IAnimSequence *pSeq = pIt->first();
	while (pSeq)
	{

		if (pMovies->IsPlaying(pSeq))
		{
			stm.Write((BYTE)CHUNK_INGAME_SEQUENCE);	
			stm.Write(pSeq->GetName());
			stm.Write(pMovies->GetPlayingTime(pSeq));
		}
		pSeq = pIt->next();
	}
	pIt->Release();

  // save required hud/clientstuff data
  stm.Write((BYTE)CHUNK_HUD);
  if (m_pUIHud)
  {    
    GetScriptSystem()->BeginCall("Hud", "OnSave");
    GetScriptSystem()->PushFuncParam(m_pUIHud->GetScript());
    GetScriptSystem()->PushFuncParam(scriptStream.GetScriptObject());
    GetScriptSystem()->EndCall();
  }
  
  _SmartScriptObject pClientStuff(m_pScriptSystem,true);  
  if(m_pScriptSystem->GetGlobalValue("ClientStuff",pClientStuff))	
  {
    m_pScriptSystem->BeginCall("ClientStuff","OnSave");
    m_pScriptSystem->PushFuncParam(pClientStuff);
    m_pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
    m_pScriptSystem->EndCall();
  }

	return true;
};

//////////////////////////////////////////////////////////////////////////
void CXGame::Save(string sFileName, Vec3d *pos, Vec3d *angles,bool bFirstCheckpoint )
{
	FUNCTION_PROFILER( GetISystem(), PROFILE_GAME );

	//// Saving in Multiplayer only possible on server.
	//if (!GetServer() && IsMultiplayer())
	//	return;

  // No saving in mp
  if(IsMultiplayer())
  {
    m_pLog->Log("Cannot save multiplayer game");
  	return;    
  }

	m_sLastSavedCheckpointFilename = "";

	if (m_bIsLoadingLevelFromFile) 
	{
		m_pLog->Log("\001 [ERROR!!!] CANNOT SAVE WHILE LOADING!!!");
		return;
	}

	if(!m_pClient)
	{
		m_pLog->Log("Cannot save game with no map loaded");
		return;
	};

  // ?
	if(!m_pServer)
	{
		m_pLog->Log("Cannot save multiplayer game");
		return;
	};

	CDefaultStreamAllocator sa;
	CStream stm(3000, &sa);   

	if (SaveToStream(stm, pos, angles,sFileName))
	{
		m_strLastSaveGame = sFileName;
		assert(g_playerprofile);

		if (g_playerprofile->GetString() && strlen(g_playerprofile->GetString()))
		{
			string tmp( g_playerprofile->GetString() );
			SaveName(sFileName, tmp);
		}
		else
		{
			string tmp( "default" );
			SaveName(sFileName, tmp);
		}
		
		m_pLog->LogToConsole("Level saved in %d bytes(%s)", BITS2BYTES(stm.GetSize()), sFileName.c_str());

		// replace / by \ because MakeSureDirectoryPathExists does not work with unix paths
		size_t pos = 1;
		
		for(;;)
		{
			pos = sFileName.find_first_of("/", pos);

			if (pos == string::npos)
			{
				break;
			}

			sFileName.replace(pos, 1, "\\", 1);
			pos+=1;
		}

		if (MakeSureDirectoryPathExists(sFileName.c_str()))
		{
			if(!m_pSystem->WriteCompressedFile((char *)sFileName.c_str(), stm.GetPtr(), stm.GetSize()))
			{
				m_pLog->Log("cannot write savegame to file %s", sFileName.c_str());

				return;
			};
			m_sLastSavedCheckpointFilename = sFileName;

			/*
			//////////////////////////////////////////////////////////////////////////
			// Make screenshot of current location, and save it to the .dds file.
			//////////////////////////////////////////////////////////////////////////
			if (!bFirstCheckpoint)
				m_fTimeToSaveThumbnail = 1.0f; // Save checkpoint thumbnail 1 second from now.
			else
			{
				m_fTimeToSaveThumbnail = 5.0f; // Save checkpoint thumbnail 5 seconds from now.
			}
			//////////////////////////////////////////////////////////////////////////
			*/
		}
	};
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::LoadFromStream(CStream &stm, bool isdemo)
{
	
	if(IsMultiplayer())
	{
		assert(0);
		m_pLog->LogError("ERROR: LoadFromStream IsMultiplayer=true");				// severe problem - stream is different for MP
    return false;
	}

	m_bIsLoadingLevelFromFile = true;
	// [anton] make sure physical world has the most recent IsMultiplayer flag before loading
	m_pSystem->GetIPhysicalWorld()->GetPhysVars()->bMultiplayer = IsMultiplayer() ? 1:0;
		
	CScriptObjectStream scriptStream;
	scriptStream.Create(m_pScriptSystem);
	scriptStream.Attach(&stm);

	string sMagic;
	stm.Read(sMagic);
	if(strcmp(sMagic.c_str(), SAVEMAGIC))
	{
		m_pLog->LogToConsole("ERROR: this is not a valid savegame file");
		m_bIsLoadingLevelFromFile = false;
		return false;
	};

	int nVersion;
	stm.Read(nVersion);

  stm.SetStreamVersion(nVersion);

	switch (nVersion)
	{
		case SAVEVERSION:
			return LoadFromStream_RELEASEVERSION(stm,isdemo,scriptStream);
		case PATCH1_SAVEVERSION:
			return LoadFromStream_PATCH_1(stm,isdemo,scriptStream);
		// add more here as more patches are released
	}

	if(nVersion!=PATCH1_SAVEVERSION && nVersion!=PATCH2_SAVEVERSION)
	{
		m_pLog->LogToConsole("ERROR: savegame file from different version of the game");
		m_bIsLoadingLevelFromFile = false;
		return false;
	};

	IBitStream *pBitStream=GetIBitStream();

	IEntitySystem *pEntitySystem=m_pSystem->GetIEntitySystem();
	IEntityClassRegistry *pECR=GetClassRegistry();
	IEntity *pEnt=NULL;
	CEntityDesc ed;
	int			localPlayerId=0;

	string sLevelName;
	string sMissionName;
	stm.Read(sLevelName);
	stm.Read(sMissionName);

	// read dummy save date and time
	unsigned char bDummy;
	unsigned short wDummy;
	stm.Read(bDummy);	// hour
	stm.Read(bDummy);	// minute
	stm.Read(bDummy);	// second
	stm.Read(bDummy);	// day
	stm.Read(bDummy);	// month
	stm.Read(wDummy);	// year

	// load savegame name
	string sFilename;
	stm.Read(sFilename);

	// load the number of entities, for loading screen bar
	int nEnt;
	stm.Read(nEnt);

	bool bS=IsServer();
	bool bC=IsClient();

	assert(!IsMultiplayer());

	VERIFY_COOKIE_NO(stm,0x22);

	// Load EAX preset
	int nPreset;
	CS_REVERB_PROPERTIES tProps;
	m_pSystem->GetISoundSystem()->GetCurrentEaxEnvironment(nPreset,tProps);
	stm.Read(nPreset);
	if (nPreset==-1)
	{	
		stm.ReadBits((BYTE *)&tProps,sizeof(CS_REVERB_PROPERTIES));
	}

	VERIFY_COOKIE_NO(stm,0x12);

	// load saved cvars
	string varname,val;
	int nCount,i;
	stm.Read(nCount);
	IConsole *pCon=m_pSystem->GetIConsole();	
	for (i=0;i<nCount;i++)
	{
		if(stm.Read(varname))
		if(stm.Read(val))
		{
			ICVar *pCVar=m_pSystem->GetIConsole()->GetCVar(varname.c_str());
			if (!pCVar)
			{
				m_pSystem->GetILog()->Log("\001 WARNING, CVar %s(%s) was saved but is not present",varname.c_str(),val.c_str());
			}
			else
				pCVar->Set(val.c_str());
		}
		else
		{
			m_pSystem->GetILog()->LogError("CXGame::LoadFromStream %d/%d critical error",i,nCount);
			stm.Debug();
			m_bIsLoadingLevelFromFile = false;
			return false;
		}
	} //i

  if(m_pSystem->GetISoundSystem())
    m_pSystem->GetISoundSystem()->Silence();

	m_pSystem->GetISoundSystem()->Mute(true); 

	bool			bLoadBar = false;
	IConsole *pConsole = m_pSystem->GetIConsole();

	assert(pConsole);

	if(stricmp(g_LevelName->GetString(),sLevelName.c_str()) != 0 || !m_pServer || (stricmp(m_pServer->m_GameContext.strMission.c_str(),sMissionName.c_str()) != 0))
	{

		m_pLog->LogToConsole("Loading %s / %s", sLevelName.c_str(), sMissionName.c_str());

		if (m_pServer) 
			ShutdownServer();
		
		if (isdemo)
		{			
			//StartupClient();
			m_pClient->DemoConnect();
			LoadLevelCS( false,sLevelName.c_str(), sMissionName.c_str(), false);
		}
		else
		{		
			GetISystem()->GetIInput()->EnableEventPosting(false);
			//m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			LoadLevelCS(false,sLevelName.c_str(), sMissionName.c_str(), false);
			//m_pClient->Connect("localhost");				
			//m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			GetISystem()->GetIInput()->EnableEventPosting(true);			
		};
	}
	else
	{
		bLoadBar = 1;
		
		string sLoadingScreenTexture = m_currentLevelFolder + "/loadscreen_" + m_currentLevel + ".dds";
		pConsole->SetLoadingImage( sLoadingScreenTexture.c_str() );

		pConsole->Clear();
		pConsole->ResetProgressBar(nEnt + 3);
		pConsole->SetScrollMax(600);
		pConsole->ShowConsole(1);
		DeleteMessage("Switch"); // no switching during loading

		// local player has to exit all areas before starting to delete entities
		IEntity *pIMyPlayer = GetMyPlayer();
		if( pIMyPlayer )
		{
			IEntityContainer *pIContainer = pIMyPlayer->GetContainer();
			if (pIContainer)
			{
				CPlayer *pPlayer = NULL;
				if (pIContainer->QueryContainerInterface(CIT_IPLAYER, (void**)&pPlayer))
					m_XAreaMgr.ExitAllAreas( pPlayer->m_AreaUser );
			}
		}

		m_pSystem->GetI3DEngine()->RestoreTerrainFromDisk();
		m_pSystem->GetIMovieSystem()->Reset( false );
		m_pLog->Log("REMOVING entities:");
		IEntityItPtr pEntities=pEntitySystem->GetEntityIterator();

		pEntities->MoveFirst();
		IEntity *pEnt=NULL;
		while((pEnt=pEntities->Next())!=NULL)
		{
			EntityClass *pClass=pECR->GetByClass(pEnt->GetEntityClassName());
			if (m_pWeaponSystemEx->IsProjectileClass(pClass->ClassId)) continue;
#ifdef _DEBUG
			m_pLog->Log("REMOVING entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,pEnt->GetId());
#endif
			pEntitySystem->RemoveEntity(pEnt->GetId());		
		}

		pConsole->TickProgressBar();	// advance progress

		pEntitySystem->Update();
		
		SoftReset();
		m_pEntitySystem->Reset();

		pConsole->TickProgressBar();	// advance progress
	}
	
	// PETAR lets delete all guys since they will be spawned anyway
	m_pSystem->GetAISystem()->Reset();	

	IAISystem *pAISystem = m_pSystem->GetAISystem();

	// adaptive balancing
	if (pAISystem)
	{
		if (cv_game_Difficulty->GetIVal())
		{
			float fAcc,fAgg,fHealth;
			if (pAISystem->GetAutoBalanceInterface())
			{
				pAISystem->GetAutoBalanceInterface()->GetMultipliers(fAcc,fAgg,fHealth);
				cv_game_Aggression->Set(fAgg);
				cv_game_Accuracy->Set(fAcc);
				cv_game_Health->Set(fHealth);
			}
		}
	}


	//[petar] load autobalancing stuff
	int nAllowedDeaths;
	stm.Read(nAllowedDeaths);
	pAISystem->GetAutoBalanceInterface()->SetAllowedDeathCount(nAllowedDeaths);


	// apparently these 20 updates are needed to setup everything,
	// from hud to netwrok stream etc. do not remove it
	// or savegame won't load
	for (int i = 0; i<20; i++) 
		Update();

	if (bLoadBar)
	{
		pConsole->TickProgressBar();	// advance progress
	}
	
	VERIFY_COOKIE_NO(stm,0x3c);

	// loading reserver IDs for dynacally created saved entities
	int dynReservedIDsNumber=0;
	stm.Read(dynReservedIDsNumber);
	for( ; dynReservedIDsNumber>0; --dynReservedIDsNumber )
	{
	int reservedId;
		stm.Read((int&)reservedId);
		pEntitySystem->MarkId( reservedId );
	}

  // only load this in case of older save
  if(nVersion<PATCH2_SAVEVERSION)
  {
    VERIFY_COOKIE_NO(stm,0x73);

    // load hud objectives
    if (m_pUIHud)
    {
      GetScriptSystem()->BeginCall("Hud", "OnLoadOld");
      GetScriptSystem()->PushFuncParam(m_pUIHud->GetScript());
      GetScriptSystem()->PushFuncParam(scriptStream.GetScriptObject());
      GetScriptSystem()->EndCall();
    }
  }

	VERIFY_COOKIE_NO(stm,61);

	while (!stm.EOS())
	{
		BYTE cChunk=0;
		stm.Read(cChunk);
		switch(cChunk)
		{
		case CHUNK_ENTITY:
			{
				EntityId id;
				EntityClassId ClassID;	
				
				pBitStream->ReadBitStream(stm,ClassID,eEntityClassId);

				EntityClass *pClass=pECR->GetByClassId(ClassID);
				ASSERT(pClass);
				ed.className=pClass->strClassName;
				ed.ClassId=pClass->ClassId;
				
				stm.Read(id);
				ed.id=id;
				
				// [kirill] if this entity was dynamicly created - ID was marked when dynReservedIDsNumber loaded, to prevent 
				// from being taked by some other dynamically spawned non-saved entity. So now we load it and let's free the id
				if (pEntitySystem->IsDynamicEntityId( id ))
					pEntitySystem->ClearId(id);

				// position and angles are read later - 
				// with pEnt->Load(
				//////////////////////////////////////////////////////////////////////////				
				//[marco] position and angles must be read before spawining the entity - must
				// be consistent with load level!								
				Vec3d vPos,vAngles;
				if (!stm.Read(vPos))
					CryError("Error while reading position for entity id=%d",id);
				if (!stm.Read(vAngles))
					CryError("Error while reading position for entity id=%d",id);
				ed.pos=vPos;
				ed.angles=vAngles;				

				//////////////////////////////////////////////////////////////////////////				
				// renderer stuff

				float fScale;
				stm.Read(fScale);

				int dwRendFlags;
				stm.Read(dwRendFlags);
				unsigned char uViewDistRatio;
				stm.Read(uViewDistRatio);
				unsigned char uLodRatio;
				stm.Read(uLodRatio);

				string MatName;
				stm.Read(MatName);
				
				bool bHidden=false;
				stm.Read(bHidden);

				//////////////////////////////////////////////////////////////////////////
				VERIFY_COOKIE_NO(stm,76);

				string name;
				stm.Read(name);

				if (m_pLog->GetVerbosityLevel()>=5)
					m_pLog->Log("LOADED entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,id);

				//////////////////////////////////////////////////////////////////////////

				VERIFY_COOKIE_NO(stm,77);

				stm.AlignRead(); 

				_SmartScriptObject props(m_pScriptSystem);
				LoadProperties(props, stm, m_pScriptSystem, "_root");

				_SmartScriptObject propsi(m_pScriptSystem);
				LoadProperties(propsi, stm, m_pScriptSystem, "_root");

				_SmartScriptObject events(m_pScriptSystem);
				LoadProperties(events, stm, m_pScriptSystem, "_root");
				
				VERIFY_COOKIE_NO(stm,78);

				//m_pScriptSystem->BeginCall("dump");
				//m_pScriptSystem->PushFuncParam(props);
				//m_pScriptSystem->EndCall();

				//ASSERT(!pEntitySystem->GetEntity(id));    // entity already exists in map
				//IEntity* dbgEnt=pEntitySystem->GetEntity(id);

				if (pEntitySystem->GetEntity(id))
				{
					CryError("entity classname %s classid=%d id=%d ALREADY EXISTING IN THE MAP",pClass->strClassName.c_str(),(int)pClass->ClassId,id);
				}

				ed.pProperties=props;
				ed.pPropertiesInstance=propsi;
				ed.name = name;
				pEnt=pEntitySystem->SpawnEntity(ed);
				//ASSERT(pEnt);
				if (!pEnt)
					CryError("entity classname %s classid=%02d id=%3id CANNOT BE SPAWNED",pClass->strClassName.c_str(),(int)pClass->ClassId,id);

				if(id==0)		// it's a local player
					localPlayerId = ed.id;

				//////////////////////////////////////////////////////////////////////////
				// render flags
				pEnt->SetRndFlags(dwRendFlags);
				pEnt->SetScale(fScale);
				pEnt->SetViewDistRatio(uViewDistRatio);
				pEnt->SetLodRatio(uLodRatio);

				if (!MatName.empty())
				{
					IMatInfo *pMtl = GetSystem()->GetI3DEngine()->FindMaterial(MatName.c_str());
					if (pMtl)					
						pEnt->SetMaterial(pMtl);					
				}
				
				pEnt->Hide(bHidden);

				//////////////////////////////////////////////////////////////////////////				

				///pEnt->SetName(name.c_str());

				IScriptObject *so = pEnt->GetScriptObject();
				ASSERT(so);

				//so->SetValue("Properties", props);
				//so->SetValue("PropertiesInstance", propsi);
				so->SetValue("Events", events);

				CPlayer *pPlayer=NULL;
				if(pEnt->GetContainer()) pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);


				VERIFY_COOKIE_NO(stm,45);

				int health = 0;
				stm.Read(health);

				VERIFY_COOKIE_NO(stm,46);

				if(pPlayer)
				{
					if(health<=0)
					{                    
						pEnt->GetCharInterface()->KillCharacter(0);
#ifdef _DEBUG
						m_pLog->Log("DEAD entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,pEnt->GetId());
#endif
						//pEntitySystem->RemoveEntity(pEnt->GetId());		
					};
				};

				bool b = pEnt->Load(stm,scriptStream.GetScriptObject());
				if (!b)
					CryError("entity classname %s classid=%02d id=%3id CANNOT BE LOADED",pClass->strClassName.c_str(),(int)pClass->ClassId,id);
				//ASSERT(b);

				// [anton] proper state serialization was absent BasicEntity.lua,
				// we'll have to at least make sure that activated rigid bodies that were initially not active 
				// don't load velocity from active state
				IPhysicalEntity *pPhys = pEnt->GetPhysics();
				pe_status_dynamics sd;
				if (nVersion<=PATCH1_SAVEVERSION && pPhys && pPhys->GetType()==PE_RIGID && pPhys->GetStatus(&sd) && sd.mass==0)
				{
					pe_action_set_velocity asv;
					asv.v.Set(0,0,0); asv.w.Set(0,0,0);
					pPhys->Action(&asv);
				}

				// update position if ended up under terrain and outdoors

				if(pPlayer)
				{

					if(id==0)
					{
						I3DEngine *pEngine = m_pSystem->GetI3DEngine();
						if (pEngine && pAISystem)
						{

							Vec3d pos = pEnt->GetPos();
							IVisArea *pArea;
							int nBuildingid;
							if (!pAISystem->CheckInside(pos,nBuildingid,pArea))
							{
								float newz = pEngine->GetTerrainElevation(pos.x,pos.y);
								if (newz>pos.z)
									pos.z=newz+0.1f;
								pEnt->SetPos(pos);
							}
						}

					}

//[kirill] the OnReset called from OnInit which is called when entity is spawned
//					if(health>0)
//					{
//						// call on reset for the spawned entity
//						m_pScriptSystem->BeginCall(pEnt->GetEntityClassName(),"OnReset");
//						m_pScriptSystem->PushFuncParam(pEnt->GetScriptObject());
//						m_pScriptSystem->EndCall();
//					};

					pPlayer->LoadGame(stm);
					if (pEnt->GetAI())
					{
						char sName[255];
						stm.Read(sName,255);
						pEnt->GetAI()->SetName(sName);
					}
				};
				break;
			}

		case CHUNK_PLAYER:
			{
				EntityId wPlayerID = 0;
				bool b = stm.Read(wPlayerID);
				ASSERT(b);

				if(wPlayerID==0)				// we write 0 for localplayer on save
					wPlayerID = localPlayerId;
				m_pClient->SetPlayerID(wPlayerID);

				pEnt=pEntitySystem->GetEntity(wPlayerID);
//				if( wPlayerID==0 )
//					pEnt=m_pClient->m_pISystem->GetLocalPlayer();
				if (!pEnt)
					CryError("player id=%3id NOT FOUND",wPlayerID);
				//ASSERT(pEnt);

				CPlayer *pPlayer=NULL;
				if(pEnt->GetContainer()) pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
				ASSERT(pPlayer);
				stm.Read(pPlayer->m_bFirstPerson);
				SetViewMode(!pPlayer->m_bFirstPerson); // hack, but works

				/* [kirill] moved this to int CScriptObjectGame::TouchCheckPoint(IFunctionHandler *pH)
					needed to fix quickLoad restoreHealth problem 
				// do we want to overwrite health with half of maximum
				if(p_restorehalfhealth->GetIVal())
				{
					pPlayer->m_stats.health = 255;	// [marco] danger! this should be set by
													// gamerules but it gets overwritten by the save checkpoint

					//m_pSystem->GetILog()->Log("player health=%d",pPlayer->m_stats.health);
					// [kirill]
					// this was requested by UBI. It's expected here that current health value is the maximum 
					//Everytime Jack dies he should respawn with half of his hit points instead of full health. 
					//Same mechanics for Val, she should get half her hit points everytime Jack respawns.
					pPlayer->m_stats.health/=2;
				}

				if (pPlayer->m_stats.health<128)
					pPlayer->m_stats.health = 128;
				*/
				IEntityCamera *pCam=pEnt->GetCamera();
				ASSERT(pCam);

				Vec3d vPos;
				stm.Read(vPos);
				pCam->SetPos(vPos);
				//pEnt->SetPos(vPos);

				m_pLog->Log("PLAYER %d (%f %f %f) ", wPlayerID, vPos.x, vPos.y, vPos.z);

				Vec3d vAngles;
				stm.Read(vAngles);
				pCam->SetAngles(vAngles);
				//[kirill]
				//why do we do it here? entity angles are set when entity is loaded
				//pEnt->SetAngles(vAngles);
				m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(vAngles);

				GetSystem()->SetViewCamera( pCam->GetCamera() );

				GetSystem()->GetISoundSystem()->SetListener(pCam->GetCamera(),Vec3(0,0,0));

				if(!isdemo)
				{
					CXServer::XSlotMap::iterator itor;
					ASSERT(m_pServer->GetSlotsMap().size()==1);
					itor = m_pServer->GetSlotsMap().begin();

					CXServerSlot *pSSlot=itor->second;							// serverslot associated with the player
					
					pSSlot->SetPlayerID(wPlayerID);
					pSSlot->SetGameState(CGS_INPROGRESS);						// start game imediatly
				};
				break;
			}

		case CHUNK_AI:
			{				
				// find for which entity this chunk is
				int nID;
				stm.Read(nID);

				IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
				if ( nID == 0 )	// it's local player
					pEntity = m_pClient->m_pISystem->GetLocalPlayer();
				if (pEntity)
				{
					m_pLog->Log("Now loading AICHUNK for entity %s", pEntity->GetName());
					CPlayer *pPlayer=0;
					CVehicle *pVehicle=0;
					if (pEntity->GetContainer())
					{
						if (pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
							pPlayer->LoadAIState(stm, scriptStream);
						if (pEntity->GetContainer()->QueryContainerInterface(CIT_IVEHICLE,(void**)&pVehicle))
							pVehicle->LoadAIState(stm, scriptStream);
					}
					else if( pEntity->GetAI() )
					{
						pEntity->GetAI()->Load(stm);
						IScriptSystem *pScriptSystem = GetSystem()->GetIScriptSystem();
						HSCRIPTFUNCTION	loadOverallFunction=NULL;
						if( pEntity->GetScriptObject() && pEntity->GetScriptObject()->GetValue("OnLoadOverall", loadOverallFunction) )
						{
							pScriptSystem->BeginCall(loadOverallFunction);
							pScriptSystem->PushFuncParam(pEntity->GetScriptObject());
							pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
							pScriptSystem->EndCall();
						}
					}
				}

				VERIFY_COOKIE_NO(stm,13);
			}
			break;
		case CHUNK_INGAME_SEQUENCE:
			{
#if !defined(LINUX)	
				IMovieSystem *pMovies = m_pSystem->GetIMovieSystem();
				char szName[1024];
				stm.Read(szName,1024);
				float fTime;
				stm.Read(fTime);
				IAnimSequence *pSeq = pMovies->FindSequence(szName);
				pMovies->PlaySequence(pSeq,false);
				pMovies->SetPlayingTime(pSeq,fTime);
#endif
			}
			break;
    case CHUNK_HUD:
      {
        // load hud/clientstuff viewlayers data
        if (m_pUIHud)
        {
          GetScriptSystem()->BeginCall("Hud", "OnLoad");
          GetScriptSystem()->PushFuncParam(m_pUIHud->GetScript());
          GetScriptSystem()->PushFuncParam(scriptStream.GetScriptObject());
          GetScriptSystem()->EndCall();
        }

        _SmartScriptObject pClientStuff(m_pScriptSystem,true);  
        if(m_pScriptSystem->GetGlobalValue("ClientStuff",pClientStuff))	
        {
          m_pScriptSystem->BeginCall("ClientStuff","OnLoad");
          m_pScriptSystem->PushFuncParam(pClientStuff);
          m_pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
          m_pScriptSystem->EndCall();
        }
      }
      break;  

		default:
			ASSERT(0);
		};

		if (bLoadBar)
		{
			pConsole->TickProgressBar();	// advance progress
		}
	}

	{	// [Anton] - allow entities to restore pointer links between them during post load step 
		// [kirill]	restore all the bindings
		IEntityItPtr pEntities=pEntitySystem->GetEntityIterator();

		pEntities->MoveFirst();
		IEntity *pEnt=NULL;
		while((pEnt=pEntities->Next())!=NULL)
			pEnt->PostLoad();		
	}

	pEntitySystem->Update();
	m_pSystem->GetIMovieSystem()->PlayOnLoadSequences();	// yes, we reset this twice, the first time to remove all entity-pointers and now to restore them
	m_pClient->Reset();
	
	m_bIsLoadingLevelFromFile = false;
	m_pSystem->GetISoundSystem()->Mute(false); 

	m_bMapLoadedFromCheckpoint=true;

	
	m_pEntitySystem->PauseTimers(false,true);	

	//	m_pLog->Log("HIDE CONSOLE");
	m_pRenderer->ClearColorBuffer(Vec3(0,0,0));
	m_pSystem->GetIConsole()->ResetProgressBar(0);
	m_pSystem->GetIConsole()->ShowConsole(false);
	m_pSystem->GetIConsole()->SetScrollMax(600/2);

	if (nPreset!=-1)
		m_pSystem->GetISoundSystem()->SetEaxListenerEnvironment(nPreset,NULL);
	else
		m_pSystem->GetISoundSystem()->SetEaxListenerEnvironment(nPreset,&tProps);

	GotoGame(1);
	m_nDEBUG_TIMING = 0;

	return true;
};

//////////////////////////////////////////////////////////////////////////
bool CXGame::Load(string sFileName)
{
	m_pSystem->VTuneResume();

	assert(g_playerprofile);
	
	string tmp( g_playerprofile->GetString() );
	SaveName(sFileName, tmp);

	CDefaultStreamAllocator sa;
	CStream stm(300, &sa); 

	int bitslen=m_pSystem->GetCompressedFileSize((char *)sFileName.c_str());
	if(bitslen==0) 
	{
		return false;
	}

	IInput *pInput=GetSystem()->GetIInput();
	if(pInput)
		pInput->SetMouseExclusive(false);

	stm.Resize(bitslen);
	int bitsread = m_pSystem->ReadCompressedFile((char *)sFileName.c_str(), stm.GetPtr(), stm.GetAllocatedSize());
	if(!bitsread)
	{
		m_pLog->Log("No such savegame: %s", sFileName.c_str());
		if(pInput)
			pInput->SetMouseExclusive(true);

		return false;
	};

	stm.SetSize(bitsread);

	if (m_pUISystem)
	{
		m_pUISystem->StopAllVideo();
	}

	//////////////////////////////////////////////////////////////////////////
	// Lock resources before loading checkpoint.
	// Speed ups loading a lot.
	m_pSystem->GetI3DEngine()->LockCGFResources();
	m_pSystem->GetIAnimationSystem()->LockResources();
	m_pSystem->GetISoundSystem()->LockResources();
	//////////////////////////////////////////////////////////////////////////

	bool ok = LoadFromStream(stm, false);

	//////////////////////////////////////////////////////////////////////////
	// Unlock resources after loading checkpoint.
	// Some uneeded resources that were locked before may get released here.
	m_pSystem->GetISoundSystem()->UnlockResources();
	m_pSystem->GetIAnimationSystem()->UnlockResources();
	m_pSystem->GetI3DEngine()->UnlockCGFResources();
	//////////////////////////////////////////////////////////////////////////


	if(ok)
		m_strLastSaveGame = sFileName;

	if(pInput)
		pInput->SetMouseExclusive(true);

	m_pSystem->VTunePause();

	//pInput->GetIKeyboard()->ClearKeyState();

	if (!IsMultiplayer())
	{
		_SmartScriptObject pMissionScript(m_pScriptSystem);
		m_pScriptSystem->GetGlobalValue("Mission", pMissionScript);

		if (((IScriptObject *)pMissionScript) != 0)
		{
			HSCRIPTFUNCTION pfnOnCheckpointLoaded = 0;

			pMissionScript->GetValue("OnCheckpointLoaded", pfnOnCheckpointLoaded);

			if (pfnOnCheckpointLoaded)
			{
				m_pScriptSystem->BeginCall(pfnOnCheckpointLoaded);
				m_pScriptSystem->PushFuncParam((IScriptObject*)pMissionScript);
				m_pScriptSystem->EndCall();

				m_pScriptSystem->ReleaseFunc(pfnOnCheckpointLoaded);
			}
		}
	}

	AllowQuicksave(true);
	return ok;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::LoadLatest()
{
	if(!m_strLastSaveGame.empty())
	{
		Load(m_strLastSaveGame);
		m_pServer->m_pISystem->BindChildren();
		//m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
	}
}; 

//////////////////////////////////////////////////////////////////////////
class CCVarSaveDump : public ICVarDumpSink
{
private:
	FILE *m_pFile;
public:
	CCVarSaveDump(FILE *pFile)
	{
		m_pFile=pFile;
	}
	virtual void OnElementFound(ICVar *pCVar)
	{
		if (pCVar && (pCVar->GetFlags() & VF_DUMPTODISK))
		{
			string szValue = pCVar->GetString();
			int pos;

			// replace \ with \\ 
			pos = 1;
			for(;;)
			{
				pos = szValue.find_first_of("\\", pos);

				if (pos == string::npos)
				{
					break;
				}

				szValue.replace(pos, 1, "\\\\", 2);
				pos+=2;
			}

			// replace " with \" 
			pos = 1;
			for(;;)
			{
				pos = szValue.find_first_of("\"", pos);

				if (pos == string::npos)
				{
					break;
				}

				szValue.replace(pos, 1, "\\\"", 2);
				pos+=2;
			}

			string szLine = pCVar->GetName();
			szLine += " = \"";
			szLine += szValue;
			szLine += "\"\r\n";

			fputs(szLine.c_str(), m_pFile);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
class CActionMapDumpSink : public IActionMapDumpSink
{
private:
	CXGame *m_pGame;
	FILE *m_pFile;
public:
	CActionMapDumpSink(CXGame *pGame, FILE *pFile)
	{
		m_pGame=pGame;
		m_pFile=pFile;
		fputs("Input:ResetAllBindings();\r\n", m_pFile);
	}
	virtual void OnElementFound(const char *pszActionMapName, IActionMap *pActionMap)
	{
		char pszKey[256];
		char pszMod[256];
		ActionsEnumMap &ActionsMap=m_pGame->GetActionsEnumMap();
		ActionsEnumMapItor It=ActionsMap.begin();
		while (It!=ActionsMap.end())
		{
			ActionInfo &Info=It->second;
			for (int i=0;i<MAX_BINDS_PER_ACTION;i++)
			{
				pActionMap->GetBinding(Info.nId, i, pszKey, pszMod);

				if (strlen(pszKey))
				{
					if (strcmp(pszKey, "\\") == 0)
					{
						strcpy(pszKey, "\\\\");
					}
					else if (strcmp(pszKey, "\"") == 0)
					{
						strcpy(pszKey, "\\\"");
					}

					char szLine[1024] = {0};

					sprintf(szLine, "Input:BindAction(\"%s\", \"%s\", \"%s\", %d);\r\n", It->first.c_str(), pszKey, pszActionMapName, i);
					fputs(szLine, m_pFile);
				}
			}
			++It;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
void CXGame::SaveConfiguration( const char *pszSystemCfg,const char *pszGameCfg,const char *sProfileName)
{
	if(m_pSystem->IsQuitting())
	{
		// shutdown the client if there is one (SERVERSYNC vars get restored and SaveConfiguration() can save the right config)
		ShutdownClient();
	}
	else
	{
		 if(IsClient() && !IsServer())		// only a client
			 assert(0);			// shouldn't be - saving config while connected to a client means you save the server synced variables of the server
	}

	string sSystemCfg = pszSystemCfg;
	string sGameCfg = pszGameCfg;
	if (sProfileName)
	{	
		sSystemCfg=string("Profiles/Player/")+sProfileName+"_"+sSystemCfg;
		sGameCfg=string("Profiles/Player/")+sProfileName+"_"+sGameCfg;		
	}

	FILE *pFile=fxopen(sSystemCfg.c_str(), "wb");
	if (pFile)
	{
		fputs("-- [System-Configuration]\r\n", pFile);
		fputs("-- Attention: This file is generated by the system, do not modify! Editing is not recommended! \r\n\r\n", pFile);
		CCVarSaveDump SaveDump(pFile);
		m_pSystem->GetIConsole()->DumpCVars(&SaveDump);
		//m_pConsole->DumpCVars(&SaveDump);
		fclose(pFile); 
	}

	if (m_pIActionMapManager)
	{
		pFile=fxopen(sGameCfg.c_str(), "wb");
		if (pFile)
		{
			fputs("-- [Game-Configuration]\r\n", pFile);
			fputs("-- Attention: This file will be overwritten when updated, so dont add lines ! Editing is not recommended !\r\n\r\n", pFile);
			CActionMapDumpSink SaveActionMaps(this, pFile);
			m_pIActionMapManager->GetActionMaps(&SaveActionMaps);
			char sValue[32];
			sprintf(sValue, "%4.4f", m_pSystem->GetIInput()->GetIMouse()->GetSensitvity());
			fputs(string(string("Input:SetMouseSensitivity(")+string(sValue)+string(");\r\n")).c_str(), pFile);
			m_pIActionMapManager->GetInvertedMouse() ? strcpy(sValue, "1") : strcpy(sValue, "nil");
			fputs(string(string("Input:SetInvertedMouse(")+string(sValue)+string(");\r\n")).c_str(), pFile);
			//Input:BindCommandToKey("#System:ShowDebugger();", "f8", 1);
			//fputs("Input:BindCommandToKey(\"#System:ShowDebugger();\", \"f8\", 1);\r\n" ,pFile);
			//Input:BindCommandToKey("\\SkipCutScene","F7",1);			
			fputs("Input:BindCommandToKey(\"\\\\SkipCutScene\",\"F7\",1);\r\n",pFile);
			fputs("Input:BindCommandToKey(\"\\\\SkipCutScene\",\"spacebar\",1);\r\n",pFile);
			fclose(pFile); 
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::LoadConfiguration(const string &sSystemCfg,const string &sGameCfg)
{			
	m_pSystem->LoadConfiguration(sSystemCfg);

	FILE *pFile=fxopen(sGameCfg.c_str(), "rb");
	if (!pFile)
	{
		// if for some reason the game config is not found 
		// (first time, new installation etc.)
		char szBuffer[512];
		strcpy(szBuffer,"Input:BindCommandToKey(\"\\\\SkipCutScene\",\"F7\",1);");
		m_pSystem->GetIScriptSystem()->ExecuteBuffer(szBuffer,strlen(szBuffer));
		strcpy(szBuffer,"Input:BindCommandToKey(\"\\\\SkipCutScene\",\"spacebar\",1);");
		m_pSystem->GetIScriptSystem()->ExecuteBuffer(szBuffer,strlen(szBuffer));
		return;
	}
	
	char szLine[512];
	char szBuffer[512];
	while (fgets(szLine,512,pFile))
	{			
		// skip comments
		if (szLine[0]=='-')
			continue; 

		// extract command
		if (!strstr(szLine,";"))
			continue;

		// check for malicious commands
		bool bValid=false;
				
		if (strstr(szLine,"#"))
		{
			// someone is trying to bind script code
			// to a key - silently skip this line
			continue;
		}
		else
		if (strstr(szLine,"Input:ResetAllBindings"))
		{
			// valid command
			bValid=true;
		}
		else
		if (strstr(szLine,"Input:BindAction"))
		{
			// valid command
			bValid=true;
		}
		else
		if (strstr(szLine,"Input:SetMouseSensitivity"))
		{
			// valid command
			bValid=true;
		}					
		else
		if (strstr(szLine,"Input:SetInvertedMouse"))
		{
			// valid command
			bValid=true;
		}					
		else
		if (strstr(szLine,"Input:BindCommandToKey"))
		{
			//if (strstr(szLine,"SkipCutScene"))											
			// valid command
			bValid=true;
		}					

		if (bValid)
		{					
			strcpy(szBuffer,szLine);
			m_pSystem->GetIScriptSystem()->ExecuteBuffer(szBuffer,strlen(szBuffer));
		}
		else
		{
			m_pSystem->GetILog()->Log("Invalid game cfg:%s",szLine);
		}
	}

	fclose(pFile);

	//m_pScriptSystem->ExecuteFile(sGameCfg.c_str(), false);
	//m_pScriptSystem->ExecuteFile("GameCfgOverride.Lua",false); //?
} 


//////////////////////////////////////////////////////////////////////////
void CXGame::RemoveConfiguration(string &sSystemCfg,string &sGameCfg,const char *sProfileName)
{
	if (sProfileName)
	{	
		sSystemCfg=string("Profiles/Player/")+sProfileName+"_"+sSystemCfg;
		sGameCfg=string("Profiles/Player/")+sProfileName+"_"+sGameCfg;
	}
	
#if defined(LINUX)
	remove( sSystemCfg.c_str() ); 
	remove( sGameCfg.c_str() );
#else
	DeleteFile(sSystemCfg.c_str());
	DeleteFile(sGameCfg.c_str());
#endif

	// remove the folder
	string path = "profiles/player/";
	path += sProfileName;
	path += "/";

	m_pSystem->Deltree(path.c_str(), 1);
}

//////////////////////////////////////////////////////////////////////////
//[PETAR]
// DO NOT UPDATE THIS FUNCTION ITS HERE TO ENABLE THAT ALL NEWER VERSIONS
// SUPPORT LOADING OLDER SAVEFILES
bool CXGame::LoadFromStream_RELEASEVERSION(CStream &stm, bool isdemo, CScriptObjectStream &scriptStream)
{
	IBitStream *pBitStream=GetIBitStream();
	IEntitySystem *pEntitySystem=m_pSystem->GetIEntitySystem();
	IEntityClassRegistry *pECR=GetClassRegistry();
	IEntity *pEnt=NULL;
	CEntityDesc ed;
	int			localPlayerId=0;

	string sLevelName;
	string sMissionName;
	stm.Read(sLevelName);
	stm.Read(sMissionName);

	// read dummy save date and time
	unsigned char bDummy;
	unsigned short wDummy;
	stm.Read(bDummy);	// hour
	stm.Read(bDummy);	// minute
	stm.Read(bDummy);	// second
	stm.Read(bDummy);	// day
	stm.Read(bDummy);	// month
	stm.Read(wDummy);	// year

	// load savegame name
	string sFilename;
	stm.Read(sFilename);

	// load the number of entities, for loading screen bar
	int nEnt;
	stm.Read(nEnt);

	bool bS=IsServer();
	bool bC=IsClient();

	assert(!IsMultiplayer());

	VERIFY_COOKIE_NO(stm,0x22);

	// Load EAX preset
	int nPreset;
	CS_REVERB_PROPERTIES tProps;
	m_pSystem->GetISoundSystem()->GetCurrentEaxEnvironment(nPreset,tProps);
	stm.Read(nPreset);
	if (nPreset==-1)
	{	
		stm.ReadBits((BYTE *)&tProps,sizeof(CS_REVERB_PROPERTIES));
	}

	VERIFY_COOKIE_NO(stm,0x12);

	// load saved cvars
	string varname,val;
	int nCount,i;
	stm.Read(nCount);
	IConsole *pCon=m_pSystem->GetIConsole();	
	for (i=0;i<nCount;i++)
	{
		if(stm.Read(varname))
		if(stm.Read(val))
		{
			ICVar *pCVar=m_pSystem->GetIConsole()->GetCVar(varname.c_str());
			if (!pCVar)
			{
				m_pSystem->GetILog()->Log("\001 WARNING, CVar %s(%s) was saved but is not present",varname.c_str(),val.c_str());
			}
			else
				pCVar->Set(val.c_str());
		}
		else
		{
			m_pSystem->GetILog()->LogError("CXGame::LoadFromStream %d/%d critical error",i,nCount);
			stm.Debug();
			m_bIsLoadingLevelFromFile = false;
			return false;
		}
	} //i

  if(m_pSystem->GetISoundSystem())
    m_pSystem->GetISoundSystem()->Silence();

	m_pSystem->GetISoundSystem()->Mute(true); 

	bool			bLoadBar = false;
	IConsole *pConsole = m_pSystem->GetIConsole();

	assert(pConsole);

	if(stricmp(g_LevelName->GetString(),sLevelName.c_str()) != 0 || !m_pServer || (stricmp(m_pServer->m_GameContext.strMission.c_str(),sMissionName.c_str()) != 0))
	{

		m_pLog->LogToConsole("Loading %s / %s", sLevelName.c_str(), sMissionName.c_str());

		if (m_pServer) 
			ShutdownServer();
		
		if (isdemo)
		{			
			//StartupClient();
			m_pClient->DemoConnect();
			LoadLevelCS( false,sLevelName.c_str(), sMissionName.c_str(), false);
		}
		else
		{
			GetISystem()->GetIInput()->EnableEventPosting(false);
			m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			LoadLevelCS(false,sLevelName.c_str(), sMissionName.c_str(), false);
			//m_pClient->Connect("localhost");				
			m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			GetISystem()->GetIInput()->EnableEventPosting(true);			
		};
	}
	else
	{
		bLoadBar = 1;
		
		string sLoadingScreenTexture = m_currentLevelFolder + "/loadscreen_" + m_currentLevel + ".dds";
		pConsole->SetLoadingImage( sLoadingScreenTexture.c_str() );

		pConsole->Clear();
		pConsole->ResetProgressBar(nEnt + 3);
		pConsole->SetScrollMax(600);
		pConsole->ShowConsole(1);
		DeleteMessage("Switch"); // no switching during loading

		// local player has to exit all areas before starting to delete entities
		IEntity *pIMyPlayer = GetMyPlayer();
		if( pIMyPlayer )
		{
			IEntityContainer *pIContainer = pIMyPlayer->GetContainer();
			if (pIContainer)
			{
				CPlayer *pPlayer = NULL;
				if (pIContainer->QueryContainerInterface(CIT_IPLAYER, (void**)&pPlayer))
					m_XAreaMgr.ExitAllAreas( pPlayer->m_AreaUser );
			}
		}

		m_pSystem->GetI3DEngine()->RestoreTerrainFromDisk();
		m_pSystem->GetIMovieSystem()->Reset( false );
		m_pLog->Log("REMOVING entities:");
		IEntityItPtr pEntities=pEntitySystem->GetEntityIterator();

		pEntities->MoveFirst();
		IEntity *pEnt=NULL;
		while((pEnt=pEntities->Next())!=NULL)
		{
			EntityClass *pClass=pECR->GetByClass(pEnt->GetEntityClassName());
			if (m_pWeaponSystemEx->IsProjectileClass(pClass->ClassId)) continue;
#ifdef _DEBUG
			m_pLog->Log("REMOVING entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,pEnt->GetId());
#endif
			pEntitySystem->RemoveEntity(pEnt->GetId());		
		}

		pConsole->TickProgressBar();	// advance progress

		pEntitySystem->Update();
		
		SoftReset();
		m_pEntitySystem->Reset();

		pConsole->TickProgressBar();	// advance progress
	}
	
	// PETAR lets delete all guys since they will be spawned anyway
	m_pSystem->GetAISystem()->Reset();	

	IAISystem *pAISystem = m_pSystem->GetAISystem();

	// adaptive balancing
	if (pAISystem)
	{
		if (cv_game_Difficulty->GetIVal())
		{
			float fAcc,fAgg,fHealth;
			if (pAISystem->GetAutoBalanceInterface())
			{
				pAISystem->GetAutoBalanceInterface()->GetMultipliers(fAcc,fAgg,fHealth);
				cv_game_Aggression->Set(fAgg);
				cv_game_Accuracy->Set(fAcc);
				cv_game_Health->Set(fHealth);
			}
		}
	}


	//[petar] load autobalancing stuff
	int nAllowedDeaths;
	stm.Read(nAllowedDeaths);
	pAISystem->GetAutoBalanceInterface()->SetAllowedDeathCount(nAllowedDeaths);


	// apparently these 20 updates are needed to setup everything,
	// from hud to netwrok stream etc. do not remove it
	// or savegame won't load
	for (int i = 0; i<20; i++) 
		Update();

	if (bLoadBar)
	{
		pConsole->TickProgressBar();	// advance progress
	}
	
	VERIFY_COOKIE_NO(stm,0x3c);

	// loading reserver IDs for dynacally created saved entities
	int dynReservedIDsNumber=0;
	stm.Read(dynReservedIDsNumber);
	for( ; dynReservedIDsNumber>0; --dynReservedIDsNumber )
	{
	int reservedId;
		stm.Read((int&)reservedId);
		pEntitySystem->MarkId( reservedId );
	}
	VERIFY_COOKIE_NO(stm,0x73);

	// load hud objectives
	if (m_pUIHud)
	{
		GetScriptSystem()->BeginCall("Hud", "OnLoadOld");
		GetScriptSystem()->PushFuncParam(m_pUIHud->GetScript());
		GetScriptSystem()->PushFuncParam(scriptStream.GetScriptObject());
		GetScriptSystem()->EndCall();
	}

	VERIFY_COOKIE_NO(stm,61);

	while (!stm.EOS())
	{
		BYTE cChunk=0;
		stm.Read(cChunk);
    
		switch(cChunk)
		{
		case CHUNK_ENTITY:
			{
				EntityId id;
				EntityClassId ClassID;	
				
				pBitStream->ReadBitStream(stm,ClassID,eEntityClassId);

				EntityClass *pClass=pECR->GetByClassId(ClassID);
				ASSERT(pClass);
				ed.className=pClass->strClassName;
				ed.ClassId=pClass->ClassId;
				
				stm.Read(id);
				ed.id=id;
				
				// [kirill] if this entity was dynamicly created - ID was marked when dynReservedIDsNumber loaded, to prevent 
				// from being taked by some other dynamically spawned non-saved entity. So now we load it and let's free the id
				if (pEntitySystem->IsDynamicEntityId( id ))
					pEntitySystem->ClearId(id);

				// position and angles are read later - 
				// with pEnt->Load(
				//////////////////////////////////////////////////////////////////////////				
				//[marco] position and angles must be read before spawining the entity - must
				// be consistent with load level!								
				Vec3d vPos,vAngles;
				if (!stm.Read(vPos))
					CryError("Error while reading position for entity id=%d",id);
				if (!stm.Read(vAngles))
					CryError("Error while reading position for entity id=%d",id);
				ed.pos=vPos;
				ed.angles=vAngles;				

				//////////////////////////////////////////////////////////////////////////				
				// renderer stuff

				float fScale;
				stm.Read(fScale);

				int dwRendFlags;
				stm.Read(dwRendFlags);
				unsigned char uViewDistRatio;
				stm.Read(uViewDistRatio);
				unsigned char uLodRatio;
				stm.Read(uLodRatio);

				string MatName;
				stm.Read(MatName);
				
				bool bHidden=false;
				stm.Read(bHidden);

				//////////////////////////////////////////////////////////////////////////
				VERIFY_COOKIE_NO(stm,76);

				string name;
				stm.Read(name);

				if (m_pLog->GetVerbosityLevel()>=5)
					m_pLog->Log("LOADED entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,id);

				//////////////////////////////////////////////////////////////////////////

				VERIFY_COOKIE_NO(stm,77);

				stm.AlignRead(); 

				_SmartScriptObject props(m_pScriptSystem);
				LoadProperties(props, stm, m_pScriptSystem, "_root");

				_SmartScriptObject propsi(m_pScriptSystem);
				LoadProperties(propsi, stm, m_pScriptSystem, "_root");

				_SmartScriptObject events(m_pScriptSystem);
				LoadProperties(events, stm, m_pScriptSystem, "_root");
				
				VERIFY_COOKIE_NO(stm,78);

				//m_pScriptSystem->BeginCall("dump");
				//m_pScriptSystem->PushFuncParam(props);
				//m_pScriptSystem->EndCall();

				//ASSERT(!pEntitySystem->GetEntity(id));    // entity already exists in map
				//IEntity* dbgEnt=pEntitySystem->GetEntity(id);

				if (pEntitySystem->GetEntity(id))
				{
					CryError("entity classname %s classid=%d id=%d ALREADY EXISTING IN THE MAP",pClass->strClassName.c_str(),(int)pClass->ClassId,id);
				}

				ed.pProperties=props;
				ed.pPropertiesInstance=propsi;
				ed.name = name;
				pEnt=pEntitySystem->SpawnEntity(ed);
				//ASSERT(pEnt);
				if (!pEnt)
					CryError("entity classname %s classid=%02d id=%3id CANNOT BE SPAWNED",pClass->strClassName.c_str(),(int)pClass->ClassId,id);

				if(id==0)		// it's a local player
					localPlayerId = ed.id;

				//////////////////////////////////////////////////////////////////////////
				// render flags
				pEnt->SetRndFlags(dwRendFlags);
				pEnt->SetScale(fScale);
				pEnt->SetViewDistRatio(uViewDistRatio);
				pEnt->SetLodRatio(uLodRatio);

				if (!MatName.empty())
				{
					IMatInfo *pMtl = GetSystem()->GetI3DEngine()->FindMaterial(MatName.c_str());
					if (pMtl)					
						pEnt->SetMaterial(pMtl);					
				}
				
				pEnt->Hide(bHidden);

				//////////////////////////////////////////////////////////////////////////				

				///pEnt->SetName(name.c_str());

				IScriptObject *so = pEnt->GetScriptObject();
				ASSERT(so);

				//so->SetValue("Properties", props);
				//so->SetValue("PropertiesInstance", propsi);
				so->SetValue("Events", events);

				CPlayer *pPlayer=NULL;
				if(pEnt->GetContainer()) pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);


				VERIFY_COOKIE_NO(stm,45);

				int health = 0;
				stm.Read(health);

				VERIFY_COOKIE_NO(stm,46);

				if(pPlayer)
				{
					if(health<=0)
					{                    
						pEnt->GetCharInterface()->KillCharacter(0);
#ifdef _DEBUG
						m_pLog->Log("DEAD entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,pEnt->GetId());
#endif
						//pEntitySystem->RemoveEntity(pEnt->GetId());		
					};
				};

				bool b = pEnt->LoadRELEASE(stm,scriptStream.GetScriptObject());
				if (!b)
					CryError("entity classname %s classid=%02d id=%3id CANNOT BE LOADED",pClass->strClassName.c_str(),(int)pClass->ClassId,id);
				//ASSERT(b);

				// [anton] proper state serialization was absent BasicEntity.lua,
				// we'll have to at least make sure that activated rigid bodies that were initially not active 
				// don't load velocity from active state
				IPhysicalEntity *pPhys = pEnt->GetPhysics();
				pe_status_dynamics sd;
				if (pPhys && pPhys->GetType()==PE_RIGID && pPhys->GetStatus(&sd) && sd.mass==0)
				{
					pe_action_set_velocity asv;
					asv.v.Set(0,0,0); asv.w.Set(0,0,0);
					pPhys->Action(&asv);
				}

				if(pPlayer)
				{
//[kirill] the OnReset called from OnInit which is called when entity is spawned
//					if(health>0)
//					{
//						// call on reset for the spawned entity
//						m_pScriptSystem->BeginCall(pEnt->GetEntityClassName(),"OnReset");
//						m_pScriptSystem->PushFuncParam(pEnt->GetScriptObject());
//						m_pScriptSystem->EndCall();
//					};

					pPlayer->LoadGame_PATCH_1(stm);
				};
				break;
			}

		case CHUNK_PLAYER:
			{
				EntityId wPlayerID = 0;
				bool b = stm.Read(wPlayerID);
				ASSERT(b);

				if(wPlayerID==0)				// we write 0 for localplayer on save
					wPlayerID = localPlayerId;
				m_pClient->SetPlayerID(wPlayerID);

				pEnt=pEntitySystem->GetEntity(wPlayerID);
//				if( wPlayerID==0 )
//					pEnt=m_pClient->m_pISystem->GetLocalPlayer();
				if (!pEnt)
					CryError("player id=%3id NOT FOUND",wPlayerID);
				//ASSERT(pEnt);

				CPlayer *pPlayer=NULL;
				if(pEnt->GetContainer()) pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
				ASSERT(pPlayer);
				stm.Read(pPlayer->m_bFirstPerson);
				SetViewMode(!pPlayer->m_bFirstPerson); // hack, but works
				// do we want to overwrite health with half of maximum
				if(p_restorehalfhealth->GetIVal())
				{
					pPlayer->m_stats.health = 255;	// [marco] danger! this should be set by
													// gamerules but it gets overwritten by the save checkpoint

					//m_pSystem->GetILog()->Log("player health=%d",pPlayer->m_stats.health);
					// [kirill]
					// this was requested by UBI. It's expected here that current health value is the maximum 
					//Everytime Jack dies he should respawn with half of his hit points instead of full health. 
					//Same mechanics for Val, she should get half her hit points everytime Jack respawns.
					pPlayer->m_stats.health/=2;
				}

				if (pPlayer->m_stats.health<128)
					pPlayer->m_stats.health = 128;

				IEntityCamera *pCam=pEnt->GetCamera();
				ASSERT(pCam);

				Vec3d vPos;
				stm.Read(vPos);
				pCam->SetPos(vPos);
				pEnt->SetPos(vPos);				

				m_pLog->Log("PLAYER %d (%f %f %f) ", wPlayerID, vPos.x, vPos.y, vPos.z);

				Vec3d vAngles;
				stm.Read(vAngles);
				pCam->SetAngles(vAngles);
				pEnt->SetAngles(vAngles);
				m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(vAngles);

				GetSystem()->SetViewCamera( pCam->GetCamera() );

				GetSystem()->GetISoundSystem()->SetListener(pCam->GetCamera(),Vec3(0,0,0));

				if(!isdemo)
				{
					CXServer::XSlotMap::iterator itor;
					ASSERT(m_pServer->GetSlotsMap().size()==1);
					itor = m_pServer->GetSlotsMap().begin();

					CXServerSlot *pSSlot=itor->second;							// serverslot associated with the player
					
					pSSlot->SetPlayerID(wPlayerID);
					pSSlot->SetGameState(CGS_INPROGRESS);						// start game imediatly
				};
				break;
			}

		case CHUNK_AI:
			{				
				// find for which entity this chunk is
				int nID;
				stm.Read(nID);

				IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
				if (pEntity)
				{
					CPlayer *pPlayer=0;
					if (pEntity->GetContainer())
					{
						if (pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
							pPlayer->LoadAIState_RELEASE(stm);
					}
				}

				VERIFY_COOKIE_NO(stm,13);
			}
			break;
		default:
			ASSERT(0);
		};

		if (bLoadBar)
		{
			pConsole->TickProgressBar();	// advance progress
		}
	}

	{	// [Anton] - allow entities to restore pointer links between them during post load step 
		// [kirill]	restore all the bindings
		IEntityItPtr pEntities=pEntitySystem->GetEntityIterator();

		pEntities->MoveFirst();
		IEntity *pEnt=NULL;
		while((pEnt=pEntities->Next())!=NULL)
			pEnt->PostLoad();		
	}

	pEntitySystem->Update();
	m_pSystem->GetIMovieSystem()->PlayOnLoadSequences();	// yes, we reset this twice, the first time to remove all entity-pointers and now to restore them
	m_pClient->Reset();
	
	m_bIsLoadingLevelFromFile = false;
	m_pSystem->GetISoundSystem()->Mute(false); 

	m_bMapLoadedFromCheckpoint=true;

	
	m_pEntitySystem->PauseTimers(false,true);	

	//	m_pLog->Log("HIDE CONSOLE");
	m_pRenderer->ClearColorBuffer(Vec3(0,0,0));
	m_pSystem->GetIConsole()->ResetProgressBar(0);
	m_pSystem->GetIConsole()->ShowConsole(false);
	m_pSystem->GetIConsole()->SetScrollMax(600/2);

	if (nPreset!=-1)
		m_pSystem->GetISoundSystem()->SetEaxListenerEnvironment(nPreset,NULL);
	else
		m_pSystem->GetISoundSystem()->SetEaxListenerEnvironment(nPreset,&tProps);

	GotoGame(1);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//[KIRILL]
// DO NOT UPDATE THIS FUNCTION ITS HERE TO ENABLE THAT ALL NEWER VERSIONS
// SUPPORT LOADING OLDER SAVEFILES  -  from PATCH 1
bool CXGame::LoadFromStream_PATCH_1(CStream &stm, bool isdemo, CScriptObjectStream &scriptStream)
{
	IBitStream *pBitStream=GetIBitStream();

	IEntitySystem *pEntitySystem=m_pSystem->GetIEntitySystem();
	IEntityClassRegistry *pECR=GetClassRegistry();
	IEntity *pEnt=NULL;
	CEntityDesc ed;
	int			localPlayerId=0;

	string sLevelName;
	string sMissionName;
	stm.Read(sLevelName);
	stm.Read(sMissionName);

	// read dummy save date and time
	unsigned char bDummy;
	unsigned short wDummy;
	stm.Read(bDummy);	// hour
	stm.Read(bDummy);	// minute
	stm.Read(bDummy);	// second
	stm.Read(bDummy);	// day
	stm.Read(bDummy);	// month
	stm.Read(wDummy);	// year

	// load savegame name
	string sFilename;
	stm.Read(sFilename);

	// load the number of entities, for loading screen bar
	int nEnt;
	stm.Read(nEnt);

	bool bS=IsServer();
	bool bC=IsClient();

	assert(!IsMultiplayer());

	VERIFY_COOKIE_NO(stm,0x22);

	// Load EAX preset
	int nPreset;
	CS_REVERB_PROPERTIES tProps;
	m_pSystem->GetISoundSystem()->GetCurrentEaxEnvironment(nPreset,tProps);
	stm.Read(nPreset);
	if (nPreset==-1)
	{	
		stm.ReadBits((BYTE *)&tProps,sizeof(CS_REVERB_PROPERTIES));
	}

	VERIFY_COOKIE_NO(stm,0x12);

	// load saved cvars
	string varname,val;
	int nCount,i;
	stm.Read(nCount);
	IConsole *pCon=m_pSystem->GetIConsole();	
	for (i=0;i<nCount;i++)
	{
		if(stm.Read(varname))
			if(stm.Read(val))
			{
				ICVar *pCVar=m_pSystem->GetIConsole()->GetCVar(varname.c_str());
				if (!pCVar)
				{
					m_pSystem->GetILog()->Log("\001 WARNING, CVar %s(%s) was saved but is not present",varname.c_str(),val.c_str());
				}
				else
					pCVar->Set(val.c_str());
			}
			else
			{
				m_pSystem->GetILog()->LogError("CXGame::LoadFromStream %d/%d critical error",i,nCount);
				stm.Debug();
				m_bIsLoadingLevelFromFile = false;
				return false;
			}
	} //i

	if(m_pSystem->GetISoundSystem())
		m_pSystem->GetISoundSystem()->Silence();

	m_pSystem->GetISoundSystem()->Mute(true); 

	bool			bLoadBar = false;
	IConsole *pConsole = m_pSystem->GetIConsole();

	assert(pConsole);

	if(stricmp(g_LevelName->GetString(),sLevelName.c_str()) != 0 || !m_pServer || (stricmp(m_pServer->m_GameContext.strMission.c_str(),sMissionName.c_str()) != 0))
	{

		m_pLog->LogToConsole("Loading %s / %s", sLevelName.c_str(), sMissionName.c_str());

		if (m_pServer) 
			ShutdownServer();

		if (isdemo)
		{			
			//StartupClient();
			m_pClient->DemoConnect();
			LoadLevelCS( false,sLevelName.c_str(), sMissionName.c_str(), false);
		}
		else
		{		
			GetISystem()->GetIInput()->EnableEventPosting(false);
			//m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			LoadLevelCS(false,sLevelName.c_str(), sMissionName.c_str(), false);
			//m_pClient->Connect("localhost");				
			//m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			GetISystem()->GetIInput()->EnableEventPosting(true);			
		};
	}
	else
	{
		bLoadBar = 1;

		string sLoadingScreenTexture = m_currentLevelFolder + "/loadscreen_" + m_currentLevel + ".dds";
		pConsole->SetLoadingImage( sLoadingScreenTexture.c_str() );

		pConsole->Clear();
		pConsole->ResetProgressBar(nEnt + 3);
		pConsole->SetScrollMax(600);
		pConsole->ShowConsole(1);
		DeleteMessage("Switch"); // no switching during loading

		// local player has to exit all areas before starting to delete entities
		IEntity *pIMyPlayer = GetMyPlayer();
		if( pIMyPlayer )
		{
			IEntityContainer *pIContainer = pIMyPlayer->GetContainer();
			if (pIContainer)
			{
				CPlayer *pPlayer = NULL;
				if (pIContainer->QueryContainerInterface(CIT_IPLAYER, (void**)&pPlayer))
					m_XAreaMgr.ExitAllAreas( pPlayer->m_AreaUser );
			}
		}

		m_pSystem->GetI3DEngine()->RestoreTerrainFromDisk();
		m_pSystem->GetIMovieSystem()->Reset( false );
		m_pLog->Log("REMOVING entities:");
		IEntityItPtr pEntities=pEntitySystem->GetEntityIterator();

		pEntities->MoveFirst();
		IEntity *pEnt=NULL;
		while((pEnt=pEntities->Next())!=NULL)
		{
			EntityClass *pClass=pECR->GetByClass(pEnt->GetEntityClassName());
			if (m_pWeaponSystemEx->IsProjectileClass(pClass->ClassId)) continue;
#ifdef _DEBUG
			m_pLog->Log("REMOVING entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,pEnt->GetId());
#endif
			pEntitySystem->RemoveEntity(pEnt->GetId());		
		}

		pConsole->TickProgressBar();	// advance progress

		pEntitySystem->Update();

		SoftReset();
		m_pEntitySystem->Reset();

		pConsole->TickProgressBar();	// advance progress
	}

	// PETAR lets delete all guys since they will be spawned anyway
	m_pSystem->GetAISystem()->Reset();	

	IAISystem *pAISystem = m_pSystem->GetAISystem();

	// adaptive balancing
	if (pAISystem)
	{
		if (cv_game_Difficulty->GetIVal())
		{
			float fAcc,fAgg,fHealth;
			if (pAISystem->GetAutoBalanceInterface())
			{
				pAISystem->GetAutoBalanceInterface()->GetMultipliers(fAcc,fAgg,fHealth);
				cv_game_Aggression->Set(fAgg);
				cv_game_Accuracy->Set(fAcc);
				cv_game_Health->Set(fHealth);
			}
		}
	}


	//[petar] load autobalancing stuff
	int nAllowedDeaths;
	stm.Read(nAllowedDeaths);
	pAISystem->GetAutoBalanceInterface()->SetAllowedDeathCount(nAllowedDeaths);


	// apparently these 20 updates are needed to setup everything,
	// from hud to netwrok stream etc. do not remove it
	// or savegame won't load
	for (int i = 0; i<20; i++) 
		Update();

	if (bLoadBar)
	{
		pConsole->TickProgressBar();	// advance progress
	}

	VERIFY_COOKIE_NO(stm,0x3c);

	// loading reserver IDs for dynacally created saved entities
	int dynReservedIDsNumber=0;
	stm.Read(dynReservedIDsNumber);
	for( ; dynReservedIDsNumber>0; --dynReservedIDsNumber )
	{
		int reservedId;
		stm.Read((int&)reservedId);
		pEntitySystem->MarkId( reservedId );
	}

	// only load this in case of older save
	VERIFY_COOKIE_NO(stm,0x73);
	// load hud objectives
	if (m_pUIHud)
	{
		GetScriptSystem()->BeginCall("Hud", "OnLoadOld");
		GetScriptSystem()->PushFuncParam(m_pUIHud->GetScript());
		GetScriptSystem()->PushFuncParam(scriptStream.GetScriptObject());
		GetScriptSystem()->EndCall();
	}

	VERIFY_COOKIE_NO(stm,61);

	while (!stm.EOS())
	{
		BYTE cChunk=0;
		stm.Read(cChunk);
		switch(cChunk)
		{
		case CHUNK_ENTITY:
			{
				EntityId id;
				EntityClassId ClassID;	

				pBitStream->ReadBitStream(stm,ClassID,eEntityClassId);

				EntityClass *pClass=pECR->GetByClassId(ClassID);
				ASSERT(pClass);
				ed.className=pClass->strClassName;
				ed.ClassId=pClass->ClassId;

				stm.Read(id);
				ed.id=id;

				// [kirill] if this entity was dynamicly created - ID was marked when dynReservedIDsNumber loaded, to prevent 
				// from being taked by some other dynamically spawned non-saved entity. So now we load it and let's free the id
				if (pEntitySystem->IsDynamicEntityId( id ))
					pEntitySystem->ClearId(id);

				// position and angles are read later - 
				// with pEnt->Load(
				//////////////////////////////////////////////////////////////////////////				
				//[marco] position and angles must be read before spawining the entity - must
				// be consistent with load level!								
				Vec3d vPos,vAngles;
				if (!stm.Read(vPos))
					CryError("Error while reading position for entity id=%d",id);
				if (!stm.Read(vAngles))
					CryError("Error while reading position for entity id=%d",id);
				ed.pos=vPos;
				ed.angles=vAngles;				

				//////////////////////////////////////////////////////////////////////////				
				// renderer stuff

				float fScale;
				stm.Read(fScale);

				int dwRendFlags;
				stm.Read(dwRendFlags);
				unsigned char uViewDistRatio;
				stm.Read(uViewDistRatio);
				unsigned char uLodRatio;
				stm.Read(uLodRatio);

				string MatName;
				stm.Read(MatName);

				bool bHidden=false;
				stm.Read(bHidden);

				//////////////////////////////////////////////////////////////////////////
				VERIFY_COOKIE_NO(stm,76);

				string name;
				stm.Read(name);

				if (m_pLog->GetVerbosityLevel()>=5)
					m_pLog->Log("LOADED entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,id);

				//////////////////////////////////////////////////////////////////////////

				VERIFY_COOKIE_NO(stm,77);

				stm.AlignRead(); 

				_SmartScriptObject props(m_pScriptSystem);
				LoadProperties(props, stm, m_pScriptSystem, "_root");

				_SmartScriptObject propsi(m_pScriptSystem);
				LoadProperties(propsi, stm, m_pScriptSystem, "_root");

				_SmartScriptObject events(m_pScriptSystem);
				LoadProperties(events, stm, m_pScriptSystem, "_root");

				VERIFY_COOKIE_NO(stm,78);

				//m_pScriptSystem->BeginCall("dump");
				//m_pScriptSystem->PushFuncParam(props);
				//m_pScriptSystem->EndCall();

				//ASSERT(!pEntitySystem->GetEntity(id));    // entity already exists in map
				//IEntity* dbgEnt=pEntitySystem->GetEntity(id);

				if (pEntitySystem->GetEntity(id))
				{
					CryError("entity classname %s classid=%d id=%d ALREADY EXISTING IN THE MAP",pClass->strClassName.c_str(),(int)pClass->ClassId,id);
				}

				ed.pProperties=props;
				ed.pPropertiesInstance=propsi;
				ed.name = name;
				pEnt=pEntitySystem->SpawnEntity(ed);
				//ASSERT(pEnt);
				if (!pEnt)
					CryError("entity classname %s classid=%02d id=%3id CANNOT BE SPAWNED",pClass->strClassName.c_str(),(int)pClass->ClassId,id);

				if(id==0)		// it's a local player
					localPlayerId = ed.id;

				//////////////////////////////////////////////////////////////////////////
				// render flags
				pEnt->SetRndFlags(dwRendFlags);
				pEnt->SetScale(fScale);
				pEnt->SetViewDistRatio(uViewDistRatio);
				pEnt->SetLodRatio(uLodRatio);

				if (!MatName.empty())
				{
					IMatInfo *pMtl = GetSystem()->GetI3DEngine()->FindMaterial(MatName.c_str());
					if (pMtl)					
						pEnt->SetMaterial(pMtl);					
				}

				pEnt->Hide(bHidden);

				//////////////////////////////////////////////////////////////////////////				

				///pEnt->SetName(name.c_str());

				IScriptObject *so = pEnt->GetScriptObject();
				ASSERT(so);

				//so->SetValue("Properties", props);
				//so->SetValue("PropertiesInstance", propsi);
				so->SetValue("Events", events);

				CPlayer *pPlayer=NULL;
				if(pEnt->GetContainer()) pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);


				VERIFY_COOKIE_NO(stm,45);

				int health = 0;
				stm.Read(health);

				VERIFY_COOKIE_NO(stm,46);

				if(pPlayer)
				{
					if(health<=0)
					{                    
						pEnt->GetCharInterface()->KillCharacter(0);
#ifdef _DEBUG
						m_pLog->Log("DEAD entity classname %s classid=%02d id=%3id ",pClass->strClassName.c_str(),(int)pClass->ClassId,pEnt->GetId());
#endif
						//pEntitySystem->RemoveEntity(pEnt->GetId());		
					};
				};

				bool b = pEnt->LoadPATCH1(stm,scriptStream.GetScriptObject());
				if (!b)
					CryError("entity classname %s classid=%02d id=%3id CANNOT BE LOADED",pClass->strClassName.c_str(),(int)pClass->ClassId,id);
				//ASSERT(b);

				// [anton] proper state serialization was absent BasicEntity.lua,
				// we'll have to at least make sure that activated rigid bodies that were initially not active 
				// don't load velocity from active state
				IPhysicalEntity *pPhys = pEnt->GetPhysics();
				pe_status_dynamics sd;
				if (pPhys && pPhys->GetType()==PE_RIGID && pPhys->GetStatus(&sd) && sd.mass==0)
				{
					pe_action_set_velocity asv;
					asv.v.Set(0,0,0); asv.w.Set(0,0,0);
					pPhys->Action(&asv);
				}

				// update position if ended up under terrain and outdoors

				if(pPlayer)
				{

					if(id==0)
					{
						I3DEngine *pEngine = m_pSystem->GetI3DEngine();
						if (pEngine && pAISystem)
						{

							Vec3d pos = pEnt->GetPos();
							IVisArea *pArea;
							int nBuildingid;
							if (!pAISystem->CheckInside(pos,nBuildingid,pArea))
							{
								float newz = pEngine->GetTerrainElevation(pos.x,pos.y);
								if (newz>pos.z)
									pos.z=newz+0.1f;
								pEnt->SetPos(pos);
							}
						}

					}

					//[kirill] the OnReset called from OnInit which is called when entity is spawned
					//					if(health>0)
					//					{
					//						// call on reset for the spawned entity
					//						m_pScriptSystem->BeginCall(pEnt->GetEntityClassName(),"OnReset");
					//						m_pScriptSystem->PushFuncParam(pEnt->GetScriptObject());
					//						m_pScriptSystem->EndCall();
					//					};

					pPlayer->LoadGame_PATCH_1(stm);
					if (pEnt->GetAI())
					{
						char sName[255];
						stm.Read(sName,255);
						pEnt->GetAI()->SetName(sName);
					}
				};
				break;
			}

		case CHUNK_PLAYER:
			{
				EntityId wPlayerID = 0;
				bool b = stm.Read(wPlayerID);
				ASSERT(b);

				if(wPlayerID==0)				// we write 0 for localplayer on save
					wPlayerID = localPlayerId;
				m_pClient->SetPlayerID(wPlayerID);

				pEnt=pEntitySystem->GetEntity(wPlayerID);
				//				if( wPlayerID==0 )
				//					pEnt=m_pClient->m_pISystem->GetLocalPlayer();
				if (!pEnt)
					CryError("player id=%3id NOT FOUND",wPlayerID);
				//ASSERT(pEnt);

				CPlayer *pPlayer=NULL;
				if(pEnt->GetContainer()) pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
				ASSERT(pPlayer);
				stm.Read(pPlayer->m_bFirstPerson);
				SetViewMode(!pPlayer->m_bFirstPerson); // hack, but works

				/* [kirill] moved this to int CScriptObjectGame::TouchCheckPoint(IFunctionHandler *pH)
				needed to fix quickLoad restoreHealth problem 
				// do we want to overwrite health with half of maximum
				if(p_restorehalfhealth->GetIVal())
				{
				pPlayer->m_stats.health = 255;	// [marco] danger! this should be set by
				// gamerules but it gets overwritten by the save checkpoint

				//m_pSystem->GetILog()->Log("player health=%d",pPlayer->m_stats.health);
				// [kirill]
				// this was requested by UBI. It's expected here that current health value is the maximum 
				//Everytime Jack dies he should respawn with half of his hit points instead of full health. 
				//Same mechanics for Val, she should get half her hit points everytime Jack respawns.
				pPlayer->m_stats.health/=2;
				}

				if (pPlayer->m_stats.health<128)
				pPlayer->m_stats.health = 128;
				*/
				IEntityCamera *pCam=pEnt->GetCamera();
				ASSERT(pCam);

				Vec3d vPos;
				stm.Read(vPos);
				pCam->SetPos(vPos);
				//pEnt->SetPos(vPos);

				m_pLog->Log("PLAYER %d (%f %f %f) ", wPlayerID, vPos.x, vPos.y, vPos.z);

				Vec3d vAngles;
				stm.Read(vAngles);
				pCam->SetAngles(vAngles);
				//[kirill]
				//why do we do it here? entity angles are set when entity is loaded
				//pEnt->SetAngles(vAngles);
				m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(vAngles);

				GetSystem()->SetViewCamera( pCam->GetCamera() );

				GetSystem()->GetISoundSystem()->SetListener(pCam->GetCamera(),Vec3(0,0,0));

				if(!isdemo)
				{
					CXServer::XSlotMap::iterator itor;
					ASSERT(m_pServer->GetSlotsMap().size()==1);
					itor = m_pServer->GetSlotsMap().begin();

					CXServerSlot *pSSlot=itor->second;							// serverslot associated with the player

					pSSlot->SetPlayerID(wPlayerID);
					pSSlot->SetGameState(CGS_INPROGRESS);						// start game imediatly
				};
				break;
			}

		case CHUNK_AI:
			{				
				// find for which entity this chunk is
				int nID;
				stm.Read(nID);

				IEntity *pEntity = m_pEntitySystem->GetEntity(nID);
				if ( nID == 0 )	// it's local player
					pEntity = m_pClient->m_pISystem->GetLocalPlayer();
				if (pEntity)
				{
					m_pLog->Log("Now loading AICHUNK <PATCH_1> for entity %s", pEntity->GetName());
					CPlayer *pPlayer=0;
					CVehicle *pVehicle=0;
					if (pEntity->GetContainer())
					{
						if (pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
							pPlayer->LoadAIState_PATCH_1(stm);
						if (pEntity->GetContainer()->QueryContainerInterface(CIT_IVEHICLE,(void**)&pVehicle))
							pVehicle->LoadAIState(stm, scriptStream);
					}
					else if( pEntity->GetAI() )
					{
						pEntity->GetAI()->Load(stm);
						IScriptSystem *pScriptSystem = GetSystem()->GetIScriptSystem();
						HSCRIPTFUNCTION	loadOverallFunction=NULL;
//						if( pEntity->GetScriptObject() && pEntity->GetScriptObject()->GetValue("OnLoadOverall", loadOverallFunction) )
//						{
//							pScriptSystem->BeginCall(loadOverallFunction);
//							pScriptSystem->PushFuncParam(pEntity->GetScriptObject());
//							pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
//							pScriptSystem->EndCall();
//						}
					}
				}

				VERIFY_COOKIE_NO(stm,13);
			}
			break;
		case CHUNK_INGAME_SEQUENCE:
			{
#if !defined(LINUX)	
				IMovieSystem *pMovies = m_pSystem->GetIMovieSystem();
				char szName[1024];
				stm.Read(szName,1024);
				float fTime;
				stm.Read(fTime);
				IAnimSequence *pSeq = pMovies->FindSequence(szName);
				pMovies->PlaySequence(pSeq,false);
				pMovies->SetPlayingTime(pSeq,fTime);
#endif
			}
			break;
		case CHUNK_HUD:
			{
				// load hud/clientstuff viewlayers data
				if (m_pUIHud)
				{
					GetScriptSystem()->BeginCall("Hud", "OnLoad");
					GetScriptSystem()->PushFuncParam(m_pUIHud->GetScript());
					GetScriptSystem()->PushFuncParam(scriptStream.GetScriptObject());
					GetScriptSystem()->EndCall();
				}

				_SmartScriptObject pClientStuff(m_pScriptSystem,true);  
				if(m_pScriptSystem->GetGlobalValue("ClientStuff",pClientStuff))	
				{
					m_pScriptSystem->BeginCall("ClientStuff","OnLoad");
					m_pScriptSystem->PushFuncParam(pClientStuff);
					m_pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
					m_pScriptSystem->EndCall();
				}
			}
			break;  

		default:
			ASSERT(0);
		};

		if (bLoadBar)
		{
			pConsole->TickProgressBar();	// advance progress
		}
	}

	{	// [Anton] - allow entities to restore pointer links between them during post load step 
		// [kirill]	restore all the bindings
		IEntityItPtr pEntities=pEntitySystem->GetEntityIterator();

		pEntities->MoveFirst();
		IEntity *pEnt=NULL;
		while((pEnt=pEntities->Next())!=NULL)
			pEnt->PostLoad();		
	}

	pEntitySystem->Update();
	m_pSystem->GetIMovieSystem()->PlayOnLoadSequences();	// yes, we reset this twice, the first time to remove all entity-pointers and now to restore them
	m_pClient->Reset();

	m_bIsLoadingLevelFromFile = false;
	m_pSystem->GetISoundSystem()->Mute(false); 

	m_bMapLoadedFromCheckpoint=true;


	m_pEntitySystem->PauseTimers(false,true);	

	//	m_pLog->Log("HIDE CONSOLE");
	m_pRenderer->ClearColorBuffer(Vec3(0,0,0));
	m_pSystem->GetIConsole()->ResetProgressBar(0);
	m_pSystem->GetIConsole()->ShowConsole(false);
	m_pSystem->GetIConsole()->SetScrollMax(600/2);

	if (nPreset!=-1)
		m_pSystem->GetISoundSystem()->SetEaxListenerEnvironment(nPreset,NULL);
	else
		m_pSystem->GetISoundSystem()->SetEaxListenerEnvironment(nPreset,&tProps);

	GotoGame(1);
	m_nDEBUG_TIMING = 0;

	return true;
};


