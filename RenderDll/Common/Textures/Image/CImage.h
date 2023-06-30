
#ifndef CIMAGE_H
#define CIMAGE_H

#include <assert.h>

#define CHK(x) x

#define SH_LITTLE_ENDIAN

// The mask for extracting just R/G/B from an ulong or SRGBPixel
#ifdef SH_BIG_ENDIAN
#  define RGB_MASK 0xffffff00
#else
#  define RGB_MASK 0x00ffffff
#endif

/**
 * An RGB pixel.
 */
struct SRGBPixel
{
  uchar blue, green, red, alpha;
  SRGBPixel () /* : red(0), green(0), blue(0), alpha(255) {} */
  { *(unsigned long *)this = (unsigned long)~RGB_MASK; }
  SRGBPixel (int r, int g, int b) : red (r), green (g), blue (b), alpha (255) {}
  bool eq (const SRGBPixel& p) const { return ((*(unsigned long *)this) & RGB_MASK) == ((*(unsigned long *)&p) & RGB_MASK); }
  /// Get the pixel intensity
  int Intensity () { return (red + green + blue) / 3; }
};


/**
 * An RGB palette entry with statistics information.
 */
struct SRGBPalEntry
{
  uchar red, green, blue;
  long count;
};

/**
 * Possible errors for CImageFile::mfGet_error.
 */
enum EImFileError { eIFE_OK = 0, eIFE_IOerror, eIFE_OutOfMemory, eIFE_BadFormat };

/**
 * Eye sensivity to different color components, from NTSC grayscale equation.
 * The coefficients are multiplied by 100 and rounded towards nearest integer,
 * to facilitate integer math. The squared coefficients are also multiplied
 * by 100 and rounded to nearest integer (thus 173 == 1.73, 242 == 2.42 etc).
 */
/// Red component sensivity
#define R_COEF		173
/// Green component sensivity
#define G_COEF		242
/// Blue component sensivity
#define B_COEF		107
/// Eye sensivity to different color components, squared
/// Red component sensivity, squared
#define R_COEF_SQ	299
/// Green component sensivity, squared
#define G_COEF_SQ	587
/// Blue component sensivity, squared
#define B_COEF_SQ	114

#define FIM_NORMALMAP 1
#define FIM_DSDT      2

/**
 * An abstract class implementing an image loader. For every image
 * type supported, a subclass should be created for loading that image
 * type and ImageFile::load_file should be extended to recognize that
 * image format.
 */
class CImageFile
{
friend class CImageDDSFile;
friend class CImageCCTFile;
friend class CImageBmpFile;
friend class CImagePcxFile;
friend class CImageJpgFile;
friend class CTexMan;

private:
  /// Width of image.
  int m_Width;
  /// Height of image.
  int m_Height;
  /// Depth of image.
  int m_Depth;

  int m_Bps;
  int m_ImgSize;

  int m_NumMips;
  int m_Flags;

  /// The image data.
  union 
  {
    SRGBPixel* m_pPixImage;
    byte* m_pByteImage;
  };

  /// Last error code.
  static EImFileError m_eError;
  /// Last error detail information.
  static char m_Error_detail[256];

protected:

  EImFormat m_eFormat;
  SRGBPixel* m_pPal;

  /**
   * Constructor is private since this object can only be
   * created by load_file.
   */
  CImageFile ();

  /**
   * Before failing, a ImageFile subclass should call set_error to
   * set the code and detail.
   */
  static void mfSet_error (EImFileError error, char* detail = NULL);

  /**
   * Set the width and height. This will also allocate the 'image'
   * buffer to hold the bitmap.
   */
  void mfSet_dimensions (int w, int h);

public:
  static char m_CurFileName[128];
  char m_FileName[128];

  ///
  virtual ~CImageFile ();

  ///
  int mfGet_width () { return m_Width; }
  ///
  int mfGet_height () { return m_Height; }
  ///
  int mfGet_depth () { return m_Depth; }
  ///
  byte* mfGet_image ()
  {
    if (!m_pByteImage)
    {
      if (m_ImgSize)
        m_pByteImage = new byte [m_ImgSize];
    }
    return m_pByteImage;
  }
  ///
  SRGBPixel* mfGet_palette () { return m_pPal; }

  int mfGet_bps () { return m_Bps; }
  void mfSet_bps(int b) { m_Bps = b; }
  void mfSet_ImageSize (int Size) {m_ImgSize = Size;}
  int mfGet_ImageSize () {return m_ImgSize;}
  
  EImFormat mfGetFormat() { return m_eFormat; }

  void mfSet_numMips (int num) { m_NumMips = num; }
  int  mfGet_numMips (void) { return m_NumMips; }
  void mfSet_Flags (int Flags) { m_Flags |= Flags; }
  int mfGet_Flags () { return m_Flags; }

  ///
  static EImFileError mfGet_error () { return m_eError; }
  ///
  static char* mfGet_error_detail () { return m_Error_detail ? m_Error_detail : (char *)""; }
  /// Write a message describing the error on screen.
  static void mfWrite_error (char* extra);

  /**
   * Load the file given the filename.
   * This routine will open the file and call load_file (FILE*).
   */
  static CImageFile* mfLoad_file (char* filename);

  /**
   * Load the file given a file pointer.
   * This routine will read from the file pointer and call load_file (UByte*, long).
   */
  static CImageFile* mfLoad_file (FILE* fp);

  /**
   * Load the file from a buffer.
   * This routine will try to recognize the image file type and then
   * created an appropriate ImageFile subclass.
   */
  static CImageFile* mfLoad_file (byte* buf, long size);
};

#include "Quantize.h"
#include "Inv_cmap.h"

#endif

