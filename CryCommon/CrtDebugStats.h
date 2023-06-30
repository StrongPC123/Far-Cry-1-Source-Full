// support for leak dumping and statistics gathering using vs Crt Debug
// should be included in every DLL below DllMain()
#ifdef WIN32

#ifdef _DEBUG

#include <ILog.h>
#include <crtdbg.h>
#include <algorithm>
#include <vector>

// copied from DBGINT.H (not a public header!)

#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
        struct _CrtMemBlockHeader * pBlockHeaderNext;
        struct _CrtMemBlockHeader * pBlockHeaderPrev;
        char *                      szFileName;
        int                         nLine;
        size_t                      nDataSize;
        int                         nBlockUse;
        long                        lRequest;
        unsigned char               gap[nNoMansLandSize];
        /* followed by:
         *  unsigned char           data[nDataSize];
         *  unsigned char           anotherGap[nNoMansLandSize];
         */
} _CrtMemBlockHeader;

struct SFileInfo
{
	int blocks; INT_PTR bytes;							//AMD Port
	SFileInfo(INT_PTR b) { blocks = 1; bytes = b; };	//AMD Port
};

_CrtMemState lastcheckpoint;
bool checkpointset = false;

extern "C" void __declspec(dllexport) CheckPoint()
{
	_CrtMemCheckpoint(&lastcheckpoint); 
	checkpointset = true;
};

bool pairgreater( const std::pair<string, SFileInfo> &elem1, const std::pair<string, SFileInfo> &elem2)
{
   return elem1.second.bytes > elem2.second.bytes;
}

extern "C" void __declspec(dllexport) UsageSummary(ILog *log, char *modulename, int *extras)
{
	_CrtMemState state;
	
	if(checkpointset)
	{
		_CrtMemState recent;
		_CrtMemCheckpoint(&recent);
		_CrtMemDifference(&state, &lastcheckpoint, &recent);
	}
	else
	{
		_CrtMemCheckpoint(&state);
	};

	INT_PTR numblocks = state.lCounts[_NORMAL_BLOCK];	//AMD Port
	INT_PTR totalalloc = state.lSizes[_NORMAL_BLOCK];	//AMD Port

	extras[0] = totalalloc;
	extras[1] = numblocks;

	log->Log("\001$5---------------------------------------------------------------------------------------------------");

    if(!numblocks)
    {
	    log->Log("\001$3Module %s has no memory in use", modulename);
	    return;
    };

	log->Log("\001$5Usage summary for module %s", modulename);
	log->Log("\001%d kbytes (peak %d) in %d objects of %d average bytes\n",
		totalalloc/1024, state.lHighWaterCount/1024, numblocks, numblocks ? totalalloc/numblocks : 0);
	log->Log("\001%d kbytes allocated over time\n", state.lTotalCount/1024);
	
	typedef std::map<string, SFileInfo> FileMap;
	FileMap fm;
	
    for(_CrtMemBlockHeader *h = state.pBlockHeader; h; h = h->pBlockHeaderNext)
    {
		if(_BLOCK_TYPE(h->nBlockUse)!=_NORMAL_BLOCK) continue;
		string s = h->szFileName ? h->szFileName : "NO_SOURCE";
		if (h->nLine > 0)
		{
			char buf[16];
			sprintf(buf, "_%d", h->nLine);
			s += buf;
		}
		FileMap::iterator it = fm.find(s);
		if(it!=fm.end())
		{
			(*it).second.blocks++;
			(*it).second.bytes += h->nDataSize;
		}
		else
		{
			fm.insert(FileMap::value_type(s, SFileInfo(h->nDataSize)));
		};
    };

	typedef std::vector< std::pair<string, SFileInfo> > FileVector;
	FileVector fv;
	for(FileMap::iterator it = fm.begin(); it!=fm.end(); ++it) fv.push_back((*it));
	std::sort(fv.begin(), fv.end(), pairgreater);
	
	for(FileVector::iterator it = fv.begin(); it!=fv.end(); ++it)
	{
		log->Log("\001%6d kbytes / %6d blocks allocated from %s\n",
			(*it).second.bytes/1024, (*it).second.blocks, (*it).first.c_str());
	};
	
	if(extras[2])
	{
	    OutputDebugString("---------------------------------------------------------------------------------------------------\n");
	    OutputDebugString(modulename);
	    OutputDebugString("\n\n");
	    _CrtDumpMemoryLeaks();
	    OutputDebugString("\n\n");
	};
};

#endif

#if !defined(_RELEASE) && !defined(_DLL)
extern "C" HANDLE _crtheap;
extern "C" HANDLE __declspec(dllexport) GetDLLHeap() { return _crtheap; };
#endif

#endif //WIN32
