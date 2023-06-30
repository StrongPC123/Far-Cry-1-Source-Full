/*=============================================================================
  CImage.cpp : Common Image class implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"


#include "PcxImage.h"
#include "DDSImage.h"
#include "BmpImage.h"
#include "TgaImage.h"
#include "JpgImage.h"

#ifdef PS2
#include "XtfImage.h"
#endif

//---------------------------------------------------------------------------

EImFileError CImageFile::m_eError = eIFE_OK;
char CImageFile::m_Error_detail[256];
char CImageFile::m_CurFileName[128];

CImageFile::CImageFile ()
{
  m_pByteImage = NULL;
  m_pPal = NULL;
  m_eError = eIFE_OK;
  m_Error_detail[0] = 0;
  m_eFormat = eIF_Unknown;
  m_NumMips = 0;
  m_Flags = 0;
  m_ImgSize = 0;
  m_Depth = 1;
}

CImageFile::~CImageFile ()
{
  SAFE_DELETE_ARRAY(m_pByteImage);
  SAFE_DELETE_ARRAY(m_pPal)
}

void CImageFile::mfSet_dimensions (int w, int h)
{
  m_Width = w;
  m_Height = h;
}

void CImageFile::mfSet_error (EImFileError error, char* detail)
{
  CImageFile::m_eError = error;
  if (detail)
    strcpy (m_Error_detail, detail);
  m_Error_detail[0] = 0;
}

void CImageFile::mfWrite_error (char* extra)
{
  if (m_eError == eIFE_OK)
    return;
  char buf[1000];
  int idx = 0;
  if (extra)
    idx += sprintf (buf+idx, "'%s': ", extra);
  switch (m_eError)
  {
    case eIFE_OK:
      return;

    case eIFE_IOerror:
      idx += sprintf (buf+idx, "IO error");
      break;

    case eIFE_OutOfMemory:
      idx += sprintf (buf+idx, "Out of memory");
      break;

    case eIFE_BadFormat:
      idx += sprintf (buf+idx, "Bad format");
      break;
  }
  if (m_Error_detail[0])
    sprintf (buf+idx, " (%s)!\n", m_Error_detail);
  else
    sprintf (buf+idx, "!\n");
  iConsole->Exit ("%s", buf);
}

float gFOpenTime;
int nRejectFOpen;
int nAcceptFOpen;

CImageFile* CImageFile::mfLoad_file (char* szFileName)
{
  double dTime0 = 0;
  ticks(dTime0);

  FILE* pRawFile = iSystem->GetIPak()->FOpen (szFileName, "rb");

  unticks(dTime0);
  gFOpenTime += (float)(dTime0*1000.0*g_SecondsPerCycle);

  if (!pRawFile)
  {
    nRejectFOpen++;
    return NULL;
  }
  nAcceptFOpen++;

  strcpy(m_CurFileName, szFileName);
  strlwr(m_CurFileName);
  CImageFile* pImageFile = mfLoad_file (pRawFile);
	if (pImageFile)
	{
		strcpy(pImageFile->m_FileName, m_CurFileName);
		iSystem->GetIPak()->FClose (pRawFile);
	}
	else
		iSystem->GetILog()->LogToFile("\002Warning: Cannot load texture %s, pImageFile format is invalid", szFileName);
	return pImageFile;
}

CImageFile* CImageFile::mfLoad_file (FILE* fp)
{
  iSystem->GetIPak()->FSeek (fp, 0, SEEK_END);
  long size = iSystem->GetIPak()->FTell (fp);
  iSystem->GetIPak()->FSeek (fp, 0, SEEK_SET);
  CHK (byte* buf = new byte [size+1]);
  iSystem->GetIPak()->FRead (buf, 1, size + 1, fp);
  CImageFile* file = mfLoad_file (buf, size);
  CHK (delete [] buf);
  return file;
}

CImageFile* CImageFile::mfLoad_file (byte* buf, long size)
{
  CImageFile* file = NULL;
  CImageFile::m_eError = eIFE_OK;

  // Catch NULL pointers (for example, when ZIP file is corrupt)
  assert (buf);

  const char *ext = GetExtension(m_CurFileName);

#ifdef PS2
  // Try XTF first
  if (!strcmp(ext, ".xtf"))
    CHK (file = (CImageFile *)new CImageXtfFile (buf, (int)size));
#endif

  // Try DDS first
  if (!strcmp(ext, ".dds") || !strcmp(ext, ".ddn") || !strcmp(ext, ".ddp") || !strcmp(ext, ".ddt"))
    CHK (file = (CImageFile *)new CImageDDSFile (buf, size));

  // If failed, try BMP
  if (!strcmp(ext, ".bmp"))
    CHK (file = (CImageFile *)new CImageBmpFile (buf, size));

  // If failed, try PCX
  if (!strcmp(ext, ".pcx"))
    CHK (file = (CImageFile *)new CImagePcxFile (buf, size));
#if !defined(NULL_RENDERER)
  // Try JPG next
  if (!strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg"))
    CHK (file = (CImageFile *)new CImageJpgFile (buf, size));
#endif
  // As a last resort, try TGA
  if (!strcmp(ext, ".tga"))
    CHK (file = (CImageFile *)new CImageTgaFile (buf, size));

  if (file && (CImageFile::mfGet_error () != eIFE_OK))
  {
    CHK (delete file);
    file = NULL;
  } /* endif */

  return file;
}

//---------------------------------------------------------------------------

