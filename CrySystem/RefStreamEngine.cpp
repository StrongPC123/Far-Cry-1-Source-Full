#include "stdafx.h"
#include <TArrays.h>
#include <ilog.h>
#include "RefStreamEngine.h"
#include "RefReadStream.h"
#include "RefReadStreamProxy.h"

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif
extern CMTSafeHeap* g_pSmallHeap;
extern CMTSafeHeap* g_pBigHeap;

//////////////////////////////////////////////////////////////////////////
// useWorkerThreads is the number of worker threads  to use;
// currently only values 0 and 1 are supported: 0 - overlapped IO in the main thread, and 1 - overlapped IO in the worker thread
// MT: Main thread only
CRefStreamEngine::CRefStreamEngine (CCryPak* pPak, IMiniLog* pLog, unsigned useWorkerThreads, bool bOverlappedIO):
	m_pPak(pPak),
	m_pLog(pLog),
	m_nMaxReadDepth (16),
	m_nMaxQueueLength (4*1024),
	m_nMaxIOMemPool (128*1024*1024),
#if defined(LINUX)
	m_hIOWorker (INVALID_HANDLE_VALUE),//only diff is here, but what can i do?
#else
	m_hIOWorker (NULL),
#endif
	m_dwWorkerThreadId(0),
	m_queIOJobs(ProxyPtrAllocator(g_pSmallHeap)),
	m_setIOPending(ProxyPtrPredicate(), ProxyPtrAllocator(g_pSmallHeap)),
	m_queIOExecuted(ProxyPtrAllocator(g_pSmallHeap)),
	m_bEnableOverlapped (bOverlappedIO),
	m_nSuspendCallbackTimeQuota(0)
{
	m_dwMainThreadId = GetCurrentThreadId();
	CheckOSCaps();

	if (!QueryPerformanceFrequency((LARGE_INTEGER*)&m_nPerfFreq))
	{
		m_nPerfFreq = 0;
		m_nSuspendCallbackTimeQuota = 1; // suspend forever
	}

	//m_nSuspendCallbackTimeQuota = 1; // suspend anyway: we don't support this..
	m_nCallbackTimeQuota = 0;
	SetCallbackTimeQuota (50000);
	m_dwMask=0;

	m_hIOJob = CreateEvent (NULL, FALSE, FALSE, NULL);
	m_hIOExecuted = CreateEvent (NULL, TRUE, FALSE, NULL);
	m_hDummyEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
	memset (m_nSectorSizes, 0, sizeof(m_nSectorSizes));

	if (useWorkerThreads)
		StartWorkerThread();
}

//////////////////////////////////////////////////////////////////////////
// MT: Main thread only
CRefStreamEngine::~CRefStreamEngine()
{
	StopWorkerThread();

	m_setLockedStreams.clear();

	// fail all outstanding requests
	// we don't need to lock, since there's already no worker thread at this moment
	while (!m_queIOJobs.empty())
	{
		CRefReadStreamProxy_AutoPtr pJob = m_queIOJobs.front();
		m_queIOJobs.pop_front();
		pJob->OnFinishRead(ERROR_ABORTED_ON_SHUTDOWN); // aborted
	}

	// finalize all the jobs that can be finalized
	while (!m_queIOExecuted.empty() && FinalizeIOJobs(FLAGS_DISABLE_CALLBACK_TIME_QUOTA) > 0)
		continue;

	// if we still have some streams, that's because someone didn't release the proxy
	if (m_pLog)
		for (NameStreamMap::iterator it = m_mapFilesByName.begin(); it != m_mapFilesByName.end(); ++it)
			m_pLog->Log("%s: %s", it->first.c_str(), it->second->Dump().c_str());

	CloseHandle (m_hIOJob);
	CloseHandle (m_hIOExecuted);
	CloseHandle (m_hDummyEvent);
}

unsigned CRefStreamEngine::UpdateAndWait (unsigned nMilliseconds, unsigned nFlags)
{
	if (IsMainThread())
		Update(nFlags);
	return Wait (nMilliseconds,nFlags);
}

// returns true if called from the main thread for this engine
bool CRefStreamEngine::IsMainThread()
{
	return GetCurrentThreadId() == m_dwMainThreadId;
}

bool CRefStreamEngine::IsWorkerThread()
{
	return GetCurrentThreadId() == m_dwWorkerThreadId;
} 

//////////////////////////////////////////////////////////////////////////
// Starts asynchronous read from the specified file
// MT: Main thread only
IReadStream_AutoPtr CRefStreamEngine::StartRead (const char* szSource, const char* szFilePathPC, IStreamCallback* pCallback, StreamReadParams* pParams)
{
	unsigned nFlags = 0;
	if (pParams)
		nFlags = pParams->nFlags;

	m_pPak->RecordFile( szFilePathPC );

	// get rid of some jobs if there are too many in the queue
	if (!(nFlags & SRP_QUICK_STARTREAD))
		while (numIOJobs(eWaiting) >= m_nMaxQueueLength)
		{
			m_pLog->LogWarning("StreamEngine: The number of jobs waiting %d >= max queue length %d, waiting to free up the queue", numIOJobs(eWaiting), m_nMaxQueueLength);
			UpdateAndWait(20, FLAGS_DISABLE_CALLBACK_TIME_QUOTA);
		}

	char szFilePathBuf[CCryPak::g_nMaxPath];
	const char* szFilePath = m_pPak->AdjustFileName (szFilePathPC, szFilePathBuf, pParams && (pParams->nFlags & SRP_FLAGS_PATH_REAL) ? ICryPak::FLAGS_PATH_REAL: 0);

	// first try to find such file; if it's already pending, add a client to it only
	CRefReadStream_AutoPtr pStream;
	NameStreamMap::iterator it = m_mapFilesByName.find (szFilePath);

	if (it == m_mapFilesByName.end())
	{
		pStream = new CRefReadStream (szFilePath, this);
	}
	else
		pStream = it->second;

	// make sure that the permanent streams get locked in memory;
	// if it's already locked, insert() won't do anything
	if (nFlags & SRP_FLAGS_MAKE_PERMANENT)
		m_setLockedStreams.insert (pStream);
	else
	if (nFlags & SRP_FLAGS_MAKE_TRANSIENT)
		m_setLockedStreams.erase (pStream);
	
	// at this moment the stream should self-register in this engine and the stream sets should get initialized
	CRefReadStreamProxy_AutoPtr pProxy = new CRefReadStreamProxy(szSource, pStream, pCallback, pParams);

	// register the proxy
	AddIOJob (pProxy);
	
	if (!(nFlags & SRP_QUICK_STARTREAD))
		Update(0);

	return (IReadStream*)pProxy;
}

// signals that this proxy needs to be executed (StartRead called)
void CRefStreamEngine::AddIOJob (CRefReadStreamProxy* pJobProxy)
{
	if (!m_hIOWorker)
	{ // its very simple with single-threaded model: we just put the job to the queue
		// for the next update will execute it
		m_queIOJobs.push_back(pJobProxy);
		SortIOJobs_NoLock();
	}
	else
	// for multi-threaded model, we need to put the job to the queue and signal the worker
	// thread about it.
	{
		// put to the queue
		{
			AUTO_LOCK (m_csIOJobs);
			m_queIOJobs.push_back(pJobProxy);
			SortIOJobs_NoLock();
		}
		SetEvent (m_hIOJob);
	} 
} 
 
  
//////////////////////////////////////////////////////////////////////////
// returns the size of the file; returns 0 if there's no such file.
unsigned CRefStreamEngine::GetFileSize (const char* szFilePathPC, unsigned nCryPakFlags)
{
	char szFilePathBuf[m_pPak->g_nMaxPath];
	const char *szFilePath = m_pPak->AdjustFileName (szFilePathPC, szFilePathBuf, nCryPakFlags);
	NameStreamMap::iterator it = m_mapFilesByName.find (szFilePath);
	if (it != m_mapFilesByName.end())
	{
		if (!it->second->GetFileSize())
			it->second->Activate();
		return it->second->GetFileSize(); 
	}  
   
	// we didn't find the file size in the cache - open the file and query the size
#if defined(LINUX)
	HANDLE hFile = CreateFile (szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
#else
	HANDLE hFile = CreateFile (szFilePath, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
#endif
	if (hFile != INVALID_HANDLE_VALUE)
	{
		unsigned nFileSize = ::GetFileSize(hFile, NULL);
		CloseHandle (hFile);
		return nFileSize;
	}
	else
	{
		CCachedFileDataPtr pFileData = m_pPak->GetFileData(szFilePath);
		if (pFileData)
			return pFileData->GetFileEntry()->desc.lSizeUncompressed;
		else
		{
			m_pPak->OnMissingFile(szFilePathPC);
			return 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Gets called regularly, to finalize those proxies whose jobs have
// already been executed (e.g. to call the callbacks)
//  - to be called from the main thread only
//  - starts new jobs in the single-threaded model
void CRefStreamEngine::Update(unsigned nFlags)
{
	unsigned numRemovedJobs = 0;
	unsigned numFinalizedJobs = 0;
	do {
	
		if (!m_hIOWorker)
		{
			// If we're in single-threaded mode, update means the whole cycle:
			// start the jobs, wait for their IO completion routine and finalize them
			numRemovedJobs = StartIOJobs();

			// enter alertable state so that the IO completion routines can be called
			WaitForSingleObjectEx(m_hDummyEvent, 0, TRUE);
		}
		
		// If we're in multi-threaded mode, Update from main thread means
		// just finalization of executed in the worker IO jobs.
		numFinalizedJobs = FinalizeIOJobs(nFlags);

		// Update from worker thread shouldn't be called at all
		
		// We continue updating until all the jobs that are possible to move out
		// of the IO Job queue are moved. Even if no jobs were moved, but some were finalized,
		// the finalized jobs may have spawned some new jobs, we'll try to start those, too
		// but we don't let it go on forever because of the limits
	} while(numRemovedJobs | numFinalizedJobs);
}

//////////////////////////////////////////////////////////////////////////
// Only waits at most the specified amount of time for some IO to complete
// - may initiate a new job
// - may be called from any non-worker thread
unsigned CRefStreamEngine::Wait(unsigned nMilliseconds, unsigned nFlags)
{
	ResetEvent (m_hIOExecuted);
	if (!IsMainThread())
	{
		// special case - this function is called from non-main thread
		// just wait until some io gets executed, if there's anything to wait for
		if (numIOJobs(eWaiting)+numIOJobs(ePending) > 0) // no sense to wait here if there are no waiting or pending jobs
			WaitForSingleObject(m_hIOExecuted, nMilliseconds);
		return 0;
	}

	AddCallbackTimeQuota (nMilliseconds * 1000);

	if (m_hIOWorker)
	{
		unsigned nFinalized = FinalizeIOJobs(nFlags); // finalize whatever may not have been finalized
		if (nFinalized)
			return nFinalized; // we don't wait if we finalized something
		if (numIOJobs(eWaiting)+numIOJobs(ePending) > 0) // no sense to wait here if there are no waiting or pending jobs
			WaitForSingleObject(m_hIOExecuted, nMilliseconds);
	}
	else
	{
		// really wait for some IO to complete
		if (numIOJobs(ePending) > 0) // no sense to wait here if there are no pending jobs
			SleepEx(nMilliseconds, TRUE);
		StartIOJobs(); // perhaps there's room for new tasks to be started now
	}
	return FinalizeIOJobs(nFlags);
}

// adds to the callback time quota in this frame, the specified number of microseconds
void CRefStreamEngine::AddCallbackTimeQuota (int nMicroseconds)
{
	m_nCallbackTimeQuota += nMicroseconds * m_nPerfFreq / 1000000;
	if (m_nCallbackTimeQuota < 0)
		m_nCallbackTimeQuota = 0;
}

void CRefStreamEngine::SetStreamCompressionMask( const DWORD indwMask )
{
	m_dwMask=indwMask;
}

void CRefStreamEngine::SetCallbackTimeQuota (int nMicroseconds)
{
	// if we have 
	if (m_nCallbackTimeQuota < 0)
	{
		m_pLog->LogWarning("\004io: overdraft of callback time quota in the last frame: %I64d mcs", m_nCallbackTimeQuota);
		AddCallbackTimeQuota(nMicroseconds);
	}
	else
		m_nCallbackTimeQuota = nMicroseconds * m_nPerfFreq / 1000000;
}


bool CRefStreamEngine::IsCallbackTimeQuota(unsigned nFlags)
{
	if (m_nSuspendCallbackTimeQuota == 0
		&& !(nFlags&FLAGS_DISABLE_CALLBACK_TIME_QUOTA)
		&& m_nCallbackTimeQuota <= 0
		)
		return false;
	return true;
}

// In the Multi-Threaded model (with the IO Worker thread)
// removes the proxies from the IO Queue as needed, and the proxies may call their callbacks
unsigned CRefStreamEngine::FinalizeIOJobs(unsigned nFlags)
{
	unsigned numFinalizedJobs = 0;
	// we fetch the executed jobs one-by-one, and finalize them
	// during finalization, the queue itself may be changed

	if (!IsCallbackTimeQuota(nFlags))
		return 0;

	AUTO_LOCK(m_csIOExecuted);
	while (!m_queIOExecuted.empty())
	{
		CRefReadStreamProxy_AutoPtr pProxy = m_queIOExecuted.front();
		m_queIOExecuted.pop_front();
		// to avoid locking the whole array during execution of the callbacks:
		AUTO_UNLOCK(m_csIOExecuted);

		assert(pProxy->IsIOExecuted());

		int64 nStartTime, nEndTime;
		QueryPerformanceCounter ((LARGE_INTEGER*)&nStartTime);
		// TODO: add control over the callback execution time
		// this proxy needs to be moved out of the IO queue
		pProxy->FinalizeIO ();
		++numFinalizedJobs;
		QueryPerformanceCounter((LARGE_INTEGER*)&nEndTime);

		m_nCallbackTimeQuota -= nEndTime - nStartTime;

		if (!IsCallbackTimeQuota(nFlags))
			break;
	}

	return numFinalizedJobs;
}


// this will be the thread that executes everything that can take time
void CRefStreamEngine::IOWorkerThread ()
{
	do
	{
		// we start whatever IO jobs we have in the queue
		StartIOJobs();
		// we wait for new jobs to arrive or for the IO callbacks to be called
		// even if it was a callback, we check for the new jobs: some jobs may have
		// been suspended because of performance reasons, until the next callback;
		// besides, the callback might have spawned some new jobs.
		// the pending->executed move will happen in the callback
		WaitForSingleObjectEx(m_hIOJob, INFINITE, TRUE);
	}
	while (!m_bStopIOWorker);

	// wait for pending IO
	AUTO_LOCK(m_csIOPending);
	for (int nRetries = 0; nRetries < 100 && !m_setIOPending.empty(); ++nRetries)
	{
		AUTO_UNLOCK(m_csIOPending);
		SleepEx(300, TRUE);
	}
}


// sort the IO jobs in the IOQueue by priority
void CRefStreamEngine::SortIOJobs()
{
	if (m_hIOWorker)
	{
		AUTO_LOCK(m_csIOJobs);
		SortIOJobs_NoLock();
	}
	else
	{
		SortIOJobs_NoLock();
	}
}


//////////////////////////////////////////////////////////////////////////
// this sorts the IO jobs, without bothering about synchronization
void CRefStreamEngine::SortIOJobs_NoLock()
{
	std::sort (m_queIOJobs.begin(), m_queIOJobs.end(), CRefReadStreamProxy::Order());
}

bool CRefStreamEngine::IsSuspended()
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
// 
unsigned CRefStreamEngine::StartIOJobs()
{
	unsigned numMovedJobs = 0;
	AUTO_LOCK(m_csIOJobs);
	{
		AUTO_LOCK(m_csIOPending);

		CRefReadStreamProxy* pEndJob = NULL; // the job that will mark the end of loop
		// TODO: implement limitation on the number of simultaneous read requests
		while(!m_queIOJobs.empty() 
			&& m_setIOPending.size() < m_nMaxReadDepth
			&& !IsSuspended())
		{
			CRefReadStreamProxy_AutoPtr pProxy = m_queIOJobs.front();

			m_queIOJobs.pop_front();
			m_setIOPending.insert (pProxy);

			// temporary unlock both queue and set and start the reading
			bool bReadStarted;
			{
				AUTO_UNLOCK(m_csIOPending);
				{
					AUTO_UNLOCK(m_csIOJobs);
					// try to start reading
					bReadStarted = pProxy->StartRead();
				}
			}

			if (bReadStarted)
			{
				// in case of no error - this should be in most cases:
				// we started the operation successfully
				// perhaps now it's even already read and moved from Pending to Executed queue

				// in case of unrecoverable error:
				// we didn't start reading and can't do so. It's already moved into Executed queue as errorneous
				++numMovedJobs;

				pEndJob = NULL; // start the whole loop all over again.
			}
			else
			{
				// recoverable error - we'll try again next time
				m_queIOJobs.push_back(pProxy);
				m_setIOPending.erase (pProxy);

				if (pEndJob)
				{
					if (pEndJob == pProxy)
						break; // we are looping - we can't start this job for the second time in a row now
				}
				else
				{
					pEndJob = pProxy; // mark this job as the end of loop; we'll erase the marker if the job is started
				}
			}
		}
	}
	return numMovedJobs;
}

void CRefStreamEngine::OnIOJobExecuted (CRefReadStreamProxy* pJobProxy)
{
	if (m_hIOWorker && (pJobProxy->GetParams().nFlags & SRP_FLAGS_ASYNC_CALLBACK))
		pJobProxy->FinalizeIO();

	{
		AUTO_LOCK(m_csIOPending);
		{
			AUTO_LOCK(m_csIOExecuted);
			// first add, then erase - to avoid releasing the autopointer
			// this is under double-locked CS's to avoid in-between artifacts
			m_queIOExecuted.push_back(pJobProxy);
			m_setIOPending.erase (pJobProxy);
		}
	}

	SetEvent (m_hIOExecuted);

	// perhaps this will free the way for another IO job.
	// but we won't call StartIOJobs(), because this same funciton can only be
	// called as part of Waiting after which the caller is aware that some jobs may have been executed
}

#ifndef LINUX
// returns the (cached) size of the sector on the volume where the given path points
// MT: MT-Safe + non-blocking, so far as writing to DWORD is atomic operation on the architecture it's executed at.
unsigned CRefStreamEngine::GetSectorSize(const char* szPath)
{
	DWORD dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	if (szPath[0] == '\\' && szPath[1] == '\\')
	{
		// this is a share, try to get the share sector size..
		// find the end of the share name \\server\share specification
		const char* pEnd = szPath+1; // the pEnd points to \server\share\...
		int nSlashes = 0;
		while(*pEnd && nSlashes < 2)
		{
			// the pEnd points to server\share\... the first time the loop is entered
			// when it reaches \share\.., nSlashes == 1, and 
			// when it reaches \..., nSlashes == 2 and pEnd points to the backslash 
			if (*++pEnd == '\\') 
				++nSlashes;
		}

		TElementaryArray<char> pShareName;
		pShareName.reinit (pEnd - szPath+1);
		memcpy (&pShareName[0], szPath, pEnd - szPath);
		pShareName[pEnd - szPath] = '\0';

		if (!GetDiskFreeSpace (&pShareName[0], &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
			// set some default for share
			dwSectorsPerCluster = 4 * 1024;
		return dwSectorsPerCluster;
	}
	else
	if (szPath[0] && szPath[1] == ':')
	{
		return GetDriveSectorSize(szPath[0]);
	}
	else
	{
		// this is relative path
		TElementaryArray<char> pDir;
		DWORD dwLen = GetCurrentDirectory(0, NULL);
		pDir.reinit (dwLen);
		if (dwLen != GetCurrentDirectory(dwLen, &pDir[0]))
		{
			DWORD dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
			if (!GetDiskFreeSpace(NULL, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
				dwSectorsPerCluster = 2 * 1024;
			return dwSectorsPerCluster;
		}
		else
			return GetDriveSectorSize(pDir[0]);
	}
}

unsigned CRefStreamEngine::GetDriveSectorSize (char cDrive)
{
	cDrive = tolower(cDrive);
	// this is an absolute path
	char szBuf[4] = "X:\\";
	szBuf[0] = cDrive;
	// determine the pointer to the cached value of the sector size (NULL if there's no such)
	unsigned * pCachedSectorSize = NULL;
	if (szBuf[0]>='c' && unsigned(szBuf[0] - 'c') < SIZEOF_ARRAY(m_nSectorSizes))
		pCachedSectorSize = m_nSectorSizes + szBuf[0]-'c';

	// if we have don't have this disk size cache, (or just it wasn't cached yet), calculate and cache it
	if (!pCachedSectorSize || *pCachedSectorSize)
	{
		DWORD dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
		if (!GetDiskFreeSpace (szBuf, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
			// set some default for disk
			dwSectorsPerCluster = 2 * 1024;
		else
			// cache the sector size
			*pCachedSectorSize = dwSectorsPerCluster;
	}
	return *pCachedSectorSize;
}
#endif //LINUX

void CRefStreamEngine::StopWorkerThread()
{
	if (m_hIOWorker)
	{
		m_bStopIOWorker = true;
		SetEvent(m_hIOJob);
		WaitForSingleObject (m_hIOWorker, INFINITE);
		CloseHandle (m_hIOWorker);
		m_hIOWorker = NULL;
	}
}

void CRefStreamEngine::StartWorkerThread()
{
	StopWorkerThread();
	m_bStopIOWorker = false;
	m_hIOWorker = CreateThread (NULL, 0x8000, IOWorkerThreadProc, this, 0, &m_dwWorkerThreadId);
}

//////////////////////////////////////////////////////////////////////////
// registers a new stream (added to the system: queued)
// MT: Main thread only
void CRefStreamEngine::Register (CRefReadStream* pStream)
{
	m_mapFilesByName.insert (NameStreamMap::value_type(pStream->GetFileName(), pStream));
}

//////////////////////////////////////////////////////////////////////////
// unregisters: happens upon release of all resources
// MT: Main thread only
void CRefStreamEngine::Unregister (CRefReadStream* pStream)
{
	m_mapFilesByName.erase(pStream->GetFileName());
}

// this function checks for the OS version and disables some capabilities of Streaming Engine when needed
// currently, in Win 9x overlapped IO is disabled
void CRefStreamEngine::CheckOSCaps()
{
#if defined(WIN32)
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(os);
	if (!GetVersionEx(&os))
	{
		m_bEnableOverlapped = false; // just in case
		return;
	}

	if (os.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		m_pLog->LogWarning("StreamEngine: OS (platform %d) doesn't support streaming, turning overlapped IO off",os.dwPlatformId );
		// only NT supports overlapped IO
		m_bEnableOverlapped = false;
	}
#elif defined(_XBOX) || defined(LINUX)
	// in XBox, nothing to disable
#else
#error // if your OS doesn't support it, you should disable Overlapped IO here
	m_bEnableOverlapped = false;
#endif
}

unsigned CRefStreamEngine::numIOJobs(IOJobKindEnum nKind)
{
	switch (nKind)
	{
	case eWaiting:
		{
			AUTO_LOCK(m_csIOJobs);
			return (unsigned)m_queIOJobs.size();
		}
		break;

	case ePending:
		{
			AUTO_LOCK(m_csIOPending);
			return (unsigned)m_setIOPending.size();
		}
		break;

	case eExecuted:
		{
			AUTO_LOCK(m_csIOExecuted);
			return (unsigned)m_queIOExecuted.size();
		}
		break;
	}

	return 0;
}


//! Puts the memory statistics into the given sizer object
//! According to the specifications in interface ICrySizer
void CRefStreamEngine::GetMemoryStatistics(ICrySizer *pSizer)
{
	size_t nSize = sizeof(*this);

	for (NameStreamMap::iterator it = m_mapFilesByName.begin(); it != m_mapFilesByName.end(); ++it)
	{
		nSize += sizeof(NameStreamMap::value_type) + it->first.capacity() + it->second->GetSize();
	}

	nSize += m_setLockedStreams.size() * sizeof(CRefReadStream_AutoSet::value_type);

	// here we calculate the capacities of 3 arrays; we don't want a deadlock so we lock them one by one, 
	// small discrepancies because something can be moved somewhere don't matter
	{
		AUTO_LOCK(m_csIOJobs);
		nSize += m_queIOJobs.size() * sizeof(CRefReadStreamProxy_AutoDeque_MT::value_type);
	}

	{
		AUTO_LOCK(m_csIOPending);
		nSize += m_setIOPending.size() * sizeof(CRefReadStreamProxy_AutoSet_MT::value_type);
	}

	{
		AUTO_LOCK(m_csIOExecuted);
		nSize += m_queIOExecuted.size() * sizeof(CRefReadStreamProxy_AutoDeque_MT::value_type);
	}

	// this is calculated because each queue capacity is taken into account
	//nSize += m_SmallHeap.getAllocatedSize();
	// this is calculated because each stream Proxy contain pointer to this data
	//nSize += m_BigHeap.getAllocatedSize();

	pSizer->AddObject(this, nSize);
}

//! Enables or disables callback time quota per frame
void CRefStreamEngine::SuspendCallbackTimeQuota ()
{
	if (m_nPerfFreq > 0)
	{
		++m_nSuspendCallbackTimeQuota;
	}
}
void CRefStreamEngine::ResumeCallbackTimeQuota ()
{
	if (m_nPerfFreq > 0)
	{
		--m_nSuspendCallbackTimeQuota;
		if (m_nSuspendCallbackTimeQuota == 0 && m_nCallbackTimeQuota < 0)
		{
			m_pLog->LogWarning("\005io: overdraft of callback time quota after resume: %I64d", m_nCallbackTimeQuota);
			m_nCallbackTimeQuota = 0;
		}
	}
}


DWORD CRefStreamEngine::GetStreamCompressionMask() const
{
	return m_dwMask;
}