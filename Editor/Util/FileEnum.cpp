// FileEnum.cpp: Implementation of the class CFileEnum.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileEnum.h"

#include <io.h>
#include <ISound.h>

//////////////////////////////////////////////////////////////////////
// Construcktion / destrucktion
//////////////////////////////////////////////////////////////////////

CFileEnum::CFileEnum()
{
	m_hEnumFile = 0;
}

CFileEnum::~CFileEnum()
{
	// End the enumeration
	if (m_hEnumFile)
	{
		_findclose(m_hEnumFile);
		m_hEnumFile = 0;
	}
}

bool CFileEnum::StartEnumeration( const char* szEnumPath, char szEnumPattern[], __finddata64_t *pFile)
{
	//////////////////////////////////////////////////////////////////////
	// Take path and search pattern as separate arguments
	//////////////////////////////////////////////////////////////////////

	char szPath[_MAX_PATH];

	// Build enumeration path
	strcpy(szPath, szEnumPath);
	if (szPath[strlen(szPath)] != '\\' &&
		szPath[strlen(szPath)] != '/')
	{
		strcat(szPath, "\\");
	}
	strcat(szPath, szEnumPattern);
	
	return StartEnumeration(szPath, pFile);
}

bool CFileEnum::StartEnumeration( const char *szEnumPathAndPattern, __finddata64_t *pFile)
{
	//////////////////////////////////////////////////////////////////////
	// Start a new file enumeration
	//////////////////////////////////////////////////////////////////////

	// End any previous enumeration
	if (m_hEnumFile)
	{
		_findclose(m_hEnumFile);
		m_hEnumFile = 0;
	}

	// Start the enumeration
	if ((m_hEnumFile = _findfirst64(szEnumPathAndPattern, pFile)) == -1L)
	{
		// No files found
		_findclose(m_hEnumFile);
		m_hEnumFile = 0;
		return false;
	}

	return true;
}

bool CFileEnum::GetNextFile(__finddata64_t *pFile)
{
	//////////////////////////////////////////////////////////////////////
	// Start a new file enumeration
	//////////////////////////////////////////////////////////////////////

	// Fill file strcuture
	if (_findnext64(m_hEnumFile, pFile) == -1)
	{
		// No more files left
		_findclose(m_hEnumFile);
		m_hEnumFile = 0;
		return false;
	};

	// At least one file left
	return true;
}


// Get directory contents.
inline bool ScanDirectoryRecursive( const CString &root,const CString &path,const CString &file,std::vector<CString> &files, bool recursive )
{
	struct __finddata64_t c_file;
	intptr_t hFile;

	bool anyFound = false;

	CString fullPath = root + path + file;
	if( (hFile = _findfirst64( fullPath, &c_file )) != -1L )
	{
		// Find the rest of the files.
		do {
			anyFound = true;
			files.push_back( path + c_file.name );
		}	while (_findnext64( hFile, &c_file ) == 0);
		_findclose( hFile );
	}

	if (recursive)
	{
		fullPath = root + path + "*.*";
		if( (hFile = _findfirst64( fullPath, &c_file )) != -1L )
		{
			// Find directories.
			do {
				if (c_file.attrib & _A_SUBDIR)
				{
					// If recursive.
					if (c_file.name[0] != '.')
					{
						if (ScanDirectoryRecursive( root,path + c_file.name + "\\",file,files,recursive ))
							anyFound = true;
					}
				}
			}	while (_findnext64( hFile, &c_file ) == 0);
			_findclose( hFile );
		}
	}

	return anyFound;
}

bool CFileEnum::ScanDirectory( const CString &path,const CString &file,std::vector<CString> &files, bool recursive )
{
	return ScanDirectoryRecursive(path,"",file,files,recursive );
}