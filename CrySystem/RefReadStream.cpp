#include "stdafx.h"
#include "CritSection.h"
#include "RefStreamEngine.h"
#include "RefReadStreamProxy.h"
#include "RefReadStream.h"

// creates a read stream from the file, in the engine, 
// with the given default principal client
// the path is always the real path, it shouldn't undergo MOD path adjustments
CRefReadStream::CRefReadStream (const string& strFileName, CRefStreamEngine* pEngine):
	m_pEngine (pEngine), 
	m_bError (false),    // no error yet because we didn't try to do anything
	m_strFileName (strFileName),
	m_nFileSize (0),
	m_nSectorSize(0),
	m_hFile (INVALID_HANDLE_VALUE),
	m_bOverlapped ( false),
	m_pZipEntry (NULL)
{
	pEngine->Register(this);
}


// activates: opens the file, gets its size. If failed, returns false
bool CRefReadStream::Activate()
{
	static CCritSection g_csActivate;

	AUTO_LOCK(g_csActivate);

	m_bOverlapped = m_pEngine->isOverlappedIoEnabled();
#if !defined(LINUX64)
	if (m_pZipEntry == NULL && m_hFile == INVALID_HANDLE_VALUE)
#else
	if (m_pZipEntry == 0 && m_hFile == INVALID_HANDLE_VALUE)
#endif
		m_hFile = CreateFile (m_strFileName.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			m_bOverlapped?FILE_FLAG_OVERLAPPED:0,
			NULL);
#if !defined(LINUX64)
	if (m_pZipEntry == NULL && m_hFile == INVALID_HANDLE_VALUE)
#else
	if (m_pZipEntry == 0 && m_hFile == INVALID_HANDLE_VALUE)
#endif
	{
		// perhaps this is in a zip file
		m_pZipEntry = m_pEngine->GetPak()->GetFileData(m_strFileName.c_str());
		if (m_pZipEntry)
		{
			m_nFileSize = m_pZipEntry->GetFileEntry()->desc.lSizeUncompressed;
			m_bError = false;
	
			// try to open the actual file if can be done - this is only worthy
			// if the data isn't yet in the cache AND the actual file is big enough
			// AND it's uncompressed in the zip file
			if (!m_pZipEntry->GetData(false)
				&& m_pZipEntry->GetFileEntry()->nMethod == ZipFile::METHOD_STORE
				//&& m_pZipEntry->GetFileEntry()->nSizeUncompressed > g_nMaxCacheUncompressed
				)
			{
				// try to open the file - this should really be not often the case
				const char* szPakFile = m_pZipEntry->GetZip()->GetFilePath();
				// even if we can't open it, it doesn't matter: we automatically resort to using the cache
				m_hFile = CreateFile (szPakFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
					m_bOverlapped?FILE_FLAG_OVERLAPPED:0,
					NULL);
			}

			if (m_hFile == INVALID_HANDLE_VALUE)
				m_bOverlapped = false;
			return true;
		}
		else
		{
			DWORD dwError = GetLastError();
			m_bError = true;
			m_pEngine->GetPak()->OnMissingFile(m_strFileName.c_str());
			return false;
		}
	}
	else
	{
		if (!m_nFileSize)
		{
			if (m_pZipEntry)
				m_nFileSize = m_pZipEntry->GetFileEntry()->desc.lSizeUncompressed;
			else
				m_nFileSize = ::GetFileSize (m_hFile, NULL);
	
			if (m_nFileSize == INVALID_FILE_SIZE)
			{
				m_bError = true;
				m_nFileSize = 0;
				return false;
			}
			m_bError = false;
		}
		return true;
	}
}

// the clients are not allowed to destroy this object directly; only via Release()
CRefReadStream::~CRefReadStream()
{
	m_pEngine->Unregister(this);
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
}


// request to abort comes from the proxy. This doesn't means immediate deallocation.
void CRefReadStream::Abort(CRefReadStreamProxy* pProxy)
{
	if (m_setProxies.size() == 1)
	{
		// there's only one proxy that uses this object; so we can safely cancel io
		// on this file
		CancelIo (m_hFile);
	}
}

// Client (through the given Proxy) has requested priority rise
void CRefReadStream::OnRaisePriority (CRefReadStreamProxy* pProxy, unsigned nPriority)
{
	if (!pProxy->IsIOExecuted())
		m_pEngine->SortIOJobs();
}

#ifndef LINUX
// returns the size of the sector on the disk on which this file resides
unsigned CRefReadStream::GetSectorSize()
{
	if (!m_nSectorSize)
		m_nSectorSize = m_pEngine->GetSectorSize(m_strFileName.c_str());
	return m_nSectorSize;
}
#endif //LINUX

void CRefReadStream::OnIOExecuted(CRefReadStreamProxy* pProxy)
{
	m_pEngine->OnIOJobExecuted(pProxy);
}

// dumps all clients (proxies) - each uses the Dump function of the proxy
string CRefReadStream::Dump()
{
	string strClients;
	for (ProxySet::iterator it = m_setProxies.begin(); it != m_setProxies.end(); ++it)
	{
		if (!strClients.empty())
			strClients += ",";
		strClients += (*it)->Dump();
	}
	return "{"+strClients+"}";
}

// returns the size of allocated memory for this object and all subobjects (Proxies)
size_t CRefReadStream::GetSize()
{
	size_t nSize = sizeof(*this);
	nSize += m_strFileName.capacity();

	for (ProxySet::iterator it = m_setProxies.begin();  it != m_setProxies.end(); ++it)
		nSize += sizeof(ProxySet::value_type) + (*it)->GetSize();
	return nSize;
}
