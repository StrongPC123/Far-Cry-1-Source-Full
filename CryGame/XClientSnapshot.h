//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XClientSnapshot.h
//  Description: Snapshot manager class.
//
//  History:
//  - August 14, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XCLIENTSNAPSHOT_H
#define GAME_XCLIENTSNAPSHOT_H

//////////////////////////////////////////////////////////////////////////////////////////////
class CXClientSnapshot
{
public:
	//! constructor
	CXClientSnapshot();
	//! destructor
	~CXClientSnapshot();

	bool IsTimeToSend(float fFrameTimeInSec);

	void Reset();

	void SetSendPerSecond(BYTE cSendPerSecond); // 0 mean 25 per second

	inline BYTE GetSendPerSecond() { return m_cSendPerSecond; }

	inline CStream& GetReliableStream() { return m_ReliableStream; }

	inline CStream& GetUnreliableStream() { return m_UnreliableStream; }

private: // ------------------------------------------------------------------------------

	BYTE						m_cSendPerSecond;			//!< can be changed with cl_cmdrate
	unsigned int		m_nTimer;							//!< in ms
	CStream					m_ReliableStream;			//!<
	CStream					m_UnreliableStream;		//!<
	ICVar *					sv_maxcmdrate;				//!< is limiting the commandrate of the clients
};

#endif // GAME_XCLIENTSNAPSHOT_H
