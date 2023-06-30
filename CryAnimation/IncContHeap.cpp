#include "stdafx.h"
#include "IncContHeap.h"

void CTempStorage::done()
{
	if (m_pTemp)
	{
		// read the offset to the actual memory
		int nOffset = ((int*)m_pTemp)[-1];
		assert (nOffset >= -16 && nOffset <=4);
		// the actual memory block start
		void* pRealMem = (char*)m_pTemp + nOffset;
		free (pRealMem); // free it
		m_pTemp = NULL;
		m_nSize = 0;
	}
}

void CTempStorage::reserve (unsigned sizeMin)
{
	if (m_nSize < sizeMin)
	{
		done();
		m_nSize = (sizeMin+g_nBlockIncrement-1)&~(g_nBlockIncrement-1);
		unsigned* pRealMem;

#ifdef WIN32
#ifdef _DEBUG
		pRealMem = (unsigned*)_malloc_dbg (m_nSize+0x10, _NORMAL_BLOCK, "Scratchpad", 0);
#else
		pRealMem = (unsigned*)malloc (m_nSize+0x10);
#endif
#else
//#ifdef GAMECUBE
		pRealMem = (unsigned*)malloc (m_nSize+0x10);
#endif

		// align the memory on 0x10-byte-boundary
		m_pTemp = (LPVOID) (((UINT_PTR)pRealMem + 0x10)&~0xF);
		// memorize the offset to the actual memory (it's always guaranteed
		// that at least 4 bytes (sizeof(int)) will be behind the m_pTemp)
		// it's a negative number
		((int*)m_pTemp)[-1] = (char*)pRealMem-(char*)m_pTemp;
	}
}

CTempStorage g_Temp;