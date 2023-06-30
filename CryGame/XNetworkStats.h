
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef XNETWORKSTATS_H
#define XNETWORKSTATS_H

#include <vector>				// STL vector<>

class CXNetworkStats
{
public:

	//! constructor
	CXNetworkStats();

	//! destructor
	virtual ~CXNetworkStats();

	//! /param inBitSize including the Item identification
	void AddPacket( const DWORD indwItem, const size_t inBitSize, const bool inbReliable );

	//!
	void Update( const CTimeValue &inCurrentTime );

	//! /param iniN 0..
	//! /param outItem depending on AddPacket(inItem...)
	//! /param outCount
	//! /param outSumSize
	//! /return true=returned data is valid (go on the itertaion), false=Item with that number does not exist (stop iteration) 
	bool GetStats( const int iniN, DWORD &outdwItem, DWORD &outRelCount, DWORD &outUnrelCount, size_t &outSumBitSize );

	//! /param inPos [0..5000[
	void AddToSumGraph( const DWORD inPos, const DWORD indwDelta );
	void AddToUpdateCount(DWORD dwDelta) { m_dwSumGraphGatherCount += dwDelta; };

	//! /param inPos [0..5000[
	float GetSumGraphValue( const DWORD inPos ) const;

	DWORD GetResetCount() const;

private: // ---------------------------------------------------------------------------------


	struct SCountSize
	{
		DWORD				m_dwRelCount;				//!< reliable 0..
		DWORD				m_dwUnrelCount;			//!< unreliable 0..
		size_t			m_SumBitSize;				//!< sum in bts 0..
	};

	std::vector<SCountSize>				m_DrawStats;						//!< m_Stats[Item]=SCountSize
	std::vector<SCountSize>				m_GatherStats;					//!< m_Stats[Item]=SCountSize
	CTimeValue										m_LastTimeReset;				//!< 

	std::vector<DWORD>						m_SumGraphGather;				//!<
	std::vector<DWORD>						m_SumGraphDraw;					//!<
	DWORD													m_dwSumGraphGatherCount;//!<
	DWORD													m_dwSumGraphDrawCount;	//!<
	DWORD													m_dwResetCount;					//!<

	//! /param iniN 0..
	int GetNthUsedDrawIndex( const int iniN );

	//!
	void ResetDrawStats();
	//!
	void ResetGatherStats();
};


#endif // XNETWORKSTATS_H
