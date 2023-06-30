//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: ipaddress.h
//  Description: ip address wrapper
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#ifndef _IP_ADDRESS_H_
#define _IP_ADDRESS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Stream.h>

#include "platform.h"

#ifdef LINUX
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif //LINUX

struct _SOCKADDR_IN
{
	short sin_family;
	unsigned short sin_port;
#if defined(LINUX)
	union
	{
		struct in_addr_windows sin_addr_win;
		struct in_addr sin_addr;
	};
#else
	struct in_addr sin_addr;
#endif
	char sin_zero[8];
};

class CStream;


#ifdef WIN32
#include <winsock.h>
#endif //WIN32

#ifdef PS2
#include <SocketManager.h>
#define PS2_MEM_SOCKET
#endif //PS2

/*
#ifdef _XBOX
#include <xtl.h>
// #include <winsockx.h>
#endif
#ifndef PS2
struct _SOCKADDR_IN{
   short sin_family;
   unsigned short sin_port;
   struct in_addr sin_addr;
   char sin_zero[8];
};
#include <winsock.h>
#else
#include <SocketManager.h>
//#	define PS2_SONY_SOCKET
#define PS2_MEM_SOCKET
#endif
*/

inline unsigned short __ntohs(unsigned short us)
{
	unsigned short nTemp=(us>>8)|(us<<8);
	return nTemp;
}
/* NOTE FOR PS2 PROGRAMMERS ABOUT THIS CLASS
	I rollback this file to our version because 
	your change caused a bug inside our version.

	in your version you changed this 
		m_Address.sin_addr.S_un.S_addr
	to this
		m_Address.sin_addr.S_addr

	but you also replaced 
		m_Address.sin_port
	with
		m_Address.sin_addr.S_addr

	be careful because for some strange reason
	was partially working :)	
*/
#if defined(LINUX)
	#define ADDR sin_addr_win.S_un.S_addr
#else
	#define ADDR sin_addr.S_un.S_addr
#endif

class CIPAddress
{
public:
	CIPAddress(WORD wPort,const char *sAddress)
	{
		m_Address.sin_family = AF_INET;
		m_Address.sin_port = htons(wPort);
		m_Address.ADDR = inet_addr(sAddress);
		m_Address.sin_zero[0] = 0;

		if(m_Address.ADDR == INADDR_NONE)
		{
			struct hostent *pHostEntry;
			pHostEntry = gethostbyname(sAddress);

			if(pHostEntry)
				m_Address.ADDR = *(unsigned int *)pHostEntry->h_addr_list[0];
			else
				m_Address.ADDR = 0;
		}
	}
	
	CIPAddress(sockaddr_in *sa)
	{
		
		memcpy(&m_Address, sa, sizeof(sockaddr_in));
	}
	
	CIPAddress()
	{
		
		memset(&m_Address, 0, sizeof(sockaddr_in));
	}
	
	CIPAddress(const CIPAddress &xa)
	{
		
		memcpy(&m_Address, &xa.m_Address, sizeof(sockaddr_in));
	}
	
	virtual ~CIPAddress()
	{
//		delete &m_Address;
	}
	bool IsLocalHost()
	{
		if(m_Address.ADDR==inet_addr("127.0.0.1"))
			return true;
		return false;
	}
		
	inline void Set(WORD wPort, char *sAddress);
	inline void Set(WORD wPort, UINT dwAddress);
	inline void Set(sockaddr_in *sa);
	inline void Set(const CIPAddress &xa);
	UINT GetAsUINT() const;
	inline const CIPAddress& operator =(const CIPAddress& xa);
	inline bool operator !=(const CIPAddress& s1);
	inline bool operator ==(const CIPAddress& s1);
	inline bool operator <(const CIPAddress& s1) const ;
	inline char *GetAsString(bool bPort = false) const;
	inline bool Load(CStream &s);
	inline bool Save(CStream &s);
public:
	struct _SOCKADDR_IN m_Address;
};

inline void CIPAddress::Set(WORD wPort, char *sAddress)
{
	m_Address.sin_family = AF_INET;
	m_Address.sin_port = htons(wPort);
	m_Address.ADDR = inet_addr(sAddress);
	m_Address.sin_zero[0] = 0;

	// win2003 server edition compatibility
	// the behaviour changed in this OS version
	// inet_addr returns INADDR_NONE instead of 0 when an empty string is passed.
	if(m_Address.ADDR == INADDR_NONE)
	{
		m_Address.ADDR = 0;
	}
}

inline void CIPAddress::Set(WORD wPort, UINT dwAddress)
{
	m_Address.sin_family = AF_INET;
	m_Address.sin_port = htons(wPort);
	m_Address.ADDR = dwAddress;
	m_Address.sin_zero[0] = 0;
}



inline void CIPAddress::Set(sockaddr_in *sa)
{
	memcpy(&m_Address, sa, sizeof(sockaddr_in));
}


inline char *CIPAddress::GetAsString(bool bPort) const
{
	static char s[64];
	if (bPort)
#ifndef LINUX	
		wsprintf(s, "%i.%i.%i.%i:%i", m_Address.sin_addr.S_un.S_un_b.s_b1,
		m_Address.sin_addr.S_un.S_un_b.s_b2,
		m_Address.sin_addr.S_un.S_un_b.s_b3,
		m_Address.sin_addr.S_un.S_un_b.s_b4, __ntohs(m_Address.sin_port));
#else	//LINUX
		sprintf(s, "%s:%i", 
			inet_ntoa(m_Address.sin_addr), 
			ntohs(m_Address.sin_port)
		);
#endif	//LINUX
	else
#ifndef LINUX
		wsprintf(s, "%i.%i.%i.%i", m_Address.sin_addr.S_un.S_un_b.s_b1,
		m_Address.sin_addr.S_un.S_un_b.s_b2,
		m_Address.sin_addr.S_un.S_un_b.s_b3,
		m_Address.sin_addr.S_un.S_un_b.s_b4);
#else	//LINUX
		sprintf(s, "%s", 
			inet_ntoa(m_Address.sin_addr)
		);
		//sprintf(s, "%i", InAddress());
#endif	//LINUX
	return s;
}

inline UINT CIPAddress::GetAsUINT() const
{
	return m_Address.ADDR;
}

inline void CIPAddress::Set(const CIPAddress &xa)
{
	memcpy(&m_Address, &xa.m_Address, sizeof(_SOCKADDR_IN));
}

inline bool CIPAddress::operator ==(const CIPAddress& s1)
{
	return (!(memcmp(&s1.m_Address, &m_Address, sizeof(_SOCKADDR_IN))))?true:false;
}

inline bool CIPAddress::operator <(const CIPAddress& s1) const 
{
	if(s1.m_Address.ADDR <m_Address.ADDR)
	{
		return true;
	}
	else
	{
		if(s1.m_Address.ADDR==m_Address.ADDR)
		{
			if(s1.m_Address.sin_port<m_Address.sin_port)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
}



inline bool CIPAddress::operator !=(const CIPAddress& s1) 
{
	return (memcmp(&s1.m_Address, &m_Address, sizeof(sockaddr_in)))?true:false;
}

inline const CIPAddress& CIPAddress::operator =(const CIPAddress& xa)
{
	Set(xa);
	return *this;
}

inline bool CIPAddress::Load(CStream &s)
{
	if (!s.Read((unsigned int&)(m_Address.ADDR)))
		return false;
	if (!s.Read(m_Address.sin_port))
		return false;
	return true;
}

inline bool CIPAddress::Save(CStream &s)
{
	if (!s.Write((unsigned int&)m_Address.ADDR))
		return false;
	if (!s.Write(m_Address.sin_port))
		return false;
	return true;
}

#endif
