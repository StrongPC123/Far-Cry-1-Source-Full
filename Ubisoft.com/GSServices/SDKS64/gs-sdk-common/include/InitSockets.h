//****************************************************************************
//*   Author:  Scott Schmeisser gsdevelopers@ubisoft.com
//*   Date:    5/15/01 10:05:17 AM
 /*!  \file    InitSockets.h
  *   \brief   Functions used to initialize the socket library in different
  *            platforms
  *	  Socket loading/unloading for different platforms.
  */
//****************************************************************************

#ifndef _INITSOCKETS_H
#ifndef DOX_SKIP_THIS
#define _INITSOCKETS_H
#endif // DOX_SKIP_THIS


extern "C" {

#if defined(GS_WIN32) || defined(GS_WIN64)


//============================================================================
// Function InitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 9:48:06 AM
/*!
 \brief	 (WIN32/XBOX/LINUX) Initialize the socket library
 \par       Description:
 Initialize the socket library with an optional IP address to bind to.

 \return    The status of the call to the function

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param		szIPAddress  the IP address to bind to
*/
//============================================================================
GSbool __stdcall InitializeSockets(const GSchar *szIPAddress = NULL);

//============================================================================
// Function InitializeSockets_SetOnConnectTimeout
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		28/08/2002 10:05:51 AM
/*!
 \brief	 (WIN32) Sets the Timeout value when connecting.
 \par       Description:
 Set the amount of time to wait for a connection to establish.
 Defaults to 5 seconds;

 \return    The status of the call to the function.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param	iTimeOut  The number of seconds to wait for the connection.

*/
//============================================================================
GSbool __stdcall InitializeSockets_SetOnConnectTimeout(GSint iTimeOut = 5);

//============================================================================
// Function UninitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 9:50:18 AM
/*!
 \brief	 (WIN32/XBOX/LINUX/PSX2) Uninitialize the socket library
 \par       Description:
 Unload the socket library.

 \return    The status of the call to the functions.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure
*/
//============================================================================
GSbool __stdcall UninitializeSockets();

#endif //GS_WIN32


#ifdef GS_XBOX
//============================================================================
// Function InitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 9:48:06 AM
/*!
 \brief	 (WIN32/XBOX/LINUX) Initialize the socket library
 \par       Description:
 Initialize the socket library with an optional IP address to bind to.

 \return    The status of the call to the function

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param		szIPAddress  the IP address to bind to
*/
//============================================================================
GSbool __stdcall InitializeSockets(GSchar *szIPAddress = NULL);

//============================================================================
// Function UninitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 9:50:18 AM
/*!
 \brief	 (WIN32/XBOX/LINUX/PSX2) Uninitialize the socket library
 \par       Description:
 Unload the socket library.

 \return    The status of the call to the functions.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure
*/
//============================================================================
GSbool __stdcall UninitializeSockets();
#endif //GS_XBOX

#ifdef GS_PSX2

//============================================================================
// Function InitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 10:05:51 AM
/*!
 \brief	 (PSX2) Initialize the socket library
 \par       Description:
This function now longer iInitializes the socket library on PSX2.  The game must load all IOP modules
and network configuration itself.  The ubi.com SDKs are not built using libeenet.  See the libeenet
documantation in the SONY libraries on how to load and initialize the libeenet.

 \return    The status of the call to the function.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param	szIPAddress	The IP address to bound to.

*/
//============================================================================
GSbool __stdcall InitializeSockets(const GSchar *szIPAddress = NULL);

//============================================================================
// Function InitializeSockets_Test
// Author:		Scott Schmeisser sschmeisser@ubisoft.com
// Date:		01/04/2003 10:05:51 AM
/*!
 \brief	 (PSX2) Initialize the socket library
 \par       Description:
 Initialize the socket library on PSX2.  THIS FUNCTION IS NOT TO BE USED.
 IT IS FOR TEST PURPOSES ONLY.  USE THE ABOVE InitializeSockets() INSTEAD.

 \return    The status of the call to the function.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param	szIPAddress	The IP address to bound to.

*/
//============================================================================
GSbool __stdcall InitializeSockets_Test(GSchar *szIPAddress = NULL);

//============================================================================
// Function InitializeSockets_SetOnConnectTimeout
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		28/08/2002 10:05:51 AM
/*!
 \brief	 (PSX2) Sets the Timeout value when connecting.
 \par       Description:
 Set the amount of time to wait for a connection to establish.
 Defaults to 5 seconds;

 \return    The status of the call to the function.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param	iTimeOut  The number of seconds to wait for the connection.

*/
//============================================================================
GSbool __stdcall InitializeSockets_SetOnConnectTimeout(GSint iTimeOut = 5);

//============================================================================
// Function UninitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 9:50:18 AM
/*!
 \brief	 (WIN32/XBOX/LINUX/PSX2) Uninitialize the socket library
 \par       Description:
 Unload the socket library.

 \return    The status of the call to the functions.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure
*/
//============================================================================
GSbool __stdcall UninitializeSockets();
#endif GS_PSX2

#ifdef GS_LINUX
//============================================================================
// Function InitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 9:48:06 AM
/*!
 \brief	 (WIN32/XBOX/LINUX/PSX2) Initialize the socket library
 \par       Description:
 Initialize the socket library with an optional IP address to bind to.

 \return    The status of the call to the function

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param		szIPAddress  the IP address to bind to
*/
//============================================================================
GSbool __stdcall InitializeSockets(GSchar *szIPAddress = NULL);

//============================================================================
// Function InitializeSockets_SetOnConnectTimeout
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		28/08/2002 10:05:51 AM
/*!
 \brief	 (LINUX) Sets the Timeout value when connecting.
 \par       Description:
 Set the amount of time to wait for a connection to establish.
 Defaults to 5 seconds;

 \return    The status of the call to the function.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure

 \param	iTimeOut  The number of seconds to wait for the connection.

*/
//============================================================================
GSbool __stdcall InitializeSockets_SetOnConnectTimeout(GSint iTimeOut = 5);

//============================================================================
// Function UninitializeSockets
// Author:		Luc Bouchard  lbouchard@ubisoft.qc.ca
// Date:		14/09/2001 9:50:18 AM
/*!
 \brief	 (WIN32/XBOX/LINUX/PSX2) Uninitialize the socket library
 \par       Description:
 Unload the socket library.

 \return    The status of the call to the functions.

 \retval	GS_TRUE		on success
 \retval	GS_FALSE	on failure
*/
//============================================================================
GSbool __stdcall UninitializeSockets();
#endif

}
#endif //_INITSOCKETS_H
