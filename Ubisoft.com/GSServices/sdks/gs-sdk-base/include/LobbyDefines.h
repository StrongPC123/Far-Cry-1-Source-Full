/***SDOC*******************************************************************************************
 *                                UbiSoft Network Development
 *                                ---------------------------
 *
 * FILE........: LobbyDefines.h
 * CREATION....: October 2001
 * AUTHOR......: GS Dev
 *
 * DESCRIPTION.: Miscelleanous definitions for the Lobby server 
 *
 ******************************************************************************************EDOC***/
#ifndef __LOBBYDEFINES_H__
#define __LOBBYDEFINES_H__

#include "GSTypes.h"


//-------------------- Group Type ------------------------------
//             Constant                      ---->     Name in gslobbyserver.conf
const GSushort LOBBY                          = 0;  //"LOBBY"                      
const GSushort ROOM_DIRECTPLAY_CLIENTSERVER   = 1;  //"DIRECTPLAY_CLIENTSERVER"
const GSushort ROOM_DIRECTPLAY_P2P            = 2;  //"DIRECTPLAY_P2P"
const GSushort ROOM_HYBRID                    = 3;  //"HYBRID"
const GSushort ROOM_HYBRID_REGSERVER          = 4;  //"HYBRID_REGSERVER"
const GSushort ROOM_UBI_CLIENTHOST            = 5;  //"UBI_CLIENTHOST"
const GSushort ROOM_UBI_CLIENTHOST_REGSERVER  = 6;  //"UBI_CLIENTHOST_REGSERVER"
const GSushort ROOM_UBI_P2P                   = 7;  //"UBI_P2P"
const GSushort ROOM_UBI_GAMESERVER            = 8;  //"UBI_GAMESERVER"
const GSushort ROOM_UBI_GAMESERVER_REGSERVER  = 9;  //"UBI_GAMESERVER_REGSERVER"
const GSushort ROOM_REGSERVER                 = 10; //"REGISTER_SERVER"

//------------------------ Group and Game Masks  ------------------------------
const GSuint LSM_PRIVATE                    = 1 << 0;   //The group is protected by a password
const GSuint LSM_NEEDMASTER                 = 1 << 1;   //The group need a master 
const GSuint LSM_ETERNEL                    = 1 << 2;   //The group is eternel ie when no player is on the group it is not deleted
const GSuint LSM_ACTIVE                     = 1 << 3;   //The game is started
const GSuint LSM_OPEN                       = 1 << 4;   //The group is open 
const GSuint LSM_STARTABLE                  = 1 << 5;   //The group can be started 
const GSuint LSM_MATCHACTIVE                = 1 << 12;  //The match is started

const GSuint LSM_CREATE_SUBLOBBY            = 1 << 9;   //Allow to create sublobby
const GSuint LSM_OPEN_WHEN_ACTIVE           = 1 << 10;  //When the game is started the group is still open
const GSuint LSM_SCORES_SUBMISSION          = 1 << 11;  //Allow score submission
const GSuint LSM_DEDICATEDSERVER            = 1 << 14;  //The group represent a dedicated server 
const GSuint LSM_REGISTERSERVER             = 1 << 13; 
const GSuint LSM_JOINRULE                   = 1 << 15;  //The access to the group is protected by a rule ( use with passport )
const GSuint LSM_CREATERULE                 = 1 << 16;  //The group caretion is restricted by a rule ( use with passport )


//--------------- Join info Masks ----------------------------------------------------
const GSuint LSM_GROUPINFO                  = 1 << 6;   //Get the group info 
const GSuint LSM_GROUPMEMBERS               = 1 << 7;   //Get the group members 
const GSuint LSM_CHILDGROUPINFO             = 1 << 8;   //Get the childs group info

const GSuint LSM_ALLINFO                    = LSM_GROUPINFO | LSM_GROUPMEMBERS | LSM_CHILDGROUPINFO;

//------------------- Player Status ----------------------------
const GSushort PS_SILENT                    = 1 << 0;   //The player is limited ( doesn't access to chat, page, etc ... )
const GSushort PS_GAMECONNECTED             = 1 << 1;   //The player is playing a game
const GSushort PS_GAMEREADY                 = 1 << 2;   //not implemented yet
const GSushort PS_MATCHREADY                = 1 << 3;   //not implemented yet 
const GSushort PS_MATCHPLAYING              = 1 << 4;   //The player is playing a match

//-------------------- error messages  -------------------------
const GSushort    ERRORLOBBYSRV_UNKNOWNERROR                    = 0;
const GSushort    ERRORLOBBYSRV_GROUPNOTEXIST                   = 1;
const GSushort    ERRORLOBBYSRV_GAMENOTALLOWED                  = 2;
const GSushort    ERRORLOBBYSRV_SPECTATORNOTALLOWED             = 4;
const GSushort    ERRORLOBBYSRV_NOMOREPLAYERS                   = 5;
const GSushort    ERRORLOBBYSRV_NOMORESPECTATORS                = 6;
const GSushort    ERRORLOBBYSRV_NOMOREMEMBERS                   = 7;
const GSushort    ERRORLOBBYSRV_MEMBERNOTREGISTERED             = 8;
const GSushort    ERRORLOBBYSRV_GAMEINPROGRESS                  = 9;
const GSushort    ERRORLOBBYSRV_WRONGGAMEVERSION                = 10;
const GSushort    ERRORLOBBYSRV_PASSWORDNOTCORRECT              = 11;
const GSushort    ERRORLOBBYSRV_ALREADYINGROUP                  = 12;
const GSushort    ERRORLOBBYSRV_NOTMASTER                       = 13;
const GSushort    ERRORLOBBYSRV_NOTINGROUP                      = 14;
const GSushort    ERRORLOBBYSRV_MINPLAYERSNOTREACH              = 15;
const GSushort    ERRORLOBBYSRV_CONNECTADDCONNECTION            = 16;
const GSushort    ERRORLOBBYSRV_CONNECTSENDLOGINMSG             = 17;
const GSushort    ERRORLOBBYSRV_ERRORLOGINMESSAGE               = 18;
const GSushort    ERRORLOBBYSRV_NOHOSTLOBBYSERVER               = 19;
const GSushort    ERRORLOBBYSRV_LOBBYSRVDISCONNECTED            = 20;
const GSushort    ERRORLOBBYSRV_INVALIDGROUPNAME                = 21;
const GSushort    ERRORLOBBYSRV_INVALIDGAMETYPE                 = 22;
const GSushort    ERRORLOBBYSRV_NOMOREGAMEMODULE                = 23;
const GSushort    ERRORLOBBYSRV_CREATENOTALLOWED                = 24;
const GSushort    ERRORLOBBYSRV_GROUPCLOSE                      = 25;
const GSushort    ERRORLOBBYSRV_WRONGGROUPTYPE                  = 26;
const GSushort    ERRORLOBBYSRV_MEMBERNOTFOUND                  = 27;
const GSushort    ERRORLOBBYSRV_MATCHNOTEXIST                   = 30;
const GSushort    ERRORLOBBYSRV_MATCHNOTFINISHED                = 31;  
const GSushort    ERRORLOBBYSRV_GAMENOTINITIATED                = 32;
const GSushort    ERRORLOBBYSRV_BEGINALREADYDONE                = 33;
const GSushort    ERRORLOBBYSRV_MATCHALREADYFINISHEDFORYOU      = 34;
const GSushort    ERRORLOBBYSRV_MATCHSCORESSUBMISSIONEVENTFAIL  = 35;
const GSushort    ERRORLOBBYSRV_MATCHSCORESSUBMISSIONALREDYSENT = 36; 
const GSushort    ERRORLOBBYSRV_MATCHRESULTSPROCESSNOTFINISHED  = 37;
const GSushort    ERRORLOBBYSRV_MEMBERBANNED                    = 38;
const GSushort    ERRORLOBBYSRV_PASSPORTFAIL                    = 39;
const GSushort    ERRORLOBBYSRV_NOTCREATOR                      = 40;
const GSushort    ERRORLOBBYSRV_GAMENOTFINISHED                 = 41;
const GSushort    ERRORLOBBYSRV_PASSPORTTIMEOUT                 = 42;
const GSushort    ERRORLOBBYSRV_PASSPORTNOTFOUND                = 43;
const GSushort    ERRORLOBBYSRV_GROUPALREADYEXIST               = 44;

#endif

