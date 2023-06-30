/*=============================================================================
  PcxImage.cpp : PCX image file format implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "CImage.h"
#include "PcxImage.h"
#if defined(LINUX)
#include "ILog.h"
#endif


typedef struct
{
  char	manufacturer;
  char	version;
  char	encoding;
  char	bits_per_pixel;
  short	xmin;
  short	ymin;
  short	xmax;
  short	ymax;
  short	hres;
  short	vres;
  byte	palette[48];
  char	reserved;
  char	color_planes;
  short	bytes_per_line;
  short	palette_type;
  char	filler[58];
  byte	data;
} pcx_header;

CImagePcxFile::~CImagePcxFile ()
{
}
 
CImagePcxFile::CImagePcxFile (byte* ptr, long filesize) : CImageFile ()
{
  pcx_header *pcx;
  byte* raw, *p, dat;
  int x, y, runLength;
  int sx, sy;

  //
  // parse the PCX file
  //
  pcx = (pcx_header *)ptr;
  raw = &pcx->data;

  if (pcx->manufacturer != 0x0a
      || pcx->version != 5
      || pcx->encoding != 1
      || pcx->bits_per_pixel != 8
      || pcx->xmax >= 640
      || pcx->ymax >= 480)
  {
      mfSet_error (eIFE_BadFormat, "not a PCX file");
      return;
  }


  sx=pcx->xmax+1;
  sy=pcx->ymax+1;

  m_eFormat = eIF_Pcx;
                  
/* Read in global colormap. */
  CHK (m_pPal = new SRGBPixel [256]);

  p = (byte *)pcx + filesize - 768; 
  int i;
  for (i=0; i<256; i++)
  {
    m_pPal[i].red = p[0];
    m_pPal[i].green = p[1];
    m_pPal[i].blue = p[2];
    m_pPal[i].alpha = 255;
    p += 3;
  }

  // Set the dimensions which will also allocate the image data
  // buffer.
  mfSet_dimensions (sx, sy);
  mfSet_ImageSize(m_Width * m_Height);
  byte *IndexImage = mfGet_image ();
  p = IndexImage;

  i = 0;
  for (y=0 ; y<=pcx->ymax ; y++, p += pcx->xmax+1)
  {
    for (x=0 ; x<=pcx->xmax ; )
    {
      dat = *raw++;

      if((dat & 0xC0) == 0xC0)
      {
        runLength = dat & 0x3F;
        dat = *raw++;
      }
      else
        runLength = 1;

      while(runLength-- > 0)
      {
        p[x++] = dat;           
      }
    }
  }
}

void WritePCX (char *name, byte *data, byte *pal, int width, int height)
{
	int i, j, len;
	pcx_header *pcx;
	byte *pack;
	FILE *fp;

	pcx = (pcx_header*)malloc (width*height*2+1000);
	pcx->manufacturer = 10;         // Some programs complains if this is 0
	pcx->version = 5;
	pcx->encoding = 1;
	pcx->bits_per_pixel = 8;
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = width - 1;
	pcx->ymax = height - 1;
	pcx->hres = width;
	pcx->vres = height;
	memset (pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;
	pcx->bytes_per_line = width;
	pcx->palette_type = 2;
	memset (pcx->filler, 0, sizeof(pcx->filler));
	pack = &(pcx->data);

	for (i=0; i<height; i++)
	{
		for (j=0; j<width; j++)
		{
			if  ((*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}
//		data += width;
	}
	*pack++ = 0x0c;
	for (i=0; i<768; i++)
		*pack++ = *pal++;
	len = pack - (byte *)pcx;
	fp = fxopen (name, "wb");
	if (!fp)
    return;
	fwrite (pcx, len, 1, fp);
	fclose (fp);
	free (pcx);
}
