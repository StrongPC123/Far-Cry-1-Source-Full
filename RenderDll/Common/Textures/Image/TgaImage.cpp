/*=============================================================================
  TgaImage.cpp : TGA image file format implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "CImage.h"
#include "TgaImage.h"



#define MAXCOLORS 16384

static int mapped, rlencoded;

static SRGBPixel ColorMap[MAXCOLORS];
static int RLE_count = 0, RLE_flag = 0;

static void readtga ( byte*& ptr, struct SImageHeader* tgaP );
static void get_map_entry ( byte*& ptr, SRGBPixel* Value, int Size );
static void get_pixel ( byte*& ptr, SRGBPixel* dest, int Size );
static byte getbyte ( byte*& ptr );

CImageTgaFile::~CImageTgaFile ()
{
}

CImageTgaFile::CImageTgaFile (byte* ptr, long filesize) : CImageFile ()
{
  (void)filesize;
  struct SImageHeader tga_head;
  int i;
  unsigned int temp1, temp2;
  int rows, cols, row, col, realrow, truerow, baserow;
  int maxval;
  SRGBPixel *pixels;

  /* @@@ to do: Add TGA format detection */

  /* Read the Targa file header. */
  readtga (ptr, &tga_head);

  rows = ( (int) tga_head.Height_lo ) + ( (int) tga_head.Height_hi ) * 256;
  cols = ( (int) tga_head.Width_lo ) + ( (int) tga_head.Width_hi ) * 256;

  m_eFormat = eIF_Tga;

  switch ( tga_head.ImgType )
  {
    case TGA_Map:
    case TGA_RGB:
    case TGA_Mono:
    case TGA_RLEMap:
    case TGA_RLERGB:
    case TGA_RLEMono:
    break;

    default:
      mfSet_error (eIFE_BadFormat, "Unknown Targa image type");
      return;
  }

  if ( tga_head.ImgType == TGA_Map ||
       tga_head.ImgType == TGA_RLEMap ||
       tga_head.ImgType == TGA_CompMap ||
       tga_head.ImgType == TGA_CompMap4 )
  { /* Color-mapped image */
    if ( tga_head.CoMapType != 1 )
    {
      mfSet_error (eIFE_BadFormat, "Mapped image with bad color map type");
      return;
    }
    mapped = 1;
    /* Figure maxval from CoSize. */
    switch ( tga_head.CoSize )
    {
      case 8:
      case 24:
      case 32:
        maxval = 255;
        break;

      case 15:
      case 16:
        maxval = 31;
        break;

      default:
        mfSet_error (eIFE_BadFormat, "Unknown colormap pixel size");
        return;
    }
  }
  else
  { /* Not colormap, so figure maxval from PixelSize. */
    mapped = 0;
    switch ( tga_head.PixelSize )
    {
      case 8:
      case 24:
      case 32:
        maxval = 255;
        break;

      case 15:
      case 16:
        maxval = 31;
        break;

      default:
        mfSet_error (eIFE_BadFormat, "Unknown pixel size");
        return;
    }
  }

  mfSet_bps(tga_head.PixelSize);

  /* If required, read the color map information. */
  if ( tga_head.CoMapType != 0 )
  {
    temp1 = tga_head.Index_lo + tga_head.Index_hi * 256;
    temp2 = tga_head.Length_lo + tga_head.Length_hi * 256;
    if ( ( temp1 + temp2 + 1 ) >= MAXCOLORS )
    {
      mfSet_error (eIFE_BadFormat, "Too many colors in colormap");
      return;
    }
    for ( i = temp1; i < (int)( temp1 + temp2 ); ++i )
	    get_map_entry( ptr, &ColorMap[i], (int) tga_head.CoSize );
  }

  /* Check run-length encoding. */
  if ( tga_head.ImgType == TGA_RLEMap || tga_head.ImgType == TGA_RLERGB || tga_head.ImgType == TGA_RLEMono )
    rlencoded = 1;
  else
    rlencoded = 0;

  /* Read the Targa file body and convert to portable format. */
  mfSet_dimensions (cols, rows);
  mfSet_ImageSize(cols * rows * 4);
  pixels = (SRGBPixel *)mfGet_image();

  truerow = 0;
  baserow = 0;
  for ( row = 0; row < rows; ++row )
  {
    realrow = truerow;
    if ( tga_head.OrgBit == 0 )
      realrow = rows - realrow - 1;

    for ( col = 0; col < cols; ++col )
      get_pixel( ptr, &(pixels[realrow*cols+col]), (int) tga_head.PixelSize );
    if ( tga_head.IntrLve == TGA_IL_Four )
      truerow += 4;
    else
    if ( tga_head.IntrLve == TGA_IL_Two )
      truerow += 2;
    else
     ++truerow;
    if ( truerow >= rows )
      truerow = ++baserow;
  }
}


static void readtga (byte*& ptr, SImageHeader* tgaP)
{
    byte flags;

    tgaP->IDLength = getbyte( ptr );
    tgaP->CoMapType = getbyte( ptr );
    tgaP->ImgType = getbyte( ptr );
    tgaP->Index_lo = getbyte( ptr );
    tgaP->Index_hi = getbyte( ptr );
    tgaP->Length_lo = getbyte( ptr );
    tgaP->Length_hi = getbyte( ptr );
    tgaP->CoSize = getbyte( ptr );
    tgaP->X_org_lo = getbyte( ptr );
    tgaP->X_org_hi = getbyte( ptr );
    tgaP->Y_org_lo = getbyte( ptr );
    tgaP->Y_org_hi = getbyte( ptr );
    tgaP->Width_lo = getbyte( ptr );
    tgaP->Width_hi = getbyte( ptr );
    tgaP->Height_lo = getbyte( ptr );
    tgaP->Height_hi = getbyte( ptr );
    tgaP->PixelSize = getbyte( ptr );
    flags = getbyte( ptr );
    tgaP->AttBits = flags & 0xf;
    tgaP->Rsrvd = ( flags & 0x10 ) >> 4;
    tgaP->OrgBit = ( flags & 0x20 ) >> 5;
    tgaP->IntrLve = ( flags & 0xc0 ) >> 6;

    if ( tgaP->IDLength != 0 )
      ptr += tgaP->IDLength;
}

static void get_map_entry (byte*& ptr, SRGBPixel* Value, int Size)
{
  byte j, k, r, g, b;
  r=g=b=0; /* get rid of 'might be used uninited' warning */

  /* Read appropriate number of bytes, break into rgb & put in map. */
  switch ( Size )
	{
  	case 8:				/* Grey scale, read and triplicate. */
      r = g = b = getbyte( ptr );
      break;

    case 16:			/* 5 bits each of red green and blue. */
    case 15:			/* Watch for byte order. */
      j = getbyte( ptr );
      k = getbyte( ptr );
      r = ( k & 0x7C ) >> 2;
      g = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
      b = j & 0x1F;
      break;

      case 32:
      case 24:			/* 8 bits each of blue green and red. */
        b = getbyte( ptr );
        g = getbyte( ptr );
        r = getbyte( ptr );
        if ( Size == 32 )
          (void) getbyte( ptr );	/* Read alpha byte & throw away. */
        break;

      default:
	      //mfSet_error (eIFE_BadFormat, "Unknown colormap pixel size");
	    return;
	}
  Value->red=r; Value->green=g; Value->blue=b;
}

static void get_pixel (byte*& ptr, SRGBPixel* dest, int Size)
{
    static int Red, Grn, Blu, Alpha;
    byte j, k;
    static unsigned int l;

    if (Size != 32)
      Alpha = 255;
    /* Check if run length encoded. */
    if ( rlencoded )
	{
	if ( RLE_count == 0 )
	    { /* Have to restart run. */
      byte i;
	    i = getbyte( ptr );
	    RLE_flag = ( i & 0x80 );
	    if ( RLE_flag == 0 )
		/* Stream of unencoded pixels. */
		RLE_count = i + 1;
	    else
		/* Single pixel replicated. */
		RLE_count = i - 127;
	    /* Decrement count & get pixel. */
	    --RLE_count;
	    }
	else
	    { /* Have already read count & (at least) first pixel. */
	    --RLE_count;
	    if ( RLE_flag != 0 )
		/* Replicated pixels. */
		goto PixEncode;
	    }
	}
    /* Read appropriate number of bytes, break into RGB. */
    switch ( Size )
	{
	case 8:				/* Grey scale, read and triplicate. */
	Red = Grn = Blu = l = getbyte( ptr );
	break;

	case 16:			/* 5 bits each of red green and blue. */
	case 15:			/* Watch byte order. */
	j = getbyte( ptr );
	k = getbyte( ptr );
	l = ( (unsigned int) k << 8 ) + j;
	Red = ( k & 0x7C ) >> 2;
	Grn = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
	Blu = j & 0x1F;
	break;

	case 32:
	case 24:			/* 8 bits each of blue green and red. */
	Blu = getbyte( ptr );
	Grn = getbyte( ptr );
	Red = getbyte( ptr );
	if ( Size == 32 )
	    Alpha = getbyte( ptr );	/* Read alpha byte & throw away. */
	l = 0;
	break;

	default:
	  //mfSet_error (eIFE_BadFormat, "Unknown pixel size");
	  return;
	}

PixEncode:
  if ( mapped )
  	*dest = ColorMap[l];
  else
  {
	  dest->red=Red;dest->green=Grn;dest->blue=Blu;
    dest->alpha = Alpha;
  }
}

static byte getbyte (byte*& ptr)
{
  byte c = *ptr++;
  return c;
}

//=============================================================

#if !defined(LINUX)
#include <io.h>
#include <fcntl.h>
#endif

static FILE *sFileData;
static int src_bits_per_pixel;

static void bwrite(unsigned char data)
{
  fputc(data, sFileData);
}

void wwrite(unsigned short data)
{
  unsigned char h, l;

  l = data & 0xFF;
  h = data >> 8;
  bwrite(l);
  bwrite(h);
}


static void WritePixel(int depth, unsigned long a, unsigned long r, unsigned long g, unsigned long b)
{
  DWORD color16;

  switch(depth)
  {
    case 32:
        bwrite((byte)b);    // b
        bwrite((byte)g);    // g
        bwrite((byte)r);    // r
        bwrite((byte)a);    // a
        break;

    case 24:
        bwrite((byte)b);    // b
        bwrite((byte)g);    // g
        bwrite((byte)r);    // r
        break;

    case 16:
        r >>= 3;
        g >>= 3;
        b >>= 3;

        r &= 0x1F;
        g &= 0x1F;
        b &= 0x1F;

        color16 = (r << 10) | (g << 5) | b;

        wwrite((unsigned short)color16);
        break;
  }
}

static void GetPixel(unsigned char * data, int depth,  unsigned long &a, unsigned long &r, unsigned long &g, unsigned long &b)
{
  switch(depth)
  {
    case 32:
      r = *data++;
      g = *data++;
      b = *data++;
      a = *data++;
      break;

    case 24:
      r = *data++;
      g = *data++;
      b = *data++;
      a = 0xFF;
      break;

    default:
      assert(0);
      break;
  }
}

void WriteTGA8(byte *data8, int width, int height, char *filename)
{
  unsigned char * data32 = new unsigned char [width*height*4];
  for(int i=0; i<width*height; i++)
  {
    data32[i*4+0] = data8[i];
    data32[i*4+1] = data8[i];
    data32[i*4+2] = data8[i];
    data32[i*4+3] = 255;
  }

  WriteTGA(data32, width, height, filename, 32);
  delete [] data32;
}

void WriteTGA(byte *data, int width, int height, char *filename, int dest_bits_per_pixel)
{
#ifndef PS2
  int i;
  unsigned long r,g,b,a;

	src_bits_per_pixel = 32;

  if ((sFileData = fopen(filename, "wb")) == NULL)
    return;

  //mdesc |= LR;   // left right
  //m_desc |= UL_TGA_BT;   // top

  int id_length = 0;
  int x_org = 0;
  int y_org = 0;
  int desc = 0;

  // 32 bpp

  int cm_index = 0;
  int cm_length = 0;
  int cm_entry_size = 0;
  int color_map_type = 0;

  int type = 2;

  bwrite(id_length);
  bwrite(color_map_type);
  bwrite(type);

  wwrite(cm_index);
  wwrite(cm_length);

  bwrite(cm_entry_size);

  wwrite(x_org);
  wwrite(y_org);
  wwrite((unsigned short) width);
  wwrite((unsigned short) height);

  bwrite( dest_bits_per_pixel );

  bwrite(desc);

  int hxw = height * width;

  int right = 0;
  int top   = 1;

  DWORD * temp_dp = (DWORD*) data;		// data = input pointer

  DWORD * swap = 0;


  if( !top )
  {
  	assert( src_bits_per_pixel == 32 );

    swap = (DWORD *) new DWORD[ hxw ];

  	// copy whole image data to swap buffer
    cryMemcpy(swap, temp_dp, hxw * sizeof( DWORD ));

    DWORD * src, * dest;

    for (i = 0; i < height; i++)
    {
  		// copy lines from old into new buffer

		  src = & temp_dp[ ( height - i - 1) * width ];

		  dest = & swap[ i * width ];

      cryMemcpy(dest, src, width * sizeof(DWORD) );
    }
      // use the swapped area in further processing & to write out the data
      data = (unsigned char *)swap;
  }

  UINT src_bytes_per_pixel = src_bits_per_pixel / 8;

  UINT size_in_bytes	= hxw * src_bytes_per_pixel;

  if( src_bits_per_pixel == dest_bits_per_pixel)
  {
    fwrite(data, hxw, src_bytes_per_pixel, sFileData);
  }
  else
  {
    for (i = 0; i < hxw; i++)
    {
      GetPixel( data, src_bits_per_pixel, a, b, g, r);
      WritePixel( dest_bits_per_pixel, a, b, g, r);
      data += src_bytes_per_pixel;
    }
  }

  fclose(sFileData);

  SAFE_DELETE_ARRAY(swap);
#else
	OutputDebugString("Not Implemented");
#endif
}

void BlurImage8(byte * pImage, int nSize, int nPassesNum)
{
#define DATA_TMP(_x,_y) (pTemp [(_x)+nSize*(_y)])
#define DATA_IMG(_x,_y) (pImage[(_x)+nSize*(_y)])

  byte * pTemp = new byte [nSize*nSize];

  for(int nPass=0; nPass<nPassesNum; nPass++)
  {
    cryMemcpy(pTemp,pImage,nSize*nSize);

    for(int x=1; x<nSize-1; x++)
    for(int y=1; y<nSize-1; y++)
    {
      float fVal = 0;
      fVal += DATA_TMP(x,y);
      fVal += DATA_TMP(x+1,y+1);
      fVal += DATA_TMP(x-1,y+1);
      fVal += DATA_TMP(x+1,y-1);
      fVal += DATA_TMP(x-1,y-1);
      DATA_IMG(x,y) = uchar(fVal*0.2f);
    }
  }

  delete [] pTemp;

#undef DATA_IMG
#undef DATA_TMP
}
