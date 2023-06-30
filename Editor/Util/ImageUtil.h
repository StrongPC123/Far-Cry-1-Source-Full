////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ImageUtil.h
//  Version:     v1.00
//  Created:     30/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Image utilities.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ImageUtil_h__
#define __ImageUtil_h__

#if _MSC_VER > 1000
#pragma once
#endif

/*!
 *	Utility Class to manipulate images.
 */
class CImageUtil
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Image loading.
	//////////////////////////////////////////////////////////////////////////
	//! Load image, detect image type by file extension.
	static bool LoadImage( const CString &fileName, CImage &image );
	//! Save image, detect image type by file extension.
	static bool SaveImage( const CString &fileName, CImage &image );

	// General image fucntions
	static bool LoadJPEG( const CString &strFileName,CImage &image );
	static bool SaveJPEG( const CString &strFileName,CImage &image );

	static bool SaveBitmap( const CString &szFileName, CImage &image,bool inverseY=true );
	static bool SaveBitmap(LPCSTR szFileName, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, HDC hdc);
	static bool LoadBmp( const CString &file,CImage &image );
	
	//! Save image in PGM format.
	static bool SavePGM( const CString &fileName, uint dwWidth, uint dwHeight, uint *pData);
	//! Load image in PGM format.
	static bool LoadPGM( const CString &fileName, uint *pWidthOut, uint *pHeightOut, uint **pImageDataOut);

	//////////////////////////////////////////////////////////////////////////
	// Image scaling.
	//////////////////////////////////////////////////////////////////////////
	//! Scale source image to fit size of target image.
	static void ScaleToFit( const CByteImage &srcImage,CByteImage &trgImage );
	//! Scale source image to fit size of target image.
	static void ScaleToFit( const CImage &srcImage,CImage &trgImage );

	//! Smooth image.
	static void SmoothImage( CByteImage &image,int numSteps );

	//////////////////////////////////////////////////////////////////////////
	// filtered lookup 
	//////////////////////////////////////////////////////////////////////////

	//! behaviour outside of the texture is not defined
	//! \param iniX in fix point 24.8
	//! \param iniY in fix point 24.8
	//! \return 0..255
	static unsigned char GetBilinearFilteredAt( const int iniX256, const int iniY256, const CByteImage &image );
};

#endif // __ImageUtil_h__
