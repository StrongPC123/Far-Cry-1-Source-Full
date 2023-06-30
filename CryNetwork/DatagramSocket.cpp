//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: DatagramSocket.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include <IPAddress.h>
#include "DatagramSocket.h"
#include "Network.h"

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


NRESULT CDatagramSocket::Create(SocketType st)
{
	int nErr = 0;
	m_nStartTick=::GetTickCount();
#if defined(LINUX)
	if ((m_hSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
#else
	if ((m_hSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
#endif
	{
		nErr = WSAGetLastError();
		return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr);
	}
	m_stSocketType = st;
	if (m_stSocketType == NonBlocking)
	{
#if defined(LINUX)
		if(fcntl( m_hSocket, F_SETFL, O_NONBLOCK ) < 0)
#else
		unsigned long nTrue = 1;
		if (ioctlsocket(m_hSocket, FIONBIO, &nTrue) == SOCKET_ERROR)
#endif
		{
			nErr = WSAGetLastError();
			Close();
			return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr);
		}
	}
	return NET_OK;
}

void CDatagramSocket::Close()
{
	if (m_hSocket == INVALID_SOCKET)
		return;
	// disable receiving 
#if defined(LINUX)
	setsockopt(m_hSocket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char *)&m_imMulticastReq, sizeof(m_imMulticastReq));
#endif
	shutdown(m_hSocket, 0x00);
	// should be chnaged for BSD socket close() instead closesocket()
	closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;
}

NRESULT CDatagramSocket::Listen(WORD wPort, CIPAddress *xaMulticastAddress, CIPAddress *ipLocalAddress)
{
	int nErr = 0;
	if (m_hSocket == INVALID_SOCKET)
		return NET_SOCKET_NOT_CREATED;
	sockaddr_in saLocalAddr;
	saLocalAddr.sin_family = AF_INET;
	saLocalAddr.sin_port = htons(wPort);  
	// check if we need to bind to a specific interface
	if (ipLocalAddress && ipLocalAddress->GetAsUINT())
	{
		// yes, we need to bind to this ip address
		saLocalAddr.sin_addr.s_addr = inet_addr(ipLocalAddress->GetAsString());
	}
	else
	{
		// no, use any interface
		saLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	// allow many servers on the same machine to list to the 
	if (xaMulticastAddress)
	{
		BOOL bReuse=true;
#if defined(LINUX)
		if(setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEADDR,(const char *)&bReuse,sizeof(BOOL)) < 0)
#else
		if(setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEADDR,(const char *)&bReuse,sizeof(BOOL)) == SOCKET_ERROR)
#endif
		{
			nErr = WSAGetLastError();

			GetISystem()->GetILog()->Log("setsockopt 1 failed with registering multicast (WSAGetLastError returned %d)",nErr);
		}
	}
#if defined(LINUX)
	BOOL bReuse=true;
	if(setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEADDR,(const char *)&bReuse,sizeof(BOOL)) < 0)
	{
		nErr = WSAGetLastError();
		GetISystem()->GetILog()->Log("setsockopt 1 failed with setting reusablity to socket (WSAGetLastError returned %d)",nErr);
	}
	if (bind(m_hSocket, (struct sockaddr*)&saLocalAddr, sizeof(saLocalAddr)) < 0)
#else
	if (bind(m_hSocket, (struct sockaddr*)&saLocalAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
#endif
	{
		nErr = WSAGetLastError();
		GetISystem()->GetILog()->Log("Registering Multicast: bind failed(%d)",nErr);
		Close();
		return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr);
	}

	sockaddr_in sockname;
#if defined(LINUX)
	socklen_t size = (socklen_t)sizeof(sockaddr_in);
#else
	int size = sizeof(sockaddr_in);
#endif
	getsockname(m_hSocket, (sockaddr *)&sockname, &size);

	CIPAddress local(&saLocalAddr);
	CIPAddress bound(&sockname);

	GetISystem()->GetILog()->Log("NAMES Local %s   Bind %s", local.GetAsString(), bound.GetAsString());

	// Setup Multicast registration
	
	if (xaMulticastAddress)
	{
#ifdef _XBOX
		// DEBUG_BREAK;
#else
		struct ip_mreq imMulticastReq;

		if (ipLocalAddress && ipLocalAddress->GetAsUINT())
		{
			imMulticastReq.imr_interface.s_addr = inet_addr(ipLocalAddress->GetAsString());
			GetISystem()->GetILog()->Log("Registering Multicast: %s",ipLocalAddress->GetAsString());
		}
		else
		{
			imMulticastReq.imr_interface.s_addr = INADDR_ANY;
			GetISystem()->GetILog()->Log("Registering  Multicast: ANY");
		}

		imMulticastReq.imr_multiaddr.s_addr = xaMulticastAddress->GetAsUINT();
#if defined(LINUX)
		if (setsockopt(m_hSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&imMulticastReq, sizeof(ip_mreq))  < 0) 
#else
		if(setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&imMulticastReq, sizeof(ip_mreq)) == SOCKET_ERROR)
#endif
		{
			// Problem: http://support.microsoft.com/default.aspx?scid=kb;en-us;Q257460
			// But: but we need Ws2_32.lib for UBI.com integration
			// Solution: make sure the libs are coming in in this order: Wsock32.lib Ws2_32.lib

			nErr = WSAGetLastError();

			GetISystem()->GetILog()->Log("setsockopt 2 failed with registering multicast (WSAGetLastError returned %d)",nErr);

			//NET_TRACE(EnumerateError(MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr)));
			Close();
			return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr);
		}
#if defined(LINUX)
		const int TTL=255; 
		if(setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, &TTL, sizeof(TTL)) < 0)
		{
			nErr = WSAGetLastError();

			GetISystem()->GetILog()->Log("setsockopt 2 failed with setting TTL for multicast (WSAGetLastError returned %d)",nErr);

			//NET_TRACE(EnumerateError(MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr)));
			Close();
			return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr);
		}
		m_imMulticastReq = imMulticastReq;	//store for call to IP_DROP_MEMBERSHIP
#endif

#endif
	}
	return NET_OK;
}

NRESULT CDatagramSocket::GetSocketAddresses(CIPAddress *pAddr, DWORD nMaCIPAddresses)
{
#ifdef _XBOX
	return NET_FAIL;
#else
	if (m_hSocket == INVALID_SOCKET)
		return NET_SOCKET_NOT_CREATED;
	DWORD i;
	char	buf[256];
	struct	hostent *hp;
	sockaddr_in port;
#if defined(LINUX)	
	socklen_t n;
#else
	int n;
#endif
#ifndef PS2	
	getsockname(m_hSocket, (sockaddr *)&port, &n);
#else	//PS2
	getsockname((int)m_hSocket, (sockaddr *)&port, (unsigned int *)&n);
#endif	//PS2
	if (!gethostname(buf, sizeof(buf))) 
	{
		hp = gethostbyname(buf);
		if (hp) 
		{
			//			if (hp->h_addrtype != AF_INET) 
			//				CLog::Log("gethostbyname: address type was not AF_INET\n");
			// CLog::Log("Hostname: %s\n", hp->h_name);
			
			i = 0;
			while (hp->h_aliases[i]) 
			{
				//	CLog::Log("Alias: %s\n", hp->h_aliases[i]);
				i++;
			}
			i = 0;
			while (hp->h_addr_list[i] && i < nMaCIPAddresses)
			{
				sockaddr_in temp;
				memcpy(&(temp.sin_addr), hp->h_addr_list[i], hp->h_length);
				temp.sin_port = port.sin_port;
				pAddr[i].Set(&temp);
				i++;
			}
			
			return NET_OK;			
		}
	}
	
	return NET_FAIL;
#endif
}

//////////////////////////////////////////////////////////////////////////
NRESULT CDatagramSocket::Send(BYTE *pBuffer, int nLenBytes, CIPAddress *saAddress)
{
	if (m_hSocket == INVALID_SOCKET)
		return NET_SOCKET_NOT_CREATED;
	if (!saAddress)
		saAddress = &m_saDefaultAddress;
	if (sendto(m_hSocket, (const char *)pBuffer, nLenBytes, 0, (sockaddr*)&saAddress->m_Address, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		int nErr = WSAGetLastError();
		//			Close();
		return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr);
	}
	/// compute the bandwitdh///////////////////////
	if ((::GetTickCount() - m_nStartTick)>1000)
		ComputeBandwidth();

	m_nSentBytesInThisSec += nLenBytes;
	m_nSentPacketsInThisSec++;
	///////////////////////////////////////////////

	CNetwork *pNetwork = (CNetwork*)GetISystem()->GetINetwork();
	if (pNetwork && pNetwork->GetLogLevel() == 1)
	{
		CryLog( "[NET] Send to %s, PacketSize=%d bytes",saAddress->GetAsString(),nLenBytes );
	}

	return NET_OK;
}

//////////////////////////////////////////////////////////////////////////
NRESULT CDatagramSocket::Receive(unsigned char *pBuf/*[MAX_UDP_PACKET_SIZE]*/, int nBufLen, int &nRecvBytes, CIPAddress &pFrom)
{
	if (m_hSocket == INVALID_SOCKET)
		return NET_SOCKET_NOT_CREATED;
	int nRetValue;
#if defined(LINUX)
	socklen_t n = (socklen_t)sizeof(sockaddr_in);
#else
	int n = sizeof(sockaddr_in);
#endif
#if defined(LINUX)
	if ((nRetValue = recvfrom(m_hSocket, (char *)pBuf, nBufLen, 0, (sockaddr*)&pFrom.m_Address, &n)) < 0)
#else
	if ((nRetValue = recvfrom(m_hSocket, (char *)pBuf, nBufLen, 0, (sockaddr*)&pFrom.m_Address, &n)) == SOCKET_ERROR)
#endif
	{
#if !defined(LINUX)
		int nErr = GetLastError();
		switch (nErr)
		{
		case WSAEWOULDBLOCK:
			nRecvBytes = 0;
			return NET_OK; 
			break;
		case WSAEMSGSIZE:
			//<<FIXME>> warning message "packet oversize"
			return NET_OK;
			break;
		default:
			return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, nErr);
#else
			return MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, errno);
#endif

#if !defined(LINUX)
			break;
		}
#endif
	}
	nRecvBytes = nRetValue;
	/// compute the bandwith///////////////////////
	if ((::GetTickCount() - m_nStartTick)>1000)
	{
		ComputeBandwidth();
	}
	m_nReceivedBytesInThisSec += nRecvBytes;
	m_nReceivedPacketsInThisSec++;
	///////////////////////////////////////////////

	CNetwork *pNetwork = (CNetwork*)GetISystem()->GetINetwork();
	if (pNetwork && pNetwork->GetLogLevel() == 1)
	{
		CryLog( "[NET] Recv from %s, PacketSize=%d bytes",pFrom.GetAsString(),nRecvBytes );
	}

	return NET_OK;
}
const char *GetHostName()
{
#ifdef _XBOX
	//DEBUG_BREAK;
	return "__XBOX__";
#else
	static char szBuf[56];
	memset(szBuf, 0, sizeof(szBuf));
	gethostname(szBuf, sizeof(szBuf));
	return szBuf;
#endif
}

//////////////////////////////////////////////////////////////////////////
const char* CDatagramSocket::GetHostName()
{
#ifdef _XBOX
	//DEBUG_BREAK;
	return "__XBOX__";
#else
	static char szBuf[56];
	memset(szBuf, 0, sizeof(szBuf));
	gethostname(szBuf, sizeof(szBuf));
	return szBuf;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CDatagramSocket::ComputeBandwidth()
{
	m_fOutgoingKbPerSec = ((float)(m_nSentBytesInThisSec*8))/1024.f;
	m_fIncomingKbPerSec = ((float)(m_nReceivedBytesInThisSec*8))/1024.f;
	m_nOutgoingPacketsPerSec = m_nSentPacketsInThisSec;
	m_nIncomingPacketsPerSec = m_nReceivedPacketsInThisSec;

	m_nStartTick=::GetTickCount();

	m_nSentBytesInThisSec = 0;
	m_nReceivedBytesInThisSec = 0;
	m_nSentPacketsInThisSec = 0;
	m_nReceivedPacketsInThisSec = 0;
}
