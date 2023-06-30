
#ifndef DDSIMAGE_H
#define DDSIMAGE_H

/**
 * An ImageFile subclass for reading DDS files.
 */
class CImageDDSFile : public CImageFile
{
  ///
  friend class CImageFile;	// For constructor

private:

public:
  /// Read the DDS file from the buffer.
  CImageDDSFile (byte* buf, long size);
  int mfSizeWithMips(int filesize, int sx, int sy, int numMips);
///
  virtual ~CImageDDSFile ();
};

void WriteDDS(byte *dat, int wdt, int hgt, int Size, char *name, EImFormat eF, int NumMips);

#endif


