
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:CryPak.h
//
//	History:
//	-Feb 2,2001:Created by Honich Andrey
//  -June 12, 2003: Taken over by Sergiy Migdalskiy.
//     Got rid of unzip usage, now using ZipDir for much more effective
//     memory usage (~3-6 times less memory, and no allocator overhead)
//     to keep the directory of the zip file; better overall effectiveness and
//     more readable and manageable code, made the connection to Streaming Engine
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYPAK_H
#define CRYPAK_H

#include <ICryPak.h>
#include "IMiniLog.h"
#include "ZipDir.h"
#include "MTSafeAllocator.h"
#include "CritSection.h"
#include "StlUtils.h"
#include "PakVars.h"

extern CMTSafeHeap* g_pSmallHeap;
extern CMTSafeHeap* g_pBigHeap;

// this is the header in the cache of the file data
struct CCachedFileData: public _reference_target_MT_novtbl<CCachedFileData>
{
	CCachedFileData (class CCryPak* pPak, ZipDir::Cache*pZip, ZipDir::FileEntry* pFileEntry);

	~CCachedFileData();

	// return the data in the file, or NULL if error
	// by default, if bRefreshCache is true, and the data isn't in the cache already,m
	// the cache is refreshed. Otherwise, it returns whatever cache is (NULL if the data isn't cached yet)
	void* GetData(bool bRefreshCache = true);

	ZipDir::Cache* GetZip(){return m_pZip;}
	ZipDir::FileEntry* GetFileEntry() {return m_pFileEntry;}

	// the memory needs to be allocated out of a MT-safe heap here
	void * __cdecl operator new   (size_t size) { return g_pSmallHeap->Alloc(size, "CCachedFileData::new"); } 
	void __cdecl operator delete  (void *p) { g_pSmallHeap->Free(p); };

	unsigned GetFileDataOffset()
	{
		if (m_pFileEntry->nFileDataOffset == m_pFileEntry->INVALID_DATA_OFFSET)
			m_pZip->Refresh (m_pFileEntry);
		return m_pFileEntry->nFileDataOffset;
	}

	unsigned sizeofThis()const
	{
		return sizeof(*this) + (m_pFileData&&m_pFileEntry?m_pFileEntry->desc.lSizeUncompressed:0);
	}
protected:

	void* m_pFileData;

	volatile LONG m_nRefCounter;

	// the zip file in which this file is opened
	ZipDir::CachePtr m_pZip;
	// the file entry : if this is NULL, the entry is free and all the other fields are meaningless
	ZipDir::FileEntry* m_pFileEntry;

	class CCryPak* m_pPak;

	friend struct CCachedFileDataOrder;
};

TYPEDEF_AUTOPTR(CCachedFileData);
typedef CCachedFileData_AutoPtr CCachedFileDataPtr;

// the cached data pointers are sorted by the FileEntries
struct CCachedFileDataOrder
{
	bool operator () (const CCachedFileData* left, const CCachedFileData* right)const
	{
		return left->m_pFileEntry < right->m_pFileEntry;
	}
};



// an (inside zip) emultated open file
struct CZipPseudoFile
{
	CZipPseudoFile()
	{
		Construct();
	}

	enum
	{
		_O_COMMIT_FLUSH_MODE = 1 << 31,
		_O_DIRECT_OPERATION  = 1 << 30
	};

	// this object must be constructed before usage
	// nFlags is a combination of _O_... flags
	void Construct(CCachedFileData* pFileData = NULL, unsigned nFlags = 0)
	{
		m_pFileData = pFileData;
		m_nFlags = nFlags;
		m_nCurSeek = 0;

		if ((nFlags & _O_DIRECT_OPERATION)&&pFileData&&pFileData->GetFileEntry()->nMethod == ZipFile::METHOD_STORE)
		{
			m_fDirect = fopen (pFileData->GetZip()->GetFilePath(), "rb"); // "rb" is the only supported mode
			FSeek(0,SEEK_SET); // the user assumes offset 0 right after opening the file
		}
		else
			m_fDirect = NULL;
	}

	// this object needs to be freed manually when the CryPak shuts down..
	void Destruct()
	{
		assert ((bool)m_pFileData);
		// mark it free, and deallocate the pseudofile memory
		m_pFileData = NULL;
		if (m_fDirect)
			fclose (m_fDirect);
	}

	CCachedFileData* GetFile() {return m_pFileData;}

	long FTell() {return m_nCurSeek;}
	
	unsigned GetFileSize()
	{
		return GetFile()?GetFile()->GetFileEntry()->desc.lSizeUncompressed:0;
	}

	int FSeek (long nOffset, int nMode);
	size_t FRead (void* pDest, size_t nSize, size_t nCount, FILE* hFile);
	int FEof();
	int FScanfv (const char* szFormat, va_list args);
	char* FGets(char* pBuf, int n);
	int Getc();
	int Ungetc(int c);

	FILETIME	 GetModificationTime()
	{
		return m_pFileData->GetFileEntry()->GetModificationTime();
	}


	const char* GetArchivePath()
	{
		return m_pFileData->GetZip()->GetFilePath();
	}
protected:
  long m_nCurSeek;
  CCachedFileDataPtr m_pFileData;
	// nFlags is a combination of _O_... flags
  unsigned m_nFlags;
	FILE* m_fDirect;
};


struct CIStringOrder
{
	bool operator () (const string& left, const string& right) const
	{
		return stricmp(left.c_str(), right.c_str()) < 0;
	}
};

class CCryPakFindData: public _reference_target_t
{
public:
	// the directory wildcard must already be adjusted
	CCryPakFindData (class CCryPak*pPak, const char* szDir);
	bool	empty() const;
	bool	Fetch(_finddata_t* pfd);
	void	Scan(CCryPak*pPak, const char* szDir);

	size_t sizeofThis()const;
protected:
	void ScanFS(CCryPak*pPak, const char* szDir);
	void ScanZips(CCryPak*pPak, const char* szDir);

	struct FileDesc
	{
		unsigned nAttrib;
		unsigned nSize;
		time_t tAccess;
		time_t tCreate;
		time_t tWrite;

		FileDesc (struct _finddata_t* fd);
		FileDesc (struct __finddata64_t* fd);

		FileDesc (ZipDir::FileEntry* fe);

		// default initialization is as a directory entry
		FileDesc ();
	};
	typedef std::map<string, FileDesc, CIStringOrder> FileMap;
	FileMap m_mapFiles;
};

TYPEDEF_AUTOPTR(CCryPakFindData);




//////////////////////////////////////////////////////////////////////
class CCryPak : public ICryPak
{
	// the array of pseudo-files : emulated files in the virtual zip file system
	// the handle to the file is its index inside this array.
	// some of the entries can be free. The entries need to be destructed manually
	typedef std::vector<CZipPseudoFile> ZipPseudoFileArray;
	ZipPseudoFileArray m_arrOpenFiles;

	// the array of file datas; they are relatively self-contained and can
	// read and cache the file data on-demand. It's up to the clients
	// to use caching or access the zips directly
	CCritSection m_csCachedFiles;
	typedef CMTSafeAllocator<CCachedFileData*> CachedFileDataAllocator;
	typedef std::set<CCachedFileData*,CCachedFileDataOrder, CachedFileDataAllocator > CachedFileDataSet;
	CachedFileDataSet m_setCachedFiles;

	// The F* emulation functions critical sectio: protects all F* functions
	// that don't have a chance to be called recursively (to avoid deadlocks)
	CCritSection m_csMain;

	// open zip cache objects that can be reused. They're self-[un]registered
	// they're sorted by the path and 
	typedef std::vector<ICryArchive*> ArchiveArray;
	ArchiveArray m_arrArchives;

	// the array of opened caches - they get destructed by themselves (these are auto-pointers, see the ZipDir::Cache documentation)
	struct PackDesc
	{
		string strBindRoot; // the zip binding root WITH the trailing native slash
		//string strFileName; // the zip file name (no path)

		const char* GetFullPath()const {return pZip->GetFilePath();}

		ICryArchive_AutoPtr pArchive;
		ZipDir::CachePtr pZip;
		size_t sizeofThis()
		{
			return strBindRoot.capacity() + pZip->GetSize();
		}
	};
	typedef std::vector<PackDesc > ZipArray;
	CCritSection m_csZips;
	ZipArray m_arrZips;
	friend class CCryPakFindData;

	typedef std::set<CCryPakFindData_AutoPtr> CryPakFindDataSet;
	CryPakFindDataSet m_setFindData;

	IMiniLog *m_pLog;

	// the root: "C:\MasterCD\"		  
	string m_strMasterCDRoot;

	// this is the list of MOD subdirectories that will be prepended to the actual relative file path
	// they all have trailing forward slash. "" means the root dir
	std::vector<string> m_arrMods;

	//////////////////////////////////////////////////////////////////////////
	// Opened files collector.
	//////////////////////////////////////////////////////////////////////////
	bool m_bRememberOpenedFiles;
	typedef std::set<string,stl::less_stricmp<string> > RecordedFilesSet;
	RecordedFilesSet m_recordedFilesSet;

	const PakVars* m_pPakVars;

  bool InitPack(const char *szBasePath, unsigned nFlags = FLAGS_PATH_REAL);

public:
	// given the source relative path, constructs the full path to the file according to the flags
	const char* AdjustFileName (const char *src, char dst[g_nMaxPath], unsigned nFlags,bool *bFoundInPak=NULL);

	// this is the start of indexation of pseudofiles: 
	// to the actual index , this offset is added to get the valid handle
	enum {g_nPseudoFileIdxOffset = 1};

	// this defines which slash will be kept internally
	enum {g_cNativeSlash = '\\', g_cNonNativeSlash = '/'};

  // makes the path lower-case and removes the duplicate and non native slashes
  // may make some other fool-proof stuff
  // may NOT write beyond the string buffer (may not make it longer)
  // returns: the pointer to the ending terminator \0
  static char* BeautifyPath(char* dst);

	CCryPak(IMiniLog* pLog,PakVars* pPakVars = NULL);
	~CCryPak();

	const PakVars* GetPakVars()const {return m_pPakVars;}

public:
	PakInfo* GetPakInfo();
	void FreePakInfo (PakInfo*);

	//! adds a mod to the list of mods
	void AddMod(const char* szMod);
	//! removes a mod from the list of mods
	void RemoveMod(const char* szMod);

	//! Puts the memory statistics into the given sizer object
	//! According to the specifications in interface ICrySizer
	void GetMemoryStatistics(ICrySizer *pSizer);

	// open the physical archive file - creates if it doesn't exist
	// returns NULL if it's invalid or can't open the file
	virtual ICryArchive* OpenArchive (const char* szPath, unsigned nFlags = 0);

	// returns the path to the archive in which the file was opened
	virtual const char* GetFileArchivePath (FILE* f);

	CCritSection& GetCachedFileLock() {return m_csCachedFiles;}

	void Register (CCachedFileData* p)
	{
		// actually, registration may only happen when the set is already locked, but for generality..
		AUTO_LOCK(m_csCachedFiles);
		m_setCachedFiles.insert (p);
	}

	void Unregister (CCachedFileData* p)
	{
		AUTO_LOCK(m_csCachedFiles);
		m_setCachedFiles.erase (p);
	}
  // ICryPak interface
  virtual bool Init (const char *szBasePath);
  virtual void Release();
  virtual bool OpenPack(const char *pName, unsigned nFlags = 0);
	virtual bool OpenPack(const char* szBindRoot, const char *pName, unsigned nFlags = 0);
	virtual bool OpenPackCommon(const char* szBindRoot, const char *pName, unsigned nFlags = 0);
	// after this call, the file will be unlocked and closed, and its contents won't be used to search for files
	virtual bool ClosePack(const char* pName, unsigned nFlags = 0);
	virtual bool OpenPacks(const char *pWildcard, unsigned nFlags = 0);
	virtual bool OpenPacks(const char* szBindRoot, const char *pWildcard, unsigned nFlags = 0);
	bool OpenPacksCommon(const char* szDir, char *cWork, unsigned nFlags);
	// closes pack files by the path and wildcard
	virtual bool ClosePacks(const char* pWildcard, unsigned nFlags = 0);

	// returns the file modification time
	virtual FILETIME GetModificationTime(FILE* f);


	// this function gets the file data for the given file, if found.
	// The file data object may be created in this function,
	// and it's important that the autoptr is returned: another thread may release the existing
	// cached data before the function returns
	// the path must be absolute normalized lower-case with forward-slashes
	CCachedFileDataPtr GetFileData(const char* szName);

	// tests if the given file path refers to an existing file inside registered (opened) packs
	// the path must be absolute normalized lower-case with forward-slashes
	bool HasFileEntry (const char* szPath);

  virtual FILE *FOpen(const char *pName, const char *mode, unsigned nFlags);
	virtual FILE *FOpen(const char *pName, const char *mode,char *szFileGamePath,int nLen);
  virtual size_t FRead(void *data, size_t length, size_t elems, FILE *handle);
  virtual size_t FWrite(void *data, size_t length, size_t elems, FILE *handle);
  virtual int FSeek(FILE *handle, long seek, int mode);
  virtual long FTell(FILE *handle);
  virtual int FFlush(FILE *handle);
  virtual int FClose(FILE *handle);
  virtual intptr_t FindFirst(const char *pDir, struct _finddata_t *fd);
  virtual int FindNext(intptr_t handle, struct _finddata_t *fd);
  virtual int FindClose(intptr_t handle);
  virtual int FEof(FILE *handle);
  virtual int FScanf(FILE *, const char *, ...);
  virtual char *FGets(char *, int, FILE *);
  virtual int Getc(FILE *);
  virtual int Ungetc(int c, FILE *);
  virtual int FPrintf(FILE *handle, const char *format, ...);
	unsigned FGetSize(FILE* f);
//	virtual bool IsOutOfDate(const char * szFileName, const char * szMasterFile);
	//virtual FILETIME GetFileTime(const char * szFileName);

	// creates a directory
	virtual bool MakeDir (const char* szPath);

	// returns the current game directory, with trailing slash (or empty string if it's right in MasterCD)
	// this is used to support Resource Compiler which doesn't have access to this interface:
	// in case all the contents is located in a subdirectory of MasterCD, this string is the subdirectory name with slash
	//virtual const char* GetGameDir();

	void Register (ICryArchive* pArchive);
	void Unregister (ICryArchive* pArchive);
	ICryArchive* FindArchive (const char* szFullPath);

	// compresses the raw data into raw data. The buffer for compressed data itself with the heap passed. Uses method 8 (deflate)
	// returns one of the Z_* errors (Z_OK upon success)
	// MT-safe
	int RawCompress (const void* pUncompressed, unsigned long* pDestSize, void* pCompressed, unsigned long nSrcSize, int nLevel = -1);

	// Uncompresses raw (without wrapping) data that is compressed with method 8 (deflated) in the Zip file
	// returns one of the Z_* errors (Z_OK upon success)
	// This function just mimics the standard uncompress (with modification taken from unzReadCurrentFile)
	// with 2 differences: there are no 16-bit checks, and 
	// it initializes the inflation to start without waiting for compression method byte, as this is the 
	// way it's stored into zip file
	int RawUncompress (void* pUncompressed, unsigned long* pDestSize, const void* pCompressed, unsigned long nSrcSize) ;

	//////////////////////////////////////////////////////////////////////////
	// Files opening recorder.
	//////////////////////////////////////////////////////////////////////////
	void RecordFileOpen( bool bEnable );
	void RecordFile( const char *szFilename );
	void EnumerateRecordedFiles( RecordedFilesEnumCallback enumCallback );

	void OnMissingFile (const char* szPath);
	// missing file -> count of missing files
	typedef CMTSafeAllocator<std::pair<string, unsigned> > MissingFileMapAllocator;
	typedef std::map<string, unsigned, std::less<string>, MissingFileMapAllocator > MissingFileMap;
	MissingFileMap m_mapMissingFiles;
};

struct CryArchiveSortByName
{
	bool operator () (const ICryArchive* left, const ICryArchive* right)const
	{
		return stricmp(left->GetFullPath(), right->GetFullPath()) < 0;
	}
	bool operator () (const char* left, const ICryArchive* right)const
	{
		return stricmp(left, right->GetFullPath()) < 0;
	}
	bool operator () (const ICryArchive* left, const char* right)const
	{
		return stricmp(left->GetFullPath(), right) < 0;
	}
};

template <class Cache>
class TCryArchive: public ICryArchive
{			
public:
	TCryArchive (CCryPak*pPak, const string& strBindRoot, Cache* pCache, unsigned nFlags = 0):
		m_pCache(pCache),
		m_strBindRoot (strBindRoot),
		m_pPak(pPak),
		m_nFlags (nFlags)
	{
		pPak->Register(this);
	}

	~TCryArchive()
	{
		m_pPak->Unregister (this);
	}

	// finds the file; you don't have to close the returned handle
	Handle FindFile (const char* szRelativePath)
	{
		char szFullPath[CCryPak::g_nMaxPath];
		const char*pPath = AdjustPath (szRelativePath, szFullPath);
		if (!pPath)
			return NULL;
		return m_pCache->FindFile(pPath);
	}
	
	// returns the size of the file (unpacked) by the handle
	unsigned GetFileSize (Handle h)
	{
		assert (m_pCache->IsOwnerOf((ZipDir::FileEntry*)h));
		return ((ZipDir::FileEntry*)h)->desc.lSizeUncompressed;
	}

	// reads the file into the preallocated buffer (must be at least the size of GetFileSize())
	int ReadFile (Handle h, void* pBuffer)
	{
		assert (m_pCache->IsOwnerOf((ZipDir::FileEntry*)h));
		return m_pCache->ReadFile ((ZipDir::FileEntry*)h, NULL, pBuffer);
	}

	// returns the full path to the archive file
	const char* GetFullPath() const
	{
		return m_pCache->GetFilePath();
	}

	unsigned GetFlags() const {return m_nFlags;}
	bool SetFlags(unsigned nFlagsToSet)
	{
		if (nFlagsToSet & FLAGS_RELATIVE_PATHS_ONLY)
			m_nFlags |= FLAGS_RELATIVE_PATHS_ONLY;

		if (nFlagsToSet & ~(FLAGS_RELATIVE_PATHS_ONLY))
		{
			// we don't support changing of any other flags
			return false;
		}
		return true;
	}

	bool ResetFlags (unsigned nFlagsToReset)
	{
		if (nFlagsToReset & FLAGS_RELATIVE_PATHS_ONLY)
			m_nFlags &= ~FLAGS_RELATIVE_PATHS_ONLY;

		if (nFlagsToReset & ~(FLAGS_RELATIVE_PATHS_ONLY))
		{
			// we don't support changing of any other flags
			return false;
		}
		return true;
	}


	Cache* GetCache() {return m_pCache;}
protected:
	// returns the pointer to the relative file path to be passed
	// to the underlying Cache pointer. Uses the given buffer to construct the path.
	// returns NULL if the file path is invalid
	const char* AdjustPath (const char* szRelativePath, char szFullPathBuf[CCryPak::g_nMaxPath])
	{
		if (!szRelativePath[0])
			return NULL;

		if (m_nFlags & FLAGS_RELATIVE_PATHS_ONLY)
			return szRelativePath;

		if (szRelativePath[1] == ':' || (m_nFlags & FLAGS_ABSOLUTE_PATHS))
		{
			// make the normalized full path and try to match it against the binding root of this object
			const char* szFullPath = m_pPak->AdjustFileName (szRelativePath, szFullPathBuf, m_nFlags & FLAGS_IGNORE_MODS ? ICryPak::FLAGS_PATH_REAL : 0);
			size_t nPathLen = strlen(szFullPath);
			if (nPathLen <= m_strBindRoot.length())
				return NULL;

			// you should access exactly the file under the directly in which the zip is situated
			if (szFullPath[m_strBindRoot.length()] != '/' && szFullPath[m_strBindRoot.length()] != '\\')
				return NULL;
#if defined(LINUX)
			if (comparePathNames(szFullPath, m_strBindRoot.c_str(), m_strBindRoot.length()))
#else
			if (memicmp(szFullPath, m_strBindRoot.c_str(), m_strBindRoot.length()))
#endif
				return NULL; // the roots don't match

			return szFullPath + m_strBindRoot.length() + 1;
		}

		return szRelativePath;
	}
protected:
	_smart_ptr<Cache> m_pCache;
	// the binding root may be empty string - in this case, the absolute path binding won't work
	string m_strBindRoot;
	CCryPak* m_pPak;
	unsigned m_nFlags;
};


class CryArchiveRW: public TCryArchive<ZipDir::CacheRW>
{
public:

	CryArchiveRW (CCryPak*pPak, const string& strBindRoot, ZipDir::CacheRW* pCache, unsigned nFlags = 0):
		TCryArchive<ZipDir::CacheRW>(pPak, strBindRoot, pCache, nFlags)
	{
	}

	~CryArchiveRW()
	{
	}

	// Adds a new file to the zip or update an existing one
	// adds a directory (creates several nested directories if needed)
	// compression methods supported are 0 (store) and 8 (deflate) , compression level is 0..9 or -1 for default (like in zlib)
	int UpdateFile (const char* szRelativePath, void* pUncompressed, unsigned nSize, unsigned nCompressionMethod = 0, int nCompressionLevel = -1);

	// deletes the file from the archive
	int RemoveFile (const char* szRelativePath);

	// deletes the directory, with all its descendants (files and subdirs)
	int RemoveDir (const char* szRelativePath);

	int RemoveAll();

	enum {gClassId = 1};
	unsigned GetClassId()const {return gClassId;}
};


class CryArchive: public TCryArchive<ZipDir::Cache>
{
public:
	CryArchive (CCryPak* pPak, const string& strBindRoot, ZipDir::Cache* pCache, unsigned nFlags):
		TCryArchive<ZipDir::Cache>(pPak, strBindRoot, pCache, nFlags)
	{	}

	~CryArchive(){}

	// Adds a new file to the zip or update an existing one
	// adds a directory (creates several nested directories if needed)
	// compression methods supported are METHOD_STORE == 0 (store) and
	// METHOD_DEFLATE == METHOD_COMPRESS == 8 (deflate) , compression
	// level is LEVEL_FASTEST == 0 till LEVEL_BEST == 9 or LEVEL_DEFAULT == -1
	// for default (like in zlib)
	int UpdateFile (const char* szRelativePath, void* pUncompressed, unsigned nSize, unsigned nCompressionMethod = 0, int nCompressionLevel = -1) {return ZipDir::ZD_ERROR_INVALID_CALL;}

	// deletes the file from the archive
	int RemoveFile (const char* szRelativePath) {return ZipDir::ZD_ERROR_INVALID_CALL;}
	int RemoveAll();

	// deletes the directory, with all its descendants (files and subdirs)
	int RemoveDir (const char* szRelativePath) {return ZipDir::ZD_ERROR_INVALID_CALL;}
	enum {gClassId = 2};
	unsigned GetClassId()const {return gClassId;}

};

#endif