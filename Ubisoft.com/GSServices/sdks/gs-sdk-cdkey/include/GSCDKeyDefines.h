//****************************************************************************
//*   Author:  Guillaume Plante  gsdevelopers@ubisoft.com
//*   Date:    2002-06-06 15:20:01
 /*!  \file   GSCDKeyDefines.h
  *   \brief  Structure definitions used in the cd key interface
  *
  *   This file contains the different structure definitions used in the 
  *	  cd key interface
  */
//****************************************************************************

#ifndef _GSCDKEYDEFINES_H_
#define _GSCDKEYDEFINES_H_

#include "GSTypes.h"
#include "define.h"
#include "CDKeyDefines.h"

typedef GSuint GSCDKeyRequest; //!< Request identifier


/*! 
	\brief Reply information structure (callback usage)

    This structure contains the data associated with a response to a request sent to the validation server
*/
typedef struct _REPLY_INFORMATION	
{
	GSCDKeyRequest CDKeyRequest;	//!< The cd key request id
	GSbool bSucceeded;	//!< GS_TRUE is the request is a success, GS_FALSE else
	GSushort usErrorID;	//!< The error id in case of failure (0 in case of success)
} REPLY_INFORMATION,*PREPLY_INFORMATION; 

/*! 
	\brief Validation server information structure

    This structure contains the validation server information that is used when sending data
*/
typedef struct _VALIDATION_SERVER_INFO
{
	GSchar szIPAddress[IPADDRESSLENGTH];	//!< The ipaddress of the validation server
	GSushort usPort;	//!< The port the validation server will listen to
} VALIDATION_SERVER_INFO,*PVALIDATION_SERVER_INFO;


/*!
	\brief Activation information structure

    This structure contains the activation information needed when requesting for a activation id
*/
typedef struct _ACTIVATION_INFO
{
	GSchar szGameName[GAMELENGTH];	//!< The name of the game
	GSchar szCDKey[CDKEY_SIZE + 1];	//!< The user's cd key
} ACTIVATION_INFO,*PACTIVATION_INFO;

/*! 
	\brief Validation information structure

    This structure contains the validation information needed when requesting for cd key validation
*/
typedef struct _VALIDATION_INFO
{
	GSubyte ucActivationID[ACTIVATION_ID_SIZE];	//!< The activation id associated with a cd key
	GSchar szCDKey[CDKEY_SIZE + 1];	//!< The user's cd key
} VALIDATION_INFO,*PVALIDATION_INFO;

#endif //_GSCDKEYDEFINES_H_