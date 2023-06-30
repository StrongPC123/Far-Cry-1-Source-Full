/***SDOC*******************************************************************************************
 *                                UbiSoft Network Development
 *                                ---------------------------
 *
 * FILE........: CDKeyDefines.h
 * CREATION....: May 2002
 * AUTHOR......: Guillaume Plante
 *
 * DESCRIPTION.: This file contains numeric definition for the CDKey-system size and error #
 *
 **************************************************************************************************
 *                                         FILE HISTORY
 **************************************************************************************************
 *
 * DATE........: 
 * AUTHOR......: 
 * DESCRIPTION.: 
 *
 ******************************************************************************************EDOC***/


#ifndef _CDKEY_DEFINES_H_
#define _CDKEY_DEFINES_H_

#include "GSTypes.h"

//-------------------- player status ------------------------------

enum CDKEY_PLAYER_STATUS
{
	E_PLAYER_UNKNOWN,
	E_PLAYER_INVALID,
	E_PLAYER_VALID
};

//-------------------- defined sizes ------------------------------

const GSuint ACTIVATION_ID_SIZE			= 16;
const GSuint AUTHORIZATION_ID_SIZE		= 20;
const GSuint CDKEY_SIZE					= 23;
const GSuint CDKEY_ID_SIZE				= 20;
const GSuint CHALLENGE_SIZE				= 5;
const GSuint VALIDATION_KEY_SIZE		= 20;
const GSuint GLOBAL_ID_SIZE             = 16;

//-------------------- error codes --------------------------------

const GSushort    ERRORCDKEY_SUCCESS	                    	= 0;
const GSushort    ERRORCDKEY_TIMEOUT							= 1;
const GSushort    ERRORCDKEY_INVALID_CDKEY						= 2;
const GSushort    ERRORCDKEY_NOT_CHALLENGED						= 3;
const GSushort    ERRORCDKEY_ALREADY_ONLINE						= 4;
const GSushort    ERRORCDKEY_INTERNAL_ERROR						= 5;

#endif //_CDKEY_DEFINES_H_
