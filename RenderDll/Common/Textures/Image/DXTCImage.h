/*********************************************************************NVMH2****
File:  Image_DXTC.h

Copyright (C) 1999, 2000 NVIDIA Corporation
Copyright (C) 2002, Ubi Soft Milan
					Tiziano Sardone


Comments:
A class to load and decompress DXT textures to 32-bit raw image data format.
.RAW output files can be loaded into photoshop by specifying the resolution 
and 4 color channels of 8-bit, interleaved.

A few approaches to block decompression are in place and a simple code timing
function is called.  Output of timing test is saved to a local .txt file.

TiZ: some modification to adapt the code to run under PS2.

******************************************************************************/



#if !defined(AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_)
#define AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#include <d3d.h>
#endif

#ifdef PS2
/////////////////////////////////////
// should be in ddraw.h

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

#endif

struct TimingInfo;		// defined in Image_DXTC.cpp

#define byte 	unsigned char
#define BYTE 	unsigned char
//#ifndef PS2
#define WORD	unsigned short
#define DWORD	unsigned int
#define LONG	unsigned int
#define	LPVOID	void*
#define VOID	void
#define CHAR	char
#define LARGE_INTEGER	int
//#endif

enum PixFormat
{
	PF_ARGB,
	PF_DXT1,
	PF_DXT2,
	PF_DXT3,
	PF_DXT4,
	PF_DXT5,
	PF_UNKNOWN,
};

#define COMPRESSED_S3TC_DXT1 PF_DXT1

typedef struct _DDSCAPS2 {
  DWORD  dwCaps;
  DWORD  dwCaps2;
  DWORD  dwCaps3;
  DWORD  dwCaps4;
} DDSCAPS2, *LPDDSCAPS2;

typedef struct _DDPIXELFORMAT {
  DWORD  dwSize;
  DWORD  dwFlags;
  DWORD  dwFourCC;
union {
  DWORD  dwRGBBitCount;
  DWORD  dwYUVBitCount;
  DWORD  dwZBufferBitDepth;
  DWORD  dwAlphaBitDepth;
  DWORD  dwLuminanceBitCount;
  DWORD  dwBumpBitCount;
  DWORD  dwPrivateFormatBitCount;
} ;
union {
  DWORD  dwRBitMask;
  DWORD  dwYBitMask;
  DWORD  dwStencilBitDepth;
  DWORD  dwLuminanceBitMask;
  DWORD  dwBumpDuBitMask;
  DWORD  dwOperations;
} ;
union {
  DWORD  dwGBitMask;
  DWORD  dwUBitMask;
  DWORD  dwZBitMask;
  DWORD  dwBumpDvBitMask;
  struct {
    WORD wFlipMSTypes;
    WORD wBltMSTypes;
  } MultiSampleCaps;
} ;
union {
  DWORD  dwBBitMask;
  DWORD  dwVBitMask;
  DWORD  dwStencilBitMask;
  DWORD  dwBumpLuminanceBitMask;
} ;
union {
  DWORD  dwRGBAlphaBitMask;
  DWORD  dwYUVAlphaBitMask;
  DWORD  dwLuminanceAlphaBitMask;
  DWORD  dwRGBZBitMask;
  DWORD  dwYUVZBitMask;
} ;
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

typedef struct _DDCOLORKEY{ 
    DWORD dwColorSpaceLowValue; 
    DWORD dwColorSpaceHighValue; 
} DDCOLORKEY, *LPDDCOLORKEY;  

typedef struct _DDSURFACEDESC2 {
    DWORD         dwSize;
    DWORD         dwFlags;
    DWORD         dwHeight;
    DWORD         dwWidth;
    union
    {
        LONG      lPitch;
        DWORD     dwLinearSize;
    } DUMMYUNIONNAMEN_1;
    DWORD         dwBackBufferCount;
    union
    {
        DWORD     dwMipMapCount;
        DWORD     dwRefreshRate;
    } DUMMYUNIONNAMEN_2;
    DWORD         dwAlphaBitDepth;
    DWORD         dwReserved;
    LPVOID        lpSurface;
    DDCOLORKEY    ddckCKDestOverlay;
    DDCOLORKEY    ddckCKDestBlt;
    DDCOLORKEY    ddckCKSrcOverlay;
    DDCOLORKEY    ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2      ddsCaps;
    DWORD         dwTextureStage;
} DDSURFACEDESC2, *LPDDSURFACEDESC2; 


class Image_DXTC  
{
public:
	unsigned char	* m_pCompBytes;		// compressed image bytes
	unsigned char	* m_pDecompBytes;

	int		m_nCompSize;
	int		m_nCompLineSz;


	char			m_strFormat[256];
	PixFormat		m_CompFormat;

	DDSURFACEDESC2      m_DDSD;			// read from dds file
    bool				m_bMipTexture;	// texture has mipmaps?


	int	m_nWidth;	// in pixels of uncompressed image 
	int m_nHeight;

	bool LoadFromFile( char * filename );		// true if success

	VOID DecodePixelFormat( CHAR* strPixelFormat, DDPIXELFORMAT* pddpf );

	void AllocateDecompBytes();

	void Decompress();
	
	void DecompressDXT1();
	void DecompressDXT2();
	void DecompressDXT3();
	void DecompressDXT4();
	void DecompressDXT5();

	void SaveAsRaw8888(const char *name);			// save decompressed bits
	void SaveAsRaw888(const char *name);			// save decompressed bits

	void RunTimingSession();	// run a few methods & time the code
								// must use dxt5 texture
	void Time_Decomp5_01( int ntimes, TimingInfo * info );
	void Time_Decomp5_02( int ntimes, TimingInfo * info );
	void Time_Decomp5_03( int ntimes, TimingInfo * info );
	void Time_Decomp5_04( int ntimes, TimingInfo * info );


	Image_DXTC();
	virtual ~Image_DXTC();

};

#endif // !defined(AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_)
