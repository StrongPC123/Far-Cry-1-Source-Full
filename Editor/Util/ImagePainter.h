////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   imagepainter.h
//  Version:     v1.00
//  Created:     11/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __imagepainter_h__
#define __imagepainter_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** Contains image painting functions.
*/
class CImagePainter
{
public:
	CImagePainter();
	~CImagePainter();

	enum EBrushType
	{
		PAINT_BRUSH,
		SMOOTH_BRUSH,
	};

	// Brush structure used for painting.
	struct SPaintBrush
	{
		EBrushType type;
		unsigned char color;	//!< Painting color.
		int radius;						//!< Inner radius.
		float hardness;				//!< 0-1 hardness of brush.
	};

	//! Paint spot on image at position px,py with specified paint brush parameters.
	void PaintBrush( CByteImage &image,int px,int py,SPaintBrush &brush );
	//void PaintBrush( int iX, int iY, int radius, float fHeigh,float fHardness=1.0f,bool bAddNoise = false,float noiseFreq=1,float noiseScale=1 );

private:
	void SmoothBrush( CByteImage &image,int px,int py,SPaintBrush &brush );
};


#endif // __imagepainter_h__
