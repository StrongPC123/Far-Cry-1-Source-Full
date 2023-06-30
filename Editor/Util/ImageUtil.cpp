////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ImageUtil.cpp
//  Version:     v1.00
//  Created:     30/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Image utilities implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageUtil.h"
#include "ImageGif.h"
#include "Image_DXTC.h"
#include "CryFile.h"

#ifndef WIN64
// Required linking with (Intels Jpeg Library) IJL15.LIB
#include "ijl.h"
#endif

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SaveBitmap( const CString &szFileName, CImage &inImage,bool inverseY )
{
	////////////////////////////////////////////////////////////////////////
	// Simple DIB save code
	////////////////////////////////////////////////////////////////////////

	HANDLE hfile;
	DWORD dwBytes;
	unsigned int i;
	DWORD *pLine1 = NULL;
	DWORD *pLine2 = NULL;
	DWORD *pTemp = NULL;
	BITMAPFILEHEADER bitmapfileheader;
	BITMAPINFOHEADER bitmapinfoheader;
	
	CLogFile::FormatLine("Saving data to bitmap... %s", (const char*)szFileName);

	int dwWidth = inImage.GetWidth();
	int dwHeight = inImage.GetHeight();
	DWORD *pData = (DWORD*)inImage.GetData();
	
	uchar *pImage = new uchar[dwWidth*dwHeight*3];

	i = 0;
	for (int y = 0; y < dwHeight; y++)
	{
		int src_y=y;

		if(inverseY)
			src_y=(dwHeight-1)-y;

		for (int x = 0; x < dwWidth; x++)
		{
			DWORD c = pData[x+src_y*dwWidth];
			pImage[i] = GetBValue(c);
			pImage[i+1] = GetGValue(c);
			pImage[i+2] = GetRValue(c);
			i+=3;
		}
	}

	// Fill in bitmap structures
	bitmapfileheader.bfType = 0x4D42;
	bitmapfileheader.bfSize = (dwWidth * dwHeight * 3) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bitmapfileheader.bfReserved1 = 0;
	bitmapfileheader.bfReserved2 = 0;
	bitmapfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + 
		sizeof(BITMAPINFOHEADER) + (0 * sizeof(RGBQUAD));  
	bitmapinfoheader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfoheader.biWidth = dwWidth;
	bitmapinfoheader.biHeight = dwHeight;
	bitmapinfoheader.biPlanes = 1;
	bitmapinfoheader.biBitCount = (WORD) 24;
	bitmapinfoheader.biCompression = BI_RGB;
	bitmapinfoheader.biSizeImage = 0;
	bitmapinfoheader.biXPelsPerMeter = 0;
	bitmapinfoheader.biYPelsPerMeter = 0;
	bitmapinfoheader.biClrUsed = 0;
	bitmapinfoheader.biClrImportant = 0;
	
	// Write bitmap to disk
	hfile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE) 
	{
		delete []pImage;
		return false;
	}

	// Write the headers to the file
	WriteFile(hfile, &bitmapfileheader, sizeof(BITMAPFILEHEADER), &dwBytes, NULL);
	WriteFile(hfile, &bitmapinfoheader, sizeof(BITMAPINFOHEADER), &dwBytes, NULL);
	
	// Write the data
	DWORD written;
	WriteFile(hfile, pImage, (dwWidth * dwHeight * 3), &written, NULL);

	CloseHandle(hfile);

	delete []pImage;
		
	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SaveJPEG( const CString &strFileName,CImage &inImage )
{
#ifdef WIN64
	return false;
#else //WIN64
	////////////////////////////////////////////////////////////////////////
	// Save an array as a JPEG
	////////////////////////////////////////////////////////////////////////

	bool bSuccess = true;
	JPEG_CORE_PROPERTIES Image;
	unsigned char *pImageData = NULL;
	unsigned char *pImageDataStart = NULL;

	CLogFile::FormatLine("Saving data to JPEG... %s", (const char*)strFileName);


	int dwWidth = inImage.GetWidth();
	int dwHeight = inImage.GetHeight();
	DWORD *pData = (DWORD*)inImage.GetData();

	// Convert from RGBA to RGB
	// Allocate memory for the converted image
	pImageData = new unsigned char[dwWidth * dwHeight * 3];

	// Set the loop pointer
	pImageDataStart = pImageData;
	DWORD *pDataEnd = &pData[dwWidth * dwHeight];

	// Convert
	while (pData != pDataEnd)
	{
		// Extract the color channels and copy them into the indivdual 
		// bytes of the converted image
		*pImageData++ = GetRValue(*pData);
		*pImageData++ = GetGValue(*pData);
		*pImageData++ = GetBValue(*pData);
		
		// Advance to the next source pixel
		pData++;
	}
	
	// Restore the pointer
	pImageData = pImageDataStart;
	
	// Init the JPEG structure
	memset(&Image, 0, sizeof(JPEG_CORE_PROPERTIES));
	
	bool failed = false;
	
	// Initialize
	if (ijlInit(&Image) != IJL_OK)
	{
		Warning("Can't initialize Intel(R) JPEG library !");
		return failed;
	}
		
	// Setup DIB
	Image.DIBWidth    = dwWidth;
	Image.DIBHeight   = dwHeight;
	Image.DIBBytes    = pImageData;
	Image.DIBPadBytes = IJL_DIB_PAD_BYTES(Image.DIBWidth, 3);
		
	// Setup JPEG
	Image.JPGFile   = const_cast<char *> ((const char*)strFileName);
	Image.JPGWidth  = dwWidth;
	Image.JPGHeight = dwHeight;
		
	Image.jquality = 100;
		
	Image.DIBColor = IJL_RGB;
		
	// Remove the read-only attribute so the JPEG library can overwrite the file.
	SetFileAttributes(strFileName, FILE_ATTRIBUTE_NORMAL);
		
	// Write the image
	if (ijlWrite( &Image, IJL_JFILE_WRITEWHOLEIMAGE ) != IJL_OK)
	{
		Warning("Can't JPEG write image !");
		bSuccess = false;
	}
		
	if (ijlFree(&Image) != IJL_OK)
	{
		Warning("Can't free Intel(R) JPEG library !");
	}

	// Free the temporary memory
	delete [] pImageData;
	pImageData = 0;

	return bSuccess;

#endif //WIN64
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::LoadJPEG( const CString &strFileName,CImage &outImage )
{
#ifdef WIN64
	return false;
#else //WIN64
	////////////////////////////////////////////////////////////////////////
	// Loads a JPEG file and stores image dimension data in the passed
	// pointers. Memory for the image itself is allocated and passed
	// and the passed pointer is set
	//
	// TODO: Verify that this functions works
	////////////////////////////////////////////////////////////////////////

	JPEG_CORE_PROPERTIES Image;
	ASSERT(!strFileName.IsEmpty());
	BYTE *pImageData = NULL;

	CLogFile::FormatLine("Loading JPEG %s...", (const char*)strFileName );

	ZeroMemory(&Image, sizeof(JPEG_CORE_PROPERTIES));	

	// Initialize the JPEG library
	if (ijlInit(&Image) != IJL_OK)
	{
		Warning("Can't initialize Intel(R) JPEG library !");

		return false;
	}

	// Set the filename
	Image.JPGFile = strFileName;

	// Read the JPEG header
	if (ijlRead(&Image, IJL_JFILE_READPARAMS) != IJL_OK)
	{
		Warning("Error while reading JPEG file (Header) !");
		
		return false;
	}
	if (Image.JPGChannels != 3)
	{
		CLogFile::FormatLine( "JPG File %s is not 3 channels jpeg format.",(const char*)strFileName );
		return false;
	}

	// Allocate memory for the image
	pImageData = new BYTE[Image.JPGWidth * Image.JPGHeight * Image.JPGChannels];
	ASSERT(pImageData);

	// Fill in the DIB header
	Image.DIBBytes    = pImageData;
	Image.DIBColor    = (Image.JPGChannels == 3) ? IJL_RGB : IJL_G;
	Image.DIBWidth    = Image.JPGWidth;
	Image.DIBHeight   = Image.JPGHeight;
  Image.DIBChannels = Image.JPGChannels;

	// Read the JPEG image
	if (ijlRead(&Image, IJL_JFILE_READWHOLEIMAGE) != IJL_OK)
	{
		Warning("Error while reading JPEG file (Whole Image) !");

		ijlFree(&Image);
		delete []pImageData;
		return false;
	}

	// Free the image
	if (ijlFree(&Image) != IJL_OK)
	{
		Warning("Can't free Intel(R) JPEG library !");

		delete []pImageData;
		return false;
	}

	int memSize = Image.JPGWidth*Image.JPGHeight*4;
	outImage.Allocate( Image.JPGWidth,Image.JPGHeight );
	uint *trg = outImage.GetData();
	if (!trg)
	{
		Warning( "CImageUtil::LoadJPEG Memory allocation faild\r\nFailed to allocate %dMb of RAM for Jpg %s",
						memSize/(1024*1024),(const char*)strFileName );
    return false;
	}
	unsigned char *src = pImageData;
	if (src && trg)
	{
		for (int i = 0; i < Image.JPGHeight*Image.JPGWidth; i++)
		{
			*trg++ = RGB(src[0],src[1],src[2]) | 0xFF000000;
			src += 3;
		}
	}
	delete []pImageData;

	return true;

#endif //WIN64
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SavePGM( const CString &fileName, uint dwWidth, uint dwHeight, uint *pData)
{
	FILE *file = fopen( fileName,"wt" );
	if (!file)
		return false;

	char s[65536];

	fprintf( file, "P2\n", s );
	fprintf( file, "%d %d\n", dwWidth,dwHeight );
	fprintf( file, "65535\n" );
	for (int y = 0; y < dwHeight; y++)
	{
		for (int x = 0; x < dwWidth; x++)
		{
			fprintf( file, "%d ",(uint)pData[x+y*dwWidth] );
		}
		fprintf( file, "\n" );
	}
	
	fclose( file );
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::LoadPGM( const CString &fileName, uint *pWidthOut, uint *pHeightOut, uint **pImageDataOut)
{
	FILE *file = fopen( fileName,"rt" );
	if (!file)
		return false;

	const char seps[] = " \n\t";
	char *token;


	int width=0;
	int height=0;
	int numColors = 1;


	fseek( file,0,SEEK_END );
	int fileSize = ftell(file);
	fseek( file,0,SEEK_SET );

	char *str = new char[fileSize];
	fread( str,fileSize,1,file );

	token = strtok( str,seps );

	while (token != NULL && token[0] == '#')
	{
		if (token != NULL && token[0] == '#')
			strtok( NULL, "\n" );
		token = strtok( NULL, seps );
	}
	if (stricmp(token,"P2") != 0)
	{
		// Bad file. not supported pgm.
		delete []str;
		fclose( file );
		return false;
	}

	do {
		token = strtok( NULL, seps );
		if (token != NULL && token[0] == '#')	{
			strtok( NULL, "\n" );
		}
	} while (token != NULL && token[0] == '#');
	width = atoi(token);

	do {
		token = strtok( NULL, seps );
		if (token != NULL && token[0] == '#')
			strtok( NULL, "\n" );
	} while (token != NULL && token[0] == '#');
	height = atoi(token);
	
	do {
		token = strtok( NULL, seps );
		if (token != NULL && token[0] == '#')
			strtok( NULL, "\n" );
	} while (token != NULL && token[0] == '#');
	numColors = atoi(token);

	*pWidthOut = width;
	*pHeightOut = height;

	uint *pImage = new uint[width*height];
	*pImageDataOut = pImage;

	uint *p = pImage;
	int size = width*height;
	int i = 0;
	while (token != NULL && i < size)
	{
		do { token = strtok( NULL,seps ); } while (token != NULL && token[0] == '#');
		*p++ = atoi(token);
		i++;
	}

	delete []str;
	
	fclose( file );
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static inline ushort us_endian (const byte* ptr)
{
  short n;
  memcpy(&n, ptr, sizeof(n));
  return n;
}

static inline unsigned long ul_endian (const byte* ptr)
{
  long n;
  memcpy(&n, ptr, sizeof(n));
  return n;
}

static inline long l_endian (const byte* ptr)
{
  long n;
  memcpy(&n, ptr, sizeof(n));
  return n;
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
bool CImageUtil::LoadBmp( const CString &fileName,CImage &image )
{
#pragma pack(push,1)
	struct SRGBcolor
	{
		uchar red, green, blue;
	};
	struct SRGBPixel
	{
		uchar red, green, blue, alpha;
	};
#pragma pack(pop)

	std::vector<uchar> data;

	CCryFile file;
	if (!file.Open( fileName,"rb"))
	{
		CLogFile::FormatLine( "File not found %s",(const char*)fileName );
		return false;
	}

	long iSize = file.GetLength();

	data.resize(iSize);
  uchar* iBuffer = &data[0];
	file.Read( iBuffer,iSize );

	if (!((memcmp(iBuffer, BM, 2) == 0) && BISIZE(iBuffer) == WinHSize))
	{
		// Not bmp file.
		CLogFile::FormatLine( "Invalid BMP file format %s",(const char*)fileName );
		return false;
	}

	int mWidth = BIWIDTH(iBuffer);
	int mHeight = BIHEIGHT(iBuffer);
	image.Allocate(mWidth,mHeight);
  const int bmp_size = mWidth * mHeight;

  byte *iPtr = iBuffer + BFOFFBITS(iBuffer);

  // The last scanline in BMP corresponds to the top line in the image
  int  buffer_y = mWidth * (mHeight - 1);
  bool blip     = false;

  if (BITCOUNT(iBuffer) == _256Color)
  {
    //mpIndexImage = mfGet_IndexImage();
    byte *buffer = new uchar[mWidth*mHeight];
    SRGBcolor mspPal[256];
    SRGBcolor *pwork = mspPal;
    byte    *inpal   = BIPALETTE(iBuffer);
    //mfSet_bps (8);

    for (int color=0; color<256; color++, pwork++)
    {  
      // Whacky BMP palette is in BGR order.
      pwork->blue  = *inpal++;
      pwork->green = *inpal++;
      pwork->red   = *inpal++;
      inpal++; // Skip unused byte.
    }

    if (BICOMP(iBuffer) == BI_RGB)
    {
      // Read the pixels from "top" to "bottom"
      while (iPtr < iBuffer + iSize && buffer_y >= 0)
      {
        memcpy (buffer + buffer_y, iPtr, mWidth);
        iPtr += mWidth;
        buffer_y -= mWidth;
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
            buffer_y -= mWidth;
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
          buffer_y -= (mWidth * (*iPtr++));
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

          if (++buffer_x >= mWidth)
          {
            buffer_x  = 0;
            buffer_y -= mWidth;
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

		// Convert indexed to RGBA
		for (int y = 0; y < mHeight; y++)
		{
			for (int x = 0; x < mWidth; x++)
			{
				SRGBcolor &entry = mspPal[buffer[x+y*mWidth]];
				image.ValueAt(x,y) = 0xFF000000 | RGB(entry.red,entry.green,entry.blue);
			}
		}

		delete buffer;
    return true;
  }
  else
  if (!BICLRUSED(iBuffer) && BITCOUNT(iBuffer) == TRUECOLOR24)
  {
    //mfSet_bps (24);
    SRGBPixel *buffer = (SRGBPixel*)image.GetData();

    while (iPtr < iBuffer + iSize && buffer_y >= 0)
    {
      SRGBPixel *d = buffer + buffer_y;
      for (int x = mWidth; x; x--)
      {
        d->blue    = *iPtr++;
        d->green   = *iPtr++;
        d->red     = *iPtr++;
        d->alpha = 255;
        d++;
      } /* endfor */

      buffer_y -= mWidth;
    }
    return true;
  }
  else
  if (!BICLRUSED(iBuffer) && BITCOUNT(iBuffer) == TRUECOLOR32)
  {
    //mfSet_bps (32);
    SRGBPixel *buffer = (SRGBPixel*)image.GetData();

    while (iPtr < iBuffer + iSize && buffer_y >= 0)
    {
      SRGBPixel *d = buffer + buffer_y;
      for (int x = mWidth; x; x--)
      {
        d->blue    = *iPtr++;
        d->green   = *iPtr++;
        d->red     = *iPtr++;
        d->alpha   = *iPtr++;
        d++;
      } /* endfor */

      buffer_y -= mWidth;
    }
    return true;
  }

	CLogFile::FormatLine( "Unknown BMP image format %s",(const char*)fileName );

  return false;
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::LoadImage( const CString &fileName, CImage &image )
{
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];

	_splitpath( fileName,drive,dir,fname,ext );
	if (stricmp(ext,".bmp") == 0)
	{
		return LoadBmp( fileName,image );
	} else if (stricmp(ext,".jpg") == 0)
	{
		return LoadJPEG( fileName,image );
	} else if (stricmp(ext,".gif") == 0)
	{
		CImageGif gif;
		return gif.Load( fileName,image );
	} else if (stricmp(ext,".pgm") == 0)
	{
		UINT w,h;
		uint *pData;
		bool res = LoadPGM( fileName,&w,&h,&pData );
		if (!res)
			return false;
		image.Allocate(w,h);
		memcpy( image.GetData(),pData,image.GetSize() );
		delete []pData;
	}
	else if (stricmp(ext,".dds") == 0)
	{
		// Load DDS file.
		CImage_DXTC dds;
		return dds.Load( fileName,image );
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CImageUtil::SaveImage( const CString &fileName, CImage &image )
{
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];

	// Remove the read-only attribute so the file can be overwritten.
	SetFileAttributes(fileName,FILE_ATTRIBUTE_NORMAL);

	_splitpath( fileName,drive,dir,fname,ext );
	if (stricmp(ext,".bmp") == 0)
	{
		return SaveBitmap( fileName,image );
	} else if (stricmp(ext,".jpg") == 0)
	{
		return SaveJPEG( fileName,image );
	} else if (stricmp(ext,".pgm") == 0)
	{
		return SavePGM( fileName,image.GetWidth(),image.GetHeight(),image.GetData() );
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::ScaleToFit( const CByteImage &srcImage,CByteImage &trgImage )
{
	uint x,y,u,v;
  unsigned char *destRow,*dest,*src,*sourceRow;

	uint srcW = srcImage.GetWidth();
	uint srcH = srcImage.GetHeight();

	uint trgW = trgImage.GetWidth();
	uint trgH = trgImage.GetHeight();

	uint xratio = (srcW << 16)/trgW;
	uint yratio = (srcH << 16)/trgH;

	src = srcImage.GetData();
	destRow = trgImage.GetData();

	v = 0;
	for (y = 0; y < trgH; y++)
	{
		u=0;
		sourceRow = src + (v >> 16) * srcW;
		dest = destRow;
		for (x = 0; x < trgW; x++)
		{
			*dest++ = sourceRow[u>>16];
			u += xratio;
		}
		v += yratio;
		destRow += trgW;
	}
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::ScaleToFit( const CImage &srcImage,CImage &trgImage )
{
	uint x,y,u,v;
  unsigned int *destRow,*dest,*src,*sourceRow;

	uint srcW = srcImage.GetWidth();
	uint srcH = srcImage.GetHeight();

	uint trgW = trgImage.GetWidth();
	uint trgH = trgImage.GetHeight();

	uint xratio = (srcW << 16)/trgW;
	uint yratio = (srcH << 16)/trgH;

	src = srcImage.GetData();
	destRow = trgImage.GetData();

	v = 0;
	for (y = 0; y < trgH; y++)
	{
		u=0;
		sourceRow = src + (v >> 16) * srcW;
		dest = destRow;
		for (x = 0; x < trgW; x++)
		{
			*dest++ = sourceRow[u>>16];
			u += xratio;
		}
		v += yratio;
		destRow += trgW;
	}
}

//////////////////////////////////////////////////////////////////////////
void CImageUtil::SmoothImage( CByteImage &image,int numSteps )
{
	assert( numSteps > 0 );
	uchar *buf = image.GetData();
	int w = image.GetWidth();
	int h = image.GetHeight();

	for (int steps = 0; steps < numSteps; steps++)
	{
		// Smooth the image.
		for (int y = 1; y < h-1; y++)
		{
			// Precalculate for better speed
			uchar *ptr = &buf[y*w + 1];

			for (int x = 1; x < w-1; x++)
			{
				// Smooth it out
				*ptr =
				( 
		(uint)ptr[1] +
					ptr[w] +
					ptr[-1] +
					ptr[-w] +
					ptr[w+1] +
					ptr[w-1] +
					ptr[-w+1] +
					ptr[-w-1]
				) >> 3;

				// Next pixel
				ptr++;
			}
		}
	}
}

unsigned char CImageUtil::GetBilinearFilteredAt( const int iniX256, const int iniY256, const CByteImage &image )
{
//	assert(image.IsValid());		if(!image.IsValid())return(0);		// this shouldn't be 

	DWORD x=(DWORD)(iniX256) >> 8;
	DWORD y=(DWORD)(iniY256) >> 8;

	if(x>=image.GetWidth()-1 || y>=image.GetHeight()-1)
		return image.ValueAt(x,y);															// border is not filtered, 255 to get in range 0..1

	DWORD rx=(DWORD)(iniX256) & 0xff;		// fractional aprt
	DWORD ry=(DWORD)(iniY256) & 0xff;		// fractional aprt

	DWORD top=(DWORD)image.ValueAt((int)x  ,(int)y  )*(256-rx)						// left top
					 +(DWORD)image.ValueAt((int)x+1,(int)y  )*rx;									// right top

	DWORD bottom =(DWORD)image.ValueAt((int)x  ,(int)y+1)*(256-rx)				// left bottom
							 +(DWORD)image.ValueAt((int)x+1,(int)y+1)*rx;							// right bottom

	return (unsigned char)((top*(256-ry) + bottom*ry)>>16);
}

