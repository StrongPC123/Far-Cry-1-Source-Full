//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Create and update a texture with the most recently used glyphs
//
// History:
//  - [6/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include "FontTexture.h"


//-------------------------------------------------------------------------------------------------
CFontTexture::CFontTexture()
: m_dwUsage(1), m_iWidth(0), m_iHeight(0), m_iCellWidth(0), m_iCellHeight(0), m_fTextureCellWidth(0),
  m_fTextureCellHeight(0), m_iWidthCellCount(0), m_iHeightCellCount(0), m_nTextureSlotCount(0), m_pBuffer(0),
  m_iSmoothMethod(FONT_SMOOTH_NONE), m_iSmoothAmount(FONT_SMOOTH_AMOUNT_NONE)
{
	m_pSlotList.clear();
	m_pSlotTable.clear();
}

//-------------------------------------------------------------------------------------------------
CFontTexture::~CFontTexture()
{
	Release();
}

//-------------------------------------------------------------------------------------------------
int CFontTexture::CreateFromFile(const string &szFileName, int iWidth, int iHeight, int iSmoothMethod, int iSmoothAmount, float fSizeRatio, int iWidthCellCount, int iHeightCellCount)
{
	if (!m_pGlyphCache.LoadFontFromFile(szFileName))
	{
		Release();

		return 0;
	}

	if (!Create(iWidth, iHeight, iSmoothMethod, iSmoothAmount, fSizeRatio, iWidthCellCount, iHeightCellCount))
	{
		return 0;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CFontTexture::CreateFromMemory(unsigned char *pFileData, int iDataSize, int iWidth, int iHeight, int iSmoothMethod, int iSmoothAmount, float fSizeRatio, int iWidthCellCount, int iHeightCellCount)
{
	if (!m_pGlyphCache.LoadFontFromMemory(pFileData, iDataSize))
	{
		Release();

		return 0;
	}

	if (!Create(iWidth, iHeight, iSmoothMethod, iSmoothAmount, fSizeRatio, iWidthCellCount, iHeightCellCount))
	{
		return 0;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CFontTexture::Create(int iWidth, int iHeight, int iSmoothMethod, int iSmoothAmount, float fSizeRatio, int iWidthCellCount, int iHeightCellCount)
{
#ifdef FONT_USE_32BIT_TEXTURE
	m_pBuffer = new unsigned int[iWidth * iHeight];
#else
	m_pBuffer = new unsigned char[iWidth * iHeight];
#endif

	if (!m_pBuffer)
	{
		return 0;
	}

#ifdef FONT_USE_32BIT_TEXTURE
	memset(m_pBuffer, 0, iWidth * iHeight * 4);
#else
	memset(m_pBuffer, 0, iWidth * iHeight);
#endif

	if (!(iWidthCellCount * iHeightCellCount))
	{
		return 0;
	}

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_iWidthCellCount = iWidthCellCount;
	m_iHeightCellCount = iHeightCellCount;
	m_nTextureSlotCount = m_iWidthCellCount * m_iHeightCellCount;

	m_iSmoothMethod = iSmoothMethod;
	m_iSmoothAmount = iSmoothAmount;

	m_iCellWidth = m_iWidth / m_iWidthCellCount;
	m_iCellHeight = m_iHeight / m_iHeightCellCount;

	m_fTextureCellWidth = m_iCellWidth / (float)m_iWidth;
	m_fTextureCellHeight = m_iCellHeight / (float)m_iHeight;

	if (!m_pGlyphCache.Create(FONT_GLYPH_CACHE_SIZE, m_iCellWidth, m_iCellHeight, iSmoothMethod, iSmoothAmount, fSizeRatio))
	{
		Release();

		return 0;
	}

	if (!CreateSlotList(m_nTextureSlotCount))
	{
		Release();

		return 0;
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
int CFontTexture::Release()
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
	}

	m_pBuffer = 0;

	ReleaseSlotList();

	m_pSlotTable.clear();

	m_pGlyphCache.Release();

	m_iWidthCellCount = 0;
	m_iHeightCellCount = 0;
	m_nTextureSlotCount = 0;

	m_iWidth = 0;
	m_iHeight = 0;

	m_iCellWidth = 0;
	m_iCellHeight = 0;

	m_iSmoothMethod = 0;
	m_iSmoothAmount = 0;

	m_fTextureCellWidth = 0.0f;
	m_fTextureCellHeight = 0.0f;

	m_dwUsage = 1;

	return 1;
}

//-------------------------------------------------------------------------------------------------
wchar_t CFontTexture::GetSlotChar(int iSlot)
{
	return m_pSlotList[iSlot]->cCurrentChar;
}

//-------------------------------------------------------------------------------------------------
CTextureSlot *CFontTexture::GetCharSlot(wchar_t cChar)
{
	CTextureSlotTableItor pItor = m_pSlotTable.find(cChar);

	if (pItor != m_pSlotTable.end())
	{
		return pItor->second;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
CTextureSlot *CFontTexture::GetLRUSlot()
{
	unsigned int	dwMinUsage = 0xffffffff;
	CTextureSlot	*pLRUSlot = 0;
	CTextureSlot	*pSlot;

	CTextureSlotListItor pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		pSlot = *pItor;

		if (pSlot->dwUsage == 0)
		{
			return pSlot;
		}
		else
		{
			if (pSlot->dwUsage < dwMinUsage)
			{
				pLRUSlot = pSlot;
				dwMinUsage = pSlot->dwUsage;
			}
		}

		pItor++;
	}

	return pLRUSlot;
}

//-------------------------------------------------------------------------------------------------
CTextureSlot *CFontTexture::GetMRUSlot()
{
	unsigned int	dwMaxUsage = 0;
	CTextureSlot	*pMRUSlot = 0;
	CTextureSlot	*pSlot;

	CTextureSlotListItor pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		pSlot = *pItor;

		if (pSlot->dwUsage != 0)
		{
			if (pSlot->dwUsage > dwMaxUsage)
			{
				pMRUSlot = pSlot;
				dwMaxUsage = pSlot->dwUsage;
			}
		}

		pItor++;
	}

	return pMRUSlot;
}

//------------------------------------------------------------------------------------------------- 
int CFontTexture::PreCacheString(const wchar_t *szString, int *pUpdated)
{
	unsigned int dwUsage = m_dwUsage++;
	int iLength = wcslen(szString);
	int iUpdated = 0;

	for (int i = 0; i < iLength; i++)
	{
		wchar_t cChar = szString[i];

		CTextureSlot *pSlot = GetCharSlot(cChar);

		if (!pSlot)
		{
			pSlot = GetLRUSlot();

			if (!pSlot)
			{
				return 0;
			}

			if (!UpdateSlot(pSlot->iTextureSlot, dwUsage, cChar))
			{
				return 0;
			}

			++iUpdated;
		}
		else
		{
			pSlot->dwUsage = dwUsage;
		}
	}

	if (pUpdated)
	{
		*pUpdated = iUpdated;
	}

	if (iUpdated)
	{
		return 1;
	}

	return 2;
}

//------------------------------------------------------------------------------------------------- 
int CFontTexture::GetTextureCoord(wchar_t cChar, float *fU, float *fV)
{
	CTextureSlotTableItor pItor = m_pSlotTable.find(cChar);

	if (pItor == m_pSlotTable.end())
	{
		return 0;
	}

	if (fU)
	{
		*fU = pItor->second->vTexCoord[0];
	}

	if (fV)
	{
		*fV = pItor->second->vTexCoord[1];
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CFontTexture::GetCharWidth(wchar_t cChar)
{
	CTextureSlotTableItor pItor = m_pSlotTable.find(cChar);

	if (pItor != m_pSlotTable.end())
	{
		return pItor->second->iCharWidth;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CFontTexture::GetCharHeight(wchar_t cChar)
{
	CTextureSlotTableItor pItor = m_pSlotTable.find(cChar);

	if (pItor != m_pSlotTable.end())
	{
		return pItor->second->iCharHeight;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CFontTexture::WriteToFile(const string &szFileName)
{
	FILE *hFile = fopen(szFileName.c_str(), "wb");

	if(!hFile)
	{
		return 0;
	}

	BITMAPFILEHEADER pHeader;
	BITMAPINFOHEADER pInfoHeader;

	memset(&pHeader, 0, sizeof(BITMAPFILEHEADER));
	memset(&pInfoHeader, 0, sizeof(BITMAPINFOHEADER));
	
	pHeader.bfType = 0x4D42;
	pHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_iWidth * m_iHeight * 3;
	pHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	pInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	pInfoHeader.biWidth = m_iWidth;
	pInfoHeader.biHeight = m_iHeight;
	pInfoHeader.biPlanes = 1;
	pInfoHeader.biBitCount = 24;
	pInfoHeader.biCompression = 0;
	pInfoHeader.biSizeImage = m_iWidth * m_iHeight * 3;

	fwrite(&pHeader, 1, sizeof(BITMAPFILEHEADER), hFile);
	fwrite(&pInfoHeader, 1, sizeof(BITMAPINFOHEADER), hFile);

	unsigned char cRGB[3];

	for(int i = m_iHeight - 1; i >= 0; i--)
	{
		for(int j = 0; j < m_iWidth; j++)
		{
#ifdef FONT_USE_32BIT_TEXTURE
			cRGB[0] = ((char *)(m_pBuffer + i * m_iWidth + j))[3];
#else
			cRGB[0] = m_pBuffer[(i * m_iWidth) + j];
#endif
			cRGB[1] = *cRGB;

			cRGB[2] = *cRGB;

			fwrite(cRGB, 1, 3, hFile);
		}
	}

	fclose(hFile);

	return 1;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int CFontTexture::CreateSlotList(int iListSize)
{
	int y, x;

	for (int i = 0; i < iListSize; i++)
	{
		CTextureSlot *pTextureSlot = new CTextureSlot;

		if (!pTextureSlot)
		{
			return 0;
		}

		pTextureSlot->iTextureSlot = i;
		pTextureSlot->Reset();

		y = i / m_iWidthCellCount;
		x = i % m_iWidthCellCount;

		pTextureSlot->vTexCoord[0] = (float)(x * m_fTextureCellWidth);
		pTextureSlot->vTexCoord[1] = (float)(y * m_fTextureCellHeight);

		m_pSlotList.push_back(pTextureSlot);
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
int CFontTexture::ReleaseSlotList()
{
	CTextureSlotListItor pItor = m_pSlotList.begin();

	while (pItor != m_pSlotList.end())
	{
		delete (*pItor);

		pItor = m_pSlotList.erase(pItor);
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
int CFontTexture::UpdateSlot(int iSlot, unsigned int dwUsage, wchar_t cChar)
{
	CTextureSlot *pSlot = m_pSlotList[iSlot];

	if (!pSlot)
	{
		return 0;
	}

	CTextureSlotTableItor pItor = m_pSlotTable.find(pSlot->cCurrentChar);

	if (pItor != m_pSlotTable.end())
	{
		m_pSlotTable.erase(pItor);
	}

	m_pSlotTable.insert(std::pair<wchar_t, CTextureSlot *>(cChar, pSlot));

	pSlot->dwUsage = dwUsage;
	pSlot->cCurrentChar = cChar;

	int iWidth = 0;
	int iHeight = 0;
	
	// blit the char glyph into the texture
	int x = pSlot->iTextureSlot % m_iWidthCellCount;
	int y = pSlot->iTextureSlot / m_iWidthCellCount;

	CGlyphBitmap *pGlyphBitmap;

	if (!m_pGlyphCache.GetGlyph(&pGlyphBitmap, &iWidth, &iHeight, cChar))
	{
		return 0;
	}

	pSlot->iCharWidth = iWidth;
	pSlot->iCharHeight = iHeight;

#ifdef FONT_USE_32BIT_TEXTURE
	pGlyphBitmap->BlitTo32(m_pBuffer, 0, 0, pGlyphBitmap->GetWidth(), pGlyphBitmap->GetHeight(), x * m_iCellWidth, y * m_iCellHeight, m_iWidth);
#else
	pGlyphBitmap->BlitTo8(m_pBuffer, 0, 0, pGlyphBitmap->GetWidth(), pGlyphBitmap->GetHeight(), x * m_iCellWidth, y * m_iCellHeight, m_iWidth);
#endif

	return 1;
}

//------------------------------------------------------------------------------------------------- 