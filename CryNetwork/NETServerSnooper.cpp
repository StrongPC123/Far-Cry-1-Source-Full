#include "stdafx.h"
#include "ILog.h"
#include "CNP.h"
#include "ITimer.h"
#include "NETServerSnooper.h"


//------------------------------------------------------------------------------------------------- 
CNETServerSnooper::CNETServerSnooper()
: m_iWaitingCount(0),
	m_pSink(0),
	m_pSystem(0),
	cl_snooptimeout(0),
  cl_snoopretries(0),
  cl_snoopcount(0)
{
}

//------------------------------------------------------------------------------------------------- 
CNETServerSnooper::~CNETServerSnooper()
{
}

//------------------------------------------------------------------------------------------------- 
bool CNETServerSnooper::Create(ISystem *pSystem, INETServerSnooperSink *pSink)
{
	assert(pSystem);
	assert(pSink);

	m_pSystem = pSystem;
	m_pSink = pSink;

	if (NET_FAILED(m_sSocket.Create()))
	{
		return 0;
	}

	cl_snoopcount = m_pSystem->GetIConsole()->GetCVar("cl_snoopcount");
	cl_snoopretries = m_pSystem->GetIConsole()->GetCVar("cl_snoopretries");
	cl_snooptimeout = m_pSystem->GetIConsole()->GetCVar("cl_snooptimeout");

	return 1;
}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::Release()
{
	m_hmServerTable.clear();
	delete this;
}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::Update(unsigned int dwTime)
{
	if (m_hmServerTable.empty())
	{
		return;
	}

	m_dwCurrentTime = dwTime;

	int iReceived = 0;
	static CStream			stmBuffer;
	static CIPAddress		ipFrom;

	do
	{
		iReceived = 0;
		stmBuffer.Reset();

		m_sSocket.Receive(stmBuffer.GetPtr(), stmBuffer.GetAllocatedSize(), iReceived, ipFrom);

		if(iReceived > 0)
		{
			stmBuffer.SetSize(BYTES2BITS(iReceived));
		
			ProcessPacket(stmBuffer, ipFrom);
		}
	} while(iReceived > 0);

	// if we have waiting servers
	if (m_iWaitingCount > 0)
	{
		// handle timeouts
		ProcessTimeout();
	}

	int iMaxWaiting = NET_SNOOPER_MAXWAITING;

	if (cl_snoopcount && cl_snoopcount->GetIVal() > 0)
	{
		iMaxWaiting = cl_snoopcount->GetIVal();
	}

	while (m_iWaitingCount < iMaxWaiting)
	{
		if (!ProcessNext())
		{
			break;
		}
	}
}

void CNETServerSnooper::OnReceivingPacket( const unsigned char inPacketID, CStream &stmPacket, CIPAddress &ip )
{

}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::AddServer(const CIPAddress &ip)
{
	NETSnooperServer Server;

	Server.bDoing = 0;
	Server.cTry = 0;
	Server.dwStartTime = 0;
	Server.dwTimeout = 0;
	Server.ipAddress = ip;

	m_hmServerTable.insert(std::pair<CIPAddress, NETSnooperServer>(Server.ipAddress, Server));
}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::AddServerList(const std::vector<CIPAddress> &vIP)
{
	for (std::vector<CIPAddress>::const_iterator it = vIP.begin(); it != vIP.end(); ++it)
	{
		AddServer(*it);
	}
}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::ClearList()
{
	m_hmServerTable.clear();
	m_iWaitingCount = 0;
}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::QueryServer(CIPAddress &ip)
{
	CStream					stmPacket;
	CQPInfoRequest	cqpInfoRequest("status");

	cqpInfoRequest.Save(stmPacket);	
	m_sSocket.Send(stmPacket.GetPtr(), BITS2BYTES(stmPacket.GetSize()), (CIPAddress *)&ip);
}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::ProcessPacket(CStream &stmPacket, CIPAddress &ip)
{
	// since we are not expecting any response
	// since is either a timedout server,
	// or is some undesirable packet
	if (m_iWaitingCount < 1)
	{
		return;
	}

	CNP cnpPacket;
	cnpPacket.LoadAndSeekToZero(stmPacket);

	if (cnpPacket.m_cFrameType == FT_CQP_INFO_RESPONSE)
	{
		NETSnooperServer *pServer = 0;
		HMServerTableItor it = m_hmServerTable.find(ip);

		if (it == m_hmServerTable.end())
		{
			return;
		}

		pServer = &(it->second);

		if (!pServer->bDoing)	// was this expected ?
		{
			return;
		}

		// we got a good response
		// so process it
		CQPInfoResponse cqpInfoResponse;
		cqpInfoResponse.Load(stmPacket);

		if(m_pSink)
		{
			m_pSink->OnNETServerFound(ip, cqpInfoResponse.szResponse, m_dwCurrentTime - pServer->dwStartTime);
		}

		--m_iWaitingCount;
		assert(m_iWaitingCount >= 0);

		// remove server from table
		m_hmServerTable.erase(it);
	}
}

//------------------------------------------------------------------------------------------------- 
void CNETServerSnooper::ProcessTimeout()
{
	NETSnooperServer *pServer = 0;
	HMServerTableItor it = m_hmServerTable.begin();

	unsigned int dwRetry = NET_SNOOPER_RETRY;
	unsigned int dwTimeDelta = NET_SNOOPER_TIMEOUT;

	if (cl_snoopretries && cl_snoopretries->GetIVal() > 0)
	{
		dwRetry = cl_snoopretries->GetIVal();
	}

	if (cl_snooptimeout && cl_snooptimeout->GetFVal() > 0)
	{
		dwTimeDelta = (unsigned int)(cl_snooptimeout->GetFVal() * 1000.0f);
	}

	while(it != m_hmServerTable.end())
	{
		pServer = &(it->second);

		if ((m_dwCurrentTime >= pServer->dwTimeout) && pServer->bDoing)
		{
			pServer->cTry++;
			pServer->dwStartTime = m_dwCurrentTime;
			pServer->dwTimeout = m_dwCurrentTime + dwTimeDelta;

			if (pServer->cTry > dwRetry)
			{
				m_pSink->OnNETServerTimeout(it->second.ipAddress);
				// quit snooping this server
#if defined(LINUX)	//dunno why the replaced statement does not work on its own
				HMServerTableItor itTemp = it;
				itTemp++;
				m_hmServerTable.erase(it);
				it = itTemp;
#else
				it = m_hmServerTable.erase(it);
#endif
				--m_iWaitingCount;
				assert(m_iWaitingCount >= 0);
			}
			else
			{
				QueryServer(pServer->ipAddress);

				++it;
			}
		}
		else
		{
			++it;
		}
	}
}

//------------------------------------------------------------------------------------------------- 
bool CNETServerSnooper::ProcessNext()
{
	NETSnooperServer *pServer = 0;
	HMServerTableItor it = m_hmServerTable.begin();

	unsigned int dwTimeDelta = NET_SNOOPER_TIMEOUT;
	if (cl_snooptimeout && cl_snooptimeout->GetFVal() > 0)
	{
		dwTimeDelta = (unsigned int)(cl_snooptimeout->GetFVal() * 1000.0f);
	}

	while(it != m_hmServerTable.end())
	{
		pServer = &(it->second);

		if (pServer->bDoing)
		{
			++it;

			continue;
		}

		pServer->dwStartTime = m_dwCurrentTime;
		pServer->dwTimeout = m_dwCurrentTime + dwTimeDelta;
		pServer->bDoing = 1;

		QueryServer(pServer->ipAddress);
  
		m_iWaitingCount++;

		return 1;
	}

	return 0;
}
