//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Render a glyph outline into a bitmap using FreeType 2
//
// History:
//  - [6/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include "FontRenderer.h"
#include "FreeType2/include/freetype/ftoutln.h"
#include "FreeType2/include/freetype/ftglyph.h"
#include "FreeType2/include/freetype/ftimage.h"



//------------------------------------------------------------------------------------------------- 
CFontRenderer::CFontRenderer()
: m_pLibrary(0), m_pFace(0), m_pGlyph(0), m_fSizeRatio(0.8f), m_pEncoding(FONT_ENCODING_UNICODE),
  m_iGlyphBitmapWidth(0), m_iGlyphBitmapHeight(0)
{
}

//------------------------------------------------------------------------------------------------- 
CFontRenderer::~CFontRenderer()
{
	FT_Done_Face(m_pFace);;
	FT_Done_FreeType(m_pLibrary);
	m_pFace = NULL;
	m_pLibrary = NULL;
}

//------------------------------------------------------------------------------------------------- 
int CFontRenderer::LoadFromFile(const string &szFileName)
{
	int iError = FT_Init_FreeType(&m_pLibrary);

	if (iError)
	{
		return 0;
	}

	if (m_pFace)
	{
		FT_Done_Face(m_pFace);
		m_pFace = 0;
	}
	
	iError = FT_New_Face(m_pLibrary, szFileName.c_str(), 0, &m_pFace);

	if (iError)
	{
		return 0;
	}

	SetEncoding(FONT_ENCODING_UNICODE);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CFontRenderer::LoadFromMemory(unsigned char *pBuffer, int iBufferSize)
{
	int iError = FT_Init_FreeType(&m_pLibrary);

	if (iError)
	{
		return 0;
	}

	if (m_pFace)
	{
		FT_Done_Face(m_pFace);
		m_pFace = 0;
	}
	iError = FT_New_Memory_Face(m_pLibrary, pBuffer, iBufferSize, 0, &m_pFace);

	if (iError)
	{
		return 0;
	}

	SetEncoding(FONT_ENCODING_UNICODE);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CFontRenderer::Release()
{
	FT_Done_Face(m_pFace);;
	FT_Done_FreeType(m_pLibrary);
	m_pFace = NULL;
	m_pLibrary = NULL;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CFontRenderer::SetGlyphBitmapSize(int iWidth, int iHeight)
{
	m_iGlyphBitmapWidth = iWidth;
	m_iGlyphBitmapHeight = iHeight;

	FT_Set_Pixel_Sizes(m_pFace, (int)(m_iGlyphBitmapWidth * m_fSizeRatio), (int)(m_iGlyphBitmapHeight * m_fSizeRatio));

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int	CFontRenderer::GetGlyphBitmapSize(int *pWidth, int *pHeight)
{
	if (pWidth)
	{
		*pWidth = m_iGlyphBitmapWidth;
	}

	if (pHeight)
	{
		*pHeight = m_iGlyphBitmapHeight;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CFontRenderer::SetEncoding(FT_Encoding pEncoding)
{
	if (FT_Select_Charmap(m_pFace, pEncoding))
	{
		return 0;
	}

	return 1;
}




//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
/*int CFontRenderer::CreateFontBitmap(CFBitmap **pFontBitmap, Vec3d *pTexCoord, int iWidth, int iHeight)
{
	CFBitmap *pBitmap = new CFBitmap;
	
	if ((!pBitmap) || (!pBitmap->Create(iWidth, iHeight)))
	{
		if (pBitmap)
		{
			delete pBitmap;
		}

		return 0;
	}
	
	memset(pBitmap->GetData(), 0, pBitmap->GetWidth() * pBitmap->GetHeight());

	int iGlyphWidth = iWidth >> 4;
	int iGlyphHeight = iHeight >> 4;

	CFBitmap *pGlyph = new CFBitmap;

	if ((!pGlyph) || (!pGlyph->Create(iGlyphWidth, iGlyphHeight)))
	{
		pBitmap->Release();

		if (pGlyph)
		{
			pGlyph->Release();
		}

		return 0;
	}

	int iCharWidth;
	int iCharHeight;

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			memset(pGlyph->GetData(), 0, pGlyph->GetHeight() * pGlyph->GetWidth());

			int iCharCode = i * 16 + j;

			if (!FT_GetChar(pGlyph, &iCharWidth, &iCharHeight, 1, 0, iGlyphWidth, iGlyphHeight, iCharCode))
			{
				continue;
			}

            pTexCoord[i * 16 + j].z = ((float)iCharWidth / (float)iGlyphWidth);
			pBitmap->BlitFrom(pGlyph, 0, 0, j * iGlyphWidth, i * iGlyphHeight, iGlyphWidth, iGlyphHeight);
		}
	}

	pGlyph->Release();

	*pFontBitmap = pBitmap;

	return 1;
}

*/

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CFontRenderer::GetGlyph(CGlyphBitmap *pGlyphBitmap, int *iGlyphWidth, int *iGlyphHeight, int iX, int iY, int iCharCode)
{
#if defined(WIN64) && defined(_DEBUG)
	{
		char szBuf[0x100];
		sprintf (szBuf, "GetGlyph(%p,%p->%d,%p->%d,x=%d,y=%d,char=%d)\n", pGlyphBitmap, iGlyphWidth, *iGlyphWidth, iGlyphHeight, *iGlyphHeight, iX, iY, iCharCode);
		OutputDebugString (szBuf);
	}
#endif
	int iError = FT_Load_Char(m_pFace, iCharCode, FT_LOAD_DEFAULT);

	if (iError)
	{
		return 0;
	}

	m_pGlyph = m_pFace->glyph;

	iError = FT_Render_Glyph(m_pGlyph, FT_RENDER_MODE_NORMAL);

	if (iError)
	{
		return 0;
	}

	if (iGlyphWidth)
	{
		*iGlyphWidth = (m_pGlyph->metrics.width >> 6) + 1;
	}

	if (iGlyphHeight)
	{
		*iGlyphHeight = (m_pGlyph->metrics.height >> 6);
	}

	int				iTopOffset = (m_iGlyphBitmapHeight - (int)(m_iGlyphBitmapHeight * m_fSizeRatio)) + m_pGlyph->bitmap_top;
	unsigned char	*pBuffer = pGlyphBitmap->GetBuffer();

	for (int i = 0; i < m_pGlyph->bitmap.rows; i++)
	{
		int iNewY = i + (iY + m_iGlyphBitmapHeight - iTopOffset);

		for (int j = 0; j < m_pGlyph->bitmap.width; j++)
		{
			unsigned char	cColor = m_pGlyph->bitmap.buffer[(i * m_pGlyph->bitmap.width) + j];
			int				iOffset = iNewY * m_iGlyphBitmapWidth + iX + j;

			if ((iOffset >= m_iGlyphBitmapWidth * m_iGlyphBitmapHeight) || ( iOffset < 0))
			{
				continue;
			}

			pBuffer[iOffset] = cColor;
		}
	}

	return 1;
}

int	CFontRenderer::GetGlyphScaled(CGlyphBitmap *pGlyphBitmap, int *iGlyphWidth, int *iGlyphHeight, int iX, int iY, float fScaleX, float fScaleY, int iCharCode)
{
	return 1;
}

//------------------------------------------------------------------------------------------------- 
/*
int CFontRenderer::FT_GetIndex(int iCharCode)
{
	if (iCharCode < 256)
	{
		int iIndex = 0;
		int iUnicode;

		// try unicode
		for (int i = 0; i < m_pFace->num_charmaps; i++)
		{
			if ((m_pFace->charmaps[i]->platform_id == 3) && (m_pFace->charmaps[i]->encoding_id == 1))
			{
				iUnicode = i;

				FT_Set_Charmap(m_pFace, m_pFace->charmaps[i]);

				iIndex = FT_Get_Char_Index(m_pFace, iCharCode);

				// not unicode, try ascii
				if (iIndex == 0)
				{
					for (int i = 0; i < m_pFace->num_charmaps; i++)
					{
						if ((m_pFace->charmaps[i]->platform_id == 0) && (m_pFace->charmaps[i]->encoding_id == 0))
						{
							FT_Set_Charmap(m_pFace, m_pFace->charmaps[i]);

							iIndex = FT_Get_Char_Index(m_pFace, iCharCode);

							// not ascii either, reuse unicode default "missing char"
							if (iIndex == 0)
							{
								FT_Set_Charmap(m_pFace, m_pFace->charmaps[iUnicode]);

								return FT_Get_Char_Index(m_pFace, iCharCode);
							}
						}
					}
				}

				return  iIndex;
			}
		}

		return 0;
	}
	else
	{
		for (int i = 0; i < m_pFace->num_charmaps; i++)
		{
			if ((m_pFace->charmaps[i]->platform_id == 3) && (m_pFace->charmaps[i]->encoding_id == 1))
			{
				FT_Set_Charmap(m_pFace, m_pFace->charmaps[i]);

				return FT_Get_Char_Index(m_pFace, iCharCode);
			}
		}

		return 0;
	}

	return 0;
}
*/