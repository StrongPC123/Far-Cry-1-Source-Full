//****************************************************************************
//*   Author:  Guillaume Plante  gsdevelopers@ubisoft.com
//*   Date:    2002-06-06 15:47:17
 /*!  \file   GSCDKeyCallbacks.h
  *   \brief  Callback definitions and register functionalities
  *
  *   This file contains the callback definitions and register functionalities
  *   for the cd key interface
  */
//****************************************************************************

#ifndef _GSCDKEYCALLBACKS_H_
#define _GSCDKEYCALLBACKS_H_

#include "GSTypes.h"
#include "GSCDKeyDefines.h"


/*! @defgroup group4 Game client callback
\brief Game client callback

These callback function are used by the game client to 
process response to queries sent to the validation server
    @{
*/

//============================================================================
// CallBack CBCDKey_RcvActivationID
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:34:14
/*!
 \brief	 Received a activation id request response from the validation server
 \par       Description:
 This callback will be called whenever the game client receive a response
 after requesting the validation server for a activation id.

 \param	psReplyInfo	Pointer to a reply information structure that contains 
        request id and status
 \param	psValidationServerInfo	Validation server information from where the
		response came
 \param	pucActivationID	The activation id in case the request succedded
 \param pucGlobalID The unique global identifier of the CDKey

*/
//============================================================================
typedef GSvoid (__stdcall *CBCDKey_RcvActivationID)(PREPLY_INFORMATION psReplyInfo,
			PVALIDATION_SERVER_INFO psValidationServerInfo,GSubyte *pucActivationID,GSubyte *pucGlobalID);


//============================================================================
// CallBack CBCDKey_RcvAuthorizationID
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:38:23
/*!
 \brief	 Received a authorization id request response from the validation server
 \par       Description:
 This callback will be called whenever the game client receive a response
 after requesting the validation server for a authorization id.

 \param	psReplyInfo	Pointer to a reply information structure that contains 
        request id and status
 \param	psValidationServerInfo	Validation server information from where the
		response came
 \param	pucAuhorizationID	The authorization id in case the request succedded

*/
//============================================================================
typedef GSvoid (__stdcall *CBCDKey_RcvAuthorizationID)(PREPLY_INFORMATION psReplyInfo,
			PVALIDATION_SERVER_INFO psValidationServerInfo,GSubyte *pucAuhorizationID);

/*! @} end of group4 */

/*! @defgroup group5 Game server callback
\brief Game server callback

These callback function are used by the game server to 
process response to queries sent to or by the validation server
    @{
*/


//============================================================================
// CallBack CBCDKey_RcvValidationResponse
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:44:57
/*!
 \brief	 Received a validation status for a player from the validation server
 \par       Description:
 This callback will be called whenever the game server receives a validation
 status for a player after having sent a validation status query to the
 validation server. It will inform the game server of the player validation status
 so that the game server can choose to keep or disconnect the user.

 \param	psReplyInfo	Pointer to a reply information structure that contains 
        request id and status
 \param	psValidationServerInfo	Validation server information from where the
		response came
 \param	pucAuhorizationID	The authorization id in the query
 \param	eStatus	The validation status of the player
 \param pucGlobalID The unique global identifier of the CDKey

*/
//============================================================================
typedef GSvoid (__stdcall *CBCDKey_RcvValidationResponse)(PREPLY_INFORMATION psReplyInfo,
			PVALIDATION_SERVER_INFO psValidationServerInfo,GSubyte *pucAuhorizationID,CDKEY_PLAYER_STATUS eStatus,GSubyte *pucGlobalID);


//============================================================================
// CallBack CBCDKey_RcvPlayerStatusRequest
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:49:13
/*!
 \brief	 Receive a player status request from the validation server
 \par       Description:
 This callback will be called whenever the game server receives a player 
 status request from the validation server. The game server should then 
 respond to this query with the function GSCDKey_PlayerStatusReply
 to inform the validation server of the player status. The status
 should be E_PLAYER_UNKNOWN if the player is not on the game server
 at the moment of the request.

 \param	psValidationServerInfo	Validation server information from where the
		response came
 \param	pucAuhorizationID	The authorization id of the concerned player

*/
//============================================================================
typedef GSvoid (__stdcall *CBCDKey_RcvPlayerStatusRequest)(PVALIDATION_SERVER_INFO psValidationServerInfo,
			GSubyte *pucAuhorizationID);


/*! @} end of group5 */

/*! @defgroup group6 Callback register functions
\brief Callback register functions
These functions are used to register the callback for a created cd key module

    @{
*/


//============================================================================
// Function GSCDKey_FixRcvActivationID
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:56:15
/*!
 \brief	 Register the callback
 \par       Description:
 This function is used to register the callback CBCDKey_RcvActivationID
 in the library. If this is not correctly called, the callback will
 nerver be triggered.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	fRcvActivationID	Function of type CBCDKey_RcvActivationID

*/
//============================================================================
GSbool __stdcall GSCDKey_FixRcvActivationID(GShandle pGSCDKeyHandle,CBCDKey_RcvActivationID fRcvActivationID);


//============================================================================
// Function GSCDKey_FixRcvAuthorizationID
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:58:29
/*!
 \brief	 Register the callback
 \par       Description:
 This function is used to register the callback CBCDKey_RcvAuthorizationID
 in the library. If this is not correctly called, the callback will
 nerver be triggered.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	fRcvAuthorizationID	Function of type CBCDKey_RcvAuthorizationID
 
*/
//============================================================================
GSbool __stdcall GSCDKey_FixRcvAuthorizationID(GShandle pGSCDKeyHandle,CBCDKey_RcvAuthorizationID fRcvAuthorizationID);


//============================================================================
// Function GSCDKey_FixRcvValidationResponse
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:58:37
/*!
 \brief	 Register the callback
 \par       Description:
 This function is used to register the callback CBCDKey_RcvValidationResponse
 in the library. If this is not correctly called, the callback will
 nerver be triggered.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	fRcvValidationResponse	Function of type CBCDKey_RcvValidationResponse

*/
//============================================================================
GSbool __stdcall GSCDKey_FixRcvValidationResponse(GShandle pGSCDKeyHandle,CBCDKey_RcvValidationResponse fRcvValidationResponse);


//============================================================================
// Function GSCDKey_FixRcvPlayerStatusRequest
// Author:		Guillaume Plante  gsdevelopers@ubisoft.com
// Date:		2002-06-06 14:58:50
/*!
 \brief	 Register the callback
 \par       Description:
 This function is used to register the callback CBCDKey_RcvPlayerStatusRequest
 in the library. If this is not correctly called, the callback will
 nerver be triggered.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	pGSCDKeyHandle	Handle on the created cd key module
 \param	fRcvPlayerStatusRequest	Function of type CBCDKey_RcvPlayerStatusRequest

*/
//============================================================================
GSbool __stdcall GSCDKey_FixRcvPlayerStatusRequest(GShandle pGSCDKeyHandle,CBCDKey_RcvPlayerStatusRequest fRcvPlayerStatusRequest);

/*! @} end of group6 */

#endif //_GSCDKEYCALLBACKS_H_
