
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameActionInput.cpp
//  
//	History:
//	-December 11,2001: created
//	-October	31,2003: split from GameLoading.cpp
//	
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"
#include "UIHud.h"
#include "XPlayer.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include <IEntitySystem.h>
#include <ICryPak.h>
#include "UISystem.h"
#include "ScriptObjectUI.h"

//////////////////////////////////////////
//small utility macro
//////////////////////////////////////////
#define ADD_ACTION(_action,_activationmode,_desc,_type,_configurable) \
{	\
	ActionInfo Info; \
	Info.nId=ACTION_##_action; \
	Info.sDesc=_desc; \
	Info.bConfigurable=_configurable; \
	Info.ActivationMode=_activationmode; \
	Info.nType=_type; \
	m_pIActionMapManager->CreateAction(ACTION_##_action,#_action,_activationmode); \
	m_mapActionsEnum.insert(ActionsEnumMapItor::value_type(#_action,Info)); \
}

#define ACTIONMAPS_ALL "default", "zoom", "binozoom", "vehicle", "player_dead", ""
#define ACTIONMAPS_NODEAD "default", "zoom", "binozoom", "vehicle", ""
#define ACTIONMAPS_WEAPON "default", "binozoom", ""
#define ACTIONMAPS_DEF_ZOOMS "default", "zoom", "binozoom",""

//////////////////////////////////////////////////////////////////////////
void CXGame::SetConfigToActionMap(const char *pszActionName, ...)
{
	ActionsEnumMapItor It=m_mapActionsEnum.find(pszActionName);
	if (It==m_mapActionsEnum.end())
		return;
	ActionInfo &Info=It->second;
	va_list v;
	va_start(v, pszActionName);            
#if defined(LINUX64)
	char *sActionMapName=va_arg(v, char*);
	while (*sActionMapName)
	{
		Info.vecSetToActionMap.push_back(string(sActionMapName));
		sActionMapName=va_arg(v, char*);
	}
#else
	char *sActionMapName=*(char**)v;

	while (*sActionMapName)
	{
		Info.vecSetToActionMap.push_back(string(sActionMapName));
		sActionMapName=*(char**)(v+=sizeof(char*));
	}
#endif
	va_end(v);
}

//////////////////////////////////////////////////////////////////////
void CXGame::SetCommonKeyBindings(IActionMap *pMap)
{
	//movements
	//strafe left
	pMap->BindAction(ACTION_MOVE_LEFT,XKEY_A);
	pMap->BindAction(ACTION_MOVE_LEFT,XKEY_NUMPAD4);

	//strafe right
	pMap->BindAction(ACTION_MOVE_RIGHT,XKEY_D);
	pMap->BindAction(ACTION_MOVE_RIGHT,XKEY_NUMPAD6);

	//run forward
	pMap->BindAction(ACTION_MOVE_FORWARD,XKEY_W);
	//pMap->BindAction(ACTION_MOVE_FORWARD,XKEY_NUMPAD8);
	pMap->BindAction(ACTION_MOVE_FORWARD,XKEY_NUMPAD5);

	//run backward
	pMap->BindAction(ACTION_MOVE_BACKWARD,XKEY_S);
	//pMap->BindAction(ACTION_MOVE_BACKWARD,XKEY_NUMPAD5);
	pMap->BindAction(ACTION_MOVE_BACKWARD,XKEY_NUMPAD2);

	//look around
	pMap->BindAction(ACTION_TURNLR,XKEY_MAXIS_X);
	pMap->BindAction(ACTION_TURNUD,XKEY_MAXIS_Y);
		
	//reload 
	pMap->BindAction(ACTION_RELOAD,XKEY_R);

	//change firemode
	//pMap->BindAction(ACTION_FIREMODE,XKEY_F); 
	pMap->BindAction(ACTION_FIREMODE,XKEY_X); 
	pMap->BindAction(ACTION_FIREMODE,XKEY_NUMLOCK); 

	//throw grenade
	pMap->BindAction(ACTION_FIRE_GRENADE, XKEY_G);
	//pMap->BindAction(ACTION_FIRE_GRENADE, XKEY_DIVIDE);
	//pMap->BindAction(ACTION_FIRE_GRENADE, XKEY_NUMPAD8);
 
	//use
	//pMap->BindAction(ACTION_USE,XKEY_RETURN); 	
	pMap->BindAction(ACTION_USE,XKEY_F); 
	//pMap->BindAction(ACTION_USE,XKEY_NUMPADENTER); 

	//cycle grenade
	pMap->BindAction(ACTION_CYCLE_GRENADE,XKEY_H); 

	//flashlight
	pMap->BindAction(ACTION_FLASHLIGHT,XKEY_L);
	pMap->BindAction(ACTION_FLASHLIGHT, XKEY_DIVIDE);

	////run
	pMap->BindAction(ACTION_WALK,XKEY_Z);
	//pMap->BindAction(ACTION_WALK,XKEY_LEFT);

	//run sprint
	pMap->BindAction(ACTION_RUNSPRINT,XKEY_LSHIFT);

	//crouch
	pMap->BindAction(ACTION_MOVEMODE,XKEY_LCONTROL);	
	//pMap->BindAction(ACTION_MOVEMODE,XKEY_RIGHT);
	//pMap->BindAction(ACTION_MOVEMODE,XKEY_C);	
	pMap->BindAction(ACTION_MOVEMODE,XKEY_NUMPAD0);

	//prone
	pMap->BindAction(ACTION_MOVEMODE2,XKEY_V);
	//pMap->BindAction(ACTION_MOVEMODE2,XKEY_NUMPAD1);
	//pMap->BindAction(ACTION_MOVEMODE2,XKEY_PAGE_DOWN);

	//mission reminder
	pMap->BindAction(ACTION_SCORE_BOARD,XKEY_TAB);			

	//concentration feature
	//pMap->BindAction(ACTION_CONCENTRATION,XKEY_K);

	//chat
	pMap->BindAction(ACTION_MESSAGEMODE, XKEY_Y);
	pMap->BindAction(ACTION_MESSAGEMODE2, XKEY_U);
 
	// save/load bindings	
	// disable this conflicts with cutscene
//	pMap->BindAction(ACTION_QUICKLOAD,XKEY_F6);
//	pMap->BindAction(ACTION_QUICKSAVE,XKEY_F5);
	pMap->BindAction(ACTION_TAKESCREENSHOT,XKEY_F12);

#ifdef _DEBUG
	// <<FIXME>> Hack only in debug mode
	pMap->BindAction(ACTION_SAVEPOS,XKEY_F9);
	pMap->BindAction(ACTION_LOADPOS,XKEY_F10);
#endif
}

//////////////////////////////////////////////////////////////////////////
// Define console commands.
void CXGame::InitConsoleCommands()
{
	IConsole *pConsole = m_pSystem->GetIConsole();
 
	pConsole->AddCommand("reload_materials","Game.ReloadMaterials()");
	if(!m_bEditor)
	{
		pConsole->AddCommand("sensitivity", "if (Game) then Game:SetSensitivity(%%); end", VF_NOHELP,"");

		pConsole->AddCommand("kick", "if (GameRules) then GameRules:Kick(%%); end", VF_NOHELP,"");
	
		pConsole->AddCommand("kickid", "if (GameRules) then GameRules:KickID(%%); end", VF_NOHELP,"");
		
		pConsole->AddCommand("ban", "if (GameRules) then GameRules:Ban(%%); end",0,
			"Bans the specified player! (Server only)\n"
			"Use \\listplayers to get a list of playernames and IDs."
			"Usage: ban <player>");
		
		pConsole->AddCommand("banid", "if (GameRules) then GameRules:BanID(%%); end",0,
			"Bans the specified player id! (Server only)\n"
			"Use \\listplayers to get a list of playernames and IDs."
			"Usage: banid <id>");
		
		pConsole->AddCommand("unban", "if (GameRules) then GameRules:Unban(%%); end",0,
			"Removed the specified ban! (Server only)\n"
			"Use \\listban to get a list of banned IPs and IDs."
			"Usage: unban <bannumber>");
		
		pConsole->AddCommand("listban", "if Server then Server:ListBans(); end",0,
			"Lists current banned ids on the server! (Server only)\n"
			"Usage: listplayers");
		
		pConsole->AddCommand("listplayers", "if MultiplayerUtils and MultiplayerUtils.ListPlayers then MultiplayerUtils:ListPlayers(); end",0,
			"Lists current players with id! (Server and Client)\n"
			"Usage: listplayers");
		
		pConsole->AddCommand("sv_restart", "if (GameRules) then GameRules:Restart(%%) end",0,
			"Restarts the game in X seconds! (Server only)\n"
			"Usage: sv_restart X");
		
		pConsole->AddCommand("sv_reloadmapcycle", "if (MapCycle) then MapCycle:Reload() end",0,
			"Reloads the mapcycle file, specified in the sv_mapcyclefile cvar!\n"
			"Usage: sv_reloadmapcycle");
		
		pConsole->AddCommand("sv_changemap", "if (GameRules) then GameRules:ChangeMap(%%) end",0,
			"Changes to the specified map and game type! (Server only)\n"
			"If no gametype is specified, the current gametype is used!\n"
			"Usage: sv_changemap mapname gametype");
		
		pConsole->AddCommand("messagemode", "Game:MessageMode(%line)",0,
			"Requests input from the player!\n"
			"Usage: messagemode command arg1 arg2"
			"Executes the passed command, with the player input as argument! If no param given, say_team is used.");
		
		pConsole->AddCommand("messagemode2", "Game:MessageMode2(%line)",0,
			"Requests input from the player!\n"
			"Usage: messagemode command arg1 arg2"
			"Executes the passed command, with the player input as argument! If no param given, say_team is used.");

		pConsole->AddCommand("start_server",		"local name=tonotnil(%%); "
																						"local mission=\"Default\"; "
																						"local gametype=strupper(tostring(g_GameType)); "
																						"if AvailableMODList[gametype] then "
																						"  mission=AvailableMODList[gametype].mission; "
																						"end "
																						"if(Game:CheckMap(name,mission))then "
																						" Game:LoadLevelListen(name); "
																						"else "
																						" System:Log(\"\\001map '\"..tostring(name)..\"' mission '\"..tostring(mission)..\"' not found!\"); "
																						"end",0,
			"Starts a multiplayer server.\n"
			"Usage: start_server levelname\n"
			"Loads levelname on the local machine and listens for client connections.\n");
	
		// this is needed to load a map with a specified mission
		pConsole->AddCommand("load_level","Game:LoadLevel(%%)",0,
			"Loads a new level (mission is the second parameter).\n"
			"Usage: load_level levelname [mission]\n"
			"Similar as 'map' command.");

		// you cannot specify the mission name with this command
		pConsole->AddCommand("map",							"local name = tonotnil(%%); "
																						"if name then "
																						" if(Game:CheckMap(name))then "
																						"  Game:LoadLevel(name); " 
																						" else "
																						"  System:Log(\"\\001map '\"..tostring(name)..\"' not found!\"); "
																						" end "
																						"end",0,
			"Loads a new level (mission cannot be specified).\n"
			"Usage: map levelname\n"
			"Similar as 'load_level' command.");

		pConsole->AddCommand("save_game","Game:Save(%%)",0,
			"Saves the current game.\n"
			"Usage: save_game gamename\n");

		pConsole->AddCommand("load_game","Game:Load(%%)",0,
			"Loads a previously saved game.\n"
			"Usage: load_game gamename\n");

		// marked with VF_CHEAT because it offers a MP cheat (impression to run faster on client)
		pConsole->AddCommand("record",       "Game:StartRecord(%%)",VF_CHEAT,
			"Starts recording of a demo.\n"
			"Usage: record demoname\n"
			"File 'demoname.?' will be created.");

		pConsole->AddCommand("stoprecording","Game:StopRecord()",VF_CHEAT,
			"Stops recording of a demo.\n"
			"Usage: stoprecording\n"
			"File 'demoname.?' will be saved.");

		pConsole->AddCommand("demo",         "Game:StartDemoPlay(%%)",0,
			"Plays a demo from file.\n"
			"Usage: demo demoname\n"
			"Loads demoname.? for playback in the FarCry game.");

		pConsole->AddCommand("stopdemo",     "Game:StopDemoPlay()",0,
			"Stop playing demo.\n" );
		
		if (!m_pSystem->IsDedicated())
		{
			pConsole->AddCommand("connect","Game:Connect(%1,1,1)",0,
				"Connects to the specified multiplayer server.\n"
				"Usage: connect ip\n"
				);
			pConsole->AddCommand("reconnect","Game:Reconnect()",0,
				"Reconnects to the most recent multiplayer server.\n"
				"Usage: reconnect\n"
				);
			pConsole->AddCommand("retry","Game:Reconnect()",0,
				"Reconnects to the most recent multiplayer server.\n"
				"Usage: retry\n"
				);
			pConsole->AddCommand("disconnect","Game:Disconnect(\"@UserDisconnected\")",0,
				"Breaks the client connection with the multiplayer server.\n"
				"Usage: disconnect\n");
			pConsole->AddCommand("ubilogout","if (NewUbisoftClient) then NewUbisoftClient:Client_Disconnect(); Game:Disconnect(); GotoPage(\"Multiplayer\")end",0,
				"Disconnects from ubi.com game service.\n"
				"Usage: ubilogout\n"
				);
			pConsole->AddCommand("ubilogin","if (NewUbisoftClient) then NewUbisoftClient:ConsoleLogin(%%); end",0,
				"Disconnects from ubi.com game service.\n"
				"Usage: ubilogout\n"
				);
		}
		pConsole->CreateVariable("cl_saveubipassword", "1", VF_DUMPTODISK,
			"Enables/Disables saving of Ubi.com username and password for later use.\n"
			);
		pConsole->AddCommand("load_lastcheckpoint","Game:LoadLatestCheckPoint()",0,
			"Respawns the player at the last checkpoint reached.\n"
			"Usage: load_lastcheckpoint\n");
	}

	pConsole->AddCommand("reload_script","Script:ReloadScript(%1)",VF_CHEAT,
		"\n"
		"Usage: \n"
		"");
	pConsole->AddCommand("dump_scripts","Script:DumpLoadedScripts()",VF_CHEAT,
		"Outputs a list of currently loaded scripts.\n"
		"Usage: dump_scripts\n");
	pConsole->AddCommand("quit","Game:Quit()",0,
		"Quits the game.\n"
		"Usage: quit\n");
	pConsole->AddCommand("clear","System:ClearConsole()",0,
		"Clears console text.\n"
		"Usage: clear\n");
	pConsole->AddCommand("rstats","System:DumpMMStats()",0,
		"Displays memory usage stats for release mode builds.\n"
		"Usage: rstats");
	pConsole->AddCommand("dstats","System:DebugStats(nil)",0,
		"Displays debugging statistics.\n"
		"Usage: dstats\n");
	pConsole->AddCommand("dcheckpoint","System:DebugStats(1)",0,
		"\n"
		"Usage: \n"
		"");
	pConsole->AddCommand("fov","Game:SetCameraFov(%1/180*3.14159)",VF_CHEAT,
		"Sets the player's field of view.\n"
		"Usage: fov 120\n"
		"The field of vision is set in degrees between 1 and 180.");
	pConsole->AddCommand("rcon","Game:ExecuteRConCommand(%line)",0,
		"Execute a console command on a RCon server (call rcon_connect to set up the connection)\n"
		"Usage: rcon sv_restart\n");
	pConsole->AddCommand("dump_ents","Game:DumpEntities()",VF_CHEAT,
		"Outputs a list of the loaded entities.\n"
		"Usage: dump_ents\n");
	pConsole->AddCommand("dumpcommandsvars","System:DumpCommandsVars(%%)",0,
		"Outputs a list of commands and variables.\n"
		"Usage: dumpcommandsvars\n"
		"Saves a list of all registered commands and variables\n"
		"to a file called consolecommandsandvars.txt");

	pConsole->AddCommand("SProfile_load", "local szName=tonotnil(%%); if (szName) then SProfile_load(szName); end",0,
			"Load the saved server profiles. To start the game you have\n"
			"to start the server by hand with 'start_server' or 'start_ubiserver'\n"
			"Usage: SProfile_load <profilename>");

	pConsole->AddCommand("SProfile_run", "local szName=tonotnil(%%); if (szName) then SProfile_run(szName); end",0,
			"Load and run the server with the server setting specified in the profile (generate in game)\n"
			"Usage: SProfile_run <profilename>");

	pConsole->AddCommand("frame_profiling","System:FrameProfiler(%%)",0);
	//deprecated, remove..
	pConsole->AddCommand("start_frame_profiling","System:FrameProfiler(1,nil,\"\")",0);
	pConsole->AddCommand("end_frame_profiling","System:FrameProfiler(nil,nil,\"\")",0);
	pConsole->AddCommand("start_frame_display","System:FrameProfiler(1,1,\"\")",0);
	pConsole->AddCommand("end_frame_display","System:FrameProfiler(nil,1,\"\")",0);

	pConsole->AddCommand("savepos","Game:SavePlayerPos(%%)",0,
		"Saves current player position to the tagpoint file.\n"
		"Usage: savepos pointname\n");
	pConsole->AddCommand("loadpos","Game:LoadPlayerPos(%%)",0,
		"Loads player position from the tagpoint file.\n"
		"Usage: loadpos pointname\n");
	pConsole->AddCommand("SkipCutScene","Movie:StopAllCutScenes()");
}

//////////////////////////////////////////////////////////////////////////
// Defines Console Variables used by Game.
void CXGame::InitConsoleVars()
{
	m_pScriptSystem->SetGlobalValue("__tz0", "3423");
	m_pScriptSystem->SetGlobalValue("__tz1", "31337");

	IConsole *pConsole = m_pSystem->GetIConsole();

#ifdef _INTERNET_SIMULATOR
	g_internet_simulator_maxping = GetISystem()->GetIConsole()->CreateVariable("g_internet_simulator_maxping","0",VF_CHEAT,
		"Turn on ping.\n"
		"Usage: g_internet_simulator_ping 300"
		"Default value is 0.");

	g_internet_simulator_minping = GetISystem()->GetIConsole()->CreateVariable("g_internet_simulator_minping","0",VF_CHEAT,
		"Turn on ping.\n"
		"Usage: g_internet_simulator_ping 100"
		"Default value is 0.");

	g_internet_simulator_packetloss = GetISystem()->GetIConsole()->CreateVariable("g_internet_simulator_packetloss","0",VF_CHEAT,
		"Turn on packetloss.\n"
		"Usage: g_internet_simulator_packetloss 2"
		"Default value is 0.");
#endif
	cl_scope_flare = GetISystem()->GetIConsole()->CreateVariable("cl_scope_flare","1",0,
		"Draw a flare at the weapon's scope.\n"
		"Usage: cl_scope_flare 0/1"
		"Default value is 1.");

	cl_lazy_weapon = GetISystem()->GetIConsole()->CreateVariable("cl_lazy_weapon","0.6",VF_DUMPTODISK,
		"Control if the weapon follows the camera in a lazy way.\n"
		"Usage: cl_lazy_weapon [0..1]"
		"Default value is 0.6.");

	cl_weapon_fx = GetISystem()->GetIConsole()->CreateVariable("cl_weapon_fx","2",VF_DUMPTODISK,
		"Control the complexity of weapon firing effects.\n"
		"Usage: cl_weapon_fx [0..2], 0=low,1=medium,2=high"
		"Default value is 2, but its autodetected the first time based on pc spec.");

	cl_projectile_light = GetISystem()->GetIConsole()->CreateVariable("cl_projectile_light","0",VF_DUMPTODISK,
		"Controls if projectiles are allowed to use dynamic lights.\n"
		"Usage: cl_projectile_light [0/1/2], 0=no light, 1=diffuse only, 2=diffuse and specular"
		"Default value is 0, but it should be set to 1 on high and 2 on very high spec.");

	cl_weapon_light = GetISystem()->GetIConsole()->CreateVariable("cl_weapon_light","0",VF_DUMPTODISK,
		"Controls if weapons are allowed to use dynamic lights.\n"
		"Usage: cl_weapon_light [0/1/2], 0=no light, 1=diffuse only, 2=diffuse and specular"
		"Default value is 0, but it should be set to 1 on high and 2 on very high spec.");

	GetISystem()->GetIConsole()->CreateVariable("g_timezone","0",VF_DUMPTODISK, "");

	w_underwaterbubbles = GetISystem()->GetIConsole()->CreateVariable("w_underwaterbubbles","1",0,
		"Control if underwater bullet bubble trails are displayed (should be disabled for slow machines).\n"
		"Usage: w_underwaterbubbles 0/1"
		"Default value is 1.");

	g_MP_fixed_timestep = pConsole->CreateVariable("g_MP_fixed_timestep","0.01",VF_REQUIRE_NET_SYNC|VF_CHEAT,
		"Enables fixed timestep physics simulation for multiplayer mode.\n"
		"Usage: g_MP_fixed_timestep 0.01\n"
		"Default is 0.");	

	g_maxfps = pConsole->CreateVariable("g_maxfps","500",0,
		"Sets the maximum frame rate.\n"
		"Usage: g_maxfps 500"
		"Default value is 500.");
	ai_num_of_bots = pConsole->CreateVariable("ai_num_of_bots","0",0);

	p_always_run= pConsole->CreateVariable("p_always_run","1",0,
		"Toggles the player's 'always run' setting.\n"
		"Usage: p_always_run [0/1]\n"
		"Default is 1 (on). Set to 0 to disable 'always run'.");
	g_language= pConsole->CreateVariable("g_language","english",VF_DUMPTODISK, //|VF_READONLY,
		"Sets the game language.\n"
		"Usage: g_language [english/other?]\n"
		"Default is 'english'.");
	g_playerprofile= pConsole->CreateVariable("g_playerprofile","default",VF_DUMPTODISK,
		"Sets the player profile (to store player settings).\n"
		"Leave this empty to get the defaults from the root directory.\n");
	g_serverprofile= pConsole->CreateVariable("g_serverprofile","",VF_DUMPTODISK,
		"Sets the server profile (to store server settings).\n"
		"Leave this empty to get the server defaults.\n");
	g_GC_Frequence= pConsole->CreateVariable("g_GC_Frequence","1200",0,
		"The maximum time in seconds a garbage collector pause can be.\n" 
		"Usage: g_GC_Frequence 1000\n"
		"Default setting is 1200 s");
	sv_port= pConsole->CreateVariable("sv_port",DEFAULT_SERVERPORT_STR,0,
		"Sets the server port for a multiplayer game.\n"
		"Usage: sv_port portnumber\n"
		"Default is '49001'.");
	sv_mapcyclefile = pConsole->CreateVariable("sv_mapcyclefile", "profiles/server/mapcycle.txt",VF_DUMPTODISK,
		"Sets the server map cycle list for a multiplayer game.\n"
		"Usage: sv_mapcyclefile <textfile>\n"
		"Default is 'mapcycle.txt'.");
	sv_cheater_kick = pConsole->CreateVariable("sv_cheater_kick", "1",0,"Kick player suspected cheating.\n" );
	sv_cheater_ban = pConsole->CreateVariable("sv_cheater_ban", "0",0,"Ban player suspected cheating.\n" );

	cl_loadtimeout = pConsole->CreateVariable("cl_loadtimeout", "120.0",0,
		"Sets the client timeout during loading (seconds).\n"
		"Usage: cl_loadtimeout 120\n"
		"The default loading timeout is 120 seconds. This is the time the\n"
		"client waits for packets from the server when the server is changing map."); 
	cl_timeout = pConsole->CreateVariable("cl_timeout", "5.0",0,
		"Sets the client timeout (seconds).\n"
		"Usage: cl_timeout 5\n"
		"The default timeout is 5 seconds. This is the time the\n"
		"client waits for packets from the server."); 
	cl_snooptimeout = pConsole->CreateVariable("cl_snooptimeout", "1.0",0,
		"Sets the time to wait for a server, when pinging (seconds).\n"
		"Usage: cl_snooptimeout 3\n"
		"The default timeout is 1 second. This is the time the\n"
		"pinger waits for response from the server."); 
	cl_snoopretries = pConsole->CreateVariable("cl_snoopretries", "2.0",0,
		"Sets the number or times to retry a timedout server when pinging.\n"
		"Usage: cl_snoopretries 2\n"
		"The default number of retries is 2."); 
	cl_snoopcount = pConsole->CreateVariable("cl_snoopcount", "20.0",0,
		"Sets the number or servers to ping at the same time.\n"
		"Usage: cl_snoopcount 20\n"
		"The default number of servers is 20."); 
	// initalise the game infos
	pConsole->CreateVariable("sv_maxupdaterate", "50",0,
		"Specify the max server network update rate\n"
		"(less is better for bandwidth, more is better for response,\n"
		"the actual rate is limited by frame/update rate and the client setting as well)\n"
		"Usage: sv_maxupdaterate [5..100]\n"
		"Default is 50");
	pConsole->CreateVariable("sv_maxcmdrate", "100",VF_REQUIRE_NET_SYNC,
		"Specify the max client network command rate\n"
		"(less is better for bandwidth, more is better for response,\n"
		"the actual rate is limited the client setting as well)\n"
		"Usage: sv_maxcmdrate [5..100]\n"
		"Default is 100");
	pConsole->CreateVariable("sv_password", "",0,
		"Specifies the server password in a multiplayer game.\n"
		"Specifying a null string means, no password.\n"
		"Usage: sv_password <password>\n");
	pConsole->CreateVariable("sv_maxplayers","16",0,
		"Sets the maximum number of of players for a multiplayer server.\n"
		"Usage: sv_maxplayers 8\n");

	g_InstallerVersion = pConsole->CreateVariable("cl_installshieldversion","44",VF_DUMPTODISK|VF_READONLY,
		"\n"
		"Usage: \n"
		"");

	pConsole->CreateVariable("sv_maxrate","30000",VF_DUMPTODISK,
		"Defines the maximum number of bits per second upload per player.\n"
		"Usage: sv_maxrate 40000\n"
		"Default is 30000.");
	pConsole->CreateVariable("sv_maxrate_lan","100000",VF_DUMPTODISK,
		"Defines the maximum number of bits per second upload per player.\n"
		"Usage: sv_maxrate_lan 30000\n"
		"Default is 100000.");
	pConsole->CreateVariable("sv_netstats","0",0,
		"Toggles network statistics.\n"
		"Usage: sv_netstats [0..7]\n"
		"Default is 0 (off).\n"
		"Bit 0 (value 1) display net statistics\n"
		"Bit 1 (value 2) display a updatecount graph\n"
		"Bit 2 (value 4) log netentities sent/count");
	pConsole->CreateVariable("sv_max_scheduling_delay","200",0,
		"Sets the scheduling delay upper limit for fixed timestep multiplayer physics (in milliseconds).\n"
		"Usage: sv_max_scheduling_delay 200"
		"Default value is 200.");
	pConsole->CreateVariable("sv_min_scheduling_delay","100",0,
		"Sets the scheduling delay lower limit for fixed timestep multiplayer physics (in milliseconds).\n"
		"Usage: sv_min_scheduling_delay 200"
		"Default value is 200.");

	string szServerName;
	if (m_pSystem->IsDedicated())
		szServerName = string(m_pSystem->GetUserName()) + string("'s FarCry Server");
	pConsole->CreateVariable("sv_name", szServerName.c_str(),0,
		"Specifies the server name in a multiplayer game.\n"
		"Usage: sv_name name\n");
	g_GameType= pConsole->CreateVariable("g_GameType","Default",VF_REQUIRE_NET_SYNC,
		"Sets the game type.\n"
		"Usage: g_GameType [Default/FFA/TDM/ASSAULT]\n"
		"Default game type is 'default'.");	
	g_LeftHanded = pConsole->CreateVariable("g_LeftHanded", "0", 0, "Sets left-handed 1st person weapons");

	cl_ThirdPersonRange=pConsole->CreateVariable("ThirdPersonRange","7",VF_CHEAT,
		"Sets the range of the third person camera.\n"
		"Usage: cl_ThirdPersonRange 6\n"
		"Default is 7 metres.  This is the distance\n"
		"of the third person camera from the player.");		

	cl_ThirdPersonRangeBoat=pConsole->CreateVariable("ThirdPersonRangeBoat","12",VF_CHEAT,
		"Sets the range of the third person camera for boats.\n"
		"Usage: cl_ThirdPersonRangeBoat 6\n"
		"Default is 12 metres.  This is the distance\n"
		"of the third person camera from the player.");

	cl_ThirdPersonAngle=pConsole->CreateVariable("ThirdPersonAngle",".2",VF_CHEAT);

	cl_ThirdPersonOffs=pConsole->CreateVariable("ThirdPersonOffs","1.7",VF_CHEAT);
	cl_ThirdPersonOffsAngHor=pConsole->CreateVariable("ThirdPersonOffsAngHor","30.0",VF_CHEAT);
	cl_ThirdPersonOffsAngVert=pConsole->CreateVariable("ThirdPersonOffsAngVert","30.0",VF_CHEAT);


	// RCon -----------------
	pConsole->CreateVariable("cl_rcon_serverip","",0,
		"RCon (RemoteControl) ip adress\n"
		"Usage: cl_rcon_port 192.168.0.123\n"
		"Default: ''\n");
	pConsole->CreateVariable("cl_rcon_port","49001",0,
		"RCon (RemoteControl) port number\n"
		"Usage: cl_rcon_port 49001\n"
		"Default: 49001\n");
	pConsole->CreateVariable("cl_rcon_password","",0,
		"RCon (RemoteControl) client password (specify the RCon server password)\n"
		"Usage: cl_rcon_password mypassword\n");
	pConsole->CreateVariable("sv_rcon_password","",0,
		"RCon (RemoteControl) server password (if there is no password specified RCon is not activated for this server)\n"
		"Usage: sv_rcon_password mypassword\n");
	// -----------------------


	g_Render= pConsole->CreateVariable("g_Render","1",0,
		"\n"
		"Usage:\n"
		"");

	cl_ViewFace=pConsole->CreateVariable("cl_ViewFace","0",CVAR_FLOAT,
		"\n"
		"Usage: \n"
		"");
	cl_display_hud = pConsole->CreateVariable("cl_display_hud","1",0,
    "Toggles the head up display (HUD).\n"
		"Usage: cl_display_hud [0/1]\n"
		"Default is 1 (HUD on).");  
  cl_motiontracker = pConsole->CreateVariable("cl_motiontracker","1",0,
    "Toggles the motion tracker.\n"
    "Usage: cl_motiontracker [0/1]\n"
    "Default is 1 (tracker on).");  
  cl_hud_pickup_icons = pConsole->CreateVariable("cl_hud_pickup_icons","1",0,
    "Toggles the display of pickup icons (HUD).\n"
    "Usage: cl_hud_pickup_icons [0/1]\n"
    "Default is 1 (HUD on).");
  cl_msg_notification = pConsole->CreateVariable("cl_msg_notification","1",0,
    "Toggles the hud messages sound notification (HUD).\n"
    "Usage: cl_msg_notification [0/1]\n"
    "Default is 1 (notification sound on).");
	cl_hud_name = pConsole->CreateVariable("cl_hud_name","hud.lua",0,
		"Sets the name of the HUD script.\n"
		"Usage: cl_hud_name Scriptname.lua\n"
		"Default is 'hud.lua'.");
	pConsole->CreateVariable("cl_password","",0,
		"Sets the client password to join a password protected server.\n"
		"It must match the server password. Otherwise you will get disconnected\n");
	p_name= pConsole->CreateVariable("p_name","Jack Carver",VF_DUMPTODISK,
		"Sets the player's name.\n"
		"Usage: p_name <playername>\n");
	p_color= pConsole->CreateVariable("p_color","4",VF_DUMPTODISK,
		"Sets the player's color in non team base multiplayer mods.\n"
		"Usage: p_color [0|1|2|3|4|5|6|7|8|9]\n"
		"Default is '4'.");
	p_model= pConsole->CreateVariable("p_model", "objects/characters/pmodels/hero/hero.cgf", VF_DUMPTODISK,
		"Sets the player model.\n"
		"Usage: p_model <modelpath>\n"
		"Default is 'objects/characters/pmodels/hero/hero.cgf'.");
	mp_model= pConsole->CreateVariable("mp_model", "objects/characters/pmodels/hero/hero_mp.cgf", VF_DUMPTODISK,
		"Sets the multiplayer player model.\n"
		"Usage: mp_model <modelpath>\n"
		"Default is 'objects/characters/pmodels/hero/hero_mp.cgf'.");
	g_LevelName= pConsole->CreateVariable("g_LevelName","0.4",0);
	GetISystem()->GetIConsole()->CreateVariable("g_LevelStated","0",0, "");
	sv_timeout= pConsole->CreateVariable("sv_timeout","60",0, // until entity loading speeds up :)	
		"Sets the server timeout (seconds).\n"
		"Usage: sv_timeout 60\n"
		"The default timeout is 60 seconds. This is the time the\n"
		"server waits while trying to establish a connection with\n"
		"a client."); 

	g_StartLevel= pConsole->CreateVariable("g_StartLevel","DEFAULT",0,
		"\n"
		"Usage: \n"
		"");	
	g_StartMission= pConsole->CreateVariable("g_StartMission","",0,
		"\n"
		"Usage: \n"
		"");	

	p_HitImpulse= pConsole->CreateVariable("p_hit_impulse","0.01",0, // don't set to 0
		"\n"
		"Usage: \n"
		"");  
	p_DeadBody= pConsole->CreateVariable("p_deadbody","1",0,
		"\n"
		"Usage: \n"
		"");

	//////////////////////////////////////////////////////////////////////////
	
	ICryPak	*pPak=m_pSystem->GetIPak();
	if (pPak)
	{
		FILE *fTest=pPak->FOpen("Languages/Voicepacks/Jack/jump_46.wav","rb");
		if (fTest)
		{
			g_InstallerVersion->ForceSet("46");
			pPak->FClose(fTest);
		}
		else
		{		
			fTest=pPak->FOpen("Languages/Voicepacks/Jack/jump_48.wav","rb");
			if (fTest)
			{
				g_InstallerVersion->ForceSet("48");
				pPak->FClose(fTest);
			}
			else
				g_InstallerVersion->ForceSet("44");
		}
	}

	if (g_InstallerVersion->GetIVal()==46)		
	{			
		g_Gore = pConsole->CreateVariable("g_gore","0",VF_DUMPTODISK|VF_READONLY);
		g_Gore->ForceSet("0");
	}
	else
	if (g_InstallerVersion->GetIVal()==48)		
	{			
		g_Gore = pConsole->CreateVariable("g_gore","1",VF_DUMPTODISK|VF_READONLY);
		g_Gore->ForceSet("1");
	}
	else
	if (g_InstallerVersion->GetIVal()==44)		
	{
		g_Gore = pConsole->CreateVariable("g_gore","2",VF_DUMPTODISK);
	}
	else
	{
		g_Gore = pConsole->CreateVariable("g_gore","0",VF_DUMPTODISK|VF_READONLY);
		g_Gore->ForceSet("0");
	}

	//////////////////////////////////////////////////////////////////////////


	//everything related to vehicle will be in another file 
	//should be the same for other cvars, code and includes,
	InitVehicleCvars();
	m_jump_vel = pConsole->CreateVariable("m_jump_vel","15.4",VF_CHEAT);
	m_jump_arc = pConsole->CreateVariable("m_jump_arc","25.4",VF_CHEAT);
	p_speed_run = pConsole->CreateVariable("p_speed_run","3.4",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_sprint_scale = pConsole->CreateVariable("p_sprint_scale","1.7",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_sprint_decoy = pConsole->CreateVariable("p_sprint_decoy","20.0",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_sprint_restore_run = pConsole->CreateVariable("p_sprint_restore_run","7.0",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_sprint_restore_idle = pConsole->CreateVariable("p_sprint_restore_idle","14.0",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_speed_walk = pConsole->CreateVariable("p_speed_walk","2.1",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_speed_crouch = pConsole->CreateVariable("p_speed_crouch","1.0",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_speed_prone = pConsole->CreateVariable("p_speed_prone","0.4",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_jump_force = pConsole->CreateVariable("p_jump_force","4.4",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_jump_walk_time = pConsole->CreateVariable("p_jump_walk_time",".2",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_jump_run_time = pConsole->CreateVariable("p_jump_run_time",".4",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_jump_run_d = pConsole->CreateVariable("p_jump_run_d","3.5",VF_CHEAT);
	p_jump_run_h = pConsole->CreateVariable("p_jump_run_h","1.1",VF_CHEAT);
	p_jump_walk_d = pConsole->CreateVariable("p_jump_walk_d","1.5",VF_CHEAT);
	p_jump_walk_h = pConsole->CreateVariable("p_jump_walk_h","1.0",VF_CHEAT);
//	p_lean = pConsole->CreateVariable("p_lean","20.0",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_lean_offset = pConsole->CreateVariable("p_lean_offset",".33",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_bob_pitch = pConsole->CreateVariable("p_bob_pitch","0.2",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_bob_roll = pConsole->CreateVariable("p_bob_roll","0.2",VF_REQUIRE_NET_SYNC|VF_CHEAT);
	p_bob_length = pConsole->CreateVariable("p_bob_length","7.4",VF_REQUIRE_NET_SYNC);
	p_bob_weapon = pConsole->CreateVariable("p_bob_weapon",".01",VF_REQUIRE_NET_SYNC);
	p_bob_fcoeff = pConsole->CreateVariable("p_bob_fcoeff","15.0",VF_REQUIRE_NET_SYNC);
	p_gravity_modifier = pConsole->CreateVariable("p_gravity_modifier","1.0",VF_REQUIRE_NET_SYNC|VF_CHEAT);

	p_restorehalfhealth = pConsole->CreateVariable("p_restore_halfhealth","0",VF_CHEAT);

	p_deathtime = pConsole->CreateVariable("p_deathtime","30.0",VF_REQUIRE_NET_SYNC|VF_DUMPTODISK);

	p_waterbob_pitch = pConsole->CreateVariable("p_waterbob_pitch","0.1",0,
		"\n"
		"Usage: \n"
		"");
	p_waterbob_pitchspeed = pConsole->CreateVariable("p_waterbob_pitchspeed","0.2",0,
		"\n"
		"Usage: \n"
		"");
	p_waterbob_roll = pConsole->CreateVariable("p_waterbob_roll","6.1",0,
		"\n"
		"Usage: \n"
		"");
	p_waterbob_rollspeed = pConsole->CreateVariable("p_waterbob_rollspeed","0.35",0,
		"\n"
		"Usage: \n"
		"");
	p_waterbob_mindepth = pConsole->CreateVariable("p_waterbob_mindepth","0.5",0,
		"\n"
		"Usage: \n"
		"");
	p_weapon_switch = pConsole->CreateVariable("p_weapon_switch","0",0,
		"Toggles auto switch when a player picks up a weapon.\n"
		"Usage: p_weapon_switch [0/1]\n"
		"Default is 0 (no automatic weapon switch).");

	w_recoil_speed_up = 10;
	w_recoil_speed_down = 20;
	w_recoil_max_degree = 12.0f;
	w_accuracy_decay_speed = 18.0f;
	w_accuracy_gain_scale = 2.0f;
	w_recoil_vertical_only = 0;
/*
	pConsole->Register("w_recoil_speed_up",&w_recoil_speed_up,10);
	pConsole->Register("w_recoil_speed_down",&w_recoil_speed_down,20);
	pConsole->Register("w_recoil_max_degree",&w_recoil_max_degree,12.f,VF_CHEAT);
	pConsole->Register("w_accuracy_decay_speed",&w_accuracy_decay_speed,18.f,VF_CHEAT);
	pConsole->Register("w_accuracy_gain_scale",&w_accuracy_gain_scale,2.0f);
	pConsole->Register("w_recoil_vertical_only",&w_recoil_vertical_only,0);
*/
	p_ai_runspeedmult = pConsole->CreateVariable("p_ai_runspeedmult","3.63",0,
		"\n"
		"Usage: \n"
		"");
	p_ai_crouchspeedmult = pConsole->CreateVariable("p_ai_crouchspeedmult","0.8",0,
		"\n"
		"Usage: \n"
		"");
	p_ai_pronespeedmult = pConsole->CreateVariable("p_ai_pronespeedmult","0.5",0,
		"\n"
		"Usage: \n"
		"");
	p_ai_rrunspeedmult = pConsole->CreateVariable("p_ai_rrunspeedmult","3.63",0,
		"\n"
		"Usage: \n"
		"");
	p_ai_rwalkspeedmult = pConsole->CreateVariable("p_ai_rwalkspeedmult","0.81",0,
		"\n"
		"Usage: \n"
		"");
	p_ai_xrunspeedmult = pConsole->CreateVariable("p_ai_xrunspeedmult","1.5",0,
		"\n"
		"Usage: \n"
		"");
	p_ai_xwalkspeedmult = pConsole->CreateVariable("p_ai_xwalkspeedmult","0.94",0,
		"\n"
		"Usage: \n"
		"");

	p_lightrange = pConsole->CreateVariable("p_lightrange","15.0",VF_DUMPTODISK,
		"\n"
		"Usage: \n"
		"");
	p_lightfrustum = pConsole->CreateVariable("p_lightfrustum","30.0",0,
		"\n"
		"Usage: \n"
		"");

	// player animation control parametrs
	pa_leg_idletime = pConsole->CreateVariable("pa_leg_idletime","1",0,
		"\n"
		"Usage: \n"
		"");
	pa_leg_velmoving = pConsole->CreateVariable("pa_leg_velmoving","350",0,
		"\n"
		"Usage: \n"
		"");
	pa_leg_velidle = pConsole->CreateVariable("pa_leg_velidle","170",0,
		"\n"
		"Usage: \n"
		"");
	pa_leg_limitidle = pConsole->CreateVariable("pa_leg_limitidle","110",0,
		"\n"
		"Usage: \n"
		"");
	pa_leg_limitaim = pConsole->CreateVariable("pa_leg_limitaim","45",0,
		"\n"
		"Usage: \n"
		"");
	pa_blend0 = pConsole->CreateVariable("pa_blend0","0.3",0,
		"\n"
		"Usage: \n"
		"");
	pa_blend1 = pConsole->CreateVariable("pa_blend1","0.1",0,
		"\n"
		"Usage: \n"
		"");
//	pa_blend2 = pConsole->CreateVariable("pa_blend2","0.4",0);
	pa_blend2 = pConsole->CreateVariable("pa_blend2","-1.0",0,
		"\n"
		"Usage: \n"
		"");
	pa_forcerelax = pConsole->CreateVariable("pa_forcerelax","0",0,
		"\n"
		"Usage: \n"
		"");
	pa_spine = pConsole->CreateVariable("pa_spine",".3",0,
		"\n"
		"Usage: \n"
		"");
	pa_spine1 = pConsole->CreateVariable("pa_spine1",".4",0,
		"\n"
		"Usage: \n"
		"");

	p_limp = pConsole->CreateVariable("p_limp","0",0,
		"\n"
		"Usage: \n"
		"");

	pl_force = pConsole->CreateVariable("pl_force","0",0,
		"\n"
		"Usage: \n"
		"");
	pl_dist = pConsole->CreateVariable("pl_dist","600",0,
		"\n"
		"Usage: \n"
		"");
	pl_intensity = pConsole->CreateVariable("pl_intensity","1",0,
		"\n"
		"Usage: \n"
		"");
	pl_fadescale = pConsole->CreateVariable("pl_fadescale","1.3",0,
		"\n"
		"Usage: \n"
		"");
	pl_head = pConsole->CreateVariable("pl_head","0",0,
		"\n"
		"Usage: \n"
		"");

	p_RotateHead = pConsole->CreateVariable("p_rotate_head","1",0,
		"\n"
		"Usage: \n"
		"");
	p_RotateMove = pConsole->CreateVariable("p_rotate_Move","0",0,
		"\n"
		"Usage: \n"
		"");
	p_HeadCamera = pConsole->CreateVariable("p_head_camera","0",0,
		"\n"
		"Usage: \n"
		"");
	p_EyeFire	= pConsole->CreateVariable("p_eye_fire","1",0,
		"\n"
		"Usage: \n"
		"");
	a_DrawArea = pConsole->CreateVariable("a_draw_area","0",VF_CHEAT,
		"\n"
		"Usage: \n"
		"");	
	a_LogArea = pConsole->CreateVariable("a_log_area","0",VF_CHEAT,
		"\n"
		"Usage: \n"
		"");	
	pConsole->CreateVariable("game_DifficultyLevel","1",VF_SAVEGAME,
		"0 = easy, 1 = normal, 2 = hard");
	cv_game_Difficulty = pConsole->CreateVariable("game_AdaptiveDifficulty","0",VF_SAVEGAME,
		"0=off, 1=on\n"
		"Usage: \n"
		"");	
	cv_game_Aggression = pConsole->CreateVariable("game_Aggression","1",VF_SAVEGAME,
		"Factor to scale the ai agression, default = 1.0\n"
		"Usage: cv_game_Aggression 1.2\n"
		"");	
	cv_game_Accuracy = pConsole->CreateVariable("game_Accuracy","1",VF_SAVEGAME,
		"Factor to scale the ai accuracy, default = 1.0\n"
		"Usage: game_Accuracy 1.2\n"
		"");
	cv_game_AllowAIMovement = pConsole->CreateVariable("game_Allow_AI_Movement","1",0,
		"Allow or not allow actual AI movement - AI will still think its moving (default 1)\n"
		"Usage: game_Allow_AI_Movement (1/0)\n"
		"");	
	cv_game_AllAIInvulnerable= pConsole->CreateVariable("game_AI_Invulnerable","0",0,
		"When set to 1 all AI in the game will become invulnerable (default 0)\n"
		"Usage: game_AI_Invulnerable (1/0)\n"
		"");	

	cv_game_Health = pConsole->CreateVariable("game_Health","1",VF_SAVEGAME,
		"Factor to scale the ai health, default = 1.0\n"
		"Usage: game_Health 1.2\n"
		"");	
	cv_game_GliderGravity=pConsole->CreateVariable("game_GliderGravity","-0.1f",VF_DUMPTODISK,
		"Sets paraglider's gravity.\n"
		"Usage: game_GliderGravity -.1\n");
	cv_game_GliderBackImpulse=pConsole->CreateVariable("game_GliderBackImpulse","2.5f",VF_DUMPTODISK,
		"Sets paraglider's back impulse (heading up).\n"
		"Usage: game_GliderBackImpulse 2.5\n");
	cv_game_GliderDamping=pConsole->CreateVariable("game_GliderDamping","0.15f",VF_DUMPTODISK,
		"Sets paraglider's damping (control/inertia).\n"
		"Usage: game_GliderDamping 0.15\n");
	cv_game_GliderStartGravity=pConsole->CreateVariable("game_GliderStartGravity","-0.8f",VF_DUMPTODISK,
		"Sets initial paraglider's gravity.\n"
		"Usage: game_GliderStartGravity -0.8");

	m_pCVarCheatMode=pConsole->CreateVariable("zz0x067MD4","0",VF_REQUIRE_NET_SYNC);

	g_timedemo_file = pConsole->CreateVariable("demo_file","timedemo",0);

	cv_game_physics_quality = pConsole->CreateVariable("physics_quality","2",VF_REQUIRE_NET_SYNC);

	cv_game_subtitles = pConsole->CreateVariable("game_subtitles","0",0,"toggles game subtitles");

	pl_JumpNegativeImpulse = GetISystem()->GetIConsole()->CreateVariable("JumpNegativeImpulse","0.0f",VF_REQUIRE_NET_SYNC,
		"this represent the downward impulse power applied when the player reach the max height of the jump, 0 means no impulse.\n"
		"Usage: JumpNegativeImpulse 0-100 is a good range to test.\n"
		"Default value is 0, disabled.\n");
}

void CXGame::ResetInputMap()
{
	assert(m_pIActionMapManager);

	m_pIActionMapManager->ResetAllBindings();

	//------------------------------------------------------------------------------------------------- 
	ADD_ACTION(MOVE_LEFT,aamOnHold,"@MoveLeft",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("MOVE_LEFT", ACTIONMAPS_ALL);
	ADD_ACTION(MOVE_RIGHT,aamOnHold,"@MoveRight",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("MOVE_RIGHT", ACTIONMAPS_ALL);
	ADD_ACTION(MOVE_FORWARD,aamOnHold,"@MoveForward",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("MOVE_FORWARD", ACTIONMAPS_NODEAD);

	ADD_ACTION(WALK,aamOnHold,"@Walk",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("WALK", ACTIONMAPS_NODEAD);
	ADD_ACTION(RUNSPRINT,aamOnHold,"@SprintRun",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("RUNSPRINT", ACTIONMAPS_NODEAD);

	ADD_ACTION(MESSAGEMODE,aamOnPress,"@Chat",ACTIONTYPE_MULTIPLAYER, true) SetConfigToActionMap("MESSAGEMODE", ACTIONMAPS_ALL);
	ADD_ACTION(MESSAGEMODE2,aamOnPress,"@TeamChat",ACTIONTYPE_MULTIPLAYER, true) SetConfigToActionMap("MESSAGEMODE2", ACTIONMAPS_ALL);

	//ADD_ACTION(RUN,aamOnDoublePress,"Move Forward2",false) SetConfigToActionMap("RUN", ACTIONMAPS_NODEAD);
	ADD_ACTION(MOVE_BACKWARD,aamOnHold,"@MoveBackward",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("MOVE_BACKWARD", ACTIONMAPS_NODEAD);
	//ADD_ACTION(VEHICLE_BOOST,aamOnDoublePress)
	ADD_ACTION(SCORE_BOARD,aamOnHold,"@ViewScoreboard",ACTIONTYPE_MULTIPLAYER,true) SetConfigToActionMap("SCORE_BOARD", ACTIONMAPS_ALL);
	ADD_ACTION(JUMP,aamOnHold,"@Jump",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("JUMP", "default", "binozoom", "vehicle", "");
	ADD_ACTION(MOVEMODE,aamOnHold,"@Crouch",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("MOVEMODE", ACTIONMAPS_NODEAD);
	ADD_ACTION(MOVEMODE2,aamOnPress,"@Prone",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("MOVEMODE2", ACTIONMAPS_NODEAD);
	//	ADD_ACTION(MOVEMODE2,aamOnDoublePress,"Change Movemode",true)
	ADD_ACTION(LEANLEFT,aamOnHold,"@LeanLeft",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("LEANLEFT", "default", "zoom", "binozoom","");
	ADD_ACTION(LEANRIGHT,aamOnHold,"@LeanRight",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("LEANRIGHT", "default", "zoom", "binozoom","");
	ADD_ACTION(FIRE0,aamOnHold,"@Fire",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("FIRE0", "default", "zoom", "vehicle", "player_dead", "");
	//ADD_ACTION(FIRECANCEL,aamOnHold,"@CancelCurrentTarget",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("FIRECANCEL", "default", "zoom", "vehicle", "");
	ADD_ACTION(USE,aamOnPress,"@Use",ACTIONTYPE_GAME,true) SetConfigToActionMap("USE", ACTIONMAPS_NODEAD);
	ADD_ACTION(TURNLR,aamOnHold,"@TurnLeftRight",0,false) SetConfigToActionMap("TURNLR", ACTIONMAPS_NODEAD);
	ADD_ACTION(TURNUD,aamOnHold,"@TurnUpDown",0,false) SetConfigToActionMap("TURNUD", ACTIONMAPS_NODEAD);
	//	ADD_ACTION(RUN,aamOnHold,"Toggle Run",true)
	ADD_ACTION(NEXT_WEAPON,aamOnPress,"@NextWeapon",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("NEXT_WEAPON", "default", "");
	ADD_ACTION(PREV_WEAPON,aamOnPress,"@PrevWeapon",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("PREV_WEAPON", "default", "");
	ADD_ACTION(RELOAD,aamOnPress,"@Reload",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("RELOAD", ACTIONMAPS_NODEAD);
	//ADD_ACTION(WEAPON_0,aamOnPress,"@Slot0",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("WEAPON_0", ACTIONMAPS_WEAPON);
	ADD_ACTION(DROPWEAPON,aamOnPress,"@DropWeapon",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("DROPWEAPON", "default", "");
	ADD_ACTION(WEAPON_1,aamOnPress,"@Slot1",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("WEAPON_1", "default", "zoom", "binozoom", "");
	ADD_ACTION(WEAPON_2,aamOnPress,"@Slot2",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("WEAPON_2", "default", "zoom", "binozoom", "");
	ADD_ACTION(WEAPON_3,aamOnPress,"@Slot3",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("WEAPON_3", "default", "zoom", "binozoom", "");
	ADD_ACTION(WEAPON_4,aamOnPress,"@Slot4",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("WEAPON_4", "default", "zoom", "binozoom", "");
	/*ADD_ACTION(WEAPON_5,aamOnPress,"@Slot5",true) SetConfigToActionMap("WEAPON_5", ACTIONMAPS_WEAPON);
	ADD_ACTION(WEAPON_6,aamOnPress,"@Slot6",true) SetConfigToActionMap("WEAPON_6", ACTIONMAPS_WEAPON);
	ADD_ACTION(WEAPON_7,aamOnPress,"@Slot7",true) SetConfigToActionMap("WEAPON_7", ACTIONMAPS_WEAPON);
	ADD_ACTION(WEAPON_8,aamOnPress,"@Slot8",true) SetConfigToActionMap("WEAPON_8", ACTIONMAPS_WEAPON);
	*/
	
	ADD_ACTION(CYCLE_GRENADE,aamOnPress,"@CycleGrenade",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("CYCLE_GRENADE", ACTIONMAPS_NODEAD);

	//ADD_ACTION(LOADPOS,aamOnPress,"Load Position",ACTIONTYPE_DEBUG,true) SetConfigToActionMap("LOADPOS", ACTIONMAPS_ALL);
	//ADD_ACTION(SAVEPOS,aamOnPress,"Save Position",ACTIONTYPE_DEBUG,true) SetConfigToActionMap("SAVEPOS", ACTIONMAPS_ALL);
	ADD_ACTION(ITEM_0,aamOnPress,"@Binoculars",ACTIONTYPE_GAME,true) SetConfigToActionMap("ITEM_0", ACTIONMAPS_DEF_ZOOMS);
	ADD_ACTION(ITEM_1,aamOnPress,"@ThermalVision",ACTIONTYPE_GAME,true) SetConfigToActionMap("ITEM_1", ACTIONMAPS_NODEAD);  
	//ADD_ACTION(ITEM_2,aamOnPress,"@Item2",0,false) SetConfigToActionMap("ITEM_2", ACTIONMAPS_NODEAD);
	//ADD_ACTION(ITEM_3,aamOnPress,"@Item3",0,false) SetConfigToActionMap("ITEM_3", ACTIONMAPS_NODEAD);	
	//ADD_ACTION(ITEM_4,aamOnPress,"@Item4",0,false) SetConfigToActionMap("ITEM_4", ACTIONMAPS_NODEAD);
	//ADD_ACTION(ITEM_5,aamOnPress,"@Item5",0,false) SetConfigToActionMap("ITEM_5", ACTIONMAPS_NODEAD);
	//ADD_ACTION(ITEM_6,aamOnPress,"@Item6",0,false) SetConfigToActionMap("ITEM_6", ACTIONMAPS_NODEAD);
	//ADD_ACTION(ITEM_7,aamOnPress,"@Item7",0,false) SetConfigToActionMap("ITEM_7", ACTIONMAPS_NODEAD);
	//ADD_ACTION(ITEM_8,aamOnPress,"@Item8",0,false) SetConfigToActionMap("ITEM_8", ACTIONMAPS_NODEAD);

	ADD_ACTION(ZOOM_TOGGLE,aamOnPressAndRelease,"@ToggleZoom",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("ZOOM_TOGGLE", "default", "zoom", "");
	ADD_ACTION(ZOOM_IN,aamOnPress,"@ZoomIn",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("ZOOM_IN", "zoom", "binozoom", "");
	ADD_ACTION(ZOOM_OUT,aamOnPress,"@ZoomOut",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("ZOOM_OUT", "zoom", "binozoom", "");
	ADD_ACTION(HOLDBREATH,aamOnHold,"@HoldBreath",ACTIONTYPE_GAME,true) SetConfigToActionMap("HOLDBREATH", "zoom", "");
	ADD_ACTION(FIREMODE,aamOnPress,"@ToggleFiremode",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("FIREMODE", ACTIONMAPS_NODEAD);
//	ADD_ACTION(QUICKLOAD,aamOnPress,"@Quickload",ACTIONTYPE_GAME,true) SetConfigToActionMap("QUICKLOAD", ACTIONMAPS_ALL);
//	ADD_ACTION(QUICKSAVE,aamOnPress,"@Quicksave",ACTIONTYPE_GAME,true) SetConfigToActionMap("QUICKSAVE", ACTIONMAPS_ALL);
	ADD_ACTION(FIRE_GRENADE,aamOnHold,"@ThrowGrenade",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("FIRE_GRENADE", ACTIONMAPS_NODEAD);
	//ADD_ACTION(CONCENTRATION,aamOnHold,"@Concentration",ACTIONTYPE_GAME,true) SetConfigToActionMap("CONCENTRATION", "default", "");
	ADD_ACTION(FLASHLIGHT,aamOnPress,"@ToggleFlashlight",ACTIONTYPE_GAME,true) SetConfigToActionMap("FLASHLIGHT", ACTIONMAPS_NODEAD);
	ADD_ACTION(CHANGE_VIEW,aamOnPress,"@SwitchView",ACTIONTYPE_GAME,true) SetConfigToActionMap("CHANGE_VIEW", "default", "vehicle", "");
	ADD_ACTION(TAKESCREENSHOT,aamOnPress,"@TakeScreenshot",ACTIONTYPE_GAME,true) SetConfigToActionMap("TAKESCREENSHOT", ACTIONMAPS_ALL);

	ADD_ACTION(MOVEMODE_TOGGLE,aamOnPress,"@CrouchToggle",ACTIONTYPE_MOVEMENT,true) SetConfigToActionMap("MOVEMODE_TOGGLE", ACTIONMAPS_NODEAD);
	ADD_ACTION(AIM_TOGGLE,aamOnPress,"@ToggleAim",ACTIONTYPE_COMBAT,true) SetConfigToActionMap("AIM_TOGGLE", "default", "zoom", "");

	//default action map
	//////////////////////////////////////////////////////////////////////
	IActionMap *pMap=m_pIActionMapManager->CreateActionMap("default");	

	SetCommonKeyBindings(pMap);

	//change to distinct weapons
	pMap->BindAction(ACTION_WEAPON_0,XKEY_0);
	pMap->BindAction(ACTION_WEAPON_1,XKEY_1);
	pMap->BindAction(ACTION_WEAPON_2,XKEY_2);
	pMap->BindAction(ACTION_WEAPON_3,XKEY_3);
	pMap->BindAction(ACTION_WEAPON_4,XKEY_4);
	/*	pMap->BindAction(ACTION_WEAPON_5,XKEY_5);
	pMap->BindAction(ACTION_WEAPON_6,XKEY_6);
	pMap->BindAction(ACTION_WEAPON_7,XKEY_7);
	pMap->BindAction(ACTION_WEAPON_8,XKEY_8);
	*/
	//Scroll up through weapons	
	pMap->BindAction(ACTION_PREV_WEAPON,XKEY_PAGE_UP);
	//Scroll down through weapons	
	pMap->BindAction(ACTION_NEXT_WEAPON,XKEY_PAGE_DOWN);

	//lean (not active in zoom mode)
	//lean left
	pMap->BindAction(ACTION_LEANLEFT,XKEY_Q);
	pMap->BindAction(ACTION_LEANLEFT,XKEY_NUMPAD7);

	//lean right
	pMap->BindAction(ACTION_LEANRIGHT,XKEY_E);
	pMap->BindAction(ACTION_LEANRIGHT,XKEY_NUMPAD9);

	//fire (outside common key bindings because is not possible in binozoom)
	pMap->BindAction(ACTION_FIRE0,XKEY_MOUSE1);

	//jump (outside common key bindings because space is reserved for hold the breath 
	//in zoom mode) 
	pMap->BindAction(ACTION_JUMP,XKEY_SPACE);	
	pMap->BindAction(ACTION_JUMP,XKEY_NUMPADENTER);

	//scroll up/down through weapons 
	//mouse wheel up/down is reserved for zoom in other zoom mode
	pMap->BindAction(ACTION_NEXT_WEAPON,XKEY_MWHEEL_UP); 
	pMap->BindAction(ACTION_PREV_WEAPON,XKEY_MWHEEL_DOWN); 

	//binocular/motion tracker/listening device 
	//(should not be enabled if already in zoom mode)

	//aimed mode/scoped mode (use zoom)		
	pMap->BindAction(ACTION_ZOOM_TOGGLE,XKEY_MOUSE2);

	//drop weapon
	pMap->BindAction(ACTION_DROPWEAPON,XKEY_J);

	//vision modes
	//binocular/motion tracker/listening device 
	pMap->BindAction(ACTION_ITEM_0, XKEY_B);

	// thermal vision
	pMap->BindAction(ACTION_ITEM_1, XKEY_T);

	pMap->BindAction(ACTION_CHANGE_VIEW,XKEY_F1);

	//////////////////////////////////////////////////////////////////////
	pMap=m_pIActionMapManager->CreateActionMap("zoom");

	SetCommonKeyBindings(pMap);

	// weapon switch
	pMap->BindAction(ACTION_WEAPON_1,XKEY_1);
	pMap->BindAction(ACTION_WEAPON_2,XKEY_2);
	pMap->BindAction(ACTION_WEAPON_3,XKEY_3);
	pMap->BindAction(ACTION_WEAPON_4,XKEY_4);

	//vision modes
	//binocular/motion tracker/listening device 
	pMap->BindAction(ACTION_ITEM_0,XKEY_B); 

	//lean (not active in zoom mode)
	//lean left
	pMap->BindAction(ACTION_LEANLEFT,XKEY_Q);
	pMap->BindAction(ACTION_LEANLEFT,XKEY_NUMPAD7);

	//lean right
	pMap->BindAction(ACTION_LEANRIGHT,XKEY_E);
	pMap->BindAction(ACTION_LEANRIGHT,XKEY_NUMPAD9);
	//zoom 
	pMap->BindAction(ACTION_ZOOM_IN,XKEY_MWHEEL_UP); 
	pMap->BindAction(ACTION_ZOOM_IN,XKEY_ADD);

	pMap->BindAction(ACTION_ZOOM_OUT,XKEY_MWHEEL_DOWN); 
	pMap->BindAction(ACTION_ZOOM_OUT,XKEY_SUBTRACT);

	//fire (outside common key bindings because is not possible in binozoom)
	pMap->BindAction(ACTION_FIRE0,XKEY_MOUSE1);

	//hold the breath
	pMap->BindAction(ACTION_HOLDBREATH,XKEY_SPACE); 

	//aimed mode/scoped mode (use zoom)		
	pMap->BindAction(ACTION_ZOOM_TOGGLE,XKEY_MOUSE2);

	// rmb is used for binocular, night vision & thermal vision 
	pMap->BindAction(ACTION_ITEM_1,XKEY_T); //thermal vision	

	//////////////////////////////////////////////////////////////////////
	pMap=m_pIActionMapManager->CreateActionMap("binozoom");

	SetCommonKeyBindings(pMap);

	// weapon switch
	pMap->BindAction(ACTION_WEAPON_1,XKEY_1);
	pMap->BindAction(ACTION_WEAPON_2,XKEY_2);
	pMap->BindAction(ACTION_WEAPON_3,XKEY_3);
	pMap->BindAction(ACTION_WEAPON_4,XKEY_4);

	//vision modes
	//binocular/motion tracker/listening device 
	pMap->BindAction(ACTION_ITEM_0,XKEY_B); 

	//lean (not active in zoom mode)
	//lean left
	pMap->BindAction(ACTION_LEANLEFT,XKEY_Q);
	pMap->BindAction(ACTION_LEANLEFT,XKEY_NUMPAD7);

	//lean right
	pMap->BindAction(ACTION_LEANRIGHT,XKEY_E);
	pMap->BindAction(ACTION_LEANRIGHT,XKEY_NUMPAD9);

	//change to distinct weapons(outside common key bindings because is not possible in zoom)
	/*
	pMap->BindAction(ACTION_WEAPON_0,XKEY_0);
	pMap->BindAction(ACTION_WEAPON_1,XKEY_1);
	pMap->BindAction(ACTION_WEAPON_2,XKEY_2);
	pMap->BindAction(ACTION_WEAPON_3,XKEY_3);
	pMap->BindAction(ACTION_WEAPON_4,XKEY_4);
	pMap->BindAction(ACTION_WEAPON_5,XKEY_5);
	pMap->BindAction(ACTION_WEAPON_6,XKEY_6);
	pMap->BindAction(ACTION_WEAPON_7,XKEY_7);
	pMap->BindAction(ACTION_WEAPON_8,XKEY_8);
	//Scroll up through weapons	
	pMap->BindAction(ACTION_PREV_WEAPON,XKEY_PAGE_UP);
	//Scroll down through weapons	
	pMap->BindAction(ACTION_NEXT_WEAPON,XKEY_PAGE_DOWN);
	*/

	//zoom
	pMap->BindAction(ACTION_ZOOM_IN,XKEY_MWHEEL_UP); 
	pMap->BindAction(ACTION_ZOOM_IN,XKEY_ADD);

	pMap->BindAction(ACTION_ZOOM_OUT,XKEY_MWHEEL_DOWN); 
	pMap->BindAction(ACTION_ZOOM_OUT,XKEY_SUBTRACT);

	//jump (outside common key bindings because space is reserved for holding the breath 
	//in zoom mode)
	pMap->BindAction(ACTION_JUMP,XKEY_SPACE);	
	pMap->BindAction(ACTION_JUMP,XKEY_NUMPADENTER);

	//thermal vision	
	pMap->BindAction(ACTION_ITEM_1,XKEY_T); 

	//fire (outside common key bindings because is not possible in binozoom)	
	// right now used to scan the AI when in zoom mode
	//pMap->BindAction(ACTION_USE,XKEY_LMB);

	//////////////////////////////////////////////////////////////////////
	pMap=m_pIActionMapManager->CreateActionMap("vehicle");

	SetCommonKeyBindings(pMap);

	//fire (outside common key bindings because is not possible in binozoom)
	pMap->BindAction(ACTION_FIRE0,XKEY_MOUSE1);

	// breaks (actually use - to jump out of the car)
	pMap->BindAction(ACTION_JUMP,XKEY_SPACE);
	pMap->BindAction(ACTION_JUMP,XKEY_NUMPADENTER);

	//accellerate 
	pMap->BindAction(ACTION_VEHICLE_BOOST,XKEY_UP);

	// switch between 1st and 3rd pesron while driving
	//pMap->BindAction(ACTION_CHANGE_VIEW,XKEY_RMB);
	pMap->BindAction(ACTION_CHANGE_VIEW,XKEY_F1);

	//thermal vision	
	pMap->BindAction(ACTION_ITEM_1,XKEY_T); 

	//////////////////////////////////////////////////////////////////////
	pMap=m_pIActionMapManager->CreateActionMap("player_dead");

	pMap->BindAction(ACTION_FIRE0, XKEY_MOUSE1);

	pMap->BindAction(ACTION_MOVE_LEFT,XKEY_LEFT);
	pMap->BindAction(ACTION_MOVE_RIGHT,XKEY_RIGHT);
	pMap->BindAction(ACTION_TAKESCREENSHOT,XKEY_F12);
//	pMap->BindAction(ACTION_QUICKLOAD,XKEY_F6);

	//////////////////////////////////////////////////////////////////////
	//switch to default action map now

	m_pIActionMapManager->SetActionMap("default");
}

//////////////////////////////////////////////////////////////////////
// Define all actions and bind default keys.
void CXGame::InitInputMap()
{
	IInput *pInput = m_pSystem->GetIInput();

	m_pIActionMapManager = pInput->CreateActionMapManager();

	ResetInputMap();
}

//////////////////////////////////////////////////////////////////////////
// Bind a key to a action in a specified actionmap.
void CXGame::BindAction(const char *sAction,const char *sKeys,const char *sActionMap, int iKeyPos)
{
	ActionsEnumMapItor itor;
	// find action
	itor=m_mapActionsEnum.find(sAction);
	if(itor==m_mapActionsEnum.end())
		return;
	IActionMap *pMap=NULL;
	// if no actionmap specified we use the default one
	if(!sActionMap)
		pMap=m_pIActionMapManager->GetActionMap("default");
	else
		pMap=m_pIActionMapManager->GetActionMap(sActionMap);
	// bind
	pMap->BindAction(itor->second.nId,sKeys, 0, iKeyPos);
}
																																				 
//////////////////////////////////////////////////////////////////////////
// Binds a key (or other input) to a certain action (eg. JUMP) in all action-maps associated with this action
void CXGame::BindActionMultipleMaps(const char *sAction,const char *sKeys, int iKeyPos)
{
	XBind Bind;
	Bind.nKey=m_pSystem->GetIInput()->GetKeyID(sKeys);
	// find action
	ActionsEnumMapItor It=m_mapActionsEnum.find(sAction);
	if (It!=m_mapActionsEnum.end())
	{
		ActionInfo &Info=It->second;
		for (Vec2StrIt Itor=Info.vecSetToActionMap.begin();Itor!=Info.vecSetToActionMap.end();++Itor)
		{
			for (ActionsEnumMapItor RemIt=m_mapActionsEnum.begin();RemIt!=m_mapActionsEnum.end();++RemIt)
			{
				if (RemIt->second.nId!=Info.nId)
				{
					for (Vec2StrIt RemItor=RemIt->second.vecSetToActionMap.begin();RemItor!=RemIt->second.vecSetToActionMap.end();++RemItor)
					{
						if (strcmp((*RemItor).c_str(), (*Itor).c_str())==0)
						{
							for (Vec2StrIt RemItor=RemIt->second.vecSetToActionMap.begin();RemItor!=RemIt->second.vecSetToActionMap.end();++RemItor)
							{
								IActionMap *pMap=m_pIActionMapManager->GetActionMap((*RemItor).c_str());
								if (pMap)
									pMap->RemoveBind(RemIt->second.nId, Bind, RemIt->second.ActivationMode);
							}
							goto NextAction;
						}
					}
				}
NextAction:;
			}
			BindAction(sAction, sKeys, (*Itor).c_str(), iKeyPos);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Check if a action is triggered.
bool CXGame::CheckForAction(const char *sAction)
{
	ActionsEnumMapItor itor;
	// find action
	itor=m_mapActionsEnum.find(sAction);
	if(itor==m_mapActionsEnum.end())
		return false;
	// triggered ?
	if(m_pIActionMapManager->CheckActionMap(itor->second.nId))
		return true;
	return false;
} 

//////////////////////////////////////////////////////////////////////
// Clears a action-flag (untrigger)
void CXGame::ClearAction(const char *sAction)//<<FIXME>> remove
{

}
