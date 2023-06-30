//****************************************************************************
//*   Author:  Guillaume Plante  gsdevelopers@ubisoft.com
//*   Date:    2002-06-06 10:49:52
 /*!  \file    GSCDKeyInterface.h
  *   \brief   Interface ubi.com's cd key validation library.
  *
  *   This interface provides game server and game client functionality
  *   to activate and validate a user's cd key. 
  */
//****************************************************************************

/*!
\mainpage gs-sdk-cdkey
\section intro Introduction
 ubi.com cd key validation system interface


\section description Description
 This sdk provides functionalities for a game developper to add a cd key validation
 mechanism in his project. This system works along the ubi.com validation server.

 It includes game server side functionalities:
    - Get a player validation status
    - Inform the validation server of the player status

 and game client side functionalities:
    - Request cd key activation (once)
    - Request cd key validation

  <b>Note for game client side:</b><br>
    The cd key activation is done once before the first cd key athorization request.
  After requesting for a cd key activation , the user will receive a activation id
  that should be kept safely to be use in all futur authorization request before joining
  a game server.

  <b>Note for game server side:</b><br>
    The game server will need to ask the validation server for a player
  validation status (VALID or INVALID) before choosing to disconnect or to keep the player,
  this is done after the game server has received the authorization id of the game client
  that want to join.

  These are the chronological steps needed to be accomplish in a client validation
  from a game server and game client point of view. The first step(1) must be accomplish
  only once, when the game client does not possess his ACTIVATION ID.

  1.  The game client get his ACTIVATION ID from the validation server using the 
     GSCDKey_RequestActivation() function call. <b>This is done once</b> or
	 if the client does not possess a ACTIVATION ID. The ACTIVATION ID should be 
	 kept localy in a file or in the registry for further access.

  2. The game client get his AUTHORIZATION ID from the validation server using the
     GSCDKey_RequestAuthorization() function call. This step will be done every time
	 a client want to join a game server, the client needs a new authorization id
	 each time he want to join a game server. If this call fails the client has 2 choice:
	 - send the request again using GSCDKey_RequestAuthorization() using a bigger timeout 
	   value (if the error was caused by a timeout)
	 - use the last authorization id received the last time the user joined a game server.
	   (there is a chance that the validation of this old authorization id will fail
	   if the latter has timed-out on the validation server side)

  3. After connecting to the game server, the game client send his AUTHORIZATION ID to the game server. 

  4. After the game server have received a client connection and AUTHORIZATION ID,
     he validate the client using the GSCDKey_ValidateUser() function call. The
	 status of the player (CDKEY_PLAYER_STATUS) will be returned via the callback 
	 CBCDKey_RcvValidationResponse(). If this request does not succeed, and that the error
	 code is ERRORCDKEY_TIMEOUT it is recommended that the request be sent again 
	 with a bigger timeout value. If a ERRORCDKEY_TIMEOUT error still occurs after that,
	 the ubi.com network may have problems. It is however recommended that the game server
	 does not let the player connect. Otherwise, it may lead to :
	    - hacks in the game to force a timeout
	    - attacks on ubi.com server to have request time out

	 Recommendations after the game server has receive the player status via the
	 GSCDKey_ValidateUser() callback:
	   - if the player status is E_PLAYER_UNKNOWN or E_PLAYER_INVALID the game server should
	     disconnect the player.
	   - if the player status is E_PLAYER_VALID the game server should keep the player.	 
  
*/

#ifndef _GSCDKEYINTERFACE_H_
#define _GSCDKEYINTERFACE_H_

#include "GSCDKeyDefines.h"
#include "GSCDKeyCallbacks.h"

extern "C" {

/*! @defgroup group1 General functionalities
\brief General functionalities

These functions are used both by the game client and game server.
They are use to initialize, uninitialize the library and process
incomming/outgoing messages.
    @{
*/

//============================================================================
// Function GSCDKey_Initialize
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 10:51:55
/*!
 \brief	 Initialize the cd key library
 \par       Description:
 This function will initialize the cd key library, on success
 it will return a handle that can be use for futur calls on this service.
 The provided port will be reserved for library communication and will be
 freed once the library is uninitialized, if the supplied port is not available
 (already binded) on the local machine, the initialization will fail.

 \return    Status of the function call

 \retval	Handle on the created cd key module
 \retval	0 if the operation failed

 \param	usPort	The port on wich the client will listen to.

*/
//============================================================================
GShandle __stdcall GSCDKey_Initialize(GSushort usPort);


//============================================================================
// Function GSCDKey_Uninitialize
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 10:58:49
/*!
 \brief	 Uinitialize a cd key handle or the whole library
 \par       Description:
 This function will uninitialize a provided cd key handle or
 it will uninitialize all cd key handle if no argument are supplied.

 \return    void

 \param	pGSCDKeyHandle	Handle on the cd key module

*/
//============================================================================
GSvoid __stdcall GSCDKey_Uninitialize(GShandle pGSCDKeyHandle = 0);



//============================================================================
// Function GSCDKey_Engine
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 11:06:12
/*!
 \brief	 Message pump
 \par       Description:
 This function send outgoing messages and process incomming messages.
 To insure smooth operation of the library, this function should be called 
 at least 10 time a second. It will return GS_FALSE if the provided
 cd key handle is not valid.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	pGSCDKeyHandle	Handle on the cd key module
 \param	uiMaxDelay		Maximum number of milisecond spent on processing messages

*/
//============================================================================
GSbool __stdcall GSCDKey_Engine(GShandle pGSCDKeyHandle,GSuint uiMaxDelay = 500);

/*! @} end of group1 */

/*! @defgroup group2 Game server functionalities
\brief Game server functionalities

These functions are used by the game server to 
ask the validation status of a player, inform the 
validation server of a player status and disconnection.
    @{
*/

//============================================================================
// Function GSCDKey_ValidateUser
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 13:23:34
/*!
 \brief	 Ask the validation server for a validation status of a player
 \par       Description:
 This function is used to ask the validation server for a validation status 
 of a player after the latter has connected on the game server. The game server
 will received the result via the CBCDKey_RcvValidationResponse callback.

 \return    The id of the request

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	psValidationServerInfo	Validation server information
 \param	pucAuhorizationID	Authorization id sent by the game client
 \param	szGameName	Name of the game on the game server
 \param	usTimeout	Number of second before the request is considered timed out
					default for this request is 3. 0 means a unlimited timeout value.

*/
//============================================================================
GSCDKeyRequest __stdcall GSCDKey_ValidateUser(GShandle pGSCDKeyHandle,
										PVALIDATION_SERVER_INFO psValidationServerInfo,
										GSubyte *pucAuhorizationID,GSchar *szGameName,
										GSushort usTimeout = 3);


//============================================================================
// Function GSCDKey_DisconnectUser
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 13:57:12
/*!
 \brief	 Inform the validation server that a player has disconnected
 \par       Description:
 This function is used to inform the validation server that a player
 has disconnected from the game server. This must be called whenever a game
 server detects a client disconnection.

 \return    The id of the request

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	psValidationServerInfo	Validation server information
 \param	pucAuhorizationID	Authorization id of the game client

*/
//============================================================================
GSCDKeyRequest __stdcall GSCDKey_DisconnectUser(GShandle pGSCDKeyHandle,
									  PVALIDATION_SERVER_INFO psValidationServerInfo,
									  GSubyte *pucAuhorizationID);


//============================================================================
// Function GSCDKey_PlayerStatusReply
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 13:59:14
/*!
 \brief	 Inform the validation server of a player status
 \par       Description:
 This function is used to inform the validation server of a player status.
 This must be called with the correct status for a player whenever the game server
 receive a status request from the validation server via the 
 CBCDKey_RcvPlayerStatusRequest callback.

 \return    The id of the request

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	psValidationServerInfo	Validation server information
 \param	pucAuhorizationID	Authorization id of the game client
 \param	eStatus		Status of the player
*/
//============================================================================
GSCDKeyRequest __stdcall GSCDKey_PlayerStatusReply(GShandle pGSCDKeyHandle,
										 PVALIDATION_SERVER_INFO psValidationServerInfo,
										 GSubyte *pucAuhorizationID,CDKEY_PLAYER_STATUS eStatus);

/*! @} end of group2 */
/*! @defgroup group3 Game client functionalities
\brief Game client functionalities

These functions are used by the game client to 
request a activation and validation information
from the validation server.
    @{
*/


//============================================================================
// Function GSCDKey_RequestActivation
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:05:39
/*!
 \brief	 Request cd key activation id for futur authorization request (called once)
 \par       Description:
 This function is used to get the activation id that will be use in
 futur client authentication. This function should be called the first
 time a cd key authentication is needed (i.g. the first time a user plays
 online after the game installation). The activation id should be safely 
 kept so that each subsequent authorization request use that activation id.
 After calling this request, the result will be returned via the
 CBCDKey_RcvActivationID callback. 0 means a unlimited timeout value.

 \return    The id of the request

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	psValidationServerInfo	Validation server information
 \param	psActivationInfo   Validation information
 \param	usTimeout	Number of second before the request is considered timed out
					default for this request is 6. 0 means a unlimited timeout value.
*/
//============================================================================
GSCDKeyRequest __stdcall GSCDKey_RequestActivation(GShandle pGSCDKeyHandle,
										 PVALIDATION_SERVER_INFO psValidationServerInfo,
										 PACTIVATION_INFO psActivationInfo,
										 GSushort usTimeout = 6);


//============================================================================
// Function GSCDKey_RequestAuthorization
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:16:15
/*!
 \brief	 Send a authorization request to the validation server
 \par       Description:
 This function is used to validate a cd key before joining a game server.
 The validation information structure contains the activation id and 
 the cd key. After having receive the result from this request via the
 CBCDKey_RcvAuthorizationID callback, the user can join a game server
 wich will ask the validation server for the player status (authenticated or not)

 \return    The id of the request

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	psValidationServerInfo	Validation server information
 \param	psValidationInfo	Validation information
 \param	usTimeout	Number of second before the request is considered timed out
					default for this request is 3. 0 means a unlimited timeout value.

*/
//============================================================================
GSCDKeyRequest __stdcall GSCDKey_RequestAuthorization(GShandle pGSCDKeyHandle,
											PVALIDATION_SERVER_INFO psValidationServerInfo,
											PVALIDATION_INFO psValidationInfo,
											GSushort usTimeout = 3);

} // extern "C"

/*! @} end of group3 */
#endif //_GSCDKEYINTERFACE_H_
