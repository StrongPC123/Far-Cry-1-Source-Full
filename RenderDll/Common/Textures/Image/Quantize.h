
#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

extern void shQuantizeRGB (SRGBPixel *image, int pixels, int pixperline,
  byte *&outimage, SRGBPixel *&outpalette, int &maxcolors, bool dither);


/// Begin quantization
extern void shQuantizeBegin ();
/// Finish quantization
extern void shQuantizeEnd ();
/// Count the colors in a image and update the color histogram
extern void shQuantizeCount (SRGBPixel *image, int pixels,
  SRGBPixel *transp = NULL);
/// Bias the color histogram towards given colors (weight = 0..100)
extern void shQuantizeBias (SRGBPixel *colors, int count, int weight);
/// Compute the optimal palette for all images passed to QuantizeCount()
extern void shQuantizePalette (SRGBPixel *&outpalette, int &maxcolors,
  SRGBPixel *transp = NULL);
/// Remap a image to the palette computed by shQuantizePalette()
extern void shQuantizeRemap (SRGBPixel *image, int pixels,
  byte *&outimage, SRGBPixel *transp = NULL);
/// Same but apply Floyd-Steinberg dithering for nicer (but slower) results
extern void shQuantizeRemapDither (SRGBPixel *image, int pixels, int pixperline,
  SRGBPixel *palette, int colors, byte *&outimage, SRGBPixel *transp = NULL);

#endif // __QUANTIZE_H__
