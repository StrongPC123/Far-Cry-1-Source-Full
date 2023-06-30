//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:ChunkFileReader.h
//  Declaration of class CChunkFileReader
//
//	History:
//	06/26/2002 :Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#ifndef _CHUNK_FILE_READER_HDR_
#define _CHUNK_FILE_READER_HDR_

#include "CryHeaders.h"
#include "smartptr.h"

class CFileMapping;
TYPEDEF_AUTOPTR(CFileMapping);

////////////////////////////////////////////////////////////////////////
// Chunk file reader. 
// Accesses a chunked file structure through file mapping object.
// Opens a chunk file and checks for its validity.
// If it's invalid, closes it as if there was no open operation.
// Error handling is performed through the return value of open: it must
// be true for successfully open files
////////////////////////////////////////////////////////////////////////

class CChunkFileReader:
	public _reference_target_t
{
public:
	typedef FILE_HEADER FileHeader;
	typedef CHUNK_HEADER ChunkHeader;

	CChunkFileReader();
	~CChunkFileReader();

	// attaches the file mapping object to this file reader and checks
	// whether the file is a valid chunked file
	bool open(CFileMapping* pFile);

	// attaches a new file mapping object to this file reader and checks
	// whether the file is a valid chunked file
	bool open(const char* szFileName, unsigned nFlags = 0);

	bool open (const string& strFileName, unsigned nFlags = 0);

	// closes the file mapping object and thus detaches the file from this reader
	void close();

	// returns the raw data of the file from the given offset
  const void* getRawData(unsigned nOffset)const;

	// returns the raw data of the i-th chunk
	const void* getChunkData(int nChunkIdx)const;

	// retrieves the raw chunk header, as it appears in the file
	const ChunkHeader& getChunkHeader(int nChunkIdx)const;

	// calculates the chunk size, based on the very next chunk with greater offset
	// or the end of the raw data portion of the file
	int getChunkSize(int nChunkIdx) const;

	// number of chunks
	int numChunks()const;

	// number of chunks of the specified type
	int numChunksOfType (ChunkTypes nChunkType) const;

	// returns the file headers
	const FileHeader& getFileHeader() const;

	bool isValid () const;

	const char* getLastError()const {return gLastError;}
protected:
	// this variable contains the last error occured in this class
	static const char* gLastError;

	CFileMapping_AutoPtr m_pFile;
	// array of offsets used by the chunks
	typedef std::vector<int> ChunkSizeArray;
	ChunkSizeArray m_arrChunkSize;
	// pointer to the array of chunks in the m_pFile
	const ChunkHeader* m_pChunks;
};

TYPEDEF_AUTOPTR(CChunkFileReader);

// this function eats the given number of elements from the raw data pointer pRawData
// and increments the pRawData to point to the end of data just eaten
template <typename T>
void EatRawData (T*pArray, unsigned nSize, const void*&pRawData)
{
	memcpy (pArray, pRawData, sizeof(T)*nSize);
	pRawData = static_cast<const T*>(pRawData) + nSize;
}

// this function eats the given number of elements from the raw data pointer pRawData
// and increments the pRawData to point to the end of data just eaten
// RETURNS:
//   false if failed to read the data
template <typename T>
bool EatRawData (T*pArray, unsigned nSize, const void*&pRawData, unsigned& nDataSize)
{
	if (sizeof(T)*nSize <= nDataSize)
	{
		memcpy (pArray, pRawData, sizeof(T)*nSize);
		pRawData = static_cast <const T*> (pRawData) + nSize;
		nDataSize -= sizeof(T)*nSize;
		return true;
	}
	else
		return false;
}

template <typename T>
bool EatRawData (T*pArray, unsigned nSize, const void*&pRawData, const void* pRawDataEnd)
{
	if ((const char*)pRawData + sizeof(T)*nSize <= (const char*)pRawDataEnd)
	{
		memcpy (pArray, pRawData, sizeof(T)*nSize);
		pRawData = static_cast <const T*> (pRawData) + nSize;
		return true;
	}
	else
		return false;
}


// this function puts the pointer to the data to the given pointer, and moves
// the raw data pointer further; if fails, nothing happens and false is returned
// PARAMETERS:
//   pArray - reference to the (pointer) variable to which the pointer to the actual data will be stored
//   nSize  - number of elements in the array (depending on this, the raw data pointer will be moved)
//   pRawData - reference to the actual raw data pointer, this will be incremented
//   nDataSize - reference to the data size counter, this will be decremented upon success
// RETURNS:
//   false if failed to read the data
template <typename T>
bool EatRawDataPtr(const T*&pArray, unsigned nSize, const void*&pRawData, unsigned& nDataSize)
{
	if (sizeof(T)*nSize <= nDataSize)
	{
		pArray = (const T*)pRawData;
		pRawData = static_cast <const T*> (pRawData) + nSize;
		nDataSize -= sizeof(T)*nSize;
		return true;
	}
	else
		return false;
}

template <typename T>
bool EatRawDataPtr(const T*&pArray, unsigned nSize, const void*&pRawData, const void* pRawDataEnd)
{
	if ((const char*)pRawData + sizeof(T)*nSize <= (const char*)pRawDataEnd)
	{
		pArray = (const T*)pRawData;
		pRawData = static_cast <const T*> (pRawData) + nSize;
		return true;
	}
	else
		return false;
}

// ... non-const version, this will hardly be ever needed
/*
template <typename T>
bool EatRawDataPtr(T*&pArray, unsigned nSize, void*&pRawData, unsigned& nDataSize)
{
	if (sizeof(T)*nSize <= nDataSize)
	{
		pArray = (const T*)pRawData;
		pRawData = static_cast <const T*> (pRawData) + nSize;
		nDataSize -= sizeof(T)*nSize;
		return true;
	}
	else
		return false;
}
*/

#endif