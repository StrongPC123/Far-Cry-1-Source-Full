//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: CCPEndpoint.h
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef _CCPENDPOINT_H_
#define _CCPENDPOINT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Stream.h"
#include "CNP.h"
#include "Interfaces.h"
#include <queue>

#define INC_BOOL(xxx) xxx=!(xxx)
#define TM_BUFFER_TIMER 1000

typedef std::queue<CCPPayload *> CCP_QUEUE;

class CCCPEndpoint   
{
public:

	//! constructor
	CCCPEndpoint();
	//! destructor
	virtual ~CCCPEndpoint();

	//!
	void Init( _ICCPUser *pParent );

	void SendDisconnect(const char* szCause);
	void SendConnectResp(CStream &stm);
	void SendContextSetup(CStream &stm);
	void SendContextReady(CStream &stm);
	void SendServerReady();
	void SendConnect(CNPServerVariables &sv);
	void SendSetup();
	void SendPunkBusterMsg(CStream &stm);

	void SendSecurityQuery(CStream &stm);
	void SendSecurityResp(CStream &stm);
	void SetPublicCryptKey( unsigned int nKey ) { m_nPublicKey = nKey; };
	unsigned int GetPublicCryptKey( unsigned int nKey ) { return m_nPublicKey; };

	//! @return true=disconnect and this pointer is destrozed, false otherwise
	bool Update(unsigned int nTime,unsigned char cFrameType,CStream *pStm);
	void Reset();

	void GetMemoryStatistics(ICrySizer *pSizer);
protected:

	//! @return true=disconnect and this pointer is destrozed, false otherwise
	bool ProcessPayload(unsigned char cFrameType,CStream &stmStrea);
	void HandleTimeout();
	void ProcessTimers();
	void SetTimer();
	void StopTimer();
	void SendFrame();
	void EnableSend();
	void DisableSend();
	void SendAck(bool bSequenceNumber);
	bool IsTimeToSend();

private:		// --------------------------------------------------------------

	bool							m_bFrameExpected;						//!<
	bool							m_bNextFrameToSend;					//!<
	bool							m_bAckExpected;							//!<
	bool							m_bReadyToSend;							//!<
	unsigned int			m_ulTimeout;								//!<

	_ICCPUser *				m_pParent;									//!< pointer to the parent object (is not released), is 0 is you forgot to call Init()

	CCP_QUEUE					m_qOutgoingData;						//!<
	CStream						m_stmRetrasmissionBuffer;		//!<

	unsigned int			m_nCurrentTime;							//!< 
	unsigned int			m_nPublicKey;
};

#endif //_CCPENDPOINT_H_
