//============================================================================
/*!
\file
\brief  The header file for the NAT Library

This header file describes the API for the NAT Library
*/
//============================================================================

#ifndef _NATLIBRARY_H_
#define _NATLIBRARY_H_

extern "C"
{
    
/*!
\mainpage gs-sdk-nat
\section intro Introduction
 This is the NAT Library.  It allows a program to find the external IP and port
 address of a socket.


\section description Description

	Definitions:
 
	- Game Port: The port that the game wants to find the external address of.
	- External Address: The Internet IP address and port of the game port.
 
	Getting started:
	- The game must only use UDP for its game communication.  You can't tunnel NATs
		using TCP.
	- The game must have a function that will send a buffer of data as a 
		UDP packet from the game port. See the callback ::GSNAT_SendMessageCB.
	- The game must have a function that this library can call, that will inform
		the game of the game ports's external address. See the callback 
		::GSNAT_RequestFinishedCB.
 
	Using the library:
	- The game calls GSNAT_Initialize().  If it fails the connection to the NAT
		Server could not be established.  Try a different NAT Server.
	- Continue calling GSNAT_Engine() in order to process the requests and keep
		the requested external addresses alive.
	- Call GSNAT_RequestNATAddress() to request the external address of a game
		port.  Mulitple game ports can have their external addresses reqested at
		once, but only one request per game port can be sent.
	- The ::GSNAT_RequestFinishedCB callback will inform the game of requested
		Game ports external address.
		
	Distributing the external address:
	- The Game Server must distribute its external address to all the Game
		Clients.  This can be done using the vpData parameter of
		LobbySend_CreateRoom() in gs-sdk-base SDK or pstGroupInfo paramter of
		RegServerSend_RegisterServerOnLobby() in the gs-sdk-regserver SDK.
	-	The Game Client will now get the Game Server's external address with the
 		CBLobbyRcv_NewRoom callback in gs-sdk-base or MSClient_GameServerCB in 
		gs-sdk-msclient.
	-	The Game Client must distribute its external address to the Game Server.
		This can be done using LobbySend_SetPlayerInfo() in the gs-sdk-base SDK.
	-	The Game Server will now get the Game Client's external address in the
		CBLobbyRcv_MemberJoined callback of gs-sdk-base or the
		CBRegServerRcv_LobbyServerMemberNew callback of gs-sdk-regserver
		
	Tunneling the NAT:
	- Now that the Game Server and Game Client know each others external address
		they must start sending "connection" messages to each others external
		address from their game ports.  When both sides receive a connection message
		their NATs have been tunneled.
	- This means that a Game Server must start sending messages to a Game Client
		before it has received any messages from it.  If it doesn't, its NAT may not
		allow message from the Game Client to reach it.
		
	Problems with NATs:
	- This won't work with all NATs.  Some open up a different external address
		for every endpoint.  This means that the external address when sending
		messages to the game server will be different then the external address that
		was opened when sending messages to NAT server.  Most NATs don't do this so
		most will behave properly.
	- If both NATs don't behave properly their is nothing a game can do.  The user
		will have to setup port forwarding on the Game Server.
	- If only one of the NATs is behaving properly the Game behind it will notice
		that it is receiving connection messages from a different port then the one
		it is sending its connection messages to.  For example, it is sending
		connection messages to 1.1.1.1:1200 but it is receving connection messages
		from 1.1.1.1:1300.  The Game on the other side is not receiving any
		messages.  When this is noticed it should stop sending messages to port 1200
		and switch to port 1300.  The other side should now start receiving them.
*/

/*! \defgroup Structures The Structure Definitions
\brief The definitions for the structures

These strutures are used by the library.
\{
*/

/// The Internet Address stored as a character string.
struct GSInternetAddress_char
{
	GSchar szIPAddress[IPADDRESSLENGTH]; ///< The IP Address or Domain Name.
	GSushort usPort; ///< The port in host byte order.
};

/// The Internet Address stored as a unsigned integer.
struct GSInternetAddress_uint
{
	GSuint uiIPAddress; ///< The IP Address in host byte order.
	GSushort usPort; ///< The port in host byte order.
};

/// \}


/*! \defgroup Callbacks The Callback Typedefs
\brief The typedefs for the callbacks

These typedefs describe the callback functions that the game must implement.
\{
*/

//============================================================================
// CallBack GSNAT_SendMessageCB
/*!
	\brief The callback to send data from the requested game socket.
	\par Description:
	In order for the NATServer to know the external address of the
	Game Socket a message must be received by the NAT Server from it.
 
	\par
	The implementation of this function must send the data on the Game Socket
	that matches the ubRequestID given by GSNAT_RequestNATAddress().  The library
	will use this function as if it is the BSD Socket function sendto().
 
	\param ubRequestID The Request ID that was assigned by
			GSNAT_RequestNATAddress().
	\param pvSendMessageData The same pointer that was passed to
			GSNAT_RequestNATAddress().
	\param pubData The data to send on the Game Socket.
	\param uiDataSize The size in bytes of the pubData parameter.
	\param pstNetAddress The Internet Address to send the data to.

*/
//============================================================================
typedef GSvoid (__stdcall *GSNAT_SendMessageCB)(GSubyte ubRequestID,
		GSvoid *pvSendMessageData, GSubyte *pubData, GSuint uiDataSize,
		const GSInternetAddress_uint *pstNetAddress);

//============================================================================
// CallBack GSNAT_RequestFinishedCB
/*!
	\brief The callback to inform the client the request has finished.
	\par Description:
	This callback tells the client what the External address of the requested socket
	it.  The possibe values for iResult are:
			- GSS_OK: There was no error.
			- GSE_CONNECTERROR: The connection to the NAT Server could not be
					established.
			- GSE_TIMEOUT: The request timed out.	
 
	\param ubRequestID The Request ID that was assigned by
			GSNAT_RequestExternalAddress().
	\param iResult The Result of the Callback.
	\param pvRequestFinishedData The same pointer that was passed to
			GSNAT_RequestExternalAddress().
	\param pstNATAddress The NAT address of the Game Socket.  NULL if GSFAIL.
 
*/
//============================================================================
typedef GSvoid (__stdcall *GSNAT_RequestFinishedCB)(GSubyte ubRequestID,
		GSRESULT iResult, GSvoid *pvRequestFinishedData,
		const GSInternetAddress_uint *pstExternalAddress);

/// \}


/*! \defgroup Functions The Functions
\brief The Functions in the library

\{
*/
//============================================================================
// Function GSNAT_Initialize
/*!
	\brief Initializes the library.
	\par Description:
	This function initializes the library so it can be used.
	\par
	If it fails the	connection to the NAT server could not be established and the
	client should	call it again with a different NAT servers address.  You do not
	have to call GSNAT_Uninitialize() if this function fails. Once it returns true
	the other functions in this library can be used.
	
	\param pstServerAddress The Internet address NAT Server.
	\param usLocalPort The local port to reserve for the NAT Server connection.
			Use 0 for the default value.
	\param szUsername The username of the client.
	\param szGameName The game name of the client.
	\param szVersion The version of the client.
	
	\retval GSS_OK The connection to the NAT Server was successful.
	\retval GSE_CONNECTERROR The connection to the NAT Server failed.
	\retval GSE_ALREADYINITIALIZED The library was already initialized.
*/
//============================================================================
GSRESULT __stdcall GSNAT_Initialize(const GSInternetAddress_char *pstServerAddress,
		GSushort usLocalPort, const GSchar *szUsername, const GSchar *szGameName,
		const GSchar *szVersion);

//============================================================================
// Function GSNAT_Uninitialize
/*!
	\brief Uninitializes the library.
	\par Description:
	This function uninitializes the library and frees all memory.  It should be
	called only when the game is finished with the sockets it requested the
	External Addresses of.  Don't uninitialize the library after receiving the
	::GSNAT_RequestFinishedCB as the library must contine to send pings to the NAT
	Server
	
	\retval GSS_OK The library was unintialized
	\retval GSE_NOTINITIALIZED The library wasn't initialized
*/
//============================================================================
GSRESULT __stdcall GSNAT_Uninitialize();

//============================================================================
// Function GSNAT_Engine
/*!
	\brief Runs the library.
	\par Description:
	This function must be called repetedly in order to process the requests and 
	keep the NAT Address alive.  Even after receiving the responses to the
	requests, the client must continue to call this function or the NAT may
	change the Game Sockets NAT Address.
	
	\retval GSS_OK There was no error
	\retval GSE_NOTINITIALIZED The library wasn't initialized
*/
//============================================================================
GSRESULT __stdcall GSNAT_Engine();

//============================================================================
// Function NAT_RequestNATAddress
/*!
	\brief Request the External Address of a Game Socket.
	\par Description:
	This function requests the External address of a Game Socket.  The Game Socket
	it self is not passed down.  Instead a function that can send data on the
	socket is used instead. See the ::GSNAT_SendMessageCB.
	\par
	The pvSendMessageData and	pvRequestFinsihedData are pointers that library
	passes to the callbacks.  These could be pointers to structures or objects
	that the Callbacks can use	to identify the Game Socket.  These pointers are
	not	used by the library in any way, they are just passed back in the
	callbacks.
	
	\param pubRequestID The request ID assigned by the library.  This can be used
			to identify the Game Socket.
	\param fSendMessageCB The callback to use to send data on the Game Socket.
			See ::GSNAT_SendMessageCB.
	\param pvSendMessageData The pointer to pass to the ::GSNAT_SendMessageCB
			callback.
	\param fRequestFinishedCB The callback to use to inform the client that the
			request has finished.  See ::GSNAT_RequestFinishedCB
	\param pvRequestFinishedData The pointer to pass to the
			::GSNAT_RequestFinishedCB callback.
	
	\retval GSS_OK There was no error
	\retval GSE_NOTINITIALIZED The library wasn't initialized
*/
//============================================================================
GSRESULT __stdcall GSNAT_RequestNATAddress(GSubyte *pubRequestID,
		GSNAT_SendMessageCB fSendMessageCB, GSvoid *pvSendMessageData,
		GSNAT_RequestFinishedCB fRequestFinishedCB, GSvoid *pvRequestFinishedData);
/// \}
 
} // extern "C"

#endif //_NATLIBRARY_H_
