//////////////////////////////////////////////////////////////////////////
// This file contains only the support definitions for CZipDir class
// implementation. This it to unload the ZipDir.h from secondary stuff.
#ifndef _ZIP_DIR_DEFINITIONS
#define _ZIP_DIR_DEFINITIONS

#if defined(LINUX)
	#include <ctype.h>
#endif

namespace ZipDir
{

// possible errors occuring during the method execution
// to avoid clushing with the global Windows defines, we prefix these with ZD_
enum ErrorEnum
{
	ZD_ERROR_SUCCESS = 0,
	ZD_ERROR_IO_FAILED,
	ZD_ERROR_UNEXPECTED,
	ZD_ERROR_UNSUPPORTED,
	ZD_ERROR_INVALID_SIGNATURE,
	ZD_ERROR_ZIP_FILE_IS_CORRUPT,
	ZD_ERROR_DATA_IS_CORRUPT,
	ZD_ERROR_NO_CDR,
	ZD_ERROR_CDR_IS_CORRUPT,
	ZD_ERROR_NO_MEMORY,
	ZD_ERROR_VALIDATION_FAILED,
	ZD_ERROR_CRC32_CHECK,
	ZD_ERROR_ZLIB_FAILED,
	ZD_ERROR_ZLIB_CORRUPTED_DATA,
	ZD_ERROR_ZLIB_NO_MEMORY,
	ZD_ERROR_CORRUPTED_DATA,
	ZD_ERROR_INVALID_CALL,
	ZD_ERROR_NOT_IMPLEMENTED,
	ZD_ERROR_FILE_NOT_FOUND,
	ZD_ERROR_DIR_NOT_FOUND,
	ZD_ERROR_NAME_TOO_LONG,
	ZD_ERROR_INVALID_PATH,
	ZD_ERROR_FILE_ALREADY_EXISTS
};

// the error describes the reason of the error, as well as the error code, line of code where it happened etc.
struct Error
{
	Error(ErrorEnum _nError, const char* _szDescription, const char* _szFunction, const char* _szFile, unsigned _nLine):
		nError(_nError),
		m_szDescription(_szDescription),
		szFunction(_szFunction),
		szFile(_szFile),
		nLine(_nLine)
	{
	}

	ErrorEnum nError;
	const char* getError();

	const char* getDescription() {return m_szDescription;}
	const char* szFunction, *szFile;
	unsigned nLine;
protected:
	// the description of the error; if needed, will be made as a dynamic string
	const char* m_szDescription;
};

#define THROW_ZIPDIR_ERROR(ZD_ERR,DESC) throw Error (ZD_ERR, DESC, __FUNCTION__, __FILE__, __LINE__)


// possible initialization methods
enum InitMethodEnum
{
	// initialize as fast as possible, with minimal validation
	ZD_INIT_FAST,
	// after initialization, scan through all file headers, precache the actual file data offset values and validate the headers
	ZD_INIT_FULL,
	// scan all file headers and try to decompress the data, searching for corrupted files
	ZD_INIT_VALIDATE,
	// maximum level of validation, checks for integrity of the archive
	ZD_INIT_VALIDATE_MAX = ZD_INIT_VALIDATE
};

typedef void* (*FnAlloc) (void* pUserData, unsigned nItems, unsigned nSize);
typedef void (*FnFree)  (void* pUserData, void* pAddress);

//////////////////////////////////////////////////////////////////////////
// This structure contains the pointers to functions for memory management
// by default, it's initialized to default malloc/free
#if 0
struct Allocator
{
	FnAlloc fnAlloc;
	FnFree fnFree;
	void* pOpaque;

	static void* DefaultAlloc (void*, unsigned nItems, unsigned nSize)
	{
		return malloc (nItems*nSize);
	}

	static void DefaultFree (void*, void* pAddress)
	{
		free (pAddress);
	}

	void* Alloc (unsigned nItems, unsigned nSize)
	{
		return this->fnAlloc(this->pOpaque, nItems, nSize);
	}

	void Free (void*pAddress)
	{
		this->fnFree (this->pOpaque, pAddress);
	}

	// constructs the allocator object; by default, the stdlib functions are used
	Allocator (FnAlloc fnAllocIn = DefaultAlloc, FnFree fnFreeIn = DefaultFree, void* pOpaqueIn = NULL):
		fnAlloc(fnAllocIn),
		fnFree (fnFreeIn),
		pOpaque(pOpaqueIn)
	{
	}
};
#endif
// instance of this class just releases the memory when it's destructed
template <class _Heap>
struct TSmartHeapPtr
{
	TSmartHeapPtr(_Heap* pHeap):
		m_pHeap(pHeap),
		m_pAddress(NULL)
	{
	}
	~TSmartHeapPtr()
	{
		Release();
	}

	void Attach (void*p)
	{
		Release();
		m_pAddress = p;
	}

	void* Detach()
	{
		void* p = m_pAddress;
		m_pAddress = NULL;
		return p;
	}

	void Release()
	{
		if (m_pAddress)
		{
			m_pHeap->Free(m_pAddress);
			m_pAddress = NULL;
		}
	}
protected:
	// the pointer to free
	void* m_pAddress;
	_Heap* m_pHeap;
};

typedef TSmartHeapPtr<CMTSafeHeap> SmartPtr;

// Uncompresses raw (without wrapping) data that is compressed with method 8 (deflated) in the Zip file
// returns one of the Z_* errors (Z_OK upon success)
extern int ZipRawUncompress (CMTSafeHeap*pHeap, void* pUncompressed, unsigned long* pDestSize, const void* pCompressed, unsigned long nSrcSize);

// compresses the raw data into raw data. The buffer for compressed data itself with the heap passed. Uses method 8 (deflate)
// returns one of the Z_* errors (Z_OK upon success), and the size in *pDestSize. the pCompressed buffer must be at least nSrcSize*1.001+12 size
extern int ZipRawCompress (CMTSafeHeap*pHeap, const void* pUncompressed, unsigned long* pDestSize, void* pCompressed, unsigned long nSrcSize, int nLevel);

// this is the record about the file in the Zip file.
struct FileEntry
{
	enum {INVALID_DATA_OFFSET = 0xFFFFFFFF};
	
	ZipFile::DataDescriptor desc;
	ZipFile::ulong nFileHeaderOffset; // offset of the local file header
	ZipFile::ulong nFileDataOffset; // offsed of the packed info inside the file; NOTE: this can be INVALID_DATA_OFFSET, if not calculated yet!
	ZipFile::ushort nMethod; // the method of compression (0 if no compression/store)
	ZipFile::ushort nNameOffset; // offset of the file name in the name pool for the directory

	// the file modification times
	ZipFile::ushort nLastModTime;
	ZipFile::ushort nLastModDate;

	// the offset to the start of the next file's header - this
	// can be used to calculate the available space in zip file
	ZipFile::ulong nEOFOffset;


	FileEntry():nFileHeaderOffset(INVALID_DATA_OFFSET){}
	FileEntry(const ZipFile::CDRFileHeader& header);

	bool IsInitialized ()
	{
		// structure marked as non-initialized should have nFileHeaderOffset == INVALID_DATA_OFFSET
		return nFileHeaderOffset != INVALID_DATA_OFFSET;
	}
	// returns the name of this file, given the pointer to the name pool
	const char* GetName(const char* pNamePool)const
	{
		return pNamePool + nNameOffset;
	}

	// sets the current time to modification time
	// calculates CRC32 for the new data
	void OnNewFileData(void* pUncompressed, unsigned nSize, unsigned nCompressedSize, unsigned nCompressionMethod);

	FILETIME GetModificationTime();
};

// tries to refresh the file entry from the given file (reads fromthere if needed)
// returns the error code if the operation was impossible to complete
extern ErrorEnum Refresh (FILE*f, FileEntry* pFileEntry);

// writes into the file local header (NOT including the name, only the header structure)
// the file must be opened both for reading and writing
extern ErrorEnum UpdateLocalHeader (FILE*f, FileEntry* pFileEntry);

// writes into the file local header - without Extra data
// puts the new offset to the file data to the file entry
// in case of error can put INVALID_DATA_OFFSET into the data offset field of file entry
extern ErrorEnum WriteLocalHeader (FILE*f, FileEntry* pFileEntry, const char* szRelativePath);

// conversion routines for the date/time fields used in Zip
extern ZipFile::ushort DOSDate(tm*);
extern ZipFile::ushort DOSTime(tm*);

extern const char* DOSTimeCStr(ZipFile::ushort nTime);
extern const char* DOSDateCStr(ZipFile::ushort nTime);

struct DirHeader;
// this structure represents a subdirectory descriptor in the directory record.
// it points to the actual directory info (list of its subdirs and files), as well 
// as on its name
struct DirEntry
{
	ZipFile::ulong  nDirHeaderOffset; // offset, in bytes, relative to this object, of the actual directory record header
	ZipFile::ushort nNameOffset; // offset of the dir name in the name pool of the parent directory
	ZipFile::ushort nPadBytes; // just padding

	// returns the name of this directory, given the pointer to the name pool of hte parent directory
	const char* GetName(const char* pNamePool)const
	{
		return pNamePool + nNameOffset;
	}

	// returns the pointer to the actual directory record. 
	// call this function only for the actual structure instance contained in a directory record and
	// followed by the other directory records
	const DirHeader* GetDirectory ()	const
	{
		return (const DirHeader*)(((const char*)this) + nDirHeaderOffset);
	}
	DirHeader* GetDirectory ()	
	{
		return (DirHeader*)(((char*)this) + nDirHeaderOffset);
	}
};

// this is the head of the directory record
// the name pool follows straight the directory and file entries.
struct DirHeader
{
	ZipFile::ushort numDirs; // number of directory entries - DirEntry structures
	ZipFile::ushort numFiles; // number of file entries - FileEntry structures

	// returns the pointer to the name pool that follows this object
	// you can only call this method for the structure instance actually followed by the dir record
	const char* GetNamePool()const
	{
		return ((char*)(this+1)) + this->numDirs * sizeof(DirEntry) + this->numFiles * sizeof(FileEntry);
	}
	char* GetNamePool()
	{
		return ((char*)(this+1)) + this->numDirs * sizeof(DirEntry) + this->numFiles * sizeof(FileEntry);
	}

	// returns the pointer to the i-th directory
	// call this only for the actual instance of the structure at the head of dir record
	const DirEntry* GetSubdirEntry(unsigned i)const
	{
		assert (i < numDirs);
		return ((const DirEntry*)(this+1)) + i;
	}
	DirEntry* GetSubdirEntry(unsigned i)
	{
		assert (i < numDirs);
		return ((DirEntry*)(this+1)) + i;
	}

	// returns the pointer to the i-th file
	// call this only for the actual instance of the structure at the head of dir record
	const FileEntry* GetFileEntry (unsigned i)const 
	{
		assert (i < numFiles);
		return (const FileEntry*)(((const DirEntry*)(this+1)) + numDirs) + i;
	}
	FileEntry* GetFileEntry (unsigned i)
	{
		assert (i < numFiles);
		return (FileEntry*)(((DirEntry*)(this+1)) + numDirs) + i;
	}

	// finds the subdirectory entry by the name, using the names from the name pool
	// assumes: all directories are sorted in alphabetical order.
	// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
	DirEntry* FindSubdirEntry(const char* szName);

	// finds the file entry by the name, using the names from the name pool
	// assumes: all directories are sorted in alphabetical order.
	// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
	FileEntry* FindFileEntry(const char* szName);

};

// this is the sorting predicate for directory entries
struct DirEntrySortPred
{
	DirEntrySortPred (const char* pNamePool):
		m_pNamePool (pNamePool)
		{
		}

	bool operator () (const FileEntry& left, const FileEntry& right)const
	{
		return strcmp(left.GetName(m_pNamePool), right.GetName(m_pNamePool)) < 0;
	}

	bool operator () (const FileEntry& left, const char* szRight)const
	{
		return strcmp(left.GetName(m_pNamePool), szRight) < 0;
	}

	bool operator () (const char* szLeft, const FileEntry& right)const
	{
		return strcmp(szLeft, right.GetName(m_pNamePool)) < 0;
	}

	bool operator () (const DirEntry& left, const DirEntry& right)const
	{
		return strcmp(left.GetName(m_pNamePool), right.GetName(m_pNamePool)) < 0;
	}

	bool operator () (const DirEntry& left, const char* szName)const
	{
		return strcmp(left.GetName(m_pNamePool), szName) < 0;
	}

	bool operator () (const char* szLeft, const DirEntry& right)const
	{
		return strcmp(szLeft, right.GetName(m_pNamePool)) < 0;
	}

	const char *m_pNamePool;
};

inline void tolower (string& str)
{
	for (string::iterator it = str.begin(); it != str.end(); ++it)
		*it = ::tolower(*it);
}

}

#endif