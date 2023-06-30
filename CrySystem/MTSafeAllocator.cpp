#include "stdafx.h"
#include "MTSafeAllocator.h"

#if defined(LINUX)
	#define USE_CRT 1
#else
//#ifdef _DEBUG
//#define USE_CRT 1
//#else
#define USE_CRT 0
//#endif
#endif	//LINUX

CMTSafeHeap::CMTSafeHeap(unsigned nInitialSize, unsigned nMaxSize)
{
	m_numAllocations = 0;
	m_nTotalAllocations = 0;
#if !USE_CRT
	m_hHeap = HeapCreate (0, nInitialSize, nMaxSize);
#endif
}

CMTSafeHeap::~CMTSafeHeap()
{
#if !defined(LINUX)
	assert (m_numAllocations == 0);
#endif
#if !USE_CRT
	assert (HeapValidate(m_hHeap, 0, NULL));
	HeapDestroy (m_hHeap);
#endif
}

// consolidates free space inthe heap, uncommits if too much
void CMTSafeHeap::Compact()
{
#if !defined(LINUX)//empty
	#if !USE_CRT
		HeapCompact(m_hHeap, 0);
	#else
		_heapmin();
	#endif
#endif
}

void* CMTSafeHeap::TryAlloc(size_t nSize, const char* szDbgSource = NULL)
{
#if !USE_CRT
	return HeapAlloc (m_hHeap, 0, nSize);
#else
#if !defined(LINUX) && defined(_DEBUG) && defined(_INC_CRTDBG) && defined(WIN32)
	return _malloc_dbg (nSize, _NORMAL_BLOCK, szDbgSource, 0);
#else
	return ::malloc (nSize);
#endif
#endif
}

void* CMTSafeHeap::Alloc (size_t nSize, const char* szDbgSource = NULL)
{
	InterlockedIncrement(&m_numAllocations);
	void *pResult = TryAlloc(nSize, szDbgSource);
	if (!pResult)
	{
		Compact();
		pResult = TryAlloc(nSize, szDbgSource);
	}

	if (!pResult)
		InterlockedDecrement(&m_numAllocations);
	return pResult;
}

void CMTSafeHeap::Free (void*p)
{
	if (p)
	{
		BOOL bFreed;
#if !USE_CRT
		bFreed = HeapFree (m_hHeap, 0, p);
#else
#if !defined(LINUX) && defined(_DEBUG) && defined(_INC_CRTDBG) && defined(WIN32)
		_free_dbg (p, _NORMAL_BLOCK);
#else
		::free (p);
#endif
		bFreed = true;
#endif
		InterlockedDecrement(&m_numAllocations);
		assert (bFreed);
	}
}
