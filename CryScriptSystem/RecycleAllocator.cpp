// really fast recycling allocator (for use with Lua), by Wouter

#include "StdAfx.h"

#ifdef _DEBUG

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "RecycleAllocator.h"

RecyclePool::RecyclePool()
{
    blocks = 0;
    allocnext(POOLSIZE);
    for(int i = 0; i<MAXBUCKETS; i++) reuse[i] = NULL;
};

void *RecyclePool::alloc(int size)
{
	if(size>MAXREUSESIZE)
	{
	    //char buf[256];
	    //sprintf(buf, "recyclealloc %d\n", size);
		//::OutputDebugString(buf);
		return malloc(size);
	}
	else
	{
		size = bucket(size);
        void **r = (void **)reuse[size];
        if(r)
        {
            reuse[size] = *r;
            return (void *)r;
        }
		else
		{
			size <<= PTRBITS;
			if(left<size) allocnext(POOLSIZE);
			char *r = p;
			p += size;
			left -= size;
			return r;
		};
	};
};

void RecyclePool::dealloc(void *p, int size)
{
	if(size>MAXREUSESIZE)
	{
		free(p);
	}
	else
	{
		size = bucket(size);
		if(size)	// only needed for 0-size free, are there any?
		{
			*((void **)p) = reuse[size];
			reuse[size] = p;
		};
	};
};

void *RecyclePool::realloc(void *p, int oldsize, int newsize)
{
	void *np = alloc(newsize);
	if(!oldsize) return np;
	memcpy(np, p, newsize>oldsize ? oldsize : newsize);
	dealloc(p, oldsize);
	return np;
};

void RecyclePool::dealloc_block(void *b)
{
    if(b)
    {
        dealloc_block(*((char **)b));
        free(b);
    };
}

void RecyclePool::allocnext(int allocsize)
{
    char *b = (char *)malloc(allocsize+PTRSIZE);
    *((char **)b) = blocks;
    blocks = b;
    p = b+PTRSIZE;
    left = allocsize;
};

void RecyclePool::stats()
{
    int totalwaste = 0;
    for(int i = 0; i<MAXBUCKETS; i++)
    {
        int n = 0;
        for(void **r = (void **)reuse[i]; r; r = (void **)*r) n++;
        if(n)
        {
            char buf[100];
            int waste = i*4*n/1024;
            totalwaste += waste;
            sprintf(buf, "bucket %d -> %d (%d k)\n", i*4, n, waste);
            ::OutputDebugString(buf); 
        };
    };
    char buf[100];
    sprintf(buf, "totalwaste %d k\n", totalwaste);
    ::OutputDebugString(buf); 
};

RecyclePool *g_pLuaRecyclePool = new RecyclePool();		// TODO: add to CScriptSystem ?

void recycle_cleanup() { if(g_pLuaRecyclePool) delete g_pLuaRecyclePool; };
void *recycle_realloc(void *p, int oldsize, int newsize) { return g_pLuaRecyclePool->realloc(p, oldsize, newsize); };
void recycle_free(void *p, int size) { g_pLuaRecyclePool->dealloc(p, size); };
void recycle_stats() { g_pLuaRecyclePool->stats(); };

#endif
