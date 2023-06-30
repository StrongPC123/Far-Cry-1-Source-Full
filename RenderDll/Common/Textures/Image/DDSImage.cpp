/*=============================================================================
  DDSImage.cpp : DDS image file format implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "CImage.h"
#include "DDSImage.h"

/* needed for DirectX's DDSURFACEDESC2 structure definition */
#if !defined(_XBOX) && !defined(PS2) && !defined(LINUX)
#include <ddraw.h>
#else
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))
#endif

#include "dds.h"
#if defined(LINUX)
	#include "ILog.h"
#endif

//===========================================================================

static int sDDSSize(int sx, int sy, EImFormat eF )
{
  switch (eF)
  {
    case eIF_DXT1:
    case eIF_DXT3:
    case eIF_DXT5:
      {
        int blockSize = (eF == eIF_DXT1) ? 8 : 16;
        return ((sx+3)/4)*((sy+3)/4)*blockSize;
      }
  	  break;
    case eIF_DDS_LUMINANCE:
      return sx * sy;
  	  break;
    case eIF_DDS_RGB8:
    case eIF_DDS_SIGNED_RGB8:
      return sx*sy*3;
  	  break;
    case eIF_DDS_RGBA8:
      return sx*sy*4;
  	  break;
    case eIF_DDS_RGBA4:
      return sx*sy*2;
      break;
    case eIF_DDS_DSDT:
      return sx*sy*3;
  	  break;
    default:
      assert(0);
  }
  return 0;
}

int CImageDDSFile::mfSizeWithMips(int filesize, int sx, int sy, int nMips)
{
  int nSize = 0;
  for (int i=0; i<nMips; i++)
  {
    assert(sx || sy);
    if (!sx)
      sx = 1;
    if (!sy)
      sy = 1;
    nSize += sDDSSize(sx, sy, m_eFormat);
    sx >>= 1;
    sy >>= 1;
  }
  assert((int)(filesize-sizeof(DDS_HEADER)-4) >= nSize);
  return nSize;
}

static FILE *sFILELog;

CImageDDSFile::CImageDDSFile (byte* ptr, long filesize) : CImageFile ()
{
  int sx, sy;
  int numMips;

  DWORD dwMagic;
  DDS_HEADER *ddsh;

  dwMagic = *(DWORD *)ptr;
  ptr += sizeof(DWORD);
  if (dwMagic != MAKEFOURCC('D','D','S',' '))
  {
    mfSet_error (eIFE_BadFormat, "Not a DDS file");
    return;
  }
  ddsh = (DDS_HEADER *)ptr;
  ptr += sizeof(DDS_HEADER);
  if (ddsh->dwSize != sizeof(DDS_HEADER))
  {
    mfSet_error (eIFE_BadFormat, "Unknown DDS file header");
    return;
  }
  sx = ddsh->dwWidth;
  sy = ddsh->dwHeight;
  numMips = ddsh->dwMipMapCount;
  if (numMips == 0)
    numMips = 1;

  if (ddsh->ddspf.dwFourCC == FOURCC_DXT1)
    m_eFormat = eIF_DXT1;
  else
  if (ddsh->ddspf.dwFourCC == FOURCC_DXT3)
    m_eFormat = eIF_DXT3;
  else
  if (ddsh->ddspf.dwFourCC == FOURCC_DXT5)
    m_eFormat = eIF_DXT5;
  else
  if (ddsh->ddspf.dwFlags == DDS_RGBA && ddsh->ddspf.dwRGBBitCount == 32 && ddsh->ddspf.dwABitMask == 0xff000000)
    m_eFormat = eIF_DDS_RGBA8;
  else
  if (ddsh->ddspf.dwFlags == DDS_RGBA && ddsh->ddspf.dwRGBBitCount == 16)
    m_eFormat = eIF_DDS_RGBA4;
  else
  if (ddsh->ddspf.dwFlags == DDS_RGB  && ddsh->ddspf.dwRGBBitCount == 24)
    m_eFormat = eIF_DDS_RGB8;
  else
  if (ddsh->ddspf.dwFlags == DDS_RGB  && ddsh->ddspf.dwRGBBitCount == 32)
    m_eFormat = eIF_DDS_RGBA8;
  else
  if (ddsh->ddspf.dwFlags == DDS_LUMINANCE  && ddsh->ddspf.dwRGBBitCount == 8)
    m_eFormat = eIF_DDS_LUMINANCE;
  else
  {
    mfSet_error (eIFE_BadFormat, "Unknown DDS image format");
    return;
  }
  mfSet_numMips(numMips);
  const char *ext = GetExtension(m_CurFileName);
  if ((ddsh->dwReserved1[0] & DDS_RESF1_NORMALMAP) ||
       !stricmp(ext, ".ddn") || !stricmp(ext, ".ddp") ||
       (strlen(m_CurFileName)>4 && (strstr(m_CurFileName, "_ddn") || strstr(m_CurFileName, "_ddp"))))
    mfSet_Flags(FIM_NORMALMAP);
  else
  if ((ddsh->dwReserved1[0] & DDS_RESF1_DSDT) ||
       !stricmp(ext, ".ddt") ||
       (strlen(m_CurFileName)>4 && strstr(m_CurFileName, "_ddt")))
  {
    mfSet_Flags(FIM_DSDT);
    m_eFormat = eIF_DDS_DSDT;
  }
  int nDepth = ddsh->dwDepth;
  if (nDepth <= 0)
    nDepth = 1;
  m_Width = sx;
  m_Height = sy;
  m_Depth = nDepth;

  SAFE_DELETE_ARRAY(m_pByteImage);
  int size = filesize - sizeof(DDS_HEADER) - 4;
  if (m_eFormat == eIF_DDS_DSDT || m_eFormat == eIF_DDS_RGB8)
  {
    size = mfSizeWithMips(filesize, sx, sy, numMips);
    size = size/3*4*nDepth;
  }
  mfSet_ImageSize(size);
  mfGet_image();

  int nOffsSrc = 0;
  int nOffsDst = 0;

  for (int dpt=0; dpt<nDepth; dpt++)
  {
    if (m_eFormat == eIF_DXT1 || m_eFormat == eIF_DXT3 || m_eFormat == eIF_DXT5)
    {
      int size = mfSizeWithMips(filesize, sx, sy, numMips);
      cryMemcpy(&m_pByteImage[nOffsDst], &ptr[nOffsSrc], size);
      nOffsSrc += size;
      nOffsDst += size;
    }
    else
    if (m_eFormat == eIF_DDS_LUMINANCE)
    {
      int size = mfSizeWithMips(filesize, sx, sy, numMips);
      cryMemcpy(&m_pByteImage[nOffsDst], &ptr[nOffsSrc], size);
      nOffsSrc += size;
      nOffsDst += size;
    }
    else
    if (m_eFormat == eIF_DDS_RGBA8 || m_eFormat == eIF_DDS_RGBA4)
    {
      int size = mfSizeWithMips(filesize, sx, sy, numMips);
      cryMemcpy(&m_pByteImage[nOffsDst], &ptr[nOffsSrc], size);
      nOffsSrc += size;
      nOffsDst += size;
    }
    else
    if (m_eFormat == eIF_DDS_RGB8)
    {
      int size = mfSizeWithMips(filesize, sx, sy, numMips);
      int n = size/3;
      int sizeDst = n * 4;
      for (int i=0; i<n; i++)
      {
        m_pByteImage[i*4+nOffsDst+0] = ptr[i*3+nOffsSrc+0];
        m_pByteImage[i*4+nOffsDst+1] = ptr[i*3+nOffsSrc+1];
        m_pByteImage[i*4+nOffsDst+2] = ptr[i*3+nOffsSrc+2];
        m_pByteImage[i*4+nOffsDst+3] = 255;
      }
      nOffsSrc += size;
      nOffsDst += sizeDst;
      if (CRenderer::CV_r_logusedtextures == 10 && (m_Flags & FIM_NORMALMAP))
      {
        if (!sFILELog)
          sFILELog = fopen("LogBumpTexturesNoAlpha.txt", "w");
        if (sFILELog)
        {
          fprintf(sFILELog, "%s\n", m_CurFileName);
          fflush(sFILELog);
        }
      }
    }
    else
    if (m_eFormat == eIF_DDS_DSDT)
    {
      int size = mfSizeWithMips(filesize, sx, sy, numMips);
      int n = size/3;
      int sizeDst = n * 4;
      for (int i=0; i<n; i++)
      {
        m_pByteImage[i*4+nOffsDst+0] = ptr[i*3+nOffsSrc+2];
        m_pByteImage[i*4+nOffsDst+1] = ptr[i*3+nOffsSrc+1];
        m_pByteImage[i*4+nOffsDst+2] = ptr[i*3+nOffsSrc+0];
        m_pByteImage[i*4+nOffsDst+3] = 255;
      }
      nOffsSrc += size;
      nOffsDst += sizeDst;
    }
  }
}

CImageDDSFile::~CImageDDSFile ()
{
}

void WriteDDS(byte *dat, int wdt, int hgt, int Size, const char *name, EImFormat eF, int NumMips)
{
  DWORD dwMagic;
  DDS_HEADER ddsh;
  memset(&ddsh, 0, sizeof(ddsh));

  FILE *fp = fxopen(name, "wb");
  if (!fp)
    return;

  dwMagic = MAKEFOURCC('D','D','S',' ');
  fwrite(&dwMagic, 1, sizeof(DWORD), fp);

  ddsh.dwSize = sizeof(DDS_HEADER);
  ddsh.dwWidth = wdt;
  ddsh.dwHeight = hgt;
  ddsh.dwMipMapCount = NumMips;
  if (!NumMips)
    ddsh.dwMipMapCount = 1;
  ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
  ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE | DDS_SURFACE_FLAGS_MIPMAP;
  size_t len = strlen(name);
	if (len > 4)
	{
		if (!stricmp(&name[len-4], ".ddn"))
			ddsh.dwReserved1[0] = DDS_RESF1_NORMALMAP;
		else
		if (!stricmp(&name[len-4], ".ddt"))
			ddsh.dwReserved1[0] = DDS_RESF1_DSDT;
	}

  switch (eF)
  {
    case eIF_DXT1:
      ddsh.ddspf = DDSPF_DXT1;
      break;
    case eIF_DXT3:
      ddsh.ddspf = DDSPF_DXT3;
      break;
    case eIF_DXT5:
      ddsh.ddspf = DDSPF_DXT5;
      break;
    case eIF_DDS_RGB8:
    case eIF_DDS_SIGNED_RGB8:
    case eIF_DDS_DSDT:
      ddsh.ddspf = DDSPF_R8G8B8;
      break;
    case eIF_DDS_RGBA8:
      ddsh.ddspf = DDSPF_A8R8G8B8;
      break;
    default:
      assert(0);
      return;
  }
  fwrite(&ddsh, 1, sizeof(ddsh), fp);

  byte *data = NULL;

  if (eF == eIF_DDS_RGB8 || eF == eIF_DDS_SIGNED_RGB8 || eF == eIF_DDS_DSDT)
  {
    data = new byte[Size];
    int n = Size / 3;
    for (int i=0; i<n; i++)
    {
      data[i*3+0] = dat[i*3+2];
      data[i*3+1] = dat[i*3+1];
      data[i*3+2] = dat[i*3+0];
    }
    fwrite(data, 1, Size, fp);
  }
  else
  if (eF == eIF_DDS_RGBA8)
  {
    data = new byte[Size];
    int n = Size / 4;
    for (int i=0; i<n; i++)
    {
      data[i*4+0] = dat[i*4+2];
      data[i*4+1] = dat[i*4+1];
      data[i*4+2] = dat[i*4+0];
      data[i*4+3] = dat[i*4+3];
    }
  }
  else
    fwrite(dat, 1, Size, fp);

  SAFE_DELETE_ARRAY(data);

  fclose (fp);
}
