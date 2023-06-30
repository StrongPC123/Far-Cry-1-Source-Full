//////////////////////////////////////////////////////////////////////////
// Implementation of CRefReadStreamProxy
// The proxy is the per-client unique object that is acquired by the Streaming
// Engine clients via StartRead() API. Several Proxies can refer to the same
// Stream (file) but perform their operations independently.

#include "stdafx.h"
#include <ISystem.h>
#include <ILog.h>
#include "RefStreamEngine.h"
#include "RefReadStream.h"
#include "RefReadStreamProxy.h"

extern ISystem* g_System;
extern CMTSafeHeap* g_pSmallHeap;
extern CMTSafeHeap* g_pBigHeap;

CRefReadStreamProxy::CRefReadStreamProxy (const char* szSource, CRefReadStream* pStream, IStreamCallback* pCallback, StreamReadParams* pParams):
	m_strClient(szSource),
	m_pStream (pStream),
	m_pCallback(pCallback),
	m_bError(false),
	m_bFinished (false),
	m_bFreeBuffer (false),
	m_bPending (false),
	m_pBuffer (NULL),
	m_numBytesRead (0),
	m_numRetries (0)
{
	if (pParams)
		m_Params = *pParams;
	m_pBuffer = m_Params.pBuffer;
#if LOG_IO
	g_System->GetILog()->LogToFile ("\006io:CRefReadStreamProxy %p(%s, %p)", this, szSource, pCallback);
#endif
	pStream->Register(this);
}

CRefReadStreamProxy::~CRefReadStreamProxy ()
{
	if (!m_bFinished && !m_bError)
		OnFinishRead(ERROR_UNEXPECTED_DESTRUCTION);
	if (m_bFreeBuffer && m_pBuffer)
		g_pBigHeap->Free (m_pBuffer);
#if LOG_IO
	g_System->GetILog()->LogToFile ("\006io:~CRefReadStreamProxy %p(%s, %p)", this, m_strClient.c_str(), m_pCallback);
#endif
	m_pStream->Unregister(this);
}

// returns true if the file read was not successful.
bool CRefReadStreamProxy::IsError()
{
	return m_bError;
}

// returns true if the file read was completed (successfully or unsuccessfully)
// check IsError to check if the whole requested file (piece) was read
bool CRefReadStreamProxy::IsFinished()
{
	return m_bFinished;
}

// returns the number of bytes read so far (the whole buffer size if IsFinished())
unsigned int CRefReadStreamProxy::GetBytesRead (bool bWait)
{
	if (m_bPending)
	{
		if (m_pStream->isOverlapped())
		{
			DWORD dwBytesRead;
			if (GetOverlappedResult(m_pStream->GetFile(), &m_Overlapped, &dwBytesRead, bWait))
			{
				m_numBytesRead = m_nPieceOffset + dwBytesRead;
				assert (dwBytesRead <= m_nPieceLength);
			}
		}
	}
	return m_numBytesRead;
}


// returns the buffer into which the data has been or will be read
// at least GetBytesRead() bytes in this buffer are guaranteed to be already read
const void* CRefReadStreamProxy::GetBuffer ()
{
	return m_pBuffer;
}

// tries to stop reading the stream; this is advisory and may have no effect
// but the callback	will not be called after this. If you just destructing object,
// dereference this object and it will automatically abort and release all associated resources.
void CRefReadStreamProxy::Abort()
{
	// lock this object to avoid preliminary destruction
	CRefReadStreamProxy_AutoPtr pLock (this);
	if (m_bPending)
	{
		m_pStream->Abort(this);
		// we need to wait to avoid letting the client freeing the buffer before the read is finished
		if (!m_bFreeBuffer) // [sergiy] Comment this line (only if) out to let it complete all operations anyway
			Wait();
	}
	else
	{
		m_pCallback = NULL;
		m_bError = true;
		m_nIOError = ERROR_USER_ABORT;
	}
	//assert (m_pCallback == NULL);
	// perhaps the callback was already called, or perhaps not. In any case we forget about the callback
	m_pCallback = NULL;
}

// tries to raise the priority of the read; this is advisory and may have no effect
void CRefReadStreamProxy::RaisePriority (unsigned nPriority)
{
	if (m_Params.nPriority != nPriority)
	{
		m_Params.nPriority = nPriority;
		m_pStream->OnRaisePriority(this, nPriority);
	}
}

// unconditionally waits until the callback is called
// i.e. if the stream hasn't yet finish, it's guaranteed that the user-supplied callback
// is called before return from this function (unless no callback was specified)
void CRefReadStreamProxy::Wait()
{
	// lock this object to avoid preliminary destruction
	CRefReadStreamProxy_AutoPtr pLock (this);
	// move it to the top of the corresponding queues
	RaisePriority(INT_MAX);

	// while the stream reading hasn't finished, OR the callback isn't still called, wait
	while ((!m_bFinished && !m_bError) || m_pCallback)
	{
		m_pStream->GetEngine()->UpdateAndWait(100, IStreamEngine::FLAGS_DISABLE_CALLBACK_TIME_QUOTA);
	}
}

// the interface for the actual stream
// returns true if the read has been started and no further attempts to do so are required
// returns false if couldn't start,and retry is required
// nMemQuota is the max number of bytes to allocate from the Engine's "Big Heap" for the piece of file
// that is read. Pass a big value to let it allocate as much as it wants.
bool CRefReadStreamProxy::StartRead (unsigned nMemQuota)
{
	if (m_bError || m_bFinished || m_bPending)
	{
		// Why this assert happened?
		//---------------------------
		// THe stream read was automatically initiated, while the stream is marked as
		// having been read, being read, or failed, i.e. it can't be restarted again.
		
		// This can generally happen when the read has been started, and immediately aborted,
		// while being put on hold. When there are enough IO resources to start reading it,
		// we detect that it's been already aborted and don't restart it.
		// This is the only known reason for that.
		assert (m_nIOError == ERROR_USER_ABORT);
		return true; // invalid call, no need to retry
	}

	if (!m_pStream->Activate())
	{
		OnFinishRead(ERROR_CANT_OPEN_FILE); // can't open file
		return true; // no need to retry
	}

	if (m_pStream->IsError())
	{
		OnFinishRead(ERROR_REFSTREAM_ERROR); // file is invalid
		return true;
	}

	if (m_Params.nOffset >= m_pStream->GetFileSize())
	{
		// offset out of range
		OnFinishRead(ERROR_OFFSET_OUT_OF_RANGE);
		return true;
	}

	if (m_Params.nSize > m_pStream->GetFileSize())
	{
		OnFinishRead(ERROR_SIZE_OUT_OF_RANGE);
		return true;
	}

	if (m_Params.nSize == 0)
	{
		// by default, we read the whole file
		m_Params.nSize = m_pStream->GetFileSize() - m_Params.nOffset;
	}
	else
	if (m_Params.nOffset + m_Params.nSize > m_pStream->GetFileSize())
	{
		// it's impossible to read the specified region of the file
		OnFinishRead(ERROR_REGION_OUT_OF_RANGE);
		return true;
	}

	if (!m_pBuffer)
	{
		if (nMemQuota < m_Params.nSize)
		{
			// try another time, of fail now if the retries are over
			if (++m_numRetries < g_numMaxRetries)
				return false;
			else
			{
				OnFinishRead(ERROR_OUT_OF_MEMORY_QUOTA);
				return false;
			}
		}

		m_pBuffer = g_pBigHeap->Alloc(m_Params.nSize, "CRefReadStreamProxy::StartRead: m_pBuffer");
		if (!m_pBuffer)
		{
			OnFinishRead(ERROR_OUT_OF_MEMORY);
			return true;
		}
		m_bFreeBuffer = true;
	}

	HANDLE hFile = m_pStream->GetFile();
	m_nPieceOffset = 0; // we're just start reading
	// we should load in blocks, if we load overlapped; 
	// we should load in one continuous read, if we load non-overlapped
	unsigned nMaxBlockLength = m_pStream->GetEngine()->GetPak()->GetPakVars()->nReadSlice * 1024;
	if (!nMaxBlockLength)
		nMaxBlockLength = g_nBlockLength;
	m_nPieceLength = m_pStream->isOverlapped() ? min (m_Params.nSize, nMaxBlockLength) : m_Params.nSize;
	m_numBytesRead = 0;

	// lock the object for the time of read operation
	this->AddRef();
	
	if ((m_Params.nFlags & SRP_FLAGS_ASYNC_PROGRESS) && m_pCallback)
		m_pCallback->StreamOnProgress(this);

	m_bPending = true;
	++g_numPendingOperations;
	DWORD dwError = CallReadFileEx ();
	if (dwError)
	{
		m_bPending = false;
		--g_numPendingOperations;

		bool bResult = true; // by default, signal an error
		switch (dwError)
		{
#if !defined(LINUX)
		case ERROR_NOT_ENOUGH_MEMORY:
		case ERROR_INVALID_USER_BUFFER:
#endif
		case ERROR_NO_SYSTEM_RESOURCES:
			if (++m_numRetries < g_numMaxRetries)
				bResult = false; // try again
		}
		// if bResult == false, it means we will want to try again, so we don't finish reading in this case
		if (bResult)
			OnFinishRead(dwError);

		this->Release();
		return bResult;
	}
	else
		return true;
}

unsigned CRefReadStreamProxy::g_numPendingOperations = 0;

VOID CALLBACK CRefReadStreamProxy::FileIOCompletionRoutine (
	DWORD dwErrorCode,                // completion code
	DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
	LPOVERLAPPED lpOverlapped         // I/O information buffer
	)
{
#if defined(LINUX)
	assert(lpOverlapped->pCaller != 0);
	CRefReadStreamProxy* pThis = (CRefReadStreamProxy*)lpOverlapped->pCaller;
#else
	const LONG_PTR nOOffset = (LONG_PTR)(&((CRefReadStreamProxy*)0)->m_Overlapped);
	CRefReadStreamProxy* pThis = (CRefReadStreamProxy*)((LONG_PTR)lpOverlapped - nOOffset);
#endif
	// this is only called when the stream is overlapped
	assert (pThis->m_pStream->isOverlapped());
	pThis->OnIOComplete (dwErrorCode, dwNumberOfBytesTransfered);
}

void CRefReadStreamProxy::OnIOComplete(unsigned nError, unsigned numBytesRead)
{
	m_numBytesRead = m_nPieceOffset + numBytesRead;

	// if there are more bytes read than was requested (e.g. because of the sector content after the EOF), we trim it.
	if (!nError && m_numBytesRead > m_Params.nSize)
		m_numBytesRead = m_Params.nSize;

	--g_numPendingOperations;
	m_bPending = false;

	// calculate the next piece offset/length
	m_nPieceOffset = m_numBytesRead;
	assert (m_Params.nSize>=m_numBytesRead);

	unsigned nMaxBlockLength = m_pStream->GetEngine()->GetPak()->GetPakVars()->nReadSlice * 1024;
	if (!nMaxBlockLength)
		nMaxBlockLength = g_nBlockLength;

	m_nPieceLength = min (m_Params.nSize-m_numBytesRead,nMaxBlockLength);
	
	// if there's nothing more to read,
	// or there's an error, finish reading
	if (nError || !m_nPieceLength)
	{
		OnFinishRead(nError);

		// unlock the object for the time of read operation
		this->Release();
	}
	else
	{
		// there's no error and there's still a piece to read. Read it further.
		// we can't call progress messages from another thread
		//if (m_pCallback)
		//	m_pCallback->StreamOnProgress(this);

		m_bPending = true;
		DWORD dwError = CallReadFileEx();
		if (dwError)
		{
			m_bPending = false;
			OnFinishRead(dwError);
			this->Release();
			return;
		}
		else
		{
			++g_numPendingOperations;
		}
	}
}


// on the platforms that support overlapped IO, calls ReadFileEx.
// on other platforms merely reads the file, calling OnIOComplete()
DWORD CRefReadStreamProxy::CallReadFileEx ()
{
	HANDLE hFile = m_pStream->GetFile();

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// this is a special case, reading from the cache memory directly - we should handle it separately
		void* pSource = m_pStream->GetFileData();
		if (!pSource)
		{
			OnIOComplete(ERROR_ZIP_CACHE_FAILURE,0);
			return 0;
		}
		memcpy (((char*)m_pBuffer) + m_nPieceOffset, ((char*)pSource) + m_Params.nOffset + m_nPieceOffset, m_nPieceLength);
		OnIOComplete(0, m_nPieceLength);
		return 0;
	}

	if (m_pStream->isOverlapped())
	{
		memset (&m_Overlapped, 0, sizeof(m_Overlapped));
		m_Overlapped.Offset = m_Params.nOffset + m_nPieceOffset + m_pStream->GetArchiveOffset();
#if defined(LINUX)
		m_Overlapped.pCaller = (void*)this;//store caller address here
#endif
		if (!ReadFileEx (hFile, ((char*)m_pBuffer) + m_nPieceOffset, m_nPieceLength, &m_Overlapped, FileIOCompletionRoutine))
		{
			DWORD dwError = GetLastError();
			if (!dwError)
				dwError = ERROR_CANT_START_READING;

			return dwError;
		}
		else
			return 0;
	}
	else
	{
		// the actual number of bytes read
		DWORD dwRead = 0;
		unsigned newOffset = m_Params.nOffset + m_nPieceOffset + m_pStream->GetArchiveOffset();
		if (SetFilePointer (hFile, newOffset, NULL, FILE_BEGIN) != newOffset)
		{
			// the positioning error is strange, we should examine it and perhaps retry (in case the file write wasn't finished.)
			DWORD dwError = GetLastError();
			return dwError;
		}
		// just read the file
		if (!ReadFile (hFile, ((char*)m_pBuffer) + m_nPieceOffset, m_nPieceLength, &dwRead, NULL))
		{
			// we failed to read; we don't call the callback, but we could as well call OnIOComplete()
			// with this error code and return 0 as success flag emulating error during load
			DWORD dwError = GetLastError();
			//return dwError;
			// we call the callback to signal about the error, and return 0 signaling that the operation has completed
			OnIOComplete(dwError, dwRead);
			return 0;
		}
		else
		{
			OnIOComplete (0,dwRead);
			return 0;
		}
	}
}


void CRefReadStreamProxy::OnFinishRead(unsigned nError)
{
	// [marco] commented out, according to sergiy this is harmless	
	//assert (!m_bFinished && !m_bError); 
	if (!nError)
		m_bFinished = true;
	else
		m_bError = true;

	m_nIOError = nError;
	m_pStream->OnIOExecuted(this);
}

// this returns true after the main IO job has been executed (either in worker or in main thread)
bool CRefReadStreamProxy::IsIOExecuted()
{
	return IsFinished() || IsError();
}

// this gets called upon the IO has been executed to call the callbacks
void CRefReadStreamProxy::FinalizeIO()
{
	if (m_pCallback)
	{
		// be carefull! this object can be deallocated inside the callback!
		IStreamCallback* pCallback = m_pCallback;
		m_pCallback = NULL;	// don't call this callback any more as it may have been deallocated
#if LOG_IO
		g_System->GetILog()->LogToFile ("\006io(%s) err %d%s%s%s%s piece(%d:%d) read %d %s userdata %d, pri %d, flags %x, offs %d, size %d", m_strClient.c_str(), m_nIOError, 
			m_bError?" Error":"", m_bFinished?" Finished":"", m_bFreeBuffer?" FreeBuffer":"", m_bPending?" Pending":"",
			m_nPieceOffset, m_nPieceLength,
			m_numBytesRead,
			m_pStream->GetFileName().c_str(),
			m_Params.dwUserData,
			m_Params.nPriority,
			m_Params.nFlags,
			m_Params.nOffset,m_Params.nSize);
#endif
		pCallback->StreamOnComplete(this, m_bError?m_nIOError:0);
#if LOG_IO
		g_System->GetILog()->LogToFile ("\006io callback %p returned", pCallback);
#endif
	}
}

string CRefReadStreamProxy::Dump()
{
	char szDump[0x300];
	_snprintf (szDump, sizeof(szDump), "%s: callback %p, %s%s%s %d bytes read, offset=%d, size=%d, flags=%x",
		m_strClient.c_str(),
		m_pCallback,
		m_bPending?"PENDING ":"",
		m_bFinished?"FINISHED ":"",
		m_bError?"ERROR ":"",
		m_numBytesRead,
		m_Params.nOffset,
		m_Params.nSize,
		m_Params.nFlags);
	return szDump;
}

// returns the size of allocated memory for this object and all subobjects if any
size_t CRefReadStreamProxy::GetSize()
{
	size_t nSize = sizeof(*this);
	nSize += m_strClient.capacity();
	if (m_pBuffer)
		nSize += m_Params.nSize;
	return nSize;
}