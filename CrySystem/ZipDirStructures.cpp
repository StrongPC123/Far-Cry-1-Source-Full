#include "stdafx.h"
#include "MTSafeAllocator.h"
#include "smartptr.h"
#include "zlib/zlib.h"
#include "ZipFileFormat.h"
#include "ZipDirStructures.h"
#include <time.h>
#include <ISystem.h>

using namespace ZipFile;

ZipDir::FileEntry::FileEntry(const CDRFileHeader& header)
{
	this->desc              = header.desc;
	this->nFileHeaderOffset = header.lLocalHeaderOffset;
	this->nFileDataOffset   = INVALID_DATA_OFFSET; // we don't know yet
	this->nMethod           = header.nMethod;
	this->nNameOffset       = 0; // we don't know yet
	this->nLastModTime      = header.nLastModTime;
	this->nLastModDate      = header.nLastModDate;

	// make an estimation (at least this offset should be there), but we don't actually know yet
	this->nEOFOffset        = header.lLocalHeaderOffset + sizeof (ZipFile::LocalFileHeader) + header.nFileNameLength + header.desc.lSizeCompressed;
}



// Uncompresses raw (without wrapping) data that is compressed with method 8 (deflated) in the Zip file
// returns one of the Z_* errors (Z_OK upon success)
// This function just mimics the standard uncompress (with modification taken from unzReadCurrentFile)
// with 2 differences: there are no 16-bit checks, and 
// it initializes the inflation to start without waiting for compression method byte, as this is the 
// way it's stored into zip file
int ZipDir::ZipRawUncompress (CMTSafeHeap*pHeap, void* pUncompressed, unsigned long* pDestSize, const void* pCompressed, unsigned long nSrcSize)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)pCompressed;
	stream.avail_in = (uInt)nSrcSize;

	stream.next_out = (Bytef*)pUncompressed;
	stream.avail_out = (uInt)*pDestSize;

	stream.zalloc = CMTSafeHeap::StaticAlloc;
	stream.zfree = CMTSafeHeap::StaticFree;
	stream.opaque = pHeap;

	err = inflateInit2(&stream, -MAX_WBITS);
	if (err != Z_OK) return err;

	// for some strange reason, passing Z_FINISH doesn't work - 
	// it seems the stream isn't finished for some files and 
	// inflate returns an error due to stream-end-not-reached (though expected) problem
	err = inflate(&stream, Z_SYNC_FLUSH);
	if (err != Z_STREAM_END && err != Z_OK)
	{
		inflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}

	*pDestSize = stream.total_out;

	err = inflateEnd(&stream);
	return err;
}

// compresses the raw data into raw data. The buffer for compressed data itself with the heap passed. Uses method 8 (deflate)
// returns one of the Z_* errors (Z_OK upon success)
int ZipDir::ZipRawCompress (CMTSafeHeap*pHeap, const void* pUncompressed, unsigned long* pDestSize, void* pCompressed, unsigned long nSrcSize, int nLevel)
{
	z_stream stream;
	int err;

	stream.next_in   = (Bytef*)pUncompressed;
	stream.avail_in  = (uInt)nSrcSize;

	stream.next_out  = (Bytef*)pCompressed;
	stream.avail_out = (uInt)*pDestSize;

	stream.zalloc = CMTSafeHeap::StaticAlloc;
	stream.zfree  = CMTSafeHeap::StaticFree;
	stream.opaque = pHeap;

	err = deflateInit2 (&stream, nLevel, Z_DEFLATED, -MAX_WBITS, 9, Z_DEFAULT_STRATEGY);
	if (err != Z_OK) return err;

	err = deflate (&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*pDestSize = stream.total_out;

	err = deflateEnd(&stream);
	return err;
}

// finds the subdirectory entry by the name, using the names from the name pool
// assumes: all directories are sorted in alphabetical order.
// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
ZipDir::DirEntry* ZipDir::DirHeader::FindSubdirEntry(const char* szName)
{
	if (this->numDirs)
	{
		const char* pNamePool = GetNamePool();
		DirEntrySortPred pred(pNamePool);
		DirEntry* pBegin = GetSubdirEntry(0);
		DirEntry* pEnd = pBegin + this->numDirs;
		DirEntry* pEntry = std::lower_bound(pBegin,pEnd, szName, pred);
#if defined(LINUX)
		if (pEntry != pEnd && !strcasecmp(szName, pEntry->GetName(pNamePool)))
#else
		if (pEntry != pEnd && !strcmp(szName, pEntry->GetName(pNamePool)))
#endif
			return pEntry;
	}
	return NULL;
}

// finds the file entry by the name, using the names from the name pool
// assumes: all directories are sorted in alphabetical order.
// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
ZipDir::FileEntry* ZipDir::DirHeader::FindFileEntry(const char* szName)
{
	if (this->numFiles)
	{
		const char* pNamePool = GetNamePool();
		DirEntrySortPred pred(pNamePool);
		FileEntry* pBegin = GetFileEntry(0);
		FileEntry* pEnd = pBegin + this->numFiles;
		FileEntry* pEntry = std::lower_bound(pBegin,pEnd, szName, pred);
#if defined(LINUX)
		if (pEntry != pEnd && !strcasecmp(szName, pEntry->GetName(pNamePool)))
#else
		if (pEntry != pEnd && !strcmp(szName, pEntry->GetName(pNamePool)))
#endif
			return pEntry;
	}
	return NULL;
}


// tries to refresh the file entry from the given file (reads fromthere if needed)
// returns the error code if the operation was impossible to complete
ZipDir::ErrorEnum ZipDir::Refresh (FILE*f, FileEntry* pFileEntry)
{
	if (pFileEntry->nFileDataOffset != pFileEntry->INVALID_DATA_OFFSET)
		return ZD_ERROR_SUCCESS;
		
	if (fseek(f, pFileEntry->nFileHeaderOffset, SEEK_SET))
		return ZD_ERROR_IO_FAILED;

	// read the local file header and the name (for validation) into the buffer
	LocalFileHeader fileHeader;
	if (1 != fread (&fileHeader, sizeof(fileHeader), 1, f))
		return ZD_ERROR_IO_FAILED;

	if (fileHeader.desc != pFileEntry->desc 
		|| fileHeader.nMethod                != pFileEntry->nMethod)
	{				
		/*
		char szErrDesc[1024];
		sprintf(szErrDesc,"ERROR: File header doesn't match previously cached file entry record (%s) \n fileheader desc=(%d,%d,%d), method=%d, \n fileentry desc=(%d,%d,%d),method=%d", \
			f->_tmpfname, \
		fileHeader.desc.lCRC32,fileHeader.desc.lSizeCompressed,fileHeader.desc.lSizeUncompressed, \
		fileHeader.nMethod, \
		pFileEntry->desc.lCRC32,pFileEntry->desc.lSizeCompressed,pFileEntry->desc.lSizeUncompressed, \
		pFileEntry->nMethod);
		*/
		return ZD_ERROR_IO_FAILED;		
		//CryError(szErrDesc);				
		//THROW_ZIPDIR_ERROR(ZD_ERROR_VALIDATION_FAILED,szErrDesc);
	}

	pFileEntry->nFileDataOffset = pFileEntry->nFileHeaderOffset + sizeof(LocalFileHeader) + fileHeader.nFileNameLength + fileHeader.nExtraFieldLength;
	pFileEntry->nEOFOffset      = pFileEntry->nFileDataOffset + pFileEntry->desc.lSizeCompressed;
	return ZD_ERROR_SUCCESS;
}


// writes into the file local header (NOT including the name, only the header structure)
// the file must be opened both for reading and writing
ZipDir::ErrorEnum ZipDir::UpdateLocalHeader (FILE*f, FileEntry* pFileEntry)
{
	if (fseek (f, pFileEntry->nFileHeaderOffset, SEEK_SET))
		return ZD_ERROR_IO_FAILED;

	LocalFileHeader h;
	if (1 != fread(&h, sizeof(h),1, f))
		return ZD_ERROR_IO_FAILED;

	assert (h.lSignature == h.SIGNATURE);

	h.desc.lCRC32 = pFileEntry->desc.lCRC32;
	h.desc.lSizeCompressed = pFileEntry->desc.lSizeCompressed;
	h.desc.lSizeUncompressed = pFileEntry->desc.lSizeUncompressed;
	h.nMethod = pFileEntry->nMethod;
	h.nFlags &= ~GPF_ENCRYPTED; // we don't support encrypted files

	if (fseek (f, pFileEntry->nFileHeaderOffset, SEEK_SET))
		return ZD_ERROR_IO_FAILED;

	if (1 != fwrite (&h, sizeof(h), 1, f))
		return ZD_ERROR_IO_FAILED;

	return ZD_ERROR_SUCCESS;
}


// writes into the file local header - without Extra data
// puts the new offset to the file data to the file entry
// in case of error can put INVALID_DATA_OFFSET into the data offset field of file entry
ZipDir::ErrorEnum ZipDir::WriteLocalHeader (FILE*f, FileEntry* pFileEntry, const char* szRelativePath)
{
	LocalFileHeader h;
	h.lSignature        = h.SIGNATURE;
	h.nVersionNeeded    = 10;
	h.nFlags            = 0;
	h.nMethod           = pFileEntry->nMethod;
	h.nLastModDate      = pFileEntry->nLastModDate;
	h.nLastModTime      = pFileEntry->nLastModTime;
	h.desc              = pFileEntry->desc;
	h.nFileNameLength   = (unsigned short)strlen(szRelativePath);
	h.nExtraFieldLength = 0;

	pFileEntry->nFileDataOffset = pFileEntry->nFileHeaderOffset + sizeof(h) + h.nFileNameLength;
	pFileEntry->nEOFOffset = pFileEntry->nFileDataOffset + pFileEntry->desc.lSizeCompressed;

	if (fseek (f, pFileEntry->nFileHeaderOffset, SEEK_SET))
		return ZD_ERROR_IO_FAILED;

	if (1 != fwrite (&h, sizeof(h),1, f))
		return ZD_ERROR_IO_FAILED;

	if (1 != fwrite (szRelativePath, h.nFileNameLength, 1, f))
		return ZD_ERROR_IO_FAILED;

	return ZD_ERROR_SUCCESS;
}


// conversion routines for the date/time fields used in Zip
ZipFile::ushort ZipDir::DOSDate(tm* t)
{
	return
		 ((t->tm_year - 80) << 9)
		|(t->tm_mon << 5)
		|t->tm_mday;
}

ZipFile::ushort ZipDir::DOSTime(tm* t)
{
	return
		 ((t->tm_hour) << 11)
		|((t->tm_min) << 5)
		|((t->tm_sec) >> 1);
}



// sets the current time to modification time
// calculates CRC32 for the new data
void ZipDir::FileEntry::OnNewFileData(void* pUncompressed, unsigned nSize, unsigned nCompressedSize, unsigned nCompressionMethod)
{
	time_t nTime;
	time(&nTime);
	tm* t = localtime(&nTime);
	this->nLastModTime = DOSTime(t);
	this->nLastModDate = DOSDate(t);
	// we'll need CRC32 of the file to pack it
	this->desc.lCRC32 = crc32(0L, Z_NULL, 0);
	this->desc.lCRC32 = crc32(this->desc.lCRC32, (Bytef*)pUncompressed, nSize);

	this->desc.lSizeCompressed = nCompressedSize;
	this->desc.lSizeUncompressed = nSize;

	this->nMethod = nCompressionMethod;
}


const char* ZipDir::DOSTimeCStr(ZipFile::ushort nTime)
{
	static char szBuf[16];
	sprintf (szBuf, "%02:%02.%02", (nTime >> 11), ((nTime&((1<<11)-1))>>5), ((nTime&((1<<5)-1))<<1));
	return szBuf;
}

const char* ZipDir::DOSDateCStr(ZipFile::ushort nTime)
{
	static char szBuf[32];
	sprintf (szBuf, "%02d.%02d.%04d", (nTime&0x1F), (nTime>>5)&0xF, (nTime >> 9) + 1980);
	return szBuf;
}

FILETIME ZipDir::FileEntry::GetModificationTime()
{
	// TODO/TIME: check and test
	SYSTEMTIME st;
	st.wYear  = (nLastModDate>>9)+1980;
	st.wMonth = ((nLastModDate>>5)&0xF)+1;
	st.wDay   =	(nLastModDate&0x1F);
	st.wHour  = (nLastModTime>>11);
	st.wMinute= (nLastModTime>>5)&0x3F;
	st.wSecond= (nLastModTime<<1)&0x3F;
	st.wMilliseconds = 0;
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	return ft;
}

const char* ZipDir::Error::getError()
{
	switch (this->nError)
	{
#define DECLARE_ERROR(x) case ZD_ERROR_##x: return #x;
	DECLARE_ERROR(SUCCESS);
	DECLARE_ERROR(IO_FAILED);
	DECLARE_ERROR(UNEXPECTED);
	DECLARE_ERROR(UNSUPPORTED);
	DECLARE_ERROR(INVALID_SIGNATURE);
	DECLARE_ERROR(ZIP_FILE_IS_CORRUPT);
	DECLARE_ERROR(DATA_IS_CORRUPT);
	DECLARE_ERROR(NO_CDR);
	DECLARE_ERROR(CDR_IS_CORRUPT);
	DECLARE_ERROR(NO_MEMORY);
	DECLARE_ERROR(VALIDATION_FAILED);
	DECLARE_ERROR(CRC32_CHECK);
	DECLARE_ERROR(ZLIB_FAILED);
	DECLARE_ERROR(ZLIB_CORRUPTED_DATA);
	DECLARE_ERROR(ZLIB_NO_MEMORY);
	DECLARE_ERROR(CORRUPTED_DATA);
	DECLARE_ERROR(INVALID_CALL);
	DECLARE_ERROR(NOT_IMPLEMENTED);
	DECLARE_ERROR(FILE_NOT_FOUND);
	DECLARE_ERROR(DIR_NOT_FOUND);
	DECLARE_ERROR(NAME_TOO_LONG);
	DECLARE_ERROR(INVALID_PATH);
	DECLARE_ERROR(FILE_ALREADY_EXISTS);
#undef DECLARE_ERROR
	default: return "Unknown ZD_ERROR code";
	}
}