// declarations of control structures in Zip File
#ifndef _ZIP_FILE_FORMAT_HDR_
#define _ZIP_FILE_FORMAT_HDR_

#if !defined(LINUX)
#pragma pack(push)
#pragma pack(1)
#define PACK_GCC
#else
#define PACK_GCC __attribute__ ((packed))
#endif

namespace ZipFile
{
	typedef unsigned int ulong;
	typedef unsigned short ushort;

	// General-purpose bit field flags
	enum {
		GPF_ENCRYPTED = 1, // If set, indicates that the file is encrypted.
		GPF_DATA_DESCRIPTOR = 1 << 3, // if set, the CRC32 and sizes aren't set in the file header, but only in the data descriptor following compressed data
		GPF_RESERVED_8_ENHANCED_DEFLATING = 1 << 4, // Reserved for use with method 8, for enhanced deflating.
		GPF_COMPRESSED_PATCHED = 1 << 5, // the file is compressed patched data
	};

	// compression methods
	enum {
		METHOD_STORE  = 0, // The file is stored (no compression)
		METHOD_SHRINK = 1, // The file is Shrunk
		METHOD_REDUCE_1 = 2, // The file is Reduced with compression factor 1
		METHOD_REDUCE_2 = 3, // The file is Reduced with compression factor 2
		METHOD_REDUCE_3 = 4, // The file is Reduced with compression factor 3
		METHOD_REDUCE_4 = 5, // The file is Reduced with compression factor 4
		METHOD_IMPLODE  = 6, // The file is Imploded
		METHOD_TOKENIZE = 7, // Reserved for Tokenizing compression algorithm
		METHOD_DEFLATE  = 8, // The file is Deflated
		METHOD_DEFLATE64 = 9, // Enhanced Deflating using Deflate64(tm)
		METHOD_IMPLODE_PKWARE = 10// PKWARE Date Compression Library Imploding
	};


	// end of Central Directory Record
	// followed by the .zip file comment (variable size, can be empty, obtained from nCommentLength)
	struct CDREnd
	{
		enum {SIGNATURE = 0x06054b50};
		ulong  lSignature;       // end of central dir signature    4 bytes  (0x06054b50)
		ushort nDisk;            // number of this disk             2 bytes
		ushort nCDRStartDisk;    // number of the disk with the start of the central directory  2 bytes
		ushort numEntriesOnDisk; // total number of entries in the central directory on this disk  2 bytes
		ushort numEntriesTotal;  // total number of entries in the central directory           2 bytes
		ulong  lCDRSize;          // size of the central directory   4 bytes
		ulong  lCDROffset;        // offset of start of central directory with respect to the starting disk number        4 bytes
		ushort nCommentLength;   // .ZIP file comment length        2 bytes
		// .ZIP file comment (variable size, can be empty) follows
	} PACK_GCC;

	// This descriptor exists only if bit 3 of the general
	// purpose bit flag is set (see below).  It is byte aligned
	// and immediately follows the last byte of compressed data.
	// This descriptor is used only when it was not possible to
	// seek in the output .ZIP file, e.g., when the output .ZIP file
	// was standard output or a non seekable device.  For Zip64 format
	// archives, the compressed and uncompressed sizes are 8 bytes each.
	struct DataDescriptor
	{
		ulong lCRC32;             // crc-32                          4 bytes
		ulong lSizeCompressed;    // compressed size                 4 bytes
		ulong lSizeUncompressed;  // uncompressed size               4 bytes

		bool operator == (const DataDescriptor& d)const
		{
			return lCRC32 == d.lCRC32 && lSizeCompressed == d.lSizeCompressed && lSizeUncompressed == d.lSizeUncompressed;
		}
		bool operator != (const DataDescriptor& d)const
		{
			return lCRC32 != d.lCRC32 || lSizeCompressed != d.lSizeCompressed || lSizeUncompressed != d.lSizeUncompressed;
		}
	} PACK_GCC;

	// the File Header as it appears in the CDR
	// followed by:
	//    file name (variable size)
	//    extra field (variable size)
	//    file comment (variable size)
	struct CDRFileHeader
	{
		enum {SIGNATURE = 0x02014b50};
		ulong  lSignature;         // central file header signature   4 bytes  (0x02014b50)
		ushort nVersionMadeBy;     // version made by                 2 bytes
		ushort nVersionNeeded;     // version needed to extract       2 bytes
		ushort nFlags;             // general purpose bit flag        2 bytes
		ushort nMethod;            // compression method              2 bytes
		ushort nLastModTime;       // last mod file time              2 bytes
		ushort nLastModDate;       // last mod file date              2 bytes
		DataDescriptor desc;
		ushort nFileNameLength;    // file name length                2 bytes
		ushort nExtraFieldLength;  // extra field length              2 bytes
		ushort nFileCommentLength; // file comment length             2 bytes
		ushort nDiskNumberStart;   // disk number start               2 bytes
		ushort nAttrInternal;      // internal file attributes        2 bytes
		ulong  lAttrExternal;      // external file attributes        4 bytes

		// This is the offset from the start of the first disk on
		// which this file appears, to where the local header should
		// be found.  If an archive is in zip64 format and the value
		// in this field is 0xFFFFFFFF, the size will be in the
		// corresponding 8 byte zip64 extended information extra field.
		enum {ZIP64_LOCAL_HEADER_OFFSET = 0xFFFFFFFF};
		ulong  lLocalHeaderOffset; // relative offset of local header 4 bytes
	} PACK_GCC;


	// this is the local file header that appears before the compressed data
	// followed by:
	//    file name (variable size)
	//    extra field (variable size)
	struct LocalFileHeader
	{
		enum {SIGNATURE = 0x04034b50};
		ulong  lSignature;        // local file header signature     4 bytes  (0x04034b50)
		ushort nVersionNeeded;    // version needed to extract       2 bytes
		ushort nFlags;            // general purpose bit flag        2 bytes
		ushort nMethod;           // compression method              2 bytes
		ushort nLastModTime;      // last mod file time              2 bytes
		ushort nLastModDate;      // last mod file date              2 bytes
		DataDescriptor desc;
		ushort nFileNameLength;   // file name length                2 bytes
		ushort nExtraFieldLength; // extra field length              2 bytes
	} PACK_GCC;

}

#undef PACK_GCC

#if !defined(LINUX)
#pragma pack(pop)
#endif

#endif // _ZIP_FILE_FORMAT_HDR_