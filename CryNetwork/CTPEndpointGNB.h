#ifndef _CTP_ENDPOINT_GNB_
#define _CTP_ENDPOINT_GNB_

#include "Stream.h"
#include "CNP.h"
#include "Interfaces.h"
#include "PingCalculator.h"
#include "Network.h"

#define MODULO32(a) ((a)&0x1f)
#define MAX_SEQ_NUM 31
#define NUM_OF_BUFS ((MAX_SEQ_NUM+1)/2)
#define INC(a) (a)=MODULO32((++a))

#define MAX_QUEUED_PACKET_PER_CHANNEL 24

//<<FIXME>> buffer timeout...replace with a ping adaptive algorithm
#define TM_BUFFER 600
#define TM_ACK	TM_BUFFER/5
#define MAX_PLAYLOAD_SIZE_IN_BITS 1024*8

#include <queue>

typedef struct tagBuffer{
	CStream stmData;
	DWORD dwTimeout;
	LONG nSeq;//only for retrasmission
}Buffer;

typedef std::queue<CStream> STREAM_QUEUE;

class CCTPEndpointGNB
{
public:
	CCTPEndpointGNB( CNetwork *pNetwork );
	virtual ~CCTPEndpointGNB(void);
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
	void GetMemoryStatistics(ICrySizer *pSizer);
  //
	unsigned int GetPing();
	unsigned int GetLostPackets(){return m_nLostPackets;}
	unsigned int GetUnreliableLostPackets(){return m_nUnreliableLostPackets;}


	void EnableCompression(bool bEnable){m_bCompress=bEnable;}
	// Changes crypt key specifically for this connection.
	void SetPublicCryptKey( unsigned int key );

protected:
	virtual void BuildOutgoingFrame();
	virtual bool IsTimeToSend();
	
private:
//	CStream CompressStream(CStream &stmUncompressed);
//	CStream UncompressStream(CStream &stmCompressed);
	void ProcessBufferTimers();
	void ProcessAckTimer();
	void HandleAckTimeout();
	void HandleDataFrame(CTPData &f);
	void HandleTimeout(LONG nOldestFrame);
	void SendFrame(LONG nType,LONG nFrameNum,LONG nFrameExpected,CStream *pUnreliable = NULL,bool bUnreliable=false);
	void StopTimer(LONG nIndex);
	void SetTimer(LONG nIndex);
	void StopAckTimer();
	void SetAckTimer();

	void CryptPacket( CTPData &data );
	void UncryptPacket( CTPData &data );

//////////////////////////////////////
	CNetwork *m_pNetwork;
	_IEndpointUser *m_pParent;
	bool m_bSecondary;

	Buffer m_OutBuffers[NUM_OF_BUFS];
	STREAM_QUEUE m_qOutgoingReliableData;
	STREAM_QUEUE m_qOutgoingUnreliableData;
	//protocol
	DWORD m_nFrameExpected;
	DWORD m_nNextFrameToSend;
	DWORD m_nAckExpected;
	DWORD m_nBuffered;	//number of output buffers currently used	
	DWORD m_dwOutAckTimer;
	DWORD m_dwPingTime;
	DWORD m_nAllowedBytes;
	DWORD m_nLastPacketSent;
  DWORD m_nLostPackets;
	DWORD m_nUnreliableLostPackets;
	unsigned int m_nCurrentTime;
	CPingCalculator m_LatencyCalculator;
	bool m_bCompress;
	// Encryption key used in Transfer protocol.
	unsigned int					m_nEncryptKey[4];
};

#endif //_CTP_ENDPOINT_GNB_
