/*=============================================================================
  BmpImage.cpp : BMP image file format implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "CImage.h"
#include "BmpImage.h"

#include "SHEndian.h"

//===========================================================================

//-----------------------------------------------------------------------------
// Some platforms require strict-alignment, which means that values of
// primitive types must be accessed at memory locations which are multiples
// of the size of those types.  For instance, a 'long' can only be accessed
// at a memory location which is a multiple of four.  Consequently, the
// following endian-conversion functions first copy the raw data into a
// variable of the proper data type using memcpy() prior to attempting to
// access it as the given type.
//-----------------------------------------------------------------------------

static inline ushort us_endian (const byte* ptr)
{
  short n;
  memcpy(&n, ptr, sizeof(n));
  return convert_endian(n);
}

static inline unsigned long ul_endian (const byte* ptr)
{
  long n;
  memcpy(&n, ptr, sizeof(n));
  return convert_endian(n);
}

static inline long l_endian (const byte* ptr)
{
  long n;
  memcpy(&n, ptr, sizeof(n));
  return convert_endian(n);
}

#define BFTYPE(x)    us_endian((x) +  0)
#define BFSIZE(x)    ul_endian((x) +  2)
#define BFOFFBITS(x) ul_endian((x) + 10)
#define BISIZE(x)    ul_endian((x) + 14)
#define BIWIDTH(x)   l_endian ((x) + 18)
#define BIHEIGHT(x)  l_endian ((x) + 22)
#define BITCOUNT(x)  us_endian((x) + 28)
#define BICOMP(x)    ul_endian((x) + 30)
#define IMAGESIZE(x) ul_endian((x) + 34)
#define BICLRUSED(x) ul_endian((x) + 46)
#define BICLRIMP(x)  ul_endian((x) + 50)
#define BIPALETTE(x) ((x) + 54)

// Type ID
#define BM "BM" // Windows 3.1x, 95, NT, ...
#define BA "BA" // OS/2 Bitmap Array
#define CI "CI" // OS/2 Color Icon
#define CP "CP" // OS/2 Color Pointer
#define IC "IC" // OS/2 Icon
#define PT "PT" // OS/2 Pointer

// Possible values for the header size
#define WinHSize   0x28
#define OS21xHSize 0x0C
#define OS22xHSize 0xF0

// Possible values for the BPP setting
#define Mono          1  // Monochrome bitmap
#define _16Color      4  // 16 color bitmap
#define _256Color     8  // 256 color bitmap
#define HIGHCOLOR    16  // 16bit (high color) bitmap
#define TRUECOLOR24  24  // 24bit (true color) bitmap
#define TRUECOLOR32  32  // 32bit (true color) bitmap

// Compression Types
#ifndef BI_RGB
#define BI_RGB        0  // none
#define BI_RLE8       1  // RLE 8-bit / pixel
#define BI_RLE4       2  // RLE 4-bit / pixel
#define BI_BITFIELDS  3  // Bitfields
#endif

//===========================================================================

void CImageBmpFile::mfLoadWindowsBitmap (byte* iBuffer, long iSize)
{
  mfSet_dimensions (BIWIDTH(iBuffer), BIHEIGHT(iBuffer));
  const int bmp_size = m_Width * m_Height;
  m_eFormat = eIF_Bmp;

  byte *iPtr = iBuffer + BFOFFBITS(iBuffer);

  // The last scanline in BMP corresponds to the top line in the image
  int  buffer_y = m_Width * (m_Height - 1);
  bool blip     = false;

  if (BITCOUNT(iBuffer) == _256Color && BICLRUSED(iBuffer))
  {
    mfSet_ImageSize(m_Width * m_Height);
    byte *buffer = mfGet_image();
    m_pPal = new SRGBPixel [256];
    SRGBPixel *pwork   = m_pPal;
    byte    *inpal   = BIPALETTE(iBuffer);
    mfSet_bps(8);

    for (int color=0; color<256; color++, pwork++)
    {  
      // Whacky BMP palette is in BGR order.
      pwork->blue  = *inpal++;
      pwork->green = *inpal++;
      pwork->red   = *inpal++;
      pwork->alpha = 255;
      inpal++; // Skip unused byte.
    }

    if (BICOMP(iBuffer) == BI_RGB)
    {
      // Read the pixels from "top" to "bottom"
      while (iPtr < iBuffer + iSize && buffer_y >= 0)
      {
        memcpy (buffer + buffer_y, iPtr, m_Width);
        iPtr += m_Width;
        buffer_y -= m_Width;
      } /* endwhile */
    }
    else
    if (BICOMP(iBuffer) == BI_RLE8)
    {
      // Decompress pixel data
      byte rl, rl1, i;			// runlength
      byte clridx, clridx1;		// colorindex
      int buffer_x = 0;
      while (iPtr < iBuffer + iSize && buffer_y >= 0)
      {
        rl = rl1 = *iPtr++;
        clridx = clridx1 = *iPtr++;
        if (rl == 0)
           if (clridx == 0)
        {
    	  // new scanline
          if (!blip)
          {
            // if we didnt already jumped to the new line, do it now
            buffer_x  = 0;
            buffer_y -= m_Width;
          }
          continue;
        }
        else
        if (clridx == 1)
          // end of bitmap
          break;
        else
        if (clridx == 2)
        {
          // next 2 bytes mean column- and scanline- offset
          buffer_x += *iPtr++;
          buffer_y -= (m_Width * (*iPtr++));
          continue;
        }
        else
        if (clridx > 2)
          rl1 = clridx;

        for ( i = 0; i < rl1; i++ )
        {
          if (!rl)
            clridx1 = *iPtr++;
          buffer [buffer_y + buffer_x] = clridx1;

          if (++buffer_x >= m_Width)
          {
            buffer_x  = 0;
            buffer_y -= m_Width;
            blip = true;
          }
          else
            blip = false;
        }
        // pad in case rl == 0 and clridx in [3..255]
        if (rl == 0 && (clridx & 0x01))
          iPtr++;
      }
    }
    return;
  }
  else
  if (!BICLRUSED(iBuffer) && BITCOUNT(iBuffer) == TRUECOLOR24)
  {
    mfSet_ImageSize(m_Width * m_Height * 4);
    mfSet_bps (24);
    SRGBPixel *buffer = (SRGBPixel *)mfGet_image();

    while (iPtr < iBuffer + iSize && buffer_y >= 0)
    {
      SRGBPixel *d = buffer + buffer_y;
      for (int x = m_Width; x; x--)
      {
        d->blue    = *iPtr++;
        d->green   = *iPtr++;
        d->red     = *iPtr++;
        d->alpha = 255;
        d++;
      } /* endfor */

      buffer_y -= m_Width;
    }
    return;
  }
  else
  if (!BICLRUSED(iBuffer) && BITCOUNT(iBuffer) == TRUECOLOR32)
  {
    mfSet_ImageSize(m_Width * m_Height * 4);
    mfSet_bps (32);
    SRGBPixel *buffer = (SRGBPixel *)mfGet_image();

    while (iPtr < iBuffer + iSize && buffer_y >= 0)
    {
      SRGBPixel *d = buffer + buffer_y;
      for (int x = m_Width; x; x--)
      {
        d->blue    = *iPtr++;
        d->green   = *iPtr++;
        d->red     = *iPtr++;
        d->alpha   = *iPtr++;
        d++;
      } /* endfor */

      buffer_y -= m_Width;
    }
    return;
  }

  mfSet_error (eIFE_BadFormat, "Unknown BMP image format");

  return;
}

CImageBmpFile::CImageBmpFile (byte* ptr, long filesize) : CImageFile ()
{
  if ((memcmp (ptr, BM, 2) == 0) && BISIZE(ptr) == WinHSize)
    mfLoadWindowsBitmap (ptr, filesize);
  else
    mfSet_error (eIFE_BadFormat, "Not a Windows BMP file");
  return;
}

CImageBmpFile::~CImageBmpFile ()
{
}
