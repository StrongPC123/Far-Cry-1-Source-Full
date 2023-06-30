// CryMemoryManager.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"


#include <stdlib.h>
#include <stdio.h>
#include <new.h>

#include <ISystem.h>

#include "platform.h"


#undef USE_NEWPOOL
#include <CryMemoryManager.h>

//////////////////////////////////////////////////////////////////////////
// Some globals for fast profiling.
//////////////////////////////////////////////////////////////////////////
size_t g_TotalAllocatedMemory = 0;
size_t g_ScriptAllocatedMemory = 0;
int g_nPrecaution = 0; // will cause delayed crash, will make engine extremally unstable.

//#if !defined(LINUX)

extern ISystem* g_System;
extern bool g_bProfilerEnabled;

//////////////////////////////////////////////////////////////////////////

// Undefine malloc for memory manager itself..
#undef malloc
#undef realloc
#undef free

 
#ifndef _XBOX

#define GLOBALPOOLSIZE (32*1025*1025+16)
#define EXTRAPOOLSIZE (32*1025*1025+16)    // how much to allocate if we allow the pool to be exceeded
// currently biggest allocation happends during hires screenshot creation

#else // _XBOX

#ifndef _DEBUG
#define GLOBALPOOLSIZE (8*1025*1025+16)
#define EXTRAPOOLSIZE (8*1025*1025+16)
#else // _DEBUG
#define GLOBALPOOLSIZE (0)
#define EXTRAPOOLSIZE (0)
#endif // _DEBUG

#endif // _XBOX



#define MAXPOOLS 128
void *poolbufs[MAXPOOLS];
int poolsizes[MAXPOOLS];
int numpools = 0;



#if defined(WIN32) || defined(LINUX)
#if defined(WIN64) || defined(LINUX64)
// non-512 doesn't work for some reason: an infinite loop (recursion with stack overflow) occurs
#define BUCKETQUANT 512    // wouter: affects performance of bucket allocator, modify with care!
#else
#define BUCKETQUANT 512*2
#endif
#else
#define BUCKETQUANT 256		// save some memory overhead with tiny speed cost on consoles
#endif

/*
PoolContext *AllocPool(PoolContext *pCtx, int size)
{
	// TODO: replace "malloc" with however we obtain memory on other platforms
	void *buf = VirtualAlloc(NULL,size,MEM_COMMIT,PAGE_READWRITE);
	if(!buf)
	{
		CryError( "<CrySystem> (AllocPool) malloc() Failed" );
	};
	bpool(pCtx, buf, size);
	if (numpools==MAXPOOLS)
	{
		CryError( "<CrySystem> (AllocPool) Maximum number of memory pools reached." );
	}
	poolsizes[numpools] = size;
	poolbufs[numpools++] = buf;
	return pCtx; 
};
*/

//static PoolContext globalpool;
//static PoolContext *g_pool = AllocPool(InitPoolContext(&globalpool), GLOBALPOOLSIZE);
static unsigned int biggestalloc = 0;
/*
#define MAXSTAT 1000
static int stats[MAXSTAT];
void addstat(int size) { if(size<0 || size>=MAXSTAT) size = MAXSTAT-1; stats[size]++; };
void printstats() { for(int i = 0; i<MAXSTAT; i++) if(stats[i]) { char buf[100]; sprintf(buf, "bucket %d -> %d\n", i, stats[i]); ::OutputDebugString(buf); }; };
int clearstats() { for(int i = 0; i<MAXSTAT; i++) stats[i] = 0; return 0; };
static int foo = clearstats();
*/

class PageBucketAllocator
{
	/*
	Generic allocator that combines bucket allocation with reference counted 1 size object pages.
	manages to perform well along each axis:
	- very fast for small objects: only a few instructions in the general case for alloc/dealloc,
	up to several orders of magnitude faster than traditional best fit allocators
	- low per object memory overhead: 0 bytes overhead on small objects, small overhead for
	pages that are partially in use (still significantly lower than other allocators).
	- almost no fragmentation, reuse of pages is close to optimal
	- very good cache locality (page aligned, same size objects)
	*/
	enum { PAGESIZE = 4096 };
	enum { PAGEMASK = (~(PAGESIZE-1)) };
	//enum { PAGESATONCE = 64 };
	enum { PAGESATONCE = 32 };
	enum { PAGEBLOCKSIZE = PAGESIZE*PAGESATONCE };
	enum { PTRSIZE = sizeof(char *) };
	enum { MAXBUCKETS = BUCKETQUANT/4+1 }; // meaning up to size 512 on 32bit pointer systems
	enum { MAXREUSESIZE = MAXBUCKETS*PTRSIZE-PTRSIZE };
	int bucket(int s) { return (s+PTRSIZE-1)>>PTRBITS; };
	int *ppage(void *p) { return (int *)(((INT_PTR)p)&PAGEMASK); };
	enum { PTRBITS = PTRSIZE==2 ? 1 : PTRSIZE==4 ? 2 : 3 };

	void *reuse[MAXBUCKETS];
	void **pages;
	//! Total allocated size.

	void putinbuckets(char *start, char *end, int bsize)
	{
		int size = bsize*PTRSIZE;        
		for(end -= size; start<=end; start += size)
		{
			*((void **)start) = reuse[bsize];
			reuse[bsize] = start;
		};
	};

	void newpageblocks()
	{
		char *b = (char *)::malloc(PAGEBLOCKSIZE); // if we could get page aligned memory here, that would be even better
		char *first = ((char *)ppage(b))+PAGESIZE;
		for(int i = 0; i<PAGESATONCE-1; i++)
		{
			void **p = (void **)(first+i*PAGESIZE);
			*p = pages;
			pages = p;
		};
		//if(b-first+PAGESIZE>BUCKETQUANT) bpool(g_pool, first+PAGEBLOCKSIZE-PAGESIZE, b-first+PAGESIZE);
	};

	void *newpage(unsigned int bsize)
	{
		if(!pages) newpageblocks();
		void **page = pages;
		pages = (void **)*pages;
		*page = 0;
		putinbuckets((char *)(page+1), ((char *)page)+PAGESIZE, bsize);
		return alloc(bsize*PTRSIZE);
	};

	void freepage(int *page, int bsize) // worst case if very large amounts of objects get deallocated in random order from when they were allocated
	{
		for(void **r = &reuse[bsize]; *r; )
		{
			if(page == ppage(*r)) *r = *((void **)*r);
			else r = (void **)*r;
		};
		void **p = (void **)page;
		*p = pages;
		pages = p;
	};

public:

	PageBucketAllocator()
	{
		pages = NULL;
		for(int i = 0; i<MAXBUCKETS; i++) reuse[i] = NULL;
	};

	void *alloc(unsigned int size)
	{
		if(size>biggestalloc)
		{
			biggestalloc = size;
		};
		if(size>MAXREUSESIZE) return ::malloc(size);
		size = bucket(size);
		void **r = (void **)reuse[size];
		if(!r) return newpage(size);
		reuse[size] = *r;
		int *page = ppage(r);
		(*page)++;
		return (void *)r;
	};

	void dealloc(void *p, unsigned int size)
	{
		if(size>MAXREUSESIZE)
		{
			::free(p);
			//brel(g_pool, p);
		}
		else
		{
			size = bucket(size);
			*((void **)p) = reuse[size];
			reuse[size] = p;
			int *page = ppage(p);
			if(!--(*page)) freepage(page, size);
		};
	};

	void stats()
	{
		int totalwaste = 0;
		char buf[100];
		for(int i = 0; i<MAXBUCKETS; i++)
		{
			int n = 0;
			for(void **r = (void **)reuse[i]; r; r = (void **)*r) n++;
			if(n)
			{
				int waste = i*4*n/1024;
				totalwaste += waste;
				sprintf(buf, "bucket %d -> %d (%d k)\n", i*4, n, waste);
				::OutputDebugString(buf); 
			};
		};
		sprintf(buf, "totalwaste %d k\n", totalwaste);
		::OutputDebugString(buf); 
	};
};

PageBucketAllocator g_GlobPageBucketAllocator;

CRYMEMORYMANAGER_API void *CryMalloc(size_t size)
{
	if (!g_bProfilerEnabled)
	{
		g_TotalAllocatedMemory += size+sizeof(int);
		int *p = (int *)g_GlobPageBucketAllocator.alloc(size+sizeof(int));
		*p++ = size;  // stores 2 sizes for big objects!
		return p;
	}
	else
	{
		FUNCTION_PROFILER_FAST( g_System,PROFILE_SYSTEM,g_bProfilerEnabled );
		g_TotalAllocatedMemory += size+sizeof(int);
		int *p = (int *)g_GlobPageBucketAllocator.alloc(size+sizeof(int));
		*p++ = size;  // stores 2 sizes for big objects!
		return p;
	}
}

CRYMEMORYMANAGER_API void CryFree(void *p) 
{
	if (!g_bProfilerEnabled)
	{
		if (p != NULL)
		{
			unsigned int *t = (unsigned int *)p;
			unsigned int size = *--t;	

#ifdef MINIMALDEBUG
			if (size>=100000000)
			{
				CryError( "\001[CRYMANAGER ERROR](CryFree): illegal size 0x%X - block header was corrupted",size);
			}
#endif

			size += sizeof(int);
#ifdef GARBAGEMEMORY     //FIXME: *disabling* memset caused random crash???
			memset(t, 0xBA, size); 
#endif
			g_TotalAllocatedMemory -= size;
			g_GlobPageBucketAllocator.dealloc(t, size);
		}
	}
	else
	{
		// With profiler.
		if (p != NULL)
		{
			FUNCTION_PROFILER_FAST( g_System,PROFILE_SYSTEM,g_bProfilerEnabled );
			unsigned int *t = (unsigned int *)p;
			unsigned int size = *--t;	

#ifdef MINIMALDEBUG
			if (size>=100000000)
			{
				CryError( "\001[CRYMANAGER ERROR](CryFree): illegal size 0x%X - block header was corrupted",size);
			}
#endif

			size += sizeof(int);
#ifdef GARBAGEMEMORY     //FIXME: *disabling* memset caused random crash???
			memset(t, 0xBA, size); 
#endif
			g_TotalAllocatedMemory -= size;
			g_GlobPageBucketAllocator.dealloc(t, size);
		}
	}
}

CRYMEMORYMANAGER_API void CryFreeSize(void *p, size_t size)
{
	g_ScriptAllocatedMemory -= size;
	g_TotalAllocatedMemory -= size;
	if (!g_bProfilerEnabled)
	{
		if (p != NULL)
		{
#ifdef MINIMALDEBUG
			if (size>=100000000)
			{
				CryError("[CRYMANAGER ERROR](CryFreeSize): illegal size 0x%X - block header was corrupted",size );
			}
#endif

#ifdef GARBAGEMEMORY      //FIXME: idem
			memset(p, 0xBB, size); 
#endif
			g_GlobPageBucketAllocator.dealloc(p, size);
		}
	}
	else
	{
		// With profiler.
		if (p != NULL)
		{
			FUNCTION_PROFILER_FAST( g_System,PROFILE_SYSTEM,g_bProfilerEnabled );
#ifdef MINIMALDEBUG
			if (size>=100000000)
			{
				CryError("[CRYMANAGER ERROR](CryFreeSize): illegal size 0x%X - block header was corrupted",size );
			}
#endif

#ifdef GARBAGEMEMORY      //FIXME: idem
			memset(p, 0xBB, size); 
#endif
			g_GlobPageBucketAllocator.dealloc(p, size);
		}
	}
}

CRYMEMORYMANAGER_API void *CryRealloc(void *memblock, size_t size)
{
	if (!g_bProfilerEnabled)
	{
		// Without profiler.
		if(memblock==NULL)
			return CryMalloc(size);
		else
		{
			void *np = CryMalloc(size);
			size_t oldsize = ((int *)memblock)[-1];
			memcpy(np, memblock, size>oldsize ? oldsize : size);
			CryFree(memblock);
			return np;
		}
	}
	else
	{
		// With Profiler.
		FUNCTION_PROFILER_FAST( g_System,PROFILE_SYSTEM,g_bProfilerEnabled );
		if(memblock==NULL)
			return CryMalloc(size);
		else
		{
			void *np = CryMalloc(size);
			size_t oldsize = ((int *)memblock)[-1];
			memcpy(np, memblock, size>oldsize ? oldsize : size);
			CryFree(memblock);
			return np;
		}
	}
}

CRYMEMORYMANAGER_API void *CryReallocSize(void *memblock, size_t oldsize, size_t size)
{
	g_ScriptAllocatedMemory += size; // -old size done in CryFreeSize
	g_TotalAllocatedMemory += size;
	if (!g_bProfilerEnabled)
	{
		if(memblock==NULL)
		{
			return (char*)g_GlobPageBucketAllocator.alloc(size) + g_nPrecaution;
		}
		else
		{
			void *np = (char*)g_GlobPageBucketAllocator.alloc(size) + g_nPrecaution;
			memcpy(np, memblock, size>oldsize ? oldsize : size);
			CryFreeSize(memblock, oldsize);
			return np;
		}
	}
	else
	{
		FUNCTION_PROFILER_FAST( g_System,PROFILE_SYSTEM,g_bProfilerEnabled );
		if(memblock==NULL)
		{
			return (char*)g_GlobPageBucketAllocator.alloc(size) + g_nPrecaution;
		}
		else
		{
			void *np = (char*)g_GlobPageBucketAllocator.alloc(size) + g_nPrecaution;
			memcpy(np, memblock, size>oldsize ? oldsize : size);
			CryFreeSize(memblock, oldsize);
			return np;
		}
	}
}

CRYMEMORYMANAGER_API void CryFlushAll()  // releases/resets ALL memory... this is useful for restarting the game
{
	/*
	InitPoolContext(g_pool);
	for(int i = 0; i<numpools; i++) 
		bpool(g_pool, poolbufs[i], poolsizes[i]);
		*/
	new (&g_GlobPageBucketAllocator) PageBucketAllocator();
	g_ScriptAllocatedMemory = 0;
	g_TotalAllocatedMemory = 0;
};

/* MarcoK: This is never used anywhere ... commented out (LINUX port)
extern "C" CRYMEMORYMANAGER_API void CryFreeMemoryPools()  // releases/resets ALL memory... this is useful for restarting the game
{
	for(int i = 0; i<numpools; i++) 
	{
		void *pBuf = poolbufs[i];
		VirtualFree( pBuf,0,MEM_RELEASE );
	}
	numpools = 0;
	g_ScriptAllocatedMemory = 0;
	g_TotalAllocatedMemory = 0;
};
*/

//////////////////////////////////////////////////////////////////////////
// Returns ammount of memory allocated with CryMalloc/CryFree functions.
//////////////////////////////////////////////////////////////////////////
CRYMEMORYMANAGER_API int CryMemoryGetAllocatedSize()
{
	return g_TotalAllocatedMemory;
}

//////////////////////////////////////////////////////////////////////////
CRYMEMORYMANAGER_API int CryMemoryGetAllocatedInScriptSize()
{
	return g_ScriptAllocatedMemory;
}

//////////////////////////////////////////////////////////////////////////
CRYMEMORYMANAGER_API int CryMemoryGetPoolSize()
{
	int totalsize = 0;
	for(int i = 0; i<numpools; i++) 
		totalsize += poolsizes[i];
	return totalsize;
}

//////////////////////////////////////////////////////////////////////////
CRYMEMORYMANAGER_API int CryStats(char *buf)
{
	long curalloc=0, totfree=0, maxfree=0, nget=0, nrel=0;
	//bstats(g_pool, &curalloc, &totfree, &maxfree, &nget, &nrel);
	if(buf)
	{
		int poolsize = CryMemoryGetPoolSize();
		int scriptalloc = CryMemoryGetAllocatedInScriptSize();
		sprintf(buf, "Memory Allocated = %d K, totfree = %d K , maxfree = %d K, nmalloc = %d, nfree = %d, biggestalloc = %d, Pool Size = %d K, Lua Allocated = %d K",
			curalloc/1024, totfree/1024, maxfree/1024, nget, nrel, biggestalloc,poolsize/1024,scriptalloc/1024);
		//printstats();
		g_GlobPageBucketAllocator.stats();
	};
	return curalloc/1024;
}

/*
extern "C" void debug(int n)
{
	char buf[100];
	sprintf(buf, "BESTFIT: %d\n", n);  
	::OutputDebugString(buf); 
};
*/// CryMemoryManager.cpp : Defines the entry point for the DLL application.
//#endif //LINUX
