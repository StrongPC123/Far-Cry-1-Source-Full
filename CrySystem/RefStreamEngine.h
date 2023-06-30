//////////////////////////////////////////////////////////////////////////
// Reference implementation of streamEngine

#ifndef _CRY_SYSTEM_REFERENCE_STREAM_ENGINE_HDR_
#define _CRY_SYSTEM_REFERENCE_STREAM_ENGINE_HDR_
#include "IMiniLog.h"
#include "IStreamEngine.h"
#include "RefReadStream.h"
#include "RefReadStreamProxy.h"
#include "CritSection.h"
#include "MTSafeAllocator.h"
#include "CryPak.h"

struct ICryPak;


class CRefStreamEngine:public IStreamEngine
{
public:
	//! useWorkerThreads is the number of worker threads  to use;
	//! currently only values 0 and 1 are supported: 0 - overlapped IO in the main thread, and 1 - overlapped IO in the worker thread
	CRefStreamEngine(CCryPak* pPak, IMiniLog* pLog, unsigned useWorkerThreads = 1, bool bOverlappedIO = true);

	//! destructor
	~CRefStreamEngine();

	// interface IStreamEngine --------------------------------

	virtual DWORD GetStreamCompressionMask() const;

	virtual IReadStream_AutoPtr StartRead (const char* szSource, const char* szFile, IStreamCallback* pCallback, StreamReadParams* pParams = NULL);

	virtual unsigned GetFileSize (const char* szFile, unsigned nCryPakFlags);

	virtual void Update(unsigned nFlags);

	virtual unsigned Wait(unsigned nMilliseconds, unsigned nFlags);

	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	// ------------------------------------------------

	//! registers a new stream (added to the system: queued)
	void Register (CRefReadStream*);
	//! unregisters: happens upon release of all resources
	void Unregister (CRefReadStream*);

	CCryPak* GetPak() {return m_pPak;}

	unsigned UpdateAndWait (unsigned nMilliseconds, unsigned nFlags);

#ifndef LINUX
	// returns the (cached) size of the sector on the volume where the given path points
	unsigned GetSectorSize(const char* szPath);
	unsigned GetDriveSectorSize (char cDrive);
#endif //LINUX

	// sort the IO jobs in the IOQueue by priority
	void SortIOJobs();
	void OnIOJobExecuted (CRefReadStreamProxy* pJobProxy);

	bool IsSuspended();

	// returns true, if overlapped io with FILE_FLAG_OVERLAPPED and ReadFileEx is enabled
	bool isOverlappedIoEnabled() {return m_bEnableOverlapped;}

	enum IOJobKindEnum{eWaiting, ePending, eExecuted};
	unsigned numIOJobs(IOJobKindEnum nKind);

	//!
	void SetCallbackTimeQuota(int nMicroseconds);

	//!
	void SetStreamCompressionMask( const DWORD indwMask );

	// returns true if called from the main thread for this engine
	bool IsMainThread();

	// returns true if called from the worker thread
	bool IsWorkerThread();
protected:

	// this function checks for the OS version and disables some capabilities of Streaming Engine when needed
	// currently, in Win 9x overlapped IO is disabled
	void CheckOSCaps();

	// this sorts the IO jobs, without bothering about synchronization
	void SortIOJobs_NoLock();
	// this will be the thread that executes everything that can take time
	void IOWorkerThread ();

	// the static function used to start the thread
#if defined(LINUX)
	static void* WINAPI IOWorkerThreadProc (LPVOID pThis)
	{
		((CRefStreamEngine*)pThis)->IOWorkerThread();
		pthread_exit(NULL);//finish thread
		return NULL;
	}
#else
	static DWORD WINAPI IOWorkerThreadProc (LPVOID pThis)
	{
		((CRefStreamEngine*)pThis)->IOWorkerThread();
		return 0;
	}
#endif

	void StartWorkerThread();
	void StopWorkerThread();

	// signals that this proxy needs to be executed (StartRead called)
	void AddIOJob (CRefReadStreamProxy* pJobProxy);

	// executes StartRead on the proxies that need it, and remove those proxies
	// from the IO Queue. As soon as the proxy is removed from the IO Queue, it's on its own
	// and should complete the operation itself and unregister upon destruction
	// returns the number of jobs moved from the IO Job QUeue
	unsigned StartIOJobs();

	// In the Multi-Threaded model only (with the IO Worker thread)
	// removes the proxies from the IO Queue as needed, and the proxies may call their callbacks
	// returns the number of finalized jobs
	unsigned FinalizeIOJobs(unsigned nFlags);

	// adds to the callback time quota in this frame, the specified number of microseconds
	void AddCallbackTimeQuota(int nMicroseconds);
	bool IsCallbackTimeQuota(unsigned nFlags);

	//! Enables or disables callback time quota per frame
	void SuspendCallbackTimeQuota();
	void ResumeCallbackTimeQuota();

protected:

	DWORD				m_dwMask;			//!< default: 0, bitmask that affects the stram compression (look into Stream.h)

	int64			m_nPerfFreq;
	int64			m_nCallbackTimeQuota;

	CCryPak* m_pPak;
	// the set of file objects (not deleted yet)
	typedef std::map<string, CRefReadStream*> NameStreamMap;
	NameStreamMap m_mapFilesByName;

	typedef std::set<CRefReadStream_AutoPtr> CRefReadStream_AutoSet;
	CRefReadStream_AutoSet m_setLockedStreams;

	IMiniLog* m_pLog;

	// these are the cached sizes of sectors of disks, starting from c
	// if the number contained here is 0, it means the sector size wasn't cached
	unsigned m_nSectorSizes[8];

	// the max number of simultaneous reads
	unsigned m_nMaxReadDepth;

	// the max length of the IO Job queue: when the max queue length is reached, the next StartRead
	// will wait until it's shrinked
	unsigned m_nMaxQueueLength;

	// the max size of the allocated memory from the big pool
	unsigned m_nMaxIOMemPool;

	// this is the set of proxies that need to be started (StartRead needs to be called)
	// These proxies can be rearranged, added to or removed by the main thread;
	// protected by m_csMainWriter
	typedef CMTSafeAllocator<CRefReadStreamProxy_AutoPtr> ProxyPtrAllocator;
	typedef std::less<CRefReadStreamProxy_AutoPtr> ProxyPtrPredicate;
	typedef std::deque<CRefReadStreamProxy_AutoPtr, ProxyPtrAllocator > CRefReadStreamProxy_AutoDeque_MT;
	CRefReadStreamProxy_AutoDeque_MT m_queIOJobs;
	CCritSection m_csIOJobs;

	typedef std::set<CRefReadStreamProxy_AutoPtr, ProxyPtrPredicate, ProxyPtrAllocator> CRefReadStreamProxy_AutoSet_MT;
	CRefReadStreamProxy_AutoSet_MT m_setIOPending;
	CCritSection m_csIOPending;

	// the event used to signal the worker thread that a new job arrived
	// the job can be: ask to suspend, ask to read, ask to stop or basically anything that needs attention of the worker thread
	EVENT_HANDLE m_hIOJob;//EVENT_HANDLE is a typedef to HANDLE under windows

	// signals about arrival of executed jobs. This event is MANUAL-reset
	EVENT_HANDLE m_hIOExecuted;//EVENT_HANDLE is a typedef to HANDLE under windows

	// the set of proxies awaiting finalization in the main thread
	// this is the number of proxes from m_arrSuspendedProxies
	// they need attention of the main thread: e.g. they have
	// already finished their operations and need to be removed,
	// or they need memory allocated from the main thread.
	CRefReadStreamProxy_AutoDeque_MT m_queIOExecuted;
	CCritSection m_csIOExecuted;

	// the handle to the worker thread, or NULL if single-threaded overlapped IO is used
	THREAD_HANDLE m_hIOWorker;//THREAD_HANDLE is a typedef to HANDLE under windows

	// this event is never signaled
	EVENT_HANDLE m_hDummyEvent;//EVENT_HANDLE is a typedef to HANDLE under windows

	// this flag is set to stop the worker thread
	bool m_bStopIOWorker;

	// this flag enables or disables usage of really asynchronous file io (ReadFileEx and FILE_FLAG_OVERLAPPED)
	bool m_bEnableOverlapped;

	// this flag is set if the callback time quota is enabled
	int m_nSuspendCallbackTimeQuota;
	
	// this is the id of the main thread in which this engine operates
	DWORD m_dwMainThreadId;
	// the id of the worker thread, if any 
	DWORD m_dwWorkerThreadId;

	// This critical section protects the objects that can be written to by the main thread only
	// It must be locked for the time of access from non-main thread and for the time of writing from the main thread
	// Main thread can have read access to those objects anytime without serialization
	//CCritSection m_csMainWriter;
};


#endif