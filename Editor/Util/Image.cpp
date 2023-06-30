////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Image.cpp
//  Version:     v1.00
//  Created:     26/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Image implementation,
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Image.h"
//#include "libtiff\tiffio.h"

//#pragma comment( lib,"libtiff/tiff.lib" )

//////////////////////////////////////////////////////////////////////////
bool CImage::LoadGrayscale16Tiff( const CString &fileName )
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CImage::SaveGrayscale16Tiff( const CString &fileName )
{
/*
	TIFF * tif;
  int Width, Height,Bpp;
	tdata_t buf;

  tif = TIFFOpen(fileName, "rb");
  if (tif == 0)
  {
    MessageBox( NULL,"Not a Tiff file","Warning",MB_OK|MB_ICONEXCLAMATION );
    return false;
  }

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &Width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &Height);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &Bpp);

	int bytes = Bpp/8;
	int mask = Bpp-1;

  Allocate(Width, Height);
	*/
/*
  buf = _TIFFmalloc(TIFFScanlineSize(tif));
	for (int row = 0; row < Height; row++)
	{
		TIFFReadScanline(tif, buf, row);
		unsigned char *pBuf = (unsigned char*)buf;
		for (int x = 0; x < Width*bytes; x += bytes)
		{
			ValueAt( x,row ) = (*(uint*)buf) & mask;
			pBuf += bytes;
		}
	}
	_TIFFfree(buf);
	*/

  //TIFFClose(tif);
	return true;
}

void CImage::SwapRedAndBlue()
{
	if (!IsValid())
		return;
	// Set the loop pointers
	uint *pPixData = GetData();
	uint *pPixDataEnd = pPixData + GetWidth()*GetHeight();
	// Switch R and B
	while (pPixData != pPixDataEnd)
	{
		// Extract the bits, shift them, put them back and advance to the next pixel
		*pPixData++ = ((* pPixData & 0x00FF0000) >> 16) | 
			(* pPixData & 0x0000FF00) | ((* pPixData & 0x000000FF) << 16);
	}
}