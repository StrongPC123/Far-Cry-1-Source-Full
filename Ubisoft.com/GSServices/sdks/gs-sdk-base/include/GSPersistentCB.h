
//****************************************************************************
//*   Author:  Guillaume Plante  <gsdevelopers@ubisoft.com>
//*   Date:	  2001-09-20
 /*!  \file   GSPersistentCB.h
  *   \brief  Callback functions for the <b><i>persistent storage service</i></b>.
  *
  *   This file contains all callback functions declaration for the persistent
	*   storage service.
  */
//****************************************************************************

#ifndef _GSPersistentCB_H_
#define _GSPersistentCB_H_

#include "GSTypes.h"

#ifdef __cplusplus
class clPersistentCallbacks
{
	public:

		virtual GSvoid PSRcv_LoginResult(GSubyte ucType, GSint iReason) = 0;
		virtual GSvoid PSRcv_Disconnection() = 0;
		virtual	GSvoid PSRcv_GetDataReply(GSubyte ucType, GSint iReason,GSuint iID,
				GSvoid *pData,GSint iSize) = 0;
		virtual	GSvoid PSRcv_SetDataReply(GSubyte ucType, GSint iReason,
				GSuint iID) = 0;
};

#endif //__cplusplus

/*! @addtogroup group_PSCB Persistent
    @{
*/

//============================================================================
// CallBack CBPSRcv_LoginResult 

/*!
 \brief	 Receive status of the Persistent data storage login request
 \par    Description:
  This callback will be called when the client receive a response from the
	router after asking to join the Persistent storage server
  \par Related Function:
	PSSend_Login()

 \param	ucType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	lReason	The reason of failure if ucType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBPSRcv_LoginResult)(GSubyte ucType, GSint  iReason);

//============================================================================
// CallBack CBPSRcv_Disconnection
/*!
 \brief	 Client as been disconnected from the Persistent storage service
 \par    Description:
  This callback will be called when the client has been disconnected from the 
  Persistent storage service
 

*/
//============================================================================
typedef GSvoid (__stdcall *CBPSRcv_Disconnection)();


//============================================================================
// CallBack CBPSRcv_SetDataReply

/*!
 \brief	 Proxy reply on client request to set persistent data
 \par    Description:
  This callback will be called when the client has set data using
  Persistent storage service

 \param	ucType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ucType is GSFAIL  
 \param	iID 	The id of the request

*/
//============================================================================
typedef GSvoid (__stdcall *CBPSRcv_SetDataReply)(GSubyte ucType, GSint iReason,
		GSuint iID);

//============================================================================
// CallBack CBPSRcv_GetDataReply

/*!
 \brief	 Proxy reply on client request to get persistent data
 \par    Description:
  This callback will be called when the client ask for persistent
  data using the Persistent storage service

 \param	ucType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ucType is GSFAIL  
 \param	iID 	The id of the request
 \param	pData 	Pointer to the data buffer
 \param	iSize 	Size of the data buffer


*/
//============================================================================
typedef GSvoid (__stdcall *CBPSRcv_GetDataReply)(GSubyte ucType, GSint  iReason,
		GSuint iID,GSvoid *pData,GSint iSize);

#endif //_GSPersistentCB_H_
