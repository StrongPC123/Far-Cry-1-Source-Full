// Interface to the crytek pack files management

#ifndef _ICRY_PAK_HDR_
#define _ICRY_PAK_HDR_

#include <smartptr.h>
#ifdef LINUX
	#include <winbase.h>
	#include <stdint.h>
#endif

// this represents one particular archive filcare
struct ICryArchive: public _reference_target_t
{
	// Compression methods
	enum ECompressionMethods
	{
		METHOD_STORE    = 0,
		METHOD_COMPRESS = 8,
		METHOD_DEFLATE  = 8
	};

	// Compression levels
	enum ECompressionLevels
	{
		LEVEL_FASTEST = 0,
		LEVEL_FASTER  = 2,
		LEVEL_NORMAL  = 8,
		LEVEL_BETTER  = 8,
		LEVEL_BEST    = 9,
		LEVEL_DEFAULT = -1
	};

	enum EPakFlags
	{
		// support for absolute and other complex path specifications -
		// all paths will be treated relatively to the current directory (normally MasterCD)
		FLAGS_ABSOLUTE_PATHS = 1,

		// if this is set, the object will only understand relative to the zip file paths,
		// but this can give an opportunity to optimize for frequent quick accesses
		// FLAGS_SIMPLE_RELATIVE_PATHS and FLAGS_ABSOLUTE_PATHS are mutually exclusive
		FLAGS_RELATIVE_PATHS_ONLY = 1 << 1,

		// if this flag is set, the archive update/remove operations will not work
		// this is useful when you open a read-only or already opened for reading files.
		// If FLAGS_OPEN_READ_ONLY | FLAGS_SIMPLE_RELATIVE_PATHS are set, ICryPak 
		// will try to return an object optimized for memory, with long life cycle
		FLAGS_READ_ONLY = 1 << 2,

		// if this flag is set, FLAGS_OPEN_READ_ONLY
		// flags are also implied. The returned object will be optimized for quick access and
		// memory footprint
		FLAGS_OPTIMIZED_READ_ONLY = (1 << 3),

		// if this is set, the existing file (if any) will be overwritten
		FLAGS_CREATE_NEW = 1 << 4,

		// if this flag is set, and the file is opened for writing, and some files were updated
		// so that the archive is no more continuous, the archive will nevertheless NOT be compacted
		// upon closing the archive file. This can be faster if you open/close the archive for writing 
		// multiple times
		FLAGS_DONT_COMPACT   = 1 << 5,

		// if this is set, CryPak doesn't try to transform the path according to the MOD subdirectories
		FLAGS_IGNORE_MODS    = 1 << 6
	};

	virtual ~ICryArchive(){}

	// Summary:
	//   Adds a new file to the zip or update an existing one.
	// Description:
	//   Adds a new file to the zip or update an existing one
	//   adds a directory (creates several nested directories if needed)
	//   compression methods supported are METHOD_STORE == 0 (store) and
	//   METHOD_DEFLATE == METHOD_COMPRESS == 8 (deflate) , compression
	//   level is LEVEL_FASTEST == 0 till LEVEL_BEST == 9 or LEVEL_DEFAULT == -1
	//   for default (like in zlib)
	virtual int UpdateFile (const char* szRelativePath, void* pUncompressed, unsigned nSize, unsigned nCompressionMethod = 0, int nCompressionLevel = -1) = 0;

	// Summary:
	//   Deletes the file from the archive.
	virtual int RemoveFile (const char* szRelativePath) = 0;

	// Summary:
	//   Deletes the directory, with all its descendants (files and subdirs).
	virtual int RemoveDir (const char* szRelativePath) = 0;

	// Summary:
	//   Deletes all files and directories in the archive.
	virtual int RemoveAll() = 0;

	typedef void* Handle;

	// Summary:
	//   Finds the file; you don't have to close the returned handle.
	// Returns:
	//   NULL if the file doesn't exist
	virtual Handle FindFile (const char* szPath) = 0;
	// Summary:
	//   Get the file size (uncompressed).
	// Returns:
	//   The size of the file (unpacked) by the handle
	virtual unsigned GetFileSize (Handle) = 0;
	// Summary:
	//   Reads the file into the preallocated buffer
	// Note:
	//    Must be at least the size returned by GetFileSize.
	virtual int ReadFile (Handle, void* pBuffer) = 0;

	// Summary:
	//   Get the full path to the archive file.
	virtual const char* GetFullPath() const = 0;

	// Summary:
	//   Get the flags of this object.
	// Description:
	//   The possibles flags are defined in EPakFlags.
	// See Also:
	//   SetFlags, ResetFlags
	virtual unsigned GetFlags() const = 0;

	// Summary:
	//   Sets the flags of this object.
	// Description:
	//   The possibles flags are defined in EPakFlags.
	// See Also:
	//   GetFlags, ResetFlags
	virtual bool SetFlags(unsigned nFlagsToSet) = 0;

	// Summary:
	//   Resets the flags of this object.
	// See Also:
	//   SetFlags, GetFlags
	virtual bool ResetFlags(unsigned nFlagsToSet) = 0;

	// Summary:
	//   Determines if the archive is read only.
	// Returns:
	//   true if this archive is read-only
	inline bool IsReadOnly()const {return (GetFlags() & FLAGS_READ_ONLY) != 0;}

	// Summary:
	//   Get the class id.
	virtual unsigned GetClassId()const = 0; 

};

TYPEDEF_AUTOPTR(ICryArchive);

// this special flag is used for findfirst/findnext routines
// to mark the files that were actually found in Archive
enum {_A_IN_CRYPAK = 0x80000000};

struct ICryPak
{
	// Flags used in file path resolution rules
	enum EPathResolutionRules
	{
		// If used, the source path will be treated as the destination path
		// and no transformations will be done. Pass this flag when the path is to be the actual
		// path on the disk/in the packs and doesn't need adjustment (or after it has come through adjustments already)
		FLAGS_PATH_REAL = 1 << 16,

		// AdjustFileName will always copy the file path to the destination path:
		// regardless of the returned value, szDestpath can be used
		FLAGS_COPY_DEST_ALWAYS = 1 << 17,

		// Adds trailing slash to the path
		FLAGS_ADD_TRAILING_SLASH = 1 << 18,

		// Doesn't take mods into account
		FLAGS_IGNORE_MOD_DIRS    = 1 << 19,

		// Search only in MODs; if not found, return NULL; may not be used with FLAGS_IGNORE_MOD_DIRS
		FLAGS_ONLY_MOD_DIRS      = 1 << 20
	};

	// Used for widening FOpen functionality. They're ignored for the regular File System files.
	enum EFOpenFlags
	{
		// If possible, will prevent the file from being read from memory.
		FOPEN_HINT_DIRECT_OPERATION = 1,
		// Will prevent a "missing file" warnings to be created.
		FOPEN_HINT_QUIET = 1 << 1
	};


	//! the size of the buffer that receives the full path to the file
	enum {g_nMaxPath = 0x800};

	// given the source relative path, constructs the full path to the file according to the flags
	// returns the pointer to the constructed path (can be either szSourcePath, or szDestPath, or NULL in case of error
	const char* AdjustFileName(const char *szSourcePath, char szDestPath[g_nMaxPath], unsigned nFlags = 0);

  virtual bool Init (const char *szBasePath)=0;
  virtual void Release()=0;
	//! after this call, the pak file will be searched for files when they aren't on the OS file system
  virtual bool OpenPack(const char *pName, unsigned nFlags = FLAGS_PATH_REAL)=0;
	//! after this call, the pak file will be searched for files when they aren't on the OS file system
	virtual bool OpenPack(const char* pBindingRoot, const char *pName, unsigned nFlags = FLAGS_PATH_REAL)=0;
	//! after this call, the file will be unlocked and closed, and its contents won't be used to search for files
	virtual bool ClosePack(const char* pName, unsigned nFlags = FLAGS_PATH_REAL) = 0;
	//! opens pack files by the path and wildcard
	virtual bool OpenPacks(const char *pWildcard, unsigned nFlags = FLAGS_PATH_REAL)=0;
	//! opens pack files by the path and wildcard
	virtual bool OpenPacks(const char* pBindingRoot, const char *pWildcard, unsigned nFlags = FLAGS_PATH_REAL)=0;
	//! closes pack files by the path and wildcard
	virtual bool ClosePacks(const char* pWildcard, unsigned nFlags = FLAGS_PATH_REAL) = 0;
	//! adds a mod to the list
	virtual void AddMod(const char* szMod)=0;
	//! removes a mod from the list
	virtual void RemoveMod(const char* szMod)=0;

	struct PakInfo
	{
		struct Pak
		{
			const char* szFilePath;
			const char* szBindRoot;
			size_t nUsedMem;
		};
		// the number of packs in the arrPacks array
		unsigned numOpenPaks;
		// the packs
		Pak arrPaks[1];
	};
	// returns an array of PackInfo structures inside OpenPacks structure
	// you MUST call FreeOpenPackInfo
	virtual PakInfo* GetPakInfo() = 0;
	virtual void FreePakInfo (PakInfo*) = 0;

  virtual FILE *FOpen(const char *pName, const char *mode, unsigned nFlags = 0)=0;
	virtual FILE *FOpen(const char *pName, const char *mode,char *szFileGamePath,int nLen)=0;
  virtual size_t FRead(void *data, size_t length, size_t elems, FILE *handle)=0;
  virtual size_t FWrite(void *data, size_t length, size_t elems, FILE *handle)=0;
  virtual int FSeek(FILE *handle, long seek, int mode)=0;
  virtual long FTell(FILE *handle)=0;
  virtual int FClose(FILE *handle)=0;
  virtual int FEof(FILE *handle)=0;
  virtual int FFlush(FILE *handle)=0;
  //virtual int FScanf(FILE *, const char *, ...)=0;
  virtual int FPrintf(FILE *handle, const char *format, ...)=0;
  virtual char *FGets(char *, int, FILE *)=0;
  virtual int Getc(FILE *)=0;
  virtual int Ungetc(int c, FILE *)=0;

  virtual intptr_t 
  FindFirst(
  const char *pDir, 
  struct _finddata_t *fd
  )=0;

  virtual int FindNext(intptr_t handle, struct _finddata_t *fd)=0;
  virtual int FindClose(intptr_t handle)=0;
//	virtual bool IsOutOfDate(const char * szCompiledName, const char * szMasterFile)=0;
	//returns file modification time
	virtual FILETIME GetModificationTime(FILE*f)=0;

	// creates a directory
	virtual bool MakeDir (const char* szPath) = 0;
		
	// open the physical archive file - creates if it doesn't exist
	// returns NULL if it's invalid or can't open the file
	// nFlags can have the following flag set:
	//   FLAGS_ABSOLUTE_PATHS	
	virtual ICryArchive* OpenArchive (const char* szPath, unsigned nFlags = FLAGS_PATH_REAL) = 0;

	// returns the current game directory, with trailing slash (or empty string if it's right in MasterCD)
	// this is used to support Resource Compiler which doesn't have access to this interface:
	// in case all the contents is located in a subdirectory of MasterCD, this string is the subdirectory name with slash
	//virtual const char* GetGameDir() = 0;

	// returns the path to the archive in which the file was opened
	// returns NULL if the file is a physical file, and "" if the path to archive is unknown (shouldn't ever happen)
	virtual const char* GetFileArchivePath (FILE* f) = 0;

	// compresses the raw data into raw data. The buffer for compressed data itself with the heap passed. Uses method 8 (deflate)
	// returns one of the Z_* errors (Z_OK upon success)
	// MT-safe
	virtual int RawCompress (const void* pUncompressed, unsigned long* pDestSize, void* pCompressed, unsigned long nSrcSize, int nLevel = -1) = 0;

	// Uncompresses raw (without wrapping) data that is compressed with method 8 (deflated) in the Zip file
	// returns one of the Z_* errors (Z_OK upon success)
	// This function just mimics the standard uncompress (with modification taken from unzReadCurrentFile)
	// with 2 differences: there are no 16-bit checks, and 
	// it initializes the inflation to start without waiting for compression method byte, as this is the 
	// way it's stored into zip file
	virtual int RawUncompress (void* pUncompressed, unsigned long* pDestSize, const void* pCompressed, unsigned long nSrcSize) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Files collector.
	//////////////////////////////////////////////////////////////////////////
	typedef void (*RecordedFilesEnumCallback)( const char *filename );
	//! Turn on/off recording of filenames of opened files.
	virtual void RecordFileOpen( bool bEnable ) = 0;
	//! Record this file if recording is enabled.
	virtual void RecordFile( const char *szFilename ) = 0;
	//! Get filenames of all recorded files.
	virtual void EnumerateRecordedFiles( RecordedFilesEnumCallback enumCallback ) = 0;
};

#endif
