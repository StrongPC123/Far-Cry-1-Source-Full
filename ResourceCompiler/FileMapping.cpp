#include "StdAfx.h"
#include "FileMapping.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// Initializes an empty file mapping object
CFileMapping::CFileMapping():
	m_nSize(0)
	,m_pData(0)
#ifdef USE_FILE_MAPPING
	,m_hFile (INVALID_HANDLE_VALUE)
	,m_hMapping (0)	
#endif
{

}

//////////////////////////////////////////////////////////////////////////////
// initializes the object and tries to open the given file mapping
CFileMapping::CFileMapping (const char* szFileName, unsigned nFlags):
	m_nSize(0)
	,m_pData(0)
#ifdef USE_FILE_MAPPING
	,m_hFile (INVALID_HANDLE_VALUE)
	,m_hMapping (0)	
#endif
{
	open (szFileName, nFlags);
}

//////////////////////////////////////////////////////////////////////////////
// closes file mapping
CFileMapping::~CFileMapping()
{
	close();
}

//////////////////////////////////////////////////////////////////////////////
// Retuns the size of the mapped file, or 0 if no file was mapped or the file is empty
unsigned CFileMapping::getSize()const
{
	return m_nSize;
}

//////////////////////////////////////////////////////////////////////////////
// Returns the pointer to the mapped file start in memory, or NULL if the file
// wasn't mapped
CFileMapping::PData CFileMapping::getData() const
{
	return m_pData;
}

//////////////////////////////////////////////////////////////////////////
// Returns the file data at the given offset
CFileMapping::PData CFileMapping::getData(unsigned nOffset) const
{
	if (m_pData)
		return ((char*)m_pData)+nOffset;
	else
		return NULL;
}

#ifndef USE_FILE_MAPPING
// sets the given (already allocated) buffer to this object
// the memory must be allocated with malloc()
void CFileMapping::attach (PData pData, unsigned nSize)
{
	close();
	m_pData = pData;
	m_nSize = nSize;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// initializes the object, opening the given file
// if file open has failed, subsequent getData() and
// getSize() will return zeros
// Returns true if open was successful
bool CFileMapping::open (const char* szFileName, unsigned nFlags)
{
	close();
#ifdef USE_FILE_MAPPING
	m_hFile = CreateFile (szFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	DWORD dwError = 0;
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		m_nSize = GetFileSize(m_hFile, NULL);
		m_hMapping = CreateFileMapping (m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (m_hMapping != NULL)
		{
			m_pData = MapViewOfFile (m_hMapping, FILE_MAP_READ, 0, 0, 0);
		}
	}
	else
	{
		dwError = GetLastError();
	}
#elif defined(_CRY_ANIMATION_BASE_HEADER_)
	ICryPak* pPak = g_GetPak();
	FILE* f = pPak->FOpen (szFileName, "rb", nFlags);
	if (f != NULL)
	{
		if (0 == pPak->FSeek (f, 0, SEEK_END))
		{
			m_nSize = pPak->FTell (f);
			if ((int)m_nSize >= 0)
			{
				if (0 == pPak->FSeek (f, 0, SEEK_SET))
				{
					void* pData = malloc (m_nSize);
					if (pData != NULL && 1 != pPak->FRead (pData, m_nSize, 1, f))
						free (pData);
					else
						m_pData = pData;
				}
			}
		}
		pPak->FClose (f);
	}
#else
	FILE* f = fxopen (szFileName, "rb");
	if (f != NULL)
	{
		if (0 == fseek (f, 0, SEEK_END))
		{
			m_nSize = ftell (f);
			if ((int)m_nSize >= 0)
			{
				if (0 == fseek (f, 0, SEEK_SET))
				{
					void* pData = malloc (m_nSize);
					if (pData != NULL && 1 != fread (pData, m_nSize, 1, f))
						free (pData);
					else
						m_pData = pData;
				}
			}
		}
		fclose (f);
	}
#endif

	if (!m_pData)
	{
		// we couldn't map the file
		close();
		return false;
	}
	else
		return true;
}

//////////////////////////////////////////////////////////////////////////////
// closes file mapping
// NOTE:
// this function can also be used for rollback of unsuccessful file mapping open
// opration and thus must be able to close partially open file mapping object
void CFileMapping::close()
{
#ifdef USE_FILE_MAPPING
	if (m_pData)
	{
		UnmapViewOfFile(m_pData);
		m_pData = NULL;
	}

	if (m_hMapping != NULL)
	{
		CloseHandle (m_hMapping);
		m_hMapping = NULL;
	}

	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle (m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
#else
	if (m_pData)
	{
		if (m_pData)
			free (m_pData);
		m_pData = NULL;
	}
#endif
	m_nSize = 0;
}
