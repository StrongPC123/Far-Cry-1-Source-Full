//****************************************************************************
//*   Author:  Luc Bouchard  lbouchard@ubisoft.qc.ca
//*   Date:    16/05/2001 11:23:39 AM
 /*!  \file   define.h
  *   \brief  Global defines
  *
  *   This file defines all the global values used by the different
  *   Game Service SDKs.
  */
//****************************************************************************


/*!
\mainpage gs-sdk-common
\section intro Introduction
 This SDK contains the basic libraries used by all the Game Service SDKs


\section description Description
 libgsconnect: connection management<br>
 libgssocket: socket management<br>
 libgsutility: message packaging management and other stuff. <br>


\section see_also See Also
<a href="../../gs-sdk-base/doc/index.html">gs-sdk-base</a><br>
<a href="../../gs-sdk-game/doc/index.html">gs-sdk-game</a><br>
<a href="../../gs-sdk-gameserver/doc/index.html">gs-sdk-gameserver</a><br>
<a href="../../gs-sdk-chat/doc/index.html">gs-sdk-chat</a><br>
*/

#ifndef _DEFINE_H_
#ifndef DOX_SKIP_THIS
#define _DEFINE_H_
#endif //DOX_SKIP_THIS

#include "GSTypes.h"

/*! @defgroup group1 Buffer lenghts
\brief The length of the buffer used throughout the SDKs.

The length of the buffer used throughout the SDKs.
    @{
*/
#define STOREDPROCLENGTH   30
#define NICKNAMELENGTH     16
#define FIRSTNAMELENGTH    33
#define SURNAMELENGTH      33
#define PASSWORDLENGTH     17
#define ADDRESSLENGTH      129
#define CITYLENGTH         33
#define CODELENGTH         33
#define EMAILLENGTH        129
#define COMMENTSLENGTH     1025
#define WEBPAGELENGTH      129
#define GAMELENGTH         33
#define IRCIDLENGTH        10
#define NAMELENGTH         33
#define COUNTRYLENGTH      65
#define CHATLENGTH         1025
#define IPADDRESSLENGTH    129
#define IPDOTADDRESSLENGTH 16
#define GAMELENGTH         33
#define VERSIONLENGTH      33
#define INFOLENGTH         1025
#define FILELENGTH         129
#define ARENALENGTH        33
#define SESSIONLENGTH      33
#define SCORELENGTH        129
#define REASONLENGTH       129
#define URLLENGTH          1025
#define YESNOLENGTH	       4
#define MOTDLENGTH         513
#define LANGUAGELENGTH     3
/*! @} end of group1 */


/*! @defgroup group2 Error/Success
\brief The Error/Success constant

The Error/Success constant
    @{
*/
#ifndef GSSUCCESS
#define GSSUCCESS            38 /* c->r */
#endif

#ifndef GSFAIL
#define GSFAIL               39 /* r->c */
#endif

#ifndef GSPENDING
#define GSPENDING            40
#endif
/*! @} end of group2 */


#define CHARSIZE          1
#define SHORTSIZE         2
#define LONGSIZE          4

/*! @defgroup group3 Player statuses
\brief The possible status for a player

The possible status for a player
    @{
*/
#define PLAYERONLINE         0
#define PLAYEROFFLINE        1 //Only set by Server
#define PLAYERAWAY           2
#define PLAYERBRB            3
//#define PLAYERSESSIONCANJOIN  4
//#define PLAYERSESSIONCANTJOIN  5
#define PLAYERINVISIBLE      6
#define PLAYERCOREONLINE     7
#define PLAYERINLOBBY        8 //Only set by Server
#define PLAYERINROOM         9 //Only set by Server
#define PLAYERINGAMEOPEN     10 //Only set by Server
#define PLAYERINGAMECLOSE    11 //Only set by Server
#define PLAYERSTATUSCOUNT    12
#define PLAYERCORESTART   3000000
#define PLAYERCOREEND     4999999
/*! @} end of group3 */


/*! @defgroup group4 Admin page types
\brief The possible type of a admin page message

The possible type of a admin page message
    @{
*/
// SYSTEMPAGE subtypes
#define ADDEDASFRIEND        0
#define ADDEDASIGNOREE       1
#define REMOVEDASIGNOREE     2
#define ADMINPAGE            185
/*! @} end of group4 */


/*! @defgroup group5 Friends options
\brief The possible options to set when connecting to the FRIENDS service.

The possible options to set when connecting to the FRIENDS service.
    @{
*/
#define MASK_PAGE           (1L<<0)
#define MASK_FILES          (1L<<1)
#define MASK_AUTOFILES      (1L<<2)
#define MASK_INVISIBLE      (1L<<3)
#define MASK_AWAY           (1L<<4)
/*! @} end of group5 */

/*! @defgroup group6 Masks
\brief Masks used throughout the Game Service SDKs.

Masks used throughout the Game Service SDKs.
    @{
*/

/*---------------- session and player mask ----------------------*/
#define MASKSESSIONNAME (1L<<0)
#define MASKSCORE       (1L<<2)
#define MASKADDRESS     (1L<<5)

const GSuint MASKPRIVATE                    = 0x00000001;  //(1L<<0)
const GSuint MASKNEEDMASTER                 = 0x00000002;  //(1L<<1)
const GSuint MASKETERNEL                    = 0x00000004;  //(1L<<2)
const GSuint MASKACTIVE                     = 0x00000008;  //(1L<<3)
const GSuint MASKOPEN                       = 0x00000010;  //(1L<<4)
const GSuint STARTABLE                      = 0x00000020;  //(1L<<5)
const GSuint MASKVISITOR                    = 0x00000040;  //(1L<<6)
const GSuint DEFEREDSTARTGAME               = 0x00000080;  //(1L<<7)
const GSuint MASKPLAYERETERNEL              = 0x00000100;
const GSuint MASKDEDICATEDSERVER            = 0x00080000;  //(1L<<19)

/*! @} end of group6 */


// ERROR CODES IN USE
// 0 @ 56
// 60 @ 68
// -1 @ -3
// 100, 501, 502, 512

/*! @defgroup group7 Error messages
\brief The error message returned by the different services.

The error message returned by the different services.
    @{
*/


/*! @defgroup group7_1 LOGIN service.
\brief Errors returned by the LOGIN service.

Errors returned by the LOGIN service.
    @{
*/
/*! Unknown error. */
#define ERRORROUTER_UNKNOWNERROR             0
/*! You are not registered, create a new account first. */
#define ERRORROUTER_NOTREGISTERED            1
/*! Your password is incorrect. */
#define ERRORROUTER_PASSWORDNOTCORRECT       2
/*! The arena has not yet detected your disconnection. */
#define ERRORROUTER_NOTDISCONNECTED          3
/*! The arena is not available. */
#define ERRORROUTER_ARENANOTAVAILABLE        4
/*! The Friends server is not available. */
#define ERRORROUTER_FRIENDSNOTAVAILABLE      5
/*! A player with the same name as yours is already connected. */
#define ERRORROUTER_NAMEALREADYUSED          6
/*! This player is not currently connected to the GameService. */
#define ERRORROUTER_PLAYERNOTCONNECTED       7
/*! This player is not registered on the GameService. */
#define ERRORROUTER_PLAYERNOTREGISTERED      8
/*! The name you chose is already used by another player. */
#define ERRORROUTER_PLAYERCONNECTED          9
/*! You are already registered. */
#define ERRORROUTER_PLAYERALREADYREGISTERED  10
/*! The version of GSClient you are using is too old and can't be upgraded. */
#define ERRORROUTER_CLIENTVERSIONTOOOLD      11
/*! GS Database problem. Some functions are disabled. */
#define ERRORROUTER_DBINBACKUPMODE           12
/*! GS Database problem. Please notify the administrator. */
#define ERRORROUTER_DBPROBLEM                13
/*! The client is incompatible with the server. */
#define ERRORROUTER_CLIENTINCOMPATIBLE       50
/*! @} end of group7_1 */



/*! @defgroup group7_2 FRIENDS service.
\brief Errors returned by the FRIENDS service.

Errors returned by the FRIENDS service.
    @{
*/
/*! The Player does not exist. */
#define ERRORFRIENDS_FRIENDNOTEXIST          14
/*! The Player is not connected to an arena. */
#define ERRORFRIENDS_NOTINARENA              15
/*! The Player is not online. */
#define ERRORFRIENDS_PLAYERNOTONLINE         16
/*! The Player is not in a session. */
#define ERRORFRIENDS_NOTINSESSION            17
/*! The Player is ignoring you */
#define ERRORFRIENDS_PLAYERIGNORE            18
/*! The Player is already connected. */
#define ERRORFRIENDS_ALREADYCONNECTED        19
/*! The Friends server cannot accept more players. */
#define ERRORFRIENDS_NOMOREPLAYERS           20
/*! The Player has no scores in database. */
#define ERRORFRIENDS_NOPLAYERSCORE           47
/*! Search Player Finished. */
#define ERRORFRIENDS_SEARCHPLAYERFINISHED    48
/*! The Players status is COREONLINE and is not receiving pages/peer messages */
#define ERRORFRIENDS_PLAYERSTATUSCOREONLINE  56

/*! @} end of group7_2 */



/*! @defgroup group7_3 SESSION service.
\brief Errors returned by the SESSION service.

Errors returned by the SESSION service.
    @{
*/
#define ERRORARENA_SESSIONEXIST              21
#define ERRORARENA_GAMENOTALLOWED            22
#define ERRORARENA_NUMBERPLAYER              23
#define ERRORARENA_NUMBERSPECTATOR           24
#define ERRORARENA_VISITORNOTALLOWED         25
#define ERRORARENA_NOTREGISTERED             26
#define ERRORARENA_NOMOREPLAYERS             27
#define ERRORARENA_NOMORESPECTATORS          28
#define ERRORARENA_PLAYERNOTREGISTERED       29
#define ERRORARENA_SESSIONNOTAVAILABLE       30
#define ERRORARENA_SESSIONINPROCESS          31
#define ERRORARENA_BADGAMEVERSION            32
#define ERRORARENA_PASSWORDNOTCORRECT        33
#define ERRORARENA_ALREADYINSESSION          34
#define ERRORARENA_NOTMASTER                 35
#define ERRORARENA_NOTINSESSION              36
#define ERRORARENA_MINPLAYERS                37
#define ERRORARENA_ADMINGAMEDOESNOTEXIST     38
#define ERRORARENA_ADMINSESSIONDOESNOTEXIST  39
#define ERRORARENA_CONNECTADDCONNECTION      40
#define ERRORARENA_CONNECTSENDLOGINMSG       41
#define ERRORARENA_ERRORLOGINMESSAGE         42
#define ERRORARENA_NOHOSTARENA               43
#define ERRORARENA_ARENADISCONNECTED         44
#define ERRORARENA_INVALIDGROUPNAME          45
#define ERRORARENA_INVALIDGAMETYPE           46
#define ERRORARENA_NOMOREGAMEMODULE          47
#define ERRORARENA_PASSPORTLABELNOTFOUND     48
#define ERRORARENA_PASSPORTFAIL              49
#define ERRORARENA_CREATENOTALLOWED          50
#define ERRORARENA_INVALIDSESSIONTYPE        51
#define ERRORARENA_SESSIONCLOSE              52
#define ERRORARENA_NOTCREATOR                53
#define ERRORARENA_DEDICATEDSERVERONLY       54

/*! @} end of group7_3 */


/*! @defgroup group7_4 CLANMANAGER service.

Errors returned by the CLANMANAGER service.
    @{
*/
#define ERRORCLAN_INVALIDPROFILE             49
/*! @} end of group7_4 */


/*! @defgroup group7_5 DB/Proxy
\briefError messages for db/proxy services

Error messages for db/proxy services
    @{
*/
#define ERROR_SERVICENOTAVAILABLE            55

/*! @defgroup group7_5_1 Ladder Query Service
\brief Ladder Query Service Errors
Error codes for the proxy's ladder query service
@{
*/

#define ERRLQS_DUPLICATEFIELD                60
#define ERRLQS_DATABASEFAILURE               61
#define ERRLQS_INTERNAL_OUTOFMEMORY          62
#define ERRLQS_INTERNAL_WRONGRESULTVERSION   63
#define ERRLQS_INTERNAL_BADRESULTFORMAT      64

/*! @} end of group7_5_1 */

/*! @defgroup group7_5_2 Score Submission
\brief Score Submission Error Codes
Error codes for the proxy's score submission
@{
*/

#define ERRSS_BADFORMAT                     65
#define ERRSS_DBFAILURE                     66
#define ERRSS_SUBMISSIONFAILED              67
#define ERRSS_VALIDATIONFAILED              68

/*! @} end of group7_5_2 */

/*! @} end of group7_5 */


/*! @defgroup group7_6 Other
\brief Other error messages

Other error messages
    @{
*/
#define ERROR_ROUTERCONNECTION               -1
#define ERROR_ARENACONNECTION                -2
#define ERROR_LOBBYSRVDISCONNECTED			 -3
/*! @} end of group7_6 */

/*! @defgroup group7_7 Secure Accounts
\brief Errors for Secure Accounts

These Error messages are for Secure Accounts
    @{
*/
// CREATE: The username already exists
#define ERRORSECURE_USERNAMEEXISTS 1
// CREATE: The username is malformed
#define ERRORSECURE_USERNAMEMALFORMED 2
// CREATE: The username is not allowed to be used (e.g. contains smut)
#define ERRORSECURE_USERNAMEFORBIDDEN 3
// LOGIN: The account does not exist
#define ERRORSECURE_INVALIDACCOUNT 4
// CREATE: The username is reserved (e.g. Ubi_* usernames)
#define ERRORSECURE_USERNAMERESERVED 5
// CREATE/UPDATE: The password is malformed
#define ERRORSECURE_PASSWORDMALFORMED 11
// CREATE/UPDATE: The password is not allowed (e.g. contains username)
#define ERRORSECURE_PASSWORDFORBIDDEN 13
// LOGIN: The password is incorrect
#define ERRORSECURE_INVALIDPASSWORD 15
// ALL: The database returned an error
#define ERRORSECURE_DATABASEFAILED 100
// LOGIN: The account has been banned
#define ERRORSECURE_BANNEDACCOUNT 501
// LOGIN: The account has been blocked
#define ERRORSECURE_BLOCKEDACCOUNT 502
// LOGIN: The account has been locked
#define ERRORSECURE_LOCKEDACCOUNT 512
/*! @} end of group7_6 */


/*! @} end of group7 */


#endif
