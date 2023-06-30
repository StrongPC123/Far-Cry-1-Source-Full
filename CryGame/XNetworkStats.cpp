
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xnetworkstats.h"

#define MAX_ITEMS							30
#define MAX_SUMGRAPHELEMENTS	1000

CXNetworkStats::CXNetworkStats()
{
	m_DrawStats.resize(MAX_ITEMS);
	m_GatherStats.resize(MAX_ITEMS);
	ResetDrawStats();
	ResetGatherStats();
	m_SumGraphGather.resize(MAX_SUMGRAPHELEMENTS);
	m_SumGraphDraw.resize(MAX_SUMGRAPHELEMENTS);
	m_dwResetCount=0;
}

CXNetworkStats::~CXNetworkStats()
{
}


DWORD CXNetworkStats::GetResetCount() const
{
	return m_dwResetCount;
}


void CXNetworkStats::Update( const CTimeValue &inCurrentTime )
{
	if(m_LastTimeReset==CTimeValue())
		m_LastTimeReset=inCurrentTime;

	CTimeValue RelTime=inCurrentTime-m_LastTimeReset;
	CTimeValue sec,zero;

	// todo: improve CTimeValue interface
	sec.SetSeconds(1);zero.SetSeconds(0);

	if(RelTime>sec || RelTime<zero)		// after one sec or if timer was reseted
	{
		m_DrawStats=m_GatherStats;
		m_SumGraphDraw = m_SumGraphGather;
		m_dwSumGraphDrawCount = m_dwSumGraphGatherCount;
		ResetGatherStats();
		m_LastTimeReset=inCurrentTime;

		m_dwResetCount++;
	}
}

void CXNetworkStats::ResetDrawStats()
{	
	std::vector<SCountSize>::iterator it;

	for(it=m_DrawStats.begin();it!=m_DrawStats.end();it++)
	{
		SCountSize &ref=*it;																

		ref.m_dwRelCount=0;
		ref.m_dwUnrelCount=0;
		ref.m_SumBitSize=0;
	}

	for (std::vector<DWORD>::iterator dwit = m_SumGraphDraw.begin(); dwit != m_SumGraphDraw.end(); ++dwit)
	{
		*dwit = 0;
	}

	m_dwSumGraphDrawCount = 0;
}

void CXNetworkStats::ResetGatherStats()
{	
	std::vector<SCountSize>::iterator it;

	for(it=m_GatherStats.begin();it!=m_GatherStats.end();it++)
	{
		SCountSize &ref=*it;																

		ref.m_dwRelCount=0;
		ref.m_dwUnrelCount=0;
		ref.m_SumBitSize=0;
	}

	for (std::vector<DWORD>::iterator dwit = m_SumGraphGather.begin(); dwit != m_SumGraphGather.end(); ++dwit)
	{
		*dwit = 0;
	}
	m_dwSumGraphGatherCount = 0;
}


void CXNetworkStats::AddPacket( const DWORD indwItem, const size_t inBitSize, const bool inbReliable )
{
	if(indwItem>=MAX_ITEMS)
	{
		assert(0);
		return;
	}

	SCountSize &ref=m_GatherStats[indwItem];

	if(inbReliable)
		ref.m_dwRelCount++;
	 else
		ref.m_dwUnrelCount++;

	ref.m_SumBitSize+=inBitSize;
}

int CXNetworkStats::GetNthUsedDrawIndex( const int iniN )
{
	std::vector<SCountSize>::iterator it;

	int iUsed=0,i=0;
	for(it=m_DrawStats.begin();it!=m_DrawStats.end();++it,i++)
	{
		SCountSize &ref=*it;																

		if(ref.m_SumBitSize==0)continue;

		if(iniN==iUsed)
			return i;

		iUsed++;
	}

	return -1;
}

bool CXNetworkStats::GetStats( const int iniN, DWORD &outdwItem, DWORD &outRelCount, DWORD &outUnrelCount, size_t &outSumBitSize )
{	
	int iIndex=GetNthUsedDrawIndex(iniN);

	if(iIndex==-1)
		return false;

	outdwItem=(DWORD)iIndex;

	SCountSize &ref=m_DrawStats[iIndex];

	outRelCount=ref.m_dwRelCount;
	outUnrelCount=ref.m_dwUnrelCount;
	outSumBitSize=ref.m_SumBitSize;
	assert(outSumBitSize);

	return true;
}


//------------------------------------------------------------------------------------------------- 
void CXNetworkStats::AddToSumGraph( const DWORD inPos, const DWORD indwDelta )
{
	assert(inPos<m_SumGraphDraw.size());

	m_SumGraphGather[inPos] += indwDelta;
}


//------------------------------------------------------------------------------------------------- 
float CXNetworkStats::GetSumGraphValue( const DWORD inPos ) const
{
	assert(inPos<m_SumGraphDraw.size());

	return (float)(m_SumGraphDraw[inPos]);
}
