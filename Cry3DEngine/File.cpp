////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   file.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: ceached file access
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#if defined(LINUX)
	#include <sys/io.h>
#else
	#include <io.h>
#endif

#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
#else
#include <xtl.h>
#endif

#include "File.h"
#include "ICryPak.h"

ICryPak * CXFile::m_pCryPak = 0;

//////////////////////////////////////////////////////////////////////
CXFile::CXFile(struct ICryPak * pCryPak)
{
  m_pCryPak = pCryPak;
  m_szFileStart=NULL;
  m_nFileSize=0;
  m_pCurrPos=0;
  m_pEndOfFile=NULL;
  m_sLoadedFileName[0]=0;
}

//////////////////////////////////////////////////////////////////////
int CXFile::FRead(void *pDest,int nSize,int nNumElems)
{
  int nTotSize=nSize*nNumElems;
  char *pTest=m_pCurrPos+nTotSize;
  if (pTest>m_pEndOfFile)
    return (0);

  memcpy(pDest,m_pCurrPos,nTotSize);
  m_pCurrPos+=nTotSize;
  return (nNumElems);
}

//////////////////////////////////////////////////////////////////////
int CXFile::FSeek(int nOff,int nFrom)
{
  if (nFrom==SEEK_SET)
  {
    m_pCurrPos=m_szFileStart+nOff;
    if (m_pCurrPos>m_pEndOfFile)
      return (1);
  }

  return (0);
}

//////////////////////////////////////////////////////////////////////
void CXFile::FClose()
{
  if (m_szFileStart)
  {
    delete [] m_szFileStart;
    m_szFileStart=NULL;
  }

  m_pCurrPos=NULL;
  m_nFileSize=0;
  m_pEndOfFile=NULL;
  m_sLoadedFileName[0]=0;
}

//////////////////////////////////////////////////////////////////////
int CXFile::FLoad(const char * filename)
{
  if(!m_szFileStart || strcmp(m_sLoadedFileName,filename)!=0)
  {
    FClose();
    m_nFileSize=LoadInMemory(filename,(void**)&m_szFileStart);
    strncpy(m_sLoadedFileName,filename,sizeof(m_sLoadedFileName));
  }

  m_pCurrPos=m_szFileStart;
  m_pEndOfFile=m_szFileStart+m_nFileSize;
  return (m_nFileSize);
}

//get filename's extension
//////////////////////////////////////////////////////////////////////
char *CXFile::GetExtension(const char *filename)
{
	char *src = (char *)filename+strlen(filename)-1;
	while (*src)
	{
		if (*src == '.')
		{ 			
			return (++src);
		}
		src--;
	}

	return (NULL);
}

//remove extension from filename
//////////////////////////////////////////////////////////////////////
void CXFile::RemoveExtension(char *path)
{
	char *src = path+strlen(path)-1;
	while (*src)
	{
		if (*src == '.')
		{ 
			*src = 0; // remove extension 
			return;  
		}
		src--;
	}
}

//replace filename extension
//////////////////////////////////////////////////////////////////////
void CXFile::ReplaceExtension(char *path, const char *new_ext)
{
  RemoveExtension(path);
  strcat(path,".");
  strcat(path,new_ext);
}

//check if file exist
//////////////////////////////////////////////////////////////////////
bool CXFile::IsFileExist(const char *filename)
{
  return FileExist(filename);
}

//check if file exist
//////////////////////////////////////////////////////////////////////
bool CXFile::FileExist(const char *filename)
{
  FILE * fp = m_pCryPak->FOpen(filename,"rb");
	if (!fp) 
    return (false);
	m_pCryPak->FClose(fp);
	return (true);		
}

//get length of the file 
//return (-1) if error
//////////////////////////////////////////////////////////////////////
int CXFile::GetLength(const char *filename)
{
	FILE * fp = m_pCryPak->FOpen(filename,"rb");
	if (!fp) 
    return (-1);

	int pos;
	int end;

	pos = m_pCryPak->FTell(fp);
	m_pCryPak->FSeek(fp, 0, SEEK_END);
	end = m_pCryPak->FTell(fp);
	m_pCryPak->FSeek(fp, pos, SEEK_SET);

	m_pCryPak->FClose(fp);
	return (end);	
}

//tell if filename1 is older than masterfile
//////////////////////////////////////////////////////////////////////
/*bool CXFile::IsOutOfDate(const char *pFileName1,const char *pMasterFile)
{
  
	FILE * f = m_pCryPak->FOpen(pMasterFile,"rb");
	if (f)
		m_pCryPak->FClose(f);
	else
		return (false);

	f = m_pCryPak->FOpen(pFileName1,"rb");
	if (f)
		m_pCryPak->FClose(f);
	else
		return (true);	

#ifdef WIN32

	HANDLE status1 = CreateFile(pFileName1,GENERIC_READ,FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	HANDLE status2 = CreateFile(pMasterFile,GENERIC_READ,FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	FILETIME writetime1,writetime2;

	GetFileTime(status1,NULL,NULL,&writetime1);
	GetFileTime(status2,NULL,NULL,&writetime2);

  CloseHandle(status1);
  CloseHandle(status2);

	if (CompareFileTime(&writetime1,&writetime2)==-1)
		return(true);

	return (false);
#else

	return (false);

#endif		

}*/

//////////////////////////////////////////////////////////////////////
/*int CXFile::GetWriteTime(const char *pFileName1)
{  
	FILE * f = m_pCryPak->FOpen(pFileName1,"rb");
	if (f)
		m_pCryPak->FClose(f);
	else
		return (0);

#ifdef WIN32

	HANDLE status1 = CreateFile(pFileName1,GENERIC_READ,FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	FILETIME writetime1;
  memset(&writetime1,0,sizeof(writetime1));

	GetFileTime(status1,NULL,NULL,&writetime1);

  CloseHandle(status1);

	return (writetime1.dwHighDateTime + writetime1.dwLowDateTime);
#else

	return (0);

#endif		

}  */

//////////////////////////////////////////////////////////////////////
int CXFile::GetLength(FILE *f)
{
	int pos;
	int end;

	pos = m_pCryPak->FTell(f);
	m_pCryPak->FSeek(f, 0, SEEK_END);
	end = m_pCryPak->FTell(f);
	m_pCryPak->FSeek(f, pos, SEEK_SET);

	return end;
}

//////////////////////////////////////////////////////////////////////
void CXFile::SafeRead(FILE *f, void *buffer, int count)
{
	(m_pCryPak->FRead(buffer, 1, count, f) !=  (unsigned)count);
}

//////////////////////////////////////////////////////////////////////
int CXFile::LoadInMemory(const char *filename, void **bufferptr)
{  
	FILE *f = m_pCryPak->FOpen(filename,"rb");
  if (!f)
    return (0);
  int length = CXFile::GetLength(f);
	void *buffer = new char[length+1];
		 
	SafeRead(f, buffer, length);
	m_pCryPak->FClose(f);

  char * bbp = (char *)buffer;
  bbp[length] = 0; //null terminated
	*bufferptr = buffer;
	return (length);
}

//////////////////////////////////////////////////////////////////////
void CXFile::GetPath(char *path)
{
	char *src = path+strlen(path)-1;
	while (*src)
	{
		if (*src == '\\')
		{ 
      src++;
			*src = 0; // remove extension 
			return;  
		}
		src--;
	}  
}
