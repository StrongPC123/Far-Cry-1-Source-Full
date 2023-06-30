
//****************************************************************************
//*   Author:  Guillaume Plante  <gsdevelopers@ubisoft.com>
//*   Date:	  2003-07-14
 /*!  \file   GSLadderQueryCB.h
  *   \brief  Callback functions for the <b><i>ladder query service</i></b>.
  *
  *   This file contains all callback functions declaration for the ladder query
  *   service.
  */
//****************************************************************************

#ifndef __GSLADDERQUERYCB_H__
#define __GSLADDERQUERYCB_H__

#include "GSTypes.h"

#ifdef __cplusplus
class clLadderQueryCallbacks
{
	public:
		virtual GSvoid LadderQueryRcv_RequestReply(GSubyte ucType, GSint  iReason, GSuint uiRequestId) = 0;
};

#endif //__cplusplus

/*! @addtogroup group_LadderQuery

    @{
*/

//============================================================================
// Callback CBLadderQueryRcv_RequestReply 

/*!
 \brief	 Receive the result of a ladder query to the ubi.com servers.
 \par    Description:
  This callback will be called when the client receive a response from the ubi.com data provider
  after asking a ladder data request.
 \par Related functions
  LadderQuery_SendRequest()

 \param	ucType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ucType is GSFAIL
 \param	uiRequestId	The id of the request from wich we received results

*/
//============================================================================
typedef GSvoid (__stdcall *CBLadderQueryRcv_RequestReply)(GSubyte ucType, GSint  iReason, GSuint uiRequestId);


/*! @} end of group_LadderQuery */

#endif //__GSLADDERQUERYCB_H__
