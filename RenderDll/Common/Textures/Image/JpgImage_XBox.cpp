/*=============================================================================
  JpgImage.cpp : JPG image file format implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#if !defined(LINUX)

#include "stdafx.h"
#include "CImage.h"
#include "JpgImage.h"

extern "C"
{
  #include "Jpeg6/Jpeglib.h"
  #include "Jmemsrc.c" // include buffer source input code
}

#include <setjmp.h>

/* ==== Error mgmnt ==== */
static char jpg_err_msg[256];
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr *my_error_ptr;
static void my_error_exit (j_common_ptr cinfo)
{
  char errmsg [256];

  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->format_message) (cinfo,errmsg);
  strcpy (jpg_err_msg,errmsg);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


/* ==== Constructor ==== */
CImageJpgFile::~CImageJpgFile () {
  /* do nothing */
}

CImageJpgFile::CImageJpgFile (byte* ptr, long filesize) : CImageFile () {
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  int bufp;
  int i;
  SRGBPixel *pixels;

  m_eFormat = eIF_Jpg;

  /* ==== Step 1: allocate and initialize JPEG decompression object */
  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = my_jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    my_jpeg_destroy_decompress(&cinfo);
    mfSet_error (eIFE_BadFormat,jpg_err_msg);
    return;
  }
  /* Now we can initialize the JPEG decompression object. */
  my_jpeg_create_decompress(&cinfo);

  /* ==== Step 2: specify data source (memory buffer, in this case) */
  my_jpeg_memory_src(&cinfo, (char *)ptr, filesize);

  /* ==== Step 3: read file parameters with jpeg_read_header() */
  (void) my_jpeg_read_header(&cinfo, TRUE);

  /* ==== Step 4: set parameters for decompression */
  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* ==== Step 5: Start decompressor */

  (void) my_jpeg_start_decompress(&cinfo);
  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */

  mfSet_dimensions (cinfo.output_width, cinfo.output_height);
  mfSet_ImageSize(cinfo.output_width * cinfo.output_height * 4);
  pixels = (SRGBPixel *)mfGet_image();
  bufp = 0;

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* ==== Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) my_jpeg_read_scanlines(&cinfo, buffer, 1);

    /* Assume put_scanline_someplace wants a pointer and sample count. */
    /* put_scanline_someplace(buffer[0], row_stride); */
    if (cinfo.output_components == 1)
    { /* grey scale */
      for (i=0;i<row_stride;i++)
      {
         pixels[bufp].red = buffer[0][i];
         pixels[bufp].green = buffer[0][i];
         pixels[bufp].blue = buffer[0][i];
         pixels[bufp].alpha = 255;
         bufp ++;
      }
    }
    else
    if (cinfo.output_components == 3)
    { /* rgb triplets */
      for (i = 0; i < (int)cinfo.output_width; i++)
      {
        pixels[bufp].red = buffer[0][i*3+0];
        pixels[bufp].green = buffer[0][i*3+1];
        pixels[bufp].blue = buffer[0][i*3+2];
        pixels[bufp].alpha = 255;
        bufp ++;
      }
    }
    else
    {
      for (i = 0; i < (int)cinfo.output_width; i++)
      {
        pixels[bufp].red = buffer[0][i*4+0];
        pixels[bufp].green = buffer[0][i*4+1];
        pixels[bufp].blue = buffer[0][i*4+2];
        pixels[bufp].alpha = 255;
        bufp ++;
      }
    }
  }

  /* ==== Step 7: Finish decompression */

  (void) my_jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the buffer data source.
   */

  /* ==== Step 8: Release JPEG decompression object */
  /* This is an important step since it will release a good deal of memory. */
  my_jpeg_destroy_decompress(&cinfo);


  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
}

#endif