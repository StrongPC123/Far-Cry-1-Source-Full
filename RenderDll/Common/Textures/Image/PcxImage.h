
#ifndef PCXIMAGE_H
#define PCXIMAGE_H

/**
 * An ImageFile subclass for reading PCX files.
 */
class CImagePcxFile : public CImageFile
{
  ///
  friend class CImageFile;	// For constructor

private:
  /// Read the PCX file from the buffer.
  CImagePcxFile (byte* buf, long size);

public:
  ///
  virtual ~CImagePcxFile ();
};


#endif


