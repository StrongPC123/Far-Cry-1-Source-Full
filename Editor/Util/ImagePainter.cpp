////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   imagepainter.cpp
//  Version:     v1.00
//  Created:     11/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ImagePainter.h"
#include "Image.h"

//////////////////////////////////////////////////////////////////////////
CImagePainter::CImagePainter()
{}

//////////////////////////////////////////////////////////////////////////
CImagePainter::~CImagePainter()
{}

//////////////////////////////////////////////////////////////////////////
void CImagePainter::PaintBrush( TImage<unsigned char> &image,int px,int py,SPaintBrush &brush )
{
	if (brush.type == SMOOTH_BRUSH)
	{
		SmoothBrush( image,px,py,brush );
		return;
	}
	////////////////////////////////////////////////////////////////////////
	// Draw an attenuated spot on the map
	////////////////////////////////////////////////////////////////////////
	int i, j;
	int iPosX, iPosY;
	float fMaxDist, fAttenuation, fYSquared;
	float fHardness = brush.hardness;

	unsigned int pos;

	unsigned char *src = image.GetData();

	int value = brush.color;

	// Calculate the maximum distance
	fMaxDist = brush.radius;

	int width = image.GetWidth();
	int height = image.GetHeight();

	for (j = -brush.radius; j < brush.radius; j++)
	{
		// Precalculate
		iPosY = py + j;
		fYSquared = (float)(j*j);

		for (i = -brush.radius; i < brush.radius; i++)
		{
			// Calculate the position
			iPosX = px + i;

			// Skip invalid locations
			if (iPosX < 0 || iPosY < 0 ||	iPosX > width - 1 || iPosY > height - 1)
				continue;

			// Only circle.
			float dist = sqrtf(fYSquared + i*i);
			if (dist > fMaxDist)
				continue;

			// Calculate the array index
			pos = iPosX + iPosY * width;

			// Calculate attenuation factor
			fAttenuation = 1.0f - __min(1.0f, dist/fMaxDist);

			int h = src[pos];
			int dh = value - h;
			float fh = (fAttenuation)*dh*fHardness + h;
			
			src[pos] = ftoi(fh);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CImagePainter::SmoothBrush( CByteImage &image,int px,int py,SPaintBrush &brush )
{
	////////////////////////////////////////////////////////////////////////
	// Draw an attenuated spot on the map
	////////////////////////////////////////////////////////////////////////
	int i, j;
	int iPosX, iPosY;
	float fMaxDist, fYSquared;
	float fHardness = brush.hardness;

	unsigned int pos;

	unsigned char *src = image.GetData();

	int value = brush.color;

	// Calculate the maximum distance
	fMaxDist = brush.radius;

	int width = image.GetWidth();
	int height = image.GetHeight();
	int maxsize = width*height;

	for (j = -brush.radius; j < brush.radius; j++)
	{
		// Precalculate
		iPosY = py + j;
		fYSquared = (float)(j*j);

		for (i = -brush.radius; i < brush.radius; i++)
		{
			// Calculate the position
			iPosX = px + i;

			// Skip invalid locations
			if (iPosX < 1 || iPosY < 1 ||	iPosX > width - 2 || iPosY > height - 2)
				continue;

			// Only circle.
			float dist = sqrtf(fYSquared + i*i);
			if (dist > fMaxDist)
				continue;

			// Calculate the array index
			pos = iPosX + iPosY * width;

			// Calculate attenuation factor
			float currH = src[pos];
			float h =	
				((uint)currH +
					src[pos+1] +
					src[pos-1] +
					src[pos+width] +
					src[pos-width] + 
					src[pos+1+width] +
					src[pos+1-width] +
					src[pos-1+width] +
					src[pos-1-width]
				) / 9.0f;

			src[pos] = FloatToIntRet(currH + (h-currH)*fHardness);
		}
	}
}