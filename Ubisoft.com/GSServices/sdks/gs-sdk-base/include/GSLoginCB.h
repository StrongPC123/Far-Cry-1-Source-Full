//****************************************************************************
//*   Author:  Guillaume Plante  <gsdevteam@ubisoft.com>
//*   Date:	5/16/01 9:20:13 AM
 /*!  \file   GSLoginCB.h
  *   \brief  Callback functions for the Login service
  *
  *   This file contains all callback functions declaration for the<b><i>login
  *   service</i></b>.
  */
//****************************************************************************


#ifndef __GSLOGINCB_H_
#define __GSLOGINCB_H_

#include "GSTypes.h"

#ifdef __cplusplus
class clLoginCallbacks
{
	public:
	virtual GSvoid LoginRcv_PlayerInfo(GSubyte ubType, GSchar * pszNickName,
			GSchar * pszSurName, GSchar * pszFirstName, GSchar * pszCountry,
			GSchar * pszEmail, GSchar * szIRCID, GSchar * szIPAddress,
			GSint iReason ) = 0;
	virtual GSvoid LoginRcv_JoinWaitModuleResult(GSubyte ubType,
			GSchar * pszAddress, GSushort lPort, GSint iReason) = 0;
	virtual GSvoid LoginRcv_LoginRouterResult(GSubyte ubType, GSint iReason) = 0;
	virtual GSvoid LoginRcv_LoginWaitModuleResult(GSubyte ubType,
			GSint iReason) = 0;
	virtual GSvoid LoginRcv_SystemPage(GSint lSubType, GSchar * pszText) = 0;
	virtual GSvoid LoginRcv_LoginDisconnection() = 0;
	virtual GSvoid LoginRcv_AccountCreationResult(GSubyte ubType,
			GSint iReason) = 0;
	virtual GSvoid LoginRcv_ModifyUserResult(GSubyte ubType, GSint iReason) = 0;
	virtual GSvoid LoginRcv_RequestMOTD(GSubyte ubType, GSchar *szUbiMOTD,
			GSchar *szGameMOTD, GSint iReason)=0;
};
#endif //__cplusplus


/*! @addtogroup group_LoginCB 
    @{
*/

//============================================================================
// Callback CBLoginRcv_PlayerInfo
/*!
 \brief		Receive information on a player
 \par       Description:
 This callback will be called when the client requests information on a player
 
 \par Related Function:
 LoginSend_PlayerInfo()

	\par Errors:
	ERRORROUTER_DBPROBLEM: There is a problem with the database.<br>
	ERRORROUTER_NOTREGISTERED: The username doesn't exist.<br>

 \param	 ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	 szUsername	The username of the player in the correct case
 \param	 szSurName	The last name of a player
 \param	 szFirstName	The first name of the player
 \param	 szCountry	The country of the player
 \param	 szEmail	Email address of the player
 \param	 szIRCID	Player's IRC ID
 \param	 szIPAddress	Player's ip address
 \param	 iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_PlayerInfo)(GSubyte ubType,
		GSchar *szUsername, GSchar *szSurName, GSchar *szFirstName,
		GSchar *szCountry, GSchar *szEmail,GSchar *szIRCID, GSchar *szIPAddress,
		GSint iReason);


//============================================================================
// Callback CBLoginRcv_JoinWaitModuleResult
/*!
 \brief	 Receive status of the join wait module request
 \par       Description:
  This callback will be called when the client receive a response from the
	router after asking to join the wait module.  The client should then call
	LoginSend_Connect() with this szAddress and usPort.

  \par Related Function:
	LoginSend_JoinWaitModule()<br>

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	szAddress	IP address of the wait module
 \param	usPort	The port of the wait module
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_JoinWaitModuleResult)(GSubyte ubType,
		GSchar *szAddress, GSushort usPort, GSint iReason);


//============================================================================
// Callback CBLoginRcv_LoginRouterResult

/*!
 \brief	 Receive status of the login request
 \par       Description:
  This callback will be called when the client receive a response from the
	router after asking to log into the router.  If you reveive a GSSCUCCESS you
	can then call LoginSend_JoinWaitModule().
	
  \par Related Function:
	LoginSend_LoginRouter()<br>
	
	\par Errors:
	ERRORSECURE_DATABASEFAILED: There is a problem with the Database.<br>
	ERRORROUTER_NOTDISCONNECTED: The player is already logged in.<br>
	ERRORSECURE_INVALIDPASSWORD: The password is not correct.<br>
	ERRORSECURE_LOCKEDACCOUNT: The account has been locked.<br>
	ERRORSECURE_INVALIDACCOUNT: The username doesn't exist.<br>
	ERRORSECURE_BANNEDACCOUNT: The account has been banned.<br>
	ERRORSECURE_BLOCKEDACCOUNT: The account has been blocked.<br>

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_LoginRouterResult)(GSubyte ubType,
		GSint iReason);


//============================================================================
// Callback CBLoginRcv_LoginWaitModuleResult

/*!
 \brief	 Receive status of the login wait module request
 \par       Description:
  This callback will be called when the client receive a response from the
	router after asking to login to the wait module.  After receving this callback
	you will be fully connected to the Game Service.  It's recommened that the
	client now call LoginSend_PlayerInfo() with the players username to get his
	szIRCID and the correct case of his username.
	
  \par Related Function:
	LoginSend_LoginWaitModule()<br>

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_LoginWaitModuleResult)(GSubyte ubType,
		GSint iReason);

//============================================================================
// Callback CBLoginRcv_SystemPage

/*!
 \brief	 Received a system page 
 \par       Description: 
  This callback will be called when the client received a system
  page wich is usually called by a administrator or when another
  player adds the client to his friend list.

 \param	iSubType  The type of message. The possibilities are:
                  <UL>
                      <LI>ADDEDASFRIEND<BR>
                      The player was added to the friend list of the other
                      player who's name is in the szText parameter
                      <LI>ADDEDASIGNOREE<BR>
                      The player was added to the ignore list of the other
                      player who's name is in the szText parameter
                      <LI>REMOVEDASIGNOREE<BR>
                      The player was removed from the ignore list of the other
                      player who's name is in the szText parameter
                  </UL>
 \param	szText	  The actual message

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_SystemPage)(GSint iSubType,
	GSchar *szText);


//============================================================================
// Callback CBLoginRcv_LoginDisconnection

/*!
 \brief	 Disconnection from router
 \par       Description:
  This callback will be called when the client is disconnected
  from the router
  
*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_LoginDisconnection)();


//============================================================================
// Callback CBLoginRcv_AccountCreationResult

/*!
 \brief	 Receive the status of the account creation request
 \par       Description:
  This callback will be called when the client receive a response from the
	router after sending a account creation request

  \par Related Function:
	LoginSend_Disconnect()
	
	\par Errors:
	ERRORSECURE_USERNAMEEXISTS: The account name already exists.<br>
	ERRORSECURE_USERNAMEMALFORMED: The account name does not match the format
	                               rules <code>^[a-zA-Z][a-zA-Z0-9_\.-]{2,14}$</code>.<br>
	ERRORSECURE_USERNAMEFORBIDDEN: The account name contains forbidden
	                               substrings (e.g. smut)<br>
	ERRORSECURE_USERNAMERESERVED: The account name is reserved<br>
	ERRORSECURE_PASSWORDMALFORMED: The password does not match the format
	                               rules <code>^.{2,16}$</code>.<br>
	ERRORSECURE_PASSWORDFORBIDDEN: The password contains the username<br>
	ERRORSECURE_DATABASEFAILED: There is a problem with the database.<br>

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_AccountCreationResult)(GSubyte ubType,
		GSint iReason);


//============================================================================
// Callback CBLoginRcv_ModifyUserResult

/*!
 \brief	 Receive the status of the user modifycation request
 \par       Description:
  This callback will be called when the client receive a response from the
	router after sending a user info modification request
	
  \par Related Function:
	LoginSend_ModifyAccount()

  \par Errors:
    ERRORSECURE_PASSWORDMALFORMED: The password does not match the format
	                               rules <code>^.{2,16}$</code>.<br>
	ERRORSECURE_PASSWORDFORBIDDEN: The password contains the username<br>
	ERRORSECURE_DATABASEFAILED: There is a problem with the database.<br>

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_ModifyUserResult)(GSubyte ubType,
		GSint iReason);


//============================================================================
// Callback DBLoginRcv_RequestMOTD

/*!
 \brief	 Receive the message of the day
 \par       Description:
  This callback will be called when the client receives the MOTDs from the
	server.  The messages will never be greater the MOTDLENGTH.
	
  \par Related function:
	LoginSend_RequestMOTD()
	
	\par Errors:
	ERRORROUTER_DBPROBLEM: There is a problem with the database.<br>

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param szUbiMOTD The message of the day for the Ubi.com Game Service
 \param szGameMOTD The message of the day for the game.
 \param	iReason	The reason for the failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLoginRcv_RequestMOTD)(GSubyte ubType,
		GSchar *szUbiMOTD, GSchar *szGameMOTD, GSint iReason);

/*! @} end of group_LoginCB */

#endif //__GSLOGINCB_H_
