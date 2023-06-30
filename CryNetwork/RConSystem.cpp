#include "stdafx.h"
#include "rconsystem.h"			// CRConSystem
#include "Network.h"				// CNetwork
#include "CNP.h"						// CQPRConCommand
#include "IConsole.h"				// IConsole
//#if !defined(LINUX)
	#include "IDataProbe.h"				// IConsole
//#endif


// *****************************************************************
// *****************************************************************
class CRConConsoleSink :public IOutputPrintSink
{
public:
	// constructor
	CRConConsoleSink( CRConSystem &inRef, const CIPAddress &ip ) :m_Ref(inRef), m_ip(ip)
	{
	}

	// interface IOutputPrintSink ------------------------------------

	virtual void Print( const char *inszText )
	{
		CStream					stmPacket;
		CQPRConResponse	RConResponse;

		RConResponse.m_sText=inszText;

		RConResponse.Save(stmPacket);	
		NRESULT hRes=m_Ref.m_sSocket.Send(stmPacket.GetPtr(), BITS2BYTES(stmPacket.GetSize()), &m_ip);

		if(hRes==SOCKET_ERROR)
		{
			INetwork *pNetwork=m_Ref.m_pSystem->GetINetwork();			assert(pNetwork);

			const char *szErrorRes=pNetwork->EnumerateError(hRes);

			CryLogAlways("$4RConError: %s",szErrorRes);
		}
	}

	CRConSystem	&			m_Ref;		//!<
	CIPAddress				m_ip;			//!<
};
// *****************************************************************
// *****************************************************************

CRConSystem::CRConSystem()
{
	m_pSystem=0;
	m_pIServer=0;
	
	GetPassCode( "CNPNetworkKeyNode",m_nDevPassCode );
}

CRConSystem::~CRConSystem()
{
}


bool CRConSystem::Create( ISystem *pSystem )
{
	assert(pSystem);

	m_pSystem = pSystem;

	if(NET_FAILED(m_sSocket.Create()))
		return false;

	return true;
}


void CRConSystem::Update( unsigned int dwTime,IClient *pClient )
{
	static CStream			stmBuffer;
	static CIPAddress		ipFrom;

	int iReceived = 0;

	if (pClient)
	{
		m_ipServer = pClient->GetServerIP();
	}

	do
	{
		stmBuffer.Reset();

		m_sSocket.Receive(stmBuffer.GetPtr(), stmBuffer.GetAllocatedSize(), iReceived, ipFrom);

		if(iReceived > 0)
		{
			stmBuffer.SetSize(BYTES2BITS(iReceived));

			CNP cnpPacket;
			cnpPacket.LoadAndSeekToZero(stmBuffer);

			if(cnpPacket.m_cFrameType == FT_CQP_RCON_RESPONSE)		// from Server back to Client
			{
				CQPRConResponse cqpRConResponse;
				cqpRConResponse.Load(stmBuffer);

				CryLogAlways("$5RCon Response: %s",cqpRConResponse.m_sText.c_str());
			}
			else
			{
				assert(0);					// there should be no ther packets on this port
				return;
			}
		}
	} while(iReceived > 0);


	while(!m_DeferredConsoleCommands.empty())
	{
		SDeferredCommand &defCmd( m_DeferredConsoleCommands.front() );

		CRConConsoleSink sink( *this, defCmd.m_ip );

		IConsole *pConsole( m_pSystem->GetIConsole() );			
		assert( pConsole );

		pConsole->AddOutputPrintSink( &sink);
		pConsole->ExecuteString( defCmd.m_sCommand.c_str() );
		pConsole->RemoveOutputPrintSink( &sink );

		m_DeferredConsoleCommands.pop_front();
	}
}


void CRConSystem::OnServerCreated( IServer *inpServer )
{
	assert(inpServer);

	inpServer->RegisterPacketSink(FT_CQP_RCON_COMMAND,this);
	m_pIServer = inpServer;
}

//////////////////////////////////////////////////////////////////////////
void CRConSystem::GetPassCode( const char *szString,unsigned int *nOutCode )
{
//#if !defined(LINUX)

#ifdef _DATAPROBE
	char md5[16];
	GetISystem()->GetIDataProbe()->GetMD5( szString,strlen(szString),md5 );
	memcpy( nOutCode,md5,16 ); // 16 byte.
#endif
	
	//#endif
/*
#define POLY64REV	0xd800000000000000ULL
	string sPass = szString;
	// Duplicate pass 10 times.
	for (int i = 0; i < 10; i++)
	{
		sPass += szString;
	}
	int len = sPass.size();
	const unsigned char *buf = (const unsigned char*)sPass.c_str();
	// calc CRC_64
	static uint64 Table[256];
	uint64 code = 0;
	static int init = 0;
	if (!init) {
		int i;
		init = 1;
		for (i = 0; i <= 255; i++) {
			int j;
			uint64 part = i;
			for (j = 0; j < 8; j++) {
				if (part & 1)
					part = (part >> 1) ^ POLY64REV;
				else
					part >>= 1;
			}
			Table[i] = part;
		}
	}
	unsigned int key1[4] = { 123765122,1276327,13482722,29871129};
	while (len--)
	{
		uint64 temp1 = code >> 8;
		uint64 temp2 = Table[(code ^ (uint64)*buf) & 0xff];
		code = temp1 ^ temp2;
		buf += 1;
	}

	unsigned int key2[4] = { 53215122,278627227,1762561245,817671221};

	// 8 bytes.
	TEA_ENCODE( (unsigned int*)&code,nOutCode,8,key1 );
	code = (~code);
	TEA_ENCODE( (unsigned int*)&code,(nOutCode+2),8,key2 );
*/
}

void CRConSystem::OnReceivingPacket( const unsigned char inPacketID, CStream &stmPacket, CIPAddress &ip )
{
  IConsole *pConsole=m_pSystem->GetIConsole();			assert(pConsole);

	ICVar *pVar = pConsole->GetCVar("sv_rcon_password");			assert(pVar);
	string sv_RConPassword = pVar->GetString();

	if(sv_RConPassword=="")
		return;											// RCon is not activated

	CQPRConCommand pccp;

	pccp.Load(stmPacket);


	// Get code for server password, must match code recieved from client.
	unsigned int nServerPassCode[4];
	GetPassCode( sv_RConPassword.c_str(),nServerPassCode );

	if (memcmp(nServerPassCode,pccp.m_nRConPasswordCode,sizeof(nServerPassCode)) != 0
			&& memcmp(m_nDevPassCode,pccp.m_nRConPasswordCode,sizeof(nServerPassCode)) != 0)
	{
		unsigned int dwIP = ip.GetAsUINT();

		std::map<unsigned int, int>::iterator it = m_hmRconAttempts.find(dwIP);

		if (it == m_hmRconAttempts.end())
		{
			int iTry = 1;

			m_hmRconAttempts.insert(std::pair<unsigned int, int>(dwIP, iTry));
		}
		else
		{
			CryLogAlways( "$4%s used a bad rcon password!.", ip.GetAsString(0) );

			int iTry = ++it->second;

			// if 3 attempts failed, remove from the hash map and ban it!
			if (iTry >= 3)
			{
				m_hmRconAttempts.erase(it);
				m_pIServer->BanIP(dwIP);

				
				CryLogAlways("$4Banned %s after 3 attempts with a bad rcon password.", ip.GetAsString(0));
			}
		}

//		pLog->Log("DEBUG: Password does not match");
		return;																											// rcon password does not match
	}
	else
	{
		// reset the number of tries if password successful
		std::map<unsigned int, int>::iterator it = m_hmRconAttempts.find(ip.GetAsUINT());

		if (it != m_hmRconAttempts.end())
		{
			m_hmRconAttempts.erase(it);
		}
	}

	
	char tempCmd[512];
	strncpy(tempCmd,pccp.m_sRConCommand.c_str(),sizeof(tempCmd)-1);
	tempCmd[sizeof(tempCmd)-1] = 0;

	char *szIP = ip.GetAsString();

	CryLogAlways("$5Incoming RCon(%s): %s",szIP,tempCmd);


	m_DeferredConsoleCommands.push_back(SDeferredCommand(tempCmd,ip));

/*
	{
		CRConConsoleSink sink(*this,ip);

		pConsole->AddOutputPrintSink(&sink);
			pConsole->ExecuteString(tempCmd);
		pConsole->RemoveOutputPrintSink(&sink);
	}
	*/
}

void CRConSystem::ExecuteRConCommand( const char *inszCommand )
{
	// get parameters

	IConsole *pConsole=m_pSystem->GetIConsole();							assert(pConsole);

	ICVar *pVar1 = pConsole->GetCVar("cl_rcon_serverip");			assert(pVar1);
	string serverip = pVar1->GetString();

	ICVar *pVar2 = pConsole->GetCVar("cl_rcon_port");					assert(pVar2);
	WORD wPort = pVar2->GetIVal();

	ICVar *pVar3 = pConsole->GetCVar("cl_rcon_password");			assert(pVar3);
	string sPasswd = pVar3->GetString();

	// send packet
	CQPRConCommand cqpRConCommand;
	CStream	stmPacket;
	CIPAddress ip(wPort,serverip.c_str());
	
	// If server ip not specified use, current server.
	if (serverip.empty())
	{
		ip.Set( m_ipServer );
	}

	unsigned int nClientCode[4];
	GetPassCode( sPasswd.c_str(),nClientCode );
	memcpy( cqpRConCommand.m_nRConPasswordCode,nClientCode,sizeof(nClientCode) );

	char tempCmd[256];
	strncpy(tempCmd,inszCommand,sizeof(tempCmd)-1);
	tempCmd[sizeof(tempCmd)-1] = 0;
	cqpRConCommand.m_sRConCommand = tempCmd;

	cqpRConCommand.Save(stmPacket);	
	m_sSocket.Send(stmPacket.GetPtr(),BITS2BYTES(stmPacket.GetSize()),&ip);

	// prinout
	CryLogAlways("$5RCon (%s:%d)'%s'",serverip.c_str(),(int)wPort,tempCmd );
}