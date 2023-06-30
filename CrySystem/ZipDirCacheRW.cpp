#include "stdafx.h"
#include <smartptr.h>
#include "MTSafeAllocator.h"
#include "ZipFileFormat.h"
#include "ZipDirStructures.h"
#include "ZipDirTree.h"
#include "ZipDirList.h"
#include "ZipDirCache.h"
#include "ZipDirCacheRW.h"
#include "ZipDirCacheFactory.h"
#include "ZipDirFindRW.h"


// declaration of Z_OK for ZipRawDecompress
#include "zlib/zlib.h"

using namespace ZipFile;


void ZipDir::CacheRW::Close()
{
	if (m_pFile)
	{
		if (!(m_nFlags & FLAGS_READ_ONLY))
		{
			if ((m_nFlags & FLAGS_UNCOMPACTED) && !(m_nFlags&FLAGS_DONT_COMPACT))
			{
				if (!RelinkZip())
					WriteCDR();
			}
			else
			if (m_nFlags & FLAGS_CDR_DIRTY)
				WriteCDR();
		}

		fclose (m_pFile);  
		m_pFile = NULL;
	}
	m_pHeap = NULL;
	m_treeDir.Clear();
}


// Adds a new file to the zip or update an existing one
// adds a directory (creates several nested directories if needed)
ZipDir::ErrorEnum ZipDir::CacheRW::UpdateFile (const char* szRelativePath, void* pUncompressed, unsigned nSize, unsigned nCompressionMethod, int nCompressionLevel)
{
	SmartPtr pBufferDestroyer(m_pHeap);

	// we'll need the compressed data
	void* pCompressed;
	unsigned long nSizeCompressed;
	int nError;

	switch (nCompressionMethod)
	{
	case METHOD_DEFLATE:
		// allocate memory for compression. Min is nSize * 1.001 + 12
		nSizeCompressed = nSize + (nSize >> 3) + 32;
		pCompressed = m_pHeap->Alloc(nSizeCompressed, "ZipDir::CacheRW::UpdateFile");
		pBufferDestroyer.Attach(pCompressed);
		nError = ZipRawCompress(m_pHeap, pUncompressed, &nSizeCompressed, pCompressed, nSize, nCompressionLevel);
		if (Z_OK != nError)
			return ZD_ERROR_ZLIB_FAILED;
		break;
		
	case METHOD_STORE:
		pCompressed = pUncompressed;
		nSizeCompressed = nSize;
		break;

	default:
		return ZD_ERROR_UNSUPPORTED;
	}

	// create or find the file entry.. this object will rollback (delete the object
	// if the operation fails) if needed.
	FileEntryTransactionAdd pFileEntry(this, szRelativePath);

	if (!pFileEntry)
		return ZD_ERROR_INVALID_PATH;

	pFileEntry->OnNewFileData (pUncompressed, nSize, nSizeCompressed, nCompressionMethod);
	// since we changed the time, we'll have to update CDR
	m_nFlags |= FLAGS_CDR_DIRTY;

	// the new CDR position, if the operation completes successfully
	unsigned lNewCDROffset = m_lCDROffset;

	if (pFileEntry->IsInitialized())
	{
		// this file entry is already allocated in CDR

		// check if the new compressed data fits into the old place
		unsigned nFreeSpace = pFileEntry->nEOFOffset - pFileEntry->nFileHeaderOffset - (unsigned)sizeof(ZipFile::LocalFileHeader) - (unsigned)strlen(szRelativePath);

		if (nFreeSpace != nSizeCompressed)
			m_nFlags |= FLAGS_UNCOMPACTED;

		if (nFreeSpace >= nSizeCompressed)
		{
			// and we can just override the compressed data in the file
			ErrorEnum e = WriteLocalHeader(m_pFile, pFileEntry, szRelativePath);
			if (e != ZD_ERROR_SUCCESS)
				return e;
		}
		else
		{
			// we need to write the file anew - in place of current CDR
			pFileEntry->nFileHeaderOffset = m_lCDROffset;
			ErrorEnum e = WriteLocalHeader(m_pFile, pFileEntry, szRelativePath);
			lNewCDROffset = pFileEntry->nEOFOffset;
			if (e != ZD_ERROR_SUCCESS)
				return e;
		}
	}
	else
	{
		pFileEntry->nFileHeaderOffset = m_lCDROffset;
		ErrorEnum e = WriteLocalHeader(m_pFile, pFileEntry, szRelativePath);
		if (e != ZD_ERROR_SUCCESS)
			return e;

		lNewCDROffset = pFileEntry->nFileDataOffset + nSizeCompressed;

		m_nFlags |= FLAGS_CDR_DIRTY;
	}

	// now we have the fresh local header and data offset
	if (fseek (m_pFile, pFileEntry->nFileDataOffset, SEEK_SET)!=0
		||fwrite (pCompressed, nSizeCompressed, 1, m_pFile) != 1)
	{
		// we need to rollback the transaction: file write failed in the middle, the old
		// data is unrecoverably damaged, and we need to destroy this file entry
		// the CDR is already marked dirty
		return ZD_ERROR_IO_FAILED;
	}

	// since we wrote the file successfully, update the new CDR position
	m_lCDROffset = lNewCDROffset;
	pFileEntry.Commit();

	return ZD_ERROR_SUCCESS;
}


// deletes the file from the archive
ZipDir::ErrorEnum ZipDir::CacheRW::RemoveFile (const char* szRelativePath)
{
	// find the last slash in the path
	const char* pSlash = max(strrchr(szRelativePath, '/'), strrchr(szRelativePath, '\\'));

	const char* pFileName; // the name of the file to delete

	FileEntryTree* pDir; // the dir from which the subdir will be deleted

	if (pSlash)
	{
		FindDirRW fd (GetRoot());
		// the directory to remove
		pDir = fd.FindExact(string (szRelativePath, pSlash-szRelativePath).c_str());
		if (!pDir)
			return ZD_ERROR_DIR_NOT_FOUND;// there is no such directory
		pFileName = pSlash+1;
	}
	else
	{
		pDir = GetRoot();
		pFileName = szRelativePath;
	}

	ErrorEnum e = pDir->RemoveFile (pFileName);
	if (e == ZD_ERROR_SUCCESS)
		m_nFlags |= FLAGS_UNCOMPACTED|FLAGS_CDR_DIRTY;
	return e;
}


// deletes the directory, with all its descendants (files and subdirs)
ZipDir::ErrorEnum ZipDir::CacheRW::RemoveDir (const char* szRelativePath)
{
	// find the last slash in the path
	const char* pSlash = max(strrchr(szRelativePath, '/'), strrchr(szRelativePath, '\\'));
	
	const char* pDirName; // the name of the dir to delete
	
	FileEntryTree* pDir; // the dir from which the subdir will be deleted

	if (pSlash)
	{
		FindDirRW fd (GetRoot());
		// the directory to remove
		pDir = fd.FindExact(string (szRelativePath, pSlash-szRelativePath).c_str());
		if (!pDir)
			return ZD_ERROR_DIR_NOT_FOUND;// there is no such directory
		pDirName = pSlash+1;
	}
	else
	{
		pDir = GetRoot();
		pDirName = szRelativePath;
	}

	ErrorEnum e = pDir->RemoveDir (pDirName);
	if (e == ZD_ERROR_SUCCESS)
		m_nFlags |= FLAGS_UNCOMPACTED|FLAGS_CDR_DIRTY;
	return e;
}

// deletes all files and directories in this archive
ZipDir::ErrorEnum ZipDir::CacheRW::RemoveAll()
{
	ErrorEnum e = m_treeDir.RemoveAll();
	if (e == ZD_ERROR_SUCCESS)
		m_nFlags |= FLAGS_UNCOMPACTED|FLAGS_CDR_DIRTY;
	return e;
}

ZipDir::ErrorEnum ZipDir::CacheRW::ReadFile (FileEntry* pFileEntry, void* pCompressed, void* pUncompressed)
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
			//assert (pFileEntry->nSizeCompressed == pFileEntry->nSizeUncompressed);
			//memcpy (pUncompressed, pBuffer, pFileEntry->nSizeCompressed);
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


//////////////////////////////////////////////////////////////////////////
// finds the file by exact path
ZipDir::FileEntry* ZipDir::CacheRW::FindFile (const char* szPath, bool bFullInfo)
{
	if (!this)
		return NULL;
	ZipDir::FindFileRW fd (GetRoot());
	if (!fd.FindExact(szPath))
	{
		assert (!fd.GetFileEntry());
		return NULL;
	}
	assert (fd.GetFileEntry());
	return fd.GetFileEntry();
}

// returns the size of memory occupied by the instance referred to by this cache
size_t ZipDir::CacheRW::GetSize()const
{
	return sizeof(*this) + m_strFilePath.capacity() + m_treeDir.GetSize() - sizeof(m_treeDir);
}

// refreshes information about the given file entry into this file entry
ZipDir::ErrorEnum ZipDir::CacheRW::Refresh (FileEntry* pFileEntry)
{
	if (!pFileEntry)
		return ZD_ERROR_INVALID_CALL;

	if (pFileEntry->nFileDataOffset != pFileEntry->INVALID_DATA_OFFSET)
		return ZD_ERROR_SUCCESS; // the data offset has been successfully read..

	if (!this)
		return ZD_ERROR_INVALID_CALL; // from which cache is this file entry???

	return ZipDir::Refresh(m_pFile, pFileEntry);
}


// writes the CDR to the disk
bool ZipDir::CacheRW::WriteCDR(FILE* fTarget)
{
	if (!fTarget)
		return false;

	if (fseek(fTarget, m_lCDROffset, SEEK_SET))
		return false;

	FileRecordList arrFiles(GetRoot());
	//arrFiles.SortByFileOffset();
	size_t nSizeCDR = arrFiles.GetStats().nSizeCDR;
	void* pCDR = m_pHeap->Alloc(nSizeCDR, "ZipDir::CacheRW::WriteCDR");
  size_t nSizeCDRSerialized = arrFiles.MakeZipCDR(m_lCDROffset,pCDR);
	assert (nSizeCDRSerialized == nSizeCDR);
	size_t nWriteRes = fwrite (pCDR, nSizeCDR, 1, fTarget);
	m_pHeap->Free(pCDR);
	return nWriteRes == 1;
}

// generates random file name
string ZipDir::CacheRW::GetRandomName(int nAttempt)
{
	if (nAttempt)
	{
		char szBuf[8];
		int i;
		for (i = 0; i < sizeof(szBuf)-1;++i)
		{
			int r = rand()%(10 + 'z' - 'a' + 1);
			szBuf[i] = r > 9 ? (r-10)+'a' : '0' + r;
		}
		szBuf[i] = '\0';
		return szBuf;
	}
	else
		return string();
}

bool ZipDir::CacheRW::RelinkZip()
{
	for (int nAttempt = 0; nAttempt < 32; ++nAttempt)
	{
		string strNewFilePath = m_strFilePath + "$" + GetRandomName(nAttempt);
		if (GetFileAttributes (strNewFilePath.c_str()) != -1)
			continue; //  we don't want to overwrite the old temp files for safety reasons

		FILE* f = fopen (strNewFilePath.c_str(), "wb");
		if (f)
		{
			bool bOk = RelinkZip(f);
			fclose (f); // we don't need the temporary file handle anyway

			if (!bOk)
			{
				// we don't need the temporary file
				unlink (strNewFilePath.c_str());
				return false;
			}

			// we successfully relinked, now copy the temporary file to the original file
			fclose (m_pFile);
			m_pFile = NULL;

			remove (m_strFilePath.c_str());
			if (rename (strNewFilePath.c_str(), m_strFilePath.c_str()) == 0)
			{
				// successfully renamed - reopen
				m_pFile = fopen (m_strFilePath.c_str(), "r+b");
				return m_pFile == NULL;
			}
			else
			{
				// could not rename
				
				//m_pFile = fopen (strNewFilePath.c_str(), "r+b");
				return false;
			}
		}
	}

	// couldn't open temp file
	return false; 
}

bool ZipDir::CacheRW::RelinkZip(FILE* fTmp)
{
	FileRecordList arrFiles(GetRoot());
	arrFiles.SortByFileOffset();
	FileRecordList::ZipStats Stats = arrFiles.GetStats();

	// we back up our file entries, because we'll need to restore them
	// in case the operation fails
	std::vector<FileEntry> arrFileEntryBackup;
	arrFiles.Backup (arrFileEntryBackup);

	// this is the set of files that are to be written out - compressed data and the file record iterator
	std::vector<FileDataRecordPtr> queFiles;
	queFiles.reserve (g_nMaxItemsRelinkBuffer);

	// the total size of data in the queue
	unsigned nQueueSize = 0;

	for (FileRecordList::iterator it = arrFiles.begin(); it != arrFiles.end(); ++it)
	{
		// find the file data offset
		if (ZD_ERROR_SUCCESS != Refresh (it->pFileEntry))
			return false;

		// go to the file data
		if (fseek (m_pFile, it->pFileEntry->nFileDataOffset, SEEK_SET) != 0)
			return false;

		// allocate memory for the file compressed data
		FileDataRecordPtr pFile = FileDataRecord::New (*it, m_pHeap);

		// read the compressed data
		if (it->pFileEntry->desc.lSizeCompressed && fread (pFile->GetData(), it->pFileEntry->desc.lSizeCompressed, 1, m_pFile) != 1)
			return false;

		// put the file into the queue for copying (writing)
		queFiles.push_back(pFile);
		nQueueSize += it->pFileEntry->desc.lSizeCompressed;

		// if the queue is big enough, write it out
		if(nQueueSize > g_nSizeRelinkBuffer || queFiles.size() >= g_nMaxItemsRelinkBuffer)
		{
			nQueueSize = 0;
			if (!WriteZipFiles(queFiles, fTmp))
				return false;
		}
	}

	if (!WriteZipFiles(queFiles, fTmp))
		return false;

	ZipFile::ulong lOldCDROffset = m_lCDROffset;
	// the file data has now been written out. Now write the CDR
	m_lCDROffset = ftell(fTmp);
	if (m_lCDROffset >= 0 && WriteCDR(fTmp) && 0 == fflush (fTmp))
		// the new file positions are already there - just discard the backup and return
		return true;
	// recover from backup
	arrFiles.Restore (arrFileEntryBackup);
	m_lCDROffset = lOldCDROffset;
	return false;
}

// writes out the file data in the queue into the given file. Empties the queue
bool ZipDir::CacheRW::WriteZipFiles(std::vector<FileDataRecordPtr>& queFiles, FILE* fTmp)
{
	for (std::vector<FileDataRecordPtr>::iterator it = queFiles.begin(); it != queFiles.end(); ++it)
	{
		// set the new header offset to the file entry - we won't need it
		(*it)->pFileEntry->nFileHeaderOffset = ftell (fTmp);

		// while writing the local header, the data offset will also be calculated
		if (ZD_ERROR_SUCCESS != WriteLocalHeader(fTmp, (*it)->pFileEntry, (*it)->strPath.c_str()))
			return false;;

		// write the compressed file data
		if ((*it)->pFileEntry->desc.lSizeCompressed && fwrite ((*it)->GetData(), (*it)->pFileEntry->desc.lSizeCompressed, 1, fTmp) != 1)
			return false;

		assert ((*it)->pFileEntry->nEOFOffset == ftell (fTmp));
	}
	queFiles.clear();
	queFiles.reserve (g_nMaxItemsRelinkBuffer);
	return true;
}