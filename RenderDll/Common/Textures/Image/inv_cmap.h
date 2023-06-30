
#ifndef __INV_CMAP_H__
#define __INV_CMAP_H__

extern void shInverseColormap (int colors, SRGBPixel *colormap,
  int rbits, int gbits, int bbits, byte *&rgbmap,
  unsigned long *dist_buf = NULL);

#endif // __INV_CMAP_H__
