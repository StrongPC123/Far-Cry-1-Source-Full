#include "stdafx.h"
#include "MTSafeAllocator.h"
#include "TArrays.h"
#include "zlib/zlib.h"
#include "smartptr.h"
#include "ZipFileFormat.h"
#include "ZipDirStructures.h"
#include "ZipDirTree.h"
#include "ZipDirCache.h"
#include "ZipDirCacheRW.h"
#include "ZipDirCacheFactory.h"
#include "ZipDirList.h"

using namespace ZipFile;

ZipDir::CacheFactory::CacheFactory (CMTSafeHeap* pHeap, InitMethodEnum nInitMethod, unsigned nFlags):
	m_pHeap(pHeap)
{
	m_nCDREndPos = 0;
	m_f = NULL;
	m_bBuildFileEntryMap = false; // we only need it for validation/debugging
	m_bBuildFileEntryTree = true; // we need it to actually build the optimized structure of directories

	m_nInitMethod = nInitMethod;
	m_nFlags = nFlags;
}

ZipDir::CacheFactory::~CacheFactory()
{
	Clear();
}

ZipDir::CachePtr ZipDir::CacheFactory::New (const char* szFile)
{  
	Clear();
	m_f = fopen (szFile, "rb");
	if (!m_f)
		THROW_ZIPDIR_ERROR(ZD_ERROR_IO_FAILED,"Cannot open file in binary mode for reading, probably missing file");
	try
	{
		return MakeCache (szFile);
	}
	catch(Error)
	{
		Clear();
		throw;
	}
}


ZipDir::CacheRWPtr ZipDir::CacheFactory::NewRW(const char* szFileName)
{
	CacheRWPtr pCache = new CacheRW;
	// opens the given zip file and connects to it. Creates a new file if no such file exists
	// if successful, returns true.
	pCache->m_pHeap = m_pHeap;
	if (!(m_nFlags&FLAGS_DONT_MEMORIZE_ZIP_PATH))
		pCache->m_strFilePath = szFileName;

	if (m_nFlags & FLAGS_DONT_COMPACT)
		pCache->m_nFlags |= CacheRW::FLAGS_DONT_COMPACT;

	// first, try to open the file for reading or reading/writing
	if (m_nFlags & FLAGS_READ_ONLY)
	{
		m_f = fopen (szFileName, "rb");
		pCache->m_nFlags |= CacheRW::FLAGS_CDR_DIRTY|CacheRW::FLAGS_READ_ONLY;

		if (!m_f)
			THROW_ZIPDIR_ERROR(ZD_ERROR_IO_FAILED, "Could not open file in binary mode for reading");
	}
	else
	{
		m_f = NULL;
		if (!(m_nFlags & FLAGS_CREATE_NEW))
			m_f = fopen (szFileName, "r+b");

		if (m_f)
			Read (*pCache);
		else
		if ((m_f = fopen (szFileName, "w+b"))!= NULL)
		{
			// there's no such file, but we'll create one. We'll need to write out the CDR here
			pCache->m_lCDROffset = 0;
			pCache->m_nFlags |= CacheRW::FLAGS_CDR_DIRTY;
		}

		if (!m_f)
			THROW_ZIPDIR_ERROR(ZD_ERROR_IO_FAILED, "Could not open file in binary mode for appending (read/write)");
	}


	// give the cache the file handle:
	pCache->m_pFile = m_f;
	// the factory doesn't own it after that
	m_f = NULL;

	return pCache;
}

void ZipDir::CacheFactory::Read (CacheRW& rwCache)
{
	m_bBuildFileEntryTree = true;
	Prepare();

	// since it's open for R/W, we need to know exactly how much space
	// we have for each file to use the gaps efficiently
	FileEntryList Adjuster (&m_treeFileEntries, m_CDREnd.lCDROffset);
	Adjuster.RefreshEOFOffsets();

	m_treeFileEntries.Swap(rwCache.m_treeDir);
	// very important: we need this offset to be able to add to the zip file
	rwCache.m_lCDROffset = m_CDREnd.lCDROffset;
}

// reads everything and prepares the maps
void ZipDir::CacheFactory::Prepare ()
{
	FindCDREnd();

	// we don't support multivolume archives
	if (m_CDREnd.nDisk != 0 
		|| m_CDREnd.nCDRStartDisk != 0
		|| m_CDREnd.numEntriesOnDisk != m_CDREnd.numEntriesTotal)
		THROW_ZIPDIR_ERROR(ZD_ERROR_UNSUPPORTED, "Multivolume archive detected. Current version of ZipDir does not support multivolume archives");

	// if the central directory offset or size are out of range, 
	// the CDREnd record is probably corrupt
	if (m_CDREnd.lCDROffset > m_nCDREndPos
		|| m_CDREnd.lCDRSize > m_nCDREndPos
		|| m_CDREnd.lCDROffset + m_CDREnd.lCDRSize > m_nCDREndPos)
		THROW_ZIPDIR_ERROR (ZD_ERROR_DATA_IS_CORRUPT, "The central directory offset or size are out of range, the pak is probably corrupt, try to repare or delete the file");

	BuildFileEntryMap();

	// the number of parsed files MUST be the declared number of entries
	// in the central directory
	if (m_bBuildFileEntryMap && m_CDREnd.numEntriesTotal != m_mapFileEntries.size())
		THROW_ZIPDIR_ERROR (ZD_ERROR_CDR_IS_CORRUPT, "The number of parsed files does not match the declared number of entries in the central directory, the pak is probably corrupt, try to repare or delete the file");

	unsigned numFilesFound = m_treeFileEntries.NumFilesTotal();
	if (m_bBuildFileEntryTree && m_CDREnd.numEntriesTotal != numFilesFound)
		THROW_ZIPDIR_ERROR (ZD_ERROR_CDR_IS_CORRUPT, "The number of parsed files does not match the declared number of entries in the central directory. The pak does not appear to be corrupt, but perhaps there are some duplicated or missing file entries, try to repare the file");
}

ZipDir::CachePtr ZipDir::CacheFactory::MakeCache (const char* szFile)
{

	Prepare();

// initializes this object from the given tree, which is a convenient representation of the file tree
	size_t nSizeRequired = m_treeFileEntries.GetSizeSerialized();
	size_t nSizeZipPath = 1; // we need to remember the terminating 0
	if (!(m_nFlags & FLAGS_DONT_MEMORIZE_ZIP_PATH))
		nSizeZipPath += strlen(szFile);
	// allocate and initialize the memory that'll be the root now
	size_t nCacheInstanceSize = sizeof(Cache) + nSizeRequired + nSizeZipPath;

	Cache* pCacheInstance = (Cache*)m_pHeap->Alloc(nCacheInstanceSize, "ZipDir::CacheFactory::New");
	pCacheInstance->Construct(m_f, m_pHeap, nSizeRequired);
	CachePtr cache = pCacheInstance;
	m_f = NULL; // we don't own the file anymore - it's in possession of the cache instance

	// try to serialize into the memory
	size_t nSizeSerialized = m_treeFileEntries.Serialize (cache->GetRoot());

	assert (nSizeSerialized == nSizeRequired);

	char* pZipPath = ((char*)(pCacheInstance + 1)) + nSizeRequired;

	if (!(m_nFlags & FLAGS_DONT_MEMORIZE_ZIP_PATH))
		memcpy (pZipPath, szFile, nSizeZipPath);
	else
		pZipPath[0] = '\0';

	Clear();

	return cache;
}

void ZipDir::CacheFactory::Clear()
{
	if (m_f)
		fclose (m_f);
	m_nCDREndPos = 0;
	memset (&m_CDREnd, 0, sizeof(m_CDREnd));
	m_mapFileEntries.clear();
	m_treeFileEntries.Clear();
}


//////////////////////////////////////////////////////////////////////////
// searches for CDREnd record in the given file
void ZipDir::CacheFactory::FindCDREnd()
{
	// this buffer will be used to find the CDR End record
	// the additional bytes are required to store the potential tail of the CDREnd structure
	// when moving the window to the next position in the file
	char pReservedBuffer[g_nCDRSearchWindowSize + sizeof(CDREnd)-1];

	Seek (0,SEEK_END);
	long nFileSize = Tell();

	if (nFileSize < sizeof(ZipFile::CDREnd))
		THROW_ZIPDIR_ERROR (ZD_ERROR_NO_CDR, "The file is too small, it doesn't even contain the CDREnd structure. Please check and delete the file. Truncated files are not deleted automatically");

	// this will point to the place where the buffer was loaded
	int nOldBufPos = nFileSize;
	// start scanning well before the end of the file to avoid reading beyond the end

	int nScanPos = nFileSize - sizeof(CDREnd);

	m_CDREnd.lSignature = 0; // invalid signature as the flag of not-found CDR End structure
	while (true)
	{
		int nNewBufPos; // the new buf pos
		char* pWindow = pReservedBuffer; // the window pointer into which data will be read (takes into account the possible tail-of-CDREnd)
		if (nOldBufPos <= g_nCDRSearchWindowSize)
		{
			// the old buffer position doesn't let us read the full search window size
			// therefore the new buffer pos will be 0 (instead of negative beyond the start of the file)
			// and the window pointer will be closer tot he end of the buffer because the end of the buffer
			// contains the data from the previous iteration (possibly)
			nNewBufPos = 0;
			pWindow = pReservedBuffer + g_nCDRSearchWindowSize - (nOldBufPos - nNewBufPos);
		}
		else
		{
			nNewBufPos = nOldBufPos - g_nCDRSearchWindowSize;
			assert (nNewBufPos > 0);
		}

		// if the new buffer pos is beyond 64k limit for the comment size
		if (nNewBufPos < int(nFileSize - sizeof(CDREnd) - 0xFFFF))
			nNewBufPos = nFileSize - sizeof(CDREnd) - 0xFFFF;

		// if there's nothing to search
		if (nNewBufPos >= nOldBufPos)
			THROW_ZIPDIR_ERROR (ZD_ERROR_NO_CDR, "Cannot find Central Directory Record in pak. This is either not a pak file, or a pak file without Central Directory. It does not mean that the data is permanently lost, but it may be severely damaged. Please repair the file with external tools, there may be enough information left to recover the file completely"); // we didn't find anything

		// seek to the start of the new window and read it
		Seek (nNewBufPos);
		Read (pWindow,nOldBufPos - nNewBufPos);

		while (nScanPos >= nNewBufPos)
		{
			const CDREnd* pEnd = (const CDREnd*)(pWindow + nScanPos - nNewBufPos);
			if (pEnd->lSignature == pEnd->SIGNATURE)
			{
				if (pEnd->nCommentLength == nFileSize - nScanPos - sizeof(CDREnd))
				{
					// the comment length is exactly what we expected
					m_CDREnd = *pEnd;
					m_nCDREndPos = nScanPos;
					break;
				}
				else
					THROW_ZIPDIR_ERROR (ZD_ERROR_DATA_IS_CORRUPT, "Central Directory Record is followed by a comment of inconsistent length. This might be a minor misconsistency, please try to repair the file. However, it is dangerous to open the file because I will have to guess some structure offsets, which can lead to permanent unrecoverable damage of the archive content");
			}

			--nScanPos;
		}

		if (m_CDREnd.lSignature == m_CDREnd.SIGNATURE)
			return; // we've found it

		nOldBufPos = nNewBufPos;
		memmove (pReservedBuffer + g_nCDRSearchWindowSize, pWindow, sizeof(CDREnd)-1);
	}
	THROW_ZIPDIR_ERROR (ZD_ERROR_UNEXPECTED, "The program flow may not have possibly lead here. This error is unexplainable"); // we shouldn't be here
}


//////////////////////////////////////////////////////////////////////////
// uses the found CDREnd to scan the CDR and probably the Zip file itself
// builds up the m_mapFileEntries
void ZipDir::CacheFactory::BuildFileEntryMap()
{
	Seek (m_CDREnd.lCDROffset);

  if (m_CDREnd.lCDRSize == 0)
    return;

  TElementaryArray<char> pBuffer;
	pBuffer.reinit (m_CDREnd.lCDRSize);
	if (pBuffer.empty()) // couldn't allocate enough memory for temporary copy of CDR
		THROW_ZIPDIR_ERROR (ZD_ERROR_NO_MEMORY, "Not enough memory to cache Central Directory record for fast initialization. This error may not happen on non-console systems");

  Read (&pBuffer[0], m_CDREnd.lCDRSize);

	// now we've read the complete CDR - parse it.
	const CDRFileHeader* pFile = (const CDRFileHeader*)(&pBuffer[0]);
	const char* pEndOfData = &pBuffer[0]+m_CDREnd.lCDRSize, *pFileName;

	while ((pFileName = (const char*)(pFile+1)) <= pEndOfData
		&& pFile->lSignature == pFile->SIGNATURE)
	{
		// the end of this file record
		const char* pEndOfRecord = (pFileName + pFile->nFileNameLength + pFile->nExtraFieldLength + pFile->nFileCommentLength);
		// if the record overlaps with the End Of CDR structure, something is wrong
		if (pEndOfRecord > pEndOfData)
			THROW_ZIPDIR_ERROR (ZD_ERROR_CDR_IS_CORRUPT, "Central Directory record is either corrupt, or truncated, or missing. Cannot read the archive directory");

		// put this entry into the map
		AddFileEntry (GetFilePath(pFile), pFile);

		// move to the next file
		pFile = (const CDRFileHeader*)pEndOfRecord;
	}

	// finished reading CDR
}


//////////////////////////////////////////////////////////////////////////
// give the CDR File Header entry, reads the local file header to validate
// and determine where the actual file lies
void ZipDir::CacheFactory::AddFileEntry (const string& strFilePath, const ZipFile::CDRFileHeader* pFileHeader)
{
	if (pFileHeader->lLocalHeaderOffset > m_CDREnd.lCDROffset)
		THROW_ZIPDIR_ERROR (ZD_ERROR_CDR_IS_CORRUPT, "Central Directory contains file descriptors pointing outside the archive file boundaries. The archive file is either truncated or damaged. Please try to repair the file"); // the file offset is beyond the CDR: impossible

	if (pFileHeader->nMethod == METHOD_STORE && pFileHeader->desc.lSizeUncompressed != pFileHeader->desc.lSizeCompressed)
		THROW_ZIPDIR_ERROR (ZD_ERROR_VALIDATION_FAILED, "File with STORE compression method declares its compressed size not matching its uncompressed size. File descriptor is inconsistent, archive content may be damaged, please try to repair the archive");

	FileEntry fileEntry (*pFileHeader);

	if (m_nInitMethod >= ZD_INIT_FULL && pFileHeader->desc.lSizeUncompressed)
		InitDataOffset(fileEntry, pFileHeader);

	if (m_bBuildFileEntryMap)
		m_mapFileEntries.insert (FileEntryMap::value_type(strFilePath, fileEntry));

	if (m_bBuildFileEntryTree)
		m_treeFileEntries.Add(strFilePath.c_str(), fileEntry);
}


//////////////////////////////////////////////////////////////////////////
// initializes the actual data offset in the file in the fileEntry structure
// searches to the local file header, reads it and calculates the actual offset in the file
void ZipDir::CacheFactory::InitDataOffset (FileEntry& fileEntry, const ZipFile::CDRFileHeader* pFileHeader)
{
	// make sure it's the same file and the fileEntry structure is properly initialized
	assert (fileEntry.nFileHeaderOffset == pFileHeader->lLocalHeaderOffset);

	/*
	// without validation, it would be like this:
	ErrorEnum nError = Refresh(&fileEntry);
	if (nError != ZD_ERROR_SUCCESS)
		THROW_ZIPDIR_ERROR(nError,"Cannot refresh file entry. Probably corrupted file header inside zip file");
	*/
	Seek(pFileHeader->lLocalHeaderOffset);

	// read the local file header and the name (for validation) into the buffer
	TElementaryArray<char>pBuffer;
	unsigned nBufferLength = sizeof(LocalFileHeader) + pFileHeader->nFileNameLength;
	pBuffer.reinit (nBufferLength);
	Read (&pBuffer[0], nBufferLength);

	// validate the local file header (compare with the CDR file header - they should contain basically the same information)
	const LocalFileHeader* pLocalFileHeader = (const LocalFileHeader*)&pBuffer[0];
	if (pFileHeader->desc != pLocalFileHeader->desc
		|| pFileHeader->nMethod != pLocalFileHeader->nMethod
		|| pFileHeader->nFileNameLength != pLocalFileHeader->nFileNameLength
		// for a tough validation, we can compare the timestamps of the local and central directory entries
		// but we won't do that for backward compatibility with ZipDir
		//|| pFileHeader->nLastModDate != pLocalFileHeader->nLastModDate
		//|| pFileHeader->nLastModTime != pLocalFileHeader->nLastModTime
		)
		THROW_ZIPDIR_ERROR (ZD_ERROR_VALIDATION_FAILED, "The local file header descriptor doesn't match the basic parameters declared in the global file header in the file. The archive content is misconsistent and may be damaged. Please try to repair the archive");

	// now compare the local file name with the one recorded in CDR: they must match.
	if (memicmp ((const char*)&pBuffer[sizeof(LocalFileHeader)], (const char*)pFileHeader+1, pFileHeader->nFileNameLength))
	{
		// either file name, or the extra field do not match
		THROW_ZIPDIR_ERROR(ZD_ERROR_VALIDATION_FAILED, "The local file header contains file name which does not match the file name of the global file header. The archive content is misconsistent with its directory. Please repair the archive");
	}

	fileEntry.nFileDataOffset = pFileHeader->lLocalHeaderOffset + sizeof(LocalFileHeader) + pLocalFileHeader->nFileNameLength + pLocalFileHeader->nExtraFieldLength;

	if (fileEntry.nFileDataOffset >= m_nCDREndPos)
		THROW_ZIPDIR_ERROR(ZD_ERROR_VALIDATION_FAILED, "The global file header declares the file which crosses the boundaries of the archive. The archive is either corrupted or truncated, please try to repair it");

	if (m_nInitMethod >= ZD_INIT_VALIDATE)
		Validate (fileEntry);
}

//////////////////////////////////////////////////////////////////////////
// reads the file pointed by the given header and entry (they must be coherent)
// and decompresses it; then calculates and validates its CRC32
void ZipDir::CacheFactory::Validate(const FileEntry& fileEntry)
{
	TElementaryArray<char> pBuffer;
	// validate the file contents
	// allocate memory for both the compressed data and uncompressed data
	pBuffer.reinit (fileEntry.desc.lSizeCompressed + fileEntry.desc.lSizeUncompressed);
	char* pUncompressed = &pBuffer[fileEntry.desc.lSizeCompressed];
	char* pCompressed = &pBuffer[0];

	assert (fileEntry.nFileDataOffset != FileEntry::INVALID_DATA_OFFSET);
	Seek(fileEntry.nFileDataOffset);

	Read(pCompressed, fileEntry.desc.lSizeCompressed);

	unsigned long nDestSize = fileEntry.desc.lSizeUncompressed;
	int nError = Z_OK;
	if (fileEntry.nMethod)
	{
		nError = ZipRawUncompress (m_pHeap, pUncompressed, &nDestSize, pCompressed, fileEntry.desc.lSizeCompressed);
	}
	else
	{
		assert (fileEntry.desc.lSizeCompressed == fileEntry.desc.lSizeUncompressed);
		memcpy (pUncompressed, pCompressed, fileEntry.desc.lSizeUncompressed);
	}
	switch (nError)
	{
	case Z_OK:
		break;
	case Z_MEM_ERROR:
		THROW_ZIPDIR_ERROR(ZD_ERROR_ZLIB_NO_MEMORY, "ZLib reported out-of-memory error");
	case Z_BUF_ERROR:
		THROW_ZIPDIR_ERROR(ZD_ERROR_ZLIB_CORRUPTED_DATA, "ZLib reported compressed stream buffer error");
	case Z_DATA_ERROR:
		THROW_ZIPDIR_ERROR(ZD_ERROR_ZLIB_CORRUPTED_DATA, "ZLib reported compressed stream data error");
	default:
		THROW_ZIPDIR_ERROR(ZD_ERROR_ZLIB_FAILED, "ZLib reported an unexpected unknown error");
	}

	if (nDestSize != fileEntry.desc.lSizeUncompressed)
		THROW_ZIPDIR_ERROR(ZD_ERROR_CORRUPTED_DATA, "Uncompressed stream doesn't match the size of uncompressed file stored in the archive file headers");

	uLong uCRC32 = crc32(0L, Z_NULL, 0);
	uCRC32 = crc32(uCRC32, (Bytef*)pUncompressed, nDestSize);
	if (uCRC32 != fileEntry.desc.lCRC32)
		THROW_ZIPDIR_ERROR(ZD_ERROR_CRC32_CHECK, "Uncompressed stream CRC32 check failed");
}


//////////////////////////////////////////////////////////////////////////
// extracts the file path from the file header with subsequent information
// may, or may not, put all letters to lower-case (depending on whether the system is to be case-sensitive or not)
// it's the responsibility of the caller to ensure that the file name is in readable valid memory
string ZipDir::CacheFactory::GetFilePath (const char* pFileName, ZipFile::ushort nFileNameLength)
{
	// create lower-case path from the zip entry
	/*
	TElementaryArray<char> pFileNameBuf;
	pFileNameBuf.reinit (nFileNameLength + 1);
	pFileNameBuf[nFileNameLength] = '\0';
	for (unsigned i = 0; i < nFileNameLength; ++i)
	pFileNameBuf[i] = tolower(pFileName[i]);
	return &pFileNameBuf[0];
	*/
	string strResult(pFileName, nFileNameLength);
	tolower(strResult);

	assert (strResult.length() == nFileNameLength);
	assert (strlen(strResult.c_str()) == nFileNameLength);
	return strResult;
}

// seeks in the file relative to the starting position
void ZipDir::CacheFactory::Seek (ZipFile::ulong nPos, int nOrigin) // throw
{
	if (fseek (m_f, nPos, nOrigin))
		THROW_ZIPDIR_ERROR(ZD_ERROR_IO_FAILED, "Cannot fseek() to the new position in the file. This is unexpected error and should not happen under any circumstances. Perhaps some network or disk failure error has caused this");
}

long ZipDir::CacheFactory::Tell () // throw
{
	long nPos = ftell (m_f);
	if (nPos == -1)
		THROW_ZIPDIR_ERROR(ZD_ERROR_IO_FAILED, "Cannot ftell() position in the archive. This is unexpected error and should not happen under any circumstances. Perhaps some network or disk failure error has caused this");
	return nPos;
}

void ZipDir::CacheFactory::Read (void* pDest, unsigned nSize) // throw
{
	if (fread (pDest, nSize, 1, m_f) !=1)
		THROW_ZIPDIR_ERROR(ZD_ERROR_IO_FAILED, "Cannot fread() a portion of data from archive");
}
