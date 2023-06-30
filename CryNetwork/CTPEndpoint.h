//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: CTPEndpoint.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_CTPENDPOINT_H__F28737F3_12B9_4394_B63C_7FC8CFFF3033__INCLUDED_)
#define AFX_CTPENDPOINT_H__F28737F3_12B9_4394_B63C_7FC8CFFF3033__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <queue>

#define MODULO32(a) ((a)&0x1f)
#define MAX_SEQ_NUM 7
#define NUM_OF_BUFS ((MAX_SEQ_NUM+1)/2)
#define INC(a) (a)=MODULO32((++a))

#define MAX_QUEUED_PACKET_PER_CHANNEL 24

//<<FIXME>> buffer timeout...replace with a ping adaptive algorithm
#define TM_BUFFER 300
#define TM_ACK	TM_BUFFER/5
#define MAX_PLAYLOAD_SIZE_IN_BITS 1024*8

#include "Stream.h"
#include "CNP.h"
#include "Interfaces.h"
#include "PingCalculator.h"

typedef std::queue<CStream> STREAM_QUEUE;

typedef struct tagBuffer{
	CStream stmData;
	DWORD dwTimeout;
	LONG nSeq;//only for retrasmission
}Buffer;


/*! Implements a CTP endpoint
*/
class CCTPEndpoint  
{
public:
	CCTPEndpoint();
	virtual ~CCTPEndpoint();
public:
	void Reset();
	void Init(_IEndpointUser *pParent,bool bSecondary){m_pParent=pParent;m_bSecondary=bSecondary;}
	bool SendUnreliable(CStream &stmData);
	bool SendReliable(CStream &stmData);
	void Update(unsigned int nTime,unsigned char cFrameType = 0,CStream *pStm=NULL);
	void SetRate(unsigned int nBytePerSec);
	void Dump();
	unsigned int GetRemoteTimestamp(unsigned int nTime){
		return m_LatencyCalculator.GetCurrentRemoteTimestamp(nTime);
	}
	//
	unsigned int GetPing();
	unsigned int GetLostPackets(){return m_nLostPackets;}
	// Changes crypt key specifically for this connection.
	void SetPublicCryptKey( unsigned int key );
protected:
	virtual void BuildOutgoingFrame()=0;
	virtual bool IsTimeToSend()=0;

	void ProcessBufferTimers();
	void ProcessAckTimer();
	void HandleAckTimeout();
	void HandleDataFrame(CTPData &f);
	void HandleNak(CTPNak &f);
	void HandleTimeout(LONG nOldestFrame);
	void SendFrame(LONG nType,LONG nFrameNum,LONG nFrameExpected,CStream *pUnreliable = NULL,bool bUnreliable=false);
	void StopTimer(LONG nIndex);
	void SetTimer(LONG nIndex);
	void StopAckTimer();
	void SetAckTimer();

	void CryptStream( CStream &stm );
	void UncyptStream( CStream &stm );

//	CStream CompressStream(CStream &stm);
//	CStream UncompressStream(CStream &stm);

	// ............................................................

	bool									m_nArrived[NUM_OF_BUFS];
	LONG									m_nFrameExpected;
	LONG									m_nNextFrameToSend;
	LONG									m_nTooFar;
	LONG									m_nAckExpected;
	bool									m_bNoNak;											//!< only one nak per frame
	LONG									m_nBuffered;									//!< number of output buffers currently used
	CStream								m_stmInBuffers[NUM_OF_BUFS];
	Buffer								m_OutBuffers[NUM_OF_BUFS];
	LONG									m_nOldestFrame;
//TIMERS=0 mean disabled
	//DWORD m_dwTimers[NUM_OF_BUFS]; //expected ack timers
	DWORD									m_dwOutAckTimer;							//!< outgoing ack timer
	STREAM_QUEUE					m_qOutgoingReliableData;
	STREAM_QUEUE					m_qOutgoingUnreliableData;

	_IEndpointUser *			m_pParent;

	bool									m_bSecondary;
	class CPingCalculator m_LatencyCalculator;					//!< Ping calculation stuff
	DWORD									m_nLostPackets;
	DWORD									m_dwPingTime;									//!< store the timestamp of the latest pong request
	//<<FIXME>> implement the rate control
	unsigned int					m_nRate; 
	float									m_fBytePerMillisec;						//!< only used when RATE_CONTROL is defined
	int									m_nAllowedBytes;
	unsigned int					m_nLastPacketSent;
protected:
	unsigned int					m_nSnaps;
	unsigned int					m_nCurrentTime;
	// Encryption key used in Transfer protocol.
	unsigned int					m_nEncryptKey[4];
};

class CCTPClient :public CCTPEndpoint
{
protected:
//	virtual void BuildOutgoingFrame();
	virtual bool IsTimeToSend();
};

class CCTPServer :public CCTPEndpoint
{
public:
	CCTPServer()
	{
	}
protected:
	
//	virtual void BuildOutgoingFrame();
	virtual bool IsTimeToSend();

};

#endif // !defined(AFX_CTPENDPOINT_H__F28737F3_12B9_4394_B63C_7FC8CFFF3033__INCLUDED_)
