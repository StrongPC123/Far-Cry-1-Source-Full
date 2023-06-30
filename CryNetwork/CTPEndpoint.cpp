//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: CTPEndpoint.cpp
//  Description: non-sequential receive protocol
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CTPEndpoint.h"
#include "IDataProbe.h"


#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static bool Between(LONG a, LONG b, LONG c) 
{
	return ((a <= b) &&(b < c)) ||((c < a) &&(a <= b)) ||((b < c) &&(c < a));
}

//////////////////////////////////////////////////////////////////////
//!	constructor!
CCTPEndpoint::CCTPEndpoint()
{
	Reset();
	SetRate(10000);
	m_nSnaps = 20;
	m_nEncryptKey[0] = 1714732178u;
	m_nEncryptKey[1] = 2157124251u;
	m_nEncryptKey[2] = 3766711231u;
	m_nEncryptKey[3] = 715376114u;
}

//////////////////////////////////////////////////////////////////////
//!	destructor!!
CCTPEndpoint::~CCTPEndpoint()
{
}

void CCTPEndpoint::SetPublicCryptKey( unsigned int key )
{
	m_nEncryptKey[1] = key;
}

//////////////////////////////////////////////////////////////////////
//!	set the average rate of the outgoing stream
void CCTPEndpoint::SetRate(unsigned int nBytePerSec)
{
	m_nRate = nBytePerSec;
	m_fBytePerMillisec = ((float)m_nRate)/1000.0f;
	NET_TRACE("BYTES per MILLISEC=%f\n", m_fBytePerMillisec);
	m_nAllowedBytes = m_nRate;
}

//////////////////////////////////////////////////////////////////////
//! set all varible to the initial state
void CCTPEndpoint::Reset()
{
	//memset(m_nArrived, 0, sizeof(m_nArrived));
	m_nFrameExpected = 0;
	m_nNextFrameToSend = 0;
	m_nTooFar = NUM_OF_BUFS;
	m_bNoNak = true;
	m_nAckExpected = 0;
	m_nBuffered = 0;
	m_nLostPackets = 0;
	m_dwOutAckTimer = 0;
  //memset(m_dwTimers, 0, sizeof(m_dwTimers));
	for (int n = 0; n < NUM_OF_BUFS; n++)
	{
		m_OutBuffers[n].dwTimeout = 0;
		m_OutBuffers[n].nSeq = 0;
		m_nArrived[n]=false;
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

//////////////////////////////////////////////////////////////////////
//! called for every FT_DATA (CTPData)
void CCTPEndpoint::HandleDataFrame(CTPData &f)
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
			UncyptStream( f.m_stmData );

			m_pParent->OnData(f.m_stmData);
		}
		else{
			NET_TRACE("[%02d]expected-[%02d]received Packet discarded\n",m_nFrameExpected,f.m_cSequenceNumber);
		}

	}
	else
	{
		////////////////////////////////////////////////////////////////////////////////////////////
		//RELIABLE PACKET
		////////////////////////////////////////////////////////////////////////////////////////////
		// if there is a possible packet loss send a negative acknoledge (FT_NAK)
		if ((f.m_cSequenceNumber != m_nFrameExpected) && m_bNoNak)
		{
			//<<FIXME>> re enable the following two lines to enable NAKs
			NET_TRACE("NAK !!! f.m_cSequanceNumber=%02d m_nFrameExpected=%02d\n", f.m_cSequenceNumber, m_nFrameExpected);
			SendFrame(FT_CTP_NAK, 0, m_nFrameExpected);
		}
		else
			SetAckTimer();
		if (Between(m_nFrameExpected, f.m_cSequenceNumber, m_nTooFar) &&(m_nArrived[f.m_cSequenceNumber%NUM_OF_BUFS] == false))
		{
			// FRAME ACCEPTED
			NET_TRACE("FRAME ACCEPTED %02d\n",f.m_cSequenceNumber);
			m_nArrived[f.m_cSequenceNumber%NUM_OF_BUFS] = true;
			m_stmInBuffers[f.m_cSequenceNumber%NUM_OF_BUFS] = f.m_stmData;

			while (m_nArrived[m_nFrameExpected%NUM_OF_BUFS])
			{
				NET_TRACE("ARRIVED %02d\n",m_nFrameExpected%NUM_OF_BUFS);
//				CStream stmUncompressed=UncompressStream(m_stmInBuffers[m_nFrameExpected%NUM_OF_BUFS]);
//				m_pParent->OnData(stmUncompressed);
				UncyptStream( m_stmInBuffers[m_nFrameExpected%NUM_OF_BUFS] );
				m_pParent->OnData(m_stmInBuffers[m_nFrameExpected%NUM_OF_BUFS]);

				/////////////////////////////////////////////////
				m_bNoNak = true;
				m_nArrived[m_nFrameExpected%NUM_OF_BUFS] = false; // set buffer flag to false

				NET_TRACE("m_nFrameExpected=%02d\n",m_nFrameExpected);
				INC(m_nFrameExpected);
				INC(m_nTooFar);
				SetAckTimer();
			}
		}
		else
		{
			NET_TRACE("received out of window frame (%d)\n",f.m_cSequenceNumber);
		}
	}
}

//////////////////////////////////////////////////////////////////////
//! called every incoming NAK (this code perform packet retrasmission)
void CCTPEndpoint::HandleNak(CTPNak &f)
{
	if (Between(m_nAckExpected, (f.m_cAck + 1)%(MAX_SEQ_NUM + 1), m_nNextFrameToSend))
	{
		m_nLostPackets++;
		SendFrame(FT_CTP_DATA, (f.m_cAck + 1)%(MAX_SEQ_NUM + 1), m_nNextFrameToSend);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CCTPEndpoint::Dump()
{
	NET_TRACE("m_nTooFar=%d |||", m_nTooFar);
	NET_TRACE("m_nFrameExpected=%d |||", m_nFrameExpected);
	NET_TRACE("m_nBuffered=%d |||", m_nBuffered);
	NET_TRACE("m_nAckExpected=%d |||", m_nAckExpected);
	NET_TRACE("m_nNextFrameToSend=%d\n", m_nNextFrameToSend);
}

//////////////////////////////////////////////////////////////////////
//! main loop of the class
void CCTPEndpoint::Update(unsigned int nTime, unsigned char cFrameType, CStream *pStm)
{
		m_nCurrentTime = nTime;
		// if there is a incoming frame
		CTP *pFrame = NULL;
		CTPAck ack;
		CTPData data;
		CTPNak nak;
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
				case FT_CTP_NAK:
					{
						nak.Load(*pStm);
						pFrame = &nak;
						HandleNak(nak);
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

//////////////////////////////////////////////////////////////////////
//! set the ack timer timeout
//! NOTE: if there aren't outgoing data packets for several time 
//! the protocol will transmit an "ack dedicated" packet(FT_CTP_ACK)
//! this is the timer to guard this condition
void CCTPEndpoint::SetAckTimer()
{
	if (m_dwOutAckTimer == 0)
		m_dwOutAckTimer = m_nCurrentTime;
}
//////////////////////////////////////////////////////////////////////
//! stop the ack timer
void CCTPEndpoint::StopAckTimer()
{
	m_dwOutAckTimer = 0;
}
//////////////////////////////////////////////////////////////////////
//!	set packet retrasmisson timeout
void CCTPEndpoint::SetTimer(LONG nIndex)
{
//	if(m_OutBuffers[nIndex].dwTimeout!=0)
//		DEBUG_BREAK;
	m_OutBuffers[nIndex].dwTimeout = m_nCurrentTime;
	NET_TRACE("SETTIMER %02d %d\n",nIndex,m_nCurrentTime);
}
//////////////////////////////////////////////////////////////////////
//! stop packet retrasmisson timeout
void CCTPEndpoint::StopTimer(LONG nIndex)
{
	DWORD nTimeout = m_OutBuffers[nIndex].dwTimeout;
	m_OutBuffers[nIndex].dwTimeout=0;
	// PING STUFF
	//	m_ulPingCounter+=m_nCurrentTime-m_OutBuffers[nIndex].dwTimeout;
	// m_ulSamplesNum++;
	// End of ping stuff
	/*for (int n = 0; n < NUM_OF_BUFS; n++)
	{
		if (m_OutBuffers[n].dwTimeout <= nTimeout)
			m_OutBuffers[n].dwTimeout = 0;
	}*/
	NET_TRACE("STOPTIMER %02d %d\n",nIndex,nTimeout);
}

void CCTPEndpoint::CryptStream( CStream &stm )
{
	/*
	IDataProbe *pProbe = GetISystem()->GetIDataProbe();
	char temp[2048];
	//int len = stm.
	unsigned int* pBuffer = (unsigned int*)stm.GetPtr();
	unsigned int destLen = 2048;
	pProbe->Compress( temp,destLen,pBuffer,stm.GetSize()*8 );
	int len = TEA_GETSIZE(stm.GetSize()*8);
	//TEA_ENCODE( pBuffer,pBuffer,len,m_nEncryptKey );
	CryLogAlways( "Stream Size: %.6d/%.6d",stm.GetSize()*8,destLen );
	pProbe->Release();
	*/
}

void CCTPEndpoint::UncyptStream( CStream &stm )
{
	/*
	unsigned int* pBuffer = (unsigned int*)stm.GetPtr();
	int len = TEA_GETSIZE(stm.GetSize()*8);
	//TEA_DECODE( pBuffer,pBuffer,len,m_nEncryptKey );
	*/
}

//////////////////////////////////////////////////////////////////////
//! push a reliable stream in the outgoing queue
bool CCTPEndpoint::SendReliable(CStream &stmData)
{
	//if(m_qOutgoingReliableData.size()>MAX_QUEUED_PACKET_PER_CHANNEL)
	//	return false;
	CryptStream( stmData );
	m_qOutgoingReliableData.push(stmData);
	return true;
}
//////////////////////////////////////////////////////////////////////
//! push an UNreliable stream in the outgoing queue
bool CCTPEndpoint::SendUnreliable(CStream &stmData)
{
	//if(m_qOutgoingUnreliableData.size()>MAX_QUEUED_PACKET_PER_CHANNEL)
	//	return false;
	CryptStream( stmData );
	m_qOutgoingUnreliableData.push(stmData);
	return true;
}

/*
void MTFEncode(BYTE *pDest,BYTE *pSource,int nSize)
{
	unsigned char order[ 256 ];
	int i,c,count;

	for ( i = 0 ; i < 256 ; i++ )
	{
		order[ i ] = (unsigned char) i;
	}

	for(count=0;count<nSize;count++)
	{
		c=pSource[count];
		
		//
		// Find the char, and output it
		//
		for ( i = 0 ; i < 256 ; i++ )
		{
			if ( order[ i ] == ( c & 0xff ) )
				break;
		}
			//putc( (char) i, stdout );
		pDest[count]=i;
		//
		// Now shuffle the order array
		//
		for ( int j = i ; j > 0 ; j-- )
		{
			order[ j ] = order[ j - 1 ];
		}
		order[ 0 ] = (unsigned char) c;
	}
}

void MTFDecode(BYTE *pDest,BYTE *pSource,int nSize)
{
	
	 unsigned char order[ 256 ];
	 int i,c,count;

	 for ( i = 0 ; i < 256 ; i++ )
	 {
		 order[ i ] = (unsigned char) i;
	 }

	 for(count=0;count<nSize;count++) 
	 {
		 i=pSource[count];
		 
		 //
		 // Find the char
		 //
		 //putc( order[ i ], stdout );
		 pDest[count]=order[i];
		 c = order[ i ];
		 //
		 // Now shuffle the order array
		 //
		 int j;
		 for ( j = i ; j > 0 ; j-- )
		 {
			 order[ j ] = order[ j - 1 ];
		 }
		 order[ 0 ] = (unsigned char) c;
	 }
}
*/

/*
CStream CCTPEndpoint::CompressStream(CStream &stmUncompressed)
{
	CStream stmCompressed;
#ifdef USE_PACKET_COMPRESSION
	BYTE *pUncompressed=NULL;
	pUncompressed=stmUncompressed.GetPtr();
	unsigned short nUncompressedSizeInBits=stmUncompressed.GetSize();
	unsigned short nUncompressedSize=BITS2BYTES(nUncompressedSizeInBits);
	unsigned short n=0;
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
	
#else
	stmCompressed=stmUncompressed;
#endif
	return stmCompressed;
}

CStream CCTPEndpoint::UncompressStream(CStream &stmCompressed)
{
	CStream stmUncompressed;
#ifdef USE_PACKET_COMPRESSION
	unsigned short nUncompressedSize;
	stmUncompressed.Reset();
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
	
#else
	stmUncompressed=stmCompressed;
#endif
	return stmUncompressed;
}
*/

//////////////////////////////////////////////////////////////////////
//! format and send a frame(FT_DATA or FT_ACK or FT_NAK)
void CCTPEndpoint::SendFrame(LONG nType, LONG nFrameNum, LONG nFrameExpected, CStream *pUnreliable,bool bUnreliable)
{
	//	Dump();
	CStream stmUncompressed;
	
	static BYTE cUncompressed[1000];
	CStream stm;
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
		case FT_CTP_NAK:
			pFrame = &nak;
			break;
		default:
			NET_ASSERT(0);
			break;
	}
//	pFrame->m_cClientID = m_pParent->GetID();
	pFrame->m_cFrameType =(BYTE) nType;

//////////////////////////////////////////////////////////////////
//DATA
//////////////////////////////////////////////////////////////////
	if (nType == FT_CTP_DATA)
	{
		if(!bUnreliable)
		{
			stmUncompressed = m_OutBuffers[nFrameNum%NUM_OF_BUFS].stmData;
			m_OutBuffers[nFrameNum%NUM_OF_BUFS].nSeq = nFrameNum;
			if (pUnreliable)
			{
				if(pUnreliable->GetSize())
					stmUncompressed.Write(*pUnreliable);
			}
		}
		else
		{
			stmUncompressed=*pUnreliable;
		}
		//pack the stream with RLE
//		data.m_stmData=CompressStream(stmUncompressed);
		data.m_stmData=stmUncompressed;
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
//NAK
	if (nType == FT_CTP_NAK)
		m_bNoNak = false;

//////////////////////////////////////////////////////////////////
//CHECK IF A PING REQUEST IS NEEDED
	if (m_LatencyCalculator.IsTimeToPing(m_nCurrentTime))
	{
		m_dwPingTime = m_nCurrentTime;
		// set a piggybacked pong request
		pFrame->m_bPingRequest = true;
	}
//////////////////////////////////////////////////////////////////
//SERIALIZE THE FRAME
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
//////////////////////////////////////////////////////////////////////
//!	handle a buffer timeout (see SetTimer)
void CCTPEndpoint::HandleTimeout(LONG nOldestFrame)
{
	NET_TRACE("HandleTimeout()\n");
	m_nLostPackets++;
	if (m_nBuffered)
		SendFrame(FT_CTP_DATA, nOldestFrame, m_nFrameExpected);
}
//////////////////////////////////////////////////////////////////////
//!	handle an ack timeout (see SetAckTimer)
void CCTPEndpoint::HandleAckTimeout()
{
	NET_TRACE("HandleAckTimeout()\n");
	SendFrame(FT_CTP_ACK, 0, m_nFrameExpected);
}


//////////////////////////////////////////////////////////////////////
//!	check "the clock" for a possible buffers timer expiration
void CCTPEndpoint::ProcessBufferTimers()
{
	unsigned int ulTick = m_nCurrentTime;
	DWORD nLowest = 0xFFFFFFFF;
	DWORD nLowestIdx;
	bool bFound = false;
	// search for the oldest timer
	for (int n = 0; n < NUM_OF_BUFS; n++)
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
		if ((ulTick - nLowest)>(TM_BUFFER + m_LatencyCalculator.GetAverageLatency()))
		{
			/////////////////////////////////
			m_OutBuffers[nLowestIdx].dwTimeout = 0;
			HandleTimeout(m_OutBuffers[nLowestIdx].nSeq);
		}
	}
}
//////////////////////////////////////////////////////////////////////
//!	check "the clock" for a possible ack timer expiration
void CCTPEndpoint::ProcessAckTimer()
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


unsigned int CCTPEndpoint::GetPing()
{
	return (unsigned int)m_LatencyCalculator.GetAverageLatency();
}
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Client logic
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//! return true if is time to send something 
bool CCTPClient::IsTimeToSend()
{
	static int dwTimer = 0;
	// is there a free slot?
	if (m_nBuffered < NUM_OF_BUFS)
	{
		return true;
		/*if ((m_nCurrentTime - dwTimer)>30)
		{
		dwTimer = m_nCurrentTime;
		return true;
		}
		return false; */
	}
	return false;
}
//////////////////////////////////////////////////////////////////////
//! build a data frame from the outgoing queue
void CCTPEndpoint::BuildOutgoingFrame()
{
	if (m_qOutgoingReliableData.empty() == false || m_qOutgoingUnreliableData.empty() == false)
	{
		CStream stmUnreliable;
		
		m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.Reset();
		while ((m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.GetSize() < MAX_PLAYLOAD_SIZE_IN_BITS) && (!m_qOutgoingReliableData.empty()))
		{
			m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.Write(m_qOutgoingReliableData.front());
			m_qOutgoingReliableData.pop();
		}
		while ((m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.GetSize() < MAX_PLAYLOAD_SIZE_IN_BITS) && (!m_qOutgoingUnreliableData.empty()))
		{
			stmUnreliable.Write(m_qOutgoingUnreliableData.front());
			m_qOutgoingUnreliableData.pop();
		}
		//CHECK IF THERE IS RELIABLE DATA IN THE QUEUE
		if(m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.GetSize()>0)
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
			} }
		
	}
	//<<FIXME>> write some stuff to polulate a packet
	//<<FIXME>> remember never put unreliable data in the retrasmission buffer(m_OutBuffer)
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Server logic
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//! return true if is time to send something 
//#define RATE_CONTROL
bool CCTPServer::IsTimeToSend()
{
#ifdef RATE_CONTROL
	static dwTimer = 0;
	
	m_nAllowedBytes += (int)((m_fBytePerMillisec)*(m_nCurrentTime - m_nLastPacketSent));
	if (m_nAllowedBytes>(int)m_nRate)
		m_nAllowedBytes =(int)m_nRate;
	 
	
	if (m_nAllowedBytes>((int)(m_nRate/m_nSnaps)))
	{ 
		return true;
	}//<<FIXME>>
	return false;
#else
	static int dwTimer = 0;
	// is there a free slot?
	if (m_nBuffered < NUM_OF_BUFS)
	{
		return true;
		/*if ((m_nCurrentTime - dwTimer)>30)
		{
		dwTimer = m_nCurrentTime;
		return true;
		}
		return false; */
	}
	return false;
#endif
}

/*
//////////////////////////////////////////////////////////////////////
//! build a data frame from the outgoing queue
void CCTPServer::BuildOutgoingFrame()
{
	if (m_qOutgoingReliableData.empty() == false || m_qOutgoingUnreliableData.empty() == false)
	{
		CStream stmUnreliable;
		
		m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.Reset();
		while ((m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.GetSize() < MAX_PLAYLOAD_SIZE_IN_BITS) && (!m_qOutgoingReliableData.empty()))
		{
			m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.Write(m_qOutgoingReliableData.front());
			m_qOutgoingReliableData.pop();
		}
		while ((m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.GetSize() < MAX_PLAYLOAD_SIZE_IN_BITS) && (!m_qOutgoingUnreliableData.empty()))
		{
			stmUnreliable.Write(m_qOutgoingUnreliableData.front());
			m_qOutgoingUnreliableData.pop();
		}
		//CHECK IF THERE IS RELIABLE DATA IN THE QUEUE
		if(m_OutBuffers[m_nNextFrameToSend%NUM_OF_BUFS].stmData.GetSize()>0)
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
	//<<FIXME>> write some stuff to polulate a packet
	//<<FIXME>> remember never put unreliable data in the retrasmission buffer(m_OutBuffer)
}
*/
