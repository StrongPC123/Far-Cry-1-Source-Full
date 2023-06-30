#ifndef _GSSIMPLECONNECT_H
#define _GSSIMPLECONNECT_H
#include "define.h"

class clGSConnect;
class clConnectElem;
class clConnectList;
class clTCPClient;
class clUNIXClient;

//===================================================================================================
class clSimpleClient
{
private:
	clConnectElem* m_pstConnectElem;
	GSchar m_szAddress[129];

public:
    /* Constructor/Destructor */
    clSimpleClient( GSint lAliveDuring, GSint lRcvTimeout,
                    GSint iUDPSndBuf = 0, GSint iUDPRcvBuf = 0);
    ~clSimpleClient();
  
    /* To connect on a peer address using TCP */
    GSbool ConnectHost(GSchar *szHost, GSushort lPort);

#ifdef LINUX
	/* To connect to a peer UNIX process */
	GSbool ConnectUNIXHost(GSchar *sSockPipe);
#endif // LINUX

    /* Disconnect from peer address */
    GSbool Disconnect(GSvoid);

    /* Update messages receive/send */
    GSbool CheckConnection(GSvoid);
  
	/* To send a message (put in the send queue) (Priority : 0-31) */
	GSbool SendGuaranteed( GSubyte ucType, GSubyte ucPriority, GSvoid *pvMessage, GSint lMsgSize);
	GSbool SendLostable( GSubyte ucType, GSubyte ucPriority, GSvoid *pvMessage, GSint lMsgSize);

	/* To read a message (from the received queue) */
    GSvoid* ReadGuaranteed( GSubyte& rucType, GSint& rlSize);
    GSvoid* ReadLostable( GSubyte& rucType, GSint& rlSize);

	/* IP management */
    GSchar* GetPeerIPAddress(GSvoid);
	GSchar* GetLocalIPAddress(GSvoid);
	
	// Checks to see if the UDP connection has been established.
	// You must call CheckConnection, ReadGuaranteed and ReadLostable
	// between calls to IsUDPConnected to generate and send internal messages
	// that establish the UDP connection
	GSbool IsUDPConnected();
};

//===================================================================================================
class clSimpleServer
{
private:
    clConnectList* m_pstConnectList;

    GSint  m_lConnectedMode;
    GSint  m_lStillAliveDuring;
    GSint  m_lRcvDuring;
    GSchar  m_szAddress[129];
    
public:
    clSimpleServer( GSint lAliveDuring, GSint lRcvTimeout,
                    GSint iUDPSndBuf = 0, GSint iUDPRcvBuf = 0);
    ~clSimpleServer();
    
    /* Return the port reserved */
    GSbool ReservePort(GSushort lPort);
    
#ifdef LINUX
    /* Puts the UNIX server into listening mode (UNIX Sockets) */
    GSbool OpenSocket(GSchar *p_strSockPipe);
#endif // LINUX
    
    /* Return the ID of the new connection */
    GSint AcceptConnection(GSvoid);
    
    /* Return the ID of the element disconnected */
    GSint CheckDisconnection(GSvoid);

    /* To disconnect an element */
    GSbool DisconnectElement(GSint lId);

	/* To send a message (put in the send queue) (Priority : 0-31) */
	GSbool SendGuaranteed( GSint lId, GSubyte ucType, GSubyte ucPriority, GSvoid *pvMessage, GSint lMsgSize);
	GSbool SendLostable( GSint lId, GSubyte ucType, GSubyte ucPriority, GSvoid *pvMessage, GSint lMsgSize);
	GSbool SendGuaranteedToAll( GSubyte ucType, GSubyte ucPriority, GSvoid *pvMessage, GSint lMsgSize);
	GSbool SendLostableToAll( GSubyte ucType, GSubyte ucPriority, GSvoid *pvMessage, GSint lMsgSize);

	/* To read a message (from the received queue) */
    GSvoid* ReadGuaranteed( GSint& rlId, GSubyte& rucType, GSint& rlSize);
    GSvoid* ReadLostable( GSint& rlId, GSubyte& rucType, GSint& rlSize);

	/* IP management */
    GSchar* GetPeerIPAddress(GSint lId);
		
	// Checks to see if the UDP connection has been established for that
	// connection ID.  You must call CheckDisconnection, ReadGuaranteed and
	// ReadLostable between calls to IsUDPConnected to generate and send internal
	// messages that establish the UDP connection
		GSbool IsUDPConnected(GSint iID);
};

#endif // _GSSIMPLECONNECT_H
