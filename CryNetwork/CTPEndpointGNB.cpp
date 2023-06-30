#include "stdafx.h"
#include "ctpendpointgnb.h"
#include <IDataProbe.h>

#define PACKET_SIZE_COMPRESSION_LIMIT 100


CCTPEndpointGNB::CCTPEndpointGNB( CNetwork *pNetwork )
{
	m_pNetwork = pNetwork;
	Reset();

	m_bCompress = true;
	m_nEncryptKey[0] = 1282732178u;
	m_nEncryptKey[1] = 1718763272u;
	m_nEncryptKey[2] = 297614432u;
	m_nEncryptKey[3] = 3389628651u;
}

CCTPEndpointGNB::~CCTPEndpointGNB(void) 
{
}

void CCTPEndpointGNB::SetPublicCryptKey( unsigned int key )
{
	m_nEncryptKey[2] = key;
}

static bool Between(LONG a, LONG b, LONG c) 
{
	return ((a <= b) &&(b < c)) ||((c < a) &&(a <= b)) ||((b < c) &&(c < a));
}

void CCTPEndpointGNB::Reset()
{
	m_nFrameExpected=0;
	m_nNextFrameToSend=0;
	m_nAckExpected=0;
	m_nBuffered=0;	//number of output buffers currently used
	m_nCurrentTime=0;
	m_dwOutAckTimer=0;
	m_dwPingTime=0;
	m_nLostPackets=0;
	m_nUnreliableLostPackets=0;
	m_nBuffered=0;
	for (long n = 0; n < NUM_OF_BUFS; n++)
	{
		m_OutBuffers[n].dwTimeout = 0;
		m_OutBuffers[n].nSeq = 0;
//		m_nArrived[n]=false;
	}
	
	while (!m_qOutgoingReliableData.empty())
	{
		m_qOutgoingReliableData.pop();
	}
	while (!m_qOutgoingUnreliableData.empty())
	{
		m_qOutgoingUnreliableData.pop();
	}
}

//////////////////////////////////////////////////////////////////////////
void CCTPEndpointGNB::CryptPacket( CTPData &data )
{
	// Write 1 bit of compressed packed info.
	data.m_bCompressed = false;
	CStream &stm = data.m_stmData;
	unsigned int* pBuffer = (unsigned int*)stm.GetPtr();
	unsigned int srclen = (stm.GetSize()+7)/8; // Always cover last byte.
	unsigned int destlen = srclen;
	// Try to compress big packets.
	if (srclen > PACKET_SIZE_COMPRESSION_LIMIT && m_pNetwork->IsPacketCompressionEnabled())
	{
		BYTE temp[DEFAULT_STREAM_BYTESIZE*2];
		destlen = sizeof(temp);
		IDataProbe *pProbe = GetISystem()->GetIDataProbe();
		pProbe->Compress( temp,destlen,pBuffer,srclen,1 );
		if (destlen < srclen)
		{
			data.m_bCompressed = true;
			data.m_nUncompressedSize = stm.GetSize(); // In bits.
			TEA_ENCODE( (unsigned int*)temp,(unsigned int*)temp,TEA_GETSIZE(destlen),m_nEncryptKey );
			stm.Reset();
			stm.WriteBits(temp,destlen*8);
		}
	}
	if (data.m_bCompressed)
	{
		//@TOOD: remove log.
		float f1 = 100.0f/srclen;
		float f2 = f1 * destlen;
		int prc = (int)(100 - f2);
		//if (m_pNetwork->GetLogLevel() > 1)
			//CryLog( "<NET> PckSize Compressed: Was:%.3d Now:%.3d, Win: %.2d%%",srclen,destlen,prc );
	}
	else
	{
		int len = stm.GetSize()/8;
		if (len >= 8)
		{
			TEA_ENCODE( pBuffer,pBuffer,TEA_GETSIZE(len),m_nEncryptKey );
		}
	} 
}

//////////////////////////////////////////////////////////////////////////
void CCTPEndpointGNB::UncryptPacket( CTPData &data )
{
	CStream &stm = data.m_stmData;
	if (data.m_bCompressed)
	{
		// Compressed data packet.
		BYTE temp[DEFAULT_STREAM_BYTESIZE*2];
		unsigned int* pBuffer = (unsigned int*)stm.GetPtr();
		int srclen = (stm.GetSize()+7)/8; // Always cover last byte.
		TEA_DECODE( pBuffer,pBuffer,TEA_GETSIZE(srclen),m_nEncryptKey );
		unsigned int destLen = sizeof(temp);
		IDataProbe *pProbe = GetISystem()->GetIDataProbe();
		pProbe->Uncompress( temp,destLen,pBuffer,srclen );

		stm.Reset();
		stm.WriteBits( temp,data.m_nUncompressedSize );

	}
	else
	{
		// Uncompressed data packet.
		unsigned int* pBuffer = (unsigned int*)stm.GetPtr();
		int len = stm.GetSize()/8;
		if (len >= 8)
		{
			TEA_DECODE( pBuffer,pBuffer,TEA_GETSIZE(len),m_nEncryptKey );
		}
	}
}


bool CCTPEndpointGNB::SendUnreliable(CStream &stmData)
{
	m_qOutgoingUnreliableData.push(stmData);
	return true;
}

bool CCTPEndpointGNB::SendReliable(CStream &stmData)
{
	m_qOutgoingReliableData.push(stmData);
	return true;
}

void CCTPEndpointGNB::Update(unsigned int nTime,unsigned char cFrameType,CStream *pStm)
{
	m_nCurrentTime = nTime;
	CTP *pFrame = NULL;
	CTPAck ack;
	CTPData data;
	CTPPong pong;
	if (cFrameType)
	{
		switch (cFrameType)
		{
		case FT_CTP_DATA:
			{
				data.Load(*pStm);
				pFrame = &data;
				HandleDataFrame(data);
			}
			break;
		case FT_CTP_ACK:
			{
				pFrame = &ack;
				ack.Load(*pStm);
			}
			break;
		case FT_CTP_PONG:
			{
				pong.Load(*pStm);
				m_LatencyCalculator.AddSample((float)m_nCurrentTime - m_dwPingTime, m_nCurrentTime, pong.m_nTimestamp);
			}
			break;
		default:
			NET_ASSERT(0);
			break;
		}
		///////////////////////////////////////////////////////////
		if (pFrame)
		{
			if (pFrame->m_bPingRequest == true)
			{
				CStream stm;
				CTPPong pong;
				//					pong.m_cClientID = m_pParent->GetID();
				pong.m_nTimestamp = m_nCurrentTime;
				pong.Save(stm);
				m_pParent->Send(stm);
			}
			///////////////////////////////////////////////////////////

			// manage piggybacked ack (all frames hold a piggybacked ack)

			NET_TRACE("--->>>[CTP] SEQ %02d ACK %02d\n", pFrame->m_cSequenceNumber, pFrame->m_cAck);

			if(!pFrame->m_bUnreliable)
				while (Between(m_nAckExpected, pFrame->m_cAck, m_nNextFrameToSend))
				{	
					m_nBuffered = m_nBuffered - 1;
					NET_TRACE("Ack [%02d] STOPPING TIMER m_nBuffered=%02d\n",pFrame->m_cAck,m_nBuffered);
					StopTimer(m_nAckExpected%NUM_OF_BUFS);
					INC(m_nAckExpected);
				}
		}
	}
	// handle outgoing buffer timers
	ProcessBufferTimers();

	// if there is some out-buffer free, I retrive some new data to send
	// and if the network layer is ready(enough bandwith) ...send outgoing frames
	if (IsTimeToSend())
	{
		BuildOutgoingFrame();	
	}
	// handle ack timer
	ProcessAckTimer();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCTPEndpointGNB::SetRate(unsigned int nBytePerSec)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCTPEndpointGNB::Dump()
{
	NET_TRACE("m_nFrameExpected=%d |||", m_nFrameExpected);
	NET_TRACE("m_nBuffered=%d |||", m_nBuffered);
	NET_TRACE("m_nAckExpected=%d |||", m_nAckExpected);
	NET_TRACE("m_nNextFrameToSend=%d\n", m_nNextFrameToSend);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int CCTPEndpointGNB::GetPing()
{
	return (unsigned int)m_LatencyCalculator.GetAverageLatency();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCTPEndpointGNB::HandleDataFrame(CTPData &f)
{
	CStream stmUncompressed;
	if(f.m_bUnreliable)
	{
		////////////////////////////////////////////////////////////////////////////////////////////
		//UNRELIABLE PACKET
		////////////////////////////////////////////////////////////////////////////////////////////
		//if the packet is out of sequence will be discarded
		if(f.m_cSequenceNumber==m_nFrameExpected)
		{
//			CStream stmUncompressed=UncompressStream(f.m_stmData);
//			m_pParent->OnData(stmUncompressed);
			UncryptPacket( f  );
			m_pParent->OnData( f.m_stmData );
		}
		else
		{
			m_nUnreliableLostPackets++;
			NET_TRACE("[%02d]expected-[%02d]received Packet discarded\n",m_nFrameExpected,f.m_cSequenceNumber);
		}

	}
	else
	{
		////////////////////////////////////////////////////////////////////////////////////////////
		//RELIABLE PACKET
		////////////////////////////////////////////////////////////////////////////////////////////
		if(f.m_cSequenceNumber==m_nFrameExpected)
		{
//			CStream stmUncompressed=UncompressStream(f.m_stmData);
//			m_pParent->OnData(stmUncompressed);
			//UncryptStream( f.m_stmData );
			//m_pParent->OnData(f.m_stmData);
			UncryptPacket( f );
			m_pParent->OnData( f.m_stmData );

			INC(m_nFrameExpected);
			SetAckTimer();
		}
		else
		{
			SetAckTimer();
			
			NET_TRACE("[%02d]expected-[%02d]received Packet discarded\n",m_nFrameExpected,f.m_cSequenceNumber);
		}

		while(Between(m_nAckExpected,f.m_cAck,m_nNextFrameToSend))
		{
			m_nBuffered=m_nBuffered-1;
			StopTimer(m_nAckExpected%NUM_OF_BUFS);
			INC(m_nAckExpected);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCTPEndpointGNB::ProcessBufferTimers()
{
	unsigned int ulTick = m_nCurrentTime;
	DWORD nLowest = 0xFFFFFFFF;
	DWORD nLowestIdx;
	bool bFound = false;
	// search for the oldest timer
	for (long n = 0; n < NUM_OF_BUFS; n++)
	{
		if ((m_OutBuffers[n].dwTimeout != 0) &&(m_OutBuffers[n].dwTimeout < nLowest))
		{
			bFound = true;
			nLowest = m_OutBuffers[n].dwTimeout;
			nLowestIdx = n;
		}
	}
	// test the oldest timer
	if (bFound)
	{
	/*	if ((ulTick - nLowest)>(TM_BUFFER + m_LatencyCalculator.GetAverageLatency()))
		{
			/////////////////////////////////
			m_OutBuffers[nLowestIdx].dwTimeout = 0;
			HandleTimeout(m_OutBuffers[nLowestIdx].nSeq);
		}
	}*/
	DWORD dwTO=TM_BUFFER + (DWORD)(m_LatencyCalculator.GetAverageLatency());
	for (int n = 0; n < NUM_OF_BUFS; n++)
	{
		if((m_OutBuffers[nLowestIdx].dwTimeout!=0) && ((ulTick-m_OutBuffers[nLowestIdx].dwTimeout)>dwTO))
		{
			m_OutBuffers[nLowestIdx].dwTimeout = 0;
			HandleTimeout(m_OutBuffers[nLowestIdx].nSeq);
		}
		nLowestIdx=(nLowestIdx+1)%NUM_OF_BUFS;
	}
	}
}

void CCTPEndpointGNB::ProcessAckTimer()
{
	// Process Ack timeout
	unsigned int ulTick = m_nCurrentTime;
	/////////////////////////////////
	if (m_dwOutAckTimer)
	{
		if ((ulTick - m_dwOutAckTimer)>(TM_ACK + m_LatencyCalculator.GetAverageLatency()))
		{
			HandleAckTimeout();
			m_dwOutAckTimer = 0;
		}
	}
}

bool CCTPEndpointGNB::IsTimeToSend()
{
	if (m_nBuffered < NUM_OF_BUFS)
	{
		return true;
	}
	return false;
}

void CCTPEndpointGNB::HandleAckTimeout()
{
	NET_TRACE("HandleAckTimeout()\n");
	SendFrame(FT_CTP_ACK, 0, m_nFrameExpected);
}


void CCTPEndpointGNB::HandleTimeout(LONG nOldestFrame)
{
	NET_TRACE("HandleTimeout()\n");
	m_nLostPackets++;
	if (m_nBuffered)
		SendFrame(FT_CTP_DATA, nOldestFrame, m_nFrameExpected);
}

/*
CStream CCTPEndpointGNB::CompressStream(CStream &stmUncompressed)
{
	CStream stmCompressed;
#ifdef USE_PACKET_COMPRESSION
	if(m_bCompress)
	{
		BYTE *pUncompressed=NULL;
		pUncompressed=stmUncompressed.GetPtr();
		unsigned short nUncompressedSizeInBits=(unsigned short)stmUncompressed.GetSize();
		unsigned short nUncompressedSize=BITS2BYTES(nUncompressedSizeInBits);
		unsigned short n=0;
		stmCompressed.Write(true);
		stmCompressed.Write(nUncompressedSizeInBits);
		while(n<nUncompressedSize)
		{
			BYTE b;

			b=pUncompressed[n];
			if(b==0)
			{
				//write a 0
				stmCompressed.Write(false);
			}
			else
			{
				//write a 1
				stmCompressed.Write(true);
				//write a byte
				stmCompressed.Write(b);
			}
			n++;
		}
	}
	else
	{
		stmCompressed.Write(false);
		stmCompressed.WriteBits(stmUncompressed.GetPtr(),stmUncompressed.GetSize());
	}
	
#else
	stmCompressed=stmUncompressed;
#endif
	return stmCompressed;
}

CStream CCTPEndpointGNB::UncompressStream(CStream &stmCompressed)
{
	CStream stmUncompressed;
#ifdef USE_PACKET_COMPRESSION
	unsigned short nUncompressedSize;
	stmUncompressed.Reset();
	bool bIsCompressed;
	stmCompressed.Read(bIsCompressed);
	if(bIsCompressed){
		stmCompressed.Read(nUncompressedSize);
		while(!stmCompressed.EOS())
		{
			bool bBit;
			BYTE bData;
			stmCompressed.Read(bBit);
			if(bBit)
			{
				stmCompressed.Read(bData);
			}
			else
			{
				bData=0;
			}
			stmUncompressed.Write(bData);
			if(stmUncompressed.GetSize()==nUncompressedSize)
				break;
		}
	}
	else
	{
		stmCompressed.Read(stmUncompressed);
	}
#else
	stmUncompressed=stmCompressed;
#endif
	return stmUncompressed;
}
*/
void CCTPEndpointGNB::SendFrame(LONG nType, LONG nFrameNum, LONG nFrameExpected, CStream *pUnreliable,bool bUnreliable)
{
	static BYTE cUncompressed[1000];
	CTPData data;
	CTPAck ack;
	CTPNak nak;
	CTP *pFrame;
	switch (nType)
	{
		case FT_CTP_DATA:
			pFrame = &data;
			break;
		case FT_CTP_ACK:
			pFrame = &ack;
			break;
		default:
			NET_ASSERT(0);
			break;
	}
	pFrame->m_cFrameType=(BYTE)nType;
	pFrame->m_bSecondaryTC=m_bSecondary;
//////////////////////////////////////////////////////////////////
//DATA
//////////////////////////////////////////////////////////////////
	if (nType == FT_CTP_DATA)
	{
		if(!bUnreliable)
		{
			data.m_stmData = m_OutBuffers[nFrameNum%NUM_OF_BUFS].stmData;
			m_OutBuffers[nFrameNum%NUM_OF_BUFS].nSeq = nFrameNum;
			if (pUnreliable)
			{
				if(pUnreliable->GetSize())
					data.m_stmData.Write(*pUnreliable);
			}
		}
		else
		{
			data.m_stmData = *pUnreliable;
		}
		CryptPacket( data );
	}

//////////////////////////////////////////////////////////////////
//SEQ AND ACK
	pFrame->m_bUnreliable = bUnreliable;
	
	pFrame->m_cSequenceNumber =(BYTE) nFrameNum;
	
	pFrame->m_cAck =(BYTE) (nFrameExpected + MAX_SEQ_NUM)%(MAX_SEQ_NUM + 1);

	if(nType == FT_CTP_DATA)
	{
		NET_TRACE("SEND [CTP] %s FRAME SEQ [%02d] ACK [%02d] \n",pFrame->m_bUnreliable?"unreliable":"reliable",pFrame->m_cSequenceNumber,pFrame->m_cAck);
	}

//////////////////////////////////////////////////////////////////
//CHECK IF A PING REQUEST IS NEEDED
	if ((!m_bSecondary) && m_LatencyCalculator.IsTimeToPing(m_nCurrentTime))
	{
		m_dwPingTime = m_nCurrentTime;
		// set a piggybacked pong request
		pFrame->m_bPingRequest = true;
	}
//////////////////////////////////////////////////////////////////
//SERIALIZE THE FRAME
	CStream stm;
	pFrame->Save(stm);
	m_pParent->Send(stm);
	
	// Update the rate control
	m_nAllowedBytes -= BITS2BYTES(stm.GetSize());
	m_nLastPacketSent = m_nCurrentTime;
	//	NET_TRACE(">>m_nAllowedBytes=%d\n",m_nAllowedBytes);
	
	//////////////////////////////////////////
	if (nType == FT_CTP_DATA && (!bUnreliable))
		SetTimer(nFrameNum%NUM_OF_BUFS);

	if(!bUnreliable)
		StopAckTimer();
}


void CCTPEndpointGNB::SetAckTimer()
{
	if (m_dwOutAckTimer == 0)
		m_dwOutAckTimer = m_nCurrentTime;
}
//////////////////////////////////////////////////////////////////////
//! stop the ack timer
void CCTPEndpointGNB::StopAckTimer()
{
	m_dwOutAckTimer = 0;
}
//////////////////////////////////////////////////////////////////////
//!	set packet retrasmisson timeout
void CCTPEndpointGNB::SetTimer(LONG nIndex)
{
	m_OutBuffers[nIndex].dwTimeout = m_nCurrentTime;
	NET_TRACE("SETTIMER %02d %d\n",nIndex,m_nCurrentTime);
}
//////////////////////////////////////////////////////////////////////
//! stop packet retrasmisson timeout
void CCTPEndpointGNB::StopTimer(LONG nIndex)
{
	DWORD nTimeout = m_OutBuffers[nIndex].dwTimeout;
	m_OutBuffers[nIndex].dwTimeout=0;
	NET_TRACE("STOPTIMER %02d %d\n",nIndex,nTimeout);
}


void CCTPEndpointGNB::BuildOutgoingFrame()
{
	if (m_qOutgoingReliableData.empty() == false || m_qOutgoingUnreliableData.empty() == false)
	{
		CStream stmUnreliable;

		CStream &stm=m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData;
		stm.Reset();
		while ((!m_qOutgoingReliableData.empty()) 
			&& ((stm.GetSize() + m_qOutgoingReliableData.front().GetSize()) < MAX_PLAYLOAD_SIZE_IN_BITS))
		{
			stm.Write(m_qOutgoingReliableData.front());
			m_qOutgoingReliableData.pop();
		}
		while ((!m_qOutgoingUnreliableData.empty()) 
			&& ((stm.GetSize() + m_qOutgoingUnreliableData.front().GetSize()) < MAX_PLAYLOAD_SIZE_IN_BITS))
		{
			stmUnreliable.Write(m_qOutgoingUnreliableData.front());
			m_qOutgoingUnreliableData.pop();
		}
		//CHECK IF THERE IS RELIABLE DATA IN THE QUEUE
		if(stm.GetSize()>0)
		{
			SendFrame(FT_CTP_DATA, m_nNextFrameToSend, m_nFrameExpected, &stmUnreliable,false);
			INC(m_nNextFrameToSend);
			m_nBuffered += 1;
			NET_ASSERT(m_nBuffered<=NUM_OF_BUFS);
			NET_TRACE("SEND RELIABLE m_nBuffered=%02d\n",m_nBuffered);
		}
		else
		{
			//IF THERE ISN'T RELIABLE DATA SEND A UNRELIABLE ONLY PACKET
			//IF THERE IS UNRELIABLE DATA
			if(stmUnreliable.GetSize()>0)
			{
				SendFrame(FT_CTP_DATA, m_nNextFrameToSend, m_nFrameExpected, &stmUnreliable,true);
				NET_TRACE("SEND UNRELIABLE\n");
			} 
		}
	}
}

void CCTPEndpointGNB::GetMemoryStatistics(ICrySizer *pSizer)
{
	pSizer->AddObject(&m_qOutgoingReliableData,m_qOutgoingReliableData.size()*sizeof(CStream));
	pSizer->AddObject(&m_qOutgoingUnreliableData,m_qOutgoingUnreliableData.size()*sizeof(CStream));
}
