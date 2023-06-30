//////////////////////////////////////////////////////////////////////
//
//  CryFont Source Code
//
//  File: FBitmap.cpp
//  Description: Bitmap class implementation.
//
//  History:
//  - August 17, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FBitmap.h"
//#include <Image.h>
#include <stdio.h>
//
/////////////////////////////////////////////////
//CFBitmap::CFBitmap(int nWidth, int nHeight)
//{
//	m_bOK = false;
//
//	m_nWidth = nWidth;
//	m_nHeight = nHeight;
//
//	m_pData = new unsigned long[m_nWidth*m_nHeight];
//	if(!m_pData)
//	{
//		CryError( "<CryFont> Only 32bpp images are supported" );
//		return;	// only 32 bits image are supported
//	}
//
//	m_bOK = true;
//	m_pIRenderData = NULL;
//}
//
/////////////////////////////////////////////////
///*CFBitmap::CFBitmap(CImage *pImg)
//{
//	m_bOK = false;
//
//	if(pImg->GetBpp() != 32)
//	{
//		_asm int 3	// <<FIXME>>
//		return;	// only 32 bits image are supported
//	}
//
//	m_nWidth = pImg->GetWidth();/
//	m_nHeight = pImg->GetHeight();
//
//	m_pData = new unsigned long[m_nWidth*m_nHeight];
//	if(!m_pData)
//	{
//		_asm int 3	// <<FIXME>>
//		return; // unable to allocate the memory, return immediatly so IsOK() will return false
//	}
//
//	memcpy(m_pData, pImg->GetData(), m_nWidth*m_nHeight*4);
//
//	m_bOK;
//	m_pIRenderData = NULL;
//}*/
//
/////////////////////////////////////////////////
//CFBitmap::~CFBitmap()
//{
//	if(m_pData)
//	{
//		delete [] m_pData;
//		m_pData = NULL;
//	}
//	m_nWidth = 0;
//	m_nHeight = 0;
//	m_bOK = false;
//	m_pIRenderData = NULL;
//}
//
/////////////////////////////////////////////////
//void CFBitmap::Release()
//{
//	delete this;
//}
//
/////////////////////////////////////////////////
//bool CFBitmap::SmoothAndScale1On4()
//{
//	int i, j;
//	int sum;
//	int xoffset;
//	int yoffset;
//	unsigned long *ptr;
//
//	// create the destination buffer
//	int w = m_nWidth >> 2;
//	int h = m_nHeight >> 2;
//	unsigned long *pNew = new unsigned long[w*h];	
//	if(!pNew)
//		return false;
//	
//	// fill it
//	for (i = 0; i < w; i++)
//	{
//		for (j = 0; j < h; j++)
//		{
//			sum = 0;
//			xoffset = i << 2;
//			yoffset = j << 2;
//			
//			ptr = m_pData + (GetWidth()*yoffset++) + xoffset;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr)>>24;
//			
//			ptr = m_pData + (GetWidth()*yoffset++) + xoffset;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr)>>24;
//			
//			ptr = m_pData + (GetWidth()*yoffset++) + xoffset;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr)>>24;
//			
//			ptr = m_pData + (GetWidth()*yoffset) + xoffset;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr++)>>24;
//			sum += (*ptr)>>24;
//			
//			pNew[(j*w) + i] = ((sum>>4) << 24) | ((sum>>4) << 16) | ((sum>>4) << 8) | (sum>>4);
//		}
//	}
//
//	// set the new size
//	delete [] m_pData;
//	m_nWidth = w;
//	m_nHeight = h;
//	m_pData = pNew;
//
//	return true;
//}
//
/////////////////////////////////////////////////
//void CFBitmap::BlitFrom(CFBitmap *pSrc, int iSX, int iSY, int iDX, int iDY, int iW, int iH)
//{
//	int sy = iSY;
//	int dy = iDY;
//	while(iH--)
//	{
//		int w = iW;
//		unsigned long *pS = pSrc->GetData() + (sy*pSrc->GetWidth()) + iSX;
//		unsigned long *pD = m_pData + (dy*m_nWidth) + iDX;
//		while(w--)
//		{
//			*(pD++) = *(pS++);
//		}
//		++dy;
//		++sy;
//	}
//}

///////////////////////////////////////////////
/*
bool CFBitmap::SaveBitmap(const char *szFile, bool bSaveRGB)
{
	FILE *fp = fxopen(szFile, "wb");
	if(!fp)
		return false;

	int bfSize = 54 + m_nWidth*m_nHeight*3;
	int i, j;
	unsigned long c;
	unsigned char rgb[3];
	unsigned short tmp16;
	unsigned long tmp32;

	#define W16(val) tmp16 = val; fwrite(&tmp16,2,1,fp);
	#define W32(val) tmp32 = val; fwrite(&tmp32,4,1,fp);

	W16(0x4D42);
	W32(bfSize);
	W16(0);
	W16(0);

	W32(54);
	
	bfSize = m_nWidth * m_nHeight * 3;

	W32(40);
	W32(m_nWidth);
	W32(m_nHeight);
	W16(1);
	W16(24);
	W32(0);
	W32(bfSize);
	W32(0);
	W32(0);
	W32(0);
	W32(0);
	
	if(!bSaveRGB)
	{
		for(i = m_nHeight-1; i >= 0; i--)
		{
			for(j = 0; j < m_nWidth; j++)
			{
				c = (m_pData[(i*m_nWidth)+j]>>24)&0xFF;
				rgb[0] = (unsigned char)c;
				rgb[1] = (unsigned char)c;
				rgb[2] = (unsigned char)c;
				fwrite(rgb,3,1,fp);
			}
		}
	}
	else
	{
		for(i = m_nHeight-1; i >= 0; i--)
		{
			for(j = 0; j < m_nWidth; j++)
			{
				c = m_pData[(i*m_nWidth)+j];
				rgb[2] = (unsigned char)((c>>16)&0xFF);
				rgb[1] = (unsigned char)((c>>8)&0xFF);
				rgb[0] = (unsigned char)(c&0xFF);
				fwrite(rgb,3,1,fp);
			}
		}
	}
	
	fclose(fp);
	return true;
}

// puts the size of this structure and all its contained objects in bytes into the sizer
void CFBitmap::GetMemoryUsage (class ICrySizer* pSizer)
{
	pSizer->Add (*this);
	pSizer->Add (m_pData, m_nWidth*m_nHeight);
}
*/



//------------------------------------------------------------------------------------
CFBitmap::CFBitmap()
: m_iWidth(0), m_iHeight(0), m_pData(0), m_pIRenderData(0)
{
}

//------------------------------------------------------------------------------------
CFBitmap::~CFBitmap()
{
}

//------------------------------------------------------------------------------------
int CFBitmap::Blur(int iIterations)
{
	int cSum;
	int yOffset;
	int yupOffset;
	int ydownOffset;

	for (int i = 0; i < iIterations; i++)
	{
		for (int y = 0; y < m_iHeight; y++)
		{
			yOffset = y * m_iWidth;

			if (y - 1 >= 0)
			{
				yupOffset = (y-1) * m_iWidth;
			}
			else
			{
				yupOffset = (y) * m_iWidth;
			}

			if (y + 1 <= m_iHeight)
			{
				ydownOffset = (y+1) * m_iWidth;
			}
			else
			{
				ydownOffset = (y) * m_iWidth;
			}

			for (int x = 0; x < m_iWidth; x++)
			{
				cSum = m_pData[yupOffset + x] + m_pData[ydownOffset + x];

				if (x - 1 >= 0)
				{
					cSum += m_pData[yOffset + x - 1];
				}
				else
				{
					cSum += m_pData[yOffset + x];
				}

				if (x + 1 < m_iWidth)
				{
					cSum += m_pData[yOffset + x + 1];
				}
				else
				{
					cSum += m_pData[yOffset + x];
				}

				m_pData[yOffset + x] = cSum >> 2;
			}
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------
int CFBitmap::Scale(float fScaleX, float fScaleY)
{
	int iNewWidth = (int)(m_iWidth * fScaleX);
	int iNewHeight = (int)(m_iHeight * fScaleY);

	unsigned char *pNewData = new unsigned char[iNewWidth * iNewHeight];

	if (!pNewData)
	{
		return 0;
	}

	float xFactor = m_iWidth / (float)iNewWidth;
	float yFactor = m_iHeight / (float)iNewHeight;

	float xFractioned, yFractioned, xFraction, yFraction, oneMinusX, oneMinusY, fR0, fR1;
	int xCeil, yCeil, xFloor, yFloor, yNewOffset;

	unsigned char c0, c1, c2, c3;

	for (int y = 0; y < iNewHeight; ++y)
	{
		yFractioned = y * yFactor;
		yFloor = (int)floorf(yFractioned);
		yCeil = yFloor + 1;

		if (yCeil >= m_iHeight)
		{
			yCeil = yFloor;
		}

		yFraction = yFractioned - yFloor;
		oneMinusY = 1.0f - yFraction;

//		yOffset = y * m_iWidth;
		yNewOffset = y * iNewWidth;


		for (int x = 0; x < iNewWidth; ++x)
		{
			xFractioned = x * xFactor;
			xFloor = (int)floorf(xFractioned);
			xCeil = xFloor + 1;

			if (xCeil >= m_iWidth)
			{
				xCeil = xFloor;
			}

			xFraction = xFractioned - xFloor;
			oneMinusX = 1.0f - xFraction;
			
			c0 = m_pData[yFloor * m_iWidth + xFloor];
			c1 = m_pData[yFloor * m_iWidth + xCeil];
			c2 = m_pData[yCeil * m_iWidth + xFloor];
			c3 = m_pData[yCeil * m_iWidth + xCeil];

			fR0 = (oneMinusX * c0 + xFraction * c1);
			fR1 = (oneMinusX * c2 + xFraction * c3);

			pNewData[yNewOffset + x] = (unsigned char)((oneMinusY * fR0) + (yFraction * fR1));
		}
	}

	m_iWidth = iNewWidth;
	m_iHeight = iNewHeight;

	delete[] m_pData;
	m_pData = pNewData;

	return 1;
}

//------------------------------------------------------------------------------------
int CFBitmap::BlitFrom(CFBitmap *pSrc, int iSX, int iSY, int iDX, int iDY, int iW, int iH)
{
	for (int y = 0; y < iH; y++)
	{
		for (int x = 0; x < iW; x++)
		{
			m_pData[(iDY + y) * m_iWidth + (iDX + x)] = pSrc->m_pData[(iSY + y) * pSrc->m_iWidth + (iSX + x)];
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------
int CFBitmap::BlitTo(CFBitmap *pDst, int iDX, int iDY, int iSX, int iSY, int iW, int iH)
{
	for (int y = 0; y < iH; y++)
	{
		for (int x = 0; x < iW; x++)
		{
			pDst->m_pData[(iDY + y) * pDst->m_iWidth + (iDX + x)] = m_pData[(iSY + y) * m_iWidth + (iSX + x)];
		}
	}
	
	return 1;
}

//------------------------------------------------------------------------------------
int CFBitmap::Create(int iWidth, int iHeight)
{
	SAFE_DELETE_ARRAY(m_pData);

	m_pData = new unsigned char [iWidth * iHeight];

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	return 1;
}

//------------------------------------------------------------------------------------
int CFBitmap::Release()
{
	SAFE_DELETE_ARRAY(m_pData);
	m_iWidth = m_iHeight = 0;
//	m_pIRenderData = 0;

	delete this;

	return 1;
}


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
#define W16(val) { unsigned short t16 = (val); fwrite(&t16, 2, 1, hFile); }
#define W32(val) { unsigned int t32 = (val); fwrite(&t32, 4, 1, hFile); }
//------------------------------------------------------------------------------------
int CFBitmap::SaveBitmap(const std::string &szFileName)
{
	FILE *hFile = fopen(szFileName.c_str(), "wb");

	if(!hFile)
	{
		return 0;
	}

	int iFileSize = 54 + (m_iWidth * m_iHeight * 3);

	W16(0x4D42);
	W32(iFileSize);
	W16(0);
	W16(0);
	
	W32(54);

	W32(40);

	W32(m_iWidth);
	W32(m_iHeight);

	iFileSize = m_iWidth * m_iHeight * 3;

	W16(1);
	W16(24);
	W32(0);
	W32(iFileSize);
	W32(0);
	W32(0);
	W32(0);
	W32(0);

	for(int i = m_iHeight - 1; i >= 0; i--)
	{
		for(int j = 0; j < m_iWidth; j++)
		{
			fwrite(&m_pData[(i * m_iWidth) + j], 1, 1, hFile);
			fwrite(&m_pData[(i * m_iWidth) + j], 1, 1, hFile);
			fwrite(&m_pData[(i * m_iWidth) + j], 1, 1, hFile);
		}
	}

	fclose(hFile);

	return 1;
}

//------------------------------------------------------------------------------------
void CFBitmap::GetMemoryUsage (class ICrySizer *pSizer)
{
	pSizer->Add (*this);
	pSizer->Add (m_pData, m_iWidth * m_iHeight);
}