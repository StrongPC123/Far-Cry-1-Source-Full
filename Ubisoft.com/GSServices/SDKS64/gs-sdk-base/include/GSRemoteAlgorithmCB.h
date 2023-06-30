#ifndef __GSREMOTEALGORITHMCB_H__
#define __GSREMOTEALGORITHMCB_H__

// remote algorithm execution service definitions
#include "RemoteAlgorithmDefines.h"

// error code system
#include "GSErrors.h"

/*! @addtogroup group_RAECB
@{
*/

//============================================================================
// Callback RemoteAlgorithm_OutputCB

/*!
\brief  Callback that will receive the output of an algorithm
\par    Description:
        This callback will be called with the output of a previously called
        remote algorithm.
        <BR>
        Related function : RemoteAlgorithm_Execute()

\param      pData           Custom data passed at the RemoteAlgorithm_Execute()
                            call
\param      uiRequestId     Identifier of the request being replied
\param      rResult         Result code of the execution. Possible values are:
                            <UL>
                            <LI>GSS_OK
                            <BR>There was no error
                            <LI>GSE_DBFAILURE
                            <BR>An error occured on the DB while processing
                            the algorithm. The ubi.com server logs will
                            contain the exact nature of the error.
                            <LI>GSE_UNEXPECTED
                            <BR>This is an internal error meaning that either
                            the request or the reply was in a format that
                            could not be decoded.
                            <LI>GSE_HOSTUNREACHABLE
                            <BR>The remote algorithm execution service is
                            not available.
                            </UL>
\param      pOutput         An array of output values from the algorithm.
                            You do not have ownership of this array. The
                            memory will be freed when the callback function
                            returns.
\param      uiNumOutput     The number of values in the output array
*/
//============================================================================
typedef GSvoid (__stdcall *RemoteAlgorithm_OutputCB)(
    const GSvoid * pData, GSuint uiRequestId, GSRESULT rResult,
    const RAE_VALUE * pOutput, GSuint uiNumOutput);

/*! @} end of group_RAECB */

#endif // __GSREMOTEALGORITHMCB_H__
