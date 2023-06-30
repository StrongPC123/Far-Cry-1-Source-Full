
#ifndef BMPIMAGE_H
#define BMPIMAGE_H

/**
 * An ImageFile subclass for reading BMP files.
 */
class CImageBmpFile : public CImageFile
{
  ///
  friend class CImageFile;	// For constructor

private:
  /// Read the BMP file from the buffer.
  CImageBmpFile (byte* buf, long size);
  void mfLoadWindowsBitmap (byte* ptr, long filesize);

public:
  ///
  virtual ~CImageBmpFile ();
};


#endif


