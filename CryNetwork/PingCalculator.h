//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: PingCalculator.h
//  Description: Latency(Ping) calculator
//
//  History:
//  - August 10, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#ifndef _PING_CALCULATOR_H_
#define _PING_CALCULATOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NUM_OF_SAMPLES 10
#define DIV_MULTIPLIER (1.0/NUM_OF_SAMPLES)
#define INC_INDEX(a) (a) = ((++a)%NUM_OF_SAMPLES)
#define PING_RATE 500

class CPingCalculator  
{
public:
	CPingCalculator()
	{
		m_nSampleIndex = 0;
		m_nLastSample = 0;
		m_bSyncronized = false;
		m_nRemoteTimestamp = 0;
		memset(m_fSample, 0, sizeof(m_fSample));
		m_fPing=100;
	}
	float GetAverageLatency()
	{
		float fSum = 0;
		
		// return (fSum*(DIV_MULTIPLIER));
		return m_fPing;
	}
	bool IsTimeToPing(unsigned int nCurrentTime)
	{
		if ((nCurrentTime - m_nLastSample)>PING_RATE)
		{
			m_nLastSample = nCurrentTime;
			return true;
		}
		return false;
	}
	void AddSample(float f,unsigned int nLocalTimestamp,unsigned int nRemoteTimestamp)
	{
		float fSum = 0;
		float fTemp[NUM_OF_SAMPLES];

		m_fSample[m_nSampleIndex] = f*0.5f;
		memcpy(fTemp, m_fSample, sizeof(fTemp));
		qsort(fTemp, NUM_OF_SAMPLES, sizeof(float), CPingCalculator::Compare);
		
		m_fPing = fTemp[NUM_OF_SAMPLES/2];
		
		INC_INDEX(m_nSampleIndex);
		NET_TRACE("PING %04d\n", LONG(m_fPing));
		m_nRemoteTimestamp=(unsigned int)(nRemoteTimestamp-m_fPing);
		m_nLocalTimestamp=nLocalTimestamp;
	}
	unsigned int GetCurrentRemoteTimestamp(unsigned int nLocalTimestamp){
		unsigned int nCurrentDelta=m_nLocalTimestamp-nLocalTimestamp;
		return m_nRemoteTimestamp+nCurrentDelta;
	}
	unsigned int GetPacketLatency(unsigned int nLocalTimestamp,unsigned int nPacketTimestamp){
			unsigned int nRemote=GetCurrentRemoteTimestamp(nLocalTimestamp);
			return nPacketTimestamp-nRemote;
	}
	static int __cdecl Compare(const void *arg1, const void *arg2)
	{
		float f = ((*(float *)arg1 - (*(float *)arg2)));
		if (f>0)
			return 1; // greater
		if (f < 0)
			return -1; // less
		return 0; // equel
	}
private:
	float m_fSample[NUM_OF_SAMPLES];
	//! middle value of all samples
	float m_fPing;
	unsigned int m_nSampleIndex;
	unsigned int m_nLastSample;
	bool m_bSyncronized;
	//! average remote timestamp
	unsigned int m_nRemoteTimestamp;
	//! local timestamp
	unsigned int m_nLocalTimestamp;
};

#endif // _PING_CALCULATOR_H_
