//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: DatagramSocket.h
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef _DATAGRAM_SOCKET_H_
#define _DATAGRAM_SOCKET_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_UDP_PACKET_SIZE 8192
// struct ip_mreq mreq;
// mreq is the ip_mreqstructure
//{
//  struct in_addr imr_multiaddr;  // The multicast group to join 
//  struct in_addr imr_interface;  // The interface to join on
//}
// multicast sample addr is --> "234.5.6.7"

#include "Stream.h"
#include <IPAddress.h>

#ifndef _XBOX
#if _MSC_VER > 1000
#pragma comment(lib, "WSOCK32.LIB")
#endif // _MSC_VER > 1000
#endif

#ifdef LINUX
#include <time.h>
#endif //LINUX

/*inline int	gethostname(char *__name, size_t __len)
{
#pragma message ("gethostname not implemented")
	return 0;
}
*/

class CDatagramSocket
{
public:
	enum SocketType
	{
		Blocking,
		NonBlocking
	};

	//! constructor
	CDatagramSocket()
	{
		m_hSocket=INVALID_SOCKET;

		m_nSentBytesInThisSec=0;
		m_nReceivedBytesInThisSec=0;
		m_nSentPacketsInThisSec=0;
		m_nReceivedPacketsInThisSec=0;

		m_fIncomingKbPerSec=0.0f;
		m_fOutgoingKbPerSec=0.0f;
		m_nIncomingPacketsPerSec=0;
		m_nOutgoingPacketsPerSec=0;
	}
	//! destructor
	virtual ~CDatagramSocket()
	{
		Close(); 
		//<<FIXME>>
	}
	//!
	NRESULT Create(SocketType st = NonBlocking);
	//!
	NRESULT Listen(WORD wPort, CIPAddress *xaMulticastAddress = NULL, CIPAddress *ipLocalAddress = 0);
	//!
	void SetDefaultTarget(CIPAddress &saAddress)
	{
		m_saDefaultAddress.Set(saAddress);
	}
	//!
	NRESULT Send(BYTE *pBuffer, int nLenBytes, CIPAddress *saAddress = NULL);
	//!
	NRESULT Receive(unsigned char *pBuf/*[MAX_UDP_PACKET_SIZE]*/, int nBufLen, int &nRecvBytes, CIPAddress &pFrom);

	const char *GetHostName();
	//!
	void ComputeBandwidth();
	//!
	NRESULT GetSocketAddresses(CIPAddress *pAddr, DWORD nMaCIPAddresses);
	//!
	int GetLastError(){return WSAGetLastError();}
	//!
	void Close();

private:
	SOCKET						m_hSocket;										//!<
	SocketType				m_stSocketType;								//!<
	CIPAddress				m_saDefaultAddress;						//!< Default target host and port [optional] for Send()
	unsigned int			m_nStartTick;									//!< tick for bandwitdh computation
	
	unsigned int			m_nSentBytesInThisSec;				//!< is counting up and reseted every second
	unsigned int			m_nReceivedBytesInThisSec;		//!< is counting up and reseted every second
	unsigned int			m_nSentPacketsInThisSec;			//!< is counting up and reseted every second
	unsigned int			m_nReceivedPacketsInThisSec;	//!< is counting up and reseted every second
#if defined(LINUX)
	struct ip_mreq		m_imMulticastReq;							//!< needed for call to IP_DROP_MEMBERSHIP
#endif

public:
	float							m_fOutgoingKbPerSec;					//!< is updated every second
	float							m_fIncomingKbPerSec;					//!< is updated every second
	unsigned int			m_nOutgoingPacketsPerSec;			//!< is updated every second
	unsigned int			m_nIncomingPacketsPerSec;			//!< is updated every second
};


#endif //_DATAGRAM_SOCKET_H_
