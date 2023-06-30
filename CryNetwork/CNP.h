//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: CNP.h
//  Description: Crytek network protocol declarations
//
//	History:
//	 * 7/25/2001 Created by Alberto Demichelis
//   * 11/5/2003 MM cleaned up, documented
//
//////////////////////////////////////////////////////////////////////


#ifndef _CNP_H_
#define _CNP_H_

#define CNP_VERSION								0x02

// CCP = Crytek Control protocol 
#define FT_CCP_SETUP							0x10
#define FT_CCP_CONNECT						0x20
#define FT_CCP_CONNECT_RESP				0x30
#define FT_CCP_DISCONNECT					0x40
#define FT_CCP_CONTEXT_SETUP			0x50
#define FT_CCP_CONTEXT_READY			0x60
#define FT_CCP_SERVER_READY				0x70
#define FT_CCP_ACK								0x80
#define FT_CCP_SECURITY_QUERY			0x90
#define FT_CCP_SECURITY_RESP			0x91
#define FT_CCP_PUNK_BUSTER_MSG		0x92

// CQP = Crytek Transport protocol 
#define FT_CTP_DATA								0x01
#define FT_CTP_ACK								0x02
#define FT_CTP_NAK								0x03
#define FT_CTP_PONG								0x04

// CQP = Crytek Query protocol (serverlist queries, RCon system for server remote control)
#define FT_CQP_INFO_REQUEST				0xff			// ask server about it's settings (for serverlist) or XML request.
#define FT_CQP_INFO_RESPONSE			0xff			// respond server settings (for serverlist) or XML response.

#define FT_CQP_XML_REQUEST				0x15			// XML request to server.
#define FT_CQP_XML_RESPONSE				0x16			// XML response from server.

#define FT_CQP_RCON_COMMAND				0x13			// execute remote a command on a server
#define FT_CQP_RCON_RESPONSE			0x14			// get the response back from the server


#define IS_TRANSPORT_FRAME(a)((a)&0x0F)?1:0
#define IS_CONTROL_FRAME(a)((a)&0xF0)?1:0

#define MAX_REQUEST_XML_LENGTH 1024

// Available Client flags
enum EClientFlags
{
	// This client running 64 bit version.
	CLIENT_FLAGS_64BIT				= (1 << 0),
	// This client running with PunkBuster enabled.
	CLIENT_FLAGS_PUNK_BUSTER	= (1 << 1),
	// This client is/has cheated
	CLIENT_FLAGS_DEVMODE			= (1 << 3),
};

///////////////////////////////////////////////////////////////
struct CNPServerVariables
{
	void Save(CStream &stm)
	{
		stm.Write(nPublicKey);
		stm.Write(nDataStreamTimeout);
	}
	void Load(CStream &stm)
	{
		stm.Read(nPublicKey);
		stm.Read(nDataStreamTimeout);
	}

	unsigned int nDataStreamTimeout;			//!< time elapsed without any data packet
	unsigned int nPublicKey;
};
///////////////////////////////////////////////////////////////
struct CNP
{
	CNP()
	{
		//m_cClientID = 0;
		m_cFrameType = 0;
		m_bSecondaryTC=false;
	}
	virtual ~CNP(){}
	//BYTE m_cClientID;
	BYTE m_cFrameType;
	bool m_bSecondaryTC;
	void Save(CStream &stm)
	{
		stm.WritePkd(m_cFrameType);
		stm.Write(m_bSecondaryTC);
	}
	void Load(CStream &stm)
	{
		stm.ReadPkd(m_cFrameType);
		stm.Read(m_bSecondaryTC);
	}
	void LoadAndSeekToZero(CStream &stm)
	{
		CNP::Load(stm);
		//stm.Read(m_cFrameType);
		stm.Seek(0);
	}
};
///////////////////////////////////////////////////////////////
struct CTP :public CNP
{
	CTP()
	{
		m_cSequenceNumber = 0;
		m_cAck = 0;
		m_bPingRequest = false;
		m_bUnreliable = false;
	}
	virtual ~CTP(){}
	BYTE m_cSequenceNumber;//<<FIXME>> will be 6 bits
	BYTE m_cAck;//<<FIXME>> will be 6 bits
	bool m_bPingRequest;
	bool m_bUnreliable;
	virtual void Save(CStream &stm)
	{
		CNP::Save(stm);
		stm.WritePkd(m_cSequenceNumber);
		stm.WritePkd(m_cAck);
		stm.Write(m_bPingRequest);
		stm.Write(m_bUnreliable);
	}
	virtual void Load(CStream &stm)
	{
		CNP::Load(stm);
		stm.ReadPkd(m_cSequenceNumber);
		stm.ReadPkd(m_cAck);
		stm.Read(m_bPingRequest);
		stm.Read(m_bUnreliable);
	}
};

///////////////////////////////////////////////////////////////
struct CCP :public CNP
{
	virtual ~CCP(){}
	virtual void Save(CStream &stm)
	{
		CNP::Save(stm);
	}
	virtual void Load(CStream &stm)
	{
		CNP::Load(stm);
	}
};
///////////////////////////////////////////////////////////////
// should be a byte aligned, so it can be manipulated by not-so-low level languages, like web languages (php, perl, ..)
// should always start with 4bytes 0x7f 0xff 0xff 0xff
// fist 10bits are CNP properties, wich are 0 11111111 1
// next 6bits are control bits, 111111
// and then two bytes 0xff 0xff
struct CQP :public CNP
{
	CQP()
	{
		m_bSecondaryTC = true;
		for (int i=0; i<6; i++)
			m_bControlBits[i] = 0;
		m_cControlBytes[0] = 0;
		m_cControlBytes[1] = 0;
	}
	virtual ~CQP(){}

	bool		m_bControlBits[6];				// control bits, must be all 1
	unsigned char m_cControlBytes[2];	// control bytes, must be all 0xff;

	virtual void Save(CStream &stm)
	{
		CNP::Save(stm);
		for(int i=0; i<6; i++) // write 6 control bits
			stm.Write(true);
		stm.Write((unsigned char)0xff); // write the two control bytes
		stm.Write((unsigned char)0xff);
	}

	virtual void Load(CStream &stm)
	{
		CNP::Load(stm);

		for(int i=0; i<6; i++)
		{
			if (stm.GetReadPos() < stm.GetSize())
				stm.Read(m_bControlBits[i]);
			else
				return;
		}
		if (stm.GetSize()-stm.GetReadPos() >= 16)
		{
			stm.Read(m_cControlBytes[0]);
			stm.Read(m_cControlBytes[1]);
		}
	}

	bool IsOk()
	{
		for (int i=0; i < 6; i++)
		{
			if (!m_bControlBits[i])
				return false;
		}
		return (m_cControlBytes[0] == 0xff) && (m_cControlBytes[0] == 0xff);
	}
};
///////////////////////////////////////////////////////////////
struct CQPInfoRequest: public CQP
{
	CQPInfoRequest() { m_cFrameType = FT_CQP_INFO_REQUEST; };
	CQPInfoRequest(const string &szQuery): szRequest(szQuery) { m_cFrameType = FT_CQP_INFO_REQUEST; };
	virtual ~CQPInfoRequest(){}

	string	szRequest;

	virtual void Save(CStream &stm)
	{
		CQP::Save(stm);
		// write every byte, because if we write the string, it will get compressed :(
		for (uint32 i=0; i<szRequest.size(); i++)
			stm.Write(szRequest[i]);
	}
	virtual void Load(CStream &stm)
	{
		szRequest.resize(0);

		CQP::Load(stm);

		while(stm.GetSize()-stm.GetReadPos() >= 8)
		{
			char cByte;
			stm.Read(cByte);
			szRequest.push_back(cByte);
		}
	}
};
///////////////////////////////////////////////////////////////
struct CQPInfoResponse :public CQP
{
	CQPInfoResponse()
	{
		m_cFrameType = FT_CQP_INFO_RESPONSE;
	}
	virtual ~CQPInfoResponse(){}
	string szResponse;
	virtual void Save(CStream &stm)
	{
		CQP::Save(stm);
		// write every byte, because if we write the string, it will get compressed :(
		for (uint32 i=0; i<szResponse.size(); i++)
			stm.Write(szResponse[i]);

	}
	virtual void Load(CStream &stm)
	{
		szResponse.resize(0);

		CQP::Load(stm);

		while(stm.GetSize()-stm.GetReadPos() >= 8)
		{
			char cByte;
			stm.Read(cByte);
			szResponse.push_back(cByte);
		}
	}
};

///////////////////////////////////////////////////////////////
struct CQPXMLRequest :public CQP
{
	CQPXMLRequest()
	{
		m_cFrameType = FT_CQP_XML_REQUEST;
	}
	virtual ~CQPXMLRequest(){}
	virtual void Save(CStream &stm)
	{
		CQP::Save(stm);
		if (m_sXML.size() > MAX_REQUEST_XML_LENGTH)
			m_sXML.resize(MAX_REQUEST_XML_LENGTH);
		stm.Write(m_sXML.c_str());
	}
	virtual void Load(CStream &stm)
	{
		CQP::Load(stm);
		static char sTemp[MAX_REQUEST_XML_LENGTH+1];
		stm.Read( (char*)sTemp,sizeof(sTemp)-1 );
		sTemp[sizeof(sTemp)-1] = 0;
		m_sXML = sTemp;
	}
	string m_sXML;
};
///////////////////////////////////////////////////////////////
struct CQPXMLResponse :public CQP
{
	CQPXMLResponse()
	{
		m_cFrameType = FT_CQP_XML_RESPONSE;
	}
	virtual ~CQPXMLResponse(){}
	virtual void Save(CStream &stm)
	{
		CQP::Save(stm);
		if (m_sXML.size() > MAX_REQUEST_XML_LENGTH)
			m_sXML.resize(MAX_REQUEST_XML_LENGTH);
		stm.Write(m_sXML.c_str());
	}
	virtual void Load(CStream &stm)
	{
		CQP::Load(stm);
		static char sTemp[MAX_REQUEST_XML_LENGTH+1];
		stm.Read( (char*)sTemp,sizeof(sTemp)-1 );
		sTemp[sizeof(sTemp)-1] = 0;
		m_sXML = sTemp;
	}
	string m_sXML;
};
///////////////////////////////////////////////////////////////
//! remote command to a server
struct CQPRConCommand :public CQP
{
	CQPRConCommand()
	{
		m_cFrameType = FT_CQP_RCON_COMMAND;
	}
	virtual ~CQPRConCommand(){}
	virtual void Save(CStream &stm)
	{
		CQP::Save(stm);
		stm.WriteData( &m_nRConPasswordCode,16 );
		stm.Write(m_sRConCommand.c_str());
	}
	virtual void Load(CStream &stm)
	{
		static char sTemp[512];
		CQP::Load(stm);
		
		stm.ReadData( &m_nRConPasswordCode,16 );
		stm.Read((char *)sTemp,sizeof(sTemp)-1 );
		sTemp[sizeof(sTemp)-1] = 0;
		m_sRConCommand = sTemp;
	}

	unsigned int m_nRConPasswordCode[4];
	string m_sRConCommand;			//!<
};
///////////////////////////////////////////////////////////////
//! response from a server on a remote command
struct CQPRConResponse :public CQP
{
	CQPRConResponse()
	{
		m_cFrameType = FT_CQP_RCON_RESPONSE;
	}
	virtual ~CQPRConResponse(){}
	virtual void Save(CStream &stm)
	{
		CQP::Save(stm);
		stm.Write(m_sText.c_str());
	}
	virtual void Load(CStream &stm)
	{
		static char sTemp[512];
		CQP::Load(stm);
		
		stm.Read((char *)sTemp,512);
		m_sText = sTemp;
	}
	string				m_sText;				//!<
};
///////////////////////////////////////////////////////////////
struct CCPPayload :public CCP
{
	CCPPayload(){
		m_bSequenceNumber = 0;
	}
	virtual ~CCPPayload(){}
	bool m_bSequenceNumber;
	virtual void Save(CStream &stm)
	{
		CCP::Save(stm);
		stm.Write(m_bSequenceNumber);
	}
	virtual void Load(CStream &stm)
	{
		CCP::Load(stm);
		stm.Read(m_bSequenceNumber);
	}
};
///////////////////////////////////////////////////////////////
struct CCPAck :public CCP
{
	bool m_bAck;
	CCPAck()
	{
		m_cFrameType = FT_CCP_ACK;
	}
	virtual ~CCPAck(){}
	virtual void Save(CStream &stm)
	{
		CCP::Save(stm);
		stm.Write(m_bAck);
	}
	virtual void Load(CStream &stm)
	{
		CCP::Load(stm);
		stm.Read(m_bAck);
	}
};
///////////////////////////////////////////////////////////////
struct CCPSetup :public CCPPayload
{
	CCPSetup()
	{
		m_cFrameType = FT_CCP_SETUP;
		m_cProtocolVersion = CNP_VERSION;
		m_nClientFlags = 0;
		m_nClientOSMinor = 0;
		m_nClientOSMajor = 0;
	}
	virtual ~CCPSetup(){}
	BYTE m_cProtocolVersion;
	string m_sPlayerPassword;
	// What version client is running.
	unsigned int m_nClientFlags;
	// What OS client is running.
	unsigned int m_nClientOSMinor;
	unsigned int m_nClientOSMajor;
	//string m_sSpectatorPassword;
	virtual void Save(CStream &stm)
	{
		CCPPayload::Save(stm);
		stm.Write(m_cProtocolVersion);
		stm.Write(m_sPlayerPassword.c_str());
		stm.Write(m_nClientFlags);
		stm.Write(m_nClientOSMinor);
		stm.Write(m_nClientOSMajor);
		//stm.Write(m_sSpectatorPassword.c_str());
	}
	virtual void Load(CStream &stm)
	{
		static char sTemp[512];
		CCPPayload::Load(stm);
		stm.Read(m_cProtocolVersion);
		stm.Read((char *)sTemp,512);
		m_sPlayerPassword = sTemp;
		stm.Read(m_nClientFlags);
		stm.Read(m_nClientOSMinor);
		stm.Read(m_nClientOSMajor);
		//stm.Read((char *)sTemp);
		//m_sSpectatorPassword = sTemp;
	}
};
///////////////////////////////////////////////////////////////
struct CCPConnect :public CCPPayload
{
	CCPConnect()
	{
		m_cFrameType = FT_CCP_CONNECT;
		m_cResponse = 0;
		//m_cNewClientID = 0;
	}
	virtual ~CCPConnect(){}
	BYTE m_cResponse;		//!< bitflag to specify early information about the server! i.e. punkbuster or not..
	// Random part of key used for encryption.
	//BYTE m_cNewClientID;
	// protocol variables
	CNPServerVariables m_ServerVariables;
	virtual void Save(CStream &stm)
	{
		CCPPayload::Save(stm);
		stm.Write(m_cResponse);
		//stm.Write(m_cNewClientID);
		// protocol variables
		m_ServerVariables.Save(stm);
	}
	virtual void Load(CStream &stm)
	{
		CCPPayload::Load(stm);
		stm.Read(m_cResponse);
		//stm.Read(m_cNewClientID);
		// protocol variables
		m_ServerVariables.Load(stm);
	}
};
///////////////////////////////////////////////////////////////
struct CCPConnectResp :public CCPPayload
{
	CCPConnectResp()
	{
		m_cFrameType = FT_CCP_CONNECT_RESP;
		m_cResponse = 0;
	}
	virtual ~CCPConnectResp(){}
	BYTE m_cResponse;
	CStream m_stmAuthorizationID;
	virtual void Save(CStream &stm)
	{
		unsigned int uiSize;

		CCPPayload::Save(stm);
		stm.Write(m_cResponse);

		uiSize = m_stmAuthorizationID.GetSize();
		stm.Write(uiSize);
		stm.Write(m_stmAuthorizationID);
	}
	virtual void Load(CStream &stm)
	{
		unsigned int uiSize;
		CCPPayload::Load(stm);
		stm.Read(m_cResponse);
		stm.Read(uiSize);
		m_stmAuthorizationID.Reset();
		m_stmAuthorizationID.SetSize(uiSize);
		stm.ReadBits(m_stmAuthorizationID.GetPtr(),uiSize);
		//stm.Read(m_stmAuthorizationID);		
	}
};
///////////////////////////////////////////////////////////////
struct CCPContextSetup:public CCPPayload
{
	CCPContextSetup()
	{
		m_cFrameType = FT_CCP_CONTEXT_SETUP;
	}
	virtual ~CCPContextSetup(){}
	CStream m_stmData;
	virtual void Save(CStream &stm)
	{
		unsigned short usSize;
		usSize = (unsigned short)m_stmData.GetSize();
		CCPPayload::Save(stm);
		stm.Write(usSize);
		stm.Write(m_stmData);
	}
	virtual void Load(CStream &stm)
	{
		unsigned short usSize;
		CCPPayload::Load(stm);
		stm.Read(usSize);
		m_stmData.Reset();
		m_stmData.SetSize(usSize);
		stm.ReadBits(m_stmData.GetPtr(), usSize);
	}
};
///////////////////////////////////////////////////////////////
struct CCPContextReady :public CCPPayload
{
	CCPContextReady()
	{
		m_cFrameType = FT_CCP_CONTEXT_READY;
	}
	virtual ~CCPContextReady(){}
	CStream m_stmData;
	virtual void Save(CStream &stm)
	{
		unsigned short usSize;
		usSize = (unsigned short)m_stmData.GetSize();
		CCPPayload::Save(stm);
		stm.Write(usSize);
		stm.Write(m_stmData);
	}
	virtual void Load(CStream &stm)
	{
		unsigned short usSize;
		CCPPayload::Load(stm);
		stm.Read(usSize);
		m_stmData.Reset();
		m_stmData.SetSize(usSize);
		stm.ReadBits(m_stmData.GetPtr(), usSize);
	};
};
///////////////////////////////////////////////////////////////
struct CCPServerReady :public CCPPayload
{
	CCPServerReady()
	{
		m_cFrameType = FT_CCP_SERVER_READY;
	}
	virtual ~CCPServerReady(){}
	virtual void Save(CStream &stm)
	{
		CCPPayload::Save(stm);
	}
	virtual void Load(CStream &stm)
	{
		CCPPayload::Load(stm);
	}
};
///////////////////////////////////////////////////////////////
struct CCPDisconnect :public CCPPayload
{
	CCPDisconnect()
	{
		m_cFrameType = FT_CCP_DISCONNECT;
	}
	virtual ~CCPDisconnect(){}
	string m_sCause;
	virtual void Save(CStream &stm)
	{
		CCPPayload::Save(stm);
		stm.Write(m_sCause);
	}
	virtual void Load(CStream &stm)
	{
		CCPPayload::Load(stm);
		stm.Read(m_sCause);
	}
};
///////////////////////////////////////////////////////////////
struct CCPSecurityQuery :public CCPPayload
{
	CCPSecurityQuery()
	{
		m_cFrameType = FT_CCP_SECURITY_QUERY;
	}
	virtual ~CCPSecurityQuery(){}
	CStream m_stmData;
	virtual void Save(CStream &stm)
	{
		unsigned short usSize;
		usSize = (unsigned short)m_stmData.GetSize();
		CCPPayload::Save(stm);
		stm.Write(usSize);
		stm.Write(m_stmData);
	}
	virtual void Load(CStream &stm)
	{
		unsigned short usSize;
		CCPPayload::Load(stm);
		stm.Read(usSize);
		m_stmData.Reset();
		m_stmData.SetSize(usSize);
		stm.ReadBits(m_stmData.GetPtr(), usSize);
	};
};
///////////////////////////////////////////////////////////////
struct CCPSecurityResp :public CCPSecurityQuery
{
	CCPSecurityResp()
	{
		m_cFrameType = FT_CCP_SECURITY_RESP;
	}
};

///////////////////////////////////////////////////////////////
struct CCPPunkBusterMsg :public CCPSecurityQuery
{
	CCPPunkBusterMsg()
	{
		m_cFrameType = FT_CCP_PUNK_BUSTER_MSG;
	}
};

///////////////////////////////////////////////////////////////
struct CTPData :public CTP
{
	CTPData()
	{
		m_cFrameType = FT_CTP_DATA;
		m_bCompressed = false;
		m_nUncompressedSize = 0;
	}
	virtual ~CTPData(){}
	CStream m_stmData;
	bool m_bCompressed; // Compressed data.
	unsigned short m_nUncompressedSize;
	virtual void Save(CStream &stm)
	{
		unsigned short usSize;
		usSize = (unsigned short)m_stmData.GetSize();
		CTP::Save(stm);
		stm.Write(m_bCompressed);
		if (m_bCompressed)
			stm.WritePkd(m_nUncompressedSize);
		stm.WritePkd(usSize);
		stm.Write(m_stmData);
	}
	virtual void Load(CStream &stm)
	{
		unsigned short usSize;
		CTP::Load(stm);
		stm.Read(m_bCompressed);
		if (m_bCompressed)
			stm.ReadPkd(m_nUncompressedSize);
		stm.ReadPkd(usSize);
		m_stmData.Reset();
		m_stmData.SetSize(usSize);
		stm.ReadBits(m_stmData.GetPtr(), usSize);
	}
};
///////////////////////////////////////////////////////////////
struct CTPNak :public CTP
{
	CTPNak()
	{
		m_cFrameType = FT_CTP_NAK;
	}
	virtual ~CTPNak(){}
	virtual void Save(CStream &stm)
	{
		CTP::Save(stm);
	}
	virtual void Load(CStream &stm)
	{
		CTP::Load(stm);
	}
};
///////////////////////////////////////////////////////////////
struct CTPAck :public CTP
{
	CTPAck()
	{
		m_cFrameType = FT_CTP_ACK;
	}
	virtual ~CTPAck(){}
	virtual void Save(CStream &stm)
	{
		CTP::Save(stm);
	}
	virtual void Load(CStream &stm)
	{
		CTP::Load(stm);
	}
};


// this packet is unreliable(sequence number and ack are ignored)
struct CTPPong :public CTP
{
	CTPPong()
	{
		m_cFrameType = FT_CTP_PONG;
		m_nTimestamp = 0;
	}
	virtual ~CTPPong(){}
	unsigned int m_nTimestamp;
	virtual void Save(CStream &stm)
	{
		CTP::Save(stm);
		stm.Write(m_nTimestamp);
	}
	virtual void Load(CStream &stm)
	{
		CTP::Load(stm);
		stm.Read(m_nTimestamp);
	}
};
///////////////////////////////////////////////////////////////
#endif //_CNP_H_
