#ifndef RCONSYSTEM_H
#define RCONSYSTEM_H

#include "INetwork.h"				// IRConSystem
#include <list>

class CRConSystem :public IRConSystem, public INetworkPacketSink
{
public:
	//! constructor
	CRConSystem();
	//! destructor
	virtual ~CRConSystem();

	// interface INetworkPacketSink ------------------------------------------

	virtual void OnReceivingPacket( const unsigned char inPacketID, CStream &stmPacket, CIPAddress &ip );

	// interface IRConSystem -------------------------------------------------

	virtual void Release(){ delete this; }
	virtual void Update( unsigned int dwTime,IClient *pClient=NULL );
	virtual void ExecuteRConCommand( const char *inszCommand );
	virtual void OnServerCreated( IServer *inpServer );

	// -----------------------------------------------------------------------

	//!
	bool Create( ISystem *pSystem );
	
private: 	// -----------------------------------------------------------------------

	struct SDeferredCommand
	{
		//! constructor
		SDeferredCommand( const string &sCmd, const CIPAddress &ip ) :m_sCommand(sCmd), m_ip(ip)
		{
		}

		string			m_sCommand;		//!<
		CIPAddress 	m_ip;					//!<
	};

 	ISystem *												m_pSystem;										//!< pointer to the system interface (is zero when not initialized)
	IServer	*												m_pIServer;										//!< 
	CDatagramSocket									m_sSocket;										//!<
	std::map<unsigned int, int>			m_hmRconAttempts;							//! hash map that maps ip -> rcon attempts, for security.
	std::list<SDeferredCommand>			m_DeferredConsoleCommands;		//!< to execute commands at a defined point in the update loop
	unsigned int										m_nDevPassCode[4];
	CIPAddress											m_ipServer;

	//! Get 128bit code from string. (4 ints)
	void GetPassCode( const char *szString,unsigned int *nOutCode );

	friend class CRConConsoleSink;
};

#endif // RCONSYSTEM_H