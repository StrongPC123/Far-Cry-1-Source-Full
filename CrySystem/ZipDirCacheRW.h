//////////////////////////////////////////////////////////////////////////
// Declaration of the class that will keep the ZipDir Cache object
// and will provide all its services to access Zip file, plus it will
// provide services to write to the zip file efficiently
// Time to time, the contained Cache object will be recreated during 
// an archive add operation

#ifndef _ZIP_DIR_CACHE_RW_HDR_
#define _ZIP_DIR_CACHE_RW_HDR_

namespace ZipDir
{

struct FileDataRecord;
TYPEDEF_AUTOPTR(FileDataRecord);
typedef FileDataRecord_AutoPtr FileDataRecordPtr;


class CacheRW: public _reference_target_MT_novtbl<CacheRW>
{
public:
	// the size of the buffer that's using during re-linking the zip file
	enum {g_nSizeRelinkBuffer = 1024*1024,
		g_nMaxItemsRelinkBuffer = 128 // max number of files to read before (without) writing
	};

	CacheRW():
		m_pFile (NULL),
		m_pHeap (NULL),
		m_nFlags (0),
		m_lCDROffset (0)
	{
	}

	~CacheRW()
	{
		Close();
	}

	bool IsValid ()const
	{
		return m_pFile != NULL;
	}

	// opens the given zip file and connects to it. Creates a new file if no such file exists
	// if successful, returns true.
	//ErrorEnum Open (CMTSafeHeap* pHeap, InitMethodEnum nInitMethod, unsigned nFlags, const char* szFile);

	// Adds a new file to the zip or update an existing one
	// adds a directory (creates several nested directories if needed)
	ErrorEnum UpdateFile (const char* szRelativePath, void* pUncompressed, unsigned nSize, unsigned nCompressionMethod = ZipFile::METHOD_STORE, int nCompressionLevel = -1);

	// deletes the file from the archive
	ErrorEnum RemoveFile (const char* szRelativePath);

	// deletes the directory, with all its descendants (files and subdirs)
	ErrorEnum RemoveDir (const char* szRelativePath);

	// deletes all files and directories in this archive
	ErrorEnum RemoveAll();

	// closes the current zip file
	void Close();

	FileEntry* FindFile (const char* szPath, bool bFullInfo = false);

	ErrorEnum ReadFile (FileEntry* pFileEntry, void* pCompressed, void* pUncompressed);

	void* AllocAndReadFile (FileEntry* pFileEntry);

	void Free (void*p)
	{
		m_pHeap->Free(p);
	}

	// refreshes information about the given file entry into this file entry
	ErrorEnum Refresh (FileEntry* pFileEntry);

	// returns the size of memory occupied by the instance of this cache
	size_t GetSize()const;

	// QUICK check to determine whether the file entry belongs to this object
	bool IsOwnerOf (const FileEntry*pFileEntry)const
	{
		return m_treeDir.IsOwnerOf(pFileEntry);
	}

	// returns the string - path to the zip file from which this object was constructed.
	// this will be "" if the object was constructed with a factory that wasn't created with FLAGS_MEMORIZE_ZIP_PATH
	const char* GetFilePath()const
	{
		return m_strFilePath.c_str();
	}

	FileEntryTree* GetRoot()
	{
		return &m_treeDir;
	}
	
	// writes the CDR to the disk
	bool WriteCDR() {return WriteCDR(m_pFile);}
	bool WriteCDR(FILE* fTarget);

	bool RelinkZip();
protected:
	bool RelinkZip(FILE* fTmp);
	// writes out the file data in the queue into the given file. Empties the queue
	bool WriteZipFiles(std::vector<FileDataRecordPtr>& queFiles, FILE* fTmp);
	// generates random file name
	string GetRandomName(int nAttempt);

protected:
	friend class CacheFactory;
	FileEntryTree m_treeDir;
	FILE *m_pFile;
	CMTSafeHeap* m_pHeap;
	string m_strFilePath;

	// offset to the start of CDR in the file,even if there's no CDR there currently
	// when a new file is added, it can start from here, but this value will need to be updated then
	ZipFile::ulong m_lCDROffset;

	enum
	{
		// if this is set, the file needs to be compacted before it can be used by 
		// all standard zip tools, because gaps between file datas can be present
		FLAGS_UNCOMPACTED = 1 << 0,
		// if this is set, the CDR needs to be written to the file
		FLAGS_CDR_DIRTY   = 1 << 1,
		// if this is set, the file is opened in read-only mode. no write operations are to be performed
		FLAGS_READ_ONLY   = 1 << 2,
		// when this is set, compact operation is not performed
		FLAGS_DONT_COMPACT = 1 << 3
	};
	unsigned m_nFlags;
};

TYPEDEF_AUTOPTR(CacheRW);
typedef CacheRW_AutoPtr CacheRWPtr;

// creates and if needed automatically destroys the file entry
class FileEntryTransactionAdd
{
	class CacheRW* m_pCache;
	const char* m_szRelativePath;
	FileEntry* m_pFileEntry;
	bool m_bComitted;
public:
	operator FileEntry* () {return m_pFileEntry;}
	operator bool()const{return m_pFileEntry != NULL;}
	FileEntry* operator -> () {return m_pFileEntry;}
	FileEntryTransactionAdd(class CacheRW* pCache, const char* szRelativePath):
		m_pCache(pCache),
		m_szRelativePath(szRelativePath),
		m_bComitted (false)
	{
		// this is the name of the directory - create it or find it
		m_pFileEntry = m_pCache->GetRoot()->Add (szRelativePath);
	}
	~FileEntryTransactionAdd()
	{
		if (m_pFileEntry && !m_bComitted)
			m_pCache->RemoveFile(m_szRelativePath);
	}
	void Commit()
	{
		m_bComitted = true;
	}
};


}

#endif