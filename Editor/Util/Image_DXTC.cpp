////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   image_dxtc.cpp
//  Version:     v1.00
//  Created:     5/9/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Image_DXTC.h"
#include <CryFile.h>

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)
#include "dds.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CImage_DXTC::CImage_DXTC()
{
}

CImage_DXTC::~CImage_DXTC()
{
}

static ETEX_Format DecodePixelFormat( DDS_PIXELFORMAT* pddpf )
{
	ETEX_Format format = eTF_Unknown;
	switch( pddpf->dwFourCC )
	{
	case 0:
		// This dds texture isn't compressed so write out ARGB format
		format = eTF_8888;
		break;

	case MAKEFOURCC('D','X','T','1'):
		format = eTF_DXT1;
		break;

	case MAKEFOURCC('D','X','T','2'):
		//format = eTF_DXT2;
		break;

	case MAKEFOURCC('D','X','T','3'):
		format = eTF_DXT3;
		break;

	case MAKEFOURCC('D','X','T','4'):
		//format = eTF_DXT4;
		break;

	case MAKEFOURCC('D','X','T','5'):
		format = eTF_DXT5;
		break;
	}
	return format;
}

//////////////////////////////////////////////////////////////////////////
bool CImage_DXTC::Load( const char *filename,CImage &outImage )
{
	CCryFile file;
	if (!file.Open( filename,"rb" ))
	{
		return( false );
	}

	DDS_HEADER ddsd;
	DWORD			dwMagic;

	BYTE	*pCompBytes;		// compressed image bytes
	BYTE	*pDecompBytes;


	// Read magic number
	file.Read( &dwMagic, sizeof(DWORD) );

	if( dwMagic != MAKEFOURCC('D','D','S',' ') )
	{
		return( false);
	}

	// Read the surface description
	file.Read( &ddsd, sizeof(DDS_HEADER) );


	// Does texture have mipmaps?
	bool bMipTexture = ( ddsd.dwMipMapCount > 0 ) ? TRUE : FALSE;

	ETEX_Format format = DecodePixelFormat( &ddsd.ddspf );

	if (format == eTF_Unknown)
	{
		return false;
	}

	int nCompSize = file.GetLength() - sizeof(DDS_HEADER) - sizeof(DWORD);
	pCompBytes = new BYTE[nCompSize+32];
	file.Read( pCompBytes, nCompSize );

	/*
	// Read only first mip level for now:
	if (ddsd.dwFlags & DDSD_LINEARSIZE)
	{
		pCompBytes = new BYTE[ddsd.dwLinearSize];
		if (pCompBytes == NULL )
		{
			return( false );
		}
		file.Read( pCompBytes, ddsd.dwLinearSize );
	}
	else
	{
		DWORD dwBytesPerRow = ddsd.dwWidth * ddsd.ddspf.dwRGBBitCount / 8;
		pCompBytes = new BYTE[ddsd.lPitch*ddsd.dwHeight];
		if (pCompBytes == NULL )
		{
			return( false );
		}

		nCompSize = ddsd.lPitch * ddsd.dwHeight;
		nCompLineSz = dwBytesPerRow;

		BYTE* pDest = pCompBytes;
		for( DWORD yp = 0; yp < ddsd.dwHeight; yp++ )
		{
			file.Read( pDest, dwBytesPerRow );
			pDest += ddsd.lPitch;
		}
	}
	*/

	outImage.Allocate( ddsd.dwWidth,ddsd.dwHeight );
	pDecompBytes = (BYTE*)outImage.GetData();
	if (!pDecompBytes)
	{
		Warning( "Cannot allocate image %dx%d, Out of memory",ddsd.dwWidth,ddsd.dwHeight );
	}

	if (format == eTF_8888)
	{
		if (ddsd.ddspf.dwRGBBitCount == 24)
		{
			unsigned char *dest = pDecompBytes;
			unsigned char *src = pCompBytes;
			for (int y = 0; y < ddsd.dwHeight; y++)
			{
				for (int x = 0; x < ddsd.dwWidth; x++)
				{
					dest[0] = src[0];
					dest[1] = src[1];
					dest[2] = src[2];
					dest[3] = 255;
					dest+=4;
					src+=3;
				}
			}
		}
		else if (ddsd.ddspf.dwRGBBitCount == 32)
		{
			unsigned char *dest = pDecompBytes;
			unsigned char *src = pCompBytes;
			for (int y = 0; y < ddsd.dwHeight; y++)
			{
				for (int x = 0; x < ddsd.dwWidth; x++)
				{
					dest[0] = src[0];
					dest[1] = src[1];
					dest[2] = src[2];
					dest[3] = src[3];
					dest+=4;
					src+=4;
				}
			}
		}
		//memcpy( pDecompBytes,pCompBytes,nCompSize );
	}
	else if (format == eTF_DXT1 || format == eTF_DXT3 || format == eTF_DXT5)
	{
		GetIEditor()->GetRenderer()->DXTDecompress( pCompBytes,pDecompBytes,ddsd.dwWidth,ddsd.dwHeight,format,false,4 );
	}

	delete []pCompBytes;
	
	outImage.SwapRedAndBlue();

	// done reading file
	return( true );
}