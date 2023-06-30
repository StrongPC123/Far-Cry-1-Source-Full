#include "stdafx.h"
#include "MTSafeAllocator.h"
#include <TArrays.h>
#include <smartptr.h>
#include "ZipFileFormat.h"
#include "ZipDirStructures.h"
#include "ZipDirTree.h"
#include "ZipDirCache.h"
#include "ZipDirFind.h"
#include "ZipDirCacheFactory.h"
#include "zlib/zlib.h"

using namespace ZipFile;

// initializes the instance structure
void ZipDir::Cache::Construct(FILE* fNew, CMTSafeHeap* pHeap, size_t nDataSizeIn)
{
	ConstructRefCounter();
	m_pFile = fNew;
	m_pHeap = pHeap;
	m_nDataSize = nDataSizeIn;
	m_nZipPathOffset = nDataSizeIn;
}

// self-destruct when ref count drops to 0
void ZipDir::Cache::Delete()
{
	if (m_pFile)
		fclose (m_pFile);
	m_pHeap->Free(this);
}


// initializes this object from the given Zip file: caches the central directory
// returns 0 if successfully parsed, error code if an error has occured
ZipDir::CachePtr ZipDir::NewCache (const char* szFileName, CMTSafeHeap* pHeap, InitMethodEnum nInitMethod)
{
	// try .. catch(Error)
	CacheFactory factory(pHeap, nInitMethod);
	return factory.New(szFileName);
}



// looks for the given file record in the Central Directory. If there's none, returns NULL.
// if there is some, returns the pointer to it.
// the Path must be the relative path to the file inside the Zip
// if the file handle is passed, it will be used to find the file data offset, if one hasn't been initialized yet
ZipDir::FileEntry* ZipDir::Cache::FindFile (const char* szPath, bool bRefresh)
{
	if (!this)
		return NULL;
	ZipDir::FindFile fd (this);
	if (!fd.FindExact(szPath))
	{
		assert (!fd.GetFileEntry());
		return NULL;
	}
	assert (fd.GetFileEntry());
	return fd.GetFileEntry();

}

// loads the given file into the pCompressed buffer (the actual compressed data)
// if the pUncompressed buffer is supplied, uncompresses the data there
// buffers must have enough memory allocated, according to the info in the FileEntry
// NOTE: there's no need to decompress if the method is 0 (store)
// returns 0 if successful or error code if couldn't do something
ZipDir::ErrorEnum ZipDir::Cache::ReadFile (FileEntry* pFileEntry, void* pCompressed, void* pUncompressed)
{
	if (!pFileEntry)
		return ZD_ERROR_INVALID_CALL;

	if (pFileEntry->desc.lSizeUncompressed == 0)
	{
		assert (pFileEntry->desc.lSizeCompressed == 0);
		return ZD_ERROR_SUCCESS;
	}

	assert (pFileEntry->desc.lSizeCompressed > 0);

	ErrorEnum nError = Refresh(pFileEntry);
	if (nError != ZD_ERROR_SUCCESS)
		return nError;

	if (fseek (m_pFile, pFileEntry->nFileDataOffset, SEEK_SET))
		return ZD_ERROR_IO_FAILED;

	SmartPtr pBufferDestroyer(m_pHeap);

	void* pBuffer = pCompressed; // the buffer where the compressed data will go

	if (pFileEntry->nMethod == 0 && pUncompressed)
	{
		// we can directly read into the uncompress buffer
		pBuffer = pUncompressed;
	}

	if (!pBuffer)
	{
		if (!pUncompressed)
		// what's the sense of it - no buffers at all?
			return ZD_ERROR_INVALID_CALL;

		pBuffer = m_pHeap->Alloc(pFileEntry->desc.lSizeCompressed, "ZipDir::Cache::ReadFile");
		pBufferDestroyer.Attach(pBuffer); // we want it auto-freed once we return
	}

	if (fread (pBuffer, pFileEntry->desc.lSizeCompressed, 1, m_pFile) != 1)
		return ZD_ERROR_IO_FAILED;

	// if there's a buffer for uncompressed data, uncompress it to that buffer
	if (pUncompressed)
	{
		if (pFileEntry->nMethod == 0)
		{
			assert (pBuffer == pUncompressed);
			//assert (pFileEntry->desc.lSizeCompressed == pFileEntry->nSizeUncompressed);
			//memcpy (pUncompressed, pBuffer, pFileEntry->desc.lSizeCompressed);
		}
		else
		{
			unsigned long nSizeUncompressed = pFileEntry->desc.lSizeUncompressed;
			if (Z_OK != ZipRawUncompress(m_pHeap, pUncompressed, &nSizeUncompressed, pBuffer, pFileEntry->desc.lSizeCompressed))
				return ZD_ERROR_CORRUPTED_DATA;
		}
	}

	return ZD_ERROR_SUCCESS;
}

// loads and unpacks the file into a newly created buffer (that must be subsequently freed with
// Free()) Returns NULL if failed
void* ZipDir::Cache::AllocAndReadFile (FileEntry* pFileEntry)
{
	if (!pFileEntry)
		return NULL;

	void* pData = m_pHeap->Alloc(pFileEntry->desc.lSizeUncompressed, "ZipDir::Cache::AllocAndReadFile");
	if (pData)
	{
		if (ZD_ERROR_SUCCESS != ReadFile (pFileEntry, NULL, pData))
		{
			m_pHeap->Free (pData);
			pData = NULL;
		}
	}
	return pData;
}

// frees the memory block that was previously allocated by AllocAndReadFile
void ZipDir::Cache::Free (void* pData)
{
	m_pHeap->Free(pData);
}

// refreshes information about the given file entry into this file entry
ZipDir::ErrorEnum ZipDir::Cache::Refresh (FileEntry* pFileEntry)
{
	if (!pFileEntry)
		return ZD_ERROR_INVALID_CALL;

	if (pFileEntry->nFileDataOffset != pFileEntry->INVALID_DATA_OFFSET)
		return ZD_ERROR_SUCCESS; // the data offset has been successfully read..

	if (!this)
		return ZD_ERROR_INVALID_CALL; // from which cache is this file entry???

	return ZipDir::Refresh(m_pFile, pFileEntry);
}

// returns the size of memory occupied by the instance referred to by this cache
// must be exact, because it's used by CacheRW to reallocate this cache
size_t ZipDir::Cache::GetSize()const
{
	if (this)
		return m_nDataSize + sizeof(Cache) + strlen(GetFilePath());
	else
		return NULL;
}


// QUICK check to determine whether the file entry belongs to this object
bool ZipDir::Cache::IsOwnerOf (const FileEntry*pFileEntry)const
{
	if (this)
	{
		// just check whether the pointer is within the memory block of this cache instance
		return ((ULONG_PTR)pFileEntry >= (ULONG_PTR)(GetRoot()+1)
			&&(ULONG_PTR)pFileEntry <= ((ULONG_PTR)GetRoot()) + m_nDataSize - sizeof(FileEntry));
	}
	else
		return false;
}