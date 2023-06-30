/*
**----------------------------------------------------------------------------
**
**	Xtf File load/save for PS2
**	
**	(C) 2002, Ubi Soft
**	
**	Author:	Tiziano Sardone
**----------------------------------------------------------------------------
*/

#ifndef XTFIMAGE_H
#define XTFIMAGE_H

#define XTF_FILE_TYPE		0xABBA	///<file type
#define XTF_FILE_VERSION	0x0001	///<file version

///Xtf file format
typedef struct _SXtfHeader
{
	u_short	Type;	///<File version
	u_short	Version;	///<File version
	STexPic	Info;		///<Texture information
	u_char	Pad  __attribute__ ( ( aligned( 16 ) ) );
}SXtfHeader;


/**
 * An ImageFile subclass for reading XTF files.
 */
class CImageXtfFile : public CImageFile
{
	///
	friend class CImageFile;	// For constructor

	SXtfHeader XtfFile;
	
private:
	CImageXtfFile (void);
	CImageXtfFile(byte* ptr, long filesize);

	static bool CImageXtfFileSave(const char *name, STexPic *pTexture);
	static bool CImageXtfFileSave(const char *name, u_char *pTexture, u_int dimx, u_int dimy);

	/// Read the XTF file 
	static STexPic* CImageXtfFileLoad (const char *Filename);
public:
	///
	virtual ~CImageXtfFile ();
};



#endif

