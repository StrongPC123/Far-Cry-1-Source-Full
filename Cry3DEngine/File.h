////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   file.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: cecahed file access
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef PS2
#ifndef FILE_H
#define FILE_H

#define MAX_PATH_LENGTH	512

#include <stdio.h>

class CXFile
{
public:

  CXFile(struct ICryPak * pCryPak);
	~CXFile()	{ FClose(); }

  int     FRead(void *pDest,int nSize,int nNumElems);
  static void SafeRead(FILE *f, void *buffer, int count);
  int     FSeek(int nOff,int nFrom);
  int     FLoad(const char *filename);
  void    FClose();

	static	bool	FileExist(const char *filename);
  static	bool	IsFileExist(const char *filename);
	//-1 if file does not exist
	static	int		GetLength(const char *filename);	
  static  int		GetLength(FILE  *f);	
  static  int   LoadInMemory(const char *filename, void **bufferptr);

//	static	bool	IsOutOfDate(const char *pFileName1,const char *pMasterFile);
//  static	int   GetWriteTime(const char *pFileName1);

	//utils
	static	void	RemoveExtension	(char *path);
  static	void  ReplaceExtension(char *path, const char *new_ext);
	static	char	*GetExtension	(const char *filename);
	static	char	*GetString		(FILE *fp,const char *key);
  static  void  GetPath(char *path);

	//Tim code
	static	void	SetLanguage		(const char *command=NULL);
	static	char	*ConvertFilename(const char *filename);
	static void SetIPack(ICryPak * pCryPak) { m_pCryPak = pCryPak; }

private:	

  char    *m_szFileStart,*m_pCurrPos,*m_pEndOfFile;
  int     m_nFileSize;    
  char m_sLoadedFileName[512];

  static struct ICryPak * m_pCryPak;
};

#endif
#else //PS2
#include "..\CryCommon\File.h"
#endif