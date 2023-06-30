#include "stdafx.h"
#include "compressionhelper.h"


CStaticCharCompressor::CStaticCharCompressor()
{
	m_dwRootIndex=0xffffffff;
}


void CStaticCharCompressor::InitFromStatistics( const DWORD indwPriority[256] )
{
	std::multiset<SPriIndex>	Sorter;

	m_Nodes.clear();
	m_Nodes.reserve(256*2-1);

	for(DWORD i=0;i<256;i++)
	{
		m_Nodes.push_back( SNode((unsigned char)i) );
		Sorter.insert( SPriIndex(i,indwPriority[i]+1) );			// +1 to ensure if we add two priorites the value is raising
	}
 
	// iterate through all nodes from lowest to highest priority
	for(;;)
	{
		DWORD dwIndex1,dwIndex2;
		DWORD dwPriority;

		// get element with lowest priorty
		{
			std::multiset<SPriIndex>::iterator top = Sorter.begin();

			assert(top!=Sorter.end());			// cannot be because we always remove two and add one
			
			const SPriIndex &ref = *top;

			dwIndex1=ref.m_dwIndex;
			dwPriority=ref.m_dwPriority;

			Sorter.erase(top);
		}

		// get element with second lowest priorty
		{
			std::multiset<SPriIndex>::iterator top = Sorter.begin();

			if(top==Sorter.end())
			{
				m_dwRootIndex=dwIndex1;
				break;
			}

			const SPriIndex &ref = *top;

			dwIndex2=ref.m_dwIndex;
			dwPriority+=ref.m_dwPriority;

			Sorter.erase(top);
		}

		DWORD dwNextIndex = m_Nodes.size();

		m_Nodes.push_back( SNode(dwIndex1,dwIndex2) );
		Sorter.insert( SPriIndex(dwNextIndex,dwPriority) );
	}

	// build m_CharToBit
	{
		for(DWORD i=0;i<256;i++)
		{
			bool bRet=GetCharFromBits_slow( (unsigned char)i, m_dwRootIndex, m_CharToBit[i] );

			assert(bRet);		// it must be there
		}
	}
}


bool CStaticCharCompressor::GetCharFromBits_slow( const unsigned char inVal, const DWORD indwIndex, SBitToChar &outDat )
{
	SNode &ref = m_Nodes[indwIndex];

	if(ref.IsLeaf())
	{
		if(ref.m_Value==inVal)
			return true;
	}
	else
	{
		outDat.m_BitCount++;
		assert(outDat.m_BitCount<16);
		outDat.m_Bitfield<<=1;

		// one
		outDat.m_Bitfield|=0x1;

		if(GetCharFromBits_slow(inVal,ref.m_dwIndexOne,outDat))
			return true;

		// zero
		outDat.m_Bitfield&=~0x1;
		if(GetCharFromBits_slow(inVal,ref.m_dwIndexZero,outDat))
			return true;

		outDat.m_Bitfield>>=1;
		outDat.m_BitCount--;
	}

	return false;
}


bool CStaticCharCompressor::Write( CStream &outStream, const unsigned char inChar )
{
	assert(m_dwRootIndex!=0xffffffff);			// instance is not initialized

	SBitToChar &ref=m_CharToBit[inChar];
/*
#ifdef _DEBUG
	CStream test;

	unsigned char cTest;

	test.WriteNumberInBits((DWORD)(ref.m_Bitfield),ref.m_BitCount);
	Read(test,cTest);
	assert(cTest==inChar);
#endif
*/

	for(DWORD i=0;i<ref.m_BitCount;i++)
	{
		bool b=(ref.m_Bitfield & (1<<(ref.m_BitCount-i-1)))!=0;

		if(!outStream.Write(b))
			return false;
	}
//	return outStream.WriteNumberInBits((DWORD)(ref.m_Bitfield),ref.m_BitCount);
	return true;
}

bool CStaticCharCompressor::Read( CStream &inStream, unsigned char &outChar )
{
	assert(m_dwRootIndex!=0xffffffff);			// 

	DWORD dwCurrentIndex=m_dwRootIndex;

	for(;;)
	{
		bool bBit;

		SNode &ref=m_Nodes[dwCurrentIndex];

		if(ref.IsLeaf())
		{
			outChar=ref.m_Value;
			return true;
		}

		if(!inStream.Read(bBit))
			return false;

		if(bBit)
			dwCurrentIndex=ref.m_dwIndexOne;
		 else
			dwCurrentIndex=ref.m_dwIndexZero;
	}
}
