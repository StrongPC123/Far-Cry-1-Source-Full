#include "StdAfx.h"
#include "FileMapping.h"
#include "chunkfilereader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char* CChunkFileReader::gLastError = "";

CChunkFileReader::CChunkFileReader():
	m_pChunks(NULL)
	//,m_arrChunkSize ("CChunkFileReader.ChunkSize")
{
}

CChunkFileReader::~CChunkFileReader()
{
	close();
}

bool CChunkFileReader::open (const string& strFileName, unsigned nFlags)
{
	return open (strFileName.c_str(), nFlags);
}


//////////////////////////////////////////////////////////////////////////
// attaches the file mapping object to this file reader and checks
// whether the file is a valid chunked file
bool CChunkFileReader::open(CFileMapping* pFile)
{
	close();
	m_pFile = pFile;

	bool bSuccess = false;

	if ( (m_pFile != (CFileMapping*)NULL) && (m_pFile->getData() != NULL) )
	{
		if (m_pFile->getSize() >= sizeof(FileHeader))
		{// the file must contain the header
			const FileHeader& fileHeader = getFileHeader();
			if (m_pFile->getSize() >= fileHeader.ChunkTableOffset + sizeof(int)
					&& (int)fileHeader.ChunkTableOffset > (int)sizeof(fileHeader)
				)
			{// there must be room for the chunk table header
				unsigned numChunks = *static_cast<const unsigned*>(m_pFile->getData(fileHeader.ChunkTableOffset));
				unsigned nChunk;
				if (m_pFile->getSize() >= fileHeader.ChunkTableOffset + sizeof(int) + numChunks*sizeof(ChunkHeader)
					&& numChunks <= (pFile->getSize () - fileHeader.ChunkTableOffset - sizeof(int)) / sizeof(ChunkHeader))
				{
					// the file must contain the full chunk table
					m_pChunks = (const ChunkHeader*)m_pFile->getData(fileHeader.ChunkTableOffset + sizeof(int));

					bool bInvalidChunkOffsetsFound = false; // sets to true if there are chunks pointing outside the raw data section of the file

					// step through all the chunks to fetch file offsets
					std::set<int> setOffsets;
					for (nChunk = 0; nChunk < numChunks; ++nChunk)
					{
						int nOffset = m_pChunks[nChunk].FileOffset;

						if (nOffset < sizeof(FileHeader) || nOffset >= fileHeader.ChunkTableOffset)
						{
							gLastError = "CryFile is corrupted: chunk table is corrupted (invalid chunk offsets found)";
							bInvalidChunkOffsetsFound = true;
						}

						setOffsets.insert(nOffset);
					}
					m_arrChunkSize.clear();
					m_arrChunkSize.resize (numChunks);
					for (nChunk = 0; nChunk < numChunks; ++nChunk)
					{
						int nOffset = m_pChunks[nChunk].FileOffset;
						int nSize = 0; // the size for invalid chunks (with invalid offsets)
						if (nOffset >= sizeof(FileHeader) && nOffset < fileHeader.ChunkTableOffset)
						{
							// find the next offset
							std::set<int>::const_iterator it = setOffsets.find (nOffset);
							assert (it != setOffsets.end());
							assert (*it == nOffset);
							++it;
							nSize = (it==setOffsets.end()?fileHeader.ChunkTableOffset:*it) - nOffset;
						}
							
						assert (nSize >= 0);
						m_arrChunkSize[nChunk] = nSize;
					}

					bSuccess = true;
					// don't let the files with invalid chunks..
					//bSuccess = !bInvalidChunkOffsetsFound;
				}
				else
					gLastError = "CryFile is corrupted: chunk table is corrupted or truncated (file too small)";
			}
			else
				gLastError = "CryFile is corrupted: chunk table is trucated (file header is probably corrupted)";
		}
		else
			gLastError = "CryFile is corrupted: header is truncated (file too small)";
	}
	else
		gLastError = "Invalid file mapping passed to CChunkFileReader::open";


	if (!bSuccess)
		close();

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
// attaches a new file mapping object to this file reader and checks
// whether the file is a valid chunked file
bool CChunkFileReader::open(const char* szFileName, unsigned nFlags)
{
	CFileMapping_AutoPtr pFileMapping = new CFileMapping(szFileName, nFlags);
	if (!pFileMapping->getData())
	{
		gLastError = "Cannot open file";
		return false;
	}
	return open (pFileMapping);
}

void CChunkFileReader::close()
{
	m_arrChunkSize.clear();
	m_pFile = NULL;
	m_pChunks = NULL;
}

// returns the raw data of the file from the given offset
const void* CChunkFileReader::getRawData(unsigned nOffset)const
{
	if ((m_pFile != (CFileMapping*)NULL) && m_pFile->getData())
		return ((char*)m_pFile->getData())+nOffset;
	else
		return NULL;
}

// retrieves the raw chunk header, as it appears in the file
const CChunkFileReader::ChunkHeader& CChunkFileReader::getChunkHeader(int nChunkIdx)const
{
	return m_pChunks[nChunkIdx];
}


// returns the raw data of the i-th chunk
const void* CChunkFileReader::getChunkData(int nChunkIdx)const
{
	if (nChunkIdx>= 0 && nChunkIdx < numChunks())
	{
		int nOffset = m_pChunks[nChunkIdx].FileOffset;
		if (nOffset < sizeof(FileHeader) || nOffset >= getFileHeader().ChunkTableOffset)
			return 0;
		else
			return m_pFile->getData(nOffset);
	}
	else
		return 0;
}

// number of chunks
int CChunkFileReader::numChunks()const
{
	return (int)m_arrChunkSize.size();
}

// number of chunks of the specified type
int CChunkFileReader::numChunksOfType (ChunkTypes nChunkType)const
{
	int nResult = 0;
	for (int i = 0; i < numChunks(); ++i)
	{
		if (m_pChunks[i].ChunkType == nChunkType)
			++nResult;
	}
	return nResult;
}


// returns the file headers
const CChunkFileReader::FileHeader& CChunkFileReader::getFileHeader() const
{
	return m_pFile?*((const FileHeader*)(m_pFile->getData())):*(const FileHeader*)NULL;
}

bool CChunkFileReader::isValid () const
{
	return m_pFile != (CFileMapping*)NULL;
}

//////////////////////////////////////////////////////////////////////////
// calculates the chunk size, based on the very next chunk with greater offset
// or the end of the raw data portion of the file
int CChunkFileReader::getChunkSize(int nChunkIdx) const
{
	assert (nChunkIdx >= 0 && nChunkIdx < numChunks());
	return m_arrChunkSize[nChunkIdx];
}

