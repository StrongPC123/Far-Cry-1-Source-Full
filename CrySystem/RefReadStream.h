#ifndef _REF_READ_STREAM_HDR_
#define _REF_READ_STREAM_HDR_

#include "ZipDir.h"
#include <IStreamEngine.h>
#include "CryPak.h"

class CRefStreamEngine;
class CRefReadStreamProxy;

class CRefReadStream: public _reference_target_t
{
public:

	// this is the maximum size in bytes of a file inside zip
	// that will be cached even though it's not compressed
	enum {g_nMaxCacheUncompressed = 64 * 1024};

	// if the file can't be opened, this object permanently returns an error (right after construction)
	CRefReadStream (const string& strFileName, CRefStreamEngine* pEngine);

	// returns true if the file read was not successful.
	virtual bool IsError() {return m_bError;}

	// request to abort comes from the proxy. This doesn't means immediate deallocation.
	virtual void Abort(CRefReadStreamProxy* pProxy);

	// Client (through the given Proxy) has requested priority rise
	virtual void OnRaisePriority (CRefReadStreamProxy* pProxy, unsigned nPriority);

	// the proxy to this stream appeared, take it into account (increase the ref counter)
	void Register (CRefReadStreamProxy* pProxy){this->AddRef();m_setProxies.insert (pProxy);}
	// the proxy deallocates, don't take it into account (decrease ref counter)
	void Unregister (CRefReadStreamProxy* pProxy) {m_setProxies.erase (pProxy);this->Release();}

	// returns the path to the file; this is always the real path, it shouldn't undergo MOD path adjustments
	const string& GetFileName() const {return m_strFileName;}

	unsigned GetFileSize() const {return m_nFileSize;}
	HANDLE GetFile () {return m_hFile;}

	void OnIOExecuted(CRefReadStreamProxy* pProxy);

#ifndef LINUX
	// returns the size of the sector on the disk on which this file resides
	unsigned GetSectorSize();
#endif //LINUX
	
	// activates: opens the file, gets its size. If failed, returns false
	bool Activate();

	CRefStreamEngine* GetEngine() {return m_pEngine;}

	// returns the offset that is to be added to the desired offset in the file handle opened by this stream
	// this is an artificial offset that will encounter for the files being in an archive
	unsigned GetArchiveOffset()const
	{
		if (m_pZipEntry)
			return m_pZipEntry->GetFileDataOffset();
		else
			return 0;
	}

	// this is true when the file was open with FILE_FLAG_OVERLAPPED flag;
	// and therefore the operations on the file may and should be overlapped.
	// otherwise perhaps some limitation exists and we should only work synchronously.
	bool isOverlapped() const {return m_bOverlapped;}

	// dumps all clients (proxies) - each uses the Dump function of the proxy
	string Dump();

	// returns the size of allocated memory for this object and all subobjects (Proxies)
	size_t GetSize();

	void* GetFileData()
	{
		if (m_pZipEntry)
			return m_pZipEntry->GetData();
		else
			return NULL;
	}
private:
	// the clients are not allowed to destroy this object directly; only via Release()
	~CRefReadStream();

	// parent object
	CRefStreamEngine* m_pEngine;

	// path of the file. this is always the real path, it shouldn't undergo MOD path adjustments
	string m_strFileName;

	// the handle to opened file, or invalid handle value if the file couldn't be opened
	// if the file can't be opened, this object permanently returns an error (right after construction)
	HANDLE m_hFile;

	// if this is not NULL, the file actually resides in a virtual file system inside a zip file.
	// m_hFile may not or may be NULL. if m_hFile != NULL and this is not null, m_hFile is the handle
	// to the actual zip archieve where the file resides
	CCachedFileDataPtr m_pZipEntry;

	// this is the size of the sector on the disk on which this file resides
	unsigned m_nSectorSize;

	// the file size, or 0 if the file couldn't be opened
	DWORD m_nFileSize;
	// the set of proxies
	typedef std::set<CRefReadStreamProxy*> ProxySet;
	ProxySet m_setProxies;

	// error/finished conditions
	bool m_bError;

	// this flag is meaningful only with valid m_hFile. If it's true, it means the 
	// file was opened for Overlapped access (it can't be opened so in Win 9x)
	bool m_bOverlapped;
};

TYPEDEF_AUTOPTR(CRefReadStream);
typedef std::vector<CRefReadStream_AutoPtr> CRefReadStream_AutoArray;


#endif