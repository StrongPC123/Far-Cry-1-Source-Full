
#ifndef JPGIMAGE_H
#define JPGIMAGE_H

/**
 * An ImageFile subclass for reading JPG files.<p>
 * This implementation needs libjpeg to read JFIF files.
 */
class CImageJpgFile : public CImageFile
{
  ///
  friend class CImageFile;	// For constructor

private:
  /// Read the JPG file from the buffer.
  CImageJpgFile (byte* buf, long size);

public:
  ///
  virtual ~CImageJpgFile ();
};

#endif //JPGIMAGE_H

