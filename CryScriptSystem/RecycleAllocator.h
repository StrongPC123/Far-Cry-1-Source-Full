// really fast recycling allocator (for use with Lua), by Wouter

#ifdef _DEBUG

#ifdef __cplusplus

class RecyclePool
{
    enum { POOLSIZE = 4096 }; // can be absolutely anything
    enum { PTRSIZE = sizeof(char *) };
    enum { MAXBUCKETS = 65 }; // meaning up to size 256 on 32bit pointer systems
	enum { MAXREUSESIZE = MAXBUCKETS*PTRSIZE-PTRSIZE };
    //int roundup(int s) { return (s+(PTRSIZE-1))&(~(PTRSIZE-1)); };
	int bucket(int s) { return (s+PTRSIZE-1)>>PTRBITS; };
    enum { PTRBITS = PTRSIZE==2 ? 1 : PTRSIZE==4 ? 2 : 3 };

    char *p;
    int left;
    char *blocks;
    void *reuse[MAXBUCKETS];

    public:

    RecyclePool();
    ~RecyclePool() { dealloc_block(blocks); };

    void *alloc(int size);
    void dealloc(void *p, int size);
	void *realloc(void *p, int oldsize, int newsize);
    void stats();

	private:

    void dealloc_block(void *b);
    void allocnext(int allocsize);
};

extern "C" {
#endif

void *recycle_realloc(void *p, int oldsize, int newsize);
void recycle_free(void *p, int size);
void recycle_cleanup();
void recycle_stats();

#ifdef __cplusplus
}
#endif

#endif