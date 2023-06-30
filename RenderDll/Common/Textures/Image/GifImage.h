
#ifndef GIFIMAGE_H
#define GIFIMAGE_H

/**
 * An ImageFile subclass for reading GIF files.
 */
class CImageGifFile : public CImageFile
{
  ///
  friend class CImageFile;	// For constructor

private:
  /// Read the GIF file from the buffer.
  CImageGifFile (byte* buf, long size);

public:
  ///
  virtual ~CImageGifFile ();
};

#endif


